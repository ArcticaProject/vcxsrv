/*
 * Copyright 1995-1999 by Frederic Lepied, France. <Lepied@XFree86.org>
 *                                                                            
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is  hereby granted without fee, provided that
 * the  above copyright   notice appear  in   all  copies and  that both  that
 * copyright  notice   and   this  permission   notice  appear  in  supporting
 * documentation, and that   the  name of  Frederic   Lepied not  be  used  in
 * advertising or publicity pertaining to distribution of the software without
 * specific,  written      prior  permission.     Frederic  Lepied   makes  no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.                   
 *                                                                            
 * FREDERIC  LEPIED DISCLAIMS ALL   WARRANTIES WITH REGARD  TO  THIS SOFTWARE,
 * INCLUDING ALL IMPLIED   WARRANTIES OF MERCHANTABILITY  AND   FITNESS, IN NO
 * EVENT  SHALL FREDERIC  LEPIED BE   LIABLE   FOR ANY  SPECIAL, INDIRECT   OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA  OR PROFITS, WHETHER  IN  AN ACTION OF  CONTRACT,  NEGLIGENCE OR OTHER
 * TORTIOUS  ACTION, ARISING    OUT OF OR   IN  CONNECTION  WITH THE USE    OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
/*
 * Copyright (c) 2000-2002 by The XFree86 Project, Inc.
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
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/Xfuncproto.h>
#include <X11/Xmd.h>
#ifdef XINPUT
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#endif
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Xinput.h"
#ifdef XINPUT
#include "XIstubs.h"
#include "xf86Optrec.h"
#endif
#include "mipointer.h"
#include "xf86InPriv.h"

#ifdef DPMSExtension
#define DPMS_SERVER
#include <X11/extensions/dpms.h>
#include "dpmsproc.h"
#endif

#include "exevents.h"	/* AddInputDevice */
#include "exglobals.h"

#define EXTENSION_PROC_ARGS void *
#include "extnsionst.h"

#include "windowstr.h"	/* screenIsSaved */

#include <stdarg.h>

#include <X11/Xpoll.h>

#include "mi.h"

#ifdef XFreeXDGA
#include "dgaproc.h"
#endif

xEvent *xf86Events = NULL;

static Bool
xf86SendDragEvents(DeviceIntPtr	device)
{
    LocalDevicePtr local = (LocalDevicePtr) device->public.devicePrivate;
    
    if (device->button && device->button->buttonsDown > 0)
        return (local->flags & XI86_SEND_DRAG_EVENTS);
    else
        return (TRUE);
}

/***********************************************************************
 *
 * xf86ProcessCommonOptions --
 * 
 *	Process global options.
 *
 ***********************************************************************
 */
_X_EXPORT void
xf86ProcessCommonOptions(LocalDevicePtr local,
                         pointer	list)
{
    if (!xf86SetBoolOption(list, "AlwaysCore", 1) ||
        !xf86SetBoolOption(list, "SendCoreEvents", 1) ||
        !xf86SetBoolOption(list, "CorePointer", 1) ||
        !xf86SetBoolOption(list, "CoreKeyboard", 1)) {
        xf86Msg(X_CONFIG, "%s: doesn't report core events\n", local->name);
    } else {
        local->flags |= XI86_ALWAYS_CORE;
        xf86Msg(X_CONFIG, "%s: always reports core events\n", local->name);
    }

    if (xf86SetBoolOption(list, "SendDragEvents", 1)) {
        local->flags |= XI86_SEND_DRAG_EVENTS;
    } else {
        xf86Msg(X_CONFIG, "%s: doesn't report drag events\n", local->name);
    }

    /* Backwards compatibility. */
    local->history_size = GetMotionHistorySize();
    /* Preallocate xEvent store */
    if (!xf86Events)
        xf86Events = (xEvent *)xcalloc(sizeof(xEvent), GetMaximumEventsNum());
    if (!xf86Events)
        FatalError("Couldn't allocate event store\n");
}

/***********************************************************************
 *
 * xf86ActivateDevice --
 * 
 *	Initialize an input device.
 *
 ***********************************************************************
 */
_X_EXPORT void
xf86ActivateDevice(LocalDevicePtr local)
{
    DeviceIntPtr	dev;

    if (local->flags & XI86_CONFIGURED) {
        dev = AddInputDevice(local->device_control, TRUE);

        if (dev == NULL)
            FatalError("Too many input devices");
        
        local->atom = MakeAtom(local->type_name,
                               strlen(local->type_name),
                               TRUE);
        AssignTypeAndName(dev, local->atom, local->name);
        dev->public.devicePrivate = (pointer) local;
        local->dev = dev;      
        
        dev->coreEvents = local->flags & XI86_ALWAYS_CORE;
        RegisterOtherDevice(dev);

#ifdef XKB
        if (!noXkbExtension)
            XkbSetExtension(dev, ProcessKeyboardEvent);
#endif

        if (serverGeneration == 1) 
            xf86Msg(X_INFO, "XINPUT: Adding extended input device \"%s\" (type: %s)\n",
                    local->name, local->type_name);
    }
}


#ifdef XINPUT
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
 ***********************************************************************
 */

void
OpenInputDevice(DeviceIntPtr	dev,
                ClientPtr	client,
                int		*status)
{
    if (!dev->inited)
        ActivateDevice(dev);

    *status = Success;
}

void
CloseInputDevice(DeviceIntPtr dev,
                 ClientPtr client)
{
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
 ***********************************************************************
 */

int
SetDeviceMode (ClientPtr client, DeviceIntPtr dev, int mode)
{
  LocalDevicePtr        local = (LocalDevicePtr)dev->public.devicePrivate;

  if (local->switch_mode) {
    return (*local->switch_mode)(client, dev, mode);
  }
  else
    return BadMatch;
}


/***********************************************************************
 *
 * Caller:	ProcXSetDeviceValuators
 *
 * Set the value of valuators on an extension input device.
 * This function is used to set the initial value of valuators on
 * those input devices that are capable of reporting either relative
 * motion or an absolute position, and allow an initial position to be set.
 * The default implementation below is that no such devices are supported.
 *
 ***********************************************************************
 */

int
SetDeviceValuators (ClientPtr client, DeviceIntPtr dev, int *valuators,
                    int first_valuator, int num_valuators)
{
    LocalDevicePtr local = (LocalDevicePtr) dev->public.devicePrivate;

    if (local->set_device_valuators)
	return (*local->set_device_valuators)(local, valuators, first_valuator,
					      num_valuators);

    return BadMatch;
}


/***********************************************************************
 *
 * Caller:	ProcXChangeDeviceControl
 *
 * Change the specified device controls on an extension input device.
 *
 ***********************************************************************
 */

int
ChangeDeviceControl (ClientPtr client, DeviceIntPtr dev, xDeviceCtl *control)
{
  LocalDevicePtr        local = (LocalDevicePtr)dev->public.devicePrivate;

  if (!local->control_proc) {
      switch (control->control) {
      case DEVICE_CORE:
      case DEVICE_RESOLUTION:
      case DEVICE_ABS_CALIB:
      case DEVICE_ABS_AREA:
      case DEVICE_ENABLE:
        return Success;
      default:
        return BadMatch;
      }
  }
  else {
      return (*local->control_proc)(local, control);
  }
}

void
AddOtherInputDevices()
{
}
#endif

int
NewInputDeviceRequest (InputOption *options, DeviceIntPtr *pdev)
{
    IDevRec *idev = NULL;
    InputDriverPtr drv = NULL;
    InputInfoPtr pInfo = NULL;
    InputOption *option = NULL;
    DeviceIntPtr dev = NULL;
    int rval = Success;
    int is_auto = 0;

    idev = xcalloc(sizeof(*idev), 1);
    if (!idev)
        return BadAlloc;

    for (option = options; option; option = option->next) {
        if (strcasecmp(option->key, "driver") == 0) {
            if (idev->driver) {
                rval = BadRequest;
                goto unwind;
            }
            /* Memory leak for every attached device if we don't
             * test if the module is already loaded first */
            drv = xf86LookupInputDriver(option->value);
            if (!drv)
                if (xf86LoadOneModule(option->value, NULL))
                    drv = xf86LookupInputDriver(option->value);
            if (!drv) {
                xf86Msg(X_ERROR, "No input driver matching `%s'\n",
                        option->value);
                rval = BadName;
                goto unwind;
            }
            idev->driver = xstrdup(option->value);
            if (!idev->driver) {
                rval = BadAlloc;
                goto unwind;
            }
        }

        if (strcasecmp(option->key, "name") == 0 ||
            strcasecmp(option->key, "identifier") == 0) {
            if (idev->identifier) {
                rval = BadRequest;
                goto unwind;
            }
            idev->identifier = xstrdup(option->value);
            if (!idev->identifier) {
                rval = BadAlloc;
                goto unwind;
            }
        }

        /* Right now, the only automatic config we know of is HAL. */
        if (strcmp(option->key, "_source") == 0 &&
            strcmp(option->value, "server/hal") == 0) {
            if (!xf86Info.autoAddDevices) {
                rval = BadMatch;
                goto unwind;
            }

            is_auto = 1;
        }
    }
    if (!idev->driver || !idev->identifier) {
        xf86Msg(X_ERROR, "No input driver/identifier specified (ignoring)\n");
        rval = BadRequest;
        goto unwind;
    }

    if (!drv->PreInit) {
        xf86Msg(X_ERROR,
                "Input driver `%s' has no PreInit function (ignoring)\n",
                drv->driverName);
        rval = BadImplementation;
        goto unwind;
    }

    for (option = options; option; option = option->next) {
        /* Steal option key/value strings from the provided list.
         * We need those strings, the InputOption list doesn't. */
        idev->commonOptions = xf86addNewOption(idev->commonOptions,
                                               option->key, option->value);
        option->key = NULL;
        option->value = NULL;
    }

    pInfo = drv->PreInit(drv, idev, 0);

    if (!pInfo) {
        xf86Msg(X_ERROR, "PreInit returned NULL for \"%s\"\n", idev->identifier);
        rval = BadMatch;
        goto unwind;
    }
    else if (!(pInfo->flags & XI86_CONFIGURED)) {
        xf86Msg(X_ERROR, "PreInit failed for input device \"%s\"\n",
                idev->identifier);
        rval = BadMatch;
        goto unwind;
    }

    xf86ActivateDevice(pInfo);

    dev = pInfo->dev;
    ActivateDevice(dev);
    /* Enable it if it's properly initialised, we're currently in the VT, and
     * either it's a manual request, or we're automatically enabling devices. */
    if (dev->inited && dev->startup && xf86Screens[0]->vtSema &&
        (!is_auto || xf86Info.autoEnableDevices))
        EnableDevice(dev);

    *pdev = dev;
    return Success;

unwind:
    if(pInfo) {
        if(drv->UnInit)
            drv->UnInit(drv, pInfo, 0);
        else
            xf86DeleteInput(pInfo, 0);
    }
    if(idev->driver)
        xfree(idev->driver);
    if(idev->identifier)
        xfree(idev->identifier);
    xf86optionListFree(idev->commonOptions);
    xfree(idev);
    return rval;
}

void
DeleteInputDeviceRequest(DeviceIntPtr pDev)
{
    LocalDevicePtr pInfo = (LocalDevicePtr) pDev->public.devicePrivate;
    InputDriverPtr drv;
    IDevRec *idev;
    BOOL found;
    IDevPtr *it;

    if (pInfo) /* need to get these before RemoveDevice */
    {
        drv = pInfo->drv;
        idev = pInfo->conf_idev;
    }
    RemoveDevice(pDev);

    if (!pInfo) /* VCP and VCK */
        return;

    if(drv->UnInit)
        drv->UnInit(drv, pInfo, 0);
    else
        xf86DeleteInput(pInfo, 0);

    /* devices added through HAL aren't in the config layout */
    it = xf86ConfigLayout.inputs;
    while(*it && *it != idev)
        it++;

    if (!(*it)) /* end of list, not in the layout */
    {
        xfree(idev->driver);
        xfree(idev->identifier);
        xf86optionListFree(idev->commonOptions);
        xfree(idev);
    }
}

/* 
 * convenient functions to post events
 */

#define MAX_VALUATORS 36 /* XXX from comment in dix/getevents.c */

_X_EXPORT void
xf86PostMotionEvent(DeviceIntPtr	device,
                    int			is_absolute,
                    int			first_valuator,
                    int			num_valuators,
                    ...)
{
    va_list var;
    int i = 0;
    static int valuators[MAX_VALUATORS];

    if (num_valuators > MAX_VALUATORS) {
	xf86Msg(X_ERROR, "xf86PostMotionEvent: num_valuator %d"
	    " is greater than MAX_VALUATORS\n", num_valuators);
	return;
    }

    va_start(var, num_valuators);
    for (i = 0; i < num_valuators; i++)
        valuators[i] = va_arg(var, int);
    va_end(var);

    xf86PostMotionEventP(device, is_absolute, first_valuator, num_valuators, valuators);
}

_X_EXPORT void
xf86PostMotionEventP(DeviceIntPtr	device,
                    int			is_absolute,
                    int			first_valuator,
                    int			num_valuators,
                    int			*valuators)
{
    int i = 0, nevents = 0;
    int dx, dy;
    Bool drag = xf86SendDragEvents(device);
    xEvent *xE = NULL;
    int index;
    int flags = 0;

    if (num_valuators > MAX_VALUATORS) {
	xf86Msg(X_ERROR, "xf86PostMotionEvent: num_valuator %d"
	    " is greater than MAX_VALUATORS\n", num_valuators);
	return;
    }

    if (is_absolute)
        flags = POINTER_ABSOLUTE;
    else
        flags = POINTER_RELATIVE | POINTER_ACCELERATE;

#if XFreeXDGA
    if (first_valuator == 0 && num_valuators >= 2) {
        if (miPointerGetScreen(inputInfo.pointer)) {
            index = miPointerGetScreen(inputInfo.pointer)->myNum;
            if (is_absolute) {
                dx = valuators[0] - device->valuator->lastx;
                dy = valuators[1] - device->valuator->lasty;
            }
            else {
                dx = valuators[0];
                dy = valuators[1];
            }
            if (DGAStealMotionEvent(index, dx, dy))
                return;
        }
    }
#endif

    if (!xf86Events)
        FatalError("Didn't allocate event store\n");

    nevents = GetPointerEvents(xf86Events, device, MotionNotify, 0,
                               flags, first_valuator, num_valuators,
                               valuators);

    for (i = 0; i < nevents; i++) {
        xE = xf86Events + i;
        /* Don't post core motion events for devices not registered to send
         * drag events. */
        if (xE->u.u.type != MotionNotify || drag) {
            mieqEnqueue(device, xf86Events + i);
        }
    }
}

_X_EXPORT void
xf86PostProximityEvent(DeviceIntPtr	device,
                       int		is_in,
                       int		first_valuator,
                       int		num_valuators,
                       ...)
{
    va_list var;
    int i, nevents;
    int valuators[MAX_VALUATORS];


    if (num_valuators > MAX_VALUATORS) {
	xf86Msg(X_ERROR, "xf86PostMotionEvent: num_valuator %d"
	    " is greater than MAX_VALUATORS\n", num_valuators);
	return;
    }

    va_start(var, num_valuators);
    for (i = 0; i < num_valuators; i++)
        valuators[i] = va_arg(var, int);
    va_end(var);

    if (!xf86Events)
        FatalError("Didn't allocate event store\n");

    nevents = GetProximityEvents(xf86Events, device,
                                 is_in ? ProximityIn : ProximityOut, 
                                 first_valuator, num_valuators, valuators);
    for (i = 0; i < nevents; i++)
        mieqEnqueue(device, xf86Events + i);

}

_X_EXPORT void
xf86PostButtonEvent(DeviceIntPtr	device,
                    int			is_absolute,
                    int			button,
                    int			is_down,
                    int			first_valuator,
                    int			num_valuators,
                    ...)
{
    va_list var;
    int valuators[MAX_VALUATORS];
    int i = 0, nevents = 0;
    int index;

#if XFreeXDGA
    if (miPointerGetScreen(inputInfo.pointer)) {
        index = miPointerGetScreen(inputInfo.pointer)->myNum;
        if (DGAStealButtonEvent(index, button, is_down))
            return;
    }
#endif
    if (num_valuators > MAX_VALUATORS) {
	xf86Msg(X_ERROR, "xf86PostMotionEvent: num_valuator %d"
	    " is greater than MAX_VALUATORS\n", num_valuators);
	return;
    }
    
    va_start(var, num_valuators);
    for (i = 0; i < num_valuators; i++)
        valuators[i] = va_arg(var, int);
    va_end(var);

    if (!xf86Events)
        FatalError("Didn't allocate event store\n");

    nevents = GetPointerEvents(xf86Events, device,
                               is_down ? ButtonPress : ButtonRelease, button,
                               is_absolute ? POINTER_ABSOLUTE :
                                             POINTER_RELATIVE,
                               first_valuator, num_valuators, valuators);

    for (i = 0; i < nevents; i++)
        mieqEnqueue(device, xf86Events + i);
}

_X_EXPORT void
xf86PostKeyEvent(DeviceIntPtr	device,
                 unsigned int	key_code,
                 int		is_down,
                 int		is_absolute,
                 int		first_valuator,
                 int		num_valuators,
                 ...)
{
    va_list var;
    int i = 0, nevents = 0;
    static int valuators[MAX_VALUATORS];

    /* instil confidence in the user */
    DebugF("this function has never been tested properly.  if things go quite "
           "badly south after this message, then xf86PostKeyEvent is "
           "broken.\n");

    if (num_valuators > MAX_VALUATORS) {
	xf86Msg(X_ERROR, "xf86PostMotionEvent: num_valuator %d"
	    " is greater than MAX_VALUATORS\n", num_valuators);
	return;
    }

    if (!xf86Events)
        FatalError("Didn't allocate event store\n");

    if (is_absolute) {
        va_start(var, num_valuators);
        for (i = 0; i < num_valuators; i++)
            valuators[i] = va_arg(var, int);
        va_end(var);

        nevents = GetKeyboardValuatorEvents(xf86Events, device,
                                            is_down ? KeyPress : KeyRelease,
                                            key_code, first_valuator,
                                            num_valuators, valuators);
    }
    else {
        nevents = GetKeyboardEvents(xf86Events, device,
                                    is_down ? KeyPress : KeyRelease,
                                    key_code);
    }

    for (i = 0; i < nevents; i++)
        mieqEnqueue(device, xf86Events + i);
}

_X_EXPORT void
xf86PostKeyboardEvent(DeviceIntPtr      device,
                      unsigned int      key_code,
                      int               is_down)
{
    int nevents = 0, i = 0;
    int index;

#if XFreeXDGA
    if (miPointerGetScreen(inputInfo.pointer)) {
        index = miPointerGetScreen(inputInfo.pointer)->myNum;
        if (DGAStealKeyEvent(index, key_code, is_down))
            return;
    }
#endif

    if (!xf86Events)
        FatalError("Didn't allocate event store\n");

    nevents = GetKeyboardEvents(xf86Events, device,
                                is_down ? KeyPress : KeyRelease, key_code);

    for (i = 0; i < nevents; i++)
        mieqEnqueue(device, xf86Events + i);
}

_X_EXPORT LocalDevicePtr
xf86FirstLocalDevice()
{
    return xf86InputDevs;
}

/* 
 * Cx     - raw data from touch screen
 * Sxhigh - scaled highest dimension
 *          (remember, this is of rows - 1 because of 0 origin)
 * Sxlow  - scaled lowest dimension
 * Rxhigh - highest raw value from touch screen calibration
 * Rxlow  - lowest raw value from touch screen calibration
 *
 * This function is the same for X or Y coordinates.
 * You may have to reverse the high and low values to compensate for
 * different orgins on the touch screen vs X.
 */

_X_EXPORT int
xf86ScaleAxis(int	Cx,
              int	Sxhigh,
              int	Sxlow,
              int	Rxhigh,
              int	Rxlow )
{
    int X;
    int dSx = Sxhigh - Sxlow;
    int dRx = Rxhigh - Rxlow;

    dSx = Sxhigh - Sxlow;
    if (dRx) {
	X = ((dSx * (Cx - Rxlow)) / dRx) + Sxlow;
    }
    else {
	X = 0;
	ErrorF ("Divide by Zero in xf86ScaleAxis");
    }
    
    if (X > Sxlow)
	X = Sxlow;
    if (X < Sxhigh)
	X = Sxhigh;
    
    return (X);
}

/*
 * This function checks the given screen against the current screen and
 * makes changes if appropriate. It should be called from an XInput driver's
 * ReadInput function before any events are posted, if the device is screen
 * specific like a touch screen.
 */
_X_EXPORT void
xf86XInputSetScreen(LocalDevicePtr	local,
		    int			screen_number,
		    int			x,
		    int			y)
{
    if (miPointerGetScreen(local->dev) !=
          screenInfo.screens[screen_number]) {
	miPointerSetScreen(local->dev, screen_number, x, y);
    }
}


_X_EXPORT void
xf86InitValuatorAxisStruct(DeviceIntPtr dev, int axnum, int minval, int maxval,
			   int resolution, int min_res, int max_res)
{
    if (!dev || !dev->valuator)
        return;

    InitValuatorAxisStruct(dev, axnum, minval, maxval, resolution, min_res,
			   max_res);
}

/*
 * Set the valuator values to be in synch with dix/event.c
 * DefineInitialRootWindow().
 */
_X_EXPORT void
xf86InitValuatorDefaults(DeviceIntPtr dev, int axnum)
{
    if (axnum == 0) {
	dev->valuator->axisVal[0] = screenInfo.screens[0]->width / 2;
        dev->valuator->lastx = dev->valuator->axisVal[0];
    }
    else if (axnum == 1) {
	dev->valuator->axisVal[1] = screenInfo.screens[0]->height / 2;
        dev->valuator->lasty = dev->valuator->axisVal[1];
    }
}


/**
 * Deactivate a device. Call this function from the driver if you receive a
 * read error or something else that spoils your day.
 * Device will be moved to the off_devices list, but it will still be there
 * until you really clean up after it.
 * Notifies the client about an inactive device.
 * 
 * @param panic True if device is unrecoverable and needs to be removed.
 */
_X_EXPORT void
xf86DisableDevice(DeviceIntPtr dev, Bool panic)
{
    devicePresenceNotify ev;
    DeviceIntRec dummyDev;

    if(!panic)
    {
        DisableDevice(dev);
    } else
    {
        ev.type = DevicePresenceNotify;
        ev.time = currentTime.milliseconds;
        ev.devchange = DeviceUnrecoverable;
        ev.deviceid = dev->id;
        dummyDev.id = 0;
        SendEventToAllWindows(&dummyDev, DevicePresenceNotifyMask,
                (xEvent *) &ev, 1);

        DeleteInputDeviceRequest(dev);
    }
}

/**
 * Reactivate a device. Call this function from the driver if you just found
 * out that the read error wasn't quite that bad after all.
 * Device will be re-activated, and an event sent to the client. 
 */
_X_EXPORT void
xf86EnableDevice(DeviceIntPtr dev)
{
    EnableDevice(dev);
}

/* end of xf86Xinput.c */
