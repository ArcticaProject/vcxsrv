/*
 * Copyright © 2009 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
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
#include "pixman-private.h"

static void
delegate_composite (pixman_implementation_t * imp,
                    pixman_op_t               op,
                    pixman_image_t *          src,
                    pixman_image_t *          mask,
                    pixman_image_t *          dest,
                    int32_t                   src_x,
                    int32_t                   src_y,
                    int32_t                   mask_x,
                    int32_t                   mask_y,
                    int32_t                   dest_x,
                    int32_t                   dest_y,
                    int32_t                   width,
                    int32_t                   height)
{
    _pixman_implementation_composite (imp->delegate,
                                      op,
                                      src, mask, dest,
                                      src_x, src_y,
                                      mask_x, mask_y,
                                      dest_x, dest_y,
                                      width, height);
}

static void
delegate_combine_32 (pixman_implementation_t * imp,
                     pixman_op_t               op,
                     uint32_t *                dest,
                     const uint32_t *          src,
                     const uint32_t *          mask,
                     int                       width)
{
    _pixman_implementation_combine_32 (imp->delegate,
                                       op, dest, src, mask, width);
}

static void
delegate_combine_64 (pixman_implementation_t * imp,
                     pixman_op_t               op,
                     uint64_t *                dest,
                     const uint64_t *          src,
                     const uint64_t *          mask,
                     int                       width)
{
    _pixman_implementation_combine_64 (imp->delegate,
                                       op, dest, src, mask, width);
}

static void
delegate_combine_32_ca (pixman_implementation_t * imp,
                        pixman_op_t               op,
                        uint32_t *                dest,
                        const uint32_t *          src,
                        const uint32_t *          mask,
                        int                       width)
{
    _pixman_implementation_combine_32_ca (imp->delegate,
                                          op, dest, src, mask, width);
}

static void
delegate_combine_64_ca (pixman_implementation_t * imp,
                        pixman_op_t               op,
                        uint64_t *                dest,
                        const uint64_t *          src,
                        const uint64_t *          mask,
                        int                       width)
{
    _pixman_implementation_combine_64_ca (imp->delegate,
                                          op, dest, src, mask, width);
}

static pixman_bool_t
delegate_blt (pixman_implementation_t * imp,
              uint32_t *                src_bits,
              uint32_t *                dst_bits,
              int                       src_stride,
              int                       dst_stride,
              int                       src_bpp,
              int                       dst_bpp,
              int                       src_x,
              int                       src_y,
              int                       dst_x,
              int                       dst_y,
              int                       width,
              int                       height)
{
    return _pixman_implementation_blt (
	imp->delegate, src_bits, dst_bits, src_stride, dst_stride,
	src_bpp, dst_bpp, src_x, src_y, dst_x, dst_y,
	width, height);
}

static pixman_bool_t
delegate_fill (pixman_implementation_t *imp,
               uint32_t *               bits,
               int                      stride,
               int                      bpp,
               int                      x,
               int                      y,
               int                      width,
               int                      height,
               uint32_t                 xor)
{
    return _pixman_implementation_fill (
	imp->delegate, bits, stride, bpp, x, y, width, height, xor);
}

pixman_implementation_t *
_pixman_implementation_create (pixman_implementation_t *delegate)
{
    pixman_implementation_t *imp = malloc (sizeof (pixman_implementation_t));
    pixman_implementation_t *d;
    int i;

    if (!imp)
	return NULL;

    /* Make sure the whole delegate chain has the right toplevel */
    imp->delegate = delegate;
    for (d = imp; d != NULL; d = d->delegate)
	d->toplevel = imp;

    /* Fill out function pointers with ones that just delegate
     */
    imp->composite = delegate_composite;
    imp->blt = delegate_blt;
    imp->fill = delegate_fill;

    for (i = 0; i < PIXMAN_N_OPERATORS; ++i)
    {
	imp->combine_32[i] = delegate_combine_32;
	imp->combine_64[i] = delegate_combine_64;
	imp->combine_32_ca[i] = delegate_combine_32_ca;
	imp->combine_64_ca[i] = delegate_combine_64_ca;
    }

    return imp;
}

void
_pixman_implementation_combine_32 (pixman_implementation_t * imp,
                                   pixman_op_t               op,
                                   uint32_t *                dest,
                                   const uint32_t *          src,
                                   const uint32_t *          mask,
                                   int                       width)
{
    (*imp->combine_32[op]) (imp, op, dest, src, mask, width);
}

void
_pixman_implementation_combine_64 (pixman_implementation_t * imp,
                                   pixman_op_t               op,
                                   uint64_t *                dest,
                                   const uint64_t *          src,
                                   const uint64_t *          mask,
                                   int                       width)
{
    (*imp->combine_64[op]) (imp, op, dest, src, mask, width);
}

void
_pixman_implementation_combine_32_ca (pixman_implementation_t * imp,
                                      pixman_op_t               op,
                                      uint32_t *                dest,
                                      const uint32_t *          src,
                                      const uint32_t *          mask,
                                      int                       width)
{
    (*imp->combine_32_ca[op]) (imp, op, dest, src, mask, width);
}

void
_pixman_implementation_combine_64_ca (pixman_implementation_t * imp,
                                      pixman_op_t               op,
                                      uint64_t *                dest,
                                      const uint64_t *          src,
                                      const uint64_t *          mask,
                                      int                       width)
{
    (*imp->combine_64_ca[op]) (imp, op, dest, src, mask, width);
}

void
_pixman_implementation_composite (pixman_implementation_t * imp,
                                  pixman_op_t               op,
                                  pixman_image_t *          src,
                                  pixman_image_t *          mask,
                                  pixman_image_t *          dest,
                                  int32_t                   src_x,
                                  int32_t                   src_y,
                                  int32_t                   mask_x,
                                  int32_t                   mask_y,
                                  int32_t                   dest_x,
                                  int32_t                   dest_y,
                                  int32_t                   width,
                                  int32_t                   height)
{
    (*imp->composite) (imp, op,
		       src, mask, dest,
		       src_x, src_y, mask_x, mask_y, dest_x, dest_y,
		       width, height);
}

pixman_bool_t
_pixman_implementation_blt (pixman_implementation_t * imp,
                            uint32_t *                src_bits,
                            uint32_t *                dst_bits,
                            int                       src_stride,
                            int                       dst_stride,
                            int                       src_bpp,
                            int                       dst_bpp,
                            int                       src_x,
                            int                       src_y,
                            int                       dst_x,
                            int                       dst_y,
                            int                       width,
                            int                       height)
{
    return (*imp->blt) (imp, src_bits, dst_bits, src_stride, dst_stride,
			src_bpp, dst_bpp, src_x, src_y, dst_x, dst_y,
			width, height);
}

pixman_bool_t
_pixman_implementation_fill (pixman_implementation_t *imp,
                             uint32_t *               bits,
                             int                      stride,
                             int                      bpp,
                             int                      x,
                             int                      y,
                             int                      width,
                             int                      height,
                             uint32_t                 xor)
{
    return (*imp->fill) (imp, bits, stride, bpp, x, y, width, height, xor);
}

