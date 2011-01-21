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
 * *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
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
#include <glib.h>

#include "inputstr.h"
#include "eventstr.h"
#include "eventconvert.h"
#include "exevents.h"
#include <X11/extensions/XI2proto.h>


static void test_values_XIRawEvent(RawDeviceEvent *in, xXIRawEvent *out,
                                   BOOL swap)
{
    int i;
    unsigned char *ptr;
    FP3232 *value, *raw_value;
    int nvals = 0;
    int bits_set;
    int len;

    if (swap)
    {
        char n;

        swaps(&out->sequenceNumber, n);
        swapl(&out->length, n);
        swaps(&out->evtype, n);
        swaps(&out->deviceid, n);
        swapl(&out->time, n);
        swapl(&out->detail, n);
        swaps(&out->valuators_len, n);
    }


    g_assert(out->type == GenericEvent);
    g_assert(out->extension == 0); /* IReqCode defaults to 0 */
    g_assert(out->evtype == GetXI2Type((InternalEvent*)in));
    g_assert(out->time == in->time);
    g_assert(out->detail == in->detail.button);
    g_assert(out->deviceid == in->deviceid);
    g_assert(out->valuators_len >= bytes_to_int32(bits_to_bytes(sizeof(in->valuators.mask))));
    g_assert(out->flags == 0); /* FIXME: we don't set the flags yet */

    ptr = (unsigned char*)&out[1];
    bits_set = 0;

    for (i = 0; out->valuators_len && i < sizeof(in->valuators.mask) * 8; i++)
    {
        g_assert (XIMaskIsSet(in->valuators.mask, i) == XIMaskIsSet(ptr, i));
        if (XIMaskIsSet(in->valuators.mask, i))
            bits_set++;
    }

    /* length is len of valuator mask (in 4-byte units) + the number of bits
     * set. Each bit set represents 2 8-byte values, hence the
     * 'bits_set * 4' */
    len = out->valuators_len + bits_set * 4;
    g_assert(out->length == len);

    nvals = 0;

    for (i = 0; out->valuators_len && i < MAX_VALUATORS; i++)
    {
        g_assert (XIMaskIsSet(in->valuators.mask, i) == XIMaskIsSet(ptr, i));
        if (XIMaskIsSet(in->valuators.mask, i))
        {
            FP3232 vi, vo;
            value = (FP3232*)(((unsigned char*)&out[1]) + out->valuators_len * 4);
            value += nvals;

            vi.integral = in->valuators.data[i];
            vi.frac = in->valuators.data_frac[i];

            vo.integral = value->integral;
            vo.frac = value->frac;
            if (swap)
            {
                char n;
                swapl(&vo.integral, n);
                swapl(&vo.frac, n);
            }

            g_assert(vi.integral == vo.integral);
            g_assert(vi.frac == vo.frac);

            raw_value = value + bits_set;

            vi.integral = in->valuators.data_raw[i];
            vi.frac = in->valuators.data_raw_frac[i];

            vo.integral = raw_value->integral;
            vo.frac = raw_value->frac;
            if (swap)
            {
                char n;
                swapl(&vo.integral, n);
                swapl(&vo.frac, n);
            }

            g_assert(vi.integral == vo.integral);
            g_assert(vi.frac == vo.frac);

            nvals++;
        }
    }
}

static void test_XIRawEvent(RawDeviceEvent *in)
{
    xXIRawEvent *out, *swapped;
    int rc;

    rc = EventToXI2((InternalEvent*)in, (xEvent**)&out);
    g_assert(rc == Success);

    test_values_XIRawEvent(in, out, FALSE);

    swapped = calloc(1, sizeof(xEvent) + out->length * 4);
    XI2EventSwap((xGenericEvent*)out, (xGenericEvent*)swapped);
    test_values_XIRawEvent(in, swapped, TRUE);

    free(out);
    free(swapped);
}

static void test_convert_XIFocusEvent(void)
{
    xEvent *out;
    DeviceEvent in;
    int rc;

    in.header = ET_Internal;
    in.type = ET_Enter;
    rc = EventToXI2((InternalEvent*)&in, &out);
    g_assert(rc == Success);
    g_assert(out == NULL);

    in.header = ET_Internal;
    in.type = ET_FocusIn;
    rc = EventToXI2((InternalEvent*)&in, &out);
    g_assert(rc == Success);
    g_assert(out == NULL);

    in.header = ET_Internal;
    in.type = ET_FocusOut;
    rc = EventToXI2((InternalEvent*)&in, &out);
    g_assert(rc == BadImplementation);

    in.header = ET_Internal;
    in.type = ET_Leave;
    rc = EventToXI2((InternalEvent*)&in, &out);
    g_assert(rc == BadImplementation);
}


static void test_convert_XIRawEvent(void)
{
    RawDeviceEvent in;
    int i;

    memset(&in, 0, sizeof(in));

    g_test_message("Testing all event types");
    in.header = ET_Internal;
    in.type = ET_RawMotion;
    test_XIRawEvent(&in);

    in.header = ET_Internal;
    in.type = ET_RawKeyPress;
    test_XIRawEvent(&in);

    in.header = ET_Internal;
    in.type = ET_RawKeyRelease;
    test_XIRawEvent(&in);

    in.header = ET_Internal;
    in.type = ET_RawButtonPress;
    test_XIRawEvent(&in);

    in.header = ET_Internal;
    in.type = ET_RawButtonRelease;
    test_XIRawEvent(&in);

    g_test_message("Testing details and other fields");
    in.detail.button = 1L;
    test_XIRawEvent(&in);
    in.detail.button = 1L << 8;
    test_XIRawEvent(&in);
    in.detail.button = 1L << 16;
    test_XIRawEvent(&in);
    in.detail.button = 1L << 24;
    test_XIRawEvent(&in);
    in.detail.button = ~0L;
    test_XIRawEvent(&in);

    in.detail.button = 0;

    in.time = 1L;
    test_XIRawEvent(&in);
    in.time = 1L << 8;
    test_XIRawEvent(&in);
    in.time = 1L << 16;
    test_XIRawEvent(&in);
    in.time = 1L << 24;
    test_XIRawEvent(&in);
    in.time = ~0L;
    test_XIRawEvent(&in);

    in.deviceid = 1;
    test_XIRawEvent(&in);
    in.deviceid = 1 << 8;
    test_XIRawEvent(&in);
    in.deviceid = ~0 & 0xFF;
    test_XIRawEvent(&in);

    g_test_message("Testing valuator masks");
    for (i = 0; i < sizeof(in.valuators.mask) * 8; i++)
    {
        XISetMask(in.valuators.mask, i);
        test_XIRawEvent(&in);
        XIClearMask(in.valuators.mask, i);
    }

    for (i = 0; i < MAX_VALUATORS; i++)
    {
        XISetMask(in.valuators.mask, i);

        in.valuators.data[i] = i;
        in.valuators.data_raw[i] = i + 10;
        in.valuators.data_frac[i] = i + 20;
        in.valuators.data_raw_frac[i] = i + 30;
        test_XIRawEvent(&in);
        XIClearMask(in.valuators.mask, i);
    }

    for (i = 0; i < sizeof(in.valuators.mask) * 8; i++)
    {
        XISetMask(in.valuators.mask, i);
        test_XIRawEvent(&in);
    }
}

static void test_values_XIDeviceEvent(DeviceEvent *in, xXIDeviceEvent *out,
                                      BOOL swap)
{
    int buttons, valuators;
    int i;
    unsigned char *ptr;
    uint32_t flagmask = 0;
    FP3232 *values;

    if (swap) {
        char n;

        swaps(&out->sequenceNumber, n);
        swapl(&out->length, n);
        swaps(&out->evtype, n);
        swaps(&out->deviceid, n);
        swaps(&out->sourceid, n);
        swapl(&out->time, n);
        swapl(&out->detail, n);
        swapl(&out->root, n);
        swapl(&out->event, n);
        swapl(&out->child, n);
        swapl(&out->root_x, n);
        swapl(&out->root_y, n);
        swapl(&out->event_x, n);
        swapl(&out->event_y, n);
        swaps(&out->buttons_len, n);
        swaps(&out->valuators_len, n);
        swapl(&out->mods.base_mods, n);
        swapl(&out->mods.latched_mods, n);
        swapl(&out->mods.locked_mods, n);
        swapl(&out->mods.effective_mods, n);
        swapl(&out->flags, n);
    }

    g_assert(out->extension == 0); /* IReqCode defaults to 0 */
    g_assert(out->evtype == GetXI2Type((InternalEvent*)in));
    g_assert(out->time == in->time);
    g_assert(out->detail == in->detail.button);
    g_assert(out->length >= 12);

    g_assert(out->deviceid == in->deviceid);
    g_assert(out->sourceid == in->sourceid);

    switch (in->type) {
        case ET_KeyPress:
            flagmask = XIKeyRepeat;
            break;
        default:
            flagmask = 0;
            break;
    }
    g_assert((out->flags & ~flagmask) == 0);

    g_assert(out->root == in->root);
    g_assert(out->event == None); /* set in FixUpEventFromWindow */
    g_assert(out->child == None); /* set in FixUpEventFromWindow */

    g_assert(out->mods.base_mods == in->mods.base);
    g_assert(out->mods.latched_mods == in->mods.latched);
    g_assert(out->mods.locked_mods == in->mods.locked);
    g_assert(out->mods.effective_mods == in->mods.effective);

    g_assert(out->group.base_group == in->group.base);
    g_assert(out->group.latched_group == in->group.latched);
    g_assert(out->group.locked_group == in->group.locked);
    g_assert(out->group.effective_group == in->group.effective);

    g_assert(out->event_x == 0); /* set in FixUpEventFromWindow */
    g_assert(out->event_y == 0); /* set in FixUpEventFromWindow */

    g_assert(out->root_x == FP1616(in->root_x, in->root_x_frac));
    g_assert(out->root_y == FP1616(in->root_y, in->root_y_frac));

    buttons = 0;
    for (i = 0; i < bits_to_bytes(sizeof(in->buttons)); i++)
    {
        if (XIMaskIsSet(in->buttons, i))
        {
            g_assert(out->buttons_len >= bytes_to_int32(bits_to_bytes(i)));
            buttons++;
        }
    }

    ptr = (unsigned char*)&out[1];
    for (i = 0; i < sizeof(in->buttons) * 8; i++)
        g_assert(XIMaskIsSet(in->buttons, i) == XIMaskIsSet(ptr, i));


    valuators = 0;
    for (i = 0; i < sizeof(in->valuators.mask) * 8; i++)
        if (XIMaskIsSet(in->valuators.mask, i))
            valuators++;

    g_assert(out->valuators_len >= bytes_to_int32(bits_to_bytes(valuators)));

    ptr += out->buttons_len * 4;
    values = (FP3232*)(ptr + out->valuators_len * 4);
    for (i = 0; i < sizeof(in->valuators.mask) * 8 ||
                i < (out->valuators_len * 4) * 8; i++)
    {
        if (i > sizeof(in->valuators.mask) * 8)
            g_assert(!XIMaskIsSet(ptr, i));
        else if (i > out->valuators_len * 4 * 8)
            g_assert(!XIMaskIsSet(in->valuators.mask, i));
        else {
            g_assert(XIMaskIsSet(in->valuators.mask, i) ==
                     XIMaskIsSet(ptr, i));

            if (XIMaskIsSet(ptr, i))
            {
                FP3232 vi, vo;

                vi.integral = in->valuators.data[i];
                vi.frac = in->valuators.data_frac[i];

                vo = *values;

                if (swap)
                {
                    char n;
                    swapl(&vo.integral, n);
                    swapl(&vo.frac, n);
                }


                g_assert(vi.integral == vo.integral);
                g_assert(vi.frac == vo.frac);
                values++;
            }
        }
    }
}

static void test_XIDeviceEvent(DeviceEvent *in)
{
    xXIDeviceEvent *out, *swapped;
    int rc;

    rc = EventToXI2((InternalEvent*)in, (xEvent**)&out);
    g_assert(rc == Success);

    test_values_XIDeviceEvent(in, out, FALSE);

    swapped = calloc(1, sizeof(xEvent) + out->length * 4);
    XI2EventSwap((xGenericEvent*)out, (xGenericEvent*)swapped);
    test_values_XIDeviceEvent(in, swapped, TRUE);

    free(out);
    free(swapped);
}

static void test_convert_XIDeviceEvent(void)
{
    DeviceEvent in;
    int i;

    memset(&in, 0, sizeof(in));

    g_test_message("Testing simple field values");
    in.header = ET_Internal;
    in.type = ET_Motion;
    in.length = sizeof(DeviceEvent);
    in.time             = 0;
    in.deviceid         = 1;
    in.sourceid         = 2;
    in.root             = 3;
    in.root_x           = 4;
    in.root_x_frac      = 5;
    in.root_y           = 6;
    in.root_y_frac      = 7;
    in.detail.button    = 8;
    in.mods.base        = 9;
    in.mods.latched     = 10;
    in.mods.locked      = 11;
    in.mods.effective   = 11;
    in.group.base       = 12;
    in.group.latched    = 13;
    in.group.locked     = 14;
    in.group.effective  = 15;

    test_XIDeviceEvent(&in);

    g_test_message("Testing field ranges");
    /* 32 bit */
    in.detail.button = 1L;
    test_XIDeviceEvent(&in);
    in.detail.button = 1L << 8;
    test_XIDeviceEvent(&in);
    in.detail.button = 1L << 16;
    test_XIDeviceEvent(&in);
    in.detail.button = 1L << 24;
    test_XIDeviceEvent(&in);
    in.detail.button = ~0L;
    test_XIDeviceEvent(&in);

    /* 32 bit */
    in.time = 1L;
    test_XIDeviceEvent(&in);
    in.time = 1L << 8;
    test_XIDeviceEvent(&in);
    in.time = 1L << 16;
    test_XIDeviceEvent(&in);
    in.time = 1L << 24;
    test_XIDeviceEvent(&in);
    in.time = ~0L;
    test_XIDeviceEvent(&in);

    /* 16 bit */
    in.deviceid = 1;
    test_XIDeviceEvent(&in);
    in.deviceid = 1 << 8;
    test_XIDeviceEvent(&in);
    in.deviceid = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    /* 16 bit */
    in.sourceid = 1;
    test_XIDeviceEvent(&in);
    in.deviceid = 1 << 8;
    test_XIDeviceEvent(&in);
    in.deviceid = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    /* 32 bit */
    in.root = 1L;
    test_XIDeviceEvent(&in);
    in.root = 1L << 8;
    test_XIDeviceEvent(&in);
    in.root = 1L << 16;
    test_XIDeviceEvent(&in);
    in.root = 1L << 24;
    test_XIDeviceEvent(&in);
    in.root = ~0L;
    test_XIDeviceEvent(&in);

    /* 16 bit */
    in.root_x = 1;
    test_XIDeviceEvent(&in);
    in.root_x = 1 << 8;
    test_XIDeviceEvent(&in);
    in.root_x = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    in.root_x_frac = 1;
    test_XIDeviceEvent(&in);
    in.root_x_frac = 1 << 8;
    test_XIDeviceEvent(&in);
    in.root_x_frac = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    in.root_y = 1;
    test_XIDeviceEvent(&in);
    in.root_y = 1 << 8;
    test_XIDeviceEvent(&in);
    in.root_y = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    in.root_y_frac = 1;
    test_XIDeviceEvent(&in);
    in.root_y_frac = 1 << 8;
    test_XIDeviceEvent(&in);
    in.root_y_frac = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    /* 32 bit */
    in.mods.base = 1L;
    test_XIDeviceEvent(&in);
    in.mods.base = 1L << 8;
    test_XIDeviceEvent(&in);
    in.mods.base = 1L << 16;
    test_XIDeviceEvent(&in);
    in.mods.base = 1L << 24;
    test_XIDeviceEvent(&in);
    in.mods.base = ~0L;
    test_XIDeviceEvent(&in);

    in.mods.latched = 1L;
    test_XIDeviceEvent(&in);
    in.mods.latched = 1L << 8;
    test_XIDeviceEvent(&in);
    in.mods.latched = 1L << 16;
    test_XIDeviceEvent(&in);
    in.mods.latched = 1L << 24;
    test_XIDeviceEvent(&in);
    in.mods.latched = ~0L;
    test_XIDeviceEvent(&in);

    in.mods.locked = 1L;
    test_XIDeviceEvent(&in);
    in.mods.locked = 1L << 8;
    test_XIDeviceEvent(&in);
    in.mods.locked = 1L << 16;
    test_XIDeviceEvent(&in);
    in.mods.locked = 1L << 24;
    test_XIDeviceEvent(&in);
    in.mods.locked = ~0L;
    test_XIDeviceEvent(&in);

    in.mods.effective = 1L;
    test_XIDeviceEvent(&in);
    in.mods.effective = 1L << 8;
    test_XIDeviceEvent(&in);
    in.mods.effective = 1L << 16;
    test_XIDeviceEvent(&in);
    in.mods.effective = 1L << 24;
    test_XIDeviceEvent(&in);
    in.mods.effective = ~0L;
    test_XIDeviceEvent(&in);

    /* 8 bit */
    in.group.base = 1;
    test_XIDeviceEvent(&in);
    in.group.base = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    in.group.latched = 1;
    test_XIDeviceEvent(&in);
    in.group.latched = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    in.group.locked = 1;
    test_XIDeviceEvent(&in);
    in.group.locked = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    in.mods.effective = 1;
    test_XIDeviceEvent(&in);
    in.mods.effective = ~0 & 0xFF;
    test_XIDeviceEvent(&in);

    g_test_message("Testing button masks");
    for (i = 0; i < sizeof(in.buttons) * 8; i++)
    {
        XISetMask(in.buttons, i);
        test_XIDeviceEvent(&in);
        XIClearMask(in.buttons, i);
    }

    for (i = 0; i < sizeof(in.buttons) * 8; i++)
    {
        XISetMask(in.buttons, i);
        test_XIDeviceEvent(&in);
    }

    g_test_message("Testing valuator masks");
    for (i = 0; i < sizeof(in.valuators.mask) * 8; i++)
    {
        XISetMask(in.valuators.mask, i);
        test_XIDeviceEvent(&in);
        XIClearMask(in.valuators.mask, i);
    }

    for (i = 0; i < sizeof(in.valuators.mask) * 8; i++)
    {
        XISetMask(in.valuators.mask, i);

        in.valuators.data[i] = i;
        in.valuators.data_frac[i] = i + 20;
        test_XIDeviceEvent(&in);
        XIClearMask(in.valuators.mask, i);
    }

    for (i = 0; i < sizeof(in.valuators.mask) * 8; i++)
    {
        XISetMask(in.valuators.mask, i);
        test_XIDeviceEvent(&in);
    }
}

static void test_values_XIDeviceChangedEvent(DeviceChangedEvent *in,
                                             xXIDeviceChangedEvent *out,
                                             BOOL swap)
{
    int i, j;
    unsigned char *ptr;

    if (swap)
    {
        char n;

        swaps(&out->sequenceNumber, n);
        swapl(&out->length, n);
        swaps(&out->evtype, n);
        swaps(&out->deviceid, n);
        swaps(&out->sourceid, n);
        swapl(&out->time, n);
        swaps(&out->num_classes, n);
    }

    g_assert(out->type == GenericEvent);
    g_assert(out->extension == 0); /* IReqCode defaults to 0 */
    g_assert(out->evtype == GetXI2Type((InternalEvent*)in));
    g_assert(out->time == in->time);
    g_assert(out->deviceid == in->deviceid);
    g_assert(out->sourceid == in->sourceid);

    ptr = (unsigned char*)&out[1];
    for (i = 0; i < out->num_classes; i++)
    {
        xXIAnyInfo* any = (xXIAnyInfo*)ptr;

        if (swap)
        {
            char n;
            swaps(&any->length, n);
            swaps(&any->type, n);
            swaps(&any->sourceid, n);
        }

        switch(any->type)
        {
            case XIButtonClass:
                {
                    xXIButtonInfo *b = (xXIButtonInfo*)any;
                    Atom *names;

                    if (swap)
                    {
                        char n;
                        swaps(&b->num_buttons, n);
                    }

                    g_assert(b->length ==
                            bytes_to_int32(sizeof(xXIButtonInfo)) +
                            bytes_to_int32(bits_to_bytes(b->num_buttons)) +
                            b->num_buttons);
                    g_assert(b->num_buttons == in->buttons.num_buttons);

                    names = (Atom*)((char*)&b[1] +
                            pad_to_int32(bits_to_bytes(b->num_buttons)));
                    for (j = 0; j < b->num_buttons; j++)
                    {
                        if (swap)
                        {
                            char n;
                            swapl(&names[j], n);
                        }
                        g_assert(names[j] == in->buttons.names[j]);
                    }
                }
                break;
            case XIKeyClass:
                {
                    xXIKeyInfo *k = (xXIKeyInfo*)any;
                    uint32_t *kc;

                    if (swap)
                    {
                        char n;
                        swaps(&k->num_keycodes, n);
                    }

                    g_assert(k->length ==
                            bytes_to_int32(sizeof(xXIKeyInfo)) +
                            k->num_keycodes);
                    g_assert(k->num_keycodes == in->keys.max_keycode -
                            in->keys.min_keycode + 1);

                    kc = (uint32_t*)&k[1];
                    for (j = 0; j < k->num_keycodes; j++)
                    {
                        if (swap)
                        {
                            char n;
                            swapl(&kc[j], n);
                        }
                        g_assert(kc[j] >= in->keys.min_keycode);
                        g_assert(kc[j] <= in->keys.max_keycode);
                    }
                }
                break;
            case XIValuatorClass:
                {
                    xXIValuatorInfo *v = (xXIValuatorInfo*)any;
                    g_assert(v->length ==
                             bytes_to_int32(sizeof(xXIValuatorInfo)));

                }
                break;
            default:
                g_error("Invalid class type.\n");
                break;
        }

        ptr += any->length * 4;
    }

}

static void test_XIDeviceChangedEvent(DeviceChangedEvent *in)
{
    xXIDeviceChangedEvent *out, *swapped;
    int rc;

    rc = EventToXI2((InternalEvent*)in, (xEvent**)&out);
    g_assert(rc == Success);

    test_values_XIDeviceChangedEvent(in, out, FALSE);

    swapped = calloc(1, sizeof(xEvent) + out->length * 4);
    XI2EventSwap((xGenericEvent*)out, (xGenericEvent*)swapped);
    test_values_XIDeviceChangedEvent(in, swapped, TRUE);

    free(out);
    free(swapped);
}

static void test_convert_XIDeviceChangedEvent(void)
{
    DeviceChangedEvent in;
    int i;

    g_test_message("Testing simple field values");
    memset(&in, 0, sizeof(in));
    in.header = ET_Internal;
    in.type = ET_DeviceChanged;
    in.length = sizeof(DeviceChangedEvent);
    in.time             = 0;
    in.deviceid         = 1;
    in.sourceid         = 2;
    in.masterid         = 3;
    in.num_valuators    = 4;
    in.flags = DEVCHANGE_SLAVE_SWITCH | DEVCHANGE_POINTER_EVENT | DEVCHANGE_KEYBOARD_EVENT;

    for (i = 0; i < MAX_BUTTONS; i++)
        in.buttons.names[i] = i + 10;

    in.keys.min_keycode = 8;
    in.keys.max_keycode = 255;

    test_XIDeviceChangedEvent(&in);

    in.time = 1L;
    test_XIDeviceChangedEvent(&in);
    in.time = 1L << 8;
    test_XIDeviceChangedEvent(&in);
    in.time = 1L << 16;
    test_XIDeviceChangedEvent(&in);
    in.time = 1L << 24;
    test_XIDeviceChangedEvent(&in);
    in.time = ~0L;
    test_XIDeviceChangedEvent(&in);

    in.deviceid = 1L;
    test_XIDeviceChangedEvent(&in);
    in.deviceid = 1L << 8;
    test_XIDeviceChangedEvent(&in);
    in.deviceid = ~0 & 0xFFFF;
    test_XIDeviceChangedEvent(&in);

    in.sourceid = 1L;
    test_XIDeviceChangedEvent(&in);
    in.sourceid = 1L << 8;
    test_XIDeviceChangedEvent(&in);
    in.sourceid = ~0 & 0xFFFF;
    test_XIDeviceChangedEvent(&in);

    in.masterid = 1L;
    test_XIDeviceChangedEvent(&in);
    in.masterid = 1L << 8;
    test_XIDeviceChangedEvent(&in);
    in.masterid = ~0 & 0xFFFF;
    test_XIDeviceChangedEvent(&in);

    in.buttons.num_buttons = 0;
    test_XIDeviceChangedEvent(&in);

    in.buttons.num_buttons = 1;
    test_XIDeviceChangedEvent(&in);

    in.buttons.num_buttons = MAX_BUTTONS;
    test_XIDeviceChangedEvent(&in);

    in.keys.min_keycode = 0;
    in.keys.max_keycode = 0;
    test_XIDeviceChangedEvent(&in);

    in.keys.max_keycode = 1 << 8;
    test_XIDeviceChangedEvent(&in);

    in.keys.max_keycode = 0xFFFC; /* highest range, above that the length
                                     field gives up */
    test_XIDeviceChangedEvent(&in);

    in.keys.min_keycode = 1 << 8;
    in.keys.max_keycode = 1 << 8;
    test_XIDeviceChangedEvent(&in);

    in.keys.min_keycode = 1 << 8;
    in.keys.max_keycode = 0;
    test_XIDeviceChangedEvent(&in);

    in.num_valuators = 0;
    test_XIDeviceChangedEvent(&in);

    in.num_valuators = 1;
    test_XIDeviceChangedEvent(&in);

    in.num_valuators = MAX_VALUATORS;
    test_XIDeviceChangedEvent(&in);

    for (i = 0; i < MAX_VALUATORS; i++)
    {
        in.valuators[i].min = 0;
        in.valuators[i].max = 0;
        test_XIDeviceChangedEvent(&in);

        in.valuators[i].max = 1 << 8;
        test_XIDeviceChangedEvent(&in);
        in.valuators[i].max = 1 << 16;
        test_XIDeviceChangedEvent(&in);
        in.valuators[i].max = 1 << 24;
        test_XIDeviceChangedEvent(&in);
        in.valuators[i].max = abs(~0);
        test_XIDeviceChangedEvent(&in);

        in.valuators[i].resolution = 1 << 8;
        test_XIDeviceChangedEvent(&in);
        in.valuators[i].resolution = 1 << 16;
        test_XIDeviceChangedEvent(&in);
        in.valuators[i].resolution = 1 << 24;
        test_XIDeviceChangedEvent(&in);
        in.valuators[i].resolution = abs(~0);
        test_XIDeviceChangedEvent(&in);

        in.valuators[i].name = i;
        test_XIDeviceChangedEvent(&in);

        in.valuators[i].mode = Relative;
        test_XIDeviceChangedEvent(&in);

        in.valuators[i].mode = Absolute;
        test_XIDeviceChangedEvent(&in);
    }
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    g_test_add_func("/xi2/eventconvert/XIRawEvent", test_convert_XIRawEvent);
    g_test_add_func("/xi2/eventconvert/XIFocusEvent", test_convert_XIFocusEvent);
    g_test_add_func("/xi2/eventconvert/XIDeviceEvent", test_convert_XIDeviceEvent);
    g_test_add_func("/xi2/eventconvert/XIDeviceChangedEvent", test_convert_XIDeviceChangedEvent);

    return g_test_run();
}
