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
#include <X11/Xatom.h>
#include "misc.h"
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

#include "xkbsrv.h"
#include "xkbstr.h"

#ifdef DPMSExtension
#include <X11/extensions/dpmsconst.h>
#include "dpmsproc.h"
#endif

#include "xf86platformBus.h"
#include "systemd-logind.h"

/*
 * This is a toggling variable:
 *  FALSE = No VT switching keys have been pressed last time around
 *  TRUE  = Possible VT switch Pending
 * (DWH - 12/2/93)
 *
 * This has been generalised to work with Linux and *BSD+syscons (DHD)
 */

Bool VTSwitchEnabled = TRUE;    /* Allows run-time disabling for
                                 *BSD and for avoiding VT
                                 switches when using the DRI
                                 automatic full screen mode.*/

extern fd_set EnabledDevices;

#ifdef XF86PM
extern void (*xf86OSPMClose) (void);
#endif

static void xf86VTSwitch(void);

/*
 * Allow arbitrary drivers or other XFree86 code to register with our main
 * Wakeup handler.
 */
typedef struct x_IHRec {
    int fd;
    InputHandlerProc ihproc;
    void *data;
    Bool enabled;
    Bool is_input;
    struct x_IHRec *next;
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
TimeSinceLastInputEvent(void)
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
void
SetTimeSinceLastInputEvent(void)
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
ProcessInputEvents(void)
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
    DebugF("ProcessActionEvent(%d,%x)\n", (int) action, arg);
    switch (action) {
    case ACTION_TERMINATE:
        if (!xf86Info.dontZap) {
            xf86Msg(X_INFO, "Server zapped. Shutting down.\n");
#ifdef XFreeXDGA
            DGAShutdown();
#endif
            GiveUp(0);
        }
        break;
    case ACTION_NEXT_MODE:
        if (!xf86Info.dontZoom)
            xf86ZoomViewport(xf86Info.currentScreen, 1);
        break;
    case ACTION_PREV_MODE:
        if (!xf86Info.dontZoom)
            xf86ZoomViewport(xf86Info.currentScreen, -1);
        break;
    case ACTION_SWITCHSCREEN:
        if (VTSwitchEnabled && !xf86Info.dontVTSwitch && arg) {
            int vtno = *((int *) arg);

            if (vtno != xf86Info.vtno) {
                if (!xf86VTActivate(vtno)) {
                    ErrorF("Failed to switch from vt%02d to vt%02d: %s\n",
                           xf86Info.vtno, vtno, strerror(errno));
                }
            }
        }
        break;
    case ACTION_SWITCHSCREEN_NEXT:
        if (VTSwitchEnabled && !xf86Info.dontVTSwitch) {
            if (!xf86VTActivate(xf86Info.vtno + 1)) {
                /* If first try failed, assume this is the last VT and
                 * try wrapping around to the first vt.
                 */
                if (!xf86VTActivate(1)) {
                    ErrorF("Failed to switch from vt%02d to next vt: %s\n",
                           xf86Info.vtno, strerror(errno));
                }
            }
        }
        break;
    case ACTION_SWITCHSCREEN_PREV:
        if (VTSwitchEnabled && !xf86Info.dontVTSwitch && xf86Info.vtno > 0) {
            if (!xf86VTActivate(xf86Info.vtno - 1)) {
                /* Don't know what the maximum VT is, so can't wrap around */
                ErrorF("Failed to switch from vt%02d to previous vt: %s\n",
                       xf86Info.vtno, strerror(errno));
            }
        }
        break;
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
xf86Wakeup(void *blockData, int err, void *pReadmask)
{
    fd_set *LastSelectMask = (fd_set *) pReadmask;
    fd_set devicesWithInput;
    InputInfoPtr pInfo;

    if (err >= 0) {

        XFD_ANDSET(&devicesWithInput, LastSelectMask, &EnabledDevices);
        if (XFD_ANYSET(&devicesWithInput)) {
            pInfo = xf86InputDevs;
            while (pInfo) {
                if (pInfo->read_input && pInfo->fd >= 0 &&
                    (FD_ISSET(pInfo->fd, &devicesWithInput) != 0)) {
                    OsBlockSIGIO();

                    /*
                     * Remove the descriptior from the set because more than one
                     * device may share the same file descriptor.
                     */
                    FD_CLR(pInfo->fd, &devicesWithInput);

                    pInfo->read_input(pInfo);
                    OsReleaseSIGIO();
                }
                pInfo = pInfo->next;
            }
        }
    }

    if (err >= 0) {             /* we don't want the handlers called if select() */
        IHPtr ih, ih_tmp;       /* returned with an error condition, do we?      */

        nt_list_for_each_entry_safe(ih, ih_tmp, InputHandlers, next) {
            if (ih->enabled && ih->fd >= 0 && ih->ihproc &&
                (FD_ISSET(ih->fd, ((fd_set *) pReadmask)) != 0)) {
                ih->ihproc(ih->fd, ih->data);
            }
        }
    }

    if (xf86VTSwitchPending())
        xf86VTSwitch();
}

/*
 * xf86SigioReadInput --
 *    signal handler for the SIGIO signal.
 */
static void
xf86SigioReadInput(int fd, void *closure)
{
    int errno_save = errno;
    InputInfoPtr pInfo = closure;

    pInfo->read_input(pInfo);

    errno = errno_save;
}

/*
 * xf86AddEnabledDevice --
 *
 */
void
xf86AddEnabledDevice(InputInfoPtr pInfo)
{
    if (!xf86InstallSIGIOHandler(pInfo->fd, xf86SigioReadInput, pInfo)) {
        AddEnabledDevice(pInfo->fd);
    }
}

/*
 * xf86RemoveEnabledDevice --
 *
 */
void
xf86RemoveEnabledDevice(InputInfoPtr pInfo)
{
    if (!xf86RemoveSIGIOHandler(pInfo->fd)) {
        RemoveEnabledDevice(pInfo->fd);
    }
}

static int *xf86SignalIntercept = NULL;

void
xf86InterceptSignals(int *signo)
{
    if ((xf86SignalIntercept = signo))
        *signo = -1;
}

static void (*xf86SigIllHandler) (void) = NULL;

void
xf86InterceptSigIll(void (*sigillhandler) (void))
{
    xf86SigIllHandler = sigillhandler;
}

/*
 * xf86SigWrapper --
 *    Catch unexpected signals and exit or continue cleanly.
 */
int
xf86SigWrapper(int signo)
{
    if ((signo == SIGILL) && xf86SigIllHandler) {
        (*xf86SigIllHandler) ();
        return 0;               /* continue */
    }

    if (xf86SignalIntercept && (*xf86SignalIntercept < 0)) {
        *xf86SignalIntercept = signo;
        return 0;               /* continue */
    }

    xf86Info.caughtSignal = TRUE;
    return 1;                   /* abort */
}

/*
 * xf86PrintBacktrace --
 *    Print a stack backtrace for debugging purposes.
 */
void
xf86PrintBacktrace(void)
{
    xorg_backtrace();
}

static void
xf86ReleaseKeys(DeviceIntPtr pDev)
{
    KeyClassPtr keyc;
    int i;

    if (!pDev || !pDev->key)
        return;

    keyc = pDev->key;

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

    for (i = keyc->xkbInfo->desc->min_key_code;
         i < keyc->xkbInfo->desc->max_key_code; i++) {
        if (key_is_down(pDev, i, KEY_POSTED)) {
            OsBlockSIGIO();
            QueueKeyboardEvents(pDev, KeyRelease, i, NULL);
            OsReleaseSIGIO();
        }
    }
}

void
xf86DisableInputDeviceForVTSwitch(InputInfoPtr pInfo)
{
    if (!pInfo->dev)
        return;

    if (!pInfo->dev->enabled)
        pInfo->flags |= XI86_DEVICE_DISABLED;

    xf86ReleaseKeys(pInfo->dev);
    ProcessInputEvents();
    DisableDevice(pInfo->dev, TRUE);
}

void
xf86EnableInputDeviceForVTSwitch(InputInfoPtr pInfo)
{
    if (pInfo->dev && (pInfo->flags & XI86_DEVICE_DISABLED) == 0)
        EnableDevice(pInfo->dev, TRUE);
    pInfo->flags &= ~XI86_DEVICE_DISABLED;
}

/*
 * xf86UpdateHasVTProperty --
 *    Update a flag property on the root window to say whether the server VT
 *    is currently the active one as some clients need to know this.
 */
static void
xf86UpdateHasVTProperty(Bool hasVT)
{
    Atom property_name;
    int32_t value = hasVT ? 1 : 0;
    int i;

    property_name = MakeAtom(HAS_VT_ATOM_NAME, sizeof(HAS_VT_ATOM_NAME) - 1,
                             FALSE);
    if (property_name == BAD_RESOURCE)
        FatalError("Failed to retrieve \"HAS_VT\" atom\n");
    for (i = 0; i < xf86NumScreens; i++) {
        ChangeWindowProperty(xf86ScrnToScreen(xf86Screens[i])->root,
                             property_name, XA_INTEGER, 32,
                             PropModeReplace, 1, &value, TRUE);
    }
}

void
xf86VTLeave(void)
{
    int i;
    InputInfoPtr pInfo;
    IHPtr ih;

    DebugF("xf86VTSwitch: Leaving, xf86Exiting is %s\n",
           BOOLTOSTRING((dispatchException & DE_TERMINATE) ? TRUE : FALSE));
#ifdef DPMSExtension
    if (DPMSPowerLevel != DPMSModeOn)
        DPMSSet(serverClient, DPMSModeOn);
#endif
    for (i = 0; i < xf86NumScreens; i++) {
        if (!(dispatchException & DE_TERMINATE))
            if (xf86Screens[i]->EnableDisableFBAccess)
                (*xf86Screens[i]->EnableDisableFBAccess) (xf86Screens[i], FALSE);
    }

    /*
     * Keep the order: Disable Device > LeaveVT
     *                        EnterVT > EnableDevice
     */
    for (ih = InputHandlers; ih; ih = ih->next) {
        if (ih->is_input)
            xf86DisableInputHandler(ih);
        else
            xf86DisableGeneralHandler(ih);
    }
    for (pInfo = xf86InputDevs; pInfo; pInfo = pInfo->next)
        xf86DisableInputDeviceForVTSwitch(pInfo);

    OsBlockSIGIO();
    for (i = 0; i < xf86NumScreens; i++)
        xf86Screens[i]->LeaveVT(xf86Screens[i]);
    for (i = 0; i < xf86NumGPUScreens; i++)
        xf86GPUScreens[i]->LeaveVT(xf86GPUScreens[i]);

    xf86AccessLeave();      /* We need this here, otherwise */

    if (!xf86VTSwitchAway())
        goto switch_failed;

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
    }
    if (xorgHWAccess)
        xf86DisableIO();

    xf86UpdateHasVTProperty(FALSE);

    return;

switch_failed:
    DebugF("xf86VTSwitch: Leave failed\n");
    xf86AccessEnter();
    for (i = 0; i < xf86NumScreens; i++) {
        if (!xf86Screens[i]->EnterVT(xf86Screens[i]))
            FatalError("EnterVT failed for screen %d\n", i);
    }
    for (i = 0; i < xf86NumGPUScreens; i++) {
        if (!xf86GPUScreens[i]->EnterVT(xf86GPUScreens[i]))
            FatalError("EnterVT failed for gpu screen %d\n", i);
    }
    if (!(dispatchException & DE_TERMINATE)) {
        for (i = 0; i < xf86NumScreens; i++) {
            if (xf86Screens[i]->EnableDisableFBAccess)
                (*xf86Screens[i]->EnableDisableFBAccess) (xf86Screens[i], TRUE);
        }
    }
    dixSaveScreens(serverClient, SCREEN_SAVER_FORCER, ScreenSaverReset);

    for (pInfo = xf86InputDevs; pInfo; pInfo = pInfo->next)
        xf86EnableInputDeviceForVTSwitch(pInfo);
    for (ih = InputHandlers; ih; ih = ih->next) {
        if (ih->is_input)
            xf86EnableInputHandler(ih);
        else
            xf86EnableGeneralHandler(ih);
    }
    OsReleaseSIGIO();
}

void
xf86VTEnter(void)
{
    int i;
    InputInfoPtr pInfo;
    IHPtr ih;

    DebugF("xf86VTSwitch: Entering\n");
    if (!xf86VTSwitchTo())
        return;

#ifdef XF86PM
    xf86OSPMClose = xf86OSPMOpen();
#endif

    if (xorgHWAccess)
        xf86EnableIO();
    xf86AccessEnter();
    for (i = 0; i < xf86NumScreens; i++) {
        xf86Screens[i]->vtSema = TRUE;
        if (!xf86Screens[i]->EnterVT(xf86Screens[i]))
            FatalError("EnterVT failed for screen %d\n", i);
    }
    for (i = 0; i < xf86NumGPUScreens; i++) {
        xf86GPUScreens[i]->vtSema = TRUE;
        if (!xf86GPUScreens[i]->EnterVT(xf86GPUScreens[i]))
            FatalError("EnterVT failed for gpu screen %d\n", i);
    }
    for (i = 0; i < xf86NumScreens; i++) {
        if (xf86Screens[i]->EnableDisableFBAccess)
            (*xf86Screens[i]->EnableDisableFBAccess) (xf86Screens[i], TRUE);
    }

    /* Turn screen saver off when switching back */
    dixSaveScreens(serverClient, SCREEN_SAVER_FORCER, ScreenSaverReset);

    /* If we use systemd-logind it will enable input devices for us */
    if (!systemd_logind_controls_session())
        for (pInfo = xf86InputDevs; pInfo; pInfo = pInfo->next)
            xf86EnableInputDeviceForVTSwitch(pInfo);

    for (ih = InputHandlers; ih; ih = ih->next) {
        if (ih->is_input)
            xf86EnableInputHandler(ih);
        else
            xf86EnableGeneralHandler(ih);
    }
#ifdef XSERVER_PLATFORM_BUS
    /* check for any new output devices */
    xf86platformVTProbe();
#endif

    xf86UpdateHasVTProperty(TRUE);

    OsReleaseSIGIO();
}

/*
 * xf86VTSwitch --
 *      Handle requests for switching the vt.
 */
static void
xf86VTSwitch(void)
{
    DebugF("xf86VTSwitch()\n");

#ifdef XFreeXDGA
    if (!DGAVTSwitch())
        return;
#endif

    /*
     * Since all screens are currently all in the same state it is sufficient
     * check the first.  This might change in future.
     *
     * VTLeave is always handled here (VT_PROCESS guarantees this is safe),
     * if we use systemd_logind xf86VTEnter() gets called by systemd-logind.c
     * once it has resumed all drm nodes.
     */
    if (xf86VTOwner())
        xf86VTLeave();
    else if (!systemd_logind_controls_session())
        xf86VTEnter();
}

/* Input handler registration */

static void *
addInputHandler(int fd, InputHandlerProc proc, void *data)
{
    IHPtr ih;

    if (fd < 0 || !proc)
        return NULL;

    ih = calloc(sizeof(*ih), 1);
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

void *
xf86AddInputHandler(int fd, InputHandlerProc proc, void *data)
{
    IHPtr ih = addInputHandler(fd, proc, data);

    if (ih) {
        AddEnabledDevice(fd);
        ih->is_input = TRUE;
    }
    return ih;
}

void *
xf86AddGeneralHandler(int fd, InputHandlerProc proc, void *data)
{
    IHPtr ih = addInputHandler(fd, proc, data);

    if (ih)
        AddGeneralSocket(fd);
    return ih;
}

/**
 * Set the handler for the console's fd. Replaces (and returns) the previous
 * handler or NULL, whichever appropriate.
 * proc may be NULL if the server should not handle events on the console.
 */
InputHandlerProc
xf86SetConsoleHandler(InputHandlerProc proc, void *data)
{
    static IHPtr handler = NULL;
    InputHandlerProc old_proc = NULL;

    if (handler) {
        old_proc = handler->ihproc;
        xf86RemoveGeneralHandler(handler);
    }

    handler = xf86AddGeneralHandler(xf86Info.consoleFd, proc, data);

    return old_proc;
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
    free(ih);
}

int
xf86RemoveInputHandler(void *handler)
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

int
xf86RemoveGeneralHandler(void *handler)
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

void
xf86DisableInputHandler(void *handler)
{
    IHPtr ih;

    if (!handler)
        return;

    ih = handler;
    ih->enabled = FALSE;
    if (ih->fd >= 0)
        RemoveEnabledDevice(ih->fd);
}

void
xf86DisableGeneralHandler(void *handler)
{
    IHPtr ih;

    if (!handler)
        return;

    ih = handler;
    ih->enabled = FALSE;
    if (ih->fd >= 0)
        RemoveGeneralSocket(ih->fd);
}

void
xf86EnableInputHandler(void *handler)
{
    IHPtr ih;

    if (!handler)
        return;

    ih = handler;
    ih->enabled = TRUE;
    if (ih->fd >= 0)
        AddEnabledDevice(ih->fd);
}

void
xf86EnableGeneralHandler(void *handler)
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
Bool
xf86EnableVTSwitch(Bool new)
{
    static Bool def = TRUE;
    Bool old;

    old = VTSwitchEnabled;
    if (!new) {
        /* Disable VT switching */
        def = VTSwitchEnabled;
        VTSwitchEnabled = FALSE;
    }
    else {
        /* Restore VT switching to default */
        VTSwitchEnabled = def;
    }
    return old;
}

void
DDXRingBell(int volume, int pitch, int duration)
{
    xf86OSRingBell(volume, pitch, duration);
}

Bool
xf86VTOwner(void)
{
    /* at system startup xf86Screens[0] won't be set - but we will own the VT */
    if (xf86NumScreens == 0)
	return TRUE;
    return xf86Screens[0]->vtSema;
}
