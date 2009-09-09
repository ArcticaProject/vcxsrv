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
 * Protocol testing for XIGetClientPointer request.
 */
#include <stdint.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/extensions/XI2proto.h>
#include "inputstr.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "xigetclientpointer.h"
#include "exevents.h"

#include "protocol-common.h"
#include <glib.h>

struct {
    int cp_is_set;
    DeviceIntPtr dev;
    int win;
} test_data;

static ClientRec client_window;
static ClientRec client_request;

int __wrap_dixLookupClient(ClientPtr *pClient, XID rid, ClientPtr client, Mask access)
{
    if (rid == ROOT_WINDOW_ID)
        return BadWindow;

    if (rid == CLIENT_WINDOW_ID)
    {
        *pClient = &client_window;
        return Success;
    }

    return __real_dixLookupClient(pClient, rid, client, access);
}


static void reply_XIGetClientPointer(ClientPtr client, int len, char *data, void *userdata)
{
    xXIGetClientPointerReply *rep = (xXIGetClientPointerReply*)data;

    if (client->swapped)
    {
        char n;
        swapl(&rep->length, n);
        swaps(&rep->sequenceNumber, n);
        swaps(&rep->deviceid, n);
    }

    reply_check_defaults(rep, len, XIGetClientPointer);

    g_assert(rep->set == test_data.cp_is_set);
    if (rep->set)
        g_assert(rep->deviceid == test_data.dev->id);
}

static void request_XIGetClientPointer(ClientPtr client, xXIGetClientPointerReq* req, int error)
{
    char n;
    int rc;

    test_data.win = req->win;

    rc = ProcXIGetClientPointer(&client_request);
    g_assert(rc == error);

    if (rc == BadWindow)
        g_assert(client_request.errorValue == req->win);

    client_request.swapped = TRUE;
    swapl(&req->win, n);
    swaps(&req->length, n);
    rc = SProcXIGetClientPointer(&client_request);
    g_assert(rc == error);

    if (rc == BadWindow)
        g_assert(client_request.errorValue == req->win);

}

static void test_XIGetClientPointer(void)
{
    xXIGetClientPointerReq request;

    request_init(&request, XIGetClientPointer);

    request.win = CLIENT_WINDOW_ID;


    reply_handler = reply_XIGetClientPointer;

    client_request = init_client(request.length, &request);

    g_test_message("Testing invalid window");
    request.win = INVALID_WINDOW_ID;
    request_XIGetClientPointer(&client_request, &request, BadWindow);

    test_data.cp_is_set = FALSE;

    g_test_message("Testing window None, unset ClientPointer.");
    request.win = None;
    request_XIGetClientPointer(&client_request, &request, Success);

    g_test_message("Testing valid window, unset ClientPointer.");
    request.win = CLIENT_WINDOW_ID;
    request_XIGetClientPointer(&client_request, &request, Success);

    g_test_message("Testing valid window, set ClientPointer.");
    client_window.clientPtr = devices.vcp;
    test_data.dev = devices.vcp;
    test_data.cp_is_set = TRUE;
    request.win = CLIENT_WINDOW_ID;
    request_XIGetClientPointer(&client_request, &request, Success);

    client_window.clientPtr = NULL;

    g_test_message("Testing window None, set ClientPointer.");
    client_request.clientPtr = devices.vcp;
    test_data.dev = devices.vcp;
    test_data.cp_is_set = TRUE;
    request.win = None;
    request_XIGetClientPointer(&client_request, &request, Success);
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    init_simple();
    client_window = init_client(0, NULL);


    g_test_add_func("/xi2/protocol/XIGetClientPointer", test_XIGetClientPointer);

    return g_test_run();
}
