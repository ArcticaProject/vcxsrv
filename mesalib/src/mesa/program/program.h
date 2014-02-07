/*
 * Mesa 3-D graphics library
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file program.c
 * Vertex and fragment program support functions.
 * \author Brian Paul
 */


/**
 * \mainpage Mesa vertex and fragment program module
 *
 * This module or directory contains most of the code for vertex and
 * fragment programs and shaders, including state management, parsers,
 * and (some) software routines for executing programs
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#include "main/compiler.h"
#include "main/mtypes.h"


#ifdef __cplusplus
extern "C" {
#endif

extern struct gl_program _mesa_DummyProgram;


extern void
_mesa_init_program(struct gl_context *ctx);

extern void
_mesa_free_program_data(struct gl_context *ctx);

extern void
_mesa_update_default_objects_program(struct gl_context *ctx);

extern void
_mesa_set_program_error(struct gl_context *ctx, GLint pos, const char *string);

extern const GLubyte *
_mesa_find_line_column(const GLubyte *string, const GLubyte *pos,
                       GLint *line, GLint *col);


extern struct gl_program *
_mesa_init_vertex_program(struct gl_context *ctx,
                          struct gl_vertex_program *prog,
                          GLenum target, GLuint id);

extern struct gl_program *
_mesa_init_fragment_program(struct gl_context *ctx,
                            struct gl_fragment_program *prog,
                            GLenum target, GLuint id);

extern struct gl_program *
_mesa_init_geometry_program(struct gl_context *ctx,
                            struct gl_geometry_program *prog,
                            GLenum target, GLuint id);

extern struct gl_program *
_mesa_init_compute_program(struct gl_context *ctx,
                           struct gl_compute_program *prog,
                           GLenum target, GLuint id);

extern struct gl_program *
_mesa_new_program(struct gl_context *ctx, GLenum target, GLuint id);

extern void
_mesa_delete_program(struct gl_context *ctx, struct gl_program *prog);

extern struct gl_program *
_mesa_lookup_program(struct gl_context *ctx, GLuint id);

extern void
_mesa_reference_program_(struct gl_context *ctx,
                         struct gl_program **ptr,
                         struct gl_program *prog);

static inline void
_mesa_reference_program(struct gl_context *ctx,
                        struct gl_program **ptr,
                        struct gl_program *prog)
{
   if (*ptr != prog)
      _mesa_reference_program_(ctx, ptr, prog);
}

static inline void
_mesa_reference_vertprog(struct gl_context *ctx,
                         struct gl_vertex_program **ptr,
                         struct gl_vertex_program *prog)
{
   _mesa_reference_program(ctx, (struct gl_program **) ptr,
                           (struct gl_program *) prog);
}

static inline void
_mesa_reference_fragprog(struct gl_context *ctx,
                         struct gl_fragment_program **ptr,
                         struct gl_fragment_program *prog)
{
   _mesa_reference_program(ctx, (struct gl_program **) ptr,
                           (struct gl_program *) prog);
}

static inline void
_mesa_reference_geomprog(struct gl_context *ctx,
                         struct gl_geometry_program **ptr,
                         struct gl_geometry_program *prog)
{
   _mesa_reference_program(ctx, (struct gl_program **) ptr,
                           (struct gl_program *) prog);
}

extern struct gl_program *
_mesa_clone_program(struct gl_context *ctx, const struct gl_program *prog);

static inline struct gl_vertex_program *
_mesa_clone_vertex_program(struct gl_context *ctx,
                           const struct gl_vertex_program *prog)
{
   return (struct gl_vertex_program *) _mesa_clone_program(ctx, &prog->Base);
}

static inline struct gl_geometry_program *
_mesa_clone_geometry_program(struct gl_context *ctx,
                             const struct gl_geometry_program *prog)
{
   return (struct gl_geometry_program *) _mesa_clone_program(ctx, &prog->Base);
}

static inline struct gl_fragment_program *
_mesa_clone_fragment_program(struct gl_context *ctx,
                             const struct gl_fragment_program *prog)
{
   return (struct gl_fragment_program *) _mesa_clone_program(ctx, &prog->Base);
}


extern  GLboolean
_mesa_insert_instructions(struct gl_program *prog, GLuint start, GLuint count);

extern  GLboolean
_mesa_delete_instructions(struct gl_program *prog, GLuint start, GLuint count);

extern struct gl_program *
_mesa_combine_programs(struct gl_context *ctx,
                       const struct gl_program *progA,
                       const struct gl_program *progB);

extern void
_mesa_find_used_registers(const struct gl_program *prog,
                          gl_register_file file,
                          GLboolean used[], GLuint usedSize);

extern GLint
_mesa_find_free_register(const GLboolean used[],
                         GLuint maxRegs, GLuint firstReg);


extern GLboolean
_mesa_valid_register_index(const struct gl_context *ctx,
                           gl_shader_stage shaderType,
                           gl_register_file file, GLint index);

extern void
_mesa_postprocess_program(struct gl_context *ctx, struct gl_program *prog);

extern GLint
_mesa_get_min_invocations_per_fragment(struct gl_context *ctx,
                                       const struct gl_fragment_program *prog,
                                       bool ignore_sample_qualifier);

static inline GLuint
_mesa_program_enum_to_shader_stage(GLenum v)
{
   switch (v) {
   case GL_VERTEX_PROGRAM_ARB:
      return MESA_SHADER_VERTEX;
   case GL_FRAGMENT_PROGRAM_ARB:
      return MESA_SHADER_FRAGMENT;
   case GL_GEOMETRY_PROGRAM_NV:
      return MESA_SHADER_GEOMETRY;
   case GL_COMPUTE_PROGRAM_NV:
      return MESA_SHADER_COMPUTE;
   default:
      ASSERT(0);
      return ~0;
   }
}


static inline GLenum
_mesa_shader_stage_to_program(unsigned stage)
{
   switch (stage) {
   case MESA_SHADER_VERTEX:
      return GL_VERTEX_PROGRAM_ARB;
   case MESA_SHADER_FRAGMENT:
      return GL_FRAGMENT_PROGRAM_ARB;
   case MESA_SHADER_GEOMETRY:
      return GL_GEOMETRY_PROGRAM_NV;
   case MESA_SHADER_COMPUTE:
      return GL_COMPUTE_PROGRAM_NV;
   }

   assert(!"Unexpected shader stage in _mesa_shader_stage_to_program");
   return GL_VERTEX_PROGRAM_ARB;
}


/* Cast wrappers from gl_program to gl_vertex/geometry/fragment_program */

static inline struct gl_fragment_program *
gl_fragment_program(struct gl_program *prog)
{
   return (struct gl_fragment_program *) prog;
}

static inline const struct gl_fragment_program *
gl_fragment_program_const(const struct gl_program *prog)
{
   return (const struct gl_fragment_program *) prog;
}


static inline struct gl_vertex_program *
gl_vertex_program(struct gl_program *prog)
{
   return (struct gl_vertex_program *) prog;
}

static inline const struct gl_vertex_program *
gl_vertex_program_const(const struct gl_program *prog)
{
   return (const struct gl_vertex_program *) prog;
}


static inline struct gl_geometry_program *
gl_geometry_program(struct gl_program *prog)
{
   return (struct gl_geometry_program *) prog;
}

static inline const struct gl_geometry_program *
gl_geometry_program_const(const struct gl_program *prog)
{
   return (const struct gl_geometry_program *) prog;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PROGRAM_H */
