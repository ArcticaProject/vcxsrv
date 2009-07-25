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
#include <X11/extensions/xfixeswire.h>
#include "dixstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "xf86drm.h"
#include "xfixes.h"
#include "dri2.h"

/* The only xf86 include */
#include "xf86Module.h"

static ExtensionEntry	*dri2Extension;
static RESTYPE		 dri2DrawableRes;

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
    rep.majorVersion = 1;
    rep.minorVersion = 1;

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
    DrawablePtr pDraw;
    int fd, status;
    const char *driverName;
    const char *deviceName;

    REQUEST_SIZE_MATCH(xDRI2ConnectReq);
    if (!validDrawable(client, stuff->window, &pDraw, &status))
	return status;
    
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.driverNameLength = 0;
    rep.deviceNameLength = 0;

    if (!DRI2Connect(pDraw->pScreen,
		     stuff->driverType, &fd, &driverName, &deviceName))
	goto fail;

    rep.driverNameLength = strlen(driverName);
    rep.deviceNameLength = strlen(deviceName);
    rep.length = (rep.driverNameLength + 3) / 4 +
	    (rep.deviceNameLength + 3) / 4;

 fail:
    WriteToClient(client, sizeof(xDRI2ConnectReply), &rep);
    WriteToClient(client, rep.driverNameLength, driverName);
    WriteToClient(client, rep.deviceNameLength, deviceName);

    return client->noClientException;
}

static int
ProcDRI2Authenticate(ClientPtr client)
{
    REQUEST(xDRI2AuthenticateReq);
    xDRI2AuthenticateReply rep;
    DrawablePtr pDraw;
    int status;

    REQUEST_SIZE_MATCH(xDRI2AuthenticateReq);
    if (!validDrawable(client, stuff->window, &pDraw, &status))
	return status;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.authenticated = DRI2Authenticate(pDraw->pScreen, stuff->magic);
    WriteToClient(client, sizeof(xDRI2AuthenticateReply), &rep);

    return client->noClientException;
}

static int
ProcDRI2CreateDrawable(ClientPtr client)
{
    REQUEST(xDRI2CreateDrawableReq);
    DrawablePtr pDrawable;
    int status;

    REQUEST_SIZE_MATCH(xDRI2CreateDrawableReq);

    if (!validDrawable(client, stuff->drawable, &pDrawable, &status))
	return status;

    status = DRI2CreateDrawable(pDrawable);
    if (status != Success)
	return status;

    if (!AddResource(stuff->drawable, dri2DrawableRes, pDrawable)) {
	DRI2DestroyDrawable(pDrawable);
	return BadAlloc;
    }

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


static void
send_buffers_reply(ClientPtr client, DrawablePtr pDrawable,
		   DRI2Buffer2Ptr *buffers, int count, int width, int height)
{
    xDRI2GetBuffersReply rep;
    int skip = 0;
    int i;

    if (pDrawable->type == DRAWABLE_WINDOW) {
	for (i = 0; i < count; i++) {
	    /* Do not send the real front buffer of a window to the client.
	     */
	    if (buffers[i]->attachment == DRI2BufferFrontLeft) {
		skip++;
		continue;
	    }
	}
    }

    rep.type = X_Reply;
    rep.length = (count - skip) * sizeof(xDRI2Buffer) / 4;
    rep.sequenceNumber = client->sequence;
    rep.width = width;
    rep.height = height;
    rep.count = count - skip;
    WriteToClient(client, sizeof(xDRI2GetBuffersReply), &rep);

    for (i = 0; i < count; i++) {
	xDRI2Buffer buffer;

	/* Do not send the real front buffer of a window to the client.
	 */
	if ((pDrawable->type == DRAWABLE_WINDOW)
	    && (buffers[i]->attachment == DRI2BufferFrontLeft)) {
	    continue;
	}

	buffer.attachment = buffers[i]->attachment;
	buffer.name = buffers[i]->name;
	buffer.pitch = buffers[i]->pitch;
	buffer.cpp = buffers[i]->cpp;
	buffer.flags = buffers[i]->flags;
	WriteToClient(client, sizeof(xDRI2Buffer), &buffer);
    }
}


static int
ProcDRI2GetBuffers(ClientPtr client)
{
    REQUEST(xDRI2GetBuffersReq);
    DrawablePtr pDrawable;
    DRI2Buffer2Ptr *buffers;
    int status, width, height, count;
    unsigned int *attachments;

    REQUEST_FIXED_SIZE(xDRI2GetBuffersReq, stuff->count * 4);
    if (!validDrawable(client, stuff->drawable, &pDrawable, &status))
	return status;

    attachments = (unsigned int *) &stuff[1];
    buffers = DRI2GetBuffers(pDrawable, &width, &height,
			     attachments, stuff->count, &count);


    send_buffers_reply(client, pDrawable, buffers, count, width, height);

    return client->noClientException;
}

static int
ProcDRI2GetBuffersWithFormat(ClientPtr client)
{
    REQUEST(xDRI2GetBuffersReq);
    DrawablePtr pDrawable;
    DRI2Buffer2Ptr *buffers;
    int status, width, height, count;
    unsigned int *attachments;

    REQUEST_FIXED_SIZE(xDRI2GetBuffersReq, stuff->count * (2 * 4));
    if (!validDrawable(client, stuff->drawable, &pDrawable, &status))
	return status;

    attachments = (unsigned int *) &stuff[1];
    buffers = DRI2GetBuffersWithFormat(pDrawable, &width, &height,
				       attachments, stuff->count, &count);

    send_buffers_reply(client, pDrawable, buffers, count, width, height);

    return client->noClientException;
}

static int
ProcDRI2CopyRegion(ClientPtr client)
{
    REQUEST(xDRI2CopyRegionReq);
    xDRI2CopyRegionReply rep;
    DrawablePtr pDrawable;
    int status;
    RegionPtr pRegion;

    REQUEST_SIZE_MATCH(xDRI2CopyRegionReq);

    if (!validDrawable(client, stuff->drawable, &pDrawable, &status))
	return status;

    VERIFY_REGION(pRegion, stuff->region, client, DixReadAccess);

    status = DRI2CopyRegion(pDrawable, pRegion, stuff->dest, stuff->src);
    if (status != Success)
	return status;

    /* CopyRegion needs to be a round trip to make sure the X server
     * queues the swap buffer rendering commands before the DRI client
     * continues rendering.  The reply has a bitmask to signal the
     * presense of optional return values as well, but we're not using
     * that yet.
     */

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    WriteToClient(client, sizeof(xDRI2CopyRegionReply), &rep);

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
    case X_DRI2Authenticate:
	return ProcDRI2Authenticate(client);
    case X_DRI2CreateDrawable:
	return ProcDRI2CreateDrawable(client);
    case X_DRI2DestroyDrawable:
	return ProcDRI2DestroyDrawable(client);
    case X_DRI2GetBuffers:
	return ProcDRI2GetBuffers(client);
    case X_DRI2CopyRegion:
	return ProcDRI2CopyRegion(client);
    case X_DRI2GetBuffersWithFormat:
	return ProcDRI2GetBuffersWithFormat(client);
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
    rep.deviceNameLength = 0;

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
				 NULL,
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
