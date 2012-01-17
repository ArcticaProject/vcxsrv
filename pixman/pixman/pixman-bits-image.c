/*
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *             2005 Lars Knoll & Zack Rusin, Trolltech
 *             2008 Aaron Plattner, NVIDIA Corporation
 * Copyright © 2000 SuSE, Inc.
 * Copyright © 2007, 2009 Red Hat, Inc.
 * Copyright © 2008 André Tupinambá <andrelrt@gmail.com>
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
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pixman-private.h"
#include "pixman-combine32.h"
#include "pixman-inlines.h"

/*
 * By default, just evaluate the image at 32bpp and expand.  Individual image
 * types can plug in a better scanline getter if they want to. For example
 * we  could produce smoother gradients by evaluating them at higher color
 * depth, but that's a project for the future.
 */
static void
_pixman_image_get_scanline_generic_64 (pixman_image_t * image,
                                       int              x,
                                       int              y,
                                       int              width,
                                       uint32_t *       buffer,
                                       const uint32_t * mask)
{
    uint32_t *mask8 = NULL;

    /* Contract the mask image, if one exists, so that the 32-bit fetch
     * function can use it.
     */
    if (mask)
    {
	mask8 = pixman_malloc_ab (width, sizeof(uint32_t));
	if (!mask8)
	    return;

	pixman_contract (mask8, (uint64_t *)mask, width);
    }

    /* Fetch the source image into the first half of buffer. */
    image->bits.get_scanline_32 (image, x, y, width, (uint32_t*)buffer, mask8);

    /* Expand from 32bpp to 64bpp in place. */
    pixman_expand ((uint64_t *)buffer, buffer, PIXMAN_a8r8g8b8, width);

    free (mask8);
}

/* Fetch functions */

static force_inline uint32_t
fetch_pixel_no_alpha (bits_image_t *image,
		      int x, int y, pixman_bool_t check_bounds)
{
    if (check_bounds &&
	(x < 0 || x >= image->width || y < 0 || y >= image->height))
    {
	return 0;
    }

    return image->fetch_pixel_32 (image, x, y);
}

typedef uint32_t (* get_pixel_t) (bits_image_t *image,
				  int x, int y, pixman_bool_t check_bounds);

static force_inline uint32_t
bits_image_fetch_pixel_nearest (bits_image_t   *image,
				pixman_fixed_t  x,
				pixman_fixed_t  y,
				get_pixel_t	get_pixel)
{
    int x0 = pixman_fixed_to_int (x - pixman_fixed_e);
    int y0 = pixman_fixed_to_int (y - pixman_fixed_e);

    if (image->common.repeat != PIXMAN_REPEAT_NONE)
    {
	repeat (image->common.repeat, &x0, image->width);
	repeat (image->common.repeat, &y0, image->height);

	return get_pixel (image, x0, y0, FALSE);
    }
    else
    {
	return get_pixel (image, x0, y0, TRUE);
    }
}

static force_inline uint32_t
bits_image_fetch_pixel_bilinear (bits_image_t   *image,
				 pixman_fixed_t  x,
				 pixman_fixed_t  y,
				 get_pixel_t	 get_pixel)
{
    pixman_repeat_t repeat_mode = image->common.repeat;
    int width = image->width;
    int height = image->height;
    int x1, y1, x2, y2;
    uint32_t tl, tr, bl, br;
    int32_t distx, disty;

    x1 = x - pixman_fixed_1 / 2;
    y1 = y - pixman_fixed_1 / 2;

    distx = (x1 >> 8) & 0xff;
    disty = (y1 >> 8) & 0xff;

    x1 = pixman_fixed_to_int (x1);
    y1 = pixman_fixed_to_int (y1);
    x2 = x1 + 1;
    y2 = y1 + 1;

    if (repeat_mode != PIXMAN_REPEAT_NONE)
    {
	repeat (repeat_mode, &x1, width);
	repeat (repeat_mode, &y1, height);
	repeat (repeat_mode, &x2, width);
	repeat (repeat_mode, &y2, height);

	tl = get_pixel (image, x1, y1, FALSE);
	bl = get_pixel (image, x1, y2, FALSE);
	tr = get_pixel (image, x2, y1, FALSE);
	br = get_pixel (image, x2, y2, FALSE);
    }
    else
    {
	tl = get_pixel (image, x1, y1, TRUE);
	tr = get_pixel (image, x2, y1, TRUE);
	bl = get_pixel (image, x1, y2, TRUE);
	br = get_pixel (image, x2, y2, TRUE);
    }

    return bilinear_interpolation (tl, tr, bl, br, distx, disty);
}

static void
bits_image_fetch_bilinear_no_repeat_8888 (pixman_image_t * ima,
					  int              offset,
					  int              line,
					  int              width,
					  uint32_t *       buffer,
					  const uint32_t * mask)
{
    bits_image_t *bits = &ima->bits;
    pixman_fixed_t x_top, x_bottom, x;
    pixman_fixed_t ux_top, ux_bottom, ux;
    pixman_vector_t v;
    uint32_t top_mask, bottom_mask;
    uint32_t *top_row;
    uint32_t *bottom_row;
    uint32_t *end;
    uint32_t zero[2] = { 0, 0 };
    uint32_t one = 1;
    int y, y1, y2;
    int disty;
    int mask_inc;
    int w;

    /* reference point is the center of the pixel */
    v.vector[0] = pixman_int_to_fixed (offset) + pixman_fixed_1 / 2;
    v.vector[1] = pixman_int_to_fixed (line) + pixman_fixed_1 / 2;
    v.vector[2] = pixman_fixed_1;

    if (!pixman_transform_point_3d (bits->common.transform, &v))
	return;

    ux = ux_top = ux_bottom = bits->common.transform->matrix[0][0];
    x = x_top = x_bottom = v.vector[0] - pixman_fixed_1/2;

    y = v.vector[1] - pixman_fixed_1/2;
    disty = (y >> 8) & 0xff;

    /* Load the pointers to the first and second lines from the source
     * image that bilinear code must read.
     *
     * The main trick in this code is about the check if any line are
     * outside of the image;
     *
     * When I realize that a line (any one) is outside, I change
     * the pointer to a dummy area with zeros. Once I change this, I
     * must be sure the pointer will not change, so I set the
     * variables to each pointer increments inside the loop.
     */
    y1 = pixman_fixed_to_int (y);
    y2 = y1 + 1;

    if (y1 < 0 || y1 >= bits->height)
    {
	top_row = zero;
	x_top = 0;
	ux_top = 0;
    }
    else
    {
	top_row = bits->bits + y1 * bits->rowstride;
	x_top = x;
	ux_top = ux;
    }

    if (y2 < 0 || y2 >= bits->height)
    {
	bottom_row = zero;
	x_bottom = 0;
	ux_bottom = 0;
    }
    else
    {
	bottom_row = bits->bits + y2 * bits->rowstride;
	x_bottom = x;
	ux_bottom = ux;
    }

    /* Instead of checking whether the operation uses the mast in
     * each loop iteration, verify this only once and prepare the
     * variables to make the code smaller inside the loop.
     */
    if (!mask)
    {
        mask_inc = 0;
        mask = &one;
    }
    else
    {
        /* If have a mask, prepare the variables to check it */
        mask_inc = 1;
    }

    /* If both are zero, then the whole thing is zero */
    if (top_row == zero && bottom_row == zero)
    {
	memset (buffer, 0, width * sizeof (uint32_t));
	return;
    }
    else if (bits->format == PIXMAN_x8r8g8b8)
    {
	if (top_row == zero)
	{
	    top_mask = 0;
	    bottom_mask = 0xff000000;
	}
	else if (bottom_row == zero)
	{
	    top_mask = 0xff000000;
	    bottom_mask = 0;
	}
	else
	{
	    top_mask = 0xff000000;
	    bottom_mask = 0xff000000;
	}
    }
    else
    {
	top_mask = 0;
	bottom_mask = 0;
    }

    end = buffer + width;

    /* Zero fill to the left of the image */
    while (buffer < end && x < pixman_fixed_minus_1)
    {
	*buffer++ = 0;
	x += ux;
	x_top += ux_top;
	x_bottom += ux_bottom;
	mask += mask_inc;
    }

    /* Left edge
     */
    while (buffer < end && x < 0)
    {
	uint32_t tr, br;
	int32_t distx;

	tr = top_row[pixman_fixed_to_int (x_top) + 1] | top_mask;
	br = bottom_row[pixman_fixed_to_int (x_bottom) + 1] | bottom_mask;

	distx = (x >> 8) & 0xff;

	*buffer++ = bilinear_interpolation (0, tr, 0, br, distx, disty);

	x += ux;
	x_top += ux_top;
	x_bottom += ux_bottom;
	mask += mask_inc;
    }

    /* Main part */
    w = pixman_int_to_fixed (bits->width - 1);

    while (buffer < end  &&  x < w)
    {
	if (*mask)
	{
	    uint32_t tl, tr, bl, br;
	    int32_t distx;

	    tl = top_row [pixman_fixed_to_int (x_top)] | top_mask;
	    tr = top_row [pixman_fixed_to_int (x_top) + 1] | top_mask;
	    bl = bottom_row [pixman_fixed_to_int (x_bottom)] | bottom_mask;
	    br = bottom_row [pixman_fixed_to_int (x_bottom) + 1] | bottom_mask;

	    distx = (x >> 8) & 0xff;

	    *buffer = bilinear_interpolation (tl, tr, bl, br, distx, disty);
	}

	buffer++;
	x += ux;
	x_top += ux_top;
	x_bottom += ux_bottom;
	mask += mask_inc;
    }

    /* Right Edge */
    w = pixman_int_to_fixed (bits->width);
    while (buffer < end  &&  x < w)
    {
	if (*mask)
	{
	    uint32_t tl, bl;
	    int32_t distx;

	    tl = top_row [pixman_fixed_to_int (x_top)] | top_mask;
	    bl = bottom_row [pixman_fixed_to_int (x_bottom)] | bottom_mask;

	    distx = (x >> 8) & 0xff;

	    *buffer = bilinear_interpolation (tl, 0, bl, 0, distx, disty);
	}

	buffer++;
	x += ux;
	x_top += ux_top;
	x_bottom += ux_bottom;
	mask += mask_inc;
    }

    /* Zero fill to the left of the image */
    while (buffer < end)
	*buffer++ = 0;
}

static force_inline uint32_t
bits_image_fetch_pixel_convolution (bits_image_t   *image,
				    pixman_fixed_t  x,
				    pixman_fixed_t  y,
				    get_pixel_t     get_pixel)
{
    pixman_fixed_t *params = image->common.filter_params;
    int x_off = (params[0] - pixman_fixed_1) >> 1;
    int y_off = (params[1] - pixman_fixed_1) >> 1;
    int32_t cwidth = pixman_fixed_to_int (params[0]);
    int32_t cheight = pixman_fixed_to_int (params[1]);
    int32_t srtot, sgtot, sbtot, satot;
    int32_t i, j, x1, x2, y1, y2;
    pixman_repeat_t repeat_mode = image->common.repeat;
    int width = image->width;
    int height = image->height;

    params += 2;

    x1 = pixman_fixed_to_int (x - pixman_fixed_e - x_off);
    y1 = pixman_fixed_to_int (y - pixman_fixed_e - y_off);
    x2 = x1 + cwidth;
    y2 = y1 + cheight;

    srtot = sgtot = sbtot = satot = 0;

    for (i = y1; i < y2; ++i)
    {
	for (j = x1; j < x2; ++j)
	{
	    int rx = j;
	    int ry = i;

	    pixman_fixed_t f = *params;

	    if (f)
	    {
		uint32_t pixel;

		if (repeat_mode != PIXMAN_REPEAT_NONE)
		{
		    repeat (repeat_mode, &rx, width);
		    repeat (repeat_mode, &ry, height);

		    pixel = get_pixel (image, rx, ry, FALSE);
		}
		else
		{
		    pixel = get_pixel (image, rx, ry, TRUE);
		}

		srtot += RED_8 (pixel) * f;
		sgtot += GREEN_8 (pixel) * f;
		sbtot += BLUE_8 (pixel) * f;
		satot += ALPHA_8 (pixel) * f;
	    }

	    params++;
	}
    }

    satot >>= 16;
    srtot >>= 16;
    sgtot >>= 16;
    sbtot >>= 16;

    satot = CLIP (satot, 0, 0xff);
    srtot = CLIP (srtot, 0, 0xff);
    sgtot = CLIP (sgtot, 0, 0xff);
    sbtot = CLIP (sbtot, 0, 0xff);

    return ((satot << 24) | (srtot << 16) | (sgtot <<  8) | (sbtot));
}

static force_inline uint32_t
bits_image_fetch_pixel_filtered (bits_image_t *image,
				 pixman_fixed_t x,
				 pixman_fixed_t y,
				 get_pixel_t    get_pixel)
{
    switch (image->common.filter)
    {
    case PIXMAN_FILTER_NEAREST:
    case PIXMAN_FILTER_FAST:
	return bits_image_fetch_pixel_nearest (image, x, y, get_pixel);
	break;

    case PIXMAN_FILTER_BILINEAR:
    case PIXMAN_FILTER_GOOD:
    case PIXMAN_FILTER_BEST:
	return bits_image_fetch_pixel_bilinear (image, x, y, get_pixel);
	break;

    case PIXMAN_FILTER_CONVOLUTION:
	return bits_image_fetch_pixel_convolution (image, x, y, get_pixel);
	break;

    default:
        break;
    }

    return 0;
}

static void
bits_image_fetch_affine_no_alpha (pixman_image_t * image,
				  int              offset,
				  int              line,
				  int              width,
				  uint32_t *       buffer,
				  const uint32_t * mask)
{
    pixman_fixed_t x, y;
    pixman_fixed_t ux, uy;
    pixman_vector_t v;
    int i;

    /* reference point is the center of the pixel */
    v.vector[0] = pixman_int_to_fixed (offset) + pixman_fixed_1 / 2;
    v.vector[1] = pixman_int_to_fixed (line) + pixman_fixed_1 / 2;
    v.vector[2] = pixman_fixed_1;

    if (image->common.transform)
    {
	if (!pixman_transform_point_3d (image->common.transform, &v))
	    return;

	ux = image->common.transform->matrix[0][0];
	uy = image->common.transform->matrix[1][0];
    }
    else
    {
	ux = pixman_fixed_1;
	uy = 0;
    }

    x = v.vector[0];
    y = v.vector[1];

    for (i = 0; i < width; ++i)
    {
	if (!mask || mask[i])
	{
	    buffer[i] = bits_image_fetch_pixel_filtered (
		&image->bits, x, y, fetch_pixel_no_alpha);
	}

	x += ux;
	y += uy;
    }
}

/* General fetcher */
static force_inline uint32_t
fetch_pixel_general (bits_image_t *image, int x, int y, pixman_bool_t check_bounds)
{
    uint32_t pixel;

    if (check_bounds &&
	(x < 0 || x >= image->width || y < 0 || y >= image->height))
    {
	return 0;
    }

    pixel = image->fetch_pixel_32 (image, x, y);

    if (image->common.alpha_map)
    {
	uint32_t pixel_a;

	x -= image->common.alpha_origin_x;
	y -= image->common.alpha_origin_y;

	if (x < 0 || x >= image->common.alpha_map->width ||
	    y < 0 || y >= image->common.alpha_map->height)
	{
	    pixel_a = 0;
	}
	else
	{
	    pixel_a = image->common.alpha_map->fetch_pixel_32 (
		image->common.alpha_map, x, y);

	    pixel_a = ALPHA_8 (pixel_a);
	}

	pixel &= 0x00ffffff;
	pixel |= (pixel_a << 24);
    }

    return pixel;
}

static void
bits_image_fetch_general (pixman_image_t * image,
			  int              offset,
			  int              line,
			  int              width,
			  uint32_t *       buffer,
			  const uint32_t * mask)
{
    pixman_fixed_t x, y, w;
    pixman_fixed_t ux, uy, uw;
    pixman_vector_t v;
    int i;

    /* reference point is the center of the pixel */
    v.vector[0] = pixman_int_to_fixed (offset) + pixman_fixed_1 / 2;
    v.vector[1] = pixman_int_to_fixed (line) + pixman_fixed_1 / 2;
    v.vector[2] = pixman_fixed_1;

    if (image->common.transform)
    {
	if (!pixman_transform_point_3d (image->common.transform, &v))
	    return;

	ux = image->common.transform->matrix[0][0];
	uy = image->common.transform->matrix[1][0];
	uw = image->common.transform->matrix[2][0];
    }
    else
    {
	ux = pixman_fixed_1;
	uy = 0;
	uw = 0;
    }

    x = v.vector[0];
    y = v.vector[1];
    w = v.vector[2];

    for (i = 0; i < width; ++i)
    {
	pixman_fixed_t x0, y0;

	if (!mask || mask[i])
	{
	    if (w != 0)
	    {
		x0 = ((pixman_fixed_48_16_t)x << 16) / w;
		y0 = ((pixman_fixed_48_16_t)y << 16) / w;
	    }
	    else
	    {
		x0 = 0;
		y0 = 0;
	    }

	    buffer[i] = bits_image_fetch_pixel_filtered (
		&image->bits, x0, y0, fetch_pixel_general);
	}

	x += ux;
	y += uy;
	w += uw;
    }
}

static const uint8_t zero[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

typedef uint32_t (* convert_pixel_t) (const uint8_t *row, int x);

static force_inline void
bits_image_fetch_bilinear_affine (pixman_image_t * image,
				  int              offset,
				  int              line,
				  int              width,
				  uint32_t *       buffer,
				  const uint32_t * mask,

				  convert_pixel_t	convert_pixel,
				  pixman_format_code_t	format,
				  pixman_repeat_t	repeat_mode)
{
    pixman_fixed_t x, y;
    pixman_fixed_t ux, uy;
    pixman_vector_t v;
    bits_image_t *bits = &image->bits;
    int i;

    /* reference point is the center of the pixel */
    v.vector[0] = pixman_int_to_fixed (offset) + pixman_fixed_1 / 2;
    v.vector[1] = pixman_int_to_fixed (line) + pixman_fixed_1 / 2;
    v.vector[2] = pixman_fixed_1;

    if (!pixman_transform_point_3d (image->common.transform, &v))
	return;

    ux = image->common.transform->matrix[0][0];
    uy = image->common.transform->matrix[1][0];

    x = v.vector[0];
    y = v.vector[1];

    for (i = 0; i < width; ++i)
    {
	int x1, y1, x2, y2;
	uint32_t tl, tr, bl, br;
	int32_t distx, disty;
	int width = image->bits.width;
	int height = image->bits.height;
	const uint8_t *row1;
	const uint8_t *row2;

	if (mask && !mask[i])
	    goto next;

	x1 = x - pixman_fixed_1 / 2;
	y1 = y - pixman_fixed_1 / 2;

	distx = (x1 >> 8) & 0xff;
	disty = (y1 >> 8) & 0xff;

	y1 = pixman_fixed_to_int (y1);
	y2 = y1 + 1;
	x1 = pixman_fixed_to_int (x1);
	x2 = x1 + 1;

	if (repeat_mode != PIXMAN_REPEAT_NONE)
	{
	    uint32_t mask;

	    mask = PIXMAN_FORMAT_A (format)? 0 : 0xff000000;

	    repeat (repeat_mode, &x1, width);
	    repeat (repeat_mode, &y1, height);
	    repeat (repeat_mode, &x2, width);
	    repeat (repeat_mode, &y2, height);

	    row1 = (uint8_t *)bits->bits + bits->rowstride * 4 * y1;
	    row2 = (uint8_t *)bits->bits + bits->rowstride * 4 * y2;

	    tl = convert_pixel (row1, x1) | mask;
	    tr = convert_pixel (row1, x2) | mask;
	    bl = convert_pixel (row2, x1) | mask;
	    br = convert_pixel (row2, x2) | mask;
	}
	else
	{
	    uint32_t mask1, mask2;
	    int bpp;

	    /* Note: PIXMAN_FORMAT_BPP() returns an unsigned value,
	     * which means if you use it in expressions, those
	     * expressions become unsigned themselves. Since
	     * the variables below can be negative in some cases,
	     * that will lead to crashes on 64 bit architectures.
	     *
	     * So this line makes sure bpp is signed
	     */
	    bpp = PIXMAN_FORMAT_BPP (format);

	    if (x1 >= width || x2 < 0 || y1 >= height || y2 < 0)
	    {
		buffer[i] = 0;
		goto next;
	    }

	    if (y2 == 0)
	    {
		row1 = zero;
		mask1 = 0;
	    }
	    else
	    {
		row1 = (uint8_t *)bits->bits + bits->rowstride * 4 * y1;
		row1 += bpp / 8 * x1;

		mask1 = PIXMAN_FORMAT_A (format)? 0 : 0xff000000;
	    }

	    if (y1 == height - 1)
	    {
		row2 = zero;
		mask2 = 0;
	    }
	    else
	    {
		row2 = (uint8_t *)bits->bits + bits->rowstride * 4 * y2;
		row2 += bpp / 8 * x1;

		mask2 = PIXMAN_FORMAT_A (format)? 0 : 0xff000000;
	    }

	    if (x2 == 0)
	    {
		tl = 0;
		bl = 0;
	    }
	    else
	    {
		tl = convert_pixel (row1, 0) | mask1;
		bl = convert_pixel (row2, 0) | mask2;
	    }

	    if (x1 == width - 1)
	    {
		tr = 0;
		br = 0;
	    }
	    else
	    {
		tr = convert_pixel (row1, 1) | mask1;
		br = convert_pixel (row2, 1) | mask2;
	    }
	}

	buffer[i] = bilinear_interpolation (
	    tl, tr, bl, br, distx, disty);

    next:
	x += ux;
	y += uy;
    }
}

static force_inline void
bits_image_fetch_nearest_affine (pixman_image_t * image,
				 int              offset,
				 int              line,
				 int              width,
				 uint32_t *       buffer,
				 const uint32_t * mask,
				 
				 convert_pixel_t	convert_pixel,
				 pixman_format_code_t	format,
				 pixman_repeat_t	repeat_mode)
{
    pixman_fixed_t x, y;
    pixman_fixed_t ux, uy;
    pixman_vector_t v;
    bits_image_t *bits = &image->bits;
    int i;

    /* reference point is the center of the pixel */
    v.vector[0] = pixman_int_to_fixed (offset) + pixman_fixed_1 / 2;
    v.vector[1] = pixman_int_to_fixed (line) + pixman_fixed_1 / 2;
    v.vector[2] = pixman_fixed_1;

    if (!pixman_transform_point_3d (image->common.transform, &v))
	return;

    ux = image->common.transform->matrix[0][0];
    uy = image->common.transform->matrix[1][0];

    x = v.vector[0];
    y = v.vector[1];

    for (i = 0; i < width; ++i)
    {
	int width, height, x0, y0;
	const uint8_t *row;

	if (mask && !mask[i])
	    goto next;
	
	width = image->bits.width;
	height = image->bits.height;
	x0 = pixman_fixed_to_int (x - pixman_fixed_e);
	y0 = pixman_fixed_to_int (y - pixman_fixed_e);

	if (repeat_mode == PIXMAN_REPEAT_NONE &&
	    (y0 < 0 || y0 >= height || x0 < 0 || x0 >= width))
	{
	    buffer[i] = 0;
	}
	else
	{
	    uint32_t mask = PIXMAN_FORMAT_A (format)? 0 : 0xff000000;

	    if (repeat_mode != PIXMAN_REPEAT_NONE)
	    {
		repeat (repeat_mode, &x0, width);
		repeat (repeat_mode, &y0, height);
	    }

	    row = (uint8_t *)bits->bits + bits->rowstride * 4 * y0;

	    buffer[i] = convert_pixel (row, x0) | mask;
	}

    next:
	x += ux;
	y += uy;
    }
}

static force_inline uint32_t
convert_a8r8g8b8 (const uint8_t *row, int x)
{
    return *(((uint32_t *)row) + x);
}

static force_inline uint32_t
convert_x8r8g8b8 (const uint8_t *row, int x)
{
    return *(((uint32_t *)row) + x);
}

static force_inline uint32_t
convert_a8 (const uint8_t *row, int x)
{
    return *(row + x) << 24;
}

static force_inline uint32_t
convert_r5g6b5 (const uint8_t *row, int x)
{
    return CONVERT_0565_TO_0888 (*((uint16_t *)row + x));
}

#define MAKE_BILINEAR_FETCHER(name, format, repeat_mode)		\
    static void								\
    bits_image_fetch_bilinear_affine_ ## name (pixman_image_t *image,	\
					       int              offset,	\
					       int              line,	\
					       int              width,	\
					       uint32_t *       buffer,	\
					       const uint32_t * mask)	\
    {									\
	bits_image_fetch_bilinear_affine (image, offset, line,		\
					  width, buffer, mask,		\
					  convert_ ## format,		\
					  PIXMAN_ ## format,		\
					  repeat_mode);			\
    }

#define MAKE_NEAREST_FETCHER(name, format, repeat_mode)			\
    static void								\
    bits_image_fetch_nearest_affine_ ## name (pixman_image_t *image,	\
					      int              offset,	\
					      int              line,	\
					      int              width,	\
					      uint32_t *       buffer,	\
					      const uint32_t * mask)	\
    {									\
	bits_image_fetch_nearest_affine (image, offset, line,		\
					 width, buffer, mask,		\
					 convert_ ## format,		\
					 PIXMAN_ ## format,		\
					 repeat_mode);			\
    }

#define MAKE_FETCHERS(name, format, repeat_mode)			\
    MAKE_NEAREST_FETCHER (name, format, repeat_mode)			\
    MAKE_BILINEAR_FETCHER (name, format, repeat_mode)

MAKE_FETCHERS (pad_a8r8g8b8,     a8r8g8b8, PIXMAN_REPEAT_PAD)
MAKE_FETCHERS (none_a8r8g8b8,    a8r8g8b8, PIXMAN_REPEAT_NONE)
MAKE_FETCHERS (reflect_a8r8g8b8, a8r8g8b8, PIXMAN_REPEAT_REFLECT)
MAKE_FETCHERS (normal_a8r8g8b8,  a8r8g8b8, PIXMAN_REPEAT_NORMAL)
MAKE_FETCHERS (pad_x8r8g8b8,     x8r8g8b8, PIXMAN_REPEAT_PAD)
MAKE_FETCHERS (none_x8r8g8b8,    x8r8g8b8, PIXMAN_REPEAT_NONE)
MAKE_FETCHERS (reflect_x8r8g8b8, x8r8g8b8, PIXMAN_REPEAT_REFLECT)
MAKE_FETCHERS (normal_x8r8g8b8,  x8r8g8b8, PIXMAN_REPEAT_NORMAL)
MAKE_FETCHERS (pad_a8,           a8,       PIXMAN_REPEAT_PAD)
MAKE_FETCHERS (none_a8,          a8,       PIXMAN_REPEAT_NONE)
MAKE_FETCHERS (reflect_a8,	 a8,       PIXMAN_REPEAT_REFLECT)
MAKE_FETCHERS (normal_a8,	 a8,       PIXMAN_REPEAT_NORMAL)
MAKE_FETCHERS (pad_r5g6b5,       r5g6b5,   PIXMAN_REPEAT_PAD)
MAKE_FETCHERS (none_r5g6b5,      r5g6b5,   PIXMAN_REPEAT_NONE)
MAKE_FETCHERS (reflect_r5g6b5,   r5g6b5,   PIXMAN_REPEAT_REFLECT)
MAKE_FETCHERS (normal_r5g6b5,    r5g6b5,   PIXMAN_REPEAT_NORMAL)

static void
replicate_pixel_32 (bits_image_t *   bits,
		    int              x,
		    int              y,
		    int              width,
		    uint32_t *       buffer)
{
    uint32_t color;
    uint32_t *end;

    color = bits->fetch_pixel_32 (bits, x, y);

    end = buffer + width;
    while (buffer < end)
	*(buffer++) = color;
}

static void
replicate_pixel_64 (bits_image_t *   bits,
		    int              x,
		    int              y,
		    int              width,
		    uint32_t *       b)
{
    uint64_t color;
    uint64_t *buffer = (uint64_t *)b;
    uint64_t *end;

    color = bits->fetch_pixel_64 (bits, x, y);

    end = buffer + width;
    while (buffer < end)
	*(buffer++) = color;
}

static void
bits_image_fetch_solid_32 (pixman_image_t * image,
                           int              x,
                           int              y,
                           int              width,
                           uint32_t *       buffer,
                           const uint32_t * mask)
{
    replicate_pixel_32 (&image->bits, 0, 0, width, buffer);
}

static void
bits_image_fetch_solid_64 (pixman_image_t * image,
                           int              x,
                           int              y,
                           int              width,
                           uint32_t *       b,
                           const uint32_t * unused)
{
    replicate_pixel_64 (&image->bits, 0, 0, width, b);
}

static void
bits_image_fetch_untransformed_repeat_none (bits_image_t *image,
                                            pixman_bool_t wide,
                                            int           x,
                                            int           y,
                                            int           width,
                                            uint32_t *    buffer)
{
    uint32_t w;

    if (y < 0 || y >= image->height)
    {
	memset (buffer, 0, width * (wide? 8 : 4));
	return;
    }

    if (x < 0)
    {
	w = MIN (width, -x);

	memset (buffer, 0, w * (wide ? 8 : 4));

	width -= w;
	buffer += w * (wide? 2 : 1);
	x += w;
    }

    if (x < image->width)
    {
	w = MIN (width, image->width - x);

	if (wide)
	    image->fetch_scanline_64 ((pixman_image_t *)image, x, y, w, buffer, NULL);
	else
	    image->fetch_scanline_32 ((pixman_image_t *)image, x, y, w, buffer, NULL);

	width -= w;
	buffer += w * (wide? 2 : 1);
	x += w;
    }

    memset (buffer, 0, width * (wide ? 8 : 4));
}

static void
bits_image_fetch_untransformed_repeat_normal (bits_image_t *image,
                                              pixman_bool_t wide,
                                              int           x,
                                              int           y,
                                              int           width,
                                              uint32_t *    buffer)
{
    uint32_t w;

    while (y < 0)
	y += image->height;

    while (y >= image->height)
	y -= image->height;

    if (image->width == 1)
    {
	if (wide)
	    replicate_pixel_64 (image, 0, y, width, buffer);
	else
	    replicate_pixel_32 (image, 0, y, width, buffer);

	return;
    }

    while (width)
    {
	while (x < 0)
	    x += image->width;
	while (x >= image->width)
	    x -= image->width;

	w = MIN (width, image->width - x);

	if (wide)
	    image->fetch_scanline_64 ((pixman_image_t *)image, x, y, w, buffer, NULL);
	else
	    image->fetch_scanline_32 ((pixman_image_t *)image, x, y, w, buffer, NULL);

	buffer += w * (wide? 2 : 1);
	x += w;
	width -= w;
    }
}

static void
bits_image_fetch_untransformed_32 (pixman_image_t * image,
                                   int              x,
                                   int              y,
                                   int              width,
                                   uint32_t *       buffer,
                                   const uint32_t * mask)
{
    if (image->common.repeat == PIXMAN_REPEAT_NONE)
    {
	bits_image_fetch_untransformed_repeat_none (
	    &image->bits, FALSE, x, y, width, buffer);
    }
    else
    {
	bits_image_fetch_untransformed_repeat_normal (
	    &image->bits, FALSE, x, y, width, buffer);
    }
}

static void
bits_image_fetch_untransformed_64 (pixman_image_t * image,
                                   int              x,
                                   int              y,
                                   int              width,
                                   uint32_t *       buffer,
                                   const uint32_t * unused)
{
    if (image->common.repeat == PIXMAN_REPEAT_NONE)
    {
	bits_image_fetch_untransformed_repeat_none (
	    &image->bits, TRUE, x, y, width, buffer);
    }
    else
    {
	bits_image_fetch_untransformed_repeat_normal (
	    &image->bits, TRUE, x, y, width, buffer);
    }
}

typedef struct
{
    pixman_format_code_t	format;
    uint32_t			flags;
    fetch_scanline_t		fetch_32;
    fetch_scanline_t		fetch_64;
} fetcher_info_t;

static const fetcher_info_t fetcher_info[] =
{
    { PIXMAN_solid,
      FAST_PATH_NO_ALPHA_MAP,
      bits_image_fetch_solid_32,
      bits_image_fetch_solid_64
    },

    { PIXMAN_any,
      (FAST_PATH_NO_ALPHA_MAP			|
       FAST_PATH_ID_TRANSFORM			|
       FAST_PATH_NO_CONVOLUTION_FILTER		|
       FAST_PATH_NO_PAD_REPEAT			|
       FAST_PATH_NO_REFLECT_REPEAT),
      bits_image_fetch_untransformed_32,
      bits_image_fetch_untransformed_64
    },

#define FAST_BILINEAR_FLAGS						\
    (FAST_PATH_NO_ALPHA_MAP		|				\
     FAST_PATH_NO_ACCESSORS		|				\
     FAST_PATH_HAS_TRANSFORM		|				\
     FAST_PATH_AFFINE_TRANSFORM		|				\
     FAST_PATH_X_UNIT_POSITIVE		|				\
     FAST_PATH_Y_UNIT_ZERO		|				\
     FAST_PATH_NONE_REPEAT		|				\
     FAST_PATH_BILINEAR_FILTER)

    { PIXMAN_a8r8g8b8,
      FAST_BILINEAR_FLAGS,
      bits_image_fetch_bilinear_no_repeat_8888,
      _pixman_image_get_scanline_generic_64
    },

    { PIXMAN_x8r8g8b8,
      FAST_BILINEAR_FLAGS,
      bits_image_fetch_bilinear_no_repeat_8888,
      _pixman_image_get_scanline_generic_64
    },

#define GENERAL_BILINEAR_FLAGS						\
    (FAST_PATH_NO_ALPHA_MAP		|				\
     FAST_PATH_NO_ACCESSORS		|				\
     FAST_PATH_HAS_TRANSFORM		|				\
     FAST_PATH_AFFINE_TRANSFORM		|				\
     FAST_PATH_BILINEAR_FILTER)

#define GENERAL_NEAREST_FLAGS						\
    (FAST_PATH_NO_ALPHA_MAP		|				\
     FAST_PATH_NO_ACCESSORS		|				\
     FAST_PATH_HAS_TRANSFORM		|				\
     FAST_PATH_AFFINE_TRANSFORM		|				\
     FAST_PATH_NEAREST_FILTER)

#define BILINEAR_AFFINE_FAST_PATH(name, format, repeat)			\
    { PIXMAN_ ## format,						\
      GENERAL_BILINEAR_FLAGS | FAST_PATH_ ## repeat ## _REPEAT,		\
      bits_image_fetch_bilinear_affine_ ## name,			\
      _pixman_image_get_scanline_generic_64				\
    },

#define NEAREST_AFFINE_FAST_PATH(name, format, repeat)			\
    { PIXMAN_ ## format,						\
      GENERAL_NEAREST_FLAGS | FAST_PATH_ ## repeat ## _REPEAT,		\
      bits_image_fetch_nearest_affine_ ## name,			\
      _pixman_image_get_scanline_generic_64				\
    },

#define AFFINE_FAST_PATHS(name, format, repeat)				\
    BILINEAR_AFFINE_FAST_PATH(name, format, repeat)			\
    NEAREST_AFFINE_FAST_PATH(name, format, repeat)
    
    AFFINE_FAST_PATHS (pad_a8r8g8b8, a8r8g8b8, PAD)
    AFFINE_FAST_PATHS (none_a8r8g8b8, a8r8g8b8, NONE)
    AFFINE_FAST_PATHS (reflect_a8r8g8b8, a8r8g8b8, REFLECT)
    AFFINE_FAST_PATHS (normal_a8r8g8b8, a8r8g8b8, NORMAL)
    AFFINE_FAST_PATHS (pad_x8r8g8b8, x8r8g8b8, PAD)
    AFFINE_FAST_PATHS (none_x8r8g8b8, x8r8g8b8, NONE)
    AFFINE_FAST_PATHS (reflect_x8r8g8b8, x8r8g8b8, REFLECT)
    AFFINE_FAST_PATHS (normal_x8r8g8b8, x8r8g8b8, NORMAL)
    AFFINE_FAST_PATHS (pad_a8, a8, PAD)
    AFFINE_FAST_PATHS (none_a8, a8, NONE)
    AFFINE_FAST_PATHS (reflect_a8, a8, REFLECT)
    AFFINE_FAST_PATHS (normal_a8, a8, NORMAL)
    AFFINE_FAST_PATHS (pad_r5g6b5, r5g6b5, PAD)
    AFFINE_FAST_PATHS (none_r5g6b5, r5g6b5, NONE)
    AFFINE_FAST_PATHS (reflect_r5g6b5, r5g6b5, REFLECT)
    AFFINE_FAST_PATHS (normal_r5g6b5, r5g6b5, NORMAL)

    /* Affine, no alpha */
    { PIXMAN_any,
      (FAST_PATH_NO_ALPHA_MAP | FAST_PATH_HAS_TRANSFORM | FAST_PATH_AFFINE_TRANSFORM),
      bits_image_fetch_affine_no_alpha,
      _pixman_image_get_scanline_generic_64
    },

    /* General */
    { PIXMAN_any, 0, bits_image_fetch_general, _pixman_image_get_scanline_generic_64 },

    { PIXMAN_null },
};

static void
bits_image_property_changed (pixman_image_t *image)
{
    uint32_t flags = image->common.flags;
    pixman_format_code_t format = image->common.extended_format_code;
    const fetcher_info_t *info;

    _pixman_bits_image_setup_accessors (&image->bits);

    info = fetcher_info;
    while (info->format != PIXMAN_null)
    {
	if ((info->format == format || info->format == PIXMAN_any)	&&
	    (info->flags & flags) == info->flags)
	{
	    image->bits.get_scanline_32 = info->fetch_32;
	    image->bits.get_scanline_64 = info->fetch_64;
	    break;
	}

	info++;
    }
}

static uint32_t *
src_get_scanline_narrow (pixman_iter_t *iter, const uint32_t *mask)
{
    iter->image->bits.get_scanline_32 (
	iter->image, iter->x, iter->y++, iter->width, iter->buffer, mask);

    return iter->buffer;
}

static uint32_t *
src_get_scanline_wide (pixman_iter_t *iter, const uint32_t *mask)
{
    iter->image->bits.get_scanline_64 (
	iter->image, iter->x, iter->y++, iter->width, iter->buffer, mask);

    return iter->buffer;
}

void
_pixman_bits_image_src_iter_init (pixman_image_t *image, pixman_iter_t *iter)
{
    if (iter->flags & ITER_NARROW)
	iter->get_scanline = src_get_scanline_narrow;
    else
	iter->get_scanline = src_get_scanline_wide;
}

static uint32_t *
dest_get_scanline_narrow (pixman_iter_t *iter, const uint32_t *mask)
{
    pixman_image_t *image  = iter->image;
    int             x      = iter->x;
    int             y      = iter->y;
    int             width  = iter->width;
    uint32_t *	    buffer = iter->buffer;

    image->bits.fetch_scanline_32 (image, x, y, width, buffer, mask);
    if (image->common.alpha_map)
    {
	uint32_t *alpha;

	if ((alpha = malloc (width * sizeof (uint32_t))))
	{
	    int i;

	    x -= image->common.alpha_origin_x;
	    y -= image->common.alpha_origin_y;

	    image->common.alpha_map->fetch_scanline_32 (
		(pixman_image_t *)image->common.alpha_map,
		x, y, width, alpha, mask);

	    for (i = 0; i < width; ++i)
	    {
		buffer[i] &= ~0xff000000;
		buffer[i] |= (alpha[i] & 0xff000000);
	    }

	    free (alpha);
	}
    }

    return iter->buffer;
}

static uint32_t *
dest_get_scanline_wide (pixman_iter_t *iter, const uint32_t *mask)
{
    bits_image_t *  image  = &iter->image->bits;
    int             x      = iter->x;
    int             y      = iter->y;
    int             width  = iter->width;
    uint64_t *	    buffer = (uint64_t *)iter->buffer;

    image->fetch_scanline_64 (
	(pixman_image_t *)image, x, y, width, (uint32_t *)buffer, mask);
    if (image->common.alpha_map)
    {
	uint64_t *alpha;

	if ((alpha = malloc (width * sizeof (uint64_t))))
	{
	    int i;

	    x -= image->common.alpha_origin_x;
	    y -= image->common.alpha_origin_y;

	    image->common.alpha_map->fetch_scanline_64 (
		(pixman_image_t *)image->common.alpha_map,
		x, y, width, (uint32_t *)alpha, mask);

	    for (i = 0; i < width; ++i)
	    {
		buffer[i] &= ~0xffff000000000000ULL;
		buffer[i] |= (alpha[i] & 0xffff000000000000ULL);
	    }

	    free (alpha);
	}
    }

    return iter->buffer;
}

static void
dest_write_back_narrow (pixman_iter_t *iter)
{
    bits_image_t *  image  = &iter->image->bits;
    int             x      = iter->x;
    int             y      = iter->y;
    int             width  = iter->width;
    const uint32_t *buffer = iter->buffer;

    image->store_scanline_32 (image, x, y, width, buffer);

    if (image->common.alpha_map)
    {
	x -= image->common.alpha_origin_x;
	y -= image->common.alpha_origin_y;

	image->common.alpha_map->store_scanline_32 (
	    image->common.alpha_map, x, y, width, buffer);
    }

    iter->y++;
}

static void
dest_write_back_wide (pixman_iter_t *iter)
{
    bits_image_t *  image  = &iter->image->bits;
    int             x      = iter->x;
    int             y      = iter->y;
    int             width  = iter->width;
    const uint32_t *buffer = iter->buffer;

    image->store_scanline_64 (image, x, y, width, buffer);

    if (image->common.alpha_map)
    {
	x -= image->common.alpha_origin_x;
	y -= image->common.alpha_origin_y;

	image->common.alpha_map->store_scanline_64 (
	    image->common.alpha_map, x, y, width, buffer);
    }

    iter->y++;
}

void
_pixman_bits_image_dest_iter_init (pixman_image_t *image, pixman_iter_t *iter)
{
    if (iter->flags & ITER_NARROW)
    {
	if ((iter->flags & (ITER_IGNORE_RGB | ITER_IGNORE_ALPHA)) ==
	    (ITER_IGNORE_RGB | ITER_IGNORE_ALPHA))
	{
	    iter->get_scanline = _pixman_iter_get_scanline_noop;
	}
	else
	{
	    iter->get_scanline = dest_get_scanline_narrow;
	}
	
	iter->write_back = dest_write_back_narrow;
    }
    else
    {
	iter->get_scanline = dest_get_scanline_wide;
	iter->write_back = dest_write_back_wide;
    }
}

static uint32_t *
create_bits (pixman_format_code_t format,
             int                  width,
             int                  height,
             int *		  rowstride_bytes)
{
    int stride;
    size_t buf_size;
    int bpp;

    /* what follows is a long-winded way, avoiding any possibility of integer
     * overflows, of saying:
     * stride = ((width * bpp + 0x1f) >> 5) * sizeof (uint32_t);
     */

    bpp = PIXMAN_FORMAT_BPP (format);
    if (_pixman_multiply_overflows_int (width, bpp))
	return NULL;

    stride = width * bpp;
    if (_pixman_addition_overflows_int (stride, 0x1f))
	return NULL;

    stride += 0x1f;
    stride >>= 5;

    stride *= sizeof (uint32_t);

    if (_pixman_multiply_overflows_size (height, stride))
	return NULL;

    buf_size = height * stride;

    if (rowstride_bytes)
	*rowstride_bytes = stride;

    return calloc (buf_size, 1);
}

pixman_bool_t
_pixman_bits_image_init (pixman_image_t *     image,
                         pixman_format_code_t format,
                         int                  width,
                         int                  height,
                         uint32_t *           bits,
                         int                  rowstride)
{
    uint32_t *free_me = NULL;

    if (!bits && width && height)
    {
	int rowstride_bytes;

	free_me = bits = create_bits (format, width, height, &rowstride_bytes);

	if (!bits)
	    return FALSE;

	rowstride = rowstride_bytes / (int) sizeof (uint32_t);
    }

    _pixman_image_init (image);

    image->type = BITS;
    image->bits.format = format;
    image->bits.width = width;
    image->bits.height = height;
    image->bits.bits = bits;
    image->bits.free_me = free_me;
    image->bits.read_func = NULL;
    image->bits.write_func = NULL;
    image->bits.rowstride = rowstride;
    image->bits.indexed = NULL;

    image->common.property_changed = bits_image_property_changed;

    _pixman_image_reset_clip_region (image);

    return TRUE;
}

PIXMAN_EXPORT pixman_image_t *
pixman_image_create_bits (pixman_format_code_t format,
                          int                  width,
                          int                  height,
                          uint32_t *           bits,
                          int                  rowstride_bytes)
{
    pixman_image_t *image;

    /* must be a whole number of uint32_t's
     */
    return_val_if_fail (
	bits == NULL || (rowstride_bytes % sizeof (uint32_t)) == 0, NULL);

    return_val_if_fail (PIXMAN_FORMAT_BPP (format) >= PIXMAN_FORMAT_DEPTH (format), NULL);

    image = _pixman_image_allocate ();

    if (!image)
	return NULL;

    if (!_pixman_bits_image_init (image, format, width, height, bits,
				  rowstride_bytes / (int) sizeof (uint32_t)))
    {
	free (image);
	return NULL;
    }

    return image;
}
