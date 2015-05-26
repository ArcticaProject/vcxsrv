/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
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
 * \file teximage.c
 * Texture image-related functions.
 */

#include <stdbool.h>
#include "glheader.h"
#include "bufferobj.h"
#include "context.h"
#include "enums.h"
#include "fbobject.h"
#include "framebuffer.h"
#include "hash.h"
#include "image.h"
#include "imports.h"
#include "macros.h"
#include "multisample.h"
#include "pixelstore.h"
#include "state.h"
#include "texcompress.h"
#include "texcompress_cpal.h"
#include "teximage.h"
#include "texobj.h"
#include "texstate.h"
#include "texstorage.h"
#include "textureview.h"
#include "mtypes.h"
#include "glformats.h"
#include "texstore.h"
#include "pbo.h"


/**
 * State changes which we care about for glCopyTex[Sub]Image() calls.
 * In particular, we care about pixel transfer state and buffer state
 * (such as glReadBuffer to make sure we read from the right renderbuffer).
 */
#define NEW_COPY_TEX_STATE (_NEW_BUFFERS | _NEW_PIXEL)

/**
 * Returns a corresponding internal floating point format for a given base
 * format as specifed by OES_texture_float. In case of GL_FLOAT, the internal
 * format needs to be a 32 bit component and in case of GL_HALF_FLOAT_OES it
 * needs to be a 16 bit component.
 *
 * For example, given base format GL_RGBA, type GL_Float return GL_RGBA32F_ARB.
 */
static GLenum
adjust_for_oes_float_texture(GLenum format, GLenum type)
{
   switch (type) {
   case GL_FLOAT:
      switch (format) {
      case GL_RGBA:
         return GL_RGBA32F;
      case GL_RGB:
         return GL_RGB32F;
      case GL_ALPHA:
         return GL_ALPHA32F_ARB;
      case GL_LUMINANCE:
         return GL_LUMINANCE32F_ARB;
      case GL_LUMINANCE_ALPHA:
         return GL_LUMINANCE_ALPHA32F_ARB;
      default:
         break;
      }
      break;

   case GL_HALF_FLOAT_OES:
      switch (format) {
      case GL_RGBA:
         return GL_RGBA16F;
      case GL_RGB:
         return GL_RGB16F;
      case GL_ALPHA:
         return GL_ALPHA16F_ARB;
      case GL_LUMINANCE:
         return GL_LUMINANCE16F_ARB;
      case GL_LUMINANCE_ALPHA:
         return GL_LUMINANCE_ALPHA16F_ARB;
      default:
         break;
      }
      break;

   default:
      break;
   }

   return format;
}

/**
 * Return the simple base format for a given internal texture format.
 * For example, given GL_LUMINANCE12_ALPHA4, return GL_LUMINANCE_ALPHA.
 *
 * \param ctx GL context.
 * \param internalFormat the internal texture format token or 1, 2, 3, or 4.
 *
 * \return the corresponding \u base internal format (GL_ALPHA, GL_LUMINANCE,
 * GL_LUMANCE_ALPHA, GL_INTENSITY, GL_RGB, or GL_RGBA), or -1 if invalid enum.
 *
 * This is the format which is used during texture application (i.e. the
 * texture format and env mode determine the arithmetic used.
 */
GLint
_mesa_base_tex_format( struct gl_context *ctx, GLint internalFormat )
{
   switch (internalFormat) {
   case GL_ALPHA:
   case GL_ALPHA4:
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
      return (ctx->API != API_OPENGL_CORE) ? GL_ALPHA : -1;
   case 1:
   case GL_LUMINANCE:
   case GL_LUMINANCE4:
   case GL_LUMINANCE8:
   case GL_LUMINANCE12:
   case GL_LUMINANCE16:
      return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE : -1;
   case 2:
   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE4_ALPHA4:
   case GL_LUMINANCE6_ALPHA2:
   case GL_LUMINANCE8_ALPHA8:
   case GL_LUMINANCE12_ALPHA4:
   case GL_LUMINANCE12_ALPHA12:
   case GL_LUMINANCE16_ALPHA16:
      return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE_ALPHA : -1;
   case GL_INTENSITY:
   case GL_INTENSITY4:
   case GL_INTENSITY8:
   case GL_INTENSITY12:
   case GL_INTENSITY16:
      return (ctx->API != API_OPENGL_CORE) ? GL_INTENSITY : -1;
   case 3:
      return (ctx->API != API_OPENGL_CORE) ? GL_RGB : -1;
   case GL_RGB:
   case GL_R3_G3_B2:
   case GL_RGB4:
   case GL_RGB5:
   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      return GL_RGB;
   case 4:
      return (ctx->API != API_OPENGL_CORE) ? GL_RGBA : -1;
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
      ; /* fallthrough */
   }

   /* GL_BGRA can be an internal format *only* in OpenGL ES (1.x or 2.0).
    */
   if (_mesa_is_gles(ctx)) {
      switch (internalFormat) {
      case GL_BGRA:
         return GL_RGBA;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.ARB_ES2_compatibility) {
      switch (internalFormat) {
      case GL_RGB565:
         return GL_RGB;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.ARB_depth_texture) {
      switch (internalFormat) {
      case GL_DEPTH_COMPONENT:
      case GL_DEPTH_COMPONENT16:
      case GL_DEPTH_COMPONENT24:
      case GL_DEPTH_COMPONENT32:
         return GL_DEPTH_COMPONENT;
      case GL_DEPTH_STENCIL:
      case GL_DEPTH24_STENCIL8:
         return GL_DEPTH_STENCIL;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.ARB_stencil_texturing) {
      switch (internalFormat) {
      case GL_STENCIL_INDEX:
      case GL_STENCIL_INDEX1:
      case GL_STENCIL_INDEX4:
      case GL_STENCIL_INDEX8:
      case GL_STENCIL_INDEX16:
         return GL_STENCIL_INDEX;
      default:
         ; /* fallthrough */
      }
   }

   switch (internalFormat) {
   case GL_COMPRESSED_ALPHA:
      return GL_ALPHA;
   case GL_COMPRESSED_LUMINANCE:
      return GL_LUMINANCE;
   case GL_COMPRESSED_LUMINANCE_ALPHA:
      return GL_LUMINANCE_ALPHA;
   case GL_COMPRESSED_INTENSITY:
      return GL_INTENSITY;
   case GL_COMPRESSED_RGB:
      return GL_RGB;
   case GL_COMPRESSED_RGBA:
      return GL_RGBA;
   default:
      ; /* fallthrough */
   }

   if (ctx->Extensions.TDFX_texture_compression_FXT1) {
      switch (internalFormat) {
      case GL_COMPRESSED_RGB_FXT1_3DFX:
         return GL_RGB;
      case GL_COMPRESSED_RGBA_FXT1_3DFX:
         return GL_RGBA;
      default:
         ; /* fallthrough */
      }
   }

   /* Assume that the ANGLE flag will always be set if the EXT flag is set.
    */
   if (ctx->Extensions.ANGLE_texture_compression_dxt) {
      switch (internalFormat) {
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
         return GL_RGB;
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
         return GL_RGBA;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_is_desktop_gl(ctx)
       && ctx->Extensions.ANGLE_texture_compression_dxt) {
      switch (internalFormat) {
      case GL_RGB_S3TC:
      case GL_RGB4_S3TC:
         return GL_RGB;
      case GL_RGBA_S3TC:
      case GL_RGBA4_S3TC:
         return GL_RGBA;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.MESA_ycbcr_texture) {
      if (internalFormat == GL_YCBCR_MESA)
         return GL_YCBCR_MESA;
   }

   if (ctx->Extensions.ARB_texture_float) {
      switch (internalFormat) {
      case GL_ALPHA16F_ARB:
      case GL_ALPHA32F_ARB:
         return GL_ALPHA;
      case GL_RGBA16F_ARB:
      case GL_RGBA32F_ARB:
         return GL_RGBA;
      case GL_RGB16F_ARB:
      case GL_RGB32F_ARB:
         return GL_RGB;
      case GL_INTENSITY16F_ARB:
      case GL_INTENSITY32F_ARB:
         return GL_INTENSITY;
      case GL_LUMINANCE16F_ARB:
      case GL_LUMINANCE32F_ARB:
         return GL_LUMINANCE;
      case GL_LUMINANCE_ALPHA16F_ARB:
      case GL_LUMINANCE_ALPHA32F_ARB:
         return GL_LUMINANCE_ALPHA;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.EXT_texture_snorm) {
      switch (internalFormat) {
      case GL_RED_SNORM:
      case GL_R8_SNORM:
      case GL_R16_SNORM:
         return GL_RED;
      case GL_RG_SNORM:
      case GL_RG8_SNORM:
      case GL_RG16_SNORM:
         return GL_RG;
      case GL_RGB_SNORM:
      case GL_RGB8_SNORM:
      case GL_RGB16_SNORM:
         return GL_RGB;
      case GL_RGBA_SNORM:
      case GL_RGBA8_SNORM:
      case GL_RGBA16_SNORM:
         return GL_RGBA;
      case GL_ALPHA_SNORM:
      case GL_ALPHA8_SNORM:
      case GL_ALPHA16_SNORM:
         return GL_ALPHA;
      case GL_LUMINANCE_SNORM:
      case GL_LUMINANCE8_SNORM:
      case GL_LUMINANCE16_SNORM:
         return GL_LUMINANCE;
      case GL_LUMINANCE_ALPHA_SNORM:
      case GL_LUMINANCE8_ALPHA8_SNORM:
      case GL_LUMINANCE16_ALPHA16_SNORM:
         return GL_LUMINANCE_ALPHA;
      case GL_INTENSITY_SNORM:
      case GL_INTENSITY8_SNORM:
      case GL_INTENSITY16_SNORM:
         return GL_INTENSITY;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.EXT_texture_sRGB) {
      switch (internalFormat) {
      case GL_SRGB_EXT:
      case GL_SRGB8_EXT:
      case GL_COMPRESSED_SRGB_EXT:
         return GL_RGB;
      case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
         return ctx->Extensions.EXT_texture_compression_s3tc ? GL_RGB : -1;
      case GL_SRGB_ALPHA_EXT:
      case GL_SRGB8_ALPHA8_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_EXT:
         return GL_RGBA;
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
         return ctx->Extensions.EXT_texture_compression_s3tc ? GL_RGBA : -1;
      case GL_SLUMINANCE_ALPHA_EXT:
      case GL_SLUMINANCE8_ALPHA8_EXT:
      case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
         return GL_LUMINANCE_ALPHA;
      case GL_SLUMINANCE_EXT:
      case GL_SLUMINANCE8_EXT:
      case GL_COMPRESSED_SLUMINANCE_EXT:
         return GL_LUMINANCE;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Version >= 30 ||
       ctx->Extensions.EXT_texture_integer) {
      switch (internalFormat) {
      case GL_RGBA8UI_EXT:
      case GL_RGBA16UI_EXT:
      case GL_RGBA32UI_EXT:
      case GL_RGBA8I_EXT:
      case GL_RGBA16I_EXT:
      case GL_RGBA32I_EXT:
      case GL_RGB10_A2UI:
         return GL_RGBA;
      case GL_RGB8UI_EXT:
      case GL_RGB16UI_EXT:
      case GL_RGB32UI_EXT:
      case GL_RGB8I_EXT:
      case GL_RGB16I_EXT:
      case GL_RGB32I_EXT:
         return GL_RGB;
      }
   }

   if (ctx->Extensions.EXT_texture_integer) {
      switch (internalFormat) {
      case GL_ALPHA8UI_EXT:
      case GL_ALPHA16UI_EXT:
      case GL_ALPHA32UI_EXT:
      case GL_ALPHA8I_EXT:
      case GL_ALPHA16I_EXT:
      case GL_ALPHA32I_EXT:
         return GL_ALPHA;
      case GL_INTENSITY8UI_EXT:
      case GL_INTENSITY16UI_EXT:
      case GL_INTENSITY32UI_EXT:
      case GL_INTENSITY8I_EXT:
      case GL_INTENSITY16I_EXT:
      case GL_INTENSITY32I_EXT:
         return GL_INTENSITY;
      case GL_LUMINANCE8UI_EXT:
      case GL_LUMINANCE16UI_EXT:
      case GL_LUMINANCE32UI_EXT:
      case GL_LUMINANCE8I_EXT:
      case GL_LUMINANCE16I_EXT:
      case GL_LUMINANCE32I_EXT:
         return GL_LUMINANCE;
      case GL_LUMINANCE_ALPHA8UI_EXT:
      case GL_LUMINANCE_ALPHA16UI_EXT:
      case GL_LUMINANCE_ALPHA32UI_EXT:
      case GL_LUMINANCE_ALPHA8I_EXT:
      case GL_LUMINANCE_ALPHA16I_EXT:
      case GL_LUMINANCE_ALPHA32I_EXT:
         return GL_LUMINANCE_ALPHA;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.ARB_texture_rg) {
      switch (internalFormat) {
      case GL_R16F:
      case GL_R32F:
	 if (!ctx->Extensions.ARB_texture_float)
	    break;
         return GL_RED;
      case GL_R8I:
      case GL_R8UI:
      case GL_R16I:
      case GL_R16UI:
      case GL_R32I:
      case GL_R32UI:
	 if (ctx->Version < 30 && !ctx->Extensions.EXT_texture_integer)
	    break;
	 /* FALLTHROUGH */
      case GL_R8:
      case GL_R16:
      case GL_RED:
      case GL_COMPRESSED_RED:
         return GL_RED;

      case GL_RG16F:
      case GL_RG32F:
	 if (!ctx->Extensions.ARB_texture_float)
	    break;
         return GL_RG;
      case GL_RG8I:
      case GL_RG8UI:
      case GL_RG16I:
      case GL_RG16UI:
      case GL_RG32I:
      case GL_RG32UI:
	 if (ctx->Version < 30 && !ctx->Extensions.EXT_texture_integer)
	    break;
	 /* FALLTHROUGH */
      case GL_RG:
      case GL_RG8:
      case GL_RG16:
      case GL_COMPRESSED_RG:
         return GL_RG;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.EXT_texture_shared_exponent) {
      switch (internalFormat) {
      case GL_RGB9_E5_EXT:
         return GL_RGB;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.EXT_packed_float) {
      switch (internalFormat) {
      case GL_R11F_G11F_B10F_EXT:
         return GL_RGB;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.ARB_depth_buffer_float) {
      switch (internalFormat) {
      case GL_DEPTH_COMPONENT32F:
         return GL_DEPTH_COMPONENT;
      case GL_DEPTH32F_STENCIL8:
         return GL_DEPTH_STENCIL;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.ARB_texture_compression_rgtc) {
      switch (internalFormat) {
      case GL_COMPRESSED_RED_RGTC1:
      case GL_COMPRESSED_SIGNED_RED_RGTC1:
         return GL_RED;
      case GL_COMPRESSED_RG_RGTC2:
      case GL_COMPRESSED_SIGNED_RG_RGTC2:
         return GL_RG;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.EXT_texture_compression_latc) {
      switch (internalFormat) {
      case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
      case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
         return GL_LUMINANCE;
      case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
      case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
         return GL_LUMINANCE_ALPHA;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.ATI_texture_compression_3dc) {
      switch (internalFormat) {
      case GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI:
         return GL_LUMINANCE_ALPHA;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->Extensions.OES_compressed_ETC1_RGB8_texture) {
      switch (internalFormat) {
      case GL_ETC1_RGB8_OES:
         return GL_RGB;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_is_gles3(ctx) || ctx->Extensions.ARB_ES3_compatibility) {
      switch (internalFormat) {
      case GL_COMPRESSED_RGB8_ETC2:
      case GL_COMPRESSED_SRGB8_ETC2:
         return GL_RGB;
      case GL_COMPRESSED_RGBA8_ETC2_EAC:
      case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
      case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
         return GL_RGBA;
      case GL_COMPRESSED_R11_EAC:
      case GL_COMPRESSED_SIGNED_R11_EAC:
         return GL_RED;
      case GL_COMPRESSED_RG11_EAC:
      case GL_COMPRESSED_SIGNED_RG11_EAC:
         return GL_RG;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_is_desktop_gl(ctx) &&
       ctx->Extensions.ARB_texture_compression_bptc) {
      switch (internalFormat) {
      case GL_COMPRESSED_RGBA_BPTC_UNORM:
      case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
         return GL_RGBA;
      case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
      case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
         return GL_RGB;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->API == API_OPENGLES) {
      switch (internalFormat) {
      case GL_PALETTE4_RGB8_OES:
      case GL_PALETTE4_R5_G6_B5_OES:
      case GL_PALETTE8_RGB8_OES:
      case GL_PALETTE8_R5_G6_B5_OES:
	 return GL_RGB;
      case GL_PALETTE4_RGBA8_OES:
      case GL_PALETTE8_RGB5_A1_OES:
      case GL_PALETTE4_RGBA4_OES:
      case GL_PALETTE4_RGB5_A1_OES:
      case GL_PALETTE8_RGBA8_OES:
      case GL_PALETTE8_RGBA4_OES:
	 return GL_RGBA;
      default:
         ; /* fallthrough */
      }
   }

   return -1; /* error */
}


/**
 * For cube map faces, return a face index in [0,5].
 * For other targets return 0;
 */
GLuint
_mesa_tex_target_to_face(GLenum target)
{
   if (_mesa_is_cube_face(target))
      return (GLuint) target - (GLuint) GL_TEXTURE_CUBE_MAP_POSITIVE_X;
   else
      return 0;
}



/**
 * Install gl_texture_image in a gl_texture_object according to the target
 * and level parameters.
 * 
 * \param tObj texture object.
 * \param target texture target.
 * \param level image level.
 * \param texImage texture image.
 */
static void
set_tex_image(struct gl_texture_object *tObj,
              GLenum target, GLint level,
              struct gl_texture_image *texImage)
{
   const GLuint face = _mesa_tex_target_to_face(target);

   assert(tObj);
   assert(texImage);
   if (target == GL_TEXTURE_RECTANGLE_NV || target == GL_TEXTURE_EXTERNAL_OES)
      assert(level == 0);

   tObj->Image[face][level] = texImage;

   /* Set the 'back' pointer */
   texImage->TexObject = tObj;
   texImage->Level = level;
   texImage->Face = face;
}


/**
 * Allocate a texture image structure.
 *
 * Called via ctx->Driver.NewTextureImage() unless overriden by a device
 * driver.
 *
 * \return a pointer to gl_texture_image struct with all fields initialized to
 * zero.
 */
struct gl_texture_image *
_mesa_new_texture_image( struct gl_context *ctx )
{
   (void) ctx;
   return CALLOC_STRUCT(gl_texture_image);
}


/**
 * Free a gl_texture_image and associated data.
 * This function is a fallback called via ctx->Driver.DeleteTextureImage().
 *
 * \param texImage texture image.
 *
 * Free the texture image structure and the associated image data.
 */
void
_mesa_delete_texture_image(struct gl_context *ctx,
                           struct gl_texture_image *texImage)
{
   /* Free texImage->Data and/or any other driver-specific texture
    * image storage.
    */
   assert(ctx->Driver.FreeTextureImageBuffer);
   ctx->Driver.FreeTextureImageBuffer( ctx, texImage );
   free(texImage);
}


/**
 * Test if a target is a proxy target.
 *
 * \param target texture target.
 *
 * \return GL_TRUE if the target is a proxy target, GL_FALSE otherwise.
 */
GLboolean
_mesa_is_proxy_texture(GLenum target)
{
   unsigned i;
   static const GLenum targets[] = {
      GL_PROXY_TEXTURE_1D,
      GL_PROXY_TEXTURE_2D,
      GL_PROXY_TEXTURE_3D,
      GL_PROXY_TEXTURE_CUBE_MAP,
      GL_PROXY_TEXTURE_RECTANGLE,
      GL_PROXY_TEXTURE_1D_ARRAY,
      GL_PROXY_TEXTURE_2D_ARRAY,
      GL_PROXY_TEXTURE_CUBE_MAP_ARRAY,
      GL_PROXY_TEXTURE_2D_MULTISAMPLE,
      GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY
   };
   /*
    * NUM_TEXTURE_TARGETS should match number of terms above, except there's no
    * proxy for GL_TEXTURE_BUFFER and GL_TEXTURE_EXTERNAL_OES.
    */
   STATIC_ASSERT(NUM_TEXTURE_TARGETS == ARRAY_SIZE(targets) + 2);

   for (i = 0; i < ARRAY_SIZE(targets); ++i)
      if (target == targets[i])
         return GL_TRUE;
   return GL_FALSE;
}


/**
 * Test if a target is an array target.
 *
 * \param target texture target.
 *
 * \return true if the target is an array target, false otherwise.
 */
bool
_mesa_is_array_texture(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return true;
   default:
      return false;
   };
}


/**
 * Return the proxy target which corresponds to the given texture target
 */
static GLenum
proxy_target(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
      return GL_PROXY_TEXTURE_1D;
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
      return GL_PROXY_TEXTURE_2D;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      return GL_PROXY_TEXTURE_3D;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
   case GL_TEXTURE_CUBE_MAP_ARB:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
      return GL_PROXY_TEXTURE_CUBE_MAP_ARB;
   case GL_TEXTURE_RECTANGLE_NV:
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      return GL_PROXY_TEXTURE_RECTANGLE_NV;
   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
      return GL_PROXY_TEXTURE_1D_ARRAY_EXT;
   case GL_TEXTURE_2D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
      return GL_PROXY_TEXTURE_2D_ARRAY_EXT;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      return GL_PROXY_TEXTURE_CUBE_MAP_ARRAY;
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      return GL_PROXY_TEXTURE_2D_MULTISAMPLE;
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY;
   default:
      _mesa_problem(NULL, "unexpected target in proxy_target()");
      return 0;
   }
}




/**
 * Get a texture image pointer from a texture object, given a texture
 * target and mipmap level.  The target and level parameters should
 * have already been error-checked.
 *
 * \param texObj texture unit.
 * \param target texture target.
 * \param level image level.
 *
 * \return pointer to the texture image structure, or NULL on failure.
 */
struct gl_texture_image *
_mesa_select_tex_image(const struct gl_texture_object *texObj,
		                 GLenum target, GLint level)
{
   const GLuint face = _mesa_tex_target_to_face(target);

   assert(texObj);
   assert(level >= 0);
   assert(level < MAX_TEXTURE_LEVELS);

   return texObj->Image[face][level];
}


/**
 * Like _mesa_select_tex_image() but if the image doesn't exist, allocate
 * it and install it.  Only return NULL if passed a bad parameter or run
 * out of memory.
 */
struct gl_texture_image *
_mesa_get_tex_image(struct gl_context *ctx, struct gl_texture_object *texObj,
                    GLenum target, GLint level)
{
   struct gl_texture_image *texImage;

   if (!texObj)
      return NULL;

   texImage = _mesa_select_tex_image(texObj, target, level);
   if (!texImage) {
      texImage = ctx->Driver.NewTextureImage(ctx);
      if (!texImage) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "texture image allocation");
         return NULL;
      }

      set_tex_image(texObj, target, level, texImage);
   }

   return texImage;
}


/**
 * Return pointer to the specified proxy texture image.
 * Note that proxy textures are per-context, not per-texture unit.
 * \return pointer to texture image or NULL if invalid target, invalid
 *         level, or out of memory.
 */
static struct gl_texture_image *
get_proxy_tex_image(struct gl_context *ctx, GLenum target, GLint level)
{
   struct gl_texture_image *texImage;
   GLuint texIndex;

   if (level < 0)
      return NULL;

   switch (target) {
   case GL_PROXY_TEXTURE_1D:
      if (level >= ctx->Const.MaxTextureLevels)
         return NULL;
      texIndex = TEXTURE_1D_INDEX;
      break;
   case GL_PROXY_TEXTURE_2D:
      if (level >= ctx->Const.MaxTextureLevels)
         return NULL;
      texIndex = TEXTURE_2D_INDEX;
      break;
   case GL_PROXY_TEXTURE_3D:
      if (level >= ctx->Const.Max3DTextureLevels)
         return NULL;
      texIndex = TEXTURE_3D_INDEX;
      break;
   case GL_PROXY_TEXTURE_CUBE_MAP:
      if (level >= ctx->Const.MaxCubeTextureLevels)
         return NULL;
      texIndex = TEXTURE_CUBE_INDEX;
      break;
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      if (level > 0)
         return NULL;
      texIndex = TEXTURE_RECT_INDEX;
      break;
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
      if (level >= ctx->Const.MaxTextureLevels)
         return NULL;
      texIndex = TEXTURE_1D_ARRAY_INDEX;
      break;
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
      if (level >= ctx->Const.MaxTextureLevels)
         return NULL;
      texIndex = TEXTURE_2D_ARRAY_INDEX;
      break;
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      if (level >= ctx->Const.MaxCubeTextureLevels)
         return NULL;
      texIndex = TEXTURE_CUBE_ARRAY_INDEX;
      break;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      if (level > 0)
         return 0;
      texIndex = TEXTURE_2D_MULTISAMPLE_INDEX;
      break;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      if (level > 0)
         return 0;
      texIndex = TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX;
      break;
   default:
      return NULL;
   }

   texImage = ctx->Texture.ProxyTex[texIndex]->Image[0][level];
   if (!texImage) {
      texImage = ctx->Driver.NewTextureImage(ctx);
      if (!texImage) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "proxy texture allocation");
         return NULL;
      }
      ctx->Texture.ProxyTex[texIndex]->Image[0][level] = texImage;
      /* Set the 'back' pointer */
      texImage->TexObject = ctx->Texture.ProxyTex[texIndex];
   }
   return texImage;
}


/**
 * Get the maximum number of allowed mipmap levels.
 *
 * \param ctx GL context.
 * \param target texture target.
 *
 * \return the maximum number of allowed mipmap levels for the given
 * texture target, or zero if passed a bad target.
 *
 * \sa gl_constants.
 */
GLint
_mesa_max_texture_levels(struct gl_context *ctx, GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
      return ctx->Const.MaxTextureLevels;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      return ctx->Const.Max3DTextureLevels;
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
      return ctx->Extensions.ARB_texture_cube_map
         ? ctx->Const.MaxCubeTextureLevels : 0;
   case GL_TEXTURE_RECTANGLE_NV:
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      return ctx->Extensions.NV_texture_rectangle ? 1 : 0;
   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
   case GL_TEXTURE_2D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
      return ctx->Extensions.EXT_texture_array
         ? ctx->Const.MaxTextureLevels : 0;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      return ctx->Extensions.ARB_texture_cube_map_array
         ? ctx->Const.MaxCubeTextureLevels : 0;
   case GL_TEXTURE_BUFFER:
      return ctx->API == API_OPENGL_CORE &&
             ctx->Extensions.ARB_texture_buffer_object ? 1 : 0;
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return _mesa_is_desktop_gl(ctx)
         && ctx->Extensions.ARB_texture_multisample
         ? 1 : 0;
   case GL_TEXTURE_EXTERNAL_OES:
      /* fall-through */
   default:
      return 0; /* bad target */
   }
}


/**
 * Return number of dimensions per mipmap level for the given texture target.
 */
GLint
_mesa_get_texture_dimensions(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
      return 1;
   case GL_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_CUBE_MAP:
   case GL_PROXY_TEXTURE_2D:
   case GL_PROXY_TEXTURE_RECTANGLE:
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_TEXTURE_1D_ARRAY:
   case GL_PROXY_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      return 2;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_PROXY_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return 3;
   case GL_TEXTURE_BUFFER:
      /* fall-through */
   default:
      _mesa_problem(NULL, "invalid target 0x%x in get_texture_dimensions()",
                    target);
      return 2;
   }
}


/**
 * Check if a texture target can have more than one layer.
 */
GLboolean
_mesa_tex_target_is_layered(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
   case GL_PROXY_TEXTURE_RECTANGLE:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_BUFFER:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_TEXTURE_EXTERNAL_OES:
      return GL_FALSE;

   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_1D_ARRAY:
   case GL_PROXY_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_2D_ARRAY:
   case GL_PROXY_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return GL_TRUE;

   default:
      assert(!"Invalid texture target.");
      return GL_FALSE;
   }
}


/**
 * Return the number of layers present in the given level of an array,
 * cubemap or 3D texture.  If the texture is not layered return zero.
 */
GLuint
_mesa_get_texture_layers(const struct gl_texture_object *texObj, GLint level)
{
   assert(level >= 0 && level < MAX_TEXTURE_LEVELS);

   switch (texObj->Target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_BUFFER:
   case GL_TEXTURE_EXTERNAL_OES:
      return 0;

   case GL_TEXTURE_CUBE_MAP:
      return 6;

   case GL_TEXTURE_1D_ARRAY: {
      struct gl_texture_image *img = texObj->Image[0][level];
      return img ? img->Height : 0;
   }

   case GL_TEXTURE_3D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: {
      struct gl_texture_image *img = texObj->Image[0][level];
      return img ? img->Depth : 0;
   }

   default:
      assert(!"Invalid texture target.");
      return 0;
   }
}


/**
 * Return the maximum number of mipmap levels for the given target
 * and the dimensions.
 * The dimensions are expected not to include the border.
 */
GLsizei
_mesa_get_tex_max_num_levels(GLenum target, GLsizei width, GLsizei height,
                             GLsizei depth)
{
   GLsizei size;

   switch (target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_1D_ARRAY:
   case GL_PROXY_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D_ARRAY:
      size = width;
      break;
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      size = width;
      break;
   case GL_TEXTURE_2D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_PROXY_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D_ARRAY:
      size = MAX2(width, height);
      break;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      size = MAX3(width, height, depth);
      break;
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_RECTANGLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return 1;
   default:
      assert(0);
      return 1;
   }

   return _mesa_logbase2(size) + 1;
}


#if 000 /* not used anymore */
/*
 * glTexImage[123]D can accept a NULL image pointer.  In this case we
 * create a texture image with unspecified image contents per the OpenGL
 * spec.
 */
static GLubyte *
make_null_texture(GLint width, GLint height, GLint depth, GLenum format)
{
   const GLint components = _mesa_components_in_format(format);
   const GLint numPixels = width * height * depth;
   GLubyte *data = (GLubyte *) malloc(numPixels * components * sizeof(GLubyte));

#ifdef DEBUG
   /*
    * Let's see if anyone finds this.  If glTexImage2D() is called with
    * a NULL image pointer then load the texture image with something
    * interesting instead of leaving it indeterminate.
    */
   if (data) {
      static const char message[8][32] = {
         "   X   X  XXXXX   XXX     X    ",
         "   XX XX  X      X   X   X X   ",
         "   X X X  X      X      X   X  ",
         "   X   X  XXXX    XXX   XXXXX  ",
         "   X   X  X          X  X   X  ",
         "   X   X  X      X   X  X   X  ",
         "   X   X  XXXXX   XXX   X   X  ",
         "                               "
      };

      GLubyte *imgPtr = data;
      GLint h, i, j, k;
      for (h = 0; h < depth; h++) {
         for (i = 0; i < height; i++) {
            GLint srcRow = 7 - (i % 8);
            for (j = 0; j < width; j++) {
               GLint srcCol = j % 32;
               GLubyte texel = (message[srcRow][srcCol]=='X') ? 255 : 70;
               for (k = 0; k < components; k++) {
                  *imgPtr++ = texel;
               }
            }
         }
      }
   }
#endif

   return data;
}
#endif



/**
 * Set the size and format-related fields of a gl_texture_image struct
 * to zero.  This is used when a proxy texture test fails.
 */
static void
clear_teximage_fields(struct gl_texture_image *img)
{
   assert(img);
   img->_BaseFormat = 0;
   img->InternalFormat = 0;
   img->Border = 0;
   img->Width = 0;
   img->Height = 0;
   img->Depth = 0;
   img->Width2 = 0;
   img->Height2 = 0;
   img->Depth2 = 0;
   img->WidthLog2 = 0;
   img->HeightLog2 = 0;
   img->DepthLog2 = 0;
   img->TexFormat = MESA_FORMAT_NONE;
   img->NumSamples = 0;
   img->FixedSampleLocations = GL_TRUE;
}


/**
 * Initialize basic fields of the gl_texture_image struct.
 *
 * \param ctx GL context.
 * \param img texture image structure to be initialized.
 * \param width image width.
 * \param height image height.
 * \param depth image depth.
 * \param border image border.
 * \param internalFormat internal format.
 * \param format  the actual hardware format (one of MESA_FORMAT_*)
 * \param numSamples  number of samples per texel, or zero for non-MS.
 * \param fixedSampleLocations  are sample locations fixed?
 *
 * Fills in the fields of \p img with the given information.
 * Note: width, height and depth include the border.
 */
static void
init_teximage_fields_ms(struct gl_context *ctx,
                        struct gl_texture_image *img,
                        GLsizei width, GLsizei height, GLsizei depth,
                        GLint border, GLenum internalFormat,
                        mesa_format format,
                        GLuint numSamples, GLboolean fixedSampleLocations)
{
   GLenum target;
   assert(img);
   assert(width >= 0);
   assert(height >= 0);
   assert(depth >= 0);

   target = img->TexObject->Target;
   img->_BaseFormat = _mesa_base_tex_format( ctx, internalFormat );
   assert(img->_BaseFormat != -1);
   img->InternalFormat = internalFormat;
   img->Border = border;
   img->Width = width;
   img->Height = height;
   img->Depth = depth;

   img->Width2 = width - 2 * border;   /* == 1 << img->WidthLog2; */
   img->WidthLog2 = _mesa_logbase2(img->Width2);

   img->NumSamples = 0;
   img->FixedSampleLocations = GL_TRUE;

   switch(target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_BUFFER:
   case GL_PROXY_TEXTURE_1D:
      if (height == 0)
         img->Height2 = 0;
      else
         img->Height2 = 1;
      img->HeightLog2 = 0;
      if (depth == 0)
         img->Depth2 = 0;
      else
         img->Depth2 = 1;
      img->DepthLog2 = 0;
      break;
   case GL_TEXTURE_1D_ARRAY:
   case GL_PROXY_TEXTURE_1D_ARRAY:
      img->Height2 = height; /* no border */
      img->HeightLog2 = 0; /* not used */
      if (depth == 0)
         img->Depth2 = 0;
      else
         img->Depth2 = 1;
      img->DepthLog2 = 0;
      break;
   case GL_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_PROXY_TEXTURE_2D:
   case GL_PROXY_TEXTURE_RECTANGLE:
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      img->Height2 = height - 2 * border; /* == 1 << img->HeightLog2; */
      img->HeightLog2 = _mesa_logbase2(img->Height2);
      if (depth == 0)
         img->Depth2 = 0;
      else
         img->Depth2 = 1;
      img->DepthLog2 = 0;
      break;
   case GL_TEXTURE_2D_ARRAY:
   case GL_PROXY_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      img->Height2 = height - 2 * border; /* == 1 << img->HeightLog2; */
      img->HeightLog2 = _mesa_logbase2(img->Height2);
      img->Depth2 = depth; /* no border */
      img->DepthLog2 = 0; /* not used */
      break;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      img->Height2 = height - 2 * border; /* == 1 << img->HeightLog2; */
      img->HeightLog2 = _mesa_logbase2(img->Height2);
      img->Depth2 = depth - 2 * border;   /* == 1 << img->DepthLog2; */
      img->DepthLog2 = _mesa_logbase2(img->Depth2);
      break;
   default:
      _mesa_problem(NULL, "invalid target 0x%x in _mesa_init_teximage_fields()",
                    target);
   }

   img->MaxNumLevels =
      _mesa_get_tex_max_num_levels(target,
                                   img->Width2, img->Height2, img->Depth2);
   img->TexFormat = format;
   img->NumSamples = numSamples;
   img->FixedSampleLocations = fixedSampleLocations;
}


void
_mesa_init_teximage_fields(struct gl_context *ctx,
                           struct gl_texture_image *img,
                           GLsizei width, GLsizei height, GLsizei depth,
                           GLint border, GLenum internalFormat,
                           mesa_format format)
{
   init_teximage_fields_ms(ctx, img, width, height, depth, border,
                           internalFormat, format, 0, GL_TRUE);
}


/**
 * Free and clear fields of the gl_texture_image struct.
 *
 * \param ctx GL context.
 * \param texImage texture image structure to be cleared.
 *
 * After the call, \p texImage will have no data associated with it.  Its
 * fields are cleared so that its parent object will test incomplete.
 */
void
_mesa_clear_texture_image(struct gl_context *ctx,
                          struct gl_texture_image *texImage)
{
   ctx->Driver.FreeTextureImageBuffer(ctx, texImage);
   clear_teximage_fields(texImage);
}


/**
 * Check the width, height, depth and border of a texture image are legal.
 * Used by all the glTexImage, glCompressedTexImage and glCopyTexImage
 * functions.
 * The target and level parameters will have already been validated.
 * \return GL_TRUE if size is OK, GL_FALSE otherwise.
 */
GLboolean
_mesa_legal_texture_dimensions(struct gl_context *ctx, GLenum target,
                               GLint level, GLint width, GLint height,
                               GLint depth, GLint border)
{
   GLint maxSize;

   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
      maxSize = 1 << (ctx->Const.MaxTextureLevels - 1); /* level zero size */
      maxSize >>= level;  /* level size */
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !_mesa_is_pow_two(width - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      maxSize = 1 << (ctx->Const.MaxTextureLevels - 1);
      maxSize >>= level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !_mesa_is_pow_two(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !_mesa_is_pow_two(height - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      maxSize = 1 << (ctx->Const.Max3DTextureLevels - 1);
      maxSize >>= level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (depth < 2 * border || depth > 2 * border + maxSize)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !_mesa_is_pow_two(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !_mesa_is_pow_two(height - 2 * border))
            return GL_FALSE;
         if (depth > 0 && !_mesa_is_pow_two(depth - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_RECTANGLE_NV:
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      if (level != 0)
         return GL_FALSE;
      maxSize = ctx->Const.MaxTextureRectSize;
      if (width < 0 || width > maxSize)
         return GL_FALSE;
      if (height < 0 || height > maxSize)
         return GL_FALSE;
      return GL_TRUE;

   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
      maxSize = 1 << (ctx->Const.MaxCubeTextureLevels - 1);
      maxSize >>= level;
      if (width != height)
         return GL_FALSE;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !_mesa_is_pow_two(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !_mesa_is_pow_two(height - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
      maxSize = 1 << (ctx->Const.MaxTextureLevels - 1);
      maxSize >>= level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 0 || height > ctx->Const.MaxArrayTextureLayers)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !_mesa_is_pow_two(width - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_2D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      maxSize = 1 << (ctx->Const.MaxTextureLevels - 1);
      maxSize >>= level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (depth < 0 || depth > ctx->Const.MaxArrayTextureLayers)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !_mesa_is_pow_two(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !_mesa_is_pow_two(height - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      maxSize = 1 << (ctx->Const.MaxCubeTextureLevels - 1);
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (depth < 0 || depth > ctx->Const.MaxArrayTextureLayers || depth % 6)
         return GL_FALSE;
      if (width != height)
         return GL_FALSE;
      if (level >= ctx->Const.MaxCubeTextureLevels)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !_mesa_is_pow_two(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !_mesa_is_pow_two(height - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;
   default:
      _mesa_problem(ctx, "Invalid target in _mesa_legal_texture_dimensions()");
      return GL_FALSE;
   }
}


/**
 * Do error checking of xoffset, yoffset, zoffset, width, height and depth
 * for glTexSubImage, glCopyTexSubImage and glCompressedTexSubImage.
 * \param destImage  the destination texture image.
 * \return GL_TRUE if error found, GL_FALSE otherwise.
 */
static GLboolean
error_check_subtexture_dimensions(struct gl_context *ctx, GLuint dims,
                                  const struct gl_texture_image *destImage,
                                  GLint xoffset, GLint yoffset, GLint zoffset,
                                  GLsizei subWidth, GLsizei subHeight,
                                  GLsizei subDepth, const char *func)
{
   const GLenum target = destImage->TexObject->Target;
   GLuint bw, bh;

   /* Check size */
   if (subWidth < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(width=%d)", func, subWidth);
      return GL_TRUE;
   }

   if (dims > 1 && subHeight < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(height=%d)", func, subHeight);
      return GL_TRUE;
   }

   if (dims > 2 && subDepth < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(depth=%d)", func, subDepth);
      return GL_TRUE;
   }

   /* check xoffset and width */
   if (xoffset < - (GLint) destImage->Border) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(xoffset)", func);
      return GL_TRUE;
   }

   if (xoffset + subWidth > (GLint) destImage->Width) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(xoffset+width)", func);
      return GL_TRUE;
   }

   /* check yoffset and height */
   if (dims > 1) {
      GLint yBorder = (target == GL_TEXTURE_1D_ARRAY) ? 0 : destImage->Border;
      if (yoffset < -yBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(yoffset)", func);
         return GL_TRUE;
      }
      if (yoffset + subHeight > (GLint) destImage->Height) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(yoffset+height)", func);
         return GL_TRUE;
      }
   }

   /* check zoffset and depth */
   if (dims > 2) {
      GLint depth;
      GLint zBorder = (target == GL_TEXTURE_2D_ARRAY ||
                       target == GL_TEXTURE_CUBE_MAP_ARRAY) ?
                         0 : destImage->Border;

      if (zoffset < -zBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(zoffset)", func);
         return GL_TRUE;
      }

      depth = (GLint) destImage->Depth;
      if (target == GL_TEXTURE_CUBE_MAP)
         depth = 6;
      if (zoffset + subDepth  > depth) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(zoffset+depth)", func);
         return GL_TRUE;
      }
   }

   /*
    * The OpenGL spec (and GL_ARB_texture_compression) says only whole
    * compressed texture images can be updated.  But, that restriction may be
    * relaxed for particular compressed formats.  At this time, all the
    * compressed formats supported by Mesa allow sub-textures to be updated
    * along compressed block boundaries.
    */
   _mesa_get_format_block_size(destImage->TexFormat, &bw, &bh);

   if (bw != 1 || bh != 1) {
      /* offset must be multiple of block size */
      if ((xoffset % bw != 0) || (yoffset % bh != 0)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(xoffset = %d, yoffset = %d)",
                     func, xoffset, yoffset);
         return GL_TRUE;
      }

      /* The size must be a multiple of bw x bh, or we must be using a
       * offset+size that exactly hits the edge of the image.  This
       * is important for small mipmap levels (1x1, 2x1, etc) and for
       * NPOT textures.
       */
      if ((subWidth % bw != 0) &&
          (xoffset + subWidth != (GLint) destImage->Width)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(width = %d)", func, subWidth);
         return GL_TRUE;
      }

      if ((subHeight % bh != 0) &&
          (yoffset + subHeight != (GLint) destImage->Height)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(height = %d)", func, subHeight);
         return GL_TRUE;
      }
   }

   return GL_FALSE;
}




/**
 * This is the fallback for Driver.TestProxyTexImage() for doing device-
 * specific texture image size checks.
 *
 * A hardware driver might override this function if, for example, the
 * max 3D texture size is 512x512x64 (i.e. not a cube).
 *
 * Note that width, height, depth == 0 is not an error.  However, a
 * texture with zero width/height/depth will be considered "incomplete"
 * and texturing will effectively be disabled.
 *
 * \param target  any texture target/type
 * \param level  as passed to glTexImage
 * \param format  the MESA_FORMAT_x for the tex image
 * \param width  as passed to glTexImage
 * \param height  as passed to glTexImage
 * \param depth  as passed to glTexImage
 * \param border  as passed to glTexImage
 * \return GL_TRUE if the image is acceptable, GL_FALSE if not acceptable.
 */
GLboolean
_mesa_test_proxy_teximage(struct gl_context *ctx, GLenum target, GLint level,
                          mesa_format format,
                          GLint width, GLint height, GLint depth, GLint border)
{
   /* We just check if the image size is less than MaxTextureMbytes.
    * Some drivers may do more specific checks.
    */
   uint64_t bytes = _mesa_format_image_size64(format, width, height, depth);
   uint64_t mbytes = bytes / (1024 * 1024); /* convert to MB */
   mbytes *= _mesa_num_tex_faces(target);
   return mbytes <= (uint64_t) ctx->Const.MaxTextureMbytes;
}


/**
 * Return true if the format is only valid for glCompressedTexImage.
 */
static GLboolean
compressedteximage_only_format(const struct gl_context *ctx, GLenum format)
{
   switch (format) {
   case GL_ETC1_RGB8_OES:
   case GL_PALETTE4_RGB8_OES:
   case GL_PALETTE4_RGBA8_OES:
   case GL_PALETTE4_R5_G6_B5_OES:
   case GL_PALETTE4_RGBA4_OES:
   case GL_PALETTE4_RGB5_A1_OES:
   case GL_PALETTE8_RGB8_OES:
   case GL_PALETTE8_RGBA8_OES:
   case GL_PALETTE8_R5_G6_B5_OES:
   case GL_PALETTE8_RGBA4_OES:
   case GL_PALETTE8_RGB5_A1_OES:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}


/**
 * Helper function to determine whether a target and specific compression
 * format are supported.
 */
GLboolean
_mesa_target_can_be_compressed(const struct gl_context *ctx, GLenum target,
                               GLenum intFormat)
{
   (void) intFormat;  /* not used yet */

   switch (target) {
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
      return GL_TRUE; /* true for any compressed format so far */
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      return ctx->Extensions.ARB_texture_cube_map;
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
   case GL_TEXTURE_2D_ARRAY_EXT:
      return ctx->Extensions.EXT_texture_array;
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      return ctx->Extensions.ARB_texture_cube_map_array;
   default:
      return GL_FALSE;
   }
}


/**
 * Check if the given texture target value is legal for a
 * glTexImage1/2/3D call.
 */
static GLboolean
legal_teximage_target(struct gl_context *ctx, GLuint dims, GLenum target)
{
   switch (dims) {
   case 1:
      switch (target) {
      case GL_TEXTURE_1D:
      case GL_PROXY_TEXTURE_1D:
         return _mesa_is_desktop_gl(ctx);
      default:
         return GL_FALSE;
      }
   case 2:
      switch (target) {
      case GL_TEXTURE_2D:
         return GL_TRUE;
      case GL_PROXY_TEXTURE_2D:
         return _mesa_is_desktop_gl(ctx);
      case GL_PROXY_TEXTURE_CUBE_MAP:
         return _mesa_is_desktop_gl(ctx)
            && ctx->Extensions.ARB_texture_cube_map;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         return ctx->Extensions.ARB_texture_cube_map;
      case GL_TEXTURE_RECTANGLE_NV:
      case GL_PROXY_TEXTURE_RECTANGLE_NV:
         return _mesa_is_desktop_gl(ctx)
            && ctx->Extensions.NV_texture_rectangle;
      case GL_TEXTURE_1D_ARRAY_EXT:
      case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
         return _mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array;
      default:
         return GL_FALSE;
      }
   case 3:
      switch (target) {
      case GL_TEXTURE_3D:
         return GL_TRUE;
      case GL_PROXY_TEXTURE_3D:
         return _mesa_is_desktop_gl(ctx);
      case GL_TEXTURE_2D_ARRAY_EXT:
         return (_mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array)
            || _mesa_is_gles3(ctx);
      case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
         return _mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array;
      case GL_TEXTURE_CUBE_MAP_ARRAY:
      case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
         return ctx->Extensions.ARB_texture_cube_map_array;
      default:
         return GL_FALSE;
      }
   default:
      _mesa_problem(ctx, "invalid dims=%u in legal_teximage_target()", dims);
      return GL_FALSE;
   }
}


/**
 * Check if the given texture target value is legal for a
 * glTexSubImage, glCopyTexSubImage or glCopyTexImage call.
 * The difference compared to legal_teximage_target() above is that
 * proxy targets are not supported.
 */
static GLboolean
legal_texsubimage_target(struct gl_context *ctx, GLuint dims, GLenum target,
                         bool dsa)
{
   switch (dims) {
   case 1:
      return _mesa_is_desktop_gl(ctx) && target == GL_TEXTURE_1D;
   case 2:
      switch (target) {
      case GL_TEXTURE_2D:
         return GL_TRUE;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         return ctx->Extensions.ARB_texture_cube_map;
      case GL_TEXTURE_RECTANGLE_NV:
         return _mesa_is_desktop_gl(ctx)
            && ctx->Extensions.NV_texture_rectangle;
      case GL_TEXTURE_1D_ARRAY_EXT:
         return _mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array;
      default:
         return GL_FALSE;
      }
   case 3:
      switch (target) {
      case GL_TEXTURE_3D:
         return GL_TRUE;
      case GL_TEXTURE_2D_ARRAY_EXT:
         return (_mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array)
            || _mesa_is_gles3(ctx);
      case GL_TEXTURE_CUBE_MAP_ARRAY:
      case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
         return ctx->Extensions.ARB_texture_cube_map_array;

      /* Table 8.15 of the OpenGL 4.5 core profile spec
       * (20141030) says that TEXTURE_CUBE_MAP is valid for TextureSubImage3D
       * and CopyTextureSubImage3D.
       */
      case GL_TEXTURE_CUBE_MAP:
         return dsa;
      default:
         return GL_FALSE;
      }
   default:
      _mesa_problem(ctx, "invalid dims=%u in legal_texsubimage_target()",
                    dims);
      return GL_FALSE;
   }
}


/**
 * Helper function to determine if a texture object is mutable (in terms
 * of GL_ARB_texture_storage).
 */
static GLboolean
mutable_tex_object(struct gl_context *ctx, GLenum target)
{
   struct gl_texture_object *texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return GL_FALSE;

   return !texObj->Immutable;
}


/**
 * Return expected size of a compressed texture.
 */
static GLuint
compressed_tex_size(GLsizei width, GLsizei height, GLsizei depth,
                    GLenum glformat)
{
   mesa_format mesaFormat = _mesa_glenum_to_compressed_format(glformat);
   return _mesa_format_image_size(mesaFormat, width, height, depth);
}

/**
 * Verify that a texture format is valid with a particular target
 *
 * In particular, textures with base format of \c GL_DEPTH_COMPONENT or
 * \c GL_DEPTH_STENCIL are only valid with certain, context dependent texture
 * targets.
 *
 * \param ctx             GL context
 * \param target          Texture target
 * \param internalFormat  Internal format of the texture image
 * \param dimensions      Dimensionality at the caller.  This is \b not used
 *                        in the validation.  It is only used when logging
 *                        error messages.
 * \param caller          Base name of the calling function (e.g.,
 *                        "glTexImage" or "glTexStorage").
 *
 * \returns true if the combination is legal, false otherwise.
 */
bool
_mesa_legal_texture_base_format_for_target(struct gl_context *ctx,
                                           GLenum target, GLenum internalFormat,
                                           unsigned dimensions,
                                           const char *caller)
{
   if (_mesa_base_tex_format(ctx, internalFormat) == GL_DEPTH_COMPONENT
       || _mesa_base_tex_format(ctx, internalFormat) == GL_DEPTH_STENCIL
       || _mesa_base_tex_format(ctx, internalFormat) == GL_STENCIL_INDEX) {
      /* Section 3.8.3 (Texture Image Specification) of the OpenGL 3.3 Core
       * Profile spec says:
       *
       *     "Textures with a base internal format of DEPTH_COMPONENT or
       *     DEPTH_STENCIL are supported by texture image specification
       *     commands only if target is TEXTURE_1D, TEXTURE_2D,
       *     TEXTURE_1D_ARRAY, TEXTURE_2D_ARRAY, TEXTURE_RECTANGLE,
       *     TEXTURE_CUBE_MAP, PROXY_TEXTURE_1D, PROXY_TEXTURE_2D,
       *     PROXY_TEXTURE_1D_ARRAY, PROXY_TEXTURE_2D_ARRAY,
       *     PROXY_TEXTURE_RECTANGLE, or PROXY_TEXTURE_CUBE_MAP. Using these
       *     formats in conjunction with any other target will result in an
       *     INVALID_OPERATION error."
       *
       * Cubemaps are only supported with desktop OpenGL version >= 3.0,
       * EXT_gpu_shader4, or, on OpenGL ES 2.0+, OES_depth_texture_cube_map.
       */
      if (target != GL_TEXTURE_1D &&
          target != GL_PROXY_TEXTURE_1D &&
          target != GL_TEXTURE_2D &&
          target != GL_PROXY_TEXTURE_2D &&
          target != GL_TEXTURE_1D_ARRAY &&
          target != GL_PROXY_TEXTURE_1D_ARRAY &&
          target != GL_TEXTURE_2D_ARRAY &&
          target != GL_PROXY_TEXTURE_2D_ARRAY &&
          target != GL_TEXTURE_RECTANGLE_ARB &&
          target != GL_PROXY_TEXTURE_RECTANGLE_ARB &&
         !((_mesa_is_cube_face(target) ||
            target == GL_TEXTURE_CUBE_MAP ||
            target == GL_PROXY_TEXTURE_CUBE_MAP) &&
           (ctx->Version >= 30 || ctx->Extensions.EXT_gpu_shader4
            || (ctx->API == API_OPENGLES2 && ctx->Extensions.OES_depth_texture_cube_map))) &&
          !((target == GL_TEXTURE_CUBE_MAP_ARRAY ||
             target == GL_PROXY_TEXTURE_CUBE_MAP_ARRAY) &&
            ctx->Extensions.ARB_texture_cube_map_array)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s%dD(bad target for depth texture)",
                     caller, dimensions);
         return false;
      }
   }

   return true;
}

static bool
texture_formats_agree(GLenum internalFormat,
                      GLenum format)
{
   GLboolean colorFormat;
   GLboolean is_format_depth_or_depthstencil;
   GLboolean is_internalFormat_depth_or_depthstencil;

   /* Even though there are no color-index textures, we still have to support
    * uploading color-index data and remapping it to RGB via the
    * GL_PIXEL_MAP_I_TO_[RGBA] tables.
    */
   const GLboolean indexFormat = (format == GL_COLOR_INDEX);

   is_internalFormat_depth_or_depthstencil =
      _mesa_is_depth_format(internalFormat) ||
      _mesa_is_depthstencil_format(internalFormat);

   is_format_depth_or_depthstencil =
      _mesa_is_depth_format(format) ||
      _mesa_is_depthstencil_format(format);

   colorFormat = _mesa_is_color_format(format);

   if (_mesa_is_color_format(internalFormat) && !colorFormat && !indexFormat)
      return false;

   if (is_internalFormat_depth_or_depthstencil !=
       is_format_depth_or_depthstencil)
      return false;

   if (_mesa_is_ycbcr_format(internalFormat) != _mesa_is_ycbcr_format(format))
      return false;

   return true;
}

/**
 * Test the glTexImage[123]D() parameters for errors.
 *
 * \param ctx GL context.
 * \param dimensions texture image dimensions (must be 1, 2 or 3).
 * \param target texture target given by the user (already validated).
 * \param level image level given by the user.
 * \param internalFormat internal format given by the user.
 * \param format pixel data format given by the user.
 * \param type pixel data type given by the user.
 * \param width image width given by the user.
 * \param height image height given by the user.
 * \param depth image depth given by the user.
 * \param border image border given by the user.
 *
 * \return GL_TRUE if a error is found, GL_FALSE otherwise
 *
 * Verifies each of the parameters against the constants specified in
 * __struct gl_contextRec::Const and the supported extensions, and according
 * to the OpenGL specification.
 * Note that we don't fully error-check the width, height, depth values
 * here.  That's done in _mesa_legal_texture_dimensions() which is used
 * by several other GL entrypoints.  Plus, texture dims have a special
 * interaction with proxy textures.
 */
static GLboolean
texture_error_check( struct gl_context *ctx,
                     GLuint dimensions, GLenum target,
                     GLint level, GLint internalFormat,
                     GLenum format, GLenum type,
                     GLint width, GLint height,
                     GLint depth, GLint border,
                     const GLvoid *pixels )
{
   GLenum err;

   /* Note: for proxy textures, some error conditions immediately generate
    * a GL error in the usual way.  But others do not generate a GL error.
    * Instead, they cause the width, height, depth, format fields of the
    * texture image to be zeroed-out.  The GL spec seems to indicate that the
    * zero-out behaviour is only used in cases related to memory allocation.
    */

   /* level check */
   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexImage%dD(level=%d)", dimensions, level);
      return GL_TRUE;
   }

   /* Check border */
   if (border < 0 || border > 1 ||
       ((ctx->API != API_OPENGL_COMPAT ||
         target == GL_TEXTURE_RECTANGLE_NV ||
         target == GL_PROXY_TEXTURE_RECTANGLE_NV) && border != 0)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexImage%dD(border=%d)", dimensions, border);
      return GL_TRUE;
   }

   if (width < 0 || height < 0 || depth < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexImage%dD(width, height or depth < 0)", dimensions);
      return GL_TRUE;
   }

   /* OpenGL ES 1.x and OpenGL ES 2.0 impose additional restrictions on the
    * combinations of format, internalFormat, and type that can be used.
    * Formats and types that require additional extensions (e.g., GL_FLOAT
    * requires GL_OES_texture_float) are filtered elsewhere.
    */

   if (_mesa_is_gles(ctx)) {
      if (_mesa_is_gles3(ctx)) {
         err = _mesa_es3_error_check_format_and_type(ctx, format, type,
                                                     internalFormat);
      } else {
         if (format != internalFormat) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glTexImage%dD(format = %s, internalFormat = %s)",
                        dimensions,
                        _mesa_lookup_enum_by_nr(format),
                        _mesa_lookup_enum_by_nr(internalFormat));
            return GL_TRUE;
         }

         err = _mesa_es_error_check_format_and_type(format, type, dimensions);
      }
      if (err != GL_NO_ERROR) {
         _mesa_error(ctx, err,
                     "glTexImage%dD(format = %s, type = %s, internalFormat = %s)",
                     dimensions,
                     _mesa_lookup_enum_by_nr(format),
                     _mesa_lookup_enum_by_nr(type),
                     _mesa_lookup_enum_by_nr(internalFormat));
         return GL_TRUE;
      }
   }

   /* Check internalFormat */
   if (_mesa_base_tex_format(ctx, internalFormat) < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexImage%dD(internalFormat=%s)",
                  dimensions, _mesa_lookup_enum_by_nr(internalFormat));
      return GL_TRUE;
   }

   /* Check incoming image format and type */
   err = _mesa_error_check_format_and_type(ctx, format, type);
   if (err != GL_NO_ERROR) {
      _mesa_error(ctx, err,
                  "glTexImage%dD(incompatible format = %s, type = %s)",
                  dimensions, _mesa_lookup_enum_by_nr(format),
                  _mesa_lookup_enum_by_nr(type));
      return GL_TRUE;
   }

   /* validate the bound PBO, if any */
   if (!_mesa_validate_pbo_source(ctx, dimensions, &ctx->Unpack,
                                  width, height, depth, format, type,
                                  INT_MAX, pixels, "glTexImage")) {
      return GL_TRUE;
   }

   /* make sure internal format and format basically agree */
   if (!texture_formats_agree(internalFormat, format)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTexImage%dD(incompatible internalFormat = %s, format = %s)",
                  dimensions, _mesa_lookup_enum_by_nr(internalFormat),
                  _mesa_lookup_enum_by_nr(format));
      return GL_TRUE;
   }

   /* additional checks for ycbcr textures */
   if (internalFormat == GL_YCBCR_MESA) {
      assert(ctx->Extensions.MESA_ycbcr_texture);
      if (type != GL_UNSIGNED_SHORT_8_8_MESA &&
          type != GL_UNSIGNED_SHORT_8_8_REV_MESA) {
         char message[100];
         _mesa_snprintf(message, sizeof(message),
                        "glTexImage%dD(format/type YCBCR mismatch)",
                        dimensions);
         _mesa_error(ctx, GL_INVALID_ENUM, "%s", message);
         return GL_TRUE; /* error */
      }
      if (target != GL_TEXTURE_2D &&
          target != GL_PROXY_TEXTURE_2D &&
          target != GL_TEXTURE_RECTANGLE_NV &&
          target != GL_PROXY_TEXTURE_RECTANGLE_NV) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glTexImage%dD(bad target for YCbCr texture)",
                     dimensions);
         return GL_TRUE;
      }
      if (border != 0) {
         char message[100];
         _mesa_snprintf(message, sizeof(message),
                        "glTexImage%dD(format=GL_YCBCR_MESA and border=%d)",
                        dimensions, border);
         _mesa_error(ctx, GL_INVALID_VALUE, "%s", message);
         return GL_TRUE;
      }
   }

   /* additional checks for depth textures */
   if (!_mesa_legal_texture_base_format_for_target(ctx, target, internalFormat,
                                                   dimensions, "glTexImage"))
      return GL_TRUE;

   /* additional checks for compressed textures */
   if (_mesa_is_compressed_format(ctx, internalFormat)) {
      if (!_mesa_target_can_be_compressed(ctx, target, internalFormat)) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glTexImage%dD(target can't be compressed)", dimensions);
         return GL_TRUE;
      }
      if (compressedteximage_only_format(ctx, internalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glTexImage%dD(no compression for format)", dimensions);
         return GL_TRUE;
      }
      if (border != 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glTexImage%dD(border!=0)", dimensions);
         return GL_TRUE;
      }
   }

   /* additional checks for integer textures */
   if ((ctx->Version >= 30 || ctx->Extensions.EXT_texture_integer) &&
       (_mesa_is_enum_format_integer(format) !=
        _mesa_is_enum_format_integer(internalFormat))) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTexImage%dD(integer/non-integer format mismatch)",
                  dimensions);
      return GL_TRUE;
   }

   if (!mutable_tex_object(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTexImage%dD(immutable texture)", dimensions);
      return GL_TRUE;
   }

   /* if we get here, the parameters are OK */
   return GL_FALSE;
}


/**
 * Error checking for glCompressedTexImage[123]D().
 * Note that the width, height and depth values are not fully error checked
 * here.
 * \return GL_TRUE if a error is found, GL_FALSE otherwise
 */
static GLenum
compressed_texture_error_check(struct gl_context *ctx, GLint dimensions,
                               GLenum target, GLint level,
                               GLenum internalFormat, GLsizei width,
                               GLsizei height, GLsizei depth, GLint border,
                               GLsizei imageSize, const GLvoid *data)
{
   const GLint maxLevels = _mesa_max_texture_levels(ctx, target);
   GLint expectedSize;
   GLenum error = GL_NO_ERROR;
   char *reason = ""; /* no error */

   if (!_mesa_target_can_be_compressed(ctx, target, internalFormat)) {
      reason = "target";
      /* From section 3.8.6, page 146 of OpenGL ES 3.0 spec:
       *
       *    "The ETC2/EAC texture compression algorithm supports only
       *     two-dimensional images. If internalformat is an ETC2/EAC format,
       *     CompressedTexImage3D will generate an INVALID_OPERATION error if
       *     target is not TEXTURE_2D_ARRAY."
       */
      error = _mesa_is_desktop_gl(ctx) ? GL_INVALID_ENUM : GL_INVALID_OPERATION;
      goto error;
   }

   /* This will detect any invalid internalFormat value */
   if (!_mesa_is_compressed_format(ctx, internalFormat)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glCompressedTexImage%dD(internalFormat=%s)",
                  dimensions, _mesa_lookup_enum_by_nr(internalFormat));
      return GL_TRUE;
   }

   /* validate the bound PBO, if any */
   if (!_mesa_validate_pbo_source_compressed(ctx, dimensions, &ctx->Unpack,
                                             imageSize, data,
                                             "glCompressedTexImage")) {
      return GL_TRUE;
   }

   switch (internalFormat) {
   case GL_PALETTE4_RGB8_OES:
   case GL_PALETTE4_RGBA8_OES:
   case GL_PALETTE4_R5_G6_B5_OES:
   case GL_PALETTE4_RGBA4_OES:
   case GL_PALETTE4_RGB5_A1_OES:
   case GL_PALETTE8_RGB8_OES:
   case GL_PALETTE8_RGBA8_OES:
   case GL_PALETTE8_R5_G6_B5_OES:
   case GL_PALETTE8_RGBA4_OES:
   case GL_PALETTE8_RGB5_A1_OES:
      /* check level (note that level should be zero or less!) */
      if (level > 0 || level < -maxLevels) {
         reason = "level";
         error = GL_INVALID_VALUE;
         goto error;
      }

      if (dimensions != 2) {
         reason = "compressed paletted textures must be 2D";
         error = GL_INVALID_OPERATION;
         goto error;
      }

      /* Figure out the expected texture size (in bytes).  This will be
       * checked against the actual size later.
       */
      expectedSize = _mesa_cpal_compressed_size(level, internalFormat,
                                                width, height);

      /* This is for the benefit of the TestProxyTexImage below.  It expects
       * level to be non-negative.  OES_compressed_paletted_texture uses a
       * weird mechanism where the level specified to glCompressedTexImage2D
       * is -(n-1) number of levels in the texture, and the data specifies the
       * complete mipmap stack.  This is done to ensure the palette is the
       * same for all levels.
       */
      level = -level;
      break;

   default:
      /* check level */
      if (level < 0 || level >= maxLevels) {
         reason = "level";
         error = GL_INVALID_VALUE;
         goto error;
      }

      /* Figure out the expected texture size (in bytes).  This will be
       * checked against the actual size later.
       */
      expectedSize = compressed_tex_size(width, height, depth, internalFormat);
      break;
   }

   /* This should really never fail */
   if (_mesa_base_tex_format(ctx, internalFormat) < 0) {
      reason = "internalFormat";
      error = GL_INVALID_ENUM;
      goto error;
   }

   /* No compressed formats support borders at this time */
   if (border != 0) {
      reason = "border != 0";
      error = GL_INVALID_VALUE;
      goto error;
   }

   /* Check for invalid pixel storage modes */
   if (!_mesa_compressed_pixel_storage_error_check(ctx, dimensions,
                                                   &ctx->Unpack,
                                                   "glCompressedTexImage")) {
      return GL_FALSE;
   }

   /* check image size in bytes */
   if (expectedSize != imageSize) {
      /* Per GL_ARB_texture_compression:  GL_INVALID_VALUE is generated [...]
       * if <imageSize> is not consistent with the format, dimensions, and
       * contents of the specified image.
       */
      reason = "imageSize inconsistant with width/height/format";
      error = GL_INVALID_VALUE;
      goto error;
   }

   if (!mutable_tex_object(ctx, target)) {
      reason = "immutable texture";
      error = GL_INVALID_OPERATION;
      goto error;
   }

   return GL_FALSE;

error:
   /* Note: not all error paths exit through here. */
   _mesa_error(ctx, error, "glCompressedTexImage%dD(%s)",
               dimensions, reason);
   return GL_TRUE;
}



/**
 * Test glTexSubImage[123]D() parameters for errors.
 *
 * \param ctx GL context.
 * \param dimensions texture image dimensions (must be 1, 2 or 3).
 * \param target texture target given by the user (already validated)
 * \param level image level given by the user.
 * \param xoffset sub-image x offset given by the user.
 * \param yoffset sub-image y offset given by the user.
 * \param zoffset sub-image z offset given by the user.
 * \param format pixel data format given by the user.
 * \param type pixel data type given by the user.
 * \param width image width given by the user.
 * \param height image height given by the user.
 * \param depth image depth given by the user.
 *
 * \return GL_TRUE if an error was detected, or GL_FALSE if no errors.
 *
 * Verifies each of the parameters against the constants specified in
 * __struct gl_contextRec::Const and the supported extensions, and according
 * to the OpenGL specification.
 */
static GLboolean
texsubimage_error_check(struct gl_context *ctx, GLuint dimensions,
                        struct gl_texture_object *texObj,
                        GLenum target, GLint level,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLint width, GLint height, GLint depth,
                        GLenum format, GLenum type, const GLvoid *pixels,
                        bool dsa, const char *callerName)
{
   struct gl_texture_image *texImage;
   GLenum err;

   if (!texObj) {
      /* must be out of memory */
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s()", callerName);
      return GL_TRUE;
   }

   /* check target (proxies not allowed) */
   if (!legal_texsubimage_target(ctx, dimensions, target, dsa)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target=%s)",
                  callerName, _mesa_lookup_enum_by_nr(target));
      return GL_TRUE;
   }

   /* level check */
   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(level=%d)", callerName, level);
      return GL_TRUE;
   }

   /* OpenGL ES 1.x and OpenGL ES 2.0 impose additional restrictions on the
    * combinations of format and type that can be used.  Formats and types
    * that require additional extensions (e.g., GL_FLOAT requires
    * GL_OES_texture_float) are filtered elsewhere.
    */
   if (_mesa_is_gles(ctx) && !_mesa_is_gles3(ctx)) {
      err = _mesa_es_error_check_format_and_type(format, type, dimensions);
      if (err != GL_NO_ERROR) {
         _mesa_error(ctx, err, "%s(format = %s, type = %s)",
                     callerName, _mesa_lookup_enum_by_nr(format),
                     _mesa_lookup_enum_by_nr(type));
         return GL_TRUE;
      }
   }

   err = _mesa_error_check_format_and_type(ctx, format, type);
   if (err != GL_NO_ERROR) {
      _mesa_error(ctx, err,
                  "%s(incompatible format = %s, type = %s)",
                  callerName, _mesa_lookup_enum_by_nr(format),
                  _mesa_lookup_enum_by_nr(type));
      return GL_TRUE;
   }

   /* validate the bound PBO, if any */
   if (!_mesa_validate_pbo_source(ctx, dimensions, &ctx->Unpack,
                                  width, height, depth, format, type,
                                  INT_MAX, pixels, callerName)) {
      return GL_TRUE;
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   if (!texImage) {
      /* non-existant texture level */
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid texture image)",
                  callerName);
      return GL_TRUE;
   }

   if (error_check_subtexture_dimensions(ctx, dimensions,
                                         texImage, xoffset, yoffset, zoffset,
                                         width, height, depth, callerName)) {
      return GL_TRUE;
   }

   if (_mesa_is_format_compressed(texImage->TexFormat)) {
      if (compressedteximage_only_format(ctx, texImage->InternalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
               "%s(no compression for format)", callerName);
         return GL_TRUE;
      }
   }

   if (ctx->Version >= 30 || ctx->Extensions.EXT_texture_integer) {
      /* both source and dest must be integer-valued, or neither */
      if (_mesa_is_format_integer_color(texImage->TexFormat) !=
          _mesa_is_enum_format_integer(format)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(integer/non-integer format mismatch)", callerName);
         return GL_TRUE;
      }
   }

   return GL_FALSE;
}


/**
 * Test glCopyTexImage[12]D() parameters for errors.
 *
 * \param ctx GL context.
 * \param dimensions texture image dimensions (must be 1, 2 or 3).
 * \param target texture target given by the user.
 * \param level image level given by the user.
 * \param internalFormat internal format given by the user.
 * \param width image width given by the user.
 * \param height image height given by the user.
 * \param border texture border.
 *
 * \return GL_TRUE if an error was detected, or GL_FALSE if no errors.
 *
 * Verifies each of the parameters against the constants specified in
 * __struct gl_contextRec::Const and the supported extensions, and according
 * to the OpenGL specification.
 */
static GLboolean
copytexture_error_check( struct gl_context *ctx, GLuint dimensions,
                         GLenum target, GLint level, GLint internalFormat,
                         GLint width, GLint height, GLint border )
{
   GLint baseFormat;
   GLint rb_base_format;
   struct gl_renderbuffer *rb;
   GLenum rb_internal_format;

   /* check target */
   if (!legal_texsubimage_target(ctx, dimensions, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glCopyTexImage%uD(target=%s)",
                  dimensions, _mesa_lookup_enum_by_nr(target));
      return GL_TRUE;
   }

   /* level check */
   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyTexImage%dD(level=%d)", dimensions, level);
      return GL_TRUE;
   }

   /* Check that the source buffer is complete */
   if (_mesa_is_user_fbo(ctx->ReadBuffer)) {
      if (ctx->ReadBuffer->_Status == 0) {
         _mesa_test_framebuffer_completeness(ctx, ctx->ReadBuffer);
      }
      if (ctx->ReadBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
         _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                     "glCopyTexImage%dD(invalid readbuffer)", dimensions);
         return GL_TRUE;
      }

      if (ctx->ReadBuffer->Visual.samples > 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(multisample FBO)", dimensions);
         return GL_TRUE;
      }
   }

   /* Check border */
   if (border < 0 || border > 1 ||
       ((ctx->API != API_OPENGL_COMPAT ||
         target == GL_TEXTURE_RECTANGLE_NV ||
         target == GL_PROXY_TEXTURE_RECTANGLE_NV) && border != 0)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyTexImage%dD(border=%d)", dimensions, border);
      return GL_TRUE;
   }

   rb = _mesa_get_read_renderbuffer_for_format(ctx, internalFormat);
   if (rb == NULL) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyTexImage%dD(read buffer)", dimensions);
      return GL_TRUE;
   }

   /* OpenGL ES 1.x and OpenGL ES 2.0 impose additional restrictions on the
    * internalFormat.
    */
   if (_mesa_is_gles(ctx) && !_mesa_is_gles3(ctx)) {
      switch (internalFormat) {
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
         break;
      default:
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                     _mesa_lookup_enum_by_nr(internalFormat));
         return GL_TRUE;
      }
   }

   baseFormat = _mesa_base_tex_format(ctx, internalFormat);
   if (baseFormat < 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                  _mesa_lookup_enum_by_nr(internalFormat));
      return GL_TRUE;
   }

   rb_internal_format = rb->InternalFormat;
   rb_base_format = _mesa_base_tex_format(ctx, rb->InternalFormat);
   if (_mesa_is_color_format(internalFormat)) {
      if (rb_base_format < 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                     _mesa_lookup_enum_by_nr(internalFormat));
         return GL_TRUE;
      }
   }

   if (_mesa_is_gles(ctx)) {
      bool valid = true;
      if (_mesa_base_format_component_count(baseFormat) >
          _mesa_base_format_component_count(rb_base_format)) {
         valid = false;
      }
      if (baseFormat == GL_DEPTH_COMPONENT ||
          baseFormat == GL_DEPTH_STENCIL ||
          rb_base_format == GL_DEPTH_COMPONENT ||
          rb_base_format == GL_DEPTH_STENCIL ||
          ((baseFormat == GL_LUMINANCE_ALPHA ||
            baseFormat == GL_ALPHA) &&
           rb_base_format != GL_RGBA) ||
          internalFormat == GL_RGB9_E5) {
         valid = false;
      }
      if (internalFormat == GL_RGB9_E5) {
         valid = false;
      }
      if (!valid) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                     _mesa_lookup_enum_by_nr(internalFormat));
         return GL_TRUE;
      }
   }

   if (_mesa_is_gles3(ctx)) {
      bool rb_is_srgb = false;
      bool dst_is_srgb = false;

      if (ctx->Extensions.EXT_framebuffer_sRGB &&
          _mesa_get_format_color_encoding(rb->Format) == GL_SRGB) {
         rb_is_srgb = true;
      }

      if (_mesa_get_linear_internalformat(internalFormat) != internalFormat) {
         dst_is_srgb = true;
      }

      if (rb_is_srgb != dst_is_srgb) {
         /* Page 137 (page 149 of the PDF) in section 3.8.5 of the
          * OpenGLES 3.0.0 spec says:
          *
          *     "The error INVALID_OPERATION is also generated if the
          *     value of FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the
          *     framebuffer attachment corresponding to the read buffer
          *     is LINEAR (see section 6.1.13) and internalformat is
          *     one of the sRGB formats described in section 3.8.16, or
          *     if the value of FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING is
          *     SRGB and internalformat is not one of the sRGB formats."
          */
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(srgb usage mismatch)", dimensions);
         return GL_TRUE;
      }

      /* Page 139, Table 3.15 of OpenGL ES 3.0 spec does not define ReadPixels
       * types for SNORM formats. Also, conversion to SNORM formats is not
       * allowed by Table 3.2 on Page 110.
       */
      if(_mesa_is_enum_format_snorm(internalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                     _mesa_lookup_enum_by_nr(internalFormat));
         return GL_TRUE;
      }
   }

   if (!_mesa_source_buffer_exists(ctx, baseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyTexImage%dD(missing readbuffer)", dimensions);
      return GL_TRUE;
   }

   /* From the EXT_texture_integer spec:
    *
    *     "INVALID_OPERATION is generated by CopyTexImage* and CopyTexSubImage*
    *      if the texture internalformat is an integer format and the read color
    *      buffer is not an integer format, or if the internalformat is not an
    *      integer format and the read color buffer is an integer format."
    */
   if (_mesa_is_color_format(internalFormat)) {
      bool is_int = _mesa_is_enum_format_integer(internalFormat);
      bool is_rbint = _mesa_is_enum_format_integer(rb_internal_format);
      bool is_unorm = _mesa_is_enum_format_unorm(internalFormat);
      bool is_rbunorm = _mesa_is_enum_format_unorm(rb_internal_format);
      if (is_int || is_rbint) {
         if (is_int != is_rbint) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glCopyTexImage%dD(integer vs non-integer)", dimensions);
            return GL_TRUE;
         } else if (_mesa_is_gles(ctx) &&
                    _mesa_is_enum_format_unsigned_int(internalFormat) !=
                      _mesa_is_enum_format_unsigned_int(rb_internal_format)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glCopyTexImage%dD(signed vs unsigned integer)", dimensions);
            return GL_TRUE;
         }
      }

      /* From page 138 of OpenGL ES 3.0 spec:
       *    "The error INVALID_OPERATION is generated if floating-point RGBA
       *    data is required; if signed integer RGBA data is required and the
       *    format of the current color buffer is not signed integer; if
       *    unsigned integer RGBA data is required and the format of the
       *    current color buffer is not unsigned integer; or if fixed-point
       *    RGBA data is required and the format of the current color buffer
       *    is not fixed-point.
       */
      if (_mesa_is_gles(ctx) && is_unorm != is_rbunorm)
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glCopyTexImage%dD(unorm vs non-unorm)", dimensions);
   }

   if (_mesa_is_compressed_format(ctx, internalFormat)) {
      if (!_mesa_target_can_be_compressed(ctx, target, internalFormat)) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glCopyTexImage%dD(target)", dimensions);
         return GL_TRUE;
      }
      if (compressedteximage_only_format(ctx, internalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
               "glCopyTexImage%dD(no compression for format)", dimensions);
         return GL_TRUE;
      }
      if (border != 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(border!=0)", dimensions);
         return GL_TRUE;
      }
   }

   if (!mutable_tex_object(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyTexImage%dD(immutable texture)", dimensions);
      return GL_TRUE;
   }

   /* if we get here, the parameters are OK */
   return GL_FALSE;
}


/**
 * Test glCopyTexSubImage[12]D() parameters for errors.
 * \return GL_TRUE if an error was detected, or GL_FALSE if no errors.
 */
static GLboolean
copytexsubimage_error_check(struct gl_context *ctx, GLuint dimensions,
                            const struct gl_texture_object *texObj,
                            GLenum target, GLint level,
                            GLint xoffset, GLint yoffset, GLint zoffset,
                            GLint width, GLint height, const char *caller)
{
   struct gl_texture_image *texImage;

   /* Check that the source buffer is complete */
   if (_mesa_is_user_fbo(ctx->ReadBuffer)) {
      if (ctx->ReadBuffer->_Status == 0) {
         _mesa_test_framebuffer_completeness(ctx, ctx->ReadBuffer);
      }
      if (ctx->ReadBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
         _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                     "%s(invalid readbuffer)", caller);
         return GL_TRUE;
      }

      if (ctx->ReadBuffer->Visual.samples > 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                "%s(multisample FBO)", caller);
         return GL_TRUE;
      }
   }

   /* Check level */
   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(level=%d)", caller, level);
      return GL_TRUE;
   }

   /* Get dest image pointers */
   if (!texObj) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s()", caller);
      return GL_TRUE;
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   if (!texImage) {
      /* destination image does not exist */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(invalid texture image)", caller);
      return GL_TRUE;
   }

   if (error_check_subtexture_dimensions(ctx, dimensions, texImage,
                                         xoffset, yoffset, zoffset,
                                         width, height, 1, caller)) {
      return GL_TRUE;
   }

   if (_mesa_is_format_compressed(texImage->TexFormat)) {
      if (compressedteximage_only_format(ctx, texImage->InternalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
               "%s(no compression for format)", caller);
         return GL_TRUE;
      }
   }

   if (texImage->InternalFormat == GL_YCBCR_MESA) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s()", caller);
      return GL_TRUE;
   }

   if (!_mesa_source_buffer_exists(ctx, texImage->_BaseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(missing readbuffer, format=0x%x)", caller,
                  texImage->_BaseFormat);
      return GL_TRUE;
   }

   /* From the EXT_texture_integer spec:
    *
    *     "INVALID_OPERATION is generated by CopyTexImage* and
    *     CopyTexSubImage* if the texture internalformat is an integer format
    *     and the read color buffer is not an integer format, or if the
    *     internalformat is not an integer format and the read color buffer
    *     is an integer format."
    */
   if (_mesa_is_color_format(texImage->InternalFormat)) {
      struct gl_renderbuffer *rb = ctx->ReadBuffer->_ColorReadBuffer;

      if (_mesa_is_format_integer_color(rb->Format) !=
          _mesa_is_format_integer_color(texImage->TexFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(integer vs non-integer)", caller);
         return GL_TRUE;
      }
   }

   /* if we get here, the parameters are OK */
   return GL_FALSE;
}


/** Callback info for walking over FBO hash table */
struct cb_info
{
   struct gl_context *ctx;
   struct gl_texture_object *texObj;
   GLuint level, face;
};


/**
 * Check render to texture callback.  Called from _mesa_HashWalk().
 */
static void
check_rtt_cb(GLuint key, void *data, void *userData)
{
   struct gl_framebuffer *fb = (struct gl_framebuffer *) data;
   const struct cb_info *info = (struct cb_info *) userData;
   struct gl_context *ctx = info->ctx;
   const struct gl_texture_object *texObj = info->texObj;
   const GLuint level = info->level, face = info->face;

   /* If this is a user-created FBO */
   if (_mesa_is_user_fbo(fb)) {
      GLuint i;
      /* check if any of the FBO's attachments point to 'texObj' */
      for (i = 0; i < BUFFER_COUNT; i++) {
         struct gl_renderbuffer_attachment *att = fb->Attachment + i;
         if (att->Type == GL_TEXTURE &&
             att->Texture == texObj &&
             att->TextureLevel == level &&
             att->CubeMapFace == face) {
            _mesa_update_texture_renderbuffer(ctx, ctx->DrawBuffer, att);
            assert(att->Renderbuffer->TexImage);
            /* Mark fb status as indeterminate to force re-validation */
            fb->_Status = 0;
         }
      }
   }
}


/**
 * When a texture image is specified we have to check if it's bound to
 * any framebuffer objects (render to texture) in order to detect changes
 * in size or format since that effects FBO completeness.
 * Any FBOs rendering into the texture must be re-validated.
 */
void
_mesa_update_fbo_texture(struct gl_context *ctx,
                         struct gl_texture_object *texObj,
                         GLuint face, GLuint level)
{
   /* Only check this texture if it's been marked as RenderToTexture */
   if (texObj->_RenderToTexture) {
      struct cb_info info;
      info.ctx = ctx;
      info.texObj = texObj;
      info.level = level;
      info.face = face;
      _mesa_HashWalk(ctx->Shared->FrameBuffers, check_rtt_cb, &info);
   }
}


/**
 * If the texture object's GenerateMipmap flag is set and we've
 * changed the texture base level image, regenerate the rest of the
 * mipmap levels now.
 */
static inline void
check_gen_mipmap(struct gl_context *ctx, GLenum target,
                 struct gl_texture_object *texObj, GLint level)
{
   if (texObj->GenerateMipmap &&
       level == texObj->BaseLevel &&
       level < texObj->MaxLevel) {
      assert(ctx->Driver.GenerateMipmap);
      ctx->Driver.GenerateMipmap(ctx, target, texObj);
   }
}


/** Debug helper: override the user-requested internal format */
static GLenum
override_internal_format(GLenum internalFormat, GLint width, GLint height)
{
#if 0
   if (internalFormat == GL_RGBA16F_ARB ||
       internalFormat == GL_RGBA32F_ARB) {
      printf("Convert rgba float tex to int %d x %d\n", width, height);
      return GL_RGBA;
   }
   else if (internalFormat == GL_RGB16F_ARB ||
            internalFormat == GL_RGB32F_ARB) {
      printf("Convert rgb float tex to int %d x %d\n", width, height);
      return GL_RGB;
   }
   else if (internalFormat == GL_LUMINANCE_ALPHA16F_ARB ||
            internalFormat == GL_LUMINANCE_ALPHA32F_ARB) {
      printf("Convert luminance float tex to int %d x %d\n", width, height);
      return GL_LUMINANCE_ALPHA;
   }
   else if (internalFormat == GL_LUMINANCE16F_ARB ||
            internalFormat == GL_LUMINANCE32F_ARB) {
      printf("Convert luminance float tex to int %d x %d\n", width, height);
      return GL_LUMINANCE;
   }
   else if (internalFormat == GL_ALPHA16F_ARB ||
            internalFormat == GL_ALPHA32F_ARB) {
      printf("Convert luminance float tex to int %d x %d\n", width, height);
      return GL_ALPHA;
   }
   /*
   else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) {
      internalFormat = GL_RGBA;
   }
   */
   else {
      return internalFormat;
   }
#else
   return internalFormat;
#endif
}


/**
 * Choose the actual hardware format for a texture image.
 * Try to use the same format as the previous image level when possible.
 * Otherwise, ask the driver for the best format.
 * It's important to try to choose a consistant format for all levels
 * for efficient texture memory layout/allocation.  In particular, this
 * comes up during automatic mipmap generation.
 */
mesa_format
_mesa_choose_texture_format(struct gl_context *ctx,
                            struct gl_texture_object *texObj,
                            GLenum target, GLint level,
                            GLenum internalFormat, GLenum format, GLenum type)
{
   mesa_format f;

   /* see if we've already chosen a format for the previous level */
   if (level > 0) {
      struct gl_texture_image *prevImage =
	 _mesa_select_tex_image(texObj, target, level - 1);
      /* See if the prev level is defined and has an internal format which
       * matches the new internal format.
       */
      if (prevImage &&
          prevImage->Width > 0 &&
          prevImage->InternalFormat == internalFormat) {
         /* use the same format */
         assert(prevImage->TexFormat != MESA_FORMAT_NONE);
         return prevImage->TexFormat;
      }
   }

   /* If the application requested compression to an S3TC format but we don't
    * have the DXTn library, force a generic compressed format instead.
    */
   if (internalFormat != format && format != GL_NONE) {
      const GLenum before = internalFormat;

      switch (internalFormat) {
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
         if (!ctx->Mesa_DXTn)
            internalFormat = GL_COMPRESSED_RGB;
         break;
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
         if (!ctx->Mesa_DXTn)
            internalFormat = GL_COMPRESSED_RGBA;
         break;
      default:
         break;
      }

      if (before != internalFormat) {
         _mesa_warning(ctx,
                       "DXT compression requested (%s), "
                       "but libtxc_dxtn library not installed.  Using %s "
                       "instead.",
                       _mesa_lookup_enum_by_nr(before),
                       _mesa_lookup_enum_by_nr(internalFormat));
      }
   }

   /* choose format from scratch */
   f = ctx->Driver.ChooseTextureFormat(ctx, target, internalFormat,
                                       format, type);
   assert(f != MESA_FORMAT_NONE);
   return f;
}


/**
 * Adjust pixel unpack params and image dimensions to strip off the
 * one-pixel texture border.
 *
 * Gallium and intel don't support texture borders.  They've seldem been used
 * and seldom been implemented correctly anyway.
 *
 * \param unpackNew returns the new pixel unpack parameters
 */
static void
strip_texture_border(GLenum target,
                     GLint *width, GLint *height, GLint *depth,
                     const struct gl_pixelstore_attrib *unpack,
                     struct gl_pixelstore_attrib *unpackNew)
{
   assert(width);
   assert(height);
   assert(depth);

   *unpackNew = *unpack;

   if (unpackNew->RowLength == 0)
      unpackNew->RowLength = *width;

   if (unpackNew->ImageHeight == 0)
      unpackNew->ImageHeight = *height;

   assert(*width >= 3);
   unpackNew->SkipPixels++;  /* skip the border */
   *width = *width - 2;      /* reduce the width by two border pixels */

   /* The min height of a texture with a border is 3 */
   if (*height >= 3 && target != GL_TEXTURE_1D_ARRAY) {
      unpackNew->SkipRows++;  /* skip the border */
      *height = *height - 2;  /* reduce the height by two border pixels */
   }

   if (*depth >= 3 &&
       target != GL_TEXTURE_2D_ARRAY &&
       target != GL_TEXTURE_CUBE_MAP_ARRAY) {
      unpackNew->SkipImages++;  /* skip the border */
      *depth = *depth - 2;      /* reduce the depth by two border pixels */
   }
}


/**
 * Common code to implement all the glTexImage1D/2D/3D functions
 * as well as glCompressedTexImage1D/2D/3D.
 * \param compressed  only GL_TRUE for glCompressedTexImage1D/2D/3D calls.
 * \param format  the user's image format (only used if !compressed)
 * \param type  the user's image type (only used if !compressed)
 * \param imageSize  only used for glCompressedTexImage1D/2D/3D calls.
 */
static void
teximage(struct gl_context *ctx, GLboolean compressed, GLuint dims,
         GLenum target, GLint level, GLint internalFormat,
         GLsizei width, GLsizei height, GLsizei depth,
         GLint border, GLenum format, GLenum type,
         GLsizei imageSize, const GLvoid *pixels)
{
   const char *func = compressed ? "glCompressedTexImage" : "glTexImage";
   struct gl_pixelstore_attrib unpack_no_border;
   const struct gl_pixelstore_attrib *unpack = &ctx->Unpack;
   struct gl_texture_object *texObj;
   mesa_format texFormat;
   GLboolean dimensionsOK, sizeOK;

   FLUSH_VERTICES(ctx, 0);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE)) {
      if (compressed)
         _mesa_debug(ctx,
                     "glCompressedTexImage%uD %s %d %s %d %d %d %d %p\n",
                     dims,
                     _mesa_lookup_enum_by_nr(target), level,
                     _mesa_lookup_enum_by_nr(internalFormat),
                     width, height, depth, border, pixels);
      else
         _mesa_debug(ctx,
                     "glTexImage%uD %s %d %s %d %d %d %d %s %s %p\n",
                     dims,
                     _mesa_lookup_enum_by_nr(target), level,
                     _mesa_lookup_enum_by_nr(internalFormat),
                     width, height, depth, border,
                     _mesa_lookup_enum_by_nr(format),
                     _mesa_lookup_enum_by_nr(type), pixels);
   }

   internalFormat = override_internal_format(internalFormat, width, height);

   /* target error checking */
   if (!legal_teximage_target(ctx, dims, target)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s%uD(target=%s)",
                  func, dims, _mesa_lookup_enum_by_nr(target));
      return;
   }

   /* general error checking */
   if (compressed) {
      if (compressed_texture_error_check(ctx, dims, target, level,
                                         internalFormat,
                                         width, height, depth,
                                         border, imageSize, pixels))
         return;
   }
   else {
      if (texture_error_check(ctx, dims, target, level, internalFormat,
                              format, type, width, height, depth, border,
                              pixels))
         return;
   }

   /* Here we convert a cpal compressed image into a regular glTexImage2D
    * call by decompressing the texture.  If we really want to support cpal
    * textures in any driver this would have to be changed.
    */
   if (ctx->API == API_OPENGLES && compressed && dims == 2) {
      switch (internalFormat) {
      case GL_PALETTE4_RGB8_OES:
      case GL_PALETTE4_RGBA8_OES:
      case GL_PALETTE4_R5_G6_B5_OES:
      case GL_PALETTE4_RGBA4_OES:
      case GL_PALETTE4_RGB5_A1_OES:
      case GL_PALETTE8_RGB8_OES:
      case GL_PALETTE8_RGBA8_OES:
      case GL_PALETTE8_R5_G6_B5_OES:
      case GL_PALETTE8_RGBA4_OES:
      case GL_PALETTE8_RGB5_A1_OES:
         _mesa_cpal_compressed_teximage2d(target, level, internalFormat,
                                          width, height, imageSize, pixels);
         return;
      }
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   assert(texObj);

   if (compressed) {
      /* For glCompressedTexImage() the driver has no choice about the
       * texture format since we'll never transcode the user's compressed
       * image data.  The internalFormat was error checked earlier.
       */
      texFormat = _mesa_glenum_to_compressed_format(internalFormat);
   }
   else {
      /* In case of HALF_FLOAT_OES or FLOAT_OES, find corresponding sized
       * internal floating point format for the given base format.
       */
      if (_mesa_is_gles(ctx) && format == internalFormat) {
         if (type == GL_FLOAT) {
            texObj->_IsFloat = GL_TRUE;
         } else if (type == GL_HALF_FLOAT_OES || type == GL_HALF_FLOAT) {
            texObj->_IsHalfFloat = GL_TRUE;
         }

         internalFormat = adjust_for_oes_float_texture(format, type);
      }

      texFormat = _mesa_choose_texture_format(ctx, texObj, target, level,
                                              internalFormat, format, type);
   }

   assert(texFormat != MESA_FORMAT_NONE);

   /* check that width, height, depth are legal for the mipmap level */
   dimensionsOK = _mesa_legal_texture_dimensions(ctx, target, level, width,
                                                 height, depth, border);

   /* check that the texture won't take too much memory, etc */
   sizeOK = ctx->Driver.TestProxyTexImage(ctx, proxy_target(target),
                                          level, texFormat,
                                          width, height, depth, border);

   if (_mesa_is_proxy_texture(target)) {
      /* Proxy texture: just clear or set state depending on error checking */
      struct gl_texture_image *texImage =
         get_proxy_tex_image(ctx, target, level);

      if (!texImage)
         return;  /* GL_OUT_OF_MEMORY already recorded */

      if (dimensionsOK && sizeOK) {
         _mesa_init_teximage_fields(ctx, texImage, width, height, depth,
                                    border, internalFormat, texFormat);
      }
      else {
         clear_teximage_fields(texImage);
      }
   }
   else {
      /* non-proxy target */
      const GLuint face = _mesa_tex_target_to_face(target);
      struct gl_texture_image *texImage;

      if (!dimensionsOK) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glTexImage%uD(invalid width or height or depth)",
                     dims);
         return;
      }

      if (!sizeOK) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY,
                     "glTexImage%uD(image too large: %d x %d x %d, %s format)",
                     dims, width, height, depth,
                     _mesa_lookup_enum_by_nr(internalFormat));
         return;
      }

      /* Allow a hardware driver to just strip out the border, to provide
       * reliable but slightly incorrect hardware rendering instead of
       * rarely-tested software fallback rendering.
       */
      if (border && ctx->Const.StripTextureBorder) {
	 strip_texture_border(target, &width, &height, &depth, unpack,
			      &unpack_no_border);
         border = 0;
	 unpack = &unpack_no_border;
      }

      if (ctx->NewState & _NEW_PIXEL)
	 _mesa_update_state(ctx);

      _mesa_lock_texture(ctx, texObj);
      {
	 texImage = _mesa_get_tex_image(ctx, texObj, target, level);

	 if (!texImage) {
	    _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s%uD", func, dims);
	 }
         else {
            ctx->Driver.FreeTextureImageBuffer(ctx, texImage);

            _mesa_init_teximage_fields(ctx, texImage,
                                       width, height, depth,
                                       border, internalFormat, texFormat);

            /* Give the texture to the driver.  <pixels> may be null. */
            if (width > 0 && height > 0 && depth > 0) {
               if (compressed) {
                  ctx->Driver.CompressedTexImage(ctx, dims, texImage,
                                                 imageSize, pixels);
               }
               else {
                  ctx->Driver.TexImage(ctx, dims, texImage, format,
                                       type, pixels, unpack);
               }
            }

            check_gen_mipmap(ctx, target, texObj, level);

            _mesa_update_fbo_texture(ctx, texObj, face, level);

            _mesa_dirty_texobj(ctx, texObj);
         }
      }
      _mesa_unlock_texture(ctx, texObj);
   }
}



/*
 * Called from the API.  Note that width includes the border.
 */
void GLAPIENTRY
_mesa_TexImage1D( GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLint border, GLenum format,
                  GLenum type, const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   teximage(ctx, GL_FALSE, 1, target, level, internalFormat, width, 1, 1,
            border, format, type, 0, pixels);
}


void GLAPIENTRY
_mesa_TexImage2D( GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type,
                  const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   teximage(ctx, GL_FALSE, 2, target, level, internalFormat, width, height, 1,
            border, format, type, 0, pixels);
}


/*
 * Called by the API or display list executor.
 * Note that width and height include the border.
 */
void GLAPIENTRY
_mesa_TexImage3D( GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLsizei depth,
                  GLint border, GLenum format, GLenum type,
                  const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   teximage(ctx, GL_FALSE, 3, target, level, internalFormat,
            width, height, depth,
            border, format, type, 0, pixels);
}


void GLAPIENTRY
_mesa_TexImage3DEXT( GLenum target, GLint level, GLenum internalFormat,
                     GLsizei width, GLsizei height, GLsizei depth,
                     GLint border, GLenum format, GLenum type,
                     const GLvoid *pixels )
{
   _mesa_TexImage3D(target, level, (GLint) internalFormat, width, height,
                    depth, border, format, type, pixels);
}


void GLAPIENTRY
_mesa_EGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;
   bool valid_target;
   GET_CURRENT_CONTEXT(ctx);
   FLUSH_VERTICES(ctx, 0);

   switch (target) {
   case GL_TEXTURE_2D:
      valid_target = ctx->Extensions.OES_EGL_image;
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      valid_target =
         _mesa_is_gles(ctx) ? ctx->Extensions.OES_EGL_image_external : false;
      break;
   default:
      valid_target = false;
      break;
   }

   if (!valid_target) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glEGLImageTargetTexture2D(target=%d)", target);
      return;
   }

   if (!image) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glEGLImageTargetTexture2D(image=%p)", image);
      return;
   }

   if (ctx->NewState & _NEW_PIXEL)
      _mesa_update_state(ctx);

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_lock_texture(ctx, texObj);

   if (texObj->Immutable) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glEGLImageTargetTexture2D(texture is immutable)");
      _mesa_unlock_texture(ctx, texObj);
      return;
   }

   texImage = _mesa_get_tex_image(ctx, texObj, target, 0);
   if (!texImage) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "glEGLImageTargetTexture2D");
   } else {
      ctx->Driver.FreeTextureImageBuffer(ctx, texImage);

      ctx->Driver.EGLImageTargetTexture2D(ctx, target,
					  texObj, texImage, image);

      _mesa_dirty_texobj(ctx, texObj);
   }
   _mesa_unlock_texture(ctx, texObj);

}


/**
 * Helper that implements the glTexSubImage1/2/3D()
 * and glTextureSubImage1/2/3D() functions.
 */
void
_mesa_texture_sub_image(struct gl_context *ctx, GLuint dims,
                        struct gl_texture_object *texObj,
                        struct gl_texture_image *texImage,
                        GLenum target, GLint level,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLsizei width, GLsizei height, GLsizei depth,
                        GLenum format, GLenum type, const GLvoid *pixels,
                        bool dsa)
{
   FLUSH_VERTICES(ctx, 0);

   /* check target (proxies not allowed) */
   if (!legal_texsubimage_target(ctx, dims, target, dsa)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glTex%sSubImage%uD(target=%s)",
                  dsa ? "ture" : "",
                  dims, _mesa_lookup_enum_by_nr(target));
      return;
   }

   if (ctx->NewState & _NEW_PIXEL)
      _mesa_update_state(ctx);

   _mesa_lock_texture(ctx, texObj);
   {
      if (width > 0 && height > 0 && depth > 0) {
         /* If we have a border, offset=-1 is legal.  Bias by border width. */
         switch (dims) {
         case 3:
            if (target != GL_TEXTURE_2D_ARRAY)
               zoffset += texImage->Border;
            /* fall-through */
         case 2:
            if (target != GL_TEXTURE_1D_ARRAY)
               yoffset += texImage->Border;
            /* fall-through */
         case 1:
            xoffset += texImage->Border;
         }

         ctx->Driver.TexSubImage(ctx, dims, texImage,
                                 xoffset, yoffset, zoffset,
                                 width, height, depth,
                                 format, type, pixels, &ctx->Unpack);

         check_gen_mipmap(ctx, target, texObj, level);

         /* NOTE: Don't signal _NEW_TEXTURE since we've only changed
          * the texel data, not the texture format, size, etc.
          */
      }
   }
   _mesa_unlock_texture(ctx, texObj);
}

/**
 * Implement all the glTexSubImage1/2/3D() functions.
 * Must split this out this way because of GL_TEXTURE_CUBE_MAP.
 */
static void
texsubimage(struct gl_context *ctx, GLuint dims, GLenum target, GLint level,
            GLint xoffset, GLint yoffset, GLint zoffset,
            GLsizei width, GLsizei height, GLsizei depth,
            GLenum format, GLenum type, const GLvoid *pixels,
            const char *callerName)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   if (texsubimage_error_check(ctx, dims, texObj, target, level,
                               xoffset, yoffset, zoffset,
                               width, height, depth, format, type,
                               pixels, false, callerName)) {
      return;   /* error was detected */
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   /* texsubimage_error_check ensures that texImage is not NULL */

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glTexSubImage%uD %s %d %d %d %d %d %d %d %s %s %p\n",
                  dims,
                  _mesa_lookup_enum_by_nr(target), level,
                  xoffset, yoffset, zoffset, width, height, depth,
                  _mesa_lookup_enum_by_nr(format),
                  _mesa_lookup_enum_by_nr(type), pixels);

   _mesa_texture_sub_image(ctx, dims, texObj, texImage, target, level,
                           xoffset, yoffset, zoffset, width, height, depth,
                           format, type, pixels, false);
}


/**
 * Implement all the glTextureSubImage1/2/3D() functions.
 * Must split this out this way because of GL_TEXTURE_CUBE_MAP.
 */
static void
texturesubimage(struct gl_context *ctx, GLuint dims,
                GLuint texture, GLint level,
                GLint xoffset, GLint yoffset, GLint zoffset,
                GLsizei width, GLsizei height, GLsizei depth,
                GLenum format, GLenum type, const GLvoid *pixels,
                const char *callerName)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;
   int i;

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx,
                  "glTextureSubImage%uD %d %d %d %d %d %d %d %d %s %s %p\n",
                  dims, texture, level,
                  xoffset, yoffset, zoffset, width, height, depth,
                  _mesa_lookup_enum_by_nr(format),
                  _mesa_lookup_enum_by_nr(type), pixels);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTextureSubImage%uD(GL_ARB_direct_state_access "
                  "is not supported)", dims);
      return;
   }

   /* Get the texture object by Name. */
   texObj = _mesa_lookup_texture(ctx, texture);
   if (!texObj) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glTextureSubImage%uD(texture)",
                  dims);
      return;
   }

   if (texsubimage_error_check(ctx, dims, texObj, texObj->Target, level,
                               xoffset, yoffset, zoffset,
                               width, height, depth, format, type,
                               pixels, true, callerName)) {
      return;   /* error was detected */
   }


   /* Must handle special case GL_TEXTURE_CUBE_MAP. */
   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      GLint rowStride;

      /*
       * What do we do if the user created a texture with the following code
       * and then called this function with its handle?
       *
       *    GLuint tex;
       *    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &tex);
       *    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, ...);
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, ...);
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, ...);
       *    // Note: GL_TEXTURE_CUBE_MAP_NEGATIVE_Y not set, or given the
       *    // wrong format, or given the wrong size, etc.
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, ...);
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, ...);
       *
       * A bug has been filed against the spec for this case.  In the
       * meantime, we will check for cube completeness.
       *
       * According to Section 8.17 Texture Completeness in the OpenGL 4.5
       * Core Profile spec (30.10.2014):
       *    "[A] cube map texture is cube complete if the
       *    following conditions all hold true: The [base level] texture
       *    images of each of the six cube map faces have identical, positive,
       *    and square dimensions. The [base level] images were each specified
       *    with the same internal format."
       *
       * It seems reasonable to check for cube completeness of an arbitrary
       * level here so that the image data has a consistent format and size.
       */
      if (!_mesa_cube_level_complete(texObj, level)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glTextureSubImage%uD(cube map incomplete)",
                     dims);
         return;
      }

      rowStride = _mesa_image_image_stride(&ctx->Unpack, width, height,
                                           format, type);
      /* Copy in each face. */
      for (i = 0; i < 6; ++i) {
         texImage = texObj->Image[i][level];
         assert(texImage);

         _mesa_texture_sub_image(ctx, 3, texObj, texImage, texObj->Target,
                                 level, xoffset, yoffset, zoffset,
                                 width, height, 1, format,
                                 type, pixels, true);
         pixels = (GLubyte *) pixels + rowStride;
      }
   }
   else {
      texImage = _mesa_select_tex_image(texObj, texObj->Target, level);
      assert(texImage);

      _mesa_texture_sub_image(ctx, dims, texObj, texImage, texObj->Target,
                              level, xoffset, yoffset, zoffset,
                              width, height, depth, format,
                              type, pixels, true);
   }
}


void GLAPIENTRY
_mesa_TexSubImage1D( GLenum target, GLint level,
                     GLint xoffset, GLsizei width,
                     GLenum format, GLenum type,
                     const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage(ctx, 1, target, level,
               xoffset, 0, 0,
               width, 1, 1,
               format, type, pixels, "glTexSubImage1D");
}


void GLAPIENTRY
_mesa_TexSubImage2D( GLenum target, GLint level,
                     GLint xoffset, GLint yoffset,
                     GLsizei width, GLsizei height,
                     GLenum format, GLenum type,
                     const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage(ctx, 2, target, level,
               xoffset, yoffset, 0,
               width, height, 1,
               format, type, pixels, "glTexSubImage2D");
}



void GLAPIENTRY
_mesa_TexSubImage3D( GLenum target, GLint level,
                     GLint xoffset, GLint yoffset, GLint zoffset,
                     GLsizei width, GLsizei height, GLsizei depth,
                     GLenum format, GLenum type,
                     const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage(ctx, 3, target, level,
               xoffset, yoffset, zoffset,
               width, height, depth,
               format, type, pixels, "glTexSubImage3D");
}

void GLAPIENTRY
_mesa_TextureSubImage1D(GLuint texture, GLint level,
                        GLint xoffset, GLsizei width,
                        GLenum format, GLenum type,
                        const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage(ctx, 1, texture, level,
                   xoffset, 0, 0,
                   width, 1, 1,
                   format, type, pixels, "glTextureSubImage1D");
}


void GLAPIENTRY
_mesa_TextureSubImage2D(GLuint texture, GLint level,
                        GLint xoffset, GLint yoffset,
                        GLsizei width, GLsizei height,
                        GLenum format, GLenum type,
                        const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage(ctx, 2, texture, level,
                   xoffset, yoffset, 0,
                   width, height, 1,
                   format, type, pixels, "glTextureSubImage2D");
}


void GLAPIENTRY
_mesa_TextureSubImage3D(GLuint texture, GLint level,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLsizei width, GLsizei height, GLsizei depth,
                        GLenum format, GLenum type,
                        const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage(ctx, 3, texture, level,
                   xoffset, yoffset, zoffset,
                   width, height, depth,
                   format, type, pixels, "glTextureSubImage3D");
}


/**
 * For glCopyTexSubImage, return the source renderbuffer to copy texel data
 * from.  This depends on whether the texture contains color or depth values.
 */
static struct gl_renderbuffer *
get_copy_tex_image_source(struct gl_context *ctx, mesa_format texFormat)
{
   if (_mesa_get_format_bits(texFormat, GL_DEPTH_BITS) > 0) {
      /* reading from depth/stencil buffer */
      return ctx->ReadBuffer->Attachment[BUFFER_DEPTH].Renderbuffer;
   }
   else {
      /* copying from color buffer */
      return ctx->ReadBuffer->_ColorReadBuffer;
   }
}

static void
copytexsubimage_by_slice(struct gl_context *ctx,
                         struct gl_texture_image *texImage,
                         GLuint dims,
                         GLint xoffset, GLint yoffset, GLint zoffset,
                         struct gl_renderbuffer *rb,
                         GLint x, GLint y,
                         GLsizei width, GLsizei height)
{
   if (texImage->TexObject->Target == GL_TEXTURE_1D_ARRAY) {
      int slice;

      /* For 1D arrays, we copy each scanline of the source rectangle into the
       * next array slice.
       */
      assert(zoffset == 0);

      for (slice = 0; slice < height; slice++) {
         assert(yoffset + slice < texImage->Height);
         ctx->Driver.CopyTexSubImage(ctx, 2, texImage,
                                     xoffset, 0, yoffset + slice,
                                     rb, x, y + slice, width, 1);
      }
   } else {
      ctx->Driver.CopyTexSubImage(ctx, dims, texImage,
                                  xoffset, yoffset, zoffset,
                                  rb, x, y, width, height);
   }
}

static GLboolean
formats_differ_in_component_sizes (mesa_format f1,
                                   mesa_format f2)
{
   GLint f1_r_bits = _mesa_get_format_bits(f1, GL_RED_BITS);
   GLint f1_g_bits = _mesa_get_format_bits(f1, GL_GREEN_BITS);
   GLint f1_b_bits = _mesa_get_format_bits(f1, GL_BLUE_BITS);
   GLint f1_a_bits = _mesa_get_format_bits(f1, GL_ALPHA_BITS);

   GLint f2_r_bits = _mesa_get_format_bits(f2, GL_RED_BITS);
   GLint f2_g_bits = _mesa_get_format_bits(f2, GL_GREEN_BITS);
   GLint f2_b_bits = _mesa_get_format_bits(f2, GL_BLUE_BITS);
   GLint f2_a_bits = _mesa_get_format_bits(f2, GL_ALPHA_BITS);

   if ((f1_r_bits && f2_r_bits && f1_r_bits != f2_r_bits)
       || (f1_g_bits && f2_g_bits && f1_g_bits != f2_g_bits)
       || (f1_b_bits && f2_b_bits && f1_b_bits != f2_b_bits)
       || (f1_a_bits && f2_a_bits && f1_a_bits != f2_a_bits))
      return GL_TRUE;

   return GL_FALSE;
}

/**
 * Implement the glCopyTexImage1/2D() functions.
 */
static void
copyteximage(struct gl_context *ctx, GLuint dims,
             GLenum target, GLint level, GLenum internalFormat,
             GLint x, GLint y, GLsizei width, GLsizei height, GLint border )
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;
   const GLuint face = _mesa_tex_target_to_face(target);
   mesa_format texFormat;
   struct gl_renderbuffer *rb;

   FLUSH_VERTICES(ctx, 0);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glCopyTexImage%uD %s %d %s %d %d %d %d %d\n",
                  dims,
                  _mesa_lookup_enum_by_nr(target), level,
                  _mesa_lookup_enum_by_nr(internalFormat),
                  x, y, width, height, border);

   if (ctx->NewState & NEW_COPY_TEX_STATE)
      _mesa_update_state(ctx);

   if (copytexture_error_check(ctx, dims, target, level, internalFormat,
                               width, height, border))
      return;

   if (!_mesa_legal_texture_dimensions(ctx, target, level, width, height,
                                       1, border)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyTexImage%uD(invalid width or height)", dims);
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   assert(texObj);

   texFormat = _mesa_choose_texture_format(ctx, texObj, target, level,
                                           internalFormat, GL_NONE, GL_NONE);

   rb = _mesa_get_read_renderbuffer_for_format(ctx, internalFormat);

   if (_mesa_is_gles3(ctx)) {
      if (_mesa_is_enum_format_unsized(internalFormat)) {
      /* Conversion from GL_RGB10_A2 source buffer format is not allowed in
       * OpenGL ES 3.0. Khronos bug# 9807.
       */
         if (rb->InternalFormat == GL_RGB10_A2) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glCopyTexImage%uD(Reading from GL_RGB10_A2 buffer and"
                           " writing to unsized internal format)", dims);
               return;
         }
      }
      /* From Page 139 of OpenGL ES 3.0 spec:
       *    "If internalformat is sized, the internal format of the new texel
       *    array is internalformat, and this is also the new texel array’s
       *    effective internal format. If the component sizes of internalformat
       *    do not exactly match the corresponding component sizes of the source
       *    buffer’s effective internal format, described below, an
       *    INVALID_OPERATION error is generated. If internalformat is unsized,
       *    the internal format of the new texel array is the effective internal
       *    format of the source buffer, and this is also the new texel array’s
       *    effective internal format.
       */
      else if (formats_differ_in_component_sizes (texFormat, rb->Format)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glCopyTexImage%uD(componenet size changed in"
                        " internal format)", dims);
            return;
      }
   }

   assert(texFormat != MESA_FORMAT_NONE);

   if (!ctx->Driver.TestProxyTexImage(ctx, proxy_target(target),
                                      level, texFormat,
                                      width, height, 1, border)) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY,
                  "glCopyTexImage%uD(image too large)", dims);
      return;
   }

   if (border && ctx->Const.StripTextureBorder) {
      x += border;
      width -= border * 2;
      if (dims == 2) {
	 y += border;
	 height -= border * 2;
      }
      border = 0;
   }

   _mesa_lock_texture(ctx, texObj);
   {
      texImage = _mesa_get_tex_image(ctx, texObj, target, level);

      if (!texImage) {
	 _mesa_error(ctx, GL_OUT_OF_MEMORY, "glCopyTexImage%uD", dims);
      }
      else {
         GLint srcX = x, srcY = y, dstX = 0, dstY = 0, dstZ = 0;

         /* Free old texture image */
         ctx->Driver.FreeTextureImageBuffer(ctx, texImage);

         _mesa_init_teximage_fields(ctx, texImage, width, height, 1,
                                    border, internalFormat, texFormat);

         if (width && height) {
            /* Allocate texture memory (no pixel data yet) */
            ctx->Driver.AllocTextureImageBuffer(ctx, texImage);

            if (_mesa_clip_copytexsubimage(ctx, &dstX, &dstY, &srcX, &srcY,
                                           &width, &height)) {
               struct gl_renderbuffer *srcRb =
                  get_copy_tex_image_source(ctx, texImage->TexFormat);

               copytexsubimage_by_slice(ctx, texImage, dims,
                                        dstX, dstY, dstZ,
                                        srcRb, srcX, srcY, width, height);
            }

            check_gen_mipmap(ctx, target, texObj, level);
         }

         _mesa_update_fbo_texture(ctx, texObj, face, level);

         _mesa_dirty_texobj(ctx, texObj);
      }
   }
   _mesa_unlock_texture(ctx, texObj);
}



void GLAPIENTRY
_mesa_CopyTexImage1D( GLenum target, GLint level,
                      GLenum internalFormat,
                      GLint x, GLint y,
                      GLsizei width, GLint border )
{
   GET_CURRENT_CONTEXT(ctx);
   copyteximage(ctx, 1, target, level, internalFormat, x, y, width, 1, border);
}



void GLAPIENTRY
_mesa_CopyTexImage2D( GLenum target, GLint level, GLenum internalFormat,
                      GLint x, GLint y, GLsizei width, GLsizei height,
                      GLint border )
{
   GET_CURRENT_CONTEXT(ctx);
   copyteximage(ctx, 2, target, level, internalFormat,
                x, y, width, height, border);
}

/**
 * Implementation for glCopyTex(ture)SubImage1/2/3D() functions.
 */
void
_mesa_copy_texture_sub_image(struct gl_context *ctx, GLuint dims,
                             struct gl_texture_object *texObj,
                             GLenum target, GLint level,
                             GLint xoffset, GLint yoffset, GLint zoffset,
                             GLint x, GLint y,
                             GLsizei width, GLsizei height,
                             const char *caller)
{
   struct gl_texture_image *texImage;

   FLUSH_VERTICES(ctx, 0);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "%s %s %d %d %d %d %d %d %d %d\n", caller,
                  _mesa_lookup_enum_by_nr(target),
                  level, xoffset, yoffset, zoffset, x, y, width, height);

   if (ctx->NewState & NEW_COPY_TEX_STATE)
      _mesa_update_state(ctx);

   if (copytexsubimage_error_check(ctx, dims, texObj, target, level,
                                   xoffset, yoffset, zoffset,
                                   width, height, caller)) {
      return;
   }

   _mesa_lock_texture(ctx, texObj);
   {
      texImage = _mesa_select_tex_image(texObj, target, level);

      /* If we have a border, offset=-1 is legal.  Bias by border width. */
      switch (dims) {
      case 3:
         if (target != GL_TEXTURE_2D_ARRAY)
            zoffset += texImage->Border;
         /* fall-through */
      case 2:
         if (target != GL_TEXTURE_1D_ARRAY)
            yoffset += texImage->Border;
         /* fall-through */
      case 1:
         xoffset += texImage->Border;
      }

      if (_mesa_clip_copytexsubimage(ctx, &xoffset, &yoffset, &x, &y,
                                     &width, &height)) {
         struct gl_renderbuffer *srcRb =
            get_copy_tex_image_source(ctx, texImage->TexFormat);

         copytexsubimage_by_slice(ctx, texImage, dims,
                                  xoffset, yoffset, zoffset,
                                  srcRb, x, y, width, height);

         check_gen_mipmap(ctx, target, texObj, level);

         /* NOTE: Don't signal _NEW_TEXTURE since we've only changed
          * the texel data, not the texture format, size, etc.
          */
      }
   }
   _mesa_unlock_texture(ctx, texObj);
}

void GLAPIENTRY
_mesa_CopyTexSubImage1D( GLenum target, GLint level,
                         GLint xoffset, GLint x, GLint y, GLsizei width )
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTexSubImage1D";
   GET_CURRENT_CONTEXT(ctx);

   /* Check target (proxies not allowed). Target must be checked prior to
    * calling _mesa_get_current_tex_object.
    */
   if (!legal_texsubimage_target(ctx, 1, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_lookup_enum_by_nr(target));
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_copy_texture_sub_image(ctx, 1, texObj, target, level, xoffset, 0, 0,
                                x, y, width, 1, self);
}



void GLAPIENTRY
_mesa_CopyTexSubImage2D( GLenum target, GLint level,
                         GLint xoffset, GLint yoffset,
                         GLint x, GLint y, GLsizei width, GLsizei height )
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTexSubImage2D";
   GET_CURRENT_CONTEXT(ctx);

   /* Check target (proxies not allowed). Target must be checked prior to
    * calling _mesa_get_current_tex_object.
    */
   if (!legal_texsubimage_target(ctx, 2, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_lookup_enum_by_nr(target));
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_copy_texture_sub_image(ctx, 2, texObj, target, level,
                                xoffset, yoffset, 0,
                                x, y, width, height, self);
}



void GLAPIENTRY
_mesa_CopyTexSubImage3D( GLenum target, GLint level,
                         GLint xoffset, GLint yoffset, GLint zoffset,
                         GLint x, GLint y, GLsizei width, GLsizei height )
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTexSubImage3D";
   GET_CURRENT_CONTEXT(ctx);

   /* Check target (proxies not allowed). Target must be checked prior to
    * calling _mesa_get_current_tex_object.
    */
   if (!legal_texsubimage_target(ctx, 3, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_lookup_enum_by_nr(target));
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_copy_texture_sub_image(ctx, 3, texObj, target, level,
                                xoffset, yoffset, zoffset,
                                x, y, width, height, self);
}

void GLAPIENTRY
_mesa_CopyTextureSubImage1D(GLuint texture, GLint level,
                            GLint xoffset, GLint x, GLint y, GLsizei width)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage1D";
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(GL_ARB_direct_state_access is not supported)", self);
      return;
   }

   texObj = _mesa_lookup_texture_err(ctx, texture, self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 1, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_lookup_enum_by_nr(texObj->Target));
      return;
   }

   _mesa_copy_texture_sub_image(ctx, 1, texObj, texObj->Target, level,
                                xoffset, 0, 0, x, y, width, 1, self);
}

void GLAPIENTRY
_mesa_CopyTextureSubImage2D(GLuint texture, GLint level,
                            GLint xoffset, GLint yoffset,
                            GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage2D";
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(GL_ARB_direct_state_access is not supported)", self);
      return;
   }

   texObj = _mesa_lookup_texture_err(ctx, texture, self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 2, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_lookup_enum_by_nr(texObj->Target));
      return;
   }

   _mesa_copy_texture_sub_image(ctx, 2, texObj, texObj->Target, level,
                                xoffset, yoffset, 0,
                                x, y, width, height, self);
}



void GLAPIENTRY
_mesa_CopyTextureSubImage3D(GLuint texture, GLint level,
                            GLint xoffset, GLint yoffset, GLint zoffset,
                            GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage3D";
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(GL_ARB_direct_state_access is not supported)", self);
      return;
   }

   texObj = _mesa_lookup_texture_err(ctx, texture, self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 3, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_lookup_enum_by_nr(texObj->Target));
      return;
   }

   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      /* Act like CopyTexSubImage2D */
      _mesa_copy_texture_sub_image(ctx, 2, texObj,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + zoffset,
                                   level, xoffset, yoffset, 0,
                                   x, y, width, height, self);
   }
   else
      _mesa_copy_texture_sub_image(ctx, 3, texObj, texObj->Target, level,
                                   xoffset, yoffset, zoffset,
                                   x, y, width, height, self);
}

static bool
check_clear_tex_image(struct gl_context *ctx,
                      const char *function,
                      struct gl_texture_image *texImage,
                      GLenum format, GLenum type,
                      const void *data,
                      GLubyte *clearValue)
{
   struct gl_texture_object *texObj = texImage->TexObject;
   static const GLubyte zeroData[MAX_PIXEL_BYTES];
   GLenum internalFormat = texImage->InternalFormat;
   GLenum err;

   if (texObj->Target == GL_TEXTURE_BUFFER) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer texture)", function);
      return false;
   }

   if (_mesa_is_compressed_format(ctx, internalFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(compressed texture)", function);
      return false;
   }

   err = _mesa_error_check_format_and_type(ctx, format, type);
   if (err != GL_NO_ERROR) {
      _mesa_error(ctx, err,
                  "%s(incompatible format = %s, type = %s)",
                  function,
                  _mesa_lookup_enum_by_nr(format),
                  _mesa_lookup_enum_by_nr(type));
      return false;
   }

   /* make sure internal format and format basically agree */
   if (!texture_formats_agree(internalFormat, format)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(incompatible internalFormat = %s, format = %s)",
                  function,
                  _mesa_lookup_enum_by_nr(internalFormat),
                  _mesa_lookup_enum_by_nr(format));
      return false;
   }

   if (ctx->Version >= 30 || ctx->Extensions.EXT_texture_integer) {
      /* both source and dest must be integer-valued, or neither */
      if (_mesa_is_format_integer_color(texImage->TexFormat) !=
          _mesa_is_enum_format_integer(format)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(integer/non-integer format mismatch)",
                     function);
         return false;
      }
   }

   if (!_mesa_texstore(ctx,
                       1, /* dims */
                       texImage->_BaseFormat,
                       texImage->TexFormat,
                       0, /* dstRowStride */
                       &clearValue,
                       1, 1, 1, /* srcWidth/Height/Depth */
                       format, type,
                       data ? data : zeroData,
                       &ctx->DefaultPacking)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid format)", function);
      return false;
   }

   return true;
}

static struct gl_texture_object *
get_tex_obj_for_clear(struct gl_context *ctx,
                      const char *function,
                      GLuint texture)
{
   struct gl_texture_object *texObj;

   if (texture == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(zero texture)", function);
      return NULL;
   }

   texObj = _mesa_lookup_texture(ctx, texture);

   if (texObj == NULL) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(non-gen name)", function);
      return NULL;
   }

   if (texObj->Target == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(unbound tex)", function);
      return NULL;
   }

   return texObj;
}

static int
get_tex_images_for_clear(struct gl_context *ctx,
                         const char *function,
                         struct gl_texture_object *texObj,
                         GLint level,
                         struct gl_texture_image **texImages)
{
   GLenum target;
   int i;

   if (level < 0 || level >= MAX_TEXTURE_LEVELS) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid level)", function);
      return 0;
   }

   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      for (i = 0; i < MAX_FACES; i++) {
         target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

         texImages[i] = _mesa_select_tex_image(texObj, target, level);
         if (texImages[i] == NULL) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(invalid level)", function);
            return 0;
         }
      }

      return MAX_FACES;
   }

   texImages[0] = _mesa_select_tex_image(texObj, texObj->Target, level);

   if (texImages[0] == NULL) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid level)", function);
      return 0;
   }

   return 1;
}

void GLAPIENTRY
_mesa_ClearTexSubImage( GLuint texture, GLint level,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLsizei width, GLsizei height, GLsizei depth,
                        GLenum format, GLenum type, const void *data )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImages[MAX_FACES];
   GLubyte clearValue[MAX_FACES][MAX_PIXEL_BYTES];
   int i, numImages;
   int minDepth, maxDepth;

   texObj = get_tex_obj_for_clear(ctx, "glClearTexSubImage", texture);

   if (texObj == NULL)
      return;

   _mesa_lock_texture(ctx, texObj);

   numImages = get_tex_images_for_clear(ctx, "glClearTexSubImage",
                                        texObj, level, texImages);
   if (numImages == 0)
      goto out;

   if (numImages == 1) {
      minDepth = -(int) texImages[0]->Border;
      maxDepth = texImages[0]->Depth;
   } else {
      minDepth = 0;
      maxDepth = numImages;
   }

   if (xoffset < -(GLint) texImages[0]->Border ||
       yoffset < -(GLint) texImages[0]->Border ||
       zoffset < minDepth ||
       width < 0 ||
       height < 0 ||
       depth < 0 ||
       xoffset + width > texImages[0]->Width ||
       yoffset + height > texImages[0]->Height ||
       zoffset + depth > maxDepth) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glClearSubTexImage(invalid dimensions)");
      goto out;
   }

   if (numImages == 1) {
      if (check_clear_tex_image(ctx, "glClearTexSubImage",
                                texImages[0],
                                format, type, data, clearValue[0])) {
         ctx->Driver.ClearTexSubImage(ctx,
                                      texImages[0],
                                      xoffset, yoffset, zoffset,
                                      width, height, depth,
                                      data ? clearValue[0] : NULL);
      }
   } else {
      for (i = zoffset; i < zoffset + depth; i++) {
         if (!check_clear_tex_image(ctx, "glClearTexSubImage",
                                    texImages[i],
                                    format, type, data, clearValue[i]))
            goto out;
      }
      for (i = zoffset; i < zoffset + depth; i++) {
         ctx->Driver.ClearTexSubImage(ctx,
                                      texImages[i],
                                      xoffset, yoffset, 0,
                                      width, height, 1,
                                      data ? clearValue[i] : NULL);
      }
   }

 out:
   _mesa_unlock_texture(ctx, texObj);
}

void GLAPIENTRY
_mesa_ClearTexImage( GLuint texture, GLint level,
                     GLenum format, GLenum type, const void *data )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImages[MAX_FACES];
   GLubyte clearValue[MAX_FACES][MAX_PIXEL_BYTES];
   int i, numImages;

   texObj = get_tex_obj_for_clear(ctx, "glClearTexImage", texture);

   if (texObj == NULL)
      return;

   _mesa_lock_texture(ctx, texObj);

   numImages = get_tex_images_for_clear(ctx, "glClearTexImage",
                                        texObj, level, texImages);

   for (i = 0; i < numImages; i++) {
      if (!check_clear_tex_image(ctx, "glClearTexImage",
                                 texImages[i],
                                 format, type, data,
                                 clearValue[i]))
         goto out;
   }

   for (i = 0; i < numImages; i++) {
      ctx->Driver.ClearTexSubImage(ctx, texImages[i],
                                   -(GLint) texImages[i]->Border, /* xoffset */
                                   -(GLint) texImages[i]->Border, /* yoffset */
                                   -(GLint) texImages[i]->Border, /* zoffset */
                                   texImages[i]->Width,
                                   texImages[i]->Height,
                                   texImages[i]->Depth,
                                   data ? clearValue[i] : NULL);
   }

out:
   _mesa_unlock_texture(ctx, texObj);
}




/**********************************************************************/
/******                   Compressed Textures                    ******/
/**********************************************************************/


/**
 * Target checking for glCompressedTexSubImage[123]D().
 * \return GL_TRUE if error, GL_FALSE if no error
 * Must come before other error checking so that the texture object can
 * be correctly retrieved using _mesa_get_current_tex_object.
 */
static GLboolean
compressed_subtexture_target_check(struct gl_context *ctx, GLenum target,
                                   GLint dims, GLenum format, bool dsa,
                                   const char *caller)
{
   GLboolean targetOK;

   if (dsa && target == GL_TEXTURE_RECTANGLE) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid target %s)", caller,
                  _mesa_lookup_enum_by_nr(target));
      return GL_TRUE;
   }

   switch (dims) {
   case 2:
      switch (target) {
      case GL_TEXTURE_2D:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         targetOK = GL_TRUE;
         break;
      default:
         targetOK = GL_FALSE;
         break;
      }
      break;
   case 3:
      targetOK = (target == GL_TEXTURE_3D) ||
                 (target == GL_TEXTURE_2D_ARRAY) ||
                 (target == GL_TEXTURE_CUBE_MAP_ARRAY) ||
                 (target == GL_TEXTURE_CUBE_MAP && dsa);

      /* OpenGL 4.5 spec (30.10.2014) says in Section 8.7 Compressed Texture
       * Images:
       *    "An INVALID_OPERATION error is generated by
       *    CompressedTex*SubImage3D if the internal format of the texture is
       *    one of the EAC, ETC2, or RGTC formats and either border is
       *    non-zero, or the effective target for the texture is not
       *    TEXTURE_2D_ARRAY."
       */
      if (target != GL_TEXTURE_2D_ARRAY) {
         bool invalidformat;
         switch (format) {
            /* These came from _mesa_is_compressed_format in glformats.c. */
            /* EAC formats */
            case GL_COMPRESSED_RGBA8_ETC2_EAC:
            case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
            case GL_COMPRESSED_R11_EAC:
            case GL_COMPRESSED_RG11_EAC:
            case GL_COMPRESSED_SIGNED_R11_EAC:
            case GL_COMPRESSED_SIGNED_RG11_EAC:
            /* ETC2 formats */
            case GL_COMPRESSED_RGB8_ETC2:
            case GL_COMPRESSED_SRGB8_ETC2:
            case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            /* RGTC formats */
            case GL_COMPRESSED_RED_RGTC1:
            case GL_COMPRESSED_SIGNED_RED_RGTC1:
            case GL_COMPRESSED_RG_RGTC2:
            case GL_COMPRESSED_SIGNED_RG_RGTC2:
               invalidformat = true;
               break;
            default:
               invalidformat = false;
         }
         if (invalidformat) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(invalid target %s for format %s)", caller,
                        _mesa_lookup_enum_by_nr(target),
                        _mesa_lookup_enum_by_nr(format));
            return GL_TRUE;
         }
      }

      break;
   default:
      assert(dims == 1);
      /* no 1D compressed textures at this time */
      targetOK = GL_FALSE;
      break;
   }

   if (!targetOK) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", caller,
                  _mesa_lookup_enum_by_nr(target));
      return GL_TRUE;
   }

   return GL_FALSE;
}

/**
 * Error checking for glCompressedTexSubImage[123]D().
 * \return GL_TRUE if error, GL_FALSE if no error
 */
static GLboolean
compressed_subtexture_error_check(struct gl_context *ctx, GLint dims,
                                  const struct gl_texture_object *texObj,
                                  GLenum target, GLint level,
                                  GLint xoffset, GLint yoffset, GLint zoffset,
                                  GLsizei width, GLsizei height, GLsizei depth,
                                  GLenum format, GLsizei imageSize,
                                  const GLvoid *data, const char *callerName)
{
   struct gl_texture_image *texImage;
   GLint expectedSize;

   /* this will catch any invalid compressed format token */
   if (!_mesa_is_compressed_format(ctx, format)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(format)", callerName);
      return GL_TRUE;
   }

   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(level=%d)",
                  callerName, level);
      return GL_TRUE;
   }

   /* validate the bound PBO, if any */
   if (!_mesa_validate_pbo_source_compressed(ctx, dims, &ctx->Unpack,
                                     imageSize, data, callerName)) {
      return GL_TRUE;
   }

   /* Check for invalid pixel storage modes */
   if (!_mesa_compressed_pixel_storage_error_check(ctx, dims,
                                                   &ctx->Unpack, callerName)) {
      return GL_TRUE;
   }

   expectedSize = compressed_tex_size(width, height, depth, format);
   if (expectedSize != imageSize) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(size=%d)",
                  callerName, imageSize);
      return GL_TRUE;
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   if (!texImage) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(invalid texture image)",
                  callerName);
      return GL_TRUE;
   }

   if ((GLint) format != texImage->InternalFormat) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(format=0x%x)",
                  callerName, format);
      return GL_TRUE;
   }

   if (compressedteximage_only_format(ctx, format)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
               "%s(format=0x%x cannot be updated)",
               callerName, format);
      return GL_TRUE;
   }

   if (error_check_subtexture_dimensions(ctx, dims,
                                         texImage, xoffset, yoffset, zoffset,
                                         width, height, depth,
                                         callerName)) {
      return GL_TRUE;
   }

   return GL_FALSE;
}


void GLAPIENTRY
_mesa_CompressedTexImage1D(GLenum target, GLint level,
                              GLenum internalFormat, GLsizei width,
                              GLint border, GLsizei imageSize,
                              const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage(ctx, GL_TRUE, 1, target, level, internalFormat,
            width, 1, 1, border, GL_NONE, GL_NONE, imageSize, data);
}


void GLAPIENTRY
_mesa_CompressedTexImage2D(GLenum target, GLint level,
                              GLenum internalFormat, GLsizei width,
                              GLsizei height, GLint border, GLsizei imageSize,
                              const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage(ctx, GL_TRUE, 2, target, level, internalFormat,
            width, height, 1, border, GL_NONE, GL_NONE, imageSize, data);
}


void GLAPIENTRY
_mesa_CompressedTexImage3D(GLenum target, GLint level,
                              GLenum internalFormat, GLsizei width,
                              GLsizei height, GLsizei depth, GLint border,
                              GLsizei imageSize, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage(ctx, GL_TRUE, 3, target, level, internalFormat,
            width, height, depth, border, GL_NONE, GL_NONE, imageSize, data);
}


/**
 * Common helper for glCompressedTexSubImage1/2/3D() and
 * glCompressedTextureSubImage1/2/3D().
 */
void
_mesa_compressed_texture_sub_image(struct gl_context *ctx, GLuint dims,
                                   struct gl_texture_object *texObj,
                                   struct gl_texture_image *texImage,
                                   GLenum target, GLint level,
                                   GLint xoffset, GLint yoffset,
                                   GLint zoffset,
                                   GLsizei width, GLsizei height,
                                   GLsizei depth,
                                   GLenum format, GLsizei imageSize,
                                   const GLvoid *data)
{
   FLUSH_VERTICES(ctx, 0);

   _mesa_lock_texture(ctx, texObj);
   {
      if (width > 0 && height > 0 && depth > 0) {
         ctx->Driver.CompressedTexSubImage(ctx, dims, texImage,
                                           xoffset, yoffset, zoffset,
                                           width, height, depth,
                                           format, imageSize, data);

         check_gen_mipmap(ctx, target, texObj, level);

         /* NOTE: Don't signal _NEW_TEXTURE since we've only changed
          * the texel data, not the texture format, size, etc.
          */
      }
   }
   _mesa_unlock_texture(ctx, texObj);
}


void GLAPIENTRY
_mesa_CompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                              GLsizei width, GLenum format,
                              GLsizei imageSize, const GLvoid *data)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   GET_CURRENT_CONTEXT(ctx);

   if (compressed_subtexture_target_check(ctx, target, 1, format, false,
                                          "glCompressedTexSubImage1D")) {
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   if (compressed_subtexture_error_check(ctx, 1, texObj, target,
                                         level, xoffset, 0, 0,
                                         width, 1, 1,
                                         format, imageSize, data,
                                         "glCompressedTexSubImage1D")) {
      return;
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   assert(texImage);

   _mesa_compressed_texture_sub_image(ctx, 1, texObj, texImage, target, level,
                                      xoffset, 0, 0, width, 1, 1,
                                      format, imageSize, data);
}

void GLAPIENTRY
_mesa_CompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset,
                                  GLsizei width, GLenum format,
                                  GLsizei imageSize, const GLvoid *data)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCompressedTextureSubImage1D(GL_ARB_direct_state_access "
                  "is not supported)");
      return;
   }

   texObj = _mesa_lookup_texture_err(ctx, texture,
                                     "glCompressedTextureSubImage1D");
   if (!texObj)
      return;

   if (compressed_subtexture_target_check(ctx, texObj->Target, 1, format,
                                          true,
                                          "glCompressedTextureSubImage1D")) {
      return;
   }

   if (compressed_subtexture_error_check(ctx, 1, texObj, texObj->Target,
                                         level, xoffset, 0, 0,
                                         width, 1, 1,
                                         format, imageSize, data,
                                         "glCompressedTextureSubImage1D")) {
      return;
   }

   texImage = _mesa_select_tex_image(texObj, texObj->Target, level);
   assert(texImage);

   _mesa_compressed_texture_sub_image(ctx, 1, texObj, texImage,
                                      texObj->Target, level,
                                      xoffset, 0, 0, width, 1, 1,
                                      format, imageSize, data);
}


void GLAPIENTRY
_mesa_CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                              GLint yoffset, GLsizei width, GLsizei height,
                              GLenum format, GLsizei imageSize,
                              const GLvoid *data)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   GET_CURRENT_CONTEXT(ctx);

   if (compressed_subtexture_target_check(ctx, target, 2, format, false,
                                          "glCompressedTexSubImage2D")) {
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   if (compressed_subtexture_error_check(ctx, 2, texObj, target,
                                         level, xoffset, yoffset, 0,
                                         width, height, 1,
                                         format, imageSize, data,
                                         "glCompressedTexSubImage2D")) {
      return;
   }


   texImage = _mesa_select_tex_image(texObj, target, level);
   assert(texImage);

   _mesa_compressed_texture_sub_image(ctx, 2, texObj, texImage, target, level,
                                      xoffset, yoffset, 0, width, height, 1,
                                      format, imageSize, data);
}

void GLAPIENTRY
_mesa_CompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset,
                                  GLint yoffset,
                                  GLsizei width, GLsizei height,
                                  GLenum format, GLsizei imageSize,
                                  const GLvoid *data)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCompressedTextureSubImage2D(GL_ARB_direct_state_access "
                  "is not supported)");
      return;
   }

   texObj = _mesa_lookup_texture_err(ctx, texture,
                                 "glCompressedTextureSubImage2D");
   if (!texObj)
      return;

   if (compressed_subtexture_target_check(ctx, texObj->Target, 2, format,
                                          true,
                                          "glCompressedTextureSubImage2D")) {
      return;
   }

   if (compressed_subtexture_error_check(ctx, 2, texObj, texObj->Target,
                                         level, xoffset, yoffset, 0,
                                         width, height, 1,
                                         format, imageSize, data,
                                         "glCompressedTextureSubImage2D")) {
      return;
   }

   texImage = _mesa_select_tex_image(texObj, texObj->Target, level);
   assert(texImage);

   _mesa_compressed_texture_sub_image(ctx, 2, texObj, texImage,
                                      texObj->Target, level,
                                      xoffset, yoffset, 0, width, height, 1,
                                      format, imageSize, data);
}

void GLAPIENTRY
_mesa_CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset,
                              GLint yoffset, GLint zoffset, GLsizei width,
                              GLsizei height, GLsizei depth, GLenum format,
                              GLsizei imageSize, const GLvoid *data)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   GET_CURRENT_CONTEXT(ctx);

   if (compressed_subtexture_target_check(ctx, target, 3, format, false,
                                          "glCompressedTexSubImage3D")) {
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   if (compressed_subtexture_error_check(ctx, 3, texObj, target,
                                         level, xoffset, yoffset, zoffset,
                                         width, height, depth,
                                         format, imageSize, data,
                                         "glCompressedTexSubImage3D")) {
      return;
   }


   texImage = _mesa_select_tex_image(texObj, target, level);
   assert(texImage);

   _mesa_compressed_texture_sub_image(ctx, 3, texObj, texImage, target, level,
                                      xoffset, yoffset, zoffset,
                                      width, height, depth,
                                      format, imageSize, data);
}

void GLAPIENTRY
_mesa_CompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset,
                                  GLint yoffset, GLint zoffset, GLsizei width,
                                  GLsizei height, GLsizei depth,
                                  GLenum format, GLsizei imageSize,
                                  const GLvoid *data)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCompressedTextureSubImage3D(GL_ARB_direct_state_access "
                  "is not supported)");
      return;
   }

   texObj = _mesa_lookup_texture_err(ctx, texture,
                                     "glCompressedTextureSubImage3D");
   if (!texObj)
      return;

   if (compressed_subtexture_target_check(ctx, texObj->Target, 3, format,
                                          true,
                                          "glCompressedTextureSubImage3D")) {
      return;
   }

   if (compressed_subtexture_error_check(ctx, 3, texObj, texObj->Target,
                                         level, xoffset, yoffset, zoffset,
                                         width, height, depth,
                                         format, imageSize, data,
                                         "glCompressedTextureSubImage3D")) {
      return;
   }

   /* Must handle special case GL_TEXTURE_CUBE_MAP. */
   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      const char *pixels = data;
      int i;
      GLint image_stride;

      /* Make sure the texture object is a proper cube.
       * (See texturesubimage in teximage.c for details on why this check is
       * performed.)
       */
      if (!_mesa_cube_level_complete(texObj, level)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCompressedTextureSubImage3D(cube map incomplete)");
         return;
      }

      /* Copy in each face. */
      for (i = 0; i < 6; ++i) {
         texImage = texObj->Image[i][level];
         assert(texImage);

         _mesa_compressed_texture_sub_image(ctx, 3, texObj, texImage,
                                            texObj->Target, level,
                                            xoffset, yoffset, zoffset,
                                            width, height, 1,
                                            format, imageSize, pixels);

         /* Compressed images don't have a client format */
         image_stride = _mesa_format_image_size(texImage->TexFormat,
                                                texImage->Width,
                                                texImage->Height, 1);

         pixels += image_stride;
         imageSize -= image_stride;
      }
   }
   else {
      texImage = _mesa_select_tex_image(texObj, texObj->Target, level);
      assert(texImage);

      _mesa_compressed_texture_sub_image(ctx, 3, texObj, texImage,
                                         texObj->Target, level,
                                         xoffset, yoffset, zoffset,
                                         width, height, depth,
                                         format, imageSize, data);
   }
}

static mesa_format
get_texbuffer_format(const struct gl_context *ctx, GLenum internalFormat)
{
   if (ctx->API != API_OPENGL_CORE) {
      switch (internalFormat) {
      case GL_ALPHA8:
         return MESA_FORMAT_A_UNORM8;
      case GL_ALPHA16:
         return MESA_FORMAT_A_UNORM16;
      case GL_ALPHA16F_ARB:
         return MESA_FORMAT_A_FLOAT16;
      case GL_ALPHA32F_ARB:
         return MESA_FORMAT_A_FLOAT32;
      case GL_ALPHA8I_EXT:
         return MESA_FORMAT_A_SINT8;
      case GL_ALPHA16I_EXT:
         return MESA_FORMAT_A_SINT16;
      case GL_ALPHA32I_EXT:
         return MESA_FORMAT_A_SINT32;
      case GL_ALPHA8UI_EXT:
         return MESA_FORMAT_A_UINT8;
      case GL_ALPHA16UI_EXT:
         return MESA_FORMAT_A_UINT16;
      case GL_ALPHA32UI_EXT:
         return MESA_FORMAT_A_UINT32;
      case GL_LUMINANCE8:
         return MESA_FORMAT_L_UNORM8;
      case GL_LUMINANCE16:
         return MESA_FORMAT_L_UNORM16;
      case GL_LUMINANCE16F_ARB:
         return MESA_FORMAT_L_FLOAT16;
      case GL_LUMINANCE32F_ARB:
         return MESA_FORMAT_L_FLOAT32;
      case GL_LUMINANCE8I_EXT:
         return MESA_FORMAT_L_SINT8;
      case GL_LUMINANCE16I_EXT:
         return MESA_FORMAT_L_SINT16;
      case GL_LUMINANCE32I_EXT:
         return MESA_FORMAT_L_SINT32;
      case GL_LUMINANCE8UI_EXT:
         return MESA_FORMAT_L_UINT8;
      case GL_LUMINANCE16UI_EXT:
         return MESA_FORMAT_L_UINT16;
      case GL_LUMINANCE32UI_EXT:
         return MESA_FORMAT_L_UINT32;
      case GL_LUMINANCE8_ALPHA8:
         return MESA_FORMAT_L8A8_UNORM;
      case GL_LUMINANCE16_ALPHA16:
         return MESA_FORMAT_L16A16_UNORM;
      case GL_LUMINANCE_ALPHA16F_ARB:
         return MESA_FORMAT_LA_FLOAT16;
      case GL_LUMINANCE_ALPHA32F_ARB:
         return MESA_FORMAT_LA_FLOAT32;
      case GL_LUMINANCE_ALPHA8I_EXT:
         return MESA_FORMAT_LA_SINT8;
      case GL_LUMINANCE_ALPHA16I_EXT:
         return MESA_FORMAT_LA_SINT16;
      case GL_LUMINANCE_ALPHA32I_EXT:
         return MESA_FORMAT_LA_SINT32;
      case GL_LUMINANCE_ALPHA8UI_EXT:
         return MESA_FORMAT_LA_UINT8;
      case GL_LUMINANCE_ALPHA16UI_EXT:
         return MESA_FORMAT_LA_UINT16;
      case GL_LUMINANCE_ALPHA32UI_EXT:
         return MESA_FORMAT_LA_UINT32;
      case GL_INTENSITY8:
         return MESA_FORMAT_I_UNORM8;
      case GL_INTENSITY16:
         return MESA_FORMAT_I_UNORM16;
      case GL_INTENSITY16F_ARB:
         return MESA_FORMAT_I_FLOAT16;
      case GL_INTENSITY32F_ARB:
         return MESA_FORMAT_I_FLOAT32;
      case GL_INTENSITY8I_EXT:
         return MESA_FORMAT_I_SINT8;
      case GL_INTENSITY16I_EXT:
         return MESA_FORMAT_I_SINT16;
      case GL_INTENSITY32I_EXT:
         return MESA_FORMAT_I_SINT32;
      case GL_INTENSITY8UI_EXT:
         return MESA_FORMAT_I_UINT8;
      case GL_INTENSITY16UI_EXT:
         return MESA_FORMAT_I_UINT16;
      case GL_INTENSITY32UI_EXT:
         return MESA_FORMAT_I_UINT32;
      default:
         break;
      }
   }

   if (ctx->API == API_OPENGL_CORE &&
       ctx->Extensions.ARB_texture_buffer_object_rgb32) {
      switch (internalFormat) {
      case GL_RGB32F:
         return MESA_FORMAT_RGB_FLOAT32;
      case GL_RGB32UI:
         return MESA_FORMAT_RGB_UINT32;
      case GL_RGB32I:
         return MESA_FORMAT_RGB_SINT32;
      default:
         break;
      }
   }

   switch (internalFormat) {
   case GL_RGBA8:
      return MESA_FORMAT_R8G8B8A8_UNORM;
   case GL_RGBA16:
      return MESA_FORMAT_RGBA_UNORM16;
   case GL_RGBA16F_ARB:
      return MESA_FORMAT_RGBA_FLOAT16;
   case GL_RGBA32F_ARB:
      return MESA_FORMAT_RGBA_FLOAT32;
   case GL_RGBA8I_EXT:
      return MESA_FORMAT_RGBA_SINT8;
   case GL_RGBA16I_EXT:
      return MESA_FORMAT_RGBA_SINT16;
   case GL_RGBA32I_EXT:
      return MESA_FORMAT_RGBA_SINT32;
   case GL_RGBA8UI_EXT:
      return MESA_FORMAT_RGBA_UINT8;
   case GL_RGBA16UI_EXT:
      return MESA_FORMAT_RGBA_UINT16;
   case GL_RGBA32UI_EXT:
      return MESA_FORMAT_RGBA_UINT32;

   case GL_RG8:
      return MESA_FORMAT_R8G8_UNORM;
   case GL_RG16:
      return MESA_FORMAT_R16G16_UNORM;
   case GL_RG16F:
      return MESA_FORMAT_RG_FLOAT16;
   case GL_RG32F:
      return MESA_FORMAT_RG_FLOAT32;
   case GL_RG8I:
      return MESA_FORMAT_RG_SINT8;
   case GL_RG16I:
      return MESA_FORMAT_RG_SINT16;
   case GL_RG32I:
      return MESA_FORMAT_RG_SINT32;
   case GL_RG8UI:
      return MESA_FORMAT_RG_UINT8;
   case GL_RG16UI:
      return MESA_FORMAT_RG_UINT16;
   case GL_RG32UI:
      return MESA_FORMAT_RG_UINT32;

   case GL_R8:
      return MESA_FORMAT_R_UNORM8;
   case GL_R16:
      return MESA_FORMAT_R_UNORM16;
   case GL_R16F:
      return MESA_FORMAT_R_FLOAT16;
   case GL_R32F:
      return MESA_FORMAT_R_FLOAT32;
   case GL_R8I:
      return MESA_FORMAT_R_SINT8;
   case GL_R16I:
      return MESA_FORMAT_R_SINT16;
   case GL_R32I:
      return MESA_FORMAT_R_SINT32;
   case GL_R8UI:
      return MESA_FORMAT_R_UINT8;
   case GL_R16UI:
      return MESA_FORMAT_R_UINT16;
   case GL_R32UI:
      return MESA_FORMAT_R_UINT32;

   default:
      return MESA_FORMAT_NONE;
   }
}


mesa_format
_mesa_validate_texbuffer_format(const struct gl_context *ctx,
                                GLenum internalFormat)
{
   mesa_format format = get_texbuffer_format(ctx, internalFormat);
   GLenum datatype;

   if (format == MESA_FORMAT_NONE)
      return MESA_FORMAT_NONE;

   datatype = _mesa_get_format_datatype(format);

   /* The GL_ARB_texture_buffer_object spec says:
    *
    *     "If ARB_texture_float is not supported, references to the
    *     floating-point internal formats provided by that extension should be
    *     removed, and such formats may not be passed to TexBufferARB."
    *
    * As a result, GL_HALF_FLOAT internal format depends on both
    * GL_ARB_texture_float and GL_ARB_half_float_pixel.
    */
   if ((datatype == GL_FLOAT || datatype == GL_HALF_FLOAT) &&
       !ctx->Extensions.ARB_texture_float)
      return MESA_FORMAT_NONE;

   if (!ctx->Extensions.ARB_texture_rg) {
      GLenum base_format = _mesa_get_format_base_format(format);
      if (base_format == GL_R || base_format == GL_RG)
         return MESA_FORMAT_NONE;
   }

   if (!ctx->Extensions.ARB_texture_buffer_object_rgb32) {
      GLenum base_format = _mesa_get_format_base_format(format);
      if (base_format == GL_RGB)
         return MESA_FORMAT_NONE;
   }
   return format;
}


void
_mesa_texture_buffer_range(struct gl_context *ctx,
                           struct gl_texture_object *texObj,
                           GLenum internalFormat,
                           struct gl_buffer_object *bufObj,
                           GLintptr offset, GLsizeiptr size,
                           const char *caller)
{
   mesa_format format;

   /* NOTE: ARB_texture_buffer_object has interactions with
    * the compatibility profile that are not implemented.
    */
   if (!(ctx->API == API_OPENGL_CORE &&
         ctx->Extensions.ARB_texture_buffer_object)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(ARB_texture_buffer_object is not"
                  " implemented for the compatibility profile)", caller);
      return;
   }

   format = _mesa_validate_texbuffer_format(ctx, internalFormat);
   if (format == MESA_FORMAT_NONE) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(internalFormat 0x%x)", caller, internalFormat);
      return;
   }

   FLUSH_VERTICES(ctx, 0);

   _mesa_lock_texture(ctx, texObj);
   {
      _mesa_reference_buffer_object(ctx, &texObj->BufferObject, bufObj);
      texObj->BufferObjectFormat = internalFormat;
      texObj->_BufferObjectFormat = format;
      texObj->BufferOffset = offset;
      texObj->BufferSize = size;
   }
   _mesa_unlock_texture(ctx, texObj);

   ctx->NewDriverState |= ctx->DriverFlags.NewTextureBuffer;

   if (bufObj) {
      bufObj->UsageHistory |= USAGE_TEXTURE_BUFFER;
   }
}


/**
 * Make sure the texture buffer target is GL_TEXTURE_BUFFER.
 * Return true if it is, and return false if it is not
 * (and throw INVALID ENUM as dictated in the OpenGL 4.5
 * core spec, 02.02.2015, PDF page 245).
 */
static bool
check_texture_buffer_target(struct gl_context *ctx, GLenum target,
                            const char *caller)
{
   if (target != GL_TEXTURE_BUFFER_ARB) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(texture target is not GL_TEXTURE_BUFFER)", caller);
      return false;
   }
   else
      return true;
}

/**
 * Check for errors related to the texture buffer range.
 * Return false if errors are found, true if none are found.
 */
static bool
check_texture_buffer_range(struct gl_context *ctx,
                           struct gl_buffer_object *bufObj,
                           GLintptr offset, GLsizeiptr size,
                           const char *caller)
{
   /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
    * Textures (PDF page 245):
    *    "An INVALID_VALUE error is generated if offset is negative, if
    *    size is less than or equal to zero, or if offset + size is greater
    *    than the value of BUFFER_SIZE for the buffer bound to target."
    */
   if (offset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(offset=%d < 0)", caller,
                  (int) offset);
      return false;
   }

   if (size <= 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(size=%d <= 0)", caller,
                  (int) size);
      return false;
   }

   if (offset + size > bufObj->Size) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(offset=%d + size=%d > buffer_size=%d)", caller,
                  (int) offset, (int) size, (int) bufObj->Size);
      return false;
   }

   /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
    * Textures (PDF page 245):
    *    "An INVALID_VALUE error is generated if offset is not an integer
    *    multiple of the value of TEXTURE_BUFFER_OFFSET_ALIGNMENT."
    */
   if (offset % ctx->Const.TextureBufferOffsetAlignment) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(invalid offset alignment)", caller);
      return false;
   }

   return true;
}


/** GL_ARB_texture_buffer_object */
void GLAPIENTRY
_mesa_TexBuffer(GLenum target, GLenum internalFormat, GLuint buffer)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   /* Need to catch a bad target before it gets to
    * _mesa_get_current_tex_object.
    */
   if (!check_texture_buffer_target(ctx, target, "glTexBuffer"))
      return;

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glTexBuffer");
      if (!bufObj)
         return;
   } else
      bufObj = NULL;

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_texture_buffer_range(ctx, texObj, internalFormat, bufObj, 0,
                              buffer ? -1 : 0, "glTexBuffer");
}


/** GL_ARB_texture_buffer_range */
void GLAPIENTRY
_mesa_TexBufferRange(GLenum target, GLenum internalFormat, GLuint buffer,
                     GLintptr offset, GLsizeiptr size)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   /* Need to catch a bad target before it gets to
    * _mesa_get_current_tex_object.
    */
   if (!check_texture_buffer_target(ctx, target, "glTexBufferRange"))
      return;

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glTexBufferRange");
      if (!bufObj)
         return;

      if (!check_texture_buffer_range(ctx, bufObj, offset, size,
          "glTexBufferRange"))
         return;

   } else {

      /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
       * Textures (PDF page 254):
       *    "If buffer is zero, then any buffer object attached to the buffer
       *    texture is detached, the values offset and size are ignored and
       *    the state for offset and size for the buffer texture are reset to
       *    zero."
       */
      offset = 0;
      size = 0;
      bufObj = NULL;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_texture_buffer_range(ctx, texObj, internalFormat, bufObj,
                              offset, size, "glTexBufferRange");
}

void GLAPIENTRY
_mesa_TextureBuffer(GLuint texture, GLenum internalFormat, GLuint buffer)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTextureBuffer(GL_ARB_direct_state_access "
                  "is not supported)");
      return;
   }

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glTextureBuffer");
      if (!bufObj)
         return;
   } else
      bufObj = NULL;

   /* Get the texture object by Name. */
   texObj = _mesa_lookup_texture_err(ctx, texture, "glTextureBuffer");
   if (!texObj)
      return;

   if (!check_texture_buffer_target(ctx, texObj->Target, "glTextureBuffer"))
      return;

   _mesa_texture_buffer_range(ctx, texObj, internalFormat,
                              bufObj, 0, buffer ? -1 : 0, "glTextureBuffer");
}

void GLAPIENTRY
_mesa_TextureBufferRange(GLuint texture, GLenum internalFormat, GLuint buffer,
                         GLintptr offset, GLsizeiptr size)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTextureBufferRange(GL_ARB_direct_state_access "
                  "is not supported)");
      return;
   }

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                          "glTextureBufferRange");
      if (!bufObj)
         return;

      if (!check_texture_buffer_range(ctx, bufObj, offset, size,
          "glTextureBufferRange"))
         return;

   } else {

      /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
       * Textures (PDF page 254):
       *    "If buffer is zero, then any buffer object attached to the buffer
       *    texture is detached, the values offset and size are ignored and
       *    the state for offset and size for the buffer texture are reset to
       *    zero."
       */
      offset = 0;
      size = 0;
      bufObj = NULL;
   }

   /* Get the texture object by Name. */
   texObj = _mesa_lookup_texture_err(ctx, texture, "glTextureBufferRange");
   if (!texObj)
      return;

   if (!check_texture_buffer_target(ctx, texObj->Target,
       "glTextureBufferRange"))
      return;

   _mesa_texture_buffer_range(ctx, texObj, internalFormat,
                              bufObj, offset, size, "glTextureBufferRange");
}

static GLboolean
is_renderable_texture_format(struct gl_context *ctx, GLenum internalformat)
{
   /* Everything that is allowed for renderbuffers,
    * except for a base format of GL_STENCIL_INDEX.
    */
   GLenum baseFormat = _mesa_base_fbo_format(ctx, internalformat);
   return baseFormat != 0 && baseFormat != GL_STENCIL_INDEX;
}


/** GL_ARB_texture_multisample */
static GLboolean
check_multisample_target(GLuint dims, GLenum target, bool dsa)
{
   switch(target) {
   case GL_TEXTURE_2D_MULTISAMPLE:
      return dims == 2;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      return dims == 2 && !dsa;

   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return dims == 3;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return dims == 3 && !dsa;

   default:
      return GL_FALSE;
   }
}


void
_mesa_texture_image_multisample(struct gl_context *ctx, GLuint dims,
                                struct gl_texture_object *texObj,
                                GLenum target, GLsizei samples,
                                GLint internalformat, GLsizei width,
                                GLsizei height, GLsizei depth,
                                GLboolean fixedsamplelocations,
                                GLboolean immutable, const char *func)
{
   struct gl_texture_image *texImage;
   GLboolean sizeOK, dimensionsOK, samplesOK;
   mesa_format texFormat;
   GLenum sample_count_error;
   bool dsa = strstr(func, "ture") ? true : false;

   if (!(ctx->Extensions.ARB_texture_multisample
      && _mesa_is_desktop_gl(ctx))) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(unsupported)", func);
      return;
   }

   if (!check_multisample_target(dims, target, dsa)) {
      if (dsa) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(target)", func);
         return;
      }
      else {
         _mesa_error(ctx, GL_INVALID_ENUM, "%s(target)", func);
         return;
      }
   }

   /* check that the specified internalformat is color/depth/stencil-renderable;
    * refer GL3.1 spec 4.4.4
    */

   if (immutable && !_mesa_is_legal_tex_storage_format(ctx, internalformat)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
            "%s(internalformat=%s not legal for immutable-format)",
            func, _mesa_lookup_enum_by_nr(internalformat));
      return;
   }

   if (!is_renderable_texture_format(ctx, internalformat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
            "%s(internalformat=%s)",
            func, _mesa_lookup_enum_by_nr(internalformat));
      return;
   }

   sample_count_error = _mesa_check_sample_count(ctx, target,
         internalformat, samples);
   samplesOK = sample_count_error == GL_NO_ERROR;

   /* Page 254 of OpenGL 4.4 spec says:
    *   "Proxy arrays for two-dimensional multisample and two-dimensional
    *    multisample array textures are operated on in the same way when
    *    TexImage2DMultisample is called with target specified as
    *    PROXY_TEXTURE_2D_MULTISAMPLE, or TexImage3DMultisample is called
    *    with target specified as PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY.
    *    However, if samples is not supported, then no error is generated.
    */
   if (!samplesOK && !_mesa_is_proxy_texture(target)) {
      _mesa_error(ctx, sample_count_error, "%s(samples)", func);
      return;
   }

   if (immutable && (!texObj || (texObj->Name == 0))) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
            "%s(texture object 0)",
            func);
      return;
   }

   texImage = _mesa_get_tex_image(ctx, texObj, 0, 0);

   if (texImage == NULL) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s()", func);
      return;
   }

   texFormat = _mesa_choose_texture_format(ctx, texObj, target, 0,
         internalformat, GL_NONE, GL_NONE);
   assert(texFormat != MESA_FORMAT_NONE);

   dimensionsOK = _mesa_legal_texture_dimensions(ctx, target, 0,
         width, height, depth, 0);

   sizeOK = ctx->Driver.TestProxyTexImage(ctx, target, 0, texFormat,
         width, height, depth, 0);

   if (_mesa_is_proxy_texture(target)) {
      if (samplesOK && dimensionsOK && sizeOK) {
         init_teximage_fields_ms(ctx, texImage, width, height, depth, 0,
                                 internalformat, texFormat,
                                 samples, fixedsamplelocations);
      }
      else {
         /* clear all image fields */
         clear_teximage_fields(texImage);
      }
   }
   else {
      if (!dimensionsOK) {
         _mesa_error(ctx, GL_INVALID_VALUE,
               "%s(invalid width or height)", func);
         return;
      }

      if (!sizeOK) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY,
               "%s(texture too large)", func);
         return;
      }

      /* Check if texObj->Immutable is set */
      if (texObj->Immutable) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(immutable)", func);
         return;
      }

      ctx->Driver.FreeTextureImageBuffer(ctx, texImage);

      init_teximage_fields_ms(ctx, texImage, width, height, depth, 0,
                              internalformat, texFormat,
                              samples, fixedsamplelocations);

      if (width > 0 && height > 0 && depth > 0) {
         if (!ctx->Driver.AllocTextureStorage(ctx, texObj, 1,
                  width, height, depth)) {
            /* tidy up the texture image state. strictly speaking,
             * we're allowed to just leave this in whatever state we
             * like, but being tidy is good.
             */
            _mesa_init_teximage_fields(ctx, texImage,
                  0, 0, 0, 0, GL_NONE, MESA_FORMAT_NONE);
         }
      }

      texObj->Immutable |= immutable;

      if (immutable) {
         _mesa_set_texture_view_state(ctx, texObj, target, 1);
      }

      _mesa_update_fbo_texture(ctx, texObj, 0, 0);
   }
}


void GLAPIENTRY
_mesa_TexImage2DMultisample(GLenum target, GLsizei samples,
                            GLenum internalformat, GLsizei width,
                            GLsizei height, GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_texture_image_multisample(ctx, 2, texObj, target, samples,
                                   internalformat, width, height, 1,
                                   fixedsamplelocations, GL_FALSE,
                                   "glTexImage2DMultisample");
}


void GLAPIENTRY
_mesa_TexImage3DMultisample(GLenum target, GLsizei samples,
                            GLenum internalformat, GLsizei width,
                            GLsizei height, GLsizei depth,
                            GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_texture_image_multisample(ctx, 3, texObj, target, samples,
                                   internalformat, width, height, depth,
                                   fixedsamplelocations, GL_FALSE,
                                   "glTexImage3DMultisample");
}


void GLAPIENTRY
_mesa_TexStorage2DMultisample(GLenum target, GLsizei samples,
                              GLenum internalformat, GLsizei width,
                              GLsizei height, GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_texture_image_multisample(ctx, 2, texObj, target, samples,
                                   internalformat, width, height, 1,
                                   fixedsamplelocations, GL_TRUE,
                                   "glTexStorage2DMultisample");
}

void GLAPIENTRY
_mesa_TexStorage3DMultisample(GLenum target, GLsizei samples,
                              GLenum internalformat, GLsizei width,
                              GLsizei height, GLsizei depth,
                              GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   _mesa_texture_image_multisample(ctx, 3, texObj, target, samples,
                                   internalformat, width, height, depth,
                                   fixedsamplelocations, GL_TRUE,
                                   "glTexStorage3DMultisample");
}

void GLAPIENTRY
_mesa_TextureStorage2DMultisample(GLuint texture, GLsizei samples,
                                  GLenum internalformat, GLsizei width,
                                  GLsizei height,
                                  GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTextureStorage2DMultisample(GL_ARB_direct_state_access "
                  "is not supported)");
      return;
   }

   texObj = _mesa_lookup_texture_err(ctx, texture,
                                     "glTextureStorage2DMultisample");
   if (!texObj)
      return;

   _mesa_texture_image_multisample(ctx, 2, texObj, texObj->Target, samples,
                                   internalformat, width, height, 1,
                                   fixedsamplelocations, GL_TRUE,
                                   "glTextureStorage2DMultisample");
}

void GLAPIENTRY
_mesa_TextureStorage3DMultisample(GLuint texture, GLsizei samples,
                                  GLenum internalformat, GLsizei width,
                                  GLsizei height, GLsizei depth,
                                  GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   if (!ctx->Extensions.ARB_direct_state_access) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTextureStorage3DMultisample(GL_ARB_direct_state_access "
                  "is not supported)");
      return;
   }

   /* Get the texture object by Name. */
   texObj = _mesa_lookup_texture_err(ctx, texture,
                                     "glTextureStorage3DMultisample");
   if (!texObj)
      return;

   _mesa_texture_image_multisample(ctx, 3, texObj, texObj->Target, samples,
                                   internalformat, width, height, depth,
                                   fixedsamplelocations, GL_TRUE,
                                   "glTextureStorage3DMultisample");
}
