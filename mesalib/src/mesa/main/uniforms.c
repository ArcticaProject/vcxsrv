/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2004-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009-2010  VMware, Inc.  All Rights Reserved.
 * Copyright © 2010 Intel Corporation
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
 * \file uniforms.c
 * Functions related to GLSL uniform variables.
 * \author Brian Paul
 */

/**
 * XXX things to do:
 * 1. Check that the right error code is generated for all _mesa_error() calls.
 * 2. Insert FLUSH_VERTICES calls in various places
 */

#include "main/glheader.h"
#include "main/context.h"
#include "main/dispatch.h"
#include "main/shaderapi.h"
#include "main/shaderobj.h"
#include "main/uniforms.h"
#include "main/enums.h"
#include "ir_uniform.h"
#include "glsl_types.h"
#include "program/program.h"

/**
 * Update the vertex/fragment program's TexturesUsed array.
 *
 * This needs to be called after glUniform(set sampler var) is called.
 * A call to glUniform(samplerVar, value) causes a sampler to point to a
 * particular texture unit.  We know the sampler's texture target
 * (1D/2D/3D/etc) from compile time but the sampler's texture unit is
 * set by glUniform() calls.
 *
 * So, scan the program->SamplerUnits[] and program->SamplerTargets[]
 * information to update the prog->TexturesUsed[] values.
 * Each value of TexturesUsed[unit] is one of zero, TEXTURE_1D_INDEX,
 * TEXTURE_2D_INDEX, TEXTURE_3D_INDEX, etc.
 * We'll use that info for state validation before rendering.
 */
void
_mesa_update_shader_textures_used(struct gl_shader_program *shProg,
				  struct gl_program *prog)
{
   GLuint s;
   struct gl_shader *shader =
      shProg->_LinkedShaders[_mesa_program_enum_to_shader_stage(prog->Target)];

   assert(shader);

   memcpy(prog->SamplerUnits, shader->SamplerUnits, sizeof(prog->SamplerUnits));
   memset(prog->TexturesUsed, 0, sizeof(prog->TexturesUsed));

   for (s = 0; s < MAX_SAMPLERS; s++) {
      if (prog->SamplersUsed & (1 << s)) {
         GLuint unit = shader->SamplerUnits[s];
         GLuint tgt = shader->SamplerTargets[s];
         assert(unit < Elements(prog->TexturesUsed));
         assert(tgt < NUM_TEXTURE_TARGETS);
         prog->TexturesUsed[unit] |= (1 << tgt);
      }
   }
}

/**
 * Connect a piece of driver storage with a part of a uniform
 *
 * \param uni            The uniform with which the storage will be associated
 * \param element_stride Byte-stride between array elements.
 *                       \sa gl_uniform_driver_storage::element_stride.
 * \param vector_stride  Byte-stride between vectors (in a matrix).
 *                       \sa gl_uniform_driver_storage::vector_stride.
 * \param format         Conversion from native format to driver format
 *                       required by the driver.
 * \param data           Location to dump the data.
 */
void
_mesa_uniform_attach_driver_storage(struct gl_uniform_storage *uni,
				    unsigned element_stride,
				    unsigned vector_stride,
				    enum gl_uniform_driver_format format,
				    void *data)
{
   uni->driver_storage =
      realloc(uni->driver_storage,
	      sizeof(struct gl_uniform_driver_storage)
	      * (uni->num_driver_storage + 1));

   uni->driver_storage[uni->num_driver_storage].element_stride = element_stride;
   uni->driver_storage[uni->num_driver_storage].vector_stride = vector_stride;
   uni->driver_storage[uni->num_driver_storage].format = format;
   uni->driver_storage[uni->num_driver_storage].data = data;

   uni->num_driver_storage++;
}

/**
 * Sever all connections with all pieces of driver storage for all uniforms
 *
 * \warning
 * This function does \b not release any of the \c data pointers
 * previously passed in to \c _mesa_uniform_attach_driver_stoarge.
 */
void
_mesa_uniform_detach_all_driver_storage(struct gl_uniform_storage *uni)
{
   free(uni->driver_storage);
   uni->driver_storage = NULL;
   uni->num_driver_storage = 0;
}

void GLAPIENTRY
_mesa_Uniform1f(GLint location, GLfloat v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, &v0, GL_FLOAT);
}

void GLAPIENTRY
_mesa_Uniform2f(GLint location, GLfloat v0, GLfloat v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_FLOAT_VEC2);
}

void GLAPIENTRY
_mesa_Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_FLOAT_VEC3);
}

void GLAPIENTRY
_mesa_Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2,
                   GLfloat v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_FLOAT_VEC4);
}

void GLAPIENTRY
_mesa_Uniform1i(GLint location, GLint v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, &v0, GL_INT);
}

void GLAPIENTRY
_mesa_Uniform2i(GLint location, GLint v0, GLint v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_INT_VEC2);
}

void GLAPIENTRY
_mesa_Uniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_INT_VEC3);
}

void GLAPIENTRY
_mesa_Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_INT_VEC4);
}

void GLAPIENTRY
_mesa_Uniform1fv(GLint location, GLsizei count, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_FLOAT);
}

void GLAPIENTRY
_mesa_Uniform2fv(GLint location, GLsizei count, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_FLOAT_VEC2);
}

void GLAPIENTRY
_mesa_Uniform3fv(GLint location, GLsizei count, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_FLOAT_VEC3);
}

void GLAPIENTRY
_mesa_Uniform4fv(GLint location, GLsizei count, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_FLOAT_VEC4);
}

void GLAPIENTRY
_mesa_Uniform1iv(GLint location, GLsizei count, const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_INT);
}

void GLAPIENTRY
_mesa_Uniform2iv(GLint location, GLsizei count, const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_INT_VEC2);
}

void GLAPIENTRY
_mesa_Uniform3iv(GLint location, GLsizei count, const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_INT_VEC3);
}

void GLAPIENTRY
_mesa_Uniform4iv(GLint location, GLsizei count, const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_INT_VEC4);
}

/** Same as above with direct state access **/
void GLAPIENTRY
_mesa_ProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1f");
   _mesa_uniform(ctx, shProg, location, 1, &v0, GL_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[2];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform2f");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_FLOAT_VEC2);
}

void GLAPIENTRY
_mesa_ProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1,
                       GLfloat v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[3];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform3f");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_FLOAT_VEC3);
}

void GLAPIENTRY
_mesa_ProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1,
                       GLfloat v2, GLfloat v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLfloat v[4];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform4f");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_FLOAT_VEC4);
}

void GLAPIENTRY
_mesa_ProgramUniform1i(GLuint program, GLint location, GLint v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1i");
   _mesa_uniform(ctx, shProg, location, 1, &v0, GL_INT);
}

void GLAPIENTRY
_mesa_ProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[2];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform2i");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_INT_VEC2);
}

void GLAPIENTRY
_mesa_ProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1,
                       GLint v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[3];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform3i");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_INT_VEC3);
}

void GLAPIENTRY
_mesa_ProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1,
                       GLint v2, GLint v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLint v[4];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform4i");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_INT_VEC4);
}

void GLAPIENTRY
_mesa_ProgramUniform1fv(GLuint program, GLint location, GLsizei count,
                        const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1fv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_FLOAT);
}

void GLAPIENTRY
_mesa_ProgramUniform2fv(GLuint program, GLint location, GLsizei count,
                        const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform2fv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_FLOAT_VEC2);
}

void GLAPIENTRY
_mesa_ProgramUniform3fv(GLuint program, GLint location, GLsizei count,
                        const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform3fv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_FLOAT_VEC3);
}

void GLAPIENTRY
_mesa_ProgramUniform4fv(GLuint program, GLint location, GLsizei count,
                        const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform4fv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_FLOAT_VEC4);
}

void GLAPIENTRY
_mesa_ProgramUniform1iv(GLuint program, GLint location, GLsizei count,
                        const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1iv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_INT);
}

void GLAPIENTRY
_mesa_ProgramUniform2iv(GLuint program, GLint location, GLsizei count,
                        const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform2iv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_INT_VEC2);
}

void GLAPIENTRY
_mesa_ProgramUniform3iv(GLuint program, GLint location, GLsizei count,
                        const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform3iv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_INT_VEC3);
}

void GLAPIENTRY
_mesa_ProgramUniform4iv(GLuint program, GLint location, GLsizei count,
                        const GLint * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform4iv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_INT_VEC4);
}


/** OpenGL 3.0 GLuint-valued functions **/
void GLAPIENTRY
_mesa_Uniform1ui(GLint location, GLuint v0)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, &v0, GL_UNSIGNED_INT);
}

void GLAPIENTRY
_mesa_Uniform2ui(GLint location, GLuint v0, GLuint v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[2];
   v[0] = v0;
   v[1] = v1;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_UNSIGNED_INT_VEC2);
}

void GLAPIENTRY
_mesa_Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[3];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_UNSIGNED_INT_VEC3);
}

void GLAPIENTRY
_mesa_Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[4];
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, 1, v, GL_UNSIGNED_INT_VEC4);
}

void GLAPIENTRY
_mesa_Uniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_UNSIGNED_INT);
}

void GLAPIENTRY
_mesa_Uniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_UNSIGNED_INT_VEC2);
}

void GLAPIENTRY
_mesa_Uniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_UNSIGNED_INT_VEC3);
}

void GLAPIENTRY
_mesa_Uniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform(ctx, ctx->_Shader->ActiveProgram, location, count, value, GL_UNSIGNED_INT_VEC4);
}



void GLAPIENTRY
_mesa_UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose,
                          const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			2, 2, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose,
                          const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			3, 3, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose,
                          const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			4, 4, location, count, transpose, value);
}

/** Same as above with direct state access **/

void GLAPIENTRY
_mesa_ProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1ui");
   _mesa_uniform(ctx, shProg, location, 1, &v0, GL_UNSIGNED_INT);
}

void GLAPIENTRY
_mesa_ProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[2];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   shProg = _mesa_lookup_shader_program_err(ctx, program,
                                            "glProgramUniform2ui");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_UNSIGNED_INT_VEC2);
}

void GLAPIENTRY
_mesa_ProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1,
                        GLuint v2)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[3];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   shProg = _mesa_lookup_shader_program_err(ctx, program,
                                            "glProgramUniform3ui");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_UNSIGNED_INT_VEC3);
}

void GLAPIENTRY
_mesa_ProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1,
                        GLuint v2, GLuint v3)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint v[4];
   struct gl_shader_program *shProg;
   v[0] = v0;
   v[1] = v1;
   v[2] = v2;
   v[3] = v3;
   shProg = _mesa_lookup_shader_program_err(ctx, program, "glProgramUniform4ui");
   _mesa_uniform(ctx, shProg, location, 1, v, GL_UNSIGNED_INT_VEC4);
}

void GLAPIENTRY
_mesa_ProgramUniform1uiv(GLuint program, GLint location, GLsizei count,
                         const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform1uiv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_UNSIGNED_INT);
}

void GLAPIENTRY
_mesa_ProgramUniform2uiv(GLuint program, GLint location, GLsizei count,
                         const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform2uiv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_UNSIGNED_INT_VEC2);
}

void GLAPIENTRY
_mesa_ProgramUniform3uiv(GLuint program, GLint location, GLsizei count,
                         const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform3uiv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_UNSIGNED_INT_VEC3);
}

void GLAPIENTRY
_mesa_ProgramUniform4uiv(GLuint program, GLint location, GLsizei count,
                         const GLuint *value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniform4uiv");
   _mesa_uniform(ctx, shProg, location, count, value, GL_UNSIGNED_INT_VEC4);
}



void GLAPIENTRY
_mesa_ProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2fv");
   _mesa_uniform_matrix(ctx, shProg, 2, 2, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3fv");
   _mesa_uniform_matrix(ctx, shProg, 3, 3, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count,
                              GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4fv");
   _mesa_uniform_matrix(ctx, shProg, 4, 4, location, count, transpose, value);
}


/**
 * Non-square UniformMatrix are OpenGL 2.1
 */
void GLAPIENTRY
_mesa_UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			2, 3, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			3, 2, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			2, 4, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			4, 2, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			3, 4, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose,
                         const GLfloat *value)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_uniform_matrix(ctx, ctx->_Shader->ActiveProgram,
			4, 3, location, count, transpose, value);
}

/** Same as above with direct state access **/

void GLAPIENTRY
_mesa_ProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2x3fv");
   _mesa_uniform_matrix(ctx, shProg, 2, 3, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3x2fv");
   _mesa_uniform_matrix(ctx, shProg, 3, 2, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix2x4fv");
   _mesa_uniform_matrix(ctx, shProg, 2, 4, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4x2fv");
   _mesa_uniform_matrix(ctx, shProg, 4, 2, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix3x4fv");
   _mesa_uniform_matrix(ctx, shProg, 3, 4, location, count, transpose, value);
}

void GLAPIENTRY
_mesa_ProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count,
                                GLboolean transpose, const GLfloat * value)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg =
      _mesa_lookup_shader_program_err(ctx, program,
            "glProgramUniformMatrix4x3fv");
   _mesa_uniform_matrix(ctx, shProg, 4, 3, location, count, transpose, value);
}


void GLAPIENTRY
_mesa_GetnUniformfvARB(GLuint program, GLint location,
                       GLsizei bufSize, GLfloat *params)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_FLOAT, params);
}

void GLAPIENTRY
_mesa_GetUniformfv(GLuint program, GLint location, GLfloat *params)
{
   _mesa_GetnUniformfvARB(program, location, INT_MAX, params);
}


void GLAPIENTRY
_mesa_GetnUniformivARB(GLuint program, GLint location,
                       GLsizei bufSize, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_INT, params);
}

void GLAPIENTRY
_mesa_GetUniformiv(GLuint program, GLint location, GLint *params)
{
   _mesa_GetnUniformivARB(program, location, INT_MAX, params);
}


/* GL3 */
void GLAPIENTRY
_mesa_GetnUniformuivARB(GLuint program, GLint location,
                        GLsizei bufSize, GLuint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_UINT, params);
}

void GLAPIENTRY
_mesa_GetUniformuiv(GLuint program, GLint location, GLuint *params)
{
   _mesa_GetnUniformuivARB(program, location, INT_MAX, params);
}


/* GL4 */
void GLAPIENTRY
_mesa_GetnUniformdvARB(GLuint program, GLint location,
                       GLsizei bufSize, GLdouble *params)
{
   GET_CURRENT_CONTEXT(ctx);

   (void) program;
   (void) location;
   (void) bufSize;
   (void) params;

   /*
   _mesa_get_uniform(ctx, program, location, bufSize, GLSL_TYPE_DOUBLE, params);
   */
   _mesa_error(ctx, GL_INVALID_OPERATION, "glGetUniformdvARB"
               "(GL_ARB_gpu_shader_fp64 not implemented)");
}

void GLAPIENTRY
_mesa_GetUniformdv(GLuint program, GLint location, GLdouble *params)
{
   _mesa_GetnUniformdvARB(program, location, INT_MAX, params);
}


GLint GLAPIENTRY
_mesa_GetUniformLocation(GLuint programObj, const GLcharARB *name)
{
   struct gl_shader_program *shProg;
   GLuint index, offset;

   GET_CURRENT_CONTEXT(ctx);

   shProg = _mesa_lookup_shader_program_err(ctx, programObj,
					    "glGetUniformLocation");
   if (!shProg)
      return -1;

   /* Page 80 (page 94 of the PDF) of the OpenGL 2.1 spec says:
    *
    *     "If program has not been successfully linked, the error
    *     INVALID_OPERATION is generated."
    */
   if (shProg->LinkStatus == GL_FALSE) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
		  "glGetUniformLocation(program not linked)");
      return -1;
   }

   index = _mesa_get_uniform_location(ctx, shProg, name, &offset);
   if (index == GL_INVALID_INDEX)
      return -1;

   /* From the GL_ARB_uniform_buffer_object spec:
    *
    *     "The value -1 will be returned if <name> does not correspond to an
    *      active uniform variable name in <program>, if <name> is associated
    *      with a named uniform block, or if <name> starts with the reserved
    *      prefix "gl_"."
    */
   if (shProg->UniformStorage[index].block_index != -1 ||
       shProg->UniformStorage[index].atomic_buffer_index != -1)
      return -1;

   /* location in remap table + array element offset */
   return shProg->UniformStorage[index].remap_location + offset;
}

GLuint GLAPIENTRY
_mesa_GetUniformBlockIndex(GLuint program,
			   const GLchar *uniformBlockName)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint i;
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetUniformBlockIndex");
      return GL_INVALID_INDEX;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glGetUniformBlockIndex");
   if (!shProg)
      return GL_INVALID_INDEX;

   for (i = 0; i < shProg->NumUniformBlocks; i++) {
      if (!strcmp(shProg->UniformBlocks[i].Name, uniformBlockName))
	 return i;
   }

   return GL_INVALID_INDEX;
}

void GLAPIENTRY
_mesa_GetUniformIndices(GLuint program,
			GLsizei uniformCount,
			const GLchar * const *uniformNames,
			GLuint *uniformIndices)
{
   GET_CURRENT_CONTEXT(ctx);
   GLsizei i;
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetUniformIndices");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glGetUniformIndices");
   if (!shProg)
      return;

   if (uniformCount < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetUniformIndices(uniformCount < 0)");
      return;
   }

   for (i = 0; i < uniformCount; i++) {
      unsigned offset;
      uniformIndices[i] = _mesa_get_uniform_location(ctx, shProg,
						     uniformNames[i], &offset);
   }
}

void GLAPIENTRY
_mesa_UniformBlockBinding(GLuint program,
			  GLuint uniformBlockIndex,
			  GLuint uniformBlockBinding)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glUniformBlockBinding");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glUniformBlockBinding");
   if (!shProg)
      return;

   if (uniformBlockIndex >= shProg->NumUniformBlocks) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glUniformBlockBinding(block index %u >= %u)",
		  uniformBlockIndex, shProg->NumUniformBlocks);
      return;
   }

   if (uniformBlockBinding >= ctx->Const.MaxUniformBufferBindings) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glUniformBlockBinding(block binding %u >= %u)",
		  uniformBlockBinding, ctx->Const.MaxUniformBufferBindings);
      return;
   }

   if (shProg->UniformBlocks[uniformBlockIndex].Binding !=
       uniformBlockBinding) {
      int i;

      FLUSH_VERTICES(ctx, 0);
      ctx->NewDriverState |= ctx->DriverFlags.NewUniformBuffer;

      shProg->UniformBlocks[uniformBlockIndex].Binding = uniformBlockBinding;

      for (i = 0; i < MESA_SHADER_STAGES; i++) {
	 int stage_index = shProg->UniformBlockStageIndex[i][uniformBlockIndex];

	 if (stage_index != -1) {
	    struct gl_shader *sh = shProg->_LinkedShaders[i];
	    sh->UniformBlocks[stage_index].Binding = uniformBlockBinding;
	 }
      }
   }
}

void GLAPIENTRY
_mesa_GetActiveUniformBlockiv(GLuint program,
			      GLuint uniformBlockIndex,
			      GLenum pname,
			      GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;
   struct gl_uniform_block *block;
   unsigned i;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetActiveUniformBlockiv");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glGetActiveUniformBlockiv");
   if (!shProg)
      return;

   if (uniformBlockIndex >= shProg->NumUniformBlocks) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetActiveUniformBlockiv(block index %u >= %u)",
		  uniformBlockIndex, shProg->NumUniformBlocks);
      return;
   }

   block = &shProg->UniformBlocks[uniformBlockIndex];

   switch (pname) {
   case GL_UNIFORM_BLOCK_BINDING:
      params[0] = block->Binding;
      return;

   case GL_UNIFORM_BLOCK_DATA_SIZE:
      params[0] = block->UniformBufferSize;
      return;

   case GL_UNIFORM_BLOCK_NAME_LENGTH:
      params[0] = strlen(block->Name) + 1;
      return;

   case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS: {
      unsigned count = 0;

      for (i = 0; i < block->NumUniforms; i++) {
	 unsigned offset;
         const int idx =
            _mesa_get_uniform_location(ctx, shProg,
                                       block->Uniforms[i].IndexName,
                                       &offset);
         if (idx != -1)
            count++;
      }

      params[0] = count;
      return;
   }

   case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES: {
      unsigned count = 0;

      for (i = 0; i < block->NumUniforms; i++) {
	 unsigned offset;
         const int idx =
            _mesa_get_uniform_location(ctx, shProg,
                                       block->Uniforms[i].IndexName,
                                       &offset);

         if (idx != -1)
            params[count++] = idx;
      }
      return;
   }

   case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
      params[0] = shProg->UniformBlockStageIndex[MESA_SHADER_VERTEX][uniformBlockIndex] != -1;
      return;

   case GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER:
      params[0] = shProg->UniformBlockStageIndex[MESA_SHADER_GEOMETRY][uniformBlockIndex] != -1;
      return;

   case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
      params[0] = shProg->UniformBlockStageIndex[MESA_SHADER_FRAGMENT][uniformBlockIndex] != -1;
      return;

   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
		  "glGetActiveUniformBlockiv(pname 0x%x (%s))",
		  pname, _mesa_lookup_enum_by_nr(pname));
      return;
   }
}

void GLAPIENTRY
_mesa_GetActiveUniformBlockName(GLuint program,
				GLuint uniformBlockIndex,
				GLsizei bufSize,
				GLsizei *length,
				GLchar *uniformBlockName)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;
   struct gl_uniform_block *block;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetActiveUniformBlockiv");
      return;
   }

   if (bufSize < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetActiveUniformBlockName(bufSize %d < 0)",
		  bufSize);
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
					    "glGetActiveUniformBlockiv");
   if (!shProg)
      return;

   if (uniformBlockIndex >= shProg->NumUniformBlocks) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetActiveUniformBlockiv(block index %u >= %u)",
		  uniformBlockIndex, shProg->NumUniformBlocks);
      return;
   }

   block = &shProg->UniformBlocks[uniformBlockIndex];

   if (uniformBlockName) {
      _mesa_copy_string(uniformBlockName, bufSize, length, block->Name);
   }
}

void GLAPIENTRY
_mesa_GetActiveUniformName(GLuint program, GLuint uniformIndex,
			   GLsizei bufSize, GLsizei *length,
			   GLchar *uniformName)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;

   if (!ctx->Extensions.ARB_uniform_buffer_object) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetActiveUniformBlockiv");
      return;
   }

   if (bufSize < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
		  "glGetActiveUniformName(bufSize %d < 0)",
		  bufSize);
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program, "glGetActiveUniformName");

   if (!shProg)
      return;

   if (uniformIndex >= shProg->NumUserUniformStorage) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glGetActiveUniform(index)");
      return;
   }

   if (uniformName) {
      _mesa_get_uniform_name(& shProg->UniformStorage[uniformIndex],
                             bufSize, length, uniformName);
   }
}

void
_mesa_get_uniform_name(const struct gl_uniform_storage *uni,
                       GLsizei maxLength, GLsizei *length,
                       GLchar *nameOut)
{
   GLsizei localLength;

   if (length == NULL)
      length = &localLength;

   _mesa_copy_string(nameOut, maxLength, length, uni->name);

   /* Page 61 (page 73 of the PDF) in section 2.11 of the OpenGL ES 3.0
    * spec says:
    *
    *     "If the active uniform is an array, the uniform name returned in
    *     name will always be the name of the uniform array appended with
    *     "[0]"."
    *
    * The same text also appears in the OpenGL 4.2 spec.  It does not,
    * however, appear in any previous spec.  Previous specifications are
    * ambiguous in this regard.  However, either name can later be passed
    * to glGetUniformLocation (and related APIs), so there shouldn't be any
    * harm in always appending "[0]" to uniform array names.
    */
   if (uni->array_elements != 0) {
      int i;

      /* The comparison is strange because *length does *NOT* include the
       * terminating NUL, but maxLength does.
       */
      for (i = 0; i < 3 && (*length + i + 1) < maxLength; i++)
         nameOut[*length + i] = "[0]"[i];

      nameOut[*length + i] = '\0';
      *length += i;
   }
}

void GLAPIENTRY
_mesa_GetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex,
                                     GLenum pname, GLint *params)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_shader_program *shProg;
   struct gl_active_atomic_buffer *ab;
   GLuint i;

   if (!ctx->Extensions.ARB_shader_atomic_counters) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetActiveAtomicCounterBufferiv");
      return;
   }

   shProg = _mesa_lookup_shader_program_err(ctx, program,
                                            "glGetActiveAtomicCounterBufferiv");
   if (!shProg)
      return;

   if (bufferIndex >= shProg->NumAtomicBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetActiveAtomicCounterBufferiv(bufferIndex)");
      return;
   }

   ab = &shProg->AtomicBuffers[bufferIndex];

   switch (pname) {
   case GL_ATOMIC_COUNTER_BUFFER_BINDING:
      params[0] = ab->Binding;
      return;
   case GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE:
      params[0] = ab->MinimumSize;
      return;
   case GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS:
      params[0] = ab->NumUniforms;
      return;
   case GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES:
      for (i = 0; i < ab->NumUniforms; ++i)
         params[i] = ab->Uniforms[i];
      return;
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER:
      params[0] = ab->StageReferences[MESA_SHADER_VERTEX];
      return;
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER:
      params[0] = ab->StageReferences[MESA_SHADER_GEOMETRY];
      return;
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER:
      params[0] = ab->StageReferences[MESA_SHADER_FRAGMENT];
      return;
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER:
      params[0] = GL_FALSE;
      return;
   case GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER:
      params[0] = GL_FALSE;
      return;
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetActiveAtomicCounterBufferiv(pname 0x%x (%s))",
                  pname, _mesa_lookup_enum_by_nr(pname));
      return;
   }
}
