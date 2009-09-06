/*
 * Copyright © 2008 George Sapountzis <gsap7@yahoo.gr>
 * Copyright © 2008 Red Hat, Inc
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <dlfcn.h>

#include <GL/gl.h>
#include <GL/internal/dri_interface.h>
#include <GL/glxtokens.h>

#include "scrnintstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "os.h"

#include "glxserver.h"
#include "glxutil.h"
#include "glxdricommon.h"

#include "g_disptab.h"
#include "glapitable.h"
#include "glapi.h"
#include "glthread.h"
#include "dispatch.h"
#include "extension_string.h"

/* RTLD_LOCAL is not defined on Cygwin */
#ifdef __CYGWIN__
#ifndef RTLD_LOCAL
#define RTLD_LOCAL 0
#endif
#endif

typedef struct __GLXDRIscreen   __GLXDRIscreen;
typedef struct __GLXDRIcontext  __GLXDRIcontext;
typedef struct __GLXDRIdrawable __GLXDRIdrawable;

struct __GLXDRIscreen {
    __GLXscreen		 base;
    __DRIscreen		*driScreen;
    void		*driver;

    const __DRIcoreExtension *core;
    const __DRIswrastExtension *swrast;
    const __DRIcopySubBufferExtension *copySubBuffer;
    const __DRItexBufferExtension *texBuffer;
};

struct __GLXDRIcontext {
    __GLXcontext	 base;
    __DRIcontext	*driContext;
};

struct __GLXDRIdrawable {
    __GLXdrawable	 base;
    __DRIdrawable	*driDrawable;
    __GLXDRIscreen	*screen;

    GCPtr gc;		/* scratch GC for span drawing */
    GCPtr swapgc;	/* GC for swapping the color buffers */
};

static void
__glXDRIdrawableDestroy(__GLXdrawable *drawable)
{
    __GLXDRIdrawable *private = (__GLXDRIdrawable *) drawable;
    const __DRIcoreExtension *core = private->screen->core;

    (*core->destroyDrawable)(private->driDrawable);

    FreeScratchGC(private->gc);
    FreeScratchGC(private->swapgc);

    __glXDrawableRelease(drawable);

    xfree(private);
}

static GLboolean
__glXDRIdrawableSwapBuffers(__GLXdrawable *drawable)
{
    __GLXDRIdrawable *private = (__GLXDRIdrawable *) drawable;
    const __DRIcoreExtension *core = private->screen->core;

    (*core->swapBuffers)(private->driDrawable);

    return TRUE;
}

static void
__glXDRIdrawableCopySubBuffer(__GLXdrawable *basePrivate,
			       int x, int y, int w, int h)
{
    __GLXDRIdrawable *private = (__GLXDRIdrawable *) basePrivate;
    const __DRIcopySubBufferExtension *copySubBuffer =
	    private->screen->copySubBuffer;

    if (copySubBuffer)
	(*copySubBuffer->copySubBuffer)(private->driDrawable, x, y, w, h);
}

static void
__glXDRIcontextDestroy(__GLXcontext *baseContext)
{
    __GLXDRIcontext *context = (__GLXDRIcontext *) baseContext;
    __GLXDRIscreen *screen = (__GLXDRIscreen *) context->base.pGlxScreen;

    (*screen->core->destroyContext)(context->driContext);
    __glXContextDestroy(&context->base);
    xfree(context);
}

static int
__glXDRIcontextMakeCurrent(__GLXcontext *baseContext)
{
    __GLXDRIcontext *context = (__GLXDRIcontext *) baseContext;
    __GLXDRIdrawable *draw = (__GLXDRIdrawable *) baseContext->drawPriv;
    __GLXDRIdrawable *read = (__GLXDRIdrawable *) baseContext->readPriv;
    __GLXDRIscreen *screen = (__GLXDRIscreen *) context->base.pGlxScreen;

    return (*screen->core->bindContext)(context->driContext,
					draw->driDrawable,
					read->driDrawable);
}

static int
__glXDRIcontextLoseCurrent(__GLXcontext *baseContext)
{
    __GLXDRIcontext *context = (__GLXDRIcontext *) baseContext;
    __GLXDRIscreen *screen = (__GLXDRIscreen *) context->base.pGlxScreen;

    return (*screen->core->unbindContext)(context->driContext);
}

static int
__glXDRIcontextCopy(__GLXcontext *baseDst, __GLXcontext *baseSrc,
		    unsigned long mask)
{
    __GLXDRIcontext *dst = (__GLXDRIcontext *) baseDst;
    __GLXDRIcontext *src = (__GLXDRIcontext *) baseSrc;
    __GLXDRIscreen *screen = (__GLXDRIscreen *) dst->base.pGlxScreen;

    return (*screen->core->copyContext)(dst->driContext,
					src->driContext, mask);
}

static int
__glXDRIcontextForceCurrent(__GLXcontext *baseContext)
{
    __GLXDRIcontext *context = (__GLXDRIcontext *) baseContext;
    __GLXDRIdrawable *draw = (__GLXDRIdrawable *) baseContext->drawPriv;
    __GLXDRIdrawable *read = (__GLXDRIdrawable *) baseContext->readPriv;
    __GLXDRIscreen *screen = (__GLXDRIscreen *) context->base.pGlxScreen;

    return (*screen->core->bindContext)(context->driContext,
					draw->driDrawable,
					read->driDrawable);
}

#ifdef __DRI_TEX_BUFFER

static int
__glXDRIbindTexImage(__GLXcontext *baseContext,
		     int buffer,
		     __GLXdrawable *glxPixmap)
{
    __GLXDRIdrawable *drawable = (__GLXDRIdrawable *) glxPixmap;
    const __DRItexBufferExtension *texBuffer = drawable->screen->texBuffer;
    __GLXDRIcontext *context = (__GLXDRIcontext *) baseContext;

    if (texBuffer == NULL)
        return Success;

    texBuffer->setTexBuffer(context->driContext,
			    glxPixmap->target,
			    drawable->driDrawable);

    return Success;
}

static int
__glXDRIreleaseTexImage(__GLXcontext *baseContext,
			int buffer,
			__GLXdrawable *pixmap)
{
    /* FIXME: Just unbind the texture? */
    return Success;
}

#else

static int
__glXDRIbindTexImage(__GLXcontext *baseContext,
		     int buffer,
		     __GLXdrawable *glxPixmap)
{
    return Success;
}

static int
__glXDRIreleaseTexImage(__GLXcontext *baseContext,
			int buffer,
			__GLXdrawable *pixmap)
{
    return Success;
}

#endif

static __GLXtextureFromPixmap __glXDRItextureFromPixmap = {
    __glXDRIbindTexImage,
    __glXDRIreleaseTexImage
};

static void
__glXDRIscreenDestroy(__GLXscreen *baseScreen)
{
    __GLXDRIscreen *screen = (__GLXDRIscreen *) baseScreen;

    (*screen->core->destroyScreen)(screen->driScreen);

    dlclose(screen->driver);

    __glXScreenDestroy(baseScreen);

    xfree(screen);
}

static __GLXcontext *
__glXDRIscreenCreateContext(__GLXscreen *baseScreen,
			    __GLXconfig *glxConfig,
			    __GLXcontext *baseShareContext)
{
    __GLXDRIscreen *screen = (__GLXDRIscreen *) baseScreen;
    __GLXDRIcontext *context, *shareContext;
    __GLXDRIconfig *config = (__GLXDRIconfig *) glxConfig;
    const __DRIcoreExtension *core = screen->core;
    __DRIcontext *driShare;

    shareContext = (__GLXDRIcontext *) baseShareContext;
    if (shareContext)
	driShare = shareContext->driContext;
    else
	driShare = NULL;

    context = xcalloc(1, sizeof *context);
    if (context == NULL)
	return NULL;

    context->base.destroy           = __glXDRIcontextDestroy;
    context->base.makeCurrent       = __glXDRIcontextMakeCurrent;
    context->base.loseCurrent       = __glXDRIcontextLoseCurrent;
    context->base.copy              = __glXDRIcontextCopy;
    context->base.forceCurrent      = __glXDRIcontextForceCurrent;
    context->base.textureFromPixmap = &__glXDRItextureFromPixmap;

    context->driContext =
	(*core->createNewContext)(screen->driScreen,
				  config->driConfig, driShare, context);

    return &context->base;
}

static void
glxChangeGC(GCPtr gc, BITS32 mask, CARD32 val)
{
    CARD32 v[1];
    v[0] = val;
    dixChangeGC(NullClient, gc, mask, v, NULL);
}

static __GLXdrawable *
__glXDRIscreenCreateDrawable(__GLXscreen *screen,
			     DrawablePtr pDraw,
			     int type,
			     XID drawId,
			     __GLXconfig *glxConfig)
{
    __GLXDRIscreen *driScreen = (__GLXDRIscreen *) screen;
    __GLXDRIconfig *config = (__GLXDRIconfig *) glxConfig;
    __GLXDRIdrawable *private;

    ScreenPtr pScreen = driScreen->base.pScreen;

    private = xcalloc(1, sizeof *private);
    if (private == NULL)
	return NULL;

    private->screen = driScreen;
    if (!__glXDrawableInit(&private->base, screen,
			   pDraw, type, drawId, glxConfig)) {
        xfree(private);
	return NULL;
    }

    private->base.destroy       = __glXDRIdrawableDestroy;
    private->base.swapBuffers   = __glXDRIdrawableSwapBuffers;
    private->base.copySubBuffer = __glXDRIdrawableCopySubBuffer;

    private->gc = CreateScratchGC(pScreen, pDraw->depth);
    private->swapgc = CreateScratchGC(pScreen, pDraw->depth);

    glxChangeGC(private->gc, GCFunction, GXcopy);
    glxChangeGC(private->swapgc, GCFunction, GXcopy);
    glxChangeGC(private->swapgc, GCGraphicsExposures, FALSE);

    private->driDrawable =
	(*driScreen->swrast->createNewDrawable)(driScreen->driScreen,
						config->driConfig,
						private);

    return &private->base;
}

static void
swrastGetDrawableInfo(__DRIdrawable *draw,
		      int *x, int *y, int *w, int *h,
		      void *loaderPrivate)
{
    __GLXDRIdrawable *drawable = loaderPrivate;
    DrawablePtr pDraw = drawable->base.pDraw;

    *x = pDraw->x;
    *y = pDraw->x;
    *w = pDraw->width;
    *h = pDraw->height;
}

static void
swrastPutImage(__DRIdrawable *draw, int op,
	     int x, int y, int w, int h, char *data,
	     void *loaderPrivate)
{
    __GLXDRIdrawable *drawable = loaderPrivate;
    DrawablePtr pDraw = drawable->base.pDraw;
    GCPtr gc;

    switch (op) {
    case __DRI_SWRAST_IMAGE_OP_DRAW:
	gc = drawable->gc;
	break;
    case __DRI_SWRAST_IMAGE_OP_SWAP:
	gc = drawable->swapgc;
	break;
    default:
	return;
    }

    ValidateGC(pDraw, gc);

    gc->ops->PutImage(pDraw, gc, pDraw->depth,
		      x, y, w, h, 0, ZPixmap, data);
}

static void
swrastGetImage(__DRIdrawable *draw,
	     int x, int y, int w, int h, char *data,
	     void *loaderPrivate)
{
    __GLXDRIdrawable *drawable = loaderPrivate;
    DrawablePtr pDraw = drawable->base.pDraw;
    ScreenPtr pScreen = pDraw->pScreen;

    pScreen->GetImage(pDraw, x, y, w, h, ZPixmap, ~0L, data);
}

static const __DRIswrastLoaderExtension swrastLoaderExtension = {
    { __DRI_SWRAST_LOADER, __DRI_SWRAST_LOADER_VERSION },
    swrastGetDrawableInfo,
    swrastPutImage,
    swrastGetImage
};

static const __DRIextension *loader_extensions[] = {
    &systemTimeExtension.base,
    &swrastLoaderExtension.base,
    NULL
};

static void
initializeExtensions(__GLXDRIscreen *screen)
{
    const __DRIextension **extensions;
    int i;

    extensions = screen->core->getExtensions(screen->driScreen);

    for (i = 0; extensions[i]; i++) {
#ifdef __DRI_COPY_SUB_BUFFER
	if (strcmp(extensions[i]->name, __DRI_COPY_SUB_BUFFER) == 0) {
	    screen->copySubBuffer =
		(const __DRIcopySubBufferExtension *) extensions[i];
	    /* GLX_MESA_copy_sub_buffer is always enabled. */
	}
#endif

#ifdef __DRI_TEX_BUFFER
	if (strcmp(extensions[i]->name, __DRI_TEX_BUFFER) == 0) {
	    screen->texBuffer =
		(const __DRItexBufferExtension *) extensions[i];
	    /* GLX_EXT_texture_from_pixmap is always enabled. */
	}
#endif
	/* Ignore unknown extensions */
    }
}

static const char dri_driver_path[] = DRI_DRIVER_PATH;

static __GLXscreen *
__glXDRIscreenProbe(ScreenPtr pScreen)
{
    const char *driverName = "swrast";
    __GLXDRIscreen *screen;
    char filename[128];
    const __DRIextension **extensions;
    const __DRIconfig **driConfigs;
    int i;

    screen = xcalloc(1, sizeof *screen);
    if (screen == NULL)
	return NULL;

    screen->base.destroy        = __glXDRIscreenDestroy;
    screen->base.createContext  = __glXDRIscreenCreateContext;
    screen->base.createDrawable = __glXDRIscreenCreateDrawable;
    screen->base.swapInterval   = NULL;
    screen->base.pScreen       = pScreen;

    snprintf(filename, sizeof filename,
	     "%s/%s_dri.so", dri_driver_path, driverName);

    screen->driver = dlopen(filename, RTLD_LAZY | RTLD_LOCAL);
    if (screen->driver == NULL) {
	LogMessage(X_ERROR, "AIGLX error: dlopen of %s failed (%s)\n",
		   filename, dlerror());
        goto handle_error;
    }

    extensions = dlsym(screen->driver, __DRI_DRIVER_EXTENSIONS);
    if (extensions == NULL) {
	LogMessage(X_ERROR, "AIGLX error: %s exports no extensions (%s)\n",
		   driverName, dlerror());
	goto handle_error;
    }

    for (i = 0; extensions[i]; i++) {
        if (strcmp(extensions[i]->name, __DRI_CORE) == 0 &&
	    extensions[i]->version >= __DRI_CORE_VERSION) {
		screen->core = (const __DRIcoreExtension *) extensions[i];
	}
        if (strcmp(extensions[i]->name, __DRI_SWRAST) == 0 &&
	    extensions[i]->version >= __DRI_SWRAST_VERSION) {
		screen->swrast = (const __DRIswrastExtension *) extensions[i];
	}
    }

    if (screen->core == NULL || screen->swrast == NULL) {
	LogMessage(X_ERROR, "AIGLX error: %s exports no DRI extension\n",
		   driverName);
	goto handle_error;
    }

    screen->driScreen =
	(*screen->swrast->createNewScreen)(pScreen->myNum,
					   loader_extensions,
					   &driConfigs,
					   screen);

    if (screen->driScreen == NULL) {
	LogMessage(X_ERROR,
		   "AIGLX error: Calling driver entry point failed\n");
	goto handle_error;
    }

    initializeExtensions(screen);

    screen->base.fbconfigs = glxConvertConfigs(screen->core, driConfigs);

    __glXScreenInit(&screen->base, pScreen);

    LogMessage(X_INFO,
	       "AIGLX: Loaded and initialized %s\n", filename);

    return &screen->base;

 handle_error:
    if (screen->driver)
        dlclose(screen->driver);

    xfree(screen);

    LogMessage(X_ERROR, "GLX: could not load software renderer\n");

    return NULL;
}

_X_EXPORT __GLXprovider __glXDRISWRastProvider = {
    __glXDRIscreenProbe,
    "DRISWRAST",
    NULL
};
