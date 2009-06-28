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
 * Authors:	Kensuke Matsuzaki
 *		Earle F. Philhower, III
 *		Harold L Hunt II
 */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"
#include "dixevents.h"
#include "winmultiwindowclass.h"
#include "winprefs.h"
#include "winmsg.h"
#include "inputstr.h"

/*
 * External global variables
 */

extern Bool			g_fCursor;
extern Bool			g_fKeyboardHookLL;
extern Bool			g_fSoftwareCursor;
extern Bool			g_fButton[3];


/*
 * Local globals
 */

static UINT_PTR		g_uipMousePollingTimerID = 0;


/*
 * Constant defines
 */

#define MOUSE_POLLING_INTERVAL		500
#define WIN_MULTIWINDOW_SHAPE		YES


/*
 * ConstrainSize - Taken from TWM sources - Respects hints for sizing
 */
#define makemult(a,b) ((b==1) ? (a) : (((int)((a)/(b))) * (b)) )
static void
ConstrainSize (WinXSizeHints hints, int *widthp, int *heightp)
{
  int minWidth, minHeight, maxWidth, maxHeight, xinc, yinc, delta;
  int baseWidth, baseHeight;
  int dwidth = *widthp, dheight = *heightp;
  
  if (hints.flags & PMinSize)
    {
      minWidth = hints.min_width;
      minHeight = hints.min_height;
    }
  else if (hints.flags & PBaseSize)
    {
      minWidth = hints.base_width;
      minHeight = hints.base_height;
    }
  else
    minWidth = minHeight = 1;
  
  if (hints.flags & PBaseSize)
    {
      baseWidth = hints.base_width;
      baseHeight = hints.base_height;
    } 
  else if (hints.flags & PMinSize)
    {
      baseWidth = hints.min_width;
      baseHeight = hints.min_height;
    }
  else
    baseWidth = baseHeight = 0;

  if (hints.flags & PMaxSize)
    {
      maxWidth = hints.max_width;
      maxHeight = hints.max_height;
    }
  else
    {
      maxWidth = MAXINT;
      maxHeight = MAXINT;
    }

  if (hints.flags & PResizeInc)
    {
      xinc = hints.width_inc;
      yinc = hints.height_inc;
    }
  else
    xinc = yinc = 1;

  /*
   * First, clamp to min and max values
   */
  if (dwidth < minWidth)
    dwidth = minWidth;
  if (dheight < minHeight)
    dheight = minHeight;

  if (dwidth > maxWidth)
    dwidth = maxWidth;
  if (dheight > maxHeight)
    dheight = maxHeight;

  /*
   * Second, fit to base + N * inc
   */
  dwidth = ((dwidth - baseWidth) / xinc * xinc) + baseWidth;
  dheight = ((dheight - baseHeight) / yinc * yinc) + baseHeight;
  
  /*
   * Third, adjust for aspect ratio
   */

  /*
   * The math looks like this:
   *
   * minAspectX    dwidth     maxAspectX
   * ---------- <= ------- <= ----------
   * minAspectY    dheight    maxAspectY
   *
   * If that is multiplied out, then the width and height are
   * invalid in the following situations:
   *
   * minAspectX * dheight > minAspectY * dwidth
   * maxAspectX * dheight < maxAspectY * dwidth
   * 
   */
  
  if (hints.flags & PAspect)
    {
      if (hints.min_aspect.x * dheight > hints.min_aspect.y * dwidth)
        {
	  delta = makemult(hints.min_aspect.x * dheight / hints.min_aspect.y - dwidth, xinc);
	  if (dwidth + delta <= maxWidth)
	    dwidth += delta;
	  else
            {
	      delta = makemult(dheight - dwidth*hints.min_aspect.y/hints.min_aspect.x, yinc);
	      if (dheight - delta >= minHeight)
		dheight -= delta;
            }
        }
      
      if (hints.max_aspect.x * dheight < hints.max_aspect.y * dwidth)
        {
	  delta = makemult(dwidth * hints.max_aspect.y / hints.max_aspect.x - dheight, yinc);
	  if (dheight + delta <= maxHeight)
	    dheight += delta;
	  else
            {
	      delta = makemult(dwidth - hints.max_aspect.x*dheight/hints.max_aspect.y, xinc);
	      if (dwidth - delta >= minWidth)
		dwidth -= delta;
            }
        }
    }
  
  /* Return computed values */
  *widthp = dwidth;
  *heightp = dheight;
}
#undef makemult



/*
 * ValidateSizing - Ensures size request respects hints
 */
static int
ValidateSizing (HWND hwnd, WindowPtr pWin,
		WPARAM wParam, LPARAM lParam)
{
  WinXSizeHints sizeHints;
  RECT *rect;
  int iWidth, iHeight;

  /* Invalid input checking */
  if (pWin==NULL || lParam==0)
    return FALSE;

  /* No size hints, no checking */
  if (!winMultiWindowGetWMNormalHints (pWin, &sizeHints))
    return FALSE;
  
  /* Avoid divide-by-zero */
  if (sizeHints.flags & PResizeInc)
    {
      if (sizeHints.width_inc == 0) sizeHints.width_inc = 1;
      if (sizeHints.height_inc == 0) sizeHints.height_inc = 1;
    }
  
  rect = (RECT*)lParam;
  
  iWidth = rect->right - rect->left;
  iHeight = rect->bottom - rect->top;

  /* Now remove size of any borders */
  iWidth -= 2 * GetSystemMetrics(SM_CXSIZEFRAME);
  iHeight -= (GetSystemMetrics(SM_CYCAPTION)
	      + 2 * GetSystemMetrics(SM_CYSIZEFRAME));
	      

  /* Constrain the size to legal values */
  ConstrainSize (sizeHints, &iWidth, &iHeight);

  /* Add back the borders */
  iWidth += 2 * GetSystemMetrics(SM_CXSIZEFRAME);
  iHeight += (GetSystemMetrics(SM_CYCAPTION)
	      + 2 * GetSystemMetrics(SM_CYSIZEFRAME));

  /* Adjust size according to where we're dragging from */
  switch(wParam) {
  case WMSZ_TOP:
  case WMSZ_TOPRIGHT:
  case WMSZ_BOTTOM:
  case WMSZ_BOTTOMRIGHT:
  case WMSZ_RIGHT:
    rect->right = rect->left + iWidth;
    break;
  default:
    rect->left = rect->right - iWidth;
    break;
  }
  switch(wParam) {
  case WMSZ_BOTTOM:
  case WMSZ_BOTTOMRIGHT:
  case WMSZ_BOTTOMLEFT:
  case WMSZ_RIGHT:
  case WMSZ_LEFT:
    rect->bottom = rect->top + iHeight;
    break;
  default:
    rect->top = rect->bottom - iHeight;
    break;
  }
  return TRUE;
}

extern Bool winInDestroyWindowsWindow;
static Bool winInRaiseWindow = FALSE;
static void winRaiseWindow(WindowPtr pWin)
{
  if (!winInDestroyWindowsWindow && !winInRaiseWindow)
  {
    BOOL oldstate = winInRaiseWindow;
    winInRaiseWindow = TRUE;
    /* Call configure window directly to make sure it gets processed 
     * in time
     */
    XID vlist[1] = { 0 };
    ConfigureWindow(pWin, CWStackMode, vlist, serverClient); 
    winInRaiseWindow = oldstate;
  }
}


/*
 * winTopLevelWindowProc - Window procedure for all top-level Windows windows.
 */

LRESULT CALLBACK
winTopLevelWindowProc (HWND hwnd, UINT message, 
		       WPARAM wParam, LPARAM lParam)
{
  POINT			ptMouse;
  HDC			hdcUpdate;
  PAINTSTRUCT		ps;
  WindowPtr		pWin = NULL;
  winPrivWinPtr	        pWinPriv = NULL;
  ScreenPtr		s_pScreen = NULL;
  winPrivScreenPtr	s_pScreenPriv = NULL;
  winScreenInfo		*s_pScreenInfo = NULL;
  HWND			hwndScreen = NULL;
  DrawablePtr		pDraw = NULL;
  winWMMessageRec	wmMsg;
  Bool                  fWMMsgInitialized = FALSE;
  static Bool		s_fTracking = FALSE;
  Bool			needRestack = FALSE;
  LRESULT		ret;

#if CYGDEBUG
  winDebugWin32Message("winTopLevelWindowProc", hwnd, message, wParam, lParam);
#endif
  
  /* Check if the Windows window property for our X window pointer is valid */
  if ((pWin = GetProp (hwnd, WIN_WINDOW_PROP)) != NULL)
    {
      /* Our X window pointer is valid */

      /* Get pointers to the drawable and the screen */
      pDraw		= &pWin->drawable;
      s_pScreen		= pWin->drawable.pScreen;

      /* Get a pointer to our window privates */
      pWinPriv		= winGetWindowPriv(pWin);

      /* Get pointers to our screen privates and screen info */
      s_pScreenPriv	= pWinPriv->pScreenPriv;
      s_pScreenInfo	= s_pScreenPriv->pScreenInfo;

      /* Get the handle for our screen-sized window */
      hwndScreen	= s_pScreenPriv->hwndScreen;

      /* */
      wmMsg.msg		= 0;
      wmMsg.hwndWindow	= hwnd;
      wmMsg.iWindow	= (Window)GetProp (hwnd, WIN_WID_PROP);

      wmMsg.iX		= pDraw->x;
      wmMsg.iY		= pDraw->y;
      wmMsg.iWidth	= pDraw->width;
      wmMsg.iHeight	= pDraw->height;

      fWMMsgInitialized = TRUE;

#if 0
      /*
       * Print some debugging information
       */

      ErrorF ("hWnd %08X\n", hwnd);
      ErrorF ("pWin %08X\n", pWin);
      ErrorF ("pDraw %08X\n", pDraw);
      ErrorF ("\ttype %08X\n", pWin->drawable.type);
      ErrorF ("\tclass %08X\n", pWin->drawable.class);
      ErrorF ("\tdepth %08X\n", pWin->drawable.depth);
      ErrorF ("\tbitsPerPixel %08X\n", pWin->drawable.bitsPerPixel);
      ErrorF ("\tid %08X\n", pWin->drawable.id);
      ErrorF ("\tx %08X\n", pWin->drawable.x);
      ErrorF ("\ty %08X\n", pWin->drawable.y);
      ErrorF ("\twidth %08X\n", pWin->drawable.width);
      ErrorF ("\thenght %08X\n", pWin->drawable.height);
      ErrorF ("\tpScreen %08X\n", pWin->drawable.pScreen);
      ErrorF ("\tserialNumber %08X\n", pWin->drawable.serialNumber);
      ErrorF ("g_iWindowPrivateKey %p\n", g_iWindowPrivateKey);
      ErrorF ("pWinPriv %08X\n", pWinPriv);
      ErrorF ("s_pScreenPriv %08X\n", s_pScreenPriv);
      ErrorF ("s_pScreenInfo %08X\n", s_pScreenInfo);
      ErrorF ("hwndScreen %08X\n", hwndScreen);
#endif
    }

  /* Branch on message type */
  switch (message)
    {
    case WM_CREATE:

      /* */
      SetProp (hwnd,
	       WIN_WINDOW_PROP,
	       (HANDLE)((LPCREATESTRUCT) lParam)->lpCreateParams);
      
      /* */
      SetProp (hwnd,
	       WIN_WID_PROP,
	       (HANDLE)winGetWindowID (((LPCREATESTRUCT) lParam)->lpCreateParams));

      /*
       * Make X windows' Z orders sync with Windows windows because
       * there can be AlwaysOnTop windows overlapped on the window
       * currently being created.
       */
      winReorderWindowsMultiWindow ();

      /* Fix a 'round title bar corner background should be transparent not black' problem when first painted */
      RECT rWindow;
      HRGN hRgnWindow;
      GetWindowRect(hwnd, &rWindow);
      hRgnWindow = CreateRectRgnIndirect(&rWindow);
      SetWindowRgn (hwnd, hRgnWindow, TRUE);
      DeleteObject(hRgnWindow);

      return 0;

    case WM_INIT_SYS_MENU:
      /*
       * Add whatever the setup file wants to for this window
       */
      SetupSysMenu ((unsigned long)hwnd);
      return 0;

    case WM_SYSCOMMAND:
      /*
       * Any window menu items go through here
       */
      if (HandleCustomWM_COMMAND ((unsigned long)hwnd, LOWORD(wParam)))
      {
        /* Don't pass customized menus to DefWindowProc */
        return 0;
      }
      if (wParam == SC_RESTORE || wParam == SC_MAXIMIZE)
      {
        WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(wndpl);
	if (GetWindowPlacement(hwnd, &wndpl) && wndpl.showCmd == SW_SHOWMINIMIZED)
          needRestack = TRUE;
      }
      break;

    case WM_INITMENU:
      /* Checks/Unchecks any menu items before they are displayed */
      HandleCustomWM_INITMENU ((unsigned long)hwnd, wParam);
      break;

    case WM_PAINT:
      /* Only paint if our window handle is valid */
      if (hwndScreen == NULL)
	break;

      /* BeginPaint gives us an hdc that clips to the invalidated region */
      hdcUpdate = BeginPaint (hwnd, &ps);
      /* Avoid the BitBlt's if the PAINTSTRUCT is bogus */
      if (ps.rcPaint.right==0 && ps.rcPaint.bottom==0 && ps.rcPaint.left==0 && ps.rcPaint.top==0)
      {
	EndPaint (hwnd, &ps);
	return 0;
      }

      /* Try to copy from the shadow buffer */
      if (!BitBlt (hdcUpdate,
		   ps.rcPaint.left, ps.rcPaint.top,
		   ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top,
		   s_pScreenPriv->hdcShadow,
		   ps.rcPaint.left + pWin->drawable.x, ps.rcPaint.top + pWin->drawable.y,
		   SRCCOPY))
	{
	  LPVOID lpMsgBuf;
	  
	  /* Display a fancy error message */
	  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			 FORMAT_MESSAGE_FROM_SYSTEM | 
			 FORMAT_MESSAGE_IGNORE_INSERTS,
			 NULL,
			 GetLastError (),
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			 (LPTSTR) &lpMsgBuf,
			 0, NULL);

	  ErrorF ("winTopLevelWindowProc - BitBlt failed: %s\n",
		  (LPSTR)lpMsgBuf);
	  LocalFree (lpMsgBuf);
	}

      /* EndPaint frees the DC */
      EndPaint (hwnd, &ps);
      return 0;

    case WM_MOUSEMOVE:
      /* Unpack the client area mouse coordinates */
      ptMouse.x = GET_X_LPARAM(lParam);
      ptMouse.y = GET_Y_LPARAM(lParam);

      /* Translate the client area mouse coordinates to screen coordinates */
      ClientToScreen (hwnd, &ptMouse);

      /* Screen Coords from (-X, -Y) -> Root Window (0, 0) */
      ptMouse.x -= GetSystemMetrics (SM_XVIRTUALSCREEN);
      ptMouse.y -= GetSystemMetrics (SM_YVIRTUALSCREEN);

      /* We can't do anything without privates */
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;

      /* Has the mouse pointer crossed screens? */
      if (s_pScreen != miPointerGetScreen(inputInfo.pointer))
	miPointerSetScreen (inputInfo.pointer, s_pScreenInfo->dwScreen,
			       ptMouse.x - s_pScreenInfo->dwXOffset,
			       ptMouse.y - s_pScreenInfo->dwYOffset);

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
	    ErrorF ("winTopLevelWindowProc - _TrackMouseEvent failed\n");

	  /* Flag that we are tracking now */
	  s_fTracking = TRUE;
	}
      
      /* Hide or show the Windows mouse cursor */
      if (g_fSoftwareCursor && g_fCursor)
	{
	  /* Hide Windows cursor */
	  g_fCursor = FALSE;
	  ShowCursor (FALSE);
	}

      /* Kill the timer used to poll mouse events */
      if (g_uipMousePollingTimerID != 0)
	{
	  KillTimer (s_pScreenPriv->hwndScreen, WIN_POLLING_MOUSE_TIMER_ID);
	  g_uipMousePollingTimerID = 0;
	}

      /* Deliver absolute cursor position to X Server */
      miPointerAbsoluteCursor (ptMouse.x - s_pScreenInfo->dwXOffset,
			       ptMouse.y - s_pScreenInfo->dwYOffset,
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

      /*
       * Timer to poll mouse events.  This is needed to make
       * programs like xeyes follow the mouse properly.
       */
      if (g_uipMousePollingTimerID == 0)
	g_uipMousePollingTimerID = SetTimer (s_pScreenPriv->hwndScreen,
					     WIN_POLLING_MOUSE_TIMER_ID,
					     MOUSE_POLLING_INTERVAL,
					     NULL);
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

      /*
       * Timer to poll mouse events.  This is needed to make
       * programs like xeyes follow the mouse properly.
       */
      if (g_uipMousePollingTimerID == 0)
	g_uipMousePollingTimerID = SetTimer (s_pScreenPriv->hwndScreen,
					     WIN_POLLING_MOUSE_TIMER_ID,
					     MOUSE_POLLING_INTERVAL,
					     NULL);
      return 0;

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      g_fButton[0] = TRUE;
      return winMouseButtonsHandle (s_pScreen, ButtonPress, Button1, wParam);
      
    case WM_LBUTTONUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      g_fButton[0] = FALSE;
      return winMouseButtonsHandle (s_pScreen, ButtonRelease, Button1, wParam);

    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      g_fButton[1] = TRUE;
      return winMouseButtonsHandle (s_pScreen, ButtonPress, Button2, wParam);
      
    case WM_MBUTTONUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      g_fButton[1] = FALSE;
      return winMouseButtonsHandle (s_pScreen, ButtonRelease, Button2, wParam);
      
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      g_fButton[2] = TRUE;
      return winMouseButtonsHandle (s_pScreen, ButtonPress, Button3, wParam);
      
    case WM_RBUTTONUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      g_fButton[2] = FALSE;
      return winMouseButtonsHandle (s_pScreen, ButtonRelease, Button3, wParam);

    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      return winMouseButtonsHandle (s_pScreen, ButtonPress, HIWORD(wParam) + 5, wParam);
    case WM_XBUTTONUP:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;
      return winMouseButtonsHandle (s_pScreen, ButtonRelease, HIWORD(wParam) + 5, wParam);

    case WM_MOUSEWHEEL:
      
      /* Pass the message to the root window */
      SendMessage (hwndScreen, message, wParam, lParam);
      return 0;

    case WM_SETFOCUS:
      if (s_pScreenPriv == NULL || s_pScreenInfo->fIgnoreInput)
	break;

      winRestoreModeKeyStates ();

      /* Add the keyboard hook if possible */
      if (g_fKeyboardHookLL)
	g_fKeyboardHookLL = winInstallKeyboardHookLL ();
      return 0;
      
    case WM_KILLFOCUS:
      /* Pop any pressed keys since we are losing keyboard focus */
      winKeybdReleaseKeys ();

      /* Remove our keyboard hook if it is installed */
      winRemoveKeyboardHookLL ();
      return 0;

    case WM_SYSDEADCHAR:      
    case WM_DEADCHAR:
      /*
       * NOTE: We do nothing with WM_*CHAR messages,
       * nor does the root window, so we can just toss these messages.
       */
      return 0;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:

      /*
       * Don't pass Alt-F4 key combo to root window,
       * let Windows translate to WM_CLOSE and close this top-level window.
       *
       * NOTE: We purposely don't check the fUseWinKillKey setting because
       * it should only apply to the key handling for the root window,
       * not for top-level window-manager windows.
       *
       * ALSO NOTE: We do pass Ctrl-Alt-Backspace to the root window
       * because that is a key combo that no X app should be expecting to
       * receive, since it has historically been used to shutdown the X server.
       * Passing Ctrl-Alt-Backspace to the root window preserves that
       * behavior, assuming that -unixkill has been passed as a parameter.
       */
      if (wParam == VK_F4 && (GetKeyState (VK_MENU) & 0x8000))
	  break;

#if CYGWINDOWING_DEBUG
      if (wParam == VK_ESCAPE)
	{
	  /* Place for debug: put any tests and dumps here */
	  WINDOWPLACEMENT windPlace;
	  RECT rc;
	  LPRECT pRect;
	  
	  windPlace.length = sizeof (WINDOWPLACEMENT);
	  GetWindowPlacement (hwnd, &windPlace);
	  pRect = &windPlace.rcNormalPosition;
	  ErrorF ("\nCYGWINDOWING Dump:\n"
		  "\tdrawable: (%hd, %hd) - %hdx%hd\n", pDraw->x,
		  pDraw->y, pDraw->width, pDraw->height);
	  ErrorF ("\twindPlace: (%ld, %ld) - %ldx%ld\n", pRect->left,
		  pRect->top, pRect->right - pRect->left,
		  pRect->bottom - pRect->top);
	  if (GetClientRect (hwnd, &rc))
	    {
	      pRect = &rc;
	      ErrorF ("\tClientRect: (%ld, %ld) - %ldx%ld\n", pRect->left,
		      pRect->top, pRect->right - pRect->left,
		      pRect->bottom - pRect->top);
	    }
	  if (GetWindowRect (hwnd, &rc))
	    {
	      pRect = &rc;
	      ErrorF ("\tWindowRect: (%ld, %ld) - %ldx%ld\n", pRect->left,
		      pRect->top, pRect->right - pRect->left,
		      pRect->bottom - pRect->top);
	    }
	  ErrorF ("\n");
	}
#endif
      
      /* Pass the message to the root window */
      return winWindowProc(hwndScreen, message, wParam, lParam);

    case WM_SYSKEYUP:
    case WM_KEYUP:


      /* Pass the message to the root window */
      return winWindowProc(hwndScreen, message, wParam, lParam);

    case WM_HOTKEY:

      /* Pass the message to the root window */
      SendMessage (hwndScreen, message, wParam, lParam);
      return 0;

    case WM_ACTIVATE:

      /* Pass the message to the root window */
      SendMessage (hwndScreen, message, wParam, lParam);

      if (LOWORD(wParam) != WA_INACTIVE)
	{
	  /* Raise the window to the top in Z order */
          /* ago: Activate does not mean putting it to front! */
          /*
	  wmMsg.msg = WM_WM_RAISE;
	  if (fWMMsgInitialized)
	    winSendMessageToWM (s_pScreenPriv->pWMInfo, &wmMsg);
          */
	  
	  /* Tell our Window Manager thread to activate the window */
	  wmMsg.msg = WM_WM_ACTIVATE;
	  if (fWMMsgInitialized)
	    if (!pWin || !pWin->overrideRedirect) /* for OOo menus */
	      winSendMessageToWM (s_pScreenPriv->pWMInfo, &wmMsg);
	}
      return 0;

    case WM_ACTIVATEAPP:
      /*
       * This message is also sent to the root window
       * so we do nothing for individual multiwindow windows
       */
      break;

    case WM_CLOSE:
      /* Branch on if the window was killed in X already */
      if (pWinPriv->fXKilled)
        {
	  /* Window was killed, go ahead and destroy the window */
	  DestroyWindow (hwnd);
	}
      else
	{
	  /* Tell our Window Manager thread to kill the window */
	  wmMsg.msg = WM_WM_KILL;
	  if (fWMMsgInitialized)
	    winSendMessageToWM (s_pScreenPriv->pWMInfo, &wmMsg);
	}
      return 0;

    case WM_DESTROY:

      /* Branch on if the window was killed in X already */
      if (pWinPriv && !pWinPriv->fXKilled)
	{
	  ErrorF ("winTopLevelWindowProc - WM_DESTROY - WM_WM_KILL\n");
	  
	  /* Tell our Window Manager thread to kill the window */
	  wmMsg.msg = WM_WM_KILL;
	  if (fWMMsgInitialized)
	    winSendMessageToWM (s_pScreenPriv->pWMInfo, &wmMsg);
	}

      RemoveProp (hwnd, WIN_WINDOW_PROP);
      RemoveProp (hwnd, WIN_WID_PROP);
      RemoveProp (hwnd, WIN_NEEDMANAGE_PROP);

      break;

    case WM_MOVE:
      /* Adjust the X Window to the moved Windows window */
      winAdjustXWindow (pWin, hwnd);
      return 0;

    case WM_SHOWWINDOW:
      /* Bail out if the window is being hidden */
      if (!wParam)
	return 0;

      /* Tell X to map the window */
      MapWindow (pWin, wClient(pWin));

      /* */
      if (!pWin->overrideRedirect)
	{
	  DWORD		dwExStyle;
	  DWORD		dwStyle;
	  RECT		rcNew;
	  int		iDx, iDy;
	      
	  /* Flag that this window needs to be made active when clicked */
	  SetProp (hwnd, WIN_NEEDMANAGE_PROP, (HANDLE) 1);

	  /* Get the standard and extended window style information */
	  dwExStyle = GetWindowLongPtr (hwnd, GWL_EXSTYLE);
	  dwStyle = GetWindowLongPtr (hwnd, GWL_STYLE);

	  /* */
	  if (dwExStyle != WS_EX_APPWINDOW)
	    {
	      /* Setup a rectangle with the X window position and size */
	      SetRect (&rcNew,
		       pDraw->x,
		       pDraw->y,
		       pDraw->x + pDraw->width,
		       pDraw->y + pDraw->height);

#if 0
	      ErrorF ("winTopLevelWindowProc - (%d, %d)-(%d, %d)\n",
		      rcNew.left, rcNew.top,
		      rcNew.right, rcNew.bottom);
#endif

	      /* */
	      AdjustWindowRectEx (&rcNew,
				  WS_POPUP | WS_SIZEBOX | WS_OVERLAPPEDWINDOW,
				  FALSE,
				  WS_EX_APPWINDOW);

	      /* Calculate position deltas */
	      iDx = pDraw->x - rcNew.left;
	      iDy = pDraw->y - rcNew.top;

	      /* Calculate new rectangle */
	      rcNew.left += iDx;
	      rcNew.right += iDx;
	      rcNew.top += iDy;
	      rcNew.bottom += iDy;

#if 0
	      ErrorF ("winTopLevelWindowProc - (%d, %d)-(%d, %d)\n",
		      rcNew.left, rcNew.top,
		      rcNew.right, rcNew.bottom);
#endif

	      /* Set the window extended style flags */
	      SetWindowLongPtr (hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

	      /* Set the window standard style flags */
	      SetWindowLongPtr (hwnd, GWL_STYLE,
				WS_POPUP | WS_SIZEBOX | WS_OVERLAPPEDWINDOW);

	      /* Position the Windows window */
	      SetWindowPos (hwnd, HWND_TOP,
			    rcNew.left, rcNew.top,
			    rcNew.right - rcNew.left, rcNew.bottom - rcNew.top,
			    SWP_NOMOVE | SWP_FRAMECHANGED
			    | SWP_SHOWWINDOW | SWP_NOACTIVATE);

	      /* Bring the Windows window to the foreground */
	      SetForegroundWindow (hwnd);
	    }
	}
      else /* It is an overridden window so make it top of Z stack */
	{
#if CYGWINDOWING_DEBUG
	  ErrorF ("overridden window is shown\n");
#endif
	  SetWindowPos (hwnd, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	  
      /* Setup the Window Manager message */
      wmMsg.msg = WM_WM_MAP;
      wmMsg.iWidth = pDraw->width;
      wmMsg.iHeight = pDraw->height;

      /* Tell our Window Manager thread to map the window */
      if (fWMMsgInitialized)
	winSendMessageToWM (s_pScreenPriv->pWMInfo, &wmMsg);

      return 0;

    case WM_SIZING:
      /* Need to legalize the size according to WM_NORMAL_HINTS */
      /* for applications like xterm */
      return ValidateSizing (hwnd, pWin, wParam, lParam);

    case WM_WINDOWPOSCHANGED:
      {
	LPWINDOWPOS pWinPos = (LPWINDOWPOS) lParam;

	if (!(pWinPos->flags & SWP_NOZORDER))
	  {
#if CYGWINDOWING_DEBUG
	    winDebug ("\twindow z order was changed\n");
#endif
	    if (pWinPos->hwndInsertAfter == HWND_TOP
		||pWinPos->hwndInsertAfter == HWND_TOPMOST
		||pWinPos->hwndInsertAfter == HWND_NOTOPMOST)
	      {
#if CYGWINDOWING_DEBUG
		winDebug ("\traise to top\n");
#endif
		/* Raise the window to the top in Z order */
		winRaiseWindow(pWin);
	      }
	    else if (pWinPos->hwndInsertAfter == HWND_BOTTOM)
	      {
	      }
	    else
	      {
		/* Check if this window is top of X windows. */
		HWND hWndAbove = NULL;
		DWORD dwCurrentProcessID = GetCurrentProcessId ();
		DWORD dwWindowProcessID = 0;

		for (hWndAbove = pWinPos->hwndInsertAfter;
		     hWndAbove != NULL;
		     hWndAbove = GetNextWindow (hWndAbove, GW_HWNDPREV))
		  {
		    /* Ignore other XWin process's window */
		    GetWindowThreadProcessId (hWndAbove, &dwWindowProcessID);

		    if ((dwWindowProcessID == dwCurrentProcessID)
			&& GetProp (hWndAbove, WIN_WINDOW_PROP)
			&& !IsWindowVisible (hWndAbove)
			&& !IsIconic (hWndAbove) ) /* ignore minimized windows */
		      break;
		  }
		/* If this is top of X windows in Windows stack,
		   raise it in X stack. */
		if (hWndAbove == NULL)
		  {
#if CYGWINDOWING_DEBUG
		    winDebug ("\traise to top\n");
#endif
		    winRaiseWindow(pWin);
		  }
	      }
	  }
      }
      /*
       * Pass the message to DefWindowProc to let the function
       * break down WM_WINDOWPOSCHANGED to WM_MOVE and WM_SIZE.
      */
      break; 

    case WM_SIZE:
      /* see dix/window.c */
#if CYGWINDOWING_DEBUG
      {
	char buf[64];
	switch (wParam)
	  {
	  case SIZE_MINIMIZED:
	    strcpy(buf, "SIZE_MINIMIZED");
	    break;
	  case SIZE_MAXIMIZED:
	    strcpy(buf, "SIZE_MAXIMIZED");
	    break;
	  case SIZE_RESTORED:
	    strcpy(buf, "SIZE_RESTORED");
	    break;
	  default:
	    strcpy(buf, "UNKNOWN_FLAG");
	  }
	ErrorF ("winTopLevelWindowProc - WM_SIZE to %dx%d (%s) - %d ms\n",
		(int)LOWORD(lParam), (int)HIWORD(lParam), buf,
		(int)(GetTickCount ()));
      }
#endif
      /* Adjust the X Window to the moved Windows window */
      winAdjustXWindow (pWin, hwnd);
      return 0; /* end of WM_SIZE handler */

    case WM_MOUSEACTIVATE:

      /* Check if this window needs to be made active when clicked */
      if (!GetProp (pWinPriv->hWnd, WIN_NEEDMANAGE_PROP))
	{
#if CYGMULTIWINDOW_DEBUG
	  ErrorF ("winTopLevelWindowProc - WM_MOUSEACTIVATE - "
		  "MA_NOACTIVATE\n");
#endif

	  /* */
	  return MA_NOACTIVATE;
	}
      break;

    case WM_SETCURSOR:
      if (LOWORD(lParam) == HTCLIENT)
	{
	  if (!g_fSoftwareCursor) SetCursor (s_pScreenPriv->cursor.handle);
	  return TRUE;
	}
      break;

    default:
      break;
    }

  ret = DefWindowProc (hwnd, message, wParam, lParam);
  /*
   * If the window was minized we get the stack change before the window is restored
   * and so it gets lost. Ensure there stacking order is correct.
   */
  if (needRestack)
    winReorderWindowsMultiWindow();
  return ret;
}
