/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (c) 2008-2009  VMware, Inc.
 * Copyright (c) 2012 Intel Corporation
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


#include "context.h"
#include "glformats.h"


/**
 * \return GL_TRUE if type is packed pixel type, GL_FALSE otherwise.
 */
GLboolean
_mesa_type_is_packed(GLenum type)
{
   switch (type) {
   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
   case MESA_UNSIGNED_BYTE_4_4:
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
   case GL_UNSIGNED_SHORT_8_8_MESA:
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:
   case GL_UNSIGNED_INT_24_8_EXT:
   case GL_UNSIGNED_INT_5_9_9_9_REV:
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      return GL_TRUE;
   }

   return GL_FALSE;
}


/**
 * Get the size of a GL data type.
 *
 * \param type GL data type.
 *
 * \return the size, in bytes, of the given data type, 0 if a GL_BITMAP, or -1
 * if an invalid type enum.
 */
GLint
_mesa_sizeof_type(GLenum type)
{
   switch (type) {
   case GL_BITMAP:
      return 0;
   case GL_UNSIGNED_BYTE:
      return sizeof(GLubyte);
   case GL_BYTE:
      return sizeof(GLbyte);
   case GL_UNSIGNED_SHORT:
      return sizeof(GLushort);
   case GL_SHORT:
      return sizeof(GLshort);
   case GL_UNSIGNED_INT:
      return sizeof(GLuint);
   case GL_INT:
      return sizeof(GLint);
   case GL_FLOAT:
      return sizeof(GLfloat);
   case GL_DOUBLE:
      return sizeof(GLdouble);
   case GL_HALF_FLOAT_ARB:
      return sizeof(GLhalfARB);
   case GL_FIXED:
      return sizeof(GLfixed);
   default:
      return -1;
   }
}


/**
 * Same as _mesa_sizeof_type() but also accepting the packed pixel
 * format data types.
 */
GLint
_mesa_sizeof_packed_type(GLenum type)
{
   switch (type) {
   case GL_BITMAP:
      return 0;
   case GL_UNSIGNED_BYTE:
      return sizeof(GLubyte);
   case GL_BYTE:
      return sizeof(GLbyte);
   case GL_UNSIGNED_SHORT:
      return sizeof(GLushort);
   case GL_SHORT:
      return sizeof(GLshort);
   case GL_UNSIGNED_INT:
      return sizeof(GLuint);
   case GL_INT:
      return sizeof(GLint);
   case GL_HALF_FLOAT_ARB:
      return sizeof(GLhalfARB);
   case GL_FLOAT:
      return sizeof(GLfloat);
   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
   case MESA_UNSIGNED_BYTE_4_4:
      return sizeof(GLubyte);
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
   case GL_UNSIGNED_SHORT_8_8_MESA:
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:
      return sizeof(GLushort);
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
   case GL_UNSIGNED_INT_24_8_EXT:
   case GL_UNSIGNED_INT_5_9_9_9_REV:
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
      return sizeof(GLuint);
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      return 8;
   default:
      return -1;
   }
}


/**
 * Get the number of components in a pixel format.
 *
 * \param format pixel format.
 *
 * \return the number of components in the given format, or -1 if a bad format.
 */
GLint
_mesa_components_in_format(GLenum format)
{
   switch (format) {
   case GL_COLOR_INDEX:
   case GL_STENCIL_INDEX:
   case GL_DEPTH_COMPONENT:
   case GL_RED:
   case GL_RED_INTEGER_EXT:
   case GL_GREEN:
   case GL_GREEN_INTEGER_EXT:
   case GL_BLUE:
   case GL_BLUE_INTEGER_EXT:
   case GL_ALPHA:
   case GL_ALPHA_INTEGER_EXT:
   case GL_LUMINANCE:
   case GL_LUMINANCE_INTEGER_EXT:
   case GL_INTENSITY:
      return 1;

   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
   case GL_RG:
   case GL_YCBCR_MESA:
   case GL_DEPTH_STENCIL_EXT:
   case GL_RG_INTEGER:
      return 2;

   case GL_RGB:
   case GL_BGR:
   case GL_RGB_INTEGER_EXT:
   case GL_BGR_INTEGER_EXT:
      return 3;

   case GL_RGBA:
   case GL_BGRA:
   case GL_ABGR_EXT:
   case GL_RGBA_INTEGER_EXT:
   case GL_BGRA_INTEGER_EXT:
      return 4;

   default:
      return -1;
   }
}


/**
 * Get the bytes per pixel of pixel format type pair.
 *
 * \param format pixel format.
 * \param type pixel type.
 *
 * \return bytes per pixel, or -1 if a bad format or type was given.
 */
GLint
_mesa_bytes_per_pixel(GLenum format, GLenum type)
{
   GLint comps = _mesa_components_in_format(format);
   if (comps < 0)
      return -1;

   switch (type) {
   case GL_BITMAP:
      return 0;  /* special case */
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      return comps * sizeof(GLubyte);
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
      return comps * sizeof(GLshort);
   case GL_INT:
   case GL_UNSIGNED_INT:
      return comps * sizeof(GLint);
   case GL_FLOAT:
      return comps * sizeof(GLfloat);
   case GL_HALF_FLOAT_ARB:
      return comps * sizeof(GLhalfARB);
   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
      if (format == GL_RGB || format == GL_BGR ||
          format == GL_RGB_INTEGER_EXT || format == GL_BGR_INTEGER_EXT)
         return sizeof(GLubyte);
      else
         return -1;  /* error */
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
      if (format == GL_RGB || format == GL_BGR ||
          format == GL_RGB_INTEGER_EXT || format == GL_BGR_INTEGER_EXT)
         return sizeof(GLushort);
      else
         return -1;  /* error */
   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
      if (format == GL_RGBA || format == GL_BGRA || format == GL_ABGR_EXT ||
          format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT)
         return sizeof(GLushort);
      else
         return -1;
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      if (format == GL_RGBA || format == GL_BGRA || format == GL_ABGR_EXT ||
          format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT ||
          format == GL_RGB)
         return sizeof(GLuint);
      else
         return -1;
   case GL_UNSIGNED_SHORT_8_8_MESA:
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:
      if (format == GL_YCBCR_MESA)
         return sizeof(GLushort);
      else
         return -1;
   case GL_UNSIGNED_INT_24_8_EXT:
      if (format == GL_DEPTH_STENCIL_EXT)
         return sizeof(GLuint);
      else
         return -1;
   case GL_UNSIGNED_INT_5_9_9_9_REV:
      if (format == GL_RGB)
         return sizeof(GLuint);
      else
         return -1;
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
      if (format == GL_RGB)
         return sizeof(GLuint);
      else
         return -1;
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      if (format == GL_DEPTH_STENCIL)
         return 8;
      else
         return -1;
   default:
      return -1;
   }
}


/**
 * Get the number of bytes for a vertex attrib with the given number of
 * components and type.
 *
 * \param comps number of components.
 * \param type data type.
 *
 * \return bytes per attribute, or -1 if a bad comps/type combination was given.
 */
GLint
_mesa_bytes_per_vertex_attrib(GLint comps, GLenum type)
{
   switch (type) {
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      return comps * sizeof(GLubyte);
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
      return comps * sizeof(GLshort);
   case GL_INT:
   case GL_UNSIGNED_INT:
      return comps * sizeof(GLint);
   case GL_FLOAT:
      return comps * sizeof(GLfloat);
   case GL_HALF_FLOAT_ARB:
      return comps * sizeof(GLhalfARB);
   case GL_DOUBLE:
      return comps * sizeof(GLdouble);
   case GL_FIXED:
      return comps * sizeof(GLfixed);
   case GL_INT_2_10_10_10_REV:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      if (comps == 4)
         return sizeof(GLuint);
      else
         return -1;
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
      if (comps == 3)
         return sizeof(GLuint);
      else
         return -1;
   default:
      return -1;
   }
}

/**
 * Test if the given format is unsized.
 */
GLboolean
_mesa_is_enum_format_unsized(GLenum format)
{
   switch (format) {
   case GL_RGBA:
   case GL_BGRA:
   case GL_ABGR_EXT:
   case GL_RGB:
   case GL_BGR:
   case GL_RG:
   case GL_RED:
   case GL_GREEN:
   case GL_BLUE:
   case GL_ALPHA:
   case GL_INTENSITY:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:

   case GL_SRGB:
   case GL_SRGB_ALPHA:
   case GL_SLUMINANCE:
   case GL_SLUMINANCE_ALPHA:

   case GL_RGBA_SNORM:
   case GL_RGB_SNORM:
   case GL_RG_SNORM:
   case GL_RED_SNORM:
   case GL_ALPHA_SNORM:
   case GL_INTENSITY_SNORM:
   case GL_LUMINANCE_SNORM:
   case GL_LUMINANCE_ALPHA_SNORM:

   case GL_RED_INTEGER:
   case GL_GREEN_INTEGER:
   case GL_BLUE_INTEGER:
   case GL_ALPHA_INTEGER:
   case GL_RGB_INTEGER:
   case GL_RGBA_INTEGER:
   case GL_BGR_INTEGER:
   case GL_BGRA_INTEGER:
   case GL_RG_INTEGER:
   case GL_LUMINANCE_INTEGER_EXT:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:

   case GL_DEPTH_COMPONENT:
   case GL_DEPTH_STENCIL:
   case GL_STENCIL_INDEX:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}

/**
 * Test if the given format is a UNORM (unsigned-normalized) format.
 */
GLboolean
_mesa_is_enum_format_unorm(GLenum format)
{
      switch(format) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA8:
      case GL_ALPHA12:
      case GL_ALPHA16:
      case 1:
      case GL_LUMINANCE:
      case GL_SLUMINANCE:
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
      case 2:
      case GL_LUMINANCE_ALPHA:
      case GL_SLUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
      case GL_INTENSITY:
      case GL_INTENSITY4:
      case GL_INTENSITY8:
      case GL_INTENSITY12:
      case GL_INTENSITY16:
      case GL_R8:
      case GL_R16:
      case GL_RG:
      case GL_RG8:
      case GL_RG16:
      case 3:
      case GL_RGB:
      case GL_BGR:
      case GL_SRGB:
      case GL_R3_G3_B2:
      case GL_RGB4:
      case GL_RGB5:
      case GL_RGB565:
      case GL_RGB8:
      case GL_RGB10:
      case GL_RGB12:
      case GL_RGB16:
      case 4:
      case GL_ABGR_EXT:
      case GL_RGBA:
      case GL_BGRA:
      case GL_SRGB_ALPHA:
      case GL_RGBA2:
      case GL_RGBA4:
      case GL_RGB5_A1:
      case GL_RGBA8:
      case GL_RGB10_A2:
      case GL_RGBA12:
      case GL_RGBA16:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}

/**
 * Test if the given format is a SNORM (signed-normalized) format.
 */
GLboolean
_mesa_is_enum_format_snorm(GLenum format)
{
   switch (format) {
   /* signed, normalized texture formats */
   case GL_RED_SNORM:
   case GL_R8_SNORM:
   case GL_R16_SNORM:
   case GL_RG_SNORM:
   case GL_RG8_SNORM:
   case GL_RG16_SNORM:
   case GL_RGB_SNORM:
   case GL_RGB8_SNORM:
   case GL_RGB16_SNORM:
   case GL_RGBA_SNORM:
   case GL_RGBA8_SNORM:
   case GL_RGBA16_SNORM:
   case GL_ALPHA_SNORM:
   case GL_ALPHA8_SNORM:
   case GL_ALPHA16_SNORM:
   case GL_LUMINANCE_SNORM:
   case GL_LUMINANCE8_SNORM:
   case GL_LUMINANCE16_SNORM:
   case GL_LUMINANCE_ALPHA_SNORM:
   case GL_LUMINANCE8_ALPHA8_SNORM:
   case GL_LUMINANCE16_ALPHA16_SNORM:
   case GL_INTENSITY_SNORM:
   case GL_INTENSITY8_SNORM:
   case GL_INTENSITY16_SNORM:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}

/**
 * Test if the given format is an integer (non-normalized) format.
 */
GLboolean
_mesa_is_enum_format_unsigned_int(GLenum format)
{
   switch (format) {
   /* specific integer formats */
   case GL_RGBA32UI_EXT:
   case GL_RGB32UI_EXT:
   case GL_RG32UI:
   case GL_R32UI:
   case GL_ALPHA32UI_EXT:
   case GL_INTENSITY32UI_EXT:
   case GL_LUMINANCE32UI_EXT:
   case GL_LUMINANCE_ALPHA32UI_EXT:
   case GL_RGBA16UI_EXT:
   case GL_RGB16UI_EXT:
   case GL_RG16UI:
   case GL_R16UI:
   case GL_ALPHA16UI_EXT:
   case GL_INTENSITY16UI_EXT:
   case GL_LUMINANCE16UI_EXT:
   case GL_LUMINANCE_ALPHA16UI_EXT:
   case GL_RGBA8UI_EXT:
   case GL_RGB8UI_EXT:
   case GL_RG8UI:
   case GL_R8UI:
   case GL_ALPHA8UI_EXT:
   case GL_INTENSITY8UI_EXT:
   case GL_LUMINANCE8UI_EXT:
   case GL_LUMINANCE_ALPHA8UI_EXT:
   case GL_RGB10_A2UI:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}


/**
 * Test if the given format is an integer (non-normalized) format.
 */
GLboolean
_mesa_is_enum_format_signed_int(GLenum format)
{
   switch (format) {
   /* generic integer formats */
   case GL_RED_INTEGER_EXT:
   case GL_GREEN_INTEGER_EXT:
   case GL_BLUE_INTEGER_EXT:
   case GL_ALPHA_INTEGER_EXT:
   case GL_RGB_INTEGER_EXT:
   case GL_RGBA_INTEGER_EXT:
   case GL_BGR_INTEGER_EXT:
   case GL_BGRA_INTEGER_EXT:
   case GL_LUMINANCE_INTEGER_EXT:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
   case GL_RG_INTEGER:
   /* specific integer formats */
   case GL_RGBA32I_EXT:
   case GL_RGB32I_EXT:
   case GL_RG32I:
   case GL_R32I:
   case GL_ALPHA32I_EXT:
   case GL_INTENSITY32I_EXT:
   case GL_LUMINANCE32I_EXT:
   case GL_LUMINANCE_ALPHA32I_EXT:
   case GL_RGBA16I_EXT:
   case GL_RGB16I_EXT:
   case GL_RG16I:
   case GL_R16I:
   case GL_ALPHA16I_EXT:
   case GL_INTENSITY16I_EXT:
   case GL_LUMINANCE16I_EXT:
   case GL_LUMINANCE_ALPHA16I_EXT:
   case GL_RGBA8I_EXT:
   case GL_RGB8I_EXT:
   case GL_RG8I:
   case GL_R8I:
   case GL_ALPHA8I_EXT:
   case GL_INTENSITY8I_EXT:
   case GL_LUMINANCE8I_EXT:
   case GL_LUMINANCE_ALPHA8I_EXT:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}


/**
 * Test if the given format is an integer (non-normalized) format.
 */
GLboolean
_mesa_is_enum_format_integer(GLenum format)
{
   return _mesa_is_enum_format_unsigned_int(format) ||
          _mesa_is_enum_format_signed_int(format);
}


GLboolean
_mesa_is_type_unsigned(GLenum type)
{
   switch (type) {
   case GL_UNSIGNED_INT:
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:

   case GL_UNSIGNED_SHORT:
   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
   case GL_UNSIGNED_SHORT_8_8_MESA:
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:

   case GL_UNSIGNED_BYTE:
   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
      return GL_TRUE;

   default:
      return GL_FALSE;
   }
}


/**
 * Test if the given image format is a color/RGBA format (i.e., not color
 * index, depth, stencil, etc).
 * \param format  the image format value (may by an internal texture format)
 * \return GL_TRUE if its a color/RGBA format, GL_FALSE otherwise.
 */
GLboolean
_mesa_is_color_format(GLenum format)
{
   switch (format) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA8:
      case GL_ALPHA12:
      case GL_ALPHA16:
      case 1:
      case GL_LUMINANCE:
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
      case 2:
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
      case GL_INTENSITY:
      case GL_INTENSITY4:
      case GL_INTENSITY8:
      case GL_INTENSITY12:
      case GL_INTENSITY16:
      case GL_R8:
      case GL_R16:
      case GL_RG:
      case GL_RG8:
      case GL_RG16:
      case 3:
      case GL_RGB:
      case GL_BGR:
      case GL_R3_G3_B2:
      case GL_RGB4:
      case GL_RGB5:
      case GL_RGB565:
      case GL_RGB8:
      case GL_RGB10:
      case GL_RGB12:
      case GL_RGB16:
      case 4:
      case GL_ABGR_EXT:
      case GL_RGBA:
      case GL_BGRA:
      case GL_RGBA2:
      case GL_RGBA4:
      case GL_RGB5_A1:
      case GL_RGBA8:
      case GL_RGB10_A2:
      case GL_RGBA12:
      case GL_RGBA16:
      /* float texture formats */
      case GL_ALPHA16F_ARB:
      case GL_ALPHA32F_ARB:
      case GL_LUMINANCE16F_ARB:
      case GL_LUMINANCE32F_ARB:
      case GL_LUMINANCE_ALPHA16F_ARB:
      case GL_LUMINANCE_ALPHA32F_ARB:
      case GL_INTENSITY16F_ARB:
      case GL_INTENSITY32F_ARB:
      case GL_R16F:
      case GL_R32F:
      case GL_RG16F:
      case GL_RG32F:
      case GL_RGB16F_ARB:
      case GL_RGB32F_ARB:
      case GL_RGBA16F_ARB:
      case GL_RGBA32F_ARB:
      /* compressed formats */
      case GL_COMPRESSED_ALPHA:
      case GL_COMPRESSED_LUMINANCE:
      case GL_COMPRESSED_LUMINANCE_ALPHA:
      case GL_COMPRESSED_INTENSITY:
      case GL_COMPRESSED_RED:
      case GL_COMPRESSED_RG:
      case GL_COMPRESSED_RGB:
      case GL_COMPRESSED_RGBA:
      case GL_RGB_S3TC:
      case GL_RGB4_S3TC:
      case GL_RGBA_S3TC:
      case GL_RGBA4_S3TC:
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      case GL_COMPRESSED_RGB_FXT1_3DFX:
      case GL_COMPRESSED_RGBA_FXT1_3DFX:
      case GL_SRGB_EXT:
      case GL_SRGB8_EXT:
      case GL_SRGB_ALPHA_EXT:
      case GL_SRGB8_ALPHA8_EXT:
      case GL_SLUMINANCE_ALPHA_EXT:
      case GL_SLUMINANCE8_ALPHA8_EXT:
      case GL_SLUMINANCE_EXT:
      case GL_SLUMINANCE8_EXT:
      case GL_COMPRESSED_SRGB_EXT:
      case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
      case GL_COMPRESSED_SLUMINANCE_EXT:
      case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
      case GL_COMPRESSED_RED_RGTC1:
      case GL_COMPRESSED_SIGNED_RED_RGTC1:
      case GL_COMPRESSED_RG_RGTC2:
      case GL_COMPRESSED_SIGNED_RG_RGTC2:
      case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
      case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
      case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
      case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
      case GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI:
      case GL_ETC1_RGB8_OES:
      case GL_COMPRESSED_RGB8_ETC2:
      case GL_COMPRESSED_SRGB8_ETC2:
      case GL_COMPRESSED_RGBA8_ETC2_EAC:
      case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
      case GL_COMPRESSED_R11_EAC:
      case GL_COMPRESSED_RG11_EAC:
      case GL_COMPRESSED_SIGNED_R11_EAC:
      case GL_COMPRESSED_SIGNED_RG11_EAC:
      case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      case GL_COMPRESSED_RGBA_BPTC_UNORM:
      case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
      case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
      case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
      /* generic integer formats */
      case GL_RED_INTEGER_EXT:
      case GL_GREEN_INTEGER_EXT:
      case GL_BLUE_INTEGER_EXT:
      case GL_ALPHA_INTEGER_EXT:
      case GL_RGB_INTEGER_EXT:
      case GL_RGBA_INTEGER_EXT:
      case GL_BGR_INTEGER_EXT:
      case GL_BGRA_INTEGER_EXT:
      case GL_RG_INTEGER:
      case GL_LUMINANCE_INTEGER_EXT:
      case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      /* sized integer formats */
      case GL_RGBA32UI_EXT:
      case GL_RGB32UI_EXT:
      case GL_RG32UI:
      case GL_R32UI:
      case GL_ALPHA32UI_EXT:
      case GL_INTENSITY32UI_EXT:
      case GL_LUMINANCE32UI_EXT:
      case GL_LUMINANCE_ALPHA32UI_EXT:
      case GL_RGBA16UI_EXT:
      case GL_RGB16UI_EXT:
      case GL_RG16UI:
      case GL_R16UI:
      case GL_ALPHA16UI_EXT:
      case GL_INTENSITY16UI_EXT:
      case GL_LUMINANCE16UI_EXT:
      case GL_LUMINANCE_ALPHA16UI_EXT:
      case GL_RGBA8UI_EXT:
      case GL_RGB8UI_EXT:
      case GL_RG8UI:
      case GL_R8UI:
      case GL_ALPHA8UI_EXT:
      case GL_INTENSITY8UI_EXT:
      case GL_LUMINANCE8UI_EXT:
      case GL_LUMINANCE_ALPHA8UI_EXT:
      case GL_RGBA32I_EXT:
      case GL_RGB32I_EXT:
      case GL_RG32I:
      case GL_R32I:
      case GL_ALPHA32I_EXT:
      case GL_INTENSITY32I_EXT:
      case GL_LUMINANCE32I_EXT:
      case GL_LUMINANCE_ALPHA32I_EXT:
      case GL_RGBA16I_EXT:
      case GL_RGB16I_EXT:
      case GL_RG16I:
      case GL_R16I:
      case GL_ALPHA16I_EXT:
      case GL_INTENSITY16I_EXT:
      case GL_LUMINANCE16I_EXT:
      case GL_LUMINANCE_ALPHA16I_EXT:
      case GL_RGBA8I_EXT:
      case GL_RGB8I_EXT:
      case GL_RG8I:
      case GL_R8I:
      case GL_ALPHA8I_EXT:
      case GL_INTENSITY8I_EXT:
      case GL_LUMINANCE8I_EXT:
      case GL_LUMINANCE_ALPHA8I_EXT:
      /* signed, normalized texture formats */
      case GL_RED_SNORM:
      case GL_R8_SNORM:
      case GL_R16_SNORM:
      case GL_RG_SNORM:
      case GL_RG8_SNORM:
      case GL_RG16_SNORM:
      case GL_RGB_SNORM:
      case GL_RGB8_SNORM:
      case GL_RGB16_SNORM:
      case GL_RGBA_SNORM:
      case GL_RGBA8_SNORM:
      case GL_RGBA16_SNORM:
      case GL_ALPHA_SNORM:
      case GL_ALPHA8_SNORM:
      case GL_ALPHA16_SNORM:
      case GL_LUMINANCE_SNORM:
      case GL_LUMINANCE8_SNORM:
      case GL_LUMINANCE16_SNORM:
      case GL_LUMINANCE_ALPHA_SNORM:
      case GL_LUMINANCE8_ALPHA8_SNORM:
      case GL_LUMINANCE16_ALPHA16_SNORM:
      case GL_INTENSITY_SNORM:
      case GL_INTENSITY8_SNORM:
      case GL_INTENSITY16_SNORM:
      case GL_RGB9_E5:
      case GL_R11F_G11F_B10F:
      case GL_RGB10_A2UI:
         return GL_TRUE;
      case GL_YCBCR_MESA:  /* not considered to be RGB */
         /* fall-through */
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a depth component format.
 */
GLboolean
_mesa_is_depth_format(GLenum format)
{
   switch (format) {
      case GL_DEPTH_COMPONENT:
      case GL_DEPTH_COMPONENT16:
      case GL_DEPTH_COMPONENT24:
      case GL_DEPTH_COMPONENT32:
      case GL_DEPTH_COMPONENT32F:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a stencil format.
 */
GLboolean
_mesa_is_stencil_format(GLenum format)
{
   switch (format) {
      case GL_STENCIL_INDEX:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a YCbCr format.
 */
GLboolean
_mesa_is_ycbcr_format(GLenum format)
{
   switch (format) {
      case GL_YCBCR_MESA:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a depth+stencil format.
 */
GLboolean
_mesa_is_depthstencil_format(GLenum format)
{
   switch (format) {
      case GL_DEPTH24_STENCIL8_EXT:
      case GL_DEPTH_STENCIL_EXT:
      case GL_DEPTH32F_STENCIL8:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a depth or stencil format.
 */
GLboolean
_mesa_is_depth_or_stencil_format(GLenum format)
{
   switch (format) {
      case GL_DEPTH_COMPONENT:
      case GL_DEPTH_COMPONENT16:
      case GL_DEPTH_COMPONENT24:
      case GL_DEPTH_COMPONENT32:
      case GL_STENCIL_INDEX:
      case GL_STENCIL_INDEX1_EXT:
      case GL_STENCIL_INDEX4_EXT:
      case GL_STENCIL_INDEX8_EXT:
      case GL_STENCIL_INDEX16_EXT:
      case GL_DEPTH_STENCIL_EXT:
      case GL_DEPTH24_STENCIL8_EXT:
      case GL_DEPTH_COMPONENT32F:
      case GL_DEPTH32F_STENCIL8:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if an image format is a supported compressed format.
 * \param format the internal format token provided by the user.
 * \return GL_TRUE if compressed, GL_FALSE if uncompressed
 */
GLboolean
_mesa_is_compressed_format(struct gl_context *ctx, GLenum format)
{
   switch (format) {
   case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
   case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
   case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
   case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      /* Assume that the ANGLE flag will always be set if the EXT flag is set.
       */
      return ctx->Extensions.ANGLE_texture_compression_dxt;
   case GL_RGB_S3TC:
   case GL_RGB4_S3TC:
   case GL_RGBA_S3TC:
   case GL_RGBA4_S3TC:
      return _mesa_is_desktop_gl(ctx) &&
         ctx->Extensions.ANGLE_texture_compression_dxt;
   case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
      return _mesa_is_desktop_gl(ctx)
         && ctx->Extensions.EXT_texture_sRGB
         && ctx->Extensions.EXT_texture_compression_s3tc;
   case GL_COMPRESSED_RGB_FXT1_3DFX:
   case GL_COMPRESSED_RGBA_FXT1_3DFX:
      return _mesa_is_desktop_gl(ctx)
         && ctx->Extensions.TDFX_texture_compression_FXT1;
   case GL_COMPRESSED_RED_RGTC1:
   case GL_COMPRESSED_SIGNED_RED_RGTC1:
   case GL_COMPRESSED_RG_RGTC2:
   case GL_COMPRESSED_SIGNED_RG_RGTC2:
      return _mesa_is_desktop_gl(ctx)
         && ctx->Extensions.ARB_texture_compression_rgtc;
   case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
   case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
   case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
   case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
      return ctx->API == API_OPENGL_COMPAT
         && ctx->Extensions.EXT_texture_compression_latc;
   case GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI:
      return ctx->API == API_OPENGL_COMPAT
         && ctx->Extensions.ATI_texture_compression_3dc;
   case GL_ETC1_RGB8_OES:
      return _mesa_is_gles(ctx)
         && ctx->Extensions.OES_compressed_ETC1_RGB8_texture;
   case GL_COMPRESSED_RGB8_ETC2:
   case GL_COMPRESSED_SRGB8_ETC2:
   case GL_COMPRESSED_RGBA8_ETC2_EAC:
   case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
   case GL_COMPRESSED_R11_EAC:
   case GL_COMPRESSED_RG11_EAC:
   case GL_COMPRESSED_SIGNED_R11_EAC:
   case GL_COMPRESSED_SIGNED_RG11_EAC:
   case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      return _mesa_is_gles3(ctx) || ctx->Extensions.ARB_ES3_compatibility;
   case GL_COMPRESSED_RGBA_BPTC_UNORM:
   case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
   case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
   case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
      return _mesa_is_desktop_gl(ctx) &&
         ctx->Extensions.ARB_texture_compression_bptc;
   case GL_PALETTE4_RGB8_OES:
   case GL_PALETTE4_RGBA8_OES:
   case GL_PALETTE4_R5_G6_B5_OES:
   case GL_PALETTE4_RGBA4_OES:
   case GL_PALETTE4_RGB5_A1_OES:
   case GL_PALETTE8_RGB8_OES:
   case GL_PALETTE8_RGBA8_OES:
   case GL_PALETTE8_R5_G6_B5_OES:
   case GL_PALETTE8_RGBA4_OES:
   case GL_PALETTE8_RGB5_A1_OES:
      return ctx->API == API_OPENGLES;
   default:
      return GL_FALSE;
   }
}


/**
 * Convert various base formats to the cooresponding integer format.
 */
GLenum
_mesa_base_format_to_integer_format(GLenum format)
{
   switch(format) {
   case GL_RED:
      return GL_RED_INTEGER;
   case GL_GREEN:
      return GL_GREEN_INTEGER;
   case GL_BLUE:
      return GL_BLUE_INTEGER;
   case GL_RG:
      return GL_RG_INTEGER;
   case GL_RGB:
      return GL_RGB_INTEGER;
   case GL_RGBA:
      return GL_RGBA_INTEGER;
   case GL_BGR:
      return GL_BGR_INTEGER;
   case GL_BGRA:
      return GL_BGRA_INTEGER;
   case GL_ALPHA:
      return GL_ALPHA_INTEGER;
   case GL_LUMINANCE:
      return GL_LUMINANCE_INTEGER_EXT;
   case GL_LUMINANCE_ALPHA:
      return GL_LUMINANCE_ALPHA_INTEGER_EXT;
   }

   return format;
}


/**
 * Does the given base texture/renderbuffer format have the channel
 * named by 'pname'?
 */
GLboolean
_mesa_base_format_has_channel(GLenum base_format, GLenum pname)
{
   switch (pname) {
   case GL_TEXTURE_RED_SIZE:
   case GL_TEXTURE_RED_TYPE:
   case GL_RENDERBUFFER_RED_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
      if (base_format == GL_RED ||
	  base_format == GL_RG ||
	  base_format == GL_RGB ||
	  base_format == GL_RGBA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_GREEN_SIZE:
   case GL_TEXTURE_GREEN_TYPE:
   case GL_RENDERBUFFER_GREEN_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
      if (base_format == GL_RG ||
	  base_format == GL_RGB ||
	  base_format == GL_RGBA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_BLUE_SIZE:
   case GL_TEXTURE_BLUE_TYPE:
   case GL_RENDERBUFFER_BLUE_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
      if (base_format == GL_RGB ||
	  base_format == GL_RGBA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_ALPHA_SIZE:
   case GL_TEXTURE_ALPHA_TYPE:
   case GL_RENDERBUFFER_ALPHA_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
      if (base_format == GL_RGBA ||
	  base_format == GL_ALPHA ||
	  base_format == GL_LUMINANCE_ALPHA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_LUMINANCE_SIZE:
   case GL_TEXTURE_LUMINANCE_TYPE:
      if (base_format == GL_LUMINANCE ||
	  base_format == GL_LUMINANCE_ALPHA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_INTENSITY_SIZE:
   case GL_TEXTURE_INTENSITY_TYPE:
      if (base_format == GL_INTENSITY) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_DEPTH_SIZE:
   case GL_TEXTURE_DEPTH_TYPE:
   case GL_RENDERBUFFER_DEPTH_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
      if (base_format == GL_DEPTH_STENCIL ||
	  base_format == GL_DEPTH_COMPONENT) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_RENDERBUFFER_STENCIL_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
      if (base_format == GL_DEPTH_STENCIL ||
	  base_format == GL_STENCIL_INDEX) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   default:
      _mesa_warning(NULL, "%s: Unexpected channel token 0x%x\n",
		    __FUNCTION__, pname);
      return GL_FALSE;
   }

   return GL_FALSE;
}


/**
 * Returns the number of channels/components for a base format.
 */
GLint
_mesa_base_format_component_count(GLenum base_format)
{
   switch (base_format) {
   case GL_RED:
   case GL_ALPHA:
   case GL_INTENSITY:
   case GL_DEPTH_COMPONENT:
      return 1;
   case GL_RG:
   case GL_LUMINANCE_ALPHA:
   case GL_DEPTH_STENCIL:
      return 2;
   case GL_RGB:
      return 3;
   case GL_RGBA:
      return 4;
   default:
      return -1;
   }
}


/**
 * If format is a generic compressed format, return the corresponding
 * non-compressed format.  For other formats, return the format as-is.
 */
GLenum
_mesa_generic_compressed_format_to_uncompressed_format(GLenum format)
{
   switch (format) {
   case GL_COMPRESSED_RED:
      return GL_RED;
   case GL_COMPRESSED_RG:
      return GL_RG;
   case GL_COMPRESSED_RGB:
      return GL_RGB;
   case GL_COMPRESSED_RGBA:
      return GL_RGBA;
   case GL_COMPRESSED_ALPHA:
      return GL_ALPHA;
   case GL_COMPRESSED_LUMINANCE:
      return GL_LUMINANCE;
   case GL_COMPRESSED_LUMINANCE_ALPHA:
      return GL_LUMINANCE_ALPHA;
   case GL_COMPRESSED_INTENSITY:
      return GL_INTENSITY;
   /* sRGB formats */
   case GL_COMPRESSED_SRGB:
      return GL_SRGB;
   case GL_COMPRESSED_SRGB_ALPHA:
      return GL_SRGB_ALPHA;
   case GL_COMPRESSED_SLUMINANCE:
      return GL_SLUMINANCE;
   case GL_COMPRESSED_SLUMINANCE_ALPHA:
      return GL_SLUMINANCE_ALPHA;
   default:
      return format;
   }
}


/**
 * Return the equivalent non-generic internal format.
 * This is useful for comparing whether two internal formats are equivalent.
 */
GLenum
_mesa_get_nongeneric_internalformat(GLenum format)
{
   switch (format) {
   /* GL 1.1 formats. */
   case 4:
   case GL_RGBA:
      return GL_RGBA8;
   case 3:
   case GL_RGB:
      return GL_RGB8;
   case 2:
   case GL_LUMINANCE_ALPHA:
      return GL_LUMINANCE8_ALPHA8;
   case 1:
   case GL_LUMINANCE:
      return GL_LUMINANCE8;
   case GL_ALPHA:
      return GL_ALPHA8;
   case GL_INTENSITY:
      return GL_INTENSITY8;

   /* GL_ARB_texture_rg */
   case GL_RED:
      return GL_R8;
   case GL_RG:
      return GL_RG8;

   /* GL_EXT_texture_sRGB */
   case GL_SRGB:
      return GL_SRGB8;
   case GL_SRGB_ALPHA:
      return GL_SRGB8_ALPHA8;
   case GL_SLUMINANCE:
      return GL_SLUMINANCE8;
   case GL_SLUMINANCE_ALPHA:
      return GL_SLUMINANCE8_ALPHA8;

   /* GL_EXT_texture_snorm */
   case GL_RGBA_SNORM:
      return GL_RGBA8_SNORM;
   case GL_RGB_SNORM:
      return GL_RGB8_SNORM;
   case GL_RG_SNORM:
      return GL_RG8_SNORM;
   case GL_RED_SNORM:
      return GL_R8_SNORM;
   case GL_LUMINANCE_ALPHA_SNORM:
      return GL_LUMINANCE8_ALPHA8_SNORM;
   case GL_LUMINANCE_SNORM:
      return GL_LUMINANCE8_SNORM;
   case GL_ALPHA_SNORM:
      return GL_ALPHA8_SNORM;
   case GL_INTENSITY_SNORM:
      return GL_INTENSITY8_SNORM;

   default:
      return format;
   }
}


/**
 * Convert an sRGB internal format to linear.
 */
GLenum
_mesa_get_linear_internalformat(GLenum format)
{
   switch (format) {
   case GL_SRGB:
      return GL_RGB;
   case GL_SRGB_ALPHA:
      return GL_RGBA;
   case GL_SRGB8:
      return GL_RGB8;
   case GL_SRGB8_ALPHA8:
      return GL_RGBA8;
   case GL_SLUMINANCE8:
      return GL_LUMINANCE8;
   case GL_SLUMINANCE:
      return GL_LUMINANCE;
   case GL_SLUMINANCE_ALPHA:
      return GL_LUMINANCE_ALPHA;
   case GL_SLUMINANCE8_ALPHA8:
      return GL_LUMINANCE8_ALPHA8;
   default:
      return format;
   }
}


/**
 * Do error checking of format/type combinations for glReadPixels,
 * glDrawPixels and glTex[Sub]Image.  Note that depending on the format
 * and type values, we may either generate GL_INVALID_OPERATION or
 * GL_INVALID_ENUM.
 *
 * \param format pixel format.
 * \param type pixel type.
 *
 * \return GL_INVALID_ENUM, GL_INVALID_OPERATION or GL_NO_ERROR
 */
GLenum
_mesa_error_check_format_and_type(const struct gl_context *ctx,
                                  GLenum format, GLenum type)
{
   /* From OpenGL 3.3 spec, page 220:
    *    "If the format is DEPTH_STENCIL, then values are taken from
    *    both the depth buffer and the stencil buffer. If there is no
    *    depth buffer or if there is no stencil buffer, then the error
    *    INVALID_OPERATION occurs. If the type parameter is not
    *    UNSIGNED_INT_24_8 or FLOAT_32_UNSIGNED_INT_24_8_REV, then the
    *    error INVALID_ENUM occurs."
    *
    * OpenGL ES still generates GL_INVALID_OPERATION because glReadPixels
    * cannot be used to read depth or stencil in that API.
    */
   if (_mesa_is_desktop_gl(ctx) && format == GL_DEPTH_STENCIL
       && type != GL_UNSIGNED_INT_24_8
       && type != GL_FLOAT_32_UNSIGNED_INT_24_8_REV)
      return GL_INVALID_ENUM;

   /* special type-based checks (see glReadPixels, glDrawPixels error lists) */
   switch (type) {
   case GL_BITMAP:
      if (format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX) {
         return GL_INVALID_ENUM;
      }
      break;

   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
      if (format == GL_RGB) {
         break; /* OK */
      }
      if (format == GL_RGB_INTEGER_EXT &&
          ctx->Extensions.ARB_texture_rgb10_a2ui) {
         break; /* OK */
      }
      return GL_INVALID_OPERATION;

   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      if (format == GL_RGBA ||
          format == GL_BGRA ||
          format == GL_ABGR_EXT) {
         break; /* OK */
      }
      if ((format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT) &&
          ctx->Extensions.ARB_texture_rgb10_a2ui) {
         break; /* OK */
      }
      if (type == GL_UNSIGNED_INT_2_10_10_10_REV && format == GL_RGB &&
          ctx->API == API_OPENGLES2) {
         break; /* OK by GL_EXT_texture_type_2_10_10_10_REV */
      }
      return GL_INVALID_OPERATION;

   case GL_UNSIGNED_INT_24_8:
      if (format != GL_DEPTH_STENCIL) {
         return GL_INVALID_OPERATION;
      }
      return GL_NO_ERROR;

   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      if (!ctx->Extensions.ARB_depth_buffer_float) {
         return GL_INVALID_ENUM;
      }
      if (format != GL_DEPTH_STENCIL) {
         return GL_INVALID_OPERATION;
      }
      return GL_NO_ERROR;

   case GL_UNSIGNED_INT_10F_11F_11F_REV:
      if (!ctx->Extensions.EXT_packed_float) {
         return GL_INVALID_ENUM;
      }
      if (format != GL_RGB) {
         return GL_INVALID_OPERATION;
      }
      return GL_NO_ERROR;

   default:
      ; /* fall-through */
   }

   /* now, for each format, check the type for compatibility */
   switch (format) {
      case GL_COLOR_INDEX:
      case GL_STENCIL_INDEX:
         switch (type) {
            case GL_BITMAP:
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
#if 0 /* not legal!  see table 3.6 of the 1.5 spec */
      case GL_INTENSITY:
#endif
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_DEPTH_COMPONENT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RG:
	 if (!ctx->Extensions.ARB_texture_rg)
	    return GL_INVALID_ENUM;
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RGB:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_UNSIGNED_BYTE_3_3_2:
            case GL_UNSIGNED_BYTE_2_3_3_REV:
            case GL_UNSIGNED_SHORT_5_6_5:
            case GL_UNSIGNED_SHORT_5_6_5_REV:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            case GL_UNSIGNED_INT_2_10_10_10_REV:
               /* OK by GL_EXT_texture_type_2_10_10_10_REV */
               return (ctx->API == API_OPENGLES2)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            case GL_UNSIGNED_INT_5_9_9_9_REV:
               return ctx->Extensions.EXT_texture_shared_exponent
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            case GL_UNSIGNED_INT_10F_11F_11F_REV:
               return ctx->Extensions.EXT_packed_float
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_BGR:
         switch (type) {
            /* NOTE: no packed types are supported with BGR.  That's
             * intentional, according to the GL spec.
             */
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RGBA:
      case GL_BGRA:
      case GL_ABGR_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_UNSIGNED_SHORT_4_4_4_4:
            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            case GL_UNSIGNED_SHORT_5_5_5_1:
            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            case GL_UNSIGNED_INT_8_8_8_8:
            case GL_UNSIGNED_INT_8_8_8_8_REV:
            case GL_UNSIGNED_INT_10_10_10_2:
            case GL_UNSIGNED_INT_2_10_10_10_REV:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_YCBCR_MESA:
         if (!ctx->Extensions.MESA_ycbcr_texture)
            return GL_INVALID_ENUM;
         if (type == GL_UNSIGNED_SHORT_8_8_MESA ||
             type == GL_UNSIGNED_SHORT_8_8_REV_MESA)
            return GL_NO_ERROR;
         else
            return GL_INVALID_OPERATION;

      case GL_DEPTH_STENCIL:
         if (type == GL_UNSIGNED_INT_24_8)
            return GL_NO_ERROR;
         else if (ctx->Extensions.ARB_depth_buffer_float &&
             type == GL_FLOAT_32_UNSIGNED_INT_24_8_REV)
            return GL_NO_ERROR;
         else
            return GL_INVALID_ENUM;

      /* integer-valued formats */
      case GL_RED_INTEGER_EXT:
      case GL_GREEN_INTEGER_EXT:
      case GL_BLUE_INTEGER_EXT:
      case GL_ALPHA_INTEGER_EXT:
      case GL_RG_INTEGER:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
               return (ctx->Version >= 30 ||
                       ctx->Extensions.EXT_texture_integer)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RGB_INTEGER_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
               return (ctx->Version >= 30 ||
                       ctx->Extensions.EXT_texture_integer)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            case GL_UNSIGNED_BYTE_3_3_2:
            case GL_UNSIGNED_BYTE_2_3_3_REV:
            case GL_UNSIGNED_SHORT_5_6_5:
            case GL_UNSIGNED_SHORT_5_6_5_REV:
               return ctx->Extensions.ARB_texture_rgb10_a2ui
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_BGR_INTEGER_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            /* NOTE: no packed formats w/ BGR format */
               return (ctx->Version >= 30 ||
                       ctx->Extensions.EXT_texture_integer)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RGBA_INTEGER_EXT:
      case GL_BGRA_INTEGER_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
               return (ctx->Version >= 30 ||
                       ctx->Extensions.EXT_texture_integer)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            case GL_UNSIGNED_SHORT_4_4_4_4:
            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            case GL_UNSIGNED_SHORT_5_5_5_1:
            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            case GL_UNSIGNED_INT_8_8_8_8:
            case GL_UNSIGNED_INT_8_8_8_8_REV:
            case GL_UNSIGNED_INT_10_10_10_2:
            case GL_UNSIGNED_INT_2_10_10_10_REV:
               return ctx->Extensions.ARB_texture_rgb10_a2ui
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_LUMINANCE_INTEGER_EXT:
      case GL_LUMINANCE_ALPHA_INTEGER_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
               return ctx->Extensions.EXT_texture_integer
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      default:
         return GL_INVALID_ENUM;
   }
   return GL_NO_ERROR;
}


/**
 * Do error checking of format/type combinations for OpenGL ES glReadPixels
 * and glTex[Sub]Image.
 * \return error code, or GL_NO_ERROR.
 */
GLenum
_mesa_es_error_check_format_and_type(GLenum format, GLenum type,
                                     unsigned dimensions)
{
   GLboolean type_valid = GL_TRUE;

   switch (format) {
   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:
      type_valid = (type == GL_UNSIGNED_BYTE
                    || type == GL_FLOAT
                    || type == GL_HALF_FLOAT_OES);
      break;

   case GL_RGB:
      type_valid = (type == GL_UNSIGNED_BYTE
                    || type == GL_UNSIGNED_SHORT_5_6_5
                    || type == GL_FLOAT
                    || type == GL_HALF_FLOAT_OES);
      break;

   case GL_RGBA:
      type_valid = (type == GL_UNSIGNED_BYTE
                    || type == GL_UNSIGNED_SHORT_4_4_4_4
                    || type == GL_UNSIGNED_SHORT_5_5_5_1
                    || type == GL_FLOAT
                    || type == GL_HALF_FLOAT_OES
                    || type == GL_UNSIGNED_INT_2_10_10_10_REV);
      break;

   case GL_DEPTH_COMPONENT:
      /* This format is filtered against invalid dimensionalities elsewhere.
       */
      type_valid = (type == GL_UNSIGNED_SHORT
                    || type == GL_UNSIGNED_INT);
      break;

   case GL_DEPTH_STENCIL:
      /* This format is filtered against invalid dimensionalities elsewhere.
       */
      type_valid = (type == GL_UNSIGNED_INT_24_8);
      break;

   case GL_BGRA_EXT:
      type_valid = (type == GL_UNSIGNED_BYTE);

      /* This feels like a bug in the EXT_texture_format_BGRA8888 spec, but
       * the format does not appear to be allowed for 3D textures in OpenGL
       * ES.
       */
      if (dimensions != 2)
         return GL_INVALID_VALUE;

      break;

   default:
      return GL_INVALID_VALUE;
   }

   return type_valid ? GL_NO_ERROR : GL_INVALID_OPERATION;
}


/**
 * Do error checking of format/type combinations for OpenGL ES 3
 * glTex[Sub]Image.
 * \return error code, or GL_NO_ERROR.
 */
GLenum
_mesa_es3_error_check_format_and_type(GLenum format, GLenum type,
                                      GLenum internalFormat)
{
   switch (format) {
   case GL_RGBA:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         switch (internalFormat) {
         case GL_RGBA:
         case GL_RGBA8:
         case GL_RGB5_A1:
         case GL_RGBA4:
         case GL_SRGB8_ALPHA8_EXT:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_BYTE:
         if (internalFormat != GL_RGBA8_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT_4_4_4_4:
         switch (internalFormat) {
         case GL_RGBA:
         case GL_RGBA4:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_UNSIGNED_SHORT_5_5_5_1:
         switch (internalFormat) {
         case GL_RGBA:
         case GL_RGB5_A1:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_UNSIGNED_INT_2_10_10_10_REV:
         switch (internalFormat) {
         case GL_RGBA: /* GL_EXT_texture_type_2_10_10_10_REV */
         case GL_RGB10_A2:
         case GL_RGB5_A1:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_HALF_FLOAT:
         if (internalFormat != GL_RGBA16F)
            return GL_INVALID_OPERATION;
         break;

      case GL_FLOAT:
         switch (internalFormat) {
         case GL_RGBA16F:
         case GL_RGBA32F:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RGBA_INTEGER:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_RGBA8UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_RGBA8I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RGBA16UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RGBA16I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         if (internalFormat != GL_RGBA32UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_INT:
         if (internalFormat != GL_RGBA32I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT_2_10_10_10_REV:
         if (internalFormat != GL_RGB10_A2UI)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RGB:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         switch (internalFormat) {
         case GL_RGB:
         case GL_RGB8:
         case GL_RGB565:
         case GL_SRGB8:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_BYTE:
         if (internalFormat != GL_RGB8_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT_5_6_5:
         switch (internalFormat) {
         case GL_RGB:
         case GL_RGB565:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_UNSIGNED_INT_10F_11F_11F_REV:
         if (internalFormat != GL_R11F_G11F_B10F)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT_5_9_9_9_REV:
         if (internalFormat != GL_RGB9_E5)
            return GL_INVALID_OPERATION;
         break;

      case GL_HALF_FLOAT:
         switch (internalFormat) {
         case GL_RGB16F:
         case GL_R11F_G11F_B10F:
         case GL_RGB9_E5:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_FLOAT:
         switch (internalFormat) {
         case GL_RGB16F:
         case GL_RGB32F:
         case GL_R11F_G11F_B10F:
         case GL_RGB9_E5:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_UNSIGNED_INT_2_10_10_10_REV:
         switch (internalFormat) {
         case GL_RGB: /* GL_EXT_texture_type_2_10_10_10_REV */
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RGB_INTEGER:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_RGB8UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_RGB8I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RGB16UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RGB16I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         if (internalFormat != GL_RGB32UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_INT:
         if (internalFormat != GL_RGB32I)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RG:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_RG8)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_RG8_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_HALF_FLOAT:
         if (internalFormat != GL_RG16F)
            return GL_INVALID_OPERATION;
         break;

      case GL_FLOAT:
         switch (internalFormat) {
         case GL_RG16F:
         case GL_RG32F:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RG_INTEGER:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_RG8UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_RG8I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RG16UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RG16I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         if (internalFormat != GL_RG32UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_INT:
         if (internalFormat != GL_RG32I)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RED:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_R8)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_R8_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_HALF_FLOAT:
         if (internalFormat != GL_R16F)
            return GL_INVALID_OPERATION;
         break;

      case GL_FLOAT:
         switch (internalFormat) {
         case GL_R16F:
         case GL_R32F:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RED_INTEGER:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_R8UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_R8I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_R16UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_R16I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         if (internalFormat != GL_R32UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_INT:
         if (internalFormat != GL_R32I)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_DEPTH_COMPONENT:
      switch (type) {
      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_DEPTH_COMPONENT
             && internalFormat != GL_DEPTH_COMPONENT16)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         switch (internalFormat) {
         case GL_DEPTH_COMPONENT:
         case GL_DEPTH_COMPONENT16:
         case GL_DEPTH_COMPONENT24:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_FLOAT:
         if (internalFormat != GL_DEPTH_COMPONENT32F)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_DEPTH_STENCIL:
      switch (type) {
      case GL_UNSIGNED_INT_24_8:
         if (internalFormat != GL_DEPTH_STENCIL
             && internalFormat != GL_DEPTH24_STENCIL8)
            return GL_INVALID_OPERATION;
         break;

      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
         if (internalFormat != GL_DEPTH32F_STENCIL8)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:
      if (type != GL_UNSIGNED_BYTE || format != internalFormat)
         return GL_INVALID_OPERATION;
      break;
   }

   return GL_NO_ERROR;
}
