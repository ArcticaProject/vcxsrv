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

/**
 * \file pixelstore.c
 * glPixelStore functions.
 */


#include "glheader.h"
#include "bufferobj.h"
#include "context.h"
#include "pixelstore.h"
#include "mfeatures.h"
#include "mtypes.h"


void GLAPIENTRY
_mesa_PixelStorei( GLenum pname, GLint param )
{
   /* NOTE: this call can't be compiled into the display list */
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END(ctx);

   switch (pname) {
      case GL_PACK_SWAP_BYTES:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
	 if (param == (GLint)ctx->Pack.SwapBytes)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
         ctx->Pack.SwapBytes = param ? GL_TRUE : GL_FALSE;
	 break;
      case GL_PACK_LSB_FIRST:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
	 if (param == (GLint)ctx->Pack.LsbFirst)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
         ctx->Pack.LsbFirst = param ? GL_TRUE : GL_FALSE;
	 break;
      case GL_PACK_ROW_LENGTH:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
	 if (param<0) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Pack.RowLength == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Pack.RowLength = param;
	 break;
      case GL_PACK_IMAGE_HEIGHT:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
         if (param<0) {
            _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Pack.ImageHeight == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Pack.ImageHeight = param;
         break;
      case GL_PACK_SKIP_PIXELS:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
	 if (param<0) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Pack.SkipPixels == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Pack.SkipPixels = param;
	 break;
      case GL_PACK_SKIP_ROWS:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
	 if (param<0) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Pack.SkipRows == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Pack.SkipRows = param;
	 break;
      case GL_PACK_SKIP_IMAGES:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
	 if (param<0) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Pack.SkipImages == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Pack.SkipImages = param;
	 break;
      case GL_PACK_ALIGNMENT:
         if (param!=1 && param!=2 && param!=4 && param!=8) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Pack.Alignment == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Pack.Alignment = param;
	 break;
      case GL_PACK_INVERT_MESA:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
         if (!ctx->Extensions.MESA_pack_invert) {
            _mesa_error( ctx, GL_INVALID_ENUM, "glPixelstore(pname)" );
            return;
         }
         if (ctx->Pack.Invert == param)
            return;
         FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
         ctx->Pack.Invert = param;
         break;

      case GL_UNPACK_SWAP_BYTES:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
	 if (param == (GLint)ctx->Unpack.SwapBytes)
	    return;
	 if ((GLint)ctx->Unpack.SwapBytes == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Unpack.SwapBytes = param ? GL_TRUE : GL_FALSE;
         break;
      case GL_UNPACK_LSB_FIRST:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_enum_error;
	 if (param == (GLint)ctx->Unpack.LsbFirst)
	    return;
	 if ((GLint)ctx->Unpack.LsbFirst == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Unpack.LsbFirst = param ? GL_TRUE : GL_FALSE;
	 break;
      case GL_UNPACK_ROW_LENGTH:
         if (ctx->API == API_OPENGLES)
            goto invalid_enum_error;
	 if (param<0) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Unpack.RowLength == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Unpack.RowLength = param;
	 break;
      case GL_UNPACK_IMAGE_HEIGHT:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
         if (param<0) {
            _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Unpack.ImageHeight == param)
	    return;

	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Unpack.ImageHeight = param;
         break;
      case GL_UNPACK_SKIP_PIXELS:
         if (ctx->API == API_OPENGLES)
            goto invalid_enum_error;
	 if (param<0) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Unpack.SkipPixels == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Unpack.SkipPixels = param;
	 break;
      case GL_UNPACK_SKIP_ROWS:
         if (ctx->API == API_OPENGLES)
            goto invalid_enum_error;
	 if (param<0) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Unpack.SkipRows == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Unpack.SkipRows = param;
	 break;
      case GL_UNPACK_SKIP_IMAGES:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_enum_error;
	 if (param < 0) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore(param)" );
	    return;
	 }
	 if (ctx->Unpack.SkipImages == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Unpack.SkipImages = param;
	 break;
      case GL_UNPACK_ALIGNMENT:
         if (param!=1 && param!=2 && param!=4 && param!=8) {
	    _mesa_error( ctx, GL_INVALID_VALUE, "glPixelStore" );
	    return;
	 }
	 if (ctx->Unpack.Alignment == param)
	    return;
	 FLUSH_VERTICES(ctx, _NEW_PACKUNPACK);
	 ctx->Unpack.Alignment = param;
	 break;
      default:
         goto invalid_enum_error;
   }

   return;

invalid_enum_error:
   _mesa_error( ctx, GL_INVALID_ENUM, "glPixelStore" );
   return;
}


void GLAPIENTRY
_mesa_PixelStoref( GLenum pname, GLfloat param )
{
   _mesa_PixelStorei( pname, IROUND(param) );
}



/**
 * Initialize the context's pixel store state.
 */
void
_mesa_init_pixelstore( struct gl_context *ctx )
{
   /* Pixel transfer */
   ctx->Pack.Alignment = 4;
   ctx->Pack.RowLength = 0;
   ctx->Pack.ImageHeight = 0;
   ctx->Pack.SkipPixels = 0;
   ctx->Pack.SkipRows = 0;
   ctx->Pack.SkipImages = 0;
   ctx->Pack.SwapBytes = GL_FALSE;
   ctx->Pack.LsbFirst = GL_FALSE;
   ctx->Pack.Invert = GL_FALSE;
#if FEATURE_EXT_pixel_buffer_object
   _mesa_reference_buffer_object(ctx, &ctx->Pack.BufferObj,
                                 ctx->Shared->NullBufferObj);
#endif
   ctx->Unpack.Alignment = 4;
   ctx->Unpack.RowLength = 0;
   ctx->Unpack.ImageHeight = 0;
   ctx->Unpack.SkipPixels = 0;
   ctx->Unpack.SkipRows = 0;
   ctx->Unpack.SkipImages = 0;
   ctx->Unpack.SwapBytes = GL_FALSE;
   ctx->Unpack.LsbFirst = GL_FALSE;
   ctx->Unpack.Invert = GL_FALSE;
#if FEATURE_EXT_pixel_buffer_object
   _mesa_reference_buffer_object(ctx, &ctx->Unpack.BufferObj,
                                 ctx->Shared->NullBufferObj);
#endif

   /*
    * _mesa_unpack_image() returns image data in this format.  When we
    * execute image commands (glDrawPixels(), glTexImage(), etc) from
    * within display lists we have to be sure to set the current
    * unpacking parameters to these values!
    */
   ctx->DefaultPacking.Alignment = 1;
   ctx->DefaultPacking.RowLength = 0;
   ctx->DefaultPacking.SkipPixels = 0;
   ctx->DefaultPacking.SkipRows = 0;
   ctx->DefaultPacking.ImageHeight = 0;
   ctx->DefaultPacking.SkipImages = 0;
   ctx->DefaultPacking.SwapBytes = GL_FALSE;
   ctx->DefaultPacking.LsbFirst = GL_FALSE;
   ctx->DefaultPacking.Invert = GL_FALSE;
#if FEATURE_EXT_pixel_buffer_object
   _mesa_reference_buffer_object(ctx, &ctx->DefaultPacking.BufferObj,
                                 ctx->Shared->NullBufferObj);
#endif
}
