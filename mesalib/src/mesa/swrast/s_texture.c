/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2011 VMware, Inc.
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
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * Functions for mapping/unmapping texture images.
 */


#include "main/context.h"
#include "main/fbobject.h"
#include "swrast/swrast.h"
#include "swrast/s_context.h"

/**
 * Error checking for debugging only.
 */
static void
_mesa_check_map_teximage(struct gl_texture_image *texImage,
                         GLuint slice, GLuint x, GLuint y, GLuint w, GLuint h)
{

   if (texImage->TexObject->Target == GL_TEXTURE_1D)
      assert(y == 0 && h == 1);

   assert(x < texImage->Width || texImage->Width == 0);
   assert(y < texImage->Height || texImage->Height == 0);
   assert(x + w <= texImage->Width);
   assert(y + h <= texImage->Height);
}

/**
 * Map a 2D slice of a texture image into user space.
 * (x,y,w,h) defines a region of interest (ROI).  Reading/writing texels
 * outside of the ROI is undefined.
 *
 * \param texImage  the texture image
 * \param slice  the 3D image slice or array texture slice
 * \param x, y, w, h  region of interest
 * \param mode  bitmask of GL_MAP_READ_BIT, GL_MAP_WRITE_BIT
 * \param mapOut  returns start of mapping of region of interest
 * \param rowStrideOut  returns row stride (in bytes)
 */
void
_swrast_map_teximage(struct gl_context *ctx,
                     struct gl_texture_image *texImage,
                     GLuint slice,
                     GLuint x, GLuint y, GLuint w, GLuint h,
                     GLbitfield mode,
                     GLubyte **mapOut,
                     GLint *rowStrideOut)
{
   GLubyte *map;
   GLint stride, texelSize;
   GLuint bw, bh;

   _mesa_check_map_teximage(texImage, slice, x, y, w, h);

   texelSize = _mesa_get_format_bytes(texImage->TexFormat);
   stride = _mesa_format_row_stride(texImage->TexFormat, texImage->Width);
   _mesa_get_format_block_size(texImage->TexFormat, &bw, &bh);

   assert(texImage->Data);

   map = texImage->Data;

   if (texImage->TexObject->Target == GL_TEXTURE_3D ||
       texImage->TexObject->Target == GL_TEXTURE_2D_ARRAY) {
      GLuint sliceSize = _mesa_format_image_size(texImage->TexFormat,
                                                 texImage->Width,
                                                 texImage->Height,
                                                 1);
      assert(slice < texImage->Depth);
      map += slice * sliceSize;
   }

   /* apply x/y offset to map address */
   map += stride * (y / bh) + texelSize * (x / bw);

   *mapOut = map;
   *rowStrideOut = stride;
}

void
_swrast_unmap_teximage(struct gl_context *ctx,
                       struct gl_texture_image *texImage,
                       GLuint slice)
{
   /* nop */
}
