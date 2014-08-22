/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
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
 * \file buffers.c
 * glReadBuffer, DrawBuffer functions.
 */



#include "glheader.h"
#include "buffers.h"
#include "colormac.h"
#include "context.h"
#include "enums.h"
#include "fbobject.h"
#include "mtypes.h"


#define BAD_MASK ~0u


/**
 * Return bitmask of BUFFER_BIT_* flags indicating which color buffers are
 * available to the rendering context (for drawing or reading).
 * This depends on the type of framebuffer.  For window system framebuffers
 * we look at the framebuffer's visual.  But for user-create framebuffers we
 * look at the number of supported color attachments.
 * \param fb  the framebuffer to draw to, or read from
 * \return  bitmask of BUFFER_BIT_* flags
 */
static GLbitfield
supported_buffer_bitmask(const struct gl_context *ctx,
                         const struct gl_framebuffer *fb)
{
   GLbitfield mask = 0x0;

   if (_mesa_is_user_fbo(fb)) {
      /* A user-created renderbuffer */
      GLuint i;
      for (i = 0; i < ctx->Const.MaxColorAttachments; i++) {
         mask |= (BUFFER_BIT_COLOR0 << i);
      }
   }
   else {
      /* A window system framebuffer */
      GLint i;
      mask = BUFFER_BIT_FRONT_LEFT; /* always have this */
      if (fb->Visual.stereoMode) {
         mask |= BUFFER_BIT_FRONT_RIGHT;
         if (fb->Visual.doubleBufferMode) {
            mask |= BUFFER_BIT_BACK_LEFT | BUFFER_BIT_BACK_RIGHT;
         }
      }
      else if (fb->Visual.doubleBufferMode) {
         mask |= BUFFER_BIT_BACK_LEFT;
      }

      for (i = 0; i < fb->Visual.numAuxBuffers; i++) {
         mask |= (BUFFER_BIT_AUX0 << i);
      }
   }

   return mask;
}


/**
 * Helper routine used by glDrawBuffer and glDrawBuffersARB.
 * Given a GLenum naming one or more color buffers (such as
 * GL_FRONT_AND_BACK), return the corresponding bitmask of BUFFER_BIT_* flags.
 */
static GLbitfield
draw_buffer_enum_to_bitmask(const struct gl_context *ctx, GLenum buffer)
{
   switch (buffer) {
      case GL_NONE:
         return 0;
      case GL_FRONT:
         return BUFFER_BIT_FRONT_LEFT | BUFFER_BIT_FRONT_RIGHT;
      case GL_BACK:
         if (_mesa_is_gles(ctx)) {
            /* Page 181 (page 192 of the PDF) in section 4.2.1 of the OpenGL
             * ES 3.0.1 specification says:
             *
             *     "When draw buffer zero is BACK, color values are written
             *     into the sole buffer for single-buffered contexts, or into
             *     the back buffer for double-buffered contexts."
             *
             * Since there is no stereo rendering in ES 3.0, only return the
             * LEFT bits.  This also satisfies the "n must be 1" requirement.
             *
             * We also do this for GLES 1 and 2 because those APIs have no
             * concept of selecting the front and back buffer anyway and it's
             * convenient to be able to maintain the magic behaviour of
             * GL_BACK in that case.
             */
            if (ctx->DrawBuffer->Visual.doubleBufferMode)
               return BUFFER_BIT_BACK_LEFT;
            return BUFFER_BIT_FRONT_LEFT;
         }
         return BUFFER_BIT_BACK_LEFT | BUFFER_BIT_BACK_RIGHT;
      case GL_RIGHT:
         return BUFFER_BIT_FRONT_RIGHT | BUFFER_BIT_BACK_RIGHT;
      case GL_FRONT_RIGHT:
         return BUFFER_BIT_FRONT_RIGHT;
      case GL_BACK_RIGHT:
         return BUFFER_BIT_BACK_RIGHT;
      case GL_BACK_LEFT:
         return BUFFER_BIT_BACK_LEFT;
      case GL_FRONT_AND_BACK:
         return BUFFER_BIT_FRONT_LEFT | BUFFER_BIT_BACK_LEFT
              | BUFFER_BIT_FRONT_RIGHT | BUFFER_BIT_BACK_RIGHT;
      case GL_LEFT:
         return BUFFER_BIT_FRONT_LEFT | BUFFER_BIT_BACK_LEFT;
      case GL_FRONT_LEFT:
         return BUFFER_BIT_FRONT_LEFT;
      case GL_AUX0:
         return BUFFER_BIT_AUX0;
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
         return 1 << BUFFER_COUNT; /* invalid, but not BAD_MASK */
      case GL_COLOR_ATTACHMENT0_EXT:
         return BUFFER_BIT_COLOR0;
      case GL_COLOR_ATTACHMENT1_EXT:
         return BUFFER_BIT_COLOR1;
      case GL_COLOR_ATTACHMENT2_EXT:
         return BUFFER_BIT_COLOR2;
      case GL_COLOR_ATTACHMENT3_EXT:
         return BUFFER_BIT_COLOR3;
      case GL_COLOR_ATTACHMENT4_EXT:
         return BUFFER_BIT_COLOR4;
      case GL_COLOR_ATTACHMENT5_EXT:
         return BUFFER_BIT_COLOR5;
      case GL_COLOR_ATTACHMENT6_EXT:
         return BUFFER_BIT_COLOR6;
      case GL_COLOR_ATTACHMENT7_EXT:
         return BUFFER_BIT_COLOR7;
      default:
         /* error */
         return BAD_MASK;
   }
}


/**
 * Helper routine used by glReadBuffer.
 * Given a GLenum naming a color buffer, return the index of the corresponding
 * renderbuffer (a BUFFER_* value).
 * return -1 for an invalid buffer.
 */
static GLint
read_buffer_enum_to_index(GLenum buffer)
{
   switch (buffer) {
      case GL_FRONT:
         return BUFFER_FRONT_LEFT;
      case GL_BACK:
         return BUFFER_BACK_LEFT;
      case GL_RIGHT:
         return BUFFER_FRONT_RIGHT;
      case GL_FRONT_RIGHT:
         return BUFFER_FRONT_RIGHT;
      case GL_BACK_RIGHT:
         return BUFFER_BACK_RIGHT;
      case GL_BACK_LEFT:
         return BUFFER_BACK_LEFT;
      case GL_LEFT:
         return BUFFER_FRONT_LEFT;
      case GL_FRONT_LEFT:
         return BUFFER_FRONT_LEFT;
      case GL_AUX0:
         return BUFFER_AUX0;
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
         return BUFFER_COUNT; /* invalid, but not -1 */
      case GL_COLOR_ATTACHMENT0_EXT:
         return BUFFER_COLOR0;
      case GL_COLOR_ATTACHMENT1_EXT:
         return BUFFER_COLOR1;
      case GL_COLOR_ATTACHMENT2_EXT:
         return BUFFER_COLOR2;
      case GL_COLOR_ATTACHMENT3_EXT:
         return BUFFER_COLOR3;
      case GL_COLOR_ATTACHMENT4_EXT:
         return BUFFER_COLOR4;
      case GL_COLOR_ATTACHMENT5_EXT:
         return BUFFER_COLOR5;
      case GL_COLOR_ATTACHMENT6_EXT:
         return BUFFER_COLOR6;
      case GL_COLOR_ATTACHMENT7_EXT:
         return BUFFER_COLOR7;
      default:
         /* error */
         return -1;
   }
}


/**
 * Called by glDrawBuffer().
 * Specify which renderbuffer(s) to draw into for the first color output.
 * <buffer> can name zero, one, two or four renderbuffers!
 * \sa _mesa_DrawBuffers
 *
 * \param buffer  buffer token such as GL_LEFT or GL_FRONT_AND_BACK, etc.
 *
 * Note that the behaviour of this function depends on whether the
 * current ctx->DrawBuffer is a window-system framebuffer or a user-created
 * framebuffer object.
 *   In the former case, we update the per-context ctx->Color.DrawBuffer
 *   state var _and_ the FB's ColorDrawBuffer state.
 *   In the later case, we update the FB's ColorDrawBuffer state only.
 *
 * Furthermore, upon a MakeCurrent() or BindFramebuffer() call, if the
 * new FB is a window system FB, we need to re-update the FB's
 * ColorDrawBuffer state to match the context.  This is handled in
 * _mesa_update_framebuffer().
 *
 * See the GL_EXT_framebuffer_object spec for more info.
 */
void GLAPIENTRY
_mesa_DrawBuffer(GLenum buffer)
{
   GLbitfield destMask;
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0);

   if (MESA_VERBOSE & VERBOSE_API) {
      _mesa_debug(ctx, "glDrawBuffer %s\n", _mesa_lookup_enum_by_nr(buffer));
   }

   if (buffer == GL_NONE) {
      destMask = 0x0;
   }
   else {
      const GLbitfield supportedMask
         = supported_buffer_bitmask(ctx, ctx->DrawBuffer);
      destMask = draw_buffer_enum_to_bitmask(ctx, buffer);
      if (destMask == BAD_MASK) {
         /* totally bogus buffer */
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glDrawBuffer(buffer=0x%x)", buffer);
         return;
      }
      destMask &= supportedMask;
      if (destMask == 0x0) {
         /* none of the named color buffers exist! */
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glDrawBuffer(buffer=0x%x)", buffer);
         return;
      }
   }

   /* if we get here, there's no error so set new state */
   _mesa_drawbuffers(ctx, 1, &buffer, &destMask);

   /*
    * Call device driver function.
    */
   if (ctx->Driver.DrawBuffers)
      ctx->Driver.DrawBuffers(ctx, 1, &buffer);
   else if (ctx->Driver.DrawBuffer)
      ctx->Driver.DrawBuffer(ctx, buffer);
}


/**
 * Called by glDrawBuffersARB; specifies the destination color renderbuffers
 * for N fragment program color outputs.
 * \sa _mesa_DrawBuffer
 * \param n  number of outputs
 * \param buffers  array [n] of renderbuffer names.  Unlike glDrawBuffer, the
 *                 names cannot specify more than one buffer.  For example,
 *                 GL_FRONT_AND_BACK is illegal.
 */
void GLAPIENTRY
_mesa_DrawBuffers(GLsizei n, const GLenum *buffers)
{
   GLint output;
   GLbitfield usedBufferMask, supportedMask;
   GLbitfield destMask[MAX_DRAW_BUFFERS];
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0);

   /* Turns out n==0 is a valid input that should not produce an error.
    * The remaining code below correctly handles the n==0 case.
    *
    * From the OpenGL 3.0 specification, page 258:
    * "An INVALID_VALUE error is generated if n is greater than
    *  MAX_DRAW_BUFFERS."
    */
   if (n < 0 || n > (GLsizei) ctx->Const.MaxDrawBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glDrawBuffersARB(n)");
      return;
   }

   supportedMask = supported_buffer_bitmask(ctx, ctx->DrawBuffer);
   usedBufferMask = 0x0;

   /* From the ES 3.0 specification, page 180:
    * "If the GL is bound to the default framebuffer, then n must be 1
    *  and the constant must be BACK or NONE."
    */
   if (_mesa_is_gles3(ctx) && _mesa_is_winsys_fbo(ctx->DrawBuffer) &&
       (n != 1 || (buffers[0] != GL_NONE && buffers[0] != GL_BACK))) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glDrawBuffers(buffer)");
      return;
   }

   /* complicated error checking... */
   for (output = 0; output < n; output++) {
      if (buffers[output] == GL_NONE) {
         destMask[output] = 0x0;
      }
      else {
         /* Page 259 (page 275 of the PDF) in section 4.2.1 of the OpenGL 3.0
          * spec (20080923) says:
          *
          *     "If the GL is bound to a framebuffer object and DrawBuffers is
          *     supplied with [...] COLOR_ATTACHMENTm where m is greater than
          *     or equal to the value of MAX_COLOR_ATTACHMENTS, then the error
          *     INVALID_OPERATION results."
          */
         if (_mesa_is_user_fbo(ctx->DrawBuffer) && buffers[output] >=
             GL_COLOR_ATTACHMENT0 + ctx->Const.MaxDrawBuffers) {
            _mesa_error(ctx, GL_INVALID_OPERATION, "glDrawBuffersARB(buffer)");
            return;
         }

         destMask[output] = draw_buffer_enum_to_bitmask(ctx, buffers[output]);

         /* From the OpenGL 3.0 specification, page 258:
          * "Each buffer listed in bufs must be one of the values from tables
          *  4.5 or 4.6.  Otherwise, an INVALID_ENUM error is generated.
          */
         if (destMask[output] == BAD_MASK) {
            _mesa_error(ctx, GL_INVALID_ENUM, "glDrawBuffersARB(buffer)");
            return;
         }         

         /* From the OpenGL 4.0 specification, page 256:
          * "For both the default framebuffer and framebuffer objects, the
          *  constants FRONT, BACK, LEFT, RIGHT, and FRONT_AND_BACK are not
          *  valid in the bufs array passed to DrawBuffers, and will result in
          *  the error INVALID_ENUM. This restriction is because these
          *  constants may themselves refer to multiple buffers, as shown in
          *  table 4.4."
          *  Previous versions of the OpenGL specification say INVALID_OPERATION,
          *  but the Khronos conformance tests expect INVALID_ENUM.
          */
         if (_mesa_bitcount(destMask[output]) > 1) {
            _mesa_error(ctx, GL_INVALID_ENUM, "glDrawBuffersARB(buffer)");
            return;
         }

         /* From the OpenGL 3.0 specification, page 259:
          * "If the GL is bound to the default framebuffer and DrawBuffers is
          *  supplied with a constant (other than NONE) that does not indicate
          *  any of the color buffers allocated to the GL context by the window
          *  system, the error INVALID_OPERATION will be generated.
          *
          *  If the GL is bound to a framebuffer object and DrawBuffers is
          *  supplied with a constant from table 4.6 [...] then the error
          *  INVALID_OPERATION results."
          */
         destMask[output] &= supportedMask;
         if (destMask[output] == 0) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glDrawBuffersARB(unsupported buffer)");
            return;
         }

         /* ES 3.0 is even more restrictive.  From the ES 3.0 spec, page 180:
          * "If the GL is bound to a framebuffer object, the ith buffer listed
          *  in bufs must be COLOR_ATTACHMENTi or NONE. [...] INVALID_OPERATION."
          */
         if (_mesa_is_gles3(ctx) && _mesa_is_user_fbo(ctx->DrawBuffer) &&
             buffers[output] != GL_NONE &&
             buffers[output] != GL_COLOR_ATTACHMENT0 + output) {
            _mesa_error(ctx, GL_INVALID_OPERATION, "glDrawBuffers(buffer)");
            return;
         }

         /* From the OpenGL 3.0 specification, page 258:
          * "Except for NONE, a buffer may not appear more than once in the
          *  array pointed to by bufs.  Specifying a buffer more then once will
          *  result in the error INVALID_OPERATION."
          */
         if (destMask[output] & usedBufferMask) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glDrawBuffersARB(duplicated buffer)");
            return;
         }

         /* update bitmask */
         usedBufferMask |= destMask[output];
      }
   }

   /* OK, if we get here, there were no errors so set the new state */
   _mesa_drawbuffers(ctx, n, buffers, destMask);

   /*
    * Call device driver function.  Note that n can be equal to 0,
    * in which case we don't want to reference buffers[0], which
    * may not be valid.
    */
   if (ctx->Driver.DrawBuffers)
      ctx->Driver.DrawBuffers(ctx, n, buffers);
   else if (ctx->Driver.DrawBuffer)
      ctx->Driver.DrawBuffer(ctx, n > 0 ? buffers[0] : GL_NONE);
}


/**
 * Performs necessary state updates when _mesa_drawbuffers makes an
 * actual change.
 */
static void
updated_drawbuffers(struct gl_context *ctx)
{
   FLUSH_VERTICES(ctx, _NEW_BUFFERS);

   if (ctx->API == API_OPENGL_COMPAT && !ctx->Extensions.ARB_ES2_compatibility) {
      struct gl_framebuffer *fb = ctx->DrawBuffer;

      /* Flag the FBO as requiring validation. */
      if (_mesa_is_user_fbo(fb)) {
	 fb->_Status = 0;
      }
   }
}


/**
 * Helper function to set the GL_DRAW_BUFFER state in the context and
 * current FBO.  Called via glDrawBuffer(), glDrawBuffersARB()
 *
 * All error checking will have been done prior to calling this function
 * so nothing should go wrong at this point.
 *
 * \param ctx  current context
 * \param n    number of color outputs to set
 * \param buffers  array[n] of colorbuffer names, like GL_LEFT.
 * \param destMask  array[n] of BUFFER_BIT_* bitmasks which correspond to the
 *                  colorbuffer names.  (i.e. GL_FRONT_AND_BACK =>
 *                  BUFFER_BIT_FRONT_LEFT | BUFFER_BIT_BACK_LEFT).
 */
void
_mesa_drawbuffers(struct gl_context *ctx, GLuint n, const GLenum *buffers,
                  const GLbitfield *destMask)
{
   struct gl_framebuffer *fb = ctx->DrawBuffer;
   GLbitfield mask[MAX_DRAW_BUFFERS];
   GLuint buf;

   if (!destMask) {
      /* compute destMask values now */
      const GLbitfield supportedMask = supported_buffer_bitmask(ctx, fb);
      GLuint output;
      for (output = 0; output < n; output++) {
         mask[output] = draw_buffer_enum_to_bitmask(ctx, buffers[output]);
         ASSERT(mask[output] != BAD_MASK);
         mask[output] &= supportedMask;
      }
      destMask = mask;
   }

   /*
    * destMask[0] may have up to four bits set
    * (ex: glDrawBuffer(GL_FRONT_AND_BACK)).
    * Otherwise, destMask[x] can only have one bit set.
    */
   if (n > 0 && _mesa_bitcount(destMask[0]) > 1) {
      GLuint count = 0, destMask0 = destMask[0];
      while (destMask0) {
         GLint bufIndex = ffs(destMask0) - 1;
         if (fb->_ColorDrawBufferIndexes[count] != bufIndex) {
            updated_drawbuffers(ctx);
            fb->_ColorDrawBufferIndexes[count] = bufIndex;
         }
         count++;
         destMask0 &= ~(1 << bufIndex);
      }
      fb->ColorDrawBuffer[0] = buffers[0];
      fb->_NumColorDrawBuffers = count;
   }
   else {
      GLuint count = 0;
      for (buf = 0; buf < n; buf++ ) {
         if (destMask[buf]) {
            GLint bufIndex = ffs(destMask[buf]) - 1;
            /* only one bit should be set in the destMask[buf] field */
            ASSERT(_mesa_bitcount(destMask[buf]) == 1);
            if (fb->_ColorDrawBufferIndexes[buf] != bufIndex) {
	       updated_drawbuffers(ctx);
               fb->_ColorDrawBufferIndexes[buf] = bufIndex;
            }
            count = buf + 1;
         }
         else {
            if (fb->_ColorDrawBufferIndexes[buf] != -1) {
	       updated_drawbuffers(ctx);
               fb->_ColorDrawBufferIndexes[buf] = -1;
            }
         }
         fb->ColorDrawBuffer[buf] = buffers[buf];
      }
      fb->_NumColorDrawBuffers = count;
   }

   /* set remaining outputs to -1 (GL_NONE) */
   for (buf = fb->_NumColorDrawBuffers; buf < ctx->Const.MaxDrawBuffers; buf++) {
      if (fb->_ColorDrawBufferIndexes[buf] != -1) {
         updated_drawbuffers(ctx);
         fb->_ColorDrawBufferIndexes[buf] = -1;
      }
   }
   for (buf = n; buf < ctx->Const.MaxDrawBuffers; buf++) {
      fb->ColorDrawBuffer[buf] = GL_NONE;
   }

   if (_mesa_is_winsys_fbo(fb)) {
      /* also set context drawbuffer state */
      for (buf = 0; buf < ctx->Const.MaxDrawBuffers; buf++) {
         if (ctx->Color.DrawBuffer[buf] != fb->ColorDrawBuffer[buf]) {
	    updated_drawbuffers(ctx);
            ctx->Color.DrawBuffer[buf] = fb->ColorDrawBuffer[buf];
         }
      }
   }
}


/**
 * Update the current drawbuffer's _ColorDrawBufferIndex[] list, etc.
 * from the context's Color.DrawBuffer[] state.
 * Use when changing contexts.
 */
void
_mesa_update_draw_buffers(struct gl_context *ctx)
{
   /* should be a window system FBO */
   assert(_mesa_is_winsys_fbo(ctx->DrawBuffer));

   _mesa_drawbuffers(ctx, ctx->Const.MaxDrawBuffers,
                     ctx->Color.DrawBuffer, NULL);
}


/**
 * Like \sa _mesa_drawbuffers(), this is a helper function for setting
 * GL_READ_BUFFER state in the context and current FBO.
 * \param ctx  the rendering context
 * \param buffer  GL_FRONT, GL_BACK, GL_COLOR_ATTACHMENT0, etc.
 * \param bufferIndex  the numerical index corresponding to 'buffer'
 */
void
_mesa_readbuffer(struct gl_context *ctx, GLenum buffer, GLint bufferIndex)
{
   struct gl_framebuffer *fb = ctx->ReadBuffer;

   if (_mesa_is_winsys_fbo(fb)) {
      /* Only update the per-context READ_BUFFER state if we're bound to
       * a window-system framebuffer.
       */
      ctx->Pixel.ReadBuffer = buffer;
   }

   fb->ColorReadBuffer = buffer;
   fb->_ColorReadBufferIndex = bufferIndex;

   ctx->NewState |= _NEW_BUFFERS;
}



/**
 * Called by glReadBuffer to set the source renderbuffer for reading pixels.
 * \param mode color buffer such as GL_FRONT, GL_BACK, etc.
 */
void GLAPIENTRY
_mesa_ReadBuffer(GLenum buffer)
{
   struct gl_framebuffer *fb;
   GLbitfield supportedMask;
   GLint srcBuffer;
   GET_CURRENT_CONTEXT(ctx);

   FLUSH_VERTICES(ctx, 0);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glReadBuffer %s\n", _mesa_lookup_enum_by_nr(buffer));

   fb = ctx->ReadBuffer;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glReadBuffer %s\n", _mesa_lookup_enum_by_nr(buffer));

   if (buffer == GL_NONE) {
      /* This is legal--it means that no buffer should be bound for reading. */
      srcBuffer = -1;
   }
   else {
      /* general case / window-system framebuffer */
      srcBuffer = read_buffer_enum_to_index(buffer);
      if (srcBuffer == -1) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glReadBuffer(buffer=0x%x)", buffer);
         return;
      }
      supportedMask = supported_buffer_bitmask(ctx, fb);
      if (((1 << srcBuffer) & supportedMask) == 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glReadBuffer(buffer=0x%x)", buffer);
         return;
      }
   }

   /* OK, all error checking has been completed now */

   _mesa_readbuffer(ctx, buffer, srcBuffer);

   /*
    * Call device driver function.
    */
   if (ctx->Driver.ReadBuffer)
      (*ctx->Driver.ReadBuffer)(ctx, buffer);
}
