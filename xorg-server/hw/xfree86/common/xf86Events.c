/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/*
 * Copyright (c) 1994-2003 by The XFree86 Project, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

/* [JCH-96/01/21] Extended std reverse map to four buttons. */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include <X11/Xpoll.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#define XF86_OS_PRIVS
#include "xf86_OSlib.h"
#include <X11/keysym.h>

#ifdef XFreeXDGA
#include "dgaproc.h"
#endif

#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "inputstr.h"
#include "xf86Xinput.h"

#include "mi.h"
#include "mipointer.h"

#ifdef XF86BIGFONT
#define _XF86BIGFONT_SERVER_
#include <X11/extensions/xf86bigfont.h>
#endif

#ifdef DPMSExtension
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#include "dpmsproc.h"
#endif

/*
 * The first of many hacks to get VT switching to work under
 * Solaris 2.1 for x86. The basic problem is that Solaris is supposed
 * to be SVR4. It is for the most part, except where the video interface
 * is concerned.  These hacks work around those problems.
 * See the comments for Linux, and SCO.
 *
 * This is a toggling variable:
 *  FALSE = No VT switching keys have been pressed last time around
 *  TRUE  = Possible VT switch Pending
 * (DWH - 12/2/93)
 *
 * This has been generalised to work with Linux and *BSD+syscons (DHD)
 */

_X_EXPORT Bool VTSwitchEnabled = TRUE;	/* Allows run-time disabling for
                                         *BSD and for avoiding VT
                                         switches when using the DRI
                                         automatic full screen mode.*/

extern fd_set EnabledDevices;

#ifdef XF86PM
extern void (*xf86OSPMClose)(void);
#endif

static void xf86VTSwitch(void);

/*
 * Allow arbitrary drivers or other XFree86 code to register with our main
 * Wakeup handler.
 */
typedef struct x_IHRec {
    int			fd;
    InputHandlerProc	ihproc;
    pointer		data;
    Bool		enabled;
    struct x_IHRec *	next;
} IHRec, *IHPtr;

static IHPtr InputHandlers = NULL;


Bool
LegalModifier(unsigned int key, DeviceIntPtr pDev)
{
    return TRUE;
}

/*
 * TimeSinceLastInputEvent --
 *      Function used for screensaver purposes by the os module. Returns the
 *      time in milliseconds since there last was any input.
 */
int
TimeSinceLastInputEvent()
{
  if (xf86Info.lastEventTime == 0) {
    xf86Info.lastEventTime = GetTimeInMillis();
  }
  return GetTimeInMillis() - xf86Info.lastEventTime;
}

/*
 * SetTimeSinceLastInputEvent --
 *      Set the lastEventTime to now.
 */
_X_EXPORT void
SetTimeSinceLastInputEvent()
{
  xf86Info.lastEventTime = GetTimeInMillis();
}

/*
 * ProcessInputEvents --
 *      Retrieve all waiting input events and pass them to DIX in their
 *      correct chronological order. Only reads from the system pointer
 *      and keyboard.
 */
void
ProcessInputEvents ()
{
  int x, y;

  mieqProcessInputEvents();

  /* FIXME: This is a problem if we have multiple pointers */
  miPointerGetPosition(inputInfo.pointer, &x, &y);
  xf86SetViewport(xf86Info.currentScreen, x, y);
}

/*
 * Handle keyboard events that cause some kind of "action"
 * (i.e., server termination, video mode changes, VT switches, etc.)
 */
void
xf86ProcessActionEvent(ActionEvent action, void *arg)
{
#ifdef DEBUG
    ErrorF("ProcessActionEvent(%d,%x)\n", (int) action, arg);
#endif
    switch (action) {
    case ACTION_TERMINATE:
	if (!xf86Info.dontZap) {
#ifdef XFreeXDGA
	    DGAShutdown();
#endif
	    GiveUp(0);
	}
	break;
    case ACTION_NEXT_MODE:
	if (!xf86Info.dontZoom)
	    xf86ZoomViewport(xf86Info.currentScreen,  1);
	break;
    case ACTION_PREV_MODE:
	if (!xf86Info.dontZoom)
	    xf86ZoomViewport(xf86Info.currentScreen, -1);
	break;
#if !defined(__SOL8__) && \
    (!defined(sun) || defined(__i386__)) && defined(VT_ACTIVATE)
    case ACTION_SWITCHSCREEN:
	if (VTSwitchEnabled && !xf86Info.dontVTSwitch && arg) {
	    int vtno = *((int *) arg);
#if defined(__SCO__) || defined(__UNIXWARE__)
	    vtno--;
#endif
	    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, vtno) < 0)
		ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
	}
	break;
    case ACTION_SWITCHSCREEN_NEXT:
	if (VTSwitchEnabled && !xf86Info.dontVTSwitch) {
/* Shouldn't this be true for (sun) && (i386) && (SVR4) ? */
#if defined(__SCO__) || defined(__UNIXWARE__)
	    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) < 0)
#else
	    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno + 1) < 0)
#endif
#if defined (__SCO__) || (defined(sun) && defined (__i386__) && defined (SVR4)) || defined(__UNIXWARE__)
		if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, 0) < 0)
#else
		if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, 1) < 0)
#endif
		    ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
	}
	break;
    case ACTION_SWITCHSCREEN_PREV:
	if (VTSwitchEnabled && !xf86Info.dontVTSwitch && xf86Info.vtno > 0) {
	    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno - 1) < 0)
		ErrorF("Failed to switch consoles (%s)\n", strerror(errno));
	}
	break;
#endif
    default:
	break;
    }
}

/*
 * xf86Wakeup --
 *      Os wakeup handler.
 */

/* ARGSUSED */
void
xf86Wakeup(pointer blockData, int err, pointer pReadmask)
{
    fd_set* LastSelectMask = (fd_set*)pReadmask;
    fd_set devicesWithInput;
    InputInfoPtr pInfo;

    if (err >= 0) {

	XFD_ANDSET(&devicesWithInput, LastSelectMask, &EnabledDevices);
	if (XFD_ANYSET(&devicesWithInput)) {
	    pInfo = xf86InputDevs;
	    while (pInfo) {
		if (pInfo->read_input && pInfo->fd >= 0 &&
		    (FD_ISSET(pInfo->fd, &devicesWithInput) != 0)) {
		    int sigstate = xf86BlockSIGIO();

		    pInfo->read_input(pInfo);
		    xf86UnblockSIGIO(sigstate);
		    /*
		     * Remove the descriptior from the set because more than one
		     * device may share the same file descriptor.
		     */
		    FD_CLR(pInfo->fd, &devicesWithInput);
		}
		pInfo = pInfo->next;
	    }
	}
    }

    if (err >= 0) { /* we don't want the handlers called if select() */
	IHPtr ih;   /* returned with an error condition, do we?      */
	
	for (ih = InputHandlers; ih; ih = ih->next) {
	    if (ih->enabled && ih->fd >= 0 && ih->ihproc &&
		(FD_ISSET(ih->fd, ((fd_set *)pReadmask)) != 0)) {
		ih->ihproc(ih->fd, ih->data);
	    }
	}
    }

    if (xf86VTSwitchPending()) xf86VTSwitch();
}


/*
 * xf86SigioReadInput --
 *    signal handler for the SIGIO signal.
 */
static void
xf86SigioReadInput(int fd,
		   void *closure)
{
    int errno_save = errno;
    int sigstate = xf86BlockSIGIO();
    InputInfoPtr pInfo = (InputInfoPtr) closure;

    pInfo->read_input(pInfo);

    xf86UnblockSIGIO(sigstate);
    errno = errno_save;
}

/*
 * xf86AddEnabledDevice --
 *
 */
_X_EXPORT void
xf86AddEnabledDevice(InputInfoPtr pInfo)
{
    if (!xf86InstallSIGIOHandler (pInfo->fd, xf86SigioReadInput, pInfo)) {
	AddEnabledDevice(pInfo->fd);
    }
}

/*
 * xf86RemoveEnabledDevice --
 *
 */
_X_EXPORT void
xf86RemoveEnabledDevice(InputInfoPtr pInfo)
{
    if (!xf86RemoveSIGIOHandler (pInfo->fd)) {
	RemoveEnabledDevice(pInfo->fd);
    }
}

static int *xf86SignalIntercept = NULL;

_X_EXPORT void
xf86InterceptSignals(int *signo)
{
    if ((xf86SignalIntercept = signo))
	*signo = -1;
}

static void (*xf86SigIllHandler)(void) = NULL;

_X_EXPORT void
xf86InterceptSigIll(void (*sigillhandler)(void))
{
    xf86SigIllHandler = sigillhandler;
}

/*
 * xf86SigHandler --
 *    Catch unexpected signals and exit or continue cleanly.
 */
void
xf86SigHandler(int signo)
{
  if ((signo == SIGILL) && xf86SigIllHandler) {
    (*xf86SigIllHandler)();
    /* Re-arm handler just in case we unexpectedly return here */
    (void) signal(signo, xf86SigHandler);
    return;
  }

  if (xf86SignalIntercept && (*xf86SignalIntercept < 0)) {
    *xf86SignalIntercept = signo;
    /* Re-arm handler just in case */
    (void) signal(signo, xf86SigHandler);
    return;
  }

  signal(signo,SIG_IGN);
  xf86Info.caughtSignal = TRUE;
#ifdef XF86BIGFONT
  XF86BigfontCleanup();
#endif

  xorg_backtrace();

  FatalError("Caught signal %d.  Server aborting\n", signo);
}

/*
 * xf86PrintBacktrace --
 *    Print a stack backtrace for debugging purposes.
 */
_X_EXPORT void
xf86PrintBacktrace(void)
{
    xorg_backtrace();
}

#define KeyPressed(k) (keyc->postdown[k >> 3] & (1 << (k & 7)))
#define ModifierDown(k) ((keyc->state & (k)) == (k))

static void
xf86ReleaseKeys(DeviceIntPtr pDev)
{
    KeyClassPtr keyc = NULL;
    KeySym *map = NULL;
    xEvent ke;
    int i = 0, j = 0, nevents = 0;

    if (!pDev || !pDev->key)
        return;

    keyc = pDev->key;
    map = keyc->curKeySyms.map;

    /*
     * Hmm... here is the biggest hack of every time !
     * It may be possible that a switch-vt procedure has finished BEFORE
     * you released all keys neccessary to do this. That peculiar behavior
     * can fool the X-server pretty much, cause it assumes that some keys
     * were not released. TWM may stuck alsmost completly....
     * OK, what we are doing here is after returning from the vt-switch
     * exeplicitely unrelease all keyboard keys before the input-devices
     * are reenabled.
     */

    for (i = keyc->curKeySyms.minKeyCode, map = keyc->curKeySyms.map;
         i < keyc->curKeySyms.maxKeyCode;
         i++, map += keyc->curKeySyms.mapWidth) {
        if (KeyPressed(i)) {
            switch (*map) {
            /* Don't release the lock keys */
            case XK_Caps_Lock:
            case XK_Shift_Lock:
            case XK_Num_Lock:
            case XK_Scroll_Lock:
            case XK_Kana_Lock:
                break;
            default:
                if (pDev == inputInfo.keyboard) {
                    ke.u.keyButtonPointer.time = GetTimeInMillis();
                    ke.u.keyButtonPointer.rootX = 0;
                    ke.u.keyButtonPointer.rootY = 0;
                    ke.u.u.type = KeyRelease;
                    ke.u.u.detail = i;
                    (*pDev->public.processInputProc) (&ke, pDev, 1);
                }
                else {
		    int sigstate = xf86BlockSIGIO ();
                    nevents = GetKeyboardEvents(xf86Events, pDev, KeyRelease, i);
                    for (j = 0; j < nevents; j++)
                        mieqEnqueue(pDev, (xf86Events + j)->event);
		    xf86UnblockSIGIO(sigstate);
                }
                break;
            }
        }
    }
}

/*
 * xf86VTSwitch --
 *      Handle requests for switching the vt.
 */
static void
xf86VTSwitch()
{
  int i, prevSIGIO;
  InputInfoPtr pInfo;
  IHPtr ih;

#ifdef DEBUG
  ErrorF("xf86VTSwitch()\n");
#endif

#ifdef XFreeXDGA
  if(!DGAVTSwitch())
	return;
#endif

  /*
   * Since all screens are currently all in the same state it is sufficient
   * check the first.  This might change in future.
   */
  if (xf86Screens[0]->vtSema) {

#ifdef DEBUG
    ErrorF("xf86VTSwitch: Leaving, xf86Exiting is %s\n",
	   BOOLTOSTRING((dispatchException & DE_TERMINATE) ? TRUE : FALSE));
#endif
#ifdef DPMSExtension
    if (DPMSPowerLevel != DPMSModeOn)
	DPMSSet(serverClient, DPMSModeOn);
#endif
    for (i = 0; i < xf86NumScreens; i++) {
      if (!(dispatchException & DE_TERMINATE))
	if (xf86Screens[i]->EnableDisableFBAccess)
	  (*xf86Screens[i]->EnableDisableFBAccess) (i, FALSE);
    }

    /*
     * Keep the order: Disable Device > LeaveVT
     *                        EnterVT > EnableDevice
     */
    pInfo = xf86InputDevs;
    while (pInfo) {
      if (pInfo->dev)
          DisableDevice(pInfo->dev);
      pInfo = pInfo->next;
    }
    xf86EnterServerState(SETUP);
    for (i = 0; i < xf86NumScreens; i++)
	xf86Screens[i]->LeaveVT(i, 0);

    for (ih = InputHandlers; ih; ih = ih->next)
      xf86DisableInputHandler(ih);
    xf86AccessLeave();      /* We need this here, otherwise */
    xf86AccessLeaveState(); /* console won't be restored    */

    if (!xf86VTSwitchAway()) {
      /*
       * switch failed
       */

#ifdef DEBUG
      ErrorF("xf86VTSwitch: Leave failed\n");
#endif
      prevSIGIO = xf86BlockSIGIO();
      xf86AccessEnter();
      xf86EnterServerState(SETUP);
      for (i = 0; i < xf86NumScreens; i++) {
	if (!xf86Screens[i]->EnterVT(i, 0))
	  FatalError("EnterVT failed for screen %d\n", i);
      }
      xf86EnterServerState(OPERATING);
      if (!(dispatchException & DE_TERMINATE)) {
	for (i = 0; i < xf86NumScreens; i++) {
	  if (xf86Screens[i]->EnableDisableFBAccess)
	    (*xf86Screens[i]->EnableDisableFBAccess) (i, TRUE);
	}
      }
      dixSaveScreens(serverClient, SCREEN_SAVER_FORCER, ScreenSaverReset);

      pInfo = xf86InputDevs;
      while (pInfo) {
        if (pInfo->dev) {
            xf86ReleaseKeys(pInfo->dev);
            EnableDevice(pInfo->dev);
        }
	pInfo = pInfo->next;
      }
      /* XXX HACK */
      xf86ReleaseKeys(inputInfo.keyboard);
      for (ih = InputHandlers; ih; ih = ih->next)
        xf86EnableInputHandler(ih);

      xf86UnblockSIGIO(prevSIGIO);

    } else {
#ifdef XF86PM
	  if (xf86OSPMClose)
	      xf86OSPMClose();
	  xf86OSPMClose = NULL;
#endif

	for (i = 0; i < xf86NumScreens; i++) {
 	    /*
 	     * zero all access functions to
 	     * trap calls when switched away.
 	     */
	    xf86Screens[i]->vtSema = FALSE;
	    xf86Screens[i]->access = NULL;
	    xf86Screens[i]->busAccess = NULL;
	}
	if (xorgHWAccess)
	    xf86DisableIO();
    }
  } else {

#ifdef DEBUG
    ErrorF("xf86VTSwitch: Entering\n");
#endif
    if (!xf86VTSwitchTo()) return;

    prevSIGIO = xf86BlockSIGIO();
#ifdef XF86PM
    xf86OSPMClose = xf86OSPMOpen();
#endif

    if (xorgHWAccess)
	xf86EnableIO();
    xf86AccessEnter();
    xf86EnterServerState(SETUP);
    for (i = 0; i < xf86NumScreens; i++) {
      xf86Screens[i]->vtSema = TRUE;
      if (!xf86Screens[i]->EnterVT(i, 0))
	  FatalError("EnterVT failed for screen %d\n", i);
    }
    xf86EnterServerState(OPERATING);
    for (i = 0; i < xf86NumScreens; i++) {
      if (xf86Screens[i]->EnableDisableFBAccess)
	(*xf86Screens[i]->EnableDisableFBAccess)(i, TRUE);
    }

    /* Turn screen saver off when switching back */
    dixSaveScreens(serverClient, SCREEN_SAVER_FORCER, ScreenSaverReset);

    pInfo = xf86InputDevs;
    while (pInfo) {
      if (pInfo->dev) {
          xf86ReleaseKeys(pInfo->dev);
          EnableDevice(pInfo->dev);
      }
      pInfo = pInfo->next;
    }
    /* XXX HACK */
    xf86ReleaseKeys(inputInfo.keyboard);

    for (ih = InputHandlers; ih; ih = ih->next)
      xf86EnableInputHandler(ih);

    xf86UnblockSIGIO(prevSIGIO);
  }
}


/* Input handler registration */

static pointer
addInputHandler(int fd, InputHandlerProc proc, pointer data)
{
    IHPtr ih;

    if (fd < 0 || !proc)
	return NULL;

    ih = xcalloc(sizeof(*ih), 1);
    if (!ih)
	return NULL;

    ih->fd = fd;
    ih->ihproc = proc;
    ih->data = data;
    ih->enabled = TRUE;

    ih->next = InputHandlers;
    InputHandlers = ih;

    return ih;
}

_X_EXPORT pointer
xf86AddInputHandler(int fd, InputHandlerProc proc, pointer data)
{
    IHPtr ih = addInputHandler(fd, proc, data);

    if (ih)
        AddEnabledDevice(fd);
    return ih;
}

_X_EXPORT pointer
xf86AddGeneralHandler(int fd, InputHandlerProc proc, pointer data)
{
    IHPtr ih = addInputHandler(fd, proc, data);

    if (ih)
        AddGeneralSocket(fd);
    return ih;
}

static void
removeInputHandler(IHPtr ih)
{
    IHPtr p;

    if (ih == InputHandlers)
	InputHandlers = ih->next;
    else {
	p = InputHandlers;
	while (p && p->next != ih)
	    p = p->next;
	if (ih)
	    p->next = ih->next;
    }
    xfree(ih);
}

_X_EXPORT int
xf86RemoveInputHandler(pointer handler)
{
    IHPtr ih;
    int fd;

    if (!handler)
	return -1;

    ih = handler;
    fd = ih->fd;

    if (ih->fd >= 0)
	RemoveEnabledDevice(ih->fd);
    removeInputHandler(ih);

    return fd;
}

_X_EXPORT int
xf86RemoveGeneralHandler(pointer handler)
{
    IHPtr ih;
    int fd;

    if (!handler)
	return -1;

    ih = handler;
    fd = ih->fd;

    if (ih->fd >= 0)
	RemoveGeneralSocket(ih->fd);
    removeInputHandler(ih);

    return fd;
}

_X_EXPORT void
xf86DisableInputHandler(pointer handler)
{
    IHPtr ih;

    if (!handler)
	return;

    ih = handler;
    ih->enabled = FALSE;
    if (ih->fd >= 0)
	RemoveEnabledDevice(ih->fd);
}

_X_EXPORT void
xf86DisableGeneralHandler(pointer handler)
{
    IHPtr ih;

    if (!handler)
	return;

    ih = handler;
    ih->enabled = FALSE;
    if (ih->fd >= 0)
	RemoveGeneralSocket(ih->fd);
}

_X_EXPORT void
xf86EnableInputHandler(pointer handler)
{
    IHPtr ih;

    if (!handler)
	return;

    ih = handler;
    ih->enabled = TRUE;
    if (ih->fd >= 0)
	AddEnabledDevice(ih->fd);
}

_X_EXPORT void
xf86EnableGeneralHandler(pointer handler)
{
    IHPtr ih;

    if (!handler)
	return;

    ih = handler;
    ih->enabled = TRUE;
    if (ih->fd >= 0)
	AddGeneralSocket(ih->fd);
}

/*
 * As used currently by the DRI, the return value is ignored.
 */
_X_EXPORT Bool
xf86EnableVTSwitch(Bool new)
{
    static Bool def = TRUE;
    Bool old;

    old = VTSwitchEnabled;
    if (!new) {
	/* Disable VT switching */
	def = VTSwitchEnabled;
	VTSwitchEnabled = FALSE;
    } else {
	/* Restore VT switching to default */
	VTSwitchEnabled = def;
    }
    return old;
}

void
xf86ReloadInputDevs(int sig)
{
  InputInfoPtr pInfo;

  signal(sig, (void(*)(int))xf86ReloadInputDevs);

  pInfo = xf86InputDevs;
  while (pInfo) {
    DisableDevice(pInfo->dev);
    EnableDevice(pInfo->dev);
    pInfo = pInfo->next;
  }

  return;
}

_X_EXPORT void
DDXRingBell(int volume, int pitch, int duration) {
    xf86OSRingBell(volume, pitch, duration);
}
