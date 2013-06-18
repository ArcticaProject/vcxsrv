/**
 * \file points.h
 * Point operations.
 */

/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2005  Brian Paul   All Rights Reserved.
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


#ifndef POINTS_H
#define POINTS_H


#include "glheader.h"

struct gl_context;


extern void GLAPIENTRY
_mesa_PointSize( GLfloat size );

extern void GLAPIENTRY
_mesa_PointParameteri( GLenum pname, GLint param );

extern void GLAPIENTRY
_mesa_PointParameteriv( GLenum pname, const GLint *params );

extern void GLAPIENTRY
_mesa_PointParameterf( GLenum pname, GLfloat param );

extern void GLAPIENTRY
_mesa_PointParameterfv( GLenum pname, const GLfloat *params );

extern void 
_mesa_init_point( struct gl_context * ctx );


#endif
