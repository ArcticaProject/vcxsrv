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


#ifndef S_RENDERBUFFER_H
#define S_RENDERBUFFER_H

#include "main/glheader.h"


struct gl_context;
struct gl_framebuffer;
struct gl_renderbuffer;


extern struct gl_renderbuffer *
_swrast_new_soft_renderbuffer(struct gl_context *ctx, GLuint name);

extern void
_swrast_map_soft_renderbuffer(struct gl_context *ctx,
                              struct gl_renderbuffer *rb,
                              GLuint x, GLuint y, GLuint w, GLuint h,
                              GLbitfield mode,
                              GLubyte **out_map,
                              GLint *out_stride);

extern void
_swrast_unmap_soft_renderbuffer(struct gl_context *ctx,
                                struct gl_renderbuffer *rb);

extern void
_swrast_set_renderbuffer_accessors(struct gl_renderbuffer *rb);


extern void
_swrast_add_soft_renderbuffers(struct gl_framebuffer *fb,
                               GLboolean color,
                               GLboolean depth,
                               GLboolean stencil,
                               GLboolean accum,
                               GLboolean alpha,
                               GLboolean aux);


#endif /* S_RENDERBUFFER_H */
