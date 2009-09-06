/*
 * Copyright © 2008 Daniel Stone
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
 * Author: Daniel Stone <daniel@fooishbar.org>
 */

#ifdef HAVE_DIX_CONFIG_H
#include "dix-config.h"
#endif

#include "exevents.h"
#include "exglobals.h"
#include "misc.h"
#include "input.h"
#include "inputstr.h"
#include "xace.h"
#include "xkbsrv.h"
#include "xkbstr.h"

/* Check if a button map change is okay with the device.
 * Returns -1 for BadValue, as it collides with MappingBusy. */
static int
check_butmap_change(DeviceIntPtr dev, CARD8 *map, int len, CARD32 *errval_out,
                    ClientPtr client)
{
    int i, ret;

    if (!dev || !dev->button)
    {
        client->errorValue = (dev) ? dev->id : 0;
        return BadDevice;
    }

    ret = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixManageAccess);
    if (ret != Success)
    {
        client->errorValue = dev->id;
        return ret;
    }

    for (i = 0; i < len; i++) {
        if (dev->button->map[i + 1] != map[i] && dev->button->down[i + 1])
            return MappingBusy;
    }

    return Success;
}

static void
do_butmap_change(DeviceIntPtr dev, CARD8 *map, int len, ClientPtr client)
{
    int i;
    xEvent core_mn;
    deviceMappingNotify xi_mn;

    /* The map in ButtonClassRec refers to button numbers, whereas the
     * protocol is zero-indexed.  Sigh. */
    memcpy(&(dev->button->map[1]), map, len);

    core_mn.u.u.type = MappingNotify;
    core_mn.u.mappingNotify.request = MappingPointer;

    /* 0 is the server client. */
    for (i = 1; i < currentMaxClients; i++) {
        /* Don't send irrelevant events to naïve clients. */
        if (!clients[i] || clients[i]->clientState != ClientStateRunning)
            continue;

        if (!XIShouldNotify(clients[i], dev))
            continue;

        core_mn.u.u.sequenceNumber = clients[i]->sequence;
        WriteEventsToClient(clients[i], 1, &core_mn);
    }

    xi_mn.type = DeviceMappingNotify;
    xi_mn.request = MappingPointer;
    xi_mn.deviceid = dev->id;
    xi_mn.time = GetTimeInMillis();

    SendEventToAllWindows(dev, DeviceMappingNotifyMask, (xEvent *) &xi_mn, 1);
}

/*
 * Does what it says on the box, both for core and Xi.
 *
 * Faithfully reports any errors encountered while trying to apply the map
 * to the requested device, faithfully ignores any errors encountered while
 * trying to apply the map to its master/slaves.
 */
int
ApplyPointerMapping(DeviceIntPtr dev, CARD8 *map, int len, ClientPtr client)
{
    int ret;

    /* If we can't perform the change on the requested device, bail out. */
    ret = check_butmap_change(dev, map, len, &client->errorValue, client);
    if (ret != Success)
        return ret;
    do_butmap_change(dev, map, len, client);

    return Success;
}

/* Check if a modifier map change is okay with the device.
 * Returns -1 for BadValue, as it collides with MappingBusy; this particular
 * caveat can be removed with LegalModifier, as we have no other reason to
 * set MappingFailed.  Sigh. */
static int
check_modmap_change(ClientPtr client, DeviceIntPtr dev, KeyCode *modmap)
{
    int ret, i;
    XkbDescPtr xkb;

    ret = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixManageAccess);
    if (ret != Success)
        return ret;

    if (!dev->key)
        return BadMatch;
    xkb = dev->key->xkbInfo->desc;

    for (i = 0; i < MAP_LENGTH; i++) {
        if (!modmap[i])
            continue;

        /* Check that all the new modifiers fall within the advertised
         * keycode range. */
        if (i < xkb->min_key_code || i > xkb->max_key_code) {
            client->errorValue = i;
            return -1;
        }

        /* Make sure the mapping is okay with the DDX. */
        if (!LegalModifier(i, dev)) {
            client->errorValue = i;
            return MappingFailed;
        }

        /* None of the new modifiers may be down while we change the
         * map. */
        if (key_is_down(dev, i, KEY_POSTED | KEY_PROCESSED)) {
            client->errorValue = i;
            return MappingBusy;
        }
    }

    /* None of the old modifiers may be down while we change the map,
     * either. */
    for (i = xkb->min_key_code; i < xkb->max_key_code; i++) {
        if (!xkb->map->modmap[i])
            continue;
        if (key_is_down(dev, i, KEY_POSTED | KEY_PROCESSED)) {
            client->errorValue = i;
            return MappingBusy;
        }
    }

    return Success;
}

static int
check_modmap_change_slave(ClientPtr client, DeviceIntPtr master,
                          DeviceIntPtr slave, CARD8 *modmap)
{
    XkbDescPtr master_xkb, slave_xkb;
    int i, j;

    if (!slave->key || !master->key)
        return 0;

    master_xkb = master->key->xkbInfo->desc;
    slave_xkb = slave->key->xkbInfo->desc;

    /* Ignore devices with a clearly different keymap. */
    if (slave_xkb->min_key_code != master_xkb->min_key_code ||
        slave_xkb->max_key_code != master_xkb->max_key_code)
        return 0;

    for (i = 0; i < MAP_LENGTH; i++) {
        if (!modmap[i])
            continue;

        /* If we have different symbols for any modifier on an
         * extended keyboard, ignore the whole remap request. */
        for (j = 0;
             j < XkbKeyNumSyms(slave_xkb, i) &&
              j < XkbKeyNumSyms(master_xkb, i);
             j++)
            if (XkbKeySymsPtr(slave_xkb, i)[j] != XkbKeySymsPtr(master_xkb, i)[j])
                return 0;
    }

    if (check_modmap_change(client, slave, modmap) != Success)
        return 0;

    return 1;
}

/* Actually change the modifier map, and send notifications.  Cannot fail. */
static void
do_modmap_change(ClientPtr client, DeviceIntPtr dev, CARD8 *modmap)
{
    XkbApplyMappingChange(dev, NULL, 0, 0, modmap, serverClient);
}

/* Rebuild modmap (key -> mod) from map (mod -> key). */
static int build_modmap_from_modkeymap(CARD8 *modmap, KeyCode *modkeymap,
                                       int max_keys_per_mod)
{
    int i, len = max_keys_per_mod * 8;

    memset(modmap, 0, MAP_LENGTH);

    for (i = 0; i < len; i++) {
        if (!modkeymap[i])
            continue;

        if (modkeymap[i] >= MAP_LENGTH)
            return BadValue;

        if (modmap[modkeymap[i]])
            return BadValue;

        modmap[modkeymap[i]] = 1 << (i / max_keys_per_mod);
    }

    return Success;
}

int
change_modmap(ClientPtr client, DeviceIntPtr dev, KeyCode *modkeymap,
              int max_keys_per_mod)
{
    int ret;
    CARD8 modmap[MAP_LENGTH];
    DeviceIntPtr tmp;

    ret = build_modmap_from_modkeymap(modmap, modkeymap, max_keys_per_mod);
    if (ret != Success)
        return ret;

    /* If we can't perform the change on the requested device, bail out. */
    ret = check_modmap_change(client, dev, modmap);
    if (ret != Success)
        return ret;
    do_modmap_change(client, dev, modmap);

    /* Change any attached masters/slaves. */
    if (IsMaster(dev)) {
        for (tmp = inputInfo.devices; tmp; tmp = tmp->next) {
            if (!IsMaster(tmp) && tmp->u.master == dev)
                if (check_modmap_change_slave(client, dev, tmp, modmap))
                    do_modmap_change(client, tmp, modmap);
        }
    }
    else if (dev->u.master && dev->u.master->u.lastSlave == dev) {
        /* If this fails, expect the results to be weird. */
        if (check_modmap_change(client, dev->u.master, modmap))
            do_modmap_change(client, dev->u.master, modmap);
    }

    return Success;
}

int generate_modkeymap(ClientPtr client, DeviceIntPtr dev,
                       KeyCode **modkeymap_out, int *max_keys_per_mod_out)
{
    CARD8 keys_per_mod[8];
    int max_keys_per_mod;
    KeyCode *modkeymap;
    int i, j, ret;

    ret = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixGetAttrAccess);
    if (ret != Success)
        return ret;

    if (!dev->key)
        return BadMatch;

    /* Count the number of keys per modifier to determine how wide we
     * should make the map. */
    max_keys_per_mod = 0;
    for (i = 0; i < 8; i++)
        keys_per_mod[i] = 0;
    for (i = 8; i < MAP_LENGTH; i++) {
        for (j = 0; j < 8; j++) {
            if (dev->key->xkbInfo->desc->map->modmap[i] & (1 << j)) {
                if (++keys_per_mod[j] > max_keys_per_mod)
                    max_keys_per_mod = keys_per_mod[j];
            }
        }
    }

    modkeymap = xcalloc(max_keys_per_mod * 8, sizeof(KeyCode));
    if (!modkeymap)
        return BadAlloc;

    for (i = 0; i < 8; i++)
        keys_per_mod[i] = 0;

    for (i = 8; i < MAP_LENGTH; i++) {
        for (j = 0; j < 8; j++) {
            if (dev->key->xkbInfo->desc->map->modmap[i] & (1 << j)) {
                modkeymap[(j * max_keys_per_mod) + keys_per_mod[j]] = i;
                keys_per_mod[j]++;
            }
        }
    }

    *max_keys_per_mod_out = max_keys_per_mod;
    *modkeymap_out = modkeymap;

    return Success;
}
