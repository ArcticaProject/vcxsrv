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

static glitz_geometry_format_t tileGeometryFormat = {
    {
	GLITZ_PRIMITIVE_QUADS,
	GLITZ_DATA_TYPE_FLOAT,
	sizeof (glitz_float_t) * 4,
	GLITZ_VERTEX_ATTRIBUTE_SRC_COORD_MASK, {
	    GLITZ_DATA_TYPE_FLOAT,
	    GLITZ_COORDINATE_SIZE_XY,
	    sizeof (glitz_float_t) * 2,
	}, {
	    0, 0, 0
	}
    }
};

xglGeometryPtr
xglTiledBoxGeometry (PixmapPtr pTile,
		     int       tileX,
		     int       tileY,
		     BoxPtr    pBox,
		     int       nBox)
{
    ScreenPtr		pScreen = pTile->drawable.pScreen;
    glitz_point_fixed_t p1, p2;
    xglGeometryPtr	pGeometry;
    glitz_float_t	x1, x2, y1, y2;
    int			x, y, width, height, i;
    int			xTile, yTile;
    int			widthTile, heightTile;
    int			widthTmp, xTmp, yTmp, xTileTmp;
    int			tileWidth, tileHeight;
    int			size = 0;
    glitz_float_t	*data;

    XGL_PIXMAP_PRIV (pTile);

    tileWidth  = pTile->drawable.width;
    tileHeight = pTile->drawable.height;

    for (i = 0; i < nBox; i++)
	size +=
	    (((pBox[i].x2 - pBox[i].x1) / tileWidth) + 2) *
	    (((pBox[i].y2 - pBox[i].y1) / tileHeight) + 2);

    pGeometry = xglGetScratchVertexGeometryWithType (pScreen,
						     GEOMETRY_DATA_TYPE_FLOAT,
						     8 * size);

    data = glitz_buffer_map (pGeometry->buffer,
			     GLITZ_BUFFER_ACCESS_WRITE_ONLY);

    while (nBox--)
    {
	x = pBox->x1;
	y = pBox->y1;
	width = pBox->x2 - pBox->x1;
	height = pBox->y2 - pBox->y1;

	xTile = MOD (tileX + x, tileWidth);
	yTile = MOD (tileY + y, tileHeight);

	yTmp = y;

	while (height)
	{
	    heightTile = MIN (tileHeight - yTile, height);

	    xTileTmp = xTile;
	    widthTmp = width;
	    xTmp     = x;

	    while (widthTmp)
	    {
		widthTile = MIN (tileWidth - xTileTmp, widthTmp);

		p1.x = xTileTmp << 16;
		p1.y = yTile << 16;
		p2.x = (xTileTmp + widthTile) << 16;
		p2.y = (yTile + heightTile) << 16;

		glitz_surface_translate_point (pPixmapPriv->surface, &p1, &p1);
		glitz_surface_translate_point (pPixmapPriv->surface, &p2, &p2);

		x1 = FIXED_TO_FLOAT (p1.x);
		y1 = FIXED_TO_FLOAT (p1.y);
		x2 = FIXED_TO_FLOAT (p2.x);
		y2 = FIXED_TO_FLOAT (p2.y);

		*data++ = (glitz_float_t) xTmp;
		*data++ = (glitz_float_t) yTmp;
		*data++ = x1;
		*data++ = y1;

		*data++ = (glitz_float_t) (xTmp + widthTile);
		*data++ = (glitz_float_t) yTmp;
		*data++ = x2;
		*data++ = y1;

		*data++ = (glitz_float_t) (xTmp + widthTile);
		*data++ = (glitz_float_t) (yTmp + heightTile);
		*data++ = x2;
		*data++ = y2;

		*data++ = (glitz_float_t) xTmp;
		*data++ = (glitz_float_t) (yTmp + heightTile);
		*data++ = x1;
		*data++ = y2;

		pGeometry->endOffset += sizeof (glitz_float_t) * 16;

		xTileTmp  = 0;
		xTmp     += widthTile;
		widthTmp -= widthTile;
	    }

	    yTile   = 0;
	    yTmp   += heightTile;
	    height -= heightTile;
	}

	pBox++;
    }

    if (glitz_buffer_unmap (pGeometry->buffer))
	return NULL;

    pGeometry->f     = tileGeometryFormat;
    pGeometry->count =
	pGeometry->endOffset / tileGeometryFormat.vertex.bytes_per_vertex;

    pPixmapPriv->pictureMask |= xglPCFillMask;
    glitz_surface_set_fill (pPixmapPriv->surface, GLITZ_FILL_TRANSPARENT);

    return pGeometry;
}

Bool
xglTile (DrawablePtr	  pDrawable,
	 glitz_operator_t op,
	 PixmapPtr	  pTile,
	 int		  tileX,
	 int		  tileY,
	 xglGeometryPtr	  pGeometry,
	 int		  x,
	 int		  y,
	 int		  width,
	 int		  height,
	 BoxPtr		  pBox,
	 int		  nBox)
{
    xglPixmapPtr    pTilePriv;
    glitz_surface_t *surface;
    int		    xOff, yOff;

    if (nBox < 1)
	return TRUE;

    if (!xglPrepareTarget (pDrawable))
	return FALSE;

    if (!xglSyncSurface (&pTile->drawable))
	return FALSE;

    XGL_GET_DRAWABLE (pDrawable, surface, xOff, yOff);

    pTilePriv = XGL_GET_PIXMAP_PRIV (pTile);

    pTilePriv->pictureMask |= xglPCFilterMask | xglPCTransformMask;
    glitz_surface_set_filter (pTilePriv->surface,
			      GLITZ_FILTER_NEAREST,
			      NULL, 0);
    glitz_surface_set_transform (pTilePriv->surface, NULL);

    if (pTilePriv->acceleratedTile)
    {
	if (pGeometry)
	{
	    glitz_surface_set_clip_region (surface, xOff, yOff,
					   (glitz_box_t *) pBox, nBox);
	    nBox = 0;
	}
	else
	{
	    pGeometry = xglGetScratchVertexGeometry (pDrawable->pScreen,
						     4 * nBox);
	    GEOMETRY_ADD_BOX (pDrawable->pScreen, pGeometry, pBox, nBox);
	}

	GEOMETRY_TRANSLATE (pGeometry, xOff, yOff);

	if (!GEOMETRY_ENABLE (pGeometry, surface))
	    return FALSE;

	pTilePriv->pictureMask |= xglPCFillMask;
	glitz_surface_set_fill (pTilePriv->surface, GLITZ_FILL_REPEAT);

	glitz_composite (op,
			 pTilePriv->surface, NULL, surface,
			 x + tileX,
			 y + tileY,
			 0, 0,
			 x + xOff,
			 y + yOff,
			 width, height);

	glitz_surface_set_clip_region (surface, 0, 0, NULL, 0);

	if (!glitz_surface_get_status (surface))
	    return TRUE;

	if (!nBox)
	    return FALSE;
    }
    else
    {
	if (pGeometry)
	    return FALSE;
    }

    pGeometry = xglTiledBoxGeometry (pTile, tileX, tileY, pBox, nBox);
    if (!pGeometry)
	return FALSE;

    GEOMETRY_TRANSLATE (pGeometry, xOff, yOff);

    if (!GEOMETRY_ENABLE (pGeometry, surface))
	return FALSE;

    glitz_composite (op,
		     pTilePriv->surface, NULL, surface,
		     0, 0,
		     0, 0,
		     x + xOff,
		     y + yOff,
		     width, height);

    if (glitz_surface_get_status (surface))
	return FALSE;

    return TRUE;
}
