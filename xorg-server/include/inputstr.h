/************************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/


#ifndef INPUTSTRUCT_H
#define INPUTSTRUCT_H

#include "input.h"
#include "window.h"
#include "dixstruct.h"
#include "cursorstr.h"
#include "geext.h"
#include "privates.h"

#define BitIsOn(ptr, bit) (((BYTE *) (ptr))[(bit)>>3] & (1 << ((bit) & 7)))
#define SetBit(ptr, bit)  (((BYTE *) (ptr))[(bit)>>3] |= (1 << ((bit) & 7)))
#define ClearBit(ptr, bit) (((BYTE *)(ptr))[(bit)>>3] &= ~(1 << ((bit) & 7)))

#define SameClient(obj,client) \
	(CLIENT_BITS((obj)->resource) == (client)->clientAsMask)

#define EMASKSIZE	MAXDEVICES + 2

/* This is the last XI2 event supported by the server. If you add
 * events to the protocol, the server will not support these events until
 * this number here is bumped.
 */
#define XI2LASTEVENT    17 /* XI_RawMotion */
#define XI2MASKSIZE     ((XI2LASTEVENT + 7)/8) /* no of bits for masks */

/**
 * This struct stores the core event mask for each client except the client
 * that created the window.
 *
 * Each window that has events selected from other clients has at least one of
 * these masks. If multiple clients selected for events on the same window,
 * these masks are in a linked list.
 *
 * The event mask for the client that created the window is stored in
 * win->eventMask instead.
 *
 * The resource id is simply a fake client ID to associate this mask with a
 * client.
 *
 * Kludge: OtherClients and InputClients must be compatible, see code.
 */
typedef struct _OtherClients {
    OtherClientsPtr	next; /**< Pointer to the next mask */
    XID			resource; /**< id for putting into resource manager */
    Mask		mask; /**< Core event mask */
} OtherClients;

/**
 * This struct stores the XI event mask for each client.
 *
 * Each window that has events selected has at least one of these masks. If
 * multiple client selected for events on the same window, these masks are in
 * a linked list.
 */
typedef struct _InputClients {
    InputClientsPtr	next; /**< Pointer to the next mask */
    XID			resource; /**< id for putting into resource manager */
    Mask		mask[EMASKSIZE]; /**< Actual XI event mask, deviceid is index */
    /** XI2 event masks. One per device, each bit is a mask of (1 << type) */
    unsigned char       xi2mask[EMASKSIZE][XI2MASKSIZE];
} InputClients;

/**
 * Combined XI event masks from all devices.
 *
 * This is the XI equivalent of the deliverableEvents, eventMask and
 * dontPropagate mask of the WindowRec (or WindowOptRec).
 *
 * A window that has an XI client selecting for events has exactly one
 * OtherInputMasks struct and exactly one InputClients struct hanging off
 * inputClients. Each further client appends to the inputClients list.
 * Each Mask field is per-device, with the device id as the index.
 * Exception: for non-device events (Presence events), the MAXDEVICES
 * deviceid is used.
 */
typedef struct _OtherInputMasks {
    /**
     * Bitwise OR of all masks by all clients and the window's parent's masks.
     */
    Mask		deliverableEvents[EMASKSIZE];
    /**
     * Bitwise OR of all masks by all clients on this window.
     */
    Mask		inputEvents[EMASKSIZE];
    /** The do-not-propagate masks for each device. */
    Mask		dontPropagateMask[EMASKSIZE];
    /** The clients that selected for events */
    InputClientsPtr	inputClients;
    /* XI2 event masks. One per device, each bit is a mask of (1 << type) */
    unsigned char       xi2mask[EMASKSIZE][XI2MASKSIZE];
} OtherInputMasks;

/*
 * The following structure gets used for both active and passive grabs. For
 * active grabs some of the fields (e.g. modifiers) are not used. However,
 * that is not much waste since there aren't many active grabs (one per
 * keyboard/pointer device) going at once in the server.
 */

#define MasksPerDetailMask 8		/* 256 keycodes and 256 possible
                                           modifier combinations, but only	
                                           3 buttons. */

typedef struct _DetailRec {		/* Grab details may be bit masks */
    unsigned int        exact;
    Mask                *pMask;
} DetailRec;

typedef enum {
    GRABTYPE_CORE,
    GRABTYPE_XI,
    GRABTYPE_XI2
} GrabType;

union _GrabMask {
    Mask core;
    Mask xi;
    char xi2mask[EMASKSIZE][XI2MASKSIZE];
};

/**
 * Central struct for device grabs. 
 * The same struct is used for both core grabs and device grabs, with
 * different fields being set. 
 * If the grab is a core grab (GrabPointer/GrabKeyboard), then the eventMask
 * is a combination of standard event masks (i.e. PointerMotionMask |
 * ButtonPressMask).
 * If the grab is a device grab (GrabDevice), then the eventMask is a
 * combination of event masks for a given XI event type (see SetEventInfo).
 *
 * If the grab is a result of a ButtonPress, then eventMask is the core mask
 * and deviceMask is set to the XI event mask for the grab.
 */
typedef struct _GrabRec {
    GrabPtr		next;		/* for chain of passive grabs */
    XID			resource;
    DeviceIntPtr	device;
    WindowPtr		window;
    unsigned		ownerEvents:1;
    unsigned		keyboardMode:1;
    unsigned		pointerMode:1;
    GrabType		grabtype;
    CARD8		type;		/* event type */
    DetailRec		modifiersDetail;
    DeviceIntPtr	modifierDevice;
    DetailRec		detail;		/* key or button */
    WindowPtr		confineTo;	/* always NULL for keyboards */
    CursorPtr		cursor;		/* always NULL for keyboards */
    Mask		eventMask;
    Mask                deviceMask;     
    /* XI2 event masks. One per device, each bit is a mask of (1 << type) */
    unsigned char       xi2mask[EMASKSIZE][XI2MASKSIZE];
} GrabRec;

typedef struct _KeyClassRec {
    int			sourceid;
    CARD8		down[DOWN_LENGTH];
    CARD8		postdown[DOWN_LENGTH];
    int                 modifierKeyCount[8];
    struct _XkbSrvInfo *xkbInfo;
} KeyClassRec, *KeyClassPtr;

typedef struct _AxisInfo {
    int		resolution;
    int		min_resolution;
    int		max_resolution;
    int		min_value;
    int		max_value;
    Atom	label;
} AxisInfo, *AxisInfoPtr;

typedef struct _ValuatorAccelerationRec {
    int                         number;
    PointerAccelSchemeProc      AccelSchemeProc;
    void                       *accelData; /* at disposal of AccelScheme */
    DeviceCallbackProc          AccelCleanupProc;
} ValuatorAccelerationRec, *ValuatorAccelerationPtr;

typedef struct _ValuatorClassRec {
    int                   sourceid;
    int		 	  numMotionEvents;
    int                   first_motion;
    int                   last_motion;
    void                  *motion; /* motion history buffer. Different layout
                                      for MDs and SDs!*/
    WindowPtr             motionHintWindow;

    AxisInfoPtr 	  axes;
    unsigned short	  numAxes;
    double		  *axisVal; /* always absolute, but device-coord system */
    CARD8	 	  mode;
    ValuatorAccelerationRec	accelScheme;
} ValuatorClassRec, *ValuatorClassPtr;

typedef struct _ButtonClassRec {
    int			sourceid;
    CARD8		numButtons;
    CARD8		buttonsDown;	/* number of buttons currently down
                                           This counts logical buttons, not
					   physical ones, i.e if some buttons
					   are mapped to 0, they're not counted
					   here */
    unsigned short	state;
    Mask		motionMask;
    CARD8		down[DOWN_LENGTH];
    CARD8		postdown[DOWN_LENGTH];
    CARD8		map[MAP_LENGTH];
    union _XkbAction    *xkb_acts;
    Atom		labels[MAX_BUTTONS];
} ButtonClassRec, *ButtonClassPtr;

typedef struct _FocusClassRec {
    int		sourceid;
    WindowPtr	win; /* May be set to a int constant (e.g. PointerRootWin)! */
    int		revert;
    TimeStamp	time;
    WindowPtr	*trace;
    int		traceSize;
    int		traceGood;
} FocusClassRec, *FocusClassPtr;

typedef struct _ProximityClassRec {
    int		sourceid;
    char	pad;
} ProximityClassRec, *ProximityClassPtr;

typedef struct _AbsoluteClassRec {
    int         sourceid;
    /* Calibration. */
    int         min_x;
    int         max_x;
    int         min_y;
    int         max_y;
    int         flip_x;
    int         flip_y;
    int		rotation;
    int         button_threshold;

    /* Area. */
    int         offset_x;
    int         offset_y;
    int         width;
    int         height;
    int         screen;
    XID		following;
} AbsoluteClassRec, *AbsoluteClassPtr;

typedef struct _KbdFeedbackClassRec *KbdFeedbackPtr;
typedef struct _PtrFeedbackClassRec *PtrFeedbackPtr;
typedef struct _IntegerFeedbackClassRec *IntegerFeedbackPtr;
typedef struct _StringFeedbackClassRec *StringFeedbackPtr;
typedef struct _BellFeedbackClassRec *BellFeedbackPtr;
typedef struct _LedFeedbackClassRec *LedFeedbackPtr;

typedef struct _KbdFeedbackClassRec {
    BellProcPtr		BellProc;
    KbdCtrlProcPtr	CtrlProc;
    KeybdCtrl	 	ctrl;
    KbdFeedbackPtr	next;
    struct _XkbSrvLedInfo *xkb_sli;
} KbdFeedbackClassRec;

typedef struct _PtrFeedbackClassRec {
    PtrCtrlProcPtr	CtrlProc;
    PtrCtrl		ctrl;
    PtrFeedbackPtr	next;
} PtrFeedbackClassRec;

typedef struct _IntegerFeedbackClassRec {
    IntegerCtrlProcPtr	CtrlProc;
    IntegerCtrl	 	ctrl;
    IntegerFeedbackPtr	next;
} IntegerFeedbackClassRec;

typedef struct _StringFeedbackClassRec {
    StringCtrlProcPtr	CtrlProc;
    StringCtrl	 	ctrl;
    StringFeedbackPtr	next;
} StringFeedbackClassRec;

typedef struct _BellFeedbackClassRec {
    BellProcPtr		BellProc;
    BellCtrlProcPtr	CtrlProc;
    BellCtrl	 	ctrl;
    BellFeedbackPtr	next;
} BellFeedbackClassRec;

typedef struct _LedFeedbackClassRec {
    LedCtrlProcPtr	CtrlProc;
    LedCtrl	 	ctrl;
    LedFeedbackPtr	next;
    struct _XkbSrvLedInfo *xkb_sli;
} LedFeedbackClassRec;


typedef struct _ClassesRec {
    KeyClassPtr		key;
    ValuatorClassPtr	valuator;
    ButtonClassPtr	button;
    FocusClassPtr	focus;
    ProximityClassPtr	proximity;
    AbsoluteClassPtr    absolute;
    KbdFeedbackPtr	kbdfeed;
    PtrFeedbackPtr	ptrfeed;
    IntegerFeedbackPtr	intfeed;
    StringFeedbackPtr	stringfeed;
    BellFeedbackPtr	bell;
    LedFeedbackPtr	leds;
} ClassesRec;


/**
 * Sprite information for a device.
 */
typedef struct {
    CursorPtr	current;
    BoxRec	hotLimits;	/* logical constraints of hot spot */
    Bool	confined;	/* confined to screen */
    RegionPtr	hotShape;	/* additional logical shape constraint */
    BoxRec	physLimits;	/* physical constraints of hot spot */
    WindowPtr	win;		/* window of logical position */
    HotSpot	hot;		/* logical pointer position */
    HotSpot	hotPhys;	/* physical pointer position */
#ifdef PANORAMIX
    ScreenPtr	screen;		/* all others are in Screen 0 coordinates */
    RegionRec   Reg1;	        /* Region 1 for confining motion */
    RegionRec   Reg2;		/* Region 2 for confining virtual motion */
    WindowPtr   windows[MAXSCREENS];
    WindowPtr	confineWin;	/* confine window */ 
#endif
    /* The window trace information is used at dix/events.c to avoid having
     * to compute all the windows between the root and the current pointer
     * window each time a button or key goes down. The grabs on each of those
     * windows must be checked.
     * spriteTraces should only be used at dix/events.c! */
    WindowPtr *spriteTrace;
    int spriteTraceSize;
    int spriteTraceGood;

    /* Due to delays between event generation and event processing, it is
     * possible that the pointer has crossed screen boundaries between the
     * time in which it begins generating events and the time when
     * those events are processed.
     *
     * pEnqueueScreen: screen the pointer was on when the event was generated
     * pDequeueScreen: screen the pointer was on when the event is processed
     */
    ScreenPtr pEnqueueScreen;
    ScreenPtr pDequeueScreen;

} SpriteRec, *SpritePtr;

/* Device properties */
typedef struct _XIPropertyValue
{
    Atom                type;           /* ignored by server */
    short               format;         /* format of data for swapping - 8,16,32 */
    long                size;           /* size of data in (format/8) bytes */
    pointer             data;           /* private to client */
} XIPropertyValueRec;

typedef struct _XIProperty
{
    struct _XIProperty   *next;
    Atom                  propertyName;
    BOOL                  deletable;    /* clients can delete this prop? */
    XIPropertyValueRec    value;
} XIPropertyRec;

typedef XIPropertyRec      *XIPropertyPtr;
typedef XIPropertyValueRec *XIPropertyValuePtr;


typedef struct _XIPropertyHandler
{
    struct _XIPropertyHandler* next;
    long id;
    int (*SetProperty) (DeviceIntPtr dev,
                        Atom property,
                        XIPropertyValuePtr prop,
                        BOOL checkonly);
    int (*GetProperty) (DeviceIntPtr dev,
                        Atom property);
    int (*DeleteProperty) (DeviceIntPtr dev,
                           Atom property);
} XIPropertyHandler, *XIPropertyHandlerPtr;

/* states for devices */

#define NOT_GRABBED		0
#define THAWED			1
#define THAWED_BOTH		2	/* not a real state */
#define FREEZE_NEXT_EVENT	3
#define FREEZE_BOTH_NEXT_EVENT	4
#define FROZEN			5	/* any state >= has device frozen */
#define FROZEN_NO_EVENT		5
#define FROZEN_WITH_EVENT	6
#define THAW_OTHERS		7


typedef struct _GrabInfoRec {
    TimeStamp	    grabTime;
    Bool            fromPassiveGrab;    /* true if from passive grab */
    Bool            implicitGrab;       /* implicit from ButtonPress */
    GrabRec         activeGrab;
    GrabPtr         grab;
    CARD8           activatingKey;
    void	    (*ActivateGrab) (
                    DeviceIntPtr /*device*/,
                    GrabPtr /*grab*/,
                    TimeStamp /*time*/,
                    Bool /*autoGrab*/);
    void	    (*DeactivateGrab)(
                    DeviceIntPtr /*device*/);
    struct {
	Bool		frozen;
	int		state;
	GrabPtr		other;		/* if other grab has this frozen */
	DeviceEvent	*event;		/* saved to be replayed */
    } sync;
} GrabInfoRec, *GrabInfoPtr;

typedef struct _SpriteInfoRec {
    /* sprite must always point to a valid sprite. For devices sharing the
     * sprite, let sprite point to a paired spriteOwner's sprite. */
    SpritePtr           sprite;      /* sprite information */
    Bool                spriteOwner; /* True if device owns the sprite */
    DeviceIntPtr        paired;      /* The paired device. Keyboard if
                                        spriteOwner is TRUE, otherwise the
                                        pointer that owns the sprite. */ 
} SpriteInfoRec, *SpriteInfoPtr;

/* device types */
#define MASTER_POINTER          1
#define MASTER_KEYBOARD         2
#define SLAVE                   3

typedef struct _DeviceIntRec {
    DeviceRec	public;
    DeviceIntPtr next;
    Bool	startup;		/* true if needs to be turned on at
				          server intialization time */
    DeviceProc	deviceProc;		/* proc(DevicePtr, DEVICE_xx). It is
					  used to initialize, turn on, or
					  turn off the device */
    Bool	inited;			/* TRUE if INIT returns Success */
    Bool        enabled;                /* TRUE if ON returns Success */
    Bool        coreEvents;             /* TRUE if device also sends core */
    GrabInfoRec deviceGrab;             /* grab on the device */
    int         type;                   /* MASTER_POINTER, MASTER_KEYBOARD, SLAVE */
    Atom		xinput_type;
    char		*name;
    int			id;
    KeyClassPtr		key;
    ValuatorClassPtr	valuator;
    ButtonClassPtr	button;
    FocusClassPtr	focus;
    ProximityClassPtr	proximity;
    AbsoluteClassPtr    absolute;
    KbdFeedbackPtr	kbdfeed;
    PtrFeedbackPtr	ptrfeed;
    IntegerFeedbackPtr	intfeed;
    StringFeedbackPtr	stringfeed;
    BellFeedbackPtr	bell;
    LedFeedbackPtr	leds;
    struct _XkbInterest *xkb_interest;
    char                *config_info; /* used by the hotplug layer */
    PrivateRec		*devPrivates;
    int			nPrivates;
    DeviceUnwrapProc    unwrapProc;
    SpriteInfoPtr       spriteInfo;
    union {
        DeviceIntPtr        master;     /* master device */
        DeviceIntPtr        lastSlave;  /* last slave device used */
    } u;

    /* last valuator values recorded, not posted to client;
     * for slave devices, valuators is in device coordinates
     * for master devices, valuators is in screen coordinates
     * see dix/getevents.c
     * remainder supports acceleration
     */
    struct {
        int             valuators[MAX_VALUATORS];
        float           remainder[MAX_VALUATORS];
        int             numValuators;
        DeviceIntPtr    slave;
    } last;

    /* Input device property handling. */
    struct {
        XIPropertyPtr   properties;
        XIPropertyHandlerPtr handlers; /* NULL-terminated */
    } properties;
} DeviceIntRec;

typedef struct {
    int			numDevices;	/* total number of devices */
    DeviceIntPtr	devices;	/* all devices turned on */
    DeviceIntPtr	off_devices;	/* all devices turned off */
    DeviceIntPtr	keyboard;	/* the main one for the server */
    DeviceIntPtr	pointer;
    DeviceIntPtr	all_devices;
    DeviceIntPtr	all_master_devices;
} InputInfo;

extern _X_EXPORT InputInfo inputInfo;

/* for keeping the events for devices grabbed synchronously */
typedef struct _QdEvent *QdEventPtr;
typedef struct _QdEvent {
    QdEventPtr		next;
    DeviceIntPtr	device;
    ScreenPtr		pScreen;	/* what screen the pointer was on */
    unsigned long	months;		/* milliseconds is in the event */
    InternalEvent	*event;
} QdEventRec;

/**
 * syncEvents is the global structure for queued events.
 *
 * Devices can be frozen through GrabModeSync pointer grabs. If this is the
 * case, events from these devices are added to "pending" instead of being
 * processed normally. When the device is unfrozen, events in "pending" are
 * replayed and processed as if they would come from the device directly.
 */
typedef struct _EventSyncInfo {
    QdEventPtr          pending, /**<  list of queued events */
                        *pendtail; /**< last event in list */
    /** The device to replay events for. Only set in AllowEvents(), in which
     * case it is set to the device specified in the request. */
    DeviceIntPtr        replayDev;      /* kludgy rock to put flag for */

    /**
     * The window the events are supposed to be replayed on.
     * This window may be set to the grab's window (but only when
     * Replay{Pointer|Keyboard} is given in the XAllowEvents()
     * request. */
    WindowPtr           replayWin;      /*   ComputeFreezes            */
    /**
     * Flag to indicate whether we're in the process of
     * replaying events. Only set in ComputeFreezes(). */
    Bool                playingEvents;
    TimeStamp           time;
} EventSyncInfoRec, *EventSyncInfoPtr;

extern EventSyncInfoRec syncEvents;

#endif /* INPUTSTRUCT_H */
