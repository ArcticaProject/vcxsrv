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

#include "exglobals.h" /* BadDevice */
#include "exevents.h"
#include "xigrabdev.h"

int
SProcXIGrabDevice(ClientPtr client)
{
    char n;

    REQUEST(xXIGrabDeviceReq);

    swaps(&stuff->length, n);
    swaps(&stuff->deviceid, n);
    swapl(&stuff->grab_window, n);
    swapl(&stuff->cursor, n);
    swapl(&stuff->time, n);
    swaps(&stuff->mask_len, n);

    return ProcXIGrabDevice(client);
}

int
ProcXIGrabDevice(ClientPtr client)
{
    DeviceIntPtr dev;
    xXIGrabDeviceReply rep;
    int ret = Success;
    uint8_t status;
    GrabMask mask;
    int mask_len;

    REQUEST(xXIGrabDeviceReq);
    REQUEST_AT_LEAST_SIZE(xXIGrabDeviceReq);

    ret = dixLookupDevice(&dev, stuff->deviceid, client, DixGrabAccess);
    if (ret != Success)
	return ret;

    if (!IsMaster(dev))
        stuff->paired_device_mode = GrabModeAsync;

    if (XICheckInvalidMaskBits((unsigned char*)&stuff[1],
                               stuff->mask_len * 4) != Success)
        return BadValue;

    mask_len = min(sizeof(mask.xi2mask[stuff->deviceid]), stuff->mask_len * 4);
    memset(mask.xi2mask, 0, sizeof(mask.xi2mask));
    memcpy(mask.xi2mask, (char*)&stuff[1], mask_len);

    ret = GrabDevice(client, dev, stuff->grab_mode,
                     stuff->paired_device_mode,
                     stuff->grab_window,
                     stuff->owner_events,
                     stuff->time,
                     &mask,
                     GRABTYPE_XI2,
                     stuff->cursor,
                     None /* confineTo */,
                     &status);

    if (ret != Success)
        return ret;

    rep.repType = X_Reply;
    rep.RepType = X_XIGrabDevice;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.status = status;


    WriteReplyToClient(client, sizeof(rep), &rep);
    return ret;
}

int
SProcXIUngrabDevice(ClientPtr client)
{
    char n;

    REQUEST(xXIUngrabDeviceReq);

    swaps(&stuff->length, n);
    swaps(&stuff->deviceid, n);
    swapl(&stuff->time, n);

    return ProcXIUngrabDevice(client);
}

int
ProcXIUngrabDevice(ClientPtr client)
{
    DeviceIntPtr dev;
    GrabPtr grab;
    int ret = Success;
    TimeStamp time;

    REQUEST(xXIUngrabDeviceReq);

    ret = dixLookupDevice(&dev, stuff->deviceid, client, DixGetAttrAccess);
    if (ret != Success)
	return ret;

    grab = dev->deviceGrab.grab;

    time = ClientTimeToServerTime(stuff->time);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	(CompareTimeStamps(time, dev->deviceGrab.grabTime) != EARLIER) &&
	(grab) && SameClient(grab, client) && grab->grabtype == GRABTYPE_XI2)
	(*dev->deviceGrab.DeactivateGrab) (dev);

    return Success;
}

void SRepXIGrabDevice(ClientPtr client, int size, xXIGrabDeviceReply * rep)
{
    char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, (char *)rep);
}
