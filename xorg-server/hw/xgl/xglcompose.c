/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xgl.h"

#ifdef RENDER

static glitz_operator_t xglOperators[] = {
    GLITZ_OPERATOR_CLEAR,
    GLITZ_OPERATOR_SRC,
    GLITZ_OPERATOR_DST,
    GLITZ_OPERATOR_OVER,
    GLITZ_OPERATOR_OVER_REVERSE,
    GLITZ_OPERATOR_IN,
    GLITZ_OPERATOR_IN_REVERSE,
    GLITZ_OPERATOR_OUT,
    GLITZ_OPERATOR_OUT_REVERSE,
    GLITZ_OPERATOR_ATOP,
    GLITZ_OPERATOR_ATOP_REVERSE,
    GLITZ_OPERATOR_XOR,
    GLITZ_OPERATOR_ADD
};

#define NUM_XGL_OPERATORS			       \
    (sizeof (xglOperators) / sizeof (xglOperators[0]))

#define XGL_OPERATOR(op) (xglOperators[op])

#define XGL_GET_SOURCE_PICTURE(pPicture, outSurface)			   \
    (outSurface) = ((pPicture)->pDrawable) ?				   \
	XGL_GET_PIXMAP_PRIV ((PixmapPtr) (pPicture)->pDrawable)->surface : \
	(glitz_surface_t *) (pPicture)->pSourcePict->source.devPrivate.ptr

Bool
xglCompositeGeneral (CARD8	     op,
		     PicturePtr	     pSrc,
		     PicturePtr	     pMask,
		     PicturePtr	     pDst,
		     xglGeometryPtr  pGeometry,
		     INT16	     xSrc,
		     INT16	     ySrc,
		     INT16	     xMask,
		     INT16	     yMask,
		     INT16	     xDst,
		     INT16	     yDst,
		     CARD16	     width,
		     CARD16	     height)
{
    ScreenPtr	    pScreen = pDst->pDrawable->pScreen;
    INT16	    xOff, yOff;
    glitz_surface_t *src, *mask = NULL, *dst;
    int		    dstXoff, dstYoff;
    RegionRec	    region;
    BoxPtr	    pBox, pExt;
    int		    nBox;

    if (pDst->alphaMap)
	return FALSE;

    if (op >= NUM_XGL_OPERATORS)
	return FALSE;

    if (pSrc->pDrawable)
    {
	if (pSrc->pDrawable->type != DRAWABLE_PIXMAP)
	    return FALSE;

	if (pSrc->pDrawable->bitsPerPixel == 1)
	    return FALSE;
    }

    if (pMask)
    {
       if (pMask->pDrawable)
       {
	   if (pMask->pDrawable->type != DRAWABLE_PIXMAP)
	       return FALSE;

	   if (pSrc->pDrawable == pMask->pDrawable && pSrc != pMask)
	       return FALSE;
       }
    }

    if (!xglPrepareTarget (pDst->pDrawable))
	return FALSE;

    if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst,
				   xSrc, ySrc, xMask, yMask,
				   xDst, yDst, width, height))
	return TRUE;

    pBox = REGION_RECTS (&region);
    nBox = REGION_NUM_RECTS (&region);
    pExt = REGION_EXTENTS (pScreen, &region);

    XGL_GET_DRAWABLE (pDst->pDrawable, dst, dstXoff, dstYoff);

    if (!xglSyncPicture (pScreen, pSrc,
			 pExt->x1 + xSrc - xDst,
			 pExt->y1 + ySrc - yDst,
			 pExt->x2 - pExt->x1,
			 pExt->y2 - pExt->y1,
			 &xOff, &yOff))
    {
	REGION_UNINIT (pScreen, &region);
	return FALSE;
    }

    xSrc -= xOff;
    ySrc -= yOff;

    XGL_GET_SOURCE_PICTURE (pSrc, src);

    if (pMask)
    {
	/* bitmap as mask */
	if (pMask->pDrawable && pMask->pDrawable->bitsPerPixel == 1)
	{
	    if (pGeometry)
	    {
		REGION_UNINIT (pScreen, &region);
		return FALSE;
	    }

	    pGeometry = xglPixmapToGeometry ((PixmapPtr) pMask->pDrawable,
					     xDst - xMask,
					     yDst - yMask);
	    if (!pGeometry)
	    {
		REGION_UNINIT (pScreen, &region);
		return FALSE;
	    }
	}
	else
	{
	    if (!xglSyncPicture (pScreen, pMask,
				 pExt->x1 + xMask - xDst,
				 pExt->y1 + yMask - yDst,
				 pExt->x2 - pExt->x1,
				 pExt->y2 - pExt->y1,
				 &xOff, &yOff))
	    {
		REGION_UNINIT (pScreen, &region);
		return FALSE;
	    }

	    xMask -= xOff;
	    yMask -= yOff;

	    XGL_GET_SOURCE_PICTURE (pMask, mask);
	}
    }

    if (!pGeometry)
    {
	if (!pSrc->transform && pSrc->filter != PictFilterConvolution)
	{
	    if (pSrc->pDrawable && pSrc->repeatType == RepeatNormal)
	    {
		XGL_PIXMAP_PRIV ((PixmapPtr) pSrc->pDrawable);

		/* tile */
		if (!pPixmapPriv->acceleratedTile)
		{
		    pGeometry =
			xglTiledBoxGeometry ((PixmapPtr) pSrc->pDrawable,
					     xSrc - xDst, ySrc - yDst,
					     pBox, nBox);
		    if (!pGeometry)
		    {
			REGION_UNINIT (pScreen, &region);
			return FALSE;
		    }

		    pBox = pExt;
		    nBox = 1;
		}
	    }
	    else
	    {
		/* copy */
		if (op == PictOpSrc && !mask)
		{
		    if (xglCopy (pSrc->pDrawable,
				 pDst->pDrawable,
				 xSrc - xDst,
				 ySrc - yDst,
				 pBox,
				 nBox))
		    {
			REGION_UNINIT (pScreen, &region);
			return TRUE;
		    }
		}
	    }
	}

	if (nBox > 1)
	{
	    pGeometry = xglGetScratchVertexGeometry (pScreen, 4 * nBox);

	    GEOMETRY_ADD_BOX (pScreen, pGeometry, pBox, nBox);

	    pBox = pExt;
	}

	xSrc += pBox->x1 - xDst;
	ySrc += pBox->y1 - yDst;

	if (pMask)
	{
	    xMask += pBox->x1 - xDst;
	    yMask += pBox->y1 - yDst;
	}

	xDst = pBox->x1;
	yDst = pBox->y1;

	width  = pBox->x2 - pBox->x1;
	height = pBox->y2 - pBox->y1;
    }
    else
    {
	glitz_surface_set_clip_region (dst,
				       dstXoff, dstYoff,
				       (glitz_box_t *) pBox, nBox);
    }

    if (pGeometry)
    {
	GEOMETRY_TRANSLATE (pGeometry, dstXoff, dstYoff);

	if (!GEOMETRY_ENABLE (pGeometry, dst))
	{
	    REGION_UNINIT (pScreen, &region);
	    return FALSE;
	}
    }
    else
	GEOMETRY_DISABLE (dst);

    glitz_composite (XGL_OPERATOR (op),
		     src, mask, dst,
		     xSrc, ySrc,
		     xMask, yMask,
		     xDst + dstXoff, yDst + dstYoff,
		     width, height);

    glitz_surface_set_clip_region (dst, 0, 0, NULL, 0);

    REGION_UNINIT (pScreen, &region);

    if (glitz_surface_get_status (dst))
	return FALSE;

    return TRUE;
}

#endif
