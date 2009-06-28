/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>

#include <glitz-egl.h>

#include "inputstr.h"
#include "cursorstr.h"
#include "mipointer.h"

#include "xegl.h"

#define XEGL_DEFAULT_SCREEN_WIDTH  800
#define XEGL_DEFAULT_SCREEN_HEIGHT 600

DevPrivateKey xeglScreenPrivateKey = &xeglScreenPrivateKey;

#define XEGL_GET_SCREEN_PRIV(pScreen) ((xeglScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, xeglScreenPrivateKey))

#define XEGL_SET_SCREEN_PRIV(pScreen, v) \
    dixSetPrivate(&(pScreen)->devPrivates, xeglScreenPrivateKey, v)

#define XEGL_SCREEN_PRIV(pScreen)			       \
    xeglScreenPtr pScreenPriv = XEGL_GET_SCREEN_PRIV (pScreen)

static EGLDisplay    eDisplay;
static EGLScreenMESA eScreen;
static ScreenPtr     currentScreen = 0;
static Bool	     softCursor = TRUE;

extern miPointerScreenFuncRec kdPointerScreenFuncs;

static Bool
xeglAllocatePrivates (ScreenPtr pScreen)
{
    xeglScreenPtr pScreenPriv;

    pScreenPriv = xalloc (sizeof (xeglScreenRec));
    if (!pScreenPriv)
	return FALSE;

    XEGL_SET_SCREEN_PRIV (pScreen, pScreenPriv);

    return TRUE;
}

static Bool
xeglCloseScreen (int	   index,
		 ScreenPtr pScreen)
{
    glitz_drawable_t *drawable;

    XEGL_SCREEN_PRIV (pScreen);

    drawable = XGL_GET_SCREEN_PRIV (pScreen)->drawable;
    if (drawable)
	glitz_drawable_destroy (drawable);

    xglClearVisualTypes ();

    XGL_SCREEN_UNWRAP (CloseScreen);
    xfree (pScreenPriv);

    return (*pScreen->CloseScreen) (index, pScreen);
}

static Bool
xeglScreenInit (int	  index,
		ScreenPtr pScreen,
		int	  argc,
		char	  **argv)
{
    EGLSurface		    eSurface;
    EGLModeMESA		    mode;
    int			    count;
    xeglScreenPtr	    pScreenPriv;
    glitz_drawable_format_t *format;
    glitz_drawable_t	    *drawable;
    EGLint screenAttribs[] = {
	EGL_WIDTH, 1024,
	EGL_HEIGHT, 768,
	EGL_NONE
    };

    if (xglScreenInfo.width == 0 || xglScreenInfo.height == 0)
    {
      xglScreenInfo.width = XEGL_DEFAULT_SCREEN_WIDTH;
      xglScreenInfo.height = XEGL_DEFAULT_SCREEN_HEIGHT;

    }
    
    screenAttribs[1] = xglScreenInfo.width;
    screenAttribs[3] = xglScreenInfo.height;

    format = xglVisuals[0].format;

    if (!xeglAllocatePrivates (pScreen))
	return FALSE;

    currentScreen = pScreen;

    pScreenPriv = XEGL_GET_SCREEN_PRIV (pScreen);

    if (xglScreenInfo.width == 0 || xglScreenInfo.height == 0)
    {
	xglScreenInfo.width  = XEGL_DEFAULT_SCREEN_WIDTH;
	xglScreenInfo.height = XEGL_DEFAULT_SCREEN_HEIGHT;
    }

    eglGetModesMESA (eDisplay, eScreen, &mode, 1, &count);

    eSurface = eglCreateScreenSurfaceMESA (eDisplay, format->id, screenAttribs);
    if (eSurface == EGL_NO_SURFACE)
    {
	ErrorF ("failed to create screen surface\n");
	return FALSE;
    }

    eglShowScreenSurfaceMESA (eDisplay, eScreen, eSurface, mode);

    drawable = glitz_egl_create_surface (eDisplay, eScreen, format, eSurface,
					 xglScreenInfo.width,
					 xglScreenInfo.height);
    if (!drawable)
    {
	ErrorF ("[%d] couldn't create glitz drawable for window\n", index);
	return FALSE;
    }

    xglScreenInfo.drawable = drawable;

    if (!xglScreenInit (pScreen))
	return FALSE;

#ifdef GLXEXT
    if (!xglInitVisualConfigs (pScreen))
	return FALSE;
#endif

    XGL_SCREEN_WRAP (CloseScreen, xeglCloseScreen);

    miDCInitialize (pScreen, &kdPointerScreenFuncs);
    miCreateDefColormap(pScreen);

    if (!xglFinishScreenInit (pScreen))
	return FALSE;

    return TRUE;
}

void
xeglInitOutput (ScreenInfo *pScreenInfo,
		int	   argc,
		char       **argv)
{
    glitz_drawable_format_t *format, templ;
    int			    i, maj, min, count;
    unsigned long	    mask;
    
    xglSetPixmapFormats (pScreenInfo);

    if (!eDisplay)
    {
	eDisplay = eglGetDisplay (":0");

	if (!eglInitialize (eDisplay, &maj, &min))
	    FatalError ("can't open display");

	eglGetScreensMESA (eDisplay, &eScreen, 1, &count);
    }

    templ.samples          = 1;
    templ.doublebuffer     = 1;
    templ.color.alpha_size = 8;

    mask = GLITZ_FORMAT_SAMPLES_MASK;

    format = glitz_egl_find_window_config (eDisplay, eScreen,
				    mask, &templ, 0);

    if (!format)
	FatalError ("no visual format found");

    xglSetVisualTypesAndMasks (pScreenInfo, format, (1 << TrueColor));

    xglInitVisuals (pScreenInfo);

    AddScreen (xeglScreenInit, argc, argv);
}

static void
xeglBlockHandler (pointer   blockData,
		  OSTimePtr pTimeout,
		  pointer   pReadMask)
{
    XGL_SCREEN_PRIV (currentScreen);

    if (!xglSyncSurface (&pScreenPriv->pScreenPixmap->drawable))
	FatalError (XGL_SW_FAILURE_STRING);

    glitz_surface_flush (pScreenPriv->surface);
    glitz_drawable_finish (pScreenPriv->drawable);
}

void
xeglInitInput (int  argc,
	       char **argv)
{
    eglInitInput (&LinuxEvdevMouseFuncs, &LinuxEvdevKeyboardFuncs);
    RegisterBlockAndWakeupHandlers (xeglBlockHandler, KdWakeupHandler, NULL);
}

Bool
xeglLegalModifier (unsigned int key,
		   DeviceIntPtr pDev)
{
    return KdLegalModifier (key, pDev);
}

void
xeglProcessInputEvents (void)
{
    KdProcessInputEvents ();
}

void
xeglUseMsg (void)
{
    ErrorF ("-screen WIDTH[/WIDTHMM]xHEIGHT[/HEIGHTMM] "
	    "specify screen characteristics\n");
    ErrorF ("-softcursor            force software cursor\n");
}

int
xeglProcessArgument (int  argc,
		     char **argv,
		     int  i)
{
    if (!strcmp (argv[i], "-screen"))
    {
	if ((i + 1) < argc)
	{
	    xglParseScreen (argv[i + 1]);
	}
	else
	    return 1;

	return 2;
    }
    else if (!strcmp (argv[i], "-softcursor"))
    {
	softCursor = TRUE;
	return 1;
    }

    return 0;
}

void
xeglAbort (void)
{
}

void
xeglGiveUp (void)
{
    AbortDDX ();
}

void
xeglOsVendorInit (void)
{
}
