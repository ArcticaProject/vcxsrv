/*
 * Copyright 2001 by J. Kean Johnston <jkj@sco.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name J. Kean Johnston not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  J. Kean Johnston makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * J. KEAN JOHNSTON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL J. KEAN JOHNSTON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86Xinput.h"
#include "xf86OSmouse.h"
#include "mipointer.h"
#include <sys/event.h>
#include <mouse.h>

static int
SupportedInterfaces (void)
{
  return MSE_MISC;
}

static const char *internalNames[] = {
  "OSMouse",
  NULL
};

static const char **
BuiltinNames (void)
{
  return internalNames;
}

static Bool
CheckProtocol (const char *protocol)
{
  int i;

  for (i = 0; internalNames[i]; i++) {
    if (xf86NameCmp (protocol, internalNames[i]) == 0)
      return TRUE;
  }

  return FALSE;
}

static const char *
DefaultProtocol (void)
{
  return "OSMouse";
}

static const char *
evtErrStr (int evterr)
{
  switch (evterr) {
  case -1: return "error in config files";
  case -2: return "no mouse devices to attach";
  case -3: return "unable to open device";
  case -4: return "unable to open event queue";
  case -999: return "unable to initialize event driver";
  default: return "unknown event driver error";
  }
}

static int
OsMouseProc (DeviceIntPtr pPointer, int what)
{
  InputInfoPtr pInfo;
  MouseDevPtr pMse;
  unsigned char map[9];
  dmask_t dmask;
  MessageType from = X_CONFIG;
  int evi;

  pInfo = pPointer->public.devicePrivate;
  pMse = pInfo->private;
  pMse->device = pPointer;

  switch (what) {
  case DEVICE_INIT: 
    pPointer->public.on = FALSE;

    dmask = D_ABS | D_REL | D_BUTTON;
    if ((evi = ev_initf(xf86Info.consoleFd)) < 0) {
      FatalError ("OsMouseProc: Event driver initialization failed (%s)\n",
          evtErrStr(evi));
    }
    pInfo->fd = ev_open (&dmask);
    if (pInfo->fd < 0) {
      FatalError ("OsMouseProc: DEVICE_INIT failed (%s)\n", evtErrStr(pInfo->fd));
    }

    pMse->buttons = xf86SetIntOption (pInfo->options, "Buttons", 0);
    if (pMse->buttons == 0) {
      pMse->buttons = 8;
      from = X_DEFAULT;
    }
    xf86Msg (from, "%s: Buttons: %d\n", pInfo->name, pMse->buttons);

    for (evi = 0; evi <= 8; evi++)
      map[evi] = evi;

    InitPointerDeviceStruct((DevicePtr)pPointer, map, 8,
        miPointerGetMotionEvents, pMse->Ctrl,
        miPointerGetMotionBufferSize());

    /* X valuator */
    xf86InitValuatorAxisStruct(pPointer, 0, 0, -1, 1, 0, 1);
    xf86InitValuatorDefaults(pPointer, 0);

    /* Y valuator */
    xf86InitValuatorAxisStruct(pPointer, 1, 0, -1, 1, 0, 1);
    xf86InitValuatorDefaults(pPointer, 1);

    xf86MotionHistoryAllocate(pInfo);

    ev_flush();
    ev_suspend();
    break;

  case DEVICE_ON:
    pMse->lastButtons = 0;
    pMse->lastMappedButtons = 0;
    pMse->emulateState = 0;
    pPointer->public.on = TRUE;
    ev_resume();
    AddEnabledDevice (pInfo->fd);
    break;

  case DEVICE_OFF:
  case DEVICE_CLOSE:
    pPointer->public.on = FALSE;
    RemoveEnabledDevice (pInfo->fd);
    if (what == DEVICE_CLOSE) {
      ev_close();
      pInfo->fd = -1;
    } else {
      ev_suspend();
    }
    break;
  }

  return Success;
}

static void
OsMouseReadInput (InputInfoPtr pInfo)
{
  MouseDevPtr pMse;
  EVENT *evp;

  pMse = pInfo->private;

  while ((evp = ev_read()) != (EVENT *)0) {
    int buttons = EV_BUTTONS(*evp);
    int dx = EV_DX(*evp), dy = -(EV_DY(*evp)), dz = 0;

    if (buttons & WHEEL_FWD)
      dz = -1;
    else if (buttons & WHEEL_BACK)
      dz = 1;

    buttons &= ~(WHEEL_FWD | WHEEL_BACK);

    pMse->PostEvent (pInfo, buttons, dx, dy, dz, 0);
    ev_pop();
  }
}

static Bool
OsMousePreInit(InputInfoPtr pInfo, const char *protocol, int flags)
{
  MouseDevPtr pMse;

  /* This is called when the protocol is "OSMouse". */

  pMse = pInfo->private;
  pMse->protocol = protocol;
  xf86Msg(X_CONFIG, "%s: Protocol: %s\n", pInfo->name, protocol);

  /* Collect the options, and process the common options. */
  xf86CollectInputOptions(pInfo, NULL, NULL);
  xf86ProcessCommonOptions(pInfo, pInfo->options);

  /* Check if the device can be opened. */
  pInfo->fd = ev_initf(xf86Info.consoleFd);
  if (pInfo->fd != -1) {
    dmask_t dmask = (D_ABS | D_REL | D_BUTTON);
    pInfo->fd = ev_open(&dmask);
  } else {
    pInfo->fd = -999;
  }

  if (pInfo->fd < 0) {
    if (xf86GetAllowMouseOpenFail())
      xf86Msg(X_WARNING, "%s: cannot open event manager (%s)\n",
          pInfo->name, evtErrStr(pInfo->fd));
    else {
      xf86Msg(X_ERROR, "%s: cannot open event manager (%s)\n",
          pInfo->name, evtErrStr(pInfo->fd));
      xfree(pMse);
      return FALSE;
    }
  }
  ev_close();
  pInfo->fd = -1;

  /* Process common mouse options (like Emulate3Buttons, etc). */
  pMse->CommonOptions(pInfo);

  /* Setup the local procs. */
  pInfo->device_control = OsMouseProc;
  pInfo->read_input = OsMouseReadInput;
    
  pInfo->flags |= XI86_CONFIGURED;
  return TRUE;
}

_X_EXPORT OSMouseInfoPtr
xf86OSMouseInit (int flags)
{
  OSMouseInfoPtr p;

  p = xcalloc(sizeof(OSMouseInfoRec), 1);
  if (!p)
    return NULL;

  p->SupportedInterfaces        = SupportedInterfaces;
  p->BuiltinNames               = BuiltinNames;
  p->DefaultProtocol            = DefaultProtocol;
  p->CheckProtocol              = CheckProtocol;
  p->PreInit                    = OsMousePreInit;

  return p;
}
