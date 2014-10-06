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

#ifndef WINCLIPBOARD_INTERNAL_H
#define WINCLIPBOARD_INTERNAL_H

/* Standard library headers */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#ifdef __CYGWIN__
#include <sys/select.h>
#else
#include <X11/Xwinsock.h>
#endif
#include <fcntl.h>
#include <setjmp.h>
#ifdef _MSC_VER
typedef int pid_t;
#endif
#include <pthread.h>

/* X headers */
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

/* Windows headers */
#include <X11/Xwindows.h>

#include "winmsg.h"

#define WIN_XEVENTS_SUCCESS			0
#define WIN_XEVENTS_CONVERT			2
#define WIN_XEVENTS_NOTIFY			3
#define WIN_LOCAL_PROPERTY			"CYGX_CUT_BUFFER"

#define WM_WM_REINIT                           (WM_USER + 200)

/*
 * References to external symbols
 */

extern char *display;
/*
 * winclipboardinit.c
 */

Bool
 winInitClipboard(void);

/*
 * winclipboardtextconv.c
 */

void
 winClipboardDOStoUNIX(char *pszData, int iLength);

void
 winClipboardUNIXtoDOS(char **ppszData, int iLength);

/*
 * winclipboardthread.c
 */


/*
 * winclipboardwndproc.c
 */

Bool winClipboardFlushWindowsMessageQueue(HWND hwnd);

LRESULT CALLBACK
winClipboardWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * winclipboardxevents.c
 */

int

winClipboardFlushXEvents(HWND hwnd,
                         int iWindow, Display * pDisplay, Bool fUnicodeSupport, Bool ClipboardOpened);
#endif
