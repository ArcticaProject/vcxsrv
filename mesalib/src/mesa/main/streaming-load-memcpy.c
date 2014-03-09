/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Matt Turner <mattst88@gmail.com>
 *
 */

#ifdef __SSE4_1__
#include "main/macros.h"
#include "main/streaming-load-memcpy.h"
#include <smmintrin.h>

/* Copies memory from src to dst, using SSE 4.1's MOVNTDQA to get streaming
 * read performance from uncached memory.
 */
void
_mesa_streaming_load_memcpy(void *restrict dst, void *restrict src, size_t len)
{
   char *restrict d = dst;
   char *restrict s = src;

   /* If dst and src are not co-aligned, fallback to memcpy(). */
   if (((uintptr_t)d & 15) != ((uintptr_t)s & 15)) {
      memcpy(d, s, len);
      return;
   }

   /* memcpy() the misaligned header. At the end of this if block, <d> and <s>
    * are aligned to a 16-byte boundary or <len> == 0.
    */
   if ((uintptr_t)d & 15) {
      uintptr_t bytes_before_alignment_boundary = 16 - ((uintptr_t)d & 15);
      assert(bytes_before_alignment_boundary < 16);

      memcpy(d, s, MIN2(bytes_before_alignment_boundary, len));

      d = (char *)ALIGN((uintptr_t)d, 16);
      s = (char *)ALIGN((uintptr_t)s, 16);
      len -= MIN2(bytes_before_alignment_boundary, len);
   }

   while (len >= 64) {
      __m128i *dst_cacheline = (__m128i *)d;
      __m128i *src_cacheline = (__m128i *)s;

      __m128i temp1 = _mm_stream_load_si128(src_cacheline + 0);
      __m128i temp2 = _mm_stream_load_si128(src_cacheline + 1);
      __m128i temp3 = _mm_stream_load_si128(src_cacheline + 2);
      __m128i temp4 = _mm_stream_load_si128(src_cacheline + 3);

      _mm_store_si128(dst_cacheline + 0, temp1);
      _mm_store_si128(dst_cacheline + 1, temp2);
      _mm_store_si128(dst_cacheline + 2, temp3);
      _mm_store_si128(dst_cacheline + 3, temp4);

      d += 64;
      s += 64;
      len -= 64;
   }

   /* memcpy() the tail. */
   if (len) {
      memcpy(d, s, len);
   }
}

#endif
