/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xgl.h"
#include "gcstruct.h"
#include "damage.h"

#ifdef RENDER

#define XGL_TRAP_FALLBACK_PROLOGUE(pPicture, func) \
    xglSyncDamageBoxBits (pPicture->pDrawable);	   \
    XGL_PICTURE_SCREEN_UNWRAP (func)

#define XGL_TRAP_FALLBACK_EPILOGUE(pPicture, func, xglfunc) \
    XGL_PICTURE_SCREEN_WRAP (func, xglfunc);		    \
    xglAddCurrentSurfaceDamage (pPicture->pDrawable)

/* just a guess */
#define SMOOTH_TRAPS_ESTIMATE_RECTS(nTrap) (30 * nTrap)

#define LINE_FIXED_X(l, _y, v)			 \
    dx = (l)->p2.x - (l)->p1.x;			 \
    ex = (xFixed_32_32) ((_y) - (l)->p1.y) * dx; \
    dy = (l)->p2.y - (l)->p1.y;			 \
    (v) = (l)->p1.x + (xFixed) (ex / dy)

#define LINE_FIXED_X_CEIL(l, _y, v)		      \
    dx = (l)->p2.x - (l)->p1.x;			      \
    ex = (xFixed_32_32) ((_y) - (l)->p1.y) * dx;      \
    dy = (l)->p2.y - (l)->p1.y;			      \
    (v) = (l)->p1.x + (xFixed) ((ex + (dy - 1)) / dy)

static Bool
xglTrapezoidExtents (PicturePtr pDst,
		     int        ntrap,
		     xTrapezoid *traps,
		     BoxPtr     extents)
{
    Bool	 x_overlap, overlap = FALSE;
    xFixed	 dx, dy, top, bottom;
    xFixed_32_32 ex;

    if (!ntrap)
    {
	extents->x1 = MAXSHORT;
	extents->x2 = MINSHORT;
	extents->y1 = MAXSHORT;
	extents->y2 = MINSHORT;

	return FALSE;
    }

    extents->y1 = xFixedToInt (traps->top);
    extents->y2 = xFixedToInt (xFixedCeil (traps->bottom));

    LINE_FIXED_X (&traps->left, traps->top, top);
    LINE_FIXED_X (&traps->left, traps->bottom, bottom);
    extents->x1 = xFixedToInt (MIN (top, bottom));

    LINE_FIXED_X_CEIL (&traps->right, traps->top, top);
    LINE_FIXED_X_CEIL (&traps->right, traps->bottom, bottom);
    extents->x2 = xFixedToInt (xFixedCeil (MAX (top, bottom)));

    ntrap--;
    traps++;

    for (; ntrap; ntrap--, traps++)
    {
	INT16 x1, y1, x2, y2;

	if (!xTrapezoidValid (traps))
	    continue;

	y1 = xFixedToInt (traps->top);
	y2 = xFixedToInt (xFixedCeil (traps->bottom));

	LINE_FIXED_X (&traps->left, traps->top, top);
	LINE_FIXED_X (&traps->left, traps->bottom, bottom);
	x1 = xFixedToInt (MIN (top, bottom));

	LINE_FIXED_X_CEIL (&traps->right, traps->top, top);
	LINE_FIXED_X_CEIL (&traps->right, traps->bottom, bottom);
	x2 = xFixedToInt (xFixedCeil (MAX (top, bottom)));

	x_overlap = FALSE;
	if (x1 >= extents->x2)
	    extents->x2 = x2;
	else if (x2 <= extents->x1)
	    extents->x1 = x1;
	else
	{
	    x_overlap = TRUE;
	    if (x1 < extents->x1)
		extents->x1 = x1;
	    if (x2 > extents->x2)
		extents->x2 = x2;
	}

	if (y1 >= extents->y2)
	    extents->y2 = y2;
	else if (y2 <= extents->y1)
	    extents->y1 = y1;
	else
	{
	    if (y1 < extents->y1)
		extents->y1 = y1;
	    if (y2 > extents->y2)
		extents->y2 = y2;

	    if (x_overlap)
		overlap = TRUE;
	}
    }

    xglPictureClipExtents (pDst, extents);

    return overlap;
}

void
xglTrapezoids (CARD8	     op,
	       PicturePtr    pSrc,
	       PicturePtr    pDst,
	       PictFormatPtr maskFormat,
	       INT16	     xSrc,
	       INT16	     ySrc,
	       int	     nTrap,
	       xTrapezoid    *traps)
{
    ScreenPtr	    pScreen = pDst->pDrawable->pScreen;
    PicturePtr	    pMask = NULL, pSrcPicture, pDstPicture;
    PicturePtr	    pMaskPicture = NULL;
    xglGeometryPtr  pGeometry = NULL;
    unsigned int    polyEdge = pDst->polyEdge;
    INT16	    xDst, yDst;
    INT16	    xOff, yOff;
    BoxRec	    extents;
    Bool	    overlap;
    Bool	    target;

    XGL_SCREEN_PRIV (pScreen);

    xDst = traps[0].left.p1.x >> 16;
    yDst = traps[0].left.p1.y >> 16;

    overlap = xglTrapezoidExtents (pDst, nTrap, traps, &extents);
    if (extents.y1 >= extents.y2 || extents.x1 >= extents.x2)
	return;

    target = xglPrepareTarget (pDst->pDrawable);

    if (nTrap > 1 && op != PictOpAdd && maskFormat &&
	(!target || overlap || op != PictOpOver))
    {
	PixmapPtr  pPixmap;
	GCPtr	   pGC;
	xRectangle rect;
	int	   error;
	int	   area;

	if (!pScreenPriv->pSolidAlpha)
	{
	    xglCreateSolidAlphaPicture (pScreen);
	    if (!pScreenPriv->pSolidAlpha)
		return;
	}

	rect.x = 0;
	rect.y = 0;
	rect.width = extents.x2 - extents.x1;
	rect.height = extents.y2 - extents.y1;

	pPixmap = (*pScreen->CreatePixmap) (pScreen,
					    rect.width, rect.height,
					    maskFormat->depth,
					    CREATE_PIXMAP_USAGE_SCRATCH);
	if (!pPixmap)
	    return;

	pMask = CreatePicture (0, &pPixmap->drawable, maskFormat,
			       0, 0, serverClient, &error);
	if (!pMask)
	{
	    (*pScreen->DestroyPixmap) (pPixmap);
	    return;
	}

	if (!target)
	{
	    /* make sure we don't do accelerated drawing to mask */
	    xglSetPixmapVisual (pPixmap, NULL);
	}

	area = rect.width * rect.height;
	if ((SMOOTH_TRAPS_ESTIMATE_RECTS (nTrap) * 4) > area)
	    XGL_GET_PIXMAP_PRIV (pPixmap)->target = xglPixmapTargetNo;

	ValidatePicture (pMask);
	pGC = GetScratchGC (pPixmap->drawable.depth, pScreen);
	ValidateGC (&pPixmap->drawable, pGC);
	(*pGC->ops->PolyFillRect) (&pPixmap->drawable, pGC, 1, &rect);
	FreeScratchGC (pGC);

	(*pScreen->DestroyPixmap) (pPixmap);

	target = xglPrepareTarget (pMask->pDrawable);

	xOff = -extents.x1;
	yOff = -extents.y1;
	pSrcPicture = pScreenPriv->pSolidAlpha;
	pDstPicture = pMask;
    }
    else
    {
	if (maskFormat)
	{
	    if (maskFormat->depth == 1)
		polyEdge = PolyEdgeSharp;
	    else
		polyEdge = PolyEdgeSmooth;
	}

	xOff = 0;
	yOff = 0;
	pSrcPicture = pSrc;
	pDstPicture = pDst;
    }

    if (target)
    {
	if (maskFormat || polyEdge == PolyEdgeSmooth)
	{
	    glitz_vertex_format_t *format;
	    glitz_surface_t	  *mask;
	    xTrapezoid		  *pTrap = traps;
	    int			  nAddedTrap, n = nTrap;
	    int			  offset = 0;
	    int			  size = SMOOTH_TRAPS_ESTIMATE_RECTS (n);

	    pMaskPicture = pScreenPriv->trapInfo.pMask;
	    format = &pScreenPriv->trapInfo.format.vertex;
	    mask = pMaskPicture->pSourcePict->source.devPrivate.ptr;

	    size *= format->bytes_per_vertex;
	    pGeometry = xglGetScratchGeometryWithSize (pScreen, size);

	    while (n)
	    {
		if (pGeometry->size < size)
		    GEOMETRY_RESIZE (pScreen, pGeometry, size);

		if (!pGeometry->buffer)
		{
		    if (pMask)
			FreePicture (pMask, 0);

		    return;
		}

		offset +=
		    glitz_add_trapezoids (pGeometry->buffer,
					  offset, size - offset, format->type,
					  mask, (glitz_trapezoid_t *) pTrap, n,
					  &nAddedTrap);

		n     -= nAddedTrap;
		pTrap += nAddedTrap;
		size  *= 2;
	    }

	    pGeometry->f     = pScreenPriv->trapInfo.format;
	    pGeometry->count = offset / format->bytes_per_vertex;
	}
	else
	{
	    pGeometry =
		xglGetScratchVertexGeometryWithType (pScreen,
						     GEOMETRY_DATA_TYPE_FLOAT,
						     4 * nTrap);
	    if (!pGeometry->buffer)
	    {
		if (pMask)
		    FreePicture (pMask, 0);

		return;
	    }

	    GEOMETRY_ADD_TRAPEZOID (pScreen, pGeometry, traps, nTrap);
	}

	GEOMETRY_TRANSLATE (pGeometry,
			    pDstPicture->pDrawable->x + xOff,
			    pDstPicture->pDrawable->y + yOff);
    }

    if (pGeometry &&
	xglCompositeGeneral (pMask ? PictOpAdd : op,
			     pSrcPicture,
			     pMaskPicture,
			     pDstPicture,
			     pGeometry,
			     extents.x1 + xOff + xSrc - xDst,
			     extents.y1 + yOff + ySrc - yDst,
			     0, 0,
			     pDstPicture->pDrawable->x + extents.x1 + xOff,
			     pDstPicture->pDrawable->y + extents.y1 + yOff,
			     extents.x2 - extents.x1,
			     extents.y2 - extents.y1))
    {
	/* no intermediate mask? we need to register damage from here as
	   CompositePicture will never be called. */
	if (!pMask)
	{
	    RegionRec region;

	    REGION_INIT (pScreen, &region, &extents, 1);
	    REGION_TRANSLATE (pScreen, &region,
			      pDst->pDrawable->x, pDst->pDrawable->y);

	    DamageDamageRegion (pDst->pDrawable, &region);

	    REGION_UNINIT (pScreen, &region);
	}

	xglAddCurrentBitDamage (pDstPicture->pDrawable);
    }
    else
    {
	XGL_DRAWABLE_PIXMAP_PRIV (pDstPicture->pDrawable);

	pPixmapPriv->damageBox.x1 = extents.x1 + xOff;
	pPixmapPriv->damageBox.y1 = extents.y1 + yOff;
	pPixmapPriv->damageBox.x2 = extents.x2 + xOff;
	pPixmapPriv->damageBox.y2 = extents.y2 + yOff;

	xglSyncDamageBoxBits (pDstPicture->pDrawable);

	if (pMask || (polyEdge == PolyEdgeSmooth &&
		      op == PictOpAdd && miIsSolidAlpha (pSrc)))
	{
	    PictureScreenPtr ps = GetPictureScreen (pScreen);

	    for (; nTrap; nTrap--, traps++)
		(*ps->RasterizeTrapezoid) (pDstPicture, traps, xOff, yOff);

	    xglAddCurrentSurfaceDamage (pDstPicture->pDrawable);
	}
	else
	    miTrapezoids (op, pSrc, pDstPicture, maskFormat,
			  xSrc, ySrc, nTrap, traps);
    }

    if (pMask)
    {
	CompositePicture (op, pSrc, pMask, pDst,
			  extents.x1 + xSrc - xDst,
			  extents.y1 + ySrc - yDst,
			  0, 0,
			  extents.x1, extents.y1,
			  extents.x2 - extents.x1,
			  extents.y2 - extents.y1);

	FreePicture (pMask, 0);
    }
}

void
xglAddTraps (PicturePtr pDst,
	     INT16	xOff,
	     INT16	yOff,
	     int	nTrap,
	     xTrap	*traps)
{
    PictureScreenPtr pPictureScreen;
    ScreenPtr	     pScreen = pDst->pDrawable->pScreen;

    XGL_SCREEN_PRIV (pScreen);
    XGL_DRAWABLE_PIXMAP_PRIV (pDst->pDrawable);

    if (!pScreenPriv->pSolidAlpha)
    {
	xglCreateSolidAlphaPicture (pScreen);
	if (!pScreenPriv->pSolidAlpha)
	    return;
    }

    pPixmapPriv->damageBox.x1 = 0;
    pPixmapPriv->damageBox.y1 = 0;
    pPixmapPriv->damageBox.x2 = pDst->pDrawable->width;
    pPixmapPriv->damageBox.y2 = pDst->pDrawable->height;

    if (xglPrepareTarget (pDst->pDrawable))
    {
	PicturePtr	      pMask;
	glitz_vertex_format_t *format;
	glitz_surface_t	      *mask;
	xglGeometryPtr	      pGeometry;
	xTrap		      *pTrap = traps;
	int		      nAddedTrap, n = nTrap;
	int		      offset = 0;
	int		      size = SMOOTH_TRAPS_ESTIMATE_RECTS (n);

	pMask = pScreenPriv->trapInfo.pMask;
	format = &pScreenPriv->trapInfo.format.vertex;
	mask = pMask->pSourcePict->source.devPrivate.ptr;

	size *= format->bytes_per_vertex;
	pGeometry = xglGetScratchGeometryWithSize (pScreen, size);

	while (n)
	{
	    if (pGeometry->size < size)
		GEOMETRY_RESIZE (pScreen, pGeometry, size);

	    if (!pGeometry->buffer)
		return;

	    offset +=
		glitz_add_traps (pGeometry->buffer,
				 offset, size - offset, format->type, mask,
				 (glitz_trap_t *) pTrap, n,
				 &nAddedTrap);

	    n     -= nAddedTrap;
	    pTrap += nAddedTrap;
	    size  *= 2;
	}

	pGeometry->f     = pScreenPriv->trapInfo.format;
	pGeometry->count = offset / format->bytes_per_vertex;

	GEOMETRY_TRANSLATE (pGeometry,
			    pDst->pDrawable->x + xOff,
			    pDst->pDrawable->y + yOff);

	if (xglCompositeGeneral (PictOpAdd,
				 pScreenPriv->pSolidAlpha,
				 pMask,
				 pDst,
				 pGeometry,
				 0, 0,
				 0, 0,
				 pDst->pDrawable->x, pDst->pDrawable->y,
				 pDst->pDrawable->width,
				 pDst->pDrawable->height))
	{
	    xglAddCurrentBitDamage (pDst->pDrawable);
	    return;
	}
    }

    pPictureScreen = GetPictureScreen (pScreen);

    XGL_TRAP_FALLBACK_PROLOGUE (pDst, AddTraps);
    (*pPictureScreen->AddTraps) (pDst, xOff, yOff, nTrap, traps);
    XGL_TRAP_FALLBACK_EPILOGUE (pDst, AddTraps, xglAddTraps);
}

#endif
