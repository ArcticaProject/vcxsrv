/*
 * Mesa 3-D graphics library
 * Version:  6.3
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
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keith@tungstengraphics.com>
 */

#include "main/imports.h"
#include "main/mtypes.h"
#include "main/api_arrayelt.h"
#include "main/bufferobj.h"
#include "math/m_eval.h"
#include "vbo.h"
#include "vbo_context.h"

#define NR_MAT_ATTRIBS 12

static GLuint check_size( const GLfloat *attr )
{
   if (attr[3] != 1.0) return 4;
   if (attr[2] != 0.0) return 3;
   if (attr[1] != 0.0) return 2;
   return 1;		
}


static void init_legacy_currval(struct gl_context *ctx)
{
   struct vbo_context *vbo = vbo_context(ctx);
   struct gl_client_array *arrays = &vbo->currval[VBO_ATTRIB_POS];
   GLuint i;

   memset(arrays, 0, sizeof(*arrays) * VERT_ATTRIB_FF_MAX);

   /* Set up a constant (StrideB == 0) array for each current
    * attribute:
    */
   for (i = 0; i < VERT_ATTRIB_FF_MAX; i++) {
      struct gl_client_array *cl = &arrays[i];

      /* Size will have to be determined at runtime:
       */
      cl->Size = check_size(ctx->Current.Attrib[i]);
      cl->Stride = 0;
      cl->StrideB = 0;
      cl->Enabled = 1;
      cl->Type = GL_FLOAT;
      cl->Format = GL_RGBA;
      cl->Ptr = (const void *)ctx->Current.Attrib[i];
      cl->_ElementSize = cl->Size * sizeof(GLfloat);
      _mesa_reference_buffer_object(ctx, &cl->BufferObj,
                                    ctx->Shared->NullBufferObj);
   }
}


static void init_generic_currval(struct gl_context *ctx)
{
   struct vbo_context *vbo = vbo_context(ctx);
   struct gl_client_array *arrays = &vbo->currval[VBO_ATTRIB_GENERIC0];
   GLuint i;

   memset(arrays, 0, sizeof(*arrays) * VERT_ATTRIB_GENERIC_MAX);

   for (i = 0; i < VERT_ATTRIB_GENERIC_MAX; i++) {
      struct gl_client_array *cl = &arrays[i];

      /* This will have to be determined at runtime:
       */
      cl->Size = 1;
      cl->Type = GL_FLOAT;
      cl->Format = GL_RGBA;
      cl->Ptr = (const void *)ctx->Current.Attrib[VERT_ATTRIB_GENERIC0 + i];
      cl->Stride = 0;
      cl->StrideB = 0;
      cl->Enabled = 1;
      cl->_ElementSize = cl->Size * sizeof(GLfloat);
      _mesa_reference_buffer_object(ctx, &cl->BufferObj,
                                    ctx->Shared->NullBufferObj);
   }
}


static void init_mat_currval(struct gl_context *ctx)
{
   struct vbo_context *vbo = vbo_context(ctx);
   struct gl_client_array *arrays =
      &vbo->currval[VBO_ATTRIB_MAT_FRONT_AMBIENT];
   GLuint i;

   ASSERT(NR_MAT_ATTRIBS == MAT_ATTRIB_MAX);

   memset(arrays, 0, sizeof(*arrays) * NR_MAT_ATTRIBS);

   /* Set up a constant (StrideB == 0) array for each current
    * attribute:
    */
   for (i = 0; i < NR_MAT_ATTRIBS; i++) {
      struct gl_client_array *cl = &arrays[i];

      /* Size is fixed for the material attributes, for others will
       * be determined at runtime:
       */
      switch (i - VERT_ATTRIB_GENERIC0) {
      case MAT_ATTRIB_FRONT_SHININESS:
      case MAT_ATTRIB_BACK_SHININESS:
	 cl->Size = 1;
	 break;
      case MAT_ATTRIB_FRONT_INDEXES:
      case MAT_ATTRIB_BACK_INDEXES:
	 cl->Size = 3;
	 break;
      default:
	 cl->Size = 4;
	 break;
      }

      cl->Ptr = (const void *)ctx->Light.Material.Attrib[i];
      cl->Type = GL_FLOAT;
      cl->Format = GL_RGBA;
      cl->Stride = 0;
      cl->StrideB = 0;
      cl->Enabled = 1;
      cl->_ElementSize = cl->Size * sizeof(GLfloat);
      _mesa_reference_buffer_object(ctx, &cl->BufferObj,
                                    ctx->Shared->NullBufferObj);
   }
}


GLboolean _vbo_CreateContext( struct gl_context *ctx )
{
   struct vbo_context *vbo = CALLOC_STRUCT(vbo_context);

   ctx->swtnl_im = (void *)vbo;

   /* Initialize the arrayelt helper
    */
   if (!ctx->aelt_context &&
       !_ae_create_context( ctx )) {
      return GL_FALSE;
   }

   init_legacy_currval( ctx );
   init_generic_currval( ctx );
   init_mat_currval( ctx );

   /* Build mappings from VERT_ATTRIB -> VBO_ATTRIB depending on type
    * of vertex program active.
    */
   {
      GLuint i;

      /* identity mapping */
      for (i = 0; i < Elements(vbo->map_vp_none); i++) 
	 vbo->map_vp_none[i] = i;
      /* map material attribs to generic slots */
      for (i = 0; i < NR_MAT_ATTRIBS; i++) 
	 vbo->map_vp_none[VERT_ATTRIB_GENERIC(i)]
            = VBO_ATTRIB_MAT_FRONT_AMBIENT + i;

      for (i = 0; i < Elements(vbo->map_vp_arb); i++)
	 vbo->map_vp_arb[i] = i;
   }


   /* Hook our functions into exec and compile dispatch tables.  These
    * will pretty much be permanently installed, which means that the
    * vtxfmt mechanism can be removed now.
    */
   vbo_exec_init( ctx );
   if (ctx->API == API_OPENGL)
      vbo_save_init( ctx );

   _math_init_eval();

   return GL_TRUE;
}


void _vbo_InvalidateState( struct gl_context *ctx, GLuint new_state )
{
   vbo_exec_invalidate_state(ctx, new_state);
}


void _vbo_DestroyContext( struct gl_context *ctx )
{
   struct vbo_context *vbo = vbo_context(ctx);

   if (ctx->aelt_context) {
      _ae_destroy_context( ctx );
      ctx->aelt_context = NULL;
   }

   if (vbo) {
      GLuint i;

      for (i = 0; i < VBO_ATTRIB_MAX; i++) {
         _mesa_reference_buffer_object(ctx, &vbo->currval[i].BufferObj, NULL);
      }

      vbo_exec_destroy(ctx);
      if (ctx->API == API_OPENGL)
         vbo_save_destroy(ctx);
      FREE(vbo);
      ctx->swtnl_im = NULL;
   }
}


void vbo_set_draw_func(struct gl_context *ctx, vbo_draw_func func)
{
   struct vbo_context *vbo = vbo_context(ctx);
   vbo->draw_prims = func;
}

