/*
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *             2005 Lars Knoll & Zack Rusin, Trolltech
 *             2008 Aaron Plattner, NVIDIA Corporation
 * Copyright © 2000 SuSE, Inc.
 * Copyright © 2007, 2009 Red Hat, Inc.
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
#include <stdlib.h>
#include <string.h>
#include "pixman-private.h"
#include "pixman-combine32.h"

/* Store functions */

static void
bits_image_store_scanline_32 (bits_image_t *  image,
                              int             x,
                              int             y,
                              int             width,
                              const uint32_t *buffer)
{
    image->store_scanline_raw_32 (image, x, y, width, buffer);

    if (image->common.alpha_map)
    {
	x -= image->common.alpha_origin_x;
	y -= image->common.alpha_origin_y;

	bits_image_store_scanline_32 (image->common.alpha_map, x, y, width, buffer);
    }
}

static void
bits_image_store_scanline_64 (bits_image_t *  image,
                              int             x,
                              int             y,
                              int             width,
                              const uint32_t *buffer)
{
    image->store_scanline_raw_64 (image, x, y, width, buffer);

    if (image->common.alpha_map)
    {
	x -= image->common.alpha_origin_x;
	y -= image->common.alpha_origin_y;

	bits_image_store_scanline_64 (image->common.alpha_map, x, y, width, buffer);
    }
}

void
_pixman_image_store_scanline_32 (bits_image_t *  image,
                                 int             x,
                                 int             y,
                                 int             width,
                                 const uint32_t *buffer)
{
    image->store_scanline_32 (image, x, y, width, buffer);
}

void
_pixman_image_store_scanline_64 (bits_image_t *  image,
                                 int             x,
                                 int             y,
                                 int             width,
                                 const uint32_t *buffer)
{
    image->store_scanline_64 (image, x, y, width, buffer);
}

/* Fetch functions */

/* On entry, @buffer should contain @n_pixels (x, y) coordinate pairs, where
 * x and y are both uint32_ts. On exit, buffer will contain the corresponding
 * pixels.
 *
 * The coordinates must be within the sample grid. If either x or y is 0xffffffff,
 * the pixel returned will be 0.
 */
static void
bits_image_fetch_raw_pixels (bits_image_t *image,
                             uint32_t *    buffer,
                             int           n_pixels)
{
    image->fetch_pixels_raw_32 (image, buffer, n_pixels);
}

static void
bits_image_fetch_alpha_pixels (bits_image_t *image,
                               uint32_t *    buffer,
                               int           n_pixels)
{
#define N_ALPHA_PIXELS 256

    uint32_t alpha_pixels[N_ALPHA_PIXELS * 2];
    int i;

    if (!image->common.alpha_map)
    {
	bits_image_fetch_raw_pixels (image, buffer, n_pixels);
	return;
    }

    /* Alpha map */
    i = 0;
    while (i < n_pixels)
    {
	int tmp_n_pixels = MIN (N_ALPHA_PIXELS, n_pixels - i);
	int j;
	int32_t *coords;

	memcpy (alpha_pixels, buffer + 2 * i, tmp_n_pixels * 2 * sizeof (int32_t));
	coords = (int32_t *)alpha_pixels;
	for (j = 0; j < tmp_n_pixels; ++j)
	{
	    int32_t x = coords[0];
	    int32_t y = coords[1];

	    if (x != 0xffffffff)
	    {
		x -= image->common.alpha_origin_x;

		if (x < 0 || x >= image->common.alpha_map->width)
		    x = 0xffffffff;
	    }

	    if (y != 0xffffffff)
	    {
		y -= image->common.alpha_origin_y;

		if (y < 0 || y >= image->common.alpha_map->height)
		    y = 0xffffffff;
	    }

	    coords[0] = x;
	    coords[1] = y;

	    coords += 2;
	}

	bits_image_fetch_raw_pixels (image->common.alpha_map, alpha_pixels,
	                             tmp_n_pixels);
	bits_image_fetch_raw_pixels (image, buffer + 2 * i, tmp_n_pixels);

	for (j = 0; j < tmp_n_pixels; ++j)
	{
	    int a = alpha_pixels[j] >> 24;
	    uint32_t p = buffer[2 * i - j] | 0xff000000;

	    UN8x4_MUL_UN8 (p, a);

	    buffer[i++] = p;
	}
    }
}

static void
bits_image_fetch_pixels_src_clip (bits_image_t *image,
                                  uint32_t *    buffer,
                                  int           n_pixels)
{
    bits_image_fetch_alpha_pixels (image, buffer, n_pixels);
}

static force_inline void
repeat (pixman_repeat_t repeat,
        int             width,
        int             height,
        int *           x,
        int *           y)
{
    switch (repeat)
    {
    case PIXMAN_REPEAT_NORMAL:
	*x = MOD (*x, width);
	*y = MOD (*y, height);
	break;

    case PIXMAN_REPEAT_PAD:
	*x = CLIP (*x, 0, width - 1);
	*y = CLIP (*y, 0, height - 1);
	break;

    case PIXMAN_REPEAT_REFLECT:
	*x = MOD (*x, width * 2);
	*y = MOD (*y, height * 2);

	if (*x >= width)
	    *x = width * 2 - *x - 1;

	if (*y >= height)
	    *y = height * 2 - *y - 1;
	break;

    case PIXMAN_REPEAT_NONE:
	if (*x < 0 || *x >= width)
	    *x = 0xffffffff;

	if (*y < 0 || *y >= height)
	    *y = 0xffffffff;
	break;
    }
}

/* Buffer contains list of fixed-point coordinates on input,
 * a list of pixels on output
 */
static void
bits_image_fetch_nearest_pixels (bits_image_t *image,
                                 uint32_t *    buffer,
                                 int           n_pixels)
{
    pixman_repeat_t repeat_mode = image->common.repeat;
    int width = image->width;
    int height = image->height;
    int i;

    for (i = 0; i < 2 * n_pixels; i += 2)
    {
	int32_t *coords = (int32_t *)buffer;
	int32_t x, y;

	/* Subtract pixman_fixed_e to ensure that 0.5 rounds to 0, not 1 */
	x = pixman_fixed_to_int (coords[i] - pixman_fixed_e);
	y = pixman_fixed_to_int (coords[i + 1] - pixman_fixed_e);

	repeat (repeat_mode, width, height, &x, &y);

	coords[i] = x;
	coords[i + 1] = y;
    }

    bits_image_fetch_pixels_src_clip (image, buffer, n_pixels);
}

#define N_TMP_PIXELS    (256)

/* Buffer contains list of fixed-point coordinates on input,
 * a list of pixels on output
 */
static void
bits_image_fetch_bilinear_pixels (bits_image_t *image,
                                  uint32_t *    buffer,
                                  int           n_pixels)
{
/* (Four pixels * two coordinates) per pixel */
#define N_TEMPS         (N_TMP_PIXELS * 8)
#define N_DISTS         (N_TMP_PIXELS * 2)

    uint32_t temps[N_TEMPS];
    int32_t dists[N_DISTS];
    pixman_repeat_t repeat_mode = image->common.repeat;
    int width = image->width;
    int height = image->height;
    int32_t *coords;
    int i;

    i = 0;
    coords = (int32_t *)buffer;
    while (i < n_pixels)
    {
	int tmp_n_pixels = MIN (N_TMP_PIXELS, n_pixels - i);
	int32_t distx, disty;
	uint32_t *u;
	int32_t *t, *d;
	int j;

	t = (int32_t *)temps;
	d = dists;
	for (j = 0; j < tmp_n_pixels; ++j)
	{
	    int32_t x1, y1, x2, y2;

	    x1 = coords[0] - pixman_fixed_1 / 2;
	    y1 = coords[1] - pixman_fixed_1 / 2;

	    distx = (x1 >> 8) & 0xff;
	    disty = (y1 >> 8) & 0xff;

	    x1 >>= 16;
	    y1 >>= 16;
	    x2 = x1 + 1;
	    y2 = y1 + 1;

	    repeat (repeat_mode, width, height, &x1, &y1);
	    repeat (repeat_mode, width, height, &x2, &y2);

	    *t++ = x1;
	    *t++ = y1;
	    *t++ = x2;
	    *t++ = y1;
	    *t++ = x1;
	    *t++ = y2;
	    *t++ = x2;
	    *t++ = y2;

	    *d++ = distx;
	    *d++ = disty;

	    coords += 2;
	}

	bits_image_fetch_pixels_src_clip (image, temps, tmp_n_pixels * 4);

	u = (uint32_t *)temps;
	d = dists;
	for (j = 0; j < tmp_n_pixels; ++j)
	{
	    uint32_t tl, tr, bl, br, r;
	    int32_t idistx, idisty;
	    uint32_t ft, fb;

	    tl = *u++;
	    tr = *u++;
	    bl = *u++;
	    br = *u++;

	    distx = *d++;
	    disty = *d++;

	    idistx = 256 - distx;
	    idisty = 256 - disty;

#define GET8(v, i)   ((uint16_t) (uint8_t) ((v) >> i))

	    ft = GET8 (tl, 0) * idistx + GET8 (tr, 0) * distx;
	    fb = GET8 (bl, 0) * idistx + GET8 (br, 0) * distx;
	    r = (((ft * idisty + fb * disty) >> 16) & 0xff);
	    ft = GET8 (tl, 8) * idistx + GET8 (tr, 8) * distx;
	    fb = GET8 (bl, 8) * idistx + GET8 (br, 8) * distx;
	    r |= (((ft * idisty + fb * disty) >> 8) & 0xff00);
	    ft = GET8 (tl, 16) * idistx + GET8 (tr, 16) * distx;
	    fb = GET8 (bl, 16) * idistx + GET8 (br, 16) * distx;
	    r |= (((ft * idisty + fb * disty)) & 0xff0000);
	    ft = GET8 (tl, 24) * idistx + GET8 (tr, 24) * distx;
	    fb = GET8 (bl, 24) * idistx + GET8 (br, 24) * distx;
	    r |= (((ft * idisty + fb * disty) << 8) & 0xff000000);

	    buffer[i++] = r;
	}
    }
}

/* Buffer contains list of fixed-point coordinates on input,
 * a list of pixels on output
 */
static void
bits_image_fetch_convolution_pixels (bits_image_t *image,
                                     uint32_t *    buffer,
                                     int           n_pixels)
{
    uint32_t tmp_pixels_stack[N_TMP_PIXELS * 2]; /* Two coordinates per pixel */
    uint32_t *tmp_pixels = tmp_pixels_stack;
    pixman_fixed_t *params = image->common.filter_params;
    int x_off = (params[0] - pixman_fixed_1) >> 1;
    int y_off = (params[1] - pixman_fixed_1) >> 1;
    int n_tmp_pixels;
    int32_t *coords;
    int32_t *t;
    uint32_t *u;
    int i;
    int max_n_kernels;

    int32_t cwidth = pixman_fixed_to_int (params[0]);
    int32_t cheight = pixman_fixed_to_int (params[1]);
    int kernel_size = cwidth * cheight;

    params += 2;

    n_tmp_pixels = N_TMP_PIXELS;
    if (kernel_size > n_tmp_pixels)
    {
	/* Two coordinates per pixel */
	tmp_pixels = malloc (kernel_size * 2 * sizeof (uint32_t));
	n_tmp_pixels = kernel_size;

	if (!tmp_pixels)
	{
	    /* We ignore out-of-memory during rendering */
	    return;
	}
    }

    max_n_kernels = n_tmp_pixels / kernel_size;

    i = 0;
    coords = (int32_t *)buffer;
    while (i < n_pixels)
    {
	int n_kernels = MIN (max_n_kernels, (n_pixels - i));
	pixman_repeat_t repeat_mode = image->common.repeat;
	int width = image->width;
	int height = image->height;
	int j;

	t = (int32_t *)tmp_pixels;
	for (j = 0; j < n_kernels; ++j)
	{
	    int32_t x, y, x1, x2, y1, y2;

	    /* Subtract pixman_fixed_e to ensure that 0.5 rounds to 0, not 1 */
	    x1 = pixman_fixed_to_int (coords[0] - pixman_fixed_e - x_off);
	    y1 = pixman_fixed_to_int (coords[1] - pixman_fixed_e - y_off);
	    x2 = x1 + cwidth;
	    y2 = y1 + cheight;

	    for (y = y1; y < y2; ++y)
	    {
		for (x = x1; x < x2; ++x)
		{
		    int rx = x;
		    int ry = y;

		    repeat (repeat_mode, width, height, &rx, &ry);

		    *t++ = rx;
		    *t++ = ry;
		}
	    }

	    coords += 2;
	}

	bits_image_fetch_pixels_src_clip (image, tmp_pixels, n_kernels * kernel_size);

	u = tmp_pixels;
	for (j = 0; j < n_kernels; ++j)
	{
	    int32_t srtot, sgtot, sbtot, satot;
	    pixman_fixed_t *p = params;
	    int k;

	    srtot = sgtot = sbtot = satot = 0;

	    for (k = 0; k < kernel_size; ++k)
	    {
		pixman_fixed_t f = *p++;
		if (f)
		{
		    uint32_t c = *u++;

		    srtot += RED_8 (c) * f;
		    sgtot += GREEN_8 (c) * f;
		    sbtot += BLUE_8 (c) * f;
		    satot += ALPHA_8 (c) * f;
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

	    buffer[i++] = ((satot << 24) |
	                   (srtot << 16) |
	                   (sgtot <<  8) |
	                   (sbtot       ));
	}
    }

    if (tmp_pixels != tmp_pixels_stack)
	free (tmp_pixels);
}

static void
bits_image_fetch_filtered (bits_image_t *pict,
                           uint32_t *    buffer,
                           int           n_pixels)
{
    switch (pict->common.filter)
    {
    case PIXMAN_FILTER_NEAREST:
    case PIXMAN_FILTER_FAST:
	bits_image_fetch_nearest_pixels (pict, buffer, n_pixels);
	break;

    case PIXMAN_FILTER_BILINEAR:
    case PIXMAN_FILTER_GOOD:
    case PIXMAN_FILTER_BEST:
	bits_image_fetch_bilinear_pixels (pict, buffer, n_pixels);
	break;

    case PIXMAN_FILTER_CONVOLUTION:
	bits_image_fetch_convolution_pixels (pict, buffer, n_pixels);
	break;
    }
}

static void
bits_image_fetch_transformed (pixman_image_t * pict,
                              int              x,
                              int              y,
                              int              width,
                              uint32_t *       buffer,
                              const uint32_t * mask,
                              uint32_t         mask_bits)
{
    pixman_bool_t affine = TRUE;
    uint32_t tmp_buffer[2 * N_TMP_PIXELS];
    pixman_vector_t unit;
    pixman_vector_t v;
    uint32_t *bits;
    int32_t stride;
    int32_t *coords;
    int i;

    bits = pict->bits.bits;
    stride = pict->bits.rowstride;

    /* reference point is the center of the pixel */
    v.vector[0] = pixman_int_to_fixed (x) + pixman_fixed_1 / 2;
    v.vector[1] = pixman_int_to_fixed (y) + pixman_fixed_1 / 2;
    v.vector[2] = pixman_fixed_1;

    /* when using convolution filters or PIXMAN_REPEAT_PAD one
     * might get here without a transform */
    if (pict->common.transform)
    {
	if (!pixman_transform_point_3d (pict->common.transform, &v))
	    return;

	unit.vector[0] = pict->common.transform->matrix[0][0];
	unit.vector[1] = pict->common.transform->matrix[1][0];
	unit.vector[2] = pict->common.transform->matrix[2][0];

	affine = (v.vector[2] == pixman_fixed_1 && unit.vector[2] == 0);
    }
    else
    {
	unit.vector[0] = pixman_fixed_1;
	unit.vector[1] = 0;
	unit.vector[2] = 0;
    }

    i = 0;
    while (i < width)
    {
	int n_pixels = MIN (N_TMP_PIXELS, width - i);
	int j;

	coords = (int32_t *)tmp_buffer;

	for (j = 0; j < n_pixels; ++j)
	{
	    if (affine)
	    {
		coords[0] = v.vector[0];
		coords[1] = v.vector[1];
	    }
	    else
	    {
		pixman_fixed_48_16_t div;

		div = ((pixman_fixed_48_16_t)v.vector[0] << 16) / v.vector[2];

		if ((div >> 16) > 0x7fff)
		    coords[0] = 0x7fffffff;
		else if ((div >> 16) < 0x8000)
		    coords[0] = 0x80000000;
		else
		    coords[0] = div;

		div = ((pixman_fixed_48_16_t)v.vector[1] << 16) / v.vector[2];

		if ((div >> 16) > 0x7fff)
		    coords[1] = 0x7fffffff;
		else if ((div >> 16) < 0x8000)
		    coords[1] = 0x8000000;
		else
		    coords[1] = div;

		v.vector[2] += unit.vector[2];
	    }

	    coords += 2;

	    v.vector[0] += unit.vector[0];
	    v.vector[1] += unit.vector[1];
	}

	bits_image_fetch_filtered (&pict->bits, tmp_buffer, n_pixels);

	for (j = 0; j < n_pixels; ++j)
	    buffer[i++] = tmp_buffer[j];
    }
}

static void
bits_image_fetch_solid_32 (pixman_image_t * image,
                           int              x,
                           int              y,
                           int              width,
                           uint32_t *       buffer,
                           const uint32_t * mask,
                           uint32_t         mask_bits)
{
    uint32_t color[2];
    uint32_t *end;

    color[0] = 0;
    color[1] = 0;

    image->bits.fetch_pixels_raw_32 (&image->bits, color, 1);

    end = buffer + width;
    while (buffer < end)
	*(buffer++) = color[0];
}

static void
bits_image_fetch_solid_64 (pixman_image_t * image,
                           int              x,
                           int              y,
                           int              width,
                           uint32_t *       b,
                           const uint32_t * unused,
                           uint32_t         unused2)
{
    uint64_t color;
    uint32_t *coords = (uint32_t *)&color;
    uint64_t *buffer = (uint64_t *)b;
    uint64_t *end;

    coords[0] = 0;
    coords[1] = 0;
    
    image->bits.fetch_pixels_raw_64 (&image->bits, (uint32_t *)&color, 1);
    
    end = buffer + width;
    while (buffer < end)
	*(buffer++) = color;
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
	    image->fetch_scanline_raw_64 ((pixman_image_t *)image, x, y, w, buffer, NULL, 0);
	else
	    image->fetch_scanline_raw_32 ((pixman_image_t *)image, x, y, w, buffer, NULL, 0);

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

    while (width)
    {
	while (x < 0)
	    x += image->width;
	while (x >= image->width)
	    x -= image->width;

	w = MIN (width, image->width - x);

	if (wide)
	    image->fetch_scanline_raw_64 ((pixman_image_t *)image, x, y, w, buffer, NULL, 0);
	else
	    image->fetch_scanline_raw_32 ((pixman_image_t *)image, x, y, w, buffer, NULL, 0);
	
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
                                   const uint32_t * mask,
                                   uint32_t         mask_bits)
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
                                   const uint32_t * unused,
                                   uint32_t         unused2)
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

static pixman_bool_t out_of_bounds_workaround = TRUE;

/* Old X servers rely on out-of-bounds accesses when they are asked
 * to composite with a window as the source. They create a pixman image
 * pointing to some bogus position in memory, but then they set a clip
 * region to the position where the actual bits are.
 *
 * Due to a bug in old versions of pixman, where it would not clip
 * against the image bounds when a clip region was set, this would
 * actually work. So by default we allow certain out-of-bound access
 * to happen unless explicitly disabled.
 *
 * Fixed X servers should call this function to disable the workaround.
 */
PIXMAN_EXPORT void
pixman_disable_out_of_bounds_workaround (void)
{
    out_of_bounds_workaround = FALSE;
}

static pixman_bool_t
source_image_needs_out_of_bounds_workaround (bits_image_t *image)
{
    if (image->common.clip_sources                      &&
        image->common.repeat == PIXMAN_REPEAT_NONE      &&
	image->common.have_clip_region			&&
        out_of_bounds_workaround)
    {
	if (!image->common.client_clip)
	{
	    /* There is no client clip, so the drawable in question
	     * is a window if the clip region extends beyond the
	     * drawable geometry.
	     */
	    const pixman_box32_t *extents = pixman_region32_extents (&image->common.clip_region);

	    if (extents->x1 >= 0 && extents->x2 < image->width &&
		extents->y1 >= 0 && extents->y2 < image->height)
	    {
		return FALSE;
	    }
	}

	return TRUE;
    }

    return FALSE;
}

static void
bits_image_property_changed (pixman_image_t *image)
{
    bits_image_t *bits = (bits_image_t *)image;

    _pixman_bits_image_setup_raw_accessors (bits);

    if (bits->common.alpha_map)
    {
	image->common.get_scanline_64 =
	    _pixman_image_get_scanline_generic_64;
	image->common.get_scanline_32 =
	    bits_image_fetch_transformed;
    }
    else if ((bits->common.repeat != PIXMAN_REPEAT_NONE) &&
             bits->width == 1 &&
             bits->height == 1)
    {
	image->common.get_scanline_64 = bits_image_fetch_solid_64;
	image->common.get_scanline_32 = bits_image_fetch_solid_32;
    }
    else if (!bits->common.transform &&
             bits->common.filter != PIXMAN_FILTER_CONVOLUTION &&
             (bits->common.repeat == PIXMAN_REPEAT_NONE ||
              bits->common.repeat == PIXMAN_REPEAT_NORMAL))
    {
	image->common.get_scanline_64 = bits_image_fetch_untransformed_64;
	image->common.get_scanline_32 = bits_image_fetch_untransformed_32;
    }
    else
    {
	image->common.get_scanline_64 =
	    _pixman_image_get_scanline_generic_64;
	image->common.get_scanline_32 =
	    bits_image_fetch_transformed;
    }

    bits->store_scanline_64 = bits_image_store_scanline_64;
    bits->store_scanline_32 = bits_image_store_scanline_32;

    bits->common.need_workaround =
        source_image_needs_out_of_bounds_workaround (bits);
}

static uint32_t *
create_bits (pixman_format_code_t format,
             int                  width,
             int                  height,
             int *                rowstride_bytes)
{
    int stride;
    int buf_size;
    int bpp;

    /* what follows is a long-winded way, avoiding any possibility of integer
     * overflows, of saying:
     * stride = ((width * bpp + 0x1f) >> 5) * sizeof (uint32_t);
     */

    bpp = PIXMAN_FORMAT_BPP (format);
    if (pixman_multiply_overflows_int (width, bpp))
	return NULL;

    stride = width * bpp;
    if (pixman_addition_overflows_int (stride, 0x1f))
	return NULL;

    stride += 0x1f;
    stride >>= 5;

    stride *= sizeof (uint32_t);

    if (pixman_multiply_overflows_int (height, stride))
	return NULL;

    buf_size = height * stride;

    if (rowstride_bytes)
	*rowstride_bytes = stride;

    return calloc (buf_size, 1);
}

PIXMAN_EXPORT pixman_image_t *
pixman_image_create_bits (pixman_format_code_t format,
                          int                  width,
                          int                  height,
                          uint32_t *           bits,
                          int                  rowstride_bytes)
{
    pixman_image_t *image;
    uint32_t *free_me = NULL;

    /* must be a whole number of uint32_t's
     */
    return_val_if_fail (bits == NULL ||
                        (rowstride_bytes % sizeof (uint32_t)) == 0, NULL);

    if (!bits && width && height)
    {
	free_me = bits = create_bits (format, width, height, &rowstride_bytes);
	if (!bits)
	    return NULL;
    }

    image = _pixman_image_allocate ();

    if (!image)
    {
	if (free_me)
	    free (free_me);
	return NULL;
    }

    image->type = BITS;
    image->bits.format = format;
    image->bits.width = width;
    image->bits.height = height;
    image->bits.bits = bits;
    image->bits.free_me = free_me;
    image->bits.read_func = NULL;
    image->bits.write_func = NULL;

    /* The rowstride is stored in number of uint32_t */
    image->bits.rowstride = rowstride_bytes / (int) sizeof (uint32_t);

    image->bits.indexed = NULL;

    image->common.property_changed = bits_image_property_changed;

    bits_image_property_changed (image);

    _pixman_image_reset_clip_region (image);

    return image;
}
