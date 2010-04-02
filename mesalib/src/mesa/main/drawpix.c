/*
 * Mesa 3-D graphics library
 * Version:  7.1
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "glheader.h"
#include "imports.h"
#include "bufferobj.h"
#include "context.h"
#include "drawpix.h"
#include "enums.h"
#include "feedback.h"
#include "framebuffer.h"
#include "readpix.h"
#include "state.h"
#include "main/dispatch.h"


#if FEATURE_drawpix


/**
 * If a fragment program is enabled, check that it's valid.
 * \return GL_TRUE if valid, GL_FALSE otherwise
 */
static GLboolean
valid_fragment_program(GLcontext *ctx)
{
   return !(ctx->FragmentProgram.Enabled && !ctx->FragmentProgram._Enabled);
}


/*
 * Execute glDrawPixels
 */
static void GLAPIENTRY
_mesa_DrawPixels( GLsizei width, GLsizei height,
                  GLenum format, GLenum type, const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);

   if (width < 0 || height < 0) {
      _mesa_error( ctx, GL_INVALID_VALUE, "glDrawPixels(width or height < 0" );
      return;
   }

   /* We're not using the current vertex program, and the driver may install
    * it's own.
    */
   _mesa_set_vp_override(ctx, GL_TRUE);

   if (ctx->NewState) {
      _mesa_update_state(ctx);
   }

   if (!valid_fragment_program(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glDrawPixels (invalid fragment program)");
      goto end;
   }

   if (_mesa_error_check_format_type(ctx, format, type, GL_TRUE)) {
      /* the error was already recorded */
      goto end;
   }

   if (ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "glDrawPixels(incomplete framebuffer)" );
      goto end;
   }

   if (!ctx->Current.RasterPosValid) {
      goto end; /* no-op, not an error */
   }

   if (ctx->RenderMode == GL_RENDER) {
      if (width > 0 && height > 0) {
         /* Round, to satisfy conformance tests (matches SGI's OpenGL) */
         GLint x = IROUND(ctx->Current.RasterPos[0]);
         GLint y = IROUND(ctx->Current.RasterPos[1]);

         if (ctx->Unpack.BufferObj->Name) {
            /* unpack from PBO */
            if (!_mesa_validate_pbo_access(2, &ctx->Unpack, width, height, 1,
                                           format, type, pixels)) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glDrawPixels(invalid PBO access)");
               goto end;
            }
            if (_mesa_bufferobj_mapped(ctx->Unpack.BufferObj)) {
               /* buffer is mapped - that's an error */
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glDrawPixels(PBO is mapped)");
               goto end;
            }
         }

         ctx->Driver.DrawPixels(ctx, x, y, width, height, format, type,
                                &ctx->Unpack, pixels);
      }
   }
   else if (ctx->RenderMode == GL_FEEDBACK) {
      /* Feedback the current raster pos info */
      FLUSH_CURRENT( ctx, 0 );
      _mesa_feedback_token( ctx, (GLfloat) (GLint) GL_DRAW_PIXEL_TOKEN );
      _mesa_feedback_vertex( ctx,
                             ctx->Current.RasterPos,
                             ctx->Current.RasterColor,
                             ctx->Current.RasterTexCoords[0] );
   }
   else {
      ASSERT(ctx->RenderMode == GL_SELECT);
      /* Do nothing.  See OpenGL Spec, Appendix B, Corollary 6. */
   }

end:
   _mesa_set_vp_override(ctx, GL_FALSE);
}


static void GLAPIENTRY
_mesa_CopyPixels( GLint srcx, GLint srcy, GLsizei width, GLsizei height,
                  GLenum type )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);

   if (width < 0 || height < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glCopyPixels(width or height < 0)");
      return;
   }

   /* Note: more detailed 'type' checking is done by the
    * _mesa_source/dest_buffer_exists() calls below.  That's where we
    * check if the stencil buffer exists, etc.
    */
   if (type != GL_COLOR &&
       type != GL_DEPTH &&
       type != GL_STENCIL &&
       type != GL_DEPTH_STENCIL) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glCopyPixels(type=%s)",
                  _mesa_lookup_enum_by_nr(type));
      return;
   }

   /* We're not using the current vertex program, and the driver may install
    * it's own.
    */
   _mesa_set_vp_override(ctx, GL_TRUE);

   if (ctx->NewState) {
      _mesa_update_state(ctx);
   }

   if (!valid_fragment_program(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyPixels (invalid fragment program)");
      goto end;
   }

   if (ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT ||
       ctx->ReadBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "glCopyPixels(incomplete framebuffer)" );
      goto end;
   }

   if (!_mesa_source_buffer_exists(ctx, type) ||
       !_mesa_dest_buffer_exists(ctx, type)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyPixels(missing source or dest buffer)");
      goto end;
   }

   if (!ctx->Current.RasterPosValid || width ==0 || height == 0) {
      goto end; /* no-op, not an error */
   }

   if (ctx->RenderMode == GL_RENDER) {
      /* Round to satisfy conformance tests (matches SGI's OpenGL) */
      if (width > 0 && height > 0) {
         GLint destx = IROUND(ctx->Current.RasterPos[0]);
         GLint desty = IROUND(ctx->Current.RasterPos[1]);
         ctx->Driver.CopyPixels( ctx, srcx, srcy, width, height, destx, desty,
                                 type );
      }
   }
   else if (ctx->RenderMode == GL_FEEDBACK) {
      FLUSH_CURRENT( ctx, 0 );
      _mesa_feedback_token( ctx, (GLfloat) (GLint) GL_COPY_PIXEL_TOKEN );
      _mesa_feedback_vertex( ctx, 
                             ctx->Current.RasterPos,
                             ctx->Current.RasterColor,
                             ctx->Current.RasterTexCoords[0] );
   }
   else {
      ASSERT(ctx->RenderMode == GL_SELECT);
      /* Do nothing.  See OpenGL Spec, Appendix B, Corollary 6. */
   }

end:
   _mesa_set_vp_override(ctx, GL_FALSE);
}


static void GLAPIENTRY
_mesa_Bitmap( GLsizei width, GLsizei height,
              GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove,
              const GLubyte *bitmap )
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);

   if (width < 0 || height < 0) {
      _mesa_error( ctx, GL_INVALID_VALUE, "glBitmap(width or height < 0)" );
      return;
   }

   if (!ctx->Current.RasterPosValid) {
      return;    /* do nothing */
   }

   if (ctx->NewState) {
      _mesa_update_state(ctx);
   }

   if (!valid_fragment_program(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glBitmap (invalid fragment program)");
      return;
   }

   if (ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "glBitmap(incomplete framebuffer)");
      return;
   }

   if (ctx->RenderMode == GL_RENDER) {
      /* Truncate, to satisfy conformance tests (matches SGI's OpenGL). */
      if (width > 0 && height > 0) {
         const GLfloat epsilon = 0.0001F;
         GLint x = IFLOOR(ctx->Current.RasterPos[0] + epsilon - xorig);
         GLint y = IFLOOR(ctx->Current.RasterPos[1] + epsilon - yorig);

         if (ctx->Unpack.BufferObj->Name) {
            /* unpack from PBO */
            if (!_mesa_validate_pbo_access(2, &ctx->Unpack, width, height, 1,
                                           GL_COLOR_INDEX, GL_BITMAP,
                                           (GLvoid *) bitmap)) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glBitmap(invalid PBO access)");
               return;
            }
            if (_mesa_bufferobj_mapped(ctx->Unpack.BufferObj)) {
               /* buffer is mapped - that's an error */
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glBitmap(PBO is mapped)");
               return;
            }
         }

         ctx->Driver.Bitmap( ctx, x, y, width, height, &ctx->Unpack, bitmap );
      }
   }
#if _HAVE_FULL_GL
   else if (ctx->RenderMode == GL_FEEDBACK) {
      FLUSH_CURRENT(ctx, 0);
      _mesa_feedback_token( ctx, (GLfloat) (GLint) GL_BITMAP_TOKEN );
      _mesa_feedback_vertex( ctx,
                             ctx->Current.RasterPos,
                             ctx->Current.RasterColor,
                             ctx->Current.RasterTexCoords[0] );
   }
   else {
      ASSERT(ctx->RenderMode == GL_SELECT);
      /* Do nothing.  See OpenGL Spec, Appendix B, Corollary 6. */
   }
#endif

   /* update raster position */
   ctx->Current.RasterPos[0] += xmove;
   ctx->Current.RasterPos[1] += ymove;
}


void
_mesa_init_drawpix_dispatch(struct _glapi_table *disp)
{
   SET_Bitmap(disp, _mesa_Bitmap);
   SET_CopyPixels(disp, _mesa_CopyPixels);
   SET_DrawPixels(disp, _mesa_DrawPixels);
}


#endif /* FEATURE_drawpix */
