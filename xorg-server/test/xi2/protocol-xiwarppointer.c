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
 * Protocol testing for XIWarpPointer request.
 */
#include <stdint.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/extensions/XI2proto.h>
#include "inputstr.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "xiwarppointer.h"
#include "exevents.h"

#include "protocol-common.h"
#include <glib.h>

static int expected_x = SPRITE_X;
static int expected_y = SPRITE_Y;

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

/**
 * This function overrides the one in the screen rec.
 */
static Bool ScreenSetCursorPosition(DeviceIntPtr dev, ScreenPtr screen,
                                    int x, int y, Bool generateEvent)
{
    g_assert(x == expected_x);
    g_assert(y == expected_y);
    return TRUE;
}


static void request_XIWarpPointer(ClientPtr client, xXIWarpPointerReq* req,
        int error)
{
    char n;
    int rc;

    rc = ProcXIWarpPointer(client);
    g_assert(rc == error);

    if (rc == BadDevice)
        g_assert(client->errorValue == req->deviceid);
    else if (rc == BadWindow)
        g_assert(client->errorValue == req->dst_win ||
                 client->errorValue == req->src_win);


    client->swapped = TRUE;

    swapl(&req->src_win, n);
    swapl(&req->dst_win, n);
    swapl(&req->src_x, n);
    swapl(&req->src_y, n);
    swapl(&req->dst_x, n);
    swapl(&req->dst_y, n);
    swaps(&req->src_width, n);
    swaps(&req->src_height, n);
    swaps(&req->deviceid, n);

    rc = SProcXIWarpPointer(client);
    g_assert(rc == error);

    if (rc == BadDevice)
        g_assert(client->errorValue == req->deviceid);
    else if (rc == BadWindow)
        g_assert(client->errorValue == req->dst_win ||
                 client->errorValue == req->src_win);

    client->swapped = FALSE;
}

static void test_XIWarpPointer(void)
{
    int i;
    ClientRec client_request;
    xXIWarpPointerReq request;

    memset(&request, 0, sizeof(request));

    request_init(&request, XIWarpPointer);

    client_request = init_client(request.length, &request);

    request.deviceid = XIAllDevices;
    request_XIWarpPointer(&client_request, &request, BadDevice);

    request.deviceid = XIAllMasterDevices;
    request_XIWarpPointer(&client_request, &request, BadDevice);

    request.src_win = root.drawable.id;
    request.dst_win = root.drawable.id;
    request.deviceid = devices.vcp->id;
    request_XIWarpPointer(&client_request, &request, Success);
    request.deviceid = devices.vck->id;
    request_XIWarpPointer(&client_request, &request, BadDevice);
    request.deviceid = devices.mouse->id;
    request_XIWarpPointer(&client_request, &request, BadDevice);
    request.deviceid = devices.kbd->id;
    request_XIWarpPointer(&client_request, &request, BadDevice);

    devices.mouse->u.master = NULL; /* Float, kind-of */
    request.deviceid = devices.mouse->id;
    request_XIWarpPointer(&client_request, &request, Success);

    for (i = devices.kbd->id + 1; i <= 0xFFFF; i++)
    {
        request.deviceid = i;
        request_XIWarpPointer(&client_request, &request, BadDevice);
    }

    request.src_win = window.drawable.id;
    request.deviceid = devices.vcp->id;
    request_XIWarpPointer(&client_request, &request, Success);

    request.deviceid = devices.mouse->id;
    request_XIWarpPointer(&client_request, &request, Success);

    request.src_win = root.drawable.id;
    request.dst_win = 0xFFFF; /* invalid window */
    request_XIWarpPointer(&client_request, &request, BadWindow);

    request.src_win = 0xFFFF; /* invalid window */
    request.dst_win = root.drawable.id;
    request_XIWarpPointer(&client_request, &request, BadWindow);

    request.src_win = None;
    request.dst_win = None;

    request.dst_y = 0;
    expected_y = SPRITE_Y;

    request.dst_x = 1 << 16;
    expected_x = SPRITE_X + 1;
    request.deviceid = devices.vcp->id;
    request_XIWarpPointer(&client_request, &request, Success);

    request.dst_x = -1 << 16;
    expected_x = SPRITE_X - 1;
    request.deviceid = devices.vcp->id;
    request_XIWarpPointer(&client_request, &request, Success);

    request.dst_x = 0;
    expected_x = SPRITE_X;

    request.dst_y = 1 << 16;
    expected_y = SPRITE_Y + 1;
    request.deviceid = devices.vcp->id;
    request_XIWarpPointer(&client_request, &request, Success);

    request.dst_y = -1 << 16;
    expected_y = SPRITE_Y - 1;
    request.deviceid = devices.vcp->id;
    request_XIWarpPointer(&client_request, &request, Success);

    /* FIXME: src_x/y checks */
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    init_simple();
    screen.SetCursorPosition = ScreenSetCursorPosition;

    g_test_add_func("/xi2/protocol/XIWarpPointer", test_XIWarpPointer);

    return g_test_run();
}
