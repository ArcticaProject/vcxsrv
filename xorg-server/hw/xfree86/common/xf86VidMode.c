/*
 * Copyright (c) 1999-2003 by The XFree86 Project, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

/*
 * This file contains the VidMode functions required by the extension.
 * These have been added to avoid the need for the higher level extension
 * code to access the private XFree86 data structures directly. Wherever
 * possible this code uses the functions in xf86Mode.c to do the work,
 * so that two version of code that do similar things don't have to be
 * maintained.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

#ifdef XF86VIDMODE
#include "vidmodeproc.h"
#include "xf86cmap.h"

static int VidModeKeyIndex;
static DevPrivateKey VidModeKey;
static int VidModeCount = 0;
static Bool VidModeClose(int i, ScreenPtr pScreen);

#define VMPTR(p) ((VidModePtr)dixLookupPrivate(&(p)->devPrivates, VidModeKey))

#endif

#ifdef DEBUG
# define DEBUG_P(x) ErrorF(x"\n");
#else
# define DEBUG_P(x) /**/
#endif

_X_EXPORT Bool
VidModeExtensionInit(ScreenPtr pScreen)
{
#ifdef XF86VIDMODE
    VidModePtr pVidMode;
    
    if (!xf86GetVidModeEnabled()) {
	DEBUG_P("!xf86GetVidModeEnabled()");
	return FALSE;
    }

    VidModeKey = &VidModeKeyIndex;

    if (!dixSetPrivate(&pScreen->devPrivates, VidModeKey,
		       xcalloc(sizeof(VidModeRec), 1))) {
	DEBUG_P("xcalloc failed");
	return FALSE;
    }

    pVidMode = VMPTR(pScreen);
    pVidMode->Flags = 0;
    pVidMode->Next = NULL;
    pVidMode->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = VidModeClose;
    VidModeCount++;
    return TRUE;
#else
    DEBUG_P("no vidmode extension");
    return FALSE;
#endif
}


#ifdef XF86VIDMODE

static Bool
VidModeClose(int i, ScreenPtr pScreen)
{
    VidModePtr pVidMode = VMPTR(pScreen);

    /* This shouldn't happen */
    if (!pVidMode)
	return FALSE;

    pScreen->CloseScreen = pVidMode->CloseScreen;

    if (--VidModeCount == 0) {
	xfree(dixLookupPrivate(&pScreen->devPrivates, VidModeKey));
	dixSetPrivate(&pScreen->devPrivates, VidModeKey, NULL);
	VidModeKey = NULL;
    }
    return pScreen->CloseScreen(i, pScreen);
}

Bool
VidModeAvailable(int scrnIndex)
{
    ScrnInfoPtr pScrn;
    VidModePtr pVidMode;

    if (VidModeKey == NULL) {
	DEBUG_P("VidModeKey == NULL");
	return FALSE;
    }
 
    pScrn = xf86Screens[scrnIndex];
    if (pScrn == NULL) {
	DEBUG_P("pScrn == NULL");
	return FALSE;
    }
    
    pVidMode = VMPTR(pScrn->pScreen);
    if (pVidMode)
	return TRUE;
    else {
	DEBUG_P("pVidMode == NULL");
	return FALSE;
    }
}

_X_EXPORT Bool
VidModeGetCurrentModeline(int scrnIndex, pointer *mode, int *dotClock)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];

    if (pScrn->currentMode) {
	*mode = (pointer)(pScrn->currentMode);
	*dotClock = pScrn->currentMode->Clock;

	return TRUE;
    }
    return FALSE;
}

_X_EXPORT int
VidModeGetDotClock(int scrnIndex, int Clock)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return 0;

    pScrn = xf86Screens[scrnIndex];
    if ((pScrn->progClock) || (Clock >= MAXCLOCKS))
	return Clock;
    else  
	return pScrn->clock[Clock];
}

_X_EXPORT int
VidModeGetNumOfClocks(int scrnIndex, Bool *progClock)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return 0;

    pScrn = xf86Screens[scrnIndex];
    if (pScrn->progClock){
	*progClock = TRUE;
	return 0;
    } else {
	*progClock = FALSE;
	return pScrn->numClocks;
    }
}

_X_EXPORT Bool
VidModeGetClocks(int scrnIndex, int *Clocks)
{
    ScrnInfoPtr pScrn;
    int i;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];

    if (pScrn->progClock)
	return FALSE;

    for (i = 0;  i < pScrn->numClocks;  i++)
	*Clocks++ = pScrn->clock[i];

    return TRUE;
}


_X_EXPORT Bool
VidModeGetFirstModeline(int scrnIndex, pointer *mode, int *dotClock)
{
    ScrnInfoPtr pScrn;
    VidModePtr pVidMode;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    pVidMode = VMPTR(pScrn->pScreen);
    pVidMode->First = pScrn->modes;
    pVidMode->Next =  pVidMode->First->next;

    if (pVidMode->First->status == MODE_OK) {
      *mode = (pointer)(pVidMode->First);
      *dotClock = VidModeGetDotClock(scrnIndex, pVidMode->First->Clock);
      return TRUE;
    }

    return VidModeGetNextModeline(scrnIndex, mode, dotClock);
}

_X_EXPORT Bool
VidModeGetNextModeline(int scrnIndex, pointer *mode, int *dotClock)
{
    ScrnInfoPtr pScrn;
    VidModePtr pVidMode;
    DisplayModePtr p;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    pVidMode = VMPTR(pScrn->pScreen);

    for (p = pVidMode->Next; p != NULL && p != pVidMode->First; p = p->next) {
	if (p->status == MODE_OK) {
	    pVidMode->Next = p->next;
	    *mode = (pointer)p;
	    *dotClock = VidModeGetDotClock(scrnIndex, p->Clock);
	    return TRUE;
	}
    }
    
    return FALSE;
}

_X_EXPORT Bool
VidModeDeleteModeline(int scrnIndex, pointer mode)
{
    ScrnInfoPtr pScrn;

    if ((mode == NULL) || (!VidModeAvailable(scrnIndex)))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    xf86DeleteMode(&(pScrn->modes), (DisplayModePtr)mode);
    return TRUE;
}

_X_EXPORT Bool
VidModeZoomViewport(int scrnIndex, int zoom)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    xf86ZoomViewport(pScrn->pScreen, zoom);
    return TRUE;
}

_X_EXPORT Bool
VidModeSetViewPort(int scrnIndex, int x, int y)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    pScrn->frameX0 = min( max(x, 0),
	                 pScrn->virtualX - pScrn->currentMode->HDisplay );
    pScrn->frameX1 = pScrn->frameX0 + pScrn->currentMode->HDisplay - 1;
    pScrn->frameY0 = min( max(y, 0),
	                 pScrn->virtualY - pScrn->currentMode->VDisplay );
    pScrn->frameY1 = pScrn->frameY0 + pScrn->currentMode->VDisplay - 1;
    if (pScrn->AdjustFrame != NULL)
	(pScrn->AdjustFrame)(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

    return TRUE;
}

_X_EXPORT Bool
VidModeGetViewPort(int scrnIndex, int *x, int *y)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    *x = pScrn->frameX0;
    *y = pScrn->frameY0;
    return TRUE;
}

_X_EXPORT Bool
VidModeSwitchMode(int scrnIndex, pointer mode)
{
    ScrnInfoPtr pScrn;
    DisplayModePtr pTmpMode;
    Bool retval;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    /* save in case we fail */
    pTmpMode = pScrn->currentMode;
    /* Force a mode switch */
    pScrn->currentMode = NULL;
    retval = xf86SwitchMode(pScrn->pScreen, mode);
    /* we failed: restore it */
    if (retval == FALSE)
	pScrn->currentMode = pTmpMode;
    return retval;
}

_X_EXPORT Bool
VidModeLockZoom(int scrnIndex, Bool lock)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];

    if (xf86Info.dontZoom)
	return FALSE;

    xf86LockZoom(pScrn->pScreen, lock);
    return TRUE;
}

_X_EXPORT Bool
VidModeGetMonitor(int scrnIndex, pointer *monitor)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    *monitor = (pointer)(pScrn->monitor);

    return TRUE;
}

_X_EXPORT ModeStatus
VidModeCheckModeForMonitor(int scrnIndex, pointer mode)
{
    ScrnInfoPtr pScrn;

    if ((mode == NULL) || (!VidModeAvailable(scrnIndex)))
	return MODE_ERROR;

    pScrn = xf86Screens[scrnIndex];

    return xf86CheckModeForMonitor((DisplayModePtr)mode, pScrn->monitor);
}

_X_EXPORT ModeStatus
VidModeCheckModeForDriver(int scrnIndex, pointer mode)
{
    ScrnInfoPtr pScrn;

    if ((mode == NULL) || (!VidModeAvailable(scrnIndex)))
	return MODE_ERROR;

    pScrn = xf86Screens[scrnIndex];

    return xf86CheckModeForDriver(pScrn, (DisplayModePtr)mode, 0);
}

_X_EXPORT void
VidModeSetCrtcForMode(int scrnIndex, pointer mode)
{
    ScrnInfoPtr pScrn;
    DisplayModePtr ScreenModes;
    
    if ((mode == NULL) || (!VidModeAvailable(scrnIndex)))
	return;

    /* Ugly hack so that the xf86Mode.c function can be used without change */
    pScrn = xf86Screens[scrnIndex];
    ScreenModes = pScrn->modes;
    pScrn->modes = (DisplayModePtr)mode;
    
    xf86SetCrtcForModes(pScrn, pScrn->adjustFlags);
    pScrn->modes = ScreenModes;
    return;
}

_X_EXPORT Bool
VidModeAddModeline(int scrnIndex, pointer mode)
{
    ScrnInfoPtr pScrn;
    
    if ((mode == NULL) || (!VidModeAvailable(scrnIndex)))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];

    ((DisplayModePtr)mode)->name         = strdup(""); /* freed by deletemode */
    ((DisplayModePtr)mode)->status       = MODE_OK;
    ((DisplayModePtr)mode)->next         = pScrn->modes->next;
    ((DisplayModePtr)mode)->prev         = pScrn->modes;
    pScrn->modes->next                   = (DisplayModePtr)mode;
    if( ((DisplayModePtr)mode)->next != NULL )
      ((DisplayModePtr)mode)->next->prev   = (DisplayModePtr)mode;

    return TRUE;
}

_X_EXPORT int
VidModeGetNumOfModes(int scrnIndex)
{
    pointer mode = NULL;
    int dotClock= 0, nummodes = 0;
  
    if (!VidModeGetFirstModeline(scrnIndex, &mode, &dotClock))
	return nummodes;

    do {
	nummodes++;
	if (!VidModeGetNextModeline(scrnIndex, &mode, &dotClock))
	    return nummodes;
    } while (TRUE);
}

_X_EXPORT Bool
VidModeSetGamma(int scrnIndex, float red, float green, float blue)
{
    ScrnInfoPtr pScrn;
    Gamma gamma;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    gamma.red = red;
    gamma.green = green;
    gamma.blue = blue;
    if (xf86ChangeGamma(pScrn->pScreen, gamma) != Success)
	return FALSE;
    else
	return TRUE;
}

_X_EXPORT Bool
VidModeGetGamma(int scrnIndex, float *red, float *green, float *blue)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
	return FALSE;

    pScrn = xf86Screens[scrnIndex];
    *red = pScrn->gamma.red;
    *green = pScrn->gamma.green;
    *blue = pScrn->gamma.blue;
    return TRUE;
}

_X_EXPORT Bool
VidModeSetGammaRamp(int scrnIndex, int size, CARD16 *r, CARD16 *g, CARD16 *b)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
        return FALSE;
 
    pScrn = xf86Screens[scrnIndex];
    xf86ChangeGammaRamp(pScrn->pScreen, size, r, g, b);
    return TRUE;
}

_X_EXPORT Bool
VidModeGetGammaRamp(int scrnIndex, int size, CARD16 *r, CARD16 *g, CARD16 *b)
{
    ScrnInfoPtr pScrn;

    if (!VidModeAvailable(scrnIndex))
        return FALSE;

    pScrn = xf86Screens[scrnIndex];
    xf86GetGammaRamp(pScrn->pScreen, size, r, g, b);
    return TRUE;
}

_X_EXPORT int
VidModeGetGammaRampSize(int scrnIndex)
{
    if (!VidModeAvailable(scrnIndex))
        return 0;

    return xf86GetGammaRampSize(xf86Screens[scrnIndex]->pScreen);
}

_X_EXPORT pointer
VidModeCreateMode(void)
{
    DisplayModePtr mode;
  
    mode = xalloc(sizeof(DisplayModeRec));
    if (mode != NULL) {
	mode->name          = "";
	mode->VScan         = 1;    /* divides refresh rate. default = 1 */
	mode->Private       = NULL;
	mode->next          = mode;
	mode->prev          = mode;
    }
    return mode;
}

_X_EXPORT void
VidModeCopyMode(pointer modefrom, pointer modeto)
{
  memcpy(modeto, modefrom, sizeof(DisplayModeRec));
}


_X_EXPORT int
VidModeGetModeValue(pointer mode, int valtyp)
{
  int ret = 0;
  
  switch (valtyp) {
    case VIDMODE_H_DISPLAY:
	ret = ((DisplayModePtr) mode)->HDisplay;
	break;
    case VIDMODE_H_SYNCSTART:
	ret = ((DisplayModePtr)mode)->HSyncStart;
	break;
    case VIDMODE_H_SYNCEND:
	ret = ((DisplayModePtr)mode)->HSyncEnd;
	break;
    case VIDMODE_H_TOTAL:
	ret = ((DisplayModePtr)mode)->HTotal;
	break;
    case VIDMODE_H_SKEW:
	ret = ((DisplayModePtr)mode)->HSkew;
	break;
    case VIDMODE_V_DISPLAY:
	ret = ((DisplayModePtr)mode)->VDisplay;
	break;
    case VIDMODE_V_SYNCSTART:
	ret = ((DisplayModePtr)mode)->VSyncStart;
	break;
    case VIDMODE_V_SYNCEND:
	ret = ((DisplayModePtr)mode)->VSyncEnd;
	break;
    case VIDMODE_V_TOTAL:
	ret = ((DisplayModePtr)mode)->VTotal;
	break;
    case VIDMODE_FLAGS:
	ret = ((DisplayModePtr)mode)->Flags;
	break;
    case VIDMODE_CLOCK:
	ret = ((DisplayModePtr)mode)->Clock;
	break;
  }
  return ret;
}

_X_EXPORT void
VidModeSetModeValue(pointer mode, int valtyp, int val)
{
  switch (valtyp) {
    case VIDMODE_H_DISPLAY:
	((DisplayModePtr)mode)->HDisplay = val;
	break;
    case VIDMODE_H_SYNCSTART:
	((DisplayModePtr)mode)->HSyncStart = val;
	break;
    case VIDMODE_H_SYNCEND:
	((DisplayModePtr)mode)->HSyncEnd = val;
	break;
    case VIDMODE_H_TOTAL:
	((DisplayModePtr)mode)->HTotal = val;
	break;
    case VIDMODE_H_SKEW:
	((DisplayModePtr)mode)->HSkew = val;
	break;
    case VIDMODE_V_DISPLAY:
	((DisplayModePtr)mode)->VDisplay = val;
	break;
    case VIDMODE_V_SYNCSTART:
	((DisplayModePtr)mode)->VSyncStart = val;
	break;
    case VIDMODE_V_SYNCEND:
	((DisplayModePtr)mode)->VSyncEnd = val;
	break;
    case VIDMODE_V_TOTAL:
	((DisplayModePtr)mode)->VTotal = val;
	break;
    case VIDMODE_FLAGS:
	((DisplayModePtr)mode)->Flags = val;
	break;
    case VIDMODE_CLOCK:
	((DisplayModePtr)mode)->Clock = val;
	break;
  }
  return;
}

_X_EXPORT vidMonitorValue
VidModeGetMonitorValue(pointer monitor, int valtyp, int indx)
{
  vidMonitorValue ret;
  
  switch (valtyp) {
    case VIDMODE_MON_VENDOR:
	ret.ptr = (((MonPtr)monitor)->vendor);
	break;
    case VIDMODE_MON_MODEL:
	ret.ptr = (((MonPtr)monitor)->model);
	break;
    case VIDMODE_MON_NHSYNC:
	ret.i = ((MonPtr)monitor)->nHsync;
	break;
    case VIDMODE_MON_NVREFRESH:
	ret.i = ((MonPtr)monitor)->nVrefresh;
	break;
    case VIDMODE_MON_HSYNC_LO:
	ret.f = (100.0 * ((MonPtr)monitor)->hsync[indx].lo);
	break;
    case VIDMODE_MON_HSYNC_HI:
	ret.f = (100.0 * ((MonPtr)monitor)->hsync[indx].hi);
	break;
    case VIDMODE_MON_VREFRESH_LO:
	ret.f = (100.0 * ((MonPtr)monitor)->vrefresh[indx].lo);
	break;
    case VIDMODE_MON_VREFRESH_HI:
	ret.f = (100.0 * ((MonPtr)monitor)->vrefresh[indx].hi);
	break;
  }
  return ret;
}


#endif /* XF86VIDMODE */
