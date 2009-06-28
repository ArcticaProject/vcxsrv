/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xgl.h"

#ifdef GLXEXT

#include "glxserver.h"
#include "glxscreens.h"
#include "glxext.h"
#include "glapitable.h"


typedef struct _xglGLXFunc {
    void (*extensionInit)     (void);
    void (*setVisualConfigs)  (int		    nconfigs,
			       __GLXvisualConfig    *configs,
			       void                 **privates);
    void (*wrapInitVisuals)   (miInitVisualsProcPtr *initVisuals);
    int  (*initVisuals)	      (VisualPtr	    *visualp,
			       DepthPtr		    *depthp,
			       int		    *nvisualp,
			       int		    *ndepthp,
			       int		    *rootDepthp,
			       VisualID		    *defaultVisp,
			       unsigned long	    sizes,
			       int		    bitsPerRGB,
			       int		    preferredVis);

    void (*flushContextCache) (void);
    void *(*DDXExtensionInfo) (void);
    void *(*DDXScreenInfo) (void);
  void (*setRenderTables)   (struct _glapi_table	*table);
    void (*copy_visual_to_context_mode)( __GLcontextModes *mode, const __GLXvisualConfig *config );
    __GLcontextModes *(*context_modes_create)( unsigned count, size_t minimum_size );
    void (*context_modes_destroy)( __GLcontextModes * modes );
    GLint (*convert_from_x_visual_type)( int visualType );
    GLint (*convert_to_x_visual_type)( int visualType );
} xglGLXFuncRec, *xglGLXFuncPtr;

extern xglGLXFuncRec __xglGLXFunc;

#ifndef NGLXEXTLOG

extern FILE *__xglGLXLogFp;

#endif

/* xglglx.c */

Bool
xglLoadGLXModules (void);

void
xglUnloadGLXModules (void);

#endif
