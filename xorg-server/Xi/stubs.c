/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/*
 * stubs.c -- stub routines for the X server side of the XINPUT
 * extension.  This file is mainly to be used only as documentation.
 * There is not much code here, and you can't get a working XINPUT
 * server just using this.
 * The Xvfb server uses this file so it will compile with the same
 * object files as the real X server for a platform that has XINPUT.
 * Xnest could do the same thing.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "inputstr.h"
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "XIstubs.h"
#include "xace.h"

/***********************************************************************
 *
 * Caller:	ProcXCloseDevice
 *
 * Take care of implementation-dependent details of closing a device.
 * Some implementations may actually close the device, others may just
 * remove this clients interest in that device.
 *
 * The default implementation is to do nothing (assume all input devices
 * are initialized during X server initialization and kept open).
 *
 */

void
CloseInputDevice(DeviceIntPtr d, ClientPtr client)
{
}

/***********************************************************************
 *
 * Caller:	ProcXListInputDevices
 *
 * This is the implementation-dependent routine to initialize an input
 * device to the point that information about it can be listed.
 * Some implementations open all input devices when the server is first
 * initialized, and never close them.  Other implementations open only
 * the X pointer and keyboard devices during server initialization,
 * and only open other input devices when some client makes an
 * XOpenDevice request.  If some other process has the device open, the
 * server may not be able to get information about the device to list it.
 *
 * This procedure should be used by implementations that do not initialize
 * all input devices at server startup.  It should do device-dependent
 * initialization for any devices not previously initialized, and call
 * AddInputDevice for each of those devices so that a DeviceIntRec will be
 * created for them.
 *
 * The default implementation is to do nothing (assume all input devices
 * are initialized during X server initialization and kept open).
 * The commented-out sample code shows what you might do if you don't want
 * the default.
 *
 */

void
AddOtherInputDevices(void)
{
    /**********************************************************************
     for each uninitialized device, do something like:

    DeviceIntPtr dev;
    DeviceProc deviceProc;
    pointer private;

    dev = (DeviceIntPtr) AddInputDevice(deviceProc, TRUE);
    dev->public.devicePrivate = private;
    RegisterOtherDevice(dev);
    dev->inited = ((*dev->deviceProc)(dev, DEVICE_INIT) == Success);
    ************************************************************************/

}

/***********************************************************************
 *
 * Caller:	ProcXOpenDevice
 *
 * This is the implementation-dependent routine to open an input device.
 * Some implementations open all input devices when the server is first
 * initialized, and never close them.  Other implementations open only
 * the X pointer and keyboard devices during server initialization,
 * and only open other input devices when some client makes an
 * XOpenDevice request.  This entry point is for the latter type of
 * implementation.
 *
 * If the physical device is not already open, do it here.  In this case,
 * you need to keep track of the fact that one or more clients has the
 * device open, and physically close it when the last client that has
 * it open does an XCloseDevice.
 *
 * The default implementation is to do nothing (assume all input devices
 * are opened during X server initialization and kept open).
 *
 */

void
OpenInputDevice(DeviceIntPtr dev, ClientPtr client, int *status)
{
    *status = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixUseAccess);
}

/****************************************************************************
 *
 * Caller:	ProcXSetDeviceMode
 *
 * Change the mode of an extension device.
 * This function is used to change the mode of a device from reporting
 * relative motion to reporting absolute positional information, and
 * vice versa.
 * The default implementation below is that no such devices are supported.
 *
 */

int
SetDeviceMode(ClientPtr client, DeviceIntPtr dev, int mode)
{
    return BadMatch;
}

/****************************************************************************
 *
 * Caller:	ProcXSetDeviceValuators
 *
 * Set the value of valuators on an extension input device.
 * This function is used to set the initial value of valuators on
 * those input devices that are capable of reporting either relative
 * motion or an absolute position, and allow an initial position to be set.
 * The default implementation below is that no such devices are supported.
 *
 */

int
SetDeviceValuators(ClientPtr client, DeviceIntPtr dev,
		   int *valuators, int first_valuator, int num_valuators)
{
    return BadMatch;
}

/****************************************************************************
 *
 * Caller:	ProcXChangeDeviceControl
 *
 * Change the specified device controls on an extension input device.
 *
 */

int
ChangeDeviceControl(ClientPtr client, DeviceIntPtr dev,
		    xDeviceCtl * control)
{
    switch (control->control) {
    case DEVICE_RESOLUTION:
	return (BadMatch);
    case DEVICE_ABS_CALIB:
    case DEVICE_ABS_AREA:
        return (BadMatch);
    case DEVICE_CORE:
        return (BadMatch);
    default:
	return (BadMatch);
    }
}


/****************************************************************************
 *
 * Caller: configAddDevice (and others)
 *
 * Add a new device with the specified options.
 *
 */
int
NewInputDeviceRequest(InputOption *options, DeviceIntPtr *pdev)
{
    return BadValue;
}

/****************************************************************************
 *
 * Caller: configRemoveDevice (and others)
 *
 * Remove the specified device previously added.
 *
 */
void
DeleteInputDeviceRequest(DeviceIntPtr dev)
{
}
