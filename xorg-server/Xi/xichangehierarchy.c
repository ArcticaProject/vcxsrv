/*
 * Copyright 2007-2008 Peter Hutterer
 * Copyright 2009 Red Hat, Inc.
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
 * Author: Peter Hutterer, University of South Australia, NICTA
 */

/***********************************************************************
 *
 * Request change in the device hierarchy.
 *
 */


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>	/* for inputstr.h    */
#include <X11/Xproto.h>	/* Request macro     */
#include "inputstr.h"	/* DeviceIntPtr      */
#include "windowstr.h"	/* window structure  */
#include "scrnintstr.h"	/* screen structure  */
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/geproto.h>
#include "extnsionst.h"
#include "exevents.h"
#include "exglobals.h"
#include "geext.h"
#include "xace.h"
#include "xiquerydevice.h" /* for GetDeviceUse */

#include "xkbsrv.h"

#include "xichangehierarchy.h"

/**
 * Send the current state of the device hierarchy to all clients.
 */
void XISendDeviceHierarchyEvent(int flags[MAXDEVICES])
{
    xXIHierarchyEvent *ev;
    xXIHierarchyInfo *info;
    DeviceIntRec dummyDev;
    DeviceIntPtr dev;
    int i;

    if (!flags)
        return;

    ev = xcalloc(1, sizeof(xXIHierarchyEvent) +
                 MAXDEVICES * sizeof(xXIHierarchyInfo));
    ev->type = GenericEvent;
    ev->extension = IReqCode;
    ev->evtype = XI_HierarchyChanged;
    ev->time = GetTimeInMillis();
    ev->flags = 0;
    ev->num_info = inputInfo.numDevices;

    info = (xXIHierarchyInfo*)&ev[1];
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
        info->deviceid = dev->id;
        info->enabled = dev->enabled;
        info->use = GetDeviceUse(dev, &info->attachment);
        info->flags = flags[dev->id];
        ev->flags |= info->flags;
        info++;
    }
    for (dev = inputInfo.off_devices; dev; dev = dev->next)
    {
        info->deviceid = dev->id;
        info->enabled = dev->enabled;
        info->use = GetDeviceUse(dev, &info->attachment);
        info->flags = flags[dev->id];
        ev->flags |= info->flags;
        info++;
    }


    for (i = 0; i < MAXDEVICES; i++)
    {
        if (flags[i] & (XIMasterRemoved | XISlaveRemoved))
        {
            info->deviceid = i;
            info->enabled = FALSE;
            info->flags = flags[i];
            info->use = 0;
            ev->flags |= info->flags;
            ev->num_info++;
            info++;
        }
    }

    ev->length = bytes_to_int32(ev->num_info * sizeof(xXIHierarchyInfo));

    dummyDev.id = XIAllDevices;
    SendEventToAllWindows(&dummyDev, (XI_HierarchyChangedMask >> 8), (xEvent*)ev, 1);
    xfree(ev);
}


/***********************************************************************
 *
 * This procedure allows a client to change the device hierarchy through
 * adding new master devices, removing them, etc.
 *
 */

int SProcXIChangeHierarchy(ClientPtr client)
{
    char n;

    REQUEST(xXIChangeHierarchyReq);
    swaps(&stuff->length, n);
    return (ProcXIChangeHierarchy(client));
}

#define SWAPIF(cmd) if (client->swapped) { cmd; }

int
ProcXIChangeHierarchy(ClientPtr client)
{
    DeviceIntPtr ptr, keybd, XTestptr, XTestkeybd;
    xXIAnyHierarchyChangeInfo *any;
    int required_len = sizeof(xXIChangeHierarchyReq);
    char n;
    int rc = Success;
    int flags[MAXDEVICES] = {0};

    REQUEST(xXIChangeHierarchyReq);
    REQUEST_AT_LEAST_SIZE(xXIChangeHierarchyReq);

    if (!stuff->num_changes)
        return rc;

    any = (xXIAnyHierarchyChangeInfo*)&stuff[1];
    while(stuff->num_changes--)
    {
        SWAPIF(swapl(&any->type, n));
        SWAPIF(swaps(&any->length, n));

        required_len += any->length;
        if ((stuff->length * 4) < required_len)
            return BadLength;

        switch(any->type)
        {
            case XIAddMaster:
                {
                    xXIAddMasterInfo* c = (xXIAddMasterInfo*)any;
                    char* name;

                    SWAPIF(swaps(&c->name_len, n));
                    name = xcalloc(c->name_len + 1, sizeof(char));
                    strncpy(name, (char*)&c[1], c->name_len);


                    rc = AllocDevicePair(client, name, &ptr, &keybd,
                                         CorePointerProc, CoreKeyboardProc,
                                         TRUE);
                    if (rc != Success)
                    {
                        xfree(name);
                        goto unwind;
                    }

                    if (!c->send_core)
                        ptr->coreEvents = keybd->coreEvents =  FALSE;

                    /* Allocate virtual slave devices for xtest events */
                    rc = AllocXTestDevice(client, name, &XTestptr, &XTestkeybd,
                                         ptr, keybd);
                    if (rc != Success)
                    {

                        xfree(name);
                        goto unwind;
                    }

                    ActivateDevice(ptr, FALSE);
                    ActivateDevice(keybd, FALSE);
                    flags[ptr->id] |= XIMasterAdded;
                    flags[keybd->id] |= XIMasterAdded;

                    ActivateDevice(XTestptr, FALSE);
                    ActivateDevice(XTestkeybd, FALSE);
                    flags[XTestptr->id] |= XISlaveAdded;
                    flags[XTestkeybd->id] |= XISlaveAdded;

                    if (c->enable)
                    {
                        EnableDevice(ptr, FALSE);
                        EnableDevice(keybd, FALSE);
                        flags[ptr->id] |= XIDeviceEnabled;
                        flags[keybd->id] |= XIDeviceEnabled;

                        EnableDevice(XTestptr, FALSE);
                        EnableDevice(XTestkeybd, FALSE);
                        flags[XTestptr->id] |= XIDeviceEnabled;
                        flags[XTestkeybd->id] |= XIDeviceEnabled;
                    }

                    /* Attach the XTest virtual devices to the newly
                       created master device */
                    AttachDevice(NULL, XTestptr, ptr);
                    AttachDevice(NULL, XTestkeybd, keybd);
                    flags[XTestptr->id] |= XISlaveAttached;
                    flags[XTestkeybd->id] |= XISlaveAttached;

                    xfree(name);
                }
                break;
            case XIRemoveMaster:
                {
                    xXIRemoveMasterInfo* r = (xXIRemoveMasterInfo*)any;

                    if (r->return_mode != XIAttachToMaster &&
                            r->return_mode != XIFloating)
                        return BadValue;

                    rc = dixLookupDevice(&ptr, r->deviceid, client,
                                         DixDestroyAccess);
                    if (rc != Success)
                        goto unwind;

                    if (!IsMaster(ptr))
                    {
                        client->errorValue = r->deviceid;
                        rc = BadDevice;
                        goto unwind;
                    }

                    /* XXX: For now, don't allow removal of VCP, VCK */
                    if (ptr == inputInfo.pointer ||
                            ptr == inputInfo.keyboard)
                    {
                        rc = BadDevice;
                        goto unwind;
                    }


                    ptr = GetMaster(ptr, MASTER_POINTER);
                    rc = dixLookupDevice(&ptr,
                                         ptr->id,
                                         client,
                                         DixDestroyAccess);
                    if (rc != Success)
                        goto unwind;
                    keybd = GetMaster(ptr, MASTER_KEYBOARD);
                    rc = dixLookupDevice(&keybd,
                                         keybd->id,
                                         client,
                                         DixDestroyAccess);
                    if (rc != Success)
                        goto unwind;

                    XTestptr = GetXTestDevice(ptr);
                    rc = dixLookupDevice(&XTestptr, XTestptr->id, client,
                                         DixDestroyAccess);
                    if (rc != Success)
                        goto unwind;

                    XTestkeybd = GetXTestDevice(keybd);
                    rc = dixLookupDevice(&XTestkeybd, XTestkeybd->id, client,
                                         DixDestroyAccess);
                    if (rc != Success)
                        goto unwind;

                    /* Disabling sends the devices floating, reattach them if
                     * desired. */
                    if (r->return_mode == XIAttachToMaster)
                    {
                        DeviceIntPtr attached,
                                     newptr,
                                     newkeybd;

                        rc = dixLookupDevice(&newptr, r->return_pointer,
                                             client, DixAddAccess);
                        if (rc != Success)
                            goto unwind;

                        if (!IsMaster(newptr))
                        {
                            client->errorValue = r->return_pointer;
                            rc = BadDevice;
                            goto unwind;
                        }

                        rc = dixLookupDevice(&newkeybd, r->return_keyboard,
                                             client, DixAddAccess);
                        if (rc != Success)
                            goto unwind;

                        if (!IsMaster(newkeybd))
                        {
                            client->errorValue = r->return_keyboard;
                            rc = BadDevice;
                            goto unwind;
                        }

                        for (attached = inputInfo.devices;
                                attached;
                                attached = attached->next)
                        {
                            if (!IsMaster(attached)) {
                                if (attached->u.master == ptr)
                                {
                                    AttachDevice(client, attached, newptr);
                                    flags[attached->id] |= XISlaveAttached;
                                }
                                if (attached->u.master == keybd)
                                {
                                    AttachDevice(client, attached, newkeybd);
                                    flags[attached->id] |= XISlaveAttached;
                                }
                            }
                        }
                    }

                    /* can't disable until we removed pairing */
                    keybd->spriteInfo->paired = NULL;
                    ptr->spriteInfo->paired = NULL;
                    XTestptr->spriteInfo->paired = NULL;
                    XTestkeybd->spriteInfo->paired = NULL;

                    /* disable the remove the devices, XTest devices must be done first
                       else the sprites they rely on will be destroyed  */
                    DisableDevice(XTestptr, FALSE);
                    DisableDevice(XTestkeybd, FALSE);
                    DisableDevice(keybd, FALSE);
                    DisableDevice(ptr, FALSE);
                    flags[XTestptr->id] |= XIDeviceDisabled | XISlaveDetached;
                    flags[XTestkeybd->id] |= XIDeviceDisabled | XISlaveDetached;
                    flags[keybd->id] |= XIDeviceDisabled;
                    flags[ptr->id] |= XIDeviceDisabled;

                    RemoveDevice(XTestptr, FALSE);
                    RemoveDevice(XTestkeybd, FALSE);
                    RemoveDevice(keybd, FALSE);
                    RemoveDevice(ptr, FALSE);
                    flags[XTestptr->id] |= XISlaveRemoved;
                    flags[XTestkeybd->id] |= XISlaveRemoved;
                    flags[keybd->id] |= XIMasterRemoved;
                    flags[ptr->id] |= XIMasterRemoved;
                }
                break;
            case XIDetachSlave:
                {
                    xXIDetachSlaveInfo* c = (xXIDetachSlaveInfo*)any;

                    rc = dixLookupDevice(&ptr, c->deviceid, client,
                                          DixManageAccess);
                    if (rc != Success)
                       goto unwind;

                    if (IsMaster(ptr))
                    {
                        client->errorValue = c->deviceid;
                        rc = BadDevice;
                        goto unwind;
                    }

                    /* Don't allow changes to XTest Devices, these are fixed */
                    if (IsXTestDevice(ptr, NULL))
                    {
                        client->errorValue = c->deviceid;
                        rc = BadDevice;
                        goto unwind;
                    }

                    AttachDevice(client, ptr, NULL);
                    flags[ptr->id] |= XISlaveDetached;
                }
                break;
            case XIAttachSlave:
                {
                    xXIAttachSlaveInfo* c = (xXIAttachSlaveInfo*)any;
                    DeviceIntPtr newmaster;

                    rc = dixLookupDevice(&ptr, c->deviceid, client,
                                          DixManageAccess);
                    if (rc != Success)
                       goto unwind;

                    if (IsMaster(ptr))
                    {
                        client->errorValue = c->deviceid;
                        rc = BadDevice;
                        goto unwind;
                    }

                    /* Don't allow changes to XTest Devices, these are fixed */
                    if (IsXTestDevice(ptr, NULL))
                    {
                        client->errorValue = c->deviceid;
                        rc = BadDevice;
                        goto unwind;
                    }

                    rc = dixLookupDevice(&newmaster, c->new_master,
                            client, DixAddAccess);
                    if (rc != Success)
                        goto unwind;
                    if (!IsMaster(newmaster))
                    {
                        client->errorValue = c->new_master;
                        rc = BadDevice;
                        goto unwind;
                    }

                    if (!((IsPointerDevice(newmaster) &&
                                    IsPointerDevice(ptr)) ||
                                (IsKeyboardDevice(newmaster) &&
                                 IsKeyboardDevice(ptr))))
                    {
                        rc = BadDevice;
                        goto unwind;
                    }
                    AttachDevice(client, ptr, newmaster);
                    flags[ptr->id] |= XISlaveAttached;
                }
                break;
        }

        any = (xXIAnyHierarchyChangeInfo*)((char*)any + any->length * 4);
    }

unwind:

    XISendDeviceHierarchyEvent(flags);
    return rc;
}

