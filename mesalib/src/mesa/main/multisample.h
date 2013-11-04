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


#ifndef MULTISAMPLE_H
#define MULTISAMPLE_H

#include "glheader.h"

struct gl_context;

extern void GLAPIENTRY
_mesa_SampleCoverage(GLclampf value, GLboolean invert);


extern void
_mesa_init_multisample(struct gl_context *ctx);


extern void GLAPIENTRY
_mesa_GetMultisamplefv(GLenum pname, GLuint index, GLfloat* val);

extern void GLAPIENTRY
_mesa_SampleMaski(GLuint index, GLbitfield mask);

extern void GLAPIENTRY
_mesa_MinSampleShading(GLclampf value);

extern GLenum
_mesa_check_sample_count(struct gl_context *ctx, GLenum target,
                   GLenum internalFormat, GLsizei samples);

#endif
