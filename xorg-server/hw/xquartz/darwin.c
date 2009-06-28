/**************************************************************
 *
 * Shared code for the Darwin X Server
 * running with Quartz or IOKit display mode
 *
 * Copyright (c) 2001-2004 Torrey T. Lyons. All Rights Reserved.
 * Copyright (c) 2007 Apple Inc.
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

#include <X11/X.h>
#include <X11/Xproto.h>
#include "os.h"
#include "servermd.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "mibstore.h"		// mi backing store implementation
#include "mipointer.h"		// mi software cursor
#include "micmap.h"		// mi colormap code
#include "fb.h"			// fb framebuffer code
#include "site.h"
#include "globals.h"
#include "dix.h"

#ifdef XINPUT
# include <X11/extensions/XI.h>
# include <X11/extensions/XIproto.h>
# include "exevents.h"
# include "extinit.h"
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <sys/syslimits.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define HAS_UTSNAME 1
#include <sys/utsname.h>

#define NO_CFPLUGIN
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/ev_keymap.h>

#ifdef MITSHM
#define _XSHM_SERVER_
#include <X11/extensions/XShm.h>
#endif

#include "darwin.h"
#include "darwinEvents.h"
#include "darwinKeyboard.h"
#include "quartz.h"
//#include "darwinClut8.h"

#ifdef ENABLE_DEBUG_LOG
FILE *debug_log_fp = NULL;
#endif

/*
 * X server shared global variables
 */
int                     darwinScreensFound = 0;
DevPrivateKey           darwinScreenKey = &darwinScreenKey;
io_connect_t            darwinParamConnect = 0;
int                     darwinEventReadFD = -1;
int                     darwinEventWriteFD = -1;
// int                     darwinMouseAccelChange = 1;
int                     darwinFakeButtons = 0;

// location of X11's (0,0) point in global screen coordinates
int                     darwinMainScreenX = 0;
int                     darwinMainScreenY = 0;

// parameters read from the command line or user preferences
unsigned int            darwinDesiredWidth = 0, darwinDesiredHeight = 0;
int                     darwinDesiredDepth = -1;
int                     darwinDesiredRefresh = -1;
char                    *darwinKeymapFile = "USA.keymapping";
int                     darwinSyncKeymap = FALSE;

// modifier masks for faking mouse buttons
int                     darwinFakeMouse2Mask = NX_ALTERNATEMASK;
int                     darwinFakeMouse3Mask = NX_COMMANDMASK;

// devices
DeviceIntPtr            darwinPointer = NULL;
DeviceIntPtr            darwinKeyboard = NULL;

// Common pixmap formats
static PixmapFormatRec formats[] = {
        { 1,    1,      BITMAP_SCANLINE_PAD },
        { 4,    8,      BITMAP_SCANLINE_PAD },
        { 8,    8,      BITMAP_SCANLINE_PAD },
        { 15,   16,     BITMAP_SCANLINE_PAD },
        { 16,   16,     BITMAP_SCANLINE_PAD },
        { 24,   32,     BITMAP_SCANLINE_PAD },
        { 32,   32,     BITMAP_SCANLINE_PAD }
};
const int NUMFORMATS = sizeof(formats)/sizeof(formats[0]);

#ifndef OSNAME
#define OSNAME " Darwin"
#endif
#ifndef OSVENDOR
#define OSVENDOR ""
#endif
#ifndef PRE_RELEASE
#define PRE_RELEASE XORG_VERSION_SNAP
#endif
#ifndef BUILD_DATE
#define BUILD_DATE ""
#endif
#ifndef XORG_RELEASE
#define XORG_RELEASE "?"
#endif

void DDXRingBell(int volume, int pitch, int duration) {
  // FIXME -- make some noise, yo
}

void
DarwinPrintBanner(void)
{ 
  // this should change depending on which specific server we are building
  ErrorF("XQuartz starting:\n");
  ErrorF("X.org Release 7.2\n"); // This is here to help fink until they fix their packages.
  ErrorF("X.Org X Server %s\nBuild Date: %s\n", XSERVER_VERSION, BUILD_DATE );
}


/*
 * DarwinSaveScreen
 *  X screensaver support. Not implemented.
 */
static Bool DarwinSaveScreen(ScreenPtr pScreen, int on)
{
    // FIXME
    if (on == SCREEN_SAVER_FORCER) {
    } else if (on == SCREEN_SAVER_ON) {
    } else {
    }
    return TRUE;
}


/*
 * DarwinAddScreen
 *  This is a callback from dix during AddScreen() from InitOutput().
 *  Initialize the screen and communicate information about it back to dix.
 */
static Bool DarwinAddScreen(int index, ScreenPtr pScreen, int argc, char **argv) {
    int         dpi;
    static int  foundIndex = 0;
    Bool        ret;
    DarwinFramebufferPtr dfb;

    // reset index of found screens for each server generation
    if (index == 0) foundIndex = 0;

    // allocate space for private per screen storage
    dfb = xalloc(sizeof(DarwinFramebufferRec));

    // SCREEN_PRIV(pScreen) = dfb;
    dixSetPrivate(&pScreen->devPrivates, darwinScreenKey, dfb);

    // setup hardware/mode specific details
    ret = QuartzAddScreen(foundIndex, pScreen);
    foundIndex++;
    if (! ret)
        return FALSE;

    // reset the visual list
    miClearVisualTypes();

    // setup a single visual appropriate for our pixel type
    if(!miSetVisualTypesAndMasks(dfb->depth, dfb->visuals, dfb->bitsPerRGB,
                                 dfb->preferredCVC, dfb->redMask,
                                 dfb->greenMask, dfb->blueMask)) {
        return FALSE;
    }

    miSetPixmapDepths();

    // machine independent screen init
    // setup _Screen structure in pScreen
    if (monitorResolution)
        dpi = monitorResolution;
    else
        dpi = 75;

    // initialize fb
    if (! fbScreenInit(pScreen,
                dfb->framebuffer,                 // pointer to screen bitmap
                dfb->width, dfb->height,          // screen size in pixels
                dpi, dpi,                         // dots per inch
                dfb->pitch/(dfb->bitsPerPixel/8), // pixel width of framebuffer
                dfb->bitsPerPixel))               // bits per pixel for screen
    {
        return FALSE;
    }

//    ErrorF("Screen type: %d, %d=%d, %d=%d, %d=%d, %x=%x=%x, %x=%x=%x, %x=%x=%x\n", pScreen->visuals->class,
//           pScreen->visuals->offsetRed, dfb->bitsPerRGB * 2,
//           pScreen->visuals->offsetGreen, dfb->bitsPerRGB,
//           pScreen->visuals->offsetBlue, 0,
//           pScreen->visuals->redMask, dfb->redMask, ((1<<dfb->bitsPerRGB)-1) << pScreen->visuals->offsetRed,
//           pScreen->visuals->greenMask, dfb->greenMask, ((1<<dfb->bitsPerRGB)-1) << pScreen->visuals->offsetGreen,
//           pScreen->visuals->blueMask, dfb->blueMask, ((1<<dfb->bitsPerRGB)-1) << pScreen->visuals->offsetBlue);

    // set the RGB order correctly for TrueColor
//    if (dfb->bitsPerPixel > 8) {
//        for (i = 0, visual = pScreen->visuals;  // someday we may have more than 1
//            i < pScreen->numVisuals; i++, visual++) {
//            if (visual->class == TrueColor) {
//                visual->offsetRed = bitsPerRGB * 2;
//                visual->offsetGreen = bitsPerRGB;
//                visual->offsetBlue = 0;
//                visual->redMask = ((1<<bitsPerRGB)-1) << visual->offsetRed;
//                visual->greenMask = ((1<<bitsPerRGB)-1) << visual->offsetGreen;
//                visual->blueMask = ((1<<bitsPerRGB)-1) << visual->offsetBlue;
//            }
//        }
//    }

#ifdef RENDER
    if (! fbPictureInit(pScreen, 0, 0)) {
        return FALSE;
    }
#endif

#ifdef MITSHM
    ShmRegisterFbFuncs(pScreen);
#endif

    // this must be initialized (why doesn't X have a default?)
    pScreen->SaveScreen = DarwinSaveScreen;

    // finish mode dependent screen setup including cursor support
    if (!QuartzSetupScreen(index, pScreen)) {
        return FALSE;
    }

    // create and install the default colormap and
    // set pScreen->blackPixel / pScreen->white
    if (!miCreateDefColormap( pScreen )) {
        return FALSE;
    }

    /* Set the colormap to the statically defined one if we're in 8 bit
     * mode and we're using a fixed color map.  Essentially this translates
     * to Darwin/x86 in 8-bit mode.
     */
//    if(dfb->depth == 8) {
//        ColormapPtr map = RootlessGetColormap (pScreen);
//        for( i = 0; i < map->pVisual->ColormapEntries; i++ ) {
//            Entry *ent = map->red + i;
//            ErrorF("Setting lo %d -> r: %04x g: %04x b: %04x\n", i, darwinClut8[i].red, darwinClut8[i].green, darwinClut8[i].blue);
//            ent->co.local.red   = darwinClut8[i].red;
//            ent->co.local.green = darwinClut8[i].green;
//            ent->co.local.blue  = darwinClut8[i].blue;
//        }
//    }

    dixScreenOrigins[index].x = dfb->x;
    dixScreenOrigins[index].y = dfb->y;

    /*    ErrorF("Screen %d added: %dx%d @ (%d,%d)\n",
	  index, dfb->width, dfb->height, dfb->x, dfb->y); */

    return TRUE;
}

/*
 =============================================================================

 mouse and keyboard callbacks

 =============================================================================
*/

#if 0
/*
 * DarwinChangePointerControl
 *  Set mouse acceleration and thresholding
 *  FIXME: We currently ignore the threshold in ctrl->threshold.
 */
static void DarwinChangePointerControl(
    DeviceIntPtr    device,
    PtrCtrl         *ctrl )
{
    kern_return_t   kr;
    double          acceleration;

    if (!darwinMouseAccelChange)
        return;

    acceleration = ctrl->num / ctrl->den;
    kr = IOHIDSetMouseAcceleration( darwinParamConnect, acceleration );
    if (kr != KERN_SUCCESS)
        ErrorF( "Could not set mouse acceleration with kernel return = 0x%x.\n", kr );
}
#endif

/*
 * DarwinMouseProc
 *  Handle the initialization, etc. of a mouse
 */
static int DarwinMouseProc(
    DeviceIntPtr    pPointer,
    int             what )
{
    CARD8 map[6];

    switch (what) {

        case DEVICE_INIT:
            pPointer->public.on = FALSE;

            // Set button map.
            map[1] = 1;
            map[2] = 2;
            map[3] = 3;
            map[4] = 4;
            map[5] = 5;
            InitPointerDeviceStruct( (DevicePtr)pPointer, map, 5,
				     GetMotionHistory,
				     (PtrCtrlProcPtr)NoopDDA,
				     GetMotionHistorySize(), 2);

#ifdef XINPUT
            InitValuatorAxisStruct( pPointer,
                                    0,     // X axis
                                    0,     // min value
                                    16000, // max value (fixme screen size?)
                                    1,     // resolution (fixme ?)
                                    1,     // min resolution
                                    1 );   // max resolution
            InitValuatorAxisStruct( pPointer,
                                    1,     // X axis
                                    0,     // min value
                                    16000, // max value (fixme screen size?)
                                    1,     // resolution (fixme ?)
                                    1,     // min resolution
                                    1 );   // max resolution
#endif
            break;

        case DEVICE_ON:
            pPointer->public.on = TRUE;
            AddEnabledDevice( darwinEventReadFD );
            return Success;

        case DEVICE_CLOSE:
        case DEVICE_OFF:
            pPointer->public.on = FALSE;
            RemoveEnabledDevice( darwinEventReadFD );
            return Success;
    }

    return Success;
}


/*
 * DarwinKeybdProc
 *  Callback from X
 */
static int DarwinKeybdProc( DeviceIntPtr pDev, int onoff )
{
    switch ( onoff ) {
        case DEVICE_INIT:
            DarwinKeyboardInit( pDev );
            break;
        case DEVICE_ON:
            pDev->public.on = TRUE;
            AddEnabledDevice( darwinEventReadFD );
            break;
        case DEVICE_OFF:
            pDev->public.on = FALSE;
            RemoveEnabledDevice( darwinEventReadFD );
            break;
        case DEVICE_CLOSE:
            break;
    }

    return Success;
}

/*
===========================================================================

 Utility routines

===========================================================================
*/

/*
 * DarwinFindLibraryFile
 *  Search for a file in the standard Library paths, which are (in order):
 *
 *      ~/Library/              user specific
 *      /Library/               host specific
 *      /Network/Library/       LAN specific
 *      /System/Library/        OS specific
 *
 *  A sub-path can be specified to search in below the various Library
 *  directories. Returns a new character string (owned by the caller)
 *  containing the full path to the first file found.
 */
static char * DarwinFindLibraryFile(
    const char *file,
    const char *pathext )
{
    // Library search paths
    char *pathList[] = {
        "",
        "/Network",
        "/System",
        NULL
    };
    char *home;
    char *fullPath;
    int i = 0;

    // Return the file name as is if it is already a fully qualified path.
    if (!access(file, F_OK)) {
        fullPath = xalloc(strlen(file)+1);
        strcpy(fullPath, file);
        return fullPath;
    }

    fullPath = xalloc(PATH_MAX);

    home = getenv("HOME");
    if (home) {
        snprintf(fullPath, PATH_MAX, "%s/Library/%s/%s", home, pathext, file);
        if (!access(fullPath, F_OK))
            return fullPath;
    }

    while (pathList[i]) {
        snprintf(fullPath, PATH_MAX, "%s/Library/%s/%s", pathList[i++],
                 pathext, file);
        if (!access(fullPath, F_OK))
            return fullPath;
    }

    xfree(fullPath);
    return NULL;
}


/*
 * DarwinParseModifierList
 *  Parse a list of modifier names and return a corresponding modifier mask
 */
int DarwinParseModifierList(
    const char *constmodifiers) // string containing list of modifier names
{
    int result = 0;

    if (constmodifiers) {
        char *modifiers = strdup(constmodifiers);
        char *modifier;
        int nxkey;
        char *p = modifiers;

        while (p) {
            modifier = strsep(&p, " ,+&|/"); // allow lots of separators
            nxkey = DarwinModifierStringToNXKey(modifier);
            if (nxkey != -1)
                result |= DarwinModifierNXKeyToNXMask(nxkey);
            else
                ErrorF("fakebuttons: Unknown modifier \"%s\"\n", modifier);
        }
        free(modifiers);
    }
    return result;
}

/*
===========================================================================

 Functions needed to link against device independent X

===========================================================================
*/

/*
 * InitInput
 *  Register the keyboard and mouse devices
 */
void InitInput( int argc, char **argv )
{
    darwinPointer = AddInputDevice(DarwinMouseProc, TRUE);
    RegisterPointerDevice( darwinPointer );

    darwinKeyboard = AddInputDevice(DarwinKeybdProc, TRUE);
    RegisterKeyboardDevice( darwinKeyboard );

    DarwinEQInit( (DevicePtr)darwinKeyboard, (DevicePtr)darwinPointer );

    QuartzInitInput(argc, argv);
}


/*
 * DarwinAdjustScreenOrigins
 *  Shift all screens so the X11 (0, 0) coordinate is at the top
 *  left of the global screen coordinates.
 *
 *  Screens can be arranged so the top left isn't on any screen, so
 *  instead use the top left of the leftmost screen as (0,0). This
 *  may mean some screen space is in -y, but it's better that (0,0)
 *  be onscreen, or else default xterms disappear. It's better that
 *  -y be used than -x, because when popup menus are forced
 *  "onscreen" by dumb window managers like twm, they'll shift the
 *  menus down instead of left, which still looks funny but is an
 *  easier target to hit.
 */
void
DarwinAdjustScreenOrigins(ScreenInfo *pScreenInfo)
{
    int i, left, top;

    left = dixScreenOrigins[0].x;
    top  = dixScreenOrigins[0].y;

    /* Find leftmost screen. If there's a tie, take the topmost of the two. */
    for (i = 1; i < pScreenInfo->numScreens; i++) {
        if (dixScreenOrigins[i].x < left  ||
            (dixScreenOrigins[i].x == left &&
             dixScreenOrigins[i].y < top))
        {
            left = dixScreenOrigins[i].x;
            top = dixScreenOrigins[i].y;
        }
    }

    darwinMainScreenX = left;
    darwinMainScreenY = top;

    /* Shift all screens so that there is a screen whose top left
       is at X11 (0,0) and at global screen coordinate
       (darwinMainScreenX, darwinMainScreenY). */

    if (darwinMainScreenX != 0 || darwinMainScreenY != 0) {
        for (i = 0; i < pScreenInfo->numScreens; i++) {
            dixScreenOrigins[i].x -= darwinMainScreenX;
            dixScreenOrigins[i].y -= darwinMainScreenY;
    /*            ErrorF("Screen %d placed at X11 coordinate (%d,%d).\n",
		  i, dixScreenOrigins[i].x, dixScreenOrigins[i].y); */
        }
    }
}


/*
 * InitOutput
 *  Initialize screenInfo for all actually accessible framebuffers.
 *
 *  The display mode dependent code gets called three times. The mode
 *  specific InitOutput routines are expected to discover the number
 *  of potentially useful screens and cache routes to them internally.
 *  Inside DarwinAddScreen are two other mode specific calls.
 *  A mode specific AddScreen routine is called for each screen to
 *  actually initialize the screen with the ScreenPtr structure.
 *  After other screen setup has been done, a mode specific
 *  SetupScreen function can be called to finalize screen setup.
 */
void InitOutput( ScreenInfo *pScreenInfo, int argc, char **argv )
{
    int i;

    pScreenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

    // List how we want common pixmap formats to be padded
    pScreenInfo->numPixmapFormats = NUMFORMATS;
    for (i = 0; i < NUMFORMATS; i++)
        pScreenInfo->formats[i] = formats[i];

    // Discover screens and do mode specific initialization
    QuartzInitOutput(argc, argv);

    // Add screens
    for (i = 0; i < darwinScreensFound; i++) {
        AddScreen( DarwinAddScreen, argc, argv );
    }

    DarwinAdjustScreenOrigins(pScreenInfo);
}


/*
 * OsVendorFataError
 */
void OsVendorFatalError( void )
{
    ErrorF( "   OsVendorFatalError\n" );
}


/*
 * OsVendorInit
 *  Initialization of Darwin OS support.
 */
void OsVendorInit(void)
{
    if (serverGeneration == 1) {
        DarwinPrintBanner();
#ifdef ENABLE_DEBUG_LOG
	{
	  char *home_dir=NULL, *log_file_path=NULL;
	  home_dir = getenv("HOME");
	  if (home_dir) asprintf(&log_file_path, "%s/%s", home_dir, DEBUG_LOG_NAME);
	  if (log_file_path) {
	    if (!access(log_file_path, F_OK)) {
	      debug_log_fp = fopen(log_file_path, "a");
	      if (debug_log_fp) ErrorF("Debug logging enabled to %s\n", log_file_path);
	    }
	    free(log_file_path);
	  }
	}
#endif
    }
    //    DEBUG_LOG("Xquartz started at %s\n", ctime(time(NULL)));

    // Find the full path to the keymapping file.
    if ( darwinKeymapFile ) {
        char *tempStr = DarwinFindLibraryFile(darwinKeymapFile, "Keyboards");
        if ( !tempStr ) {
            ErrorF("Could not find keymapping file %s.\n", darwinKeymapFile);
        } else {
            ErrorF("Using keymapping provided in %s.\n", tempStr);
        }
        darwinKeymapFile = tempStr;
    }
}


/*
 * ddxInitGlobals
 *  Called by InitGlobals() from os/util.c.
 */
void ddxInitGlobals(void)
{
}


/*
 * ddxProcessArgument
 *  Process device-dependent command line args. Returns 0 if argument is
 *  not device dependent, otherwise Count of number of elements of argv
 *  that are part of a device dependent commandline option.
 */
int ddxProcessArgument( int argc, char *argv[], int i )
{
//    if ( !strcmp( argv[i], "-fullscreen" ) ) {
//        ErrorF( "Running full screen in parallel with Mac OS X Quartz window server.\n" );
//        return 1;
//    }

//    if ( !strcmp( argv[i], "-rootless" ) ) {
//        ErrorF( "Running rootless inside Mac OS X window server.\n" );
//        return 1;
//    }

    // This command line arg is passed when launched from the Aqua GUI.
    if ( !strncmp( argv[i], "-psn_", 5 ) ) {
        return 1;
    }

    if ( !strcmp( argv[i], "-fakebuttons" ) ) {
        darwinFakeButtons = TRUE;
        ErrorF( "Faking a three button mouse\n" );
        return 1;
    }

    if ( !strcmp( argv[i], "-nofakebuttons" ) ) {
        darwinFakeButtons = FALSE;
        ErrorF( "Not faking a three button mouse\n" );
        return 1;
    }

    if (!strcmp( argv[i], "-fakemouse2" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-fakemouse2 must be followed by a modifer list\n" );
        }
        if (!strcasecmp(argv[i+1], "none") || !strcmp(argv[i+1], ""))
            darwinFakeMouse2Mask = 0;
        else
            darwinFakeMouse2Mask = DarwinParseModifierList(argv[i+1]);
        ErrorF("Modifier mask to fake mouse button 2 = 0x%x\n",
               darwinFakeMouse2Mask);
        return 2;
    }

    if (!strcmp( argv[i], "-fakemouse3" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-fakemouse3 must be followed by a modifer list\n" );
        }
        if (!strcasecmp(argv[i+1], "none") || !strcmp(argv[i+1], ""))
            darwinFakeMouse3Mask = 0;
        else
            darwinFakeMouse3Mask = DarwinParseModifierList(argv[i+1]);
        ErrorF("Modifier mask to fake mouse button 3 = 0x%x\n",
               darwinFakeMouse3Mask);
        return 2;
    }

    if ( !strcmp( argv[i], "-keymap" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-keymap must be followed by a filename\n" );
        }
        darwinKeymapFile = argv[i+1];
        return 2;
    }

    if ( !strcmp( argv[i], "-nokeymap" ) ) {
        darwinKeymapFile = NULL;
        return 1;
    }

    if ( !strcmp( argv[i], "+synckeymap" ) ) {
        darwinSyncKeymap = TRUE;
        return 1;
    }

    if ( !strcmp( argv[i], "-synckeymap" ) ) {
        darwinSyncKeymap = FALSE;
        return 1;
    }

    if ( !strcmp( argv[i], "-size" ) ) {
        if ( i >= argc-2 ) {
            FatalError( "-size must be followed by two numbers\n" );
        }
#ifdef OLD_POWERBOOK_G3
        ErrorF( "Ignoring unsupported -size option on old PowerBook G3\n" );
#else
        darwinDesiredWidth = atoi( argv[i+1] );
        darwinDesiredHeight = atoi( argv[i+2] );
        ErrorF( "Attempting to use width x height = %i x %i\n",
                darwinDesiredWidth, darwinDesiredHeight );
#endif
        return 3;
    }

    if ( !strcmp( argv[i], "-depth" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-depth must be followed by a number\n" );
        }
#ifdef OLD_POWERBOOK_G3
        ErrorF( "Ignoring unsupported -depth option on old PowerBook G3\n");
#else
        darwinDesiredDepth = atoi( argv[i+1] );
        if(darwinDesiredDepth != -1 &&
           darwinDesiredDepth != 8 &&
           darwinDesiredDepth != 15 &&
           darwinDesiredDepth != 24) {
            FatalError( "Unsupported pixel depth. Use 8, 15, or 24 bits\n" );
        }

        ErrorF( "Attempting to use pixel depth of %i\n", darwinDesiredDepth );
#endif
        return 2;
    }

    if ( !strcmp( argv[i], "-refresh" ) ) {
        if ( i == argc-1 ) {
            FatalError( "-refresh must be followed by a number\n" );
        }
#ifdef OLD_POWERBOOK_G3
        ErrorF( "Ignoring unsupported -refresh option on old PowerBook G3\n");
#else
        darwinDesiredRefresh = atoi( argv[i+1] );
        ErrorF( "Attempting to use refresh rate of %i\n", darwinDesiredRefresh );
#endif
        return 2;
    }

    if (!strcmp( argv[i], "-showconfig" ) || !strcmp( argv[i], "-version" )) {
        DarwinPrintBanner();
        exit(0);
    }

    return 0;
}


/*
 * ddxUseMsg --
 *  Print out correct use of device dependent commandline options.
 *  Maybe the user now knows what really to do ...
 */
void ddxUseMsg( void )
{
    ErrorF("\n");
    ErrorF("\n");
    ErrorF("Device Dependent Usage:\n");
    ErrorF("\n");
    ErrorF("-fakebuttons : fake a three button mouse with Command and Option keys.\n");
    ErrorF("-nofakebuttons : don't fake a three button mouse.\n");
    ErrorF("-fakemouse2 <modifiers> : fake middle mouse button with modifier keys.\n");
    ErrorF("-fakemouse3 <modifiers> : fake right mouse button with modifier keys.\n");
    ErrorF("  ex: -fakemouse2 \"option,shift\" = option-shift-click is middle button.\n");
    ErrorF("-keymap <file> : read the keymapping from a file instead of the kernel.\n");
    ErrorF("-version : show the server version.\n");
    ErrorF("\n");
//    ErrorF("Quartz modes (Experimental / In Development):\n");
//    ErrorF("-fullscreen : run full screen in parallel with Mac OS X window server.\n");
//    ErrorF("-rootless : run rootless inside Mac OS X window server.\n");
    ErrorF("\n");
    ErrorF("Options ignored in rootless mode:\n");
    ErrorF("-size <height> <width> : use a screen resolution of <height> x <width>.\n");
    ErrorF("-depth <8,15,24> : use this bit depth.\n");
    ErrorF("-refresh <rate> : use a monitor refresh rate of <rate> Hz.\n");
    ErrorF("\n");
}


/*
 * ddxGiveUp --
 *      Device dependent cleanup. Called by dix before normal server death.
 */
void ddxGiveUp( void )
{
    ErrorF( "Quitting XQuartz...\n" );

    //if (!quartzRootless)
    //    quartzProcs->ReleaseScreens();
}


/*
 * AbortDDX --
 *      DDX - specific abort routine.  Called by AbortServer(). The attempt is
 *      made to restore all original setting of the displays. Also all devices
 *      are closed.
 */
void AbortDDX( void )
{
    ErrorF( "   AbortDDX\n" );
    /*
     * This is needed for a abnormal server exit, since the normal exit stuff
     * MUST also be performed (i.e. the vt must be left in a defined state)
     */
    ddxGiveUp();
}

#include "mivalidate.h" // for union _Validate used by windowstr.h
#include "windowstr.h"  // for struct _Window
#include "scrnintstr.h" // for struct _Screen

// This is copied from Xserver/hw/xfree86/common/xf86Helper.c.
// Quartz mode uses this when switching in and out of Quartz.
// Quartz or IOKit can use this when waking from sleep.
// Copyright (c) 1997-1998 by The XFree86 Project, Inc.

/*
 * xf86SetRootClip --
 *	Enable or disable rendering to the screen by
 *	setting the root clip list and revalidating
 *	all of the windows
 */

void
xf86SetRootClip (ScreenPtr pScreen, BOOL enable)
{
    WindowPtr	pWin = WindowTable[pScreen->myNum];
    WindowPtr	pChild;
    Bool	WasViewable = (Bool)(pWin->viewable);
    Bool	anyMarked = TRUE;
    RegionPtr	pOldClip = NULL, bsExposed;
#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif
    WindowPtr   pLayerWin;
    BoxRec	box;

    if (WasViewable)
    {
	for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	{
	    (void) (*pScreen->MarkOverlappedWindows)(pChild,
						     pChild,
						     &pLayerWin);
	}
	(*pScreen->MarkWindow) (pWin);
	anyMarked = TRUE;
	if (pWin->valdata)
	{
	    if (HasBorder (pWin))
	    {
		RegionPtr	borderVisible;

		borderVisible = REGION_CREATE(pScreen, NullBox, 1);
		REGION_SUBTRACT(pScreen, borderVisible,
				&pWin->borderClip, &pWin->winSize);
		pWin->valdata->before.borderVisible = borderVisible;
	    }
	    pWin->valdata->before.resized = TRUE;
	}
    }

    /*
     * Use REGION_BREAK to avoid optimizations in ValidateTree
     * that assume the root borderClip can't change well, normally
     * it doesn't...)
     */
    if (enable)
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pScreen->width;
	box.y2 = pScreen->height;
	REGION_RESET(pScreen, &pWin->borderClip, &box);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }
    else
    {
	REGION_EMPTY(pScreen, &pWin->borderClip);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }

    ResizeChildrenWinSize (pWin, 0, 0, 0, 0);

    if (WasViewable)
    {
	if (pWin->backStorage)
	{
	    pOldClip = REGION_CREATE(pScreen, NullBox, 1);
	    REGION_COPY(pScreen, pOldClip, &pWin->clipList);
	}

	if (pWin->firstChild)
	{
	    anyMarked |= (*pScreen->MarkOverlappedWindows)(pWin->firstChild,
							   pWin->firstChild,
							   (WindowPtr *)NULL);
	}
	else
	{
	    (*pScreen->MarkWindow) (pWin);
	    anyMarked = TRUE;
	}

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS(pWin))
	{
	    dosave = (*pScreen->ChangeSaveUnder)(pLayerWin, pLayerWin);
	}
#endif /* DO_SAVE_UNDERS */

	if (anyMarked)
	    (*pScreen->ValidateTree)(pWin, NullWindow, VTOther);
    }

    if (pWin->backStorage &&
	((pWin->backingStore == Always) || WasViewable))
    {
	if (!WasViewable)
	    pOldClip = &pWin->clipList; /* a convenient empty region */
	bsExposed = (*pScreen->TranslateBackingStore)
			     (pWin, 0, 0, pOldClip,
			      pWin->drawable.x, pWin->drawable.y);
	if (WasViewable)
	    REGION_DESTROY(pScreen, pOldClip);
	if (bsExposed)
	{
	    RegionPtr	valExposed = NullRegion;

	    if (pWin->valdata)
		valExposed = &pWin->valdata->after.exposed;
	    (*pScreen->WindowExposures) (pWin, valExposed, bsExposed);
	    if (valExposed)
		REGION_EMPTY(pScreen, valExposed);
	    REGION_DESTROY(pScreen, bsExposed);
	}
    }
    if (WasViewable)
    {
	if (anyMarked)
	    (*pScreen->HandleExposures)(pWin);
#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder)(pLayerWin, pLayerWin);
#endif /* DO_SAVE_UNDERS */
	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree)(pWin, NullWindow, VTOther);
    }
    if (pWin->realized)
	WindowsRestructured ();
    FlushAllOutput ();
}
