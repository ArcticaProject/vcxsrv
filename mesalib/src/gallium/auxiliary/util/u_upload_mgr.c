/**************************************************************************
 * 
 * Copyright 2009 VMware, Inc.
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
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/* Helper utility for uploading user buffers & other data, and
 * coalescing small buffers into larger ones.
 */

#include "pipe/p_defines.h"
#include "util/u_inlines.h"
#include "pipe/p_context.h"
#include "util/u_memory.h"
#include "util/u_math.h"

#include "u_upload_mgr.h"


struct u_upload_mgr {
   struct pipe_context *pipe;

   unsigned default_size;  /* Minimum size of the upload buffer, in bytes. */
   unsigned alignment;     /* Alignment of each sub-allocation. */
   unsigned bind;          /* Bitmask of PIPE_BIND_* flags. */
   unsigned map_flags;     /* Bitmask of PIPE_TRANSFER_* flags. */
   boolean map_persistent; /* If persistent mappings are supported. */

   struct pipe_resource *buffer;   /* Upload buffer. */
   struct pipe_transfer *transfer; /* Transfer object for the upload buffer. */
   uint8_t *map;    /* Pointer to the mapped upload buffer. */
   unsigned offset; /* Aligned offset to the upload buffer, pointing
                     * at the first unused byte. */
};


struct u_upload_mgr *u_upload_create( struct pipe_context *pipe,
                                      unsigned default_size,
                                      unsigned alignment,
                                      unsigned bind )
{
   struct u_upload_mgr *upload = CALLOC_STRUCT( u_upload_mgr );
   if (!upload)
      return NULL;

   upload->pipe = pipe;
   upload->default_size = default_size;
   upload->alignment = alignment;
   upload->bind = bind;

   upload->map_persistent =
      pipe->screen->get_param(pipe->screen,
                              PIPE_CAP_BUFFER_MAP_PERSISTENT_COHERENT);

   if (upload->map_persistent) {
      upload->map_flags = PIPE_TRANSFER_WRITE |
                          PIPE_TRANSFER_PERSISTENT |
                          PIPE_TRANSFER_COHERENT;
   }
   else {
      upload->map_flags = PIPE_TRANSFER_WRITE |
                          PIPE_TRANSFER_UNSYNCHRONIZED |
                          PIPE_TRANSFER_FLUSH_EXPLICIT;
   }

   return upload;
}


static void upload_unmap_internal(struct u_upload_mgr *upload, boolean destroying)
{
   if (!destroying && upload->map_persistent)
      return;

   if (upload->transfer) {
      struct pipe_box *box = &upload->transfer->box;

      if (!upload->map_persistent && (int) upload->offset > box->x) {
         pipe_buffer_flush_mapped_range(upload->pipe, upload->transfer,
                                        box->x, upload->offset - box->x);
      }

      pipe_transfer_unmap(upload->pipe, upload->transfer);
      upload->transfer = NULL;
      upload->map = NULL;
   }
}


void u_upload_unmap( struct u_upload_mgr *upload )
{
   upload_unmap_internal(upload, FALSE);
}


static void u_upload_release_buffer(struct u_upload_mgr *upload)
{
   /* Unmap and unreference the upload buffer. */
   upload_unmap_internal(upload, TRUE);
   pipe_resource_reference( &upload->buffer, NULL );
}


void u_upload_destroy( struct u_upload_mgr *upload )
{
   u_upload_release_buffer( upload );
   FREE( upload );
}


static enum pipe_error 
u_upload_alloc_buffer( struct u_upload_mgr *upload,
                       unsigned min_size )
{
   struct pipe_screen *screen = upload->pipe->screen;
   struct pipe_resource buffer;
   unsigned size;

   /* Release the old buffer, if present:
    */
   u_upload_release_buffer( upload );

   /* Allocate a new one: 
    */
   size = align(MAX2(upload->default_size, min_size), 4096);

   memset(&buffer, 0, sizeof buffer);
   buffer.target = PIPE_BUFFER;
   buffer.format = PIPE_FORMAT_R8_UNORM; /* want TYPELESS or similar */
   buffer.bind = upload->bind;
   buffer.usage = PIPE_USAGE_STREAM;
   buffer.width0 = size;
   buffer.height0 = 1;
   buffer.depth0 = 1;
   buffer.array_size = 1;

   if (upload->map_persistent) {
      buffer.flags = PIPE_RESOURCE_FLAG_MAP_PERSISTENT |
                     PIPE_RESOURCE_FLAG_MAP_COHERENT;
   }

   upload->buffer = screen->resource_create(screen, &buffer);
   if (upload->buffer == NULL) {
      return PIPE_ERROR_OUT_OF_MEMORY;
   }

   /* Map the new buffer. */
   upload->map = pipe_buffer_map_range(upload->pipe, upload->buffer,
                                       0, size, upload->map_flags,
                                       &upload->transfer);
   if (upload->map == NULL) {
      upload->transfer = NULL;
      pipe_resource_reference(&upload->buffer, NULL);
      return PIPE_ERROR_OUT_OF_MEMORY;
   }

   upload->offset = 0;
   return PIPE_OK;
}

enum pipe_error u_upload_alloc( struct u_upload_mgr *upload,
                                unsigned min_out_offset,
                                unsigned size,
                                unsigned *out_offset,
                                struct pipe_resource **outbuf,
                                void **ptr )
{
   unsigned alloc_size = align( size, upload->alignment );
   unsigned alloc_offset = align(min_out_offset, upload->alignment);
   unsigned offset;

   /* Init these return values here in case we fail below to make
    * sure the caller doesn't get garbage values.
    */
   *out_offset = ~0;
   pipe_resource_reference(outbuf, NULL);
   *ptr = NULL;

   /* Make sure we have enough space in the upload buffer
    * for the sub-allocation. */
   if (!upload->buffer ||
       MAX2(upload->offset, alloc_offset) + alloc_size > upload->buffer->width0) {
      enum pipe_error ret = u_upload_alloc_buffer(upload,
                                                  alloc_offset + alloc_size);
      if (ret != PIPE_OK)
         return ret;
   }

   offset = MAX2(upload->offset, alloc_offset);

   if (!upload->map) {
      upload->map = pipe_buffer_map_range(upload->pipe, upload->buffer,
                                          offset,
                                          upload->buffer->width0 - offset,
                                          upload->map_flags,
					  &upload->transfer);
      if (!upload->map) {
         upload->transfer = NULL;
         return PIPE_ERROR_OUT_OF_MEMORY;
      }

      upload->map -= offset;
   }

   assert(offset < upload->buffer->width0);
   assert(offset + size <= upload->buffer->width0);
   assert(size);

   /* Emit the return values: */
   *ptr = upload->map + offset;
   pipe_resource_reference( outbuf, upload->buffer );
   *out_offset = offset;

   upload->offset = offset + alloc_size;
   return PIPE_OK;
}

enum pipe_error u_upload_data( struct u_upload_mgr *upload,
                               unsigned min_out_offset,
                               unsigned size,
                               const void *data,
                               unsigned *out_offset,
                               struct pipe_resource **outbuf)
{
   uint8_t *ptr;
   enum pipe_error ret = u_upload_alloc(upload, min_out_offset, size,
                                        out_offset, outbuf,
                                        (void**)&ptr);
   if (ret != PIPE_OK)
      return ret;

   memcpy(ptr, data, size);
   return PIPE_OK;
}


/* As above, but upload the full contents of a buffer.  Useful for
 * uploading user buffers, avoids generating an explosion of GPU
 * buffers if you have an app that does lots of small vertex buffer
 * renders or DrawElements calls.
 */
enum pipe_error u_upload_buffer( struct u_upload_mgr *upload,
                                 unsigned min_out_offset,
                                 unsigned offset,
                                 unsigned size,
                                 struct pipe_resource *inbuf,
                                 unsigned *out_offset,
                                 struct pipe_resource **outbuf)
{
   enum pipe_error ret = PIPE_OK;
   struct pipe_transfer *transfer = NULL;
   const char *map = NULL;

   map = (const char *)pipe_buffer_map_range(upload->pipe,
                                             inbuf,
                                             offset, size,
                                             PIPE_TRANSFER_READ,
                                             &transfer);

   if (map == NULL) {
      return PIPE_ERROR_OUT_OF_MEMORY;
   }

   if (0)
      debug_printf("upload ptr %p ofs %d sz %d\n", map, offset, size);

   ret = u_upload_data( upload,
                        min_out_offset,
                        size,
                        map,
                        out_offset,
                        outbuf);

   pipe_buffer_unmap( upload->pipe, transfer );

   return ret;
}
