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
#include "exglobals.h"
#include "dixgrabs.h"
#include "eventstr.h"
#include "inpututils.h"
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
    g_assert(val->axisVal);

    for (i = 0; i < num_axes; i++)
    {
        g_assert(val->axisVal[i] == 0);
        g_assert(val->axes->min_value == NO_AXIS_LIMITS);
        g_assert(val->axes->max_value == NO_AXIS_LIMITS);
        g_assert(val->axes->mode == Absolute);
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
    const int ROOT_WINDOW_ID = 0x100;

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
    g_assert(core.u.keyButtonPointer.root == ROOT_WINDOW_ID); \
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
    SetBit(ev.valuators.mask, 0);
    SetBit(ev.valuators.mask, 1);
    ev.root     = ROOT_WINDOW_ID;
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

static void dix_event_to_core_fail(int evtype, int expected_rc)
{
    DeviceEvent ev;
    xEvent core;
    int rc;

    ev.header   = 0xFF;
    ev.length   = sizeof(DeviceEvent);

    ev.type     = evtype;
    rc = EventToCore((InternalEvent*)&ev, &core);
    g_assert(rc == expected_rc);
}

static void dix_event_to_core_conversion(void)
{
    dix_event_to_core_fail(0, BadImplementation);
    dix_event_to_core_fail(1, BadImplementation);
    dix_event_to_core_fail(ET_ProximityOut + 1, BadImplementation);
    dix_event_to_core_fail(ET_ProximityIn, BadMatch);
    dix_event_to_core_fail(ET_ProximityOut, BadMatch);

    dix_event_to_core(ET_KeyPress);
    dix_event_to_core(ET_KeyRelease);
    dix_event_to_core(ET_ButtonPress);
    dix_event_to_core(ET_ButtonRelease);
    dix_event_to_core(ET_Motion);
}

static void
_dix_test_xi_convert(DeviceEvent *ev, int expected_rc, int expected_count)
{
    xEvent *xi;
    int count = 0;
    int rc;

    rc = EventToXI((InternalEvent*)ev, &xi, &count);
    g_assert(rc == expected_rc);
    g_assert(count >= expected_count);
    if (count > 0){
        deviceKeyButtonPointer *kbp = (deviceKeyButtonPointer*)xi;
        g_assert(kbp->type == IEventBase + ev->type);
        g_assert(kbp->detail == ev->detail.key);
        g_assert(kbp->time == ev->time);
        g_assert((kbp->deviceid & ~MORE_EVENTS) == ev->deviceid);
        g_assert(kbp->root_x == ev->root_x);
        g_assert(kbp->root_y == ev->root_y);
        g_assert(kbp->state == ev->corestate);
        g_assert(kbp->event_x == 0);
        g_assert(kbp->event_y == 0);
        g_assert(kbp->root == ev->root);
        g_assert(kbp->event == 0);
        g_assert(kbp->child == 0);
        g_assert(kbp->same_screen == FALSE);

        while (--count > 0) {
            deviceValuator *v = (deviceValuator*)&xi[count];
            g_assert(v->type == DeviceValuator);
            g_assert(v->num_valuators <= 6);
        }


        free(xi);
    }
}

/**
 * This tests for internal event → XI1 event conversion
 * - all conversions should generate the right XI event type
 * - right number of events generated
 * - extra events are valuators
 */
static void dix_event_to_xi1_conversion(void)
{
    DeviceEvent ev = {0};
    int time;
    int x, y;
    int state;
    int detail;
    const int ROOT_WINDOW_ID = 0x100;
    int deviceid;

    IEventBase = 80;
    DeviceValuator      = IEventBase - 1;
    DeviceKeyPress      = IEventBase + ET_KeyPress;
    DeviceKeyRelease    = IEventBase + ET_KeyRelease;
    DeviceButtonPress   = IEventBase + ET_ButtonPress;
    DeviceButtonRelease = IEventBase + ET_ButtonRelease;
    DeviceMotionNotify  = IEventBase + ET_Motion;
    DeviceFocusIn       = IEventBase + ET_FocusIn;
    DeviceFocusOut      = IEventBase + ET_FocusOut;
    ProximityIn         = IEventBase + ET_ProximityIn;
    ProximityOut        = IEventBase + ET_ProximityOut;

    /* EventToXI callocs */
    x = 0;
    y = 0;
    time = 12345;
    state = 0;
    detail = 0;
    deviceid = 4;

    ev.header   = 0xFF;

    ev.header           = 0xFF;
    ev.length           = sizeof(DeviceEvent);
    ev.time             = time;
    ev.root_y           = x;
    ev.root_x           = y;
    SetBit(ev.valuators.mask, 0);
    SetBit(ev.valuators.mask, 1);
    ev.root             = ROOT_WINDOW_ID;
    ev.corestate        = state;
    ev.detail.key       = detail;
    ev.deviceid         = deviceid;

    /* test all types for bad match */
    ev.type = ET_KeyPress;         _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_KeyRelease;       _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_ButtonPress;      _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_ButtonRelease;    _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_Motion;           _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_ProximityIn;      _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_ProximityOut;     _dix_test_xi_convert(&ev, Success, 1);

    /* No axes */
    ClearBit(ev.valuators.mask, 0);
    ClearBit(ev.valuators.mask, 1);
    ev.type = ET_KeyPress;         _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_KeyRelease;       _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_ButtonPress;      _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_ButtonRelease;    _dix_test_xi_convert(&ev, Success, 1);
    ev.type = ET_Motion;           _dix_test_xi_convert(&ev, BadMatch, 0);
    ev.type = ET_ProximityIn;      _dix_test_xi_convert(&ev, BadMatch, 0);
    ev.type = ET_ProximityOut;     _dix_test_xi_convert(&ev, BadMatch, 0);

    /* more than 6 axes → 2 valuator events */
    SetBit(ev.valuators.mask, 0);
    SetBit(ev.valuators.mask, 1);
    SetBit(ev.valuators.mask, 2);
    SetBit(ev.valuators.mask, 3);
    SetBit(ev.valuators.mask, 4);
    SetBit(ev.valuators.mask, 5);
    SetBit(ev.valuators.mask, 6);
    ev.type = ET_KeyPress;         _dix_test_xi_convert(&ev, Success, 2);
    ev.type = ET_KeyRelease;       _dix_test_xi_convert(&ev, Success, 2);
    ev.type = ET_ButtonPress;      _dix_test_xi_convert(&ev, Success, 2);
    ev.type = ET_ButtonRelease;    _dix_test_xi_convert(&ev, Success, 2);
    ev.type = ET_Motion;           _dix_test_xi_convert(&ev, Success, 2);
    ev.type = ET_ProximityIn;      _dix_test_xi_convert(&ev, Success, 2);
    ev.type = ET_ProximityOut;     _dix_test_xi_convert(&ev, Success, 2);


    /* keycode too high */
    ev.type = ET_KeyPress;
    ev.detail.key = 256;
    _dix_test_xi_convert(&ev, Success, 0);

    /* deviceid too high */
    ev.type = ET_KeyPress;
    ev.detail.key = 18;
    ev.deviceid = 128;
    _dix_test_xi_convert(&ev, Success, 0);
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

static void test_bits_to_byte(int i)
{
        int expected_bytes;
        expected_bytes = (i + 7)/8;

        g_assert(bits_to_bytes(i) >= i/8);
        g_assert((bits_to_bytes(i) * 8) - i <= 7);
        g_assert(expected_bytes == bits_to_bytes(i));
}

static void test_bytes_to_int32(int i)
{
        int expected_4byte;
        expected_4byte = (i + 3)/4;

        g_assert(bytes_to_int32(i) <= i);
        g_assert((bytes_to_int32(i) * 4) - i <= 3);
        g_assert(expected_4byte == bytes_to_int32(i));
}

static void test_pad_to_int32(int i)
{
        int expected_bytes;
        expected_bytes = ((i + 3)/4) * 4;

        g_assert(pad_to_int32(i) >= i);
        g_assert(pad_to_int32(i) - i <= 3);
        g_assert(expected_bytes == pad_to_int32(i));
}
static void include_byte_padding_macros(void)
{
    g_test_message("Testing bits_to_bytes()");

    /* the macros don't provide overflow protection */
    test_bits_to_byte(0);
    test_bits_to_byte(1);
    test_bits_to_byte(2);
    test_bits_to_byte(7);
    test_bits_to_byte(8);
    test_bits_to_byte(0xFF);
    test_bits_to_byte(0x100);
    test_bits_to_byte(INT_MAX - 9);
    test_bits_to_byte(INT_MAX - 8);

    g_test_message("Testing bytes_to_int32()");

    test_bytes_to_int32(0);
    test_bytes_to_int32(1);
    test_bytes_to_int32(2);
    test_bytes_to_int32(7);
    test_bytes_to_int32(8);
    test_bytes_to_int32(0xFF);
    test_bytes_to_int32(0x100);
    test_bytes_to_int32(0xFFFF);
    test_bytes_to_int32(0x10000);
    test_bytes_to_int32(0xFFFFFF);
    test_bytes_to_int32(0x1000000);
    test_bytes_to_int32(INT_MAX - 4);
    test_bytes_to_int32(INT_MAX - 3);

    g_test_message("Testing pad_to_int32");

    test_pad_to_int32(0);
    test_pad_to_int32(0);
    test_pad_to_int32(1);
    test_pad_to_int32(2);
    test_pad_to_int32(7);
    test_pad_to_int32(8);
    test_pad_to_int32(0xFF);
    test_pad_to_int32(0x100);
    test_pad_to_int32(0xFFFF);
    test_pad_to_int32(0x10000);
    test_pad_to_int32(0xFFFFFF);
    test_pad_to_int32(0x1000000);
    test_pad_to_int32(INT_MAX - 4);
    test_pad_to_int32(INT_MAX - 3);
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

static void cmp_attr_fields(InputAttributes *attr1,
                            InputAttributes *attr2)
{
    char **tags1, **tags2;

    g_assert(attr1 && attr2);
    g_assert(attr1 != attr2);
    g_assert(attr1->flags == attr2->flags);

    if (attr1->product != NULL)
    {
        g_assert(attr1->product != attr2->product);
        g_assert(strcmp(attr1->product, attr2->product) == 0);
    } else
        g_assert(attr2->product == NULL);

    if (attr1->vendor != NULL)
    {
        g_assert(attr1->vendor != attr2->vendor);
        g_assert(strcmp(attr1->vendor, attr2->vendor) == 0);
    } else
        g_assert(attr2->vendor == NULL);

    if (attr1->device != NULL)
    {
        g_assert(attr1->device != attr2->device);
        g_assert(strcmp(attr1->device, attr2->device) == 0);
    } else
        g_assert(attr2->device == NULL);

    if (attr1->pnp_id != NULL)
    {
        g_assert(attr1->pnp_id != attr2->pnp_id);
        g_assert(strcmp(attr1->pnp_id, attr2->pnp_id) == 0);
    } else
        g_assert(attr2->pnp_id == NULL);

    if (attr1->usb_id != NULL)
    {
        g_assert(attr1->usb_id != attr2->usb_id);
        g_assert(strcmp(attr1->usb_id, attr2->usb_id) == 0);
    } else
        g_assert(attr2->usb_id == NULL);

    tags1 = attr1->tags;
    tags2 = attr2->tags;

    /* if we don't have any tags, skip the tag checking bits */
    if (!tags1)
    {
        g_assert(!tags2);
        return;
    }

    /* Don't lug around empty arrays */
    g_assert(*tags1);
    g_assert(*tags2);

    /* check for identical content, but duplicated */
    while (*tags1)
    {
        g_assert(*tags1 != *tags2);
        g_assert(strcmp(*tags1, *tags2) == 0);
        tags1++;
        tags2++;
    }

    /* ensure tags1 and tags2 have the same no of elements */
    g_assert(!*tags2);

    /* check for not sharing memory */
    tags1 = attr1->tags;
    while (*tags1)
    {
        tags2 = attr2->tags;
        while (*tags2)
            g_assert(*tags1 != *tags2++);

        tags1++;
    }
}

static void dix_input_attributes(void)
{
    InputAttributes orig = {0};
    InputAttributes *new;
    char *tags[4] = {"tag1", "tag2", "tag2", NULL};

    new = DuplicateInputAttributes(NULL);
    g_assert(!new);

    new = DuplicateInputAttributes(&orig);
    g_assert(memcmp(&orig, new, sizeof(InputAttributes)) == 0);

    orig.product = "product name";
    new = DuplicateInputAttributes(&orig);
    cmp_attr_fields(&orig, new);
    FreeInputAttributes(new);

    orig.vendor = "vendor name";
    new = DuplicateInputAttributes(&orig);
    cmp_attr_fields(&orig, new);
    FreeInputAttributes(new);

    orig.device = "device path";
    new = DuplicateInputAttributes(&orig);
    cmp_attr_fields(&orig, new);
    FreeInputAttributes(new);

    orig.pnp_id = "PnPID";
    new = DuplicateInputAttributes(&orig);
    cmp_attr_fields(&orig, new);
    FreeInputAttributes(new);

    orig.usb_id = "USBID";
    new = DuplicateInputAttributes(&orig);
    cmp_attr_fields(&orig, new);
    FreeInputAttributes(new);

    orig.flags = 0xF0;
    new = DuplicateInputAttributes(&orig);
    cmp_attr_fields(&orig, new);
    FreeInputAttributes(new);

    orig.tags = tags;
    new = DuplicateInputAttributes(&orig);
    cmp_attr_fields(&orig, new);
    FreeInputAttributes(new);
}

static void dix_input_valuator_masks(void)
{
    ValuatorMask *mask = NULL, *copy;
    int nvaluators = MAX_VALUATORS;
    int valuators[nvaluators];
    int i;
    int first_val, num_vals;

    for (i = 0; i < nvaluators; i++)
        valuators[i] = i;

    mask = valuator_mask_new(nvaluators);
    g_assert(mask != NULL);
    g_assert(valuator_mask_size(mask) == 0);
    g_assert(valuator_mask_num_valuators(mask) == 0);

    for (i = 0; i < nvaluators; i++)
    {
        g_assert(!valuator_mask_isset(mask, i));
        valuator_mask_set(mask, i, valuators[i]);
        g_assert(valuator_mask_isset(mask, i));
        g_assert(valuator_mask_get(mask, i) == valuators[i]);
        g_assert(valuator_mask_size(mask) == i + 1);
        g_assert(valuator_mask_num_valuators(mask) == i + 1);
    }

    for (i = 0; i < nvaluators; i++)
    {
        g_assert(valuator_mask_isset(mask, i));
        valuator_mask_unset(mask, i);
        /* we're removing valuators from the front, so size should stay the
         * same until the last bit is removed */
        if (i < nvaluators - 1)
            g_assert(valuator_mask_size(mask) == nvaluators);
        g_assert(!valuator_mask_isset(mask, i));
    }

    g_assert(valuator_mask_size(mask) == 0);
    valuator_mask_zero(mask);
    g_assert(valuator_mask_size(mask) == 0);
    g_assert(valuator_mask_num_valuators(mask) == 0);
    for (i = 0; i < nvaluators; i++)
        g_assert(!valuator_mask_isset(mask, i));

    first_val = 5;
    num_vals = 6;

    valuator_mask_set_range(mask, first_val, num_vals, valuators);
    g_assert(valuator_mask_size(mask) == first_val + num_vals);
    g_assert(valuator_mask_num_valuators(mask) == num_vals);
    for (i = 0; i < nvaluators; i++)
    {
        if (i < first_val || i >= first_val + num_vals)
            g_assert(!valuator_mask_isset(mask, i));
        else
        {
            g_assert(valuator_mask_isset(mask, i));
            g_assert(valuator_mask_get(mask, i) == valuators[i - first_val]);
        }
    }

    copy = valuator_mask_new(nvaluators);
    valuator_mask_copy(copy, mask);
    g_assert(mask != copy);
    g_assert(valuator_mask_size(mask) == valuator_mask_size(copy));
    g_assert(valuator_mask_num_valuators(mask) == valuator_mask_num_valuators(copy));

    for (i = 0; i < nvaluators; i++)
    {
        g_assert(valuator_mask_isset(mask, i) == valuator_mask_isset(copy, i));
        g_assert(valuator_mask_get(mask, i) == valuator_mask_get(copy, i));
    }

    valuator_mask_free(&mask);
    g_assert(mask == NULL);
}

static void dix_valuator_mode(void)
{
    DeviceIntRec dev;
    const int num_axes = MAX_VALUATORS;
    int i;
    Atom atoms[MAX_VALUATORS] = { 0 };

    memset(&dev, 0, sizeof(DeviceIntRec));
    dev.type = MASTER_POINTER; /* claim it's a master to stop ptracccel */

    g_assert(InitValuatorClassDeviceStruct(NULL, 0, atoms, 0, 0) == FALSE);
    g_assert(InitValuatorClassDeviceStruct(&dev, num_axes, atoms, 0, Absolute));

    for (i = 0; i < num_axes; i++)
    {
        g_assert(valuator_get_mode(&dev, i) == Absolute);
        valuator_set_mode(&dev, i, Relative);
        g_assert(dev.valuator->axes[i].mode == Relative);
        g_assert(valuator_get_mode(&dev, i) == Relative);
    }

    valuator_set_mode(&dev, VALUATOR_MODE_ALL_AXES, Absolute);
    for (i = 0; i < num_axes; i++)
        g_assert(valuator_get_mode(&dev, i) == Absolute);

    valuator_set_mode(&dev, VALUATOR_MODE_ALL_AXES, Relative);
    for (i = 0; i < num_axes; i++)
        g_assert(valuator_get_mode(&dev, i) == Relative);
}

static void include_bit_test_macros(void)
{
    uint8_t mask[9] = { 0 };
    int i;

    for (i = 0; i < sizeof(mask)/sizeof(mask[0]); i++)
    {
        g_assert(BitIsOn(mask, i) == 0);
        SetBit(mask, i);
        g_assert(BitIsOn(mask, i) == 1);
        g_assert(!!(mask[i/8] & (1 << (i % 8))));
        g_assert(CountBits(mask, sizeof(mask)) == 1);
        ClearBit(mask, i);
        g_assert(BitIsOn(mask, i) == 0);
    }
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv,NULL);
    g_test_bug_base("https://bugzilla.freedesktop.org/show_bug.cgi?id=");

    g_test_add_func("/dix/input/valuator-masks", dix_input_valuator_masks);
    g_test_add_func("/dix/input/attributes", dix_input_attributes);
    g_test_add_func("/dix/input/init-valuators", dix_init_valuators);
    g_test_add_func("/dix/input/event-core-conversion", dix_event_to_core_conversion);
    g_test_add_func("/dix/input/event-xi1-conversion", dix_event_to_xi1_conversion);
    g_test_add_func("/dix/input/check-grab-values", dix_check_grab_values);
    g_test_add_func("/dix/input/xi2-struct-sizes", xi2_struct_sizes);
    g_test_add_func("/dix/input/grab_matching", dix_grab_matching);
    g_test_add_func("/dix/input/valuator_mode", dix_valuator_mode);
    g_test_add_func("/include/byte_padding_macros", include_byte_padding_macros);
    g_test_add_func("/include/bit_test_macros", include_bit_test_macros);
    g_test_add_func("/Xi/xiproperty/register-unregister", xi_unregister_handlers);


    return g_test_run();
}
