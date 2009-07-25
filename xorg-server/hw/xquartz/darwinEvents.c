/*
Darwin event queue and event handling

Copyright 2007-2008 Apple Inc.
Copyright 2004 Kaleb S. KEITHLEY. All Rights Reserved.
Copyright (c) 2002-2004 Torrey T. Lyons. All Rights Reserved.

This file is based on mieq.c by Keith Packard,
which contains the following copyright:
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
 */

#include "sanitizedCarbon.h"

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#define NEED_EVENTS
#include   <X11/X.h>
#include   <X11/Xmd.h>
#include   <X11/Xproto.h>
#include   "misc.h"
#include   "windowstr.h"
#include   "pixmapstr.h"
#include   "inputstr.h"
#include   "mi.h"
#include   "scrnintstr.h"
#include   "mipointer.h"
#include   "os.h"

#include "darwin.h"
#include "quartz.h"
#include "quartzKeyboard.h"
#include "darwinEvents.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <IOKit/hidsystem/IOLLEvent.h>

/* Fake button press/release for scroll wheel move. */
#define SCROLLWHEELUPFAKE    4
#define SCROLLWHEELDOWNFAKE  5
#define SCROLLWHEELLEFTFAKE  6
#define SCROLLWHEELRIGHTFAKE 7

#define _APPLEWM_SERVER_
#include "applewmExt.h"
#include <X11/extensions/applewm.h>

/* FIXME: Abstract this better */
void QuartzModeEQInit(void);

int darwin_modifier_flags = 0;  // last known modifier state

#define FD_ADD_MAX 128
static int fd_add[FD_ADD_MAX];
int fd_add_count = 0;
static pthread_mutex_t fd_add_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t fd_add_ready_cond = PTHREAD_COND_INITIALIZER;
static pthread_t fd_add_tid = NULL;

static EventList *darwinEvents = NULL;

static pthread_mutex_t mieq_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mieq_ready_cond = PTHREAD_COND_INITIALIZER;

/*** Pthread Magics ***/
static pthread_t create_thread(void *func, void *arg) {
    pthread_attr_t attr;
    pthread_t tid;

    pthread_attr_init (&attr);
    pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
    pthread_create (&tid, &attr, func, arg);
    pthread_attr_destroy (&attr);

    return tid;
}

void darwinEvents_lock(void);
void darwinEvents_lock(void) {
    int err;
    if((err = pthread_mutex_lock(&mieq_lock))) {
        ErrorF("%s:%s:%d: Failed to lock mieq_lock: %d\n",
               __FILE__, __FUNCTION__, __LINE__, err);
        spewCallStack();
    }
    if(darwinEvents == NULL) {
        pthread_cond_wait(&mieq_ready_cond, &mieq_lock);
    }
}

void darwinEvents_unlock(void);
void darwinEvents_unlock(void) {
    int err;
    if((err = pthread_mutex_unlock(&mieq_lock))) {
        ErrorF("%s:%s:%d: Failed to unlock mieq_lock: %d\n",
               __FILE__, __FUNCTION__, __LINE__, err);
        spewCallStack();
    }
}

/*
 * DarwinPressModifierKey
 * Press or release the given modifier key (one of NX_MODIFIERKEY_* constants)
 */
static void DarwinPressModifierKey(int pressed, int key) {
    int keycode = DarwinModifierNXKeyToNXKeycode(key, 0);

    if (keycode == 0) {
        ErrorF("DarwinPressModifierKey bad keycode: key=%d\n", key);
        return;
    }

    DarwinSendKeyboardEvents(pressed, keycode);
}

/*
 * DarwinUpdateModifiers
 *  Send events to update the modifier state.
 */

int darwin_modifier_mask_list[] = {
#ifdef NX_DEVICELCMDKEYMASK
    NX_DEVICELCTLKEYMASK, NX_DEVICERCTLKEYMASK,
    NX_DEVICELSHIFTKEYMASK, NX_DEVICERSHIFTKEYMASK,
    NX_DEVICELCMDKEYMASK, NX_DEVICERCMDKEYMASK,
    NX_DEVICELALTKEYMASK, NX_DEVICERALTKEYMASK,
#else
    NX_CONTROLMASK, NX_SHIFTMASK, NX_COMMANDMASK, NX_ALTERNATEMASK,
#endif
    NX_ALPHASHIFTMASK,
    0
};

static void DarwinUpdateModifiers(
    int pressed,        // KeyPress or KeyRelease
    int flags )         // modifier flags that have changed
{
    int *f;
    int key;

    /* Capslock is special.  This mask is the state of capslock (on/off),
     * not the state of the button.  Hopefully we can find a better solution.
     */
    if(NX_ALPHASHIFTMASK & flags) {
        DarwinPressModifierKey(KeyPress, NX_MODIFIERKEY_ALPHALOCK);
        DarwinPressModifierKey(KeyRelease, NX_MODIFIERKEY_ALPHALOCK);
    }
    
    for(f=darwin_modifier_mask_list; *f; f++)
        if(*f & flags && *f != NX_ALPHASHIFTMASK) {
            key = DarwinModifierNXMaskToNXKey(*f);
            if(key == -1)
                ErrorF("DarwinUpdateModifiers: Unsupported NXMask: 0x%x\n", *f);
            else
                DarwinPressModifierKey(pressed, key);
        }
}

/* Generic handler for Xquartz-specifc events.  When possible, these should
   be moved into their own individual functions and set as handlers using
   mieqSetHandler. */

static void DarwinEventHandler(int screenNum, xEventPtr xe, DeviceIntPtr dev, int nevents) {
    int i;
    
    TA_SERVER();

//    DEBUG_LOG("DarwinEventHandler(%d, %p, %p, %d)\n", screenNum, xe, dev, nevents);
    for (i=0; i<nevents; i++) {
        switch(xe[i].u.u.type) {
            case kXquartzControllerNotify:
                DEBUG_LOG("kXquartzControllerNotify\n");
                AppleWMSendEvent(AppleWMControllerNotify,
                                 AppleWMControllerNotifyMask,
                                 xe[i].u.clientMessage.u.l.longs0,
                                 xe[i].u.clientMessage.u.l.longs1);
                break;
                
            case kXquartzPasteboardNotify:
                DEBUG_LOG("kXquartzPasteboardNotify\n");
                AppleWMSendEvent(AppleWMPasteboardNotify,
                                 AppleWMPasteboardNotifyMask,
                                 xe[i].u.clientMessage.u.l.longs0,
                                 xe[i].u.clientMessage.u.l.longs1);
                break;
                
            case kXquartzActivate:
                DEBUG_LOG("kXquartzActivate\n");
                QuartzShow(xe[i].u.keyButtonPointer.rootX,
                           xe[i].u.keyButtonPointer.rootY);
                AppleWMSendEvent(AppleWMActivationNotify,
                                 AppleWMActivationNotifyMask,
                                 AppleWMIsActive, 0);
                break;
                
            case kXquartzDeactivate:
                DEBUG_LOG("kXquartzDeactivate\n");
                AppleWMSendEvent(AppleWMActivationNotify,
                                 AppleWMActivationNotifyMask,
                                 AppleWMIsInactive, 0);
                QuartzHide();
                break;

            case kXquartzReloadPreferences:
                DEBUG_LOG("kXquartzReloadPreferences\n");
                AppleWMSendEvent(AppleWMActivationNotify,
                                 AppleWMActivationNotifyMask,
                                 AppleWMReloadPreferences, 0);
                break;
                
            case kXquartzToggleFullscreen:
                DEBUG_LOG("kXquartzToggleFullscreen\n");
                if (quartzEnableRootless) 
                    QuartzSetFullscreen(!quartzHasRoot);
                else if (quartzHasRoot)
                    QuartzHide();
                else
                    QuartzShow(xe[i].u.keyButtonPointer.rootX,
                               xe[i].u.keyButtonPointer.rootY);
                break;
                
            case kXquartzSetRootless:
                DEBUG_LOG("kXquartzSetRootless\n");
                QuartzSetRootless(xe[i].u.clientMessage.u.l.longs0);
                if (!quartzEnableRootless && !quartzHasRoot)
                    QuartzHide();
                break;
                
            case kXquartzSetRootClip:
                QuartzSetRootClip((Bool)xe[i].u.clientMessage.u.l.longs0);
                break;
                
            case kXquartzQuit:
                GiveUp(0);
                break;
                
            case kXquartzSpaceChanged:
                DEBUG_LOG("kXquartzSpaceChanged\n");
                QuartzSpaceChanged(xe[i].u.clientMessage.u.l.longs0);
                break;

            default:
                ErrorF("Unknown application defined event type %d.\n", xe[i].u.u.type);
		}	
    }
}

void DarwinListenOnOpenFD(int fd) {
    ErrorF("DarwinListenOnOpenFD: %d\n", fd);
    
    pthread_mutex_lock(&fd_add_lock);
    if(fd_add_count < FD_ADD_MAX)
        fd_add[fd_add_count++] = fd;
    else
        ErrorF("FD Addition buffer at max.  Dropping fd addition request.\n");

    pthread_cond_broadcast(&fd_add_ready_cond);
    pthread_mutex_unlock(&fd_add_lock);
}

static void DarwinProcessFDAdditionQueue_thread(void *args) {
    pthread_mutex_lock(&fd_add_lock);
    while(true) {
        while(fd_add_count) {
            DarwinSendDDXEvent(kXquartzListenOnOpenFD, 1, fd_add[--fd_add_count]);
        }
        pthread_cond_wait(&fd_add_ready_cond, &fd_add_lock);
    }
}

static void kXquartzListenOnOpenFDHandler(int screenNum, xEventPtr xe, DeviceIntPtr dev, int nevents) {
    size_t i;
    TA_SERVER();
    
    for (i=0; i<nevents; i++) {
        ErrorF("Calling ListenOnOpenFD() for new fd: %d\n", (int)xe[i].u.clientMessage.u.l.longs0);
        ListenOnOpenFD((int)xe[i].u.clientMessage.u.l.longs0, 1);
    }
}

Bool DarwinEQInit(void) { 
    mieqInit();
    mieqSetHandler(kXquartzReloadKeymap, DarwinKeyboardReloadHandler);
    mieqSetHandler(kXquartzActivate, DarwinEventHandler);
    mieqSetHandler(kXquartzDeactivate, DarwinEventHandler);
    mieqSetHandler(kXquartzReloadPreferences, DarwinEventHandler);
    mieqSetHandler(kXquartzSetRootClip, DarwinEventHandler);
    mieqSetHandler(kXquartzQuit, DarwinEventHandler);
    mieqSetHandler(kXquartzReadPasteboard, QuartzReadPasteboard);
    mieqSetHandler(kXquartzWritePasteboard, QuartzWritePasteboard);
    mieqSetHandler(kXquartzToggleFullscreen, DarwinEventHandler);
    mieqSetHandler(kXquartzSetRootless, DarwinEventHandler);
    mieqSetHandler(kXquartzSpaceChanged, DarwinEventHandler);
    mieqSetHandler(kXquartzControllerNotify, DarwinEventHandler);
    mieqSetHandler(kXquartzPasteboardNotify, DarwinEventHandler);
    mieqSetHandler(kXquartzDisplayChanged, QuartzDisplayChangedHandler);
    mieqSetHandler(kXquartzListenOnOpenFD, kXquartzListenOnOpenFDHandler);
    
    QuartzModeEQInit();

    /* Note that this *could* cause a potential async issue, since we're checking
     * darwinEvents without holding the lock, but darwinEvents is only ever set
     * here, so I don't bother.
     */
    if (!darwinEvents) {
        darwinEvents = InitEventList(GetMaximumEventsNum());;
        
        if (!darwinEvents)
            FatalError("Couldn't allocate event buffer\n");
        
        darwinEvents_lock();
        pthread_cond_broadcast(&mieq_ready_cond);
        darwinEvents_unlock();
    }

    if(!fd_add_tid)
        fd_add_tid = create_thread(DarwinProcessFDAdditionQueue_thread, NULL);
    
    return TRUE;
}

/*
 * ProcessInputEvents
 *  Read and process events from the event queue until it is empty.
 */
void ProcessInputEvents(void) {
    xEvent  xe;
	int x = sizeof(xe);
    
    TA_SERVER();

    mieqProcessInputEvents();

    // Empty the signaling pipe
    while (x == sizeof(xe)) {
      x = read(darwinEventReadFD, &xe, sizeof(xe));
    }
}

/* Sends a null byte down darwinEventWriteFD, which will cause the
   Dispatch() event loop to check out event queue */
static void DarwinPokeEQ(void) {
	char nullbyte=0;
	//  <daniels> oh, i ... er ... christ.
	write(darwinEventWriteFD, &nullbyte, 1);
}

/* Convert from Appkit pointer input values to X input values:
 * Note: pointer_x and pointer_y are relative to the upper-left of primary
 *       display.
 */
static void DarwinPrepareValuators(DeviceIntPtr pDev, int *valuators, ScreenPtr screen,
                                   float pointer_x, float pointer_y, 
                                   float pressure, float tilt_x, float tilt_y) {
    /* Fix offset between darwin and X screens */
    pointer_x -= darwinMainScreenX + dixScreenOrigins[screen->myNum].x;
    pointer_y -= darwinMainScreenY + dixScreenOrigins[screen->myNum].y;

    if(pointer_x < 0.0)
        pointer_x = 0.0;

    if(pointer_y < 0.0)
        pointer_y = 0.0;
    
    if(pDev == darwinPointer) {
        valuators[0] = pointer_x;
        valuators[1] = pointer_y;
        valuators[2] = 0;
        valuators[3] = 0;
        valuators[4] = 0;
    } else {
        /* Setup our array of values */
        valuators[0] = XQUARTZ_VALUATOR_LIMIT * (pointer_x / (float)screenInfo.screens[0]->width);
        valuators[1] = XQUARTZ_VALUATOR_LIMIT * (pointer_y / (float)screenInfo.screens[0]->height);
        valuators[2] = XQUARTZ_VALUATOR_LIMIT * pressure;
        valuators[3] = XQUARTZ_VALUATOR_LIMIT * tilt_x;
        valuators[4] = XQUARTZ_VALUATOR_LIMIT * tilt_y;
    }
    //DEBUG_LOG("Pointer (%f, %f), Valuators: {%d,%d,%d,%d,%d}\n", pointer_x, pointer_y,
    //          valuators[0], valuators[1], valuators[2], valuators[3], valuators[4]);
}

void DarwinSendPointerEvents(DeviceIntPtr pDev, int ev_type, int ev_button, float pointer_x, float pointer_y, 
			     float pressure, float tilt_x, float tilt_y) {
	static int darwinFakeMouseButtonDown = 0;
	int i, num_events;
    ScreenPtr screen;
    int valuators[5];
	
    //DEBUG_LOG("x=%f, y=%f, p=%f, tx=%f, ty=%f\n", pointer_x, pointer_y, pressure, tilt_x, tilt_y);
    
	if(!darwinEvents) {
		DEBUG_LOG("DarwinSendPointerEvents called before darwinEvents was initialized\n");
		return;
	}

    screen = miPointerGetScreen(pDev);
    if(!screen) {
        DEBUG_LOG("DarwinSendPointerEvents called before screen was initialized\n");
        return;
    }

    /* Handle fake click */
	if (ev_type == ButtonPress && darwinFakeButtons && ev_button == 1) {
        if(darwinFakeMouseButtonDown != 0) {
            /* We're currently "down" with another button, so release it first */
            DarwinSendPointerEvents(pDev, ButtonRelease, darwinFakeMouseButtonDown, pointer_x, pointer_y, pressure, tilt_x, tilt_y);
            darwinFakeMouseButtonDown=0;
        }
		if (darwin_modifier_flags & darwinFakeMouse2Mask) {
            ev_button = 2;
			darwinFakeMouseButtonDown = 2;
            DarwinUpdateModKeys(darwin_modifier_flags & ~darwinFakeMouse2Mask);
		} else if (darwin_modifier_flags & darwinFakeMouse3Mask) {
            ev_button = 3;
			darwinFakeMouseButtonDown = 3;
            DarwinUpdateModKeys(darwin_modifier_flags & ~darwinFakeMouse3Mask);
		}
	}

	if (ev_type == ButtonRelease && ev_button == 1) {
        if(darwinFakeMouseButtonDown) {
            ev_button = darwinFakeMouseButtonDown;
        }

        if(darwinFakeMouseButtonDown == 2) {
            DarwinUpdateModKeys(darwin_modifier_flags & ~darwinFakeMouse2Mask);
        } else if(darwinFakeMouseButtonDown == 3) {
            DarwinUpdateModKeys(darwin_modifier_flags & ~darwinFakeMouse3Mask);
        }

        darwinFakeMouseButtonDown = 0;
	}

    DarwinPrepareValuators(pDev, valuators, screen, pointer_x, pointer_y, pressure, tilt_x, tilt_y);
    darwinEvents_lock(); {
        num_events = GetPointerEvents(darwinEvents, pDev, ev_type, ev_button, 
                                      POINTER_ABSOLUTE, 0, pDev==darwinTabletCurrent?5:2, valuators);
        for(i=0; i<num_events; i++) mieqEnqueue (pDev, darwinEvents[i].event);
        DarwinPokeEQ();
    } darwinEvents_unlock();
}

void DarwinSendKeyboardEvents(int ev_type, int keycode) {
	int i, num_events;

	if(!darwinEvents) {
		DEBUG_LOG("DarwinSendKeyboardEvents called before darwinEvents was initialized\n");
		return;
	}

    darwinEvents_lock(); {
        num_events = GetKeyboardEvents(darwinEvents, darwinKeyboard, ev_type, keycode + MIN_KEYCODE);
        for(i=0; i<num_events; i++) mieqEnqueue(darwinKeyboard,darwinEvents[i].event);
        DarwinPokeEQ();
    } darwinEvents_unlock();
}

void DarwinSendProximityEvents(int ev_type, float pointer_x, float pointer_y) {
	int i, num_events;
    ScreenPtr screen;
    DeviceIntPtr pDev = darwinTabletCurrent;
    int valuators[5];

	DEBUG_LOG("DarwinSendProximityEvents(%d, %f, %f)\n", ev_type, pointer_x, pointer_y);

	if(!darwinEvents) {
		DEBUG_LOG("DarwinSendProximityEvents called before darwinEvents was initialized\n");
		return;
	}
    
    screen = miPointerGetScreen(pDev);
    if(!screen) {
        DEBUG_LOG("DarwinSendPointerEvents called before screen was initialized\n");
        return;
    }    

    DarwinPrepareValuators(pDev, valuators, screen, pointer_x, pointer_y, 0.0f, 0.0f, 0.0f);
    darwinEvents_lock(); {
        num_events = GetProximityEvents(darwinEvents, pDev, ev_type,
                                        0, 5, valuators);
        for(i=0; i<num_events; i++) mieqEnqueue (pDev,darwinEvents[i].event);
        DarwinPokeEQ();
    } darwinEvents_unlock();
}


/* Send the appropriate number of button clicks to emulate scroll wheel */
void DarwinSendScrollEvents(float count_x, float count_y, 
							float pointer_x, float pointer_y, 
			    			float pressure, float tilt_x, float tilt_y) {
	if(!darwinEvents) {
		DEBUG_LOG("DarwinSendScrollEvents called before darwinEvents was initialized\n");
		return;
	}

	int sign_x = count_x > 0.0f ? SCROLLWHEELLEFTFAKE : SCROLLWHEELRIGHTFAKE;
	int sign_y = count_y > 0.0f ? SCROLLWHEELUPFAKE : SCROLLWHEELDOWNFAKE;
	count_x = fabs(count_x);
	count_y = fabs(count_y);
	
	while ((count_x > 0.0f) || (count_y > 0.0f)) {
		if (count_x > 0.0f) {
			DarwinSendPointerEvents(darwinPointer, ButtonPress, sign_x, pointer_x, pointer_y, pressure, tilt_x, tilt_y);
			DarwinSendPointerEvents(darwinPointer, ButtonRelease, sign_x, pointer_x, pointer_y, pressure, tilt_x, tilt_y);
			count_x = count_x - 1.0f;
		}
		if (count_y > 0.0f) {
			DarwinSendPointerEvents(darwinPointer, ButtonPress, sign_y, pointer_x, pointer_y, pressure, tilt_x, tilt_y);
			DarwinSendPointerEvents(darwinPointer, ButtonRelease, sign_y, pointer_x, pointer_y, pressure, tilt_x, tilt_y);
			count_y = count_y - 1.0f;
		}
	}
}

/* Send the appropriate KeyPress/KeyRelease events to GetKeyboardEvents to
   reflect changing modifier flags (alt, control, meta, etc) */
void DarwinUpdateModKeys(int flags) {
	DarwinUpdateModifiers(KeyRelease, darwin_modifier_flags & ~flags);
	DarwinUpdateModifiers(KeyPress, ~darwin_modifier_flags & flags);
	darwin_modifier_flags = flags;
}

/*
 * DarwinSendDDXEvent
 *  Send the X server thread a message by placing it on the event queue.
 */
void DarwinSendDDXEvent(int type, int argc, ...) {
    xEvent xe;
    INT32 *argv;
    int i, max_args;
    va_list args;

    memset(&xe, 0, sizeof(xe));
    xe.u.u.type = type;
    xe.u.clientMessage.u.l.type = type;

    argv = &xe.u.clientMessage.u.l.longs0;
    max_args = 4;

    if (argc > 0 && argc <= max_args) {
        va_start (args, argc);
        for (i = 0; i < argc; i++)
            argv[i] = (int) va_arg (args, int);
        va_end (args);
    }

    darwinEvents_lock(); {
        mieqEnqueue(darwinPointer, &xe);
        DarwinPokeEQ();
    } darwinEvents_unlock();
}
