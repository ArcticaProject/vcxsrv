/*
 * Mesa 3-D graphics library
 * Version:  6.5.1
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 * Copyright (c) 2008 VMware, Inc.
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


/**
 * \file texformat.c
 * Texture formats.
 *
 * \author Gareth Hughes
 */


#include "colormac.h"
#include "context.h"
#include "texformat.h"
#include "texstore.h"


#if FEATURE_EXT_texture_sRGB

/**
 * Convert an 8-bit sRGB value from non-linear space to a
 * linear RGB value in [0, 1].
 * Implemented with a 256-entry lookup table.
 */
static INLINE GLfloat
nonlinear_to_linear(GLubyte cs8)
{
   static GLfloat table[256];
   static GLboolean tableReady = GL_FALSE;
   if (!tableReady) {
      /* compute lookup table now */
      GLuint i;
      for (i = 0; i < 256; i++) {
         const GLfloat cs = UBYTE_TO_FLOAT(i);
         if (cs <= 0.04045) {
            table[i] = cs / 12.92f;
         }
         else {
            table[i] = (GLfloat) _mesa_pow((cs + 0.055) / 1.055, 2.4);
         }
      }
      tableReady = GL_TRUE;
   }
   return table[cs8];
}


#endif /* FEATURE_EXT_texture_sRGB */


/* Texel fetch routines for all supported formats
 */
#define DIM 1
#include "texformat_tmp.h"

#define DIM 2
#include "texformat_tmp.h"

#define DIM 3
#include "texformat_tmp.h"

/**
 * Null texel fetch function.
 *
 * Have to have this so the FetchTexel function pointer is never NULL.
 */
static void fetch_null_texel( const struct gl_texture_image *texImage,
			      GLint i, GLint j, GLint k, GLchan *texel )
{
   (void) texImage; (void) i; (void) j; (void) k;
   texel[RCOMP] = 0;
   texel[GCOMP] = 0;
   texel[BCOMP] = 0;
   texel[ACOMP] = 0;
   _mesa_warning(NULL, "fetch_null_texel() called!");
}

static void fetch_null_texelf( const struct gl_texture_image *texImage,
                               GLint i, GLint j, GLint k, GLfloat *texel )
{
   (void) texImage; (void) i; (void) j; (void) k;
   texel[RCOMP] = 0.0;
   texel[GCOMP] = 0.0;
   texel[BCOMP] = 0.0;
   texel[ACOMP] = 0.0;
   _mesa_warning(NULL, "fetch_null_texelf() called!");
}

static void store_null_texel(struct gl_texture_image *texImage,
                             GLint i, GLint j, GLint k, const void *texel)
{
   (void) texImage;
   (void) i;
   (void) j;
   (void) k;
   (void) texel;
   /* no-op */
}


/**
 * Notes about the predefined gl_texture_formats:
 *
 * 1. There are 1D, 2D and 3D functions for fetching texels from texture
 *    images, returning both GLchan values and GLfloat values.  (six
 *    functions in total)
 *    You don't have to provide both the GLchan and GLfloat functions;
 *    just one or the other is OK.  Mesa will use an "adaptor" to convert
 *    between GLchan/GLfloat when needed.
 *    Since the adaptors have small performance penalty, we provide both
 *    GLchan and GLfloat functions for some common formats like RGB, RGBA.
 */


/***************************************************************/
/** \name Default GLchan-based formats */
/*@{*/

const struct gl_texture_format _mesa_texformat_rgba = {
   MESA_FORMAT_RGBA,			/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   CHAN_BITS,				/* RedBits */
   CHAN_BITS,				/* GreenBits */
   CHAN_BITS,				/* BlueBits */
   CHAN_BITS,				/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4 * sizeof(GLchan),			/* TexelBytes */
   _mesa_texstore_rgba,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgba,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgba,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgba,		/* FetchTexel3Df */
   store_texel_rgba			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgb = {
   MESA_FORMAT_RGB,			/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   CHAN_BITS,				/* RedBits */
   CHAN_BITS,				/* GreenBits */
   CHAN_BITS,				/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   3 * sizeof(GLchan),			/* TexelBytes */
   _mesa_texstore_rgba,/*yes*/		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgb,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgb,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgb,		/* FetchTexel3Df */
   store_texel_rgb			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_alpha = {
   MESA_FORMAT_ALPHA,			/* MesaFormat */
   GL_ALPHA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   CHAN_BITS,				/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   sizeof(GLchan),			/* TexelBytes */
   _mesa_texstore_rgba,/*yes*/		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_alpha,		/* FetchTexel1Df */
   fetch_texel_2d_f_alpha,		/* FetchTexel2Df */
   fetch_texel_3d_f_alpha,		/* FetchTexel3Df */
   store_texel_alpha			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_luminance = {
   MESA_FORMAT_LUMINANCE,		/* MesaFormat */
   GL_LUMINANCE,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   CHAN_BITS,				/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   sizeof(GLchan),			/* TexelBytes */
   _mesa_texstore_rgba,/*yes*/		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_luminance,		/* FetchTexel1Df */
   fetch_texel_2d_f_luminance,		/* FetchTexel2Df */
   fetch_texel_3d_f_luminance,		/* FetchTexel3Df */
   store_texel_luminance		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_luminance_alpha = {
   MESA_FORMAT_LUMINANCE_ALPHA,		/* MesaFormat */
   GL_LUMINANCE_ALPHA,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   CHAN_BITS,				/* AlphaBits */
   CHAN_BITS,				/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2 * sizeof(GLchan),			/* TexelBytes */
   _mesa_texstore_rgba,/*yes*/		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_luminance_alpha,	/* FetchTexel1Df */
   fetch_texel_2d_f_luminance_alpha,	/* FetchTexel2Df */
   fetch_texel_3d_f_luminance_alpha,	/* FetchTexel3Df */
   store_texel_luminance_alpha		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_intensity = {
   MESA_FORMAT_INTENSITY,		/* MesaFormat */
   GL_INTENSITY,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   CHAN_BITS,				/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   sizeof(GLchan),			/* TexelBytes */
   _mesa_texstore_rgba,/*yes*/		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_intensity,		/* FetchTexel1Df */
   fetch_texel_2d_f_intensity,		/* FetchTexel2Df */
   fetch_texel_3d_f_intensity,		/* FetchTexel3Df */
   store_texel_intensity		/* StoreTexel */
};


#if FEATURE_EXT_texture_sRGB

const struct gl_texture_format _mesa_texformat_srgb8 = {
   MESA_FORMAT_SRGB8,			/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   3,					/* TexelBytes */
   _mesa_texstore_srgb8,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_srgb8,		/* FetchTexel1Df */
   fetch_texel_2d_srgb8,		/* FetchTexel2Df */
   fetch_texel_3d_srgb8,		/* FetchTexel3Df */
   store_texel_srgb8			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_srgba8 = {
   MESA_FORMAT_SRGBA8,			/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_srgba8,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_srgba8,		/* FetchTexel1Df */
   fetch_texel_2d_srgba8,		/* FetchTexel2Df */
   fetch_texel_3d_srgba8,		/* FetchTexel3Df */
   store_texel_srgba8			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_sargb8 = {
   MESA_FORMAT_SARGB8,			/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_sargb8,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_sargb8,		/* FetchTexel1Df */
   fetch_texel_2d_sargb8,		/* FetchTexel2Df */
   fetch_texel_3d_sargb8,		/* FetchTexel3Df */
   store_texel_sargb8			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_sl8 = {
   MESA_FORMAT_SL8,			/* MesaFormat */
   GL_LUMINANCE,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   8,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1,					/* TexelBytes */
   _mesa_texstore_sl8,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_sl8,			/* FetchTexel1Df */
   fetch_texel_2d_sl8,			/* FetchTexel2Df */
   fetch_texel_3d_sl8,			/* FetchTexel3Df */
   store_texel_sl8			/* StoreTexel */
};

/* Note: this format name looks like a misnomer, make it sal8? */
const struct gl_texture_format _mesa_texformat_sla8 = {
   MESA_FORMAT_SLA8,			/* MesaFormat */
   GL_LUMINANCE_ALPHA,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   8,					/* AlphaBits */
   8,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_sla8,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_sla8,			/* FetchTexel1Df */
   fetch_texel_2d_sla8,			/* FetchTexel2Df */
   fetch_texel_3d_sla8,			/* FetchTexel3Df */
   store_texel_sla8			/* StoreTexel */
};

#endif /* FEATURE_EXT_texture_sRGB */

const struct gl_texture_format _mesa_texformat_rgba_float32 = {
   MESA_FORMAT_RGBA_FLOAT32,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_FLOAT,				/* DataType */
   8 * sizeof(GLfloat),			/* RedBits */
   8 * sizeof(GLfloat),			/* GreenBits */
   8 * sizeof(GLfloat),			/* BlueBits */
   8 * sizeof(GLfloat),			/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4 * sizeof(GLfloat),			/* TexelBytes */
   _mesa_texstore_rgba_float32,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   fetch_texel_1d_f_rgba_f32,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgba_f32,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgba_f32,		/* FetchTexel3Df */
   store_texel_rgba_f32			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgba_float16 = {
   MESA_FORMAT_RGBA_FLOAT16,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_FLOAT,				/* DataType */
   8 * sizeof(GLhalfARB),		/* RedBits */
   8 * sizeof(GLhalfARB),		/* GreenBits */
   8 * sizeof(GLhalfARB),		/* BlueBits */
   8 * sizeof(GLhalfARB),		/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4 * sizeof(GLhalfARB),		/* TexelBytes */
   _mesa_texstore_rgba_float16,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   fetch_texel_1d_f_rgba_f16,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgba_f16,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgba_f16,		/* FetchTexel3Df */
   store_texel_rgba_f16			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgb_float32 = {
   MESA_FORMAT_RGB_FLOAT32,		/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_FLOAT,				/* DataType */
   8 * sizeof(GLfloat),			/* RedBits */
   8 * sizeof(GLfloat),			/* GreenBits */
   8 * sizeof(GLfloat),			/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   3 * sizeof(GLfloat),			/* TexelBytes */
   _mesa_texstore_rgba_float32,/*yes*/	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   fetch_texel_1d_f_rgb_f32,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgb_f32,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgb_f32,		/* FetchTexel3Df */
   store_texel_rgb_f32			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgb_float16 = {
   MESA_FORMAT_RGB_FLOAT16,		/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_FLOAT,				/* DataType */
   8 * sizeof(GLhalfARB),		/* RedBits */
   8 * sizeof(GLhalfARB),		/* GreenBits */
   8 * sizeof(GLhalfARB),		/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   3 * sizeof(GLhalfARB),		/* TexelBytes */
   _mesa_texstore_rgba_float16,/*yes*/	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   fetch_texel_1d_f_rgb_f16,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgb_f16,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgb_f16,		/* FetchTexel3Df */
   store_texel_rgb_f16			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_alpha_float32 = {
   MESA_FORMAT_ALPHA_FLOAT32,		/* MesaFormat */
   GL_ALPHA,				/* BaseFormat */
   GL_FLOAT,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   8 * sizeof(GLfloat),			/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1 * sizeof(GLfloat),			/* TexelBytes */
   _mesa_texstore_rgba_float32,/*yes*/	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   fetch_texel_1d_f_alpha_f32,		/* FetchTexel1Df */
   fetch_texel_2d_f_alpha_f32,		/* FetchTexel2Df */
   fetch_texel_3d_f_alpha_f32,		/* FetchTexel3Df */
   store_texel_alpha_f32		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_alpha_float16 = {
   MESA_FORMAT_ALPHA_FLOAT16,		/* MesaFormat */
   GL_ALPHA,				/* BaseFormat */
   GL_FLOAT,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   8 * sizeof(GLhalfARB),		/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1 * sizeof(GLhalfARB),		/* TexelBytes */
   _mesa_texstore_rgba_float16,/*yes*/	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   fetch_texel_1d_f_alpha_f16,		/* FetchTexel1Df */
   fetch_texel_2d_f_alpha_f16,		/* FetchTexel2Df */
   fetch_texel_3d_f_alpha_f16,		/* FetchTexel3Df */
   store_texel_alpha_f16		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_luminance_float32 = {
   MESA_FORMAT_LUMINANCE_FLOAT32,	/* MesaFormat */
   GL_LUMINANCE,			/* BaseFormat */
   GL_FLOAT,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   8 * sizeof(GLfloat),			/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1 * sizeof(GLfloat),			/* TexelBytes */
   _mesa_texstore_rgba_float32,/*yes*/	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_luminance_f32,	/* FetchTexel1Df */
   fetch_texel_2d_f_luminance_f32,	/* FetchTexel2Df */
   fetch_texel_3d_f_luminance_f32,	/* FetchTexel3Df */
   store_texel_luminance_f32		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_luminance_float16 = {
   MESA_FORMAT_LUMINANCE_FLOAT16,	/* MesaFormat */
   GL_LUMINANCE,			/* BaseFormat */
   GL_FLOAT,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   8 * sizeof(GLhalfARB),		/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1 * sizeof(GLhalfARB),		/* TexelBytes */
   _mesa_texstore_rgba_float16,/*yes*/	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_luminance_f16,	/* FetchTexel1Df */
   fetch_texel_2d_f_luminance_f16,	/* FetchTexel2Df */
   fetch_texel_3d_f_luminance_f16,	/* FetchTexel3Df */
   store_texel_luminance_f16		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_luminance_alpha_float32 = {
   MESA_FORMAT_LUMINANCE_ALPHA_FLOAT32,	/* MesaFormat */
   GL_LUMINANCE_ALPHA,			/* BaseFormat */
   GL_FLOAT,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   8 * sizeof(GLfloat),			/* AlphaBits */
   8 * sizeof(GLfloat),			/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2 * sizeof(GLfloat),			/* TexelBytes */
   _mesa_texstore_rgba_float32,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_luminance_alpha_f32,/* FetchTexel1Df */
   fetch_texel_2d_f_luminance_alpha_f32,/* FetchTexel2Df */
   fetch_texel_3d_f_luminance_alpha_f32,/* FetchTexel3Df */
   store_texel_luminance_alpha_f32	/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_luminance_alpha_float16 = {
   MESA_FORMAT_LUMINANCE_ALPHA_FLOAT16,	/* MesaFormat */
   GL_LUMINANCE_ALPHA,			/* BaseFormat */
   GL_FLOAT,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   8 * sizeof(GLhalfARB),		/* AlphaBits */
   8 * sizeof(GLhalfARB),		/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2 * sizeof(GLhalfARB),		/* TexelBytes */
   _mesa_texstore_rgba_float16,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_luminance_alpha_f16,/* FetchTexel1Df */
   fetch_texel_2d_f_luminance_alpha_f16,/* FetchTexel2Df */
   fetch_texel_3d_f_luminance_alpha_f16,/* FetchTexel3Df */
   store_texel_luminance_alpha_f16	/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_intensity_float32 = {
   MESA_FORMAT_INTENSITY_FLOAT32,	/* MesaFormat */
   GL_INTENSITY,			/* BaseFormat */
   GL_FLOAT,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   8 * sizeof(GLfloat),			/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1 * sizeof(GLfloat),			/* TexelBytes */
   _mesa_texstore_rgba_float32,/*yes*/	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_intensity_f32,	/* FetchTexel1Df */
   fetch_texel_2d_f_intensity_f32,	/* FetchTexel2Df */
   fetch_texel_3d_f_intensity_f32,	/* FetchTexel3Df */
   store_texel_intensity_f32		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_intensity_float16 = {
   MESA_FORMAT_INTENSITY_FLOAT16,	/* MesaFormat */
   GL_INTENSITY,			/* BaseFormat */
   GL_FLOAT,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   8 * sizeof(GLhalfARB),		/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1 * sizeof(GLhalfARB),		/* TexelBytes */
   _mesa_texstore_rgba_float16,/*yes*/	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_intensity_f16,	/* FetchTexel1Df */
   fetch_texel_2d_f_intensity_f16,	/* FetchTexel2Df */
   fetch_texel_3d_f_intensity_f16,	/* FetchTexel3Df */
   store_texel_intensity_f16		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_dudv8 = {
   MESA_FORMAT_DUDV8,			/* MesaFormat */
   GL_DUDV_ATI,				/* BaseFormat */
   GL_SIGNED_NORMALIZED,		/* DataType */
   /* maybe should add dudvBits field, but spec seems to be
      lacking the ability to query with GetTexLevelParameter anyway */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_dudv8,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_dudv8,		/* FetchTexel1Df */
   fetch_texel_2d_dudv8,		/* FetchTexel2Df */
   fetch_texel_3d_dudv8,		/* FetchTexel3Df */
   NULL					/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_signed_rgba8888 = {
   MESA_FORMAT_SIGNED_RGBA8888,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_SIGNED_NORMALIZED,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_signed_rgba8888,	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_signed_rgba8888,	/* FetchTexel1Df */
   fetch_texel_2d_signed_rgba8888,	/* FetchTexel2Df */
   fetch_texel_3d_signed_rgba8888,	/* FetchTexel3Df */
   store_texel_signed_rgba8888		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_signed_rgba8888_rev = {
   MESA_FORMAT_SIGNED_RGBA8888_REV,	/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_SIGNED_NORMALIZED,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_signed_rgba8888,	/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_signed_rgba8888_rev,	/* FetchTexel1Df */
   fetch_texel_2d_signed_rgba8888_rev,	/* FetchTexel2Df */
   fetch_texel_3d_signed_rgba8888_rev,	/* FetchTexel3Df */
   store_texel_signed_rgba8888_rev		/* StoreTexel */
};

/*@}*/


/***************************************************************/
/** \name Hardware formats */
/*@{*/

const struct gl_texture_format _mesa_texformat_rgba8888 = {
   MESA_FORMAT_RGBA8888,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_rgba8888,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgba8888,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgba8888,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgba8888,		/* FetchTexel3Df */
   store_texel_rgba8888			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgba8888_rev = {
   MESA_FORMAT_RGBA8888_REV,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_rgba8888,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgba8888_rev,	/* FetchTexel1Df */
   fetch_texel_2d_f_rgba8888_rev,	/* FetchTexel2Df */
   fetch_texel_3d_f_rgba8888_rev,	/* FetchTexel3Df */
   store_texel_rgba8888_rev		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_argb8888 = {
   MESA_FORMAT_ARGB8888,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_argb8888,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_argb8888,		/* FetchTexel1Df */
   fetch_texel_2d_f_argb8888,		/* FetchTexel2Df */
   fetch_texel_3d_f_argb8888,		/* FetchTexel3Df */
   store_texel_argb8888			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_argb8888_rev = {
   MESA_FORMAT_ARGB8888_REV,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_argb8888,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_argb8888_rev,	/* FetchTexel1Df */
   fetch_texel_2d_f_argb8888_rev,	/* FetchTexel2Df */
   fetch_texel_3d_f_argb8888_rev,	/* FetchTexel3Df */
   store_texel_argb8888_rev		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgb888 = {
   MESA_FORMAT_RGB888,			/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   3,					/* TexelBytes */
   _mesa_texstore_rgb888,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgb888,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgb888,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgb888,		/* FetchTexel3Df */
   store_texel_rgb888			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_bgr888 = {
   MESA_FORMAT_BGR888,			/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   8,					/* RedBits */
   8,					/* GreenBits */
   8,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   3,					/* TexelBytes */
   _mesa_texstore_bgr888,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_bgr888,		/* FetchTexel1Df */
   fetch_texel_2d_f_bgr888,		/* FetchTexel2Df */
   fetch_texel_3d_f_bgr888,		/* FetchTexel3Df */
   store_texel_bgr888			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgb565 = {
   MESA_FORMAT_RGB565,			/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   5,					/* RedBits */
   6,					/* GreenBits */
   5,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_rgb565,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgb565,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgb565,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgb565,		/* FetchTexel3Df */
   store_texel_rgb565			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgb565_rev = {
   MESA_FORMAT_RGB565_REV,		/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   5,					/* RedBits */
   6,					/* GreenBits */
   5,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_rgb565,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgb565_rev,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgb565_rev,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgb565_rev,		/* FetchTexel3Df */
   store_texel_rgb565_rev		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgba4444 = {
   MESA_FORMAT_RGBA4444,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   4,					/* RedBits */
   4,					/* GreenBits */
   4,					/* BlueBits */
   4,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_rgba4444,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgba4444,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgba4444,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgba4444,		/* FetchTexel3Df */
   store_texel_rgba4444			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_argb4444 = {
   MESA_FORMAT_ARGB4444,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   4,					/* RedBits */
   4,					/* GreenBits */
   4,					/* BlueBits */
   4,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_argb4444,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_argb4444,		/* FetchTexel1Df */
   fetch_texel_2d_f_argb4444,		/* FetchTexel2Df */
   fetch_texel_3d_f_argb4444,		/* FetchTexel3Df */
   store_texel_argb4444			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_argb4444_rev = {
   MESA_FORMAT_ARGB4444_REV,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   4,					/* RedBits */
   4,					/* GreenBits */
   4,					/* BlueBits */
   4,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_argb4444,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_argb4444_rev,	/* FetchTexel1Df */
   fetch_texel_2d_f_argb4444_rev,	/* FetchTexel2Df */
   fetch_texel_3d_f_argb4444_rev,	/* FetchTexel3Df */
   store_texel_argb4444_rev		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgba5551 = {
   MESA_FORMAT_RGBA5551,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   5,					/* RedBits */
   5,					/* GreenBits */
   5,					/* BlueBits */
   1,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_rgba5551,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgba5551,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgba5551,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgba5551,		/* FetchTexel3Df */
   store_texel_rgba5551			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_argb1555 = {
   MESA_FORMAT_ARGB1555,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   5,					/* RedBits */
   5,					/* GreenBits */
   5,					/* BlueBits */
   1,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_argb1555,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_argb1555,		/* FetchTexel1Df */
   fetch_texel_2d_f_argb1555,		/* FetchTexel2Df */
   fetch_texel_3d_f_argb1555,		/* FetchTexel3Df */
   store_texel_argb1555			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_argb1555_rev = {
   MESA_FORMAT_ARGB1555_REV,		/* MesaFormat */
   GL_RGBA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   5,					/* RedBits */
   5,					/* GreenBits */
   5,					/* BlueBits */
   1,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_argb1555,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_argb1555_rev,	/* FetchTexel1Df */
   fetch_texel_2d_f_argb1555_rev,	/* FetchTexel2Df */
   fetch_texel_3d_f_argb1555_rev,	/* FetchTexel3Df */
   store_texel_argb1555_rev		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_al88 = {
   MESA_FORMAT_AL88,			/* MesaFormat */
   GL_LUMINANCE_ALPHA,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   8,					/* AlphaBits */
   8,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_al88,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_al88,		/* FetchTexel1Df */
   fetch_texel_2d_f_al88,		/* FetchTexel2Df */
   fetch_texel_3d_f_al88,		/* FetchTexel3Df */
   store_texel_al88			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_al88_rev = {
   MESA_FORMAT_AL88_REV,		/* MesaFormat */
   GL_LUMINANCE_ALPHA,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   8,					/* AlphaBits */
   8,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_al88,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_al88_rev,		/* FetchTexel1Df */
   fetch_texel_2d_f_al88_rev,		/* FetchTexel2Df */
   fetch_texel_3d_f_al88_rev,		/* FetchTexel3Df */
   store_texel_al88_rev			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_rgb332 = {
   MESA_FORMAT_RGB332,			/* MesaFormat */
   GL_RGB,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   3,					/* RedBits */
   3,					/* GreenBits */
   2,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1,					/* TexelBytes */
   _mesa_texstore_rgb332,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_rgb332,		/* FetchTexel1Df */
   fetch_texel_2d_f_rgb332,		/* FetchTexel2Df */
   fetch_texel_3d_f_rgb332,		/* FetchTexel3Df */
   store_texel_rgb332			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_a8 = {
   MESA_FORMAT_A8,			/* MesaFormat */
   GL_ALPHA,				/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   8,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1,					/* TexelBytes */
   _mesa_texstore_a8,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_a8,			/* FetchTexel1Df */
   fetch_texel_2d_f_a8,			/* FetchTexel2Df */
   fetch_texel_3d_f_a8,			/* FetchTexel3Df */
   store_texel_a8			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_l8 = {
   MESA_FORMAT_L8,			/* MesaFormat */
   GL_LUMINANCE,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   8,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1,					/* TexelBytes */
   _mesa_texstore_a8,/*yes*/		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_l8,			/* FetchTexel1Df */
   fetch_texel_2d_f_l8,			/* FetchTexel2Df */
   fetch_texel_3d_f_l8,			/* FetchTexel3Df */
   store_texel_l8			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_i8 = {
   MESA_FORMAT_I8,			/* MesaFormat */
   GL_INTENSITY,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   8,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1,					/* TexelBytes */
   _mesa_texstore_a8,/*yes*/		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_i8,			/* FetchTexel1Df */
   fetch_texel_2d_f_i8,			/* FetchTexel2Df */
   fetch_texel_3d_f_i8,			/* FetchTexel3Df */
   store_texel_i8			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_ci8 = {
   MESA_FORMAT_CI8,			/* MesaFormat */
   GL_COLOR_INDEX,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   8,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   1,					/* TexelBytes */
   _mesa_texstore_ci8,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_ci8,		/* FetchTexel1Df */
   fetch_texel_2d_f_ci8,		/* FetchTexel2Df */
   fetch_texel_3d_f_ci8,		/* FetchTexel3Df */
   store_texel_ci8			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_ycbcr = {
   MESA_FORMAT_YCBCR,			/* MesaFormat */
   GL_YCBCR_MESA,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_ycbcr,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_ycbcr,		/* FetchTexel1Df */
   fetch_texel_2d_f_ycbcr,		/* FetchTexel2Df */
   fetch_texel_3d_f_ycbcr,		/* FetchTexel3Df */
   store_texel_ycbcr			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_ycbcr_rev = {
   MESA_FORMAT_YCBCR_REV,		/* MesaFormat */
   GL_YCBCR_MESA,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   2,					/* TexelBytes */
   _mesa_texstore_ycbcr,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_ycbcr_rev,		/* FetchTexel1Df */
   fetch_texel_2d_f_ycbcr_rev,		/* FetchTexel2Df */
   fetch_texel_3d_f_ycbcr_rev,		/* FetchTexel3Df */
   store_texel_ycbcr_rev		/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_z24_s8 = {
   MESA_FORMAT_Z24_S8,			/* MesaFormat */
   GL_DEPTH_STENCIL_EXT,		/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   24,					/* DepthBits */
   8,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_z24_s8,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_z24_s8,		/* FetchTexel1Df */
   fetch_texel_2d_f_z24_s8,		/* FetchTexel2Df */
   fetch_texel_3d_f_z24_s8,		/* FetchTexel3Df */
   store_texel_z24_s8			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_s8_z24 = {
   MESA_FORMAT_S8_Z24,			/* MesaFormat */
   GL_DEPTH_STENCIL_EXT,		/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   24,					/* DepthBits */
   8,					/* StencilBits */
   4,					/* TexelBytes */
   _mesa_texstore_s8_z24,		/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel2D */
   NULL,				/* FetchTexel3D */
   fetch_texel_1d_f_s8_z24,		/* FetchTexel1Df */
   fetch_texel_2d_f_s8_z24,		/* FetchTexel2Df */
   fetch_texel_3d_f_s8_z24,		/* FetchTexel3Df */
   store_texel_s8_z24			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_z16 = {
   MESA_FORMAT_Z16,			/* MesaFormat */
   GL_DEPTH_COMPONENT,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   sizeof(GLushort) * 8,		/* DepthBits */
   0,					/* StencilBits */
   sizeof(GLushort),			/* TexelBytes */
   _mesa_texstore_z16,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   fetch_texel_1d_f_z16,		/* FetchTexel1Df */
   fetch_texel_2d_f_z16,		/* FetchTexel2Df */
   fetch_texel_3d_f_z16,		/* FetchTexel3Df */
   store_texel_z16			/* StoreTexel */
};

const struct gl_texture_format _mesa_texformat_z32 = {
   MESA_FORMAT_Z32,			/* MesaFormat */
   GL_DEPTH_COMPONENT,			/* BaseFormat */
   GL_UNSIGNED_NORMALIZED_ARB,		/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   sizeof(GLuint) * 8,			/* DepthBits */
   0,					/* StencilBits */
   sizeof(GLuint),			/* TexelBytes */
   _mesa_texstore_z32,			/* StoreTexImageFunc */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   NULL,				/* FetchTexel1D */
   fetch_texel_1d_f_z32,		/* FetchTexel1Df */
   fetch_texel_2d_f_z32,		/* FetchTexel2Df */
   fetch_texel_3d_f_z32,		/* FetchTexel3Df */
   store_texel_z32			/* StoreTexel */
};

/*@}*/


/***************************************************************/
/** \name Null format (useful for proxy textures) */
/*@{*/

const struct gl_texture_format _mesa_null_texformat = {
   -1,					/* MesaFormat */
   0,					/* BaseFormat */
   GL_NONE,				/* DataType */
   0,					/* RedBits */
   0,					/* GreenBits */
   0,					/* BlueBits */
   0,					/* AlphaBits */
   0,					/* LuminanceBits */
   0,					/* IntensityBits */
   0,					/* IndexBits */
   0,					/* DepthBits */
   0,					/* StencilBits */
   0,					/* TexelBytes */
   NULL,				/* StoreTexImageFunc */
   fetch_null_texel,			/* FetchTexel1D */
   fetch_null_texel,			/* FetchTexel2D */
   fetch_null_texel,			/* FetchTexel3D */
   fetch_null_texelf,			/* FetchTexel1Df */
   fetch_null_texelf,			/* FetchTexel2Df */
   fetch_null_texelf,			/* FetchTexel3Df */
   store_null_texel			/* StoreTexel */
};

/*@}*/


/**
 * Choose an appropriate texture format given the format, type and
 * internalFormat parameters passed to glTexImage().
 *
 * \param ctx  the GL context.
 * \param internalFormat  user's prefered internal texture format.
 * \param format  incoming image pixel format.
 * \param type  incoming image data type.
 *
 * \return a pointer to a gl_texture_format object which describes the
 * choosen texture format, or NULL on failure.
 * 
 * This is called via dd_function_table::ChooseTextureFormat.  Hardware drivers
 * will typically override this function with a specialized version.
 */
const struct gl_texture_format *
_mesa_choose_tex_format( GLcontext *ctx, GLint internalFormat,
                         GLenum format, GLenum type )
{
   (void) format;
   (void) type;

   switch (internalFormat) {
      /* RGBA formats */
      case 4:
      case GL_RGBA:
      case GL_RGB10_A2:
      case GL_RGBA12:
      case GL_RGBA16:
         return &_mesa_texformat_rgba;
      case GL_RGBA8:
         return &_mesa_texformat_rgba8888;
      case GL_RGB5_A1:
         return &_mesa_texformat_argb1555;
      case GL_RGBA2:
         return &_mesa_texformat_argb4444_rev; /* just to test another format*/
      case GL_RGBA4:
         return &_mesa_texformat_argb4444;

      /* RGB formats */
      case 3:
      case GL_RGB:
      case GL_RGB10:
      case GL_RGB12:
      case GL_RGB16:
         return &_mesa_texformat_rgb;
      case GL_RGB8:
         return &_mesa_texformat_rgb888;
      case GL_R3_G3_B2:
         return &_mesa_texformat_rgb332;
      case GL_RGB4:
         return &_mesa_texformat_rgb565_rev; /* just to test another format */
      case GL_RGB5:
         return &_mesa_texformat_rgb565;

      /* Alpha formats */
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA12:
      case GL_ALPHA16:
         return &_mesa_texformat_alpha;
      case GL_ALPHA8:
         return &_mesa_texformat_a8;

      /* Luminance formats */
      case 1:
      case GL_LUMINANCE:
      case GL_LUMINANCE4:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
         return &_mesa_texformat_luminance;
      case GL_LUMINANCE8:
         return &_mesa_texformat_l8;

      /* Luminance/Alpha formats */
      case 2:
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
         return &_mesa_texformat_luminance_alpha;
      case GL_LUMINANCE8_ALPHA8:
         return &_mesa_texformat_al88;

      case GL_INTENSITY:
      case GL_INTENSITY4:
      case GL_INTENSITY12:
      case GL_INTENSITY16:
         return &_mesa_texformat_intensity;
      case GL_INTENSITY8:
         return &_mesa_texformat_i8;

      case GL_COLOR_INDEX:
      case GL_COLOR_INDEX1_EXT:
      case GL_COLOR_INDEX2_EXT:
      case GL_COLOR_INDEX4_EXT:
      case GL_COLOR_INDEX12_EXT:
      case GL_COLOR_INDEX16_EXT:
      case GL_COLOR_INDEX8_EXT:
         return &_mesa_texformat_ci8;

      default:
         ; /* fallthrough */
   }

   if (ctx->Extensions.ARB_depth_texture) {
      switch (internalFormat) {
         case GL_DEPTH_COMPONENT:
         case GL_DEPTH_COMPONENT24:
         case GL_DEPTH_COMPONENT32:
            return &_mesa_texformat_z32;
         case GL_DEPTH_COMPONENT16:
            return &_mesa_texformat_z16;
         default:
            ; /* fallthrough */
      }
   }

   switch (internalFormat) {
      case GL_COMPRESSED_ALPHA_ARB:
         return &_mesa_texformat_alpha;
      case GL_COMPRESSED_LUMINANCE_ARB:
         return &_mesa_texformat_luminance;
      case GL_COMPRESSED_LUMINANCE_ALPHA_ARB:
         return &_mesa_texformat_luminance_alpha;
      case GL_COMPRESSED_INTENSITY_ARB:
         return &_mesa_texformat_intensity;
      case GL_COMPRESSED_RGB_ARB:
#if FEATURE_texture_s3tc
         if (ctx->Extensions.EXT_texture_compression_s3tc ||
             ctx->Extensions.S3_s3tc)
            return &_mesa_texformat_rgb_dxt1;
#endif
#if FEATURE_texture_fxt1
         if (ctx->Extensions.TDFX_texture_compression_FXT1)
            return &_mesa_texformat_rgb_fxt1;
#endif
         return &_mesa_texformat_rgb;
      case GL_COMPRESSED_RGBA_ARB:
#if FEATURE_texture_s3tc
         if (ctx->Extensions.EXT_texture_compression_s3tc ||
             ctx->Extensions.S3_s3tc)
            return &_mesa_texformat_rgba_dxt5; /* Not rgba_dxt1, see spec */
#endif
#if FEATURE_texture_fxt1
         if (ctx->Extensions.TDFX_texture_compression_FXT1)
            return &_mesa_texformat_rgba_fxt1;
#endif
         return &_mesa_texformat_rgba;
      default:
         ; /* fallthrough */
   }

   if (ctx->Extensions.MESA_ycbcr_texture) {
      if (internalFormat == GL_YCBCR_MESA) {
         if (type == GL_UNSIGNED_SHORT_8_8_MESA)
            return &_mesa_texformat_ycbcr;
         else
            return &_mesa_texformat_ycbcr_rev;
      }
   }

#if FEATURE_texture_fxt1
   if (ctx->Extensions.TDFX_texture_compression_FXT1) {
      switch (internalFormat) {
         case GL_COMPRESSED_RGB_FXT1_3DFX:
            return &_mesa_texformat_rgb_fxt1;
         case GL_COMPRESSED_RGBA_FXT1_3DFX:
            return &_mesa_texformat_rgba_fxt1;
         default:
            ; /* fallthrough */
      }
   }
#endif

#if FEATURE_texture_s3tc
   if (ctx->Extensions.EXT_texture_compression_s3tc) {
      switch (internalFormat) {
         case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            return &_mesa_texformat_rgb_dxt1;
         case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            return &_mesa_texformat_rgba_dxt1;
         case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            return &_mesa_texformat_rgba_dxt3;
         case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return &_mesa_texformat_rgba_dxt5;
         default:
            ; /* fallthrough */
      }
   }

   if (ctx->Extensions.S3_s3tc) {
      switch (internalFormat) {
         case GL_RGB_S3TC:
         case GL_RGB4_S3TC:
            return &_mesa_texformat_rgb_dxt1;
         case GL_RGBA_S3TC:
         case GL_RGBA4_S3TC:
            return &_mesa_texformat_rgba_dxt3;
         default:
            ; /* fallthrough */
      }
   }
#endif

   if (ctx->Extensions.ARB_texture_float) {
      switch (internalFormat) {
         case GL_ALPHA16F_ARB:
            return &_mesa_texformat_alpha_float16;
         case GL_ALPHA32F_ARB:
            return &_mesa_texformat_alpha_float32;
         case GL_LUMINANCE16F_ARB:
            return &_mesa_texformat_luminance_float16;
         case GL_LUMINANCE32F_ARB:
            return &_mesa_texformat_luminance_float32;
         case GL_LUMINANCE_ALPHA16F_ARB:
            return &_mesa_texformat_luminance_alpha_float16;
         case GL_LUMINANCE_ALPHA32F_ARB:
            return &_mesa_texformat_luminance_alpha_float32;
         case GL_INTENSITY16F_ARB:
            return &_mesa_texformat_intensity_float16;
         case GL_INTENSITY32F_ARB:
            return &_mesa_texformat_intensity_float32;
         case GL_RGB16F_ARB:
            return &_mesa_texformat_rgb_float16;
         case GL_RGB32F_ARB:
            return &_mesa_texformat_rgb_float32;
         case GL_RGBA16F_ARB:
            return &_mesa_texformat_rgba_float16;
         case GL_RGBA32F_ARB:
            return &_mesa_texformat_rgba_float32;
         default:
            ; /* fallthrough */
      }
   }

   if (ctx->Extensions.EXT_packed_depth_stencil) {
      switch (internalFormat) {
         case GL_DEPTH_STENCIL_EXT:
         case GL_DEPTH24_STENCIL8_EXT:
            return &_mesa_texformat_z24_s8;
         default:
            ; /* fallthrough */
      }
   }

   if (ctx->Extensions.ATI_envmap_bumpmap) {
      switch (internalFormat) {
         case GL_DUDV_ATI:
         case GL_DU8DV8_ATI:
            return &_mesa_texformat_dudv8;
         default:
            ; /* fallthrough */
      }
   }

   if (ctx->Extensions.MESA_texture_signed_rgba) {
      switch (internalFormat) {
         case GL_RGBA_SNORM:
         case GL_RGBA8_SNORM:
            return &_mesa_texformat_signed_rgba8888;
         default:
            ; /* fallthrough */
      }
   }


#if FEATURE_EXT_texture_sRGB
   if (ctx->Extensions.EXT_texture_sRGB) {
      switch (internalFormat) {
         case GL_SRGB_EXT:
         case GL_SRGB8_EXT:
            return &_mesa_texformat_srgb8;
         case GL_SRGB_ALPHA_EXT:
         case GL_SRGB8_ALPHA8_EXT:
            return &_mesa_texformat_srgba8;
         case GL_SLUMINANCE_EXT:
         case GL_SLUMINANCE8_EXT:
            return &_mesa_texformat_sl8;
         case GL_SLUMINANCE_ALPHA_EXT:
         case GL_SLUMINANCE8_ALPHA8_EXT:
            return &_mesa_texformat_sla8;
         case GL_COMPRESSED_SLUMINANCE_EXT:
            return &_mesa_texformat_sl8;
         case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
            return &_mesa_texformat_sla8;
         case GL_COMPRESSED_SRGB_EXT:
#if FEATURE_texture_s3tc
            if (ctx->Extensions.EXT_texture_compression_s3tc)
               return &_mesa_texformat_srgb_dxt1;
#endif
            return &_mesa_texformat_srgb8;
         case GL_COMPRESSED_SRGB_ALPHA_EXT:
#if FEATURE_texture_s3tc
            if (ctx->Extensions.EXT_texture_compression_s3tc)
               return &_mesa_texformat_srgba_dxt3; /* Not srgba_dxt1, see spec */
#endif
            return &_mesa_texformat_srgba8;
#if FEATURE_texture_s3tc
         case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
            if (ctx->Extensions.EXT_texture_compression_s3tc)
               return &_mesa_texformat_srgb_dxt1;
            break;
         case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
            if (ctx->Extensions.EXT_texture_compression_s3tc)
               return &_mesa_texformat_srgba_dxt1;
            break;
         case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
            if (ctx->Extensions.EXT_texture_compression_s3tc)
               return &_mesa_texformat_srgba_dxt3;
            break;
         case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
            if (ctx->Extensions.EXT_texture_compression_s3tc)
               return &_mesa_texformat_srgba_dxt5;
            break;
#endif
         default:
            ; /* fallthrough */
      }
   }
#endif /* FEATURE_EXT_texture_sRGB */

   _mesa_problem(ctx, "unexpected format in _mesa_choose_tex_format()");
   return NULL;
}



/**
 * Return datatype and number of components per texel for the
 * given gl_texture_format.
 */
void
_mesa_format_to_type_and_comps(const struct gl_texture_format *format,
                               GLenum *datatype, GLuint *comps)
{
   switch (format->MesaFormat) {
   case MESA_FORMAT_RGBA8888:
   case MESA_FORMAT_RGBA8888_REV:
   case MESA_FORMAT_ARGB8888:
   case MESA_FORMAT_ARGB8888_REV:
      *datatype = CHAN_TYPE;
      *comps = 4;
      return;
   case MESA_FORMAT_RGB888:
   case MESA_FORMAT_BGR888:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 3;
      return;
   case MESA_FORMAT_RGB565:
   case MESA_FORMAT_RGB565_REV:
      *datatype = GL_UNSIGNED_SHORT_5_6_5;
      *comps = 3;
      return;

   case MESA_FORMAT_ARGB4444:
   case MESA_FORMAT_ARGB4444_REV:
      *datatype = GL_UNSIGNED_SHORT_4_4_4_4;
      *comps = 4;
      return;

   case MESA_FORMAT_ARGB1555:
   case MESA_FORMAT_ARGB1555_REV:
      *datatype = GL_UNSIGNED_SHORT_1_5_5_5_REV;
      *comps = 4;
      return;

   case MESA_FORMAT_AL88:
   case MESA_FORMAT_AL88_REV:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 2;
      return;
   case MESA_FORMAT_RGB332:
      *datatype = GL_UNSIGNED_BYTE_3_3_2;
      *comps = 3;
      return;

   case MESA_FORMAT_A8:
   case MESA_FORMAT_L8:
   case MESA_FORMAT_I8:
   case MESA_FORMAT_CI8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 1;
      return;

   case MESA_FORMAT_YCBCR:
   case MESA_FORMAT_YCBCR_REV:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 2;
      return;

   case MESA_FORMAT_Z24_S8:
      *datatype = GL_UNSIGNED_INT;
      *comps = 1; /* XXX OK? */
      return;

   case MESA_FORMAT_S8_Z24:
      *datatype = GL_UNSIGNED_INT;
      *comps = 1; /* XXX OK? */
      return;

   case MESA_FORMAT_Z16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 1;
      return;

   case MESA_FORMAT_Z32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 1;
      return;

   case MESA_FORMAT_DUDV8:
      *datatype = GL_BYTE;
      *comps = 2;
      return;

   case MESA_FORMAT_SIGNED_RGBA8888:
   case MESA_FORMAT_SIGNED_RGBA8888_REV:
      *datatype = GL_BYTE;
      *comps = 4;
      return;

#if FEATURE_EXT_texture_sRGB
   case MESA_FORMAT_SRGB8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 3;
      return;
   case MESA_FORMAT_SRGBA8:
   case MESA_FORMAT_SARGB8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 4;
      return;
   case MESA_FORMAT_SL8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 1;
      return;
   case MESA_FORMAT_SLA8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 2;
      return;
#endif

#if FEATURE_texture_fxt1
   case MESA_FORMAT_RGB_FXT1:
   case MESA_FORMAT_RGBA_FXT1:
#endif
#if FEATURE_texture_s3tc
   case MESA_FORMAT_RGB_DXT1:
   case MESA_FORMAT_RGBA_DXT1:
   case MESA_FORMAT_RGBA_DXT3:
   case MESA_FORMAT_RGBA_DXT5:
#if FEATURE_EXT_texture_sRGB
   case MESA_FORMAT_SRGB_DXT1:
   case MESA_FORMAT_SRGBA_DXT1:
   case MESA_FORMAT_SRGBA_DXT3:
   case MESA_FORMAT_SRGBA_DXT5:
#endif
      /* XXX generate error instead? */
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 0;
      return;
#endif

   case MESA_FORMAT_RGBA:
      *datatype = CHAN_TYPE;
      *comps = 4;
      return;
   case MESA_FORMAT_RGB:
      *datatype = CHAN_TYPE;
      *comps = 3;
      return;
   case MESA_FORMAT_LUMINANCE_ALPHA:
      *datatype = CHAN_TYPE;
      *comps = 2;
      return;
   case MESA_FORMAT_ALPHA:
   case MESA_FORMAT_LUMINANCE:
   case MESA_FORMAT_INTENSITY:
      *datatype = CHAN_TYPE;
      *comps = 1;
      return;

   case MESA_FORMAT_RGBA_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 4;
      return;
   case MESA_FORMAT_RGBA_FLOAT16:
      *datatype = GL_HALF_FLOAT_ARB;
      *comps = 4;
      return;
   case MESA_FORMAT_RGB_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 3;
      return;
   case MESA_FORMAT_RGB_FLOAT16:
      *datatype = GL_HALF_FLOAT_ARB;
      *comps = 3;
      return;
   case MESA_FORMAT_LUMINANCE_ALPHA_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 2;
      return;
   case MESA_FORMAT_LUMINANCE_ALPHA_FLOAT16:
      *datatype = GL_HALF_FLOAT_ARB;
      *comps = 2;
      return;
   case MESA_FORMAT_ALPHA_FLOAT32:
   case MESA_FORMAT_LUMINANCE_FLOAT32:
   case MESA_FORMAT_INTENSITY_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 1;
      return;
   case MESA_FORMAT_ALPHA_FLOAT16:
   case MESA_FORMAT_LUMINANCE_FLOAT16:
   case MESA_FORMAT_INTENSITY_FLOAT16:
      *datatype = GL_HALF_FLOAT_ARB;
      *comps = 1;
      return;

   default:
      _mesa_problem(NULL, "bad format in _mesa_format_to_type_and_comps");
      *datatype = 0;
      *comps = 1;
   }
}
