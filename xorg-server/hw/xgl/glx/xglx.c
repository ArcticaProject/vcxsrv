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

#include "xglx.h"

#include <X11/extensions/Xrandr.h>
#include <X11/cursorfont.h>

#include <glitz-glx.h>

#ifdef GLXEXT
#include "xglglxext.h"
#endif

#include "inputstr.h"
#include "cursorstr.h"
#include "mipointer.h"

#ifdef RANDR
#include "randrstr.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#ifdef XKB
#include <X11/extensions/XKB.h>
#include <xkbsrv.h>
#include <X11/extensions/XKBconfig.h>

extern Bool
XkbQueryExtension (Display *dpy,
		   int     *opcodeReturn,
		   int     *eventBaseReturn,
		   int     *errorBaseReturn,
		   int     *majorRtrn,
		   int     *minorRtrn);

extern XkbDescPtr
XkbGetKeyboard (Display      *dpy,
		unsigned int which,
		unsigned int deviceSpec);

extern Status
XkbGetControls (Display	    *dpy,
		unsigned long which,
		XkbDescPtr    desc);

#ifndef XKB_BASE_DIRECTORY
#define	XKB_BASE_DIRECTORY	"/usr/lib/X11/xkb/"
#endif
#ifndef XKB_CONFIG_FILE
#define	XKB_CONFIG_FILE		"X0-config.keyboard"
#endif
#ifndef XKB_DFLT_RULES_FILE
#define	XKB_DFLT_RULES_FILE	"xorg"
#endif
#ifndef XKB_DFLT_KB_LAYOUT
#define	XKB_DFLT_KB_LAYOUT	"us"
#endif
#ifndef XKB_DFLT_KB_MODEL
#define	XKB_DFLT_KB_MODEL	"pc101"
#endif
#ifndef XKB_DFLT_KB_VARIANT
#define	XKB_DFLT_KB_VARIANT	NULL
#endif
#ifndef XKB_DFLT_KB_OPTIONS
#define	XKB_DFLT_KB_OPTIONS	NULL
#endif

#endif

#define XGLX_DEFAULT_SCREEN_WIDTH  800
#define XGLX_DEFAULT_SCREEN_HEIGHT 600

typedef struct _xglxScreen {
    Window	       win, root;
    Colormap	       colormap;
    Bool	       fullscreen;
    CloseScreenProcPtr CloseScreen;
} xglxScreenRec, *xglxScreenPtr;

DevPrivateKey xglxScreenPrivateKey = &xglxScreenPrivateKey;

#define XGLX_GET_SCREEN_PRIV(pScreen) ((xglxScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, xglxScreenPrivateKey))

#define XGLX_SET_SCREEN_PRIV(pScreen, v) \
    dixSetPrivate(&(pScreen)->devPrivates, xglxScreenPrivateKey, v)

#define XGLX_SCREEN_PRIV(pScreen)			       \
    xglxScreenPtr pScreenPriv = XGLX_GET_SCREEN_PRIV (pScreen)

typedef struct _xglxCursor {
    Cursor cursor;
} xglxCursorRec, *xglxCursorPtr;

#define XGLX_GET_CURSOR_PRIV(pCursor, pScreen)		   \
    ((xglxCursorPtr)dixLookupPrivate(&(pCursor)->devPrivates, pScreen))

#define XGLX_SET_CURSOR_PRIV(pCursor, pScreen, v)	 \
    dixSetPrivate(&(pCursor)->devPrivates, pScreen, v)

#define XGLX_CURSOR_PRIV(pCursor, pScreen)			        \
    xglxCursorPtr pCursorPriv = XGLX_GET_CURSOR_PRIV (pCursor, pScreen)

static char	 *xDisplayName = 0;
static Display	 *xdisplay     = 0;
static int	 xscreen;
static CARD32	 lastEventTime = 0;
static ScreenPtr currentScreen = 0;
static Bool	 softCursor    = FALSE;
static Bool	 fullscreen    = TRUE;

static Bool randrExtension = FALSE;
static int  randrEvent, randrError;

static glitz_drawable_format_t *xglxScreenFormat = 0;

static Bool
xglxAllocatePrivates (ScreenPtr pScreen)
{
    xglxScreenPtr pScreenPriv;

    pScreenPriv = xalloc (sizeof (xglxScreenRec));
    if (!pScreenPriv)
	return FALSE;

    XGLX_SET_SCREEN_PRIV (pScreen, pScreenPriv);

    return TRUE;
}

#ifdef RANDR

#define DEFAULT_REFRESH_RATE 50

static Bool
xglxRandRGetInfo (ScreenPtr pScreen,
		  Rotation  *rotations)
{
    RRScreenSizePtr pSize;

    *rotations = RR_Rotate_0;

    if (randrExtension)
    {
	XRRScreenConfiguration *xconfig;
	XRRScreenSize	       *sizes;
	int		       nSizes, currentSize = 0;
	short		       *rates, currentRate;
	int		       nRates, i, j;

	XGLX_SCREEN_PRIV (pScreen);

	xconfig	    = XRRGetScreenInfo (xdisplay, pScreenPriv->root);
	sizes	    = XRRConfigSizes (xconfig, &nSizes);
	currentRate = XRRConfigCurrentRate (xconfig);

	if (pScreenPriv->fullscreen)
	{
	    Rotation rotation;

	    currentSize = XRRConfigCurrentConfiguration (xconfig, &rotation);

	    for (i = 0; i < nSizes; i++)
	    {
		pSize = RRRegisterSize (pScreen,
					sizes[i].width,
					sizes[i].height,
					sizes[i].mwidth,
					sizes[i].mheight);

		rates = XRRConfigRates (xconfig, i, &nRates);

		for (j = 0; j < nRates; j++)
		{
		    RRRegisterRate (pScreen, pSize, rates[j]);

		    if (i == currentSize && rates[j] == currentRate)
			RRSetCurrentConfig (pScreen, RR_Rotate_0, currentRate,
					    pSize);
		}
	    }
	}
	else
	{
	    pSize = RRRegisterSize (pScreen,
				    pScreen->width,
				    pScreen->height,
				    pScreen->mmWidth,
				    pScreen->mmHeight);

	    for (i = 0; i < nSizes; i++)
	    {
		rates = XRRConfigRates (xconfig, i, &nRates);

		for (j = 0; j < nRates; j++)
		{
		    RRRegisterRate (pScreen, pSize, rates[j]);

		    if (rates[j] == currentRate)
			RRSetCurrentConfig (pScreen, RR_Rotate_0, currentRate,
					    pSize);
		}
	    }
	}

	XRRFreeScreenConfigInfo (xconfig);
    }
    else
    {
	pSize = RRRegisterSize (pScreen,
				pScreen->width,
				pScreen->height,
				pScreen->mmWidth,
				pScreen->mmHeight);

	RRRegisterRate (pScreen, pSize, DEFAULT_REFRESH_RATE);
	RRSetCurrentConfig (pScreen, RR_Rotate_0, DEFAULT_REFRESH_RATE, pSize);
    }

    return TRUE;
}

static Bool
xglxRandRSetConfig (ScreenPtr	    pScreen,
		    Rotation	    rotations,
		    int		    rate,
		    RRScreenSizePtr pSize)
{
    if (randrExtension)
    {
	XRRScreenConfiguration *xconfig;
	XRRScreenSize	       *sizes;
	int		       nSizes, currentSize;
	int		       i, size = -1;
	int		       status = RRSetConfigFailed;
	Rotation	       rotation;

	XGLX_SCREEN_PRIV (pScreen);

	xconfig	    = XRRGetScreenInfo (xdisplay, pScreenPriv->root);
	sizes	    = XRRConfigSizes (xconfig, &nSizes);
	currentSize = XRRConfigCurrentConfiguration (xconfig, &rotation);

	for (i = 0; i < nSizes; i++)
	{
	    if (pScreenPriv->fullscreen)
	    {
		if (sizes[i].width   == pSize->width   &&
		    sizes[i].height  == pSize->height  &&
		    sizes[i].mwidth  == pSize->mmWidth &&
		    sizes[i].mheight == pSize->mmHeight)
		{
		    size = i;
		    break;
		}
	    }
	    else
	    {
		short *rates;
		int   nRates, j;

		rates = XRRConfigRates (xconfig, i, &nRates);

		for (j = 0; j < nRates; j++)
		{
		    if (rates[j] == rate)
		    {
			size = i;
			if (i >= currentSize)
			    break;
		    }
		}
	    }
	}

	if (size >= 0)
	    status = XRRSetScreenConfigAndRate (xdisplay,
						xconfig,
						pScreenPriv->root,
						size,
						RR_Rotate_0,
						rate,
						CurrentTime);

	XRRFreeScreenConfigInfo (xconfig);

	if (status == RRSetConfigSuccess)
	{
	    PixmapPtr pPixmap;

	    pPixmap = (*pScreen->GetScreenPixmap) (pScreen);

	    if (pScreenPriv->fullscreen)
	    {
		XGL_PIXMAP_PRIV (pPixmap);

		xglSetRootClip (pScreen, FALSE);

		XResizeWindow (xdisplay, pScreenPriv->win,
			       pSize->width, pSize->height);

		glitz_drawable_update_size (pPixmapPriv->drawable,
					    pSize->width, pSize->height);

		pScreen->width    = pSize->width;
		pScreen->height   = pSize->height;
		pScreen->mmWidth  = pSize->mmWidth;
		pScreen->mmHeight = pSize->mmHeight;

		(*pScreen->ModifyPixmapHeader) (pPixmap,
						pScreen->width,
						pScreen->height,
						pPixmap->drawable.depth,
						pPixmap->drawable.bitsPerPixel,
						0, 0);

		xglSetRootClip (pScreen, TRUE);
	    }

	    return TRUE;
	}
    }

    return FALSE;
}

static Bool
xglxRandRInit (ScreenPtr pScreen)
{
    rrScrPrivPtr pScrPriv;

    if (!RRScreenInit (pScreen))
	return FALSE;

    pScrPriv = rrGetScrPriv (pScreen);
    pScrPriv->rrGetInfo   = xglxRandRGetInfo;
    pScrPriv->rrSetConfig = xglxRandRSetConfig;

    return TRUE;
}

#endif

static void
xglxConstrainCursor (ScreenPtr pScreen,
		     BoxPtr    pBox)
{
}

static void
xglxCursorLimits (ScreenPtr pScreen,
		  CursorPtr pCursor,
		  BoxPtr    pHotBox,
		  BoxPtr    pTopLeftBox)
{
    *pTopLeftBox = *pHotBox;
}

static Bool
xglxDisplayCursor (ScreenPtr pScreen,
		   CursorPtr pCursor)
{
    XGLX_SCREEN_PRIV (pScreen);
    XGLX_CURSOR_PRIV (pCursor, pScreen);

    XDefineCursor (xdisplay, pScreenPriv->win, pCursorPriv->cursor);

    return TRUE;
}

#ifdef ARGB_CURSOR

static Bool
xglxARGBCursorSupport (void);

static Cursor
xglxCreateARGBCursor (ScreenPtr pScreen,
		      CursorPtr pCursor);

#endif

static Bool
xglxRealizeCursor (ScreenPtr pScreen,
		   CursorPtr pCursor)
{
    xglxCursorPtr pCursorPriv;
    XImage	  *ximage;
    Pixmap	  source, mask;
    XColor	  fgColor, bgColor;
    XlibGC	  xgc;
    unsigned long valuemask;
    XGCValues	  values;

    XGLX_SCREEN_PRIV (pScreen);

    valuemask = GCForeground | GCBackground;

    values.foreground = 1L;
    values.background = 0L;

    pCursorPriv = xalloc (sizeof (xglxCursorRec));
    if (!pCursorPriv)
	return FALSE;

    XGLX_SET_CURSOR_PRIV (pCursor, pScreen, pCursorPriv);

#ifdef ARGB_CURSOR
    if (pCursor->bits->argb)
    {
	pCursorPriv->cursor = xglxCreateARGBCursor (pScreen, pCursor);
	if (pCursorPriv->cursor)
	    return TRUE;
    }
#endif

    source = XCreatePixmap (xdisplay,
			    pScreenPriv->win,
			    pCursor->bits->width,
			    pCursor->bits->height,
			    1);

    mask = XCreatePixmap (xdisplay,
			  pScreenPriv->win,
			  pCursor->bits->width,
			  pCursor->bits->height,
			  1);

    xgc = XCreateGC (xdisplay, source, valuemask, &values);

    ximage = XCreateImage (xdisplay,
			   DefaultVisual (xdisplay, xscreen),
			   1, XYBitmap, 0,
			   (char *) pCursor->bits->source,
			   pCursor->bits->width,
			   pCursor->bits->height,
			   BitmapPad (xdisplay), 0);

    XPutImage (xdisplay, source, xgc, ximage,
	       0, 0, 0, 0, pCursor->bits->width, pCursor->bits->height);

    XFree (ximage);

    ximage = XCreateImage (xdisplay,
			   DefaultVisual (xdisplay, xscreen),
			   1, XYBitmap, 0,
			   (char *) pCursor->bits->mask,
			   pCursor->bits->width,
			   pCursor->bits->height,
			   BitmapPad (xdisplay), 0);

    XPutImage (xdisplay, mask, xgc, ximage,
	       0, 0, 0, 0, pCursor->bits->width, pCursor->bits->height);

    XFree (ximage);
    XFreeGC (xdisplay, xgc);

    fgColor.red   = pCursor->foreRed;
    fgColor.green = pCursor->foreGreen;
    fgColor.blue  = pCursor->foreBlue;

    bgColor.red   = pCursor->backRed;
    bgColor.green = pCursor->backGreen;
    bgColor.blue  = pCursor->backBlue;

    pCursorPriv->cursor =
	XCreatePixmapCursor (xdisplay, source, mask, &fgColor, &bgColor,
			     pCursor->bits->xhot, pCursor->bits->yhot);

    XFreePixmap (xdisplay, mask);
    XFreePixmap (xdisplay, source);

    return TRUE;
}

static Bool
xglxUnrealizeCursor (ScreenPtr pScreen,
		     CursorPtr pCursor)
{
    XGLX_CURSOR_PRIV (pCursor, pScreen);

    XFreeCursor (xdisplay, pCursorPriv->cursor);
    xfree (pCursorPriv);

    return TRUE;
}

static void
xglxRecolorCursor (ScreenPtr pScreen,
		   CursorPtr pCursor,
		   Bool	     displayed)
{
    XColor fgColor, bgColor;

    XGLX_CURSOR_PRIV (pCursor, pScreen);

    fgColor.red   = pCursor->foreRed;
    fgColor.green = pCursor->foreGreen;
    fgColor.blue  = pCursor->foreBlue;

    bgColor.red   = pCursor->backRed;
    bgColor.green = pCursor->backGreen;
    bgColor.blue  = pCursor->backBlue;

    XRecolorCursor (xdisplay, pCursorPriv->cursor, &fgColor, &bgColor);
}

static Bool
xglxSetCursorPosition (ScreenPtr pScreen,
		       int	 x,
		       int	 y,
		       Bool	 generateEvent)
{
    XGLX_SCREEN_PRIV (pScreen);

    XWarpPointer (xdisplay, pScreenPriv->win, pScreenPriv->win,
		  0, 0, 0, 0, x, y);

    return TRUE;
}

static Bool
xglxCloseScreen (int	   index,
		 ScreenPtr pScreen)
{
    glitz_drawable_t *drawable;

    XGLX_SCREEN_PRIV (pScreen);

    drawable = XGL_GET_SCREEN_PRIV (pScreen)->drawable;
    if (drawable)
	glitz_drawable_destroy (drawable);

    xglClearVisualTypes ();

    if (pScreenPriv->win)
	XDestroyWindow (xdisplay, pScreenPriv->win);

    if (pScreenPriv->colormap)
	XFreeColormap (xdisplay, pScreenPriv->colormap);

    XGL_SCREEN_UNWRAP (CloseScreen);
    xfree (pScreenPriv);

    return (*pScreen->CloseScreen) (index, pScreen);
}

static Bool
xglxCursorOffScreen (ScreenPtr *ppScreen, int *x, int *y)
{
    return FALSE;
}

static void
xglxCrossScreen (ScreenPtr pScreen, Bool entering)
{
}

static void
xglxWarpCursor (ScreenPtr pScreen, int x, int y)
{
    miPointerWarpCursor (pScreen, x, y);
}

miPointerScreenFuncRec xglxPointerScreenFuncs = {
    xglxCursorOffScreen,
    xglxCrossScreen,
    xglxWarpCursor
};

static Bool
xglxScreenInit (int	  index,
		ScreenPtr pScreen,
		int	  argc,
		char	  **argv)
{
    XSetWindowAttributes    xswa;
    XWMHints		    *wmHints;
    XSizeHints		    *normalHints;
    XClassHint		    *classHint;
    xglxScreenPtr	    pScreenPriv;
    XVisualInfo		    *vinfo;
    XEvent		    xevent;
    glitz_drawable_format_t *format;
    glitz_drawable_t	    *drawable;

    format = xglxScreenFormat;

    if (!xglxAllocatePrivates (pScreen))
	return FALSE;

    currentScreen = pScreen;

    pScreenPriv = XGLX_GET_SCREEN_PRIV (pScreen);

    pScreenPriv->root	    = RootWindow (xdisplay, xscreen);
    pScreenPriv->fullscreen = fullscreen;

    vinfo = glitz_glx_get_visual_info_from_format (xdisplay, xscreen, format);
    if (!vinfo)
    {
	ErrorF ("[%d] no visual info from format\n", index);
	return FALSE;
    }

    pScreenPriv->colormap =
	XCreateColormap (xdisplay, pScreenPriv->root, vinfo->visual,
			 AllocNone);

    if (XRRQueryExtension (xdisplay, &randrEvent, &randrError))
	randrExtension = TRUE;

    if (fullscreen)
    {
	xglScreenInfo.width    = DisplayWidth (xdisplay, xscreen);
	xglScreenInfo.height   = DisplayHeight (xdisplay, xscreen);
	xglScreenInfo.widthMm  = DisplayWidthMM (xdisplay, xscreen);
	xglScreenInfo.heightMm = DisplayHeightMM (xdisplay, xscreen);

	if (randrExtension)
	{
	    XRRScreenConfiguration *xconfig;
	    Rotation		   rotation;
	    XRRScreenSize	   *sizes;
	    int			   nSizes, currentSize;

	    xconfig	= XRRGetScreenInfo (xdisplay, pScreenPriv->root);
	    currentSize = XRRConfigCurrentConfiguration (xconfig, &rotation);
	    sizes	= XRRConfigSizes (xconfig, &nSizes);

	    xglScreenInfo.width    = sizes[currentSize].width;
	    xglScreenInfo.height   = sizes[currentSize].height;
	    xglScreenInfo.widthMm  = sizes[currentSize].mwidth;
	    xglScreenInfo.heightMm = sizes[currentSize].mheight;

	    XRRFreeScreenConfigInfo (xconfig);
	}
    }
    else if (xglScreenInfo.width == 0 || xglScreenInfo.height == 0)
    {
	xglScreenInfo.width  = XGLX_DEFAULT_SCREEN_WIDTH;
	xglScreenInfo.height = XGLX_DEFAULT_SCREEN_HEIGHT;
    }

    xswa.colormap = pScreenPriv->colormap;

    pScreenPriv->win =
	XCreateWindow (xdisplay, pScreenPriv->root, 0, 0,
		       xglScreenInfo.width, xglScreenInfo.height, 0,
		       vinfo->depth, InputOutput, vinfo->visual,
		       CWColormap, &xswa);

    XFree (vinfo);

    normalHints = XAllocSizeHints ();
    normalHints->flags      = PMinSize | PMaxSize | PSize;
    normalHints->min_width  = xglScreenInfo.width;
    normalHints->min_height = xglScreenInfo.height;
    normalHints->max_width  = xglScreenInfo.width;
    normalHints->max_height = xglScreenInfo.height;

    if (fullscreen)
    {
	normalHints->x = 0;
	normalHints->y = 0;
	normalHints->flags |= PPosition;
    }

    classHint = XAllocClassHint ();
    classHint->res_name = "xglx";
    classHint->res_class = "Xglx";

    wmHints = XAllocWMHints ();
    wmHints->flags = InputHint;
    wmHints->input = TRUE;

    Xutf8SetWMProperties (xdisplay, pScreenPriv->win, "Xglx", "Xglx", 0, 0,
			  normalHints, wmHints, classHint);

    XFree (wmHints);
    XFree (classHint);
    XFree (normalHints);

    drawable = glitz_glx_create_drawable_for_window (xdisplay, xscreen,
						     format, pScreenPriv->win,
						     xglScreenInfo.width,
						     xglScreenInfo.height);
    if (!drawable)
    {
	ErrorF ("[%d] couldn't create glitz drawable for window\n", index);
	return FALSE;
    }

    XSelectInput (xdisplay, pScreenPriv->win,
		  ButtonPressMask | ButtonReleaseMask |
		  KeyPressMask | KeyReleaseMask | EnterWindowMask |
		  PointerMotionMask | ExposureMask);

    XMapWindow (xdisplay, pScreenPriv->win);

    if (fullscreen)
    {
	XClientMessageEvent xev;

	memset (&xev, 0, sizeof (xev));

	xev.type = ClientMessage;
	xev.message_type = XInternAtom (xdisplay, "_NET_WM_STATE", FALSE);
	xev.display = xdisplay;
	xev.window = pScreenPriv->win;
	xev.format = 32;
	xev.data.l[0] = 1;
	xev.data.l[1] =
	    XInternAtom (xdisplay, "_NET_WM_STATE_FULLSCREEN", FALSE);

	XSendEvent (xdisplay, pScreenPriv->root, FALSE,
		    SubstructureRedirectMask, (XEvent *) &xev);
    }

    xglScreenInfo.drawable = drawable;

    if (!xglScreenInit (pScreen))
	return FALSE;

#ifdef GLXEXT
    if (!xglInitVisualConfigs (pScreen))
	return FALSE;
#endif

    XGL_SCREEN_WRAP (CloseScreen, xglxCloseScreen);

#ifdef ARGB_CURSOR
    if (!xglxARGBCursorSupport ())
	softCursor = TRUE;
#endif

    if (softCursor)
    {
	static char data = 0;
	XColor	    black, dummy;
	Pixmap	    bitmap;
	Cursor	    cursor;

	if (!XAllocNamedColor (xdisplay, pScreenPriv->colormap,
			       "black", &black, &dummy))
	    return FALSE;

	bitmap = XCreateBitmapFromData (xdisplay, pScreenPriv->win, &data,
					1, 1);
	if (!bitmap)
	    return FALSE;

	cursor = XCreatePixmapCursor (xdisplay, bitmap, bitmap, &black, &black,
				      0, 0);
	if (!cursor)
	    return FALSE;

	XDefineCursor (xdisplay, pScreenPriv->win, cursor);

	XFreeCursor (xdisplay, cursor);
	XFreePixmap (xdisplay, bitmap);
	XFreeColors (xdisplay, pScreenPriv->colormap, &black.pixel, 1, 0);

	miDCInitialize (pScreen, &xglxPointerScreenFuncs);
    }
    else
    {
	pScreen->ConstrainCursor   = xglxConstrainCursor;
	pScreen->CursorLimits      = xglxCursorLimits;
	pScreen->DisplayCursor     = xglxDisplayCursor;
	pScreen->RealizeCursor     = xglxRealizeCursor;
	pScreen->UnrealizeCursor   = xglxUnrealizeCursor;
	pScreen->RecolorCursor     = xglxRecolorCursor;
	pScreen->SetCursorPosition = xglxSetCursorPosition;
    }

    if (!xglFinishScreenInit (pScreen))
	return FALSE;

#ifdef RANDR
    if (!xglxRandRInit (pScreen))
	return FALSE;
#endif

    while (XNextEvent (xdisplay, &xevent))
	if (xevent.type == Expose)
	    break;

    return TRUE;
}

void
xglxInitOutput (ScreenInfo *pScreenInfo,
		int	   argc,
		char	   **argv)
{
    glitz_drawable_format_t *format, templ;
    int			    i;
    unsigned long	    mask;
    unsigned long	    extraMask[] = {
	GLITZ_FORMAT_DOUBLEBUFFER_MASK | GLITZ_FORMAT_ALPHA_SIZE_MASK,
	GLITZ_FORMAT_DOUBLEBUFFER_MASK,
	GLITZ_FORMAT_ALPHA_SIZE_MASK,
	0
    };

    xglClearVisualTypes ();

    xglSetPixmapFormats (pScreenInfo);

    if (!xdisplay)
    {
	char *name = xDisplayName;

	if (!name)
	    name = xglxInitXorg ();

	xdisplay = XOpenDisplay (name);
	if (!xdisplay)
	    FatalError ("can't open display: %s\n", name ? name : "NULL");

	xscreen = DefaultScreen (xdisplay);

	if (!xDisplayName)
	    XDefineCursor (xdisplay, RootWindow (xdisplay, xscreen),
			   XCreateFontCursor (xdisplay, XC_watch));
    }

    templ.samples          = 1;
    templ.doublebuffer     = 1;
    templ.color.fourcc     = GLITZ_FOURCC_RGB;
    templ.color.alpha_size = 8;

    mask = GLITZ_FORMAT_SAMPLES_MASK | GLITZ_FORMAT_FOURCC_MASK;

    for (i = 0; i < sizeof (extraMask) / sizeof (extraMask[0]); i++)
    {
	format = glitz_glx_find_window_format (xdisplay, xscreen,
					       mask | extraMask[i],
					       &templ, 0);
	if (format)
	    break;
    }

    if (!format)
	FatalError ("no visual format found");

    xglScreenInfo.depth =
	format->color.red_size   +
	format->color.green_size +
	format->color.blue_size;

    xglSetVisualTypes (xglScreenInfo.depth,
		       (1 << TrueColor),
		       format->color.red_size,
		       format->color.green_size,
		       format->color.blue_size);

    xglxScreenFormat = format;

    AddScreen (xglxScreenInit, argc, argv);
}

static Bool
xglxExposurePredicate (Display *xdisplay,
		       XEvent  *xevent,
		       char    *args)
{
    return (xevent->type == Expose);
}

static Bool
xglxNotExposurePredicate (Display *xdisplay,
			  XEvent  *xevent,
			  char	  *args)
{
    return (xevent->type != Expose);
}

static int
xglxWindowExposures (WindowPtr pWin,
		     pointer   pReg)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    RegionRec ClipList;

    if (HasBorder (pWin))
    {
	REGION_INIT (pScreen, &ClipList, NullBox, 0);
	REGION_SUBTRACT (pScreen, &ClipList, &pWin->borderClip,
			 &pWin->winSize);
	REGION_INTERSECT (pScreen, &ClipList, &ClipList, (RegionPtr) pReg);
	miPaintWindow(pWin, &ClipList, PW_BORDER);
	REGION_UNINIT (pScreen, &ClipList);
    }

    REGION_INIT (pScreen, &ClipList, NullBox, 0);
    REGION_INTERSECT (pScreen, &ClipList, &pWin->clipList, (RegionPtr) pReg);
    (*pScreen->WindowExposures) (pWin, &ClipList, NullRegion);
    REGION_UNINIT (pScreen, &ClipList);

    return WT_WALKCHILDREN;
}

static void
xglxBlockHandler (pointer   blockData,
		  OSTimePtr pTimeout,
		  pointer   pReadMask)
{
    XEvent    X;
    RegionRec region;
    BoxRec    box;

    XGL_SCREEN_PRIV (currentScreen);

    while (XCheckIfEvent (xdisplay, &X, xglxExposurePredicate, NULL))
    {
	ScreenPtr pScreen = currentScreen;

	box.x1 = X.xexpose.x;
	box.y1 = X.xexpose.y;
	box.x2 = box.x1 + X.xexpose.width;
	box.y2 = box.y1 + X.xexpose.height;

	REGION_INIT (currentScreen, &region, &box, 1);

	WalkTree (pScreen, xglxWindowExposures, &region);

	REGION_UNINIT (pScreen, &region);
    }

    if (!xglSyncSurface (&pScreenPriv->pScreenPixmap->drawable))
	FatalError (XGL_SW_FAILURE_STRING);

    glitz_surface_flush (pScreenPriv->surface);
    glitz_drawable_flush (pScreenPriv->drawable);

    XFlush (xdisplay);
}

static void
xglxWakeupHandler (pointer blockData,
		   int     result,
		   pointer pReadMask)
{
    ScreenPtr pScreen = currentScreen;
    XEvent    X;
    xEvent    x;

    while (XCheckIfEvent (xdisplay, &X, xglxNotExposurePredicate, NULL))
    {
	switch (X.type) {
	case KeyPress:
	    x.u.u.type = KeyPress;
	    x.u.u.detail = X.xkey.keycode;
	    x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis ();
	    mieqEnqueue (&x);
	    break;
	case KeyRelease:
	    x.u.u.type = KeyRelease;
	    x.u.u.detail = X.xkey.keycode;
	    x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis ();
	    mieqEnqueue (&x);
	    break;
	case ButtonPress:
	    x.u.u.type = ButtonPress;
	    x.u.u.detail = X.xbutton.button;
	    x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis ();
	    mieqEnqueue (&x);
	    break;
	case ButtonRelease:
	    x.u.u.type = ButtonRelease;
	    x.u.u.detail = X.xbutton.button;
	    x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis ();
	    mieqEnqueue (&x);
	    break;
	case MotionNotify:
	    x.u.u.type = MotionNotify;
	    x.u.u.detail = 0;
	    x.u.keyButtonPointer.rootX = X.xmotion.x;
	    x.u.keyButtonPointer.rootY = X.xmotion.y;
	    x.u.keyButtonPointer.time = lastEventTime = GetTimeInMillis ();
	    miPointerAbsoluteCursor (X.xmotion.x, X.xmotion.y, lastEventTime);
	    mieqEnqueue (&x);
	    break;
	case EnterNotify:
	    if (X.xcrossing.detail != NotifyInferior) {
		if (pScreen) {
		    NewCurrentScreen (pScreen, X.xcrossing.x, X.xcrossing.y);
		    x.u.u.type = MotionNotify;
		    x.u.u.detail = 0;
		    x.u.keyButtonPointer.rootX = X.xcrossing.x;
		    x.u.keyButtonPointer.rootY = X.xcrossing.y;
		    x.u.keyButtonPointer.time = lastEventTime =
			GetTimeInMillis ();
		    mieqEnqueue (&x);
		}
	    }
	    break;
	default:
	    break;
	}
    }
}

static void
xglxBell (int	       volume,
	  DeviceIntPtr pDev,
	  pointer      ctrl,
	  int	       cls)
{
  XBell (xdisplay, volume);
}

static void
xglxKbdCtrl (DeviceIntPtr pDev,
	     KeybdCtrl    *ctrl)
{
    unsigned long    valueMask;
    XKeyboardControl values;
    int		     i;

    valueMask = KBKeyClickPercent | KBBellPercent | KBBellPitch |
	KBBellDuration | KBAutoRepeatMode;

    values.key_click_percent = ctrl->click;
    values.bell_percent	     = ctrl->bell;
    values.bell_pitch	     = ctrl->bell_pitch;
    values.bell_duration     = ctrl->bell_duration;
    values.auto_repeat_mode  = (ctrl->autoRepeat) ? AutoRepeatModeOn :
	AutoRepeatModeOff;

    XChangeKeyboardControl (xdisplay, valueMask, &values);

    valueMask = KBLed | KBLedMode;

    for (i = 1; i <= 32; i++)
    {
	values.led = i;
	values.led_mode = (ctrl->leds & (1 << (i - 1))) ? LedModeOn :
	    LedModeOff;

	XChangeKeyboardControl (xdisplay, valueMask, &values);
    }
}

static int
xglxKeybdProc (DeviceIntPtr pDevice,
	       int	    onoff)
{
    Bool      ret = FALSE;
    DevicePtr pDev = (DevicePtr) pDevice;

    if (!pDev)
	return BadImplementation;

    switch (onoff) {
    case DEVICE_INIT: {
      XModifierKeymap *xmodMap;
      KeySym	      *xkeyMap;
      int	      minKeyCode, maxKeyCode, mapWidth, i, j;
      KeySymsRec      xglxKeySyms;
      CARD8	      xglxModMap[256];
      XKeyboardState  values;

#ifdef _XSERVER64
      KeySym64	      *xkeyMap64;
      int	      len;
#endif

#ifdef XKB
      Bool	      xkbExtension = FALSE;
      int	      xkbOp, xkbEvent, xkbError, xkbMajor, xkbMinor;
#endif

      if (pDev != (DevicePtr)inputInfo.keyboard)
	  return !Success;

      xmodMap = XGetModifierMapping (xdisplay);

      XDisplayKeycodes (xdisplay, &minKeyCode, &maxKeyCode);

#ifdef _XSERVER64
      xkeyMap64 = XGetKeyboardMapping (xdisplay,
				       minKeyCode,
				       maxKeyCode - minKeyCode + 1,
				       &mapWidth);

      len = (maxKeyCode - minKeyCode + 1) * mapWidth;
      xkeyMap = (KeySym *) xalloc (len * sizeof (KeySym));
      for (i = 0; i < len; ++i)
	  xkeyMap[i] = xkeyMap64[i];

      XFree (xkeyMap64);
#else
      xkeyMap = XGetKeyboardMapping (xdisplay,
				     minKeyCode,
				     maxKeyCode - minKeyCode + 1,
				     &mapWidth);
#endif

      memset (xglxModMap, 0, 256);

      for (j = 0; j < 8; j++)
      {
	  for (i = 0; i < xmodMap->max_keypermod; i++)
	  {
	      CARD8 keyCode;

	      keyCode = xmodMap->modifiermap[j * xmodMap->max_keypermod + i];
	      if (keyCode)
		  xglxModMap[keyCode] |= 1 << j;
	  }
      }

      XFreeModifiermap (xmodMap);

      xglxKeySyms.minKeyCode = minKeyCode;
      xglxKeySyms.maxKeyCode = maxKeyCode;
      xglxKeySyms.mapWidth   = mapWidth;
      xglxKeySyms.map	     = xkeyMap;

#ifdef XKB
      if (!noXkbExtension)
	  xkbExtension = XkbQueryExtension (xdisplay,
					    &xkbOp, &xkbEvent, &xkbError,
					    &xkbMajor, &xkbMinor);

      if (xkbExtension)
      {
	  XkbDescPtr desc;
	  char	     *rules, *model, *layout, *variants, *options;

	  desc = XkbGetKeyboard (xdisplay,
				 XkbGBN_AllComponentsMask,
				 XkbUseCoreKbd);

	  if (desc && desc->geom)
	  {
	      XkbComponentNamesRec names;
	      FILE		   *file;

	      rules    = XKB_DFLT_RULES_FILE;
	      model    = XKB_DFLT_KB_MODEL;
	      layout   = XKB_DFLT_KB_LAYOUT;
	      variants = XKB_DFLT_KB_VARIANT;
	      options  = XKB_DFLT_KB_OPTIONS;

	      XkbGetControls (xdisplay, XkbAllControlsMask, desc);

	      memset (&names, 0, sizeof (XkbComponentNamesRec));

	      XkbSetRulesDflts (rules, model, layout, variants, options);

	      ret = XkbInitKeyboardDeviceStruct ((pointer) pDev,
						 &names,
						 &xglxKeySyms,
						 xglxModMap,
						 xglxBell,
						 xglxKbdCtrl);

	      if (ret)
		  XkbDDXChangeControls ((pointer) pDev, desc->ctrls,
					desc->ctrls);

	      XkbFreeKeyboard (desc, 0, False);
	  }
      }
#endif

      if (!ret)
      {
	  XGetKeyboardControl (xdisplay, &values);

	  memmove (defaultKeyboardControl.autoRepeats,
		   values.auto_repeats, sizeof (values.auto_repeats));

	  ret = InitKeyboardDeviceStruct (pDev,
					  &xglxKeySyms,
					  xglxModMap,
					  xglxBell,
					  xglxKbdCtrl);
      }

#ifdef _XSERVER64
      xfree (xkeyMap);
#else
      XFree (xkeyMap);
#endif

      if (!ret)
	  return BadImplementation;

    } break;
    case DEVICE_ON:
	pDev->on = TRUE;
	break;
    case DEVICE_OFF:
    case DEVICE_CLOSE:
	pDev->on = FALSE;
	break;
    }

    return Success;
}

Bool
xglxLegalModifier (unsigned int key,
		   DeviceIntPtr pDev)
{
    return TRUE;
}

void
xglxProcessInputEvents (void)
{
    mieqProcessInputEvents ();
    miPointerUpdate ();
}

void
xglxInitInput (int  argc,
	       char **argv)
{
    DeviceIntPtr pKeyboard, pPointer;

    pPointer  = AddInputDevice (xglMouseProc, TRUE);
    pKeyboard = AddInputDevice (xglxKeybdProc, TRUE);

    RegisterPointerDevice (pPointer);
    RegisterKeyboardDevice (pKeyboard);

    miRegisterPointerDevice (screenInfo.screens[0], pPointer);
    mieqInit (&pKeyboard->public, &pPointer->public);

    AddEnabledDevice (XConnectionNumber (xdisplay));

    RegisterBlockAndWakeupHandlers (xglxBlockHandler,
				    xglxWakeupHandler,
				    NULL);
}

void
xglxUseMsg (void)
{
    ErrorF ("-screen WIDTH[/WIDTHMM]xHEIGHT[/HEIGHTMM] "
	    "specify screen characteristics\n");
    ErrorF ("-fullscreen            run fullscreen\n");
    ErrorF ("-display string        display name of the real server\n");
    ErrorF ("-softcursor            force software cursor\n");

    if (!xDisplayName)
	xglxUseXorgMsg ();
}

int
xglxProcessArgument (int  argc,
		     char **argv,
		     int  i)
{
    static Bool checkDisplayName = FALSE;

    if (!checkDisplayName)
    {
	char *display = ":0";
	int  j;

	for (j = i; j < argc; j++)
	{
	    if (!strcmp (argv[j], "-display"))
	    {
		if (++j < argc)
		    xDisplayName = argv[j];

		break;
	    }
	    else if (argv[j][0] == ':')
	    {
		display = argv[j];
	    }
	}

	if (!xDisplayName)
	    xDisplayName = getenv ("DISPLAY");

	if (xDisplayName)
	{
	    int n;

	    n = strspn (xDisplayName, ":0123456789");
	    if (strncmp (xDisplayName, display, n) == 0)
		xDisplayName = 0;
	}

	if (xDisplayName)
	    fullscreen = FALSE;

	checkDisplayName = TRUE;
    }

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
    else if (!strcmp (argv[i], "-fullscreen"))
    {
	fullscreen = TRUE;
	return 1;
    }
    else if (!strcmp (argv[i], "-display"))
    {
	if (++i < argc)
	    return 2;

	return 0;
    }
    else if (!strcmp (argv[i], "-softcursor"))
    {
	softCursor = TRUE;
	return 1;
    }
    else if (!xDisplayName)
    {
	return xglxProcessXorgArgument (argc, argv, i);
    }

    return 0;
}

void
xglxAbort (void)
{
    xglxAbortXorg ();
}

void
xglxGiveUp (void)
{
    AbortDDX ();
}

void
xglxOsVendorInit (void)
{
}

#ifdef ARGB_CURSOR

#include <X11/extensions/Xrender.h>

static Bool
xglxARGBCursorSupport (void)
{
    int renderMajor, renderMinor;

    if (!XRenderQueryVersion (xdisplay, &renderMajor, &renderMinor))
	renderMajor = renderMinor = -1;

    return (renderMajor > 0 || renderMinor > 4);
}

static Cursor
xglxCreateARGBCursor (ScreenPtr pScreen,
		      CursorPtr pCursor)
{
    Pixmap	      xpixmap;
    XlibGC	      xgc;
    XImage	      *ximage;
    XRenderPictFormat *xformat;
    Picture	      xpicture;
    Cursor	      cursor;

    XGLX_SCREEN_PRIV (pScreen);

    xpixmap = XCreatePixmap (xdisplay,
			     pScreenPriv->win,
			     pCursor->bits->width,
			     pCursor->bits->height,
			     32);

    xgc = XCreateGC (xdisplay, xpixmap, 0, NULL);

    ximage = XCreateImage (xdisplay,
			   DefaultVisual (xdisplay, xscreen),
			   32, ZPixmap, 0,
			   (char *) pCursor->bits->argb,
			   pCursor->bits->width,
			   pCursor->bits->height,
			   32, pCursor->bits->width * 4);

    XPutImage (xdisplay, xpixmap, xgc, ximage,
	       0, 0, 0, 0, pCursor->bits->width, pCursor->bits->height);

    XFree (ximage);
    XFreeGC (xdisplay, xgc);

    xformat = XRenderFindStandardFormat (xdisplay, PictStandardARGB32);
    xpicture = XRenderCreatePicture (xdisplay, xpixmap, xformat, 0, 0);

    cursor = XRenderCreateCursor (xdisplay, xpicture,
				  pCursor->bits->xhot,
				  pCursor->bits->yhot);

    XRenderFreePicture (xdisplay, xpicture);
    XFreePixmap (xdisplay, xpixmap);

    return cursor;
}

#endif
