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
 */


/**
 * \file state.c
 * State management.
 * 
 * This file manages recalculation of derived values in struct gl_context.
 */


#include "glheader.h"
#include "mtypes.h"
#include "arrayobj.h"
#include "context.h"
#include "debug.h"
#include "macros.h"
#include "ffvertex_prog.h"
#include "framebuffer.h"
#include "light.h"
#include "matrix.h"
#include "pixel.h"
#include "program/program.h"
#include "program/prog_parameter.h"
#include "shaderobj.h"
#include "state.h"
#include "stencil.h"
#include "texenvprogram.h"
#include "texobj.h"
#include "texstate.h"
#include "varray.h"
#include "blend.h"


/**
 * Update the following fields:
 *   ctx->VertexProgram._Enabled
 *   ctx->FragmentProgram._Enabled
 *   ctx->ATIFragmentShader._Enabled
 * This needs to be done before texture state validation.
 */
static void
update_program_enables(struct gl_context *ctx)
{
   /* These _Enabled flags indicate if the user-defined ARB/NV vertex/fragment
    * program is enabled AND valid.  Similarly for ATI fragment shaders.
    * GLSL shaders not relevant here.
    */
   ctx->VertexProgram._Enabled = ctx->VertexProgram.Enabled
      && ctx->VertexProgram.Current->Base.Instructions;
   ctx->FragmentProgram._Enabled = ctx->FragmentProgram.Enabled
      && ctx->FragmentProgram.Current->Base.Instructions;
   ctx->ATIFragmentShader._Enabled = ctx->ATIFragmentShader.Enabled
      && ctx->ATIFragmentShader.Current->Instructions[0];
}


/**
 * Update the ctx->Vertex/Geometry/FragmentProgram._Current pointers to point
 * to the current/active programs.  Then call ctx->Driver.BindProgram() to
 * tell the driver which programs to use.
 *
 * Programs may come from 3 sources: GLSL shaders, ARB/NV_vertex/fragment
 * programs or programs derived from fixed-function state.
 *
 * This function needs to be called after texture state validation in case
 * we're generating a fragment program from fixed-function texture state.
 *
 * \return bitfield which will indicate _NEW_PROGRAM state if a new vertex
 * or fragment program is being used.
 */
static GLbitfield
update_program(struct gl_context *ctx)
{
   const struct gl_shader_program *vsProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX];
   const struct gl_shader_program *gsProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY];
   struct gl_shader_program *fsProg =
      ctx->_Shader->CurrentProgram[MESA_SHADER_FRAGMENT];
   const struct gl_vertex_program *prevVP = ctx->VertexProgram._Current;
   const struct gl_fragment_program *prevFP = ctx->FragmentProgram._Current;
   const struct gl_geometry_program *prevGP = ctx->GeometryProgram._Current;
   GLbitfield new_state = 0x0;

   /*
    * Set the ctx->VertexProgram._Current and ctx->FragmentProgram._Current
    * pointers to the programs that should be used for rendering.  If either
    * is NULL, use fixed-function code paths.
    *
    * These programs may come from several sources.  The priority is as
    * follows:
    *   1. OpenGL 2.0/ARB vertex/fragment shaders
    *   2. ARB/NV vertex/fragment programs
    *   3. Programs derived from fixed-function state.
    *
    * Note: it's possible for a vertex shader to get used with a fragment
    * program (and vice versa) here, but in practice that shouldn't ever
    * come up, or matter.
    */

   if (fsProg && fsProg->LinkStatus
       && fsProg->_LinkedShaders[MESA_SHADER_FRAGMENT]) {
      /* Use GLSL fragment shader */
      _mesa_reference_shader_program(ctx,
                                     &ctx->_Shader->_CurrentFragmentProgram,
				     fsProg);
      _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._Current,
                               gl_fragment_program(fsProg->_LinkedShaders[MESA_SHADER_FRAGMENT]->Program));
      _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._TexEnvProgram,
			       NULL);
   }
   else if (ctx->FragmentProgram._Enabled) {
      /* Use user-defined fragment program */
      _mesa_reference_shader_program(ctx,
                                     &ctx->_Shader->_CurrentFragmentProgram,
				     NULL);
      _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._Current,
                               ctx->FragmentProgram.Current);
      _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._TexEnvProgram,
			       NULL);
   }
   else if (ctx->FragmentProgram._MaintainTexEnvProgram) {
      /* Use fragment program generated from fixed-function state */
      struct gl_shader_program *f = _mesa_get_fixed_func_fragment_program(ctx);

      _mesa_reference_shader_program(ctx,
                                     &ctx->_Shader->_CurrentFragmentProgram,
				     f);
      _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._Current,
			       gl_fragment_program(f->_LinkedShaders[MESA_SHADER_FRAGMENT]->Program));
      _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._TexEnvProgram,
			       gl_fragment_program(f->_LinkedShaders[MESA_SHADER_FRAGMENT]->Program));
   }
   else {
      /* No fragment program */
      _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._Current, NULL);
      _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._TexEnvProgram,
			       NULL);
   }

   if (gsProg && gsProg->LinkStatus
       && gsProg->_LinkedShaders[MESA_SHADER_GEOMETRY]) {
      /* Use GLSL geometry shader */
      _mesa_reference_geomprog(ctx, &ctx->GeometryProgram._Current,
			       gl_geometry_program(gsProg->_LinkedShaders[MESA_SHADER_GEOMETRY]->Program));
   } else {
      /* No geometry program */
      _mesa_reference_geomprog(ctx, &ctx->GeometryProgram._Current, NULL);
   }

   /* Examine vertex program after fragment program as
    * _mesa_get_fixed_func_vertex_program() needs to know active
    * fragprog inputs.
    */
   if (vsProg && vsProg->LinkStatus
       && vsProg->_LinkedShaders[MESA_SHADER_VERTEX]) {
      /* Use GLSL vertex shader */
      _mesa_reference_vertprog(ctx, &ctx->VertexProgram._Current,
			       gl_vertex_program(vsProg->_LinkedShaders[MESA_SHADER_VERTEX]->Program));
   }
   else if (ctx->VertexProgram._Enabled) {
      /* Use user-defined vertex program */
      _mesa_reference_vertprog(ctx, &ctx->VertexProgram._Current,
                               ctx->VertexProgram.Current);
   }
   else if (ctx->VertexProgram._MaintainTnlProgram) {
      /* Use vertex program generated from fixed-function state */
      _mesa_reference_vertprog(ctx, &ctx->VertexProgram._Current,
                               _mesa_get_fixed_func_vertex_program(ctx));
      _mesa_reference_vertprog(ctx, &ctx->VertexProgram._TnlProgram,
                               ctx->VertexProgram._Current);
   }
   else {
      /* no vertex program */
      _mesa_reference_vertprog(ctx, &ctx->VertexProgram._Current, NULL);
   }

   /* Let the driver know what's happening:
    */
   if (ctx->FragmentProgram._Current != prevFP) {
      new_state |= _NEW_PROGRAM;
      if (ctx->Driver.BindProgram) {
         ctx->Driver.BindProgram(ctx, GL_FRAGMENT_PROGRAM_ARB,
                          (struct gl_program *) ctx->FragmentProgram._Current);
      }
   }

   if (ctx->GeometryProgram._Current != prevGP) {
      new_state |= _NEW_PROGRAM;
      if (ctx->Driver.BindProgram) {
         ctx->Driver.BindProgram(ctx, MESA_GEOMETRY_PROGRAM,
                            (struct gl_program *) ctx->GeometryProgram._Current);
      }
   }

   if (ctx->VertexProgram._Current != prevVP) {
      new_state |= _NEW_PROGRAM;
      if (ctx->Driver.BindProgram) {
         ctx->Driver.BindProgram(ctx, GL_VERTEX_PROGRAM_ARB,
                            (struct gl_program *) ctx->VertexProgram._Current);
      }
   }

   return new_state;
}


/**
 * Examine shader constants and return either _NEW_PROGRAM_CONSTANTS or 0.
 */
static GLbitfield
update_program_constants(struct gl_context *ctx)
{
   GLbitfield new_state = 0x0;

   if (ctx->FragmentProgram._Current) {
      const struct gl_program_parameter_list *params =
         ctx->FragmentProgram._Current->Base.Parameters;
      if (params && params->StateFlags & ctx->NewState) {
         new_state |= _NEW_PROGRAM_CONSTANTS;
      }
   }

   if (ctx->GeometryProgram._Current) {
      const struct gl_program_parameter_list *params =
         ctx->GeometryProgram._Current->Base.Parameters;
      /*FIXME: StateFlags is always 0 because we have unnamed constant
       *       not state changes */
      if (params /*&& params->StateFlags & ctx->NewState*/) {
         new_state |= _NEW_PROGRAM_CONSTANTS;
      }
   }

   if (ctx->VertexProgram._Current) {
      const struct gl_program_parameter_list *params =
         ctx->VertexProgram._Current->Base.Parameters;
      if (params && params->StateFlags & ctx->NewState) {
         new_state |= _NEW_PROGRAM_CONSTANTS;
      }
   }

   return new_state;
}




static void
update_viewport_matrix(struct gl_context *ctx)
{
   const GLfloat depthMax = ctx->DrawBuffer->_DepthMaxF;
   unsigned i;

   ASSERT(depthMax > 0);

   /* Compute scale and bias values. This is really driver-specific
    * and should be maintained elsewhere if at all.
    * NOTE: RasterPos uses this.
    */
   for (i = 0; i < ctx->Const.MaxViewports; i++) {
      _math_matrix_viewport(&ctx->ViewportArray[i]._WindowMap,
                            ctx->ViewportArray[i].X, ctx->ViewportArray[i].Y,
                            ctx->ViewportArray[i].Width, ctx->ViewportArray[i].Height,
                            ctx->ViewportArray[i].Near, ctx->ViewportArray[i].Far,
                            depthMax);
   }
}


/**
 * Update derived multisample state.
 */
static void
update_multisample(struct gl_context *ctx)
{
   ctx->Multisample._Enabled = GL_FALSE;
   if (ctx->Multisample.Enabled &&
       ctx->DrawBuffer &&
       ctx->DrawBuffer->Visual.sampleBuffers)
      ctx->Multisample._Enabled = GL_TRUE;
}


/**
 * Update the ctx->VertexProgram._TwoSideEnabled flag.
 */
static void
update_twoside(struct gl_context *ctx)
{
   if (ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX] ||
       ctx->VertexProgram._Enabled) {
      ctx->VertexProgram._TwoSideEnabled = ctx->VertexProgram.TwoSideEnabled;
   } else {
      ctx->VertexProgram._TwoSideEnabled = (ctx->Light.Enabled &&
					    ctx->Light.Model.TwoSide);
   }
}


/**
 * Compute derived GL state.
 * If __struct gl_contextRec::NewState is non-zero then this function \b must
 * be called before rendering anything.
 *
 * Calls dd_function_table::UpdateState to perform any internal state
 * management necessary.
 * 
 * \sa _mesa_update_modelview_project(), _mesa_update_texture(),
 * _mesa_update_buffer_bounds(),
 * _mesa_update_lighting() and _mesa_update_tnl_spaces().
 */
void
_mesa_update_state_locked( struct gl_context *ctx )
{
   GLbitfield new_state = ctx->NewState;
   GLbitfield prog_flags = _NEW_PROGRAM;
   GLbitfield new_prog_state = 0x0;

   if (new_state == _NEW_CURRENT_ATTRIB) 
      goto out;

   if (MESA_VERBOSE & VERBOSE_STATE)
      _mesa_print_state("_mesa_update_state", new_state);

   /* Determine which state flags effect vertex/fragment program state */
   if (ctx->FragmentProgram._MaintainTexEnvProgram) {
      prog_flags |= (_NEW_BUFFERS | _NEW_TEXTURE | _NEW_FOG |
		     _NEW_VARYING_VP_INPUTS | _NEW_LIGHT | _NEW_POINT |
		     _NEW_RENDERMODE | _NEW_PROGRAM | _NEW_FRAG_CLAMP |
		     _NEW_COLOR);
   }
   if (ctx->VertexProgram._MaintainTnlProgram) {
      prog_flags |= (_NEW_VARYING_VP_INPUTS | _NEW_TEXTURE |
                     _NEW_TEXTURE_MATRIX | _NEW_TRANSFORM | _NEW_POINT |
                     _NEW_FOG | _NEW_LIGHT |
                     _MESA_NEW_NEED_EYE_COORDS);
   }

   /*
    * Now update derived state info
    */

   if (new_state & prog_flags)
      update_program_enables( ctx );

   if (new_state & (_NEW_MODELVIEW|_NEW_PROJECTION))
      _mesa_update_modelview_project( ctx, new_state );

   if (new_state & (_NEW_PROGRAM|_NEW_TEXTURE|_NEW_TEXTURE_MATRIX))
      _mesa_update_texture( ctx, new_state );

   if (new_state & _NEW_BUFFERS)
      _mesa_update_framebuffer(ctx);

   if (new_state & (_NEW_SCISSOR | _NEW_BUFFERS | _NEW_VIEWPORT))
      _mesa_update_draw_buffer_bounds( ctx );

   if (new_state & _NEW_LIGHT)
      _mesa_update_lighting( ctx );

   if (new_state & (_NEW_LIGHT | _NEW_PROGRAM))
      update_twoside( ctx );

   if (new_state & (_NEW_STENCIL | _NEW_BUFFERS))
      _mesa_update_stencil( ctx );

   if (new_state & _NEW_PIXEL)
      _mesa_update_pixel( ctx, new_state );

   if (new_state & (_NEW_BUFFERS | _NEW_VIEWPORT))
      update_viewport_matrix(ctx);

   if (new_state & (_NEW_MULTISAMPLE | _NEW_BUFFERS))
      update_multisample( ctx );

   /* ctx->_NeedEyeCoords is now up to date.
    *
    * If the truth value of this variable has changed, update for the
    * new lighting space and recompute the positions of lights and the
    * normal transform.
    *
    * If the lighting space hasn't changed, may still need to recompute
    * light positions & normal transforms for other reasons.
    */
   if (new_state & _MESA_NEW_NEED_EYE_COORDS) 
      _mesa_update_tnl_spaces( ctx, new_state );

   if (new_state & prog_flags) {
      /* When we generate programs from fixed-function vertex/fragment state
       * this call may generate/bind a new program.  If so, we need to
       * propogate the _NEW_PROGRAM flag to the driver.
       */
      new_prog_state |= update_program( ctx );
   }

   if (new_state & _NEW_ARRAY)
      _mesa_update_vao_client_arrays(ctx, ctx->Array.VAO);

   if (ctx->Const.CheckArrayBounds &&
       new_state & (_NEW_ARRAY | _NEW_PROGRAM | _NEW_BUFFER_OBJECT)) {
      _mesa_update_vao_max_element(ctx, ctx->Array.VAO);
   }

 out:
   new_prog_state |= update_program_constants(ctx);

   /*
    * Give the driver a chance to act upon the new_state flags.
    * The driver might plug in different span functions, for example.
    * Also, this is where the driver can invalidate the state of any
    * active modules (such as swrast_setup, swrast, tnl, etc).
    *
    * Set ctx->NewState to zero to avoid recursion if
    * Driver.UpdateState() has to call FLUSH_VERTICES().  (fixed?)
    */
   new_state = ctx->NewState | new_prog_state;
   ctx->NewState = 0;
   ctx->Driver.UpdateState(ctx, new_state);
   ctx->Array.VAO->NewArrays = 0x0;
}


/* This is the usual entrypoint for state updates:
 */
void
_mesa_update_state( struct gl_context *ctx )
{
   _mesa_lock_context_textures(ctx);
   _mesa_update_state_locked(ctx);
   _mesa_unlock_context_textures(ctx);
}




/**
 * Want to figure out which fragment program inputs are actually
 * constant/current values from ctx->Current.  These should be
 * referenced as a tracked state variable rather than a fragment
 * program input, to save the overhead of putting a constant value in
 * every submitted vertex, transferring it to hardware, interpolating
 * it across the triangle, etc...
 *
 * When there is a VP bound, just use vp->outputs.  But when we're
 * generating vp from fixed function state, basically want to
 * calculate:
 *
 * vp_out_2_fp_in( vp_in_2_vp_out( varying_inputs ) | 
 *                 potential_vp_outputs )
 *
 * Where potential_vp_outputs is calculated by looking at enabled
 * texgen, etc.
 * 
 * The generated fragment program should then only declare inputs that
 * may vary or otherwise differ from the ctx->Current values.
 * Otherwise, the fp should track them as state values instead.
 */
void
_mesa_set_varying_vp_inputs( struct gl_context *ctx,
                             GLbitfield64 varying_inputs )
{
   if (ctx->varying_vp_inputs != varying_inputs) {
      ctx->varying_vp_inputs = varying_inputs;

      /* Only the fixed-func generated programs need to use the flag
       * and the fixed-func fragment program uses it only if there is also
       * a fixed-func vertex program, so this only depends on the latter.
       *
       * It's okay to check the VP pointer here, because this is called after
       * _mesa_update_state in the vbo module. */
      if (ctx->VertexProgram._TnlProgram ||
          ctx->FragmentProgram._TexEnvProgram) {
         ctx->NewState |= _NEW_VARYING_VP_INPUTS;
      }
      /*printf("%s %x\n", __FUNCTION__, varying_inputs);*/
   }
}


/**
 * Used by drivers to tell core Mesa that the driver is going to
 * install/ use its own vertex program.  In particular, this will
 * prevent generated fragment programs from using state vars instead
 * of ordinary varyings/inputs.
 */
void
_mesa_set_vp_override(struct gl_context *ctx, GLboolean flag)
{
   if (ctx->VertexProgram._Overriden != flag) {
      ctx->VertexProgram._Overriden = flag;

      /* Set one of the bits which will trigger fragment program
       * regeneration:
       */
      ctx->NewState |= _NEW_PROGRAM;
   }
}
