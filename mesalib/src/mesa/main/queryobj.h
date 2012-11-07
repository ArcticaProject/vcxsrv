/*
 * Mesa 3-D graphics library
 * Version:  7.1
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#ifndef QUERYOBJ_H
#define QUERYOBJ_H


#include "main/mfeatures.h"
#include "main/mtypes.h"
#include "main/hash.h"


static inline struct gl_query_object *
_mesa_lookup_query_object(struct gl_context *ctx, GLuint id)
{
   return (struct gl_query_object *)
      _mesa_HashLookup(ctx->Query.QueryObjects, id);
}


extern void
_mesa_init_query_object_functions(struct dd_function_table *driver);

extern void
_mesa_init_queryobj_dispatch(const struct gl_context *ctx,
                             struct _glapi_table *disp);

extern void
_mesa_init_queryobj(struct gl_context *ctx);

extern void
_mesa_free_queryobj_data(struct gl_context *ctx);

void GLAPIENTRY
_mesa_GenQueriesARB(GLsizei n, GLuint *ids);
void GLAPIENTRY
_mesa_DeleteQueriesARB(GLsizei n, const GLuint *ids);
GLboolean GLAPIENTRY
_mesa_IsQueryARB(GLuint id);
void GLAPIENTRY
_mesa_BeginQueryIndexed(GLenum target, GLuint index, GLuint id);
void GLAPIENTRY
_mesa_EndQueryIndexed(GLenum target, GLuint index);
void GLAPIENTRY
_mesa_BeginQueryARB(GLenum target, GLuint id);
void GLAPIENTRY
_mesa_EndQueryARB(GLenum target);
void GLAPIENTRY
_mesa_QueryCounter(GLuint id, GLenum target);
void GLAPIENTRY
_mesa_GetQueryIndexediv(GLenum target, GLuint index, GLenum pname,
                        GLint *params);
void GLAPIENTRY
_mesa_GetQueryivARB(GLenum target, GLenum pname, GLint *params);
void GLAPIENTRY
_mesa_GetQueryObjectivARB(GLuint id, GLenum pname, GLint *params);
void GLAPIENTRY
_mesa_GetQueryObjectuivARB(GLuint id, GLenum pname, GLuint *params);
void GLAPIENTRY
_mesa_GetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64EXT *params);
void GLAPIENTRY
_mesa_GetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64EXT *params);

#endif /* QUERYOBJ_H */
