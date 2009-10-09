/*
 * Mesa 3-D graphics library
 * Version:  7.1
 *
 * Copyright (C) 2005-2008  Brian Paul   All Rights Reserved.
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SLANG_EMIT_H
#define SLANG_EMIT_H


#include "main/imports.h"
#include "slang_compile.h"
#include "slang_ir.h"
#include "main/mtypes.h"


extern GLuint
_slang_swizzle_swizzle(GLuint swz1, GLuint swz2);


extern GLuint
_slang_var_swizzle(GLint size, GLint comp);


extern GLboolean
_slang_emit_code(slang_ir_node *n, slang_var_table *vartable,
                 struct gl_program *prog,
                 const struct gl_sl_pragmas *pragmas,
                 GLboolean withEnd,
                 slang_info_log *log);


#endif /* SLANG_EMIT_H */
