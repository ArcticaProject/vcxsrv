/*
 *Copyright (C) 2003-2004 Harold L Hunt II All Rights Reserved.
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
 * General global variables
 */

int		g_iNumScreens = 0;
winScreenInfo	g_ScreenInfo[MAXSCREENS];
int		g_iLastScreen = -1;
#ifdef HAS_DEVWINDOWS
int		g_fdMessageQueue = WIN_FD_INVALID;
#endif
static int	g_iScreenPrivateKeyIndex;
DevPrivateKey	g_iScreenPrivateKey = &g_iScreenPrivateKeyIndex;
static int	g_iCmapPrivateKeyIndex;
DevPrivateKey	g_iCmapPrivateKey = &g_iCmapPrivateKeyIndex;
static int	g_iGCPrivateKeyIndex;
DevPrivateKey	g_iGCPrivateKey = &g_iGCPrivateKeyIndex;
static int	g_iPixmapPrivateKeyIndex;
DevPrivateKey	g_iPixmapPrivateKey = &g_iPixmapPrivateKeyIndex;
static int	g_iWindowPrivateKeyIndex;
DevPrivateKey	g_iWindowPrivateKey = &g_iWindowPrivateKeyIndex;
unsigned long	g_ulServerGeneration = 0;
Bool		g_fInitializedDefaultScreens = FALSE;
DWORD		g_dwEnginesSupported = 0;
HINSTANCE	g_hInstance = 0;
HWND		g_hDlgDepthChange = NULL;
HWND		g_hDlgExit = NULL;
HWND		g_hDlgAbout = NULL;
const char *	g_pszQueryHost = NULL;
Bool		g_fXdmcpEnabled = FALSE;
HICON		g_hIconX = NULL;
HICON		g_hSmallIconX = NULL;
#ifndef RELOCATE_PROJECTROOT
char *		g_pszLogFile = "/tmp/XWin.log";
#else
char *		g_pszLogFile = "XWin.log";
Bool		g_fLogFileChanged = FALSE;
#endif
int		g_iLogVerbose = 2;
Bool		g_fLogInited = FALSE;
char *		g_pszCommandLine = NULL;
Bool		g_fSilentFatalError = FALSE;
DWORD		g_dwCurrentThreadID = 0;
Bool		g_fKeyboardHookLL = FALSE;
HHOOK		g_hhookKeyboardLL = NULL;
HWND		g_hwndKeyboardFocus = NULL;
Bool		g_fNoHelpMessageBox = FALSE;
Bool		g_fSoftwareCursor = FALSE;
Bool		g_fSilentDupError = FALSE;


/*
 * Global variables for dynamically loaded libraries and
 * their function pointers
 */

HMODULE		g_hmodDirectDraw = NULL;
FARPROC		g_fpDirectDrawCreate = NULL;
FARPROC		g_fpDirectDrawCreateClipper = NULL;

HMODULE		g_hmodCommonControls = NULL;
FARPROC		g_fpTrackMouseEvent = (FARPROC) (void (*)(void))NoopDDA;


#ifdef XWIN_CLIPBOARD
/*
 * Wrapped DIX functions
 */
winDispatchProcPtr	winProcEstablishConnectionOrig = NULL;
winDispatchProcPtr	winProcQueryTreeOrig = NULL;
winDispatchProcPtr	winProcSetSelectionOwnerOrig = NULL;


/*
 * Clipboard variables
 */

Bool			g_fUnicodeClipboard = TRUE;
Bool			g_fClipboard = FALSE;
Bool			g_fClipboardLaunched = FALSE;
Bool			g_fClipboardStarted = FALSE;
pthread_t		g_ptClipboardProc;
HWND			g_hwndClipboard = NULL;
void			*g_pClipboardDisplay = NULL;
Window			g_iClipboardWindow = None;
Atom			g_atomLastOwnedSelection = None;
#endif


/*
 * Re-initialize global variables that are invalidated
 * by a server reset.
 */

void
winInitializeGlobals (void)
{
  g_dwCurrentThreadID = GetCurrentThreadId ();
  g_hwndKeyboardFocus = NULL;
#ifdef XWIN_CLIPBOARD
  g_fClipboardLaunched = FALSE;
  g_fClipboardStarted = FALSE;
  g_iClipboardWindow = None;
  g_pClipboardDisplay = NULL;
  g_atomLastOwnedSelection = None;
  g_hwndClipboard = NULL;
#endif
}
