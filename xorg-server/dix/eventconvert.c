/*
 * Copyright © 2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * @file eventconvert.c
 * This file contains event conversion routines from InternalEvent to the
 * matching protocol events.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdint.h>
#include <X11/X.h>
#include <X11/extensions/XIproto.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2.h>

#include "dix.h"
#include "inputstr.h"
#include "misc.h"
#include "eventstr.h"
#include "exglobals.h"
#include "eventconvert.h"
#include "xiquerydevice.h"
#include "xkbsrv.h"


static int countValuators(DeviceEvent *ev, int *first);
static int getValuatorEvents(DeviceEvent *ev, deviceValuator *xv);
static int eventToKeyButtonPointer(DeviceEvent *ev, xEvent **xi, int *count);
static int eventToDeviceChanged(DeviceChangedEvent *ev, xEvent **dcce);
static int eventToDeviceEvent(DeviceEvent *ev, xEvent **xi);
static int eventToRawEvent(RawDeviceEvent *ev, xEvent **xi);

/* Do not use, read comments below */
BOOL EventIsKeyRepeat(xEvent *event);

/**
 * Hack to allow detectable autorepeat for core and XI1 events.
 * The sequence number is unused until we send to the client and can be
 * misused to store data. More or less, anyway.
 *
 * Do not use this. It may change any time without warning, eat your babies
 * and piss on your cat.
 */
static void
EventSetKeyRepeatFlag(xEvent *event, BOOL on)
{
    event->u.u.sequenceNumber = on;
}

/**
 * Check if the event was marked as a repeat event before.
 * NOTE: This is a nasty hack and should NOT be used by anyone else but
 * TryClientEvents.
 */
BOOL
EventIsKeyRepeat(xEvent *event)
{
    return !!event->u.u.sequenceNumber;
}

/**
 * Convert the given event to the respective core event.
 *
 * Return values:
 * Success ... core contains the matching core event.
 * BadValue .. One or more values in the internal event are invalid.
 * BadMatch .. The event has no core equivalent.
 *
 * @param[in] event The event to convert into a core event.
 * @param[in] core The memory location to store the core event at.
 * @return Success or the matching error code.
 */
int
EventToCore(InternalEvent *event, xEvent *core)
{
    switch(event->any.type)
    {
        case ET_Motion:
        case ET_ButtonPress:
        case ET_ButtonRelease:
        case ET_KeyPress:
        case ET_KeyRelease:
            {
                DeviceEvent *e = &event->device_event;

                if (e->detail.key > 0xFF)
                    return BadMatch;

                memset(core, 0, sizeof(xEvent));
                core->u.u.type = e->type - ET_KeyPress + KeyPress;
                core->u.u.detail = e->detail.key & 0xFF;
                core->u.keyButtonPointer.time = e->time;
                core->u.keyButtonPointer.rootX = e->root_x;
                core->u.keyButtonPointer.rootY = e->root_y;
                core->u.keyButtonPointer.state = e->corestate;
                EventSetKeyRepeatFlag(core, (e->type == ET_KeyPress && e->key_repeat));
            }
            break;
        case ET_ProximityIn:
        case ET_ProximityOut:
        case ET_RawKeyPress:
        case ET_RawKeyRelease:
        case ET_RawButtonPress:
        case ET_RawButtonRelease:
        case ET_RawMotion:
            return BadMatch;
        default:
            /* XXX: */
            ErrorF("[dix] EventToCore: Not implemented yet \n");
            return BadImplementation;
    }
    return Success;
}

/**
 * Convert the given event to the respective XI 1.x event and store it in
 * xi. xi is allocated on demand and must be freed by the caller.
 * count returns the number of events in xi. If count is 1, and the type of
 * xi is GenericEvent, then xi may be larger than 32 bytes.
 *
 * Return values:
 * Success ... core contains the matching core event.
 * BadValue .. One or more values in the internal event are invalid.
 * BadMatch .. The event has no XI equivalent.
 *
 * @param[in] ev The event to convert into an XI 1 event.
 * @param[out] xi Future memory location for the XI event.
 * @param[out] count Number of elements in xi.
 *
 * @return Success or the error code.
 */
int
EventToXI(InternalEvent *ev, xEvent **xi, int *count)
{
    switch (ev->any.type)
    {
        case ET_Motion:
        case ET_ButtonPress:
        case ET_ButtonRelease:
        case ET_KeyPress:
        case ET_KeyRelease:
        case ET_ProximityIn:
        case ET_ProximityOut:
            return eventToKeyButtonPointer(&ev->device_event, xi, count);
        case ET_DeviceChanged:
        case ET_RawKeyPress:
        case ET_RawKeyRelease:
        case ET_RawButtonPress:
        case ET_RawButtonRelease:
        case ET_RawMotion:
            *count = 0;
            *xi = NULL;
            return BadMatch;
        default:
            break;
    }

    ErrorF("[dix] EventToXI: Not implemented for %d \n", ev->any.type);
    return BadImplementation;
}

/**
 * Convert the given event to the respective XI 2.x event and store it in xi.
 * xi is allocated on demand and must be freed by the caller.
 *
 * Return values:
 * Success ... core contains the matching core event.
 * BadValue .. One or more values in the internal event are invalid.
 * BadMatch .. The event has no XI2 equivalent.
 *
 * @param[in] ev The event to convert into an XI2 event
 * @param[out] xi Future memory location for the XI2 event.
 *
 * @return Success or the error code.
 */
int
EventToXI2(InternalEvent *ev, xEvent **xi)
{
    switch (ev->any.type)
    {
        /* Enter/FocusIn are for grabs. We don't need an actual event, since
         * the real events delivered are triggered elsewhere */
        case ET_Enter:
        case ET_FocusIn:
            *xi = NULL;
            return Success;
        case ET_Motion:
        case ET_ButtonPress:
        case ET_ButtonRelease:
        case ET_KeyPress:
        case ET_KeyRelease:
            return eventToDeviceEvent(&ev->device_event, xi);
        case ET_ProximityIn:
        case ET_ProximityOut:
            *xi = NULL;
            return BadMatch;
        case ET_DeviceChanged:
            return eventToDeviceChanged(&ev->changed_event, xi);
        case ET_RawKeyPress:
        case ET_RawKeyRelease:
        case ET_RawButtonPress:
        case ET_RawButtonRelease:
        case ET_RawMotion:
            return eventToRawEvent(&ev->raw_event, xi);
        default:
            break;
    }

    ErrorF("[dix] EventToXI2: Not implemented for %d \n", ev->any.type);
    return BadImplementation;
}

static int
eventToKeyButtonPointer(DeviceEvent *ev, xEvent **xi, int *count)
{
    int num_events;
    int first; /* dummy */
    deviceKeyButtonPointer *kbp;

    /* Sorry, XI 1.x protocol restrictions. */
    if (ev->detail.button > 0xFF || ev->deviceid >= 0x80)
    {
        *count = 0;
        return Success;
    }

    num_events = (countValuators(ev, &first) + 5)/6; /* valuator ev */
    num_events++; /* the actual event event */

    *xi = xcalloc(num_events, sizeof(xEvent));
    if (!(*xi))
    {
        return BadAlloc;
    }

    kbp           = (deviceKeyButtonPointer*)(*xi);
    kbp->detail   = ev->detail.button;
    kbp->time     = ev->time;
    kbp->root     = ev->root;
    kbp->root_x   = ev->root_x;
    kbp->root_y   = ev->root_y;
    kbp->deviceid = ev->deviceid;
    kbp->state    = ev->corestate;
    EventSetKeyRepeatFlag((xEvent*)kbp,
                          (ev->type == ET_KeyPress && ev->key_repeat));

    if (num_events > 1)
        kbp->deviceid |= MORE_EVENTS;

    switch(ev->type)
    {
        case ET_Motion:        kbp->type = DeviceMotionNotify;  break;
        case ET_ButtonPress:   kbp->type = DeviceButtonPress;   break;
        case ET_ButtonRelease: kbp->type = DeviceButtonRelease; break;
        case ET_KeyPress:      kbp->type = DeviceKeyPress;      break;
        case ET_KeyRelease:    kbp->type = DeviceKeyRelease;    break;
        case ET_ProximityIn:   kbp->type = ProximityIn;         break;
        case ET_ProximityOut:  kbp->type = ProximityOut;        break;
        default:
            break;
    }

    if (num_events > 1)
    {
        getValuatorEvents(ev, (deviceValuator*)(kbp + 1));
    }

    *count = num_events;
    return Success;
}


/**
 * Set first to the first valuator in the event ev and return the number of
 * valuators from first to the last set valuator.
 */
static int
countValuators(DeviceEvent *ev, int *first)
{
    int first_valuator = -1, last_valuator = -1, num_valuators = 0;
    int i;

    for (i = 0; i < sizeof(ev->valuators.mask) * 8; i++)
    {
        if (BitIsOn(ev->valuators.mask, i))
        {
            if (first_valuator == -1)
                first_valuator = i;
            last_valuator = i;
        }
    }

    if (first_valuator != -1)
    {
        num_valuators = last_valuator - first_valuator + 1;
        *first = first_valuator;
    }

    return num_valuators;
}

static int
getValuatorEvents(DeviceEvent *ev, deviceValuator *xv)
{
    int i;
    int state = 0;
    int first_valuator, num_valuators;


    num_valuators = countValuators(ev, &first_valuator);
    if (num_valuators > 0)
    {
        DeviceIntPtr dev = NULL;
        dixLookupDevice(&dev, ev->deviceid, serverClient, DixUseAccess);
        /* State needs to be assembled BEFORE the device is updated. */
        state = (dev && dev->key) ? XkbStateFieldFromRec(&dev->key->xkbInfo->state) : 0;
        state |= (dev && dev->button) ? (dev->button->state) : 0;
    }

    /* FIXME: non-continuous valuator data in internal events*/
    for (i = 0; i < num_valuators; i += 6, xv++) {
        xv->type = DeviceValuator;
        xv->first_valuator = first_valuator + i;
        xv->num_valuators = ((num_valuators - i) > 6) ? 6 : (num_valuators - i);
        xv->deviceid = ev->deviceid;
        xv->device_state = state;
        switch (xv->num_valuators) {
        case 6:
            xv->valuator5 = ev->valuators.data[i + 5];
        case 5:
            xv->valuator4 = ev->valuators.data[i + 4];
        case 4:
            xv->valuator3 = ev->valuators.data[i + 3];
        case 3:
            xv->valuator2 = ev->valuators.data[i + 2];
        case 2:
            xv->valuator1 = ev->valuators.data[i + 1];
        case 1:
            xv->valuator0 = ev->valuators.data[i + 0];
        }

        if (i + 6 < num_valuators)
            xv->deviceid |= MORE_EVENTS;
    }

    return (num_valuators + 5) / 6;
}


static int
appendKeyInfo(DeviceChangedEvent *dce, xXIKeyInfo* info)
{
    uint32_t *kc;
    int i;

    info->type = XIKeyClass;
    info->num_keycodes = dce->keys.max_keycode - dce->keys.min_keycode + 1;
    info->length = sizeof(xXIKeyInfo)/4 + info->num_keycodes;
    info->sourceid = dce->sourceid;

    kc = (uint32_t*)&info[1];
    for (i = 0; i < info->num_keycodes; i++)
        *kc++ = i + dce->keys.min_keycode;

    return info->length * 4;
}

static int
appendButtonInfo(DeviceChangedEvent *dce, xXIButtonInfo *info)
{
    unsigned char *bits;
    int mask_len;

    mask_len = bytes_to_int32(bits_to_bytes(dce->buttons.num_buttons));

    info->type = XIButtonClass;
    info->num_buttons = dce->buttons.num_buttons;
    info->length = bytes_to_int32(sizeof(xXIButtonInfo)) +
                   info->num_buttons + mask_len;
    info->sourceid = dce->sourceid;

    bits = (unsigned char*)&info[1];
    memset(bits, 0, mask_len * 4);
    /* FIXME: is_down? */

    bits += mask_len * 4;
    memcpy(bits, dce->buttons.names, dce->buttons.num_buttons * sizeof(Atom));

    return info->length * 4;
}

static int
appendValuatorInfo(DeviceChangedEvent *dce, xXIValuatorInfo *info, int axisnumber)
{
    info->type = XIValuatorClass;
    info->length = sizeof(xXIValuatorInfo)/4;
    info->label = dce->valuators[axisnumber].name;
    info->min.integral = dce->valuators[axisnumber].min;
    info->min.frac = 0;
    info->max.integral = dce->valuators[axisnumber].max;
    info->max.frac = 0;
    /* FIXME: value */
    info->value.integral = 0;
    info->value.frac = 0;
    info->resolution = dce->valuators[axisnumber].resolution;
    info->number = axisnumber;
    info->mode = dce->valuators[axisnumber].mode; /* Server doesn't have per-axis mode yet */
    info->sourceid = dce->sourceid;

    return info->length * 4;
}

static int
eventToDeviceChanged(DeviceChangedEvent *dce, xEvent **xi)
{
    xXIDeviceChangedEvent *dcce;
    int len = sizeof(xXIDeviceChangedEvent);
    int nkeys;
    char *ptr;

    if (dce->buttons.num_buttons)
    {
        len += sizeof(xXIButtonInfo);
        len += dce->buttons.num_buttons * sizeof(Atom); /* button names */
        len += pad_to_int32(bits_to_bytes(dce->buttons.num_buttons));
    }
    if (dce->num_valuators)
        len += sizeof(xXIValuatorInfo) * dce->num_valuators;

    nkeys = (dce->keys.max_keycode > 0) ?
                dce->keys.max_keycode - dce->keys.min_keycode + 1 : 0;
    if (nkeys > 0)
    {
        len += sizeof(xXIKeyInfo);
        len += sizeof(CARD32) * nkeys; /* keycodes */
    }

    dcce = xcalloc(1, len);
    if (!dcce)
    {
        ErrorF("[Xi] BadAlloc in SendDeviceChangedEvent.\n");
        return BadAlloc;
    }

    dcce->type         = GenericEvent;
    dcce->extension    = IReqCode;
    dcce->evtype       = XI_DeviceChanged;
    dcce->time         = dce->time;
    dcce->deviceid     = dce->deviceid;
    dcce->sourceid     = dce->sourceid;
    dcce->reason       = (dce->flags & DEVCHANGE_DEVICE_CHANGE) ? XIDeviceChange : XISlaveSwitch;
    dcce->num_classes  = 0;
    dcce->length = bytes_to_int32(len - sizeof(xEvent));

    ptr = (char*)&dcce[1];
    if (dce->buttons.num_buttons)
    {
        dcce->num_classes++;
        ptr += appendButtonInfo(dce, (xXIButtonInfo*)ptr);
    }

    if (nkeys)
    {
        dcce->num_classes++;
        ptr += appendKeyInfo(dce, (xXIKeyInfo*)ptr);
    }

    if (dce->num_valuators)
    {
        int i;

        dcce->num_classes += dce->num_valuators;
        for (i = 0; i < dce->num_valuators; i++)
            ptr += appendValuatorInfo(dce, (xXIValuatorInfo*)ptr, i);
    }

    *xi = (xEvent*)dcce;

    return Success;
}

static int count_bits(unsigned char* ptr, int len)
{
    int bits = 0;
    unsigned int i;
    unsigned char x;

    for (i = 0; i < len; i++)
    {
        x = ptr[i];
        while(x > 0)
        {
            bits += (x & 0x1);
            x >>= 1;
        }
    }
    return bits;
}

static int
eventToDeviceEvent(DeviceEvent *ev, xEvent **xi)
{
    int len = sizeof(xXIDeviceEvent);
    xXIDeviceEvent *xde;
    int i, btlen, vallen;
    char *ptr;
    FP3232 *axisval;

    /* FIXME: this should just send the buttons we have, not MAX_BUTTONs. Same
     * with MAX_VALUATORS below */
    /* btlen is in 4 byte units */
    btlen = bytes_to_int32(bits_to_bytes(MAX_BUTTONS));
    len += btlen * 4; /* buttonmask len */


    vallen = count_bits(ev->valuators.mask, sizeof(ev->valuators.mask)/sizeof(ev->valuators.mask[0]));
    len += vallen * 2 * sizeof(uint32_t); /* axisvalues */
    vallen = bytes_to_int32(bits_to_bytes(MAX_VALUATORS));
    len += vallen * 4; /* valuators mask */

    *xi = xcalloc(1, len);
    xde = (xXIDeviceEvent*)*xi;
    xde->type           = GenericEvent;
    xde->extension      = IReqCode;
    xde->evtype         = GetXI2Type((InternalEvent*)ev);
    xde->time           = ev->time;
    xde->length         = bytes_to_int32(len - sizeof(xEvent));
    xde->detail         = ev->detail.button;
    xde->root           = ev->root;
    xde->buttons_len    = btlen;
    xde->valuators_len  = vallen;
    xde->deviceid       = ev->deviceid;
    xde->sourceid       = ev->sourceid;
    xde->root_x         = FP1616(ev->root_x, ev->root_x_frac);
    xde->root_y         = FP1616(ev->root_y, ev->root_y_frac);

    if (ev->key_repeat)
        xde->flags      |= XIKeyRepeat;

    xde->mods.base_mods         = ev->mods.base;
    xde->mods.latched_mods      = ev->mods.latched;
    xde->mods.locked_mods       = ev->mods.locked;
    xde->mods.effective_mods    = ev->mods.effective;

    xde->group.base_group       = ev->group.base;
    xde->group.latched_group    = ev->group.latched;
    xde->group.locked_group     = ev->group.locked;
    xde->group.effective_group  = ev->group.effective;

    ptr = (char*)&xde[1];
    for (i = 0; i < sizeof(ev->buttons) * 8; i++)
    {
        if (BitIsOn(ev->buttons, i))
            SetBit(ptr, i);
    }

    ptr += xde->buttons_len * 4;
    axisval = (FP3232*)(ptr + xde->valuators_len * 4);
    for (i = 0; i < sizeof(ev->valuators.mask) * 8; i++)
    {
        if (BitIsOn(ev->valuators.mask, i))
        {
            SetBit(ptr, i);
            axisval->integral = ev->valuators.data[i];
            axisval->frac = ev->valuators.data_frac[i];
            axisval++;
        }
    }

    return Success;
}

static int
eventToRawEvent(RawDeviceEvent *ev, xEvent **xi)
{
    xXIRawEvent* raw;
    int vallen, nvals;
    int i, len = sizeof(xXIRawEvent);
    char *ptr;
    FP3232 *axisval;

    nvals = count_bits(ev->valuators.mask, sizeof(ev->valuators.mask));
    len += nvals * sizeof(FP3232) * 2; /* 8 byte per valuator, once
                                    raw, once processed */
    vallen = bytes_to_int32(bits_to_bytes(MAX_VALUATORS));
    len += vallen * 4; /* valuators mask */

    *xi = xcalloc(1, len);
    raw = (xXIRawEvent*)*xi;
    raw->type           = GenericEvent;
    raw->extension      = IReqCode;
    raw->evtype         = GetXI2Type((InternalEvent*)ev);
    raw->time           = ev->time;
    raw->length         = bytes_to_int32(len - sizeof(xEvent));
    raw->detail         = ev->detail.button;
    raw->deviceid       = ev->deviceid;
    raw->valuators_len  = vallen;

    ptr = (char*)&raw[1];
    axisval = (FP3232*)(ptr + raw->valuators_len * 4);
    for (i = 0; i < sizeof(ev->valuators.mask) * 8; i++)
    {
        if (BitIsOn(ev->valuators.mask, i))
        {
            SetBit(ptr, i);
            axisval->integral = ev->valuators.data[i];
            axisval->frac = ev->valuators.data_frac[i];
            (axisval + nvals)->integral = ev->valuators.data_raw[i];
            (axisval + nvals)->frac = ev->valuators.data_raw_frac[i];
            axisval++;
        }
    }

    return Success;
}

/**
 * Return the corresponding core type for the given event or 0 if no core
 * equivalent exists.
 */
int
GetCoreType(InternalEvent *event)
{
    int coretype = 0;
    switch(event->any.type)
    {
        case ET_Motion:         coretype = MotionNotify;  break;
        case ET_ButtonPress:    coretype = ButtonPress;   break;
        case ET_ButtonRelease:  coretype = ButtonRelease; break;
        case ET_KeyPress:       coretype = KeyPress;      break;
        case ET_KeyRelease:     coretype = KeyRelease;    break;
        default:
            break;
    }
    return coretype;
}

/**
 * Return the corresponding XI 1.x type for the given event or 0 if no
 * equivalent exists.
 */
int
GetXIType(InternalEvent *event)
{
    int xitype = 0;
    switch(event->any.type)
    {
        case ET_Motion:         xitype = DeviceMotionNotify;  break;
        case ET_ButtonPress:    xitype = DeviceButtonPress;   break;
        case ET_ButtonRelease:  xitype = DeviceButtonRelease; break;
        case ET_KeyPress:       xitype = DeviceKeyPress;      break;
        case ET_KeyRelease:     xitype = DeviceKeyRelease;    break;
        case ET_ProximityIn:    xitype = ProximityIn;         break;
        case ET_ProximityOut:   xitype = ProximityOut;        break;
        default:
            break;
    }
    return xitype;
}

/**
 * Return the corresponding XI 2.x type for the given event or 0 if no
 * equivalent exists.
 */
int
GetXI2Type(InternalEvent *event)
{
    int xi2type = 0;

    switch(event->any.type)
    {
        case ET_Motion:         xi2type = XI_Motion;           break;
        case ET_ButtonPress:    xi2type = XI_ButtonPress;      break;
        case ET_ButtonRelease:  xi2type = XI_ButtonRelease;    break;
        case ET_KeyPress:       xi2type = XI_KeyPress;         break;
        case ET_KeyRelease:     xi2type = XI_KeyRelease;       break;
        case ET_Enter:          xi2type = XI_Enter;            break;
        case ET_Leave:          xi2type = XI_Leave;            break;
        case ET_Hierarchy:      xi2type = XI_HierarchyChanged; break;
        case ET_DeviceChanged:  xi2type = XI_DeviceChanged;    break;
        case ET_RawKeyPress:    xi2type = XI_RawKeyPress;      break;
        case ET_RawKeyRelease:  xi2type = XI_RawKeyRelease;    break;
        case ET_RawButtonPress: xi2type = XI_RawButtonPress;   break;
        case ET_RawButtonRelease: xi2type = XI_RawButtonRelease; break;
        case ET_RawMotion:      xi2type = XI_RawMotion;        break;
        case ET_FocusIn:        xi2type = XI_FocusIn;          break;
        case ET_FocusOut:       xi2type = XI_FocusOut;         break;
        default:
            break;
    }
    return xi2type;
}
