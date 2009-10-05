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
# include   "eventstr.h"

#ifdef DPMSExtension
# include "dpmsproc.h"
# include <X11/extensions/dpmsconst.h>
#endif

#define QUEUE_SIZE  512

#define EnqueueScreen(dev) dev->spriteInfo->sprite->pEnqueueScreen
#define DequeueScreen(dev) dev->spriteInfo->sprite->pDequeueScreen

typedef struct _Event {
    EventListPtr    events;
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
	if (miEventQueue.events[i].events == NULL) {
	    EventListPtr evlist = InitEventList(1);
	    if (!evlist)
		FatalError("Could not allocate event queue.\n");
	    miEventQueue.events[i].events = evlist;
	}
    }

    SetInputCheck(&miEventQueue.head, &miEventQueue.tail);
    return TRUE;
}

void
mieqFini(void)
{
    int i;
    for (i = 0; i < QUEUE_SIZE; i++)
    {
	if (miEventQueue.events[i].events != NULL) {
	    FreeEventList(miEventQueue.events[i].events, 1);
	    miEventQueue.events[i].events = NULL;
	}
    }
}

/*
 * Must be reentrant with ProcessInputEvents.  Assumption: mieqEnqueue
 * will never be interrupted.  If this is called from both signal
 * handlers and regular code, make sure the signal is suspended when
 * called from regular code.
 */

void
mieqEnqueue(DeviceIntPtr pDev, InternalEvent *e)
{
    unsigned int           oldtail = miEventQueue.tail;
    EventListPtr           evt;
    int                    isMotion = 0;
    int                    evlen;
    Time                   time;

#ifdef XQUARTZ
    wait_for_server_init();
    pthread_mutex_lock(&miEventQueueMutex);
#endif

    CHECKEVENT(e);

    /* avoid merging events from different devices */
    if (e->any.type == ET_Motion)
        isMotion = pDev->id;

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
            if (!stuck) {
                ErrorF("[mi] EQ overflowing. The server is probably stuck "
                        "in an infinite loop.\n");
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

    evlen = e->any.length;
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

    time = e->any.time;
    /* Make sure that event times don't go backwards - this
     * is "unnecessary", but very useful. */
    if (time < miEventQueue.lastEventTime &&
        miEventQueue.lastEventTime - time < 10000)
        e->any.time = miEventQueue.lastEventTime;

    miEventQueue.lastEventTime = ((InternalEvent*)evt->event)->any.time;
    miEventQueue.events[oldtail].pScreen = pDev ? EnqueueScreen(pDev) : NULL;
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
ChangeDeviceID(DeviceIntPtr dev, InternalEvent* event)
{
    switch(event->any.type)
    {
        case ET_Motion:
        case ET_KeyPress:
        case ET_KeyRelease:
        case ET_ButtonPress:
        case ET_ButtonRelease:
        case ET_ProximityIn:
        case ET_ProximityOut:
        case ET_Hierarchy:
        case ET_DeviceChanged:
            event->device_event.deviceid = dev->id;
            break;
#if XFreeXDGA
        case ET_DGAEvent:
            break;
#endif
        case ET_RawKeyPress:
        case ET_RawKeyRelease:
        case ET_RawButtonPress:
        case ET_RawButtonRelease:
        case ET_RawMotion:
            event->raw_event.deviceid = dev->id;
            break;
        default:
            ErrorF("[mi] Unknown event type (%d), cannot change id.\n",
                   event->any.type);
    }
}

static void
FixUpEventForMaster(DeviceIntPtr mdev, DeviceIntPtr sdev,
                    InternalEvent* original, InternalEvent *master)
{
    CHECKEVENT(original);
    CHECKEVENT(master);
    /* Ensure chained button mappings, i.e. that the detail field is the
     * value of the mapped button on the SD, not the physical button */
    if (original->any.type == ET_ButtonPress ||
        original->any.type == ET_ButtonRelease)
    {
        int btn = original->device_event.detail.button;
        if (!sdev->button)
            return; /* Should never happen */

        master->device_event.detail.button = sdev->button->map[btn];
    }
}

/**
 * Copy the given event into master.
 * @param sdev The slave device the original event comes from
 * @param original The event as it came from the EQ
 * @param copy The event after being copied
 * @return The master device or NULL if the device is a floating slave.
 */
DeviceIntPtr
CopyGetMasterEvent(DeviceIntPtr sdev,
                   InternalEvent* original, InternalEvent *copy)
{
    DeviceIntPtr mdev;
    int len = original->any.length;

    CHECKEVENT(original);

    /* ET_XQuartz has sdev == NULL */
    if (!sdev || !sdev->u.master)
        return NULL;

    switch(original->any.type)
    {
        case ET_KeyPress:
        case ET_KeyRelease:
            mdev = GetMaster(sdev, MASTER_KEYBOARD);
            break;
        case ET_ButtonPress:
        case ET_ButtonRelease:
        case ET_Motion:
        case ET_ProximityIn:
        case ET_ProximityOut:
            mdev = GetMaster(sdev, MASTER_POINTER);
            break;
        default:
            mdev = sdev->u.master;
            break;
    }

    memcpy(copy, original, len);
    ChangeDeviceID(mdev, copy);
    FixUpEventForMaster(mdev, sdev, original, copy);

    return mdev;
}


/**
 * Post the given @event through the device hierarchy, as appropriate.
 * Use this function if an event must be posted for a given device during the
 * usual event processing cycle.
 */
void
mieqProcessDeviceEvent(DeviceIntPtr dev,
                       InternalEvent *event,
                       ScreenPtr screen)
{
    mieqHandler handler;
    int x = 0, y = 0;
    DeviceIntPtr master;
    InternalEvent mevent; /* master event */

    CHECKEVENT(event);

    /* Custom event handler */
    handler = miEventQueue.handlers[event->any.type];

    switch (event->any.type) {
        /* Catch events that include valuator information and check if they
         * are changing the screen */
        case ET_Motion:
        case ET_KeyPress:
        case ET_KeyRelease:
        case ET_ButtonPress:
        case ET_ButtonRelease:
            if (dev && screen && screen != DequeueScreen(dev) && !handler) {
                DequeueScreen(dev) = screen;
                x = event->device_event.root_x;
                y = event->device_event.root_y;
                NewCurrentScreen (dev, DequeueScreen(dev), x, y);
            }
            break;
        default:
            break;
    }
    master = CopyGetMasterEvent(dev, event, &mevent);

    if (master)
        master->u.lastSlave = dev;

    /* If someone's registered a custom event handler, let them
     * steal it. */
    if (handler)
    {
        int screenNum = dev && DequeueScreen(dev) ? DequeueScreen(dev)->myNum : (screen ? screen->myNum : 0);
        handler(screenNum, event, dev);
        /* Check for the SD's master in case the device got detached
         * during event processing */
        if (master && dev->u.master)
            handler(screenNum, &mevent, master);
    } else
    {
        /* process slave first, then master */
        dev->public.processInputProc(event, dev);

        /* Check for the SD's master in case the device got detached
         * during event processing */
        if (master && dev->u.master)
            master->public.processInputProc(&mevent, master);
    }
}

/* Call this from ProcessInputEvents(). */
void
mieqProcessInputEvents(void)
{
    EventRec *e = NULL;
    int evlen;
    ScreenPtr screen;
    static InternalEvent *event = NULL;
    static size_t event_size = 0;
    DeviceIntPtr dev = NULL,
                 master = NULL;

#ifdef XQUARTZ
    pthread_mutex_lock(&miEventQueueMutex);
#endif
    
    while (miEventQueue.head != miEventQueue.tail) {
        e = &miEventQueue.events[miEventQueue.head];

        evlen   = e->events->evlen;
        if(evlen > event_size)
            event = xrealloc(event, evlen);

        if (!event)
            FatalError("[mi] No memory left for event processing.\n");

        memcpy(event, e->events->event, evlen);


        dev     = e->pDev;
        screen  = e->pScreen;

        miEventQueue.head = (miEventQueue.head + 1) % QUEUE_SIZE;

#ifdef XQUARTZ
        pthread_mutex_unlock(&miEventQueueMutex);
#endif

        master  = (dev && !IsMaster(dev) && dev->u.master) ? dev->u.master : NULL;

        if (screenIsSaved == SCREEN_SAVER_ON)
            dixSaveScreens (serverClient, SCREEN_SAVER_OFF, ScreenSaverReset);
#ifdef DPMSExtension
        else if (DPMSPowerLevel != DPMSModeOn)
            SetScreenSaverTimer();

        if (DPMSPowerLevel != DPMSModeOn)
            DPMSSet(serverClient, DPMSModeOn);
#endif

        mieqProcessDeviceEvent(dev, event, screen);

        /* Update the sprite now. Next event may be from different device. */
        if (event->any.type == ET_Motion && master)
            miPointerUpdateSprite(dev);

#ifdef XQUARTZ
        pthread_mutex_lock(&miEventQueueMutex);
#endif
    }
#ifdef XQUARTZ
    pthread_mutex_unlock(&miEventQueueMutex);
#endif
}

