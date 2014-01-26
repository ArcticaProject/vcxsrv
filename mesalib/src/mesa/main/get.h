/**
 * \file get.h
 * State query functions.
 */

/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
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


#ifndef GET_H
#define GET_H


#include "glheader.h"


extern void GLAPIENTRY
_mesa_GetBooleanv( GLenum pname, GLboolean *params );

extern void GLAPIENTRY
_mesa_GetDoublev( GLenum pname, GLdouble *params );

extern void GLAPIENTRY
_mesa_GetFloatv( GLenum pname, GLfloat *params );

extern void GLAPIENTRY
_mesa_GetIntegerv( GLenum pname, GLint *params );

extern void GLAPIENTRY
_mesa_GetInteger64v( GLenum pname, GLint64 *params );

extern void GLAPIENTRY
_mesa_GetFixedv(GLenum pname, GLfixed *params);

extern void GLAPIENTRY
_mesa_GetBooleani_v( GLenum pname, GLuint index, GLboolean *params );

extern void GLAPIENTRY
_mesa_GetIntegeri_v( GLenum pname, GLuint index, GLint *params );

extern void GLAPIENTRY
_mesa_GetInteger64i_v( GLenum pname, GLuint index, GLint64 *params );

extern void GLAPIENTRY
_mesa_GetPointerv( GLenum pname, GLvoid **params );

extern void GLAPIENTRY
_mesa_GetFloati_v(GLenum target, GLuint index, GLfloat *data);

extern void GLAPIENTRY
_mesa_GetDoublei_v(GLenum target, GLuint index, GLdouble *data);

extern const GLubyte * GLAPIENTRY
_mesa_GetString( GLenum name );

extern const GLubyte * GLAPIENTRY
_mesa_GetStringi(GLenum name, GLuint index);

extern GLenum GLAPIENTRY
_mesa_GetError( void );

/* GL_ARB_robustness */
extern GLenum GLAPIENTRY
_mesa_GetGraphicsResetStatusARB( void );

#endif
