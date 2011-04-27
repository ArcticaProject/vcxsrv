/*
 * Mesa 3-D graphics library
 * Version:  7.1
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "glheader.h"
#include "bufferobj.h"
#include "colortab.h"
#include "context.h"
#include "image.h"
#include "macros.h"
#include "mfeatures.h"
#include "mtypes.h"
#include "pack.h"
#include "pbo.h"
#include "state.h"
#include "teximage.h"
#include "texstate.h"
#include "main/dispatch.h"


#if FEATURE_colortable


/**
 * Given an internalFormat token passed to glColorTable,
 * return the corresponding base format.
 * Return -1 if invalid token.
 */
static GLint
base_colortab_format( GLenum format )
{
   switch (format) {
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA8:
      case GL_ALPHA12:
      case GL_ALPHA16:
         return GL_ALPHA;
      case GL_LUMINANCE:
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
         return GL_LUMINANCE;
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
         return GL_LUMINANCE_ALPHA;
      case GL_INTENSITY:
      case GL_INTENSITY4:
      case GL_INTENSITY8:
      case GL_INTENSITY12:
      case GL_INTENSITY16:
         return GL_INTENSITY;
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
         return GL_RGBA;
      default:
         return -1;  /* error */
   }
}



/**
 * Examine table's format and set the component sizes accordingly.
 */
static void
set_component_sizes( struct gl_color_table *table )
{
   /* assuming the ubyte table */
   const GLubyte sz = 8;

   switch (table->_BaseFormat) {
      case GL_ALPHA:
         table->RedSize = 0;
         table->GreenSize = 0;
         table->BlueSize = 0;
         table->AlphaSize = sz;
         table->IntensitySize = 0;
         table->LuminanceSize = 0;
         break;
      case GL_LUMINANCE:
         table->RedSize = 0;
         table->GreenSize = 0;
         table->BlueSize = 0;
         table->AlphaSize = 0;
         table->IntensitySize = 0;
         table->LuminanceSize = sz;
         break;
      case GL_LUMINANCE_ALPHA:
         table->RedSize = 0;
         table->GreenSize = 0;
         table->BlueSize = 0;
         table->AlphaSize = sz;
         table->IntensitySize = 0;
         table->LuminanceSize = sz;
         break;
      case GL_INTENSITY:
         table->RedSize = 0;
         table->GreenSize = 0;
         table->BlueSize = 0;
         table->AlphaSize = 0;
         table->IntensitySize = sz;
         table->LuminanceSize = 0;
         break;
      case GL_RGB:
         table->RedSize = sz;
         table->GreenSize = sz;
         table->BlueSize = sz;
         table->AlphaSize = 0;
         table->IntensitySize = 0;
         table->LuminanceSize = 0;
         break;
      case GL_RGBA:
         table->RedSize = sz;
         table->GreenSize = sz;
         table->BlueSize = sz;
         table->AlphaSize = sz;
         table->IntensitySize = 0;
         table->LuminanceSize = 0;
         break;
      default:
         _mesa_problem(NULL, "unexpected format in set_component_sizes");
   }
}



/**
 * Update/replace all or part of a color table.  Helper function
 * used by _mesa_ColorTable() and _mesa_ColorSubTable().
 * The table->Table buffer should already be allocated.
 * \param start first entry to update
 * \param count number of entries to update
 * \param format format of user-provided table data
 * \param type datatype of user-provided table data
 * \param data user-provided table data
 * \param [rgba]Scale - RGBA scale factors
 * \param [rgba]Bias - RGBA bias factors
 */
static void
store_colortable_entries(struct gl_context *ctx, struct gl_color_table *table,
			 GLsizei start, GLsizei count,
			 GLenum format, GLenum type, const GLvoid *data,
			 GLfloat rScale, GLfloat rBias,
			 GLfloat gScale, GLfloat gBias,
			 GLfloat bScale, GLfloat bBias,
			 GLfloat aScale, GLfloat aBias)
{
   data = _mesa_map_validate_pbo_source(ctx, 
                                        1, &ctx->Unpack, count, 1, 1,
                                        format, type, INT_MAX, data,
                                        "glColor[Sub]Table");
   if (!data)
      return;

   {
      /* convert user-provided data to GLfloat values */
      GLfloat tempTab[MAX_COLOR_TABLE_SIZE * 4];
      GLfloat *tableF;
      GLint i;

      _mesa_unpack_color_span_float(ctx,
                                    count,         /* number of pixels */
                                    table->_BaseFormat, /* dest format */
                                    tempTab,       /* dest address */
                                    format, type,  /* src format/type */
                                    data,          /* src data */
                                    &ctx->Unpack,
                                    IMAGE_CLAMP_BIT); /* transfer ops */

      /* the destination */
      tableF = table->TableF;

      /* Apply scale & bias & clamp now */
      switch (table->_BaseFormat) {
         case GL_INTENSITY:
            for (i = 0; i < count; i++) {
               GLuint j = start + i;
               tableF[j] = CLAMP(tempTab[i] * rScale + rBias, 0.0F, 1.0F);
            }
            break;
         case GL_LUMINANCE:
            for (i = 0; i < count; i++) {
               GLuint j = start + i;
               tableF[j] = CLAMP(tempTab[i] * rScale + rBias, 0.0F, 1.0F);
            }
            break;
         case GL_ALPHA:
            for (i = 0; i < count; i++) {
               GLuint j = start + i;
               tableF[j] = CLAMP(tempTab[i] * aScale + aBias, 0.0F, 1.0F);
            }
            break;
         case GL_LUMINANCE_ALPHA:
            for (i = 0; i < count; i++) {
               GLuint j = start + i;
               tableF[j*2+0] = CLAMP(tempTab[i*2+0] * rScale + rBias, 0.0F, 1.0F);
               tableF[j*2+1] = CLAMP(tempTab[i*2+1] * aScale + aBias, 0.0F, 1.0F);
            }
            break;
         case GL_RGB:
            for (i = 0; i < count; i++) {
               GLuint j = start + i;
               tableF[j*3+0] = CLAMP(tempTab[i*3+0] * rScale + rBias, 0.0F, 1.0F);
               tableF[j*3+1] = CLAMP(tempTab[i*3+1] * gScale + gBias, 0.0F, 1.0F);
               tableF[j*3+2] = CLAMP(tempTab[i*3+2] * bScale + bBias, 0.0F, 1.0F);
            }
            break;
         case GL_RGBA:
            for (i = 0; i < count; i++) {
               GLuint j = start + i;
               tableF[j*4+0] = CLAMP(tempTab[i*4+0] * rScale + rBias, 0.0F, 1.0F);
               tableF[j*4+1] = CLAMP(tempTab[i*4+1] * gScale + gBias, 0.0F, 1.0F);
               tableF[j*4+2] = CLAMP(tempTab[i*4+2] * bScale + bBias, 0.0F, 1.0F);
               tableF[j*4+3] = CLAMP(tempTab[i*4+3] * aScale + aBias, 0.0F, 1.0F);
            }
            break;
         default:
            _mesa_problem(ctx, "Bad format in store_colortable_entries");
            return;
         }
   }

   /* update the ubyte table */
   {
      const GLint comps = _mesa_components_in_format(table->_BaseFormat);
      const GLfloat *tableF = table->TableF + start * comps;
      GLubyte *tableUB = table->TableUB + start * comps;
      GLint i;
      for (i = 0; i < count * comps; i++) {
         CLAMPED_FLOAT_TO_UBYTE(tableUB[i], tableF[i]);
      }
   }

   _mesa_unmap_pbo_source(ctx, &ctx->Unpack);
}



void GLAPIENTRY
_mesa_ColorTable( GLenum target, GLenum internalFormat,
                  GLsizei width, GLenum format, GLenum type,
                  const GLvoid *data )
{
   static const GLfloat one[4] = { 1.0, 1.0, 1.0, 1.0 };
   static const GLfloat zero[4] = { 0.0, 0.0, 0.0, 0.0 };
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = _mesa_get_current_tex_unit(ctx);
   struct gl_texture_object *texObj = NULL;
   struct gl_color_table *table = NULL;
   GLboolean proxy = GL_FALSE;
   GLint baseFormat;
   const GLfloat *scale = one, *bias = zero;
   GLint comps;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx); /* too complex */

   switch (target) {
      case GL_SHARED_TEXTURE_PALETTE_EXT:
         table = &ctx->Texture.Palette;
         break;
      default:
         /* try texture targets */
         {
            struct gl_texture_object *texobj
               = _mesa_select_tex_object(ctx, texUnit, target);
            if (texobj) {
               table = &texobj->Palette;
               proxy = _mesa_is_proxy_texture(target);
            }
            else {
               _mesa_error(ctx, GL_INVALID_ENUM, "glColorTable(target)");
               return;
            }
         }
   }

   assert(table);

   if (!_mesa_is_legal_format_and_type(ctx, format, type) ||
       format == GL_INTENSITY) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glColorTable(format or type)");
      return;
   }

   baseFormat = base_colortab_format(internalFormat);
   if (baseFormat < 0) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glColorTable(internalFormat)");
      return;
   }

   if (width < 0 || (width != 0 && !_mesa_is_pow_two(width))) {
      /* error */
      if (proxy) {
         table->Size = 0;
         table->InternalFormat = (GLenum) 0;
         table->_BaseFormat = (GLenum) 0;
      }
      else {
         _mesa_error(ctx, GL_INVALID_VALUE, "glColorTable(width=%d)", width);
      }
      return;
   }

   if (width > (GLsizei) ctx->Const.MaxColorTableSize) {
      if (proxy) {
         table->Size = 0;
         table->InternalFormat = (GLenum) 0;
         table->_BaseFormat = (GLenum) 0;
      }
      else {
         _mesa_error(ctx, GL_TABLE_TOO_LARGE, "glColorTable(width)");
      }
      return;
   }

   table->Size = width;
   table->InternalFormat = internalFormat;
   table->_BaseFormat = (GLenum) baseFormat;

   comps = _mesa_components_in_format(table->_BaseFormat);
   assert(comps > 0);  /* error should have been caught sooner */

   if (!proxy) {
      _mesa_free_colortable_data(table);

      if (width > 0) {
         table->TableF = (GLfloat *) malloc(comps * width * sizeof(GLfloat));
         table->TableUB = (GLubyte *) malloc(comps * width * sizeof(GLubyte));

	 if (!table->TableF || !table->TableUB) {
	    _mesa_error(ctx, GL_OUT_OF_MEMORY, "glColorTable");
	    return;
	 }

	 store_colortable_entries(ctx, table,
				  0, width,  /* start, count */
				  format, type, data,
				  scale[0], bias[0],
				  scale[1], bias[1],
				  scale[2], bias[2],
				  scale[3], bias[3]);
      }
   } /* proxy */

   /* do this after the table's Type and Format are set */
   set_component_sizes(table);

   if (texObj || target == GL_SHARED_TEXTURE_PALETTE_EXT) {
      /* texture object palette, texObj==NULL means the shared palette */
      if (ctx->Driver.UpdateTexturePalette) {
         (*ctx->Driver.UpdateTexturePalette)( ctx, texObj );
      }
   }

   ctx->NewState |= _NEW_PIXEL;
}



void GLAPIENTRY
_mesa_ColorSubTable( GLenum target, GLsizei start,
                     GLsizei count, GLenum format, GLenum type,
                     const GLvoid *data )
{
   static const GLfloat one[4] = { 1.0, 1.0, 1.0, 1.0 };
   static const GLfloat zero[4] = { 0.0, 0.0, 0.0, 0.0 };
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = _mesa_get_current_tex_unit(ctx);
   struct gl_texture_object *texObj = NULL;
   struct gl_color_table *table = NULL;
   const GLfloat *scale = one, *bias = zero;

   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);

   switch (target) {
      case GL_SHARED_TEXTURE_PALETTE_EXT:
         table = &ctx->Texture.Palette;
         break;
      default:
         /* try texture targets */
         texObj = _mesa_select_tex_object(ctx, texUnit, target);
         if (texObj && !_mesa_is_proxy_texture(target)) {
            table = &texObj->Palette;
         }
         else {
            _mesa_error(ctx, GL_INVALID_ENUM, "glColorSubTable(target)");
            return;
         }
   }

   assert(table);

   if (!_mesa_is_legal_format_and_type(ctx, format, type) ||
       format == GL_INTENSITY) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glColorSubTable(format or type)");
      return;
   }

   if (count < 1) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glColorSubTable(count)");
      return;
   }

   /* error should have been caught sooner */
   assert(_mesa_components_in_format(table->_BaseFormat) > 0);

   if (start + count > (GLint) table->Size) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glColorSubTable(count)");
      return;
   }

   if (!table->TableF || !table->TableUB) {
      /* a GL_OUT_OF_MEMORY error would have been recorded previously */
      return;
   }

   store_colortable_entries(ctx, table, start, count,
			    format, type, data,
                            scale[0], bias[0],
                            scale[1], bias[1],
                            scale[2], bias[2],
                            scale[3], bias[3]);

   if (texObj || target == GL_SHARED_TEXTURE_PALETTE_EXT) {
      /* per-texture object palette */
      if (ctx->Driver.UpdateTexturePalette) {
         (*ctx->Driver.UpdateTexturePalette)( ctx, texObj );
      }
   }

   ctx->NewState |= _NEW_PIXEL;
}



static void GLAPIENTRY
_mesa_CopyColorTable(GLenum target, GLenum internalformat,
                     GLint x, GLint y, GLsizei width)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);

   if (!ctx->ReadBuffer->_ColorReadBuffer) {
      return;      /* no readbuffer - OK */
   }

   ctx->Driver.CopyColorTable( ctx, target, internalformat, x, y, width );
}



static void GLAPIENTRY
_mesa_CopyColorSubTable(GLenum target, GLsizei start,
                        GLint x, GLint y, GLsizei width)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);

   if (!ctx->ReadBuffer->_ColorReadBuffer) {
      return;      /* no readbuffer - OK */
   }

   ctx->Driver.CopyColorSubTable( ctx, target, start, x, y, width );
}



static void GLAPIENTRY
_mesa_GetnColorTableARB( GLenum target, GLenum format, GLenum type,
                         GLsizei bufSize, GLvoid *data )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = _mesa_get_current_tex_unit(ctx);
   struct gl_color_table *table = NULL;
   GLfloat rgba[MAX_COLOR_TABLE_SIZE][4];
   GLbitfield transferOps = 0;
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);

   if (ctx->NewState) {
      _mesa_update_state(ctx);
   }

   switch (target) {
      case GL_SHARED_TEXTURE_PALETTE_EXT:
         table = &ctx->Texture.Palette;
         break;
      default:
         /* try texture targets */
         {
            struct gl_texture_object *texobj
               = _mesa_select_tex_object(ctx, texUnit, target);
            if (texobj && !_mesa_is_proxy_texture(target)) {
               table = &texobj->Palette;
            }
            else {
               _mesa_error(ctx, GL_INVALID_ENUM, "glGetColorTable(target)");
               return;
            }
         }
   }

   ASSERT(table);

   if (table->Size <= 0) {
      return;
   }

   switch (table->_BaseFormat) {
   case GL_ALPHA:
      {
         GLuint i;
         for (i = 0; i < table->Size; i++) {
            rgba[i][RCOMP] = 0;
            rgba[i][GCOMP] = 0;
            rgba[i][BCOMP] = 0;
            rgba[i][ACOMP] = table->TableF[i];
         }
      }
      break;
   case GL_LUMINANCE:
      {
         GLuint i;
         for (i = 0; i < table->Size; i++) {
            rgba[i][RCOMP] =
            rgba[i][GCOMP] =
            rgba[i][BCOMP] = table->TableF[i];
            rgba[i][ACOMP] = 1.0F;
         }
      }
      break;
   case GL_LUMINANCE_ALPHA:
      {
         GLuint i;
         for (i = 0; i < table->Size; i++) {
            rgba[i][RCOMP] =
            rgba[i][GCOMP] =
            rgba[i][BCOMP] = table->TableF[i*2+0];
            rgba[i][ACOMP] = table->TableF[i*2+1];
         }
      }
      break;
   case GL_INTENSITY:
      {
         GLuint i;
         for (i = 0; i < table->Size; i++) {
            rgba[i][RCOMP] =
            rgba[i][GCOMP] =
            rgba[i][BCOMP] =
            rgba[i][ACOMP] = table->TableF[i];
         }
      }
      break;
   case GL_RGB:
      {
         GLuint i;
         for (i = 0; i < table->Size; i++) {
            rgba[i][RCOMP] = table->TableF[i*3+0];
            rgba[i][GCOMP] = table->TableF[i*3+1];
            rgba[i][BCOMP] = table->TableF[i*3+2];
            rgba[i][ACOMP] = 1.0F;
         }
      }
      break;
   case GL_RGBA:
      memcpy(rgba, table->TableF, 4 * table->Size * sizeof(GLfloat));
      break;
   default:
      _mesa_problem(ctx, "bad table format in glGetColorTable");
      return;
   }

   data = _mesa_map_validate_pbo_dest(ctx, 
                                      1, &ctx->Pack, table->Size, 1, 1,
                                      format, type, bufSize, data,
                                      "glGetColorTable");
   if (!data)
      return;

   /* TODO: is this correct? */
   if(ctx->Color._ClampReadColor)
      transferOps |= IMAGE_CLAMP_BIT;

   _mesa_pack_rgba_span_float(ctx, table->Size, rgba,
                              format, type, data, &ctx->Pack, transferOps);

   _mesa_unmap_pbo_dest(ctx, &ctx->Pack);
}


static void GLAPIENTRY
_mesa_GetColorTable( GLenum target, GLenum format,
                     GLenum type, GLvoid *data )
{
   _mesa_GetnColorTableARB(target, format, type, INT_MAX, data);
}


static void GLAPIENTRY
_mesa_ColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
   /* no extensions use this function */
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);
   _mesa_error(ctx, GL_INVALID_ENUM, "glColorTableParameterfv(target)");
}



static void GLAPIENTRY
_mesa_ColorTableParameteriv(GLenum target, GLenum pname, const GLint *params)
{
   /* no extensions use this function */
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);
   _mesa_error(ctx, GL_INVALID_ENUM, "glColorTableParameteriv(target)");
}



static void GLAPIENTRY
_mesa_GetColorTableParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = _mesa_get_current_tex_unit(ctx);
   struct gl_color_table *table = NULL;
   ASSERT_OUTSIDE_BEGIN_END(ctx);

   switch (target) {
      case GL_SHARED_TEXTURE_PALETTE_EXT:
         table = &ctx->Texture.Palette;
         break;
      default:
         /* try texture targets */
         {
            struct gl_texture_object *texobj
               = _mesa_select_tex_object(ctx, texUnit, target);
            if (texobj) {
               table = &texobj->Palette;
            }
            else {
               _mesa_error(ctx, GL_INVALID_ENUM,
                           "glGetColorTableParameterfv(target)");
               return;
            }
         }
   }

   assert(table);

   switch (pname) {
      case GL_COLOR_TABLE_FORMAT:
         *params = (GLfloat) table->InternalFormat;
         break;
      case GL_COLOR_TABLE_WIDTH:
         *params = (GLfloat) table->Size;
         break;
      case GL_COLOR_TABLE_RED_SIZE:
         *params = (GLfloat) table->RedSize;
         break;
      case GL_COLOR_TABLE_GREEN_SIZE:
         *params = (GLfloat) table->GreenSize;
         break;
      case GL_COLOR_TABLE_BLUE_SIZE:
         *params = (GLfloat) table->BlueSize;
         break;
      case GL_COLOR_TABLE_ALPHA_SIZE:
         *params = (GLfloat) table->AlphaSize;
         break;
      case GL_COLOR_TABLE_LUMINANCE_SIZE:
         *params = (GLfloat) table->LuminanceSize;
         break;
      case GL_COLOR_TABLE_INTENSITY_SIZE:
         *params = (GLfloat) table->IntensitySize;
         break;
      default:
         _mesa_error(ctx, GL_INVALID_ENUM, "glGetColorTableParameterfv(pname)" );
         return;
   }
}



static void GLAPIENTRY
_mesa_GetColorTableParameteriv( GLenum target, GLenum pname, GLint *params )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_unit *texUnit = _mesa_get_current_tex_unit(ctx);
   struct gl_color_table *table = NULL;
   ASSERT_OUTSIDE_BEGIN_END(ctx);

   switch (target) {
      case GL_SHARED_TEXTURE_PALETTE_EXT:
         table = &ctx->Texture.Palette;
         break;
      default:
         /* Try texture targets */
         {
            struct gl_texture_object *texobj
               = _mesa_select_tex_object(ctx, texUnit, target);
            if (texobj) {
               table = &texobj->Palette;
            }
            else {
               _mesa_error(ctx, GL_INVALID_ENUM,
                           "glGetColorTableParameteriv(target)");
               return;
            }
         }
   }

   assert(table);

   switch (pname) {
      case GL_COLOR_TABLE_FORMAT:
         *params = table->InternalFormat;
         break;
      case GL_COLOR_TABLE_WIDTH:
         *params = table->Size;
         break;
      case GL_COLOR_TABLE_RED_SIZE:
         *params = table->RedSize;
         break;
      case GL_COLOR_TABLE_GREEN_SIZE:
         *params = table->GreenSize;
         break;
      case GL_COLOR_TABLE_BLUE_SIZE:
         *params = table->BlueSize;
         break;
      case GL_COLOR_TABLE_ALPHA_SIZE:
         *params = table->AlphaSize;
         break;
      case GL_COLOR_TABLE_LUMINANCE_SIZE:
         *params = table->LuminanceSize;
         break;
      case GL_COLOR_TABLE_INTENSITY_SIZE:
         *params = table->IntensitySize;
         break;
      default:
         _mesa_error(ctx, GL_INVALID_ENUM, "glGetColorTableParameteriv(pname)" );
         return;
   }
}


void
_mesa_init_colortable_dispatch(struct _glapi_table *disp)
{
   SET_ColorSubTable(disp, _mesa_ColorSubTable);
   SET_ColorTable(disp, _mesa_ColorTable);
   SET_ColorTableParameterfv(disp, _mesa_ColorTableParameterfv);
   SET_ColorTableParameteriv(disp, _mesa_ColorTableParameteriv);
   SET_CopyColorSubTable(disp, _mesa_CopyColorSubTable);
   SET_CopyColorTable(disp, _mesa_CopyColorTable);
   SET_GetColorTable(disp, _mesa_GetColorTable);
   SET_GetColorTableParameterfv(disp, _mesa_GetColorTableParameterfv);
   SET_GetColorTableParameteriv(disp, _mesa_GetColorTableParameteriv);

   /* GL_ARB_robustness */
   SET_GetnColorTableARB(disp, _mesa_GetnColorTableARB);
}


#endif /* FEATURE_colortable */


/**********************************************************************/
/*****                      Initialization                        *****/
/**********************************************************************/


void
_mesa_init_colortable( struct gl_color_table *p )
{
   p->TableF = NULL;
   p->TableUB = NULL;
   p->Size = 0;
   p->InternalFormat = GL_RGBA;
}



void
_mesa_free_colortable_data( struct gl_color_table *p )
{
   if (p->TableF) {
      free(p->TableF);
      p->TableF = NULL;
   }
   if (p->TableUB) {
      free(p->TableUB);
      p->TableUB = NULL;
   }
}
