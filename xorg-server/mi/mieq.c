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
# include   <X11/extensions/geproto.h>
# include   "extinit.h"
# include   "exglobals.h"

#ifdef DPMSExtension
# include "dpmsproc.h"
# define DPMS_SERVER
# include <X11/extensions/dpms.h>
#endif

#define QUEUE_SIZE  512

#define EnqueueScreen(dev) dev->spriteInfo->sprite->pEnqueueScreen
#define DequeueScreen(dev) dev->spriteInfo->sprite->pDequeueScreen

typedef struct _Event {
    EventListPtr    events;
    int             nevents;
    ScreenPtr	    pScreen;
    DeviceIntPtr    pDev; /* device this event _originated_ from */
} EventRec, *EventPtr;

typedef struct _EventQueue {
    HWEventQueueType head, tail;         /* long for SetInputCheck */
    CARD32           lastEventTime;      /* to avoid time running backwards */
    int              lastMotion;         /* device ID if last event motion? */
    EventRec         events[QUEUE_SIZE]; /* static allocation for signals */
    mieqHandler      handlers[128];      /* custom event handler */
} EventQueueRec, *EventQueuePtr;

static EventQueueRec miEventQueue;
static EventListPtr masterEvents; /* for use in mieqProcessInputEvents */

#ifdef XQUARTZ
#include  <pthread.h>
static pthread_mutex_t miEventQueueMutex = PTHREAD_MUTEX_INITIALIZER;

extern BOOL serverInitComplete;
extern pthread_mutex_t serverInitCompleteMutex;
extern pthread_cond_t serverInitCompleteCond;

static inline void wait_for_server_init(void) {
    /* If the server hasn't finished initializing, wait for it... */
    if(!serverInitComplete) {
        pthread_mutex_lock(&serverInitCompleteMutex);
        while(!serverInitComplete)
            pthread_cond_wait(&serverInitCompleteCond, &serverInitCompleteMutex);
        pthread_mutex_unlock(&serverInitCompleteMutex);
    }
}
#endif

Bool
mieqInit(void)
{
    int i;

    miEventQueue.head = miEventQueue.tail = 0;
    miEventQueue.lastEventTime = GetTimeInMillis ();
    miEventQueue.lastMotion = FALSE;
    for (i = 0; i < 128; i++)
        miEventQueue.handlers[i] = NULL;
    for (i = 0; i < QUEUE_SIZE; i++)
    {
        EventListPtr evlist = InitEventList(1 + MAX_VALUATOR_EVENTS);
        if (!evlist)
            FatalError("Could not allocate event queue.\n");
        miEventQueue.events[i].events = evlist;
    }

    /* XXX: mE is just 1 event long, if we have Motion + Valuator they are
     * squashed into the first event to make passing it into the event
     * processing handlers easier. This should be fixed when the processing
     * handlers switch to EventListPtr instead of xEvent */
    masterEvents = InitEventList(1);
    if (!masterEvents)
        FatalError("Could not allocated MD event queue.\n");
    SetMinimumEventSize(masterEvents, 1,
                        (1 + MAX_VALUATOR_EVENTS) * sizeof(xEvent));

    SetInputCheck(&miEventQueue.head, &miEventQueue.tail);
    return TRUE;
}

/* Ensure all events in the EQ are at least size bytes. */
void
mieqResizeEvents(int min_size)
{
    int i;

    for (i = 0; i < QUEUE_SIZE; i++)
        SetMinimumEventSize(miEventQueue.events[i].events, 7, min_size);
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
    unsigned int           oldtail = miEventQueue.tail;
    EventListPtr           evt;
    int                    isMotion = 0;
    int                    evlen;

#ifdef XQUARTZ
    wait_for_server_init();
    pthread_mutex_lock(&miEventQueueMutex);
#endif

    /* avoid merging events from different devices */
    if (e->u.u.type == MotionNotify)
        isMotion = pDev->id;
    else if (e->u.u.type == DeviceMotionNotify)
        isMotion = pDev->id | (1 << 8); /* flag to indicate DeviceMotion */

    /* We silently steal valuator events: just tack them on to the last
     * motion event they need to be attached to.  Sigh. */
    if (e->u.u.type == DeviceValuator) {
        deviceValuator         *v = (deviceValuator *) e;
        EventPtr               laste;
        deviceKeyButtonPointer *lastkbp;

        laste = &miEventQueue.events[(oldtail - 1) % QUEUE_SIZE];
        lastkbp = (deviceKeyButtonPointer *) laste->events->event;

        if (laste->nevents > 6) {
#ifdef XQUARTZ
            pthread_mutex_unlock(&miEventQueueMutex);
#endif
            
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
#ifdef XQUARTZ
            pthread_mutex_unlock(&miEventQueueMutex);
#endif
            ErrorF("[mi] mieqEnequeue: out-of-order valuator event; dropping.\n");
            return;
        }

        memcpy((laste->events[laste->nevents++].event), e, sizeof(xEvent));
#ifdef XQUARTZ
        pthread_mutex_unlock(&miEventQueueMutex);
#endif
        return;
    }

    if (isMotion && isMotion == miEventQueue.lastMotion &&
        oldtail != miEventQueue.head) {
	oldtail = (oldtail - 1) % QUEUE_SIZE;
    }
    else {
	static int stuck = 0;
	/* Toss events which come in late.  Usually this means your server's
         * stuck in an infinite loop somewhere, but SIGIO is still getting
         * handled. */
	if (((oldtail + 1) % QUEUE_SIZE) == miEventQueue.head) {
            ErrorF("[mi] EQ overflowing. The server is probably stuck "
                   "in an infinite loop.\n");
	    if (!stuck) {
		xorg_backtrace();
		stuck = 1;
	    }
#ifdef XQUARTZ
	    pthread_mutex_unlock(&miEventQueueMutex);
#endif
	    return;
        }
	stuck = 0;
    }

    evlen = sizeof(xEvent);
    if (e->u.u.type == GenericEvent)
        evlen += ((xGenericEvent*)e)->length * 4;

    evt = miEventQueue.events[oldtail].events;
    if (evt->evlen < evlen)
    {
        evt->evlen = evlen;
        evt->event = xrealloc(evt->event, evt->evlen);
        if (!evt->event)
        {
            ErrorF("[mi] Running out of memory. Tossing event.\n");
#ifdef XQUARTZ
            pthread_mutex_unlock(&miEventQueueMutex);
#endif
            return;
        }
    }

    memcpy(evt->event, e, evlen);
    miEventQueue.events[oldtail].nevents = 1;

    /* Make sure that event times don't go backwards - this
     * is "unnecessary", but very useful. */
    if (e->u.u.type != GenericEvent &&
        e->u.keyButtonPointer.time < miEventQueue.lastEventTime &&
            miEventQueue.lastEventTime - e->u.keyButtonPointer.time < 10000)
        evt->event->u.keyButtonPointer.time = miEventQueue.lastEventTime;

    miEventQueue.lastEventTime = evt->event->u.keyButtonPointer.time;
    miEventQueue.events[oldtail].pScreen = EnqueueScreen(pDev);
    miEventQueue.events[oldtail].pDev = pDev;

    miEventQueue.lastMotion = isMotion;
    miEventQueue.tail = (oldtail + 1) % QUEUE_SIZE;
#ifdef XQUARTZ
    pthread_mutex_unlock(&miEventQueueMutex);
#endif
}

void
mieqSwitchScreen(DeviceIntPtr pDev, ScreenPtr pScreen, Bool fromDIX)
{
#ifdef XQUARTZ
    pthread_mutex_lock(&miEventQueueMutex);
#endif
    EnqueueScreen(pDev) = pScreen;
    if (fromDIX)
	DequeueScreen(pDev) = pScreen;
#ifdef XQUARTZ
    pthread_mutex_unlock(&miEventQueueMutex);
#endif
}

void
mieqSetHandler(int event, mieqHandler handler)
{
#ifdef XQUARTZ
    pthread_mutex_lock(&miEventQueueMutex);
#endif
    if (handler && miEventQueue.handlers[event])
        ErrorF("[mi] mieq: warning: overriding existing handler %p with %p for "
               "event %d\n", miEventQueue.handlers[event], handler, event);

    miEventQueue.handlers[event] = handler;
#ifdef XQUARTZ
    pthread_mutex_unlock(&miEventQueueMutex);
#endif
}

/**
 * Change the device id of the given event to the given device's id.
 */
static void
ChangeDeviceID(DeviceIntPtr dev, xEvent* event)
{
    int type = event->u.u.type;

    if (type == DeviceKeyPress || type == DeviceKeyRelease ||
            type == DeviceButtonPress || type == DeviceButtonRelease ||
            type == DeviceMotionNotify || type == ProximityIn ||
            type == ProximityOut)
        ((deviceKeyButtonPointer*)event)->deviceid = dev->id;
    else if (type == DeviceValuator)
        ((deviceValuator*)event)->deviceid = dev->id;
    else if (type == GenericEvent)
    {
        DebugF("[mi] Unknown generic event (%d/%d), cannot change id.\n",
                ((xGenericEvent*)event)->extension,
                ((xGenericEvent*)event)->evtype);
    } else
        DebugF("[mi] Unknown event type (%d), cannot change id.\n", type);
}

static void
FixUpEventForMaster(DeviceIntPtr mdev, DeviceIntPtr sdev, xEvent* original,
                    EventListPtr master, int count)
{
    /* Ensure chained button mappings, i.e. that the detail field is the
     * value of the mapped button on the SD, not the physical button */
    if (original->u.u.type == DeviceButtonPress || original->u.u.type == DeviceButtonRelease)
    {
        int btn = original->u.u.detail;
        if (!sdev->button)
            return; /* Should never happen */

        master->event->u.u.detail = sdev->button->map[btn];
    }
}

/**
 * Copy the given event into master.
 * @param mdev The master device
 * @param sdev The slave device the original event comes from
 * @param original The event as it came from the EQ
 * @param master The event after being copied
 * @param count Number of events in original.
 */
void
CopyGetMasterEvent(DeviceIntPtr mdev, DeviceIntPtr sdev, xEvent* original,
                   EventListPtr master, int count)
{
    int len = count * sizeof(xEvent);

    /* Assumption: GenericEvents always have count 1 */

    if (GEV(original)->type == GenericEvent)
        len += GEV(original)->length * 4;

    if (master->evlen < len)
        SetMinimumEventSize(master, 1, len);

    memcpy(master->event, original, len);
    while (count--)
    {
        ChangeDeviceID(mdev, &master->event[count]);
        FixUpEventForMaster(mdev, sdev, original, master, count);
    }
}
extern void
CopyKeyClass(DeviceIntPtr device, DeviceIntPtr master);



/* Call this from ProcessInputEvents(). */
void
mieqProcessInputEvents(void)
{
    mieqHandler handler;
    EventRec *e = NULL;
    int x = 0, y = 0;
    int type, nevents, evlen, i;
    ScreenPtr screen;
    static xEvent *event = NULL;
    static size_t event_size = 0;
    DeviceIntPtr dev = NULL,
                 master = NULL;

#ifdef XQUARTZ
    pthread_mutex_lock(&miEventQueueMutex);
#endif
    
    while (miEventQueue.head != miEventQueue.tail) {
        e = &miEventQueue.events[miEventQueue.head];

        /* GenericEvents always have nevents == 1 */
        nevents = e->nevents;
        evlen   = (nevents > 1) ? sizeof(xEvent) : e->events->evlen;
        if((nevents * evlen) > event_size) {
            event_size = nevents * evlen;
            event = (xEvent *)xrealloc(event, event_size);
        }

        if (!event)
            FatalError("[mi] No memory left for event processing.\n");

        for (i = 0; i < nevents; i++)
            memcpy(&event[i], e->events[i].event, evlen);


        dev     = e->pDev;
        screen  = e->pScreen;

        miEventQueue.head = (miEventQueue.head + 1) % QUEUE_SIZE;

#ifdef XQUARTZ
        pthread_mutex_unlock(&miEventQueueMutex);
#endif
        
        type    = event->u.u.type;
        master  = (!dev->isMaster && dev->u.master) ? dev->u.master : NULL;

        if (screenIsSaved == SCREEN_SAVER_ON)
            dixSaveScreens (serverClient, SCREEN_SAVER_OFF, ScreenSaverReset);
#ifdef DPMSExtension
        else if (DPMSPowerLevel != DPMSModeOn)
            SetScreenSaverTimer();

        if (DPMSPowerLevel != DPMSModeOn)
            DPMSSet(serverClient, DPMSModeOn);
#endif

        /* Custom event handler */
        handler = miEventQueue.handlers[type];

        if (screen != DequeueScreen(dev) && !handler) {
            /* Assumption - screen switching can only occur on motion events. */
            DequeueScreen(dev) = screen;
            x = event->u.keyButtonPointer.rootX;
            y = event->u.keyButtonPointer.rootY;
            NewCurrentScreen (dev, DequeueScreen(dev), x, y);
        }
        else {
            if (master) {
                /* Force a copy of the key class into the VCK so that the layout
                   is transferred. */
                if (event->u.u.type == DeviceKeyPress ||
                    event->u.u.type == DeviceKeyRelease)
                {
                    if (!master->key)
                        master = GetPairedDevice(master);
		    CopyKeyClass(dev, master);
                }

                CopyGetMasterEvent(master, dev, event, masterEvents, nevents);
            }

            /* If someone's registered a custom event handler, let them
             * steal it. */
            if (handler)
            {
                handler(DequeueScreen(dev)->myNum, event, dev, nevents);
                if (master)
                    handler(DequeueScreen(master)->myNum,
                            masterEvents->event, master, nevents);
            } else
            {
                /* process slave first, then master */
                dev->public.processInputProc(event, dev, nevents);

                if (master)
                    master->public.processInputProc(masterEvents->event, master,
                                                    nevents);
            }
        }

        /* Update the sprite now. Next event may be from different device. */
        if (type == DeviceMotionNotify && dev->coreEvents)
            miPointerUpdateSprite(dev);

#ifdef XQUARTZ
        pthread_mutex_lock(&miEventQueueMutex);
#endif
    }
#ifdef XQUARTZ
    pthread_mutex_unlock(&miEventQueueMutex);
#endif
}

