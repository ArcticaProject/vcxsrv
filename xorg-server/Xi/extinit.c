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

/********************************************************************
 *
 *  Dispatch routines and initialization routines for the X input extension.
 *
 */

#define	 NUMTYPES 15

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "inputstr.h"
#include "gcstruct.h"	/* pointer for extnsionst.h */
#include "extnsionst.h"	/* extension entry   */
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include <X11/extensions/geproto.h>
#include "geext.h" /* extension interfaces for ge */

#include "dixevents.h"
#include "exevents.h"
#include "extinit.h"
#include "exglobals.h"
#include "swaprep.h"
#include "registry.h"
#include "privates.h"

/* modules local to Xi */
#include "allowev.h"
#include "chgdctl.h"
#include "chgfctl.h"
#include "chgkbd.h"
#include "chgprop.h"
#include "chgptr.h"
#include "closedev.h"
#include "devbell.h"
#include "getbmap.h"
#include "getbmap.h"
#include "getdctl.h"
#include "getfctl.h"
#include "getfocus.h"
#include "getkmap.h"
#include "getmmap.h"
#include "getprop.h"
#include "getselev.h"
#include "getvers.h"
#include "getvers.h"
#include "grabdev.h"
#include "grabdevb.h"
#include "grabdevk.h"
#include "gtmotion.h"
#include "listdev.h"
#include "opendev.h"
#include "queryst.h"
#include "selectev.h"
#include "sendexev.h"
#include "chgkmap.h"
#include "setbmap.h"
#include "setdval.h"
#include "setfocus.h"
#include "setmmap.h"
#include "setmode.h"
#include "ungrdev.h"
#include "ungrdevb.h"
#include "ungrdevk.h"
#include "xiproperty.h"


static Mask lastExtEventMask = 1;
int ExtEventIndex;
Mask ExtValidMasks[EMASKSIZE];
Mask ExtExclusiveMasks[EMASKSIZE];

static struct dev_type
{
    Atom type;
    char *name;
} dev_type[] = {
    {
    0, XI_KEYBOARD}, {
    0, XI_MOUSE}, {
    0, XI_TABLET}, {
    0, XI_TOUCHSCREEN}, {
    0, XI_TOUCHPAD}, {
    0, XI_BARCODE}, {
    0, XI_BUTTONBOX}, {
    0, XI_KNOB_BOX}, {
    0, XI_ONE_KNOB}, {
    0, XI_NINE_KNOB}, {
    0, XI_TRACKBALL}, {
    0, XI_QUADRATURE}, {
    0, XI_ID_MODULE}, {
    0, XI_SPACEBALL}, {
    0, XI_DATAGLOVE}, {
    0, XI_EYETRACKER}, {
    0, XI_CURSORKEYS}, {
0, XI_FOOTMOUSE}};

CARD8 event_base[numInputClasses];
XExtEventInfo EventInfo[32];

/**
 * Dispatch vector. Functions defined in here will be called when the matching
 * request arrives.
 */
static int (*ProcIVector[])(ClientPtr) = {
        NULL,                                   /*  0 */
	ProcXGetExtensionVersion,               /*  1 */
	ProcXListInputDevices,                  /*  2 */
	ProcXOpenDevice,                        /*  3 */
	ProcXCloseDevice,                       /*  4 */
	ProcXSetDeviceMode,                     /*  5 */
	ProcXSelectExtensionEvent,              /*  6 */
	ProcXGetSelectedExtensionEvents,        /*  7 */
	ProcXChangeDeviceDontPropagateList,     /*  8 */
	ProcXGetDeviceDontPropagateList,        /*  9 */
	ProcXGetDeviceMotionEvents,             /* 10 */
	ProcXChangeKeyboardDevice,              /* 11 */
	ProcXChangePointerDevice,               /* 12 */
	ProcXGrabDevice,                        /* 13 */
	ProcXUngrabDevice,                      /* 14 */
	ProcXGrabDeviceKey,                     /* 15 */
	ProcXUngrabDeviceKey,                   /* 16 */
	ProcXGrabDeviceButton,                  /* 17 */
	ProcXUngrabDeviceButton,                /* 18 */
	ProcXAllowDeviceEvents,                 /* 19 */
	ProcXGetDeviceFocus,                    /* 20 */
	ProcXSetDeviceFocus,                    /* 21 */
	ProcXGetFeedbackControl,                /* 22 */
	ProcXChangeFeedbackControl,             /* 23 */
	ProcXGetDeviceKeyMapping,               /* 24 */
	ProcXChangeDeviceKeyMapping,            /* 25 */
	ProcXGetDeviceModifierMapping,          /* 26 */
	ProcXSetDeviceModifierMapping,          /* 27 */
	ProcXGetDeviceButtonMapping,            /* 28 */
	ProcXSetDeviceButtonMapping,            /* 29 */
	ProcXQueryDeviceState,                  /* 30 */
	ProcXSendExtensionEvent,                /* 31 */
	ProcXDeviceBell,                        /* 32 */
	ProcXSetDeviceValuators,                /* 33 */
	ProcXGetDeviceControl,                  /* 34 */
	ProcXChangeDeviceControl,               /* 35 */
        /* XI 1.5 */
        ProcXListDeviceProperties,              /* 36 */
        ProcXChangeDeviceProperty,              /* 37 */
        ProcXDeleteDeviceProperty,              /* 38 */
        ProcXGetDeviceProperty                  /* 39 */
};

/* For swapped clients */
static int (*SProcIVector[])(ClientPtr) = {
        NULL,                                    /*  0 */
	SProcXGetExtensionVersion,               /*  1 */
	SProcXListInputDevices,                  /*  2 */
	SProcXOpenDevice,                        /*  3 */
	SProcXCloseDevice,                       /*  4 */
	SProcXSetDeviceMode,                     /*  5 */
	SProcXSelectExtensionEvent,              /*  6 */
	SProcXGetSelectedExtensionEvents,        /*  7 */
	SProcXChangeDeviceDontPropagateList,     /*  8 */
	SProcXGetDeviceDontPropagateList,        /*  9 */
	SProcXGetDeviceMotionEvents,             /* 10 */
	SProcXChangeKeyboardDevice,              /* 11 */
	SProcXChangePointerDevice,               /* 12 */
	SProcXGrabDevice,                        /* 13 */
	SProcXUngrabDevice,                      /* 14 */
	SProcXGrabDeviceKey,                     /* 15 */
	SProcXUngrabDeviceKey,                   /* 16 */
	SProcXGrabDeviceButton,                  /* 17 */
	SProcXUngrabDeviceButton,                /* 18 */
	SProcXAllowDeviceEvents,                 /* 19 */
	SProcXGetDeviceFocus,                    /* 20 */
	SProcXSetDeviceFocus,                    /* 21 */
	SProcXGetFeedbackControl,                /* 22 */
	SProcXChangeFeedbackControl,             /* 23 */
	SProcXGetDeviceKeyMapping,               /* 24 */
	SProcXChangeDeviceKeyMapping,            /* 25 */
	SProcXGetDeviceModifierMapping,          /* 26 */
	SProcXSetDeviceModifierMapping,          /* 27 */
	SProcXGetDeviceButtonMapping,            /* 28 */
	SProcXSetDeviceButtonMapping,            /* 29 */
	SProcXQueryDeviceState,                  /* 30 */
	SProcXSendExtensionEvent,                /* 31 */
	SProcXDeviceBell,                        /* 32 */
	SProcXSetDeviceValuators,                /* 33 */
	SProcXGetDeviceControl,                  /* 34 */
	SProcXChangeDeviceControl,               /* 35 */
        SProcXListDeviceProperties,              /* 36 */
        SProcXChangeDeviceProperty,              /* 37 */
        SProcXDeleteDeviceProperty,              /* 38 */
        SProcXGetDeviceProperty                  /* 39 */
};

/*****************************************************************
 *
 * Globals referenced elsewhere in the server.
 *
 */

int IReqCode = 0;
int IEventBase = 0;
int BadDevice = 0;
static int BadEvent = 1;
int BadMode = 2;
int DeviceBusy = 3;
int BadClass = 4;

Mask DevicePointerMotionMask;
Mask DevicePointerMotionHintMask;
Mask DeviceFocusChangeMask;
Mask DeviceStateNotifyMask;
static Mask ChangeDeviceNotifyMask;
Mask DeviceMappingNotifyMask;
Mask DeviceOwnerGrabButtonMask;
Mask DeviceButtonGrabMask;
Mask DeviceButtonMotionMask;
Mask DevicePresenceNotifyMask;
Mask DevicePropertyNotifyMask;

int DeviceValuator;
int DeviceKeyPress;
int DeviceKeyRelease;
int DeviceButtonPress;
int DeviceButtonRelease;
int DeviceMotionNotify;
int DeviceFocusIn;
int DeviceFocusOut;
int ProximityIn;
int ProximityOut;
int DeviceStateNotify;
int DeviceKeyStateNotify;
int DeviceButtonStateNotify;
int DeviceMappingNotify;
int ChangeDeviceNotify;
int DevicePresenceNotify;
int DevicePropertyNotify;

int RT_INPUTCLIENT;

/*****************************************************************
 *
 * Externs defined elsewhere in the X server.
 *
 */

extern XExtensionVersion AllExtensionVersions[];


Mask PropagateMask[MAXDEVICES];

/*****************************************************************
 *
 * Versioning support
 *
 */

static int XIClientPrivateKeyIndex;
DevPrivateKey XIClientPrivateKey = &XIClientPrivateKeyIndex;

static XExtensionVersion thisversion = { XI_Present,
    XI_Add_DeviceProperties_Major,
    XI_Add_DeviceProperties_Minor
};


/*****************************************************************
 *
 * Declarations of local routines.
 *
 */

static void
XIClientCallback(CallbackListPtr        *list,
                 pointer                closure,
                 pointer                data)
{
    NewClientInfoRec *clientinfo = (NewClientInfoRec*)data;
    ClientPtr pClient = clientinfo->client;
    XIClientPtr pXIClient;

    pXIClient = dixLookupPrivate(&pClient->devPrivates, XIClientPrivateKey);
    pXIClient->major_version = 0;
    pXIClient->minor_version = 0;
}

/*************************************************************************
 *
 * ProcIDispatch - main dispatch routine for requests to this extension.
 * This routine is used if server and client have the same byte ordering.
 *
 */

static int
ProcIDispatch(ClientPtr client)
{
    REQUEST(xReq);
    if (stuff->data > IREQUESTS || !ProcIVector[stuff->data])
        return BadRequest;

    return (*ProcIVector[stuff->data])(client);
}

/*******************************************************************************
 *
 * SProcXDispatch
 *
 * Main swapped dispatch routine for requests to this extension.
 * This routine is used if server and client do not have the same byte ordering.
 *
 */

static int
SProcIDispatch(ClientPtr client)
{
    REQUEST(xReq);
    if (stuff->data > IREQUESTS || !SProcIVector[stuff->data])
        return BadRequest;

    return (*SProcIVector[stuff->data])(client);
}

/**********************************************************************
 *
 * SReplyIDispatch
 * Swap any replies defined in this extension.
 *
 */

static void
SReplyIDispatch(ClientPtr client, int len, xGrabDeviceReply * rep)
					/* All we look at is the type field */
{	/* This is common to all replies    */
    if (rep->RepType == X_GetExtensionVersion)
	SRepXGetExtensionVersion(client, len,
				 (xGetExtensionVersionReply *) rep);
    else if (rep->RepType == X_ListInputDevices)
	SRepXListInputDevices(client, len, (xListInputDevicesReply *) rep);
    else if (rep->RepType == X_OpenDevice)
	SRepXOpenDevice(client, len, (xOpenDeviceReply *) rep);
    else if (rep->RepType == X_SetDeviceMode)
	SRepXSetDeviceMode(client, len, (xSetDeviceModeReply *) rep);
    else if (rep->RepType == X_GetSelectedExtensionEvents)
	SRepXGetSelectedExtensionEvents(client, len,
					(xGetSelectedExtensionEventsReply *)
					rep);
    else if (rep->RepType == X_GetDeviceDontPropagateList)
	SRepXGetDeviceDontPropagateList(client, len,
					(xGetDeviceDontPropagateListReply *)
					rep);
    else if (rep->RepType == X_GetDeviceMotionEvents)
	SRepXGetDeviceMotionEvents(client, len,
				   (xGetDeviceMotionEventsReply *) rep);
    else if (rep->RepType == X_GrabDevice)
	SRepXGrabDevice(client, len, (xGrabDeviceReply *) rep);
    else if (rep->RepType == X_GetDeviceFocus)
	SRepXGetDeviceFocus(client, len, (xGetDeviceFocusReply *) rep);
    else if (rep->RepType == X_GetFeedbackControl)
	SRepXGetFeedbackControl(client, len, (xGetFeedbackControlReply *) rep);
    else if (rep->RepType == X_GetDeviceKeyMapping)
	SRepXGetDeviceKeyMapping(client, len,
				 (xGetDeviceKeyMappingReply *) rep);
    else if (rep->RepType == X_GetDeviceModifierMapping)
	SRepXGetDeviceModifierMapping(client, len,
				      (xGetDeviceModifierMappingReply *) rep);
    else if (rep->RepType == X_SetDeviceModifierMapping)
	SRepXSetDeviceModifierMapping(client, len,
				      (xSetDeviceModifierMappingReply *) rep);
    else if (rep->RepType == X_GetDeviceButtonMapping)
	SRepXGetDeviceButtonMapping(client, len,
				    (xGetDeviceButtonMappingReply *) rep);
    else if (rep->RepType == X_SetDeviceButtonMapping)
	SRepXSetDeviceButtonMapping(client, len,
				    (xSetDeviceButtonMappingReply *) rep);
    else if (rep->RepType == X_QueryDeviceState)
	SRepXQueryDeviceState(client, len, (xQueryDeviceStateReply *) rep);
    else if (rep->RepType == X_SetDeviceValuators)
	SRepXSetDeviceValuators(client, len, (xSetDeviceValuatorsReply *) rep);
    else if (rep->RepType == X_GetDeviceControl)
	SRepXGetDeviceControl(client, len, (xGetDeviceControlReply *) rep);
    else if (rep->RepType == X_ChangeDeviceControl)
	SRepXChangeDeviceControl(client, len,
				 (xChangeDeviceControlReply *) rep);
    else if (rep->RepType == X_ListDeviceProperties)
        SRepXListDeviceProperties(client, len, (xListDevicePropertiesReply*)rep);
    else if (rep->RepType == X_GetDeviceProperty)
	SRepXGetDeviceProperty(client, len, (xGetDevicePropertyReply *) rep);
    else {
	FatalError("XINPUT confused sending swapped reply");
    }
}

/************************************************************************
 *
 * This function swaps the DeviceValuator event.
 *
 */

static void
SEventDeviceValuator(deviceValuator * from, deviceValuator * to)
{
    char n;
    int i;
    INT32 *ip B32;

    *to = *from;
    swaps(&to->sequenceNumber, n);
    swaps(&to->device_state, n);
    ip = &to->valuator0;
    for (i = 0; i < 6; i++) {
	swapl((ip + i), n);	/* macro - braces are required      */
    }
}

static void
SEventFocus(deviceFocus * from, deviceFocus * to)
{
    char n;

    *to = *from;
    swaps(&to->sequenceNumber, n);
    swapl(&to->time, n);
    swapl(&to->window, n);
}

static void
SDeviceStateNotifyEvent(deviceStateNotify * from, deviceStateNotify * to)
{
    int i;
    char n;
    INT32 *ip B32;

    *to = *from;
    swaps(&to->sequenceNumber, n);
    swapl(&to->time, n);
    ip = &to->valuator0;
    for (i = 0; i < 3; i++) {
	swapl((ip + i), n);	/* macro - braces are required      */
    }
}

static void
SDeviceKeyStateNotifyEvent(deviceKeyStateNotify * from,
			   deviceKeyStateNotify * to)
{
    char n;

    *to = *from;
    swaps(&to->sequenceNumber, n);
}

static void
SDeviceButtonStateNotifyEvent(deviceButtonStateNotify * from,
			      deviceButtonStateNotify * to)
{
    char n;

    *to = *from;
    swaps(&to->sequenceNumber, n);
}

static void
SChangeDeviceNotifyEvent(changeDeviceNotify * from, changeDeviceNotify * to)
{
    char n;

    *to = *from;
    swaps(&to->sequenceNumber, n);
    swapl(&to->time, n);
}

static void
SDeviceMappingNotifyEvent(deviceMappingNotify * from, deviceMappingNotify * to)
{
    char n;

    *to = *from;
    swaps(&to->sequenceNumber, n);
    swapl(&to->time, n);
}

static void
SDevicePresenceNotifyEvent (devicePresenceNotify *from, devicePresenceNotify *to)
{
    char n;

    *to = *from;
    swaps(&to->sequenceNumber,n);
    swapl(&to->time, n);
    swaps(&to->control, n);
}

static void
SDevicePropertyNotifyEvent (devicePropertyNotify *from, devicePropertyNotify *to)
{
    char n;

    *to = *from;
    swaps(&to->sequenceNumber,n);
    swapl(&to->time, n);
    swapl(&to->atom, n);
}

/**************************************************************************
 *
 * Allow the specified event to have its propagation suppressed.
 * The default is to not allow suppression of propagation.
 *
 */

static void
AllowPropagateSuppress(Mask mask)
{
    int i;

    for (i = 0; i < MAXDEVICES; i++)
	PropagateMask[i] |= mask;
}

/**************************************************************************
 *
 * Return the next available extension event mask.
 *
 */

static Mask
GetNextExtEventMask(void)
{
    int i;
    Mask mask = lastExtEventMask;

    if (lastExtEventMask == 0) {
	FatalError("GetNextExtEventMask: no more events are available.");
    }
    lastExtEventMask <<= 1;

    for (i = 0; i < MAXDEVICES; i++)
	ExtValidMasks[i] |= mask;
    return mask;
}

/**************************************************************************
 *
 * Record an event mask where there is no unique corresponding event type.
 * We can't call SetMaskForEvent, since that would clobber the existing
 * mask for that event.  MotionHint and ButtonMotion are examples.
 *
 * Since extension event types will never be less than 64, we can use
 * 0-63 in the EventInfo array as the "type" to be used to look up this
 * mask.  This means that the corresponding macros such as
 * DevicePointerMotionHint must have access to the same constants.
 *
 */

static void
SetEventInfo(Mask mask, int constant)
{
    EventInfo[ExtEventIndex].mask = mask;
    EventInfo[ExtEventIndex++].type = constant;
}

/**************************************************************************
 *
 * Allow the specified event to be restricted to being selected by one
 * client at a time.
 * The default is to allow more than one client to select the event.
 *
 */

static void
SetExclusiveAccess(Mask mask)
{
    int i;

    for (i = 0; i < MAXDEVICES; i++)
	ExtExclusiveMasks[i] |= mask;
}

/**************************************************************************
 *
 * Assign the specified mask to the specified event.
 *
 */

static void
SetMaskForExtEvent(Mask mask, int event)
{
    int i;

    EventInfo[ExtEventIndex].mask = mask;
    EventInfo[ExtEventIndex++].type = event;

    if ((event < LASTEvent) || (event >= 128))
	FatalError("MaskForExtensionEvent: bogus event number");

    for (i = 0; i < MAXDEVICES; i++)
        SetMaskForEvent(i, mask, event);
}

/************************************************************************
 *
 * This function sets up extension event types and masks.
 *
 */

static void
FixExtensionEvents(ExtensionEntry * extEntry)
{
    Mask mask;

    DeviceValuator = extEntry->eventBase;
    DeviceKeyPress = DeviceValuator + 1;
    DeviceKeyRelease = DeviceKeyPress + 1;
    DeviceButtonPress = DeviceKeyRelease + 1;
    DeviceButtonRelease = DeviceButtonPress + 1;
    DeviceMotionNotify = DeviceButtonRelease + 1;
    DeviceFocusIn = DeviceMotionNotify + 1;
    DeviceFocusOut = DeviceFocusIn + 1;
    ProximityIn = DeviceFocusOut + 1;
    ProximityOut = ProximityIn + 1;
    DeviceStateNotify = ProximityOut + 1;
    DeviceMappingNotify = DeviceStateNotify + 1;
    ChangeDeviceNotify = DeviceMappingNotify + 1;
    DeviceKeyStateNotify = ChangeDeviceNotify + 1;
    DeviceButtonStateNotify = DeviceKeyStateNotify + 1;
    DevicePresenceNotify = DeviceButtonStateNotify + 1;
    DevicePropertyNotify = DevicePresenceNotify + 1;

    event_base[KeyClass] = DeviceKeyPress;
    event_base[ButtonClass] = DeviceButtonPress;
    event_base[ValuatorClass] = DeviceMotionNotify;
    event_base[ProximityClass] = ProximityIn;
    event_base[FocusClass] = DeviceFocusIn;
    event_base[OtherClass] = DeviceStateNotify;

    BadDevice += extEntry->errorBase;
    BadEvent += extEntry->errorBase;
    BadMode += extEntry->errorBase;
    DeviceBusy += extEntry->errorBase;
    BadClass += extEntry->errorBase;

    mask = GetNextExtEventMask();
    SetMaskForExtEvent(mask, DeviceKeyPress);
    AllowPropagateSuppress(mask);

    mask = GetNextExtEventMask();
    SetMaskForExtEvent(mask, DeviceKeyRelease);
    AllowPropagateSuppress(mask);

    mask = GetNextExtEventMask();
    SetMaskForExtEvent(mask, DeviceButtonPress);
    AllowPropagateSuppress(mask);

    mask = GetNextExtEventMask();
    SetMaskForExtEvent(mask, DeviceButtonRelease);
    AllowPropagateSuppress(mask);

    mask = GetNextExtEventMask();
    SetMaskForExtEvent(mask, ProximityIn);
    SetMaskForExtEvent(mask, ProximityOut);
    AllowPropagateSuppress(mask);

    mask = GetNextExtEventMask();
    DeviceStateNotifyMask = mask;
    SetMaskForExtEvent(mask, DeviceStateNotify);

    mask = GetNextExtEventMask();
    DevicePointerMotionMask = mask;
    SetMaskForExtEvent(mask, DeviceMotionNotify);
    AllowPropagateSuppress(mask);

    DevicePointerMotionHintMask = GetNextExtEventMask();
    SetEventInfo(DevicePointerMotionHintMask, _devicePointerMotionHint);
    SetEventInfo(GetNextExtEventMask(), _deviceButton1Motion);
    SetEventInfo(GetNextExtEventMask(), _deviceButton2Motion);
    SetEventInfo(GetNextExtEventMask(), _deviceButton3Motion);
    SetEventInfo(GetNextExtEventMask(), _deviceButton4Motion);
    SetEventInfo(GetNextExtEventMask(), _deviceButton5Motion);

    /* If DeviceButtonMotionMask is != ButtonMotionMask, event delivery
     * breaks down. The device needs the dev->button->motionMask. If DBMM is
     * the same as BMM, we can ensure that both core and device events can be
     * delivered, without the need for extra structures in the DeviceIntRec.
     */
    DeviceButtonMotionMask = GetNextExtEventMask();
    SetEventInfo(DeviceButtonMotionMask, _deviceButtonMotion);
    if (DeviceButtonMotionMask != ButtonMotionMask)
    {
        /* This should never happen, but if it does, hide under the
         * bed and cry for help. */
        ErrorF("[Xi] DeviceButtonMotionMask != ButtonMotionMask. Trouble!\n");
    }

    DeviceFocusChangeMask = GetNextExtEventMask();
    SetMaskForExtEvent(DeviceFocusChangeMask, DeviceFocusIn);
    SetMaskForExtEvent(DeviceFocusChangeMask, DeviceFocusOut);

    mask = GetNextExtEventMask();
    SetMaskForExtEvent(mask, DeviceMappingNotify);
    DeviceMappingNotifyMask = mask;

    mask = GetNextExtEventMask();
    SetMaskForExtEvent(mask, ChangeDeviceNotify);
    ChangeDeviceNotifyMask = mask;

    DeviceButtonGrabMask = GetNextExtEventMask();
    SetEventInfo(DeviceButtonGrabMask, _deviceButtonGrab);
    SetExclusiveAccess(DeviceButtonGrabMask);

    DeviceOwnerGrabButtonMask = GetNextExtEventMask();
    SetEventInfo(DeviceOwnerGrabButtonMask, _deviceOwnerGrabButton);

    DevicePresenceNotifyMask = GetNextExtEventMask();
    SetEventInfo(DevicePresenceNotifyMask, _devicePresence);

    DevicePropertyNotifyMask = GetNextExtEventMask();
    SetMaskForExtEvent(DevicePropertyNotifyMask, DevicePropertyNotify);

    SetEventInfo(0, _noExtensionEvent);
}

/************************************************************************
 *
 * This function restores extension event types and masks to their
 * initial state.
 *
 */

static void
RestoreExtensionEvents(void)
{
    int i, j;

    IReqCode = 0;
    IEventBase = 0;

    for (i = 0; i < ExtEventIndex - 1; i++) {
	if ((EventInfo[i].type >= LASTEvent) && (EventInfo[i].type < 128))
        {
            for (j = 0; j < MAXDEVICES; j++)
                SetMaskForEvent(j, 0, EventInfo[i].type);
        }
	EventInfo[i].mask = 0;
	EventInfo[i].type = 0;
    }
    ExtEventIndex = 0;
    lastExtEventMask = 1;
    DeviceValuator = 0;
    DeviceKeyPress = 1;
    DeviceKeyRelease = 2;
    DeviceButtonPress = 3;
    DeviceButtonRelease = 4;
    DeviceMotionNotify = 5;
    DeviceFocusIn = 6;
    DeviceFocusOut = 7;
    ProximityIn = 8;
    ProximityOut = 9;
    DeviceStateNotify = 10;
    DeviceMappingNotify = 11;
    ChangeDeviceNotify = 12;
    DeviceKeyStateNotify = 13;
    DeviceButtonStateNotify = 13;
    DevicePresenceNotify = 14;
    DevicePropertyNotify = 15;

    BadDevice = 0;
    BadEvent = 1;
    BadMode = 2;
    DeviceBusy = 3;
    BadClass = 4;

}

/***********************************************************************
 *
 * IResetProc.
 * Remove reply-swapping routine.
 * Remove event-swapping routine.
 *
 */

static void
IResetProc(ExtensionEntry * unused)
{

    ReplySwapVector[IReqCode] = ReplyNotSwappd;
    EventSwapVector[DeviceValuator] = NotImplemented;
    EventSwapVector[DeviceKeyPress] = NotImplemented;
    EventSwapVector[DeviceKeyRelease] = NotImplemented;
    EventSwapVector[DeviceButtonPress] = NotImplemented;
    EventSwapVector[DeviceButtonRelease] = NotImplemented;
    EventSwapVector[DeviceMotionNotify] = NotImplemented;
    EventSwapVector[DeviceFocusIn] = NotImplemented;
    EventSwapVector[DeviceFocusOut] = NotImplemented;
    EventSwapVector[ProximityIn] = NotImplemented;
    EventSwapVector[ProximityOut] = NotImplemented;
    EventSwapVector[DeviceStateNotify] = NotImplemented;
    EventSwapVector[DeviceKeyStateNotify] = NotImplemented;
    EventSwapVector[DeviceButtonStateNotify] = NotImplemented;
    EventSwapVector[DeviceMappingNotify] = NotImplemented;
    EventSwapVector[ChangeDeviceNotify] = NotImplemented;
    EventSwapVector[DevicePresenceNotify] = NotImplemented;
    EventSwapVector[DevicePropertyNotify] = NotImplemented;
    RestoreExtensionEvents();
}

/*****************************************************************
 *
 * Returns TRUE if the device has some sort of pointer type.
 *
 */

Bool
DeviceIsPointerType(DeviceIntPtr dev)
{
    if (dev_type[1].type == dev->type)
        return TRUE;

    return FALSE;
}


/***********************************************************************
 *
 * Assign an id and type to an input device.
 *
 */

void
AssignTypeAndName(DeviceIntPtr dev, Atom type, char *name)
{
    dev->type = type;
    dev->name = (char *)xalloc(strlen(name) + 1);
    strcpy(dev->name, name);
}

/***********************************************************************
 *
 * Make device type atoms.
 *
 */

static void
MakeDeviceTypeAtoms(void)
{
    int i;

    for (i = 0; i < NUMTYPES; i++)
	dev_type[i].type =
	    MakeAtom(dev_type[i].name, strlen(dev_type[i].name), 1);
}

/*****************************************************************************
 *
 *	SEventIDispatch
 *
 *	Swap any events defined in this extension.
 */
#define DO_SWAP(func,type) func ((type *)from, (type *)to)

static void
SEventIDispatch(xEvent * from, xEvent * to)
{
    int type = from->u.u.type & 0177;

    if (type == DeviceValuator)
	DO_SWAP(SEventDeviceValuator, deviceValuator);
    else if (type == DeviceKeyPress) {
	SKeyButtonPtrEvent(from, to);
	to->u.keyButtonPointer.pad1 = from->u.keyButtonPointer.pad1;
    } else if (type == DeviceKeyRelease) {
	SKeyButtonPtrEvent(from, to);
	to->u.keyButtonPointer.pad1 = from->u.keyButtonPointer.pad1;
    } else if (type == DeviceButtonPress) {
	SKeyButtonPtrEvent(from, to);
	to->u.keyButtonPointer.pad1 = from->u.keyButtonPointer.pad1;
    } else if (type == DeviceButtonRelease) {
	SKeyButtonPtrEvent(from, to);
	to->u.keyButtonPointer.pad1 = from->u.keyButtonPointer.pad1;
    } else if (type == DeviceMotionNotify) {
	SKeyButtonPtrEvent(from, to);
	to->u.keyButtonPointer.pad1 = from->u.keyButtonPointer.pad1;
    } else if (type == DeviceFocusIn)
	DO_SWAP(SEventFocus, deviceFocus);
    else if (type == DeviceFocusOut)
	DO_SWAP(SEventFocus, deviceFocus);
    else if (type == ProximityIn) {
	SKeyButtonPtrEvent(from, to);
	to->u.keyButtonPointer.pad1 = from->u.keyButtonPointer.pad1;
    } else if (type == ProximityOut) {
	SKeyButtonPtrEvent(from, to);
	to->u.keyButtonPointer.pad1 = from->u.keyButtonPointer.pad1;
    } else if (type == DeviceStateNotify)
	DO_SWAP(SDeviceStateNotifyEvent, deviceStateNotify);
    else if (type == DeviceKeyStateNotify)
	DO_SWAP(SDeviceKeyStateNotifyEvent, deviceKeyStateNotify);
    else if (type == DeviceButtonStateNotify)
	DO_SWAP(SDeviceButtonStateNotifyEvent, deviceButtonStateNotify);
    else if (type == DeviceMappingNotify)
	DO_SWAP(SDeviceMappingNotifyEvent, deviceMappingNotify);
    else if (type == ChangeDeviceNotify)
	DO_SWAP(SChangeDeviceNotifyEvent, changeDeviceNotify);
    else if (type == DevicePresenceNotify)
	DO_SWAP(SDevicePresenceNotifyEvent, devicePresenceNotify);
    else if (type == DevicePropertyNotify)
	DO_SWAP(SDevicePropertyNotifyEvent, devicePropertyNotify);
    else {
	FatalError("XInputExtension: Impossible event!\n");
    }
}

/**********************************************************************
 *
 * IExtensionInit - initialize the input extension.
 *
 * Called from InitExtensions in main() or from QueryExtension() if the
 * extension is dynamically loaded.
 *
 * This extension has several events and errors.
 *
 * XI is mandatory nowadays, so if we fail to init XI, we die.
 */

void
XInputExtensionInit(void)
{
    ExtensionEntry *extEntry;

    if (!dixRequestPrivate(XIClientPrivateKey, sizeof(XIClientRec)))
        FatalError("Cannot request private for XI.\n");

    if (!AddCallback(&ClientStateCallback, XIClientCallback, 0))
        FatalError("Failed to add callback to XI.\n");

    extEntry = AddExtension(INAME, IEVENTS, IERRORS, ProcIDispatch,
			    SProcIDispatch, IResetProc, StandardMinorOpcode);
    if (extEntry) {
	IReqCode = extEntry->base;
	IEventBase = extEntry->eventBase;
	AllExtensionVersions[IReqCode - 128] = thisversion;
	MakeDeviceTypeAtoms();
	XIInitKnownProperties();
	RT_INPUTCLIENT = CreateNewResourceType((DeleteType) InputClientGone);
	RegisterResourceName(RT_INPUTCLIENT, "INPUTCLIENT");
	FixExtensionEvents(extEntry);
	ReplySwapVector[IReqCode] = (ReplySwapPtr) SReplyIDispatch;
	EventSwapVector[DeviceValuator] = SEventIDispatch;
	EventSwapVector[DeviceKeyPress] = SEventIDispatch;
	EventSwapVector[DeviceKeyRelease] = SEventIDispatch;
	EventSwapVector[DeviceButtonPress] = SEventIDispatch;
	EventSwapVector[DeviceButtonRelease] = SEventIDispatch;
	EventSwapVector[DeviceMotionNotify] = SEventIDispatch;
	EventSwapVector[DeviceFocusIn] = SEventIDispatch;
	EventSwapVector[DeviceFocusOut] = SEventIDispatch;
	EventSwapVector[ProximityIn] = SEventIDispatch;
	EventSwapVector[ProximityOut] = SEventIDispatch;
	EventSwapVector[DeviceStateNotify] = SEventIDispatch;
	EventSwapVector[DeviceKeyStateNotify] = SEventIDispatch;
	EventSwapVector[DeviceButtonStateNotify] = SEventIDispatch;
	EventSwapVector[DeviceMappingNotify] = SEventIDispatch;
	EventSwapVector[ChangeDeviceNotify] = SEventIDispatch;
	EventSwapVector[DevicePresenceNotify] = SEventIDispatch;

    } else {
	FatalError("IExtensionInit: AddExtensions failed\n");
    }
}
