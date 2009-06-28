/*
 * Copyright © 2007 Red Hat, Inc.
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
#include "region.h" 
#include "damage.h" 
#include "dri2.h"
#include <GL/internal/dri_sarea.h>

#include "xf86.h"

static DevPrivateKey dri2ScreenPrivateKey = &dri2ScreenPrivateKey;
static DevPrivateKey dri2WindowPrivateKey = &dri2WindowPrivateKey;
static DevPrivateKey dri2PixmapPrivateKey = &dri2PixmapPrivateKey;

typedef struct _DRI2DrawablePriv {
    unsigned int		 refCount;
    unsigned int		 boHandle;
    unsigned int		 dri2Handle;
} DRI2DrawablePrivRec, *DRI2DrawablePrivPtr;

typedef struct _DRI2Screen {
    int				 fd;
    drmBO			 sareaBO;
    void			*sarea;
    unsigned int		 sareaSize;
    const char			*driverName;
    unsigned int		 nextHandle;

    __DRIEventBuffer		*buffer;
    int				 locked;

    DRI2GetPixmapHandleProcPtr   getPixmapHandle;
    DRI2BeginClipNotifyProcPtr	 beginClipNotify;
    DRI2EndClipNotifyProcPtr	 endClipNotify;

    ClipNotifyProcPtr		 ClipNotify;
    HandleExposuresProcPtr	 HandleExposures;
} DRI2ScreenRec, *DRI2ScreenPtr;

static DRI2ScreenPtr
DRI2GetScreen(ScreenPtr pScreen)
{
    return dixLookupPrivate(&pScreen->devPrivates, dri2ScreenPrivateKey);
}

static void *
DRI2ScreenAllocEvent(DRI2ScreenPtr ds, size_t size)
{
    unsigned int *pad, mask = ds->buffer->size - 1;
    size_t pad_size;
    void *p;
    
    if ((ds->buffer->head & mask) + size > ds->buffer->size) {
	/* The requested event size would wrap the buffer, so pad to
	 * the end and allocate the event from the start. */
	pad_size = ds->buffer->size - (ds->buffer->head & mask);
	pad = (unsigned int *)
	    (ds->buffer->data + (ds->buffer->prealloc & mask));
	*pad = DRI2_EVENT_HEADER(DRI2_EVENT_PAD, pad_size);
	ds->buffer->prealloc += pad_size;
    }

    p = ds->buffer->data + (ds->buffer->prealloc & mask);
    ds->buffer->prealloc += size;

    return p;
}

static void
DRI2ScreenCommitEvents(DRI2ScreenPtr ds)
{
    ds->buffer->head = ds->buffer->prealloc;
}

static void
DRI2PostDrawableConfig(DrawablePtr pDraw)
{
    ScreenPtr			 pScreen = pDraw->pScreen;
    DRI2ScreenPtr		 ds = DRI2GetScreen(pScreen);
    DRI2DrawablePrivPtr		 pPriv;
    WindowPtr			 pWin;
    PixmapPtr			 pPixmap;
    BoxPtr			 pBox;
    BoxRec			 pixmapBox;
    int				 nBox;
    int				 i;
    __DRIDrawableConfigEvent	*e;
    size_t			 size;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pWin = (WindowPtr) pDraw;
	pPriv = dixLookupPrivate(&pWin->devPrivates, dri2WindowPrivateKey);

	nBox = REGION_NUM_RECTS(&pWin->clipList);
	pBox = REGION_RECTS(&pWin->clipList);

	pPixmap = pScreen->GetWindowPixmap(pWin);
    } else {
	pPixmap = (PixmapPtr) pDraw;
	pPriv = dixLookupPrivate(&pPixmap->devPrivates, dri2PixmapPrivateKey);

	pixmapBox.x1 = 0;
	pixmapBox.y1 = 0;
	pixmapBox.x2 = pDraw->width;
	pixmapBox.y2 = pDraw->height;
	nBox = 1;
	pBox = &pixmapBox;
    }

    if (!pPriv)
	return;

    size = sizeof *e + nBox * sizeof e->rects[0];

    e = DRI2ScreenAllocEvent(ds, size);
    e->event_header = DRI2_EVENT_HEADER(DRI2_EVENT_DRAWABLE_CONFIG, size);
    e->drawable = pPriv->dri2Handle;
    e->x = pDraw->x - pPixmap->screen_x;
    e->y = pDraw->y - pPixmap->screen_y;
    e->width = pDraw->width;
    e->height = pDraw->height;

    e->num_rects = nBox;
    for (i = 0; i < nBox; i++) {
	e->rects[i].x1 = pBox->x1 - pPixmap->screen_x;
	e->rects[i].y1 = pBox->y1 - pPixmap->screen_y;
	e->rects[i].x2 = pBox->x2 - pPixmap->screen_x;
	e->rects[i].y2 = pBox->y2 - pPixmap->screen_y;
	pBox++;
    }
}

static void
DRI2PostBufferAttach(DrawablePtr pDraw, Bool force)
{
    ScreenPtr			 pScreen = pDraw->pScreen;
    DRI2ScreenPtr		 ds = DRI2GetScreen(pScreen);
    DRI2DrawablePrivPtr		 pPriv;
    WindowPtr			 pWin;
    PixmapPtr			 pPixmap;
    __DRIBufferAttachEvent	*e;
    size_t			 size;
    unsigned int		 flags;
    unsigned int		 boHandle;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pWin = (WindowPtr) pDraw;
	pPixmap = pScreen->GetWindowPixmap(pWin);
	pPriv = dixLookupPrivate(&pWin->devPrivates, dri2WindowPrivateKey);
    } else {
	pPixmap = (PixmapPtr) pDraw;
	pPriv = dixLookupPrivate(&pPixmap->devPrivates, dri2PixmapPrivateKey);
    }

    if (!pPriv)
	return;

    boHandle = ds->getPixmapHandle(pPixmap, &flags);
    if (boHandle == pPriv->boHandle && !force)
	return;

    pPriv->boHandle = boHandle;
    size = sizeof *e;
    e = DRI2ScreenAllocEvent(ds, size);
    e->event_header = DRI2_EVENT_HEADER(DRI2_EVENT_BUFFER_ATTACH, size);
    e->drawable = pPriv->dri2Handle;
    e->buffer.attachment = DRI_DRAWABLE_BUFFER_FRONT_LEFT;
    e->buffer.handle = pPriv->boHandle;
    e->buffer.pitch = pPixmap->devKind;
    e->buffer.cpp = pPixmap->drawable.bitsPerPixel / 8;
    e->buffer.flags = flags;
}

static void
DRI2ClipNotify(WindowPtr pWin, int dx, int dy)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    DRI2ScreenPtr	ds = DRI2GetScreen(pScreen);

    if (!ds->locked) {
        ds->beginClipNotify(pScreen);
	ds->locked = 1;
    }

    if (ds->ClipNotify) {
	pScreen->ClipNotify = ds->ClipNotify;
	pScreen->ClipNotify(pWin, dx, dy);
	pScreen->ClipNotify = DRI2ClipNotify;
    }

    DRI2PostDrawableConfig(&pWin->drawable);
    DRI2PostBufferAttach(&pWin->drawable, FALSE);
}

static void
DRI2HandleExposures(WindowPtr pWin)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    DRI2ScreenPtr	ds = DRI2GetScreen(pScreen);

    if (ds->HandleExposures) {
	pScreen->HandleExposures = ds->HandleExposures;
	pScreen->HandleExposures(pWin);
	pScreen->HandleExposures = DRI2HandleExposures;
    }

    DRI2ScreenCommitEvents(ds);

    if (ds->locked) {
        ds->endClipNotify(pScreen);
	ds->locked = 0;
    }
}

void
DRI2CloseScreen(ScreenPtr pScreen)
{
    DRI2ScreenPtr	ds = DRI2GetScreen(pScreen);

    pScreen->ClipNotify		= ds->ClipNotify;
    pScreen->HandleExposures	= ds->HandleExposures;

    drmBOUnmap(ds->fd, &ds->sareaBO);
    drmBOUnreference(ds->fd, &ds->sareaBO);

    xfree(ds);
    dixSetPrivate(&pScreen->devPrivates, dri2ScreenPrivateKey, NULL);
}

Bool
DRI2CreateDrawable(DrawablePtr pDraw,
		   unsigned int *handle, unsigned int *head)
{
    DRI2ScreenPtr	ds = DRI2GetScreen(pDraw->pScreen);
    WindowPtr		pWin;
    PixmapPtr		pPixmap;
    DRI2DrawablePrivPtr pPriv;
    DevPrivateKey	key;
    PrivateRec		**devPrivates;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pWin = (WindowPtr) pDraw;
	devPrivates = &pWin->devPrivates;
	key = dri2WindowPrivateKey;
    } else {
	pPixmap = (PixmapPtr) pDraw;
	devPrivates = &pPixmap->devPrivates;
	key = dri2PixmapPrivateKey;
    }

    pPriv = dixLookupPrivate(devPrivates, key);
    if (pPriv != NULL) {
	pPriv->refCount++;
    } else {
	pPriv = xalloc(sizeof *pPriv);
	pPriv->refCount = 1;
	pPriv->boHandle = 0;
	pPriv->dri2Handle = ds->nextHandle++;
	dixSetPrivate(devPrivates, key, pPriv);
    }

    *handle = pPriv->dri2Handle;
    *head = ds->buffer->head;

    DRI2PostDrawableConfig(pDraw);
    DRI2PostBufferAttach(pDraw, TRUE);
    DRI2ScreenCommitEvents(ds);

    return TRUE;
}

void
DRI2DestroyDrawable(DrawablePtr pDraw)
{
    PixmapPtr		  pPixmap;
    WindowPtr		  pWin;
    DRI2DrawablePrivPtr   pPriv;
    DevPrivateKey	  key;
    PrivateRec		**devPrivates;

    if (pDraw->type == DRAWABLE_WINDOW) {
	pWin = (WindowPtr) pDraw;
	devPrivates = &pWin->devPrivates;
	key = dri2WindowPrivateKey;
    } else {
	pPixmap = (PixmapPtr) pDraw;
	devPrivates = &pPixmap->devPrivates;
	key = dri2PixmapPrivateKey;
    }

    pPriv = dixLookupPrivate(devPrivates, key);
    if (pPriv == NULL)
	return;
    
    pPriv->refCount--;
    if (pPriv->refCount == 0) {
	dixSetPrivate(devPrivates, key, NULL);
	xfree(pPriv);
    }
}

void
DRI2ReemitDrawableInfo(DrawablePtr pDraw, unsigned int *head)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pDraw->pScreen);

    *head = ds->buffer->head;

    DRI2PostDrawableConfig(pDraw);
    DRI2PostBufferAttach(pDraw, TRUE);
    DRI2ScreenCommitEvents(ds);
}

Bool
DRI2Connect(ScreenPtr pScreen, int *fd, const char **driverName,
	    unsigned int *sareaHandle)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pScreen);

    if (ds == NULL)
	return FALSE;

    *fd = ds->fd;
    *driverName = ds->driverName;
    *sareaHandle = ds->sareaBO.handle;

    return TRUE;
}

Bool
DRI2AuthConnection(ScreenPtr pScreen, drm_magic_t magic)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pScreen);

    if (ds == NULL || drmAuthMagic(ds->fd, magic))
	return FALSE;

    return TRUE;
}

unsigned int
DRI2GetPixmapHandle(PixmapPtr pPixmap, unsigned int *flags)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pPixmap->drawable.pScreen);

    return ds->getPixmapHandle(pPixmap, flags);
}

static void *
DRI2SetupSAREA(ScreenPtr pScreen, size_t driverSareaSize)
{
    DRI2ScreenPtr ds = DRI2GetScreen(pScreen);
    unsigned long mask;
    const size_t event_buffer_size = 32 * 1024;

    ds->sareaSize = 
	sizeof(*ds->buffer) + event_buffer_size +
	driverSareaSize +
	sizeof (unsigned int);

    mask = DRM_BO_FLAG_READ | DRM_BO_FLAG_WRITE | DRM_BO_FLAG_MAPPABLE |
	DRM_BO_FLAG_MEM_LOCAL | DRM_BO_FLAG_SHAREABLE;

    if (drmBOCreate(ds->fd, ds->sareaSize, 1, NULL, mask, 0, &ds->sareaBO))
	return NULL;

    if (drmBOMap(ds->fd, &ds->sareaBO,
		 DRM_BO_FLAG_READ | DRM_BO_FLAG_WRITE, 0, &ds->sarea)) {
	drmBOUnreference(ds->fd, &ds->sareaBO);
	return NULL;
    }

    xf86DrvMsg(pScreen->myNum, X_INFO,
	       "[DRI2] Allocated %d byte SAREA, BO handle 0x%08x\n",
	       ds->sareaSize, ds->sareaBO.handle);
    memset(ds->sarea, 0, ds->sareaSize);

    ds->buffer = ds->sarea;
    ds->buffer->block_header =
	DRI2_SAREA_BLOCK_HEADER(DRI2_SAREA_BLOCK_EVENT_BUFFER,
				sizeof *ds->buffer + event_buffer_size);
    ds->buffer->size = event_buffer_size;

    return DRI2_SAREA_BLOCK_NEXT(ds->buffer);
}

void *
DRI2ScreenInit(ScreenPtr pScreen, DRI2InfoPtr info)
{
    DRI2ScreenPtr ds;
    void *p;

    ds = xalloc(sizeof *ds);
    if (!ds)
	return NULL;

    ds->fd			= info->fd;
    ds->driverName		= info->driverName;
    ds->nextHandle		= 1;

    ds->getPixmapHandle		= info->getPixmapHandle;
    ds->beginClipNotify		= info->beginClipNotify;
    ds->endClipNotify		= info->endClipNotify;

    ds->ClipNotify		= pScreen->ClipNotify;
    pScreen->ClipNotify		= DRI2ClipNotify;
    ds->HandleExposures		= pScreen->HandleExposures;
    pScreen->HandleExposures	= DRI2HandleExposures;

    dixSetPrivate(&pScreen->devPrivates, dri2ScreenPrivateKey, ds);

    p = DRI2SetupSAREA(pScreen, info->driverSareaSize);
    if (p == NULL) {
	xfree(ds);
	return NULL;
    }

    xf86DrvMsg(pScreen->myNum, X_INFO, "[DRI2] Setup complete\n");

    return p;
}

extern ExtensionModule dri2ExtensionModule;

static pointer
DRI2Setup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    static Bool setupDone = FALSE;

    if (!setupDone) {
	setupDone = TRUE;
	LoadExtension(&dri2ExtensionModule, FALSE);
    } else {
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
    1, 0, 0,
    ABI_CLASS_EXTENSION,
    ABI_EXTENSION_VERSION,
    MOD_CLASS_NONE,
    { 0, 0, 0, 0 }
};

_X_EXPORT XF86ModuleData dri2ModuleData = { &DRI2VersRec, DRI2Setup, NULL };

