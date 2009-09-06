/**
 * Copyright © 2009 Red Hat, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

/*
 * Protocol testing for XIGetSelectedEvents request.
 *
 * Tests include:
 * BadWindow on wrong window.
 * Zero-length masks if no masks are set.
 * Valid masks for valid devices.
 * Masks set on non-existent devices are not returned.
 *
 * Note that this test is not connected to the XISelectEvents request.
 */
#include <stdint.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/extensions/XI2proto.h>
#include "inputstr.h"
#include "windowstr.h"
#include "extinit.h" /* for XInputExtensionInit */
#include "scrnintstr.h"
#include "xiselectev.h"
#include "exevents.h"

#include "protocol-common.h"
#include <glib.h>

static void reply_XIGetSelectedEvents(ClientPtr client, int len, char *data, void *userdata);
static void reply_XIGetSelectedEvents_data(ClientPtr client, int len, char *data, void *userdata);


struct {
    int num_masks_expected;
    unsigned char mask[MAXDEVICES][XI2LASTEVENT]; /* intentionally bigger */
    int mask_len;
} test_data;

/* dixLookupWindow requires a lot of setup not necessary for this test.
 * Simple wrapper that returns either one of the fake root window or the
 * fake client window. If the requested ID is neither of those wanted,
 * return whatever the real dixLookupWindow does.
 */
int __wrap_dixLookupWindow(WindowPtr *win, XID id, ClientPtr client, Mask access)
{
    if (id == root.drawable.id)
    {
        *win = &root;
        return Success;
    } else if (id == window.drawable.id)
    {
        *win = &window;
        return Success;
    }

    return __real_dixLookupWindow(win, id, client, access);
}

/* AddResource is called from XISetSEventMask, we don't need this */
Bool __wrap_AddResource(XID id, RESTYPE type, pointer value)
{
    return TRUE;
}

static void reply_XIGetSelectedEvents(ClientPtr client, int len, char *data, void *userdata)
{
    xXIGetSelectedEventsReply *rep = (xXIGetSelectedEventsReply*)data;

    if (client->swapped)
    {
        char n;
        swapl(&rep->length, n);
        swaps(&rep->sequenceNumber, n);
        swaps(&rep->num_masks, n);
    }

    reply_check_defaults(rep, len, XIGetSelectedEvents);

    g_assert(rep->num_masks == test_data.num_masks_expected);

    reply_handler = reply_XIGetSelectedEvents_data;
}

static void reply_XIGetSelectedEvents_data(ClientPtr client, int len, char *data, void *userdata)
{
    int i;
    xXIEventMask *mask;
    unsigned char *bitmask;

    mask = (xXIEventMask*)data;
    for (i = 0; i < test_data.num_masks_expected; i++)
    {
        if (client->swapped)
        {
            char n;
            swaps(&mask->deviceid, n);
            swaps(&mask->mask_len, n);
        }

        g_assert(mask->deviceid < 6);
        g_assert(mask->mask_len <= (((XI2LASTEVENT + 8)/8) + 3)/4) ;

        bitmask = (unsigned char*)&mask[1];
        g_assert(memcmp(bitmask,
                    test_data.mask[mask->deviceid],
                    mask->mask_len * 4) == 0);

        mask = (xXIEventMask*)((char*)mask + mask->mask_len * 4 + sizeof(xXIEventMask));
    }


}

static void request_XIGetSelectedEvents(xXIGetSelectedEventsReq* req, int error)
{
    char n;
    int rc;
    ClientRec client;
    client = init_client(req->length, req);

    reply_handler = reply_XIGetSelectedEvents;

    rc = ProcXIGetSelectedEvents(&client);
    g_assert(rc == error);

    reply_handler = reply_XIGetSelectedEvents;
    client.swapped = TRUE;
    swapl(&req->win, n);
    swaps(&req->length, n);
    rc = SProcXIGetSelectedEvents(&client);
    g_assert(rc == error);
}

static void test_XIGetSelectedEvents(void)
{
    int i, j;
    xXIGetSelectedEventsReq request;
    ClientRec client = init_client(0, NULL);
    unsigned char *mask;
    DeviceIntRec dev;

    request_init(&request, XIGetSelectedEvents);

    g_test_message("Testing for BadWindow on invalid window.");
    request.win = None;
    request_XIGetSelectedEvents(&request, BadWindow);

    g_test_message("Testing for zero-length (unset) masks.");
    /* No masks set yet */
    test_data.num_masks_expected = 0;
    request.win = ROOT_WINDOW_ID;
    request_XIGetSelectedEvents(&request, Success);

    request.win = CLIENT_WINDOW_ID;
    request_XIGetSelectedEvents(&request, Success);

    memset(test_data.mask, 0,
           sizeof(test_data.mask));

    g_test_message("Testing for valid masks");
    memset(&dev, 0, sizeof(dev)); /* dev->id is enough for XISetEventMask */
    request.win = ROOT_WINDOW_ID;

    /* devices 6 - MAXDEVICES don't exist, they mustn't be included in the
     * reply even if a mask is set */
    for (j = 0; j < MAXDEVICES; j++)
    {
        test_data.num_masks_expected = min(j + 1, devices.num_devices + 2);
        dev.id = j;
        mask = test_data.mask[j];
        /* bits one-by-one */
        for (i = 0; i < XI2LASTEVENT; i++)
        {
            SetBit(mask, i);
            XISetEventMask(&dev, &root, &client, (i + 8)/8, mask);
            request_XIGetSelectedEvents(&request, Success);
            ClearBit(mask, i);
        }

        /* all valid mask bits */
        for (i = 0; i < XI2LASTEVENT; i++)
        {
            SetBit(mask, i);
            XISetEventMask(&dev, &root, &client, (i + 8)/8, mask);
            request_XIGetSelectedEvents(&request, Success);
        }
    }

    g_test_message("Testing removing all masks");
    /* Unset all masks one-by-one */
    for (j = MAXDEVICES - 1; j >= 0; j--)
    {
        if (j < devices.num_devices + 2)
            test_data.num_masks_expected--;

        mask = test_data.mask[j];
        memset(mask, 0, XI2LASTEVENT);

        dev.id = j;
        XISetEventMask(&dev, &root, &client, 0, NULL);

        request_XIGetSelectedEvents(&request, Success);
    }
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    init_simple();

    g_test_add_func("/xi2/protocol/XIGetSelectedEvents", test_XIGetSelectedEvents);

    return g_test_run();
}

