/*
 * Mesa 3-D graphics library
 * Version:  7.7
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
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
 * \file mtypes.h
 * Main Mesa data structures.
 *
 * Please try to mark derived values with a leading underscore ('_').
 */

#ifndef MTYPES_H
#define MTYPES_H


#include "main/glheader.h"
#include "main/config.h"
#include "main/mfeatures.h"
#include "glapi/glapi.h"
#include "math/m_matrix.h"	/* GLmatrix */
#include "main/simple_list.h"	/* struct simple_node */
#include "main/formats.h"       /* MESA_FORMAT_COUNT */


#ifdef __cplusplus
extern "C" {
#endif


/**
 * \name 64-bit extension of GLbitfield.
 */
/*@{*/
typedef GLuint64 GLbitfield64;

/** Set a single bit */
#define BITFIELD64_BIT(b)      ((GLbitfield64)1 << (b))
/** Set all bits up to excluding bit b */
#define BITFIELD64_MASK(b)      \
   ((b) == 64 ? (~(GLbitfield64)0) : BITFIELD64_BIT(b) - 1)
/** Set count bits starting from bit b  */
#define BITFIELD64_RANGE(b, count) \
   (BITFIELD64_MASK((b) + (count)) & ~BITFIELD64_MASK(b))


/**
 * \name Some forward type declarations
 */
/*@{*/
struct _mesa_HashTable;
struct gl_attrib_node;
struct gl_list_extensions;
struct gl_meta_state;
struct gl_program_cache;
struct gl_texture_object;
struct gl_context;
struct st_context;
struct gl_uniform_storage;
struct prog_instruction;
struct gl_program_parameter_list;
/*@}*/


/** Extra draw modes beyond GL_POINTS, GL_TRIANGLE_FAN, etc */
#define PRIM_OUTSIDE_BEGIN_END   (GL_POLYGON+1)
#define PRIM_INSIDE_UNKNOWN_PRIM (GL_POLYGON+2)
#define PRIM_UNKNOWN             (GL_POLYGON+3)



/**
 * Indexes for vertex program attributes.
 * GL_NV_vertex_program aliases generic attributes over the conventional
 * attributes.  In GL_ARB_vertex_program shader the aliasing is optional.
 * In GL_ARB_vertex_shader / OpenGL 2.0 the aliasing is disallowed (the
 * generic attributes are distinct/separate).
 */
typedef enum
{
   VERT_ATTRIB_POS = 0,
   VERT_ATTRIB_WEIGHT = 1,
   VERT_ATTRIB_NORMAL = 2,
   VERT_ATTRIB_COLOR0 = 3,
   VERT_ATTRIB_COLOR1 = 4,
   VERT_ATTRIB_FOG = 5,
   VERT_ATTRIB_COLOR_INDEX = 6,
   VERT_ATTRIB_EDGEFLAG = 7,
   VERT_ATTRIB_TEX0 = 8,
   VERT_ATTRIB_TEX1 = 9,
   VERT_ATTRIB_TEX2 = 10,
   VERT_ATTRIB_TEX3 = 11,
   VERT_ATTRIB_TEX4 = 12,
   VERT_ATTRIB_TEX5 = 13,
   VERT_ATTRIB_TEX6 = 14,
   VERT_ATTRIB_TEX7 = 15,
   VERT_ATTRIB_POINT_SIZE = 16,
   VERT_ATTRIB_GENERIC0 = 17,
   VERT_ATTRIB_GENERIC1 = 18,
   VERT_ATTRIB_GENERIC2 = 19,
   VERT_ATTRIB_GENERIC3 = 20,
   VERT_ATTRIB_GENERIC4 = 21,
   VERT_ATTRIB_GENERIC5 = 22,
   VERT_ATTRIB_GENERIC6 = 23,
   VERT_ATTRIB_GENERIC7 = 24,
   VERT_ATTRIB_GENERIC8 = 25,
   VERT_ATTRIB_GENERIC9 = 26,
   VERT_ATTRIB_GENERIC10 = 27,
   VERT_ATTRIB_GENERIC11 = 28,
   VERT_ATTRIB_GENERIC12 = 29,
   VERT_ATTRIB_GENERIC13 = 30,
   VERT_ATTRIB_GENERIC14 = 31,
   VERT_ATTRIB_GENERIC15 = 32,
   VERT_ATTRIB_MAX = 33
} gl_vert_attrib;

/**
 * Symbolic constats to help iterating over
 * specific blocks of vertex attributes.
 *
 * VERT_ATTRIB_FF
 *   includes all fixed function attributes as well as
 *   the aliased GL_NV_vertex_program shader attributes.
 * VERT_ATTRIB_TEX
 *   include the classic texture coordinate attributes.
 *   Is a subset of VERT_ATTRIB_FF.
 * VERT_ATTRIB_GENERIC_NV
 *   include the NV shader attributes.
 *   Is a subset of VERT_ATTRIB_FF.
 * VERT_ATTRIB_GENERIC
 *   include the OpenGL 2.0+ GLSL generic shader attributes.
 *   These alias the generic GL_ARB_vertex_shader attributes.
 */
#define VERT_ATTRIB_FF(i)           (VERT_ATTRIB_POS + (i))
#define VERT_ATTRIB_FF_MAX          VERT_ATTRIB_GENERIC0

#define VERT_ATTRIB_TEX(i)          (VERT_ATTRIB_TEX0 + (i))
#define VERT_ATTRIB_TEX_MAX         MAX_TEXTURE_COORD_UNITS

#define VERT_ATTRIB_GENERIC_NV(i)   (VERT_ATTRIB_POS + (i))
#define VERT_ATTRIB_GENERIC_NV_MAX  MAX_VERTEX_GENERIC_ATTRIBS

#define VERT_ATTRIB_GENERIC(i)      (VERT_ATTRIB_GENERIC0 + (i))
#define VERT_ATTRIB_GENERIC_MAX     MAX_VERTEX_GENERIC_ATTRIBS

/**
 * Bitflags for vertex attributes.
 * These are used in bitfields in many places.
 */
/*@{*/
#define VERT_BIT_POS             BITFIELD64_BIT(VERT_ATTRIB_POS)
#define VERT_BIT_WEIGHT          BITFIELD64_BIT(VERT_ATTRIB_WEIGHT)
#define VERT_BIT_NORMAL          BITFIELD64_BIT(VERT_ATTRIB_NORMAL)
#define VERT_BIT_COLOR0          BITFIELD64_BIT(VERT_ATTRIB_COLOR0)
#define VERT_BIT_COLOR1          BITFIELD64_BIT(VERT_ATTRIB_COLOR1)
#define VERT_BIT_FOG             BITFIELD64_BIT(VERT_ATTRIB_FOG)
#define VERT_BIT_COLOR_INDEX     BITFIELD64_BIT(VERT_ATTRIB_COLOR_INDEX)
#define VERT_BIT_EDGEFLAG        BITFIELD64_BIT(VERT_ATTRIB_EDGEFLAG)
#define VERT_BIT_TEX0            BITFIELD64_BIT(VERT_ATTRIB_TEX0)
#define VERT_BIT_TEX1            BITFIELD64_BIT(VERT_ATTRIB_TEX1)
#define VERT_BIT_TEX2            BITFIELD64_BIT(VERT_ATTRIB_TEX2)
#define VERT_BIT_TEX3            BITFIELD64_BIT(VERT_ATTRIB_TEX3)
#define VERT_BIT_TEX4            BITFIELD64_BIT(VERT_ATTRIB_TEX4)
#define VERT_BIT_TEX5            BITFIELD64_BIT(VERT_ATTRIB_TEX5)
#define VERT_BIT_TEX6            BITFIELD64_BIT(VERT_ATTRIB_TEX6)
#define VERT_BIT_TEX7            BITFIELD64_BIT(VERT_ATTRIB_TEX7)
#define VERT_BIT_POINT_SIZE      BITFIELD64_BIT(VERT_ATTRIB_POINT_SIZE)
#define VERT_BIT_GENERIC0        BITFIELD64_BIT(VERT_ATTRIB_GENERIC0)

#define VERT_BIT(i)              BITFIELD64_BIT(i)
#define VERT_BIT_ALL             BITFIELD64_RANGE(0, VERT_ATTRIB_MAX)

#define VERT_BIT_FF(i)           VERT_BIT(i)
#define VERT_BIT_FF_ALL          BITFIELD64_RANGE(0, VERT_ATTRIB_FF_MAX)
#define VERT_BIT_TEX(i)          VERT_BIT(VERT_ATTRIB_TEX(i))
#define VERT_BIT_TEX_ALL         \
   BITFIELD64_RANGE(VERT_ATTRIB_TEX(0), VERT_ATTRIB_TEX_MAX)
#define VERT_BIT_FF_NVALIAS      \
   BITFIELD64_RANGE(VERT_ATTRIB_POS, VERT_ATTRIB_TEX(VERT_ATTRIB_TEX_MAX))

#define VERT_BIT_GENERIC_NV(i)   VERT_BIT(VERT_ATTRIB_GENERIC_NV(i))
#define VERT_BIT_GENERIC_NV_ALL  \
   BITFIELD64_RANGE(VERT_ATTRIB_GENERIC_NV(0), VERT_ATTRIB_GENERIC_NV_MAX)

#define VERT_BIT_GENERIC(i)      VERT_BIT(VERT_ATTRIB_GENERIC(i))
#define VERT_BIT_GENERIC_ALL     \
   BITFIELD64_RANGE(VERT_ATTRIB_GENERIC(0), VERT_ATTRIB_GENERIC_MAX)
/*@}*/


/**
 * Indexes for vertex program result attributes.  Note that
 * _mesa_vert_result_to_frag_attrib() and _mesa_frag_attrib_to_vert_result() make
 * assumptions about the layout of this enum.
 */
typedef enum
{
   VERT_RESULT_HPOS = 0,
   VERT_RESULT_COL0 = 1,
   VERT_RESULT_COL1 = 2,
   VERT_RESULT_FOGC = 3,
   VERT_RESULT_TEX0 = 4,
   VERT_RESULT_TEX1 = 5,
   VERT_RESULT_TEX2 = 6,
   VERT_RESULT_TEX3 = 7,
   VERT_RESULT_TEX4 = 8,
   VERT_RESULT_TEX5 = 9,
   VERT_RESULT_TEX6 = 10,
   VERT_RESULT_TEX7 = 11,
   VERT_RESULT_PSIZ = 12,
   VERT_RESULT_BFC0 = 13,
   VERT_RESULT_BFC1 = 14,
   VERT_RESULT_EDGE = 15,
   VERT_RESULT_CLIP_VERTEX = 16,
   VERT_RESULT_CLIP_DIST0 = 17,
   VERT_RESULT_CLIP_DIST1 = 18,
   VERT_RESULT_VAR0 = 19,  /**< shader varying */
   VERT_RESULT_MAX = (VERT_RESULT_VAR0 + MAX_VARYING)
} gl_vert_result;


/*********************************************/

/**
 * Indexes for geometry program attributes.
 */
typedef enum
{
   GEOM_ATTRIB_POSITION = 0,
   GEOM_ATTRIB_COLOR0 = 1,
   GEOM_ATTRIB_COLOR1 = 2,
   GEOM_ATTRIB_SECONDARY_COLOR0 = 3,
   GEOM_ATTRIB_SECONDARY_COLOR1 = 4,
   GEOM_ATTRIB_FOG_FRAG_COORD = 5,
   GEOM_ATTRIB_POINT_SIZE = 6,
   GEOM_ATTRIB_CLIP_VERTEX = 7,
   GEOM_ATTRIB_PRIMITIVE_ID = 8,
   GEOM_ATTRIB_TEX_COORD = 9,

   GEOM_ATTRIB_VAR0 = 16,
   GEOM_ATTRIB_MAX = (GEOM_ATTRIB_VAR0 + MAX_VARYING)
} gl_geom_attrib;

/**
 * Bitflags for geometry attributes.
 * These are used in bitfields in many places.
 */
/*@{*/
#define GEOM_BIT_COLOR0      (1 << GEOM_ATTRIB_COLOR0)
#define GEOM_BIT_COLOR1      (1 << GEOM_ATTRIB_COLOR1)
#define GEOM_BIT_SCOLOR0     (1 << GEOM_ATTRIB_SECONDARY_COLOR0)
#define GEOM_BIT_SCOLOR1     (1 << GEOM_ATTRIB_SECONDARY_COLOR1)
#define GEOM_BIT_TEX_COORD   (1 << GEOM_ATTRIB_TEX_COORD)
#define GEOM_BIT_FOG_COORD   (1 << GEOM_ATTRIB_FOG_FRAG_COORD)
#define GEOM_BIT_POSITION    (1 << GEOM_ATTRIB_POSITION)
#define GEOM_BIT_POINT_SIDE  (1 << GEOM_ATTRIB_POINT_SIZE)
#define GEOM_BIT_CLIP_VERTEX (1 << GEOM_ATTRIB_CLIP_VERTEX)
#define GEOM_BIT_PRIM_ID     (1 << GEOM_ATTRIB_PRIMITIVE_ID)
#define GEOM_BIT_VAR0        (1 << GEOM_ATTRIB_VAR0)

#define GEOM_BIT_VAR(g)  (1 << (GEOM_BIT_VAR0 + (g)))
/*@}*/


/**
 * Indexes for geometry program result attributes
 */
typedef enum
{
   GEOM_RESULT_POS  = 0,
   GEOM_RESULT_COL0  = 1,
   GEOM_RESULT_COL1  = 2,
   GEOM_RESULT_SCOL0 = 3,
   GEOM_RESULT_SCOL1 = 4,
   GEOM_RESULT_FOGC = 5,
   GEOM_RESULT_TEX0 = 6,
   GEOM_RESULT_TEX1 = 7,
   GEOM_RESULT_TEX2 = 8,
   GEOM_RESULT_TEX3 = 9,
   GEOM_RESULT_TEX4 = 10,
   GEOM_RESULT_TEX5 = 11,
   GEOM_RESULT_TEX6 = 12,
   GEOM_RESULT_TEX7 = 13,
   GEOM_RESULT_PSIZ = 14,
   GEOM_RESULT_CLPV = 15,
   GEOM_RESULT_PRID = 16,
   GEOM_RESULT_LAYR = 17,
   GEOM_RESULT_VAR0 = 18,  /**< shader varying, should really be 16 */
   /* ### we need to -2 because var0 is 18 instead 16 like in the others */
   GEOM_RESULT_MAX  =  (GEOM_RESULT_VAR0 + MAX_VARYING - 2)
} gl_geom_result;


/**
 * Indexes for fragment program input attributes.  Note that
 * _mesa_vert_result_to_frag_attrib() and frag_attrib_to_vert_result() make
 * assumptions about the layout of this enum.
 */
typedef enum
{
   FRAG_ATTRIB_WPOS = 0,
   FRAG_ATTRIB_COL0 = 1,
   FRAG_ATTRIB_COL1 = 2,
   FRAG_ATTRIB_FOGC = 3,
   FRAG_ATTRIB_TEX0 = 4,
   FRAG_ATTRIB_TEX1 = 5,
   FRAG_ATTRIB_TEX2 = 6,
   FRAG_ATTRIB_TEX3 = 7,
   FRAG_ATTRIB_TEX4 = 8,
   FRAG_ATTRIB_TEX5 = 9,
   FRAG_ATTRIB_TEX6 = 10,
   FRAG_ATTRIB_TEX7 = 11,
   FRAG_ATTRIB_FACE = 12,  /**< front/back face */
   FRAG_ATTRIB_PNTC = 13,  /**< sprite/point coord */
   FRAG_ATTRIB_CLIP_DIST0 = 14,
   FRAG_ATTRIB_CLIP_DIST1 = 15,
   FRAG_ATTRIB_VAR0 = 16,  /**< shader varying */
   FRAG_ATTRIB_MAX = (FRAG_ATTRIB_VAR0 + MAX_VARYING)
} gl_frag_attrib;


/**
 * Convert from a gl_vert_result value to the corresponding gl_frag_attrib.
 *
 * VERT_RESULT_HPOS is converted to FRAG_ATTRIB_WPOS.
 *
 * gl_vert_result values which have no corresponding gl_frag_attrib
 * (VERT_RESULT_PSIZ, VERT_RESULT_BFC0, VERT_RESULT_BFC1, and
 * VERT_RESULT_EDGE) are converted to a value of -1.
 */
static inline int
_mesa_vert_result_to_frag_attrib(gl_vert_result vert_result)
{
   if (vert_result >= VERT_RESULT_CLIP_DIST0)
      return vert_result - VERT_RESULT_CLIP_DIST0 + FRAG_ATTRIB_CLIP_DIST0;
   else if (vert_result <= VERT_RESULT_TEX7)
      return vert_result;
   else
      return -1;
}


/**
 * Convert from a gl_frag_attrib value to the corresponding gl_vert_result.
 *
 * FRAG_ATTRIB_WPOS is converted to VERT_RESULT_HPOS.
 *
 * gl_frag_attrib values which have no corresponding gl_vert_result
 * (FRAG_ATTRIB_FACE and FRAG_ATTRIB_PNTC) are converted to a value of -1.
 */
static inline int
_mesa_frag_attrib_to_vert_result(gl_frag_attrib frag_attrib)
{
   if (frag_attrib <= FRAG_ATTRIB_TEX7)
      return frag_attrib;
   else if (frag_attrib >= FRAG_ATTRIB_CLIP_DIST0)
      return frag_attrib - FRAG_ATTRIB_CLIP_DIST0 + VERT_RESULT_CLIP_DIST0;
   else
      return -1;
}


/**
 * Bitflags for fragment program input attributes.
 */
/*@{*/
#define FRAG_BIT_WPOS  (1 << FRAG_ATTRIB_WPOS)
#define FRAG_BIT_COL0  (1 << FRAG_ATTRIB_COL0)
#define FRAG_BIT_COL1  (1 << FRAG_ATTRIB_COL1)
#define FRAG_BIT_FOGC  (1 << FRAG_ATTRIB_FOGC)
#define FRAG_BIT_FACE  (1 << FRAG_ATTRIB_FACE)
#define FRAG_BIT_PNTC  (1 << FRAG_ATTRIB_PNTC)
#define FRAG_BIT_TEX0  (1 << FRAG_ATTRIB_TEX0)
#define FRAG_BIT_TEX1  (1 << FRAG_ATTRIB_TEX1)
#define FRAG_BIT_TEX2  (1 << FRAG_ATTRIB_TEX2)
#define FRAG_BIT_TEX3  (1 << FRAG_ATTRIB_TEX3)
#define FRAG_BIT_TEX4  (1 << FRAG_ATTRIB_TEX4)
#define FRAG_BIT_TEX5  (1 << FRAG_ATTRIB_TEX5)
#define FRAG_BIT_TEX6  (1 << FRAG_ATTRIB_TEX6)
#define FRAG_BIT_TEX7  (1 << FRAG_ATTRIB_TEX7)
#define FRAG_BIT_VAR0  (1 << FRAG_ATTRIB_VAR0)

#define FRAG_BIT_TEX(U)  (FRAG_BIT_TEX0 << (U))
#define FRAG_BIT_VAR(V)  (FRAG_BIT_VAR0 << (V))

#define FRAG_BITS_TEX_ANY (FRAG_BIT_TEX0|	\
			   FRAG_BIT_TEX1|	\
			   FRAG_BIT_TEX2|	\
			   FRAG_BIT_TEX3|	\
			   FRAG_BIT_TEX4|	\
			   FRAG_BIT_TEX5|	\
			   FRAG_BIT_TEX6|	\
			   FRAG_BIT_TEX7)
/*@}*/


/**
 * Fragment program results
 */
typedef enum
{
   FRAG_RESULT_DEPTH = 0,
   FRAG_RESULT_STENCIL = 1,
   /* If a single color should be written to all render targets, this
    * register is written.  No FRAG_RESULT_DATAn will be written.
    */
   FRAG_RESULT_COLOR = 2,

   /* FRAG_RESULT_DATAn are the per-render-target (GLSL gl_FragData[n]
    * or ARB_fragment_program fragment.color[n]) color results.  If
    * any are written, FRAG_RESULT_COLOR will not be written.
    */
   FRAG_RESULT_DATA0 = 3,
   FRAG_RESULT_MAX = (FRAG_RESULT_DATA0 + MAX_DRAW_BUFFERS)
} gl_frag_result;


/**
 * Indexes for all renderbuffers
 */
typedef enum
{
   /* the four standard color buffers */
   BUFFER_FRONT_LEFT,
   BUFFER_BACK_LEFT,
   BUFFER_FRONT_RIGHT,
   BUFFER_BACK_RIGHT,
   BUFFER_DEPTH,
   BUFFER_STENCIL,
   BUFFER_ACCUM,
   /* optional aux buffer */
   BUFFER_AUX0,
   /* generic renderbuffers */
   BUFFER_COLOR0,
   BUFFER_COLOR1,
   BUFFER_COLOR2,
   BUFFER_COLOR3,
   BUFFER_COLOR4,
   BUFFER_COLOR5,
   BUFFER_COLOR6,
   BUFFER_COLOR7,
   BUFFER_COUNT
} gl_buffer_index;

/**
 * Bit flags for all renderbuffers
 */
#define BUFFER_BIT_FRONT_LEFT   (1 << BUFFER_FRONT_LEFT)
#define BUFFER_BIT_BACK_LEFT    (1 << BUFFER_BACK_LEFT)
#define BUFFER_BIT_FRONT_RIGHT  (1 << BUFFER_FRONT_RIGHT)
#define BUFFER_BIT_BACK_RIGHT   (1 << BUFFER_BACK_RIGHT)
#define BUFFER_BIT_AUX0         (1 << BUFFER_AUX0)
#define BUFFER_BIT_AUX1         (1 << BUFFER_AUX1)
#define BUFFER_BIT_AUX2         (1 << BUFFER_AUX2)
#define BUFFER_BIT_AUX3         (1 << BUFFER_AUX3)
#define BUFFER_BIT_DEPTH        (1 << BUFFER_DEPTH)
#define BUFFER_BIT_STENCIL      (1 << BUFFER_STENCIL)
#define BUFFER_BIT_ACCUM        (1 << BUFFER_ACCUM)
#define BUFFER_BIT_COLOR0       (1 << BUFFER_COLOR0)
#define BUFFER_BIT_COLOR1       (1 << BUFFER_COLOR1)
#define BUFFER_BIT_COLOR2       (1 << BUFFER_COLOR2)
#define BUFFER_BIT_COLOR3       (1 << BUFFER_COLOR3)
#define BUFFER_BIT_COLOR4       (1 << BUFFER_COLOR4)
#define BUFFER_BIT_COLOR5       (1 << BUFFER_COLOR5)
#define BUFFER_BIT_COLOR6       (1 << BUFFER_COLOR6)
#define BUFFER_BIT_COLOR7       (1 << BUFFER_COLOR7)

/**
 * Mask of all the color buffer bits (but not accum).
 */
#define BUFFER_BITS_COLOR  (BUFFER_BIT_FRONT_LEFT | \
                            BUFFER_BIT_BACK_LEFT | \
                            BUFFER_BIT_FRONT_RIGHT | \
                            BUFFER_BIT_BACK_RIGHT | \
                            BUFFER_BIT_AUX0 | \
                            BUFFER_BIT_COLOR0 | \
                            BUFFER_BIT_COLOR1 | \
                            BUFFER_BIT_COLOR2 | \
                            BUFFER_BIT_COLOR3 | \
                            BUFFER_BIT_COLOR4 | \
                            BUFFER_BIT_COLOR5 | \
                            BUFFER_BIT_COLOR6 | \
                            BUFFER_BIT_COLOR7)


/**
 * Framebuffer configuration (aka visual / pixelformat)
 * Note: some of these fields should be boolean, but it appears that
 * code in drivers/dri/common/util.c requires int-sized fields.
 */
struct gl_config
{
   GLboolean rgbMode;
   GLboolean floatMode;
   GLboolean colorIndexMode;  /* XXX is this used anywhere? */
   GLuint doubleBufferMode;
   GLuint stereoMode;

   GLboolean haveAccumBuffer;
   GLboolean haveDepthBuffer;
   GLboolean haveStencilBuffer;

   GLint redBits, greenBits, blueBits, alphaBits;	/* bits per comp */
   GLuint redMask, greenMask, blueMask, alphaMask;
   GLint rgbBits;		/* total bits for rgb */
   GLint indexBits;		/* total bits for colorindex */

   GLint accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;
   GLint depthBits;
   GLint stencilBits;

   GLint numAuxBuffers;

   GLint level;

   /* EXT_visual_rating / GLX 1.2 */
   GLint visualRating;

   /* EXT_visual_info / GLX 1.2 */
   GLint transparentPixel;
   /*    colors are floats scaled to ints */
   GLint transparentRed, transparentGreen, transparentBlue, transparentAlpha;
   GLint transparentIndex;

   /* ARB_multisample / SGIS_multisample */
   GLint sampleBuffers;
   GLint samples;

   /* SGIX_pbuffer / GLX 1.3 */
   GLint maxPbufferWidth;
   GLint maxPbufferHeight;
   GLint maxPbufferPixels;
   GLint optimalPbufferWidth;   /* Only for SGIX_pbuffer. */
   GLint optimalPbufferHeight;  /* Only for SGIX_pbuffer. */

   /* OML_swap_method */
   GLint swapMethod;

   /* EXT_texture_from_pixmap */
   GLint bindToTextureRgb;
   GLint bindToTextureRgba;
   GLint bindToMipmapTexture;
   GLint bindToTextureTargets;
   GLint yInverted;

   /* EXT_framebuffer_sRGB */
   GLint sRGBCapable;
};


/**
 * \name Bit flags used for updating material values.
 */
/*@{*/
#define MAT_ATTRIB_FRONT_AMBIENT           0 
#define MAT_ATTRIB_BACK_AMBIENT            1
#define MAT_ATTRIB_FRONT_DIFFUSE           2 
#define MAT_ATTRIB_BACK_DIFFUSE            3
#define MAT_ATTRIB_FRONT_SPECULAR          4 
#define MAT_ATTRIB_BACK_SPECULAR           5
#define MAT_ATTRIB_FRONT_EMISSION          6
#define MAT_ATTRIB_BACK_EMISSION           7
#define MAT_ATTRIB_FRONT_SHININESS         8
#define MAT_ATTRIB_BACK_SHININESS          9
#define MAT_ATTRIB_FRONT_INDEXES           10
#define MAT_ATTRIB_BACK_INDEXES            11
#define MAT_ATTRIB_MAX                     12

#define MAT_ATTRIB_AMBIENT(f)  (MAT_ATTRIB_FRONT_AMBIENT+(f))  
#define MAT_ATTRIB_DIFFUSE(f)  (MAT_ATTRIB_FRONT_DIFFUSE+(f))  
#define MAT_ATTRIB_SPECULAR(f) (MAT_ATTRIB_FRONT_SPECULAR+(f)) 
#define MAT_ATTRIB_EMISSION(f) (MAT_ATTRIB_FRONT_EMISSION+(f)) 
#define MAT_ATTRIB_SHININESS(f)(MAT_ATTRIB_FRONT_SHININESS+(f))
#define MAT_ATTRIB_INDEXES(f)  (MAT_ATTRIB_FRONT_INDEXES+(f))  

#define MAT_INDEX_AMBIENT  0
#define MAT_INDEX_DIFFUSE  1
#define MAT_INDEX_SPECULAR 2

#define MAT_BIT_FRONT_AMBIENT         (1<<MAT_ATTRIB_FRONT_AMBIENT)
#define MAT_BIT_BACK_AMBIENT          (1<<MAT_ATTRIB_BACK_AMBIENT)
#define MAT_BIT_FRONT_DIFFUSE         (1<<MAT_ATTRIB_FRONT_DIFFUSE)
#define MAT_BIT_BACK_DIFFUSE          (1<<MAT_ATTRIB_BACK_DIFFUSE)
#define MAT_BIT_FRONT_SPECULAR        (1<<MAT_ATTRIB_FRONT_SPECULAR)
#define MAT_BIT_BACK_SPECULAR         (1<<MAT_ATTRIB_BACK_SPECULAR)
#define MAT_BIT_FRONT_EMISSION        (1<<MAT_ATTRIB_FRONT_EMISSION)
#define MAT_BIT_BACK_EMISSION         (1<<MAT_ATTRIB_BACK_EMISSION)
#define MAT_BIT_FRONT_SHININESS       (1<<MAT_ATTRIB_FRONT_SHININESS)
#define MAT_BIT_BACK_SHININESS        (1<<MAT_ATTRIB_BACK_SHININESS)
#define MAT_BIT_FRONT_INDEXES         (1<<MAT_ATTRIB_FRONT_INDEXES)
#define MAT_BIT_BACK_INDEXES          (1<<MAT_ATTRIB_BACK_INDEXES)


#define FRONT_MATERIAL_BITS	(MAT_BIT_FRONT_EMISSION | 	\
				 MAT_BIT_FRONT_AMBIENT |	\
				 MAT_BIT_FRONT_DIFFUSE | 	\
				 MAT_BIT_FRONT_SPECULAR |	\
				 MAT_BIT_FRONT_SHININESS | 	\
				 MAT_BIT_FRONT_INDEXES)

#define BACK_MATERIAL_BITS	(MAT_BIT_BACK_EMISSION |	\
				 MAT_BIT_BACK_AMBIENT |		\
				 MAT_BIT_BACK_DIFFUSE |		\
				 MAT_BIT_BACK_SPECULAR |	\
				 MAT_BIT_BACK_SHININESS |	\
				 MAT_BIT_BACK_INDEXES)

#define ALL_MATERIAL_BITS	(FRONT_MATERIAL_BITS | BACK_MATERIAL_BITS)
/*@}*/


/**
 * Material state.
 */
struct gl_material
{
   GLfloat Attrib[MAT_ATTRIB_MAX][4];
};


/**
 * Light state flags.
 */
/*@{*/
#define LIGHT_SPOT         0x1
#define LIGHT_LOCAL_VIEWER 0x2
#define LIGHT_POSITIONAL   0x4
#define LIGHT_NEED_VERTICES (LIGHT_POSITIONAL|LIGHT_LOCAL_VIEWER)
/*@}*/


/**
 * Light source state.
 */
struct gl_light
{
   struct gl_light *next;	/**< double linked list with sentinel */
   struct gl_light *prev;

   GLfloat Ambient[4];		/**< ambient color */
   GLfloat Diffuse[4];		/**< diffuse color */
   GLfloat Specular[4];		/**< specular color */
   GLfloat EyePosition[4];	/**< position in eye coordinates */
   GLfloat SpotDirection[4];	/**< spotlight direction in eye coordinates */
   GLfloat SpotExponent;
   GLfloat SpotCutoff;		/**< in degrees */
   GLfloat _CosCutoff;		/**< = MAX(0, cos(SpotCutoff)) */
   GLfloat ConstantAttenuation;
   GLfloat LinearAttenuation;
   GLfloat QuadraticAttenuation;
   GLboolean Enabled;		/**< On/off flag */

   /** 
    * \name Derived fields
    */
   /*@{*/
   GLbitfield _Flags;		/**< Mask of LIGHT_x bits defined above */

   GLfloat _Position[4];	/**< position in eye/obj coordinates */
   GLfloat _VP_inf_norm[3];	/**< Norm direction to infinite light */
   GLfloat _h_inf_norm[3];	/**< Norm( _VP_inf_norm + <0,0,1> ) */
   GLfloat _NormSpotDirection[4]; /**< normalized spotlight direction */
   GLfloat _VP_inf_spot_attenuation;

   GLfloat _MatAmbient[2][3];	/**< material ambient * light ambient */
   GLfloat _MatDiffuse[2][3];	/**< material diffuse * light diffuse */
   GLfloat _MatSpecular[2][3];	/**< material spec * light specular */
   /*@}*/
};


/**
 * Light model state.
 */
struct gl_lightmodel
{
   GLfloat Ambient[4];		/**< ambient color */
   GLboolean LocalViewer;	/**< Local (or infinite) view point? */
   GLboolean TwoSide;		/**< Two (or one) sided lighting? */
   GLenum ColorControl;		/**< either GL_SINGLE_COLOR
				 *    or GL_SEPARATE_SPECULAR_COLOR */
};


/**
 * Accumulation buffer attribute group (GL_ACCUM_BUFFER_BIT)
 */
struct gl_accum_attrib
{
   GLfloat ClearColor[4];	/**< Accumulation buffer clear color */
};


/**
 * Used for storing clear color, texture border color, etc.
 * The float values are typically unclamped.
 */
union gl_color_union
{
   GLfloat f[4];
   GLint i[4];
   GLuint ui[4];
};


/**
 * Color buffer attribute group (GL_COLOR_BUFFER_BIT).
 */
struct gl_colorbuffer_attrib
{
   GLuint ClearIndex;                      /**< Index for glClear */
   union gl_color_union ClearColor;        /**< Color for glClear, unclamped */
   GLuint IndexMask;                       /**< Color index write mask */
   GLubyte ColorMask[MAX_DRAW_BUFFERS][4]; /**< Each flag is 0xff or 0x0 */

   GLenum DrawBuffer[MAX_DRAW_BUFFERS];	/**< Which buffer to draw into */

   /** 
    * \name alpha testing
    */
   /*@{*/
   GLboolean AlphaEnabled;		/**< Alpha test enabled flag */
   GLenum AlphaFunc;			/**< Alpha test function */
   GLfloat AlphaRefUnclamped;
   GLclampf AlphaRef;			/**< Alpha reference value */
   /*@}*/

   /** 
    * \name Blending
    */
   /*@{*/
   GLbitfield BlendEnabled;		/**< Per-buffer blend enable flags */

   /* NOTE: this does _not_ depend on fragment clamping or any other clamping
    * control, only on the fixed-pointness of the render target.
    * The query does however depend on fragment color clamping.
    */
   GLfloat BlendColorUnclamped[4];               /**< Blending color */
   GLfloat BlendColor[4];		/**< Blending color */

   struct
   {
      GLenum SrcRGB;             /**< RGB blend source term */
      GLenum DstRGB;             /**< RGB blend dest term */
      GLenum SrcA;               /**< Alpha blend source term */
      GLenum DstA;               /**< Alpha blend dest term */
      GLenum EquationRGB;        /**< GL_ADD, GL_SUBTRACT, etc. */
      GLenum EquationA;          /**< GL_ADD, GL_SUBTRACT, etc. */
      /**
       * Set if any blend factor uses SRC1.  Computed at the time blend factors
       * get set.
       */
      GLboolean _UsesDualSrc;
   } Blend[MAX_DRAW_BUFFERS];
   /** Are the blend func terms currently different for each buffer/target? */
   GLboolean _BlendFuncPerBuffer;
   /** Are the blend equations currently different for each buffer/target? */
   GLboolean _BlendEquationPerBuffer;
   /*@}*/

   /** 
    * \name Logic op
    */
   /*@{*/
   GLenum LogicOp;			/**< Logic operator */
   GLboolean IndexLogicOpEnabled;	/**< Color index logic op enabled flag */
   GLboolean ColorLogicOpEnabled;	/**< RGBA logic op enabled flag */
   /*@}*/

   GLboolean DitherFlag;		/**< Dither enable flag */

   GLenum ClampFragmentColor; /**< GL_TRUE, GL_FALSE or GL_FIXED_ONLY_ARB */
   GLboolean _ClampFragmentColor; /** < with GL_FIXED_ONLY_ARB resolved */
   GLenum ClampReadColor;     /**< GL_TRUE, GL_FALSE or GL_FIXED_ONLY_ARB */
   GLboolean _ClampReadColor;     /** < with GL_FIXED_ONLY_ARB resolved */

   GLboolean sRGBEnabled;	/**< Framebuffer sRGB blending/updating requested */
};


/**
 * Current attribute group (GL_CURRENT_BIT).
 */
struct gl_current_attrib
{
   /**
    * \name Current vertex attributes.
    * \note Values are valid only after FLUSH_VERTICES has been called.
    * \note Index and Edgeflag current values are stored as floats in the 
    * SIX and SEVEN attribute slots.
    */
   GLfloat Attrib[VERT_ATTRIB_MAX][4];	/**< Position, color, texcoords, etc */

   /**
    * \name Current raster position attributes (always valid).
    * \note This set of attributes is very similar to the SWvertex struct.
    */
   /*@{*/
   GLfloat RasterPos[4];
   GLfloat RasterDistance;
   GLfloat RasterColor[4];
   GLfloat RasterSecondaryColor[4];
   GLfloat RasterTexCoords[MAX_TEXTURE_COORD_UNITS][4];
   GLboolean RasterPosValid;
   /*@}*/
};


/**
 * Depth buffer attribute group (GL_DEPTH_BUFFER_BIT).
 */
struct gl_depthbuffer_attrib
{
   GLenum Func;			/**< Function for depth buffer compare */
   GLclampd Clear;		/**< Value to clear depth buffer to */
   GLboolean Test;		/**< Depth buffering enabled flag */
   GLboolean Mask;		/**< Depth buffer writable? */
   GLboolean BoundsTest;        /**< GL_EXT_depth_bounds_test */
   GLfloat BoundsMin, BoundsMax;/**< GL_EXT_depth_bounds_test */
};


/**
 * Evaluator attribute group (GL_EVAL_BIT).
 */
struct gl_eval_attrib
{
   /**
    * \name Enable bits 
    */
   /*@{*/
   GLboolean Map1Color4;
   GLboolean Map1Index;
   GLboolean Map1Normal;
   GLboolean Map1TextureCoord1;
   GLboolean Map1TextureCoord2;
   GLboolean Map1TextureCoord3;
   GLboolean Map1TextureCoord4;
   GLboolean Map1Vertex3;
   GLboolean Map1Vertex4;
   GLboolean Map1Attrib[16];  /* GL_NV_vertex_program */
   GLboolean Map2Color4;
   GLboolean Map2Index;
   GLboolean Map2Normal;
   GLboolean Map2TextureCoord1;
   GLboolean Map2TextureCoord2;
   GLboolean Map2TextureCoord3;
   GLboolean Map2TextureCoord4;
   GLboolean Map2Vertex3;
   GLboolean Map2Vertex4;
   GLboolean Map2Attrib[16];  /* GL_NV_vertex_program */
   GLboolean AutoNormal;
   /*@}*/
   
   /**
    * \name Map Grid endpoints and divisions and calculated du values
    */
   /*@{*/
   GLint MapGrid1un;
   GLfloat MapGrid1u1, MapGrid1u2, MapGrid1du;
   GLint MapGrid2un, MapGrid2vn;
   GLfloat MapGrid2u1, MapGrid2u2, MapGrid2du;
   GLfloat MapGrid2v1, MapGrid2v2, MapGrid2dv;
   /*@}*/
};


/**
 * Fog attribute group (GL_FOG_BIT).
 */
struct gl_fog_attrib
{
   GLboolean Enabled;		/**< Fog enabled flag */
   GLfloat ColorUnclamped[4];            /**< Fog color */
   GLfloat Color[4];		/**< Fog color */
   GLfloat Density;		/**< Density >= 0.0 */
   GLfloat Start;		/**< Start distance in eye coords */
   GLfloat End;			/**< End distance in eye coords */
   GLfloat Index;		/**< Fog index */
   GLenum Mode;			/**< Fog mode */
   GLboolean ColorSumEnabled;
   GLenum FogCoordinateSource;  /**< GL_EXT_fog_coord */
   GLfloat _Scale;		/**< (End == Start) ? 1.0 : 1.0 / (End - Start) */
   GLenum FogDistanceMode;     /**< GL_NV_fog_distance */
};


/** 
 * Hint attribute group (GL_HINT_BIT).
 * 
 * Values are always one of GL_FASTEST, GL_NICEST, or GL_DONT_CARE.
 */
struct gl_hint_attrib
{
   GLenum PerspectiveCorrection;
   GLenum PointSmooth;
   GLenum LineSmooth;
   GLenum PolygonSmooth;
   GLenum Fog;
   GLenum ClipVolumeClipping;   /**< GL_EXT_clip_volume_hint */
   GLenum TextureCompression;   /**< GL_ARB_texture_compression */
   GLenum GenerateMipmap;       /**< GL_SGIS_generate_mipmap */
   GLenum FragmentShaderDerivative; /**< GL_ARB_fragment_shader */
};


/**
 * Lighting attribute group (GL_LIGHT_BIT).
 */
struct gl_light_attrib
{
   struct gl_light Light[MAX_LIGHTS];	/**< Array of light sources */
   struct gl_lightmodel Model;		/**< Lighting model */

   /**
    * Front and back material values.
    * Note: must call FLUSH_VERTICES() before using.
    */
   struct gl_material Material;

   GLboolean Enabled;			/**< Lighting enabled flag */
   GLenum ShadeModel;			/**< GL_FLAT or GL_SMOOTH */
   GLenum ProvokingVertex;              /**< GL_EXT_provoking_vertex */
   GLenum ColorMaterialFace;		/**< GL_FRONT, BACK or FRONT_AND_BACK */
   GLenum ColorMaterialMode;		/**< GL_AMBIENT, GL_DIFFUSE, etc */
   GLbitfield _ColorMaterialBitmask;	/**< bitmask formed from Face and Mode */
   GLboolean ColorMaterialEnabled;
   GLenum ClampVertexColor;             /**< GL_TRUE, GL_FALSE, GL_FIXED_ONLY */
   GLboolean _ClampVertexColor;

   struct gl_light EnabledList;         /**< List sentinel */

   /** 
    * Derived state for optimizations: 
    */
   /*@{*/
   GLboolean _NeedEyeCoords;		
   GLboolean _NeedVertices;		/**< Use fast shader? */
   GLfloat _BaseColor[2][3];
   /*@}*/
};


/**
 * Line attribute group (GL_LINE_BIT).
 */
struct gl_line_attrib
{
   GLboolean SmoothFlag;	/**< GL_LINE_SMOOTH enabled? */
   GLboolean StippleFlag;	/**< GL_LINE_STIPPLE enabled? */
   GLushort StipplePattern;	/**< Stipple pattern */
   GLint StippleFactor;		/**< Stipple repeat factor */
   GLfloat Width;		/**< Line width */
};


/**
 * Display list attribute group (GL_LIST_BIT).
 */
struct gl_list_attrib
{
   GLuint ListBase;
};


/**
 * Multisample attribute group (GL_MULTISAMPLE_BIT).
 */
struct gl_multisample_attrib
{
   GLboolean Enabled;
   GLboolean _Enabled;   /**< true if Enabled and multisample buffer */
   GLboolean SampleAlphaToCoverage;
   GLboolean SampleAlphaToOne;
   GLboolean SampleCoverage;
   GLfloat SampleCoverageValue;
   GLboolean SampleCoverageInvert;
};


/**
 * A pixelmap (see glPixelMap)
 */
struct gl_pixelmap
{
   GLint Size;
   GLfloat Map[MAX_PIXEL_MAP_TABLE];
};


/**
 * Collection of all pixelmaps
 */
struct gl_pixelmaps
{
   struct gl_pixelmap RtoR;  /**< i.e. GL_PIXEL_MAP_R_TO_R */
   struct gl_pixelmap GtoG;
   struct gl_pixelmap BtoB;
   struct gl_pixelmap AtoA;
   struct gl_pixelmap ItoR;
   struct gl_pixelmap ItoG;
   struct gl_pixelmap ItoB;
   struct gl_pixelmap ItoA;
   struct gl_pixelmap ItoI;
   struct gl_pixelmap StoS;
};


/**
 * Pixel attribute group (GL_PIXEL_MODE_BIT).
 */
struct gl_pixel_attrib
{
   GLenum ReadBuffer;		/**< source buffer for glRead/CopyPixels() */

   /*--- Begin Pixel Transfer State ---*/
   /* Fields are in the order in which they're applied... */

   /** Scale & Bias (index shift, offset) */
   /*@{*/
   GLfloat RedBias, RedScale;
   GLfloat GreenBias, GreenScale;
   GLfloat BlueBias, BlueScale;
   GLfloat AlphaBias, AlphaScale;
   GLfloat DepthBias, DepthScale;
   GLint IndexShift, IndexOffset;
   /*@}*/

   /* Pixel Maps */
   /* Note: actual pixel maps are not part of this attrib group */
   GLboolean MapColorFlag;
   GLboolean MapStencilFlag;

   /*--- End Pixel Transfer State ---*/

   /** glPixelZoom */
   GLfloat ZoomX, ZoomY;
};


/**
 * Point attribute group (GL_POINT_BIT).
 */
struct gl_point_attrib
{
   GLboolean SmoothFlag;	/**< True if GL_POINT_SMOOTH is enabled */
   GLfloat Size;		/**< User-specified point size */
   GLfloat Params[3];		/**< GL_EXT_point_parameters */
   GLfloat MinSize, MaxSize;	/**< GL_EXT_point_parameters */
   GLfloat Threshold;		/**< GL_EXT_point_parameters */
   GLboolean _Attenuated;	/**< True if Params != [1, 0, 0] */
   GLboolean PointSprite;	/**< GL_NV/ARB_point_sprite */
   GLboolean CoordReplace[MAX_TEXTURE_COORD_UNITS]; /**< GL_ARB_point_sprite*/
   GLenum SpriteRMode;		/**< GL_NV_point_sprite (only!) */
   GLenum SpriteOrigin;		/**< GL_ARB_point_sprite */
};


/**
 * Polygon attribute group (GL_POLYGON_BIT).
 */
struct gl_polygon_attrib
{
   GLenum FrontFace;		/**< Either GL_CW or GL_CCW */
   GLenum FrontMode;		/**< Either GL_POINT, GL_LINE or GL_FILL */
   GLenum BackMode;		/**< Either GL_POINT, GL_LINE or GL_FILL */
   GLboolean _FrontBit;		/**< 0=GL_CCW, 1=GL_CW */
   GLboolean CullFlag;		/**< Culling on/off flag */
   GLboolean SmoothFlag;	/**< True if GL_POLYGON_SMOOTH is enabled */
   GLboolean StippleFlag;	/**< True if GL_POLYGON_STIPPLE is enabled */
   GLenum CullFaceMode;		/**< Culling mode GL_FRONT or GL_BACK */
   GLfloat OffsetFactor;	/**< Polygon offset factor, from user */
   GLfloat OffsetUnits;		/**< Polygon offset units, from user */
   GLboolean OffsetPoint;	/**< Offset in GL_POINT mode */
   GLboolean OffsetLine;	/**< Offset in GL_LINE mode */
   GLboolean OffsetFill;	/**< Offset in GL_FILL mode */
};


/**
 * Scissor attributes (GL_SCISSOR_BIT).
 */
struct gl_scissor_attrib
{
   GLboolean Enabled;		/**< Scissor test enabled? */
   GLint X, Y;			/**< Lower left corner of box */
   GLsizei Width, Height;	/**< Size of box */
};


/**
 * Stencil attribute group (GL_STENCIL_BUFFER_BIT).
 *
 * Three sets of stencil data are tracked so that OpenGL 2.0,
 * GL_EXT_stencil_two_side, and GL_ATI_separate_stencil can all be supported
 * simultaneously.  In each of the stencil state arrays, element 0 corresponds
 * to GL_FRONT.  Element 1 corresponds to the OpenGL 2.0 /
 * GL_ATI_separate_stencil GL_BACK state.  Element 2 corresponds to the
 * GL_EXT_stencil_two_side GL_BACK state.
 *
 * The derived value \c _BackFace is either 1 or 2 depending on whether or
 * not GL_STENCIL_TEST_TWO_SIDE_EXT is enabled.
 *
 * The derived value \c _TestTwoSide is set when the front-face and back-face
 * stencil state are different.
 */
struct gl_stencil_attrib
{
   GLboolean Enabled;		/**< Enabled flag */
   GLboolean TestTwoSide;	/**< GL_EXT_stencil_two_side */
   GLubyte ActiveFace;		/**< GL_EXT_stencil_two_side (0 or 2) */
   GLboolean _Enabled;          /**< Enabled and stencil buffer present */
   GLboolean _TestTwoSide;
   GLubyte _BackFace;           /**< Current back stencil state (1 or 2) */
   GLenum Function[3];		/**< Stencil function */
   GLenum FailFunc[3];		/**< Fail function */
   GLenum ZPassFunc[3];		/**< Depth buffer pass function */
   GLenum ZFailFunc[3];		/**< Depth buffer fail function */
   GLint Ref[3];		/**< Reference value */
   GLuint ValueMask[3];		/**< Value mask */
   GLuint WriteMask[3];		/**< Write mask */
   GLuint Clear;		/**< Clear value */
};


/**
 * An index for each type of texture object.  These correspond to the GL
 * texture target enums, such as GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP, etc.
 * Note: the order is from highest priority to lowest priority.
 */
typedef enum
{
   TEXTURE_BUFFER_INDEX,
   TEXTURE_2D_ARRAY_INDEX,
   TEXTURE_1D_ARRAY_INDEX,
   TEXTURE_EXTERNAL_INDEX,
   TEXTURE_CUBE_INDEX,
   TEXTURE_3D_INDEX,
   TEXTURE_RECT_INDEX,
   TEXTURE_2D_INDEX,
   TEXTURE_1D_INDEX,
   NUM_TEXTURE_TARGETS
} gl_texture_index;


/**
 * Bit flags for each type of texture object
 * Used for Texture.Unit[]._ReallyEnabled flags.
 */
/*@{*/
#define TEXTURE_BUFFER_BIT   (1 << TEXTURE_BUFFER_INDEX)
#define TEXTURE_2D_ARRAY_BIT (1 << TEXTURE_2D_ARRAY_INDEX)
#define TEXTURE_1D_ARRAY_BIT (1 << TEXTURE_1D_ARRAY_INDEX)
#define TEXTURE_EXTERNAL_BIT (1 << TEXTURE_EXTERNAL_INDEX)
#define TEXTURE_CUBE_BIT     (1 << TEXTURE_CUBE_INDEX)
#define TEXTURE_3D_BIT       (1 << TEXTURE_3D_INDEX)
#define TEXTURE_RECT_BIT     (1 << TEXTURE_RECT_INDEX)
#define TEXTURE_2D_BIT       (1 << TEXTURE_2D_INDEX)
#define TEXTURE_1D_BIT       (1 << TEXTURE_1D_INDEX)
/*@}*/


/**
 * Texture image state.  Drivers will typically create a subclass of this
 * with extra fields for memory buffers, etc.
 */
struct gl_texture_image
{
   GLint InternalFormat;	/**< Internal format as given by the user */
   GLenum _BaseFormat;		/**< Either GL_RGB, GL_RGBA, GL_ALPHA,
				 *   GL_LUMINANCE, GL_LUMINANCE_ALPHA,
				 *   GL_INTENSITY, GL_DEPTH_COMPONENT or
				 *   GL_DEPTH_STENCIL_EXT only. Used for
				 *   choosing TexEnv arithmetic.
				 */
   gl_format TexFormat;         /**< The actual texture memory format */

   GLuint Border;		/**< 0 or 1 */
   GLuint Width;		/**< = 2^WidthLog2 + 2*Border */
   GLuint Height;		/**< = 2^HeightLog2 + 2*Border */
   GLuint Depth;		/**< = 2^DepthLog2 + 2*Border */
   GLuint Width2;		/**< = Width - 2*Border */
   GLuint Height2;		/**< = Height - 2*Border */
   GLuint Depth2;		/**< = Depth - 2*Border */
   GLuint WidthLog2;		/**< = log2(Width2) */
   GLuint HeightLog2;		/**< = log2(Height2) */
   GLuint DepthLog2;		/**< = log2(Depth2) */
   GLuint MaxLog2;		/**< = MAX(WidthLog2, HeightLog2) */

   struct gl_texture_object *TexObject;  /**< Pointer back to parent object */
   GLuint Level;                /**< Which mipmap level am I? */
   /** Cube map face: index into gl_texture_object::Image[] array */
   GLuint Face;
};


/**
 * Indexes for cube map faces.
 */
typedef enum
{
   FACE_POS_X = 0,
   FACE_NEG_X = 1,
   FACE_POS_Y = 2,
   FACE_NEG_Y = 3,
   FACE_POS_Z = 4,
   FACE_NEG_Z = 5,
   MAX_FACES = 6
} gl_face_index;


/**
 * Sampler object state.  These objects are new with GL_ARB_sampler_objects
 * and OpenGL 3.3.  Legacy texture objects also contain a sampler object.
 */
struct gl_sampler_object
{
   GLuint Name;
   GLint RefCount;

   GLenum WrapS;		/**< S-axis texture image wrap mode */
   GLenum WrapT;		/**< T-axis texture image wrap mode */
   GLenum WrapR;		/**< R-axis texture image wrap mode */
   GLenum MinFilter;		/**< minification filter */
   GLenum MagFilter;		/**< magnification filter */
   union gl_color_union BorderColor;  /**< Interpreted according to texture format */
   GLfloat MinLod;		/**< min lambda, OpenGL 1.2 */
   GLfloat MaxLod;		/**< max lambda, OpenGL 1.2 */
   GLfloat LodBias;		/**< OpenGL 1.4 */
   GLfloat MaxAnisotropy;	/**< GL_EXT_texture_filter_anisotropic */
   GLenum CompareMode;		/**< GL_ARB_shadow */
   GLenum CompareFunc;		/**< GL_ARB_shadow */
   GLenum sRGBDecode;           /**< GL_DECODE_EXT or GL_SKIP_DECODE_EXT */
   GLboolean CubeMapSeamless;   /**< GL_AMD_seamless_cubemap_per_texture */
};


/**
 * Texture object state.  Contains the array of mipmap images, border color,
 * wrap modes, filter modes, and shadow/texcompare state.
 */
struct gl_texture_object
{
   _glthread_Mutex Mutex;	/**< for thread safety */
   GLint RefCount;		/**< reference count */
   GLuint Name;			/**< the user-visible texture object ID */
   GLenum Target;               /**< GL_TEXTURE_1D, GL_TEXTURE_2D, etc. */

   struct gl_sampler_object Sampler;

   GLenum DepthMode;		/**< GL_ARB_depth_texture */

   GLfloat Priority;		/**< in [0,1] */
   GLint BaseLevel;		/**< min mipmap level, OpenGL 1.2 */
   GLint MaxLevel;		/**< max mipmap level, OpenGL 1.2 */
   GLint _MaxLevel;		/**< actual max mipmap level (q in the spec) */
   GLfloat _MaxLambda;		/**< = _MaxLevel - BaseLevel (q - b in spec) */
   GLint CropRect[4];           /**< GL_OES_draw_texture */
   GLenum Swizzle[4];           /**< GL_EXT_texture_swizzle */
   GLuint _Swizzle;             /**< same as Swizzle, but SWIZZLE_* format */
   GLboolean GenerateMipmap;    /**< GL_SGIS_generate_mipmap */
   GLboolean _BaseComplete;     /**< Is the base texture level valid? */
   GLboolean _MipmapComplete;   /**< Is the whole mipmap valid? */
   GLboolean _IsIntegerFormat;  /**< Does the texture store integer values? */
   GLboolean _RenderToTexture;  /**< Any rendering to this texture? */
   GLboolean Purgeable;         /**< Is the buffer purgeable under memory pressure? */
   GLboolean Immutable;         /**< GL_ARB_texture_storage */

   /** Actual texture images, indexed by [cube face] and [mipmap level] */
   struct gl_texture_image *Image[MAX_FACES][MAX_TEXTURE_LEVELS];

   /** GL_ARB_texture_buffer_object */
   struct gl_buffer_object *BufferObject;
   GLenum BufferObjectFormat;
   /** Equivalent Mesa format for BufferObjectFormat. */
   gl_format _BufferObjectFormat;

   /** GL_OES_EGL_image_external */
   GLint RequiredTextureImageUnits;
};


/** Up to four combiner sources are possible with GL_NV_texture_env_combine4 */
#define MAX_COMBINER_TERMS 4


/**
 * Texture combine environment state.
 */
struct gl_tex_env_combine_state
{
   GLenum ModeRGB;       /**< GL_REPLACE, GL_DECAL, GL_ADD, etc. */
   GLenum ModeA;         /**< GL_REPLACE, GL_DECAL, GL_ADD, etc. */
   /** Source terms: GL_PRIMARY_COLOR, GL_TEXTURE, etc */
   GLenum SourceRGB[MAX_COMBINER_TERMS];
   GLenum SourceA[MAX_COMBINER_TERMS];
   /** Source operands: GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, etc */
   GLenum OperandRGB[MAX_COMBINER_TERMS];
   GLenum OperandA[MAX_COMBINER_TERMS];
   GLuint ScaleShiftRGB; /**< 0, 1 or 2 */
   GLuint ScaleShiftA;   /**< 0, 1 or 2 */
   GLuint _NumArgsRGB;   /**< Number of inputs used for the RGB combiner */
   GLuint _NumArgsA;     /**< Number of inputs used for the A combiner */
};


/**
 * TexGenEnabled flags.
 */
/*@{*/
#define S_BIT 1
#define T_BIT 2
#define R_BIT 4
#define Q_BIT 8
#define STR_BITS (S_BIT | T_BIT | R_BIT)
/*@}*/


/**
 * Bit flag versions of the corresponding GL_ constants.
 */
/*@{*/
#define TEXGEN_SPHERE_MAP        0x1
#define TEXGEN_OBJ_LINEAR        0x2
#define TEXGEN_EYE_LINEAR        0x4
#define TEXGEN_REFLECTION_MAP_NV 0x8
#define TEXGEN_NORMAL_MAP_NV     0x10

#define TEXGEN_NEED_NORMALS      (TEXGEN_SPHERE_MAP        | \
				  TEXGEN_REFLECTION_MAP_NV | \
				  TEXGEN_NORMAL_MAP_NV)
#define TEXGEN_NEED_EYE_COORD    (TEXGEN_SPHERE_MAP        | \
				  TEXGEN_REFLECTION_MAP_NV | \
				  TEXGEN_NORMAL_MAP_NV     | \
				  TEXGEN_EYE_LINEAR)
/*@}*/



/** Tex-gen enabled for texture unit? */
#define ENABLE_TEXGEN(unit) (1 << (unit))

/** Non-identity texture matrix for texture unit? */
#define ENABLE_TEXMAT(unit) (1 << (unit))


/**
 * Texture coord generation state.
 */
struct gl_texgen
{
   GLenum Mode;         /**< GL_EYE_LINEAR, GL_SPHERE_MAP, etc */
   GLbitfield _ModeBit; /**< TEXGEN_x bit corresponding to Mode */
   GLfloat ObjectPlane[4];
   GLfloat EyePlane[4];
};


/**
 * Texture unit state.  Contains enable flags, texture environment/function/
 * combiners, texgen state, and pointers to current texture objects.
 */
struct gl_texture_unit
{
   GLbitfield Enabled;          /**< bitmask of TEXTURE_*_BIT flags */
   GLbitfield _ReallyEnabled;   /**< 0 or exactly one of TEXTURE_*_BIT flags */

   GLenum EnvMode;              /**< GL_MODULATE, GL_DECAL, GL_BLEND, etc. */
   GLclampf EnvColor[4];
   GLfloat EnvColorUnclamped[4];

   struct gl_texgen GenS;
   struct gl_texgen GenT;
   struct gl_texgen GenR;
   struct gl_texgen GenQ;
   GLbitfield TexGenEnabled;	/**< Bitwise-OR of [STRQ]_BIT values */
   GLbitfield _GenFlags;	/**< Bitwise-OR of Gen[STRQ]._ModeBit */

   GLfloat LodBias;		/**< for biasing mipmap levels */
   GLenum BumpTarget;
   GLfloat RotMatrix[4]; /* 2x2 matrix */

   /** Current sampler object (GL_ARB_sampler_objects) */
   struct gl_sampler_object *Sampler;

   /** 
    * \name GL_EXT_texture_env_combine 
    */
   struct gl_tex_env_combine_state Combine;

   /**
    * Derived state based on \c EnvMode and the \c BaseFormat of the
    * currently enabled texture.
    */
   struct gl_tex_env_combine_state _EnvMode;

   /**
    * Currently enabled combiner state.  This will point to either
    * \c Combine or \c _EnvMode.
    */
   struct gl_tex_env_combine_state *_CurrentCombine;

   /** Current texture object pointers */
   struct gl_texture_object *CurrentTex[NUM_TEXTURE_TARGETS];

   /** Points to highest priority, complete and enabled texture object */
   struct gl_texture_object *_Current;
};


/**
 * Texture attribute group (GL_TEXTURE_BIT).
 */
struct gl_texture_attrib
{
   GLuint CurrentUnit;   /**< GL_ACTIVE_TEXTURE */
   struct gl_texture_unit Unit[MAX_COMBINED_TEXTURE_IMAGE_UNITS];

   struct gl_texture_object *ProxyTex[NUM_TEXTURE_TARGETS];

   /** GL_ARB_texture_buffer_object */
   struct gl_buffer_object *BufferObject;

   /** GL_ARB_seamless_cubemap */
   GLboolean CubeMapSeamless;

   /** Texture units/samplers used by vertex or fragment texturing */
   GLbitfield _EnabledUnits;

   /** Texture coord units/sets used for fragment texturing */
   GLbitfield _EnabledCoordUnits;

   /** Texture coord units that have texgen enabled */
   GLbitfield _TexGenEnabled;

   /** Texture coord units that have non-identity matrices */
   GLbitfield _TexMatEnabled;

   /** Bitwise-OR of all Texture.Unit[i]._GenFlags */
   GLbitfield _GenFlags;
};


/**
 * Data structure representing a single clip plane (e.g. one of the elements
 * of the ctx->Transform.EyeUserPlane or ctx->Transform._ClipUserPlane array).
 */
typedef GLfloat gl_clip_plane[4];


/**
 * Transformation attribute group (GL_TRANSFORM_BIT).
 */
struct gl_transform_attrib
{
   GLenum MatrixMode;				/**< Matrix mode */
   gl_clip_plane EyeUserPlane[MAX_CLIP_PLANES];	/**< User clip planes */
   gl_clip_plane _ClipUserPlane[MAX_CLIP_PLANES]; /**< derived */
   GLbitfield ClipPlanesEnabled;                /**< on/off bitmask */
   GLboolean Normalize;				/**< Normalize all normals? */
   GLboolean RescaleNormals;			/**< GL_EXT_rescale_normal */
   GLboolean RasterPositionUnclipped;           /**< GL_IBM_rasterpos_clip */
   GLboolean DepthClamp;			/**< GL_ARB_depth_clamp */

   GLfloat CullEyePos[4];
   GLfloat CullObjPos[4];
};


/**
 * Viewport attribute group (GL_VIEWPORT_BIT).
 */
struct gl_viewport_attrib
{
   GLint X, Y;			/**< position */
   GLsizei Width, Height;	/**< size */
   GLfloat Near, Far;		/**< Depth buffer range */
   GLmatrix _WindowMap;		/**< Mapping transformation as a matrix. */
};


/**
 * GL_ARB_vertex/pixel_buffer_object buffer object
 */
struct gl_buffer_object
{
   _glthread_Mutex Mutex;
   GLint RefCount;
   GLuint Name;
   GLenum Usage;        /**< GL_STREAM_DRAW_ARB, GL_STREAM_READ_ARB, etc. */
   GLsizeiptrARB Size;  /**< Size of buffer storage in bytes */
   GLubyte *Data;       /**< Location of storage either in RAM or VRAM. */
   /** Fields describing a mapped buffer */
   /*@{*/
   GLbitfield AccessFlags; /**< Mask of GL_MAP_x_BIT flags */
   GLvoid *Pointer;     /**< User-space address of mapping */
   GLintptr Offset;     /**< Mapped offset */
   GLsizeiptr Length;   /**< Mapped length */
   /*@}*/
   GLboolean DeletePending;   /**< true if buffer object is removed from the hash */
   GLboolean Written;   /**< Ever written to? (for debugging) */
   GLboolean Purgeable; /**< Is the buffer purgeable under memory pressure? */
};


/**
 * Client pixel packing/unpacking attributes
 */
struct gl_pixelstore_attrib
{
   GLint Alignment;
   GLint RowLength;
   GLint SkipPixels;
   GLint SkipRows;
   GLint ImageHeight;
   GLint SkipImages;
   GLboolean SwapBytes;
   GLboolean LsbFirst;
   GLboolean Invert;        /**< GL_MESA_pack_invert */
   struct gl_buffer_object *BufferObj; /**< GL_ARB_pixel_buffer_object */
};


/**
 * Client vertex array attributes
 */
struct gl_client_array
{
   GLint Size;                  /**< components per element (1,2,3,4) */
   GLenum Type;                 /**< datatype: GL_FLOAT, GL_INT, etc */
   GLenum Format;               /**< default: GL_RGBA, but may be GL_BGRA */
   GLsizei Stride;		/**< user-specified stride */
   GLsizei StrideB;		/**< actual stride in bytes */
   const GLubyte *Ptr;          /**< Points to array data */
   GLboolean Enabled;		/**< Enabled flag is a boolean */
   GLboolean Normalized;        /**< GL_ARB_vertex_program */
   GLboolean Integer;           /**< Integer-valued? */
   GLuint InstanceDivisor;      /**< GL_ARB_instanced_arrays */
   GLuint _ElementSize;         /**< size of each element in bytes */

   struct gl_buffer_object *BufferObj;/**< GL_ARB_vertex_buffer_object */
   GLuint _MaxElement;          /**< max element index into array buffer + 1 */
};


/**
 * Collection of vertex arrays.  Defined by the GL_APPLE_vertex_array_object
 * extension, but a nice encapsulation in any case.
 */
struct gl_array_object
{
   /** Name of the array object as received from glGenVertexArrayAPPLE. */
   GLuint Name;

   GLint RefCount;
   _glthread_Mutex Mutex;

   /**
    * Does the VAO use ARB semantics or Apple semantics?
    *
    * There are several ways in which ARB_vertex_array_object and
    * APPLE_vertex_array_object VAOs have differing semantics.  At the very
    * least,
    *
    *     - ARB VAOs require that all array data be sourced from vertex buffer
    *       objects, but Apple VAOs do not.
    *
    *     - ARB VAOs require that names come from GenVertexArrays.
    *
    * This flag notes which behavior governs this VAO.
    */
   GLboolean ARBsemantics;

   /**
    * Has this array object been bound?
    */
   GLboolean _Used;

   /** Vertex attribute arrays */
   struct gl_client_array VertexAttrib[VERT_ATTRIB_MAX];

   /** Mask of VERT_BIT_* values indicating which arrays are enabled */
   GLbitfield64 _Enabled;

   /** Mask of VERT_BIT_* values indicating changed/dirty arrays */
   GLbitfield64 NewArrays;

   /**
    * Min of all enabled arrays' _MaxElement.  When arrays reside inside VBOs
    * we can determine the max legal (in bounds) glDrawElements array index.
    */
   GLuint _MaxElement;

   struct gl_buffer_object *ElementArrayBufferObj;
};


/**
 * Vertex array state
 */
struct gl_array_attrib
{
   /** Currently bound array object. See _mesa_BindVertexArrayAPPLE() */
   struct gl_array_object *ArrayObj;

   /** The default vertex array object */
   struct gl_array_object *DefaultArrayObj;

   /** Array objects (GL_ARB/APPLE_vertex_array_object) */
   struct _mesa_HashTable *Objects;

   GLint ActiveTexture;		/**< Client Active Texture */
   GLuint LockFirst;            /**< GL_EXT_compiled_vertex_array */
   GLuint LockCount;            /**< GL_EXT_compiled_vertex_array */

   /** GL 3.1 (slightly different from GL_NV_primitive_restart) */
   GLboolean PrimitiveRestart;
   GLuint RestartIndex;

   /* GL_ARB_vertex_buffer_object */
   struct gl_buffer_object *ArrayBufferObj;

   /**
    * Vertex arrays as consumed by a driver.
    * The array pointer is set up only by the VBO module. */
   const struct gl_client_array **_DrawArrays; /**< 0..VERT_ATTRIB_MAX-1 */
};


/**
 * Feedback buffer state
 */
struct gl_feedback
{
   GLenum Type;
   GLbitfield _Mask;    /**< FB_* bits */
   GLfloat *Buffer;
   GLuint BufferSize;
   GLuint Count;
};


/**
 * Selection buffer state
 */
struct gl_selection
{
   GLuint *Buffer;	/**< selection buffer */
   GLuint BufferSize;	/**< size of the selection buffer */
   GLuint BufferCount;	/**< number of values in the selection buffer */
   GLuint Hits;		/**< number of records in the selection buffer */
   GLuint NameStackDepth; /**< name stack depth */
   GLuint NameStack[MAX_NAME_STACK_DEPTH]; /**< name stack */
   GLboolean HitFlag;	/**< hit flag */
   GLfloat HitMinZ;	/**< minimum hit depth */
   GLfloat HitMaxZ;	/**< maximum hit depth */
};


/**
 * 1-D Evaluator control points
 */
struct gl_1d_map
{
   GLuint Order;	/**< Number of control points */
   GLfloat u1, u2, du;	/**< u1, u2, 1.0/(u2-u1) */
   GLfloat *Points;	/**< Points to contiguous control points */
};


/**
 * 2-D Evaluator control points
 */
struct gl_2d_map
{
   GLuint Uorder;		/**< Number of control points in U dimension */
   GLuint Vorder;		/**< Number of control points in V dimension */
   GLfloat u1, u2, du;
   GLfloat v1, v2, dv;
   GLfloat *Points;		/**< Points to contiguous control points */
};


/**
 * All evaluator control point state
 */
struct gl_evaluators
{
   /** 
    * \name 1-D maps
    */
   /*@{*/
   struct gl_1d_map Map1Vertex3;
   struct gl_1d_map Map1Vertex4;
   struct gl_1d_map Map1Index;
   struct gl_1d_map Map1Color4;
   struct gl_1d_map Map1Normal;
   struct gl_1d_map Map1Texture1;
   struct gl_1d_map Map1Texture2;
   struct gl_1d_map Map1Texture3;
   struct gl_1d_map Map1Texture4;
   struct gl_1d_map Map1Attrib[16];  /**< GL_NV_vertex_program */
   /*@}*/

   /** 
    * \name 2-D maps 
    */
   /*@{*/
   struct gl_2d_map Map2Vertex3;
   struct gl_2d_map Map2Vertex4;
   struct gl_2d_map Map2Index;
   struct gl_2d_map Map2Color4;
   struct gl_2d_map Map2Normal;
   struct gl_2d_map Map2Texture1;
   struct gl_2d_map Map2Texture2;
   struct gl_2d_map Map2Texture3;
   struct gl_2d_map Map2Texture4;
   struct gl_2d_map Map2Attrib[16];  /**< GL_NV_vertex_program */
   /*@}*/
};


struct gl_transform_feedback_varying_info
{
   char *Name;
   GLenum Type;
   GLint Size;
};


/**
 * Per-output info vertex shaders for transform feedback.
 */
struct gl_transform_feedback_output
{
   unsigned OutputRegister;
   unsigned OutputBuffer;
   unsigned NumComponents;

   /** offset (in DWORDs) of this output within the interleaved structure */
   unsigned DstOffset;

   /**
    * Offset into the output register of the data to output.  For example,
    * if NumComponents is 2 and ComponentOffset is 1, then the data to
    * offset is in the y and z components of the output register.
    */
   unsigned ComponentOffset;
};


/** Post-link transform feedback info. */
struct gl_transform_feedback_info
{
   unsigned NumOutputs;

   /**
    * Number of transform feedback buffers in use by this program.
    */
   unsigned NumBuffers;

   struct gl_transform_feedback_output *Outputs;

   /** Transform feedback varyings used for the linking of this shader program.
    *
    * Use for glGetTransformFeedbackVarying().
    */
   struct gl_transform_feedback_varying_info *Varyings;
   GLint NumVarying;

   /**
    * Total number of components stored in each buffer.  This may be used by
    * hardware back-ends to determine the correct stride when interleaving
    * multiple transform feedback outputs in the same buffer.
    */
   unsigned BufferStride[MAX_FEEDBACK_BUFFERS];
};


/**
 * Transform feedback object state
 */
struct gl_transform_feedback_object
{
   GLuint Name;  /**< AKA the object ID */
   GLint RefCount;
   GLboolean Active;  /**< Is transform feedback enabled? */
   GLboolean Paused;  /**< Is transform feedback paused? */
   GLboolean EndedAnytime; /**< Has EndTransformFeedback been called
                                at least once? */

   /** The feedback buffers */
   GLuint BufferNames[MAX_FEEDBACK_BUFFERS];
   struct gl_buffer_object *Buffers[MAX_FEEDBACK_BUFFERS];

   /** Start of feedback data in dest buffer */
   GLintptr Offset[MAX_FEEDBACK_BUFFERS];
   /** Max data to put into dest buffer (in bytes) */
   GLsizeiptr Size[MAX_FEEDBACK_BUFFERS];
};


/**
 * Context state for transform feedback.
 */
struct gl_transform_feedback_state
{
   GLenum Mode;       /**< GL_POINTS, GL_LINES or GL_TRIANGLES */

   /** The general binding point (GL_TRANSFORM_FEEDBACK_BUFFER) */
   struct gl_buffer_object *CurrentBuffer;

   /** The table of all transform feedback objects */
   struct _mesa_HashTable *Objects;

   /** The current xform-fb object (GL_TRANSFORM_FEEDBACK_BINDING) */
   struct gl_transform_feedback_object *CurrentObject;

   /** The default xform-fb object (Name==0) */
   struct gl_transform_feedback_object *DefaultObject;
};


/**
 * Names of the various vertex/fragment program register files, etc.
 *
 * NOTE: first four tokens must fit into 2 bits (see t_vb_arbprogram.c)
 * All values should fit in a 4-bit field.
 *
 * NOTE: PROGRAM_ENV_PARAM, PROGRAM_STATE_VAR, PROGRAM_NAMED_PARAM,
 * PROGRAM_CONSTANT, and PROGRAM_UNIFORM can all be considered to
 * be "uniform" variables since they can only be set outside glBegin/End.
 * They're also all stored in the same Parameters array.
 */
typedef enum
{
   PROGRAM_TEMPORARY,   /**< machine->Temporary[] */
   PROGRAM_INPUT,       /**< machine->Inputs[] */
   PROGRAM_OUTPUT,      /**< machine->Outputs[] */
   PROGRAM_VARYING,     /**< machine->Inputs[]/Outputs[] */
   PROGRAM_LOCAL_PARAM, /**< gl_program->LocalParams[] */
   PROGRAM_ENV_PARAM,   /**< gl_program->Parameters[] */
   PROGRAM_STATE_VAR,   /**< gl_program->Parameters[] */
   PROGRAM_NAMED_PARAM, /**< gl_program->Parameters[] */
   PROGRAM_CONSTANT,    /**< gl_program->Parameters[] */
   PROGRAM_UNIFORM,     /**< gl_program->Parameters[] */
   PROGRAM_WRITE_ONLY,  /**< A dummy, write-only register */
   PROGRAM_ADDRESS,     /**< machine->AddressReg */
   PROGRAM_SAMPLER,     /**< for shader samplers, compile-time only */
   PROGRAM_SYSTEM_VALUE,/**< InstanceId, PrimitiveID, etc. */
   PROGRAM_UNDEFINED,   /**< Invalid/TBD value */
   PROGRAM_FILE_MAX
} gl_register_file;


/**
 * If the register file is PROGRAM_SYSTEM_VALUE, the register index will be
 * one of these values.
 */
typedef enum
{
   SYSTEM_VALUE_FRONT_FACE,  /**< Fragment shader only (not done yet) */
   SYSTEM_VALUE_VERTEX_ID,   /**< Vertex shader only */
   SYSTEM_VALUE_INSTANCE_ID, /**< Vertex shader only */
   SYSTEM_VALUE_MAX          /**< Number of values */
} gl_system_value;


/**
 * The possible interpolation qualifiers that can be applied to a fragment
 * shader input in GLSL.
 *
 * Note: INTERP_QUALIFIER_NONE must be 0 so that memsetting the
 * gl_fragment_program data structure to 0 causes the default behavior.
 */
enum glsl_interp_qualifier
{
   INTERP_QUALIFIER_NONE = 0,
   INTERP_QUALIFIER_SMOOTH,
   INTERP_QUALIFIER_FLAT,
   INTERP_QUALIFIER_NOPERSPECTIVE
};


/**
 * \brief Layout qualifiers for gl_FragDepth.
 *
 * Extension AMD_conservative_depth allows gl_FragDepth to be redeclared with
 * a layout qualifier.
 *
 * \see enum ir_depth_layout
 */
enum gl_frag_depth_layout
{
   FRAG_DEPTH_LAYOUT_NONE, /**< No layout is specified. */
   FRAG_DEPTH_LAYOUT_ANY,
   FRAG_DEPTH_LAYOUT_GREATER,
   FRAG_DEPTH_LAYOUT_LESS,
   FRAG_DEPTH_LAYOUT_UNCHANGED
};


/**
 * Base class for any kind of program object
 */
struct gl_program
{
   GLuint Id;
   GLubyte *String;  /**< Null-terminated program text */
   GLint RefCount;
   GLenum Target;    /**< GL_VERTEX/FRAGMENT_PROGRAM_ARB, GL_FRAGMENT_PROGRAM_NV */
   GLenum Format;    /**< String encoding format */
   GLboolean Resident;

   struct prog_instruction *Instructions;

   GLbitfield64 InputsRead;     /**< Bitmask of which input regs are read */
   GLbitfield64 OutputsWritten; /**< Bitmask of which output regs are written */
   GLbitfield SystemValuesRead;   /**< Bitmask of SYSTEM_VALUE_x inputs used */
   GLbitfield InputFlags[MAX_PROGRAM_INPUTS];   /**< PROG_PARAM_BIT_x flags */
   GLbitfield OutputFlags[MAX_PROGRAM_OUTPUTS]; /**< PROG_PARAM_BIT_x flags */
   GLbitfield TexturesUsed[MAX_COMBINED_TEXTURE_IMAGE_UNITS];  /**< TEXTURE_x_BIT bitmask */
   GLbitfield SamplersUsed;   /**< Bitfield of which samplers are used */
   GLbitfield ShadowSamplers; /**< Texture units used for shadow sampling. */


   /** Named parameters, constants, etc. from program text */
   struct gl_program_parameter_list *Parameters;
   /** Numbered local parameters */
   GLfloat LocalParams[MAX_PROGRAM_LOCAL_PARAMS][4];

   /** Map from sampler unit to texture unit (set by glUniform1i()) */
   GLubyte SamplerUnits[MAX_SAMPLERS];

   /** Bitmask of which register files are read/written with indirect
    * addressing.  Mask of (1 << PROGRAM_x) bits.
    */
   GLbitfield IndirectRegisterFiles;

   /** Logical counts */
   /*@{*/
   GLuint NumInstructions;
   GLuint NumTemporaries;
   GLuint NumParameters;
   GLuint NumAttributes;
   GLuint NumAddressRegs;
   GLuint NumAluInstructions;
   GLuint NumTexInstructions;
   GLuint NumTexIndirections;
   /*@}*/
   /** Native, actual h/w counts */
   /*@{*/
   GLuint NumNativeInstructions;
   GLuint NumNativeTemporaries;
   GLuint NumNativeParameters;
   GLuint NumNativeAttributes;
   GLuint NumNativeAddressRegs;
   GLuint NumNativeAluInstructions;
   GLuint NumNativeTexInstructions;
   GLuint NumNativeTexIndirections;
   /*@}*/
};


/** Vertex program object */
struct gl_vertex_program
{
   struct gl_program Base;   /**< base class */
   GLboolean IsNVProgram;    /**< is this a GL_NV_vertex_program program? */
   GLboolean IsPositionInvariant;
   GLboolean UsesClipDistance;
};


/** Geometry program object */
struct gl_geometry_program
{
   struct gl_program Base;   /**< base class */

   GLint VerticesOut;
   GLenum InputType;  /**< GL_POINTS, GL_LINES, GL_LINES_ADJACENCY_ARB,
                           GL_TRIANGLES, or GL_TRIANGLES_ADJACENCY_ARB */
   GLenum OutputType; /**< GL_POINTS, GL_LINE_STRIP or GL_TRIANGLE_STRIP */
};


/** Fragment program object */
struct gl_fragment_program
{
   struct gl_program Base;   /**< base class */
   GLboolean UsesKill;          /**< shader uses KIL instruction */
   GLboolean UsesDFdy;          /**< shader uses DDY instruction */
   GLboolean OriginUpperLeft;
   GLboolean PixelCenterInteger;
   enum gl_frag_depth_layout FragDepthLayout;

   /**
    * GLSL interpolation qualifier associated with each fragment shader input.
    * For inputs that do not have an interpolation qualifier specified in
    * GLSL, the value is INTERP_QUALIFIER_NONE.
    */
   enum glsl_interp_qualifier InterpQualifier[FRAG_ATTRIB_MAX];

   /**
    * Bitfield indicating, for each fragment shader input, 1 if that input
    * uses centroid interpolation, 0 otherwise.  Unused inputs are 0.
    */
   GLbitfield64 IsCentroid;
};


/**
 * State common to vertex and fragment programs.
 */
struct gl_program_state
{
   GLint ErrorPos;                       /* GL_PROGRAM_ERROR_POSITION_ARB/NV */
   const char *ErrorString;              /* GL_PROGRAM_ERROR_STRING_ARB/NV */
};


/**
 * Context state for vertex programs.
 */
struct gl_vertex_program_state
{
   GLboolean Enabled;            /**< User-set GL_VERTEX_PROGRAM_ARB/NV flag */
   GLboolean _Enabled;           /**< Enabled and _valid_ user program? */
   GLboolean PointSizeEnabled;   /**< GL_VERTEX_PROGRAM_POINT_SIZE_ARB/NV */
   GLboolean TwoSideEnabled;     /**< GL_VERTEX_PROGRAM_TWO_SIDE_ARB/NV */
   /** Computed two sided lighting for fixed function/programs. */
   GLboolean _TwoSideEnabled;
   struct gl_vertex_program *Current;  /**< User-bound vertex program */

   /** Currently enabled and valid vertex program (including internal
    * programs, user-defined vertex programs and GLSL vertex shaders).
    * This is the program we must use when rendering.
    */
   struct gl_vertex_program *_Current;

   GLfloat Parameters[MAX_PROGRAM_ENV_PARAMS][4]; /**< Env params */

   /* For GL_NV_vertex_program only: */
   GLenum TrackMatrix[MAX_PROGRAM_ENV_PARAMS / 4];
   GLenum TrackMatrixTransform[MAX_PROGRAM_ENV_PARAMS / 4];

   /** Should fixed-function T&L be implemented with a vertex prog? */
   GLboolean _MaintainTnlProgram;

   /** Program to emulate fixed-function T&L (see above) */
   struct gl_vertex_program *_TnlProgram;

   /** Cache of fixed-function programs */
   struct gl_program_cache *Cache;

   GLboolean _Overriden;
};


/**
 * Context state for geometry programs.
 */
struct gl_geometry_program_state
{
   GLboolean Enabled;               /**< GL_ARB_GEOMETRY_SHADER4 */
   GLboolean _Enabled;              /**< Enabled and valid program? */
   struct gl_geometry_program *Current;  /**< user-bound geometry program */

   /** Currently enabled and valid program (including internal programs
    * and compiled shader programs).
    */
   struct gl_geometry_program *_Current;

   GLfloat Parameters[MAX_PROGRAM_ENV_PARAMS][4]; /**< Env params */

   /** Cache of fixed-function programs */
   struct gl_program_cache *Cache;
};

/**
 * Context state for fragment programs.
 */
struct gl_fragment_program_state
{
   GLboolean Enabled;     /**< User-set fragment program enable flag */
   GLboolean _Enabled;    /**< Enabled and _valid_ user program? */
   struct gl_fragment_program *Current;  /**< User-bound fragment program */

   /** Currently enabled and valid fragment program (including internal
    * programs, user-defined fragment programs and GLSL fragment shaders).
    * This is the program we must use when rendering.
    */
   struct gl_fragment_program *_Current;

   GLfloat Parameters[MAX_PROGRAM_ENV_PARAMS][4]; /**< Env params */

   /** Should fixed-function texturing be implemented with a fragment prog? */
   GLboolean _MaintainTexEnvProgram;

   /** Program to emulate fixed-function texture env/combine (see above) */
   struct gl_fragment_program *_TexEnvProgram;

   /** Cache of fixed-function programs */
   struct gl_program_cache *Cache;
};


/**
 * ATI_fragment_shader runtime state
 */
#define ATI_FS_INPUT_PRIMARY 0
#define ATI_FS_INPUT_SECONDARY 1

struct atifs_instruction;
struct atifs_setupinst;

/**
 * ATI fragment shader
 */
struct ati_fragment_shader
{
   GLuint Id;
   GLint RefCount;
   struct atifs_instruction *Instructions[2];
   struct atifs_setupinst *SetupInst[2];
   GLfloat Constants[8][4];
   GLbitfield LocalConstDef;  /**< Indicates which constants have been set */
   GLubyte numArithInstr[2];
   GLubyte regsAssigned[2];
   GLubyte NumPasses;         /**< 1 or 2 */
   GLubyte cur_pass;
   GLubyte last_optype;
   GLboolean interpinp1;
   GLboolean isValid;
   GLuint swizzlerq;
};

/**
 * Context state for GL_ATI_fragment_shader
 */
struct gl_ati_fragment_shader_state
{
   GLboolean Enabled;
   GLboolean _Enabled;                  /**< enabled and valid shader? */
   GLboolean Compiling;
   GLfloat GlobalConstants[8][4];
   struct ati_fragment_shader *Current;
};


/** Set by #pragma directives */
struct gl_sl_pragmas
{
   GLboolean IgnoreOptimize;  /**< ignore #pragma optimize(on/off) ? */
   GLboolean IgnoreDebug;     /**< ignore #pragma debug(on/off) ? */
   GLboolean Optimize;  /**< defaults on */
   GLboolean Debug;     /**< defaults off */
};


/**
 * A GLSL vertex or fragment shader object.
 */
struct gl_shader
{
   GLenum Type;  /**< GL_FRAGMENT_SHADER || GL_VERTEX_SHADER || GL_GEOMETRY_SHADER_ARB (first field!) */
   GLuint Name;  /**< AKA the handle */
   GLint RefCount;  /**< Reference count */
   GLboolean DeletePending;
   GLboolean CompileStatus;
   const GLchar *Source;  /**< Source code string */
   GLuint SourceChecksum;       /**< for debug/logging purposes */
   struct gl_program *Program;  /**< Post-compile assembly code */
   GLchar *InfoLog;
   struct gl_sl_pragmas Pragmas;

   unsigned Version;       /**< GLSL version used for linking */

   /**
    * \name Sampler tracking
    *
    * \note Each of these fields is only set post-linking.
    */
   /*@{*/
   unsigned num_samplers;	/**< Number of samplers used by this shader. */
   GLbitfield active_samplers;	/**< Bitfield of which samplers are used */
   GLbitfield shadow_samplers;	/**< Samplers used for shadow sampling. */
   /*@}*/

   /**
    * Number of uniform components used by this shader.
    *
    * This field is only set post-linking.
    */
   unsigned num_uniform_components;

   /**
    * This shader's uniform block information.
    *
    * The offsets of the variables are assigned only for shaders in a program's
    * _LinkedShaders[].
    */
   struct gl_uniform_block *UniformBlocks;
   unsigned NumUniformBlocks;

   struct exec_list *ir;
   struct glsl_symbol_table *symbols;

   /** Shaders containing built-in functions that are used for linking. */
   struct gl_shader *builtins_to_link[16];
   unsigned num_builtins_to_link;
};


/**
 * Shader stages. Note that these will become 5 with tessellation.
 * These MUST have the same values as gallium's PIPE_SHADER_*
 */
typedef enum
{
   MESA_SHADER_VERTEX = 0,
   MESA_SHADER_FRAGMENT = 1,
   MESA_SHADER_GEOMETRY = 2,
   MESA_SHADER_TYPES = 3
} gl_shader_type;

struct gl_uniform_buffer_variable
{
   char *Name;
   const struct glsl_type *Type;
   unsigned int Buffer;
   unsigned int Offset;
   GLboolean RowMajor;
};

struct gl_uniform_block
{
   /** Declared name of the uniform block */
   char *Name;

   /** Array of supplemental information about UBO ir_variables. */
   struct gl_uniform_buffer_variable *Uniforms;
   GLuint NumUniforms;

   /**
    * Index (GL_UNIFORM_BLOCK_BINDING) into ctx->UniformBufferBindings[] to use
    * with glBindBufferBase to bind a buffer object to this uniform block.  When
    * updated in the program, _NEW_BUFFER_OBJECT will be set.
    */
   GLuint Binding;

   /**
    * Minimum size of a buffer object to back this uniform buffer
    * (GL_UNIFORM_BLOCK_DATA_SIZE).
    */
   GLuint UniformBufferSize;
};

/**
 * A GLSL program object.
 * Basically a linked collection of vertex and fragment shaders.
 */
struct gl_shader_program
{
   GLenum Type;  /**< Always GL_SHADER_PROGRAM (internal token) */
   GLuint Name;  /**< aka handle or ID */
   GLint RefCount;  /**< Reference count */
   GLboolean DeletePending;

   /**
    * Flags that the linker should not reject the program if it lacks
    * a vertex or fragment shader.  GLES2 doesn't allow separate
    * shader objects, and would reject them.  However, we internally
    * build separate shader objects for fixed function programs, which
    * we use for drivers/common/meta.c and for handling
    * _mesa_update_state with no program bound (for example in
    * glClear()).
    */
   GLboolean InternalSeparateShader;

   GLuint NumShaders;          /**< number of attached shaders */
   struct gl_shader **Shaders; /**< List of attached the shaders */

   /**
    * User-defined attribute bindings
    *
    * These are set via \c glBindAttribLocation and are used to direct the
    * GLSL linker.  These are \b not the values used in the compiled shader,
    * and they are \b not the values returned by \c glGetAttribLocation.
    */
   struct string_to_uint_map *AttributeBindings;

   /**
    * User-defined fragment data bindings
    *
    * These are set via \c glBindFragDataLocation and are used to direct the
    * GLSL linker.  These are \b not the values used in the compiled shader,
    * and they are \b not the values returned by \c glGetFragDataLocation.
    */
   struct string_to_uint_map *FragDataBindings;
   struct string_to_uint_map *FragDataIndexBindings;

   /**
    * Transform feedback varyings last specified by
    * glTransformFeedbackVaryings().
    *
    * For the current set of transform feeedback varyings used for transform
    * feedback output, see LinkedTransformFeedback.
    */
   struct {
      GLenum BufferMode;
      GLuint NumVarying;
      GLchar **VaryingNames;  /**< Array [NumVarying] of char * */
   } TransformFeedback;

   /** Post-link transform feedback info. */
   struct gl_transform_feedback_info LinkedTransformFeedback;

   /** Post-link gl_FragDepth layout for ARB_conservative_depth. */
   enum gl_frag_depth_layout FragDepthLayout;

   /** Geometry shader state - copied into gl_geometry_program at link time */
   struct {
      GLint VerticesOut;
      GLenum InputType;  /**< GL_POINTS, GL_LINES, GL_LINES_ADJACENCY_ARB,
                              GL_TRIANGLES, or GL_TRIANGLES_ADJACENCY_ARB */
      GLenum OutputType; /**< GL_POINTS, GL_LINE_STRIP or GL_TRIANGLE_STRIP */
   } Geom;

   /** Vertex shader state - copied into gl_vertex_program at link time */
   struct {
      GLboolean UsesClipDistance; /**< True if gl_ClipDistance is written to. */
      GLuint ClipDistanceArraySize; /**< Size of the gl_ClipDistance array, or
                                         0 if not present. */
   } Vert;

   /* post-link info: */
   unsigned NumUserUniformStorage;
   struct gl_uniform_storage *UniformStorage;

   struct gl_uniform_block *UniformBlocks;
   unsigned NumUniformBlocks;

   /**
    * Indices into the _LinkedShaders's UniformBlocks[] array for each stage
    * they're used in, or -1.
    *
    * This is used to maintain the Binding values of the stage's UniformBlocks[]
    * and to answer the GL_UNIFORM_BLOCK_REFERENCED_BY_*_SHADER queries.
    */
   int *UniformBlockStageIndex[MESA_SHADER_TYPES];

   /**
    * Map of active uniform names to locations
    *
    * Maps any active uniform that is not an array element to a location.
    * Each active uniform, including individual structure members will appear
    * in this map.  This roughly corresponds to the set of names that would be
    * enumerated by \c glGetActiveUniform.
    */
   struct string_to_uint_map *UniformHash;

   /**
    * Map from sampler unit to texture unit (set by glUniform1i())
    *
    * A sampler unit is associated with each sampler uniform by the linker.
    * The sampler unit associated with each uniform is stored in the
    * \c gl_uniform_storage::sampler field.
    */
   GLubyte SamplerUnits[MAX_SAMPLERS];
   /** Which texture target is being sampled (TEXTURE_1D/2D/3D/etc_INDEX) */
   gl_texture_index SamplerTargets[MAX_SAMPLERS];

   GLboolean LinkStatus;   /**< GL_LINK_STATUS */
   GLboolean Validated;
   GLboolean _Used;        /**< Ever used for drawing? */
   GLchar *InfoLog;

   unsigned Version;       /**< GLSL version used for linking */

   /**
    * Per-stage shaders resulting from the first stage of linking.
    *
    * Set of linked shaders for this program.  The array is accessed using the
    * \c MESA_SHADER_* defines.  Entries for non-existent stages will be
    * \c NULL.
    */
   struct gl_shader *_LinkedShaders[MESA_SHADER_TYPES];
};   


#define GLSL_DUMP      0x1  /**< Dump shaders to stdout */
#define GLSL_LOG       0x2  /**< Write shaders to files */
#define GLSL_OPT       0x4  /**< Force optimizations (override pragmas) */
#define GLSL_NO_OPT    0x8  /**< Force no optimizations (override pragmas) */
#define GLSL_UNIFORMS 0x10  /**< Print glUniform calls */
#define GLSL_NOP_VERT 0x20  /**< Force no-op vertex shaders */
#define GLSL_NOP_FRAG 0x40  /**< Force no-op fragment shaders */
#define GLSL_USE_PROG 0x80  /**< Log glUseProgram calls */
#define GLSL_REPORT_ERRORS 0x100  /**< Print compilation errors */


/**
 * Context state for GLSL vertex/fragment shaders.
 */
struct gl_shader_state
{
   /**
    * Programs used for rendering
    *
    * There is a separate program set for each shader stage.  If
    * GL_EXT_separate_shader_objects is not supported, each of these must point
    * to \c NULL or to the same program.
    */
   struct gl_shader_program *CurrentVertexProgram;
   struct gl_shader_program *CurrentGeometryProgram;
   struct gl_shader_program *CurrentFragmentProgram;

   struct gl_shader_program *_CurrentFragmentProgram;

   /**
    * Program used by glUniform calls.
    *
    * Explicitly set by \c glUseProgram and \c glActiveProgramEXT.
    */
   struct gl_shader_program *ActiveProgram;

   GLbitfield Flags;                    /**< Mask of GLSL_x flags */
};


/**
 * Compiler options for a single GLSL shaders type
 */
struct gl_shader_compiler_options
{
   /** Driver-selectable options: */
   GLboolean EmitCondCodes;             /**< Use condition codes? */
   GLboolean EmitNVTempInitialization;  /**< 0-fill NV temp registers */
   GLboolean EmitNoLoops;
   GLboolean EmitNoFunctions;
   GLboolean EmitNoCont;                  /**< Emit CONT opcode? */
   GLboolean EmitNoMainReturn;            /**< Emit CONT/RET opcodes? */
   GLboolean EmitNoNoise;                 /**< Emit NOISE opcodes? */
   GLboolean EmitNoPow;                   /**< Emit POW opcodes? */
   GLboolean LowerClipDistance; /**< Lower gl_ClipDistance from float[8] to vec4[2]? */

   /**
    * \name Forms of indirect addressing the driver cannot do.
    */
   /*@{*/
   GLboolean EmitNoIndirectInput;   /**< No indirect addressing of inputs */
   GLboolean EmitNoIndirectOutput;  /**< No indirect addressing of outputs */
   GLboolean EmitNoIndirectTemp;    /**< No indirect addressing of temps */
   GLboolean EmitNoIndirectUniform; /**< No indirect addressing of constants */
   /*@}*/

   GLuint MaxIfDepth;               /**< Maximum nested IF blocks */
   GLuint MaxUnrollIterations;

   struct gl_sl_pragmas DefaultPragmas; /**< Default #pragma settings */
};


/**
 * Occlusion/timer query object.
 */
struct gl_query_object
{
   GLenum Target;      /**< The query target, when active */
   GLuint Id;          /**< hash table ID/name */
   GLuint64EXT Result; /**< the counter */
   GLboolean Active;   /**< inside Begin/EndQuery */
   GLboolean Ready;    /**< result is ready? */
};


/**
 * Context state for query objects.
 */
struct gl_query_state
{
   struct _mesa_HashTable *QueryObjects;
   struct gl_query_object *CurrentOcclusionObject; /* GL_ARB_occlusion_query */
   struct gl_query_object *CurrentTimerObject;     /* GL_EXT_timer_query */

   /** GL_NV_conditional_render */
   struct gl_query_object *CondRenderQuery;

   /** GL_EXT_transform_feedback */
   struct gl_query_object *PrimitivesGenerated;
   struct gl_query_object *PrimitivesWritten;

   /** GL_ARB_timer_query */
   struct gl_query_object *TimeElapsed;

   GLenum CondRenderMode;
};


/** Sync object state */
struct gl_sync_object
{
   struct simple_node link;
   GLenum Type;               /**< GL_SYNC_FENCE */
   GLuint Name;               /**< Fence name */
   GLint RefCount;            /**< Reference count */
   GLboolean DeletePending;   /**< Object was deleted while there were still
			       * live references (e.g., sync not yet finished)
			       */
   GLenum SyncCondition;
   GLbitfield Flags;          /**< Flags passed to glFenceSync */
   GLuint StatusFlag:1;       /**< Has the sync object been signaled? */
};


/**
 * State which can be shared by multiple contexts:
 */
struct gl_shared_state
{
   _glthread_Mutex Mutex;		   /**< for thread safety */
   GLint RefCount;			   /**< Reference count */
   struct _mesa_HashTable *DisplayList;	   /**< Display lists hash table */
   struct _mesa_HashTable *TexObjects;	   /**< Texture objects hash table */

   /** Default texture objects (shared by all texture units) */
   struct gl_texture_object *DefaultTex[NUM_TEXTURE_TARGETS];

   /** Fallback texture used when a bound texture is incomplete */
   struct gl_texture_object *FallbackTex[NUM_TEXTURE_TARGETS];

   /**
    * \name Thread safety and statechange notification for texture
    * objects. 
    *
    * \todo Improve the granularity of locking.
    */
   /*@{*/
   _glthread_Mutex TexMutex;		/**< texobj thread safety */
   GLuint TextureStateStamp;	        /**< state notification for shared tex */
   /*@}*/

   /** Default buffer object for vertex arrays that aren't in VBOs */
   struct gl_buffer_object *NullBufferObj;

   /**
    * \name Vertex/geometry/fragment programs
    */
   /*@{*/
   struct _mesa_HashTable *Programs; /**< All vertex/fragment programs */
   struct gl_vertex_program *DefaultVertexProgram;
   struct gl_fragment_program *DefaultFragmentProgram;
   struct gl_geometry_program *DefaultGeometryProgram;
   /*@}*/

   /* GL_ATI_fragment_shader */
   struct _mesa_HashTable *ATIShaders;
   struct ati_fragment_shader *DefaultFragmentShader;

   struct _mesa_HashTable *BufferObjects;

   /** Table of both gl_shader and gl_shader_program objects */
   struct _mesa_HashTable *ShaderObjects;

   /* GL_EXT_framebuffer_object */
   struct _mesa_HashTable *RenderBuffers;
   struct _mesa_HashTable *FrameBuffers;

   /* GL_ARB_sync */
   struct simple_node SyncObjects;

   /** GL_ARB_sampler_objects */
   struct _mesa_HashTable *SamplerObjects;
};



/**
 * Renderbuffers represent drawing surfaces such as color, depth and/or
 * stencil.  A framebuffer object has a set of renderbuffers.
 * Drivers will typically derive subclasses of this type.
 */
struct gl_renderbuffer
{
   _glthread_Mutex Mutex; /**< for thread safety */
   GLuint ClassID;        /**< Useful for drivers */
   GLuint Name;
   GLint RefCount;
   GLuint Width, Height;
   GLboolean Purgeable;  /**< Is the buffer purgeable under memory pressure? */
   GLboolean AttachedAnytime; /**< TRUE if it was attached to a framebuffer */
   GLubyte NumSamples;
   GLenum InternalFormat; /**< The user-specified format */
   GLenum _BaseFormat;    /**< Either GL_RGB, GL_RGBA, GL_DEPTH_COMPONENT or
                               GL_STENCIL_INDEX. */
   gl_format Format;      /**< The actual renderbuffer memory format */

   /** Delete this renderbuffer */
   void (*Delete)(struct gl_renderbuffer *rb);

   /** Allocate new storage for this renderbuffer */
   GLboolean (*AllocStorage)(struct gl_context *ctx,
                             struct gl_renderbuffer *rb,
                             GLenum internalFormat,
                             GLuint width, GLuint height);
};


/**
 * A renderbuffer attachment points to either a texture object (and specifies
 * a mipmap level, cube face or 3D texture slice) or points to a renderbuffer.
 */
struct gl_renderbuffer_attachment
{
   GLenum Type;  /**< \c GL_NONE or \c GL_TEXTURE or \c GL_RENDERBUFFER_EXT */
   GLboolean Complete;

   /**
    * If \c Type is \c GL_RENDERBUFFER_EXT, this stores a pointer to the
    * application supplied renderbuffer object.
    */
   struct gl_renderbuffer *Renderbuffer;

   /**
    * If \c Type is \c GL_TEXTURE, this stores a pointer to the application
    * supplied texture object.
    */
   struct gl_texture_object *Texture;
   GLuint TextureLevel; /**< Attached mipmap level. */
   GLuint CubeMapFace;  /**< 0 .. 5, for cube map textures. */
   GLuint Zoffset;      /**< Slice for 3D textures,  or layer for both 1D
                         * and 2D array textures */
};


/**
 * A framebuffer is a collection of renderbuffers (color, depth, stencil, etc).
 * In C++ terms, think of this as a base class from which device drivers
 * will make derived classes.
 */
struct gl_framebuffer
{
   _glthread_Mutex Mutex;  /**< for thread safety */
   /**
    * If zero, this is a window system framebuffer.  If non-zero, this
    * is a FBO framebuffer; note that for some devices (i.e. those with
    * a natural pixel coordinate system for FBOs that differs from the
    * OpenGL/Mesa coordinate system), this means that the viewport,
    * polygon face orientation, and polygon stipple will have to be inverted.
    */
   GLuint Name;

   GLint RefCount;
   GLboolean DeletePending;

   /**
    * The framebuffer's visual. Immutable if this is a window system buffer.
    * Computed from attachments if user-made FBO.
    */
   struct gl_config Visual;

   GLboolean Initialized;

   GLuint Width, Height;	/**< size of frame buffer in pixels */

   /** \name  Drawing bounds (Intersection of buffer size and scissor box) */
   /*@{*/
   GLint _Xmin, _Xmax;  /**< inclusive */
   GLint _Ymin, _Ymax;  /**< exclusive */
   /*@}*/

   /** \name  Derived Z buffer stuff */
   /*@{*/
   GLuint _DepthMax;	/**< Max depth buffer value */
   GLfloat _DepthMaxF;	/**< Float max depth buffer value */
   GLfloat _MRD;	/**< minimum resolvable difference in Z values */
   /*@}*/

   /** One of the GL_FRAMEBUFFER_(IN)COMPLETE_* tokens */
   GLenum _Status;

   /** Integer color values */
   GLboolean _IntegerColor;

   /** Array of all renderbuffer attachments, indexed by BUFFER_* tokens. */
   struct gl_renderbuffer_attachment Attachment[BUFFER_COUNT];

   /* In unextended OpenGL these vars are part of the GL_COLOR_BUFFER
    * attribute group and GL_PIXEL attribute group, respectively.
    */
   GLenum ColorDrawBuffer[MAX_DRAW_BUFFERS];
   GLenum ColorReadBuffer;

   /** Computed from ColorDraw/ReadBuffer above */
   GLuint _NumColorDrawBuffers;
   GLint _ColorDrawBufferIndexes[MAX_DRAW_BUFFERS]; /**< BUFFER_x or -1 */
   GLint _ColorReadBufferIndex; /* -1 = None */
   struct gl_renderbuffer *_ColorDrawBuffers[MAX_DRAW_BUFFERS];
   struct gl_renderbuffer *_ColorReadBuffer;

   /** Delete this framebuffer */
   void (*Delete)(struct gl_framebuffer *fb);
};


/**
 * Precision info for shader datatypes.  See glGetShaderPrecisionFormat().
 */
struct gl_precision
{
   GLushort RangeMin;   /**< min value exponent */
   GLushort RangeMax;   /**< max value exponent */
   GLushort Precision;  /**< number of mantissa bits */
};


/**
 * Limits for vertex, geometry and fragment programs/shaders.
 */
struct gl_program_constants
{
   /* logical limits */
   GLuint MaxInstructions;
   GLuint MaxAluInstructions;
   GLuint MaxTexInstructions;
   GLuint MaxTexIndirections;
   GLuint MaxAttribs;
   GLuint MaxTemps;
   GLuint MaxAddressRegs;
   GLuint MaxAddressOffset;  /**< [-MaxAddressOffset, MaxAddressOffset-1] */
   GLuint MaxParameters;
   GLuint MaxLocalParams;
   GLuint MaxEnvParams;
   /* native/hardware limits */
   GLuint MaxNativeInstructions;
   GLuint MaxNativeAluInstructions;
   GLuint MaxNativeTexInstructions;
   GLuint MaxNativeTexIndirections;
   GLuint MaxNativeAttribs;
   GLuint MaxNativeTemps;
   GLuint MaxNativeAddressRegs;
   GLuint MaxNativeParameters;
   /* For shaders */
   GLuint MaxUniformComponents;  /**< Usually == MaxParameters * 4 */
   /* ES 2.0 and GL_ARB_ES2_compatibility */
   struct gl_precision LowFloat, MediumFloat, HighFloat;
   struct gl_precision LowInt, MediumInt, HighInt;
   /* GL_ARB_uniform_buffer_object */
   GLuint MaxUniformBlocks;
   GLuint MaxCombinedUniformComponents;
};


/**
 * Constants which may be overridden by device driver during context creation
 * but are never changed after that.
 */
struct gl_constants
{
   GLint MaxTextureMbytes;      /**< Max memory per image, in MB */
   GLint MaxTextureLevels;      /**< Max mipmap levels. */ 
   GLint Max3DTextureLevels;    /**< Max mipmap levels for 3D textures */
   GLint MaxCubeTextureLevels;  /**< Max mipmap levels for cube textures */
   GLint MaxArrayTextureLayers; /**< Max layers in array textures */
   GLint MaxTextureRectSize;    /**< Max rectangle texture size, in pixes */
   GLuint MaxTextureCoordUnits;
   GLuint MaxTextureImageUnits;
   GLuint MaxVertexTextureImageUnits;
   GLuint MaxCombinedTextureImageUnits;
   GLuint MaxGeometryTextureImageUnits;
   GLuint MaxTextureUnits;           /**< = MIN(CoordUnits, ImageUnits) */
   GLfloat MaxTextureMaxAnisotropy;  /**< GL_EXT_texture_filter_anisotropic */
   GLfloat MaxTextureLodBias;        /**< GL_EXT_texture_lod_bias */
   GLuint MaxTextureBufferSize;      /**< GL_ARB_texture_buffer_object */

   GLuint MaxArrayLockSize;

   GLint SubPixelBits;

   GLfloat MinPointSize, MaxPointSize;	     /**< aliased */
   GLfloat MinPointSizeAA, MaxPointSizeAA;   /**< antialiased */
   GLfloat PointSizeGranularity;
   GLfloat MinLineWidth, MaxLineWidth;       /**< aliased */
   GLfloat MinLineWidthAA, MaxLineWidthAA;   /**< antialiased */
   GLfloat LineWidthGranularity;

   GLuint MaxColorTableSize;

   GLuint MaxClipPlanes;
   GLuint MaxLights;
   GLfloat MaxShininess;                     /**< GL_NV_light_max_exponent */
   GLfloat MaxSpotExponent;                  /**< GL_NV_light_max_exponent */

   GLuint MaxViewportWidth, MaxViewportHeight;

   struct gl_program_constants VertexProgram;   /**< GL_ARB_vertex_program */
   struct gl_program_constants FragmentProgram; /**< GL_ARB_fragment_program */
   struct gl_program_constants GeometryProgram;  /**< GL_ARB_geometry_shader4 */
   GLuint MaxProgramMatrices;
   GLuint MaxProgramMatrixStackDepth;

   struct {
      GLuint SamplesPassed;
      GLuint TimeElapsed;
      GLuint Timestamp;
      GLuint PrimitivesGenerated;
      GLuint PrimitivesWritten;
   } QueryCounterBits;

   /** vertex array / buffer object bounds checking */
   GLboolean CheckArrayBounds;

   GLuint MaxDrawBuffers;    /**< GL_ARB_draw_buffers */

   GLuint MaxColorAttachments;   /**< GL_EXT_framebuffer_object */
   GLuint MaxRenderbufferSize;   /**< GL_EXT_framebuffer_object */
   GLuint MaxSamples;            /**< GL_ARB_framebuffer_object */

   /** Number of varying vectors between vertex and fragment shaders */
   GLuint MaxVarying;
   GLuint MaxVertexVaryingComponents;   /**< Between vert and geom shader */
   GLuint MaxGeometryVaryingComponents; /**< Between geom and frag shader */

   /** @{
    * GL_ARB_uniform_buffer_object
    */
   GLuint MaxCombinedUniformBlocks;
   GLuint MaxUniformBufferBindings;
   GLuint MaxUniformBlockSize;
   GLuint UniformBufferOffsetAlignment;
   /** @} */

   /** GL_ARB_geometry_shader4 */
   GLuint MaxGeometryOutputVertices;
   GLuint MaxGeometryTotalOutputComponents;

   GLuint GLSLVersion;  /**< GLSL version supported (ex: 120 = 1.20) */

   /**
    * Changes default GLSL extension behavior from "error" to "warn".  It's out
    * of spec, but it can make some apps work that otherwise wouldn't.
    */
   GLboolean ForceGLSLExtensionsWarn;

   /**
    * Does the driver support real 32-bit integers?  (Otherwise, integers are
    * simulated via floats.)
    */
   GLboolean NativeIntegers;

   /**
    * If the driver supports real 32-bit integers, what integer value should be
    * used for boolean true in uniform uploads?  (Usually 1 or ~0.)
    */
   GLuint UniformBooleanTrue;

   /** Which texture units support GL_ATI_envmap_bumpmap as targets */
   GLbitfield SupportedBumpUnits;

   /**
    * Maximum amount of time, measured in nanseconds, that the server can wait.
    */
   GLuint64 MaxServerWaitTimeout;

   /** GL_EXT_provoking_vertex */
   GLboolean QuadsFollowProvokingVertexConvention;

   /** OpenGL version 3.0 */
   GLbitfield ContextFlags;  /**< Ex: GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT */

   /** OpenGL version 3.2 */
   GLbitfield ProfileMask;   /**< Mask of CONTEXT_x_PROFILE_BIT */

   /** GL_EXT_transform_feedback */
   GLuint MaxTransformFeedbackBuffers;
   GLuint MaxTransformFeedbackSeparateComponents;
   GLuint MaxTransformFeedbackInterleavedComponents;
   GLuint MaxVertexStreams;

   /** GL_EXT_gpu_shader4 */
   GLint MinProgramTexelOffset, MaxProgramTexelOffset;

   /* GL_ARB_robustness */
   GLenum ResetStrategy;

   /* GL_ARB_blend_func_extended */
   GLuint MaxDualSourceDrawBuffers;

   /**
    * Whether the implementation strips out and ignores texture borders.
    *
    * Many GPU hardware implementations don't support rendering with texture
    * borders and mipmapped textures.  (Note: not static border color, but the
    * old 1-pixel border around each edge).  Implementations then have to do
    * slow fallbacks to be correct, or just ignore the border and be fast but
    * wrong.  Setting the flag strips the border off of TexImage calls,
    * providing "fast but wrong" at significantly reduced driver complexity.
    *
    * Texture borders are deprecated in GL 3.0.
    **/
   GLboolean StripTextureBorder;

   /**
    * For drivers which can do a better job at eliminating unused varyings
    * and uniforms than the GLSL compiler.
    *
    * XXX Remove these as soon as a better solution is available.
    */
   GLboolean GLSLSkipStrictMaxVaryingLimitCheck;
   GLboolean GLSLSkipStrictMaxUniformLimitCheck;

   /**
    * Force software support for primitive restart in the VBO module.
    */
   GLboolean PrimitiveRestartInSoftware;
};


/**
 * Enable flag for each OpenGL extension.  Different device drivers will
 * enable different extensions at runtime.
 */
struct gl_extensions
{
   GLboolean dummy;  /* don't remove this! */
   GLboolean dummy_true;  /* Set true by _mesa_init_extensions(). */
   GLboolean dummy_false; /* Set false by _mesa_init_extensions(). */
   GLboolean ANGLE_texture_compression_dxt;
   GLboolean ARB_ES2_compatibility;
   GLboolean ARB_base_instance;
   GLboolean ARB_blend_func_extended;
   GLboolean ARB_color_buffer_float;
   GLboolean ARB_conservative_depth;
   GLboolean ARB_copy_buffer;
   GLboolean ARB_depth_buffer_float;
   GLboolean ARB_depth_clamp;
   GLboolean ARB_depth_texture;
   GLboolean ARB_draw_buffers_blend;
   GLboolean ARB_draw_elements_base_vertex;
   GLboolean ARB_draw_instanced;
   GLboolean ARB_fragment_coord_conventions;
   GLboolean ARB_fragment_program;
   GLboolean ARB_fragment_program_shadow;
   GLboolean ARB_fragment_shader;
   GLboolean ARB_framebuffer_object;
   GLboolean ARB_explicit_attrib_location;
   GLboolean ARB_geometry_shader4;
   GLboolean ARB_half_float_pixel;
   GLboolean ARB_half_float_vertex;
   GLboolean ARB_instanced_arrays;
   GLboolean ARB_map_buffer_range;
   GLboolean ARB_occlusion_query;
   GLboolean ARB_occlusion_query2;
   GLboolean ARB_point_sprite;
   GLboolean ARB_seamless_cube_map;
   GLboolean ARB_shader_bit_encoding;
   GLboolean ARB_shader_objects;
   GLboolean ARB_shader_stencil_export;
   GLboolean ARB_shader_texture_lod;
   GLboolean ARB_shading_language_100;
   GLboolean ARB_shadow;
   GLboolean ARB_sync;
   GLboolean ARB_texture_border_clamp;
   GLboolean ARB_texture_buffer_object;
   GLboolean ARB_texture_compression_rgtc;
   GLboolean ARB_texture_cube_map;
   GLboolean ARB_texture_env_combine;
   GLboolean ARB_texture_env_crossbar;
   GLboolean ARB_texture_env_dot3;
   GLboolean ARB_texture_float;
   GLboolean ARB_texture_multisample;
   GLboolean ARB_texture_non_power_of_two;
   GLboolean ARB_texture_rg;
   GLboolean ARB_texture_rgb10_a2ui;
   GLboolean ARB_texture_storage;
   GLboolean ARB_timer_query;
   GLboolean ARB_transform_feedback2;
   GLboolean ARB_transform_feedback3;
   GLboolean ARB_transform_feedback_instanced;
   GLboolean ARB_transpose_matrix;
   GLboolean ARB_uniform_buffer_object;
   GLboolean ARB_vertex_program;
   GLboolean ARB_vertex_shader;
   GLboolean ARB_vertex_type_2_10_10_10_rev;
   GLboolean ARB_window_pos;
   GLboolean EXT_blend_color;
   GLboolean EXT_blend_equation_separate;
   GLboolean EXT_blend_func_separate;
   GLboolean EXT_blend_minmax;
   GLboolean EXT_clip_volume_hint;
   GLboolean EXT_compiled_vertex_array;
   GLboolean EXT_depth_bounds_test;
   GLboolean EXT_draw_buffers2;
   GLboolean EXT_draw_range_elements;
   GLboolean EXT_fog_coord;
   GLboolean EXT_framebuffer_blit;
   GLboolean EXT_framebuffer_multisample;
   GLboolean EXT_framebuffer_object;
   GLboolean EXT_framebuffer_sRGB;
   GLboolean EXT_gpu_program_parameters;
   GLboolean EXT_gpu_shader4;
   GLboolean EXT_packed_depth_stencil;
   GLboolean EXT_packed_float;
   GLboolean EXT_packed_pixels;
   GLboolean EXT_pixel_buffer_object;
   GLboolean EXT_point_parameters;
   GLboolean EXT_provoking_vertex;
   GLboolean EXT_rescale_normal;
   GLboolean EXT_shadow_funcs;
   GLboolean EXT_secondary_color;
   GLboolean EXT_separate_shader_objects;
   GLboolean EXT_separate_specular_color;
   GLboolean EXT_stencil_two_side;
   GLboolean EXT_texture3D;
   GLboolean EXT_texture_array;
   GLboolean EXT_texture_compression_latc;
   GLboolean EXT_texture_compression_s3tc;
   GLboolean EXT_texture_env_dot3;
   GLboolean EXT_texture_filter_anisotropic;
   GLboolean EXT_texture_integer;
   GLboolean EXT_texture_mirror_clamp;
   GLboolean EXT_texture_shared_exponent;
   GLboolean EXT_texture_snorm;
   GLboolean EXT_texture_sRGB;
   GLboolean EXT_texture_sRGB_decode;
   GLboolean EXT_texture_swizzle;
   GLboolean EXT_transform_feedback;
   GLboolean EXT_timer_query;
   GLboolean EXT_vertex_array_bgra;
   GLboolean OES_standard_derivatives;
   /* vendor extensions */
   GLboolean AMD_seamless_cubemap_per_texture;
   GLboolean APPLE_packed_pixels;
   GLboolean APPLE_object_purgeable;
   GLboolean ATI_envmap_bumpmap;
   GLboolean ATI_texture_compression_3dc;
   GLboolean ATI_texture_mirror_once;
   GLboolean ATI_texture_env_combine3;
   GLboolean ATI_fragment_shader;
   GLboolean ATI_separate_stencil;
   GLboolean IBM_rasterpos_clip;
   GLboolean IBM_multimode_draw_arrays;
   GLboolean MESA_pack_invert;
   GLboolean MESA_resize_buffers;
   GLboolean MESA_ycbcr_texture;
   GLboolean MESA_texture_array;
   GLboolean NV_blend_square;
   GLboolean NV_conditional_render;
   GLboolean NV_fog_distance;
   GLboolean NV_fragment_program;
   GLboolean NV_fragment_program_option;
   GLboolean NV_light_max_exponent;
   GLboolean NV_point_sprite;
   GLboolean NV_primitive_restart;
   GLboolean NV_read_buffer;
   GLboolean NV_texture_barrier;
   GLboolean NV_texgen_reflection;
   GLboolean NV_texture_env_combine4;
   GLboolean NV_texture_rectangle;
   GLboolean NV_vertex_program;
   GLboolean NV_vertex_program1_1;
   GLboolean SGIS_texture_lod;
   GLboolean TDFX_texture_compression_FXT1;
   GLboolean S3_s3tc;
   GLboolean OES_EGL_image;
   GLboolean OES_draw_texture;
   GLboolean OES_EGL_image_external;
   GLboolean OES_compressed_ETC1_RGB8_texture;
   GLboolean extension_sentinel;
   /** The extension string */
   const GLubyte *String;
   /** Number of supported extensions */
   GLuint Count;
};


/**
 * A stack of matrices (projection, modelview, color, texture, etc).
 */
struct gl_matrix_stack
{
   GLmatrix *Top;      /**< points into Stack */
   GLmatrix *Stack;    /**< array [MaxDepth] of GLmatrix */
   GLuint Depth;       /**< 0 <= Depth < MaxDepth */
   GLuint MaxDepth;    /**< size of Stack[] array */
   GLuint DirtyFlag;   /**< _NEW_MODELVIEW or _NEW_PROJECTION, for example */
};


/**
 * \name Bits for image transfer operations 
 * \sa __struct gl_contextRec::ImageTransferState.
 */
/*@{*/
#define IMAGE_SCALE_BIAS_BIT                      0x1
#define IMAGE_SHIFT_OFFSET_BIT                    0x2
#define IMAGE_MAP_COLOR_BIT                       0x4
#define IMAGE_CLAMP_BIT                           0x800


/** Pixel Transfer ops */
#define IMAGE_BITS (IMAGE_SCALE_BIAS_BIT |			\
		    IMAGE_SHIFT_OFFSET_BIT |			\
		    IMAGE_MAP_COLOR_BIT)

/**
 * \name Bits to indicate what state has changed.  
 */
/*@{*/
#define _NEW_MODELVIEW         (1 << 0)   /**< gl_context::ModelView */
#define _NEW_PROJECTION        (1 << 1)   /**< gl_context::Projection */
#define _NEW_TEXTURE_MATRIX    (1 << 2)   /**< gl_context::TextureMatrix */
#define _NEW_COLOR             (1 << 3)   /**< gl_context::Color */
#define _NEW_DEPTH             (1 << 4)   /**< gl_context::Depth */
#define _NEW_EVAL              (1 << 5)   /**< gl_context::Eval, EvalMap */
#define _NEW_FOG               (1 << 6)   /**< gl_context::Fog */
#define _NEW_HINT              (1 << 7)   /**< gl_context::Hint */
#define _NEW_LIGHT             (1 << 8)   /**< gl_context::Light */
#define _NEW_LINE              (1 << 9)   /**< gl_context::Line */
#define _NEW_PIXEL             (1 << 10)  /**< gl_context::Pixel */
#define _NEW_POINT             (1 << 11)  /**< gl_context::Point */
#define _NEW_POLYGON           (1 << 12)  /**< gl_context::Polygon */
#define _NEW_POLYGONSTIPPLE    (1 << 13)  /**< gl_context::PolygonStipple */
#define _NEW_SCISSOR           (1 << 14)  /**< gl_context::Scissor */
#define _NEW_STENCIL           (1 << 15)  /**< gl_context::Stencil */
#define _NEW_TEXTURE           (1 << 16)  /**< gl_context::Texture */
#define _NEW_TRANSFORM         (1 << 17)  /**< gl_context::Transform */
#define _NEW_VIEWPORT          (1 << 18)  /**< gl_context::Viewport */
#define _NEW_PACKUNPACK        (1 << 19)  /**< gl_context::Pack, Unpack */
#define _NEW_ARRAY             (1 << 20)  /**< gl_context::Array */
#define _NEW_RENDERMODE        (1 << 21)  /**< gl_context::RenderMode, etc */
#define _NEW_BUFFERS           (1 << 22)  /**< gl_context::Visual, DrawBuffer, */
#define _NEW_CURRENT_ATTRIB    (1 << 23)  /**< gl_context::Current */
#define _NEW_MULTISAMPLE       (1 << 24)  /**< gl_context::Multisample */
#define _NEW_TRACK_MATRIX      (1 << 25)  /**< gl_context::VertexProgram */
#define _NEW_PROGRAM           (1 << 26)  /**< New program/shader state */
#define _NEW_PROGRAM_CONSTANTS (1 << 27)
#define _NEW_BUFFER_OBJECT     (1 << 28)
#define _NEW_FRAG_CLAMP        (1 << 29)
#define _NEW_TRANSFORM_FEEDBACK (1 << 30) /**< gl_context::TransformFeedback */
#define _NEW_VARYING_VP_INPUTS (1 << 31) /**< gl_context::varying_vp_inputs */
#define _NEW_ALL ~0

/**
 * We use _NEW_TRANSFORM for GL_RASTERIZER_DISCARD.  This #define is for
 * clarity.
 */
#define _NEW_RASTERIZER_DISCARD _NEW_TRANSFORM
/*@}*/


/**
 * \name A bunch of flags that we think might be useful to drivers.
 * 
 * Set in the __struct gl_contextRec::_TriangleCaps bitfield.
 */
/*@{*/
#define DD_SEPARATE_SPECULAR        (1 << 0)
#define DD_TRI_LIGHT_TWOSIDE        (1 << 1)
#define DD_TRI_UNFILLED             (1 << 2)
#define DD_TRI_SMOOTH               (1 << 3)
#define DD_TRI_STIPPLE              (1 << 4)
#define DD_TRI_OFFSET               (1 << 5)
#define DD_LINE_SMOOTH              (1 << 6)
#define DD_LINE_STIPPLE             (1 << 7)
#define DD_POINT_SMOOTH             (1 << 8)
#define DD_POINT_ATTEN              (1 << 9)
/*@}*/


/**
 * Composite state flags
 */
/*@{*/
#define _MESA_NEW_NEED_EYE_COORDS         (_NEW_LIGHT |		\
                                           _NEW_TEXTURE |	\
                                           _NEW_POINT |		\
                                           _NEW_PROGRAM |	\
                                           _NEW_MODELVIEW)

#define _MESA_NEW_SEPARATE_SPECULAR        (_NEW_LIGHT | \
                                            _NEW_FOG | \
                                            _NEW_PROGRAM)


/*@}*/




/* This has to be included here. */
#include "dd.h"


/**
 * Display list flags.
 * Strictly this is a tnl-private concept, but it doesn't seem
 * worthwhile adding a tnl private structure just to hold this one bit
 * of information:
 */
#define DLIST_DANGLING_REFS     0x1 


/** Opaque declaration of display list payload data type */
union gl_dlist_node;


/**
 * Provide a location where information about a display list can be
 * collected.  Could be extended with driverPrivate structures,
 * etc. in the future.
 */
struct gl_display_list
{
   GLuint Name;
   GLbitfield Flags;  /**< DLIST_x flags */
   /** The dlist commands are in a linked list of nodes */
   union gl_dlist_node *Head;
};


/**
 * State used during display list compilation and execution.
 */
struct gl_dlist_state
{
   GLuint CallDepth;		/**< Current recursion calling depth */

   struct gl_display_list *CurrentList; /**< List currently being compiled */
   union gl_dlist_node *CurrentBlock; /**< Pointer to current block of nodes */
   GLuint CurrentPos;		/**< Index into current block of nodes */

   GLvertexformat ListVtxfmt;

   GLubyte ActiveAttribSize[VERT_ATTRIB_MAX];
   GLfloat CurrentAttrib[VERT_ATTRIB_MAX][4];
   
   GLubyte ActiveMaterialSize[MAT_ATTRIB_MAX];
   GLfloat CurrentMaterial[MAT_ATTRIB_MAX][4];

   struct {
      /* State known to have been set by the currently-compiling display
       * list.  Used to eliminate some redundant state changes.
       */
      GLenum ShadeModel;
   } Current;
};

/**
 * An error, warning, or other piece of debug information for an application
 * to consume via GL_ARB_debug_output.
 */
struct gl_debug_msg
{
   GLenum source;
   GLenum type;
   GLuint id;
   GLenum severity;
   GLsizei length;
   GLcharARB *message;
};

typedef enum {
   API_ERROR_UNKNOWN,
   API_ERROR_COUNT
} gl_api_error;

typedef enum {
   WINSYS_ERROR_UNKNOWN,
   WINSYS_ERROR_COUNT
} gl_winsys_error;

typedef enum {
   SHADER_ERROR_UNKNOWN,
   SHADER_ERROR_COUNT
} gl_shader_error;

typedef enum {
   OTHER_ERROR_UNKNOWN,
   OTHER_ERROR_OUT_OF_MEMORY,
   OTHER_ERROR_COUNT
} gl_other_error;

struct gl_client_namespace
{
   struct _mesa_HashTable *IDs;
   unsigned ZeroID; /* a HashTable won't take zero, so store its state here */
   struct simple_node Severity[3]; /* lists of IDs in the hash table */
};

struct gl_client_debug
{
   GLboolean Defaults[3][2][6]; /* severity, source, type */
   struct gl_client_namespace Namespaces[2][6]; /* source, type */
};

struct gl_debug_state
{
   GLDEBUGPROCARB Callback;
   GLvoid *CallbackData;
   GLboolean SyncOutput;
   GLboolean ApiErrors[API_ERROR_COUNT];
   GLboolean WinsysErrors[WINSYS_ERROR_COUNT];
   GLboolean ShaderErrors[SHADER_ERROR_COUNT];
   GLboolean OtherErrors[OTHER_ERROR_COUNT];
   struct gl_client_debug ClientIDs;
   struct gl_debug_msg Log[MAX_DEBUG_LOGGED_MESSAGES];
   GLint NumMessages;
   GLint NextMsg;
   GLint NextMsgLength; /* redundant, but copied here from Log[NextMsg].length
                           for the sake of the offsetof() code in get.c */
};

/**
 * Enum for the OpenGL APIs we know about and may support.
 */
typedef enum
{
   API_OPENGL,      /* legacy / compatibility contexts */
   API_OPENGLES,
   API_OPENGLES2,
   API_OPENGL_CORE,
} gl_api;

/**
 * Driver-specific state flags.
 *
 * These are or'd with gl_context::NewDriverState to notify a driver about
 * a state change. The driver sets the flags at context creation and
 * the meaning of the bits set is opaque to core Mesa.
 */
struct gl_driver_flags
{
   GLbitfield NewArray;             /**< Vertex array state */
};

struct gl_uniform_buffer_binding
{
   struct gl_buffer_object *BufferObject;
   /** Start of uniform block data in the buffer */
   GLintptr Offset;
   /** Size of data allowed to be referenced from the buffer (in bytes) */
   GLsizeiptr Size;
   /**
    * glBindBufferBase() indicates that the Size should be ignored and only
    * limited by the current size of the BufferObject.
    */
   GLboolean AutomaticSize;
};

/**
 * Mesa rendering context.
 *
 * This is the central context data structure for Mesa.  Almost all
 * OpenGL state is contained in this structure.
 * Think of this as a base class from which device drivers will derive
 * sub classes.
 *
 * The struct gl_context typedef names this structure.
 */
struct gl_context
{
   /** State possibly shared with other contexts in the address space */
   struct gl_shared_state *Shared;

   /** \name API function pointer tables */
   /*@{*/
   gl_api API;
   struct _glapi_table *Save;	/**< Display list save functions */
   struct _glapi_table *Exec;	/**< Execute functions */
   struct _glapi_table *CurrentDispatch;  /**< == Save or Exec !! */
   /*@}*/

   struct gl_config Visual;
   struct gl_framebuffer *DrawBuffer;	/**< buffer for writing */
   struct gl_framebuffer *ReadBuffer;	/**< buffer for reading */
   struct gl_framebuffer *WinSysDrawBuffer;  /**< set with MakeCurrent */
   struct gl_framebuffer *WinSysReadBuffer;  /**< set with MakeCurrent */

   /**
    * Device driver function pointer table
    */
   struct dd_function_table Driver;

   /** Core/Driver constants */
   struct gl_constants Const;

   /** \name The various 4x4 matrix stacks */
   /*@{*/
   struct gl_matrix_stack ModelviewMatrixStack;
   struct gl_matrix_stack ProjectionMatrixStack;
   struct gl_matrix_stack TextureMatrixStack[MAX_TEXTURE_UNITS];
   struct gl_matrix_stack ProgramMatrixStack[MAX_PROGRAM_MATRICES];
   struct gl_matrix_stack *CurrentStack; /**< Points to one of the above stacks */
   /*@}*/

   /** Combined modelview and projection matrix */
   GLmatrix _ModelProjectMatrix;

   /** \name Display lists */
   struct gl_dlist_state ListState;

   GLboolean ExecuteFlag;	/**< Execute GL commands? */
   GLboolean CompileFlag;	/**< Compile GL commands into display list? */

   /** Extension information */
   struct gl_extensions Extensions;

   /** GL version integer, for example 31 for GL 3.1, or 20 for GLES 2.0. */
   GLuint Version;
   char *VersionString;

   /** \name State attribute stack (for glPush/PopAttrib) */
   /*@{*/
   GLuint AttribStackDepth;
   struct gl_attrib_node *AttribStack[MAX_ATTRIB_STACK_DEPTH];
   /*@}*/

   /** \name Renderer attribute groups
    * 
    * We define a struct for each attribute group to make pushing and popping
    * attributes easy.  Also it's a good organization.
    */
   /*@{*/
   struct gl_accum_attrib	Accum;		/**< Accum buffer attributes */
   struct gl_colorbuffer_attrib	Color;		/**< Color buffer attributes */
   struct gl_current_attrib	Current;	/**< Current attributes */
   struct gl_depthbuffer_attrib	Depth;		/**< Depth buffer attributes */
   struct gl_eval_attrib	Eval;		/**< Eval attributes */
   struct gl_fog_attrib		Fog;		/**< Fog attributes */
   struct gl_hint_attrib	Hint;		/**< Hint attributes */
   struct gl_light_attrib	Light;		/**< Light attributes */
   struct gl_line_attrib	Line;		/**< Line attributes */
   struct gl_list_attrib	List;		/**< List attributes */
   struct gl_multisample_attrib Multisample;
   struct gl_pixel_attrib	Pixel;		/**< Pixel attributes */
   struct gl_point_attrib	Point;		/**< Point attributes */
   struct gl_polygon_attrib	Polygon;	/**< Polygon attributes */
   GLuint PolygonStipple[32];			/**< Polygon stipple */
   struct gl_scissor_attrib	Scissor;	/**< Scissor attributes */
   struct gl_stencil_attrib	Stencil;	/**< Stencil buffer attributes */
   struct gl_texture_attrib	Texture;	/**< Texture attributes */
   struct gl_transform_attrib	Transform;	/**< Transformation attributes */
   struct gl_viewport_attrib	Viewport;	/**< Viewport attributes */
   /*@}*/

   /** \name Client attribute stack */
   /*@{*/
   GLuint ClientAttribStackDepth;
   struct gl_attrib_node *ClientAttribStack[MAX_CLIENT_ATTRIB_STACK_DEPTH];
   /*@}*/

   /** \name Client attribute groups */
   /*@{*/
   struct gl_array_attrib	Array;	/**< Vertex arrays */
   struct gl_pixelstore_attrib	Pack;	/**< Pixel packing */
   struct gl_pixelstore_attrib	Unpack;	/**< Pixel unpacking */
   struct gl_pixelstore_attrib	DefaultPacking;	/**< Default params */
   /*@}*/

   /** \name Other assorted state (not pushed/popped on attribute stack) */
   /*@{*/
   struct gl_pixelmaps          PixelMaps;

   struct gl_evaluators EvalMap;   /**< All evaluators */
   struct gl_feedback   Feedback;  /**< Feedback */
   struct gl_selection  Select;    /**< Selection */

   struct gl_program_state Program;  /**< general program state */
   struct gl_vertex_program_state VertexProgram;
   struct gl_fragment_program_state FragmentProgram;
   struct gl_geometry_program_state GeometryProgram;
   struct gl_ati_fragment_shader_state ATIFragmentShader;

   struct gl_shader_state Shader; /**< GLSL shader object state */
   struct gl_shader_compiler_options ShaderCompilerOptions[MESA_SHADER_TYPES];

   struct gl_query_state Query;  /**< occlusion, timer queries */

   struct gl_transform_feedback_state TransformFeedback;

   struct gl_buffer_object *CopyReadBuffer; /**< GL_ARB_copy_buffer */
   struct gl_buffer_object *CopyWriteBuffer; /**< GL_ARB_copy_buffer */

   /**
    * Current GL_ARB_uniform_buffer_object binding referenced by
    * GL_UNIFORM_BUFFER target for glBufferData, glMapBuffer, etc.
    */
   struct gl_buffer_object *UniformBuffer;

   /**
    * Array of uniform buffers for GL_ARB_uniform_buffer_object and GL 3.1.
    * This is set up using glBindBufferRange() or glBindBufferBase().  They are
    * associated with uniform blocks by glUniformBlockBinding()'s state in the
    * shader program.
    */
   struct gl_uniform_buffer_binding *UniformBufferBindings;

   /*@}*/

   struct gl_meta_state *Meta;  /**< for "meta" operations */

   /* GL_EXT_framebuffer_object */
   struct gl_renderbuffer *CurrentRenderbuffer;

   GLenum ErrorValue;        /**< Last error code */

   /* GL_ARB_robustness */
   GLenum ResetStatus;

   /**
    * Recognize and silence repeated error debug messages in buggy apps.
    */
   const char *ErrorDebugFmtString;
   GLuint ErrorDebugCount;

   /* GL_ARB_debug_output */
   struct gl_debug_state Debug;

   GLenum RenderMode;        /**< either GL_RENDER, GL_SELECT, GL_FEEDBACK */
   GLbitfield NewState;      /**< bitwise-or of _NEW_* flags */
   GLbitfield NewDriverState;/**< bitwise-or of flags from DriverFlags */

   struct gl_driver_flags DriverFlags;

   GLboolean ViewportInitialized;  /**< has viewport size been initialized? */

   GLbitfield64 varying_vp_inputs;  /**< mask of VERT_BIT_* flags */

   /** \name Derived state */
   /*@{*/
   /** Bitwise-or of DD_* flags.  Note that this bitfield may be used before
    * state validation so they need to always be current.
    */
   GLbitfield _TriangleCaps;
   GLbitfield _ImageTransferState;/**< bitwise-or of IMAGE_*_BIT flags */
   GLfloat _EyeZDir[3];
   GLfloat _ModelViewInvScale;
   GLboolean _NeedEyeCoords;
   GLboolean _ForceEyeCoords; 

   GLuint TextureStateTimestamp; /**< detect changes to shared state */

   struct gl_list_extensions *ListExt; /**< driver dlist extensions */

   /** \name For debugging/development only */
   /*@{*/
   GLboolean FirstTimeCurrent;
   /*@}*/

   /** software compression/decompression supported or not */
   GLboolean Mesa_DXTn;

   GLboolean TextureFormatSupported[MESA_FORMAT_COUNT];

   /** 
    * Use dp4 (rather than mul/mad) instructions for position
    * transformation?
    */
   GLboolean mvp_with_dp4;

   GLboolean RasterDiscard;  /**< GL_RASTERIZER_DISCARD */

   /**
    * \name Hooks for module contexts.  
    *
    * These will eventually live in the driver or elsewhere.
    */
   /*@{*/
   void *swrast_context;
   void *swsetup_context;
   void *swtnl_context;
   void *swtnl_im;
   struct st_context *st;
   void *aelt_context;
   /*@}*/
};


#ifdef DEBUG
extern int MESA_VERBOSE;
extern int MESA_DEBUG_FLAGS;
# define MESA_FUNCTION __FUNCTION__
#else
# define MESA_VERBOSE 0
# define MESA_DEBUG_FLAGS 0
# define MESA_FUNCTION "a function"
# ifndef NDEBUG
#  define NDEBUG
# endif
#endif


/** The MESA_VERBOSE var is a bitmask of these flags */
enum _verbose
{
   VERBOSE_VARRAY		= 0x0001,
   VERBOSE_TEXTURE		= 0x0002,
   VERBOSE_MATERIAL		= 0x0004,
   VERBOSE_PIPELINE		= 0x0008,
   VERBOSE_DRIVER		= 0x0010,
   VERBOSE_STATE		= 0x0020,
   VERBOSE_API			= 0x0040,
   VERBOSE_DISPLAY_LIST		= 0x0100,
   VERBOSE_LIGHTING		= 0x0200,
   VERBOSE_PRIMS		= 0x0400,
   VERBOSE_VERTS		= 0x0800,
   VERBOSE_DISASSEM		= 0x1000,
   VERBOSE_DRAW                 = 0x2000,
   VERBOSE_SWAPBUFFERS          = 0x4000
};


/** The MESA_DEBUG_FLAGS var is a bitmask of these flags */
enum _debug
{
   DEBUG_SILENT                 = (1 << 0),
   DEBUG_ALWAYS_FLUSH		= (1 << 1),
   DEBUG_INCOMPLETE_TEXTURE     = (1 << 2),
   DEBUG_INCOMPLETE_FBO         = (1 << 3)
};



#ifdef __cplusplus
}
#endif

#endif /* MTYPES_H */
