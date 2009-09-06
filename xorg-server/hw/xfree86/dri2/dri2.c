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

#include <xf86drm.h>
#include "xf86Module.h"
#include "scrnintstr.h"
#include "windowstr.h"
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
    unsigned int	 pendingSequence;
} DRI2DrawableRec, *DRI2DrawablePtr;

typedef struct _DRI2Screen {
    const char			*driverName;
    const char			*deviceName;
    int				 fd;
    unsigned int		 lastSequence;

    DRI2CreateBufferProcPtr	 CreateBuffer;
    DRI2DestroyBufferProcPtr	 DestroyBuffer;
    DRI2CopyRegionProcPtr	 CopyRegion;

    HandleExposuresProcPtr       HandleExposures;
} DRI2ScreenRec, *DRI2ScreenPtr;

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

static DRI2BufferPtr
allocate_or_reuse_buffer(DrawablePtr pDraw, DRI2ScreenPtr ds,
			 DRI2DrawablePtr pPriv,
			 unsigned int attachment, unsigned int format,
			 int dimensions_match)
{
    DRI2BufferPtr buffer;
    int old_buf;

    old_buf = find_attachment(pPriv, attachment);

    if ((old_buf < 0)
	|| !dimensions_match
	|| (pPriv->buffers[old_buf]->format != format)) {
	buffer = (*ds->CreateBuffer)(pDraw, attachment, format);
    } else {
	buffer = pPriv->buffers[old_buf];
	pPriv->buffers[old_buf] = NULL;
    }

    return buffer;
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

	buffers[i] = allocate_or_reuse_buffer(pDraw, ds, pPriv, attachment,
					      format, dimensions_match);

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
	buffers[i++] = allocate_or_reuse_buffer(pDraw, ds, pPriv,
						DRI2BufferFrontLeft,
						front_format, dimensions_match);
    }

    if (need_fake_front > 0) {
	buffers[i++] = allocate_or_reuse_buffer(pDraw, ds, pPriv,
						DRI2BufferFakeFrontLeft,
						front_format, dimensions_match);
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
    if (have_fake_front) {
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

void
DRI2DestroyDrawable(DrawablePtr pDraw)
{
    DRI2ScreenPtr   ds = DRI2GetScreen(pDraw->pScreen);
    DRI2DrawablePtr pPriv;
    WindowPtr  	    pWin;
    PixmapPtr	    pPixmap;

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

Bool
DRI2Connect(ScreenPtr pScreen, unsigned int driverType, int *fd,
	    const char **driverName, const char **deviceName)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pScreen);

    if (ds == NULL)
	return FALSE;

    if (driverType != DRI2DriverDRI)
	return BadValue;

    *fd = ds->fd;
    *driverName = ds->driverName;
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

    if (info->version < 3)
	return FALSE;

    if (!xf86VGAarbiterAllowDRI(pScreen)) {
        xf86DrvMsg(pScreen->myNum, X_WARNING,
                  "[DRI2] Direct rendering is not supported when VGA arb is necessary for the device\n");
        return FALSE;
    }

    ds = xalloc(sizeof *ds);
    if (!ds)
	return FALSE;

    ds->fd	       = info->fd;
    ds->driverName     = info->driverName;
    ds->deviceName     = info->deviceName;

    ds->CreateBuffer   = info->CreateBuffer;
    ds->DestroyBuffer  = info->DestroyBuffer;
    ds->CopyRegion     = info->CopyRegion;

    dixSetPrivate(&pScreen->devPrivates, dri2ScreenPrivateKey, ds);

    xf86DrvMsg(pScreen->myNum, X_INFO, "[DRI2] Setup complete\n");

    return TRUE;
}

void
DRI2CloseScreen(ScreenPtr pScreen)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pScreen);

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
    1, 1, 0,
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
