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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "scrnintstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "mi.h"
#include "picturestr.h"
#include "mipict.h"

#ifndef __GNUC__
#define __inline
#endif

int
miCreatePicture (PicturePtr pPicture)
{
    return Success;
}

void
miDestroyPicture (PicturePtr pPicture)
{
    if (pPicture->freeCompClip)
	REGION_DESTROY(pPicture->pDrawable->pScreen, pPicture->pCompositeClip);
}

void
miDestroyPictureClip (PicturePtr pPicture)
{
    switch (pPicture->clientClipType) {
    case CT_NONE:
	return;
    case CT_PIXMAP:
	(*pPicture->pDrawable->pScreen->DestroyPixmap) ((PixmapPtr) (pPicture->clientClip));
	break;
    default:
	/*
	 * we know we'll never have a list of rectangles, since ChangeClip
	 * immediately turns them into a region
	 */
	REGION_DESTROY(pPicture->pDrawable->pScreen, pPicture->clientClip);
	break;
    }
    pPicture->clientClip = NULL;
    pPicture->clientClipType = CT_NONE;
}    

int
miChangePictureClip (PicturePtr    pPicture,
		     int	   type,
		     pointer	   value,
		     int	   n)
{
    ScreenPtr		pScreen = pPicture->pDrawable->pScreen;
    PictureScreenPtr    ps = GetPictureScreen(pScreen);
    pointer		clientClip;
    int			clientClipType;
    
    switch (type) {
    case CT_PIXMAP:
	/* convert the pixmap to a region */
	clientClip = (pointer) BITMAP_TO_REGION(pScreen, (PixmapPtr) value);
	if (!clientClip)
	    return BadAlloc;
	clientClipType = CT_REGION;
	(*pScreen->DestroyPixmap) ((PixmapPtr) value);
	break;
    case CT_REGION:
	clientClip = value;
	clientClipType = CT_REGION;
	break;
    case CT_NONE:
	clientClip = 0;
	clientClipType = CT_NONE;
	break;
    default:
	clientClip = (pointer) RECTS_TO_REGION(pScreen, n,
					       (xRectangle *) value,
					       type);
	if (!clientClip)
	    return BadAlloc;
	clientClipType = CT_REGION;
	xfree(value);
	break;
    }
    (*ps->DestroyPictureClip) (pPicture);
    pPicture->clientClip = clientClip;
    pPicture->clientClipType = clientClipType;
    pPicture->stateChanges |= CPClipMask;
    return Success;
}

void
miChangePicture (PicturePtr pPicture,
		 Mask       mask)
{
    return;
}

void
miValidatePicture (PicturePtr pPicture,
		   Mask       mask)
{
    DrawablePtr	    pDrawable = pPicture->pDrawable;

    if ((mask & (CPClipXOrigin|CPClipYOrigin|CPClipMask|CPSubwindowMode)) ||
	(pDrawable->serialNumber != (pPicture->serialNumber & DRAWABLE_SERIAL_BITS)))
    {
	if (pDrawable->type == DRAWABLE_WINDOW)
	{
	    WindowPtr       pWin = (WindowPtr) pDrawable;
	    RegionPtr       pregWin;
	    Bool            freeTmpClip, freeCompClip;

	    if (pPicture->subWindowMode == IncludeInferiors)
	    {
		pregWin = NotClippedByChildren(pWin);
		freeTmpClip = TRUE;
	    }
	    else
	    {
		pregWin = &pWin->clipList;
		freeTmpClip = FALSE;
	    }
	    freeCompClip = pPicture->freeCompClip;

	    /*
	     * if there is no client clip, we can get by with just keeping the
	     * pointer we got, and remembering whether or not should destroy
	     * (or maybe re-use) it later.  this way, we avoid unnecessary
	     * copying of regions.  (this wins especially if many clients clip
	     * by children and have no client clip.)
	     */
	    if (pPicture->clientClipType == CT_NONE)
	    {
		if (freeCompClip)
		    REGION_DESTROY(pScreen, pPicture->pCompositeClip);
		pPicture->pCompositeClip = pregWin;
		pPicture->freeCompClip = freeTmpClip;
	    }
	    else
	    {
		/*
		 * we need one 'real' region to put into the composite clip. if
		 * pregWin the current composite clip are real, we can get rid of
		 * one. if pregWin is real and the current composite clip isn't,
		 * use pregWin for the composite clip. if the current composite
		 * clip is real and pregWin isn't, use the current composite
		 * clip. if neither is real, create a new region.
		 */

		REGION_TRANSLATE(pScreen, pPicture->clientClip,
				 pDrawable->x + pPicture->clipOrigin.x,
				 pDrawable->y + pPicture->clipOrigin.y);

		if (freeCompClip)
		{
		    REGION_INTERSECT(pScreen, pPicture->pCompositeClip,
				     pregWin, pPicture->clientClip);
		    if (freeTmpClip)
			REGION_DESTROY(pScreen, pregWin);
		}
		else if (freeTmpClip)
		{
		    REGION_INTERSECT(pScreen, pregWin, pregWin, pPicture->clientClip);
		    pPicture->pCompositeClip = pregWin;
		}
		else
		{
		    pPicture->pCompositeClip = REGION_CREATE(pScreen, NullBox, 0);
		    REGION_INTERSECT(pScreen, pPicture->pCompositeClip,
				     pregWin, pPicture->clientClip);
		}
		pPicture->freeCompClip = TRUE;
		REGION_TRANSLATE(pScreen, pPicture->clientClip,
				 -(pDrawable->x + pPicture->clipOrigin.x),
				 -(pDrawable->y + pPicture->clipOrigin.y));
	    }
	}	/* end of composite clip for a window */
	else
	{
	    BoxRec          pixbounds;

	    /* XXX should we translate by drawable.x/y here ? */
	    /* If you want pixmaps in offscreen memory, yes */
	    pixbounds.x1 = pDrawable->x;
	    pixbounds.y1 = pDrawable->y;
	    pixbounds.x2 = pDrawable->x + pDrawable->width;
	    pixbounds.y2 = pDrawable->y + pDrawable->height;

	    if (pPicture->freeCompClip)
	    {
		REGION_RESET(pScreen, pPicture->pCompositeClip, &pixbounds);
	    }
	    else
	    {
		pPicture->freeCompClip = TRUE;
		pPicture->pCompositeClip = REGION_CREATE(pScreen, &pixbounds, 1);
	    }

	    if (pPicture->clientClipType == CT_REGION)
	    {
		if(pDrawable->x || pDrawable->y) {
		    REGION_TRANSLATE(pScreen, pPicture->clientClip,
				     pDrawable->x + pPicture->clipOrigin.x, 
				     pDrawable->y + pPicture->clipOrigin.y);
		    REGION_INTERSECT(pScreen, pPicture->pCompositeClip,
				     pPicture->pCompositeClip, pPicture->clientClip);
		    REGION_TRANSLATE(pScreen, pPicture->clientClip,
				     -(pDrawable->x + pPicture->clipOrigin.x), 
				     -(pDrawable->y + pPicture->clipOrigin.y));
		} else {
		    REGION_TRANSLATE(pScreen, pPicture->pCompositeClip,
				     -pPicture->clipOrigin.x, -pPicture->clipOrigin.y);
		    REGION_INTERSECT(pScreen, pPicture->pCompositeClip,
				     pPicture->pCompositeClip, pPicture->clientClip);
		    REGION_TRANSLATE(pScreen, pPicture->pCompositeClip,
				     pPicture->clipOrigin.x, pPicture->clipOrigin.y);
		}
	    }
	}	/* end of composite clip for pixmap */
    }
}

int
miChangePictureTransform (PicturePtr	pPicture,
			  PictTransform *transform)
{
    return Success;
}

int
miChangePictureFilter (PicturePtr pPicture,
		       int	  filter,
		       xFixed     *params,
		       int	  nparams)
{
    return Success;
}

#define BOUND(v)	(INT16) ((v) < MINSHORT ? MINSHORT : (v) > MAXSHORT ? MAXSHORT : (v))

static inline pixman_bool_t
miClipPictureReg (pixman_region16_t *	pRegion,
		  pixman_region16_t *	pClip,
		  int		dx,
		  int		dy)
{
    if (pixman_region_n_rects(pRegion) == 1 &&
	pixman_region_n_rects(pClip) == 1)
    {
	pixman_box16_t *  pRbox = pixman_region_rectangles(pRegion, NULL);
	pixman_box16_t *  pCbox = pixman_region_rectangles(pClip, NULL);
	int	v;
	
	if (pRbox->x1 < (v = pCbox->x1 + dx))
	    pRbox->x1 = BOUND(v);
	if (pRbox->x2 > (v = pCbox->x2 + dx))
	    pRbox->x2 = BOUND(v);
	if (pRbox->y1 < (v = pCbox->y1 + dy))
	    pRbox->y1 = BOUND(v);
	if (pRbox->y2 > (v = pCbox->y2 + dy))
	    pRbox->y2 = BOUND(v);
	if (pRbox->x1 >= pRbox->x2 ||
	    pRbox->y1 >= pRbox->y2)
	{
	    pixman_region_init (pRegion);
	}
    }
    else if (!pixman_region_not_empty (pClip))
	return FALSE;
    else
    {
	if (dx || dy)
	    pixman_region_translate (pRegion, -dx, -dy);
	if (!pixman_region_intersect (pRegion, pRegion, pClip))
	    return FALSE;
	if (dx || dy)
	    pixman_region_translate(pRegion, dx, dy);
    }
    return pixman_region_not_empty(pRegion);
}

static __inline Bool
miClipPictureSrc (RegionPtr	pRegion,
		  PicturePtr	pPicture,
		  int		dx,
		  int		dy)
{
    /* XXX what to do with clipping from transformed pictures? */
    if (pPicture->transform || !pPicture->pDrawable)
	return TRUE;
    if (pPicture->repeat)
    {
	if (pPicture->clientClipType != CT_NONE)
	{
	    pixman_region_translate ( pRegion, 
			     dx - pPicture->clipOrigin.x,
			     dy - pPicture->clipOrigin.y);
	    if (!REGION_INTERSECT (pScreen, pRegion, pRegion, 
				   (RegionPtr) pPicture->pCompositeClip)) // clientClip))
		return FALSE;
	    pixman_region_translate ( pRegion, 
			     - (dx - pPicture->clipOrigin.x),
			     - (dy - pPicture->clipOrigin.y));
	}
	return TRUE;
    }
    else
    {
	return miClipPictureReg (pRegion,
				 pPicture->pCompositeClip,
				 dx,
				 dy);
    }
}

void
miCompositeSourceValidate (PicturePtr	pPicture,
			   INT16	x,
			   INT16	y,
			   CARD16	width,
			   CARD16	height)
{
    DrawablePtr	pDrawable = pPicture->pDrawable;
    ScreenPtr	pScreen;

    if (!pDrawable)
        return;

    pScreen = pDrawable->pScreen;
    
    if (pScreen->SourceValidate)
    {
        x -= pPicture->pDrawable->x;
        y -= pPicture->pDrawable->y;
	if (pPicture->transform)
	{
	    xPoint	    points[4];
	    int		    i;
	    int		    xmin, ymin, xmax, ymax;

#define VectorSet(i,_x,_y) { points[i].x = _x; points[i].y = _y; }
	    VectorSet (0, x, y);
	    VectorSet (1, x + width, y);
	    VectorSet (2, x, y + height);
	    VectorSet (3, x + width, y + height);
	    xmin = ymin = 32767;
	    xmax = ymax = -32737;
	    for (i = 0; i < 4; i++)
	    {
		PictVector  t;
		t.vector[0] = IntToxFixed (points[i].x);
		t.vector[1] = IntToxFixed (points[i].y);
		t.vector[2] = xFixed1;
		if (PictureTransformPoint (pPicture->transform, &t))
		{
		    int	tx = xFixedToInt (t.vector[0]);
		    int ty = xFixedToInt (t.vector[1]);
		    if (tx < xmin) xmin = tx;
		    if (tx > xmax) xmax = tx;
		    if (ty < ymin) ymin = ty;
		    if (ty > ymax) ymax = ty;
		}
	    }
	    x = xmin;
	    y = ymin;
	    width = xmax - xmin;
	    height = ymax - ymin;
	}
	(*pScreen->SourceValidate) (pDrawable, x, y, width, height);
    }
}

/*
 * returns FALSE if the final region is empty.  Indistinguishable from
 * an allocation failure, but rendering ignores those anyways.
 */

_X_EXPORT Bool
miComputeCompositeRegion (RegionPtr	pRegion,
			  PicturePtr	pSrc,
			  PicturePtr	pMask,
			  PicturePtr	pDst,
			  INT16		xSrc,
			  INT16		ySrc,
			  INT16		xMask,
			  INT16		yMask,
			  INT16		xDst,
			  INT16		yDst,
			  CARD16	width,
			  CARD16	height)
{
    
    int		v;

    pRegion->extents.x1 = xDst;
    v = xDst + width;
    pRegion->extents.x2 = BOUND(v);
    pRegion->extents.y1 = yDst;
    v = yDst + height;
    pRegion->extents.y2 = BOUND(v);
    pRegion->data = 0;
    /* Check for empty operation */
    if (pRegion->extents.x1 >= pRegion->extents.x2 ||
	pRegion->extents.y1 >= pRegion->extents.y2)
    {
	pixman_region_init (pRegion);
	return FALSE;
    }
    /* clip against dst */
    if (!miClipPictureReg (pRegion, pDst->pCompositeClip, 0, 0))
    {
	pixman_region_fini (pRegion);
	return FALSE;
    }
    if (pDst->alphaMap)
    {
	if (!miClipPictureReg (pRegion, pDst->alphaMap->pCompositeClip,
			       -pDst->alphaOrigin.x,
			       -pDst->alphaOrigin.y))
	{
	    pixman_region_fini (pRegion);
	    return FALSE;
	}
    }
    /* clip against src */
    if (!miClipPictureSrc (pRegion, pSrc, xDst - xSrc, yDst - ySrc))
    {
	pixman_region_fini (pRegion);
	return FALSE;
    }
    if (pSrc->alphaMap)
    {
	if (!miClipPictureSrc (pRegion, pSrc->alphaMap,
			       xDst - (xSrc + pSrc->alphaOrigin.x),
			       yDst - (ySrc + pSrc->alphaOrigin.y)))
	{
	    pixman_region_fini (pRegion);
	    return FALSE;
	}
    }
    /* clip against mask */
    if (pMask)
    {
	if (!miClipPictureSrc (pRegion, pMask, xDst - xMask, yDst - yMask))
	{
	    pixman_region_fini (pRegion);
	    return FALSE;
	}	
	if (pMask->alphaMap)
	{
	    if (!miClipPictureSrc (pRegion, pMask->alphaMap,
				   xDst - (xMask + pMask->alphaOrigin.x),
				   yDst - (yMask + pMask->alphaOrigin.y)))
	    {
		pixman_region_fini (pRegion);
		return FALSE;
	    }
	}
    }

    
    miCompositeSourceValidate (pSrc, xSrc, ySrc, width, height);
    if (pMask)
	miCompositeSourceValidate (pMask, xMask, yMask, width, height);

    return TRUE;
}

void
miRenderColorToPixel (PictFormatPtr format,
		      xRenderColor  *color,
		      CARD32	    *pixel)
{
    CARD32	    r, g, b, a;
    miIndexedPtr    pIndexed;

    switch (format->type) {
    case PictTypeDirect:
	r = color->red >> (16 - Ones (format->direct.redMask));
	g = color->green >> (16 - Ones (format->direct.greenMask));
	b = color->blue >> (16 - Ones (format->direct.blueMask));
	a = color->alpha >> (16 - Ones (format->direct.alphaMask));
	r = r << format->direct.red;
	g = g << format->direct.green;
	b = b << format->direct.blue;
	a = a << format->direct.alpha;
	*pixel = r|g|b|a;
	break;
    case PictTypeIndexed:
	pIndexed = (miIndexedPtr) (format->index.devPrivate);
	if (pIndexed->color)
	{
	    r = color->red >> 11;
	    g = color->green >> 11;
	    b = color->blue >> 11;
	    *pixel = miIndexToEnt15 (pIndexed, (r << 10) | (g << 5) | b);
	}
	else
	{
	    r = color->red >> 8;
	    g = color->green >> 8;
	    b = color->blue >> 8;
	    *pixel = miIndexToEntY24 (pIndexed, (r << 16) | (g << 8) | b);
	}
	break;
    }
}

static CARD16
miFillColor (CARD32 pixel, int bits)
{
    while (bits < 16)
    {
	pixel |= pixel << bits;
	bits <<= 1;
    }
    return (CARD16) pixel;
}

Bool
miIsSolidAlpha (PicturePtr pSrc)
{
    ScreenPtr	pScreen;
    char	line[1];

    if (!pSrc->pDrawable)
        return FALSE;

    pScreen = pSrc->pDrawable->pScreen;
    
    /* Alpha-only */
    if (PICT_FORMAT_TYPE (pSrc->format) != PICT_TYPE_A)
	return FALSE;
    /* repeat */
    if (!pSrc->repeat)
	return FALSE;
    /* 1x1 */
    if (pSrc->pDrawable->width != 1 || pSrc->pDrawable->height != 1)
	return FALSE;
    line[0] = 1;
    (*pScreen->GetImage) (pSrc->pDrawable, 0, 0, 1, 1, ZPixmap, ~0L, line);
    switch (pSrc->pDrawable->bitsPerPixel) {
    case 1:
	return (CARD8) line[0] == 1 || (CARD8) line[0] == 0x80;
    case 4:
	return (CARD8) line[0] == 0xf || (CARD8) line[0] == 0xf0;
    case 8:
	return (CARD8) line[0] == 0xff;
    default:
	return FALSE;
    }
}

void
miRenderPixelToColor (PictFormatPtr format,
		      CARD32	    pixel,
		      xRenderColor  *color)
{
    CARD32	    r, g, b, a;
    miIndexedPtr    pIndexed;
    
    switch (format->type) {
    case PictTypeDirect:
	r = (pixel >> format->direct.red) & format->direct.redMask;
	g = (pixel >> format->direct.green) & format->direct.greenMask;
	b = (pixel >> format->direct.blue) & format->direct.blueMask;
	a = (pixel >> format->direct.alpha) & format->direct.alphaMask;
	color->red = miFillColor (r, Ones (format->direct.redMask));
	color->green = miFillColor (g, Ones (format->direct.greenMask));
	color->blue = miFillColor (b, Ones (format->direct.blueMask));
	color->alpha = miFillColor (a, Ones (format->direct.alphaMask));
	break;
    case PictTypeIndexed:
	pIndexed = (miIndexedPtr) (format->index.devPrivate);
	pixel = pIndexed->rgba[pixel & (MI_MAX_INDEXED-1)];
	r = (pixel >> 16) & 0xff;
	g = (pixel >>  8) & 0xff;
	b = (pixel      ) & 0xff;
	color->red = miFillColor (r, 8);
	color->green = miFillColor (g, 8);
	color->blue = miFillColor (b, 8);
	color->alpha = 0xffff;
	break;
    }
}

_X_EXPORT Bool
miPictureInit (ScreenPtr pScreen, PictFormatPtr formats, int nformats)
{
    PictureScreenPtr    ps;
    
    if (!PictureInit (pScreen, formats, nformats))
	return FALSE;
    ps = GetPictureScreen(pScreen);
    ps->CreatePicture = miCreatePicture;
    ps->DestroyPicture = miDestroyPicture;
    ps->ChangePictureClip = miChangePictureClip;
    ps->DestroyPictureClip = miDestroyPictureClip;
    ps->ChangePicture = miChangePicture;
    ps->ValidatePicture = miValidatePicture;
    ps->InitIndexed = miInitIndexed;
    ps->CloseIndexed = miCloseIndexed;
    ps->UpdateIndexed = miUpdateIndexed;
    ps->ChangePictureTransform = miChangePictureTransform;
    ps->ChangePictureFilter = miChangePictureFilter;
    ps->RealizeGlyph = miRealizeGlyph;
    ps->UnrealizeGlyph = miUnrealizeGlyph;

    /* MI rendering routines */
    ps->Composite	= 0;			/* requires DDX support */
    ps->Glyphs		= miGlyphs;
    ps->CompositeRects	= miCompositeRects;
    ps->Trapezoids	= miTrapezoids;
    ps->Triangles	= miTriangles;
    ps->TriStrip	= miTriStrip;
    ps->TriFan		= miTriFan;
    
    ps->RasterizeTrapezoid = 0;			/* requires DDX support */
    ps->AddTraps	= 0;			/* requires DDX support */
    ps->AddTriangles	= 0;			/* requires DDX support */

    return TRUE;
}
