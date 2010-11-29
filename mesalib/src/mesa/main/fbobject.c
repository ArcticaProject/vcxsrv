/*
 * Mesa 3-D graphics library
 * Version:  7.1
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 1999-2009  VMware, Inc.  All Rights Reserved.
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


/*
 * GL_EXT/ARB_framebuffer_object extensions
 *
 * Authors:
 *   Brian Paul
 */


#include "buffers.h"
#include "context.h"
#include "enums.h"
#include "fbobject.h"
#include "formats.h"
#include "framebuffer.h"
#include "hash.h"
#include "macros.h"
#include "renderbuffer.h"
#include "state.h"
#include "teximage.h"
#include "texobj.h"


/** Set this to 1 to help debug FBO incompleteness problems */
#define DEBUG_FBO 0

/** Set this to 1 to debug/log glBlitFramebuffer() calls */
#define DEBUG_BLIT 0


/**
 * Notes:
 *
 * None of the GL_EXT_framebuffer_object functions are compiled into
 * display lists.
 */



/*
 * When glGenRender/FramebuffersEXT() is called we insert pointers to
 * these placeholder objects into the hash table.
 * Later, when the object ID is first bound, we replace the placeholder
 * with the real frame/renderbuffer.
 */
static struct gl_framebuffer DummyFramebuffer;
static struct gl_renderbuffer DummyRenderbuffer;

/* We bind this framebuffer when applications pass a NULL
 * drawable/surface in make current. */
static struct gl_framebuffer IncompleteFramebuffer;


#define IS_CUBE_FACE(TARGET) \
   ((TARGET) >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && \
    (TARGET) <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)


static void
delete_dummy_renderbuffer(struct gl_renderbuffer *rb)
{
   /* no op */
}

static void
delete_dummy_framebuffer(struct gl_framebuffer *fb)
{
   /* no op */
}


void
_mesa_init_fbobjects(GLcontext *ctx)
{
   _glthread_INIT_MUTEX(DummyFramebuffer.Mutex);
   _glthread_INIT_MUTEX(DummyRenderbuffer.Mutex);
   _glthread_INIT_MUTEX(IncompleteFramebuffer.Mutex);
   DummyFramebuffer.Delete = delete_dummy_framebuffer;
   DummyRenderbuffer.Delete = delete_dummy_renderbuffer;
   IncompleteFramebuffer.Delete = delete_dummy_framebuffer;
}

struct gl_framebuffer *
_mesa_get_incomplete_framebuffer(void)
{
   return &IncompleteFramebuffer;
}

/**
 * Helper routine for getting a gl_renderbuffer.
 */
struct gl_renderbuffer *
_mesa_lookup_renderbuffer(GLcontext *ctx, GLuint id)
{
   struct gl_renderbuffer *rb;

   if (id == 0)
      return NULL;

   rb = (struct gl_renderbuffer *)
      _mesa_HashLookup(ctx->Shared->RenderBuffers, id);
   return rb;
}


/**
 * Helper routine for getting a gl_framebuffer.
 */
struct gl_framebuffer *
_mesa_lookup_framebuffer(GLcontext *ctx, GLuint id)
{
   struct gl_framebuffer *fb;

   if (id == 0)
      return NULL;

   fb = (struct gl_framebuffer *)
      _mesa_HashLookup(ctx->Shared->FrameBuffers, id);
   return fb;
}


/**
 * Mark the given framebuffer as invalid.  This will force the
 * test for framebuffer completeness to be done before the framebuffer
 * is used.
 */
static void
invalidate_framebuffer(struct gl_framebuffer *fb)
{
   fb->_Status = 0; /* "indeterminate" */
}


/**
 * Given a GL_*_ATTACHMENTn token, return a pointer to the corresponding
 * gl_renderbuffer_attachment object.
 * This function is only used for user-created FB objects, not the
 * default / window-system FB object.
 * If \p attachment is GL_DEPTH_STENCIL_ATTACHMENT, return a pointer to
 * the depth buffer attachment point.
 */
struct gl_renderbuffer_attachment *
_mesa_get_attachment(GLcontext *ctx, struct gl_framebuffer *fb,
                     GLenum attachment)
{
   GLuint i;

   assert(fb->Name > 0);

   switch (attachment) {
   case GL_COLOR_ATTACHMENT0_EXT:
   case GL_COLOR_ATTACHMENT1_EXT:
   case GL_COLOR_ATTACHMENT2_EXT:
   case GL_COLOR_ATTACHMENT3_EXT:
   case GL_COLOR_ATTACHMENT4_EXT:
   case GL_COLOR_ATTACHMENT5_EXT:
   case GL_COLOR_ATTACHMENT6_EXT:
   case GL_COLOR_ATTACHMENT7_EXT:
   case GL_COLOR_ATTACHMENT8_EXT:
   case GL_COLOR_ATTACHMENT9_EXT:
   case GL_COLOR_ATTACHMENT10_EXT:
   case GL_COLOR_ATTACHMENT11_EXT:
   case GL_COLOR_ATTACHMENT12_EXT:
   case GL_COLOR_ATTACHMENT13_EXT:
   case GL_COLOR_ATTACHMENT14_EXT:
   case GL_COLOR_ATTACHMENT15_EXT:
      i = attachment - GL_COLOR_ATTACHMENT0_EXT;
      if (i >= ctx->Const.MaxColorAttachments) {
	 return NULL;
      }
      return &fb->Attachment[BUFFER_COLOR0 + i];
   case GL_DEPTH_STENCIL_ATTACHMENT:
      /* fall-through */
   case GL_DEPTH_BUFFER:
      /* fall-through / new in GL 3.0 */
   case GL_DEPTH_ATTACHMENT_EXT:
      return &fb->Attachment[BUFFER_DEPTH];
   case GL_STENCIL_BUFFER:
      /* fall-through / new in GL 3.0 */
   case GL_STENCIL_ATTACHMENT_EXT:
      return &fb->Attachment[BUFFER_STENCIL];
   default:
      return NULL;
   }
}


/**
 * As above, but only used for getting attachments of the default /
 * window-system framebuffer (not user-created framebuffer objects).
 */
static struct gl_renderbuffer_attachment *
_mesa_get_fb0_attachment(GLcontext *ctx, struct gl_framebuffer *fb,
                         GLenum attachment)
{
   assert(fb->Name == 0);

   switch (attachment) {
   case GL_FRONT_LEFT:
      return &fb->Attachment[BUFFER_FRONT_LEFT];
   case GL_FRONT_RIGHT:
      return &fb->Attachment[BUFFER_FRONT_RIGHT];
   case GL_BACK_LEFT:
      return &fb->Attachment[BUFFER_BACK_LEFT];
   case GL_BACK_RIGHT:
      return &fb->Attachment[BUFFER_BACK_RIGHT];
   case GL_AUX0:
      if (fb->Visual.numAuxBuffers == 1) {
         return &fb->Attachment[BUFFER_AUX0];
      }
      return NULL;
   case GL_DEPTH_BUFFER:
      /* fall-through / new in GL 3.0 */
   case GL_DEPTH_ATTACHMENT_EXT:
      return &fb->Attachment[BUFFER_DEPTH];
   case GL_STENCIL_BUFFER:
      /* fall-through / new in GL 3.0 */
   case GL_STENCIL_ATTACHMENT_EXT:
      return &fb->Attachment[BUFFER_STENCIL];
   default:
      return NULL;
   }
}



/**
 * Remove any texture or renderbuffer attached to the given attachment
 * point.  Update reference counts, etc.
 */
void
_mesa_remove_attachment(GLcontext *ctx, struct gl_renderbuffer_attachment *att)
{
   if (att->Type == GL_TEXTURE) {
      ASSERT(att->Texture);
      if (ctx->Driver.FinishRenderTexture) {
         /* tell driver that we're done rendering to this texture. */
         ctx->Driver.FinishRenderTexture(ctx, att);
      }
      _mesa_reference_texobj(&att->Texture, NULL); /* unbind */
      ASSERT(!att->Texture);
   }
   if (att->Type == GL_TEXTURE || att->Type == GL_RENDERBUFFER_EXT) {
      ASSERT(!att->Texture);
      _mesa_reference_renderbuffer(&att->Renderbuffer, NULL); /* unbind */
      ASSERT(!att->Renderbuffer);
   }
   att->Type = GL_NONE;
   att->Complete = GL_TRUE;
}


/**
 * Bind a texture object to an attachment point.
 * The previous binding, if any, will be removed first.
 */
void
_mesa_set_texture_attachment(GLcontext *ctx,
                             struct gl_framebuffer *fb,
                             struct gl_renderbuffer_attachment *att,
                             struct gl_texture_object *texObj,
                             GLenum texTarget, GLuint level, GLuint zoffset)
{
   if (att->Texture == texObj) {
      /* re-attaching same texture */
      ASSERT(att->Type == GL_TEXTURE);
      if (ctx->Driver.FinishRenderTexture)
	 ctx->Driver.FinishRenderTexture(ctx, att);
   }
   else {
      /* new attachment */
      if (ctx->Driver.FinishRenderTexture && att->Texture)
	 ctx->Driver.FinishRenderTexture(ctx, att);
      _mesa_remove_attachment(ctx, att);
      att->Type = GL_TEXTURE;
      assert(!att->Texture);
      _mesa_reference_texobj(&att->Texture, texObj);
   }

   /* always update these fields */
   att->TextureLevel = level;
   att->CubeMapFace = _mesa_tex_target_to_face(texTarget);
   att->Zoffset = zoffset;
   att->Complete = GL_FALSE;

   if (att->Texture->Image[att->CubeMapFace][att->TextureLevel]) {
      ctx->Driver.RenderTexture(ctx, fb, att);
   }

   invalidate_framebuffer(fb);
}


/**
 * Bind a renderbuffer to an attachment point.
 * The previous binding, if any, will be removed first.
 */
void
_mesa_set_renderbuffer_attachment(GLcontext *ctx,
                                  struct gl_renderbuffer_attachment *att,
                                  struct gl_renderbuffer *rb)
{
   /* XXX check if re-doing same attachment, exit early */
   _mesa_remove_attachment(ctx, att);
   att->Type = GL_RENDERBUFFER_EXT;
   att->Texture = NULL; /* just to be safe */
   att->Complete = GL_FALSE;
   _mesa_reference_renderbuffer(&att->Renderbuffer, rb);
}


/**
 * Fallback for ctx->Driver.FramebufferRenderbuffer()
 * Attach a renderbuffer object to a framebuffer object.
 */
void
_mesa_framebuffer_renderbuffer(GLcontext *ctx, struct gl_framebuffer *fb,
                               GLenum attachment, struct gl_renderbuffer *rb)
{
   struct gl_renderbuffer_attachment *att;

   _glthread_LOCK_MUTEX(fb->Mutex);

   att = _mesa_get_attachment(ctx, fb, attachment);
   ASSERT(att);
   if (rb) {
      _mesa_set_renderbuffer_attachment(ctx, att, rb);
      if (attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
         /* do stencil attachment here (depth already done above) */
         att = _mesa_get_attachment(ctx, fb, GL_STENCIL_ATTACHMENT_EXT);
         assert(att);
         _mesa_set_renderbuffer_attachment(ctx, att, rb);
      }
   }
   else {
      _mesa_remove_attachment(ctx, att);
   }

   invalidate_framebuffer(fb);

   _glthread_UNLOCK_MUTEX(fb->Mutex);
}


/**
 * For debug only.
 */
static void
att_incomplete(const char *msg)
{
#if DEBUG_FBO
   _mesa_debug(NULL, "attachment incomplete: %s\n", msg);
#else
   (void) msg;
#endif
}


/**
 * For debug only.
 */
static void
fbo_incomplete(const char *msg, int index)
{
#if DEBUG_FBO
   _mesa_debug(NULL, "FBO Incomplete: %s [%d]\n", msg, index);
#else
   (void) msg;
   (void) index;
#endif
}




/**
 * Test if an attachment point is complete and update its Complete field.
 * \param format if GL_COLOR, this is a color attachment point,
 *               if GL_DEPTH, this is a depth component attachment point,
 *               if GL_STENCIL, this is a stencil component attachment point.
 */
static void
test_attachment_completeness(const GLcontext *ctx, GLenum format,
                             struct gl_renderbuffer_attachment *att)
{
   assert(format == GL_COLOR || format == GL_DEPTH || format == GL_STENCIL);

   /* assume complete */
   att->Complete = GL_TRUE;

   /* Look for reasons why the attachment might be incomplete */
   if (att->Type == GL_TEXTURE) {
      const struct gl_texture_object *texObj = att->Texture;
      struct gl_texture_image *texImage;
      GLenum baseFormat;

      if (!texObj) {
         att_incomplete("no texobj");
         att->Complete = GL_FALSE;
         return;
      }

      texImage = texObj->Image[att->CubeMapFace][att->TextureLevel];
      if (!texImage) {
         att_incomplete("no teximage");
         att->Complete = GL_FALSE;
         return;
      }
      if (texImage->Width < 1 || texImage->Height < 1) {
         att_incomplete("teximage width/height=0");
         printf("texobj = %u\n", texObj->Name);
         printf("level = %d\n", att->TextureLevel);
         att->Complete = GL_FALSE;
         return;
      }
      if (texObj->Target == GL_TEXTURE_3D && att->Zoffset >= texImage->Depth) {
         att_incomplete("bad z offset");
         att->Complete = GL_FALSE;
         return;
      }

      baseFormat = _mesa_get_format_base_format(texImage->TexFormat);

      if (format == GL_COLOR) {
         if (baseFormat != GL_RGB &&
             baseFormat != GL_RGBA &&
	     (!ctx->Extensions.ARB_framebuffer_object ||
	      baseFormat != GL_ALPHA)) {
            att_incomplete("bad format");
            att->Complete = GL_FALSE;
            return;
         }
         if (_mesa_is_format_compressed(texImage->TexFormat)) {
            att_incomplete("compressed internalformat");
            att->Complete = GL_FALSE;
            return;
         }
      }
      else if (format == GL_DEPTH) {
         if (baseFormat == GL_DEPTH_COMPONENT) {
            /* OK */
         }
         else if (ctx->Extensions.EXT_packed_depth_stencil &&
                  ctx->Extensions.ARB_depth_texture &&
                  baseFormat == GL_DEPTH_STENCIL_EXT) {
            /* OK */
         }
         else {
            att->Complete = GL_FALSE;
            att_incomplete("bad depth format");
            return;
         }
      }
      else {
         ASSERT(format == GL_STENCIL);
         if (ctx->Extensions.EXT_packed_depth_stencil &&
             ctx->Extensions.ARB_depth_texture &&
             baseFormat == GL_DEPTH_STENCIL_EXT) {
            /* OK */
         }
         else {
            /* no such thing as stencil-only textures */
            att_incomplete("illegal stencil texture");
            att->Complete = GL_FALSE;
            return;
         }
      }
   }
   else if (att->Type == GL_RENDERBUFFER_EXT) {
      const GLenum baseFormat =
         _mesa_get_format_base_format(att->Renderbuffer->Format);

      ASSERT(att->Renderbuffer);
      if (!att->Renderbuffer->InternalFormat ||
          att->Renderbuffer->Width < 1 ||
          att->Renderbuffer->Height < 1) {
         att_incomplete("0x0 renderbuffer");
         att->Complete = GL_FALSE;
         return;
      }
      if (format == GL_COLOR) {
         if (baseFormat != GL_RGB &&
             baseFormat != GL_RGBA) {
            att_incomplete("bad renderbuffer color format");
            att->Complete = GL_FALSE;
            return;
         }
      }
      else if (format == GL_DEPTH) {
         if (baseFormat == GL_DEPTH_COMPONENT) {
            /* OK */
         }
         else if (ctx->Extensions.EXT_packed_depth_stencil &&
                  baseFormat == GL_DEPTH_STENCIL_EXT) {
            /* OK */
         }
         else {
            att_incomplete("bad renderbuffer depth format");
            att->Complete = GL_FALSE;
            return;
         }
      }
      else {
         assert(format == GL_STENCIL);
         if (baseFormat == GL_STENCIL_INDEX) {
            /* OK */
         }
         else if (ctx->Extensions.EXT_packed_depth_stencil &&
                  baseFormat == GL_DEPTH_STENCIL_EXT) {
            /* OK */
         }
         else {
            att->Complete = GL_FALSE;
            att_incomplete("bad renderbuffer stencil format");
            return;
         }
      }
   }
   else {
      ASSERT(att->Type == GL_NONE);
      /* complete */
      return;
   }
}


/**
 * Test if the given framebuffer object is complete and update its
 * Status field with the results.
 * Calls the ctx->Driver.ValidateFramebuffer() function to allow the
 * driver to make hardware-specific validation/completeness checks.
 * Also update the framebuffer's Width and Height fields if the
 * framebuffer is complete.
 */
void
_mesa_test_framebuffer_completeness(GLcontext *ctx, struct gl_framebuffer *fb)
{
   GLuint numImages;
   GLenum intFormat = GL_NONE; /* color buffers' internal format */
   GLuint minWidth = ~0, minHeight = ~0, maxWidth = 0, maxHeight = 0;
   GLint numSamples = -1;
   GLint i;
   GLuint j;

   assert(fb->Name != 0);

   numImages = 0;
   fb->Width = 0;
   fb->Height = 0;

   /* Start at -2 to more easily loop over all attachment points.
    *  -2: depth buffer
    *  -1: stencil buffer
    * >=0: color buffer
    */
   for (i = -2; i < (GLint) ctx->Const.MaxColorAttachments; i++) {
      struct gl_renderbuffer_attachment *att;
      GLenum f;

      /*
       * XXX for ARB_fbo, only check color buffers that are named by
       * GL_READ_BUFFER and GL_DRAW_BUFFERi.
       */

      /* check for attachment completeness
       */
      if (i == -2) {
         att = &fb->Attachment[BUFFER_DEPTH];
         test_attachment_completeness(ctx, GL_DEPTH, att);
         if (!att->Complete) {
            fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT;
            fbo_incomplete("depth attachment incomplete", -1);
            return;
         }
      }
      else if (i == -1) {
         att = &fb->Attachment[BUFFER_STENCIL];
         test_attachment_completeness(ctx, GL_STENCIL, att);
         if (!att->Complete) {
            fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT;
            fbo_incomplete("stencil attachment incomplete", -1);
            return;
         }
      }
      else {
         att = &fb->Attachment[BUFFER_COLOR0 + i];
         test_attachment_completeness(ctx, GL_COLOR, att);
         if (!att->Complete) {
            fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT;
            fbo_incomplete("color attachment incomplete", i);
            return;
         }
      }

      /* get width, height, format of the renderbuffer/texture
       */
      if (att->Type == GL_TEXTURE) {
         const struct gl_texture_image *texImg
            = att->Texture->Image[att->CubeMapFace][att->TextureLevel];
         minWidth = MIN2(minWidth, texImg->Width);
         maxWidth = MAX2(maxWidth, texImg->Width);
         minHeight = MIN2(minHeight, texImg->Height);
         maxHeight = MAX2(maxHeight, texImg->Height);
         f = texImg->_BaseFormat;
         numImages++;
         if (f != GL_RGB && f != GL_RGBA && f != GL_DEPTH_COMPONENT
             && f != GL_DEPTH_STENCIL_EXT
	     && (!ctx->Extensions.ARB_framebuffer_object || f != GL_ALPHA)) {
            fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT;
            fbo_incomplete("texture attachment incomplete", -1);
            return;
         }
      }
      else if (att->Type == GL_RENDERBUFFER_EXT) {
         minWidth = MIN2(minWidth, att->Renderbuffer->Width);
         maxWidth = MAX2(minWidth, att->Renderbuffer->Width);
         minHeight = MIN2(minHeight, att->Renderbuffer->Height);
         maxHeight = MAX2(minHeight, att->Renderbuffer->Height);
         f = att->Renderbuffer->InternalFormat;
         numImages++;
      }
      else {
         assert(att->Type == GL_NONE);
         continue;
      }

      if (numSamples < 0) {
         /* first buffer */
         numSamples = att->Renderbuffer->NumSamples;
      }

      /* Error-check width, height, format, samples
       */
      if (numImages == 1) {
         /* save format, num samples */
         if (i >= 0) {
            intFormat = f;
         }
      }
      else {
         if (!ctx->Extensions.ARB_framebuffer_object) {
            /* check that width, height, format are same */
            if (minWidth != maxWidth || minHeight != maxHeight) {
               fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT;
               fbo_incomplete("width or height mismatch", -1);
               return;
            }
            /* check that all color buffer have same format */
            if (intFormat != GL_NONE && f != intFormat) {
               fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT;
               fbo_incomplete("format mismatch", -1);
               return;
            }
         }
         if (att->Renderbuffer &&
             att->Renderbuffer->NumSamples != numSamples) {
            fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE;
            fbo_incomplete("inconsistant number of samples", i);
            return;
         }            

      }
   }

#if FEATURE_GL
   if (ctx->API == API_OPENGL) {
      /* Check that all DrawBuffers are present */
      for (j = 0; j < ctx->Const.MaxDrawBuffers; j++) {
	 if (fb->ColorDrawBuffer[j] != GL_NONE) {
	    const struct gl_renderbuffer_attachment *att
	       = _mesa_get_attachment(ctx, fb, fb->ColorDrawBuffer[j]);
	    assert(att);
	    if (att->Type == GL_NONE) {
	       fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT;
	       fbo_incomplete("missing drawbuffer", j);
	       return;
	    }
	 }
      }

      /* Check that the ReadBuffer is present */
      if (fb->ColorReadBuffer != GL_NONE) {
	 const struct gl_renderbuffer_attachment *att
	    = _mesa_get_attachment(ctx, fb, fb->ColorReadBuffer);
	 assert(att);
	 if (att->Type == GL_NONE) {
	    fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT;
            fbo_incomplete("missing readbuffer", -1);
	    return;
	 }
      }
   }
#else
   (void) j;
#endif

   if (numImages == 0) {
      fb->_Status = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT;
      fbo_incomplete("no attachments", -1);
      return;
   }

   /* Provisionally set status = COMPLETE ... */
   fb->_Status = GL_FRAMEBUFFER_COMPLETE_EXT;

   /* ... but the driver may say the FB is incomplete.
    * Drivers will most likely set the status to GL_FRAMEBUFFER_UNSUPPORTED
    * if anything.
    */
   if (ctx->Driver.ValidateFramebuffer) {
      ctx->Driver.ValidateFramebuffer(ctx, fb);
      if (fb->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
         fbo_incomplete("driver marked FBO as incomplete", -1);
      }
   }

   if (fb->_Status == GL_FRAMEBUFFER_COMPLETE_EXT) {
      /*
       * Note that if ARB_framebuffer_object is supported and the attached
       * renderbuffers/textures are different sizes, the framebuffer
       * width/height will be set to the smallest width/height.
       */
      fb->Width = minWidth;
      fb->Height = minHeight;

      /* finally, update the visual info for the framebuffer */
      _mesa_update_framebuffer_visual(fb);
   }
}


GLboolean GLAPIENTRY
_mesa_IsRenderbufferEXT(GLuint renderbuffer)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);
   if (renderbuffer) {
      struct gl_renderbuffer *rb = _mesa_lookup_renderbuffer(ctx, renderbuffer);
      if (rb != NULL && rb != &DummyRenderbuffer)
         return GL_TRUE;
   }
   return GL_FALSE;
}


void GLAPIENTRY
_mesa_BindRenderbufferEXT(GLenum target, GLuint renderbuffer)
{
   struct gl_renderbuffer *newRb;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (target != GL_RENDERBUFFER_EXT) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindRenderbufferEXT(target)");
      return;
   }

   /* No need to flush here since the render buffer binding has no
    * effect on rendering state.
    */

   if (renderbuffer) {
      newRb = _mesa_lookup_renderbuffer(ctx, renderbuffer);
      if (newRb == &DummyRenderbuffer) {
         /* ID was reserved, but no real renderbuffer object made yet */
         newRb = NULL;
      }
      else if (!newRb && ctx->Extensions.ARB_framebuffer_object) {
         /* All RB IDs must be Gen'd */
         _mesa_error(ctx, GL_INVALID_OPERATION, "glBindRenderbuffer(buffer)");
         return;
      }

      if (!newRb) {
	 /* create new renderbuffer object */
	 newRb = ctx->Driver.NewRenderbuffer(ctx, renderbuffer);
	 if (!newRb) {
	    _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBindRenderbufferEXT");
	    return;
	 }
         ASSERT(newRb->AllocStorage);
         _mesa_HashInsert(ctx->Shared->RenderBuffers, renderbuffer, newRb);
         newRb->RefCount = 1; /* referenced by hash table */
      }
   }
   else {
      newRb = NULL;
   }

   ASSERT(newRb != &DummyRenderbuffer);

   _mesa_reference_renderbuffer(&ctx->CurrentRenderbuffer, newRb);
}


/**
 * If the given renderbuffer is anywhere attached to the framebuffer, detach
 * the renderbuffer.
 * This is used when a renderbuffer object is deleted.
 * The spec calls for unbinding.
 */
static void
detach_renderbuffer(GLcontext *ctx,
                    struct gl_framebuffer *fb,
                    struct gl_renderbuffer *rb)
{
   GLuint i;
   for (i = 0; i < BUFFER_COUNT; i++) {
      if (fb->Attachment[i].Renderbuffer == rb) {
         _mesa_remove_attachment(ctx, &fb->Attachment[i]);
      }
   }
   invalidate_framebuffer(fb);
}


void GLAPIENTRY
_mesa_DeleteRenderbuffersEXT(GLsizei n, const GLuint *renderbuffers)
{
   GLint i;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);
   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   for (i = 0; i < n; i++) {
      if (renderbuffers[i] > 0) {
	 struct gl_renderbuffer *rb;
	 rb = _mesa_lookup_renderbuffer(ctx, renderbuffers[i]);
	 if (rb) {
            /* check if deleting currently bound renderbuffer object */
            if (rb == ctx->CurrentRenderbuffer) {
               /* bind default */
               ASSERT(rb->RefCount >= 2);
               _mesa_BindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
            }

            if (ctx->DrawBuffer->Name) {
               detach_renderbuffer(ctx, ctx->DrawBuffer, rb);
            }
            if (ctx->ReadBuffer->Name && ctx->ReadBuffer != ctx->DrawBuffer) {
               detach_renderbuffer(ctx, ctx->ReadBuffer, rb);
            }

	    /* Remove from hash table immediately, to free the ID.
             * But the object will not be freed until it's no longer
             * referenced anywhere else.
             */
	    _mesa_HashRemove(ctx->Shared->RenderBuffers, renderbuffers[i]);

            if (rb != &DummyRenderbuffer) {
               /* no longer referenced by hash table */
               _mesa_reference_renderbuffer(&rb, NULL);
	    }
	 }
      }
   }
}


void GLAPIENTRY
_mesa_GenRenderbuffersEXT(GLsizei n, GLuint *renderbuffers)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint first;
   GLint i;

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGenRenderbuffersEXT(n)");
      return;
   }

   if (!renderbuffers)
      return;

   first = _mesa_HashFindFreeKeyBlock(ctx->Shared->RenderBuffers, n);

   for (i = 0; i < n; i++) {
      GLuint name = first + i;
      renderbuffers[i] = name;
      /* insert dummy placeholder into hash table */
      _glthread_LOCK_MUTEX(ctx->Shared->Mutex);
      _mesa_HashInsert(ctx->Shared->RenderBuffers, name, &DummyRenderbuffer);
      _glthread_UNLOCK_MUTEX(ctx->Shared->Mutex);
   }
}


/**
 * Given an internal format token for a render buffer, return the
 * corresponding base format.
 * This is very similar to _mesa_base_tex_format() but the set of valid
 * internal formats is somewhat different.
 *
 * \return one of GL_RGB, GL_RGBA, GL_STENCIL_INDEX, GL_DEPTH_COMPONENT
 *  GL_DEPTH_STENCIL_EXT or zero if error.
 *
 * XXX in the future when we support red-only and red-green formats
 * we'll also return GL_RED and GL_RG.
 */
GLenum
_mesa_base_fbo_format(GLcontext *ctx, GLenum internalFormat)
{
   switch (internalFormat) {
   case GL_ALPHA:
   case GL_ALPHA4:
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
      return GL_ALPHA;
   case GL_RGB:
   case GL_R3_G3_B2:
   case GL_RGB4:
   case GL_RGB5:
   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      return GL_RGB;
   case GL_RGBA:
   case GL_RGBA2:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGBA12:
   case GL_RGBA16:
   case GL_RGBA16_SNORM:
      return GL_RGBA;
   case GL_STENCIL_INDEX:
   case GL_STENCIL_INDEX1_EXT:
   case GL_STENCIL_INDEX4_EXT:
   case GL_STENCIL_INDEX8_EXT:
   case GL_STENCIL_INDEX16_EXT:
      return GL_STENCIL_INDEX;
   case GL_DEPTH_COMPONENT:
   case GL_DEPTH_COMPONENT16:
   case GL_DEPTH_COMPONENT24:
   case GL_DEPTH_COMPONENT32:
      return GL_DEPTH_COMPONENT;
   case GL_DEPTH_STENCIL_EXT:
   case GL_DEPTH24_STENCIL8_EXT:
      if (ctx->Extensions.EXT_packed_depth_stencil)
         return GL_DEPTH_STENCIL_EXT;
      else
         return 0;
   /* XXX add floating point formats eventually */
   default:
      return 0;
   }
}


/** sentinal value, see below */
#define NO_SAMPLES 1000


/**
 * Helper function used by _mesa_RenderbufferStorageEXT() and 
 * _mesa_RenderbufferStorageMultisample().
 * samples will be NO_SAMPLES if called by _mesa_RenderbufferStorageEXT().
 */
static void
renderbuffer_storage(GLenum target, GLenum internalFormat,
                     GLsizei width, GLsizei height, GLsizei samples)
{
   const char *func = samples == NO_SAMPLES ?
      "glRenderbufferStorage" : "RenderbufferStorageMultisample";
   struct gl_renderbuffer *rb;
   GLenum baseFormat;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (target != GL_RENDERBUFFER_EXT) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target)", func);
      return;
   }

   baseFormat = _mesa_base_fbo_format(ctx, internalFormat);
   if (baseFormat == 0) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(internalFormat)", func);
      return;
   }

   if (width < 1 || width > (GLsizei) ctx->Const.MaxRenderbufferSize) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(width)", func);
      return;
   }

   if (height < 1 || height > (GLsizei) ctx->Const.MaxRenderbufferSize) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(height)", func);
      return;
   }

   if (samples == NO_SAMPLES) {
      /* NumSamples == 0 indicates non-multisampling */
      samples = 0;
   }
   else if (samples > (GLsizei) ctx->Const.MaxSamples) {
      /* note: driver may choose to use more samples than what's requested */
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(samples)", func);
      return;
   }

   rb = ctx->CurrentRenderbuffer;
   if (!rb) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s", func);
      return;
   }

   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   if (rb->InternalFormat == internalFormat &&
       rb->Width == (GLuint) width &&
       rb->Height == (GLuint) height) {
      /* no change in allocation needed */
      return;
   }

   /* These MUST get set by the AllocStorage func */
   rb->Format = MESA_FORMAT_NONE;
   rb->NumSamples = samples;

   /* Now allocate the storage */
   ASSERT(rb->AllocStorage);
   if (rb->AllocStorage(ctx, rb, internalFormat, width, height)) {
      /* No error - check/set fields now */
      assert(rb->Format != MESA_FORMAT_NONE);
      assert(rb->Width == (GLuint) width);
      assert(rb->Height == (GLuint) height);
      rb->InternalFormat = internalFormat;
      rb->_BaseFormat = baseFormat;
      assert(rb->_BaseFormat != 0);
   }
   else {
      /* Probably ran out of memory - clear the fields */
      rb->Width = 0;
      rb->Height = 0;
      rb->Format = MESA_FORMAT_NONE;
      rb->InternalFormat = GL_NONE;
      rb->_BaseFormat = GL_NONE;
      rb->NumSamples = 0;
   }

   /*
   test_framebuffer_completeness(ctx, fb);
   */
   /* XXX if this renderbuffer is attached anywhere, invalidate attachment
    * points???
    */
}


#if FEATURE_OES_EGL_image
void GLAPIENTRY
_mesa_EGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image)
{
   struct gl_renderbuffer *rb;
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (!ctx->Extensions.OES_EGL_image) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glEGLImageTargetRenderbufferStorageOES(unsupported)");
      return;
   }

   if (target != GL_RENDERBUFFER) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "EGLImageTargetRenderbufferStorageOES");
      return;
   }

   rb = ctx->CurrentRenderbuffer;
   if (!rb) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "EGLImageTargetRenderbufferStorageOES");
      return;
   }

   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   ctx->Driver.EGLImageTargetRenderbufferStorage(ctx, rb, image);
}
#endif


/**
 * Helper function for _mesa_GetRenderbufferParameterivEXT() and
 * _mesa_GetFramebufferAttachmentParameterivEXT()
 * We have to be careful to respect the base format.  For example, if a
 * renderbuffer/texture was created with internalFormat=GL_RGB but the
 * driver actually chose a GL_RGBA format, when the user queries ALPHA_SIZE
 * we need to return zero.
 */
static GLint
get_component_bits(GLenum pname, GLenum baseFormat, gl_format format)
{
   switch (pname) {
   case GL_RENDERBUFFER_RED_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
   case GL_RENDERBUFFER_GREEN_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
   case GL_RENDERBUFFER_BLUE_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
      if (baseFormat == GL_RGB || baseFormat == GL_RGBA)
         return _mesa_get_format_bits(format, pname);
      else
         return 0;
   case GL_RENDERBUFFER_ALPHA_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
      if (baseFormat == GL_RGBA || baseFormat == GL_ALPHA)
         return _mesa_get_format_bits(format, pname);
      else
         return 0;
   case GL_RENDERBUFFER_DEPTH_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
      if (baseFormat == GL_DEPTH_COMPONENT || baseFormat == GL_DEPTH_STENCIL)
         return _mesa_get_format_bits(format, pname);
      else
         return 0;
   case GL_RENDERBUFFER_STENCIL_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
      if (baseFormat == GL_STENCIL_INDEX || baseFormat == GL_DEPTH_STENCIL)
         return _mesa_get_format_bits(format, pname);
      else
         return 0;
   default:
      return 0;
   }
}



void GLAPIENTRY
_mesa_RenderbufferStorageEXT(GLenum target, GLenum internalFormat,
                             GLsizei width, GLsizei height)
{
   /* GL_ARB_fbo says calling this function is equivalent to calling
    * glRenderbufferStorageMultisample() with samples=0.  We pass in
    * a token value here just for error reporting purposes.
    */
   renderbuffer_storage(target, internalFormat, width, height, NO_SAMPLES);
}


void GLAPIENTRY
_mesa_RenderbufferStorageMultisample(GLenum target, GLsizei samples,
                                     GLenum internalFormat,
                                     GLsizei width, GLsizei height)
{
   renderbuffer_storage(target, internalFormat, width, height, samples);
}


/**
 * OpenGL ES version of glRenderBufferStorage.
 */
void GLAPIENTRY
_es_RenderbufferStorageEXT(GLenum target, GLenum internalFormat,
			   GLsizei width, GLsizei height)
{
   switch (internalFormat) {
   case GL_RGB565:
      /* XXX this confuses GL_RENDERBUFFER_INTERNAL_FORMAT_OES */
      /* choose a closest format */
      internalFormat = GL_RGB5;
      break;
   default:
      break;
   }

   renderbuffer_storage(target, internalFormat, width, height, 0);
}


void GLAPIENTRY
_mesa_GetRenderbufferParameterivEXT(GLenum target, GLenum pname, GLint *params)
{
   struct gl_renderbuffer *rb;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (target != GL_RENDERBUFFER_EXT) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetRenderbufferParameterivEXT(target)");
      return;
   }

   rb = ctx->CurrentRenderbuffer;
   if (!rb) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetRenderbufferParameterivEXT");
      return;
   }

   /* No need to flush here since we're just quering state which is
    * not effected by rendering.
    */

   switch (pname) {
   case GL_RENDERBUFFER_WIDTH_EXT:
      *params = rb->Width;
      return;
   case GL_RENDERBUFFER_HEIGHT_EXT:
      *params = rb->Height;
      return;
   case GL_RENDERBUFFER_INTERNAL_FORMAT_EXT:
      *params = rb->InternalFormat;
      return;
   case GL_RENDERBUFFER_RED_SIZE_EXT:
   case GL_RENDERBUFFER_GREEN_SIZE_EXT:
   case GL_RENDERBUFFER_BLUE_SIZE_EXT:
   case GL_RENDERBUFFER_ALPHA_SIZE_EXT:
   case GL_RENDERBUFFER_DEPTH_SIZE_EXT:
   case GL_RENDERBUFFER_STENCIL_SIZE_EXT:
      *params = get_component_bits(pname, rb->_BaseFormat, rb->Format);
      break;
   case GL_RENDERBUFFER_SAMPLES:
      if (ctx->Extensions.ARB_framebuffer_object) {
         *params = rb->NumSamples;
         break;
      }
      /* fallthrough */
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetRenderbufferParameterivEXT(target)");
      return;
   }
}


GLboolean GLAPIENTRY
_mesa_IsFramebufferEXT(GLuint framebuffer)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, GL_FALSE);
   if (framebuffer) {
      struct gl_framebuffer *rb = _mesa_lookup_framebuffer(ctx, framebuffer);
      if (rb != NULL && rb != &DummyFramebuffer)
         return GL_TRUE;
   }
   return GL_FALSE;
}


/**
 * Check if any of the attachments of the given framebuffer are textures
 * (render to texture).  Call ctx->Driver.RenderTexture() for such
 * attachments.
 */
static void
check_begin_texture_render(GLcontext *ctx, struct gl_framebuffer *fb)
{
   GLuint i;
   ASSERT(ctx->Driver.RenderTexture);

   if (fb->Name == 0)
      return; /* can't render to texture with winsys framebuffers */

   for (i = 0; i < BUFFER_COUNT; i++) {
      struct gl_renderbuffer_attachment *att = fb->Attachment + i;
      struct gl_texture_object *texObj = att->Texture;
      if (texObj
          && texObj->Image[att->CubeMapFace][att->TextureLevel]) {
         ctx->Driver.RenderTexture(ctx, fb, att);
      }
   }
}


/**
 * Examine all the framebuffer's attachments to see if any are textures.
 * If so, call ctx->Driver.FinishRenderTexture() for each texture to
 * notify the device driver that the texture image may have changed.
 */
static void
check_end_texture_render(GLcontext *ctx, struct gl_framebuffer *fb)
{
   if (fb->Name == 0)
      return; /* can't render to texture with winsys framebuffers */

   if (ctx->Driver.FinishRenderTexture) {
      GLuint i;
      for (i = 0; i < BUFFER_COUNT; i++) {
         struct gl_renderbuffer_attachment *att = fb->Attachment + i;
         if (att->Texture && att->Renderbuffer) {
            ctx->Driver.FinishRenderTexture(ctx, att);
         }
      }
   }
}


void GLAPIENTRY
_mesa_BindFramebufferEXT(GLenum target, GLuint framebuffer)
{
   struct gl_framebuffer *newDrawFb, *newReadFb;
   struct gl_framebuffer *oldDrawFb, *oldReadFb;
   GLboolean bindReadBuf, bindDrawBuf;
   GET_CURRENT_CONTEXT(ctx);

#ifdef DEBUG
   if (ctx->Extensions.ARB_framebuffer_object) {
      ASSERT(ctx->Extensions.EXT_framebuffer_object);
      ASSERT(ctx->Extensions.EXT_framebuffer_blit);
   }
#endif

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (!ctx->Extensions.EXT_framebuffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBindFramebufferEXT(unsupported)");
      return;
   }

   switch (target) {
#if FEATURE_EXT_framebuffer_blit
   case GL_DRAW_FRAMEBUFFER_EXT:
      if (!ctx->Extensions.EXT_framebuffer_blit) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glBindFramebufferEXT(target)");
         return;
      }
      bindDrawBuf = GL_TRUE;
      bindReadBuf = GL_FALSE;
      break;
   case GL_READ_FRAMEBUFFER_EXT:
      if (!ctx->Extensions.EXT_framebuffer_blit) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glBindFramebufferEXT(target)");
         return;
      }
      bindDrawBuf = GL_FALSE;
      bindReadBuf = GL_TRUE;
      break;
#endif
   case GL_FRAMEBUFFER_EXT:
      bindDrawBuf = GL_TRUE;
      bindReadBuf = GL_TRUE;
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glBindFramebufferEXT(target)");
      return;
   }

   if (framebuffer) {
      /* Binding a user-created framebuffer object */
      newDrawFb = _mesa_lookup_framebuffer(ctx, framebuffer);
      if (newDrawFb == &DummyFramebuffer) {
         /* ID was reserved, but no real framebuffer object made yet */
         newDrawFb = NULL;
      }
      else if (!newDrawFb && ctx->Extensions.ARB_framebuffer_object) {
         /* All FBO IDs must be Gen'd */
         _mesa_error(ctx, GL_INVALID_OPERATION, "glBindFramebuffer(buffer)");
         return;
      }

      if (!newDrawFb) {
	 /* create new framebuffer object */
	 newDrawFb = ctx->Driver.NewFramebuffer(ctx, framebuffer);
	 if (!newDrawFb) {
	    _mesa_error(ctx, GL_OUT_OF_MEMORY, "glBindFramebufferEXT");
	    return;
	 }
         _mesa_HashInsert(ctx->Shared->FrameBuffers, framebuffer, newDrawFb);
      }
      newReadFb = newDrawFb;
   }
   else {
      /* Binding the window system framebuffer (which was originally set
       * with MakeCurrent).
       */
      newDrawFb = ctx->WinSysDrawBuffer;
      newReadFb = ctx->WinSysReadBuffer;
   }

   ASSERT(newDrawFb);
   ASSERT(newDrawFb != &DummyFramebuffer);

   /* save pointers to current/old framebuffers */
   oldDrawFb = ctx->DrawBuffer;
   oldReadFb = ctx->ReadBuffer;

   /* check if really changing bindings */
   if (oldDrawFb == newDrawFb)
      bindDrawBuf = GL_FALSE;
   if (oldReadFb == newReadFb)
      bindReadBuf = GL_FALSE;

   /*
    * OK, now bind the new Draw/Read framebuffers, if they're changing.
    *
    * We also check if we're beginning and/or ending render-to-texture.
    * When a framebuffer with texture attachments is unbound, call
    * ctx->Driver.FinishRenderTexture().
    * When a framebuffer with texture attachments is bound, call
    * ctx->Driver.RenderTexture().
    *
    * Note that if the ReadBuffer has texture attachments we don't consider
    * that a render-to-texture case.
    */
   if (bindReadBuf) {
      FLUSH_VERTICES(ctx, _NEW_BUFFERS);

      /* check if old readbuffer was render-to-texture */
      check_end_texture_render(ctx, oldReadFb);

      _mesa_reference_framebuffer(&ctx->ReadBuffer, newReadFb);
   }

   if (bindDrawBuf) {
      FLUSH_VERTICES(ctx, _NEW_BUFFERS);

      /* check if old read/draw buffers were render-to-texture */
      if (!bindReadBuf)
         check_end_texture_render(ctx, oldReadFb);

      if (oldDrawFb != oldReadFb)
         check_end_texture_render(ctx, oldDrawFb);

      /* check if newly bound framebuffer has any texture attachments */
      check_begin_texture_render(ctx, newDrawFb);

      _mesa_reference_framebuffer(&ctx->DrawBuffer, newDrawFb);
   }

   if ((bindDrawBuf || bindReadBuf) && ctx->Driver.BindFramebuffer) {
      ctx->Driver.BindFramebuffer(ctx, target, newDrawFb, newReadFb);
   }
}


void GLAPIENTRY
_mesa_DeleteFramebuffersEXT(GLsizei n, const GLuint *framebuffers)
{
   GLint i;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);
   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   for (i = 0; i < n; i++) {
      if (framebuffers[i] > 0) {
	 struct gl_framebuffer *fb;
	 fb = _mesa_lookup_framebuffer(ctx, framebuffers[i]);
	 if (fb) {
            ASSERT(fb == &DummyFramebuffer || fb->Name == framebuffers[i]);

            /* check if deleting currently bound framebuffer object */
            if (ctx->Extensions.EXT_framebuffer_blit) {
               /* separate draw/read binding points */
               if (fb == ctx->DrawBuffer) {
                  /* bind default */
                  ASSERT(fb->RefCount >= 2);
                  _mesa_BindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0);
               }
               if (fb == ctx->ReadBuffer) {
                  /* bind default */
                  ASSERT(fb->RefCount >= 2);
                  _mesa_BindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, 0);
               }
            }
            else {
               /* only one binding point for read/draw buffers */
               if (fb == ctx->DrawBuffer || fb == ctx->ReadBuffer) {
                  /* bind default */
                  ASSERT(fb->RefCount >= 2);
                  _mesa_BindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
               }    
            }

	    /* remove from hash table immediately, to free the ID */
	    _mesa_HashRemove(ctx->Shared->FrameBuffers, framebuffers[i]);

            if (fb != &DummyFramebuffer) {
               /* But the object will not be freed until it's no longer
                * bound in any context.
                */
               _mesa_reference_framebuffer(&fb, NULL);
	    }
	 }
      }
   }
}


void GLAPIENTRY
_mesa_GenFramebuffersEXT(GLsizei n, GLuint *framebuffers)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint first;
   GLint i;

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   if (n < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGenFramebuffersEXT(n)");
      return;
   }

   if (!framebuffers)
      return;

   first = _mesa_HashFindFreeKeyBlock(ctx->Shared->FrameBuffers, n);

   for (i = 0; i < n; i++) {
      GLuint name = first + i;
      framebuffers[i] = name;
      /* insert dummy placeholder into hash table */
      _glthread_LOCK_MUTEX(ctx->Shared->Mutex);
      _mesa_HashInsert(ctx->Shared->FrameBuffers, name, &DummyFramebuffer);
      _glthread_UNLOCK_MUTEX(ctx->Shared->Mutex);
   }
}



GLenum GLAPIENTRY
_mesa_CheckFramebufferStatusEXT(GLenum target)
{
   struct gl_framebuffer *buffer;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END_WITH_RETVAL(ctx, 0);

   switch (target) {
#if FEATURE_EXT_framebuffer_blit
   case GL_DRAW_FRAMEBUFFER_EXT:
      if (!ctx->Extensions.EXT_framebuffer_blit) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glCheckFramebufferStatus(target)");
         return 0;
      }
      buffer = ctx->DrawBuffer;
      break;
   case GL_READ_FRAMEBUFFER_EXT:
      if (!ctx->Extensions.EXT_framebuffer_blit) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glCheckFramebufferStatus(target)");
         return 0;
      }
      buffer = ctx->ReadBuffer;
      break;
#endif
   case GL_FRAMEBUFFER_EXT:
      buffer = ctx->DrawBuffer;
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glCheckFramebufferStatus(target)");
      return 0; /* formerly GL_FRAMEBUFFER_STATUS_ERROR_EXT */
   }

   if (buffer->Name == 0) {
      /* The window system / default framebuffer is always complete */
      return GL_FRAMEBUFFER_COMPLETE_EXT;
   }

   /* No need to flush here */

   if (buffer->_Status != GL_FRAMEBUFFER_COMPLETE) {
      _mesa_test_framebuffer_completeness(ctx, buffer);
   }

   return buffer->_Status;
}



/**
 * Common code called by glFramebufferTexture1D/2D/3DEXT().
 */
static void
framebuffer_texture(GLcontext *ctx, const char *caller, GLenum target, 
                    GLenum attachment, GLenum textarget, GLuint texture,
                    GLint level, GLint zoffset)
{
   struct gl_renderbuffer_attachment *att;
   struct gl_texture_object *texObj = NULL;
   struct gl_framebuffer *fb;
   GLboolean error = GL_FALSE;

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   switch (target) {
   case GL_READ_FRAMEBUFFER_EXT:
      error = !ctx->Extensions.EXT_framebuffer_blit;
      fb = ctx->ReadBuffer;
      break;
   case GL_DRAW_FRAMEBUFFER_EXT:
      error = !ctx->Extensions.EXT_framebuffer_blit;
      /* fall-through */
   case GL_FRAMEBUFFER_EXT:
      fb = ctx->DrawBuffer;
      break;
   default:
      error = GL_TRUE;
   }

   if (error) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glFramebufferTexture%sEXT(target=0x%x)", caller, target);
      return;
   }

   ASSERT(fb);

   /* check framebuffer binding */
   if (fb->Name == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glFramebufferTexture%sEXT", caller);
      return;
   }


   /* The textarget, level, and zoffset parameters are only validated if
    * texture is non-zero.
    */
   if (texture) {
      GLboolean err = GL_TRUE;

      texObj = _mesa_lookup_texture(ctx, texture);
      if (texObj != NULL) {
         if (textarget == 0) {
            /* XXX what's the purpose of this? */
            err = (texObj->Target != GL_TEXTURE_3D) &&
                (texObj->Target != GL_TEXTURE_1D_ARRAY_EXT) &&
                (texObj->Target != GL_TEXTURE_2D_ARRAY_EXT);
         }
         else {
            err = (texObj->Target == GL_TEXTURE_CUBE_MAP)
                ? !IS_CUBE_FACE(textarget)
                : (texObj->Target != textarget);
         }
      }
      else {
         /* can't render to a non-existant texture */
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glFramebufferTexture%sEXT(non existant texture)",
                     caller);
         return;
      }

      if (err) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glFramebufferTexture%sEXT(texture target mismatch)",
                     caller);
         return;
      }

      if (texObj->Target == GL_TEXTURE_3D) {
         const GLint maxSize = 1 << (ctx->Const.Max3DTextureLevels - 1);
         if (zoffset < 0 || zoffset >= maxSize) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glFramebufferTexture%sEXT(zoffset)", caller);
            return;
         }
      }
      else if ((texObj->Target == GL_TEXTURE_1D_ARRAY_EXT) ||
               (texObj->Target == GL_TEXTURE_2D_ARRAY_EXT)) {
         if (zoffset < 0 || zoffset >= ctx->Const.MaxArrayTextureLayers) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glFramebufferTexture%sEXT(layer)", caller);
            return;
         }
      }

      if ((level < 0) || 
          (level >= _mesa_max_texture_levels(ctx, texObj->Target))) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glFramebufferTexture%sEXT(level)", caller);
         return;
      }
   }

   att = _mesa_get_attachment(ctx, fb, attachment);
   if (att == NULL) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glFramebufferTexture%sEXT(attachment)", caller);
      return;
   }

   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   _glthread_LOCK_MUTEX(fb->Mutex);
   if (texObj) {
      _mesa_set_texture_attachment(ctx, fb, att, texObj, textarget,
                                   level, zoffset);
      /* Set the render-to-texture flag.  We'll check this flag in
       * glTexImage() and friends to determine if we need to revalidate
       * any FBOs that might be rendering into this texture.
       * This flag never gets cleared since it's non-trivial to determine
       * when all FBOs might be done rendering to this texture.  That's OK
       * though since it's uncommon to render to a texture then repeatedly
       * call glTexImage() to change images in the texture.
       */
      texObj->_RenderToTexture = GL_TRUE;
   }
   else {
      _mesa_remove_attachment(ctx, att);
   }

   invalidate_framebuffer(fb);

   _glthread_UNLOCK_MUTEX(fb->Mutex);
}



void GLAPIENTRY
_mesa_FramebufferTexture1DEXT(GLenum target, GLenum attachment,
                              GLenum textarget, GLuint texture, GLint level)
{
   GET_CURRENT_CONTEXT(ctx);

   if ((texture != 0) && (textarget != GL_TEXTURE_1D)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glFramebufferTexture1DEXT(textarget)");
      return;
   }

   framebuffer_texture(ctx, "1D", target, attachment, textarget, texture,
                       level, 0);
}


void GLAPIENTRY
_mesa_FramebufferTexture2DEXT(GLenum target, GLenum attachment,
                              GLenum textarget, GLuint texture, GLint level)
{
   GET_CURRENT_CONTEXT(ctx);

   if ((texture != 0) &&
       (textarget != GL_TEXTURE_2D) &&
       (textarget != GL_TEXTURE_RECTANGLE_ARB) &&
       (!IS_CUBE_FACE(textarget))) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glFramebufferTexture2DEXT(textarget=0x%x)", textarget);
      return;
   }

   framebuffer_texture(ctx, "2D", target, attachment, textarget, texture,
                       level, 0);
}


void GLAPIENTRY
_mesa_FramebufferTexture3DEXT(GLenum target, GLenum attachment,
                              GLenum textarget, GLuint texture,
                              GLint level, GLint zoffset)
{
   GET_CURRENT_CONTEXT(ctx);

   if ((texture != 0) && (textarget != GL_TEXTURE_3D)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glFramebufferTexture3DEXT(textarget)");
      return;
   }

   framebuffer_texture(ctx, "3D", target, attachment, textarget, texture,
                       level, zoffset);
}


void GLAPIENTRY
_mesa_FramebufferTextureLayerEXT(GLenum target, GLenum attachment,
                                 GLuint texture, GLint level, GLint layer)
{
   GET_CURRENT_CONTEXT(ctx);

   framebuffer_texture(ctx, "Layer", target, attachment, 0, texture,
                       level, layer);
}


void GLAPIENTRY
_mesa_FramebufferRenderbufferEXT(GLenum target, GLenum attachment,
                                 GLenum renderbufferTarget,
                                 GLuint renderbuffer)
{
   struct gl_renderbuffer_attachment *att;
   struct gl_framebuffer *fb;
   struct gl_renderbuffer *rb;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   switch (target) {
#if FEATURE_EXT_framebuffer_blit
   case GL_DRAW_FRAMEBUFFER_EXT:
      if (!ctx->Extensions.EXT_framebuffer_blit) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glFramebufferRenderbufferEXT(target)");
         return;
      }
      fb = ctx->DrawBuffer;
      break;
   case GL_READ_FRAMEBUFFER_EXT:
      if (!ctx->Extensions.EXT_framebuffer_blit) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glFramebufferRenderbufferEXT(target)");
         return;
      }
      fb = ctx->ReadBuffer;
      break;
#endif
   case GL_FRAMEBUFFER_EXT:
      fb = ctx->DrawBuffer;
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glFramebufferRenderbufferEXT(target)");
      return;
   }

   if (renderbufferTarget != GL_RENDERBUFFER_EXT) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glFramebufferRenderbufferEXT(renderbufferTarget)");
      return;
   }

   if (fb->Name == 0) {
      /* Can't attach new renderbuffers to a window system framebuffer */
      _mesa_error(ctx, GL_INVALID_OPERATION, "glFramebufferRenderbufferEXT");
      return;
   }

   att = _mesa_get_attachment(ctx, fb, attachment);
   if (att == NULL) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glFramebufferRenderbufferEXT(invalid attachment %s)",
                  _mesa_lookup_enum_by_nr(attachment));
      return;
   }

   if (renderbuffer) {
      rb = _mesa_lookup_renderbuffer(ctx, renderbuffer);
      if (!rb) {
	 _mesa_error(ctx, GL_INVALID_OPERATION,
		     "glFramebufferRenderbufferEXT(non-existant"
                     " renderbuffer %u)", renderbuffer);
	 return;
      }
   }
   else {
      /* remove renderbuffer attachment */
      rb = NULL;
   }

   if (attachment == GL_DEPTH_STENCIL_ATTACHMENT &&
       rb && rb->Format != MESA_FORMAT_NONE) {
      /* make sure the renderbuffer is a depth/stencil format */
      const GLenum baseFormat = _mesa_get_format_base_format(rb->Format);
      if (baseFormat != GL_DEPTH_STENCIL) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glFramebufferRenderbufferEXT(renderbuffer"
                     " is not DEPTH_STENCIL format)");
         return;
      }
   }


   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   assert(ctx->Driver.FramebufferRenderbuffer);
   ctx->Driver.FramebufferRenderbuffer(ctx, fb, attachment, rb);

   /* Some subsequent GL commands may depend on the framebuffer's visual
    * after the binding is updated.  Update visual info now.
    */
   _mesa_update_framebuffer_visual(fb);
}


void GLAPIENTRY
_mesa_GetFramebufferAttachmentParameterivEXT(GLenum target, GLenum attachment,
                                             GLenum pname, GLint *params)
{
   const struct gl_renderbuffer_attachment *att;
   struct gl_framebuffer *buffer;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);

   switch (target) {
#if FEATURE_EXT_framebuffer_blit
   case GL_DRAW_FRAMEBUFFER_EXT:
      if (!ctx->Extensions.EXT_framebuffer_blit) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glGetFramebufferAttachmentParameterivEXT(target)");
         return;
      }
      buffer = ctx->DrawBuffer;
      break;
   case GL_READ_FRAMEBUFFER_EXT:
      if (!ctx->Extensions.EXT_framebuffer_blit) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glGetFramebufferAttachmentParameterivEXT(target)");
         return;
      }
      buffer = ctx->ReadBuffer;
      break;
#endif
   case GL_FRAMEBUFFER_EXT:
      buffer = ctx->DrawBuffer;
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetFramebufferAttachmentParameterivEXT(target)");
      return;
   }

   if (buffer->Name == 0) {
      /* the default / window-system FBO */
      att = _mesa_get_fb0_attachment(ctx, buffer, attachment);
   }
   else {
      /* user-created framebuffer FBO */
      att = _mesa_get_attachment(ctx, buffer, attachment);
   }

   if (att == NULL) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetFramebufferAttachmentParameterivEXT(attachment)");
      return;
   }

   if (attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
      /* the depth and stencil attachments must point to the same buffer */
      const struct gl_renderbuffer_attachment *depthAtt, *stencilAtt;
      depthAtt = _mesa_get_attachment(ctx, buffer, GL_DEPTH_ATTACHMENT);
      stencilAtt = _mesa_get_attachment(ctx, buffer, GL_STENCIL_ATTACHMENT);
      if (depthAtt->Renderbuffer != stencilAtt->Renderbuffer) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetFramebufferAttachmentParameterivEXT(DEPTH/STENCIL"
                     " attachments differ)");
         return;
      }
   }

   /* No need to flush here */

   switch (pname) {
   case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT:
      *params = att->Type;
      return;
   case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT:
      if (att->Type == GL_RENDERBUFFER_EXT) {
	 *params = att->Renderbuffer->Name;
      }
      else if (att->Type == GL_TEXTURE) {
	 *params = att->Texture->Name;
      }
      else {
	 _mesa_error(ctx, GL_INVALID_ENUM,
		     "glGetFramebufferAttachmentParameterivEXT(pname)");
      }
      return;
   case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT:
      if (att->Type == GL_TEXTURE) {
	 *params = att->TextureLevel;
      }
      else {
	 _mesa_error(ctx, GL_INVALID_ENUM,
		     "glGetFramebufferAttachmentParameterivEXT(pname)");
      }
      return;
   case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT:
      if (att->Type == GL_TEXTURE) {
         if (att->Texture && att->Texture->Target == GL_TEXTURE_CUBE_MAP) {
            *params = GL_TEXTURE_CUBE_MAP_POSITIVE_X + att->CubeMapFace;
         }
         else {
            *params = 0;
         }
      }
      else {
	 _mesa_error(ctx, GL_INVALID_ENUM,
		     "glGetFramebufferAttachmentParameterivEXT(pname)");
      }
      return;
   case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT:
      if (att->Type == GL_TEXTURE) {
         if (att->Texture && att->Texture->Target == GL_TEXTURE_3D) {
            *params = att->Zoffset;
         }
         else {
            *params = 0;
         }
      }
      else {
	 _mesa_error(ctx, GL_INVALID_ENUM,
		     "glGetFramebufferAttachmentParameterivEXT(pname)");
      }
      return;
   case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
      if (!ctx->Extensions.ARB_framebuffer_object) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glGetFramebufferAttachmentParameterivEXT(pname)");
      }
      else {
         *params = _mesa_get_format_color_encoding(att->Renderbuffer->Format);
      }
      return;
   case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
      if (!ctx->Extensions.ARB_framebuffer_object) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glGetFramebufferAttachmentParameterivEXT(pname)");
         return;
      }
      else {
         gl_format format = att->Renderbuffer->Format;
         if (format == MESA_FORMAT_CI8 || format == MESA_FORMAT_S8) {
            /* special cases */
            *params = GL_INDEX;
         }
         else {
            *params = _mesa_get_format_datatype(format);
         }
      }
      return;
   case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
   case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
   case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
   case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
   case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
   case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
      if (!ctx->Extensions.ARB_framebuffer_object) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glGetFramebufferAttachmentParameterivEXT(pname)");
      }
      else if (att->Texture) {
         const struct gl_texture_image *texImage =
            _mesa_select_tex_image(ctx, att->Texture, att->Texture->Target,
                                   att->TextureLevel);
         if (texImage) {
            *params = get_component_bits(pname, texImage->_BaseFormat,
                                         texImage->TexFormat);
         }
         else {
            *params = 0;
         }
      }
      else if (att->Renderbuffer) {
         *params = get_component_bits(pname, att->Renderbuffer->_BaseFormat,
                                      att->Renderbuffer->Format);
      }
      else {
         *params = 0;
      }
      return;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetFramebufferAttachmentParameterivEXT(pname)");
      return;
   }
}


void GLAPIENTRY
_mesa_GenerateMipmapEXT(GLenum target)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);
   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   switch (target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
      /* OK, legal value */
      break;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM, "glGenerateMipmapEXT(target)");
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);

   if (texObj->BaseLevel >= texObj->MaxLevel) {
      /* nothing to do */
      return;
   }

   _mesa_lock_texture(ctx, texObj);
   if (target == GL_TEXTURE_CUBE_MAP) {
      GLuint face;
      for (face = 0; face < 6; face++)
	 ctx->Driver.GenerateMipmap(ctx,
				    GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + face,
				    texObj);
   }
   else {
      ctx->Driver.GenerateMipmap(ctx, target, texObj);
   }
   _mesa_unlock_texture(ctx, texObj);
}


#if FEATURE_EXT_framebuffer_blit

static const struct gl_renderbuffer_attachment *
find_attachment(const struct gl_framebuffer *fb, const struct gl_renderbuffer *rb)
{
   GLuint i;
   for (i = 0; i < Elements(fb->Attachment); i++) {
      if (fb->Attachment[i].Renderbuffer == rb)
         return &fb->Attachment[i];
   }
   return NULL;
}



/**
 * Blit rectangular region, optionally from one framebuffer to another.
 *
 * Note, if the src buffer is multisampled and the dest is not, this is
 * when the samples must be resolved to a single color.
 */
void GLAPIENTRY
_mesa_BlitFramebufferEXT(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                         GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                         GLbitfield mask, GLenum filter)
{
   const GLbitfield legalMaskBits = (GL_COLOR_BUFFER_BIT |
                                     GL_DEPTH_BUFFER_BIT |
                                     GL_STENCIL_BUFFER_BIT);
   const struct gl_framebuffer *readFb, *drawFb;
   const struct gl_renderbuffer *colorReadRb, *colorDrawRb;
   GET_CURRENT_CONTEXT(ctx);

   ASSERT_OUTSIDE_BEGIN_END(ctx);
   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   if (ctx->NewState) {
      _mesa_update_state(ctx);
   }

   readFb = ctx->ReadBuffer;
   drawFb = ctx->DrawBuffer;

   if (!readFb || !drawFb) {
      /* This will normally never happen but someday we may want to
       * support MakeCurrent() with no drawables.
       */
      return;
   }

   /* check for complete framebuffers */
   if (drawFb->_Status != GL_FRAMEBUFFER_COMPLETE_EXT ||
       readFb->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "glBlitFramebufferEXT(incomplete draw/read buffers)");
      return;
   }

   if (filter != GL_NEAREST && filter != GL_LINEAR) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBlitFramebufferEXT(filter)");
      return;
   }

   if (mask & ~legalMaskBits) {
      _mesa_error( ctx, GL_INVALID_VALUE, "glBlitFramebufferEXT(mask)");
      return;
   }

   /* depth/stencil must be blitted with nearest filtering */
   if ((mask & (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT))
        && filter != GL_NEAREST) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
             "glBlitFramebufferEXT(depth/stencil requires GL_NEAREST filter");
      return;
   }

   /* get color read/draw renderbuffers */
   if (mask & GL_COLOR_BUFFER_BIT) {
      colorReadRb = readFb->_ColorReadBuffer;
      colorDrawRb = drawFb->_ColorDrawBuffers[0];
   }
   else {
      colorReadRb = colorDrawRb = NULL;
   }

   if (mask & GL_STENCIL_BUFFER_BIT) {
      struct gl_renderbuffer *readRb = readFb->_StencilBuffer;
      struct gl_renderbuffer *drawRb = drawFb->_StencilBuffer;
      if (!readRb ||
          !drawRb ||
          _mesa_get_format_bits(readRb->Format, GL_STENCIL_BITS) != 
          _mesa_get_format_bits(drawRb->Format, GL_STENCIL_BITS)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBlitFramebufferEXT(stencil buffer size mismatch");
         return;
      }
   }

   if (mask & GL_DEPTH_BUFFER_BIT) {
      struct gl_renderbuffer *readRb = readFb->_DepthBuffer;
      struct gl_renderbuffer *drawRb = drawFb->_DepthBuffer;
      if (!readRb ||
          !drawRb ||
          _mesa_get_format_bits(readRb->Format, GL_DEPTH_BITS) != 
          _mesa_get_format_bits(drawRb->Format, GL_DEPTH_BITS)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBlitFramebufferEXT(depth buffer size mismatch");
         return;
      }
   }

   if (readFb->Visual.samples > 0 &&
       drawFb->Visual.samples > 0 &&
       readFb->Visual.samples != drawFb->Visual.samples) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBlitFramebufferEXT(mismatched samples");
      return;
   }

   /* extra checks for multisample copies... */
   if (readFb->Visual.samples > 0 || drawFb->Visual.samples > 0) {
      /* src and dest region sizes must be the same */
      if (srcX1 - srcX0 != dstX1 - dstX0 ||
          srcY1 - srcY0 != dstY1 - dstY0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                "glBlitFramebufferEXT(bad src/dst multisample region sizes");
         return;
      }

      /* color formats must match */
      if (colorReadRb &&
          colorDrawRb &&
          colorReadRb->Format != colorDrawRb->Format) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                "glBlitFramebufferEXT(bad src/dst multisample pixel formats");
         return;
      }
   }

   if (!ctx->Extensions.EXT_framebuffer_blit) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glBlitFramebufferEXT");
      return;
   }

   /* Debug code */
   if (DEBUG_BLIT) {
      printf("glBlitFramebuffer(%d, %d, %d, %d,  %d, %d, %d, %d,"
	     " 0x%x, 0x%x)\n",
	     srcX0, srcY0, srcX1, srcY1,
	     dstX0, dstY0, dstX1, dstY1,
	     mask, filter);
      if (colorReadRb) {
         const struct gl_renderbuffer_attachment *att;

         att = find_attachment(readFb, colorReadRb);
         printf("  Src FBO %u  RB %u (%dx%d)  ",
		readFb->Name, colorReadRb->Name,
		colorReadRb->Width, colorReadRb->Height);
         if (att && att->Texture) {
            printf("Tex %u  tgt 0x%x  level %u  face %u",
		   att->Texture->Name,
		   att->Texture->Target,
		   att->TextureLevel,
		   att->CubeMapFace);
         }
         printf("\n");

         att = find_attachment(drawFb, colorDrawRb);
         printf("  Dst FBO %u  RB %u (%dx%d)  ",
		drawFb->Name, colorDrawRb->Name,
		colorDrawRb->Width, colorDrawRb->Height);
         if (att && att->Texture) {
            printf("Tex %u  tgt 0x%x  level %u  face %u",
		   att->Texture->Name,
		   att->Texture->Target,
		   att->TextureLevel,
		   att->CubeMapFace);
         }
         printf("\n");
      }
   }

   ASSERT(ctx->Driver.BlitFramebuffer);
   ctx->Driver.BlitFramebuffer(ctx,
                               srcX0, srcY0, srcX1, srcY1,
                               dstX0, dstY0, dstX1, dstY1,
                               mask, filter);
}
#endif /* FEATURE_EXT_framebuffer_blit */

#if FEATURE_ARB_geometry_shader4
void GLAPIENTRY
_mesa_FramebufferTextureARB(GLenum target, GLenum attachment,
                            GLuint texture, GLint level)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_error(ctx, GL_INVALID_OPERATION,
               "glFramebufferTextureARB "
               "not implemented!");
}

void GLAPIENTRY
_mesa_FramebufferTextureFaceARB(GLenum target, GLenum attachment,
                                GLuint texture, GLint level, GLenum face)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_error(ctx, GL_INVALID_OPERATION,
               "glFramebufferTextureFaceARB "
               "not implemented!");
}
#endif /* FEATURE_ARB_geometry_shader4 */
