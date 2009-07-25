/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/***********************************************************************
 *
 * Extension function to list the available input devices.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>	/* for inputstr.h    */
#include <X11/Xproto.h>	/* Request macro     */
#include "inputstr.h"	/* DeviceIntPtr      */
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "XIstubs.h"
#include "extnsionst.h"
#include "exglobals.h"	/* FIXME */
#include "exevents.h"
#include "xace.h"

#include "listdev.h"


/***********************************************************************
 *
 * This procedure lists the input devices available to the server.
 *
 */

int
SProcXListInputDevices(ClientPtr client)
{
    char n;

    REQUEST(xListInputDevicesReq);
    swaps(&stuff->length, n);
    return (ProcXListInputDevices(client));
}

/***********************************************************************
 *
 * This procedure calculates the size of the information to be returned
 * for an input device.
 *
 */

void
SizeDeviceInfo(DeviceIntPtr d, int *namesize, int *size)
{
    int chunks;

    *namesize += 1;
    if (d->name)
	*namesize += strlen(d->name);
    if (d->key != NULL)
	*size += sizeof(xKeyInfo);
    if (d->button != NULL)
	*size += sizeof(xButtonInfo);
    if (d->valuator != NULL) {
	chunks = ((int)d->valuator->numAxes + 19) / VPC;
	*size += (chunks * sizeof(xValuatorInfo) +
		  d->valuator->numAxes * sizeof(xAxisInfo));
    }
}

/***********************************************************************
 *
 * This procedure copies data to the DeviceInfo struct, swapping if necessary.
 *
 * We need the extra byte in the allocated buffer, because the trailing null
 * hammers one extra byte, which is overwritten by the next name except for
 * the last name copied.
 *
 */

static void
CopyDeviceName(char **namebuf, char *name)
{
    char *nameptr = (char *)*namebuf;

    if (name) {
	*nameptr++ = strlen(name);
	strcpy(nameptr, name);
	*namebuf += (strlen(name) + 1);
    } else {
	*nameptr++ = 0;
	*namebuf += 1;
    }
}

/***********************************************************************
 *
 * This procedure copies ButtonClass information, swapping if necessary.
 *
 */

static void
CopySwapButtonClass(ClientPtr client, ButtonClassPtr b, char **buf)
{
    char n;
    xButtonInfoPtr b2;

    b2 = (xButtonInfoPtr) * buf;
    b2->class = ButtonClass;
    b2->length = sizeof(xButtonInfo);
    b2->num_buttons = b->numButtons;
    if (client && client->swapped) {
	swaps(&b2->num_buttons, n);	/* macro - braces are required */
    }
    *buf += sizeof(xButtonInfo);
}

/***********************************************************************
 *
 * This procedure copies data to the DeviceInfo struct, swapping if necessary.
 *
 */

static void
CopySwapDevice(ClientPtr client, DeviceIntPtr d, int num_classes,
	       char **buf)
{
    char n;
    xDeviceInfoPtr dev;

    dev = (xDeviceInfoPtr) * buf;

    dev->id = d->id;
    dev->type = d->type;
    dev->num_classes = num_classes;
    if (d->isMaster && IsKeyboardDevice(d))
	dev->use = IsXKeyboard;
    else if (d->isMaster && IsPointerDevice(d))
	dev->use = IsXPointer;
    else if (d->key && d->kbdfeed)
        dev->use = IsXExtensionKeyboard;
    else if (d->valuator && d->button)
        dev->use = IsXExtensionPointer;
    else
	dev->use = IsXExtensionDevice;

    if (client->swapped) {
	swapl(&dev->type, n);	/* macro - braces are required */
    }
    *buf += sizeof(xDeviceInfo);
}

/***********************************************************************
 *
 * This procedure copies KeyClass information, swapping if necessary.
 *
 */

static void
CopySwapKeyClass(ClientPtr client, KeyClassPtr k, char **buf)
{
    char n;
    xKeyInfoPtr k2;

    k2 = (xKeyInfoPtr) * buf;
    k2->class = KeyClass;
    k2->length = sizeof(xKeyInfo);
    k2->min_keycode = k->curKeySyms.minKeyCode;
    k2->max_keycode = k->curKeySyms.maxKeyCode;
    k2->num_keys = k2->max_keycode - k2->min_keycode + 1;
    if (client && client->swapped) {
	swaps(&k2->num_keys, n);
    }
    *buf += sizeof(xKeyInfo);
}

/***********************************************************************
 *
 * This procedure copies ValuatorClass information, swapping if necessary.
 *
 * Devices may have up to 255 valuators.  The length of a ValuatorClass is
 * defined to be sizeof(ValuatorClassInfo) + num_axes * sizeof (xAxisInfo).
 * The maximum length is therefore (8 + 255 * 12) = 3068.  However, the
 * length field is one byte.  If a device has more than 20 valuators, we
 * must therefore return multiple valuator classes to the client.
 *
 */

static int
CopySwapValuatorClass(ClientPtr client, ValuatorClassPtr v, char **buf)
{
    int i, j, axes, t_axes;
    char n;
    xValuatorInfoPtr v2;
    AxisInfo *a;
    xAxisInfoPtr a2;

    for (i = 0, axes = v->numAxes; i < ((v->numAxes + 19) / VPC);
	 i++, axes -= VPC) {
	t_axes = axes < VPC ? axes : VPC;
	if (t_axes < 0)
	    t_axes = v->numAxes % VPC;
	v2 = (xValuatorInfoPtr) * buf;
	v2->class = ValuatorClass;
	v2->length = sizeof(xValuatorInfo) + t_axes * sizeof(xAxisInfo);
	v2->num_axes = t_axes;
	v2->mode = v->mode & DeviceMode;
	v2->motion_buffer_size = v->numMotionEvents;
	if (client && client->swapped) {
	    swapl(&v2->motion_buffer_size, n);
	}
	*buf += sizeof(xValuatorInfo);
	a = v->axes + (VPC * i);
	a2 = (xAxisInfoPtr) * buf;
	for (j = 0; j < t_axes; j++) {
	    a2->min_value = a->min_value;
	    a2->max_value = a->max_value;
	    a2->resolution = a->resolution;
	    if (client && client->swapped) {
		swapl(&a2->min_value, n);
		swapl(&a2->max_value, n);
		swapl(&a2->resolution, n);
	    }
	    a2++;
	    a++;
	    *buf += sizeof(xAxisInfo);
	}
    }
    return (i);
}

/***********************************************************************
 *
 * This procedure lists information to be returned for an input device.
 *
 */

static void
ListDeviceInfo(ClientPtr client, DeviceIntPtr d, xDeviceInfoPtr dev,
	       char **devbuf, char **classbuf, char **namebuf)
{
    CopyDeviceName(namebuf, d->name);
    CopySwapDevice(client, d, 0, devbuf);
    CopySwapClasses(client, d, &dev->num_classes, classbuf);
}

void
CopySwapClasses(ClientPtr client, DeviceIntPtr dev, CARD8 *num_classes,
                char** classbuf)
{
    if (dev->key != NULL) {
	CopySwapKeyClass(client, dev->key, classbuf);
	(*num_classes)++;
    }
    if (dev->button != NULL) {
	CopySwapButtonClass(client, dev->button, classbuf);
	(*num_classes)++;
    }
    if (dev->valuator != NULL) {
	(*num_classes) +=
	    CopySwapValuatorClass(client, dev->valuator, classbuf);
    }
}

/***********************************************************************
 *
 * This procedure lists the input devices available to the server.
 *
 * If this request is called by a client that has not issued a
 * GetExtensionVersion request with major/minor version set, we don't send the
 * complete device list. Instead, we only send the VCP, the VCK and floating
 * SDs. This resembles the setup found on XI 1.x machines.
 */

int
ProcXListInputDevices(ClientPtr client)
{
    xListInputDevicesReply rep;
    XIClientPtr pXIClient;
    int numdevs = 0;
    int namesize = 1;	/* need 1 extra byte for strcpy */
    int rc, size = 0;
    int total_length;
    char *devbuf;
    char *classbuf;
    char *namebuf;
    char *savbuf;
    xDeviceInfo *dev;
    DeviceIntPtr d;

    REQUEST_SIZE_MATCH(xListInputDevicesReq);

    rep.repType = X_Reply;
    rep.RepType = X_ListInputDevices;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    pXIClient = dixLookupPrivate(&client->devPrivates, XIClientPrivateKey);

    AddOtherInputDevices();

    for (d = inputInfo.devices; d; d = d->next) {
        rc = XaceHook(XACE_DEVICE_ACCESS, client, d, DixGetAttrAccess);
        if (rc != Success)
            return rc;
        SizeDeviceInfo(d, &namesize, &size);
        numdevs++;
    }

    for (d = inputInfo.off_devices; d; d = d->next) {
        rc = XaceHook(XACE_DEVICE_ACCESS, client, d, DixGetAttrAccess);
        if (rc != Success)
            return rc;
        SizeDeviceInfo(d, &namesize, &size);
        numdevs++;
    }

    total_length = numdevs * sizeof(xDeviceInfo) + size + namesize;
    devbuf = (char *)xalloc(total_length);
    classbuf = devbuf + (numdevs * sizeof(xDeviceInfo));
    namebuf = classbuf + size;
    savbuf = devbuf;

    dev = (xDeviceInfoPtr) devbuf;
    for (d = inputInfo.devices; d; d = d->next)
    {
        ListDeviceInfo(client, d, dev++, &devbuf, &classbuf, &namebuf);
    }

    for (d = inputInfo.off_devices; d; d = d->next)
    {
        ListDeviceInfo(client, d, dev++, &devbuf, &classbuf, &namebuf);
    }
    rep.ndevices = numdevs;
    rep.length = (total_length + 3) >> 2;
    WriteReplyToClient(client, sizeof(xListInputDevicesReply), &rep);
    WriteToClient(client, total_length, savbuf);
    xfree(savbuf);
    return Success;
}

/***********************************************************************
 *
 * This procedure writes the reply for the XListInputDevices function,
 * if the client and server have a different byte ordering.
 *
 */

void
SRepXListInputDevices(ClientPtr client, int size, xListInputDevicesReply * rep)
{
    char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, (char *)rep);
}
