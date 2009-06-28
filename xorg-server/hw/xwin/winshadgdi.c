/*
 *Copyright (C) 2001-2004 Harold L Hunt II All Rights Reserved.
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
 *NONINFRINGEMENT. IN NO EVENT SHALL HAROLD L HUNT II BE LIABLE FOR
 *ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 *CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *Except as contained in this notice, the name of Harold L Hunt II
 *shall not be used in advertising or otherwise to promote the sale, use
 *or other dealings in this Software without prior written authorization
 *from Harold L Hunt II.
 *
 * Authors:	Harold L Hunt II
 */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"


/*
 * External symbols
 */

#ifdef XWIN_MULTIWINDOW
extern DWORD			g_dwCurrentThreadID;
#endif
extern HWND			g_hDlgExit;


/*
 * Local function prototypes
 */

#ifdef XWIN_MULTIWINDOW
static wBOOL CALLBACK
winRedrawAllProcShadowGDI (HWND hwnd, LPARAM lParam);

static wBOOL CALLBACK
winRedrawDamagedWindowShadowGDI (HWND hwnd, LPARAM lParam);
#endif

static Bool
winAllocateFBShadowGDI (ScreenPtr pScreen);

static void
winShadowUpdateGDI (ScreenPtr pScreen, 
		    shadowBufPtr pBuf);

static Bool
winCloseScreenShadowGDI (int nIndex, ScreenPtr pScreen);

static Bool
winInitVisualsShadowGDI (ScreenPtr pScreen);

static Bool
winAdjustVideoModeShadowGDI (ScreenPtr pScreen);

static Bool
winBltExposedRegionsShadowGDI (ScreenPtr pScreen);

static Bool
winActivateAppShadowGDI (ScreenPtr pScreen);

static Bool
winRedrawScreenShadowGDI (ScreenPtr pScreen);

static Bool
winRealizeInstalledPaletteShadowGDI (ScreenPtr pScreen);

static Bool
winInstallColormapShadowGDI (ColormapPtr pColormap);

static Bool
winStoreColorsShadowGDI (ColormapPtr pmap, 
			 int ndef,
			 xColorItem *pdefs);

static Bool
winCreateColormapShadowGDI (ColormapPtr pColormap);

static Bool
winDestroyColormapShadowGDI (ColormapPtr pColormap);


/*
 * Internal function to get the DIB format that is compatible with the screen
 */

static
Bool
winQueryScreenDIBFormat (ScreenPtr pScreen, BITMAPINFOHEADER *pbmih)
{
  winScreenPriv(pScreen);
  HBITMAP		hbmp;
#if CYGDEBUG
  LPDWORD		pdw = NULL;
#endif
  
  /* Create a memory bitmap compatible with the screen */
  hbmp = CreateCompatibleBitmap (pScreenPriv->hdcScreen, 1, 1);
  if (hbmp == NULL)
    {
      ErrorF ("winQueryScreenDIBFormat - CreateCompatibleBitmap failed\n");
      return FALSE;
    }
  
  /* Initialize our bitmap info header */
  ZeroMemory (pbmih, sizeof (BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD));
  pbmih->biSize = sizeof (BITMAPINFOHEADER);

  /* Get the biBitCount */
  if (!GetDIBits (pScreenPriv->hdcScreen,
		  hbmp,
		  0, 1,
		  NULL,
		  (BITMAPINFO*) pbmih,
		  DIB_RGB_COLORS))
    {
      ErrorF ("winQueryScreenDIBFormat - First call to GetDIBits failed\n");
      DeleteObject (hbmp);
      return FALSE;
    }

#if CYGDEBUG
  /* Get a pointer to bitfields */
  pdw = (DWORD*) ((CARD8*)pbmih + sizeof (BITMAPINFOHEADER));

  winDebug ("winQueryScreenDIBFormat - First call masks: %08x %08x %08x\n",
	  pdw[0], pdw[1], pdw[2]);
#endif

  /* Get optimal color table, or the optimal bitfields */
  if (!GetDIBits (pScreenPriv->hdcScreen,
		  hbmp,
		  0, 1,
		  NULL,
		  (BITMAPINFO*)pbmih,
		  DIB_RGB_COLORS))
    {
      ErrorF ("winQueryScreenDIBFormat - Second call to GetDIBits "
	      "failed\n");
      DeleteObject (hbmp);
      return FALSE;
    }

  /* Free memory */
  DeleteObject (hbmp);
  
  return TRUE;
}


/*
 * Internal function to determine the GDI bits per rgb and bit masks
 */

static
Bool
winQueryRGBBitsAndMasks (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  BITMAPINFOHEADER	*pbmih = NULL;
  Bool			fReturn = TRUE;
  LPDWORD		pdw = NULL;
  DWORD			dwRedBits, dwGreenBits, dwBlueBits;

  /* Color masks for 8 bpp are standardized */
  if (GetDeviceCaps (pScreenPriv->hdcScreen, RASTERCAPS) & RC_PALETTE)
    {
      /* 
       * RGB BPP for 8 bit palletes is always 8
       * and the color masks are always 0.
       */
      pScreenPriv->dwBitsPerRGB = 8;
      pScreenPriv->dwRedMask = 0x0L;
      pScreenPriv->dwGreenMask = 0x0L;
      pScreenPriv->dwBlueMask = 0x0L;
      return TRUE;
    }

  /* Color masks for 24 bpp are standardized */
  if (GetDeviceCaps (pScreenPriv->hdcScreen, PLANES)
      * GetDeviceCaps (pScreenPriv->hdcScreen, BITSPIXEL) == 24)
    {
      ErrorF ("winQueryRGBBitsAndMasks - GetDeviceCaps (BITSPIXEL) "
	      "returned 24 for the screen.  Using default 24bpp masks.\n");

      /* 8 bits per primary color */
      pScreenPriv->dwBitsPerRGB = 8;

      /* Set screen privates masks */
      pScreenPriv->dwRedMask = WIN_24BPP_MASK_RED;
      pScreenPriv->dwGreenMask = WIN_24BPP_MASK_GREEN;
      pScreenPriv->dwBlueMask = WIN_24BPP_MASK_BLUE;
      
      return TRUE;
    }

  /* Allocate a bitmap header and color table */
  pbmih = (BITMAPINFOHEADER*) malloc (sizeof (BITMAPINFOHEADER)
				      + 256  * sizeof (RGBQUAD));
  if (pbmih == NULL)
    {
      ErrorF ("winQueryRGBBitsAndMasks - malloc failed\n");
      return FALSE;
    }

  /* Get screen description */
  if (winQueryScreenDIBFormat (pScreen, pbmih))
    {
      /* Get a pointer to bitfields */
      pdw = (DWORD*) ((CARD8*)pbmih + sizeof (BITMAPINFOHEADER));
      
#if CYGDEBUG
      winDebug ("%s - Masks: %08x %08x %08x\n", __FUNCTION__,
	      pdw[0], pdw[1], pdw[2]);
      winDebug ("%s - Bitmap: %dx%d %d bpp %d planes\n", __FUNCTION__,
              pbmih->biWidth, pbmih->biHeight, pbmih->biBitCount, pbmih->biPlanes);
      winDebug ("%s - Compression: %d %s\n", __FUNCTION__,
              pbmih->biCompression,
              (pbmih->biCompression == BI_RGB?"(BI_RGB)":
               (pbmih->biCompression == BI_RLE8?"(BI_RLE8)":
                (pbmih->biCompression == BI_RLE4?"(BI_RLE4)":
                 (pbmih->biCompression == BI_BITFIELDS?"(BI_BITFIELDS)":""
                 )))));
#endif

      /* Handle BI_RGB case, which is returned by Wine */
      if (pbmih->biCompression == BI_RGB)
        {
	  dwRedBits = 5;
	  dwGreenBits = 5;
	  dwBlueBits = 5;
	  
	  pScreenPriv->dwBitsPerRGB = 5;
	  
	  /* Set screen privates masks */
	  pScreenPriv->dwRedMask = 0x7c00;
	  pScreenPriv->dwGreenMask = 0x03e0;
	  pScreenPriv->dwBlueMask = 0x001f;
        }
      else 
        {
          /* Count the number of bits in each mask */
          dwRedBits = winCountBits (pdw[0]);
          dwGreenBits = winCountBits (pdw[1]);
          dwBlueBits = winCountBits (pdw[2]);

	  /* Find maximum bits per red, green, blue */
	  if (dwRedBits > dwGreenBits && dwRedBits > dwBlueBits)
	    pScreenPriv->dwBitsPerRGB = dwRedBits;
	  else if (dwGreenBits > dwRedBits && dwGreenBits > dwBlueBits)
	    pScreenPriv->dwBitsPerRGB = dwGreenBits;
	  else
	    pScreenPriv->dwBitsPerRGB = dwBlueBits;

	  /* Set screen privates masks */
	  pScreenPriv->dwRedMask = pdw[0];
	  pScreenPriv->dwGreenMask = pdw[1];
	  pScreenPriv->dwBlueMask = pdw[2];
	}
    }
  else
    {
      ErrorF ("winQueryRGBBitsAndMasks - winQueryScreenDIBFormat failed\n");
      free (pbmih);
      fReturn = FALSE;
    }

  /* Free memory */
  free (pbmih);

  return fReturn;
}


#ifdef XWIN_MULTIWINDOW
/*
 * Redraw all ---?
 */

static wBOOL CALLBACK
winRedrawAllProcShadowGDI (HWND hwnd, LPARAM lParam)
{
  if (hwnd == (HWND)lParam)
    return TRUE;  
  InvalidateRect (hwnd, NULL, FALSE);
  UpdateWindow (hwnd);
  return TRUE;
}

static wBOOL CALLBACK
winRedrawDamagedWindowShadowGDI (HWND hwnd, LPARAM lParam)
{
  BoxPtr pDamage = (BoxPtr)lParam;
  RECT rcClient, rcDamage, rcRedraw;
  POINT topLeft, bottomRight;
  
  if (IsIconic (hwnd))
    return TRUE; /* Don't care minimized windows */
  
  /* Convert the damaged area from Screen coords to Client coords */
  topLeft.x = pDamage->x1; topLeft.y = pDamage->y1;
  bottomRight.x = pDamage->x2; bottomRight.y = pDamage->y2;
  topLeft.x += GetSystemMetrics (SM_XVIRTUALSCREEN);
  bottomRight.x += GetSystemMetrics (SM_XVIRTUALSCREEN);
  topLeft.y += GetSystemMetrics (SM_YVIRTUALSCREEN);
  bottomRight.y += GetSystemMetrics (SM_YVIRTUALSCREEN);
  ScreenToClient (hwnd, &topLeft);
  ScreenToClient (hwnd, &bottomRight);
  SetRect (&rcDamage, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);

  GetClientRect (hwnd, &rcClient);

  if (IntersectRect (&rcRedraw, &rcClient, &rcDamage))
    {
      InvalidateRect (hwnd, &rcRedraw, FALSE);
      UpdateWindow (hwnd);
    }
  return TRUE;
}
#endif


/*
 * Allocate a DIB for the shadow framebuffer GDI server
 */

static Bool
winAllocateFBShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  BITMAPINFOHEADER	*pbmih = NULL;
  DIBSECTION		dibsection;
  Bool			fReturn = TRUE;

  /* Get device contexts for the screen and shadow bitmap */
  pScreenPriv->hdcScreen = GetDC (pScreenPriv->hwndScreen);
  pScreenPriv->hdcShadow = CreateCompatibleDC (pScreenPriv->hdcScreen);

  /* Allocate bitmap info header */
  pbmih = (BITMAPINFOHEADER*) malloc (sizeof (BITMAPINFOHEADER)
				      + 256 * sizeof (RGBQUAD));
  if (pbmih == NULL)
    {
      ErrorF ("winAllocateFBShadowGDI - malloc () failed\n");
      return FALSE;
    }

  /* Query the screen format */
  fReturn = winQueryScreenDIBFormat (pScreen, pbmih);

  /* Describe shadow bitmap to be created */
  pbmih->biWidth = pScreenInfo->dwWidth;
  pbmih->biHeight = -pScreenInfo->dwHeight;
  
  ErrorF ("winAllocateFBShadowGDI - Creating DIB with width: %d height: %d "
	  "depth: %d\n",
	  (int) pbmih->biWidth, (int) -pbmih->biHeight, pbmih->biBitCount);

  /* Create a DI shadow bitmap with a bit pointer */
  pScreenPriv->hbmpShadow = CreateDIBSection (pScreenPriv->hdcScreen,
					      (BITMAPINFO *) pbmih,
					      DIB_RGB_COLORS,
					      (VOID**) &pScreenInfo->pfb,
					      NULL,
					      0);
  if (pScreenPriv->hbmpShadow == NULL || pScreenInfo->pfb == NULL)
    {
      winW32Error (2, "winAllocateFBShadowGDI - CreateDIBSection failed:");
      return FALSE;
    }
  else
    {
#if CYGDEBUG
      winDebug ("winAllocateFBShadowGDI - Shadow buffer allocated\n");
#endif
    }

  /* Get information about the bitmap that was allocated */
  GetObject (pScreenPriv->hbmpShadow,
	     sizeof (dibsection),
	     &dibsection);

#if CYGDEBUG || YES
  /* Print information about bitmap allocated */
  winDebug ("winAllocateFBShadowGDI - Dibsection width: %d height: %d "
	  "depth: %d size image: %d\n",
	  (int) dibsection.dsBmih.biWidth, (int) dibsection.dsBmih.biHeight,
	  dibsection.dsBmih.biBitCount,
	  (int) dibsection.dsBmih.biSizeImage);
#endif

  /* Select the shadow bitmap into the shadow DC */
  SelectObject (pScreenPriv->hdcShadow,
		pScreenPriv->hbmpShadow);

#if CYGDEBUG
  winDebug ("winAllocateFBShadowGDI - Attempting a shadow blit\n");
#endif

  /* Do a test blit from the shadow to the screen, I think */
  fReturn = BitBlt (pScreenPriv->hdcScreen,
		    0, 0,
		    pScreenInfo->dwWidth, pScreenInfo->dwHeight,
		    pScreenPriv->hdcShadow,
		    0, 0,
		    SRCCOPY);
  if (fReturn)
    {
#if CYGDEBUG
      winDebug ("winAllocateFBShadowGDI - Shadow blit success\n");
#endif
    }
  else
    {
      winW32Error (2, "winAllocateFBShadowGDI - Shadow blit failure\n");
#if 0      
      return FALSE;
#else 
      /* ago: ignore this error. The blit fails with wine, but does not 
       * cause any problems later. */

      fReturn = TRUE;
#endif      
    }

  /* Look for height weirdness */
  if (dibsection.dsBmih.biHeight < 0)
    {
      dibsection.dsBmih.biHeight = -dibsection.dsBmih.biHeight;
    }

  /* Set screeninfo stride */
  pScreenInfo->dwStride = ((dibsection.dsBmih.biSizeImage
			    / dibsection.dsBmih.biHeight)
			   * 8) / pScreenInfo->dwBPP;

#if CYGDEBUG || YES
  winDebug ("winAllocateFBShadowGDI - Created shadow stride: %d\n",
	  (int) pScreenInfo->dwStride);
#endif

  /* See if the shadow bitmap will be larger than the DIB size limit */
  if (pScreenInfo->dwWidth * pScreenInfo->dwHeight * pScreenInfo->dwBPP
      >= WIN_DIB_MAXIMUM_SIZE)
    {
      ErrorF ("winAllocateFBShadowGDI - Requested DIB (bitmap) "
	      "will be larger than %d MB.  The surface may fail to be "
	      "allocated on Windows 95, 98, or Me, due to a %d MB limit in "
	      "DIB size.  This limit does not apply to Windows NT/2000, and "
	      "this message may be ignored on those platforms.\n",
	      WIN_DIB_MAXIMUM_SIZE_MB, WIN_DIB_MAXIMUM_SIZE_MB);
    }

  /* Determine our color masks */
  if (!winQueryRGBBitsAndMasks (pScreen))
    {
      ErrorF ("winAllocateFBShadowGDI - winQueryRGBBitsAndMasks failed\n");
      return FALSE;
    }

#ifdef XWIN_MULTIWINDOW
  /* Redraw all windows */
  if (pScreenInfo->fMultiWindow)
    EnumThreadWindows (g_dwCurrentThreadID, winRedrawAllProcShadowGDI, 0);
#endif

  return fReturn;
}


/*
 * Blit the damaged regions of the shadow fb to the screen
 */

static void
winShadowUpdateGDI (ScreenPtr pScreen, 
		    shadowBufPtr pBuf)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  RegionPtr		damage = &pBuf->damage;
  DWORD			dwBox = REGION_NUM_RECTS (damage);
  BoxPtr		pBox = REGION_RECTS (damage);
  int			x, y, w, h;
  HRGN			hrgnTemp = NULL, hrgnCombined = NULL;
#ifdef XWIN_UPDATESTATS
  static DWORD		s_dwNonUnitRegions = 0;
  static DWORD		s_dwTotalUpdates = 0;
  static DWORD		s_dwTotalBoxes = 0;
#endif
  BoxPtr		pBoxExtents = REGION_EXTENTS (pScreen, damage);

  /*
   * Return immediately if the app is not active
   * and we are fullscreen, or if we have a bad display depth
   */
  if ((!pScreenPriv->fActive && pScreenInfo->fFullScreen)
      || pScreenPriv->fBadDepth) return;

#ifdef XWIN_UPDATESTATS
  ++s_dwTotalUpdates;
  s_dwTotalBoxes += dwBox;

  if (dwBox != 1)
    {
      ++s_dwNonUnitRegions;
      ErrorF ("winShadowUpdatGDI - dwBox: %d\n", dwBox);
    }
  
  if ((s_dwTotalUpdates % 100) == 0)
    ErrorF ("winShadowUpdateGDI - %d%% non-unity regions, avg boxes: %d "
	    "nu: %d tu: %d\n",
	    (s_dwNonUnitRegions * 100) / s_dwTotalUpdates,
	    s_dwTotalBoxes / s_dwTotalUpdates,
	    s_dwNonUnitRegions, s_dwTotalUpdates);
#endif /* XWIN_UPDATESTATS */

  /*
   * Handle small regions with multiple blits,
   * handle large regions by creating a clipping region and 
   * doing a single blit constrained to that clipping region.
   */
  if (!pScreenInfo->fMultiWindow &&
      (pScreenInfo->dwClipUpdatesNBoxes == 0 ||
      dwBox < pScreenInfo->dwClipUpdatesNBoxes))
    {
      /* Loop through all boxes in the damaged region */
      while (dwBox--)
	{
	  /*
	   * Calculate x offset, y offset, width, and height for
	   * current damage box
	   */
	  x = pBox->x1;
	  y = pBox->y1;
	  w = pBox->x2 - pBox->x1;
	  h = pBox->y2 - pBox->y1;
	  
	  BitBlt (pScreenPriv->hdcScreen,
		  x, y,
		  w, h,
		  pScreenPriv->hdcShadow,
		  x, y,
		  SRCCOPY);
	  
	  /* Get a pointer to the next box */
	  ++pBox;
	}
    }
  else if (!pScreenInfo->fMultiWindow)
    {
      /* Compute a GDI region from the damaged region */
      hrgnCombined = CreateRectRgn (pBox->x1, pBox->y1, pBox->x2, pBox->y2);
      dwBox--;
      pBox++;
      while (dwBox--)
	{
	  hrgnTemp = CreateRectRgn (pBox->x1, pBox->y1, pBox->x2, pBox->y2);
	  CombineRgn (hrgnCombined, hrgnCombined, hrgnTemp, RGN_OR);
	  DeleteObject (hrgnTemp);
	  pBox++;
	}
      
      /* Install the GDI region as a clipping region */
      SelectClipRgn (pScreenPriv->hdcScreen, hrgnCombined);
      DeleteObject (hrgnCombined);
      hrgnCombined = NULL;
      
      /*
       * Blit the shadow buffer to the screen,
       * constrained to the clipping region.
       */
      BitBlt (pScreenPriv->hdcScreen,
	      pBoxExtents->x1, pBoxExtents->y1,
	      pBoxExtents->x2 - pBoxExtents->x1,
	      pBoxExtents->y2 - pBoxExtents->y1,
	      pScreenPriv->hdcShadow,
	      pBoxExtents->x1, pBoxExtents->y1,
	      SRCCOPY);

      /* Reset the clip region */
      SelectClipRgn (pScreenPriv->hdcScreen, NULL);
    }

#ifdef XWIN_MULTIWINDOW
  /* Redraw all multiwindow windows */
  if (pScreenInfo->fMultiWindow)
    EnumThreadWindows (g_dwCurrentThreadID,
		       winRedrawDamagedWindowShadowGDI,
		       (LPARAM)pBoxExtents);
#endif
}


/* See Porting Layer Definition - p. 33 */
/*
 * We wrap whatever CloseScreen procedure was specified by fb;
 * a pointer to said procedure is stored in our privates.
 */

static Bool
winCloseScreenShadowGDI (int nIndex, ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  Bool			fReturn;

#if CYGDEBUG
  winDebug ("winCloseScreenShadowGDI - Freeing screen resources\n");
#endif

  /* Flag that the screen is closed */
  pScreenPriv->fClosed = TRUE;
  pScreenPriv->fActive = FALSE;

  /* Call the wrapped CloseScreen procedure */
  WIN_UNWRAP(CloseScreen);
  fReturn = (*pScreen->CloseScreen) (nIndex, pScreen);

  /* Delete the window property */
  RemoveProp (pScreenPriv->hwndScreen, WIN_SCR_PROP);

  /* Free the shadow DC; which allows the bitmap to be freed */
  DeleteDC (pScreenPriv->hdcShadow);
  
  /* Free the shadow bitmap */
  DeleteObject (pScreenPriv->hbmpShadow);

  /* Free the screen DC */
  ReleaseDC (pScreenPriv->hwndScreen, pScreenPriv->hdcScreen);

  /* Delete tray icon, if we have one */
  if (!pScreenInfo->fNoTrayIcon)
    winDeleteNotifyIcon (pScreenPriv);

  /* Free the exit confirmation dialog box, if it exists */
  if (g_hDlgExit != NULL)
    {
      DestroyWindow (g_hDlgExit);
      g_hDlgExit = NULL;
    }

  /* Kill our window */
  if (pScreenPriv->hwndScreen)
    {
      DestroyWindow (pScreenPriv->hwndScreen);
      pScreenPriv->hwndScreen = NULL;
    }

#if defined(XWIN_CLIPBOARD) || defined(XWIN_MULTIWINDOW)
  /* Destroy the thread startup mutex */
  pthread_mutex_destroy (&pScreenPriv->pmServerStarted);
#endif

  /* Invalidate our screeninfo's pointer to the screen */
  pScreenInfo->pScreen = NULL;

  /* Invalidate the ScreenInfo's fb pointer */
  pScreenInfo->pfb = NULL;

  /* Free the screen privates for this screen */
  free ((pointer) pScreenPriv);

  return fReturn;
}


/*
 * Tell mi what sort of visuals we need.
 * 
 * Generally we only need one visual, as our screen can only
 * handle one format at a time, I believe.  You may want
 * to verify that last sentence.
 */

static Bool
winInitVisualsShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;

  /* Display debugging information */
  ErrorF ("winInitVisualsShadowGDI - Masks %08x %08x %08x BPRGB %d d %d "
	  "bpp %d\n",
	  (unsigned int) pScreenPriv->dwRedMask,
	  (unsigned int) pScreenPriv->dwGreenMask,
	  (unsigned int) pScreenPriv->dwBlueMask,
	  (int) pScreenPriv->dwBitsPerRGB,
	  (int) pScreenInfo->dwDepth,
	  (int) pScreenInfo->dwBPP);

  /* Create a single visual according to the Windows screen depth */
  switch (pScreenInfo->dwDepth)
    {
    case 24:
    case 16:
    case 15:
#if defined(XFree86Server)
      /* Setup the real visual */
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     TrueColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     -1,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsShadowGDI - miSetVisualTypesAndMasks "
		  "failed\n");
	  return FALSE;
	}

#ifdef XWIN_EMULATEPSEUDO
      if (!pScreenInfo->fEmulatePseudo)
	break;

      /* Setup a pseudocolor visual */
      if (!miSetVisualTypesAndMasks (8,
				     PseudoColorMask,
				     8,
				     -1,
				     0,
				     0,
				     0))
	{
	  ErrorF ("winInitVisualsShadowGDI - miSetVisualTypesAndMasks "
		  "failed for PseudoColor\n");
	  return FALSE;
	}
#endif
#else /* XFree86Server */
      /* Setup the real visual */
      if (!fbSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     TrueColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsShadowGDI - fbSetVisualTypesAndMasks "
		  "failed for TrueColor\n");
	  return FALSE;
	}

#ifdef XWIN_EMULATEPSEUDO
      if (!pScreenInfo->fEmulatePseudo)
	break;

      /* Setup a pseudocolor visual */
      if (!fbSetVisualTypesAndMasks (8,
				     PseudoColorMask,
				     8,
				     0,
				     0,
				     0))
	{
	  ErrorF ("winInitVisualsShadowGDI - fbSetVisualTypesAndMasks "
		  "failed for PseudoColor\n");
	  return FALSE;
	}
#endif
#endif /* XFree86Server */
      break;

    case 8:
#if defined(XFree86Server)
      if (!miSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     PseudoColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     PseudoColor,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsShadowGDI - miSetVisualTypesAndMasks "
		  "failed\n");
	  return FALSE;
	}
#else /* XFree86Server */
      if (!fbSetVisualTypesAndMasks (pScreenInfo->dwDepth,
				     PseudoColorMask,
				     pScreenPriv->dwBitsPerRGB,
				     pScreenPriv->dwRedMask,
				     pScreenPriv->dwGreenMask,
				     pScreenPriv->dwBlueMask))
	{
	  ErrorF ("winInitVisualsShadowGDI - fbSetVisualTypesAndMasks "
		  "failed\n");
	  return FALSE;
	}
#endif
      break;

    default:
      ErrorF ("winInitVisualsShadowGDI - Unknown screen depth\n");
      return FALSE;
    }

#if CYGDEBUG
  winDebug ("winInitVisualsShadowGDI - Returning\n");
#endif

  return TRUE;
}


/*
 * Adjust the proposed video mode
 */

static Bool
winAdjustVideoModeShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  HDC			hdc;
  DWORD			dwBPP;
  
  hdc = GetDC (NULL);

  /* We're in serious trouble if we can't get a DC */
  if (hdc == NULL)
    {
      ErrorF ("winAdjustVideoModeShadowGDI - GetDC () failed\n");
      return FALSE;
    }

  /* Query GDI for current display depth */
  dwBPP = GetDeviceCaps (hdc, BITSPIXEL);

  /* GDI cannot change the screen depth */
  if (pScreenInfo->dwBPP == WIN_DEFAULT_BPP)
    {
      /* No -depth parameter passed, let the user know the depth being used */
      ErrorF ("winAdjustVideoModeShadowGDI - Using Windows display "
	      "depth of %d bits per pixel\n", (int) dwBPP);

      /* Use GDI's depth */
      pScreenInfo->dwBPP = dwBPP;
    }
  else if (dwBPP != pScreenInfo->dwBPP)
    {
      /* Warn user if GDI depth is different than -depth parameter */
      ErrorF ("winAdjustVideoModeShadowGDI - Command line bpp: %d, "\
	      "using bpp: %d\n", (int) pScreenInfo->dwBPP, (int) dwBPP);

      /* We'll use GDI's depth */
      pScreenInfo->dwBPP = dwBPP;
    }
  
  /* Release our DC */
  ReleaseDC (NULL, hdc);
  hdc = NULL;

  return TRUE;
}


/*
 * Blt exposed regions to the screen
 */

static Bool
winBltExposedRegionsShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  winPrivCmapPtr	pCmapPriv = NULL;
  HDC			hdcUpdate;
  PAINTSTRUCT		ps;

  /* BeginPaint gives us an hdc that clips to the invalidated region */
  hdcUpdate = BeginPaint (pScreenPriv->hwndScreen, &ps);

  /* Realize the palette, if we have one */
  if (pScreenPriv->pcmapInstalled != NULL)
    {
      pCmapPriv = winGetCmapPriv (pScreenPriv->pcmapInstalled);
      
      SelectPalette (hdcUpdate, pCmapPriv->hPalette, FALSE);
      RealizePalette (hdcUpdate);
    }

  /* Our BitBlt will be clipped to the invalidated region */
  BitBlt (hdcUpdate,
	  0, 0,
	  pScreenInfo->dwWidth, pScreenInfo->dwHeight,
	  pScreenPriv->hdcShadow,
	  0, 0,
	  SRCCOPY);

  /* EndPaint frees the DC */
  EndPaint (pScreenPriv->hwndScreen, &ps);

#ifdef XWIN_MULTIWINDOW
  /* Redraw all windows */
  if (pScreenInfo->fMultiWindow)
    EnumThreadWindows(g_dwCurrentThreadID, winRedrawAllProcShadowGDI, 
            (LPARAM)pScreenPriv->hwndScreen);
#endif

  return TRUE;
}


/*
 * Do any engine-specific appliation-activation processing
 */

static Bool
winActivateAppShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;

  /*
   * 2004/04/12 - Harold - We perform the restoring or minimizing
   * manually for ShadowGDI in fullscreen modes so that this engine
   * will perform just like ShadowDD and ShadowDDNL in fullscreen mode;
   * if we do not do this then our fullscreen window will appear in the
   * z-order when it is deactivated and it can be uncovered by resizing
   * or minimizing another window that is on top of it, which is not how
   * the DirectDraw engines work.  Therefore we keep this code here to
   * make sure that all engines work the same in fullscreen mode.
   */

  /*
   * Are we active?
   * Are we fullscreen?
   */
  if (pScreenPriv->fActive
      && pScreenInfo->fFullScreen)
    {
      /*
       * Activating, attempt to bring our window 
       * to the top of the display
       */
      ShowWindow (pScreenPriv->hwndScreen, SW_RESTORE);
    }
  else if (!pScreenPriv->fActive
	   && pScreenInfo->fFullScreen)
    {
      /*
       * Deactivating, stuff our window onto the
       * task bar.
       */
      ShowWindow (pScreenPriv->hwndScreen, SW_MINIMIZE);
    }

  return TRUE;
}


/*
 * Reblit the shadow framebuffer to the screen.
 */

static Bool
winRedrawScreenShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;

  /* Redraw the whole window, to take account for the new colors */
  BitBlt (pScreenPriv->hdcScreen,
	  0, 0,
	  pScreenInfo->dwWidth, pScreenInfo->dwHeight,
	  pScreenPriv->hdcShadow,
	  0, 0,
	  SRCCOPY);

#ifdef XWIN_MULTIWINDOW
  /* Redraw all windows */
  if (pScreenInfo->fMultiWindow)
    EnumThreadWindows(g_dwCurrentThreadID, winRedrawAllProcShadowGDI, 0);
#endif

  return TRUE;
}



/*
 * Realize the currently installed colormap
 */

static Bool
winRealizeInstalledPaletteShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winPrivCmapPtr	pCmapPriv = NULL;

#if CYGDEBUG
  winDebug ("winRealizeInstalledPaletteShadowGDI\n");
#endif

  /* Don't do anything if there is not a colormap */
  if (pScreenPriv->pcmapInstalled == NULL)
    {
#if CYGDEBUG
      winDebug ("winRealizeInstalledPaletteShadowGDI - No colormap "
	      "installed\n");
#endif
      return TRUE;
    }

  pCmapPriv = winGetCmapPriv (pScreenPriv->pcmapInstalled);
  
  /* Realize our palette for the screen */
  if (RealizePalette (pScreenPriv->hdcScreen) == GDI_ERROR)
    {
      ErrorF ("winRealizeInstalledPaletteShadowGDI - RealizePalette () "
	      "failed\n");
      return FALSE;
    }
  
  /* Set the DIB color table */
  if (SetDIBColorTable (pScreenPriv->hdcShadow,
			0,
			WIN_NUM_PALETTE_ENTRIES,
			pCmapPriv->rgbColors) == 0)
    {
      ErrorF ("winRealizeInstalledPaletteShadowGDI - SetDIBColorTable () "
	      "failed\n");
      return FALSE;
    }
  
  return TRUE;
}


/*
 * Install the specified colormap
 */

static Bool
winInstallColormapShadowGDI (ColormapPtr pColormap)
{
  ScreenPtr		pScreen = pColormap->pScreen;
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  winCmapPriv(pColormap);

  /*
   * Tell Windows to install the new colormap
   */
  if (SelectPalette (pScreenPriv->hdcScreen,
		     pCmapPriv->hPalette,
		     FALSE) == NULL)
    {
      ErrorF ("winInstallColormapShadowGDI - SelectPalette () failed\n");
      return FALSE;
    }
      
  /* Realize the palette */
  if (GDI_ERROR == RealizePalette (pScreenPriv->hdcScreen))
    {
      ErrorF ("winInstallColormapShadowGDI - RealizePalette () failed\n");
      return FALSE;
    }

  /* Set the DIB color table */
  if (SetDIBColorTable (pScreenPriv->hdcShadow,
			0,
			WIN_NUM_PALETTE_ENTRIES,
			pCmapPriv->rgbColors) == 0)
    {
      ErrorF ("winInstallColormapShadowGDI - SetDIBColorTable () failed\n");
      return FALSE;
    }

  /* Redraw the whole window, to take account for the new colors */
  BitBlt (pScreenPriv->hdcScreen,
	  0, 0,
	  pScreenInfo->dwWidth, pScreenInfo->dwHeight,
	  pScreenPriv->hdcShadow,
	  0, 0,
	  SRCCOPY);

  /* Save a pointer to the newly installed colormap */
  pScreenPriv->pcmapInstalled = pColormap;

#ifdef XWIN_MULTIWINDOW
  /* Redraw all windows */
  if (pScreenInfo->fMultiWindow)
    EnumThreadWindows (g_dwCurrentThreadID, winRedrawAllProcShadowGDI, 0);
#endif

  return TRUE;
}


/*
 * Store the specified colors in the specified colormap
 */

static Bool
winStoreColorsShadowGDI (ColormapPtr pColormap,
			 int ndef,
			 xColorItem *pdefs)
{
  ScreenPtr		pScreen = pColormap->pScreen;
  winScreenPriv(pScreen);
  winCmapPriv(pColormap);
  ColormapPtr curpmap = pScreenPriv->pcmapInstalled;
  
  /* Put the X colormap entries into the Windows logical palette */
  if (SetPaletteEntries (pCmapPriv->hPalette,
			 pdefs[0].pixel,
			 ndef,
			 pCmapPriv->peColors + pdefs[0].pixel) == 0)
    {
      ErrorF ("winStoreColorsShadowGDI - SetPaletteEntries () failed\n");
      return FALSE;
    }

  /* Don't install the Windows palette if the colormap is not installed */
  if (pColormap != curpmap)
    {
      return TRUE;
    }

  /* Try to install the newly modified colormap */
  if (!winInstallColormapShadowGDI (pColormap))
    {
      ErrorF ("winInstallColormapShadowGDI - winInstallColormapShadowGDI "
	      "failed\n");
      return FALSE;
    }

#if 0
  /* Tell Windows that the palette has changed */
  RealizePalette (pScreenPriv->hdcScreen);
  
  /* Set the DIB color table */
  if (SetDIBColorTable (pScreenPriv->hdcShadow,
			pdefs[0].pixel,
			ndef,
			pCmapPriv->rgbColors + pdefs[0].pixel) == 0)
    {
      ErrorF ("winInstallColormapShadowGDI - SetDIBColorTable () failed\n");
      return FALSE;
    }

  /* Save a pointer to the newly installed colormap */
  pScreenPriv->pcmapInstalled = pColormap;
#endif

  return TRUE;
}


/*
 * Colormap initialization procedure
 */

static Bool
winCreateColormapShadowGDI (ColormapPtr pColormap)
{
  LPLOGPALETTE		lpPaletteNew = NULL;
  DWORD			dwEntriesMax;
  VisualPtr		pVisual;
  HPALETTE		hpalNew = NULL;
  winCmapPriv(pColormap);

  /* Get a pointer to the visual that the colormap belongs to */
  pVisual = pColormap->pVisual;

  /* Get the maximum number of palette entries for this visual */
  dwEntriesMax = pVisual->ColormapEntries;

  /* Allocate a Windows logical color palette with max entries */
  lpPaletteNew = malloc (sizeof (LOGPALETTE)
			 + (dwEntriesMax - 1) * sizeof (PALETTEENTRY));
  if (lpPaletteNew == NULL)
    {
      ErrorF ("winCreateColormapShadowGDI - Couldn't allocate palette "
	      "with %d entries\n",
	      (int) dwEntriesMax);
      return FALSE;
    }

  /* Zero out the colormap */
  ZeroMemory (lpPaletteNew, sizeof (LOGPALETTE)
	      + (dwEntriesMax - 1) * sizeof (PALETTEENTRY));
  
  /* Set the logical palette structure */
  lpPaletteNew->palVersion = 0x0300;
  lpPaletteNew->palNumEntries = dwEntriesMax;

  /* Tell Windows to create the palette */
  hpalNew = CreatePalette (lpPaletteNew);
  if (hpalNew == NULL)
    {
      ErrorF ("winCreateColormapShadowGDI - CreatePalette () failed\n");
      free (lpPaletteNew);
      return FALSE;
    }

  /* Save the Windows logical palette handle in the X colormaps' privates */
  pCmapPriv->hPalette = hpalNew;

  /* Free the palette initialization memory */
  free (lpPaletteNew);

  return TRUE;
}


/*
 * Colormap destruction procedure
 */

static Bool
winDestroyColormapShadowGDI (ColormapPtr pColormap)
{
  winScreenPriv(pColormap->pScreen);
  winCmapPriv(pColormap);

  /*
   * Is colormap to be destroyed the default?
   *
   * Non-default colormaps should have had winUninstallColormap
   * called on them before we get here.  The default colormap
   * will not have had winUninstallColormap called on it.  Thus,
   * we need to handle the default colormap in a special way.
   */
  if (pColormap->flags & IsDefault)
    {
#if CYGDEBUG
      winDebug ("winDestroyColormapShadowGDI - Destroying default "
	      "colormap\n");
#endif
      
      /*
       * FIXME: Walk the list of all screens, popping the default
       * palette out of each screen device context.
       */
      
      /* Pop the palette out of the device context */
      SelectPalette (pScreenPriv->hdcScreen,
		     GetStockObject (DEFAULT_PALETTE),
		     FALSE);

      /* Clear our private installed colormap pointer */
      pScreenPriv->pcmapInstalled = NULL;
    }
  
  /* Try to delete the logical palette */
  if (DeleteObject (pCmapPriv->hPalette) == 0)
    {
      ErrorF ("winDestroyColormap - DeleteObject () failed\n");
      return FALSE;
    }
  
  /* Invalidate the colormap privates */
  pCmapPriv->hPalette = NULL;

  return TRUE;
}


/*
 * Set engine specific funtions
 */

Bool
winSetEngineFunctionsShadowGDI (ScreenPtr pScreen)
{
  winScreenPriv(pScreen);
  winScreenInfo		*pScreenInfo = pScreenPriv->pScreenInfo;
  
  /* Set our pointers */
  pScreenPriv->pwinAllocateFB = winAllocateFBShadowGDI;
  pScreenPriv->pwinShadowUpdate = winShadowUpdateGDI;
  pScreenPriv->pwinCloseScreen = winCloseScreenShadowGDI;
  pScreenPriv->pwinInitVisuals = winInitVisualsShadowGDI;
  pScreenPriv->pwinAdjustVideoMode = winAdjustVideoModeShadowGDI;
  if (pScreenInfo->fFullScreen)
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowFullScreen;
  else
    pScreenPriv->pwinCreateBoundingWindow = winCreateBoundingWindowWindowed;
  pScreenPriv->pwinFinishScreenInit = winFinishScreenInitFB;
  pScreenPriv->pwinBltExposedRegions = winBltExposedRegionsShadowGDI;
  pScreenPriv->pwinActivateApp = winActivateAppShadowGDI;
  pScreenPriv->pwinRedrawScreen = winRedrawScreenShadowGDI;
  pScreenPriv->pwinRealizeInstalledPalette = 
    winRealizeInstalledPaletteShadowGDI;
  pScreenPriv->pwinInstallColormap = winInstallColormapShadowGDI;
  pScreenPriv->pwinStoreColors = winStoreColorsShadowGDI;
  pScreenPriv->pwinCreateColormap = winCreateColormapShadowGDI;
  pScreenPriv->pwinDestroyColormap = winDestroyColormapShadowGDI;
  pScreenPriv->pwinHotKeyAltTab = (winHotKeyAltTabProcPtr) (void (*)(void))NoopDDA;
  pScreenPriv->pwinCreatePrimarySurface
    = (winCreatePrimarySurfaceProcPtr) (void (*)(void))NoopDDA;
  pScreenPriv->pwinReleasePrimarySurface
    = (winReleasePrimarySurfaceProcPtr) (void (*)(void))NoopDDA;
#ifdef XWIN_MULTIWINDOW
  pScreenPriv->pwinFinishCreateWindowsWindow =
    (winFinishCreateWindowsWindowProcPtr) (void (*)(void))NoopDDA;
#endif

  return TRUE;
}
