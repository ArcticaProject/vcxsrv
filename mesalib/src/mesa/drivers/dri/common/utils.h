/*
 * (C) Copyright IBM Corporation 2002, 2004
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Ian Romanick <idr@us.ibm.com>
 */

#ifndef DRI_DEBUG_H
#define DRI_DEBUG_H

#include <GL/gl.h>
#include <GL/internal/dri_interface.h>
#include "main/context.h"
#include "main/remap.h"

typedef struct __DRIutilversionRec2    __DRIutilversion2;

struct dri_debug_control {
    const char * string;
    unsigned     flag;
};

/**
 * Description of the API for an extension to OpenGL.
 */
struct dri_extension {
    /**
     * Name of the extension.
     */
    const char * name;
    

    /**
     * Pointer to a list of \c dri_extension_function structures.  The list
     * is terminated by a structure with a \c NULL
     * \c dri_extension_function::strings pointer.
     */
    const struct gl_function_remap * functions;
};

/**
 * Used to store a version which includes a major range instead of a single
 * major version number.
 */
struct __DRIutilversionRec2 {
    int    major_min;    /** min allowed Major version number. */
    int    major_max;    /** max allowed Major version number. */
    int    minor;        /**< Minor version number. */
    int    patch;        /**< Patch-level. */
};

extern unsigned driParseDebugString( const char * debug,
    const struct dri_debug_control * control );

extern unsigned driGetRendererString( char * buffer,
    const char * hardware_name, const char * driver_date, GLuint agp_mode );

extern void driInitExtensions( GLcontext * ctx, 
    const struct dri_extension * card_extensions, GLboolean enable_imaging );

extern void driInitSingleExtension( GLcontext * ctx,
    const struct dri_extension * ext );

extern GLboolean driCheckDriDdxDrmVersions2(const char * driver_name,
    const __DRIversion * driActual, const __DRIversion * driExpected,
    const __DRIversion * ddxActual, const __DRIversion * ddxExpected,
    const __DRIversion * drmActual, const __DRIversion * drmExpected);

extern GLboolean driCheckDriDdxDrmVersions3(const char * driver_name,
    const __DRIversion * driActual, const __DRIversion * driExpected,
    const __DRIversion * ddxActual, const __DRIutilversion2 * ddxExpected,
    const __DRIversion * drmActual, const __DRIversion * drmExpected);

extern GLboolean driClipRectToFramebuffer( const GLframebuffer *buffer,
					   GLint *x, GLint *y,
					   GLsizei *width, GLsizei *height );

struct __DRIconfigRec {
    __GLcontextModes modes;
};

extern __DRIconfig **
driCreateConfigs(GLenum fb_format, GLenum fb_type,
		 const uint8_t * depth_bits, const uint8_t * stencil_bits,
		 unsigned num_depth_stencil_bits,
		 const GLenum * db_modes, unsigned num_db_modes,
    		 const uint8_t * msaa_samples, unsigned num_msaa_modes);

__DRIconfig **driConcatConfigs(__DRIconfig **a,
			       __DRIconfig **b);

int
driGetConfigAttrib(const __DRIconfig *config,
		   unsigned int attrib, unsigned int *value);
int
driIndexConfigAttrib(const __DRIconfig *config, int index,
		     unsigned int *attrib, unsigned int *value);

#endif /* DRI_DEBUG_H */
