/*
 * Copyright © 2007 Red Hat, Inc
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Red Hat,
 * Inc not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Red Hat, Inc makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL RED HAT, INC BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <drm.h>
#include <GL/gl.h>
#include <GL/internal/dri_interface.h>
#include <GL/glxtokens.h>

#include <windowstr.h>
#include <os.h>

#define _XF86DRI_SERVER_
#include <xf86drm.h>
#include <xf86.h>
#include <dri2.h>

#include "glxserver.h"
#include "glxutil.h"
#include "glxdricommon.h"

#include "g_disptab.h"
#include "glapitable.h"
#include "glapi.h"
#include "glthread.h"
#include "dispatch.h"
#include "extension_string.h"

typedef struct __GLXDRIscreen   __GLXDRIscreen;
typedef struct __GLXDRIcontext  __GLXDRIcontext;
typedef struct __GLXDRIdrawable __GLXDRIdrawable;

struct __GLXDRIscreen {
    __GLXscreen		 base;
    __DRIscreen		*driScreen;
    void		*driver;
    int			 fd;

    xf86EnterVTProc	*enterVT;
    xf86LeaveVTProc	*leaveVT;

    const __DRIcoreExtension *core;
    const __DRIdri2Extension *dri2;
    const __DRIcopySubBufferExtension *copySubBuffer;
    const __DRIswapControlExtension *swapControl;
    const __DRItexBufferExtension *texBuffer;

    unsigned char glx_enable_bits[__GLX_EXT_BYTES];
};

struct __GLXDRIcontext {
    __GLXcontext	 base;
    __DRIcontext	*driContext;
};

#define MAX_DRAWABLE_BUFFERS 5

struct __GLXDRIdrawable {
    __GLXdrawable	 base;
    __DRIdrawable	*driDrawable;
    __GLXDRIscreen	*screen;

    /* Dimensions as last reported by DRI2GetBuffers. */
    int width;
    int height;
    __DRIbuffer buffers[MAX_DRAWABLE_BUFFERS];
    int count;
};

static void
__glXDRIdrawableDestroy(__GLXdrawable *drawable)
{
    __GLXDRIdrawable *private = (__GLXDRIdrawable *) drawable;
    const __DRIcoreExtension *core = private->screen->core;
    
    (*core->destroyDrawable)(private->driDrawable);

    /* If the X window was destroyed, the dri DestroyWindow hook will
     * aready have taken care of this, so only call if pDraw isn't NULL. */
    if (drawable->pDraw != NULL)
	DRI2DestroyDrawable(drawable->pDraw);

    __glXDrawableRelease(drawable);

    xfree(private);
}

static void
__glXDRIdrawableCopySubBuffer(__GLXdrawable *drawable,
			       int x, int y, int w, int h)
{
    __GLXDRIdrawable *private = (__GLXDRIdrawable *) drawable;
    BoxRec box;
    RegionRec region;

    box.x1 = x;
    box.y1 = private->height - y - h;
    box.x2 = x + w;
    box.y2 = private->height - y;
    REGION_INIT(drawable->pDraw->pScreen, &region, &box, 0);

    DRI2CopyRegion(drawable->pDraw, &region,
		   DRI2BufferFrontLeft, DRI2BufferBackLeft);
}

static GLboolean
__glXDRIdrawableSwapBuffers(__GLXdrawable *drawable)
{
    __GLXDRIdrawable *private = (__GLXDRIdrawable *) drawable;

    __glXDRIdrawableCopySubBuffer(drawable, 0, 0,
				  private->width, private->height);

    return TRUE;
}

static void
__glXDRIdrawableWaitX(__GLXdrawable *drawable)
{
    __GLXDRIdrawable *private = (__GLXDRIdrawable *) drawable;
    BoxRec box;
    RegionRec region;

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = private->width;
    box.y2 = private->height;
    REGION_INIT(drawable->pDraw->pScreen, &region, &box, 0);

    DRI2CopyRegion(drawable->pDraw, &region,
		   DRI2BufferFakeFrontLeft, DRI2BufferFrontLeft);
}

static void
__glXDRIdrawableWaitGL(__GLXdrawable *drawable)
{
    __GLXDRIdrawable *private = (__GLXDRIdrawable *) drawable;
    BoxRec box;
    RegionRec region;

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = private->width;
    box.y2 = private->height;
    REGION_INIT(drawable->pDraw->pScreen, &region, &box, 0);

    DRI2CopyRegion(drawable->pDraw, &region,
		   DRI2BufferFrontLeft, DRI2BufferFakeFrontLeft);
}

static int
__glXDRIdrawableSwapInterval(__GLXdrawable *drawable, int interval)
{
    return 0;
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

#if __DRI_TEX_BUFFER_VERSION >= 2
    if (texBuffer->base.version >= 2 && texBuffer->setTexBuffer2 != NULL) {
	(*texBuffer->setTexBuffer2)(context->driContext,
				    glxPixmap->target,
				    glxPixmap->format,
				    drawable->driDrawable);
    } else
#endif
    {
	texBuffer->setTexBuffer(context->driContext,
				glxPixmap->target,
				drawable->driDrawable);
    }

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
	(*screen->dri2->createNewContext)(screen->driScreen,
					  config->driConfig,
					  driShare, context);
    if (context->driContext == NULL) {
	    xfree(context);
        return NULL;
    }

    return &context->base;
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
    private->base.waitGL	= __glXDRIdrawableWaitGL;
    private->base.waitX		= __glXDRIdrawableWaitX;

    if (DRI2CreateDrawable(pDraw)) {
	    xfree(private);
	    return NULL;
    }

    private->driDrawable =
	(*driScreen->dri2->createNewDrawable)(driScreen->driScreen,
					      config->driConfig, private);

    return &private->base;
}

static __DRIbuffer *
dri2GetBuffers(__DRIdrawable *driDrawable,
	       int *width, int *height,
	       unsigned int *attachments, int count,
	       int *out_count, void *loaderPrivate)
{
    __GLXDRIdrawable *private = loaderPrivate;
    DRI2BufferPtr *buffers;
    int i;
    int j;

    buffers = DRI2GetBuffers(private->base.pDraw,
			     width, height, attachments, count, out_count);
    if (*out_count > MAX_DRAWABLE_BUFFERS) {
	*out_count = 0;
	return NULL;
    }
	
    private->width = *width;
    private->height = *height;

    /* This assumes the DRI2 buffer attachment tokens matches the
     * __DRIbuffer tokens. */
    j = 0;
    for (i = 0; i < *out_count; i++) {
	/* Do not send the real front buffer of a window to the client.
	 */
	if ((private->base.pDraw->type == DRAWABLE_WINDOW)
	    && (buffers[i]->attachment == DRI2BufferFrontLeft)) {
	    continue;
	}

	private->buffers[j].attachment = buffers[i]->attachment;
	private->buffers[j].name = buffers[i]->name;
	private->buffers[j].pitch = buffers[i]->pitch;
	private->buffers[j].cpp = buffers[i]->cpp;
	private->buffers[j].flags = buffers[i]->flags;
	j++;
    }

    *out_count = j;
    return private->buffers;
}

static __DRIbuffer *
dri2GetBuffersWithFormat(__DRIdrawable *driDrawable,
			 int *width, int *height,
			 unsigned int *attachments, int count,
			 int *out_count, void *loaderPrivate)
{
    __GLXDRIdrawable *private = loaderPrivate;
    DRI2BufferPtr *buffers;
    int i;
    int j = 0;

    buffers = DRI2GetBuffersWithFormat(private->base.pDraw,
				       width, height, attachments, count,
				       out_count);
    if (*out_count > MAX_DRAWABLE_BUFFERS) {
	*out_count = 0;
	return NULL;
    }

    private->width = *width;
    private->height = *height;

    /* This assumes the DRI2 buffer attachment tokens matches the
     * __DRIbuffer tokens. */
    for (i = 0; i < *out_count; i++) {
	/* Do not send the real front buffer of a window to the client.
	 */
	if ((private->base.pDraw->type == DRAWABLE_WINDOW)
	    && (buffers[i]->attachment == DRI2BufferFrontLeft)) {
	    continue;
	}

	private->buffers[j].attachment = buffers[i]->attachment;
	private->buffers[j].name = buffers[i]->name;
	private->buffers[j].pitch = buffers[i]->pitch;
	private->buffers[j].cpp = buffers[i]->cpp;
	private->buffers[j].flags = buffers[i]->flags;
	j++;
    }

    *out_count = j;
    return private->buffers;
}

static void 
dri2FlushFrontBuffer(__DRIdrawable *driDrawable, void *loaderPrivate)
{
    (void) driDrawable;
    __glXDRIdrawableWaitGL((__GLXdrawable *) loaderPrivate);
}

static const __DRIdri2LoaderExtension loaderExtension = {
    { __DRI_DRI2_LOADER, __DRI_DRI2_LOADER_VERSION },
    dri2GetBuffers,
    dri2FlushFrontBuffer,
    dri2GetBuffersWithFormat,
};

static const __DRIextension *loader_extensions[] = {
    &systemTimeExtension.base,
    &loaderExtension.base,
    NULL
};

static const char dri_driver_path[] = DRI_DRIVER_PATH;

static Bool
glxDRIEnterVT (int index, int flags)
{
    __GLXDRIscreen *screen = (__GLXDRIscreen *) 
	glxGetScreen(screenInfo.screens[index]);

    LogMessage(X_INFO, "AIGLX: Resuming AIGLX clients after VT switch\n");

    if (!(*screen->enterVT) (index, flags))
	return FALSE;
    
    glxResumeClients();

    return TRUE;
}

static void
glxDRILeaveVT (int index, int flags)
{
    __GLXDRIscreen *screen = (__GLXDRIscreen *)
	glxGetScreen(screenInfo.screens[index]);

    LogMessage(X_INFO, "AIGLX: Suspending AIGLX clients for VT switch\n");

    glxSuspendClients();

    return (*screen->leaveVT) (index, flags);
}

static void
initializeExtensions(__GLXDRIscreen *screen)
{
    const __DRIextension **extensions;
    int i;

    extensions = screen->core->getExtensions(screen->driScreen);

    __glXEnableExtension(screen->glx_enable_bits,
			 "GLX_MESA_copy_sub_buffer");
    LogMessage(X_INFO, "AIGLX: enabled GLX_MESA_copy_sub_buffer\n");

    for (i = 0; extensions[i]; i++) {
#ifdef __DRI_READ_DRAWABLE
	if (strcmp(extensions[i]->name, __DRI_READ_DRAWABLE) == 0) {
	    __glXEnableExtension(screen->glx_enable_bits,
				 "GLX_SGI_make_current_read");

	    LogMessage(X_INFO, "AIGLX: enabled GLX_SGI_make_current_read\n");
	}
#endif

#ifdef __DRI_SWAP_CONTROL
	if (strcmp(extensions[i]->name, __DRI_SWAP_CONTROL) == 0) {
	    screen->swapControl =
		(const __DRIswapControlExtension *) extensions[i];
	    __glXEnableExtension(screen->glx_enable_bits,
				 "GLX_SGI_swap_control");
	    __glXEnableExtension(screen->glx_enable_bits,
				 "GLX_MESA_swap_control");
	    
	    LogMessage(X_INFO, "AIGLX: enabled GLX_SGI_swap_control and GLX_MESA_swap_control\n");
	}
#endif

#ifdef __DRI_TEX_BUFFER
	if (strcmp(extensions[i]->name, __DRI_TEX_BUFFER) == 0) {
	    screen->texBuffer =
		(const __DRItexBufferExtension *) extensions[i];
	    /* GLX_EXT_texture_from_pixmap is always enabled. */
	    LogMessage(X_INFO, "AIGLX: GLX_EXT_texture_from_pixmap backed by buffer objects\n");
	}
#endif
	/* Ignore unknown extensions */
    }
}

static __GLXscreen *
__glXDRIscreenProbe(ScreenPtr pScreen)
{
    const char *driverName, *deviceName;
    __GLXDRIscreen *screen;
    char filename[128];
    size_t buffer_size;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    const __DRIextension **extensions;
    const __DRIconfig **driConfigs;
    int i;

    screen = xcalloc(1, sizeof *screen);
    if (screen == NULL)
	return NULL;

    if (!xf86LoaderCheckSymbol("DRI2Connect") ||
	!DRI2Connect(pScreen, DRI2DriverDRI,
		     &screen->fd, &driverName, &deviceName)) {
	LogMessage(X_INFO,
		   "AIGLX: Screen %d is not DRI2 capable\n", pScreen->myNum);
	return NULL;
    }

    screen->base.destroy        = __glXDRIscreenDestroy;
    screen->base.createContext  = __glXDRIscreenCreateContext;
    screen->base.createDrawable = __glXDRIscreenCreateDrawable;
    screen->base.swapInterval   = __glXDRIdrawableSwapInterval;
    screen->base.pScreen       = pScreen;

    __glXInitExtensionEnableBits(screen->glx_enable_bits);

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
        if (strcmp(extensions[i]->name, __DRI_DRI2) == 0 &&
	    extensions[i]->version >= __DRI_DRI2_VERSION) {
		screen->dri2 = (const __DRIdri2Extension *) extensions[i];
	}
    }

    if (screen->core == NULL || screen->dri2 == NULL) {
	LogMessage(X_ERROR, "AIGLX error: %s exports no DRI extension\n",
		   driverName);
	goto handle_error;
    }

    screen->driScreen =
	(*screen->dri2->createNewScreen)(pScreen->myNum,
					 screen->fd,
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

    /* The first call simply determines the length of the extension string.
     * This allows us to allocate some memory to hold the extension string,
     * but it requires that we call __glXGetExtensionString a second time.
     */
    buffer_size = __glXGetExtensionString(screen->glx_enable_bits, NULL);
    if (buffer_size > 0) {
	if (screen->base.GLXextensions != NULL) {
	    xfree(screen->base.GLXextensions);
	}

	screen->base.GLXextensions = xnfalloc(buffer_size);
	(void) __glXGetExtensionString(screen->glx_enable_bits, 
				       screen->base.GLXextensions);
    }

    screen->enterVT = pScrn->EnterVT;
    pScrn->EnterVT = glxDRIEnterVT; 
    screen->leaveVT = pScrn->LeaveVT;
    pScrn->LeaveVT = glxDRILeaveVT;

    LogMessage(X_INFO,
	       "AIGLX: Loaded and initialized %s\n", filename);

    return &screen->base;

 handle_error:
    if (screen->driver)
        dlclose(screen->driver);

    xfree(screen);

    LogMessage(X_ERROR, "AIGLX: reverting to software rendering\n");

    return NULL;
}

_X_EXPORT __GLXprovider __glXDRI2Provider = {
    __glXDRIscreenProbe,
    "DRI2",
    NULL
};
