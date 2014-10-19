/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc.
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

#ifndef ST_TEXTURE_H
#define ST_TEXTURE_H


#include "pipe/p_context.h"
#include "util/u_sampler.h"

#include "main/mtypes.h"


struct pipe_resource;


struct st_texture_image_transfer {
   struct pipe_transfer *transfer;

   /* For ETC fallback. */
   GLubyte *temp_data; /**< Temporary ETC texture storage. */
   unsigned temp_stride; /**< Stride of the ETC texture storage. */
   GLubyte *map; /**< Saved map pointer of the uncompressed transfer. */
};


/**
 * Subclass of gl_texure_image.
 */
struct st_texture_image
{
   struct gl_texture_image base;

   /** Used to store texture data that doesn't fit in the parent
    * object's mipmap buffer.
    */
   GLubyte *TexData;

   /* If stImage->pt != NULL, image data is stored here.
    * Else if stImage->TexData != NULL, image is stored there.
    * Else there is no image data.
    */
   struct pipe_resource *pt;

   /* List of transfers, allocated on demand.
    * transfer[layer] is a mapping for that layer.
    */
   struct st_texture_image_transfer *transfer;
   unsigned num_transfers;
};


/**
 * Subclass of gl_texure_object.
 */
struct st_texture_object
{
   struct gl_texture_object base;       /* The "parent" object */

   /* The texture must include at levels [0..lastLevel] once validated:
    */
   GLuint lastLevel;

   /** The size of the level=0 mipmap image.
    * Note that the number of 1D array layers will be in height0 and the
    * number of 2D array layers will be in depth0, as in GL.
    */
   GLuint width0, height0, depth0;

   /* On validation any active images held in main memory or in other
    * textures will be copied to this texture and the old storage freed.
    */
   struct pipe_resource *pt;

   /* Number of views in sampler_views array */
   GLuint num_sampler_views;

   /* Array of sampler views (one per context) attached to this texture
    * object. Created lazily on first binding in context.
    */
   struct pipe_sampler_view **sampler_views;

   /* True if this texture comes from the window system. Such a texture
    * cannot be reallocated and the format can only be changed with a sampler
    * view or a surface.
    */
   GLboolean surface_based;

   /* If surface_based is true, this format should be used for all sampler
    * views and surfaces instead of pt->format.
    */
   enum pipe_format surface_format;
};


static INLINE struct st_texture_image *
st_texture_image(struct gl_texture_image *img)
{
   return (struct st_texture_image *) img;
}

static INLINE struct st_texture_object *
st_texture_object(struct gl_texture_object *obj)
{
   return (struct st_texture_object *) obj;
}

static INLINE const struct st_texture_object *
st_texture_object_const(const struct gl_texture_object *obj)
{
   return (const struct st_texture_object *) obj;
}


static INLINE struct pipe_resource *
st_get_texobj_resource(struct gl_texture_object *texObj)
{
   struct st_texture_object *stObj = st_texture_object(texObj);
   return stObj ? stObj->pt : NULL;
}


static INLINE struct pipe_resource *
st_get_stobj_resource(struct st_texture_object *stObj)
{
   return stObj ? stObj->pt : NULL;
}


static INLINE struct pipe_sampler_view *
st_create_texture_sampler_view_format(struct pipe_context *pipe,
                                      struct pipe_resource *texture,
                                      enum pipe_format format)
{
   struct pipe_sampler_view templ;

   u_sampler_view_default_template(&templ, texture, format);

   return pipe->create_sampler_view(pipe, texture, &templ);
}

static INLINE struct pipe_sampler_view *
st_create_texture_sampler_view(struct pipe_context *pipe,
                               struct pipe_resource *texture)
{
   return st_create_texture_sampler_view_format(pipe, texture,
                                                texture->format);
}



extern struct pipe_resource *
st_texture_create(struct st_context *st,
                  enum pipe_texture_target target,
		  enum pipe_format format,
                  GLuint last_level,
                  GLuint width0,
                  GLuint height0,
                  GLuint depth0,
                  GLuint layers,
                  GLuint nr_samples,
                  GLuint tex_usage );


extern void
st_gl_texture_dims_to_pipe_dims(GLenum texture,
                                GLuint widthIn,
                                GLuint heightIn,
                                GLuint depthIn,
                                GLuint *widthOut,
                                GLuint *heightOut,
                                GLuint *depthOut,
                                GLuint *layersOut);

/* Check if an image fits into an existing texture object.
 */
extern GLboolean
st_texture_match_image(struct st_context *st,
                       const struct pipe_resource *pt,
                       const struct gl_texture_image *image);

/* Return a pointer to an image within a texture.  Return image stride as
 * well.
 */
extern GLubyte *
st_texture_image_map(struct st_context *st, struct st_texture_image *stImage,
                     enum pipe_transfer_usage usage,
                     GLuint x, GLuint y, GLuint z,
                     GLuint w, GLuint h, GLuint d,
                     struct pipe_transfer **transfer);

extern void
st_texture_image_unmap(struct st_context *st,
                       struct st_texture_image *stImage, unsigned slice);


/* Return pointers to each 2d slice within an image.  Indexed by depth
 * value.
 */
extern const GLuint *
st_texture_depth_offsets(struct pipe_resource *pt, GLuint level);


/* Upload an image into a texture
 */
extern void
st_texture_image_data(struct st_context *st,
                      struct pipe_resource *dst,
                      GLuint face, GLuint level, void *src,
                      GLuint src_row_pitch, GLuint src_image_pitch);


/* Copy an image between two textures
 */
extern void
st_texture_image_copy(struct pipe_context *pipe,
                      struct pipe_resource *dst, GLuint dstLevel,
                      struct pipe_resource *src, GLuint srcLevel,
                      GLuint face);


extern struct pipe_resource *
st_create_color_map_texture(struct gl_context *ctx);

extern struct pipe_sampler_view **
st_texture_get_sampler_view(struct st_context *st,
                            struct st_texture_object *stObj);

extern void
st_texture_release_sampler_view(struct st_context *st,
                                struct st_texture_object *stObj);

extern void
st_texture_release_all_sampler_views(struct st_context *st,
                                     struct st_texture_object *stObj);

void
st_texture_free_sampler_views(struct st_texture_object *stObj);

#endif
