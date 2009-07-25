/*
 *
 * Copyright © 1999 Keith Packard
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

#include "exa_priv.h"

#ifdef RENDER
#include "mipict.h"
#endif

/*
 * These functions wrap the low-level fb rendering functions and
 * synchronize framebuffer/accelerated drawing by stalling until
 * the accelerator is idle
 */

/**
 * Calls exaPrepareAccess with EXA_PREPARE_SRC for the tile, if that is the
 * current fill style.
 *
 * Solid doesn't use an extra pixmap source, and Stippled/OpaqueStippled are
 * 1bpp and never in fb, so we don't worry about them.
 * We should worry about them for completeness sake and going forward.
 */
void
exaPrepareAccessGC(GCPtr pGC)
{
    if (pGC->stipple)
        exaPrepareAccess(&pGC->stipple->drawable, EXA_PREPARE_MASK);
    if (pGC->fillStyle == FillTiled)
	exaPrepareAccess(&pGC->tile.pixmap->drawable, EXA_PREPARE_SRC);
}

/**
 * Finishes access to the tile in the GC, if used.
 */
void
exaFinishAccessGC(GCPtr pGC)
{
    if (pGC->fillStyle == FillTiled)
	exaFinishAccess(&pGC->tile.pixmap->drawable, EXA_PREPARE_SRC);
    if (pGC->stipple)
        exaFinishAccess(&pGC->stipple->drawable, EXA_PREPARE_MASK);
}

#if DEBUG_TRACE_FALL
char
exaDrawableLocation(DrawablePtr pDrawable)
{
    return exaDrawableIsOffscreen(pDrawable) ? 's' : 'm';
}
#endif /* DEBUG_TRACE_FALL */

void
ExaCheckFillSpans (DrawablePtr pDrawable, GCPtr pGC, int nspans,
		   DDXPointPtr ppt, int *pwidth, int fSorted)
{
    EXA_FALLBACK(("to %p (%c)\n", pDrawable, exaDrawableLocation(pDrawable)));
    exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
    exaPrepareAccessGC (pGC);
    fbFillSpans (pDrawable, pGC, nspans, ppt, pwidth, fSorted);
    exaFinishAccessGC (pGC);
    exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
}

void
ExaCheckSetSpans (DrawablePtr pDrawable, GCPtr pGC, char *psrc,
		 DDXPointPtr ppt, int *pwidth, int nspans, int fSorted)
{
    EXA_FALLBACK(("to %p (%c)\n", pDrawable, exaDrawableLocation(pDrawable)));
    exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
    fbSetSpans (pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
    exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
}

void
ExaCheckPutImage (DrawablePtr pDrawable, GCPtr pGC, int depth,
		 int x, int y, int w, int h, int leftPad, int format,
		 char *bits)
{
    ExaPixmapPriv(exaGetDrawablePixmap(pDrawable));

    EXA_FALLBACK(("to %p (%c)\n", pDrawable, exaDrawableLocation(pDrawable)));
    if (exaGCReadsDestination(pDrawable, pGC->planemask, pGC->fillStyle,
			      pGC->alu, pGC->clientClipType))
	exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
    else
	exaPrepareAccessReg (pDrawable, EXA_PREPARE_DEST, pExaPixmap->pDamage ?
			     DamagePendingRegion(pExaPixmap->pDamage) : NULL);
    fbPutImage (pDrawable, pGC, depth, x, y, w, h, leftPad, format, bits);
    exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
}

RegionPtr
ExaCheckCopyArea (DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
		 int srcx, int srcy, int w, int h, int dstx, int dsty)
{
    RegionPtr ret;

    EXA_FALLBACK(("from %p to %p (%c,%c)\n", pSrc, pDst,
		  exaDrawableLocation(pSrc), exaDrawableLocation(pDst)));
    exaPrepareAccess (pDst, EXA_PREPARE_DEST);
    exaPrepareAccess (pSrc, EXA_PREPARE_SRC);
    ret = fbCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
    exaFinishAccess (pSrc, EXA_PREPARE_SRC);
    exaFinishAccess (pDst, EXA_PREPARE_DEST);

    return ret;
}

RegionPtr
ExaCheckCopyPlane (DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
		  int srcx, int srcy, int w, int h, int dstx, int dsty,
		  unsigned long bitPlane)
{
    RegionPtr ret;

    EXA_FALLBACK(("from %p to %p (%c,%c)\n", pSrc, pDst,
		  exaDrawableLocation(pSrc), exaDrawableLocation(pDst)));
    exaPrepareAccess (pDst, EXA_PREPARE_DEST);
    exaPrepareAccess (pSrc, EXA_PREPARE_SRC);
    ret = fbCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty,
		       bitPlane);
    exaFinishAccess (pSrc, EXA_PREPARE_SRC);
    exaFinishAccess (pDst, EXA_PREPARE_DEST);

    return ret;
}

void
ExaCheckPolyPoint (DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
		  DDXPointPtr pptInit)
{
    EXA_FALLBACK(("to %p (%c)\n", pDrawable, exaDrawableLocation(pDrawable)));
    exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
    fbPolyPoint (pDrawable, pGC, mode, npt, pptInit);
    exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
}

void
ExaCheckPolylines (DrawablePtr pDrawable, GCPtr pGC,
		  int mode, int npt, DDXPointPtr ppt)
{
    EXA_FALLBACK(("to %p (%c), width %d, mode %d, count %d\n",
		  pDrawable, exaDrawableLocation(pDrawable),
		  pGC->lineWidth, mode, npt));

    if (pGC->lineWidth == 0) {
	exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
	exaPrepareAccessGC (pGC);
	fbPolyLine (pDrawable, pGC, mode, npt, ppt);
	exaFinishAccessGC (pGC);
	exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
	return;
    }
    /* fb calls mi functions in the lineWidth != 0 case. */
    fbPolyLine (pDrawable, pGC, mode, npt, ppt);
}

void
ExaCheckPolySegment (DrawablePtr pDrawable, GCPtr pGC,
		    int nsegInit, xSegment *pSegInit)
{
    EXA_FALLBACK(("to %p (%c) width %d, count %d\n", pDrawable,
		  exaDrawableLocation(pDrawable), pGC->lineWidth, nsegInit));
    if (pGC->lineWidth == 0) {
	exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
	exaPrepareAccessGC (pGC);
	fbPolySegment (pDrawable, pGC, nsegInit, pSegInit);
	exaFinishAccessGC (pGC);
	exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
	return;
    }
    /* fb calls mi functions in the lineWidth != 0 case. */
    fbPolySegment (pDrawable, pGC, nsegInit, pSegInit);
}

void
ExaCheckPolyArc (DrawablePtr pDrawable, GCPtr pGC,
		int narcs, xArc *pArcs)
{
    EXA_FALLBACK(("to %p (%c)\n", pDrawable, exaDrawableLocation(pDrawable)));

    /* Disable this as fbPolyArc can call miZeroPolyArc which in turn
     * can call accelerated functions, that as yet, haven't been notified
     * with exaFinishAccess().
     */
#if 0
    if (pGC->lineWidth == 0)
    {
	exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
	exaPrepareAccessGC (pGC);
	fbPolyArc (pDrawable, pGC, narcs, pArcs);
	exaFinishAccessGC (pGC);
	exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
	return;
    }
#endif
    miPolyArc (pDrawable, pGC, narcs, pArcs);
}

void
ExaCheckPolyFillRect (DrawablePtr pDrawable, GCPtr pGC,
		     int nrect, xRectangle *prect)
{
    EXA_FALLBACK(("to %p (%c)\n", pDrawable, exaDrawableLocation(pDrawable)));

    exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
    exaPrepareAccessGC (pGC);
    fbPolyFillRect (pDrawable, pGC, nrect, prect);
    exaFinishAccessGC (pGC);
    exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
}

void
ExaCheckImageGlyphBlt (DrawablePtr pDrawable, GCPtr pGC,
		      int x, int y, unsigned int nglyph,
		      CharInfoPtr *ppci, pointer pglyphBase)
{
    EXA_FALLBACK(("to %p (%c)\n", pDrawable,
		  exaDrawableLocation(pDrawable)));
    exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
    exaPrepareAccessGC (pGC);
    fbImageGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
    exaFinishAccessGC (pGC);
    exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
}

void
ExaCheckPolyGlyphBlt (DrawablePtr pDrawable, GCPtr pGC,
		     int x, int y, unsigned int nglyph,
		     CharInfoPtr *ppci, pointer pglyphBase)
{
    EXA_FALLBACK(("to %p (%c), style %d alu %d\n", pDrawable,
		  exaDrawableLocation(pDrawable), pGC->fillStyle, pGC->alu));
    exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
    exaPrepareAccessGC (pGC);
    fbPolyGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
    exaFinishAccessGC (pGC);
    exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
}

void
ExaCheckPushPixels (GCPtr pGC, PixmapPtr pBitmap,
		   DrawablePtr pDrawable,
		   int w, int h, int x, int y)
{
    EXA_FALLBACK(("from %p to %p (%c,%c)\n", pBitmap, pDrawable,
		  exaDrawableLocation(&pBitmap->drawable),
		  exaDrawableLocation(pDrawable)));
    exaPrepareAccess (pDrawable, EXA_PREPARE_DEST);
    exaPrepareAccess (&pBitmap->drawable, EXA_PREPARE_SRC);
    exaPrepareAccessGC (pGC);
    fbPushPixels (pGC, pBitmap, pDrawable, w, h, x, y);
    exaFinishAccessGC (pGC);
    exaFinishAccess (&pBitmap->drawable, EXA_PREPARE_SRC);
    exaFinishAccess (pDrawable, EXA_PREPARE_DEST);
}

void
ExaCheckGetSpans (DrawablePtr pDrawable,
		 int wMax,
		 DDXPointPtr ppt,
		 int *pwidth,
		 int nspans,
		 char *pdstStart)
{
    EXA_FALLBACK(("from %p (%c)\n", pDrawable, exaDrawableLocation(pDrawable)));
    exaPrepareAccess (pDrawable, EXA_PREPARE_SRC);
    fbGetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
    exaFinishAccess (pDrawable, EXA_PREPARE_SRC);
}

void
ExaCheckComposite (CARD8      op,
                   PicturePtr pSrc,
                   PicturePtr pMask,
                   PicturePtr pDst,
                   INT16      xSrc,
                   INT16      ySrc,
                   INT16      xMask,
                   INT16      yMask,
                   INT16      xDst,
                   INT16      yDst,
                   CARD16     width,
                   CARD16     height)
{
    RegionRec region;
    int xoff, yoff;

    REGION_NULL(pScreen, &region);

    /* We need to prepare access to any separate alpha maps first, in case the
     * driver doesn't support EXA_PREPARE_AUX*, in which case EXA_PREPARE_SRC
     * may be used for moving them out.
     */
    if (pSrc->alphaMap && pSrc->alphaMap->pDrawable)
	exaPrepareAccess(pSrc->alphaMap->pDrawable, EXA_PREPARE_AUX2);
    if (pMask && pMask->alphaMap && pMask->alphaMap->pDrawable)
	exaPrepareAccess(pMask->alphaMap->pDrawable, EXA_PREPARE_AUX1);

    if (!exaOpReadsDestination(op)) {
	if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
				       xSrc, ySrc, xMask, yMask, xDst, yDst,
				       width, height))
	    return;

	exaGetDrawableDeltas (pDst->pDrawable,
			      exaGetDrawablePixmap(pDst->pDrawable),
			      &xoff, &yoff);

	REGION_TRANSLATE(pScreen, &region, xoff, yoff);

	if (pDst->alphaMap && pDst->alphaMap->pDrawable)
	    exaPrepareAccessReg(pDst->alphaMap->pDrawable, EXA_PREPARE_AUX0,
				&region);

	exaPrepareAccessReg (pDst->pDrawable, EXA_PREPARE_DEST, &region);
    } else {
	if (pDst->alphaMap && pDst->alphaMap->pDrawable)
	    exaPrepareAccess(pDst->alphaMap->pDrawable, EXA_PREPARE_AUX0);

	exaPrepareAccess (pDst->pDrawable, EXA_PREPARE_DEST);
    }

    EXA_FALLBACK(("from picts %p/%p to pict %p\n",
		 pSrc, pMask, pDst));

    if (pSrc->pDrawable != NULL)
	exaPrepareAccess (pSrc->pDrawable, EXA_PREPARE_SRC);
    if (pMask && pMask->pDrawable != NULL)
	exaPrepareAccess (pMask->pDrawable, EXA_PREPARE_MASK);
    fbComposite (op,
                 pSrc,
                 pMask,
                 pDst,
                 xSrc,
                 ySrc,
                 xMask,
                 yMask,
                 xDst,
                 yDst,
                 width,
                 height);
    if (pMask && pMask->pDrawable != NULL)
	exaFinishAccess (pMask->pDrawable, EXA_PREPARE_MASK);
    if (pMask && pMask->alphaMap && pMask->alphaMap->pDrawable)
	exaFinishAccess(pMask->alphaMap->pDrawable, EXA_PREPARE_AUX1);
    if (pSrc->pDrawable != NULL)
	exaFinishAccess (pSrc->pDrawable, EXA_PREPARE_SRC);
    if (pSrc->alphaMap && pSrc->alphaMap->pDrawable)
	exaFinishAccess(pSrc->alphaMap->pDrawable, EXA_PREPARE_AUX2);
    exaFinishAccess (pDst->pDrawable, EXA_PREPARE_DEST);
    if (pDst->alphaMap && pDst->alphaMap->pDrawable)
	exaFinishAccess(pDst->alphaMap->pDrawable, EXA_PREPARE_AUX0);

    REGION_UNINIT(pScreen, &region);
}

void
ExaCheckAddTraps (PicturePtr	pPicture,
		  INT16		x_off,
		  INT16		y_off,
		  int		ntrap,
		  xTrap		*traps)
{
    EXA_FALLBACK(("to pict %p (%c)\n",
		  exaDrawableLocation(pPicture->pDrawable)));
    exaPrepareAccess(pPicture->pDrawable, EXA_PREPARE_DEST);
    fbAddTraps (pPicture, x_off, y_off, ntrap, traps);
    exaFinishAccess(pPicture->pDrawable, EXA_PREPARE_DEST);
}

/**
 * Gets the 0,0 pixel of a pixmap.  Used for doing solid fills of tiled pixmaps
 * that happen to be 1x1.  Pixmap must be at least 8bpp.
 *
 * XXX This really belongs in fb, so it can be aware of tiling and etc.
 */
CARD32
exaGetPixmapFirstPixel (PixmapPtr pPixmap)
{
    CARD32 pixel;
    void *fb;
    Bool need_finish = FALSE;
    BoxRec box;
    RegionRec migration;
    ExaPixmapPriv (pPixmap);
    Bool sys_valid = pExaPixmap->pDamage &&
	!miPointInRegion(&pExaPixmap->validSys, 0, 0,  &box);
    Bool damaged = pExaPixmap->pDamage &&
 	miPointInRegion(DamageRegion(pExaPixmap->pDamage), 0, 0, &box);
    Bool offscreen = exaPixmapIsOffscreen(pPixmap);

    fb = pExaPixmap->sys_ptr;

    /* Try to avoid framebuffer readbacks */
    if ((!offscreen && !sys_valid && !damaged) ||
	(offscreen && (!sys_valid || damaged)))
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = 1;
	box.y2 = 1;
	REGION_INIT(pScreen, &migration, &box, 1);

	need_finish = TRUE;

	exaPrepareAccessReg(&pPixmap->drawable, EXA_PREPARE_SRC, &migration);
	fb = pPixmap->devPrivate.ptr;
    }

    switch (pPixmap->drawable.bitsPerPixel) {
    case 32:
	pixel = *(CARD32 *)fb;
	break;
    case 16:
	pixel = *(CARD16 *)fb;
	break;
    default:
	pixel = *(CARD8 *)fb;
	break;
    }

    if (need_finish) {
	exaFinishAccess(&pPixmap->drawable, EXA_PREPARE_SRC);
	REGION_UNINIT(pScreen, &migration);
    }

    return pixel;
}
