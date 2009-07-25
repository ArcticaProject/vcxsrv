/*
 * Copyright © 2001 Keith Packard
 *
 * Partly based on code that is Copyright © The XFree86 Project Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/** @file
 * This file covers the initialization and teardown of EXA, and has various
 * functions not responsible for performing rendering, pixmap migration, or
 * memory management.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>

#include "exa_priv.h"
#include "exa.h"

static int exaScreenPrivateKeyIndex;
DevPrivateKey exaScreenPrivateKey = &exaScreenPrivateKeyIndex;
static int exaPixmapPrivateKeyIndex;
DevPrivateKey exaPixmapPrivateKey = &exaPixmapPrivateKeyIndex;

#ifdef MITSHM
static ShmFuncs exaShmFuncs = { NULL, NULL };
#endif

static _X_INLINE void*
ExaGetPixmapAddress(PixmapPtr p)
{
    ExaPixmapPriv(p);

    if (pExaPixmap->offscreen && pExaPixmap->fb_ptr)
	return pExaPixmap->fb_ptr;
    else
	return pExaPixmap->sys_ptr;
}

/**
 * exaGetPixmapOffset() returns the offset (in bytes) within the framebuffer of
 * the beginning of the given pixmap.
 *
 * Note that drivers are free to, and often do, munge this offset as necessary
 * for handing to the hardware -- for example, translating it into a different
 * aperture.  This function may need to be extended in the future if we grow
 * support for having multiple card-accessible offscreen, such as an AGP memory
 * pool alongside the framebuffer pool.
 */
unsigned long
exaGetPixmapOffset(PixmapPtr pPix)
{
    ExaScreenPriv (pPix->drawable.pScreen);

    return ((unsigned long)ExaGetPixmapAddress(pPix) -
	    (unsigned long)pExaScr->info->memoryBase);
}

void *
exaGetPixmapDriverPrivate(PixmapPtr pPix)
{
    ExaPixmapPriv(pPix);

    return pExaPixmap->driverPriv;
}

/**
 * exaGetPixmapPitch() returns the pitch (in bytes) of the given pixmap.
 *
 * This is a helper to make driver code more obvious, due to the rather obscure
 * naming of the pitch field in the pixmap.
 */
unsigned long
exaGetPixmapPitch(PixmapPtr pPix)
{
    return pPix->devKind;
}

/**
 * exaGetPixmapSize() returns the size in bytes of the given pixmap in video
 * memory. Only valid when the pixmap is currently in framebuffer.
 */
unsigned long
exaGetPixmapSize(PixmapPtr pPix)
{
    ExaPixmapPrivPtr pExaPixmap;

    pExaPixmap = ExaGetPixmapPriv(pPix);
    if (pExaPixmap != NULL)
	return pExaPixmap->fb_size;
    return 0;
}

/**
 * exaGetDrawablePixmap() returns a backing pixmap for a given drawable.
 *
 * @param pDrawable the drawable being requested.
 *
 * This function returns the backing pixmap for a drawable, whether it is a
 * redirected window, unredirected window, or already a pixmap.  Note that
 * coordinate translation is needed when drawing to the backing pixmap of a
 * redirected window, and the translation coordinates are provided by calling
 * exaGetOffscreenPixmap() on the drawable.
 */
PixmapPtr
exaGetDrawablePixmap(DrawablePtr pDrawable)
{
     if (pDrawable->type == DRAWABLE_WINDOW)
	return pDrawable->pScreen->GetWindowPixmap ((WindowPtr) pDrawable);
     else
	return (PixmapPtr) pDrawable;
}	

/**
 * Sets the offsets to add to coordinates to make them address the same bits in
 * the backing drawable. These coordinates are nonzero only for redirected
 * windows.
 */
void
exaGetDrawableDeltas (DrawablePtr pDrawable, PixmapPtr pPixmap,
		      int *xp, int *yp)
{
#ifdef COMPOSITE
    if (pDrawable->type == DRAWABLE_WINDOW) {
	*xp = -pPixmap->screen_x;
	*yp = -pPixmap->screen_y;
	return;
    }
#endif

    *xp = 0;
    *yp = 0;
}

/**
 * exaPixmapDirty() marks a pixmap as dirty, allowing for
 * optimizations in pixmap migration when no changes have occurred.
 */
void
exaPixmapDirty (PixmapPtr pPix, int x1, int y1, int x2, int y2)
{
    ExaPixmapPriv(pPix);
    BoxRec box;
    RegionPtr pDamageReg;
    RegionRec region;

    if (!pExaPixmap || !pExaPixmap->pDamage)
	return;
	
    box.x1 = max(x1, 0);
    box.y1 = max(y1, 0);
    box.x2 = min(x2, pPix->drawable.width);
    box.y2 = min(y2, pPix->drawable.height);

    if (box.x1 >= box.x2 || box.y1 >= box.y2)
	return;

    pDamageReg = DamageRegion(pExaPixmap->pDamage);

    REGION_INIT(pScreen, &region, &box, 1);
    REGION_UNION(pScreen, pDamageReg, pDamageReg, &region);
    REGION_UNINIT(pScreen, &region);
}

static Bool
exaDestroyPixmap (PixmapPtr pPixmap)
{
    ScreenPtr	pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);

    if (pPixmap->refcnt == 1)
    {
	ExaPixmapPriv (pPixmap);

	if (pExaPixmap->driverPriv) {
	    pExaScr->info->DestroyPixmap(pScreen, pExaPixmap->driverPriv);
	    pExaPixmap->driverPriv = NULL;
	}

	if (pExaPixmap->area)
	{
	    DBG_PIXMAP(("-- 0x%p (0x%x) (%dx%d)\n",
                        (void*)pPixmap->drawable.id,
			 ExaGetPixmapPriv(pPixmap)->area->offset,
			 pPixmap->drawable.width,
			 pPixmap->drawable.height));
	    /* Free the offscreen area */
	    exaOffscreenFree (pPixmap->drawable.pScreen, pExaPixmap->area);
	    pPixmap->devPrivate.ptr = pExaPixmap->sys_ptr;
	    pPixmap->devKind = pExaPixmap->sys_pitch;
	}
	REGION_UNINIT(pPixmap->drawable.pScreen, &pExaPixmap->validSys);
	REGION_UNINIT(pPixmap->drawable.pScreen, &pExaPixmap->validFB);
    }
    return fbDestroyPixmap (pPixmap);
}

static int
exaLog2(int val)
{
    int bits;

    if (val <= 0)
	return 0;
    for (bits = 0; val != 0; bits++)
	val >>= 1;
    return bits - 1;
}

static void
exaSetAccelBlock(ExaScreenPrivPtr pExaScr, ExaPixmapPrivPtr pExaPixmap,
                 int w, int h, int bpp)
{
    pExaPixmap->accel_blocked = 0;

    if (pExaScr->info->maxPitchPixels) {
        int max_pitch = pExaScr->info->maxPitchPixels * (bpp + 7) / 8;

        if (pExaPixmap->fb_pitch > max_pitch)
            pExaPixmap->accel_blocked |= EXA_RANGE_PITCH;
    }

    if (pExaScr->info->maxPitchBytes &&
        pExaPixmap->fb_pitch > pExaScr->info->maxPitchBytes)
        pExaPixmap->accel_blocked |= EXA_RANGE_PITCH;

    if (w > pExaScr->info->maxX)
        pExaPixmap->accel_blocked |= EXA_RANGE_WIDTH;

    if (h > pExaScr->info->maxY)
        pExaPixmap->accel_blocked |= EXA_RANGE_HEIGHT;
}

static void
exaSetFbPitch(ExaScreenPrivPtr pExaScr, ExaPixmapPrivPtr pExaPixmap,
              int w, int h, int bpp)
{
    if (pExaScr->info->flags & EXA_OFFSCREEN_ALIGN_POT && w != 1)
        pExaPixmap->fb_pitch = (1 << (exaLog2(w - 1) + 1)) * bpp / 8;
    else
        pExaPixmap->fb_pitch = w * bpp / 8;

    pExaPixmap->fb_pitch = EXA_ALIGN(pExaPixmap->fb_pitch,
                                     pExaScr->info->pixmapPitchAlign);
}

/**
 * exaCreatePixmap() creates a new pixmap.
 *
 * If width and height are 0, this won't be a full-fledged pixmap and it will
 * get ModifyPixmapHeader() called on it later.  So, we mark it as pinned, because
 * ModifyPixmapHeader() would break migration.  These types of pixmaps are used
 * for scratch pixmaps, or to represent the visible screen.
 */
static PixmapPtr
exaCreatePixmap(ScreenPtr pScreen, int w, int h, int depth,
		unsigned usage_hint)
{
    PixmapPtr		pPixmap;
    ExaPixmapPrivPtr	pExaPixmap;
    int                 driver_alloc = 0;
    int			bpp;
    ExaScreenPriv(pScreen);

    if (w > 32767 || h > 32767)
	return NullPixmap;

    if (!pExaScr->info->CreatePixmap) {
        pPixmap = fbCreatePixmap (pScreen, w, h, depth, usage_hint);
    } else {
        driver_alloc = 1;
        pPixmap = fbCreatePixmap(pScreen, 0, 0, depth, usage_hint);
    }

    if (!pPixmap)
        return NULL;

    pExaPixmap = ExaGetPixmapPriv(pPixmap);
    pExaPixmap->driverPriv = NULL;

    bpp = pPixmap->drawable.bitsPerPixel;

    if (driver_alloc) {
        size_t paddedWidth, datasize;

	paddedWidth = ((w * bpp + FB_MASK) >> FB_SHIFT) * sizeof(FbBits);
        if (paddedWidth / 4 > 32767 || h > 32767)
            return NullPixmap;

        exaSetFbPitch(pExaScr, pExaPixmap, w, h, bpp);

        if (paddedWidth < pExaPixmap->fb_pitch)
            paddedWidth = pExaPixmap->fb_pitch;

        datasize = h * paddedWidth;

	/* Set this before driver hooks, to allow for !offscreen pixmaps.
	 * !offscreen pixmaps have a valid pointer at all times.
	 */
	pPixmap->devPrivate.ptr = NULL;

        pExaPixmap->driverPriv = pExaScr->info->CreatePixmap(pScreen, datasize, 0);
        if (!pExaPixmap->driverPriv) {
             fbDestroyPixmap(pPixmap);
             return NULL;
        }

        (*pScreen->ModifyPixmapHeader)(pPixmap, w, h, 0, 0,
                                       paddedWidth, NULL);
        pExaPixmap->score = EXA_PIXMAP_SCORE_PINNED;
        pExaPixmap->fb_ptr = NULL;
        pExaPixmap->pDamage = NULL;
        pExaPixmap->sys_ptr = pPixmap->devPrivate.ptr;

    } else {
        pExaPixmap->driverPriv = NULL;
        /* Scratch pixmaps may have w/h equal to zero, and may not be
	 * migrated.
	 */
        if (!w || !h)
	    pExaPixmap->score = EXA_PIXMAP_SCORE_PINNED;
        else
            pExaPixmap->score = EXA_PIXMAP_SCORE_INIT;

        pExaPixmap->sys_ptr = pPixmap->devPrivate.ptr;
        pExaPixmap->sys_pitch = pPixmap->devKind;

        pPixmap->devPrivate.ptr = NULL;
        pExaPixmap->offscreen = FALSE;

        pExaPixmap->fb_ptr = NULL;
        exaSetFbPitch(pExaScr, pExaPixmap, w, h, bpp);
        pExaPixmap->fb_size = pExaPixmap->fb_pitch * h;

        if (pExaPixmap->fb_pitch > 131071) {
	     fbDestroyPixmap(pPixmap);
	     return NULL;
        }

	/* Set up damage tracking */
	pExaPixmap->pDamage = DamageCreate (NULL, NULL,
					    DamageReportNone, TRUE,
					    pScreen, pPixmap);

	if (pExaPixmap->pDamage == NULL) {
	    fbDestroyPixmap (pPixmap);
	    return NULL;
	}

	DamageRegister (&pPixmap->drawable, pExaPixmap->pDamage);
	/* This ensures that pending damage reflects the current operation. */
	/* This is used by exa to optimize migration. */
	DamageSetReportAfterOp (pExaPixmap->pDamage, TRUE);
    }
 
    pExaPixmap->area = NULL;

    /* None of the pixmap bits are valid initially */
    REGION_NULL(pScreen, &pExaPixmap->validSys);
    REGION_NULL(pScreen, &pExaPixmap->validFB);

    exaSetAccelBlock(pExaScr, pExaPixmap,
                     w, h, bpp);

    return pPixmap;
}

static Bool
exaModifyPixmapHeader(PixmapPtr pPixmap, int width, int height, int depth,
		      int bitsPerPixel, int devKind, pointer pPixData)
{
    ExaScreenPrivPtr pExaScr;
    ExaPixmapPrivPtr pExaPixmap;
    Bool ret;

    if (!pPixmap)
        return FALSE;

    pExaScr = ExaGetScreenPriv(pPixmap->drawable.pScreen);
    pExaPixmap = ExaGetPixmapPriv(pPixmap);

    if (pExaPixmap) {
        if (pPixData)
            pExaPixmap->sys_ptr = pPixData;

        if (devKind > 0)
            pExaPixmap->sys_pitch = devKind;

        if (width > 0 && height > 0 && bitsPerPixel > 0) {
            exaSetFbPitch(pExaScr, pExaPixmap,
                          width, height, bitsPerPixel);

            exaSetAccelBlock(pExaScr, pExaPixmap,
                             width, height, bitsPerPixel);
        }
    }


    if (pExaScr->info->ModifyPixmapHeader) {
	ret = pExaScr->info->ModifyPixmapHeader(pPixmap, width, height, depth,
						bitsPerPixel, devKind, pPixData);
	if (ret == TRUE)
	    return ret;
    }
    return pExaScr->SavedModifyPixmapHeader(pPixmap, width, height, depth,
					    bitsPerPixel, devKind, pPixData);
}

/**
 * exaPixmapIsOffscreen() is used to determine if a pixmap is in offscreen
 * memory, meaning that acceleration could probably be done to it, and that it
 * will need to be wrapped by PrepareAccess()/FinishAccess() when accessing it
 * with the CPU.
 *
 * Note that except for UploadToScreen()/DownloadFromScreen() (which explicitly
 * deal with moving pixmaps in and out of system memory), EXA will give drivers
 * pixmaps as arguments for which exaPixmapIsOffscreen() is TRUE.
 *
 * @return TRUE if the given drawable is in framebuffer memory.
 */
Bool
exaPixmapIsOffscreen(PixmapPtr p)
{
    ScreenPtr	pScreen = p->drawable.pScreen;
    ExaScreenPriv(pScreen);
    ExaPixmapPriv(p);
    void *save_ptr;
    Bool ret;

    save_ptr = p->devPrivate.ptr;

    if (!save_ptr && pExaPixmap && !(pExaScr->info->flags & EXA_HANDLES_PIXMAPS))
	p->devPrivate.ptr = ExaGetPixmapAddress(p);

    if (pExaScr->info->PixmapIsOffscreen)
	ret = pExaScr->info->PixmapIsOffscreen(p);
    else
       ret = ((unsigned long) ((CARD8 *) p->devPrivate.ptr -
			       (CARD8 *) pExaScr->info->memoryBase) <
	      pExaScr->info->memorySize);

    p->devPrivate.ptr = save_ptr;

    return ret;
}

/**
 * exaDrawableIsOffscreen() is a convenience wrapper for exaPixmapIsOffscreen().
 */
Bool
exaDrawableIsOffscreen (DrawablePtr pDrawable)
{
    return exaPixmapIsOffscreen (exaGetDrawablePixmap (pDrawable));
}

/**
 * Returns the pixmap which backs a drawable, and the offsets to add to
 * coordinates to make them address the same bits in the backing drawable.
 */
PixmapPtr
exaGetOffscreenPixmap (DrawablePtr pDrawable, int *xp, int *yp)
{
    PixmapPtr	pPixmap = exaGetDrawablePixmap (pDrawable);

    exaGetDrawableDeltas (pDrawable, pPixmap, xp, yp);

    if (exaPixmapIsOffscreen (pPixmap))
	return pPixmap;
    else
	return NULL;
}

void
ExaDoPrepareAccess(DrawablePtr pDrawable, int index)
{
    ScreenPtr	    pScreen = pDrawable->pScreen;
    ExaScreenPriv  (pScreen);
    PixmapPtr	    pPixmap = exaGetDrawablePixmap (pDrawable);
    Bool	    offscreen = exaPixmapIsOffscreen(pPixmap);

    /* Unhide pixmap pointer */
    if (pPixmap->devPrivate.ptr == NULL && !(pExaScr->info->flags & EXA_HANDLES_PIXMAPS)) {
	pPixmap->devPrivate.ptr = ExaGetPixmapAddress(pPixmap);
    }

    if (!offscreen)
	return;

    exaWaitSync (pDrawable->pScreen);

    if (pExaScr->info->PrepareAccess == NULL)
	return;

    if (index >= EXA_PREPARE_AUX0 &&
	!(pExaScr->info->flags & EXA_SUPPORTS_PREPARE_AUX)) {
	exaMoveOutPixmap (pPixmap);
	return;
    }

    if (!(*pExaScr->info->PrepareAccess) (pPixmap, index)) {
	ExaPixmapPriv (pPixmap);
	if (pExaPixmap->score == EXA_PIXMAP_SCORE_PINNED)
	    FatalError("Driver failed PrepareAccess on a pinned pixmap\n");
	exaMoveOutPixmap (pPixmap);
    }
}

void
exaPrepareAccessReg(DrawablePtr pDrawable, int index, RegionPtr pReg)
{
    ExaMigrationRec pixmaps[1];

    pixmaps[0].as_dst = index == EXA_PREPARE_DEST;
    pixmaps[0].as_src = index != EXA_PREPARE_DEST;
    pixmaps[0].pPix = exaGetDrawablePixmap (pDrawable);
    pixmaps[0].pReg = pReg;

    exaDoMigration(pixmaps, 1, FALSE);

    ExaDoPrepareAccess(pDrawable, index);
}

/**
 * exaPrepareAccess() is EXA's wrapper for the driver's PrepareAccess() handler.
 *
 * It deals with waiting for synchronization with the card, determining if
 * PrepareAccess() is necessary, and working around PrepareAccess() failure.
 */
void
exaPrepareAccess(DrawablePtr pDrawable, int index)
{
    exaPrepareAccessReg(pDrawable, index, NULL);
}

/**
 * exaFinishAccess() is EXA's wrapper for the driver's FinishAccess() handler.
 *
 * It deals with calling the driver's FinishAccess() only if necessary.
 */
void
exaFinishAccess(DrawablePtr pDrawable, int index)
{
    ScreenPtr	    pScreen = pDrawable->pScreen;
    ExaScreenPriv  (pScreen);
    PixmapPtr	    pPixmap = exaGetDrawablePixmap (pDrawable);
    ExaPixmapPriv  (pPixmap);

    /* Rehide pixmap pointer if we're doing that. */
    if (pExaPixmap && !(pExaScr->info->flags & EXA_HANDLES_PIXMAPS)) {
	pPixmap->devPrivate.ptr = NULL;
    }

    if (pExaScr->info->FinishAccess == NULL)
	return;

    if (!exaPixmapIsOffscreen (pPixmap))
	return;

    if (index >= EXA_PREPARE_AUX0 &&
	!(pExaScr->info->flags & EXA_SUPPORTS_PREPARE_AUX)) {
	ErrorF("EXA bug: Trying to call driver FinishAccess hook with "
	       "unsupported index EXA_PREPARE_AUX*\n");
	return;
    }

    (*pExaScr->info->FinishAccess) (pPixmap, index);
}

/**
 * exaValidateGC() sets the ops to EXA's implementations, which may be
 * accelerated or may sync the card and fall back to fb.
 */
static void
exaValidateGC (GCPtr pGC, unsigned long changes, DrawablePtr pDrawable)
{
    /* fbValidateGC will do direct access to pixmaps if the tiling has changed.
     * Preempt fbValidateGC by doing its work and masking the change out, so
     * that we can do the Prepare/FinishAccess.
     */
#ifdef FB_24_32BIT
    if ((changes & GCTile) && fbGetRotatedPixmap(pGC)) {
	(*pGC->pScreen->DestroyPixmap) (fbGetRotatedPixmap(pGC));
	fbGetRotatedPixmap(pGC) = 0;
    }
	
    if (pGC->fillStyle == FillTiled) {
	PixmapPtr	pOldTile, pNewTile;

	pOldTile = pGC->tile.pixmap;
	if (pOldTile->drawable.bitsPerPixel != pDrawable->bitsPerPixel)
	{
	    pNewTile = fbGetRotatedPixmap(pGC);
	    if (!pNewTile ||
		pNewTile ->drawable.bitsPerPixel != pDrawable->bitsPerPixel)
	    {
		if (pNewTile)
		    (*pGC->pScreen->DestroyPixmap) (pNewTile);
		/* fb24_32ReformatTile will do direct access of a newly-
		 * allocated pixmap.  This isn't a problem yet, since we don't
		 * put pixmaps in FB until at least one accelerated EXA op.
		 */
		exaPrepareAccess(&pOldTile->drawable, EXA_PREPARE_SRC);
		pNewTile = fb24_32ReformatTile (pOldTile,
						pDrawable->bitsPerPixel);
		exaPixmapDirty(pNewTile, 0, 0, pNewTile->drawable.width, pNewTile->drawable.height);
		exaFinishAccess(&pOldTile->drawable, EXA_PREPARE_SRC);
	    }
	    if (pNewTile)
	    {
		fbGetRotatedPixmap(pGC) = pOldTile;
		pGC->tile.pixmap = pNewTile;
		changes |= GCTile;
	    }
	}
    }
#endif
    if (changes & GCTile) {
	if (!pGC->tileIsPixel && FbEvenTile (pGC->tile.pixmap->drawable.width *
					     pDrawable->bitsPerPixel))
	{
	    exaPrepareAccess(&pGC->tile.pixmap->drawable, EXA_PREPARE_SRC);
	    fbPadPixmap (pGC->tile.pixmap);
	    exaFinishAccess(&pGC->tile.pixmap->drawable, EXA_PREPARE_SRC);
	    exaPixmapDirty(pGC->tile.pixmap, 0, 0,
			   pGC->tile.pixmap->drawable.width,
			   pGC->tile.pixmap->drawable.height);
	}
	/* Mask out the GCTile change notification, now that we've done FB's
	 * job for it.
	 */
	changes &= ~GCTile;
    }

    exaPrepareAccessGC(pGC);
    fbValidateGC (pGC, changes, pDrawable);
    exaFinishAccessGC(pGC);

    pGC->ops = (GCOps *) &exaOps;
}

static GCFuncs	exaGCFuncs = {
    exaValidateGC,
    miChangeGC,
    miCopyGC,
    miDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip
};

/**
 * exaCreateGC makes a new GC and hooks up its funcs handler, so that
 * exaValidateGC() will get called.
 */
static int
exaCreateGC (GCPtr pGC)
{
    if (!fbCreateGC (pGC))
	return FALSE;

    pGC->funcs = &exaGCFuncs;

    return TRUE;
}

static Bool
exaChangeWindowAttributes(WindowPtr pWin, unsigned long mask)
{
    Bool ret;

    if ((mask & CWBackPixmap) && pWin->backgroundState == BackgroundPixmap) 
        exaPrepareAccess(&pWin->background.pixmap->drawable, EXA_PREPARE_SRC);

    if ((mask & CWBorderPixmap) && pWin->borderIsPixel == FALSE)
        exaPrepareAccess(&pWin->border.pixmap->drawable, EXA_PREPARE_MASK);

    ret = fbChangeWindowAttributes(pWin, mask);

    if ((mask & CWBorderPixmap) && pWin->borderIsPixel == FALSE)
        exaFinishAccess(&pWin->border.pixmap->drawable, EXA_PREPARE_MASK);

    if ((mask & CWBackPixmap) && pWin->backgroundState == BackgroundPixmap) 
        exaFinishAccess(&pWin->background.pixmap->drawable, EXA_PREPARE_SRC);

    return ret;
}

static RegionPtr
exaBitmapToRegion(PixmapPtr pPix)
{
  RegionPtr ret;
  exaPrepareAccess(&pPix->drawable, EXA_PREPARE_SRC);
  ret = fbPixmapToRegion(pPix);
  exaFinishAccess(&pPix->drawable, EXA_PREPARE_SRC);
  return ret;
}

static Bool
exaCreateScreenResources(ScreenPtr pScreen)
{
    ExaScreenPriv(pScreen);
    PixmapPtr pScreenPixmap;
    Bool b;

    pScreen->CreateScreenResources = pExaScr->SavedCreateScreenResources;
    b = pScreen->CreateScreenResources(pScreen);
    pScreen->CreateScreenResources = exaCreateScreenResources;

    if (!b)
        return FALSE;

    pScreenPixmap = pScreen->GetScreenPixmap(pScreen);

    if (pScreenPixmap) {
        ExaPixmapPriv(pScreenPixmap);

        exaSetAccelBlock(pExaScr, pExaPixmap,
                         pScreenPixmap->drawable.width,
                         pScreenPixmap->drawable.height,
                         pScreenPixmap->drawable.bitsPerPixel);
    }

    return TRUE;
}

/**
 * exaCloseScreen() unwraps its wrapped screen functions and tears down EXA's
 * screen private, before calling down to the next CloseSccreen.
 */
static Bool
exaCloseScreen(int i, ScreenPtr pScreen)
{
    ExaScreenPriv(pScreen);
#ifdef RENDER
    PictureScreenPtr	ps = GetPictureScreenIfSet(pScreen);
#endif

    if (ps->Glyphs == exaGlyphs)
	exaGlyphsFini(pScreen);

    pScreen->CreateGC = pExaScr->SavedCreateGC;
    pScreen->CloseScreen = pExaScr->SavedCloseScreen;
    pScreen->GetImage = pExaScr->SavedGetImage;
    pScreen->GetSpans = pExaScr->SavedGetSpans;
    pScreen->CreatePixmap = pExaScr->SavedCreatePixmap;
    pScreen->DestroyPixmap = pExaScr->SavedDestroyPixmap;
    pScreen->CopyWindow = pExaScr->SavedCopyWindow;
    pScreen->ChangeWindowAttributes = pExaScr->SavedChangeWindowAttributes;
    pScreen->BitmapToRegion = pExaScr->SavedBitmapToRegion;
    pScreen->CreateScreenResources = pExaScr->SavedCreateScreenResources;
#ifdef RENDER
    if (ps) {
	ps->Composite = pExaScr->SavedComposite;
	ps->Glyphs = pExaScr->SavedGlyphs;
	ps->Trapezoids = pExaScr->SavedTrapezoids;
	ps->Triangles = pExaScr->SavedTriangles;
	ps->AddTraps = pExaScr->SavedAddTraps;
    }
#endif

    xfree (pExaScr);

    return (*pScreen->CloseScreen) (i, pScreen);
}

/**
 * This function allocates a driver structure for EXA drivers to fill in.  By
 * having EXA allocate the structure, the driver structure can be extended
 * without breaking ABI between EXA and the drivers.  The driver's
 * responsibility is to check beforehand that the EXA module has a matching
 * major number and sufficient minor.  Drivers are responsible for freeing the
 * driver structure using xfree().
 *
 * @return a newly allocated, zero-filled driver structure
 */
ExaDriverPtr
exaDriverAlloc(void)
{
    return xcalloc(1, sizeof(ExaDriverRec));
}

/**
 * @param pScreen screen being initialized
 * @param pScreenInfo EXA driver record
 *
 * exaDriverInit sets up EXA given a driver record filled in by the driver.
 * pScreenInfo should have been allocated by exaDriverAlloc().  See the
 * comments in _ExaDriver for what must be filled in and what is optional.
 *
 * @return TRUE if EXA was successfully initialized.
 */
Bool
exaDriverInit (ScreenPtr		pScreen,
               ExaDriverPtr	pScreenInfo)
{
    ExaScreenPrivPtr pExaScr;
#ifdef RENDER
    PictureScreenPtr ps;
#endif

    if (!pScreenInfo)
	return FALSE;

    if (pScreenInfo->exa_major != EXA_VERSION_MAJOR ||
	pScreenInfo->exa_minor > EXA_VERSION_MINOR)
    {
	LogMessage(X_ERROR, "EXA(%d): driver's EXA version requirements "
		   "(%d.%d) are incompatible with EXA version (%d.%d)\n",
		   pScreen->myNum,
		   pScreenInfo->exa_major, pScreenInfo->exa_minor,
		   EXA_VERSION_MAJOR, EXA_VERSION_MINOR);
	return FALSE;
    }

    if (!pScreenInfo->CreatePixmap) {
	if (!pScreenInfo->memoryBase) {
	    LogMessage(X_ERROR, "EXA(%d): ExaDriverRec::memoryBase "
		       "must be non-zero\n", pScreen->myNum);
	    return FALSE;
	}

	if (!pScreenInfo->memorySize) {
	    LogMessage(X_ERROR, "EXA(%d): ExaDriverRec::memorySize must be "
		       "non-zero\n", pScreen->myNum);
	    return FALSE;
	}

	if (pScreenInfo->offScreenBase > pScreenInfo->memorySize) {
	    LogMessage(X_ERROR, "EXA(%d): ExaDriverRec::offScreenBase must "
		       "be <= ExaDriverRec::memorySize\n", pScreen->myNum);
	    return FALSE;
	}
    }

    if (!pScreenInfo->PrepareSolid) {
	LogMessage(X_ERROR, "EXA(%d): ExaDriverRec::PrepareSolid must be "
		   "non-NULL\n", pScreen->myNum);
	return FALSE;
    }

    if (!pScreenInfo->PrepareCopy) {
	LogMessage(X_ERROR, "EXA(%d): ExaDriverRec::PrepareCopy must be "
		   "non-NULL\n", pScreen->myNum);
	return FALSE;
    }

    if (!pScreenInfo->WaitMarker) {
	LogMessage(X_ERROR, "EXA(%d): ExaDriverRec::WaitMarker must be "
		   "non-NULL\n", pScreen->myNum);
	return FALSE;
    }

    /* If the driver doesn't set any max pitch values, we'll just assume
     * that there's a limitation by pixels, and that it's the same as
     * maxX.
     *
     * We want maxPitchPixels or maxPitchBytes to be set so we can check
     * pixmaps against the max pitch in exaCreatePixmap() -- it matters
     * whether a pixmap is rejected because of its pitch or
     * because of its width.
     */
    if (!pScreenInfo->maxPitchPixels && !pScreenInfo->maxPitchBytes)
    {
        pScreenInfo->maxPitchPixels = pScreenInfo->maxX;
    }

#ifdef RENDER
    ps = GetPictureScreenIfSet(pScreen);
#endif

    pExaScr = xcalloc (sizeof (ExaScreenPrivRec), 1);

    if (!pExaScr) {
        LogMessage(X_WARNING, "EXA(%d): Failed to allocate screen private\n",
		   pScreen->myNum);
	return FALSE;
    }

    pExaScr->info = pScreenInfo;

    dixSetPrivate(&pScreen->devPrivates, exaScreenPrivateKey, pExaScr);

    pExaScr->migration = ExaMigrationAlways;

    exaDDXDriverInit(pScreen);

    /*
     * Replace various fb screen functions
     */
    pExaScr->SavedCloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = exaCloseScreen;

    pExaScr->SavedCreateGC = pScreen->CreateGC;
    pScreen->CreateGC = exaCreateGC;

    pExaScr->SavedGetImage = pScreen->GetImage;
    pScreen->GetImage = exaGetImage;

    pExaScr->SavedGetSpans = pScreen->GetSpans;
    pScreen->GetSpans = ExaCheckGetSpans;

    pExaScr->SavedCopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = exaCopyWindow;

    pExaScr->SavedChangeWindowAttributes = pScreen->ChangeWindowAttributes;
    pScreen->ChangeWindowAttributes = exaChangeWindowAttributes;

    pExaScr->SavedBitmapToRegion = pScreen->BitmapToRegion;
    pScreen->BitmapToRegion = exaBitmapToRegion;

    pExaScr->SavedCreateScreenResources = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = exaCreateScreenResources;

#ifdef RENDER
    if (ps) {
        pExaScr->SavedComposite = ps->Composite;
	ps->Composite = exaComposite;

	if (pScreenInfo->PrepareComposite) {
	    pExaScr->SavedGlyphs = ps->Glyphs;
	    ps->Glyphs = exaGlyphs;
	}
	
	pExaScr->SavedTriangles = ps->Triangles;
	ps->Triangles = exaTriangles;

	pExaScr->SavedTrapezoids = ps->Trapezoids;
	ps->Trapezoids = exaTrapezoids;

	pExaScr->SavedAddTraps = ps->AddTraps;
	ps->AddTraps = ExaCheckAddTraps;
    }
#endif

#ifdef MITSHM
    /*
     * Don't allow shared pixmaps.
     */
    ShmRegisterFuncs(pScreen, &exaShmFuncs);
#endif
    /*
     * Hookup offscreen pixmaps
     */
    if (pExaScr->info->flags & EXA_OFFSCREEN_PIXMAPS)
    {
	if (!dixRequestPrivate(exaPixmapPrivateKey, sizeof(ExaPixmapPrivRec))) {
            LogMessage(X_WARNING,
		       "EXA(%d): Failed to allocate pixmap private\n",
		       pScreen->myNum);
	    return FALSE;
        }
        pExaScr->SavedCreatePixmap = pScreen->CreatePixmap;
	pScreen->CreatePixmap = exaCreatePixmap;

        pExaScr->SavedDestroyPixmap = pScreen->DestroyPixmap;
	pScreen->DestroyPixmap = exaDestroyPixmap;

	pExaScr->SavedModifyPixmapHeader = pScreen->ModifyPixmapHeader;
	pScreen->ModifyPixmapHeader = exaModifyPixmapHeader;
	if (!pExaScr->info->CreatePixmap) {
	    LogMessage(X_INFO, "EXA(%d): Offscreen pixmap area of %lu bytes\n",
		       pScreen->myNum,
		       pExaScr->info->memorySize - pExaScr->info->offScreenBase);
	} else {
	    LogMessage(X_INFO, "EXA(%d): Driver allocated offscreen pixmaps\n",
		       pScreen->myNum);

	}
    }
    else
        LogMessage(X_INFO, "EXA(%d): No offscreen pixmaps\n", pScreen->myNum);

    if (!pExaScr->info->CreatePixmap) {
	DBG_PIXMAP(("============== %ld < %ld\n", pExaScr->info->offScreenBase,
		    pExaScr->info->memorySize));
	if (pExaScr->info->offScreenBase < pExaScr->info->memorySize) {
	    if (!exaOffscreenInit (pScreen)) {
		LogMessage(X_WARNING, "EXA(%d): Offscreen pixmap setup failed\n",
			   pScreen->myNum);
		return FALSE;
	    }
	}
    }

    if (ps->Glyphs == exaGlyphs)
	exaGlyphsInit(pScreen);

    LogMessage(X_INFO, "EXA(%d): Driver registered support for the following"
	       " operations:\n", pScreen->myNum);
    assert(pScreenInfo->PrepareSolid != NULL);
    LogMessage(X_INFO, "        Solid\n");
    assert(pScreenInfo->PrepareCopy != NULL);
    LogMessage(X_INFO, "        Copy\n");
    if (pScreenInfo->PrepareComposite != NULL) {
	LogMessage(X_INFO, "        Composite (RENDER acceleration)\n");
    }
    if (pScreenInfo->UploadToScreen != NULL) {
	LogMessage(X_INFO, "        UploadToScreen\n");
    }
    if (pScreenInfo->DownloadFromScreen != NULL) {
	LogMessage(X_INFO, "        DownloadFromScreen\n");
    }

    return TRUE;
}

/**
 * exaDriverFini tears down EXA on a given screen.
 *
 * @param pScreen screen being torn down.
 */
void
exaDriverFini (ScreenPtr pScreen)
{
    /*right now does nothing*/
}

/**
 * exaMarkSync() should be called after any asynchronous drawing by the hardware.
 *
 * @param pScreen screen which drawing occurred on
 *
 * exaMarkSync() sets a flag to indicate that some asynchronous drawing has
 * happened and a WaitSync() will be necessary before relying on the contents of
 * offscreen memory from the CPU's perspective.  It also calls an optional
 * driver MarkSync() callback, the return value of which may be used to do partial
 * synchronization with the hardware in the future.
 */
void exaMarkSync(ScreenPtr pScreen)
{
    ExaScreenPriv(pScreen);

    pExaScr->info->needsSync = TRUE;
    if (pExaScr->info->MarkSync != NULL) {
        pExaScr->info->lastMarker = (*pExaScr->info->MarkSync)(pScreen);
    }
}

/**
 * exaWaitSync() ensures that all drawing has been completed.
 *
 * @param pScreen screen being synchronized.
 *
 * Calls down into the driver to ensure that all previous drawing has completed.
 * It should always be called before relying on the framebuffer contents
 * reflecting previous drawing, from a CPU perspective.
 */
void exaWaitSync(ScreenPtr pScreen)
{
    ExaScreenPriv(pScreen);

    if (pExaScr->info->needsSync && !pExaScr->swappedOut) {
        (*pExaScr->info->WaitMarker)(pScreen, pExaScr->info->lastMarker);
        pExaScr->info->needsSync = FALSE;
    }
}
