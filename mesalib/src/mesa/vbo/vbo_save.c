/*
 * Mesa 3-D graphics library
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@vmware.com>
 */


#include "main/mtypes.h"
#include "main/bufferobj.h"
#include "main/imports.h"

#include "vbo_context.h"


static void vbo_save_callback_init( struct gl_context *ctx )
{
   ctx->Driver.NewList = vbo_save_NewList;
   ctx->Driver.EndList = vbo_save_EndList;
   ctx->Driver.SaveFlushVertices = vbo_save_SaveFlushVertices;
   ctx->Driver.BeginCallList = vbo_save_BeginCallList;
   ctx->Driver.EndCallList = vbo_save_EndCallList;
   ctx->Driver.NotifySaveBegin = vbo_save_NotifyBegin;
}



/**
 * Called at context creation time.
 */
void vbo_save_init( struct gl_context *ctx )
{
   struct vbo_context *vbo = vbo_context(ctx);
   struct vbo_save_context *save = &vbo->save;

   save->ctx = ctx;

   vbo_save_api_init( save );
   vbo_save_callback_init(ctx);

   {
      struct gl_client_array *arrays = save->arrays;
      unsigned i;

      memcpy(arrays, &vbo->currval[VBO_ATTRIB_POS],
             VERT_ATTRIB_FF_MAX * sizeof(arrays[0]));
      for (i = 0; i < VERT_ATTRIB_FF_MAX; ++i) {
         struct gl_client_array *array;
         array = &arrays[VERT_ATTRIB_FF(i)];
         array->BufferObj = NULL;
         _mesa_reference_buffer_object(ctx, &arrays->BufferObj,
                                       vbo->currval[VBO_ATTRIB_POS+i].BufferObj);
      }

      memcpy(arrays + VERT_ATTRIB_GENERIC(0),
             &vbo->currval[VBO_ATTRIB_GENERIC0],
             VERT_ATTRIB_GENERIC_MAX * sizeof(arrays[0]));

      for (i = 0; i < VERT_ATTRIB_GENERIC_MAX; ++i) {
         struct gl_client_array *array;
         array = &arrays[VERT_ATTRIB_GENERIC(i)];
         array->BufferObj = NULL;
         _mesa_reference_buffer_object(ctx, &array->BufferObj,
                           vbo->currval[VBO_ATTRIB_GENERIC0+i].BufferObj);
      }
   }

   ctx->Driver.CurrentSavePrimitive = PRIM_OUTSIDE_BEGIN_END;
}


void vbo_save_destroy( struct gl_context *ctx )
{
   struct vbo_context *vbo = vbo_context(ctx);
   struct vbo_save_context *save = &vbo->save;
   GLuint i;

   if (save->prim_store) {
      if ( --save->prim_store->refcount == 0 ) {
         free(save->prim_store);
         save->prim_store = NULL;
      }
      if ( --save->vertex_store->refcount == 0 ) {
         _mesa_reference_buffer_object(ctx,
                                       &save->vertex_store->bufferobj, NULL);
         free(save->vertex_store);
         save->vertex_store = NULL;
      }
   }

   for (i = 0; i < VBO_ATTRIB_MAX; i++) {
      _mesa_reference_buffer_object(ctx, &save->arrays[i].BufferObj, NULL);
   }
}




/* Note that this can occur during the playback of a display list:
 */
void vbo_save_fallback( struct gl_context *ctx, GLboolean fallback )
{
   struct vbo_save_context *save = &vbo_context(ctx)->save;

   if (fallback)
      save->replay_flags |= VBO_SAVE_FALLBACK;
   else
      save->replay_flags &= ~VBO_SAVE_FALLBACK;
}
