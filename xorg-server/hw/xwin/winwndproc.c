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
 *		MATSUZAKI Kensuke
 */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"
#include <commctrl.h>
#include "winprefs.h"
#include "winconfig.h"
#include "winmsg.h"
#include "inputstr.h"

#ifdef XKB
extern BOOL winCheckKeyPressed(WPARAM wParam, LPARAM lParam);
#endif
extern void winFixShiftKeys (int iScanCode);


/*
 * Global variables
 */

Bool				g_fCursor = TRUE;
Bool				g_fButton[3] = { FALSE, FALSE, FALSE };


/*
 * References to external symbols
 */

extern Bool			g_fClipboard;
extern HWND			g_hDlgDepthChange;
extern Bool			g_fKeyboardHookLL;
extern HWND			g_hwndKeyboardFocus;
extern Bool			g_fSoftwareCursor;
extern DWORD			g_dwCurrentThreadID;


/*
 * Called by winWakeupHandler
 * Processes current Windows message
 */

LRESULT CALLBACK
winWindowProc (HWND hwnd, UINT message, 
	       WPARAM wParam, LPARAM lParam)
{
  static winPrivScreenPtr	s_pScreenPriv = NULL;
  static winScreenInfo		*s_pScreenInfo = NULL;
  static ScreenPtr		s_pScreen = NULL;
  static HWND			s_hwndLastPrivates = NULL;
  static HINSTANCE		s_hInstance;
  static Bool			s_fTracking = FALSE;
  static unsigned long		s_ulServerGeneration = 0;
  static UINT			s_uTaskbarRestart = 0;
  int				iScanCode;
  int				i;

#if CYGDEBUG
  winDebugWin32Message("winWindowProc", hwnd, message, wParam, lParam);
#endif
  
  /* Watch for server regeneration */
  if (g_ulServerGeneration != s_ulServerGeneration)
    {
      /* Store new server generation */
      s_ulServerGeneration = g_ulServerGeneration;
    }

  /* Only retrieve new privates pointers if window handle is null or changed */
  if ((s_pScreenPriv == NULL || hwnd != s_hwndLastPrivates)
      && (s_pScreenPriv = GetProp (hwnd, WIN_SCR_PROP)) != NULL)
    {
#if CYGDEBUG
      winDebug ("winWindowProc - Setting privates handle\n");
#endif
      s_pScreenInfo = s_pScreenPriv->pScreenInfo;
      s_pScreen = s_pScreenInfo->pScreen;
      s_hwndLastPrivates = hwnd;
    }
  else if (s_pScreenPriv == NULL)
    {
      /* For safety, handle case that should never happen */
      s_pScreenInfo = NULL;
      s_pScreen = NULL;
      s_hwndLastPrivates = NULL;
    }

  /* Branch on message type */
  switch (message)
    {
    case WM_TRAYICON:
      return winHandleIconMessage (hwnd, message, wParam, lParam,
				   s_pScreenPriv);

    case WM_CREATE:
#if CYGDEBUG
      winDebug ("winWindowProc - WM_CREATE\n");
#endif
      
      /*
       * Add a property to our display window that references
       * this screens' privates.
       *
       * This allows the window procedure to refer to the
       * appropriate window DC and shadow DC for the window that
       * it is processing.  We use this to repaint exposed
       * areas of our display window.
       */
      s_pScreenPriv = ((LPCREATESTRUCT) lParam)->lpCreateParams;
      s_hInstance = ((LPCREATESTRUCT) lParam)->hInstance;
      s_pScreenInfo = s_pScreenPriv->pScreenInfo;
      s_pScreen = s_pScreenInfo->pScreen;
      s_hwndLastPrivates = hwnd;
      s_uTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
      SetProp (hwnd, WIN_SCR_PROP, s_pScreenPriv);

      /* Setup tray icon */
      if (!s_pScreenInfo->fNoTrayIcon)
	{
	  /*
	   * NOTE: The WM_CREATE message is processed before CreateWindowEx
	   * returns, so s_pScreenPriv->hwndScreen is invalid at this point.
	   * We go ahead and copy our hwnd parameter over top of the screen
	   * privates hwndScreen so that we have a valid value for
	   * that member.  Otherwise, the tray icon will disappear
	   * the first time you move the mouse over top of it.
	   */
	  
	  s_pScreenPriv->hwndScreen = hwnd;

	  winInitNotifyIcon (s_pScreenPriv);
	}
      return 0;

    case WM_DISPLAYCHANGE:
      /* We cannot handle a display mode change during initialization */
      if (s_pScreenInfo == NULL)
	FatalError ("winWindowProc - WM_DISPLAYCHANGE - The display "
		    "mode changed while we were intializing.  This is "
		    "very bad and unexpected.  Exiting.\n");

      /*
       * We do not care about display changes with
       * fullscreen DirectDraw engines, because those engines set
       * their own mode when they become active.
       */
      if (s_pScreenInfo->fFullScreen
	  && (s_pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DD
	      || s_pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DDNL
#ifdef XWIN_PRIMARYFB
	      || s_pScreenInfo->dwEngine == WIN_SERVER_PRIMARY_DD
#endif
	      ))
	{
	  /* 
	   * Store the new display dimensions and depth.
	   * We do this here for future compatibility in case we
	   * ever allow switching from fullscreen to windowed mode.
	   */
	  s_pScreenPriv->dwLastWindowsWidth = GetSystemMetrics (SM_CXSCREEN);
	  s_pScreenPriv->dwLastWindowsHeight = GetSystemMetrics (SM_CYSCREEN);
	  s_pScreenPriv->dwLastWindowsBitsPixel
	    = GetDeviceCaps (s_pScreenPriv->hdcScreen, BITSPIXEL);	  
	  break;
	}
      
      ErrorF ("winWindowProc - WM_DISPLAYCHANGE - orig bpp: %d, last bpp: %d, "
	      "new bpp: %d\n",
	      (int) s_pScreenInfo->dwBPP,
	      (int) s_pScreenPriv->dwLastWindowsBitsPixel,
	      wParam);

      ErrorF ("winWindowProc - WM_DISPLAYCHANGE - new width: %d "
	      "new height: %d\n",
	      LOWORD (lParam), HIWORD (lParam));

      /*
       * TrueColor --> TrueColor depth changes are disruptive for:
       *	Windowed:
       *		Shadow DirectDraw
       *		Shadow DirectDraw Non-Locking
       *		Primary DirectDraw
       *
       * TrueColor --> TrueColor depth changes are non-optimal for:
       *	Windowed:
       *		Shadow GDI
       *
       *	FullScreen:
       *		Shadow GDI
       *
       * TrueColor --> PseudoColor or vice versa are disruptive for:
       *	Windowed:
       *		Shadow DirectDraw
       *		Shadow DirectDraw Non-Locking
       *		Primary DirectDraw
       *		Shadow GDI
       */

      /*
       * Check for a disruptive change in depth.
       * We can only display a message for a disruptive depth change,
       * we cannot do anything to correct the situation.
       */
      if ((s_pScreenInfo->dwBPP != wParam)
	  && (s_pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DD
	      || s_pScreenInfo->dwEngine == WIN_SERVER_SHADOW_DDNL
#ifdef XWIN_PRIMARYFB
	      || s_pScreenInfo->dwEngine == WIN_SERVER_PRIMARY_DD
#endif
	      ))
	{
	  /* Cannot display the visual until the depth is restored */
	  ErrorF ("winWindowProc - Disruptive change in depth\n");

	  /* Display Exit dialog */
	  winDisplayDepthChangeDialog (s_pScreenPriv);

	  /* Flag that we have an invalid screen depth */
	  s_pScreenPriv->fBadDepth = TRUE;

	  /* Minimize the display window */
	  ShowWindow (hwnd, SW_MINIMIZE);
	}
      else
	{
	  /* Flag that we have a valid screen depth */
	  s_pScreenPriv->fBadDepth = FALSE;
	}
      
      /*
       * Check for a change in display dimensions.
       * We can simply recreate the same-sized primary surface when
       * the display dimensions change.
       */
      if (s_pScreenPriv->dwLastWindowsWidth != LOWORD (lParam)
	  || s_pScreenPriv->dwLastWindowsHeight != HIWORD (lParam))
	{
	  /*
	   * NOTE: The non-DirectDraw engines set the ReleasePrimarySurface
	   * and CreatePrimarySurface function pointers to point
	   * to the no operation function, NoopDDA.  This allows us
	   * to blindly call these functions, even if they are not
	   * relevant to the current engine (e.g., Shadow GDI).
	   */

#if CYGDEBUG
	  winDebug ("winWindowProc - WM_DISPLAYCHANGE - Dimensions changed\n");
#endif
	  
	  /* Release the old primary surface */
	  (*s_pScreenPriv->pwinReleasePrimarySurface) (s_pScreen);

#if CYGDEBUG
	  winDebug ("winWindowProc - WM_DISPLAYCHANGE - Released "
		  "primary surface\n");
#endif

	  /* Create the new primary surface */
	  (*s_pScreenPriv->pwinCreatePrimarySurface) (s_pScreen);

#if CYGDEBUG
	  winDebug ("winWindowProc - WM_DISPLAYCHANGE - Recreated "
		  "primary surface\n");
#endif

#if 0
	  /* Multi-Window mode uses RandR for resizes */
	  if (s_pScreenInfo->fMultiWindow)
	    {
	      RRSetScreenConfig ();
	    }
#endif
	}
      else
	{
#if CYGDEBUG
	  winDebug ("winWindowProc - WM_DISPLAYCHANGE - Dimensions did not "
		  "change\n");
#endif
	}

      /* Store the new display dimensions and depth */
      if (s_pScreenInfo->fMultipleMonitors)
	{
	  s_pScreenPriv->dwLastWindowsWidth
	    = GetSystemMetrics (SM_CXVIRTUALSCREEN);
	  s_pScreenPriv->dwLastWindowsHeight
	    = GetSystemMetrics (SM_CYVIRTUALSCREEN);
	}
      else
	{
	  s_pScreenPriv->dwLastWindowsWidth
	    = GetSystemMetrics (SM_CXSCREEN);
	  s_pScreenPriv->dwLastWindowsHeight
	    = GetSystemMetrics (SM_CYSCREEN);
	}
      s_pScreenPriv->dwLastWindowsBitsPixel
	= GetDeviceCaps (s_pScreenPriv->hdcScreen, BITSPIXEL);
      break;

    case WM_SIZE:
      {
	SCROLLINFO		si;
	RECT			rcWindow;
	int			iWidth, iHeight;

#if CYGDEBUG
	winDebug ("winWindowProc - WM_SIZE\n");
#endif

	/* Break if we do not use scrollbars */
	if (!s_pScreenInfo->fScrollbars
	    || !s_pScreenInfo->fDecoration
#ifdef XWIN_MULTIWINDOWEXTWM
	    || s_pScreenInfo->fMWExtWM
#endif
	    || s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOW
	    || s_pScreenInfo->fMultiWindow
#endif
	    || s_pScreenInfo->fFullScreen)
	  break;

	/* No need to resize if we get minimized */
	if (wParam == SIZE_MINIMIZED)
	  return 0;

	/*
	 * Get the size of the whole window, including client area,
	 * scrollbars, and non-client area decorations (caption, borders).
	 * We do this because we need to check if the client area
	 * without scrollbars is large enough to display the whole visual.
	 * The new client area size passed by lParam already subtracts
	 * the size of the scrollbars if they are currently displayed.
	 * So checking is LOWORD(lParam) == visual_width and
	 * HIWORD(lParam) == visual_height will never tell us to hide
	 * the scrollbars because the client area would always be too small.
	 * GetClientRect returns the same sizes given by lParam, so we
	 * cannot use GetClientRect either.
	 */
	GetWindowRect (hwnd, &rcWindow);
	iWidth = rcWindow.right - rcWindow.left;
	iHeight = rcWindow.bottom - rcWindow.top;

	ErrorF ("winWindowProc - WM_SIZE - window w: %d h: %d, "
		"new client area w: %d h: %d\n",
		iWidth, iHeight, LOWORD (lParam), HIWORD (lParam));

	/* Subtract the frame size from the window size. */
	iWidth -= 2 * GetSystemMetrics (SM_CXSIZEFRAME);
	iHeight -= (2 * GetSystemMetrics (SM_CYSIZEFRAME)
		    + GetSystemMetrics (SM_CYCAPTION));

	/*
	 * Update scrollbar page sizes.
	 * NOTE: If page size == range, then the scrollbar is
	 * automatically hidden.
	 */

	/* Is the naked client area large enough to show the whole visual? */
	if (iWidth < s_pScreenInfo->dwWidth
	    || iHeight < s_pScreenInfo->dwHeight)
	  {
	    /* Client area too small to display visual, use scrollbars */
	    iWidth -= GetSystemMetrics (SM_CXVSCROLL);
	    iHeight -= GetSystemMetrics (SM_CYHSCROLL);
	  }
	
	/* Set the horizontal scrollbar page size */
	si.cbSize = sizeof (si);
	si.fMask = SIF_PAGE | SIF_RANGE;
	si.nMin = 0;
	si.nMax = s_pScreenInfo->dwWidth - 1;
	si.nPage = iWidth;
	SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);
	
	/* Set the vertical scrollbar page size */
	si.cbSize = sizeof (si);
	si.fMask = SIF_PAGE | SIF_RANGE;
	si.nMin = 0;
	si.nMax = s_pScreenInfo->dwHeight - 1;
	si.nPage = iHeight;
	SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

	/*
	 * NOTE: Scrollbars may have moved if they were at the 
	 * far right/bottom, so we query their current position.
	 */
	
	/* Get the horizontal scrollbar position and set the offset */
	si.cbSize = sizeof (si);
	si.fMask = SIF_POS;
	GetScrollInfo (hwnd, SB_HORZ, &si);
	s_pScreenInfo->dwXOffset = -si.nPos;
	
	/* Get the vertical scrollbar position and set the offset */
	si.cbSize = sizeof (si);
	si.fMask = SIF_POS;
	GetScrollInfo (hwnd, SB_VERT, &si);
	s_pScreenInfo->dwYOffset = -si.nPos;
      }
      return 0;

    case WM_VSCROLL:
      {
	SCROLLINFO		si;
	int			iVertPos;

#if CYGDEBUG
	winDebug ("winWindowProc - WM_VSCROLL\n");
#endif
      
	/* Get vertical scroll bar info */
	si.cbSize = sizeof (si);
	si.fMask = SIF_ALL;
	GetScrollInfo (hwnd, SB_VERT, &si);

	/* Save the vertical position for comparison later */
	iVertPos = si.nPos;

	/*
	 * Don't forget:
	 * moving the scrollbar to the DOWN, scroll the content UP
	 */
	switch (LOWORD(wParam))
	  {
	  case SB_TOP:
	    si.nPos = si.nMin;
	    break;
	  
	  case SB_BOTTOM:
	    si.nPos = si.nMax - si.nPage + 1;
	    break;

	  case SB_LINEUP:
	    si.nPos -= 1;
	    break;
	  
	  case SB_LINEDOWN:
	    si.nPos += 1;
	    break;
	  
	  case SB_PAGEUP:
	    si.nPos -= si.nPage;
	    break;
	  
	  case SB_PAGEDOWN:
	    si.nPos += si.nPage;
	    break;

	  case SB_THUMBTRACK:
	    si.nPos = si.nTrackPos;
	    break;

	  default:
	    break;
	  }

	/*
	 * We retrieve the position after setting it,
	 * because Windows may adjust it.
	 */
	si.fMask = SIF_POS;
	SetScrollInfo (hwnd, SB_VERT, &si, TRUE);
	GetScrollInfo (hwnd, SB_VERT, &si);
      
	/* Scroll the window if the position has changed */
	if (si.nPos != iVertPos)
	  {
	    /* Save the new offset for bit block transfers, etc. */
	    s_pScreenInfo->dwYOffset = -si.nPos;

	    /* Change displayed region in the window */
	    ScrollWindowEx (hwnd,
			    0,
			    iVertPos - si.nPos,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    SW_INVALIDATE);
	  
	    /* Redraw the window contents */
	    UpdateWindow (hwnd);
	  }
      }
      return 0;

    case WM_HSCROLL:
      {
	SCROLLINFO		si;
	int			iHorzPos;

#if CYGDEBUG
	winDebug ("winWindowProc - WM_HSCROLL\n");
#endif
      
	/* Get horizontal scroll bar info */
	si.cbSize = sizeof (si);
	si.fMask = SIF_ALL;
	GetScrollInfo (hwnd, SB_HORZ, &si);

	/* Save the horizontal position for comparison later */
	iHorzPos = si.nPos;

	/*
	 * Don't forget:
	 * moving the scrollbar to the RIGHT, scroll the content LEFT
	 */
	switch (LOWORD(wParam))
	  {
	  case SB_LEFT:
	    si.nPos = si.nMin;
	    break;
	  
	  case SB_RIGHT:
	    si.nPos = si.nMax - si.nPage + 1;
	    break;

	  case SB_LINELEFT:
	    si.nPos -= 1;
	    break;
	  
	  case SB_LINERIGHT:
	    si.nPos += 1;
	    break;
	  
	  case SB_PAGELEFT:
	    si.nPos -= si.nPage;
	    break;
	  
	  case SB_PAGERIGHT:
	    si.nPos += si.nPage;
	    break;

	  case SB_THUMBTRACK:
	    si.nPos = si.nTrackPos;
	    break;

	  default:
	    break;
	  }

	/*
	 * We retrieve the position after setting it,
	 * because Windows may adjust it.
	 */
	si.fMask = SIF_POS;
	SetScrollInfo (hwnd, SB_HORZ, &si, TRUE);
	GetScrollInfo (hwnd, SB_HORZ, &si);
      
	/* Scroll the window if the position has changed */
	if (si.nPos != iHorzPos)
	  {
	    /* Save the new offset for bit block transfers, etc. */
	    s_pScreenInfo->dwXOffset = -si.nPos;

	    /* Change displayed region in the window */
	    ScrollWindowEx (hwnd,
			    iHorzPos - si.nPos,
			    0,
			    NULL,
			    NULL,
			    NULL,
			    NULL,
			    SW_INVALIDATE);
	  
	    /* Redraw the window contents */
	    UpdateWindow (hwnd);
	  }
      }
      return 0;

    case WM_GETMINMAXINFO:
      {
	MINMAXINFO		*pMinMaxInfo = (MINMAXINFO *) lParam;
	int			iCaptionHeight;
	int			iBorderHeight, iBorderWidth;

#if CYGDEBUG	
	winDebug ("winWindowProc - WM_GETMINMAXINFO - pScreenInfo: %08x\n",
		s_pScreenInfo);
#endif

	/* Can't do anything without screen info */
	if (s_pScreenInfo == NULL
	    || !s_pScreenInfo->fScrollbars
	    || s_pScreenInfo->fFullScreen
	    || !s_pScreenInfo->fDecoration
#ifdef XWIN_MULTIWINDOWEXTWM
	    || s_pScreenInfo->fMWExtWM
#endif
	    || s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOW
	    || s_pScreenInfo->fMultiWindow
#endif
	    )
	  break;

	/*
	 * Here we can override the maximum tracking size, which
	 * is the largest size that can be assigned to our window
	 * via the sizing border.
	 */

	/*
	 * FIXME: Do we only need to do this once, since our visual size
	 * does not change?  Does Windows store this value statically
	 * once we have set it once?
	 */

	/* Get the border and caption sizes */
	iCaptionHeight = GetSystemMetrics (SM_CYCAPTION);
	iBorderWidth = 2 * GetSystemMetrics (SM_CXSIZEFRAME);
	iBorderHeight = 2 * GetSystemMetrics (SM_CYSIZEFRAME);
	
	/* Allow the full visual to be displayed */
	pMinMaxInfo->ptMaxTrackSize.x
	  = s_pScreenInfo->dwWidth + iBorderWidth;
	pMinMaxInfo->ptMaxTrackSize.y
	  = s_pScreenInfo->dwHeight + iBorderHeight + iCaptionHeight;
      }
      return 0;

    case WM_ERASEBKGND:
#if CYGDEBUG
      winDebug ("winWindowProc - WM_ERASEBKGND\n");
#endif
      /*
       * Pretend that we did erase the background but we don't care,
       * the application uses the full window estate. This avoids some
       * flickering when resizing.
       */
      return TRUE;

    case WM_PAINT:
#if CYGDEBUG
      winDebug ("winWindowProc - WM_PAINT\n");
#endif
      /* Only paint if we have privates and the server is enabled */
      if (s_pScreenPriv == NULL
	  || !s_pScreenPriv->fEnabled
	  || (s_pScreenInfo->fFullScreen && !s_pScreenPriv->fActive)
	  || s_pScreenPriv->fBadDepth)
	{
	  /* We don't want to paint */
	  break;
	}

      /* Break out here if we don't have a valid paint routine */
      if (s_pScreenPriv->pwinBltExposedRegions == NULL)
	break;
      
      /* Call the engine dependent repainter */
      (*s_pScreenPriv->pwinBltExposedRegions) (s_pScreen);
      return 0;

    case WM_PALETTECHANGED:
      {
#if CYGDEBUG
	winDebug ("winWindowProc - WM_PALETTECHANGED\n");
#endif
	/*
	 * Don't process if we don't have privates or a colormap,
	 * or if we have an invalid depth.
	 */
	if (s_pScreenPriv == NULL
	    || s_pScreenPriv->pcmapInstalled == NULL
	    || s_pScreenPriv->fBadDepth)
	  break;

	/* Return if we caused the palette to change */
	if ((HWND) wParam == hwnd)
	  {
	    /* Redraw the screen */
	    (*s_pScreenPriv->pwinRedrawScreen) (s_pScreen);
	    return 0;
	  }
	
	/* Reinstall the windows palette */
	(*s_pScreenPriv->pwinRealizeInstalledPalette) (s_pScreen);
	
	/* Redraw the screen */
	(*s_pScreenPriv->pwinRedrawScreen) (s_pScreen);
	return 0;
      }

    case WM_MOUSEMOVE:
      /* We can't do anything without privates */
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;

      /* Has the mouse pointer crossed screens? */
      if (s_pScreen != miPointerGetScreen(inputInfo.pointer))
	miPointerSetScreen (inputInfo.pointer, s_pScreenInfo->dwScreen,
			       GET_X_LPARAM(lParam)-s_pScreenInfo->dwXOffset,
			       GET_Y_LPARAM(lParam)-s_pScreenInfo->dwYOffset);

      /* Are we tracking yet? */
      if (!s_fTracking)
	{
	  TRACKMOUSEEVENT		tme;
	  
	  /* Setup data structure */
	  ZeroMemory (&tme, sizeof (tme));
	  tme.cbSize = sizeof (tme);
	  tme.dwFlags = TME_LEAVE;
	  tme.hwndTrack = hwnd;

	  /* Call the tracking function */
	  if (!(*g_fpTrackMouseEvent) (&tme))
	    ErrorF ("winWindowProc - _TrackMouseEvent failed\n");

	  /* Flag that we are tracking now */
	  s_fTracking = TRUE;
	}

      /* Hide or show the Windows mouse cursor */
      if (g_fSoftwareCursor && g_fCursor && (s_pScreenPriv->fActive || s_pScreenInfo->fLessPointer))
	{
	  /* Hide Windows cursor */
	  g_fCursor = FALSE;
	  ShowCursor (FALSE);
	}
      else if (g_fSoftwareCursor && !g_fCursor && !s_pScreenPriv->fActive
	       && !s_pScreenInfo->fLessPointer)
	{
	  /* Show Windows cursor */
	  g_fCursor = TRUE;
	  ShowCursor (TRUE);
	}
      
      /* Deliver absolute cursor position to X Server */
      miPointerAbsoluteCursor (GET_X_LPARAM(lParam)-s_pScreenInfo->dwXOffset,
			       GET_Y_LPARAM(lParam)-s_pScreenInfo->dwYOffset,
			       g_c32LastInputEventTime = GetTickCount ());
      return 0;

    case WM_NCMOUSEMOVE:
      /*
       * We break instead of returning 0 since we need to call
       * DefWindowProc to get the mouse cursor changes
       * and min/max/close button highlighting in Windows XP.
       * The Platform SDK says that you should return 0 if you
       * process this message, but it fails to mention that you
       * will give up any default functionality if you do return 0.
       */
      
      /* We can't do anything without privates */
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      
      /* Non-client mouse movement, show Windows cursor */
      if (g_fSoftwareCursor && !g_fCursor)
	{
	  g_fCursor = TRUE;
	  ShowCursor (TRUE);
	}
      break;

    case WM_MOUSELEAVE:
      /* Mouse has left our client area */

      /* Flag that we are no longer tracking */
      s_fTracking = FALSE;

      /* Show the mouse cursor, if necessary */
      if (g_fSoftwareCursor && !g_fCursor)
	{
	  g_fCursor = TRUE;
	  ShowCursor (TRUE);
	}
      return 0;

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      if (s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOWEXTWM
	  || s_pScreenInfo->fMWExtWM
#endif
	  )
	SetCapture (hwnd);
      return winMouseButtonsHandle (s_pScreen, ButtonPress, Button1, wParam);
      
    case WM_LBUTTONUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      if (s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOWEXTWM
	  || s_pScreenInfo->fMWExtWM
#endif
	  )
	ReleaseCapture ();
      return winMouseButtonsHandle (s_pScreen, ButtonRelease, Button1, wParam);

    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      if (s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOWEXTWM
	  || s_pScreenInfo->fMWExtWM
#endif
	  )
	SetCapture (hwnd);
      return winMouseButtonsHandle (s_pScreen, ButtonPress, Button2, wParam);
      
    case WM_MBUTTONUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      if (s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOWEXTWM
	  || s_pScreenInfo->fMWExtWM
#endif
	  )
	ReleaseCapture ();
      return winMouseButtonsHandle (s_pScreen, ButtonRelease, Button2, wParam);
      
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      if (s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOWEXTWM
	  || s_pScreenInfo->fMWExtWM
#endif
	  )
	SetCapture (hwnd);
      return winMouseButtonsHandle (s_pScreen, ButtonPress, Button3, wParam);
      
    case WM_RBUTTONUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      if (s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOWEXTWM
	  || s_pScreenInfo->fMWExtWM
#endif
	  )
	ReleaseCapture ();
      return winMouseButtonsHandle (s_pScreen, ButtonRelease, Button3, wParam);

    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      if (s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOWEXTWM
	  || s_pScreenInfo->fMWExtWM
#endif
	  )
	SetCapture (hwnd);
      return winMouseButtonsHandle (s_pScreen, ButtonPress, HIWORD(wParam) + 5, wParam);
    case WM_XBUTTONUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      if (s_pScreenInfo->fRootless
#ifdef XWIN_MULTIWINDOWEXTWM
	  || s_pScreenInfo->fMWExtWM
#endif
	  )
	ReleaseCapture ();
      return winMouseButtonsHandle (s_pScreen, ButtonRelease, HIWORD(wParam) + 5, wParam);

    case WM_TIMER:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;

      /* Branch on the timer id */
      switch (wParam)
	{
	case WIN_E3B_TIMER_ID:
	  /* Send delayed button press */
	  winMouseButtonsSendEvent (ButtonPress,
				    s_pScreenPriv->iE3BCachedPress);

	  /* Kill this timer */
	  KillTimer (s_pScreenPriv->hwndScreen, WIN_E3B_TIMER_ID);

	  /* Clear screen privates flags */
	  s_pScreenPriv->iE3BCachedPress = 0;
	  break;

	case WIN_POLLING_MOUSE_TIMER_ID:
	  {
	    POINT		point;
	    WPARAM		wL, wM, wR, wShift, wCtrl;
	    LPARAM		lPos;
	    
	    /* Get the current position of the mouse cursor */
	    GetCursorPos (&point);
	    
	    /* Map from screen (-X, -Y) to root (0, 0) */
	    point.x -= GetSystemMetrics (SM_XVIRTUALSCREEN);
	    point.y -= GetSystemMetrics (SM_YVIRTUALSCREEN);
	    
	    /* Deliver absolute cursor position to X Server */
	    miPointerAbsoluteCursor (point.x, point.y,
				     g_c32LastInputEventTime = GetTickCount());

	    /* Check if a button was released but we didn't see it */
	    GetCursorPos (&point);
	    wL = (GetKeyState (VK_LBUTTON) & 0x8000)?MK_LBUTTON:0;
	    wM = (GetKeyState (VK_MBUTTON) & 0x8000)?MK_MBUTTON:0;
	    wR = (GetKeyState (VK_RBUTTON) & 0x8000)?MK_RBUTTON:0;
	    wShift = (GetKeyState (VK_SHIFT) & 0x8000)?MK_SHIFT:0;
	    wCtrl = (GetKeyState (VK_CONTROL) & 0x8000)?MK_CONTROL:0;
	    lPos = MAKELPARAM(point.x, point.y);
	    if (g_fButton[0] & !wL)
	    PostMessage (hwnd, WM_LBUTTONUP, wCtrl|wM|wR|wShift, lPos);
	    if (g_fButton[1] & !wM)
	      PostMessage (hwnd, WM_MBUTTONUP, wCtrl|wL|wR|wShift, lPos);
	    if (g_fButton[2] & !wR)
	      PostMessage (hwnd, WM_RBUTTONUP, wCtrl|wL|wM|wShift, lPos);
	  }
	}
      return 0;

    case WM_CTLCOLORSCROLLBAR:
      FatalError ("winWindowProc - WM_CTLCOLORSCROLLBAR - We are not "
		  "supposed to get this message.  Exiting.\n");
      return 0;

    case WM_MOUSEWHEEL:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
#if CYGDEBUG
      winDebug ("winWindowProc - WM_MOUSEWHEEL\n");
#endif
      winMouseWheel (s_pScreen, GET_WHEEL_DELTA_WPARAM(wParam));
      break;

    case WM_SETFOCUS:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;

      /* Save handle of our main window that last received focus */
      g_hwndKeyboardFocus = hwnd;

      /* Restore the state of all mode keys */
      winRestoreModeKeyStates ();

      /* Add the keyboard hook if possible */
      if (g_fKeyboardHookLL)
	g_fKeyboardHookLL = winInstallKeyboardHookLL ();
      return 0;

    case WM_KILLFOCUS:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;

      /* Clear handle of our main window that last received focus */
      g_hwndKeyboardFocus = NULL;

      /* Release any pressed keys */
      winKeybdReleaseKeys ();

      /* Remove our keyboard hook if it is installed */
      winRemoveKeyboardHookLL ();
      return 0;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;

      /*
       * FIXME: Catching Alt-F4 like this is really terrible.  This should
       * be generalized to handle other Windows keyboard signals.  Actually,
       * the list keys to catch and the actions to perform when caught should
       * be configurable; that way user's can customize the keys that they
       * need to have passed through to their window manager or apps, or they
       * can remap certain actions to new key codes that do not conflict
       * with the X apps that they are using.  Yeah, that'll take awhile.
       */
      if ((s_pScreenInfo->fUseWinKillKey && wParam == VK_F4
	   && (GetKeyState (VK_MENU) & 0x8000))
	  || (s_pScreenInfo->fUseUnixKillKey && wParam == VK_BACK
	      && (GetKeyState (VK_MENU) & 0x8000)
	      && (GetKeyState (VK_CONTROL) & 0x8000)))
	{
	  /*
	   * Better leave this message here, just in case some unsuspecting
	   * user enters Alt + F4 and is surprised when the application
	   * quits.
	   */
	  ErrorF ("winWindowProc - WM_*KEYDOWN - Closekey hit, quitting\n");
	  
	  /* Display Exit dialog */
	  winDisplayExitDialog (s_pScreenPriv);
	  return 0;
	}
      
      /*
       * Don't do anything for the Windows keys, as focus will soon
       * be returned to Windows.  We may be able to trap the Windows keys,
       * but we should determine if that is desirable before doing so.
       */
      if ((wParam == VK_LWIN || wParam == VK_RWIN) && !g_fKeyboardHookLL)
	break;

#ifdef XKB
      /* 
       * Discard presses generated from Windows auto-repeat
       * ago: Only discard them if XKB is not disabled 
       */
      if (!g_winInfo.xkb.disable && (lParam & (1<<30)))
      {
        switch (wParam)
        {
          /* ago: Pressing LControl while RControl is pressed is 
           * Indicated as repeat. Fix this!
           */
          case VK_CONTROL:
          case VK_SHIFT:
            if (winCheckKeyPressed(wParam, lParam))
              return 0;
            break;
          default:
            return 0;
        }
      } 
#endif 
      
      /* Discard fake Ctrl_L presses that precede AltGR on non-US keyboards */
      if (winIsFakeCtrl_L (message, wParam, lParam))
	return 0;
      
      /* Translate Windows key code to X scan code */
      winTranslateKey (wParam, lParam, &iScanCode);

      /* Ignore repeats for CapsLock */
      if (wParam == VK_CAPITAL)
	lParam = 1;

      /* Send the key event(s) */
      for (i = 0; i < LOWORD(lParam); ++i)
	winSendKeyEvent (iScanCode, TRUE);
      return 0;

    case WM_SYSKEYUP:
    case WM_KEYUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;

      /*
       * Don't do anything for the Windows keys, as focus will soon
       * be returned to Windows.  We may be able to trap the Windows keys,
       * but we should determine if that is desirable before doing so.
       */
      if ((wParam == VK_LWIN || wParam == VK_RWIN) && !g_fKeyboardHookLL)
	break;

      /* Ignore the fake Ctrl_L that follows an AltGr release */
      if (winIsFakeCtrl_L (message, wParam, lParam))
	return 0;

      /* Enqueue a keyup event */
      winTranslateKey (wParam, lParam, &iScanCode);
      winSendKeyEvent (iScanCode, FALSE);

      /* Release all pressed shift keys */
      if (wParam == VK_SHIFT) 
        winFixShiftKeys (iScanCode);
      return 0;

    case WM_HOTKEY:
      if (s_pScreenPriv == NULL)
	break;

      /* Call the engine-specific hot key handler */
      (*s_pScreenPriv->pwinHotKeyAltTab) (s_pScreen);
      return 0;

    case WM_ACTIVATE:
      if (s_pScreenPriv == NULL
	  || s_pScreenInfo->fIgnoreInput)
	break;

      /* TODO: Override display of window when we have a bad depth */
      if (LOWORD(wParam) != WA_INACTIVE && s_pScreenPriv->fBadDepth)
	{
	  ErrorF ("winWindowProc - WM_ACTIVATE - Bad depth, trying "
		  "to override window activation\n");

	  /* Minimize the window */
	  ShowWindow (hwnd, SW_MINIMIZE);

	  /* Display dialog box */
	  if (g_hDlgDepthChange != NULL)
	    {
	      /* Make the existing dialog box active */
	      SetActiveWindow (g_hDlgDepthChange);
	    }
	  else
	    {
	      /* TODO: Recreate the dialog box and bring to the top */
	      ShowWindow (g_hDlgDepthChange, SW_SHOWDEFAULT);
	    }

	  /* Don't do any other processing of this message */
	  return 0;
	}

#if CYGDEBUG
      winDebug ("winWindowProc - WM_ACTIVATE\n");
#endif

      /*
       * Focus is being changed to another window.
       * The other window may or may not belong to
       * our process.
       */

      /* Clear any lingering wheel delta */
      s_pScreenPriv->iDeltaZ = 0;

      /* Reshow the Windows mouse cursor if we are being deactivated */
      if (g_fSoftwareCursor && LOWORD(wParam) == WA_INACTIVE
	  && !g_fCursor)
	{
	  /* Show Windows cursor */
	  g_fCursor = TRUE;
	  ShowCursor (TRUE);
	}
      return 0;

    case WM_ACTIVATEAPP:
      if (s_pScreenPriv == NULL
	  || s_pScreenInfo->fIgnoreInput)
	break;

#if CYGDEBUG || TRUE
      winDebug ("winWindowProc - WM_ACTIVATEAPP\n");
#endif

      /* Activate or deactivate */
      s_pScreenPriv->fActive = wParam;

      /* Reshow the Windows mouse cursor if we are being deactivated */
      if (g_fSoftwareCursor && !s_pScreenPriv->fActive
	  && !g_fCursor)
	{
	  /* Show Windows cursor */
	  g_fCursor = TRUE;
	  ShowCursor (TRUE);
	}

#ifdef XWIN_CLIPBOARD
      /* Make sure the clipboard chain is ok. */
      winFixClipboardChain ();
#endif

      /* Call engine specific screen activation/deactivation function */
      (*s_pScreenPriv->pwinActivateApp) (s_pScreen);

#ifdef XWIN_MULTIWINDOWEXTWM
      if (s_pScreenPriv->fActive)
	{
	  /* Restack all window unless using built-in wm. */
	  if (s_pScreenInfo->fInternalWM && s_pScreenInfo->fAnotherWMRunning)
	    winMWExtWMRestackWindows (s_pScreen);
	}
#endif

      return 0;

    case WM_COMMAND:
      switch (LOWORD (wParam))
	{
	case ID_APP_EXIT:
	  /* Display Exit dialog */
	  winDisplayExitDialog (s_pScreenPriv);
	  return 0;

#ifdef XWIN_MULTIWINDOW
	case ID_APP_HIDE_ROOT:
	  if (s_pScreenPriv->fRootWindowShown)
	    ShowWindow (s_pScreenPriv->hwndScreen, SW_HIDE);
	  else
	    ShowWindow (s_pScreenPriv->hwndScreen, SW_SHOW);
	  s_pScreenPriv->fRootWindowShown = !s_pScreenPriv->fRootWindowShown;
	  return 0;
#endif

	case ID_APP_ABOUT:
	  /* Display the About box */
	  winDisplayAboutDialog (s_pScreenPriv);
	  return 0;

	default:
	  /* It's probably one of the custom menus... */
	  if (HandleCustomWM_COMMAND (0, LOWORD (wParam)))
	    return 0;
	}
      break;

    case WM_ENDSESSION:
    case WM_GIVEUP:
      /* Tell X that we are giving up */
#ifdef XWIN_MULTIWINDOW
      if (s_pScreenInfo->fMultiWindow)
	winDeinitMultiWindowWM ();
#endif
      GiveUp (0);
      return 0;

    case WM_CLOSE:
      /* Display Exit dialog */
      winDisplayExitDialog (s_pScreenPriv);
      return 0;

    case WM_SETCURSOR:
      if (LOWORD(lParam) == HTCLIENT)
	{
	  if (!g_fSoftwareCursor) SetCursor (s_pScreenPriv->cursor.handle);
	  return TRUE;
	}
      break;

#ifdef XWIN_MULTIWINDOWEXTWM
    case WM_MANAGE:
      ErrorF ("winWindowProc - WM_MANAGE\n");
      s_pScreenInfo->fAnotherWMRunning = FALSE;

      if (s_pScreenInfo->fInternalWM)
	{
	  EnumThreadWindows (g_dwCurrentThreadID, winMWExtWMDecorateWindow, 0);
	  //RootlessRepositionWindows (s_pScreen);
	}
      break;

    case WM_UNMANAGE:
      ErrorF ("winWindowProc - WM_UNMANAGE\n");
      s_pScreenInfo->fAnotherWMRunning = TRUE;

      if (s_pScreenInfo->fInternalWM)
	{
	  EnumThreadWindows (g_dwCurrentThreadID, winMWExtWMDecorateWindow, 0);
	  winMWExtWMRestackWindows (s_pScreen);
	}
      break;
#endif

    default:
      if(message == s_uTaskbarRestart)
	{
	  winInitNotifyIcon (s_pScreenPriv);
	}
      break;
    }

  return DefWindowProc (hwnd, message, wParam, lParam);
}
