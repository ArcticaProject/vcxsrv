/*
 * Copyright © 2007, 2008 Red Hat, Inc.
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

#include <errno.h>
#include <xf86drm.h>
#include "xf86Module.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "dixstruct.h"
#include "dri2.h"
#include "xf86VGAarbiter.h"

#include "xf86.h"

static int dri2ScreenPrivateKeyIndex;
static DevPrivateKey dri2ScreenPrivateKey = &dri2ScreenPrivateKeyIndex;
static int dri2WindowPrivateKeyIndex;
static DevPrivateKey dri2WindowPrivateKey = &dri2WindowPrivateKeyIndex;
static int dri2PixmapPrivateKeyIndex;
static DevPrivateKey dri2PixmapPrivateKey = &dri2PixmapPrivateKeyIndex;

typedef struct _DRI2Drawable {
    unsigned int	 refCount;
    int			 width;
    int			 height;
    DRI2BufferPtr	*buffers;
    int			 bufferCount;
    unsigned int	 swapsPending;
    ClientPtr		 blockedClient;
    int			 swap_interval;
    CARD64		 swap_count;
    CARD64		 target_sbc; /* -1 means no SBC wait outstanding */
    CARD64		 last_swap_target; /* most recently queued swap target */
    int			 swap_limit; /* for N-buffering */
} DRI2DrawableRec, *DRI2DrawablePtr;

typedef struct _DRI2Screen *DRI2ScreenPtr;

typedef struct _DRI2Screen {
    unsigned int		 numDrivers;
    const char			**driverNames;
    const char			*deviceName;
    int				 fd;
    unsigned int		 lastSequence;

    DRI2CreateBufferProcPtr	 CreateBuffer;
    DRI2DestroyBufferProcPtr	 DestroyBuffer;
    DRI2CopyRegionProcPtr	 CopyRegion;
    DRI2ScheduleSwapProcPtr	 ScheduleSwap;
    DRI2GetMSCProcPtr		 GetMSC;
    DRI2ScheduleWaitMSCProcPtr	 ScheduleWaitMSC;

    HandleExposuresProcPtr       HandleExposures;
} DRI2ScreenRec;

static DRI2ScreenPtr
DRI2GetScreen(ScreenPtr pScreen)
{
    return dixLookupPrivate(&pScreen->devPrivates, dri2ScreenPrivateKey);
}

static DRI2DrawablePtr
DRI2GetDrawable(DrawablePtr pDraw)
{
    WindowPtr		  pWin;
    PixmapPtr		  pPixmap;

    if (!pDraw)
	return NULL;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr) pDraw;
	return dixLookupPrivate(&pWin->devPrivates, dri2WindowPrivateKey);
    }
    else
    {
	pPixmap = (PixmapPtr) pDraw;
	return dixLookupPrivate(&pPixmap->devPrivates, dri2PixmapPrivateKey);
    }
}

int
DRI2CreateDrawable(DrawablePtr pDraw)
{
    WindowPtr	    pWin;
    PixmapPtr	    pPixmap;
    DRI2DrawablePtr pPriv;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv != NULL)
    {
	pPriv->refCount++;
	return Success;
    }

    pPriv = xalloc(sizeof *pPriv);
    if (pPriv == NULL)
	return BadAlloc;

    pPriv->refCount = 1;
    pPriv->width = pDraw->width;
    pPriv->height = pDraw->height;
    pPriv->buffers = NULL;
    pPriv->bufferCount = 0;
    pPriv->swapsPending = 0;
    pPriv->blockedClient = NULL;
    pPriv->swap_count = 0;
    pPriv->target_sbc = -1;
    pPriv->swap_interval = 1;
    pPriv->last_swap_target = -1;
    pPriv->swap_limit = 1; /* default to double buffering */

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr) pDraw;
	dixSetPrivate(&pWin->devPrivates, dri2WindowPrivateKey, pPriv);
    }
    else
    {
	pPixmap = (PixmapPtr) pDraw;
	dixSetPrivate(&pPixmap->devPrivates, dri2PixmapPrivateKey, pPriv);
    }

    return Success;
}

static void
DRI2FreeDrawable(DrawablePtr pDraw)
{
    DRI2DrawablePtr pPriv;
    WindowPtr  	    pWin;
    PixmapPtr	    pPixmap;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL)
	return;

    xfree(pPriv);

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr) pDraw;
	dixSetPrivate(&pWin->devPrivates, dri2WindowPrivateKey, NULL);
    }
    else
    {
	pPixmap = (PixmapPtr) pDraw;
	dixSetPrivate(&pPixmap->devPrivates, dri2PixmapPrivateKey, NULL);
    }
}

static int
find_attachment(DRI2DrawablePtr pPriv, unsigned attachment)
{
    int i;

    if (pPriv->buffers == NULL) {
	return -1;
    }

    for (i = 0; i < pPriv->bufferCount; i++) {
	if ((pPriv->buffers[i] != NULL)
	    && (pPriv->buffers[i]->attachment == attachment)) {
	    return i;
	}
    }

    return -1;
}

static Bool
allocate_or_reuse_buffer(DrawablePtr pDraw, DRI2ScreenPtr ds,
			 DRI2DrawablePtr pPriv,
			 unsigned int attachment, unsigned int format,
			 int dimensions_match, DRI2BufferPtr *buffer)
{
    int old_buf = find_attachment(pPriv, attachment);

    if ((old_buf < 0)
	|| !dimensions_match
	|| (pPriv->buffers[old_buf]->format != format)) {
	*buffer = (*ds->CreateBuffer)(pDraw, attachment, format);
	return TRUE;

    } else {
	*buffer = pPriv->buffers[old_buf];
	pPriv->buffers[old_buf] = NULL;
	return FALSE;
    }
}

static DRI2BufferPtr *
do_get_buffers(DrawablePtr pDraw, int *width, int *height,
	       unsigned int *attachments, int count, int *out_count,
	       int has_format)
{
    DRI2ScreenPtr   ds = DRI2GetScreen(pDraw->pScreen);
    DRI2DrawablePtr pPriv = DRI2GetDrawable(pDraw);
    DRI2BufferPtr  *buffers;
    int need_real_front = 0;
    int need_fake_front = 0;
    int have_fake_front = 0;
    int front_format = 0;
    int dimensions_match;
    int buffers_changed = 0;
    int i;

    if (!pPriv) {
	*width = pDraw->width;
	*height = pDraw->height;
	*out_count = 0;
	return NULL;
    }

    dimensions_match = (pDraw->width == pPriv->width)
	&& (pDraw->height == pPriv->height);

    buffers = xalloc((count + 1) * sizeof(buffers[0]));

    for (i = 0; i < count; i++) {
	const unsigned attachment = *(attachments++);
	const unsigned format = (has_format) ? *(attachments++) : 0;

	if (allocate_or_reuse_buffer(pDraw, ds, pPriv, attachment,
				     format, dimensions_match,
				     &buffers[i]))
		buffers_changed = 1;

	/* If the drawable is a window and the front-buffer is requested,
	 * silently add the fake front-buffer to the list of requested
	 * attachments.  The counting logic in the loop accounts for the case
	 * where the client requests both the fake and real front-buffer.
	 */
	if (attachment == DRI2BufferBackLeft) {
	    need_real_front++;
	    front_format = format;
	}

	if (attachment == DRI2BufferFrontLeft) {
	    need_real_front--;
	    front_format = format;

	    if (pDraw->type == DRAWABLE_WINDOW) {
		need_fake_front++;
	    }
	}

	if (pDraw->type == DRAWABLE_WINDOW) {
	    if (attachment == DRI2BufferFakeFrontLeft) {
		need_fake_front--;
		have_fake_front = 1;
	    }
	}
    }

    if (need_real_front > 0) {
	if (allocate_or_reuse_buffer(pDraw, ds, pPriv, DRI2BufferFrontLeft,
				     front_format, dimensions_match,
				     &buffers[i++]))
	    buffers_changed = 1;
    }

    if (need_fake_front > 0) {
	if (allocate_or_reuse_buffer(pDraw, ds, pPriv, DRI2BufferFakeFrontLeft,
				     front_format, dimensions_match,
				     &buffers[i++]))
	    buffers_changed = 1;

	have_fake_front = 1;
    }

    *out_count = i;


    if (pPriv->buffers != NULL) {
	for (i = 0; i < pPriv->bufferCount; i++) {
	    if (pPriv->buffers[i] != NULL) {
		(*ds->DestroyBuffer)(pDraw, pPriv->buffers[i]);
	    }
	}

	xfree(pPriv->buffers);
    }

    pPriv->buffers = buffers;
    pPriv->bufferCount = *out_count;
    pPriv->width = pDraw->width;
    pPriv->height = pDraw->height;
    *width = pPriv->width;
    *height = pPriv->height;


    /* If the client is getting a fake front-buffer, pre-fill it with the
     * contents of the real front-buffer.  This ensures correct operation of
     * applications that call glXWaitX before calling glDrawBuffer.
     */
    if (have_fake_front && buffers_changed) {
	BoxRec box;
	RegionRec region;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pPriv->width;
	box.y2 = pPriv->height;
	REGION_INIT(pDraw->pScreen, &region, &box, 0);

	DRI2CopyRegion(pDraw, &region, DRI2BufferFakeFrontLeft,
		       DRI2BufferFrontLeft);
    }

    return pPriv->buffers;
}

DRI2BufferPtr *
DRI2GetBuffers(DrawablePtr pDraw, int *width, int *height,
	       unsigned int *attachments, int count, int *out_count)
{
    return do_get_buffers(pDraw, width, height, attachments, count,
			  out_count, FALSE);
}

DRI2BufferPtr *
DRI2GetBuffersWithFormat(DrawablePtr pDraw, int *width, int *height,
			 unsigned int *attachments, int count, int *out_count)
{
    return do_get_buffers(pDraw, width, height, attachments, count,
			  out_count, TRUE);
}

/*
 * In the direct rendered case, we throttle the clients that have more
 * than their share of outstanding swaps (and thus busy buffers) when a
 * new GetBuffers request is received.  In the AIGLX case, we allow the
 * client to get the new buffers, but throttle when the next GLX request
 * comes in (see __glXDRIcontextWait()).
 */
Bool
DRI2ThrottleClient(ClientPtr client, DrawablePtr pDraw)
{
    DRI2DrawablePtr pPriv;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL)
	return FALSE;

    /* Throttle to swap limit */
    if ((pPriv->swapsPending >= pPriv->swap_limit) &&
	!pPriv->blockedClient) {
	ResetCurrentRequest(client);
	client->sequence--;
	IgnoreClient(client);
	pPriv->blockedClient = client;
	return TRUE;
    }

    return FALSE;
}

void
DRI2BlockClient(ClientPtr client, DrawablePtr pDraw)
{
    DRI2DrawablePtr pPriv;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL)
	return;

    if (pPriv->blockedClient == NULL) {
	IgnoreClient(client);
	pPriv->blockedClient = client;
    }
}

int
DRI2CopyRegion(DrawablePtr pDraw, RegionPtr pRegion,
	       unsigned int dest, unsigned int src)
{
    DRI2ScreenPtr   ds = DRI2GetScreen(pDraw->pScreen);
    DRI2DrawablePtr pPriv;
    DRI2BufferPtr   pDestBuffer, pSrcBuffer;
    int		    i;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL)
	return BadDrawable;

    pDestBuffer = NULL;
    pSrcBuffer = NULL;
    for (i = 0; i < pPriv->bufferCount; i++)
    {
	if (pPriv->buffers[i]->attachment == dest)
	    pDestBuffer = (DRI2BufferPtr) pPriv->buffers[i];
	if (pPriv->buffers[i]->attachment == src)
	    pSrcBuffer = (DRI2BufferPtr) pPriv->buffers[i];
    }
    if (pSrcBuffer == NULL || pDestBuffer == NULL)
	return BadValue;

    (*ds->CopyRegion)(pDraw, pRegion, pDestBuffer, pSrcBuffer);

    return Success;
}

/* Can this drawable be page flipped? */
Bool
DRI2CanFlip(DrawablePtr pDraw)
{
    ScreenPtr pScreen = pDraw->pScreen;
    WindowPtr pWin, pRoot;
    PixmapPtr pWinPixmap, pRootPixmap;

    if (pDraw->type == DRAWABLE_PIXMAP)
	return TRUE;

    pRoot = WindowTable[pScreen->myNum];
    pRootPixmap = pScreen->GetWindowPixmap(pRoot);

    pWin = (WindowPtr) pDraw;
    pWinPixmap = pScreen->GetWindowPixmap(pWin);
    if (pRootPixmap != pWinPixmap)
	return FALSE;
    if (!REGION_EQUAL(pScreen, &pWin->clipList, &pRoot->winSize))
	return FALSE;

    return TRUE;
}

/* Can we do a pixmap exchange instead of a blit? */
Bool
DRI2CanExchange(DrawablePtr pDraw)
{
    return FALSE;
}

void
DRI2WaitMSCComplete(ClientPtr client, DrawablePtr pDraw, int frame,
		    unsigned int tv_sec, unsigned int tv_usec)
{
    DRI2DrawablePtr pPriv;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL)
	return;

    ProcDRI2WaitMSCReply(client, ((CARD64)tv_sec * 1000000) + tv_usec,
			 frame, pPriv->swap_count);

    if (pPriv->blockedClient)
	AttendClient(pPriv->blockedClient);

    pPriv->blockedClient = NULL;
}

static void
DRI2WakeClient(ClientPtr client, DrawablePtr pDraw, int frame,
	       unsigned int tv_sec, unsigned int tv_usec)
{
    ScreenPtr	    pScreen = pDraw->pScreen;
    DRI2DrawablePtr pPriv;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL) {
        xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[DRI2] %s: bad drawable\n", __func__);
	return;
    }

    /*
     * Swap completed.  Either wake up an SBC waiter or a client that was
     * blocked due to GLX activity during a swap.
     */
    if (pPriv->target_sbc != -1 &&
	pPriv->target_sbc >= pPriv->swap_count) {
	ProcDRI2WaitMSCReply(client, ((CARD64)tv_sec * 1000000) + tv_usec,
			     frame, pPriv->swap_count);
	pPriv->target_sbc = -1;

	AttendClient(pPriv->blockedClient);
	pPriv->blockedClient = NULL;
    } else if (pPriv->target_sbc == -1) {
	if (pPriv->blockedClient)
	    AttendClient(pPriv->blockedClient);
	pPriv->blockedClient = NULL;
    }
}

void
DRI2SwapComplete(ClientPtr client, DrawablePtr pDraw, int frame,
		   unsigned int tv_sec, unsigned int tv_usec, int type,
		   DRI2SwapEventPtr swap_complete, void *swap_data)
{
    ScreenPtr	    pScreen = pDraw->pScreen;
    DRI2DrawablePtr pPriv;
    CARD64          ust = 0;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL) {
        xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[DRI2] %s: bad drawable\n", __func__);
	return;
    }

    if (pPriv->refCount == 0) {
        xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[DRI2] %s: bad drawable refcount\n", __func__);
	DRI2FreeDrawable(pDraw);
	return;
    }

    ust = ((CARD64)tv_sec * 1000000) + tv_usec;
    if (swap_complete)
	swap_complete(client, swap_data, type, ust, frame, pPriv->swap_count);

    pPriv->swapsPending--;
    pPriv->swap_count++;

    DRI2WakeClient(client, pDraw, frame, tv_sec, tv_usec);
}

Bool
DRI2WaitSwap(ClientPtr client, DrawablePtr pDrawable)
{
    DRI2DrawablePtr pPriv = DRI2GetDrawable(pDrawable);

    /* If we're currently waiting for a swap on this drawable, reset
     * the request and suspend the client.  We only support one
     * blocked client per drawable. */
    if ((pPriv->swapsPending) &&
	pPriv->blockedClient == NULL) {
	ResetCurrentRequest(client);
	client->sequence--;
	DRI2BlockClient(client, pDrawable);
	return TRUE;
    }

    return FALSE;
}

int
DRI2SwapBuffers(ClientPtr client, DrawablePtr pDraw, CARD64 target_msc,
		CARD64 divisor, CARD64 remainder, CARD64 *swap_target,
		DRI2SwapEventPtr func, void *data)
{
    ScreenPtr       pScreen = pDraw->pScreen;
    DRI2ScreenPtr   ds = DRI2GetScreen(pDraw->pScreen);
    DRI2DrawablePtr pPriv;
    DRI2BufferPtr   pDestBuffer = NULL, pSrcBuffer = NULL;
    CARD64          ust;
    int             ret, i;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL) {
        xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[DRI2] %s: bad drawable\n", __func__);
	return BadDrawable;
    }

    for (i = 0; i < pPriv->bufferCount; i++) {
	if (pPriv->buffers[i]->attachment == DRI2BufferFrontLeft)
	    pDestBuffer = (DRI2BufferPtr) pPriv->buffers[i];
	if (pPriv->buffers[i]->attachment == DRI2BufferBackLeft)
	    pSrcBuffer = (DRI2BufferPtr) pPriv->buffers[i];
    }
    if (pSrcBuffer == NULL || pDestBuffer == NULL) {
        xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[DRI2] %s: drawable has no back or front?\n", __func__);
	return BadDrawable;
    }

    /* Old DDX, just blit */
    if (!ds->ScheduleSwap) {
	BoxRec box;
	RegionRec region;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pDraw->width;
	box.y2 = pDraw->height;
	REGION_INIT(pScreen, &region, &box, 0);

	pPriv->swapsPending++;

	(*ds->CopyRegion)(pDraw, &region, pDestBuffer, pSrcBuffer);
	DRI2SwapComplete(client, pDraw, target_msc, 0, 0, DRI2_BLIT_COMPLETE,
			 func, data);
	return Success;
    }

    /*
     * In the simple glXSwapBuffers case, all params will be 0, and we just
     * need to schedule a swap for the last swap target + the swap interval.
     * If the last swap target hasn't been set yet, call into the driver
     * to get the current count.
     */
    if (target_msc == 0 && divisor == 0 && remainder == 0 &&
	pPriv->last_swap_target < 0) {
	ret = (*ds->GetMSC)(pDraw, &ust, &target_msc);
	if (!ret) {
	    xf86DrvMsg(pScreen->myNum, X_ERROR,
		       "[DRI2] %s: driver failed to return current MSC\n",
		       __func__);
	    return BadDrawable;
	}
    }

    /* First swap needs to initialize last_swap_target */
    if (pPriv->last_swap_target < 0)
	pPriv->last_swap_target = target_msc;

    /*
     * Swap target for this swap is last swap target + swap interval since
     * we have to account for the current swap count, interval, and the
     * number of pending swaps.
     */
    *swap_target = pPriv->last_swap_target + pPriv->swap_interval;

    ret = (*ds->ScheduleSwap)(client, pDraw, pDestBuffer, pSrcBuffer,
			      swap_target, divisor, remainder, func, data);
    if (!ret) {
        xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[DRI2] %s: driver failed to schedule swap\n", __func__);
	return BadDrawable;
    }

    pPriv->swapsPending++;
    pPriv->last_swap_target = *swap_target;

    return Success;
}

void
DRI2SwapInterval(DrawablePtr pDrawable, int interval)
{
    DRI2DrawablePtr pPriv = DRI2GetDrawable(pDrawable);

    /* fixme: check against arbitrary max? */

    pPriv->swap_interval = interval;
}

int
DRI2GetMSC(DrawablePtr pDraw, CARD64 *ust, CARD64 *msc, CARD64 *sbc)
{
    ScreenPtr pScreen = pDraw->pScreen;
    DRI2ScreenPtr ds = DRI2GetScreen(pDraw->pScreen);
    DRI2DrawablePtr pPriv;
    Bool ret;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL) {
        xf86DrvMsg(pScreen->myNum, X_ERROR,
		   "[DRI2] %s: bad drawable\n", __func__);
	return BadDrawable;
    }

    if (!ds->GetMSC) {
	*ust = 0;
	*msc = 0;
	*sbc = pPriv->swap_count;
	return Success;
    }

    /*
     * Spec needs to be updated to include unmapped or redirected
     * drawables
     */

    ret = (*ds->GetMSC)(pDraw, ust, msc);
    if (!ret)
	return BadDrawable;

    *sbc = pPriv->swap_count;

    return Success;
}

int
DRI2WaitMSC(ClientPtr client, DrawablePtr pDraw, CARD64 target_msc,
	    CARD64 divisor, CARD64 remainder)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pDraw->pScreen);
    DRI2DrawablePtr pPriv;
    Bool ret;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL)
	return BadDrawable;

    /* Old DDX just completes immediately */
    if (!ds->ScheduleWaitMSC) {
	DRI2WaitMSCComplete(client, pDraw, target_msc, 0, 0);

	return Success;
    }

    ret = (*ds->ScheduleWaitMSC)(client, pDraw, target_msc, divisor, remainder);
    if (!ret)
	return BadDrawable;

    return Success;
}

int
DRI2WaitSBC(ClientPtr client, DrawablePtr pDraw, CARD64 target_sbc,
	    CARD64 *ust, CARD64 *msc, CARD64 *sbc)
{
    DRI2DrawablePtr pPriv;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL)
	return BadDrawable;

    if (pPriv->swap_count >= target_sbc)
	return Success;

    pPriv->target_sbc = target_sbc;
    DRI2BlockClient(client, pDraw);

    return Success;
}

void
DRI2DestroyDrawable(DrawablePtr pDraw)
{
    DRI2ScreenPtr   ds = DRI2GetScreen(pDraw->pScreen);
    DRI2DrawablePtr pPriv;

    pPriv = DRI2GetDrawable(pDraw);
    if (pPriv == NULL)
	return;

    pPriv->refCount--;
    if (pPriv->refCount > 0)
	return;

    if (pPriv->buffers != NULL) {
	int i;

	for (i = 0; i < pPriv->bufferCount; i++)
	    (*ds->DestroyBuffer)(pDraw, pPriv->buffers[i]);

	xfree(pPriv->buffers);
    }

    /* If the window is destroyed while we have a swap pending, don't
     * actually free the priv yet.  We'll need it in the DRI2SwapComplete()
     * callback and we'll free it there once we're done. */
    if (!pPriv->swapsPending)
	DRI2FreeDrawable(pDraw);
}

Bool
DRI2Connect(ScreenPtr pScreen, unsigned int driverType, int *fd,
	    const char **driverName, const char **deviceName)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pScreen);

    if (ds == NULL || driverType >= ds->numDrivers ||
	    !ds->driverNames[driverType])
	return FALSE;

    *fd = ds->fd;
    *driverName = ds->driverNames[driverType];
    *deviceName = ds->deviceName;

    return TRUE;
}

Bool
DRI2Authenticate(ScreenPtr pScreen, drm_magic_t magic)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pScreen);

    if (ds == NULL || drmAuthMagic(ds->fd, magic))
	return FALSE;

    return TRUE;
}

Bool
DRI2ScreenInit(ScreenPtr pScreen, DRI2InfoPtr info)
{
    DRI2ScreenPtr ds;
    const char* driverTypeNames[] = {
	"DRI", /* DRI2DriverDRI */
	"VDPAU", /* DRI2DriverVDPAU */
    };
    unsigned int i;

    if (info->version < 3)
	return FALSE;

    if (!xf86VGAarbiterAllowDRI(pScreen)) {
        xf86DrvMsg(pScreen->myNum, X_WARNING,
                  "[DRI2] Direct rendering is not supported when VGA arb is necessary for the device\n");
        return FALSE;
    }

    ds = xcalloc(1, sizeof *ds);
    if (!ds)
	return FALSE;

    ds->fd	       = info->fd;
    ds->deviceName     = info->deviceName;

    ds->CreateBuffer   = info->CreateBuffer;
    ds->DestroyBuffer  = info->DestroyBuffer;
    ds->CopyRegion     = info->CopyRegion;

    if (info->version >= 4) {
	ds->ScheduleSwap = info->ScheduleSwap;
	ds->ScheduleWaitMSC = info->ScheduleWaitMSC;
	ds->GetMSC = info->GetMSC;
    }

    if (info->version == 3 || info->numDrivers == 0) {
	/* Driver too old: use the old-style driverName field */
	ds->numDrivers = 1;
	ds->driverNames = xalloc(sizeof(*ds->driverNames));
	if (!ds->driverNames) {
	    xfree(ds);
	    return FALSE;
	}
	ds->driverNames[0] = info->driverName;
    } else {
	ds->numDrivers = info->numDrivers;
	ds->driverNames = xalloc(info->numDrivers * sizeof(*ds->driverNames));
	if (!ds->driverNames) {
	    xfree(ds);
	    return FALSE;
	}
	memcpy(ds->driverNames, info->driverNames,
	       info->numDrivers * sizeof(*ds->driverNames));
    }

    dixSetPrivate(&pScreen->devPrivates, dri2ScreenPrivateKey, ds);

    xf86DrvMsg(pScreen->myNum, X_INFO, "[DRI2] Setup complete\n");
    for (i = 0; i < sizeof(driverTypeNames) / sizeof(driverTypeNames[0]); i++) {
	if (i < ds->numDrivers && ds->driverNames[i]) {
	    xf86DrvMsg(pScreen->myNum, X_INFO, "[DRI2]   %s driver: %s\n",
		       driverTypeNames[i], ds->driverNames[i]);
	}
    }

    return TRUE;
}

void
DRI2CloseScreen(ScreenPtr pScreen)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pScreen);

    xfree(ds->driverNames);
    xfree(ds);
    dixSetPrivate(&pScreen->devPrivates, dri2ScreenPrivateKey, NULL);
}

extern ExtensionModule dri2ExtensionModule;

static pointer
DRI2Setup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone)
    {
	setupDone = TRUE;
	LoadExtension(&dri2ExtensionModule, FALSE);
    }
    else
    {
	if (errmaj)
	    *errmaj = LDR_ONCEONLY;
    }

    return (pointer) 1;
}

static XF86ModuleVersionInfo DRI2VersRec =
{
    "dri2",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    1, 2, 0,
    ABI_CLASS_EXTENSION,
    ABI_EXTENSION_VERSION,
    MOD_CLASS_NONE,
    { 0, 0, 0, 0 }
};

_X_EXPORT XF86ModuleData dri2ModuleData = { &DRI2VersRec, DRI2Setup, NULL };

void
DRI2Version(int *major, int *minor)
{
    if (major != NULL)
	*major = DRI2VersRec.majorversion;

    if (minor != NULL)
	*minor = DRI2VersRec.minorversion;
}
