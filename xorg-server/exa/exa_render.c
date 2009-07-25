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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>

#include "exa_priv.h"

#ifdef RENDER
#include "mipict.h"

#if DEBUG_TRACE_FALL
static void exaCompositeFallbackPictDesc(PicturePtr pict, char *string, int n)
{
    char format[20];
    char size[20];
    char loc;
    int temp;

    if (!pict) {
	snprintf(string, n, "None");
	return;
    }

    switch (pict->format)
    {
    case PICT_a8r8g8b8:
	snprintf(format, 20, "ARGB8888");
	break;
    case PICT_x8r8g8b8:
	snprintf(format, 20, "XRGB8888");
	break;
    case PICT_r5g6b5:
	snprintf(format, 20, "RGB565  ");
	break;
    case PICT_x1r5g5b5:
	snprintf(format, 20, "RGB555  ");
	break;
    case PICT_a8:
	snprintf(format, 20, "A8      ");
	break;
    case PICT_a1:
	snprintf(format, 20, "A1      ");
	break;
    default:
	snprintf(format, 20, "0x%x", (int)pict->format);
	break;
    }

    loc = exaGetOffscreenPixmap(pict->pDrawable, &temp, &temp) ? 's' : 'm';

    snprintf(size, 20, "%dx%d%s", pict->pDrawable->width,
	     pict->pDrawable->height, pict->repeat ?
	     " R" : "");

    snprintf(string, n, "%p:%c fmt %s (%s)", pict->pDrawable, loc, format, size);
}

static void
exaPrintCompositeFallback(CARD8 op,
			  PicturePtr pSrc,
			  PicturePtr pMask,
			  PicturePtr pDst)
{
    char sop[20];
    char srcdesc[40], maskdesc[40], dstdesc[40];

    switch(op)
    {
    case PictOpSrc:
	sprintf(sop, "Src");
	break;
    case PictOpOver:
	sprintf(sop, "Over");
	break;
    default:
	sprintf(sop, "0x%x", (int)op);
	break;
    }

    exaCompositeFallbackPictDesc(pSrc, srcdesc, 40);
    exaCompositeFallbackPictDesc(pMask, maskdesc, 40);
    exaCompositeFallbackPictDesc(pDst, dstdesc, 40);

    ErrorF("Composite fallback: op %s, \n"
	   "                    src  %s, \n"
	   "                    mask %s, \n"
	   "                    dst  %s, \n",
	   sop, srcdesc, maskdesc, dstdesc);
}
#endif /* DEBUG_TRACE_FALL */

Bool
exaOpReadsDestination (CARD8 op)
{
    /* FALSE (does not read destination) is the list of ops in the protocol
     * document with "0" in the "Fb" column and no "Ab" in the "Fa" column.
     * That's just Clear and Src.  ReduceCompositeOp() will already have
     * converted con/disjoint clear/src to Clear or Src.
     */
    switch (op) {
    case PictOpClear:
    case PictOpSrc:
	return FALSE;
    default:
	return TRUE;
    }
}


static Bool
exaGetPixelFromRGBA(CARD32	*pixel,
		    CARD16	red,
		    CARD16	green,
		    CARD16	blue,
		    CARD16	alpha,
		    CARD32	format)
{
    int rbits, bbits, gbits, abits;
    int rshift, bshift, gshift, ashift;

    *pixel = 0;

    if (!PICT_FORMAT_COLOR(format))
	return FALSE;

    rbits = PICT_FORMAT_R(format);
    gbits = PICT_FORMAT_G(format);
    bbits = PICT_FORMAT_B(format);
    abits = PICT_FORMAT_A(format);

    if (PICT_FORMAT_TYPE(format) == PICT_TYPE_ARGB) {
	bshift = 0;
	gshift = bbits;
	rshift = gshift + gbits;
	ashift = rshift + rbits;
    } else {  /* PICT_TYPE_ABGR */
	rshift = 0;
	gshift = rbits;
	bshift = gshift + gbits;
	ashift = bshift + bbits;
    }

    *pixel |=  ( blue >> (16 - bbits)) << bshift;
    *pixel |=  (  red >> (16 - rbits)) << rshift;
    *pixel |=  (green >> (16 - gbits)) << gshift;
    *pixel |=  (alpha >> (16 - abits)) << ashift;

    return TRUE;
}

static Bool
exaGetRGBAFromPixel(CARD32	pixel,
		    CARD16	*red,
		    CARD16	*green,
		    CARD16	*blue,
		    CARD16	*alpha,
		    CARD32	format)
{
    int rbits, bbits, gbits, abits;
    int rshift, bshift, gshift, ashift;

    if (!PICT_FORMAT_COLOR(format))
	return FALSE;

    rbits = PICT_FORMAT_R(format);
    gbits = PICT_FORMAT_G(format);
    bbits = PICT_FORMAT_B(format);
    abits = PICT_FORMAT_A(format);

    if (PICT_FORMAT_TYPE(format) == PICT_TYPE_ARGB) {
	bshift = 0;
	gshift = bbits;
	rshift = gshift + gbits;
	ashift = rshift + rbits;
    } else {  /* PICT_TYPE_ABGR */
	rshift = 0;
	gshift = rbits;
	bshift = gshift + gbits;
	ashift = bshift + bbits;
    }

    *red = ((pixel >> rshift ) & ((1 << rbits) - 1)) << (16 - rbits);
    while (rbits < 16) {
	*red |= *red >> rbits;
	rbits <<= 1;
    }

    *green = ((pixel >> gshift ) & ((1 << gbits) - 1)) << (16 - gbits);
    while (gbits < 16) {
	*green |= *green >> gbits;
	gbits <<= 1;
    }

    *blue = ((pixel >> bshift ) & ((1 << bbits) - 1)) << (16 - bbits);
    while (bbits < 16) {
	*blue |= *blue >> bbits;
	bbits <<= 1;
    }

    if (abits) {
	*alpha = ((pixel >> ashift ) & ((1 << abits) - 1)) << (16 - abits);
	while (abits < 16) {
	    *alpha |= *alpha >> abits;
	    abits <<= 1;
	}
    } else
	*alpha = 0xffff;

    return TRUE;
}

static int
exaTryDriverSolidFill(PicturePtr	pSrc,
		      PicturePtr	pDst,
		      INT16		xSrc,
		      INT16		ySrc,
		      INT16		xDst,
		      INT16		yDst,
		      CARD16		width,
		      CARD16		height)
{
    ExaScreenPriv (pDst->pDrawable->pScreen);
    RegionRec region;
    BoxPtr pbox;
    int nbox;
    int dst_off_x, dst_off_y;
    PixmapPtr pSrcPix, pDstPix;
    ExaPixmapPrivPtr pSrcExaPix, pDstExaPix;
    CARD32 pixel;
    CARD16 red, green, blue, alpha;
    ExaMigrationRec pixmaps[1];

    pDstPix = exaGetDrawablePixmap (pDst->pDrawable);
    pSrcPix = exaGetDrawablePixmap (pSrc->pDrawable);

    pSrcExaPix = ExaGetPixmapPriv(pSrcPix);
    pDstExaPix = ExaGetPixmapPriv(pDstPix);

    /* Check whether the accelerator can use these pixmaps.
     */
    if (pSrcExaPix->accel_blocked || pDstExaPix->accel_blocked)
    {
	return -1;
    }

    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;
    xSrc += pSrc->pDrawable->x;
    ySrc += pSrc->pDrawable->y;

    if (!miComputeCompositeRegion (&region, pSrc, NULL, pDst,
				   xSrc, ySrc, 0, 0, xDst, yDst,
				   width, height))
	return 1;

    exaGetDrawableDeltas (pDst->pDrawable, pDstPix, &dst_off_x, &dst_off_y);

    REGION_TRANSLATE(pScreen, &region, dst_off_x, dst_off_y);

    pixel = exaGetPixmapFirstPixel (pSrcPix);

    pixmaps[0].as_dst = TRUE;
    pixmaps[0].as_src = FALSE;
    pixmaps[0].pPix = pDstPix;
    pixmaps[0].pReg = &region;
    exaDoMigration(pixmaps, 1, TRUE);

    if (!exaPixmapIsOffscreen(pDstPix)) {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return 0;
    }

    if (!exaGetRGBAFromPixel(pixel, &red, &green, &blue, &alpha,
			 pSrc->format))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }

    if (!exaGetPixelFromRGBA(&pixel, red, green, blue, alpha,
			pDst->format))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }

    if (!(*pExaScr->info->PrepareSolid) (pDstPix, GXcopy, 0xffffffff, pixel))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }

    nbox = REGION_NUM_RECTS(&region);
    pbox = REGION_RECTS(&region);

    while (nbox--)
    {
	(*pExaScr->info->Solid) (pDstPix, pbox->x1, pbox->y1, pbox->x2, pbox->y2);
	pbox++;
    }

    (*pExaScr->info->DoneSolid) (pDstPix);
    exaMarkSync(pDst->pDrawable->pScreen);

    REGION_UNINIT(pDst->pDrawable->pScreen, &region);
    return 1;
}

static int
exaTryDriverCompositeRects(CARD8	       op,
			   PicturePtr	       pSrc,
			   PicturePtr	       pDst,
			   int                 nrect,
			   ExaCompositeRectPtr rects)
{
    ExaScreenPriv (pDst->pDrawable->pScreen);
    int src_off_x, src_off_y, dst_off_x, dst_off_y;
    PixmapPtr pSrcPix, pDstPix;
    ExaPixmapPrivPtr pSrcExaPix, pDstExaPix;
    struct _Pixmap scratch;
    ExaMigrationRec pixmaps[2];

    if (!pExaScr->info->PrepareComposite)
	return -1;

    pSrcPix = exaGetDrawablePixmap(pSrc->pDrawable);
    pSrcExaPix = ExaGetPixmapPriv(pSrcPix);

    pDstPix = exaGetDrawablePixmap(pDst->pDrawable);
    pDstExaPix = ExaGetPixmapPriv(pDstPix);

    /* Check whether the accelerator can use these pixmaps.
     * FIXME: If it cannot, use temporary pixmaps so that the drawing
     * happens within limits.
     */
    if (pSrcExaPix->accel_blocked ||
	pDstExaPix->accel_blocked)
    {
	return -1;
    }

    if (pExaScr->info->CheckComposite &&
	!(*pExaScr->info->CheckComposite) (op, pSrc, NULL, pDst))
    {
	return -1;
    }
    
    exaGetDrawableDeltas (pDst->pDrawable, pDstPix, &dst_off_x, &dst_off_y);

    pixmaps[0].as_dst = TRUE;
    pixmaps[0].as_src = exaOpReadsDestination(op);
    pixmaps[0].pPix = pDstPix;
    pixmaps[0].pReg = NULL;
    pixmaps[1].as_dst = FALSE;
    pixmaps[1].as_src = TRUE;
    pixmaps[1].pPix = pSrcPix;
    pixmaps[1].pReg = NULL;
    exaDoMigration(pixmaps, 2, TRUE);

    pSrcPix = exaGetOffscreenPixmap (pSrc->pDrawable, &src_off_x, &src_off_y);
    if (!exaPixmapIsOffscreen(pDstPix))
	return 0;
    
    if (!pSrcPix && pExaScr->info->UploadToScratch)
    {
	pSrcPix = exaGetDrawablePixmap (pSrc->pDrawable);
	if ((*pExaScr->info->UploadToScratch) (pSrcPix, &scratch))
	    pSrcPix = &scratch;
    }

    if (!pSrcPix)
	return 0;

    if (!(*pExaScr->info->PrepareComposite) (op, pSrc, NULL, pDst, pSrcPix,
					     NULL, pDstPix))
	return -1;

    while (nrect--)
    {
	INT16 xDst = rects->xDst + pDst->pDrawable->x;
	INT16 yDst = rects->yDst + pDst->pDrawable->y;
	INT16 xSrc = rects->xSrc + pSrc->pDrawable->x;
	INT16 ySrc = rects->ySrc + pSrc->pDrawable->y;

	RegionRec region;
	BoxPtr pbox;
	int nbox;

	if (!miComputeCompositeRegion (&region, pSrc, NULL, pDst,
				       xSrc, ySrc, 0, 0, xDst, yDst,
				       rects->width, rects->height))
	    goto next_rect;

	REGION_TRANSLATE(pScreen, &region, dst_off_x, dst_off_y);

	nbox = REGION_NUM_RECTS(&region);
	pbox = REGION_RECTS(&region);

	xSrc = xSrc + src_off_x - xDst - dst_off_x;
	ySrc = ySrc + src_off_y - yDst - dst_off_y;

	while (nbox--)
	{
	    (*pExaScr->info->Composite) (pDstPix,
					 pbox->x1 + xSrc,
					 pbox->y1 + ySrc,
					 0, 0,
					 pbox->x1,
					 pbox->y1,
					 pbox->x2 - pbox->x1,
					 pbox->y2 - pbox->y1);
	    pbox++;
	}

    next_rect:
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);

	rects++;
    }

    (*pExaScr->info->DoneComposite) (pDstPix);
    exaMarkSync(pDst->pDrawable->pScreen);

    return 1;
}

/**
 * Copy a number of rectangles from source to destination in a single
 * operation. This is specialized for building a glyph mask: we don'y
 * have a mask argument because we don't need it for that, and we
 * don't have he special-case fallbacks found in exaComposite() - if the
 * driver can support it, we use the driver functionality, otherwise we
 * fallback straight to software.
 */
void
exaCompositeRects(CARD8	              op,
		  PicturePtr	      pSrc,
		  PicturePtr	      pDst,
		  int                 nrect,
		  ExaCompositeRectPtr rects)
{
    PixmapPtr pPixmap = exaGetDrawablePixmap(pDst->pDrawable);
    ExaPixmapPriv(pPixmap);
    int n;
    ExaCompositeRectPtr r;
    
    if (pExaPixmap->pDamage) {
	RegionRec region;
	int x1 = MAXSHORT;
	int y1 = MAXSHORT;
	int x2 = MINSHORT;
	int y2 = MINSHORT;
	BoxRec box;
    
	/* We have to manage the damage ourselves, since CompositeRects isn't
	 * something in the screen that can be managed by the damage extension,
	 * and EXA depends on damage to track what needs to be migrated between
	 * offscreen and onscreen.
	 */

	/* Compute the overall extents of the composited region - we're making
	 * the assumption here that we are compositing a bunch of glyphs that
	 * cluster closely together and damaging each glyph individually would
	 * be a loss compared to damaging the bounding box.
	 */
	n = nrect;
	r = rects;
	while (n--) {
	    int rect_x2 = r->xDst + r->width;
	    int rect_y2 = r->yDst + r->height;

	    if (r->xDst < x1) x1 = r->xDst;
	    if (r->yDst < y1) y1 = r->yDst;
	    if (rect_x2 > x2) x2 = rect_x2;
	    if (rect_y2 > y2) y2 = rect_y2;

	    r++;
	}

	if (x2 <= x1 || y2 <= y1)
	    return;

	box.x1 = x1;
	box.x2 = x2 < MAXSHORT ? x2 : MAXSHORT;
	box.y1 = y1;
	box.y2 = y2 < MAXSHORT ? y2 : MAXSHORT;

 	/* The pixmap migration code relies on pendingDamage indicating
	 * the bounds of the current rendering, so we need to force 
	 * the actual damage into that region before we do anything, and
	 * (see use of DamagePendingRegion in exaCopyDirty)
	 */

	REGION_INIT(pScreen, &region, &box, 1);
    
	DamageRegionAppend(pDst->pDrawable, &region);

	REGION_UNINIT(pScreen, &region);
    }
    
    /************************************************************/
    
    ValidatePicture (pSrc);
    ValidatePicture (pDst);
    
    if (exaTryDriverCompositeRects(op, pSrc, pDst, nrect, rects) != 1) {
	n = nrect;
	r = rects;
	while (n--) {
	    ExaCheckComposite (op, pSrc, NULL, pDst,
			       r->xSrc, r->ySrc,
			       0, 0,
			       r->xDst, r->yDst,
			       r->width, r->height);
	    r++;
	}
    }
    
    /************************************************************/

    if (pExaPixmap->pDamage) {
	/* Now we have to flush the damage out from pendingDamage => damage 
	 * Calling DamageRegionProcessPending has that effect.
	 */

	DamageRegionProcessPending(pDst->pDrawable);
    }
}

static int
exaTryDriverComposite(CARD8		op,
		      PicturePtr	pSrc,
		      PicturePtr	pMask,
		      PicturePtr	pDst,
		      INT16		xSrc,
		      INT16		ySrc,
		      INT16		xMask,
		      INT16		yMask,
		      INT16		xDst,
		      INT16		yDst,
		      CARD16		width,
		      CARD16		height)
{
    ExaScreenPriv (pDst->pDrawable->pScreen);
    RegionRec region;
    BoxPtr pbox;
    int nbox;
    int src_off_x, src_off_y, mask_off_x, mask_off_y, dst_off_x, dst_off_y;
    PixmapPtr pSrcPix, pMaskPix = NULL, pDstPix;
    ExaPixmapPrivPtr pSrcExaPix, pMaskExaPix = NULL, pDstExaPix;
    struct _Pixmap scratch;
    ExaMigrationRec pixmaps[3];

    pSrcPix = exaGetDrawablePixmap(pSrc->pDrawable);
    pSrcExaPix = ExaGetPixmapPriv(pSrcPix);

    pDstPix = exaGetDrawablePixmap(pDst->pDrawable);
    pDstExaPix = ExaGetPixmapPriv(pDstPix);

    if (pMask) {
	pMaskPix = exaGetDrawablePixmap(pMask->pDrawable);
        pMaskExaPix = ExaGetPixmapPriv(pMaskPix);
    }

    /* Check whether the accelerator can use these pixmaps.
     * FIXME: If it cannot, use temporary pixmaps so that the drawing
     * happens within limits.
     */
    if (pSrcExaPix->accel_blocked ||
	pDstExaPix->accel_blocked ||
	(pMask && (pMaskExaPix->accel_blocked)))
    {
	return -1;
    }

    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;

    if (pMask) {
	xMask += pMask->pDrawable->x;
	yMask += pMask->pDrawable->y;
    }

    xSrc += pSrc->pDrawable->x;
    ySrc += pSrc->pDrawable->y;

    if (pExaScr->info->CheckComposite &&
	!(*pExaScr->info->CheckComposite) (op, pSrc, pMask, pDst))
    {
	return -1;
    }

    if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
				   xSrc, ySrc, xMask, yMask, xDst, yDst,
				   width, height))
	return 1;

    exaGetDrawableDeltas (pDst->pDrawable, pDstPix, &dst_off_x, &dst_off_y);

    REGION_TRANSLATE(pScreen, &region, dst_off_x, dst_off_y);

    pixmaps[0].as_dst = TRUE;
    pixmaps[0].as_src = exaOpReadsDestination(op);
    pixmaps[0].pPix = pDstPix;
    pixmaps[0].pReg = pixmaps[0].as_src ? NULL : &region;
    pixmaps[1].as_dst = FALSE;
    pixmaps[1].as_src = TRUE;
    pixmaps[1].pPix = pSrcPix;
    pixmaps[1].pReg = NULL;
    if (pMask) {
	pixmaps[2].as_dst = FALSE;
	pixmaps[2].as_src = TRUE;
	pixmaps[2].pPix = pMaskPix;
	pixmaps[2].pReg = NULL;
	exaDoMigration(pixmaps, 3, TRUE);
    } else {
	exaDoMigration(pixmaps, 2, TRUE);
    }

    pSrcPix = exaGetOffscreenPixmap (pSrc->pDrawable, &src_off_x, &src_off_y);
    if (pMask)
	pMaskPix = exaGetOffscreenPixmap (pMask->pDrawable, &mask_off_x,
					  &mask_off_y);

    if (!exaPixmapIsOffscreen(pDstPix)) {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return 0;
    }

    if (!pSrcPix && (!pMask || pMaskPix) && pExaScr->info->UploadToScratch) {
	pSrcPix = exaGetDrawablePixmap (pSrc->pDrawable);
	if ((*pExaScr->info->UploadToScratch) (pSrcPix, &scratch))
	    pSrcPix = &scratch;
    } else if (pSrcPix && pMask && !pMaskPix && pExaScr->info->UploadToScratch) {
	pMaskPix = exaGetDrawablePixmap (pMask->pDrawable);
	if ((*pExaScr->info->UploadToScratch) (pMaskPix, &scratch))
	    pMaskPix = &scratch;
    }

    if (!pSrcPix || (pMask && !pMaskPix)) {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return 0;
    }

    if (!(*pExaScr->info->PrepareComposite) (op, pSrc, pMask, pDst, pSrcPix,
					     pMaskPix, pDstPix))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }

    nbox = REGION_NUM_RECTS(&region);
    pbox = REGION_RECTS(&region);

    xMask = xMask + mask_off_x - xDst - dst_off_x;
    yMask = yMask + mask_off_y - yDst - dst_off_y;

    xSrc = xSrc + src_off_x - xDst - dst_off_x;
    ySrc = ySrc + src_off_y - yDst - dst_off_y;

    while (nbox--)
    {
	(*pExaScr->info->Composite) (pDstPix,
				     pbox->x1 + xSrc,
				     pbox->y1 + ySrc,
				     pbox->x1 + xMask,
				     pbox->y1 + yMask,
				     pbox->x1,
				     pbox->y1,
				     pbox->x2 - pbox->x1,
				     pbox->y2 - pbox->y1);
	pbox++;
    }
    (*pExaScr->info->DoneComposite) (pDstPix);
    exaMarkSync(pDst->pDrawable->pScreen);

    REGION_UNINIT(pDst->pDrawable->pScreen, &region);
    return 1;
}

/**
 * exaTryMagicTwoPassCompositeHelper implements PictOpOver using two passes of
 * simpler operations PictOpOutReverse and PictOpAdd. Mainly used for component
 * alpha and limited 1-tmu cards.
 *
 * From http://anholt.livejournal.com/32058.html:
 *
 * The trouble is that component-alpha rendering requires two different sources
 * for blending: one for the source value to the blender, which is the
 * per-channel multiplication of source and mask, and one for the source alpha
 * for multiplying with the destination channels, which is the multiplication
 * of the source channels by the mask alpha. So the equation for Over is:
 *
 * dst.A = src.A * mask.A + (1 - (src.A * mask.A)) * dst.A
 * dst.R = src.R * mask.R + (1 - (src.A * mask.R)) * dst.R
 * dst.G = src.G * mask.G + (1 - (src.A * mask.G)) * dst.G
 * dst.B = src.B * mask.B + (1 - (src.A * mask.B)) * dst.B
 *
 * But we can do some simpler operations, right? How about PictOpOutReverse,
 * which has a source factor of 0 and dest factor of (1 - source alpha). We
 * can get the source alpha value (srca.X = src.A * mask.X) out of the texture
 * blenders pretty easily. So we can do a component-alpha OutReverse, which
 * gets us:
 *
 * dst.A = 0 + (1 - (src.A * mask.A)) * dst.A
 * dst.R = 0 + (1 - (src.A * mask.R)) * dst.R
 * dst.G = 0 + (1 - (src.A * mask.G)) * dst.G
 * dst.B = 0 + (1 - (src.A * mask.B)) * dst.B
 *
 * OK. And if an op doesn't use the source alpha value for the destination
 * factor, then we can do the channel multiplication in the texture blenders
 * to get the source value, and ignore the source alpha that we wouldn't use.
 * We've supported this in the Radeon driver for a long time. An example would
 * be PictOpAdd, which does:
 *
 * dst.A = src.A * mask.A + dst.A
 * dst.R = src.R * mask.R + dst.R
 * dst.G = src.G * mask.G + dst.G
 * dst.B = src.B * mask.B + dst.B
 *
 * Hey, this looks good! If we do a PictOpOutReverse and then a PictOpAdd right
 * after it, we get:
 *
 * dst.A = src.A * mask.A + ((1 - (src.A * mask.A)) * dst.A)
 * dst.R = src.R * mask.R + ((1 - (src.A * mask.R)) * dst.R)
 * dst.G = src.G * mask.G + ((1 - (src.A * mask.G)) * dst.G)
 * dst.B = src.B * mask.B + ((1 - (src.A * mask.B)) * dst.B)
 */

static int
exaTryMagicTwoPassCompositeHelper(CARD8 op,
				  PicturePtr pSrc,
				  PicturePtr pMask,
				  PicturePtr pDst,
				  INT16 xSrc,
				  INT16 ySrc,
				  INT16 xMask,
				  INT16 yMask,
				  INT16 xDst,
				  INT16 yDst,
				  CARD16 width,
				  CARD16 height)
{
    ExaScreenPriv (pDst->pDrawable->pScreen);

    assert(op == PictOpOver);

    if (pExaScr->info->CheckComposite &&
	(!(*pExaScr->info->CheckComposite)(PictOpOutReverse, pSrc, pMask,
					   pDst) ||
	 !(*pExaScr->info->CheckComposite)(PictOpAdd, pSrc, pMask, pDst)))
    {
	return -1;
    }

    /* Now, we think we should be able to accelerate this operation. First,
     * composite the destination to be the destination times the source alpha
     * factors.
     */
    exaComposite(PictOpOutReverse, pSrc, pMask, pDst, xSrc, ySrc, xMask, yMask,
		 xDst, yDst, width, height);

    /* Then, add in the source value times the destination alpha factors (1.0).
     */
    exaComposite(PictOpAdd, pSrc, pMask, pDst, xSrc, ySrc, xMask, yMask,
		 xDst, yDst, width, height);

    return 1;
}

void
exaComposite(CARD8	op,
	     PicturePtr pSrc,
	     PicturePtr pMask,
	     PicturePtr pDst,
	     INT16	xSrc,
	     INT16	ySrc,
	     INT16	xMask,
	     INT16	yMask,
	     INT16	xDst,
	     INT16	yDst,
	     CARD16	width,
	     CARD16	height)
{
    ExaScreenPriv (pDst->pDrawable->pScreen);
    int ret = -1;
    Bool saveSrcRepeat = pSrc->repeat;
    Bool saveMaskRepeat = pMask ? pMask->repeat : 0;
    RegionRec region;

    /* We currently don't support acceleration of gradients, or other pictures
     * with a NULL pDrawable.
     */
    if (pExaScr->swappedOut ||
	pSrc->pDrawable == NULL || (pMask != NULL && pMask->pDrawable == NULL))
    {
	goto fallback;
    }

    /* Remove repeat in source if useless */
    if (pSrc->repeat && !pSrc->transform && xSrc >= 0 &&
	(xSrc + width) <= pSrc->pDrawable->width && ySrc >= 0 &&
	(ySrc + height) <= pSrc->pDrawable->height)
	    pSrc->repeat = 0;

    if (!pMask)
    {
      if ((op == PictOpSrc &&
	   ((pSrc->format == pDst->format) ||
	    (pSrc->format==PICT_a8r8g8b8 && pDst->format==PICT_x8r8g8b8) ||
	    (pSrc->format==PICT_a8b8g8r8 && pDst->format==PICT_x8b8g8r8))) ||
	  (op == PictOpOver && !pSrc->alphaMap && !pDst->alphaMap &&
	   pSrc->format == pDst->format &&
	   (pSrc->format==PICT_x8r8g8b8 || pSrc->format==PICT_x8b8g8r8)))
	{
	    if (pSrc->pDrawable->width == 1 &&
		pSrc->pDrawable->height == 1 &&
		pSrc->repeat)
	    {
		ret = exaTryDriverSolidFill(pSrc, pDst, xSrc, ySrc, xDst, yDst,
					    width, height);
		if (ret == 1)
		    goto done;
	    }
	    else if (pSrc->pDrawable != NULL &&
		     !pSrc->repeat &&
		     !pSrc->transform)
	    {
		xDst += pDst->pDrawable->x;
		yDst += pDst->pDrawable->y;
		xSrc += pSrc->pDrawable->x;
		ySrc += pSrc->pDrawable->y;

		if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
					       xSrc, ySrc, xMask, yMask, xDst,
					       yDst, width, height))
		    goto done;


		exaCopyNtoN (pSrc->pDrawable, pDst->pDrawable, NULL,
			     REGION_RECTS(&region), REGION_NUM_RECTS(&region),
			     xSrc - xDst, ySrc - yDst,
			     FALSE, FALSE, 0, NULL);
		REGION_UNINIT(pDst->pDrawable->pScreen, &region);
		goto done;
	    }
	    else if (pSrc->pDrawable != NULL &&
		     pSrc->pDrawable->type == DRAWABLE_PIXMAP &&
		     !pSrc->transform &&
		     pSrc->repeatType == RepeatNormal)
	    {
		DDXPointRec patOrg;

		/* Let's see if the driver can do the repeat in one go */
		if (pExaScr->info->PrepareComposite && !pSrc->alphaMap &&
		    !pDst->alphaMap)
		{
		    ret = exaTryDriverComposite(op, pSrc, pMask, pDst, xSrc,
						ySrc, xMask, yMask, xDst, yDst,
						width, height);
		    if (ret == 1)
			goto done;
		}

		/* Now see if we can use exaFillRegionTiled() */
		xDst += pDst->pDrawable->x;
		yDst += pDst->pDrawable->y;
		xSrc += pSrc->pDrawable->x;
		ySrc += pSrc->pDrawable->y;

		if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst, xSrc,
					       ySrc, xMask, yMask, xDst, yDst,
					       width, height))
		    goto done;

		/* pattern origin is the point in the destination drawable
		 * corresponding to (0,0) in the source */
		patOrg.x = xDst - xSrc;
		patOrg.y = yDst - ySrc;

		ret = exaFillRegionTiled(pDst->pDrawable, &region,
					 (PixmapPtr)pSrc->pDrawable,
					 &patOrg, FB_ALLONES, GXcopy, CT_NONE);

		REGION_UNINIT(pDst->pDrawable->pScreen, &region);

		if (ret)
		    goto done;

		/* Let's be correct and restore the variables to their original state. */
		xDst -= pDst->pDrawable->x;
		yDst -= pDst->pDrawable->y;
		xSrc -= pSrc->pDrawable->x;
		ySrc -= pSrc->pDrawable->y;
	    }
	}
    }

    /* Remove repeat in mask if useless */
    if (pMask && pMask->repeat && !pMask->transform && xMask >= 0 &&
	(xMask + width) <= pMask->pDrawable->width && yMask >= 0 &&
	(yMask + height) <= pMask->pDrawable->height)
	    pMask->repeat = 0;

    if (pExaScr->info->PrepareComposite &&
	!pSrc->alphaMap && (!pMask || !pMask->alphaMap) && !pDst->alphaMap)
    {
	Bool isSrcSolid;

	ret = exaTryDriverComposite(op, pSrc, pMask, pDst, xSrc, ySrc, xMask,
				    yMask, xDst, yDst, width, height);
	if (ret == 1)
	    goto done;

	/* For generic masks and solid src pictures, mach64 can do Over in two
	 * passes, similar to the component-alpha case.
	 */
	isSrcSolid = pSrc->pDrawable->width == 1 &&
		     pSrc->pDrawable->height == 1 &&
		     pSrc->repeat;

	/* If we couldn't do the Composite in a single pass, and it was a
	 * component-alpha Over, see if we can do it in two passes with
	 * an OutReverse and then an Add.
	 */
	if (ret == -1 && op == PictOpOver && pMask &&
	    (pMask->componentAlpha || isSrcSolid)) {
	    ret = exaTryMagicTwoPassCompositeHelper(op, pSrc, pMask, pDst,
						    xSrc, ySrc,
						    xMask, yMask, xDst, yDst,
						    width, height);
	    if (ret == 1)
		goto done;
	}
    }

fallback:
#if DEBUG_TRACE_FALL
    exaPrintCompositeFallback (op, pSrc, pMask, pDst);
#endif

    ExaCheckComposite (op, pSrc, pMask, pDst, xSrc, ySrc,
		      xMask, yMask, xDst, yDst, width, height);

done:
    pSrc->repeat = saveSrcRepeat;
    if (pMask)
	pMask->repeat = saveMaskRepeat;
}
#endif

/**
 * Same as miCreateAlphaPicture, except it uses ExaCheckPolyFillRect instead
 * of PolyFillRect to initialize the pixmap after creating it, to prevent
 * the pixmap from being migrated.
 *
 * See the comments about exaTrapezoids and exaTriangles.
 */
static PicturePtr
exaCreateAlphaPicture (ScreenPtr     pScreen,
                       PicturePtr    pDst,
                       PictFormatPtr pPictFormat,
                       CARD16        width,
                       CARD16        height)
{
    PixmapPtr	    pPixmap;
    PicturePtr	    pPicture;
    GCPtr	    pGC;
    int		    error;
    xRectangle	    rect;

    if (width > 32767 || height > 32767)
	return 0;

    if (!pPictFormat)
    {
	if (pDst->polyEdge == PolyEdgeSharp)
	    pPictFormat = PictureMatchFormat (pScreen, 1, PICT_a1);
	else
	    pPictFormat = PictureMatchFormat (pScreen, 8, PICT_a8);
	if (!pPictFormat)
	    return 0;
    }

    pPixmap = (*pScreen->CreatePixmap) (pScreen, width, height,
					pPictFormat->depth, 0);
    if (!pPixmap)
	return 0;
    pGC = GetScratchGC (pPixmap->drawable.depth, pScreen);
    if (!pGC)
    {
	(*pScreen->DestroyPixmap) (pPixmap);
	return 0;
    }
    ValidateGC (&pPixmap->drawable, pGC);
    rect.x = 0;
    rect.y = 0;
    rect.width = width;
    rect.height = height;
    ExaCheckPolyFillRect (&pPixmap->drawable, pGC, 1, &rect);
    exaPixmapDirty (pPixmap, 0, 0, width, height);
    FreeScratchGC (pGC);
    pPicture = CreatePicture (0, &pPixmap->drawable, pPictFormat,
			      0, 0, serverClient, &error);
    (*pScreen->DestroyPixmap) (pPixmap);
    return pPicture;
}

/**
 * exaTrapezoids is essentially a copy of miTrapezoids that uses
 * exaCreateAlphaPicture instead of miCreateAlphaPicture.
 *
 * The problem with miCreateAlphaPicture is that it calls PolyFillRect
 * to initialize the contents after creating the pixmap, which
 * causes the pixmap to be moved in for acceleration. The subsequent
 * call to RasterizeTrapezoid won't be accelerated however, which
 * forces the pixmap to be moved out again.
 *
 * exaCreateAlphaPicture avoids this roundtrip by using ExaCheckPolyFillRect
 * to initialize the contents.
 */
void
exaTrapezoids (CARD8 op, PicturePtr pSrc, PicturePtr pDst,
               PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc,
               int ntrap, xTrapezoid *traps)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    BoxRec		bounds;

    if (maskFormat) {
	miTrapezoidBounds (ntrap, traps, &bounds);

	if (bounds.y1 >= bounds.y2 || bounds.x1 >= bounds.x2)
	    return;

	PicturePtr	pPicture;
	INT16		xDst, yDst;
	INT16		xRel, yRel;

	xDst = traps[0].left.p1.x >> 16;
	yDst = traps[0].left.p1.y >> 16;

	pPicture = exaCreateAlphaPicture (pScreen, pDst, maskFormat,
	                                  bounds.x2 - bounds.x1,
	                                  bounds.y2 - bounds.y1);
	if (!pPicture)
	    return;

	exaPrepareAccess(pPicture->pDrawable, EXA_PREPARE_DEST);
	for (; ntrap; ntrap--, traps++)
	    (*ps->RasterizeTrapezoid) (pPicture, traps,
				       -bounds.x1, -bounds.y1);
	exaFinishAccess(pPicture->pDrawable, EXA_PREPARE_DEST);

	xRel = bounds.x1 + xSrc - xDst;
	yRel = bounds.y1 + ySrc - yDst;
	CompositePicture (op, pSrc, pPicture, pDst,
			  xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			  bounds.x2 - bounds.x1,
			  bounds.y2 - bounds.y1);
	FreePicture (pPicture, 0);
    } else {
	if (pDst->polyEdge == PolyEdgeSharp)
	    maskFormat = PictureMatchFormat (pScreen, 1, PICT_a1);
	else
	    maskFormat = PictureMatchFormat (pScreen, 8, PICT_a8);
	for (; ntrap; ntrap--, traps++)
	    exaTrapezoids (op, pSrc, pDst, maskFormat, xSrc, ySrc, 1, traps);
    }
}

/**
 * exaTriangles is essentially a copy of miTriangles that uses
 * exaCreateAlphaPicture instead of miCreateAlphaPicture.
 *
 * The problem with miCreateAlphaPicture is that it calls PolyFillRect
 * to initialize the contents after creating the pixmap, which
 * causes the pixmap to be moved in for acceleration. The subsequent
 * call to AddTriangles won't be accelerated however, which forces the pixmap
 * to be moved out again.
 *
 * exaCreateAlphaPicture avoids this roundtrip by using ExaCheckPolyFillRect
 * to initialize the contents.
 */
void
exaTriangles (CARD8 op, PicturePtr pSrc, PicturePtr pDst,
	      PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc,
	      int ntri, xTriangle *tris)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    BoxRec		bounds;

    if (maskFormat) {
	miTriangleBounds (ntri, tris, &bounds);

	if (bounds.y1 >= bounds.y2 || bounds.x1 >= bounds.x2)
	    return;

	PicturePtr	pPicture;
	INT16		xDst, yDst;
	INT16		xRel, yRel;

	xDst = tris[0].p1.x >> 16;
	yDst = tris[0].p1.y >> 16;

	pPicture = exaCreateAlphaPicture (pScreen, pDst, maskFormat,
					  bounds.x2 - bounds.x1,
					  bounds.y2 - bounds.y1);
	if (!pPicture)
	    return;

	exaPrepareAccess(pPicture->pDrawable, EXA_PREPARE_DEST);
	(*ps->AddTriangles) (pPicture, -bounds.x1, -bounds.y1, ntri, tris);
	exaFinishAccess(pPicture->pDrawable, EXA_PREPARE_DEST);

	xRel = bounds.x1 + xSrc - xDst;
	yRel = bounds.y1 + ySrc - yDst;
	CompositePicture (op, pSrc, pPicture, pDst,
			  xRel, yRel, 0, 0, bounds.x1, bounds.y1,
			  bounds.x2 - bounds.x1, bounds.y2 - bounds.y1);
	FreePicture (pPicture, 0);
    } else {
	if (pDst->polyEdge == PolyEdgeSharp)
	    maskFormat = PictureMatchFormat (pScreen, 1, PICT_a1);
	else
	    maskFormat = PictureMatchFormat (pScreen, 8, PICT_a8);

	for (; ntri; ntri--, tris++)
	    exaTriangles (op, pSrc, pDst, maskFormat, xSrc, ySrc, 1, tris);
    }
}
