/************************************************************

Author: Eamon Walsh <ewalsh@tycho.nsa.gov>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
this permission notice appear in supporting documentation.  This permission
notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

********************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdarg.h>
#include "scrnintstr.h"
#include "extnsionst.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "xacestr.h"

#define XSERV_t
#define TRANS_SERVER
#include <X11/Xtrans/Xtrans.h>
#include "../os/osdep.h"

_X_EXPORT CallbackListPtr XaceHooks[XACE_NUM_HOOKS] = {0};

/* Special-cased hook functions.  Called by Xserver.
 */
int XaceHookDispatch(ClientPtr client, int major)
{
    /* Call the audit begin callback, there is no return value. */
    XaceAuditRec rec = { client, 0 };
    CallCallbacks(&XaceHooks[XACE_AUDIT_BEGIN], &rec);

    if (major < 128) {
	/* Call the core dispatch hook */
	XaceCoreDispatchRec rec = { client, Success /* default allow */ };
	CallCallbacks(&XaceHooks[XACE_CORE_DISPATCH], &rec);
	return rec.status;
    } else {
	/* Call the extension dispatch hook */
	ExtensionEntry *ext = GetExtensionEntry(major);
	XaceExtAccessRec rec = { client, ext, DixUseAccess, Success };
	if (ext)
	    CallCallbacks(&XaceHooks[XACE_EXT_DISPATCH], &rec);
	/* On error, pretend extension doesn't exist */
	return (rec.status == Success) ? Success : BadRequest;
    }
}

int XaceHookPropertyAccess(ClientPtr client, WindowPtr pWin,
			   PropertyPtr *ppProp, Mask access_mode)
{
    XacePropertyAccessRec rec = { client, pWin, ppProp, access_mode, Success };
    CallCallbacks(&XaceHooks[XACE_PROPERTY_ACCESS], &rec);
    return rec.status;
}

int XaceHookSelectionAccess(ClientPtr client,
			    Selection **ppSel, Mask access_mode)
{
    XaceSelectionAccessRec rec = { client, ppSel, access_mode, Success };
    CallCallbacks(&XaceHooks[XACE_SELECTION_ACCESS], &rec);
    return rec.status;
}

void XaceHookAuditEnd(ClientPtr ptr, int result)
{
    XaceAuditRec rec = { ptr, result };
    /* call callbacks, there is no return value. */
    CallCallbacks(&XaceHooks[XACE_AUDIT_END], &rec);
}

/* Entry point for hook functions.  Called by Xserver.
 */
int XaceHook(int hook, ...)
{
    pointer calldata;	/* data passed to callback */
    int *prv = NULL;	/* points to return value from callback */
    va_list ap;		/* argument list */
    va_start(ap, hook);

    /* Marshal arguments for passing to callback.
     * Each callback has its own case, which sets up a structure to hold
     * the arguments and integer return parameter, or in some cases just
     * sets calldata directly to a single argument (with no return result)
     */
    switch (hook)
    {
	case XACE_RESOURCE_ACCESS: {
	    XaceResourceAccessRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.id = va_arg(ap, XID);
	    rec.rtype = va_arg(ap, RESTYPE);
	    rec.res = va_arg(ap, pointer);
	    rec.ptype = va_arg(ap, RESTYPE);
	    rec.parent = va_arg(ap, pointer);
	    rec.access_mode = va_arg(ap, Mask);
	    rec.status = Success; /* default allow */
	    calldata = &rec;
	    prv = &rec.status;
	    break;
	}
	case XACE_DEVICE_ACCESS: {
	    XaceDeviceAccessRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.dev = va_arg(ap, DeviceIntPtr);
	    rec.access_mode = va_arg(ap, Mask);
	    rec.status = Success; /* default allow */
	    calldata = &rec;
	    prv = &rec.status;
	    break;
	}
	case XACE_SEND_ACCESS: {
	    XaceSendAccessRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.dev = va_arg(ap, DeviceIntPtr);
	    rec.pWin = va_arg(ap, WindowPtr);
	    rec.events = va_arg(ap, xEventPtr);
	    rec.count = va_arg(ap, int);
	    rec.status = Success; /* default allow */
	    calldata = &rec;
	    prv = &rec.status;
	    break;
	}
	case XACE_RECEIVE_ACCESS: {
	    XaceReceiveAccessRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.pWin = va_arg(ap, WindowPtr);
	    rec.events = va_arg(ap, xEventPtr);
	    rec.count = va_arg(ap, int);
	    rec.status = Success; /* default allow */
	    calldata = &rec;
	    prv = &rec.status;
	    break;
	}
	case XACE_CLIENT_ACCESS: {
	    XaceClientAccessRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.target = va_arg(ap, ClientPtr);
	    rec.access_mode = va_arg(ap, Mask);
	    rec.status = Success; /* default allow */
	    calldata = &rec;
	    prv = &rec.status;
	    break;
	}
	case XACE_EXT_ACCESS: {
	    XaceExtAccessRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.ext = va_arg(ap, ExtensionEntry*);
	    rec.access_mode = DixGetAttrAccess;
	    rec.status = Success; /* default allow */
	    calldata = &rec;
	    prv = &rec.status;
	    break;
	}
	case XACE_SERVER_ACCESS: {
	    XaceServerAccessRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.access_mode = va_arg(ap, Mask);
	    rec.status = Success; /* default allow */
	    calldata = &rec;
	    prv = &rec.status;
	    break;
	}
	case XACE_SCREEN_ACCESS:
	case XACE_SCREENSAVER_ACCESS: {
	    XaceScreenAccessRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.screen = va_arg(ap, ScreenPtr);
	    rec.access_mode = va_arg(ap, Mask);
	    rec.status = Success; /* default allow */
	    calldata = &rec;
	    prv = &rec.status;
	    break;
	}
	case XACE_AUTH_AVAIL: {
	    XaceAuthAvailRec rec;
	    rec.client = va_arg(ap, ClientPtr);
	    rec.authId = va_arg(ap, XID);
	    calldata = &rec;
	    break;
	}
	case XACE_KEY_AVAIL: {
	    XaceKeyAvailRec rec;
	    rec.event = va_arg(ap, xEventPtr);
	    rec.keybd = va_arg(ap, DeviceIntPtr);
	    rec.count = va_arg(ap, int);
	    calldata = &rec;
	    break;
	}
	default: {
	    va_end(ap);
	    return 0;	/* unimplemented hook number */
	}
    }
    va_end(ap);
 
    /* call callbacks and return result, if any. */
    CallCallbacks(&XaceHooks[hook], calldata);
    return prv ? *prv : Success;
}

/* XaceCensorImage
 *
 * Called after pScreen->GetImage to prevent pieces or trusted windows from
 * being returned in image data from an untrusted window.
 *
 * Arguments:
 *	client is the client doing the GetImage.
 *      pVisibleRegion is the visible region of the window.
 *	widthBytesLine is the width in bytes of one horizontal line in pBuf.
 *	pDraw is the source window.
 *	x, y, w, h is the rectangle of image data from pDraw in pBuf.
 *	format is the format of the image data in pBuf: ZPixmap or XYPixmap.
 *	pBuf is the image data.
 *
 * Returns: nothing.
 *
 * Side Effects:
 *	Any part of the rectangle (x, y, w, h) that is outside the visible
 *	region of the window will be destroyed (overwritten) in pBuf.
 */
void
XaceCensorImage(
	ClientPtr client,
	RegionPtr pVisibleRegion,
	long widthBytesLine,
	DrawablePtr pDraw,
	int x, int y, int w, int h,
	unsigned int format,
	char *pBuf)
{
    ScreenPtr pScreen;
    RegionRec imageRegion;  /* region representing x,y,w,h */
    RegionRec censorRegion; /* region to obliterate */
    BoxRec imageBox;
    int nRects;

    pScreen = pDraw->pScreen;

    imageBox.x1 = x;
    imageBox.y1 = y;
    imageBox.x2 = x + w;
    imageBox.y2 = y + h;
    REGION_INIT(pScreen, &imageRegion, &imageBox, 1);
    REGION_NULL(pScreen, &censorRegion);

    /* censorRegion = imageRegion - visibleRegion */
    REGION_SUBTRACT(pScreen, &censorRegion, &imageRegion, pVisibleRegion);
    nRects = REGION_NUM_RECTS(&censorRegion);
    if (nRects > 0)
    { /* we have something to censor */
	GCPtr pScratchGC = NULL;
	PixmapPtr pPix = NULL;
	xRectangle *pRects = NULL;
	Bool failed = FALSE;
	int depth = 1;
	int bitsPerPixel = 1;
	int i;
	BoxPtr pBox;

	/* convert region to list-of-rectangles for PolyFillRect */

	pRects = xalloc(nRects * sizeof(xRectangle));
	if (!pRects)
	{
	    failed = TRUE;
	    goto failSafe;
	}
	for (pBox = REGION_RECTS(&censorRegion), i = 0;
	     i < nRects;
	     i++, pBox++)
	{
	    pRects[i].x = pBox->x1;
	    pRects[i].y = pBox->y1 - imageBox.y1;
	    pRects[i].width  = pBox->x2 - pBox->x1;
	    pRects[i].height = pBox->y2 - pBox->y1;
	}

	/* use pBuf as a fake pixmap */

	if (format == ZPixmap)
	{
	    depth = pDraw->depth;
	    bitsPerPixel = pDraw->bitsPerPixel;
	}

	pPix = GetScratchPixmapHeader(pDraw->pScreen, w, h,
		    depth, bitsPerPixel,
		    widthBytesLine, (pointer)pBuf);
	if (!pPix)
	{
	    failed = TRUE;
	    goto failSafe;
	}

	pScratchGC = GetScratchGC(depth, pPix->drawable.pScreen);
	if (!pScratchGC)
	{
	    failed = TRUE;
	    goto failSafe;
	}

	ValidateGC(&pPix->drawable, pScratchGC);
	(* pScratchGC->ops->PolyFillRect)(&pPix->drawable,
			    pScratchGC, nRects, pRects);

    failSafe:
	if (failed)
	{
	    /* Censoring was not completed above.  To be safe, wipe out
	     * all the image data so that nothing trusted gets out.
	     */
	    bzero(pBuf, (int)(widthBytesLine * h));
	}
	if (pRects)     xfree(pRects);
	if (pScratchGC) FreeScratchGC(pScratchGC);
	if (pPix)       FreeScratchPixmapHeader(pPix);
    }
    REGION_UNINIT(pScreen, &imageRegion);
    REGION_UNINIT(pScreen, &censorRegion);
} /* XaceCensorImage */

/*
 * Xtrans wrappers for use by modules
 */
int XaceGetConnectionNumber(ClientPtr client)
{
    XtransConnInfo ci = ((OsCommPtr)client->osPrivate)->trans_conn;
    return _XSERVTransGetConnectionNumber(ci);
}

int XaceIsLocal(ClientPtr client)
{
    XtransConnInfo ci = ((OsCommPtr)client->osPrivate)->trans_conn;
    return _XSERVTransIsLocal(ci);
}
