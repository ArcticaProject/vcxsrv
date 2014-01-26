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


#ifndef SCISSOR_H
#define SCISSOR_H


#include "glheader.h"

struct gl_context;

extern void GLAPIENTRY
_mesa_Scissor( GLint x, GLint y, GLsizei width, GLsizei height );

extern void GLAPIENTRY
_mesa_ScissorArrayv(GLuint first, GLsizei count, const GLint * v);

extern void GLAPIENTRY
_mesa_ScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);

extern void GLAPIENTRY
_mesa_ScissorIndexedv(GLuint index, const GLint * v);

extern void
_mesa_set_scissor(struct gl_context *ctx, unsigned idx,
                  GLint x, GLint y, GLsizei width, GLsizei height);


extern void 
_mesa_init_scissor(struct gl_context *ctx);


#endif
