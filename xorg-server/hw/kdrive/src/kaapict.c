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

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "kaa.h"

#ifdef RENDER
#include "mipict.h"

#define KAA_DEBUG_FALLBACKS 0

#if KAA_DEBUG_FALLBACKS
static void kaaCompositeFallbackPictDesc(PicturePtr pict, char *string, int n)
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

    loc = kaaGetOffscreenPixmap(pict->pDrawable, &temp, &temp) ? 's' : 'm';

    snprintf(size, 20, "%dx%d%s", pict->pDrawable->width,
	     pict->pDrawable->height, pict->repeat ?
	     " R" : "");
    
    snprintf(string, n, "0x%lx:%c fmt %s (%s)", (long)pict, loc, format, size);
}

static void
kaaPrintCompositeFallback(CARD8 op,
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
    
    kaaCompositeFallbackPictDesc(pSrc, srcdesc, 40);
    kaaCompositeFallbackPictDesc(pMask, maskdesc, 40);
    kaaCompositeFallbackPictDesc(pDst, dstdesc, 40);

    ErrorF("Composite fallback: op %s, \n"
	   "                    src  %s, \n"
	   "                    mask %s, \n"
	   "                    dst  %s, \n", 
	   sop, srcdesc, maskdesc, dstdesc);
}

static void
kaaPrintTrapezoidFallback(PicturePtr pDst)
{
    char dstdesc[40];

    kaaCompositeFallbackPictDesc(pDst, dstdesc, 40);

    ErrorF("Trapezoid fallback: dst  %s, %c/%s\n", 
	   dstdesc,
	   (pDst->polyMode == PolyModePrecise) ? 'p' : 'i',
	   (pDst->polyEdge == PolyEdgeSharp) ? "a" : "aa");
}
#endif

static Bool
kaaGetPixelFromRGBA(CARD32	*pixel,
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
kaaGetRGBAFromPixel(CARD32	pixel,
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
kaaTryDriverSolidFill(PicturePtr	pSrc,
		      PicturePtr	pDst,
		      INT16		xSrc,
		      INT16		ySrc,
		      INT16		xDst,
		      INT16		yDst,
		      CARD16		width,
		      CARD16		height)
{
    KaaScreenPriv (pDst->pDrawable->pScreen);
    RegionRec region;
    BoxPtr pbox;
    int nbox;
    int dst_off_x, dst_off_y;
    PixmapPtr pSrcPix, pDstPix;
    CARD32 pixel;
    CARD16 red, green, blue, alpha;

    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;
    xSrc += pSrc->pDrawable->x;
    ySrc += pSrc->pDrawable->y;

    if (!miComputeCompositeRegion (&region, pSrc, NULL, pDst,
				   xSrc, ySrc, 0, 0, xDst, yDst,
				   width, height))
	return 1;

    if (pSrc->pDrawable->type == DRAWABLE_PIXMAP)
	kaaPixmapUseMemory ((PixmapPtr) pSrc->pDrawable);
    if (pDst->pDrawable->type == DRAWABLE_PIXMAP)
	kaaPixmapUseScreen ((PixmapPtr) pDst->pDrawable);

    pDstPix = kaaGetOffscreenPixmap (pDst->pDrawable, &dst_off_x, &dst_off_y);
    if (!pDstPix) {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return 0;
    }

    if (pSrc->pDrawable->type == DRAWABLE_WINDOW)
	pSrcPix = (*pSrc->pDrawable->pScreen->GetWindowPixmap)(
	    (WindowPtr) (pSrc->pDrawable));
    else
	pSrcPix = (PixmapPtr) (pSrc->pDrawable);

    /* If source is offscreen, we need to sync the accelerator
     * before accessing it.  We'd prefer for it to be in memory.
     */
    if (kaaPixmapIsOffscreen(pSrcPix)) {
	kaaWaitSync(pDst->pDrawable->pScreen);
    }

    pixel = *(CARD32 *)(pSrcPix->devPrivate.ptr);
    if (!kaaGetRGBAFromPixel(pixel, &red, &green, &blue, &alpha,
			 pSrc->format))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }
    kaaGetPixelFromRGBA(&pixel, red, green, blue, alpha,
			pDst->format);

    if (!(*pKaaScr->info->PrepareSolid) (pDstPix, GXcopy, 0xffffffff, pixel))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }

    nbox = REGION_NUM_RECTS(&region);
    pbox = REGION_RECTS(&region);
    while (nbox--)
    {
	(*pKaaScr->info->Solid) (pbox->x1 + dst_off_x,
				 pbox->y1 + dst_off_y,
				 pbox->x2 + dst_off_x,
				 pbox->y2 + dst_off_y);
	pbox++;
    }

    (*pKaaScr->info->DoneSolid) ();
    kaaMarkSync (pDst->pDrawable->pScreen);
    kaaDrawableDirty (pDst->pDrawable);

    REGION_UNINIT(pDst->pDrawable->pScreen, &region);
    return 1;
}

static int
kaaTryDriverBlend(CARD8		op,
		  PicturePtr	pSrc,
		  PicturePtr	pDst,
		  INT16		xSrc,
		  INT16		ySrc,
		  INT16		xDst,
		  INT16		yDst,
		  CARD16	width,
		  CARD16	height)
{
    KaaScreenPriv (pDst->pDrawable->pScreen);
    RegionRec region;
    BoxPtr pbox;
    int nbox;
    int src_off_x, src_off_y, dst_off_x, dst_off_y;
    PixmapPtr pSrcPix, pDstPix;
    struct _Pixmap srcScratch;

    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;

    xSrc += pSrc->pDrawable->x;
    ySrc += pSrc->pDrawable->y;

    if (!miComputeCompositeRegion (&region, pSrc, NULL, pDst,
				   xSrc, ySrc, 0, 0, xDst, yDst,
				   width, height))
	return 1;


    if (pSrc->pDrawable->type == DRAWABLE_PIXMAP)
	kaaPixmapUseScreen ((PixmapPtr) pSrc->pDrawable);
    if (pDst->pDrawable->type == DRAWABLE_PIXMAP)
	kaaPixmapUseScreen ((PixmapPtr) pDst->pDrawable);
    
    pSrcPix = kaaGetOffscreenPixmap (pSrc->pDrawable, &src_off_x, &src_off_y);
    pDstPix = kaaGetOffscreenPixmap (pDst->pDrawable, &dst_off_x, &dst_off_y);

    if (!pDstPix) {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return 0;
    }

    if (!pSrcPix && pKaaScr->info->UploadToScratch) {
	if ((*pKaaScr->info->UploadToScratch) ((PixmapPtr) pSrc->pDrawable,
					       &srcScratch))
	    pSrcPix = &srcScratch;
    }

    if (!pSrcPix) {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return 0;
    }

    if (!(*pKaaScr->info->PrepareBlend) (op, pSrc, pDst, pSrcPix,
					 pDstPix))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }
    
    nbox = REGION_NUM_RECTS(&region);
    pbox = REGION_RECTS(&region);

    xSrc -= xDst;
    ySrc -= yDst;

    while (nbox--)
    {
	(*pKaaScr->info->Blend) (pbox->x1 + xSrc + src_off_x,
				 pbox->y1 + ySrc + src_off_y,
				 pbox->x1 + dst_off_x,
				 pbox->y1 + dst_off_y,
				 pbox->x2 - pbox->x1,
				 pbox->y2 - pbox->y1);
	pbox++;
    }
    
    (*pKaaScr->info->DoneBlend) ();
    kaaMarkSync (pDst->pDrawable->pScreen);
    kaaDrawableDirty (pDst->pDrawable);

    REGION_UNINIT(pDst->pDrawable->pScreen, &region);
    return 1;
}

static int
kaaTryDriverComposite(CARD8		op,
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
    KaaScreenPriv (pDst->pDrawable->pScreen);
    RegionRec region;
    BoxPtr pbox;
    int nbox;
    int src_off_x, src_off_y, mask_off_x, mask_off_y, dst_off_x, dst_off_y;
    PixmapPtr pSrcPix, pMaskPix = NULL, pDstPix;
    struct _Pixmap scratch;

    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;

    if (pMask) {
	xMask += pMask->pDrawable->x;
	yMask += pMask->pDrawable->y;
    }

    xSrc += pSrc->pDrawable->x;
    ySrc += pSrc->pDrawable->y;

    if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
				   xSrc, ySrc, xMask, yMask, xDst, yDst,
				   width, height))
	return 1;

    if (pKaaScr->info->CheckComposite &&
	!(*pKaaScr->info->CheckComposite) (op, pSrc, pMask, pDst))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }

    if (pSrc->pDrawable->type == DRAWABLE_PIXMAP)
	kaaPixmapUseScreen ((PixmapPtr) pSrc->pDrawable);
    if (pMask && pMask->pDrawable->type == DRAWABLE_PIXMAP)
	kaaPixmapUseScreen ((PixmapPtr) pMask->pDrawable);
    if (pDst->pDrawable->type == DRAWABLE_PIXMAP)
	kaaPixmapUseScreen ((PixmapPtr) pDst->pDrawable);

    pSrcPix = kaaGetOffscreenPixmap (pSrc->pDrawable, &src_off_x, &src_off_y);
    if (pMask)
	pMaskPix = kaaGetOffscreenPixmap (pMask->pDrawable, &mask_off_x,
					  &mask_off_y);
    pDstPix = kaaGetOffscreenPixmap (pDst->pDrawable, &dst_off_x, &dst_off_y);

    if (!pDstPix) {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return 0;
    }

    if (!pSrcPix && (!pMask || pMaskPix) && pKaaScr->info->UploadToScratch) {
	if (pSrc->pDrawable->type == DRAWABLE_WINDOW)
	    pSrcPix = (*pSrc->pDrawable->pScreen->GetWindowPixmap) (
		(WindowPtr) pSrc->pDrawable);
	else
	    pSrcPix = (PixmapPtr) pSrc->pDrawable;
	if ((*pKaaScr->info->UploadToScratch) (pSrcPix, &scratch))
	    pSrcPix = &scratch;
    } else if (pSrcPix && pMask && !pMaskPix && pKaaScr->info->UploadToScratch) {
	if (pMask->pDrawable->type == DRAWABLE_WINDOW)
	    pMaskPix = (*pMask->pDrawable->pScreen->GetWindowPixmap) (
		(WindowPtr) pMask->pDrawable);
	else
	    pMaskPix = (PixmapPtr) pMask->pDrawable;
	if ((*pKaaScr->info->UploadToScratch) (pMaskPix, &scratch))
	    pMaskPix = &scratch;
    }

    if (!pSrcPix || (pMask && !pMaskPix)) {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return 0;
    }

    if (!(*pKaaScr->info->PrepareComposite) (op, pSrc, pMask, pDst, pSrcPix,
					     pMaskPix, pDstPix))
    {
	REGION_UNINIT(pDst->pDrawable->pScreen, &region);
	return -1;
    }

    nbox = REGION_NUM_RECTS(&region);
    pbox = REGION_RECTS(&region);

    xMask -= xDst;
    yMask -= yDst;

    xSrc -= xDst;
    ySrc -= yDst;

    while (nbox--)
    {
	(*pKaaScr->info->Composite) (pbox->x1 + xSrc + src_off_x,
				     pbox->y1 + ySrc + src_off_y,
				     pbox->x1 + xMask + mask_off_x,
				     pbox->y1 + yMask + mask_off_y,
				     pbox->x1 + dst_off_x,
				     pbox->y1 + dst_off_y,
				     pbox->x2 - pbox->x1,
				     pbox->y2 - pbox->y1);
	pbox++;
    }

    (*pKaaScr->info->DoneComposite) ();
    kaaMarkSync (pDst->pDrawable->pScreen);
    kaaDrawableDirty (pDst->pDrawable);

    REGION_UNINIT(pDst->pDrawable->pScreen, &region);
    return 1;
}


void
kaaComposite(CARD8	op,
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
    KdScreenPriv (pDst->pDrawable->pScreen);
    KaaScreenPriv (pDst->pDrawable->pScreen);
    int ret = -1;

    if (!pMask && pSrc->pDrawable)
    {
	if (op == PictOpSrc)
	{
	    if (pScreenPriv->enabled && pSrc->pDrawable && pSrc->pDrawable->width == 1 &&
		pSrc->pDrawable->height == 1 && pSrc->repeat)
	    {
		ret = kaaTryDriverSolidFill(pSrc, pDst, xSrc, ySrc, xDst, yDst,
					    width, height);
		if (ret == 1)
		    return;
	    }
	    else if (!pSrc->repeat && !pSrc->transform &&
		     pSrc->format == pDst->format)
	    {
		RegionRec	region;

		xDst += pDst->pDrawable->x;
		yDst += pDst->pDrawable->y;
		xSrc += pSrc->pDrawable->x;
		ySrc += pSrc->pDrawable->y;

		if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
					       xSrc, ySrc, xMask, yMask, xDst,
					       yDst, width, height))
		    return;


		kaaCopyNtoN (pSrc->pDrawable, pDst->pDrawable, 0,
			     REGION_RECTS(&region), REGION_NUM_RECTS(&region),
			     xSrc - xDst, ySrc - yDst,
			     FALSE, FALSE, 0, 0);
		return;
	    }
	}

	if (pScreenPriv->enabled && pKaaScr->info->PrepareBlend &&
	    !pSrc->alphaMap && !pDst->alphaMap)
	{
	    ret = kaaTryDriverBlend(op, pSrc, pDst, xSrc, ySrc, xDst, yDst,
				    width, height);
	    if (ret == 1)
		return;
	}
    }

    if (pSrc->pDrawable && (!pMask || pMask->pDrawable) &&
        pScreenPriv->enabled && pKaaScr->info->PrepareComposite &&
	!pSrc->alphaMap && (!pMask || !pMask->alphaMap) && !pDst->alphaMap)
    {
	ret = kaaTryDriverComposite(op, pSrc, pMask, pDst, xSrc, ySrc, xMask,
				    yMask, xDst, yDst, width, height);
	if (ret == 1)
	    return;
    }

    if (ret != 0) {
	/* failure to accelerate was not due to pixmaps being in the wrong
	 * locations.
	 */
	if (pSrc->pDrawable->type == DRAWABLE_PIXMAP)
	    kaaPixmapUseMemory ((PixmapPtr) pSrc->pDrawable);
	if (pMask && pMask->pDrawable->type == DRAWABLE_PIXMAP)
	    kaaPixmapUseMemory ((PixmapPtr) pMask->pDrawable);
	if (pDst->pDrawable->type == DRAWABLE_PIXMAP)
	    kaaPixmapUseMemory ((PixmapPtr) pDst->pDrawable);
    }

#if KAA_DEBUG_FALLBACKS
    kaaPrintCompositeFallback (op, pSrc, pMask, pDst);
#endif

    KdCheckComposite (op, pSrc, pMask, pDst, xSrc, ySrc, 
		      xMask, yMask, xDst, yDst, width, height);
}
#endif

static xFixed
miLineFixedX (xLineFixed *l, xFixed y, Bool ceil)
{
    xFixed	    dx = l->p2.x - l->p1.x;
    xFixed_32_32    ex = (xFixed_32_32) (y - l->p1.y) * dx;
    xFixed	    dy = l->p2.y - l->p1.y;
    if (ceil)
	ex += (dy - 1);
    return l->p1.x + (xFixed) (ex / dy);
}

/* Need to decide just how much to trim, to maintain translation independence
 * when converted to floating point.
 */
#define XFIXED_TO_FLOAT(x) (((float)((x) & 0xffffff00)) / 65536.0)

/* This is just to allow us to work on the hardware side of the problem while
 * waiting for cairo to get a new tesselator.  We may not be able to support
 * RasterizeTrapezoid at all due to the abutting edges requirement, but it might
 * be technically legal if we widened the trap by some epsilon, so that alpha
 * values at abutting edges were a little too big and capped at one, rather than
 * a little too small and looked bad.
 */
void kaaRasterizeTrapezoid(PicturePtr pDst,
			   xTrapezoid *trap,
			   int xoff,
			   int yoff)
{
    KdScreenPriv (pDst->pDrawable->pScreen);
    KaaScreenPriv (pDst->pDrawable->pScreen);
    KaaTrapezoid ktrap;
    PixmapPtr pPix;
    xFixed x1, x2;

    if (!pScreenPriv->enabled ||
	!pKaaScr->info->PrepareTrapezoids ||
	pDst->pDrawable->type != DRAWABLE_PIXMAP ||
        pDst->alphaMap || pDst->format != PICT_a8)
    {
	KdCheckRasterizeTrapezoid (pDst, trap, xoff, yoff);
#if KAA_DEBUG_FALLBACKS
	kaaPrintTrapezoidFallback (pDst);
#endif
	return;
    }
    pPix = (PixmapPtr)pDst->pDrawable;

    kaaPixmapUseScreen (pPix);

    if (!kaaPixmapIsOffscreen (pPix) ||
	!(*pKaaScr->info->PrepareTrapezoids) (pDst, pPix))
    {
#if KAA_DEBUG_FALLBACKS
	kaaPrintTrapezoidFallback (pDst);
#endif
	KdCheckRasterizeTrapezoid (pDst, trap, xoff, yoff);
	return;
    }

    ktrap.ty = XFIXED_TO_FLOAT(trap->top) + yoff;
    x1 = miLineFixedX (&trap->left, trap->top, FALSE);
    x2 = miLineFixedX (&trap->right, trap->top, TRUE);
    ktrap.tl = XFIXED_TO_FLOAT(x1) + xoff;
    ktrap.tr = XFIXED_TO_FLOAT(x2) + xoff;
    ktrap.by = XFIXED_TO_FLOAT(trap->bottom) + yoff;
    x1 = miLineFixedX (&trap->left, trap->bottom, FALSE);
    x2 = miLineFixedX (&trap->right, trap->bottom, TRUE);
    ktrap.bl = XFIXED_TO_FLOAT(x1) + xoff;
    ktrap.br = XFIXED_TO_FLOAT(x2) + xoff;

    (*pKaaScr->info->Trapezoids) (&ktrap, 1);
    (*pKaaScr->info->DoneTrapezoids) ();
}

void
kaaInitTrapOffsets(int grid_order, float *x_offsets, float *y_offsets,
		   float x_offset, float y_offset)
{
    int i = 0;
    float x, y, x_count, y_count;

    x_count = (1 << (grid_order / 2)) + 1;
    y_count = (1 << (grid_order / 2)) - 1;

    x_offset += 1.0 / x_count / 2.0;
    y_offset += 1.0 / y_count / 2.0;

    for (x = 0; x < x_count; x++) {
	for (y = 0; y < y_count; y++) {
	    x_offsets[i] = x / x_count + x_offset;
	    y_offsets[i] = y / y_count + y_offset;
	    i++;
	}
    }
}

