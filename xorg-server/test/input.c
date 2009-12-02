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
#include "misc.h"
#include "resource.h"
#include <X11/Xproto.h>
#include <X11/extensions/XI2proto.h>
#include <X11/Xatom.h>
#include "windowstr.h"
#include "inputstr.h"
#include "eventconvert.h"
#include "exevents.h"
#include "dixgrabs.h"
#include "eventstr.h"
#include <glib.h>

/**
 * Init a device with axes.
 * Verify values set on the device.
 *
 * Result: All axes set to default values (usually 0).
 */
static void dix_init_valuators(void)
{
    DeviceIntRec dev;
    ValuatorClassPtr val;
    const int num_axes = 2;
    int i;
    Atom atoms[MAX_VALUATORS] = { 0 };


    memset(&dev, 0, sizeof(DeviceIntRec));
    dev.type = MASTER_POINTER; /* claim it's a master to stop ptracccel */

    g_assert(InitValuatorClassDeviceStruct(NULL, 0, atoms, 0, 0) == FALSE);
    g_assert(InitValuatorClassDeviceStruct(&dev, num_axes, atoms, 0, Absolute));

    val = dev.valuator;
    g_assert(val);
    g_assert(val->numAxes == num_axes);
    g_assert(val->numMotionEvents == 0);
    g_assert(val->mode == Absolute);
    g_assert(val->axisVal);

    for (i = 0; i < num_axes; i++)
    {
        g_assert(val->axisVal[i] == 0);
        g_assert(val->axes->min_value == NO_AXIS_LIMITS);
        g_assert(val->axes->max_value == NO_AXIS_LIMITS);
    }

    g_assert(dev.last.numValuators == num_axes);
}

/* just check the known success cases, and that error cases set the client's
 * error value correctly. */
static void dix_check_grab_values(void)
{
    ClientRec client;
    GrabParameters param;
    int rc;

    memset(&client, 0, sizeof(client));

    param.grabtype = GRABTYPE_CORE;
    param.this_device_mode = GrabModeSync;
    param.other_devices_mode = GrabModeSync;
    param.modifiers = AnyModifier;
    param.ownerEvents = FALSE;

    rc = CheckGrabValues(&client, &param);
    g_assert(rc == Success);

    param.this_device_mode = GrabModeAsync;
    rc = CheckGrabValues(&client, &param);
    g_assert(rc == Success);

    param.this_device_mode = GrabModeAsync + 1;
    rc = CheckGrabValues(&client, &param);
    g_assert(rc == BadValue);
    g_assert(client.errorValue == param.this_device_mode);
    g_assert(client.errorValue == GrabModeAsync + 1);

    param.this_device_mode = GrabModeSync;
    param.other_devices_mode = GrabModeAsync;
    rc = CheckGrabValues(&client, &param);
    g_assert(rc == Success);

    param.other_devices_mode = GrabModeAsync + 1;
    rc = CheckGrabValues(&client, &param);
    g_assert(rc == BadValue);
    g_assert(client.errorValue == param.other_devices_mode);
    g_assert(client.errorValue == GrabModeAsync + 1);

    param.other_devices_mode = GrabModeSync;

    param.modifiers = 1 << 13;
    rc = CheckGrabValues(&client, &param);
    g_assert(rc == BadValue);
    g_assert(client.errorValue == param.modifiers);
    g_assert(client.errorValue == (1 << 13));


    param.modifiers = AnyModifier;
    param.ownerEvents = TRUE;
    rc = CheckGrabValues(&client, &param);
    g_assert(rc == Success);

    param.ownerEvents = 3;
    rc = CheckGrabValues(&client, &param);
    g_assert(rc == BadValue);
    g_assert(client.errorValue == param.ownerEvents);
    g_assert(client.errorValue == 3);
}


/**
 * Convert various internal events to the matching core event and verify the
 * parameters.
 */
static void dix_event_to_core(int type)
{
    DeviceEvent ev;
    xEvent core;
    int time;
    int x, y;
    int rc;
    int state;
    int detail;

    /* EventToCore memsets the event to 0 */
#define test_event() \
    g_assert(rc == Success); \
    g_assert(core.u.u.type == type); \
    g_assert(core.u.u.detail == detail); \
    g_assert(core.u.keyButtonPointer.time == time); \
    g_assert(core.u.keyButtonPointer.rootX == x); \
    g_assert(core.u.keyButtonPointer.rootY == y); \
    g_assert(core.u.keyButtonPointer.state == state); \
    g_assert(core.u.keyButtonPointer.eventX == 0); \
    g_assert(core.u.keyButtonPointer.eventY == 0); \
    g_assert(core.u.keyButtonPointer.root == 0); \
    g_assert(core.u.keyButtonPointer.event == 0); \
    g_assert(core.u.keyButtonPointer.child == 0); \
    g_assert(core.u.keyButtonPointer.sameScreen == FALSE);

    x = 0;
    y = 0;
    time = 12345;
    state = 0;
    detail = 0;

    ev.header   = 0xFF;
    ev.length   = sizeof(DeviceEvent);
    ev.time     = time;
    ev.root_y   = x;
    ev.root_x   = y;
    ev.corestate = state;
    ev.detail.key = detail;

    ev.type = type;
    ev.detail.key = 0;
    rc = EventToCore((InternalEvent*)&ev, &core);
    test_event();

    x = 1;
    y = 2;
    ev.root_x = x;
    ev.root_y = y;
    rc = EventToCore((InternalEvent*)&ev, &core);
    test_event();

    x = 0x7FFF;
    y = 0x7FFF;
    ev.root_x = x;
    ev.root_y = y;
    rc = EventToCore((InternalEvent*)&ev, &core);
    test_event();

    x = 0x8000; /* too high */
    y = 0x8000; /* too high */
    ev.root_x = x;
    ev.root_y = y;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(core.u.keyButtonPointer.rootX != x);
    g_assert(core.u.keyButtonPointer.rootY != y);

    x = 0x7FFF;
    y = 0x7FFF;
    ev.root_x = x;
    ev.root_y = y;
    time = 0;
    ev.time = time;
    rc = EventToCore((InternalEvent*)&ev, &core);
    test_event();

    detail = 1;
    ev.detail.key = detail;
    rc = EventToCore((InternalEvent*)&ev, &core);
    test_event();

    detail = 0xFF; /* highest value */
    ev.detail.key = detail;
    rc = EventToCore((InternalEvent*)&ev, &core);
    test_event();

    detail = 0xFFF; /* too big */
    ev.detail.key = detail;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(rc == BadMatch);

    detail = 0xFF; /* too big */
    ev.detail.key = detail;
    state = 0xFFFF; /* highest value */
    ev.corestate = state;
    rc = EventToCore((InternalEvent*)&ev, &core);
    test_event();

    state = 0x10000; /* too big */
    ev.corestate = state;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(core.u.keyButtonPointer.state != state);
    g_assert(core.u.keyButtonPointer.state == (state & 0xFFFF));

#undef test_event
}

static void dix_event_to_core_conversion(void)
{
    DeviceEvent ev;
    xEvent core;
    int rc;

    ev.header   = 0xFF;
    ev.length   = sizeof(DeviceEvent);

    ev.type     = 0;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(rc == BadImplementation);

    ev.type     = 1;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(rc == BadImplementation);

    ev.type     = ET_ProximityOut + 1;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(rc == BadImplementation);

    ev.type     = ET_ProximityIn;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(rc == BadMatch);

    ev.type     = ET_ProximityOut;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(rc == BadMatch);

    dix_event_to_core(ET_KeyPress);
    dix_event_to_core(ET_KeyRelease);
    dix_event_to_core(ET_ButtonPress);
    dix_event_to_core(ET_ButtonRelease);
    dix_event_to_core(ET_Motion);
}

static void xi2_struct_sizes(void)
{
#define compare(req) \
    g_assert(sizeof(req) == sz_##req);

    compare(xXIQueryVersionReq);
    compare(xXIWarpPointerReq);
    compare(xXIChangeCursorReq);
    compare(xXIChangeHierarchyReq);
    compare(xXISetClientPointerReq);
    compare(xXIGetClientPointerReq);
    compare(xXISelectEventsReq);
    compare(xXIQueryVersionReq);
    compare(xXIQueryDeviceReq);
    compare(xXISetFocusReq);
    compare(xXIGetFocusReq);
    compare(xXIGrabDeviceReq);
    compare(xXIUngrabDeviceReq);
    compare(xXIAllowEventsReq);
    compare(xXIPassiveGrabDeviceReq);
    compare(xXIPassiveUngrabDeviceReq);
    compare(xXIListPropertiesReq);
    compare(xXIChangePropertyReq);
    compare(xXIDeletePropertyReq);
    compare(xXIGetPropertyReq);
    compare(xXIGetSelectedEventsReq);
#undef compare
}


static void dix_grab_matching(void)
{
    DeviceIntRec xi_all_devices, xi_all_master_devices, dev1, dev2;
    GrabRec a, b;
    BOOL rc;

    memset(&a, 0, sizeof(a));
    memset(&b, 0, sizeof(b));

    /* different grabtypes must fail */
    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_XI2;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI2;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_CORE;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    /* XI2 grabs for different devices must fail, regardless of ignoreDevice
     * XI2 grabs for master devices must fail against a slave */
    memset(&xi_all_devices, 0, sizeof(DeviceIntRec));
    memset(&xi_all_master_devices, 0, sizeof(DeviceIntRec));
    memset(&dev1, 0, sizeof(DeviceIntRec));
    memset(&dev2, 0, sizeof(DeviceIntRec));

    xi_all_devices.id = XIAllDevices;
    xi_all_master_devices.id = XIAllMasterDevices;
    dev1.id = 10;
    dev1.type = SLAVE;
    dev2.id = 11;
    dev2.type = SLAVE;

    inputInfo.all_devices = &xi_all_devices;
    inputInfo.all_master_devices = &xi_all_master_devices;
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.device = &dev1;
    b.device = &dev2;

    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);

    a.device = &dev2;
    b.device = &dev1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&a, &b, TRUE);
    g_assert(rc == FALSE);

    a.device = inputInfo.all_master_devices;
    b.device = &dev1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&a, &b, TRUE);
    g_assert(rc == FALSE);

    a.device = &dev1;
    b.device = inputInfo.all_master_devices;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&a, &b, TRUE);
    g_assert(rc == FALSE);

    /* ignoreDevice FALSE must fail for different devices for CORE and XI */
    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    a.device = &dev1;
    b.device = &dev2;
    a.modifierDevice = &dev1;
    b.modifierDevice = &dev1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    a.device = &dev1;
    b.device = &dev2;
    a.modifierDevice = &dev1;
    b.modifierDevice = &dev1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);

    /* ignoreDevice FALSE must fail for different modifier devices for CORE
     * and XI */
    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    a.device = &dev1;
    b.device = &dev1;
    a.modifierDevice = &dev1;
    b.modifierDevice = &dev2;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    a.device = &dev1;
    b.device = &dev1;
    a.modifierDevice = &dev1;
    b.modifierDevice = &dev2;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);

    /* different event type must fail */
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.device = &dev1;
    b.device = &dev1;
    a.modifierDevice = &dev1;
    b.modifierDevice = &dev1;
    a.type = XI_KeyPress;
    b.type = XI_KeyRelease;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&a, &b, TRUE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    a.device = &dev1;
    b.device = &dev1;
    a.modifierDevice = &dev1;
    b.modifierDevice = &dev1;
    a.type = XI_KeyPress;
    b.type = XI_KeyRelease;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&a, &b, TRUE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    a.device = &dev1;
    b.device = &dev1;
    a.modifierDevice = &dev1;
    b.modifierDevice = &dev1;
    a.type = XI_KeyPress;
    b.type = XI_KeyRelease;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&a, &b, TRUE);
    g_assert(rc == FALSE);

    /* different modifiers must fail */
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.device = &dev1;
    b.device = &dev1;
    a.modifierDevice = &dev1;
    b.modifierDevice = &dev1;
    a.type = XI_KeyPress;
    b.type = XI_KeyPress;
    a.modifiersDetail.exact = 1;
    b.modifiersDetail.exact = 2;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    /* AnyModifier must fail for XI2 */
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.modifiersDetail.exact = AnyModifier;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    /* XIAnyModifier must fail for CORE and XI */
    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    a.modifiersDetail.exact = XIAnyModifier;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    a.modifiersDetail.exact = XIAnyModifier;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    /* different detail must fail */
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.detail.exact = 1;
    b.detail.exact = 2;
    a.modifiersDetail.exact = 1;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    /* detail of AnyModifier must fail */
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.detail.exact = AnyModifier;
    b.detail.exact = 1;
    a.modifiersDetail.exact = 1;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    /* detail of XIAnyModifier must fail */
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.detail.exact = XIAnyModifier;
    b.detail.exact = 1;
    a.modifiersDetail.exact = 1;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == FALSE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == FALSE);

    /* XIAnyModifier or AnyModifer must succeed */
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.detail.exact = 1;
    b.detail.exact = 1;
    a.modifiersDetail.exact = XIAnyModifier;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == TRUE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == TRUE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    a.detail.exact = 1;
    b.detail.exact = 1;
    a.modifiersDetail.exact = AnyModifier;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == TRUE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == TRUE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    a.detail.exact = 1;
    b.detail.exact = 1;
    a.modifiersDetail.exact = AnyModifier;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == TRUE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == TRUE);

    /* AnyKey or XIAnyKeycode must succeed */
    a.grabtype = GRABTYPE_XI2;
    b.grabtype = GRABTYPE_XI2;
    a.detail.exact = XIAnyKeycode;
    b.detail.exact = 1;
    a.modifiersDetail.exact = 1;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == TRUE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == TRUE);

    a.grabtype = GRABTYPE_CORE;
    b.grabtype = GRABTYPE_CORE;
    a.detail.exact = AnyKey;
    b.detail.exact = 1;
    a.modifiersDetail.exact = 1;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == TRUE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == TRUE);

    a.grabtype = GRABTYPE_XI;
    b.grabtype = GRABTYPE_XI;
    a.detail.exact = AnyKey;
    b.detail.exact = 1;
    a.modifiersDetail.exact = 1;
    b.modifiersDetail.exact = 1;
    rc = GrabMatchesSecond(&a, &b, FALSE);
    g_assert(rc == TRUE);
    rc = GrabMatchesSecond(&b, &a, FALSE);
    g_assert(rc == TRUE);
}

static void include_byte_padding_macros(void)
{
    int i;
    g_test_message("Testing bits_to_bytes()");

    /* the macros don't provide overflow protection */
    for (i = 0; i < INT_MAX - 7; i++)
    {
        int expected_bytes;
        expected_bytes = (i + 7)/8;

        g_assert(bits_to_bytes(i) >= i/8);
        g_assert((bits_to_bytes(i) * 8) - i <= 7);
    }

    g_test_message("Testing bytes_to_int32()");
    for (i = 0; i < INT_MAX - 3; i++)
    {
        int expected_4byte;
        expected_4byte = (i + 3)/4;

        g_assert(bytes_to_int32(i) <= i);
        g_assert((bytes_to_int32(i) * 4) - i <= 3);
    }

    g_test_message("Testing pad_to_int32");

    for (i = 0; i < INT_MAX - 3; i++)
    {
        int expected_bytes;
        expected_bytes = ((i + 3)/4) * 4;

        g_assert(pad_to_int32(i) >= i);
        g_assert(pad_to_int32(i) - i <= 3);
    }

}

static void xi_unregister_handlers(void)
{
    DeviceIntRec dev;
    int handler;

    memset(&dev, 0, sizeof(dev));

    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 1);
    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 2);
    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 3);

    g_test_message("Unlinking from front.");

    XIUnregisterPropertyHandler(&dev, 4); /* NOOP */
    g_assert(dev.properties.handlers->id == 3);
    XIUnregisterPropertyHandler(&dev, 3);
    g_assert(dev.properties.handlers->id == 2);
    XIUnregisterPropertyHandler(&dev, 2);
    g_assert(dev.properties.handlers->id == 1);
    XIUnregisterPropertyHandler(&dev, 1);
    g_assert(dev.properties.handlers == NULL);

    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 4);
    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 5);
    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 6);
    XIUnregisterPropertyHandler(&dev, 3); /* NOOP */
    g_assert(dev.properties.handlers->next->next->next == NULL);
    XIUnregisterPropertyHandler(&dev, 4);
    g_assert(dev.properties.handlers->next->next == NULL);
    XIUnregisterPropertyHandler(&dev, 5);
    g_assert(dev.properties.handlers->next == NULL);
    XIUnregisterPropertyHandler(&dev, 6);
    g_assert(dev.properties.handlers == NULL);

    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 7);
    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 8);
    handler = XIRegisterPropertyHandler(&dev, NULL, NULL, NULL);
    g_assert(handler == 9);

    XIDeleteAllDeviceProperties(&dev);
    g_assert(dev.properties.handlers == NULL);
    XIUnregisterPropertyHandler(&dev, 7); /* NOOP */

}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    g_test_add_func("/dix/input/init-valuators", dix_init_valuators);
    g_test_add_func("/dix/input/event-core-conversion", dix_event_to_core_conversion);
    g_test_add_func("/dix/input/check-grab-values", dix_check_grab_values);
    g_test_add_func("/dix/input/xi2-struct-sizes", xi2_struct_sizes);
    g_test_add_func("/dix/input/grab_matching", dix_grab_matching);
    g_test_add_func("/include/byte_padding_macros", include_byte_padding_macros);
    g_test_add_func("/Xi/xiproperty/register-unregister", xi_unregister_handlers);

    return g_test_run();
}
