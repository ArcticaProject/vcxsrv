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
#include "quartzRandR.h"
#include "inputstr.h"
#include "quartz.h"
#include "darwin.h"
#include "darwinEvents.h"
#include "pseudoramiX.h"
#define _APPLEWM_SERVER_
#include "applewmExt.h"

#include "X11Application.h"

#include <X11/extensions/applewmconst.h>

// X headers
#include "scrnintstr.h"
#include "windowstr.h"
#include "colormapst.h"
#include "globals.h"
#include "mi.h"

// System headers
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <pthread.h>
#include <signal.h>

#include <rootlessCommon.h>
#include <Xplugin.h>

DevPrivateKeyRec        quartzScreenKeyRec;
int                     aquaMenuBarHeight = 0;
QuartzModeProcsPtr      quartzProcs = NULL;
const char             *quartzOpenGLBundle = NULL;

Bool XQuartzFullscreenDisableHotkeys = TRUE;
Bool XQuartzOptionSendsAlt = FALSE;
Bool XQuartzEnableKeyEquivalents = TRUE;
Bool XQuartzFullscreenVisible = FALSE;
Bool XQuartzRootlessDefault = TRUE;
Bool XQuartzIsRootless = TRUE;
Bool XQuartzServerVisible = FALSE;
Bool XQuartzFullscreenMenu = FALSE;

int32_t XQuartzShieldingWindowLevel = 0;

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
    QuartzScreenPtr displayInfo = calloc(sizeof(QuartzScreenRec), 1);

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

#if defined(RANDR)
    if(!QuartzRandRInit(pScreen)) {
        DEBUG_LOG("Failed to init RandR extension.\n");
        return FALSE;
    }
#endif

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
    /* For XQuartz, we want to just use the default signal handler to work better with CrashTracer */
    signal(SIGSEGV, SIG_DFL);
    signal(SIGILL, SIG_DFL);
#ifdef SIGEMT
    signal(SIGEMT, SIG_DFL);
#endif
    signal(SIGFPE, SIG_DFL);
#ifdef SIGBUS
    signal(SIGBUS, SIG_DFL);
#endif
#ifdef SIGSYS
    signal(SIGSYS, SIG_DFL);
#endif
#ifdef SIGXCPU
    signal(SIGXCPU, SIG_DFL);
#endif
#ifdef SIGXFSZ
    signal(SIGXFSZ, SIG_DFL);
#endif

    if (!RegisterBlockAndWakeupHandlers(QuartzBlockHandler,
                                        QuartzWakeupHandler,
                                        NULL))
    {
        FatalError("Could not register block and wakeup handlers.");
    }

    if (!dixRegisterPrivateKey(&quartzScreenKeyRec, PRIVATE_SCREEN, 0))
	FatalError("Failed to alloc quartz screen private.\n");

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
    X11ApplicationSetCanQuit(0);
    X11ApplicationServerReady();
    // Do final display mode specific initialization before handling events
    if (quartzProcs->InitInput)
        quartzProcs->InitInput(argc, argv);
}


void QuartzUpdateScreens(void) {
    ScreenPtr pScreen;
    WindowPtr pRoot;
    int x, y, width, height, sx, sy;
    xEvent e;
    BoxRec bounds;
    
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
    quartzProcs->AddPseudoramiXScreens(&x, &y, &width, &height, pScreen);
    
    pScreen->x = x;
    pScreen->y = y;
    pScreen->mmWidth = pScreen->mmWidth * ((double) width / pScreen->width);
    pScreen->mmHeight = pScreen->mmHeight * ((double) height / pScreen->height);
    pScreen->width = width;
    pScreen->height = height;
    
    DarwinAdjustScreenOrigins(&screenInfo);
    
    /* DarwinAdjustScreenOrigins or UpdateScreen may change pScreen->x/y,
     * so use it rather than x/y
     */
    sx = pScreen->x + darwinMainScreenX;
    sy = pScreen->y + darwinMainScreenY;
    
    /* Adjust the root window. */
    pRoot = pScreen->root;
    AppleWMSetScreenOrigin(pRoot);
    pScreen->ResizeWindow(pRoot, x - sx, y - sy, width, height, NULL);

    miPaintWindow(pRoot, &pRoot->borderClip,  PW_BACKGROUND);

    /* <rdar://problem/7770779> pointer events are clipped to old display region after display reconfiguration
     * http://xquartz.macosforge.org/trac/ticket/346
     */
    bounds.x1 = 0;
    bounds.x2 = width;
    bounds.y1 = 0;
    bounds.y2 = height;
    pScreen->ConstrainCursor(inputInfo.pointer, pScreen, &bounds);
    inputInfo.pointer->spriteInfo->sprite->physLimits = bounds;
    inputInfo.pointer->spriteInfo->sprite->hotLimits = bounds;

    DEBUG_LOG("Root Window: %dx%d @ (%d, %d) darwinMainScreen (%d, %d) xy (%d, %d) dixScreenOrigins (%d, %d)\n", width, height, x - sx, y - sy, darwinMainScreenX, darwinMainScreenY, x, y, pScreen->x, pScreen->y);

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

    quartzProcs->UpdateScreen(pScreen);

    /* Tell RandR about the new size, so new connections get the correct info */
    RRScreenSizeNotify(pScreen);
}

static void pokeActivityCallback(CFRunLoopTimerRef timer, void *info) {
    UpdateSystemActivity(OverallAct);
}

static void QuartzScreenSaver(int state) {
    static CFRunLoopTimerRef pokeActivityTimer = NULL;
    static CFRunLoopTimerContext pokeActivityContext = { 0, NULL, NULL, NULL, NULL };
    static pthread_mutex_t pokeActivityMutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&pokeActivityMutex);
    
    if(state) {
        if(pokeActivityTimer == NULL)
            goto QuartzScreenSaverEnd;

        CFRunLoopTimerInvalidate(pokeActivityTimer);
        CFRelease(pokeActivityTimer);
        pokeActivityTimer = NULL;
    } else {
        if(pokeActivityTimer != NULL)
            goto QuartzScreenSaverEnd;
        
        pokeActivityTimer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent(), 30, 0, 0, pokeActivityCallback, &pokeActivityContext);
        if(pokeActivityTimer == NULL) {
            ErrorF("Unable to create pokeActivityTimer.\n");
            goto QuartzScreenSaverEnd;
        }

        CFRunLoopAddTimer(CFRunLoopGetMain(), pokeActivityTimer, kCFRunLoopCommonModes);
    }
QuartzScreenSaverEnd:
    pthread_mutex_unlock(&pokeActivityMutex);
}

void QuartzShowFullscreen(int state) {
    int i;
    
    DEBUG_LOG("QuartzShowFullscreen: state=%d\n", state);
    
    if(XQuartzIsRootless) {
        ErrorF("QuartzShowFullscreen called while in rootless mode.\n");
        return;
    }
    
    QuartzScreenSaver(!state);
    
    if(XQuartzFullscreenVisible == state)
        return;
    
    XQuartzFullscreenVisible = state;
    
    xp_disable_update ();
    
    if (!XQuartzFullscreenVisible)
        RootlessHideAllWindows();
    
    RootlessUpdateRooted(XQuartzFullscreenVisible);
    
    if (XQuartzFullscreenVisible) {
        RootlessShowAllWindows ();
        for (i=0; i < screenInfo.numScreens; i++) {
            ScreenPtr pScreen = screenInfo.screens[i];        
            RootlessRepositionWindows(pScreen);
            // JH: I don't think this is necessary, but keeping it here as a reminder
            //RootlessUpdateScreenPixmap(pScreen);
        }
    }

    /* Somehow the menubar manages to interfere with our event stream
     * in fullscreen mode, even though it's not visible. 
     */
    X11ApplicationShowHideMenubar(!XQuartzFullscreenVisible);
    
    xp_reenable_update ();
    
    if (XQuartzFullscreenDisableHotkeys)
        xp_disable_hot_keys(XQuartzFullscreenVisible);
}

void QuartzSetRootless(Bool state) {    
    DEBUG_LOG("QuartzSetRootless state=%d\n", state);
    
    if(XQuartzIsRootless == state)
        return;
    
    if(state)
        QuartzShowFullscreen(FALSE);
    
    XQuartzIsRootless = state;

    xp_disable_update();

    /* When in rootless, the menubar is not part of the screen, so we need to update our screens on toggle */    
    QuartzUpdateScreens();

    if(XQuartzIsRootless) {
        RootlessShowAllWindows();
    } else {
        RootlessHideAllWindows();
    }

    X11ApplicationShowHideMenubar(TRUE);

    xp_reenable_update();

    xp_disable_hot_keys(FALSE);
}

/*
 * QuartzShow
 *  Show the X server on screen. Does nothing if already shown.
 *  Calls mode specific screen resume to restore the X clip regions
 *  (if needed) and the X server cursor state.
 */
void QuartzShow(void) {
    int i;

    if (XQuartzServerVisible)
        return;
    
    XQuartzServerVisible = TRUE;
    for (i = 0; i < screenInfo.numScreens; i++) {
        if (screenInfo.screens[i]) {
            quartzProcs->ResumeScreen(screenInfo.screens[i]);
        }
    }
    
    if (!XQuartzIsRootless)
        QuartzShowFullscreen(TRUE);
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

    if (XQuartzServerVisible) {
        for (i = 0; i < screenInfo.numScreens; i++) {
            if (screenInfo.screens[i]) {
                quartzProcs->SuspendScreen(screenInfo.screens[i]);
            }
        }
    }

    if(!XQuartzIsRootless)
        QuartzShowFullscreen(FALSE);
    XQuartzServerVisible = FALSE;
}


/*
 * QuartzSetRootClip
 *  Enable or disable rendering to the X screen.
 */
void QuartzSetRootClip(
    BOOL enable)
{
    int i;

    if (!XQuartzServerVisible)
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

/*
 * QuartzCopyDisplayIDs
 *  Associate an X11 screen with one or more CoreGraphics display IDs by copying
 *  the list into a private array. Free the previously copied array, if present.
 */
void QuartzCopyDisplayIDs(ScreenPtr pScreen,
                          int displayCount, CGDirectDisplayID *displayIDs) {
    QuartzScreenPtr pQuartzScreen = QUARTZ_PRIV(pScreen);
    int size = displayCount * sizeof(CGDirectDisplayID);

    free(pQuartzScreen->displayIDs);
    pQuartzScreen->displayIDs = malloc(size);
    memcpy(pQuartzScreen->displayIDs, displayIDs, size);
    pQuartzScreen->displayCount = displayCount;
}

void NSBeep(void);
void DDXRingBell(
    int volume,         // volume is % of max
    int pitch,          // pitch is Hz
    int duration)       // duration is milliseconds
{
    if (volume)
        NSBeep();
}
