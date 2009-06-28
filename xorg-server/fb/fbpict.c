/*
 *
 * Copyright © 2000 SuSE, Inc.
 * Copyright © 2007 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <string.h>

#include "fb.h"

#ifdef RENDER

#include "picturestr.h"
#include "mipict.h"
#include "fbpict.h"

#define mod(a,b) ((b) == 1 ? 0 : (a) >= 0 ? (a) % (b) : (b) - (-a) % (b))

void
fbWalkCompositeRegion (CARD8 op,
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
		       CARD16 height,
		       Bool srcRepeat,
		       Bool maskRepeat,
		       CompositeFunc compositeRect)
{
    RegionRec	    region;
    int		    n;
    BoxPtr	    pbox;
    int		    w, h, w_this, h_this;
    int		    x_msk, y_msk, x_src, y_src, x_dst, y_dst;
    
    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;
    if (pSrc->pDrawable)
    {
        xSrc += pSrc->pDrawable->x;
        ySrc += pSrc->pDrawable->y;
    }
    if (pMask && pMask->pDrawable)
    {
	xMask += pMask->pDrawable->x;
	yMask += pMask->pDrawable->y;
    }

    if (!miComputeCompositeRegion (&region, pSrc, pMask, pDst, xSrc, ySrc,
				   xMask, yMask, xDst, yDst, width, height))
        return;
    
    n = REGION_NUM_RECTS (&region);
    pbox = REGION_RECTS (&region);
    while (n--)
    {
	h = pbox->y2 - pbox->y1;
	y_src = pbox->y1 - yDst + ySrc;
	y_msk = pbox->y1 - yDst + yMask;
	y_dst = pbox->y1;
	while (h)
	{
	    h_this = h;
	    w = pbox->x2 - pbox->x1;
	    x_src = pbox->x1 - xDst + xSrc;
	    x_msk = pbox->x1 - xDst + xMask;
	    x_dst = pbox->x1;
	    if (maskRepeat)
	    {
		y_msk = mod (y_msk - pMask->pDrawable->y, pMask->pDrawable->height);
		if (h_this > pMask->pDrawable->height - y_msk)
		    h_this = pMask->pDrawable->height - y_msk;
		y_msk += pMask->pDrawable->y;
	    }
	    if (srcRepeat)
	    {
		y_src = mod (y_src - pSrc->pDrawable->y, pSrc->pDrawable->height);
		if (h_this > pSrc->pDrawable->height - y_src)
		    h_this = pSrc->pDrawable->height - y_src;
		y_src += pSrc->pDrawable->y;
	    }
	    while (w)
	    {
		w_this = w;
		if (maskRepeat)
		{
		    x_msk = mod (x_msk - pMask->pDrawable->x, pMask->pDrawable->width);
		    if (w_this > pMask->pDrawable->width - x_msk)
			w_this = pMask->pDrawable->width - x_msk;
		    x_msk += pMask->pDrawable->x;
		}
		if (srcRepeat)
		{
		    x_src = mod (x_src - pSrc->pDrawable->x, pSrc->pDrawable->width);
		    if (w_this > pSrc->pDrawable->width - x_src)
			w_this = pSrc->pDrawable->width - x_src;
		    x_src += pSrc->pDrawable->x;
		}
		(*compositeRect) (op, pSrc, pMask, pDst,
				  x_src, y_src, x_msk, y_msk, x_dst, y_dst,
				  w_this, h_this);
		w -= w_this;
		x_src += w_this;
		x_msk += w_this;
		x_dst += w_this;
	    }
	    h -= h_this;
	    y_src += h_this;
	    y_msk += h_this;
	    y_dst += h_this;
	}
	pbox++;
    }
    REGION_UNINIT (pDst->pDrawable->pScreen, &region);
}

void
fbComposite (CARD8      op,
	     PicturePtr pSrc,
	     PicturePtr pMask,
	     PicturePtr pDst,
	     INT16      xSrc,
	     INT16      ySrc,
	     INT16      xMask,
	     INT16      yMask,
	     INT16      xDst,
	     INT16      yDst,
	     CARD16     width,
	     CARD16     height)
{
    pixman_image_t *src, *mask, *dest;
    
    xDst += pDst->pDrawable->x;
    yDst += pDst->pDrawable->y;
    if (pSrc->pDrawable)
    {
        xSrc += pSrc->pDrawable->x;
        ySrc += pSrc->pDrawable->y;
    }
    if (pMask && pMask->pDrawable)
    {
	xMask += pMask->pDrawable->x;
	yMask += pMask->pDrawable->y;
    }

    miCompositeSourceValidate (pSrc, xSrc, ySrc, width, height);
    if (pMask)
	miCompositeSourceValidate (pMask, xMask, yMask, width, height);
    
    src = image_from_pict (pSrc, TRUE);
    mask = image_from_pict (pMask, TRUE);
    dest = image_from_pict (pDst, TRUE);

    if (src && dest && !(pMask && !mask))
    {
	pixman_image_composite (op, src, mask, dest,
				xSrc, ySrc, xMask, yMask, xDst, yDst,
				width, height);
    }

    free_pixman_pict (pSrc, src);
    free_pixman_pict (pMask, mask);
    free_pixman_pict (pDst, dest);
}

void
fbCompositeGeneral (CARD8	op,
		    PicturePtr	pSrc,
		    PicturePtr	pMask,
		    PicturePtr	pDst,
		    INT16	xSrc,
		    INT16	ySrc,
		    INT16	xMask,
		    INT16	yMask,
		    INT16	xDst,
		    INT16	yDst,
		    CARD16	width,
		    CARD16	height)
{
    fbComposite (op, pSrc, pMask, pDst,
			xSrc, ySrc, xMask, yMask, xDst, yDst,
			width, height);
}

#endif /* RENDER */

static pixman_image_t *
create_solid_fill_image (PicturePtr pict)
{
    PictSolidFill *solid = &pict->pSourcePict->solidFill;
    pixman_color_t color;
    CARD32 a, r, g, b;
    
    a = (solid->color & 0xff000000) >> 24;
    r = (solid->color & 0x00ff0000) >> 16;
    g = (solid->color & 0x0000ff00) >>  8;
    b = (solid->color & 0x000000ff) >>  0;
    
    color.alpha = (a << 8) | a;
    color.red =   (r << 8) | r;
    color.green = (g << 8) | g;
    color.blue =  (b << 8) | b;
    
    return pixman_image_create_solid_fill (&color);
}

static pixman_image_t *
create_linear_gradient_image (PictGradient *gradient)
{
    PictLinearGradient *linear = (PictLinearGradient *)gradient;
    pixman_point_fixed_t p1;
    pixman_point_fixed_t p2;
    
    p1.x = linear->p1.x;
    p1.y = linear->p1.y;
    p2.x = linear->p2.x;
    p2.y = linear->p2.y;
    
    return pixman_image_create_linear_gradient (
	&p1, &p2, (pixman_gradient_stop_t *)gradient->stops, gradient->nstops);
}

static pixman_image_t *
create_radial_gradient_image (PictGradient *gradient)
{
    PictRadialGradient *radial = (PictRadialGradient *)gradient;
    pixman_point_fixed_t c1;
    pixman_point_fixed_t c2;
    
    c1.x = radial->c1.x;
    c1.y = radial->c1.y;
    c2.x = radial->c2.x;
    c2.y = radial->c2.y;
    
    return pixman_image_create_radial_gradient (
	&c1, &c2, radial->c1.radius,
	radial->c2.radius,
	(pixman_gradient_stop_t *)gradient->stops, gradient->nstops);
}

static pixman_image_t *
create_conical_gradient_image (PictGradient *gradient)
{
    PictConicalGradient *conical = (PictConicalGradient *)gradient;
    pixman_point_fixed_t center;
    
    center.x = conical->center.x;
    center.y = conical->center.y;
    
    return pixman_image_create_conical_gradient (
	&center, conical->angle, (pixman_gradient_stop_t *)gradient->stops,
	gradient->nstops);
}

static pixman_image_t *
create_bits_picture (PicturePtr pict,
		     Bool       has_clip)
{
    FbBits *bits;
    FbStride stride;
    int bpp, xoff, yoff;
    pixman_image_t *image;
    
    fbGetDrawable (pict->pDrawable, bits, stride, bpp, xoff, yoff);

    bits = (CARD8*)bits + yoff * stride * sizeof(FbBits) + xoff * (bpp / 8);

    image = pixman_image_create_bits (
	pict->format,
	pict->pDrawable->width, pict->pDrawable->height,
	(uint32_t *)bits, stride * sizeof (FbStride));
    
    
#ifdef FB_ACCESS_WRAPPER
#if FB_SHIFT==5
    
    pixman_image_set_accessors (image,
				(pixman_read_memory_func_t)wfbReadMemory,
				(pixman_write_memory_func_t)wfbWriteMemory);
    
#else
    
#error The pixman library only works when FbBits is 32 bits wide
    
#endif
#endif
    
    /* pCompositeClip is undefined for source pictures, so
     * only set the clip region for pictures with drawables
     */
    if (has_clip)
    {
	if (pict->clientClipType != CT_NONE)
	    pixman_image_set_has_client_clip (image, TRUE);
	
	pixman_image_set_clip_region (image, pict->pCompositeClip);
    }
    
    /* Indexed table */
    if (pict->pFormat->index.devPrivate)
	pixman_image_set_indexed (image, pict->pFormat->index.devPrivate);

    return image;
}

static void
set_image_properties (pixman_image_t *image, PicturePtr pict)
{
    pixman_repeat_t repeat;
    pixman_filter_t filter;
    
    if (pict->transform)
    {
	pixman_image_set_transform (
	    image, (pixman_transform_t *)pict->transform);
    }
    
    switch (pict->repeatType)
    {
    default:
    case RepeatNone:
	repeat = PIXMAN_REPEAT_NONE;
	break;
	
    case RepeatPad:
	repeat = PIXMAN_REPEAT_PAD;
	break;
	
    case RepeatNormal:
	repeat = PIXMAN_REPEAT_NORMAL;
	break;
	
    case RepeatReflect:
	repeat = PIXMAN_REPEAT_REFLECT;
	break;
    }
    
    pixman_image_set_repeat (image, repeat);
    
    if (pict->alphaMap)
    {
	pixman_image_t *alpha_map = image_from_pict (pict->alphaMap, TRUE);
	
	pixman_image_set_alpha_map (
	    image, alpha_map, pict->alphaOrigin.x, pict->alphaOrigin.y);
	
	free_pixman_pict (pict->alphaMap, alpha_map);
    }
    
    pixman_image_set_component_alpha (image, pict->componentAlpha);

    switch (pict->filter)
    {
    default:
    case PictFilterNearest:
    case PictFilterFast:
	filter = PIXMAN_FILTER_NEAREST;
	break;
	
    case PictFilterBilinear:
    case PictFilterGood:
	filter = PIXMAN_FILTER_BILINEAR;
	break;
	
    case PictFilterConvolution:
	filter = PIXMAN_FILTER_CONVOLUTION;
	break;
    }
    
    pixman_image_set_filter (image, filter, (pixman_fixed_t *)pict->filter_params, pict->filter_nparams);
    pixman_image_set_source_clipping (image, TRUE);
}

pixman_image_t *
image_from_pict (PicturePtr pict,
		 Bool has_clip)
{
    pixman_image_t *image = NULL;

    if (!pict)
	return NULL;

    if (pict->pDrawable)
    {
	image = create_bits_picture (pict, has_clip);
    }
    else if (pict->pSourcePict)
    {
	SourcePict *sp = pict->pSourcePict;
	
	if (sp->type == SourcePictTypeSolidFill)
	{
	    image = create_solid_fill_image (pict);
	}
	else
	{
	    PictGradient *gradient = &pict->pSourcePict->gradient;
	    
	    if (sp->type == SourcePictTypeLinear)
		image = create_linear_gradient_image (gradient);
	    else if (sp->type == SourcePictTypeRadial)
		image = create_radial_gradient_image (gradient);
	    else if (sp->type == SourcePictTypeConical)
		image = create_conical_gradient_image (gradient);
	}
    }
    
    if (image)
	set_image_properties (image, pict);
    
    return image;
}

void
free_pixman_pict (PicturePtr pict, pixman_image_t *image)
{
    if (image && pixman_image_unref (image) && pict->pDrawable)
	fbFinishAccess (pict->pDrawable);
}

Bool
fbPictureInit (ScreenPtr pScreen, PictFormatPtr formats, int nformats)
{

#ifdef RENDER

    PictureScreenPtr    ps;

    if (!miPictureInit (pScreen, formats, nformats))
	return FALSE;
    ps = GetPictureScreen(pScreen);
    ps->Composite = fbComposite;
    ps->Glyphs = miGlyphs;
    ps->CompositeRects = miCompositeRects;
    ps->RasterizeTrapezoid = fbRasterizeTrapezoid;
    ps->AddTraps = fbAddTraps;
    ps->AddTriangles = fbAddTriangles;

#endif /* RENDER */

    return TRUE;
}
