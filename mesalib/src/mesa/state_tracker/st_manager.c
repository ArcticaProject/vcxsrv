/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2010 LunarG Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 */

#include "main/mtypes.h"
#include "main/extensions.h"
#include "main/context.h"
#include "main/texobj.h"
#include "main/teximage.h"
#include "main/texstate.h"
#include "main/errors.h"
#include "main/framebuffer.h"
#include "main/fbobject.h"
#include "main/renderbuffer.h"
#include "main/version.h"
#include "st_texture.h"

#include "st_context.h"
#include "st_extensions.h"
#include "st_format.h"
#include "st_cb_fbo.h"
#include "st_cb_flush.h"
#include "st_manager.h"

#include "state_tracker/st_gl_api.h"

#include "pipe/p_context.h"
#include "pipe/p_screen.h"
#include "util/u_format.h"
#include "util/u_pointer.h"
#include "util/u_inlines.h"
#include "util/u_atomic.h"
#include "util/u_surface.h"

/**
 * Cast wrapper to convert a struct gl_framebuffer to an st_framebuffer.
 * Return NULL if the struct gl_framebuffer is a user-created framebuffer.
 * We'll only return non-null for window system framebuffers.
 * Note that this function may fail.
 */
static INLINE struct st_framebuffer *
st_ws_framebuffer(struct gl_framebuffer *fb)
{
   /* FBO cannot be casted.  See st_new_framebuffer */
   if (fb && _mesa_is_winsys_fbo(fb))
      return (struct st_framebuffer *) fb;
   return NULL;
}

/**
 * Map an attachment to a buffer index.
 */
static INLINE gl_buffer_index
attachment_to_buffer_index(enum st_attachment_type statt)
{
   gl_buffer_index index;

   switch (statt) {
   case ST_ATTACHMENT_FRONT_LEFT:
      index = BUFFER_FRONT_LEFT;
      break;
   case ST_ATTACHMENT_BACK_LEFT:
      index = BUFFER_BACK_LEFT;
      break;
   case ST_ATTACHMENT_FRONT_RIGHT:
      index = BUFFER_FRONT_RIGHT;
      break;
   case ST_ATTACHMENT_BACK_RIGHT:
      index = BUFFER_BACK_RIGHT;
      break;
   case ST_ATTACHMENT_DEPTH_STENCIL:
      index = BUFFER_DEPTH;
      break;
   case ST_ATTACHMENT_ACCUM:
      index = BUFFER_ACCUM;
      break;
   case ST_ATTACHMENT_SAMPLE:
   default:
      index = BUFFER_COUNT;
      break;
   }

   return index;
}

/**
 * Map a buffer index to an attachment.
 */
static INLINE enum st_attachment_type
buffer_index_to_attachment(gl_buffer_index index)
{
   enum st_attachment_type statt;

   switch (index) {
   case BUFFER_FRONT_LEFT:
      statt = ST_ATTACHMENT_FRONT_LEFT;
      break;
   case BUFFER_BACK_LEFT:
      statt = ST_ATTACHMENT_BACK_LEFT;
      break;
   case BUFFER_FRONT_RIGHT:
      statt = ST_ATTACHMENT_FRONT_RIGHT;
      break;
   case BUFFER_BACK_RIGHT:
      statt = ST_ATTACHMENT_BACK_RIGHT;
      break;
   case BUFFER_DEPTH:
      statt = ST_ATTACHMENT_DEPTH_STENCIL;
      break;
   case BUFFER_ACCUM:
      statt = ST_ATTACHMENT_ACCUM;
      break;
   default:
      statt = ST_ATTACHMENT_INVALID;
      break;
   }

   return statt;
}

/**
 * Make sure a context picks up the latest cached state of the
 * drawables it binds to.
 */
static void
st_context_validate(struct st_context *st,
                    struct st_framebuffer *stdraw,
                    struct st_framebuffer *stread)
{
    if (stdraw && stdraw->stamp != st->draw_stamp) {
       st->dirty.st |= ST_NEW_FRAMEBUFFER;
       _mesa_resize_framebuffer(st->ctx, &stdraw->Base,
                                stdraw->Base.Width,
                                stdraw->Base.Height);
       st->draw_stamp = stdraw->stamp;
    }

    if (stread && stread->stamp != st->read_stamp) {
       if (stread != stdraw) {
          st->dirty.st |= ST_NEW_FRAMEBUFFER;
          _mesa_resize_framebuffer(st->ctx, &stread->Base,
                                   stread->Base.Width,
                                   stread->Base.Height);
       }
       st->read_stamp = stread->stamp;
    }
}

/**
 * Validate a framebuffer to make sure up-to-date pipe_textures are used.
 * The context is only used for creating pipe surfaces and for calling
 * _mesa_resize_framebuffer().
 * (That should probably be rethought, since those surfaces become
 * drawable state, not context state, and can be freed by another pipe
 * context).
 */
static void
st_framebuffer_validate(struct st_framebuffer *stfb,
                        struct st_context *st)
{
   struct pipe_resource *textures[ST_ATTACHMENT_COUNT];
   uint width, height;
   unsigned i;
   boolean changed = FALSE;
   int32_t new_stamp;

   /* Check for incomplete framebuffers (e.g. EGL_KHR_surfaceless_context) */
   if (!stfb->iface)
      return;

   new_stamp = p_atomic_read(&stfb->iface->stamp);
   if (stfb->iface_stamp == new_stamp)
      return;

   /* validate the fb */
   do {
      if (!stfb->iface->validate(&st->iface, stfb->iface, stfb->statts,
				 stfb->num_statts, textures))
	 return;

      stfb->iface_stamp = new_stamp;
      new_stamp = p_atomic_read(&stfb->iface->stamp);
   } while(stfb->iface_stamp != new_stamp);

   width = stfb->Base.Width;
   height = stfb->Base.Height;

   for (i = 0; i < stfb->num_statts; i++) {
      struct st_renderbuffer *strb;
      struct pipe_surface *ps, surf_tmpl;
      gl_buffer_index idx;

      if (!textures[i])
         continue;

      idx = attachment_to_buffer_index(stfb->statts[i]);
      if (idx >= BUFFER_COUNT) {
         pipe_resource_reference(&textures[i], NULL);
         continue;
      }

      strb = st_renderbuffer(stfb->Base.Attachment[idx].Renderbuffer);
      assert(strb);
      if (strb->texture == textures[i]) {
         pipe_resource_reference(&textures[i], NULL);
         continue;
      }

      u_surface_default_template(&surf_tmpl, textures[i]);
      ps = st->pipe->create_surface(st->pipe, textures[i], &surf_tmpl);
      if (ps) {
         pipe_surface_reference(&strb->surface, ps);
         pipe_resource_reference(&strb->texture, ps->texture);
         /* ownership transfered */
         pipe_surface_reference(&ps, NULL);

         changed = TRUE;

         strb->Base.Width = strb->surface->width;
         strb->Base.Height = strb->surface->height;

         width = strb->Base.Width;
         height = strb->Base.Height;
      }

      pipe_resource_reference(&textures[i], NULL);
   }

   if (changed) {
      ++stfb->stamp;
      _mesa_resize_framebuffer(st->ctx, &stfb->Base, width, height);
   }
}

/**
 * Update the attachments to validate by looping the existing renderbuffers.
 */
static void
st_framebuffer_update_attachments(struct st_framebuffer *stfb)
{
   gl_buffer_index idx;

   stfb->num_statts = 0;
   for (idx = 0; idx < BUFFER_COUNT; idx++) {
      struct st_renderbuffer *strb;
      enum st_attachment_type statt;

      strb = st_renderbuffer(stfb->Base.Attachment[idx].Renderbuffer);
      if (!strb || strb->software)
         continue;

      statt = buffer_index_to_attachment(idx);
      if (statt != ST_ATTACHMENT_INVALID &&
          st_visual_have_buffers(stfb->iface->visual, 1 << statt))
         stfb->statts[stfb->num_statts++] = statt;
   }
   stfb->stamp++;
}

/**
 * Add a renderbuffer to the framebuffer.
 */
static boolean
st_framebuffer_add_renderbuffer(struct st_framebuffer *stfb,
                                gl_buffer_index idx)
{
   struct gl_renderbuffer *rb;
   enum pipe_format format;
   boolean sw;

   if (!stfb->iface)
      return FALSE;

   /* do not distinguish depth/stencil buffers */
   if (idx == BUFFER_STENCIL)
      idx = BUFFER_DEPTH;

   switch (idx) {
   case BUFFER_DEPTH:
      format = stfb->iface->visual->depth_stencil_format;
      sw = FALSE;
      break;
   case BUFFER_ACCUM:
      format = stfb->iface->visual->accum_format;
      sw = TRUE;
      break;
   default:
      format = stfb->iface->visual->color_format;
      if (stfb->Base.Visual.sRGBCapable)
         format = util_format_srgb(format);
      sw = FALSE;
      break;
   }

   if (format == PIPE_FORMAT_NONE)
      return FALSE;

   rb = st_new_renderbuffer_fb(format, stfb->iface->visual->samples, sw);
   if (!rb)
      return FALSE;

   if (idx != BUFFER_DEPTH) {
      _mesa_add_renderbuffer(&stfb->Base, idx, rb);
   }
   else {
      if (util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_ZS, 0))
         _mesa_add_renderbuffer(&stfb->Base, BUFFER_DEPTH, rb);
      if (util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_ZS, 1))
         _mesa_add_renderbuffer(&stfb->Base, BUFFER_STENCIL, rb);
   }

   return TRUE;
}

/**
 * Intialize a struct gl_config from a visual.
 */
static void
st_visual_to_context_mode(const struct st_visual *visual,
                          struct gl_config *mode)
{
   memset(mode, 0, sizeof(*mode));

   if (st_visual_have_buffers(visual, ST_ATTACHMENT_BACK_LEFT_MASK))
      mode->doubleBufferMode = GL_TRUE;
   if (st_visual_have_buffers(visual,
            ST_ATTACHMENT_FRONT_RIGHT_MASK | ST_ATTACHMENT_BACK_RIGHT_MASK))
      mode->stereoMode = GL_TRUE;

   if (visual->color_format != PIPE_FORMAT_NONE) {
      mode->rgbMode = GL_TRUE;

      mode->redBits =
         util_format_get_component_bits(visual->color_format,
               UTIL_FORMAT_COLORSPACE_RGB, 0);
      mode->greenBits =
         util_format_get_component_bits(visual->color_format,
               UTIL_FORMAT_COLORSPACE_RGB, 1);
      mode->blueBits =
         util_format_get_component_bits(visual->color_format,
               UTIL_FORMAT_COLORSPACE_RGB, 2);
      mode->alphaBits =
         util_format_get_component_bits(visual->color_format,
               UTIL_FORMAT_COLORSPACE_RGB, 3);

      mode->rgbBits = mode->redBits +
         mode->greenBits + mode->blueBits + mode->alphaBits;
   }

   if (visual->depth_stencil_format != PIPE_FORMAT_NONE) {
      mode->depthBits =
         util_format_get_component_bits(visual->depth_stencil_format,
               UTIL_FORMAT_COLORSPACE_ZS, 0);
      mode->stencilBits =
         util_format_get_component_bits(visual->depth_stencil_format,
               UTIL_FORMAT_COLORSPACE_ZS, 1);

      mode->haveDepthBuffer = mode->depthBits > 0;
      mode->haveStencilBuffer = mode->stencilBits > 0;
   }

   if (visual->accum_format != PIPE_FORMAT_NONE) {
      mode->haveAccumBuffer = GL_TRUE;

      mode->accumRedBits =
         util_format_get_component_bits(visual->accum_format,
               UTIL_FORMAT_COLORSPACE_RGB, 0);
      mode->accumGreenBits =
         util_format_get_component_bits(visual->accum_format,
               UTIL_FORMAT_COLORSPACE_RGB, 1);
      mode->accumBlueBits =
         util_format_get_component_bits(visual->accum_format,
               UTIL_FORMAT_COLORSPACE_RGB, 2);
      mode->accumAlphaBits =
         util_format_get_component_bits(visual->accum_format,
               UTIL_FORMAT_COLORSPACE_RGB, 3);
   }

   if (visual->samples > 1) {
      mode->sampleBuffers = 1;
      mode->samples = visual->samples;
   }
}

/**
 * Create a framebuffer from a manager interface.
 */
static struct st_framebuffer *
st_framebuffer_create(struct st_context *st,
                      struct st_framebuffer_iface *stfbi)
{
   struct st_framebuffer *stfb;
   struct gl_config mode;
   gl_buffer_index idx;

   if (!stfbi)
      return NULL;

   stfb = CALLOC_STRUCT(st_framebuffer);
   if (!stfb)
      return NULL;

   st_visual_to_context_mode(stfbi->visual, &mode);

   /*
    * For desktop GL, sRGB framebuffer write is controlled by both the
    * capability of the framebuffer and GL_FRAMEBUFFER_SRGB.  We should
    * advertise the capability when the pipe driver (and core Mesa) supports
    * it so that applications can enable sRGB write when they want to.
    *
    * This is not to be confused with GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB.  When
    * the attribute is GLX_TRUE, it tells the st manager to pick a color
    * format such that util_format_srgb(visual->color_format) can be supported
    * by the pipe driver.  We still need to advertise the capability here.
    *
    * For GLES, however, sRGB framebuffer write is controlled only by the
    * capability of the framebuffer.  There is GL_EXT_sRGB_write_control to
    * give applications the control back, but sRGB write is still enabled by
    * default.  To avoid unexpected results, we should not advertise the
    * capability.  This could change when we add support for
    * EGL_KHR_gl_colorspace.
    */
   if (_mesa_is_desktop_gl(st->ctx)) {
      struct pipe_screen *screen = st->pipe->screen;
      const enum pipe_format srgb_format =
         util_format_srgb(stfbi->visual->color_format);

      if (srgb_format != PIPE_FORMAT_NONE &&
          st_pipe_format_to_mesa_format(srgb_format) != MESA_FORMAT_NONE &&
          screen->is_format_supported(screen, srgb_format,
                                      PIPE_TEXTURE_2D, stfbi->visual->samples,
                                      PIPE_BIND_RENDER_TARGET))
         mode.sRGBCapable = GL_TRUE;
   }

   _mesa_initialize_window_framebuffer(&stfb->Base, &mode);

   stfb->iface = stfbi;
   stfb->iface_stamp = p_atomic_read(&stfbi->stamp) - 1;

   /* add the color buffer */
   idx = stfb->Base._ColorDrawBufferIndexes[0];
   if (!st_framebuffer_add_renderbuffer(stfb, idx)) {
      free(stfb);
      return NULL;
   }

   st_framebuffer_add_renderbuffer(stfb, BUFFER_DEPTH);
   st_framebuffer_add_renderbuffer(stfb, BUFFER_ACCUM);

   stfb->stamp = 0;
   st_framebuffer_update_attachments(stfb);

   return stfb;
}

/**
 * Reference a framebuffer.
 */
static void
st_framebuffer_reference(struct st_framebuffer **ptr,
                         struct st_framebuffer *stfb)
{
   struct gl_framebuffer *fb = &stfb->Base;
   _mesa_reference_framebuffer((struct gl_framebuffer **) ptr, fb);
}

static void
st_context_flush(struct st_context_iface *stctxi, unsigned flags,
                 struct pipe_fence_handle **fence)
{
   struct st_context *st = (struct st_context *) stctxi;
   unsigned pipe_flags = 0;

   if (flags & ST_FLUSH_END_OF_FRAME) {
      pipe_flags |= PIPE_FLUSH_END_OF_FRAME;
   }

   st_flush(st, fence, pipe_flags);
   if (flags & ST_FLUSH_FRONT)
      st_manager_flush_frontbuffer(st);
}

static boolean
st_context_teximage(struct st_context_iface *stctxi,
                    enum st_texture_type tex_type,
                    int level, enum pipe_format pipe_format,
                    struct pipe_resource *tex, boolean mipmap)
{
   struct st_context *st = (struct st_context *) stctxi;
   struct gl_context *ctx = st->ctx;
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;
   struct st_texture_object *stObj;
   struct st_texture_image *stImage;
   GLenum internalFormat;
   GLuint width, height, depth;
   GLenum target;

   switch (tex_type) {
   case ST_TEXTURE_1D:
      target = GL_TEXTURE_1D;
      break;
   case ST_TEXTURE_2D:
      target = GL_TEXTURE_2D;
      break;
   case ST_TEXTURE_3D:
      target = GL_TEXTURE_3D;
      break;
   case ST_TEXTURE_RECT:
      target = GL_TEXTURE_RECTANGLE_ARB;
      break;
   default:
      return FALSE;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);

   _mesa_lock_texture(ctx, texObj);

   stObj = st_texture_object(texObj);
   /* switch to surface based */
   if (!stObj->surface_based) {
      _mesa_clear_texture_object(ctx, texObj);
      stObj->surface_based = GL_TRUE;
   }

   texImage = _mesa_get_tex_image(ctx, texObj, target, level);
   stImage = st_texture_image(texImage);
   if (tex) {
      mesa_format texFormat = st_pipe_format_to_mesa_format(pipe_format);

      if (util_format_has_alpha(tex->format))
         internalFormat = GL_RGBA;
      else
         internalFormat = GL_RGB;

      _mesa_init_teximage_fields(ctx, texImage,
                                 tex->width0, tex->height0, 1, 0,
                                 internalFormat, texFormat);

      width = tex->width0;
      height = tex->height0;
      depth = tex->depth0;

      /* grow the image size until we hit level = 0 */
      while (level > 0) {
         if (width != 1)
            width <<= 1;
         if (height != 1)
            height <<= 1;
         if (depth != 1)
            depth <<= 1;
         level--;
      }
   }
   else {
      _mesa_clear_texture_image(ctx, texImage);
      width = height = depth = 0;
   }

   pipe_resource_reference(&stImage->pt, tex);
   stObj->width0 = width;
   stObj->height0 = height;
   stObj->depth0 = depth;
   stObj->surface_format = pipe_format;

   _mesa_dirty_texobj(ctx, texObj);
   _mesa_unlock_texture(ctx, texObj);
   
   return TRUE;
}

static void
st_context_copy(struct st_context_iface *stctxi,
                struct st_context_iface *stsrci, unsigned mask)
{
   struct st_context *st = (struct st_context *) stctxi;
   struct st_context *src = (struct st_context *) stsrci;

   _mesa_copy_context(src->ctx, st->ctx, mask);
}

static boolean
st_context_share(struct st_context_iface *stctxi,
                 struct st_context_iface *stsrci)
{
   struct st_context *st = (struct st_context *) stctxi;
   struct st_context *src = (struct st_context *) stsrci;

   return _mesa_share_state(st->ctx, src->ctx);
}

static void
st_context_destroy(struct st_context_iface *stctxi)
{
   struct st_context *st = (struct st_context *) stctxi;
   st_destroy_context(st);
}

static struct st_context_iface *
st_api_create_context(struct st_api *stapi, struct st_manager *smapi,
                      const struct st_context_attribs *attribs,
                      enum st_context_error *error,
                      struct st_context_iface *shared_stctxi)
{
   struct st_context *shared_ctx = (struct st_context *) shared_stctxi;
   struct st_context *st;
   struct pipe_context *pipe;
   struct gl_config mode;
   gl_api api;

   if (!(stapi->profile_mask & (1 << attribs->profile)))
      return NULL;

   switch (attribs->profile) {
   case ST_PROFILE_DEFAULT:
      api = API_OPENGL_COMPAT;
      break;
   case ST_PROFILE_OPENGL_ES1:
      api = API_OPENGLES;
      break;
   case ST_PROFILE_OPENGL_ES2:
      api = API_OPENGLES2;
      break;
   case ST_PROFILE_OPENGL_CORE:
      api = API_OPENGL_CORE;
      break;
   default:
      *error = ST_CONTEXT_ERROR_BAD_API;
      return NULL;
      break;
   }

   pipe = smapi->screen->context_create(smapi->screen, NULL);
   if (!pipe) {
      *error = ST_CONTEXT_ERROR_NO_MEMORY;
      return NULL;
   }

   st_visual_to_context_mode(&attribs->visual, &mode);
   st = st_create_context(api, pipe, &mode, shared_ctx, &attribs->options);
   if (!st) {
      *error = ST_CONTEXT_ERROR_NO_MEMORY;
      pipe->destroy(pipe);
      return NULL;
   }

   if (attribs->flags & ST_CONTEXT_FLAG_DEBUG){
      if (!_mesa_set_debug_state_int(st->ctx, GL_DEBUG_OUTPUT, GL_TRUE)) {
         *error = ST_CONTEXT_ERROR_NO_MEMORY;
         return NULL;
      }
      st->ctx->Const.ContextFlags |= GL_CONTEXT_FLAG_DEBUG_BIT;
   }

   if (attribs->flags & ST_CONTEXT_FLAG_FORWARD_COMPATIBLE)
      st->ctx->Const.ContextFlags |= GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT;

   /* need to perform version check */
   if (attribs->major > 1 || attribs->minor > 0) {
      /* Is the actual version less than the requested version?
       */
      if (st->ctx->Version < attribs->major * 10 + attribs->minor) {
	 *error = ST_CONTEXT_ERROR_BAD_VERSION;
         st_destroy_context(st);
         return NULL;
      }
   }

   st->invalidate_on_gl_viewport =
      smapi->get_param(smapi, ST_MANAGER_BROKEN_INVALIDATE);

   st->iface.destroy = st_context_destroy;
   st->iface.flush = st_context_flush;
   st->iface.teximage = st_context_teximage;
   st->iface.copy = st_context_copy;
   st->iface.share = st_context_share;
   st->iface.st_context_private = (void *) smapi;
   st->iface.cso_context = st->cso_context;
   st->iface.pipe = st->pipe;

   *error = ST_CONTEXT_SUCCESS;
   return &st->iface;
}

static struct st_context_iface *
st_api_get_current(struct st_api *stapi)
{
   GET_CURRENT_CONTEXT(ctx);
   struct st_context *st = (ctx) ? ctx->st : NULL;

   return (st) ? &st->iface : NULL;
}

static struct st_framebuffer *
st_framebuffer_reuse_or_create(struct st_context *st,
                               struct gl_framebuffer *fb,
                               struct st_framebuffer_iface *stfbi)
{
   struct st_framebuffer *cur = st_ws_framebuffer(fb), *stfb = NULL;

   /* dummy framebuffers cant be used as st_framebuffer */
   if (cur && &cur->Base != _mesa_get_incomplete_framebuffer() &&
       cur->iface == stfbi) {
      /* reuse the current stfb */
      st_framebuffer_reference(&stfb, cur);
   }
   else {
      /* create a new one */
      stfb = st_framebuffer_create(st, stfbi);
   }

   return stfb;
}

static boolean
st_api_make_current(struct st_api *stapi, struct st_context_iface *stctxi,
                    struct st_framebuffer_iface *stdrawi,
                    struct st_framebuffer_iface *streadi)
{
   struct st_context *st = (struct st_context *) stctxi;
   struct st_framebuffer *stdraw, *stread;
   boolean ret;

   _glapi_check_multithread();

   if (st) {
      /* reuse or create the draw fb */
      stdraw = st_framebuffer_reuse_or_create(st,
            st->ctx->WinSysDrawBuffer, stdrawi);
      if (streadi != stdrawi) {
         /* do the same for the read fb */
         stread = st_framebuffer_reuse_or_create(st,
               st->ctx->WinSysReadBuffer, streadi);
      }
      else {
         stread = NULL;
         /* reuse the draw fb for the read fb */
         if (stdraw)
            st_framebuffer_reference(&stread, stdraw);
      }

      if (stdraw && stread) {
         st_framebuffer_validate(stdraw, st);
         if (stread != stdraw)
            st_framebuffer_validate(stread, st);

         ret = _mesa_make_current(st->ctx, &stdraw->Base, &stread->Base);

         st->draw_stamp = stdraw->stamp - 1;
         st->read_stamp = stread->stamp - 1;
         st_context_validate(st, stdraw, stread);
      }
      else {
         struct gl_framebuffer *incomplete = _mesa_get_incomplete_framebuffer();
         ret = _mesa_make_current(st->ctx, incomplete, incomplete);
      }

      st_framebuffer_reference(&stdraw, NULL);
      st_framebuffer_reference(&stread, NULL);
   }
   else {
      ret = _mesa_make_current(NULL, NULL, NULL);
   }

   return ret;
}

static st_proc_t
st_api_get_proc_address(struct st_api *stapi, const char *procname)
{
   return (st_proc_t) _glapi_get_proc_address(procname);
}

static void
st_api_destroy(struct st_api *stapi)
{
}

/**
 * Flush the front buffer if the current context renders to the front buffer.
 */
void
st_manager_flush_frontbuffer(struct st_context *st)
{
   struct st_framebuffer *stfb = st_ws_framebuffer(st->ctx->DrawBuffer);
   struct st_renderbuffer *strb = NULL;

   if (stfb)
      strb = st_renderbuffer(stfb->Base.Attachment[BUFFER_FRONT_LEFT].Renderbuffer);
   if (!strb)
      return;

   /* never a dummy fb */
   assert(&stfb->Base != _mesa_get_incomplete_framebuffer());
   stfb->iface->flush_front(&st->iface, stfb->iface, ST_ATTACHMENT_FRONT_LEFT);
}

/**
 * Return the surface of an EGLImage.
 * FIXME: I think this should operate on resources, not surfaces
 */
struct pipe_surface *
st_manager_get_egl_image_surface(struct st_context *st, void *eglimg)
{
   struct st_manager *smapi =
      (struct st_manager *) st->iface.st_context_private;
   struct st_egl_image stimg;
   struct pipe_surface *ps, surf_tmpl;

   if (!smapi || !smapi->get_egl_image)
      return NULL;

   memset(&stimg, 0, sizeof(stimg));
   if (!smapi->get_egl_image(smapi, eglimg, &stimg))
      return NULL;

   u_surface_default_template(&surf_tmpl, stimg.texture);
   surf_tmpl.u.tex.level = stimg.level;
   surf_tmpl.u.tex.first_layer = stimg.layer;
   surf_tmpl.u.tex.last_layer = stimg.layer;
   ps = st->pipe->create_surface(st->pipe, stimg.texture, &surf_tmpl);
   pipe_resource_reference(&stimg.texture, NULL);

   return ps;
}

/**
 * Re-validate the framebuffers.
 */
void
st_manager_validate_framebuffers(struct st_context *st)
{
   struct st_framebuffer *stdraw = st_ws_framebuffer(st->ctx->DrawBuffer);
   struct st_framebuffer *stread = st_ws_framebuffer(st->ctx->ReadBuffer);

   if (stdraw)
      st_framebuffer_validate(stdraw, st);
   if (stread && stread != stdraw)
      st_framebuffer_validate(stread, st);

   st_context_validate(st, stdraw, stread);
}

/**
 * Add a color renderbuffer on demand.
 */
boolean
st_manager_add_color_renderbuffer(struct st_context *st,
                                  struct gl_framebuffer *fb,
                                  gl_buffer_index idx)
{
   struct st_framebuffer *stfb = st_ws_framebuffer(fb);

   /* FBO */
   if (!stfb)
      return FALSE;

   if (stfb->Base.Attachment[idx].Renderbuffer)
      return TRUE;

   switch (idx) {
   case BUFFER_FRONT_LEFT:
   case BUFFER_BACK_LEFT:
   case BUFFER_FRONT_RIGHT:
   case BUFFER_BACK_RIGHT:
      break;
   default:
      return FALSE;
      break;
   }

   if (!st_framebuffer_add_renderbuffer(stfb, idx))
      return FALSE;

   st_framebuffer_update_attachments(stfb);

   /*
    * Force a call to the state tracker manager to validate the
    * new renderbuffer. It might be that there is a window system
    * renderbuffer available.
    */
   if(stfb->iface)
      stfb->iface_stamp = p_atomic_read(&stfb->iface->stamp) - 1;

   st_invalidate_state(st->ctx, _NEW_BUFFERS);

   return TRUE;
}

static unsigned get_version(struct pipe_screen *screen,
                            struct st_config_options *options, gl_api api)
{
   struct gl_constants consts = {0};
   struct gl_extensions extensions = {0};
   GLuint version;

   if ((api == API_OPENGL_COMPAT || api == API_OPENGL_CORE) &&
       _mesa_override_gl_version_contextless(&consts, &api, &version)) {
      return version;
   }

   _mesa_init_constants(&consts, api);
   _mesa_init_extensions(&extensions);

   st_init_limits(screen, &consts, &extensions);
   st_init_extensions(screen, api, &consts, &extensions, options, GL_TRUE);

   return _mesa_get_version(&extensions, &consts, api);
}

static void
st_api_query_versions(struct st_api *stapi, struct st_manager *sm,
                      struct st_config_options *options,
                      int *gl_core_version,
                      int *gl_compat_version,
                      int *gl_es1_version,
                      int *gl_es2_version)
{
   *gl_core_version = get_version(sm->screen, options, API_OPENGL_CORE);
   *gl_compat_version = get_version(sm->screen, options, API_OPENGL_COMPAT);
   *gl_es1_version = get_version(sm->screen, options, API_OPENGLES);
   *gl_es2_version = get_version(sm->screen, options, API_OPENGLES2);
}

static const struct st_api st_gl_api = {
   "Mesa " PACKAGE_VERSION,
   ST_API_OPENGL,
   ST_PROFILE_DEFAULT_MASK |
   ST_PROFILE_OPENGL_CORE_MASK |
   ST_PROFILE_OPENGL_ES1_MASK |
   ST_PROFILE_OPENGL_ES2_MASK |
   0,
   ST_API_FEATURE_MS_VISUALS_MASK,
   st_api_destroy,
   st_api_query_versions,
   st_api_get_proc_address,
   st_api_create_context,
   st_api_make_current,
   st_api_get_current,
};

struct st_api *
st_gl_api_create(void)
{
   return (struct st_api *) &st_gl_api;
}
