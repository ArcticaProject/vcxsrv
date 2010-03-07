/*
 * Mesa 3-D graphics library
 * Version:  7.1
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef TEXGEN_H
#define TEXGEN_H


#include "main/mtypes.h"


#if FEATURE_texgen

#define _MESA_INIT_TEXGEN_FUNCTIONS(driver, impl) \
   do {                                           \
      (driver)->TexGen = impl ## TexGen;          \
   } while (0)

extern void GLAPIENTRY
_mesa_TexGenfv( GLenum coord, GLenum pname, const GLfloat *params );

extern void GLAPIENTRY
_mesa_TexGeni( GLenum coord, GLenum pname, GLint param );

extern void
_mesa_init_texgen_dispatch(struct _glapi_table *disp);

#else /* FEATURE_texgen */

#define _MESA_INIT_TEXGEN_FUNCTIONS(driver, impl) do { } while (0)

static void
_mesa_TexGenfv( GLenum coord, GLenum pname, const GLfloat *params )
{
}

static void INLINE
_mesa_TexGeni( GLenum coord, GLenum pname, GLint param )
{
}

static INLINE void
_mesa_init_texgen_dispatch(struct _glapi_table *disp)
{
}

#endif /* FEATURE_texgen */


#endif /* TEXGEN_H */
