/*
 *
 * Copyright 1990, 1998  The Open Group
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of The Open Group shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from The Open Group.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * dmxeq.c is derived from mi/mieq.c so that XInput events can be handled
 *
 * Modified by: Rickard E. (Rik) Faith <faith@redhat.com>
 *
 * Copyright 2002-2003 Red Hat Inc., Durham, North Carolina.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL RED HAT AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/** \file
 * This file provides an event queue that knows about XInput events.
 * All of the code is based on mi/mieq.c and was modified as little as
 * possible to provide XInput event support (the copyright and some of
 * the comments are from The Open Group, Keith Packard, MIT X
 * Consortium).  (Another example of similar code is provided in
 * hw/xfree86/common/xf86Xinput.c.) */

#ifdef HAVE_DMX_CONFIG_H
#include <dmx-config.h>
#endif

#define DMX_EQ_DEBUG 0

#include "dmx.h"
#include "dmxeq.h"
#include "dmxinput.h"
#include "dmxlog.h"
#include "dmxdpms.h"

#include "inputstr.h"
#include "scrnintstr.h"         /* For screenInfo */

#include <X11/extensions/XIproto.h>
#define EXTENSION_PROC_ARGS void *

#if DMX_EQ_DEBUG
#define DMXDBG2(f,a,b)           dmxLog(dmxDebug,f,a,b)
#define DMXDBG5(f,a,b,c,d,e)     dmxLog(dmxDebug,f,a,b,c,d,e)
#else
#define DMXDBG2(f,a,b)
#define DMXDBG5(f,a,b,c,d,e)
#endif

/** The size of our queue.  (The queue provided by mi/mieq.c has a size
 * of 256.) */
#define QUEUE_SIZE  256

/** Information about the event. */
typedef struct _Event {
    xEvent	   event;    /**< Event. */
    ScreenPtr	   pScreen;  /**< Screen on which event occurred. */
    deviceValuator valuator; /**< XInput device valuator information. */
    DeviceIntPtr   pDev;
} EventRec, *EventPtr;

/** Event queue. */
typedef struct _EventQueue {
    HWEventQueueType head; /**< Queue head; must be long for SetInputCheck. */ 
    HWEventQueueType tail; /**< Queue tail; must be long for SetInputCheck. */ 
    CARD32           lastEventTime; /**< To avoid time running backwards. */
    Bool	     lastMotion;    /**< True if last event was motion.  */
    EventRec         events[QUEUE_SIZE]; /**< Static allocation for signals. */
    DevicePtr	     pKbd, pPtr;    /**< Device pointers (to get funcs) */
    ScreenPtr	     pEnqueueScreen;/**< Screen events are delivered to. */
    ScreenPtr	     pDequeueScreen;/**< Screen events are dispatched to. */
} EventQueueRec, *EventQueuePtr;

static EventQueueRec dmxEventQueue;
static Bool          dmxeqInitializedFlag = FALSE;

Bool dmxeqInitialized(void)
{
    return dmxeqInitializedFlag;
}

Bool dmxeqInit(DevicePtr pKbd, DevicePtr pPtr)
{
    static unsigned long dmxGeneration = 0;
    
    if (dmxGeneration == serverGeneration && dmxeqInitializedFlag)
        return FALSE;
    dmxGeneration                = serverGeneration;
    dmxeqInitializedFlag         = TRUE;
    dmxEventQueue.head           = 0;
    dmxEventQueue.tail           = 0;
    dmxEventQueue.lastEventTime  = GetTimeInMillis();
    dmxEventQueue.pKbd           = pKbd;
    dmxEventQueue.pPtr           = pPtr;
    dmxEventQueue.lastMotion     = FALSE;
    dmxEventQueue.pEnqueueScreen = screenInfo.screens[0];
    dmxEventQueue.pDequeueScreen = dmxEventQueue.pEnqueueScreen;
    SetInputCheck(&dmxEventQueue.head, &dmxEventQueue.tail);
    return TRUE;
}

/**
 * This function adds an event to the end of the queue.  If the event is
 * an XInput event, then the next event (the valuator event) is also
 * stored in the queue.  If the new event has a time before the time of
 * the last event currently on the queue, then the time is updated for
 * the new event.
 *
 * Must be reentrant with ProcessInputEvents.  Assumption: dmxeqEnqueue
 * will never be interrupted.  If this is called from both signal
 * handlers and regular code, make sure the signal is suspended when
 * called from regular code.
 */

void dmxeqEnqueue(DeviceIntPtr pDev, xEvent *e)
{
    HWEventQueueType oldtail, newtail;
    Bool             isMotion;

    oldtail                               = dmxEventQueue.tail;
    isMotion                              = e->u.u.type == MotionNotify;
    if (isMotion
        && dmxEventQueue.lastMotion
        && oldtail != dmxEventQueue.head) {
	if (oldtail == 0) oldtail = QUEUE_SIZE;
	oldtail = oldtail - 1;
    } else {
    	newtail = oldtail + 1;
    	if (newtail == QUEUE_SIZE) newtail = 0;
    	/* Toss events which come in late */
    	if (newtail == dmxEventQueue.head) return;
	dmxEventQueue.tail = newtail;
    }
    DMXDBG2("dmxeqEnqueue %d %d\n", dmxEventQueue.head, dmxEventQueue.tail);
    dmxEventQueue.lastMotion              = isMotion;
    dmxEventQueue.events[oldtail].pScreen = dmxEventQueue.pEnqueueScreen;

                                /* Store the event in the queue */
    dmxEventQueue.events[oldtail].event   = *e;
    dmxEventQueue.events[oldtail].pDev    = pDev;
                            /* If this is an XInput event, store the
                             * valuator event, too */
    deviceKeyButtonPointer *ev = (deviceKeyButtonPointer *)e;
    if (e->u.u.type >= LASTEvent && (ev->deviceid & MORE_EVENTS))
        dmxEventQueue.events[oldtail].valuator = *(deviceValuator *)(ev+1);

                                /* Make sure that event times don't go
                                 * backwards - this is "unnecessary",
                                 * but very useful */
    if (e->u.keyButtonPointer.time < dmxEventQueue.lastEventTime
        && dmxEventQueue.lastEventTime - e->u.keyButtonPointer.time < 10000) {
	dmxEventQueue.events[oldtail].event.u.keyButtonPointer.time =
	    dmxEventQueue.lastEventTime;
    }
}

/** Make \a pScreen the new screen for enqueueing events.  If \a fromDIX
 * is TRUE, also make \a pScreen the new screen for dequeuing events. */
void dmxeqSwitchScreen(DeviceIntPtr pDev, ScreenPtr pScreen, Bool fromDIX)
{
    dmxEventQueue.pEnqueueScreen = pScreen;
    if (fromDIX) dmxEventQueue.pDequeueScreen = pScreen;
}

static void dmxeqProcessXInputEvent(xEvent *xe, EventRec *e)
{
    deviceKeyButtonPointer *ev     = (deviceKeyButtonPointer *)xe;
    int                    id      = ev->deviceid & DEVICE_BITS;
    DeviceIntPtr           pDevice;
    
    dixLookupDevice(&pDevice, id, serverClient, DixUnknownAccess);
    if (!pDevice) {
        dmxLog(dmxError, "dmxeqProcessInputEvents: id %d not found\n", id);
        return;
    }

    if (!pDevice->public.processInputProc) {
        dmxLog(dmxError,
               "dmxeqProcessInputEvents: no processInputProc for"
               " device id %d (%s)\n", id, pDevice->name);
        return;
    }
    
    if (ev->deviceid & MORE_EVENTS) {
        xe[1] = *(xEvent *)(&e->valuator);
        pDevice->public.processInputProc(xe, pDevice, 2);
    } else {
        pDevice->public.processInputProc(xe, pDevice, 1);
    }
}

/**
 * This function is called from #ProcessInputEvents() to remove events
 * from the queue and process them.
 */
void dmxeqProcessInputEvents(void)
{
    EventRec	*e;
    int		x, y;
    xEvent	xe[2];

    while (dmxEventQueue.head != dmxEventQueue.tail) {
        dmxDPMSWakeup();        /* Handles screen saver and DPMS */
	e = &dmxEventQueue.events[dmxEventQueue.head];
        DMXDBG5("dmxeqProcessInputEvents: type=%d screen=%p,%p root=%d,%d\n",
                e->event.u.u.type,
                e->pScreen, dmxEventQueue.pDequeueScreen,
                e->event.u.keyButtonPointer.rootX,
                e->event.u.keyButtonPointer.rootY);
	/*
	 * Assumption - screen switching can only occur on core motion events
	 */
	if (e->event.u.u.type == MotionNotify
            && e->pScreen != dmxEventQueue.pDequeueScreen) {
	    dmxEventQueue.pDequeueScreen = e->pScreen;
	    x = e->event.u.keyButtonPointer.rootX;
	    y = e->event.u.keyButtonPointer.rootY;
	    if (dmxEventQueue.head == QUEUE_SIZE - 1) dmxEventQueue.head = 0;
	    else                                      ++dmxEventQueue.head;
	    NewCurrentScreen(e->pDev, dmxEventQueue.pDequeueScreen, x, y);
	} else {
	    xe[0] = e->event;
	    if (dmxEventQueue.head == QUEUE_SIZE - 1) dmxEventQueue.head = 0;
	    else                                      ++dmxEventQueue.head;
	    switch (xe[0].u.u.type) {
	    case KeyPress:
	    case KeyRelease:
                if (!dmxEventQueue.pKbd) {
                    dmxLog(dmxError, "dmxeqProcessInputEvents: No keyboard\n");
                    return;
                }
                dmxEventQueue.pKbd
                    ->processInputProc(xe,
                                       (DeviceIntPtr)dmxEventQueue.pKbd, 1);
	    	break;
            default:
                dmxeqProcessXInputEvent(xe, e);
                break;
            case ButtonPress:
            case ButtonRelease:
            case MotionNotify:
                if (!dmxEventQueue.pPtr) {
                    dmxLog(dmxError, "dmxeqProcessInputEvents: No mouse\n");
                    return;
                }
                dmxEventQueue.pPtr
                    ->processInputProc(xe,
                                       (DeviceIntPtr)dmxEventQueue.pPtr, 1);
	    	break;
	    }
	}
    }
}
