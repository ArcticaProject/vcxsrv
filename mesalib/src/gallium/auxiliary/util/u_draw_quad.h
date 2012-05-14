/**************************************************************************
 *
 * Copyright 2008 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef U_DRAWQUAD_H
#define U_DRAWQUAD_H


#include "pipe/p_compiler.h"
#include "pipe/p_context.h"


#ifdef __cplusplus
extern "C" {
#endif

struct pipe_resource;
struct cso_context;

#include "util/u_draw.h"

extern void 
util_draw_vertex_buffer(struct pipe_context *pipe, struct cso_context *cso,
                        struct pipe_resource *vbuf, uint offset,
                        uint num_attribs, uint num_verts, uint prim_type);

void
util_draw_user_vertex_buffer(struct cso_context *cso, void *buffer,
                             uint prim_type, uint num_verts, uint num_attribs);

extern void 
util_draw_texquad(struct pipe_context *pipe, struct cso_context *cso,
                  float x0, float y0, float x1, float y1, float z);


#ifdef __cplusplus
}
#endif


#endif
