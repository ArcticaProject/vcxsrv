/*
 * GLX implementation that uses Apple's OpenGL.framework
 * (Indirect rendering path)
 *
 * Copyright (c) 2007 Apple Inc.
 * Copyright (c) 2004 Torrey T. Lyons. All Rights Reserved.
 * Copyright (c) 2002 Greg Parker. All Rights Reserved.
 *
 * Portions of this file are copied from Mesa's xf86glx.c,
 * which contains the following copyright:
 *
 * Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "dri.h"

#define GL_EXT_histogram 1
#define GL_EXT_polygon_offset 1
#define GL_SGIS_pixel_texture 1
#define GL_SGIX_pixel_texture 1
#define GL_EXT_multisample 1
#define GL_SGIS_multisample 1
#define GL_EXT_vertex_array 1
#define GL_ARB_point_parameters 1
#define GL_NV_vertex_array_range 1
#define GL_MESA_resize_buffers 1
#define GL_ARB_window_pos 1
#define GL_EXT_cull_vertex 1
#define GL_NV_vertex_program 1
#define GL_APPLE_fence 1
#define GL_IBM_multimode_draw_arrays 1
#define GL_EXT_fragment_shader 1

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLContext.h>

// X11 and X11's glx
#include <GL/gl.h>
#include <GL/glxproto.h>
#include <windowstr.h>
#include <resource.h>
#include <GL/glxint.h>
#include <GL/glxtokens.h>
#include <scrnintstr.h>
#include <glxserver.h>
#include <glxscreens.h>
#include <glxdrawable.h>
#include <glxcontext.h>
#include <glxext.h>
#include <glxutil.h>
#include <glxscreens.h>
#include <GL/internal/glcore.h>
#include "x-hash.h"
#include "x-list.h"

#include <dispatch.h>
#define GLAPIENTRYP *
typedef unsigned long long GLuint64EXT;
typedef long long GLint64EXT;
#include <Xplugin.h>
#include "glcontextmodes.h"
#include <glapi.h>
#include <glapitable.h>

// ggs: needed to call back to glx with visual configs
extern void GlxSetVisualConfigs(int nconfigs, __GLXvisualConfig *configs, void **configprivs);
__GLXprovider * GlxGetDRISWrastProvider (void);

// Write debugging output, or not
#ifdef GLAQUA_DEBUG
#define GLAQUA_DEBUG_MSG ErrorF
#else
#define GLAQUA_DEBUG_MSG(a, ...)
#endif

static void setup_dispatch_table(void);
GLuint __glFloorLog2(GLuint val);
void warn_func(void * p1, char *format, ...);

// some prototypes
static __GLXscreen * __glXAquaScreenProbe(ScreenPtr pScreen);
static __GLXdrawable * __glXAquaScreenCreateDrawable(__GLXscreen *screen, DrawablePtr pDraw, XID drawId, __GLcontextModes *modes);

static Bool glAquaInitVisuals(VisualPtr *visualp, DepthPtr *depthp,
                              int *nvisualp, int *ndepthp,
                              int *rootDepthp, VisualID *defaultVisp,
                              unsigned long sizes, int bitsPerRGB);
static void glAquaSetVisualConfigs(int nconfigs, __GLXvisualConfig *configs,
                                   void **privates);

static void glAquaResetExtension(void);
static void __glXAquaContextDestroy(__GLXcontext *baseContext);
static int __glXAquaContextMakeCurrent(__GLXcontext *baseContext);
static int __glXAquaContextLoseCurrent(__GLXcontext *baseContext);
static int __glXAquaContextForceCurrent(__GLXcontext *baseContext);
static int __glXAquaContextCopy(__GLXcontext *baseDst, __GLXcontext *baseSrc, unsigned long mask);

static CGLPixelFormatObj makeFormat(__GLcontextModes *mode);

__GLXprovider __glXDRISWRastProvider = {
  __glXAquaScreenProbe,
  "Core OpenGL",
    NULL
};

__GLXprovider *
GlxGetDRISWRastProvider (void)
{
  GLAQUA_DEBUG_MSG("GlxGetDRISWRastProvider\n");
  return &__glXDRISWRastProvider;
}

typedef struct __GLXAquaScreen   __GLXAquaScreen;
typedef struct __GLXAquaContext  __GLXAquaContext;
typedef struct __GLXAquaDrawable __GLXAquaDrawable;

struct __GLXAquaScreen {
  __GLXscreen   base;
  int           index;
    int num_vis;
    __GLcontextModes *modes;
};

static __GLXAquaScreen glAquaScreens[MAXSCREENS];

struct __GLXAquaContext {
  __GLXcontext base;
  CGLContextObj ctx;
  CGLPixelFormatObj pixelFormat;
  xp_surface_id sid;
  unsigned isAttached :1;
};

struct __GLXAquaDrawable {
  __GLXdrawable base;
    DrawablePtr pDraw;
    xp_surface_id sid;
};

static __GLXcontext *
__glXAquaScreenCreateContext(__GLXscreen *screen,
			     __GLcontextModes *modes,
			     __GLXcontext *baseShareContext)
{
  __GLXAquaContext *context;
  __GLXAquaContext *shareContext = (__GLXAquaContext *) baseShareContext;
  CGLError gl_err;
  
  GLAQUA_DEBUG_MSG("glXAquaScreenCreateContext\n");

  context = malloc (sizeof (__GLXAquaContext));
  if (context == NULL) return NULL;

  memset(context, 0, sizeof *context);

  context->base.pGlxScreen = screen;
  context->base.modes      = modes;

  context->base.destroy        = __glXAquaContextDestroy;
  context->base.makeCurrent    = __glXAquaContextMakeCurrent;
  context->base.loseCurrent    = __glXAquaContextLoseCurrent;
  context->base.copy           = __glXAquaContextCopy;
  context->base.forceCurrent   = __glXAquaContextForceCurrent;
  //  context->base.createDrawable = __glXAquaContextCreateDrawable;

  context->pixelFormat = makeFormat(modes);
  if (!context->pixelFormat) {
        free(context);
        return NULL;
  }

  context->ctx = NULL;
  gl_err = CGLCreateContext(context->pixelFormat,
                            shareContext ? shareContext->ctx : NULL,
                            &context->ctx);

  if (gl_err != 0) {
      ErrorF("CGLCreateContext error: %s\n", CGLErrorString(gl_err));
      CGLDestroyPixelFormat(context->pixelFormat);
      free(context);
      return NULL;
    }
	setup_dispatch_table();
    GLAQUA_DEBUG_MSG("glAquaCreateContext done\n");
  return &context->base;
}

static __GLXextensionInfo __glDDXExtensionInfo = {
    GL_CORE_APPLE,
    glAquaResetExtension,
    glAquaInitVisuals,
    glAquaSetVisualConfigs
};

void *__glXglDDXExtensionInfo(void) {
  GLAQUA_DEBUG_MSG("glXAglDDXExtensionInfo\n");
    return &__glDDXExtensionInfo;
}

/* maps from surface id -> list of __GLcontext */
static x_hash_table *surface_hash;

static void __glXAquaContextDestroy(__GLXcontext *baseContext) {
    x_list *lst;

    __GLXAquaContext *context = (__GLXAquaContext *) baseContext;

    GLAQUA_DEBUG_MSG("glAquaContextDestroy (ctx 0x%x)\n",
                     (unsigned int) baseContext);
    if (context != NULL) {
      if (context->sid != 0 && surface_hash != NULL) {
		lst = x_hash_table_lookup(surface_hash, (void *) context->sid, NULL);
		lst = x_list_remove(lst, context);
		x_hash_table_insert(surface_hash, (void *) context->sid, lst);
      }

      if (context->ctx != NULL) CGLDestroyContext(context->ctx);

      if (context->pixelFormat != NULL)	CGLDestroyPixelFormat(context->pixelFormat);
      
      free(context);
    }
}

static int __glXAquaContextLoseCurrent(__GLXcontext *baseContext) {
    CGLError gl_err;

    GLAQUA_DEBUG_MSG("glAquaLoseCurrent (ctx 0x%p)\n", baseContext);

    gl_err = CGLSetCurrentContext(NULL);
    if (gl_err != 0)
      ErrorF("CGLSetCurrentContext error: %s\n", CGLErrorString(gl_err));

    __glXLastContext = NULL; // Mesa does this; why?

    return GL_TRUE;
}

/* Called when a surface is destroyed as a side effect of destroying
   the window it's attached to. */
static void surface_notify(void *_arg, void *data) {
    DRISurfaceNotifyArg *arg = (DRISurfaceNotifyArg *)_arg;
    __GLXAquaDrawable *draw = (__GLXAquaDrawable *)data;
    __GLXAquaContext *context;
    x_list *lst;
	if(_arg == NULL || data == NULL) {
		ErrorF("surface_notify called with bad params");
		return;
	}
	
    GLAQUA_DEBUG_MSG("surface_notify(%p, %p)\n", _arg, data);
    switch (arg->kind) {
    case AppleDRISurfaceNotifyDestroyed:
        if (surface_hash != NULL)
            x_hash_table_remove(surface_hash, (void *) arg->id);
	        draw->base.pDraw = NULL;
			draw->sid = 0;
        break;

    case AppleDRISurfaceNotifyChanged:
        if (surface_hash != NULL) {
            lst = x_hash_table_lookup(surface_hash, (void *) arg->id, NULL);
            for (; lst != NULL; lst = lst->next)
            {
                context = lst->data;
                xp_update_gl_context(context->ctx);
            }
        }
        break;
	default:
		ErrorF("surface_notify: unknown kind %d\n", arg->kind);
		break;
    }
}

static void attach(__GLXAquaContext *context, __GLXAquaDrawable *draw) {
    DrawablePtr pDraw;
	GLAQUA_DEBUG_MSG("attach(%p, %p)\n", context, draw);
    pDraw = draw->base.pDraw;

    if (draw->sid == 0) {
//        if (!quartzProcs->CreateSurface(pDraw->pScreen, pDraw->id, pDraw,
        if (!DRICreateSurface(pDraw->pScreen, pDraw->id, pDraw,
                                        0, &draw->sid, NULL,
                                        surface_notify, draw))
            return;
        draw->pDraw = pDraw;
	} 

    if (!context->isAttached || context->sid != draw->sid) {
        x_list *lst;

        if (xp_attach_gl_context(context->ctx, draw->sid) != Success) {
//            quartzProcs->DestroySurface(pDraw->pScreen, pDraw->id, pDraw,
            DRIDestroySurface(pDraw->pScreen, pDraw->id, pDraw,
								surface_notify, draw);
            if (surface_hash != NULL)
                x_hash_table_remove(surface_hash, (void *) draw->sid);

            draw->sid = 0;
            return;
        }

        context->isAttached = TRUE;
        context->sid = draw->sid;

        if (surface_hash == NULL)
            surface_hash = x_hash_table_new(NULL, NULL, NULL, NULL);

        lst = x_hash_table_lookup(surface_hash, (void *) context->sid, NULL);
        if (x_list_find(lst, context) == NULL) {
            lst = x_list_prepend(lst, context);
            x_hash_table_insert(surface_hash, (void *) context->sid, lst);
        }

        GLAQUA_DEBUG_MSG("attached 0x%x to 0x%x\n", (unsigned int) pDraw->id,
                         (unsigned int) draw->sid);
    } 
}

#if 0     // unused
static void unattach(__GLXAquaContext *context) {
	x_list *lst;
	GLAQUA_DEBUG_MSG("unattach\n");
	if (context == NULL) {
		ErrorF("Tried to unattach a null context\n");
		return;
	}
    if (context->isAttached) {
        GLAQUA_DEBUG_MSG("unattaching\n");

        if (surface_hash != NULL) {
            lst = x_hash_table_lookup(surface_hash, (void *) context->sid, NULL);
            lst = x_list_remove(lst, context);
            x_hash_table_insert(surface_hash, (void *) context->sid, lst);
        }

        CGLClearDrawable(context->ctx);
        context->isAttached = FALSE;
        context->sid = 0;
    }
}
#endif

static int __glXAquaContextMakeCurrent(__GLXcontext *baseContext) {
    CGLError gl_err;
    __GLXAquaContext *context = (__GLXAquaContext *) baseContext;
	__GLXAquaDrawable *drawPriv = (__GLXAquaDrawable *) context->base.drawPriv;

    GLAQUA_DEBUG_MSG("glAquaMakeCurrent (ctx 0x%p)\n", baseContext);
    
    attach(context, drawPriv);

    gl_err = CGLSetCurrentContext(context->ctx);
    if (gl_err != 0)
        ErrorF("CGLSetCurrentContext error: %s\n", CGLErrorString(gl_err));
    
    return gl_err == 0;
}

static int __glXAquaContextCopy(__GLXcontext *baseDst, __GLXcontext *baseSrc, unsigned long mask)
{
    CGLError gl_err;

    __GLXAquaContext *dst = (__GLXAquaContext *) baseDst;
    __GLXAquaContext *src = (__GLXAquaContext *) baseSrc;

    GLAQUA_DEBUG_MSG("GLXAquaContextCopy\n");

    gl_err = CGLCopyContext(src->ctx, dst->ctx, mask);
    if (gl_err != 0)
        ErrorF("CGLCopyContext error: %s\n", CGLErrorString(gl_err));

    return gl_err == 0;
}

static int __glXAquaContextForceCurrent(__GLXcontext *baseContext)
{
    CGLError gl_err;
    __GLXAquaContext *context = (__GLXAquaContext *) baseContext;
    GLAQUA_DEBUG_MSG("glAquaForceCurrent (ctx %p)\n", context->ctx);

    gl_err = CGLSetCurrentContext(context->ctx);
    if (gl_err != 0)
        ErrorF("CGLSetCurrentContext error: %s\n", CGLErrorString(gl_err));

    return gl_err == 0;
}

/* Drawing surface notification callbacks */

static GLboolean __glXAquaDrawableResize(__GLXdrawable *base)  {
    GLAQUA_DEBUG_MSG("unimplemented glAquaDrawableResize\n");
    return GL_TRUE;
}

static GLboolean __glXAquaDrawableSwapBuffers(__GLXdrawable *base) {
    CGLError gl_err;
	__GLXAquaContext * drawableCtx;
//    GLAQUA_DEBUG_MSG("glAquaDrawableSwapBuffers(%p)\n",base);
	
	if(!base) {
		ErrorF("glXAquaDrawbleSwapBuffers passed NULL\n");
	    return GL_FALSE;
	}

    drawableCtx = (__GLXAquaContext *)base->drawGlxc;

    if (drawableCtx != NULL && drawableCtx->ctx != NULL) {
        gl_err = CGLFlushDrawable(drawableCtx->ctx);
        if (gl_err != 0)
            ErrorF("CGLFlushDrawable error: %s\n", CGLErrorString(gl_err));
    }
    return GL_TRUE;
}

static CGLPixelFormatObj makeFormat(__GLcontextModes *mode) {
    int i;
    CGLPixelFormatAttribute attr[64]; // currently uses max of 30
    CGLPixelFormatObj result;
    GLint n_formats;
    CGLError gl_err;
    
    GLAQUA_DEBUG_MSG("makeFormat\n");

    if (!mode->rgbMode)
        return NULL;

    i = 0;

    // attr [i++] = kCGLPFAAcelerated; // require hwaccel - BAD for multiscreen
    // attr [i++] = kCGLPFANoRecovery; // disable fallback renderers - BAD

    if (mode->stereoMode) {
        attr[i++] = kCGLPFAStereo;
    }
    if (mode->doubleBufferMode) {
        attr[i++] = kCGLPFADoubleBuffer;
    }

    if (mode->colorIndexMode) {
        /* ignored */
    }

    if (mode->rgbMode) {
        attr[i++] = kCGLPFAColorSize;
        attr[i++] = mode->redBits + mode->greenBits + mode->blueBits;
        attr[i++] = kCGLPFAAlphaSize;
        attr[i++] = 1; /* FIXME: ignoring mode->alphaBits which is always 0 */
    }

    if (mode->haveAccumBuffer) {
        attr[i++] = kCGLPFAAccumSize;
        attr[i++] = mode->accumRedBits + mode->accumGreenBits
                    + mode->accumBlueBits + mode->accumAlphaBits;
    }
	
    if (mode->haveDepthBuffer) {
        attr[i++] = kCGLPFADepthSize;
        attr[i++] = mode->depthBits;
    }
	
    if (mode->haveStencilBuffer) {
        attr[i++] = kCGLPFAStencilSize;
        attr[i++] = mode->stencilBits;
    }

    attr[i++] = kCGLPFAAuxBuffers;
    attr[i++] = mode->numAuxBuffers;

    /* mode->level ignored */

    /* mode->pixmapMode ? */

    attr[i++] = 0;

    GLAQUA_DEBUG_MSG("makeFormat almost done\n");

    result = NULL;
    gl_err = CGLChoosePixelFormat(attr, &result, &n_formats);
    if (gl_err != 0)
        ErrorF("CGLChoosePixelFormat error: %s\n", CGLErrorString(gl_err));

    GLAQUA_DEBUG_MSG("makeFormat done (0x%x)\n", (unsigned int) result);

    return result;
}

// Originally copied from Mesa

static int                 numConfigs     = 0;
static __GLXvisualConfig  *visualConfigs  = NULL;
static void              **visualPrivates = NULL;

/*
 * In the case the driver defines no GLX visuals we'll use these.
 * Note that for TrueColor and DirectColor visuals, bufferSize is the 
 * sum of redSize, greenSize, blueSize and alphaSize, which may be larger 
 * than the nplanes/rootDepth of the server's X11 visuals
 */
#define NUM_FALLBACK_CONFIGS 5
static __GLXvisualConfig FallbackConfigs[NUM_FALLBACK_CONFIGS] = {
  /* [0] = RGB, double buffered, Z */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
     0,  0,  0, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    0,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [1] = RGB, double buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
    16, 16, 16, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [2] = RGB+Alpha, double buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 8,      /* rgba sizes */
    -1, -1, -1, -1,     /* rgba masks */
    16, 16, 16, 16,     /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [3] = RGB+Alpha, single buffered, Z, stencil, accum */
  {
    -1,                 /* vid */
    -1,                 /* class */
    True,               /* rgba */
    -1, -1, -1, 8,      /* rgba sizes */
    -1, -1, -1, -1,     /* rgba masks */
    16, 16, 16, 16,     /* rgba accum sizes */
    False,              /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    8,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
  /* [4] = CI, double buffered, Z */
  {
    -1,                 /* vid */
    -1,                 /* class */
    False,              /* rgba? (false = color index) */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
     0,  0,  0, 0,      /* rgba accum sizes */
    True,               /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    0,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE,           /* visualRating */
    GLX_NONE,           /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
  },
};

static __GLXvisualConfig NullConfig = {
    -1,                 /* vid */
    -1,                 /* class */
    False,              /* rgba */
    -1, -1, -1, 0,      /* rgba sizes */
    -1, -1, -1, 0,      /* rgba masks */
     0,  0,  0, 0,      /* rgba accum sizes */
    False,              /* doubleBuffer */
    False,              /* stereo */
    -1,                 /* bufferSize */
    16,                 /* depthSize */
    0,                  /* stencilSize */
    0,                  /* auxBuffers */
    0,                  /* level */
    GLX_NONE_EXT,       /* visualRating */
    0,                  /* transparentPixel */
    0, 0, 0, 0,         /* transparent rgba color (floats scaled to ints) */
    0                   /* transparentIndex */
};


static inline int count_bits(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0f0f0f0f;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 63;
}


static Bool init_visuals(int *nvisualp, VisualPtr *visualp,
                         VisualID *defaultVisp,
                         int ndepth, DepthPtr pdepth,
                         int rootDepth)
{
    int numRGBconfigs;
    int numCIconfigs;
    int numVisuals = *nvisualp;
    int numNewVisuals;
    int numNewConfigs;
    VisualPtr pVisual = *visualp;
    VisualPtr pVisualNew = NULL;
    VisualID *orig_vid = NULL;
    __GLcontextModes *modes;
    __GLXvisualConfig *pNewVisualConfigs = NULL;
    void **glXVisualPriv;
    void **pNewVisualPriv;
    int found_default;
    int i, j, k;

    GLAQUA_DEBUG_MSG("init_visuals\n");

    if (numConfigs > 0)
        numNewConfigs = numConfigs;
    else
        numNewConfigs = NUM_FALLBACK_CONFIGS;

    /* Alloc space for the list of new GLX visuals */
    pNewVisualConfigs = (__GLXvisualConfig *)
                     malloc(numNewConfigs * sizeof(__GLXvisualConfig));
    if (!pNewVisualConfigs) {
        return FALSE;
    }

    /* Alloc space for the list of new GLX visual privates */
    pNewVisualPriv = (void **) malloc(numNewConfigs * sizeof(void *));
    if (!pNewVisualPriv) {
        free(pNewVisualConfigs);
        return FALSE;
    }

    /*
    ** If SetVisualConfigs was not called, then use default GLX
    ** visual configs.
    */
    if (numConfigs == 0) {
        memcpy(pNewVisualConfigs, FallbackConfigs,
               NUM_FALLBACK_CONFIGS * sizeof(__GLXvisualConfig));
        memset(pNewVisualPriv, 0, NUM_FALLBACK_CONFIGS * sizeof(void *));
    }
    else {
        /* copy driver's visual config info */
        for (i = 0; i < numConfigs; i++) {
            pNewVisualConfigs[i] = visualConfigs[i];
            pNewVisualPriv[i] = visualPrivates[i];
        }
    }

    /* Count the number of RGB and CI visual configs */
    numRGBconfigs = 0;
    numCIconfigs = 0;
    for (i = 0; i < numNewConfigs; i++) {
        if (pNewVisualConfigs[i].rgba)
            numRGBconfigs++;
        else
            numCIconfigs++;
    }

    /* Count the total number of visuals to compute */
    numNewVisuals = 0;
    for (i = 0; i < numVisuals; i++) {
        int count;

        count = ((pVisual[i].class == TrueColor ||
                  pVisual[i].class == DirectColor)
                ? numRGBconfigs : numCIconfigs);
        if (count == 0)
            count = 1;          /* preserve the existing visual */

        numNewVisuals += count;
    }

    /* Reset variables for use with the next screen/driver's visual configs */
    visualConfigs = NULL;
    numConfigs = 0;

    /* Alloc temp space for the list of orig VisualIDs for each new visual */
    orig_vid = (VisualID *)malloc(numNewVisuals * sizeof(VisualID));
    if (!orig_vid) {
        free(pNewVisualPriv);
        free(pNewVisualConfigs);
        return FALSE;
    }

    /* Alloc space for the list of glXVisuals */
    modes = _gl_context_modes_create(numNewVisuals, sizeof(__GLcontextModes));
    if (modes == NULL) {
        free(orig_vid);
        free(pNewVisualPriv);
        free(pNewVisualConfigs);
        return FALSE;
    }

    /* Alloc space for the list of glXVisualPrivates */
    glXVisualPriv = (void **)malloc(numNewVisuals * sizeof(void *));
    if (!glXVisualPriv) {
        _gl_context_modes_destroy( modes );
        free(orig_vid);
        free(pNewVisualPriv);
        free(pNewVisualConfigs);
        return FALSE;
    }

    /* Alloc space for the new list of the X server's visuals */
    pVisualNew = (VisualPtr)malloc(numNewVisuals * sizeof(VisualRec));
    if (!pVisualNew) {
        free(glXVisualPriv);
        _gl_context_modes_destroy( modes );
        free(orig_vid);
        free(pNewVisualPriv);
        free(pNewVisualConfigs);
        return FALSE;
    }

    /* Initialize the new visuals */
    found_default = FALSE;
    glAquaScreens[screenInfo.numScreens-1].modes = modes;
    for (i = j = 0; i < numVisuals; i++) {
        int is_rgb = (pVisual[i].class == TrueColor ||
                      pVisual[i].class == DirectColor);

        if (!is_rgb)
        {
            /* We don't support non-rgb visuals for GL. But we don't
               want to remove them either, so just pass them through
               with null glX configs */

            pVisualNew[j] = pVisual[i];
            pVisualNew[j].vid = FakeClientID(0);

            /* Check for the default visual */
            if (!found_default && pVisual[i].vid == *defaultVisp) {
                *defaultVisp = pVisualNew[j].vid;
                found_default = TRUE;
            }

            /* Save the old VisualID */
            orig_vid[j] = pVisual[i].vid;

            /* Initialize the glXVisual */
            _gl_copy_visual_to_context_mode( modes, & NullConfig );
            modes->visualID = pVisualNew[j].vid;

            j++;

            continue;
        }

        for (k = 0; k < numNewConfigs; k++) {
            if (pNewVisualConfigs[k].rgba != is_rgb)
                continue;

            assert( modes != NULL );

            /* Initialize the new visual */
            pVisualNew[j] = pVisual[i];
            pVisualNew[j].vid = FakeClientID(0);

            /* Check for the default visual */
            if (!found_default && pVisual[i].vid == *defaultVisp) {
                *defaultVisp = pVisualNew[j].vid;
                found_default = TRUE;
            }

            /* Save the old VisualID */
            orig_vid[j] = pVisual[i].vid;

            /* Initialize the glXVisual */
            _gl_copy_visual_to_context_mode( modes, & pNewVisualConfigs[k] );
            modes->visualID = pVisualNew[j].vid;

            /*
             * If the class is -1, then assume the X visual information
             * is identical to what GLX needs, and take them from the X
             * visual.  NOTE: if class != -1, then all other fields MUST
             * be initialized.
             */
            if (modes->visualType == GLX_NONE) {
                modes->visualType = _gl_convert_from_x_visual_type( pVisual[i].class );
                modes->redBits    = count_bits(pVisual[i].redMask);
                modes->greenBits  = count_bits(pVisual[i].greenMask);
                modes->blueBits   = count_bits(pVisual[i].blueMask);
                modes->alphaBits  = modes->alphaBits;
                modes->redMask    = pVisual[i].redMask;
                modes->greenMask  = pVisual[i].greenMask;
                modes->blueMask   = pVisual[i].blueMask;
                modes->alphaMask  = modes->alphaMask;
                modes->rgbBits = (is_rgb)
                    ? (modes->redBits + modes->greenBits +
                       modes->blueBits + modes->alphaBits)
                    : rootDepth;
            }

            /* Save the device-dependent private for this visual */
            glXVisualPriv[j] = pNewVisualPriv[k];

            j++;
            modes = modes->next;
        }
    }

    assert(j <= numNewVisuals);

    /* Save the GLX visuals in the screen structure */
    glAquaScreens[screenInfo.numScreens-1].num_vis = numNewVisuals;
    //    glAquaScreens[screenInfo.numScreens-1].priv = glXVisualPriv;

    /* set up depth's VisualIDs */
    for (i = 0; i < ndepth; i++) {
        int numVids = 0;
        VisualID *pVids = NULL;
        int k, n = 0;

        /* Count the new number of VisualIDs at this depth */
        for (j = 0; j < pdepth[i].numVids; j++)
            for (k = 0; k < numNewVisuals; k++)
            if (pdepth[i].vids[j] == orig_vid[k])
                numVids++;

        /* Allocate a new list of VisualIDs for this depth */
        pVids = (VisualID *)malloc(numVids * sizeof(VisualID));

        /* Initialize the new list of VisualIDs for this depth */
        for (j = 0; j < pdepth[i].numVids; j++)
            for (k = 0; k < numNewVisuals; k++)
            if (pdepth[i].vids[j] == orig_vid[k])
                pVids[n++] = pVisualNew[k].vid;

        /* Update this depth's list of VisualIDs */
        free(pdepth[i].vids);
        pdepth[i].vids = pVids;
        pdepth[i].numVids = numVids;
    }

    /* Update the X server's visuals */
    *nvisualp = numNewVisuals;
    *visualp = pVisualNew;

    /* Free the old list of the X server's visuals */
    free(pVisual);

    /* Clean up temporary allocations */
    free(orig_vid);
    free(pNewVisualPriv);
    free(pNewVisualConfigs);

    /* Free the private list created by DDX HW driver */
    if (visualPrivates)
        free(visualPrivates);
    visualPrivates = NULL;

    return TRUE;
}

Bool enable_stereo = FALSE;
/* based on code in i830_dri.c
   This ends calling glAquaSetVisualConfigs to set the static
   numconfigs, etc. */
static void
glAquaInitVisualConfigs(void)
{
    int                 lclNumConfigs     = 0;
    __GLXvisualConfig  *lclVisualConfigs  = NULL;
    void              **lclVisualPrivates = NULL;

    int stereo, depth, aux, buffers, stencil, accum;
    int i = 0;

    GLAQUA_DEBUG_MSG("glAquaInitVisualConfigs ");
        
    /* count num configs:
        2 stereo (on, off) (optional)
        2 Z buffer (0, 24 bit)
        2 AUX buffer (0, 2)
        2 buffers (single, double)
        2 stencil (0, 8 bit)
        2 accum (0, 64 bit)
        = 64 configs with stereo, or 32 without */

    if (enable_stereo) lclNumConfigs = 2 * 2 * 2 * 2 * 2 * 2; /* 64 */
    else               lclNumConfigs = 2 * 2 * 2 * 2 * 2; /* 32 */

    /* alloc */
    lclVisualConfigs = xcalloc(sizeof(__GLXvisualConfig), lclNumConfigs);
    lclVisualPrivates = xcalloc(sizeof(void *), lclNumConfigs);

    /* fill in configs */
    if (NULL != lclVisualConfigs) {
        i = 0; /* current buffer */
        for (stereo = 0; stereo < (enable_stereo ? 2 : 1); stereo++) {
	  for (depth = 0; depth < 2; depth++) {
            for (aux = 0; aux < 2; aux++) {
	      for (buffers = 0; buffers < 2; buffers++) {
		for (stencil = 0; stencil < 2; stencil++) {
		  for (accum = 0; accum < 2; accum++) {
		    lclVisualConfigs[i].vid = -1;
		    lclVisualConfigs[i].class = -1;
		    lclVisualConfigs[i].rgba = TRUE;
		    lclVisualConfigs[i].redSize = -1;
		    lclVisualConfigs[i].greenSize = -1;
		    lclVisualConfigs[i].blueSize = -1;
		    lclVisualConfigs[i].redMask = -1;
		    lclVisualConfigs[i].greenMask = -1;
		    lclVisualConfigs[i].blueMask = -1;
		    lclVisualConfigs[i].alphaMask = 0;
		    if (accum) {
		      lclVisualConfigs[i].accumRedSize = 16;
		      lclVisualConfigs[i].accumGreenSize = 16;
		      lclVisualConfigs[i].accumBlueSize = 16;
		      lclVisualConfigs[i].accumAlphaSize = 16;
		    } else {
		      lclVisualConfigs[i].accumRedSize = 0;
		      lclVisualConfigs[i].accumGreenSize = 0;
		      lclVisualConfigs[i].accumBlueSize = 0;
		      lclVisualConfigs[i].accumAlphaSize = 0;
		    }
		    lclVisualConfigs[i].doubleBuffer = buffers ? TRUE : FALSE;
		    lclVisualConfigs[i].stereo = stereo ? TRUE : FALSE;
		    lclVisualConfigs[i].bufferSize = -1;
		    
		    lclVisualConfigs[i].depthSize = depth? 24 : 0;
		    lclVisualConfigs[i].stencilSize = stencil ? 8 : 0;
		    lclVisualConfigs[i].auxBuffers = aux ? 2 : 0;
		    lclVisualConfigs[i].level = 0;
		    lclVisualConfigs[i].visualRating = GLX_NONE_EXT;
		    lclVisualConfigs[i].transparentPixel = 0;
		    lclVisualConfigs[i].transparentRed = 0;
		    lclVisualConfigs[i].transparentGreen = 0;
		    lclVisualConfigs[i].transparentBlue = 0;
		    lclVisualConfigs[i].transparentAlpha = 0;
		    lclVisualConfigs[i].transparentIndex = 0;
		    i++;
		  }
		}
	      }
            }
	  }
	}
    }
    if (i != lclNumConfigs)
        GLAQUA_DEBUG_MSG("glAquaInitVisualConfigs failed to alloc visual configs");

    GlxSetVisualConfigs(lclNumConfigs, lclVisualConfigs, lclVisualPrivates);
}


static void glAquaSetVisualConfigs(int nconfigs, __GLXvisualConfig *configs,
                                   void **privates)
{
    GLAQUA_DEBUG_MSG("glAquaSetVisualConfigs\n");

    numConfigs = nconfigs;
    visualConfigs = configs;
    visualPrivates = privates;
}

static Bool glAquaInitVisuals(VisualPtr *visualp, DepthPtr *depthp,
                              int *nvisualp, int *ndepthp,
                              int *rootDepthp, VisualID *defaultVisp,
                              unsigned long sizes, int bitsPerRGB)
{
    GLAQUA_DEBUG_MSG("glAquaInitVisuals\n");
    
    if (numConfigs == 0) /* if no configs */
        glAquaInitVisualConfigs(); /* ensure the visual configs are setup */

    /*
     * setup the visuals supported by this particular screen.
     */
    return init_visuals(nvisualp, visualp, defaultVisp,
                        *ndepthp, *depthp, *rootDepthp);
}

#if 0
static void fixup_visuals(int screen)
{
    ScreenPtr pScreen = screenInfo.screens[screen];
    glAquaScreenRec *pScr = &glAquaScreens[screen];
    int j;
    __GLcontextModes *modes;

    GLAQUA_DEBUG_MSG("fixup_visuals\n");

    for ( modes = pScr->modes ; modes != NULL ; modes = modes->next ) {
        const int vis_class = _gl_convert_to_x_visual_type( modes->visualType );
        const int nplanes = (modes->rgbBits - modes->alphaBits);
        const VisualPtr pVis = pScreen->visuals;

        /* Find a visual that matches the GLX visual's class and size */
        for (j = 0; j < pScreen->numVisuals; j++) {
            if (pVis[j].class == vis_class &&
            pVis[j].nplanes == nplanes) {

            /* Fixup the masks */
            modes->redMask   = pVis[j].redMask;
            modes->greenMask = pVis[j].greenMask;
            modes->blueMask  = pVis[j].blueMask;

            /* Recalc the sizes */
            modes->redBits   = count_bits(modes->redMask);
            modes->greenBits = count_bits(modes->greenMask);
            modes->blueBits  = count_bits(modes->blueMask);
            }
        }
    }
}
#endif
static void __glXAquaScreenDestroy(__GLXscreen *screen) {

	GLAQUA_DEBUG_MSG("glXAquaScreenDestroy(%p)\n", screen);
  __glXScreenDestroy(screen);

  free(screen);
}

static void init_screen_visuals(__GLXAquaScreen *screen) {
  ScreenPtr pScreen = screen->base.pScreen;
  
  __GLcontextModes *modes;
  int *used;
  int i, j;
  
    GLAQUA_DEBUG_MSG("init_screen_visuals\n");

    /* FIXME: Change 'used' to be a array of bits (rather than of ints),
     * FIXME: create a stack array of 8 or 16 bytes.  If 'numVisuals' is less
     * FIXME: than 64 or 128 the stack array can be used instead of calling
     * FIXME: malloc / free.  If nothing else, convert 'used' to
     * FIXME: array of bytes instead of ints!
     */
    used = (int *)malloc(pScreen->numVisuals * sizeof(int));
    memset(used, 0, pScreen->numVisuals * sizeof(int));

    i = 0;
    for ( modes = screen -> base.modes
          ; modes != NULL
          ; modes = modes->next ) {
        const int vis_class = _gl_convert_to_x_visual_type( modes->visualType );
        const int nplanes = (modes->rgbBits - modes->alphaBits);
        const VisualPtr pVis = pScreen->visuals;

        for (j = 0; j < pScreen->numVisuals; j++) {
            if (pVis[j].class     == vis_class &&
                pVis[j].nplanes   == nplanes &&
                pVis[j].redMask   == modes->redMask &&
                pVis[j].greenMask == modes->greenMask &&
                pVis[j].blueMask  == modes->blueMask &&
                !used[j]) {

                    /* set the VisualID */
                    modes->visualID = pVis[j].vid;

                    /* Mark this visual used */
                    used[j] = 1;
                    break;
            }
        }
        if ( j == pScreen->numVisuals ) {
            ErrorF("No matching visual for __GLcontextMode with "
                   "visual class = %d (%d), nplanes = %u\n",
                   vis_class, 
                   (int)modes->visualType,
                   (unsigned int)(modes->rgbBits - modes->alphaBits) );
        }
        else if ( modes->visualID == -1 ) {
            FatalError( "Matching visual found, but visualID still -1!\n" );
        }

        i++;
    }

    free(used);
}

static __GLXscreen * __glXAquaScreenProbe(ScreenPtr pScreen) {
  __GLXAquaScreen *screen;
  GLAQUA_DEBUG_MSG("glXAquaScreenProbe\n");
  if (screen == NULL) return NULL;

  screen = malloc(sizeof *screen);

  __glXScreenInit(&screen->base, pScreen);

  screen->base.destroy        = __glXAquaScreenDestroy;
  screen->base.createContext  = __glXAquaScreenCreateContext;
  screen->base.createDrawable = __glXAquaScreenCreateDrawable;
  screen->base.pScreen       = pScreen;

  init_screen_visuals(screen);

  return &screen->base;
}

static void __glXAquaDrawableDestroy(__GLXdrawable *base) {
    GLAQUA_DEBUG_MSG("glAquaDestroyDrawablePrivate\n");

    /* It doesn't work to call DRIDestroySurface here, the drawable's
       already gone.. But dri.c notices the window destruction and
       frees the surface itself. */

    free(base);
}

static __GLXdrawable *
__glXAquaScreenCreateDrawable(__GLXscreen *screen,
			      DrawablePtr pDraw,
			      XID drawId,
			      __GLcontextModes *modes) {
  __GLXAquaDrawable *glxPriv;

  GLAQUA_DEBUG_MSG("glAquaScreenCreateDrawable(%p,%p,%d,%p)\n", context, pDraw, drawId, modes);

  glxPriv = xalloc(sizeof *glxPriv);
  if (glxPriv == NULL) return NULL;

  memset(glxPriv, 0, sizeof *glxPriv);

  if (!__glXDrawableInit(&glxPriv->base, screen, pDraw, drawId, modes)) {
    xfree(glxPriv);
    return NULL;
  }

  glxPriv->base.destroy       = __glXAquaDrawableDestroy;
  glxPriv->base.resize        = __glXAquaDrawableResize;
  glxPriv->base.swapBuffers   = __glXAquaDrawableSwapBuffers;
  //  glxPriv->base.copySubBuffer = __glXAquaDrawableCopySubBuffer;

  return &glxPriv->base;
}

static void glAquaResetExtension(void)
{
    GLAQUA_DEBUG_MSG("glAquaResetExtension\n");
    CGLSetOption(kCGLGOResetLibrary, GL_TRUE);
}

// Extra goodies for glx

GLuint __glFloorLog2(GLuint val)
{
    int c = 0;

    while (val > 1) {
        c++;
        val >>= 1;
    }
    return c;
}

void warn_func(void * p1, char *format, ...) {
  va_list v;
  va_start(v, format);
  vfprintf(stderr, format, v);
  va_end(v);
}

static void setup_dispatch_table(void) {
  struct _glapi_table *disp=_glapi_get_dispatch();
  _glapi_set_warning_func((_glapi_warning_func)warn_func);
  _glapi_noop_enable_warnings(TRUE);

  SET_Accum(disp, glAccum);
  SET_ActiveStencilFaceEXT(disp, glActiveStencilFaceEXT);
  SET_ActiveTextureARB(disp, glActiveTextureARB);
//SET_AlphaFragmentOp1ATI(disp, glAlphaFragmentOp1EXT);   // <-- EXT -> ATI
//SET_AlphaFragmentOp2ATI(disp, glAlphaFragmentOp2EXT);
//SET_AlphaFragmentOp3ATI(disp, glAlphaFragmentOp3EXT);
  SET_AlphaFunc(disp, glAlphaFunc);
//SET_AreProgramsResidentNV(disp, glAreProgramsResidentNV);
  SET_AreTexturesResident(disp, glAreTexturesResident); 
  SET_ArrayElement(disp, glArrayElement);
  SET_AttachObjectARB(disp, glAttachObjectARB);
  SET_Begin(disp, glBegin);
//SET_BeginFragmentShaderATI(disp, glBeginFragmentShaderEXT);   // <-- EXT -> ATI
  SET_BeginQueryARB(disp, glBeginQueryARB);
  SET_BindAttribLocationARB(disp, glBindAttribLocationARB);
  SET_BindBufferARB(disp, glBindBufferARB);
//SET_BindFragmentShaderATI(disp, glBindFragmentShaderEXT);     // <-- EXT -> ATI
  SET_BindFramebufferEXT(disp, glBindFramebufferEXT);
//SET_BindProgramNV(disp, glBindProgramNV);
  SET_BindRenderbufferEXT(disp, glBindRenderbufferEXT);
  SET_BindTexture(disp, glBindTexture);
  SET_Bitmap(disp, glBitmap);
  SET_BlendColor(disp, glBlendColor);
  SET_BlendEquation(disp, glBlendEquation);
  SET_BlendEquationSeparateEXT(disp, glBlendEquationSeparateEXT);
  SET_BlendFunc(disp, glBlendFunc);
  SET_BlendFuncSeparateEXT(disp, glBlendFuncSeparateEXT);
  SET_BlitFramebufferEXT(disp, glBlitFramebufferEXT);
  SET_BufferDataARB(disp, glBufferDataARB);
  SET_BufferSubDataARB(disp, glBufferSubDataARB);
  SET_CallList(disp, glCallList);
  SET_CallLists(disp, glCallLists);
  SET_CheckFramebufferStatusEXT(disp, glCheckFramebufferStatusEXT);
  SET_Clear(disp, glClear);
  SET_ClearAccum(disp, glClearAccum);
  SET_ClearColor(disp, glClearColor);
  SET_ClearDepth(disp, glClearDepth);
  SET_ClearIndex(disp, glClearIndex);
  SET_ClearStencil(disp, glClearStencil);
  SET_ClientActiveTextureARB(disp, glClientActiveTextureARB);
  SET_ClipPlane(disp, glClipPlane);
  SET_Color3b(disp, glColor3b);
  SET_Color3bv(disp, glColor3bv);
  SET_Color3d(disp, glColor3d);
  SET_Color3dv(disp, glColor3dv);
  SET_Color3f(disp, glColor3f);
  SET_Color3fv(disp, glColor3fv);
  SET_Color3i(disp, glColor3i);
  SET_Color3iv(disp, glColor3iv);
  SET_Color3s(disp, glColor3s);
  SET_Color3sv(disp, glColor3sv);
  SET_Color3ub(disp, glColor3ub);
  SET_Color3ubv(disp, glColor3ubv);
  SET_Color3ui(disp, glColor3ui);
  SET_Color3uiv(disp, glColor3uiv);
  SET_Color3us(disp, glColor3us);
  SET_Color3usv(disp, glColor3usv);
  SET_Color4b(disp, glColor4b);
  SET_Color4bv(disp, glColor4bv);
  SET_Color4d(disp, glColor4d);
  SET_Color4dv(disp, glColor4dv);
  SET_Color4f(disp, glColor4f);
  SET_Color4fv(disp, glColor4fv);
  SET_Color4i(disp, glColor4i);
  SET_Color4iv(disp, glColor4iv);
  SET_Color4s(disp, glColor4s);
  SET_Color4sv(disp, glColor4sv);
  SET_Color4ub(disp, glColor4ub);
  SET_Color4ubv(disp, glColor4ubv);
  SET_Color4ui(disp, glColor4ui);
  SET_Color4uiv(disp, glColor4uiv);
  SET_Color4us(disp, glColor4us);
  SET_Color4usv(disp, glColor4usv);
//SET_ColorFragmentOp1ATI(disp, glColorFragmentOp1EXT);    // <-- EXT -> ATI
//SET_ColorFragmentOp2ATI(disp, glColorFragmentOp2EXT);
//SET_ColorFragmentOp3ATI(disp, glColorFragmentOp3EXT);
  SET_ColorMask(disp, glColorMask);
  SET_ColorMaterial(disp, glColorMaterial);
  SET_ColorPointer(disp, glColorPointer);
//SET_ColorPointerEXT(disp, glColorPointerEXT);
  SET_ColorSubTable(disp, glColorSubTable);
  SET_ColorTable(disp, glColorTable);
  SET_ColorTableParameterfv(disp, glColorTableParameterfv);
  SET_ColorTableParameteriv(disp, glColorTableParameteriv);
  SET_CombinerInputNV(disp, glCombinerInputNV);
  SET_CombinerOutputNV(disp, glCombinerOutputNV);
  SET_CombinerParameterfNV(disp, glCombinerParameterfNV);
  SET_CombinerParameterfvNV(disp, glCombinerParameterfvNV);
  SET_CombinerParameteriNV(disp, glCombinerParameteriNV);
  SET_CombinerParameterivNV(disp, glCombinerParameterivNV);
  SET_CompileShaderARB(disp, glCompileShaderARB);
  SET_CompressedTexImage1DARB(disp, glCompressedTexImage1DARB);
  SET_CompressedTexImage2DARB(disp, glCompressedTexImage2DARB);
  SET_CompressedTexImage3DARB(disp, glCompressedTexImage3DARB);
  SET_CompressedTexSubImage1DARB(disp, glCompressedTexSubImage1DARB);
  SET_CompressedTexSubImage2DARB(disp, glCompressedTexSubImage2DARB);
  SET_CompressedTexSubImage3DARB(disp, glCompressedTexSubImage3DARB);
  SET_ConvolutionFilter1D(disp, glConvolutionFilter1D);
  SET_ConvolutionFilter2D(disp, glConvolutionFilter2D);
  SET_ConvolutionParameterf(disp, glConvolutionParameterf);
  SET_ConvolutionParameterfv(disp, glConvolutionParameterfv);
  SET_ConvolutionParameteri(disp, glConvolutionParameteri);
  SET_ConvolutionParameteriv(disp, glConvolutionParameteriv);
  SET_CopyColorSubTable(disp, glCopyColorSubTable);
  SET_CopyColorTable(disp, glCopyColorTable);
  SET_CopyConvolutionFilter1D(disp, glCopyConvolutionFilter1D);
  SET_CopyConvolutionFilter2D(disp, glCopyConvolutionFilter2D);
  SET_CopyPixels(disp, glCopyPixels);
  SET_CopyTexImage1D(disp, glCopyTexImage1D);
  SET_CopyTexImage2D(disp, glCopyTexImage2D);
  SET_CopyTexSubImage1D(disp, glCopyTexSubImage1D);
  SET_CopyTexSubImage2D(disp, glCopyTexSubImage2D);
  SET_CopyTexSubImage3D(disp, glCopyTexSubImage3D);
  SET_CreateProgramObjectARB(disp, glCreateProgramObjectARB);
  SET_CreateShaderObjectARB(disp, glCreateShaderObjectARB);
  SET_CullFace(disp, glCullFace);
//SET_CullParameterdvEXT(disp, glCullParameterdvEXT);
//SET_CullParameterfvEXT(disp, glCullParameterfvEXT);
  SET_DeleteBuffersARB(disp, glDeleteBuffersARB);
  SET_DeleteFencesNV(disp, glDeleteFencesAPPLE);
//SET_DeleteFragmentShaderATI(disp, glDeleteFragmentShaderEXT);      // <-- EXT -> ATI
  SET_DeleteFramebuffersEXT(disp, glDeleteFramebuffersEXT);
  SET_DeleteLists(disp, glDeleteLists);
  SET_DeleteObjectARB(disp, glDeleteObjectARB);
//SET_DeleteProgramsNV(disp, glDeleteProgramsNV);
  SET_DeleteQueriesARB(disp, glDeleteQueriesARB);
  SET_DeleteRenderbuffersEXT(disp, glDeleteRenderbuffersEXT);
  SET_DeleteTextures(disp, glDeleteTextures);
  SET_DepthBoundsEXT(disp, glDepthBoundsEXT);
  SET_DepthFunc(disp, glDepthFunc);
  SET_DepthMask(disp, glDepthMask);
  SET_DepthRange(disp, glDepthRange);
  SET_DetachObjectARB(disp, glDetachObjectARB);
  SET_Disable(disp, glDisable);
  SET_DisableClientState(disp, glDisableClientState);
  SET_DisableVertexAttribArrayARB(disp, glDisableVertexAttribArrayARB);
  SET_DrawArrays(disp, glDrawArrays);
  SET_DrawBuffer(disp, glDrawBuffer);
  SET_DrawBuffersARB(disp, glDrawBuffersARB);
  SET_DrawElements(disp, glDrawElements);
  SET_DrawPixels(disp, glDrawPixels);
  SET_DrawRangeElements(disp, glDrawRangeElements);
  SET_EdgeFlag(disp, glEdgeFlag);
  SET_EdgeFlagPointer(disp, glEdgeFlagPointer);
//SET_EdgeFlagPointerEXT(disp, glEdgeFlagPointerEXT);
  SET_EdgeFlagv(disp, glEdgeFlagv);
  SET_Enable(disp, glEnable);
  SET_EnableClientState(disp, glEnableClientState);
  SET_EnableVertexAttribArrayARB(disp, glEnableVertexAttribArrayARB);
  SET_End(disp, glEnd);
//SET_EndFragmentShaderATI(disp, glEndFragmentShaderEXT);        // <-- EXT -> ATI
  SET_EndList(disp, glEndList);
  SET_EndQueryARB(disp, glEndQueryARB);
  SET_EvalCoord1d(disp, glEvalCoord1d);
  SET_EvalCoord1dv(disp, glEvalCoord1dv);
  SET_EvalCoord1f(disp, glEvalCoord1f);
  SET_EvalCoord1fv(disp, glEvalCoord1fv);
  SET_EvalCoord2d(disp, glEvalCoord2d);
  SET_EvalCoord2dv(disp, glEvalCoord2dv);
  SET_EvalCoord2f(disp, glEvalCoord2f);
  SET_EvalCoord2fv(disp, glEvalCoord2fv);
  SET_EvalMesh1(disp, glEvalMesh1);
  SET_EvalMesh2(disp, glEvalMesh2);
  SET_EvalPoint1(disp, glEvalPoint1);
  SET_EvalPoint2(disp, glEvalPoint2);
//SET_ExecuteProgramNV(disp, glExecuteProgramNV);
  SET_FeedbackBuffer(disp, glFeedbackBuffer);
  SET_FinalCombinerInputNV(disp, glFinalCombinerInputNV);
  SET_Finish(disp, glFinish);
  SET_FinishFenceNV(disp, glFinishFenceAPPLE);       // <-- APPLE -> NV
  SET_Flush(disp, glFlush);
//SET_FlushVertexArrayRangeNV(disp, glFlushVertexArrayRangeNV);
  SET_FogCoordPointerEXT(disp, glFogCoordPointerEXT);
  SET_FogCoorddEXT(disp, glFogCoorddEXT);
  SET_FogCoorddvEXT(disp, glFogCoorddvEXT);
  SET_FogCoordfEXT(disp, glFogCoordfEXT);
  SET_FogCoordfvEXT(disp, glFogCoordfvEXT);
  SET_Fogf(disp, glFogf);
  SET_Fogfv(disp, glFogfv);
  SET_Fogi(disp, glFogi);
  SET_Fogiv(disp, glFogiv);
  SET_FramebufferRenderbufferEXT(disp, glFramebufferRenderbufferEXT);
  SET_FramebufferTexture1DEXT(disp, glFramebufferTexture1DEXT);
  SET_FramebufferTexture2DEXT(disp, glFramebufferTexture2DEXT);
  SET_FramebufferTexture3DEXT(disp, glFramebufferTexture3DEXT);
  SET_FrontFace(disp, glFrontFace);
  SET_Frustum(disp, glFrustum);
  SET_GenBuffersARB(disp, glGenBuffersARB);
  SET_GenFencesNV(disp, glGenFencesAPPLE);            // <-- APPLE -> NV
//SET_GenFragmentShadersATI(disp, glGenFragmentShadersEXT);         // <-- EXT -> ATI
  SET_GenFramebuffersEXT(disp, glGenFramebuffersEXT);
  SET_GenLists(disp, glGenLists);
//SET_GenProgramsNV(disp, glGenProgramsNV);
  SET_GenQueriesARB(disp, glGenQueriesARB);
  SET_GenRenderbuffersEXT(disp, glGenRenderbuffersEXT);
  SET_GenTextures(disp, glGenTextures);
  SET_GenerateMipmapEXT(disp, glGenerateMipmapEXT);
  SET_GetActiveAttribARB(disp, glGetActiveAttribARB);
  SET_GetActiveUniformARB(disp, glGetActiveUniformARB);
  SET_GetAttachedObjectsARB(disp, glGetAttachedObjectsARB);
  SET_GetAttribLocationARB(disp, glGetAttribLocationARB);
  SET_GetBooleanv(disp, glGetBooleanv);
  SET_GetBufferParameterivARB(disp, glGetBufferParameterivARB);
  SET_GetBufferPointervARB(disp, glGetBufferPointervARB);
  SET_GetBufferSubDataARB(disp, glGetBufferSubDataARB);
  SET_GetClipPlane(disp, glGetClipPlane);
  SET_GetColorTable(disp, glGetColorTable);
  SET_GetColorTableParameterfv(disp, glGetColorTableParameterfv);
  SET_GetColorTableParameteriv(disp, glGetColorTableParameteriv);
  SET_GetCombinerInputParameterfvNV(disp, glGetCombinerInputParameterfvNV);
  SET_GetCombinerInputParameterivNV(disp, glGetCombinerInputParameterivNV);
  SET_GetCombinerOutputParameterfvNV(disp, glGetCombinerOutputParameterfvNV);
  SET_GetCombinerOutputParameterivNV(disp, glGetCombinerOutputParameterivNV);
  SET_GetCompressedTexImageARB(disp, glGetCompressedTexImageARB);
  SET_GetConvolutionFilter(disp, glGetConvolutionFilter);
  SET_GetConvolutionParameterfv(disp, glGetConvolutionParameterfv);
  SET_GetConvolutionParameteriv(disp, glGetConvolutionParameteriv);
  SET_GetDoublev(disp, glGetDoublev);
  SET_GetError(disp, glGetError);
//SET_GetFenceivNV(disp, glGetFenceivNV);
  SET_GetFinalCombinerInputParameterfvNV(disp, glGetFinalCombinerInputParameterfvNV);
  SET_GetFinalCombinerInputParameterivNV(disp, glGetFinalCombinerInputParameterivNV);
  SET_GetFloatv(disp, glGetFloatv);
  SET_GetFramebufferAttachmentParameterivEXT(disp, glGetFramebufferAttachmentParameterivEXT);
  SET_GetHandleARB(disp, glGetHandleARB);
  SET_GetHistogram(disp, glGetHistogram);
  SET_GetHistogramParameterfv(disp, glGetHistogramParameterfv);
  SET_GetHistogramParameteriv(disp, glGetHistogramParameteriv);
  SET_GetInfoLogARB(disp, glGetInfoLogARB);
  SET_GetIntegerv(disp, glGetIntegerv);
  SET_GetLightfv(disp, glGetLightfv);
  SET_GetLightiv(disp, glGetLightiv);
  SET_GetMapdv(disp, glGetMapdv);
  SET_GetMapfv(disp, glGetMapfv);
  SET_GetMapiv(disp, glGetMapiv);
  SET_GetMaterialfv(disp, glGetMaterialfv);
  SET_GetMaterialiv(disp, glGetMaterialiv);
  SET_GetMinmax(disp, glGetMinmax);
  SET_GetMinmaxParameterfv(disp, glGetMinmaxParameterfv);
  SET_GetMinmaxParameteriv(disp, glGetMinmaxParameteriv);
  SET_GetObjectParameterfvARB(disp, glGetObjectParameterfvARB);
  SET_GetObjectParameterivARB(disp, glGetObjectParameterivARB);
  SET_GetPixelMapfv(disp, glGetPixelMapfv);
  SET_GetPixelMapuiv(disp, glGetPixelMapuiv);
  SET_GetPixelMapusv(disp, glGetPixelMapusv);
//SET_GetPixelTexGenParameterfvSGIS(disp, glGetPixelTexGenParameterfvSGIS);
//SET_GetPixelTexGenParameterivSGIS(disp, glGetPixelTexGenParameterivSGIS);
  SET_GetPointerv(disp, glGetPointerv);
  SET_GetPolygonStipple(disp, glGetPolygonStipple);
  SET_GetProgramEnvParameterdvARB(disp, glGetProgramEnvParameterdvARB);
  SET_GetProgramEnvParameterfvARB(disp, glGetProgramEnvParameterfvARB);
  SET_GetProgramLocalParameterdvARB(disp, glGetProgramLocalParameterdvARB);
  SET_GetProgramLocalParameterfvARB(disp, glGetProgramLocalParameterfvARB);
//SET_GetProgramNamedParameterdvNV(disp, glGetProgramNamedParameterdvNV);
//SET_GetProgramNamedParameterfvNV(disp, glGetProgramNamedParameterfvNV);
//SET_GetProgramParameterdvNV(disp, glGetProgramParameterdvNV);
//SET_GetProgramParameterfvNV(disp, glGetProgramParameterfvNV);
  SET_GetProgramStringARB(disp, glGetProgramStringARB);
//SET_GetProgramStringNV(disp, glGetProgramStringNV);
  SET_GetProgramivARB(disp, glGetProgramivARB);
//SET_GetProgramivNV(disp, glGetProgramivNV);
//SET_GetQueryObjecti64vEXT(disp, glGetQueryObjecti64vEXT);
  SET_GetQueryObjectivARB(disp, glGetQueryObjectivARB);
//SET_GetQueryObjectui64vEXT(disp, glGetQueryObjectui64vEXT);
  SET_GetQueryObjectuivARB(disp, glGetQueryObjectuivARB);
  SET_GetQueryivARB(disp, glGetQueryivARB);
  SET_GetRenderbufferParameterivEXT(disp, glGetRenderbufferParameterivEXT);
  SET_GetSeparableFilter(disp, glGetSeparableFilter);
  SET_GetShaderSourceARB(disp, glGetShaderSourceARB);
  SET_GetString(disp, glGetString);
  SET_GetTexEnvfv(disp, glGetTexEnvfv);
  SET_GetTexEnviv(disp, glGetTexEnviv);
  SET_GetTexGendv(disp, glGetTexGendv);
  SET_GetTexGenfv(disp, glGetTexGenfv);
  SET_GetTexGeniv(disp, glGetTexGeniv);
  SET_GetTexImage(disp, glGetTexImage);
  SET_GetTexLevelParameterfv(disp, glGetTexLevelParameterfv);
  SET_GetTexLevelParameteriv(disp, glGetTexLevelParameteriv);
  SET_GetTexParameterfv(disp, glGetTexParameterfv);
  SET_GetTexParameteriv(disp, glGetTexParameteriv);
//SET_GetTrackMatrixivNV(disp, glGetTrackMatrixivNV);
  SET_GetUniformLocationARB(disp, glGetUniformLocationARB);
  SET_GetUniformfvARB(disp, glGetUniformfvARB);
  SET_GetUniformivARB(disp, glGetUniformivARB);
//SET_GetVertexAttribPointervNV(disp, glGetVertexAttribPointervNV);
  SET_GetVertexAttribdvARB(disp, glGetVertexAttribdvARB);
//SET_GetVertexAttribdvNV(disp, glGetVertexAttribdvNV);
  SET_GetVertexAttribfvARB(disp, glGetVertexAttribfvARB);
//SET_GetVertexAttribfvNV(disp, glGetVertexAttribfvNV);
  SET_GetVertexAttribivARB(disp, glGetVertexAttribivARB);
//SET_GetVertexAttribivNV(disp, glGetVertexAttribivNV);
  SET_Hint(disp, glHint);
  SET_Histogram(disp, glHistogram);
  SET_IndexMask(disp, glIndexMask);
  SET_IndexPointer(disp, glIndexPointer);
//SET_IndexPointerEXT(disp, glIndexPointerEXT);
  SET_Indexd(disp, glIndexd);
  SET_Indexdv(disp, glIndexdv);
  SET_Indexf(disp, glIndexf);
  SET_Indexfv(disp, glIndexfv);
  SET_Indexi(disp, glIndexi);
  SET_Indexiv(disp, glIndexiv);
  SET_Indexs(disp, glIndexs);
  SET_Indexsv(disp, glIndexsv);
  SET_Indexub(disp, glIndexub);
  SET_Indexubv(disp, glIndexubv);
  SET_InitNames(disp, glInitNames);
  SET_InterleavedArrays(disp, glInterleavedArrays);
  SET_IsBufferARB(disp, glIsBufferARB);
  SET_IsEnabled(disp, glIsEnabled);
  SET_IsFenceNV(disp, glIsFenceAPPLE);           // <-- APPLE -> NV
  SET_IsFramebufferEXT(disp, glIsFramebufferEXT);
  SET_IsList(disp, glIsList);
//SET_IsProgramNV(disp, glIsProgramNV);
  SET_IsQueryARB(disp, glIsQueryARB);
  SET_IsRenderbufferEXT(disp, glIsRenderbufferEXT);
  SET_IsTexture(disp, glIsTexture);
  SET_LightModelf(disp, glLightModelf);
  SET_LightModelfv(disp, glLightModelfv);
  SET_LightModeli(disp, glLightModeli);
  SET_LightModeliv(disp, glLightModeliv);
  SET_Lightf(disp, glLightf);
  SET_Lightfv(disp, glLightfv);
  SET_Lighti(disp, glLighti);
  SET_Lightiv(disp, glLightiv);
  SET_LineStipple(disp, glLineStipple);
  SET_LineWidth(disp, glLineWidth);
  SET_LinkProgramARB(disp, glLinkProgramARB);
  SET_ListBase(disp, glListBase);
  SET_LoadIdentity(disp, glLoadIdentity);
  SET_LoadMatrixd(disp, glLoadMatrixd);
  SET_LoadMatrixf(disp, glLoadMatrixf);
  SET_LoadName(disp, glLoadName);
//SET_LoadProgramNV(disp, glLoadProgramNV);
  SET_LoadTransposeMatrixdARB(disp, glLoadTransposeMatrixdARB);
  SET_LoadTransposeMatrixfARB(disp, glLoadTransposeMatrixfARB);
  SET_LockArraysEXT(disp, glLockArraysEXT);
  SET_LogicOp(disp, glLogicOp);
  SET_Map1d(disp, glMap1d);
  SET_Map1f(disp, glMap1f);
  SET_Map2d(disp, glMap2d);
  SET_Map2f(disp, glMap2f);
  SET_MapBufferARB(disp, glMapBufferARB);
  SET_MapGrid1d(disp, glMapGrid1d);
  SET_MapGrid1f(disp, glMapGrid1f);
  SET_MapGrid2d(disp, glMapGrid2d);
  SET_MapGrid2f(disp, glMapGrid2f);
  SET_Materialf(disp, glMaterialf);
  SET_Materialfv(disp, glMaterialfv);
  SET_Materiali(disp, glMateriali);
  SET_Materialiv(disp, glMaterialiv);
  SET_MatrixMode(disp, glMatrixMode);
  SET_Minmax(disp, glMinmax);
  SET_MultMatrixd(disp, glMultMatrixd);
  SET_MultMatrixf(disp, glMultMatrixf);
  SET_MultTransposeMatrixdARB(disp, glMultTransposeMatrixdARB);
  SET_MultTransposeMatrixfARB(disp, glMultTransposeMatrixfARB);
  SET_MultiDrawArraysEXT(disp, glMultiDrawArraysEXT);
  SET_MultiDrawElementsEXT(disp, glMultiDrawElementsEXT);
//SET_MultiModeDrawArraysIBM(disp, glMultiModeDrawArraysIBM);
//SET_MultiModeDrawElementsIBM(disp, glMultiModeDrawElementsIBM);
  SET_MultiTexCoord1dARB(disp, glMultiTexCoord1dARB);
  SET_MultiTexCoord1dvARB(disp, glMultiTexCoord1dvARB);
  SET_MultiTexCoord1fARB(disp, glMultiTexCoord1fARB);
  SET_MultiTexCoord1fvARB(disp, glMultiTexCoord1fvARB);
  SET_MultiTexCoord1iARB(disp, glMultiTexCoord1iARB);
  SET_MultiTexCoord1ivARB(disp, glMultiTexCoord1ivARB);
  SET_MultiTexCoord1sARB(disp, glMultiTexCoord1sARB);
  SET_MultiTexCoord1svARB(disp, glMultiTexCoord1svARB);
  SET_MultiTexCoord2dARB(disp, glMultiTexCoord2dARB);
  SET_MultiTexCoord2dvARB(disp, glMultiTexCoord2dvARB);
  SET_MultiTexCoord2fARB(disp, glMultiTexCoord2fARB);
  SET_MultiTexCoord2fvARB(disp, glMultiTexCoord2fvARB);
  SET_MultiTexCoord2iARB(disp, glMultiTexCoord2iARB);
  SET_MultiTexCoord2ivARB(disp, glMultiTexCoord2ivARB);
  SET_MultiTexCoord2sARB(disp, glMultiTexCoord2sARB);
  SET_MultiTexCoord2svARB(disp, glMultiTexCoord2svARB);
  SET_MultiTexCoord3dARB(disp, glMultiTexCoord3dARB);
  SET_MultiTexCoord3dvARB(disp, glMultiTexCoord3dvARB);
  SET_MultiTexCoord3fARB(disp, glMultiTexCoord3fARB);
  SET_MultiTexCoord3fvARB(disp, glMultiTexCoord3fvARB);
  SET_MultiTexCoord3iARB(disp, glMultiTexCoord3iARB);
  SET_MultiTexCoord3ivARB(disp, glMultiTexCoord3ivARB);
  SET_MultiTexCoord3sARB(disp, glMultiTexCoord3sARB);
  SET_MultiTexCoord3svARB(disp, glMultiTexCoord3svARB);
  SET_MultiTexCoord4dARB(disp, glMultiTexCoord4dARB);
  SET_MultiTexCoord4dvARB(disp, glMultiTexCoord4dvARB);
  SET_MultiTexCoord4fARB(disp, glMultiTexCoord4fARB);
  SET_MultiTexCoord4fvARB(disp, glMultiTexCoord4fvARB);
  SET_MultiTexCoord4iARB(disp, glMultiTexCoord4iARB);
  SET_MultiTexCoord4ivARB(disp, glMultiTexCoord4ivARB);
  SET_MultiTexCoord4sARB(disp, glMultiTexCoord4sARB);
  SET_MultiTexCoord4svARB(disp, glMultiTexCoord4svARB);
  SET_NewList(disp, glNewList);
  SET_Normal3b(disp, glNormal3b);
  SET_Normal3bv(disp, glNormal3bv);
  SET_Normal3d(disp, glNormal3d);
  SET_Normal3dv(disp, glNormal3dv);
  SET_Normal3f(disp, glNormal3f);
  SET_Normal3fv(disp, glNormal3fv);
  SET_Normal3i(disp, glNormal3i);
  SET_Normal3iv(disp, glNormal3iv);
  SET_Normal3s(disp, glNormal3s);
  SET_Normal3sv(disp, glNormal3sv);
  SET_NormalPointer(disp, glNormalPointer);
//SET_NormalPointerEXT(disp, glNormalPointerEXT);
  SET_Ortho(disp, glOrtho);
//SET_PassTexCoordATI(disp, glPassTexCoordEXT);         // <-- EXT -> ATI
  SET_PassThrough(disp, glPassThrough);
  SET_PixelMapfv(disp, glPixelMapfv);
  SET_PixelMapuiv(disp, glPixelMapuiv);
  SET_PixelMapusv(disp, glPixelMapusv);
  SET_PixelStoref(disp, glPixelStoref);
  SET_PixelStorei(disp, glPixelStorei);
//SET_PixelTexGenParameterfSGIS(disp, glPixelTexGenParameterfSGIS);
//SET_PixelTexGenParameterfvSGIS(disp, glPixelTexGenParameterfvSGIS);
//SET_PixelTexGenParameteriSGIS(disp, glPixelTexGenParameteriSGIS);
//SET_PixelTexGenParameterivSGIS(disp, glPixelTexGenParameterivSGIS);
//  SET_PixelTexGenSGIX(disp, glPixelTexGenSGIX);
  SET_PixelTransferf(disp, glPixelTransferf);
  SET_PixelTransferi(disp, glPixelTransferi);
  SET_PixelZoom(disp, glPixelZoom);
  SET_PointParameterfEXT(disp, glPointParameterfARB);      // <-- ARB -> EXT
  SET_PointParameterfvEXT(disp, glPointParameterfvARB);    // <-- ARB -> EXT
  SET_PointParameteriNV(disp, glPointParameteriNV);
  SET_PointParameterivNV(disp, glPointParameterivNV);
  SET_PointSize(disp, glPointSize);
  SET_PolygonMode(disp, glPolygonMode);
  SET_PolygonOffset(disp, glPolygonOffset);
//SET_PolygonOffsetEXT(disp, glPolygonOffsetEXT);
  SET_PolygonStipple(disp, glPolygonStipple);
  SET_PopAttrib(disp, glPopAttrib);
  SET_PopClientAttrib(disp, glPopClientAttrib);
  SET_PopMatrix(disp, glPopMatrix);
  SET_PopName(disp, glPopName);
  SET_PrioritizeTextures(disp, glPrioritizeTextures);
  SET_ProgramEnvParameter4dARB(disp, glProgramEnvParameter4dARB);
  SET_ProgramEnvParameter4dvARB(disp, glProgramEnvParameter4dvARB);
  SET_ProgramEnvParameter4fARB(disp, glProgramEnvParameter4fARB);
  SET_ProgramEnvParameter4fvARB(disp, glProgramEnvParameter4fvARB);
  SET_ProgramLocalParameter4dARB(disp, glProgramLocalParameter4dARB);
  SET_ProgramLocalParameter4dvARB(disp, glProgramLocalParameter4dvARB);
  SET_ProgramLocalParameter4fARB(disp, glProgramLocalParameter4fARB);
  SET_ProgramLocalParameter4fvARB(disp, glProgramLocalParameter4fvARB);
//SET_ProgramNamedParameter4dNV(disp, glProgramNamedParameter4dNV);
//SET_ProgramNamedParameter4dvNV(disp, glProgramNamedParameter4dvNV);
//SET_ProgramNamedParameter4fNV(disp, glProgramNamedParameter4fNV);
//SET_ProgramNamedParameter4fvNV(disp, glProgramNamedParameter4fvNV);
//SET_ProgramParameter4dNV(disp, glProgramParameter4dNV);
//SET_ProgramParameter4dvNV(disp, glProgramParameter4dvNV);
//SET_ProgramParameter4fNV(disp, glProgramParameter4fNV);
//SET_ProgramParameter4fvNV(disp, glProgramParameter4fvNV);
//SET_ProgramParameters4dvNV(disp, glProgramParameters4dvNV);
//SET_ProgramParameters4fvNV(disp, glProgramParameters4fvNV);
  SET_ProgramStringARB(disp, glProgramStringARB);
  SET_PushAttrib(disp, glPushAttrib);
  SET_PushClientAttrib(disp, glPushClientAttrib);
  SET_PushMatrix(disp, glPushMatrix);
  SET_PushName(disp, glPushName);
  SET_RasterPos2d(disp, glRasterPos2d);
  SET_RasterPos2dv(disp, glRasterPos2dv);
  SET_RasterPos2f(disp, glRasterPos2f);
  SET_RasterPos2fv(disp, glRasterPos2fv);
  SET_RasterPos2i(disp, glRasterPos2i);
  SET_RasterPos2iv(disp, glRasterPos2iv);
  SET_RasterPos2s(disp, glRasterPos2s);
  SET_RasterPos2sv(disp, glRasterPos2sv);
  SET_RasterPos3d(disp, glRasterPos3d);
  SET_RasterPos3dv(disp, glRasterPos3dv);
  SET_RasterPos3f(disp, glRasterPos3f);
  SET_RasterPos3fv(disp, glRasterPos3fv);
  SET_RasterPos3i(disp, glRasterPos3i);
  SET_RasterPos3iv(disp, glRasterPos3iv);
  SET_RasterPos3s(disp, glRasterPos3s);
  SET_RasterPos3sv(disp, glRasterPos3sv);
  SET_RasterPos4d(disp, glRasterPos4d);
  SET_RasterPos4dv(disp, glRasterPos4dv);
  SET_RasterPos4f(disp, glRasterPos4f);
  SET_RasterPos4fv(disp, glRasterPos4fv);
  SET_RasterPos4i(disp, glRasterPos4i);
  SET_RasterPos4iv(disp, glRasterPos4iv);
  SET_RasterPos4s(disp, glRasterPos4s);
  SET_RasterPos4sv(disp, glRasterPos4sv);
  SET_ReadBuffer(disp, glReadBuffer);
  SET_ReadPixels(disp, glReadPixels);
  SET_Rectd(disp, glRectd);
  SET_Rectdv(disp, glRectdv);
  SET_Rectf(disp, glRectf);
  SET_Rectfv(disp, glRectfv);
  SET_Recti(disp, glRecti);
  SET_Rectiv(disp, glRectiv);
  SET_Rects(disp, glRects);
  SET_Rectsv(disp, glRectsv);
  SET_RenderMode(disp, glRenderMode);
  SET_RenderbufferStorageEXT(disp, glRenderbufferStorageEXT);
//SET_RequestResidentProgramsNV(disp, glRequestResidentProgramsNV);
  SET_ResetHistogram(disp, glResetHistogram);
  SET_ResetMinmax(disp, glResetMinmax);
//SET_ResizeBuffersMESA(disp, glResizeBuffersMESA);
  SET_Rotated(disp, glRotated);
  SET_Rotatef(disp, glRotatef);
  SET_SampleCoverageARB(disp, glSampleCoverageARB);
//SET_SampleMapATI(disp, glSampleMapEXT);       // <-- EXT -> ATI
//SET_SampleMaskSGIS(disp, glSampleMaskSGIS);
//SET_SamplePatternSGIS(disp, glSamplePatternSGIS);
  SET_Scaled(disp, glScaled);
  SET_Scalef(disp, glScalef);
  SET_Scissor(disp, glScissor);
  SET_SecondaryColor3bEXT(disp, glSecondaryColor3bEXT);
  SET_SecondaryColor3bvEXT(disp, glSecondaryColor3bvEXT);
  SET_SecondaryColor3dEXT(disp, glSecondaryColor3dEXT);
  SET_SecondaryColor3dvEXT(disp, glSecondaryColor3dvEXT);
  SET_SecondaryColor3fEXT(disp, glSecondaryColor3fEXT);
  SET_SecondaryColor3fvEXT(disp, glSecondaryColor3fvEXT);
  SET_SecondaryColor3iEXT(disp, glSecondaryColor3iEXT);
  SET_SecondaryColor3ivEXT(disp, glSecondaryColor3ivEXT);
  SET_SecondaryColor3sEXT(disp, glSecondaryColor3sEXT);
  SET_SecondaryColor3svEXT(disp, glSecondaryColor3svEXT);
  SET_SecondaryColor3ubEXT(disp, glSecondaryColor3ubEXT);
  SET_SecondaryColor3ubvEXT(disp, glSecondaryColor3ubvEXT);
  SET_SecondaryColor3uiEXT(disp, glSecondaryColor3uiEXT);
  SET_SecondaryColor3uivEXT(disp, glSecondaryColor3uivEXT);
  SET_SecondaryColor3usEXT(disp, glSecondaryColor3usEXT);
  SET_SecondaryColor3usvEXT(disp, glSecondaryColor3usvEXT);
  SET_SecondaryColorPointerEXT(disp, glSecondaryColorPointerEXT);
  SET_SelectBuffer(disp, glSelectBuffer);
  SET_SeparableFilter2D(disp, glSeparableFilter2D);
  SET_SetFenceNV(disp, glSetFenceAPPLE);  // <-- APPLE -> NV
//SET_SetFragmentShaderConstantATI(disp, glSetFragmentShaderConstantEXT);   // <-- EXT -> ATI
  SET_ShadeModel(disp, glShadeModel);
  SET_ShaderSourceARB(disp, glShaderSourceARB);
  SET_StencilFunc(disp, glStencilFunc);
  SET_StencilFuncSeparate(disp, glStencilFuncSeparate);
  SET_StencilMask(disp, glStencilMask);
  SET_StencilMaskSeparate(disp, glStencilMaskSeparate);
  SET_StencilOp(disp, glStencilOp);
  SET_StencilOpSeparate(disp, glStencilOpSeparate);
  SET_TestFenceNV(disp, glTestFenceAPPLE); // <-- APPLE -> NV
  SET_TexCoord1d(disp, glTexCoord1d);
  SET_TexCoord1dv(disp, glTexCoord1dv);
  SET_TexCoord1f(disp, glTexCoord1f);
  SET_TexCoord1fv(disp, glTexCoord1fv);
  SET_TexCoord1i(disp, glTexCoord1i);
  SET_TexCoord1iv(disp, glTexCoord1iv);
  SET_TexCoord1s(disp, glTexCoord1s);
  SET_TexCoord1sv(disp, glTexCoord1sv);
  SET_TexCoord2d(disp, glTexCoord2d);
  SET_TexCoord2dv(disp, glTexCoord2dv);
  SET_TexCoord2f(disp, glTexCoord2f);
  SET_TexCoord2fv(disp, glTexCoord2fv);
  SET_TexCoord2i(disp, glTexCoord2i);
  SET_TexCoord2iv(disp, glTexCoord2iv);
  SET_TexCoord2s(disp, glTexCoord2s);
  SET_TexCoord2sv(disp, glTexCoord2sv);
  SET_TexCoord3d(disp, glTexCoord3d);
  SET_TexCoord3dv(disp, glTexCoord3dv);
  SET_TexCoord3f(disp, glTexCoord3f);
  SET_TexCoord3fv(disp, glTexCoord3fv);
  SET_TexCoord3i(disp, glTexCoord3i);
  SET_TexCoord3iv(disp, glTexCoord3iv);
  SET_TexCoord3s(disp, glTexCoord3s);
  SET_TexCoord3sv(disp, glTexCoord3sv);
  SET_TexCoord4d(disp, glTexCoord4d);
  SET_TexCoord4dv(disp, glTexCoord4dv);
  SET_TexCoord4f(disp, glTexCoord4f);
  SET_TexCoord4fv(disp, glTexCoord4fv);
  SET_TexCoord4i(disp, glTexCoord4i);
  SET_TexCoord4iv(disp, glTexCoord4iv);
  SET_TexCoord4s(disp, glTexCoord4s);
  SET_TexCoord4sv(disp, glTexCoord4sv);
  SET_TexCoordPointer(disp, glTexCoordPointer);
//SET_TexCoordPointerEXT(disp, glTexCoordPointerEXT);
  SET_TexEnvf(disp, glTexEnvf);
  SET_TexEnvfv(disp, glTexEnvfv);
  SET_TexEnvi(disp, glTexEnvi);
  SET_TexEnviv(disp, glTexEnviv);
  SET_TexGend(disp, glTexGend);
  SET_TexGendv(disp, glTexGendv);
  SET_TexGenf(disp, glTexGenf);
  SET_TexGenfv(disp, glTexGenfv);
  SET_TexGeni(disp, glTexGeni);
  SET_TexGeniv(disp, glTexGeniv);
  SET_TexImage1D(disp, glTexImage1D);
  SET_TexImage2D(disp, glTexImage2D);
  SET_TexImage3D(disp, glTexImage3D);
  SET_TexParameterf(disp, glTexParameterf);
  SET_TexParameterfv(disp, glTexParameterfv);
  SET_TexParameteri(disp, glTexParameteri);
  SET_TexParameteriv(disp, glTexParameteriv);
  SET_TexSubImage1D(disp, glTexSubImage1D);
  SET_TexSubImage2D(disp, glTexSubImage2D);
  SET_TexSubImage3D(disp, glTexSubImage3D);
//SET_TrackMatrixNV(disp, glTrackMatrixNV);
  SET_Translated(disp, glTranslated);
  SET_Translatef(disp, glTranslatef);
  SET_Uniform1fARB(disp, glUniform1fARB);
  SET_Uniform1fvARB(disp, glUniform1fvARB);
  SET_Uniform1iARB(disp, glUniform1iARB);
  SET_Uniform1ivARB(disp, glUniform1ivARB);
  SET_Uniform2fARB(disp, glUniform2fARB);
  SET_Uniform2fvARB(disp, glUniform2fvARB);
  SET_Uniform2iARB(disp, glUniform2iARB);
  SET_Uniform2ivARB(disp, glUniform2ivARB);
  SET_Uniform3fARB(disp, glUniform3fARB);
  SET_Uniform3fvARB(disp, glUniform3fvARB);
  SET_Uniform3iARB(disp, glUniform3iARB);
  SET_Uniform3ivARB(disp, glUniform3ivARB);
  SET_Uniform4fARB(disp, glUniform4fARB);
  SET_Uniform4fvARB(disp, glUniform4fvARB);
  SET_Uniform4iARB(disp, glUniform4iARB);
  SET_Uniform4ivARB(disp, glUniform4ivARB);
  SET_UniformMatrix2fvARB(disp, glUniformMatrix2fvARB);
  SET_UniformMatrix3fvARB(disp, glUniformMatrix3fvARB);
  SET_UniformMatrix4fvARB(disp, glUniformMatrix4fvARB);
  SET_UnlockArraysEXT(disp, glUnlockArraysEXT);
  SET_UnmapBufferARB(disp, glUnmapBufferARB);
  SET_UseProgramObjectARB(disp, glUseProgramObjectARB);
  SET_ValidateProgramARB(disp, glValidateProgramARB);
  SET_Vertex2d(disp, glVertex2d);
  SET_Vertex2dv(disp, glVertex2dv);
  SET_Vertex2f(disp, glVertex2f);
  SET_Vertex2fv(disp, glVertex2fv);
  SET_Vertex2i(disp, glVertex2i);
  SET_Vertex2iv(disp, glVertex2iv);
  SET_Vertex2s(disp, glVertex2s);
  SET_Vertex2sv(disp, glVertex2sv);
  SET_Vertex3d(disp, glVertex3d);
  SET_Vertex3dv(disp, glVertex3dv);
  SET_Vertex3f(disp, glVertex3f);
  SET_Vertex3fv(disp, glVertex3fv);
  SET_Vertex3i(disp, glVertex3i);
  SET_Vertex3iv(disp, glVertex3iv);
  SET_Vertex3s(disp, glVertex3s);
  SET_Vertex3sv(disp, glVertex3sv);
  SET_Vertex4d(disp, glVertex4d);
  SET_Vertex4dv(disp, glVertex4dv);
  SET_Vertex4f(disp, glVertex4f);
  SET_Vertex4fv(disp, glVertex4fv);
  SET_Vertex4i(disp, glVertex4i);
  SET_Vertex4iv(disp, glVertex4iv);
  SET_Vertex4s(disp, glVertex4s);
  SET_Vertex4sv(disp, glVertex4sv);
//SET_VertexArrayRangeNV(disp, glVertexArrayRangeNV);
  SET_VertexAttrib1dARB(disp, glVertexAttrib1dARB);
  SET_VertexAttrib1dvARB(disp, glVertexAttrib1dvARB);
  SET_VertexAttrib1fARB(disp, glVertexAttrib1fARB);
  SET_VertexAttrib1fvARB(disp, glVertexAttrib1fvARB);
  SET_VertexAttrib1sARB(disp, glVertexAttrib1sARB);
  SET_VertexAttrib1svARB(disp, glVertexAttrib1svARB);
  SET_VertexAttrib2dARB(disp, glVertexAttrib2dARB);
  SET_VertexAttrib2dvARB(disp, glVertexAttrib2dvARB);
  SET_VertexAttrib2fARB(disp, glVertexAttrib2fARB);
  SET_VertexAttrib2fvARB(disp, glVertexAttrib2fvARB);
  SET_VertexAttrib2sARB(disp, glVertexAttrib2sARB);
  SET_VertexAttrib2svARB(disp, glVertexAttrib2svARB);
  SET_VertexAttrib3dARB(disp, glVertexAttrib3dARB);
  SET_VertexAttrib3dvARB(disp, glVertexAttrib3dvARB);
  SET_VertexAttrib3fARB(disp, glVertexAttrib3fARB);
  SET_VertexAttrib3fvARB(disp, glVertexAttrib3fvARB);
  SET_VertexAttrib3sARB(disp, glVertexAttrib3sARB);
  SET_VertexAttrib3svARB(disp, glVertexAttrib3svARB);
  SET_VertexAttrib4NbvARB(disp, glVertexAttrib4NbvARB);
  SET_VertexAttrib4NivARB(disp, glVertexAttrib4NivARB);
  SET_VertexAttrib4NsvARB(disp, glVertexAttrib4NsvARB);
  SET_VertexAttrib4NubARB(disp, glVertexAttrib4NubARB);
  SET_VertexAttrib4NubvARB(disp, glVertexAttrib4NubvARB);
  SET_VertexAttrib4NuivARB(disp, glVertexAttrib4NuivARB);
  SET_VertexAttrib4NusvARB(disp, glVertexAttrib4NusvARB);
  SET_VertexAttrib4bvARB(disp, glVertexAttrib4bvARB);
  SET_VertexAttrib4dARB(disp, glVertexAttrib4dARB);
  SET_VertexAttrib4dvARB(disp, glVertexAttrib4dvARB);
  SET_VertexAttrib4fARB(disp, glVertexAttrib4fARB);
  SET_VertexAttrib4fvARB(disp, glVertexAttrib4fvARB);
  SET_VertexAttrib4ivARB(disp, glVertexAttrib4ivARB);
  SET_VertexAttrib4sARB(disp, glVertexAttrib4sARB);
  SET_VertexAttrib4svARB(disp, glVertexAttrib4svARB);
  SET_VertexAttrib4ubvARB(disp, glVertexAttrib4ubvARB);
  SET_VertexAttrib4uivARB(disp, glVertexAttrib4uivARB);
  SET_VertexAttrib4usvARB(disp, glVertexAttrib4usvARB);
  SET_VertexAttribPointerARB(disp, glVertexAttribPointerARB);
  SET_VertexPointer(disp, glVertexPointer);
//  SET_VertexPointerEXT(disp, glVertexPointerEXT);
  SET_Viewport(disp, glViewport);
  SET_WindowPos2dMESA(disp, glWindowPos2dARB);
  SET_WindowPos2dvMESA(disp, glWindowPos2dvARB);
  SET_WindowPos2fMESA(disp, glWindowPos2fARB);
  SET_WindowPos2fvMESA(disp, glWindowPos2fvARB);
  SET_WindowPos2iMESA(disp, glWindowPos2iARB);
  SET_WindowPos2ivMESA(disp, glWindowPos2ivARB);
  SET_WindowPos2sMESA(disp, glWindowPos2sARB);
  SET_WindowPos2svMESA(disp, glWindowPos2svARB);
  SET_WindowPos3dMESA(disp, glWindowPos3dARB);
  SET_WindowPos3dvMESA(disp, glWindowPos3dvARB);
  SET_WindowPos3fMESA(disp, glWindowPos3fARB);
  SET_WindowPos3fvMESA(disp, glWindowPos3fvARB);
  SET_WindowPos3iMESA(disp, glWindowPos3iARB);
  SET_WindowPos3ivMESA(disp, glWindowPos3ivARB);
  SET_WindowPos3sMESA(disp, glWindowPos3sARB);
  SET_WindowPos3svMESA(disp, glWindowPos3svARB);
//SET_WindowPos4dMESA(disp, glWindowPos4dMESA);
//SET_WindowPos4dvMESA(disp, glWindowPos4dvMESA);
//SET_WindowPos4fMESA(disp, glWindowPos4fMESA);
//SET_WindowPos4fvMESA(disp, glWindowPos4fvMESA);
//SET_WindowPos4iMESA(disp, glWindowPos4iMESA);
//SET_WindowPos4ivMESA(disp, glWindowPos4ivMESA);
//SET_WindowPos4sMESA(disp, glWindowPos4sMESA);
//SET_WindowPos4svMESA(disp, glWindowPos4svMESA);
}
