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

/*
 * External global variables
 */

extern HWND			g_hDlgDepthChange;

extern void winSelectIcons(WindowPtr pWin, HICON *pIcon, HICON *pSmallIcon);

/*
 * Prototypes for local functions
 */

void
winCreateWindowsWindow (WindowPtr pWin);

static void
winDestroyWindowsWindow (WindowPtr pWin);

static void
winUpdateWindowsWindow (WindowPtr pWin);

static void
winFindWindow (pointer value, XID id, pointer cdata);

/*
 * Constant defines
 */

#define MOUSE_POLLING_INTERVAL		500


/*
 * Macros
 */

#define SubSend(pWin) \
    ((pWin->eventMask|wOtherEventMasks(pWin)) & SubstructureNotifyMask)

#define StrSend(pWin) \
    ((pWin->eventMask|wOtherEventMasks(pWin)) & StructureNotifyMask)

#define SubStrSend(pWin,pParent) (StrSend(pWin) || SubSend(pParent))


/*
 * CreateWindow - See Porting Layer Definition - p. 37
 */

Bool
winCreateWindowMultiWindow (WindowPtr pWin)
{
  Bool			fResult = TRUE;
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGMULTIWINDOW_DEBUG
  winTrace ("winCreateWindowMultiWindow - pWin: %p\n", pWin);
#endif
  
  WIN_UNWRAP(CreateWindow);
  fResult = (*pScreen->CreateWindow) (pWin);
  WIN_WRAP(CreateWindow, winCreateWindowMultiWindow);
  
  /* Initialize some privates values */
  pWinPriv->hRgn = NULL;
  pWinPriv->hWnd = NULL;
  pWinPriv->pScreenPriv = winGetScreenPriv(pWin->drawable.pScreen);
  pWinPriv->fXKilled = FALSE;
 
  return fResult;
}


/*
 * DestroyWindow - See Porting Layer Definition - p. 37
 */

Bool
winDestroyWindowMultiWindow (WindowPtr pWin)
{
  Bool			fResult = TRUE;
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winDestroyWindowMultiWindow - pWin: %p\n", pWin);
#endif
  
  WIN_UNWRAP(DestroyWindow); 
  fResult = (*pScreen->DestroyWindow)(pWin);
  WIN_WRAP(DestroyWindow, winDestroyWindowMultiWindow);
  
  /* Flag that the window has been destroyed */
  pWinPriv->fXKilled = TRUE;
  
  /* Kill the MS Windows window associated with this window */
  winDestroyWindowsWindow (pWin); 

  return fResult;
}


/*
 * PositionWindow - See Porting Layer Definition - p. 37
 *
 * This function adjusts the position and size of Windows window
 * with respect to the underlying X window.  This is the inverse
 * of winAdjustXWindow, which adjusts X window to Windows window.
 */

Bool
winPositionWindowMultiWindow (WindowPtr pWin, int x, int y)
{
  Bool			fResult = TRUE;
  int		        iX, iY, iWidth, iHeight;
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

  HWND hWnd = pWinPriv->hWnd;
  RECT rcNew;
  RECT rcOld;
#if CYGMULTIWINDOW_DEBUG
  RECT rcClient;
  RECT *lpRc;
#endif
  DWORD dwExStyle;
  DWORD dwStyle;

#if CYGMULTIWINDOW_DEBUG
  winTrace ("winPositionWindowMultiWindow - pWin: %p\n", pWin);
#endif
  
  WIN_UNWRAP(PositionWindow);
  fResult = (*pScreen->PositionWindow)(pWin, x, y);
  WIN_WRAP(PositionWindow, winPositionWindowMultiWindow);
  
#if CYGWINDOWING_DEBUG
  ErrorF ("winPositionWindowMultiWindow: (x, y) = (%d, %d)\n",
	  x, y);
#endif

  /* Bail out if the Windows window handle is bad */
  if (!hWnd)
    {
#if CYGWINDOWING_DEBUG
      ErrorF ("\timmediately return since hWnd is NULL\n");
#endif
      return fResult;
    }

  /* Get the Windows window style and extended style */
  dwExStyle = GetWindowLongPtr (hWnd, GWL_EXSTYLE);
  dwStyle = GetWindowLongPtr (hWnd, GWL_STYLE);

  /* Get the X and Y location of the X window */
  iX = pWin->drawable.x + GetSystemMetrics (SM_XVIRTUALSCREEN);
  iY = pWin->drawable.y + GetSystemMetrics (SM_YVIRTUALSCREEN);

  /* Get the height and width of the X window */
  iWidth = pWin->drawable.width;
  iHeight = pWin->drawable.height;

  /* Store the origin, height, and width in a rectangle structure */
  SetRect (&rcNew, iX, iY, iX + iWidth, iY + iHeight);

#if CYGMULTIWINDOW_DEBUG
  lpRc = &rcNew;
  ErrorF ("winPositionWindowMultiWindow - (%d ms)drawable (%d, %d)-(%d, %d)\n",
	  GetTickCount (), lpRc->left, lpRc->top, lpRc->right, lpRc->bottom);
#endif

  /*
   * Calculate the required size of the Windows window rectangle,
   * given the size of the Windows window client area.
   */
  AdjustWindowRectEx (&rcNew, dwStyle, FALSE, dwExStyle);

  /* Get a rectangle describing the old Windows window */
  GetWindowRect (hWnd, &rcOld);

#if CYGMULTIWINDOW_DEBUG
  /* Get a rectangle describing the Windows window client area */
  GetClientRect (hWnd, &rcClient);

  lpRc = &rcNew;
  ErrorF ("winPositionWindowMultiWindow - (%d ms)rcNew (%d, %d)-(%d, %d)\n",
	  GetTickCount (), lpRc->left, lpRc->top, lpRc->right, lpRc->bottom);
      
  lpRc = &rcOld;
  ErrorF ("winPositionWindowMultiWindow - (%d ms)rcOld (%d, %d)-(%d, %d)\n",
	  GetTickCount (), lpRc->left, lpRc->top, lpRc->right, lpRc->bottom);
      
  lpRc = &rcClient;
  ErrorF ("(%d ms)rcClient (%d, %d)-(%d, %d)\n",
	  GetTickCount (), lpRc->left, lpRc->top, lpRc->right, lpRc->bottom);
#endif

  /* Check if the old rectangle and new rectangle are the same */
  if (!EqualRect (&rcNew, &rcOld))
    {
#if CYGMULTIWINDOW_DEBUG
      ErrorF ("winPositionWindowMultiWindow - Need to move\n");
#endif

#if CYGWINDOWING_DEBUG
      ErrorF ("\tMoveWindow to (%ld, %ld) - %ldx%ld\n", rcNew.left, rcNew.top,
	      rcNew.right - rcNew.left, rcNew.bottom - rcNew.top);
#endif
      /* Change the position and dimensions of the Windows window */
      MoveWindow (hWnd,
		  rcNew.left, rcNew.top,
		  rcNew.right - rcNew.left, rcNew.bottom - rcNew.top,
		  TRUE);
    }
  else
    {
#if CYGMULTIWINDOW_DEBUG
      ErrorF ("winPositionWindowMultiWindow - Not need to move\n");
#endif
    }

  return fResult;
}


/*
 * ChangeWindowAttributes - See Porting Layer Definition - p. 37
 */

Bool
winChangeWindowAttributesMultiWindow (WindowPtr pWin, unsigned long mask)
{
  Bool			fResult = TRUE;
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winChangeWindowAttributesMultiWindow - pWin: %08x\n", pWin);
#endif
  
  WIN_UNWRAP(ChangeWindowAttributes); 
  fResult = (*pScreen->ChangeWindowAttributes)(pWin, mask);
  WIN_WRAP(ChangeWindowAttributes, winChangeWindowAttributesMultiWindow);
  
  /*
   * NOTE: We do not currently need to do anything here.
   */

  return fResult;
}


/*
 * UnmapWindow - See Porting Layer Definition - p. 37
 * Also referred to as UnrealizeWindow
 */

Bool
winUnmapWindowMultiWindow (WindowPtr pWin)
{
  Bool			fResult = TRUE;
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winUnmapWindowMultiWindow - pWin: %08x\n", pWin);
#endif
  
  WIN_UNWRAP(UnrealizeWindow); 
  fResult = (*pScreen->UnrealizeWindow)(pWin);
  WIN_WRAP(UnrealizeWindow, winUnmapWindowMultiWindow);
  
  /* Flag that the window has been killed */
  pWinPriv->fXKilled = TRUE;
 
  /* Destroy the Windows window associated with this X window */
  winDestroyWindowsWindow (pWin);

  return fResult;
}


/*
 * MapWindow - See Porting Layer Definition - p. 37
 * Also referred to as RealizeWindow
 */

Bool
winMapWindowMultiWindow (WindowPtr pWin)
{
  Bool			fResult = TRUE;
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winMapWindowMultiWindow - pWin: %08x\n", pWin);
#endif
  
  WIN_UNWRAP(RealizeWindow); 
  fResult = (*pScreen->RealizeWindow)(pWin);
  WIN_WRAP(RealizeWindow, winMapWindowMultiWindow);
  
  /* Flag that this window has not been destroyed */
  pWinPriv->fXKilled = FALSE;

  /* Refresh/redisplay the Windows window associated with this X window */
  winUpdateWindowsWindow (pWin);

  /* Update the Windows window's shape */
  winReshapeMultiWindow (pWin);
  winUpdateRgnMultiWindow (pWin);

  return fResult;
}


/*
 * ReparentWindow - See Porting Layer Definition - p. 42
 */

void
winReparentWindowMultiWindow (WindowPtr pWin, WindowPtr pPriorParent)
{
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winReparentMultiWindow - pWin: %08x\n", pWin);
#endif

  WIN_UNWRAP(ReparentWindow);
  if (pScreen->ReparentWindow) 
    (*pScreen->ReparentWindow)(pWin, pPriorParent);
  WIN_WRAP(ReparentWindow, winReparentWindowMultiWindow);
  
  /* Update the Windows window associated with this X window */
  winUpdateWindowsWindow (pWin);
}


/*
 * RestackWindow - Shuffle the z-order of a window
 */

void
winRestackWindowMultiWindow (WindowPtr pWin, WindowPtr pOldNextSib)
{
  WindowPtr		pPrevWin;
  UINT			uFlags;
  HWND			hInsertAfter;
  HWND                  hWnd = NULL;
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGMULTIWINDOW_DEBUG || CYGWINDOWING_DEBUG
  winTrace ("winRestackMultiWindow - %08x\n", pWin);
#endif
  
   WIN_UNWRAP(RestackWindow);
   if (pScreen->RestackWindow) 
     (*pScreen->RestackWindow)(pWin, pOldNextSib);
   WIN_WRAP(RestackWindow, winRestackWindowMultiWindow);
  
#if 1
  /*
   * Calling winReorderWindowsMultiWindow here means our window manager
   * (i.e. Windows Explorer) has initiative to determine Z order.
   */
  if (pWin->nextSib != pOldNextSib)
    winReorderWindowsMultiWindow ();
#else
  /* Bail out if no window privates or window handle is invalid */
  if (!pWinPriv || !pWinPriv->hWnd)
    return;

  /* Get a pointer to our previous sibling window */
  pPrevWin = pWin->prevSib;

  /*
   * Look for a sibling window with
   * valid privates and window handle
   */
  while (pPrevWin
	 && !winGetWindowPriv(pPrevWin)
	 && !winGetWindowPriv(pPrevWin)->hWnd)
    pPrevWin = pPrevWin->prevSib;
      
  /* Check if we found a valid sibling */
  if (pPrevWin)
    {
      /* Valid sibling - get handle to insert window after */
      hInsertAfter = winGetWindowPriv(pPrevWin)->hWnd;
      uFlags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE;
  
      hWnd = GetNextWindow (pWinPriv->hWnd, GW_HWNDPREV);

      do
	{
	  if (GetProp (hWnd, WIN_WINDOW_PROP))
	    {
	      if (hWnd == winGetWindowPriv(pPrevWin)->hWnd)
		{
		  uFlags |= SWP_NOZORDER;
		}
	      break;
	    }
	  hWnd = GetNextWindow (hWnd, GW_HWNDPREV);
	}
      while (hWnd);
    }
  else
    {
      /* No valid sibling - make this window the top window */
      hInsertAfter = HWND_TOP;
      uFlags = SWP_NOMOVE | SWP_NOSIZE;
    }
      
  /* Perform the restacking operation in Windows */
  SetWindowPos (pWinPriv->hWnd,
		hInsertAfter,
		0, 0,
		0, 0,
		uFlags);
#endif
}


/*
 * winCreateWindowsWindow - Create a Windows window associated with an X window
 */

void
winCreateWindowsWindow (WindowPtr pWin)
{
  int                   iX, iY;
  int			iWidth;
  int			iHeight;
  HWND			hWnd;
  WNDCLASSEX		wc;
  winWindowPriv(pWin);
  HICON			hIcon;
  HICON			hIconSmall;
#define CLASS_NAME_LENGTH 512
  char                  pszClass[CLASS_NAME_LENGTH], pszWindowID[12];
  char                  *res_name, *res_class, *res_role;
  static int		s_iWindowID = 0;
  winPrivScreenPtr	pScreenPriv = pWinPriv->pScreenPriv;
  WinXSizeHints         hints;

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winCreateWindowsWindow - pWin: %08x\n", pWin);
#endif

  iX = pWin->drawable.x + GetSystemMetrics (SM_XVIRTUALSCREEN);
  iY = pWin->drawable.y + GetSystemMetrics (SM_YVIRTUALSCREEN);

  /* Default positions if none specified */
  if (!winMultiWindowGetWMNormalHints(pWin, &hints))
    hints.flags = 0;
  if ( !(hints.flags & (USPosition|PPosition)) &&
       !winMultiWindowGetTransientFor (pWin, NULL) &&
       !pWin->overrideRedirect )
    {
      iX = CW_USEDEFAULT;
      iY = CW_USEDEFAULT;
    }

  iWidth = pWin->drawable.width;
  iHeight = pWin->drawable.height;

  winSelectIcons(pWin, &hIcon, &hIconSmall); 

  /* Set standard class name prefix so we can identify window easily */
  strncpy (pszClass, WINDOW_CLASS_X, sizeof(pszClass));

  if (winMultiWindowGetClassHint (pWin, &res_name, &res_class))
    {
      strncat (pszClass, "-", 1);
      strncat (pszClass, res_name, CLASS_NAME_LENGTH - strlen (pszClass));
      strncat (pszClass, "-", 1);
      strncat (pszClass, res_class, CLASS_NAME_LENGTH - strlen (pszClass));
      
      /* Check if a window class is provided by the WM_WINDOW_ROLE property,
       * if not use the WM_CLASS information.
       * For further information see:
       * http://tronche.com/gui/x/icccm/sec-5.html
       */ 
      if (winMultiWindowGetWindowRole (pWin, &res_role) )
	{
	  strcat (pszClass, "-");
	  strcat (pszClass, res_role);
	  free (res_role);
	}

      free (res_name);
      free (res_class);
    }

  /* Add incrementing window ID to make unique class name */
  snprintf (pszWindowID, sizeof(pszWindowID), "-%x", s_iWindowID++);
  pszWindowID[sizeof(pszWindowID)-1] = 0;
  strcat (pszClass, pszWindowID);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winCreateWindowsWindow - Creating class: %s\n", pszClass);
#endif

  /* Setup our window class */
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = winTopLevelWindowProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = g_hInstance;
  wc.hIcon = hIcon;
  wc.hIconSm = hIconSmall;
  wc.hCursor = 0;
  wc.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = pszClass;
  RegisterClassEx (&wc);

  /* Create the window */
  /* Make it OVERLAPPED in create call since WS_POPUP doesn't support */
  /* CW_USEDEFAULT, change back to popup after creation */
  hWnd = CreateWindowExA (WS_EX_TOOLWINDOW,	/* Extended styles */
			  pszClass,		/* Class name */
			  WINDOW_TITLE_X,	/* Window name */
			  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			  iX,			/* Horizontal position */
			  iY,			/* Vertical position */
			  iWidth,		/* Right edge */ 
			  iHeight,		/* Bottom edge */
			  (HWND) NULL,		/* No parent or owner window */
			  (HMENU) NULL,		/* No menu */
			  GetModuleHandle (NULL), /* Instance handle */
			  pWin);		/* ScreenPrivates */
  if (hWnd == NULL)
    {
      ErrorF ("winCreateWindowsWindow - CreateWindowExA () failed: %d\n",
	      (int) GetLastError ());
    }
 
  /* Change style back to popup, already placed... */
  SetWindowLong (hWnd, GWL_STYLE, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
  SetWindowPos (hWnd, 0, 0, 0, 0, 0,
		SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  /* Make sure it gets the proper system menu for a WS_POPUP, too */
  GetSystemMenu (hWnd, TRUE);

  pWinPriv->hWnd = hWnd;

  /* Cause any .XWinrc menus to be added in main WNDPROC */
  PostMessage (hWnd, WM_INIT_SYS_MENU, 0, 0);
  
  SetProp (pWinPriv->hWnd, WIN_WID_PROP, (HANDLE) winGetWindowID(pWin));

  /* Flag that this Windows window handles its own activation */
  SetProp (pWinPriv->hWnd, WIN_NEEDMANAGE_PROP, (HANDLE) 0);

  /* Call engine-specific create window procedure */
  (*pScreenPriv->pwinFinishCreateWindowsWindow) (pWin);
}


Bool winInDestroyWindowsWindow = FALSE;
/*
 * winDestroyWindowsWindow - Destroy a Windows window associated
 * with an X window
 */
static void
winDestroyWindowsWindow (WindowPtr pWin)
{
  MSG			msg;
  winWindowPriv(pWin);
  HICON			hiconClass;
  HICON			hiconSmClass;
  HMODULE		hInstance;
  int			iReturn;
  char			pszClass[512];
  BOOL			oldstate = winInDestroyWindowsWindow;
  
#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winDestroyWindowsWindow\n");
#endif

  /* Bail out if the Windows window handle is invalid */
  if (pWinPriv->hWnd == NULL)
    return;

  winInDestroyWindowsWindow = TRUE;

  /* Store the info we need to destroy after this window is gone */
  hInstance = (HINSTANCE) GetClassLong (pWinPriv->hWnd, GCL_HMODULE);
  hiconClass = (HICON) GetClassLong (pWinPriv->hWnd, GCL_HICON);
  hiconSmClass = (HICON) GetClassLong (pWinPriv->hWnd, GCL_HICONSM);
  iReturn = GetClassName (pWinPriv->hWnd, pszClass, 512);
  
  SetProp (pWinPriv->hWnd, WIN_WINDOW_PROP, NULL);
  /* Destroy the Windows window */
  DestroyWindow (pWinPriv->hWnd);

  /* Null our handle to the Window so referencing it will cause an error */
  pWinPriv->hWnd = NULL;

  /* Process all messages on our queue */
  while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
    {
      if (g_hDlgDepthChange == 0 || !IsDialogMessage (g_hDlgDepthChange, &msg))
	{
	  DispatchMessage (&msg);
	}
    }

  /* Only if we were able to get the name */
  if (iReturn)
    { 
#if CYGMULTIWINDOW_DEBUG
      ErrorF ("winDestroyWindowsWindow - Unregistering %s: ", pszClass);
#endif
      iReturn = UnregisterClass (pszClass, hInstance);
      
#if CYGMULTIWINDOW_DEBUG
      ErrorF ("winDestroyWindowsWindow - %d Deleting Icon: ", iReturn);
#endif
      
      winDestroyIcon(hiconClass);
      winDestroyIcon(hiconSmClass);
    }

  winInDestroyWindowsWindow = oldstate;

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("-winDestroyWindowsWindow\n");
#endif
}


/*
 * winUpdateWindowsWindow - Redisplay/redraw a Windows window
 * associated with an X window
 */

static void
winUpdateWindowsWindow (WindowPtr pWin)
{
  winWindowPriv(pWin);
  HWND			hWnd = pWinPriv->hWnd;

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winUpdateWindowsWindow\n");
#endif

  /* Check if the Windows window's parents have been destroyed */
  if (pWin->parent != NULL
      && pWin->parent->parent == NULL
      && pWin->mapped)
    {
      /* Create the Windows window if it has been destroyed */
      if (hWnd == NULL)
	{
	  winCreateWindowsWindow (pWin);
	  assert (pWinPriv->hWnd != NULL);
	}

      /* Display the window without activating it */
      ShowWindow (pWinPriv->hWnd, SW_SHOWNOACTIVATE);

      /* Send first paint message */
      UpdateWindow (pWinPriv->hWnd);
    }
  else if (hWnd != NULL)
    {
      /* Destroy the Windows window if its parents are destroyed */
      winDestroyWindowsWindow (pWin);
      assert (pWinPriv->hWnd == NULL);
    }

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("-winUpdateWindowsWindow\n");
#endif
}


/*
 * winGetWindowID - 
 */

XID
winGetWindowID (WindowPtr pWin)
{
  WindowIDPairRec	wi = {pWin, 0};
  ClientPtr		c = wClient(pWin);
  
  /* */
  FindClientResourcesByType (c, RT_WINDOW, winFindWindow, &wi);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winGetWindowID - Window ID: %d\n", wi.id);
#endif

  return wi.id;
}


/*
 * winFindWindow - 
 */

static void
winFindWindow (pointer value, XID id, pointer cdata)
{
  WindowIDPairPtr	wi = (WindowIDPairPtr)cdata;

  if (value == wi->value)
    {
      wi->id = id;
    }
}


/*
 * winReorderWindowsMultiWindow - 
 */

void
winReorderWindowsMultiWindow (void)
{
  HWND hwnd = NULL;
  WindowPtr pWin = NULL;
  WindowPtr pWinSib = NULL;
  XID vlist[2];
  static Bool fRestacking = FALSE; /* Avoid recusive calls to this function */
  DWORD dwCurrentProcessID = GetCurrentProcessId ();
  DWORD dwWindowProcessID = 0;

#if CYGMULTIWINDOW_DEBUG || CYGWINDOWING_DEBUG
  winTrace ("winReorderWindowsMultiWindow\n");
#endif

  if (fRestacking)
    {
      /* It is a recusive call so immediately exit */
#if CYGWINDOWING_DEBUG
      ErrorF ("winReorderWindowsMultiWindow - "
	      "exit because fRestacking == TRUE\n");
#endif
      return;
    }
  fRestacking = TRUE;

  /* Loop through top level Window windows, descending in Z order */
  for ( hwnd = GetTopWindow (NULL);
	hwnd;
	hwnd = GetNextWindow (hwnd, GW_HWNDNEXT) )
    {
      /* Don't take care of other Cygwin/X process's windows */
      GetWindowThreadProcessId (hwnd, &dwWindowProcessID);

      if ( GetProp (hwnd, WIN_WINDOW_PROP)
	   && (dwWindowProcessID == dwCurrentProcessID)
	   && !IsIconic (hwnd) ) /* ignore minimized windows */
	{
	  pWinSib = pWin;
	  pWin = GetProp (hwnd, WIN_WINDOW_PROP);
	      
	  if (!pWinSib)
	    { /* 1st window - raise to the top */
	      vlist[0] = Above;
		  
	      ConfigureWindow (pWin, CWStackMode, vlist, wClient(pWin));
	    }
	  else
	    { /* 2nd or deeper windows - just below the previous one */
	      vlist[0] = winGetWindowID (pWinSib);
	      vlist[1] = Below;

	      ConfigureWindow (pWin, CWSibling | CWStackMode,
			       vlist, wClient(pWin));
	    }
	}
    }

  fRestacking = FALSE;
}


/*
 * winMinimizeWindow - Minimize in response to WM_CHANGE_STATE
 */

void
winMinimizeWindow (Window id)
{
  WindowPtr		pWin;
  winPrivWinPtr	pWinPriv;
#ifdef XWIN_MULTIWINDOWEXTWM
  win32RootlessWindowPtr pRLWinPriv;
#endif
  HWND hWnd;
  ScreenPtr pScreen = NULL;
  winPrivScreenPtr pScreenPriv = NULL;
  winScreenInfo *pScreenInfo = NULL;

#if CYGWINDOWING_DEBUG
  ErrorF ("winMinimizeWindow\n");
#endif

  pWin = LookupIDByType (id, RT_WINDOW);
  if (!pWin) 
  { 
      ErrorF("%s: NULL pWin. Leaving\n", __FUNCTION__); 
      return; 
  }

  pScreen = pWin->drawable.pScreen;
  if (pScreen) pScreenPriv = winGetScreenPriv(pScreen);
  if (pScreenPriv) pScreenInfo = pScreenPriv->pScreenInfo;

#ifdef XWIN_MULTIWINDOWEXTWM
  if (pScreenPriv && pScreenInfo->fInternalWM)
    {
      pRLWinPriv  = (win32RootlessWindowPtr) RootlessFrameForWindow (pWin, FALSE);
      hWnd = pRLWinPriv->hWnd;
    }
  else
#else
  if (pScreenPriv)
#endif
    {
      pWinPriv = winGetWindowPriv (pWin);
      hWnd = pWinPriv->hWnd;
    }

  ShowWindow (hWnd, SW_MINIMIZE);
}


/*
 * CopyWindow - See Porting Layer Definition - p. 39
 */
void
winCopyWindowMultiWindow (WindowPtr pWin, DDXPointRec oldpt,
			  RegionPtr oldRegion)
{
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGWINDOWING_DEBUG
  ErrorF ("CopyWindowMultiWindow\n");
#endif
  WIN_UNWRAP(CopyWindow); 
  (*pScreen->CopyWindow)(pWin, oldpt, oldRegion);
  WIN_WRAP(CopyWindow, winCopyWindowMultiWindow);
}


/*
 * MoveWindow - See Porting Layer Definition - p. 42
 */
void
winMoveWindowMultiWindow (WindowPtr pWin, int x, int y,
			  WindowPtr pSib, VTKind kind)
{
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGWINDOWING_DEBUG
  ErrorF ("MoveWindowMultiWindow to (%d, %d)\n", x, y);
#endif

  WIN_UNWRAP(MoveWindow); 
  (*pScreen->MoveWindow)(pWin, x, y, pSib, kind);
  WIN_WRAP(MoveWindow, winMoveWindowMultiWindow);
}


/*
 * ResizeWindow - See Porting Layer Definition - p. 42
 */
void
winResizeWindowMultiWindow (WindowPtr pWin, int x, int y, unsigned int w,
			    unsigned int h, WindowPtr pSib)
{
  ScreenPtr		pScreen = pWin->drawable.pScreen;
  winWindowPriv(pWin);
  winScreenPriv(pScreen);

#if CYGWINDOWING_DEBUG
  ErrorF ("ResizeWindowMultiWindow to (%d, %d) - %dx%d\n", x, y, w, h);
#endif
  WIN_UNWRAP(ResizeWindow); 
  (*pScreen->ResizeWindow)(pWin, x, y, w, h, pSib);
  WIN_WRAP(ResizeWindow, winResizeWindowMultiWindow);
}


/*
 * winAdjustXWindow
 *
 * Move and resize X window with respect to corresponding Windows window.
 * This is called from WM_MOVE/WM_SIZE handlers when the user performs
 * any windowing operation (move, resize, minimize, maximize, restore).
 *
 * The functionality is the inverse of winPositionWindowMultiWindow, which
 * adjusts Windows window with respect to X window.
 */
int
winAdjustXWindow (WindowPtr pWin, HWND hwnd)
{
  RECT rcDraw; /* Rect made from pWin->drawable to be adjusted */
  RECT rcWin;  /* The source: WindowRect from hwnd */
  DrawablePtr pDraw;
  XID vlist[4];
  LONG dX, dY, dW, dH, x, y;
  DWORD dwStyle, dwExStyle;

#define WIDTH(rc) (rc.right - rc.left)
#define HEIGHT(rc) (rc.bottom - rc.top)
  
#if CYGWINDOWING_DEBUG
  ErrorF ("winAdjustXWindow\n");
#endif

  if (IsIconic (hwnd))
    {
#if CYGWINDOWING_DEBUG
      ErrorF ("\timmediately return because the window is iconized\n");
#endif
      /*
       * If the Windows window is minimized, its WindowRect has
       * meaningless values so we don't adjust X window to it.
       */
      vlist[0] = 0;
      vlist[1] = 0;
      return ConfigureWindow (pWin, CWX | CWY, vlist, wClient(pWin));
    }
  
  pDraw = &pWin->drawable;

  /* Calculate the window rect from the drawable */
  x = pDraw->x + GetSystemMetrics (SM_XVIRTUALSCREEN);
  y = pDraw->y + GetSystemMetrics (SM_YVIRTUALSCREEN);
  SetRect (&rcDraw, x, y, x + pDraw->width, y + pDraw->height);
#ifdef CYGMULTIWINDOW_DEBUG
          winDebug("\tDrawable extend {%d, %d, %d, %d}, {%d, %d}\n", 
              rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom,
              rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top);
#endif
  dwExStyle = GetWindowLongPtr (hwnd, GWL_EXSTYLE);
  dwStyle = GetWindowLongPtr (hwnd, GWL_STYLE);
#ifdef CYGMULTIWINDOW_DEBUG
          winDebug("\tWindowStyle: %08x %08x\n", dwStyle, dwExStyle);
#endif
  AdjustWindowRectEx (&rcDraw, dwStyle, FALSE, dwExStyle);

  /* The source of adjust */
  GetWindowRect (hwnd, &rcWin);
#ifdef CYGMULTIWINDOW_DEBUG
          winDebug("\tWindow extend {%d, %d, %d, %d}, {%d, %d}\n", 
              rcWin.left, rcWin.top, rcWin.right, rcWin.bottom,
              rcWin.right - rcWin.left, rcWin.bottom - rcWin.top);
          winDebug("\tDraw extend {%d, %d, %d, %d}, {%d, %d}\n", 
              rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom,
              rcDraw.right - rcDraw.left, rcDraw.bottom - rcDraw.top);
#endif

  if (EqualRect (&rcDraw, &rcWin)) {
    /* Bail if no adjust is needed */
#if CYGWINDOWING_DEBUG
    ErrorF ("\treturn because already adjusted\n");
#endif
    return 0;
  }
  
  /* Calculate delta values */
  dX = rcWin.left - rcDraw.left;
  dY = rcWin.top - rcDraw.top;
  dW = WIDTH(rcWin) - WIDTH(rcDraw);
  dH = HEIGHT(rcWin) - HEIGHT(rcDraw);

  /*
   * Adjust.
   * We may only need to move (vlist[0] and [1]), or only resize
   * ([2] and [3]) but currently we set all the parameters and leave
   * the decision to ConfigureWindow.  The reason is code simplicity.
  */
  vlist[0] = pDraw->x + dX - wBorderWidth(pWin);
  vlist[1] = pDraw->y + dY - wBorderWidth(pWin);
  vlist[2] = pDraw->width + dW;
  vlist[3] = pDraw->height + dH;
#if CYGWINDOWING_DEBUG
  ErrorF ("\tConfigureWindow to (%ld, %ld) - %ldx%ld\n", vlist[0], vlist[1],
	  vlist[2], vlist[3]);
#endif
  return ConfigureWindow (pWin, CWX | CWY | CWWidth | CWHeight,
			  vlist, wClient(pWin));
  
#undef WIDTH
#undef HEIGHT
}

