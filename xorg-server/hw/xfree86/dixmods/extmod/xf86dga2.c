/*
 * Copyright (c) 1995  Jon Tombs
 * Copyright (c) 1995, 1996, 1999  XFree86 Inc
 * Copyright (c) 1999 - The XFree86 Project Inc.
 *
 * Written by Mark Vojkovich
 */


#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#define NEED_REPLIES
#define NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "dixstruct.h"
#include "dixevents.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86dgastr.h>
#include "swaprep.h"
#include "dgaproc.h"
#include "xf86dgaext.h"

#include <string.h>

#include "modinit.h"

#define DGA_PROTOCOL_OLD_SUPPORT 1

static DISPATCH_PROC(ProcXDGADispatch);
static DISPATCH_PROC(SProcXDGADispatch);
static DISPATCH_PROC(ProcXDGAQueryVersion);
static DISPATCH_PROC(ProcXDGAQueryModes);
static DISPATCH_PROC(ProcXDGASetMode);
static DISPATCH_PROC(ProcXDGAOpenFramebuffer);
static DISPATCH_PROC(ProcXDGACloseFramebuffer);
static DISPATCH_PROC(ProcXDGASetViewport);
static DISPATCH_PROC(ProcXDGAInstallColormap);
static DISPATCH_PROC(ProcXDGASelectInput);
static DISPATCH_PROC(ProcXDGAFillRectangle);
static DISPATCH_PROC(ProcXDGACopyArea);
static DISPATCH_PROC(ProcXDGACopyTransparentArea);
static DISPATCH_PROC(ProcXDGAGetViewportStatus);
static DISPATCH_PROC(ProcXDGASync);
static DISPATCH_PROC(ProcXDGASetClientVersion);
static DISPATCH_PROC(ProcXDGAChangePixmapMode);
static DISPATCH_PROC(ProcXDGACreateColormap);

static void XDGAResetProc(ExtensionEntry *extEntry);

static void DGAClientStateChange (CallbackListPtr*, pointer, pointer);

static ClientPtr DGAClients[MAXSCREENS];

unsigned char DGAReqCode = 0;
int DGAErrorBase;
int DGAEventBase;

static int DGAClientPrivateKeyIndex;
static DevPrivateKey DGAClientPrivateKey = &DGAClientPrivateKeyIndex;
static int DGACallbackRefCount = 0;

/* This holds the client's version information */
typedef struct {
    int		major;
    int		minor;
} DGAPrivRec, *DGAPrivPtr;

#define DGA_GETPRIV(c) ((DGAPrivPtr) \
    dixLookupPrivate(&(c)->devPrivates, DGAClientPrivateKey))
#define DGA_SETPRIV(c,p) \
    dixSetPrivate(&(c)->devPrivates, DGAClientPrivateKey, p)


void
XFree86DGAExtensionInit(INITARGS)
{
    ExtensionEntry* extEntry;

    if ((extEntry = AddExtension(XF86DGANAME,
				XF86DGANumberEvents,
				XF86DGANumberErrors,
				ProcXDGADispatch,
				SProcXDGADispatch,
				XDGAResetProc,
				StandardMinorOpcode))) {
	int i;

	for(i = 0; i < MAXSCREENS; i++)
	     DGAClients[i] = NULL;

	DGAReqCode = (unsigned char)extEntry->base;
	DGAErrorBase = extEntry->errorBase;
	DGAEventBase = extEntry->eventBase;
	for (i = KeyPress; i <= MotionNotify; i++)
	    SetCriticalEvent (DGAEventBase + i);
    }
}



static void
XDGAResetProc (ExtensionEntry *extEntry)
{
   DeleteCallback (&ClientStateCallback, DGAClientStateChange, NULL);
   DGACallbackRefCount = 0;
}


static int
ProcXDGAQueryVersion(ClientPtr client)
{
    xXDGAQueryVersionReply rep;

    REQUEST_SIZE_MATCH(xXDGAQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XDGA_MAJOR_VERSION;
    rep.minorVersion = XDGA_MINOR_VERSION;

    WriteToClient(client, sizeof(xXDGAQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}


static int
ProcXDGAOpenFramebuffer(ClientPtr client)
{
    REQUEST(xXDGAOpenFramebufferReq);
    xXDGAOpenFramebufferReply rep;
    char *deviceName;
    int nameSize;

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if (!DGAAvailable(stuff->screen))
        return DGAErrorBase + XF86DGANoDirectVideoMode;

    REQUEST_SIZE_MATCH(xXDGAOpenFramebufferReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    if(!DGAOpenFramebuffer(stuff->screen, &deviceName,
			(unsigned char**)(&rep.mem1),
			(int*)&rep.size, (int*)&rep.offset, (int*)&rep.extra))
    {
	return BadAlloc;
    }

    nameSize = deviceName ? (strlen(deviceName) + 1) : 0;
    rep.length = (nameSize + 3) >> 2;

    WriteToClient(client, sizeof(xXDGAOpenFramebufferReply), (char *)&rep);
    if(rep.length)
	WriteToClient(client, nameSize, deviceName);

    return (client->noClientException);
}


static int
ProcXDGACloseFramebuffer(ClientPtr client)
{
    REQUEST(xXDGACloseFramebufferReq);

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if (!DGAAvailable(stuff->screen))
        return DGAErrorBase + XF86DGANoDirectVideoMode;

    REQUEST_SIZE_MATCH(xXDGACloseFramebufferReq);

    DGACloseFramebuffer(stuff->screen);

    return (client->noClientException);
}

static int
ProcXDGAQueryModes(ClientPtr client)
{
    int i, num, size;
    REQUEST(xXDGAQueryModesReq);
    xXDGAQueryModesReply rep;
    xXDGAModeInfo info;
    XDGAModePtr mode;

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    REQUEST_SIZE_MATCH(xXDGAQueryModesReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.number = 0;
    rep.sequenceNumber = client->sequence;

    if (!DGAAvailable(stuff->screen)) {
	rep.number = 0;
	rep.length = 0;
	WriteToClient(client, sz_xXDGAQueryModesReply, (char*)&rep);
	return (client->noClientException);
    }

    if(!(num = DGAGetModes(stuff->screen))) {
	WriteToClient(client, sz_xXDGAQueryModesReply, (char*)&rep);
	return (client->noClientException);
    }

    if(!(mode = (XDGAModePtr)xalloc(num * sizeof(XDGAModeRec))))
	return BadAlloc;

    for(i = 0; i < num; i++)
	DGAGetModeInfo(stuff->screen, mode + i, i + 1);

    size = num * sz_xXDGAModeInfo;
    for(i = 0; i < num; i++)
	size += (strlen(mode[i].name) + 4) & ~3L;  /* plus NULL */

    rep.number = num;
    rep.length = size >> 2;

    WriteToClient(client, sz_xXDGAQueryModesReply, (char*)&rep);

    for(i = 0; i < num; i++) {
	size = strlen(mode[i].name) + 1;

	info.byte_order = mode[i].byteOrder;
	info.depth = mode[i].depth;
	info.num = mode[i].num;
	info.bpp = mode[i].bitsPerPixel;
	info.name_size = (size + 3) & ~3L;
	info.vsync_num = mode[i].VSync_num;
	info.vsync_den = mode[i].VSync_den;
	info.flags = mode[i].flags;
	info.image_width = mode[i].imageWidth;
	info.image_height = mode[i].imageHeight;
	info.pixmap_width = mode[i].pixmapWidth;
	info.pixmap_height = mode[i].pixmapHeight;
	info.bytes_per_scanline = mode[i].bytesPerScanline;
	info.red_mask = mode[i].red_mask;
	info.green_mask = mode[i].green_mask;
	info.blue_mask = mode[i].blue_mask;
	info.visual_class = mode[i].visualClass;
	info.viewport_width = mode[i].viewportWidth;
	info.viewport_height = mode[i].viewportHeight;
	info.viewport_xstep = mode[i].xViewportStep;
	info.viewport_ystep = mode[i].yViewportStep;
 	info.viewport_xmax = mode[i].maxViewportX;
	info.viewport_ymax = mode[i].maxViewportY;
	info.viewport_flags = mode[i].viewportFlags;
	info.reserved1 = mode[i].reserved1;
	info.reserved2 = mode[i].reserved2;

	WriteToClient(client, sz_xXDGAModeInfo, (char*)(&info));
	WriteToClient(client, size, mode[i].name);
    }

    xfree(mode);

    return (client->noClientException);
}


static void
DGAClientStateChange (
    CallbackListPtr* pcbl,
    pointer nulldata,
    pointer calldata
){
    NewClientInfoRec* pci = (NewClientInfoRec*) calldata;
    ClientPtr client = NULL;
    int i;

    for(i = 0; i < screenInfo.numScreens; i++) {
	if(DGAClients[i] == pci->client) {
	   client = pci->client;
	   break;
	}
    }

    if(client &&
      ((client->clientState == ClientStateGone) ||
       (client->clientState == ClientStateRetained))) {
	XDGAModeRec mode;
	PixmapPtr pPix;

	DGAClients[i] = NULL;
	DGASelectInput(i, NULL, 0);
	DGASetMode(i, 0, &mode, &pPix);

	if(--DGACallbackRefCount == 0)
	    DeleteCallback(&ClientStateCallback, DGAClientStateChange, NULL);
    }
}

static int
ProcXDGASetMode(ClientPtr client)
{
    REQUEST(xXDGASetModeReq);
    xXDGASetModeReply rep;
    XDGAModeRec mode;
    xXDGAModeInfo info;
    PixmapPtr pPix;
    int size;

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    REQUEST_SIZE_MATCH(xXDGASetModeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.offset = 0;
    rep.flags = 0;
    rep.sequenceNumber = client->sequence;

    if (!DGAAvailable(stuff->screen))
        return DGAErrorBase + XF86DGANoDirectVideoMode;

    if(DGAClients[stuff->screen] &&
      (DGAClients[stuff->screen] != client))
        return DGAErrorBase + XF86DGANoDirectVideoMode;

    if(!stuff->mode) {
	if(DGAClients[stuff->screen]) {
	  if(--DGACallbackRefCount == 0)
	    DeleteCallback(&ClientStateCallback, DGAClientStateChange, NULL);
	}
	DGAClients[stuff->screen] = NULL;
	DGASelectInput(stuff->screen, NULL, 0);
	DGASetMode(stuff->screen, 0, &mode, &pPix);
	WriteToClient(client, sz_xXDGASetModeReply, (char*)&rep);
	return (client->noClientException);
    }

    if(Success != DGASetMode(stuff->screen, stuff->mode, &mode, &pPix))
	return BadValue;

    if(!DGAClients[stuff->screen]) {
	if(DGACallbackRefCount++ == 0)
	   AddCallback (&ClientStateCallback, DGAClientStateChange, NULL);
    }

    DGAClients[stuff->screen] = client;

    if(pPix) {
	if(AddResource(stuff->pid, RT_PIXMAP, (pointer)(pPix))) {
	    pPix->drawable.id = (int)stuff->pid;
	    rep.flags = DGA_PIXMAP_AVAILABLE;
	}
    }

    size = strlen(mode.name) + 1;

    info.byte_order = mode.byteOrder;
    info.depth = mode.depth;
    info.num = mode.num;
    info.bpp = mode.bitsPerPixel;
    info.name_size = (size + 3) & ~3L;
    info.vsync_num = mode.VSync_num;
    info.vsync_den = mode.VSync_den;
    info.flags = mode.flags;
    info.image_width = mode.imageWidth;
    info.image_height = mode.imageHeight;
    info.pixmap_width = mode.pixmapWidth;
    info.pixmap_height = mode.pixmapHeight;
    info.bytes_per_scanline = mode.bytesPerScanline;
    info.red_mask = mode.red_mask;
    info.green_mask = mode.green_mask;
    info.blue_mask = mode.blue_mask;
    info.visual_class = mode.visualClass;
    info.viewport_width = mode.viewportWidth;
    info.viewport_height = mode.viewportHeight;
    info.viewport_xstep = mode.xViewportStep;
    info.viewport_ystep = mode.yViewportStep;
    info.viewport_xmax = mode.maxViewportX;
    info.viewport_ymax = mode.maxViewportY;
    info.viewport_flags = mode.viewportFlags;
    info.reserved1 = mode.reserved1;
    info.reserved2 = mode.reserved2;

    rep.length = (sz_xXDGAModeInfo + info.name_size) >> 2;

    WriteToClient(client, sz_xXDGASetModeReply, (char*)&rep);
    WriteToClient(client, sz_xXDGAModeInfo, (char*)(&info));
    WriteToClient(client, size, mode.name);

    return (client->noClientException);
}

static int
ProcXDGASetViewport(ClientPtr client)
{
    REQUEST(xXDGASetViewportReq);

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGASetViewportReq);

    DGASetViewport(stuff->screen, stuff->x, stuff->y, stuff->flags);

    return (client->noClientException);
}

static int
ProcXDGAInstallColormap(ClientPtr client)
{
    ColormapPtr cmap;
    REQUEST(xXDGAInstallColormapReq);

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGAInstallColormapReq);

    cmap = (ColormapPtr)LookupIDByType(stuff->cmap, RT_COLORMAP);
    if (cmap) {
        DGAInstallCmap(cmap);
        return (client->noClientException);
    } else {
        client->errorValue = stuff->cmap;
        return (BadColor);
    }

    return (client->noClientException);
}


static int
ProcXDGASelectInput(ClientPtr client)
{
    REQUEST(xXDGASelectInputReq);

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGASelectInputReq);

    if(DGAClients[stuff->screen] == client)
	DGASelectInput(stuff->screen, client, stuff->mask);

    return (client->noClientException);
}


static int
ProcXDGAFillRectangle(ClientPtr client)
{
    REQUEST(xXDGAFillRectangleReq);

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGAFillRectangleReq);

    if(Success != DGAFillRect(stuff->screen, stuff->x, stuff->y,
			stuff->width, stuff->height, stuff->color))
	return BadMatch;

    return (client->noClientException);
}

static int
ProcXDGACopyArea(ClientPtr client)
{
    REQUEST(xXDGACopyAreaReq);

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGACopyAreaReq);

    if(Success != DGABlitRect(stuff->screen, stuff->srcx, stuff->srcy,
		stuff->width, stuff->height, stuff->dstx, stuff->dsty))
	return BadMatch;

    return (client->noClientException);
}


static int
ProcXDGACopyTransparentArea(ClientPtr client)
{
    REQUEST(xXDGACopyTransparentAreaReq);

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGACopyTransparentAreaReq);

    if(Success != DGABlitTransRect(stuff->screen, stuff->srcx, stuff->srcy,
	stuff->width, stuff->height, stuff->dstx, stuff->dsty, stuff->key))
	return BadMatch;

    return (client->noClientException);
}


static int
ProcXDGAGetViewportStatus(ClientPtr client)
{
    REQUEST(xXDGAGetViewportStatusReq);
    xXDGAGetViewportStatusReply rep;

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGAGetViewportStatusReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    rep.status = DGAGetViewportStatus(stuff->screen);

    WriteToClient(client, sizeof(xXDGAGetViewportStatusReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXDGASync(ClientPtr client)
{
    REQUEST(xXDGASyncReq);
    xXDGASyncReply rep;

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGASyncReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    DGASync(stuff->screen);

    WriteToClient(client, sizeof(xXDGASyncReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXDGASetClientVersion(ClientPtr client)
{
    REQUEST(xXDGASetClientVersionReq);

    DGAPrivPtr pPriv;

    REQUEST_SIZE_MATCH(xXDGASetClientVersionReq);
    if ((pPriv = DGA_GETPRIV(client)) == NULL) {
	pPriv = xalloc(sizeof(DGAPrivRec));
	/* XXX Need to look into freeing this */
	if (!pPriv)
	    return BadAlloc;
	DGA_SETPRIV(client, pPriv);
    }
    pPriv->major = stuff->major;
    pPriv->minor = stuff->minor;

    return (client->noClientException);
}

static int
ProcXDGAChangePixmapMode(ClientPtr client)
{
    REQUEST(xXDGAChangePixmapModeReq);
    xXDGAChangePixmapModeReply rep;
    int x, y;

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGAChangePixmapModeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    x = stuff->x;
    y = stuff->y;

    if(!DGAChangePixmapMode(stuff->screen, &x, &y, stuff->flags))
	return BadMatch;

    rep.x = x;
    rep.y = y;
    WriteToClient(client, sizeof(xXDGAChangePixmapModeReply), (char *)&rep);

    return (client->noClientException);
}


static int
ProcXDGACreateColormap(ClientPtr client)
{
    REQUEST(xXDGACreateColormapReq);
    int result;

    if (stuff->screen > screenInfo.numScreens)
        return BadValue;

    if(DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXDGACreateColormapReq);

    if(!stuff->mode)
	return BadValue;

    result = DGACreateColormap(stuff->screen, client, stuff->id,
				stuff->mode, stuff->alloc);
    if(result != Success)
	return result;

    return (client->noClientException);
}

/*
 *
 * Support for the old DGA protocol, used to live in xf86dga.c
 *
 */

#ifdef DGA_PROTOCOL_OLD_SUPPORT

static DISPATCH_PROC(ProcXF86DGADirectVideo);
static DISPATCH_PROC(ProcXF86DGAGetVidPage);
static DISPATCH_PROC(ProcXF86DGAGetVideoLL);
static DISPATCH_PROC(ProcXF86DGAGetViewPortSize);
static DISPATCH_PROC(ProcXF86DGASetVidPage);
static DISPATCH_PROC(ProcXF86DGASetViewPort);
static DISPATCH_PROC(ProcXF86DGAInstallColormap);
static DISPATCH_PROC(ProcXF86DGAQueryDirectVideo);
static DISPATCH_PROC(ProcXF86DGAViewPortChanged);


static int
ProcXF86DGAGetVideoLL(ClientPtr client)
{
    REQUEST(xXF86DGAGetVideoLLReq);
    xXF86DGAGetVideoLLReply rep;
    XDGAModeRec mode;
    int num, offset, flags;
    char *name;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAGetVideoLLReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    if(!DGAAvailable(stuff->screen))
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    if(!(num = DGAGetOldDGAMode(stuff->screen)))
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    /* get the parameters for the mode that best matches */
    DGAGetModeInfo(stuff->screen, &mode, num);

    if(!DGAOpenFramebuffer(stuff->screen, &name,
			(unsigned char**)(&rep.offset),
			(int*)(&rep.bank_size), &offset, &flags))
	return BadAlloc;

    rep.offset += mode.offset;
    rep.width = mode.bytesPerScanline / (mode.bitsPerPixel >> 3);
    rep.ram_size = rep.bank_size >> 10;

    WriteToClient(client, SIZEOF(xXF86DGAGetVideoLLReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGADirectVideo(ClientPtr client)
{
    int num;
    PixmapPtr pix;
    XDGAModeRec mode;
    REQUEST(xXF86DGADirectVideoReq);

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGADirectVideoReq);

    if (!DGAAvailable(stuff->screen))
	return DGAErrorBase + XF86DGANoDirectVideoMode;

    if (DGAClients[stuff->screen] &&
        (DGAClients[stuff->screen] != client))
        return DGAErrorBase + XF86DGANoDirectVideoMode;

    if (stuff->enable & XF86DGADirectGraphics) {
	if(!(num = DGAGetOldDGAMode(stuff->screen)))
	    return (DGAErrorBase + XF86DGANoDirectVideoMode);
    } else
	num = 0;

    if(Success != DGASetMode(stuff->screen, num, &mode, &pix))
	return (DGAErrorBase + XF86DGAScreenNotActive);

    DGASetInputMode (stuff->screen,
		     (stuff->enable & XF86DGADirectKeyb) != 0,
		     (stuff->enable & XF86DGADirectMouse) != 0);

    /* We need to track the client and attach the teardown callback */
    if (stuff->enable &
	(XF86DGADirectGraphics | XF86DGADirectKeyb | XF86DGADirectMouse)) {
	if (!DGAClients[stuff->screen]) {
	    if (DGACallbackRefCount++ == 0)
		AddCallback (&ClientStateCallback, DGAClientStateChange, NULL);
	}

	DGAClients[stuff->screen] = client;
    } else {
	if (DGAClients[stuff->screen]) {
	    if (--DGACallbackRefCount == 0)
		DeleteCallback(&ClientStateCallback, DGAClientStateChange, NULL);
	}

	DGAClients[stuff->screen] = NULL;
    }

    return (client->noClientException);
}

static int
ProcXF86DGAGetViewPortSize(ClientPtr client)
{
    int num;
    XDGAModeRec mode;
    REQUEST(xXF86DGAGetViewPortSizeReq);
    xXF86DGAGetViewPortSizeReply rep;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAGetViewPortSizeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    if (!DGAAvailable(stuff->screen))
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    if(!(num = DGAGetOldDGAMode(stuff->screen)))
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    DGAGetModeInfo(stuff->screen, &mode, num);

    rep.width = mode.viewportWidth;
    rep.height = mode.viewportHeight;

    WriteToClient(client, SIZEOF(xXF86DGAGetViewPortSizeReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGASetViewPort(ClientPtr client)
{
    REQUEST(xXF86DGASetViewPortReq);

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    if (DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXF86DGASetViewPortReq);

    if (!DGAAvailable(stuff->screen))
	return (DGAErrorBase + XF86DGANoDirectVideoMode);

    if (!DGAActive(stuff->screen))
	return DGAErrorBase + XF86DGADirectNotActivated;

    if (DGASetViewport(stuff->screen, stuff->x, stuff->y, DGA_FLIP_RETRACE)
		!= Success)
	return DGAErrorBase + XF86DGADirectNotActivated;

    return (client->noClientException);
}

static int
ProcXF86DGAGetVidPage(ClientPtr client)
{
    REQUEST(xXF86DGAGetVidPageReq);
    xXF86DGAGetVidPageReply rep;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAGetVidPageReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.vpage = 0;  /* silently fail */

    WriteToClient(client, SIZEOF(xXF86DGAGetVidPageReply), (char *)&rep);
    return (client->noClientException);
}


static int
ProcXF86DGASetVidPage(ClientPtr client)
{
    REQUEST(xXF86DGASetVidPageReq);

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGASetVidPageReq);

    /* silently fail */

    return (client->noClientException);
}


static int
ProcXF86DGAInstallColormap(ClientPtr client)
{
    ColormapPtr pcmp;
    REQUEST(xXF86DGAInstallColormapReq);

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    if (DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXF86DGAInstallColormapReq);

    if (!DGAActive(stuff->screen))
	return (DGAErrorBase + XF86DGADirectNotActivated);

    pcmp = (ColormapPtr  )LookupIDByType(stuff->id, RT_COLORMAP);
    if (pcmp) {
	DGAInstallCmap(pcmp);
        return (client->noClientException);
    } else {
        client->errorValue = stuff->id;
        return (BadColor);
    }
}

static int
ProcXF86DGAQueryDirectVideo(ClientPtr client)
{
    REQUEST(xXF86DGAQueryDirectVideoReq);
    xXF86DGAQueryDirectVideoReply rep;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    REQUEST_SIZE_MATCH(xXF86DGAQueryDirectVideoReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.flags = 0;

    if (DGAAvailable(stuff->screen))
	rep.flags = XF86DGADirectPresent;

    WriteToClient(client, SIZEOF(xXF86DGAQueryDirectVideoReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXF86DGAViewPortChanged(ClientPtr client)
{
    REQUEST(xXF86DGAViewPortChangedReq);
    xXF86DGAViewPortChangedReply rep;

    if (stuff->screen > screenInfo.numScreens)
	return BadValue;

    if (DGAClients[stuff->screen] != client)
        return DGAErrorBase + XF86DGADirectNotActivated;

    REQUEST_SIZE_MATCH(xXF86DGAViewPortChangedReq);

    if (!DGAActive(stuff->screen))
	return (DGAErrorBase + XF86DGADirectNotActivated);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.result = 1;

    WriteToClient(client, SIZEOF(xXF86DGAViewPortChangedReply), (char *)&rep);
    return (client->noClientException);
}

#endif /* DGA_PROTOCOL_OLD_SUPPORT */

static int
SProcXDGADispatch (ClientPtr client)
{
   return DGAErrorBase + XF86DGAClientNotLocal;
}

#if 0
#define DGA_REQ_DEBUG
#endif

#ifdef DGA_REQ_DEBUG
static char *dgaMinor[] = {
    "QueryVersion",
    "GetVideoLL",
    "DirectVideo",
    "GetViewPortSize",
    "SetViewPort",
    "GetVidPage",
    "SetVidPage",
    "InstallColormap",
    "QueryDirectVideo",
    "ViewPortChanged",
    "10",
    "11",
    "QueryModes",
    "SetMode",
    "SetViewport",
    "InstallColormap",
    "SelectInput",
    "FillRectangle",
    "CopyArea",
    "CopyTransparentArea",
    "GetViewportStatus",
    "Sync",
    "OpenFramebuffer",
    "CloseFramebuffer",
    "SetClientVersion",
    "ChangePixmapMode",
    "CreateColormap",
};
#endif

static int
ProcXDGADispatch (ClientPtr client)
{
    REQUEST(xReq);

    if (!LocalClient(client))
	return DGAErrorBase + XF86DGAClientNotLocal;

#ifdef DGA_REQ_DEBUG
    if (stuff->data <= X_XDGACreateColormap)
	fprintf (stderr, "    DGA %s\n", dgaMinor[stuff->data]);
#endif

    switch (stuff->data){
    /*
     * DGA2 Protocol
     */
    case X_XDGAQueryVersion:
	return ProcXDGAQueryVersion(client);
    case X_XDGAQueryModes:
	return ProcXDGAQueryModes(client);
    case X_XDGASetMode:
	return ProcXDGASetMode(client);
    case X_XDGAOpenFramebuffer:
	return ProcXDGAOpenFramebuffer(client);
    case X_XDGACloseFramebuffer:
	return ProcXDGACloseFramebuffer(client);
    case X_XDGASetViewport:
	return ProcXDGASetViewport(client);
    case X_XDGAInstallColormap:
	return ProcXDGAInstallColormap(client);
    case X_XDGASelectInput:
	return ProcXDGASelectInput(client);
    case X_XDGAFillRectangle:
	return ProcXDGAFillRectangle(client);
    case X_XDGACopyArea:
	return ProcXDGACopyArea(client);
    case X_XDGACopyTransparentArea:
	return ProcXDGACopyTransparentArea(client);
    case X_XDGAGetViewportStatus:
	return ProcXDGAGetViewportStatus(client);
    case X_XDGASync:
	return ProcXDGASync(client);
    case X_XDGASetClientVersion:
	return ProcXDGASetClientVersion(client);
    case X_XDGAChangePixmapMode:
	return ProcXDGAChangePixmapMode(client);
    case X_XDGACreateColormap:
	return ProcXDGACreateColormap(client);
    /*
     * Old DGA Protocol
     */
#ifdef DGA_PROTOCOL_OLD_SUPPORT
    case X_XF86DGAGetVideoLL:
	return ProcXF86DGAGetVideoLL(client);
    case X_XF86DGADirectVideo:
	return ProcXF86DGADirectVideo(client);
    case X_XF86DGAGetViewPortSize:
	return ProcXF86DGAGetViewPortSize(client);
    case X_XF86DGASetViewPort:
	return ProcXF86DGASetViewPort(client);
    case X_XF86DGAGetVidPage:
	return ProcXF86DGAGetVidPage(client);
    case X_XF86DGASetVidPage:
	return ProcXF86DGASetVidPage(client);
    case X_XF86DGAInstallColormap:
	return ProcXF86DGAInstallColormap(client);
    case X_XF86DGAQueryDirectVideo:
	return ProcXF86DGAQueryDirectVideo(client);
    case X_XF86DGAViewPortChanged:
	return ProcXF86DGAViewPortChanged(client);
#endif /* DGA_PROTOCOL_OLD_SUPPORT */
    default:
	return BadRequest;
    }
}

void
XFree86DGARegister(INITARGS)
{
  XDGAEventBase = &DGAEventBase;
}
