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
#include <sys/types.h>
#include "winclipboard.h"
#ifdef __CYGWIN__
#include <errno.h>
#endif
#include "X11/Xauth.h"


/*
 * Constants
 */

#define AUTH_NAME	"MIT-MAGIC-COOKIE-1"


/*
 * References to external symbols
 */

extern Bool		g_fUnicodeClipboard;
extern unsigned long	serverGeneration;
#if defined(XCSECURITY)
extern unsigned int	g_uiAuthDataLen;
extern char		*g_pAuthData;
#endif
extern Bool		g_fClipboardStarted;
extern HWND		g_hwndClipboard;
extern void		*g_pClipboardDisplay;
extern Window		g_iClipboardWindow;


/*
 * Global variables
 */

static jmp_buf			g_jmpEntry;
Bool				g_fUnicodeSupport = FALSE;
Bool				g_fUseUnicode = FALSE;


/*
 * Local function prototypes
 */

static int
winClipboardErrorHandler (Display *pDisplay, XErrorEvent *pErr);

static int
winClipboardIOErrorHandler (Display *pDisplay);


/*
 * Main thread function
 */

void *
winClipboardProc (void *pvNotUsed)
{
  Atom			atomClipboard, atomClipboardManager;
  int			iReturn;
  HWND			hwnd = NULL;
  int			iConnectionNumber = 0;
#ifdef HAS_DEVWINDOWS
  int			fdMessageQueue = 0;
#else
  struct timeval        tvTimeout;
#endif
  fd_set		fdsRead;
  int			iMaxDescriptor;
  Display		*pDisplay = NULL;
  Window		iWindow = None;
  int			iRetries;
  Bool			fUseUnicode;
  char			szDisplay[512];
  int			iSelectError;

  ErrorF ("winClipboardProc - Hello\n");

  /* Do we have Unicode support? */
  g_fUnicodeSupport = winClipboardDetectUnicodeSupport ();

  /* Do we use Unicode clipboard? */
  fUseUnicode = g_fUnicodeClipboard && g_fUnicodeSupport;

  /* Save the Unicode support flag in a global */
  g_fUseUnicode = fUseUnicode;

  /* Allow multiple threads to access Xlib */
  if (XInitThreads () == 0)
    {
      ErrorF ("winClipboardProc - XInitThreads failed.\n");
      pthread_exit (NULL);
    }

  /* See if X supports the current locale */
  if (XSupportsLocale () == False)
    {
      ErrorF ("winClipboardProc - Locale not supported by X.  Exiting.\n");
      pthread_exit (NULL);
    }

  /* Set jump point for Error exits */
  iReturn = setjmp (g_jmpEntry);
  
  /* Check if we should continue operations */
  if (iReturn != WIN_JMP_ERROR_IO
      && iReturn != WIN_JMP_OKAY)
    {
      /* setjmp returned an unknown value, exit */
      ErrorF ("winClipboardProc - setjmp returned: %d exiting\n",
	      iReturn);
      pthread_exit (NULL);
    }
  else if (iReturn == WIN_JMP_ERROR_IO)
    {
      /* TODO: Cleanup the Win32 window and free any allocated memory */
      ErrorF ("winClipboardProc - setjmp returned for IO Error Handler.\n");
      pthread_exit (NULL);
    }

#if defined(XCSECURITY)
  /* Use our generated cookie for authentication */
  XSetAuthorization (AUTH_NAME,
		     strlen (AUTH_NAME),
		     g_pAuthData,
		     g_uiAuthDataLen);
#endif

  /* Set error handler */
  XSetErrorHandler (winClipboardErrorHandler);
  XSetIOErrorHandler (winClipboardIOErrorHandler);

  /* Initialize retry count */
  iRetries = 0;

  /* Setup the display connection string x */
  /*
   * NOTE: Always connect to screen 0 since we require that screen
   * numbers start at 0 and increase without gaps.  We only need
   * to connect to one screen on the display to get events
   * for all screens on the display.  That is why there is only
   * one clipboard client thread.
   */
  snprintf (szDisplay,
	    512,
	    "127.0.0.1:%s.0",
	    display);

  /* Print the display connection string */
  ErrorF ("winClipboardProc - DISPLAY=%s\n", szDisplay);

  /* Open the X display */
  do
    {
      pDisplay = XOpenDisplay (szDisplay);
      if (pDisplay == NULL)
	{
	  ErrorF ("winClipboardProc - Could not open display, "
		  "try: %d, sleeping: %d\n",
		  iRetries + 1, WIN_CONNECT_DELAY);
	  ++iRetries;
	  sleep (WIN_CONNECT_DELAY);
	  continue;
	}
      else
	break;
    }
  while (pDisplay == NULL && iRetries < WIN_CONNECT_RETRIES);

  /* Make sure that the display opened */
  if (pDisplay == NULL)
    {
      ErrorF ("winClipboardProc - Failed opening the display, giving up\n");
      pthread_exit (NULL);
    }

  /* Save the display in the screen privates */
  g_pClipboardDisplay = pDisplay;

  ErrorF ("winClipboardProc - XOpenDisplay () returned and "
	  "successfully opened the display.\n");

  /* Get our connection number */
  iConnectionNumber = ConnectionNumber (pDisplay);

#ifdef HAS_DEVWINDOWS
  /* Open a file descriptor for the windows message queue */
  fdMessageQueue = open (WIN_MSG_QUEUE_FNAME, O_RDONLY);
  if (fdMessageQueue == -1)
    {
      ErrorF ("winClipboardProc - Failed opening %s\n", WIN_MSG_QUEUE_FNAME);
      pthread_exit (NULL);
    }

  /* Find max of our file descriptors */
  iMaxDescriptor = max (fdMessageQueue, iConnectionNumber) + 1;
#else
  iMaxDescriptor = iConnectionNumber + 1;
#endif

  /* Select event types to watch */
  if (XSelectInput (pDisplay,
		    DefaultRootWindow (pDisplay),
		    SubstructureNotifyMask |
		    StructureNotifyMask |
		    PropertyChangeMask) == BadWindow)
    ErrorF ("winClipboardProc - XSelectInput generated BadWindow "
	    "on RootWindow\n\n");

  /* Create atoms */
  atomClipboard = XInternAtom (pDisplay, "CLIPBOARD", False);
  atomClipboardManager = XInternAtom (pDisplay, "CLIPBOARD_MANAGER", False);

  /* Create a messaging window */
  iWindow = XCreateSimpleWindow (pDisplay,
				 DefaultRootWindow (pDisplay),
				 1, 1,
				 500, 500,
				 0,
				 BlackPixel (pDisplay, 0),
				 BlackPixel (pDisplay, 0));
  if (iWindow == 0)
    {
      ErrorF ("winClipboardProc - Could not create an X window.\n");
      pthread_exit (NULL);
    }

  /* Save the window in the screen privates */
  g_iClipboardWindow = iWindow;

  /* Create Windows messaging window */
  hwnd = winClipboardCreateMessagingWindow ();
  
  /* Save copy of HWND in screen privates */
  g_hwndClipboard = hwnd;

  /* Assert ownership of selections if Win32 clipboard is owned */
  if (NULL != GetClipboardOwner ())
    {
      /* PRIMARY */
      iReturn = XSetSelectionOwner (pDisplay, XA_PRIMARY,
				    iWindow, CurrentTime);
      if (iReturn == BadAtom || iReturn == BadWindow)
	{
	  ErrorF ("winClipboardProc - Could not set PRIMARY owner\n");
	  pthread_exit (NULL);
	}

      /* CLIPBOARD */
      iReturn = XSetSelectionOwner (pDisplay, atomClipboard,
				    iWindow, CurrentTime);
      if (iReturn == BadAtom || iReturn == BadWindow)
	{
	  ErrorF ("winClipboardProc - Could not set CLIPBOARD owner\n");
	  pthread_exit (NULL);
	}
    }

  /* Pre-flush X events */
  /* 
   * NOTE: Apparently you'll freeze if you don't do this,
   *	   because there may be events in local data structures
   *	   already.
   */
  winClipboardFlushXEvents (hwnd,
			    iWindow,
			    pDisplay,
			    fUseUnicode);

  /* Pre-flush Windows messages */
  if (!winClipboardFlushWindowsMessageQueue (hwnd))
    return 0;

  /* Signal that the clipboard client has started */
  g_fClipboardStarted = TRUE;

  /* Loop for X events */
  while (1)
    {
      /* Setup the file descriptor set */
      /*
       * NOTE: You have to do this before every call to select
       *       because select modifies the mask to indicate
       *       which descriptors are ready.
       */
      FD_ZERO (&fdsRead);
      FD_SET (iConnectionNumber, &fdsRead);
#ifdef HAS_DEVWINDOWS
      FD_SET (fdMessageQueue, &fdsRead);
#else
      tvTimeout.tv_sec = 0;
      tvTimeout.tv_usec = 100;
#endif

      /* Wait for a Windows event or an X event */
      iReturn = select (iMaxDescriptor,	/* Highest fds number */
			&fdsRead,	/* Read mask */
			NULL,		/* No write mask */
			NULL,		/* No exception mask */
#ifdef HAS_DEVWINDOWS
			NULL		/* No timeout */
#else
			&tvTimeout      /* Set timeout */
#endif
          );

#ifndef HAS_WINSOCK
      iSelectError = errno;
#else
      iSelectError = WSAGetLastError();
#endif

      if (iReturn < 0)
	{
#ifndef HAS_WINSOCK
          if (iSelectError == EINTR)
#else
          if (iSelectError == WSAEINTR)
#endif
            continue;
          
	  ErrorF ("winClipboardProc - Call to select () failed: %d.  "
		  "Bailing.\n", iReturn);
	  break;
	}

      /* Branch on which descriptor became active */
      if (FD_ISSET (iConnectionNumber, &fdsRead))
	{
	  /* Process X events */
	  /* Exit when we see that server is shutting down */
	  iReturn = winClipboardFlushXEvents (hwnd,
					      iWindow,
					      pDisplay,
					      fUseUnicode);
	  if (WIN_XEVENTS_SHUTDOWN == iReturn)
	    {
	      ErrorF ("winClipboardProc - winClipboardFlushXEvents "
		      "trapped shutdown event, exiting main loop.\n");
	      break;
	    }
	}

#ifdef HAS_DEVWINDOWS
      /* Check for Windows event ready */
      if (FD_ISSET (fdMessageQueue, &fdsRead))
#else
      if (1)
#endif
	{
	  /* Process Windows messages */
	  if (!winClipboardFlushWindowsMessageQueue (hwnd))
	    {
	      ErrorF ("winClipboardProc - "
		      "winClipboardFlushWindowsMessageQueue trapped "
		      "WM_QUIT message, exiting main loop.\n");
	      break;
	    }
	}
    }

  /* Close our X window */
  if (pDisplay && iWindow)
    {
      iReturn = XDestroyWindow (pDisplay, iWindow);
      if (iReturn == BadWindow)
	ErrorF ("winClipboardProc - XDestroyWindow returned BadWindow.\n");
      else
	ErrorF ("winClipboardProc - XDestroyWindow succeeded.\n");
    }


#ifdef HAS_DEVWINDOWS
  /* Close our Win32 message handle */
  if (fdMessageQueue)
    close (fdMessageQueue);
#endif

#if 0
  /*
   * FIXME: XCloseDisplay hangs if we call it, as of 2004/03/26.  The
   * XSync and XSelectInput calls did not help.
   */

  /* Discard any remaining events */
  XSync (pDisplay, TRUE);

  /* Select event types to watch */
  XSelectInput (pDisplay,
		DefaultRootWindow (pDisplay),
		None);

  /* Close our X display */
  if (pDisplay)
    {
      XCloseDisplay (pDisplay);
    }
#endif

  g_iClipboardWindow = None;
  g_pClipboardDisplay = NULL;
  g_hwndClipboard = NULL;

  return NULL;
}


/*
 * winClipboardErrorHandler - Our application specific error handler
 */

static int
winClipboardErrorHandler (Display *pDisplay, XErrorEvent *pErr)
{
  char pszErrorMsg[100];
  
  XGetErrorText (pDisplay,
		 pErr->error_code,
		 pszErrorMsg,
		 sizeof (pszErrorMsg));
  ErrorF ("winClipboardErrorHandler - ERROR: \n\t%s\n"
	  "\tSerial: %d, Request Code: %d, Minor Code: %d\n",
	  pszErrorMsg,
	  pErr->serial,
	  pErr->request_code,
	  pErr->minor_code);
  return 0;
}


/*
 * winClipboardIOErrorHandler - Our application specific IO error handler
 */

static int
winClipboardIOErrorHandler (Display *pDisplay)
{
  ErrorF ("\nwinClipboardIOErrorHandler!\n\n");

  /* Restart at the main entry point */
  longjmp (g_jmpEntry, WIN_JMP_ERROR_IO);
  
  return 0;
}
