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

#include   <X11/X.h>
#include   <X11/Xmd.h>
#include   <X11/Xproto.h>
#include   "misc.h"
#include   "windowstr.h"
#include   "pixmapstr.h"
#include   "inputstr.h"
#include   "inpututils.h"
#include   "eventstr.h"
#include   "mi.h"
#include   "scrnintstr.h"
#include   "mipointer.h"
#include   "os.h"

#include "darwin.h"
#include "quartz.h"
#include "quartzKeyboard.h"
#include "quartzRandR.h"
#include "darwinEvents.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include <IOKit/hidsystem/IOLLEvent.h>

/* Fake button press/release for scroll wheel move. */
#define SCROLLWHEELUPFAKE    4
#define SCROLLWHEELDOWNFAKE  5
#define SCROLLWHEELLEFTFAKE  6
#define SCROLLWHEELRIGHTFAKE 7

#include <X11/extensions/applewmconst.h>
#include "applewmExt.h"

/* FIXME: Abstract this better */
extern Bool QuartzModeEventHandler(int screenNum, XQuartzEvent *e, DeviceIntPtr dev);

int darwin_all_modifier_flags = 0;  // last known modifier state
int darwin_all_modifier_mask = 0;
int darwin_x11_modifier_mask = 0;

#define FD_ADD_MAX 128
static int fd_add[FD_ADD_MAX];
int fd_add_count = 0;
static pthread_mutex_t fd_add_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t fd_add_ready_cond = PTHREAD_COND_INITIALIZER;
static pthread_t fd_add_tid = NULL;

static InternalEvent* darwinEvents = NULL;

static pthread_mutex_t mieq_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mieq_ready_cond = PTHREAD_COND_INITIALIZER;

/*** Pthread Magics ***/
static pthread_t create_thread(void *(*func)(void *), void *arg) {
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
        xorg_backtrace();
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
        xorg_backtrace();
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

static int darwin_x11_modifier_mask_list[] = {
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

static int darwin_all_modifier_mask_additions[] = { NX_SECONDARYFNMASK, };

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
    
    for(f=darwin_x11_modifier_mask_list; *f; f++)
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

static void DarwinEventHandler(int screenNum, InternalEvent *ie, DeviceIntPtr dev) {
    XQuartzEvent *e = &(ie->xquartz_event);

    switch(e->subtype) {
        case kXquartzControllerNotify:
            DEBUG_LOG("kXquartzControllerNotify\n");
            AppleWMSendEvent(AppleWMControllerNotify,
                             AppleWMControllerNotifyMask,
                             e->data[0],
                             e->data[1]);
            break;
            
        case kXquartzPasteboardNotify:
            DEBUG_LOG("kXquartzPasteboardNotify\n");
            AppleWMSendEvent(AppleWMPasteboardNotify,
                             AppleWMPasteboardNotifyMask,
                             e->data[0],
                             e->data[1]);
            break;
            
        case kXquartzActivate:
            DEBUG_LOG("kXquartzActivate\n");
            QuartzShow();
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
            if(XQuartzIsRootless)
                ErrorF("Ignoring kXquartzToggleFullscreen because of rootless mode.");
            else 
                QuartzRandRToggleFullscreen();
            break;
            
        case kXquartzSetRootless:
            DEBUG_LOG("kXquartzSetRootless\n");
            if(e->data[0]) {
                QuartzRandRSetFakeRootless();
            } else {
                QuartzRandRSetFakeFullscreen(FALSE);
            }
            break;
            
        case kXquartzSetRootClip:
            QuartzSetRootClip((Bool)e->data[0]);
            break;
            
        case kXquartzQuit:
            GiveUp(0);
            break;
            
        case kXquartzSpaceChanged:
            DEBUG_LOG("kXquartzSpaceChanged\n");
            QuartzSpaceChanged(e->data[0]);
            break;

        case kXquartzListenOnOpenFD:
            ErrorF("Calling ListenOnOpenFD() for new fd: %d\n", (int)e->data[0]);
            ListenOnOpenFD((int)e->data[0], 1);
            break;
            
        case kXquartzReloadKeymap:
            DarwinKeyboardReloadHandler();
            break;
            
        case kXquartzDisplayChanged:
            DEBUG_LOG("kXquartzDisplayChanged\n");
            QuartzUpdateScreens();

            /* Update our RandR info */
            QuartzRandRUpdateFakeModes(TRUE);
            break;
            
        default:
            if(!QuartzModeEventHandler(screenNum, e, dev))
                ErrorF("Unknown application defined event type %d.\n", e->subtype);
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

static void *DarwinProcessFDAdditionQueue_thread(void *args) {
    /* TODO: Possibly adjust this to no longer be a race... maybe trigger this
     *       once a client connects and claims to be the WM.
     *
     * From ajax:
     * There's already an internal callback chain for setting selection [in 1.5]
     * ownership.  See the CallSelectionCallback at the bottom of
     * ProcSetSelectionOwner, and xfixes/select.c for an example of how to hook
     * into it.
     */

    struct timespec sleep_for;
    struct timespec sleep_remaining;

    sleep_for.tv_sec = 3;
    sleep_for.tv_nsec = 0;

    ErrorF("X11.app: DarwinProcessFDAdditionQueue_thread: Sleeping to allow xinitrc to catchup.\n");
    while(nanosleep(&sleep_for, &sleep_remaining) != 0) {
        sleep_for = sleep_remaining;
    }

    pthread_mutex_lock(&fd_add_lock);
    while(true) {
        while(fd_add_count) {
            DarwinSendDDXEvent(kXquartzListenOnOpenFD, 1, fd_add[--fd_add_count]);
        }
        pthread_cond_wait(&fd_add_ready_cond, &fd_add_lock);
    }

    return NULL;
}

Bool DarwinEQInit(void) { 
    int *p;

    for(p=darwin_x11_modifier_mask_list, darwin_all_modifier_mask=0; *p; p++) {
        darwin_x11_modifier_mask |= *p;
    }
    
    for(p=darwin_all_modifier_mask_additions, darwin_all_modifier_mask= darwin_x11_modifier_mask; *p; p++) {
        darwin_all_modifier_mask |= *p;
    }
    
    mieqInit();
    mieqSetHandler(ET_XQuartz, DarwinEventHandler);

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

void DarwinEQFini(void) {
    mieqFini();
}

/*
 * ProcessInputEvents
 *  Read and process events from the event queue until it is empty.
 */
void ProcessInputEvents(void) {
    char nullbyte;
	int x = sizeof(nullbyte);
    
    mieqProcessInputEvents();

    // Empty the signaling pipe
    while (x == sizeof(nullbyte)) {
      x = read(darwinEventReadFD, &nullbyte, sizeof(nullbyte));
    }
}

/* Sends a null byte down darwinEventWriteFD, which will cause the
   Dispatch() event loop to check out event queue */
static void DarwinPokeEQ(void) {
	char nullbyte=0;
	//  <daniels> oh, i ... er ... christ.
	write(darwinEventWriteFD, &nullbyte, sizeof(nullbyte));
}

/* Convert from Appkit pointer input values to X input values:
 * Note: pointer_x and pointer_y are relative to the upper-left of primary
 *       display.
 */
static void DarwinPrepareValuators(DeviceIntPtr pDev, int *valuators, ScreenPtr screen,
                                   float pointer_x, float pointer_y, 
                                   float pressure, float tilt_x, float tilt_y) {
    /* Fix offset between darwin and X screens */
    pointer_x -= darwinMainScreenX + screen->x;
    pointer_y -= darwinMainScreenY + screen->y;

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
		if (darwin_all_modifier_flags & darwinFakeMouse2Mask) {
            ev_button = 2;
			darwinFakeMouseButtonDown = 2;
            DarwinUpdateModKeys(darwin_all_modifier_flags & ~darwinFakeMouse2Mask);
		} else if (darwin_all_modifier_flags & darwinFakeMouse3Mask) {
            ev_button = 3;
			darwinFakeMouseButtonDown = 3;
            DarwinUpdateModKeys(darwin_all_modifier_flags & ~darwinFakeMouse3Mask);
		}
	}

	if (ev_type == ButtonRelease && ev_button == 1) {
        if(darwinFakeMouseButtonDown) {
            ev_button = darwinFakeMouseButtonDown;
        }

        if(darwinFakeMouseButtonDown == 2) {
            DarwinUpdateModKeys(darwin_all_modifier_flags & ~darwinFakeMouse2Mask);
        } else if(darwinFakeMouseButtonDown == 3) {
            DarwinUpdateModKeys(darwin_all_modifier_flags & ~darwinFakeMouse3Mask);
        }

        darwinFakeMouseButtonDown = 0;
	}

    DarwinPrepareValuators(pDev, valuators, screen, pointer_x, pointer_y, pressure, tilt_x, tilt_y);
    darwinEvents_lock(); {
        ValuatorMask mask;
        valuator_mask_set_range(&mask, 0, (pDev == darwinPointer) ? 2 : 5, valuators);
        QueuePointerEvents(pDev, ev_type, ev_button, POINTER_ABSOLUTE, &mask);
        DarwinPokeEQ();
    } darwinEvents_unlock();
}

void DarwinSendKeyboardEvents(int ev_type, int keycode) {

	if(!darwinEvents) {
		DEBUG_LOG("DarwinSendKeyboardEvents called before darwinEvents was initialized\n");
		return;
	}

    darwinEvents_lock(); {
        QueueKeyboardEvents(darwinKeyboard, ev_type, keycode + MIN_KEYCODE, NULL);
        DarwinPokeEQ();
    } darwinEvents_unlock();
}

void DarwinSendProximityEvents(DeviceIntPtr pDev, int ev_type, float pointer_x, float pointer_y,  
                               float pressure, float tilt_x, float tilt_y) {
    ScreenPtr screen;
    int valuators[5];

    DEBUG_LOG("DarwinSendProximityEvents: %d l:%f,%f p:%f t:%f,%f\n", ev_type, pointer_x, pointer_y, pressure, tilt_x, tilt_y);

    if(!darwinEvents) {
        DEBUG_LOG("DarwinSendProximityEvents called before darwinEvents was initialized\n");
        return;
    }
    
    screen = miPointerGetScreen(pDev);
    if(!screen) {
        DEBUG_LOG("DarwinSendPointerEvents called before screen was initialized\n");
        return;
    }    

    DarwinPrepareValuators(pDev, valuators, screen, pointer_x, pointer_y, pressure, tilt_x, tilt_y);
    darwinEvents_lock(); {
        ValuatorMask mask;
        valuator_mask_set_range(&mask, 0, 5, valuators);
        QueueProximityEvents(pDev, ev_type, &mask);
        DarwinPokeEQ();
    } darwinEvents_unlock();
}


/* Send the appropriate number of button clicks to emulate scroll wheel */
void DarwinSendScrollEvents(float count_x, float count_y, 
							float pointer_x, float pointer_y, 
			    			float pressure, float tilt_x, float tilt_y) {
	int sign_x, sign_y;
	if(!darwinEvents) {
		DEBUG_LOG("DarwinSendScrollEvents called before darwinEvents was initialized\n");
		return;
	}

	sign_x = count_x > 0.0f ? SCROLLWHEELLEFTFAKE : SCROLLWHEELRIGHTFAKE;
	sign_y = count_y > 0.0f ? SCROLLWHEELUPFAKE : SCROLLWHEELDOWNFAKE;
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
	DarwinUpdateModifiers(KeyRelease, darwin_all_modifier_flags & ~flags & darwin_x11_modifier_mask);
	DarwinUpdateModifiers(KeyPress, ~darwin_all_modifier_flags & flags & darwin_x11_modifier_mask);
	darwin_all_modifier_flags = flags;
}

/*
 * DarwinSendDDXEvent
 *  Send the X server thread a message by placing it on the event queue.
 */
void DarwinSendDDXEvent(int type, int argc, ...) {
    XQuartzEvent e;
    int i;
    va_list args;

    memset(&e, 0, sizeof(e));
    e.header = ET_Internal;
    e.type = ET_XQuartz;
    e.length = sizeof(e);
    e.time = GetTimeInMillis();
    e.subtype = type;

    if (argc > 0 && argc < XQUARTZ_EVENT_MAXARGS) {
        va_start (args, argc);
        for (i = 0; i < argc; i++)
            e.data[i] = (uint32_t) va_arg (args, uint32_t);
        va_end (args);
    }

    darwinEvents_lock(); {
        mieqEnqueue(NULL, (InternalEvent*)&e);
        DarwinPokeEQ();
    } darwinEvents_unlock();
}
