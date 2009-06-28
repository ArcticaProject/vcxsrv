/************************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/



#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "misc.h"
#include "resource.h"
#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xproto.h>
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "site.h"
#ifndef XKB_IN_SERVER
#define	XKB_IN_SERVER
#endif
#ifdef XKB
#include <xkbsrv.h>
#endif
#include "privates.h"
#include "xace.h"

#include "dispatch.h"
#include "swaprep.h"
#include "dixevents.h"

#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "exglobals.h"
#include "exevents.h"

/** @file
 * This file handles input device-related stuff.
 */

DevPrivateKey CoreDevicePrivateKey = &CoreDevicePrivateKey;

/**
 * Create a new input device and init it to sane values. The device is added
 * to the server's off_devices list.
 *
 * @param deviceProc Callback for device control function (switch dev on/off).
 * @return The newly created device.
 */
DeviceIntPtr
AddInputDevice(DeviceProc deviceProc, Bool autoStart)
{
    DeviceIntPtr dev, *prev; /* not a typo */
    DeviceIntPtr devtmp;
    int devid;
    char devind[MAX_DEVICES];

    /* Find next available id */
    memset(devind, 0, sizeof(char)*MAX_DEVICES);
    for (devtmp = inputInfo.devices; devtmp; devtmp = devtmp->next)
	devind[devtmp->id]++;
    for (devtmp = inputInfo.off_devices; devtmp; devtmp = devtmp->next)
	devind[devtmp->id]++;
    for (devid = 0; devid < MAX_DEVICES && devind[devid]; devid++)
	;

    if (devid >= MAX_DEVICES)
	return (DeviceIntPtr)NULL;
    dev = (DeviceIntPtr) xcalloc(sizeof(DeviceIntRec), 1);
    if (!dev)
	return (DeviceIntPtr)NULL;
    dev->name = (char *)NULL;
    dev->type = 0;
    dev->id = devid;
    dev->public.on = FALSE;
    dev->public.processInputProc = (ProcessInputProc)NoopDDA;
    dev->public.realInputProc = (ProcessInputProc)NoopDDA;
    dev->public.enqueueInputProc = EnqueueEvent;
    dev->deviceProc = deviceProc;
    dev->startup = autoStart;
    dev->sync.frozen = FALSE;
    dev->sync.other = NullGrab;
    dev->sync.state = NOT_GRABBED;
    dev->sync.event = (xEvent *) NULL;
    dev->sync.evcount = 0;
    dev->grab = NullGrab;
    dev->grabTime = currentTime;
    dev->fromPassiveGrab = FALSE;
    dev->key = (KeyClassPtr)NULL;
    dev->valuator = (ValuatorClassPtr)NULL;
    dev->button = (ButtonClassPtr)NULL;
    dev->focus = (FocusClassPtr)NULL;
    dev->proximity = (ProximityClassPtr)NULL;
    dev->absolute = (AbsoluteClassPtr)NULL;
    dev->kbdfeed = (KbdFeedbackPtr)NULL;
    dev->ptrfeed = (PtrFeedbackPtr)NULL;
    dev->intfeed = (IntegerFeedbackPtr)NULL;
    dev->stringfeed = (StringFeedbackPtr)NULL;
    dev->bell = (BellFeedbackPtr)NULL;
    dev->leds = (LedFeedbackPtr)NULL;
#ifdef XKB
    dev->xkb_interest = NULL;
#endif
    dev->config_info = NULL;
    dev->devPrivates = NULL;
    dev->unwrapProc = NULL;
    dev->coreEvents = TRUE;
    dev->inited = FALSE;
    dev->enabled = FALSE;

    /*  security creation/labeling check
     */
    if (XaceHook(XACE_DEVICE_ACCESS, serverClient, dev, DixCreateAccess)) {
	xfree(dev);
	return NULL;
    }

    inputInfo.numDevices++;

    for (prev = &inputInfo.off_devices; *prev; prev = &(*prev)->next)
        ;
    *prev = dev;
    dev->next = NULL;

    return dev;
}

/**
 * Switch device ON through the driver and push it onto the global device
 * list. All clients are notified about the device being enabled.
 *
 * A device will send events once enabled.
 *
 * @param The device to be enabled.
 * @return TRUE on success or FALSE otherwise.
 */
Bool
EnableDevice(DeviceIntPtr dev)
{
    DeviceIntPtr *prev;
    int ret;
    DeviceIntRec dummyDev;
    devicePresenceNotify ev;

    for (prev = &inputInfo.off_devices;
	 *prev && (*prev != dev);
	 prev = &(*prev)->next)
	;
    if ((*prev != dev) || !dev->inited ||
	((ret = (*dev->deviceProc)(dev, DEVICE_ON)) != Success)) {
        ErrorF("couldn't enable device %d\n", dev->id);
	return FALSE;
    }
    dev->enabled = TRUE;
    *prev = dev->next;

    for (prev = &inputInfo.devices; *prev; prev = &(*prev)->next)
        ;
    *prev = dev;
    dev->next = NULL;

    ev.type = DevicePresenceNotify;
    ev.time = currentTime.milliseconds;
    ev.devchange = DeviceEnabled;
    ev.deviceid = dev->id;
    dummyDev.id = 0;
    SendEventToAllWindows(&dummyDev, DevicePresenceNotifyMask,
                          (xEvent *) &ev, 1);

    return TRUE;
}

/**
 * Switch a device off through the driver and push it onto the off_devices
 * list. A device will not send events while disabled. All clients are
 * notified about the device being disabled.
 *
 * @return TRUE on success or FALSE otherwise.
 */
Bool
DisableDevice(DeviceIntPtr dev)
{
    DeviceIntPtr *prev;
    DeviceIntRec dummyDev;
    devicePresenceNotify ev;

    for (prev = &inputInfo.devices;
	 *prev && (*prev != dev);
	 prev = &(*prev)->next)
	;
    if (*prev != dev)
	return FALSE;
    (void)(*dev->deviceProc)(dev, DEVICE_OFF);
    dev->enabled = FALSE;
    *prev = dev->next;
    dev->next = inputInfo.off_devices;
    inputInfo.off_devices = dev;

    ev.type = DevicePresenceNotify;
    ev.time = currentTime.milliseconds;
    ev.devchange = DeviceDisabled;
    ev.deviceid = dev->id;
    dummyDev.id = 0;
    SendEventToAllWindows(&dummyDev, DevicePresenceNotifyMask,
                          (xEvent *) &ev, 1);

    return TRUE;
}

/**
 * Initialise a new device through the driver and tell all clients about the
 * new device.
 * 
 * The device will NOT send events until it is enabled!
 *
 * @return Success or an error code on failure.
 */
int
ActivateDevice(DeviceIntPtr dev)
{
    int ret = Success;
    devicePresenceNotify ev;
    DeviceIntRec dummyDev;

    if (!dev || !dev->deviceProc)
        return BadImplementation;

    ret = (*dev->deviceProc) (dev, DEVICE_INIT);
    dev->inited = (ret == Success);
    
    ev.type = DevicePresenceNotify;
    ev.time = currentTime.milliseconds;
    ev.devchange = DeviceAdded;
    ev.deviceid = dev->id;
    dummyDev.id = 0;
    SendEventToAllWindows(&dummyDev, DevicePresenceNotifyMask,
                          (xEvent *) &ev, 1);

    return ret;
}

/**
 * Ring the bell.
 * The actual task of ringing the bell is the job of the DDX.
 */
static void
CoreKeyboardBell(int volume, DeviceIntPtr pDev, pointer arg, int something)
{
    KeybdCtrl *ctrl = arg;

    DDXRingBell(volume, ctrl->bell_pitch, ctrl->bell_duration);
}

static void
CoreKeyboardCtl(DeviceIntPtr pDev, KeybdCtrl *ctrl)
{
    return;
}

/**
 * Device control function for the Virtual Core Keyboard. 
 */
static int
CoreKeyboardProc(DeviceIntPtr pDev, int what)
{
    CARD8 *modMap;
    KeySymsRec keySyms;
#ifdef XKB
    XkbComponentNamesRec names;
#endif

    switch (what) {
    case DEVICE_INIT:
        keySyms.minKeyCode = 8;
        keySyms.maxKeyCode = 255;
        keySyms.mapWidth = 4;
        keySyms.map = (KeySym *)xcalloc(sizeof(KeySym),
                                        (keySyms.maxKeyCode -
                                         keySyms.minKeyCode + 1) *
                                        keySyms.mapWidth);
        if (!keySyms.map) {
            ErrorF("Couldn't allocate core keymap\n");
            return BadAlloc;
        }

        modMap = (CARD8 *)xalloc(MAP_LENGTH);
        if (!modMap) {
            ErrorF("Couldn't allocate core modifier map\n");
            return BadAlloc;
        }
        bzero((char *)modMap, MAP_LENGTH);

#ifdef XKB
        if (!noXkbExtension) {
            bzero(&names, sizeof(names));
            XkbSetRulesDflts("base", "pc105", "us", NULL, NULL);
            XkbInitKeyboardDeviceStruct(pDev, &names, &keySyms, modMap,
                                        CoreKeyboardBell, CoreKeyboardCtl);
        }
        else
#endif
        {
            /* FIXME Our keymap here isn't exactly useful. */
            InitKeyboardDeviceStruct((DevicePtr)pDev, &keySyms, modMap,
                                     CoreKeyboardBell, CoreKeyboardCtl);
        }

        xfree(keySyms.map);
        xfree(modMap);

        break;

    case DEVICE_CLOSE:
	dixSetPrivate(&pDev->devPrivates, CoreDevicePrivateKey, NULL);
        break;

    default:
        break;
    }
    return Success;
}

/**
 * Device control function for the Virtual Core Pointer.
 */
static int
CorePointerProc(DeviceIntPtr pDev, int what)
{
    BYTE map[33];
    int i = 0;

    switch (what) {
    case DEVICE_INIT:
        for (i = 1; i <= 32; i++)
            map[i] = i;
        InitPointerDeviceStruct((DevicePtr)pDev, map, 32,
                                GetMotionHistory, (PtrCtrlProcPtr)NoopDDA,
                                GetMotionHistorySize(), 2);
        pDev->valuator->axisVal[0] = screenInfo.screens[0]->width / 2;
        pDev->valuator->lastx = pDev->valuator->axisVal[0];
        pDev->valuator->axisVal[1] = screenInfo.screens[0]->height / 2;
        pDev->valuator->lasty = pDev->valuator->axisVal[1];
        break;

    case DEVICE_CLOSE:
	dixSetPrivate(&pDev->devPrivates, CoreDevicePrivateKey, NULL);
        break;

    default:
        break;
    }

    return Success;
}

/**
 * Initialise the two core devices, VCP and VCK (see events.c).
 * The devices are activated but not enabled.
 * Note that the server MUST have two core devices at all times, even if there
 * is no physical device connected.
 */
void
InitCoreDevices(void)
{
    DeviceIntPtr dev;

    if (!inputInfo.keyboard) {
        dev = AddInputDevice(CoreKeyboardProc, TRUE);
        if (!dev)
            FatalError("Failed to allocate core keyboard");
        dev->name = strdup("Virtual core keyboard");
#ifdef XKB
        dev->public.processInputProc = CoreProcessKeyboardEvent;
        dev->public.realInputProc = CoreProcessKeyboardEvent;
        if (!noXkbExtension)
           XkbSetExtension(dev, ProcessKeyboardEvent);
#else
        dev->public.processInputProc = ProcessKeyboardEvent;
        dev->public.realInputProc = ProcessKeyboardEvent;
#endif
        dev->ActivateGrab = ActivateKeyboardGrab;
        dev->DeactivateGrab = DeactivateKeyboardGrab;
        dev->coreEvents = FALSE;
        (void)ActivateDevice(dev);
        inputInfo.keyboard = dev;
    }

    if (!inputInfo.pointer) {
        dev = AddInputDevice(CorePointerProc, TRUE);
        if (!dev)
            FatalError("Failed to allocate core pointer");
        dev->name = strdup("Virtual core pointer");
#ifdef XKB
        dev->public.processInputProc = CoreProcessPointerEvent;
        dev->public.realInputProc = CoreProcessPointerEvent;
        if (!noXkbExtension)
           XkbSetExtension(dev, ProcessPointerEvent);
#else
        dev->public.processInputProc = ProcessPointerEvent;
        dev->public.realInputProc = ProcessPointerEvent;
#endif
        dev->ActivateGrab = ActivatePointerGrab;
        dev->DeactivateGrab = DeactivatePointerGrab;
        dev->coreEvents = FALSE;
        (void)ActivateDevice(dev);
        inputInfo.pointer = dev;
    }
}

/**
 * Activate all switched-off devices and then enable all those devices.
 * 
 * Will return an error if no core keyboard or core pointer is present.
 * In theory this should never happen if you call InitCoreDevices() first.
 * 
 * @return Success or error code on failure.
 */
int
InitAndStartDevices(void)
{
    DeviceIntPtr dev, next;

    for (dev = inputInfo.off_devices; dev; dev = dev->next) {
        DebugF("(dix) initialising device %d\n", dev->id);
        if (!dev->inited)
            ActivateDevice(dev);
    }
    for (dev = inputInfo.off_devices; dev; dev = next)
    {
        DebugF("(dix) enabling device %d\n", dev->id);
	next = dev->next;
	if (dev->inited && dev->startup)
	    (void)EnableDevice(dev);
    }
    for (dev = inputInfo.devices;
	 dev && (dev != inputInfo.keyboard);
	 dev = dev->next)
	;
    if (!dev || (dev != inputInfo.keyboard)) {
	ErrorF("No core keyboard\n");
	return BadImplementation;
    }
    for (dev = inputInfo.devices;
	 dev && (dev != inputInfo.pointer);
	 dev = dev->next)
	;
    if (!dev || (dev != inputInfo.pointer)) {
	ErrorF("No core pointer\n");
	return BadImplementation;
    }
    return Success;
}

/**
 * Close down a device and free all resources. 
 * Once closed down, the driver will probably not expect you that you'll ever
 * enable it again and free associated structs. If you want the device to just
 * be disabled, DisableDevice().
 * Don't call this function directly, use RemoveDevice() instead.
 */
static void
CloseDevice(DeviceIntPtr dev)
{
    KbdFeedbackPtr k, knext;
    PtrFeedbackPtr p, pnext;
    IntegerFeedbackPtr i, inext;
    StringFeedbackPtr s, snext;
    BellFeedbackPtr b, bnext;
    LedFeedbackPtr l, lnext;

    if (dev->inited)
	(void)(*dev->deviceProc)(dev, DEVICE_CLOSE);

    xfree(dev->name);

    if (dev->key) {
#ifdef XKB
	if (dev->key->xkbInfo)
	    XkbFreeInfo(dev->key->xkbInfo);
#endif
        dev->key->xkbInfo = NULL;
	xfree(dev->key->curKeySyms.map);
	xfree(dev->key->modifierKeyMap);
	xfree(dev->key);
        dev->key = NULL;
    }

    if (dev->valuator) {
        /* Counterpart to 'biggest hack ever' in init. */
        if (dev->valuator->motion &&
            dev->valuator->GetMotionProc == GetMotionHistory)
            xfree(dev->valuator->motion);
        xfree(dev->valuator);
    }

    if (dev->button) {
#ifdef XKB
        if (dev->button->xkb_acts)
            xfree(dev->button->xkb_acts);
#endif
        xfree(dev->button);
    }

    if (dev->focus) {
	xfree(dev->focus->trace);
	xfree(dev->focus);
    }

    if (dev->proximity)
        xfree(dev->proximity);

    for (k = dev->kbdfeed; k; k = knext) {
	knext = k->next;
#ifdef XKB
	if (k->xkb_sli)
	    XkbFreeSrvLedInfo(k->xkb_sli);
#endif
	xfree(k);
    }

    for (p = dev->ptrfeed; p; p = pnext) {
	pnext = p->next;
	xfree(p);
    }
    
    for (i = dev->intfeed; i; i = inext) {
	inext = i->next;
	xfree(i);
    }

    for (s = dev->stringfeed; s; s = snext) {
	snext = s->next;
	xfree(s->ctrl.symbols_supported);
	xfree(s->ctrl.symbols_displayed);
	xfree(s);
    }

    for (b = dev->bell; b; b = bnext) {
	bnext = b->next;
	xfree(b);
    }

    for (l = dev->leds; l; l = lnext) {
	lnext = l->next;
#ifdef XKB
	if (l->xkb_sli)
	    XkbFreeSrvLedInfo(l->xkb_sli);
#endif
	xfree(l);
    }

#ifdef XKB
    while (dev->xkb_interest)
	XkbRemoveResourceClient((DevicePtr)dev,dev->xkb_interest->resource);
#endif

    xfree(dev->sync.event);
    dixFreePrivates(dev->devPrivates);
    xfree(dev);
}

/**
 * Shut down all devices, free all resources, etc. 
 * Only useful if you're shutting down the server!
 */
void
CloseDownDevices(void)
{
    DeviceIntPtr dev, next;

    for (dev = inputInfo.devices; dev; dev = next)
    {
	next = dev->next;
        DeleteInputDeviceRequest(dev);
    }
    for (dev = inputInfo.off_devices; dev; dev = next)
    {
	next = dev->next;
        DeleteInputDeviceRequest(dev);
    }
    inputInfo.devices = NULL;
    inputInfo.off_devices = NULL;
    inputInfo.keyboard = NULL;
    inputInfo.pointer = NULL;
}

/**
 * Remove a device from the device list, closes it and thus frees all
 * resources. 
 * Removes both enabled and disabled devices and notifies all devices about
 * the removal of the device.
 */
int
RemoveDevice(DeviceIntPtr dev)
{
    DeviceIntPtr prev,tmp,next;
    int ret = BadMatch;
    devicePresenceNotify ev;
    DeviceIntRec dummyDev;
    int deviceid;

    DebugF("(dix) removing device %d\n", dev->id);

    if (!dev || dev == inputInfo.keyboard || dev == inputInfo.pointer)
        return BadImplementation;

    deviceid = dev->id;
    DisableDevice(dev);

    prev = NULL;
    for (tmp = inputInfo.devices; tmp; (prev = tmp), (tmp = next)) {
	next = tmp->next;
	if (tmp == dev) {
	    CloseDevice(tmp);

	    if (prev==NULL)
		inputInfo.devices = next;
	    else
		prev->next = next;

	    ret = Success;
	}
    }

    prev = NULL;
    for (tmp = inputInfo.off_devices; tmp; (prev = tmp), (tmp = next)) {
	next = tmp->next;
	if (tmp == dev) {
	    CloseDevice(tmp);

	    if (prev == NULL)
		inputInfo.off_devices = next;
	    else
		prev->next = next;

            ret = Success;
	}
    }
    
    if (ret == Success) {
        inputInfo.numDevices--;
        ev.type = DevicePresenceNotify;
        ev.time = currentTime.milliseconds;
        ev.devchange = DeviceRemoved;
        ev.deviceid = deviceid;
        dummyDev.id = 0;
        SendEventToAllWindows(&dummyDev, DevicePresenceNotifyMask,
                              (xEvent *) &ev, 1);
    }

    return ret;
}

int
NumMotionEvents(void)
{
    return inputInfo.pointer->valuator->numMotionEvents;
}

void
RegisterPointerDevice(DeviceIntPtr device)
{
    RegisterOtherDevice(device);
}

void
RegisterKeyboardDevice(DeviceIntPtr device)
{
    RegisterOtherDevice(device);
}

int
dixLookupDevice(DeviceIntPtr *pDev, int id, ClientPtr client, Mask access_mode)
{
    DeviceIntPtr dev;
    int rc;
    *pDev = NULL;

    for (dev=inputInfo.devices; dev; dev=dev->next) {
        if (dev->id == (CARD8)id)
            goto found;
    }
    for (dev=inputInfo.off_devices; dev; dev=dev->next) {
        if (dev->id == (CARD8)id)
	    goto found;
    }
    return BadDevice;

found:
    rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, access_mode);
    if (rc == Success)
	*pDev = dev;
    return rc;
}

void
QueryMinMaxKeyCodes(KeyCode *minCode, KeyCode *maxCode)
{
    if (inputInfo.keyboard) {
	*minCode = inputInfo.keyboard->key->curKeySyms.minKeyCode;
	*maxCode = inputInfo.keyboard->key->curKeySyms.maxKeyCode;
    }
}

Bool
SetKeySymsMap(KeySymsPtr dst, KeySymsPtr src)
{
    int i, j;
    int rowDif = src->minKeyCode - dst->minKeyCode;

    /* if keysym map size changes, grow map first */
    if (src->mapWidth < dst->mapWidth)
    {
        for (i = src->minKeyCode; i <= src->maxKeyCode; i++)
	{
#define SI(r, c) (((r-src->minKeyCode)*src->mapWidth) + (c))
#define DI(r, c) (((r - dst->minKeyCode)*dst->mapWidth) + (c))
	    for (j = 0; j < src->mapWidth; j++)
		dst->map[DI(i, j)] = src->map[SI(i, j)];
	    for (j = src->mapWidth; j < dst->mapWidth; j++)
		dst->map[DI(i, j)] = NoSymbol;
#undef SI
#undef DI
	}
	return TRUE;
    }
    else if (src->mapWidth > dst->mapWidth)
    {
        KeySym *map;
	int bytes = sizeof(KeySym) * src->mapWidth *
		    (dst->maxKeyCode - dst->minKeyCode + 1);
        map = (KeySym *)xalloc(bytes);
	if (!map)
	    return FALSE;
	bzero((char *)map, bytes);
        if (dst->map)
	{
            for (i = 0; i <= dst->maxKeyCode-dst->minKeyCode; i++)
		memmove((char *)&map[i*src->mapWidth],
			(char *)&dst->map[i*dst->mapWidth],
		      dst->mapWidth * sizeof(KeySym));
	    xfree(dst->map);
	}
	dst->mapWidth = src->mapWidth;
	dst->map = map;
    }
    memmove((char *)&dst->map[rowDif * dst->mapWidth],
	    (char *)src->map,
	  (int)(src->maxKeyCode - src->minKeyCode + 1) *
	  dst->mapWidth * sizeof(KeySym));
    return TRUE;
}

static Bool
InitModMap(KeyClassPtr keyc)
{
    int i, j;
    CARD8 keysPerModifier[8];
    CARD8 mask;

    keyc->maxKeysPerModifier = 0;
    for (i = 0; i < 8; i++)
	keysPerModifier[i] = 0;
    for (i = 8; i < MAP_LENGTH; i++)
    {
	for (j = 0, mask = 1; j < 8; j++, mask <<= 1)
	{
	    if (mask & keyc->modifierMap[i])
	    {
		if (++keysPerModifier[j] > keyc->maxKeysPerModifier)
		    keyc->maxKeysPerModifier = keysPerModifier[j];
	    }
	}
    }
    keyc->modifierKeyMap = (KeyCode *)xalloc(8*keyc->maxKeysPerModifier);
    if (!keyc->modifierKeyMap && keyc->maxKeysPerModifier)
	return (FALSE);
    bzero((char *)keyc->modifierKeyMap, 8*(int)keyc->maxKeysPerModifier);
    for (i = 0; i < 8; i++)
	keysPerModifier[i] = 0;
    for (i = 8; i < MAP_LENGTH; i++)
    {
	for (j = 0, mask = 1; j < 8; j++, mask <<= 1)
	{
	    if (mask & keyc->modifierMap[i])
	    {
		keyc->modifierKeyMap[(j*keyc->maxKeysPerModifier) +
				     keysPerModifier[j]] = i;
		keysPerModifier[j]++;
	    }
	}
    }
    return TRUE;
}

_X_EXPORT Bool
InitKeyClassDeviceStruct(DeviceIntPtr dev, KeySymsPtr pKeySyms, CARD8 pModifiers[])
{
    int i;
    KeyClassPtr keyc;
    
    keyc = (KeyClassPtr)xalloc(sizeof(KeyClassRec));
    if (!keyc)
	return FALSE;
    keyc->curKeySyms.map = (KeySym *)NULL;
    keyc->curKeySyms.mapWidth = 0;
    keyc->curKeySyms.minKeyCode = pKeySyms->minKeyCode;
    keyc->curKeySyms.maxKeyCode = pKeySyms->maxKeyCode;
    keyc->modifierKeyMap = (KeyCode *)NULL;
    keyc->state = 0;
    keyc->prev_state = 0;
    if (pModifiers)
	memmove((char *)keyc->modifierMap, (char *)pModifiers, MAP_LENGTH);
    else
	bzero((char *)keyc->modifierMap, MAP_LENGTH);
    bzero((char *)keyc->down, DOWN_LENGTH);
    bzero((char *)keyc->postdown, DOWN_LENGTH);
    for (i = 0; i < 8; i++)
	keyc->modifierKeyCount[i] = 0;
    if (!SetKeySymsMap(&keyc->curKeySyms, pKeySyms) || !InitModMap(keyc))
    {
	xfree(keyc->curKeySyms.map);
	xfree(keyc->modifierKeyMap);
	xfree(keyc);
	return FALSE;
    }
    dev->key = keyc;
#ifdef XKB
    dev->key->xkbInfo= NULL;
    if (!noXkbExtension) XkbInitDevice(dev);
#endif
    return TRUE;
}

_X_EXPORT Bool
InitButtonClassDeviceStruct(DeviceIntPtr dev, int numButtons, 
                            CARD8 *map)
{
    ButtonClassPtr butc;
    int i;

    butc = (ButtonClassPtr)xalloc(sizeof(ButtonClassRec));
    if (!butc)
	return FALSE;
    butc->numButtons = numButtons;
    for (i = 1; i <= numButtons; i++)
	butc->map[i] = map[i];
    butc->buttonsDown = 0;
    butc->state = 0;
    butc->motionMask = 0;
    bzero((char *)butc->down, DOWN_LENGTH);
#ifdef XKB
    butc->xkb_acts=	NULL;
#endif
    dev->button = butc;
    return TRUE;
}

_X_EXPORT Bool
InitValuatorClassDeviceStruct(DeviceIntPtr dev, int numAxes, 
                              ValuatorMotionProcPtr motionProc, 
                              int numMotionEvents, int mode)
{
    int i;
    ValuatorClassPtr valc;

    if (!dev)
        return FALSE;

    valc = (ValuatorClassPtr)xalloc(sizeof(ValuatorClassRec) +
				    numAxes * sizeof(AxisInfo) +
				    numAxes * sizeof(unsigned int));
    if (!valc)
	return FALSE;

    valc->motion = NULL;
    valc->first_motion = 0;
    valc->last_motion = 0;
    valc->GetMotionProc = motionProc;

    valc->numMotionEvents = numMotionEvents;
    valc->motionHintWindow = NullWindow;
    valc->numAxes = numAxes;
    valc->mode = mode;
    valc->axes = (AxisInfoPtr)(valc + 1);
    valc->axisVal = (int *)(valc->axes + numAxes);
    valc->lastx = 0;
    valc->lasty = 0;
    valc->dxremaind = 0;
    valc->dyremaind = 0;
    dev->valuator = valc;

    /* biggest hack ever. */
    if (motionProc == GetMotionHistory)
        AllocateMotionHistory(dev);

    for (i=0; i<numAxes; i++) {
        InitValuatorAxisStruct(dev, i, 0, -1, 0, 0, 0);
	valc->axisVal[i]=0;
    }
    return TRUE;
}

_X_EXPORT Bool
InitAbsoluteClassDeviceStruct(DeviceIntPtr dev)
{
    AbsoluteClassPtr abs;

    abs = (AbsoluteClassPtr)xalloc(sizeof(AbsoluteClassRec));
    if (!abs)
        return FALSE;

    /* we don't do anything sensible with these, but should */
    abs->min_x = -1;
    abs->min_y = -1;
    abs->max_x = -1;
    abs->max_y = -1;
    abs->flip_x = 0;
    abs->flip_y = 0;
    abs->rotation = 0;
    abs->button_threshold = 0;

    abs->offset_x = 0;
    abs->offset_y = 0;
    abs->width = -1;
    abs->height = -1;
    abs->following = 0;
    abs->screen = 0;

    dev->absolute = abs;

    return TRUE;
}

_X_EXPORT Bool
InitFocusClassDeviceStruct(DeviceIntPtr dev)
{
    FocusClassPtr focc;

    focc = (FocusClassPtr)xalloc(sizeof(FocusClassRec));
    if (!focc)
	return FALSE;
    focc->win = PointerRootWin;
    focc->revert = None;
    focc->time = currentTime;
    focc->trace = (WindowPtr *)NULL;
    focc->traceSize = 0;
    focc->traceGood = 0;
    dev->focus = focc;
    return TRUE;
}

_X_EXPORT Bool
InitKbdFeedbackClassDeviceStruct(DeviceIntPtr dev, BellProcPtr bellProc, 
                                 KbdCtrlProcPtr controlProc)
{
    KbdFeedbackPtr feedc;

    feedc = (KbdFeedbackPtr)xalloc(sizeof(KbdFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->BellProc = bellProc;
    feedc->CtrlProc = controlProc;
#ifdef XKB
    defaultKeyboardControl.autoRepeat = TRUE;
#endif
    feedc->ctrl = defaultKeyboardControl;
    feedc->ctrl.id = 0;
    if ((feedc->next = dev->kbdfeed) != 0)
	feedc->ctrl.id = dev->kbdfeed->ctrl.id + 1;
    dev->kbdfeed = feedc;
#ifdef XKB
    feedc->xkb_sli= NULL;
    if (!noXkbExtension)
	XkbFinishDeviceInit(dev);
#endif
    (*dev->kbdfeed->CtrlProc)(dev,&dev->kbdfeed->ctrl);
    return TRUE;
}

_X_EXPORT Bool
InitPtrFeedbackClassDeviceStruct(DeviceIntPtr dev, PtrCtrlProcPtr controlProc)
{
    PtrFeedbackPtr feedc;

    feedc = (PtrFeedbackPtr)xalloc(sizeof(PtrFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->ctrl = defaultPointerControl;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->ptrfeed) )
        feedc->ctrl.id = dev->ptrfeed->ctrl.id + 1;
    dev->ptrfeed = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}


static LedCtrl defaultLedControl = {
	DEFAULT_LEDS, DEFAULT_LEDS_MASK, 0};

static BellCtrl defaultBellControl = {
	DEFAULT_BELL,
	DEFAULT_BELL_PITCH,
	DEFAULT_BELL_DURATION,
	0};

static IntegerCtrl defaultIntegerControl = {
	DEFAULT_INT_RESOLUTION,
	DEFAULT_INT_MIN_VALUE,
	DEFAULT_INT_MAX_VALUE,
	DEFAULT_INT_DISPLAYED,
	0};

_X_EXPORT Bool
InitStringFeedbackClassDeviceStruct (
      DeviceIntPtr dev, StringCtrlProcPtr controlProc, 
      int max_symbols, int num_symbols_supported, KeySym *symbols)
{
    int i;
    StringFeedbackPtr feedc;

    feedc = (StringFeedbackPtr)xalloc(sizeof(StringFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->ctrl.num_symbols_supported = num_symbols_supported;
    feedc->ctrl.num_symbols_displayed = 0;
    feedc->ctrl.max_symbols = max_symbols;
    feedc->ctrl.symbols_supported = (KeySym *) 
	xalloc (sizeof (KeySym) * num_symbols_supported);
    feedc->ctrl.symbols_displayed = (KeySym *) 
	xalloc (sizeof (KeySym) * max_symbols);
    if (!feedc->ctrl.symbols_supported || !feedc->ctrl.symbols_displayed)
    {
	if (feedc->ctrl.symbols_supported)
	    xfree(feedc->ctrl.symbols_supported);
	if (feedc->ctrl.symbols_displayed)
	    xfree(feedc->ctrl.symbols_displayed);
	xfree(feedc);
	return FALSE;
    }
    for (i=0; i<num_symbols_supported; i++)
	*(feedc->ctrl.symbols_supported+i) = *symbols++;
    for (i=0; i<max_symbols; i++)
	*(feedc->ctrl.symbols_displayed+i) = (KeySym) NULL;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->stringfeed) )
	feedc->ctrl.id = dev->stringfeed->ctrl.id + 1;
    dev->stringfeed = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}

_X_EXPORT Bool
InitBellFeedbackClassDeviceStruct (DeviceIntPtr dev, BellProcPtr bellProc, 
                                   BellCtrlProcPtr controlProc)
{
    BellFeedbackPtr feedc;

    feedc = (BellFeedbackPtr)xalloc(sizeof(BellFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->BellProc = bellProc;
    feedc->ctrl = defaultBellControl;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->bell) )
	feedc->ctrl.id = dev->bell->ctrl.id + 1;
    dev->bell = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}

_X_EXPORT Bool
InitLedFeedbackClassDeviceStruct (DeviceIntPtr dev, LedCtrlProcPtr controlProc)
{
    LedFeedbackPtr feedc;

    feedc = (LedFeedbackPtr)xalloc(sizeof(LedFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->ctrl = defaultLedControl;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->leds) )
	feedc->ctrl.id = dev->leds->ctrl.id + 1;
#ifdef XKB
    feedc->xkb_sli= NULL;
#endif
    dev->leds = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}

_X_EXPORT Bool
InitIntegerFeedbackClassDeviceStruct (DeviceIntPtr dev, IntegerCtrlProcPtr controlProc)
{
    IntegerFeedbackPtr feedc;

    feedc = (IntegerFeedbackPtr)xalloc(sizeof(IntegerFeedbackClassRec));
    if (!feedc)
	return FALSE;
    feedc->CtrlProc = controlProc;
    feedc->ctrl = defaultIntegerControl;
    feedc->ctrl.id = 0;
    if ( (feedc->next = dev->intfeed) )
	feedc->ctrl.id = dev->intfeed->ctrl.id + 1;
    dev->intfeed = feedc;
    (*controlProc)(dev, &feedc->ctrl);
    return TRUE;
}

_X_EXPORT Bool
InitPointerDeviceStruct(DevicePtr device, CARD8 *map, int numButtons, 
                        ValuatorMotionProcPtr motionProc, 
                        PtrCtrlProcPtr controlProc, int numMotionEvents,
                        int numAxes)
{
    DeviceIntPtr dev = (DeviceIntPtr)device;

    return(InitButtonClassDeviceStruct(dev, numButtons, map) &&
	   InitValuatorClassDeviceStruct(dev, numAxes, motionProc,
					 numMotionEvents, 0) &&
	   InitPtrFeedbackClassDeviceStruct(dev, controlProc));
}

_X_EXPORT Bool
InitKeyboardDeviceStruct(DevicePtr device, KeySymsPtr pKeySyms, 
                         CARD8 pModifiers[], BellProcPtr bellProc, 
                         KbdCtrlProcPtr controlProc) 
{
    DeviceIntPtr dev = (DeviceIntPtr)device;

    return(InitKeyClassDeviceStruct(dev, pKeySyms, pModifiers) &&
	   InitFocusClassDeviceStruct(dev) &&
	   InitKbdFeedbackClassDeviceStruct(dev, bellProc, controlProc));
}

_X_EXPORT void
SendMappingNotify(unsigned request, unsigned firstKeyCode, unsigned count, 
                  ClientPtr client)
{
    int i;
    xEvent event;

    event.u.u.type = MappingNotify;
    event.u.mappingNotify.request = request;
    if (request == MappingKeyboard)
    {
        event.u.mappingNotify.firstKeyCode = firstKeyCode;
        event.u.mappingNotify.count = count;
    }
#ifdef XKB
    if (!noXkbExtension &&
	((request == MappingKeyboard) || (request == MappingModifier)))
        XkbApplyMappingChange(inputInfo.keyboard, request, firstKeyCode, count,
                              client);
#endif

   /* 0 is the server client */
    for (i=1; i<currentMaxClients; i++)
    {
	if (clients[i] && clients[i]->clientState == ClientStateRunning)
	{
#ifdef XKB
	    if (!noXkbExtension &&
		(request == MappingKeyboard) &&
		(clients[i]->xkbClientFlags != 0) &&
		(clients[i]->mapNotifyMask&XkbKeySymsMask))
		continue;
#endif
	    event.u.u.sequenceNumber = clients[i]->sequence;
	    WriteEventsToClient(clients[i], 1, &event);
	}
    }
}

/*
 * n-squared algorithm. n < 255 and don't want to copy the whole thing and
 * sort it to do the checking. How often is it called? Just being lazy?
 */
Bool
BadDeviceMap(BYTE *buff, int length, unsigned low, unsigned high, XID *errval)
{
    int i, j;

    for (i = 0; i < length; i++)
	if (buff[i])		       /* only check non-zero elements */
	{
	    if ((low > buff[i]) || (high < buff[i]))
	    {
		*errval = buff[i];
		return TRUE;
	    }
	    for (j = i + 1; j < length; j++)
		if (buff[i] == buff[j])
		{
		    *errval = buff[i];
		    return TRUE;
		}
	}
    return FALSE;
}

Bool
AllModifierKeysAreUp(dev, map1, per1, map2, per2)
    DeviceIntPtr dev;
    CARD8 *map1, *map2;
    int per1, per2;
{
    int i, j, k;
    CARD8 *down = dev->key->down;

    for (i = 8; --i >= 0; map2 += per2)
    {
	for (j = per1; --j >= 0; map1++)
	{
	    if (*map1 && BitIsOn(down, *map1))
	    {
		for (k = per2; (--k >= 0) && (*map1 != map2[k]);)
		  ;
		if (k < 0)
		    return FALSE;
	    }
	}
    }
    return TRUE;
}

static int
DoSetModifierMapping(ClientPtr client, KeyCode *inputMap,
                     int numKeyPerModifier, xSetModifierMappingReply *rep)
{
    DeviceIntPtr pDev = NULL;
    int rc, i = 0, inputMapLen = numKeyPerModifier * 8;

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if ((pDev->coreEvents || pDev == inputInfo.keyboard) && pDev->key) {
            for (i = 0; i < inputMapLen; i++) {
                /* Check that all the new modifiers fall within the advertised
                 * keycode range, and are okay with the DDX. */
                if (inputMap[i] && ((inputMap[i] < pDev->key->curKeySyms.minKeyCode ||
                                    inputMap[i] > pDev->key->curKeySyms.maxKeyCode) ||
                                    !LegalModifier(inputMap[i], pDev))) {
                    client->errorValue = inputMap[i];
                    return BadValue;
                }
            }

	    rc = XaceHook(XACE_DEVICE_ACCESS, client, pDev, DixManageAccess);
	    if (rc != Success)
		return rc;

            /* None of the modifiers (old or new) may be down while we change
             * the map. */
            if (!AllModifierKeysAreUp(pDev, pDev->key->modifierKeyMap,
                                      pDev->key->maxKeysPerModifier,
                                      inputMap, numKeyPerModifier) ||
                !AllModifierKeysAreUp(pDev, inputMap, numKeyPerModifier,
                                      pDev->key->modifierKeyMap,
                                      pDev->key->maxKeysPerModifier)) {
		rep->success = MappingBusy;
                return Success;
            }
        }
    }

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {

        if ((pDev->coreEvents || pDev == inputInfo.keyboard) && pDev->key) {
            bzero(pDev->key->modifierMap, MAP_LENGTH);

            /* Annoyingly, we lack a modifierKeyMap size, so we have to just free
             * and re-alloc it every time. */
            if (pDev->key->modifierKeyMap)
                xfree(pDev->key->modifierKeyMap);

            if (inputMapLen) {
                pDev->key->modifierKeyMap = (KeyCode *) xalloc(inputMapLen);
                if (!pDev->key->modifierKeyMap)
                    return BadAlloc;

                memcpy(pDev->key->modifierKeyMap, inputMap, inputMapLen);
                pDev->key->maxKeysPerModifier = numKeyPerModifier;

                for (i = 0; i < inputMapLen; i++) {
                    if (inputMap[i]) {
                        pDev->key->modifierMap[inputMap[i]] |=
                            (1 << (((unsigned int)i) / numKeyPerModifier));
                    }
                }
            }
            else {
                pDev->key->modifierKeyMap = NULL;
                pDev->key->maxKeysPerModifier = 0;
            }
        }
    }

    rep->success = Success;
    return Success;
}

int 
ProcSetModifierMapping(ClientPtr client)
{
    xSetModifierMappingReply rep;
    DeviceIntPtr dev;
    int rc;
    REQUEST(xSetModifierMappingReq);
    REQUEST_AT_LEAST_SIZE(xSetModifierMappingReq);

    if (client->req_len != ((stuff->numKeyPerModifier << 1) +
			    (sizeof (xSetModifierMappingReq) >> 2)))
	return BadLength;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    rc = DoSetModifierMapping(client, (KeyCode *)&stuff[1],
			      stuff->numKeyPerModifier, &rep);
    if (rc != Success)
	return rc;

    SendMappingNotify(MappingModifier, 0, 0, client);
    for (dev = inputInfo.devices; dev; dev = dev->next)
        if (dev->key && dev->coreEvents)
            SendDeviceMappingNotify(client, MappingModifier, 0, 0, dev);
    WriteReplyToClient(client, sizeof(xSetModifierMappingReply), &rep);
    return client->noClientException;
}

int
ProcGetModifierMapping(ClientPtr client)
{
    xGetModifierMappingReply rep;
    KeyClassPtr keyc = inputInfo.keyboard->key;
    int rc;
    REQUEST_SIZE_MATCH(xReq);

    rc = XaceHook(XACE_DEVICE_ACCESS, client, inputInfo.keyboard,
		  DixGetAttrAccess);
    if (rc != Success)
	return rc;

    rep.type = X_Reply;
    rep.numKeyPerModifier = keyc->maxKeysPerModifier;
    rep.sequenceNumber = client->sequence;
    /* length counts 4 byte quantities - there are 8 modifiers 1 byte big */
    rep.length = keyc->maxKeysPerModifier << 1;

    WriteReplyToClient(client, sizeof(xGetModifierMappingReply), &rep);

    /* Use the (modified by DDX) map that SetModifierMapping passed in */
    (void)WriteToClient(client, (int)(keyc->maxKeysPerModifier << 3),
			(char *)keyc->modifierKeyMap);
    return client->noClientException;
}

int
ProcChangeKeyboardMapping(ClientPtr client)
{
    REQUEST(xChangeKeyboardMappingReq);
    unsigned len;
    KeySymsRec keysyms;
    KeySymsPtr curKeySyms = &inputInfo.keyboard->key->curKeySyms;
    DeviceIntPtr pDev = NULL;
    int rc;
    REQUEST_AT_LEAST_SIZE(xChangeKeyboardMappingReq);

    len = client->req_len - (sizeof(xChangeKeyboardMappingReq) >> 2);  
    if (len != (stuff->keyCodes * stuff->keySymsPerKeyCode))
            return BadLength;

    if ((stuff->firstKeyCode < curKeySyms->minKeyCode) ||
	(stuff->firstKeyCode > curKeySyms->maxKeyCode)) {
	    client->errorValue = stuff->firstKeyCode;
	    return BadValue;

    }
    if (((unsigned)(stuff->firstKeyCode + stuff->keyCodes - 1) >
        curKeySyms->maxKeyCode) || (stuff->keySymsPerKeyCode == 0)) {
	    client->errorValue = stuff->keySymsPerKeyCode;
	    return BadValue;
    }

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if ((pDev->coreEvents || pDev == inputInfo.keyboard) && pDev->key) {
            rc = XaceHook(XACE_DEVICE_ACCESS, client, pDev, DixManageAccess);
	    if (rc != Success)
                return rc;
        }
    }

    keysyms.minKeyCode = stuff->firstKeyCode;
    keysyms.maxKeyCode = stuff->firstKeyCode + stuff->keyCodes - 1;
    keysyms.mapWidth = stuff->keySymsPerKeyCode;
    keysyms.map = (KeySym *)&stuff[1];
    for (pDev = inputInfo.devices; pDev; pDev = pDev->next)
        if ((pDev->coreEvents || pDev == inputInfo.keyboard) && pDev->key)
            if (!SetKeySymsMap(&pDev->key->curKeySyms, &keysyms))
                return BadAlloc;

    SendMappingNotify(MappingKeyboard, stuff->firstKeyCode, stuff->keyCodes,
                      client);
    for (pDev = inputInfo.devices; pDev; pDev = pDev->next)
        if (pDev->key && pDev->coreEvents)
            SendDeviceMappingNotify(client, MappingKeyboard,
                                    stuff->firstKeyCode, stuff->keyCodes,
                                    pDev);

    return client->noClientException;
}

static int
DoSetPointerMapping(ClientPtr client, DeviceIntPtr device, BYTE *map, int n)
{
    int rc, i = 0;
    DeviceIntPtr dev = NULL;

    if (!device || !device->button)
        return BadDevice;

    for (dev = inputInfo.devices; dev; dev = dev->next) {
        if ((dev->coreEvents || dev == inputInfo.pointer) && dev->button) {
	    rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixManageAccess);
	    if (rc != Success)
		return rc;
	}
    }

    for (dev = inputInfo.devices; dev; dev = dev->next) {
        if ((dev->coreEvents || dev == inputInfo.pointer) && dev->button) {
            for (i = 0; i < n; i++) {
                if ((device->button->map[i + 1] != map[i]) &&
                    BitIsOn(device->button->down, i + 1)) {
                    return MappingBusy;
                }
            }
        }
    }

    for (dev = inputInfo.devices; dev; dev = dev->next) {
        if ((dev->coreEvents || dev == inputInfo.pointer) && dev->button) {
            for (i = 0; i < n; i++)
                dev->button->map[i + 1] = map[i];
        }
    }

    return Success;
}

int
ProcSetPointerMapping(ClientPtr client)
{
    BYTE *map;
    int ret;
    xSetPointerMappingReply rep;
    REQUEST(xSetPointerMappingReq);
    REQUEST_AT_LEAST_SIZE(xSetPointerMappingReq);

    if (client->req_len != (sizeof(xSetPointerMappingReq)+stuff->nElts+3) >> 2)
	return BadLength;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.success = MappingSuccess;
    map = (BYTE *)&stuff[1];
    
    /* So we're bounded here by the number of core buttons.  This check
     * probably wants disabling through XFixes. */
    if (stuff->nElts != inputInfo.pointer->button->numButtons) {
	client->errorValue = stuff->nElts;
	return BadValue;
    }
    if (BadDeviceMap(&map[0], (int)stuff->nElts, 1, 255, &client->errorValue))
	return BadValue;

    ret = DoSetPointerMapping(client, inputInfo.pointer, map, stuff->nElts);
    if (ret != Success) {
        rep.success = ret;
        WriteReplyToClient(client, sizeof(xSetPointerMappingReply), &rep);
        return Success;
    }

    /* FIXME: Send mapping notifies for all the extended devices as well. */
    SendMappingNotify(MappingPointer, 0, 0, client);
    WriteReplyToClient(client, sizeof(xSetPointerMappingReply), &rep);
    return Success;
}

int
ProcGetKeyboardMapping(ClientPtr client)
{
    xGetKeyboardMappingReply rep;
    KeySymsPtr curKeySyms = &inputInfo.keyboard->key->curKeySyms;
    int rc;
    REQUEST(xGetKeyboardMappingReq);
    REQUEST_SIZE_MATCH(xGetKeyboardMappingReq);

    rc = XaceHook(XACE_DEVICE_ACCESS, client, inputInfo.keyboard,
		  DixGetAttrAccess);
    if (rc != Success)
	return rc;

    if ((stuff->firstKeyCode < curKeySyms->minKeyCode) ||
        (stuff->firstKeyCode > curKeySyms->maxKeyCode)) {
	client->errorValue = stuff->firstKeyCode;
	return BadValue;
    }
    if (stuff->firstKeyCode + stuff->count >
	(unsigned)(curKeySyms->maxKeyCode + 1)) {
	client->errorValue = stuff->count;
        return BadValue;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.keySymsPerKeyCode = curKeySyms->mapWidth;
    /* length is a count of 4 byte quantities and KeySyms are 4 bytes */
    rep.length = (curKeySyms->mapWidth * stuff->count);
    WriteReplyToClient(client, sizeof(xGetKeyboardMappingReply), &rep);
    client->pSwapReplyFunc = (ReplySwapPtr) CopySwap32Write;
    WriteSwappedDataToClient(
	client,
	curKeySyms->mapWidth * stuff->count * sizeof(KeySym),
	&curKeySyms->map[(stuff->firstKeyCode - curKeySyms->minKeyCode) *
			 curKeySyms->mapWidth]);

    return client->noClientException;
}

int
ProcGetPointerMapping(ClientPtr client)
{
    xGetPointerMappingReply rep;
    ButtonClassPtr butc = inputInfo.pointer->button;
    int rc;
    REQUEST_SIZE_MATCH(xReq);

    rc = XaceHook(XACE_DEVICE_ACCESS, client, inputInfo.pointer,
		  DixGetAttrAccess);
    if (rc != Success)
	return rc;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.nElts = butc->numButtons;
    rep.length = ((unsigned)rep.nElts + (4-1))/4;
    WriteReplyToClient(client, sizeof(xGetPointerMappingReply), &rep);
    (void)WriteToClient(client, (int)rep.nElts, (char *)&butc->map[1]);
    return Success;    
}

void
NoteLedState(DeviceIntPtr keybd, int led, Bool on)
{
    KeybdCtrl *ctrl = &keybd->kbdfeed->ctrl;
    if (on)
	ctrl->leds |= ((Leds)1 << (led - 1));
    else
	ctrl->leds &= ~((Leds)1 << (led - 1));
}

_X_EXPORT int
Ones(unsigned long mask)             /* HACKMEM 169 */
{
    unsigned long y;

    y = (mask >> 1) &033333333333;
    y = mask - y - ((y >>1) & 033333333333);
    return (((y + (y >> 3)) & 030707070707) % 077);
}

static int
DoChangeKeyboardControl (ClientPtr client, DeviceIntPtr keybd, XID *vlist,
                         BITS32 vmask)
{
#define DO_ALL    (-1)
    KeybdCtrl ctrl;
    int t;
    int led = DO_ALL;
    int key = DO_ALL;
    BITS32 index2;
    int mask = vmask, i;

    ctrl = keybd->kbdfeed->ctrl;
    while (vmask) {
	index2 = (BITS32) lowbit (vmask);
	vmask &= ~index2;
	switch (index2) {
	case KBKeyClickPercent: 
	    t = (INT8)*vlist;
	    vlist++;
	    if (t == -1) {
		t = defaultKeyboardControl.click;
            }
	    else if (t < 0 || t > 100) {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.click = t;
	    break;
	case KBBellPercent:
	    t = (INT8)*vlist;
	    vlist++;
	    if (t == -1) {
		t = defaultKeyboardControl.bell;
            }
	    else if (t < 0 || t > 100) {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell = t;
	    break;
	case KBBellPitch:
	    t = (INT16)*vlist;
	    vlist++;
	    if (t == -1) {
		t = defaultKeyboardControl.bell_pitch;
            }
	    else if (t < 0) {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell_pitch = t;
	    break;
	case KBBellDuration:
	    t = (INT16)*vlist;
	    vlist++;
	    if (t == -1)
		t = defaultKeyboardControl.bell_duration;
	    else if (t < 0) {
		client->errorValue = t;
		return BadValue;
	    }
	    ctrl.bell_duration = t;
	    break;
	case KBLed:
	    led = (CARD8)*vlist;
	    vlist++;
	    if (led < 1 || led > 32) {
		client->errorValue = led;
		return BadValue;
	    }
	    if (!(mask & KBLedMode))
		return BadMatch;
	    break;
	case KBLedMode:
	    t = (CARD8)*vlist;
	    vlist++;
	    if (t == LedModeOff) {
		if (led == DO_ALL)
		    ctrl.leds = 0x0;
		else
		    ctrl.leds &= ~(((Leds)(1)) << (led - 1));
	    }
	    else if (t == LedModeOn) {
		if (led == DO_ALL)
		    ctrl.leds = ~0L;
		else
		    ctrl.leds |= (((Leds)(1)) << (led - 1));
	    }
	    else {
		client->errorValue = t;
		return BadValue;
	    }
#ifdef XKB
            if (!noXkbExtension) {
                XkbEventCauseRec cause;
                XkbSetCauseCoreReq(&cause,X_ChangeKeyboardControl,client);
                XkbSetIndicators(keybd,((led == DO_ALL) ? ~0L : (1L<<(led-1))),
				 			ctrl.leds, &cause);
                ctrl.leds = keybd->kbdfeed->ctrl.leds;
            }
#endif
	    break;
	case KBKey:
	    key = (KeyCode)*vlist;
	    vlist++;
	    if ((KeyCode)key < inputInfo.keyboard->key->curKeySyms.minKeyCode ||
		(KeyCode)key > inputInfo.keyboard->key->curKeySyms.maxKeyCode) {
		client->errorValue = key;
		return BadValue;
	    }
	    if (!(mask & KBAutoRepeatMode))
		return BadMatch;
	    break;
	case KBAutoRepeatMode:
	    i = (key >> 3);
	    mask = (1 << (key & 7));
	    t = (CARD8)*vlist;
	    vlist++;
#ifdef XKB
            if (!noXkbExtension && key != DO_ALL)
                XkbDisableComputedAutoRepeats(keybd,key);
#endif
	    if (t == AutoRepeatModeOff) {
		if (key == DO_ALL)
		    ctrl.autoRepeat = FALSE;
		else
		    ctrl.autoRepeats[i] &= ~mask;
	    }
	    else if (t == AutoRepeatModeOn) {
		if (key == DO_ALL)
		    ctrl.autoRepeat = TRUE;
		else
		    ctrl.autoRepeats[i] |= mask;
	    }
	    else if (t == AutoRepeatModeDefault) {
		if (key == DO_ALL)
		    ctrl.autoRepeat = defaultKeyboardControl.autoRepeat;
		else
		    ctrl.autoRepeats[i] =
			    (ctrl.autoRepeats[i] & ~mask) |
			    (defaultKeyboardControl.autoRepeats[i] & mask);
	    }
	    else {
		client->errorValue = t;
		return BadValue;
	    }
	    break;
	default:
	    client->errorValue = mask;
	    return BadValue;
	}
    }
    keybd->kbdfeed->ctrl = ctrl;

#ifdef XKB
    /* The XKB RepeatKeys control and core protocol global autorepeat */
    /* value are linked	*/
    if (!noXkbExtension)
        XkbSetRepeatKeys(keybd, key, keybd->kbdfeed->ctrl.autoRepeat);
    else
#endif
        (*keybd->kbdfeed->CtrlProc)(keybd, &keybd->kbdfeed->ctrl);

    return Success;

#undef DO_ALL
} 

int
ProcChangeKeyboardControl (ClientPtr client)
{
    XID *vlist;
    BITS32 vmask;
    int ret = Success, error = Success;
    DeviceIntPtr pDev = NULL;
    REQUEST(xChangeKeyboardControlReq);

    REQUEST_AT_LEAST_SIZE(xChangeKeyboardControlReq);

    vmask = stuff->mask;
    vlist = (XID *)&stuff[1];

    if (client->req_len != (sizeof(xChangeKeyboardControlReq)>>2)+Ones(vmask))
	return BadLength;

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if ((pDev->coreEvents || pDev == inputInfo.keyboard) &&
            pDev->kbdfeed && pDev->kbdfeed->CtrlProc) {
            ret = XaceHook(XACE_DEVICE_ACCESS, client, pDev, DixManageAccess);
	    if (ret != Success)
                return ret;
        }
    }

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next) {
        if ((pDev->coreEvents || pDev == inputInfo.keyboard) &&
            pDev->kbdfeed && pDev->kbdfeed->CtrlProc) {
            ret = DoChangeKeyboardControl(client, pDev, vlist, vmask);
            if (ret != Success)
                error = ret;
        }
    }

    return error;
}

int
ProcGetKeyboardControl (ClientPtr client)
{
    int rc, i;
    KeybdCtrl *ctrl = &inputInfo.keyboard->kbdfeed->ctrl;
    xGetKeyboardControlReply rep;
    REQUEST_SIZE_MATCH(xReq);

    rc = XaceHook(XACE_DEVICE_ACCESS, client, inputInfo.keyboard,
		  DixGetAttrAccess);
    if (rc != Success)
	return rc;

    rep.type = X_Reply;
    rep.length = 5;
    rep.sequenceNumber = client->sequence;
    rep.globalAutoRepeat = ctrl->autoRepeat;
    rep.keyClickPercent = ctrl->click;
    rep.bellPercent = ctrl->bell;
    rep.bellPitch = ctrl->bell_pitch;
    rep.bellDuration = ctrl->bell_duration;
    rep.ledMask = ctrl->leds;
    for (i = 0; i < 32; i++)
	rep.map[i] = ctrl->autoRepeats[i];
    WriteReplyToClient(client, sizeof(xGetKeyboardControlReply), &rep);
    return Success;
} 

int
ProcBell(ClientPtr client)
{
    DeviceIntPtr keybd = inputInfo.keyboard;
    int base = keybd->kbdfeed->ctrl.bell;
    int newpercent;
    int rc;
    REQUEST(xBellReq);
    REQUEST_SIZE_MATCH(xBellReq);

    if (!keybd->kbdfeed->BellProc)
        return BadDevice;
    
    if (stuff->percent < -100 || stuff->percent > 100) {
	client->errorValue = stuff->percent;
	return BadValue;
    }

    newpercent = (base * stuff->percent) / 100;
    if (stuff->percent < 0)
        newpercent = base + newpercent;
    else
    	newpercent = base - newpercent + stuff->percent;

    for (keybd = inputInfo.devices; keybd; keybd = keybd->next) {
        if ((keybd->coreEvents || keybd == inputInfo.keyboard) &&
            keybd->kbdfeed && keybd->kbdfeed->BellProc) {

	    rc = XaceHook(XACE_DEVICE_ACCESS, client, keybd, DixBellAccess);
	    if (rc != Success)
		return rc;
#ifdef XKB
            if (!noXkbExtension)
                XkbHandleBell(FALSE, FALSE, keybd, newpercent,
                              &keybd->kbdfeed->ctrl, 0, None, NULL, client);
            else
#endif
                (*keybd->kbdfeed->BellProc)(newpercent, keybd,
                                            &keybd->kbdfeed->ctrl, 0);
        }
    }

    return Success;
} 

int
ProcChangePointerControl(ClientPtr client)
{
    DeviceIntPtr mouse = inputInfo.pointer;
    PtrCtrl ctrl;		/* might get BadValue part way through */
    int rc;
    REQUEST(xChangePointerControlReq);
    REQUEST_SIZE_MATCH(xChangePointerControlReq);
    
    if (!mouse->ptrfeed->CtrlProc)
        return BadDevice;
    
    ctrl = mouse->ptrfeed->ctrl;
    if ((stuff->doAccel != xTrue) && (stuff->doAccel != xFalse)) {
	client->errorValue = stuff->doAccel;
	return(BadValue);
    }
    if ((stuff->doThresh != xTrue) && (stuff->doThresh != xFalse)) {
	client->errorValue = stuff->doThresh;
	return(BadValue);
    }
    if (stuff->doAccel) {
	if (stuff->accelNum == -1) {
	    ctrl.num = defaultPointerControl.num;
        }
	else if (stuff->accelNum < 0) {
	    client->errorValue = stuff->accelNum;
	    return BadValue;
	}
	else {
            ctrl.num = stuff->accelNum;
        }

	if (stuff->accelDenum == -1) {
	    ctrl.den = defaultPointerControl.den;
        }
	else if (stuff->accelDenum <= 0) {
	    client->errorValue = stuff->accelDenum;
	    return BadValue;
	}
	else {
            ctrl.den = stuff->accelDenum;
        }
    }
    if (stuff->doThresh) {
	if (stuff->threshold == -1) {
	    ctrl.threshold = defaultPointerControl.threshold;
        }
	else if (stuff->threshold < 0) {
	    client->errorValue = stuff->threshold;
	    return BadValue;
	}
	else {
            ctrl.threshold = stuff->threshold;
        }
    }

    for (mouse = inputInfo.devices; mouse; mouse = mouse->next) {
        if ((mouse->coreEvents || mouse == inputInfo.pointer) &&
            mouse->ptrfeed && mouse->ptrfeed->CtrlProc) {
	    rc = XaceHook(XACE_DEVICE_ACCESS, client, mouse, DixManageAccess);
	    if (rc != Success)
		return rc;
	}
    }

    for (mouse = inputInfo.devices; mouse; mouse = mouse->next) {
        if ((mouse->coreEvents || mouse == inputInfo.pointer) &&
            mouse->ptrfeed && mouse->ptrfeed->CtrlProc) {
            mouse->ptrfeed->ctrl = ctrl;
            (*mouse->ptrfeed->CtrlProc)(mouse, &mouse->ptrfeed->ctrl);
        }
    }

    return Success;
} 

int
ProcGetPointerControl(ClientPtr client)
{
    PtrCtrl *ctrl = &inputInfo.pointer->ptrfeed->ctrl;
    xGetPointerControlReply rep;
    int rc;
    REQUEST_SIZE_MATCH(xReq);

    rc = XaceHook(XACE_DEVICE_ACCESS, client, inputInfo.pointer,
		  DixGetAttrAccess);
    if (rc != Success)
	return rc;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.threshold = ctrl->threshold;
    rep.accelNumerator = ctrl->num;
    rep.accelDenominator = ctrl->den;
    WriteReplyToClient(client, sizeof(xGenericReply), &rep);
    return Success;
}

void
MaybeStopHint(DeviceIntPtr dev, ClientPtr client)
{
    GrabPtr grab = dev->grab;

    if ((grab && SameClient(grab, client) &&
	 ((grab->eventMask & PointerMotionHintMask) ||
	  (grab->ownerEvents &&
	   (EventMaskForClient(dev->valuator->motionHintWindow, client) &
	    PointerMotionHintMask)))) ||
	(!grab &&
	 (EventMaskForClient(dev->valuator->motionHintWindow, client) &
	  PointerMotionHintMask)))
	dev->valuator->motionHintWindow = NullWindow;
}

int
ProcGetMotionEvents(ClientPtr client)
{
    WindowPtr pWin;
    xTimecoord * coords = (xTimecoord *) NULL;
    xGetMotionEventsReply rep;
    int i, count, xmin, xmax, ymin, ymax, rc;
    unsigned long nEvents;
    DeviceIntPtr mouse = inputInfo.pointer;
    TimeStamp start, stop;
    REQUEST(xGetMotionEventsReq);
    REQUEST_SIZE_MATCH(xGetMotionEventsReq);

    rc = dixLookupWindow(&pWin, stuff->window, client, DixGetAttrAccess);
    if (rc != Success)
	return rc;
    rc = XaceHook(XACE_DEVICE_ACCESS, client, mouse, DixReadAccess);
    if (rc != Success)
	return rc;

    if (mouse->valuator->motionHintWindow)
	MaybeStopHint(mouse, client);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    nEvents = 0;
    start = ClientTimeToServerTime(stuff->start);
    stop = ClientTimeToServerTime(stuff->stop);
    if ((CompareTimeStamps(start, stop) != LATER) &&
	(CompareTimeStamps(start, currentTime) != LATER) &&
	mouse->valuator->numMotionEvents)
    {
	if (CompareTimeStamps(stop, currentTime) == LATER)
	    stop = currentTime;
	coords = (xTimecoord *)xalloc(mouse->valuator->numMotionEvents
					      * sizeof(xTimecoord));
	if (!coords)
	    return BadAlloc;
	count = (*mouse->valuator->GetMotionProc) (mouse, coords,
						   start.milliseconds,
						   stop.milliseconds,
						   pWin->drawable.pScreen);
	xmin = pWin->drawable.x - wBorderWidth (pWin);
	xmax = pWin->drawable.x + (int)pWin->drawable.width +
		wBorderWidth (pWin);
	ymin = pWin->drawable.y - wBorderWidth (pWin);
	ymax = pWin->drawable.y + (int)pWin->drawable.height +
		wBorderWidth (pWin);
	for (i = 0; i < count; i++)
	    if ((xmin <= coords[i].x) && (coords[i].x < xmax) &&
		    (ymin <= coords[i].y) && (coords[i].y < ymax))
	    {
		coords[nEvents].time = coords[i].time;
		coords[nEvents].x = coords[i].x - pWin->drawable.x;
		coords[nEvents].y = coords[i].y - pWin->drawable.y;
		nEvents++;
	    }
    }
    rep.length = nEvents * (sizeof(xTimecoord) >> 2);
    rep.nEvents = nEvents;
    WriteReplyToClient(client, sizeof(xGetMotionEventsReply), &rep);
    if (nEvents)
    {
	client->pSwapReplyFunc = (ReplySwapPtr) SwapTimeCoordWrite;
	WriteSwappedDataToClient(client, nEvents * sizeof(xTimecoord),
				 (char *)coords);
    }
    if (coords)
	xfree(coords);
    return Success;
}

int
ProcQueryKeymap(ClientPtr client)
{
    xQueryKeymapReply rep;
    int rc, i;
    CARD8 *down = inputInfo.keyboard->key->down;

    REQUEST_SIZE_MATCH(xReq);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 2;

    rc = XaceHook(XACE_DEVICE_ACCESS, client, inputInfo.keyboard,
		  DixReadAccess);
    if (rc != Success)
	return rc;

    for (i = 0; i<32; i++)
	rep.map[i] = down[i];

    WriteReplyToClient(client, sizeof(xQueryKeymapReply), &rep);
    return Success;
}
