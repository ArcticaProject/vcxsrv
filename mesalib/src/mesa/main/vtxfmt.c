/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2004  Brian Paul   All Rights Reserved.
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
 *
 * Authors:
 *    Keith Whitwell <keithw@vmware.com> Gareth Hughes
 */

#include "glheader.h"
#include "api_arrayelt.h"
#include "context.h"
#include "imports.h"
#include "mtypes.h"
#include "vtxfmt.h"
#include "eval.h"
#include "dlist.h"
#include "main/dispatch.h"
#include "vbo/vbo_context.h"


/**
 * Copy the functions found in the GLvertexformat object into the
 * dispatch table.
 */
static void
install_vtxfmt(struct gl_context *ctx, struct _glapi_table *tab,
               const GLvertexformat *vfmt)
{
   assert(ctx->Version > 0);

   if (ctx->API != API_OPENGL_CORE && ctx->API != API_OPENGLES2) {
      SET_Color4f(tab, vfmt->Color4f);
   }

   if (ctx->API == API_OPENGL_COMPAT) {
      _mesa_install_arrayelt_vtxfmt(tab, vfmt);
      SET_Color3f(tab, vfmt->Color3f);
      SET_Color3fv(tab, vfmt->Color3fv);
      SET_Color4fv(tab, vfmt->Color4fv);
      SET_EdgeFlag(tab, vfmt->EdgeFlag);
   }

   if (ctx->API == API_OPENGL_COMPAT) {
      _mesa_install_eval_vtxfmt(tab, vfmt);
   }

   if (ctx->API != API_OPENGL_CORE && ctx->API != API_OPENGLES2) {
      SET_Materialfv(tab, vfmt->Materialfv);
      SET_MultiTexCoord4fARB(tab, vfmt->MultiTexCoord4fARB);
      SET_Normal3f(tab, vfmt->Normal3f);
   }

   if (ctx->API == API_OPENGL_COMPAT) {
      SET_FogCoordfEXT(tab, vfmt->FogCoordfEXT);
      SET_FogCoordfvEXT(tab, vfmt->FogCoordfvEXT);
      SET_Indexf(tab, vfmt->Indexf);
      SET_Indexfv(tab, vfmt->Indexfv);
      SET_MultiTexCoord1fARB(tab, vfmt->MultiTexCoord1fARB);
      SET_MultiTexCoord1fvARB(tab, vfmt->MultiTexCoord1fvARB);
      SET_MultiTexCoord2fARB(tab, vfmt->MultiTexCoord2fARB);
      SET_MultiTexCoord2fvARB(tab, vfmt->MultiTexCoord2fvARB);
      SET_MultiTexCoord3fARB(tab, vfmt->MultiTexCoord3fARB);
      SET_MultiTexCoord3fvARB(tab, vfmt->MultiTexCoord3fvARB);
      SET_MultiTexCoord4fvARB(tab, vfmt->MultiTexCoord4fvARB);
      SET_Normal3fv(tab, vfmt->Normal3fv);
   }

   if (ctx->API == API_OPENGL_COMPAT) {
      SET_SecondaryColor3fEXT(tab, vfmt->SecondaryColor3fEXT);
      SET_SecondaryColor3fvEXT(tab, vfmt->SecondaryColor3fvEXT);
      SET_TexCoord1f(tab, vfmt->TexCoord1f);
      SET_TexCoord1fv(tab, vfmt->TexCoord1fv);
      SET_TexCoord2f(tab, vfmt->TexCoord2f);
      SET_TexCoord2fv(tab, vfmt->TexCoord2fv);
      SET_TexCoord3f(tab, vfmt->TexCoord3f);
      SET_TexCoord3fv(tab, vfmt->TexCoord3fv);
      SET_TexCoord4f(tab, vfmt->TexCoord4f);
      SET_TexCoord4fv(tab, vfmt->TexCoord4fv);
      SET_Vertex2f(tab, vfmt->Vertex2f);
      SET_Vertex2fv(tab, vfmt->Vertex2fv);
      SET_Vertex3f(tab, vfmt->Vertex3f);
      SET_Vertex3fv(tab, vfmt->Vertex3fv);
      SET_Vertex4f(tab, vfmt->Vertex4f);
      SET_Vertex4fv(tab, vfmt->Vertex4fv);
   }

   if (ctx->API == API_OPENGL_COMPAT) {
      _mesa_install_dlist_vtxfmt(tab, vfmt);   /* glCallList / glCallLists */

      SET_Begin(tab, vfmt->Begin);
      SET_End(tab, vfmt->End);
      SET_PrimitiveRestartNV(tab, vfmt->PrimitiveRestartNV);
   }

   /* Originally for GL_NV_vertex_program, this is also used by dlist.c */
   if (ctx->API == API_OPENGL_COMPAT) {
      SET_VertexAttrib1fNV(tab, vfmt->VertexAttrib1fNV);
      SET_VertexAttrib1fvNV(tab, vfmt->VertexAttrib1fvNV);
      SET_VertexAttrib2fNV(tab, vfmt->VertexAttrib2fNV);
      SET_VertexAttrib2fvNV(tab, vfmt->VertexAttrib2fvNV);
      SET_VertexAttrib3fNV(tab, vfmt->VertexAttrib3fNV);
      SET_VertexAttrib3fvNV(tab, vfmt->VertexAttrib3fvNV);
      SET_VertexAttrib4fNV(tab, vfmt->VertexAttrib4fNV);
      SET_VertexAttrib4fvNV(tab, vfmt->VertexAttrib4fvNV);
   }

   if (ctx->API != API_OPENGLES) {
      SET_VertexAttrib1fARB(tab, vfmt->VertexAttrib1fARB);
      SET_VertexAttrib1fvARB(tab, vfmt->VertexAttrib1fvARB);
      SET_VertexAttrib2fARB(tab, vfmt->VertexAttrib2fARB);
      SET_VertexAttrib2fvARB(tab, vfmt->VertexAttrib2fvARB);
      SET_VertexAttrib3fARB(tab, vfmt->VertexAttrib3fARB);
      SET_VertexAttrib3fvARB(tab, vfmt->VertexAttrib3fvARB);
      SET_VertexAttrib4fARB(tab, vfmt->VertexAttrib4fARB);
      SET_VertexAttrib4fvARB(tab, vfmt->VertexAttrib4fvARB);
   }

   /* GL_EXT_gpu_shader4 / OpenGL 3.0 */
   if (_mesa_is_desktop_gl(ctx)) {
      SET_VertexAttribI1iEXT(tab, vfmt->VertexAttribI1i);
      SET_VertexAttribI2iEXT(tab, vfmt->VertexAttribI2i);
      SET_VertexAttribI3iEXT(tab, vfmt->VertexAttribI3i);
      SET_VertexAttribI2ivEXT(tab, vfmt->VertexAttribI2iv);
      SET_VertexAttribI3ivEXT(tab, vfmt->VertexAttribI3iv);

      SET_VertexAttribI1uiEXT(tab, vfmt->VertexAttribI1ui);
      SET_VertexAttribI2uiEXT(tab, vfmt->VertexAttribI2ui);
      SET_VertexAttribI3uiEXT(tab, vfmt->VertexAttribI3ui);
      SET_VertexAttribI2uivEXT(tab, vfmt->VertexAttribI2uiv);
      SET_VertexAttribI3uivEXT(tab, vfmt->VertexAttribI3uiv);
   }

   if (_mesa_is_desktop_gl(ctx) || _mesa_is_gles3(ctx)) {
      SET_VertexAttribI4iEXT(tab, vfmt->VertexAttribI4i);
      SET_VertexAttribI4ivEXT(tab, vfmt->VertexAttribI4iv);
      SET_VertexAttribI4uiEXT(tab, vfmt->VertexAttribI4ui);
      SET_VertexAttribI4uivEXT(tab, vfmt->VertexAttribI4uiv);
   }

   if (ctx->API == API_OPENGL_COMPAT) {
      /* GL_ARB_vertex_type_10_10_10_2_rev / GL 3.3 */
      SET_VertexP2ui(tab, vfmt->VertexP2ui);
      SET_VertexP2uiv(tab, vfmt->VertexP2uiv);
      SET_VertexP3ui(tab, vfmt->VertexP3ui);
      SET_VertexP3uiv(tab, vfmt->VertexP3uiv);
      SET_VertexP4ui(tab, vfmt->VertexP4ui);
      SET_VertexP4uiv(tab, vfmt->VertexP4uiv);

      SET_TexCoordP1ui(tab, vfmt->TexCoordP1ui);
      SET_TexCoordP1uiv(tab, vfmt->TexCoordP1uiv);
      SET_TexCoordP2ui(tab, vfmt->TexCoordP2ui);
      SET_TexCoordP2uiv(tab, vfmt->TexCoordP2uiv);
      SET_TexCoordP3ui(tab, vfmt->TexCoordP3ui);
      SET_TexCoordP3uiv(tab, vfmt->TexCoordP3uiv);
      SET_TexCoordP4ui(tab, vfmt->TexCoordP4ui);
      SET_TexCoordP4uiv(tab, vfmt->TexCoordP4uiv);

      SET_MultiTexCoordP1ui(tab, vfmt->MultiTexCoordP1ui);
      SET_MultiTexCoordP2ui(tab, vfmt->MultiTexCoordP2ui);
      SET_MultiTexCoordP3ui(tab, vfmt->MultiTexCoordP3ui);
      SET_MultiTexCoordP4ui(tab, vfmt->MultiTexCoordP4ui);
      SET_MultiTexCoordP1uiv(tab, vfmt->MultiTexCoordP1uiv);
      SET_MultiTexCoordP2uiv(tab, vfmt->MultiTexCoordP2uiv);
      SET_MultiTexCoordP3uiv(tab, vfmt->MultiTexCoordP3uiv);
      SET_MultiTexCoordP4uiv(tab, vfmt->MultiTexCoordP4uiv);

      SET_NormalP3ui(tab, vfmt->NormalP3ui);
      SET_NormalP3uiv(tab, vfmt->NormalP3uiv);

      SET_ColorP3ui(tab, vfmt->ColorP3ui);
      SET_ColorP4ui(tab, vfmt->ColorP4ui);
      SET_ColorP3uiv(tab, vfmt->ColorP3uiv);
      SET_ColorP4uiv(tab, vfmt->ColorP4uiv);

      SET_SecondaryColorP3ui(tab, vfmt->SecondaryColorP3ui);
      SET_SecondaryColorP3uiv(tab, vfmt->SecondaryColorP3uiv);
   }

   if (_mesa_is_desktop_gl(ctx)) {
      SET_VertexAttribP1ui(tab, vfmt->VertexAttribP1ui);
      SET_VertexAttribP2ui(tab, vfmt->VertexAttribP2ui);
      SET_VertexAttribP3ui(tab, vfmt->VertexAttribP3ui);
      SET_VertexAttribP4ui(tab, vfmt->VertexAttribP4ui);

      SET_VertexAttribP1uiv(tab, vfmt->VertexAttribP1uiv);
      SET_VertexAttribP2uiv(tab, vfmt->VertexAttribP2uiv);
      SET_VertexAttribP3uiv(tab, vfmt->VertexAttribP3uiv);
      SET_VertexAttribP4uiv(tab, vfmt->VertexAttribP4uiv);
   }
}


/**
 * Install per-vertex functions into the API dispatch table for execution.
 */
void
_mesa_install_exec_vtxfmt(struct gl_context *ctx, const GLvertexformat *vfmt)
{
   install_vtxfmt(ctx, ctx->Exec, vfmt);
   if (ctx->BeginEnd)
      install_vtxfmt(ctx, ctx->BeginEnd, vfmt);
}


/**
 * Install per-vertex functions into the API dispatch table for display
 * list compilation.
 */
void
_mesa_install_save_vtxfmt(struct gl_context *ctx, const GLvertexformat *vfmt)
{
   if (_mesa_is_desktop_gl(ctx))
      install_vtxfmt(ctx, ctx->Save, vfmt);
}


/**
 * Install VBO vtxfmt functions.
 *
 * This function depends on ctx->Version.
 */
void
_mesa_initialize_vbo_vtxfmt(struct gl_context *ctx)
{
   struct vbo_exec_context *exec = &vbo_context(ctx)->exec;
   _mesa_install_exec_vtxfmt(ctx, &exec->vtxfmt);
   if (ctx->API == API_OPENGL_COMPAT) {
      _mesa_install_save_vtxfmt(ctx, &ctx->ListState.ListVtxfmt);
   }
}

