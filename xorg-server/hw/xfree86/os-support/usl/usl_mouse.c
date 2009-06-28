
/*
 * Copyright 2005 Kean Johnston
 * Copyright 1999 by The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of The XFree86 Project, Inc
 * and Kean Johnston not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * The XFree86 Project, Inc and Kean Johnston make no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE XFREE86 PROJECT, INC AND KEAN JOHNSTON DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THOMAS ROELLm DAVID WEXELBLAT
 * OR KEAN JOHNSTON BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 */

#include "X.h"
#include "compiler.h"
#include "xf86.h"
#include "xf86Xinput.h"
#include "xf86OSmouse.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "mipointer.h"

static int
SupportedInterfaces(void)
{
  return MSE_MISC;
}

static const char *internalNames[] = {
  NULL
};

static const char **
BuiltinNames(void)
{
  return internalNames;
}

static const char *
DefaultProtocol (void)
{
  return "OSMouse";
}

static Bool
CheckProtocol(const char *protocol)
{
  int i;

  for (i = 0; internalNames[i]; i++)
    if (xf86NameCmp(protocol, internalNames[i]) == 0)
      return TRUE;
  return FALSE;
}

static int
OsMouseProc(DeviceIntPtr pPointer, int what)
{
  InputInfoPtr pInfo;
  MouseDevPtr pMse;
  unsigned char map[9];
  int ret;
 
  pInfo = pPointer->public.devicePrivate;
  pMse = pInfo->private;
  pMse->device = pPointer;

  switch (what) {
    case DEVICE_INIT: 
      pPointer->public.on = FALSE;

      for (ret = 0; ret <= 8; ret++)
	map[ret] = ret;

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
      break;

    case DEVICE_ON:
      pMse->lastButtons = 0;
      pMse->emulateState = 0;
      pPointer->public.on = TRUE;
      XqMseOnOff (pInfo, 1);
      break;
      
    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPointer->public.on = FALSE;
	XqMseOnOff (pInfo, 0);
	break;
    }
    return Success;
}

static Bool
OsMousePreInit(InputInfoPtr pInfo, const char *protocol, int flags)
{
  MouseDevPtr pMse;

  pMse = pInfo->private;
  pMse->protocol = protocol;
  xf86Msg(X_CONFIG, "%s: Protocol: %s\n", pInfo->name, protocol);

  /* Collect the options, and process the common options. */
  xf86CollectInputOptions(pInfo, NULL, NULL);
  xf86ProcessCommonOptions(pInfo, pInfo->options);

  pInfo->fd = -1;
#if 0
  /* Make sure we can open the mouse */
  pInfo->fd = open ("/dev/mouse", O_RDONLY | O_NONBLOCK);

  if (pInfo->fd < 0) {
    if (xf86GetAllowMouseOpenFail()) {
      xf86Msg(X_WARNING, "%s: cannot open /dev/mouse (%s)\n",
          pInfo->name, strerror(errno));
    } else {
      xf86Msg(X_ERROR, "%s: cannot open /dev/mouse (%s)\n",
          pInfo->name, strerror(errno));
      xfree(pMse);
      return FALSE;
    }
  }
#endif

  /* Process common mouse options (like Emulate3Buttons, etc). */
  pMse->CommonOptions(pInfo);

  /* Setup the local procs. */
  pInfo->device_control = OsMouseProc;
  pInfo->read_input     = NULL;

  pInfo->flags |= XI86_CONFIGURED;
  return TRUE;
}

_X_EXPORT OSMouseInfoPtr
xf86OSMouseInit(int flags)
{
  OSMouseInfoPtr p;

  p = xcalloc(sizeof(OSMouseInfoRec), 1);
  if (!p)
    return NULL;

  p->SupportedInterfaces = SupportedInterfaces;
  p->BuiltinNames        = BuiltinNames;
  p->DefaultProtocol     = DefaultProtocol;
  p->CheckProtocol       = CheckProtocol;
  p->PreInit             = OsMousePreInit;
  return p;
}

