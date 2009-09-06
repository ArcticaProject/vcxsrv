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
 * Request to allow some device events.
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
#include "xiallowev.h"

int
SProcXIAllowEvents(ClientPtr client)
{
    char n;

    REQUEST(xXIAllowEventsReq);

    swaps(&stuff->length, n);
    swaps(&stuff->deviceid, n);
    swapl(&stuff->time, n);

    return ProcXIAllowEvents(client);
}

int
ProcXIAllowEvents(ClientPtr client)
{
    TimeStamp time;
    DeviceIntPtr dev;
    int ret = Success;

    REQUEST(xXIAllowEventsReq);
    REQUEST_SIZE_MATCH(xXIAllowEventsReq);

    ret = dixLookupDevice(&dev, stuff->deviceid, client, DixGetAttrAccess);
    if (ret != Success)
	return ret;

    time = ClientTimeToServerTime(stuff->time);

    switch (stuff->mode) {
    case XIReplayDevice:
	AllowSome(client, time, dev, NOT_GRABBED);
	break;
    case XISyncDevice:
	AllowSome(client, time, dev, FREEZE_NEXT_EVENT);
	break;
    case XIAsyncDevice:
	AllowSome(client, time, dev, THAWED);
	break;
    case XIAsyncPairedDevice:
        if (IsMaster(dev))
            AllowSome(client, time, dev, THAW_OTHERS);
	break;
    case XISyncPair:
        if (IsMaster(dev))
            AllowSome(client, time, dev, FREEZE_BOTH_NEXT_EVENT);
	break;
    case XIAsyncPair:
        if (IsMaster(dev))
            AllowSome(client, time, dev, THAWED_BOTH);
	break;
    default:
	client->errorValue = stuff->mode;
	ret = BadValue;
    }

    return ret;
}

