/**************************************************************************
 *
 * Copyright 2010 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 **************************************************************************/


#include "u_math.h"
#include "u_format_other.h"
#include "u_format_rgb9e5.h"
#include "u_format_r11g11b10f.h"


void
util_format_r9g9b9e5_float_unpack_rgba_float(float *dst_row, unsigned dst_stride,
                                        const uint8_t *src_row, unsigned src_stride,
                                        unsigned width, unsigned height)
{
   unsigned x, y;
   for(y = 0; y < height; y += 1) {
      float *dst = dst_row;
      const uint8_t *src = src_row;
      for(x = 0; x < width; x += 1) {
         uint32_t value = *(const uint32_t *)src;
#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap32(value);
#endif
         rgb9e5_to_float3(value, dst);
         dst[3] = 1; /* a */
         src += 4;
         dst += 4;
      }
      src_row += src_stride;
      dst_row += dst_stride/sizeof(*dst_row);
   }
}

void
util_format_r9g9b9e5_float_pack_rgba_float(uint8_t *dst_row, unsigned dst_stride,
                                      const float *src_row, unsigned src_stride,
                                      unsigned width, unsigned height)
{
   unsigned x, y;
   for(y = 0; y < height; y += 1) {
      const float *src = src_row;
      uint8_t *dst = dst_row;
      for(x = 0; x < width; x += 1) {
         uint32_t value = float3_to_rgb9e5(src);
#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap32(value);
#endif
         *(uint32_t *)dst = value;
         src += 4;
         dst += 4;
      }
      dst_row += dst_stride;
      src_row += src_stride/sizeof(*src_row);
   }
}

void
util_format_r9g9b9e5_float_fetch_rgba_float(float *dst, const uint8_t *src,
                                       unsigned i, unsigned j)
{
   uint32_t value = *(const uint32_t *)src;
#ifdef PIPE_ARCH_BIG_ENDIAN
   value = util_bswap32(value);
#endif
   rgb9e5_to_float3(value, dst);
   dst[3] = 1; /* a */
}


void
util_format_r9g9b9e5_float_unpack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                         const uint8_t *src_row, unsigned src_stride,
                                         unsigned width, unsigned height)
{
   unsigned x, y;
   float p[3];
   for(y = 0; y < height; y += 1) {
      uint8_t *dst = dst_row;
      const uint8_t *src = src_row;
      for(x = 0; x < width; x += 1) {
         uint32_t value = *(const uint32_t *)src;
#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap32(value);
#endif
         rgb9e5_to_float3(value, p);
         dst[0] = float_to_ubyte(p[0]); /* r */
         dst[1] = float_to_ubyte(p[1]); /* g */
         dst[2] = float_to_ubyte(p[2]); /* b */
         dst[3] = 255; /* a */
         src += 4;
         dst += 4;
      }
      src_row += src_stride;
      dst_row += dst_stride/sizeof(*dst_row);
   }
}


void
util_format_r9g9b9e5_float_pack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                       const uint8_t *src_row, unsigned src_stride,
                                       unsigned width, unsigned height)
{
   unsigned x, y;
   float p[3];
   for(y = 0; y < height; y += 1) {
      const uint8_t *src = src_row;
      uint8_t *dst = dst_row;
      for(x = 0; x < width; x += 1) {
         uint32_t value;
         p[0] = ubyte_to_float(src[0]);
         p[1] = ubyte_to_float(src[1]);
         p[2] = ubyte_to_float(src[2]);
         value = float3_to_rgb9e5(p);
#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap32(value);
#endif
         *(uint32_t *)dst = value;
         src += 4;
         dst += 4;
      }
      dst_row += dst_stride;
      src_row += src_stride/sizeof(*src_row);
   }
}


void
util_format_r11g11b10_float_unpack_rgba_float(float *dst_row, unsigned dst_stride,
                                        const uint8_t *src_row, unsigned src_stride,
                                        unsigned width, unsigned height)
{
   unsigned x, y;
   for(y = 0; y < height; y += 1) {
      float *dst = dst_row;
      const uint8_t *src = src_row;
      for(x = 0; x < width; x += 1) {
         uint32_t value = *(const uint32_t *)src;
#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap32(value);
#endif
         r11g11b10f_to_float3(value, dst);
         dst[3] = 1; /* a */
         src += 4;
         dst += 4;
      }
      src_row += src_stride;
      dst_row += dst_stride/sizeof(*dst_row);
   }
}

void
util_format_r11g11b10_float_pack_rgba_float(uint8_t *dst_row, unsigned dst_stride,
                                      const float *src_row, unsigned src_stride,
                                      unsigned width, unsigned height)
{
   unsigned x, y;
   for(y = 0; y < height; y += 1) {
      const float *src = src_row;
      uint8_t *dst = dst_row;
      for(x = 0; x < width; x += 1) {
         uint32_t value = float3_to_r11g11b10f(src);
#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap32(value);
#endif
         *(uint32_t *)dst = value;
         src += 4;
         dst += 4;
      }
      dst_row += dst_stride;
      src_row += src_stride/sizeof(*src_row);
   }
}

void
util_format_r11g11b10_float_fetch_rgba_float(float *dst, const uint8_t *src,
                                       unsigned i, unsigned j)
{
   uint32_t value = *(const uint32_t *)src;
#ifdef PIPE_ARCH_BIG_ENDIAN
   value = util_bswap32(value);
#endif
   r11g11b10f_to_float3(value, dst);
   dst[3] = 1; /* a */
}


void
util_format_r11g11b10_float_unpack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                         const uint8_t *src_row, unsigned src_stride,
                                         unsigned width, unsigned height)
{
   unsigned x, y;
   float p[3];
   for(y = 0; y < height; y += 1) {
      uint8_t *dst = dst_row;
      const uint8_t *src = src_row;
      for(x = 0; x < width; x += 1) {
         uint32_t value = *(const uint32_t *)src;
#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap32(value);
#endif
         r11g11b10f_to_float3(value, p);
         dst[0] = float_to_ubyte(p[0]); /* r */
         dst[1] = float_to_ubyte(p[1]); /* g */
         dst[2] = float_to_ubyte(p[2]); /* b */
         dst[3] = 255; /* a */
         src += 4;
         dst += 4;
      }
      src_row += src_stride;
      dst_row += dst_stride/sizeof(*dst_row);
   }
}


void
util_format_r11g11b10_float_pack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                       const uint8_t *src_row, unsigned src_stride,
                                       unsigned width, unsigned height)
{
   unsigned x, y;
   float p[3];
   for(y = 0; y < height; y += 1) {
      const uint8_t *src = src_row;
      uint8_t *dst = dst_row;
      for(x = 0; x < width; x += 1) {
         uint32_t value;
         p[0] = ubyte_to_float(src[0]);
         p[1] = ubyte_to_float(src[1]);
         p[2] = ubyte_to_float(src[2]);
         value = float3_to_r11g11b10f(p);
#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap32(value);
#endif
         *(uint32_t *)dst = value;
         src += 4;
         dst += 4;
      }
      dst_row += dst_stride;
      src_row += src_stride/sizeof(*src_row);
   }
}


void
util_format_r1_unorm_unpack_rgba_float(float *dst_row, unsigned dst_stride,
                                  const uint8_t *src_row, unsigned src_stride,
                                  unsigned width, unsigned height)
{

}


void
util_format_r1_unorm_pack_rgba_float(uint8_t *dst_row, unsigned dst_stride,
                                const float *src_row, unsigned src_stride,
                                unsigned width, unsigned height)
{

}


void
util_format_r1_unorm_fetch_rgba_float(float *dst, const uint8_t *src,
                                 unsigned i, unsigned j)
{

}


void
util_format_r1_unorm_unpack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                   const uint8_t *src_row, unsigned src_stride,
                                   unsigned width, unsigned height)
{

}


void
util_format_r1_unorm_pack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                 const uint8_t *src_row, unsigned src_stride,
                                 unsigned width, unsigned height)
{
}


/*
 * PIPE_FORMAT_R8G8Bx_SNORM
 *
 * A.k.a. D3DFMT_CxV8U8
 */

static uint8_t
r8g8bx_derive(int16_t r, int16_t g)
{
   /* Derive blue from red and green components.
    * Apparently, we must always use integers to perform calculations,
    * otherwise the results won't match D3D's CxV8U8 definition.
    */
   return (uint8_t)sqrtf(0x7f * 0x7f - r * r - g * g) * 0xff / 0x7f;
}

void
util_format_r8g8bx_snorm_unpack_rgba_float(float *dst_row, unsigned dst_stride,
                                      const uint8_t *src_row, unsigned src_stride,
                                      unsigned width, unsigned height)
{
   unsigned x, y;

   for(y = 0; y < height; y += 1) {
      float *dst = dst_row;
      const uint16_t *src = (const uint16_t *)src_row;
      for(x = 0; x < width; x += 1) {
         uint16_t value = *src++;
         int16_t r, g;

#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap16(value);
#endif

         r = ((int16_t)(value << 8)) >> 8;
         g = ((int16_t)(value << 0)) >> 8;

         dst[0] = (float)(r * (1.0f/0x7f)); /* r */
         dst[1] = (float)(g * (1.0f/0x7f)); /* g */
         dst[2] = r8g8bx_derive(r, g) * (1.0f/0xff); /* b */
         dst[3] = 1.0f; /* a */
         dst += 4;
      }
      src_row += src_stride;
      dst_row += dst_stride/sizeof(*dst_row);
   }
}


void
util_format_r8g8bx_snorm_unpack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                       const uint8_t *src_row, unsigned src_stride,
                                       unsigned width, unsigned height)
{
   unsigned x, y;
   for(y = 0; y < height; y += 1) {
      uint8_t *dst = dst_row;
      const uint16_t *src = (const uint16_t *)src_row;
      for(x = 0; x < width; x += 1) {
         uint16_t value = *src++;
         int16_t r, g;

#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap16(value);
#endif

         r = ((int16_t)(value << 8)) >> 8;
         g = ((int16_t)(value << 0)) >> 8;

         dst[0] = (uint8_t)(((uint16_t)MAX2(r, 0)) * 0xff / 0x7f); /* r */
         dst[1] = (uint8_t)(((uint16_t)MAX2(g, 0)) * 0xff / 0x7f); /* g */
         dst[2] = r8g8bx_derive(r, g); /* b */
         dst[3] = 255; /* a */
         dst += 4;
      }
      src_row += src_stride;
      dst_row += dst_stride/sizeof(*dst_row);
   }
}


void
util_format_r8g8bx_snorm_pack_rgba_float(uint8_t *dst_row, unsigned dst_stride,
                                    const float *src_row, unsigned src_stride,
                                    unsigned width, unsigned height)
{
   unsigned x, y;
   for(y = 0; y < height; y += 1) {
      const float *src = src_row;
      uint16_t *dst = (uint16_t *)dst_row;
      for(x = 0; x < width; x += 1) {
         uint16_t value = 0;

         value |= (uint16_t)(((int8_t)(CLAMP(src[0], -1, 1) * 0x7f)) & 0xff) ;
         value |= (uint16_t)((((int8_t)(CLAMP(src[1], -1, 1) * 0x7f)) & 0xff) << 8) ;

#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap16(value);
#endif

         *dst++ = value;

         src += 4;
      }
      dst_row += dst_stride;
      src_row += src_stride/sizeof(*src_row);
   }
}


void
util_format_r8g8bx_snorm_pack_rgba_8unorm(uint8_t *dst_row, unsigned dst_stride,
                                     const uint8_t *src_row, unsigned src_stride,
                                     unsigned width, unsigned height)
{
   unsigned x, y;

   for(y = 0; y < height; y += 1) {
      const uint8_t *src = src_row;
      uint16_t *dst = (uint16_t *)dst_row;
      for(x = 0; x < width; x += 1) {
         uint16_t value = 0;

         value |= src[0] >> 1;
         value |= (src[1] >> 1) << 8;

#ifdef PIPE_ARCH_BIG_ENDIAN
         value = util_bswap16(value);
#endif

         *dst++ = value;

         src += 4;
      }
      dst_row += dst_stride;
      src_row += src_stride/sizeof(*src_row);
   }
}


void
util_format_r8g8bx_snorm_fetch_rgba_float(float *dst, const uint8_t *src,
                                     unsigned i, unsigned j)
{
   uint16_t value = *(const uint16_t *)src;
   int16_t r, g;

#ifdef PIPE_ARCH_BIG_ENDIAN
   value = util_bswap16(value);
#endif

   r = ((int16_t)(value << 8)) >> 8;
   g = ((int16_t)(value << 0)) >> 8;

   dst[0] = r * (1.0f/0x7f); /* r */
   dst[1] = g * (1.0f/0x7f); /* g */
   dst[2] = r8g8bx_derive(r, g) * (1.0f/0xff); /* b */
   dst[3] = 1.0f; /* a */
}
