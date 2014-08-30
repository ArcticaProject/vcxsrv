/**************************************************************************
 *
 * Copyright 2011 Marek Olšák <maraeo@gmail.com>
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
 * IN NO EVENT SHALL AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef U_VBUF_H
#define U_VBUF_H

/* This module takes care of user buffer uploads and vertex format fallbacks.
 * It's designed for the drivers which don't want to use the Draw module.
 * There is a more detailed description at the beginning of the .c file.
 */

#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include "pipe/p_format.h"

struct cso_context;
struct u_vbuf;

/* Hardware vertex fetcher limitations can be described by this structure. */
struct u_vbuf_caps {
   enum pipe_format format_translation[PIPE_FORMAT_COUNT];

   /* Whether vertex fetches don't have to be 4-byte-aligned. */
   /* TRUE if hardware supports it. */
   unsigned buffer_offset_unaligned:1;
   unsigned buffer_stride_unaligned:1;
   unsigned velem_src_offset_unaligned:1;

   /* Whether the driver supports user vertex buffers. */
   unsigned user_vertex_buffers:1;
};


boolean u_vbuf_get_caps(struct pipe_screen *screen, struct u_vbuf_caps *caps);

struct u_vbuf *
u_vbuf_create(struct pipe_context *pipe,
              struct u_vbuf_caps *caps, unsigned aux_vertex_buffer_index);

void u_vbuf_destroy(struct u_vbuf *mgr);

/* State and draw functions. */
void u_vbuf_set_vertex_elements(struct u_vbuf *mgr, unsigned count,
                                const struct pipe_vertex_element *states);
void u_vbuf_set_vertex_buffers(struct u_vbuf *mgr,
                               unsigned start_slot, unsigned count,
                               const struct pipe_vertex_buffer *bufs);
void u_vbuf_set_index_buffer(struct u_vbuf *mgr,
                             const struct pipe_index_buffer *ib);
void u_vbuf_draw_vbo(struct u_vbuf *mgr, const struct pipe_draw_info *info);

/* Save/restore functionality. */
void u_vbuf_save_vertex_elements(struct u_vbuf *mgr);
void u_vbuf_restore_vertex_elements(struct u_vbuf *mgr);
void u_vbuf_save_aux_vertex_buffer_slot(struct u_vbuf *mgr);
void u_vbuf_restore_aux_vertex_buffer_slot(struct u_vbuf *mgr);

#endif
