/*
 * Copyright 2002-2003 Red Hat Inc., Durham, North Carolina.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL RED HAT AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Authors:
 *   Rickard E. (Rik) Faith <faith@redhat.com>
 *
 */

/** \file
 *
 * This file implements support required by the XINPUT extension.
 */

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#define	 NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
#include "inputstr.h"
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "XIstubs.h"

#include "mipointer.h"
#include "dmxinputinit.h"
#include "exevents.h"

/** Change the core keyboard from \a old_dev to \a new_dev.  Currently
 * this is not implemented. */
int ChangeKeyboardDevice(DeviceIntPtr old_dev, DeviceIntPtr new_dev)
{
#if 0
    DMXLocalInputInfoPtr dmxLocalOld = old_dev->public.devicePrivate;
    DMXLocalInputInfoPtr dmxLocalNew = new_dev->public.devicePrivate;
    
                                /* Switch our notion of core keyboard */
    dmxLocalOld->isCore         = 0;
    dmxLocalOld->sendsCore      = dmxLocalOld->savedSendsCore;

    dmxLocalNew->isCore         = 1;
    dmxLocalNew->savedSendsCore = dmxLocalNew->sendsCore;
    dmxLocalNew->sendsCore      = 1;
    dmxLocalCorePointer         = dmxLocalNew;

    RegisterKeyboardDevice(new_dev);
    RegisterOtherDevice(old_dev);
    
    return Success;
#endif
    return BadMatch;
}

/** Change the core pointer from \a old_dev to \a new_dev. */
int ChangePointerDevice(DeviceIntPtr old_dev,
                        DeviceIntPtr new_dev,
                        unsigned char x,
                        unsigned char y)
{
    DMXLocalInputInfoPtr dmxLocalOld = old_dev->public.devicePrivate;
    DMXLocalInputInfoPtr dmxLocalNew = new_dev->public.devicePrivate;
    
    if (x != 0 || y != 1) return BadMatch;

                                /* Make sure the new device can focus */
    InitFocusClassDeviceStruct(old_dev);

                                /* Switch the motion history buffers */
    if (dmxLocalOld->savedMotionProc) {
        old_dev->valuator->GetMotionProc   = dmxLocalOld->savedMotionProc;
        old_dev->valuator->numMotionEvents = dmxLocalOld->savedMotionEvents;
    }
    dmxLocalNew->savedMotionProc       = new_dev->valuator->GetMotionProc;
    dmxLocalNew->savedMotionEvents     = new_dev->valuator->numMotionEvents;
#if 00 /*BP*/
    new_dev->valuator->GetMotionProc   = miPointerGetMotionEvents;
    new_dev->valuator->numMotionEvents = miPointerGetMotionBufferSize();
#else
    new_dev->valuator->GetMotionProc   = GetMotionHistory;
    new_dev->valuator->numMotionEvents = GetMaximumEventsNum();
#endif
                                /* Switch our notion of core pointer */
    dmxLocalOld->isCore         = 0;
    dmxLocalOld->sendsCore      = dmxLocalOld->savedSendsCore;

    dmxLocalNew->isCore         = 1;
    dmxLocalNew->savedSendsCore = dmxLocalNew->sendsCore;
    dmxLocalNew->sendsCore      = 1;
    dmxLocalCorePointer         = dmxLocalNew;
    
    return Success;
}

/** Close the input device.  This is not required by the XINPUT model
 * that DMX uses. */
void CloseInputDevice (DeviceIntPtr d, ClientPtr client)
{
}

/** This is not required by the XINPUT model that DMX uses. */
void AddOtherInputDevices(void)
{
}

/** Open an input device.  This is not required by the XINPUT model that
 * DMX uses. */
void OpenInputDevice (DeviceIntPtr dev, ClientPtr client, int *status)
{
}

/** Set device mode to \a mode.  This is not implemented. */
int SetDeviceMode(ClientPtr client, DeviceIntPtr dev, int mode)
{
    return BadMatch;
}

/** Set device valuators.  This is not implemented. */
int SetDeviceValuators (ClientPtr client,
                        DeviceIntPtr dev,
                        int *valuators,
                        int first_valuator,
                        int num_valuators)
{
    return BadMatch;
}

/** Change device control.  This is not implemented. */
int ChangeDeviceControl(ClientPtr client,
                        DeviceIntPtr dev,
                        xDeviceCtl *control)
{
    return BadMatch;
}
