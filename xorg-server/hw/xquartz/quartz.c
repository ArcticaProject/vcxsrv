/*
 *
 * Quartz-specific support for the Darwin X Server
 *
 * Copyright (c) 2001-2004 Greg Parker and Torrey T. Lyons.
 *                 All Rights Reserved.
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
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#include "sanitizedCarbon.h"

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "quartzCommon.h"
#include "inputstr.h"
#include "quartz.h"
#include "darwin.h"
#include "darwinEvents.h"
#include "pseudoramiX.h"
#define _APPLEWM_SERVER_
#include "applewmExt.h"

#include "X11Application.h"

#include <X11/extensions/applewm.h>
#include <X11/extensions/randr.h>

// X headers
#include "scrnintstr.h"
#include "windowstr.h"
#include "colormapst.h"
#include "globals.h"
#include "mi.h"

// System headers
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

#include <rootlessCommon.h>
#include <Xplugin.h>

#define FAKE_RANDR 1

// Shared global variables for Quartz modes
int                     quartzEventWriteFD = -1;
int                     quartzUseSysBeep = 0;
int                     quartzUseAGL = 1;
int                     quartzEnableKeyEquivalents = 1;
int                     quartzServerVisible = FALSE;
int                     quartzServerQuitting = FALSE;
static int              quartzScreenKeyIndex;
DevPrivateKey           quartzScreenKey = &quartzScreenKeyIndex;
int                     aquaMenuBarHeight = 0;
QuartzModeProcsPtr      quartzProcs = NULL;
const char             *quartzOpenGLBundle = NULL;
int                     quartzFullscreenDisableHotkeys = TRUE;

#if defined(RANDR) && !defined(FAKE_RANDR)
Bool QuartzRandRGetInfo (ScreenPtr pScreen, Rotation *rotations) {
  return FALSE;
}

Bool QuartzRandRSetConfig (ScreenPtr           pScreen,
			       Rotation            randr,
			       int                 rate,
			       RRScreenSizePtr     pSize) {
  return FALSE;
}

Bool QuartzRandRInit (ScreenPtr pScreen) {
  rrScrPrivPtr    pScrPriv;
    
  if (!RRScreenInit (pScreen)) return FALSE;

  pScrPriv = rrGetScrPriv(pScreen);
  pScrPriv->rrGetInfo = QuartzRandRGetInfo;
  pScrPriv->rrSetConfig = QuartzRandRSetConfig;
  return TRUE;
}
#endif

/*
===========================================================================

 Screen functions

===========================================================================
*/

/*
 * QuartzAddScreen
 *  Do mode dependent initialization of each screen for Quartz.
 */
Bool QuartzAddScreen(
    int index,
    ScreenPtr pScreen)
{
    // allocate space for private per screen Quartz specific storage
    QuartzScreenPtr displayInfo = xcalloc(sizeof(QuartzScreenRec), 1);

    // QUARTZ_PRIV(pScreen) = displayInfo;
    dixSetPrivate(&pScreen->devPrivates, quartzScreenKey, displayInfo);

    // do Quartz mode specific initialization
    return quartzProcs->AddScreen(index, pScreen);
}


/*
 * QuartzSetupScreen
 *  Finalize mode specific setup of each screen.
 */
Bool QuartzSetupScreen(
    int index,
    ScreenPtr pScreen)
{
    // do Quartz mode specific setup
    if (! quartzProcs->SetupScreen(index, pScreen))
        return FALSE;

    // setup cursor support
    if (! quartzProcs->InitCursor(pScreen))
        return FALSE;

    return TRUE;
}


/*
 * QuartzInitOutput
 *  Quartz display initialization.
 */
void QuartzInitOutput(
    int argc,
    char **argv )
{
    if (!RegisterBlockAndWakeupHandlers(QuartzBlockHandler,
                                        QuartzWakeupHandler,
                                        NULL))
    {
        FatalError("Could not register block and wakeup handlers.");
    }

    // Do display mode specific initialization
    quartzProcs->DisplayInit();
}


/*
 * QuartzInitInput
 *  Inform the main thread the X server is ready to handle events.
 */
void QuartzInitInput(
    int argc,
    char **argv )
{
    X11ApplicationSetCanQuit(1);
    X11ApplicationServerReady();
    // Do final display mode specific initialization before handling events
    if (quartzProcs->InitInput)
        quartzProcs->InitInput(argc, argv);
}


#ifdef FAKE_RANDR

static const int padlength[4] = {0, 3, 2, 1};

static void
RREditConnectionInfo (ScreenPtr pScreen)
{
    xConnSetup	    *connSetup;
    char	    *vendor;
    xPixmapFormat   *formats;
    xWindowRoot	    *root;
    xDepth	    *depth;
    xVisualType	    *visual;
    int		    screen = 0;
    int		    d;

    connSetup = (xConnSetup *) ConnectionInfo;
    vendor = (char *) connSetup + sizeof (xConnSetup);
    formats = (xPixmapFormat *) ((char *) vendor +
				 connSetup->nbytesVendor +
				 padlength[connSetup->nbytesVendor & 3]);
    root = (xWindowRoot *) ((char *) formats +
			    sizeof (xPixmapFormat) * screenInfo.numPixmapFormats);
    while (screen != pScreen->myNum)
    {
	depth = (xDepth *) ((char *) root + 
			    sizeof (xWindowRoot));
	for (d = 0; d < root->nDepths; d++)
	{
	    visual = (xVisualType *) ((char *) depth +
				      sizeof (xDepth));
	    depth = (xDepth *) ((char *) visual +
				depth->nVisuals * sizeof (xVisualType));
	}
	root = (xWindowRoot *) ((char *) depth);
	screen++;
    }
    root->pixWidth = pScreen->width;
    root->pixHeight = pScreen->height;
    root->mmWidth = pScreen->mmWidth;
    root->mmHeight = pScreen->mmHeight;
}
#endif

static void QuartzUpdateScreens(void) {
    ScreenPtr pScreen;
    WindowPtr pRoot;
    int x, y, width, height, sx, sy;
    xEvent e;
    
    if (noPseudoramiXExtension || screenInfo.numScreens != 1)
    {
        /* FIXME: if not using Xinerama, we have multiple screens, and
         to do this properly may need to add or remove screens. Which
         isn't possible. So don't do anything. Another reason why
         we default to running with Xinerama. */
        
        return;
    }
    
    pScreen = screenInfo.screens[0];
    
    PseudoramiXResetScreens();
    quartzProcs->AddPseudoramiXScreens(&x, &y, &width, &height);
    
    dixScreenOrigins[pScreen->myNum].x = x;
    dixScreenOrigins[pScreen->myNum].y = y;
    pScreen->mmWidth = pScreen->mmWidth * ((double) width / pScreen->width);
    pScreen->mmHeight = pScreen->mmHeight * ((double) height / pScreen->height);
    pScreen->width = width;
    pScreen->height = height;
    
#ifndef FAKE_RANDR
    if(!QuartzRandRInit(pScreen))
        FatalError("Failed to init RandR extension.\n");
#endif
    
    DarwinAdjustScreenOrigins(&screenInfo);
    quartzProcs->UpdateScreen(pScreen);
    
    sx = dixScreenOrigins[pScreen->myNum].x + darwinMainScreenX;
    sy = dixScreenOrigins[pScreen->myNum].y + darwinMainScreenY;
    
    /* Adjust the root window. */
    pRoot = WindowTable[pScreen->myNum];
    AppleWMSetScreenOrigin(pRoot);
    pScreen->ResizeWindow(pRoot, x - sx, y - sy, width, height, NULL);
    //pScreen->PaintWindowBackground (pRoot, &pRoot->borderClip,  PW_BACKGROUND);
    miPaintWindow(pRoot, &pRoot->borderClip,  PW_BACKGROUND);
    DefineInitialRootWindow(pRoot);

    DEBUG_LOG("Root Window: %dx%d @ (%d, %d) darwinMainScreen (%d, %d) xy (%d, %d) dixScreenOrigins (%d, %d)\n", width, height, x - sx, y - sy, darwinMainScreenX, darwinMainScreenY, x, y, dixScreenOrigins[pScreen->myNum].x, dixScreenOrigins[pScreen->myNum].y);

    /* Send an event for the root reconfigure */
    e.u.u.type = ConfigureNotify;
    e.u.configureNotify.window = pRoot->drawable.id;
    e.u.configureNotify.aboveSibling = None;
    e.u.configureNotify.x = x - sx;
    e.u.configureNotify.y = y - sy;
    e.u.configureNotify.width = width;
    e.u.configureNotify.height = height;
    e.u.configureNotify.borderWidth = wBorderWidth(pRoot);
    e.u.configureNotify.override = pRoot->overrideRedirect;
    DeliverEvents(pRoot, &e, 1, NullWindow);
    
#ifdef FAKE_RANDR
    RREditConnectionInfo(pScreen);
#endif    
}

/*
 * QuartzDisplayChangeHandler
 *  Adjust for screen arrangement changes.
 */
void QuartzDisplayChangedHandler(int screenNum, xEventPtr xe, DeviceIntPtr dev, int nevents) {
    QuartzUpdateScreens();
}

void QuartzSetFullscreen(Bool state) {
    
    DEBUG_LOG("QuartzSetFullscreen: state=%d\n", state);
    
    if(quartzHasRoot == state)
        return;
    
    quartzHasRoot = state;
    
    xp_disable_update ();
    
    if (!quartzHasRoot && !quartzEnableRootless)
        RootlessHideAllWindows();
    
    RootlessUpdateRooted(quartzHasRoot);
    
    if (quartzHasRoot && !quartzEnableRootless)
        RootlessShowAllWindows ();
    
    if (quartzHasRoot || quartzEnableRootless) {
        RootlessRepositionWindows(screenInfo.screens[0]);
    }

    /* Somehow the menubar manages to interfere with our event stream
     * in fullscreen mode, even though it's not visible. 
     */
    X11ApplicationShowHideMenubar(!quartzHasRoot);
    
    xp_reenable_update ();
    
    if (quartzFullscreenDisableHotkeys)
        xp_disable_hot_keys(quartzHasRoot);
}

void QuartzSetRootless(Bool state) {
    if(quartzEnableRootless == state)
        return;
    
    quartzEnableRootless = state;

    xp_disable_update();

    /* When in rootless, the menubar is not part of the screen, so we need to update our screens on toggle */    
    QuartzUpdateScreens();

    if (!quartzEnableRootless && !quartzHasRoot) {
        RootlessHideAllWindows();
    } else if (quartzEnableRootless && !quartzHasRoot) {
        RootlessShowAllWindows();
    }

    xp_reenable_update();
}

/*
 * QuartzShow
 *  Show the X server on screen. Does nothing if already shown.
 *  Calls mode specific screen resume to restore the X clip regions
 *  (if needed) and the X server cursor state.
 */
void QuartzShow(
    int x,      // cursor location
    int y )
{
    int i;

    if (quartzServerVisible)
        return;
    
    quartzServerVisible = TRUE;
    for (i = 0; i < screenInfo.numScreens; i++) {
        if (screenInfo.screens[i]) {
            quartzProcs->ResumeScreen(screenInfo.screens[i], x, y);
        }
    }
    
    if (!quartzEnableRootless)
        QuartzSetFullscreen(TRUE);
}


/*
 * QuartzHide
 *  Remove the X server display from the screen. Does nothing if already
 *  hidden. Calls mode specific screen suspend to set X clip regions to
 *  prevent drawing (if needed) and restore the Aqua cursor.
 */
void QuartzHide(void)
{
    int i;

    if (quartzServerVisible) {
        for (i = 0; i < screenInfo.numScreens; i++) {
            if (screenInfo.screens[i]) {
                quartzProcs->SuspendScreen(screenInfo.screens[i]);
            }
        }
    }
    
    QuartzSetFullscreen(FALSE);
    quartzServerVisible = FALSE;
}


/*
 * QuartzSetRootClip
 *  Enable or disable rendering to the X screen.
 */
void QuartzSetRootClip(
    BOOL enable)
{
    int i;

    if (!quartzServerVisible)
        return;

    for (i = 0; i < screenInfo.numScreens; i++) {
        if (screenInfo.screens[i]) {
            xf86SetRootClip(screenInfo.screens[i], enable);
        }
    }
}

/* 
 * QuartzSpaceChanged
 *  Unmap offscreen windows, map onscreen windows
 */
void QuartzSpaceChanged(uint32_t space_id) {
    /* Do something special here, so we don't depend on quartz-wm for spaces to work... */
    DEBUG_LOG("Space Changed (%u) ... do something interesting...\n", space_id);
}
