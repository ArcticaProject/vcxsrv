/*

   Copyright 1992, 1998  The Open Group

   Permission to use, copy, modify, distribute, and sell this software and its
   documentation for any purpose is hereby granted without fee, provided that
   the above copyright notice appear in all copies and that both that
   copyright notice and this permission notice appear in supporting
   documentation.

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name of The Open Group shall
   not be used in advertising or otherwise to promote the sale, use or
   other dealings in this Software without prior written authorization
   from The Open Group.

 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#define NEED_EVENTS
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "dixevents.h"
#include "sleepuntil.h"
#include "mi.h"
#define _XTEST_SERVER_
#include <X11/extensions/XTest.h>
#include <X11/extensions/xteststr.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>

#include "modinit.h"

extern int DeviceValuator;
extern int DeviceMotionNotify;

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

static int XTestSwapFakeInput(
        ClientPtr /* client */,
        xReq * /* req */
        );

static DISPATCH_PROC(ProcXTestCompareCursor);
static DISPATCH_PROC(ProcXTestDispatch);
static DISPATCH_PROC(ProcXTestFakeInput);
static DISPATCH_PROC(ProcXTestGetVersion);
static DISPATCH_PROC(ProcXTestGrabControl);
static DISPATCH_PROC(SProcXTestCompareCursor);
static DISPATCH_PROC(SProcXTestDispatch);
static DISPATCH_PROC(SProcXTestFakeInput);
static DISPATCH_PROC(SProcXTestGetVersion);
static DISPATCH_PROC(SProcXTestGrabControl);

void
XTestExtensionInit(INITARGS)
{
    AddExtension(XTestExtensionName, 0, 0,
            ProcXTestDispatch, SProcXTestDispatch,
            NULL, StandardMinorOpcode);
}

static int
ProcXTestGetVersion(client)
    ClientPtr client;
{
    xXTestGetVersionReply rep;
    int n;

    REQUEST_SIZE_MATCH(xXTestGetVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XTestMajorVersion;
    rep.minorVersion = XTestMinorVersion;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sizeof(xXTestGetVersionReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXTestCompareCursor(client)
    ClientPtr client;
{
    REQUEST(xXTestCompareCursorReq);
    xXTestCompareCursorReply rep;
    WindowPtr pWin;
    CursorPtr pCursor;
    int n, rc;
    DeviceIntPtr ptr = PickPointer(client);

    REQUEST_SIZE_MATCH(xXTestCompareCursorReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixGetAttrAccess);
    if (rc != Success)
        return rc;
    if (stuff->cursor == None)
        pCursor = NullCursor;
    else if (stuff->cursor == XTestCurrentCursor)
        pCursor = GetSpriteCursor(ptr);
    else {
        rc = dixLookupResourceByType((pointer *)&pCursor, stuff->cursor, RT_CURSOR,
				     client, DixReadAccess);
        if (rc != Success)
        {
            client->errorValue = stuff->cursor;
            return (rc == BadValue) ? BadCursor : rc;
        }
    }
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.same = (wCursor(pWin) == pCursor);
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
    }
    WriteToClient(client, sizeof(xXTestCompareCursorReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcXTestFakeInput(client)
    ClientPtr client;
{
    REQUEST(xXTestFakeInputReq);
    int nev, n, type, rc;
    xEvent *ev;
    DeviceIntPtr dev = NULL;
    WindowPtr root;
    Bool extension = FALSE;
    deviceValuator *dv = NULL;
    int valuators[MAX_VALUATORS] = {0};
    int numValuators = 0;
    int firstValuator = 0;
    EventListPtr events;
    int nevents = 0;
    int i;
    int base = 0;
    int flags = 0;

    nev = (stuff->length << 2) - sizeof(xReq);
    if ((nev % sizeof(xEvent)) || !nev)
        return BadLength;
    nev /= sizeof(xEvent);
    UpdateCurrentTime();
    ev = (xEvent *)&((xReq *)stuff)[1];
    type = ev->u.u.type & 0177;

    if (type >= EXTENSION_EVENT_BASE)
    {
        extension = TRUE;

        /* check device */
        rc = dixLookupDevice(&dev, stuff->deviceid & 0177, client,
                DixWriteAccess);
        if (rc != Success)
        {
            client->errorValue = stuff->deviceid & 0177;
            return rc;
        }

        /* check type */
        type -= DeviceValuator;
        switch (type) {
            case XI_DeviceKeyPress:
            case XI_DeviceKeyRelease:
            case XI_DeviceButtonPress:
            case XI_DeviceButtonRelease:
            case XI_DeviceMotionNotify:
            case XI_ProximityIn:
            case XI_ProximityOut:
                break;
            default:
                client->errorValue = ev->u.u.type;
                return BadValue;
        }

        /* check validity */
        if (nev == 1 && type == XI_DeviceMotionNotify)
            return BadLength; /* DevMotion must be followed by DevValuator */

        if (type == XI_DeviceMotionNotify)
        {
            firstValuator = ((deviceValuator *)(ev+1))->first_valuator;
            if (firstValuator > dev->valuator->numAxes)
            {
                client->errorValue = ev->u.u.type;
                return BadValue;
            }

            if (ev->u.u.detail == xFalse)
                flags |= POINTER_ABSOLUTE;
        } else
        {
            firstValuator = 0;
            flags |= POINTER_ABSOLUTE;
        }

        if (nev == 1 && type == XI_DeviceMotionNotify && !dev->valuator)
        {
            client->errorValue = dv->first_valuator;
            return BadValue;
        }


        /* check validity of valuator events */
        base = firstValuator;
        for (n = 1; n < nev; n++)
        {
            dv = (deviceValuator *)(ev + n);
            if (dv->type != DeviceValuator)
            {
                client->errorValue = dv->type;
                return BadValue;
            }
            if (dv->first_valuator != base)
            {
                client->errorValue = dv->first_valuator;
                return BadValue;
            }
            switch(dv->num_valuators)
            {
                case 6: valuators[base + 5] = dv->valuator5;
                case 5: valuators[base + 4] = dv->valuator4;
                case 4: valuators[base + 3] = dv->valuator3;
                case 3: valuators[base + 2] = dv->valuator2;
                case 2: valuators[base + 1] = dv->valuator1;
                case 1: valuators[base] = dv->valuator0;
                        break;
                default:
                        client->errorValue = dv->num_valuators;
                        return BadValue;
            }

            base += dv->num_valuators;
            numValuators += dv->num_valuators;

            if (firstValuator + numValuators > dev->valuator->numAxes)
            {
                client->errorValue = dv->num_valuators;
                return BadValue;
            }
        }
        type = type - XI_DeviceKeyPress + KeyPress;

    } else
    {
        if (nev != 1)
            return BadLength;
        switch (type)
        {
            case KeyPress:
            case KeyRelease:
                dev = PickKeyboard(client);
                break;
            case ButtonPress:
            case ButtonRelease:
                dev = PickPointer(client);
                break;
            case MotionNotify:
                dev = PickPointer(client);
                valuators[0] = ev->u.keyButtonPointer.rootX;
                valuators[1] = ev->u.keyButtonPointer.rootY;
                numValuators = 2;
                firstValuator = 0;
                if (ev->u.u.detail == xFalse)
                    flags = POINTER_ABSOLUTE | POINTER_SCREEN;
                break;
            default:
                client->errorValue = ev->u.u.type;
                return BadValue;
        }

        if (dev->u.lastSlave)
            dev = dev->u.lastSlave;
    }

    /* If the event has a time set, wait for it to pass */
    if (ev->u.keyButtonPointer.time)
    {
        TimeStamp activateTime;
        CARD32 ms;

        activateTime = currentTime;
        ms = activateTime.milliseconds + ev->u.keyButtonPointer.time;
        if (ms < activateTime.milliseconds)
            activateTime.months++;
        activateTime.milliseconds = ms;
        ev->u.keyButtonPointer.time = 0;

        /* see mbuf.c:QueueDisplayRequest for code similar to this */

        if (!ClientSleepUntil(client, &activateTime, NULL, NULL))
        {
            return BadAlloc;
        }
        /* swap the request back so we can simply re-execute it */
        if (client->swapped)
        {
            (void) XTestSwapFakeInput(client, (xReq *)stuff);
            swaps(&stuff->length, n);
        }
        ResetCurrentRequest (client);
        client->sequence--;
        return Success;
    }

    switch (type)
    {
        case KeyPress:
        case KeyRelease:
            if (ev->u.u.detail < dev->key->curKeySyms.minKeyCode ||
                    ev->u.u.detail > dev->key->curKeySyms.maxKeyCode)
            {
                client->errorValue = ev->u.u.detail;
                return BadValue;
            }
            break;
        case MotionNotify:
            /* broken lib, XI events have root uninitialized */
            if (extension || ev->u.keyButtonPointer.root == None)
                root = GetCurrentRootWindow(dev);
            else
            {
                rc = dixLookupWindow(&root, ev->u.keyButtonPointer.root,
                                     client, DixGetAttrAccess);
                if (rc != Success)
                    return rc;
                if (root->parent)
                {
                    client->errorValue = ev->u.keyButtonPointer.root;
                    return BadValue;
                }
            }
            if (ev->u.u.detail != xTrue && ev->u.u.detail != xFalse)
            {
                client->errorValue = ev->u.u.detail;
                return BadValue;
            }

            /* FIXME: Xinerama! */

            break;
        case ButtonPress:
        case ButtonRelease:
            if (!extension)
            {
                dev = PickPointer(client);
                if (dev->u.lastSlave)
                    dev = dev->u.lastSlave;
            }
            if (!ev->u.u.detail || ev->u.u.detail > dev->button->numButtons)
            {
                client->errorValue = ev->u.u.detail;
                return BadValue;
            }
            break;
    }
    if (screenIsSaved == SCREEN_SAVER_ON)
        dixSaveScreens(serverClient, SCREEN_SAVER_OFF, ScreenSaverReset);

    OsBlockSignals();
    GetEventList(&events);
    switch(type) {
        case MotionNotify:
            nevents = GetPointerEvents(events, dev, type, 0, flags,
                            firstValuator, numValuators, valuators);
            break;
        case ButtonPress:
        case ButtonRelease:
            nevents = GetPointerEvents(events, dev, type, ev->u.u.detail,
                                       flags, firstValuator,
                                       numValuators, valuators);
            break;
        case KeyPress:
        case KeyRelease:
            nevents = GetKeyboardEvents(events, dev, type, ev->u.u.detail);
            break;
    }

    for (i = 0; i < nevents; i++)
        mieqEnqueue(dev, (events+i)->event);
    OsReleaseSignals();

    return client->noClientException;
}

static int
ProcXTestGrabControl(client)
    ClientPtr client;
{
    REQUEST(xXTestGrabControlReq);

    REQUEST_SIZE_MATCH(xXTestGrabControlReq);
    if ((stuff->impervious != xTrue) && (stuff->impervious != xFalse))
    {
        client->errorValue = stuff->impervious;
        return(BadValue);
    }
    if (stuff->impervious)
        MakeClientGrabImpervious(client);
    else
        MakeClientGrabPervious(client);
    return(client->noClientException);
}

static int
ProcXTestDispatch (client)
    ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
        case X_XTestGetVersion:
            return ProcXTestGetVersion(client);
        case X_XTestCompareCursor:
            return ProcXTestCompareCursor(client);
        case X_XTestFakeInput:
            return ProcXTestFakeInput(client);
        case X_XTestGrabControl:
            return ProcXTestGrabControl(client);
        default:
            return BadRequest;
    }
}

static int
SProcXTestGetVersion(client)
    ClientPtr	client;
{
    int n;
    REQUEST(xXTestGetVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestGetVersionReq);
    swaps(&stuff->minorVersion, n);
    return ProcXTestGetVersion(client);
}

static int
SProcXTestCompareCursor(client)
    ClientPtr	client;
{
    int n;
    REQUEST(xXTestCompareCursorReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestCompareCursorReq);
    swapl(&stuff->window, n);
    swapl(&stuff->cursor, n);
    return ProcXTestCompareCursor(client);
}

static int
XTestSwapFakeInput(client, req)
    ClientPtr	client;
    xReq *req;
{
    int nev;
    xEvent *ev;
    xEvent sev;
    EventSwapPtr proc;

    nev = ((req->length << 2) - sizeof(xReq)) / sizeof(xEvent);
    for (ev = (xEvent *)&req[1]; --nev >= 0; ev++)
    {
        /* Swap event */
        proc = EventSwapVector[ev->u.u.type & 0177];
        /* no swapping proc; invalid event type? */
        if (!proc ||  proc ==  NotImplemented) {
            client->errorValue = ev->u.u.type;
            return BadValue;
        }
        (*proc)(ev, &sev);
        *ev = sev;
    }
    return Success;
}

static int
SProcXTestFakeInput(client)
    ClientPtr	client;
{
    int n;
    REQUEST(xReq);

    swaps(&stuff->length, n);
    n = XTestSwapFakeInput(client, stuff);
    if (n != Success)
        return n;
    return ProcXTestFakeInput(client);
}

static int
SProcXTestGrabControl(client)
    ClientPtr	client;
{
    int n;
    REQUEST(xXTestGrabControlReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xXTestGrabControlReq);
    return ProcXTestGrabControl(client);
}

static int
SProcXTestDispatch (client)
    ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
        case X_XTestGetVersion:
            return SProcXTestGetVersion(client);
        case X_XTestCompareCursor:
            return SProcXTestCompareCursor(client);
        case X_XTestFakeInput:
            return SProcXTestFakeInput(client);
        case X_XTestGrabControl:
            return SProcXTestGrabControl(client);
        default:
            return BadRequest;
    }
}
