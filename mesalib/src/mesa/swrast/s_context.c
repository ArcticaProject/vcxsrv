/*
 * Mesa 3-D graphics library
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@vmware.com> Brian Paul
 */

#include "main/imports.h"
#include "main/bufferobj.h"
#include "main/colormac.h"
#include "main/mtypes.h"
#include "main/samplerobj.h"
#include "main/teximage.h"
#include "program/prog_parameter.h"
#include "program/prog_statevars.h"
#include "swrast.h"
#include "s_blend.h"
#include "s_context.h"
#include "s_lines.h"
#include "s_points.h"
#include "s_span.h"
#include "s_texfetch.h"
#include "s_triangle.h"
#include "s_texfilter.h"


/**
 * Recompute the value of swrast->_RasterMask, etc. according to
 * the current context.  The _RasterMask field can be easily tested by
 * drivers to determine certain basic GL state (does the primitive need
 * stenciling, logic-op, fog, etc?).
 */
static void
_swrast_update_rasterflags( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   GLbitfield rasterMask = 0;
   GLuint i;

   if (ctx->Color.AlphaEnabled)           rasterMask |= ALPHATEST_BIT;
   if (ctx->Color.BlendEnabled)           rasterMask |= BLEND_BIT;
   if (ctx->Depth.Test)                   rasterMask |= DEPTH_BIT;
   if (swrast->_FogEnabled)               rasterMask |= FOG_BIT;
   if (ctx->Scissor.EnableFlags)          rasterMask |= CLIP_BIT;
   if (ctx->Stencil._Enabled)             rasterMask |= STENCIL_BIT;
   for (i = 0; i < ctx->Const.MaxDrawBuffers; i++) {
      if (!ctx->Color.ColorMask[i][0] ||
          !ctx->Color.ColorMask[i][1] ||
          !ctx->Color.ColorMask[i][2] ||
          !ctx->Color.ColorMask[i][3]) {
         rasterMask |= MASKING_BIT;
         break;
      }
   }
   if (ctx->Color.ColorLogicOpEnabled) rasterMask |= LOGIC_OP_BIT;
   if (ctx->Texture._MaxEnabledTexImageUnit >= 0) rasterMask |= TEXTURE_BIT;
   if (   ctx->ViewportArray[0].X < 0
       || ctx->ViewportArray[0].X + ctx->ViewportArray[0].Width > (GLfloat) ctx->DrawBuffer->Width
       || ctx->ViewportArray[0].Y < 0
       || ctx->ViewportArray[0].Y + ctx->ViewportArray[0].Height > (GLfloat) ctx->DrawBuffer->Height) {
      rasterMask |= CLIP_BIT;
   }

   if (ctx->Query.CurrentOcclusionObject)
      rasterMask |= OCCLUSION_BIT;


   /* If we're not drawing to exactly one color buffer set the
    * MULTI_DRAW_BIT flag.  Also set it if we're drawing to no
    * buffers or the RGBA or CI mask disables all writes.
    */
   if (ctx->DrawBuffer->_NumColorDrawBuffers != 1) {
      /* more than one color buffer designated for writing (or zero buffers) */
      rasterMask |= MULTI_DRAW_BIT;
   }

   for (i = 0; i < ctx->Const.MaxDrawBuffers; i++) {
      if (ctx->Color.ColorMask[i][0] +
          ctx->Color.ColorMask[i][1] +
          ctx->Color.ColorMask[i][2] +
          ctx->Color.ColorMask[i][3] == 0) {
         rasterMask |= MULTI_DRAW_BIT; /* all RGBA channels disabled */
         break;
      }
   }


   if (_swrast_use_fragment_program(ctx)) {
      rasterMask |= FRAGPROG_BIT;
   }

   if (ctx->ATIFragmentShader._Enabled) {
      rasterMask |= ATIFRAGSHADER_BIT;
   }

#if CHAN_TYPE == GL_FLOAT
   if (ctx->Color.ClampFragmentColor == GL_TRUE) {
      rasterMask |= CLAMPING_BIT;
   }
#endif

   SWRAST_CONTEXT(ctx)->_RasterMask = rasterMask;
}


/**
 * Examine polygon cull state to compute the _BackfaceCullSign field.
 * _BackfaceCullSign will be 0 if no culling, -1 if culling back-faces,
 * and 1 if culling front-faces.  The Polygon FrontFace state also
 * factors in.
 */
static void
_swrast_update_polygon( struct gl_context *ctx )
{
   GLfloat backface_sign;

   if (ctx->Polygon.CullFlag) {
      switch (ctx->Polygon.CullFaceMode) {
      case GL_BACK:
         backface_sign = -1.0F;
	 break;
      case GL_FRONT:
         backface_sign = 1.0F;
	 break;
      case GL_FRONT_AND_BACK:
         /* fallthrough */
      default:
	 backface_sign = 0.0F;
      }
   }
   else {
      backface_sign = 0.0F;
   }

   SWRAST_CONTEXT(ctx)->_BackfaceCullSign = backface_sign;

   /* This is for front/back-face determination, but not for culling */
   SWRAST_CONTEXT(ctx)->_BackfaceSign
      = (ctx->Polygon.FrontFace == GL_CW) ? -1.0F : 1.0F;
}



/**
 * Update the _PreferPixelFog field to indicate if we need to compute
 * fog blend factors (from the fog coords) per-fragment.
 */
static void
_swrast_update_fog_hint( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   swrast->_PreferPixelFog = (!swrast->AllowVertexFog ||
			      _swrast_use_fragment_program(ctx) ||
			      (ctx->Hint.Fog == GL_NICEST &&
			       swrast->AllowPixelFog));
}



/**
 * Update the swrast->_TextureCombinePrimary flag.
 */
static void
_swrast_update_texture_env( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   GLuint i;

   swrast->_TextureCombinePrimary = GL_FALSE;

   for (i = 0; i < ctx->Const.MaxTextureUnits; i++) {
      const struct gl_tex_env_combine_state *combine =
         ctx->Texture.Unit[i]._CurrentCombine;
      GLuint term;
      for (term = 0; term < combine->_NumArgsRGB; term++) {
         if (combine->SourceRGB[term] == GL_PRIMARY_COLOR) {
            swrast->_TextureCombinePrimary = GL_TRUE;
            return;
         }
         if (combine->SourceA[term] == GL_PRIMARY_COLOR) {
            swrast->_TextureCombinePrimary = GL_TRUE;
            return;
         }
      }
   }
}


/**
 * Determine if we can defer texturing/shading until after Z/stencil
 * testing.  This potentially allows us to skip texturing/shading for
 * lots of fragments.
 */
static void
_swrast_update_deferred_texture(struct gl_context *ctx)
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   if (ctx->Color.AlphaEnabled) {
      /* alpha test depends on post-texture/shader colors */
      swrast->_DeferredTexture = GL_FALSE;
   }
   else {
      GLboolean use_fprog = _swrast_use_fragment_program(ctx);
      const struct gl_fragment_program *fprog
         = ctx->FragmentProgram._Current;
      if (use_fprog && (fprog->Base.OutputsWritten & (1 << FRAG_RESULT_DEPTH))) {
         /* Z comes from fragment program/shader */
         swrast->_DeferredTexture = GL_FALSE;
      }
      else if (use_fprog && fprog->UsesKill) {
         swrast->_DeferredTexture = GL_FALSE;
      }
      else if (ctx->Query.CurrentOcclusionObject) {
         /* occlusion query depends on shader discard/kill results */
         swrast->_DeferredTexture = GL_FALSE;
      }
      else {
         swrast->_DeferredTexture = GL_TRUE;
      }
   }
}


/**
 * Update swrast->_FogColor and swrast->_FogEnable values.
 */
static void
_swrast_update_fog_state( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   const struct gl_fragment_program *fp = ctx->FragmentProgram._Current;

   assert(fp == NULL || fp->Base.Target == GL_FRAGMENT_PROGRAM_ARB);

   /* determine if fog is needed, and if so, which fog mode */
   swrast->_FogEnabled = (!_swrast_use_fragment_program(ctx) &&
			  ctx->Fog.Enabled);
}


/**
 * Update state for running fragment programs.  Basically, load the
 * program parameters with current state values.
 */
static void
_swrast_update_fragment_program(struct gl_context *ctx, GLbitfield newState)
{
   if (!_swrast_use_fragment_program(ctx))
      return;

   _mesa_load_state_parameters(ctx,
                               ctx->FragmentProgram._Current->Base.Parameters);
}


/**
 * See if we can do early diffuse+specular (primary+secondary) color
 * add per vertex instead of per-fragment.
 */
static void
_swrast_update_specular_vertex_add(struct gl_context *ctx)
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   GLboolean separateSpecular = ctx->Fog.ColorSumEnabled ||
      (ctx->Light.Enabled &&
       ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR);

   swrast->SpecularVertexAdd = (separateSpecular
                                && ctx->Texture._MaxEnabledTexImageUnit == -1
                                && !_swrast_use_fragment_program(ctx)
                                && !ctx->ATIFragmentShader._Enabled);
}


#define _SWRAST_NEW_DERIVED (_SWRAST_NEW_RASTERMASK |	\
                             _NEW_PROGRAM_CONSTANTS |   \
			     _NEW_TEXTURE |		\
			     _NEW_HINT |		\
			     _NEW_POLYGON )

/* State referenced by _swrast_choose_triangle, _swrast_choose_line.
 */
#define _SWRAST_NEW_TRIANGLE (_SWRAST_NEW_DERIVED |		\
			      _NEW_RENDERMODE|			\
                              _NEW_POLYGON|			\
                              _NEW_DEPTH|			\
                              _NEW_STENCIL|			\
                              _NEW_COLOR|			\
                              _NEW_TEXTURE|			\
                              _SWRAST_NEW_RASTERMASK|		\
                              _NEW_LIGHT|			\
                              _NEW_FOG |			\
			      _MESA_NEW_SEPARATE_SPECULAR)

#define _SWRAST_NEW_LINE (_SWRAST_NEW_DERIVED |		\
			  _NEW_RENDERMODE|		\
                          _NEW_LINE|			\
                          _NEW_TEXTURE|			\
                          _NEW_LIGHT|			\
                          _NEW_FOG|			\
                          _NEW_DEPTH |			\
                          _MESA_NEW_SEPARATE_SPECULAR)

#define _SWRAST_NEW_POINT (_SWRAST_NEW_DERIVED |	\
			   _NEW_RENDERMODE |		\
			   _NEW_POINT |			\
			   _NEW_TEXTURE |		\
			   _NEW_LIGHT |			\
			   _NEW_FOG |			\
                           _MESA_NEW_SEPARATE_SPECULAR)

#define _SWRAST_NEW_TEXTURE_SAMPLE_FUNC _NEW_TEXTURE

#define _SWRAST_NEW_TEXTURE_ENV_MODE _NEW_TEXTURE

#define _SWRAST_NEW_BLEND_FUNC _NEW_COLOR



/**
 * Stub for swrast->Triangle to select a true triangle function
 * after a state change.
 */
static void
_swrast_validate_triangle( struct gl_context *ctx,
			   const SWvertex *v0,
                           const SWvertex *v1,
                           const SWvertex *v2 )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);

   _swrast_validate_derived( ctx );
   swrast->choose_triangle( ctx );
   ASSERT(swrast->Triangle);

   if (swrast->SpecularVertexAdd) {
      /* separate specular color, but no texture */
      swrast->SpecTriangle = swrast->Triangle;
      swrast->Triangle = _swrast_add_spec_terms_triangle;
   }

   swrast->Triangle( ctx, v0, v1, v2 );
}

/**
 * Called via swrast->Line.  Examine current GL state and choose a software
 * line routine.  Then call it.
 */
static void
_swrast_validate_line( struct gl_context *ctx, const SWvertex *v0, const SWvertex *v1 )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);

   _swrast_validate_derived( ctx );
   swrast->choose_line( ctx );
   ASSERT(swrast->Line);

   if (swrast->SpecularVertexAdd) {
      swrast->SpecLine = swrast->Line;
      swrast->Line = _swrast_add_spec_terms_line;
   }

   swrast->Line( ctx, v0, v1 );
}

/**
 * Called via swrast->Point.  Examine current GL state and choose a software
 * point routine.  Then call it.
 */
static void
_swrast_validate_point( struct gl_context *ctx, const SWvertex *v0 )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);

   _swrast_validate_derived( ctx );
   swrast->choose_point( ctx );

   if (swrast->SpecularVertexAdd) {
      swrast->SpecPoint = swrast->Point;
      swrast->Point = _swrast_add_spec_terms_point;
   }

   swrast->Point( ctx, v0 );
}


/**
 * Called via swrast->BlendFunc.  Examine GL state to choose a blending
 * function, then call it.
 */
static void _ASMAPI
_swrast_validate_blend_func(struct gl_context *ctx, GLuint n, const GLubyte mask[],
                            GLvoid *src, const GLvoid *dst,
                            GLenum chanType )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);

   _swrast_validate_derived( ctx ); /* why is this needed? */
   _swrast_choose_blend_func( ctx, chanType );

   swrast->BlendFunc( ctx, n, mask, src, dst, chanType );
}

static void
_swrast_sleep( struct gl_context *ctx, GLbitfield new_state )
{
   (void) ctx; (void) new_state;
}


static void
_swrast_invalidate_state( struct gl_context *ctx, GLbitfield new_state )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   GLuint i;

   swrast->NewState |= new_state;

   /* After 10 statechanges without any swrast functions being called,
    * put the module to sleep.
    */
   if (++swrast->StateChanges > 10) {
      swrast->InvalidateState = _swrast_sleep;
      swrast->NewState = ~0;
      new_state = ~0;
   }

   if (new_state & swrast->InvalidateTriangleMask)
      swrast->Triangle = _swrast_validate_triangle;

   if (new_state & swrast->InvalidateLineMask)
      swrast->Line = _swrast_validate_line;

   if (new_state & swrast->InvalidatePointMask)
      swrast->Point = _swrast_validate_point;

   if (new_state & _SWRAST_NEW_BLEND_FUNC)
      swrast->BlendFunc = _swrast_validate_blend_func;

   if (new_state & _SWRAST_NEW_TEXTURE_SAMPLE_FUNC)
      for (i = 0 ; i < ARRAY_SIZE(swrast->TextureSample); i++)
	 swrast->TextureSample[i] = NULL;
}


void
_swrast_update_texture_samplers(struct gl_context *ctx)
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   GLuint u;

   if (!swrast)
      return; /* pipe hack */

   for (u = 0; u < ARRAY_SIZE(swrast->TextureSample); u++) {
      struct gl_texture_object *tObj = ctx->Texture.Unit[u]._Current;
      /* Note: If tObj is NULL, the sample function will be a simple
       * function that just returns opaque black (0,0,0,1).
       */
      _mesa_update_fetch_functions(ctx, u);
      swrast->TextureSample[u] =
         _swrast_choose_texture_sample_func(ctx, tObj,
                                            _mesa_get_samplerobj(ctx, u));
   }
}


/**
 * Update swrast->_ActiveAttribs, swrast->_NumActiveAttribs,
 * swrast->_ActiveAtttribMask.
 */
static void
_swrast_update_active_attribs(struct gl_context *ctx)
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   GLbitfield64 attribsMask;

   /*
    * Compute _ActiveAttribsMask = which fragment attributes are needed.
    */
   if (_swrast_use_fragment_program(ctx)) {
      /* fragment program/shader */
      attribsMask = ctx->FragmentProgram._Current->Base.InputsRead;
      attribsMask &= ~VARYING_BIT_POS; /* WPOS is always handled specially */
   }
   else if (ctx->ATIFragmentShader._Enabled) {
      attribsMask = ~0;  /* XXX fix me */
   }
   else {
      /* fixed function */
      attribsMask = 0x0;

#if CHAN_TYPE == GL_FLOAT
      attribsMask |= VARYING_BIT_COL0;
#endif

      if (ctx->Fog.ColorSumEnabled ||
          (ctx->Light.Enabled &&
           ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)) {
         attribsMask |= VARYING_BIT_COL1;
      }

      if (swrast->_FogEnabled)
         attribsMask |= VARYING_BIT_FOGC;

      attribsMask |= (ctx->Texture._EnabledCoordUnits << VARYING_SLOT_TEX0);
   }

   swrast->_ActiveAttribMask = attribsMask;

   /* Update _ActiveAttribs[] list */
   {
      GLuint i, num = 0;
      for (i = 0; i < VARYING_SLOT_MAX; i++) {
         if (attribsMask & BITFIELD64_BIT(i)) {
            swrast->_ActiveAttribs[num++] = i;
            /* how should this attribute be interpolated? */
            if (i == VARYING_SLOT_COL0 || i == VARYING_SLOT_COL1)
               swrast->_InterpMode[i] = ctx->Light.ShadeModel;
            else
               swrast->_InterpMode[i] = GL_SMOOTH;
         }
      }
      swrast->_NumActiveAttribs = num;
   }
}


void
_swrast_validate_derived( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);

   if (swrast->NewState) {
      if (swrast->NewState & _NEW_POLYGON)
	 _swrast_update_polygon( ctx );

      if (swrast->NewState & (_NEW_HINT | _NEW_PROGRAM))
	 _swrast_update_fog_hint( ctx );

      if (swrast->NewState & _SWRAST_NEW_TEXTURE_ENV_MODE)
	 _swrast_update_texture_env( ctx );

      if (swrast->NewState & (_NEW_FOG | _NEW_PROGRAM))
         _swrast_update_fog_state( ctx );

      if (swrast->NewState & (_NEW_PROGRAM_CONSTANTS | _NEW_PROGRAM))
	 _swrast_update_fragment_program( ctx, swrast->NewState );

      if (swrast->NewState & (_NEW_TEXTURE | _NEW_PROGRAM)) {
         _swrast_update_texture_samplers( ctx );
      }

      if (swrast->NewState & (_NEW_COLOR | _NEW_PROGRAM))
         _swrast_update_deferred_texture(ctx);

      if (swrast->NewState & _SWRAST_NEW_RASTERMASK)
 	 _swrast_update_rasterflags( ctx );

      if (swrast->NewState & (_NEW_DEPTH |
                              _NEW_FOG |
                              _NEW_LIGHT |
                              _NEW_PROGRAM |
                              _NEW_TEXTURE))
         _swrast_update_active_attribs(ctx);

      if (swrast->NewState & (_NEW_FOG | 
                              _NEW_PROGRAM |
                              _NEW_LIGHT |
                              _NEW_TEXTURE))
         _swrast_update_specular_vertex_add(ctx);

      swrast->NewState = 0;
      swrast->StateChanges = 0;
      swrast->InvalidateState = _swrast_invalidate_state;
   }
}

#define SWRAST_DEBUG 0

/* Public entrypoints:  See also s_bitmap.c, etc.
 */
void
_swrast_Quad( struct gl_context *ctx,
	      const SWvertex *v0, const SWvertex *v1,
              const SWvertex *v2, const SWvertex *v3 )
{
   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_Quad\n");
      _swrast_print_vertex( ctx, v0 );
      _swrast_print_vertex( ctx, v1 );
      _swrast_print_vertex( ctx, v2 );
      _swrast_print_vertex( ctx, v3 );
   }
   SWRAST_CONTEXT(ctx)->Triangle( ctx, v0, v1, v3 );
   SWRAST_CONTEXT(ctx)->Triangle( ctx, v1, v2, v3 );
}

void
_swrast_Triangle( struct gl_context *ctx, const SWvertex *v0,
                  const SWvertex *v1, const SWvertex *v2 )
{
   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_Triangle\n");
      _swrast_print_vertex( ctx, v0 );
      _swrast_print_vertex( ctx, v1 );
      _swrast_print_vertex( ctx, v2 );
   }
   SWRAST_CONTEXT(ctx)->Triangle( ctx, v0, v1, v2 );
}

void
_swrast_Line( struct gl_context *ctx, const SWvertex *v0, const SWvertex *v1 )
{
   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_Line\n");
      _swrast_print_vertex( ctx, v0 );
      _swrast_print_vertex( ctx, v1 );
   }
   SWRAST_CONTEXT(ctx)->Line( ctx, v0, v1 );
}

void
_swrast_Point( struct gl_context *ctx, const SWvertex *v0 )
{
   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_Point\n");
      _swrast_print_vertex( ctx, v0 );
   }
   SWRAST_CONTEXT(ctx)->Point( ctx, v0 );
}

void
_swrast_InvalidateState( struct gl_context *ctx, GLbitfield new_state )
{
   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_InvalidateState\n");
   }
   SWRAST_CONTEXT(ctx)->InvalidateState( ctx, new_state );
}

void
_swrast_ResetLineStipple( struct gl_context *ctx )
{
   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_ResetLineStipple\n");
   }
   SWRAST_CONTEXT(ctx)->StippleCounter = 0;
}

void
_swrast_SetFacing(struct gl_context *ctx, GLuint facing)
{
   SWRAST_CONTEXT(ctx)->PointLineFacing = facing;
}

void
_swrast_allow_vertex_fog( struct gl_context *ctx, GLboolean value )
{
   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_allow_vertex_fog %d\n", value);
   }
   SWRAST_CONTEXT(ctx)->InvalidateState( ctx, _NEW_HINT );
   SWRAST_CONTEXT(ctx)->AllowVertexFog = value;
}

void
_swrast_allow_pixel_fog( struct gl_context *ctx, GLboolean value )
{
   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_allow_pixel_fog %d\n", value);
   }
   SWRAST_CONTEXT(ctx)->InvalidateState( ctx, _NEW_HINT );
   SWRAST_CONTEXT(ctx)->AllowPixelFog = value;
}


/**
 * Initialize native program limits by copying the logical limits.
 * See comments in init_program_limits() in context.c
 */
static void
init_program_native_limits(struct gl_program_constants *prog)
{
   prog->MaxNativeInstructions = prog->MaxInstructions;
   prog->MaxNativeAluInstructions = prog->MaxAluInstructions;
   prog->MaxNativeTexInstructions = prog->MaxTexInstructions;
   prog->MaxNativeTexIndirections = prog->MaxTexIndirections;
   prog->MaxNativeAttribs = prog->MaxAttribs;
   prog->MaxNativeTemps = prog->MaxTemps;
   prog->MaxNativeAddressRegs = prog->MaxAddressRegs;
   prog->MaxNativeParameters = prog->MaxParameters;
}


GLboolean
_swrast_CreateContext( struct gl_context *ctx )
{
   GLuint i;
   SWcontext *swrast = calloc(1, sizeof(SWcontext));
#ifdef _OPENMP
   const GLuint maxThreads = omp_get_max_threads();
#else
   const GLuint maxThreads = 1;
#endif

   assert(ctx->Const.MaxViewportWidth <= SWRAST_MAX_WIDTH);
   assert(ctx->Const.MaxViewportHeight <= SWRAST_MAX_WIDTH);

   assert(ctx->Const.MaxRenderbufferSize <= SWRAST_MAX_WIDTH);

   /* make sure largest texture image is <= SWRAST_MAX_WIDTH in size */
   assert((1 << (ctx->Const.MaxTextureLevels - 1)) <= SWRAST_MAX_WIDTH);
   assert((1 << (ctx->Const.MaxCubeTextureLevels - 1)) <= SWRAST_MAX_WIDTH);
   assert((1 << (ctx->Const.Max3DTextureLevels - 1)) <= SWRAST_MAX_WIDTH);

   assert(PROG_MAX_WIDTH == SWRAST_MAX_WIDTH);

   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_CreateContext\n");
   }

   if (!swrast)
      return GL_FALSE;

   swrast->NewState = ~0;

   swrast->choose_point = _swrast_choose_point;
   swrast->choose_line = _swrast_choose_line;
   swrast->choose_triangle = _swrast_choose_triangle;

   swrast->InvalidatePointMask = _SWRAST_NEW_POINT;
   swrast->InvalidateLineMask = _SWRAST_NEW_LINE;
   swrast->InvalidateTriangleMask = _SWRAST_NEW_TRIANGLE;

   swrast->Point = _swrast_validate_point;
   swrast->Line = _swrast_validate_line;
   swrast->Triangle = _swrast_validate_triangle;
   swrast->InvalidateState = _swrast_sleep;
   swrast->BlendFunc = _swrast_validate_blend_func;

   swrast->AllowVertexFog = GL_TRUE;
   swrast->AllowPixelFog = GL_TRUE;

   swrast->Driver.SpanRenderStart = _swrast_span_render_start;
   swrast->Driver.SpanRenderFinish = _swrast_span_render_finish;

   for (i = 0; i < ARRAY_SIZE(swrast->TextureSample); i++)
      swrast->TextureSample[i] = NULL;

   /* SpanArrays is global and shared by all SWspan instances. However, when
    * using multiple threads, it is necessary to have one SpanArrays instance
    * per thread.
    */
   swrast->SpanArrays = malloc(maxThreads * sizeof(SWspanarrays));
   if (!swrast->SpanArrays) {
      free(swrast);
      return GL_FALSE;
   }
   for(i = 0; i < maxThreads; i++) {
      swrast->SpanArrays[i].ChanType = CHAN_TYPE;
#if CHAN_TYPE == GL_UNSIGNED_BYTE
      swrast->SpanArrays[i].rgba = swrast->SpanArrays[i].rgba8;
#elif CHAN_TYPE == GL_UNSIGNED_SHORT
      swrast->SpanArrays[i].rgba = swrast->SpanArrays[i].rgba16;
#else
      swrast->SpanArrays[i].rgba = swrast->SpanArrays[i].attribs[VARYING_SLOT_COL0];
#endif
   }

   /* init point span buffer */
   swrast->PointSpan.primitive = GL_POINT;
   swrast->PointSpan.end = 0;
   swrast->PointSpan.facing = 0;
   swrast->PointSpan.array = swrast->SpanArrays;

   init_program_native_limits(&ctx->Const.Program[MESA_SHADER_VERTEX]);
   init_program_native_limits(&ctx->Const.Program[MESA_SHADER_GEOMETRY]);
   init_program_native_limits(&ctx->Const.Program[MESA_SHADER_FRAGMENT]);

   ctx->swrast_context = swrast;

   swrast->stencil_temp.buf1 = malloc(SWRAST_MAX_WIDTH * sizeof(GLubyte));
   swrast->stencil_temp.buf2 = malloc(SWRAST_MAX_WIDTH * sizeof(GLubyte));
   swrast->stencil_temp.buf3 = malloc(SWRAST_MAX_WIDTH * sizeof(GLubyte));
   swrast->stencil_temp.buf4 = malloc(SWRAST_MAX_WIDTH * sizeof(GLubyte));

   if (!swrast->stencil_temp.buf1 ||
       !swrast->stencil_temp.buf2 ||
       !swrast->stencil_temp.buf3 ||
       !swrast->stencil_temp.buf4) {
      _swrast_DestroyContext(ctx);
      return GL_FALSE;
   }

   return GL_TRUE;
}

void
_swrast_DestroyContext( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);

   if (SWRAST_DEBUG) {
      _mesa_debug(ctx, "_swrast_DestroyContext\n");
   }

   free( swrast->SpanArrays );
   free( swrast->ZoomedArrays );
   free( swrast->TexelBuffer );

   free(swrast->stencil_temp.buf1);
   free(swrast->stencil_temp.buf2);
   free(swrast->stencil_temp.buf3);
   free(swrast->stencil_temp.buf4);

   free( swrast );

   ctx->swrast_context = 0;
}


struct swrast_device_driver *
_swrast_GetDeviceDriverReference( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   return &swrast->Driver;
}

void
_swrast_flush( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   /* flush any pending fragments from rendering points */
   if (swrast->PointSpan.end > 0) {
      _swrast_write_rgba_span(ctx, &(swrast->PointSpan));
      swrast->PointSpan.end = 0;
   }
}

void
_swrast_render_primitive( struct gl_context *ctx, GLenum prim )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   if (swrast->Primitive == GL_POINTS && prim != GL_POINTS) {
      _swrast_flush(ctx);
   }
   swrast->Primitive = prim;
}


/** called via swrast->Driver.SpanRenderStart() */
void
_swrast_span_render_start(struct gl_context *ctx)
{
   _swrast_map_textures(ctx);
   _swrast_map_renderbuffers(ctx);
}


/** called via swrast->Driver.SpanRenderFinish() */
void
_swrast_span_render_finish(struct gl_context *ctx)
{
   _swrast_unmap_textures(ctx);
   _swrast_unmap_renderbuffers(ctx);
}


void
_swrast_render_start( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);
   if (swrast->Driver.SpanRenderStart)
      swrast->Driver.SpanRenderStart( ctx );
   swrast->PointSpan.end = 0;
}
 
void
_swrast_render_finish( struct gl_context *ctx )
{
   SWcontext *swrast = SWRAST_CONTEXT(ctx);

   _swrast_flush(ctx);

   if (swrast->Driver.SpanRenderFinish)
      swrast->Driver.SpanRenderFinish( ctx );
}


#define SWRAST_DEBUG_VERTICES 0

void
_swrast_print_vertex( struct gl_context *ctx, const SWvertex *v )
{
   GLuint i;

   if (SWRAST_DEBUG_VERTICES) {
      _mesa_debug(ctx, "win %f %f %f %f\n",
                  v->attrib[VARYING_SLOT_POS][0],
                  v->attrib[VARYING_SLOT_POS][1],
                  v->attrib[VARYING_SLOT_POS][2],
                  v->attrib[VARYING_SLOT_POS][3]);

      for (i = 0 ; i < ctx->Const.MaxTextureCoordUnits ; i++)
	 if (ctx->Texture.Unit[i]._Current)
	    _mesa_debug(ctx, "texcoord[%d] %f %f %f %f\n", i,
                        v->attrib[VARYING_SLOT_TEX0 + i][0],
                        v->attrib[VARYING_SLOT_TEX0 + i][1],
                        v->attrib[VARYING_SLOT_TEX0 + i][2],
                        v->attrib[VARYING_SLOT_TEX0 + i][3]);

#if CHAN_TYPE == GL_FLOAT
      _mesa_debug(ctx, "color %f %f %f %f\n",
                  v->color[0], v->color[1], v->color[2], v->color[3]);
#else
      _mesa_debug(ctx, "color %d %d %d %d\n",
                  v->color[0], v->color[1], v->color[2], v->color[3]);
#endif
      _mesa_debug(ctx, "spec %g %g %g %g\n",
                  v->attrib[VARYING_SLOT_COL1][0],
                  v->attrib[VARYING_SLOT_COL1][1],
                  v->attrib[VARYING_SLOT_COL1][2],
                  v->attrib[VARYING_SLOT_COL1][3]);
      _mesa_debug(ctx, "fog %f\n", v->attrib[VARYING_SLOT_FOGC][0]);
      _mesa_debug(ctx, "index %f\n", v->attrib[VARYING_SLOT_CI][0]);
      _mesa_debug(ctx, "pointsize %f\n", v->pointSize);
      _mesa_debug(ctx, "\n");
   }
}
