/*
 * Mesa 3-D graphics library
 * Version:  7.1
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
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
 * \file mfeatures.h
 * Flags to enable/disable specific parts of the API.
 */

#ifndef FEATURES_H
#define FEATURES_H


#ifndef _HAVE_FULL_GL
#define _HAVE_FULL_GL 1
#endif

/* assert that a feature is disabled and should never be used */
#define ASSERT_NO_FEATURE() ASSERT(0)

/**
 * A feature can be anything.  But most of them share certain characteristics.
 *
 * When a feature defines driver entries, they can be initialized by
 *   _MESA_INIT_<FEATURE>_FUNCTIONS
 *
 * When a feature defines vtxfmt entries, they can be initialized and
 * installed by
 *   _MESA_INIT_<FEATURE>_VTXFMT
 *   _mesa_install_<feature>_vtxfmt
 *
 * When a feature defines dispatch entries, they are initialized by
 *   _mesa_init_<feature>_dispatch
 *
 * When a feature has states, they are initialized and freed by
 *   _mesa_init_<feature>
 *   _mesa_free_<feature>_data
 *
 * Except for states, the others compile to no-op when a feature is disabled.
 *
 * The GLAPIENTRYs and helper functions defined by a feature should also
 * compile to no-op when it is disabled.  But to save typings and to catch
 * bugs, some of them may be unavailable, or compile to ASSERT_NO_FEATURE()
 * when the feature is disabled.
 *
 * A feature following the conventions may be used without knowing if it is
 * enabled or not.
 */

#ifdef IN_DRI_DRIVER
#define FEATURE_remap_table 1
#else
#define FEATURE_remap_table 0
#endif

#define FEATURE_accum  _HAVE_FULL_GL
#define FEATURE_arrayelt  _HAVE_FULL_GL
#define FEATURE_attrib_stack  _HAVE_FULL_GL
/* this disables vtxfmt, api_loopback, and api_noop completely */
#define FEATURE_beginend  _HAVE_FULL_GL
#define FEATURE_colortable  _HAVE_FULL_GL
#define FEATURE_convolve  _HAVE_FULL_GL
#define FEATURE_dispatch  _HAVE_FULL_GL
#define FEATURE_dlist  (_HAVE_FULL_GL && FEATURE_arrayelt && FEATURE_beginend)
#define FEATURE_draw_read_buffer  _HAVE_FULL_GL
#define FEATURE_drawpix  _HAVE_FULL_GL
#define FEATURE_evaluators  _HAVE_FULL_GL
#define FEATURE_feedback  _HAVE_FULL_GL
#define FEATURE_fixedpt 0
#define FEATURE_histogram  _HAVE_FULL_GL
#define FEATURE_pixel_transfer  _HAVE_FULL_GL
#define FEATURE_point_size_array 0
#define FEATURE_queryobj  _HAVE_FULL_GL
#define FEATURE_rastpos  _HAVE_FULL_GL
#define FEATURE_texgen  _HAVE_FULL_GL
#define FEATURE_texture_fxt1  _HAVE_FULL_GL
#define FEATURE_texture_s3tc  _HAVE_FULL_GL
#define FEATURE_userclip  _HAVE_FULL_GL
#define FEATURE_vertex_array_byte 0
#define FEATURE_es2_glsl 0

#define FEATURE_ARB_fragment_program  _HAVE_FULL_GL
#define FEATURE_ARB_framebuffer_object  _HAVE_FULL_GL
#define FEATURE_ARB_map_buffer_range  _HAVE_FULL_GL
#define FEATURE_ARB_pixel_buffer_object  _HAVE_FULL_GL
#define FEATURE_ARB_vertex_buffer_object  _HAVE_FULL_GL
#define FEATURE_ARB_vertex_program  _HAVE_FULL_GL
#define FEATURE_ARB_vertex_shader _HAVE_FULL_GL
#define FEATURE_ARB_fragment_shader _HAVE_FULL_GL
#define FEATURE_ARB_shader_objects (FEATURE_ARB_vertex_shader || FEATURE_ARB_fragment_shader)
#define FEATURE_ARB_shading_language_100 FEATURE_ARB_shader_objects
#define FEATURE_ARB_shading_language_120 FEATURE_ARB_shader_objects
#define FEATURE_ARB_sync _HAVE_FULL_GL

#define FEATURE_EXT_framebuffer_blit _HAVE_FULL_GL
#define FEATURE_EXT_framebuffer_object _HAVE_FULL_GL
#define FEATURE_EXT_pixel_buffer_object  _HAVE_FULL_GL
#define FEATURE_EXT_texture_sRGB _HAVE_FULL_GL
#define FEATURE_ATI_fragment_shader _HAVE_FULL_GL
#define FEATURE_NV_fence  _HAVE_FULL_GL
#define FEATURE_NV_fragment_program  _HAVE_FULL_GL
#define FEATURE_NV_vertex_program  _HAVE_FULL_GL


#endif /* FEATURES_H */
