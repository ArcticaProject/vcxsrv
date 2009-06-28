/*
 * Copyright © 2002 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#endif

#include "xf86.h"
#include "os.h"
#include "mibank.h"
#include "globals.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86DDC.h"
#include "mipointer.h"
#include "windowstr.h"
#include <randrstr.h>
#include <X11/extensions/render.h>

#include "xf86Crtc.h"
#include "xf86RandR12.h"

typedef struct _xf86RandR12Info {
    int				    virtualX;
    int				    virtualY;
    int				    mmWidth;
    int				    mmHeight;
    int				    maxX;
    int				    maxY;
    Rotation			    rotation; /* current mode */
    Rotation                        supported_rotations; /* driver supported */
} XF86RandRInfoRec, *XF86RandRInfoPtr;

#ifdef RANDR_12_INTERFACE
static Bool xf86RandR12Init12 (ScreenPtr pScreen);
static Bool xf86RandR12CreateScreenResources12 (ScreenPtr pScreen);
#endif

static int xf86RandR12Generation;
static DevPrivateKey xf86RandR12Key;

#define XF86RANDRINFO(p) ((XF86RandRInfoPtr) \
    dixLookupPrivate(&(p)->devPrivates, xf86RandR12Key))

static int
xf86RandR12ModeRefresh (DisplayModePtr mode)
{
    if (mode->VRefresh)
	return (int) (mode->VRefresh + 0.5);
    else
	return (int) (mode->Clock * 1000.0 / mode->HTotal / mode->VTotal + 0.5);
}

static Bool
xf86RandR12GetInfo (ScreenPtr pScreen, Rotation *rotations)
{
    RRScreenSizePtr	    pSize;
    ScrnInfoPtr		    scrp = XF86SCRNINFO(pScreen);
    XF86RandRInfoPtr	    randrp = XF86RANDRINFO(pScreen);
    DisplayModePtr	    mode;
    int			    refresh0 = 60;
    int			    maxX = 0, maxY = 0;

    *rotations = randrp->supported_rotations;

    if (randrp->virtualX == -1 || randrp->virtualY == -1)
    {
	randrp->virtualX = scrp->virtualX;
	randrp->virtualY = scrp->virtualY;
    }

    /* Re-probe the outputs for new monitors or modes */
    if (scrp->vtSema)
    {
	xf86ProbeOutputModes (scrp, 0, 0);
	xf86SetScrnInfoModes (scrp);
	xf86DiDGAReInit (pScreen);
    }

    for (mode = scrp->modes; ; mode = mode->next)
    {
	int refresh = xf86RandR12ModeRefresh (mode);
	if (randrp->maxX == 0 || randrp->maxY == 0)
	{
		if (maxX < mode->HDisplay)
			maxX = mode->HDisplay;
		if (maxY < mode->VDisplay)
			maxY = mode->VDisplay;
	}
	if (mode == scrp->modes)
	    refresh0 = refresh;
	pSize = RRRegisterSize (pScreen,
				mode->HDisplay, mode->VDisplay,
				randrp->mmWidth, randrp->mmHeight);
	if (!pSize)
	    return FALSE;
	RRRegisterRate (pScreen, pSize, refresh);

	if (xf86ModesEqual(mode, scrp->currentMode))
	{
	    RRSetCurrentConfig (pScreen, randrp->rotation, refresh, pSize);
	}
	if (mode->next == scrp->modes)
	    break;
    }

    if (randrp->maxX == 0 || randrp->maxY == 0)
    {
	randrp->maxX = maxX;
	randrp->maxY = maxY;
    }

    return TRUE;
}

static Bool
xf86RandR12SetMode (ScreenPtr	    pScreen,
		  DisplayModePtr    mode,
		  Bool		    useVirtual,
		  int		    mmWidth,
		  int		    mmHeight)
{
    ScrnInfoPtr		scrp = XF86SCRNINFO(pScreen);
    XF86RandRInfoPtr	randrp = XF86RANDRINFO(pScreen);
    int			oldWidth = pScreen->width;
    int			oldHeight = pScreen->height;
    int			oldmmWidth = pScreen->mmWidth;
    int			oldmmHeight = pScreen->mmHeight;
    WindowPtr		pRoot = WindowTable[pScreen->myNum];
    DisplayModePtr      currentMode = NULL;
    Bool 		ret = TRUE;
    PixmapPtr 		pspix = NULL;

    if (pRoot)
	(*scrp->EnableDisableFBAccess) (pScreen->myNum, FALSE);
    if (useVirtual)
    {
	scrp->virtualX = randrp->virtualX;
	scrp->virtualY = randrp->virtualY;
    }
    else
    {
	scrp->virtualX = mode->HDisplay;
	scrp->virtualY = mode->VDisplay;
    }

    if(randrp->rotation & (RR_Rotate_90 | RR_Rotate_270))
    {
	/* If the screen is rotated 90 or 270 degrees, swap the sizes. */
	pScreen->width = scrp->virtualY;
	pScreen->height = scrp->virtualX;
	pScreen->mmWidth = mmHeight;
	pScreen->mmHeight = mmWidth;
    }
    else
    {
	pScreen->width = scrp->virtualX;
	pScreen->height = scrp->virtualY;
	pScreen->mmWidth = mmWidth;
	pScreen->mmHeight = mmHeight;
    }
    if (scrp->currentMode == mode) {
        /* Save current mode */
        currentMode = scrp->currentMode;
        /* Reset, just so we ensure the drivers SwitchMode is called */
        scrp->currentMode = NULL;
    }
    /*
     * We know that if the driver failed to SwitchMode to the rotated
     * version, then it should revert back to it's prior mode.
     */
    if (!xf86SwitchMode (pScreen, mode))
    {
        ret = FALSE;
	scrp->virtualX = pScreen->width = oldWidth;
	scrp->virtualY = pScreen->height = oldHeight;
	pScreen->mmWidth = oldmmWidth;
	pScreen->mmHeight = oldmmHeight;
        scrp->currentMode = currentMode;
    }
    /*
     * Get the new Screen pixmap ptr as SwitchMode might have called
     * ModifyPixmapHeader and xf86EnableDisableFBAccess will put it back...
     * Unfortunately.
     */
    pspix = (*pScreen->GetScreenPixmap) (pScreen);
    if (pspix->devPrivate.ptr)
       scrp->pixmapPrivate = pspix->devPrivate;

    /*
     * Make sure the layout is correct
     */
    xf86ReconfigureLayout();

    /*
     * Make sure the whole screen is visible
     */
    xf86SetViewport (pScreen, pScreen->width, pScreen->height);
    xf86SetViewport (pScreen, 0, 0);
    if (pRoot)
	(*scrp->EnableDisableFBAccess) (pScreen->myNum, TRUE);
    return ret;
}

_X_EXPORT Bool
xf86RandR12SetConfig (ScreenPtr		pScreen,
		    Rotation		rotation,
		    int			rate,
		    RRScreenSizePtr	pSize)
{
    ScrnInfoPtr		scrp = XF86SCRNINFO(pScreen);
    XF86RandRInfoPtr	randrp = XF86RANDRINFO(pScreen);
    DisplayModePtr	mode;
    int			px, py;
    Bool		useVirtual = FALSE;
    int			maxX = 0, maxY = 0;
    Rotation		oldRotation = randrp->rotation;

    randrp->rotation = rotation;

    if (randrp->virtualX == -1 || randrp->virtualY == -1)
    {
	randrp->virtualX = scrp->virtualX;
	randrp->virtualY = scrp->virtualY;
    }

    miPointerPosition (&px, &py);
    for (mode = scrp->modes; ; mode = mode->next)
    {
	if (randrp->maxX == 0 || randrp->maxY == 0)
	{
		if (maxX < mode->HDisplay)
			maxX = mode->HDisplay;
		if (maxY < mode->VDisplay)
			maxY = mode->VDisplay;
	}
	if (mode->HDisplay == pSize->width &&
	    mode->VDisplay == pSize->height &&
	    (rate == 0 || xf86RandR12ModeRefresh (mode) == rate))
	    break;
	if (mode->next == scrp->modes)
	{
	    if (pSize->width == randrp->virtualX &&
		pSize->height == randrp->virtualY)
	    {
		mode = scrp->modes;
		useVirtual = TRUE;
		break;
	    }
    	    if (randrp->maxX == 0 || randrp->maxY == 0)
    	    {
		randrp->maxX = maxX;
		randrp->maxY = maxY;
    	    }
	    return FALSE;
	}
    }

    if (randrp->maxX == 0 || randrp->maxY == 0)
    {
	randrp->maxX = maxX;
	randrp->maxY = maxY;
    }

    if (!xf86RandR12SetMode (pScreen, mode, useVirtual, pSize->mmWidth,
			   pSize->mmHeight)) {
        randrp->rotation = oldRotation;
	return FALSE;
    }

    /*
     * Move the cursor back where it belongs; SwitchMode repositions it
     */
    if (pScreen == miPointerCurrentScreen ())
    {
        px = (px >= pScreen->width ? (pScreen->width - 1) : px);
        py = (py >= pScreen->height ? (pScreen->height - 1) : py);

	xf86SetViewport(pScreen, px, py);

	(*pScreen->SetCursorPosition) (pScreen, px, py, FALSE);
    }

    return TRUE;
}

static Bool
xf86RandR12ScreenSetSize (ScreenPtr	pScreen,
			CARD16		width,
			CARD16		height,
			CARD32		mmWidth,
			CARD32		mmHeight)
{
    XF86RandRInfoPtr	randrp = XF86RANDRINFO(pScreen);
    ScrnInfoPtr		pScrn = XF86SCRNINFO(pScreen);
    xf86CrtcConfigPtr	config = XF86_CRTC_CONFIG_PTR(pScrn);
    WindowPtr		pRoot = WindowTable[pScreen->myNum];
    PixmapPtr		pScrnPix = (*pScreen->GetScreenPixmap)(pScreen);
    Bool		ret = FALSE;

    if (xf86RandR12Key) {
        if (randrp->virtualX == -1 || randrp->virtualY == -1)
        {
	    randrp->virtualX = pScrn->virtualX;
	    randrp->virtualY = pScrn->virtualY;
        }
    }
    if (pRoot && pScrn->vtSema)
	(*pScrn->EnableDisableFBAccess) (pScreen->myNum, FALSE);

    /* Let the driver update virtualX and virtualY */
    if (!(*config->funcs->resize)(pScrn, width, height))
	goto finish;

    ret = TRUE;

    pScreen->width = pScrnPix->drawable.width = width;
    pScreen->height = pScrnPix->drawable.height = height;
    randrp->mmWidth = pScreen->mmWidth = mmWidth;
    randrp->mmHeight = pScreen->mmHeight = mmHeight;

    xf86SetViewport (pScreen, pScreen->width-1, pScreen->height-1);
    xf86SetViewport (pScreen, 0, 0);

finish:
    if (pRoot && pScrn->vtSema)
	(*pScrn->EnableDisableFBAccess) (pScreen->myNum, TRUE);
#if RANDR_12_INTERFACE
    if (xf86RandR12Key && WindowTable[pScreen->myNum] && ret)
	RRScreenSizeNotify (pScreen);
#endif
    return ret;
}

_X_EXPORT Rotation
xf86RandR12GetRotation(ScreenPtr pScreen)
{
    XF86RandRInfoPtr	    randrp = XF86RANDRINFO(pScreen);

    return randrp->rotation;
}

_X_EXPORT Bool
xf86RandR12CreateScreenResources (ScreenPtr pScreen)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   config = XF86_CRTC_CONFIG_PTR(pScrn);
    XF86RandRInfoPtr	randrp = XF86RANDRINFO(pScreen);
    int			c;
    int			width, height;
    int			mmWidth, mmHeight;
#ifdef PANORAMIX
    /* XXX disable RandR when using Xinerama */
    if (!noPanoramiXExtension)
	return TRUE;
#endif

    /*
     * Compute size of screen
     */
    width = 0; height = 0;
    for (c = 0; c < config->num_crtc; c++)
    {
	xf86CrtcPtr crtc = config->crtc[c];
	int	    crtc_width = crtc->x + xf86ModeWidth (&crtc->mode, crtc->rotation);
	int	    crtc_height = crtc->y + xf86ModeHeight (&crtc->mode, crtc->rotation);
	
	if (crtc->enabled && crtc_width > width)
	    width = crtc_width;
	if (crtc->enabled && crtc_height > height)
	    height = crtc_height;
    }
    
    if (width && height)
    {
	/*
	 * Compute physical size of screen
	 */
	if (monitorResolution) 
	{
	    mmWidth = width * 25.4 / monitorResolution;
	    mmHeight = height * 25.4 / monitorResolution;
	}
	else
	{
	    xf86OutputPtr   output = config->output[config->compat_output];
	    xf86CrtcPtr	    crtc = output->crtc;

	    if (output->conf_monitor &&
		(output->conf_monitor->mon_width  > 0 &&
		 output->conf_monitor->mon_height > 0))
	    {
		/*
		 * Prefer user configured DisplaySize
		 */
		mmWidth = output->conf_monitor->mon_width;
		mmHeight = output->conf_monitor->mon_height;
	    }
	    else if (crtc && crtc->mode.HDisplay &&
		     output->mm_width && output->mm_height)
	    {
		/*
		 * If the output has a mode and a declared size, use that
		 * to scale the screen size
		 */
		DisplayModePtr	mode = &crtc->mode;
		mmWidth = output->mm_width * width / mode->HDisplay;
		mmHeight = output->mm_height * height / mode->VDisplay;
	    }
	    else
	    {
		/*
		 * Otherwise, just set the screen to DEFAULT_DPI
		 */
		mmWidth = width * 25.4 / DEFAULT_DPI;
		mmHeight = height * 25.4 / DEFAULT_DPI;
	    }
	}
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "Setting screen physical size to %d x %d\n",
		   mmWidth, mmHeight);
	xf86RandR12ScreenSetSize (pScreen,
				  width,
				  height,
				  mmWidth,
				  mmHeight);
    }

    if (xf86RandR12Key == NULL)
	return TRUE;

    if (randrp->virtualX == -1 || randrp->virtualY == -1)
    {
	randrp->virtualX = pScrn->virtualX;
	randrp->virtualY = pScrn->virtualY;
    }
    xf86CrtcSetScreenSubpixelOrder (pScreen);
#if RANDR_12_INTERFACE
    if (xf86RandR12CreateScreenResources12 (pScreen))
	return TRUE;
#endif
    return TRUE;
}


_X_EXPORT Bool
xf86RandR12Init (ScreenPtr pScreen)
{
    rrScrPrivPtr	rp;
    XF86RandRInfoPtr	randrp;

#ifdef PANORAMIX
    /* XXX disable RandR when using Xinerama */
    if (!noPanoramiXExtension)
	return TRUE;
#endif

    if (xf86RandR12Generation != serverGeneration)
	xf86RandR12Generation = serverGeneration;

    xf86RandR12Key = &xf86RandR12Key;

    randrp = xalloc (sizeof (XF86RandRInfoRec));
    if (!randrp)
	return FALSE;

    if (!RRScreenInit(pScreen))
    {
	xfree (randrp);
	return FALSE;
    }
    rp = rrGetScrPriv(pScreen);
    rp->rrGetInfo = xf86RandR12GetInfo;
    rp->rrSetConfig = xf86RandR12SetConfig;

    randrp->virtualX = -1;
    randrp->virtualY = -1;
    randrp->mmWidth = pScreen->mmWidth;
    randrp->mmHeight = pScreen->mmHeight;

    randrp->rotation = RR_Rotate_0; /* initial rotated mode */

    randrp->supported_rotations = RR_Rotate_0;

    randrp->maxX = randrp->maxY = 0;

    dixSetPrivate(&pScreen->devPrivates, xf86RandR12Key, randrp);

#if RANDR_12_INTERFACE
    if (!xf86RandR12Init12 (pScreen))
	return FALSE;
#endif
    return TRUE;
}

_X_EXPORT void
xf86RandR12SetRotations (ScreenPtr pScreen, Rotation rotations)
{
    XF86RandRInfoPtr	randrp;
#if RANDR_12_INTERFACE
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    int			c;
    xf86CrtcConfigPtr   config = XF86_CRTC_CONFIG_PTR(pScrn);
#endif

    if (xf86RandR12Key == NULL)
	return;

    randrp = XF86RANDRINFO(pScreen);
#if RANDR_12_INTERFACE
    for (c = 0; c < config->num_crtc; c++) {
	xf86CrtcPtr    crtc = config->crtc[c];

	RRCrtcSetRotations (crtc->randr_crtc, rotations);
    }
#endif
    randrp->supported_rotations = rotations;
}

_X_EXPORT void
xf86RandR12GetOriginalVirtualSize(ScrnInfoPtr pScrn, int *x, int *y)
{
    ScreenPtr pScreen = screenInfo.screens[pScrn->scrnIndex];

    if (xf86RandR12Generation != serverGeneration ||
	XF86RANDRINFO(pScreen)->virtualX == -1)
    {
	*x = pScrn->virtualX;
	*y = pScrn->virtualY;
    } else {
	XF86RandRInfoPtr randrp = XF86RANDRINFO(pScreen);

	*x = randrp->virtualX;
	*y = randrp->virtualY;
    }
}

#if RANDR_12_INTERFACE

#define FLAG_BITS (RR_HSyncPositive | \
		   RR_HSyncNegative | \
		   RR_VSyncPositive | \
		   RR_VSyncNegative | \
		   RR_Interlace | \
		   RR_DoubleScan | \
		   RR_CSync | \
		   RR_CSyncPositive | \
		   RR_CSyncNegative | \
		   RR_HSkewPresent | \
		   RR_BCast | \
		   RR_PixelMultiplex | \
		   RR_DoubleClock | \
		   RR_ClockDivideBy2)

static Bool
xf86RandRModeMatches (RRModePtr		randr_mode,
		      DisplayModePtr	mode)
{
#if 0
    if (match_name)
    {
	/* check for same name */
	int	len = strlen (mode->name);
	if (randr_mode->mode.nameLength != len)			return FALSE;
	if (memcmp (randr_mode->name, mode->name, len) != 0)	return FALSE;
    }
#endif
    
    /* check for same timings */
    if (randr_mode->mode.dotClock / 1000 != mode->Clock)    return FALSE;
    if (randr_mode->mode.width        != mode->HDisplay)    return FALSE;
    if (randr_mode->mode.hSyncStart   != mode->HSyncStart)  return FALSE;
    if (randr_mode->mode.hSyncEnd     != mode->HSyncEnd)    return FALSE;
    if (randr_mode->mode.hTotal       != mode->HTotal)	    return FALSE;
    if (randr_mode->mode.hSkew        != mode->HSkew)	    return FALSE;
    if (randr_mode->mode.height       != mode->VDisplay)    return FALSE;
    if (randr_mode->mode.vSyncStart   != mode->VSyncStart)  return FALSE;
    if (randr_mode->mode.vSyncEnd     != mode->VSyncEnd)    return FALSE;
    if (randr_mode->mode.vTotal       != mode->VTotal)	    return FALSE;
    
    /* check for same flags (using only the XF86 valid flag bits) */
    if ((randr_mode->mode.modeFlags & FLAG_BITS) != (mode->Flags & FLAG_BITS))
	return FALSE;
    
    /* everything matches */
    return TRUE;
}

static Bool
xf86RandR12CrtcNotify (RRCrtcPtr	randr_crtc)
{
    ScreenPtr		pScreen = randr_crtc->pScreen;
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   config = XF86_CRTC_CONFIG_PTR(pScrn);
    RRModePtr		randr_mode = NULL;
    int			x;
    int			y;
    Rotation		rotation;
    int			numOutputs;
    RROutputPtr		*randr_outputs;
    RROutputPtr		randr_output;
    xf86CrtcPtr		crtc = randr_crtc->devPrivate;
    xf86OutputPtr	output;
    int			i, j;
    DisplayModePtr	mode = &crtc->mode;
    Bool		ret;

    randr_outputs = xalloc(config->num_output * sizeof (RROutputPtr));
    if (!randr_outputs)
	return FALSE;
    x = crtc->x;
    y = crtc->y;
    rotation = crtc->rotation;
    numOutputs = 0;
    randr_mode = NULL;
    for (i = 0; i < config->num_output; i++)
    {
	output = config->output[i];
	if (output->crtc == crtc)
	{
	    randr_output = output->randr_output;
	    randr_outputs[numOutputs++] = randr_output;
	    /*
	     * We make copies of modes, so pointer equality 
	     * isn't sufficient
	     */
	    for (j = 0; j < randr_output->numModes + randr_output->numUserModes; j++)
	    {
		RRModePtr   m = (j < randr_output->numModes ?
				 randr_output->modes[j] :
				 randr_output->userModes[j-randr_output->numModes]);
					 
		if (xf86RandRModeMatches (m, mode))
		{
		    randr_mode = m;
		    break;
		}
	    }
	}
    }
    ret = RRCrtcNotify (randr_crtc, randr_mode, x, y,
			rotation, numOutputs, randr_outputs);
    xfree(randr_outputs);
    return ret;
}

/*
 * Convert a RandR mode to a DisplayMode
 */
static void
xf86RandRModeConvert (ScrnInfoPtr	scrn,
		      RRModePtr		randr_mode,
		      DisplayModePtr	mode)
{
    memset(mode, 0, sizeof(DisplayModeRec));
    mode->status = MODE_OK;

    mode->Clock = randr_mode->mode.dotClock / 1000;
    
    mode->HDisplay = randr_mode->mode.width;
    mode->HSyncStart = randr_mode->mode.hSyncStart;
    mode->HSyncEnd = randr_mode->mode.hSyncEnd;
    mode->HTotal = randr_mode->mode.hTotal;
    mode->HSkew = randr_mode->mode.hSkew;
    
    mode->VDisplay = randr_mode->mode.height;
    mode->VSyncStart = randr_mode->mode.vSyncStart;
    mode->VSyncEnd = randr_mode->mode.vSyncEnd;
    mode->VTotal = randr_mode->mode.vTotal;
    mode->VScan = 0;

    mode->Flags = randr_mode->mode.modeFlags & FLAG_BITS;

    xf86SetModeCrtc (mode, scrn->adjustFlags);
}

static Bool
xf86RandR12CrtcSet (ScreenPtr	pScreen,
		  RRCrtcPtr	randr_crtc,
		  RRModePtr	randr_mode,
		  int		x,
		  int		y,
		  Rotation	rotation,
		  int		num_randr_outputs,
		  RROutputPtr	*randr_outputs)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   config = XF86_CRTC_CONFIG_PTR(pScrn);
    xf86CrtcPtr		crtc = randr_crtc->devPrivate;
    Bool		changed = FALSE;
    int			o, ro;
    xf86CrtcPtr		*save_crtcs;
    Bool		save_enabled = crtc->enabled;

    if (!crtc->scrn->vtSema)
	return FALSE;

    save_crtcs = xalloc(config->num_output * sizeof (xf86CrtcPtr));
    if ((randr_mode != NULL) != crtc->enabled)
	changed = TRUE;
    else if (randr_mode && !xf86RandRModeMatches (randr_mode, &crtc->mode))
	changed = TRUE;
    
    if (rotation != crtc->rotation)
	changed = TRUE;

    if (x != crtc->x || y != crtc->y)
	changed = TRUE;
    for (o = 0; o < config->num_output; o++) 
    {
	xf86OutputPtr  output = config->output[o];
	xf86CrtcPtr    new_crtc;

	save_crtcs[o] = output->crtc;
	
	if (output->crtc == crtc)
	    new_crtc = NULL;
	else
	    new_crtc = output->crtc;
	for (ro = 0; ro < num_randr_outputs; ro++) 
	    if (output->randr_output == randr_outputs[ro])
	    {
		new_crtc = crtc;
		break;
	    }
	if (new_crtc != output->crtc)
	{
	    changed = TRUE;
	    output->crtc = new_crtc;
	}
    }
    for (ro = 0; ro < num_randr_outputs; ro++) 
        if (randr_outputs[ro]->pendingProperties)
	    changed = TRUE;

    /* XXX need device-independent mode setting code through an API */
    if (changed)
    {
	crtc->enabled = randr_mode != NULL;

	if (randr_mode)
	{
	    DisplayModeRec  mode;

	    xf86RandRModeConvert (pScrn, randr_mode, &mode);
	    if (!xf86CrtcSetMode (crtc, &mode, rotation, x, y))
	    {
		crtc->enabled = save_enabled;
		for (o = 0; o < config->num_output; o++)
		{
		    xf86OutputPtr	output = config->output[o];
		    output->crtc = save_crtcs[o];
		}
		xfree(save_crtcs);
		return FALSE;
	    }
	    /*
	     * Save the last successful setting for EnterVT
	     */
	    crtc->desiredMode = mode;
	    crtc->desiredRotation = rotation;
	    crtc->desiredX = x;
	    crtc->desiredY = y;
	}
	xf86DisableUnusedFunctions (pScrn);
    }
    xfree(save_crtcs);
    return xf86RandR12CrtcNotify (randr_crtc);
}

static Bool
xf86RandR12CrtcSetGamma (ScreenPtr    pScreen,
			 RRCrtcPtr    randr_crtc)
{
    xf86CrtcPtr		crtc = randr_crtc->devPrivate;

    if (crtc->funcs->gamma_set == NULL)
	return FALSE;

    if (!crtc->scrn->vtSema)
	return TRUE;

    crtc->funcs->gamma_set(crtc, randr_crtc->gammaRed, randr_crtc->gammaGreen,
			   randr_crtc->gammaBlue, randr_crtc->gammaSize);

    return TRUE;
}

static Bool
xf86RandR12OutputSetProperty (ScreenPtr pScreen,
			      RROutputPtr randr_output,
			      Atom property,
			      RRPropertyValuePtr value)
{
    xf86OutputPtr output = randr_output->devPrivate;

    /* If we don't have any property handler, then we don't care what the
     * user is setting properties to.
     */
    if (output->funcs->set_property == NULL)
	return TRUE;

    /*
     * This function gets called even when vtSema is FALSE, as
     * drivers will need to remember the correct value to apply
     * when the VT switch occurs
     */
    return output->funcs->set_property(output, property, value);
}

static Bool
xf86RandR12OutputValidateMode (ScreenPtr    pScreen,
			       RROutputPtr  randr_output,
			       RRModePtr    randr_mode)
{
    ScrnInfoPtr	    pScrn = xf86Screens[pScreen->myNum];
    xf86OutputPtr   output = randr_output->devPrivate;
    DisplayModeRec  mode;

    xf86RandRModeConvert (pScrn, randr_mode, &mode);
    /*
     * This function may be called when vtSema is FALSE, so
     * the underlying function must either avoid touching the hardware
     * or return FALSE when vtSema is FALSE
     */
    if (output->funcs->mode_valid (output, &mode) != MODE_OK)
	return FALSE;
    return TRUE;
}

static void
xf86RandR12ModeDestroy (ScreenPtr pScreen, RRModePtr randr_mode)
{
}

/**
 * Given a list of xf86 modes and a RandR Output object, construct
 * RandR modes and assign them to the output
 */
static Bool
xf86RROutputSetModes (RROutputPtr randr_output, DisplayModePtr modes)
{
    DisplayModePtr  mode;
    RRModePtr	    *rrmodes = NULL;
    int		    nmode = 0;
    int		    npreferred = 0;
    Bool	    ret = TRUE;
    int		    pref;

    for (mode = modes; mode; mode = mode->next)
	nmode++;

    if (nmode) {
	rrmodes = xalloc (nmode * sizeof (RRModePtr));
	
	if (!rrmodes)
	    return FALSE;
	nmode = 0;

	for (pref = 1; pref >= 0; pref--) {
	    for (mode = modes; mode; mode = mode->next) {
		if ((pref != 0) == ((mode->type & M_T_PREFERRED) != 0)) {
		    xRRModeInfo		modeInfo;
		    RRModePtr		rrmode;
		    
		    modeInfo.nameLength = strlen (mode->name);
		    modeInfo.width = mode->HDisplay;
		    modeInfo.dotClock = mode->Clock * 1000;
		    modeInfo.hSyncStart = mode->HSyncStart;
		    modeInfo.hSyncEnd = mode->HSyncEnd;
		    modeInfo.hTotal = mode->HTotal;
		    modeInfo.hSkew = mode->HSkew;

		    modeInfo.height = mode->VDisplay;
		    modeInfo.vSyncStart = mode->VSyncStart;
		    modeInfo.vSyncEnd = mode->VSyncEnd;
		    modeInfo.vTotal = mode->VTotal;
		    modeInfo.modeFlags = mode->Flags;

		    rrmode = RRModeGet (&modeInfo, mode->name);
		    if (rrmode) {
			rrmodes[nmode++] = rrmode;
			npreferred += pref;
		    }
		}
	    }
	}
    }
    
    ret = RROutputSetModes (randr_output, rrmodes, nmode, npreferred);
    xfree (rrmodes);
    return ret;
}

/*
 * Mirror the current mode configuration to RandR
 */
static Bool
xf86RandR12SetInfo12 (ScreenPtr pScreen)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   config = XF86_CRTC_CONFIG_PTR(pScrn);
    RROutputPtr		*clones;
    RRCrtcPtr		*crtcs;
    int			ncrtc;
    int			o, c, l;
    RRCrtcPtr		randr_crtc;
    int			nclone;
    
    clones = xalloc(config->num_output * sizeof (RROutputPtr));
    crtcs = xalloc (config->num_crtc * sizeof (RRCrtcPtr));
    for (o = 0; o < config->num_output; o++)
    {
	xf86OutputPtr	output = config->output[o];
	
	ncrtc = 0;
	for (c = 0; c < config->num_crtc; c++)
	    if (output->possible_crtcs & (1 << c))
		crtcs[ncrtc++] = config->crtc[c]->randr_crtc;

	if (output->crtc)
	    randr_crtc = output->crtc->randr_crtc;
	else
	    randr_crtc = NULL;

	if (!RROutputSetCrtcs (output->randr_output, crtcs, ncrtc))
	{
	    xfree (crtcs);
	    xfree (clones);
	    return FALSE;
	}

	RROutputSetPhysicalSize(output->randr_output, 
				output->mm_width,
				output->mm_height);
	xf86RROutputSetModes (output->randr_output, output->probed_modes);

	switch (output->status) {
	case XF86OutputStatusConnected:
	    RROutputSetConnection (output->randr_output, RR_Connected);
	    break;
	case XF86OutputStatusDisconnected:
	    RROutputSetConnection (output->randr_output, RR_Disconnected);
	    break;
	case XF86OutputStatusUnknown:
	    RROutputSetConnection (output->randr_output, RR_UnknownConnection);
	    break;
	}

	RROutputSetSubpixelOrder (output->randr_output, output->subpixel_order);

	/*
	 * Valid clones
	 */
	nclone = 0;
	for (l = 0; l < config->num_output; l++)
	{
	    xf86OutputPtr	    clone = config->output[l];
	    
	    if (l != o && (output->possible_clones & (1 << l)))
		clones[nclone++] = clone->randr_output;
	}
	if (!RROutputSetClones (output->randr_output, clones, nclone))
	{
	    xfree (crtcs);
	    xfree (clones);
	    return FALSE;
	}
    }
    xfree (crtcs);
    xfree (clones);
    return TRUE;
}



/*
 * Query the hardware for the current state, then mirror
 * that to RandR
 */
static Bool
xf86RandR12GetInfo12 (ScreenPtr pScreen, Rotation *rotations)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];

    if (!pScrn->vtSema)
	return TRUE;
    xf86ProbeOutputModes (pScrn, 0, 0);
    xf86SetScrnInfoModes (pScrn);
    xf86DiDGAReInit (pScreen);
    return xf86RandR12SetInfo12 (pScreen);
}

static Bool
xf86RandR12CreateObjects12 (ScreenPtr pScreen)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   config = XF86_CRTC_CONFIG_PTR(pScrn);
    int			c;
    int			o;
    
    if (!RRInit ())
	return FALSE;

    /*
     * Configure crtcs
     */
    for (c = 0; c < config->num_crtc; c++)
    {
	xf86CrtcPtr    crtc = config->crtc[c];
	
	crtc->randr_crtc = RRCrtcCreate (pScreen, crtc);
	RRCrtcGammaSetSize (crtc->randr_crtc, 256);
    }
    /*
     * Configure outputs
     */
    for (o = 0; o < config->num_output; o++)
    {
	xf86OutputPtr	output = config->output[o];

	output->randr_output = RROutputCreate (pScreen, output->name, 
					       strlen (output->name),
					       output);

	if (output->funcs->create_resources != NULL)
	    output->funcs->create_resources(output);
	RRPostPendingProperties (output->randr_output);
    }
    return TRUE;
}

static Bool
xf86RandR12CreateScreenResources12 (ScreenPtr pScreen)
{
    int			c;
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   config = XF86_CRTC_CONFIG_PTR(pScrn);

    if (xf86RandR12Key == NULL)
	return TRUE;

    for (c = 0; c < config->num_crtc; c++)
        xf86RandR12CrtcNotify (config->crtc[c]->randr_crtc);
    
    RRScreenSetSizeRange (pScreen, config->minWidth, config->minHeight,
			  config->maxWidth, config->maxHeight);
    return TRUE;
}

/*
 * Something happened within the screen configuration due
 * to DGA, VidMode or hot key. Tell RandR
 */

_X_EXPORT void
xf86RandR12TellChanged (ScreenPtr pScreen)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   config = XF86_CRTC_CONFIG_PTR(pScrn);
    int			c;

    if (xf86RandR12Key == NULL)
	return;

    xf86RandR12SetInfo12 (pScreen);
    for (c = 0; c < config->num_crtc; c++)
	xf86RandR12CrtcNotify (config->crtc[c]->randr_crtc);

    RRTellChanged (pScreen);
}

static void
xf86RandR12PointerMoved (int scrnIndex, int x, int y)
{
}

static Bool
xf86RandR12Init12 (ScreenPtr pScreen)
{
    ScrnInfoPtr		pScrn = xf86Screens[pScreen->myNum];
    rrScrPrivPtr	rp = rrGetScrPriv(pScreen);

    rp->rrGetInfo = xf86RandR12GetInfo12;
    rp->rrScreenSetSize = xf86RandR12ScreenSetSize;
    rp->rrCrtcSet = xf86RandR12CrtcSet;
    rp->rrCrtcSetGamma = xf86RandR12CrtcSetGamma;
    rp->rrOutputSetProperty = xf86RandR12OutputSetProperty;
    rp->rrOutputValidateMode = xf86RandR12OutputValidateMode;
    rp->rrModeDestroy = xf86RandR12ModeDestroy;
    rp->rrSetConfig = NULL;
    pScrn->PointerMoved = xf86RandR12PointerMoved;
    if (!xf86RandR12CreateObjects12 (pScreen))
	return FALSE;

    /*
     * Configure output modes
     */
    if (!xf86RandR12SetInfo12 (pScreen))
	return FALSE;
    return TRUE;
}

#endif

_X_EXPORT Bool
xf86RandR12PreInit (ScrnInfoPtr pScrn)
{
    return TRUE;
}
