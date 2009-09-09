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

#include <stdint.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/extensions/XI2proto.h>
#include <X11/Xatom.h>
#include "inputstr.h"
#include "extinit.h"
#include "scrnintstr.h"
#include "xkbsrv.h"

#include "xiquerydevice.h"

#include "protocol-common.h"
#include <glib.h>
/*
 * Protocol testing for XIQueryDevice request and reply.
 *
 * Test approach:
 * Wrap WriteToClient to intercept server's reply. ProcXIQueryDevice returns
 * data in two batches, once for the request, once for the trailing data
 * with the device information.
 * Repeatedly test with varying deviceids and check against data in reply.
 */

struct test_data {
    int which_device;
    int num_devices_in_reply;
};

static void reply_XIQueryDevice_data(ClientPtr client, int len, char *data, void *userdata);
static void reply_XIQueryDevice(ClientPtr client, int len, char* data, void *userdata);

/* reply handling for the first bytes that constitute the reply */
static void reply_XIQueryDevice(ClientPtr client, int len, char* data, void *userdata)
{
    xXIQueryDeviceReply *rep = (xXIQueryDeviceReply*)data;
    struct test_data *querydata = (struct test_data*)userdata;

    if (client->swapped)
    {
        char n;
        swapl(&rep->length, n);
        swaps(&rep->sequenceNumber, n);
        swaps(&rep->num_devices, n);
    }

    reply_check_defaults(rep, len, XIQueryDevice);

    if (querydata->which_device == XIAllDevices)
        g_assert(rep->num_devices == devices.num_devices);
    else if (querydata->which_device == XIAllMasterDevices)
        g_assert(rep->num_devices == devices.num_master_devices);
    else
        g_assert(rep->num_devices == 1);

    querydata->num_devices_in_reply = rep->num_devices;
    reply_handler = reply_XIQueryDevice_data;
}

/* reply handling for the trailing bytes that constitute the device info */
static void reply_XIQueryDevice_data(ClientPtr client, int len, char *data, void *userdata)
{
    char n;
    int i, j;
    struct test_data *querydata = (struct test_data*)userdata;

    DeviceIntPtr dev;
    xXIDeviceInfo *info = (xXIDeviceInfo*)data;
    xXIAnyInfo *any;

    for (i = 0; i < querydata->num_devices_in_reply; i++)
    {
        if (client->swapped)
        {
            swaps(&info->deviceid, n);
            swaps(&info->attachment, n);
            swaps(&info->use, n);
            swaps(&info->num_classes, n);
            swaps(&info->name_len, n);
        }

        if (querydata->which_device > XIAllMasterDevices)
            g_assert(info->deviceid == querydata->which_device);

        g_assert(info->deviceid >=  2); /* 0 and 1 is reserved */


        switch(info->deviceid)
        {
            case 2:  /* VCP */
                dev = devices.vcp;
                g_assert(info->use == XIMasterPointer);
                g_assert(info->attachment == devices.vck->id);
                g_assert(info->num_classes == 3); /* 2 axes + button */
                break;
            case 3:  /* VCK */
                dev = devices.vck;
                g_assert(info->use == XIMasterKeyboard);
                g_assert(info->attachment == devices.vcp->id);
                g_assert(info->num_classes == 1);
                break;
            case 4:  /* mouse */
                dev = devices.mouse;
                g_assert(info->use == XISlavePointer);
                g_assert(info->attachment == devices.vcp->id);
                g_assert(info->num_classes == 3); /* 2 axes + button */
                break;
            case 5:  /* keyboard */
                dev = devices.kbd;
                g_assert(info->use == XISlaveKeyboard);
                g_assert(info->attachment == devices.vck->id);
                g_assert(info->num_classes == 1);
                break;

            default:
                /* We shouldn't get here */
                g_assert(0);
                break;
        }
        g_assert(info->enabled == dev->enabled);
        g_assert(info->name_len == strlen(dev->name));
        g_assert(strncmp((char*)&info[1], dev->name, info->name_len) == 0);

        any = (xXIAnyInfo*)((char*)&info[1] + ((info->name_len + 3)/4) * 4);
        for (j = 0; j < info->num_classes; j++)
        {
            if (client->swapped)
            {
                swaps(&any->type, n);
                swaps(&any->length, n);
                swaps(&any->sourceid, n);
            }

            switch(info->deviceid)
            {
                case 3: /* VCK and kbd have the same properties */
                case 5:
                    {
                        int k;
                        xXIKeyInfo *ki = (xXIKeyInfo*)any;
                        XkbDescPtr xkb = devices.vck->key->xkbInfo->desc;
                        uint32_t *kc;

                        if (client->swapped)
                            swaps(&ki->num_keycodes, n);

                        g_assert(any->type == XIKeyClass);
                        g_assert(ki->num_keycodes == (xkb->max_key_code - xkb->min_key_code + 1));
                        g_assert(any->length == (2 + ki->num_keycodes));

                        kc = (uint32_t*)&ki[1];
                        for (k = 0; k < ki->num_keycodes; k++, kc++)
                        {
                            if (client->swapped)
                                swapl(kc, n);

                            g_assert(*kc >= xkb->min_key_code);
                            g_assert(*kc <= xkb->max_key_code);
                        }
                        break;
                    }
                case 2: /* VCP and mouse have the same properties */
                case 4:
                    {
                        g_assert(any->type == XIButtonClass ||
                                any->type == XIValuatorClass);

                        if (any->type == XIButtonClass)
                        {
                            int len;
                            xXIButtonInfo *bi = (xXIButtonInfo*)any;

                            if (client->swapped)
                                swaps(&bi->num_buttons, n);

                            g_assert(bi->num_buttons == devices.vcp->button->numButtons);

                            len = 2 + bi->num_buttons + bytes_to_int32(bits_to_bytes(bi->num_buttons));
                            g_assert(bi->length == len);
                        } else if (any->type == XIValuatorClass)
                        {
                            xXIValuatorInfo *vi = (xXIValuatorInfo*)any;

                            if (client->swapped)
                            {
                                swaps(&vi->number, n);
                                swapl(&vi->label, n);
                                swapl(&vi->min.integral, n);
                                swapl(&vi->min.frac, n);
                                swapl(&vi->max.integral, n);
                                swapl(&vi->max.frac, n);
                                swapl(&vi->resolution, n);
                            }

                            g_assert(vi->length == 11);
                            g_assert(vi->number == 0 ||
                                     vi->number == 1);
                            g_assert(vi->mode == XIModeRelative);
                            /* device was set up as relative, so standard
                             * values here. */
                            g_assert(vi->min.integral == -1);
                            g_assert(vi->min.frac == 0);
                            g_assert(vi->max.integral == -1);
                            g_assert(vi->max.frac == 0);
                            g_assert(vi->resolution == 0);
                        }
                    }
                    break;
            }
            any = (xXIAnyInfo*)(((char*)any) + any->length * 4);
        }

        info = (xXIDeviceInfo*)any;
    }
}

static void request_XIQueryDevice(struct test_data *querydata,
                                 int deviceid, int error)
{
    int rc;
    char n;
    ClientRec client;
    xXIQueryDeviceReq request;

    request_init(&request, XIQueryDevice);
    client = init_client(request.length, &request);
    reply_handler = reply_XIQueryDevice;

    querydata->which_device = deviceid;

    request.deviceid = deviceid;
    rc = ProcXIQueryDevice(&client);
    g_assert(rc == error);

    if (rc != Success)
        g_assert(client.errorValue == deviceid);

    reply_handler = reply_XIQueryDevice;

    client.swapped = TRUE;
    swaps(&request.length, n);
    swaps(&request.deviceid, n);
    rc = SProcXIQueryDevice(&client);
    g_assert(rc == error);

    if (rc != Success)
        g_assert(client.errorValue == deviceid);
}

static void test_XIQueryDevice(void)
{
    int i;
    xXIQueryDeviceReq request;
    struct test_data data;

    reply_handler = reply_XIQueryDevice;
    userdata = &data;
    request_init(&request, XIQueryDevice);

    g_test_message("Testing XIAllDevices.");
    request_XIQueryDevice(&data, XIAllDevices, Success);
    g_test_message("Testing XIAllMasterDevices.");
    request_XIQueryDevice(&data, XIAllMasterDevices, Success);

    g_test_message("Testing existing device ids.");
    for (i = 2; i < 6; i++)
        request_XIQueryDevice(&data, i, Success);

    g_test_message("Testing non-existing device ids.");
    for (i = 6; i <= 0xFFFF; i++)
        request_XIQueryDevice(&data, i, BadDevice);


    reply_handler = NULL;

}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    init_simple();

    g_test_add_func("/dix/xi2protocol/XIQueryDevice", test_XIQueryDevice);

    return g_test_run();
}

