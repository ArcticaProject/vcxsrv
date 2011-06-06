/*
 * GLX implementation that uses Apple's OpenGL.framework
 * (Indirect rendering path -- it's also used for some direct mode code too)
 *
 * Copyright (c) 2007-2011 Apple Inc.
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

#include <AvailabilityMacros.h>

#include <dlfcn.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>     /* Just to prevent glxserver.h from loading mesa's and colliding with OpenGL.h */

#include <X11/Xproto.h>
#include <GL/glxproto.h>

#include <glxserver.h>
#include <glxutil.h>

typedef unsigned long long GLuint64EXT;
typedef long long GLint64EXT;
#include <dispatch.h>
#include <glapi.h>

#include "x-hash.h"

#include "visualConfigs.h"
#include "dri.h"

#include "darwin.h"
#define GLAQUA_DEBUG_MSG(msg, args...) ASL_LOG(ASL_LEVEL_DEBUG, "GLXAqua", msg, ##args)

__GLXprovider * GlxGetDRISWrastProvider (void);

static void setup_dispatch_table(void);
GLuint __glFloorLog2(GLuint val);
void warn_func(void * p1, char *format, ...);

// some prototypes
static __GLXscreen * __glXAquaScreenProbe(ScreenPtr pScreen);
static __GLXdrawable * __glXAquaScreenCreateDrawable(ClientPtr client, __GLXscreen *screen, DrawablePtr pDraw, XID drawId, int type, XID glxDrawId, __GLXconfig *conf);

static void __glXAquaContextDestroy(__GLXcontext *baseContext);
static int __glXAquaContextMakeCurrent(__GLXcontext *baseContext);
static int __glXAquaContextLoseCurrent(__GLXcontext *baseContext);
static int __glXAquaContextCopy(__GLXcontext *baseDst, __GLXcontext *baseSrc, unsigned long mask);

static CGLPixelFormatObj makeFormat(__GLXconfig *conf);

__GLXprovider __glXDRISWRastProvider = {
    __glXAquaScreenProbe,
    "Core OpenGL",
    NULL
};

typedef struct __GLXAquaScreen   __GLXAquaScreen;
typedef struct __GLXAquaContext  __GLXAquaContext;
typedef struct __GLXAquaDrawable __GLXAquaDrawable;

struct __GLXAquaScreen {
    __GLXscreen base;
    int index;
    int num_vis;
};

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
    __GLXAquaContext *context;
};


static __GLXcontext *
__glXAquaScreenCreateContext(__GLXscreen *screen,
			     __GLXconfig *conf,
			     __GLXcontext *baseShareContext)
{
    __GLXAquaContext *context;
    __GLXAquaContext *shareContext = (__GLXAquaContext *) baseShareContext;
    CGLError gl_err;
  
    GLAQUA_DEBUG_MSG("glXAquaScreenCreateContext\n");
    
    context = calloc(1, sizeof (__GLXAquaContext));
    
    if (context == NULL)
	return NULL;

    memset(context, 0, sizeof *context);
    
    context->base.pGlxScreen = screen;
    
    context->base.destroy        = __glXAquaContextDestroy;
    context->base.makeCurrent    = __glXAquaContextMakeCurrent;
    context->base.loseCurrent    = __glXAquaContextLoseCurrent;
    context->base.copy           = __glXAquaContextCopy;
    /*FIXME verify that the context->base is fully initialized. */
    
    context->pixelFormat = makeFormat(conf);
    
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

/* maps from surface id -> list of __GLcontext */
static x_hash_table *surface_hash;

static void __glXAquaContextDestroy(__GLXcontext *baseContext) {
    x_list *lst;

    __GLXAquaContext *context = (__GLXAquaContext *) baseContext;
    
    GLAQUA_DEBUG_MSG("glAquaContextDestroy (ctx %p)\n", baseContext);
    if (context != NULL) {
      if (context->sid != 0 && surface_hash != NULL) {
		lst = x_hash_table_lookup(surface_hash, x_cvt_uint_to_vptr(context->sid), NULL);
		lst = x_list_remove(lst, context);
		x_hash_table_insert(surface_hash, x_cvt_uint_to_vptr(context->sid), lst);
      }

      if (context->ctx != NULL)
	  CGLDestroyContext(context->ctx);

      if (context->pixelFormat != NULL)
	  CGLDestroyPixelFormat(context->pixelFormat);
      
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
            x_hash_table_remove(surface_hash, x_cvt_uint_to_vptr(arg->id));
	draw->pDraw = NULL;
	draw->sid = 0;
        break;

    case AppleDRISurfaceNotifyChanged:
        if (surface_hash != NULL) {
            lst = x_hash_table_lookup(surface_hash, x_cvt_uint_to_vptr(arg->id), NULL);
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

static BOOL attach(__GLXAquaContext *context, __GLXAquaDrawable *draw) {
    DrawablePtr pDraw;
    
    GLAQUA_DEBUG_MSG("attach(%p, %p)\n", context, draw);
	
    if(NULL == context || NULL == draw)
	return TRUE;

    pDraw = draw->base.pDraw;

    if(NULL == pDraw) {
	ErrorF("%s:%s() pDraw is NULL!\n", __FILE__, __func__);
	return TRUE;
    }

    if (draw->sid == 0) {
	//if (!quartzProcs->CreateSurface(pDraw->pScreen, pDraw->id, pDraw,
        if (!DRICreateSurface(pDraw->pScreen, pDraw->id, pDraw,
			      0, &draw->sid, NULL,
			      surface_notify, draw))
            return TRUE;
        draw->pDraw = pDraw;
    } 
    
    if (!context->isAttached || context->sid != draw->sid) {
        x_list *lst;
	
        if (xp_attach_gl_context(context->ctx, draw->sid) != Success) {
	    //quartzProcs->DestroySurface(pDraw->pScreen, pDraw->id, pDraw,
            DRIDestroySurface(pDraw->pScreen, pDraw->id, pDraw,
			      surface_notify, draw);
            if (surface_hash != NULL)
                x_hash_table_remove(surface_hash, x_cvt_uint_to_vptr(draw->sid));
	    
            draw->sid = 0;
            return TRUE;
        }
	
        context->isAttached = TRUE;
        context->sid = draw->sid;
	
        if (surface_hash == NULL)
            surface_hash = x_hash_table_new(NULL, NULL, NULL, NULL);
	
        lst = x_hash_table_lookup(surface_hash, x_cvt_uint_to_vptr(context->sid), NULL);
        if (x_list_find(lst, context) == NULL) {
            lst = x_list_prepend(lst, context);
            x_hash_table_insert(surface_hash, x_cvt_uint_to_vptr(context->sid), lst);
        }
	
	

        GLAQUA_DEBUG_MSG("attached 0x%x to 0x%x\n", (unsigned int) pDraw->id,
                         (unsigned int) draw->sid);
    } 

    draw->context = context;

    return FALSE;
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
    
    if(attach(context, drawPriv))
	return /*error*/ 0;

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

/* Drawing surface notification callbacks */
static GLboolean __glXAquaDrawableSwapBuffers(ClientPtr client, __GLXdrawable *base) {
    CGLError err;
    __GLXAquaDrawable *drawable;
 
    //    GLAQUA_DEBUG_MSG("glAquaDrawableSwapBuffers(%p)\n",base);
	
    if(!base) {
	ErrorF("%s passed NULL\n", __func__);
	return GL_FALSE;
    }

    drawable = (__GLXAquaDrawable *)base;

    if(NULL == drawable->context) {
	ErrorF("%s called with a NULL->context for drawable %p!\n",
	       __func__, (void *)drawable);
	return GL_FALSE;
    }

    err = CGLFlushDrawable(drawable->context->ctx);

    if(kCGLNoError != err) {
	ErrorF("CGLFlushDrawable error: %s in %s\n", CGLErrorString(err),
	       __func__);
	return GL_FALSE;
    }

    return GL_TRUE;
}


static CGLPixelFormatObj makeFormat(__GLXconfig *conf) {
    CGLPixelFormatAttribute attr[64];
    CGLPixelFormatObj fobj;
    GLint formats;
    CGLError error;
    int i = 0;
    
    if(conf->doubleBufferMode)
	attr[i++] = kCGLPFADoubleBuffer;

    if(conf->stereoMode)
	attr[i++] = kCGLPFAStereo;

    attr[i++] = kCGLPFAColorSize;
    attr[i++] = conf->redBits + conf->greenBits + conf->blueBits;
    attr[i++] = kCGLPFAAlphaSize;
    attr[i++] = conf->alphaBits;

    if((conf->accumRedBits + conf->accumGreenBits + conf->accumBlueBits +
	conf->accumAlphaBits) > 0) {

	attr[i++] = kCGLPFAAccumSize;
        attr[i++] = conf->accumRedBits + conf->accumGreenBits
	    + conf->accumBlueBits + conf->accumAlphaBits;
    }
    
    attr[i++] = kCGLPFADepthSize;
    attr[i++] = conf->depthBits;

    if(conf->stencilBits) {
	attr[i++] = kCGLPFAStencilSize;
        attr[i++] = conf->stencilBits;	
    }
    
    if(conf->numAuxBuffers > 0) {
	attr[i++] = kCGLPFAAuxBuffers;
	attr[i++] = conf->numAuxBuffers;
    }

    if(conf->sampleBuffers > 0) {
       attr[i++] = kCGLPFASampleBuffers;
       attr[i++] = conf->sampleBuffers;
       attr[i++] = kCGLPFASamples;
       attr[i++] = conf->samples;
    }
     
    attr[i] = 0;

    error = CGLChoosePixelFormat(attr, &fobj, &formats);
    if(error) {
	ErrorF("error: creating pixel format %s\n", CGLErrorString(error));
	return NULL;
    }

    return fobj;
}

static void __glXAquaScreenDestroy(__GLXscreen *screen) {

    GLAQUA_DEBUG_MSG("glXAquaScreenDestroy(%p)\n", screen);
    __glXScreenDestroy(screen);

    free(screen);
}

/* This is called by __glXInitScreens(). */
static __GLXscreen * __glXAquaScreenProbe(ScreenPtr pScreen) {
    __GLXAquaScreen *screen;

    GLAQUA_DEBUG_MSG("glXAquaScreenProbe\n");

    if (pScreen == NULL) 
	return NULL;

    screen = calloc(1, sizeof *screen);

    if(NULL == screen)
	return NULL;
    
    screen->base.destroy        = __glXAquaScreenDestroy;
    screen->base.createContext  = __glXAquaScreenCreateContext;
    screen->base.createDrawable = __glXAquaScreenCreateDrawable;
    screen->base.swapInterval = /*FIXME*/ NULL;
    screen->base.pScreen       = pScreen;
    
    screen->base.fbconfigs = __glXAquaCreateVisualConfigs(&screen->base.numFBConfigs, pScreen->myNum);

    __glXScreenInit(&screen->base, pScreen);

    screen->base.GLXversion = strdup("1.4");
    screen->base.GLXextensions = strdup("GLX_SGIX_fbconfig "
                                        "GLX_SGIS_multisample "
                                        "GLX_ARB_multisample "
                                        "GLX_EXT_visual_info "
                                        "GLX_EXT_import_context ");
    
    /*We may be able to add more GLXextensions at a later time. */
    
    return &screen->base;
}

#if 0 // unused
static void __glXAquaDrawableCopySubBuffer (__GLXdrawable *drawable,
					    int x, int y, int w, int h) {
    /*TODO finish me*/
}
#endif

static void __glXAquaDrawableDestroy(__GLXdrawable *base) {
    /* gstaplin: base is the head of the structure, so it's at the same 
     * offset in memory.
     * Is this safe with strict aliasing?   I noticed that the other dri code
     * does this too...
     */
    __GLXAquaDrawable *glxPriv = (__GLXAquaDrawable *)base;

    GLAQUA_DEBUG_MSG(__func__);
    
    /* It doesn't work to call DRIDestroySurface here, the drawable's
       already gone.. But dri.c notices the window destruction and
       frees the surface itself. */

    /*gstaplin: verify the statement above.  The surface destroy
     *messages weren't making it through, and may still not be.
     *We need a good test case for surface creation and destruction.
     *We also need a good way to enable introspection on the server
     *to validate the test, beyond using gdb with print.
     */

    free(glxPriv);
}

static __GLXdrawable *
__glXAquaScreenCreateDrawable(ClientPtr client,
                              __GLXscreen *screen,
			      DrawablePtr pDraw,
			      XID drawId,
			      int type,
			      XID glxDrawId,
			      __GLXconfig *conf) {
  __GLXAquaDrawable *glxPriv;

  glxPriv = malloc(sizeof *glxPriv);

  if(glxPriv == NULL)
      return NULL;

  memset(glxPriv, 0, sizeof *glxPriv);

  if(!__glXDrawableInit(&glxPriv->base, screen, pDraw, type, glxDrawId, conf)) {
    free(glxPriv);
    return NULL;
  }

  glxPriv->base.destroy       = __glXAquaDrawableDestroy;
  glxPriv->base.swapBuffers   = __glXAquaDrawableSwapBuffers;
  glxPriv->base.copySubBuffer = NULL; /* __glXAquaDrawableCopySubBuffer; */

  glxPriv->pDraw = pDraw;
  glxPriv->sid = 0;
  glxPriv->context = NULL;
  
  return &glxPriv->base;
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

static void setup_dispatch_table(void) {
    static struct _glapi_table *disp = NULL;

    if(disp)  {
        _glapi_set_dispatch(disp);
        return;
    }

    disp=calloc(1,sizeof(struct _glapi_table));
    assert(disp);

    /* to update:
     * for f in $(grep 'define SET_' ../../../glx/dispatch.h  | cut -f2 -d' ' | cut -f1 -d\( | sort -u); do grep -q $f indirect.c && echo $f ; done | grep -v by_offset | sed 's:SET_\(.*\)$:SET_\1(disp, dlsym(RTLD_DEFAULT, "gl\1"))\;:'
     */

    SET_Accum(disp, dlsym(RTLD_DEFAULT, "glAccum"));
    SET_AlphaFunc(disp, dlsym(RTLD_DEFAULT, "glAlphaFunc"));
    SET_AreTexturesResident(disp, dlsym(RTLD_DEFAULT, "glAreTexturesResident"));
    SET_ArrayElement(disp, dlsym(RTLD_DEFAULT, "glArrayElement"));
    SET_Begin(disp, dlsym(RTLD_DEFAULT, "glBegin"));
    SET_BindTexture(disp, dlsym(RTLD_DEFAULT, "glBindTexture"));
    SET_Bitmap(disp, dlsym(RTLD_DEFAULT, "glBitmap"));
    SET_BlendColor(disp, dlsym(RTLD_DEFAULT, "glBlendColor"));
    SET_BlendEquation(disp, dlsym(RTLD_DEFAULT, "glBlendEquation"));
    SET_BlendFunc(disp, dlsym(RTLD_DEFAULT, "glBlendFunc"));
    SET_CallList(disp, dlsym(RTLD_DEFAULT, "glCallList"));
    SET_CallLists(disp, dlsym(RTLD_DEFAULT, "glCallLists"));
    SET_Clear(disp, dlsym(RTLD_DEFAULT, "glClear"));
    SET_ClearAccum(disp, dlsym(RTLD_DEFAULT, "glClearAccum"));
    SET_ClearColor(disp, dlsym(RTLD_DEFAULT, "glClearColor"));
    SET_ClearDepth(disp, dlsym(RTLD_DEFAULT, "glClearDepth"));
    SET_ClearIndex(disp, dlsym(RTLD_DEFAULT, "glClearIndex"));
    SET_ClearStencil(disp, dlsym(RTLD_DEFAULT, "glClearStencil"));
    SET_ClipPlane(disp, dlsym(RTLD_DEFAULT, "glClipPlane"));
    SET_Color3b(disp, dlsym(RTLD_DEFAULT, "glColor3b"));
    SET_Color3bv(disp, dlsym(RTLD_DEFAULT, "glColor3bv"));
    SET_Color3d(disp, dlsym(RTLD_DEFAULT, "glColor3d"));
    SET_Color3dv(disp, dlsym(RTLD_DEFAULT, "glColor3dv"));
    SET_Color3f(disp, dlsym(RTLD_DEFAULT, "glColor3f"));
    SET_Color3fv(disp, dlsym(RTLD_DEFAULT, "glColor3fv"));
    SET_Color3i(disp, dlsym(RTLD_DEFAULT, "glColor3i"));
    SET_Color3iv(disp, dlsym(RTLD_DEFAULT, "glColor3iv"));
    SET_Color3s(disp, dlsym(RTLD_DEFAULT, "glColor3s"));
    SET_Color3sv(disp, dlsym(RTLD_DEFAULT, "glColor3sv"));
    SET_Color3ub(disp, dlsym(RTLD_DEFAULT, "glColor3ub"));
    SET_Color3ubv(disp, dlsym(RTLD_DEFAULT, "glColor3ubv"));
    SET_Color3ui(disp, dlsym(RTLD_DEFAULT, "glColor3ui"));
    SET_Color3uiv(disp, dlsym(RTLD_DEFAULT, "glColor3uiv"));
    SET_Color3us(disp, dlsym(RTLD_DEFAULT, "glColor3us"));
    SET_Color3usv(disp, dlsym(RTLD_DEFAULT, "glColor3usv"));
    SET_Color4b(disp, dlsym(RTLD_DEFAULT, "glColor4b"));
    SET_Color4bv(disp, dlsym(RTLD_DEFAULT, "glColor4bv"));
    SET_Color4d(disp, dlsym(RTLD_DEFAULT, "glColor4d"));
    SET_Color4dv(disp, dlsym(RTLD_DEFAULT, "glColor4dv"));
    SET_Color4f(disp, dlsym(RTLD_DEFAULT, "glColor4f"));
    SET_Color4fv(disp, dlsym(RTLD_DEFAULT, "glColor4fv"));
    SET_Color4i(disp, dlsym(RTLD_DEFAULT, "glColor4i"));
    SET_Color4iv(disp, dlsym(RTLD_DEFAULT, "glColor4iv"));
    SET_Color4s(disp, dlsym(RTLD_DEFAULT, "glColor4s"));
    SET_Color4sv(disp, dlsym(RTLD_DEFAULT, "glColor4sv"));
    SET_Color4ub(disp, dlsym(RTLD_DEFAULT, "glColor4ub"));
    SET_Color4ubv(disp, dlsym(RTLD_DEFAULT, "glColor4ubv"));
    SET_Color4ui(disp, dlsym(RTLD_DEFAULT, "glColor4ui"));
    SET_Color4uiv(disp, dlsym(RTLD_DEFAULT, "glColor4uiv"));
    SET_Color4us(disp, dlsym(RTLD_DEFAULT, "glColor4us"));
    SET_Color4usv(disp, dlsym(RTLD_DEFAULT, "glColor4usv"));
    SET_ColorMask(disp, dlsym(RTLD_DEFAULT, "glColorMask"));
    SET_ColorMaterial(disp, dlsym(RTLD_DEFAULT, "glColorMaterial"));
    SET_ColorPointer(disp, dlsym(RTLD_DEFAULT, "glColorPointer"));
    SET_ColorSubTable(disp, dlsym(RTLD_DEFAULT, "glColorSubTable"));
    SET_ColorTable(disp, dlsym(RTLD_DEFAULT, "glColorTable"));
    SET_ColorTableParameterfv(disp, dlsym(RTLD_DEFAULT, "glColorTableParameterfv"));
    SET_ColorTableParameteriv(disp, dlsym(RTLD_DEFAULT, "glColorTableParameteriv"));
    SET_ConvolutionFilter1D(disp, dlsym(RTLD_DEFAULT, "glConvolutionFilter1D"));
    SET_ConvolutionFilter2D(disp, dlsym(RTLD_DEFAULT, "glConvolutionFilter2D"));
    SET_ConvolutionParameterf(disp, dlsym(RTLD_DEFAULT, "glConvolutionParameterf"));
    SET_ConvolutionParameterfv(disp, dlsym(RTLD_DEFAULT, "glConvolutionParameterfv"));
    SET_ConvolutionParameteri(disp, dlsym(RTLD_DEFAULT, "glConvolutionParameteri"));
    SET_ConvolutionParameteriv(disp, dlsym(RTLD_DEFAULT, "glConvolutionParameteriv"));
    SET_CopyColorSubTable(disp, dlsym(RTLD_DEFAULT, "glCopyColorSubTable"));
    SET_CopyColorTable(disp, dlsym(RTLD_DEFAULT, "glCopyColorTable"));
    SET_CopyConvolutionFilter1D(disp, dlsym(RTLD_DEFAULT, "glCopyConvolutionFilter1D"));
    SET_CopyConvolutionFilter2D(disp, dlsym(RTLD_DEFAULT, "glCopyConvolutionFilter2D"));
    SET_CopyPixels(disp, dlsym(RTLD_DEFAULT, "glCopyPixels"));
    SET_CopyTexImage1D(disp, dlsym(RTLD_DEFAULT, "glCopyTexImage1D"));
    SET_CopyTexImage2D(disp, dlsym(RTLD_DEFAULT, "glCopyTexImage2D"));
    SET_CopyTexSubImage1D(disp, dlsym(RTLD_DEFAULT, "glCopyTexSubImage1D"));
    SET_CopyTexSubImage2D(disp, dlsym(RTLD_DEFAULT, "glCopyTexSubImage2D"));
    SET_CopyTexSubImage3D(disp, dlsym(RTLD_DEFAULT, "glCopyTexSubImage3D"));
    SET_CullFace(disp, dlsym(RTLD_DEFAULT, "glCullFace"));
    SET_DeleteLists(disp, dlsym(RTLD_DEFAULT, "glDeleteLists"));
    SET_DeleteTextures(disp, dlsym(RTLD_DEFAULT, "glDeleteTextures"));
    SET_DepthFunc(disp, dlsym(RTLD_DEFAULT, "glDepthFunc"));
    SET_DepthMask(disp, dlsym(RTLD_DEFAULT, "glDepthMask"));
    SET_DepthRange(disp, dlsym(RTLD_DEFAULT, "glDepthRange"));
    SET_Disable(disp, dlsym(RTLD_DEFAULT, "glDisable"));
    SET_DisableClientState(disp, dlsym(RTLD_DEFAULT, "glDisableClientState"));
    SET_DrawArrays(disp, dlsym(RTLD_DEFAULT, "glDrawArrays"));
    SET_DrawBuffer(disp, dlsym(RTLD_DEFAULT, "glDrawBuffer"));
    SET_DrawElements(disp, dlsym(RTLD_DEFAULT, "glDrawElements"));
    SET_DrawPixels(disp, dlsym(RTLD_DEFAULT, "glDrawPixels"));
    SET_DrawRangeElements(disp, dlsym(RTLD_DEFAULT, "glDrawRangeElements"));
    SET_EdgeFlag(disp, dlsym(RTLD_DEFAULT, "glEdgeFlag"));
    SET_EdgeFlagPointer(disp, dlsym(RTLD_DEFAULT, "glEdgeFlagPointer"));
    SET_EdgeFlagv(disp, dlsym(RTLD_DEFAULT, "glEdgeFlagv"));
    SET_Enable(disp, dlsym(RTLD_DEFAULT, "glEnable"));
    SET_EnableClientState(disp, dlsym(RTLD_DEFAULT, "glEnableClientState"));
    SET_End(disp, dlsym(RTLD_DEFAULT, "glEnd"));
    SET_EndList(disp, dlsym(RTLD_DEFAULT, "glEndList"));
    SET_EvalCoord1d(disp, dlsym(RTLD_DEFAULT, "glEvalCoord1d"));
    SET_EvalCoord1dv(disp, dlsym(RTLD_DEFAULT, "glEvalCoord1dv"));
    SET_EvalCoord1f(disp, dlsym(RTLD_DEFAULT, "glEvalCoord1f"));
    SET_EvalCoord1fv(disp, dlsym(RTLD_DEFAULT, "glEvalCoord1fv"));
    SET_EvalCoord2d(disp, dlsym(RTLD_DEFAULT, "glEvalCoord2d"));
    SET_EvalCoord2dv(disp, dlsym(RTLD_DEFAULT, "glEvalCoord2dv"));
    SET_EvalCoord2f(disp, dlsym(RTLD_DEFAULT, "glEvalCoord2f"));
    SET_EvalCoord2fv(disp, dlsym(RTLD_DEFAULT, "glEvalCoord2fv"));
    SET_EvalMesh1(disp, dlsym(RTLD_DEFAULT, "glEvalMesh1"));
    SET_EvalMesh2(disp, dlsym(RTLD_DEFAULT, "glEvalMesh2"));
    SET_EvalPoint1(disp, dlsym(RTLD_DEFAULT, "glEvalPoint1"));
    SET_EvalPoint2(disp, dlsym(RTLD_DEFAULT, "glEvalPoint2"));
    SET_FeedbackBuffer(disp, dlsym(RTLD_DEFAULT, "glFeedbackBuffer"));
    SET_Finish(disp, dlsym(RTLD_DEFAULT, "glFinish"));
    SET_Flush(disp, dlsym(RTLD_DEFAULT, "glFlush"));
    SET_Fogf(disp, dlsym(RTLD_DEFAULT, "glFogf"));
    SET_Fogfv(disp, dlsym(RTLD_DEFAULT, "glFogfv"));
    SET_Fogi(disp, dlsym(RTLD_DEFAULT, "glFogi"));
    SET_Fogiv(disp, dlsym(RTLD_DEFAULT, "glFogiv"));
    SET_FrontFace(disp, dlsym(RTLD_DEFAULT, "glFrontFace"));
    SET_Frustum(disp, dlsym(RTLD_DEFAULT, "glFrustum"));
    SET_GenLists(disp, dlsym(RTLD_DEFAULT, "glGenLists"));
    SET_GenTextures(disp, dlsym(RTLD_DEFAULT, "glGenTextures"));
    SET_GetBooleanv(disp, dlsym(RTLD_DEFAULT, "glGetBooleanv"));
    SET_GetClipPlane(disp, dlsym(RTLD_DEFAULT, "glGetClipPlane"));
    SET_GetColorTable(disp, dlsym(RTLD_DEFAULT, "glGetColorTable"));
    SET_GetColorTableParameterfv(disp, dlsym(RTLD_DEFAULT, "glGetColorTableParameterfv"));
    SET_GetColorTableParameteriv(disp, dlsym(RTLD_DEFAULT, "glGetColorTableParameteriv"));
    SET_GetConvolutionFilter(disp, dlsym(RTLD_DEFAULT, "glGetConvolutionFilter"));
    SET_GetConvolutionParameterfv(disp, dlsym(RTLD_DEFAULT, "glGetConvolutionParameterfv"));
    SET_GetConvolutionParameteriv(disp, dlsym(RTLD_DEFAULT, "glGetConvolutionParameteriv"));
    SET_GetDoublev(disp, dlsym(RTLD_DEFAULT, "glGetDoublev"));
    SET_GetError(disp, dlsym(RTLD_DEFAULT, "glGetError"));
    SET_GetFloatv(disp, dlsym(RTLD_DEFAULT, "glGetFloatv"));
    SET_GetHistogram(disp, dlsym(RTLD_DEFAULT, "glGetHistogram"));
    SET_GetHistogramParameterfv(disp, dlsym(RTLD_DEFAULT, "glGetHistogramParameterfv"));
    SET_GetHistogramParameteriv(disp, dlsym(RTLD_DEFAULT, "glGetHistogramParameteriv"));
    SET_GetIntegerv(disp, dlsym(RTLD_DEFAULT, "glGetIntegerv"));
    SET_GetLightfv(disp, dlsym(RTLD_DEFAULT, "glGetLightfv"));
    SET_GetLightiv(disp, dlsym(RTLD_DEFAULT, "glGetLightiv"));
    SET_GetMapdv(disp, dlsym(RTLD_DEFAULT, "glGetMapdv"));
    SET_GetMapfv(disp, dlsym(RTLD_DEFAULT, "glGetMapfv"));
    SET_GetMapiv(disp, dlsym(RTLD_DEFAULT, "glGetMapiv"));
    SET_GetMaterialfv(disp, dlsym(RTLD_DEFAULT, "glGetMaterialfv"));
    SET_GetMaterialiv(disp, dlsym(RTLD_DEFAULT, "glGetMaterialiv"));
    SET_GetMinmax(disp, dlsym(RTLD_DEFAULT, "glGetMinmax"));
    SET_GetMinmaxParameterfv(disp, dlsym(RTLD_DEFAULT, "glGetMinmaxParameterfv"));
    SET_GetMinmaxParameteriv(disp, dlsym(RTLD_DEFAULT, "glGetMinmaxParameteriv"));
    SET_GetPixelMapfv(disp, dlsym(RTLD_DEFAULT, "glGetPixelMapfv"));
    SET_GetPixelMapuiv(disp, dlsym(RTLD_DEFAULT, "glGetPixelMapuiv"));
    SET_GetPixelMapusv(disp, dlsym(RTLD_DEFAULT, "glGetPixelMapusv"));
    SET_GetPointerv(disp, dlsym(RTLD_DEFAULT, "glGetPointerv"));
    SET_GetPolygonStipple(disp, dlsym(RTLD_DEFAULT, "glGetPolygonStipple"));
    SET_GetSeparableFilter(disp, dlsym(RTLD_DEFAULT, "glGetSeparableFilter"));
    SET_GetString(disp, dlsym(RTLD_DEFAULT, "glGetString"));
    SET_GetTexEnvfv(disp, dlsym(RTLD_DEFAULT, "glGetTexEnvfv"));
    SET_GetTexEnviv(disp, dlsym(RTLD_DEFAULT, "glGetTexEnviv"));
    SET_GetTexGendv(disp, dlsym(RTLD_DEFAULT, "glGetTexGendv"));
    SET_GetTexGenfv(disp, dlsym(RTLD_DEFAULT, "glGetTexGenfv"));
    SET_GetTexGeniv(disp, dlsym(RTLD_DEFAULT, "glGetTexGeniv"));
    SET_GetTexImage(disp, dlsym(RTLD_DEFAULT, "glGetTexImage"));
    SET_GetTexLevelParameterfv(disp, dlsym(RTLD_DEFAULT, "glGetTexLevelParameterfv"));
    SET_GetTexLevelParameteriv(disp, dlsym(RTLD_DEFAULT, "glGetTexLevelParameteriv"));
    SET_GetTexParameterfv(disp, dlsym(RTLD_DEFAULT, "glGetTexParameterfv"));
    SET_GetTexParameteriv(disp, dlsym(RTLD_DEFAULT, "glGetTexParameteriv"));
    SET_Hint(disp, dlsym(RTLD_DEFAULT, "glHint"));
    SET_Histogram(disp, dlsym(RTLD_DEFAULT, "glHistogram"));
    SET_IndexMask(disp, dlsym(RTLD_DEFAULT, "glIndexMask"));
    SET_IndexPointer(disp, dlsym(RTLD_DEFAULT, "glIndexPointer"));
    SET_Indexd(disp, dlsym(RTLD_DEFAULT, "glIndexd"));
    SET_Indexdv(disp, dlsym(RTLD_DEFAULT, "glIndexdv"));
    SET_Indexf(disp, dlsym(RTLD_DEFAULT, "glIndexf"));
    SET_Indexfv(disp, dlsym(RTLD_DEFAULT, "glIndexfv"));
    SET_Indexi(disp, dlsym(RTLD_DEFAULT, "glIndexi"));
    SET_Indexiv(disp, dlsym(RTLD_DEFAULT, "glIndexiv"));
    SET_Indexs(disp, dlsym(RTLD_DEFAULT, "glIndexs"));
    SET_Indexsv(disp, dlsym(RTLD_DEFAULT, "glIndexsv"));
    SET_Indexub(disp, dlsym(RTLD_DEFAULT, "glIndexub"));
    SET_Indexubv(disp, dlsym(RTLD_DEFAULT, "glIndexubv"));
    SET_InitNames(disp, dlsym(RTLD_DEFAULT, "glInitNames"));
    SET_InterleavedArrays(disp, dlsym(RTLD_DEFAULT, "glInterleavedArrays"));
    SET_IsEnabled(disp, dlsym(RTLD_DEFAULT, "glIsEnabled"));
    SET_IsList(disp, dlsym(RTLD_DEFAULT, "glIsList"));
    SET_IsTexture(disp, dlsym(RTLD_DEFAULT, "glIsTexture"));
    SET_LightModelf(disp, dlsym(RTLD_DEFAULT, "glLightModelf"));
    SET_LightModelfv(disp, dlsym(RTLD_DEFAULT, "glLightModelfv"));
    SET_LightModeli(disp, dlsym(RTLD_DEFAULT, "glLightModeli"));
    SET_LightModeliv(disp, dlsym(RTLD_DEFAULT, "glLightModeliv"));
    SET_Lightf(disp, dlsym(RTLD_DEFAULT, "glLightf"));
    SET_Lightfv(disp, dlsym(RTLD_DEFAULT, "glLightfv"));
    SET_Lighti(disp, dlsym(RTLD_DEFAULT, "glLighti"));
    SET_Lightiv(disp, dlsym(RTLD_DEFAULT, "glLightiv"));
    SET_LineStipple(disp, dlsym(RTLD_DEFAULT, "glLineStipple"));
    SET_LineWidth(disp, dlsym(RTLD_DEFAULT, "glLineWidth"));
    SET_ListBase(disp, dlsym(RTLD_DEFAULT, "glListBase"));
    SET_LoadIdentity(disp, dlsym(RTLD_DEFAULT, "glLoadIdentity"));
    SET_LoadMatrixd(disp, dlsym(RTLD_DEFAULT, "glLoadMatrixd"));
    SET_LoadMatrixf(disp, dlsym(RTLD_DEFAULT, "glLoadMatrixf"));
    SET_LoadName(disp, dlsym(RTLD_DEFAULT, "glLoadName"));
    SET_LogicOp(disp, dlsym(RTLD_DEFAULT, "glLogicOp"));
    SET_Map1d(disp, dlsym(RTLD_DEFAULT, "glMap1d"));
    SET_Map1f(disp, dlsym(RTLD_DEFAULT, "glMap1f"));
    SET_Map2d(disp, dlsym(RTLD_DEFAULT, "glMap2d"));
    SET_Map2f(disp, dlsym(RTLD_DEFAULT, "glMap2f"));
    SET_MapGrid1d(disp, dlsym(RTLD_DEFAULT, "glMapGrid1d"));
    SET_MapGrid1f(disp, dlsym(RTLD_DEFAULT, "glMapGrid1f"));
    SET_MapGrid2d(disp, dlsym(RTLD_DEFAULT, "glMapGrid2d"));
    SET_MapGrid2f(disp, dlsym(RTLD_DEFAULT, "glMapGrid2f"));
    SET_Materialf(disp, dlsym(RTLD_DEFAULT, "glMaterialf"));
    SET_Materialfv(disp, dlsym(RTLD_DEFAULT, "glMaterialfv"));
    SET_Materiali(disp, dlsym(RTLD_DEFAULT, "glMateriali"));
    SET_Materialiv(disp, dlsym(RTLD_DEFAULT, "glMaterialiv"));
    SET_MatrixMode(disp, dlsym(RTLD_DEFAULT, "glMatrixMode"));
    SET_Minmax(disp, dlsym(RTLD_DEFAULT, "glMinmax"));
    SET_MultMatrixd(disp, dlsym(RTLD_DEFAULT, "glMultMatrixd"));
    SET_MultMatrixf(disp, dlsym(RTLD_DEFAULT, "glMultMatrixf"));
    SET_NewList(disp, dlsym(RTLD_DEFAULT, "glNewList"));
    SET_Normal3b(disp, dlsym(RTLD_DEFAULT, "glNormal3b"));
    SET_Normal3bv(disp, dlsym(RTLD_DEFAULT, "glNormal3bv"));
    SET_Normal3d(disp, dlsym(RTLD_DEFAULT, "glNormal3d"));
    SET_Normal3dv(disp, dlsym(RTLD_DEFAULT, "glNormal3dv"));
    SET_Normal3f(disp, dlsym(RTLD_DEFAULT, "glNormal3f"));
    SET_Normal3fv(disp, dlsym(RTLD_DEFAULT, "glNormal3fv"));
    SET_Normal3i(disp, dlsym(RTLD_DEFAULT, "glNormal3i"));
    SET_Normal3iv(disp, dlsym(RTLD_DEFAULT, "glNormal3iv"));
    SET_Normal3s(disp, dlsym(RTLD_DEFAULT, "glNormal3s"));
    SET_Normal3sv(disp, dlsym(RTLD_DEFAULT, "glNormal3sv"));
    SET_NormalPointer(disp, dlsym(RTLD_DEFAULT, "glNormalPointer"));
    SET_Ortho(disp, dlsym(RTLD_DEFAULT, "glOrtho"));
    SET_PassThrough(disp, dlsym(RTLD_DEFAULT, "glPassThrough"));
    SET_PixelMapfv(disp, dlsym(RTLD_DEFAULT, "glPixelMapfv"));
    SET_PixelMapuiv(disp, dlsym(RTLD_DEFAULT, "glPixelMapuiv"));
    SET_PixelMapusv(disp, dlsym(RTLD_DEFAULT, "glPixelMapusv"));
    SET_PixelStoref(disp, dlsym(RTLD_DEFAULT, "glPixelStoref"));
    SET_PixelStorei(disp, dlsym(RTLD_DEFAULT, "glPixelStorei"));
    SET_PixelTransferf(disp, dlsym(RTLD_DEFAULT, "glPixelTransferf"));
    SET_PixelTransferi(disp, dlsym(RTLD_DEFAULT, "glPixelTransferi"));
    SET_PixelZoom(disp, dlsym(RTLD_DEFAULT, "glPixelZoom"));
    SET_PointSize(disp, dlsym(RTLD_DEFAULT, "glPointSize"));
    SET_PolygonMode(disp, dlsym(RTLD_DEFAULT, "glPolygonMode"));
    SET_PolygonOffset(disp, dlsym(RTLD_DEFAULT, "glPolygonOffset"));
    SET_PolygonStipple(disp, dlsym(RTLD_DEFAULT, "glPolygonStipple"));
    SET_PopAttrib(disp, dlsym(RTLD_DEFAULT, "glPopAttrib"));
    SET_PopClientAttrib(disp, dlsym(RTLD_DEFAULT, "glPopClientAttrib"));
    SET_PopMatrix(disp, dlsym(RTLD_DEFAULT, "glPopMatrix"));
    SET_PopName(disp, dlsym(RTLD_DEFAULT, "glPopName"));
    SET_PrioritizeTextures(disp, dlsym(RTLD_DEFAULT, "glPrioritizeTextures"));
    SET_PushAttrib(disp, dlsym(RTLD_DEFAULT, "glPushAttrib"));
    SET_PushClientAttrib(disp, dlsym(RTLD_DEFAULT, "glPushClientAttrib"));
    SET_PushMatrix(disp, dlsym(RTLD_DEFAULT, "glPushMatrix"));
    SET_PushName(disp, dlsym(RTLD_DEFAULT, "glPushName"));
    SET_RasterPos2d(disp, dlsym(RTLD_DEFAULT, "glRasterPos2d"));
    SET_RasterPos2dv(disp, dlsym(RTLD_DEFAULT, "glRasterPos2dv"));
    SET_RasterPos2f(disp, dlsym(RTLD_DEFAULT, "glRasterPos2f"));
    SET_RasterPos2fv(disp, dlsym(RTLD_DEFAULT, "glRasterPos2fv"));
    SET_RasterPos2i(disp, dlsym(RTLD_DEFAULT, "glRasterPos2i"));
    SET_RasterPos2iv(disp, dlsym(RTLD_DEFAULT, "glRasterPos2iv"));
    SET_RasterPos2s(disp, dlsym(RTLD_DEFAULT, "glRasterPos2s"));
    SET_RasterPos2sv(disp, dlsym(RTLD_DEFAULT, "glRasterPos2sv"));
    SET_RasterPos3d(disp, dlsym(RTLD_DEFAULT, "glRasterPos3d"));
    SET_RasterPos3dv(disp, dlsym(RTLD_DEFAULT, "glRasterPos3dv"));
    SET_RasterPos3f(disp, dlsym(RTLD_DEFAULT, "glRasterPos3f"));
    SET_RasterPos3fv(disp, dlsym(RTLD_DEFAULT, "glRasterPos3fv"));
    SET_RasterPos3i(disp, dlsym(RTLD_DEFAULT, "glRasterPos3i"));
    SET_RasterPos3iv(disp, dlsym(RTLD_DEFAULT, "glRasterPos3iv"));
    SET_RasterPos3s(disp, dlsym(RTLD_DEFAULT, "glRasterPos3s"));
    SET_RasterPos3sv(disp, dlsym(RTLD_DEFAULT, "glRasterPos3sv"));
    SET_RasterPos4d(disp, dlsym(RTLD_DEFAULT, "glRasterPos4d"));
    SET_RasterPos4dv(disp, dlsym(RTLD_DEFAULT, "glRasterPos4dv"));
    SET_RasterPos4f(disp, dlsym(RTLD_DEFAULT, "glRasterPos4f"));
    SET_RasterPos4fv(disp, dlsym(RTLD_DEFAULT, "glRasterPos4fv"));
    SET_RasterPos4i(disp, dlsym(RTLD_DEFAULT, "glRasterPos4i"));
    SET_RasterPos4iv(disp, dlsym(RTLD_DEFAULT, "glRasterPos4iv"));
    SET_RasterPos4s(disp, dlsym(RTLD_DEFAULT, "glRasterPos4s"));
    SET_RasterPos4sv(disp, dlsym(RTLD_DEFAULT, "glRasterPos4sv"));
    SET_ReadBuffer(disp, dlsym(RTLD_DEFAULT, "glReadBuffer"));
    SET_ReadPixels(disp, dlsym(RTLD_DEFAULT, "glReadPixels"));
    SET_Rectd(disp, dlsym(RTLD_DEFAULT, "glRectd"));
    SET_Rectdv(disp, dlsym(RTLD_DEFAULT, "glRectdv"));
    SET_Rectf(disp, dlsym(RTLD_DEFAULT, "glRectf"));
    SET_Rectfv(disp, dlsym(RTLD_DEFAULT, "glRectfv"));
    SET_Recti(disp, dlsym(RTLD_DEFAULT, "glRecti"));
    SET_Rectiv(disp, dlsym(RTLD_DEFAULT, "glRectiv"));
    SET_Rects(disp, dlsym(RTLD_DEFAULT, "glRects"));
    SET_Rectsv(disp, dlsym(RTLD_DEFAULT, "glRectsv"));
    SET_RenderMode(disp, dlsym(RTLD_DEFAULT, "glRenderMode"));
    SET_ResetHistogram(disp, dlsym(RTLD_DEFAULT, "glResetHistogram"));
    SET_ResetMinmax(disp, dlsym(RTLD_DEFAULT, "glResetMinmax"));
    SET_Rotated(disp, dlsym(RTLD_DEFAULT, "glRotated"));
    SET_Rotatef(disp, dlsym(RTLD_DEFAULT, "glRotatef"));
    SET_Scaled(disp, dlsym(RTLD_DEFAULT, "glScaled"));
    SET_Scalef(disp, dlsym(RTLD_DEFAULT, "glScalef"));
    SET_Scissor(disp, dlsym(RTLD_DEFAULT, "glScissor"));
    SET_SelectBuffer(disp, dlsym(RTLD_DEFAULT, "glSelectBuffer"));
    SET_SeparableFilter2D(disp, dlsym(RTLD_DEFAULT, "glSeparableFilter2D"));
    SET_ShadeModel(disp, dlsym(RTLD_DEFAULT, "glShadeModel"));
    SET_StencilFunc(disp, dlsym(RTLD_DEFAULT, "glStencilFunc"));
    SET_StencilMask(disp, dlsym(RTLD_DEFAULT, "glStencilMask"));
    SET_StencilOp(disp, dlsym(RTLD_DEFAULT, "glStencilOp"));
    SET_TexCoord1d(disp, dlsym(RTLD_DEFAULT, "glTexCoord1d"));
    SET_TexCoord1dv(disp, dlsym(RTLD_DEFAULT, "glTexCoord1dv"));
    SET_TexCoord1f(disp, dlsym(RTLD_DEFAULT, "glTexCoord1f"));
    SET_TexCoord1fv(disp, dlsym(RTLD_DEFAULT, "glTexCoord1fv"));
    SET_TexCoord1i(disp, dlsym(RTLD_DEFAULT, "glTexCoord1i"));
    SET_TexCoord1iv(disp, dlsym(RTLD_DEFAULT, "glTexCoord1iv"));
    SET_TexCoord1s(disp, dlsym(RTLD_DEFAULT, "glTexCoord1s"));
    SET_TexCoord1sv(disp, dlsym(RTLD_DEFAULT, "glTexCoord1sv"));
    SET_TexCoord2d(disp, dlsym(RTLD_DEFAULT, "glTexCoord2d"));
    SET_TexCoord2dv(disp, dlsym(RTLD_DEFAULT, "glTexCoord2dv"));
    SET_TexCoord2f(disp, dlsym(RTLD_DEFAULT, "glTexCoord2f"));
    SET_TexCoord2fv(disp, dlsym(RTLD_DEFAULT, "glTexCoord2fv"));
    SET_TexCoord2i(disp, dlsym(RTLD_DEFAULT, "glTexCoord2i"));
    SET_TexCoord2iv(disp, dlsym(RTLD_DEFAULT, "glTexCoord2iv"));
    SET_TexCoord2s(disp, dlsym(RTLD_DEFAULT, "glTexCoord2s"));
    SET_TexCoord2sv(disp, dlsym(RTLD_DEFAULT, "glTexCoord2sv"));
    SET_TexCoord3d(disp, dlsym(RTLD_DEFAULT, "glTexCoord3d"));
    SET_TexCoord3dv(disp, dlsym(RTLD_DEFAULT, "glTexCoord3dv"));
    SET_TexCoord3f(disp, dlsym(RTLD_DEFAULT, "glTexCoord3f"));
    SET_TexCoord3fv(disp, dlsym(RTLD_DEFAULT, "glTexCoord3fv"));
    SET_TexCoord3i(disp, dlsym(RTLD_DEFAULT, "glTexCoord3i"));
    SET_TexCoord3iv(disp, dlsym(RTLD_DEFAULT, "glTexCoord3iv"));
    SET_TexCoord3s(disp, dlsym(RTLD_DEFAULT, "glTexCoord3s"));
    SET_TexCoord3sv(disp, dlsym(RTLD_DEFAULT, "glTexCoord3sv"));
    SET_TexCoord4d(disp, dlsym(RTLD_DEFAULT, "glTexCoord4d"));
    SET_TexCoord4dv(disp, dlsym(RTLD_DEFAULT, "glTexCoord4dv"));
    SET_TexCoord4f(disp, dlsym(RTLD_DEFAULT, "glTexCoord4f"));
    SET_TexCoord4fv(disp, dlsym(RTLD_DEFAULT, "glTexCoord4fv"));
    SET_TexCoord4i(disp, dlsym(RTLD_DEFAULT, "glTexCoord4i"));
    SET_TexCoord4iv(disp, dlsym(RTLD_DEFAULT, "glTexCoord4iv"));
    SET_TexCoord4s(disp, dlsym(RTLD_DEFAULT, "glTexCoord4s"));
    SET_TexCoord4sv(disp, dlsym(RTLD_DEFAULT, "glTexCoord4sv"));
    SET_TexCoordPointer(disp, dlsym(RTLD_DEFAULT, "glTexCoordPointer"));
    SET_TexEnvf(disp, dlsym(RTLD_DEFAULT, "glTexEnvf"));
    SET_TexEnvfv(disp, dlsym(RTLD_DEFAULT, "glTexEnvfv"));
    SET_TexEnvi(disp, dlsym(RTLD_DEFAULT, "glTexEnvi"));
    SET_TexEnviv(disp, dlsym(RTLD_DEFAULT, "glTexEnviv"));
    SET_TexGend(disp, dlsym(RTLD_DEFAULT, "glTexGend"));
    SET_TexGendv(disp, dlsym(RTLD_DEFAULT, "glTexGendv"));
    SET_TexGenf(disp, dlsym(RTLD_DEFAULT, "glTexGenf"));
    SET_TexGenfv(disp, dlsym(RTLD_DEFAULT, "glTexGenfv"));
    SET_TexGeni(disp, dlsym(RTLD_DEFAULT, "glTexGeni"));
    SET_TexGeniv(disp, dlsym(RTLD_DEFAULT, "glTexGeniv"));
    
    /* Pointer Incompatability:
     * internalformat is a GLenum according to /System/Library/Frameworks/OpenGL.framework/Headers/gl.h
     * extern void glTexImage1D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
     * extern void glTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
     * extern void glTexImage3D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
     *
     * and it's a GLint in glx/glapitable.h and according to the man page
     * void ( * TexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
     * void ( * TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
     * void ( * TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
     *
     * <rdar://problem/6953344> gl.h contains incorrect prototypes for glTexImage[123]D
     */
    
    SET_TexImage1D(disp, dlsym(RTLD_DEFAULT, "glTexImage1D"));
    SET_TexImage2D(disp, dlsym(RTLD_DEFAULT, "glTexImage2D"));
    SET_TexImage3D(disp, dlsym(RTLD_DEFAULT, "glTexImage3D"));
    SET_TexParameterf(disp, dlsym(RTLD_DEFAULT, "glTexParameterf"));
    SET_TexParameterfv(disp, dlsym(RTLD_DEFAULT, "glTexParameterfv"));
    SET_TexParameteri(disp, dlsym(RTLD_DEFAULT, "glTexParameteri"));
    SET_TexParameteriv(disp, dlsym(RTLD_DEFAULT, "glTexParameteriv"));
    SET_TexSubImage1D(disp, dlsym(RTLD_DEFAULT, "glTexSubImage1D"));
    SET_TexSubImage2D(disp, dlsym(RTLD_DEFAULT, "glTexSubImage2D"));
    SET_TexSubImage3D(disp, dlsym(RTLD_DEFAULT, "glTexSubImage3D"));
    SET_Translated(disp, dlsym(RTLD_DEFAULT, "glTranslated"));
    SET_Translatef(disp, dlsym(RTLD_DEFAULT, "glTranslatef"));
    SET_Vertex2d(disp, dlsym(RTLD_DEFAULT, "glVertex2d"));
    SET_Vertex2dv(disp, dlsym(RTLD_DEFAULT, "glVertex2dv"));
    SET_Vertex2f(disp, dlsym(RTLD_DEFAULT, "glVertex2f"));
    SET_Vertex2fv(disp, dlsym(RTLD_DEFAULT, "glVertex2fv"));
    SET_Vertex2i(disp, dlsym(RTLD_DEFAULT, "glVertex2i"));
    SET_Vertex2iv(disp, dlsym(RTLD_DEFAULT, "glVertex2iv"));
    SET_Vertex2s(disp, dlsym(RTLD_DEFAULT, "glVertex2s"));
    SET_Vertex2sv(disp, dlsym(RTLD_DEFAULT, "glVertex2sv"));
    SET_Vertex3d(disp, dlsym(RTLD_DEFAULT, "glVertex3d"));
    SET_Vertex3dv(disp, dlsym(RTLD_DEFAULT, "glVertex3dv"));
    SET_Vertex3f(disp, dlsym(RTLD_DEFAULT, "glVertex3f"));
    SET_Vertex3fv(disp, dlsym(RTLD_DEFAULT, "glVertex3fv"));
    SET_Vertex3i(disp, dlsym(RTLD_DEFAULT, "glVertex3i"));
    SET_Vertex3iv(disp, dlsym(RTLD_DEFAULT, "glVertex3iv"));
    SET_Vertex3s(disp, dlsym(RTLD_DEFAULT, "glVertex3s"));
    SET_Vertex3sv(disp, dlsym(RTLD_DEFAULT, "glVertex3sv"));
    SET_Vertex4d(disp, dlsym(RTLD_DEFAULT, "glVertex4d"));
    SET_Vertex4dv(disp, dlsym(RTLD_DEFAULT, "glVertex4dv"));
    SET_Vertex4f(disp, dlsym(RTLD_DEFAULT, "glVertex4f"));
    SET_Vertex4fv(disp, dlsym(RTLD_DEFAULT, "glVertex4fv"));
    SET_Vertex4i(disp, dlsym(RTLD_DEFAULT, "glVertex4i"));
    SET_Vertex4iv(disp, dlsym(RTLD_DEFAULT, "glVertex4iv"));
    SET_Vertex4s(disp, dlsym(RTLD_DEFAULT, "glVertex4s"));
    SET_Vertex4sv(disp, dlsym(RTLD_DEFAULT, "glVertex4sv"));
    SET_VertexPointer(disp, dlsym(RTLD_DEFAULT, "glVertexPointer"));
    SET_Viewport(disp, dlsym(RTLD_DEFAULT, "glViewport"));

    /* GL_VERSION_2_0 */
    SET_AttachShader(disp, dlsym(RTLD_DEFAULT, "glAttachShader"));
    SET_DeleteShader(disp, dlsym(RTLD_DEFAULT, "glDeleteShader"));
    SET_DetachShader(disp, dlsym(RTLD_DEFAULT, "glDetachShader"));
    SET_GetAttachedShaders(disp, dlsym(RTLD_DEFAULT, "glGetAttachedShaders"));
    SET_GetProgramInfoLog(disp, dlsym(RTLD_DEFAULT, "glGetProgramInfoLog"));
    SET_GetShaderInfoLog(disp, dlsym(RTLD_DEFAULT, "glGetShaderInfoLog"));
    SET_GetShaderiv(disp, dlsym(RTLD_DEFAULT, "glGetShaderiv"));
    SET_IsShader(disp, dlsym(RTLD_DEFAULT, "glIsShader"));
    SET_StencilFuncSeparate(disp, dlsym(RTLD_DEFAULT, "glStencilFuncSeparate"));
    SET_StencilMaskSeparate(disp, dlsym(RTLD_DEFAULT, "glStencilMaskSeparate"));
    SET_StencilOpSeparate(disp, dlsym(RTLD_DEFAULT, "glStencilOpSeparate"));

    /* GL_VERSION_2_1 */
    SET_UniformMatrix2x3fv(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix2x3fv"));
    SET_UniformMatrix2x4fv(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix2x4fv"));
    SET_UniformMatrix3x2fv(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix3x2fv"));
    SET_UniformMatrix3x4fv(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix3x4fv"));
    SET_UniformMatrix4x2fv(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix4x2fv"));
    SET_UniformMatrix4x3fv(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix4x3fv"));

    /* GL_APPLE_vertex_array_object */
    SET_BindVertexArrayAPPLE(disp, dlsym(RTLD_DEFAULT, "glBindVertexArrayAPPLE"));
    SET_DeleteVertexArraysAPPLE(disp, dlsym(RTLD_DEFAULT, "glDeleteVertexArraysAPPLE"));
    SET_GenVertexArraysAPPLE(disp, dlsym(RTLD_DEFAULT, "glGenVertexArraysAPPLE"));
    SET_IsVertexArrayAPPLE(disp, dlsym(RTLD_DEFAULT, "glIsVertexArrayAPPLE"));

    /* GL_ARB_draw_buffers */
    SET_DrawBuffersARB(disp, dlsym(RTLD_DEFAULT, "glDrawBuffersARB"));

    /* GL_ARB_multisample */
    SET_SampleCoverageARB(disp, dlsym(RTLD_DEFAULT, "glSampleCoverageARB"));

    /* GL_ARB_multitexture */
    SET_ActiveTextureARB(disp, dlsym(RTLD_DEFAULT, "glActiveTextureARB"));
    SET_ClientActiveTextureARB(disp, dlsym(RTLD_DEFAULT, "glClientActiveTextureARB"));
    SET_MultiTexCoord1dARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord1dARB"));
    SET_MultiTexCoord1dvARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord1dvARB"));
    SET_MultiTexCoord1fARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord1fARB"));
    SET_MultiTexCoord1fvARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord1fvARB"));
    SET_MultiTexCoord1iARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord1iARB"));
    SET_MultiTexCoord1ivARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord1ivARB"));
    SET_MultiTexCoord1sARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord1sARB"));
    SET_MultiTexCoord1svARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord1svARB"));
    SET_MultiTexCoord2dARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord2dARB"));
    SET_MultiTexCoord2dvARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord2dvARB"));
    SET_MultiTexCoord2fARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord2fARB"));
    SET_MultiTexCoord2fvARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord2fvARB"));
    SET_MultiTexCoord2iARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord2iARB"));
    SET_MultiTexCoord2ivARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord2ivARB"));
    SET_MultiTexCoord2sARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord2sARB"));
    SET_MultiTexCoord2svARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord2svARB"));
    SET_MultiTexCoord3dARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord3dARB"));
    SET_MultiTexCoord3dvARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord3dvARB"));
    SET_MultiTexCoord3fARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord3fARB"));
    SET_MultiTexCoord3fvARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord3fvARB"));
    SET_MultiTexCoord3iARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord3iARB"));
    SET_MultiTexCoord3ivARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord3ivARB"));
    SET_MultiTexCoord3sARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord3sARB"));
    SET_MultiTexCoord3svARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord3svARB"));
    SET_MultiTexCoord4dARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord4dARB"));
    SET_MultiTexCoord4dvARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord4dvARB"));
    SET_MultiTexCoord4fARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord4fARB"));
    SET_MultiTexCoord4fvARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord4fvARB"));
    SET_MultiTexCoord4iARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord4iARB"));
    SET_MultiTexCoord4ivARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord4ivARB"));
    SET_MultiTexCoord4sARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord4sARB"));
    SET_MultiTexCoord4svARB(disp, dlsym(RTLD_DEFAULT, "glMultiTexCoord4svARB"));

    /* GL_ARB_occlusion_query */
    SET_BeginQueryARB(disp, dlsym(RTLD_DEFAULT, "glBeginQueryARB"));
    SET_DeleteQueriesARB(disp, dlsym(RTLD_DEFAULT, "glDeleteQueriesARB"));
    SET_EndQueryARB(disp, dlsym(RTLD_DEFAULT, "glEndQueryARB"));
    SET_GenQueriesARB(disp, dlsym(RTLD_DEFAULT, "glGenQueriesARB"));
    SET_GetQueryObjectivARB(disp, dlsym(RTLD_DEFAULT, "glGetQueryObjectivARB"));
    SET_GetQueryObjectuivARB(disp, dlsym(RTLD_DEFAULT, "glGetQueryObjectuivARB"));
    SET_GetQueryivARB(disp, dlsym(RTLD_DEFAULT, "glGetQueryivARB"));
    SET_IsQueryARB(disp, dlsym(RTLD_DEFAULT, "glIsQueryARB"));

    /* GL_ARB_shader_objects */
    SET_AttachObjectARB(disp, dlsym(RTLD_DEFAULT, "glAttachObjectARB"));
    SET_CompileShaderARB(disp, dlsym(RTLD_DEFAULT, "glCompileShaderARB"));
    SET_DeleteObjectARB(disp, dlsym(RTLD_DEFAULT, "glDeleteObjectARB"));
    SET_GetHandleARB(disp, dlsym(RTLD_DEFAULT, "glGetHandleARB"));
    SET_DetachObjectARB(disp, dlsym(RTLD_DEFAULT, "glDetachObjectARB"));
    SET_CreateProgramObjectARB(disp, dlsym(RTLD_DEFAULT, "glCreateProgramObjectARB"));
    SET_CreateShaderObjectARB(disp, dlsym(RTLD_DEFAULT, "glCreateShaderObjectARB"));
    SET_GetInfoLogARB(disp, dlsym(RTLD_DEFAULT, "glGetInfoLogARB"));
    SET_GetActiveUniformARB(disp, dlsym(RTLD_DEFAULT, "glGetActiveUniformARB"));
    SET_GetAttachedObjectsARB(disp, dlsym(RTLD_DEFAULT, "glGetAttachedObjectsARB"));
    SET_GetObjectParameterfvARB(disp, dlsym(RTLD_DEFAULT, "glGetObjectParameterfvARB"));
    SET_GetObjectParameterivARB(disp, dlsym(RTLD_DEFAULT, "glGetObjectParameterivARB"));
    SET_GetShaderSourceARB(disp, dlsym(RTLD_DEFAULT, "glGetShaderSourceARB"));
    SET_GetUniformLocationARB(disp, dlsym(RTLD_DEFAULT, "glGetUniformLocationARB"));
    SET_GetUniformfvARB(disp, dlsym(RTLD_DEFAULT, "glGetUniformfvARB"));
    SET_GetUniformivARB(disp, dlsym(RTLD_DEFAULT, "glGetUniformivARB"));
    SET_LinkProgramARB(disp, dlsym(RTLD_DEFAULT, "glLinkProgramARB"));
    SET_ShaderSourceARB(disp, dlsym(RTLD_DEFAULT, "glShaderSourceARB"));
    SET_Uniform1fARB(disp, dlsym(RTLD_DEFAULT, "glUniform1fARB"));
    SET_Uniform1fvARB(disp, dlsym(RTLD_DEFAULT, "glUniform1fvARB"));
    SET_Uniform1iARB(disp, dlsym(RTLD_DEFAULT, "glUniform1iARB"));
    SET_Uniform1ivARB(disp, dlsym(RTLD_DEFAULT, "glUniform1ivARB"));
    SET_Uniform2fARB(disp, dlsym(RTLD_DEFAULT, "glUniform2fARB"));
    SET_Uniform2fvARB(disp, dlsym(RTLD_DEFAULT, "glUniform2fvARB"));
    SET_Uniform2iARB(disp, dlsym(RTLD_DEFAULT, "glUniform2iARB"));
    SET_Uniform2ivARB(disp, dlsym(RTLD_DEFAULT, "glUniform2ivARB"));
    SET_Uniform3fARB(disp, dlsym(RTLD_DEFAULT, "glUniform3fARB"));
    SET_Uniform3fvARB(disp, dlsym(RTLD_DEFAULT, "glUniform3fvARB"));
    SET_Uniform3iARB(disp, dlsym(RTLD_DEFAULT, "glUniform3iARB"));
    SET_Uniform3ivARB(disp, dlsym(RTLD_DEFAULT, "glUniform3ivARB"));
    SET_Uniform4fARB(disp, dlsym(RTLD_DEFAULT, "glUniform4fARB"));
    SET_Uniform4fvARB(disp, dlsym(RTLD_DEFAULT, "glUniform4fvARB"));
    SET_Uniform4iARB(disp, dlsym(RTLD_DEFAULT, "glUniform4iARB"));
    SET_Uniform4ivARB(disp, dlsym(RTLD_DEFAULT, "glUniform4ivARB"));
    SET_UniformMatrix2fvARB(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix2fvARB"));
    SET_UniformMatrix3fvARB(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix3fvARB"));
    SET_UniformMatrix4fvARB(disp, dlsym(RTLD_DEFAULT, "glUniformMatrix4fvARB"));
    SET_UseProgramObjectARB(disp, dlsym(RTLD_DEFAULT, "glUseProgramObjectARB"));
    SET_ValidateProgramARB(disp, dlsym(RTLD_DEFAULT, "glValidateProgramARB"));

    /* GL_ARB_texture_compression */
    SET_CompressedTexImage1DARB(disp, dlsym(RTLD_DEFAULT, "glCompressedTexImage1DARB"));
    SET_CompressedTexImage2DARB(disp, dlsym(RTLD_DEFAULT, "glCompressedTexImage2DARB"));
    SET_CompressedTexImage3DARB(disp, dlsym(RTLD_DEFAULT, "glCompressedTexImage3DARB"));
    SET_CompressedTexSubImage1DARB(disp, dlsym(RTLD_DEFAULT, "glCompressedTexSubImage1DARB"));
    SET_CompressedTexSubImage2DARB(disp, dlsym(RTLD_DEFAULT, "glCompressedTexSubImage2DARB"));
    SET_CompressedTexSubImage3DARB(disp, dlsym(RTLD_DEFAULT, "glCompressedTexSubImage3DARB"));
    SET_GetCompressedTexImageARB(disp, dlsym(RTLD_DEFAULT, "glGetCompressedTexImageARB"));

    /* GL_ARB_transpose_matrix */
    SET_LoadTransposeMatrixdARB(disp, dlsym(RTLD_DEFAULT, "glLoadTransposeMatrixdARB"));
    SET_LoadTransposeMatrixfARB(disp, dlsym(RTLD_DEFAULT, "glLoadTransposeMatrixfARB"));
    SET_MultTransposeMatrixdARB(disp, dlsym(RTLD_DEFAULT, "glMultTransposeMatrixdARB"));
    SET_MultTransposeMatrixfARB(disp, dlsym(RTLD_DEFAULT, "glMultTransposeMatrixfARB"));

    /* GL_ARB_vertex_buffer_object */
    SET_BindBufferARB(disp, dlsym(RTLD_DEFAULT, "glBindBufferARB"));
    SET_BufferDataARB(disp, dlsym(RTLD_DEFAULT, "glBufferDataARB"));
    SET_BufferSubDataARB(disp, dlsym(RTLD_DEFAULT, "glBufferSubDataARB"));
    SET_DeleteBuffersARB(disp, dlsym(RTLD_DEFAULT, "glDeleteBuffersARB"));
    SET_GenBuffersARB(disp, dlsym(RTLD_DEFAULT, "glGenBuffersARB"));
    SET_GetBufferParameterivARB(disp, dlsym(RTLD_DEFAULT, "glGetBufferParameterivARB"));
    SET_GetBufferPointervARB(disp, dlsym(RTLD_DEFAULT, "glGetBufferPointervARB"));
    SET_GetBufferSubDataARB(disp, dlsym(RTLD_DEFAULT, "glGetBufferSubDataARB"));
    SET_IsBufferARB(disp, dlsym(RTLD_DEFAULT, "glIsBufferARB"));
    SET_MapBufferARB(disp, dlsym(RTLD_DEFAULT, "glMapBufferARB"));
    SET_UnmapBufferARB(disp, dlsym(RTLD_DEFAULT, "glUnmapBufferARB"));

    /* GL_ARB_vertex_program */
    SET_DisableVertexAttribArrayARB(disp, dlsym(RTLD_DEFAULT, "glDisableVertexAttribArrayARB"));
    SET_EnableVertexAttribArrayARB(disp, dlsym(RTLD_DEFAULT, "glEnableVertexAttribArrayARB"));
    SET_GetProgramEnvParameterdvARB(disp, dlsym(RTLD_DEFAULT, "glGetProgramEnvParameterdvARB"));
    SET_GetProgramEnvParameterfvARB(disp, dlsym(RTLD_DEFAULT, "glGetProgramEnvParameterfvARB"));
    SET_GetProgramLocalParameterdvARB(disp, dlsym(RTLD_DEFAULT, "glGetProgramLocalParameterdvARB"));
    SET_GetProgramLocalParameterfvARB(disp, dlsym(RTLD_DEFAULT, "glGetProgramLocalParameterfvARB"));
    SET_GetProgramStringARB(disp, dlsym(RTLD_DEFAULT, "glGetProgramStringARB"));
    SET_GetProgramivARB(disp, dlsym(RTLD_DEFAULT, "glGetProgramivARB"));
    SET_GetVertexAttribdvARB(disp, dlsym(RTLD_DEFAULT, "glGetVertexAttribdvARB"));
    SET_GetVertexAttribfvARB(disp, dlsym(RTLD_DEFAULT, "glGetVertexAttribfvARB"));
    SET_GetVertexAttribivARB(disp, dlsym(RTLD_DEFAULT, "glGetVertexAttribivARB"));
    SET_ProgramEnvParameter4dARB(disp, dlsym(RTLD_DEFAULT, "glProgramEnvParameter4dARB"));
    SET_ProgramEnvParameter4dvARB(disp, dlsym(RTLD_DEFAULT, "glProgramEnvParameter4dvARB"));
    SET_ProgramEnvParameter4fARB(disp, dlsym(RTLD_DEFAULT, "glProgramEnvParameter4fARB"));
    SET_ProgramEnvParameter4fvARB(disp, dlsym(RTLD_DEFAULT, "glProgramEnvParameter4fvARB"));
    SET_ProgramLocalParameter4dARB(disp, dlsym(RTLD_DEFAULT, "glProgramLocalParameter4dARB"));
    SET_ProgramLocalParameter4dvARB(disp, dlsym(RTLD_DEFAULT, "glProgramLocalParameter4dvARB"));
    SET_ProgramLocalParameter4fARB(disp, dlsym(RTLD_DEFAULT, "glProgramLocalParameter4fARB"));
    SET_ProgramLocalParameter4fvARB(disp, dlsym(RTLD_DEFAULT, "glProgramLocalParameter4fvARB"));
    SET_ProgramStringARB(disp, dlsym(RTLD_DEFAULT, "glProgramStringARB"));
    SET_VertexAttrib1dARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1dARB"));
    SET_VertexAttrib1dvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1dvARB"));
    SET_VertexAttrib1fARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1fARB"));
    SET_VertexAttrib1fvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1fvARB"));
    SET_VertexAttrib1sARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1sARB"));
    SET_VertexAttrib1svARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1svARB"));
    SET_VertexAttrib2dARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2dARB"));
    SET_VertexAttrib2dvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2dvARB"));
    SET_VertexAttrib2fARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2fARB"));
    SET_VertexAttrib2fvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2fvARB"));
    SET_VertexAttrib2sARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2sARB"));
    SET_VertexAttrib2svARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2svARB"));
    SET_VertexAttrib3dARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3dARB"));
    SET_VertexAttrib3dvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3dvARB"));
    SET_VertexAttrib3fARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3fARB"));
    SET_VertexAttrib3fvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3fvARB"));
    SET_VertexAttrib3sARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3sARB"));
    SET_VertexAttrib3svARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3svARB"));
    SET_VertexAttrib4NbvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4NbvARB"));
    SET_VertexAttrib4NivARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4NivARB"));
    SET_VertexAttrib4NsvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4NsvARB"));
    SET_VertexAttrib4NubARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4NubARB"));
    SET_VertexAttrib4NubvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4NubvARB"));
    SET_VertexAttrib4NuivARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4NuivARB"));
    SET_VertexAttrib4NusvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4NusvARB"));
    SET_VertexAttrib4bvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4bvARB"));
    SET_VertexAttrib4dARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4dARB"));
    SET_VertexAttrib4dvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4dvARB"));
    SET_VertexAttrib4fARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4fARB"));
    SET_VertexAttrib4fvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4fvARB"));
    SET_VertexAttrib4ivARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4ivARB"));
    SET_VertexAttrib4sARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4sARB"));
    SET_VertexAttrib4svARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4svARB"));
    SET_VertexAttrib4ubvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4ubvARB"));
    SET_VertexAttrib4uivARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4uivARB"));
    SET_VertexAttrib4usvARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4usvARB"));
    SET_VertexAttribPointerARB(disp, dlsym(RTLD_DEFAULT, "glVertexAttribPointerARB"));

    /* GL_ARB_vertex_shader */
    SET_BindAttribLocationARB(disp, dlsym(RTLD_DEFAULT, "glBindAttribLocationARB"));
    SET_GetActiveAttribARB(disp, dlsym(RTLD_DEFAULT, "glGetActiveAttribARB"));
    SET_GetAttribLocationARB(disp, dlsym(RTLD_DEFAULT, "glGetAttribLocationARB"));

    /* GL_ARB_window_pos */
    SET_WindowPos2dMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos2dARB"));
    SET_WindowPos2dvMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos2dvARB"));
    SET_WindowPos2fMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos2fARB"));
    SET_WindowPos2fvMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos2fvARB"));
    SET_WindowPos2iMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos2iARB"));
    SET_WindowPos2ivMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos2ivARB"));
    SET_WindowPos2sMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos2sARB"));
    SET_WindowPos2svMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos2svARB"));
    SET_WindowPos3dMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos3dARB"));
    SET_WindowPos3dvMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos3dvARB"));
    SET_WindowPos3fMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos3fARB"));
    SET_WindowPos3fvMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos3fvARB"));
    SET_WindowPos3iMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos3iARB"));
    SET_WindowPos3ivMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos3ivARB"));
    SET_WindowPos3sMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos3sARB"));
    SET_WindowPos3svMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos3svARB"));

    /* GL_ATI_fragment_shader / GL_EXT_fragment_shader */
    if(dlsym(RTLD_DEFAULT, "glAlphaFragmentOp1ATI")) {
        /* GL_ATI_fragment_shader */
        SET_AlphaFragmentOp1ATI(disp, dlsym(RTLD_DEFAULT, "glAlphaFragmentOp1ATI"));
        SET_AlphaFragmentOp2ATI(disp, dlsym(RTLD_DEFAULT, "glAlphaFragmentOp2ATI"));
        SET_AlphaFragmentOp3ATI(disp, dlsym(RTLD_DEFAULT, "glAlphaFragmentOp3ATI"));
        SET_BeginFragmentShaderATI(disp, dlsym(RTLD_DEFAULT, "glBeginFragmentShaderATI"));
        SET_BindFragmentShaderATI(disp, dlsym(RTLD_DEFAULT, "glBindFragmentShaderATI"));
        SET_ColorFragmentOp1ATI(disp, dlsym(RTLD_DEFAULT, "glColorFragmentOp1ATI"));
        SET_ColorFragmentOp2ATI(disp, dlsym(RTLD_DEFAULT, "glColorFragmentOp2ATI"));
        SET_ColorFragmentOp3ATI(disp, dlsym(RTLD_DEFAULT, "glColorFragmentOp3ATI"));
        SET_DeleteFragmentShaderATI(disp, dlsym(RTLD_DEFAULT, "glDeleteFragmentShaderATI"));
        SET_EndFragmentShaderATI(disp, dlsym(RTLD_DEFAULT, "glEndFragmentShaderATI"));
        SET_GenFragmentShadersATI(disp, dlsym(RTLD_DEFAULT, "glGenFragmentShadersATI"));
        SET_PassTexCoordATI(disp, dlsym(RTLD_DEFAULT, "glPassTexCoordATI"));
        SET_SampleMapATI(disp, dlsym(RTLD_DEFAULT, "glSampleMapATI"));
        SET_SetFragmentShaderConstantATI(disp, dlsym(RTLD_DEFAULT, "glSetFragmentShaderConstantATI"));
    } else {
        /* GL_EXT_fragment_shader */
        SET_AlphaFragmentOp1ATI(disp, dlsym(RTLD_DEFAULT, "glAlphaFragmentOp1EXT"));
        SET_AlphaFragmentOp2ATI(disp, dlsym(RTLD_DEFAULT, "glAlphaFragmentOp2EXT"));
        SET_AlphaFragmentOp3ATI(disp, dlsym(RTLD_DEFAULT, "glAlphaFragmentOp3EXT"));
        SET_BeginFragmentShaderATI(disp, dlsym(RTLD_DEFAULT, "glBeginFragmentShaderEXT"));
        SET_BindFragmentShaderATI(disp, dlsym(RTLD_DEFAULT, "glBindFragmentShaderEXT"));
        SET_ColorFragmentOp1ATI(disp, dlsym(RTLD_DEFAULT, "glColorFragmentOp1EXT"));
        SET_ColorFragmentOp2ATI(disp, dlsym(RTLD_DEFAULT, "glColorFragmentOp2EXT"));
        SET_ColorFragmentOp3ATI(disp, dlsym(RTLD_DEFAULT, "glColorFragmentOp3EXT"));
        SET_DeleteFragmentShaderATI(disp, dlsym(RTLD_DEFAULT, "glDeleteFragmentShaderEXT"));
        SET_EndFragmentShaderATI(disp, dlsym(RTLD_DEFAULT, "glEndFragmentShaderEXT"));
        SET_GenFragmentShadersATI(disp, dlsym(RTLD_DEFAULT, "glGenFragmentShadersEXT"));
        SET_PassTexCoordATI(disp, dlsym(RTLD_DEFAULT, "glPassTexCoordEXT"));
        SET_SampleMapATI(disp, dlsym(RTLD_DEFAULT, "glSampleMapEXT"));
        SET_SetFragmentShaderConstantATI(disp, dlsym(RTLD_DEFAULT, "glSetFragmentShaderConstantEXT"));
    }

    /* GL_ATI_separate_stencil */
    SET_StencilFuncSeparateATI(disp, dlsym(RTLD_DEFAULT, "glStencilFuncSeparateATI"));

    /* GL_EXT_blend_equation_separate */
    SET_BlendEquationSeparateEXT(disp, dlsym(RTLD_DEFAULT, "glBlendEquationSeparateEXT"));

    /* GL_EXT_blend_func_separate */
    SET_BlendFuncSeparateEXT(disp, dlsym(RTLD_DEFAULT, "glBlendFuncSeparateEXT"));

    /* GL_EXT_depth_bounds_test */
    SET_DepthBoundsEXT(disp, dlsym(RTLD_DEFAULT, "glDepthBoundsEXT"));

    /* GL_EXT_compiled_vertex_array */
    SET_LockArraysEXT(disp, dlsym(RTLD_DEFAULT, "glLockArraysEXT"));
    SET_UnlockArraysEXT(disp, dlsym(RTLD_DEFAULT, "glUnlockArraysEXT"));

    /* GL_EXT_cull_vertex */
    SET_CullParameterdvEXT(disp, dlsym(RTLD_DEFAULT, "glCullParameterdvEXT"));
    SET_CullParameterfvEXT(disp, dlsym(RTLD_DEFAULT, "glCullParameterfvEXT"));

    /* GL_EXT_fog_coord */
    SET_FogCoordPointerEXT(disp, dlsym(RTLD_DEFAULT, "glFogCoordPointerEXT"));
    SET_FogCoorddEXT(disp, dlsym(RTLD_DEFAULT, "glFogCoorddEXT"));
    SET_FogCoorddvEXT(disp, dlsym(RTLD_DEFAULT, "glFogCoorddvEXT"));
    SET_FogCoordfEXT(disp, dlsym(RTLD_DEFAULT, "glFogCoordfEXT"));
    SET_FogCoordfvEXT(disp, dlsym(RTLD_DEFAULT, "glFogCoordfvEXT"));

    /* GL_EXT_framebuffer_blit */
    SET_BlitFramebufferEXT(disp, dlsym(RTLD_DEFAULT, "glBlitFramebufferEXT"));

    /* GL_EXT_framebuffer_object */
    SET_BindFramebufferEXT(disp, dlsym(RTLD_DEFAULT, "glBindFramebufferEXT"));
    SET_BindRenderbufferEXT(disp, dlsym(RTLD_DEFAULT, "glBindRenderbufferEXT"));
    SET_CheckFramebufferStatusEXT(disp, dlsym(RTLD_DEFAULT, "glCheckFramebufferStatusEXT"));
    SET_DeleteFramebuffersEXT(disp, dlsym(RTLD_DEFAULT, "glDeleteFramebuffersEXT"));
    SET_DeleteRenderbuffersEXT(disp, dlsym(RTLD_DEFAULT, "glDeleteRenderbuffersEXT"));
    SET_FramebufferRenderbufferEXT(disp, dlsym(RTLD_DEFAULT, "glFramebufferRenderbufferEXT"));
    SET_FramebufferTexture1DEXT(disp, dlsym(RTLD_DEFAULT, "glFramebufferTexture1DEXT"));
    SET_FramebufferTexture2DEXT(disp, dlsym(RTLD_DEFAULT, "glFramebufferTexture2DEXT"));
    SET_FramebufferTexture3DEXT(disp, dlsym(RTLD_DEFAULT, "glFramebufferTexture3DEXT"));
    SET_GenerateMipmapEXT(disp, dlsym(RTLD_DEFAULT, "glGenerateMipmapEXT"));
    SET_GenFramebuffersEXT(disp, dlsym(RTLD_DEFAULT, "glGenFramebuffersEXT"));
    SET_GenRenderbuffersEXT(disp, dlsym(RTLD_DEFAULT, "glGenRenderbuffersEXT"));
    SET_GetFramebufferAttachmentParameterivEXT(disp, dlsym(RTLD_DEFAULT, "glGetFramebufferAttachmentParameterivEXT"));
    SET_GetRenderbufferParameterivEXT(disp, dlsym(RTLD_DEFAULT, "glGetRenderbufferParameterivEXT"));
    SET_IsFramebufferEXT(disp, dlsym(RTLD_DEFAULT, "glIsFramebufferEXT"));
    SET_IsRenderbufferEXT(disp, dlsym(RTLD_DEFAULT, "glIsRenderbufferEXT"));
    SET_RenderbufferStorageEXT(disp, dlsym(RTLD_DEFAULT, "glRenderbufferStorageEXT"));

    /* GL_EXT_gpu_program_parameters */
    SET_ProgramEnvParameters4fvEXT(disp, dlsym(RTLD_DEFAULT, "glProgramEnvParameters4fvEXT"));
    SET_ProgramLocalParameters4fvEXT(disp, dlsym(RTLD_DEFAULT, "glProgramLocalParameters4fvEXT"));

    /* Pointer Incompatability:
     * This warning can be safely ignored.  OpenGL.framework adds const to the
     * two pointers.
     *
     * extern void glMultiDrawArraysEXT (GLenum, const GLint *, const GLsizei *, GLsizei);
     *
     * void ( * MultiDrawArraysEXT)(GLenum mode, GLint * first, GLsizei * count, GLsizei primcount);
     */

    /* GL_EXT_multi_draw_arrays */
    SET_MultiDrawArraysEXT(disp, (void *)dlsym(RTLD_DEFAULT, "glMultiDrawArraysEXT"));
    SET_MultiDrawElementsEXT(disp, dlsym(RTLD_DEFAULT, "glMultiDrawElementsEXT"));

    /* GL_EXT_point_parameters / GL_ARB_point_parameters */
    if(dlsym(RTLD_DEFAULT, "glPointParameterfEXT")) {
        /* GL_EXT_point_parameters */
        SET_PointParameterfEXT(disp, dlsym(RTLD_DEFAULT, "glPointParameterfEXT"));
        SET_PointParameterfvEXT(disp, dlsym(RTLD_DEFAULT, "glPointParameterfvEXT"));
    } else {
        /* GL_ARB_point_parameters */
        SET_PointParameterfEXT(disp, dlsym(RTLD_DEFAULT, "glPointParameterfARB"));
        SET_PointParameterfvEXT(disp, dlsym(RTLD_DEFAULT, "glPointParameterfvARB"));
    }

    /* GL_EXT_polygon_offset */
    SET_PolygonOffsetEXT(disp, dlsym(RTLD_DEFAULT, "glPolygonOffsetEXT"));

    /* GL_EXT_secondary_color */
    SET_SecondaryColor3bEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3bEXT"));
    SET_SecondaryColor3bvEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3bvEXT"));
    SET_SecondaryColor3dEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3dEXT"));
    SET_SecondaryColor3dvEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3dvEXT"));
    SET_SecondaryColor3fEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3fEXT"));
    SET_SecondaryColor3fvEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3fvEXT"));
    SET_SecondaryColor3iEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3iEXT"));
    SET_SecondaryColor3ivEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3ivEXT"));
    SET_SecondaryColor3sEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3sEXT"));
    SET_SecondaryColor3svEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3svEXT"));
    SET_SecondaryColor3ubEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3ubEXT"));
    SET_SecondaryColor3ubvEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3ubvEXT"));
    SET_SecondaryColor3uiEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3uiEXT"));
    SET_SecondaryColor3uivEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3uivEXT"));
    SET_SecondaryColor3usEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3usEXT"));
    SET_SecondaryColor3usvEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColor3usvEXT"));
    SET_SecondaryColorPointerEXT(disp, dlsym(RTLD_DEFAULT, "glSecondaryColorPointerEXT"));

    /* GL_EXT_stencil_two_side */
    SET_ActiveStencilFaceEXT(disp, dlsym(RTLD_DEFAULT, "glActiveStencilFaceEXT"));

    /* GL_EXT_timer_query */
    SET_GetQueryObjecti64vEXT(disp, dlsym(RTLD_DEFAULT, "glGetQueryObjecti64vEXT"));
    SET_GetQueryObjectui64vEXT(disp, dlsym(RTLD_DEFAULT, "glGetQueryObjectui64vEXT"));

    /* GL_EXT_vertex_array */
    SET_ColorPointerEXT(disp, dlsym(RTLD_DEFAULT, "glColorPointerEXT"));
    SET_EdgeFlagPointerEXT(disp, dlsym(RTLD_DEFAULT, "glEdgeFlagPointerEXT"));
    SET_IndexPointerEXT(disp, dlsym(RTLD_DEFAULT, "glIndexPointerEXT"));
    SET_NormalPointerEXT(disp, dlsym(RTLD_DEFAULT, "glNormalPointerEXT"));
    SET_TexCoordPointerEXT(disp, dlsym(RTLD_DEFAULT, "glTexCoordPointerEXT"));
    SET_VertexPointerEXT(disp, dlsym(RTLD_DEFAULT, "glVertexPointerEXT"));

    /* GL_IBM_multimode_draw_arrays */
    SET_MultiModeDrawArraysIBM(disp, dlsym(RTLD_DEFAULT, "glMultiModeDrawArraysIBM"));
    SET_MultiModeDrawElementsIBM(disp, dlsym(RTLD_DEFAULT, "glMultiModeDrawElementsIBM"));

    /* GL_MESA_resize_buffers */
    SET_ResizeBuffersMESA(disp, dlsym(RTLD_DEFAULT, "glResizeBuffersMESA"));

    /* GL_MESA_window_pos */
    SET_WindowPos4dMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos4dMESA"));
    SET_WindowPos4dvMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos4dvMESA"));
    SET_WindowPos4fMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos4fMESA"));
    SET_WindowPos4fvMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos4fvMESA"));
    SET_WindowPos4iMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos4iMESA"));
    SET_WindowPos4ivMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos4ivMESA"));
    SET_WindowPos4sMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos4sMESA"));
    SET_WindowPos4svMESA(disp, dlsym(RTLD_DEFAULT, "glWindowPos4svMESA"));

    /* GL_NV_fence */
    SET_DeleteFencesNV(disp, dlsym(RTLD_DEFAULT, "glDeleteFencesNV"));
    SET_FinishFenceNV(disp, dlsym(RTLD_DEFAULT, "glFinishFenceNV"));
    SET_GenFencesNV(disp, dlsym(RTLD_DEFAULT, "glGenFencesNV"));
    SET_GetFenceivNV(disp, dlsym(RTLD_DEFAULT, "glGetFenceivNV"));
    SET_IsFenceNV(disp, dlsym(RTLD_DEFAULT, "glIsFenceNV"));
    SET_SetFenceNV(disp, dlsym(RTLD_DEFAULT, "glSetFenceNV"));
    SET_TestFenceNV(disp, dlsym(RTLD_DEFAULT, "glTestFenceNV"));

    /* GL_NV_fragment_program */
    SET_GetProgramNamedParameterdvNV(disp, dlsym(RTLD_DEFAULT, "glGetProgramNamedParameterdvNV"));
    SET_GetProgramNamedParameterfvNV(disp, dlsym(RTLD_DEFAULT, "glGetProgramNamedParameterfvNV"));
    SET_ProgramNamedParameter4dNV(disp, dlsym(RTLD_DEFAULT, "glProgramNamedParameter4dNV"));
    SET_ProgramNamedParameter4dvNV(disp, dlsym(RTLD_DEFAULT, "glProgramNamedParameter4dvNV"));
    SET_ProgramNamedParameter4fNV(disp, dlsym(RTLD_DEFAULT, "glProgramNamedParameter4fNV"));
    SET_ProgramNamedParameter4fvNV(disp, dlsym(RTLD_DEFAULT, "glProgramNamedParameter4fvNV"));

    /* GL_NV_geometry_program4 */
    SET_FramebufferTextureLayerEXT(disp, dlsym(RTLD_DEFAULT, "glFramebufferTextureLayerEXT"));

    /* GL_NV_point_sprite */
    SET_PointParameteriNV(disp, dlsym(RTLD_DEFAULT, "glPointParameteriNV"));
    SET_PointParameterivNV(disp, dlsym(RTLD_DEFAULT, "glPointParameterivNV"));

    /* GL_NV_register_combiners */
    SET_CombinerInputNV(disp, dlsym(RTLD_DEFAULT, "glCombinerInputNV"));
    SET_CombinerOutputNV(disp, dlsym(RTLD_DEFAULT, "glCombinerOutputNV"));
    SET_CombinerParameterfNV(disp, dlsym(RTLD_DEFAULT, "glCombinerParameterfNV"));
    SET_CombinerParameterfvNV(disp, dlsym(RTLD_DEFAULT, "glCombinerParameterfvNV"));
    SET_CombinerParameteriNV(disp, dlsym(RTLD_DEFAULT, "glCombinerParameteriNV"));
    SET_CombinerParameterivNV(disp, dlsym(RTLD_DEFAULT, "glCombinerParameterivNV"));
    SET_FinalCombinerInputNV(disp, dlsym(RTLD_DEFAULT, "glFinalCombinerInputNV"));
    SET_GetCombinerInputParameterfvNV(disp, dlsym(RTLD_DEFAULT, "glGetCombinerInputParameterfvNV"));
    SET_GetCombinerInputParameterivNV(disp, dlsym(RTLD_DEFAULT, "glGetCombinerInputParameterivNV"));
    SET_GetCombinerOutputParameterfvNV(disp, dlsym(RTLD_DEFAULT, "glGetCombinerOutputParameterfvNV"));
    SET_GetCombinerOutputParameterivNV(disp, dlsym(RTLD_DEFAULT, "glGetCombinerOutputParameterivNV"));
    SET_GetFinalCombinerInputParameterfvNV(disp, dlsym(RTLD_DEFAULT, "glGetFinalCombinerInputParameterfvNV"));
    SET_GetFinalCombinerInputParameterivNV(disp, dlsym(RTLD_DEFAULT, "glGetFinalCombinerInputParameterivNV"));

    /* GL_NV_vertex_array_range */
    SET_FlushVertexArrayRangeNV(disp, dlsym(RTLD_DEFAULT, "glFlushVertexArrayRangeNV"));
    SET_VertexArrayRangeNV(disp, dlsym(RTLD_DEFAULT, "glVertexArrayRangeNV"));

    /* GL_NV_vertex_program */
    SET_AreProgramsResidentNV(disp, dlsym(RTLD_DEFAULT, "glAreProgramsResidentNV"));
    SET_BindProgramNV(disp, dlsym(RTLD_DEFAULT, "glBindProgramNV"));
    SET_DeleteProgramsNV(disp, dlsym(RTLD_DEFAULT, "glDeleteProgramsNV"));
    SET_ExecuteProgramNV(disp, dlsym(RTLD_DEFAULT, "glExecuteProgramNV"));
    SET_GenProgramsNV(disp, dlsym(RTLD_DEFAULT, "glGenProgramsNV"));
    SET_GetProgramParameterdvNV(disp, dlsym(RTLD_DEFAULT, "glGetProgramParameterdvNV"));
    SET_GetProgramParameterfvNV(disp, dlsym(RTLD_DEFAULT, "glGetProgramParameterfvNV"));
    SET_GetProgramStringNV(disp, dlsym(RTLD_DEFAULT, "glGetProgramStringNV"));
    SET_GetProgramivNV(disp, dlsym(RTLD_DEFAULT, "glGetProgramivNV"));
    SET_GetTrackMatrixivNV(disp, dlsym(RTLD_DEFAULT, "glGetTrackMatrixivNV"));
    SET_GetVertexAttribPointervNV(disp, dlsym(RTLD_DEFAULT, "glGetVertexAttribPointervNV"));
    SET_GetVertexAttribdvNV(disp, dlsym(RTLD_DEFAULT, "glGetVertexAttribdvNV"));
    SET_GetVertexAttribfvNV(disp, dlsym(RTLD_DEFAULT, "glGetVertexAttribfvNV"));
    SET_GetVertexAttribivNV(disp, dlsym(RTLD_DEFAULT, "glGetVertexAttribivNV"));
    SET_IsProgramNV(disp, dlsym(RTLD_DEFAULT, "glIsProgramNV"));
    SET_LoadProgramNV(disp, dlsym(RTLD_DEFAULT, "glLoadProgramNV"));
    SET_ProgramParameters4dvNV(disp, dlsym(RTLD_DEFAULT, "glProgramParameters4dvNV"));
    SET_ProgramParameters4fvNV(disp, dlsym(RTLD_DEFAULT, "glProgramParameters4fvNV"));
    SET_RequestResidentProgramsNV(disp, dlsym(RTLD_DEFAULT, "glRequestResidentProgramsNV"));
    SET_TrackMatrixNV(disp, dlsym(RTLD_DEFAULT, "glTrackMatrixNV"));
    SET_VertexAttrib1dNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1dNV"));
    SET_VertexAttrib1dvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1dvNV"));
    SET_VertexAttrib1fNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1fNV"));
    SET_VertexAttrib1fvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1fvNV"));
    SET_VertexAttrib1sNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1sNV"));
    SET_VertexAttrib1svNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib1svNV"));
    SET_VertexAttrib2dNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2dNV"));
    SET_VertexAttrib2dvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2dvNV"));
    SET_VertexAttrib2fNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2fNV"));
    SET_VertexAttrib2fvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2fvNV"));
    SET_VertexAttrib2sNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2sNV"));
    SET_VertexAttrib2svNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib2svNV"));
    SET_VertexAttrib3dNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3dNV"));
    SET_VertexAttrib3dvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3dvNV"));
    SET_VertexAttrib3fNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3fNV"));
    SET_VertexAttrib3fvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3fvNV"));
    SET_VertexAttrib3sNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3sNV"));
    SET_VertexAttrib3svNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib3svNV"));
    SET_VertexAttrib4dNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4dNV"));
    SET_VertexAttrib4dvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4dvNV"));
    SET_VertexAttrib4fNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4fNV"));
    SET_VertexAttrib4fvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4fvNV"));
    SET_VertexAttrib4sNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4sNV"));
    SET_VertexAttrib4svNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4svNV"));
    SET_VertexAttrib4ubNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4ubNV"));
    SET_VertexAttrib4ubvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttrib4ubvNV"));
    SET_VertexAttribPointerNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribPointerNV"));
    SET_VertexAttribs1dvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs1dvNV"));
    SET_VertexAttribs1fvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs1fvNV"));
    SET_VertexAttribs1svNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs1svNV"));
    SET_VertexAttribs2dvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs2dvNV"));
    SET_VertexAttribs2fvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs2fvNV"));
    SET_VertexAttribs2svNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs2svNV"));
    SET_VertexAttribs3dvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs3dvNV"));
    SET_VertexAttribs3fvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs3fvNV"));
    SET_VertexAttribs3svNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs3svNV"));
    SET_VertexAttribs4dvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs4dvNV"));
    SET_VertexAttribs4fvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs4fvNV"));
    SET_VertexAttribs4svNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs4svNV"));
    SET_VertexAttribs4ubvNV(disp, dlsym(RTLD_DEFAULT, "glVertexAttribs4ubvNV"));

    /* GL_SGIS_multisample */
    SET_SampleMaskSGIS(disp, dlsym(RTLD_DEFAULT, "glSampleMaskSGIS"));
    SET_SamplePatternSGIS(disp, dlsym(RTLD_DEFAULT, "glSamplePatternSGIS"));

    /* GL_SGIS_pixel_texture */
    SET_GetPixelTexGenParameterfvSGIS(disp, dlsym(RTLD_DEFAULT, "glGetPixelTexGenParameterfvSGIS"));
    SET_GetPixelTexGenParameterivSGIS(disp, dlsym(RTLD_DEFAULT, "glGetPixelTexGenParameterivSGIS"));
    SET_PixelTexGenParameterfSGIS(disp, dlsym(RTLD_DEFAULT, "glPixelTexGenParameterfSGIS"));
    SET_PixelTexGenParameterfvSGIS(disp, dlsym(RTLD_DEFAULT, "glPixelTexGenParameterfvSGIS"));
    SET_PixelTexGenParameteriSGIS(disp, dlsym(RTLD_DEFAULT, "glPixelTexGenParameteriSGIS"));
    SET_PixelTexGenParameterivSGIS(disp, dlsym(RTLD_DEFAULT, "glPixelTexGenParameterivSGIS"));
    SET_PixelTexGenSGIX(disp, dlsym(RTLD_DEFAULT, "glPixelTexGenSGIX"));

    _glapi_set_dispatch(disp);
}
