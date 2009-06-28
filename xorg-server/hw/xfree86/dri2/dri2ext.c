/*
 * Copyright © 2008 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Soft-
 * ware"), to deal in the Software without restriction, including without
 * limitation the rights to use, copy, modify, merge, publish, distribute,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, provided that the above copyright
 * notice(s) and this permission notice appear in all copies of the Soft-
 * ware and that both the above copyright notice(s) and this permission
 * notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
 * ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY
 * RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
 * THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSE-
 * QUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFOR-
 * MANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization of
 * the copyright holder.
 *
 * Authors:
 *   Kristian Høgsberg (krh@redhat.com)
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#define NEED_REPLIES
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/extensions/dri2proto.h>
#include "dixstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "xf86drm.h"
#include "dri2.h"

/* The only xf86 include */
#include "xf86Module.h"

static ExtensionEntry	*dri2Extension;
static RESTYPE		 dri2DrawableRes;

static Bool
validScreen(ClientPtr client, int screen, ScreenPtr *pScreen)
{
    if (screen >= screenInfo.numScreens) {
	client->errorValue = screen;
	return FALSE;
    }

    *pScreen = screenInfo.screens[screen];

    return TRUE;
}

static Bool
validDrawable(ClientPtr client, XID drawable,
	      DrawablePtr *pDrawable, int *status)
{
    *status = dixLookupDrawable(pDrawable, drawable, client, 0, DixReadAccess);
    if (*status != Success) {
	client->errorValue = drawable;
	return FALSE;
    }

    return TRUE;
}

static int
ProcDRI2QueryVersion(ClientPtr client)
{
    REQUEST(xDRI2QueryVersionReq);
    xDRI2QueryVersionReply rep;
    int n;

    if (client->swapped)
	swaps(&stuff->length, n);

    REQUEST_SIZE_MATCH(xDRI2QueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = DRI2_MAJOR;
    rep.minorVersion = DRI2_MINOR;

    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.majorVersion, n);
	swapl(&rep.minorVersion, n);
    }

    WriteToClient(client, sizeof(xDRI2QueryVersionReply), &rep);

    return client->noClientException;
}

static int
ProcDRI2Connect(ClientPtr client)
{
    REQUEST(xDRI2ConnectReq);
    xDRI2ConnectReply rep;
    ScreenPtr pScreen;
    int fd;
    const char *driverName;
    char *busId = NULL;
    unsigned int sareaHandle;

    REQUEST_SIZE_MATCH(xDRI2ConnectReq);
    if (!validScreen(client, stuff->screen, &pScreen))
	return BadValue;
    
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.driverNameLength = 0;
    rep.busIdLength = 0;
    rep.sareaHandle = 0;

    if (!DRI2Connect(pScreen, &fd, &driverName, &sareaHandle))
	goto fail;

    busId = drmGetBusid(fd);
    if (busId == NULL)
	goto fail;

    rep.driverNameLength = strlen(driverName);
    rep.busIdLength = strlen(busId);
    rep.sareaHandle = sareaHandle;
    rep.length = (rep.driverNameLength + 3) / 4 + (rep.busIdLength + 3) / 4;

 fail:
    WriteToClient(client, sizeof(xDRI2ConnectReply), &rep);
    WriteToClient(client, rep.driverNameLength, driverName);
    WriteToClient(client, rep.busIdLength, busId);
    drmFreeBusid(busId);

    return client->noClientException;
}

static int
ProcDRI2AuthConnection(ClientPtr client)
{
    REQUEST(xDRI2AuthConnectionReq);
    xDRI2AuthConnectionReply rep;
    ScreenPtr pScreen;

    REQUEST_SIZE_MATCH(xDRI2AuthConnectionReq);
    if (!validScreen(client, stuff->screen, &pScreen))
	return BadValue;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.authenticated = 1;

    if (!DRI2AuthConnection(pScreen, stuff->magic)) {
        ErrorF("DRI2: Failed to authenticate %lu\n",
	       (unsigned long) stuff->magic);
	rep.authenticated = 0;
    }

    WriteToClient(client, sizeof(xDRI2AuthConnectionReply), &rep);

    return client->noClientException;
}

static int
ProcDRI2CreateDrawable(ClientPtr client)
{
    REQUEST(xDRI2CreateDrawableReq);
    xDRI2CreateDrawableReply rep;
    DrawablePtr pDrawable;
    unsigned int handle, head;
    int status;

    REQUEST_SIZE_MATCH(xDRI2CreateDrawableReq);

    if (!validDrawable(client, stuff->drawable, &pDrawable, &status))
	return status;

    if (!DRI2CreateDrawable(pDrawable, &handle, &head))
	return BadMatch;

    if (!AddResource(stuff->drawable, dri2DrawableRes, pDrawable)) {
	DRI2DestroyDrawable(pDrawable);
	return BadAlloc;
    }

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.handle = handle;
    rep.head = head;

    WriteToClient(client, sizeof(xDRI2CreateDrawableReply), &rep);

    return client->noClientException;
}

static int
ProcDRI2DestroyDrawable(ClientPtr client)
{
    REQUEST(xDRI2DestroyDrawableReq);
    DrawablePtr pDrawable;
    int status;

    REQUEST_SIZE_MATCH(xDRI2DestroyDrawableReq);
    if (!validDrawable(client, stuff->drawable, &pDrawable, &status))
	return status;

    FreeResourceByType(stuff->drawable, dri2DrawableRes, FALSE);

    return client->noClientException;
}

static int
ProcDRI2ReemitDrawableInfo(ClientPtr client)
{
    REQUEST(xDRI2ReemitDrawableInfoReq);
    xDRI2ReemitDrawableInfoReply rep;
    DrawablePtr pDrawable;
    unsigned int head;
    int status;

    REQUEST_SIZE_MATCH(xDRI2ReemitDrawableInfoReq);
    if (!validDrawable(client, stuff->drawable, &pDrawable, &status))
	return status;

    DRI2ReemitDrawableInfo(pDrawable, &head);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.head = head;

    WriteToClient(client, sizeof(xDRI2ReemitDrawableInfoReply), &rep);

    return client->noClientException;
}

static int
ProcDRI2Dispatch (ClientPtr client)
{
    REQUEST(xReq);
    
    switch (stuff->data) {
    case X_DRI2QueryVersion:
	return ProcDRI2QueryVersion(client);
    }

    if (!LocalClient(client))
	return BadRequest;

    switch (stuff->data) {
    case X_DRI2Connect:
	return ProcDRI2Connect(client);
    case X_DRI2AuthConnection:
	return ProcDRI2AuthConnection(client);
    case X_DRI2CreateDrawable:
	return ProcDRI2CreateDrawable(client);
    case X_DRI2DestroyDrawable:
	return ProcDRI2DestroyDrawable(client);
    case X_DRI2ReemitDrawableInfo:
	return ProcDRI2ReemitDrawableInfo(client);
    default:
	return BadRequest;
    }
}

static int
SProcDRI2Connect(ClientPtr client)
{
    REQUEST(xDRI2ConnectReq);
    xDRI2ConnectReply rep;
    int n;

    /* If the client is swapped, it's not local.  Talk to the hand. */

    swaps(&stuff->length, n);
    if (sizeof(*stuff) / 4 != client->req_len)
	return BadLength;

    rep.sequenceNumber = client->sequence;
    swaps(&rep.sequenceNumber, n);
    rep.length = 0;
    rep.driverNameLength = 0;
    rep.busIdLength = 0;
    rep.sareaHandle = 0;

    return client->noClientException;
}

static int
SProcDRI2Dispatch (ClientPtr client)
{
    REQUEST(xReq);

    /*
     * Only local clients are allowed DRI access, but remote clients
     * still need these requests to find out cleanly.
     */
    switch (stuff->data)
    {
    case X_DRI2QueryVersion:
	return ProcDRI2QueryVersion(client);
    case X_DRI2Connect:
	return SProcDRI2Connect(client);
    default:
	return BadRequest;
    }
}

static void
DRI2ResetProc (ExtensionEntry *extEntry)
{
}

static int DRI2DrawableGone(pointer p, XID id)
{
    DrawablePtr pDrawable = p;

    DRI2DestroyDrawable(pDrawable);

    return Success;
}

static void
DRI2ExtensionInit(void)
{
    dri2Extension = AddExtension(DRI2_NAME,
				 DRI2NumberEvents,
				 DRI2NumberErrors,
				 ProcDRI2Dispatch,
				 SProcDRI2Dispatch,
				 DRI2ResetProc,
				 StandardMinorOpcode);

    dri2DrawableRes = CreateNewResourceType(DRI2DrawableGone);
}

extern Bool noDRI2Extension;

_X_HIDDEN ExtensionModule dri2ExtensionModule = {
    DRI2ExtensionInit,
    DRI2_NAME,
    &noDRI2Extension,
    NULL,
    NULL
};
