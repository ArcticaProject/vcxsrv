/*
 * Copyright © 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "ir.h"
#include "glsl_parser_extras.h"
#include "glsl_symbol_table.h"
#include "builtin_variables.h"

static void generate_ARB_draw_buffers_variables(exec_list *,
						struct _mesa_glsl_parse_state *,
						bool, _mesa_glsl_parser_targets);

static void
generate_ARB_draw_instanced_variables(exec_list *,
                                      struct _mesa_glsl_parse_state *,
                                      bool, _mesa_glsl_parser_targets);

static ir_variable *
add_variable(exec_list *instructions, glsl_symbol_table *symtab,
	     const char *name, const glsl_type *type,
	     enum ir_variable_mode mode, int slot)
{
   ir_variable *var = new(symtab) ir_variable(type, name, mode);

   switch (var->mode) {
   case ir_var_auto:
   case ir_var_in:
   case ir_var_const_in:
   case ir_var_uniform:
   case ir_var_system_value:
      var->read_only = true;
      break;
   case ir_var_inout:
   case ir_var_out:
      break;
   default:
      assert(0);
      break;
   }

   var->location = slot;
   var->explicit_location = (slot >= 0);

   /* Once the variable is created an initialized, add it to the symbol table
    * and add the declaration to the IR stream.
    */
   instructions->push_tail(var);

   symtab->add_variable(var);
   return var;
}

static ir_variable *
add_uniform(exec_list *instructions, glsl_symbol_table *symtab,
	    const char *name, const glsl_type *type)
{
   return add_variable(instructions, symtab, name, type, ir_var_uniform, -1);
}

static void
add_builtin_variable(exec_list *instructions, glsl_symbol_table *symtab,
		     const builtin_variable *proto)
{
   /* Create a new variable declaration from the description supplied by
    * the caller.
    */
   const glsl_type *const type = symtab->get_type(proto->type);

   assert(type != NULL);

   add_variable(instructions, symtab, proto->name, type, proto->mode,
		proto->slot);
}

static void
add_builtin_constant(exec_list *instructions, glsl_symbol_table *symtab,
		     const char *name, int value)
{
   ir_variable *const var = add_variable(instructions, symtab,
					 name, glsl_type::int_type,
					 ir_var_auto, -1);
   var->constant_value = new(var) ir_constant(value);
}

/* Several constants in GLSL ES have different names than normal desktop GLSL.
 * Therefore, this function should only be called on the ES path.
 */
static void
generate_100ES_uniforms(exec_list *instructions,
		     struct _mesa_glsl_parse_state *state)
{
   glsl_symbol_table *const symtab = state->symbols;

   add_builtin_constant(instructions, symtab, "gl_MaxVertexAttribs",
			state->Const.MaxVertexAttribs);
   add_builtin_constant(instructions, symtab, "gl_MaxVertexUniformVectors",
			state->Const.MaxVertexUniformComponents);
   add_builtin_constant(instructions, symtab, "gl_MaxVaryingVectors",
			state->Const.MaxVaryingFloats / 4);
   add_builtin_constant(instructions, symtab, "gl_MaxVertexTextureImageUnits",
			state->Const.MaxVertexTextureImageUnits);
   add_builtin_constant(instructions, symtab, "gl_MaxCombinedTextureImageUnits",
			state->Const.MaxCombinedTextureImageUnits);
   add_builtin_constant(instructions, symtab, "gl_MaxTextureImageUnits",
			state->Const.MaxTextureImageUnits);
   add_builtin_constant(instructions, symtab, "gl_MaxFragmentUniformVectors",
			state->Const.MaxFragmentUniformComponents);

   add_uniform(instructions, symtab, "gl_DepthRange",
	       state->symbols->get_type("gl_DepthRangeParameters"));
}

static void
generate_110_uniforms(exec_list *instructions,
		      struct _mesa_glsl_parse_state *state)
{
   glsl_symbol_table *const symtab = state->symbols;

   for (unsigned i = 0
	   ; i < Elements(builtin_110_deprecated_uniforms)
	   ; i++) {
      add_builtin_variable(instructions, symtab,
			   & builtin_110_deprecated_uniforms[i]);
   }

   add_builtin_constant(instructions, symtab, "gl_MaxLights",
			state->Const.MaxLights);
   add_builtin_constant(instructions, symtab, "gl_MaxClipPlanes",
			state->Const.MaxClipPlanes);
   add_builtin_constant(instructions, symtab, "gl_MaxTextureUnits",
			state->Const.MaxTextureUnits);
   add_builtin_constant(instructions, symtab, "gl_MaxTextureCoords",
			state->Const.MaxTextureCoords);
   add_builtin_constant(instructions, symtab, "gl_MaxVertexAttribs",
			state->Const.MaxVertexAttribs);
   add_builtin_constant(instructions, symtab, "gl_MaxVertexUniformComponents",
			state->Const.MaxVertexUniformComponents);
   add_builtin_constant(instructions, symtab, "gl_MaxVaryingFloats",
			state->Const.MaxVaryingFloats);
   add_builtin_constant(instructions, symtab, "gl_MaxVertexTextureImageUnits",
			state->Const.MaxVertexTextureImageUnits);
   add_builtin_constant(instructions, symtab, "gl_MaxCombinedTextureImageUnits",
			state->Const.MaxCombinedTextureImageUnits);
   add_builtin_constant(instructions, symtab, "gl_MaxTextureImageUnits",
			state->Const.MaxTextureImageUnits);
   add_builtin_constant(instructions, symtab, "gl_MaxFragmentUniformComponents",
			state->Const.MaxFragmentUniformComponents);

   const glsl_type *const mat4_array_type =
      glsl_type::get_array_instance(glsl_type::mat4_type,
				    state->Const.MaxTextureCoords);

   add_uniform(instructions, symtab, "gl_TextureMatrix", mat4_array_type);
   add_uniform(instructions, symtab, "gl_TextureMatrixInverse", mat4_array_type);
   add_uniform(instructions, symtab, "gl_TextureMatrixTranspose", mat4_array_type);
   add_uniform(instructions, symtab, "gl_TextureMatrixInverseTranspose", mat4_array_type);

   add_uniform(instructions, symtab, "gl_DepthRange",
		symtab->get_type("gl_DepthRangeParameters"));

   add_uniform(instructions, symtab, "gl_ClipPlane",
	       glsl_type::get_array_instance(glsl_type::vec4_type,
					     state->Const.MaxClipPlanes));
   add_uniform(instructions, symtab, "gl_Point",
	       symtab->get_type("gl_PointParameters"));

   const glsl_type *const material_parameters_type =
      symtab->get_type("gl_MaterialParameters");
   add_uniform(instructions, symtab, "gl_FrontMaterial", material_parameters_type);
   add_uniform(instructions, symtab, "gl_BackMaterial", material_parameters_type);

   const glsl_type *const light_source_array_type =
      glsl_type::get_array_instance(symtab->get_type("gl_LightSourceParameters"), state->Const.MaxLights);

   add_uniform(instructions, symtab, "gl_LightSource", light_source_array_type);

   const glsl_type *const light_model_products_type =
      symtab->get_type("gl_LightModelProducts");
   add_uniform(instructions, symtab, "gl_FrontLightModelProduct",
	       light_model_products_type);
   add_uniform(instructions, symtab, "gl_BackLightModelProduct",
	       light_model_products_type);

   const glsl_type *const light_products_type =
      glsl_type::get_array_instance(symtab->get_type("gl_LightProducts"),
				    state->Const.MaxLights);
   add_uniform(instructions, symtab, "gl_FrontLightProduct", light_products_type);
   add_uniform(instructions, symtab, "gl_BackLightProduct", light_products_type);

   add_uniform(instructions, symtab, "gl_TextureEnvColor",
	       glsl_type::get_array_instance(glsl_type::vec4_type,
					     state->Const.MaxTextureUnits));

   const glsl_type *const texcoords_vec4 =
      glsl_type::get_array_instance(glsl_type::vec4_type,
				    state->Const.MaxTextureCoords);
   add_uniform(instructions, symtab, "gl_EyePlaneS", texcoords_vec4);
   add_uniform(instructions, symtab, "gl_EyePlaneT", texcoords_vec4);
   add_uniform(instructions, symtab, "gl_EyePlaneR", texcoords_vec4);
   add_uniform(instructions, symtab, "gl_EyePlaneQ", texcoords_vec4);
   add_uniform(instructions, symtab, "gl_ObjectPlaneS", texcoords_vec4);
   add_uniform(instructions, symtab, "gl_ObjectPlaneT", texcoords_vec4);
   add_uniform(instructions, symtab, "gl_ObjectPlaneR", texcoords_vec4);
   add_uniform(instructions, symtab, "gl_ObjectPlaneQ", texcoords_vec4);

   add_uniform(instructions, symtab, "gl_Fog",
	       symtab->get_type("gl_FogParameters"));
}

/* This function should only be called for ES, not desktop GL. */
static void
generate_100ES_vs_variables(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state)
{
   for (unsigned i = 0; i < Elements(builtin_core_vs_variables); i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_core_vs_variables[i]);
   }

   generate_100ES_uniforms(instructions, state);

   generate_ARB_draw_buffers_variables(instructions, state, false,
				       vertex_shader);
}


static void
generate_110_vs_variables(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state)
{
   for (unsigned i = 0; i < Elements(builtin_core_vs_variables); i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_core_vs_variables[i]);
   }

   for (unsigned i = 0
	   ; i < Elements(builtin_110_deprecated_vs_variables)
	   ; i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_110_deprecated_vs_variables[i]);
   }
   generate_110_uniforms(instructions, state);

   /* From page 54 (page 60 of the PDF) of the GLSL 1.20 spec:
    *
    *     "As with all arrays, indices used to subscript gl_TexCoord must
    *     either be an integral constant expressions, or this array must be
    *     re-declared by the shader with a size. The size can be at most
    *     gl_MaxTextureCoords. Using indexes close to 0 may aid the
    *     implementation in preserving varying resources."
    */
   const glsl_type *const vec4_array_type =
      glsl_type::get_array_instance(glsl_type::vec4_type, 0);

   add_variable(instructions, state->symbols,
		"gl_TexCoord", vec4_array_type, ir_var_out, VERT_RESULT_TEX0);

   generate_ARB_draw_buffers_variables(instructions, state, false,
				       vertex_shader);
}


static void
generate_120_vs_variables(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state)
{
   /* GLSL version 1.20 did not add any built-in variables in the vertex
    * shader.
    */
   generate_110_vs_variables(instructions, state);
}


static void
generate_130_vs_variables(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state)
{
   generate_120_vs_variables(instructions, state);

   for (unsigned i = 0; i < Elements(builtin_130_vs_variables); i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_130_vs_variables[i]);
   }

   const glsl_type *const clip_distance_array_type =
      glsl_type::get_array_instance(glsl_type::float_type,
				    state->Const.MaxClipPlanes);

   /* FINISHME: gl_ClipDistance needs a real location assigned. */
   add_variable(instructions, state->symbols,
		"gl_ClipDistance", clip_distance_array_type, ir_var_out, -1);

}


static void
initialize_vs_variables(exec_list *instructions,
			struct _mesa_glsl_parse_state *state)
{

   switch (state->language_version) {
   case 100:
      generate_100ES_vs_variables(instructions, state);
      break;
   case 110:
      generate_110_vs_variables(instructions, state);
      break;
   case 120:
      generate_120_vs_variables(instructions, state);
      break;
   case 130:
      generate_130_vs_variables(instructions, state);
      break;
   }

   if (state->ARB_draw_instanced_enable)
      generate_ARB_draw_instanced_variables(instructions, state, false,
                                            vertex_shader);
}


/* This function should only be called for ES, not desktop GL. */
static void
generate_100ES_fs_variables(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state)
{
   for (unsigned i = 0; i < Elements(builtin_core_fs_variables); i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_core_fs_variables[i]);
   }

   for (unsigned i = 0; i < Elements(builtin_100ES_fs_variables); i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_100ES_fs_variables[i]);
   }

   generate_100ES_uniforms(instructions, state);

   generate_ARB_draw_buffers_variables(instructions, state, false,
				       fragment_shader);
}

static void
generate_110_fs_variables(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state)
{
   for (unsigned i = 0; i < Elements(builtin_core_fs_variables); i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_core_fs_variables[i]);
   }

   for (unsigned i = 0; i < Elements(builtin_110_fs_variables); i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_110_fs_variables[i]);
   }

   for (unsigned i = 0
	   ; i < Elements(builtin_110_deprecated_fs_variables)
	   ; i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_110_deprecated_fs_variables[i]);
   }
   generate_110_uniforms(instructions, state);

   /* From page 54 (page 60 of the PDF) of the GLSL 1.20 spec:
    *
    *     "As with all arrays, indices used to subscript gl_TexCoord must
    *     either be an integral constant expressions, or this array must be
    *     re-declared by the shader with a size. The size can be at most
    *     gl_MaxTextureCoords. Using indexes close to 0 may aid the
    *     implementation in preserving varying resources."
    */
   const glsl_type *const vec4_array_type =
      glsl_type::get_array_instance(glsl_type::vec4_type, 0);

   add_variable(instructions, state->symbols,
		"gl_TexCoord", vec4_array_type, ir_var_in, FRAG_ATTRIB_TEX0);

   generate_ARB_draw_buffers_variables(instructions, state, false,
				       fragment_shader);
}


static void
generate_ARB_draw_buffers_variables(exec_list *instructions,
				    struct _mesa_glsl_parse_state *state,
				    bool warn, _mesa_glsl_parser_targets target)
{
   /* gl_MaxDrawBuffers is available in all shader stages.
    */
   ir_variable *const mdb =
      add_variable(instructions, state->symbols,
		   "gl_MaxDrawBuffers", glsl_type::int_type, ir_var_auto, -1);

   if (warn)
      mdb->warn_extension = "GL_ARB_draw_buffers";

   mdb->constant_value = new(mdb)
      ir_constant(int(state->Const.MaxDrawBuffers));


   /* gl_FragData is only available in the fragment shader.
    */
   if (target == fragment_shader) {
      const glsl_type *const vec4_array_type =
	 glsl_type::get_array_instance(glsl_type::vec4_type,
				       state->Const.MaxDrawBuffers);

      ir_variable *const fd =
	 add_variable(instructions, state->symbols,
		      "gl_FragData", vec4_array_type,
		      ir_var_out, FRAG_RESULT_DATA0);

      if (warn)
	 fd->warn_extension = "GL_ARB_draw_buffers";
   }
}


static void
generate_ARB_draw_instanced_variables(exec_list *instructions,
                                      struct _mesa_glsl_parse_state *state,
                                      bool warn,
                                      _mesa_glsl_parser_targets target)
{
   /* gl_InstanceIDARB is only available in the vertex shader.
    */
   if (target == vertex_shader) {
      ir_variable *const inst =
         add_variable(instructions, state->symbols,
		      "gl_InstanceIDARB", glsl_type::int_type,
		      ir_var_system_value, SYSTEM_VALUE_INSTANCE_ID);

      if (warn)
         inst->warn_extension = "GL_ARB_draw_instanced";
   }
}


static void
generate_ARB_shader_stencil_export_variables(exec_list *instructions,
					     struct _mesa_glsl_parse_state *state,
					     bool warn)
{
   /* gl_FragStencilRefARB is only available in the fragment shader.
    */
   ir_variable *const fd =
      add_variable(instructions, state->symbols,
		   "gl_FragStencilRefARB", glsl_type::int_type,
		   ir_var_out, FRAG_RESULT_STENCIL);

   if (warn)
      fd->warn_extension = "GL_ARB_shader_stencil_export";
}

static void
generate_120_fs_variables(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state)
{
   generate_110_fs_variables(instructions, state);

   for (unsigned i = 0
	   ; i < Elements(builtin_120_fs_variables)
	   ; i++) {
      add_builtin_variable(instructions, state->symbols,
			   & builtin_120_fs_variables[i]);
   }
}

static void
generate_130_fs_variables(exec_list *instructions,
			  struct _mesa_glsl_parse_state *state)
{
   generate_120_fs_variables(instructions, state);

   const glsl_type *const clip_distance_array_type =
      glsl_type::get_array_instance(glsl_type::float_type,
				    state->Const.MaxClipPlanes);

   /* FINISHME: gl_ClipDistance needs a real location assigned. */
   add_variable(instructions, state->symbols,
		"gl_ClipDistance", clip_distance_array_type, ir_var_in, -1);
}

static void
initialize_fs_variables(exec_list *instructions,
			struct _mesa_glsl_parse_state *state)
{

   switch (state->language_version) {
   case 100:
      generate_100ES_fs_variables(instructions, state);
      break;
   case 110:
      generate_110_fs_variables(instructions, state);
      break;
   case 120:
      generate_120_fs_variables(instructions, state);
      break;
   case 130:
      generate_130_fs_variables(instructions, state);
      break;
   }

   if (state->ARB_shader_stencil_export_enable)
      generate_ARB_shader_stencil_export_variables(instructions, state,
						   state->ARB_shader_stencil_export_warn);
}

void
_mesa_glsl_initialize_variables(exec_list *instructions,
				struct _mesa_glsl_parse_state *state)
{
   switch (state->target) {
   case vertex_shader:
      initialize_vs_variables(instructions, state);
      break;
   case geometry_shader:
      break;
   case fragment_shader:
      initialize_fs_variables(instructions, state);
      break;
   }
}
