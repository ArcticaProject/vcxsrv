/*
Darwin event queue and event handling

Copyright 2007 Apple Inc.
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

#include "darwin.h"
#include "quartz.h"
#include "darwinKeyboard.h"
#include "darwinEvents.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <IOKit/hidsystem/IOLLEvent.h>

/* Fake button press/release for scroll wheel move. */
#define SCROLLWHEELUPFAKE   4
#define SCROLLWHEELDOWNFAKE 5

#define QUEUE_SIZE 256

typedef struct _Event {
    xEvent      event;
    ScreenPtr   pScreen;
} EventRec, *EventPtr;

int input_check_zero, input_check_flag;

static int old_flags = 0;  // last known modifier state

typedef struct _EventQueue {
    HWEventQueueType    head, tail; /* long for SetInputCheck */
    CARD32      lastEventTime;      /* to avoid time running backwards */
    Bool        lastMotion;
    EventRec    events[QUEUE_SIZE]; /* static allocation for signals */
    DevicePtr   pKbd, pPtr;         /* device pointer, to get funcs */
    ScreenPtr   pEnqueueScreen;     /* screen events are being delivered to */
    ScreenPtr   pDequeueScreen;     /* screen events are being dispatched to */
} EventQueueRec, *EventQueuePtr;

static EventQueueRec darwinEventQueue;
xEvent *darwinEvents;

/*
 * DarwinPressModifierMask
 *  Press or release the given modifier key, specified by its mask.
 */
static void DarwinPressModifierMask(
    int pressed,				    
    int mask)       // one of NX_*MASK constants
{
    int key = DarwinModifierNXMaskToNXKey(mask);

    if (key != -1) {
        int keycode = DarwinModifierNXKeyToNXKeycode(key, 0);
        if (keycode != 0)
	  DarwinSendKeyboardEvents(pressed, keycode);
    }
}

#ifdef NX_DEVICELCTLKEYMASK
#define CONTROL_MASK(flags) (flags & (NX_DEVICELCTLKEYMASK|NX_DEVICERCTLKEYMASK))
#else
#define CONTROL_MASK(flags) (NX_CONTROLMASK)
#endif /* NX_DEVICELCTLKEYMASK */

#ifdef NX_DEVICELSHIFTKEYMASK
#define SHIFT_MASK(flags) (flags & (NX_DEVICELSHIFTKEYMASK|NX_DEVICERSHIFTKEYMASK))
#else
#define SHIFT_MASK(flags) (NX_SHIFTMASK)
#endif /* NX_DEVICELSHIFTKEYMASK */

#ifdef NX_DEVICELCMDKEYMASK
#define COMMAND_MASK(flags) (flags & (NX_DEVICELCMDKEYMASK|NX_DEVICERCMDKEYMASK))
#else
#define COMMAND_MASK(flags) (NX_COMMANDMASK)
#endif /* NX_DEVICELCMDKEYMASK */

#ifdef NX_DEVICELALTKEYMASK
#define ALTERNATE_MASK(flags) (flags & (NX_DEVICELALTKEYMASK|NX_DEVICERALTKEYMASK))
#else
#define ALTERNATE_MASK(flags) (NX_ALTERNATEMASK)
#endif /* NX_DEVICELALTKEYMASK */

/*
 * DarwinUpdateModifiers
 *  Send events to update the modifier state.
 */
static void DarwinUpdateModifiers(
    int pressed,        // KeyPress or KeyRelease
    int flags )         // modifier flags that have changed
{
    if (flags & NX_ALPHASHIFTMASK) {
        DarwinPressModifierMask(pressed, NX_ALPHASHIFTMASK);
    }
    if (flags & NX_COMMANDMASK) {
        DarwinPressModifierMask(pressed, COMMAND_MASK(flags));
    }
    if (flags & NX_CONTROLMASK) {
        DarwinPressModifierMask(pressed, CONTROL_MASK(flags));
    }
    if (flags & NX_ALTERNATEMASK) {
        DarwinPressModifierMask(pressed, ALTERNATE_MASK(flags));
    }
    if (flags & NX_SHIFTMASK) {
        DarwinPressModifierMask(pressed, SHIFT_MASK(flags));
    }
    if (flags & NX_SECONDARYFNMASK) {
        DarwinPressModifierMask(pressed, NX_SECONDARYFNMASK);
    }
}

/*
 * DarwinReleaseModifiers
 * This hacky function releases all modifier keys.  It should be called when X11.app
 * is deactivated (kXDarwinDeactivate) to prevent modifiers from getting stuck if they
 * are held down during a "context" switch -- otherwise, we would miss the KeyUp.
 */
static void DarwinReleaseModifiers(void) {
	DarwinUpdateModifiers(KeyRelease, COMMAND_MASK(-1) | CONTROL_MASK(-1) | ALTERNATE_MASK(-1) | SHIFT_MASK(-1));
}

/*
 * DarwinSimulateMouseClick
 *  Send a mouse click to X when multiple mouse buttons are simulated
 *  with modifier-clicks, such as command-click for button 2. The dix
 *  layer is told that the previously pressed modifier key(s) are
 *  released, the simulated click event is sent. After the mouse button
 *  is released, the modifier keys are reverted to their actual state,
 *  which may or may not be pressed at that point. This is usually
 *  closest to what the user wants. Ie. the user typically wants to
 *  simulate a button 2 press instead of Command-button 2.
 */
static void DarwinSimulateMouseClick(
    int pointer_x,
    int pointer_y,
    int whichButton,    // mouse button to be pressed
    int modifierMask)   // modifiers used for the fake click
{
    // first fool X into forgetting about the keys
	// for some reason, it's not enough to tell X we released the Command key -- 
	// it has to be the *left* Command key.
	if (modifierMask & NX_COMMANDMASK) modifierMask |=NX_DEVICELCMDKEYMASK ;
    DarwinUpdateModifiers(KeyRelease, modifierMask);

    // push the mouse button
    DarwinSendPointerEvents(ButtonPress, whichButton, pointer_x, pointer_y);
    DarwinSendPointerEvents(ButtonRelease, whichButton, pointer_x, pointer_y);

    // restore old modifiers
    DarwinUpdateModifiers(KeyPress, modifierMask);
}


Bool DarwinEQInit(DevicePtr pKbd, DevicePtr pPtr) { 
    darwinEvents = (xEvent *)malloc(sizeof(xEvent) * GetMaximumEventsNum());
    mieqInit();
    darwinEventQueue.head = darwinEventQueue.tail = 0;
    darwinEventQueue.lastEventTime = GetTimeInMillis ();
    darwinEventQueue.pKbd = pKbd;
    darwinEventQueue.pPtr = pPtr;
    darwinEventQueue.pEnqueueScreen = screenInfo.screens[0];
    darwinEventQueue.pDequeueScreen = darwinEventQueue.pEnqueueScreen;
    SetInputCheck(&input_check_zero, &input_check_flag);
    return TRUE;
}


/*
 * DarwinEQEnqueue
 *  Must be thread safe with ProcessInputEvents.
 *    DarwinEQEnqueue    - called from event gathering thread
 *    ProcessInputEvents - called from X server thread
 *  DarwinEQEnqueue should never be called from more than one thread.
 * 
 * This should be deprecated in favor of miEQEnqueue -- BB
 */
void DarwinEQEnqueue(const xEventPtr e) {
    HWEventQueueType oldtail, newtail;

    oldtail = darwinEventQueue.tail;

    // mieqEnqueue() collapses successive motion events into one event.
    // This is difficult to do in a thread-safe way and rarely useful.

    newtail = oldtail + 1;
    if (newtail == QUEUE_SIZE) newtail = 0;
    /* Toss events which come in late */
    if (newtail == darwinEventQueue.head) return;

    darwinEventQueue.events[oldtail].event = *e;

    /*
     * Make sure that event times don't go backwards - this
     * is "unnecessary", but very useful
     */
    if (e->u.keyButtonPointer.time < darwinEventQueue.lastEventTime &&
        darwinEventQueue.lastEventTime - e->u.keyButtonPointer.time < 10000)
    {
        darwinEventQueue.events[oldtail].event.u.keyButtonPointer.time =
        darwinEventQueue.lastEventTime;
    }
    darwinEventQueue.events[oldtail].pScreen = darwinEventQueue.pEnqueueScreen;

    // Update the tail after the event is prepared
    darwinEventQueue.tail = newtail;

    // Signal there is an event ready to handle
    DarwinPokeEQ();
}


/*
 * DarwinEQPointerPost
 *  Post a pointer event. Used by the mipointer.c routines.
 */
void DarwinEQPointerPost(DeviceIntPtr pdev, xEventPtr e) {
    (*darwinEventQueue.pPtr->processInputProc)
            (e, (DeviceIntPtr)darwinEventQueue.pPtr, 1);
}


void DarwinEQSwitchScreen(ScreenPtr pScreen, Bool fromDIX) {
    darwinEventQueue.pEnqueueScreen = pScreen;
    if (fromDIX)
        darwinEventQueue.pDequeueScreen = pScreen;
}


/*
 * ProcessInputEvents
 *  Read and process events from the event queue until it is empty.
 */
void ProcessInputEvents(void) {
    EventRec *e;
    int     x, y;
    xEvent  xe;
    // button number and modifier mask of currently pressed fake button
    input_check_flag=0;

    //    ErrorF("calling mieqProcessInputEvents\n");
    mieqProcessInputEvents();

    // Empty the signaling pipe
    x = sizeof(xe);
    while (x == sizeof(xe)) 
        x = read(darwinEventReadFD, &xe, sizeof(xe));

    while (darwinEventQueue.head != darwinEventQueue.tail)
    {
        if (screenIsSaved == SCREEN_SAVER_ON)
            dixSaveScreens (serverClient, SCREEN_SAVER_OFF, ScreenSaverReset);

        e = &darwinEventQueue.events[darwinEventQueue.head];
        xe = e->event;

        // Shift from global screen coordinates to coordinates relative to
        // the origin of the current screen.
        xe.u.keyButtonPointer.rootX -= darwinMainScreenX +
                dixScreenOrigins[miPointerCurrentScreen()->myNum].x;
        xe.u.keyButtonPointer.rootY -= darwinMainScreenY +
                dixScreenOrigins[miPointerCurrentScreen()->myNum].y;
	
	/*	ErrorF("old rootX = (%d,%d) darwinMainScreen = (%d,%d) dixScreenOrigins[%d]=(%d,%d)\n",
	       xe.u.keyButtonPointer.rootX, xe.u.keyButtonPointer.rootY,
	       darwinMainScreenX, darwinMainScreenY,
	       miPointerCurrentScreen()->myNum,
	       dixScreenOrigins[miPointerCurrentScreen()->myNum].x,
	       dixScreenOrigins[miPointerCurrentScreen()->myNum].y); */

	//Assumption - screen switching can only occur on motion events

        if (e->pScreen != darwinEventQueue.pDequeueScreen)
        {
            darwinEventQueue.pDequeueScreen = e->pScreen;
            x = xe.u.keyButtonPointer.rootX;
            y = xe.u.keyButtonPointer.rootY;
            if (darwinEventQueue.head == QUEUE_SIZE - 1)
                darwinEventQueue.head = 0;
            else
                ++darwinEventQueue.head;
            NewCurrentScreen (darwinEventQueue.pDequeueScreen, x, y);
        }
        else
        {
            if (darwinEventQueue.head == QUEUE_SIZE - 1)
                darwinEventQueue.head = 0;
            else
                ++darwinEventQueue.head;
            switch (xe.u.u.type) {
            case KeyPress:
            case KeyRelease:
	      ErrorF("Unexpected Keyboard event in DarwinProcessInputEvents\n");
	      break;

            case ButtonPress:
	      ErrorF("Unexpected ButtonPress event in DarwinProcessInputEvents\n");
                break;

            case ButtonRelease:
	      ErrorF("Unexpected ButtonRelease event in DarwinProcessInputEvents\n");
                break;

            case MotionNotify:
	      ErrorF("Unexpected ButtonRelease event in DarwinProcessInputEvents\n");
                break;

            case kXDarwinUpdateModifiers:
	      ErrorF("Unexpected ButtonRelease event in DarwinProcessInputEvents\n");
	      break;

            case kXDarwinUpdateButtons:
	      ErrorF("Unexpected XDarwinScrollWheel event in DarwinProcessInputEvents\n");
	      break;

            case kXDarwinScrollWheel: 
	      ErrorF("Unexpected XDarwinScrollWheel event in DarwinProcessInputEvents\n");
	      break;

			case kXDarwinDeactivate:
				DarwinReleaseModifiers();
				// fall through
            default:
                // Check for mode specific event
                QuartzProcessEvent(&xe);
            }
        }
    }

    //    miPointerUpdate();
}

/* Sends a null byte down darwinEventWriteFD, which will cause the
   Dispatch() event loop to check out event queue */
void DarwinPokeEQ(void) {
  char nullbyte=0;
  input_check_flag++;
  //  <daniels> bushing: oh, i ... er ... christ.
  write(darwinEventWriteFD, &nullbyte, 1);
}

void DarwinSendPointerEvents(int ev_type, int ev_button, int pointer_x, int pointer_y) {
  static int darwinFakeMouseButtonDown = 0;
  static int darwinFakeMouseButtonMask = 0;
  int i, num_events;
  int valuators[2] = {pointer_x, pointer_y};
  if (ev_type == ButtonPress && darwinFakeButtons && ev_button == 1) {
    // Mimic multi-button mouse with modifier-clicks
    // If both sets of modifiers are pressed,
    // button 2 is clicked.
    if ((old_flags & darwinFakeMouse2Mask) == darwinFakeMouse2Mask) {
      DarwinSimulateMouseClick(pointer_x, pointer_y, 2, darwinFakeMouse2Mask);
      darwinFakeMouseButtonDown = 2;
      darwinFakeMouseButtonMask = darwinFakeMouse2Mask;
    } else if ((old_flags & darwinFakeMouse3Mask) == darwinFakeMouse3Mask) {
      DarwinSimulateMouseClick(pointer_x, pointer_y, 3, darwinFakeMouse3Mask);
      darwinFakeMouseButtonDown = 3;
      darwinFakeMouseButtonMask = darwinFakeMouse3Mask;
    }
  }
  if (ev_type == ButtonRelease && darwinFakeButtons && darwinFakeMouseButtonDown) {
    // If last mousedown was a fake click, don't check for
    // mouse modifiers here. The user may have released the
    // modifiers before the mouse button.
    ev_button = darwinFakeMouseButtonDown;
    darwinFakeMouseButtonDown = 0;
    // Bring modifiers back up to date
    DarwinUpdateModifiers(KeyPress, darwinFakeMouseButtonMask & old_flags);
    darwinFakeMouseButtonMask = 0;
  } 

  num_events = GetPointerEvents(darwinEvents, darwinPointer, ev_type, ev_button, 
				POINTER_ABSOLUTE, 0, 2, valuators);
      
  for(i=0; i<num_events; i++) mieqEnqueue (darwinPointer,&darwinEvents[i]);
  DarwinPokeEQ();
}

void DarwinSendKeyboardEvents(int ev_type, int keycode) {
  int i, num_events;
  if (old_flags == 0 && darwinSyncKeymap && darwinKeymapFile == NULL) {
    /* See if keymap has changed. */

    static unsigned int last_seed;
    unsigned int this_seed;

    this_seed = QuartzSystemKeymapSeed();
    if (this_seed != last_seed) {
      last_seed = this_seed;
      DarwinKeyboardReload(darwinKeyboard);
    }
  }

  num_events = GetKeyboardEvents(darwinEvents, darwinKeyboard, ev_type, keycode + MIN_KEYCODE);
  for(i=0; i<num_events; i++) mieqEnqueue(darwinKeyboard,&darwinEvents[i]);
  DarwinPokeEQ();
}

/* Send the appropriate number of button 4 / 5 clicks to emulate scroll wheel */
void DarwinSendScrollEvents(float count, int pointer_x, int pointer_y) {
  int i;
  int ev_button = count > 0.0f ? 4 : 5;
  int valuators[2] = {pointer_x, pointer_y};

  for (count = fabs(count); count > 0.0; count = count - 1.0f) {
    int num_events = GetPointerEvents(darwinEvents, darwinPointer, ButtonPress, ev_button, 
				      POINTER_ABSOLUTE, 0, 2, valuators);
    for(i=0; i<num_events; i++) mieqEnqueue(darwinPointer,&darwinEvents[i]);
    num_events = GetPointerEvents(darwinEvents, darwinPointer, ButtonRelease, ev_button, 
				      POINTER_ABSOLUTE, 0, 2, valuators);
    for(i=0; i<num_events; i++) mieqEnqueue(darwinPointer,&darwinEvents[i]);
  }
  DarwinPokeEQ();
}

/* Send the appropriate KeyPress/KeyRelease events to GetKeyboardEvents to
   reflect changing modifier flags (alt, control, meta, etc) */
void DarwinUpdateModKeys(int flags) {
  DarwinUpdateModifiers(KeyRelease, old_flags & ~flags);
  DarwinUpdateModifiers(KeyPress, ~old_flags & flags);
  old_flags = flags;
}
