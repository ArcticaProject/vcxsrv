/*
 * Mesa 3-D graphics library
 * Version:  6.5
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "main/glheader.h"
#include "main/condrender.h"
#include "main/image.h"
#include "main/macros.h"
#include "s_context.h"


#define ABS(X)   ((X) < 0 ? -(X) : (X))


/**
 * Generate a row resampler function for GL_NEAREST mode.
 */
#define RESAMPLE(NAME, PIXELTYPE, SIZE)			\
static void						\
NAME(GLint srcWidth, GLint dstWidth,			\
     const GLvoid *srcBuffer, GLvoid *dstBuffer,	\
     GLboolean flip)					\
{							\
   const PIXELTYPE *src = (const PIXELTYPE *) srcBuffer;\
   PIXELTYPE *dst = (PIXELTYPE *) dstBuffer;		\
   GLint dstCol;					\
							\
   if (flip) {						\
      for (dstCol = 0; dstCol < dstWidth; dstCol++) {	\
         GLint srcCol = (dstCol * srcWidth) / dstWidth;	\
         ASSERT(srcCol >= 0);				\
         ASSERT(srcCol < srcWidth);			\
         srcCol = srcWidth - 1 - srcCol; /* flip */	\
         if (SIZE == 1) {				\
            dst[dstCol] = src[srcCol];			\
         }						\
         else if (SIZE == 2) {				\
            dst[dstCol*2+0] = src[srcCol*2+0];		\
            dst[dstCol*2+1] = src[srcCol*2+1];		\
         }						\
         else if (SIZE == 4) {				\
            dst[dstCol*4+0] = src[srcCol*4+0];		\
            dst[dstCol*4+1] = src[srcCol*4+1];		\
            dst[dstCol*4+2] = src[srcCol*4+2];		\
            dst[dstCol*4+3] = src[srcCol*4+3];		\
         }						\
      }							\
   }							\
   else {						\
      for (dstCol = 0; dstCol < dstWidth; dstCol++) {	\
         GLint srcCol = (dstCol * srcWidth) / dstWidth;	\
         ASSERT(srcCol >= 0);				\
         ASSERT(srcCol < srcWidth);			\
         if (SIZE == 1) {				\
            dst[dstCol] = src[srcCol];			\
         }						\
         else if (SIZE == 2) {				\
            dst[dstCol*2+0] = src[srcCol*2+0];		\
            dst[dstCol*2+1] = src[srcCol*2+1];		\
         }						\
         else if (SIZE == 4) {				\
            dst[dstCol*4+0] = src[srcCol*4+0];		\
            dst[dstCol*4+1] = src[srcCol*4+1];		\
            dst[dstCol*4+2] = src[srcCol*4+2];		\
            dst[dstCol*4+3] = src[srcCol*4+3];		\
         }						\
      }							\
   }							\
}

/**
 * Resamplers for 1, 2, 4, 8 and 16-byte pixels.
 */
RESAMPLE(resample_row_1, GLubyte, 1)
RESAMPLE(resample_row_2, GLushort, 1)
RESAMPLE(resample_row_4, GLuint, 1)
RESAMPLE(resample_row_8, GLuint, 2)
RESAMPLE(resample_row_16, GLuint, 4)


/**
 * Blit color, depth or stencil with GL_NEAREST filtering.
 */
static void
blit_nearest(GLcontext *ctx,
             GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
             GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
             GLbitfield buffer)
{
   struct gl_renderbuffer *readRb, *drawRb;

   const GLint srcWidth = ABS(srcX1 - srcX0);
   const GLint dstWidth = ABS(dstX1 - dstX0);
   const GLint srcHeight = ABS(srcY1 - srcY0);
   const GLint dstHeight = ABS(dstY1 - dstY0);

   const GLint srcXpos = MIN2(srcX0, srcX1);
   const GLint srcYpos = MIN2(srcY0, srcY1);
   const GLint dstXpos = MIN2(dstX0, dstX1);
   const GLint dstYpos = MIN2(dstY0, dstY1);

   const GLboolean invertX = (srcX1 < srcX0) ^ (dstX1 < dstX0);
   const GLboolean invertY = (srcY1 < srcY0) ^ (dstY1 < dstY0);

   GLint dstRow;

   GLint comps, pixelSize;
   GLvoid *srcBuffer, *dstBuffer;
   GLint prevY = -1;

   typedef void (*resample_func)(GLint srcWidth, GLint dstWidth,
                                 const GLvoid *srcBuffer, GLvoid *dstBuffer,
                                 GLboolean flip);
   resample_func resampleRow;

   switch (buffer) {
   case GL_COLOR_BUFFER_BIT:
      readRb = ctx->ReadBuffer->_ColorReadBuffer;
      drawRb = ctx->DrawBuffer->_ColorDrawBuffers[0];
      comps = 4;
      break;
   case GL_DEPTH_BUFFER_BIT:
      readRb = ctx->ReadBuffer->_DepthBuffer;
      drawRb = ctx->DrawBuffer->_DepthBuffer;
      comps = 1;
      break;
   case GL_STENCIL_BUFFER_BIT:
      readRb = ctx->ReadBuffer->_StencilBuffer;
      drawRb = ctx->DrawBuffer->_StencilBuffer;
      comps = 1;
      break;
   default:
      _mesa_problem(ctx, "unexpected buffer in blit_nearest()");
      return;
   }

   switch (readRb->DataType) {
   case GL_UNSIGNED_BYTE:
      pixelSize = comps * sizeof(GLubyte);
      break;
   case GL_UNSIGNED_SHORT:
      pixelSize = comps * sizeof(GLushort);
      break;
   case GL_UNSIGNED_INT:
      pixelSize = comps * sizeof(GLuint);
      break;
   case GL_FLOAT:
      pixelSize = comps * sizeof(GLfloat);
      break;
   default:
      _mesa_problem(ctx, "unexpected buffer type (0x%x) in blit_nearest",
                    readRb->DataType);
      return;
   }

   /* choose row resampler */
   switch (pixelSize) {
   case 1:
      resampleRow = resample_row_1;
      break;
   case 2:
      resampleRow = resample_row_2;
      break;
   case 4:
      resampleRow = resample_row_4;
      break;
   case 8:
      resampleRow = resample_row_8;
      break;
   case 16:
      resampleRow = resample_row_16;
      break;
   default:
      _mesa_problem(ctx, "unexpected pixel size (%d) in blit_nearest",
                    pixelSize);
      return;
   }

   /* allocate the src/dst row buffers */
   srcBuffer = malloc(pixelSize * srcWidth);
   if (!srcBuffer) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBlitFrameBufferEXT");
      return;
   }
   dstBuffer = malloc(pixelSize * dstWidth);
   if (!dstBuffer) {
      free(srcBuffer);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBlitFrameBufferEXT");
      return;
   }

   for (dstRow = 0; dstRow < dstHeight; dstRow++) {
      const GLint dstY = dstYpos + dstRow;
      GLint srcRow = (dstRow * srcHeight) / dstHeight;
      GLint srcY;

      ASSERT(srcRow >= 0);
      ASSERT(srcRow < srcHeight);

      if (invertY) {
         srcRow = srcHeight - 1 - srcRow;
      }

      srcY = srcYpos + srcRow;

      /* get pixel row from source and resample to match dest width */
      if (prevY != srcY) {
         readRb->GetRow(ctx, readRb, srcWidth, srcXpos, srcY, srcBuffer);
         (*resampleRow)(srcWidth, dstWidth, srcBuffer, dstBuffer, invertX);
         prevY = srcY;
      }

      /* store pixel row in destination */
      drawRb->PutRow(ctx, drawRb, dstWidth, dstXpos, dstY, dstBuffer, NULL);
   }

   free(srcBuffer);
   free(dstBuffer);
}



#define LERP(T, A, B)  ( (A) + (T) * ((B) - (A)) )

static INLINE GLfloat
lerp_2d(GLfloat a, GLfloat b,
        GLfloat v00, GLfloat v10, GLfloat v01, GLfloat v11)
{
   const GLfloat temp0 = LERP(a, v00, v10);
   const GLfloat temp1 = LERP(a, v01, v11);
   return LERP(b, temp0, temp1);
}


/**
 * Bilinear interpolation of two source rows.
 * GLubyte pixels.
 */
static void
resample_linear_row_ub(GLint srcWidth, GLint dstWidth,
                       const GLvoid *srcBuffer0, const GLvoid *srcBuffer1,
                       GLvoid *dstBuffer, GLboolean flip, GLfloat rowWeight)
{
   const GLubyte (*srcColor0)[4] = (const GLubyte (*)[4]) srcBuffer0;
   const GLubyte (*srcColor1)[4] = (const GLubyte (*)[4]) srcBuffer1;
   GLubyte (*dstColor)[4] = (GLubyte (*)[4]) dstBuffer;
   const GLfloat dstWidthF = (GLfloat) dstWidth;
   GLint dstCol;

   for (dstCol = 0; dstCol < dstWidth; dstCol++) {
      const GLfloat srcCol = (dstCol * srcWidth) / dstWidthF;
      GLint srcCol0 = IFLOOR(srcCol);
      GLint srcCol1 = srcCol0 + 1;
      GLfloat colWeight = srcCol - srcCol0; /* fractional part of srcCol */
      GLfloat red, green, blue, alpha;

      ASSERT(srcCol0 >= 0);
      ASSERT(srcCol0 < srcWidth);
      ASSERT(srcCol1 <= srcWidth);

      if (srcCol1 == srcWidth) {
         /* last column fudge */
         srcCol1--;
         colWeight = 0.0;
      }

      if (flip) {
         srcCol0 = srcWidth - 1 - srcCol0;
         srcCol1 = srcWidth - 1 - srcCol1;
      }

      red = lerp_2d(colWeight, rowWeight,
                    srcColor0[srcCol0][RCOMP], srcColor0[srcCol1][RCOMP],
                    srcColor1[srcCol0][RCOMP], srcColor1[srcCol1][RCOMP]);
      green = lerp_2d(colWeight, rowWeight,
                    srcColor0[srcCol0][GCOMP], srcColor0[srcCol1][GCOMP],
                    srcColor1[srcCol0][GCOMP], srcColor1[srcCol1][GCOMP]);
      blue = lerp_2d(colWeight, rowWeight,
                    srcColor0[srcCol0][BCOMP], srcColor0[srcCol1][BCOMP],
                    srcColor1[srcCol0][BCOMP], srcColor1[srcCol1][BCOMP]);
      alpha = lerp_2d(colWeight, rowWeight,
                    srcColor0[srcCol0][ACOMP], srcColor0[srcCol1][ACOMP],
                    srcColor1[srcCol0][ACOMP], srcColor1[srcCol1][ACOMP]);
      
      dstColor[dstCol][RCOMP] = IFLOOR(red);
      dstColor[dstCol][GCOMP] = IFLOOR(green);
      dstColor[dstCol][BCOMP] = IFLOOR(blue);
      dstColor[dstCol][ACOMP] = IFLOOR(alpha);
   }
}



/**
 * Bilinear filtered blit (color only).
 */
static void
blit_linear(GLcontext *ctx,
            GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
            GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1)
{
   struct gl_renderbuffer *readRb = ctx->ReadBuffer->_ColorReadBuffer;
   struct gl_renderbuffer *drawRb = ctx->DrawBuffer->_ColorDrawBuffers[0];

   const GLint srcWidth = ABS(srcX1 - srcX0);
   const GLint dstWidth = ABS(dstX1 - dstX0);
   const GLint srcHeight = ABS(srcY1 - srcY0);
   const GLint dstHeight = ABS(dstY1 - dstY0);
   const GLfloat dstHeightF = (GLfloat) dstHeight;

   const GLint srcXpos = MIN2(srcX0, srcX1);
   const GLint srcYpos = MIN2(srcY0, srcY1);
   const GLint dstXpos = MIN2(dstX0, dstX1);
   const GLint dstYpos = MIN2(dstY0, dstY1);

   const GLboolean invertX = (srcX1 < srcX0) ^ (dstX1 < dstX0);
   const GLboolean invertY = (srcY1 < srcY0) ^ (dstY1 < dstY0);

   GLint dstRow;

   GLint pixelSize;
   GLvoid *srcBuffer0, *srcBuffer1;
   GLint srcBufferY0 = -1, srcBufferY1 = -1;
   GLvoid *dstBuffer;

   switch (readRb->DataType) {
   case GL_UNSIGNED_BYTE:
      pixelSize = 4 * sizeof(GLubyte);
      break;
   case GL_UNSIGNED_SHORT:
      pixelSize = 4 * sizeof(GLushort);
      break;
   case GL_UNSIGNED_INT:
      pixelSize = 4 * sizeof(GLuint);
      break;
   case GL_FLOAT:
      pixelSize = 4 * sizeof(GLfloat);
      break;
   default:
      _mesa_problem(ctx, "unexpected buffer type (0x%x) in blit_nearest",
                    readRb->DataType);
      return;
   }

   /* Allocate the src/dst row buffers.
    * Keep two adjacent src rows around for bilinear sampling.
    */
   srcBuffer0 = malloc(pixelSize * srcWidth);
   if (!srcBuffer0) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBlitFrameBufferEXT");
      return;
   }
   srcBuffer1 = malloc(pixelSize * srcWidth);
   if (!srcBuffer1) {
      free(srcBuffer0);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBlitFrameBufferEXT");
      return;
   }
   dstBuffer = malloc(pixelSize * dstWidth);
   if (!dstBuffer) {
      free(srcBuffer0);
      free(srcBuffer1);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBlitFrameBufferEXT");
      return;
   }

   for (dstRow = 0; dstRow < dstHeight; dstRow++) {
      const GLint dstY = dstYpos + dstRow;
      const GLfloat srcRow = (dstRow * srcHeight) / dstHeightF;
      GLint srcRow0 = IFLOOR(srcRow);
      GLint srcRow1 = srcRow0 + 1;
      GLfloat rowWeight = srcRow - srcRow0; /* fractional part of srcRow */

      ASSERT(srcRow >= 0);
      ASSERT(srcRow < srcHeight);

      if (srcRow1 == srcHeight) {
         /* last row fudge */
         srcRow1 = srcRow0;
         rowWeight = 0.0;
      }

      if (invertY) {
         srcRow0 = srcHeight - 1 - srcRow0;
         srcRow1 = srcHeight - 1 - srcRow1;
      }

      srcY0 = srcYpos + srcRow0;
      srcY1 = srcYpos + srcRow1;

      /* get the two source rows */
      if (srcY0 == srcBufferY0 && srcY1 == srcBufferY1) {
         /* use same source row buffers again */
      }
      else if (srcY0 == srcBufferY1) {
         /* move buffer1 into buffer0 by swapping pointers */
         GLvoid *tmp = srcBuffer0;
         srcBuffer0 = srcBuffer1;
         srcBuffer1 = tmp;
         /* get y1 row */
         readRb->GetRow(ctx, readRb, srcWidth, srcXpos, srcY1, srcBuffer1);
         srcBufferY0 = srcY0;
         srcBufferY1 = srcY1;
      }
      else {
         /* get both new rows */
         readRb->GetRow(ctx, readRb, srcWidth, srcXpos, srcY0, srcBuffer0);
         readRb->GetRow(ctx, readRb, srcWidth, srcXpos, srcY1, srcBuffer1);
         srcBufferY0 = srcY0;
         srcBufferY1 = srcY1;
      }

      if (readRb->DataType == GL_UNSIGNED_BYTE) {
         resample_linear_row_ub(srcWidth, dstWidth, srcBuffer0, srcBuffer1,
                                dstBuffer, invertX, rowWeight);
      }
      else {
         _mesa_problem(ctx, "Unsupported color channel type in sw blit");
         break;
      }

      /* store pixel row in destination */
      drawRb->PutRow(ctx, drawRb, dstWidth, dstXpos, dstY, dstBuffer, NULL);
   }

   free(srcBuffer0);
   free(srcBuffer1);
   free(dstBuffer);
}


/**
 * Simple case:  Blit color, depth or stencil with no scaling or flipping.
 * XXX we could easily support vertical flipping here.
 */
static void
simple_blit(GLcontext *ctx,
            GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
            GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
            GLbitfield buffer)
{
   struct gl_renderbuffer *readRb, *drawRb;
   const GLint width = srcX1 - srcX0;
   const GLint height = srcY1 - srcY0;
   GLint row, srcY, dstY, yStep;
   GLint comps, bytesPerRow;
   void *rowBuffer;

   /* only one buffer */
   ASSERT(_mesa_bitcount(buffer) == 1);
   /* no flipping checks */
   ASSERT(srcX0 < srcX1);
   ASSERT(srcY0 < srcY1);
   ASSERT(dstX0 < dstX1);
   ASSERT(dstY0 < dstY1);
   /* size checks */
   ASSERT(srcX1 - srcX0 == dstX1 - dstX0);
   ASSERT(srcY1 - srcY0 == dstY1 - dstY0);

   /* determine if copy should be bottom-to-top or top-to-bottom */
   if (srcY0 > dstY0) {
      /* src above dst: copy bottom-to-top */
      yStep = 1;
      srcY = srcY0;
      dstY = dstY0;
   }
   else {
      /* src below dst: copy top-to-bottom */
      yStep = -1;
      srcY = srcY1 - 1;
      dstY = dstY1 - 1;
   }

   switch (buffer) {
   case GL_COLOR_BUFFER_BIT:
      readRb = ctx->ReadBuffer->_ColorReadBuffer;
      drawRb = ctx->DrawBuffer->_ColorDrawBuffers[0];
      comps = 4;
      break;
   case GL_DEPTH_BUFFER_BIT:
      readRb = ctx->ReadBuffer->_DepthBuffer;
      drawRb = ctx->DrawBuffer->_DepthBuffer;
      comps = 1;
      break;
   case GL_STENCIL_BUFFER_BIT:
      readRb = ctx->ReadBuffer->_StencilBuffer;
      drawRb = ctx->DrawBuffer->_StencilBuffer;
      comps = 1;
      break;
   default:
      _mesa_problem(ctx, "unexpected buffer in simple_blit()");
      return;
   }

   ASSERT(readRb->DataType == drawRb->DataType);

   /* compute bytes per row */
   switch (readRb->DataType) {
   case GL_UNSIGNED_BYTE:
      bytesPerRow = comps * width * sizeof(GLubyte);
      break;
   case GL_UNSIGNED_SHORT:
      bytesPerRow = comps * width * sizeof(GLushort);
      break;
   case GL_UNSIGNED_INT:
      bytesPerRow = comps * width * sizeof(GLuint);
      break;
   case GL_FLOAT:
      bytesPerRow = comps * width * sizeof(GLfloat);
      break;
   default:
      _mesa_problem(ctx, "unexpected buffer type in simple_blit");
      return;
   }

   /* allocate the row buffer */
   rowBuffer = malloc(bytesPerRow);
   if (!rowBuffer) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBlitFrameBufferEXT");
      return;
   }

   for (row = 0; row < height; row++) {
      readRb->GetRow(ctx, readRb, width, srcX0, srcY, rowBuffer);
      drawRb->PutRow(ctx, drawRb, width, dstX0, dstY, rowBuffer, NULL);
      srcY += yStep;
      dstY += yStep;
   }

   free(rowBuffer);
}


/**
 * Software fallback for glBlitFramebufferEXT().
 */
void
_swrast_BlitFramebuffer(GLcontext *ctx,
                        GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                        GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                        GLbitfield mask, GLenum filter)
{
   static const GLbitfield buffers[3] = {
      GL_COLOR_BUFFER_BIT,
      GL_DEPTH_BUFFER_BIT,
      GL_STENCIL_BUFFER_BIT
   };
   GLint i;

   if (!_mesa_check_conditional_render(ctx))
      return; /* don't clear */

   if (!ctx->DrawBuffer->_NumColorDrawBuffers)
      return;

   if (!_mesa_clip_blit(ctx, &srcX0, &srcY0, &srcX1, &srcY1,
                        &dstX0, &dstY0, &dstX1, &dstY1)) {
      return;
   }

   swrast_render_start(ctx);

   if (srcX1 - srcX0 == dstX1 - dstX0 &&
       srcY1 - srcY0 == dstY1 - dstY0 &&
       srcX0 < srcX1 &&
       srcY0 < srcY1 &&
       dstX0 < dstX1 &&
       dstY0 < dstY1) {
      /* no stretching or flipping.
       * filter doesn't matter.
       */
      for (i = 0; i < 3; i++) {
         if (mask & buffers[i]) {
            simple_blit(ctx, srcX0, srcY0, srcX1, srcY1,
                        dstX0, dstY0, dstX1, dstY1, buffers[i]);
         }
      }
   }
   else {
      if (filter == GL_NEAREST) {
         for (i = 0; i < 3; i++) {
            if (mask & buffers[i]) {
               blit_nearest(ctx,  srcX0, srcY0, srcX1, srcY1,
                            dstX0, dstY0, dstX1, dstY1, buffers[i]);
            }
         }
      }
      else {
         ASSERT(filter == GL_LINEAR);
         if (mask & GL_COLOR_BUFFER_BIT) {  /* depth/stencil not allowed */
            blit_linear(ctx,  srcX0, srcY0, srcX1, srcY1,
                        dstX0, dstY0, dstX1, dstY1);
         }
      }
   }

   swrast_render_finish(ctx);
}
