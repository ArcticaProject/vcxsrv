/*
 * Mesa 3-D graphics library
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * Functions for allocating/managing software-based renderbuffers.
 * Also, routines for reading/writing software-based renderbuffer data as
 * ubytes, ushorts, uints, etc.
 */


#include "main/glheader.h"
#include "main/imports.h"
#include "main/context.h"
#include "main/fbobject.h"
#include "main/formats.h"
#include "main/mtypes.h"
#include "main/renderbuffer.h"
#include "swrast/s_context.h"
#include "swrast/s_renderbuffer.h"


/**
 * This is a software fallback for the gl_renderbuffer->AllocStorage
 * function.
 * Device drivers will typically override this function for the buffers
 * which it manages (typically color buffers, Z and stencil).
 * Other buffers (like software accumulation and aux buffers) which the driver
 * doesn't manage can be handled with this function.
 *
 * This one multi-purpose function can allocate stencil, depth, accum, color
 * or color-index buffers!
 */
static GLboolean
soft_renderbuffer_storage(struct gl_context *ctx, struct gl_renderbuffer *rb,
                          GLenum internalFormat,
                          GLuint width, GLuint height)
{
   struct swrast_renderbuffer *srb = swrast_renderbuffer(rb);
   GLuint bpp;

   switch (internalFormat) {
   case GL_RGB:
   case GL_R3_G3_B2:
   case GL_RGB4:
   case GL_RGB5:
   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      rb->Format = MESA_FORMAT_BGR_UNORM8;
      break;
   case GL_RGBA:
   case GL_RGBA2:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
#if 1
   case GL_RGB10_A2:
   case GL_RGBA12:
#endif
      if (_mesa_little_endian())
         rb->Format = MESA_FORMAT_R8G8B8A8_UNORM;
      else
         rb->Format = MESA_FORMAT_A8B8G8R8_UNORM;
      break;
   case GL_RGBA16:
   case GL_RGBA16_SNORM:
      /* for accum buffer */
      rb->Format = MESA_FORMAT_RGBA_SNORM16;
      break;
   case GL_STENCIL_INDEX:
   case GL_STENCIL_INDEX1_EXT:
   case GL_STENCIL_INDEX4_EXT:
   case GL_STENCIL_INDEX8_EXT:
   case GL_STENCIL_INDEX16_EXT:
      rb->Format = MESA_FORMAT_S_UINT8;
      break;
   case GL_DEPTH_COMPONENT:
   case GL_DEPTH_COMPONENT16:
      rb->Format = MESA_FORMAT_Z_UNORM16;
      break;
   case GL_DEPTH_COMPONENT24:
      rb->Format = MESA_FORMAT_Z24_UNORM_X8_UINT;
      break;
   case GL_DEPTH_COMPONENT32:
      rb->Format = MESA_FORMAT_Z_UNORM32;
      break;
   case GL_DEPTH_STENCIL_EXT:
   case GL_DEPTH24_STENCIL8_EXT:
      rb->Format = MESA_FORMAT_S8_UINT_Z24_UNORM;
      break;
   default:
      /* unsupported format */
      return GL_FALSE;
   }

   bpp = _mesa_get_format_bytes(rb->Format);

   /* free old buffer storage */
   free(srb->Buffer);
   srb->Buffer = NULL;

   srb->RowStride = width * bpp;

   if (width > 0 && height > 0) {
      /* allocate new buffer storage */
      srb->Buffer = malloc(srb->RowStride * height);

      if (srb->Buffer == NULL) {
         rb->Width = 0;
         rb->Height = 0;
         _mesa_error(ctx, GL_OUT_OF_MEMORY,
                     "software renderbuffer allocation (%d x %d x %d)",
                     width, height, bpp);
         return GL_FALSE;
      }
   }

   rb->Width = width;
   rb->Height = height;
   rb->_BaseFormat = _mesa_base_fbo_format(ctx, internalFormat);

   if (rb->Name == 0 &&
       internalFormat == GL_RGBA16_SNORM &&
       rb->_BaseFormat == 0) {
      /* NOTE: This is a special case just for accumulation buffers.
       * This is a very limited use case- there's no snorm texturing or
       * rendering going on.
       */
      rb->_BaseFormat = GL_RGBA;
   }
   else {
      /* the internalFormat should have been error checked long ago */
      assert(rb->_BaseFormat);
   }

   return GL_TRUE;
}


/**
 * Called via gl_renderbuffer::Delete()
 */
static void
soft_renderbuffer_delete(struct gl_context *ctx, struct gl_renderbuffer *rb)
{
   struct swrast_renderbuffer *srb = swrast_renderbuffer(rb);

   free(srb->Buffer);
   srb->Buffer = NULL;
   _mesa_delete_renderbuffer(ctx, rb);
}


void
_swrast_map_soft_renderbuffer(struct gl_context *ctx,
                              struct gl_renderbuffer *rb,
                              GLuint x, GLuint y, GLuint w, GLuint h,
                              GLbitfield mode,
                              GLubyte **out_map,
                              GLint *out_stride)
{
   struct swrast_renderbuffer *srb = swrast_renderbuffer(rb);
   GLubyte *map = srb->Buffer;
   int cpp = _mesa_get_format_bytes(rb->Format);
   int stride = rb->Width * cpp;

   if (!map) {
      *out_map = NULL;
      *out_stride = 0;
   }

   map += y * stride;
   map += x * cpp;

   *out_map = map;
   *out_stride = stride;
}


void
_swrast_unmap_soft_renderbuffer(struct gl_context *ctx,
                                struct gl_renderbuffer *rb)
{
}



/**
 * Allocate a software-based renderbuffer.  This is called via the
 * ctx->Driver.NewRenderbuffer() function when the user creates a new
 * renderbuffer.
 * This would not be used for hardware-based renderbuffers.
 */
struct gl_renderbuffer *
_swrast_new_soft_renderbuffer(struct gl_context *ctx, GLuint name)
{
   struct swrast_renderbuffer *srb = CALLOC_STRUCT(swrast_renderbuffer);
   if (srb) {
      _mesa_init_renderbuffer(&srb->Base, name);
      srb->Base.AllocStorage = soft_renderbuffer_storage;
      srb->Base.Delete = soft_renderbuffer_delete;
   }
   return &srb->Base;
}


/**
 * Add software-based color renderbuffers to the given framebuffer.
 * This is a helper routine for device drivers when creating a
 * window system framebuffer (not a user-created render/framebuffer).
 * Once this function is called, you can basically forget about this
 * renderbuffer; core Mesa will handle all the buffer management and
 * rendering!
 */
static GLboolean
add_color_renderbuffers(struct gl_context *ctx, struct gl_framebuffer *fb,
                        GLuint rgbBits, GLuint alphaBits,
                        GLboolean frontLeft, GLboolean backLeft,
                        GLboolean frontRight, GLboolean backRight)
{
   gl_buffer_index b;

   if (rgbBits > 16 || alphaBits > 16) {
      _mesa_problem(ctx,
                    "Unsupported bit depth in add_color_renderbuffers");
      return GL_FALSE;
   }

   assert(MAX_COLOR_ATTACHMENTS >= 4);

   for (b = BUFFER_FRONT_LEFT; b <= BUFFER_BACK_RIGHT; b++) {
      struct gl_renderbuffer *rb;

      if (b == BUFFER_FRONT_LEFT && !frontLeft)
         continue;
      else if (b == BUFFER_BACK_LEFT && !backLeft)
         continue;
      else if (b == BUFFER_FRONT_RIGHT && !frontRight)
         continue;
      else if (b == BUFFER_BACK_RIGHT && !backRight)
         continue;

      assert(fb->Attachment[b].Renderbuffer == NULL);

      rb = ctx->Driver.NewRenderbuffer(ctx, 0);
      if (!rb) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "Allocating color buffer");
         return GL_FALSE;
      }

      rb->InternalFormat = GL_RGBA;

      rb->AllocStorage = soft_renderbuffer_storage;
      _mesa_add_renderbuffer(fb, b, rb);
   }

   return GL_TRUE;
}


/**
 * Add a software-based depth renderbuffer to the given framebuffer.
 * This is a helper routine for device drivers when creating a
 * window system framebuffer (not a user-created render/framebuffer).
 * Once this function is called, you can basically forget about this
 * renderbuffer; core Mesa will handle all the buffer management and
 * rendering!
 */
static GLboolean
add_depth_renderbuffer(struct gl_context *ctx, struct gl_framebuffer *fb,
                       GLuint depthBits)
{
   struct gl_renderbuffer *rb;

   if (depthBits > 32) {
      _mesa_problem(ctx,
                    "Unsupported depthBits in add_depth_renderbuffer");
      return GL_FALSE;
   }

   assert(fb->Attachment[BUFFER_DEPTH].Renderbuffer == NULL);

   rb = _swrast_new_soft_renderbuffer(ctx, 0);
   if (!rb) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "Allocating depth buffer");
      return GL_FALSE;
   }

   if (depthBits <= 16) {
      rb->InternalFormat = GL_DEPTH_COMPONENT16;
   }
   else if (depthBits <= 24) {
      rb->InternalFormat = GL_DEPTH_COMPONENT24;
   }
   else {
      rb->InternalFormat = GL_DEPTH_COMPONENT32;
   }

   rb->AllocStorage = soft_renderbuffer_storage;
   _mesa_add_renderbuffer(fb, BUFFER_DEPTH, rb);

   return GL_TRUE;
}


/**
 * Add a software-based stencil renderbuffer to the given framebuffer.
 * This is a helper routine for device drivers when creating a
 * window system framebuffer (not a user-created render/framebuffer).
 * Once this function is called, you can basically forget about this
 * renderbuffer; core Mesa will handle all the buffer management and
 * rendering!
 */
static GLboolean
add_stencil_renderbuffer(struct gl_context *ctx, struct gl_framebuffer *fb,
                         GLuint stencilBits)
{
   struct gl_renderbuffer *rb;

   if (stencilBits > 16) {
      _mesa_problem(ctx,
                  "Unsupported stencilBits in add_stencil_renderbuffer");
      return GL_FALSE;
   }

   assert(fb->Attachment[BUFFER_STENCIL].Renderbuffer == NULL);

   rb = _swrast_new_soft_renderbuffer(ctx, 0);
   if (!rb) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "Allocating stencil buffer");
      return GL_FALSE;
   }

   assert(stencilBits <= 8);
   rb->InternalFormat = GL_STENCIL_INDEX8;

   rb->AllocStorage = soft_renderbuffer_storage;
   _mesa_add_renderbuffer(fb, BUFFER_STENCIL, rb);

   return GL_TRUE;
}


static GLboolean
add_depth_stencil_renderbuffer(struct gl_context *ctx,
                               struct gl_framebuffer *fb)
{
   struct gl_renderbuffer *rb;

   assert(fb->Attachment[BUFFER_DEPTH].Renderbuffer == NULL);
   assert(fb->Attachment[BUFFER_STENCIL].Renderbuffer == NULL);

   rb = _swrast_new_soft_renderbuffer(ctx, 0);
   if (!rb) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "Allocating depth+stencil buffer");
      return GL_FALSE;
   }

   rb->InternalFormat = GL_DEPTH_STENCIL;

   rb->AllocStorage = soft_renderbuffer_storage;
   _mesa_add_renderbuffer(fb, BUFFER_DEPTH, rb);
   _mesa_add_renderbuffer(fb, BUFFER_STENCIL, rb);

   return GL_TRUE;
}


/**
 * Add a software-based accumulation renderbuffer to the given framebuffer.
 * This is a helper routine for device drivers when creating a
 * window system framebuffer (not a user-created render/framebuffer).
 * Once this function is called, you can basically forget about this
 * renderbuffer; core Mesa will handle all the buffer management and
 * rendering!
 */
static GLboolean
add_accum_renderbuffer(struct gl_context *ctx, struct gl_framebuffer *fb,
                       GLuint redBits, GLuint greenBits,
                       GLuint blueBits, GLuint alphaBits)
{
   struct gl_renderbuffer *rb;

   if (redBits > 16 || greenBits > 16 || blueBits > 16 || alphaBits > 16) {
      _mesa_problem(ctx,
                    "Unsupported accumBits in add_accum_renderbuffer");
      return GL_FALSE;
   }

   assert(fb->Attachment[BUFFER_ACCUM].Renderbuffer == NULL);

   rb = _swrast_new_soft_renderbuffer(ctx, 0);
   if (!rb) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "Allocating accum buffer");
      return GL_FALSE;
   }

   rb->InternalFormat = GL_RGBA16_SNORM;
   rb->AllocStorage = soft_renderbuffer_storage;
   _mesa_add_renderbuffer(fb, BUFFER_ACCUM, rb);

   return GL_TRUE;
}



/**
 * Add a software-based aux renderbuffer to the given framebuffer.
 * This is a helper routine for device drivers when creating a
 * window system framebuffer (not a user-created render/framebuffer).
 * Once this function is called, you can basically forget about this
 * renderbuffer; core Mesa will handle all the buffer management and
 * rendering!
 *
 * NOTE: color-index aux buffers not supported.
 */
static GLboolean
add_aux_renderbuffers(struct gl_context *ctx, struct gl_framebuffer *fb,
                      GLuint colorBits, GLuint numBuffers)
{
   GLuint i;

   if (colorBits > 16) {
      _mesa_problem(ctx,
                    "Unsupported colorBits in add_aux_renderbuffers");
      return GL_FALSE;
   }

   assert(numBuffers <= MAX_AUX_BUFFERS);

   for (i = 0; i < numBuffers; i++) {
      struct gl_renderbuffer *rb = _swrast_new_soft_renderbuffer(ctx, 0);

      assert(fb->Attachment[BUFFER_AUX0 + i].Renderbuffer == NULL);

      if (!rb) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "Allocating aux buffer");
         return GL_FALSE;
      }

      assert (colorBits <= 8);
      rb->InternalFormat = GL_RGBA;

      rb->AllocStorage = soft_renderbuffer_storage;
      _mesa_add_renderbuffer(fb, BUFFER_AUX0 + i, rb);
   }
   return GL_TRUE;
}


/**
 * Create/attach software-based renderbuffers to the given framebuffer.
 * This is a helper routine for device drivers.  Drivers can just as well
 * call the individual _mesa_add_*_renderbuffer() routines directly.
 */
void
_swrast_add_soft_renderbuffers(struct gl_framebuffer *fb,
                               GLboolean color,
                               GLboolean depth,
                               GLboolean stencil,
                               GLboolean accum,
                               GLboolean alpha,
                               GLboolean aux)
{
   GLboolean frontLeft = GL_TRUE;
   GLboolean backLeft = fb->Visual.doubleBufferMode;
   GLboolean frontRight = fb->Visual.stereoMode;
   GLboolean backRight = fb->Visual.stereoMode && fb->Visual.doubleBufferMode;

   if (color) {
      assert(fb->Visual.redBits == fb->Visual.greenBits);
      assert(fb->Visual.redBits == fb->Visual.blueBits);
      add_color_renderbuffers(NULL, fb,
                              fb->Visual.redBits,
                              fb->Visual.alphaBits,
                              frontLeft, backLeft,
                              frontRight, backRight);
   }

#if 0
   /* This is pretty much for debugging purposes only since there's a perf
    * hit for using combined depth/stencil in swrast.
    */
   if (depth && fb->Visual.depthBits == 24 &&
       stencil && fb->Visual.stencilBits == 8) {
      /* use combined depth/stencil buffer */
      add_depth_stencil_renderbuffer(NULL, fb);
   }
   else
#else
   (void) add_depth_stencil_renderbuffer;
#endif
   {
      if (depth) {
         assert(fb->Visual.depthBits > 0);
         add_depth_renderbuffer(NULL, fb, fb->Visual.depthBits);
      }

      if (stencil) {
         assert(fb->Visual.stencilBits > 0);
         add_stencil_renderbuffer(NULL, fb, fb->Visual.stencilBits);
      }
   }

   if (accum) {
      assert(fb->Visual.accumRedBits > 0);
      assert(fb->Visual.accumGreenBits > 0);
      assert(fb->Visual.accumBlueBits > 0);
      add_accum_renderbuffer(NULL, fb,
                             fb->Visual.accumRedBits,
                             fb->Visual.accumGreenBits,
                             fb->Visual.accumBlueBits,
                             fb->Visual.accumAlphaBits);
   }

   if (aux) {
      assert(fb->Visual.numAuxBuffers > 0);
      add_aux_renderbuffers(NULL, fb, fb->Visual.redBits,
                            fb->Visual.numAuxBuffers);
   }

#if 0
   if (multisample) {
      /* maybe someday */
   }
#endif
}



static void
map_attachment(struct gl_context *ctx,
                 struct gl_framebuffer *fb,
                 gl_buffer_index buffer)
{
   struct gl_texture_object *texObj = fb->Attachment[buffer].Texture;
   struct gl_renderbuffer *rb = fb->Attachment[buffer].Renderbuffer;
   struct swrast_renderbuffer *srb = swrast_renderbuffer(rb);

   if (texObj) {
      /* map texture image (render to texture) */
      const GLuint level = fb->Attachment[buffer].TextureLevel;
      const GLuint face = fb->Attachment[buffer].CubeMapFace;
      const GLuint slice = fb->Attachment[buffer].Zoffset;
      struct gl_texture_image *texImage = texObj->Image[face][level];
      if (texImage) {
         ctx->Driver.MapTextureImage(ctx, texImage, slice,
                                     0, 0, texImage->Width, texImage->Height,
                                     GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
                                     &srb->Map, &srb->RowStride);
      }
   }
   else if (rb) {
      /* Map ordinary renderbuffer */
      ctx->Driver.MapRenderbuffer(ctx, rb,
                                  0, 0, rb->Width, rb->Height,
                                  GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
                                  &srb->Map, &srb->RowStride);
   }

   assert(srb->Map);
}
 

static void
unmap_attachment(struct gl_context *ctx,
                   struct gl_framebuffer *fb,
                   gl_buffer_index buffer)
{
   struct gl_texture_object *texObj = fb->Attachment[buffer].Texture;
   struct gl_renderbuffer *rb = fb->Attachment[buffer].Renderbuffer;
   struct swrast_renderbuffer *srb = swrast_renderbuffer(rb);

   if (texObj) {
      /* unmap texture image (render to texture) */
      const GLuint level = fb->Attachment[buffer].TextureLevel;
      const GLuint face = fb->Attachment[buffer].CubeMapFace;
      const GLuint slice = fb->Attachment[buffer].Zoffset;
      struct gl_texture_image *texImage = texObj->Image[face][level];
      if (texImage) {
         ctx->Driver.UnmapTextureImage(ctx, texImage, slice);
      }
   }
   else if (rb) {
      /* unmap ordinary renderbuffer */
      ctx->Driver.UnmapRenderbuffer(ctx, rb);
   }

   srb->Map = NULL;
}


/**
 * Determine what type to use (ubyte vs. float) for span colors for the
 * given renderbuffer.
 * See also _swrast_write_rgba_span().
 */
static void
find_renderbuffer_colortype(struct gl_renderbuffer *rb)
{
   struct swrast_renderbuffer *srb = swrast_renderbuffer(rb);
   GLuint rbMaxBits = _mesa_get_format_max_bits(rb->Format);
   GLenum rbDatatype = _mesa_get_format_datatype(rb->Format);

   if (rbDatatype == GL_UNSIGNED_NORMALIZED && rbMaxBits <= 8) {
      /* the buffer's values fit in GLubyte values */
      srb->ColorType = GL_UNSIGNED_BYTE;
   }
   else {
      /* use floats otherwise */
      srb->ColorType = GL_FLOAT;
   }
}


/**
 * Map the renderbuffers we'll use for tri/line/point rendering.
 */
void
_swrast_map_renderbuffers(struct gl_context *ctx)
{
   struct gl_framebuffer *fb = ctx->DrawBuffer;
   struct gl_renderbuffer *depthRb, *stencilRb;
   GLuint buf;

   depthRb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   if (depthRb) {
      /* map depth buffer */
      map_attachment(ctx, fb, BUFFER_DEPTH);
   }

   stencilRb = fb->Attachment[BUFFER_STENCIL].Renderbuffer;
   if (stencilRb && stencilRb != depthRb) {
      /* map stencil buffer */
      map_attachment(ctx, fb, BUFFER_STENCIL);
   }

   for (buf = 0; buf < fb->_NumColorDrawBuffers; buf++) {
      if (fb->_ColorDrawBufferIndexes[buf] >= 0) {
         map_attachment(ctx, fb, fb->_ColorDrawBufferIndexes[buf]);
         find_renderbuffer_colortype(fb->_ColorDrawBuffers[buf]);
      }
   }
}
 
 
/**
 * Unmap renderbuffers after rendering.
 */
void
_swrast_unmap_renderbuffers(struct gl_context *ctx)
{
   struct gl_framebuffer *fb = ctx->DrawBuffer;
   struct gl_renderbuffer *depthRb, *stencilRb;
   GLuint buf;

   depthRb = fb->Attachment[BUFFER_DEPTH].Renderbuffer;
   if (depthRb) {
      /* map depth buffer */
      unmap_attachment(ctx, fb, BUFFER_DEPTH);
   }

   stencilRb = fb->Attachment[BUFFER_STENCIL].Renderbuffer;
   if (stencilRb && stencilRb != depthRb) {
      /* map stencil buffer */
      unmap_attachment(ctx, fb, BUFFER_STENCIL);
   }

   for (buf = 0; buf < fb->_NumColorDrawBuffers; buf++) {
      if (fb->_ColorDrawBufferIndexes[buf] >= 0) {
         unmap_attachment(ctx, fb, fb->_ColorDrawBufferIndexes[buf]);
      }
   }
}
