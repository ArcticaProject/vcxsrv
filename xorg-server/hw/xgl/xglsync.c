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

Bool
xglSyncBits (DrawablePtr pDrawable,
	     BoxPtr	 pExtents)
{
    RegionRec region;
    BoxRec    box;

    XGL_DRAWABLE_PIXMAP (pDrawable);
    XGL_PIXMAP_PRIV (pPixmap);

    if (pPixmapPriv->allBits)
	return xglMapPixmapBits (pPixmap);

    if (pPixmapPriv->target == xglPixmapTargetIn && pExtents)
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pPixmap->drawable.width;
	box.y2 = pPixmap->drawable.height;
	if (pExtents->x1 > box.x1)
	    box.x1 = pExtents->x1;
	if (pExtents->y1 > box.y1)
	    box.y1 = pExtents->y1;
	if (pExtents->x2 < box.x2)
	    box.x2 = pExtents->x2;
	if (pExtents->y2 < box.y2)
	    box.y2 = pExtents->y2;

	if (box.x2 <= box.x1 || box.y2 <= box.y1)
	    return xglMapPixmapBits (pPixmap);

	if (REGION_NOTEMPTY (pDrawable->pScreen, &pPixmapPriv->bitRegion))
	{
	    switch (RECT_IN_REGION (pDrawable->pScreen,
				    &pPixmapPriv->bitRegion,
				    &box)) {
	    case rgnIN:
		REGION_INIT (pDrawable->pScreen, &region, NullBox, 0);
		break;
	    case rgnOUT:
		REGION_INIT (pDrawable->pScreen, &region, &box, 1);
		REGION_UNION (pDrawable->pScreen,
			      &pPixmapPriv->bitRegion, &pPixmapPriv->bitRegion,
			      &region);
		break;
	    case rgnPART:
		REGION_INIT (pDrawable->pScreen, &region, &box, 1);
		REGION_SUBTRACT (pDrawable->pScreen, &region, &region,
				 &pPixmapPriv->bitRegion);
		REGION_UNION (pDrawable->pScreen,
			      &pPixmapPriv->bitRegion, &pPixmapPriv->bitRegion,
			      &region);
		break;
	    }
	}
	else
	{
	    REGION_INIT (pDrawable->pScreen, &region, &box, 1);
	    REGION_SUBTRACT (pDrawable->pScreen, &pPixmapPriv->bitRegion,
			     &region, &pPixmapPriv->bitRegion);
	}

	if (REGION_NUM_RECTS (&pPixmapPriv->bitRegion) == 1)
	{
	    BoxPtr pBox;

	    pBox = REGION_RECTS (&pPixmapPriv->bitRegion);

	    if (pBox->x1 <= 0			    &&
		pBox->y1 <= 0			    &&
		pBox->x2 >= pPixmap->drawable.width &&
		pBox->y2 >= pPixmap->drawable.height)
		pPixmapPriv->allBits = TRUE;
	}
    }
    else
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pPixmap->drawable.width;
	box.y2 = pPixmap->drawable.height;

	REGION_INIT (pDrawable->pScreen, &region, &box, 1);
	REGION_SUBTRACT (pDrawable->pScreen, &region, &region,
			 &pPixmapPriv->bitRegion);

	pPixmapPriv->allBits = TRUE;
    }

    if (!pPixmapPriv->buffer)
	if (!xglAllocatePixmapBits (pPixmap, XGL_PIXMAP_USAGE_HINT_DEFAULT))
	    return FALSE;

    if (REGION_NOTEMPTY (pDrawable->pScreen, &region) && pPixmapPriv->surface)
    {
	glitz_pixel_format_t format;
	BoxPtr		     pBox;
	BoxPtr		     pExt;
	int		     nBox;

	if (!xglSyncSurface (pDrawable))
	    FatalError (XGL_SW_FAILURE_STRING);

	xglUnmapPixmapBits (pPixmap);

	pBox = REGION_RECTS (&region);
	nBox = REGION_NUM_RECTS (&region);
	pExt = REGION_EXTENTS (pDrawable->pScreen, &region);

	format.fourcc  = GLITZ_FOURCC_RGB;
	format.masks   = pPixmapPriv->pVisual->pPixel->masks;
	format.xoffset = pExt->x1;

	if (pPixmapPriv->stride < 0)
	{
	    format.skip_lines	  = pPixmap->drawable.height - pExt->y2;
	    format.bytes_per_line = -pPixmapPriv->stride;
	    format.scanline_order = GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP;
	}
	else
	{
	    format.skip_lines	  = pExt->y1;
	    format.bytes_per_line = pPixmapPriv->stride;
	    format.scanline_order = GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN;
	}

	glitz_surface_set_clip_region (pPixmapPriv->surface,
				       0, 0, (glitz_box_t *) pBox, nBox);

	glitz_get_pixels (pPixmapPriv->surface,
			  pExt->x1,
			  pExt->y1,
			  pExt->x2 - pExt->x1,
			  pExt->y2 - pExt->y1,
			  &format,
			  pPixmapPriv->buffer);

	glitz_surface_set_clip_region (pPixmapPriv->surface, 0, 0, NULL, 0);
    }

    REGION_UNINIT (pDrawable->pScreen, &region);

    if (pPixmapPriv->allBits)
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pPixmap->drawable.width;
	box.y2 = pPixmap->drawable.height;

	REGION_UNINIT (pDrawable->pScreen, &pPixmapPriv->bitRegion);
	REGION_INIT (pDrawable->pScreen, &pPixmapPriv->bitRegion, &box, 1);
    }

    return xglMapPixmapBits (pPixmap);
}

void
xglSyncDamageBoxBits (DrawablePtr pDrawable)
{
    XGL_DRAWABLE_PIXMAP_PRIV (pDrawable);

    if (!xglSyncBits (pDrawable, &pPixmapPriv->damageBox))
	FatalError (XGL_SW_FAILURE_STRING);
}

Bool
xglSyncSurface (DrawablePtr pDrawable)
{
    RegionPtr pRegion;

    XGL_DRAWABLE_PIXMAP (pDrawable);
    XGL_PIXMAP_PRIV (pPixmap);

    if (!pPixmapPriv->surface)
    {
	if (!xglCreatePixmapSurface (pPixmap))
	    return FALSE;
    }

    pRegion = DamageRegion (pPixmapPriv->pDamage);

    if (REGION_NOTEMPTY (pDrawable->pScreen, pRegion))
    {
	glitz_pixel_format_t format;
	BoxPtr		     pBox;
	BoxPtr		     pExt;
	int		     nBox;

	xglUnmapPixmapBits (pPixmap);

	nBox = REGION_NUM_RECTS (pRegion);
	pBox = REGION_RECTS (pRegion);
	pExt = REGION_EXTENTS (pDrawable->pScreen, pRegion);

	format.fourcc  = pPixmapPriv->pVisual->format.surface->color.fourcc;
	format.masks   = pPixmapPriv->pVisual->pPixel->masks;
	format.xoffset = pExt->x1;

	if (pPixmapPriv->stride < 0)
	{
	    format.skip_lines	  = pPixmap->drawable.height - pExt->y2;
	    format.bytes_per_line = -pPixmapPriv->stride;
	    format.scanline_order = GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP;
	}
	else
	{
	    format.skip_lines	  = pExt->y1;
	    format.bytes_per_line = pPixmapPriv->stride;
	    format.scanline_order = GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN;
	}

	glitz_surface_set_clip_region (pPixmapPriv->surface,
				       0, 0, (glitz_box_t *) pBox, nBox);

	glitz_set_pixels (pPixmapPriv->surface,
			  pExt->x1,
			  pExt->y1,
			  pExt->x2 - pExt->x1,
			  pExt->y2 - pExt->y1,
			  &format,
			  pPixmapPriv->buffer);

	glitz_surface_set_clip_region (pPixmapPriv->surface, 0, 0, NULL, 0);

	REGION_EMPTY (pDrawable->pScreen, pRegion);
    }

    return TRUE;
}

Bool
xglPrepareTarget (DrawablePtr pDrawable)
{
    XGL_DRAWABLE_PIXMAP (pDrawable);
    XGL_PIXMAP_PRIV (pPixmap);

    switch (pPixmapPriv->target) {
    case xglPixmapTargetNo:
	break;
    case xglPixmapTargetOut:
	if (xglSyncSurface (pDrawable))
	{
	    glitz_drawable_format_t *format;

	    XGL_SCREEN_PRIV (pDrawable->pScreen);

	    if (!pPixmapPriv->drawable)
	    {
		unsigned int width, height;

		format = pPixmapPriv->pVisual->format.drawable;
		width  = pPixmap->drawable.width;
		height = pPixmap->drawable.height;

		if (pPixmapPriv->pVisual->pbuffer)
		{
		    pPixmapPriv->drawable =
			glitz_create_pbuffer_drawable (pScreenPriv->drawable,
						       format, width, height);
		}
		else
		{
		    pPixmapPriv->drawable =
			glitz_create_drawable (pScreenPriv->drawable,
					       format, width, height);
		}
	    }

	    if (pPixmapPriv->drawable)
	    {
		glitz_surface_attach (pPixmapPriv->surface,
				      pPixmapPriv->drawable,
				      GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);

		pPixmapPriv->target = xglPixmapTargetIn;

		return TRUE;
	    }
	}
	pPixmapPriv->target = xglPixmapTargetNo;
	break;
    case xglPixmapTargetIn:
	if (xglSyncSurface (pDrawable))
	    return TRUE;
	break;
    }

    return FALSE;
}

void
xglAddSurfaceDamage (DrawablePtr pDrawable,
		     RegionPtr   pRegion)
{
    glitz_surface_t *surface;
    int		    xOff, yOff;

    XGL_DRAWABLE_PIXMAP_PRIV (pDrawable);

    pPixmapPriv->damageBox = miEmptyBox;

    XGL_GET_DRAWABLE (pDrawable, surface, xOff, yOff);

    if (xOff || yOff)
	REGION_TRANSLATE (pDrawable->pScreen, pRegion, xOff, yOff);

    if (pPixmapPriv->pDamage)
    {
	RegionPtr pDamageRegion;

	pDamageRegion = DamageRegion (pPixmapPriv->pDamage);

	REGION_UNION (pDrawable->pScreen,
		      pDamageRegion, pDamageRegion,
		      pRegion);
    }

    REGION_UNION (pDrawable->pScreen,
		  &pPixmapPriv->bitRegion, &pPixmapPriv->bitRegion,
		  pRegion);

    if (xOff || yOff)
	REGION_TRANSLATE (pDrawable->pScreen, pRegion, -xOff, -yOff);
}

void
xglAddCurrentSurfaceDamage (DrawablePtr pDrawable)
{
    XGL_DRAWABLE_PIXMAP_PRIV (pDrawable);

    if (BOX_NOTEMPTY (&pPixmapPriv->damageBox))
    {
	RegionRec region;

	REGION_INIT (pDrawable->pScreen, &region, &pPixmapPriv->damageBox, 1);

	if (pPixmapPriv->pDamage)
	{
	    RegionPtr pDamageRegion;

	    pDamageRegion = DamageRegion (pPixmapPriv->pDamage);

	    REGION_UNION (pDrawable->pScreen,
			  pDamageRegion, pDamageRegion,
			  &region);
	}

	REGION_UNION (pDrawable->pScreen,
		      &pPixmapPriv->bitRegion, &pPixmapPriv->bitRegion,
		      &region);

	REGION_UNINIT (pDrawable->pScreen, &region);

	pPixmapPriv->damageBox = miEmptyBox;
    }
}

void
xglAddBitDamage (DrawablePtr pDrawable,
		 RegionPtr   pRegion)
{
    XGL_DRAWABLE_PIXMAP_PRIV (pDrawable);

    if (REGION_NOTEMPTY (pDrawable->pScreen, &pPixmapPriv->bitRegion))
    {
	BoxPtr pBox;
	BoxPtr pExt, pBitExt;
	int    nBox;

	pBox = REGION_RECTS (pRegion);
	pExt = REGION_EXTENTS (pDrawable->pScreen, pRegion);
	nBox = REGION_NUM_RECTS (pRegion);

	pBitExt = REGION_EXTENTS (pDrawable->pScreen, &pPixmapPriv->bitRegion);

	if (pExt->x1 < pBitExt->x2 &&
	    pExt->y1 < pBitExt->y2 &&
	    pExt->x2 > pBitExt->x1 &&
	    pExt->y2 > pBitExt->y1)
	{
	    while (nBox--)
	    {
		if (pBox->x1 < pBitExt->x2 &&
		    pBox->y1 < pBitExt->y2 &&
		    pBox->x2 > pBitExt->x1 &&
		    pBox->y2 > pBitExt->y1)
		{
		    REGION_UNINIT (pDrawable->pScreen,
				   &pPixmapPriv->bitRegion);
		    REGION_INIT (pDrawable->pScreen, &pPixmapPriv->bitRegion,
				 NullBox, 0);
		    pPixmapPriv->allBits = FALSE;
		    return;
		}

		pBox++;
	    }
	}
    }
}

void
xglAddCurrentBitDamage (DrawablePtr pDrawable)
{
    XGL_DRAWABLE_PIXMAP_PRIV (pDrawable);

    if (REGION_NOTEMPTY (pDrawable->pScreen, &pPixmapPriv->bitRegion))
    {
	BoxPtr pBitExt;

	pBitExt = REGION_EXTENTS (pDrawable->pScreen, &pPixmapPriv->bitRegion);

	if (pPixmapPriv->damageBox.x1 < pBitExt->x2 &&
	    pPixmapPriv->damageBox.y1 < pBitExt->y2 &&
	    pPixmapPriv->damageBox.x2 > pBitExt->x1 &&
	    pPixmapPriv->damageBox.y2 > pBitExt->y1)
	{
	    REGION_UNINIT (pDrawable->pScreen, &pPixmapPriv->bitRegion);
	    REGION_INIT (pDrawable->pScreen, &pPixmapPriv->bitRegion,
			 NullBox, 0);
	    pPixmapPriv->allBits = FALSE;
	}
    }

    pPixmapPriv->damageBox = miEmptyBox;
}
