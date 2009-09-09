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
 * Protocol testing for XIQueryPointer request.
 */
#include <stdint.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/extensions/XI2proto.h>
#include "inputstr.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "xiquerypointer.h"
#include "exevents.h"

#include "protocol-common.h"
#include <glib.h>

static ClientRec client_request;
static void reply_XIQueryPointer_data(ClientPtr client, int len,
                                      char *data, void *userdata);

static struct {
    DeviceIntPtr dev;
    WindowPtr win;
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

static void reply_XIQueryPointer(ClientPtr client, int len, char *data,
                                 void *userdata)
{
    xXIQueryPointerReply *rep = (xXIQueryPointerReply*)data;
    SpritePtr sprite;

    if (!rep->repType)
        return;

    if (client->swapped)
    {
        char n;
        swapl(&rep->length, n);
        swaps(&rep->sequenceNumber, n);
        swapl(&rep->root, n);
        swapl(&rep->child, n);
        swapl(&rep->root_x, n);
        swapl(&rep->root_y, n);
        swapl(&rep->win_x, n);
        swapl(&rep->win_y, n);
        swaps(&rep->buttons_len, n);
    }

    reply_check_defaults(rep, len, XIQueryPointer);

    g_assert(rep->root == root.drawable.id);
    g_assert(rep->same_screen == xTrue);

    sprite = test_data.dev->spriteInfo->sprite;
    g_assert((rep->root_x >> 16) == sprite->hot.x);
    g_assert((rep->root_y >> 16) == sprite->hot.y);

    if (test_data.win == &root)
    {
        g_assert(rep->root_x == rep->win_x);
        g_assert(rep->root_y == rep->win_y);
        g_assert(rep->child == window.drawable.id);
    } else
    {
        int x, y;

        x = sprite->hot.x - window.drawable.x;
        y = sprite->hot.y - window.drawable.y;

        g_assert((rep->win_x >> 16) == x);
        g_assert((rep->win_y >> 16) == y);
        g_assert(rep->child == None);
    }


    g_assert(rep->same_screen == xTrue);

    reply_handler = reply_XIQueryPointer_data;
}

static void reply_XIQueryPointer_data(ClientPtr client, int len, char *data, void *userdata)
{
    reply_handler = reply_XIQueryPointer;
}

static void request_XIQueryPointer(ClientPtr client, xXIQueryPointerReq* req, int error)
{
    char n;
    int rc;

    rc = ProcXIQueryPointer(&client_request);
    g_assert(rc == error);

    if (rc == BadDevice)
        g_assert(client_request.errorValue == req->deviceid);

    client_request.swapped = TRUE;
    swaps(&req->deviceid, n);
    swaps(&req->length, n);
    rc = SProcXIQueryPointer(&client_request);
    g_assert(rc == error);

    if (rc == BadDevice)
        g_assert(client_request.errorValue == req->deviceid);
}

static void test_XIQueryPointer(void)
{
    int i;
    xXIQueryPointerReq request;

    memset(&request, 0, sizeof(request));

    request_init(&request, XIQueryPointer);

    reply_handler = reply_XIQueryPointer;

    client_request = init_client(request.length, &request);

    request.deviceid = XIAllDevices;
    request_XIQueryPointer(&client_request, &request, BadDevice);

    request.deviceid = XIAllMasterDevices;
    request_XIQueryPointer(&client_request, &request, BadDevice);

    request.win = root.drawable.id;
    test_data.win = &root;

    test_data.dev = devices.vcp;
    request.deviceid = devices.vcp->id;
    request_XIQueryPointer(&client_request, &request, Success);
    request.deviceid = devices.vck->id;
    request_XIQueryPointer(&client_request, &request, BadDevice);
    request.deviceid = devices.mouse->id;
    request_XIQueryPointer(&client_request, &request, BadDevice);
    request.deviceid = devices.kbd->id;
    request_XIQueryPointer(&client_request, &request, BadDevice);

    test_data.dev = devices.mouse;
    devices.mouse->u.master = NULL; /* Float, kind-of */
    request.deviceid = devices.mouse->id;
    request_XIQueryPointer(&client_request, &request, Success);

    for (i = devices.kbd->id + 1; i <= 0xFFFF; i++)
    {
        request.deviceid = i;
        request_XIQueryPointer(&client_request, &request, BadDevice);
    }

    request.win = window.drawable.id;

    test_data.dev = devices.vcp;
    test_data.win = &window;
    request.deviceid = devices.vcp->id;
    request_XIQueryPointer(&client_request, &request, Success);

    test_data.dev = devices.mouse;
    request.deviceid = devices.mouse->id;
    request_XIQueryPointer(&client_request, &request, Success);
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    init_simple();

    g_test_add_func("/xi2/protocol/XIQueryPointer", test_XIQueryPointer);

    return g_test_run();
}
