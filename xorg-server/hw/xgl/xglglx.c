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

#include "xglglx.h"

#ifdef GLXEXT

#ifdef XGL_MODULAR
#include <dlfcn.h>
#endif

xglGLXFuncRec __xglGLXFunc;

#ifndef NGLXEXTLOG
FILE *__xglGLXLogFp;
#endif

static void *glXHandle = 0;
static void *glCoreHandle = 0;

#define SYM(ptr, name) { (void **) &(ptr), (name) }

__GLXextensionInfo *__xglExtensionInfo;
__GLXscreenInfo *__xglScreenInfoPtr;

void
GlxSetVisualConfigs (int	       nconfigs,
		     __GLXvisualConfig *configs,
		     void              **privates)
{
    if (glXHandle && glCoreHandle)
	(*__xglGLXFunc.setVisualConfigs) (nconfigs, configs, privates);
}

void
GlxExtensionInit (void)
{
    if (glXHandle && glCoreHandle)
	(*__xglGLXFunc.extensionInit) ();
}

void
GlxWrapInitVisuals (miInitVisualsProcPtr *initVisuals)
{
    if (glXHandle && glCoreHandle)
	(*__xglGLXFunc.wrapInitVisuals) (initVisuals);
}

int
GlxInitVisuals (VisualPtr     *visualp,
		DepthPtr      *depthp,
		int	      *nvisualp,
		int	      *ndepthp,
		int	      *rootDepthp,
		VisualID      *defaultVisp,
		unsigned long sizes,
		int	      bitsPerRGB,
		int	      preferredVis)
{
    if (glXHandle && glCoreHandle)
	return (*__xglGLXFunc.initVisuals) (visualp, depthp, nvisualp, ndepthp,
					    rootDepthp, defaultVisp, sizes,
					    bitsPerRGB, preferredVis);

    return 0;
}

void
GlxFlushContextCache (void)
{
    (*__xglGLXFunc.flushContextCache) ();
}

void
GlxSetRenderTables (struct _glapi_table *table)
{
  (*__xglGLXFunc.setRenderTables) (table);
}

struct _glapi_table *_mglapi_Dispatch;

void *(*__glcore_DDXScreenInfo)(void);

void *__glXglDDXScreenInfo(void)
{
  return __xglScreenInfoPtr;
}

void *(*__glcore_DDXExtensionInfo)(void);

void *__glXglDDXExtensionInfo(void)
{
  return __xglExtensionInfo;
}	

void _gl_copy_visual_to_context_mode( __GLcontextModes * mode,
                                 const __GLXvisualConfig * config )
{
	(*__xglGLXFunc.copy_visual_to_context_mode)(mode, config);
}

__GLcontextModes *_gl_context_modes_create( unsigned count, size_t minimum_size )
{
	return (*__xglGLXFunc.context_modes_create)(count, minimum_size);
}

void _gl_context_modes_destroy( __GLcontextModes * modes )
{
	(*__xglGLXFunc.context_modes_destroy)(modes);
}

GLint _gl_convert_from_x_visual_type( int visualType )
{
	return (*__xglGLXFunc.convert_from_x_visual_type)(visualType);
}

GLint _gl_convert_to_x_visual_type( int visualType )
{
	return (*__xglGLXFunc.convert_to_x_visual_type)(visualType);
}



Bool
xglLoadGLXModules (void)
{

#ifdef XGL_MODULAR
    if (!glXHandle)
    {
	xglSymbolRec sym[] = {
	    SYM (__xglGLXFunc.extensionInit,     "GlxExtensionInit"),
	    SYM (__xglGLXFunc.setVisualConfigs,  "GlxSetVisualConfigs"),
	    SYM (__xglGLXFunc.wrapInitVisuals,   "GlxWrapInitVisuals"),
	    SYM (__xglGLXFunc.initVisuals,	 "GlxInitVisuals"),
	    SYM (__xglGLXFunc.flushContextCache, "__glXFlushContextCache"),
	    SYM (__xglGLXFunc.setRenderTables,   "GlxSetRenderTables"),
	    SYM (__xglGLXFunc.copy_visual_to_context_mode, "_gl_copy_visual_to_context_mode"),
	    SYM (__xglGLXFunc.context_modes_create, "_gl_context_modes_create"),
	    SYM (__xglGLXFunc.context_modes_destroy, "_gl_context_modes_destroy"),
	    SYM (__xglGLXFunc.convert_from_x_visual_type, "_gl_convert_from_x_visual_type"),
	    SYM (__xglGLXFunc.convert_to_x_visual_type, "_gl_convert_to_x_visual_type"),
	};

	glXHandle = xglLoadModule ("glx", RTLD_NOW | RTLD_LOCAL);
	if (!glXHandle)
	    return FALSE;

	if (!xglLookupSymbols (glXHandle, sym, sizeof (sym) / sizeof (sym[0])))
	{
	    xglUnloadModule (glXHandle);
	    glXHandle = 0;

	    return FALSE;
	}
    }

    if (!glCoreHandle)
    {
        xglSymbolRec ddxsym[] = {
	    SYM (__glcore_DDXExtensionInfo, "__glXglDDXExtensionInfo"),
	    SYM (__glcore_DDXScreenInfo, "__glXglDDXScreenInfo")
        };	

	glCoreHandle = xglLoadModule ("glcore", RTLD_NOW | RTLD_LOCAL);
	if (!glCoreHandle)
	    return FALSE;

	if (!xglLookupSymbols (glCoreHandle, ddxsym,
			       sizeof (ddxsym) / sizeof(ddxsym[0])))
	{
	  xglUnloadModule (glCoreHandle);
	  glCoreHandle = 0;
	  
	  return FALSE;
	}

	__xglScreenInfoPtr = __glcore_DDXScreenInfo();
	__xglExtensionInfo = __glcore_DDXExtensionInfo();
	{
	  xglSymbolRec sym[] = {
	      SYM (__xglScreenInfoPtr->screenProbe,    "__MESA_screenProbe"),
  	    SYM (__xglScreenInfoPtr->createContext,  "__MESA_createContext"),
	    SYM (__xglScreenInfoPtr->createBuffer,   "__MESA_createBuffer"),
	    SYM (__xglExtensionInfo->resetExtension,
		 "__MESA_resetExtension"),
	    SYM (__xglExtensionInfo->initVisuals, "__MESA_initVisuals"),
	    SYM (__xglExtensionInfo->setVisualConfigs,
		 "__MESA_setVisualConfigs"),

	  };


	  if (!xglLookupSymbols (glCoreHandle, sym,
				 sizeof (sym) / sizeof (sym[0])))
	  {
	    xglUnloadModule (glCoreHandle);
	    glCoreHandle = 0;
	    
	    return FALSE;
	  }
	}

	if (!xglLoadHashFuncs (glCoreHandle))
	{
	    xglUnloadModule (glCoreHandle);
	    glCoreHandle = 0;
	}
    }

    return TRUE;
#else
    return FALSE;
#endif

}

void
xglUnloadGLXModules (void)
{

#ifdef XGL_MODULAR
    if (glXHandle)
    {
	xglUnloadModule (glXHandle);
	glXHandle = 0;
    }

    if (glCoreHandle)
    {
	xglUnloadModule (glCoreHandle);
	glCoreHandle = 0;
    }
#endif

}

#endif
