/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include "main/glheader.h"
#include "main/context.h"
#include "main/formats.h"
#include "main/format_unpack.h"
#include "main/format_pack.h"
#include "main/macros.h"
#include "main/imports.h"

#include "s_context.h"
#include "s_depth.h"
#include "s_span.h"



#define Z_TEST(COMPARE)                      \
   do {                                      \
      GLuint i;                              \
      for (i = 0; i < n; i++) {              \
         if (mask[i]) {                      \
            if (COMPARE) {                   \
               /* pass */                    \
               if (write) {                  \
                  zbuffer[i] = zfrag[i];     \
               }                             \
               passed++;                     \
            }                                \
            else {                           \
               /* fail */                    \
               mask[i] = 0;                  \
            }                                \
         }                                   \
      }                                      \
   } while (0)


/**
 * Do depth test for an array of 16-bit Z values.
 * @param zbuffer  array of Z buffer values (16-bit)
 * @param zfrag  array of fragment Z values (use 16-bit in 32-bit uint)
 * @param mask  which fragments are alive, killed afterward
 * @return  number of fragments which pass the test.
 */
static GLuint
depth_test_span16( struct gl_context *ctx, GLuint n,
                   GLushort zbuffer[], const GLuint zfrag[], GLubyte mask[] )
{
   const GLboolean write = ctx->Depth.Mask;
   GLuint passed = 0;

   /* switch cases ordered from most frequent to less frequent */
   switch (ctx->Depth.Func) {
   case GL_LESS:
      Z_TEST(zfrag[i] < zbuffer[i]);
      break;
   case GL_LEQUAL:
      Z_TEST(zfrag[i] <= zbuffer[i]);
      break;
   case GL_GEQUAL:
      Z_TEST(zfrag[i] >= zbuffer[i]);
      break;
   case GL_GREATER:
      Z_TEST(zfrag[i] > zbuffer[i]);
      break;
   case GL_NOTEQUAL:
      Z_TEST(zfrag[i] != zbuffer[i]);
      break;
   case GL_EQUAL:
      Z_TEST(zfrag[i] == zbuffer[i]);
      break;
   case GL_ALWAYS:
      Z_TEST(1);
      break;
   case GL_NEVER:
      memset(mask, 0, n * sizeof(GLubyte));
      break;
   default:
      _mesa_problem(ctx, "Bad depth func in depth_test_span16");
   }

   return passed;
}


/**
 * Do depth test for an array of 32-bit Z values.
 * @param zbuffer  array of Z buffer values (32-bit)
 * @param zfrag  array of fragment Z values (use 32-bits in 32-bit uint)
 * @param mask  which fragments are alive, killed afterward
 * @return  number of fragments which pass the test.
 */
static GLuint
depth_test_span32( struct gl_context *ctx, GLuint n,
                   GLuint zbuffer[], const GLuint zfrag[], GLubyte mask[])
{
   const GLboolean write = ctx->Depth.Mask;
   GLuint passed = 0;

   /* switch cases ordered from most frequent to less frequent */
   switch (ctx->Depth.Func) {
   case GL_LESS:
      Z_TEST(zfrag[i] < zbuffer[i]);
      break;
   case GL_LEQUAL:
      Z_TEST(zfrag[i] <= zbuffer[i]);
      break;
   case GL_GEQUAL:
      Z_TEST(zfrag[i] >= zbuffer[i]);
      break;
   case GL_GREATER:
      Z_TEST(zfrag[i] > zbuffer[i]);
      break;
   case GL_NOTEQUAL:
      Z_TEST(zfrag[i] != zbuffer[i]);
      break;
   case GL_EQUAL:
      Z_TEST(zfrag[i] == zbuffer[i]);
      break;
   case GL_ALWAYS:
      Z_TEST(1);
      break;
   case GL_NEVER:
      memset(mask, 0, n * sizeof(GLubyte));
      break;
   default:
      _mesa_problem(ctx, "Bad depth func in depth_test_span32");
   }

   return passed;
}


/**
 * Clamp fragment Z values to the depth near/far range (glDepthRange()).
 * This is used when GL_ARB_depth_clamp/GL_DEPTH_CLAMP is turned on.
 * In that case, vertexes are not clipped against the near/far planes
 * so rasterization will produce fragment Z values outside the usual
 * [0,1] range.
 */
void
_swrast_depth_clamp_span( struct gl_context *ctx, SWspan *span )
{
   struct gl_framebuffer *fb = ctx->DrawBuffer;
   const GLuint count = span->end;
   GLint *zValues = (GLint *) span->array->z; /* sign change */
   GLint min, max;
   GLfloat min_f, max_f;
   GLuint i;

   if (ctx->ViewportArray[0].Near < ctx->ViewportArray[0].Far) {
      min_f = ctx->ViewportArray[0].Near;
      max_f = ctx->ViewportArray[0].Far;
   } else {
      min_f = ctx->ViewportArray[0].Far;
      max_f = ctx->ViewportArray[0].Near;
   }

   /* Convert floating point values in [0,1] to device Z coordinates in
    * [0, DepthMax].
    * ex: If the Z buffer has 24 bits, DepthMax = 0xffffff.
    * 
    * XXX this all falls apart if we have 31 or more bits of Z because
    * the triangle rasterization code produces unsigned Z values.  Negative
    * vertex Z values come out as large fragment Z uints.
    */
   min = (GLint) (min_f * fb->_DepthMaxF);
   max = (GLint) (max_f * fb->_DepthMaxF);
   if (max < 0)
      max = 0x7fffffff; /* catch over flow for 30-bit z */

   /* Note that we do the comparisons here using signed integers.
    */
   for (i = 0; i < count; i++) {
      if (zValues[i] < min)
	 zValues[i] = min;
      if (zValues[i] > max)
	 zValues[i] = max;
   }
}


/**
 * Get array of 32-bit z values from the depth buffer.  With clipping.
 * Note: the returned values are always in the range [0, 2^32-1].
 */
static void
get_z32_values(struct gl_context *ctx, struct gl_renderbuffer *rb,
               GLuint count, const GLint x[], const GLint y[],
               GLuint zbuffer[])
{
   struct swrast_renderbuffer *srb = swrast_renderbuffer(rb);
   const GLint w = rb->Width, h = rb->Height;
   const GLubyte *map = _swrast_pixel_address(rb, 0, 0);
   GLuint i;

   if (rb->Format == MESA_FORMAT_Z_UNORM32) {
      const GLint rowStride = srb->RowStride;
      for (i = 0; i < count; i++) {
         if (x[i] >= 0 && y[i] >= 0 && x[i] < w && y[i] < h) {
            zbuffer[i] = *((GLuint *) (map + y[i] * rowStride + x[i] * 4));
         }
      }
   }
   else {
      const GLint bpp = _mesa_get_format_bytes(rb->Format);
      const GLint rowStride = srb->RowStride;
      for (i = 0; i < count; i++) {
         if (x[i] >= 0 && y[i] >= 0 && x[i] < w && y[i] < h) {
            const GLubyte *src = map + y[i] * rowStride+ x[i] * bpp;
            _mesa_unpack_uint_z_row(rb->Format, 1, src, &zbuffer[i]);
         }
      }
   }
}


/**
 * Put an array of 32-bit z values into the depth buffer.
 * Note: the z values are always in the range [0, 2^32-1].
 */
static void
put_z32_values(struct gl_context *ctx, struct gl_renderbuffer *rb,
               GLuint count, const GLint x[], const GLint y[],
               const GLuint zvalues[], const GLubyte mask[])
{
   struct swrast_renderbuffer *srb = swrast_renderbuffer(rb);
   const GLint w = rb->Width, h = rb->Height;
   GLubyte *map = _swrast_pixel_address(rb, 0, 0);
   GLuint i;

   if (rb->Format == MESA_FORMAT_Z_UNORM32) {
      const GLint rowStride = srb->RowStride;
      for (i = 0; i < count; i++) {
         if (mask[i] && x[i] >= 0 && y[i] >= 0 && x[i] < w && y[i] < h) {
            GLuint *dst = (GLuint *) (map + y[i] * rowStride + x[i] * 4);
            *dst = zvalues[i];
         }
      }
   }
   else {
      gl_pack_uint_z_func packZ = _mesa_get_pack_uint_z_func(rb->Format);
      const GLint bpp = _mesa_get_format_bytes(rb->Format);
      const GLint rowStride = srb->RowStride;
      for (i = 0; i < count; i++) {
         if (mask[i] && x[i] >= 0 && y[i] >= 0 && x[i] < w && y[i] < h) {
            void *dst = map + y[i] * rowStride + x[i] * bpp;
            packZ(zvalues + i, dst);
         }
      }
   }
}


/**
 * Apply depth (Z) buffer testing to the span.
 * \return approx number of pixels that passed (only zero is reliable)
 */
GLuint
_swrast_depth_test_span(struct gl_context *ctx, SWspan *span)
{
   struct gl_framebuffer *fb = ctx->DrawBuffer;
   struct gl_renderbuffer *rb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   const GLint bpp = _mesa_get_format_bytes(rb->Format);
   void *zStart;
   const GLuint count = span->end;
   const GLuint *fragZ = span->array->z;
   GLubyte *mask = span->array->mask;
   void *zBufferVals;
   GLuint *zBufferTemp = NULL;
   GLuint passed;
   GLuint zBits = _mesa_get_format_bits(rb->Format, GL_DEPTH_BITS);
   GLboolean ztest16 = GL_FALSE;

   if (span->arrayMask & SPAN_XY)
      zStart = NULL;
   else
      zStart = _swrast_pixel_address(rb, span->x, span->y);

   if (rb->Format == MESA_FORMAT_Z_UNORM16 && !(span->arrayMask & SPAN_XY)) {
      /* directly read/write row of 16-bit Z values */
      zBufferVals = zStart;
      ztest16 = GL_TRUE;
   }
   else if (rb->Format == MESA_FORMAT_Z_UNORM32 && !(span->arrayMask & SPAN_XY)) {
      /* directly read/write row of 32-bit Z values */
      zBufferVals = zStart;
   }
   else {
      if (_mesa_get_format_datatype(rb->Format) != GL_UNSIGNED_NORMALIZED) {
         _mesa_problem(ctx, "Incorrectly writing swrast's integer depth "
                       "values to %s depth buffer",
                       _mesa_get_format_name(rb->Format));
      }

      /* copy Z buffer values into temp buffer (32-bit Z values) */
      zBufferTemp = malloc(count * sizeof(GLuint));
      if (!zBufferTemp)
         return 0;

      if (span->arrayMask & SPAN_XY) {
         get_z32_values(ctx, rb, count,
                        span->array->x, span->array->y, zBufferTemp);
      }
      else {
         _mesa_unpack_uint_z_row(rb->Format, count, zStart, zBufferTemp);
      }

      if (zBits == 24) {
         GLuint i;
         /* Convert depth buffer values from 32 to 24 bits to match the
          * fragment Z values generated by rasterization.
          */
         for (i = 0; i < count; i++) {
            zBufferTemp[i] >>= 8;
         }
      }
      else if (zBits == 16) {
         GLuint i;
         /* Convert depth buffer values from 32 to 16 bits */
         for (i = 0; i < count; i++) {
            zBufferTemp[i] >>= 16;
         }
      }
      else {
         assert(zBits == 32);
      }

      zBufferVals = zBufferTemp;
   }

   /* do the depth test either with 16 or 32-bit values */
   if (ztest16)
      passed = depth_test_span16(ctx, count, zBufferVals, fragZ, mask);
   else
      passed = depth_test_span32(ctx, count, zBufferVals, fragZ, mask);

   if (zBufferTemp) {
      /* need to write temp Z values back into the buffer */

      /* Convert depth buffer values back to 32-bit values.  The least
       * significant bits don't matter since they'll get dropped when
       * they're packed back into the depth buffer.
       */
      if (zBits == 24) {
         GLuint i;
         for (i = 0; i < count; i++) {
            zBufferTemp[i] = (zBufferTemp[i] << 8);
         }
      }
      else if (zBits == 16) {
         GLuint i;
         for (i = 0; i < count; i++) {
            zBufferTemp[i] = zBufferTemp[i] << 16;
         }
      }

      if (span->arrayMask & SPAN_XY) {
         /* random locations */
         put_z32_values(ctx, rb, count, span->array->x, span->array->y,
                        zBufferTemp, mask);
      }
      else {
         /* horizontal row */
         gl_pack_uint_z_func packZ = _mesa_get_pack_uint_z_func(rb->Format);
         GLubyte *dst = zStart;
         GLuint i;
         for (i = 0; i < count; i++) {
            if (mask[i]) {
               packZ(&zBufferTemp[i], dst);
            }
            dst += bpp;
         }
      }

      free(zBufferTemp);
   }

   if (passed < count) {
      span->writeAll = GL_FALSE;
   }
   return passed;
}


/**
 * GL_EXT_depth_bounds_test extension.
 * Discard fragments depending on whether the corresponding Z-buffer
 * values are outside the depth bounds test range.
 * Note: we test the Z buffer values, not the fragment Z values!
 * \return GL_TRUE if any fragments pass, GL_FALSE if no fragments pass
 */
GLboolean
_swrast_depth_bounds_test( struct gl_context *ctx, SWspan *span )
{
   struct gl_framebuffer *fb = ctx->DrawBuffer;
   struct gl_renderbuffer *rb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   GLubyte *zStart;
   GLuint zMin = (GLuint) (ctx->Depth.BoundsMin * fb->_DepthMaxF + 0.5F);
   GLuint zMax = (GLuint) (ctx->Depth.BoundsMax * fb->_DepthMaxF + 0.5F);
   GLubyte *mask = span->array->mask;
   const GLuint count = span->end;
   GLuint i;
   GLboolean anyPass = GL_FALSE;
   GLuint *zBufferTemp;
   const GLuint *zBufferVals;

   zBufferTemp = malloc(count * sizeof(GLuint));
   if (!zBufferTemp) {
      /* don't generate a stream of OUT_OF_MEMORY errors here */
      return GL_FALSE;
   }

   if (span->arrayMask & SPAN_XY)
      zStart = NULL;
   else
      zStart = _swrast_pixel_address(rb, span->x, span->y);

   if (rb->Format == MESA_FORMAT_Z_UNORM32 && !(span->arrayMask & SPAN_XY)) {
      /* directly access 32-bit values in the depth buffer */
      zBufferVals = (const GLuint *) zStart;
   }
   else {
      /* unpack Z values into a temporary array */
      if (span->arrayMask & SPAN_XY) {
         get_z32_values(ctx, rb, count, span->array->x, span->array->y,
                        zBufferTemp);
      }
      else {
         _mesa_unpack_uint_z_row(rb->Format, count, zStart, zBufferTemp);
      }
      zBufferVals = zBufferTemp;
   }

   /* Now do the tests */
   for (i = 0; i < count; i++) {
      if (mask[i]) {
         if (zBufferVals[i] < zMin || zBufferVals[i] > zMax)
            mask[i] = GL_FALSE;
         else
            anyPass = GL_TRUE;
      }
   }

   free(zBufferTemp);

   return anyPass;
}



/**********************************************************************/
/*****                      Read Depth Buffer                     *****/
/**********************************************************************/


/**
 * Read a span of depth values from the given depth renderbuffer, returning
 * the values as GLfloats.
 * This function does clipping to prevent reading outside the depth buffer's
 * bounds.
 */
void
_swrast_read_depth_span_float(struct gl_context *ctx,
                              struct gl_renderbuffer *rb,
                              GLint n, GLint x, GLint y, GLfloat depth[])
{
   if (!rb) {
      /* really only doing this to prevent FP exceptions later */
      memset(depth, 0, n * sizeof(GLfloat));
      return;
   }

   if (y < 0 || y >= (GLint) rb->Height ||
       x + n <= 0 || x >= (GLint) rb->Width) {
      /* span is completely outside framebuffer */
      memset(depth, 0, n * sizeof(GLfloat));
      return;
   }

   if (x < 0) {
      GLint dx = -x;
      GLint i;
      for (i = 0; i < dx; i++)
         depth[i] = 0.0;
      x = 0;
      n -= dx;
      depth += dx;
   }
   if (x + n > (GLint) rb->Width) {
      GLint dx = x + n - (GLint) rb->Width;
      GLint i;
      for (i = 0; i < dx; i++)
         depth[n - i - 1] = 0.0;
      n -= dx;
   }
   if (n <= 0) {
      return;
   }

   _mesa_unpack_float_z_row(rb->Format, n, _swrast_pixel_address(rb, x, y),
                            depth);
}


/**
 * Clear the given z/depth renderbuffer.  If the buffer is a combined
 * depth+stencil buffer, only the Z bits will be touched.
 */
void
_swrast_clear_depth_buffer(struct gl_context *ctx)
{
   struct gl_renderbuffer *rb =
      ctx->DrawBuffer->Attachment[BUFFER_DEPTH].Renderbuffer;
   GLint x, y, width, height;
   GLubyte *map;
   GLint rowStride, i, j;
   GLbitfield mapMode;

   if (!rb || !ctx->Depth.Mask) {
      /* no depth buffer, or writing to it is disabled */
      return;
   }

   /* compute region to clear */
   x = ctx->DrawBuffer->_Xmin;
   y = ctx->DrawBuffer->_Ymin;
   width  = ctx->DrawBuffer->_Xmax - ctx->DrawBuffer->_Xmin;
   height = ctx->DrawBuffer->_Ymax - ctx->DrawBuffer->_Ymin;

   mapMode = GL_MAP_WRITE_BIT;
   if (rb->Format == MESA_FORMAT_Z24_UNORM_S8_UINT ||
       rb->Format == MESA_FORMAT_Z24_UNORM_X8_UINT ||
       rb->Format == MESA_FORMAT_S8_UINT_Z24_UNORM ||
       rb->Format == MESA_FORMAT_X8_UINT_Z24_UNORM) {
      mapMode |= GL_MAP_READ_BIT;
   }

   ctx->Driver.MapRenderbuffer(ctx, rb, x, y, width, height,
                               mapMode, &map, &rowStride);
   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glClear(depth)");
      return;
   }

   switch (rb->Format) {
   case MESA_FORMAT_Z_UNORM16:
      {
         GLfloat clear = (GLfloat) ctx->Depth.Clear;
         GLushort clearVal = 0;
         _mesa_pack_float_z_row(rb->Format, 1, &clear, &clearVal);
         if (clearVal == 0xffff && width * 2 == rowStride) {
            /* common case */
            memset(map, 0xff, width * height * 2);
         }
         else {
            for (i = 0; i < height; i++) {
               GLushort *row = (GLushort *) map;
               for (j = 0; j < width; j++) {
                  row[j] = clearVal;
               }
               map += rowStride;
            }
         }
      }
      break;
   case MESA_FORMAT_Z_UNORM32:
   case MESA_FORMAT_Z_FLOAT32:
      {
         GLfloat clear = (GLfloat) ctx->Depth.Clear;
         GLuint clearVal = 0;
         _mesa_pack_float_z_row(rb->Format, 1, &clear, &clearVal);
         for (i = 0; i < height; i++) {
            GLuint *row = (GLuint *) map;
            for (j = 0; j < width; j++) {
               row[j] = clearVal;
            }
            map += rowStride;
         }
      }
      break;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
   case MESA_FORMAT_Z24_UNORM_X8_UINT:
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
   case MESA_FORMAT_X8_UINT_Z24_UNORM:
      {
         GLfloat clear = (GLfloat) ctx->Depth.Clear;
         GLuint clearVal = 0;
         GLuint mask;

         if (rb->Format == MESA_FORMAT_Z24_UNORM_S8_UINT ||
             rb->Format == MESA_FORMAT_Z24_UNORM_X8_UINT)
            mask = 0xff000000;
         else
            mask = 0xff;

         _mesa_pack_float_z_row(rb->Format, 1, &clear, &clearVal);
         for (i = 0; i < height; i++) {
            GLuint *row = (GLuint *) map;
            for (j = 0; j < width; j++) {
               row[j] = (row[j] & mask) | clearVal;
            }
            map += rowStride;
         }

      }
      break;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      /* XXX untested */
      {
         GLfloat clearVal = (GLfloat) ctx->Depth.Clear;
         for (i = 0; i < height; i++) {
            GLfloat *row = (GLfloat *) map;
            for (j = 0; j < width; j++) {
               row[j * 2] = clearVal;
            }
            map += rowStride;
         }
      }
      break;
   default:
      _mesa_problem(ctx, "Unexpected depth buffer format %s"
                    " in _swrast_clear_depth_buffer()",
                    _mesa_get_format_name(rb->Format));
   }

   ctx->Driver.UnmapRenderbuffer(ctx, rb);
}




/**
 * Clear both depth and stencil values in a combined depth+stencil buffer.
 */
void
_swrast_clear_depth_stencil_buffer(struct gl_context *ctx)
{
   const GLubyte stencilBits = ctx->DrawBuffer->Visual.stencilBits;
   const GLuint writeMask = ctx->Stencil.WriteMask[0];
   const GLuint stencilMax = (1 << stencilBits) - 1;
   struct gl_renderbuffer *rb =
      ctx->DrawBuffer->Attachment[BUFFER_DEPTH].Renderbuffer;
   GLint x, y, width, height;
   GLbitfield mapMode;
   GLubyte *map;
   GLint rowStride, i, j;

   /* check that we really have a combined depth+stencil buffer */
   assert(rb == ctx->DrawBuffer->Attachment[BUFFER_STENCIL].Renderbuffer);

   /* compute region to clear */
   x = ctx->DrawBuffer->_Xmin;
   y = ctx->DrawBuffer->_Ymin;
   width  = ctx->DrawBuffer->_Xmax - ctx->DrawBuffer->_Xmin;
   height = ctx->DrawBuffer->_Ymax - ctx->DrawBuffer->_Ymin;

   mapMode = GL_MAP_WRITE_BIT;
   if ((writeMask & stencilMax) != stencilMax) {
      /* need to mask stencil values */
      mapMode |= GL_MAP_READ_BIT;
   }

   ctx->Driver.MapRenderbuffer(ctx, rb, x, y, width, height,
                               mapMode, &map, &rowStride);
   if (!map) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glClear(depth+stencil)");
      return;
   }

   switch (rb->Format) {
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
      {
         GLfloat zClear = (GLfloat) ctx->Depth.Clear;
         GLuint clear = 0, mask;

         _mesa_pack_float_z_row(rb->Format, 1, &zClear, &clear);

         if (rb->Format == MESA_FORMAT_Z24_UNORM_S8_UINT) {
            mask = ((~writeMask) & 0xff) << 24;
            clear |= (ctx->Stencil.Clear & writeMask & 0xff) << 24;
         }
         else {
            mask = ((~writeMask) & 0xff);
            clear |= (ctx->Stencil.Clear & writeMask & 0xff);
         }

         for (i = 0; i < height; i++) {
            GLuint *row = (GLuint *) map;
            if (mask != 0x0) {
               for (j = 0; j < width; j++) {
                  row[j] = (row[j] & mask) | clear;
               }
            }
            else {
               for (j = 0; j < width; j++) {
                  row[j] = clear;
               }
            }
            map += rowStride;
         }
      }
      break;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      /* XXX untested */
      {
         const GLfloat zClear = (GLfloat) ctx->Depth.Clear;
         const GLuint sClear = ctx->Stencil.Clear & writeMask;
         const GLuint sMask = (~writeMask) & 0xff;
         for (i = 0; i < height; i++) {
            GLfloat *zRow = (GLfloat *) map;
            GLuint *sRow = (GLuint *) map;
            for (j = 0; j < width; j++) {
               zRow[j * 2 + 0] = zClear;
            }
            if (sMask != 0) {
               for (j = 0; j < width; j++) {
                  sRow[j * 2 + 1] = (sRow[j * 2 + 1] & sMask) | sClear;
               }
            }
            else {
               for (j = 0; j < width; j++) {
                  sRow[j * 2 + 1] = sClear;
               }
            }
            map += rowStride;
         }
      }
      break;
   default:
      _mesa_problem(ctx, "Unexpected depth buffer format %s"
                    " in _swrast_clear_depth_buffer()",
                    _mesa_get_format_name(rb->Format));
   }

   ctx->Driver.UnmapRenderbuffer(ctx, rb);

}
