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
#include "extinit.h" /* for XInputExtensionInit */
#include <glib.h>

#include "protocol-common.h"

struct devices devices;
ScreenRec screen;
WindowRec root;
WindowRec window;

void *userdata;

extern int CorePointerProc(DeviceIntPtr pDev, int what);
extern int CoreKeyboardProc(DeviceIntPtr pDev, int what);

static void fake_init_sprite(DeviceIntPtr dev)
{
    SpritePtr sprite;
    sprite = dev->spriteInfo->sprite;

    sprite->spriteTraceSize = 10;
    sprite->spriteTrace = xcalloc(sprite->spriteTraceSize, sizeof(WindowPtr));
    sprite->spriteTraceGood = 1;
    sprite->spriteTrace[0] = &root;
    sprite->hot.x = SPRITE_X;
    sprite->hot.y = SPRITE_Y;
    sprite->hotPhys.x = sprite->hot.x;
    sprite->hotPhys.y = sprite->hot.y;
    sprite->win = &window;
    sprite->hotPhys.pScreen = &screen;
    sprite->physLimits.x1 = 0;
    sprite->physLimits.y1 = 0;
    sprite->physLimits.x2 = screen.width;
    sprite->physLimits.y2 = screen.height;
}

/**
 * Create and init 2 master devices (VCP + VCK) and two slave devices, one
 * default mouse, one default keyboard.
 */
struct devices init_devices(void)
{
    ClientRec client;
    struct devices devices;

    client = init_client(0, NULL);

    AllocDevicePair(&client, "Virtual core", &devices.vcp, &devices.vck,
                    CorePointerProc, CoreKeyboardProc, TRUE);
    inputInfo.pointer = devices.vcp;
    inputInfo.keyboard = devices.vck;
    ActivateDevice(devices.vcp, FALSE);
    ActivateDevice(devices.vck, FALSE);
    EnableDevice(devices.vcp, FALSE);
    EnableDevice(devices.vck, FALSE);

    AllocDevicePair(&client, "", &devices.mouse, &devices.kbd,
                    CorePointerProc, CoreKeyboardProc, FALSE);
    ActivateDevice(devices.mouse, FALSE);
    ActivateDevice(devices.kbd, FALSE);
    EnableDevice(devices.mouse, FALSE);
    EnableDevice(devices.kbd, FALSE);

    devices.num_devices = 4;
    devices.num_master_devices = 2;

    fake_init_sprite(devices.mouse);
    fake_init_sprite(devices.vcp);

    return devices;
}


/* Create minimal client, with the given buffer and len as request buffer */
ClientRec init_client(int len, void *data)
{
    ClientRec client = { 0 };

    /* we store the privates now and reassign it after the memset. this way
     * we can share them across multiple test runs and don't have to worry
     * about freeing them after each test run. */
    PrivateRec *privates = client.devPrivates;

    client.index = CLIENT_INDEX;
    client.clientAsMask = CLIENT_MASK;
    client.sequence = CLIENT_SEQUENCE;
    client.req_len = len;

    client.requestBuffer = data;
    client.devPrivates = privates;
    return client;
}

void init_window(WindowPtr window, WindowPtr parent, int id)
{
    memset(window, 0, sizeof(window));

    window->drawable.id = id;
    if (parent)
    {
        window->drawable.x = 30;
        window->drawable.y = 50;
        window->drawable.width = 100;
        window->drawable.height = 200;
    }
    window->parent = parent;
    window->optional = xcalloc(1, sizeof(WindowOptRec));
    g_assert(window->optional);
}

/* Needed for the screen setup, otherwise we crash during sprite initialization */
static Bool device_cursor_init(DeviceIntPtr dev, ScreenPtr screen) { return TRUE; }
static Bool set_cursor_pos(DeviceIntPtr dev, ScreenPtr screen, int x, int y, Bool event) { return TRUE; }
void init_simple(void)
{
    screenInfo.arraySize = MAXSCREENS;
    screenInfo.numScreens = 1;
    screenInfo.screens[0] = &screen;

    screen.myNum = 0;
    screen.id = 100;
    screen.width = 640;
    screen.height = 480;
    screen.DeviceCursorInitialize = device_cursor_init;
    screen.SetCursorPosition = set_cursor_pos;

    dixResetPrivates();
    XInputExtensionInit();
    init_window(&root, NULL, ROOT_WINDOW_ID);
    init_window(&window, &root, CLIENT_WINDOW_ID);

    devices = init_devices();
}

void __wrap_WriteToClient(ClientPtr client, int len, void *data)
{
    g_assert(reply_handler != NULL);

    (*reply_handler)(client, len, data, userdata);
}

