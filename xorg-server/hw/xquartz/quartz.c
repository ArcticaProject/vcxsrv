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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "quartzCommon.h"
#include "quartz.h"
#include "darwin.h"
#include "darwinEvents.h"
#include "quartzAudio.h"
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
#include "rootlessWindow.h"

// System headers
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

#define FAKE_RANDR 1

// Shared global variables for Quartz modes
int                     quartzEventWriteFD = -1;
int                     quartzRootless = -1;
int                     quartzUseSysBeep = 0;
int                     quartzUseAGL = 1;
int                     quartzEnableKeyEquivalents = 1;
int                     quartzServerVisible = TRUE;
int                     quartzServerQuitting = FALSE;
DevPrivateKey           quartzScreenKey = &quartzScreenKey;
int                     aquaMenuBarHeight = 0;
QuartzModeProcsPtr      quartzProcs = NULL;
const char             *quartzOpenGLBundle = NULL;

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
    if (serverGeneration == 0) {
        QuartzAudioInit();
    }

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
extern char	*ConnectionInfo;

static int padlength[4] = {0, 3, 2, 1};

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

/*
 * QuartzUpdateScreens
 *  Adjust for screen arrangement changes.
 */
static void QuartzUpdateScreens(void)
{
    ScreenPtr pScreen;
    WindowPtr pRoot;
    int x, y, width, height, sx, sy;
    xEvent e;

    DEBUG_LOG("QuartzUpdateScreens()\n");
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
    miPaintWindow(pRoot, &pRoot->borderClip,  PW_BACKGROUND);
//    QuartzIgnoreNextWarpCursor();
    DefineInitialRootWindow(pRoot);

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
 * QuartzShow
 *  Show the X server on screen. Does nothing if already shown.
 *  Calls mode specific screen resume to restore the X clip regions
 *  (if needed) and the X server cursor state.
 */
static void QuartzShow(
    int x,      // cursor location
    int y )
{
    int i;

    if (!quartzServerVisible) {
        quartzServerVisible = TRUE;
        for (i = 0; i < screenInfo.numScreens; i++) {
            if (screenInfo.screens[i]) {
                quartzProcs->ResumeScreen(screenInfo.screens[i], x, y);
            }
        }
    }
}


/*
 * QuartzHide
 *  Remove the X server display from the screen. Does nothing if already
 *  hidden. Calls mode specific screen suspend to set X clip regions to
 *  prevent drawing (if needed) and restore the Aqua cursor.
 */
static void QuartzHide(void)
{
    int i;

    if (quartzServerVisible) {
        for (i = 0; i < screenInfo.numScreens; i++) {
            if (screenInfo.screens[i]) {
                quartzProcs->SuspendScreen(screenInfo.screens[i]);
            }
        }
    }
    quartzServerVisible = FALSE;
}


/*
 * QuartzSetRootClip
 *  Enable or disable rendering to the X screen.
 */
static void QuartzSetRootClip(
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
 * QuartzMessageServerThread
 *  Send the X server thread a message by placing it on the event queue.
 */
void
QuartzMessageServerThread(
    int type,
    int argc, ...)
{
    xEvent xe;
    INT32 *argv;
    int i, max_args;
    va_list args;

    memset(&xe, 0, sizeof(xe));
    xe.u.u.type = type;
    xe.u.clientMessage.u.l.type = type;

    argv = &xe.u.clientMessage.u.l.longs0;
    max_args = 4;

    if (argc > 0 && argc <= max_args) {
        va_start (args, argc);
        for (i = 0; i < argc; i++)
            argv[i] = (int) va_arg (args, int);
        va_end (args);
    }

    DarwinEQEnqueue(&xe);
}


/*
 * QuartzProcessEvent
 *  Process Quartz specific events.
 */
void QuartzProcessEvent(xEvent *xe) {
    switch (xe->u.u.type) {
        case kXDarwinControllerNotify:
            DEBUG_LOG("kXDarwinControllerNotify\n");
            AppleWMSendEvent(AppleWMControllerNotify,
                             AppleWMControllerNotifyMask,
                             xe->u.clientMessage.u.l.longs0,
                             xe->u.clientMessage.u.l.longs1);
            break;

        case kXDarwinPasteboardNotify:
            DEBUG_LOG("kXDarwinPasteboardNotify\n");
            AppleWMSendEvent(AppleWMPasteboardNotify,
                             AppleWMPasteboardNotifyMask,
                             xe->u.clientMessage.u.l.longs0,
                             xe->u.clientMessage.u.l.longs1);
            break;

        case kXDarwinActivate:
            DEBUG_LOG("kXDarwinActivate\n");
            QuartzShow(xe->u.keyButtonPointer.rootX,
                       xe->u.keyButtonPointer.rootY);
            AppleWMSendEvent(AppleWMActivationNotify,
                             AppleWMActivationNotifyMask,
                             AppleWMIsActive, 0);
            break;

        case kXDarwinDeactivate:
            DEBUG_LOG("kXDarwinDeactivate\n");
            AppleWMSendEvent(AppleWMActivationNotify,
                             AppleWMActivationNotifyMask,
                             AppleWMIsInactive, 0);
            QuartzHide();
            break;

        case kXDarwinDisplayChanged:
            DEBUG_LOG("kXDarwinDisplayChanged\n");
            QuartzUpdateScreens();
            break;

        case kXDarwinWindowState:
            DEBUG_LOG("kXDarwinWindowState\n");
            RootlessNativeWindowStateChanged(xe->u.clientMessage.u.l.longs0,
		  			     xe->u.clientMessage.u.l.longs1);
	    break;
	  
        case kXDarwinWindowMoved:
            DEBUG_LOG("kXDarwinWindowMoved\n");
            RootlessNativeWindowMoved ((WindowPtr)xe->u.clientMessage.u.l.longs0);
	    break;

        case kXDarwinToggleFullscreen:
            DEBUG_LOG("kXDarwinToggleFullscreen\n");
#ifdef DARWIN_DDX_MISSING
            if (quartzEnableRootless) QuartzSetFullscreen(!quartzHasRoot);
            else if (quartzHasRoot) QuartzHide();
            else QuartzShow();
#else
    //	    ErrorF("kXDarwinToggleFullscreen not implemented\n");
#endif
            break;

        case kXDarwinSetRootless:
            DEBUG_LOG("kXDarwinSetRootless\n");
#ifdef DARWIN_DDX_MISSING
            QuartzSetRootless(xe->u.clientMessage.u.l.longs0);
            if (!quartzEnableRootless && !quartzHasRoot) QuartzHide();
#else
    //	    ErrorF("kXDarwinSetRootless not implemented\n");
#endif
            break;

        case kXDarwinSetRootClip:
            QuartzSetRootClip((BOOL)xe->u.clientMessage.u.l.longs0);
            break;

        case kXDarwinQuit:
            GiveUp(0);
            break;

        case kXDarwinReadPasteboard:
            QuartzReadPasteboard();
            break;

        case kXDarwinWritePasteboard:
            QuartzWritePasteboard();
            break;

        case kXDarwinBringAllToFront:
            DEBUG_LOG("kXDarwinBringAllToFront\n");
            RootlessOrderAllWindows();
            break;

        default:
            ErrorF("Unknown application defined event type %d.\n", xe->u.u.type);
    }
}
