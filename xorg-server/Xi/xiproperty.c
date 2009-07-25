/*
 * Copyright © 2006 Keith Packard
 * Copyright © 2008 Peter Hutterer
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WAXIANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WAXIANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/* This code is a modified version of randr/rrproperty.c */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "dix.h"
#include "inputstr.h"
#include <X11/extensions/XI.h>
#include <X11/Xatom.h>
#include <X11/extensions/XIproto.h>
#include "exglobals.h"
#include "exevents.h"
#include "swaprep.h"

#include "xiproperty.h"
#include "xserver-properties.h"

/**
 * Properties used or alloced from inside the server.
 */
static struct dev_properties
{
    Atom type;
    char *name;
} dev_properties[] = {
    {0, XI_PROP_ENABLED},
    {0, XATOM_FLOAT}
};

static long XIPropHandlerID = 1;

/**
 * Return the type assigned to the specified atom or 0 if the atom isn't known
 * to the DIX.
 *
 * If name is NULL, None is returned.
 */
_X_EXPORT Atom
XIGetKnownProperty(char *name)
{
    int i;

    if (!name)
        return None;

    for (i = 0; i < (sizeof(dev_properties)/sizeof(struct dev_properties)); i++)
    {
        if (strcmp(name, dev_properties[i].name) == 0)
            return dev_properties[i].type;
    }

    return 0;
}

/**
 * Convert the given property's value(s) into @nelem_return integer values and
 * store them in @buf_return. If @nelem_return is larger than the number of
 * values in the property, @nelem_return is set to the number of values in the
 * property.
 *
 * If *@buf_return is NULL and @nelem_return is 0, memory is allocated
 * automatically and must be freed by the caller.
 *
 * Possible return codes.
 * Success ... No error.
 * BadMatch ... Wrong atom type, atom is not XA_INTEGER
 * BadAlloc ... NULL passed as buffer and allocation failed.
 * BadLength ... @buff is NULL but @nelem_return is non-zero.
 *
 * @param val The property value
 * @param nelem_return The maximum number of elements to return.
 * @param buf_return Pointer to an array of at least @nelem_return values.
 * @return Success or the error code if an error occured.
 */
_X_EXPORT int
XIPropToInt(XIPropertyValuePtr val, int *nelem_return, int **buf_return)
{
    int i;
    int *buf;

    if (val->type != XA_INTEGER)
        return BadMatch;
    if (!*buf_return && *nelem_return)
        return BadLength;

    switch(val->format)
    {
        case 8:
        case 16:
        case 32:
            break;
        default:
            return BadValue;
    }

    buf = *buf_return;

    if (!buf && !(*nelem_return))
    {
        buf = xcalloc(val->size, sizeof(int));
        if (!buf)
            return BadAlloc;
        *buf_return = buf;
        *nelem_return = val->size;
    } else if (val->size < *nelem_return)
        *nelem_return = val->size;

    for (i = 0; i < val->size && i < *nelem_return; i++)
    {
        switch(val->format)
        {
            case 8:  buf[i] = ((CARD8*)val->data)[i]; break;
            case 16: buf[i] = ((CARD16*)val->data)[i]; break;
            case 32: buf[i] = ((CARD32*)val->data)[i]; break;
        }
    }

    return Success;
}

/**
 * Convert the given property's value(s) into @nelem_return float values and
 * store them in @buf_return. If @nelem_return is larger than the number of
 * values in the property, @nelem_return is set to the number of values in the
 * property.
 *
 * If *@buf_return is NULL and @nelem_return is 0, memory is allocated
 * automatically and must be freed by the caller.
 *
 * Possible errors returned:
 * Success
 * BadMatch ... Wrong atom type, atom is not XA_FLOAT
 * BadValue ... Wrong format, format is not 32
 * BadAlloc ... NULL passed as buffer and allocation failed.
 * BadLength ... @buff is NULL but @nelem_return is non-zero.
 *
 * @param val The property value
 * @param nelem_return The maximum number of elements to return.
 * @param buf_return Pointer to an array of at least @nelem_return values.
 * @return Success or the error code if an error occured.
 */
_X_EXPORT int
XIPropToFloat(XIPropertyValuePtr val, int *nelem_return, float **buf_return)
{
    int i;
    float *buf;

    if (!val->type || val->type != XIGetKnownProperty(XATOM_FLOAT))
        return BadMatch;

    if (val->format != 32)
        return BadValue;
    if (!*buf_return && *nelem_return)
        return BadLength;

    buf = *buf_return;

    if (!buf && !(*nelem_return))
    {
        buf = xcalloc(val->size, sizeof(float));
        if (!buf)
            return BadAlloc;
        *buf_return = buf;
        *nelem_return = val->size;
    } else if (val->size < *nelem_return)
        *nelem_return = val->size;

    for (i = 0; i < val->size && i < *nelem_return; i++)
           buf[i] = ((float*)val->data)[i];

    return Success;
}

/**
 * Init those properties that are allocated by the server and most likely used
 * by the DIX or the DDX.
 */
void
XIInitKnownProperties(void)
{
    int i;
    for (i = 0; i < (sizeof(dev_properties)/sizeof(struct dev_properties)); i++)
    {
        dev_properties[i].type =
            MakeAtom(dev_properties[i].name,
                     strlen(dev_properties[i].name),
                     TRUE);
    }
}


/* Registers a new property handler on the given device and returns a unique
 * identifier for this handler. This identifier is required to unregister the
 * property handler again.
 * @return The handler's identifier or 0 if an error occured.
 */
long
XIRegisterPropertyHandler(DeviceIntPtr         dev,
                          int (*SetProperty) (DeviceIntPtr dev,
                                              Atom property,
                                              XIPropertyValuePtr prop,
                                              BOOL checkonly),
                          int (*GetProperty) (DeviceIntPtr dev,
                                              Atom property),
                          int (*DeleteProperty) (DeviceIntPtr dev,
                                                 Atom property))
{
    XIPropertyHandlerPtr new_handler;

    new_handler = xcalloc(1, sizeof(XIPropertyHandler));
    if (!new_handler)
        return 0;

    new_handler->id = XIPropHandlerID++;
    new_handler->SetProperty = SetProperty;
    new_handler->GetProperty = GetProperty;
    new_handler->DeleteProperty = DeleteProperty;
    new_handler->next = dev->properties.handlers;
    dev->properties.handlers = new_handler;

    return new_handler->id;
}

void
XIUnregisterPropertyHandler(DeviceIntPtr dev, long id)
{
    XIPropertyHandlerPtr curr, prev = NULL;

    curr = dev->properties.handlers;
    while(curr && curr->id != id)
    {
        prev = curr;
        curr = curr->next;
    }

    if (!curr)
        return;

    if (!prev) /* first one */
        dev->properties.handlers = curr->next;
    else
        prev->next = curr->next;

    xfree(curr);
}

static XIPropertyPtr
XICreateDeviceProperty (Atom property)
{
    XIPropertyPtr   prop;

    prop = (XIPropertyPtr)xalloc(sizeof(XIPropertyRec));
    if (!prop)
        return NULL;

    prop->next          = NULL;
    prop->propertyName  = property;
    prop->value.type   = None;
    prop->value.format = 0;
    prop->value.size   = 0;
    prop->value.data   = NULL;
    prop->deletable    = TRUE;

    return prop;
}

static XIPropertyPtr
XIFetchDeviceProperty(DeviceIntPtr dev, Atom property)
{
    XIPropertyPtr   prop;

    for (prop = dev->properties.properties; prop; prop = prop->next)
        if (prop->propertyName == property)
            return prop;
    return NULL;
}

static void
XIDestroyDeviceProperty (XIPropertyPtr prop)
{
    if (prop->value.data)
        xfree(prop->value.data);
    xfree(prop);
}

/* This function destroys all of the device's property-related stuff,
 * including removing all device handlers.
 * DO NOT CALL FROM THE DRIVER.
 */
void
XIDeleteAllDeviceProperties (DeviceIntPtr device)
{
    XIPropertyPtr               prop, next;
    XIPropertyHandlerPtr        curr_handler, next_handler;
    devicePropertyNotify        event;

    for (prop = device->properties.properties; prop; prop = next)
    {
        next = prop->next;

        event.type      = DevicePropertyNotify;
        event.deviceid  = device->id;
        event.state     = PropertyDelete;
        event.atom      = prop->propertyName;
        event.time      = currentTime.milliseconds;
        SendEventToAllWindows(device, DevicePropertyNotifyMask,
                (xEvent*)&event, 1);

        XIDestroyDeviceProperty(prop);
    }

    /* Now free all handlers */
    curr_handler = device->properties.handlers;
    while(curr_handler)
    {
        next_handler = curr_handler->next;
        xfree(curr_handler);
        curr_handler = next_handler;
    }
}


int
XIDeleteDeviceProperty (DeviceIntPtr device, Atom property, Bool fromClient)
{
    XIPropertyPtr               prop, *prev;
    devicePropertyNotify        event;
    int                         rc = Success;

    for (prev = &device->properties.properties; (prop = *prev); prev = &(prop->next))
        if (prop->propertyName == property)
            break;

    if (fromClient && !prop->deletable)
        return BadAccess;

    /* Ask handlers if we may delete the property */
    if (device->properties.handlers)
    {
        XIPropertyHandlerPtr handler = device->properties.handlers;
        while(handler)
        {
            if (handler->DeleteProperty)
                rc = handler->DeleteProperty(device, prop->propertyName);
            if (rc != Success)
                return (rc);
            handler = handler->next;
        }
    }

    if (prop)
    {
        *prev = prop->next;
        event.type      = DevicePropertyNotify;
        event.deviceid  = device->id;
        event.state     = PropertyDelete;
        event.atom      = prop->propertyName;
        event.time      = currentTime.milliseconds;
        SendEventToAllWindows(device, DevicePropertyNotifyMask,
                              (xEvent*)&event, 1);
        XIDestroyDeviceProperty (prop);
    }

    return Success;
}

int
XIChangeDeviceProperty (DeviceIntPtr dev, Atom property, Atom type,
                        int format, int mode, unsigned long len,
                        pointer value, Bool sendevent)
{
    XIPropertyPtr               prop;
    devicePropertyNotify        event;
    int                         size_in_bytes;
    int                         total_size;
    unsigned long               total_len;
    XIPropertyValuePtr          prop_value;
    XIPropertyValueRec          new_value;
    Bool                        add = FALSE;
    int                         rc;

    size_in_bytes = format >> 3;

    /* first see if property already exists */
    prop = XIFetchDeviceProperty (dev, property);
    if (!prop)   /* just add to list */
    {
        prop = XICreateDeviceProperty (property);
        if (!prop)
            return(BadAlloc);
        add = TRUE;
        mode = PropModeReplace;
    }
    prop_value = &prop->value;

    /* To append or prepend to a property the request format and type
     must match those of the already defined property.  The
     existing format and type are irrelevant when using the mode
     "PropModeReplace" since they will be written over. */

    if ((format != prop_value->format) && (mode != PropModeReplace))
        return(BadMatch);
    if ((prop_value->type != type) && (mode != PropModeReplace))
        return(BadMatch);
    new_value = *prop_value;
    if (mode == PropModeReplace)
        total_len = len;
    else
        total_len = prop_value->size + len;

    if (mode == PropModeReplace || len > 0)
    {
        pointer            new_data = NULL, old_data = NULL;

        total_size = total_len * size_in_bytes;
        new_value.data = (pointer)xalloc (total_size);
        if (!new_value.data && total_size)
        {
            if (add)
                XIDestroyDeviceProperty (prop);
            return BadAlloc;
        }
        new_value.size = len;
        new_value.type = type;
        new_value.format = format;

        switch (mode) {
        case PropModeReplace:
            new_data = new_value.data;
            old_data = NULL;
            break;
        case PropModeAppend:
            new_data = (pointer) (((char *) new_value.data) +
                                  (prop_value->size * size_in_bytes));
            old_data = new_value.data;
            break;
        case PropModePrepend:
            new_data = new_value.data;
            old_data = (pointer) (((char *) new_value.data) +
                                  (prop_value->size * size_in_bytes));
            break;
        }
        if (new_data)
            memcpy ((char *) new_data, (char *) value, len * size_in_bytes);
        if (old_data)
            memcpy ((char *) old_data, (char *) prop_value->data,
                    prop_value->size * size_in_bytes);

        if (dev->properties.handlers)
        {
            XIPropertyHandlerPtr handler;
            BOOL checkonly = TRUE;
            /* run through all handlers with checkonly TRUE, then again with
             * checkonly FALSE. Handlers MUST return error codes on the
             * checkonly run, errors on the second run are ignored */
            do
            {
                handler = dev->properties.handlers;
                while(handler)
                {
                    if (handler->SetProperty)
                    {
                        rc = handler->SetProperty(dev, prop->propertyName,
                                &new_value, checkonly);
                        if (checkonly && rc != Success)
                        {
                            if (new_value.data)
                                xfree (new_value.data);
                            return (rc);
                        }
                    }
                    handler = handler->next;
                }
                checkonly = !checkonly;
            } while (!checkonly);
        }
        if (prop_value->data)
            xfree (prop_value->data);
        *prop_value = new_value;
    } else if (len == 0)
    {
        /* do nothing */
    }

    if (add)
    {
        prop->next = dev->properties.properties;
        dev->properties.properties = prop;
    }

    if (sendevent)
    {
        event.type      = DevicePropertyNotify;
        event.deviceid  = dev->id;
        event.state     = PropertyNewValue;
        event.atom      = prop->propertyName;
        event.time      = currentTime.milliseconds;
        SendEventToAllWindows(dev, DevicePropertyNotifyMask,
                              (xEvent*)&event, 1);
    }
    return(Success);
}

int
XIGetDeviceProperty (DeviceIntPtr dev, Atom property, XIPropertyValuePtr *value)
{
    XIPropertyPtr   prop = XIFetchDeviceProperty (dev, property);
    int rc;

    if (!prop)
    {
        *value = NULL;
        return BadAtom;
    }

    /* If we can, try to update the property value first */
    if (dev->properties.handlers)
    {
        XIPropertyHandlerPtr handler = dev->properties.handlers;
        while(handler)
        {
            if (handler->GetProperty)
            {
                rc = handler->GetProperty(dev, prop->propertyName);
                if (rc != Success)
                {
                    *value = NULL;
                    return rc;
                }
            }
            handler = handler->next;
        }
    }

    *value = &prop->value;
    return Success;
}

int
XISetDevicePropertyDeletable(DeviceIntPtr dev, Atom property, Bool deletable)
{
    XIPropertyPtr prop = XIFetchDeviceProperty(dev, property);

    if (!prop)
        return BadAtom;

    prop->deletable = deletable;
    return Success;
}

int
ProcXListDeviceProperties (ClientPtr client)
{
    Atom                        *pAtoms = NULL, *temppAtoms;
    xListDevicePropertiesReply  rep;
    int                         numProps = 0;
    DeviceIntPtr                dev;
    XIPropertyPtr               prop;
    int                         rc = Success;

    REQUEST(xListDevicePropertiesReq);
    REQUEST_SIZE_MATCH(xListDevicePropertiesReq);

    rc = dixLookupDevice (&dev, stuff->deviceid, client, DixReadAccess);
    if (rc != Success)
        return rc;

    for (prop = dev->properties.properties; prop; prop = prop->next)
        numProps++;
    if (numProps)
        if(!(pAtoms = (Atom *)xalloc(numProps * sizeof(Atom))))
            return(BadAlloc);

    rep.repType = X_Reply;
    rep.RepType = X_ListDeviceProperties;
    rep.length = (numProps * sizeof(Atom)) >> 2;
    rep.sequenceNumber = client->sequence;
    rep.nAtoms = numProps;
    temppAtoms = pAtoms;
    for (prop = dev->properties.properties; prop; prop = prop->next)
        *temppAtoms++ = prop->propertyName;

    WriteReplyToClient(client, sizeof(xListDevicePropertiesReply), &rep);
    if (numProps)
    {
        client->pSwapReplyFunc = (ReplySwapPtr)Swap32Write;
        WriteSwappedDataToClient(client, numProps * sizeof(Atom), pAtoms);
        xfree(pAtoms);
    }
    return rc;
}

int
ProcXChangeDeviceProperty (ClientPtr client)
{
    REQUEST(xChangeDevicePropertyReq);
    DeviceIntPtr        dev;
    char                format, mode;
    unsigned long       len;
    int                 sizeInBytes;
    int                 totalSize;
    int                 rc;

    REQUEST_AT_LEAST_SIZE(xChangeDevicePropertyReq);
    UpdateCurrentTime();
    format = stuff->format;
    mode = stuff->mode;
    if ((mode != PropModeReplace) && (mode != PropModeAppend) &&
        (mode != PropModePrepend))
    {
        client->errorValue = mode;
        return BadValue;
    }
    if ((format != 8) && (format != 16) && (format != 32))
    {
        client->errorValue = format;
        return BadValue;
    }
    len = stuff->nUnits;
    if (len > ((0xffffffff - sizeof(xChangeDevicePropertyReq)) >> 2))
        return BadLength;
    sizeInBytes = format>>3;
    totalSize = len * sizeInBytes;
    REQUEST_FIXED_SIZE(xChangeDevicePropertyReq, totalSize);

    rc = dixLookupDevice (&dev, stuff->deviceid, client, DixWriteAccess);
    if (rc != Success)
        return rc;

    if (!ValidAtom(stuff->property))
    {
        client->errorValue = stuff->property;
        return(BadAtom);
    }
    if (!ValidAtom(stuff->type))
    {
        client->errorValue = stuff->type;
        return(BadAtom);
    }

    rc = XIChangeDeviceProperty(dev, stuff->property,
                                 stuff->type, (int)format,
                                 (int)mode, len, (pointer)&stuff[1], TRUE);

    if (rc != Success)
        client->errorValue = stuff->property;
    return rc;
}

int
ProcXDeleteDeviceProperty (ClientPtr client)
{
    REQUEST(xDeleteDevicePropertyReq);
    DeviceIntPtr        dev;
    int                 rc;

    REQUEST_SIZE_MATCH(xDeleteDevicePropertyReq);
    UpdateCurrentTime();
    rc =  dixLookupDevice (&dev, stuff->deviceid, client, DixWriteAccess);
    if (rc != Success)
        return rc;

    if (!ValidAtom(stuff->property))
    {
        client->errorValue = stuff->property;
        return (BadAtom);
    }

    rc = XIDeleteDeviceProperty(dev, stuff->property, TRUE);
    return rc;
}

int
ProcXGetDeviceProperty (ClientPtr client)
{
    REQUEST(xGetDevicePropertyReq);
    XIPropertyPtr               prop, *prev;
    XIPropertyValuePtr          prop_value;
    unsigned long               n, len, ind;
    DeviceIntPtr                dev;
    xGetDevicePropertyReply     reply;
    int                         rc;

    REQUEST_SIZE_MATCH(xGetDevicePropertyReq);
    if (stuff->delete)
        UpdateCurrentTime();
    rc = dixLookupDevice (&dev, stuff->deviceid, client,
                           stuff->delete ? DixWriteAccess :
                           DixReadAccess);
    if (rc != Success)
        return rc;

    if (!ValidAtom(stuff->property))
    {
        client->errorValue = stuff->property;
        return(BadAtom);
    }
    if ((stuff->delete != xTrue) && (stuff->delete != xFalse))
    {
        client->errorValue = stuff->delete;
        return(BadValue);
    }
    if ((stuff->type != AnyPropertyType) && !ValidAtom(stuff->type))
    {
        client->errorValue = stuff->type;
        return(BadAtom);
    }

    for (prev = &dev->properties.properties; (prop = *prev); prev = &prop->next)
        if (prop->propertyName == stuff->property)
            break;

    reply.repType = X_Reply;
    reply.RepType = X_GetDeviceProperty;
    reply.sequenceNumber = client->sequence;
    reply.deviceid = dev->id;
    if (!prop)
    {
        reply.nItems = 0;
        reply.length = 0;
        reply.bytesAfter = 0;
        reply.propertyType = None;
        reply.format = 0;
        WriteReplyToClient(client, sizeof(xGetDevicePropertyReply), &reply);
        return(client->noClientException);
    }

    rc = XIGetDeviceProperty(dev, stuff->property, &prop_value);
    if (rc != Success)
    {
        client->errorValue = stuff->property;
        return rc;
    }

    /* If the request type and actual type don't match. Return the
    property information, but not the data. */

    if (((stuff->type != prop_value->type) &&
         (stuff->type != AnyPropertyType))
       )
    {
        reply.bytesAfter = prop_value->size;
        reply.format = prop_value->format;
        reply.length = 0;
        reply.nItems = 0;
        reply.propertyType = prop_value->type;
        WriteReplyToClient(client, sizeof(xGetDevicePropertyReply), &reply);
        return(client->noClientException);
    }

/*
 *  Return type, format, value to client
 */
    n = (prop_value->format/8) * prop_value->size; /* size (bytes) of prop */
    ind = stuff->longOffset << 2;

   /* If longOffset is invalid such that it causes "len" to
            be negative, it's a value error. */

    if (n < ind)
    {
        client->errorValue = stuff->longOffset;
        return BadValue;
    }

    len = min(n - ind, 4 * stuff->longLength);

    reply.bytesAfter = n - (ind + len);
    reply.format = prop_value->format;
    reply.length = (len + 3) >> 2;
    if (prop_value->format)
        reply.nItems = len / (prop_value->format / 8);
    else
        reply.nItems = 0;
    reply.propertyType = prop_value->type;

    if (stuff->delete && (reply.bytesAfter == 0))
    {
        devicePropertyNotify    event;

        event.type      = DevicePropertyNotify;
        event.deviceid  = dev->id;
        event.state     = PropertyDelete;
        event.atom      = prop->propertyName;
        event.time      = currentTime.milliseconds;
        SendEventToAllWindows(dev, DevicePropertyNotifyMask,
                              (xEvent*)&event, 1);
    }

    WriteReplyToClient(client, sizeof(xGenericReply), &reply);
    if (len)
    {
        switch (reply.format) {
        case 32: client->pSwapReplyFunc = (ReplySwapPtr)CopySwap32Write; break;
        case 16: client->pSwapReplyFunc = (ReplySwapPtr)CopySwap16Write; break;
        default: client->pSwapReplyFunc = (ReplySwapPtr)WriteToClient; break;
        }
        WriteSwappedDataToClient(client, len,
                                 (char *)prop_value->data + ind);
    }

    if (stuff->delete && (reply.bytesAfter == 0))
    { /* delete the Property */
        *prev = prop->next;
        XIDestroyDeviceProperty (prop);
    }
    return(client->noClientException);
}


int
SProcXListDeviceProperties (ClientPtr client)
{
    char n;
    REQUEST(xListDevicePropertiesReq);

    swaps(&stuff->length, n);

    REQUEST_SIZE_MATCH(xListDevicePropertiesReq);
    return (ProcXListDeviceProperties(client));
}

int
SProcXChangeDeviceProperty (ClientPtr client)
{
    char n;
    REQUEST(xChangeDevicePropertyReq);

    swaps(&stuff->length, n);
    swapl(&stuff->property, n);
    swapl(&stuff->type, n);
    swapl(&stuff->nUnits, n);
    REQUEST_SIZE_MATCH(xChangeDevicePropertyReq);
    return (ProcXChangeDeviceProperty(client));
}

int
SProcXDeleteDeviceProperty (ClientPtr client)
{
    char n;
    REQUEST(xDeleteDevicePropertyReq);

    swaps(&stuff->length, n);
    swapl(&stuff->property, n);
    REQUEST_SIZE_MATCH(xDeleteDevicePropertyReq);
    return (ProcXDeleteDeviceProperty(client));
}

int
SProcXGetDeviceProperty (ClientPtr client)
{
    char n;
    REQUEST(xGetDevicePropertyReq);

    swaps(&stuff->length, n);
    swapl(&stuff->property, n);
    swapl(&stuff->type, n);
    swapl(&stuff->longOffset, n);
    swapl(&stuff->longLength, n);
    REQUEST_SIZE_MATCH(xGetDevicePropertyReq);
    return (ProcXGetDeviceProperty(client));
}


/* Reply swapping */

void
SRepXListDeviceProperties(ClientPtr client, int size,
                          xListDevicePropertiesReply *rep)
{
    char n;
    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    swaps(&rep->nAtoms, n);
    /* properties will be swapped later, see ProcXListDeviceProperties */
    WriteToClient(client, size, (char*)rep);
}

void
SRepXGetDeviceProperty(ClientPtr client, int size,
                       xGetDevicePropertyReply *rep)
{
    char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    swapl(&rep->propertyType, n);
    swapl(&rep->bytesAfter, n);
    swapl(&rep->nItems, n);
    /* data will be swapped, see ProcXGetDeviceProperty */
    WriteToClient(client, size, (char*)rep);
}
