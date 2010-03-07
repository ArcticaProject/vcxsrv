/*
 * Mesa 3-D graphics library
 * Version:  7.3
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
 * Copyright (C) 2008  VMware, Inc.  All Rights Reserved.
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
 * \file context.c
 * Mesa context/visual/framebuffer management functions.
 * \author Brian Paul
 */

/**
 * \mainpage Mesa Main Module
 *
 * \section MainIntroduction Introduction
 *
 * The Mesa Main module consists of all the files in the main/ directory.
 * Among the features of this module are:
 * <UL>
 * <LI> Structures to represent most GL state </LI>
 * <LI> State set/get functions </LI>
 * <LI> Display lists </LI>
 * <LI> Texture unit, object and image handling </LI>
 * <LI> Matrix and attribute stacks </LI>
 * </UL>
 *
 * Other modules are responsible for API dispatch, vertex transformation,
 * point/line/triangle setup, rasterization, vertex array caching,
 * vertex/fragment programs/shaders, etc.
 *
 *
 * \section AboutDoxygen About Doxygen
 *
 * If you're viewing this information as Doxygen-generated HTML you'll
 * see the documentation index at the top of this page.
 *
 * The first line lists the Mesa source code modules.
 * The second line lists the indexes available for viewing the documentation
 * for each module.
 *
 * Selecting the <b>Main page</b> link will display a summary of the module
 * (this page).
 *
 * Selecting <b>Data Structures</b> will list all C structures.
 *
 * Selecting the <b>File List</b> link will list all the source files in
 * the module.
 * Selecting a filename will show a list of all functions defined in that file.
 *
 * Selecting the <b>Data Fields</b> link will display a list of all
 * documented structure members.
 *
 * Selecting the <b>Globals</b> link will display a list
 * of all functions, structures, global variables and macros in the module.
 *
 */


#include "glheader.h"
#include "mfeatures.h"
#include "imports.h"
#include "accum.h"
#include "api_exec.h"
#include "arrayobj.h"
#include "attrib.h"
#include "blend.h"
#include "buffers.h"
#include "bufferobj.h"
#include "colortab.h"
#include "context.h"
#include "cpuinfo.h"
#include "debug.h"
#include "depth.h"
#include "dlist.h"
#include "eval.h"
#include "enums.h"
#include "extensions.h"
#include "fbobject.h"
#include "feedback.h"
#include "fog.h"
#include "framebuffer.h"
#include "get.h"
#include "histogram.h"
#include "hint.h"
#include "hash.h"
#include "light.h"
#include "lines.h"
#include "macros.h"
#include "matrix.h"
#include "multisample.h"
#include "pixel.h"
#include "pixelstore.h"
#include "points.h"
#include "polygon.h"
#include "queryobj.h"
#if FEATURE_ARB_sync
#include "syncobj.h"
#endif
#include "rastpos.h"
#include "remap.h"
#include "scissor.h"
#include "shared.h"
#include "simple_list.h"
#include "state.h"
#include "stencil.h"
#include "texcompress_s3tc.h"
#include "teximage.h"
#include "texobj.h"
#include "texstate.h"
#include "mtypes.h"
#include "varray.h"
#include "version.h"
#include "viewport.h"
#include "vtxfmt.h"
#include "glapi/glthread.h"
#include "glapi/glapitable.h"
#include "shader/program.h"
#include "shader/prog_print.h"
#include "shader/shader_api.h"
#if FEATURE_ATI_fragment_shader
#include "shader/atifragshader.h"
#endif
#if _HAVE_FULL_GL
#include "math/m_matrix.h"
#endif

#ifdef USE_SPARC_ASM
#include "sparc/sparc.h"
#endif

#ifndef MESA_VERBOSE
int MESA_VERBOSE = 0;
#endif

#ifndef MESA_DEBUG_FLAGS
int MESA_DEBUG_FLAGS = 0;
#endif


/* ubyte -> float conversion */
GLfloat _mesa_ubyte_to_float_color_tab[256];



/**
 * Swap buffers notification callback.
 * 
 * \param ctx GL context.
 *
 * Called by window system just before swapping buffers.
 * We have to finish any pending rendering.
 */
void
_mesa_notifySwapBuffers(__GLcontext *ctx)
{
   if (MESA_VERBOSE & VERBOSE_SWAPBUFFERS)
      _mesa_debug(ctx, "SwapBuffers\n");
   FLUSH_CURRENT( ctx, 0 );
   if (ctx->Driver.Flush) {
      ctx->Driver.Flush(ctx);
   }
}


/**********************************************************************/
/** \name GL Visual allocation/destruction                            */
/**********************************************************************/
/*@{*/

/**
 * Allocates a GLvisual structure and initializes it via
 * _mesa_initialize_visual().
 * 
 * \param rgbFlag GL_TRUE for RGB(A) mode, GL_FALSE for Color Index mode.
 * \param dbFlag double buffering
 * \param stereoFlag stereo buffer
 * \param depthBits requested bits per depth buffer value. Any value in [0, 32]
 * is acceptable but the actual depth type will be GLushort or GLuint as
 * needed.
 * \param stencilBits requested minimum bits per stencil buffer value
 * \param accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits number of bits per color component in accum buffer.
 * \param indexBits number of bits per pixel if \p rgbFlag is GL_FALSE
 * \param redBits number of bits per color component in frame buffer for RGB(A)
 * mode.  We always use 8 in core Mesa though.
 * \param greenBits same as above.
 * \param blueBits same as above.
 * \param alphaBits same as above.
 * \param numSamples not really used.
 * 
 * \return pointer to new GLvisual or NULL if requested parameters can't be
 * met.
 *
 * \note Need to add params for level and numAuxBuffers (at least)
 */
GLvisual *
_mesa_create_visual( GLboolean rgbFlag,
                     GLboolean dbFlag,
                     GLboolean stereoFlag,
                     GLint redBits,
                     GLint greenBits,
                     GLint blueBits,
                     GLint alphaBits,
                     GLint indexBits,
                     GLint depthBits,
                     GLint stencilBits,
                     GLint accumRedBits,
                     GLint accumGreenBits,
                     GLint accumBlueBits,
                     GLint accumAlphaBits,
                     GLint numSamples )
{
   GLvisual *vis = (GLvisual *) _mesa_calloc(sizeof(GLvisual));
   if (vis) {
      if (!_mesa_initialize_visual(vis, rgbFlag, dbFlag, stereoFlag,
                                   redBits, greenBits, blueBits, alphaBits,
                                   indexBits, depthBits, stencilBits,
                                   accumRedBits, accumGreenBits,
                                   accumBlueBits, accumAlphaBits,
                                   numSamples)) {
         _mesa_free(vis);
         return NULL;
      }
   }
   return vis;
}

/**
 * Makes some sanity checks and fills in the fields of the
 * GLvisual object with the given parameters.  If the caller needs
 * to set additional fields, he should just probably init the whole GLvisual
 * object himself.
 * \return GL_TRUE on success, or GL_FALSE on failure.
 *
 * \sa _mesa_create_visual() above for the parameter description.
 */
GLboolean
_mesa_initialize_visual( GLvisual *vis,
                         GLboolean rgbFlag,
                         GLboolean dbFlag,
                         GLboolean stereoFlag,
                         GLint redBits,
                         GLint greenBits,
                         GLint blueBits,
                         GLint alphaBits,
                         GLint indexBits,
                         GLint depthBits,
                         GLint stencilBits,
                         GLint accumRedBits,
                         GLint accumGreenBits,
                         GLint accumBlueBits,
                         GLint accumAlphaBits,
                         GLint numSamples )
{
   assert(vis);

   if (depthBits < 0 || depthBits > 32) {
      return GL_FALSE;
   }
   if (stencilBits < 0 || stencilBits > STENCIL_BITS) {
      return GL_FALSE;
   }
   assert(accumRedBits >= 0);
   assert(accumGreenBits >= 0);
   assert(accumBlueBits >= 0);
   assert(accumAlphaBits >= 0);

   vis->rgbMode          = rgbFlag;
   vis->doubleBufferMode = dbFlag;
   vis->stereoMode       = stereoFlag;

   vis->redBits          = redBits;
   vis->greenBits        = greenBits;
   vis->blueBits         = blueBits;
   vis->alphaBits        = alphaBits;
   vis->rgbBits          = redBits + greenBits + blueBits;

   vis->indexBits      = indexBits;
   vis->depthBits      = depthBits;
   vis->stencilBits    = stencilBits;

   vis->accumRedBits   = accumRedBits;
   vis->accumGreenBits = accumGreenBits;
   vis->accumBlueBits  = accumBlueBits;
   vis->accumAlphaBits = accumAlphaBits;

   vis->haveAccumBuffer   = accumRedBits > 0;
   vis->haveDepthBuffer   = depthBits > 0;
   vis->haveStencilBuffer = stencilBits > 0;

   vis->numAuxBuffers = 0;
   vis->level = 0;
   vis->pixmapMode = 0;
   vis->sampleBuffers = numSamples > 0 ? 1 : 0;
   vis->samples = numSamples;

   return GL_TRUE;
}


/**
 * Destroy a visual and free its memory.
 *
 * \param vis visual.
 * 
 * Frees the visual structure.
 */
void
_mesa_destroy_visual( GLvisual *vis )
{
   _mesa_free(vis);
}

/*@}*/


/**********************************************************************/
/** \name Context allocation, initialization, destroying
 *
 * The purpose of the most initialization functions here is to provide the
 * default state values according to the OpenGL specification.
 */
/**********************************************************************/
/*@{*/


/**
 * This is lame.  gdb only seems to recognize enum types that are
 * actually used somewhere.  We want to be able to print/use enum
 * values such as TEXTURE_2D_INDEX in gdb.  But we don't actually use
 * the gl_texture_index type anywhere.  Thus, this lame function.
 */
static void
dummy_enum_func(void)
{
   gl_buffer_index bi;
   gl_colortable_index ci;
   gl_face_index fi;
   gl_frag_attrib fa;
   gl_frag_result fr;
   gl_texture_index ti;
   gl_vert_attrib va;
   gl_vert_result vr;

   (void) bi;
   (void) ci;
   (void) fi;
   (void) fa;
   (void) fr;
   (void) ti;
   (void) va;
   (void) vr;
}


/**
 * One-time initialization mutex lock.
 *
 * \sa Used by one_time_init().
 */
_glthread_DECLARE_STATIC_MUTEX(OneTimeLock);

/**
 * Calls all the various one-time-init functions in Mesa.
 *
 * While holding a global mutex lock, calls several initialization functions,
 * and sets the glapi callbacks if the \c MESA_DEBUG environment variable is
 * defined.
 *
 * \sa _math_init().
 */
static void
one_time_init( GLcontext *ctx )
{
   static GLboolean alreadyCalled = GL_FALSE;
   (void) ctx;
   _glthread_LOCK_MUTEX(OneTimeLock);
   if (!alreadyCalled) {
      GLuint i;

      /* do some implementation tests */
      assert( sizeof(GLbyte) == 1 );
      assert( sizeof(GLubyte) == 1 );
      assert( sizeof(GLshort) == 2 );
      assert( sizeof(GLushort) == 2 );
      assert( sizeof(GLint) == 4 );
      assert( sizeof(GLuint) == 4 );

      _mesa_get_cpu_features();

      _mesa_init_remap_table();

      _mesa_init_sqrt_table();

      for (i = 0; i < 256; i++) {
         _mesa_ubyte_to_float_color_tab[i] = (float) i / 255.0F;
      }

      if (_mesa_getenv("MESA_DEBUG")) {
         _glapi_noop_enable_warnings(GL_TRUE);
         _glapi_set_warning_func( (_glapi_warning_func) _mesa_warning );
      }
      else {
         _glapi_noop_enable_warnings(GL_FALSE);
      }

#if defined(DEBUG) && defined(__DATE__) && defined(__TIME__)
      _mesa_debug(ctx, "Mesa %s DEBUG build %s %s\n",
                  MESA_VERSION_STRING, __DATE__, __TIME__);
#endif

      alreadyCalled = GL_TRUE;
   }
   _glthread_UNLOCK_MUTEX(OneTimeLock);

   dummy_enum_func();
}


/**
 * Initialize fields of gl_current_attrib (aka ctx->Current.*)
 */
static void
_mesa_init_current(GLcontext *ctx)
{
   GLuint i;

   /* Init all to (0,0,0,1) */
   for (i = 0; i < Elements(ctx->Current.Attrib); i++) {
      ASSIGN_4V( ctx->Current.Attrib[i], 0.0, 0.0, 0.0, 1.0 );
   }

   /* redo special cases: */
   ASSIGN_4V( ctx->Current.Attrib[VERT_ATTRIB_WEIGHT], 1.0, 0.0, 0.0, 0.0 );
   ASSIGN_4V( ctx->Current.Attrib[VERT_ATTRIB_NORMAL], 0.0, 0.0, 1.0, 1.0 );
   ASSIGN_4V( ctx->Current.Attrib[VERT_ATTRIB_COLOR0], 1.0, 1.0, 1.0, 1.0 );
   ASSIGN_4V( ctx->Current.Attrib[VERT_ATTRIB_COLOR1], 0.0, 0.0, 0.0, 1.0 );
   ASSIGN_4V( ctx->Current.Attrib[VERT_ATTRIB_COLOR_INDEX], 1.0, 0.0, 0.0, 1.0 );
   ASSIGN_4V( ctx->Current.Attrib[VERT_ATTRIB_EDGEFLAG], 1.0, 0.0, 0.0, 1.0 );
}


/**
 * Init vertex/fragment program limits.
 * Important: drivers should override these with actual limits.
 */
static void
init_program_limits(GLenum type, struct gl_program_constants *prog)
{
   prog->MaxInstructions = MAX_PROGRAM_INSTRUCTIONS;
   prog->MaxAluInstructions = MAX_PROGRAM_INSTRUCTIONS;
   prog->MaxTexInstructions = MAX_PROGRAM_INSTRUCTIONS;
   prog->MaxTexIndirections = MAX_PROGRAM_INSTRUCTIONS;
   prog->MaxTemps = MAX_PROGRAM_TEMPS;
   prog->MaxEnvParams = MAX_PROGRAM_ENV_PARAMS;
   prog->MaxLocalParams = MAX_PROGRAM_LOCAL_PARAMS;
   prog->MaxUniformComponents = 4 * MAX_UNIFORMS;

   if (type == GL_VERTEX_PROGRAM_ARB) {
      prog->MaxParameters = MAX_VERTEX_PROGRAM_PARAMS;
      prog->MaxAttribs = MAX_NV_VERTEX_PROGRAM_INPUTS;
      prog->MaxAddressRegs = MAX_VERTEX_PROGRAM_ADDRESS_REGS;
   }
   else {
      prog->MaxParameters = MAX_NV_FRAGMENT_PROGRAM_PARAMS;
      prog->MaxAttribs = MAX_NV_FRAGMENT_PROGRAM_INPUTS;
      prog->MaxAddressRegs = MAX_FRAGMENT_PROGRAM_ADDRESS_REGS;
   }

   /* Set the native limits to zero.  This implies that there is no native
    * support for shaders.  Let the drivers fill in the actual values.
    */
   prog->MaxNativeInstructions = 0;
   prog->MaxNativeAluInstructions = 0;
   prog->MaxNativeTexInstructions = 0;
   prog->MaxNativeTexIndirections = 0;
   prog->MaxNativeAttribs = 0;
   prog->MaxNativeTemps = 0;
   prog->MaxNativeAddressRegs = 0;
   prog->MaxNativeParameters = 0;
}


/**
 * Initialize fields of gl_constants (aka ctx->Const.*).
 * Use defaults from config.h.  The device drivers will often override
 * some of these values (such as number of texture units).
 */
static void 
_mesa_init_constants(GLcontext *ctx)
{
   assert(ctx);

   assert(MAX_TEXTURE_LEVELS >= MAX_3D_TEXTURE_LEVELS);
   assert(MAX_TEXTURE_LEVELS >= MAX_CUBE_TEXTURE_LEVELS);

   /* Max texture size should be <= max viewport size (render to texture) */
   assert((1 << (MAX_TEXTURE_LEVELS - 1)) <= MAX_WIDTH);

   /* Constants, may be overriden (usually only reduced) by device drivers */
   ctx->Const.MaxTextureLevels = MAX_TEXTURE_LEVELS;
   ctx->Const.Max3DTextureLevels = MAX_3D_TEXTURE_LEVELS;
   ctx->Const.MaxCubeTextureLevels = MAX_CUBE_TEXTURE_LEVELS;
   ctx->Const.MaxTextureRectSize = MAX_TEXTURE_RECT_SIZE;
   ctx->Const.MaxArrayTextureLayers = MAX_ARRAY_TEXTURE_LAYERS;
   ctx->Const.MaxTextureCoordUnits = MAX_TEXTURE_COORD_UNITS;
   ctx->Const.MaxTextureImageUnits = MAX_TEXTURE_IMAGE_UNITS;
   ctx->Const.MaxTextureUnits = MIN2(ctx->Const.MaxTextureCoordUnits,
                                     ctx->Const.MaxTextureImageUnits);
   ctx->Const.MaxTextureMaxAnisotropy = MAX_TEXTURE_MAX_ANISOTROPY;
   ctx->Const.MaxTextureLodBias = MAX_TEXTURE_LOD_BIAS;
   ctx->Const.MaxArrayLockSize = MAX_ARRAY_LOCK_SIZE;
   ctx->Const.SubPixelBits = SUB_PIXEL_BITS;
   ctx->Const.MinPointSize = MIN_POINT_SIZE;
   ctx->Const.MaxPointSize = MAX_POINT_SIZE;
   ctx->Const.MinPointSizeAA = MIN_POINT_SIZE;
   ctx->Const.MaxPointSizeAA = MAX_POINT_SIZE;
   ctx->Const.PointSizeGranularity = (GLfloat) POINT_SIZE_GRANULARITY;
   ctx->Const.MinLineWidth = MIN_LINE_WIDTH;
   ctx->Const.MaxLineWidth = MAX_LINE_WIDTH;
   ctx->Const.MinLineWidthAA = MIN_LINE_WIDTH;
   ctx->Const.MaxLineWidthAA = MAX_LINE_WIDTH;
   ctx->Const.LineWidthGranularity = (GLfloat) LINE_WIDTH_GRANULARITY;
   ctx->Const.MaxColorTableSize = MAX_COLOR_TABLE_SIZE;
   ctx->Const.MaxConvolutionWidth = MAX_CONVOLUTION_WIDTH;
   ctx->Const.MaxConvolutionHeight = MAX_CONVOLUTION_HEIGHT;
   ctx->Const.MaxClipPlanes = MAX_CLIP_PLANES;
   ctx->Const.MaxLights = MAX_LIGHTS;
   ctx->Const.MaxShininess = 128.0;
   ctx->Const.MaxSpotExponent = 128.0;
   ctx->Const.MaxViewportWidth = MAX_WIDTH;
   ctx->Const.MaxViewportHeight = MAX_HEIGHT;
#if FEATURE_ARB_vertex_program
   init_program_limits(GL_VERTEX_PROGRAM_ARB, &ctx->Const.VertexProgram);
#endif
#if FEATURE_ARB_fragment_program
   init_program_limits(GL_FRAGMENT_PROGRAM_ARB, &ctx->Const.FragmentProgram);
#endif
   ctx->Const.MaxProgramMatrices = MAX_PROGRAM_MATRICES;
   ctx->Const.MaxProgramMatrixStackDepth = MAX_PROGRAM_MATRIX_STACK_DEPTH;

   /* CheckArrayBounds is overriden by drivers/x11 for X server */
   ctx->Const.CheckArrayBounds = GL_FALSE;

   /* GL_ARB_draw_buffers */
   ctx->Const.MaxDrawBuffers = MAX_DRAW_BUFFERS;

#if FEATURE_EXT_framebuffer_object
   ctx->Const.MaxColorAttachments = MAX_COLOR_ATTACHMENTS;
   ctx->Const.MaxRenderbufferSize = MAX_WIDTH;
#endif

#if FEATURE_ARB_vertex_shader
   ctx->Const.MaxVertexTextureImageUnits = MAX_VERTEX_TEXTURE_IMAGE_UNITS;
   ctx->Const.MaxVarying = MAX_VARYING;
#endif

   /* GL_ARB_framebuffer_object */
   ctx->Const.MaxSamples = 0;

   /* GL_ARB_sync */
   ctx->Const.MaxServerWaitTimeout = (GLuint64) ~0;

   /* GL_ATI_envmap_bumpmap */
   ctx->Const.SupportedBumpUnits = SUPPORTED_ATI_BUMP_UNITS;

   /* GL_EXT_provoking_vertex */
   ctx->Const.QuadsFollowProvokingVertexConvention = GL_TRUE;

   /* sanity checks */
   ASSERT(ctx->Const.MaxTextureUnits == MIN2(ctx->Const.MaxTextureImageUnits,
                                             ctx->Const.MaxTextureCoordUnits));
   ASSERT(ctx->Const.FragmentProgram.MaxLocalParams <= MAX_PROGRAM_LOCAL_PARAMS);
   ASSERT(ctx->Const.VertexProgram.MaxLocalParams <= MAX_PROGRAM_LOCAL_PARAMS);

   ASSERT(MAX_NV_FRAGMENT_PROGRAM_TEMPS <= MAX_PROGRAM_TEMPS);
   ASSERT(MAX_NV_VERTEX_PROGRAM_TEMPS <= MAX_PROGRAM_TEMPS);
   ASSERT(MAX_NV_VERTEX_PROGRAM_INPUTS <= VERT_ATTRIB_MAX);
   ASSERT(MAX_NV_VERTEX_PROGRAM_OUTPUTS <= VERT_RESULT_MAX);

   /* check that we don't exceed the size of various bitfields */
   ASSERT(VERT_RESULT_MAX <=
	  (8 * sizeof(ctx->VertexProgram._Current->Base.OutputsWritten)));
   ASSERT(FRAG_ATTRIB_MAX <=
	  (8 * sizeof(ctx->FragmentProgram._Current->Base.InputsRead)));
}


/**
 * Do some sanity checks on the limits/constants for the given context.
 * Only called the first time a context is bound.
 */
static void
check_context_limits(GLcontext *ctx)
{
   /* Many context limits/constants are limited by the size of
    * internal arrays.
    */
   assert(ctx->Const.MaxTextureImageUnits <= MAX_TEXTURE_IMAGE_UNITS);
   assert(ctx->Const.MaxTextureCoordUnits <= MAX_TEXTURE_COORD_UNITS);
   assert(ctx->Const.MaxTextureUnits <= MAX_TEXTURE_IMAGE_UNITS);
   assert(ctx->Const.MaxTextureUnits <= MAX_TEXTURE_COORD_UNITS);

   /* number of coord units cannot be greater than number of image units */
   assert(ctx->Const.MaxTextureCoordUnits <= ctx->Const.MaxTextureImageUnits);

   assert(ctx->Const.MaxTextureLevels <= MAX_TEXTURE_LEVELS);
   assert(ctx->Const.Max3DTextureLevels <= MAX_3D_TEXTURE_LEVELS);
   assert(ctx->Const.MaxCubeTextureLevels <= MAX_CUBE_TEXTURE_LEVELS);
   assert(ctx->Const.MaxTextureRectSize <= MAX_TEXTURE_RECT_SIZE);

   /* make sure largest texture image is <= MAX_WIDTH in size */
   assert((1 << (ctx->Const.MaxTextureLevels - 1)) <= MAX_WIDTH);
   assert((1 << (ctx->Const.MaxCubeTextureLevels - 1)) <= MAX_WIDTH);
   assert((1 << (ctx->Const.Max3DTextureLevels - 1)) <= MAX_WIDTH);

   assert(ctx->Const.MaxViewportWidth <= MAX_WIDTH);
   assert(ctx->Const.MaxViewportHeight <= MAX_WIDTH);

   assert(ctx->Const.MaxDrawBuffers <= MAX_DRAW_BUFFERS);

   /* XXX probably add more tests */
}


/**
 * Initialize the attribute groups in a GL context.
 *
 * \param ctx GL context.
 *
 * Initializes all the attributes, calling the respective <tt>init*</tt>
 * functions for the more complex data structures.
 */
static GLboolean
init_attrib_groups(GLcontext *ctx)
{
   assert(ctx);

   /* Constants */
   _mesa_init_constants( ctx );

   /* Extensions */
   _mesa_init_extensions( ctx );

   /* Attribute Groups */
   _mesa_init_accum( ctx );
   _mesa_init_attrib( ctx );
   _mesa_init_buffer_objects( ctx );
   _mesa_init_color( ctx );
   _mesa_init_colortables( ctx );
   _mesa_init_current( ctx );
   _mesa_init_depth( ctx );
   _mesa_init_debug( ctx );
   _mesa_init_display_list( ctx );
   _mesa_init_eval( ctx );
   _mesa_init_fbobjects( ctx );
   _mesa_init_feedback( ctx );
   _mesa_init_fog( ctx );
   _mesa_init_histogram( ctx );
   _mesa_init_hint( ctx );
   _mesa_init_line( ctx );
   _mesa_init_lighting( ctx );
   _mesa_init_matrix( ctx );
   _mesa_init_multisample( ctx );
   _mesa_init_pixel( ctx );
   _mesa_init_pixelstore( ctx );
   _mesa_init_point( ctx );
   _mesa_init_polygon( ctx );
   _mesa_init_program( ctx );
   _mesa_init_queryobj( ctx );
#if FEATURE_ARB_sync
   _mesa_init_sync( ctx );
#endif
   _mesa_init_rastpos( ctx );
   _mesa_init_scissor( ctx );
   _mesa_init_shader_state( ctx );
   _mesa_init_stencil( ctx );
   _mesa_init_transform( ctx );
   _mesa_init_varray( ctx );
   _mesa_init_viewport( ctx );

   if (!_mesa_init_texture( ctx ))
      return GL_FALSE;

   _mesa_init_texture_s3tc( ctx );

   /* Miscellaneous */
   ctx->NewState = _NEW_ALL;
   ctx->ErrorValue = (GLenum) GL_NO_ERROR;
   ctx->varying_vp_inputs = ~0;

   return GL_TRUE;
}


/**
 * Update default objects in a GL context with respect to shared state.
 *
 * \param ctx GL context.
 *
 * Removes references to old default objects, (texture objects, program
 * objects, etc.) and changes to reference those from the current shared
 * state.
 */
static GLboolean
update_default_objects(GLcontext *ctx)
{
   assert(ctx);

   _mesa_update_default_objects_program(ctx);
   _mesa_update_default_objects_texture(ctx);
   _mesa_update_default_objects_buffer_objects(ctx);

   return GL_TRUE;
}


/**
 * This is the default function we plug into all dispatch table slots
 * This helps prevents a segfault when someone calls a GL function without
 * first checking if the extension's supported.
 */
static int
generic_nop(void)
{
   _mesa_warning(NULL, "User called no-op dispatch function (an unsupported extension function?)");
   return 0;
}


/**
 * Allocate and initialize a new dispatch table.
 */
static struct _glapi_table *
alloc_dispatch_table(void)
{
   /* Find the larger of Mesa's dispatch table and libGL's dispatch table.
    * In practice, this'll be the same for stand-alone Mesa.  But for DRI
    * Mesa we do this to accomodate different versions of libGL and various
    * DRI drivers.
    */
   GLint numEntries = MAX2(_glapi_get_dispatch_table_size(),
                           sizeof(struct _glapi_table) / sizeof(_glapi_proc));
   struct _glapi_table *table =
      (struct _glapi_table *) _mesa_malloc(numEntries * sizeof(_glapi_proc));
   if (table) {
      _glapi_proc *entry = (_glapi_proc *) table;
      GLint i;
      for (i = 0; i < numEntries; i++) {
         entry[i] = (_glapi_proc) generic_nop;
      }
   }
   return table;
}


/**
 * Initialize a GLcontext struct (rendering context).
 *
 * This includes allocating all the other structs and arrays which hang off of
 * the context by pointers.
 * Note that the driver needs to pass in its dd_function_table here since
 * we need to at least call driverFunctions->NewTextureObject to create the
 * default texture objects.
 * 
 * Called by _mesa_create_context().
 *
 * Performs the imports and exports callback tables initialization, and
 * miscellaneous one-time initializations. If no shared context is supplied one
 * is allocated, and increase its reference count.  Setups the GL API dispatch
 * tables.  Initialize the TNL module. Sets the maximum Z buffer depth.
 * Finally queries the \c MESA_DEBUG and \c MESA_VERBOSE environment variables
 * for debug flags.
 *
 * \param ctx the context to initialize
 * \param visual describes the visual attributes for this context
 * \param share_list points to context to share textures, display lists,
 *        etc with, or NULL
 * \param driverFunctions table of device driver functions for this context
 *        to use
 * \param driverContext pointer to driver-specific context data
 */
GLboolean
_mesa_initialize_context(GLcontext *ctx,
                         const GLvisual *visual,
                         GLcontext *share_list,
                         const struct dd_function_table *driverFunctions,
                         void *driverContext)
{
   struct gl_shared_state *shared;

   /*ASSERT(driverContext);*/
   assert(driverFunctions->NewTextureObject);
   assert(driverFunctions->FreeTexImageData);

   /* misc one-time initializations */
   one_time_init(ctx);

   ctx->Visual = *visual;
   ctx->DrawBuffer = NULL;
   ctx->ReadBuffer = NULL;
   ctx->WinSysDrawBuffer = NULL;
   ctx->WinSysReadBuffer = NULL;

   /* Plug in driver functions and context pointer here.
    * This is important because when we call alloc_shared_state() below
    * we'll call ctx->Driver.NewTextureObject() to create the default
    * textures.
    */
   ctx->Driver = *driverFunctions;
   ctx->DriverCtx = driverContext;

   if (share_list) {
      /* share state with another context */
      shared = share_list->Shared;
   }
   else {
      /* allocate new, unshared state */
      shared = _mesa_alloc_shared_state(ctx);
      if (!shared)
         return GL_FALSE;
   }

   _glthread_LOCK_MUTEX(shared->Mutex);
   ctx->Shared = shared;
   shared->RefCount++;
   _glthread_UNLOCK_MUTEX(shared->Mutex);

   if (!init_attrib_groups( ctx )) {
      _mesa_free_shared_state(ctx, ctx->Shared);
      return GL_FALSE;
   }

   /* setup the API dispatch tables */
   ctx->Exec = alloc_dispatch_table();
   ctx->Save = alloc_dispatch_table();
   if (!ctx->Exec || !ctx->Save) {
      _mesa_free_shared_state(ctx, ctx->Shared);
      if (ctx->Exec)
         _mesa_free(ctx->Exec);
      return GL_FALSE;
   }
#if FEATURE_dispatch
   _mesa_init_exec_table(ctx->Exec);
#endif
   ctx->CurrentDispatch = ctx->Exec;

#if FEATURE_dlist
   _mesa_init_save_table(ctx->Save);
   _mesa_install_save_vtxfmt( ctx, &ctx->ListState.ListVtxfmt );
#endif

   /* Neutral tnl module stuff */
   _mesa_init_exec_vtxfmt( ctx ); 
   ctx->TnlModule.Current = NULL;
   ctx->TnlModule.SwapCount = 0;

   ctx->FragmentProgram._MaintainTexEnvProgram
      = (_mesa_getenv("MESA_TEX_PROG") != NULL);

   ctx->VertexProgram._MaintainTnlProgram
      = (_mesa_getenv("MESA_TNL_PROG") != NULL);
   if (ctx->VertexProgram._MaintainTnlProgram) {
      /* this is required... */
      ctx->FragmentProgram._MaintainTexEnvProgram = GL_TRUE;
   }

#ifdef FEATURE_extra_context_init
   _mesa_initialize_context_extra(ctx);
#endif

   ctx->FirstTimeCurrent = GL_TRUE;

   return GL_TRUE;
}


/**
 * Allocate and initialize a GLcontext structure.
 * Note that the driver needs to pass in its dd_function_table here since
 * we need to at least call driverFunctions->NewTextureObject to initialize
 * the rendering context.
 *
 * \param visual a GLvisual pointer (we copy the struct contents)
 * \param share_list another context to share display lists with or NULL
 * \param driverFunctions points to the dd_function_table into which the
 *        driver has plugged in all its special functions.
 * \param driverContext points to the device driver's private context state
 * 
 * \return pointer to a new __GLcontextRec or NULL if error.
 */
GLcontext *
_mesa_create_context(const GLvisual *visual,
                     GLcontext *share_list,
                     const struct dd_function_table *driverFunctions,
                     void *driverContext)
{
   GLcontext *ctx;

   ASSERT(visual);
   /*ASSERT(driverContext);*/

   ctx = (GLcontext *) _mesa_calloc(sizeof(GLcontext));
   if (!ctx)
      return NULL;

   if (_mesa_initialize_context(ctx, visual, share_list,
                                driverFunctions, driverContext)) {
      return ctx;
   }
   else {
      _mesa_free(ctx);
      return NULL;
   }
}


/**
 * Free the data associated with the given context.
 * 
 * But doesn't free the GLcontext struct itself.
 *
 * \sa _mesa_initialize_context() and init_attrib_groups().
 */
void
_mesa_free_context_data( GLcontext *ctx )
{
   GLint RefCount;

   if (!_mesa_get_current_context()){
      /* No current context, but we may need one in order to delete
       * texture objs, etc.  So temporarily bind the context now.
       */
      _mesa_make_current(ctx, NULL, NULL);
   }

   /* unreference WinSysDraw/Read buffers */
   _mesa_reference_framebuffer(&ctx->WinSysDrawBuffer, NULL);
   _mesa_reference_framebuffer(&ctx->WinSysReadBuffer, NULL);
   _mesa_reference_framebuffer(&ctx->DrawBuffer, NULL);
   _mesa_reference_framebuffer(&ctx->ReadBuffer, NULL);

   _mesa_reference_vertprog(ctx, &ctx->VertexProgram.Current, NULL);
   _mesa_reference_vertprog(ctx, &ctx->VertexProgram._Current, NULL);
   _mesa_reference_vertprog(ctx, &ctx->VertexProgram._TnlProgram, NULL);

   _mesa_reference_fragprog(ctx, &ctx->FragmentProgram.Current, NULL);
   _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._Current, NULL);
   _mesa_reference_fragprog(ctx, &ctx->FragmentProgram._TexEnvProgram, NULL);

   _mesa_free_attrib_data(ctx);
   _mesa_free_lighting_data( ctx );
   _mesa_free_eval_data( ctx );
   _mesa_free_texture_data( ctx );
   _mesa_free_matrix_data( ctx );
   _mesa_free_viewport_data( ctx );
   _mesa_free_colortables_data( ctx );
   _mesa_free_program_data(ctx);
   _mesa_free_shader_state(ctx);
   _mesa_free_queryobj_data(ctx);
#if FEATURE_ARB_sync
   _mesa_free_sync_data(ctx);
#endif
   _mesa_free_varray_data(ctx);

   _mesa_delete_array_object(ctx, ctx->Array.DefaultArrayObj);

#if FEATURE_ARB_pixel_buffer_object
   _mesa_reference_buffer_object(ctx, &ctx->Pack.BufferObj, NULL);
   _mesa_reference_buffer_object(ctx, &ctx->Unpack.BufferObj, NULL);
#endif

#if FEATURE_ARB_vertex_buffer_object
   _mesa_reference_buffer_object(ctx, &ctx->Array.ArrayBufferObj, NULL);
   _mesa_reference_buffer_object(ctx, &ctx->Array.ElementArrayBufferObj, NULL);
#endif

   /* free dispatch tables */
   _mesa_free(ctx->Exec);
   _mesa_free(ctx->Save);

   /* Shared context state (display lists, textures, etc) */
   _glthread_LOCK_MUTEX(ctx->Shared->Mutex);
   RefCount = --ctx->Shared->RefCount;
   _glthread_UNLOCK_MUTEX(ctx->Shared->Mutex);
   assert(RefCount >= 0);
   if (RefCount == 0) {
      /* free shared state */
      _mesa_free_shared_state( ctx, ctx->Shared );
   }

   /* needs to be after freeing shared state */
   _mesa_free_display_list_data(ctx);

   if (ctx->Extensions.String)
      _mesa_free((void *) ctx->Extensions.String);

   /* unbind the context if it's currently bound */
   if (ctx == _mesa_get_current_context()) {
      _mesa_make_current(NULL, NULL, NULL);
   }
}


/**
 * Destroy a GLcontext structure.
 *
 * \param ctx GL context.
 * 
 * Calls _mesa_free_context_data() and frees the GLcontext structure itself.
 */
void
_mesa_destroy_context( GLcontext *ctx )
{
   if (ctx) {
      _mesa_free_context_data(ctx);
      _mesa_free( (void *) ctx );
   }
}


#if _HAVE_FULL_GL
/**
 * Copy attribute groups from one context to another.
 * 
 * \param src source context
 * \param dst destination context
 * \param mask bitwise OR of GL_*_BIT flags
 *
 * According to the bits specified in \p mask, copies the corresponding
 * attributes from \p src into \p dst.  For many of the attributes a simple \c
 * memcpy is not enough due to the existence of internal pointers in their data
 * structures.
 */
void
_mesa_copy_context( const GLcontext *src, GLcontext *dst, GLuint mask )
{
   if (mask & GL_ACCUM_BUFFER_BIT) {
      /* OK to memcpy */
      dst->Accum = src->Accum;
   }
   if (mask & GL_COLOR_BUFFER_BIT) {
      /* OK to memcpy */
      dst->Color = src->Color;
   }
   if (mask & GL_CURRENT_BIT) {
      /* OK to memcpy */
      dst->Current = src->Current;
   }
   if (mask & GL_DEPTH_BUFFER_BIT) {
      /* OK to memcpy */
      dst->Depth = src->Depth;
   }
   if (mask & GL_ENABLE_BIT) {
      /* no op */
   }
   if (mask & GL_EVAL_BIT) {
      /* OK to memcpy */
      dst->Eval = src->Eval;
   }
   if (mask & GL_FOG_BIT) {
      /* OK to memcpy */
      dst->Fog = src->Fog;
   }
   if (mask & GL_HINT_BIT) {
      /* OK to memcpy */
      dst->Hint = src->Hint;
   }
   if (mask & GL_LIGHTING_BIT) {
      GLuint i;
      /* begin with memcpy */
      dst->Light = src->Light;
      /* fixup linked lists to prevent pointer insanity */
      make_empty_list( &(dst->Light.EnabledList) );
      for (i = 0; i < MAX_LIGHTS; i++) {
         if (dst->Light.Light[i].Enabled) {
            insert_at_tail(&(dst->Light.EnabledList), &(dst->Light.Light[i]));
         }
      }
   }
   if (mask & GL_LINE_BIT) {
      /* OK to memcpy */
      dst->Line = src->Line;
   }
   if (mask & GL_LIST_BIT) {
      /* OK to memcpy */
      dst->List = src->List;
   }
   if (mask & GL_PIXEL_MODE_BIT) {
      /* OK to memcpy */
      dst->Pixel = src->Pixel;
   }
   if (mask & GL_POINT_BIT) {
      /* OK to memcpy */
      dst->Point = src->Point;
   }
   if (mask & GL_POLYGON_BIT) {
      /* OK to memcpy */
      dst->Polygon = src->Polygon;
   }
   if (mask & GL_POLYGON_STIPPLE_BIT) {
      /* Use loop instead of MEMCPY due to problem with Portland Group's
       * C compiler.  Reported by John Stone.
       */
      GLuint i;
      for (i = 0; i < 32; i++) {
         dst->PolygonStipple[i] = src->PolygonStipple[i];
      }
   }
   if (mask & GL_SCISSOR_BIT) {
      /* OK to memcpy */
      dst->Scissor = src->Scissor;
   }
   if (mask & GL_STENCIL_BUFFER_BIT) {
      /* OK to memcpy */
      dst->Stencil = src->Stencil;
   }
   if (mask & GL_TEXTURE_BIT) {
      /* Cannot memcpy because of pointers */
      _mesa_copy_texture_state(src, dst);
   }
   if (mask & GL_TRANSFORM_BIT) {
      /* OK to memcpy */
      dst->Transform = src->Transform;
   }
   if (mask & GL_VIEWPORT_BIT) {
      /* Cannot use memcpy, because of pointers in GLmatrix _WindowMap */
      dst->Viewport.X = src->Viewport.X;
      dst->Viewport.Y = src->Viewport.Y;
      dst->Viewport.Width = src->Viewport.Width;
      dst->Viewport.Height = src->Viewport.Height;
      dst->Viewport.Near = src->Viewport.Near;
      dst->Viewport.Far = src->Viewport.Far;
      _math_matrix_copy(&dst->Viewport._WindowMap, &src->Viewport._WindowMap);
   }

   /* XXX FIXME:  Call callbacks?
    */
   dst->NewState = _NEW_ALL;
}
#endif


/**
 * Check if the given context can render into the given framebuffer
 * by checking visual attributes.
 *
 * Most of these tests could go away because Mesa is now pretty flexible
 * in terms of mixing rendering contexts with framebuffers.  As long
 * as RGB vs. CI mode agree, we're probably good.
 *
 * \return GL_TRUE if compatible, GL_FALSE otherwise.
 */
static GLboolean 
check_compatible(const GLcontext *ctx, const GLframebuffer *buffer)
{
   const GLvisual *ctxvis = &ctx->Visual;
   const GLvisual *bufvis = &buffer->Visual;

   if (ctxvis == bufvis)
      return GL_TRUE;

   if (ctxvis->rgbMode != bufvis->rgbMode)
      return GL_FALSE;
#if 0
   /* disabling this fixes the fgl_glxgears pbuffer demo */
   if (ctxvis->doubleBufferMode && !bufvis->doubleBufferMode)
      return GL_FALSE;
#endif
   if (ctxvis->stereoMode && !bufvis->stereoMode)
      return GL_FALSE;
   if (ctxvis->haveAccumBuffer && !bufvis->haveAccumBuffer)
      return GL_FALSE;
   if (ctxvis->haveDepthBuffer && !bufvis->haveDepthBuffer)
      return GL_FALSE;
   if (ctxvis->haveStencilBuffer && !bufvis->haveStencilBuffer)
      return GL_FALSE;
   if (ctxvis->redMask && ctxvis->redMask != bufvis->redMask)
      return GL_FALSE;
   if (ctxvis->greenMask && ctxvis->greenMask != bufvis->greenMask)
      return GL_FALSE;
   if (ctxvis->blueMask && ctxvis->blueMask != bufvis->blueMask)
      return GL_FALSE;
#if 0
   /* disabled (see bug 11161) */
   if (ctxvis->depthBits && ctxvis->depthBits != bufvis->depthBits)
      return GL_FALSE;
#endif
   if (ctxvis->stencilBits && ctxvis->stencilBits != bufvis->stencilBits)
      return GL_FALSE;

   return GL_TRUE;
}


/**
 * Do one-time initialization for the given framebuffer.  Specifically,
 * ask the driver for the window's current size and update the framebuffer
 * object to match.
 * Really, the device driver should totally take care of this.
 */
static void
initialize_framebuffer_size(GLcontext *ctx, GLframebuffer *fb)
{
   GLuint width, height;
   if (ctx->Driver.GetBufferSize) {
      ctx->Driver.GetBufferSize(fb, &width, &height);
      if (ctx->Driver.ResizeBuffers)
         ctx->Driver.ResizeBuffers(ctx, fb, width, height);
      fb->Initialized = GL_TRUE;
   }
}


/**
 * Check if the viewport/scissor size has not yet been initialized.
 * Initialize the size if the given width and height are non-zero.
 */
void
_mesa_check_init_viewport(GLcontext *ctx, GLuint width, GLuint height)
{
   if (!ctx->ViewportInitialized && width > 0 && height > 0) {
      /* Note: set flag here, before calling _mesa_set_viewport(), to prevent
       * potential infinite recursion.
       */
      ctx->ViewportInitialized = GL_TRUE;
      _mesa_set_viewport(ctx, 0, 0, width, height);
      _mesa_set_scissor(ctx, 0, 0, width, height);
   }
}


/**
 * Bind the given context to the given drawBuffer and readBuffer and
 * make it the current context for the calling thread.
 * We'll render into the drawBuffer and read pixels from the
 * readBuffer (i.e. glRead/CopyPixels, glCopyTexImage, etc).
 *
 * We check that the context's and framebuffer's visuals are compatible
 * and return immediately if they're not.
 *
 * \param newCtx  the new GL context. If NULL then there will be no current GL
 *                context.
 * \param drawBuffer  the drawing framebuffer
 * \param readBuffer  the reading framebuffer
 */
GLboolean
_mesa_make_current( GLcontext *newCtx, GLframebuffer *drawBuffer,
                    GLframebuffer *readBuffer )
{
   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(newCtx, "_mesa_make_current()\n");

   /* Check that the context's and framebuffer's visuals are compatible.
    */
   if (newCtx && drawBuffer && newCtx->WinSysDrawBuffer != drawBuffer) {
      if (!check_compatible(newCtx, drawBuffer)) {
         _mesa_warning(newCtx,
              "MakeCurrent: incompatible visuals for context and drawbuffer");
         return GL_FALSE;
      }
   }
   if (newCtx && readBuffer && newCtx->WinSysReadBuffer != readBuffer) {
      if (!check_compatible(newCtx, readBuffer)) {
         _mesa_warning(newCtx,
              "MakeCurrent: incompatible visuals for context and readbuffer");
         return GL_FALSE;
      }
   }

   /* We used to call _glapi_check_multithread() here.  Now do it in drivers */
   _glapi_set_context((void *) newCtx);
   ASSERT(_mesa_get_current_context() == newCtx);

   if (!newCtx) {
      _glapi_set_dispatch(NULL);  /* none current */
   }
   else {
      _glapi_set_dispatch(newCtx->CurrentDispatch);

      if (drawBuffer && readBuffer) {
	 /* TODO: check if newCtx and buffer's visual match??? */

         ASSERT(drawBuffer->Name == 0);
         ASSERT(readBuffer->Name == 0);
         _mesa_reference_framebuffer(&newCtx->WinSysDrawBuffer, drawBuffer);
         _mesa_reference_framebuffer(&newCtx->WinSysReadBuffer, readBuffer);

         /*
          * Only set the context's Draw/ReadBuffer fields if they're NULL
          * or not bound to a user-created FBO.
          */
         if (!newCtx->DrawBuffer || newCtx->DrawBuffer->Name == 0) {
            /* KW: merge conflict here, revisit. 
             */
            /* fix up the fb fields - these will end up wrong otherwise
             * if the DRIdrawable changes, and everything relies on them.
             * This is a bit messy (same as needed in _mesa_BindFramebufferEXT)
             */
            unsigned int i;
            GLenum buffers[MAX_DRAW_BUFFERS];

            _mesa_reference_framebuffer(&newCtx->DrawBuffer, drawBuffer);

            for(i = 0; i < newCtx->Const.MaxDrawBuffers; i++) {
               buffers[i] = newCtx->Color.DrawBuffer[i];
            }

            _mesa_drawbuffers(newCtx, newCtx->Const.MaxDrawBuffers, buffers, NULL);
         }
         if (!newCtx->ReadBuffer || newCtx->ReadBuffer->Name == 0) {
            _mesa_reference_framebuffer(&newCtx->ReadBuffer, readBuffer);
         }

         /* XXX only set this flag if we're really changing the draw/read
          * framebuffer bindings.
          */
	 newCtx->NewState |= _NEW_BUFFERS;

#if 1
         /* We want to get rid of these lines: */

#if _HAVE_FULL_GL
         if (!drawBuffer->Initialized) {
            initialize_framebuffer_size(newCtx, drawBuffer);
         }
         if (readBuffer != drawBuffer && !readBuffer->Initialized) {
            initialize_framebuffer_size(newCtx, readBuffer);
         }

	 _mesa_resizebuffers(newCtx);
#endif

#else
         /* We want the drawBuffer and readBuffer to be initialized by
          * the driver.
          * This generally means the Width and Height match the actual
          * window size and the renderbuffers (both hardware and software
          * based) are allocated to match.  The later can generally be
          * done with a call to _mesa_resize_framebuffer().
          *
          * It's theoretically possible for a buffer to have zero width
          * or height, but for now, assert check that the driver did what's
          * expected of it.
          */
         ASSERT(drawBuffer->Width > 0);
         ASSERT(drawBuffer->Height > 0);
#endif

         if (drawBuffer) {
            _mesa_check_init_viewport(newCtx,
                                      drawBuffer->Width, drawBuffer->Height);
         }
      }

      if (newCtx->FirstTimeCurrent) {
         check_context_limits(newCtx);

         /* We can use this to help debug user's problems.  Tell them to set
          * the MESA_INFO env variable before running their app.  Then the
          * first time each context is made current we'll print some useful
          * information.
          */
	 if (_mesa_getenv("MESA_INFO")) {
	    _mesa_print_info();
	 }

	 newCtx->FirstTimeCurrent = GL_FALSE;
      }
   }
   
   return GL_TRUE;
}


/**
 * Make context 'ctx' share the display lists, textures and programs
 * that are associated with 'ctxToShare'.
 * Any display lists, textures or programs associated with 'ctx' will
 * be deleted if nobody else is sharing them.
 */
GLboolean
_mesa_share_state(GLcontext *ctx, GLcontext *ctxToShare)
{
   if (ctx && ctxToShare && ctx->Shared && ctxToShare->Shared) {
      struct gl_shared_state *oldSharedState = ctx->Shared;
      GLint RefCount;

      ctx->Shared = ctxToShare->Shared;
      
      _glthread_LOCK_MUTEX(ctx->Shared->Mutex);
      ctx->Shared->RefCount++;
      _glthread_UNLOCK_MUTEX(ctx->Shared->Mutex);

      update_default_objects(ctx);

      _glthread_LOCK_MUTEX(oldSharedState->Mutex);
      RefCount = --oldSharedState->RefCount;
      _glthread_UNLOCK_MUTEX(oldSharedState->Mutex);

      if (RefCount == 0) {
         _mesa_free_shared_state(ctx, oldSharedState);
      }

      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}



/**
 * \return pointer to the current GL context for this thread.
 * 
 * Calls _glapi_get_context(). This isn't the fastest way to get the current
 * context.  If you need speed, see the #GET_CURRENT_CONTEXT macro in
 * context.h.
 */
GLcontext *
_mesa_get_current_context( void )
{
   return (GLcontext *) _glapi_get_context();
}


/**
 * Get context's current API dispatch table.
 *
 * It'll either be the immediate-mode execute dispatcher or the display list
 * compile dispatcher.
 * 
 * \param ctx GL context.
 *
 * \return pointer to dispatch_table.
 *
 * Simply returns __GLcontextRec::CurrentDispatch.
 */
struct _glapi_table *
_mesa_get_dispatch(GLcontext *ctx)
{
   return ctx->CurrentDispatch;
}

/*@}*/


/**********************************************************************/
/** \name Miscellaneous functions                                     */
/**********************************************************************/
/*@{*/

/**
 * Record an error.
 *
 * \param ctx GL context.
 * \param error error code.
 * 
 * Records the given error code and call the driver's dd_function_table::Error
 * function if defined.
 *
 * \sa
 * This is called via _mesa_error().
 */
void
_mesa_record_error(GLcontext *ctx, GLenum error)
{
   if (!ctx)
      return;

   if (ctx->ErrorValue == GL_NO_ERROR) {
      ctx->ErrorValue = error;
   }

   /* Call device driver's error handler, if any.  This is used on the Mac. */
   if (ctx->Driver.Error) {
      ctx->Driver.Error(ctx);
   }
}


/**
 * Flush commands and wait for completion.
 */
void
_mesa_finish(GLcontext *ctx)
{
   FLUSH_CURRENT( ctx, 0 );
   if (ctx->Driver.Finish) {
      ctx->Driver.Finish(ctx);
   }
}


/**
 * Flush commands.
 */
void
_mesa_flush(GLcontext *ctx)
{
   FLUSH_CURRENT( ctx, 0 );
   if (ctx->Driver.Flush) {
      ctx->Driver.Flush(ctx);
   }
}



/**
 * Execute glFinish().
 *
 * Calls the #ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH macro and the
 * dd_function_table::Finish driver callback, if not NULL.
 */
void GLAPIENTRY
_mesa_Finish(void)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);
   _mesa_finish(ctx);
}


/**
 * Execute glFlush().
 *
 * Calls the #ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH macro and the
 * dd_function_table::Flush driver callback, if not NULL.
 */
void GLAPIENTRY
_mesa_Flush(void)
{
   GET_CURRENT_CONTEXT(ctx);
   ASSERT_OUTSIDE_BEGIN_END_AND_FLUSH(ctx);
   _mesa_flush(ctx);
}


/**
 * Set mvp_with_dp4 flag.  If a driver has a preference for DP4 over
 * MUL/MAD, or vice versa, call this function to register that.
 * Otherwise we default to MUL/MAD.
 */
void
_mesa_set_mvp_with_dp4( GLcontext *ctx,
                        GLboolean flag )
{
   ctx->mvp_with_dp4 = flag;
}



/**
 * Prior to drawing anything with glBegin, glDrawArrays, etc. this function
 * is called to see if it's valid to render.  This involves checking that
 * the current shader is valid and the framebuffer is complete.
 * If an error is detected it'll be recorded here.
 * \return GL_TRUE if OK to render, GL_FALSE if not
 */
GLboolean
_mesa_valid_to_render(GLcontext *ctx, const char *where)
{
   if (ctx->Shader.CurrentProgram) {
      /* using shaders */
      if (!ctx->Shader.CurrentProgram->LinkStatus) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(shader not linked), where");
         return GL_FALSE;
      }
#if 0 /* not normally enabled */
      {
         char errMsg[100];
         if (!_mesa_validate_shader_program(ctx, ctx->Shader.CurrentProgram,
                                            errMsg)) {
            _mesa_warning(ctx, "Shader program %u is invalid: %s",
                          ctx->Shader.CurrentProgram->Name, errMsg);
         }
      }
#endif
   }
   else {
      if (ctx->VertexProgram.Enabled && !ctx->VertexProgram._Enabled) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(vertex program not valid)", where);
         return GL_FALSE;
      }
      if (ctx->FragmentProgram.Enabled && !ctx->FragmentProgram._Enabled) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(fragment program not valid)", where);
         return GL_FALSE;
      }
   }

   if (ctx->DrawBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                  "%s(incomplete framebuffer)", where);
      return GL_FALSE;
   }

#ifdef DEBUG
   if (ctx->Shader.Flags & GLSL_LOG) {
      struct gl_shader_program *shProg = ctx->Shader.CurrentProgram;
      if (shProg) {
         if (!shProg->_Used) {
            /* This is the first time this shader is being used.
             * Append shader's constants/uniforms to log file.
             */
            GLuint i;
            for (i = 0; i < shProg->NumShaders; i++) {
               struct gl_shader *sh = shProg->Shaders[i];
               if (sh->Type == GL_VERTEX_SHADER) {
                  _mesa_append_uniforms_to_file(sh,
                                                &shProg->VertexProgram->Base);
               }
               else if (sh->Type == GL_FRAGMENT_SHADER) {
                  _mesa_append_uniforms_to_file(sh,
                                                &shProg->FragmentProgram->Base);
               }
            }
            shProg->_Used = GL_TRUE;
         }
      }
   }
#endif

   return GL_TRUE;
}


/*@}*/
