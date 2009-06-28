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
#include "gcstruct.h"
#include "fb.h"

Bool
xglFill (DrawablePtr	pDrawable,
	 GCPtr		pGC,
	 xglGeometryPtr pGeometry,
	 int		x,
	 int		y,
	 int		width,
	 int		height,
	 BoxPtr		pBox,
	 int		nBox)
{
    XGL_GC_PRIV (pGC);

    switch (pGC->fillStyle) {
    case FillSolid:
	if (xglSolid (pDrawable,
		      pGCPriv->op, pGCPriv->fg,
		      pGeometry,
		      x, y,
		      width, height,
		      pBox, nBox))
	    return TRUE;
	break;
    case FillStippled:
    case FillOpaqueStippled:
	break;
    case FillTiled:
	if (xglTile (pDrawable,
		     pGCPriv->op, pGC->tile.pixmap,
		     -(pGC->patOrg.x + pDrawable->x),
		     -(pGC->patOrg.y + pDrawable->y),
		     pGeometry,
		     x, y,
		     width, height,
		     pBox, nBox))
	    return TRUE;
	break;
    }

    return FALSE;
}

static void
xglFillBox (DrawablePtr pDrawable,
	    GCPtr	pGC,
	    int		x,
	    int		y,
	    int		width,
	    int		height,
	    BoxPtr	pBox,
	    int		nBox)
{
    if (!nBox)
	return;

    if (!xglFill (pDrawable, pGC, NULL, x, y, width, height, pBox, nBox))
    {
	RegionRec region;

	XGL_DRAWABLE_PIXMAP (pDrawable);
	XGL_PIXMAP_PRIV (pPixmap);

	if (!xglMapPixmapBits (pPixmap))
	    FatalError (XGL_SW_FAILURE_STRING);

	switch (pGC->fillStyle) {
	case FillSolid:
	    break;
	case FillStippled:
	case FillOpaqueStippled:
	    if (!xglSyncBits (&pGC->stipple->drawable, NullBox))
		FatalError (XGL_SW_FAILURE_STRING);
	    break;
	case FillTiled:
	    if (!xglSyncBits (&pGC->tile.pixmap->drawable, NullBox))
		FatalError (XGL_SW_FAILURE_STRING);
	    break;
	}

	pPixmapPriv->damageBox = miEmptyBox;

	while (nBox--)
	{
	    fbFill (pDrawable, pGC,
		    pBox->x1, pBox->y1,
		    pBox->x2 - pBox->x1, pBox->y2 - pBox->y1);

	    REGION_INIT (pDrawable->pScreen, &region, pBox, 1);
	    xglAddSurfaceDamage (pDrawable, &region);
	    REGION_UNINIT (pDrawable->pScreen, &region);

	    pBox++;
	}
    } else
	xglAddCurrentBitDamage (pDrawable);
}

#define N_STACK_BOX 1024

static BoxPtr
xglMoreBoxes (BoxPtr stackBox,
	      BoxPtr heapBox,
	      int    nBoxes)
{
    Bool stack = !heapBox;

    heapBox = xrealloc (heapBox, sizeof (BoxRec) * nBoxes);
    if (!heapBox)
	return NULL;

    if (stack)
	memcpy (heapBox, stackBox, sizeof (BoxRec) * N_STACK_BOX);

    return heapBox;
}

#define ADD_BOX(pBox, nBox, stackBox, heapBox, size, box)	\
    {								\
	if ((nBox) == (size))					\
	{							\
	    (size) *= 2;					\
	    (heapBox) = xglMoreBoxes (stackBox, heapBox, size);	\
	    if (heapBox)					\
	    {							\
		(pBox) = (heapBox) + (nBox);			\
		*(pBox)++ = (box);				\
		(nBox)++;					\
	    }							\
	}							\
	else							\
	{							\
	    *(pBox)++ = (box);					\
	    (nBox)++;						\
	}							\
    }

void
xglFillRect (DrawablePtr pDrawable,
	     GCPtr	 pGC,
	     int	 nrect,
	     xRectangle  *prect)
{
    RegionPtr pClip = pGC->pCompositeClip;
    BoxPtr    pClipBox;
    BoxPtr    pExtent = REGION_EXTENTS (pGC->pScreen, pClip);
    BoxRec    part, full;
    BoxPtr    heapBox = NULL;
    BoxRec    stackBox[N_STACK_BOX];
    int       size = N_STACK_BOX;
    BoxPtr    pBox = stackBox;
    int	      nClip, nBox = 0;

    while (nrect--)
    {
	full.x1 = prect->x + pDrawable->x;
	full.y1 = prect->y + pDrawable->y;
	full.x2 = full.x1 + (int) prect->width;
	full.y2 = full.y1 + (int) prect->height;

	prect++;

	if (full.x1 < pExtent->x1)
	    full.x1 = pExtent->x1;
	if (full.y1 < pExtent->y1)
	    full.y1 = pExtent->y1;
	if (full.x2 > pExtent->x2)
	    full.x2 = pExtent->x2;
	if (full.y2 > pExtent->y2)
	    full.y2 = pExtent->y2;

	if (full.x1 >= full.x2 || full.y1 >= full.y2)
	    continue;

	nClip = REGION_NUM_RECTS (pClip);
	if (nClip == 1)
	{
	    ADD_BOX (pBox, nBox, stackBox, heapBox, size, full);
	}
	else
	{
	    pClipBox = REGION_RECTS (pClip);
	    while (nClip--)
	    {
		part = *pClipBox++;

		if (part.x1 < full.x1)
		    part.x1 = full.x1;
		if (part.y1 < full.y1)
		    part.y1 = full.y1;
		if (part.x2 > full.x2)
		    part.x2 = full.x2;
		if (part.y2 > full.y2)
		    part.y2 = full.y2;

		if (part.x1 < part.x2 && part.y1 < part.y2)
		    ADD_BOX (pBox, nBox, stackBox, heapBox, size, part);
	    }
	}
    }

    xglFillBox (pDrawable, pGC,
		pExtent->x1, pExtent->y1,
		pExtent->x2 - pExtent->x1, pExtent->y2 - pExtent->y1,
		(heapBox) ? heapBox : stackBox, nBox);

    if (heapBox)
	xfree (heapBox);
}

void
xglFillSpan (DrawablePtr pDrawable,
	     GCPtr	 pGC,
	     int	 n,
	     DDXPointPtr ppt,
	     int	 *pwidth)
{
    RegionPtr pClip = pGC->pCompositeClip;
    BoxPtr    pClipBox;
    BoxPtr    pExtent = REGION_EXTENTS (pGC->pScreen, pClip);
    BoxRec    part, full;
    BoxPtr    heapBox = NULL;
    BoxRec    stackBox[N_STACK_BOX];
    int       size = N_STACK_BOX;
    BoxPtr    pBox = stackBox;
    int	      nClip, nBox = 0;

    while (n--)
    {
	full.x1 = ppt->x;
	full.y1 = ppt->y;
	full.x2 = full.x1 + *pwidth;
	full.y2 = full.y1 + 1;

	pwidth++;
	ppt++;

	if (full.x1 < pExtent->x1)
	    full.x1 = pExtent->x1;
	if (full.y1 < pExtent->y1)
	    full.y1 = pExtent->y1;
	if (full.x2 > pExtent->x2)
	    full.x2 = pExtent->x2;
	if (full.y2 > pExtent->y2)
	    full.y2 = pExtent->y2;

	if (full.x1 >= full.x2 || full.y1 >= full.y2)
	    continue;

	nClip = REGION_NUM_RECTS (pClip);
	if (nClip == 1)
	{
	    ADD_BOX (pBox, nBox, stackBox, heapBox, size, full);
	}
	else
	{
	    pClipBox = REGION_RECTS (pClip);
	    while (nClip--)
	    {
		part = *pClipBox++;

		if (part.x1 < full.x1)
		    part.x1 = full.x1;
		if (part.y1 < full.y1)
		    part.y1 = full.y1;
		if (part.x2 > full.x2)
		    part.x2 = full.x2;
		if (part.y2 > full.y2)
		    part.y2 = full.y2;

		if (part.x1 < part.x2 && part.y1 < part.y2)
		    ADD_BOX (pBox, nBox, stackBox, heapBox, size, part);
	    }
	}
    }

    xglFillBox (pDrawable, pGC,
		pExtent->x1, pExtent->y1,
		pExtent->x2 - pExtent->x1, pExtent->y2 - pExtent->y1,
		(heapBox) ? heapBox : stackBox, nBox);

    if (heapBox)
	xfree (heapBox);
}

Bool
xglFillLine (DrawablePtr pDrawable,
	     GCPtr       pGC,
	     int	 mode,
	     int	 npt,
	     DDXPointPtr ppt)
{
    RegionPtr	   pClip = pGC->pCompositeClip;
    BoxPtr	   pExtent = REGION_EXTENTS (pGC->pScreen, pClip);
    Bool	   coincidentEndpoints = FALSE;
    Bool	   horizontalAndVertical = TRUE;
    DDXPointPtr    pptTmp;
    int		   nptTmp;
    DDXPointRec    pt;
    xglGeometryPtr pGeometry;

    XGL_SCREEN_PRIV (pGC->pScreen);

    if (npt < 2)
	return TRUE;

    pt = *ppt;

    nptTmp = npt - 1;
    pptTmp = ppt + 1;

    if (mode == CoordModePrevious)
    {
	while (nptTmp--)
	{
	    if (pptTmp->x && pptTmp->y)
		horizontalAndVertical = FALSE;

	    pt.x += pptTmp->x;
	    pt.y += pptTmp->y;

	    pptTmp++;
	}

	if (pt.x == ppt->x && pt.y == ppt->y)
	    coincidentEndpoints = TRUE;
    }
    else
    {
	while (nptTmp--)
	{
	    if (pptTmp->x != pt.x && pptTmp->y != pt.y)
	    {
		horizontalAndVertical = FALSE;
		break;
	    }

	    pt = *pptTmp++;
	}

	if (ppt[npt - 1].x == ppt->x && ppt[npt - 1].y == ppt->y)
	    coincidentEndpoints = TRUE;
    }

    if (horizontalAndVertical)
    {
	BoxPtr pClipBox;
	BoxRec part, full;
	BoxPtr heapBox = NULL;
	BoxRec stackBox[N_STACK_BOX];
	int    size = N_STACK_BOX;
	BoxPtr pBox = stackBox;
	int    nClip, nBox = 0;
	int    dx, dy;

	pt = *ppt;

	ppt++;
	npt--;

	while (npt--)
	{
	    if (mode == CoordModePrevious)
	    {
		dx = ppt->x;
		dy = ppt->y;
	    }
	    else
	    {
		dx = ppt->x - pt.x;
		dy = ppt->y - pt.y;
	    }

	    if (dx)
	    {
		if (dx > 0)
		{
		    full.x1 = pt.x + pDrawable->x;

		    if (npt || coincidentEndpoints)
			full.x2 = full.x1 + dx;
		    else
			full.x2 = full.x1 + dx + 1;
		}
		else
		{
		    full.x2 = pt.x + pDrawable->x + 1;

		    if (npt || coincidentEndpoints)
			full.x1 = full.x2 + dx;
		    else
			full.x1 = full.x2 + dx - 1;
		}

		full.y1 = pt.y + pDrawable->y;
		full.y2 = full.y1 + 1;
	    }
	    else
	    {
		if (dy > 0)
		{
		    full.y1 = pt.y + pDrawable->y;

		    if (npt || coincidentEndpoints)
			full.y2 = full.y1 + dy;
		    else
			full.y2 = full.y1 + dy + 1;
		}
		else
		{
		    full.y2 = pt.y + pDrawable->y + 1;

		    if (npt || coincidentEndpoints)
			full.y1 = full.y2 + dy;
		    else
			full.y1 = full.y2 + dy - 1;
		}

		full.x1 = pt.x + pDrawable->x;
		full.x2 = full.x1 + 1;
	    }

	    pt.x += dx;
	    pt.y += dy;

	    ppt++;

	    if (full.x1 < pExtent->x1)
		full.x1 = pExtent->x1;
	    if (full.y1 < pExtent->y1)
		full.y1 = pExtent->y1;
	    if (full.x2 > pExtent->x2)
		full.x2 = pExtent->x2;
	    if (full.y2 > pExtent->y2)
		full.y2 = pExtent->y2;

	    if (full.x1 >= full.x2 || full.y1 >= full.y2)
		continue;

	    nClip = REGION_NUM_RECTS (pClip);
	    if (nClip == 1)
	    {
		ADD_BOX (pBox, nBox, stackBox, heapBox, size, full);
	    }
	    else
	    {
		pClipBox = REGION_RECTS (pClip);
		while (nClip--)
		{
		    part = *pClipBox++;

		    if (part.x1 < full.x1)
			part.x1 = full.x1;
		    if (part.y1 < full.y1)
			part.y1 = full.y1;
		    if (part.x2 > full.x2)
			part.x2 = full.x2;
		    if (part.y2 > full.y2)
			part.y2 = full.y2;

		    if (part.x1 < part.x2 && part.y1 < part.y2)
			ADD_BOX (pBox, nBox, stackBox, heapBox, size, part);
		}
	    }
	}

	xglFillBox (pDrawable, pGC,
		    pExtent->x1, pExtent->y1,
		    pExtent->x2 - pExtent->x1, pExtent->y2 - pExtent->y1,
		    (heapBox) ? heapBox : stackBox, nBox);

	if (heapBox)
	    xfree (heapBox);

	return TRUE;
    }

    if (!pScreenPriv->lines)
	return FALSE;

    if (coincidentEndpoints)
	npt--;

    pGeometry = xglGetScratchVertexGeometry (pGC->pScreen, npt);

    GEOMETRY_ADD_LINE (pGC->pScreen, pGeometry,
		       coincidentEndpoints, mode, npt, ppt);

    if (coincidentEndpoints)
	GEOMETRY_SET_VERTEX_PRIMITIVE (pGeometry, GLITZ_PRIMITIVE_LINE_LOOP);
    else
	GEOMETRY_SET_VERTEX_PRIMITIVE (pGeometry, GLITZ_PRIMITIVE_LINE_STRIP);

    /* Lines need a 0.5 translate */
    GEOMETRY_TRANSLATE_FIXED (pGeometry, 1 << 15, 1 << 15);

    GEOMETRY_TRANSLATE (pGeometry, pDrawable->x, pDrawable->y);

    pExtent = REGION_EXTENTS (pDrawable->pScreen, pGC->pCompositeClip);

    if (xglFill (pDrawable, pGC, pGeometry,
		 pExtent->x1, pExtent->y1,
		 pExtent->x2 - pExtent->x1, pExtent->y2 - pExtent->y1,
		 REGION_RECTS (pGC->pCompositeClip),
		 REGION_NUM_RECTS (pGC->pCompositeClip)))
    {
	xglAddCurrentBitDamage (pDrawable);
	return TRUE;
    }

    return FALSE;
}

Bool
xglFillSegment (DrawablePtr pDrawable,
		GCPtr	    pGC,
		int	    nSegInit,
		xSegment    *pSegInit)
{
    RegionPtr	   pClip = pGC->pCompositeClip;
    BoxPtr	   pExtent = REGION_EXTENTS (pGC->pScreen, pClip);
    Bool	   horizontalAndVertical = TRUE;
    xglGeometryPtr pGeometry;
    xSegment	   *pSeg;
    int		   nSeg;

    XGL_SCREEN_PRIV (pGC->pScreen);

    if (nSegInit < 1)
	return TRUE;

    pSeg = pSegInit;
    nSeg = nSegInit;
    while (nSeg--)
    {
	if (pSeg->x1 != pSeg->x2 && pSeg->y1 != pSeg->y2)
	    horizontalAndVertical = FALSE;

	pSeg++;
    }

    if (horizontalAndVertical)
    {
	BoxPtr pClipBox;
	BoxRec part, full;
	BoxPtr heapBox = NULL;
	BoxRec stackBox[N_STACK_BOX];
	int    size = N_STACK_BOX;
	BoxPtr pBox = stackBox;
	int    nClip, nBox = 0;

	while (nSegInit--)
	{
	    if (pSegInit->x1 != pSegInit->x2)
	    {
		if (pSegInit->x1 < pSegInit->x2)
		{
		    full.x1 = pSegInit->x1;
		    full.x2 = pSegInit->x2;
		}
		else
		{
		    full.x1 = pSegInit->x2;
		    full.x2 = pSegInit->x1;
		}

		full.x1 += pDrawable->x;
		full.x2 += pDrawable->x + 1;
		full.y1  = pSegInit->y1 + pDrawable->y;
		full.y2  = full.y1 + 1;
	    }
	    else
	    {
		if (pSegInit->y1 < pSegInit->y2)
		{
		    full.y1 = pSegInit->y1;
		    full.y2 = pSegInit->y2;
		}
		else
		{
		    full.y1 = pSegInit->y2;
		    full.y2 = pSegInit->y1;
		}

		full.y1 += pDrawable->y;
		full.y2 += pDrawable->y + 1;
		full.x1  = pSegInit->x1 + pDrawable->x;
		full.x2  = full.x1 + 1;
	    }

	    pSegInit++;

	    if (full.x1 < pExtent->x1)
		full.x1 = pExtent->x1;
	    if (full.y1 < pExtent->y1)
		full.y1 = pExtent->y1;
	    if (full.x2 > pExtent->x2)
		full.x2 = pExtent->x2;
	    if (full.y2 > pExtent->y2)
		full.y2 = pExtent->y2;

	    if (full.x1 >= full.x2 || full.y1 >= full.y2)
		continue;

	    nClip = REGION_NUM_RECTS (pClip);
	    if (nClip == 1)
	    {
		ADD_BOX (pBox, nBox, stackBox, heapBox, size, full);
	    }
	    else
	    {
		pClipBox = REGION_RECTS (pClip);
		while (nClip--)
		{
		    part = *pClipBox++;

		    if (part.x1 < full.x1)
			part.x1 = full.x1;
		    if (part.y1 < full.y1)
			part.y1 = full.y1;
		    if (part.x2 > full.x2)
			part.x2 = full.x2;
		    if (part.y2 > full.y2)
			part.y2 = full.y2;

		    if (part.x1 < part.x2 && part.y1 < part.y2)
			ADD_BOX (pBox, nBox, stackBox, heapBox, size, part);
		}
	    }
	}

	xglFillBox (pDrawable, pGC,
		    pExtent->x1, pExtent->y1,
		    pExtent->x2 - pExtent->x1, pExtent->y2 - pExtent->y1,
		    (heapBox) ? heapBox : stackBox, nBox);

	if (heapBox)
	    xfree (heapBox);

	return TRUE;
    }

    if (!pScreenPriv->lines)
	return FALSE;

    pGeometry = xglGetScratchVertexGeometry (pGC->pScreen, 2 * nSegInit);

    GEOMETRY_ADD_SEGMENT (pGC->pScreen, pGeometry, nSegInit, pSegInit);

    /* Line segments need 0.5 translate */
    GEOMETRY_TRANSLATE_FIXED (pGeometry, 1 << 15, 1 << 15);
    GEOMETRY_SET_VERTEX_PRIMITIVE (pGeometry, GLITZ_PRIMITIVE_LINES);

    GEOMETRY_TRANSLATE (pGeometry, pDrawable->x, pDrawable->y);

    if (xglFill (pDrawable, pGC, pGeometry,
		 pExtent->x1, pExtent->y1,
		 pExtent->x2 - pExtent->x1, pExtent->y2 - pExtent->y1,
		 REGION_RECTS (pGC->pCompositeClip),
		 REGION_NUM_RECTS (pGC->pCompositeClip)))
    {
	xglAddCurrentBitDamage (pDrawable);
	return TRUE;
    }

    return FALSE;
}

Bool
xglFillGlyph (DrawablePtr  pDrawable,
	      GCPtr	   pGC,
	      int	   x,
	      int	   y,
	      unsigned int nGlyph,
	      CharInfoPtr  *ppci,
	      pointer      pglyphBase)
{
    BoxPtr	   pExtent;
    xglGeometryRec geometry;

    if (nGlyph < 1)
	return TRUE;

    pExtent = REGION_EXTENTS (pDrawable->pScreen, pGC->pCompositeClip);

    x += pDrawable->x;
    y += pDrawable->y;

    GEOMETRY_INIT (pDrawable->pScreen, &geometry,
		   GLITZ_GEOMETRY_TYPE_BITMAP,
		   GEOMETRY_USAGE_SYSMEM, 0);

    GEOMETRY_FOR_GLYPH (pDrawable->pScreen,
			&geometry,
			nGlyph,
			ppci,
			pglyphBase);

    GEOMETRY_TRANSLATE (&geometry, x, y);

    if (xglFill (pDrawable, pGC, &geometry,
		 pExtent->x1, pExtent->y1,
		 pExtent->x2 - pExtent->x1, pExtent->y2 - pExtent->y1,
		 REGION_RECTS (pGC->pCompositeClip),
		 REGION_NUM_RECTS (pGC->pCompositeClip)))
    {
	GEOMETRY_UNINIT (&geometry);
	xglAddCurrentBitDamage (pDrawable);
	return TRUE;
    }

    GEOMETRY_UNINIT (&geometry);
    return FALSE;
}
