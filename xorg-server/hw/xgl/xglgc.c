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
#include "fb.h"
#include "gcstruct.h"
#include "migc.h"

#define XGL_GC_OP_FALLBACK_PROLOGUE(pDrawable) \
    xglSyncDamageBoxBits (pDrawable);	       \
    XGL_GC_UNWRAP (funcs);		       \
    XGL_GC_UNWRAP (ops)

#define XGL_GC_OP_FALLBACK_EPILOGUE(pDrawable)	  \
    XGL_GC_WRAP (funcs, (GCFuncs *) &xglGCFuncs); \
    XGL_GC_WRAP (ops, (GCOps *) &xglGCOps);	  \
    xglAddCurrentSurfaceDamage (pDrawable)

#define XGL_GC_FILL_OP_FALLBACK_PROLOGUE(pDrawable)		 \
    switch (pGC->fillStyle) {					 \
    case FillSolid:						 \
	break;							 \
    case FillStippled:						 \
    case FillOpaqueStippled:					 \
	if (!xglSyncBits (&pGC->stipple->drawable, NullBox))	 \
	    FatalError (XGL_SW_FAILURE_STRING);			 \
	break;							 \
    case FillTiled:						 \
	if (!xglSyncBits (&pGC->tile.pixmap->drawable, NullBox)) \
	    FatalError (XGL_SW_FAILURE_STRING);			 \
	break;							 \
    }								 \
    XGL_GC_OP_FALLBACK_PROLOGUE (pDrawable)

static const GCFuncs xglGCFuncs = {
    xglValidateGC,
    miChangeGC,
    miCopyGC,
    xglDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip
};

static const GCOps xglGCOps = {
    xglFillSpans,
    xglSetSpans,
    xglPutImage,
    xglCopyArea,
    xglCopyPlane,
    xglPolyPoint,
    xglPolylines,
    xglPolySegment,
    miPolyRectangle,
    xglPolyArc,
    miFillPolygon,
    xglPolyFillRect,
    xglPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    xglImageGlyphBlt,
    xglPolyGlyphBlt,
    xglPushPixels
};

void
xglFillSpans (DrawablePtr pDrawable,
	      GCPtr	  pGC,
	      int	  nspans,
	      DDXPointPtr ppt,
	      int	  *pwidth,
	      int	  fSorted)
{
    XGL_GC_PRIV (pGC);

    if (pGCPriv->flags || pGC->fillStyle == FillStippled)
    {
	XGL_GC_FILL_OP_FALLBACK_PROLOGUE (pDrawable);
	(*pGC->ops->FillSpans) (pDrawable, pGC, nspans, ppt, pwidth, fSorted);
	XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
    }
    else
    {
	/* xglFillSpan handles fall-back */
	xglFillSpan (pDrawable, pGC, nspans, ppt, pwidth);
    }
}

void
xglSetSpans (DrawablePtr pDrawable,
	     GCPtr	 pGC,
	     char	 *psrc,
	     DDXPointPtr ppt,
	     int	 *pwidth,
	     int	 nspans,
	     int	 fSorted)
{
    XGL_GC_PRIV (pGC);

    XGL_GC_OP_FALLBACK_PROLOGUE (pDrawable);
    (*pGC->ops->SetSpans) (pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
    XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
}

void
xglPutImage (DrawablePtr pDrawable,
	     GCPtr	 pGC,
	     int	 depth,
	     int	 x,
	     int	 y,
	     int	 w,
	     int	 h,
	     int	 leftPad,
	     int	 format,
	     char	 *bits)
{
    XGL_GC_PRIV (pGC);

    if (pGC->alu != GXcopy || (pGCPriv->flags & xglGCPlaneMaskFlag))
    {
	XGL_GC_OP_FALLBACK_PROLOGUE (pDrawable);
	(*pGC->ops->PutImage) (pDrawable, pGC, depth,
			       x, y, w, h, leftPad, format, bits);
	XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
    }
    else
    {
	RegionPtr pClip = pGC->pCompositeClip;
	RegionRec region;
	BoxRec	  box;

	XGL_DRAWABLE_PIXMAP (pDrawable);

	if (!xglMapPixmapBits (pPixmap))
	    FatalError (XGL_SW_FAILURE_STRING);

	XGL_GC_UNWRAP (funcs);
	XGL_GC_UNWRAP (ops);

	(*pGC->ops->PutImage) (pDrawable, pGC, depth,
			       x, y, w, h, leftPad, format, bits);

	XGL_GC_WRAP (funcs, (GCFuncs *) &xglGCFuncs);
	XGL_GC_WRAP (ops, (GCOps *) &xglGCOps);

	box.x1 = pDrawable->x + x;
	box.y1 = pDrawable->y + y;
	box.x2 = box.x1 + w;
	box.y2 = box.y1 + h;

	REGION_INIT (pDrawable->pScreen, &region, &box, 1);
	REGION_INTERSECT (pDrawable->pScreen, &region, pClip, &region);

	xglAddSurfaceDamage (pDrawable, &region);

	REGION_UNINIT (pDrawable->pScreen, &region);
    }
}

RegionPtr
xglCopyArea (DrawablePtr pSrc,
	     DrawablePtr pDst,
	     GCPtr	 pGC,
	     int	 srcX,
	     int	 srcY,
	     int	 w,
	     int	 h,
	     int	 dstX,
	     int	 dstY)
{
    RegionPtr pRegion;
    BoxRec    box;

    XGL_GC_PRIV (pGC);

    box.x1 = pSrc->x + srcX;
    box.y1 = pSrc->y + srcY;
    box.x2 = box.x1 + w;
    box.y2 = box.y1 + h;

    if (pGC->alu != GXcopy || pGCPriv->flags)
    {
	if (!xglSyncBits (pSrc, &box))
	    FatalError (XGL_SW_FAILURE_STRING);

	XGL_GC_OP_FALLBACK_PROLOGUE (pDst);
	pRegion = (*pGC->ops->CopyArea) (pSrc, pDst, pGC,
					 srcX, srcY, w, h, dstX, dstY);
	XGL_GC_OP_FALLBACK_EPILOGUE (pDst);
    }
    else
    {
	/* xglCopyProc handles fall-back */
	pRegion = fbDoCopy (pSrc, pDst, pGC,
			    srcX, srcY,
			    w, h,
			    dstX, dstY,
			    xglCopyProc, 0,
			    (void *) &box);
    }

    return pRegion;
}

RegionPtr
xglCopyPlane (DrawablePtr   pSrc,
	      DrawablePtr   pDst,
	      GCPtr	    pGC,
	      int	    srcX,
	      int	    srcY,
	      int	    w,
	      int	    h,
	      int	    dstX,
	      int	    dstY,
	      unsigned long bitPlane)
{
    RegionPtr pRegion;
    BoxRec    box;

    XGL_GC_PRIV (pGC);

    box.x1 = pSrc->x + srcX;
    box.y1 = pSrc->y + srcY;
    box.x2 = box.x1 + w;
    box.y2 = box.y1 + h;

    if (!xglSyncBits (pSrc, &box))
	FatalError (XGL_SW_FAILURE_STRING);

    XGL_GC_OP_FALLBACK_PROLOGUE (pDst);
    pRegion = (*pGC->ops->CopyPlane) (pSrc, pDst, pGC,
				      srcX, srcY, w, h, dstX, dstY,
				      bitPlane);
    XGL_GC_OP_FALLBACK_EPILOGUE (pDst);

    return pRegion;
}

void
xglPolyPoint (DrawablePtr pDrawable,
	      GCPtr       pGC,
	      int	  mode,
	      int	  npt,
	      DDXPointPtr pptInit)
{
    XGL_GC_PRIV (pGC);

    XGL_GC_OP_FALLBACK_PROLOGUE (pDrawable);
    (*pGC->ops->PolyPoint) (pDrawable, pGC, mode, npt, pptInit);
    XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
}

void
xglPolylines (DrawablePtr pDrawable,
	      GCPtr       pGC,
	      int	  mode,
	      int	  npt,
	      DDXPointPtr ppt)
{
    if (pGC->lineWidth == 0)
    {
	XGL_GC_PRIV (pGC);

	if (!pGCPriv->flags)
	{
	    if (pGC->lineStyle == LineSolid)
	    {
		if (xglFillLine (pDrawable, pGC, mode, npt, ppt))
		    return;
	    }
	}

	XGL_GC_FILL_OP_FALLBACK_PROLOGUE (pDrawable);
	(*pGC->ops->Polylines) (pDrawable, pGC, mode, npt, ppt);
	XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
    }
    else
    {
	if (pGC->lineStyle != LineSolid)
	    miWideDash (pDrawable, pGC, mode, npt, ppt);
	else
	    miWideLine (pDrawable, pGC, mode, npt, ppt);
    }
}

void
xglPolySegment (DrawablePtr pDrawable,
		GCPtr	    pGC,
		int	    nsegInit,
		xSegment    *pSegInit)
{
    if (pGC->lineWidth == 0)
    {
	XGL_GC_PRIV (pGC);

	if (!pGCPriv->flags)
	{
	    if (pGC->lineStyle == LineSolid)
	    {
		if (xglFillSegment (pDrawable, pGC, nsegInit, pSegInit))
		    return;
	    }
	}

	XGL_GC_FILL_OP_FALLBACK_PROLOGUE (pDrawable);
	(*pGC->ops->PolySegment) (pDrawable, pGC, nsegInit, pSegInit);
	XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
    } else
	miPolySegment (pDrawable, pGC, nsegInit, pSegInit);
}

void
xglPolyArc (DrawablePtr pDrawable,
	    GCPtr	pGC,
	    int		narcs,
	    xArc	*pArcs)
{
    if (pGC->lineWidth == 0)
    {
	XGL_GC_PRIV (pGC);

	XGL_GC_FILL_OP_FALLBACK_PROLOGUE (pDrawable);
	(*pGC->ops->PolyArc) (pDrawable, pGC, narcs, pArcs);
	XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
    } else
	miPolyArc (pDrawable, pGC, narcs, pArcs);
}

void
xglPolyFillRect (DrawablePtr pDrawable,
		 GCPtr	     pGC,
		 int	     nrect,
		 xRectangle  *prect)
{
    XGL_GC_PRIV (pGC);

    if (pGC->fillStyle == FillStippled || pGCPriv->flags)
    {
	XGL_GC_FILL_OP_FALLBACK_PROLOGUE (pDrawable);
	(*pGC->ops->PolyFillRect) (pDrawable, pGC, nrect, prect);
	XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
    }
    else
    {
	/* xglFillRect handles fall-back */
	xglFillRect (pDrawable, pGC, nrect, prect);
    }
}

void
xglPolyFillArc (DrawablePtr pDrawable,
		GCPtr	    pGC,
		int	    narcs,
		xArc	    *pArcs)
{
    XGL_GC_PRIV (pGC);

    XGL_GC_FILL_OP_FALLBACK_PROLOGUE (pDrawable);
    (*pGC->ops->PolyFillArc) (pDrawable, pGC, narcs, pArcs);
    XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
}

void
xglImageGlyphBlt (DrawablePtr  pDrawable,
		  GCPtr	       pGC,
		  int	       x,
		  int	       y,
		  unsigned int nglyph,
		  CharInfoPtr  *ppci,
		  pointer      pglyphBase)
{
    XGL_GC_PRIV (pGC);

    if (!pGCPriv->flags)
    {
	if (xglSolidGlyph (pDrawable,
			   pGC,
			   x,
			   y,
			   nglyph,
			   ppci,
			   pglyphBase))
	    return;
    }

    XGL_GC_OP_FALLBACK_PROLOGUE (pDrawable);
    (*pGC->ops->ImageGlyphBlt) (pDrawable, pGC, x, y, nglyph, ppci,
				pglyphBase);
    XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
}

void
xglPolyGlyphBlt (DrawablePtr  pDrawable,
		 GCPtr	      pGC,
		 int	      x,
		 int	      y,
		 unsigned int nglyph,
		 CharInfoPtr  *ppci,
		 pointer      pglyphBase)
{
    XGL_GC_PRIV (pGC);

    if (!pGCPriv->flags)
    {
	if (xglFillGlyph (pDrawable,
			  pGC,
			  x,
			  y,
			  nglyph,
			  ppci,
			  pglyphBase))
	    return;
    }

    XGL_GC_FILL_OP_FALLBACK_PROLOGUE (pDrawable);
    (*pGC->ops->PolyGlyphBlt) (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
    XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
}

void
xglPushPixels (GCPtr	   pGC,
	       PixmapPtr   pBitmap,
	       DrawablePtr pDrawable,
	       int	   w,
	       int	   h,
	       int	   x,
	       int	   y)
{
    XGL_GC_PRIV (pGC);

    if (!xglSyncBits (&pBitmap->drawable, NullBox))
	FatalError (XGL_SW_FAILURE_STRING);

    XGL_GC_OP_FALLBACK_PROLOGUE (pDrawable);
    (*pGC->ops->PushPixels) (pGC, pBitmap, pDrawable, w, h, x, y);
    XGL_GC_OP_FALLBACK_EPILOGUE (pDrawable);
}

Bool
xglCreateGC (GCPtr pGC)
{
    ScreenPtr pScreen = pGC->pScreen;
    Bool      ret;

    XGL_SCREEN_PRIV (pScreen);
    XGL_GC_PRIV (pGC);

    XGL_SCREEN_UNWRAP (CreateGC);
    ret = (*pScreen->CreateGC) (pGC);
    XGL_SCREEN_WRAP (CreateGC, xglCreateGC);

    XGL_GC_WRAP (funcs, (GCFuncs *) &xglGCFuncs);
    XGL_GC_WRAP (ops, (GCOps *) &xglGCOps);

    pGCPriv->flags = 0;
    pGCPriv->op = GLITZ_OPERATOR_SRC;

    pGCPriv->fg = NULL;
    pGCPriv->bg = NULL;
    pGCPriv->id = ~0;

    return ret;
}

void
xglDestroyGC (GCPtr pGC)
{
    XGL_GC_PRIV (pGC);

    if (pGCPriv->fg)
	glitz_surface_destroy (pGCPriv->fg);

    if (pGCPriv->bg)
	glitz_surface_destroy (pGCPriv->bg);

    XGL_GC_UNWRAP (funcs);
    XGL_GC_UNWRAP (ops);
    (*pGC->funcs->DestroyGC) (pGC);
    XGL_GC_WRAP (funcs, (GCFuncs *) &xglGCFuncs);
    XGL_GC_WRAP (ops, (GCOps *) &xglGCOps);
}

void
xglValidateGC (GCPtr	     pGC,
	       unsigned long changes,
	       DrawablePtr   pDrawable)
{
    XGL_GC_PRIV (pGC);

    if (changes & GCTile)
    {
	if (!pGC->tileIsPixel &&
	    FbEvenTile (pGC->tile.pixmap->drawable.width *
			pDrawable->bitsPerPixel))
	    xglSyncBits (&pGC->tile.pixmap->drawable, NULL);
    }

    if (changes & GCStipple)
    {
	if (pGC->stipple)
	    xglSyncBits (&pGC->stipple->drawable, NULL);
    }

    XGL_GC_UNWRAP (funcs);
    XGL_GC_UNWRAP (ops);
    (*pGC->funcs->ValidateGC) (pGC, changes, pDrawable);
    XGL_GC_WRAP (funcs, (GCFuncs *) &xglGCFuncs);
    XGL_GC_WRAP (ops, (GCOps *) &xglGCOps);

    if (pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
    {
	XGL_DRAWABLE_PIXMAP_PRIV (pDrawable);

	if (pPixmapPriv->pVisual && pPixmapPriv->pVisual->format.surface)
	{
	    glitz_format_t *format;

	    format = pPixmapPriv->pVisual->format.surface;
	    if (format->id != pGCPriv->id)
	    {
		XGL_SCREEN_PRIV (pDrawable->pScreen);

		pGCPriv->flags |= xglGCSoftwareDrawableFlag;

		if (pGCPriv->fg)
		    glitz_surface_destroy (pGCPriv->fg);

		pGCPriv->fg = glitz_surface_create (pScreenPriv->drawable,
						    format, 1, 1, 0, NULL);
		if (pGCPriv->fg)
		    glitz_surface_set_fill (pGCPriv->fg, GLITZ_FILL_REPEAT);

		if (pGCPriv->bg)
		    glitz_surface_destroy (pGCPriv->bg);

		pGCPriv->bg = glitz_surface_create (pScreenPriv->drawable,
						    format, 1, 1, 0, NULL);
		if (pGCPriv->bg)
		    glitz_surface_set_fill (pGCPriv->bg, GLITZ_FILL_REPEAT);

		pGCPriv->id = format->id;

		if (pGCPriv->fg && pGCPriv->bg)
		{
		    changes |= (GCForeground | GCBackground);
		    pGCPriv->flags &= ~xglGCSoftwareDrawableFlag;
		}
	    }
	}
	else
	    pGCPriv->flags |= xglGCSoftwareDrawableFlag;
    }

    if (changes & GCFunction)
    {
	switch (pGC->alu) {
	case GXclear:
	    pGCPriv->op = GLITZ_OPERATOR_CLEAR;
	    pGCPriv->flags &= ~xglGCBadFunctionFlag;
	    break;
	case GXcopy:
	    pGCPriv->op = GLITZ_OPERATOR_SRC;
	    pGCPriv->flags &= ~xglGCBadFunctionFlag;
	    break;
	case GXnoop:
	    pGCPriv->op = GLITZ_OPERATOR_DST;
	    pGCPriv->flags &= ~xglGCBadFunctionFlag;
	    break;
	default:
	    pGCPriv->flags |= xglGCBadFunctionFlag;
	    break;
	}
    }

    if (changes & GCPlaneMask)
    {
	FbBits mask;

	mask = FbFullMask (pDrawable->depth);

	if ((pGC->planemask & mask) != mask)
	    pGCPriv->flags |= xglGCPlaneMaskFlag;
	else
	    pGCPriv->flags &= ~xglGCPlaneMaskFlag;
    }

    if (!(pGCPriv->flags & xglGCSoftwareDrawableFlag))
    {
	if (changes & (GCForeground | GCBackground))
	{
	    glitz_pixel_format_t format;
	    glitz_buffer_t	 *buffer;
	    CARD32		 pixel;

	    XGL_DRAWABLE_PIXMAP_PRIV (pDrawable);

	    format.fourcc	  = GLITZ_FOURCC_RGB;
	    format.masks	  = pPixmapPriv->pVisual->pPixel->masks;
	    format.xoffset	  = 0;
	    format.skip_lines     = 0;
	    format.bytes_per_line = sizeof (CARD32);
	    format.scanline_order = GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP;

	    buffer = glitz_buffer_create_for_data (&pixel);

	    if (changes & GCForeground)
	    {
		pixel = pGC->fgPixel;
		glitz_set_pixels (pGCPriv->fg, 0, 0, 1, 1, &format, buffer);
	    }

	    if (changes & GCBackground)
	    {
		pixel = pGC->bgPixel;
		glitz_set_pixels (pGCPriv->bg, 0, 0, 1, 1, &format, buffer);
	    }

	    glitz_buffer_destroy (buffer);
	}
    }
}
