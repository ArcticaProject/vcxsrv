/*
 * Copyright 2001-2004 Red Hat Inc., Durham, North Carolina.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL RED HAT AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <kem@redhat.com>
 *   David H. Dawes <dawes@xfree86.org>
 *
 */

/** \file
 * This file provides support for screen initialization. */

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#include "dmx.h"
#include "dmxsync.h"
#include "dmxshadow.h"
#include "dmxscrinit.h"
#include "dmxcursor.h"
#include "dmxgc.h"
#include "dmxgcops.h"
#include "dmxwindow.h"
#include "dmxpixmap.h"
#include "dmxfont.h"
#include "dmxcmap.h"
#include "dmxprop.h"
#include "dmxdpms.h"

#ifdef RENDER
#include "dmxpict.h"
#endif

#include "fb.h"
#include "mipointer.h"
#include "micmap.h"

extern Bool dmxCloseScreen(int idx, ScreenPtr pScreen);
static Bool dmxSaveScreen(ScreenPtr pScreen, int what);

static unsigned long dmxGeneration;
static unsigned long *dmxCursorGeneration;

static int dmxGCPrivateKeyIndex;
DevPrivateKey dmxGCPrivateKey = &dmxGCPrivateKeyIndex; /**< Private index for GCs       */
static int dmxWinPrivateKeyIndex;
DevPrivateKey dmxWinPrivateKey = &dmxWinPrivateKeyIndex; /**< Private index for Windows   */
static int dmxPixPrivateKeyIndex;
DevPrivateKey dmxPixPrivateKey = &dmxPixPrivateKeyIndex; /**< Private index for Pixmaps   */
int dmxFontPrivateIndex;        /**< Private index for Fonts     */
static int dmxScreenPrivateKeyIndex;
DevPrivateKey dmxScreenPrivateKey = &dmxScreenPrivateKeyIndex; /**< Private index for Screens   */
static int dmxColormapPrivateKeyIndex;
DevPrivateKey dmxColormapPrivateKey = &dmxColormapPrivateKeyIndex; /**< Private index for Colormaps */
#ifdef RENDER
static int dmxPictPrivateKeyIndex;
DevPrivateKey dmxPictPrivateKey = &dmxPictPrivateKeyIndex; /**< Private index for Picts     */
static int dmxGlyphSetPrivateKeyIndex;
DevPrivateKey dmxGlyphSetPrivateKey = &dmxGlyphSetPrivateKeyIndex; /**< Private index for GlyphSets */
#endif

/** Initialize the parts of screen \a idx that require access to the
 *  back-end server. */
void dmxBEScreenInit(int idx, ScreenPtr pScreen)
{
    DMXScreenInfo        *dmxScreen = &dmxScreens[idx];
    XSetWindowAttributes  attribs;
    XGCValues             gcvals;
    unsigned long         mask;
    int                   i, j;

    /* FIXME: The dmxScreenInit() code currently assumes that it will
     * not be called if the Xdmx server is started with this screen
     * detached -- i.e., it assumes that dmxScreen->beDisplay is always
     * valid.  This is not necessarily a valid assumption when full
     * addition/removal of screens is implemented, but when this code is
     * broken out for screen reattachment, then we will reevaluate this
     * assumption.
     */

    pScreen->mmWidth = DisplayWidthMM(dmxScreen->beDisplay, 
				      DefaultScreen(dmxScreen->beDisplay));
    pScreen->mmHeight = DisplayHeightMM(dmxScreen->beDisplay, 
					DefaultScreen(dmxScreen->beDisplay));

    pScreen->whitePixel = dmxScreen->beWhitePixel;
    pScreen->blackPixel = dmxScreen->beBlackPixel;

    /* Handle screen savers and DPMS on the backend */
    dmxDPMSInit(dmxScreen);

    /* Create root window for screen */
    mask = CWBackPixel | CWEventMask | CWColormap | CWOverrideRedirect;
    attribs.background_pixel = dmxScreen->beBlackPixel;
    attribs.event_mask = (KeyPressMask
                          | KeyReleaseMask
                          | ButtonPressMask
                          | ButtonReleaseMask
                          | EnterWindowMask
                          | LeaveWindowMask
                          | PointerMotionMask
                          | KeymapStateMask
                          | FocusChangeMask);
    attribs.colormap = dmxScreen->beDefColormaps[dmxScreen->beDefVisualIndex];
    attribs.override_redirect = True;
    
    dmxScreen->scrnWin =
	XCreateWindow(dmxScreen->beDisplay, 
		      DefaultRootWindow(dmxScreen->beDisplay),
		      dmxScreen->scrnX,
		      dmxScreen->scrnY,
		      dmxScreen->scrnWidth,
		      dmxScreen->scrnHeight,
		      0,
		      pScreen->rootDepth,
		      InputOutput,
		      dmxScreen->beVisuals[dmxScreen->beDefVisualIndex].visual,
		      mask,
		      &attribs);
    dmxPropertyWindow(dmxScreen);

    /*
     * This turns off the cursor by defining a cursor with no visible
     * components.
     */
    {
	char noCursorData[] = {0, 0, 0, 0,
			       0, 0, 0, 0};
	Pixmap pixmap;
	XColor color, tmp;

	pixmap = XCreateBitmapFromData(dmxScreen->beDisplay, dmxScreen->scrnWin,
				       noCursorData, 8, 8);
	XAllocNamedColor(dmxScreen->beDisplay, dmxScreen->beDefColormaps[0],
			 "black", &color, &tmp);
	dmxScreen->noCursor = XCreatePixmapCursor(dmxScreen->beDisplay,
						  pixmap, pixmap,
						  &color, &color, 0, 0);
	XDefineCursor(dmxScreen->beDisplay, dmxScreen->scrnWin,
		      dmxScreen->noCursor);

	XFreePixmap(dmxScreen->beDisplay, pixmap);
    }

    XMapWindow(dmxScreen->beDisplay, dmxScreen->scrnWin);

    if (dmxShadowFB) {
	mask = (GCFunction
		| GCPlaneMask
		| GCClipMask);
	gcvals.function = GXcopy;
	gcvals.plane_mask = AllPlanes;
	gcvals.clip_mask = None;

	dmxScreen->shadowGC = XCreateGC(dmxScreen->beDisplay,
					dmxScreen->scrnWin,
					mask, &gcvals);

	dmxScreen->shadowFBImage =
	    XCreateImage(dmxScreen->beDisplay,
			 dmxScreen->beVisuals[dmxScreen->beDefVisualIndex].visual,
			 dmxScreen->beDepth,
			 ZPixmap,
			 0,
			 (char *)dmxScreen->shadow,
			 dmxScreen->scrnWidth, dmxScreen->scrnHeight,
			 dmxScreen->beBPP,
			 PixmapBytePad(dmxScreen->scrnWidth,
				       dmxScreen->beBPP));
    } else {
	/* Create default drawables (used during GC creation) */
	for (i = 0; i < dmxScreen->beNumPixmapFormats; i++) 
	    for (j = 0; j < dmxScreen->beNumDepths; j++)
		if ((dmxScreen->bePixmapFormats[i].depth == 1) ||
		    (dmxScreen->bePixmapFormats[i].depth ==
		     dmxScreen->beDepths[j])) {
		    dmxScreen->scrnDefDrawables[i] = (Drawable)
			XCreatePixmap(dmxScreen->beDisplay, dmxScreen->scrnWin,
				      1, 1, dmxScreen->bePixmapFormats[i].depth);
		    break;
		}
    }
}

/** Initialize screen number \a idx. */
Bool dmxScreenInit(int idx, ScreenPtr pScreen, int argc, char *argv[])
{
    DMXScreenInfo        *dmxScreen = &dmxScreens[idx];
    int                   i, j;

    if (dmxGeneration != serverGeneration) {
	/* Allocate font private index */
	dmxFontPrivateIndex = AllocateFontPrivateIndex();
	if (dmxFontPrivateIndex == -1)
	    return FALSE;

	dmxGeneration = serverGeneration;
    }

    if (dmxShadowFB) {
	dmxScreen->shadow = shadowAlloc(dmxScreen->scrnWidth,
					dmxScreen->scrnHeight,
					dmxScreen->beBPP);
    } else {
	if (!dmxInitGC(pScreen)) return FALSE;
	if (!dmxInitWindow(pScreen)) return FALSE;
	if (!dmxInitPixmap(pScreen)) return FALSE;
    }

    /*
     * Initalise the visual types.  miSetVisualTypesAndMasks() requires
     * that all of the types for each depth be collected together.  It's
     * intended for slightly different usage to what we would like here.
     * Maybe a miAddVisualTypeAndMask() function will be added to make
     * things easier here.
     */
    for (i = 0; i < dmxScreen->beNumDepths; i++) {
	int    depth;
	int    visuals        = 0;
	int    bitsPerRgb     = 0;
	int    preferredClass = -1;
	Pixel  redMask        = 0;
	Pixel  greenMask      = 0;
	Pixel  blueMask       = 0;

	depth = dmxScreen->beDepths[i];
	for (j = 0; j < dmxScreen->beNumVisuals; j++) {
	    XVisualInfo *vi;

	    vi = &dmxScreen->beVisuals[j];
	    if (vi->depth == depth) {
		/* Assume the masks are all the same. */
		visuals |= (1 << vi->class);
		bitsPerRgb = vi->bits_per_rgb;
		redMask = vi->red_mask;
		greenMask = vi->green_mask;
		blueMask = vi->blue_mask;
		if (j == dmxScreen->beDefVisualIndex) {
		    preferredClass = vi->class;
		}
	    }
	}
	miSetVisualTypesAndMasks(depth, visuals, bitsPerRgb, preferredClass,
				 redMask, greenMask, blueMask);
    }

    fbScreenInit(pScreen,
		 dmxShadowFB ? dmxScreen->shadow : NULL,
		 dmxScreen->scrnWidth,
		 dmxScreen->scrnHeight,
		 dmxScreen->beXDPI,
		 dmxScreen->beXDPI,
		 dmxScreen->scrnWidth,
		 dmxScreen->beBPP);
#ifdef RENDER
    (void)dmxPictureInit(pScreen, 0, 0);
#endif

    if (dmxShadowFB && !shadowInit(pScreen, dmxShadowUpdateProc, NULL))
	return FALSE;

    miInitializeBackingStore(pScreen);

    if (dmxShadowFB) {
	miDCInitialize(pScreen, &dmxPointerCursorFuncs);
    } else {
        MAXSCREENSALLOC(dmxCursorGeneration);
	if (dmxCursorGeneration[idx] != serverGeneration) {
	    if (!(miPointerInitialize(pScreen,
				      &dmxPointerSpriteFuncs,
				      &dmxPointerCursorFuncs,
				      FALSE)))
		return FALSE;

	    dmxCursorGeneration[idx] = serverGeneration;
	}
    }

    DMX_WRAP(CloseScreen, dmxCloseScreen, dmxScreen, pScreen);
    DMX_WRAP(SaveScreen, dmxSaveScreen, dmxScreen, pScreen);

    dmxBEScreenInit(idx, pScreen);

    if (!dmxShadowFB) {
	/* Wrap GC functions */
	DMX_WRAP(CreateGC, dmxCreateGC, dmxScreen, pScreen);

	/* Wrap Window functions */
	DMX_WRAP(CreateWindow, dmxCreateWindow, dmxScreen, pScreen);
	DMX_WRAP(DestroyWindow, dmxDestroyWindow, dmxScreen, pScreen);
	DMX_WRAP(PositionWindow, dmxPositionWindow, dmxScreen, pScreen);
	DMX_WRAP(ChangeWindowAttributes, dmxChangeWindowAttributes, dmxScreen,
		 pScreen);
	DMX_WRAP(RealizeWindow, dmxRealizeWindow, dmxScreen, pScreen);
	DMX_WRAP(UnrealizeWindow, dmxUnrealizeWindow, dmxScreen, pScreen);
	DMX_WRAP(RestackWindow, dmxRestackWindow, dmxScreen, pScreen);
	DMX_WRAP(WindowExposures, dmxWindowExposures, dmxScreen, pScreen);
	DMX_WRAP(CopyWindow, dmxCopyWindow, dmxScreen, pScreen);

	DMX_WRAP(ResizeWindow, dmxResizeWindow, dmxScreen, pScreen);
	DMX_WRAP(ReparentWindow, dmxReparentWindow, dmxScreen, pScreen);

	DMX_WRAP(ChangeBorderWidth, dmxChangeBorderWidth, dmxScreen, pScreen);

	/* Wrap Image functions */
	DMX_WRAP(GetImage, dmxGetImage, dmxScreen, pScreen);
	DMX_WRAP(GetSpans, dmxGetSpans, dmxScreen, pScreen);

	/* Wrap Pixmap functions */
	DMX_WRAP(CreatePixmap, dmxCreatePixmap, dmxScreen, pScreen);
	DMX_WRAP(DestroyPixmap, dmxDestroyPixmap, dmxScreen, pScreen);
	DMX_WRAP(BitmapToRegion, dmxBitmapToRegion, dmxScreen, pScreen);

	/* Wrap Font functions */
	DMX_WRAP(RealizeFont, dmxRealizeFont, dmxScreen, pScreen);
	DMX_WRAP(UnrealizeFont, dmxUnrealizeFont, dmxScreen, pScreen);

	/* Wrap Colormap functions */
	DMX_WRAP(CreateColormap, dmxCreateColormap, dmxScreen, pScreen);
	DMX_WRAP(DestroyColormap, dmxDestroyColormap, dmxScreen, pScreen);
	DMX_WRAP(InstallColormap, dmxInstallColormap, dmxScreen, pScreen);
	DMX_WRAP(StoreColors, dmxStoreColors, dmxScreen, pScreen);

	/* Wrap Shape functions */
	DMX_WRAP(SetShape, dmxSetShape, dmxScreen, pScreen);
    }

    if (!dmxCreateDefColormap(pScreen))
	return FALSE;

    return TRUE;
}

/** Close the \a pScreen resources on the back-end server. */
void dmxBECloseScreen(ScreenPtr pScreen)
{
    DMXScreenInfo *dmxScreen = &dmxScreens[pScreen->myNum];
    int            i;

    /* Restore the back-end screen-saver and DPMS state. */
    dmxDPMSTerm(dmxScreen);

    /* Free the screen resources */

    XFreeCursor(dmxScreen->beDisplay, dmxScreen->noCursor);
    dmxScreen->noCursor = (Cursor)0;

    XUnmapWindow(dmxScreen->beDisplay, dmxScreen->scrnWin);
    XDestroyWindow(dmxScreen->beDisplay, dmxScreen->scrnWin);
    dmxScreen->scrnWin = (Window)0;

    if (dmxShadowFB) {
	/* Free the shadow GC and image assocated with the back-end server */
	XFreeGC(dmxScreen->beDisplay, dmxScreen->shadowGC);
	dmxScreen->shadowGC = NULL;
	XFree(dmxScreen->shadowFBImage);
	dmxScreen->shadowFBImage = NULL;
    } else {
	/* Free the default drawables */
	for (i = 0; i < dmxScreen->beNumPixmapFormats; i++) {
	    XFreePixmap(dmxScreen->beDisplay, dmxScreen->scrnDefDrawables[i]);
	    dmxScreen->scrnDefDrawables[i] = (Drawable)0;
	}
    }

    /* Free resources allocated during initialization (in dmxinit.c) */
    for (i = 0; i < dmxScreen->beNumDefColormaps; i++)
	XFreeColormap(dmxScreen->beDisplay, dmxScreen->beDefColormaps[i]);
    xfree(dmxScreen->beDefColormaps);
    dmxScreen->beDefColormaps = NULL;

#if 0
    /* Do not free visuals, depths and pixmap formats here.  Free them
     * in dmxCloseScreen() instead -- see comment below. */
    XFree(dmxScreen->beVisuals);
    dmxScreen->beVisuals = NULL;

    XFree(dmxScreen->beDepths);
    dmxScreen->beDepths = NULL;

    XFree(dmxScreen->bePixmapFormats);
    dmxScreen->bePixmapFormats = NULL;
#endif

#ifdef GLXEXT
    if (dmxScreen->glxVisuals) {
	XFree(dmxScreen->glxVisuals);
	dmxScreen->glxVisuals = NULL;
	dmxScreen->numGlxVisuals = 0;
    }
#endif

    /* Close display */
    XCloseDisplay(dmxScreen->beDisplay);
    dmxScreen->beDisplay = NULL;
}

/** Close screen number \a idx. */
Bool dmxCloseScreen(int idx, ScreenPtr pScreen)
{
    DMXScreenInfo *dmxScreen = &dmxScreens[idx];

    /* Reset the proc vectors */
    if (idx == 0) {
#ifdef RENDER
	dmxResetRender();
#endif
	dmxResetFonts();
    }

    if (dmxShadowFB) {
	/* Free the shadow framebuffer */
	xfree(dmxScreen->shadow);
    } else {

	/* Unwrap Shape functions */
	DMX_UNWRAP(SetShape, dmxScreen, pScreen);

	/* Unwrap the pScreen functions */
	DMX_UNWRAP(CreateGC, dmxScreen, pScreen);

	DMX_UNWRAP(CreateWindow, dmxScreen, pScreen);
	DMX_UNWRAP(DestroyWindow, dmxScreen, pScreen);
	DMX_UNWRAP(PositionWindow, dmxScreen, pScreen);
	DMX_UNWRAP(ChangeWindowAttributes, dmxScreen, pScreen);
	DMX_UNWRAP(RealizeWindow, dmxScreen, pScreen);
	DMX_UNWRAP(UnrealizeWindow, dmxScreen, pScreen);
	DMX_UNWRAP(RestackWindow, dmxScreen, pScreen);
	DMX_UNWRAP(WindowExposures, dmxScreen, pScreen);
	DMX_UNWRAP(CopyWindow, dmxScreen, pScreen);

	DMX_UNWRAP(ResizeWindow, dmxScreen, pScreen);
	DMX_UNWRAP(ReparentWindow, dmxScreen, pScreen);

	DMX_UNWRAP(ChangeBorderWidth, dmxScreen, pScreen);

	DMX_UNWRAP(GetImage, dmxScreen, pScreen);
	DMX_UNWRAP(GetSpans, dmxScreen, pScreen);

	DMX_UNWRAP(CreatePixmap, dmxScreen, pScreen);
	DMX_UNWRAP(DestroyPixmap, dmxScreen, pScreen);
	DMX_UNWRAP(BitmapToRegion, dmxScreen, pScreen);

	DMX_UNWRAP(RealizeFont, dmxScreen, pScreen);
	DMX_UNWRAP(UnrealizeFont, dmxScreen, pScreen);

	DMX_UNWRAP(CreateColormap, dmxScreen, pScreen);
	DMX_UNWRAP(DestroyColormap, dmxScreen, pScreen);
	DMX_UNWRAP(InstallColormap, dmxScreen, pScreen);
	DMX_UNWRAP(StoreColors, dmxScreen, pScreen);
    }

    DMX_UNWRAP(SaveScreen, dmxScreen, pScreen);

    if (dmxScreen->beDisplay) {
	dmxBECloseScreen(pScreen);

#if 1
	/* Free visuals, depths and pixmap formats here so that they
	 * won't be freed when a screen is detached, thereby allowing
	 * the screen to be reattached to be compared to the one
	 * previously removed.
	 */
	XFree(dmxScreen->beVisuals);
	dmxScreen->beVisuals = NULL;

	XFree(dmxScreen->beDepths);
	dmxScreen->beDepths = NULL;

	XFree(dmxScreen->bePixmapFormats);
	dmxScreen->bePixmapFormats = NULL;
#endif
    }

    DMX_UNWRAP(CloseScreen, dmxScreen, pScreen);
    return pScreen->CloseScreen(idx, pScreen);
}

static Bool dmxSaveScreen(ScreenPtr pScreen, int what)
{
    DMXScreenInfo *dmxScreen = &dmxScreens[pScreen->myNum];

    if (dmxScreen->beDisplay) {
	switch (what) {
	case SCREEN_SAVER_OFF:
	case SCREEN_SAVER_FORCER:
	    XResetScreenSaver(dmxScreen->beDisplay);
	    dmxSync(dmxScreen, FALSE);
	    break;
	case SCREEN_SAVER_ON:
	case SCREEN_SAVER_CYCLE:
	    XActivateScreenSaver(dmxScreen->beDisplay);
	    dmxSync(dmxScreen, FALSE);
	    break;
	}
    }

    return TRUE;
}
