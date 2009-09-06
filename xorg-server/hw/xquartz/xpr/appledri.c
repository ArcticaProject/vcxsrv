/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright 2000 VA Linux Systems, Inc.
Copyright (c) 2002, 2009 Apple Computer, Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Jens Owen <jens@valinux.com>
 *   Rickard E. (Rik) Faith <faith@valinux.com>
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _APPLEDRI_SERVER_
#include "appledristr.h"
#include "swaprep.h"
#include "dri.h"
#include "dristruct.h"
#include "xpr.h"
#include "x-hash.h"

static int DRIErrorBase = 0;

static DISPATCH_PROC(ProcAppleDRIDispatch);
static DISPATCH_PROC(SProcAppleDRIDispatch);

static void AppleDRIResetProc(ExtensionEntry* extEntry);
static int ProcAppleDRICreatePixmap(ClientPtr client);

static unsigned char DRIReqCode = 0;
static int DRIEventBase = 0;

static void SNotifyEvent(xAppleDRINotifyEvent *from, xAppleDRINotifyEvent *to);

typedef struct _DRIEvent *DRIEventPtr;
typedef struct _DRIEvent {
    DRIEventPtr     next;
    ClientPtr       client;
    XID             clientResource;
    unsigned int    mask;
} DRIEventRec;


void
AppleDRIExtensionInit(void)
{
    ExtensionEntry* extEntry;

    if (DRIExtensionInit() &&
        (extEntry = AddExtension(APPLEDRINAME,
                                 AppleDRINumberEvents,
                                 AppleDRINumberErrors,
                                 ProcAppleDRIDispatch,
                                 SProcAppleDRIDispatch,
                                 AppleDRIResetProc,
                                 StandardMinorOpcode))) {
        DRIReqCode = (unsigned char)extEntry->base;
        DRIErrorBase = extEntry->errorBase;
        DRIEventBase = extEntry->eventBase;
        EventSwapVector[DRIEventBase] = (EventSwapPtr) SNotifyEvent;
    }
}

/*ARGSUSED*/
static void
AppleDRIResetProc (
    ExtensionEntry* extEntry
)
{
    DRIReset();
}

static int
ProcAppleDRIQueryVersion(
    register ClientPtr client
)
{
    xAppleDRIQueryVersionReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xAppleDRIQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = APPLE_DRI_MAJOR_VERSION;
    rep.minorVersion = APPLE_DRI_MINOR_VERSION;
    rep.patchVersion = APPLE_DRI_PATCH_VERSION;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof(xAppleDRIQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}


/* surfaces */

static int
ProcAppleDRIQueryDirectRenderingCapable(
    register ClientPtr client
)
{
    xAppleDRIQueryDirectRenderingCapableReply rep;
    Bool isCapable;

    REQUEST(xAppleDRIQueryDirectRenderingCapableReq);
    REQUEST_SIZE_MATCH(xAppleDRIQueryDirectRenderingCapableReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    if (!DRIQueryDirectRenderingCapable( screenInfo.screens[stuff->screen], 
                                         &isCapable)) {
        return BadValue;
    }
    rep.isCapable = isCapable;

    if (!LocalClient(client))
        rep.isCapable = 0;

    WriteToClient(client, 
        sizeof(xAppleDRIQueryDirectRenderingCapableReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcAppleDRIAuthConnection(
    register ClientPtr client
)
{
    xAppleDRIAuthConnectionReply rep;
    
    REQUEST(xAppleDRIAuthConnectionReq);
    REQUEST_SIZE_MATCH(xAppleDRIAuthConnectionReq);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.authenticated = 1;

    if (!DRIAuthConnection( screenInfo.screens[stuff->screen], stuff->magic)) {
        ErrorF("Failed to authenticate %u\n", (unsigned int)stuff->magic);
        rep.authenticated = 0;
    }
    WriteToClient(client, sizeof(xAppleDRIAuthConnectionReply), (char *)&rep);
    return (client->noClientException);
}

static void surface_notify(
    void *_arg,
    void *data
)
{
    DRISurfaceNotifyArg *arg = _arg;
    int client_index = (int) x_cvt_vptr_to_uint(data);
    ClientPtr client;
    xAppleDRINotifyEvent se;

    if (client_index < 0 || client_index >= currentMaxClients)
        return;

    client = clients[client_index];
    if (client == NULL || client == serverClient || client->clientGone)
        return;

    se.type = DRIEventBase + AppleDRISurfaceNotify;
    se.kind = arg->kind;
    se.arg = arg->id;
    se.sequenceNumber = client->sequence;
    se.time = currentTime.milliseconds;
    WriteEventsToClient (client, 1, (xEvent *) &se);
}

static int
ProcAppleDRICreateSurface(
    ClientPtr client
)
{
    xAppleDRICreateSurfaceReply rep;
    DrawablePtr pDrawable;
    xp_surface_id sid;
    unsigned int key[2];
    int rc;

    REQUEST(xAppleDRICreateSurfaceReq);
    REQUEST_SIZE_MATCH(xAppleDRICreateSurfaceReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    rc = dixLookupDrawable(&pDrawable, stuff->drawable, client, 0,
			   DixReadAccess);
    if (rc != Success)
	return rc;

    rep.key_0 = rep.key_1 = rep.uid = 0;

    if (!DRICreateSurface( screenInfo.screens[stuff->screen],
                           (Drawable)stuff->drawable, pDrawable,
                           stuff->client_id, &sid, key,
                           surface_notify,
                           x_cvt_uint_to_vptr(client->index))) {
        return BadValue;
    }

    rep.key_0 = key[0];
    rep.key_1 = key[1];
    rep.uid = sid;

    WriteToClient(client, sizeof(xAppleDRICreateSurfaceReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcAppleDRIDestroySurface(
    register ClientPtr client
)
{
    REQUEST(xAppleDRIDestroySurfaceReq);
    DrawablePtr pDrawable;
    REQUEST_SIZE_MATCH(xAppleDRIDestroySurfaceReq);
    int rc;

    rc = dixLookupDrawable(&pDrawable, stuff->drawable, client, 0,
			   DixReadAccess);
    if (rc != Success)
	return rc;

    if (!DRIDestroySurface( screenInfo.screens[stuff->screen], 
                            (Drawable)stuff->drawable,
                            pDrawable, NULL, NULL)) {
        return BadValue;
    }

    return (client->noClientException);
}

static int
ProcAppleDRICreatePixmap(ClientPtr client)
{
    REQUEST(xAppleDRICreatePixmapReq);
    DrawablePtr pDrawable;
    int rc;
    char path[PATH_MAX];
    xAppleDRICreatePixmapReply rep;
    int width, height, pitch, bpp;
    void *ptr;

    REQUEST_SIZE_MATCH(xAppleDRICreatePixmapReq);

    rc = dixLookupDrawable(&pDrawable, stuff->drawable, client, 0,
                           DixReadAccess);

    if(rc != Success)
        return rc;
    
    if(!DRICreatePixmap(screenInfo.screens[stuff->screen],
                              (Drawable)stuff->drawable,
                              pDrawable,
			      path, PATH_MAX)) {
        return BadValue;
    }

    if(!DRIGetPixmapData(pDrawable, &width, &height,
			 &pitch, &bpp, &ptr)) {
	return BadValue;
    } 
	
    rep.stringLength = strlen(path) + 1;
		
    /* No need for swapping, because this only runs if LocalClient is true. */
    rep.type = X_Reply;
    rep.length = sizeof(rep) + rep.stringLength;
    rep.sequenceNumber = client->sequence;
    rep.width = width;
    rep.height = height;
    rep.pitch = pitch;
    rep.bpp = bpp;
    rep.size = pitch * height;

    if(sizeof(rep) != sz_xAppleDRICreatePixmapReply)
	ErrorF("error sizeof(rep) is %zu\n", sizeof(rep)); 
    
    WriteReplyToClient(client, sizeof(rep), &rep);
    (void)WriteToClient(client, rep.stringLength, path);

    return (client->noClientException);
}

static int
ProcAppleDRIDestroyPixmap(ClientPtr client)
{
    DrawablePtr pDrawable;
    int rc;
    REQUEST(xAppleDRIDestroyPixmapReq);
    REQUEST_SIZE_MATCH(xAppleDRIDestroyPixmapReq);

    rc = dixLookupDrawable(&pDrawable, stuff->drawable, client, 0,
			    DixReadAccess);

    if(rc != Success)
	return rc;
    
    DRIDestroyPixmap(pDrawable);

    return (client->noClientException);
}

/* dispatch */

static int
ProcAppleDRIDispatch (
    register ClientPtr client
)
{
    REQUEST(xReq);

    switch (stuff->data)
    {
    case X_AppleDRIQueryVersion:
        return ProcAppleDRIQueryVersion(client);
    case X_AppleDRIQueryDirectRenderingCapable:
        return ProcAppleDRIQueryDirectRenderingCapable(client);
    }

    if (!LocalClient(client))
        return DRIErrorBase + AppleDRIClientNotLocal;

    switch (stuff->data)
    {
    case X_AppleDRIAuthConnection:
        return ProcAppleDRIAuthConnection(client);
    case X_AppleDRICreateSurface:
        return ProcAppleDRICreateSurface(client);
    case X_AppleDRIDestroySurface:
        return ProcAppleDRIDestroySurface(client);
    case X_AppleDRICreatePixmap:
	return ProcAppleDRICreatePixmap(client);
    case X_AppleDRIDestroyPixmap:
	return ProcAppleDRIDestroyPixmap(client);

    default:
        return BadRequest;
    }
}

static void
SNotifyEvent(
    xAppleDRINotifyEvent *from,
    xAppleDRINotifyEvent *to
)
{
    to->type = from->type;
    to->kind = from->kind;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->time, to->time);
    cpswapl (from->arg, to->arg);
}

static int
SProcAppleDRIQueryVersion(
    register ClientPtr client
)
{
    register int n;
    REQUEST(xAppleDRIQueryVersionReq);
    swaps(&stuff->length, n);
    return ProcAppleDRIQueryVersion(client);
}

static int
SProcAppleDRIDispatch (
    register ClientPtr client
)
{
    REQUEST(xReq);

    /* It is bound to be non-local when there is byte swapping */
    if (!LocalClient(client))
        return DRIErrorBase + AppleDRIClientNotLocal;

    /* only local clients are allowed DRI access */
    switch (stuff->data)
    {
    case X_AppleDRIQueryVersion:
        return SProcAppleDRIQueryVersion(client);
    default:
        return BadRequest;
    }
}
