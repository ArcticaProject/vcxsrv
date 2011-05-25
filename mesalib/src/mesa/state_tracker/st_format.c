/**************************************************************************
 * 
 * Copyright 2007 Tungsten Graphics, Inc., Cedar Park, Texas.
 * Copyright (c) 2008-2010 VMware, Inc.
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


/**
 * Mesa / Gallium format conversion and format selection code.
 * \author Brian Paul
 */

#include "main/imports.h"
#include "main/context.h"
#include "main/texstore.h"
#include "main/image.h"
#include "main/macros.h"
#include "main/mfeatures.h"

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_screen.h"
#include "util/u_format.h"
#include "st_context.h"
#include "st_format.h"


static GLuint
format_max_bits(enum pipe_format format)
{
   GLuint size = util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 0);

   size = MAX2(size, util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 1));
   size = MAX2(size, util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 2));
   size = MAX2(size, util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 3));
   size = MAX2(size, util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_ZS, 0));
   size = MAX2(size, util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_ZS, 1));
   return size;
}


/**
 * Return basic GL datatype for the given gallium format.
 */
GLenum
st_format_datatype(enum pipe_format format)
{
   const struct util_format_description *desc;

   desc = util_format_description(format);
   assert(desc);

   if (desc->layout == UTIL_FORMAT_LAYOUT_PLAIN) {
      if (format == PIPE_FORMAT_B5G5R5A1_UNORM ||
          format == PIPE_FORMAT_B5G6R5_UNORM) {
         return GL_UNSIGNED_SHORT;
      }
      else if (format == PIPE_FORMAT_Z24_UNORM_S8_USCALED ||
               format == PIPE_FORMAT_S8_USCALED_Z24_UNORM ||
               format == PIPE_FORMAT_Z24X8_UNORM ||
               format == PIPE_FORMAT_X8Z24_UNORM) {
         return GL_UNSIGNED_INT_24_8;
      }
      else {
         const GLuint size = format_max_bits(format);
         if (size == 8) {
            if (desc->channel[0].type == UTIL_FORMAT_TYPE_UNSIGNED)
               return GL_UNSIGNED_BYTE;
            else
               return GL_BYTE;
         }
         else if (size == 16) {
            if (desc->channel[0].type == UTIL_FORMAT_TYPE_UNSIGNED)
               return GL_UNSIGNED_SHORT;
            else
               return GL_SHORT;
         }
         else {
            assert( size <= 32 );
            if (desc->channel[0].type == UTIL_FORMAT_TYPE_UNSIGNED)
               return GL_UNSIGNED_INT;
            else
               return GL_INT;
         }
      }
   }
   else if (format == PIPE_FORMAT_UYVY) {
      return GL_UNSIGNED_SHORT;
   }
   else if (format == PIPE_FORMAT_YUYV) {
      return GL_UNSIGNED_SHORT;
   }
   else {
      /* probably a compressed format, unsupported anyway */
      return GL_NONE;
   }
}


/**
 * Translate Mesa format to Gallium format.
 */
enum pipe_format
st_mesa_format_to_pipe_format(gl_format mesaFormat)
{
   switch (mesaFormat) {
   case MESA_FORMAT_RGBA8888:
      return PIPE_FORMAT_A8B8G8R8_UNORM;
   case MESA_FORMAT_RGBA8888_REV:
      return PIPE_FORMAT_R8G8B8A8_UNORM;
   case MESA_FORMAT_ARGB8888:
      return PIPE_FORMAT_B8G8R8A8_UNORM;
   case MESA_FORMAT_ARGB8888_REV:
      return PIPE_FORMAT_A8R8G8B8_UNORM;
   case MESA_FORMAT_XRGB8888:
      return PIPE_FORMAT_B8G8R8X8_UNORM;
   case MESA_FORMAT_XRGB8888_REV:
      return PIPE_FORMAT_X8R8G8B8_UNORM;
   case MESA_FORMAT_ARGB1555:
      return PIPE_FORMAT_B5G5R5A1_UNORM;
   case MESA_FORMAT_ARGB4444:
      return PIPE_FORMAT_B4G4R4A4_UNORM;
   case MESA_FORMAT_RGB565:
      return PIPE_FORMAT_B5G6R5_UNORM;
   case MESA_FORMAT_RGB332:
      return PIPE_FORMAT_B2G3R3_UNORM;
   case MESA_FORMAT_ARGB2101010:
      return PIPE_FORMAT_B10G10R10A2_UNORM;
   case MESA_FORMAT_AL44:
      return PIPE_FORMAT_L4A4_UNORM;
   case MESA_FORMAT_AL88:
      return PIPE_FORMAT_L8A8_UNORM;
   case MESA_FORMAT_AL1616:
      return PIPE_FORMAT_L16A16_UNORM;
   case MESA_FORMAT_A8:
      return PIPE_FORMAT_A8_UNORM;
   case MESA_FORMAT_A16:
      return PIPE_FORMAT_A16_UNORM;
   case MESA_FORMAT_L8:
      return PIPE_FORMAT_L8_UNORM;
   case MESA_FORMAT_L16:
      return PIPE_FORMAT_L16_UNORM;
   case MESA_FORMAT_I8:
      return PIPE_FORMAT_I8_UNORM;
   case MESA_FORMAT_I16:
      return PIPE_FORMAT_I16_UNORM;
   case MESA_FORMAT_Z16:
      return PIPE_FORMAT_Z16_UNORM;
   case MESA_FORMAT_Z32:
      return PIPE_FORMAT_Z32_UNORM;
   case MESA_FORMAT_Z24_S8:
      return PIPE_FORMAT_S8_USCALED_Z24_UNORM;
   case MESA_FORMAT_S8_Z24:
      return PIPE_FORMAT_Z24_UNORM_S8_USCALED;
   case MESA_FORMAT_Z24_X8:
      return PIPE_FORMAT_X8Z24_UNORM;
   case MESA_FORMAT_X8_Z24:
      return PIPE_FORMAT_Z24X8_UNORM;
   case MESA_FORMAT_S8:
      return PIPE_FORMAT_S8_USCALED;
   case MESA_FORMAT_YCBCR:
      return PIPE_FORMAT_UYVY;
#if FEATURE_texture_s3tc
   case MESA_FORMAT_RGB_DXT1:
      return PIPE_FORMAT_DXT1_RGB;
   case MESA_FORMAT_RGBA_DXT1:
      return PIPE_FORMAT_DXT1_RGBA;
   case MESA_FORMAT_RGBA_DXT3:
      return PIPE_FORMAT_DXT3_RGBA;
   case MESA_FORMAT_RGBA_DXT5:
      return PIPE_FORMAT_DXT5_RGBA;
#if FEATURE_EXT_texture_sRGB
   case MESA_FORMAT_SRGB_DXT1:
      return PIPE_FORMAT_DXT1_SRGB;
   case MESA_FORMAT_SRGBA_DXT1:
      return PIPE_FORMAT_DXT1_SRGBA;
   case MESA_FORMAT_SRGBA_DXT3:
      return PIPE_FORMAT_DXT3_SRGBA;
   case MESA_FORMAT_SRGBA_DXT5:
      return PIPE_FORMAT_DXT5_SRGBA;
#endif
#endif
#if FEATURE_EXT_texture_sRGB
   case MESA_FORMAT_SLA8:
      return PIPE_FORMAT_L8A8_SRGB;
   case MESA_FORMAT_SL8:
      return PIPE_FORMAT_L8_SRGB;
   case MESA_FORMAT_SRGB8:
      return PIPE_FORMAT_R8G8B8_SRGB;
   case MESA_FORMAT_SRGBA8:
      return PIPE_FORMAT_A8B8G8R8_SRGB;
   case MESA_FORMAT_SARGB8:
      return PIPE_FORMAT_B8G8R8A8_SRGB;
#endif
   case MESA_FORMAT_RGBA_FLOAT32:
      return PIPE_FORMAT_R32G32B32A32_FLOAT;
   case MESA_FORMAT_RGBA_FLOAT16:
      return PIPE_FORMAT_R16G16B16A16_FLOAT;
   case MESA_FORMAT_RGB_FLOAT32:
      return PIPE_FORMAT_R32G32B32_FLOAT;
   case MESA_FORMAT_RGB_FLOAT16:
      return PIPE_FORMAT_R16G16B16_FLOAT;
   case MESA_FORMAT_LUMINANCE_ALPHA_FLOAT32:
      return PIPE_FORMAT_L32A32_FLOAT;
   case MESA_FORMAT_LUMINANCE_ALPHA_FLOAT16:
      return PIPE_FORMAT_L16A16_FLOAT;
   case MESA_FORMAT_LUMINANCE_FLOAT32:
      return PIPE_FORMAT_L32_FLOAT;
   case MESA_FORMAT_LUMINANCE_FLOAT16:
      return PIPE_FORMAT_L16_FLOAT;
   case MESA_FORMAT_ALPHA_FLOAT32:
      return PIPE_FORMAT_A32_FLOAT;
   case MESA_FORMAT_ALPHA_FLOAT16:
      return PIPE_FORMAT_A16_FLOAT;
   case MESA_FORMAT_INTENSITY_FLOAT32:
      return PIPE_FORMAT_I32_FLOAT;
   case MESA_FORMAT_INTENSITY_FLOAT16:
      return PIPE_FORMAT_I16_FLOAT;
   case MESA_FORMAT_R_FLOAT32:
      return PIPE_FORMAT_R32_FLOAT;
   case MESA_FORMAT_R_FLOAT16:
      return PIPE_FORMAT_R16_FLOAT;
   case MESA_FORMAT_RG_FLOAT32:
      return PIPE_FORMAT_R32G32_FLOAT;
   case MESA_FORMAT_RG_FLOAT16:
      return PIPE_FORMAT_R16G16_FLOAT;

   case MESA_FORMAT_R8:
      return PIPE_FORMAT_R8_UNORM;
   case MESA_FORMAT_R16:
      return PIPE_FORMAT_R16_UNORM;
   case MESA_FORMAT_RG88:
      return PIPE_FORMAT_R8G8_UNORM;
   case MESA_FORMAT_RG1616:
      return PIPE_FORMAT_R16G16_UNORM;
   case MESA_FORMAT_RGBA_16:
      return PIPE_FORMAT_R16G16B16A16_UNORM;

   /* signed int formats */
   case MESA_FORMAT_RGBA_INT8:
      return PIPE_FORMAT_R8G8B8A8_SSCALED;
   case MESA_FORMAT_RGBA_INT16:
      return PIPE_FORMAT_R16G16B16A16_SSCALED;
   case MESA_FORMAT_RGBA_INT32:
      return PIPE_FORMAT_R32G32B32A32_SSCALED;

   /* unsigned int formats */
   case MESA_FORMAT_RGBA_UINT8:
      return PIPE_FORMAT_R8G8B8A8_USCALED;
   case MESA_FORMAT_RGBA_UINT16:
      return PIPE_FORMAT_R16G16B16A16_USCALED;
   case MESA_FORMAT_RGBA_UINT32:
      return PIPE_FORMAT_R32G32B32A32_USCALED;

   case MESA_FORMAT_RED_RGTC1:
      return PIPE_FORMAT_RGTC1_UNORM;
   case MESA_FORMAT_SIGNED_RED_RGTC1:
      return PIPE_FORMAT_RGTC1_SNORM;
   case MESA_FORMAT_RG_RGTC2:
      return PIPE_FORMAT_RGTC2_UNORM;
   case MESA_FORMAT_SIGNED_RG_RGTC2:
      return PIPE_FORMAT_RGTC2_SNORM;

   case MESA_FORMAT_L_LATC1:
      return PIPE_FORMAT_LATC1_UNORM;
   case MESA_FORMAT_SIGNED_L_LATC1:
      return PIPE_FORMAT_LATC1_SNORM;
   case MESA_FORMAT_LA_LATC2:
      return PIPE_FORMAT_LATC2_UNORM;
   case MESA_FORMAT_SIGNED_LA_LATC2:
      return PIPE_FORMAT_LATC2_SNORM;

   /* signed normalized formats */
   case MESA_FORMAT_SIGNED_R8:
      return PIPE_FORMAT_R8_SNORM;
   case MESA_FORMAT_SIGNED_RG88_REV:
      return PIPE_FORMAT_R8G8_SNORM;
   case MESA_FORMAT_SIGNED_RGBA8888_REV:
      return PIPE_FORMAT_R8G8B8A8_SNORM;

   case MESA_FORMAT_SIGNED_A8:
      return PIPE_FORMAT_A8_SNORM;
   case MESA_FORMAT_SIGNED_L8:
      return PIPE_FORMAT_L8_SNORM;
   case MESA_FORMAT_SIGNED_AL88:
      return PIPE_FORMAT_L8A8_SNORM;
   case MESA_FORMAT_SIGNED_I8:
      return PIPE_FORMAT_I8_SNORM;

   case MESA_FORMAT_SIGNED_R16:
      return PIPE_FORMAT_R16_SNORM;
   case MESA_FORMAT_SIGNED_GR1616:
      return PIPE_FORMAT_R16G16_SNORM;
   case MESA_FORMAT_SIGNED_RGBA_16:
      return PIPE_FORMAT_R16G16B16A16_SNORM;

   case MESA_FORMAT_SIGNED_A16:
      return PIPE_FORMAT_A16_SNORM;
   case MESA_FORMAT_SIGNED_L16:
      return PIPE_FORMAT_L16_SNORM;
   case MESA_FORMAT_SIGNED_AL1616:
      return PIPE_FORMAT_L16A16_SNORM;
   case MESA_FORMAT_SIGNED_I16:
      return PIPE_FORMAT_I16_SNORM;

   case MESA_FORMAT_RGB9_E5_FLOAT:
      return PIPE_FORMAT_R9G9B9E5_FLOAT;
   case MESA_FORMAT_R11_G11_B10_FLOAT:
      return PIPE_FORMAT_R11G11B10_FLOAT;

   default:
      assert(0);
      return PIPE_FORMAT_NONE;
   }
}


/**
 * Translate Gallium format to Mesa format.
 */
gl_format
st_pipe_format_to_mesa_format(enum pipe_format format)
{
   switch (format) {
   case PIPE_FORMAT_A8B8G8R8_UNORM:
      return MESA_FORMAT_RGBA8888;
   case PIPE_FORMAT_R8G8B8A8_UNORM:
      return MESA_FORMAT_RGBA8888_REV;
   case PIPE_FORMAT_B8G8R8A8_UNORM:
      return MESA_FORMAT_ARGB8888;
   case PIPE_FORMAT_A8R8G8B8_UNORM:
      return MESA_FORMAT_ARGB8888_REV;
   case PIPE_FORMAT_B8G8R8X8_UNORM:
      return MESA_FORMAT_XRGB8888;
   case PIPE_FORMAT_X8R8G8B8_UNORM:
      return MESA_FORMAT_XRGB8888_REV;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      return MESA_FORMAT_ARGB1555;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      return MESA_FORMAT_ARGB4444;
   case PIPE_FORMAT_B5G6R5_UNORM:
      return MESA_FORMAT_RGB565;
   case PIPE_FORMAT_B2G3R3_UNORM:
      return MESA_FORMAT_RGB332;
   case PIPE_FORMAT_B10G10R10A2_UNORM:
      return MESA_FORMAT_ARGB2101010;
   case PIPE_FORMAT_L4A4_UNORM:
      return MESA_FORMAT_AL44;
   case PIPE_FORMAT_L8A8_UNORM:
      return MESA_FORMAT_AL88;
   case PIPE_FORMAT_L16A16_UNORM:
      return MESA_FORMAT_AL1616;
   case PIPE_FORMAT_A8_UNORM:
      return MESA_FORMAT_A8;
   case PIPE_FORMAT_A16_UNORM:
      return MESA_FORMAT_A16;
   case PIPE_FORMAT_L8_UNORM:
      return MESA_FORMAT_L8;
   case PIPE_FORMAT_L16_UNORM:
      return MESA_FORMAT_L16;
   case PIPE_FORMAT_I8_UNORM:
      return MESA_FORMAT_I8;
   case PIPE_FORMAT_I16_UNORM:
      return MESA_FORMAT_I16;
   case PIPE_FORMAT_S8_USCALED:
      return MESA_FORMAT_S8;

   case PIPE_FORMAT_R16G16B16A16_UNORM:
      return MESA_FORMAT_RGBA_16;

   case PIPE_FORMAT_Z16_UNORM:
      return MESA_FORMAT_Z16;
   case PIPE_FORMAT_Z32_UNORM:
      return MESA_FORMAT_Z32;
   case PIPE_FORMAT_S8_USCALED_Z24_UNORM:
      return MESA_FORMAT_Z24_S8;
   case PIPE_FORMAT_X8Z24_UNORM:
      return MESA_FORMAT_Z24_X8;
   case PIPE_FORMAT_Z24X8_UNORM:
      return MESA_FORMAT_X8_Z24;
   case PIPE_FORMAT_Z24_UNORM_S8_USCALED:
      return MESA_FORMAT_S8_Z24;

   case PIPE_FORMAT_UYVY:
      return MESA_FORMAT_YCBCR;
   case PIPE_FORMAT_YUYV:
      return MESA_FORMAT_YCBCR_REV;

#if FEATURE_texture_s3tc
   case PIPE_FORMAT_DXT1_RGB:
      return MESA_FORMAT_RGB_DXT1;
   case PIPE_FORMAT_DXT1_RGBA:
      return MESA_FORMAT_RGBA_DXT1;
   case PIPE_FORMAT_DXT3_RGBA:
      return MESA_FORMAT_RGBA_DXT3;
   case PIPE_FORMAT_DXT5_RGBA:
      return MESA_FORMAT_RGBA_DXT5;
#if FEATURE_EXT_texture_sRGB
   case PIPE_FORMAT_DXT1_SRGB:
      return MESA_FORMAT_SRGB_DXT1;
   case PIPE_FORMAT_DXT1_SRGBA:
      return MESA_FORMAT_SRGBA_DXT1;
   case PIPE_FORMAT_DXT3_SRGBA:
      return MESA_FORMAT_SRGBA_DXT3;
   case PIPE_FORMAT_DXT5_SRGBA:
      return MESA_FORMAT_SRGBA_DXT5;
#endif
#endif

#if FEATURE_EXT_texture_sRGB
   case PIPE_FORMAT_L8A8_SRGB:
      return MESA_FORMAT_SLA8;
   case PIPE_FORMAT_L8_SRGB:
      return MESA_FORMAT_SL8;
   case PIPE_FORMAT_R8G8B8_SRGB:
      return MESA_FORMAT_SRGB8;
   case PIPE_FORMAT_A8B8G8R8_SRGB:
      return MESA_FORMAT_SRGBA8;
   case PIPE_FORMAT_B8G8R8A8_SRGB:
      return MESA_FORMAT_SARGB8;
#endif
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
      return MESA_FORMAT_RGBA_FLOAT32;
   case PIPE_FORMAT_R16G16B16A16_FLOAT:
      return MESA_FORMAT_RGBA_FLOAT16;
   case PIPE_FORMAT_R32G32B32_FLOAT:
      return MESA_FORMAT_RGB_FLOAT32;
   case PIPE_FORMAT_R16G16B16_FLOAT:
      return MESA_FORMAT_RGB_FLOAT16;
   case PIPE_FORMAT_L32A32_FLOAT:
      return MESA_FORMAT_LUMINANCE_ALPHA_FLOAT32;
   case PIPE_FORMAT_L16A16_FLOAT:
      return MESA_FORMAT_LUMINANCE_ALPHA_FLOAT16;
   case PIPE_FORMAT_L32_FLOAT:
      return MESA_FORMAT_LUMINANCE_FLOAT32;
   case PIPE_FORMAT_L16_FLOAT:
      return MESA_FORMAT_LUMINANCE_FLOAT16;
   case PIPE_FORMAT_A32_FLOAT:
      return MESA_FORMAT_ALPHA_FLOAT32;
   case PIPE_FORMAT_A16_FLOAT:
      return MESA_FORMAT_ALPHA_FLOAT16;
   case PIPE_FORMAT_I32_FLOAT:
      return MESA_FORMAT_INTENSITY_FLOAT32;
   case PIPE_FORMAT_I16_FLOAT:
      return MESA_FORMAT_INTENSITY_FLOAT16;
   case PIPE_FORMAT_R32_FLOAT:
      return MESA_FORMAT_R_FLOAT32;
   case PIPE_FORMAT_R16_FLOAT:
      return MESA_FORMAT_R_FLOAT16;
   case PIPE_FORMAT_R32G32_FLOAT:
      return MESA_FORMAT_RG_FLOAT32;
   case PIPE_FORMAT_R16G16_FLOAT:
      return MESA_FORMAT_RG_FLOAT16;

   case PIPE_FORMAT_R8_UNORM:
      return MESA_FORMAT_R8;
   case PIPE_FORMAT_R16_UNORM:
      return MESA_FORMAT_R16;
   case PIPE_FORMAT_R8G8_UNORM:
      return MESA_FORMAT_RG88;
   case PIPE_FORMAT_R16G16_UNORM:
      return MESA_FORMAT_RG1616;

   /* signed int formats */
   case PIPE_FORMAT_R8G8B8A8_SSCALED:
      return MESA_FORMAT_RGBA_INT8;
   case PIPE_FORMAT_R16G16B16A16_SSCALED:
      return MESA_FORMAT_RGBA_INT16;
   case PIPE_FORMAT_R32G32B32A32_SSCALED:
      return MESA_FORMAT_RGBA_INT32;

   /* unsigned int formats */
   case PIPE_FORMAT_R8G8B8A8_USCALED:
      return MESA_FORMAT_RGBA_UINT8;
   case PIPE_FORMAT_R16G16B16A16_USCALED:
      return MESA_FORMAT_RGBA_UINT16;
   case PIPE_FORMAT_R32G32B32A32_USCALED:
      return MESA_FORMAT_RGBA_UINT32;

   case PIPE_FORMAT_RGTC1_UNORM:
      return MESA_FORMAT_RED_RGTC1;
   case PIPE_FORMAT_RGTC1_SNORM:
      return MESA_FORMAT_SIGNED_RED_RGTC1;
   case PIPE_FORMAT_RGTC2_UNORM:
      return MESA_FORMAT_RG_RGTC2;
   case PIPE_FORMAT_RGTC2_SNORM:
      return MESA_FORMAT_SIGNED_RG_RGTC2;

   case PIPE_FORMAT_LATC1_UNORM:
      return MESA_FORMAT_L_LATC1;
   case PIPE_FORMAT_LATC1_SNORM:
      return MESA_FORMAT_SIGNED_L_LATC1;
   case PIPE_FORMAT_LATC2_UNORM:
      return MESA_FORMAT_LA_LATC2;
   case PIPE_FORMAT_LATC2_SNORM:
      return MESA_FORMAT_SIGNED_LA_LATC2;

   /* signed normalized formats */
   case PIPE_FORMAT_R8_SNORM:
      return MESA_FORMAT_SIGNED_R8;
   case PIPE_FORMAT_R8G8_SNORM:
      return MESA_FORMAT_SIGNED_RG88_REV;
   case PIPE_FORMAT_R8G8B8A8_SNORM:
      return MESA_FORMAT_SIGNED_RGBA8888_REV;

   case PIPE_FORMAT_A8_SNORM:
      return MESA_FORMAT_SIGNED_A8;
   case PIPE_FORMAT_L8_SNORM:
      return MESA_FORMAT_SIGNED_L8;
   case PIPE_FORMAT_L8A8_SNORM:
      return MESA_FORMAT_SIGNED_AL88;
   case PIPE_FORMAT_I8_SNORM:
      return MESA_FORMAT_SIGNED_I8;

   case PIPE_FORMAT_R16_SNORM:
      return MESA_FORMAT_SIGNED_R16;
   case PIPE_FORMAT_R16G16_SNORM:
      return MESA_FORMAT_SIGNED_GR1616;
   case PIPE_FORMAT_R16G16B16A16_SNORM:
      return MESA_FORMAT_SIGNED_RGBA_16;

   case PIPE_FORMAT_A16_SNORM:
      return MESA_FORMAT_SIGNED_A16;
   case PIPE_FORMAT_L16_SNORM:
      return MESA_FORMAT_SIGNED_L16;
   case PIPE_FORMAT_L16A16_SNORM:
      return MESA_FORMAT_SIGNED_AL1616;
   case PIPE_FORMAT_I16_SNORM:
      return MESA_FORMAT_SIGNED_I16;

   case PIPE_FORMAT_R9G9B9E5_FLOAT:
      return MESA_FORMAT_RGB9_E5_FLOAT;
   case PIPE_FORMAT_R11G11B10_FLOAT:
      return MESA_FORMAT_R11_G11_B10_FLOAT;

   default:
      assert(0);
      return MESA_FORMAT_NONE;
   }
}


/**
 * Map GL texture formats to Gallium pipe formats.
 */
struct format_mapping
{
   GLenum glFormats[18];       /**< list of GLenum formats, 0-terminated */
   enum pipe_format pipeFormats[10]; /**< list of pipe formats, 0-terminated */
};


#define DEFAULT_RGBA_FORMATS \
      PIPE_FORMAT_B8G8R8A8_UNORM, \
      PIPE_FORMAT_A8R8G8B8_UNORM, \
      PIPE_FORMAT_A8B8G8R8_UNORM, \
      PIPE_FORMAT_B5G6R5_UNORM, \
      0

#define DEFAULT_RGB_FORMATS \
      PIPE_FORMAT_B8G8R8X8_UNORM, \
      PIPE_FORMAT_X8R8G8B8_UNORM, \
      PIPE_FORMAT_X8B8G8R8_UNORM, \
      PIPE_FORMAT_B8G8R8A8_UNORM, \
      PIPE_FORMAT_A8R8G8B8_UNORM, \
      PIPE_FORMAT_A8B8G8R8_UNORM, \
      PIPE_FORMAT_B5G6R5_UNORM, \
      0

#define DEFAULT_SRGBA_FORMATS \
      PIPE_FORMAT_B8G8R8A8_SRGB, \
      PIPE_FORMAT_A8R8G8B8_SRGB, \
      PIPE_FORMAT_A8B8G8R8_SRGB, \
      0

/**
 * This table maps OpenGL texture format enums to Gallium pipe_format enums.
 * Multiple GL enums might map to multiple pipe_formats.
 * The first pipe format in the list that's supported is the one that's chosen.
 */
static struct format_mapping format_map[] = {
   /* Basic RGB, RGBA formats */
   {
      { GL_RGB10, GL_RGB10_A2, 0 },
      { PIPE_FORMAT_B10G10R10A2_UNORM, 0 }
   },
   {
      { 4, GL_RGBA, GL_RGBA8, 0 },
      { DEFAULT_RGBA_FORMATS, 0 }
   },
   {
      { GL_BGRA, 0 },
      { PIPE_FORMAT_B8G8R8A8_UNORM, DEFAULT_RGB_FORMATS, 0 }
   },
   {
      { 3, GL_RGB, GL_RGB8, 0 },
      { DEFAULT_RGB_FORMATS, 0 }
   },
   {
      { GL_RGB12, GL_RGB16, GL_RGBA12, GL_RGBA16, 0 },
      { PIPE_FORMAT_R16G16B16A16_UNORM, DEFAULT_RGB_FORMATS, 0 }
   },
   {
      { GL_RGBA4, GL_RGBA2, 0 },
      { PIPE_FORMAT_B4G4R4A4_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_RGB5_A1, 0 },
      { PIPE_FORMAT_B5G5R5A1_UNORM, DEFAULT_RGBA_FORMATS, 0 }
   },
   {
      { GL_R3_G3_B2, 0 },
      { PIPE_FORMAT_B2G3R3_UNORM, PIPE_FORMAT_B5G6R5_UNORM,
        PIPE_FORMAT_B5G5R5A1_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_RGB5, GL_RGB4 },
      { PIPE_FORMAT_B5G6R5_UNORM, PIPE_FORMAT_B5G5R5A1_UNORM,
        DEFAULT_RGBA_FORMATS }
   },

   /* basic Alpha formats */
   {
      { GL_ALPHA12, GL_ALPHA16, 0 },
      { PIPE_FORMAT_A16_UNORM, PIPE_FORMAT_A8_UNORM,
        DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_ALPHA, GL_ALPHA4, GL_ALPHA8, GL_COMPRESSED_ALPHA, 0 },
      { PIPE_FORMAT_A8_UNORM, DEFAULT_RGBA_FORMATS }
   },

   /* basic Luminance formats */
   {
      { GL_LUMINANCE12, GL_LUMINANCE16, 0 },
      { PIPE_FORMAT_L16_UNORM, PIPE_FORMAT_L8_UNORM, DEFAULT_RGBA_FORMATS, 0 }
   },
   {
      { 1, GL_LUMINANCE, GL_LUMINANCE4, GL_LUMINANCE8, 0 },
      { PIPE_FORMAT_L8_UNORM, DEFAULT_RGBA_FORMATS }
   },

   /* basic Luminance/Alpha formats */
   {
      { GL_LUMINANCE12_ALPHA4, GL_LUMINANCE12_ALPHA12,
        GL_LUMINANCE16_ALPHA16, 0},
      { PIPE_FORMAT_L16A16_UNORM, PIPE_FORMAT_L8A8_UNORM,
        PIPE_FORMAT_L8A8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { 2, GL_LUMINANCE_ALPHA, GL_LUMINANCE6_ALPHA2, GL_LUMINANCE8_ALPHA8, 0 },
      { PIPE_FORMAT_L8A8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_LUMINANCE4_ALPHA4, 0 },
      { PIPE_FORMAT_L4A4_UNORM, PIPE_FORMAT_L4A4_UNORM,
        DEFAULT_RGBA_FORMATS }
   },

   /* basic Intensity formats */
   {
      { GL_INTENSITY12, GL_INTENSITY16, 0 },
      { PIPE_FORMAT_I16_UNORM, PIPE_FORMAT_I8_UNORM, DEFAULT_RGBA_FORMATS }
   },
   {
      { GL_INTENSITY, GL_INTENSITY4, GL_INTENSITY8,
        GL_COMPRESSED_INTENSITY, 0 },
      { PIPE_FORMAT_I8_UNORM, DEFAULT_RGBA_FORMATS }
   },

   /* YCbCr */
   {
      { GL_YCBCR_MESA, 0 },
      { PIPE_FORMAT_UYVY, PIPE_FORMAT_YUYV, 0 }
   },

   /* compressed formats */ /* XXX PIPE_BIND_SAMPLER_VIEW only */
   {
      { GL_COMPRESSED_RGB, 0 },
      { PIPE_FORMAT_DXT1_RGB, 0 }
   },
   {
      { GL_COMPRESSED_RGBA, 0 },
      { PIPE_FORMAT_DXT5_RGBA, 0 }
   },
   {
      { GL_RGB_S3TC, GL_RGB4_S3TC, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 0 },
      { PIPE_FORMAT_DXT1_RGB, 0 }
   },
   {
      { GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0 },
      { PIPE_FORMAT_DXT1_RGBA, 0 }
   },
   {
      { GL_RGBA_S3TC, GL_RGBA4_S3TC, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0 },
      { PIPE_FORMAT_DXT3_RGBA, 0 }
   },
   {
      { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0 },
      { PIPE_FORMAT_DXT5_RGBA, 0 }
   },

#if 0
   {
      { GL_COMPRESSED_RGB_FXT1_3DFX, 0 },
      { PIPE_FORMAT_RGB_FXT1, 0 }
   },
   {
      { GL_COMPRESSED_RGBA_FXT1_3DFX, 0 },
      { PIPE_FORMAT_RGB_FXT1, 0 }
   },
#endif

   /* Depth formats */
   {
      { GL_DEPTH_COMPONENT16, 0 },
      { PIPE_FORMAT_Z16_UNORM, PIPE_FORMAT_Z24_UNORM_S8_USCALED,
        PIPE_FORMAT_S8_USCALED_Z24_UNORM, PIPE_FORMAT_Z32_UNORM, 0 }
   },
   {
      { GL_DEPTH_COMPONENT24, 0 },
      { PIPE_FORMAT_Z24X8_UNORM, PIPE_FORMAT_X8Z24_UNORM,
        PIPE_FORMAT_Z24_UNORM_S8_USCALED, PIPE_FORMAT_S8_USCALED_Z24_UNORM,
        PIPE_FORMAT_Z32_UNORM, 0 }
   },
   {
      { GL_DEPTH_COMPONENT32, 0 },
      { PIPE_FORMAT_Z32_UNORM, 0 }
   },
   {
      { GL_DEPTH_COMPONENT, 0 },
      { PIPE_FORMAT_Z24X8_UNORM, PIPE_FORMAT_X8Z24_UNORM,
        PIPE_FORMAT_Z32_UNORM, PIPE_FORMAT_Z16_UNORM,
        PIPE_FORMAT_Z24_UNORM_S8_USCALED, PIPE_FORMAT_S8_USCALED_Z24_UNORM, 0 }
   },

   /* stencil formats */
   {
      { GL_STENCIL_INDEX, GL_STENCIL_INDEX1_EXT, GL_STENCIL_INDEX4_EXT,
        GL_STENCIL_INDEX8_EXT, GL_STENCIL_INDEX16_EXT, 0 },
      {
         PIPE_FORMAT_S8_USCALED, PIPE_FORMAT_Z24_UNORM_S8_USCALED,
         PIPE_FORMAT_S8_USCALED_Z24_UNORM, 0
      }
   },

   /* Depth / Stencil formats */
   {
      { GL_DEPTH_STENCIL_EXT, GL_DEPTH24_STENCIL8_EXT, 0 },
      { PIPE_FORMAT_Z24_UNORM_S8_USCALED, PIPE_FORMAT_S8_USCALED_Z24_UNORM, 0 }
   },

   /* sRGB formats */
   {
      { GL_SRGB_EXT, GL_SRGB8_EXT, GL_SRGB_ALPHA_EXT, GL_SRGB8_ALPHA8_EXT, 0 },
      { DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_SRGB_EXT, GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 0 },
      { PIPE_FORMAT_DXT1_SRGB, DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0 },
      { PIPE_FORMAT_DXT1_SRGBA, 0 }
   },
   {
      { GL_COMPRESSED_SRGB_ALPHA_EXT,
        GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0 },
      { PIPE_FORMAT_DXT3_SRGBA, DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0 },
      { PIPE_FORMAT_DXT5_SRGBA, 0 }
   },
   {
      { GL_SLUMINANCE_ALPHA_EXT, GL_SLUMINANCE8_ALPHA8_EXT,
        GL_COMPRESSED_SLUMINANCE_EXT, GL_COMPRESSED_SLUMINANCE_ALPHA_EXT, 0 },
      { PIPE_FORMAT_L8A8_SRGB, DEFAULT_SRGBA_FORMATS }
   },
   {
      { GL_SLUMINANCE_EXT, GL_SLUMINANCE8_EXT, 0 },
      { PIPE_FORMAT_L8_SRGB, DEFAULT_SRGBA_FORMATS }
   },

   /* 16-bit float formats */
   {
      { GL_RGBA16F_ARB, 0 },
      { PIPE_FORMAT_R16G16B16A16_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_RGB16F_ARB, 0 },
      { PIPE_FORMAT_R16G16B16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_R32G32B32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA16F_ARB, 0 },
      { PIPE_FORMAT_L16A16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_ALPHA16F_ARB, 0 },
      { PIPE_FORMAT_A16_FLOAT, PIPE_FORMAT_L16A16_FLOAT,
        PIPE_FORMAT_A32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_INTENSITY16F_ARB, 0 },
      { PIPE_FORMAT_I16_FLOAT, PIPE_FORMAT_L16A16_FLOAT,
        PIPE_FORMAT_I32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_LUMINANCE16F_ARB, 0 },
      { PIPE_FORMAT_L16_FLOAT, PIPE_FORMAT_L16A16_FLOAT,
        PIPE_FORMAT_L32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_R16F, 0 },
      { PIPE_FORMAT_R16_FLOAT, PIPE_FORMAT_R16G16_FLOAT,
        PIPE_FORMAT_R32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_R32G32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },
   {
      { GL_RG16F, 0 },
      { PIPE_FORMAT_R16G16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT,
        PIPE_FORMAT_R32G32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT, 0 }
   },

   /* 32-bit float formats */
   {
      { GL_RGBA32F_ARB, 0 },
      { PIPE_FORMAT_R32G32B32A32_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT, 0 }
   },
   {
      { GL_RGB32F_ARB, 0 },
      { PIPE_FORMAT_R32G32B32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT,
        PIPE_FORMAT_R16G16B16A16_FLOAT, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA32F_ARB, 0 },
      { PIPE_FORMAT_L32A32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT,
        PIPE_FORMAT_L16A16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT, 0 }
   },
   {
      { GL_ALPHA32F_ARB, 0 },
      { PIPE_FORMAT_A32_FLOAT, PIPE_FORMAT_L32A32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, PIPE_FORMAT_A16_FLOAT,
        PIPE_FORMAT_L16A16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT, 0 }
   },
   {
      { GL_INTENSITY32F_ARB, 0 },
      { PIPE_FORMAT_I32_FLOAT, PIPE_FORMAT_L32A32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, PIPE_FORMAT_I16_FLOAT,
        PIPE_FORMAT_L16A16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT, 0 }
   },
   {
      { GL_LUMINANCE32F_ARB, 0 },
      { PIPE_FORMAT_L32_FLOAT, PIPE_FORMAT_L32A32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, PIPE_FORMAT_L16_FLOAT,
        PIPE_FORMAT_L16A16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT, 0 }
   },
   {
      { GL_R32F, 0 },
      { PIPE_FORMAT_R32_FLOAT, PIPE_FORMAT_R32G32_FLOAT,
        PIPE_FORMAT_R32G32B32A32_FLOAT, PIPE_FORMAT_R16_FLOAT,
        PIPE_FORMAT_R16G16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT, 0 }
   },
   {
      { GL_RG32F, 0 },
      { PIPE_FORMAT_R32G32_FLOAT, PIPE_FORMAT_R32G32B32A32_FLOAT,
        PIPE_FORMAT_R16G16_FLOAT, PIPE_FORMAT_R16G16B16A16_FLOAT, 0 }
   },

   /* R, RG formats */
   {
      { GL_RED, GL_R8, 0 },
      { PIPE_FORMAT_R8_UNORM, 0 }
   },
   {
      { GL_RG, GL_RG8, 0 },
      { PIPE_FORMAT_R8G8_UNORM, 0 }
   },
   {
      { GL_R16, 0 },
      { PIPE_FORMAT_R16_UNORM, 0 }
   },
   {
      { GL_RG16, 0 },
      { PIPE_FORMAT_R16G16_UNORM, 0 }
   },

   /* compressed R, RG formats */
   {
      { GL_COMPRESSED_RED, GL_COMPRESSED_RED_RGTC1, 0 },
      { PIPE_FORMAT_RGTC1_UNORM, PIPE_FORMAT_R8_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_RED_RGTC1, 0 },
      { PIPE_FORMAT_RGTC1_SNORM, 0 }
   },
   {
      { GL_COMPRESSED_RG, GL_COMPRESSED_RG_RGTC2, 0 },
      { PIPE_FORMAT_RGTC2_UNORM, PIPE_FORMAT_R8G8_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_RG_RGTC2, 0 },
      { PIPE_FORMAT_RGTC2_SNORM, 0 }
   },
   {
      { GL_COMPRESSED_LUMINANCE, GL_COMPRESSED_LUMINANCE_LATC1_EXT, 0 },
      { PIPE_FORMAT_LATC1_UNORM, PIPE_FORMAT_L8_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT, 0 },
      { PIPE_FORMAT_LATC1_SNORM, 0 }
   },
   {
      { GL_COMPRESSED_LUMINANCE_ALPHA, GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,
        GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI, 0 },
      { PIPE_FORMAT_LATC2_UNORM, PIPE_FORMAT_L8A8_UNORM, 0 }
   },
   {
      { GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT, 0 },
      { PIPE_FORMAT_LATC2_SNORM, 0 }
   },

   /* signed/unsigned integer formats.
    * XXX Mesa only has formats for RGBA signed/unsigned integer formats.
    * If/when new formats are added this code should be updated.
    */
   {
      { GL_RED_INTEGER_EXT,
        GL_GREEN_INTEGER_EXT,
        GL_BLUE_INTEGER_EXT,
        GL_ALPHA_INTEGER_EXT,
        GL_RGB_INTEGER_EXT,
        GL_RGBA_INTEGER_EXT,
        GL_BGR_INTEGER_EXT,
        GL_BGRA_INTEGER_EXT,
        GL_LUMINANCE_INTEGER_EXT,
        GL_LUMINANCE_ALPHA_INTEGER_EXT,
        GL_RGBA8I_EXT,
        GL_RGB8I_EXT,
        GL_ALPHA8I_EXT,
        GL_INTENSITY8I_EXT,
        GL_LUMINANCE8I_EXT,
        GL_LUMINANCE_ALPHA8I_EXT, 0 },
      { PIPE_FORMAT_R8G8B8A8_SSCALED, 0 }
   },
   {
      {
         GL_RGBA16I_EXT,
         GL_RGB16I_EXT,
         GL_ALPHA16I_EXT,
         GL_INTENSITY16I_EXT,
         GL_LUMINANCE16I_EXT,
         GL_LUMINANCE_ALPHA16I_EXT, 0 },
      { PIPE_FORMAT_R16G16B16A16_SSCALED, 0 },
   },
   {
      {
         GL_RGBA32I_EXT,
         GL_RGB32I_EXT,
         GL_ALPHA32I_EXT,
         GL_INTENSITY32I_EXT,
         GL_LUMINANCE32I_EXT,
         GL_LUMINANCE_ALPHA32I_EXT, 0 },
      { PIPE_FORMAT_R32G32B32A32_SSCALED, 0 }
   },
   {
      {
         GL_RGBA8UI_EXT,
         GL_RGB8UI_EXT,
         GL_ALPHA8UI_EXT,
         GL_INTENSITY8UI_EXT,
         GL_LUMINANCE8UI_EXT,
         GL_LUMINANCE_ALPHA8UI_EXT, 0 },
      { PIPE_FORMAT_R8G8B8A8_USCALED, 0 }
   },
   {
      {
         GL_RGBA16UI_EXT,
         GL_RGB16UI_EXT,
         GL_ALPHA16UI_EXT,
         GL_INTENSITY16UI_EXT,
         GL_LUMINANCE16UI_EXT,
         GL_LUMINANCE_ALPHA16UI_EXT, 0 },
      { PIPE_FORMAT_R16G16B16A16_USCALED, 0 }
   },
   {
      {
         GL_RGBA32UI_EXT,
         GL_RGB32UI_EXT,
         GL_ALPHA32UI_EXT,
         GL_INTENSITY32UI_EXT,
         GL_LUMINANCE32UI_EXT,
         GL_LUMINANCE_ALPHA32UI_EXT, 0 },
      { PIPE_FORMAT_R32G32B32A32_USCALED, 0 }
   },

   /* signed normalized formats */
   {
      { GL_RED_SNORM, GL_R8_SNORM, 0 },
      { PIPE_FORMAT_R8_SNORM, PIPE_FORMAT_R8G8_SNORM,
        PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_R16_SNORM, 0 },
      { PIPE_FORMAT_R16_SNORM,
        PIPE_FORMAT_R16G16_SNORM,
        PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_R8_SNORM,
        PIPE_FORMAT_R8G8_SNORM,
        PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RG_SNORM, GL_RG8_SNORM, 0 },
      { PIPE_FORMAT_R8G8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RG16_SNORM, 0 },
      { PIPE_FORMAT_R16G16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_R8G8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RGB_SNORM, GL_RGB8_SNORM, GL_RGBA_SNORM, GL_RGBA8_SNORM, 0 },
      { PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_RGB16_SNORM, GL_RGBA16_SNORM, 0 },
      { PIPE_FORMAT_R16G16B16A16_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_ALPHA_SNORM, GL_ALPHA8_SNORM, 0 },
      { PIPE_FORMAT_A8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_ALPHA16_SNORM, 0 },
      { PIPE_FORMAT_A16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_A8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_LUMINANCE_SNORM, GL_LUMINANCE8_SNORM, 0 },
      { PIPE_FORMAT_L8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_LUMINANCE16_SNORM, 0 },
      { PIPE_FORMAT_L16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_L8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_LUMINANCE_ALPHA_SNORM, GL_LUMINANCE8_ALPHA8_SNORM, 0 },
      { PIPE_FORMAT_L8A8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_LUMINANCE16_ALPHA16_SNORM, 0 },
      { PIPE_FORMAT_L16A16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_L8A8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_INTENSITY_SNORM, GL_INTENSITY8_SNORM, 0 },
      { PIPE_FORMAT_I8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   },
   {
      { GL_INTENSITY16_SNORM, 0 },
      { PIPE_FORMAT_I16_SNORM, PIPE_FORMAT_R16G16B16A16_SNORM,
        PIPE_FORMAT_I8_SNORM, PIPE_FORMAT_R8G8B8A8_SNORM, 0 }
   }
};


/**
 * Return first supported format from the given list.
 */
static enum pipe_format
find_supported_format(struct pipe_screen *screen, 
                      const enum pipe_format formats[],
                      enum pipe_texture_target target,
                      unsigned sample_count,
                      unsigned tex_usage)
{
   uint i;
   for (i = 0; formats[i]; i++) {
      if (screen->is_format_supported(screen, formats[i], target,
                                      sample_count, tex_usage)) {
         return formats[i];
      }
   }
   return PIPE_FORMAT_NONE;
}


/**
 * Given an OpenGL internalFormat value for a texture or surface, return
 * the best matching PIPE_FORMAT_x, or PIPE_FORMAT_NONE if there's no match.
 * This is called during glTexImage2D, for example.
 *
 * The bindings parameter typically has PIPE_BIND_SAMPLER_VIEW set, plus
 * either PIPE_BINDING_RENDER_TARGET or PIPE_BINDING_DEPTH_STENCIL if
 * we want render-to-texture ability.
 *
 * \param internalFormat  the user value passed to glTexImage2D
 * \param target  one of PIPE_TEXTURE_x
 * \param bindings  bitmask of PIPE_BIND_x flags.
 */
enum pipe_format
st_choose_format(struct pipe_screen *screen, GLenum internalFormat,
                 enum pipe_texture_target target, unsigned sample_count,
                 unsigned bindings)
{
   GET_CURRENT_CONTEXT(ctx); /* XXX this should be a function parameter */
   int i, j;

   /* can't render to compressed formats at this time */
   if (_mesa_is_compressed_format(ctx, internalFormat)
       && (bindings & ~PIPE_BIND_SAMPLER_VIEW)) {
      return PIPE_FORMAT_NONE;
   }

   /* search table for internalFormat */
   for (i = 0; i < Elements(format_map); i++) {
      const struct format_mapping *mapping = &format_map[i];
      for (j = 0; mapping->glFormats[j]; j++) {
         if (mapping->glFormats[j] == internalFormat) {
            /* Found the desired internal format.  Find first pipe format
             * which is supported by the driver.
             */
            return find_supported_format(screen, mapping->pipeFormats,
                                         target, sample_count, bindings);
         }
      }
   }

   _mesa_problem(NULL, "unhandled format!\n");
   return PIPE_FORMAT_NONE;
}


/**
 * Called by FBO code to choose a PIPE_FORMAT_ for drawing surfaces.
 */
enum pipe_format
st_choose_renderbuffer_format(struct pipe_screen *screen,
                              GLenum internalFormat, unsigned sample_count)
{
   uint usage;
   if (_mesa_is_depth_or_stencil_format(internalFormat))
      usage = PIPE_BIND_DEPTH_STENCIL;
   else
      usage = PIPE_BIND_RENDER_TARGET;
   return st_choose_format(screen, internalFormat, PIPE_TEXTURE_2D,
                           sample_count, usage);
}


/**
 * Called via ctx->Driver.chooseTextureFormat().
 */
gl_format
st_ChooseTextureFormat_renderable(struct gl_context *ctx, GLint internalFormat,
				  GLenum format, GLenum type, GLboolean renderable)
{
   struct pipe_screen *screen = st_context(ctx)->pipe->screen;
   enum pipe_format pFormat;
   uint bindings;

   (void) format;
   (void) type;

   /* GL textures may wind up being render targets, but we don't know
    * that in advance.  Specify potential render target flags now.
    */
   bindings = PIPE_BIND_SAMPLER_VIEW;
   if (renderable == GL_TRUE) {
      if (_mesa_is_depth_format(internalFormat) ||
	  _mesa_is_depth_or_stencil_format(internalFormat))
	 bindings |= PIPE_BIND_DEPTH_STENCIL;
      else 
	 bindings |= PIPE_BIND_RENDER_TARGET;
   }

   pFormat = st_choose_format(screen, internalFormat,
                              PIPE_TEXTURE_2D, 0, bindings);

   if (pFormat == PIPE_FORMAT_NONE) {
      /* try choosing format again, this time without render target bindings */
      pFormat = st_choose_format(screen, internalFormat,
                                 PIPE_TEXTURE_2D, 0, PIPE_BIND_SAMPLER_VIEW);
   }

   if (pFormat == PIPE_FORMAT_NONE) {
      /* no luck at all */
      return MESA_FORMAT_NONE;
   }

   return st_pipe_format_to_mesa_format(pFormat);
}

gl_format
st_ChooseTextureFormat(struct gl_context *ctx, GLint internalFormat,
                       GLenum format, GLenum type)
{
   boolean want_renderable =
      internalFormat == 3 || internalFormat == 4 ||
      internalFormat == GL_RGB || internalFormat == GL_RGBA ||
      internalFormat == GL_RGB8 || internalFormat == GL_RGBA8 ||
      internalFormat == GL_BGRA;

   return st_ChooseTextureFormat_renderable(ctx, internalFormat,
					    format, type, want_renderable);
}

/**
 * Test if a gallium format is equivalent to a GL format/type.
 */
GLboolean
st_equal_formats(enum pipe_format pFormat, GLenum format, GLenum type)
{
   switch (pFormat) {
   case PIPE_FORMAT_A8B8G8R8_UNORM:
      return format == GL_RGBA && type == GL_UNSIGNED_BYTE;
   case PIPE_FORMAT_A8R8G8B8_UNORM:
      return format == GL_BGRA && type == GL_UNSIGNED_BYTE;
   case PIPE_FORMAT_B5G6R5_UNORM:
      return format == GL_RGB && type == GL_UNSIGNED_SHORT_5_6_5;
   /* XXX more combos... */
   default:
      return GL_FALSE;
   }
}

GLboolean
st_sampler_compat_formats(enum pipe_format format1, enum pipe_format format2)
{
   if (format1 == format2)
      return GL_TRUE;

   if (format1 == PIPE_FORMAT_B8G8R8A8_UNORM &&
       format2 == PIPE_FORMAT_B8G8R8X8_UNORM)
      return GL_TRUE;

   if (format1 == PIPE_FORMAT_B8G8R8X8_UNORM &&
       format2 == PIPE_FORMAT_B8G8R8A8_UNORM)
      return GL_TRUE;

   if (format1 == PIPE_FORMAT_A8B8G8R8_UNORM &&
       format2 == PIPE_FORMAT_X8B8G8R8_UNORM)
      return GL_TRUE;

   if (format1 == PIPE_FORMAT_X8B8G8R8_UNORM &&
       format2 == PIPE_FORMAT_A8B8G8R8_UNORM)
      return GL_TRUE;

   if (format1 == PIPE_FORMAT_A8R8G8B8_UNORM &&
       format2 == PIPE_FORMAT_X8R8G8B8_UNORM)
      return GL_TRUE;

   if (format1 == PIPE_FORMAT_X8R8G8B8_UNORM &&
       format2 == PIPE_FORMAT_A8R8G8B8_UNORM)
      return GL_TRUE;

   return GL_FALSE;
}



/**
 * This is used for translating texture border color and the clear
 * color.  For example, the clear color is interpreted according to
 * the renderbuffer's base format.  For example, if clearing a
 * GL_LUMINANCE buffer, ClearColor[0] = luminance and ClearColor[1] =
 * alpha.  Similarly for texture border colors.
 */
void
st_translate_color(const GLfloat colorIn[4], GLenum baseFormat,
                   GLfloat colorOut[4])
{
   switch (baseFormat) {
   case GL_RED:
      colorOut[0] = colorIn[0];
      colorOut[1] = 0.0F;
      colorOut[2] = 0.0F;
      colorOut[3] = 1.0F;
      break;
   case GL_RG:
      colorOut[0] = colorIn[0];
      colorOut[1] = colorIn[1];
      colorOut[2] = 0.0F;
      colorOut[3] = 1.0F;
      break;
   case GL_RGB:
      colorOut[0] = colorIn[0];
      colorOut[1] = colorIn[1];
      colorOut[2] = colorIn[2];
      colorOut[3] = 1.0F;
      break;
   case GL_ALPHA:
      colorOut[0] = colorOut[1] = colorOut[2] = 0.0;
      colorOut[3] = colorIn[3];
      break;
   case GL_LUMINANCE:
      colorOut[0] = colorOut[1] = colorOut[2] = colorIn[0];
      colorOut[3] = 1.0;
      break;
   case GL_LUMINANCE_ALPHA:
      colorOut[0] = colorOut[1] = colorOut[2] = colorIn[0];
      colorOut[3] = colorIn[3];
      break;
   case GL_INTENSITY:
      colorOut[0] = colorOut[1] = colorOut[2] = colorOut[3] = colorIn[0];
      break;
   default:
      COPY_4V(colorOut, colorIn);
   }
}
