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
 * \file texparam.c
 *
 * glTexParameter-related functions
 */

#include <stdbool.h>
#include "main/glheader.h"
#include "main/blend.h"
#include "main/colormac.h"
#include "main/context.h"
#include "main/enums.h"
#include "main/formats.h"
#include "main/glformats.h"
#include "main/macros.h"
#include "main/mtypes.h"
#include "main/state.h"
#include "main/texcompress.h"
#include "main/texobj.h"
#include "main/texparam.h"
#include "main/teximage.h"
#include "main/texstate.h"
#include "program/prog_instruction.h"


/**
 * Check if a coordinate wrap mode is supported for the texture target.
 * \return GL_TRUE if legal, GL_FALSE otherwise
 */
static GLboolean
validate_texture_wrap_mode(struct gl_context * ctx, GLenum target, GLenum wrap)
{
   const struct gl_extensions * const e = & ctx->Extensions;
   const bool is_desktop_gl = _mesa_is_desktop_gl(ctx);
   bool supported;

   switch (wrap) {
   case GL_CLAMP:
      /* GL_CLAMP was removed in the core profile, and it has never existed in
       * OpenGL ES.
       */
      supported = (ctx->API == API_OPENGL_COMPAT)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_CLAMP_TO_EDGE:
      supported = true;
      break;

   case GL_CLAMP_TO_BORDER:
      supported = is_desktop_gl && e->ARB_texture_border_clamp
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_REPEAT:
   case GL_MIRRORED_REPEAT:
      supported = (target != GL_TEXTURE_RECTANGLE_NV)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_MIRROR_CLAMP_EXT:
      supported = is_desktop_gl
         && (e->ATI_texture_mirror_once || e->EXT_texture_mirror_clamp)
         && (target != GL_TEXTURE_RECTANGLE_NV)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_MIRROR_CLAMP_TO_EDGE_EXT:
      supported = is_desktop_gl
         && (e->ATI_texture_mirror_once || e->EXT_texture_mirror_clamp || e->ARB_texture_mirror_clamp_to_edge)
         && (target != GL_TEXTURE_RECTANGLE_NV)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_MIRROR_CLAMP_TO_BORDER_EXT:
      supported = is_desktop_gl && e->EXT_texture_mirror_clamp
         && (target != GL_TEXTURE_RECTANGLE_NV)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   default:
      supported = false;
      break;
   }

   if (!supported)
      _mesa_error( ctx, GL_INVALID_ENUM, "glTexParameter(param=0x%x)", wrap );

   return supported;
}


/**
 * Get current texture object for given target.
 * Return NULL if any error (and record the error).
 * Note that this is different from _mesa_get_current_tex_object() in that
 * proxy targets are not accepted.
 * Only the glGetTexLevelParameter() functions accept proxy targets.
 */
static struct gl_texture_object *
get_texobj(struct gl_context *ctx, GLenum target, GLboolean get)
{
   struct gl_texture_unit *texUnit;
   int targetIndex;

   if (ctx->Texture.CurrentUnit >= ctx->Const.MaxCombinedTextureImageUnits) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "gl%sTexParameter(current unit)", get ? "Get" : "");
      return NULL;
   }

   texUnit = _mesa_get_current_tex_unit(ctx);

   targetIndex = _mesa_tex_target_to_index(ctx, target);
   if (targetIndex < 0 || targetIndex == TEXTURE_BUFFER_INDEX) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "gl%sTexParameter(target)", get ? "Get" : "");
      return NULL;
   }
   assert(targetIndex < NUM_TEXTURE_TARGETS);

   return texUnit->CurrentTex[targetIndex];
}


/**
 * Convert GL_RED/GREEN/BLUE/ALPHA/ZERO/ONE to SWIZZLE_X/Y/Z/W/ZERO/ONE.
 * \return -1 if error.
 */
static GLint
comp_to_swizzle(GLenum comp)
{
   switch (comp) {
   case GL_RED:
      return SWIZZLE_X;
   case GL_GREEN:
      return SWIZZLE_Y;
   case GL_BLUE:
      return SWIZZLE_Z;
   case GL_ALPHA:
      return SWIZZLE_W;
   case GL_ZERO:
      return SWIZZLE_ZERO;
   case GL_ONE:
      return SWIZZLE_ONE;
   default:
      return -1;
   }
}


static void
set_swizzle_component(GLuint *swizzle, GLuint comp, GLuint swz)
{
   ASSERT(comp < 4);
   ASSERT(swz <= SWIZZLE_NIL);
   {
      GLuint mask = 0x7 << (3 * comp);
      GLuint s = (*swizzle & ~mask) | (swz << (3 * comp));
      *swizzle = s;
   }
}


/**
 * This is called just prior to changing any texture object state which
 * will not effect texture completeness.
 */
static inline void
flush(struct gl_context *ctx)
{
   FLUSH_VERTICES(ctx, _NEW_TEXTURE);
}


/**
 * This is called just prior to changing any texture object state which
 * can effect texture completeness (texture base level, max level).
 * Any pending rendering will be flushed out, we'll set the _NEW_TEXTURE
 * state flag and then mark the texture object as 'incomplete' so that any
 * per-texture derived state gets recomputed.
 */
static inline void
incomplete(struct gl_context *ctx, struct gl_texture_object *texObj)
{
   FLUSH_VERTICES(ctx, _NEW_TEXTURE);
   _mesa_dirty_texobj(ctx, texObj);
}


static GLboolean
target_allows_setting_sampler_parameters(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return GL_FALSE;

   default:
      return GL_TRUE;
   }
}


/**
 * Set an integer-valued texture parameter
 * \return GL_TRUE if legal AND the value changed, GL_FALSE otherwise
 */
static GLboolean
set_tex_parameteri(struct gl_context *ctx,
                   struct gl_texture_object *texObj,
                   GLenum pname, const GLint *params)
{
   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      if (texObj->Sampler.MinFilter == params[0])
         return GL_FALSE;
      switch (params[0]) {
      case GL_NEAREST:
      case GL_LINEAR:
         flush(ctx);
         texObj->Sampler.MinFilter = params[0];
         return GL_TRUE;
      case GL_NEAREST_MIPMAP_NEAREST:
      case GL_LINEAR_MIPMAP_NEAREST:
      case GL_NEAREST_MIPMAP_LINEAR:
      case GL_LINEAR_MIPMAP_LINEAR:
         if (texObj->Target != GL_TEXTURE_RECTANGLE_NV &&
             texObj->Target != GL_TEXTURE_EXTERNAL_OES) {
            flush(ctx);
            texObj->Sampler.MinFilter = params[0];
            return GL_TRUE;
         }
         /* fall-through */
      default:
         goto invalid_param;
      }
      return GL_FALSE;

   case GL_TEXTURE_MAG_FILTER:
      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      if (texObj->Sampler.MagFilter == params[0])
         return GL_FALSE;
      switch (params[0]) {
      case GL_NEAREST:
      case GL_LINEAR:
         flush(ctx); /* does not effect completeness */
         texObj->Sampler.MagFilter = params[0];
         return GL_TRUE;
      default:
         goto invalid_param;
      }
      return GL_FALSE;

   case GL_TEXTURE_WRAP_S:
      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      if (texObj->Sampler.WrapS == params[0])
         return GL_FALSE;
      if (validate_texture_wrap_mode(ctx, texObj->Target, params[0])) {
         flush(ctx);
         texObj->Sampler.WrapS = params[0];
         return GL_TRUE;
      }
      return GL_FALSE;

   case GL_TEXTURE_WRAP_T:
      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      if (texObj->Sampler.WrapT == params[0])
         return GL_FALSE;
      if (validate_texture_wrap_mode(ctx, texObj->Target, params[0])) {
         flush(ctx);
         texObj->Sampler.WrapT = params[0];
         return GL_TRUE;
      }
      return GL_FALSE;

   case GL_TEXTURE_WRAP_R:
      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      if (texObj->Sampler.WrapR == params[0])
         return GL_FALSE;
      if (validate_texture_wrap_mode(ctx, texObj->Target, params[0])) {
         flush(ctx);
         texObj->Sampler.WrapR = params[0];
         return GL_TRUE;
      }
      return GL_FALSE;

   case GL_TEXTURE_BASE_LEVEL:
      if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
         goto invalid_pname;

      if (texObj->BaseLevel == params[0])
         return GL_FALSE;

      if ((texObj->Target == GL_TEXTURE_2D_MULTISAMPLE ||
           texObj->Target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) && params[0] != 0)
         goto invalid_operation;

      if (params[0] < 0 ||
          (texObj->Target == GL_TEXTURE_RECTANGLE_ARB && params[0] != 0)) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glTexParameter(param=%d)", params[0]);
         return GL_FALSE;
      }
      incomplete(ctx, texObj);

      /** See note about ARB_texture_storage below */
      if (texObj->Immutable)
         texObj->BaseLevel = MIN2(texObj->ImmutableLevels - 1, params[0]);
      else
         texObj->BaseLevel = params[0];

      return GL_TRUE;

   case GL_TEXTURE_MAX_LEVEL:
      if (texObj->MaxLevel == params[0])
         return GL_FALSE;

      if (params[0] < 0 ||
          (texObj->Target == GL_TEXTURE_RECTANGLE_ARB && params[0] > 0)) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glTexParameter(param=%d)", params[0]);
         return GL_FALSE;
      }
      incomplete(ctx, texObj);

      /** From ARB_texture_storage:
       * However, if TEXTURE_IMMUTABLE_FORMAT is TRUE, then level_base is
       * clamped to the range [0, <levels> - 1] and level_max is then clamped to
       * the range [level_base, <levels> - 1], where <levels> is the parameter
       * passed the call to TexStorage* for the texture object.
       */
      if (texObj->Immutable)
          texObj->MaxLevel = CLAMP(params[0], texObj->BaseLevel,
                                   texObj->ImmutableLevels - 1);
      else
         texObj->MaxLevel = params[0];

      return GL_TRUE;

   case GL_GENERATE_MIPMAP_SGIS:
      if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
         goto invalid_pname;

      if (params[0] && texObj->Target == GL_TEXTURE_EXTERNAL_OES)
         goto invalid_param;
      if (texObj->GenerateMipmap != params[0]) {
         /* no flush() */
	 texObj->GenerateMipmap = params[0] ? GL_TRUE : GL_FALSE;
	 return GL_TRUE;
      }
      return GL_FALSE;

   case GL_TEXTURE_COMPARE_MODE_ARB:
      if ((_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_shadow)
          || _mesa_is_gles3(ctx)) {

         if (!target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_operation;

         if (texObj->Sampler.CompareMode == params[0])
            return GL_FALSE;
         if (params[0] == GL_NONE ||
             params[0] == GL_COMPARE_R_TO_TEXTURE_ARB) {
            flush(ctx);
            texObj->Sampler.CompareMode = params[0];
            return GL_TRUE;
         }
         goto invalid_param;
      }
      goto invalid_pname;

   case GL_TEXTURE_COMPARE_FUNC_ARB:
      if ((_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_shadow)
          || _mesa_is_gles3(ctx)) {

         if (!target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_operation;

         if (texObj->Sampler.CompareFunc == params[0])
            return GL_FALSE;
         switch (params[0]) {
         case GL_LEQUAL:
         case GL_GEQUAL:
         case GL_EQUAL:
         case GL_NOTEQUAL:
         case GL_LESS:
         case GL_GREATER:
         case GL_ALWAYS:
         case GL_NEVER:
            flush(ctx);
            texObj->Sampler.CompareFunc = params[0];
            return GL_TRUE;
         default:
            goto invalid_param;
         }
      }
      goto invalid_pname;

   case GL_DEPTH_TEXTURE_MODE_ARB:
      /* GL_DEPTH_TEXTURE_MODE_ARB is removed in core-profile and it has never
       * existed in OpenGL ES.
       */
      if (ctx->API == API_OPENGL_COMPAT && ctx->Extensions.ARB_depth_texture) {
         if (texObj->DepthMode == params[0])
            return GL_FALSE;
         if (params[0] == GL_LUMINANCE ||
             params[0] == GL_INTENSITY ||
             params[0] == GL_ALPHA ||
             (ctx->Extensions.ARB_texture_rg && params[0] == GL_RED)) {
            flush(ctx);
            texObj->DepthMode = params[0];
            return GL_TRUE;
         }
         goto invalid_param;
      }
      goto invalid_pname;

   case GL_DEPTH_STENCIL_TEXTURE_MODE:
      if (_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_stencil_texturing) {
         bool stencil = params[0] == GL_STENCIL_INDEX;
         if (!stencil && params[0] != GL_DEPTH_COMPONENT)
            goto invalid_param;

         if (texObj->StencilSampling == stencil)
            return GL_FALSE;

         texObj->StencilSampling = stencil;
         return GL_TRUE;
      }
      goto invalid_pname;

   case GL_TEXTURE_CROP_RECT_OES:
      if (ctx->API != API_OPENGLES || !ctx->Extensions.OES_draw_texture)
         goto invalid_pname;

      texObj->CropRect[0] = params[0];
      texObj->CropRect[1] = params[1];
      texObj->CropRect[2] = params[2];
      texObj->CropRect[3] = params[3];
      return GL_TRUE;

   case GL_TEXTURE_SWIZZLE_R_EXT:
   case GL_TEXTURE_SWIZZLE_G_EXT:
   case GL_TEXTURE_SWIZZLE_B_EXT:
   case GL_TEXTURE_SWIZZLE_A_EXT:
      if ((_mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_swizzle)
          || _mesa_is_gles3(ctx)) {
         const GLuint comp = pname - GL_TEXTURE_SWIZZLE_R_EXT;
         const GLint swz = comp_to_swizzle(params[0]);
         if (swz < 0) {
            _mesa_error(ctx, GL_INVALID_ENUM,
                        "glTexParameter(swizzle 0x%x)", params[0]);
            return GL_FALSE;
         }
         ASSERT(comp < 4);

         flush(ctx);
         texObj->Swizzle[comp] = params[0];
         set_swizzle_component(&texObj->_Swizzle, comp, swz);
         return GL_TRUE;
      }
      goto invalid_pname;

   case GL_TEXTURE_SWIZZLE_RGBA_EXT:
      if ((_mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_swizzle)
          || _mesa_is_gles3(ctx)) {
         GLuint comp;
         flush(ctx);
         for (comp = 0; comp < 4; comp++) {
            const GLint swz = comp_to_swizzle(params[comp]);
            if (swz >= 0) {
               texObj->Swizzle[comp] = params[comp];
               set_swizzle_component(&texObj->_Swizzle, comp, swz);
            }
            else {
               _mesa_error(ctx, GL_INVALID_ENUM,
                           "glTexParameter(swizzle 0x%x)", params[comp]);
               return GL_FALSE;
            }
         }
         return GL_TRUE;
      }
      goto invalid_pname;

   case GL_TEXTURE_SRGB_DECODE_EXT:
      if (_mesa_is_desktop_gl(ctx)
          && ctx->Extensions.EXT_texture_sRGB_decode) {
         GLenum decode = params[0];

         if (!target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_operation;

	 if (decode == GL_DECODE_EXT || decode == GL_SKIP_DECODE_EXT) {
	    if (texObj->Sampler.sRGBDecode != decode) {
	       flush(ctx);
	       texObj->Sampler.sRGBDecode = decode;
	    }
	    return GL_TRUE;
	 }
      }
      goto invalid_pname;

   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      if (_mesa_is_desktop_gl(ctx)
          && ctx->Extensions.AMD_seamless_cubemap_per_texture) {
         GLenum param = params[0];

         if (!target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_operation;

         if (param != GL_TRUE && param != GL_FALSE) {
            goto invalid_param;
         }
         if (param != texObj->Sampler.CubeMapSeamless) {
            flush(ctx);
            texObj->Sampler.CubeMapSeamless = param;
         }
         return GL_TRUE;
      }
      goto invalid_pname;

   default:
      goto invalid_pname;
   }

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "glTexParameter(pname=%s)",
               _mesa_lookup_enum_by_nr(pname));
   return GL_FALSE;

invalid_param:
   _mesa_error(ctx, GL_INVALID_ENUM, "glTexParameter(param=%s)",
               _mesa_lookup_enum_by_nr(params[0]));
   return GL_FALSE;

invalid_operation:
   _mesa_error(ctx, GL_INVALID_OPERATION, "glTexParameter(pname=%s)",
               _mesa_lookup_enum_by_nr(pname));
   return GL_FALSE;
}


/**
 * Set a float-valued texture parameter
 * \return GL_TRUE if legal AND the value changed, GL_FALSE otherwise
 */
static GLboolean
set_tex_parameterf(struct gl_context *ctx,
                   struct gl_texture_object *texObj,
                   GLenum pname, const GLfloat *params)
{
   switch (pname) {
   case GL_TEXTURE_MIN_LOD:
      if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
         goto invalid_pname;

      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      if (texObj->Sampler.MinLod == params[0])
         return GL_FALSE;
      flush(ctx);
      texObj->Sampler.MinLod = params[0];
      return GL_TRUE;

   case GL_TEXTURE_MAX_LOD:
      if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
         goto invalid_pname;

      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      if (texObj->Sampler.MaxLod == params[0])
         return GL_FALSE;
      flush(ctx);
      texObj->Sampler.MaxLod = params[0];
      return GL_TRUE;

   case GL_TEXTURE_PRIORITY:
      if (ctx->API != API_OPENGL_COMPAT)
         goto invalid_pname;

      flush(ctx);
      texObj->Priority = CLAMP(params[0], 0.0F, 1.0F);
      return GL_TRUE;

   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      if (ctx->Extensions.EXT_texture_filter_anisotropic) {
         if (!target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_operation;

         if (texObj->Sampler.MaxAnisotropy == params[0])
            return GL_FALSE;
         if (params[0] < 1.0) {
            _mesa_error(ctx, GL_INVALID_VALUE, "glTexParameter(param)" );
            return GL_FALSE;
         }
         flush(ctx);
         /* clamp to max, that's what NVIDIA does */
         texObj->Sampler.MaxAnisotropy = MIN2(params[0],
                                      ctx->Const.MaxTextureMaxAnisotropy);
         return GL_TRUE;
      }
      else {
         static GLuint count = 0;
         if (count++ < 10)
            goto invalid_pname;
      }
      return GL_FALSE;

   case GL_TEXTURE_LOD_BIAS:
      /* NOTE: this is really part of OpenGL 1.4, not EXT_texture_lod_bias. */
      if (_mesa_is_gles(ctx))
         goto invalid_pname;

      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      if (texObj->Sampler.LodBias != params[0]) {
	 flush(ctx);
	 texObj->Sampler.LodBias = params[0];
	 return GL_TRUE;
      }
      break;

   case GL_TEXTURE_BORDER_COLOR:
      if (!_mesa_is_desktop_gl(ctx))
         goto invalid_pname;

      if (!target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_operation;

      flush(ctx);
      /* ARB_texture_float disables clamping */
      if (ctx->Extensions.ARB_texture_float) {
         texObj->Sampler.BorderColor.f[RCOMP] = params[0];
         texObj->Sampler.BorderColor.f[GCOMP] = params[1];
         texObj->Sampler.BorderColor.f[BCOMP] = params[2];
         texObj->Sampler.BorderColor.f[ACOMP] = params[3];
      } else {
         texObj->Sampler.BorderColor.f[RCOMP] = CLAMP(params[0], 0.0F, 1.0F);
         texObj->Sampler.BorderColor.f[GCOMP] = CLAMP(params[1], 0.0F, 1.0F);
         texObj->Sampler.BorderColor.f[BCOMP] = CLAMP(params[2], 0.0F, 1.0F);
         texObj->Sampler.BorderColor.f[ACOMP] = CLAMP(params[3], 0.0F, 1.0F);
      }
      return GL_TRUE;

   default:
      goto invalid_pname;
   }
   return GL_FALSE;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "glTexParameter(pname=%s)",
               _mesa_lookup_enum_by_nr(pname));
   return GL_FALSE;

invalid_operation:
   _mesa_error(ctx, GL_INVALID_OPERATION, "glTexParameter(pname=%s)",
               _mesa_lookup_enum_by_nr(pname));
   return GL_FALSE;
}


void GLAPIENTRY
_mesa_TexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   GLboolean need_update;
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj(ctx, target, GL_FALSE);
   if (!texObj)
      return;

   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
   case GL_TEXTURE_WRAP_R:
   case GL_TEXTURE_BASE_LEVEL:
   case GL_TEXTURE_MAX_LEVEL:
   case GL_GENERATE_MIPMAP_SGIS:
   case GL_TEXTURE_COMPARE_MODE_ARB:
   case GL_TEXTURE_COMPARE_FUNC_ARB:
   case GL_DEPTH_TEXTURE_MODE_ARB:
   case GL_DEPTH_STENCIL_TEXTURE_MODE:
   case GL_TEXTURE_SRGB_DECODE_EXT:
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
   case GL_TEXTURE_SWIZZLE_R_EXT:
   case GL_TEXTURE_SWIZZLE_G_EXT:
   case GL_TEXTURE_SWIZZLE_B_EXT:
   case GL_TEXTURE_SWIZZLE_A_EXT:
      {
         GLint p[4];
         p[0] = (param > 0) ?
                ((param > INT_MAX) ? INT_MAX : (GLint) (param + 0.5)) :
                ((param < INT_MIN) ? INT_MIN : (GLint) (param - 0.5));

         p[1] = p[2] = p[3] = 0;
         need_update = set_tex_parameteri(ctx, texObj, pname, p);
      }
      break;
   default:
      {
         /* this will generate an error if pname is illegal */
         GLfloat p[4];
         p[0] = param;
         p[1] = p[2] = p[3] = 0.0F;
         need_update = set_tex_parameterf(ctx, texObj, pname, p);
      }
   }

   if (ctx->Driver.TexParameter && need_update) {
      ctx->Driver.TexParameter(ctx, texObj, pname, &param);
   }
}


void GLAPIENTRY
_mesa_TexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
   GLboolean need_update;
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj(ctx, target, GL_FALSE);
   if (!texObj)
      return;

   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
   case GL_TEXTURE_WRAP_R:
   case GL_TEXTURE_BASE_LEVEL:
   case GL_TEXTURE_MAX_LEVEL:
   case GL_GENERATE_MIPMAP_SGIS:
   case GL_TEXTURE_COMPARE_MODE_ARB:
   case GL_TEXTURE_COMPARE_FUNC_ARB:
   case GL_DEPTH_TEXTURE_MODE_ARB:
   case GL_DEPTH_STENCIL_TEXTURE_MODE:
   case GL_TEXTURE_SRGB_DECODE_EXT:
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      {
         /* convert float param to int */
         GLint p[4];
         p[0] = (GLint) params[0];
         p[1] = p[2] = p[3] = 0;
         need_update = set_tex_parameteri(ctx, texObj, pname, p);
      }
      break;
   case GL_TEXTURE_CROP_RECT_OES:
      {
         /* convert float params to int */
         GLint iparams[4];
         iparams[0] = (GLint) params[0];
         iparams[1] = (GLint) params[1];
         iparams[2] = (GLint) params[2];
         iparams[3] = (GLint) params[3];
         need_update = set_tex_parameteri(ctx, texObj, pname, iparams);
      }
      break;
   case GL_TEXTURE_SWIZZLE_R_EXT:
   case GL_TEXTURE_SWIZZLE_G_EXT:
   case GL_TEXTURE_SWIZZLE_B_EXT:
   case GL_TEXTURE_SWIZZLE_A_EXT:
   case GL_TEXTURE_SWIZZLE_RGBA_EXT:
      {
         GLint p[4] = {0, 0, 0, 0};
         p[0] = (GLint) params[0];
         if (pname == GL_TEXTURE_SWIZZLE_RGBA_EXT) {
            p[1] = (GLint) params[1];
            p[2] = (GLint) params[2];
            p[3] = (GLint) params[3];
         }
         need_update = set_tex_parameteri(ctx, texObj, pname, p);
      }
      break;
   default:
      /* this will generate an error if pname is illegal */
      need_update = set_tex_parameterf(ctx, texObj, pname, params);
   }

   if (ctx->Driver.TexParameter && need_update) {
      ctx->Driver.TexParameter(ctx, texObj, pname, params);
   }
}


void GLAPIENTRY
_mesa_TexParameteri(GLenum target, GLenum pname, GLint param)
{
   GLboolean need_update;
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj(ctx, target, GL_FALSE);
   if (!texObj)
      return;

   switch (pname) {
   case GL_TEXTURE_MIN_LOD:
   case GL_TEXTURE_MAX_LOD:
   case GL_TEXTURE_PRIORITY:
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
   case GL_TEXTURE_LOD_BIAS:
   case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
      {
         GLfloat fparam[4];
         fparam[0] = (GLfloat) param;
         fparam[1] = fparam[2] = fparam[3] = 0.0F;
         /* convert int param to float */
         need_update = set_tex_parameterf(ctx, texObj, pname, fparam);
      }
      break;
   default:
      /* this will generate an error if pname is illegal */
      {
         GLint iparam[4];
         iparam[0] = param;
         iparam[1] = iparam[2] = iparam[3] = 0;
         need_update = set_tex_parameteri(ctx, texObj, pname, iparam);
      }
   }

   if (ctx->Driver.TexParameter && need_update) {
      GLfloat fparam = (GLfloat) param;
      ctx->Driver.TexParameter(ctx, texObj, pname, &fparam);
   }
}


void GLAPIENTRY
_mesa_TexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
   GLboolean need_update;
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj(ctx, target, GL_FALSE);
   if (!texObj)
      return;

   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      {
         /* convert int params to float */
         GLfloat fparams[4];
         fparams[0] = INT_TO_FLOAT(params[0]);
         fparams[1] = INT_TO_FLOAT(params[1]);
         fparams[2] = INT_TO_FLOAT(params[2]);
         fparams[3] = INT_TO_FLOAT(params[3]);
         need_update = set_tex_parameterf(ctx, texObj, pname, fparams);
      }
      break;
   case GL_TEXTURE_MIN_LOD:
   case GL_TEXTURE_MAX_LOD:
   case GL_TEXTURE_PRIORITY:
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
   case GL_TEXTURE_LOD_BIAS:
   case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
      {
         /* convert int param to float */
         GLfloat fparams[4];
         fparams[0] = (GLfloat) params[0];
         fparams[1] = fparams[2] = fparams[3] = 0.0F;
         need_update = set_tex_parameterf(ctx, texObj, pname, fparams);
      }
      break;
   default:
      /* this will generate an error if pname is illegal */
      need_update = set_tex_parameteri(ctx, texObj, pname, params);
   }

   if (ctx->Driver.TexParameter && need_update) {
      GLfloat fparams[4];
      fparams[0] = INT_TO_FLOAT(params[0]);
      if (pname == GL_TEXTURE_BORDER_COLOR ||
          pname == GL_TEXTURE_CROP_RECT_OES) {
         fparams[1] = INT_TO_FLOAT(params[1]);
         fparams[2] = INT_TO_FLOAT(params[2]);
         fparams[3] = INT_TO_FLOAT(params[3]);
      }
      ctx->Driver.TexParameter(ctx, texObj, pname, fparams);
   }
}


/**
 * Set tex parameter to integer value(s).  Primarily intended to set
 * integer-valued texture border color (for integer-valued textures).
 * New in GL 3.0.
 */
void GLAPIENTRY
_mesa_TexParameterIiv(GLenum target, GLenum pname, const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj(ctx, target, GL_FALSE);
   if (!texObj)
      return;

   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      FLUSH_VERTICES(ctx, _NEW_TEXTURE);
      /* set the integer-valued border color */
      COPY_4V(texObj->Sampler.BorderColor.i, params);
      break;
   default:
      _mesa_TexParameteriv(target, pname, params);
      break;
   }
   /* XXX no driver hook for TexParameterIiv() yet */
}


/**
 * Set tex parameter to unsigned integer value(s).  Primarily intended to set
 * uint-valued texture border color (for integer-valued textures).
 * New in GL 3.0
 */
void GLAPIENTRY
_mesa_TexParameterIuiv(GLenum target, GLenum pname, const GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj(ctx, target, GL_FALSE);
   if (!texObj)
      return;

   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      FLUSH_VERTICES(ctx, _NEW_TEXTURE);
      /* set the unsigned integer-valued border color */
      COPY_4V(texObj->Sampler.BorderColor.ui, params);
      break;
   default:
      _mesa_TexParameteriv(target, pname, (const GLint *) params);
      break;
   }
   /* XXX no driver hook for TexParameterIuiv() yet */
}


static GLboolean
legal_get_tex_level_parameter_target(struct gl_context *ctx, GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      return GL_TRUE;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
      return ctx->Extensions.ARB_texture_cube_map;
   case GL_TEXTURE_CUBE_MAP_ARRAY_ARB:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY_ARB:
      return ctx->Extensions.ARB_texture_cube_map_array;
   case GL_TEXTURE_RECTANGLE_NV:
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      return ctx->Extensions.NV_texture_rectangle;
   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
   case GL_TEXTURE_2D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
      return ctx->Extensions.EXT_texture_array;
   case GL_TEXTURE_BUFFER:
      /* GetTexLevelParameter accepts GL_TEXTURE_BUFFER in GL 3.1+ contexts,
       * but not in earlier versions that expose ARB_texture_buffer_object.
       *
       * From the ARB_texture_buffer_object spec:
       * "(7) Do buffer textures support texture parameters (TexParameter) or
       *      queries (GetTexParameter, GetTexLevelParameter, GetTexImage)?
       *
       *    RESOLVED:  No. [...] Note that the spec edits above don't add
       *    explicit error language for any of these cases.  That is because
       *    each of the functions enumerate the set of valid <target>
       *    parameters.  Not editing the spec to allow TEXTURE_BUFFER_ARB in
       *    these cases means that target is not legal, and an INVALID_ENUM
       *    error should be generated."
       *
       * From the OpenGL 3.1 spec:
       * "target may also be TEXTURE_BUFFER, indicating the texture buffer."
       */
      return ctx->API == API_OPENGL_CORE && ctx->Version >= 31;
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return ctx->Extensions.ARB_texture_multisample;
   default:
      return GL_FALSE;
   }
}


static void
get_tex_level_parameter_image(struct gl_context *ctx,
                              const struct gl_texture_object *texObj,
                              GLenum target, GLint level,
                              GLenum pname, GLint *params)
{
   const struct gl_texture_image *img = NULL;
   struct gl_texture_image dummy_image;
   mesa_format texFormat;

   img = _mesa_select_tex_image(ctx, texObj, target, level);
   if (!img || img->TexFormat == MESA_FORMAT_NONE) {
      /* In case of undefined texture image return the default values.
       *
       * From OpenGL 4.0 spec, page 398:
       *    "The initial internal format of a texel array is RGBA
       *     instead of 1. TEXTURE_COMPONENTS is deprecated; always
       *     use TEXTURE_INTERNAL_FORMAT."
       */
      memset(&dummy_image, 0, sizeof(dummy_image));
      dummy_image.TexFormat = MESA_FORMAT_NONE;
      dummy_image.InternalFormat = GL_RGBA;
      dummy_image._BaseFormat = GL_NONE;

      img = &dummy_image;
   }

   texFormat = img->TexFormat;

   switch (pname) {
      case GL_TEXTURE_WIDTH:
         *params = img->Width;
         break;
      case GL_TEXTURE_HEIGHT:
         *params = img->Height;
         break;
      case GL_TEXTURE_DEPTH:
         *params = img->Depth;
         break;
      case GL_TEXTURE_INTERNAL_FORMAT:
         if (_mesa_is_format_compressed(texFormat)) {
            /* need to return the actual compressed format */
            *params = _mesa_compressed_format_to_glenum(ctx, texFormat);
         }
         else {
	    /* If the true internal format is not compressed but the user
	     * requested a generic compressed format, we have to return the
	     * generic base format that matches.
	     *
	     * From page 119 (page 129 of the PDF) of the OpenGL 1.3 spec:
	     *
	     *     "If no specific compressed format is available,
	     *     internalformat is instead replaced by the corresponding base
	     *     internal format."
	     *
	     * Otherwise just return the user's requested internal format
	     */
	    const GLenum f =
	       _mesa_gl_compressed_format_base_format(img->InternalFormat);

	    *params = (f != 0) ? f : img->InternalFormat;
	 }
         break;
      case GL_TEXTURE_BORDER:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = img->Border;
         break;
      case GL_TEXTURE_RED_SIZE:
      case GL_TEXTURE_GREEN_SIZE:
      case GL_TEXTURE_BLUE_SIZE:
      case GL_TEXTURE_ALPHA_SIZE:
         if (_mesa_base_format_has_channel(img->_BaseFormat, pname))
            *params = _mesa_get_format_bits(texFormat, pname);
         else
            *params = 0;
         break;
      case GL_TEXTURE_INTENSITY_SIZE:
      case GL_TEXTURE_LUMINANCE_SIZE:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         if (_mesa_base_format_has_channel(img->_BaseFormat, pname)) {
            *params = _mesa_get_format_bits(texFormat, pname);
            if (*params == 0) {
               /* intensity or luminance is probably stored as RGB[A] */
               *params = MIN2(_mesa_get_format_bits(texFormat,
                                                    GL_TEXTURE_RED_SIZE),
                              _mesa_get_format_bits(texFormat,
                                                    GL_TEXTURE_GREEN_SIZE));
            }
         }
         else {
            *params = 0;
         }
         break;
      case GL_TEXTURE_DEPTH_SIZE_ARB:
         if (!ctx->Extensions.ARB_depth_texture)
            goto invalid_pname;
         *params = _mesa_get_format_bits(texFormat, pname);
         break;
      case GL_TEXTURE_STENCIL_SIZE:
         *params = _mesa_get_format_bits(texFormat, pname);
         break;
      case GL_TEXTURE_SHARED_SIZE:
         if (ctx->Version < 30 &&
             !ctx->Extensions.EXT_texture_shared_exponent)
            goto invalid_pname;
         *params = texFormat == MESA_FORMAT_R9G9B9E5_FLOAT ? 5 : 0;
         break;

      /* GL_ARB_texture_compression */
      case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
	 if (_mesa_is_format_compressed(texFormat) &&
             !_mesa_is_proxy_texture(target)) {
            *params = _mesa_format_image_size(texFormat, img->Width,
                                              img->Height, img->Depth);
	 }
	 else {
	    _mesa_error(ctx, GL_INVALID_OPERATION,
			"glGetTexLevelParameter[if]v(pname)");
	 }
         break;
      case GL_TEXTURE_COMPRESSED:
         *params = (GLint) _mesa_is_format_compressed(texFormat);
         break;

      /* GL_ARB_texture_float */
      case GL_TEXTURE_LUMINANCE_TYPE_ARB:
      case GL_TEXTURE_INTENSITY_TYPE_ARB:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         /* FALLTHROUGH */
      case GL_TEXTURE_RED_TYPE_ARB:
      case GL_TEXTURE_GREEN_TYPE_ARB:
      case GL_TEXTURE_BLUE_TYPE_ARB:
      case GL_TEXTURE_ALPHA_TYPE_ARB:
      case GL_TEXTURE_DEPTH_TYPE_ARB:
         if (!ctx->Extensions.ARB_texture_float)
            goto invalid_pname;
	 if (_mesa_base_format_has_channel(img->_BaseFormat, pname))
	    *params = _mesa_get_format_datatype(texFormat);
	 else
	    *params = GL_NONE;
         break;

      /* GL_ARB_texture_multisample */
      case GL_TEXTURE_SAMPLES:
         if (!ctx->Extensions.ARB_texture_multisample)
            goto invalid_pname;
         *params = img->NumSamples;
         break;

      case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
         if (!ctx->Extensions.ARB_texture_multisample)
            goto invalid_pname;
         *params = img->FixedSampleLocations;
         break;

      default:
         goto invalid_pname;
   }

   /* no error if we get here */
   return;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM,
               "glGetTexLevelParameter[if]v(pname=%s)",
               _mesa_lookup_enum_by_nr(pname));
}


static void
get_tex_level_parameter_buffer(struct gl_context *ctx,
                               const struct gl_texture_object *texObj,
                               GLenum pname, GLint *params)
{
   const struct gl_buffer_object *bo = texObj->BufferObject;
   mesa_format texFormat = texObj->_BufferObjectFormat;
   GLenum internalFormat = texObj->BufferObjectFormat;
   GLenum baseFormat = _mesa_get_format_base_format(texFormat);

   if (!bo) {
      /* undefined texture buffer object */
      *params = pname == GL_TEXTURE_COMPONENTS ? 1 : 0;
      return;
   }

   switch (pname) {
      case GL_TEXTURE_BUFFER_DATA_STORE_BINDING:
         *params = bo->Name;
         break;
      case GL_TEXTURE_WIDTH:
         *params = bo->Size;
         break;
      case GL_TEXTURE_HEIGHT:
      case GL_TEXTURE_DEPTH:
      case GL_TEXTURE_BORDER:
      case GL_TEXTURE_SHARED_SIZE:
      case GL_TEXTURE_COMPRESSED:
         *params = 0;
         break;
      case GL_TEXTURE_INTERNAL_FORMAT:
         *params = internalFormat;
         break;
      case GL_TEXTURE_RED_SIZE:
      case GL_TEXTURE_GREEN_SIZE:
      case GL_TEXTURE_BLUE_SIZE:
      case GL_TEXTURE_ALPHA_SIZE:
         if (_mesa_base_format_has_channel(baseFormat, pname))
            *params = _mesa_get_format_bits(texFormat, pname);
         else
            *params = 0;
         break;
      case GL_TEXTURE_INTENSITY_SIZE:
      case GL_TEXTURE_LUMINANCE_SIZE:
         if (_mesa_base_format_has_channel(baseFormat, pname)) {
            *params = _mesa_get_format_bits(texFormat, pname);
            if (*params == 0) {
               /* intensity or luminance is probably stored as RGB[A] */
               *params = MIN2(_mesa_get_format_bits(texFormat,
                                                    GL_TEXTURE_RED_SIZE),
                              _mesa_get_format_bits(texFormat,
                                                    GL_TEXTURE_GREEN_SIZE));
            }
         } else {
            *params = 0;
         }
         break;
      case GL_TEXTURE_DEPTH_SIZE_ARB:
      case GL_TEXTURE_STENCIL_SIZE_EXT:
         *params = _mesa_get_format_bits(texFormat, pname);
         break;

      /* GL_ARB_texture_buffer_range */
      case GL_TEXTURE_BUFFER_OFFSET:
         if (!ctx->Extensions.ARB_texture_buffer_range)
            goto invalid_pname;
         *params = texObj->BufferOffset;
         break;
      case GL_TEXTURE_BUFFER_SIZE:
         if (!ctx->Extensions.ARB_texture_buffer_range)
            goto invalid_pname;
         *params = (texObj->BufferSize == -1) ? bo->Size : texObj->BufferSize;
         break;

      /* GL_ARB_texture_compression */
      case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
         /* Always illegal for GL_TEXTURE_BUFFER */
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetTexLevelParameter[if]v(pname)");
         break;

      /* GL_ARB_texture_float */
      case GL_TEXTURE_RED_TYPE_ARB:
      case GL_TEXTURE_GREEN_TYPE_ARB:
      case GL_TEXTURE_BLUE_TYPE_ARB:
      case GL_TEXTURE_ALPHA_TYPE_ARB:
      case GL_TEXTURE_LUMINANCE_TYPE_ARB:
      case GL_TEXTURE_INTENSITY_TYPE_ARB:
      case GL_TEXTURE_DEPTH_TYPE_ARB:
         if (!ctx->Extensions.ARB_texture_float)
            goto invalid_pname;
         if (_mesa_base_format_has_channel(baseFormat, pname))
            *params = _mesa_get_format_datatype(texFormat);
         else
            *params = GL_NONE;
         break;

      default:
         goto invalid_pname;
   }

   /* no error if we get here */
   return;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM,
               "glGetTexLevelParameter[if]v(pname=%s)",
               _mesa_lookup_enum_by_nr(pname));
}


void GLAPIENTRY
_mesa_GetTexLevelParameterfv( GLenum target, GLint level,
                              GLenum pname, GLfloat *params )
{
   GLint iparam;
   _mesa_GetTexLevelParameteriv( target, level, pname, &iparam );
   *params = (GLfloat) iparam;
}


void GLAPIENTRY
_mesa_GetTexLevelParameteriv( GLenum target, GLint level,
                              GLenum pname, GLint *params )
{
   struct gl_texture_object *texObj;
   GLint maxLevels;
   GET_CURRENT_CONTEXT(ctx);

   if (ctx->Texture.CurrentUnit >= ctx->Const.MaxCombinedTextureImageUnits) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetTexLevelParameteriv(current unit)");
      return;
   }

   if (!legal_get_tex_level_parameter_target(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetTexLevelParameter[if]v(target=0x%x)", target);
      return;
   }

   maxLevels = _mesa_max_texture_levels(ctx, target);
   assert(maxLevels != 0);

   if (level < 0 || level >= maxLevels) {
      _mesa_error( ctx, GL_INVALID_VALUE, "glGetTexLevelParameter[if]v" );
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);

   if (target == GL_TEXTURE_BUFFER)
      get_tex_level_parameter_buffer(ctx, texObj, pname, params);
   else
      get_tex_level_parameter_image(ctx, texObj, target, level, pname, params);
}


void GLAPIENTRY
_mesa_GetTexParameterfv( GLenum target, GLenum pname, GLfloat *params )
{
   struct gl_texture_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = get_texobj(ctx, target, GL_TRUE);
   if (!obj)
      return;

   _mesa_lock_context_textures(ctx);
   switch (pname) {
      case GL_TEXTURE_MAG_FILTER:
	 *params = ENUM_TO_FLOAT(obj->Sampler.MagFilter);
	 break;
      case GL_TEXTURE_MIN_FILTER:
         *params = ENUM_TO_FLOAT(obj->Sampler.MinFilter);
         break;
      case GL_TEXTURE_WRAP_S:
         *params = ENUM_TO_FLOAT(obj->Sampler.WrapS);
         break;
      case GL_TEXTURE_WRAP_T:
         *params = ENUM_TO_FLOAT(obj->Sampler.WrapT);
         break;
      case GL_TEXTURE_WRAP_R:
         *params = ENUM_TO_FLOAT(obj->Sampler.WrapR);
         break;
      case GL_TEXTURE_BORDER_COLOR:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_pname;

         if (ctx->NewState & (_NEW_BUFFERS | _NEW_FRAG_CLAMP))
            _mesa_update_state_locked(ctx);
         if (_mesa_get_clamp_fragment_color(ctx)) {
            params[0] = CLAMP(obj->Sampler.BorderColor.f[0], 0.0F, 1.0F);
            params[1] = CLAMP(obj->Sampler.BorderColor.f[1], 0.0F, 1.0F);
            params[2] = CLAMP(obj->Sampler.BorderColor.f[2], 0.0F, 1.0F);
            params[3] = CLAMP(obj->Sampler.BorderColor.f[3], 0.0F, 1.0F);
         }
         else {
            params[0] = obj->Sampler.BorderColor.f[0];
            params[1] = obj->Sampler.BorderColor.f[1];
            params[2] = obj->Sampler.BorderColor.f[2];
            params[3] = obj->Sampler.BorderColor.f[3];
         }
         break;
      case GL_TEXTURE_RESIDENT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;

         *params = 1.0F;
         break;
      case GL_TEXTURE_PRIORITY:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;

         *params = obj->Priority;
         break;
      case GL_TEXTURE_MIN_LOD:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = obj->Sampler.MinLod;
         break;
      case GL_TEXTURE_MAX_LOD:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = obj->Sampler.MaxLod;
         break;
      case GL_TEXTURE_BASE_LEVEL:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = (GLfloat) obj->BaseLevel;
         break;
      case GL_TEXTURE_MAX_LEVEL:
         *params = (GLfloat) obj->MaxLevel;
         break;
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
         if (!ctx->Extensions.EXT_texture_filter_anisotropic)
            goto invalid_pname;
         *params = obj->Sampler.MaxAnisotropy;
         break;
      case GL_GENERATE_MIPMAP_SGIS:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_pname;

	 *params = (GLfloat) obj->GenerateMipmap;
         break;
      case GL_TEXTURE_COMPARE_MODE_ARB:
         if ((!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_shadow)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.CompareMode;
         break;
      case GL_TEXTURE_COMPARE_FUNC_ARB:
         if ((!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_shadow)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.CompareFunc;
         break;
      case GL_DEPTH_TEXTURE_MODE_ARB:
         /* GL_DEPTH_TEXTURE_MODE_ARB is removed in core-profile and it has
          * never existed in OpenGL ES.
          */
         if (ctx->API != API_OPENGL_COMPAT || !ctx->Extensions.ARB_depth_texture)
            goto invalid_pname;
         *params = (GLfloat) obj->DepthMode;
         break;
      case GL_DEPTH_STENCIL_TEXTURE_MODE:
         if (!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_stencil_texturing)
            goto invalid_pname;
         *params = (GLfloat)
            (obj->StencilSampling ? GL_STENCIL_INDEX : GL_DEPTH_COMPONENT);
         break;
      case GL_TEXTURE_LOD_BIAS:
         if (_mesa_is_gles(ctx))
            goto invalid_pname;

         *params = obj->Sampler.LodBias;
         break;
      case GL_TEXTURE_CROP_RECT_OES:
         if (ctx->API != API_OPENGLES || !ctx->Extensions.OES_draw_texture)
            goto invalid_pname;

         params[0] = (GLfloat) obj->CropRect[0];
         params[1] = (GLfloat) obj->CropRect[1];
         params[2] = (GLfloat) obj->CropRect[2];
         params[3] = (GLfloat) obj->CropRect[3];
         break;

      case GL_TEXTURE_SWIZZLE_R_EXT:
      case GL_TEXTURE_SWIZZLE_G_EXT:
      case GL_TEXTURE_SWIZZLE_B_EXT:
      case GL_TEXTURE_SWIZZLE_A_EXT:
         if ((!_mesa_is_desktop_gl(ctx)
              || !ctx->Extensions.EXT_texture_swizzle)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Swizzle[pname - GL_TEXTURE_SWIZZLE_R_EXT];
         break;

      case GL_TEXTURE_SWIZZLE_RGBA_EXT:
         if ((!_mesa_is_desktop_gl(ctx)
              || !ctx->Extensions.EXT_texture_swizzle)
             && !_mesa_is_gles3(ctx)) {
            goto invalid_pname;
         }
         else {
            GLuint comp;
            for (comp = 0; comp < 4; comp++) {
               params[comp] = (GLfloat) obj->Swizzle[comp];
            }
         }
         break;

      case GL_TEXTURE_CUBE_MAP_SEAMLESS:
         if (!_mesa_is_desktop_gl(ctx)
             || !ctx->Extensions.AMD_seamless_cubemap_per_texture)
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.CubeMapSeamless;
         break;

      case GL_TEXTURE_IMMUTABLE_FORMAT:
         *params = (GLfloat) obj->Immutable;
         break;

      case GL_TEXTURE_IMMUTABLE_LEVELS:
         if (_mesa_is_gles3(ctx) ||
             (_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_texture_view))
            *params = (GLfloat) obj->ImmutableLevels;
         else
            goto invalid_pname;
         break;

      case GL_TEXTURE_VIEW_MIN_LEVEL:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLfloat) obj->MinLevel;
         break;

      case GL_TEXTURE_VIEW_NUM_LEVELS:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLfloat) obj->NumLevels;
         break;

      case GL_TEXTURE_VIEW_MIN_LAYER:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLfloat) obj->MinLayer;
         break;

      case GL_TEXTURE_VIEW_NUM_LAYERS:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLfloat) obj->NumLayers;
         break;

      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         if (!_mesa_is_gles(ctx) || !ctx->Extensions.OES_EGL_image_external)
            goto invalid_pname;
         *params = (GLfloat) obj->RequiredTextureImageUnits;
         break;

      case GL_TEXTURE_SRGB_DECODE_EXT:
         if (!ctx->Extensions.EXT_texture_sRGB_decode)
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.sRGBDecode;
         break;

      default:
         goto invalid_pname;
   }

   /* no error if we get here */
   _mesa_unlock_context_textures(ctx);
   return;

invalid_pname:
   _mesa_unlock_context_textures(ctx);
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetTexParameterfv(pname=0x%x)", pname);
}


void GLAPIENTRY
_mesa_GetTexParameteriv( GLenum target, GLenum pname, GLint *params )
{
   struct gl_texture_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = get_texobj(ctx, target, GL_TRUE);
   if (!obj)
      return;

   _mesa_lock_texture(ctx, obj);
   switch (pname) {
      case GL_TEXTURE_MAG_FILTER:
         *params = (GLint) obj->Sampler.MagFilter;
         break;
      case GL_TEXTURE_MIN_FILTER:
         *params = (GLint) obj->Sampler.MinFilter;
         break;
      case GL_TEXTURE_WRAP_S:
         *params = (GLint) obj->Sampler.WrapS;
         break;
      case GL_TEXTURE_WRAP_T:
         *params = (GLint) obj->Sampler.WrapT;
         break;
      case GL_TEXTURE_WRAP_R:
         *params = (GLint) obj->Sampler.WrapR;
         break;
      case GL_TEXTURE_BORDER_COLOR:
         if (!_mesa_is_desktop_gl(ctx))
            goto invalid_pname;

         {
            GLfloat b[4];
            b[0] = CLAMP(obj->Sampler.BorderColor.f[0], 0.0F, 1.0F);
            b[1] = CLAMP(obj->Sampler.BorderColor.f[1], 0.0F, 1.0F);
            b[2] = CLAMP(obj->Sampler.BorderColor.f[2], 0.0F, 1.0F);
            b[3] = CLAMP(obj->Sampler.BorderColor.f[3], 0.0F, 1.0F);
            params[0] = FLOAT_TO_INT(b[0]);
            params[1] = FLOAT_TO_INT(b[1]);
            params[2] = FLOAT_TO_INT(b[2]);
            params[3] = FLOAT_TO_INT(b[3]);
         }
         break;
      case GL_TEXTURE_RESIDENT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;

         *params = 1;
         break;
      case GL_TEXTURE_PRIORITY:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;

         *params = FLOAT_TO_INT(obj->Priority);
         break;
      case GL_TEXTURE_MIN_LOD:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = (GLint) obj->Sampler.MinLod;
         break;
      case GL_TEXTURE_MAX_LOD:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = (GLint) obj->Sampler.MaxLod;
         break;
      case GL_TEXTURE_BASE_LEVEL:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = obj->BaseLevel;
         break;
      case GL_TEXTURE_MAX_LEVEL:
         *params = obj->MaxLevel;
         break;
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
         if (!ctx->Extensions.EXT_texture_filter_anisotropic)
            goto invalid_pname;
         *params = (GLint) obj->Sampler.MaxAnisotropy;
         break;
      case GL_GENERATE_MIPMAP_SGIS:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_pname;

	 *params = (GLint) obj->GenerateMipmap;
         break;
      case GL_TEXTURE_COMPARE_MODE_ARB:
         if ((!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_shadow)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLint) obj->Sampler.CompareMode;
         break;
      case GL_TEXTURE_COMPARE_FUNC_ARB:
         if ((!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_shadow)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLint) obj->Sampler.CompareFunc;
         break;
      case GL_DEPTH_TEXTURE_MODE_ARB:
         if (ctx->API != API_OPENGL_COMPAT || !ctx->Extensions.ARB_depth_texture)
            goto invalid_pname;
         *params = (GLint) obj->DepthMode;
         break;
      case GL_DEPTH_STENCIL_TEXTURE_MODE:
         if (!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_stencil_texturing)
            goto invalid_pname;
         *params = (GLint)
            (obj->StencilSampling ? GL_STENCIL_INDEX : GL_DEPTH_COMPONENT);
         break;
      case GL_TEXTURE_LOD_BIAS:
         if (_mesa_is_gles(ctx))
            goto invalid_pname;

         /* GL spec 'Data Conversions' section specifies that floating-point
          * value in integer Get function is rounded to nearest integer
          */
         *params = IROUND(obj->Sampler.LodBias);
         break;
      case GL_TEXTURE_CROP_RECT_OES:
         if (ctx->API != API_OPENGLES || !ctx->Extensions.OES_draw_texture)
            goto invalid_pname;

         params[0] = obj->CropRect[0];
         params[1] = obj->CropRect[1];
         params[2] = obj->CropRect[2];
         params[3] = obj->CropRect[3];
         break;
      case GL_TEXTURE_SWIZZLE_R_EXT:
      case GL_TEXTURE_SWIZZLE_G_EXT:
      case GL_TEXTURE_SWIZZLE_B_EXT:
      case GL_TEXTURE_SWIZZLE_A_EXT:
         if ((!_mesa_is_desktop_gl(ctx)
              || !ctx->Extensions.EXT_texture_swizzle)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = obj->Swizzle[pname - GL_TEXTURE_SWIZZLE_R_EXT];
         break;

      case GL_TEXTURE_SWIZZLE_RGBA_EXT:
         if ((!_mesa_is_desktop_gl(ctx)
              || !ctx->Extensions.EXT_texture_swizzle)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         COPY_4V(params, obj->Swizzle);
         break;

      case GL_TEXTURE_CUBE_MAP_SEAMLESS:
         if (!_mesa_is_desktop_gl(ctx)
             || !ctx->Extensions.AMD_seamless_cubemap_per_texture)
            goto invalid_pname;
         *params = (GLint) obj->Sampler.CubeMapSeamless;
         break;

      case GL_TEXTURE_IMMUTABLE_FORMAT:
         *params = (GLint) obj->Immutable;
         break;

      case GL_TEXTURE_IMMUTABLE_LEVELS:
         if (_mesa_is_gles3(ctx) ||
             (_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_texture_view))
            *params = obj->ImmutableLevels;
         else
            goto invalid_pname;
         break;

      case GL_TEXTURE_VIEW_MIN_LEVEL:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLint) obj->MinLevel;
         break;

      case GL_TEXTURE_VIEW_NUM_LEVELS:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLint) obj->NumLevels;
         break;

      case GL_TEXTURE_VIEW_MIN_LAYER:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLint) obj->MinLayer;
         break;

      case GL_TEXTURE_VIEW_NUM_LAYERS:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLint) obj->NumLayers;
         break;

      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         if (!_mesa_is_gles(ctx) || !ctx->Extensions.OES_EGL_image_external)
            goto invalid_pname;
         *params = obj->RequiredTextureImageUnits;
         break;

      case GL_TEXTURE_SRGB_DECODE_EXT:
         if (!ctx->Extensions.EXT_texture_sRGB_decode)
            goto invalid_pname;
         *params = obj->Sampler.sRGBDecode;
         break;

      case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
         if (!ctx->Extensions.ARB_shader_image_load_store)
            goto invalid_pname;
         *params = obj->ImageFormatCompatibilityType;
         break;

      default:
         goto invalid_pname;
   }

   /* no error if we get here */
   _mesa_unlock_texture(ctx, obj);
   return;

invalid_pname:
   _mesa_unlock_texture(ctx, obj);
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetTexParameteriv(pname=0x%x)", pname);
}


/** New in GL 3.0 */
void GLAPIENTRY
_mesa_GetTexParameterIiv(GLenum target, GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj(ctx, target, GL_TRUE);
   if (!texObj)
      return;

   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      COPY_4V(params, texObj->Sampler.BorderColor.i);
      break;
   default:
      _mesa_GetTexParameteriv(target, pname, params);
   }
}


/** New in GL 3.0 */
void GLAPIENTRY
_mesa_GetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj(ctx, target, GL_TRUE);
   if (!texObj)
      return;

   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      COPY_4V(params, texObj->Sampler.BorderColor.i);
      break;
   default:
      {
         GLint ip[4];
         _mesa_GetTexParameteriv(target, pname, ip);
         params[0] = ip[0];
         if (pname == GL_TEXTURE_SWIZZLE_RGBA_EXT ||
             pname == GL_TEXTURE_CROP_RECT_OES) {
            params[1] = ip[1];
            params[2] = ip[2];
            params[3] = ip[3];
         }
      }
   }
}
