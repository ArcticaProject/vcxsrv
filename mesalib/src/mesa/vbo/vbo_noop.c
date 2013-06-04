/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 * Copyright (C) 2011  VMware, Inc.  All Rights Reserved.
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
 * GLvertexformat no-op functions.  Used in out-of-memory situations.
 */


#include "main/glheader.h"
#include "main/context.h"
#include "main/dispatch.h"
#include "main/dlist.h"
#include "main/eval.h"
#include "vbo/vbo_noop.h"

static void GLAPIENTRY
_mesa_noop_EdgeFlag(GLboolean b)
{
}

static void GLAPIENTRY
_mesa_noop_Indexf(GLfloat f)
{
}

static void GLAPIENTRY
_mesa_noop_Indexfv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_FogCoordfEXT(GLfloat a)
{
}

static void GLAPIENTRY
_mesa_noop_FogCoordfvEXT(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_Normal3f(GLfloat a, GLfloat b, GLfloat c)
{
}

static void GLAPIENTRY
_mesa_noop_Normal3fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_Color4f(GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
}

static void GLAPIENTRY
_mesa_noop_Color4fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_Color3f(GLfloat a, GLfloat b, GLfloat c)
{
}

static void GLAPIENTRY
_mesa_noop_Color3fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_MultiTexCoord1fARB(GLenum target, GLfloat a)
{
}

static void GLAPIENTRY
_mesa_noop_MultiTexCoord1fvARB(GLenum target, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_MultiTexCoord2fARB(GLenum target, GLfloat a, GLfloat b)
{
}

static void GLAPIENTRY
_mesa_noop_MultiTexCoord2fvARB(GLenum target, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_MultiTexCoord3fARB(GLenum target, GLfloat a, GLfloat b, GLfloat c)
{
}

static void GLAPIENTRY
_mesa_noop_MultiTexCoord3fvARB(GLenum target, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_MultiTexCoord4fARB(GLenum target, GLfloat a, GLfloat b,
                              GLfloat c, GLfloat d)
{
}

static void GLAPIENTRY
_mesa_noop_MultiTexCoord4fvARB(GLenum target, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_SecondaryColor3fEXT(GLfloat a, GLfloat b, GLfloat c)
{
}

static void GLAPIENTRY
_mesa_noop_SecondaryColor3fvEXT(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_TexCoord1f(GLfloat a)
{
}

static void GLAPIENTRY
_mesa_noop_TexCoord1fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_TexCoord2f(GLfloat a, GLfloat b)
{
}

static void GLAPIENTRY
_mesa_noop_TexCoord2fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_TexCoord3f(GLfloat a, GLfloat b, GLfloat c)
{
}

static void GLAPIENTRY
_mesa_noop_TexCoord3fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_TexCoord4f(GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
}

static void GLAPIENTRY
_mesa_noop_TexCoord4fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib1fNV(GLuint index, GLfloat x)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib1fvNV(GLuint index, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib2fNV(GLuint index, GLfloat x, GLfloat y)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib2fvNV(GLuint index, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib3fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib3fvNV(GLuint index, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib4fNV(GLuint index, GLfloat x,
                            GLfloat y, GLfloat z, GLfloat w)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib4fvNV(GLuint index, const GLfloat * v)
{
}


static void GLAPIENTRY
_mesa_noop_VertexAttrib1fARB(GLuint index, GLfloat x)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib1fvARB(GLuint index, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib2fARB(GLuint index, GLfloat x, GLfloat y)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib2fvARB(GLuint index, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib3fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib3fvARB(GLuint index, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib4fARB(GLuint index, GLfloat x,
                             GLfloat y, GLfloat z, GLfloat w)
{
}

static void GLAPIENTRY
_mesa_noop_VertexAttrib4fvARB(GLuint index, const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_Materialfv(GLenum face, GLenum pname, const GLfloat * params)
{
}

static void GLAPIENTRY
_mesa_noop_Vertex2fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_Vertex3fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_Vertex4fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_Vertex2f(GLfloat a, GLfloat b)
{
}

static void GLAPIENTRY
_mesa_noop_Vertex3f(GLfloat a, GLfloat b, GLfloat c)
{
}

static void GLAPIENTRY
_mesa_noop_Vertex4f(GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
}

static void GLAPIENTRY
_mesa_noop_EvalCoord1f(GLfloat a)
{
}

static void GLAPIENTRY
_mesa_noop_EvalCoord1fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_EvalCoord2f(GLfloat a, GLfloat b)
{
}

static void GLAPIENTRY
_mesa_noop_EvalCoord2fv(const GLfloat * v)
{
}

static void GLAPIENTRY
_mesa_noop_EvalPoint1(GLint a)
{
}

static void GLAPIENTRY
_mesa_noop_EvalPoint2(GLint a, GLint b)
{
}

static void GLAPIENTRY
_mesa_noop_ArrayElement(GLint elem)
{
}


static void GLAPIENTRY
_mesa_noop_Begin(GLenum mode)
{
}

static void GLAPIENTRY
_mesa_noop_End(void)
{
}

static void GLAPIENTRY
_mesa_noop_PrimitiveRestartNV(void)
{
}


/**
 * Build a vertexformat of functions that are no-ops.
 * These are used in out-of-memory situations when we have no VBO
 * to put the vertex data into.
 */
void
_mesa_noop_vtxfmt_init(GLvertexformat * vfmt)
{
   vfmt->ArrayElement = _mesa_noop_ArrayElement;

   vfmt->Begin = _mesa_noop_Begin;

   vfmt->CallList = _mesa_CallList;
   vfmt->CallLists = _mesa_CallLists;

   vfmt->Color3f = _mesa_noop_Color3f;
   vfmt->Color3fv = _mesa_noop_Color3fv;
   vfmt->Color4f = _mesa_noop_Color4f;
   vfmt->Color4fv = _mesa_noop_Color4fv;
   vfmt->EdgeFlag = _mesa_noop_EdgeFlag;
   vfmt->End = _mesa_noop_End;

   vfmt->PrimitiveRestartNV = _mesa_noop_PrimitiveRestartNV;

   vfmt->EvalCoord1f = _mesa_noop_EvalCoord1f;
   vfmt->EvalCoord1fv = _mesa_noop_EvalCoord1fv;
   vfmt->EvalCoord2f = _mesa_noop_EvalCoord2f;
   vfmt->EvalCoord2fv = _mesa_noop_EvalCoord2fv;
   vfmt->EvalPoint1 = _mesa_noop_EvalPoint1;
   vfmt->EvalPoint2 = _mesa_noop_EvalPoint2;

   vfmt->FogCoordfEXT = _mesa_noop_FogCoordfEXT;
   vfmt->FogCoordfvEXT = _mesa_noop_FogCoordfvEXT;
   vfmt->Indexf = _mesa_noop_Indexf;
   vfmt->Indexfv = _mesa_noop_Indexfv;
   vfmt->Materialfv = _mesa_noop_Materialfv;
   vfmt->MultiTexCoord1fARB = _mesa_noop_MultiTexCoord1fARB;
   vfmt->MultiTexCoord1fvARB = _mesa_noop_MultiTexCoord1fvARB;
   vfmt->MultiTexCoord2fARB = _mesa_noop_MultiTexCoord2fARB;
   vfmt->MultiTexCoord2fvARB = _mesa_noop_MultiTexCoord2fvARB;
   vfmt->MultiTexCoord3fARB = _mesa_noop_MultiTexCoord3fARB;
   vfmt->MultiTexCoord3fvARB = _mesa_noop_MultiTexCoord3fvARB;
   vfmt->MultiTexCoord4fARB = _mesa_noop_MultiTexCoord4fARB;
   vfmt->MultiTexCoord4fvARB = _mesa_noop_MultiTexCoord4fvARB;
   vfmt->Normal3f = _mesa_noop_Normal3f;
   vfmt->Normal3fv = _mesa_noop_Normal3fv;
   vfmt->SecondaryColor3fEXT = _mesa_noop_SecondaryColor3fEXT;
   vfmt->SecondaryColor3fvEXT = _mesa_noop_SecondaryColor3fvEXT;
   vfmt->TexCoord1f = _mesa_noop_TexCoord1f;
   vfmt->TexCoord1fv = _mesa_noop_TexCoord1fv;
   vfmt->TexCoord2f = _mesa_noop_TexCoord2f;
   vfmt->TexCoord2fv = _mesa_noop_TexCoord2fv;
   vfmt->TexCoord3f = _mesa_noop_TexCoord3f;
   vfmt->TexCoord3fv = _mesa_noop_TexCoord3fv;
   vfmt->TexCoord4f = _mesa_noop_TexCoord4f;
   vfmt->TexCoord4fv = _mesa_noop_TexCoord4fv;
   vfmt->Vertex2f = _mesa_noop_Vertex2f;
   vfmt->Vertex2fv = _mesa_noop_Vertex2fv;
   vfmt->Vertex3f = _mesa_noop_Vertex3f;
   vfmt->Vertex3fv = _mesa_noop_Vertex3fv;
   vfmt->Vertex4f = _mesa_noop_Vertex4f;
   vfmt->Vertex4fv = _mesa_noop_Vertex4fv;
   vfmt->VertexAttrib1fNV = _mesa_noop_VertexAttrib1fNV;
   vfmt->VertexAttrib1fvNV = _mesa_noop_VertexAttrib1fvNV;
   vfmt->VertexAttrib2fNV = _mesa_noop_VertexAttrib2fNV;
   vfmt->VertexAttrib2fvNV = _mesa_noop_VertexAttrib2fvNV;
   vfmt->VertexAttrib3fNV = _mesa_noop_VertexAttrib3fNV;
   vfmt->VertexAttrib3fvNV = _mesa_noop_VertexAttrib3fvNV;
   vfmt->VertexAttrib4fNV = _mesa_noop_VertexAttrib4fNV;
   vfmt->VertexAttrib4fvNV = _mesa_noop_VertexAttrib4fvNV;
   vfmt->VertexAttrib1fARB = _mesa_noop_VertexAttrib1fARB;
   vfmt->VertexAttrib1fvARB = _mesa_noop_VertexAttrib1fvARB;
   vfmt->VertexAttrib2fARB = _mesa_noop_VertexAttrib2fARB;
   vfmt->VertexAttrib2fvARB = _mesa_noop_VertexAttrib2fvARB;
   vfmt->VertexAttrib3fARB = _mesa_noop_VertexAttrib3fARB;
   vfmt->VertexAttrib3fvARB = _mesa_noop_VertexAttrib3fvARB;
   vfmt->VertexAttrib4fARB = _mesa_noop_VertexAttrib4fARB;
   vfmt->VertexAttrib4fvARB = _mesa_noop_VertexAttrib4fvARB;
}


/**
 * Is the given dispatch table using the no-op functions?
 */
GLboolean
_mesa_using_noop_vtxfmt(const struct _glapi_table *dispatch)
{
   return GET_Begin((struct _glapi_table *) dispatch) == _mesa_noop_Begin;
}
