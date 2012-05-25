/*
 * Copyright (c) 2012
 *      MIPS Technologies, Inc., California.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the MIPS Technologies, Inc., nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE MIPS TECHNOLOGIES, INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE MIPS TECHNOLOGIES, INC. BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Author:  Nemanja Lukic (nlukic@mips.com)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pixman-private.h"
#include "pixman-mips-dspr2.h"

PIXMAN_MIPS_BIND_FAST_PATH_SRC_DST (0, src_x888_8888,
                                    uint32_t, 1, uint32_t, 1)
PIXMAN_MIPS_BIND_FAST_PATH_SRC_DST (0, src_8888_0565,
                                    uint32_t, 1, uint16_t, 1)
PIXMAN_MIPS_BIND_FAST_PATH_SRC_DST (0, src_0565_8888,
                                    uint16_t, 1, uint32_t, 1)
PIXMAN_MIPS_BIND_FAST_PATH_SRC_DST (DO_FAST_MEMCPY, src_0565_0565,
                                    uint16_t, 1, uint16_t, 1)
PIXMAN_MIPS_BIND_FAST_PATH_SRC_DST (DO_FAST_MEMCPY, src_8888_8888,
                                    uint32_t, 1, uint32_t, 1)
PIXMAN_MIPS_BIND_FAST_PATH_SRC_DST (DO_FAST_MEMCPY, src_0888_0888,
                                    uint8_t, 3, uint8_t, 3)

PIXMAN_MIPS_BIND_FAST_PATH_N_MASK_DST (SKIP_ZERO_SRC, over_n_8888_8888_ca,
                                       uint32_t, 1, uint32_t, 1)
PIXMAN_MIPS_BIND_FAST_PATH_N_MASK_DST (SKIP_ZERO_SRC, over_n_8888_0565_ca,
                                       uint32_t, 1, uint16_t, 1)
PIXMAN_MIPS_BIND_FAST_PATH_N_MASK_DST (SKIP_ZERO_SRC, over_n_8_8888,
                                       uint8_t, 1, uint32_t, 1)
PIXMAN_MIPS_BIND_FAST_PATH_N_MASK_DST (SKIP_ZERO_SRC, over_n_8_0565,
                                       uint8_t, 1, uint16_t, 1)

PIXMAN_MIPS_BIND_SCALED_BILINEAR_SRC_A8_DST (SKIP_ZERO_SRC, 8888_8_8888, OVER,
                                             uint32_t, uint32_t)

static pixman_bool_t
pixman_fill_mips (uint32_t *bits,
                  int       stride,
                  int       bpp,
                  int       x,
                  int       y,
                  int       width,
                  int       height,
                  uint32_t  _xor)
{
    uint8_t *byte_line;
    uint32_t byte_width;
    switch (bpp)
    {
    case 16:
        stride = stride * (int) sizeof (uint32_t) / 2;
        byte_line = (uint8_t *)(((uint16_t *)bits) + stride * y + x);
        byte_width = width * 2;
        stride *= 2;

        while (height--)
        {
            uint8_t *dst = byte_line;
            byte_line += stride;
            pixman_fill_buff16_mips (dst, byte_width, _xor & 0xffff);
        }
        return TRUE;
    case 32:
        stride = stride * (int) sizeof (uint32_t) / 4;
        byte_line = (uint8_t *)(((uint32_t *)bits) + stride * y + x);
        byte_width = width * 4;
        stride *= 4;

        while (height--)
        {
            uint8_t *dst = byte_line;
            byte_line += stride;
            pixman_fill_buff32_mips (dst, byte_width, _xor);
        }
        return TRUE;
    default:
        return FALSE;
    }
}

static pixman_bool_t
pixman_blt_mips (uint32_t *src_bits,
                 uint32_t *dst_bits,
                 int       src_stride,
                 int       dst_stride,
                 int       src_bpp,
                 int       dst_bpp,
                 int       src_x,
                 int       src_y,
                 int       dest_x,
                 int       dest_y,
                 int       width,
                 int       height)
{
    if (src_bpp != dst_bpp)
        return FALSE;

    uint8_t *src_bytes;
    uint8_t *dst_bytes;
    uint32_t byte_width;

    switch (src_bpp)
    {
    case 16:
        src_stride = src_stride * (int) sizeof (uint32_t) / 2;
        dst_stride = dst_stride * (int) sizeof (uint32_t) / 2;
        src_bytes =(uint8_t *)(((uint16_t *)src_bits)
                                          + src_stride * (src_y) + (src_x));
        dst_bytes = (uint8_t *)(((uint16_t *)dst_bits)
                                           + dst_stride * (dest_y) + (dest_x));
        byte_width = width * 2;
        src_stride *= 2;
        dst_stride *= 2;

        while (height--)
        {
            uint8_t *src = src_bytes;
            uint8_t *dst = dst_bytes;
            src_bytes += src_stride;
            dst_bytes += dst_stride;
            pixman_mips_fast_memcpy (dst, src, byte_width);
        }
        return TRUE;
    case 32:
        src_stride = src_stride * (int) sizeof (uint32_t) / 4;
        dst_stride = dst_stride * (int) sizeof (uint32_t) / 4;
        src_bytes = (uint8_t *)(((uint32_t *)src_bits)
                                           + src_stride * (src_y) + (src_x));
        dst_bytes = (uint8_t *)(((uint32_t *)dst_bits)
                                           + dst_stride * (dest_y) + (dest_x));
        byte_width = width * 4;
        src_stride *= 4;
        dst_stride *= 4;

        while (height--)
        {
            uint8_t *src = src_bytes;
            uint8_t *dst = dst_bytes;
            src_bytes += src_stride;
            dst_bytes += dst_stride;
            pixman_mips_fast_memcpy (dst, src, byte_width);
        }
        return TRUE;
    default:
        return FALSE;
    }
}

static const pixman_fast_path_t mips_dspr2_fast_paths[] =
{
    PIXMAN_STD_FAST_PATH (SRC, r5g6b5,   null, r5g6b5,   mips_composite_src_0565_0565),
    PIXMAN_STD_FAST_PATH (SRC, b5g6r5,   null, b5g6r5,   mips_composite_src_0565_0565),
    PIXMAN_STD_FAST_PATH (SRC, a8r8g8b8, null, r5g6b5,   mips_composite_src_8888_0565),
    PIXMAN_STD_FAST_PATH (SRC, x8r8g8b8, null, r5g6b5,   mips_composite_src_8888_0565),
    PIXMAN_STD_FAST_PATH (SRC, a8b8g8r8, null, b5g6r5,   mips_composite_src_8888_0565),
    PIXMAN_STD_FAST_PATH (SRC, x8b8g8r8, null, b5g6r5,   mips_composite_src_8888_0565),
    PIXMAN_STD_FAST_PATH (SRC, r5g6b5,   null, a8r8g8b8, mips_composite_src_0565_8888),
    PIXMAN_STD_FAST_PATH (SRC, r5g6b5,   null, x8r8g8b8, mips_composite_src_0565_8888),
    PIXMAN_STD_FAST_PATH (SRC, b5g6r5,   null, a8b8g8r8, mips_composite_src_0565_8888),
    PIXMAN_STD_FAST_PATH (SRC, b5g6r5,   null, x8b8g8r8, mips_composite_src_0565_8888),
    PIXMAN_STD_FAST_PATH (SRC, a8r8g8b8, null, x8r8g8b8, mips_composite_src_8888_8888),
    PIXMAN_STD_FAST_PATH (SRC, x8r8g8b8, null, x8r8g8b8, mips_composite_src_8888_8888),
    PIXMAN_STD_FAST_PATH (SRC, a8b8g8r8, null, x8b8g8r8, mips_composite_src_8888_8888),
    PIXMAN_STD_FAST_PATH (SRC, x8b8g8r8, null, x8b8g8r8, mips_composite_src_8888_8888),
    PIXMAN_STD_FAST_PATH (SRC, a8r8g8b8, null, a8r8g8b8, mips_composite_src_8888_8888),
    PIXMAN_STD_FAST_PATH (SRC, a8b8g8r8, null, a8b8g8r8, mips_composite_src_8888_8888),
    PIXMAN_STD_FAST_PATH (SRC, x8r8g8b8, null, a8r8g8b8, mips_composite_src_x888_8888),
    PIXMAN_STD_FAST_PATH (SRC, x8b8g8r8, null, a8b8g8r8, mips_composite_src_x888_8888),
    PIXMAN_STD_FAST_PATH (SRC, r8g8b8,   null, r8g8b8,   mips_composite_src_0888_0888),

    PIXMAN_STD_FAST_PATH_CA (OVER, solid, a8r8g8b8, a8r8g8b8, mips_composite_over_n_8888_8888_ca),
    PIXMAN_STD_FAST_PATH_CA (OVER, solid, a8r8g8b8, x8r8g8b8, mips_composite_over_n_8888_8888_ca),
    PIXMAN_STD_FAST_PATH_CA (OVER, solid, a8b8g8r8, a8b8g8r8, mips_composite_over_n_8888_8888_ca),
    PIXMAN_STD_FAST_PATH_CA (OVER, solid, a8b8g8r8, x8b8g8r8, mips_composite_over_n_8888_8888_ca),
    PIXMAN_STD_FAST_PATH_CA (OVER, solid, a8r8g8b8, r5g6b5,   mips_composite_over_n_8888_0565_ca),
    PIXMAN_STD_FAST_PATH_CA (OVER, solid, a8b8g8r8, b5g6r5,   mips_composite_over_n_8888_0565_ca),
    PIXMAN_STD_FAST_PATH (OVER, solid,    a8,       a8r8g8b8, mips_composite_over_n_8_8888),
    PIXMAN_STD_FAST_PATH (OVER, solid,    a8,       x8r8g8b8, mips_composite_over_n_8_8888),
    PIXMAN_STD_FAST_PATH (OVER, solid,    a8,       a8b8g8r8, mips_composite_over_n_8_8888),
    PIXMAN_STD_FAST_PATH (OVER, solid,    a8,       x8b8g8r8, mips_composite_over_n_8_8888),
    PIXMAN_STD_FAST_PATH (OVER, solid,    a8,       r5g6b5,   mips_composite_over_n_8_0565),
    PIXMAN_STD_FAST_PATH (OVER, solid,    a8,       b5g6r5,   mips_composite_over_n_8_0565),

    SIMPLE_BILINEAR_A8_MASK_FAST_PATH (OVER, a8r8g8b8, a8r8g8b8, mips_8888_8_8888),
    SIMPLE_BILINEAR_A8_MASK_FAST_PATH (OVER, a8r8g8b8, x8r8g8b8, mips_8888_8_8888),

    { PIXMAN_OP_NONE },
};

static pixman_bool_t
mips_dspr2_blt (pixman_implementation_t *imp,
                uint32_t *               src_bits,
                uint32_t *               dst_bits,
                int                      src_stride,
                int                      dst_stride,
                int                      src_bpp,
                int                      dst_bpp,
                int                      src_x,
                int                      src_y,
                int                      dest_x,
                int                      dest_y,
                int                      width,
                int                      height)
{
    if (!pixman_blt_mips (
            src_bits, dst_bits, src_stride, dst_stride, src_bpp, dst_bpp,
            src_x, src_y, dest_x, dest_y, width, height))

    {
        return _pixman_implementation_blt (
            imp->delegate,
            src_bits, dst_bits, src_stride, dst_stride, src_bpp, dst_bpp,
            src_x, src_y, dest_x, dest_y, width, height);
    }

    return TRUE;
}

static pixman_bool_t
mips_dspr2_fill (pixman_implementation_t *imp,
                 uint32_t *               bits,
                 int                      stride,
                 int                      bpp,
                 int                      x,
                 int                      y,
                 int                      width,
                 int                      height,
                 uint32_t xor)
{
    if (pixman_fill_mips (bits, stride, bpp, x, y, width, height, xor))
        return TRUE;

    return _pixman_implementation_fill (
        imp->delegate, bits, stride, bpp, x, y, width, height, xor);
}

pixman_implementation_t *
_pixman_implementation_create_mips_dspr2 (pixman_implementation_t *fallback)
{
    pixman_implementation_t *imp =
        _pixman_implementation_create (fallback, mips_dspr2_fast_paths);

    imp->blt = mips_dspr2_blt;
    imp->fill = mips_dspr2_fill;

    return imp;
}
