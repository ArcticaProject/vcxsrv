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

#include "glheader.h"
#include "imports.h"
#include "context.h"
#include "formats.h"
#include "mtypes.h"
#include "depthstencil.h"
#include "renderbuffer.h"


/**
 * Adaptor/wrappers for GL_DEPTH_STENCIL renderbuffers.
 *
 * The problem with a GL_DEPTH_STENCIL renderbuffer is that sometimes we
 * want to treat it as a stencil buffer, other times we want to treat it
 * as a depth/z buffer and still other times when we want to treat it as
 * a combined Z+stencil buffer!  That implies we need three different sets
 * of Get/Put functions.
 *
 * We solve this by wrapping the Z24_S8 or S8_Z24 renderbuffer with depth and
 * stencil adaptors, each with the right kind of depth/stencil Get/Put functions.
 */


static void *
nop_get_pointer(struct gl_context *ctx, struct gl_renderbuffer *rb, GLint x, GLint y)
{
   (void) ctx;
   (void) rb;
   (void) x;
   (void) y;
   return NULL;
}


/**
 * Delete a depth or stencil wrapper renderbuffer.
 */
static void
delete_wrapper(struct gl_renderbuffer *rb)
{
   ASSERT(rb->Format == MESA_FORMAT_S8 ||
          rb->Format == MESA_FORMAT_X8_Z24);
   _mesa_reference_renderbuffer(&rb->Wrapped, NULL);
   free(rb);
}


/**
 * Realloc storage for wrapper.
 */
static GLboolean
alloc_wrapper_storage(struct gl_context *ctx, struct gl_renderbuffer *rb,
                      GLenum internalFormat, GLuint width, GLuint height)
{
   /* just pass this on to the wrapped renderbuffer */
   struct gl_renderbuffer *dsrb = rb->Wrapped;
   GLboolean retVal;

   (void) internalFormat;

   ASSERT(dsrb->Format == MESA_FORMAT_Z24_S8 ||
          dsrb->Format == MESA_FORMAT_Z24_X8 ||
          dsrb->Format == MESA_FORMAT_S8_Z24 ||
          dsrb->Format == MESA_FORMAT_X8_Z24);

   retVal = dsrb->AllocStorage(ctx, dsrb, dsrb->InternalFormat, width, height);
   if (retVal) {
      rb->Width = width;
      rb->Height = height;
      rb->RowStride = dsrb->RowStride;
   }
   return retVal;
}




/*======================================================================
 * Depth wrapper around depth/stencil renderbuffer
 */

static void
get_row_z24(struct gl_context *ctx, struct gl_renderbuffer *z24rb, GLuint count,
            GLint x, GLint y, void *values)
{
   struct gl_renderbuffer *dsrb = z24rb->Wrapped;
   GLuint temp[MAX_WIDTH], i;
   GLuint *dst = (GLuint *) values;
   const GLuint *src = (const GLuint *) dsrb->GetPointer(ctx, dsrb, x, y);
   ASSERT(z24rb->DataType == GL_UNSIGNED_INT);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   if (!src) {
      dsrb->GetRow(ctx, dsrb, count, x, y, temp);
      src = temp;
   }
   if (dsrb->Format == MESA_FORMAT_Z24_S8) {
      for (i = 0; i < count; i++) {
         dst[i] = src[i] >> 8;
      }
   }
   else {
      assert(dsrb->Format == MESA_FORMAT_S8_Z24);
      for (i = 0; i < count; i++) {
         dst[i] = src[i] & 0xffffff;
      }
   }
}

static void
get_values_z24(struct gl_context *ctx, struct gl_renderbuffer *z24rb, GLuint count,
               const GLint x[], const GLint y[], void *values)
{
   struct gl_renderbuffer *dsrb = z24rb->Wrapped;
   GLuint temp[MAX_WIDTH], i;
   GLuint *dst = (GLuint *) values;
   ASSERT(z24rb->DataType == GL_UNSIGNED_INT);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   ASSERT(count <= MAX_WIDTH);
   /* don't bother trying direct access */
   dsrb->GetValues(ctx, dsrb, count, x, y, temp);
   if (dsrb->Format == MESA_FORMAT_Z24_S8) {
      for (i = 0; i < count; i++) {
         dst[i] = temp[i] >> 8;
      }
   }
   else {
      assert(dsrb->Format == MESA_FORMAT_S8_Z24);
      for (i = 0; i < count; i++) {
         dst[i] = temp[i] & 0xffffff;
      }
   }
}

static void
put_row_z24(struct gl_context *ctx, struct gl_renderbuffer *z24rb, GLuint count,
            GLint x, GLint y, const void *values, const GLubyte *mask)
{
   struct gl_renderbuffer *dsrb = z24rb->Wrapped;
   const GLuint *src = (const GLuint *) values;
   GLuint *dst = (GLuint *) dsrb->GetPointer(ctx, dsrb, x, y);
   ASSERT(z24rb->DataType == GL_UNSIGNED_INT);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   if (dst) {
      /* direct access */
      GLuint i;
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               dst[i] = (src[i] << 8) | (dst[i] & 0xff);
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               dst[i] = (src[i] & 0xffffff) | (dst[i] & 0xff000000);
            }
         }
      }
   }
   else {
      /* get, modify, put */
      GLuint temp[MAX_WIDTH], i;
      dsrb->GetRow(ctx, dsrb, count, x, y, temp);
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (src[i] << 8) | (temp[i] & 0xff);
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (src[i] & 0xffffff) | (temp[i] & 0xff000000);
            }
         }
      }
      dsrb->PutRow(ctx, dsrb, count, x, y, temp, mask);
   }
}

static void
put_mono_row_z24(struct gl_context *ctx, struct gl_renderbuffer *z24rb, GLuint count,
                 GLint x, GLint y, const void *value, const GLubyte *mask)
{
   struct gl_renderbuffer *dsrb = z24rb->Wrapped;
   GLuint *dst = (GLuint *) dsrb->GetPointer(ctx, dsrb, x, y);
   ASSERT(z24rb->DataType == GL_UNSIGNED_INT);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   if (dst) {
      /* direct access */
      GLuint i;
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         const GLuint shiftedVal = *((GLuint *) value) << 8;
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               dst[i] = shiftedVal | (dst[i] & 0xff);
            }
         }
      }
      else {
         const GLuint shiftedVal = *((GLuint *) value);
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               dst[i] = shiftedVal | (dst[i] & 0xff000000);
            }
         }
      }
   }
   else {
      /* get, modify, put */
      GLuint temp[MAX_WIDTH], i;
      dsrb->GetRow(ctx, dsrb, count, x, y, temp);
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         const GLuint shiftedVal = *((GLuint *) value) << 8;
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = shiftedVal | (temp[i] & 0xff);
            }
         }
      }
      else {
         const GLuint shiftedVal = *((GLuint *) value);
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = shiftedVal | (temp[i] & 0xff000000);
            }
         }
      }
      dsrb->PutRow(ctx, dsrb, count, x, y, temp, mask);
   }
}

static void
put_values_z24(struct gl_context *ctx, struct gl_renderbuffer *z24rb, GLuint count,
               const GLint x[], const GLint y[],
               const void *values, const GLubyte *mask)
{
   struct gl_renderbuffer *dsrb = z24rb->Wrapped;
   const GLuint *src = (const GLuint *) values;
   ASSERT(z24rb->DataType == GL_UNSIGNED_INT);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   if (dsrb->GetPointer(ctx, dsrb, 0, 0)) {
      /* direct access */
      GLuint i;
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               GLuint *dst = (GLuint *) dsrb->GetPointer(ctx, dsrb, x[i], y[i]);
               *dst = (src[i] << 8) | (*dst & 0xff);
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               GLuint *dst = (GLuint *) dsrb->GetPointer(ctx, dsrb, x[i], y[i]);
               *dst = (src[i] & 0xffffff) | (*dst & 0xff000000);
            }
         }
      }
   }
   else {
      /* get, modify, put */
      GLuint temp[MAX_WIDTH], i;
      dsrb->GetValues(ctx, dsrb, count, x, y, temp);
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (src[i] << 8) | (temp[i] & 0xff);
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (src[i] & 0xffffff) | (temp[i] & 0xff000000);
            }
         }
      }
      dsrb->PutValues(ctx, dsrb, count, x, y, temp, mask);
   }
}

static void
put_mono_values_z24(struct gl_context *ctx, struct gl_renderbuffer *z24rb,
                    GLuint count, const GLint x[], const GLint y[],
                    const void *value, const GLubyte *mask)
{
   struct gl_renderbuffer *dsrb = z24rb->Wrapped;
   GLuint temp[MAX_WIDTH], i;
   /* get, modify, put */
   dsrb->GetValues(ctx, dsrb, count, x, y, temp);
   if (dsrb->Format == MESA_FORMAT_Z24_S8) {
      const GLuint shiftedVal = *((GLuint *) value) << 8;
      for (i = 0; i < count; i++) {
         if (!mask || mask[i]) {
            temp[i] = shiftedVal | (temp[i] & 0xff);
         }
      }
   }
   else {
      const GLuint shiftedVal = *((GLuint *) value);
      assert(dsrb->Format == MESA_FORMAT_S8_Z24);
      for (i = 0; i < count; i++) {
         if (!mask || mask[i]) {
            temp[i] = shiftedVal | (temp[i] & 0xff000000);
         }
      }
   }
   dsrb->PutValues(ctx, dsrb, count, x, y, temp, mask);
}


/**
 * Wrap the given GL_DEPTH_STENCIL renderbuffer so that it acts like
 * a depth renderbuffer.
 * \return new depth renderbuffer
 */
struct gl_renderbuffer *
_mesa_new_z24_renderbuffer_wrapper(struct gl_context *ctx,
                                   struct gl_renderbuffer *dsrb)
{
   struct gl_renderbuffer *z24rb;

   ASSERT(dsrb->Format == MESA_FORMAT_Z24_S8 ||
          dsrb->Format == MESA_FORMAT_Z24_X8 ||
          dsrb->Format == MESA_FORMAT_S8_Z24 ||
          dsrb->Format == MESA_FORMAT_X8_Z24);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);

   z24rb = ctx->Driver.NewRenderbuffer(ctx, 0);
   if (!z24rb)
      return NULL;

   /* NOTE: need to do manual refcounting here */
   z24rb->Wrapped = dsrb;
   dsrb->RefCount++;

   z24rb->Name = dsrb->Name;
   z24rb->RefCount = 0;
   z24rb->Width = dsrb->Width;
   z24rb->Height = dsrb->Height;
   z24rb->RowStride = dsrb->RowStride;
   z24rb->InternalFormat = GL_DEPTH_COMPONENT24;
   z24rb->Format = MESA_FORMAT_X8_Z24;
   z24rb->_BaseFormat = GL_DEPTH_COMPONENT;
   z24rb->DataType = GL_UNSIGNED_INT;
   z24rb->Data = NULL;
   z24rb->Delete = delete_wrapper;
   z24rb->AllocStorage = alloc_wrapper_storage;
   z24rb->GetPointer = nop_get_pointer;
   z24rb->GetRow = get_row_z24;
   z24rb->GetValues = get_values_z24;
   z24rb->PutRow = put_row_z24;
   z24rb->PutRowRGB = NULL;
   z24rb->PutMonoRow = put_mono_row_z24;
   z24rb->PutValues = put_values_z24;
   z24rb->PutMonoValues = put_mono_values_z24;

   return z24rb;
}


/*======================================================================
 * Stencil wrapper around depth/stencil renderbuffer
 */

static void
get_row_s8(struct gl_context *ctx, struct gl_renderbuffer *s8rb, GLuint count,
           GLint x, GLint y, void *values)
{
   struct gl_renderbuffer *dsrb = s8rb->Wrapped;
   GLuint temp[MAX_WIDTH], i;
   GLubyte *dst = (GLubyte *) values;
   const GLuint *src = (const GLuint *) dsrb->GetPointer(ctx, dsrb, x, y);
   ASSERT(s8rb->DataType == GL_UNSIGNED_BYTE);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   if (!src) {
      dsrb->GetRow(ctx, dsrb, count, x, y, temp);
      src = temp;
   }
   if (dsrb->Format == MESA_FORMAT_Z24_S8) {
      for (i = 0; i < count; i++) {
         dst[i] = src[i] & 0xff;
      }
   }
   else {
      assert(dsrb->Format == MESA_FORMAT_S8_Z24);
      for (i = 0; i < count; i++) {
         dst[i] = src[i] >> 24;
      }
   }
}

static void
get_values_s8(struct gl_context *ctx, struct gl_renderbuffer *s8rb, GLuint count,
              const GLint x[], const GLint y[], void *values)
{
   struct gl_renderbuffer *dsrb = s8rb->Wrapped;
   GLuint temp[MAX_WIDTH], i;
   GLubyte *dst = (GLubyte *) values;
   ASSERT(s8rb->DataType == GL_UNSIGNED_BYTE);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   ASSERT(count <= MAX_WIDTH);
   /* don't bother trying direct access */
   dsrb->GetValues(ctx, dsrb, count, x, y, temp);
   if (dsrb->Format == MESA_FORMAT_Z24_S8) {
      for (i = 0; i < count; i++) {
         dst[i] = temp[i] & 0xff;
      }
   }
   else {
      assert(dsrb->Format == MESA_FORMAT_S8_Z24);
      for (i = 0; i < count; i++) {
         dst[i] = temp[i] >> 24;
      }
   }
}

static void
put_row_s8(struct gl_context *ctx, struct gl_renderbuffer *s8rb, GLuint count,
           GLint x, GLint y, const void *values, const GLubyte *mask)
{
   struct gl_renderbuffer *dsrb = s8rb->Wrapped;
   const GLubyte *src = (const GLubyte *) values;
   GLuint *dst = (GLuint *) dsrb->GetPointer(ctx, dsrb, x, y);
   ASSERT(s8rb->DataType == GL_UNSIGNED_BYTE);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   if (dst) {
      /* direct access */
      GLuint i;
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               dst[i] = (dst[i] & 0xffffff00) | src[i];
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               dst[i] = (dst[i] & 0xffffff) | (src[i] << 24);
            }
         }
      }
   }
   else {
      /* get, modify, put */
      GLuint temp[MAX_WIDTH], i;
      dsrb->GetRow(ctx, dsrb, count, x, y, temp);
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (temp[i] & 0xffffff00) | src[i];
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (temp[i] & 0xffffff) | (src[i] << 24);
            }
         }
      }
      dsrb->PutRow(ctx, dsrb, count, x, y, temp, mask);
   }
}

static void
put_mono_row_s8(struct gl_context *ctx, struct gl_renderbuffer *s8rb, GLuint count,
                GLint x, GLint y, const void *value, const GLubyte *mask)
{
   struct gl_renderbuffer *dsrb = s8rb->Wrapped;
   const GLubyte val = *((GLubyte *) value);
   GLuint *dst = (GLuint *) dsrb->GetPointer(ctx, dsrb, x, y);
   ASSERT(s8rb->DataType == GL_UNSIGNED_BYTE);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   if (dst) {
      /* direct access */
      GLuint i;
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               dst[i] = (dst[i] & 0xffffff00) | val;
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               dst[i] = (dst[i] & 0xffffff) | (val << 24);
            }
         }
      }
   }
   else {
      /* get, modify, put */
      GLuint temp[MAX_WIDTH], i;
      dsrb->GetRow(ctx, dsrb, count, x, y, temp);
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (temp[i] & 0xffffff00) | val;
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (temp[i] & 0xffffff) | (val << 24);
            }
         }
      }
      dsrb->PutRow(ctx, dsrb, count, x, y, temp, mask);
   }
}

static void
put_values_s8(struct gl_context *ctx, struct gl_renderbuffer *s8rb, GLuint count,
              const GLint x[], const GLint y[],
              const void *values, const GLubyte *mask)
{
   struct gl_renderbuffer *dsrb = s8rb->Wrapped;
   const GLubyte *src = (const GLubyte *) values;
   ASSERT(s8rb->DataType == GL_UNSIGNED_BYTE);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   if (dsrb->GetPointer(ctx, dsrb, 0, 0)) {
      /* direct access */
      GLuint i;
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               GLuint *dst = (GLuint *) dsrb->GetPointer(ctx, dsrb, x[i], y[i]);
               *dst = (*dst & 0xffffff00) | src[i];
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               GLuint *dst = (GLuint *) dsrb->GetPointer(ctx, dsrb, x[i], y[i]);
               *dst = (*dst & 0xffffff) | (src[i] << 24);
            }
         }
      }
   }
   else {
      /* get, modify, put */
      GLuint temp[MAX_WIDTH], i;
      dsrb->GetValues(ctx, dsrb, count, x, y, temp);
      if (dsrb->Format == MESA_FORMAT_Z24_S8) {
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (temp[i] & 0xffffff00) | src[i];
            }
         }
      }
      else {
         assert(dsrb->Format == MESA_FORMAT_S8_Z24);
         for (i = 0; i < count; i++) {
            if (!mask || mask[i]) {
               temp[i] = (temp[i] & 0xffffff) | (src[i] << 24);
            }
         }
      }
      dsrb->PutValues(ctx, dsrb, count, x, y, temp, mask);
   }
}

static void
put_mono_values_s8(struct gl_context *ctx, struct gl_renderbuffer *s8rb, GLuint count,
                   const GLint x[], const GLint y[],
                   const void *value, const GLubyte *mask)
{
   struct gl_renderbuffer *dsrb = s8rb->Wrapped;
   GLuint temp[MAX_WIDTH], i;
   const GLubyte val = *((GLubyte *) value);
   /* get, modify, put */
   dsrb->GetValues(ctx, dsrb, count, x, y, temp);
   if (dsrb->Format == MESA_FORMAT_Z24_S8) {
      for (i = 0; i < count; i++) {
         if (!mask || mask[i]) {
            temp[i] = (temp[i] & 0xffffff00) | val;
         }
      }
   }
   else {
      assert(dsrb->Format == MESA_FORMAT_S8_Z24);
      for (i = 0; i < count; i++) {
         if (!mask || mask[i]) {
            temp[i] = (temp[i] & 0xffffff) | (val << 24);
         }
      }
   }
   dsrb->PutValues(ctx, dsrb, count, x, y, temp, mask);
}


/**
 * Wrap the given GL_DEPTH_STENCIL renderbuffer so that it acts like
 * a stencil renderbuffer.
 * \return new stencil renderbuffer
 */
struct gl_renderbuffer *
_mesa_new_s8_renderbuffer_wrapper(struct gl_context *ctx, struct gl_renderbuffer *dsrb)
{
   struct gl_renderbuffer *s8rb;

   ASSERT(dsrb->Format == MESA_FORMAT_Z24_S8 ||
          dsrb->Format == MESA_FORMAT_S8_Z24);
   ASSERT(dsrb->DataType == GL_UNSIGNED_INT_24_8_EXT);

   s8rb = ctx->Driver.NewRenderbuffer(ctx, 0);
   if (!s8rb)
      return NULL;

   /* NOTE: need to do manual refcounting here */
   s8rb->Wrapped = dsrb;
   dsrb->RefCount++;

   s8rb->Name = dsrb->Name;
   s8rb->RefCount = 0;
   s8rb->Width = dsrb->Width;
   s8rb->Height = dsrb->Height;
   s8rb->RowStride = dsrb->RowStride;
   s8rb->InternalFormat = GL_STENCIL_INDEX8_EXT;
   s8rb->Format = MESA_FORMAT_S8;
   s8rb->_BaseFormat = GL_STENCIL_INDEX;
   s8rb->DataType = GL_UNSIGNED_BYTE;
   s8rb->Data = NULL;
   s8rb->Delete = delete_wrapper;
   s8rb->AllocStorage = alloc_wrapper_storage;
   s8rb->GetPointer = nop_get_pointer;
   s8rb->GetRow = get_row_s8;
   s8rb->GetValues = get_values_s8;
   s8rb->PutRow = put_row_s8;
   s8rb->PutRowRGB = NULL;
   s8rb->PutMonoRow = put_mono_row_s8;
   s8rb->PutValues = put_values_s8;
   s8rb->PutMonoValues = put_mono_values_s8;

   return s8rb;
}



/**
 ** The following functions are useful for hardware drivers that only
 ** implement combined depth/stencil buffers.
 ** The GL_EXT_framebuffer_object extension allows indepedent depth and
 ** stencil buffers to be used in any combination.
 ** Therefore, we sometimes have to merge separate depth and stencil
 ** renderbuffers into a single depth+stencil renderbuffer.  And sometimes
 ** we have to split combined depth+stencil renderbuffers into separate
 ** renderbuffers.
 **/


/**
 * Extract stencil values from the combined depth/stencil renderbuffer, storing
 * the values into a separate stencil renderbuffer.
 * \param dsRb  the source depth/stencil renderbuffer
 * \param stencilRb  the destination stencil renderbuffer
 *                   (either 8-bit or 32-bit)
 */
void
_mesa_extract_stencil(struct gl_context *ctx,
                      struct gl_renderbuffer *dsRb,
                      struct gl_renderbuffer *stencilRb)
{
   GLuint row, width, height;

   ASSERT(dsRb);
   ASSERT(stencilRb);

   ASSERT(dsRb->Format == MESA_FORMAT_Z24_S8);
   ASSERT(dsRb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   ASSERT(stencilRb->Format == MESA_FORMAT_Z24_S8 ||
          stencilRb->Format == MESA_FORMAT_S8);
   ASSERT(dsRb->Width == stencilRb->Width);
   ASSERT(dsRb->Height == stencilRb->Height);

   width = dsRb->Width;
   height = dsRb->Height;

   for (row = 0; row < height; row++) {
      GLuint depthStencil[MAX_WIDTH];
      dsRb->GetRow(ctx, dsRb, width, 0, row, depthStencil);
      if (stencilRb->Format == MESA_FORMAT_S8) {
         /* 8bpp stencil */
         GLubyte stencil[MAX_WIDTH];
         GLuint i;
         for (i = 0; i < width; i++) {
            stencil[i] = depthStencil[i] & 0xff;
         }
         stencilRb->PutRow(ctx, stencilRb, width, 0, row, stencil, NULL);
      }
      else {
         /* 32bpp stencil */
         /* the 24 depth bits will be ignored */
         ASSERT(stencilRb->Format == MESA_FORMAT_Z24_S8);
         ASSERT(stencilRb->DataType == GL_UNSIGNED_INT_24_8_EXT);
         stencilRb->PutRow(ctx, stencilRb, width, 0, row, depthStencil, NULL);
      }
   }
}


/**
 * Copy stencil values from a stencil renderbuffer into a combined
 * depth/stencil renderbuffer.
 * \param dsRb  the destination depth/stencil renderbuffer
 * \param stencilRb  the source stencil buffer (either 8-bit or 32-bit)
 */
void
_mesa_insert_stencil(struct gl_context *ctx,
                     struct gl_renderbuffer *dsRb,
                     struct gl_renderbuffer *stencilRb)
{
   GLuint row, width, height;

   ASSERT(dsRb);
   ASSERT(stencilRb);

   ASSERT(dsRb->Format == MESA_FORMAT_Z24_S8);
   ASSERT(dsRb->DataType == GL_UNSIGNED_INT_24_8_EXT);
   ASSERT(stencilRb->Format == MESA_FORMAT_Z24_S8 ||
          stencilRb->Format == MESA_FORMAT_S8);

   ASSERT(dsRb->Width == stencilRb->Width);
   ASSERT(dsRb->Height == stencilRb->Height);

   width = dsRb->Width;
   height = dsRb->Height;

   for (row = 0; row < height; row++) {
      GLuint depthStencil[MAX_WIDTH];

      dsRb->GetRow(ctx, dsRb, width, 0, row, depthStencil);

      if (stencilRb->Format == MESA_FORMAT_S8) {
         /* 8bpp stencil */
         GLubyte stencil[MAX_WIDTH];
         GLuint i;
         stencilRb->GetRow(ctx, stencilRb, width, 0, row, stencil);
         for (i = 0; i < width; i++) {
            depthStencil[i] = (depthStencil[i] & 0xffffff00) | stencil[i];
         }
      }
      else {
         /* 32bpp stencil buffer */
         GLuint stencil[MAX_WIDTH], i;
         ASSERT(stencilRb->Format == MESA_FORMAT_Z24_S8);
         ASSERT(stencilRb->DataType == GL_UNSIGNED_INT_24_8_EXT);
         stencilRb->GetRow(ctx, stencilRb, width, 0, row, stencil);
         for (i = 0; i < width; i++) {
            depthStencil[i]
               = (depthStencil[i] & 0xffffff00) | (stencil[i] & 0xff);
         }
      }

      dsRb->PutRow(ctx, dsRb, width, 0, row, depthStencil, NULL);
   }
}


/**
 * Convert the stencil buffer from 8bpp to 32bpp depth/stencil.
 * \param stencilRb  the stencil renderbuffer to promote
 */
void
_mesa_promote_stencil(struct gl_context *ctx, struct gl_renderbuffer *stencilRb)
{
   const GLsizei width = stencilRb->Width;
   const GLsizei height = stencilRb->Height;
   GLubyte *data;
   GLint i, j, k;

   ASSERT(stencilRb->Format == MESA_FORMAT_S8);
   ASSERT(stencilRb->Data);

   data = (GLubyte *) stencilRb->Data;
   stencilRb->Data = NULL;
   stencilRb->AllocStorage(ctx, stencilRb, GL_DEPTH24_STENCIL8_EXT,
                           width, height);

   ASSERT(stencilRb->DataType == GL_UNSIGNED_INT_24_8_EXT);

   k = 0;
   for (i = 0; i < height; i++) {
      GLuint depthStencil[MAX_WIDTH];
      for (j = 0; j < width; j++) {
         depthStencil[j] = data[k++];
      }
      stencilRb->PutRow(ctx, stencilRb, width, 0, i, depthStencil, NULL);
   }
   free(data);
}
