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
static int exaGCPrivateKeyIndex;
DevPrivateKey exaGCPrivateKey = &exaGCPrivateKeyIndex;

#ifdef MITSHM
static ShmFuncs exaShmFuncs = { NULL, NULL };
#endif

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
    ExaPixmapPriv (pPix);

    return (CARD8 *)pExaPixmap->fb_ptr - pExaScr->info->memoryBase;
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
    BoxRec box;
    RegionRec region;

    box.x1 = max(x1, 0);
    box.y1 = max(y1, 0);
    box.x2 = min(x2, pPix->drawable.width);
    box.y2 = min(y2, pPix->drawable.height);

    if (box.x1 >= box.x2 || box.y1 >= box.y2)
	return;

    REGION_INIT(pScreen, &region, &box, 1);
    DamageRegionAppend(&pPix->drawable, &region);
    DamageRegionProcessPending(&pPix->drawable);
    REGION_UNINIT(pScreen, &region);
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

void
exaSetAccelBlock(ExaScreenPrivPtr pExaScr, ExaPixmapPrivPtr pExaPixmap,
                 int w, int h, int bpp)
{
    pExaPixmap->accel_blocked = 0;

    if (pExaScr->info->maxPitchPixels) {
        int max_pitch = pExaScr->info->maxPitchPixels * bits_to_bytes(bpp);

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

void
exaSetFbPitch(ExaScreenPrivPtr pExaScr, ExaPixmapPrivPtr pExaPixmap,
              int w, int h, int bpp)
{
    if (pExaScr->info->flags & EXA_OFFSCREEN_ALIGN_POT && w != 1)
        pExaPixmap->fb_pitch = bits_to_bytes((1 << (exaLog2(w - 1) + 1)) * bpp);
    else
        pExaPixmap->fb_pitch = bits_to_bytes(w * bpp);

    pExaPixmap->fb_pitch = EXA_ALIGN(pExaPixmap->fb_pitch,
                                     pExaScr->info->pixmapPitchAlign);
}

/**
 * Returns TRUE if the pixmap is not movable.  This is the case where it's a
 * pixmap which has no private (almost always bad) or it's a scratch pixmap created by
 * some X Server internal component (the score says it's pinned).
 */
Bool
exaPixmapIsPinned (PixmapPtr pPix)
{
    ExaPixmapPriv (pPix);

    if (pExaPixmap == NULL)
	EXA_FatalErrorDebugWithRet(("EXA bug: exaPixmapIsPinned was called on a non-exa pixmap.\n"), TRUE);

    return pExaPixmap->score == EXA_PIXMAP_SCORE_PINNED;
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
exaPixmapIsOffscreen(PixmapPtr pPixmap)
{
    ScreenPtr	pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);

    if (!(pExaScr->info->flags & EXA_OFFSCREEN_PIXMAPS))
	return FALSE;

    return (*pExaScr->pixmap_is_offscreen)(pPixmap);
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

/**
 * Returns TRUE if pixmap can be accessed offscreen.
 */
Bool
ExaDoPrepareAccess(PixmapPtr pPixmap, int index)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv (pScreen);
    ExaPixmapPriv(pPixmap);
    Bool offscreen;
    int i;

    if (!(pExaScr->info->flags & EXA_OFFSCREEN_PIXMAPS))
	return FALSE;

    if (pExaPixmap == NULL)
	EXA_FatalErrorDebugWithRet(("EXA bug: ExaDoPrepareAccess was called on a non-exa pixmap.\n"), FALSE);

    /* Handle repeated / nested calls. */
    for (i = 0; i < EXA_NUM_PREPARE_INDICES; i++) {
	if (pExaScr->access[i].pixmap == pPixmap) {
	    pExaScr->access[i].count++;
	    return TRUE;
	}
    }

    /* If slot for this index is taken, find an empty slot */
    if (pExaScr->access[index].pixmap) {
	for (index = EXA_NUM_PREPARE_INDICES - 1; index >= 0; index--)
	    if (!pExaScr->access[index].pixmap)
		break;
    }

    /* Access to this pixmap hasn't been prepared yet, so data pointer should be NULL. */
    if (pPixmap->devPrivate.ptr != NULL) {
	EXA_FatalErrorDebug(("EXA bug: pPixmap->devPrivate.ptr was %p, but should have been NULL.\n",
			     pPixmap->devPrivate.ptr));
    }

    offscreen = exaPixmapIsOffscreen(pPixmap);

    if (offscreen && pExaPixmap->fb_ptr)
	pPixmap->devPrivate.ptr = pExaPixmap->fb_ptr;
    else
	pPixmap->devPrivate.ptr = pExaPixmap->sys_ptr;

    /* Store so we can handle repeated / nested calls. */
    pExaScr->access[index].pixmap = pPixmap;
    pExaScr->access[index].count = 1;

    if (!offscreen)
	return FALSE;

    exaWaitSync (pScreen);

    if (pExaScr->info->PrepareAccess == NULL)
	return TRUE;

    if (index >= EXA_PREPARE_AUX_DEST &&
	!(pExaScr->info->flags & EXA_SUPPORTS_PREPARE_AUX)) {
	if (pExaPixmap->score == EXA_PIXMAP_SCORE_PINNED)
	    FatalError("Unsupported AUX indices used on a pinned pixmap.\n");
	exaMoveOutPixmap (pPixmap);
	return FALSE;
    }

    if (!(*pExaScr->info->PrepareAccess) (pPixmap, index)) {
	if (pExaPixmap->score == EXA_PIXMAP_SCORE_PINNED &&
	    !(pExaScr->info->flags & EXA_MIXED_PIXMAPS))
	    FatalError("Driver failed PrepareAccess on a pinned pixmap.\n");
	exaMoveOutPixmap (pPixmap);

	return FALSE;
    }

    return TRUE;
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
    PixmapPtr pPixmap = exaGetDrawablePixmap(pDrawable);
    ExaScreenPriv(pDrawable->pScreen);

    if (pExaScr->prepare_access_reg)
	pExaScr->prepare_access_reg(pPixmap, index, NULL);
    else
	(void)ExaDoPrepareAccess(pPixmap, index);
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
    int i;

    if (!(pExaScr->info->flags & EXA_OFFSCREEN_PIXMAPS))
	return;

    if (pExaPixmap == NULL)
	EXA_FatalErrorDebugWithRet(("EXA bug: exaFinishAccesss was called on a non-exa pixmap.\n"),);

    /* Handle repeated / nested calls. */
    for (i = 0; i < EXA_NUM_PREPARE_INDICES; i++) {
	if (pExaScr->access[i].pixmap == pPixmap) {
	    if (--pExaScr->access[i].count > 0)
		return;
	    break;
	}
    }

    /* Catch unbalanced Prepare/FinishAccess calls. */
    if (i == EXA_NUM_PREPARE_INDICES)
	EXA_FatalErrorDebug(("EXA bug: FinishAccess called without PrepareAccess for pixmap 0x%p.\n",
			     pPixmap));

    pExaScr->access[i].pixmap = NULL;

    /* We always hide the devPrivate.ptr. */
    pPixmap->devPrivate.ptr = NULL;

    if (pExaScr->finish_access)
	pExaScr->finish_access(pPixmap, index);

    if (!pExaScr->info->FinishAccess || !exaPixmapIsOffscreen(pPixmap))
	return;

    if (i >= EXA_PREPARE_AUX_DEST &&
	!(pExaScr->info->flags & EXA_SUPPORTS_PREPARE_AUX)) {
	ErrorF("EXA bug: Trying to call driver FinishAccess hook with "
	       "unsupported index EXA_PREPARE_AUX*\n");
	return;
    }

    (*pExaScr->info->FinishAccess) (pPixmap, i);
}

/**
 * Here begins EXA's GC code.
 * Do not ever access the fb/mi layer directly.
 */

static void
exaValidateGC(GCPtr pGC,
		unsigned long changes,
		DrawablePtr pDrawable);

static void
exaDestroyGC(GCPtr pGC);

static void
exaChangeGC (GCPtr pGC,
		unsigned long mask);

static void
exaCopyGC (GCPtr pGCSrc,
	      unsigned long mask,
	      GCPtr	 pGCDst);

static void
exaChangeClip (GCPtr pGC,
		int type,
		pointer pvalue,
		int nrects);

static void
exaCopyClip(GCPtr pGCDst, GCPtr pGCSrc);

static void
exaCopyClip(GCPtr pGCDst, GCPtr pGCSrc);

static void
exaDestroyClip(GCPtr pGC);

const GCFuncs exaGCFuncs = {
    exaValidateGC,
    exaChangeGC,
    exaCopyGC,
    exaDestroyGC,
    exaChangeClip,
    exaDestroyClip,
    exaCopyClip
};

/*
 * This wrapper exists to allow fbValidateGC to work.
 * Note that we no longer assume newly created pixmaps to be in normal ram.
 * This assumption is certainly not garuanteed with driver allocated pixmaps.
 */
static PixmapPtr
exaCreatePixmapWithPrepare(ScreenPtr pScreen, int w, int h, int depth,
		unsigned usage_hint)
{
    PixmapPtr pPixmap;
    ExaScreenPriv(pScreen);

    /* This swaps between this function and the real upper layer function.
     * Normally this would swap to the fb layer pointer, this is a very special case.
     */
    swap(pExaScr, pScreen, CreatePixmap);
    pPixmap = pScreen->CreatePixmap(pScreen, w, h, depth, usage_hint);
    swap(pExaScr, pScreen, CreatePixmap);

    if (!pPixmap)
	return NULL;

    /* Note the usage of ExaDoPrepareAccess, this allowed because:
     * The pixmap is new, so not offscreen in the classic exa case.
     * For EXA_HANDLES_PIXMAPS the driver will handle whatever is needed.
     * We want to signal that the pixmaps will be used as destination.
     */
    ExaDoPrepareAccess(pPixmap, EXA_PREPARE_AUX_DEST);

    return pPixmap;
}

static Bool
exaDestroyPixmapWithFinish(PixmapPtr pPixmap)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);
    Bool ret;

    exaFinishAccess(&pPixmap->drawable, EXA_PREPARE_AUX_DEST);

    /* This swaps between this function and the real upper layer function.
     * Normally this would swap to the fb layer pointer, this is a very special case.
     */
    swap(pExaScr, pScreen, DestroyPixmap);
    ret = pScreen->DestroyPixmap(pPixmap);
    swap(pExaScr, pScreen, DestroyPixmap);

    return ret;
}

static void
exaValidateGC(GCPtr pGC,
		unsigned long changes,
		DrawablePtr pDrawable)
{
    /* fbValidateGC will do direct access to pixmaps if the tiling has changed.
     * Do a few smart things so fbValidateGC can do it's work.
     */

    ScreenPtr pScreen = pDrawable->pScreen;
    ExaScreenPriv(pScreen);
    CreatePixmapProcPtr old_ptr = NULL;
    DestroyPixmapProcPtr old_ptr2 = NULL;
    PixmapPtr pTile = NULL;
    EXA_GC_PROLOGUE(pGC);

    /* save the "fb" pointer. */
    old_ptr = pExaScr->SavedCreatePixmap;
    /* create a new upper layer pointer. */
    wrap(pExaScr, pScreen, CreatePixmap, exaCreatePixmapWithPrepare);

    /* save the "fb" pointer. */
    old_ptr2 = pExaScr->SavedDestroyPixmap;
    /* create a new upper layer pointer. */
    wrap(pExaScr, pScreen, DestroyPixmap, exaDestroyPixmapWithFinish);

    /* Either of these conditions is enough to trigger access to a tile pixmap. */
    /* With pGC->tileIsPixel == 1, you run the risk of dereferencing an invalid tile pixmap pointer. */
    if (pGC->fillStyle == FillTiled || ((changes & GCTile) && !pGC->tileIsPixel)) {
	pTile = pGC->tile.pixmap;

	/* Sometimes tile pixmaps are swapped, you need access to:
	 * - The current tile if it depth matches.
	 * - Or the rotated tile if that one matches depth and !(changes & GCTile).
	 * - Or the current tile pixmap and a newly created one.
	 */
	if (pTile && pTile->drawable.depth != pDrawable->depth && !(changes & GCTile)) {
	    PixmapPtr pRotatedTile = fbGetRotatedPixmap(pGC);
	    if (pRotatedTile->drawable.depth == pDrawable->depth)
		pTile = pRotatedTile;
	}
    }

    if (pGC->stipple)
        exaPrepareAccess(&pGC->stipple->drawable, EXA_PREPARE_MASK);
    if (pTile)
	exaPrepareAccess(&pTile->drawable, EXA_PREPARE_SRC);

    (*pGC->funcs->ValidateGC)(pGC, changes, pDrawable);

    if (pTile)
	exaFinishAccess(&pTile->drawable, EXA_PREPARE_SRC);
    if (pGC->stipple)
        exaFinishAccess(&pGC->stipple->drawable, EXA_PREPARE_MASK);

    /* switch back to the normal upper layer. */
    unwrap(pExaScr, pScreen, CreatePixmap);
    /* restore copy of fb layer pointer. */
    pExaScr->SavedCreatePixmap = old_ptr;

    /* switch back to the normal upper layer. */
    unwrap(pExaScr, pScreen, DestroyPixmap);
    /* restore copy of fb layer pointer. */
    pExaScr->SavedDestroyPixmap = old_ptr2;

    EXA_GC_EPILOGUE(pGC);
}

/* Is exaPrepareAccessGC() needed? */
static void
exaDestroyGC(GCPtr pGC)
{
    EXA_GC_PROLOGUE (pGC);
    (*pGC->funcs->DestroyGC)(pGC);
    EXA_GC_EPILOGUE (pGC);
}

static void
exaChangeGC (GCPtr pGC,
		unsigned long mask)
{
    EXA_GC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeGC) (pGC, mask);
    EXA_GC_EPILOGUE (pGC);
}

static void
exaCopyGC (GCPtr pGCSrc,
	      unsigned long mask,
	      GCPtr	 pGCDst)
{
    EXA_GC_PROLOGUE (pGCDst);
    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    EXA_GC_EPILOGUE (pGCDst);
}

static void
exaChangeClip (GCPtr pGC,
		int type,
		pointer pvalue,
		int nrects)
{
    EXA_GC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);
    EXA_GC_EPILOGUE (pGC);
}

static void
exaCopyClip(GCPtr pGCDst, GCPtr pGCSrc)
{
    EXA_GC_PROLOGUE (pGCDst);
    (*pGCDst->funcs->CopyClip)(pGCDst, pGCSrc);
    EXA_GC_EPILOGUE (pGCDst);
}

static void
exaDestroyClip(GCPtr pGC)
{
    EXA_GC_PROLOGUE (pGC);
    (*pGC->funcs->DestroyClip)(pGC);
    EXA_GC_EPILOGUE (pGC);
}

/**
 * exaCreateGC makes a new GC and hooks up its funcs handler, so that
 * exaValidateGC() will get called.
 */
static int
exaCreateGC (GCPtr pGC)
{
    ScreenPtr pScreen = pGC->pScreen;
    ExaScreenPriv(pScreen);
    ExaGCPriv(pGC);
    Bool ret;

    swap(pExaScr, pScreen, CreateGC);
    if ((ret = (*pScreen->CreateGC) (pGC))) {
	wrap(pExaGC, pGC, funcs, (GCFuncs *) &exaGCFuncs);
	wrap(pExaGC, pGC, ops, (GCOps *) &exaOps);
    }
    swap(pExaScr, pScreen, CreateGC);

    return ret;
}

static Bool
exaChangeWindowAttributes(WindowPtr pWin, unsigned long mask)
{
    Bool ret;
    ScreenPtr pScreen = pWin->drawable.pScreen;
    ExaScreenPriv(pScreen);
    CreatePixmapProcPtr old_ptr = NULL;
    DestroyPixmapProcPtr old_ptr2 = NULL;

    /* save the "fb" pointer. */
    old_ptr = pExaScr->SavedCreatePixmap;
    /* create a new upper layer pointer. */
    wrap(pExaScr, pScreen, CreatePixmap, exaCreatePixmapWithPrepare);

    /* save the "fb" pointer. */
    old_ptr2 = pExaScr->SavedDestroyPixmap;
    /* create a new upper layer pointer. */
    wrap(pExaScr, pScreen, DestroyPixmap, exaDestroyPixmapWithFinish);

    if ((mask & CWBackPixmap) && pWin->backgroundState == BackgroundPixmap) 
	exaPrepareAccess(&pWin->background.pixmap->drawable, EXA_PREPARE_SRC);

    if ((mask & CWBorderPixmap) && pWin->borderIsPixel == FALSE)
	exaPrepareAccess(&pWin->border.pixmap->drawable, EXA_PREPARE_MASK);

    swap(pExaScr, pScreen, ChangeWindowAttributes);
    ret = pScreen->ChangeWindowAttributes(pWin, mask);
    swap(pExaScr, pScreen, ChangeWindowAttributes);

    if ((mask & CWBackPixmap) && pWin->backgroundState == BackgroundPixmap) 
	exaFinishAccess(&pWin->background.pixmap->drawable, EXA_PREPARE_SRC);
    if ((mask & CWBorderPixmap) && pWin->borderIsPixel == FALSE)
	exaFinishAccess(&pWin->border.pixmap->drawable, EXA_PREPARE_MASK);

    /* switch back to the normal upper layer. */
    unwrap(pExaScr, pScreen, CreatePixmap);
    /* restore copy of fb layer pointer. */
    pExaScr->SavedCreatePixmap = old_ptr;

    /* switch back to the normal upper layer. */
    unwrap(pExaScr, pScreen, DestroyPixmap);
    /* restore copy of fb layer pointer. */
    pExaScr->SavedDestroyPixmap = old_ptr2;

    return ret;
}

static RegionPtr
exaBitmapToRegion(PixmapPtr pPix)
{
    RegionPtr ret;
    ScreenPtr pScreen = pPix->drawable.pScreen;
    ExaScreenPriv(pScreen);

    exaPrepareAccess(&pPix->drawable, EXA_PREPARE_SRC);
    swap(pExaScr, pScreen, BitmapToRegion);
    ret = pScreen->BitmapToRegion(pPix);
    swap(pExaScr, pScreen, BitmapToRegion);
    exaFinishAccess(&pPix->drawable, EXA_PREPARE_SRC);

    return ret;
}

static Bool
exaCreateScreenResources(ScreenPtr pScreen)
{
    ExaScreenPriv(pScreen);
    PixmapPtr pScreenPixmap;
    Bool b;

    swap(pExaScr, pScreen, CreateScreenResources);
    b = pScreen->CreateScreenResources(pScreen);
    swap(pExaScr, pScreen, CreateScreenResources);

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

static void
ExaBlockHandler(int screenNum, pointer blockData, pointer pTimeout,
		pointer pReadmask)
{
    ScreenPtr pScreen = screenInfo.screens[screenNum];
    ExaScreenPriv(pScreen);

    unwrap(pExaScr, pScreen, BlockHandler);
    (*pScreen->BlockHandler) (screenNum, blockData, pTimeout, pReadmask);
    wrap(pExaScr, pScreen, BlockHandler, ExaBlockHandler);

    /* Try and keep the offscreen memory area tidy every now and then (at most 
     * once per second) when the server has been idle for at least 100ms.
     */
    if (pExaScr->numOffscreenAvailable > 1) {
	CARD32 now = GetTimeInMillis();

	pExaScr->nextDefragment = now +
	    max(100, (INT32)(pExaScr->lastDefragment + 1000 - now));
	AdjustWaitForDelay(pTimeout, pExaScr->nextDefragment - now);
    }
}

static void
ExaWakeupHandler(int screenNum, pointer wakeupData, unsigned long result,
		 pointer pReadmask)
{
    ScreenPtr pScreen = screenInfo.screens[screenNum];
    ExaScreenPriv(pScreen);

    unwrap(pExaScr, pScreen, WakeupHandler);
    (*pScreen->WakeupHandler) (screenNum, wakeupData, result, pReadmask);
    wrap(pExaScr, pScreen, WakeupHandler, ExaWakeupHandler);

    if (result == 0 && pExaScr->numOffscreenAvailable > 1) {
	CARD32 now = GetTimeInMillis();

	if ((int)(now - pExaScr->nextDefragment) > 0) {
	    ExaOffscreenDefragment(pScreen);
	    pExaScr->lastDefragment = now;
	}
    }
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

    if (pScreen->BlockHandler == ExaBlockHandler)
	unwrap(pExaScr, pScreen, BlockHandler);
    if (pScreen->WakeupHandler == ExaWakeupHandler)
	unwrap(pExaScr, pScreen, WakeupHandler);
    unwrap(pExaScr, pScreen, CreateGC);
    unwrap(pExaScr, pScreen, CloseScreen);
    unwrap(pExaScr, pScreen, GetImage);
    unwrap(pExaScr, pScreen, GetSpans);
    if (pExaScr->SavedCreatePixmap)
	unwrap(pExaScr, pScreen, CreatePixmap);
    if (pExaScr->SavedDestroyPixmap)
	unwrap(pExaScr, pScreen, DestroyPixmap);
    if (pExaScr->SavedModifyPixmapHeader)
	unwrap(pExaScr, pScreen, ModifyPixmapHeader);
    unwrap(pExaScr, pScreen, CopyWindow);
    unwrap(pExaScr, pScreen, ChangeWindowAttributes);
    unwrap(pExaScr, pScreen, BitmapToRegion);
    unwrap(pExaScr, pScreen, CreateScreenResources);
#ifdef RENDER
    if (ps) {
	unwrap(pExaScr, ps, Composite);
	if (pExaScr->SavedGlyphs)
	    unwrap(pExaScr, ps, Glyphs);
	unwrap(pExaScr, ps, Trapezoids);
	unwrap(pExaScr, ps, Triangles);
	unwrap(pExaScr, ps, AddTraps);
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

    if (!pScreenInfo->CreatePixmap && !pScreenInfo->CreatePixmap2) {
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

    if (!dixRequestPrivate(exaGCPrivateKey, sizeof(ExaGCPrivRec))) {
	LogMessage(X_WARNING,
	       "EXA(%d): Failed to allocate GC private\n",
	       pScreen->myNum);
	return FALSE;
    }

    /*
     * Replace various fb screen functions
     */
    if ((pExaScr->info->flags & EXA_OFFSCREEN_PIXMAPS) &&
	!(pExaScr->info->flags & EXA_HANDLES_PIXMAPS)) {
	wrap(pExaScr, pScreen, BlockHandler, ExaBlockHandler);
	wrap(pExaScr, pScreen, WakeupHandler, ExaWakeupHandler);
    }
    wrap(pExaScr, pScreen, CreateGC, exaCreateGC);
    wrap(pExaScr, pScreen, CloseScreen, exaCloseScreen);
    wrap(pExaScr, pScreen, GetImage, exaGetImage);
    wrap(pExaScr, pScreen, GetSpans, ExaCheckGetSpans);
    wrap(pExaScr, pScreen, CopyWindow, exaCopyWindow);
    wrap(pExaScr, pScreen, ChangeWindowAttributes, exaChangeWindowAttributes);
    wrap(pExaScr, pScreen, BitmapToRegion, exaBitmapToRegion);
    wrap(pExaScr, pScreen, CreateScreenResources, exaCreateScreenResources);

#ifdef RENDER
    if (ps) {
	wrap(pExaScr, ps, Composite, exaComposite);
	if (pScreenInfo->PrepareComposite)
	    wrap(pExaScr, ps, Glyphs, exaGlyphs);
	wrap(pExaScr, ps, Trapezoids, exaTrapezoids);
	wrap(pExaScr, ps, Triangles, exaTriangles);
	wrap(pExaScr, ps, AddTraps, ExaCheckAddTraps);
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
	if (pExaScr->info->flags & EXA_HANDLES_PIXMAPS) {
	    if (pExaScr->info->flags & EXA_MIXED_PIXMAPS) {
		wrap(pExaScr, pScreen, CreatePixmap, exaCreatePixmap_mixed);
		wrap(pExaScr, pScreen, DestroyPixmap, exaDestroyPixmap_mixed);
		wrap(pExaScr, pScreen, ModifyPixmapHeader, exaModifyPixmapHeader_mixed);
		pExaScr->do_migration = exaDoMigration_mixed;
		pExaScr->pixmap_is_offscreen = exaPixmapIsOffscreen_mixed;
		pExaScr->do_move_in_pixmap = exaMoveInPixmap_mixed;
		pExaScr->do_move_out_pixmap = NULL;
		pExaScr->prepare_access_reg = exaPrepareAccessReg_mixed;
		pExaScr->finish_access = exaFinishAccess_mixed;
	    } else {
		wrap(pExaScr, pScreen, CreatePixmap, exaCreatePixmap_driver);
		wrap(pExaScr, pScreen, DestroyPixmap, exaDestroyPixmap_driver);
		wrap(pExaScr, pScreen, ModifyPixmapHeader, exaModifyPixmapHeader_driver);
		pExaScr->do_migration = NULL;
		pExaScr->pixmap_is_offscreen = exaPixmapIsOffscreen_driver;
		pExaScr->do_move_in_pixmap = NULL;
		pExaScr->do_move_out_pixmap = NULL;
		pExaScr->prepare_access_reg = NULL;
		pExaScr->finish_access = NULL;
	    }
	} else {
	    wrap(pExaScr, pScreen, CreatePixmap, exaCreatePixmap_classic);
	    wrap(pExaScr, pScreen, DestroyPixmap, exaDestroyPixmap_classic);
	    wrap(pExaScr, pScreen, ModifyPixmapHeader, exaModifyPixmapHeader_classic);
	    pExaScr->do_migration = exaDoMigration_classic;
	    pExaScr->pixmap_is_offscreen = exaPixmapIsOffscreen_classic;
	    pExaScr->do_move_in_pixmap = exaMoveInPixmap_classic;
	    pExaScr->do_move_out_pixmap = exaMoveOutPixmap_classic;
	    pExaScr->prepare_access_reg = exaPrepareAccessReg_classic;
	    pExaScr->finish_access = NULL;
	}
	if (!(pExaScr->info->flags & EXA_HANDLES_PIXMAPS)) {
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

    if (!(pExaScr->info->flags & EXA_HANDLES_PIXMAPS)) {
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

/**
 * Performs migration of the pixmaps according to the operation information
 * provided in pixmaps and can_accel and the migration scheme chosen in the
 * config file.
 */
void
exaDoMigration (ExaMigrationPtr pixmaps, int npixmaps, Bool can_accel)
{
    ScreenPtr pScreen = pixmaps[0].pPix->drawable.pScreen;
    ExaScreenPriv(pScreen);

    if (!(pExaScr->info->flags & EXA_OFFSCREEN_PIXMAPS))
	return;

    if (pExaScr->do_migration)
	(*pExaScr->do_migration)(pixmaps, npixmaps, can_accel);
}

void
exaMoveInPixmap (PixmapPtr pPixmap)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);

    if (!(pExaScr->info->flags & EXA_OFFSCREEN_PIXMAPS))
	return;

    if (pExaScr->do_move_in_pixmap)
	(*pExaScr->do_move_in_pixmap)(pPixmap);
}

void
exaMoveOutPixmap (PixmapPtr pPixmap)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);

    if (!(pExaScr->info->flags & EXA_OFFSCREEN_PIXMAPS))
	return;

    if (pExaScr->do_move_out_pixmap)
	(*pExaScr->do_move_out_pixmap)(pPixmap);
}
