/*
 * Copyright © 2009 ARM Ltd, Movial Creative Technologies Oy
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of ARM Ltd not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  ARM Ltd makes no
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
 *
 * Author:  Ian Rickards (ian.rickards@arm.com)
 * Author:  Jonathan Morton (jonathan.morton@movial.com)
 * Author:  Markku Vire (markku.vire@movial.com)
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <arm_neon.h>
#include <string.h>
#include "pixman-private.h"

/* Deal with an intrinsic that is defined differently in GCC */
#if !defined(__ARMCC_VERSION) && !defined(__pld)
#define __pld(_x) __builtin_prefetch (_x)
#endif

static force_inline uint8x8x4_t
unpack0565 (uint16x8_t rgb)
{
    uint16x8_t gb, b;
    uint8x8x4_t res;

    res.val[3] = vdup_n_u8 (0);
    gb = vshrq_n_u16 (rgb, 5);
    b = vshrq_n_u16 (rgb, 5 + 6);

    res.val[0] = vmovn_u16 (rgb);  /* get low 5 bits */
    res.val[1] = vmovn_u16 (gb);   /* get mid 6 bits */
    res.val[2] = vmovn_u16 (b);    /* get top 5 bits */

    res.val[0] = vshl_n_u8 (res.val[0], 3); /* shift to top */
    res.val[1] = vshl_n_u8 (res.val[1], 2); /* shift to top */
    res.val[2] = vshl_n_u8 (res.val[2], 3); /* shift to top */

    res.val[0] = vsri_n_u8 (res.val[0], res.val[0], 5);
    res.val[1] = vsri_n_u8 (res.val[1], res.val[1], 6);
    res.val[2] = vsri_n_u8 (res.val[2], res.val[2], 5);

    return res;
}

#ifdef USE_GCC_INLINE_ASM
/* Some versions of gcc have problems with vshll_n_u8 intrinsic (Bug 23576) */
#define vshll_n_u8(a, n) ({ uint16x8_t r; \
    asm ("vshll.u8 %q0, %P1, %2\n" : "=w" (r) : "w" (a), "i" (n)); r; })
#endif

static force_inline uint16x8_t
pack0565 (uint8x8x4_t s)
{
    uint16x8_t rgb, val_g, val_r;

    rgb = vshll_n_u8 (s.val[2], 8);
    val_g = vshll_n_u8 (s.val[1], 8);
    val_r = vshll_n_u8 (s.val[0], 8);
    rgb = vsriq_n_u16 (rgb, val_g, 5);
    rgb = vsriq_n_u16 (rgb, val_r, 5 + 6);

    return rgb;
}

static force_inline uint8x8_t
neon2mul (uint8x8_t x,
          uint8x8_t alpha)
{
    uint16x8_t tmp, tmp2;
    uint8x8_t res;

    tmp = vmull_u8 (x, alpha);
    tmp2 = vrshrq_n_u16 (tmp, 8);
    res = vraddhn_u16 (tmp, tmp2);

    return res;
}

static force_inline uint8x8x4_t
neon8mul (uint8x8x4_t x,
          uint8x8_t   alpha)
{
    uint16x8x4_t tmp;
    uint8x8x4_t res;
    uint16x8_t qtmp1, qtmp2;

    tmp.val[0] = vmull_u8 (x.val[0], alpha);
    tmp.val[1] = vmull_u8 (x.val[1], alpha);
    tmp.val[2] = vmull_u8 (x.val[2], alpha);
    tmp.val[3] = vmull_u8 (x.val[3], alpha);

    qtmp1 = vrshrq_n_u16 (tmp.val[0], 8);
    qtmp2 = vrshrq_n_u16 (tmp.val[1], 8);
    res.val[0] = vraddhn_u16 (tmp.val[0], qtmp1);
    qtmp1 = vrshrq_n_u16 (tmp.val[2], 8);
    res.val[1] = vraddhn_u16 (tmp.val[1], qtmp2);
    qtmp2 = vrshrq_n_u16 (tmp.val[3], 8);
    res.val[2] = vraddhn_u16 (tmp.val[2], qtmp1);
    res.val[3] = vraddhn_u16 (tmp.val[3], qtmp2);

    return res;
}

static force_inline uint8x8x4_t
neon8qadd (uint8x8x4_t x,
           uint8x8x4_t y)
{
    uint8x8x4_t res;

    res.val[0] = vqadd_u8 (x.val[0], y.val[0]);
    res.val[1] = vqadd_u8 (x.val[1], y.val[1]);
    res.val[2] = vqadd_u8 (x.val[2], y.val[2]);
    res.val[3] = vqadd_u8 (x.val[3], y.val[3]);

    return res;
}

static void
neon_composite_add_8000_8000 (pixman_implementation_t * impl,
                              pixman_op_t               op,
                              pixman_image_t *          src_image,
                              pixman_image_t *          mask_image,
                              pixman_image_t *          dst_image,
                              int32_t                   src_x,
                              int32_t                   src_y,
                              int32_t                   mask_x,
                              int32_t                   mask_y,
                              int32_t                   dest_x,
                              int32_t                   dest_y,
                              int32_t                   width,
                              int32_t                   height)
{
    uint8_t     *dst_line, *dst;
    uint8_t     *src_line, *src;
    int dst_stride, src_stride;
    uint16_t w;

    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, uint8_t, src_stride, src_line, 1);
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint8_t, dst_stride, dst_line, 1);

    if (width >= 8)
    {
	/* Use overlapping 8-pixel method */
	while (height--)
	{
	    uint8_t *keep_dst = 0;
	    uint8x8_t sval, dval, temp;

	    dst = dst_line;
	    dst_line += dst_stride;
	    src = src_line;
	    src_line += src_stride;
	    w = width;

#ifndef USE_GCC_INLINE_ASM
	    sval = vld1_u8 ((void *)src);
	    dval = vld1_u8 ((void *)dst);
	    keep_dst = dst;

	    temp = vqadd_u8 (dval, sval);

	    src += (w & 7);
	    dst += (w & 7);
	    w -= (w & 7);

	    while (w)
	    {
		sval = vld1_u8 ((void *)src);
		dval = vld1_u8 ((void *)dst);

		vst1_u8 ((void *)keep_dst, temp);
		keep_dst = dst;

		temp = vqadd_u8 (dval, sval);

		src += 8;
		dst += 8;
		w -= 8;
	    }

	    vst1_u8 ((void *)keep_dst, temp);
#else
	    asm volatile (
/* avoid using d8-d15 (q4-q7) aapcs callee-save registers */
	        "vld1.8  {d0}, [%[src]]\n\t"
	        "vld1.8  {d4}, [%[dst]]\n\t"
	        "mov     %[keep_dst], %[dst]\n\t"

	        "and ip, %[w], #7\n\t"
	        "add %[src], %[src], ip\n\t"
	        "add %[dst], %[dst], ip\n\t"
	        "subs %[w], %[w], ip\n\t"
	        "b 9f\n\t"
/* LOOP */
	        "2:\n\t"
	        "vld1.8  {d0}, [%[src]]!\n\t"
	        "vld1.8  {d4}, [%[dst]]!\n\t"
	        "vst1.8  {d20}, [%[keep_dst]]\n\t"
	        "sub     %[keep_dst], %[dst], #8\n\t"
	        "subs %[w], %[w], #8\n\t"
	        "9:\n\t"
	        "vqadd.u8 d20, d0, d4\n\t"

	        "bne 2b\n\t"

	        "1:\n\t"
	        "vst1.8  {d20}, [%[keep_dst]]\n\t"

		: [w] "+r" (w), [src] "+r" (src), [dst] "+r" (dst), [keep_dst] "=r" (keep_dst)
		:
		: "ip", "cc", "memory", "d0", "d4",
	        "d20"
	        );
#endif
	}
    }
    else
    {
	const uint8_t nil = 0;
	const uint8x8_t vnil = vld1_dup_u8 (&nil);

	while (height--)
	{
	    uint8x8_t sval = vnil, dval = vnil;
	    uint8_t *dst4 = 0, *dst2 = 0;

	    dst = dst_line;
	    dst_line += dst_stride;
	    src = src_line;
	    src_line += src_stride;
	    w = width;

	    if (w & 4)
	    {
		sval = vreinterpret_u8_u32 (
		    vld1_lane_u32 ((void *)src, vreinterpret_u32_u8 (sval), 1));
		dval = vreinterpret_u8_u32 (
		    vld1_lane_u32 ((void *)dst, vreinterpret_u32_u8 (dval), 1));

		dst4 = dst;
		src += 4;
		dst += 4;
	    }

	    if (w & 2)
	    {
		sval = vreinterpret_u8_u16 (
		    vld1_lane_u16 ((void *)src, vreinterpret_u16_u8 (sval), 1));
		dval = vreinterpret_u8_u16 (
		    vld1_lane_u16 ((void *)dst, vreinterpret_u16_u8 (dval), 1));

		dst2 = dst;
		src += 2;
		dst += 2;
	    }

	    if (w & 1)
	    {
		sval = vld1_lane_u8 (src, sval, 1);
		dval = vld1_lane_u8 (dst, dval, 1);
	    }

	    dval = vqadd_u8 (dval, sval);

	    if (w & 1)
		vst1_lane_u8 (dst, dval, 1);

	    if (w & 2)
		vst1_lane_u16 ((void *)dst2, vreinterpret_u16_u8 (dval), 1);

	    if (w & 4)
		vst1_lane_u32 ((void *)dst4, vreinterpret_u32_u8 (dval), 1);
	}
    }
}

static void
neon_composite_over_8888_8888 (pixman_implementation_t * impl,
                               pixman_op_t               op,
                               pixman_image_t *          src_image,
                               pixman_image_t *          mask_image,
                               pixman_image_t *          dst_image,
                               int32_t                   src_x,
                               int32_t                   src_y,
                               int32_t                   mask_x,
                               int32_t                   mask_y,
                               int32_t                   dest_x,
                               int32_t                   dest_y,
                               int32_t                   width,
                               int32_t                   height)
{
    uint32_t    *dst_line, *dst;
    uint32_t    *src_line, *src;
    int dst_stride, src_stride;
    uint32_t w;

    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint32_t, dst_stride, dst_line, 1);
    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, uint32_t, src_stride, src_line, 1);

    if (width >= 8)
    {
	/* Use overlapping 8-pixel method */
	while (height--)
	{
	    uint32_t *keep_dst = 0;
	    uint8x8x4_t sval, dval, temp;

	    dst = dst_line;
	    dst_line += dst_stride;
	    src = src_line;
	    src_line += src_stride;
	    w = width;

#ifndef USE_GCC_INLINE_ASM
	    sval = vld4_u8 ((void *)src);
	    dval = vld4_u8 ((void *)dst);
	    keep_dst = dst;

	    temp = neon8mul (dval, vmvn_u8 (sval.val[3]));
	    temp = neon8qadd (sval, temp);

	    src += (w & 7);
	    dst += (w & 7);
	    w -= (w & 7);

	    while (w)
	    {
		sval = vld4_u8 ((void *)src);
		dval = vld4_u8 ((void *)dst);

		vst4_u8 ((void *)keep_dst, temp);
		keep_dst = dst;

		temp = neon8mul (dval, vmvn_u8 (sval.val[3]));
		temp = neon8qadd (sval, temp);

		src += 8;
		dst += 8;
		w -= 8;
	    }

	    vst4_u8 ((void *)keep_dst, temp);
#else
	    asm volatile (
/* avoid using d8-d15 (q4-q7) aapcs callee-save registers */
	        "vld4.8  {d0-d3}, [%[src]]\n\t"
	        "vld4.8  {d4-d7}, [%[dst]]\n\t"
	        "mov     %[keep_dst], %[dst]\n\t"

	        "and ip, %[w], #7\n\t"
	        "add %[src], %[src], ip, LSL#2\n\t"
	        "add %[dst], %[dst], ip, LSL#2\n\t"
	        "subs %[w], %[w], ip\n\t"
	        "b 9f\n\t"
/* LOOP */
	        "2:\n\t"
	        "vld4.8  {d0-d3}, [%[src]]!\n\t"
	        "vld4.8  {d4-d7}, [%[dst]]!\n\t"
	        "vst4.8  {d20-d23}, [%[keep_dst]]\n\t"
	        "sub     %[keep_dst], %[dst], #8*4\n\t"
	        "subs %[w], %[w], #8\n\t"
	        "9:\n\t"
	        "vmvn.8  d31, d3\n\t"
	        "vmull.u8 q10, d31, d4\n\t"
	        "vmull.u8 q11, d31, d5\n\t"
	        "vmull.u8 q12, d31, d6\n\t"
	        "vmull.u8 q13, d31, d7\n\t"
	        "vrshr.u16 q8, q10, #8\n\t"
	        "vrshr.u16 q9, q11, #8\n\t"
	        "vraddhn.u16 d20, q10, q8\n\t"
	        "vraddhn.u16 d21, q11, q9\n\t"
	        "vrshr.u16 q8, q12, #8\n\t"
	        "vrshr.u16 q9, q13, #8\n\t"
	        "vraddhn.u16 d22, q12, q8\n\t"
	        "vraddhn.u16 d23, q13, q9\n\t"
/* result in d20-d23 */
	        "vqadd.u8 d20, d0, d20\n\t"
	        "vqadd.u8 d21, d1, d21\n\t"
	        "vqadd.u8 d22, d2, d22\n\t"
	        "vqadd.u8 d23, d3, d23\n\t"

	        "bne 2b\n\t"

	        "1:\n\t"
	        "vst4.8  {d20-d23}, [%[keep_dst]]\n\t"

		: [w] "+r" (w), [src] "+r" (src), [dst] "+r" (dst), [keep_dst] "=r" (keep_dst)
		:
		: "ip", "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
	        "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23"
	        );
#endif
	}
    }
    else
    {
	uint8x8_t alpha_selector = vreinterpret_u8_u64 (
	    vcreate_u64 (0x0707070703030303ULL));

	/* Handle width < 8 */
	while (height--)
	{
	    dst = dst_line;
	    dst_line += dst_stride;
	    src = src_line;
	    src_line += src_stride;
	    w = width;

	    while (w >= 2)
	    {
		uint8x8_t sval, dval;

		/* two 32-bit pixels packed into D-reg; ad-hoc vectorization */
		sval = vreinterpret_u8_u32 (vld1_u32 ((void *)src));
		dval = vreinterpret_u8_u32 (vld1_u32 ((void *)dst));
		dval = neon2mul (dval, vtbl1_u8 (vmvn_u8 (sval), alpha_selector));
		vst1_u8 ((void *)dst, vqadd_u8 (sval, dval));

		src += 2;
		dst += 2;
		w -= 2;
	    }

	    if (w)
	    {
		uint8x8_t sval, dval;

		/* single 32-bit pixel in lane 0 */
		sval = vreinterpret_u8_u32 (vld1_dup_u32 ((void *)src));  /* only interested in lane 0 */
		dval = vreinterpret_u8_u32 (vld1_dup_u32 ((void *)dst));  /* only interested in lane 0 */
		dval = neon2mul (dval, vtbl1_u8 (vmvn_u8 (sval), alpha_selector));
		vst1_lane_u32 ((void *)dst, vreinterpret_u32_u8 (vqadd_u8 (sval, dval)), 0);
	    }
	}
    }
}

static void
neon_composite_over_8888_n_8888 (pixman_implementation_t * impl,
                                 pixman_op_t               op,
                                 pixman_image_t *          src_image,
                                 pixman_image_t *          mask_image,
                                 pixman_image_t *          dst_image,
                                 int32_t                   src_x,
                                 int32_t                   src_y,
                                 int32_t                   mask_x,
                                 int32_t                   mask_y,
                                 int32_t                   dest_x,
                                 int32_t                   dest_y,
                                 int32_t                   width,
                                 int32_t                   height)
{
    uint32_t    *dst_line, *dst;
    uint32_t    *src_line, *src;
    uint32_t mask;
    int dst_stride, src_stride;
    uint32_t w;
    uint8x8_t mask_alpha;

    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint32_t, dst_stride, dst_line, 1);
    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, uint32_t, src_stride, src_line, 1);

    mask = _pixman_image_get_solid (mask_image, dst_image->bits.format);
    mask_alpha = vdup_n_u8 ((mask) >> 24);

    if (width >= 8)
    {
	/* Use overlapping 8-pixel method */
	while (height--)
	{
	    dst = dst_line;
	    dst_line += dst_stride;
	    src = src_line;
	    src_line += src_stride;
	    w = width;

	    uint32_t *keep_dst = 0;

#ifndef USE_GCC_INLINE_ASM
	    uint8x8x4_t sval, dval, temp;

	    sval = vld4_u8 ((void *)src);
	    dval = vld4_u8 ((void *)dst);
	    keep_dst = dst;

	    sval = neon8mul (sval, mask_alpha);
	    temp = neon8mul (dval, vmvn_u8 (sval.val[3]));
	    temp = neon8qadd (sval, temp);

	    src += (w & 7);
	    dst += (w & 7);
	    w -= (w & 7);

	    while (w)
	    {
		sval = vld4_u8 ((void *)src);
		dval = vld4_u8 ((void *)dst);

		vst4_u8 ((void *)keep_dst, temp);
		keep_dst = dst;

		sval = neon8mul (sval, mask_alpha);
		temp = neon8mul (dval, vmvn_u8 (sval.val[3]));
		temp = neon8qadd (sval, temp);

		src += 8;
		dst += 8;
		w -= 8;
	    }
	    vst4_u8 ((void *)keep_dst, temp);
#else
	    asm volatile (
/* avoid using d8-d15 (q4-q7) aapcs callee-save registers */
	        "vdup.32      d30, %[mask]\n\t"
	        "vdup.8       d30, d30[3]\n\t"

	        "vld4.8       {d0-d3}, [%[src]]\n\t"
	        "vld4.8       {d4-d7}, [%[dst]]\n\t"
	        "mov  %[keep_dst], %[dst]\n\t"

	        "and  ip, %[w], #7\n\t"
	        "add  %[src], %[src], ip, LSL#2\n\t"
	        "add  %[dst], %[dst], ip, LSL#2\n\t"
	        "subs  %[w], %[w], ip\n\t"
	        "b 9f\n\t"
/* LOOP */
	        "2:\n\t"
	        "vld4.8       {d0-d3}, [%[src]]!\n\t"
	        "vld4.8       {d4-d7}, [%[dst]]!\n\t"
	        "vst4.8       {d20-d23}, [%[keep_dst]]\n\t"
	        "sub  %[keep_dst], %[dst], #8*4\n\t"
	        "subs  %[w], %[w], #8\n\t"

	        "9:\n\t"
	        "vmull.u8     q10, d30, d0\n\t"
	        "vmull.u8     q11, d30, d1\n\t"
	        "vmull.u8     q12, d30, d2\n\t"
	        "vmull.u8     q13, d30, d3\n\t"
	        "vrshr.u16    q8, q10, #8\n\t"
	        "vrshr.u16    q9, q11, #8\n\t"
	        "vraddhn.u16  d0, q10, q8\n\t"
	        "vraddhn.u16  d1, q11, q9\n\t"
	        "vrshr.u16    q9, q13, #8\n\t"
	        "vrshr.u16    q8, q12, #8\n\t"
	        "vraddhn.u16  d3, q13, q9\n\t"
	        "vraddhn.u16  d2, q12, q8\n\t"

	        "vmvn.8       d31, d3\n\t"
	        "vmull.u8     q10, d31, d4\n\t"
	        "vmull.u8     q11, d31, d5\n\t"
	        "vmull.u8     q12, d31, d6\n\t"
	        "vmull.u8     q13, d31, d7\n\t"
	        "vrshr.u16    q8, q10, #8\n\t"
	        "vrshr.u16    q9, q11, #8\n\t"
	        "vraddhn.u16  d20, q10, q8\n\t"
	        "vrshr.u16    q8, q12, #8\n\t"
	        "vraddhn.u16  d21, q11, q9\n\t"
	        "vrshr.u16    q9, q13, #8\n\t"
	        "vraddhn.u16  d22, q12, q8\n\t"
	        "vraddhn.u16  d23, q13, q9\n\t"

/* result in d20-d23 */
	        "vqadd.u8     d20, d0, d20\n\t"
	        "vqadd.u8     d21, d1, d21\n\t"
	        "vqadd.u8     d22, d2, d22\n\t"
	        "vqadd.u8     d23, d3, d23\n\t"

	        "bne  2b\n\t"

	        "1:\n\t"
	        "vst4.8       {d20-d23}, [%[keep_dst]]\n\t"

		: [w] "+r" (w), [src] "+r" (src), [dst] "+r" (dst), [keep_dst] "=r" (keep_dst)
		: [mask] "r" (mask)
		: "ip", "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
	        "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27",
	        "d30", "d31"
	        );
#endif
	}
    }
    else
    {
	uint8x8_t alpha_selector = vreinterpret_u8_u64 (vcreate_u64 (0x0707070703030303ULL));

	/* Handle width < 8 */
	while (height--)
	{
	    dst = dst_line;
	    dst_line += dst_stride;
	    src = src_line;
	    src_line += src_stride;
	    w = width;

	    while (w >= 2)
	    {
		uint8x8_t sval, dval;

		sval = vreinterpret_u8_u32 (vld1_u32 ((void *)src));
		dval = vreinterpret_u8_u32 (vld1_u32 ((void *)dst));

		/* sval * const alpha_mul */
		sval = neon2mul (sval, mask_alpha);

		/* dval * 255-(src alpha) */
		dval = neon2mul (dval, vtbl1_u8 (vmvn_u8 (sval), alpha_selector));

		vst1_u8 ((void *)dst, vqadd_u8 (sval, dval));

		src += 2;
		dst += 2;
		w -= 2;
	    }

	    if (w)
	    {
		uint8x8_t sval, dval;

		sval = vreinterpret_u8_u32 (vld1_dup_u32 ((void *)src));
		dval = vreinterpret_u8_u32 (vld1_dup_u32 ((void *)dst));

		/* sval * const alpha_mul */
		sval = neon2mul (sval, mask_alpha);

		/* dval * 255-(src alpha) */
		dval = neon2mul (dval, vtbl1_u8 (vmvn_u8 (sval), alpha_selector));

		vst1_lane_u32 ((void *)dst, vreinterpret_u32_u8 (vqadd_u8 (sval, dval)), 0);
	    }
	}
    }
}

static void
neon_composite_over_n_8_0565 (pixman_implementation_t * impl,
			      pixman_op_t               op,
			      pixman_image_t *          src_image,
			      pixman_image_t *          mask_image,
			      pixman_image_t *          dst_image,
			      int32_t                   src_x,
			      int32_t                   src_y,
			      int32_t                   mask_x,
			      int32_t                   mask_y,
			      int32_t                   dest_x,
			      int32_t                   dest_y,
			      int32_t                   width,
			      int32_t                   height)
{
    uint32_t     src, srca;
    uint16_t    *dst_line, *dst;
    uint8_t     *mask_line, *mask;
    int          dst_stride, mask_stride;
    uint32_t     w;
    uint8x8_t    sval2;
    uint8x8x4_t  sval8;

    src = _pixman_image_get_solid (src_image, dst_image->bits.format);

    srca = src >> 24;
    if (src == 0)
	return;

    sval2=vreinterpret_u8_u32 (vdup_n_u32 (src));
    sval8.val[0]=vdup_lane_u8 (sval2,0);
    sval8.val[1]=vdup_lane_u8 (sval2,1);
    sval8.val[2]=vdup_lane_u8 (sval2,2);
    sval8.val[3]=vdup_lane_u8 (sval2,3);

    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint16_t, dst_stride, dst_line, 1);
    PIXMAN_IMAGE_GET_LINE (mask_image, mask_x, mask_y, uint8_t, mask_stride, mask_line, 1);

    if (width>=8)
    {
	/* Use overlapping 8-pixel method, modified to avoid rewritten dest being reused */
	while (height--)
	{
	    uint16_t *keep_dst=0;

	    dst = dst_line;
	    dst_line += dst_stride;
	    mask = mask_line;
	    mask_line += mask_stride;
	    w = width;

#ifndef USE_GCC_INLINE_ASM
	    uint8x8_t alpha;
	    uint16x8_t dval, temp;
	    uint8x8x4_t sval8temp;

	    alpha = vld1_u8 ((void *)mask);
	    dval = vld1q_u16 ((void *)dst);
	    keep_dst = dst;

	    sval8temp = neon8mul (sval8, alpha);
	    temp = pack0565 (neon8qadd (sval8temp, neon8mul (unpack0565 (dval), vmvn_u8 (sval8temp.val[3]))));

	    mask += (w & 7);
	    dst += (w & 7);
	    w -= (w & 7);

	    while (w)
	    {
		dval = vld1q_u16 ((void *)dst);
		alpha = vld1_u8 ((void *)mask);

		vst1q_u16 ((void *)keep_dst, temp);
		keep_dst = dst;

		sval8temp = neon8mul (sval8, alpha);
		temp = pack0565 (neon8qadd (sval8temp, neon8mul (unpack0565 (dval), vmvn_u8 (sval8temp.val[3]))));

		mask+=8;
		dst+=8;
		w-=8;
	    }
	    vst1q_u16 ((void *)keep_dst, temp);
#else
	    asm volatile (
		"vdup.32      d0, %[src]\n\t"
		"vdup.8       d1, d0[1]\n\t"
		"vdup.8       d2, d0[2]\n\t"
		"vdup.8       d3, d0[3]\n\t"
		"vdup.8       d0, d0[0]\n\t"

		"vld1.8       {q12}, [%[dst]]\n\t"
		"vld1.8       {d31}, [%[mask]]\n\t"
		"mov  %[keep_dst], %[dst]\n\t"

		"and  ip, %[w], #7\n\t"
		"add  %[mask], %[mask], ip\n\t"
		"add  %[dst], %[dst], ip, LSL#1\n\t"
		"subs  %[w], %[w], ip\n\t"
		"b  9f\n\t"
/* LOOP */
		"2:\n\t"

		"vld1.16      {q12}, [%[dst]]!\n\t"
		"vld1.8       {d31}, [%[mask]]!\n\t"
		"vst1.16      {q10}, [%[keep_dst]]\n\t"
		"sub  %[keep_dst], %[dst], #8*2\n\t"
		"subs  %[w], %[w], #8\n\t"
		"9:\n\t"
/* expand 0565 q12 to 8888 {d4-d7} */
		"vmovn.u16    d4, q12\t\n"
		"vshr.u16     q11, q12, #5\t\n"
		"vshr.u16     q10, q12, #6+5\t\n"
		"vmovn.u16    d5, q11\t\n"
		"vmovn.u16    d6, q10\t\n"
		"vshl.u8      d4, d4, #3\t\n"
		"vshl.u8      d5, d5, #2\t\n"
		"vshl.u8      d6, d6, #3\t\n"
		"vsri.u8      d4, d4, #5\t\n"
		"vsri.u8      d5, d5, #6\t\n"
		"vsri.u8      d6, d6, #5\t\n"

		"vmull.u8     q10, d31, d0\n\t"
		"vmull.u8     q11, d31, d1\n\t"
		"vmull.u8     q12, d31, d2\n\t"
		"vmull.u8     q13, d31, d3\n\t"
		"vrshr.u16    q8, q10, #8\n\t"
		"vrshr.u16    q9, q11, #8\n\t"
		"vraddhn.u16  d20, q10, q8\n\t"
		"vraddhn.u16  d21, q11, q9\n\t"
		"vrshr.u16    q9, q13, #8\n\t"
		"vrshr.u16    q8, q12, #8\n\t"
		"vraddhn.u16  d23, q13, q9\n\t"
		"vraddhn.u16  d22, q12, q8\n\t"

/* duplicate in 4/2/1 & 8pix vsns */
		"vmvn.8       d30, d23\n\t"
		"vmull.u8     q14, d30, d6\n\t"
		"vmull.u8     q13, d30, d5\n\t"
		"vmull.u8     q12, d30, d4\n\t"
		"vrshr.u16    q8, q14, #8\n\t"
		"vrshr.u16    q9, q13, #8\n\t"
		"vraddhn.u16  d6, q14, q8\n\t"
		"vrshr.u16    q8, q12, #8\n\t"
		"vraddhn.u16  d5, q13, q9\n\t"
		"vqadd.u8     d6, d6, d22\n\t"  /* moved up */
		"vraddhn.u16  d4, q12, q8\n\t"
/* intentionally don't calculate alpha */
/* result in d4-d6 */

/*              "vqadd.u8     d6, d6, d22\n\t"  ** moved up */
		"vqadd.u8     d5, d5, d21\n\t"
		"vqadd.u8     d4, d4, d20\n\t"

/* pack 8888 {d20-d23} to 0565 q10 */
		"vshll.u8     q10, d6, #8\n\t"
		"vshll.u8     q3, d5, #8\n\t"
		"vshll.u8     q2, d4, #8\n\t"
		"vsri.u16     q10, q3, #5\t\n"
		"vsri.u16     q10, q2, #11\t\n"

		"bne 2b\n\t"

		"1:\n\t"
		"vst1.16      {q10}, [%[keep_dst]]\n\t"

		: [w] "+r" (w), [dst] "+r" (dst), [mask] "+r" (mask), [keep_dst] "=r" (keep_dst)
		: [src] "r" (src)
		: "ip", "cc", "memory", "d0","d1","d2","d3","d4","d5","d6","d7",
		  "d16","d17","d18","d19","d20","d21","d22","d23","d24","d25","d26","d27","d28","d29",
		  "d30","d31"
		);
#endif
	}
    }
    else
    {
	while (height--)
	{
	    void *dst4=0, *dst2=0;

	    dst = dst_line;
	    dst_line += dst_stride;
	    mask = mask_line;
	    mask_line += mask_stride;
	    w = width;


#if 1 /* #ifndef USE_GCC_INLINE_ASM */
	    uint8x8_t alpha;
	    uint16x8_t dval, temp;
	    uint8x8x4_t sval8temp;

	    if (w&4)
	    {
		alpha = vreinterpret_u8_u32 (vld1_lane_u32 ((void *)mask, vreinterpret_u32_u8 (alpha),1));
		dval = vreinterpretq_u16_u64 (vld1q_lane_u64 ((void *)dst, vreinterpretq_u64_u16 (dval),1));
		dst4=dst;
		mask+=4;
		dst+=4;
	    }
	    if (w&2)
	    {
		alpha = vreinterpret_u8_u16 (vld1_lane_u16 ((void *)mask, vreinterpret_u16_u8 (alpha),1));
		dval = vreinterpretq_u16_u32 (vld1q_lane_u32 ((void *)dst, vreinterpretq_u32_u16 (dval),1));
		dst2=dst;
		mask+=2;
		dst+=2;
	    }
	    if (w&1)
	    {
		alpha = vld1_lane_u8 ((void *)mask, alpha,1);
		dval = vld1q_lane_u16 ((void *)dst, dval,1);
	    }

	    sval8temp = neon8mul (sval8, alpha);
	    temp = pack0565 (neon8qadd (sval8temp, neon8mul (unpack0565 (dval), vmvn_u8 (sval8temp.val[3]))));

	    if (w&1)
		vst1q_lane_u16 ((void *)dst, temp,1);
	    if (w&2)
		vst1q_lane_u32 ((void *)dst2, vreinterpretq_u32_u16 (temp),1);
	    if (w&4)
		vst1q_lane_u64 ((void *)dst4, vreinterpretq_u64_u16 (temp),1);
#else
	    /* this code has some bug (does not pass blitters-test) */
	    asm volatile (
		"vdup.32      d0, %[src]\n\t"
		"vdup.8       d1, d0[1]\n\t"
		"vdup.8       d2, d0[2]\n\t"
		"vdup.8       d3, d0[3]\n\t"
		"vdup.8       d0, d0[0]\n\t"

		"tst  %[w], #4\t\n"
		"beq  skip_load4\t\n"

		"vld1.64      {d25}, [%[dst]]\n\t"
		"vld1.32      {d31[1]}, [%[mask]]\n\t"
		"mov  %[dst4], %[dst]\t\n"
		"add  %[mask], %[mask], #4\t\n"
		"add  %[dst], %[dst], #4*2\t\n"

		"skip_load4:\t\n"
		"tst  %[w], #2\t\n"
		"beq  skip_load2\t\n"
		"vld1.32      {d24[1]}, [%[dst]]\n\t"
		"vld1.16      {d31[1]}, [%[mask]]\n\t"
		"mov  %[dst2], %[dst]\t\n"
		"add  %[mask], %[mask], #2\t\n"
		"add  %[dst], %[dst], #2*2\t\n"

		"skip_load2:\t\n"
		"tst  %[w], #1\t\n"
		"beq  skip_load1\t\n"
		"vld1.16      {d24[1]}, [%[dst]]\n\t"
		"vld1.8       {d31[1]}, [%[mask]]\n\t"

		"skip_load1:\t\n"
/* expand 0565 q12 to 8888 {d4-d7} */
		"vmovn.u16    d4, q12\t\n"
		"vshr.u16     q11, q12, #5\t\n"
		"vshr.u16     q10, q12, #6+5\t\n"
		"vmovn.u16    d5, q11\t\n"
		"vmovn.u16    d6, q10\t\n"
		"vshl.u8      d4, d4, #3\t\n"
		"vshl.u8      d5, d5, #2\t\n"
		"vshl.u8      d6, d6, #3\t\n"
		"vsri.u8      d4, d4, #5\t\n"
		"vsri.u8      d5, d5, #6\t\n"
		"vsri.u8      d6, d6, #5\t\n"

		"vmull.u8     q10, d31, d0\n\t"
		"vmull.u8     q11, d31, d1\n\t"
		"vmull.u8     q12, d31, d2\n\t"
		"vmull.u8     q13, d31, d3\n\t"
		"vrshr.u16    q8, q10, #8\n\t"
		"vrshr.u16    q9, q11, #8\n\t"
		"vraddhn.u16  d20, q10, q8\n\t"
		"vraddhn.u16  d21, q11, q9\n\t"
		"vrshr.u16    q9, q13, #8\n\t"
		"vrshr.u16    q8, q12, #8\n\t"
		"vraddhn.u16  d23, q13, q9\n\t"
		"vraddhn.u16  d22, q12, q8\n\t"

/* duplicate in 4/2/1 & 8pix vsns */
		"vmvn.8       d30, d23\n\t"
		"vmull.u8     q14, d30, d6\n\t"
		"vmull.u8     q13, d30, d5\n\t"
		"vmull.u8     q12, d30, d4\n\t"
		"vrshr.u16    q8, q14, #8\n\t"
		"vrshr.u16    q9, q13, #8\n\t"
		"vraddhn.u16  d6, q14, q8\n\t"
		"vrshr.u16    q8, q12, #8\n\t"
		"vraddhn.u16  d5, q13, q9\n\t"
		"vqadd.u8     d6, d6, d22\n\t"  /* moved up */
		"vraddhn.u16  d4, q12, q8\n\t"
/* intentionally don't calculate alpha */
/* result in d4-d6 */

/*              "vqadd.u8     d6, d6, d22\n\t"  ** moved up */
		"vqadd.u8     d5, d5, d21\n\t"
		"vqadd.u8     d4, d4, d20\n\t"

/* pack 8888 {d20-d23} to 0565 q10 */
		"vshll.u8     q10, d6, #8\n\t"
		"vshll.u8     q3, d5, #8\n\t"
		"vshll.u8     q2, d4, #8\n\t"
		"vsri.u16     q10, q3, #5\t\n"
		"vsri.u16     q10, q2, #11\t\n"

		"tst  %[w], #1\n\t"
		"beq skip_store1\t\n"
		"vst1.16      {d20[1]}, [%[dst]]\t\n"
		"skip_store1:\t\n"
		"tst  %[w], #2\n\t"
		"beq  skip_store2\t\n"
		"vst1.32      {d20[1]}, [%[dst2]]\t\n"
		"skip_store2:\t\n"
		"tst  %[w], #4\n\t"
		"beq skip_store4\t\n"
		"vst1.16      {d21}, [%[dst4]]\t\n"
		"skip_store4:\t\n"

		: [w] "+r" (w), [dst] "+r" (dst), [mask] "+r" (mask), [dst4] "+r" (dst4), [dst2] "+r" (dst2)
		: [src] "r" (src)
		: "ip", "cc", "memory", "d0","d1","d2","d3","d4","d5","d6","d7",
		  "d16","d17","d18","d19","d20","d21","d22","d23","d24","d25","d26","d27","d28","d29",
		  "d30","d31"
		);
#endif
	}
    }
}

static void
neon_composite_over_n_8_8888 (pixman_implementation_t * impl,
                              pixman_op_t               op,
                              pixman_image_t *          src_image,
                              pixman_image_t *          mask_image,
                              pixman_image_t *          dst_image,
                              int32_t                   src_x,
                              int32_t                   src_y,
                              int32_t                   mask_x,
                              int32_t                   mask_y,
                              int32_t                   dest_x,
                              int32_t                   dest_y,
                              int32_t                   width,
                              int32_t                   height)
{
    uint32_t src, srca;
    uint32_t    *dst_line, *dst;
    uint8_t     *mask_line, *mask;
    int dst_stride, mask_stride;
    uint32_t w;
    uint8x8_t sval2;
    uint8x8x4_t sval8;
    uint8x8_t mask_selector = vreinterpret_u8_u64 (vcreate_u64 (0x0101010100000000ULL));
    uint8x8_t alpha_selector = vreinterpret_u8_u64 (vcreate_u64 (0x0707070703030303ULL));

    src = _pixman_image_get_solid (src_image, dst_image->bits.format);
    
    /* bail out if fully transparent */
    srca = src >> 24;
    if (src == 0)
	return;

    sval2 = vreinterpret_u8_u32 (vdup_n_u32 (src));
    sval8.val[0] = vdup_lane_u8 (sval2, 0);
    sval8.val[1] = vdup_lane_u8 (sval2, 1);
    sval8.val[2] = vdup_lane_u8 (sval2, 2);
    sval8.val[3] = vdup_lane_u8 (sval2, 3);

    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint32_t, dst_stride, dst_line, 1);
    PIXMAN_IMAGE_GET_LINE (mask_image, mask_x, mask_y, uint8_t, mask_stride, mask_line, 1);

    if (width >= 8)
    {
	/* Use overlapping 8-pixel method, modified to avoid
	 * rewritten dest being reused
	 */
	while (height--)
	{
	    uint32_t *keep_dst = 0;

	    dst = dst_line;
	    dst_line += dst_stride;
	    mask = mask_line;
	    mask_line += mask_stride;
	    w = width;

#ifndef USE_GCC_INLINE_ASM
	    uint8x8_t alpha;
	    uint8x8x4_t dval, temp;

	    alpha = vld1_u8 ((void *)mask);
	    dval = vld4_u8 ((void *)dst);
	    keep_dst = dst;

	    temp = neon8mul (sval8, alpha);
	    dval = neon8mul (dval, vmvn_u8 (temp.val[3]));
	    temp = neon8qadd (temp, dval);

	    mask += (w & 7);
	    dst += (w & 7);
	    w -= (w & 7);

	    while (w)
	    {
		alpha = vld1_u8 ((void *)mask);
		dval = vld4_u8 ((void *)dst);

		vst4_u8 ((void *)keep_dst, temp);
		keep_dst = dst;

		temp = neon8mul (sval8, alpha);
		dval = neon8mul (dval, vmvn_u8 (temp.val[3]));
		temp = neon8qadd (temp, dval);

		mask += 8;
		dst += 8;
		w -= 8;
	    }
	    vst4_u8 ((void *)keep_dst, temp);
#else
	    asm volatile (
	        "vdup.32      d0, %[src]\n\t"
	        "vdup.8       d1, d0[1]\n\t"
	        "vdup.8       d2, d0[2]\n\t"
	        "vdup.8       d3, d0[3]\n\t"
	        "vdup.8       d0, d0[0]\n\t"

	        "vld4.8       {d4-d7}, [%[dst]]\n\t"
	        "vld1.8       {d31}, [%[mask]]\n\t"
	        "mov  %[keep_dst], %[dst]\n\t"

	        "and  ip, %[w], #7\n\t"
	        "add  %[mask], %[mask], ip\n\t"
	        "add  %[dst], %[dst], ip, LSL#2\n\t"
	        "subs  %[w], %[w], ip\n\t"
	        "b 9f\n\t"
/* LOOP */
	        "2:\n\t"
	        "vld4.8       {d4-d7}, [%[dst]]!\n\t"
	        "vld1.8       {d31}, [%[mask]]!\n\t"
	        "vst4.8       {d20-d23}, [%[keep_dst]]\n\t"
	        "sub  %[keep_dst], %[dst], #8*4\n\t"
	        "subs  %[w], %[w], #8\n\t"
	        "9:\n\t"

	        "vmull.u8     q10, d31, d0\n\t"
	        "vmull.u8     q11, d31, d1\n\t"
	        "vmull.u8     q12, d31, d2\n\t"
	        "vmull.u8     q13, d31, d3\n\t"
	        "vrshr.u16    q8, q10, #8\n\t"
	        "vrshr.u16    q9, q11, #8\n\t"
	        "vraddhn.u16  d20, q10, q8\n\t"
	        "vraddhn.u16  d21, q11, q9\n\t"
	        "vrshr.u16    q9, q13, #8\n\t"
	        "vrshr.u16    q8, q12, #8\n\t"
	        "vraddhn.u16  d23, q13, q9\n\t"
	        "vraddhn.u16  d22, q12, q8\n\t"

	        "vmvn.8       d30, d23\n\t"
	        "vmull.u8     q12, d30, d4\n\t"
	        "vmull.u8     q13, d30, d5\n\t"
	        "vmull.u8     q14, d30, d6\n\t"
	        "vmull.u8     q15, d30, d7\n\t"

	        "vrshr.u16    q8, q12, #8\n\t"
	        "vrshr.u16    q9, q13, #8\n\t"
	        "vraddhn.u16  d4, q12, q8\n\t"
	        "vrshr.u16    q8, q14, #8\n\t"
	        "vraddhn.u16  d5, q13, q9\n\t"
	        "vrshr.u16    q9, q15, #8\n\t"
	        "vraddhn.u16  d6, q14, q8\n\t"
	        "vraddhn.u16  d7, q15, q9\n\t"
/* result in d4-d7 */

	        "vqadd.u8     d20, d4, d20\n\t"
	        "vqadd.u8     d21, d5, d21\n\t"
	        "vqadd.u8     d22, d6, d22\n\t"
	        "vqadd.u8     d23, d7, d23\n\t"

	        "bne 2b\n\t"

	        "1:\n\t"
	        "vst4.8       {d20-d23}, [%[keep_dst]]\n\t"

		: [w] "+r" (w), [dst] "+r" (dst), [mask] "+r" (mask), [keep_dst] "=r" (keep_dst)
		: [src] "r" (src)
		: "ip", "cc", "memory", "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
	        "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29",
	        "d30", "d31"
	        );
#endif
	}
    }
    else
    {
	while (height--)
	{
	    uint8x8_t alpha;

	    dst = dst_line;
	    dst_line += dst_stride;
	    mask = mask_line;
	    mask_line += mask_stride;
	    w = width;

	    while (w >= 2)
	    {
		uint8x8_t dval, temp, res;

		alpha = vtbl1_u8 (
		    vreinterpret_u8_u16 (vld1_dup_u16 ((void *)mask)), mask_selector);
		dval = vld1_u8 ((void *)dst);

		temp = neon2mul (sval2, alpha);
		res = vqadd_u8 (
		    temp, neon2mul (dval, vtbl1_u8 (vmvn_u8 (temp), alpha_selector)));

		vst1_u8 ((void *)dst, res);

		mask += 2;
		dst += 2;
		w -= 2;
	    }

	    if (w)
	    {
		uint8x8_t dval, temp, res;

		alpha = vtbl1_u8 (vld1_dup_u8 ((void *)mask), mask_selector);
		dval = vreinterpret_u8_u32 (vld1_dup_u32 ((void *)dst));

		temp = neon2mul (sval2, alpha);
		res = vqadd_u8 (
		    temp, neon2mul (dval, vtbl1_u8 (vmvn_u8 (temp), alpha_selector)));

		vst1_lane_u32 ((void *)dst, vreinterpret_u32_u8 (res), 0);
	    }
	}
    }
}

static void
neon_composite_add_8888_8_8 (pixman_implementation_t * impl,
                             pixman_op_t               op,
                             pixman_image_t *          src_image,
                             pixman_image_t *          mask_image,
                             pixman_image_t *          dst_image,
                             int32_t                   src_x,
                             int32_t                   src_y,
                             int32_t                   mask_x,
                             int32_t                   mask_y,
                             int32_t                   dest_x,
                             int32_t                   dest_y,
                             int32_t                   width,
                             int32_t                   height)
{
    uint8_t     *dst_line, *dst;
    uint8_t     *mask_line, *mask;
    int dst_stride, mask_stride;
    uint32_t w;
    uint32_t src;
    uint8x8_t sa;

    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint8_t, dst_stride, dst_line, 1);
    PIXMAN_IMAGE_GET_LINE (mask_image, mask_x, mask_y, uint8_t, mask_stride, mask_line, 1);
    src = _pixman_image_get_solid (src_image, dst_image->bits.format);
    sa = vdup_n_u8 ((src) >> 24);

    if (width >= 8)
    {
	/* Use overlapping 8-pixel method, modified to avoid rewritten dest being reused */
	while (height--)
	{
	    dst = dst_line;
	    dst_line += dst_stride;
	    mask = mask_line;
	    mask_line += mask_stride;
	    w = width;

	    uint8x8_t mval, dval, res;
	    uint8_t     *keep_dst;

	    mval = vld1_u8 ((void *)mask);
	    dval = vld1_u8 ((void *)dst);
	    keep_dst = dst;

	    res = vqadd_u8 (neon2mul (mval, sa), dval);

	    mask += (w & 7);
	    dst += (w & 7);
	    w -= w & 7;

	    while (w)
	    {
		mval = vld1_u8 ((void *)mask);
		dval = vld1_u8 ((void *)dst);
		vst1_u8 ((void *)keep_dst, res);
		keep_dst = dst;

		res = vqadd_u8 (neon2mul (mval, sa), dval);

		mask += 8;
		dst += 8;
		w -= 8;
	    }
	    vst1_u8 ((void *)keep_dst, res);
	}
    }
    else
    {
	/* Use 4/2/1 load/store method to handle 1-7 pixels */
	while (height--)
	{
	    dst = dst_line;
	    dst_line += dst_stride;
	    mask = mask_line;
	    mask_line += mask_stride;
	    w = width;

	    uint8x8_t mval = sa, dval = sa, res;
	    uint8_t *dst4 = 0, *dst2 = 0;

	    if (w & 4)
	    {
		mval = vreinterpret_u8_u32 (
		    vld1_lane_u32 ((void *)mask, vreinterpret_u32_u8 (mval), 1));
		dval = vreinterpret_u8_u32 (
		    vld1_lane_u32 ((void *)dst, vreinterpret_u32_u8 (dval), 1));

		dst4 = dst;
		mask += 4;
		dst += 4;
	    }

	    if (w & 2)
	    {
		mval = vreinterpret_u8_u16 (
		    vld1_lane_u16 ((void *)mask, vreinterpret_u16_u8 (mval), 1));
		dval = vreinterpret_u8_u16 (
		    vld1_lane_u16 ((void *)dst, vreinterpret_u16_u8 (dval), 1));
		dst2 = dst;
		mask += 2;
		dst += 2;
	    }

	    if (w & 1)
	    {
		mval = vld1_lane_u8 (mask, mval, 1);
		dval = vld1_lane_u8 (dst, dval, 1);
	    }

	    res = vqadd_u8 (neon2mul (mval, sa), dval);

	    if (w & 1)
		vst1_lane_u8 (dst, res, 1);
	    if (w & 2)
		vst1_lane_u16 ((void *)dst2, vreinterpret_u16_u8 (res), 1);
	    if (w & 4)
		vst1_lane_u32 ((void *)dst4, vreinterpret_u32_u8 (res), 1);
	}
    }
}

#ifdef USE_GCC_INLINE_ASM

static void
neon_composite_src_16_16 (pixman_implementation_t * impl,
                          pixman_op_t               op,
                          pixman_image_t *          src_image,
                          pixman_image_t *          mask_image,
                          pixman_image_t *          dst_image,
                          int32_t                   src_x,
                          int32_t                   src_y,
                          int32_t                   mask_x,
                          int32_t                   mask_y,
                          int32_t                   dest_x,
                          int32_t                   dest_y,
                          int32_t                   width,
                          int32_t                   height)
{
    uint16_t    *dst_line, *src_line;
    uint32_t dst_stride, src_stride;

    if (!height || !width)
	return;

    /* We simply copy 16-bit-aligned pixels from one place to another. */
    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, uint16_t, src_stride, src_line, 1);
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint16_t, dst_stride, dst_line, 1);

    /* Preload the first input scanline */
    {
	uint16_t *src_ptr = src_line;
	uint32_t count = width;

	asm volatile (
	    "0: @ loop							\n"
	    "	subs    %[count], %[count], #32				\n"
	    "	pld     [%[src]]					\n"
	    "	add     %[src], %[src], #64				\n"
	    "	bgt 0b							\n"

	    /* Clobbered input registers marked as input/outputs */
	    : [src] "+r" (src_ptr), [count] "+r" (count)
	    :     /* no unclobbered inputs */
	    : "cc"
	    );
    }

    while (height--)
    {
	uint16_t *dst_ptr = dst_line;
	uint16_t *src_ptr = src_line;
	uint32_t count = width;
	uint32_t tmp = 0;

	/* Uses multi-register access and preloading to maximise bandwidth.
	 * Each pixel is one halfword, so a quadword contains 8px.
	 * Preload frequency assumed a 64-byte cacheline.
	 */
	asm volatile (
	    "	cmp       %[count], #64				\n"
	    "	blt 1f    @ skip oversized fragments		\n"
	    "0: @ start with eight quadwords at a time		\n"
	    /* preload from next scanline */
	    "	pld       [%[src], %[src_stride], LSL #1]	\n"
	    "	sub       %[count], %[count], #64		\n"
	    "	vld1.16   {d16, d17, d18, d19}, [%[src]]!		\n"
	    "	vld1.16   {d20, d21, d22, d23}, [%[src]]!		\n"
	    /* preload from next scanline */
	    "	pld       [%[src], %[src_stride], LSL #1]	\n"
	    "	vld1.16   {d24, d25, d26, d27}, [%[src]]!		\n"
	    "	vld1.16   {d28, d29, d30, d31}, [%[src]]!		\n"
	    "	cmp       %[count], #64				\n"
	    "	vst1.16   {d16, d17, d18, d19}, [%[dst]]!		\n"
	    "	vst1.16   {d20, d21, d22, d23}, [%[dst]]!		\n"
	    "	vst1.16   {d24, d25, d26, d27}, [%[dst]]!		\n"
	    "	vst1.16   {d28, d29, d30, d31}, [%[dst]]!		\n"
	    "	bge 0b						\n"
	    "	cmp       %[count], #0				\n"
	    "	beq 7f    @ aligned fastpath			\n"
	    "1: @ four quadwords				\n"
	    "	tst       %[count], #32				\n"
	    "	beq 2f    @ skip oversized fragment		\n"
	    /* preload from next scanline */
	    "	pld       [%[src], %[src_stride], LSL #1]	\n"
	    "	vld1.16   {d16, d17, d18, d19}, [%[src]]!		\n"
	    "	vld1.16   {d20, d21, d22, d23}, [%[src]]!		\n"
	    "	vst1.16   {d16, d17, d18, d19}, [%[dst]]!		\n"
	    "	vst1.16   {d20, d21, d22, d23}, [%[dst]]!		\n"
	    "2: @ two quadwords					\n"
	    "	tst       %[count], #16				\n"
	    "	beq 3f    @ skip oversized fragment		\n"
	    /* preload from next scanline */
	    "	pld       [%[src], %[src_stride], LSL #1]	\n"
	    "	vld1.16   {d16, d17, d18, d19}, [%[src]]!		\n"
	    "	vst1.16   {d16, d17, d18, d19}, [%[dst]]!		\n"
	    "3: @ one quadword					\n"
	    "	tst       %[count], #8				\n"
	    "	beq 4f    @ skip oversized fragment		\n"
	    "	vld1.16   {d16, d17}, [%[src]]!			\n"
	    "	vst1.16   {d16, d17}, [%[dst]]!			\n"
	    "4: @ one doubleword				\n"
	    "	tst       %[count], #4				\n"
	    "	beq 5f    @ skip oversized fragment		\n"
	    "	vld1.16   {d16}, [%[src]]!			\n"
	    "	vst1.16   {d16}, [%[dst]]!			\n"
	    "5: @ one word					\n"
	    "	tst       %[count], #2				\n"
	    "	beq 6f    @ skip oversized fragment		\n"
	    "	ldr       %[tmp], [%[src]], #4			\n"
	    "	str       %[tmp], [%[dst]], #4			\n"
	    "6: @ one halfword					\n"
	    "	tst       %[count], #1				\n"
	    "	beq 7f    @ skip oversized fragment		\n"
	    "	ldrh      %[tmp], [%[src]]			\n"
	    "	strh      %[tmp], [%[dst]]			\n"
	    "7: @ end						\n"

	    /* Clobbered input registers marked as input/outputs */
	    : [dst] "+r" (dst_ptr), [src] "+r" (src_ptr),
	      [count] "+r" (count), [tmp] "+r" (tmp)

	      /* Unclobbered input */
	    : [src_stride] "r" (src_stride)

	      /* Clobbered vector registers */
	    : "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
	      "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31", "cc", "memory"
	    );

	src_line += src_stride;
	dst_line += dst_stride;
    }
}

#endif /* USE_GCC_INLINE_ASM */

static void
neon_composite_src_24_16 (pixman_implementation_t * impl,
                          pixman_op_t               op,
                          pixman_image_t *          src_image,
                          pixman_image_t *          mask_image,
                          pixman_image_t *          dst_image,
                          int32_t                   src_x,
                          int32_t                   src_y,
                          int32_t                   mask_x,
                          int32_t                   mask_y,
                          int32_t                   dest_x,
                          int32_t                   dest_y,
                          int32_t                   width,
                          int32_t                   height)
{
    uint16_t    *dst_line;
    uint32_t    *src_line;
    uint32_t dst_stride, src_stride;

    if (!width || !height)
	return;

    /* We simply copy pixels from one place to another,
     * assuming that the source's alpha is opaque.
     */
    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, uint32_t, src_stride, src_line, 1);
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint16_t, dst_stride, dst_line, 1);

    /* Preload the first input scanline */
    {
	uint8_t *src_ptr = (uint8_t*) src_line;
	uint32_t count = (width + 15) / 16;

#ifdef USE_GCC_INLINE_ASM
	asm volatile (
	    "0: @ loop						\n"
	    "	subs    %[count], %[count], #1			\n"
	    "	pld     [%[src]]				\n"
	    "	add     %[src], %[src], #64			\n"
	    "	bgt 0b						\n"

	    /* Clobbered input registers marked as input/outputs */
	    : [src] "+r" (src_ptr), [count] "+r" (count)
	    :     /* no unclobbered inputs */
	    : "cc"
	    );
#else
	do
	{
	    __pld (src_ptr);
	    src_ptr += 64;
	}
	while (--count);
#endif
    }

    while (height--)
    {
	uint16_t *dst_ptr = dst_line;
	uint32_t *src_ptr = src_line;
	uint32_t count = width;
	const uint32_t rb_mask = 0x1F;
	const uint32_t g_mask = 0x3F;

	/* If you're going to complain about a goto, take a long hard look
	 * at the massive blocks of assembler this skips over.  ;-)
	 */
	if (count < 8)
	    goto small_stuff;

#ifdef USE_GCC_INLINE_ASM

	/* This is not as aggressive as the RGB565-source case.
	 * Generally the source is in cached RAM when the formats are
	 * different, so we use preload.
	 * 
	 * We don't need to blend, so we are not reading from the
	 * uncached framebuffer.
	 */
	asm volatile (
	    "	cmp       %[count], #16				\n"
	    "	blt 1f    @ skip oversized fragments		\n"
	    "0: @ start with sixteen pixels at a time		\n"
	    "	sub       %[count], %[count], #16		\n"
	    "	pld      [%[src], %[src_stride], lsl #2]        @ preload from next scanline			\n"
	    "	vld4.8    {d0, d1, d2, d3}, [%[src]]!		@ d3 is alpha and ignored, d2-0 are rgb.	\n"
	    "	vld4.8    {d4, d5, d6, d7}, [%[src]]!		@ d7 is alpha and ignored, d6-4 are rgb.	\n"
	    "	vshll.u8  q8, d2, #8				@ expand first red for repacking		\n"
	    "	vshll.u8  q10, d1, #8				@ expand first green for repacking		\n"
	    "	vshll.u8  q11, d0, #8				@ expand first blue for repacking		\n"
	    "	vshll.u8  q9, d6, #8				@ expand second red for repacking		\n"
	    "	vsri.u16  q8, q10, #5				@ insert first green after red			\n"
	    "	vshll.u8  q10, d5, #8				@ expand second green for repacking		\n"
	    "	vsri.u16  q8, q11, #11				@ insert first blue after green			\n"
	    "	vshll.u8  q11, d4, #8				@ expand second blue for repacking		\n"
	    "	vsri.u16  q9, q10, #5				@ insert second green after red			\n"
	    "	vsri.u16  q9, q11, #11				@ insert second blue after green		\n"
	    "	cmp       %[count], #16				\n"
	    "	vst1.16   {d16, d17, d18, d19}, [%[dst]]!          @ store 16 pixels				\n"
	    "	bge 0b						\n"
	    "1: @ end of main loop				\n"
	    "	cmp       %[count], #8				@ can we still do an 8-pixel block?		\n"
	    "	blt 2f						\n"
	    "	sub       %[count], %[count], #8		\n"
	    "	pld      [%[src], %[src_stride], lsl #2]        @ preload from next scanline			\n"
	    "	vld4.8    {d0, d1, d2, d3}, [%[src]]!		@ d3 is alpha and ignored, d2-0 are rgb.	\n"
	    "	vshll.u8  q8, d2, #8				@ expand first red for repacking		\n"
	    "	vshll.u8  q10, d1, #8				@ expand first green for repacking		\n"
	    "	vshll.u8  q11, d0, #8				@ expand first blue for repacking		\n"
	    "	vsri.u16  q8, q10, #5				@ insert first green after red			\n"
	    "	vsri.u16  q8, q11, #11				@ insert first blue after green			\n"
	    "	vst1.16   {d16, d17}, [%[dst]]!          @ store 8 pixels				\n"
	    "2: @ end						\n"

	    /* Clobbered input and working registers marked as input/outputs */
	    : [dst] "+r" (dst_ptr), [src] "+r" (src_ptr), [count] "+r" (count)

	      /* Unclobbered input */
	    : [src_stride] "r" (src_stride)

	      /* Clobbered vector registers */

	      /* NB: these are the quad aliases of the
	       * double registers used in the asm
	       */
	    : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d16", "d17",
	      "d18", "d19", "d20", "d21", "d22", "d23", "cc", "memory"
	    );
#else
	/* A copy of the above code, in intrinsics-form. */
	while (count >= 16)
	{
	    uint8x8x4_t pixel_set_a, pixel_set_b;
	    uint16x8_t red_a, green_a, blue_a;
	    uint16x8_t red_b, green_b, blue_b;
	    uint16x8_t dest_pixels_a, dest_pixels_b;

	    count -= 16;
	    __pld (src_ptr + src_stride);
	    pixel_set_a = vld4_u8 ((uint8_t*)(src_ptr));
	    pixel_set_b = vld4_u8 ((uint8_t*)(src_ptr + 8));
	    src_ptr += 16;

	    red_a   = vshll_n_u8 (pixel_set_a.val[2], 8);
	    green_a = vshll_n_u8 (pixel_set_a.val[1], 8);
	    blue_a  = vshll_n_u8 (pixel_set_a.val[0], 8);
	    
	    red_b   = vshll_n_u8 (pixel_set_b.val[2], 8);
	    green_b = vshll_n_u8 (pixel_set_b.val[1], 8);
	    blue_b  = vshll_n_u8 (pixel_set_b.val[0], 8);
	    
	    dest_pixels_a = vsriq_n_u16 (red_a, green_a, 5);
	    dest_pixels_b = vsriq_n_u16 (red_b, green_b, 5);
	    
	    dest_pixels_a = vsriq_n_u16 (dest_pixels_a, blue_a, 11);
	    dest_pixels_b = vsriq_n_u16 (dest_pixels_b, blue_b, 11);

	    /* There doesn't seem to be an intrinsic for the
	     * double-quadword variant
	     */
	    vst1q_u16 (dst_ptr, dest_pixels_a);
	    vst1q_u16 (dst_ptr + 8, dest_pixels_b);
	    dst_ptr += 16;
	}

	/* 8-pixel loop */
	if (count >= 8)
	{
	    uint8x8x4_t pixel_set_a;
	    uint16x8_t red_a, green_a, blue_a;
	    uint16x8_t dest_pixels_a;

	    __pld (src_ptr + src_stride);
	    count -= 8;
	    pixel_set_a = vld4_u8 ((uint8_t*)(src_ptr));
	    src_ptr += 8;

	    red_a   = vshll_n_u8 (pixel_set_a.val[2], 8);
	    green_a = vshll_n_u8 (pixel_set_a.val[1], 8);
	    blue_a  = vshll_n_u8 (pixel_set_a.val[0], 8);

	    dest_pixels_a = vsriq_n_u16 (red_a, green_a, 5);
	    dest_pixels_a = vsriq_n_u16 (dest_pixels_a, blue_a, 11);

	    vst1q_u16 (dst_ptr, dest_pixels_a);
	    dst_ptr += 8;
	}

#endif  /* USE_GCC_INLINE_ASM */

    small_stuff:
	if (count)
	    __pld (src_ptr + src_stride);

	while (count >= 2)
	{
	    uint32_t src_pixel_a = *src_ptr++;
	    uint32_t src_pixel_b = *src_ptr++;

	    /* ARM is really good at shift-then-ALU ops. */
	    /* This should be a total of six shift-ANDs and five shift-ORs. */
	    uint32_t dst_pixels_a;
	    uint32_t dst_pixels_b;

	    dst_pixels_a  = ((src_pixel_a >>  3) & rb_mask);
	    dst_pixels_a |= ((src_pixel_a >> 10) &  g_mask) << 5;
	    dst_pixels_a |= ((src_pixel_a >> 19) & rb_mask) << 11;

	    dst_pixels_b  = ((src_pixel_b >>  3) & rb_mask);
	    dst_pixels_b |= ((src_pixel_b >> 10) &  g_mask) << 5;
	    dst_pixels_b |= ((src_pixel_b >> 19) & rb_mask) << 11;

	    /* little-endian mode only */
	    *((uint32_t*) dst_ptr) = dst_pixels_a | (dst_pixels_b << 16);
	    dst_ptr += 2;
	    count -= 2;
	}

	if (count)
	{
	    uint32_t src_pixel = *src_ptr++;

	    /* ARM is really good at shift-then-ALU ops.
	     * This block should end up as three shift-ANDs
	     * and two shift-ORs.
	     */
	    uint32_t tmp_blue  = (src_pixel >>  3) & rb_mask;
	    uint32_t tmp_green = (src_pixel >> 10) & g_mask;
	    uint32_t tmp_red   = (src_pixel >> 19) & rb_mask;
	    uint16_t dst_pixel = (tmp_red << 11) | (tmp_green << 5) | tmp_blue;

	    *dst_ptr++ = dst_pixel;
	    count--;
	}

	src_line += src_stride;
	dst_line += dst_stride;
    }
}

static pixman_bool_t
pixman_fill_neon (uint32_t *bits,
                  int       stride,
                  int       bpp,
                  int       x,
                  int       y,
                  int       width,
                  int       height,
                  uint32_t  _xor)
{
    uint32_t byte_stride, color;
    char *dst;

    /* stride is always multiple of 32bit units in pixman */
    byte_stride = stride * sizeof(uint32_t);

    switch (bpp)
    {
    case 8:
	dst = ((char *) bits) + y * byte_stride + x;
	_xor &= 0xff;
	color = _xor << 24 | _xor << 16 | _xor << 8 | _xor;
	break;

    case 16:
	dst = ((char *) bits) + y * byte_stride + x * 2;
	_xor &= 0xffff;
	color = _xor << 16 | _xor;
	width *= 2;         /* width to bytes */
	break;

    case 32:
	dst = ((char *) bits) + y * byte_stride + x * 4;
	color = _xor;
	width *= 4;         /* width to bytes */
	break;

    default:
	return FALSE;
    }

#ifdef USE_GCC_INLINE_ASM
    if (width < 16)
    {
	/* We have a special case for such small widths that don't allow
	 * us to use wide 128-bit stores anyway. We don't waste time
	 * trying to align writes, since there are only very few of them anyway
	 */
	asm volatile (
	    "cmp		%[height], #0\n"/* Check if empty fill */
	    "beq		3f\n"
	    "vdup.32	d0, %[color]\n"/* Fill the color to neon req */

	    /* Check if we have a such width that can easily be handled by single
	     * operation for each scanline. This significantly reduces the number
	     * of test/branch instructions for each scanline
	     */
	    "cmp		%[width], #8\n"
	    "beq		4f\n"
	    "cmp		%[width], #4\n"
	    "beq		5f\n"
	    "cmp		%[width], #2\n"
	    "beq		6f\n"

	    /* Loop starts here for each scanline */
	    "1:\n"
	    "mov		r4, %[dst]\n" /* Starting address of the current line */
	    "tst		%[width], #8\n"
	    "beq		2f\n"
	    "vst1.8		{d0}, [r4]!\n"
	    "2:\n"
	    "tst		%[width], #4\n"
	    "beq		2f\n"
	    "str		%[color], [r4], #4\n"
	    "2:\n"
	    "tst		%[width], #2\n"
	    "beq		2f\n"
	    "strh		%[color], [r4], #2\n"
	    "2:\n"
	    "tst		%[width], #1\n"
	    "beq		2f\n"
	    "strb		%[color], [r4], #1\n"
	    "2:\n"

	    "subs		%[height], %[height], #1\n"
	    "add		%[dst], %[dst], %[byte_stride]\n"
	    "bne		1b\n"
	    "b		3f\n"

	    /* Special fillers for those widths that we can do with single operation */
	    "4:\n"
	    "subs		%[height], %[height], #1\n"
	    "vst1.8		{d0}, [%[dst]]\n"
	    "add		%[dst], %[dst], %[byte_stride]\n"
	    "bne		4b\n"
	    "b		3f\n"

	    "5:\n"
	    "subs		%[height], %[height], #1\n"
	    "str		%[color], [%[dst]]\n"
	    "add		%[dst], %[dst], %[byte_stride]\n"
	    "bne		5b\n"
	    "b		3f\n"

	    "6:\n"
	    "subs		%[height], %[height], #1\n"
	    "strh		%[color], [%[dst]]\n"
	    "add		%[dst], %[dst], %[byte_stride]\n"
	    "bne		6b\n"

	    "3:\n"
	    : [height] "+r" (height), [dst] "+r" (dst)
	    : [color] "r" (color), [width] "r" (width),
	      [byte_stride] "r" (byte_stride)
	    : "memory", "cc", "d0", "r4");
    }
    else
    {
	asm volatile (
	    "cmp		%[height], #0\n"/* Check if empty fill */
	    "beq		5f\n"
	    "vdup.32	q0, %[color]\n"/* Fill the color to neon req */

	    /* Loop starts here for each scanline */
	    "1:\n"
	    "mov		r4, %[dst]\n"/* Starting address of the current line */
	    "mov		r5, %[width]\n"/* We're going to write this many bytes */
	    "ands		r6, r4, #15\n"/* Are we at the 128-bit aligned address? */
	    "beq		2f\n"/* Jump to the best case */

	    /* We're not 128-bit aligned: However, we know that we can get to the
	       next aligned location, since the fill is at least 16 bytes wide */
	    "rsb                r6, r6, #16\n" /* We would need to go forward this much */
	    "sub		r5, r5, r6\n"/* Update bytes left */
	    "tst		r6, #1\n"
	    "beq		6f\n"
	    "vst1.8		{d0[0]}, [r4]!\n"/* Store byte, now we are word aligned */
	    "6:\n"
	    "tst		r6, #2\n"
	    "beq		6f\n"
	    "vst1.16	{d0[0]}, [r4, :16]!\n"/* Store half word, now we are 16-bit aligned */
	    "6:\n"
	    "tst		r6, #4\n"
	    "beq		6f\n"
	    "vst1.32	{d0[0]}, [r4, :32]!\n"/* Store word, now we're 32-bit aligned */
	    "6:\n"
	    "tst		r6, #8\n"
	    "beq		2f\n"
	    "vst1.64	{d0}, [r4, :64]!\n"/* Store qword now we're 64-bit aligned */

	    /* The good case: We're 128-bit aligned for this scanline */
	    "2:\n"
	    "and		r6, r5, #15\n"/* Number of tailing bytes */
	    "cmp		r5, r6\n"/* Do we have at least one qword to write? */
	    "beq		6f\n"/* No, we just write the tail */
	    "lsr		r5, r5, #4\n"/* This many full qwords to write */

	    /* The main block: Do 128-bit aligned writes */
	    "3:\n"
	    "subs		r5, r5, #1\n"
	    "vst1.64	{d0, d1}, [r4, :128]!\n"
	    "bne		3b\n"

	    /* Handle the tailing bytes: Do 64, 32, 16 and 8-bit aligned writes as needed.
	       We know that we're currently at 128-bit aligned address, so we can just
	       pick the biggest operations that the remaining write width allows */
	    "6:\n"
	    "cmp		r6, #0\n"
	    "beq		4f\n"
	    "tst		r6, #8\n"
	    "beq		6f\n"
	    "vst1.64	{d0}, [r4, :64]!\n"
	    "6:\n"
	    "tst		r6, #4\n"
	    "beq		6f\n"
	    "vst1.32	{d0[0]}, [r4, :32]!\n"
	    "6:\n"
	    "tst		r6, #2\n"
	    "beq		6f\n"
	    "vst1.16	{d0[0]}, [r4, :16]!\n"
	    "6:\n"
	    "tst		r6, #1\n"
	    "beq		4f\n"
	    "vst1.8		{d0[0]}, [r4]!\n"
	    "4:\n"

	    /* Handle the next scanline */
	    "subs		%[height], %[height], #1\n"
	    "add		%[dst], %[dst], %[byte_stride]\n"
	    "bne		1b\n"
	    "5:\n"
	    : [height] "+r" (height), [dst] "+r" (dst)
	    : [color] "r" (color), [width] "r" (width),
	      [byte_stride] "r" (byte_stride)
	    : "memory", "cc", "d0", "d1", "r4", "r5", "r6");
    }
    return TRUE;

#else

    /* TODO: intrinsic version for armcc */
    return FALSE;

#endif
}

/* TODO: is there a more generic way of doing this being introduced? */
#define NEON_SCANLINE_BUFFER_PIXELS (1024)

static inline void
neon_quadword_copy (void *   dst,
		    void *   src,
		    uint32_t count,         /* of quadwords */
		    uint32_t trailer_count  /* of bytes */)
{
    uint8_t *t_dst = dst, *t_src = src;

    /* Uses aligned multi-register loads to maximise read bandwidth
     * on uncached memory such as framebuffers
     * The accesses do not have the aligned qualifiers, so that the copy
     * may convert between aligned-uncached and unaligned-cached memory.
     * It is assumed that the CPU can infer alignedness from the address.
     */

#ifdef USE_GCC_INLINE_ASM

    asm volatile (
        "	cmp       %[count], #8				\n"
        "	blt 1f    @ skip oversized fragments		\n"
        "0: @ start with eight quadwords at a time		\n"
        "	sub       %[count], %[count], #8		\n"
        "	vld1.8    {d16, d17, d18, d19}, [%[src]]!		\n"
        "	vld1.8    {d20, d21, d22, d23}, [%[src]]!		\n"
        "	vld1.8    {d24, d25, d26, d27}, [%[src]]!		\n"
        "	vld1.8    {d28, d29, d30, d31}, [%[src]]!		\n"
        "	cmp       %[count], #8				\n"
        "	vst1.8    {d16, d17, d18, d19}, [%[dst]]!		\n"
        "	vst1.8    {d20, d21, d22, d23}, [%[dst]]!		\n"
        "	vst1.8    {d24, d25, d26, d27}, [%[dst]]!		\n"
        "	vst1.8    {d28, d29, d30, d31}, [%[dst]]!		\n"
        "	bge 0b						\n"
        "1: @ four quadwords					\n"
        "	tst       %[count], #4				\n"
        "	beq 2f    @ skip oversized fragment		\n"
        "	vld1.8    {d16, d17, d18, d19}, [%[src]]!		\n"
        "	vld1.8    {d20, d21, d22, d23}, [%[src]]!		\n"
        "	vst1.8    {d16, d17, d18, d19}, [%[dst]]!		\n"
        "	vst1.8    {d20, d21, d22, d23}, [%[dst]]!		\n"
        "2: @ two quadwords					\n"
        "	tst       %[count], #2				\n"
        "	beq 3f    @ skip oversized fragment		\n"
        "	vld1.8    {d16, d17, d18, d19}, [%[src]]!		\n"
        "	vst1.8    {d16, d17, d18, d19}, [%[dst]]!		\n"
        "3: @ one quadword					\n"
        "	tst       %[count], #1				\n"
        "	beq 4f    @ skip oversized fragment		\n"
        "	vld1.8    {d16, d17}, [%[src]]!			\n"
        "	vst1.8    {d16, d17}, [%[dst]]!			\n"
        "4: @ end						\n"

        /* Clobbered input registers marked as input/outputs */
	: [dst] "+r" (t_dst), [src] "+r" (t_src), [count] "+r" (count)

	  /* No unclobbered inputs */
	:

        /* Clobbered vector registers */
	: "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25",
	  "d26", "d27", "d28", "d29", "d30", "d31", "cc", "memory");

#else

    while (count >= 8)
    {
	uint8x16x4_t t1 = vld4q_u8 (t_src);
	uint8x16x4_t t2 = vld4q_u8 (t_src + sizeof(uint8x16x4_t));
	
	t_src += sizeof(uint8x16x4_t) * 2;
	vst4q_u8 (t_dst, t1);
	vst4q_u8 (t_dst + sizeof(uint8x16x4_t), t2);
	t_dst += sizeof(uint8x16x4_t) * 2;
	count -= 8;
    }

    if (count & 4)
    {
	uint8x16x4_t t1 = vld4q_u8 (t_src);
	
	t_src += sizeof(uint8x16x4_t);
	vst4q_u8 (t_dst, t1);
	t_dst += sizeof(uint8x16x4_t);
    }

    if (count & 2)
    {
	uint8x8x4_t t1 = vld4_u8 (t_src);
	
	t_src += sizeof(uint8x8x4_t);
	vst4_u8 (t_dst, t1);
	t_dst += sizeof(uint8x8x4_t);
    }

    if (count & 1)
    {
	uint8x16_t t1 = vld1q_u8 (t_src);
	
	t_src += sizeof(uint8x16_t);
	vst1q_u8 (t_dst, t1);
	t_dst += sizeof(uint8x16_t);
    }

#endif  /* !USE_GCC_INLINE_ASM */

    if (trailer_count)
    {
	if (trailer_count & 8)
	{
	    uint8x8_t t1 = vld1_u8 (t_src);
	    
	    t_src += sizeof(uint8x8_t);
	    vst1_u8 (t_dst, t1);
	    t_dst += sizeof(uint8x8_t);
	}

	if (trailer_count & 4)
	{
	    *((uint32_t*) t_dst) = *((uint32_t*) t_src);
	    
	    t_dst += 4;
	    t_src += 4;
	}

	if (trailer_count & 2)
	{
	    *((uint16_t*) t_dst) = *((uint16_t*) t_src);
	    
	    t_dst += 2;
	    t_src += 2;
	}

	if (trailer_count & 1)
	{
	    *t_dst++ = *t_src++;
	}
    }
}

static inline void
solid_over_565_8_pix_neon (uint32_t  glyph_colour,
                           uint16_t *dest,
                           uint8_t * in_mask,
                           uint32_t  dest_stride,    /* bytes, not elements */
                           uint32_t  mask_stride,
                           uint32_t  count           /* 8-pixel groups */)
{
    /* Inner loop of glyph blitter (solid colour, alpha mask) */

#ifdef USE_GCC_INLINE_ASM

    asm volatile (
        "	vld4.8 {d20[], d21[], d22[], d23[]}, [%[glyph_colour]]  @ splat solid colour components	\n"
        "0:	@ loop																				\n"
        "	vld1.16   {d0, d1}, [%[dest]]         @ load first pixels from framebuffer			\n"
        "	vld1.8    {d17}, [%[in_mask]]         @ load alpha mask of glyph						\n"
        "	vmull.u8  q9, d17, d23               @ apply glyph colour alpha to mask				\n"
        "	vshrn.u16 d17, q9, #8                @ reformat it to match original mask			\n"
        "	vmvn      d18, d17                   @ we need the inverse mask for the background	\n"
        "	vsli.u16  q3, q0, #5                 @ duplicate framebuffer blue bits				\n"
        "	vshrn.u16 d2, q0, #8                 @ unpack red from framebuffer pixels			\n"
        "	vshrn.u16 d4, q0, #3                 @ unpack green									\n"
        "	vsri.u8   d2, d2, #5                 @ duplicate red bits (extend 5 to 8)			\n"
        "	vshrn.u16 d6, q3, #2                 @ unpack extended blue (truncate 10 to 8)		\n"
        "	vsri.u8   d4, d4, #6                 @ duplicate green bits (extend 6 to 8)			\n"
        "	vmull.u8  q1, d2, d18                @ apply inverse mask to background red...		\n"
        "	vmull.u8  q2, d4, d18                @ ...green...									\n"
        "	vmull.u8  q3, d6, d18                @ ...blue										\n"
        "	subs      %[count], %[count], #1     @ decrement/test loop counter					\n"
        "	vmlal.u8  q1, d17, d22               @ add masked foreground red...					\n"
        "	vmlal.u8  q2, d17, d21               @ ...green...									\n"
        "	vmlal.u8  q3, d17, d20               @ ...blue										\n"
        "	add %[in_mask], %[in_mask], %[mask_stride] @ advance mask pointer, while we wait		\n"
        "	vsri.16   q1, q2, #5                 @ pack green behind red						\n"
        "	vsri.16   q1, q3, #11                @ pack blue into pixels						\n"
        "	vst1.16   {d2, d3}, [%[dest]]         @ store composited pixels						\n"
        "	add %[dest], %[dest], %[dest_stride]  @ advance framebuffer pointer					\n"
        "	bne 0b                               @ next please									\n"

	/* Clobbered registers marked as input/outputs */
	: [dest] "+r" (dest), [in_mask] "+r" (in_mask), [count] "+r" (count)
	  
	  /* Inputs */
	: [dest_stride] "r" (dest_stride), [mask_stride] "r" (mask_stride), [glyph_colour] "r" (&glyph_colour)

	  /* Clobbers, including the inputs we modify, and potentially lots of memory */
	: "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d17", "d18", "d19",
	  "d20", "d21", "d22", "d23", "d24", "d25", "cc", "memory"
        );

#else

    uint8x8x4_t solid_colour = vld4_dup_u8 ((uint8_t*) &glyph_colour);

    while (count--)
    {
	uint16x8_t pixels = vld1q_u16 (dest);
	uint8x8_t mask = vshrn_n_u16 (vmull_u8 (solid_colour.val[3], vld1_u8 (in_mask)), 8);
	uint8x8_t mask_image = vmvn_u8 (mask);

	uint8x8_t t_red   = vshrn_n_u16 (pixels, 8);
	uint8x8_t t_green = vshrn_n_u16 (pixels, 3);
	uint8x8_t t_blue  = vshrn_n_u16 (vsli_n_u8 (pixels, pixels, 5), 2);

	uint16x8_t s_red   = vmull_u8 (vsri_n_u8 (t_red, t_red, 5), mask_image);
	uint16x8_t s_green = vmull_u8 (vsri_n_u8 (t_green, t_green, 6), mask_image);
	uint16x8_t s_blue  = vmull_u8 (t_blue, mask_image);

	s_red   = vmlal (s_red, mask, solid_colour.val[2]);
	s_green = vmlal (s_green, mask, solid_colour.val[1]);
	s_blue  = vmlal (s_blue, mask, solid_colour.val[0]);

	pixels = vsri_n_u16 (s_red, s_green, 5);
	pixels = vsri_n_u16 (pixels, s_blue, 11);
	vst1q_u16 (dest, pixels);

	dest += dest_stride;
	mask += mask_stride;
    }

#endif
}

#if 0 /* this is broken currently */
static void
neon_composite_over_n_8_0565 (pixman_implementation_t * impl,
                              pixman_op_t               op,
                              pixman_image_t *          src_image,
                              pixman_image_t *          mask_image,
                              pixman_image_t *          dst_image,
                              int32_t                   src_x,
                              int32_t                   src_y,
                              int32_t                   mask_x,
                              int32_t                   mask_y,
                              int32_t                   dest_x,
                              int32_t                   dest_y,
                              int32_t                   width,
                              int32_t                   height)
{
    uint32_t  src, srca;
    uint16_t *dst_line, *aligned_line;
    uint8_t  *mask_line;
    uint32_t  dst_stride, mask_stride;
    uint32_t  kernel_count, copy_count, copy_tail;
    uint8_t   kernel_offset, copy_offset;

    src = _pixman_image_get_solid (src_image, dst_image->bits.format);

    /* bail out if fully transparent or degenerate */
    srca = src >> 24;
    if (src == 0)
	return;

    if (width == 0 || height == 0)
	return;

    if (width > NEON_SCANLINE_BUFFER_PIXELS)
    {
	/* split the blit, so we can use a fixed-size scanline buffer
	 * TODO: there must be a more elegant way of doing this.
	 */
	int x;
	for (x = 0; x < width; x += NEON_SCANLINE_BUFFER_PIXELS)
	{
	    neon_composite_over_n_8_0565 (
		impl, op,
		src_image, mask_image, dst_image,
		src_x + x, src_y, mask_x + x, mask_y, dest_x + x, dest_y,
		(x + NEON_SCANLINE_BUFFER_PIXELS > width) ? width - x : NEON_SCANLINE_BUFFER_PIXELS, height);
	}

	return;
    }
    
    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint16_t, dst_stride, dst_line, 1);
    PIXMAN_IMAGE_GET_LINE (mask_image, mask_x, mask_y, uint8_t, mask_stride, mask_line, 1);

    /* keep within minimum number of aligned quadwords on width
     * while also keeping the minimum number of columns to process
     */
    {
	unsigned long aligned_left = (unsigned long)(dst_line) & ~0xF;
	unsigned long aligned_right = (((unsigned long)(dst_line + width)) + 0xF) & ~0xF;
	unsigned long ceiling_length = (((unsigned long) width) * sizeof(*dst_line) + 0xF) & ~0xF;

	/* the fast copy should be quadword aligned */
	copy_offset = dst_line - ((uint16_t*) aligned_left);
	aligned_line = dst_line - copy_offset;
	copy_count = (uint32_t) ((aligned_right - aligned_left) >> 4);
	copy_tail = 0;

	if (aligned_right - aligned_left > ceiling_length)
	{
	    /* unaligned routine is tightest */
	    kernel_count = (uint32_t) (ceiling_length >> 4);
	    kernel_offset = copy_offset;
	}
	else
	{
	    /* aligned routine is equally tight, so it is safer to align */
	    kernel_count = copy_count;
	    kernel_offset = 0;
	}

	/* We should avoid reading beyond scanline ends for safety */
	if (aligned_line < (dst_line - dest_x) ||
	    (aligned_line + (copy_count * 16 / sizeof(*dst_line))) > ((dst_line - dest_x) + dst_image->bits.width))
	{
	    /* switch to precise read */
	    copy_offset = kernel_offset = 0;
	    aligned_line = dst_line;
	    kernel_count = (uint32_t) (ceiling_length >> 4);
	    copy_count = (width * sizeof(*dst_line)) >> 4;
	    copy_tail = (width * sizeof(*dst_line)) & 0xF;
	}
    }

    {
	uint16_t scan_line[NEON_SCANLINE_BUFFER_PIXELS + 8];         /* deliberately not initialised */
	uint8_t glyph_line[NEON_SCANLINE_BUFFER_PIXELS + 8];
	int y = height;

	/* row-major order */
	/* left edge, middle block, right edge */
	for ( ; y--; mask_line += mask_stride, aligned_line += dst_stride, dst_line += dst_stride)
	{
	    /* We don't want to overrun the edges of the glyph,
	     * so realign the edge data into known buffers
	     */
	    neon_quadword_copy (glyph_line + copy_offset, mask_line, width >> 4, width & 0xF);

	    /* Uncached framebuffer access is really, really slow
	     * if we do it piecemeal. It should be much faster if we
	     * grab it all at once. One scanline should easily fit in
	     * L1 cache, so this should not waste RAM bandwidth.
	     */
	    neon_quadword_copy (scan_line, aligned_line, copy_count, copy_tail);

	    /* Apply the actual filter */
	    solid_over_565_8_pix_neon (
		src, scan_line + kernel_offset,
		glyph_line + kernel_offset, 8 * sizeof(*dst_line),
		8, kernel_count);

	    /* Copy the modified scanline back */
	    neon_quadword_copy (dst_line, scan_line + copy_offset,
				width >> 3, (width & 7) * 2);
	}
    }
}
#endif

#ifdef USE_GCC_INLINE_ASM

static inline void
plain_over_565_8_pix_neon (uint32_t  colour,
			   uint16_t *dest,
			   uint32_t  dest_stride,     /* bytes, not elements */
			   uint32_t  count            /* 8-pixel groups */)
{
    /* Inner loop for plain translucent rects
     * (solid colour without alpha mask)
     */
    asm volatile (
        "	vld4.8   {d20[], d21[], d22[], d23[]}, [%[colour]]  @ solid colour load/splat \n"
        "	vmull.u8  q12, d23, d22              @ premultiply alpha red   \n"
        "	vmull.u8  q13, d23, d21              @ premultiply alpha green \n"
        "	vmull.u8  q14, d23, d20              @ premultiply alpha blue  \n"
        "	vmvn      d18, d23                   @ inverse alpha for background \n"
        "0:	@ loop\n"
        "	vld1.16   {d0, d1}, [%[dest]]         @ load first pixels from framebuffer	\n"
        "	vshrn.u16 d2, q0, #8                 @ unpack red from framebuffer pixels	\n"
        "	vshrn.u16 d4, q0, #3                 @ unpack green				\n"
        "	vsli.u16  q3, q0, #5                 @ duplicate framebuffer blue bits		\n"
        "	vsri.u8   d2, d2, #5                 @ duplicate red bits (extend 5 to 8)	\n"
        "	vsri.u8   d4, d4, #6                 @ duplicate green bits (extend 6 to 8)	\n"
        "	vshrn.u16 d6, q3, #2                 @ unpack extended blue (truncate 10 to 8)	\n"
        "	vmov      q0, q12                    @ retrieve foreground red   \n"
        "	vmlal.u8  q0, d2, d18                @ blend red - my kingdom for a four-operand MLA \n"
        "	vmov      q1, q13                    @ retrieve foreground green \n"
        "	vmlal.u8  q1, d4, d18                @ blend green               \n"
        "	vmov      q2, q14                    @ retrieve foreground blue  \n"
        "	vmlal.u8  q2, d6, d18                @ blend blue                \n"
        "	subs      %[count], %[count], #1     @ decrement/test loop counter		\n"
        "	vsri.16   q0, q1, #5                 @ pack green behind red			\n"
        "	vsri.16   q0, q2, #11                @ pack blue into pixels			\n"
        "	vst1.16   {d0, d1}, [%[dest]]         @ store composited pixels			\n"
        "	add %[dest], %[dest], %[dest_stride]  @ advance framebuffer pointer		\n"
        "	bne 0b                               @ next please				\n"

        /* Clobbered registers marked as input/outputs */
	: [dest] "+r" (dest), [count] "+r" (count)

	  /* Inputs */
	: [dest_stride] "r" (dest_stride), [colour] "r" (&colour)

	  /* Clobbers, including the inputs we modify, and
	   * potentially lots of memory
	   */
	: "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d18", "d19",
	  "d20", "d21", "d22", "d23", "d24", "d25", "d26", "d27", "d28", "d29",
	  "cc", "memory"
        );
}

static void
neon_composite_over_n_0565 (pixman_implementation_t * impl,
                            pixman_op_t               op,
                            pixman_image_t *          src_image,
                            pixman_image_t *          mask_image,
                            pixman_image_t *          dst_image,
                            int32_t                   src_x,
                            int32_t                   src_y,
                            int32_t                   mask_x,
                            int32_t                   mask_y,
                            int32_t                   dest_x,
                            int32_t                   dest_y,
                            int32_t                   width,
                            int32_t                   height)
{
    uint32_t src, srca;
    uint16_t    *dst_line, *aligned_line;
    uint32_t dst_stride;
    uint32_t kernel_count, copy_count, copy_tail;
    uint8_t kernel_offset, copy_offset;

    src = _pixman_image_get_solid (src_image, dst_image->bits.format);

    /* bail out if fully transparent */
    srca = src >> 24;
    if (src == 0)
	return;
    
    if (width == 0 || height == 0)
	return;

    if (width > NEON_SCANLINE_BUFFER_PIXELS)
    {
	/* split the blit, so we can use a fixed-size scanline buffer *
	 * TODO: there must be a more elegant way of doing this.
	 */
	int x;
	
	for (x = 0; x < width; x += NEON_SCANLINE_BUFFER_PIXELS)
	{
	    neon_composite_over_n_0565 (
		impl, op,
		src_image, mask_image, dst_image,
		src_x + x, src_y, mask_x + x, mask_y, dest_x + x, dest_y,
		(x + NEON_SCANLINE_BUFFER_PIXELS > width) ? width - x : NEON_SCANLINE_BUFFER_PIXELS, height);
	}
	return;
    }

    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint16_t, dst_stride, dst_line, 1);

    /* keep within minimum number of aligned quadwords on width
     * while also keeping the minimum number of columns to process
     */
    {
	unsigned long aligned_left = (unsigned long)(dst_line) & ~0xF;
	unsigned long aligned_right = (((unsigned long)(dst_line + width)) + 0xF) & ~0xF;
	unsigned long ceiling_length = (((unsigned long) width) * sizeof(*dst_line) + 0xF) & ~0xF;

	/* the fast copy should be quadword aligned */
	copy_offset = dst_line - ((uint16_t*) aligned_left);
	aligned_line = dst_line - copy_offset;
	copy_count = (uint32_t) ((aligned_right - aligned_left) >> 4);
	copy_tail = 0;

	if (aligned_right - aligned_left > ceiling_length)
	{
	    /* unaligned routine is tightest */
	    kernel_count = (uint32_t) (ceiling_length >> 4);
	    kernel_offset = copy_offset;
	}
	else
	{
	    /* aligned routine is equally tight, so it is safer to align */
	    kernel_count = copy_count;
	    kernel_offset = 0;
	}

	/* We should avoid reading beyond scanline ends for safety */
	if (aligned_line < (dst_line - dest_x) ||
	    (aligned_line + (copy_count * 16 / sizeof(*dst_line))) > ((dst_line - dest_x) + dst_image->bits.width))
	{
	    /* switch to precise read */
	    copy_offset = kernel_offset = 0;
	    aligned_line = dst_line;
	    kernel_count = (uint32_t) (ceiling_length >> 4);
	    copy_count = (width * sizeof(*dst_line)) >> 4;
	    copy_tail = (width * sizeof(*dst_line)) & 0xF;
	}
    }

    {
	uint16_t scan_line[NEON_SCANLINE_BUFFER_PIXELS + 8];  /* deliberately not initialised */

	/* row-major order */
	/* left edge, middle block, right edge */
	for ( ; height--; aligned_line += dst_stride, dst_line += dst_stride)
	{
	    /* Uncached framebuffer access is really, really slow if we do it piecemeal.
	     * It should be much faster if we grab it all at once.
	     * One scanline should easily fit in L1 cache, so this should
	     * not waste RAM bandwidth.
	     */
	    neon_quadword_copy (scan_line, aligned_line, copy_count, copy_tail);

	    /* Apply the actual filter */
	    plain_over_565_8_pix_neon (
		src, scan_line + kernel_offset, 8 * sizeof(*dst_line), kernel_count);

	    /* Copy the modified scanline back */
	    neon_quadword_copy (
		dst_line, scan_line + copy_offset, width >> 3, (width & 7) * 2);
	}
    }
}

static inline void
ARGB8_over_565_8_pix_neon (uint32_t *src,
                           uint16_t *dest,
                           uint32_t  src_stride,     /* bytes, not elements */
                           uint32_t  count           /* 8-pixel groups */)
{
    asm volatile (
        "0:	@ loop\n"
        "	pld   [%[src], %[src_stride]]         @ preload from next scanline	\n"
        "	vld1.16   {d0, d1}, [%[dest]]         @ load pixels from framebuffer	\n"
        "	vld4.8   {d20, d21, d22, d23},[%[src]]! @ load source image pixels		\n"
        "	vsli.u16  q3, q0, #5                 @ duplicate framebuffer blue bits		\n"
        "	vshrn.u16 d2, q0, #8                 @ unpack red from framebuffer pixels	\n"
        "	vshrn.u16 d4, q0, #3                 @ unpack green				\n"
        "	vmvn      d18, d23                   @ we need the inverse alpha for the background	\n"
        "	vsri.u8   d2, d2, #5                 @ duplicate red bits (extend 5 to 8)	\n"
        "	vshrn.u16 d6, q3, #2                 @ unpack extended blue (truncate 10 to 8)	\n"
        "	vsri.u8   d4, d4, #6                 @ duplicate green bits (extend 6 to 8)	\n"
        "	vmull.u8  q1, d2, d18                @ apply inverse alpha to background red...	\n"
        "	vmull.u8  q2, d4, d18                @ ...green...				\n"
        "	vmull.u8  q3, d6, d18                @ ...blue					\n"
        "	subs      %[count], %[count], #1     @ decrement/test loop counter		\n"
        "	vmlal.u8  q1, d23, d22               @ add blended foreground red...		\n"
        "	vmlal.u8  q2, d23, d21               @ ...green...				\n"
        "	vmlal.u8  q3, d23, d20               @ ...blue					\n"
        "	vsri.16   q1, q2, #5                 @ pack green behind red			\n"
        "	vsri.16   q1, q3, #11                @ pack blue into pixels			\n"
        "	vst1.16   {d2, d3}, [%[dest]]!        @ store composited pixels			\n"
        "	bne 0b                               @ next please				\n"

        /* Clobbered registers marked as input/outputs */
	: [dest] "+r" (dest), [src] "+r" (src), [count] "+r" (count)

	  /* Inputs */
	: [src_stride] "r" (src_stride)

	  /* Clobbers, including the inputs we modify, and potentially lots of memory */
	: "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "d17", "d18", "d20",
	  "d21", "d22", "d23", "cc", "memory"
        );
}

static void
neon_composite_over_8888_0565 (pixman_implementation_t * impl,
                               pixman_op_t               op,
                               pixman_image_t *          src_image,
                               pixman_image_t *          mask_image,
                               pixman_image_t *          dst_image,
                               int32_t                   src_x,
                               int32_t                   src_y,
                               int32_t                   mask_x,
                               int32_t                   mask_y,
                               int32_t                   dest_x,
                               int32_t                   dest_y,
                               int32_t                   width,
                               int32_t                   height)
{
    uint32_t    *src_line;
    uint16_t    *dst_line, *aligned_line;
    uint32_t dst_stride, src_stride;
    uint32_t kernel_count, copy_count, copy_tail;
    uint8_t kernel_offset, copy_offset;

    /* we assume mask is opaque 
     * so the only alpha to deal with is embedded in src
     */
    if (width > NEON_SCANLINE_BUFFER_PIXELS)
    {
	/* split the blit, so we can use a fixed-size scanline buffer */
	int x;
	for (x = 0; x < width; x += NEON_SCANLINE_BUFFER_PIXELS)
	{
	    neon_composite_over_8888_0565 (
		impl, op,
		src_image, mask_image, dst_image,
		src_x + x, src_y, mask_x + x, mask_y, dest_x + x, dest_y,
		(x + NEON_SCANLINE_BUFFER_PIXELS > width) ? width - x : NEON_SCANLINE_BUFFER_PIXELS, height);
	}
	return;
    }

    PIXMAN_IMAGE_GET_LINE (dst_image, dest_x, dest_y, uint16_t, dst_stride, dst_line, 1);
    PIXMAN_IMAGE_GET_LINE (src_image, src_x, src_y, uint32_t, src_stride, src_line, 1);

    /* keep within minimum number of aligned quadwords on width
     * while also keeping the minimum number of columns to process
     */
    {
	unsigned long aligned_left = (unsigned long)(dst_line) & ~0xF;
	unsigned long aligned_right = (((unsigned long)(dst_line + width)) + 0xF) & ~0xF;
	unsigned long ceiling_length = (((unsigned long) width) * sizeof(*dst_line) + 0xF) & ~0xF;

	/* the fast copy should be quadword aligned */
	copy_offset = dst_line - ((uint16_t*) aligned_left);
	aligned_line = dst_line - copy_offset;
	copy_count = (uint32_t) ((aligned_right - aligned_left) >> 4);
	copy_tail = 0;

	if (aligned_right - aligned_left > ceiling_length)
	{
	    /* unaligned routine is tightest */
	    kernel_count = (uint32_t) (ceiling_length >> 4);
	    kernel_offset = copy_offset;
	}
	else
	{
	    /* aligned routine is equally tight, so it is safer to align */
	    kernel_count = copy_count;
	    kernel_offset = 0;
	}

	/* We should avoid reading beyond scanline ends for safety */
	if (aligned_line < (dst_line - dest_x) ||
	    (aligned_line + (copy_count * 16 / sizeof(*dst_line))) > ((dst_line - dest_x) + dst_image->bits.width))
	{
	    /* switch to precise read */
	    copy_offset = kernel_offset = 0;
	    aligned_line = dst_line;
	    kernel_count = (uint32_t) (ceiling_length >> 4);
	    copy_count = (width * sizeof(*dst_line)) >> 4;
	    copy_tail = (width * sizeof(*dst_line)) & 0xF;
	}
    }

    /* Preload the first input scanline */
    {
	uint8_t *src_ptr = (uint8_t*) src_line;
	uint32_t count = (width + 15) / 16;

#ifdef USE_GCC_INLINE_ASM
	asm volatile (
	    "0: @ loop						\n"
	    "	subs    %[count], %[count], #1			\n"
	    "	pld     [%[src]]				\n"
	    "	add     %[src], %[src], #64			\n"
	    "	bgt 0b						\n"

	    /* Clobbered input registers marked as input/outputs */
	    : [src] "+r" (src_ptr), [count] "+r" (count)
	    :     /* no unclobbered inputs */
	    : "cc"
	    );
#else
	do
	{
	    __pld (src_ptr);
	    src_ptr += 64;
	}
	while (--count);
#endif
    }

    {
	uint16_t scan_line[NEON_SCANLINE_BUFFER_PIXELS + 8]; /* deliberately not initialised */

	/* row-major order */
	/* left edge, middle block, right edge */
	for ( ; height--; src_line += src_stride, aligned_line += dst_stride)
	{
	    /* Uncached framebuffer access is really, really slow if we do
	     * it piecemeal. It should be much faster if we grab it all at
	     * once. One scanline should easily fit in L1 cache, so this
	     * should not waste RAM bandwidth.
	     */
	    neon_quadword_copy (scan_line, aligned_line, copy_count, copy_tail);

	    /* Apply the actual filter */
	    ARGB8_over_565_8_pix_neon (
		src_line, scan_line + kernel_offset,
		src_stride * sizeof(*src_line), kernel_count);

	    /* Copy the modified scanline back */
	    neon_quadword_copy (dst_line,
				scan_line + copy_offset,
				width >> 3, (width & 7) * 2);
	}
    }
}

#endif  /* USE_GCC_INLINE_ASM */

static const pixman_fast_path_t arm_neon_fast_path_array[] =
{
    { PIXMAN_OP_ADD,  PIXMAN_solid,    PIXMAN_a8,       PIXMAN_a8,       neon_composite_add_8888_8_8,     0 },
    { PIXMAN_OP_ADD,  PIXMAN_a8,       PIXMAN_null,     PIXMAN_a8,       neon_composite_add_8000_8000,    0 },
    { PIXMAN_OP_OVER, PIXMAN_solid,    PIXMAN_a8,       PIXMAN_r5g6b5,   neon_composite_over_n_8_0565,    0 },
    { PIXMAN_OP_OVER, PIXMAN_solid,    PIXMAN_a8,       PIXMAN_b5g6r5,   neon_composite_over_n_8_0565,    0 },
    { PIXMAN_OP_SRC,  PIXMAN_a8r8g8b8, PIXMAN_null,     PIXMAN_r5g6b5,   neon_composite_src_24_16,        0 },
    { PIXMAN_OP_SRC,  PIXMAN_x8r8g8b8, PIXMAN_null,     PIXMAN_r5g6b5,   neon_composite_src_24_16,        0 },
    { PIXMAN_OP_SRC,  PIXMAN_a8b8g8r8, PIXMAN_null,     PIXMAN_b5g6r5,   neon_composite_src_24_16,        0 },
    { PIXMAN_OP_SRC,  PIXMAN_x8b8g8r8, PIXMAN_null,     PIXMAN_b5g6r5,   neon_composite_src_24_16,        0 },
#ifdef USE_GCC_INLINE_ASM
    { PIXMAN_OP_SRC,  PIXMAN_r5g6b5,   PIXMAN_null,     PIXMAN_r5g6b5,   neon_composite_src_16_16,        0 },
    { PIXMAN_OP_SRC,  PIXMAN_b5g6r5,   PIXMAN_null,     PIXMAN_b5g6r5,   neon_composite_src_16_16,        0 },
#if 0 /* this code has some bugs */
    { PIXMAN_OP_OVER, PIXMAN_solid,    PIXMAN_null,     PIXMAN_r5g6b5,   neon_composite_over_n_0565,      0 },
    { PIXMAN_OP_OVER, PIXMAN_solid,    PIXMAN_null,     PIXMAN_b5g6r5,   neon_composite_over_n_0565,      0 },
    { PIXMAN_OP_OVER, PIXMAN_a8r8g8b8, PIXMAN_null,     PIXMAN_r5g6b5,   neon_composite_over_8888_0565,   0 },
    { PIXMAN_OP_OVER, PIXMAN_a8b8g8r8, PIXMAN_null,     PIXMAN_b5g6r5,   neon_composite_over_8888_0565,   0 },
#endif
#endif
    { PIXMAN_OP_OVER, PIXMAN_a8r8g8b8, PIXMAN_null,     PIXMAN_a8r8g8b8, neon_composite_over_8888_8888,   0 },
    { PIXMAN_OP_OVER, PIXMAN_a8r8g8b8, PIXMAN_null,     PIXMAN_x8r8g8b8, neon_composite_over_8888_8888,   0 },
    { PIXMAN_OP_OVER, PIXMAN_a8b8g8r8, PIXMAN_null,     PIXMAN_a8b8g8r8, neon_composite_over_8888_8888,   0 },
    { PIXMAN_OP_OVER, PIXMAN_a8b8g8r8, PIXMAN_null,     PIXMAN_x8b8g8r8, neon_composite_over_8888_8888,   0 },
    { PIXMAN_OP_OVER, PIXMAN_a8r8g8b8, PIXMAN_a8,       PIXMAN_a8r8g8b8, neon_composite_over_8888_n_8888, NEED_SOLID_MASK },
    { PIXMAN_OP_OVER, PIXMAN_a8r8g8b8, PIXMAN_a8,       PIXMAN_x8r8g8b8, neon_composite_over_8888_n_8888, NEED_SOLID_MASK },
    { PIXMAN_OP_OVER, PIXMAN_solid,    PIXMAN_a8,       PIXMAN_a8r8g8b8, neon_composite_over_n_8_8888,    0 },
    { PIXMAN_OP_OVER, PIXMAN_solid,    PIXMAN_a8,       PIXMAN_x8r8g8b8, neon_composite_over_n_8_8888,    0 },
    { PIXMAN_OP_OVER, PIXMAN_solid,    PIXMAN_a8,       PIXMAN_a8b8g8r8, neon_composite_over_n_8_8888,    0 },
    { PIXMAN_OP_OVER, PIXMAN_solid,    PIXMAN_a8,       PIXMAN_x8b8g8r8, neon_composite_over_n_8_8888,    0 },
    { PIXMAN_OP_NONE },
};

const pixman_fast_path_t *const arm_neon_fast_paths = arm_neon_fast_path_array;

static void
arm_neon_composite (pixman_implementation_t *imp,
                    pixman_op_t              op,
                    pixman_image_t *         src,
                    pixman_image_t *         mask,
                    pixman_image_t *         dest,
                    int32_t                  src_x,
                    int32_t                  src_y,
                    int32_t                  mask_x,
                    int32_t                  mask_y,
                    int32_t                  dest_x,
                    int32_t                  dest_y,
                    int32_t                  width,
                    int32_t                  height)
{
    if (_pixman_run_fast_path (arm_neon_fast_paths, imp,
                               op, src, mask, dest,
                               src_x, src_y,
                               mask_x, mask_y,
                               dest_x, dest_y,
                               width, height))
    {
	return;
    }

    _pixman_implementation_composite (imp->delegate, op,
                                      src, mask, dest,
                                      src_x, src_y,
                                      mask_x, mask_y,
                                      dest_x, dest_y,
                                      width, height);
}

static pixman_bool_t
pixman_blt_neon (void *src_bits,
                 void *dst_bits,
                 int   src_stride,
                 int   dst_stride,
                 int   src_bpp,
                 int   dst_bpp,
                 int   src_x,
                 int   src_y,
                 int   dst_x,
                 int   dst_y,
                 int   width,
                 int   height)
{
    if (!width || !height)
	return TRUE;

    /* accelerate only straight copies involving complete bytes */
    if (src_bpp != dst_bpp || (src_bpp & 7))
	return FALSE;

    {
	uint32_t bytes_per_pixel = src_bpp >> 3;
	uint32_t byte_width = width * bytes_per_pixel;
	/* parameter is in words for some reason */
	int32_t src_stride_bytes = src_stride * 4;
	int32_t dst_stride_bytes = dst_stride * 4;
	uint8_t *src_bytes = ((uint8_t*) src_bits) +
	    src_y * src_stride_bytes + src_x * bytes_per_pixel;
	uint8_t *dst_bytes = ((uint8_t*) dst_bits) +
	    dst_y * dst_stride_bytes + dst_x * bytes_per_pixel;
	uint32_t quadword_count = byte_width / 16;
	uint32_t offset         = byte_width % 16;

	while (height--)
	{
	    neon_quadword_copy (dst_bytes, src_bytes, quadword_count, offset);
	    src_bytes += src_stride_bytes;
	    dst_bytes += dst_stride_bytes;
	}
    }

    return TRUE;
}

static pixman_bool_t
arm_neon_blt (pixman_implementation_t *imp,
              uint32_t *               src_bits,
              uint32_t *               dst_bits,
              int                      src_stride,
              int                      dst_stride,
              int                      src_bpp,
              int                      dst_bpp,
              int                      src_x,
              int                      src_y,
              int                      dst_x,
              int                      dst_y,
              int                      width,
              int                      height)
{
    if (pixman_blt_neon (
            src_bits, dst_bits, src_stride, dst_stride, src_bpp, dst_bpp,
            src_x, src_y, dst_x, dst_y, width, height))
    {
	return TRUE;
    }

    return _pixman_implementation_blt (
               imp->delegate,
               src_bits, dst_bits, src_stride, dst_stride, src_bpp, dst_bpp,
               src_x, src_y, dst_x, dst_y, width, height);
}

static pixman_bool_t
arm_neon_fill (pixman_implementation_t *imp,
               uint32_t *               bits,
               int                      stride,
               int                      bpp,
               int                      x,
               int                      y,
               int                      width,
               int                      height,
               uint32_t xor)
{
    if (pixman_fill_neon (bits, stride, bpp, x, y, width, height, xor))
	return TRUE;

    return _pixman_implementation_fill (
	imp->delegate, bits, stride, bpp, x, y, width, height, xor);
}

pixman_implementation_t *
_pixman_implementation_create_arm_neon (void)
{
    pixman_implementation_t *general = _pixman_implementation_create_fast_path ();
    pixman_implementation_t *imp = _pixman_implementation_create (general);

    imp->composite = arm_neon_composite;
#if 0 /* this code has some bugs */
    imp->blt = arm_neon_blt;
#endif
    imp->fill = arm_neon_fill;

    return imp;
}

