/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2004-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009-2010  VMware, Inc.  All Rights Reserved.
 * Copyright © 2010, 2011 Intel Corporation
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

#include <stdlib.h>

#include "main/core.h"
#include "main/context.h"
#include "ir.h"
#include "ir_uniform.h"
#include "program/hash_table.h"
#include "../glsl/program.h"
#include "../glsl/ir_uniform.h"
#include "../glsl/glsl_parser_extras.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "uniforms.h"


extern "C" void GLAPIENTRY
_mesa_GetActiveUniform(GLuint program, GLuint index,
                       GLsizei maxLength, GLsizei *length, GLint *size,
                       GLenum *type, GLcharARB *nameOut)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glGetActiveUniform");

   if (!shProg)
      return;

   if (index >= shProg->NumUserUniformStorage) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetActiveUniform(index)");
      return;
   }

   const struct gl_uniform_storage *const uni = &shProg->UniformStorage[index];

   if (nameOut) {
      _mesa_get_uniform_name(uni, maxLength, length, nameOut);
   }

   if (size) {
      /* array_elements is zero for non-arrays, but the API requires that 1 be
       * returned.
       */
      *size = MAX2(1, uni->array_elements);
   }

   if (type) {
      *type = uni->type->gl_type;
   }
}

extern "C" void GLAPIENTRY
_mesa_GetActiveUniformsiv(GLuint program,
			  GLsizei uniformCount,
			  const GLuint *uniformIndices,
			  GLenum pname,
			  GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;
   GLsizei i;

   shProg = _mesa_lookup_shader_program_err(ctx, program, "glGetActiveUniform");
   if (!shProg)
      return;

   if (uniformCount < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetActiveUniformsiv(uniformCount < 0)");
      return;
   }

   for (i = 0; i < uniformCount; i++) {
      GLuint index = uniformIndices[i];

      if (index >= shProg->NumUserUniformStorage) {
	 _mesa_error(ctx, GL_INVALID_VALUE, "glGetActiveUniformsiv(index)");
	 return;
      }
   }

   for (i = 0; i < uniformCount; i++) {
      GLuint index = uniformIndices[i];
      const struct gl_uniform_storage *uni = &shProg->UniformStorage[index];

      switch (pname) {
      case GL_UNIFORM_TYPE:
	 params[i] = uni->type->gl_type;
	 break;

      case GL_UNIFORM_SIZE:
	 /* array_elements is zero for non-arrays, but the API requires that 1 be
	  * returned.
	  */
	 params[i] = MAX2(1, uni->array_elements);
	 break;

      case GL_UNIFORM_NAME_LENGTH:
	 params[i] = strlen(uni->name) + 1;

         /* Page 61 (page 73 of the PDF) in section 2.11 of the OpenGL ES 3.0
          * spec says:
          *
          *     "If the active uniform is an array, the uniform name returned
          *     in name will always be the name of the uniform array appended
          *     with "[0]"."
          */
         if (uni->array_elements != 0)
            params[i] += 3;
	 break;

      case GL_UNIFORM_BLOCK_INDEX:
	 params[i] = uni->block_index;
	 break;

      case GL_UNIFORM_OFFSET:
	 params[i] = uni->offset;
	 break;

      case GL_UNIFORM_ARRAY_STRIDE:
	 params[i] = uni->array_stride;
	 break;

      case GL_UNIFORM_MATRIX_STRIDE:
	 params[i] = uni->matrix_stride;
	 break;

      case GL_UNIFORM_IS_ROW_MAJOR:
	 params[i] = uni->row_major;
	 break;

      case GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX:
         if (!ctx->Extensions.ARB_shader_atomic_counters)
            goto invalid_enum;
         params[i] = uni->atomic_buffer_index;
         break;

      default:
         goto invalid_enum;
      }
   }

   return;

 invalid_enum:
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetActiveUniformsiv(pname)");
}

static struct gl_uniform_storage *
validate_uniform_parameters(struct gl_context *ctx,
			    struct gl_shader_program *shProg,
			    GLint location, GLsizei count,
			    unsigned *array_index,
			    const char *caller,
			    bool negative_one_is_not_valid)
{
   if (!shProg || !shProg->LinkStatus) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(program not linked)", caller);
      return NULL;
   }

   if (location == -1) {
      /* For glGetUniform, page 264 (page 278 of the PDF) of the OpenGL 2.1
       * spec says:
       *
       *     "The error INVALID_OPERATION is generated if program has not been
       *     linked successfully, or if location is not a valid location for
       *     program."
       *
       * For glUniform, page 82 (page 96 of the PDF) of the OpenGL 2.1 spec
       * says:
       *
       *     "If the value of location is -1, the Uniform* commands will
       *     silently ignore the data passed in, and the current uniform
       *     values will not be changed."
       *
       * Allowing -1 for the location parameter of glUniform allows
       * applications to avoid error paths in the case that, for example, some
       * uniform variable is removed by the compiler / linker after
       * optimization.  In this case, the new value of the uniform is dropped
       * on the floor.  For the case of glGetUniform, there is nothing
       * sensible to do for a location of -1.
       *
       * The negative_one_is_not_valid flag selects between the two behaviors.
       */
      if (negative_one_is_not_valid) {
	 _mesa_error(ctx, GL_INVALID_OPERATION, "%s(location=%d)",
		     caller, location);
      }

      return NULL;
   }

   /* From page 12 (page 26 of the PDF) of the OpenGL 2.1 spec:
    *
    *     "If a negative number is provided where an argument of type sizei or
    *     sizeiptr is specified, the error INVALID_VALUE is generated."
    */
   if (count < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(count < 0)", caller);
      return NULL;
   }

   /* Check that the given location is in bounds of uniform remap table. */
   if (location >= (GLint) shProg->NumUniformRemapTable) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(location=%d)",
                  caller, location);
      return NULL;
   }

   /* Page 82 (page 96 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "If any of the following conditions occur, an INVALID_OPERATION
    *     error is generated by the Uniform* commands, and no uniform values
    *     are changed:
    *
    *     ...
    *
    *         - if no variable with a location of location exists in the
    *           program object currently in use and location is not -1,
    *         - if count is greater than one, and the uniform declared in the
    *           shader is not an array variable,
    */
   if (location < -1 || !shProg->UniformRemapTable[location]) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(location=%d)",
                  caller, location);
      return NULL;
   }

   /* If the driver storage pointer in remap table is -1, we ignore silently.
    *
    * GL_ARB_explicit_uniform_location spec says:
    *     "What happens if Uniform* is called with an explicitly defined
    *     uniform location, but that uniform is deemed inactive by the
    *     linker?
    *
    *     RESOLVED: The call is ignored for inactive uniform variables and
    *     no error is generated."
    *
    */
   if (shProg->UniformRemapTable[location] ==
       INACTIVE_UNIFORM_EXPLICIT_LOCATION)
      return NULL;

   struct gl_uniform_storage *const uni = shProg->UniformRemapTable[location];

   if (uni->array_elements == 0 && count > 1) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
		  "%s(count > 1 for non-array, location=%d)",
		  caller, location);
      return NULL;
   }

   /* The array index specified by the uniform location is just the uniform
    * location minus the base location of of the uniform.
    */
   *array_index = location - uni->remap_location;

   /* If the uniform is an array, check that array_index is in bounds.
    * If not an array, check that array_index is zero.
    * array_index is unsigned so no need to check for less than zero.
    */
   const unsigned limit = MAX2(uni->array_elements, 1);
   if (*array_index >= limit) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(location=%d)",
		  caller, location);
      return NULL;
   }
   return uni;
}

/**
 * Called via glGetUniform[fiui]v() to get the current value of a uniform.
 */
extern "C" void
_mesa_get_uniform(struct gl_context *ctx, GLuint program, GLint location,
		  GLsizei bufSize, enum glsl_base_type returnType,
		  GLvoid *paramsOut)
{
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program, "glGetUniformfv");
   unsigned offset;

   struct gl_uniform_storage *const uni =
      validate_uniform_parameters(ctx, shProg, location, 1,
                                  &offset, "glGetUniform", true);
   if (uni == NULL)
      return;

   {
      unsigned elements = (uni->type->is_sampler())
	 ? 1 : uni->type->components();

      /* Calculate the source base address *BEFORE* modifying elements to
       * account for the size of the user's buffer.
       */
      const union gl_constant_value *const src =
	 &uni->storage[offset * elements];

      assert(returnType == GLSL_TYPE_FLOAT || returnType == GLSL_TYPE_INT ||
             returnType == GLSL_TYPE_UINT);
      /* The three (currently) supported types all have the same size,
       * which is of course the same as their union. That'll change
       * with glGetUniformdv()...
       */
      unsigned bytes = sizeof(src[0]) * elements;
      if (bufSize < 0 || bytes > (unsigned) bufSize) {
	 _mesa_error( ctx, GL_INVALID_OPERATION,
	             "glGetnUniform*vARB(out of bounds: bufSize is %d,"
	             " but %u bytes are required)", bufSize, bytes );
	 return;
      }

      /* If the return type and the uniform's native type are "compatible,"
       * just memcpy the data.  If the types are not compatible, perform a
       * slower convert-and-copy process.
       */
      if (returnType == uni->type->base_type
	  || ((returnType == GLSL_TYPE_INT
	       || returnType == GLSL_TYPE_UINT
	       || returnType == GLSL_TYPE_SAMPLER)
	      &&
	      (uni->type->base_type == GLSL_TYPE_INT
	       || uni->type->base_type == GLSL_TYPE_UINT
	       || uni->type->base_type == GLSL_TYPE_SAMPLER))) {
	 memcpy(paramsOut, src, bytes);
      } else {
	 union gl_constant_value *const dst =
	    (union gl_constant_value *) paramsOut;

	 /* This code could be optimized by putting the loop inside the switch
	  * statements.  However, this is not expected to be
	  * performance-critical code.
	  */
	 for (unsigned i = 0; i < elements; i++) {
	    switch (returnType) {
	    case GLSL_TYPE_FLOAT:
	       switch (uni->type->base_type) {
	       case GLSL_TYPE_UINT:
		  dst[i].f = (float) src[i].u;
		  break;
	       case GLSL_TYPE_INT:
	       case GLSL_TYPE_SAMPLER:
		  dst[i].f = (float) src[i].i;
		  break;
	       case GLSL_TYPE_BOOL:
		  dst[i].f = src[i].i ? 1.0f : 0.0f;
		  break;
	       default:
		  assert(!"Should not get here.");
		  break;
	       }
	       break;

	    case GLSL_TYPE_INT:
	    case GLSL_TYPE_UINT:
	       switch (uni->type->base_type) {
	       case GLSL_TYPE_FLOAT:
		  /* While the GL 3.2 core spec doesn't explicitly
		   * state how conversion of float uniforms to integer
		   * values works, in section 6.2 "State Tables" on
		   * page 267 it says:
		   *
		   *     "Unless otherwise specified, when floating
		   *      point state is returned as integer values or
		   *      integer state is returned as floating-point
		   *      values it is converted in the fashion
		   *      described in section 6.1.2"
		   *
		   * That section, on page 248, says:
		   *
		   *     "If GetIntegerv or GetInteger64v are called,
		   *      a floating-point value is rounded to the
		   *      nearest integer..."
		   */
		  dst[i].i = IROUND(src[i].f);
		  break;
	       case GLSL_TYPE_BOOL:
		  dst[i].i = src[i].i ? 1 : 0;
		  break;
	       default:
		  assert(!"Should not get here.");
		  break;
	       }
	       break;

	    default:
	       assert(!"Should not get here.");
	       break;
	    }
	 }
      }
   }
}

static void
log_uniform(const void *values, enum glsl_base_type basicType,
	    unsigned rows, unsigned cols, unsigned count,
	    bool transpose,
	    const struct gl_shader_program *shProg,
	    GLint location,
	    const struct gl_uniform_storage *uni)
{

   const union gl_constant_value *v = (const union gl_constant_value *) values;
   const unsigned elems = rows * cols * count;
   const char *const extra = (cols == 1) ? "uniform" : "uniform matrix";

   printf("Mesa: set program %u %s \"%s\" (loc %d, type \"%s\", "
	  "transpose = %s) to: ",
	  shProg->Name, extra, uni->name, location, uni->type->name,
	  transpose ? "true" : "false");
   for (unsigned i = 0; i < elems; i++) {
      if (i != 0 && ((i % rows) == 0))
	 printf(", ");

      switch (basicType) {
      case GLSL_TYPE_UINT:
	 printf("%u ", v[i].u);
	 break;
      case GLSL_TYPE_INT:
	 printf("%d ", v[i].i);
	 break;
      case GLSL_TYPE_FLOAT:
	 printf("%g ", v[i].f);
	 break;
      default:
	 assert(!"Should not get here.");
	 break;
      }
   }
   printf("\n");
   fflush(stdout);
}

#if 0
static void
log_program_parameters(const struct gl_shader_program *shProg)
{
   for (unsigned i = 0; i < MESA_SHADER_STAGES; i++) {
      if (shProg->_LinkedShaders[i] == NULL)
	 continue;

      const struct gl_program *const prog = shProg->_LinkedShaders[i]->Program;

      printf("Program %d %s shader parameters:\n",
             shProg->Name, _mesa_shader_stage_to_string(i));
      for (unsigned j = 0; j < prog->Parameters->NumParameters; j++) {
	 printf("%s: %p %f %f %f %f\n",
		prog->Parameters->Parameters[j].Name,
		prog->Parameters->ParameterValues[j],
		prog->Parameters->ParameterValues[j][0].f,
		prog->Parameters->ParameterValues[j][1].f,
		prog->Parameters->ParameterValues[j][2].f,
		prog->Parameters->ParameterValues[j][3].f);
      }
   }
   fflush(stdout);
}
#endif

/**
 * Propagate some values from uniform backing storage to driver storage
 *
 * Values propagated from uniform backing storage to driver storage
 * have all format / type conversions previously requested by the
 * driver applied.  This function is most often called by the
 * implementations of \c glUniform1f, etc. and \c glUniformMatrix2f,
 * etc.
 *
 * \param uni          Uniform whose data is to be propagated to driver storage
 * \param array_index  If \c uni is an array, this is the element of
 *                     the array to be propagated.
 * \param count        Number of array elements to propagate.
 */
extern "C" void
_mesa_propagate_uniforms_to_driver_storage(struct gl_uniform_storage *uni,
					   unsigned array_index,
					   unsigned count)
{
   unsigned i;

   /* vector_elements and matrix_columns can be 0 for samplers.
    */
   const unsigned components = MAX2(1, uni->type->vector_elements);
   const unsigned vectors = MAX2(1, uni->type->matrix_columns);

   /* Store the data in the driver's requested type in the driver's storage
    * areas.
    */
   unsigned src_vector_byte_stride = components * 4;

   for (i = 0; i < uni->num_driver_storage; i++) {
      struct gl_uniform_driver_storage *const store = &uni->driver_storage[i];
      uint8_t *dst = (uint8_t *) store->data;
      const unsigned extra_stride =
	 store->element_stride - (vectors * store->vector_stride);
      const uint8_t *src =
	 (uint8_t *) (&uni->storage[array_index * (components * vectors)].i);

#if 0
      printf("%s: %p[%d] components=%u vectors=%u count=%u vector_stride=%u "
	     "extra_stride=%u\n",
	     __func__, dst, array_index, components,
	     vectors, count, store->vector_stride, extra_stride);
#endif

      dst += array_index * store->element_stride;

      switch (store->format) {
      case uniform_native:
      case uniform_bool_int_0_1: {
	 unsigned j;
	 unsigned v;

	 for (j = 0; j < count; j++) {
	    for (v = 0; v < vectors; v++) {
	       memcpy(dst, src, src_vector_byte_stride);
	       src += src_vector_byte_stride;
	       dst += store->vector_stride;
	    }

	    dst += extra_stride;
	 }
	 break;
      }

      case uniform_int_float:
      case uniform_bool_float: {
	 const int *isrc = (const int *) src;
	 unsigned j;
	 unsigned v;
	 unsigned c;

	 for (j = 0; j < count; j++) {
	    for (v = 0; v < vectors; v++) {
	       for (c = 0; c < components; c++) {
		  ((float *) dst)[c] = (float) *isrc;
		  isrc++;
	       }

	       dst += store->vector_stride;
	    }

	    dst += extra_stride;
	 }
	 break;
      }

      case uniform_bool_int_0_not0: {
	 const int *isrc = (const int *) src;
	 unsigned j;
	 unsigned v;
	 unsigned c;

	 for (j = 0; j < count; j++) {
	    for (v = 0; v < vectors; v++) {
	       for (c = 0; c < components; c++) {
		  ((int *) dst)[c] = *isrc == 0 ? 0 : ~0;
		  isrc++;
	       }

	       dst += store->vector_stride;
	    }

	    dst += extra_stride;
	 }
	 break;
      }

      default:
	 assert(!"Should not get here.");
	 break;
      }
   }
}

/**
 * Called via glUniform*() functions.
 */
extern "C" void
_mesa_uniform(struct gl_context *ctx, struct gl_shader_program *shProg,
	      GLint location, GLsizei count,
              const GLvoid *values, GLenum type)
{
   unsigned offset;
   unsigned components;
   unsigned src_components;
   enum glsl_base_type basicType;

   struct gl_uniform_storage *const uni =
      validate_uniform_parameters(ctx, shProg, location, count,
                                  &offset, "glUniform", false);
   if (uni == NULL)
      return;

   /* Verify that the types are compatible.
    */
   switch (type) {
   case GL_FLOAT:
      basicType = GLSL_TYPE_FLOAT;
      src_components = 1;
      break;
   case GL_FLOAT_VEC2:
      basicType = GLSL_TYPE_FLOAT;
      src_components = 2;
      break;
   case GL_FLOAT_VEC3:
      basicType = GLSL_TYPE_FLOAT;
      src_components = 3;
      break;
   case GL_FLOAT_VEC4:
      basicType = GLSL_TYPE_FLOAT;
      src_components = 4;
      break;
   case GL_UNSIGNED_INT:
      basicType = GLSL_TYPE_UINT;
      src_components = 1;
      break;
   case GL_UNSIGNED_INT_VEC2:
      basicType = GLSL_TYPE_UINT;
      src_components = 2;
      break;
   case GL_UNSIGNED_INT_VEC3:
      basicType = GLSL_TYPE_UINT;
      src_components = 3;
      break;
   case GL_UNSIGNED_INT_VEC4:
      basicType = GLSL_TYPE_UINT;
      src_components = 4;
      break;
   case GL_INT:
      basicType = GLSL_TYPE_INT;
      src_components = 1;
      break;
   case GL_INT_VEC2:
      basicType = GLSL_TYPE_INT;
      src_components = 2;
      break;
   case GL_INT_VEC3:
      basicType = GLSL_TYPE_INT;
      src_components = 3;
      break;
   case GL_INT_VEC4:
      basicType = GLSL_TYPE_INT;
      src_components = 4;
      break;
   case GL_BOOL:
   case GL_BOOL_VEC2:
   case GL_BOOL_VEC3:
   case GL_BOOL_VEC4:
   case GL_FLOAT_MAT2:
   case GL_FLOAT_MAT2x3:
   case GL_FLOAT_MAT2x4:
   case GL_FLOAT_MAT3x2:
   case GL_FLOAT_MAT3:
   case GL_FLOAT_MAT3x4:
   case GL_FLOAT_MAT4x2:
   case GL_FLOAT_MAT4x3:
   case GL_FLOAT_MAT4:
   default:
      _mesa_problem(NULL, "Invalid type in %s", __func__);
      return;
   }

   if (uni->type->is_sampler()) {
      components = 1;
   } else {
      components = uni->type->vector_elements;
   }

   bool match;
   switch (uni->type->base_type) {
   case GLSL_TYPE_BOOL:
      match = true;
      break;
   case GLSL_TYPE_SAMPLER:
   case GLSL_TYPE_IMAGE:
      match = (basicType == GLSL_TYPE_INT);
      break;
   default:
      match = (basicType == uni->type->base_type);
      break;
   }

   if (uni->type->is_matrix() || components != src_components || !match) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glUniform(type mismatch)");
      return;
   }

   if (ctx->_Shader->Flags & GLSL_UNIFORMS) {
      log_uniform(values, basicType, components, 1, count,
		  false, shProg, location, uni);
   }

   /* Page 100 (page 116 of the PDF) of the OpenGL 3.0 spec says:
    *
    *     "Setting a sampler's value to i selects texture image unit number
    *     i. The values of i range from zero to the implementation- dependent
    *     maximum supported number of texture image units."
    *
    * In addition, table 2.3, "Summary of GL errors," on page 17 (page 33 of
    * the PDF) says:
    *
    *     "Error         Description                    Offending command
    *                                                   ignored?
    *     ...
    *     INVALID_VALUE  Numeric argument out of range  Yes"
    *
    * Based on that, when an invalid sampler is specified, we generate a
    * GL_INVALID_VALUE error and ignore the command.
    */
   if (uni->type->is_sampler()) {
      int i;

      for (i = 0; i < count; i++) {
	 const unsigned texUnit = ((unsigned *) values)[i];

         /* check that the sampler (tex unit index) is legal */
         if (texUnit >= ctx->Const.MaxCombinedTextureImageUnits) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glUniform1i(invalid sampler/tex unit index for "
			"uniform %d)",
                        location);
            return;
         }
      }
   }

   if (uni->type->is_image()) {
      int i;

      for (i = 0; i < count; i++) {
         const int unit = ((GLint *) values)[i];

         /* check that the image unit is legal */
         if (unit < 0 || unit >= (int)ctx->Const.MaxImageUnits) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glUniform1i(invalid image unit index for uniform %d)",
                        location);
            return;
         }
      }
   }

   /* Page 82 (page 96 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "When loading N elements starting at an arbitrary position k in a
    *     uniform declared as an array, elements k through k + N - 1 in the
    *     array will be replaced with the new values. Values for any array
    *     element that exceeds the highest array element index used, as
    *     reported by GetActiveUniform, will be ignored by the GL."
    *
    * Clamp 'count' to a valid value.  Note that for non-arrays a count > 1
    * will have already generated an error.
    */
   if (uni->array_elements != 0) {
      count = MIN2(count, (int) (uni->array_elements - offset));
   }

   FLUSH_VERTICES(ctx, _NEW_PROGRAM_CONSTANTS);

   /* Store the data in the "actual type" backing storage for the uniform.
    */
   if (!uni->type->is_boolean()) {
      memcpy(&uni->storage[components * offset], values,
	     sizeof(uni->storage[0]) * components * count);
   } else {
      const union gl_constant_value *src =
	 (const union gl_constant_value *) values;
      union gl_constant_value *dst = &uni->storage[components * offset];
      const unsigned elems = components * count;
      unsigned i;

      for (i = 0; i < elems; i++) {
	 if (basicType == GLSL_TYPE_FLOAT) {
            dst[i].i = src[i].f != 0.0f ? ctx->Const.UniformBooleanTrue : 0;
	 } else {
            dst[i].i = src[i].i != 0    ? ctx->Const.UniformBooleanTrue : 0;
	 }
      }
   }

   uni->initialized = true;

   _mesa_propagate_uniforms_to_driver_storage(uni, offset, count);

   /* If the uniform is a sampler, do the extra magic necessary to propagate
    * the changes through.
    */
   if (uni->type->is_sampler()) {
      int i;

      bool flushed = false;
      for (i = 0; i < MESA_SHADER_STAGES; i++) {
	 struct gl_shader *const sh = shProg->_LinkedShaders[i];
         int j;

	 /* If the shader stage doesn't use the sampler uniform, skip this.
	  */
	 if (sh == NULL || !uni->sampler[i].active)
	    continue;

         for (j = 0; j < count; j++) {
            sh->SamplerUnits[uni->sampler[i].index + offset + j] =
               ((unsigned *) values)[j];
         }

	 struct gl_program *const prog = sh->Program;

	 assert(sizeof(prog->SamplerUnits) == sizeof(sh->SamplerUnits));

	 /* Determine if any of the samplers used by this shader stage have
	  * been modified.
	  */
	 bool changed = false;
	 for (unsigned j = 0; j < Elements(prog->SamplerUnits); j++) {
	    if ((sh->active_samplers & (1U << j)) != 0
		&& (prog->SamplerUnits[j] != sh->SamplerUnits[j])) {
	       changed = true;
	       break;
	    }
	 }

	 if (changed) {
	    if (!flushed) {
	       FLUSH_VERTICES(ctx, _NEW_TEXTURE | _NEW_PROGRAM);
	       flushed = true;
	    }

	    memcpy(prog->SamplerUnits,
		   sh->SamplerUnits,
		   sizeof(sh->SamplerUnits));

	    _mesa_update_shader_textures_used(shProg, prog);
            if (ctx->Driver.SamplerUniformChange)
	       ctx->Driver.SamplerUniformChange(ctx, prog->Target, prog);
	 }
      }
   }

   /* If the uniform is an image, update the mapping from image
    * uniforms to image units present in the shader data structure.
    */
   if (uni->type->is_image()) {
      int i, j;

      for (i = 0; i < MESA_SHADER_STAGES; i++) {
	 if (uni->image[i].active) {
            struct gl_shader *sh = shProg->_LinkedShaders[i];

            for (j = 0; j < count; j++)
               sh->ImageUnits[uni->image[i].index + offset + j] =
                  ((GLint *) values)[j];
         }
      }

      ctx->NewDriverState |= ctx->DriverFlags.NewImageUnits;
   }
}

/**
 * Called by glUniformMatrix*() functions.
 * Note: cols=2, rows=4  ==>  array[2] of vec4
 */
extern "C" void
_mesa_uniform_matrix(struct gl_context *ctx, struct gl_shader_program *shProg,
		     GLuint cols, GLuint rows,
                     GLint location, GLsizei count,
                     GLboolean transpose, const GLfloat *values)
{
   unsigned offset;
   unsigned vectors;
   unsigned components;
   unsigned elements;

   struct gl_uniform_storage *const uni =
      validate_uniform_parameters(ctx, shProg, location, count,
                                  &offset, "glUniformMatrix", false);
   if (uni == NULL)
      return;

   if (!uni->type->is_matrix()) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
		  "glUniformMatrix(non-matrix uniform)");
      return;
   }

   assert(!uni->type->is_sampler());
   vectors = uni->type->matrix_columns;
   components = uni->type->vector_elements;

   /* Verify that the types are compatible.  This is greatly simplified for
    * matrices because they can only have a float base type.
    */
   if (vectors != cols || components != rows) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
		  "glUniformMatrix(matrix size mismatch)");
      return;
   }

   /* GL_INVALID_VALUE is generated if `transpose' is not GL_FALSE.
    * http://www.khronos.org/opengles/sdk/docs/man/xhtml/glUniform.xml */
   if (ctx->API == API_OPENGLES
       || (ctx->API == API_OPENGLES2 && ctx->Version < 30)) {
      if (transpose) {
	 _mesa_error(ctx, GL_INVALID_VALUE,
		     "glUniformMatrix(matrix transpose is not GL_FALSE)");
	 return;
      }
   }

   if (ctx->_Shader->Flags & GLSL_UNIFORMS) {
      log_uniform(values, GLSL_TYPE_FLOAT, components, vectors, count,
		  bool(transpose), shProg, location, uni);
   }

   /* Page 82 (page 96 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "When loading N elements starting at an arbitrary position k in a
    *     uniform declared as an array, elements k through k + N - 1 in the
    *     array will be replaced with the new values. Values for any array
    *     element that exceeds the highest array element index used, as
    *     reported by GetActiveUniform, will be ignored by the GL."
    *
    * Clamp 'count' to a valid value.  Note that for non-arrays a count > 1
    * will have already generated an error.
    */
   if (uni->array_elements != 0) {
      count = MIN2(count, (int) (uni->array_elements - offset));
   }

   FLUSH_VERTICES(ctx, _NEW_PROGRAM_CONSTANTS);

   /* Store the data in the "actual type" backing storage for the uniform.
    */
   elements = components * vectors;

   if (!transpose) {
      memcpy(&uni->storage[elements * offset], values,
	     sizeof(uni->storage[0]) * elements * count);
   } else {
      /* Copy and transpose the matrix.
       */
      const float *src = values;
      float *dst = &uni->storage[elements * offset].f;

      for (int i = 0; i < count; i++) {
	 for (unsigned r = 0; r < rows; r++) {
	    for (unsigned c = 0; c < cols; c++) {
	       dst[(c * components) + r] = src[c + (r * vectors)];
	    }
	 }

	 dst += elements;
	 src += elements;
      }
   }

   uni->initialized = true;

   _mesa_propagate_uniforms_to_driver_storage(uni, offset, count);
}


/**
 * Called via glGetUniformLocation().
 *
 * Returns the uniform index into UniformStorage (also the
 * glGetActiveUniformsiv uniform index), and stores the referenced
 * array offset in *offset, or GL_INVALID_INDEX (-1).
 */
extern "C" unsigned
_mesa_get_uniform_location(struct gl_context *ctx,
                           struct gl_shader_program *shProg,
                           const GLchar *name,
                           unsigned *out_offset)
{
   /* Page 80 (page 94 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "The first element of a uniform array is identified using the
    *     name of the uniform array appended with "[0]". Except if the last
    *     part of the string name indicates a uniform array, then the
    *     location of the first element of that array can be retrieved by
    *     either using the name of the uniform array, or the name of the
    *     uniform array appended with "[0]"."
    *
    * Note: since uniform names are not allowed to use whitespace, and array
    * indices within uniform names are not allowed to use "+", "-", or leading
    * zeros, it follows that each uniform has a unique name up to the possible
    * ambiguity with "[0]" noted above.  Therefore we don't need to worry
    * about mal-formed inputs--they will properly fail when we try to look up
    * the uniform name in shProg->UniformHash.
    */

   const GLchar *base_name_end;
   long offset = parse_program_resource_name(name, &base_name_end);
   bool array_lookup = offset >= 0;
   char *name_copy;

   if (array_lookup) {
      name_copy = (char *) malloc(base_name_end - name + 1);
      memcpy(name_copy, name, base_name_end - name);
      name_copy[base_name_end - name] = '\0';
   } else {
      name_copy = (char *) name;
      offset = 0;
   }

   unsigned location = 0;
   const bool found = shProg->UniformHash->get(location, name_copy);

   assert(!found
	  || strcmp(name_copy, shProg->UniformStorage[location].name) == 0);

   /* Free the temporary buffer *before* possibly returning an error.
    */
   if (name_copy != name)
      free(name_copy);

   if (!found)
      return GL_INVALID_INDEX;

   /* If the uniform is an array, fail if the index is out of bounds.
    * (A negative index is caught above.)  This also fails if the uniform
    * is not an array, but the user is trying to index it, because
    * array_elements is zero and offset >= 0.
    */
   if (array_lookup
       && offset >= (long) shProg->UniformStorage[location].array_elements) {
      return GL_INVALID_INDEX;
   }

   *out_offset = offset;
   return location;
}

extern "C" bool
_mesa_sampler_uniforms_are_valid(const struct gl_shader_program *shProg,
				 char *errMsg, size_t errMsgLength)
{
   const glsl_type *unit_types[MAX_COMBINED_TEXTURE_IMAGE_UNITS];

   memset(unit_types, 0, sizeof(unit_types));

   for (unsigned i = 0; i < shProg->NumUserUniformStorage; i++) {
      const struct gl_uniform_storage *const storage =
	 &shProg->UniformStorage[i];
      const glsl_type *const t = (storage->type->is_array())
	 ? storage->type->fields.array : storage->type;

      if (!t->is_sampler())
	 continue;

      const unsigned count = MAX2(1, storage->type->array_size());
      for (unsigned j = 0; j < count; j++) {
	 const unsigned unit = storage->storage[j].i;

	 /* The types of the samplers associated with a particular texture
	  * unit must be an exact match.  Page 74 (page 89 of the PDF) of the
	  * OpenGL 3.3 core spec says:
	  *
	  *     "It is not allowed to have variables of different sampler
	  *     types pointing to the same texture image unit within a program
	  *     object."
	  */
	 if (unit_types[unit] == NULL) {
	    unit_types[unit] = t;
	 } else if (unit_types[unit] != t) {
	    _mesa_snprintf(errMsg, errMsgLength,
			   "Texture unit %d is accessed both as %s and %s",
			   unit, unit_types[unit]->name, t->name);
	    return false;
	 }
      }
   }

   return true;
}

extern "C" bool
_mesa_sampler_uniforms_pipeline_are_valid(struct gl_pipeline_object *pipeline)
{
   /* Section 2.11.11 (Shader Execution), subheading "Validation," of the
    * OpenGL 4.1 spec says:
    *
    *     "[INVALID_OPERATION] is generated by any command that transfers
    *     vertices to the GL if:
    *
    *         ...
    *
    *         - Any two active samplers in the current program object are of
    *           different types, but refer to the same texture image unit.
    *
    *         - The number of active samplers in the program exceeds the
    *           maximum number of texture image units allowed."
    */
   unsigned active_samplers = 0;
   const struct gl_shader_program **shProg =
      (const struct gl_shader_program **) pipeline->CurrentProgram;

   const glsl_type *unit_types[MAX_COMBINED_TEXTURE_IMAGE_UNITS];
   memset(unit_types, 0, sizeof(unit_types));

   for (unsigned idx = 0; idx < ARRAY_SIZE(pipeline->CurrentProgram); idx++) {
      if (!shProg[idx])
         continue;

      for (unsigned i = 0; i < shProg[idx]->NumUserUniformStorage; i++) {
         const struct gl_uniform_storage *const storage =
            &shProg[idx]->UniformStorage[i];
         const glsl_type *const t = (storage->type->is_array())
            ? storage->type->fields.array : storage->type;

         if (!t->is_sampler())
            continue;

         active_samplers++;

         const unsigned count = MAX2(1, storage->type->array_size());
         for (unsigned j = 0; j < count; j++) {
            const unsigned unit = storage->storage[j].i;

            /* The types of the samplers associated with a particular texture
             * unit must be an exact match.  Page 74 (page 89 of the PDF) of
             * the OpenGL 3.3 core spec says:
             *
             *     "It is not allowed to have variables of different sampler
             *     types pointing to the same texture image unit within a
             *     program object."
             */
            if (unit_types[unit] == NULL) {
               unit_types[unit] = t;
            } else if (unit_types[unit] != t) {
               pipeline->InfoLog =
                  ralloc_asprintf(pipeline,
                                  "Texture unit %d is accessed both as %s "
                                  "and %s",
                                  unit, unit_types[unit]->name, t->name);
               return false;
            }
         }
      }
   }

   if (active_samplers > MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
      pipeline->InfoLog =
         ralloc_asprintf(pipeline,
                         "the number of active samplers %d exceed the "
                         "maximum %d",
                         active_samplers, MAX_COMBINED_TEXTURE_IMAGE_UNITS);
      return false;
   }

   return true;
}
