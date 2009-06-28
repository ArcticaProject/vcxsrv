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
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"

Bool
xglSolid (DrawablePtr	   pDrawable,
	  glitz_operator_t op,
	  glitz_surface_t  *solid,
	  xglGeometryPtr   pGeometry,
	  int		   x,
	  int		   y,
	  int		   width,
	  int		   height,
	  BoxPtr	   pBox,
	  int		   nBox)
{
    glitz_surface_t *surface;
    int		    xOff, yOff;

    if (nBox < 1)
	return TRUE;

    if (!xglPrepareTarget (pDrawable))
	return FALSE;

    XGL_GET_DRAWABLE (pDrawable, surface, xOff, yOff);

    if (pGeometry)
    {
	glitz_surface_set_clip_region (surface, xOff, yOff,
				       (glitz_box_t *) pBox, nBox);
    }
    else
    {
	pGeometry = xglGetScratchVertexGeometry (pDrawable->pScreen, 4 * nBox);
	GEOMETRY_ADD_BOX (pDrawable->pScreen, pGeometry, pBox, nBox);
    }

    GEOMETRY_TRANSLATE (pGeometry, xOff, yOff);

    if (!GEOMETRY_ENABLE (pGeometry, surface))
	return FALSE;

    glitz_composite (op,
		     solid, NULL, surface,
		     0, 0,
		     0, 0,
		     x + xOff,
		     y + yOff,
		     width, height);

    glitz_surface_set_clip_region (surface, 0, 0, NULL, 0);

    if (glitz_surface_get_status (surface))
	return FALSE;

    return TRUE;
}

Bool
xglSolidGlyph (DrawablePtr  pDrawable,
	       GCPtr	    pGC,
	       int	    x,
	       int	    y,
	       unsigned int nGlyph,
	       CharInfoPtr  *ppci,
	       pointer      pglyphBase)
{
    xglGeometryRec geometry;
    int		   xBack, widthBack;
    int		   yBack, heightBack;

    XGL_GC_PRIV (pGC);

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

    widthBack = 0;
    while (nGlyph--)
	widthBack += (*ppci++)->metrics.characterWidth;

    xBack = x;
    if (widthBack < 0)
    {
	xBack += widthBack;
	widthBack = -widthBack;
    }
    yBack = y - FONTASCENT (pGC->font);
    heightBack = FONTASCENT (pGC->font) + FONTDESCENT (pGC->font);

    if (xglSolid (pDrawable,
		  pGCPriv->op,
		  pGCPriv->bg,
		  NULL,
		  xBack,
		  yBack,
		  widthBack,
		  heightBack,
		  REGION_RECTS (pGC->pCompositeClip),
		  REGION_NUM_RECTS (pGC->pCompositeClip)))
    {
	if (xglSolid (pDrawable,
		      pGCPriv->op,
		      pGCPriv->fg,
		      &geometry,
		      xBack,
		      yBack,
		      widthBack,
		      heightBack,
		      REGION_RECTS (pGC->pCompositeClip),
		      REGION_NUM_RECTS (pGC->pCompositeClip)))
	{
	    GEOMETRY_UNINIT (&geometry);
	    xglAddCurrentBitDamage (pDrawable);
	    return TRUE;
	}
    }

    GEOMETRY_UNINIT (&geometry);
    return FALSE;
}
