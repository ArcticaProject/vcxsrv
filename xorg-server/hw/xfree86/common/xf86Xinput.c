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
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include <X11/Xatom.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Xinput.h"
#include "XIstubs.h"
#include "xf86Optrec.h"
#include "mipointer.h"
#include "xf86InPriv.h"
#include "compiler.h"
#include "extinit.h"

#ifdef DPMSExtension
#include <X11/extensions/dpmsconst.h>
#include "dpmsproc.h"
#endif

#include "exevents.h"	/* AddInputDevice */
#include "exglobals.h"
#include "eventstr.h"

#include "extnsionst.h"

#include "windowstr.h"	/* screenIsSaved */

#include <stdarg.h>

#include <X11/Xpoll.h>

#include "mi.h"

#include <ptrveloc.h>          /* dix pointer acceleration */
#include <xserver-properties.h>

#ifdef XFreeXDGA
#include "dgaproc.h"
#endif

#include "xkbsrv.h"

#include "os.h"

EventListPtr xf86Events = NULL;

/**
 * Eval config and modify DeviceVelocityRec accordingly
 */
static void
ProcessVelocityConfiguration(DeviceIntPtr pDev, char* devname, pointer list,
                             DeviceVelocityPtr s)
{
    int tempi;
    float tempf;
    Atom float_prop = XIGetKnownProperty(XATOM_FLOAT);
    Atom prop;

    if(!s)
        return;

    /* common settings (available via device properties) */
    tempf = xf86SetRealOption(list, "ConstantDeceleration", 1.0);
    if(tempf > 1.0){
        xf86Msg(X_CONFIG, "%s: (accel) constant deceleration by %.1f\n",
                devname, tempf);
        prop = XIGetKnownProperty(ACCEL_PROP_CONSTANT_DECELERATION);
        XIChangeDeviceProperty(pDev, prop, float_prop, 32,
                               PropModeReplace, 1, &tempf, FALSE);
    }

    tempf = xf86SetRealOption(list, "AdaptiveDeceleration", 1.0);
    if(tempf > 1.0){
        xf86Msg(X_CONFIG, "%s: (accel) adaptive deceleration by %.1f\n",
                devname, tempf);
        prop = XIGetKnownProperty(ACCEL_PROP_ADAPTIVE_DECELERATION);
        XIChangeDeviceProperty(pDev, prop, float_prop, 32,
                               PropModeReplace, 1, &tempf, FALSE);
    }

    /* select profile by number */
    tempi = xf86SetIntOption(list, "AccelerationProfile",
            s->statistics.profile_number);

    prop = XIGetKnownProperty(ACCEL_PROP_PROFILE_NUMBER);
    if (XIChangeDeviceProperty(pDev, prop, XA_INTEGER, 32,
                               PropModeReplace, 1, &tempi, FALSE) == Success)
    {
        xf86Msg(X_CONFIG, "%s: (accel) acceleration profile %i\n", devname,
                tempi);
    } else {
        xf86Msg(X_CONFIG, "%s: (accel) acceleration profile %i is unknown\n",
                devname, tempi);
    }

    /* set scaling */
    tempf = xf86SetRealOption(list, "ExpectedRate", 0);
    prop = XIGetKnownProperty(ACCEL_PROP_VELOCITY_SCALING);
    if(tempf > 0){
        tempf = 1000.0 / tempf;
        XIChangeDeviceProperty(pDev, prop, float_prop, 32,
                               PropModeReplace, 1, &tempf, FALSE);
    }else{
        tempf = xf86SetRealOption(list, "VelocityScale", s->corr_mul);
        XIChangeDeviceProperty(pDev, prop, float_prop, 32,
                               PropModeReplace, 1, &tempf, FALSE);
    }

    tempi = xf86SetIntOption(list, "VelocityTrackerCount", -1);
    if(tempi > 1){
	InitTrackers(s, tempi);
    }

    s->initial_range = xf86SetIntOption(list, "VelocityInitialRange",
                                        s->initial_range);

    s->max_diff = xf86SetRealOption(list, "VelocityAbsDiff", s->max_diff);

    tempf = xf86SetRealOption(list, "VelocityRelDiff", -1);
    if(tempf >= 0){
	xf86Msg(X_CONFIG, "%s: (accel) max rel. velocity difference: %.1f%%\n",
	        devname, tempf*100.0);
	s->max_rel_diff = tempf;
    }

    /*  Configure softening. If const deceleration is used, this is expected
     *  to provide better subpixel information so we enable
     *  softening by default only if ConstantDeceleration is not used
     */
    s->use_softening = xf86SetBoolOption(list, "Softening",
                                         s->const_acceleration == 1.0);

    s->average_accel = xf86SetBoolOption(list, "AccelerationProfileAveraging",
                                         s->average_accel);

    s->reset_time = xf86SetIntOption(list, "VelocityReset", s->reset_time);
}

static void
ApplyAccelerationSettings(DeviceIntPtr dev){
    int scheme;
    DeviceVelocityPtr pVel;
    LocalDevicePtr local = (LocalDevicePtr)dev->public.devicePrivate;
    char* schemeStr;

    if(dev->valuator){
	schemeStr = xf86SetStrOption(local->options, "AccelerationScheme", "");

	scheme = dev->valuator->accelScheme.number;

	if(!xf86NameCmp(schemeStr, "predictable"))
	    scheme = PtrAccelPredictable;

	if(!xf86NameCmp(schemeStr, "lightweight"))
	    scheme = PtrAccelLightweight;

	if(!xf86NameCmp(schemeStr, "none"))
	    scheme = PtrAccelNoOp;

        /* reinit scheme if needed */
        if(dev->valuator->accelScheme.number != scheme){
            if(dev->valuator->accelScheme.AccelCleanupProc){
                dev->valuator->accelScheme.AccelCleanupProc(dev);
            }

            if(InitPointerAccelerationScheme(dev, scheme)){
		xf86Msg(X_CONFIG, "%s: (accel) selected scheme %s/%i\n",
		        local->name, schemeStr, scheme);
	    }else{
        	xf86Msg(X_CONFIG, "%s: (accel) could not init scheme %s\n",
        	        local->name, schemeStr);
        	scheme = dev->valuator->accelScheme.number;
            }
        }else{
            xf86Msg(X_CONFIG, "%s: (accel) keeping acceleration scheme %i\n",
                    local->name, scheme);
        }

        xfree(schemeStr);

        /* process special configuration */
        switch(scheme){
            case PtrAccelPredictable:
                pVel = GetDevicePredictableAccelData(dev);
                ProcessVelocityConfiguration (dev, local->name, local->options,
                                              pVel);
                break;
        }
    }
}

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
void
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
}

/***********************************************************************
 *
 * xf86ActivateDevice --
 *
 *	Initialize an input device.
 *
 * Returns TRUE on success, or FALSE otherwise.
 ***********************************************************************
 */
int
xf86ActivateDevice(LocalDevicePtr local)
{
    DeviceIntPtr	dev;

    if (local->flags & XI86_CONFIGURED) {
        dev = AddInputDevice(serverClient, local->device_control, TRUE);

        if (dev == NULL)
        {
            xf86Msg(X_ERROR, "Too many input devices. Ignoring %s\n",
                    local->name);
            local->dev = NULL;
            return FALSE;
        }

        local->atom = MakeAtom(local->type_name,
                               strlen(local->type_name),
                               TRUE);
        AssignTypeAndName(dev, local->atom, local->name);
        dev->public.devicePrivate = (pointer) local;
        local->dev = dev;      
        
        dev->coreEvents = local->flags & XI86_ALWAYS_CORE; 
        dev->type = SLAVE;
        dev->spriteInfo->spriteOwner = FALSE;

        dev->deviceGrab.ActivateGrab = ActivateKeyboardGrab;
        dev->deviceGrab.DeactivateGrab = DeactivateKeyboardGrab;

        RegisterOtherDevice(dev);
        XkbSetExtension(dev, ProcessKeyboardEvent);

        if (serverGeneration == 1) 
            xf86Msg(X_INFO, "XINPUT: Adding extended input device \"%s\" (type: %s)\n",
                    local->name, local->type_name);
    }

    return TRUE;
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
 ***********************************************************************
 */

void
OpenInputDevice(DeviceIntPtr	dev,
                ClientPtr	client,
                int		*status)
{
    if (!dev->inited)
        ActivateDevice(dev, TRUE);

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
          return BadMatch;
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
AddOtherInputDevices(void)
{
}

/**
 * Create a new input device, activate and enable it.
 *
 * Possible return codes:
 *    BadName .. a bad driver name was supplied.
 *    BadImplementation ... The driver does not have a PreInit function. This
 *                          is a driver bug.
 *    BadMatch .. device initialization failed.
 *    BadAlloc .. too many input devices
 *
 * @param idev The device, already set up with identifier, driver, and the
 * options.
 * @param pdev Pointer to the new device, if Success was reported.
 * @param enable Enable the device after activating it.
 *
 * @return Success or an error code
 */
_X_INTERNAL int
xf86NewInputDevice(IDevPtr idev, DeviceIntPtr *pdev, BOOL enable)
{
    InputDriverPtr drv = NULL;
    InputInfoPtr pInfo = NULL;
    DeviceIntPtr dev = NULL;
    int rval;

    /* Memory leak for every attached device if we don't
     * test if the module is already loaded first */
    drv = xf86LookupInputDriver(idev->driver);
    if (!drv)
        if (xf86LoadOneModule(idev->driver, NULL))
            drv = xf86LookupInputDriver(idev->driver);
    if (!drv) {
        xf86Msg(X_ERROR, "No input driver matching `%s'\n", idev->driver);
        rval = BadName;
        goto unwind;
    }

    if (!drv->PreInit) {
        xf86Msg(X_ERROR,
                "Input driver `%s' has no PreInit function (ignoring)\n",
                drv->driverName);
        rval = BadImplementation;
        goto unwind;
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

    if (!xf86ActivateDevice(pInfo))
    {
        rval = BadAlloc;
        goto unwind;
    }

    dev = pInfo->dev;
    rval = ActivateDevice(dev, TRUE);
    if (rval != Success)
    {
        xf86Msg(X_ERROR, "Couldn't init device \"%s\"\n", idev->identifier);
        RemoveDevice(dev, TRUE);
        goto unwind;
    }

    /* Enable it if it's properly initialised and we're currently in the VT */
    if (enable && dev->inited && dev->startup && xf86Screens[0]->vtSema)
    {
        EnableDevice(dev, TRUE);
        if (!dev->enabled)
        {
            xf86Msg(X_ERROR, "Couldn't init device \"%s\"\n", idev->identifier);
            rval = BadMatch;
            goto unwind;
        }
        /* send enter/leave event, update sprite window */
        CheckMotion(NULL, dev);
    }

    *pdev = dev;
    return Success;

unwind:
    if(pInfo) {
        if(drv->UnInit)
            drv->UnInit(drv, pInfo, 0);
        else
            xf86DeleteInput(pInfo, 0);
    }
    return rval;
}

int
NewInputDeviceRequest (InputOption *options, DeviceIntPtr *pdev)
{
    IDevRec *idev = NULL;
    InputOption *option = NULL;
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
            is_auto = 1;
            if (!xf86Info.autoAddDevices) {
                rval = BadMatch;
                goto unwind;
            }
        }
    }
    if (!idev->driver || !idev->identifier) {
        xf86Msg(X_ERROR, "No input driver/identifier specified (ignoring)\n");
        rval = BadRequest;
        goto unwind;
    }

    if (!idev->identifier) {
        xf86Msg(X_ERROR, "No device identifier specified (ignoring)\n");
        return BadMatch;
    }

    for (option = options; option; option = option->next) {
        /* Steal option key/value strings from the provided list.
         * We need those strings, the InputOption list doesn't. */
        idev->commonOptions = xf86addNewOption(idev->commonOptions,
                                               option->key, option->value);
        option->key = NULL;
        option->value = NULL;
    }

    rval = xf86NewInputDevice(idev, pdev,
                (!is_auto || (is_auto && xf86Info.autoEnableDevices)));
    if (rval == Success)
        return Success;

unwind:
    if (is_auto && !xf86Info.autoAddDevices)
        xf86Msg(X_INFO, "AutoAddDevices is off - not adding device.\n");
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
    InputDriverPtr drv = NULL;
    IDevRec *idev = NULL;
    IDevPtr *it;
    Bool isMaster = IsMaster(pDev);

    if (pInfo) /* need to get these before RemoveDevice */
    {
        drv = pInfo->drv;
        idev = pInfo->conf_idev;
    }

    OsBlockSignals();
    RemoveDevice(pDev, TRUE);

    if (!isMaster && pInfo != NULL)
    {
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
    OsReleaseSignals();
}

/* 
 * convenient functions to post events
 */

void
xf86PostMotionEvent(DeviceIntPtr	device,
                    int			is_absolute,
                    int			first_valuator,
                    int			num_valuators,
                    ...)
{
    va_list var;
    int i = 0;
    static int valuators[MAX_VALUATORS];

    XI_VERIFY_VALUATORS(num_valuators);

    va_start(var, num_valuators);
    for (i = 0; i < num_valuators; i++)
        valuators[i] = va_arg(var, int);
    va_end(var);

    xf86PostMotionEventP(device, is_absolute, first_valuator, num_valuators, valuators);
}

void
xf86PostMotionEventP(DeviceIntPtr	device,
                    int			is_absolute,
                    int			first_valuator,
                    int			num_valuators,
                    int			*valuators)
{
    int i = 0, nevents = 0;
    Bool drag = xf86SendDragEvents(device);
    DeviceEvent *event;
    int flags = 0;

#if XFreeXDGA
    int index;
    int dx = 0, dy = 0;
#endif

    XI_VERIFY_VALUATORS(num_valuators);

    if (is_absolute)
        flags = POINTER_ABSOLUTE;
    else
        flags = POINTER_RELATIVE | POINTER_ACCELERATE;

#if XFreeXDGA
    /* The evdev driver may not always send all axes across. */
    if (num_valuators >= 1 && first_valuator <= 1) {
        if (miPointerGetScreen(device)) {
            index = miPointerGetScreen(device)->myNum;
            if (first_valuator == 0)
            {
                dx = valuators[0];
                if (is_absolute)
                    dx -= device->last.valuators[0];
            }

            if (first_valuator == 1 || num_valuators >= 2)
            {
                dy = valuators[1 - first_valuator];
                if (is_absolute)
                    dy -= device->last.valuators[1];
            }

            if (DGAStealMotionEvent(device, index, dx, dy))
                return;
        }
    }
#endif

    nevents = GetPointerEvents(xf86Events, device, MotionNotify, 0,
                               flags, first_valuator, num_valuators,
                               valuators);

    for (i = 0; i < nevents; i++) {
        event = (DeviceEvent*)((xf86Events + i)->event);
        /* Don't post core motion events for devices not registered to send
         * drag events. */
        if (event->header == ET_Internal &&
            (event->type != ET_Motion || drag)) {
            mieqEnqueue(device, (InternalEvent*)((xf86Events + i)->event));
        }
    }
}

void
xf86PostProximityEvent(DeviceIntPtr	device,
                       int		is_in,
                       int		first_valuator,
                       int		num_valuators,
                       ...)
{
    va_list var;
    int i;
    int valuators[MAX_VALUATORS];

    XI_VERIFY_VALUATORS(num_valuators);

    va_start(var, num_valuators);
    for (i = 0; i < num_valuators; i++)
        valuators[i] = va_arg(var, int);
    va_end(var);

    xf86PostProximityEventP(device, is_in, first_valuator, num_valuators,
			    valuators);

}

void
xf86PostProximityEventP(DeviceIntPtr	device,
                        int		is_in,
                        int		first_valuator,
                        int		num_valuators,
                        int		*valuators)
{
    int i, nevents;

    XI_VERIFY_VALUATORS(num_valuators);

    nevents = GetProximityEvents(xf86Events, device,
                                 is_in ? ProximityIn : ProximityOut, 
                                 first_valuator, num_valuators, valuators);
    for (i = 0; i < nevents; i++)
        mieqEnqueue(device, (InternalEvent*)((xf86Events + i)->event));

}

void
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
    int i = 0;

    XI_VERIFY_VALUATORS(num_valuators);

    va_start(var, num_valuators);
    for (i = 0; i < num_valuators; i++)
        valuators[i] = va_arg(var, int);
    va_end(var);

    xf86PostButtonEventP(device, is_absolute, button, is_down, first_valuator,
			 num_valuators, valuators);

}

void
xf86PostButtonEventP(DeviceIntPtr	device,
                     int		is_absolute,
                     int		button,
                     int		is_down,
                     int		first_valuator,
                     int		num_valuators,
                     int		*valuators)
{
    int i = 0, nevents = 0;
    int flags = 0;

#if XFreeXDGA
    int index;
#endif

    XI_VERIFY_VALUATORS(num_valuators);

    if (is_absolute)
        flags = POINTER_ABSOLUTE;
    else
        flags = POINTER_RELATIVE | POINTER_ACCELERATE;

#if XFreeXDGA
    if (miPointerGetScreen(device)) {
        index = miPointerGetScreen(device)->myNum;
        if (DGAStealButtonEvent(device, index, button, is_down))
            return;
    }
#endif

    nevents = GetPointerEvents(xf86Events, device,
                               is_down ? ButtonPress : ButtonRelease, button,
                               flags, first_valuator, num_valuators, valuators);

    for (i = 0; i < nevents; i++)
        mieqEnqueue(device, (InternalEvent*)((xf86Events + i)->event));

}

void
xf86PostKeyEvent(DeviceIntPtr	device,
                 unsigned int	key_code,
                 int		is_down,
                 int		is_absolute,
                 int		first_valuator,
                 int		num_valuators,
                 ...)
{
    va_list var;
    int i = 0;
    static int valuators[MAX_VALUATORS];

    XI_VERIFY_VALUATORS(num_valuators);

    va_start(var, num_valuators);
    for (i = 0; i < num_valuators; i++)
      valuators[i] = va_arg(var, int);
    va_end(var);

    xf86PostKeyEventP(device, key_code, is_down, is_absolute, first_valuator,
		      num_valuators, valuators);

}

void
xf86PostKeyEventP(DeviceIntPtr	device,
                  unsigned int	key_code,
                  int		is_down,
                  int		is_absolute,
                  int		first_valuator,
                  int		num_valuators,
                  int		*valuators)
{
    int i = 0, nevents = 0;

    XI_VERIFY_VALUATORS(num_valuators);

    if (is_absolute) {
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
        mieqEnqueue(device, (InternalEvent*)((xf86Events + i)->event));
}

void
xf86PostKeyboardEvent(DeviceIntPtr      device,
                      unsigned int      key_code,
                      int               is_down)
{
    xf86PostKeyEventP(device, key_code, is_down, 0, 0, 0, NULL);
}

LocalDevicePtr
xf86FirstLocalDevice(void)
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

int
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
    
    if (X > Sxhigh)
	X = Sxhigh;
    if (X < Sxlow)
	X = Sxlow;
    
    return (X);
}

/*
 * This function checks the given screen against the current screen and
 * makes changes if appropriate. It should be called from an XInput driver's
 * ReadInput function before any events are posted, if the device is screen
 * specific like a touch screen.
 */
void
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


void
xf86InitValuatorAxisStruct(DeviceIntPtr dev, int axnum, Atom label, int minval, int maxval,
			   int resolution, int min_res, int max_res)
{
    if (!dev || !dev->valuator)
        return;

    InitValuatorAxisStruct(dev, axnum, label, minval, maxval, resolution, min_res,
			   max_res);
}

/*
 * Set the valuator values to be in synch with dix/event.c
 * DefineInitialRootWindow().
 */
void
xf86InitValuatorDefaults(DeviceIntPtr dev, int axnum)
{
    if (axnum == 0) {
	dev->valuator->axisVal[0] = screenInfo.screens[0]->width / 2;
        dev->last.valuators[0] = dev->valuator->axisVal[0];
    }
    else if (axnum == 1) {
	dev->valuator->axisVal[1] = screenInfo.screens[0]->height / 2;
        dev->last.valuators[1] = dev->valuator->axisVal[1];
    }

    if(axnum == 0)  /* to prevent double invocation */
	ApplyAccelerationSettings(dev);
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
void
xf86DisableDevice(DeviceIntPtr dev, Bool panic)
{
    if(!panic)
    {
        DisableDevice(dev, TRUE);
    } else
    {
        SendDevicePresenceEvent(dev->id, DeviceUnrecoverable);
        DeleteInputDeviceRequest(dev);
    }
}

/**
 * Reactivate a device. Call this function from the driver if you just found
 * out that the read error wasn't quite that bad after all.
 * Device will be re-activated, and an event sent to the client. 
 */
void
xf86EnableDevice(DeviceIntPtr dev)
{
    EnableDevice(dev, TRUE);
}

/* end of xf86Xinput.c */
