#include "pipe/p_context.h"
#include "util/u_rect.h"
#include "util/u_inlines.h"
#include "util/u_transfer.h"
#include "util/u_memory.h"

/* One-shot transfer operation with data supplied in a user
 * pointer.  XXX: strides??
 */
void u_default_transfer_inline_write( struct pipe_context *pipe,
                                      struct pipe_resource *resource,
                                      unsigned level,
                                      unsigned usage,
                                      const struct pipe_box *box,
                                      const void *data,
                                      unsigned stride,
                                      unsigned layer_stride)
{
   struct pipe_transfer *transfer = NULL;
   uint8_t *map = NULL;

   assert(!(usage & PIPE_TRANSFER_READ));

   /* the write flag is implicit by the nature of transfer_inline_write */
   usage |= PIPE_TRANSFER_WRITE;

   /* transfer_inline_write implicitly discards the rewritten buffer range */
   if (box->x == 0 && box->width == resource->width0) {
      usage |= PIPE_TRANSFER_DISCARD_WHOLE_RESOURCE;
   } else {
      usage |= PIPE_TRANSFER_DISCARD_RANGE;
   }

   transfer = pipe->get_transfer(pipe,
                                 resource,
                                 level,
                                 usage,
                                 box );
   if (transfer == NULL)
      goto out;

   map = pipe_transfer_map(pipe, transfer);
   if (map == NULL)
      goto out;

   if (resource->target == PIPE_BUFFER) {
      assert(box->height == 1);
      assert(box->depth == 1);

      memcpy(map, data, box->width);
   }
   else {
      const uint8_t *src_data = data;
      unsigned i;

      for (i = 0; i < box->depth; i++) {
         util_copy_rect(map,
                        resource->format,
                        transfer->stride, /* bytes */
                        0, 0,
                        box->width,
                        box->height,
                        src_data,
                        stride,       /* bytes */
                        0, 0);
         map += transfer->layer_stride;
         src_data += layer_stride;
      }
   }

out:
   if (map)
      pipe_transfer_unmap(pipe, transfer);

   if (transfer)
      pipe_transfer_destroy(pipe, transfer);
}


boolean u_default_resource_get_handle(struct pipe_screen *screen,
                                      struct pipe_resource *resource,
                                      struct winsys_handle *handle)
{
   return FALSE;
}



void u_default_transfer_flush_region( struct pipe_context *pipe,
                                      struct pipe_transfer *transfer,
                                      const struct pipe_box *box)
{
   /* This is a no-op implementation, nothing to do.
    */
}

struct pipe_transfer * u_default_get_transfer(struct pipe_context *context,
                                              struct pipe_resource *resource,
                                              unsigned level,
                                              unsigned usage,
                                              const struct pipe_box *box)
{
   struct pipe_transfer *transfer = CALLOC_STRUCT(pipe_transfer);
   if (transfer == NULL)
      return NULL;

   transfer->resource = resource;
   transfer->level = level;
   transfer->usage = usage;
   transfer->box = *box;

   /* Note strides are zero, this is ok for buffers, but not for
    * textures 2d & higher at least. 
    */
   return transfer;
}

void u_default_transfer_unmap( struct pipe_context *pipe,
                               struct pipe_transfer *transfer )
{
}

void u_default_transfer_destroy(struct pipe_context *pipe,
                                struct pipe_transfer *transfer)
{
   FREE(transfer);
}
