/* -*- Mode: c; c-basic-offset: 4; tab-width: 8; indent-tabs-mode: t; -*- */
/*
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "pixman-private.h"

#include <stdlib.h>

/*
 * Operator optimizations based on source or destination opacity
 */
typedef struct
{
    pixman_op_t op;
    pixman_op_t op_src_dst_opaque;
    pixman_op_t op_src_opaque;
    pixman_op_t op_dst_opaque;
} optimized_operator_info_t;

static const optimized_operator_info_t optimized_operators[] =
{
    /* Input Operator           SRC&DST Opaque          SRC Opaque              DST Opaque      */
    { PIXMAN_OP_OVER,           PIXMAN_OP_SRC,          PIXMAN_OP_SRC,          PIXMAN_OP_OVER },
    { PIXMAN_OP_OVER_REVERSE,   PIXMAN_OP_DST,          PIXMAN_OP_OVER_REVERSE, PIXMAN_OP_DST },
    { PIXMAN_OP_IN,             PIXMAN_OP_SRC,          PIXMAN_OP_IN,           PIXMAN_OP_SRC },
    { PIXMAN_OP_IN_REVERSE,     PIXMAN_OP_DST,          PIXMAN_OP_DST,          PIXMAN_OP_IN_REVERSE },
    { PIXMAN_OP_OUT,            PIXMAN_OP_CLEAR,        PIXMAN_OP_OUT,          PIXMAN_OP_CLEAR },
    { PIXMAN_OP_OUT_REVERSE,    PIXMAN_OP_CLEAR,        PIXMAN_OP_CLEAR,        PIXMAN_OP_OUT_REVERSE },
    { PIXMAN_OP_ATOP,           PIXMAN_OP_SRC,          PIXMAN_OP_IN,           PIXMAN_OP_OVER },
    { PIXMAN_OP_ATOP_REVERSE,   PIXMAN_OP_DST,          PIXMAN_OP_OVER_REVERSE, PIXMAN_OP_IN_REVERSE },
    { PIXMAN_OP_XOR,            PIXMAN_OP_CLEAR,        PIXMAN_OP_OUT,          PIXMAN_OP_OUT_REVERSE },
    { PIXMAN_OP_SATURATE,       PIXMAN_OP_DST,          PIXMAN_OP_OVER_REVERSE, PIXMAN_OP_DST },
    { PIXMAN_OP_NONE }
};

static pixman_implementation_t *imp;

/*
 * Check if the current operator could be optimized
 */
static const optimized_operator_info_t*
pixman_operator_can_be_optimized (pixman_op_t op)
{
    const optimized_operator_info_t *info;

    for (info = optimized_operators; info->op != PIXMAN_OP_NONE; info++)
    {
	if (info->op == op)
	    return info;
    }
    return NULL;
}

/*
 * Optimize the current operator based on opacity of source or destination
 * The output operator should be mathematically equivalent to the source.
 */
static pixman_op_t
pixman_optimize_operator (pixman_op_t     op,
                          pixman_image_t *src_image,
                          pixman_image_t *mask_image,
                          pixman_image_t *dst_image)
{
    pixman_bool_t is_source_opaque;
    pixman_bool_t is_dest_opaque;
    const optimized_operator_info_t *info = pixman_operator_can_be_optimized (op);

    if (!info || mask_image)
	return op;

    is_source_opaque = _pixman_image_is_opaque (src_image);
    is_dest_opaque = _pixman_image_is_opaque (dst_image);

    if (is_source_opaque == FALSE && is_dest_opaque == FALSE)
	return op;

    if (is_source_opaque && is_dest_opaque)
	return info->op_src_dst_opaque;
    else if (is_source_opaque)
	return info->op_src_opaque;
    else if (is_dest_opaque)
	return info->op_dst_opaque;

    return op;

}

static void
apply_workaround (pixman_image_t *image,
		  int32_t *       x,
		  int32_t *       y,
		  uint32_t **     save_bits,
		  int *           save_dx,
		  int *           save_dy)
{
    if (image && image->common.need_workaround)
    {
	/* Some X servers generate images that point to the
	 * wrong place in memory, but then set the clip region
	 * to point to the right place. Because of an old bug
	 * in pixman, this would actually work.
	 *
	 * Here we try and undo the damage
	 */
	int bpp = PIXMAN_FORMAT_BPP (image->bits.format) / 8;
	pixman_box32_t *extents;
	uint8_t *t;
	int dx, dy;
	
	extents = pixman_region32_extents (&(image->common.clip_region));
	dx = extents->x1;
	dy = extents->y1;
	
	*save_bits = image->bits.bits;
	
	*x -= dx;
	*y -= dy;
	pixman_region32_translate (&(image->common.clip_region), -dx, -dy);
	
	t = (uint8_t *)image->bits.bits;
	t += dy * image->bits.rowstride * 4 + dx * bpp;
	image->bits.bits = (uint32_t *)t;
	
	*save_dx = dx;
	*save_dy = dy;
    }
}

static void
unapply_workaround (pixman_image_t *image, uint32_t *bits, int dx, int dy)
{
    if (image && image->common.need_workaround)
    {
	image->bits.bits = bits;
	pixman_region32_translate (&image->common.clip_region, dx, dy);
    }
}

/*
 * Computing composite region
 */
static inline pixman_bool_t
clip_general_image (pixman_region32_t * region,
                    pixman_region32_t * clip,
                    int                 dx,
                    int                 dy)
{
    if (pixman_region32_n_rects (region) == 1 &&
        pixman_region32_n_rects (clip) == 1)
    {
	pixman_box32_t *  rbox = pixman_region32_rectangles (region, NULL);
	pixman_box32_t *  cbox = pixman_region32_rectangles (clip, NULL);
	int v;

	if (rbox->x1 < (v = cbox->x1 + dx))
	    rbox->x1 = v;
	if (rbox->x2 > (v = cbox->x2 + dx))
	    rbox->x2 = v;
	if (rbox->y1 < (v = cbox->y1 + dy))
	    rbox->y1 = v;
	if (rbox->y2 > (v = cbox->y2 + dy))
	    rbox->y2 = v;
	if (rbox->x1 >= rbox->x2 || rbox->y1 >= rbox->y2)
	{
	    pixman_region32_init (region);
	    return FALSE;
	}
    }
    else if (!pixman_region32_not_empty (clip))
    {
	return FALSE;
    }
    else
    {
	if (dx || dy)
	    pixman_region32_translate (region, -dx, -dy);

	if (!pixman_region32_intersect (region, region, clip))
	    return FALSE;

	if (dx || dy)
	    pixman_region32_translate (region, dx, dy);
    }

    return pixman_region32_not_empty (region);
}

static inline pixman_bool_t
clip_source_image (pixman_region32_t * region,
                   pixman_image_t *    image,
                   int                 dx,
                   int                 dy)
{
    /* Source clips are ignored, unless they are explicitly turned on
     * and the clip in question was set by an X client. (Because if
     * the clip was not set by a client, then it is a hierarchy
     * clip and those should always be ignored for sources).
     */
    if (!image->common.clip_sources || !image->common.client_clip)
	return TRUE;

    return clip_general_image (region,
                               &image->common.clip_region,
                               dx, dy);
}

/*
 * returns FALSE if the final region is empty.  Indistinguishable from
 * an allocation failure, but rendering ignores those anyways.
 */
static pixman_bool_t
pixman_compute_composite_region32 (pixman_region32_t * region,
                                   pixman_image_t *    src_image,
                                   pixman_image_t *    mask_image,
                                   pixman_image_t *    dst_image,
                                   int32_t             src_x,
                                   int32_t             src_y,
                                   int32_t             mask_x,
                                   int32_t             mask_y,
                                   int32_t             dest_x,
                                   int32_t             dest_y,
                                   int32_t             width,
                                   int32_t             height)
{
    region->extents.x1 = dest_x;
    region->extents.x2 = dest_x + width;
    region->extents.y1 = dest_y;
    region->extents.y2 = dest_y + height;

    region->extents.x1 = MAX (region->extents.x1, 0);
    region->extents.y1 = MAX (region->extents.y1, 0);
    region->extents.x2 = MIN (region->extents.x2, dst_image->bits.width);
    region->extents.y2 = MIN (region->extents.y2, dst_image->bits.height);

    region->data = 0;

    /* Check for empty operation */
    if (region->extents.x1 >= region->extents.x2 ||
        region->extents.y1 >= region->extents.y2)
    {
	pixman_region32_init (region);
	return FALSE;
    }

    if (dst_image->common.have_clip_region)
    {
	if (!clip_general_image (region, &dst_image->common.clip_region, 0, 0))
	{
	    pixman_region32_fini (region);
	    return FALSE;
	}
    }

    if (dst_image->common.alpha_map && dst_image->common.alpha_map->common.have_clip_region)
    {
	if (!clip_general_image (region, &dst_image->common.alpha_map->common.clip_region,
	                         -dst_image->common.alpha_origin_x,
	                         -dst_image->common.alpha_origin_y))
	{
	    pixman_region32_fini (region);
	    return FALSE;
	}
    }

    /* clip against src */
    if (src_image->common.have_clip_region)
    {
	if (!clip_source_image (region, src_image, dest_x - src_x, dest_y - src_y))
	{
	    pixman_region32_fini (region);
	    return FALSE;
	}
    }
    if (src_image->common.alpha_map && src_image->common.alpha_map->common.have_clip_region)
    {
	if (!clip_source_image (region, (pixman_image_t *)src_image->common.alpha_map,
	                        dest_x - (src_x - src_image->common.alpha_origin_x),
	                        dest_y - (src_y - src_image->common.alpha_origin_y)))
	{
	    pixman_region32_fini (region);
	    return FALSE;
	}
    }
    /* clip against mask */
    if (mask_image && mask_image->common.have_clip_region)
    {
	if (!clip_source_image (region, mask_image, dest_x - mask_x, dest_y - mask_y))
	{
	    pixman_region32_fini (region);
	    return FALSE;
	}
	if (mask_image->common.alpha_map && mask_image->common.alpha_map->common.have_clip_region)
	{
	    if (!clip_source_image (region, (pixman_image_t *)mask_image->common.alpha_map,
	                            dest_x - (mask_x - mask_image->common.alpha_origin_x),
	                            dest_y - (mask_y - mask_image->common.alpha_origin_y)))
	    {
		pixman_region32_fini (region);
		return FALSE;
	    }
	}
    }

    return TRUE;
}

static void
walk_region_internal (pixman_implementation_t *imp,
                      pixman_op_t              op,
                      pixman_image_t *         src_image,
                      pixman_image_t *         mask_image,
                      pixman_image_t *         dst_image,
                      int32_t                  src_x,
                      int32_t                  src_y,
                      int32_t                  mask_x,
                      int32_t                  mask_y,
                      int32_t                  dest_x,
                      int32_t                  dest_y,
                      int32_t                  width,
                      int32_t                  height,
                      pixman_bool_t            src_repeat,
                      pixman_bool_t            mask_repeat,
                      pixman_region32_t *      region,
                      pixman_composite_func_t  composite_rect)
{
    int w, h, w_this, h_this;
    int x_msk, y_msk, x_src, y_src, x_dst, y_dst;
    int src_dy = src_y - dest_y;
    int src_dx = src_x - dest_x;
    int mask_dy = mask_y - dest_y;
    int mask_dx = mask_x - dest_x;
    const pixman_box32_t *pbox;
    int n;

    pbox = pixman_region32_rectangles (region, &n);

    /* Fast path for non-repeating sources */
    if (!src_repeat && !mask_repeat)
    {
       while (n--)
       {
           (*composite_rect) (imp, op,
                              src_image, mask_image, dst_image,
                              pbox->x1 + src_dx,
                              pbox->y1 + src_dy,
                              pbox->x1 + mask_dx,
                              pbox->y1 + mask_dy,
                              pbox->x1,
                              pbox->y1,
                              pbox->x2 - pbox->x1,
                              pbox->y2 - pbox->y1);
           
           pbox++;
       }

       return;
    }
    
    while (n--)
    {
	h = pbox->y2 - pbox->y1;
	y_src = pbox->y1 + src_dy;
	y_msk = pbox->y1 + mask_dy;
	y_dst = pbox->y1;

	while (h)
	{
	    h_this = h;
	    w = pbox->x2 - pbox->x1;
	    x_src = pbox->x1 + src_dx;
	    x_msk = pbox->x1 + mask_dx;
	    x_dst = pbox->x1;

	    if (mask_repeat)
	    {
		y_msk = MOD (y_msk, mask_image->bits.height);
		if (h_this > mask_image->bits.height - y_msk)
		    h_this = mask_image->bits.height - y_msk;
	    }

	    if (src_repeat)
	    {
		y_src = MOD (y_src, src_image->bits.height);
		if (h_this > src_image->bits.height - y_src)
		    h_this = src_image->bits.height - y_src;
	    }

	    while (w)
	    {
		w_this = w;

		if (mask_repeat)
		{
		    x_msk = MOD (x_msk, mask_image->bits.width);
		    if (w_this > mask_image->bits.width - x_msk)
			w_this = mask_image->bits.width - x_msk;
		}

		if (src_repeat)
		{
		    x_src = MOD (x_src, src_image->bits.width);
		    if (w_this > src_image->bits.width - x_src)
			w_this = src_image->bits.width - x_src;
		}

		(*composite_rect) (imp, op,
				   src_image, mask_image, dst_image,
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
}

static void
get_image_info (pixman_image_t       *image,
		pixman_format_code_t *code,
		uint32_t	     *flags)
{
    *flags = 0;
    
    if (!image->common.transform)
    {
	*flags |= FAST_PATH_ID_TRANSFORM;
    }
    else
    {
	if (image->common.transform->matrix[0][1] == 0 &&
	    image->common.transform->matrix[1][0] == 0 &&
	    image->common.transform->matrix[2][0] == 0 &&
	    image->common.transform->matrix[2][1] == 0 &&
	    image->common.transform->matrix[2][2] == pixman_fixed_1)
	{
	    *flags |= FAST_PATH_SCALE_TRANSFORM;
	}
    }
    
    if (!image->common.alpha_map)
	*flags |= FAST_PATH_NO_ALPHA_MAP;
    
    if (image->common.filter != PIXMAN_FILTER_CONVOLUTION)
    {
	*flags |= FAST_PATH_NO_CONVOLUTION_FILTER;
	
	if (image->common.filter == PIXMAN_FILTER_NEAREST)
	    *flags |= FAST_PATH_NEAREST_FILTER;
    }
    
    if (image->common.repeat != PIXMAN_REPEAT_PAD)
	*flags |= FAST_PATH_NO_PAD_REPEAT;
    
    if (image->common.repeat != PIXMAN_REPEAT_REFLECT)
	*flags |= FAST_PATH_NO_REFLECT_REPEAT;
    
    *flags |= (FAST_PATH_NO_ACCESSORS | FAST_PATH_NO_WIDE_FORMAT);
    if (image->type == BITS)
    {
	if (image->bits.read_func || image->bits.write_func)
	    *flags &= ~FAST_PATH_NO_ACCESSORS;
	
	if (PIXMAN_FORMAT_IS_WIDE (image->bits.format))
	    *flags &= ~FAST_PATH_NO_WIDE_FORMAT;
    }
    
    if (image->common.component_alpha)
	*flags |= FAST_PATH_COMPONENT_ALPHA;
    else
	*flags |= FAST_PATH_UNIFIED_ALPHA;
    
    if (_pixman_image_is_solid (image))
	*code = PIXMAN_solid;
    else if (image->common.type == BITS)
	*code = image->bits.format;
    else
	*code = PIXMAN_unknown;
}

static force_inline pixman_bool_t
image_covers (pixman_image_t *image,
              pixman_box32_t *extents,
              int             x,
              int             y)
{
    if (image->common.type == BITS &&
	image->common.repeat == PIXMAN_REPEAT_NONE)
    {
	if (x > extents->x1 || y > extents->y1 ||
	    x + image->bits.width < extents->x2 ||
	    y + image->bits.height < extents->y2)
	{
	    return FALSE;
	}
    }

    return TRUE;
}

static void
do_composite (pixman_implementation_t *imp,
	      pixman_op_t	       op,
	      pixman_image_t	      *src,
	      pixman_image_t	      *mask,
	      pixman_image_t	      *dest,
	      int		       src_x,
	      int		       src_y,
	      int		       mask_x,
	      int		       mask_y,
	      int		       dest_x,
	      int		       dest_y,
	      int		       width,
	      int		       height)
{
    pixman_format_code_t src_format, mask_format, dest_format;
    uint32_t src_flags, mask_flags, dest_flags;
    pixman_bool_t src_repeat, mask_repeat;
    pixman_region32_t region;
    pixman_box32_t *extents;

    get_image_info (src,  &src_format,  &src_flags);
    if (mask)
    {
	get_image_info (mask, &mask_format, &mask_flags);
    }
    else
    {
	mask_format = PIXMAN_null;
	mask_flags = 0;
    }
    get_image_info (dest, &dest_format, &dest_flags);
    
    /* Check for pixbufs */
    if ((mask_format == PIXMAN_a8r8g8b8 || mask_format == PIXMAN_a8b8g8r8) &&
	(src->type == BITS && src->bits.bits == mask->bits.bits)	   &&
	(src->common.repeat == mask->common.repeat)			   &&
	(src_x == mask_x && src_y == mask_y))
    {
	if (src_format == PIXMAN_x8b8g8r8)
	    src_format = mask_format = PIXMAN_pixbuf;
	else if (src_format == PIXMAN_x8r8g8b8)
	    src_format = mask_format = PIXMAN_rpixbuf;
    }
	    
    src_repeat =
	src->type == BITS					&&
	src_flags & FAST_PATH_ID_TRANSFORM			&&
	src->common.repeat == PIXMAN_REPEAT_NORMAL		&&
	src_format != PIXMAN_solid;
    
    mask_repeat =
	mask							&&
	mask->type == BITS					&&
	mask_flags & FAST_PATH_ID_TRANSFORM			&&
	mask->common.repeat == PIXMAN_REPEAT_NORMAL		&&
	mask_format != PIXMAN_solid;
    
    pixman_region32_init (&region);
    
    if (!pixman_compute_composite_region32 (
	    &region, src, mask, dest,
	    src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height))
    {
	return;
    }
    
    extents = pixman_region32_extents (&region);
    
    if (image_covers (src, extents, dest_x - src_x, dest_y - src_y))
	src_flags |= FAST_PATH_COVERS_CLIP;
    
    if (mask && image_covers (mask, extents, dest_x - mask_x, dest_y - mask_y))
	mask_flags |= FAST_PATH_COVERS_CLIP;
	    
    while (imp)
    {
	const pixman_fast_path_t *info;
	    
	for (info = imp->fast_paths; info->op != PIXMAN_OP_NONE; ++info)
	{
	    if ((info->op == op || info->op == PIXMAN_OP_any)		&&
		/* src */
		((info->src_format == src_format) ||
		 (info->src_format == PIXMAN_any))			&&
		(info->src_flags & src_flags) == info->src_flags	&&
		/* mask */
		((info->mask_format == mask_format) ||
		 (info->mask_format == PIXMAN_any))			&&
		(info->mask_flags & mask_flags) == info->mask_flags	&&
		/* dest */
		((info->dest_format == dest_format) ||
		 (info->dest_format == PIXMAN_any))			&&
		(info->dest_flags & dest_flags) == info->dest_flags)
	    {
		walk_region_internal (imp, op,
				      src, mask, dest,
				      src_x, src_y, mask_x, mask_y,
				      dest_x, dest_y,
				      width, height,
				      src_repeat, mask_repeat,
				      &region,
				      info->func);
		
		goto done;
	    }
	}
	
	imp = imp->delegate;
    }

done:
    pixman_region32_fini (&region);
}

/*
 * Work around GCC bug causing crashes in Mozilla with SSE2
 *
 * When using -msse, gcc generates movdqa instructions assuming that
 * the stack is 16 byte aligned. Unfortunately some applications, such
 * as Mozilla and Mono, end up aligning the stack to 4 bytes, which
 * causes the movdqa instructions to fail.
 *
 * The __force_align_arg_pointer__ makes gcc generate a prologue that
 * realigns the stack pointer to 16 bytes.
 *
 * On x86-64 this is not necessary because the standard ABI already
 * calls for a 16 byte aligned stack.
 *
 * See https://bugs.freedesktop.org/show_bug.cgi?id=15693
 */
#if defined (USE_SSE2) && defined(__GNUC__) && !defined(__x86_64__) && !defined(__amd64__)
__attribute__((__force_align_arg_pointer__))
#endif
PIXMAN_EXPORT void
pixman_image_composite (pixman_op_t      op,
                        pixman_image_t * src,
                        pixman_image_t * mask,
                        pixman_image_t * dest,
                        int16_t          src_x,
                        int16_t          src_y,
                        int16_t          mask_x,
                        int16_t          mask_y,
                        int16_t          dest_x,
                        int16_t          dest_y,
                        uint16_t         width,
                        uint16_t         height)
{
    pixman_image_composite32 (op, src, mask, dest, src_x, src_y, 
                              mask_x, mask_y, dest_x, dest_y, width, height);
}

PIXMAN_EXPORT void
pixman_image_composite32 (pixman_op_t      op,
                          pixman_image_t * src,
                          pixman_image_t * mask,
                          pixman_image_t * dest,
                          int32_t          src_x,
                          int32_t          src_y,
                          int32_t          mask_x,
                          int32_t          mask_y,
                          int32_t          dest_x,
                          int32_t          dest_y,
                          int32_t          width,
                          int32_t          height)
{
    uint32_t *src_bits;
    int src_dx, src_dy;
    uint32_t *mask_bits;
    int mask_dx, mask_dy;
    uint32_t *dest_bits;
    int dest_dx, dest_dy;
    pixman_bool_t need_workaround;

    _pixman_image_validate (src);
    if (mask)
	_pixman_image_validate (mask);
    _pixman_image_validate (dest);
    
    /*
     * Check if we can replace our operator by a simpler one
     * if the src or dest are opaque. The output operator should be
     * mathematically equivalent to the source.
     */
    op = pixman_optimize_operator(op, src, mask, dest);
    if (op == PIXMAN_OP_DST		||
	op == PIXMAN_OP_CONJOINT_DST	||
	op == PIXMAN_OP_DISJOINT_DST)
    {
        return;
    }

    if (!imp)
	imp = _pixman_choose_implementation ();

    need_workaround =
	(src->common.need_workaround)			||
	(mask && mask->common.need_workaround)		||
	(dest->common.need_workaround);
   
    if (need_workaround)
    {
	apply_workaround (src, &src_x, &src_y, &src_bits, &src_dx, &src_dy);
	apply_workaround (mask, &mask_x, &mask_y, &mask_bits, &mask_dx, &mask_dy);
	apply_workaround (dest, &dest_x, &dest_y, &dest_bits, &dest_dx, &dest_dy);
    }

    do_composite (imp, op,
		  src, mask, dest,
		  src_x, src_y,
		  mask_x, mask_y,
		  dest_x, dest_y,
		  width, height);
    
    if (need_workaround)
    {
	if (src->common.need_workaround)
	    unapply_workaround (src, src_bits, src_dx, src_dy);
	if (mask && mask->common.need_workaround)
	    unapply_workaround (mask, mask_bits, mask_dx, mask_dy);
	if (dest->common.need_workaround)
	    unapply_workaround (dest, dest_bits, dest_dx, dest_dy);
    }
}

PIXMAN_EXPORT pixman_bool_t
pixman_blt (uint32_t *src_bits,
            uint32_t *dst_bits,
            int       src_stride,
            int       dst_stride,
            int       src_bpp,
            int       dst_bpp,
            int       src_x,
            int       src_y,
            int       dst_x,
            int       dst_y,
            int       width,
            int       height)
{
    if (!imp)
	imp = _pixman_choose_implementation ();

    return _pixman_implementation_blt (imp, src_bits, dst_bits, src_stride, dst_stride,
                                       src_bpp, dst_bpp,
                                       src_x, src_y,
                                       dst_x, dst_y,
                                       width, height);
}

PIXMAN_EXPORT pixman_bool_t
pixman_fill (uint32_t *bits,
             int       stride,
             int       bpp,
             int       x,
             int       y,
             int       width,
             int       height,
             uint32_t xor)
{
    if (!imp)
	imp = _pixman_choose_implementation ();

    return _pixman_implementation_fill (imp, bits, stride, bpp, x, y, width, height, xor);
}

static uint32_t
color_to_uint32 (const pixman_color_t *color)
{
    return
        (color->alpha >> 8 << 24) |
        (color->red >> 8 << 16) |
        (color->green & 0xff00) |
        (color->blue >> 8);
}

static pixman_bool_t
color_to_pixel (pixman_color_t *     color,
                uint32_t *           pixel,
                pixman_format_code_t format)
{
    uint32_t c = color_to_uint32 (color);

    if (!(format == PIXMAN_a8r8g8b8     ||
          format == PIXMAN_x8r8g8b8     ||
          format == PIXMAN_a8b8g8r8     ||
          format == PIXMAN_x8b8g8r8     ||
          format == PIXMAN_b8g8r8a8     ||
          format == PIXMAN_b8g8r8x8     ||
          format == PIXMAN_r5g6b5       ||
          format == PIXMAN_b5g6r5       ||
          format == PIXMAN_a8))
    {
	return FALSE;
    }

    if (PIXMAN_FORMAT_TYPE (format) == PIXMAN_TYPE_ABGR)
    {
	c = ((c & 0xff000000) >>  0) |
	    ((c & 0x00ff0000) >> 16) |
	    ((c & 0x0000ff00) >>  0) |
	    ((c & 0x000000ff) << 16);
    }
    if (PIXMAN_FORMAT_TYPE (format) == PIXMAN_TYPE_BGRA)
    {
	c = ((c & 0xff000000) >> 24) |
	    ((c & 0x00ff0000) >>  8) |
	    ((c & 0x0000ff00) <<  8) |
	    ((c & 0x000000ff) << 24);
    }

    if (format == PIXMAN_a8)
	c = c >> 24;
    else if (format == PIXMAN_r5g6b5 ||
             format == PIXMAN_b5g6r5)
	c = CONVERT_8888_TO_0565 (c);

#if 0
    printf ("color: %x %x %x %x\n", color->alpha, color->red, color->green, color->blue);
    printf ("pixel: %x\n", c);
#endif

    *pixel = c;
    return TRUE;
}

PIXMAN_EXPORT pixman_bool_t
pixman_image_fill_rectangles (pixman_op_t                 op,
                              pixman_image_t *            dest,
                              pixman_color_t *            color,
                              int                         n_rects,
                              const pixman_rectangle16_t *rects)
{
    pixman_box32_t stack_boxes[6];
    pixman_box32_t *boxes;
    pixman_bool_t result;
    int i;

    if (n_rects > 6)
    {
        boxes = pixman_malloc_ab (sizeof (pixman_box32_t), n_rects);
        if (boxes == NULL)
            return FALSE;
    }
    else
    {
        boxes = stack_boxes;
    }

    for (i = 0; i < n_rects; ++i)
    {
        boxes[i].x1 = rects[i].x;
        boxes[i].y1 = rects[i].y;
        boxes[i].x2 = boxes[i].x1 + rects[i].width;
        boxes[i].y2 = boxes[i].y1 + rects[i].height;
    }

    result = pixman_image_fill_boxes (op, dest, color, n_rects, boxes);

    if (boxes != stack_boxes)
        free (boxes);
    
    return result;
}

PIXMAN_EXPORT pixman_bool_t
pixman_image_fill_boxes (pixman_op_t           op,
                         pixman_image_t *      dest,
                         pixman_color_t *      color,
                         int                   n_boxes,
                         const pixman_box32_t *boxes)
{
    pixman_image_t *solid;
    pixman_color_t c;
    int i;

    _pixman_image_validate (dest);
    
    if (color->alpha == 0xffff)
    {
        if (op == PIXMAN_OP_OVER)
            op = PIXMAN_OP_SRC;
    }

    if (op == PIXMAN_OP_CLEAR)
    {
        c.red = 0;
        c.green = 0;
        c.blue = 0;
        c.alpha = 0;

        color = &c;

        op = PIXMAN_OP_SRC;
    }

    if (op == PIXMAN_OP_SRC)
    {
        uint32_t pixel;

        if (color_to_pixel (color, &pixel, dest->bits.format))
        {
            pixman_region32_t fill_region;
            int n_rects, j;
            pixman_box32_t *rects;

            if (!pixman_region32_init_rects (&fill_region, boxes, n_boxes))
                return FALSE;

            if (dest->common.have_clip_region)
            {
                if (!pixman_region32_intersect (&fill_region,
                                                &fill_region,
                                                &dest->common.clip_region))
                    return FALSE;
            }

            rects = pixman_region32_rectangles (&fill_region, &n_rects);
            for (j = 0; j < n_rects; ++j)
            {
                const pixman_box32_t *rect = &(rects[j]);
                pixman_fill (dest->bits.bits, dest->bits.rowstride, PIXMAN_FORMAT_BPP (dest->bits.format),
                             rect->x1, rect->y1, rect->x2 - rect->x1, rect->y2 - rect->y1,
                             pixel);
            }

            pixman_region32_fini (&fill_region);
            return TRUE;
        }
    }

    solid = pixman_image_create_solid_fill (color);
    if (!solid)
        return FALSE;

    for (i = 0; i < n_boxes; ++i)
    {
        const pixman_box32_t *box = &(boxes[i]);

        pixman_image_composite32 (op, solid, NULL, dest,
                                  0, 0, 0, 0,
                                  box->x1, box->y1,
                                  box->x2 - box->x1, box->y2 - box->y1);
    }

    pixman_image_unref (solid);

    return TRUE;
}

/**
 * pixman_version:
 *
 * Returns the version of the pixman library encoded in a single
 * integer as per %PIXMAN_VERSION_ENCODE. The encoding ensures that
 * later versions compare greater than earlier versions.
 *
 * A run-time comparison to check that pixman's version is greater than
 * or equal to version X.Y.Z could be performed as follows:
 *
 * <informalexample><programlisting>
 * if (pixman_version() >= PIXMAN_VERSION_ENCODE(X,Y,Z)) {...}
 * </programlisting></informalexample>
 *
 * See also pixman_version_string() as well as the compile-time
 * equivalents %PIXMAN_VERSION and %PIXMAN_VERSION_STRING.
 *
 * Return value: the encoded version.
 **/
PIXMAN_EXPORT int
pixman_version (void)
{
    return PIXMAN_VERSION;
}

/**
 * pixman_version_string:
 *
 * Returns the version of the pixman library as a human-readable string
 * of the form "X.Y.Z".
 *
 * See also pixman_version() as well as the compile-time equivalents
 * %PIXMAN_VERSION_STRING and %PIXMAN_VERSION.
 *
 * Return value: a string containing the version.
 **/
PIXMAN_EXPORT const char*
pixman_version_string (void)
{
    return PIXMAN_VERSION_STRING;
}

/**
 * pixman_format_supported_source:
 * @format: A pixman_format_code_t format
 *
 * Return value: whether the provided format code is a supported
 * format for a pixman surface used as a source in
 * rendering.
 *
 * Currently, all pixman_format_code_t values are supported.
 **/
PIXMAN_EXPORT pixman_bool_t
pixman_format_supported_source (pixman_format_code_t format)
{
    switch (format)
    {
    /* 32 bpp formats */
    case PIXMAN_a2b10g10r10:
    case PIXMAN_x2b10g10r10:
    case PIXMAN_a2r10g10b10:
    case PIXMAN_x2r10g10b10:
    case PIXMAN_a8r8g8b8:
    case PIXMAN_x8r8g8b8:
    case PIXMAN_a8b8g8r8:
    case PIXMAN_x8b8g8r8:
    case PIXMAN_b8g8r8a8:
    case PIXMAN_b8g8r8x8:
    case PIXMAN_r8g8b8:
    case PIXMAN_b8g8r8:
    case PIXMAN_r5g6b5:
    case PIXMAN_b5g6r5:
    /* 16 bpp formats */
    case PIXMAN_a1r5g5b5:
    case PIXMAN_x1r5g5b5:
    case PIXMAN_a1b5g5r5:
    case PIXMAN_x1b5g5r5:
    case PIXMAN_a4r4g4b4:
    case PIXMAN_x4r4g4b4:
    case PIXMAN_a4b4g4r4:
    case PIXMAN_x4b4g4r4:
    /* 8bpp formats */
    case PIXMAN_a8:
    case PIXMAN_r3g3b2:
    case PIXMAN_b2g3r3:
    case PIXMAN_a2r2g2b2:
    case PIXMAN_a2b2g2r2:
    case PIXMAN_c8:
    case PIXMAN_g8:
    case PIXMAN_x4a4:
    /* Collides with PIXMAN_c8
       case PIXMAN_x4c4:
     */
    /* Collides with PIXMAN_g8
       case PIXMAN_x4g4:
     */
    /* 4bpp formats */
    case PIXMAN_a4:
    case PIXMAN_r1g2b1:
    case PIXMAN_b1g2r1:
    case PIXMAN_a1r1g1b1:
    case PIXMAN_a1b1g1r1:
    case PIXMAN_c4:
    case PIXMAN_g4:
    /* 1bpp formats */
    case PIXMAN_a1:
    case PIXMAN_g1:
    /* YUV formats */
    case PIXMAN_yuy2:
    case PIXMAN_yv12:
	return TRUE;

    default:
	return FALSE;
    }
}

/**
 * pixman_format_supported_destination:
 * @format: A pixman_format_code_t format
 *
 * Return value: whether the provided format code is a supported
 * format for a pixman surface used as a destination in
 * rendering.
 *
 * Currently, all pixman_format_code_t values are supported
 * except for the YUV formats.
 **/
PIXMAN_EXPORT pixman_bool_t
pixman_format_supported_destination (pixman_format_code_t format)
{
    /* YUV formats cannot be written to at the moment */
    if (format == PIXMAN_yuy2 || format == PIXMAN_yv12)
	return FALSE;

    return pixman_format_supported_source (format);
}

PIXMAN_EXPORT pixman_bool_t
pixman_compute_composite_region (pixman_region16_t * region,
                                 pixman_image_t *    src_image,
                                 pixman_image_t *    mask_image,
                                 pixman_image_t *    dst_image,
                                 int16_t             src_x,
                                 int16_t             src_y,
                                 int16_t             mask_x,
                                 int16_t             mask_y,
                                 int16_t             dest_x,
                                 int16_t             dest_y,
                                 uint16_t            width,
                                 uint16_t            height)
{
    pixman_region32_t r32;
    pixman_bool_t retval;

    pixman_region32_init (&r32);

    retval = pixman_compute_composite_region32 (
	&r32, src_image, mask_image, dst_image,
	src_x, src_y, mask_x, mask_y, dest_x, dest_y,
	width, height);

    if (retval)
    {
	if (!pixman_region16_copy_from_region32 (region, &r32))
	    retval = FALSE;
    }

    pixman_region32_fini (&r32);
    return retval;
}
