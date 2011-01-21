/*
 *Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 *
 *Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 *"Software"), to deal in the Software without restriction, including
 *without limitation the rights to use, copy, modify, merge, publish,
 *distribute, sublicense, and/or sell copies of the Software, and to
 *permit persons to whom the Software is furnished to do so, subject to
 *the following conditions:
 *
 *The above copyright notice and this permission notice shall be
 *included in all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 *ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *Except as contained in this notice, the name of the XFree86 Project
 *shall not be used in advertising or otherwise to promote the sale, use
 *or other dealings in this Software without prior written authorization
 *from the XFree86 Project.
 *
 * Authors:	Dakshinamurthy Karra
 *		Suhaib M Siddiqi
 *		Peter Busch
 *		Harold L Hunt II
 *		Kensuke Matsuzaki
 */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"
#include "winmsg.h"


#ifdef XWIN_MULTIWINDOWEXTWM
static RootlessFrameProcsRec
winMWExtWMProcs = {	
  winMWExtWMCreateFrame,
  winMWExtWMDestroyFrame,
  
  winMWExtWMMoveFrame,
  winMWExtWMResizeFrame,
  winMWExtWMRestackFrame,
  winMWExtWMReshapeFrame,
  winMWExtWMUnmapFrame,
  
  winMWExtWMStartDrawing,
  winMWExtWMStopDrawing,
  winMWExtWMUpdateRegion,
  winMWExtWMDamageRects,
  winMWExtWMRootlessSwitchWindow,
  NULL,//winMWExtWMDoReorderWindow,
  NULL,//winMWExtWMHideWindow,
  NULL,//winMWExtWMUpdateColorMap,

  NULL,//winMWExtWMCopyBytes,
  winMWExtWMCopyWindow
};
#endif

/*
 * Prototypes
 */

/*
 * Local functions
 */

static Bool
winSaveScreen (ScreenPtr pScreen, int on);


/*
 * Determine what type of screen we are initializing
 * and call the appropriate procedure to intiailize
 * that type of screen.
 */

Bool
winScreenInit (int index,
	       ScreenPtr pScreen,
	       int argc, char **argv)
{
  winScreenInfoPtr      pScreenInfo = &g_ScreenInfo[index];
  winPrivScreenPtr	pScreenPriv;
  HDC			hdc;
  DWORD dwInitialBPP;

#if CYGDEBUG || YES
  winDebug ("winScreenInit - dwWidth: %ld dwHeight: %ld\n",
	  pScreenInfo->dwWidth, pScreenInfo->dwHeight);
#endif

  /* Allocate privates for this screen */
  if (!winAllocatePrivates (pScreen))
    {
      ErrorF ("winScreenInit - Couldn't allocate screen privates\n");
      return FALSE;
    }

  /* Get a pointer to the privates structure that was allocated */
  pScreenPriv = winGetScreenPriv (pScreen);

  /* Save a pointer to this screen in the screen info structure */
  pScreenInfo->pScreen = pScreen;

  /* Save a pointer to the screen info in the screen privates structure */
  /* This allows us to get back to the screen info from a screen pointer */
  pScreenPriv->pScreenInfo = pScreenInfo;

  /*
   * Determine which engine to use.
   *
   * NOTE: This is done once per screen because each screen possibly has
   * a preferred engine specified on the command line.
   */
  if (!winSetEngine (pScreen))
    {
      ErrorF ("winScreenInit - winSetEngine () failed\n");
      return FALSE;
    }

  /* Horribly misnamed function: Allow engine to adjust BPP for screen */
  dwInitialBPP = pScreenInfo->dwBPP;

  if (!(*pScreenPriv->pwinAdjustVideoMode) (pScreen))
    {
      ErrorF ("winScreenInit - winAdjustVideoMode () failed\n");
      return FALSE;
    }

  if (dwInitialBPP == WIN_DEFAULT_BPP)
    {
      /* No -depth parameter was passed, let the user know the depth being used */
      ErrorF ("winScreenInit - Using Windows display depth of %d bits per pixel\n", (int) pScreenInfo->dwBPP);
    }
  else if (dwInitialBPP != pScreenInfo->dwBPP)
    {
      /* Warn user if engine forced a depth different to -depth parameter */
      ErrorF ("winScreenInit - Command line depth of %d bpp overidden by engine, using %d bpp\n", (int) dwInitialBPP, (int) pScreenInfo->dwBPP);
    }
  else
    {
      ErrorF ("winScreenInit - Using command line depth of %d bpp\n", (int) pScreenInfo->dwBPP);
    }

  /* Check for supported display depth */
  if (!(WIN_SUPPORTED_BPPS & (1 << (pScreenInfo->dwBPP - 1))))
    {
      ErrorF ("winScreenInit - Unsupported display depth: %d\n" \
	      "Change your Windows display depth to 15, 16, 24, or 32 bits "
	      "per pixel.\n",
	      (int) pScreenInfo->dwBPP);
      ErrorF ("winScreenInit - Supported depths: %08x\n",
	      WIN_SUPPORTED_BPPS);
#if WIN_CHECK_DEPTH
      return FALSE;
#endif
    }

  /*
   * Check that all monitors have the same display depth if we are using
   * multiple monitors
   */
  if (pScreenInfo->fMultipleMonitors
      && !GetSystemMetrics (SM_SAMEDISPLAYFORMAT))
    {
      ErrorF ("winScreenInit - Monitors do not all have same pixel format / "
	      "display depth.\n");
      if (pScreenInfo->dwEngine == WIN_SERVER_SHADOW_GDI)
        {
          ErrorF ("winScreenInit - Performance may suffer off primary display.\n");
        }
      else
        {
          ErrorF ("winScreenInit - Using primary display only.\n");
          pScreenInfo->fMultipleMonitors = FALSE;
        }
    }

  /* Create display window */
  if (!(*pScreenPriv->pwinCreateBoundingWindow) (pScreen))
    {
      ErrorF ("winScreenInit - pwinCreateBoundingWindow () "
	      "failed\n");
      return FALSE;
    }

  /* Get a device context */
  hdc = GetDC (pScreenPriv->hwndScreen);

  /* Are we using multiple monitors? */
  if (pScreenInfo->fMultipleMonitors)
    {
      /* 
       * In this case, some of the defaults set in
       * winInitializeScreenDefaults() are not correct ...
       */
      if (!pScreenInfo->fUserGaveHeightAndWidth)
	{
	  pScreenInfo->dwWidth = GetSystemMetrics (SM_CXVIRTUALSCREEN);
	  pScreenInfo->dwHeight = GetSystemMetrics (SM_CYVIRTUALSCREEN);
	}
    }

  /* Release the device context */
  ReleaseDC (pScreenPriv->hwndScreen, hdc);
    
  /* Clear the visuals list */
  miClearVisualTypes ();

  /* Call the engine dependent screen initialization procedure */
  if (!((*pScreenPriv->pwinFinishScreenInit) (index, pScreen, argc, argv)))
    {
      ErrorF ("winScreenInit - winFinishScreenInit () failed\n");
      return FALSE;
    }

  if (!g_fSoftwareCursor)
    winInitCursor(pScreen);
  else
    winErrorFVerb(2, "winScreenInit - Using software cursor\n");  

  /*
     Note the screen origin in a normalized coordinate space where (0,0) is at the top left
     of the native virtual desktop area
  */
  pScreen->x = pScreenInfo->dwInitialX - GetSystemMetrics(SM_XVIRTUALSCREEN);
  pScreen->y = pScreenInfo->dwInitialY - GetSystemMetrics(SM_YVIRTUALSCREEN);

  ErrorF("Screen %d added at virtual desktop coordinate (%d,%d).\n",
         index, pScreen->x, pScreen->y);

#if CYGDEBUG || YES
  winDebug ("winScreenInit - returning\n");
#endif

  return TRUE;
}

static Bool
winCreateScreenResources(ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  Bool result;

  result = pScreenPriv->pwinCreateScreenResources(pScreen);

  /* Now the screen bitmap has been wrapped in a pixmap,
     add that to the Shadow framebuffer */
  if (!shadowAdd(pScreen, pScreen->devPrivate,
		 pScreenPriv->pwinShadowUpdate, NULL, 0, 0))
    {
      ErrorF ("winCreateScreenResources - shadowAdd () failed\n");
      return FALSE;
    }

  return result;
}

/* See Porting Layer Definition - p. 20 */
Bool
winFinishScreenInitFB (int index,
		       ScreenPtr pScreen,
		       int argc, char **argv)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  VisualPtr		pVisual = NULL;
  char			*pbits = NULL;
#if defined(XWIN_CLIPBOARD) || defined(XWIN_MULTIWINDOW)
  int			iReturn;
#endif

  /* Create framebuffer */
  if (!(*pScreenPriv->pwinInitScreen) (pScreen))
    {
      ErrorF ("winFinishScreenInitFB - Could not allocate framebuffer\n");
      return FALSE;
    }

  /*
   * Calculate the number of bits that are used to represent color in each pixel,
   * the color depth for the screen
   */
  if (pScreenInfo->dwBPP == 8)
    pScreenInfo->dwDepth = 8;
  else
    pScreenInfo->dwDepth = winCountBits (pScreenPriv->dwRedMask)
      + winCountBits (pScreenPriv->dwGreenMask)
      + winCountBits (pScreenPriv->dwBlueMask);

  winErrorFVerb (2, "winFinishScreenInitFB - Masks: %08x %08x %08x\n",
	  (unsigned int) pScreenPriv->dwRedMask,
	  (unsigned int) pScreenPriv->dwGreenMask,
	  (unsigned int) pScreenPriv->dwBlueMask);

  /* Init visuals */
  if (!(*pScreenPriv->pwinInitVisuals) (pScreen))
    {
      ErrorF ("winFinishScreenInitFB - winInitVisuals failed\n");
      return FALSE;
    }

  /* Setup a local variable to point to the framebuffer */
  pbits = pScreenInfo->pfb;
  
  /* Apparently we need this for the render extension */
  miSetPixmapDepths ();

  /* Start fb initialization */
  if (!fbSetupScreen (pScreen,
		      pScreenInfo->pfb,
		      pScreenInfo->dwWidth, pScreenInfo->dwHeight,
		      monitorResolution, monitorResolution,
		      pScreenInfo->dwStride,
		      pScreenInfo->dwBPP))
    {
      ErrorF ("winFinishScreenInitFB - fbSetupScreen failed\n");
      return FALSE;
    }

  /* Override default colormap routines if visual class is dynamic */
  if (pScreenInfo->dwDepth == 8
      && (pScreenInfo->dwEngine == WIN_SERVER_SHADOW_GDI
	  || (pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DDNL
	      && pScreenInfo->fFullScreen)
	  || (pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DD
	      && pScreenInfo->fFullScreen)))
    {
      winSetColormapFunctions (pScreen);

      /*
       * NOTE: Setting whitePixel to 255 causes Magic 7.1 to allocate its
       * own colormap, as it cannot allocate 7 planes in the default
       * colormap.  Setting whitePixel to 1 allows Magic to get 7
       * planes in the default colormap, so it doesn't create its
       * own colormap.  This latter situation is highly desireable,
       * as it keeps the Magic window viewable when switching to
       * other X clients that use the default colormap.
       */
      pScreen->blackPixel = 0;
      pScreen->whitePixel = 1;
    }

  /* Place our save screen function */
  pScreen->SaveScreen = winSaveScreen;

  /* Finish fb initialization */
  if (!fbFinishScreenInit (pScreen,
			   pScreenInfo->pfb,
			   pScreenInfo->dwWidth, pScreenInfo->dwHeight,
			   monitorResolution, monitorResolution,
			   pScreenInfo->dwStride,
			   pScreenInfo->dwBPP))
    {
      ErrorF ("winFinishScreenInitFB - fbFinishScreenInit failed\n");
      return FALSE;
    }

  /* Save a pointer to the root visual */
  for (pVisual = pScreen->visuals;
       pVisual->vid != pScreen->rootVisual;
       pVisual++);
  pScreenPriv->pRootVisual = pVisual;

  /* 
   * Setup points to the block and wakeup handlers.  Pass a pointer
   * to the current screen as pWakeupdata.
   */
  pScreen->BlockHandler = winBlockHandler;
  pScreen->WakeupHandler = winWakeupHandler;
  pScreen->blockData = pScreen;
  pScreen->wakeupData = pScreen;

  /* Render extension initialization, calls miPictureInit */
  if (!fbPictureInit (pScreen, NULL, 0))
    {
      ErrorF ("winFinishScreenInitFB - fbPictureInit () failed\n");
      return FALSE;
    }

#ifdef RANDR
  /* Initialize resize and rotate support */
  if (!winRandRInit (pScreen))
    {
      ErrorF ("winFinishScreenInitFB - winRandRInit () failed\n");
      return FALSE;
    }
#endif

  /* Setup the cursor routines */
#if CYGDEBUG
  winDebug ("winFinishScreenInitFB - Calling miDCInitialize ()\n");
#endif
  miDCInitialize (pScreen, &g_winPointerCursorFuncs);

  /* KDrive does winCreateDefColormap right after miDCInitialize */
  /* Create a default colormap */
#if CYGDEBUG
  winDebug ("winFinishScreenInitFB - Calling winCreateDefColormap ()\n");
#endif
  if (!winCreateDefColormap (pScreen))
    {
      ErrorF ("winFinishScreenInitFB - Could not create colormap\n");
      return FALSE;
    }

  /* Initialize the shadow framebuffer layer */
  if ((pScreenInfo->dwEngine == WIN_SERVER_SHADOW_GDI
       || pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DD
       || pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DDNL)
#ifdef XWIN_MULTIWINDOWEXTWM
      && !pScreenInfo->fMWExtWM
#endif
      )
    {
#if CYGDEBUG
      winDebug ("winFinishScreenInitFB - Calling shadowSetup ()\n");
#endif
      if (!shadowSetup(pScreen))
	{
	  ErrorF ("winFinishScreenInitFB - shadowSetup () failed\n");
	  return FALSE;
	}

      /* Wrap CreateScreenResources so we can add the screen pixmap
         to the Shadow framebuffer after it's been created */
      pScreenPriv->pwinCreateScreenResources = pScreen->CreateScreenResources;
      pScreen->CreateScreenResources = winCreateScreenResources;
    }

#ifdef XWIN_MULTIWINDOWEXTWM
  /* Handle multi-window external window manager mode */
  if (pScreenInfo->fMWExtWM)
    {
      winDebug ("winScreenInit - MultiWindowExtWM - Calling RootlessInit\n");
      
      RootlessInit(pScreen, &winMWExtWMProcs);
      
      winDebug ("winScreenInit - MultiWindowExtWM - RootlessInit returned\n");
      
      rootless_CopyBytes_threshold = 0;
      /* FIXME: How many? Profiling needed? */
      rootless_CopyWindow_threshold = 1;

      winWindowsWMExtensionInit ();
    }
#endif

  /* Handle rootless mode */
  if (pScreenInfo->fRootless)
    {
      /* Define the WRAP macro temporarily for local use */
#define WRAP(a) \
    if (pScreen->a) { \
        pScreenPriv->a = pScreen->a; \
    } else { \
        ErrorF("null screen fn " #a "\n"); \
        pScreenPriv->a = NULL; \
    }

      /* Save a pointer to each lower-level window procedure */
      WRAP(CreateWindow);
      WRAP(DestroyWindow);
      WRAP(RealizeWindow);
      WRAP(UnrealizeWindow);
      WRAP(PositionWindow);
      WRAP(ChangeWindowAttributes);
      WRAP(SetShape);

      /* Assign rootless window procedures to be top level procedures */
      pScreen->CreateWindow = winCreateWindowRootless;
      pScreen->DestroyWindow = winDestroyWindowRootless;
      pScreen->PositionWindow = winPositionWindowRootless;
      /*pScreen->ChangeWindowAttributes = winChangeWindowAttributesRootless;*/
      pScreen->RealizeWindow = winMapWindowRootless;
      pScreen->UnrealizeWindow = winUnmapWindowRootless;
      pScreen->SetShape = winSetShapeRootless;

      /* Undefine the WRAP macro, as it is not needed elsewhere */
#undef WRAP
    }


#ifdef XWIN_MULTIWINDOW
  /* Handle multi window mode */
  else if (pScreenInfo->fMultiWindow)
    {
      /* Define the WRAP macro temporarily for local use */
#define WRAP(a) \
    if (pScreen->a) { \
        pScreenPriv->a = pScreen->a; \
    } else { \
        ErrorF("null screen fn " #a "\n"); \
        pScreenPriv->a = NULL; \
    }

      /* Save a pointer to each lower-level window procedure */
      WRAP(CreateWindow);
      WRAP(DestroyWindow);
      WRAP(RealizeWindow);
      WRAP(UnrealizeWindow);
      WRAP(PositionWindow);
      WRAP(ChangeWindowAttributes);
      WRAP(ReparentWindow);
      WRAP(RestackWindow);
      WRAP(ResizeWindow);
      WRAP(MoveWindow);
      WRAP(CopyWindow);
      WRAP(SetShape);

      /* Assign multi-window window procedures to be top level procedures */
      pScreen->CreateWindow = winCreateWindowMultiWindow;
      pScreen->DestroyWindow = winDestroyWindowMultiWindow;
      pScreen->PositionWindow = winPositionWindowMultiWindow;
      /*pScreen->ChangeWindowAttributes = winChangeWindowAttributesMultiWindow;*/
      pScreen->RealizeWindow = winMapWindowMultiWindow;
      pScreen->UnrealizeWindow = winUnmapWindowMultiWindow;
      pScreen->ReparentWindow = winReparentWindowMultiWindow;
      pScreen->RestackWindow = winRestackWindowMultiWindow;
      pScreen->ResizeWindow = winResizeWindowMultiWindow;
      pScreen->MoveWindow = winMoveWindowMultiWindow;
      pScreen->CopyWindow = winCopyWindowMultiWindow;
      pScreen->SetShape = winSetShapeMultiWindow;

      /* Undefine the WRAP macro, as it is not needed elsewhere */
#undef WRAP
    }
#endif

  /* Wrap either fb's or shadow's CloseScreen with our CloseScreen */
  pScreenPriv->CloseScreen = pScreen->CloseScreen;
  pScreen->CloseScreen = pScreenPriv->pwinCloseScreen;

#if defined(XWIN_CLIPBOARD) || defined(XWIN_MULTIWINDOW)
  /* Create a mutex for modules in separate threads to wait for */
  iReturn = pthread_mutex_init (&pScreenPriv->pmServerStarted, NULL);
  if (iReturn != 0)
    {
      ErrorF ("winFinishScreenInitFB - pthread_mutex_init () failed: %d\n",
	      iReturn);
      return FALSE;
    }

  /* Own the mutex for modules in separate threads */
  iReturn = pthread_mutex_lock (&pScreenPriv->pmServerStarted);
  if (iReturn != 0)
    {
      ErrorF ("winFinishScreenInitFB - pthread_mutex_lock () failed: %d\n",
	      iReturn);
      return FALSE;
    }

  /* Set the ServerStarted flag to false */
  pScreenPriv->fServerStarted = FALSE;
#endif

#ifdef XWIN_MULTIWINDOWEXTWM
  pScreenPriv->fRestacking = FALSE;
#endif

#if defined(XWIN_MULTIWINDOW) || defined(XWIN_MULTIWINDOWEXTWM)
  if (FALSE
#ifdef XWIN_MULTIWINDOW
      || pScreenInfo->fMultiWindow
#endif
#ifdef XWIN_MULTIWINDOWEXTWM
      || pScreenInfo->fInternalWM
#endif
      )
    { 
#if CYGDEBUG || YES
      winDebug ("winFinishScreenInitFB - Calling winInitWM.\n");
#endif

      /* Initialize multi window mode */
      if (!winInitWM (&pScreenPriv->pWMInfo,
		      &pScreenPriv->ptWMProc,
		      &pScreenPriv->ptXMsgProc,
		      &pScreenPriv->pmServerStarted,
		      pScreenInfo->dwScreen,
		      (HWND)&pScreenPriv->hwndScreen,
#ifdef XWIN_MULTIWINDOWEXTWM
		      pScreenInfo->fInternalWM ||
#endif
		      FALSE))
        {
          ErrorF ("winFinishScreenInitFB - winInitWM () failed.\n");
          return FALSE;
        }
    }      
#endif

  /* Tell the server that we are enabled */
  pScreenPriv->fEnabled = TRUE;

  /* Tell the server that we have a valid depth */
  pScreenPriv->fBadDepth = FALSE;

#if CYGDEBUG || YES
  winDebug ("winFinishScreenInitFB - returning\n");
#endif

  return TRUE;
}

#ifdef XWIN_NATIVEGDI
/* See Porting Layer Definition - p. 20 */

Bool
winFinishScreenInitNativeGDI (int index,
			      ScreenPtr pScreen,
			      int argc, char **argv)
{
  winScreenPriv(pScreen);
  winScreenInfoPtr      pScreenInfo = &g_ScreenInfo[index];
  VisualPtr		pVisuals = NULL;
  DepthPtr		pDepths = NULL;
  VisualID		rootVisual = 0;
  int			nVisuals = 0, nDepths = 0, nRootDepth = 0;

  /* Ignore user input (mouse, keyboard) */
  pScreenInfo->fIgnoreInput = FALSE;

  /* Get device contexts for the screen and shadow bitmap */
  pScreenPriv->hdcScreen = GetDC (pScreenPriv->hwndScreen);
  if (pScreenPriv->hdcScreen == NULL)
    FatalError ("winFinishScreenInitNativeGDI - Couldn't get a DC\n");

  /* Init visuals */
  if (!(*pScreenPriv->pwinInitVisuals) (pScreen))
    {
      ErrorF ("winFinishScreenInitNativeGDI - pwinInitVisuals failed\n");
      return FALSE;
    }

  /* Initialize the mi visuals */
  if (!miInitVisuals (&pVisuals, &pDepths, &nVisuals, &nDepths, &nRootDepth,
		      &rootVisual,
		      ((unsigned long)1 << (pScreenInfo->dwDepth - 1)), 8,
		      TrueColor))
    {
      ErrorF ("winFinishScreenInitNativeGDI - miInitVisuals () failed\n");
      return FALSE;
    }

  /* Initialize the CloseScreen procedure pointer */
  pScreen->CloseScreen = NULL;

  /* Initialize the mi code */
  if (!miScreenInit (pScreen,
		     NULL, /* No framebuffer */
		     pScreenInfo->dwWidth, pScreenInfo->dwHeight,
		     monitorResolution, monitorResolution,
		     pScreenInfo->dwStride,
		     nRootDepth, nDepths, pDepths, rootVisual,
		     nVisuals, pVisuals))
    {
      ErrorF ("winFinishScreenInitNativeGDI - miScreenInit failed\n");
      return FALSE;
    }

  pScreen->defColormap = FakeClientID(0);

  /*
   * Register our block and wakeup handlers; these procedures
   * process messages in our Windows message queue; specifically,
   * they process mouse and keyboard input.
   */
  pScreen->BlockHandler = winBlockHandler;
  pScreen->WakeupHandler = winWakeupHandler;
  pScreen->blockData = pScreen;
  pScreen->wakeupData = pScreen;

  /* Place our save screen function */
  pScreen->SaveScreen = winSaveScreen;

  /* Pixmaps */
  pScreen->CreatePixmap = winCreatePixmapNativeGDI;
  pScreen->DestroyPixmap = winDestroyPixmapNativeGDI;

  /* Other Screen Routines */
  pScreen->QueryBestSize = winQueryBestSizeNativeGDI;
  pScreen->SaveScreen = winSaveScreen;  
  pScreen->GetImage = miGetImage;
  pScreen->GetSpans = winGetSpansNativeGDI;

  /* Window Procedures */
  pScreen->CreateWindow = winCreateWindowNativeGDI;
  pScreen->DestroyWindow = winDestroyWindowNativeGDI;
  pScreen->PositionWindow = winPositionWindowNativeGDI;
  /*pScreen->ChangeWindowAttributes = winChangeWindowAttributesNativeGDI;*/
  pScreen->RealizeWindow = winMapWindowNativeGDI;
  pScreen->UnrealizeWindow = winUnmapWindowNativeGDI;

  /* Paint window */
  pScreen->CopyWindow = winCopyWindowNativeGDI;

  /* Fonts */
  pScreen->RealizeFont = winRealizeFontNativeGDI;
  pScreen->UnrealizeFont = winUnrealizeFontNativeGDI;

  /* GC */
  pScreen->CreateGC = winCreateGCNativeGDI;

  /* Colormap Routines */
  pScreen->CreateColormap = miInitializeColormap;
  pScreen->DestroyColormap = (DestroyColormapProcPtr) (void (*)(void)) NoopDDA;
  pScreen->InstallColormap = miInstallColormap;
  pScreen->UninstallColormap = miUninstallColormap;
  pScreen->ListInstalledColormaps = miListInstalledColormaps;
  pScreen->StoreColors = (StoreColorsProcPtr) (void (*)(void)) NoopDDA;
  pScreen->ResolveColor = miResolveColor;

  /* Bitmap */
  pScreen->BitmapToRegion = winPixmapToRegionNativeGDI;

  ErrorF ("winFinishScreenInitNativeGDI - calling miDCInitialize\n");

  /* Set the default white and black pixel positions */
  pScreen->whitePixel = pScreen->blackPixel = (Pixel) 0;

  /* Initialize the cursor */
  if (!miDCInitialize (pScreen, &g_winPointerCursorFuncs))
    {
      ErrorF ("winFinishScreenInitNativeGDI - miDCInitialize failed\n");
      return FALSE;
    }
  
  /* Create a default colormap */
  if (!miCreateDefColormap (pScreen))
    {
        ErrorF ("winFinishScreenInitNativeGDI - miCreateDefColormap () "
		"failed\n");
	return FALSE;
    }

  ErrorF ("winFinishScreenInitNativeGDI - miCreateDefColormap () "
	  "returned\n");
  
  /* mi doesn't use a CloseScreen procedure, so no need to wrap */
  pScreen->CloseScreen = pScreenPriv->pwinCloseScreen;

  /* Tell the server that we are enabled */
  pScreenPriv->fEnabled = TRUE;

  ErrorF ("winFinishScreenInitNativeGDI - Successful addition of "
	  "screen %08x\n",
	  (unsigned int) pScreen);

  return TRUE;
}
#endif


/* See Porting Layer Definition - p. 33 */
static Bool
winSaveScreen (ScreenPtr pScreen, int on)
{
  return TRUE;
}
