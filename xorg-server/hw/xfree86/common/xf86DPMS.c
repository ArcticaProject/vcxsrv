
/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
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
 * This file contains the DPMS functions required by the extension.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "os.h"
#include "globals.h"
#include "xf86.h"
#include "xf86Priv.h"
#ifdef DPMSExtension
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#include "dpmsproc.h"
#endif


#ifdef DPMSExtension
static int DPMSKeyIndex;
static DevPrivateKey DPMSKey;
static Bool DPMSClose(int i, ScreenPtr pScreen);
static int DPMSCount = 0;
#endif


_X_EXPORT Bool
xf86DPMSInit(ScreenPtr pScreen, DPMSSetProcPtr set, int flags)
{
#ifdef DPMSExtension
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    DPMSPtr pDPMS;
    pointer DPMSOpt;
    MessageType enabled_from = X_INFO;

    DPMSKey = &DPMSKeyIndex;

    if (DPMSDisabledSwitch)
	DPMSEnabled = FALSE;
    if (!dixSetPrivate(&pScreen->devPrivates, DPMSKey,
		       xcalloc(sizeof(DPMSRec), 1)))
	return FALSE;

    pDPMS = (DPMSPtr)dixLookupPrivate(&pScreen->devPrivates, DPMSKey);
    pScrn->DPMSSet = set;
    pDPMS->Flags = flags;
    DPMSOpt = xf86FindOption(pScrn->options, "dpms");
    if (DPMSOpt) {
	if ((pDPMS->Enabled
	    = xf86SetBoolOption(pScrn->options, "dpms", FALSE))
	    && !DPMSDisabledSwitch)
	    DPMSEnabled = TRUE;
            enabled_from = X_CONFIG;
	xf86MarkOptionUsed(DPMSOpt);
    } else if (DPMSEnabledSwitch) {
	if (!DPMSDisabledSwitch)
	    DPMSEnabled = TRUE;
	pDPMS->Enabled = TRUE;
    }  
    else {
	pDPMS->Enabled = defaultDPMSEnabled;
    }
    if (pDPMS->Enabled)
	xf86DrvMsg(pScreen->myNum, enabled_from, "DPMS enabled\n");
    pDPMS->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = DPMSClose;
    DPMSCount++;
    return TRUE;
#else
    return FALSE;
#endif
}


#ifdef DPMSExtension

static Bool
DPMSClose(int i, ScreenPtr pScreen)
{
    DPMSPtr pDPMS;

    /* This shouldn't happen */
    if (DPMSKey == NULL)
	return FALSE;

    pDPMS = (DPMSPtr)dixLookupPrivate(&pScreen->devPrivates, DPMSKey);

    /* This shouldn't happen */
    if (!pDPMS)
	return FALSE;

    pScreen->CloseScreen = pDPMS->CloseScreen;

    /*
     * Turn on DPMS when shutting down. If this function can be used
     * depends on the order the driver wraps things. If this is called
     * after the driver has shut down everything the driver will have
     * to deal with this internally.
     */
    if (xf86Screens[i]->vtSema && xf86Screens[i]->DPMSSet) {
 	xf86Screens[i]->DPMSSet(xf86Screens[i],DPMSModeOn,0);
    }
    
    xfree((pointer)pDPMS);
    dixSetPrivate(&pScreen->devPrivates, DPMSKey, NULL);
    if (--DPMSCount == 0)
	DPMSKey = NULL;
    return pScreen->CloseScreen(i, pScreen);
}


/*
 * DPMSSet --
 *	Device dependent DPMS mode setting hook.  This is called whenever
 *	the DPMS mode is to be changed.
 */
_X_EXPORT int
DPMSSet(ClientPtr client, int level)
{
    int rc, i;
    DPMSPtr pDPMS;
    ScrnInfoPtr pScrn;

    DPMSPowerLevel = level;

    if (DPMSKey == NULL)
	return Success;

    if (level != DPMSModeOn) {
	rc = dixSaveScreens(client, SCREEN_SAVER_FORCER, ScreenSaverActive);
	if (rc != Success)
	    return rc;
    }

    /* For each screen, set the DPMS level */
    for (i = 0; i < xf86NumScreens; i++) {
    	pScrn = xf86Screens[i];
	pDPMS = (DPMSPtr)dixLookupPrivate(&screenInfo.screens[i]->devPrivates,
					  DPMSKey);
	if (pDPMS && pScrn->DPMSSet && pDPMS->Enabled && pScrn->vtSema) { 
	    xf86EnableAccess(pScrn);
	    pScrn->DPMSSet(pScrn, level, 0);
	}
    }
    return Success;
}


/*
 * DPMSSupported --
 *	Return TRUE if any screen supports DPMS.
 */
_X_EXPORT Bool
DPMSSupported(void)
{
    int i;
    DPMSPtr pDPMS;
    ScrnInfoPtr pScrn;

    if (DPMSKey == NULL) {
	return FALSE;
    }

    /* For each screen, check if DPMS is supported */
    for (i = 0; i < xf86NumScreens; i++) {
    	pScrn = xf86Screens[i];
	pDPMS = (DPMSPtr)dixLookupPrivate(&screenInfo.screens[i]->devPrivates,
					  DPMSKey);
	if (pDPMS && pScrn->DPMSSet)
	    return TRUE;
    }
    return FALSE;
}


/*
 * DPMSGet --
 *	Device dependent DPMS mode getting hook.  This returns the current
 *	DPMS mode, or -1 if DPMS is not supported.
 *
 *	This should hook in to the appropriate driver-level function, which
 *	will be added to the ScrnInfoRec.
 *
 *	NOTES:
 *	 1. the calling interface should be changed to specify which
 *	    screen to check.
 *	 2. It isn't clear that this function is ever used or what it should
 *	    return.
 */
_X_EXPORT int
DPMSGet(int *level)
{
    return DPMSPowerLevel;
}

#endif /* DPMSExtension */
