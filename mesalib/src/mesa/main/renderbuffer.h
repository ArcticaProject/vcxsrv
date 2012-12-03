/*
 * Mesa 3-D graphics library
 * Version:  6.5
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
 */


#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include "glheader.h"
#include "mtypes.h"

struct gl_context;
struct gl_framebuffer;
struct gl_renderbuffer;

extern void
_mesa_init_renderbuffer(struct gl_renderbuffer *rb, GLuint name);

extern struct gl_renderbuffer *
_mesa_new_renderbuffer(struct gl_context *ctx, GLuint name);

extern void
_mesa_delete_renderbuffer(struct gl_context *ctx, struct gl_renderbuffer *rb);

extern void
_mesa_add_renderbuffer(struct gl_framebuffer *fb,
                       gl_buffer_index bufferName, struct gl_renderbuffer *rb);

extern void
_mesa_remove_renderbuffer(struct gl_framebuffer *fb,
                          gl_buffer_index bufferName);

extern void
_mesa_reference_renderbuffer_(struct gl_renderbuffer **ptr,
                              struct gl_renderbuffer *rb);

static inline void
_mesa_reference_renderbuffer(struct gl_renderbuffer **ptr,
                             struct gl_renderbuffer *rb)
{
   if (*ptr != rb)
      _mesa_reference_renderbuffer_(ptr, rb);
}
      


#endif /* RENDERBUFFER_H */
