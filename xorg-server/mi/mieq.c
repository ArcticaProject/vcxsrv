/*
 *
Copyright 1990, 1998  The Open Group

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
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * mieq.c
 *
 * Machine independent event queue
 *
 */

#if HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

# define NEED_EVENTS
# include   <X11/X.h>
# include   <X11/Xmd.h>
# include   <X11/Xproto.h>
# include   "misc.h"
# include   "windowstr.h"
# include   "pixmapstr.h"
# include   "inputstr.h"
# include   "mi.h"
# include   "mipointer.h"
# include   "scrnintstr.h"
# include   <X11/extensions/XI.h>
# include   <X11/extensions/XIproto.h>
# include   "extinit.h"
# include   "exglobals.h"

#ifdef DPMSExtension
# include "dpmsproc.h"
# define DPMS_SERVER
# include <X11/extensions/dpms.h>
#endif

#define QUEUE_SIZE  512

typedef struct _Event {
    xEvent          event[7];
    int             nevents;
    ScreenPtr	    pScreen;
    DeviceIntPtr    pDev; /* device this event _originated_ from */
} EventRec, *EventPtr;

typedef struct _EventQueue {
    HWEventQueueType head, tail;         /* long for SetInputCheck */
    CARD32           lastEventTime;      /* to avoid time running backwards */
    int              lastMotion;         /* device ID if last event motion? */
    EventRec         events[QUEUE_SIZE]; /* static allocation for signals */
    ScreenPtr        pEnqueueScreen;     /* screen events are being delivered to */
    ScreenPtr        pDequeueScreen;     /* screen events are being dispatched to */
    mieqHandler      handlers[128];      /* custom event handler */
} EventQueueRec, *EventQueuePtr;

static EventQueueRec miEventQueue;

Bool
mieqInit(void)
{
    int i;

    miEventQueue.head = miEventQueue.tail = 0;
    miEventQueue.lastEventTime = GetTimeInMillis ();
    miEventQueue.lastMotion = FALSE;
    miEventQueue.pEnqueueScreen = screenInfo.screens[0];
    miEventQueue.pDequeueScreen = miEventQueue.pEnqueueScreen;
    for (i = 0; i < 128; i++)
        miEventQueue.handlers[i] = NULL;
    SetInputCheck(&miEventQueue.head, &miEventQueue.tail);
    return TRUE;
}

/*
 * Must be reentrant with ProcessInputEvents.  Assumption: mieqEnqueue
 * will never be interrupted.  If this is called from both signal
 * handlers and regular code, make sure the signal is suspended when
 * called from regular code.
 */

void
mieqEnqueue(DeviceIntPtr pDev, xEvent *e)
{
    unsigned int           oldtail = miEventQueue.tail, newtail;
    int                    isMotion = 0;
    deviceValuator         *v = (deviceValuator *) e;
    EventPtr               laste = &miEventQueue.events[(oldtail - 1) %
							QUEUE_SIZE];
    deviceKeyButtonPointer *lastkbp = (deviceKeyButtonPointer *)
                                      &laste->event[0];

    if (e->u.u.type == MotionNotify)
        isMotion = inputInfo.pointer->id;
    else if (e->u.u.type == DeviceMotionNotify)
        isMotion = pDev->id;

    /* We silently steal valuator events: just tack them on to the last
     * motion event they need to be attached to.  Sigh. */
    if (e->u.u.type == DeviceValuator) {
        if (laste->nevents > 6) {
            ErrorF("[mi] mieqEnqueue: more than six valuator events; dropping.\n");
            return;
        }
        if (oldtail == miEventQueue.head ||
            !(lastkbp->type == DeviceMotionNotify ||
              lastkbp->type == DeviceButtonPress ||
              lastkbp->type == DeviceButtonRelease ||
              lastkbp->type == ProximityIn ||
              lastkbp->type == ProximityOut) ||
            ((lastkbp->deviceid & DEVICE_BITS) !=
             (v->deviceid & DEVICE_BITS))) {
            ErrorF("[mi] mieqEnequeue: out-of-order valuator event; dropping.\n");
            return;
        }
        memcpy(&(laste->event[laste->nevents++]), e, sizeof(xEvent));
        return;
    }

    if (isMotion && isMotion == miEventQueue.lastMotion &&
        oldtail != miEventQueue.head) {
	oldtail = (oldtail - 1) % QUEUE_SIZE;
    }
    else {
	static int stuck = 0;
	newtail = (oldtail + 1) % QUEUE_SIZE;
	/* Toss events which come in late.  Usually this means your server's
         * stuck in an infinite loop somewhere, but SIGIO is still getting
         * handled. */
	if (newtail == miEventQueue.head) {
            ErrorF("[mi] EQ overflowing. The server is probably stuck "
                   "in an infinite loop.\n");
	    if (!stuck) {
		xorg_backtrace();
		stuck = 1;
	    }
	    return;
        }
	stuck = 0;
	miEventQueue.tail = newtail;
    }

    memcpy(&(miEventQueue.events[oldtail].event[0]), e, sizeof(xEvent));
    miEventQueue.events[oldtail].nevents = 1;

    /* Make sure that event times don't go backwards - this
     * is "unnecessary", but very useful. */
    if (e->u.keyButtonPointer.time < miEventQueue.lastEventTime &&
	miEventQueue.lastEventTime - e->u.keyButtonPointer.time < 10000)
	miEventQueue.events[oldtail].event[0].u.keyButtonPointer.time =
	    miEventQueue.lastEventTime;

    miEventQueue.lastEventTime =
	miEventQueue.events[oldtail].event[0].u.keyButtonPointer.time;
    miEventQueue.events[oldtail].pScreen = miEventQueue.pEnqueueScreen;
    miEventQueue.events[oldtail].pDev = pDev;

    miEventQueue.lastMotion = isMotion;
}

void
mieqSwitchScreen(ScreenPtr pScreen, Bool fromDIX)
{
    miEventQueue.pEnqueueScreen = pScreen;
    if (fromDIX)
	miEventQueue.pDequeueScreen = pScreen;
}

void
mieqSetHandler(int event, mieqHandler handler)
{
    if (handler && miEventQueue.handlers[event])
        ErrorF("mieq: warning: overriding existing handler %p with %p for "
               "event %d\n", miEventQueue.handlers[event], handler, event);

    miEventQueue.handlers[event] = handler;
}

/* Call this from ProcessInputEvents(). */
void
mieqProcessInputEvents(void)
{
    EventRec *e = NULL;
    int x = 0, y = 0;
    DeviceIntPtr dev = NULL;

    while (miEventQueue.head != miEventQueue.tail) {
        if (screenIsSaved == SCREEN_SAVER_ON)
            dixSaveScreens (serverClient, SCREEN_SAVER_OFF, ScreenSaverReset);
#ifdef DPMSExtension
        else if (DPMSPowerLevel != DPMSModeOn)
            SetScreenSaverTimer();

        if (DPMSPowerLevel != DPMSModeOn)
            DPMSSet(serverClient, DPMSModeOn);
#endif

        e = &miEventQueue.events[miEventQueue.head];
        /* Assumption - screen switching can only occur on motion events. */
        miEventQueue.head = (miEventQueue.head + 1) % QUEUE_SIZE;

        if (e->pScreen != miEventQueue.pDequeueScreen) {
            miEventQueue.pDequeueScreen = e->pScreen;
            x = e->event[0].u.keyButtonPointer.rootX;
            y = e->event[0].u.keyButtonPointer.rootY;
            NewCurrentScreen (miEventQueue.pDequeueScreen, x, y);
        }
        else {
            /* If someone's registered a custom event handler, let them
             * steal it. */
            if (miEventQueue.handlers[e->event->u.u.type]) {
                miEventQueue.handlers[e->event->u.u.type](miEventQueue.pDequeueScreen->myNum,
                                                          e->event, dev,
                                                          e->nevents);
                return;
            }

            /* If this is a core event, make sure our keymap, et al, is
             * changed to suit. */
            if (e->event[0].u.u.type == KeyPress ||
                e->event[0].u.u.type == KeyRelease) {
                SwitchCoreKeyboard(e->pDev);
                dev = inputInfo.keyboard;
            }
            else if (e->event[0].u.u.type == MotionNotify ||
                     e->event[0].u.u.type == ButtonPress ||
                     e->event[0].u.u.type == ButtonRelease) {
                SwitchCorePointer(e->pDev);
                dev = inputInfo.pointer;
            }
            else {
                dev = e->pDev;
            }

            dev->public.processInputProc(e->event, dev, e->nevents);
        }
    }
}
