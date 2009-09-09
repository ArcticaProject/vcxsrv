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
 * Author: Peter Hutterer
 */

/***********************************************************************
 *
 * Request to grab or ungrab input device.
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "inputstr.h"	/* DeviceIntPtr      */
#include "windowstr.h"	/* window structure  */
#include <X11/extensions/XI2.h>
#include <X11/extensions/XI2proto.h>
#include "swaprep.h"

#include "exglobals.h" /* BadDevice */
#include "exevents.h"
#include "xipassivegrab.h"
#include "dixgrabs.h"

int
SProcXIPassiveGrabDevice(ClientPtr client)
{
    int i;
    char n;
    xXIModifierInfo *mods;

    REQUEST(xXIPassiveGrabDeviceReq);

    swaps(&stuff->length, n);
    swaps(&stuff->deviceid, n);
    swapl(&stuff->grab_window, n);
    swapl(&stuff->cursor, n);
    swapl(&stuff->time, n);
    swapl(&stuff->detail, n);
    swaps(&stuff->mask_len, n);
    swaps(&stuff->num_modifiers, n);

    mods = (xXIModifierInfo*)&stuff[1];

    for (i = 0; i < stuff->num_modifiers; i++, mods++)
    {
        swapl(&mods->base_mods, n);
        swapl(&mods->latched_mods, n);
        swapl(&mods->locked_mods, n);
    }

    return ProcXIPassiveGrabDevice(client);
}

int
ProcXIPassiveGrabDevice(ClientPtr client)
{
    DeviceIntPtr dev, mod_dev;
    xXIPassiveGrabDeviceReply rep;
    int i, ret = Success;
    uint8_t status;
    uint32_t *modifiers;
    xXIGrabModifierInfo *modifiers_failed;
    GrabMask mask;
    GrabParameters param;
    void *tmp;
    int mask_len;

    REQUEST(xXIPassiveGrabDeviceReq);
    REQUEST_AT_LEAST_SIZE(xXIPassiveGrabDeviceReq);

    if (stuff->deviceid == XIAllDevices)
        dev = inputInfo.all_devices;
    else if (stuff->deviceid == XIAllMasterDevices)
        dev = inputInfo.all_master_devices;
    else
    {
        ret = dixLookupDevice(&dev, stuff->deviceid, client, DixGrabAccess);
        if (ret != Success)
            return ret;
    }

    if (stuff->grab_type != XIGrabtypeButton &&
        stuff->grab_type != XIGrabtypeKeycode &&
        stuff->grab_type != XIGrabtypeEnter &&
        stuff->grab_type != XIGrabtypeFocusIn)
    {
        client->errorValue = stuff->grab_type;
        return BadValue;
    }

    if ((stuff->grab_type == XIGrabtypeEnter ||
         stuff->grab_type == XIGrabtypeFocusIn) && stuff->detail != 0)
    {
        client->errorValue = stuff->detail;
        return BadValue;
    }

    if (XICheckInvalidMaskBits((unsigned char*)&stuff[1],
                               stuff->mask_len * 4) != Success)
        return BadValue;

    mask_len = min(sizeof(mask.xi2mask[stuff->deviceid]), stuff->mask_len * 4);
    memset(mask.xi2mask, 0, sizeof(mask.xi2mask));
    memcpy(mask.xi2mask[stuff->deviceid], &stuff[1], mask_len * 4);

    rep.repType = X_Reply;
    rep.RepType = X_XIPassiveGrabDevice;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.num_modifiers = 0;

    memset(&param, 0, sizeof(param));
    param.grabtype = GRABTYPE_XI2;
    param.ownerEvents = stuff->owner_events;
    param.this_device_mode = stuff->grab_mode;
    param.other_devices_mode = stuff->paired_device_mode;
    param.grabWindow = stuff->grab_window;
    param.cursor = stuff->cursor;

    if (stuff->cursor != None)
    {
        status = dixLookupResourceByType(&tmp, stuff->cursor,
                                         RT_CURSOR, client, DixUseAccess);
	if (status != Success)
	{
	    client->errorValue = stuff->cursor;
	    return (status == BadValue) ? BadCursor : status;
	}
    }

    status = dixLookupWindow((WindowPtr*)&tmp, stuff->grab_window, client, DixSetAttrAccess);
    if (status != Success)
	return status;

    status = CheckGrabValues(client, &param);

    modifiers = (uint32_t*)&stuff[1] + stuff->mask_len;
    modifiers_failed = xcalloc(stuff->num_modifiers, sizeof(xXIGrabModifierInfo));
    if (!modifiers_failed)
        return BadAlloc;

    if (!IsMaster(dev) && dev->u.master)
        mod_dev = GetMaster(dev, MASTER_KEYBOARD);
    else
        mod_dev = dev;

    for (i = 0; i < stuff->num_modifiers; i++, modifiers++)
    {
        param.modifiers = *modifiers;
        switch(stuff->grab_type)
        {
            case XIGrabtypeButton:
                status = GrabButton(client, dev, mod_dev, stuff->detail,
                                    &param, GRABTYPE_XI2, &mask);
                break;
            case XIGrabtypeKeycode:
                status = GrabKey(client, dev, mod_dev, stuff->detail,
                                 &param, GRABTYPE_XI2, &mask);
                break;
            case XIGrabtypeEnter:
            case XIGrabtypeFocusIn:
                status = GrabWindow(client, dev, stuff->grab_type,
                                    &param, &mask);
                break;
        }

        if (status != GrabSuccess)
        {
            xXIGrabModifierInfo *info = modifiers_failed + rep.num_modifiers;

            info->status = status;
            info->modifiers = *modifiers;
            rep.num_modifiers++;
            rep.length++;
        }
    }

    WriteReplyToClient(client, sizeof(rep), &rep);
    if (rep.num_modifiers)
    {
	client->pSwapReplyFunc = (ReplySwapPtr) Swap32Write;
        WriteSwappedDataToClient(client, rep.num_modifiers * 4, (char*)modifiers_failed);
    }
    xfree(modifiers_failed);
    return ret;
}

void
SRepXIPassiveGrabDevice(ClientPtr client, int size,
                        xXIPassiveGrabDeviceReply * rep)
{
    char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    swaps(&rep->num_modifiers, n);

    WriteToClient(client, size, (char *)rep);
}

int
SProcXIPassiveUngrabDevice(ClientPtr client)
{
    char n;
    int i;
    uint32_t *modifiers;

    REQUEST(xXIPassiveUngrabDeviceReq);

    swaps(&stuff->length, n);
    swapl(&stuff->grab_window, n);
    swaps(&stuff->deviceid, n);
    swapl(&stuff->detail, n);
    swaps(&stuff->num_modifiers, n);

    modifiers = (uint32_t*)&stuff[1];

    for (i = 0; i < stuff->num_modifiers; i++, modifiers++)
        swapl(modifiers, n);

    return ProcXIPassiveUngrabDevice(client);
}

int
ProcXIPassiveUngrabDevice(ClientPtr client)
{
    DeviceIntPtr dev, mod_dev;
    WindowPtr win;
    GrabRec tempGrab;
    uint32_t* modifiers;
    int i, rc;

    REQUEST(xXIPassiveUngrabDeviceReq);
    REQUEST_AT_LEAST_SIZE(xXIPassiveUngrabDeviceReq);

    rc = dixLookupDevice(&dev, stuff->deviceid, client, DixGrabAccess);
    if (rc != Success)
	return rc;

    if (stuff->grab_type != XIGrabtypeButton &&
        stuff->grab_type != XIGrabtypeKeycode &&
        stuff->grab_type != XIGrabtypeEnter &&
        stuff->grab_type != XIGrabtypeFocusIn)
    {
        client->errorValue = stuff->grab_type;
        return BadValue;
    }

    if ((stuff->grab_type == XIGrabtypeEnter ||
         stuff->grab_type == XIGrabtypeFocusIn) && stuff->detail != 0)
    {
        client->errorValue = stuff->detail;
        return BadValue;
    }

    rc = dixLookupWindow(&win, stuff->grab_window, client, DixSetAttrAccess);
    if (rc != Success)
        return rc;

    if (!IsMaster(dev) && dev->u.master)
        mod_dev = GetMaster(dev, MASTER_KEYBOARD);
    else
        mod_dev = dev;

    tempGrab.resource = client->clientAsMask;
    tempGrab.device = dev;
    tempGrab.window = win;
    switch(stuff->grab_type)
    {
        case XIGrabtypeButton:  tempGrab.type = XI_ButtonPress; break;
        case XIGrabtypeKeycode:  tempGrab.type = XI_KeyPress;    break;
        case XIGrabtypeEnter:   tempGrab.type = XI_Enter;       break;
        case XIGrabtypeFocusIn: tempGrab.type = XI_FocusIn;     break;
    }
    tempGrab.grabtype = GRABTYPE_XI2;
    tempGrab.modifierDevice = mod_dev;
    tempGrab.modifiersDetail.pMask = NULL;
    tempGrab.detail.exact = stuff->detail;
    tempGrab.detail.pMask = NULL;

    modifiers = (uint32_t*)&stuff[1];

    for (i = 0; i < stuff->num_modifiers; i++, modifiers++)
    {
        tempGrab.modifiersDetail.exact = *modifiers;
        DeletePassiveGrabFromList(&tempGrab);
    }

    return Success;
}
