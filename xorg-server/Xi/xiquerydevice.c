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
 * Authors: Peter Hutterer
 *
 */

/**
 * @file Protocol handling for the XIQueryDevice request/reply.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "inputstr.h"
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/extensions/XI2proto.h>
#include "xkbstr.h"
#include "xkbsrv.h"
#include "xserver-properties.h"
#include "exevents.h"
#include "xace.h"

#include "xiquerydevice.h"

static Bool ShouldSkipDevice(ClientPtr client, int deviceid, DeviceIntPtr d);
static int
ListDeviceInfo(ClientPtr client, DeviceIntPtr dev, xXIDeviceInfo* info);
static int SizeDeviceInfo(DeviceIntPtr dev);
static void SwapDeviceInfo(DeviceIntPtr dev, xXIDeviceInfo* info);
int
SProcXIQueryDevice(ClientPtr client)
{
    char n;

    REQUEST(xXIQueryDeviceReq);

    swaps(&stuff->length, n);
    swaps(&stuff->deviceid, n);

    return ProcXIQueryDevice(client);
}

int
ProcXIQueryDevice(ClientPtr client)
{
    xXIQueryDeviceReply rep;
    DeviceIntPtr dev = NULL;
    int rc = Success;
    int i = 0, len = 0;
    char *info, *ptr;
    Bool *skip = NULL;

    REQUEST(xXIQueryDeviceReq);
    REQUEST_SIZE_MATCH(xXIQueryDeviceReq);

    if (stuff->deviceid != XIAllDevices && stuff->deviceid != XIAllMasterDevices)
    {
        rc = dixLookupDevice(&dev, stuff->deviceid, client, DixGetAttrAccess);
        if (rc != Success)
        {
            client->errorValue = stuff->deviceid;
            return rc;
        }
        len += SizeDeviceInfo(dev);
    }
    else
    {
        skip = xcalloc(sizeof(Bool), inputInfo.numDevices);
        if (!skip)
            return BadAlloc;

        for (dev = inputInfo.devices; dev; dev = dev->next, i++)
        {
            skip[i] = ShouldSkipDevice(client, stuff->deviceid, dev);
            if (!skip[i])
                len += SizeDeviceInfo(dev);
        }

        for (dev = inputInfo.off_devices; dev; dev = dev->next, i++)
        {
            skip[i] = ShouldSkipDevice(client, stuff->deviceid, dev);
            if (!skip[i])
                len += SizeDeviceInfo(dev);
        }
    }

    info = xcalloc(1, len);
    if (!info)
        return BadAlloc;

    memset(&rep, 0, sizeof(xXIQueryDeviceReply));
    rep.repType = X_Reply;
    rep.RepType = X_XIQueryDevice;
    rep.sequenceNumber = client->sequence;
    rep.length = len/4;
    rep.num_devices = 0;

    ptr = info;
    if (dev)
    {
        len = ListDeviceInfo(client, dev, (xXIDeviceInfo*)info);
        if (client->swapped)
            SwapDeviceInfo(dev, (xXIDeviceInfo*)info);
        info += len;
        rep.num_devices = 1;
    } else
    {
        i = 0;
        for (dev = inputInfo.devices; dev; dev = dev->next, i++)
        {
            if (!skip[i])
            {
                len = ListDeviceInfo(client, dev, (xXIDeviceInfo*)info);
                if (client->swapped)
                    SwapDeviceInfo(dev, (xXIDeviceInfo*)info);
                info += len;
                rep.num_devices++;
            }
        }

        for (dev = inputInfo.off_devices; dev; dev = dev->next, i++)
        {
            if (!skip[i])
            {
                len = ListDeviceInfo(client, dev, (xXIDeviceInfo*)info);
                if (client->swapped)
                    SwapDeviceInfo(dev, (xXIDeviceInfo*)info);
                info += len;
                rep.num_devices++;
            }
        }
    }

    WriteReplyToClient(client, sizeof(xXIQueryDeviceReply), &rep);
    WriteToClient(client, rep.length * 4, ptr);
    xfree(ptr);
    xfree(skip);
    return rc;
}

void
SRepXIQueryDevice(ClientPtr client, int size, xXIQueryDeviceReply *rep)
{
    char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    swaps(&rep->num_devices, n);

    /* Device info is already swapped, see ProcXIQueryDevice */

    WriteToClient(client, size, (char *)rep);
}


/**
 * @return Whether the device should be included in the returned list.
 */
static Bool
ShouldSkipDevice(ClientPtr client, int deviceid, DeviceIntPtr dev)
{
    /* if all devices are not being queried, only master devices are */
    if (deviceid == XIAllDevices || IsMaster(dev))
    {
        int rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixGetAttrAccess);
        if (rc == Success)
            return FALSE;
    }
    return TRUE;
}

/**
 * @return The number of bytes needed to store this device's xXIDeviceInfo
 * (and its classes).
 */
static int
SizeDeviceInfo(DeviceIntPtr dev)
{
    int len = sizeof(xXIDeviceInfo);

    /* 4-padded name */
    len += pad_to_int32(strlen(dev->name));

    return len + SizeDeviceClasses(dev);

}

/*
 * @return The number of bytes needed to store this device's classes.
 */
int
SizeDeviceClasses(DeviceIntPtr dev)
{
    int len = 0;

    if (dev->button)
    {
        len += sizeof(xXIButtonInfo);
        len += dev->button->numButtons * sizeof(Atom);
        len += pad_to_int32(bits_to_bytes(dev->button->numButtons));
    }

    if (dev->key)
    {
        XkbDescPtr xkb = dev->key->xkbInfo->desc;
        len += sizeof(xXIKeyInfo);
        len += (xkb->max_key_code - xkb->min_key_code + 1) * sizeof(uint32_t);
    }

    if (dev->valuator)
        len += sizeof(xXIValuatorInfo) * dev->valuator->numAxes;

    return len;
}


/**
 * Write button information into info.
 * @return Number of bytes written into info.
 */
int
ListButtonInfo(DeviceIntPtr dev, xXIButtonInfo* info, Bool reportState)
{
    unsigned char *bits;
    int mask_len;
    int i;

    mask_len = bytes_to_int32(bits_to_bytes(dev->button->numButtons));

    info->type = ButtonClass;
    info->num_buttons = dev->button->numButtons;
    info->length = bytes_to_int32(sizeof(xXIButtonInfo)) +
                   info->num_buttons + mask_len;
    info->sourceid = dev->button->sourceid;

    bits = (unsigned char*)&info[1];
    memset(bits, 0, mask_len * 4);

    if (reportState)
	for (i = 0; dev && dev->button && i < dev->button->numButtons; i++)
	    if (BitIsOn(dev->button->down, i))
		SetBit(bits, i);

    bits += mask_len * 4;
    memcpy(bits, dev->button->labels, dev->button->numButtons * sizeof(Atom));

    return info->length * 4;
}

static void
SwapButtonInfo(DeviceIntPtr dev, xXIButtonInfo* info)
{
    char n;
    Atom *btn;
    int i;
    swaps(&info->type, n);
    swaps(&info->length, n);
    swaps(&info->sourceid, n);

    for (i = 0, btn = (Atom*)&info[1]; i < info->num_buttons; i++, btn++)
        swaps(btn, n);

    swaps(&info->num_buttons, n);
}

/**
 * Write key information into info.
 * @return Number of bytes written into info.
 */
int
ListKeyInfo(DeviceIntPtr dev, xXIKeyInfo* info)
{
    int i;
    XkbDescPtr xkb = dev->key->xkbInfo->desc;
    uint32_t *kc;

    info->type = KeyClass;
    info->num_keycodes = xkb->max_key_code - xkb->min_key_code + 1;
    info->length = sizeof(xXIKeyInfo)/4 + info->num_keycodes;
    info->sourceid = dev->key->sourceid;

    kc = (uint32_t*)&info[1];
    for (i = xkb->min_key_code; i <= xkb->max_key_code; i++, kc++)
        *kc = i;

    return info->length * 4;
}

static void
SwapKeyInfo(DeviceIntPtr dev, xXIKeyInfo* info)
{
    char n;
    uint32_t *key;
    int i;
    swaps(&info->type, n);
    swaps(&info->length, n);
    swaps(&info->sourceid, n);

    for (i = 0, key = (uint32_t*)&info[1]; i < info->num_keycodes; i++, key++)
        swapl(key, n);

    swaps(&info->num_keycodes, n);
}

/**
 * List axis information for the given axis.
 *
 * @return The number of bytes written into info.
 */
int
ListValuatorInfo(DeviceIntPtr dev, xXIValuatorInfo* info, int axisnumber,
		 Bool reportState)
{
    ValuatorClassPtr v = dev->valuator;

    info->type = ValuatorClass;
    info->length = sizeof(xXIValuatorInfo)/4;
    info->label = v->axes[axisnumber].label;
    info->min.integral = v->axes[axisnumber].min_value;
    info->min.frac = 0;
    info->max.integral = v->axes[axisnumber].max_value;
    info->max.frac = 0;
    info->value.integral = (int)v->axisVal[axisnumber];
    info->value.frac = (int)(v->axisVal[axisnumber] * (1 << 16) * (1 << 16));
    info->resolution = v->axes[axisnumber].resolution;
    info->number = axisnumber;
    info->mode = v->mode; /* Server doesn't have per-axis mode yet */
    info->sourceid = v->sourceid;

    if (!reportState)
	info->value = info->min;

    return info->length * 4;
}

static void
SwapValuatorInfo(DeviceIntPtr dev, xXIValuatorInfo* info)
{
    char n;
    swaps(&info->type, n);
    swaps(&info->length, n);
    swapl(&info->label, n);
    swapl(&info->min.integral, n);
    swapl(&info->min.frac, n);
    swapl(&info->max.integral, n);
    swapl(&info->max.frac, n);
    swaps(&info->number, n);
    swaps(&info->sourceid, n);
}

int GetDeviceUse(DeviceIntPtr dev, uint16_t *attachment)
{
    DeviceIntPtr master = dev->u.master;
    int use;

    if (IsMaster(dev))
    {
        DeviceIntPtr paired = GetPairedDevice(dev);
        use = IsPointerDevice(dev) ? XIMasterPointer : XIMasterKeyboard;
        *attachment = (paired ? paired->id : 0);
    } else if (master)
    {
        use = IsPointerDevice(master) ? XISlavePointer : XISlaveKeyboard;
        *attachment = master->id;
    } else
        use = XIFloatingSlave;

    return use;
}

/**
 * Write the info for device dev into the buffer pointed to by info.
 *
 * @return The number of bytes used.
 */
static int
ListDeviceInfo(ClientPtr client, DeviceIntPtr dev, xXIDeviceInfo* info)
{
    char *any = (char*)&info[1];
    int len = 0, total_len = 0;

    info->deviceid = dev->id;
    info->use = GetDeviceUse(dev, &info->attachment);
    info->num_classes = 0;
    info->name_len = strlen(dev->name);
    info->enabled = dev->enabled;
    total_len = sizeof(xXIDeviceInfo);

    len = pad_to_int32(info->name_len);
    memset(any, 0, len);
    strncpy(any, dev->name, info->name_len);
    any += len;
    total_len += len;

    total_len += ListDeviceClasses(client, dev, any, &info->num_classes);
    return total_len;
}

/**
 * Write the class info of the device into the memory pointed to by any, set
 * nclasses to the number of classes in total and return the number of bytes
 * written.
 */
int
ListDeviceClasses(ClientPtr client, DeviceIntPtr dev,
		  char *any, uint16_t *nclasses)
{
    int total_len = 0;
    int len;
    int i;
    int rc;

    /* Check if the current device state should be suppressed */
    rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixReadAccess);

    if (dev->button)
    {
        (*nclasses)++;
        len = ListButtonInfo(dev, (xXIButtonInfo*)any, rc == Success);
        any += len;
        total_len += len;
    }

    if (dev->key)
    {
        (*nclasses)++;
        len = ListKeyInfo(dev, (xXIKeyInfo*)any);
        any += len;
        total_len += len;
    }

    for (i = 0; dev->valuator && i < dev->valuator->numAxes; i++)
    {
        (*nclasses)++;
        len = ListValuatorInfo(dev, (xXIValuatorInfo*)any, i, rc == Success);
        any += len;
        total_len += len;
    }

    return total_len;
}

static void
SwapDeviceInfo(DeviceIntPtr dev, xXIDeviceInfo* info)
{
    char n;
    char *any = (char*)&info[1];
    int i;

    /* Skip over name */
    any += pad_to_int32(info->name_len);

    for (i = 0; i < info->num_classes; i++)
    {
        int len = ((xXIAnyInfo*)any)->length;
        switch(((xXIAnyInfo*)any)->type)
        {
            case XIButtonClass:
                SwapButtonInfo(dev, (xXIButtonInfo*)any);
                break;
            case XIKeyClass:
                SwapKeyInfo(dev, (xXIKeyInfo*)any);
                break;
            case XIValuatorClass:
                SwapValuatorInfo(dev, (xXIValuatorInfo*)any);
                break;
        }

        any += len * 4;
    }

    swaps(&info->deviceid, n);
    swaps(&info->use, n);
    swaps(&info->attachment, n);
    swaps(&info->num_classes, n);
    swaps(&info->name_len, n);

}
