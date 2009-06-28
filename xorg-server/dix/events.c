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

/* The panoramix components contained the following notice */
/*****************************************************************

Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.

******************************************************************/

/*****************************************************************

Copyright 2003-2005 Sun Microsystems, Inc.

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

******************************************************************/

/** @file
 * This file handles event delivery and a big part of the server-side protocol
 * handling (the parts for input devices).
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/keysym.h>
#include "misc.h"
#include "resource.h"
#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xproto.h>
#include "windowstr.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "cursorstr.h"

#include "dixstruct.h"
#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif
#include "globals.h"

#ifdef XKB
#include <X11/extensions/XKBproto.h>
#include <xkbsrv.h>
extern Bool XkbFilterEvents(ClientPtr, int, xEvent *);
#endif

#include "xace.h"

#ifdef XSERVER_DTRACE
#include <sys/types.h>
typedef const char *string;
#include "Xserver-dtrace.h"
#endif

#ifdef XEVIE
extern WindowPtr *WindowTable;
extern int       xevieFlag;
extern int       xevieClientIndex;
extern DeviceIntPtr     xeviemouse;
extern DeviceIntPtr     xeviekb;
extern Mask      xevieMask;
extern Mask      xevieFilters[128];
extern int       xevieEventSent;
extern int       xevieKBEventSent;
int    xeviegrabState = 0;
#endif

#include <X11/extensions/XIproto.h>
#include "exglobals.h"
#include "exevents.h"
#include "exglobals.h"
#include "extnsionst.h"

#include "dixevents.h"
#include "dixgrabs.h"
#include "dispatch.h"
/**
 * Extension events type numbering starts at EXTENSION_EVENT_BASE.
 */
#define EXTENSION_EVENT_BASE  64

#define NoSuchEvent 0x80000000	/* so doesn't match NoEventMask */
#define StructureAndSubMask ( StructureNotifyMask | SubstructureNotifyMask )
#define AllButtonsMask ( \
	Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )
#define MotionMask ( \
	PointerMotionMask | Button1MotionMask | \
	Button2MotionMask | Button3MotionMask | Button4MotionMask | \
	Button5MotionMask | ButtonMotionMask )
#define PropagateMask ( \
	KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | \
	MotionMask )
#define PointerGrabMask ( \
	ButtonPressMask | ButtonReleaseMask | \
	EnterWindowMask | LeaveWindowMask | \
	PointerMotionHintMask | KeymapStateMask | \
	MotionMask )
#define AllModifiersMask ( \
	ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask | \
	Mod3Mask | Mod4Mask | Mod5Mask )
#define AllEventMasks (lastEventMask|(lastEventMask-1))
/*
 * The following relies on the fact that the Button<n>MotionMasks are equal
 * to the corresponding Button<n>Masks from the current modifier/button state.
 */
#define Motion_Filter(class) (PointerMotionMask | \
			      (class)->state | (class)->motionMask)


#define WID(w) ((w) ? ((w)->drawable.id) : 0)

#define XE_KBPTR (xE->u.keyButtonPointer)


#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])

_X_EXPORT CallbackListPtr EventCallback;
_X_EXPORT CallbackListPtr DeviceEventCallback;

#define DNPMCOUNT 8

Mask DontPropagateMasks[DNPMCOUNT];
static int DontPropagateRefCnts[DNPMCOUNT];

/**
 * Main input device struct. 
 *     inputInfo.pointer 
 *     is the core pointer. Referred to as "virtual core pointer", "VCP",
 *     "core pointer" or inputInfo.pointer. There is exactly one core pointer,
 *     but multiple devices may send core events. If a device generates core
 *     events, those events will appear to originate from the core pointer.
 * 
 *     inputInfo.keyboard
 *     is the core keyboard ("virtual core keyboard", "VCK", "core keyboard").
 *     See inputInfo.pointer.
 * 
 *     inputInfo.devices
 *     linked list containing all devices including VCK and VCP. The VCK will
 *     always be the first entry, the VCP the second entry in the device list.
 *
 *     inputInfo.off_devices
 *     Devices that have not been initialized and are thus turned off.
 *
 *     inputInfo.numDevices
 *     Total number of devices.
 */
_X_EXPORT InputInfo inputInfo;

static struct {
    QdEventPtr		pending, *pendtail;
    DeviceIntPtr	replayDev;	/* kludgy rock to put flag for */
    WindowPtr		replayWin;	/*   ComputeFreezes            */
    Bool		playingEvents;
    TimeStamp		time;
} syncEvents;

/*
 * The window trace information is used to avoid having to compute all the
 * windows between the root and the current pointer window each time a button
 * or key goes down. The grabs on each of those windows must be checked.
 * 
 * @see XYToWindow() for a documentation on how the array is set up.
 */
static WindowPtr *spriteTrace = (WindowPtr *)NULL;
#define ROOT spriteTrace[0]
static int spriteTraceSize = 0;
static int spriteTraceGood;

/**
 * DIX sprite information. This is the sprite as seen from the DIX. It does
 * not represent the actual sprite rendered to the screen.
 * 
 */
static  struct {
    CursorPtr	current;
    BoxRec	hotLimits;	/* logical constraints of hot spot */
    Bool	confined;	/* confined to screen */
#if defined(SHAPE) || defined(PANORAMIX)
    RegionPtr	hotShape;	/* additional logical shape constraint */
#endif
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
} sprite;			/* info about the cursor sprite */

#ifdef XEVIE
_X_EXPORT WindowPtr xeviewin;
_X_EXPORT HotSpot xeviehot;
#endif

static void DoEnterLeaveEvents(
    WindowPtr fromWin,
    WindowPtr toWin,
    int mode
);

static WindowPtr XYToWindow(
    int x,
    int y
);

/**
 * Max event opcode.
 */
extern int lastEvent;

static Mask lastEventMask;

#ifdef XINPUT
extern int DeviceMotionNotify;
#endif

#define CantBeFiltered NoEventMask
static Mask filters[128] =
{
	NoSuchEvent,		       /* 0 */
	NoSuchEvent,		       /* 1 */
	KeyPressMask,		       /* KeyPress */
	KeyReleaseMask,		       /* KeyRelease */
	ButtonPressMask,	       /* ButtonPress */
	ButtonReleaseMask,	       /* ButtonRelease */
	PointerMotionMask,	       /* MotionNotify (initial state) */
	EnterWindowMask,	       /* EnterNotify */
	LeaveWindowMask,	       /* LeaveNotify */
	FocusChangeMask,	       /* FocusIn */
	FocusChangeMask,	       /* FocusOut */
	KeymapStateMask,	       /* KeymapNotify */
	ExposureMask,		       /* Expose */
	CantBeFiltered,		       /* GraphicsExpose */
	CantBeFiltered,		       /* NoExpose */
	VisibilityChangeMask,	       /* VisibilityNotify */
	SubstructureNotifyMask,	       /* CreateNotify */
	StructureAndSubMask,	       /* DestroyNotify */
	StructureAndSubMask,	       /* UnmapNotify */
	StructureAndSubMask,	       /* MapNotify */
	SubstructureRedirectMask,      /* MapRequest */
	StructureAndSubMask,	       /* ReparentNotify */
	StructureAndSubMask,	       /* ConfigureNotify */
	SubstructureRedirectMask,      /* ConfigureRequest */
	StructureAndSubMask,	       /* GravityNotify */
	ResizeRedirectMask,	       /* ResizeRequest */
	StructureAndSubMask,	       /* CirculateNotify */
	SubstructureRedirectMask,      /* CirculateRequest */
	PropertyChangeMask,	       /* PropertyNotify */
	CantBeFiltered,		       /* SelectionClear */
	CantBeFiltered,		       /* SelectionRequest */
	CantBeFiltered,		       /* SelectionNotify */
	ColormapChangeMask,	       /* ColormapNotify */
	CantBeFiltered,		       /* ClientMessage */
	CantBeFiltered		       /* MappingNotify */
};

static CARD8 criticalEvents[32] =
{
    0x7c				/* key and button events */
};

#ifdef PANORAMIX
static void ConfineToShape(RegionPtr shape, int *px, int *py);
static void PostNewCursor(void);

#define SyntheticMotion(x, y) \
    PostSyntheticMotion(x, y, noPanoramiXExtension ? 0 : \
                              sprite.screen->myNum, \
                        syncEvents.playingEvents ? \
                          syncEvents.time.milliseconds : \
                          currentTime.milliseconds);

static Bool
XineramaSetCursorPosition(
    int x, 
    int y, 
    Bool generateEvent
){
    ScreenPtr pScreen;
    BoxRec box;
    int i;

    /* x,y are in Screen 0 coordinates.  We need to decide what Screen
       to send the message too and what the coordinates relative to 
       that screen are. */

    pScreen = sprite.screen;
    x += panoramiXdataPtr[0].x;
    y += panoramiXdataPtr[0].y;

    if(!POINT_IN_REGION(pScreen, &XineramaScreenRegions[pScreen->myNum],
								x, y, &box)) 
    {
	FOR_NSCREENS(i) 
	{
	    if(i == pScreen->myNum) 
		continue;
	    if(POINT_IN_REGION(pScreen, &XineramaScreenRegions[i], x, y, &box))
	    {
		pScreen = screenInfo.screens[i];
		break;
	    }
	}
    }

    sprite.screen = pScreen;
    sprite.hotPhys.x = x - panoramiXdataPtr[0].x;
    sprite.hotPhys.y = y - panoramiXdataPtr[0].y;
    x -= panoramiXdataPtr[pScreen->myNum].x;
    y -= panoramiXdataPtr[pScreen->myNum].y;

    return (*pScreen->SetCursorPosition)(pScreen, x, y, generateEvent);
}


static void
XineramaConstrainCursor(void)
{
    ScreenPtr pScreen = sprite.screen;
    BoxRec newBox = sprite.physLimits;

    /* Translate the constraining box to the screen
       the sprite is actually on */
    newBox.x1 += panoramiXdataPtr[0].x - panoramiXdataPtr[pScreen->myNum].x;
    newBox.x2 += panoramiXdataPtr[0].x - panoramiXdataPtr[pScreen->myNum].x;
    newBox.y1 += panoramiXdataPtr[0].y - panoramiXdataPtr[pScreen->myNum].y;
    newBox.y2 += panoramiXdataPtr[0].y - panoramiXdataPtr[pScreen->myNum].y;

    (* pScreen->ConstrainCursor)(pScreen, &newBox);
}

static void
XineramaCheckPhysLimits(
    CursorPtr cursor,
    Bool generateEvents
){
    HotSpot new;

    if (!cursor)
	return;
 
    new = sprite.hotPhys;

    /* I don't care what the DDX has to say about it */
    sprite.physLimits = sprite.hotLimits;

    /* constrain the pointer to those limits */
    if (new.x < sprite.physLimits.x1)
	new.x = sprite.physLimits.x1;
    else
	if (new.x >= sprite.physLimits.x2)
	    new.x = sprite.physLimits.x2 - 1;
    if (new.y < sprite.physLimits.y1)
	new.y = sprite.physLimits.y1;
    else
	if (new.y >= sprite.physLimits.y2)
	    new.y = sprite.physLimits.y2 - 1;

    if (sprite.hotShape)  /* more work if the shape is a mess */
	ConfineToShape(sprite.hotShape, &new.x, &new.y);

    if((new.x != sprite.hotPhys.x) || (new.y != sprite.hotPhys.y))
    {
	XineramaSetCursorPosition (new.x, new.y, generateEvents);
	if (!generateEvents)
	    SyntheticMotion(new.x, new.y);
    }

    /* Tell DDX what the limits are */
    XineramaConstrainCursor();
}


static Bool
XineramaSetWindowPntrs(WindowPtr pWin)
{
    if(pWin == WindowTable[0]) {
	    memcpy(sprite.windows, WindowTable, 
				PanoramiXNumScreens*sizeof(WindowPtr));
    } else {
	PanoramiXRes *win;
	int i;

	win = (PanoramiXRes*)LookupIDByType(pWin->drawable.id, XRT_WINDOW);

	if(!win)
	    return FALSE;

	for(i = 0; i < PanoramiXNumScreens; i++) {
	   sprite.windows[i] = LookupIDByType(win->info[i].id, RT_WINDOW);
	   if(!sprite.windows[i])  /* window is being unmapped */
		return FALSE;
	}
    }
    return TRUE;
}

static void
XineramaCheckVirtualMotion(
   QdEventPtr qe,
   WindowPtr pWin
){

    if (qe)
    {
	sprite.hot.pScreen = qe->pScreen;  /* should always be Screen 0 */
	sprite.hot.x = qe->event->u.keyButtonPointer.rootX;
	sprite.hot.y = qe->event->u.keyButtonPointer.rootY;
	pWin = inputInfo.pointer->grab ? inputInfo.pointer->grab->confineTo :
					 NullWindow;
    }
    if (pWin)
    {
	int x, y, off_x, off_y, i;
	BoxRec lims;

	if(!XineramaSetWindowPntrs(pWin))
	    return;

	i = PanoramiXNumScreens - 1;
	
	REGION_COPY(sprite.screen, &sprite.Reg2, 
					&sprite.windows[i]->borderSize); 
	off_x = panoramiXdataPtr[i].x;
	off_y = panoramiXdataPtr[i].y;

	while(i--) {
	    x = off_x - panoramiXdataPtr[i].x;
	    y = off_y - panoramiXdataPtr[i].y;

	    if(x || y)
		REGION_TRANSLATE(sprite.screen, &sprite.Reg2, x, y);
		
	    REGION_UNION(sprite.screen, &sprite.Reg2, &sprite.Reg2, 
					&sprite.windows[i]->borderSize);

	    off_x = panoramiXdataPtr[i].x;
	    off_y = panoramiXdataPtr[i].y;
	}

	lims = *REGION_EXTENTS(sprite.screen, &sprite.Reg2);

        if (sprite.hot.x < lims.x1)
            sprite.hot.x = lims.x1;
        else if (sprite.hot.x >= lims.x2)
            sprite.hot.x = lims.x2 - 1;
        if (sprite.hot.y < lims.y1)
            sprite.hot.y = lims.y1;
        else if (sprite.hot.y >= lims.y2)
            sprite.hot.y = lims.y2 - 1;

	if (REGION_NUM_RECTS(&sprite.Reg2) > 1) 
	    ConfineToShape(&sprite.Reg2, &sprite.hot.x, &sprite.hot.y);

	if (qe)
	{
	    qe->pScreen = sprite.hot.pScreen;
	    qe->event->u.keyButtonPointer.rootX = sprite.hot.x;
	    qe->event->u.keyButtonPointer.rootY = sprite.hot.y;
	}
    }
#ifdef XEVIE
    xeviehot.x = sprite.hot.x;
    xeviehot.y = sprite.hot.y;
#endif
}


static Bool
XineramaCheckMotion(xEvent *xE)
{
    WindowPtr prevSpriteWin = sprite.win;

    if (xE && !syncEvents.playingEvents)
    {
	/* Motion events entering DIX get translated to Screen 0
	   coordinates.  Replayed events have already been 
	   translated since they've entered DIX before */
	XE_KBPTR.rootX += panoramiXdataPtr[sprite.screen->myNum].x -
			  panoramiXdataPtr[0].x;
	XE_KBPTR.rootY += panoramiXdataPtr[sprite.screen->myNum].y -
			  panoramiXdataPtr[0].y;
	sprite.hot.x = XE_KBPTR.rootX;
	sprite.hot.y = XE_KBPTR.rootY;
	if (sprite.hot.x < sprite.physLimits.x1)
	    sprite.hot.x = sprite.physLimits.x1;
	else if (sprite.hot.x >= sprite.physLimits.x2)
	    sprite.hot.x = sprite.physLimits.x2 - 1;
	if (sprite.hot.y < sprite.physLimits.y1)
	    sprite.hot.y = sprite.physLimits.y1;
	else if (sprite.hot.y >= sprite.physLimits.y2)
	    sprite.hot.y = sprite.physLimits.y2 - 1;

	if (sprite.hotShape) 
	    ConfineToShape(sprite.hotShape, &sprite.hot.x, &sprite.hot.y);

	sprite.hotPhys = sprite.hot;
	if ((sprite.hotPhys.x != XE_KBPTR.rootX) ||
	    (sprite.hotPhys.y != XE_KBPTR.rootY))
	{
	    XineramaSetCursorPosition(
			sprite.hotPhys.x, sprite.hotPhys.y, FALSE);
	}
	XE_KBPTR.rootX = sprite.hot.x;
	XE_KBPTR.rootY = sprite.hot.y;
    }

#ifdef XEVIE
    xeviehot.x = sprite.hot.x;
    xeviehot.y = sprite.hot.y;
    xeviewin =
#endif
    sprite.win = XYToWindow(sprite.hot.x, sprite.hot.y);

    if (sprite.win != prevSpriteWin)
    {
	if (prevSpriteWin != NullWindow) {
	    if (!xE)
		UpdateCurrentTimeIf();
	    DoEnterLeaveEvents(prevSpriteWin, sprite.win, NotifyNormal);
	}
	PostNewCursor();
        return FALSE;
    }
    return TRUE;
}


static void
XineramaConfineCursorToWindow(WindowPtr pWin, Bool generateEvents)
{

    if (syncEvents.playingEvents)
    {
	XineramaCheckVirtualMotion((QdEventPtr)NULL, pWin);
	SyntheticMotion(sprite.hot.x, sprite.hot.y);
    }
    else
    {
	int x, y, off_x, off_y, i;

	if(!XineramaSetWindowPntrs(pWin))
	    return;

	i = PanoramiXNumScreens - 1;
	
	REGION_COPY(sprite.screen, &sprite.Reg1, 
					&sprite.windows[i]->borderSize); 
	off_x = panoramiXdataPtr[i].x;
	off_y = panoramiXdataPtr[i].y;

	while(i--) {
	    x = off_x - panoramiXdataPtr[i].x;
	    y = off_y - panoramiXdataPtr[i].y;

	    if(x || y)
		REGION_TRANSLATE(sprite.screen, &sprite.Reg1, x, y);
		
	    REGION_UNION(sprite.screen, &sprite.Reg1, &sprite.Reg1, 
					&sprite.windows[i]->borderSize);

	    off_x = panoramiXdataPtr[i].x;
	    off_y = panoramiXdataPtr[i].y;
	}

	sprite.hotLimits = *REGION_EXTENTS(sprite.screen, &sprite.Reg1);

	if(REGION_NUM_RECTS(&sprite.Reg1) > 1)
	   sprite.hotShape = &sprite.Reg1;
	else
	   sprite.hotShape = NullRegion;
	
	sprite.confined = FALSE;
	sprite.confineWin = (pWin == WindowTable[0]) ? NullWindow : pWin;

	XineramaCheckPhysLimits(sprite.current, generateEvents);
    }
}


static void
XineramaChangeToCursor(CursorPtr cursor)
{
    if (cursor != sprite.current)
    {
	if ((sprite.current->bits->xhot != cursor->bits->xhot) ||
		(sprite.current->bits->yhot != cursor->bits->yhot))
	    XineramaCheckPhysLimits(cursor, FALSE);
    	(*sprite.screen->DisplayCursor)(sprite.screen, cursor);
	FreeCursor(sprite.current, (Cursor)0);
	sprite.current = cursor;
	sprite.current->refcnt++;
    }
}

#else
#define SyntheticMotion(x, y) \
     PostSyntheticMotion(x, y, \
                         0, \
                         syncEvents.playingEvents ? \
                           syncEvents.time.milliseconds : \
                           currentTime.milliseconds);

#endif  /* PANORAMIX */

void
SetMaskForEvent(Mask mask, int event)
{
    if ((event < LASTEvent) || (event >= 128))
	FatalError("SetMaskForEvent: bogus event number");
    filters[event] = mask;
}

_X_EXPORT void
SetCriticalEvent(int event)
{
    if (event >= 128)
	FatalError("SetCriticalEvent: bogus event number");
    criticalEvents[event >> 3] |= 1 << (event & 7);
}

#ifdef SHAPE
static void
ConfineToShape(RegionPtr shape, int *px, int *py)
{
    BoxRec box;
    int x = *px, y = *py;
    int incx = 1, incy = 1;

    if (POINT_IN_REGION(sprite.hot.pScreen, shape, x, y, &box))
	return;
    box = *REGION_EXTENTS(sprite.hot.pScreen, shape);
    /* this is rather crude */
    do {
	x += incx;
	if (x >= box.x2)
	{
	    incx = -1;
	    x = *px - 1;
	}
	else if (x < box.x1)
	{
	    incx = 1;
	    x = *px;
	    y += incy;
	    if (y >= box.y2)
	    {
		incy = -1;
		y = *py - 1;
	    }
	    else if (y < box.y1)
		return; /* should never get here! */
	}
    } while (!POINT_IN_REGION(sprite.hot.pScreen, shape, x, y, &box));
    *px = x;
    *py = y;
}
#endif

static void
CheckPhysLimits(
    CursorPtr cursor,
    Bool generateEvents,
    Bool confineToScreen,
    ScreenPtr pScreen)
{
    HotSpot new;

    if (!cursor)
	return;
    new = sprite.hotPhys;
    if (pScreen)
	new.pScreen = pScreen;
    else
	pScreen = new.pScreen;
    (*pScreen->CursorLimits) (pScreen, cursor, &sprite.hotLimits,
			      &sprite.physLimits);
    sprite.confined = confineToScreen;
    (* pScreen->ConstrainCursor)(pScreen, &sprite.physLimits);
    if (new.x < sprite.physLimits.x1)
	new.x = sprite.physLimits.x1;
    else
	if (new.x >= sprite.physLimits.x2)
	    new.x = sprite.physLimits.x2 - 1;
    if (new.y < sprite.physLimits.y1)
	new.y = sprite.physLimits.y1;
    else
	if (new.y >= sprite.physLimits.y2)
	    new.y = sprite.physLimits.y2 - 1;
#ifdef SHAPE
    if (sprite.hotShape)
	ConfineToShape(sprite.hotShape, &new.x, &new.y); 
#endif
    if ((pScreen != sprite.hotPhys.pScreen) ||
	(new.x != sprite.hotPhys.x) || (new.y != sprite.hotPhys.y))
    {
	if (pScreen != sprite.hotPhys.pScreen)
	    sprite.hotPhys = new;
	(*pScreen->SetCursorPosition) (pScreen, new.x, new.y, generateEvents);
	if (!generateEvents)
	    SyntheticMotion(new.x, new.y);
    }
}

static void
CheckVirtualMotion(
    QdEventPtr qe,
    WindowPtr pWin)
{
#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
	XineramaCheckVirtualMotion(qe, pWin);
	return;
    }
#endif
    if (qe)
    {
	sprite.hot.pScreen = qe->pScreen;
	sprite.hot.x = qe->event->u.keyButtonPointer.rootX;
	sprite.hot.y = qe->event->u.keyButtonPointer.rootY;
	pWin = inputInfo.pointer->grab ? inputInfo.pointer->grab->confineTo :
					 NullWindow;
    }
    if (pWin)
    {
	BoxRec lims;

	if (sprite.hot.pScreen != pWin->drawable.pScreen)
	{
	    sprite.hot.pScreen = pWin->drawable.pScreen;
	    sprite.hot.x = sprite.hot.y = 0;
	}
	lims = *REGION_EXTENTS(pWin->drawable.pScreen, &pWin->borderSize);
	if (sprite.hot.x < lims.x1)
	    sprite.hot.x = lims.x1;
	else if (sprite.hot.x >= lims.x2)
	    sprite.hot.x = lims.x2 - 1;
	if (sprite.hot.y < lims.y1)
	    sprite.hot.y = lims.y1;
	else if (sprite.hot.y >= lims.y2)
	    sprite.hot.y = lims.y2 - 1;
#ifdef SHAPE
	if (wBoundingShape(pWin))
	    ConfineToShape(&pWin->borderSize, &sprite.hot.x, &sprite.hot.y);
#endif
	if (qe)
	{
	    qe->pScreen = sprite.hot.pScreen;
	    qe->event->u.keyButtonPointer.rootX = sprite.hot.x;
	    qe->event->u.keyButtonPointer.rootY = sprite.hot.y;
	}
    }
#ifdef XEVIE
    xeviehot.x = sprite.hot.x;
    xeviehot.y = sprite.hot.y;
#endif
    ROOT = WindowTable[sprite.hot.pScreen->myNum];
}

static void
ConfineCursorToWindow(WindowPtr pWin, Bool generateEvents, Bool confineToScreen)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
	XineramaConfineCursorToWindow(pWin, generateEvents);
	return;
    }	
#endif

    if (syncEvents.playingEvents)
    {
	CheckVirtualMotion((QdEventPtr)NULL, pWin);
	SyntheticMotion(sprite.hot.x, sprite.hot.y);
    }
    else
    {
	sprite.hotLimits = *REGION_EXTENTS( pScreen, &pWin->borderSize);
#ifdef SHAPE
	sprite.hotShape = wBoundingShape(pWin) ? &pWin->borderSize
					       : NullRegion;
#endif
	CheckPhysLimits(sprite.current, generateEvents, confineToScreen,
			pScreen);
    }
}

_X_EXPORT Bool
PointerConfinedToScreen(void)
{
    return sprite.confined;
}

/**
 * Update the sprite cursor to the given cursor.
 *
 * ChangeToCursor() will display the new cursor and free the old cursor (if
 * applicable). If the provided cursor is already the updated cursor, nothing
 * happens.
 */
static void
ChangeToCursor(CursorPtr cursor)
{
#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
	XineramaChangeToCursor(cursor);
	return;
    }
#endif

    if (cursor != sprite.current)
    {
	if ((sprite.current->bits->xhot != cursor->bits->xhot) ||
		(sprite.current->bits->yhot != cursor->bits->yhot))
	    CheckPhysLimits(cursor, FALSE, sprite.confined,
			    (ScreenPtr)NULL);
	(*sprite.hotPhys.pScreen->DisplayCursor) (sprite.hotPhys.pScreen,
						  cursor);
	FreeCursor(sprite.current, (Cursor)0);
	sprite.current = cursor;
	sprite.current->refcnt++;
    }
}

/**
 * @returns true if b is a descendent of a 
 */
Bool
IsParent(WindowPtr a, WindowPtr b)
{
    for (b = b->parent; b; b = b->parent)
	if (b == a) return TRUE;
    return FALSE;
}

/**
 * Update the cursor displayed on the screen.
 *
 * Called whenever a cursor may have changed shape or position.  
 */
static void
PostNewCursor(void)
{
    WindowPtr win;
    GrabPtr grab = inputInfo.pointer->grab;

    if (syncEvents.playingEvents)
	return;
    if (grab)
    {
	if (grab->cursor)
	{
	    ChangeToCursor(grab->cursor);
	    return;
	}
	if (IsParent(grab->window, sprite.win))
	    win = sprite.win;
	else
	    win = grab->window;
    }
    else
	win = sprite.win;
    for (; win; win = win->parent)
	if (win->optional && win->optional->cursor != NullCursor)
	{
	    ChangeToCursor(win->optional->cursor);
	    return;
	}
}

/**
 * @return root window of current active screen.
 */
_X_EXPORT WindowPtr
GetCurrentRootWindow(void)
{
    return ROOT;
}

/**
 * @return window underneath the cursor sprite.
 */
_X_EXPORT WindowPtr
GetSpriteWindow(void)
{
    return sprite.win;
}

/**
 * @return current sprite cursor.
 */
_X_EXPORT CursorPtr
GetSpriteCursor(void)
{
    return sprite.current;
}

/**
 * Set x/y current sprite position in screen coordinates.
 */
_X_EXPORT void
GetSpritePosition(int *px, int *py)
{
    *px = sprite.hotPhys.x;
    *py = sprite.hotPhys.y;
}

#ifdef PANORAMIX
_X_EXPORT int
XineramaGetCursorScreen(void)
{
    if(!noPanoramiXExtension) {
	return sprite.screen->myNum;
    } else {
	return 0;
    }
}
#endif /* PANORAMIX */

#define TIMESLOP (5 * 60 * 1000) /* 5 minutes */

static void
MonthChangedOrBadTime(xEvent *xE)
{
    /* If the ddx/OS is careless about not processing timestamped events from
     * different sources in sorted order, then it's possible for time to go
     * backwards when it should not.  Here we ensure a decent time.
     */
    if ((currentTime.milliseconds - XE_KBPTR.time) > TIMESLOP)
	currentTime.months++;
    else
	XE_KBPTR.time = currentTime.milliseconds;
}

#define NoticeTime(xE) { \
    if ((xE)->u.keyButtonPointer.time < currentTime.milliseconds) \
	MonthChangedOrBadTime(xE); \
    currentTime.milliseconds = (xE)->u.keyButtonPointer.time; \
    lastDeviceEventTime = currentTime; }

void
NoticeEventTime(xEvent *xE)
{
    if (!syncEvents.playingEvents)
	NoticeTime(xE);
}

/**************************************************************************
 *            The following procedures deal with synchronous events       *
 **************************************************************************/

void
EnqueueEvent(xEvent *xE, DeviceIntPtr device, int count)
{
    QdEventPtr tail = *syncEvents.pendtail;
    QdEventPtr qe;
    xEvent		*qxE;

    NoticeTime(xE);

#ifdef XKB
    /* Fix for key repeating bug. */
    if (device->key != NULL && device->key->xkbInfo != NULL && 
	xE->u.u.type == KeyRelease)
	AccessXCancelRepeatKey(device->key->xkbInfo, xE->u.u.detail);
#endif

    if (DeviceEventCallback)
    {
	DeviceEventInfoRec eventinfo;
	/*  The RECORD spec says that the root window field of motion events
	 *  must be valid.  At this point, it hasn't been filled in yet, so
	 *  we do it here.  The long expression below is necessary to get
	 *  the current root window; the apparently reasonable alternative
	 *  GetCurrentRootWindow()->drawable.id doesn't give you the right
	 *  answer on the first motion event after a screen change because
	 *  the data that GetCurrentRootWindow relies on hasn't been
	 *  updated yet.
	 */
	if (xE->u.u.type == MotionNotify)
	    XE_KBPTR.root =
		WindowTable[sprite.hotPhys.pScreen->myNum]->drawable.id;
	eventinfo.events = xE;
	eventinfo.count = count;
	CallCallbacks(&DeviceEventCallback, (pointer)&eventinfo);
    }
    if (xE->u.u.type == MotionNotify)
    {
#ifdef PANORAMIX
	if(!noPanoramiXExtension) {
	    XE_KBPTR.rootX += panoramiXdataPtr[sprite.screen->myNum].x -
			      panoramiXdataPtr[0].x;
	    XE_KBPTR.rootY += panoramiXdataPtr[sprite.screen->myNum].y -
			      panoramiXdataPtr[0].y;
	}
#endif
	sprite.hotPhys.x = XE_KBPTR.rootX;
	sprite.hotPhys.y = XE_KBPTR.rootY;
	/* do motion compression, but not if from different devices */
	if (tail &&
	    (tail->event->u.u.type == MotionNotify) &&
            (tail->device == device) &&
	    (tail->pScreen == sprite.hotPhys.pScreen))
	{
	    tail->event->u.keyButtonPointer.rootX = sprite.hotPhys.x;
	    tail->event->u.keyButtonPointer.rootY = sprite.hotPhys.y;
	    tail->event->u.keyButtonPointer.time = XE_KBPTR.time;
	    tail->months = currentTime.months;
	    return;
	}
    }
    qe = (QdEventPtr)xalloc(sizeof(QdEventRec) + (count * sizeof(xEvent)));
    if (!qe)
	return;
    qe->next = (QdEventPtr)NULL;
    qe->device = device;
    qe->pScreen = sprite.hotPhys.pScreen;
    qe->months = currentTime.months;
    qe->event = (xEvent *)(qe + 1);
    qe->evcount = count;
    for (qxE = qe->event; --count >= 0; qxE++, xE++)
	*qxE = *xE;
    if (tail)
	syncEvents.pendtail = &tail->next;
    *syncEvents.pendtail = qe;
}

static void
PlayReleasedEvents(void)
{
    QdEventPtr *prev, qe;
    DeviceIntPtr dev;

    prev = &syncEvents.pending;
    while ( (qe = *prev) )
    {
	if (!qe->device->sync.frozen)
	{
	    *prev = qe->next;
	    if (*syncEvents.pendtail == *prev)
		syncEvents.pendtail = prev;
	    if (qe->event->u.u.type == MotionNotify)
		CheckVirtualMotion(qe, NullWindow);
	    syncEvents.time.months = qe->months;
	    syncEvents.time.milliseconds = qe->event->u.keyButtonPointer.time;
#ifdef PANORAMIX
	   /* Translate back to the sprite screen since processInputProc
	      will translate from sprite screen to screen 0 upon reentry
	      to the DIX layer */
	    if(!noPanoramiXExtension) {
		qe->event->u.keyButtonPointer.rootX += 
			panoramiXdataPtr[0].x - 
			panoramiXdataPtr[sprite.screen->myNum].x;
		qe->event->u.keyButtonPointer.rootY += 
			panoramiXdataPtr[0].y - 
			panoramiXdataPtr[sprite.screen->myNum].y;
	    }
#endif
	    (*qe->device->public.processInputProc)(qe->event, qe->device,
						   qe->evcount);
	    xfree(qe);
	    for (dev = inputInfo.devices; dev && dev->sync.frozen; dev = dev->next)
		;
	    if (!dev)
		break;
	    /* Playing the event may have unfrozen another device. */
	    /* So to play it safe, restart at the head of the queue */
	    prev = &syncEvents.pending;
	}
	else
	    prev = &qe->next;
    } 
}

static void
FreezeThaw(DeviceIntPtr dev, Bool frozen)
{
    dev->sync.frozen = frozen;
    if (frozen)
	dev->public.processInputProc = dev->public.enqueueInputProc;
    else
	dev->public.processInputProc = dev->public.realInputProc;
}

static void
ComputeFreezes(void)
{
    DeviceIntPtr replayDev = syncEvents.replayDev;
    int i;
    WindowPtr w;
    xEvent *xE;
    int count;
    GrabPtr grab;
    DeviceIntPtr dev;

    for (dev = inputInfo.devices; dev; dev = dev->next)
	FreezeThaw(dev, dev->sync.other || (dev->sync.state >= FROZEN));
    if (syncEvents.playingEvents || (!replayDev && !syncEvents.pending))
	return;
    syncEvents.playingEvents = TRUE;
    if (replayDev)
    {
	xE = replayDev->sync.event;
	count = replayDev->sync.evcount;
	syncEvents.replayDev = (DeviceIntPtr)NULL;

        w = XYToWindow( XE_KBPTR.rootX, XE_KBPTR.rootY);
	for (i = 0; i < spriteTraceGood; i++)
	{
	    if (syncEvents.replayWin == spriteTrace[i])
	    {
		if (!CheckDeviceGrabs(replayDev, xE, i+1, count)) {
		    if (replayDev->focus)
			DeliverFocusedEvent(replayDev, xE, w, count);
		    else
			DeliverDeviceEvents(w, xE, NullGrab, NullWindow,
					        replayDev, count);
		}
		goto playmore;
	    }
	}
	/* must not still be in the same stack */
	if (replayDev->focus)
	    DeliverFocusedEvent(replayDev, xE, w, count);
	else
	    DeliverDeviceEvents(w, xE, NullGrab, NullWindow, replayDev, count);
    }
playmore:
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (!dev->sync.frozen)
	{
	    PlayReleasedEvents();
	    break;
	}
    }
    syncEvents.playingEvents = FALSE;
    /* the following may have been skipped during replay, so do it now */
    if ((grab = inputInfo.pointer->grab) && grab->confineTo)
    {
	if (grab->confineTo->drawable.pScreen != sprite.hotPhys.pScreen)
	    sprite.hotPhys.x = sprite.hotPhys.y = 0;
	ConfineCursorToWindow(grab->confineTo, TRUE, TRUE);
    }
    else
	ConfineCursorToWindow(WindowTable[sprite.hotPhys.pScreen->myNum],
			      TRUE, FALSE);
    PostNewCursor();
}

#ifdef RANDR
void
ScreenRestructured (ScreenPtr pScreen)
{
    GrabPtr grab;

    if ((grab = inputInfo.pointer->grab) && grab->confineTo)
    {
	if (grab->confineTo->drawable.pScreen != sprite.hotPhys.pScreen)
	    sprite.hotPhys.x = sprite.hotPhys.y = 0;
	ConfineCursorToWindow(grab->confineTo, TRUE, TRUE);
    }
    else
	ConfineCursorToWindow(WindowTable[sprite.hotPhys.pScreen->myNum],
			      TRUE, FALSE);
}
#endif

static void
CheckGrabForSyncs(DeviceIntPtr thisDev, Bool thisMode, Bool otherMode)
{
    GrabPtr grab = thisDev->grab;
    DeviceIntPtr dev;

    if (thisMode == GrabModeSync)
	thisDev->sync.state = FROZEN_NO_EVENT;
    else
    {	/* free both if same client owns both */
	thisDev->sync.state = THAWED;
	if (thisDev->sync.other &&
	    (CLIENT_BITS(thisDev->sync.other->resource) ==
	     CLIENT_BITS(grab->resource)))
	    thisDev->sync.other = NullGrab;
    }
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (dev != thisDev)
	{
	    if (otherMode == GrabModeSync)
		dev->sync.other = grab;
	    else
	    {	/* free both if same client owns both */
		if (dev->sync.other &&
		    (CLIENT_BITS(dev->sync.other->resource) ==
		     CLIENT_BITS(grab->resource)))
		    dev->sync.other = NullGrab;
	    }
	}
    }
    ComputeFreezes();
}

/**
 * Activate a pointer grab on the given device. A pointer grab will cause all
 * core pointer events to be delivered to the grabbing client only. Can cause
 * the cursor to change if a grab cursor is set.
 * 
 * As a pointer grab can only be issued on the core devices, mouse is always
 * inputInfo.pointer. Extension devices are set up for ActivateKeyboardGrab().
 * 
 * @param mouse The device to grab.
 * @param grab The grab structure, needs to be setup.
 * @param autoGrab True if the grab was caused by a button down event and not
 * explicitely by a client. 
 */
void
ActivatePointerGrab(DeviceIntPtr mouse, GrabPtr grab, 
                    TimeStamp time, Bool autoGrab)
{
    WindowPtr oldWin = (mouse->grab) ? mouse->grab->window
				     : sprite.win;

    if (grab->confineTo)
    {
	if (grab->confineTo->drawable.pScreen != sprite.hotPhys.pScreen)
	    sprite.hotPhys.x = sprite.hotPhys.y = 0;
	ConfineCursorToWindow(grab->confineTo, FALSE, TRUE);
    }
    DoEnterLeaveEvents(oldWin, grab->window, NotifyGrab);
    mouse->valuator->motionHintWindow = NullWindow;
    if (syncEvents.playingEvents)
	mouse->grabTime = syncEvents.time;
    else
	mouse->grabTime = time;
    if (grab->cursor)
	grab->cursor->refcnt++;
    mouse->activeGrab = *grab;
    mouse->grab = &mouse->activeGrab;
    mouse->fromPassiveGrab = autoGrab;
    PostNewCursor();
    CheckGrabForSyncs(mouse,(Bool)grab->pointerMode, (Bool)grab->keyboardMode);
}

/**
 * Delete grab on given device, update the sprite.
 *
 * As a pointer grab can only be issued on the core devices, mouse is always
 * inputInfo.pointer. Extension devices are set up for ActivateKeyboardGrab().
 */
void
DeactivatePointerGrab(DeviceIntPtr mouse)
{
    GrabPtr grab = mouse->grab;
    DeviceIntPtr dev;

    mouse->valuator->motionHintWindow = NullWindow;
    mouse->grab = NullGrab;
    mouse->sync.state = NOT_GRABBED;
    mouse->fromPassiveGrab = FALSE;
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (dev->sync.other == grab)
	    dev->sync.other = NullGrab;
    }
    DoEnterLeaveEvents(grab->window, sprite.win, NotifyUngrab);
    if (grab->confineTo)
	ConfineCursorToWindow(ROOT, FALSE, FALSE);
    PostNewCursor();
    if (grab->cursor)
	FreeCursor(grab->cursor, (Cursor)0);
    ComputeFreezes();
}

/**
 * Activate a keyboard grab on the given device. 
 *
 * Extension devices have ActivateKeyboardGrab() set as their grabbing proc.
 */
void
ActivateKeyboardGrab(DeviceIntPtr keybd, GrabPtr grab, TimeStamp time, Bool passive)
{
    WindowPtr oldWin;

    if (keybd->grab)
	oldWin = keybd->grab->window;
    else if (keybd->focus)
	oldWin = keybd->focus->win;
    else
	oldWin = sprite.win;
    if (oldWin == FollowKeyboardWin)
	oldWin = inputInfo.keyboard->focus->win;
    if (keybd->valuator)
	keybd->valuator->motionHintWindow = NullWindow;
    DoFocusEvents(keybd, oldWin, grab->window, NotifyGrab);
    if (syncEvents.playingEvents)
	keybd->grabTime = syncEvents.time;
    else
	keybd->grabTime = time;
    keybd->activeGrab = *grab;
    keybd->grab = &keybd->activeGrab;
    keybd->fromPassiveGrab = passive;
    CheckGrabForSyncs(keybd, (Bool)grab->keyboardMode, (Bool)grab->pointerMode);
}

/**
 * Delete keyboard grab for the given device. 
 */
void
DeactivateKeyboardGrab(DeviceIntPtr keybd)
{
    GrabPtr grab = keybd->grab;
    DeviceIntPtr dev;
    WindowPtr focusWin = keybd->focus ? keybd->focus->win
					       : sprite.win;

    if (focusWin == FollowKeyboardWin)
	focusWin = inputInfo.keyboard->focus->win;
    if (keybd->valuator)
	keybd->valuator->motionHintWindow = NullWindow;
    keybd->grab = NullGrab;
    keybd->sync.state = NOT_GRABBED;
    keybd->fromPassiveGrab = FALSE;
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (dev->sync.other == grab)
	    dev->sync.other = NullGrab;
    }
    DoFocusEvents(keybd, grab->window, focusWin, NotifyUngrab);
    ComputeFreezes();
}

void
AllowSome(ClientPtr client, TimeStamp time, DeviceIntPtr thisDev, int newState)
{
    Bool thisGrabbed, otherGrabbed, othersFrozen, thisSynced;
    TimeStamp grabTime;
    DeviceIntPtr dev;

    thisGrabbed = thisDev->grab && SameClient(thisDev->grab, client);
    thisSynced = FALSE;
    otherGrabbed = FALSE;
    othersFrozen = TRUE;
    grabTime = thisDev->grabTime;
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
	if (dev == thisDev)
	    continue;
	if (dev->grab && SameClient(dev->grab, client))
	{
	    if (!(thisGrabbed || otherGrabbed) ||
		(CompareTimeStamps(dev->grabTime, grabTime) == LATER))
		grabTime = dev->grabTime;
	    otherGrabbed = TRUE;
	    if (thisDev->sync.other == dev->grab)
		thisSynced = TRUE;
	    if (dev->sync.state < FROZEN)
		othersFrozen = FALSE;
	}
	else if (!dev->sync.other || !SameClient(dev->sync.other, client))
	    othersFrozen = FALSE;
    }
    if (!((thisGrabbed && thisDev->sync.state >= FROZEN) || thisSynced))
	return;
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	(CompareTimeStamps(time, grabTime) == EARLIER))
	return;
    switch (newState)
    {
	case THAWED:	 	       /* Async */
	    if (thisGrabbed)
		thisDev->sync.state = THAWED;
	    if (thisSynced)
		thisDev->sync.other = NullGrab;
	    ComputeFreezes();
	    break;
	case FREEZE_NEXT_EVENT:		/* Sync */
	    if (thisGrabbed)
	    {
		thisDev->sync.state = FREEZE_NEXT_EVENT;
		if (thisSynced)
		    thisDev->sync.other = NullGrab;
		ComputeFreezes();
	    }
	    break;
	case THAWED_BOTH:		/* AsyncBoth */
	    if (othersFrozen)
	    {
		for (dev = inputInfo.devices; dev; dev = dev->next)
		{
		    if (dev->grab && SameClient(dev->grab, client))
			dev->sync.state = THAWED;
		    if (dev->sync.other && SameClient(dev->sync.other, client))
			dev->sync.other = NullGrab;
		}
		ComputeFreezes();
	    }
	    break;
	case FREEZE_BOTH_NEXT_EVENT:	/* SyncBoth */
	    if (othersFrozen)
	    {
		for (dev = inputInfo.devices; dev; dev = dev->next)
		{
		    if (dev->grab && SameClient(dev->grab, client))
			dev->sync.state = FREEZE_BOTH_NEXT_EVENT;
		    if (dev->sync.other && SameClient(dev->sync.other, client))
			dev->sync.other = NullGrab;
		}
		ComputeFreezes();
	    }
	    break;
	case NOT_GRABBED:		/* Replay */
	    if (thisGrabbed && thisDev->sync.state == FROZEN_WITH_EVENT)
	    {
		if (thisSynced)
		    thisDev->sync.other = NullGrab;
		syncEvents.replayDev = thisDev;
		syncEvents.replayWin = thisDev->grab->window;
		(*thisDev->DeactivateGrab)(thisDev);
		syncEvents.replayDev = (DeviceIntPtr)NULL;
	    }
	    break;
	case THAW_OTHERS:		/* AsyncOthers */
	    if (othersFrozen)
	    {
		for (dev = inputInfo.devices; dev; dev = dev->next)
		{
		    if (dev == thisDev)
			continue;
		    if (dev->grab && SameClient(dev->grab, client))
			dev->sync.state = THAWED;
		    if (dev->sync.other && SameClient(dev->sync.other, client))
			dev->sync.other = NullGrab;
		}
		ComputeFreezes();
	    }
	    break;
    }
}

/**
 * Server-side protocol handling for AllowEvents request.
 *
 * Release some events from a frozen device. Only applicable for core devices.
 */
int
ProcAllowEvents(ClientPtr client)
{
    TimeStamp		time;
    DeviceIntPtr	mouse = inputInfo.pointer;
    DeviceIntPtr	keybd = inputInfo.keyboard;
    REQUEST(xAllowEventsReq);

    REQUEST_SIZE_MATCH(xAllowEventsReq);
    time = ClientTimeToServerTime(stuff->time);
    switch (stuff->mode)
    {
	case ReplayPointer:
	    AllowSome(client, time, mouse, NOT_GRABBED);
	    break;
	case SyncPointer: 
	    AllowSome(client, time, mouse, FREEZE_NEXT_EVENT);
	    break;
	case AsyncPointer: 
	    AllowSome(client, time, mouse, THAWED);
	    break;
	case ReplayKeyboard: 
	    AllowSome(client, time, keybd, NOT_GRABBED);
	    break;
	case SyncKeyboard: 
	    AllowSome(client, time, keybd, FREEZE_NEXT_EVENT);
	    break;
	case AsyncKeyboard: 
	    AllowSome(client, time, keybd, THAWED);
	    break;
	case SyncBoth:
	    AllowSome(client, time, keybd, FREEZE_BOTH_NEXT_EVENT);
	    break;
	case AsyncBoth:
	    AllowSome(client, time, keybd, THAWED_BOTH);
	    break;
	default: 
	    client->errorValue = stuff->mode;
	    return BadValue;
    }
    return Success;
}

/**
 * Deactivate grabs from any device that has been grabbed by the client.
 */
void
ReleaseActiveGrabs(ClientPtr client)
{
    DeviceIntPtr dev;
    Bool    done;

    /* XXX CloseDownClient should remove passive grabs before
     * releasing active grabs.
     */
    do {
    	done = TRUE;
    	for (dev = inputInfo.devices; dev; dev = dev->next)
    	{
	    if (dev->grab && SameClient(dev->grab, client))
	    {
	    	(*dev->DeactivateGrab)(dev);
	    	done = FALSE;
	    }
    	}
    } while (!done);
}

/**************************************************************************
 *            The following procedures deal with delivering events        *
 **************************************************************************/

/**
 * Deliver the given events to the given client.
 *
 * More than one event may be delivered at a time. This is the case with
 * DeviceMotionNotifies which may be followed by DeviceValuator events.
 *
 * TryClientEvents() is the last station before actually writing the events to
 * the socket. Anything that is not filtered here, will get delivered to the
 * client. 
 * An event is only delivered if 
 *   - mask and filter match up.
 *   - no other client has a grab on the device that caused the event.
 * 
 *
 * @param client The target client to deliver to.
 * @param pEvents The events to be delivered.
 * @param count Number of elements in pEvents.
 * @param mask Event mask as set by the window.
 * @param filter Mask based on event type.
 * @param grab Possible grab on the device that caused the event. 
 *
 * @return 1 if event was delivered, 0 if not or -1 if grab was not set by the
 * client.
 */
_X_EXPORT int
TryClientEvents (ClientPtr client, xEvent *pEvents, int count, Mask mask, 
                 Mask filter, GrabPtr grab)
{
    int i;
    int type;

#ifdef DEBUG_EVENTS
    ErrorF("Event([%d, %d], mask=0x%x), client=%d",
	pEvents->u.u.type, pEvents->u.u.detail, mask, client->index);
#endif
    if ((client) && (client != serverClient) && (!client->clientGone) &&
	((filter == CantBeFiltered) || (mask & filter)))
    {
	if (grab && !SameClient(grab, client))
	    return -1; /* don't send, but notify caller */
	type = pEvents->u.u.type;
	if (type == MotionNotify)
	{
	    if (mask & PointerMotionHintMask)
	    {
		if (WID(inputInfo.pointer->valuator->motionHintWindow) ==
		    pEvents->u.keyButtonPointer.event)
		{
#ifdef DEBUG_EVENTS
		    ErrorF("\n");
	    ErrorF("motionHintWindow == keyButtonPointer.event\n");
#endif
		    return 1; /* don't send, but pretend we did */
		}
		pEvents->u.u.detail = NotifyHint;
	    }
	    else
	    {
		pEvents->u.u.detail = NotifyNormal;
	    }
	}
#ifdef XINPUT
	else
	{
	    if ((type == DeviceMotionNotify) &&
		MaybeSendDeviceMotionNotifyHint
			((deviceKeyButtonPointer*)pEvents, mask) != 0)
		return 1;
	}
#endif
	type &= 0177;
	if (type != KeymapNotify)
	{
	    /* all extension events must have a sequence number */
	    for (i = 0; i < count; i++)
		pEvents[i].u.u.sequenceNumber = client->sequence;
	}

	if (BitIsOn(criticalEvents, type))
	{
#ifdef SMART_SCHEDULE
	    if (client->smart_priority < SMART_MAX_PRIORITY)
		client->smart_priority++;
#endif
	    SetCriticalOutputPending();
	}

	WriteEventsToClient(client, count, pEvents);
#ifdef DEBUG_EVENTS
	ErrorF(  " delivered\n");
#endif
	return 1;
    }
    else
    {
#ifdef DEBUG_EVENTS
	ErrorF("\n");
#endif
	return 0;
    }
}

/**
 * Deliver events to a window. At this point, we do not yet know if the event
 * actually needs to be delivered. May activate a grab if the event is a
 * button press.
 *
 * More than one event may be delivered at a time. This is the case with
 * DeviceMotionNotifies which may be followed by DeviceValuator events.
 * 
 * @param pWin The window that would get the event.
 * @param pEvents The events to be delivered.
 * @param count Number of elements in pEvents.
 * @param filter Mask based on event type.
 * @param grab Possible grab on the device that caused the event. 
 * @param mskidx Mask index, depending on device that caused event.
 *
 * @return Number of events delivered to various clients.
 */
int
DeliverEventsToWindow(WindowPtr pWin, xEvent *pEvents, int count, 
                      Mask filter, GrabPtr grab, int mskidx)
{
    int deliveries = 0, nondeliveries = 0;
    int attempt;
    InputClients *other;
    ClientPtr client = NullClient;
    Mask deliveryMask = 0; /* If a grab occurs due to a button press, then
		              this mask is the mask of the grab. */
    int type = pEvents->u.u.type;

    /* CantBeFiltered means only window owner gets the event */
    if ((filter == CantBeFiltered) || !(type & EXTENSION_EVENT_BASE))
    {
	/* if nobody ever wants to see this event, skip some work */
	if (filter != CantBeFiltered &&
	    !((wOtherEventMasks(pWin)|pWin->eventMask) & filter))
	    return 0;
	if (XaceHook(XACE_RECEIVE_ACCESS, wClient(pWin), pWin, pEvents, count))
	    /* do nothing */;
	else if ( (attempt = TryClientEvents(wClient(pWin), pEvents, count,
					     pWin->eventMask, filter, grab)) )
	{
	    if (attempt > 0)
	    {
		deliveries++;
		client = wClient(pWin);
		deliveryMask = pWin->eventMask;
	    } else
		nondeliveries--;
	}
    }
    if (filter != CantBeFiltered)
    {
	if (type & EXTENSION_EVENT_BASE)
	{
	    OtherInputMasks *inputMasks;

	    inputMasks = wOtherInputMasks(pWin);
	    if (!inputMasks ||
		!(inputMasks->inputEvents[mskidx] & filter))
		return 0;
	    other = inputMasks->inputClients;
	}
	else
	    other = (InputClients *)wOtherClients(pWin);
	for (; other; other = other->next)
	{
	    if (XaceHook(XACE_RECEIVE_ACCESS, rClient(other), pWin, pEvents,
			 count))
		/* do nothing */;
	    else if ( (attempt = TryClientEvents(rClient(other), pEvents, count,
					  other->mask[mskidx], filter, grab)) )
	    {
		if (attempt > 0)
		{
		    deliveries++;
		    client = rClient(other);
		    deliveryMask = other->mask[mskidx];
		} else
		    nondeliveries--;
	    }
	}
    }
    if ((type == ButtonPress) && deliveries && (!grab))
    {
	GrabRec tempGrab;

	tempGrab.device = inputInfo.pointer;
	tempGrab.resource = client->clientAsMask;
	tempGrab.window = pWin;
	tempGrab.ownerEvents = (deliveryMask & OwnerGrabButtonMask) ? TRUE : FALSE;
	tempGrab.eventMask = deliveryMask;
	tempGrab.keyboardMode = GrabModeAsync;
	tempGrab.pointerMode = GrabModeAsync;
	tempGrab.confineTo = NullWindow;
	tempGrab.cursor = NullCursor;
	(*inputInfo.pointer->ActivateGrab)(inputInfo.pointer, &tempGrab,
					   currentTime, TRUE);
    }
    else if ((type == MotionNotify) && deliveries)
	inputInfo.pointer->valuator->motionHintWindow = pWin;
#ifdef XINPUT
    else
    {
	if (((type == DeviceMotionNotify)
#ifdef XKB
	     || (type == DeviceButtonPress)
#endif
	    ) && deliveries)
	    CheckDeviceGrabAndHintWindow (pWin, type,
					  (deviceKeyButtonPointer*) pEvents,
					  grab, client, deliveryMask);
    }
#endif
    if (deliveries)
	return deliveries;
    return nondeliveries;
}

/* If the event goes to dontClient, don't send it and return 0.  if
   send works,  return 1 or if send didn't work, return 2.
   Only works for core events.
*/

#ifdef PANORAMIX
static int 
XineramaTryClientEventsResult(
    ClientPtr client,
    GrabPtr grab,
    Mask mask, 
    Mask filter
){
    if ((client) && (client != serverClient) && (!client->clientGone) &&
        ((filter == CantBeFiltered) || (mask & filter)))
    {
        if (grab && !SameClient(grab, client)) return -1;
	else return 1;
    }
    return 0;
}
#endif

/**
 * Try to deliver events to the interested parties.
 *
 * @param pWin The window that would get the event.
 * @param pEvents The events to be delivered.
 * @param count Number of elements in pEvents.
 * @param filter Mask based on event type.
 * @param dontClient Don't deliver to the dontClient.
 */
int
MaybeDeliverEventsToClient(WindowPtr pWin, xEvent *pEvents, 
                           int count, Mask filter, ClientPtr dontClient)
{
    OtherClients *other;


    if (pWin->eventMask & filter)
    {
        if (wClient(pWin) == dontClient)
	    return 0;
#ifdef PANORAMIX
	if(!noPanoramiXExtension && pWin->drawable.pScreen->myNum) 
	    return XineramaTryClientEventsResult(
			wClient(pWin), NullGrab, pWin->eventMask, filter);
#endif
	if (XaceHook(XACE_RECEIVE_ACCESS, wClient(pWin), pWin, pEvents, count))
	    return 1; /* don't send, but pretend we did */
	return TryClientEvents(wClient(pWin), pEvents, count,
			       pWin->eventMask, filter, NullGrab);
    }
    for (other = wOtherClients(pWin); other; other = other->next)
    {
	if (other->mask & filter)
	{
            if (SameClient(other, dontClient))
		return 0;
#ifdef PANORAMIX
	    if(!noPanoramiXExtension && pWin->drawable.pScreen->myNum) 
	      return XineramaTryClientEventsResult(
			rClient(other), NullGrab, other->mask, filter);
#endif
	    if (XaceHook(XACE_RECEIVE_ACCESS, rClient(other), pWin, pEvents,
			 count))
		return 1; /* don't send, but pretend we did */
	    return TryClientEvents(rClient(other), pEvents, count,
				   other->mask, filter, NullGrab);
	}
    }
    return 2;
}

/**
 * Adjust event fields to comply with the window properties.
 *
 * @param xE Event to be modified in place
 * @param pWin The window to get the information from.
 * @param child Child window setting for event (if applicable)
 * @param calcChild If True, calculate the child window.
 */
static void
FixUpEventFromWindow(
    xEvent *xE,
    WindowPtr pWin,
    Window child,
    Bool calcChild)
{
    if (calcChild)
    {
        WindowPtr w=spriteTrace[spriteTraceGood-1];
	/* If the search ends up past the root should the child field be 
	 	set to none or should the value in the argument be passed 
		through. It probably doesn't matter since everyone calls 
		this function with child == None anyway. */

        while (w) 
        {
            /* If the source window is same as event window, child should be
		none.  Don't bother going all all the way back to the root. */

 	    if (w == pWin)
	    { 
   		child = None;
 		break;
	    }
	    
	    if (w->parent == pWin)
	    {
		child = w->drawable.id;
		break;
            }
 	    w = w->parent;
        } 	    
    }
    XE_KBPTR.root = ROOT->drawable.id;
    XE_KBPTR.event = pWin->drawable.id;
    if (sprite.hot.pScreen == pWin->drawable.pScreen)
    {
	XE_KBPTR.sameScreen = xTrue;
	XE_KBPTR.child = child;
	XE_KBPTR.eventX =
	XE_KBPTR.rootX - pWin->drawable.x;
	XE_KBPTR.eventY =
	XE_KBPTR.rootY - pWin->drawable.y;
    }
    else
    {
	XE_KBPTR.sameScreen = xFalse;
	XE_KBPTR.child = None;
	XE_KBPTR.eventX = 0;
	XE_KBPTR.eventY = 0;
    }
}

/**
 * Deliver events caused by input devices. Called for all core input events
 * and XI events. No filtering of events happens before DeliverDeviceEvents(),
 * it will be called for any event that comes out of the event queue.
 * 
 * For all core events, dev is either inputInfo.pointer or inputInfo.keyboard.
 * For all extension events, dev is the device that caused the event.
 *
 * @param pWin Window to deliver event to.
 * @param xE Events to deliver.
 * @param grab Possible grab on a device.
 * @param stopAt Don't recurse up to the root window.
 * @param dev The device that is responsible for the event.
 * @param count number of events in xE.
 *
 */
int
DeliverDeviceEvents(WindowPtr pWin, xEvent *xE, GrabPtr grab, 
                    WindowPtr stopAt, DeviceIntPtr dev, int count)
{
    Window child = None;
    int type = xE->u.u.type;
    Mask filter = filters[type];
    int deliveries = 0;

    if (XaceHook(XACE_SEND_ACCESS, NULL, dev, pWin, xE, count))
	return 0;

    if (type & EXTENSION_EVENT_BASE)
    {
	OtherInputMasks *inputMasks;
	int mskidx = dev->id;

	inputMasks = wOtherInputMasks(pWin);
	if (inputMasks && !(filter & inputMasks->deliverableEvents[mskidx]))
	    return 0;
	while (pWin)
	{
	    if (inputMasks && (inputMasks->inputEvents[mskidx] & filter))
	    {
		FixUpEventFromWindow(xE, pWin, child, FALSE);
		deliveries = DeliverEventsToWindow(pWin, xE, count, filter,
						   grab, mskidx);
		if (deliveries > 0)
		    return deliveries;
	    }
	    if ((deliveries < 0) ||
		(pWin == stopAt) ||
		(inputMasks &&
		 (filter & inputMasks->dontPropagateMask[mskidx])))
		return 0;
	    child = pWin->drawable.id;
	    pWin = pWin->parent;
	    if (pWin)
		inputMasks = wOtherInputMasks(pWin);
	}
    }
    else
    {
	if (!(filter & pWin->deliverableEvents))
	    return 0;
	while (pWin)
	{
	    if ((wOtherEventMasks(pWin)|pWin->eventMask) & filter)
	    {
		FixUpEventFromWindow(xE, pWin, child, FALSE);
		deliveries = DeliverEventsToWindow(pWin, xE, count, filter,
						   grab, 0);
		if (deliveries > 0)
		    return deliveries;
	    }
	    if ((deliveries < 0) ||
		(pWin == stopAt) ||
		(filter & wDontPropagateMask(pWin)))
		return 0;
	    child = pWin->drawable.id;
	    pWin = pWin->parent;
	}
    }
    return 0;
}

/**
 * Deliver event to a window and it's immediate parent. Used for most window
 * events (CreateNotify, ConfigureNotify, etc.). Not useful for events that
 * propagate up the tree or extension events 
 *
 * In case of a ReparentNotify event, the event will be delivered to the
 * otherParent as well.
 *
 * @param pWin Window to deliver events to.
 * @param xE Events to deliver.
 * @param count number of events in xE.
 * @param otherParent Used for ReparentNotify events.
 */
_X_EXPORT int
DeliverEvents(WindowPtr pWin, xEvent *xE, int count, 
              WindowPtr otherParent)
{
    Mask filter;
    int     deliveries;

#ifdef PANORAMIX
    if(!noPanoramiXExtension && pWin->drawable.pScreen->myNum)
	return count;
#endif

    if (!count)
	return 0;
    filter = filters[xE->u.u.type];
    if ((filter & SubstructureNotifyMask) && (xE->u.u.type != CreateNotify))
	xE->u.destroyNotify.event = pWin->drawable.id;
    if (filter != StructureAndSubMask)
	return DeliverEventsToWindow(pWin, xE, count, filter, NullGrab, 0);
    deliveries = DeliverEventsToWindow(pWin, xE, count, StructureNotifyMask,
				       NullGrab, 0);
    if (pWin->parent)
    {
	xE->u.destroyNotify.event = pWin->parent->drawable.id;
	deliveries += DeliverEventsToWindow(pWin->parent, xE, count,
					    SubstructureNotifyMask, NullGrab,
					    0);
	if (xE->u.u.type == ReparentNotify)
	{
	    xE->u.destroyNotify.event = otherParent->drawable.id;
	    deliveries += DeliverEventsToWindow(otherParent, xE, count,
						SubstructureNotifyMask,
						NullGrab, 0);
	}
    }
    return deliveries;
}


static Bool 
PointInBorderSize(WindowPtr pWin, int x, int y)
{
    BoxRec box;

    if(POINT_IN_REGION(pWin->drawable.pScreen, &pWin->borderSize, x, y, &box))
	return TRUE;

#ifdef PANORAMIX
    if(!noPanoramiXExtension && XineramaSetWindowPntrs(pWin)) {
	int i;

	for(i = 1; i < PanoramiXNumScreens; i++) {
	   if(POINT_IN_REGION(sprite.screen, 
			&sprite.windows[i]->borderSize, 
			x + panoramiXdataPtr[0].x - panoramiXdataPtr[i].x, 
			y + panoramiXdataPtr[0].y - panoramiXdataPtr[i].y, 
			&box))
		return TRUE;
	}
    }
#endif
    return FALSE;
}

/**
 * Traversed from the root window to the window at the position x/y. While
 * traversing, it sets up the traversal history in the spriteTrace array.
 * After completing, the spriteTrace history is set in the following way:
 *   spriteTrace[0] ... root window
 *   spriteTrace[1] ... top level window that encloses x/y
 *       ...
 *   spriteTrace[spriteTraceGood - 1] ... window at x/y
 *
 * @returns the window at the given coordinates.
 */
static WindowPtr 
XYToWindow(int x, int y)
{
    WindowPtr  pWin;
    BoxRec		box;

    spriteTraceGood = 1;	/* root window still there */
    pWin = ROOT->firstChild;
    while (pWin)
    {
	if ((pWin->mapped) &&
	    (x >= pWin->drawable.x - wBorderWidth (pWin)) &&
	    (x < pWin->drawable.x + (int)pWin->drawable.width +
	     wBorderWidth(pWin)) &&
	    (y >= pWin->drawable.y - wBorderWidth (pWin)) &&
	    (y < pWin->drawable.y + (int)pWin->drawable.height +
	     wBorderWidth (pWin))
#ifdef SHAPE
	    /* When a window is shaped, a further check
	     * is made to see if the point is inside
	     * borderSize
	     */
	    && (!wBoundingShape(pWin) || PointInBorderSize(pWin, x, y))
	    && (!wInputShape(pWin) ||
		POINT_IN_REGION(pWin->drawable.pScreen,
				wInputShape(pWin),
				x - pWin->drawable.x,
				y - pWin->drawable.y, &box))
#endif
	    )
	{
	    if (spriteTraceGood >= spriteTraceSize)
	    {
		spriteTraceSize += 10;
		Must_have_memory = TRUE; /* XXX */
		spriteTrace = (WindowPtr *)xrealloc(
		    spriteTrace, spriteTraceSize*sizeof(WindowPtr));
		Must_have_memory = FALSE; /* XXX */
	    }
	    spriteTrace[spriteTraceGood++] = pWin;
	    pWin = pWin->firstChild;
	}
	else
	    pWin = pWin->nextSib;
    }
    return spriteTrace[spriteTraceGood-1];
}

/**
 * Update the sprite coordinates based on the event. Update the cursor
 * position, then update the event with the new coordinates that may have been
 * changed. If the window underneath the sprite has changed, change to new
 * cursor and send enter/leave events.
 */
static Bool
CheckMotion(xEvent *xE)
{
    WindowPtr prevSpriteWin = sprite.win;

#ifdef PANORAMIX
    if(!noPanoramiXExtension)
	return XineramaCheckMotion(xE);
#endif

    if (xE && !syncEvents.playingEvents)
    {
	if (sprite.hot.pScreen != sprite.hotPhys.pScreen)
	{
	    sprite.hot.pScreen = sprite.hotPhys.pScreen;
	    ROOT = WindowTable[sprite.hot.pScreen->myNum];
	}
	sprite.hot.x = XE_KBPTR.rootX;
	sprite.hot.y = XE_KBPTR.rootY;
	if (sprite.hot.x < sprite.physLimits.x1)
	    sprite.hot.x = sprite.physLimits.x1;
	else if (sprite.hot.x >= sprite.physLimits.x2)
	    sprite.hot.x = sprite.physLimits.x2 - 1;
	if (sprite.hot.y < sprite.physLimits.y1)
	    sprite.hot.y = sprite.physLimits.y1;
	else if (sprite.hot.y >= sprite.physLimits.y2)
	    sprite.hot.y = sprite.physLimits.y2 - 1;
#ifdef SHAPE
	if (sprite.hotShape)
	    ConfineToShape(sprite.hotShape, &sprite.hot.x, &sprite.hot.y);
#endif
#ifdef XEVIE
        xeviehot.x = sprite.hot.x;
        xeviehot.y = sprite.hot.y;
#endif
	sprite.hotPhys = sprite.hot;
	if ((sprite.hotPhys.x != XE_KBPTR.rootX) ||
	    (sprite.hotPhys.y != XE_KBPTR.rootY))
	{
	    (*sprite.hotPhys.pScreen->SetCursorPosition)(
		sprite.hotPhys.pScreen,
		sprite.hotPhys.x, sprite.hotPhys.y, FALSE);
	}
	XE_KBPTR.rootX = sprite.hot.x;
	XE_KBPTR.rootY = sprite.hot.y;
    }

#ifdef XEVIE
    xeviewin =
#endif
    sprite.win = XYToWindow(sprite.hot.x, sprite.hot.y);
#ifdef notyet
    if (!(sprite.win->deliverableEvents &
	  Motion_Filter(inputInfo.pointer->button))
	!syncEvents.playingEvents)
    {
	/* XXX Do PointerNonInterestBox here */
    }
#endif
    if (sprite.win != prevSpriteWin)
    {
	if (prevSpriteWin != NullWindow) {
	    if (!xE)
		UpdateCurrentTimeIf();
	    DoEnterLeaveEvents(prevSpriteWin, sprite.win, NotifyNormal);
	}
	PostNewCursor();
        return FALSE;
    }
    return TRUE;
}

/**
 * Windows have restructured, we need to update the sprite position and the
 * sprite's cursor.
 */
_X_EXPORT void
WindowsRestructured(void)
{
    (void) CheckMotion((xEvent *)NULL);
}

#ifdef PANORAMIX
/* This was added to support reconfiguration under Xdmx.  The problem is
 * that if the 0th screen (i.e., WindowTable[0]) is moved to an origin
 * other than 0,0, the information in the private sprite structure must
 * be updated accordingly, or XYToWindow (and other routines) will not
 * compute correctly. */
void ReinitializeRootWindow(WindowPtr win, int xoff, int yoff)
{
    GrabPtr   grab;

    if (noPanoramiXExtension) return;
    
    sprite.hot.x        -= xoff;
    sprite.hot.y        -= yoff;

    sprite.hotPhys.x    -= xoff;
    sprite.hotPhys.y    -= yoff;

    sprite.hotLimits.x1 -= xoff; 
    sprite.hotLimits.y1 -= yoff;
    sprite.hotLimits.x2 -= xoff;
    sprite.hotLimits.y2 -= yoff;

    if (REGION_NOTEMPTY(sprite.screen, &sprite.Reg1))
        REGION_TRANSLATE(sprite.screen, &sprite.Reg1,    xoff, yoff);
    if (REGION_NOTEMPTY(sprite.screen, &sprite.Reg2))
        REGION_TRANSLATE(sprite.screen, &sprite.Reg2,    xoff, yoff);

    /* FIXME: if we call ConfineCursorToWindow, must we do anything else? */
    if ((grab = inputInfo.pointer->grab) && grab->confineTo) {
	if (grab->confineTo->drawable.pScreen != sprite.hotPhys.pScreen)
	    sprite.hotPhys.x = sprite.hotPhys.y = 0;
	ConfineCursorToWindow(grab->confineTo, TRUE, TRUE);
    } else
	ConfineCursorToWindow(WindowTable[sprite.hotPhys.pScreen->myNum],
			      TRUE, FALSE);
}
#endif

/**
 * Set the given window to sane values, display the cursor in the center of
 * the screen. Called from main() with the root window on the first screen.
 */
void
DefineInitialRootWindow(WindowPtr win)
{
    ScreenPtr pScreen = win->drawable.pScreen;

    sprite.hotPhys.pScreen = pScreen;
    sprite.hotPhys.x = pScreen->width / 2;
    sprite.hotPhys.y = pScreen->height / 2;
    sprite.hot = sprite.hotPhys;
    sprite.hotLimits.x2 = pScreen->width;
    sprite.hotLimits.y2 = pScreen->height;
#ifdef XEVIE
    xeviewin =
#endif
    sprite.win = win;
    sprite.current = wCursor (win);
    sprite.current->refcnt++;
    spriteTraceGood = 1;
    ROOT = win;
    (*pScreen->CursorLimits) (
	pScreen, sprite.current, &sprite.hotLimits, &sprite.physLimits);
    sprite.confined = FALSE;
    (*pScreen->ConstrainCursor) (pScreen, &sprite.physLimits);
    (*pScreen->SetCursorPosition) (pScreen, sprite.hot.x, sprite.hot.y, FALSE);
    (*pScreen->DisplayCursor) (pScreen, sprite.current);

#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
	sprite.hotLimits.x1 = -panoramiXdataPtr[0].x;
	sprite.hotLimits.y1 = -panoramiXdataPtr[0].y;
	sprite.hotLimits.x2 = PanoramiXPixWidth  - panoramiXdataPtr[0].x;
	sprite.hotLimits.y2 = PanoramiXPixHeight - panoramiXdataPtr[0].y;
	sprite.physLimits = sprite.hotLimits;
	sprite.confineWin = NullWindow;
#ifdef SHAPE
        sprite.hotShape = NullRegion;
#endif
	sprite.screen = pScreen;
	/* gotta UNINIT these someplace */
	REGION_NULL(pScreen, &sprite.Reg1);
	REGION_NULL(pScreen, &sprite.Reg2);
    }
#endif
}

/**
 * Update the mouse sprite info when the server switches from a pScreen to another.
 * Otherwise, the pScreen of the mouse sprite is never updated when we switch
 * from a pScreen to another. Never updating the pScreen of the mouse sprite
 * implies that windows that are in pScreen whose pScreen->myNum >0 will never
 * get pointer events. This is  because in CheckMotion(), sprite.hotPhys.pScreen
 * always points to the first pScreen it has been set by
 * DefineInitialRootWindow().
 *
 * Calling this function is useful for use cases where the server
 * has more than one pScreen.
 * This function is similar to DefineInitialRootWindow() but it does not
 * reset the mouse pointer position.
 * @param win must be the new pScreen we are switching to.
 */
void
UpdateSpriteForScreen(ScreenPtr pScreen)
{
    WindowPtr win = NULL;
    if (!pScreen)
        return ;
    win = WindowTable[pScreen->myNum];

    sprite.hotPhys.pScreen = pScreen;
    sprite.hot = sprite.hotPhys;
    sprite.hotLimits.x2 = pScreen->width;
    sprite.hotLimits.y2 = pScreen->height;
#ifdef XEVIE
    xeviewin =
#endif
    sprite.win = win;
    sprite.current = wCursor (win);
    sprite.current->refcnt++;
    spriteTraceGood = 1;
    ROOT = win;
    (*pScreen->CursorLimits) (pScreen,
                              sprite.current,
                              &sprite.hotLimits,
                              &sprite.physLimits);
    sprite.confined = FALSE;
    (*pScreen->ConstrainCursor) (pScreen, &sprite.physLimits);
    (*pScreen->DisplayCursor) (pScreen, sprite.current);

#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
        sprite.hotLimits.x1 = -panoramiXdataPtr[0].x;
        sprite.hotLimits.y1 = -panoramiXdataPtr[0].y;
        sprite.hotLimits.x2 = PanoramiXPixWidth  - panoramiXdataPtr[0].x;
        sprite.hotLimits.y2 = PanoramiXPixHeight - panoramiXdataPtr[0].y;
        sprite.physLimits = sprite.hotLimits;
        sprite.screen = pScreen;
    }
#endif
}

/*
 * This does not take any shortcuts, and even ignores its argument, since
 * it does not happen very often, and one has to walk up the tree since
 * this might be a newly instantiated cursor for an intermediate window
 * between the one the pointer is in and the one that the last cursor was
 * instantiated from.
 */
void
WindowHasNewCursor(WindowPtr pWin)
{
    PostNewCursor();
}

_X_EXPORT void
NewCurrentScreen(ScreenPtr newScreen, int x, int y)
{
    sprite.hotPhys.x = x;
    sprite.hotPhys.y = y;
#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
	sprite.hotPhys.x += panoramiXdataPtr[newScreen->myNum].x - 
			    panoramiXdataPtr[0].x;
	sprite.hotPhys.y += panoramiXdataPtr[newScreen->myNum].y - 
			    panoramiXdataPtr[0].y;
	if (newScreen != sprite.screen) {
	    sprite.screen = newScreen;
	    /* Make sure we tell the DDX to update its copy of the screen */
	    if(sprite.confineWin)
		XineramaConfineCursorToWindow(sprite.confineWin, TRUE);
	    else
		XineramaConfineCursorToWindow(WindowTable[0], TRUE);
	    /* if the pointer wasn't confined, the DDX won't get 
	       told of the pointer warp so we reposition it here */
	    if(!syncEvents.playingEvents)
		(*sprite.screen->SetCursorPosition)(sprite.screen,
		    sprite.hotPhys.x + panoramiXdataPtr[0].x - 
			panoramiXdataPtr[sprite.screen->myNum].x,
		    sprite.hotPhys.y + panoramiXdataPtr[0].y - 
			panoramiXdataPtr[sprite.screen->myNum].y, FALSE);
	}
    } else 
#endif
    if (newScreen != sprite.hotPhys.pScreen)
	ConfineCursorToWindow(WindowTable[newScreen->myNum], TRUE, FALSE);
}

#ifdef PANORAMIX

static Bool
XineramaPointInWindowIsVisible(
    WindowPtr pWin,
    int x,
    int y
)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    BoxRec box;
    int i, xoff, yoff;

    if (!pWin->realized) return FALSE;

    if (POINT_IN_REGION(pScreen, &pWin->borderClip, x, y, &box))
        return TRUE;
    
    if(!XineramaSetWindowPntrs(pWin)) return FALSE;

    xoff = x + panoramiXdataPtr[0].x;  
    yoff = y + panoramiXdataPtr[0].y;  

    for(i = 1; i < PanoramiXNumScreens; i++) {
	pWin = sprite.windows[i];
	pScreen = pWin->drawable.pScreen;
	x = xoff - panoramiXdataPtr[i].x;
	y = yoff - panoramiXdataPtr[i].y;

	if(POINT_IN_REGION(pScreen, &pWin->borderClip, x, y, &box)
	   && (!wInputShape(pWin) ||
	       POINT_IN_REGION(pWin->drawable.pScreen,
			       wInputShape(pWin),
			       x - pWin->drawable.x, 
			       y - pWin->drawable.y, &box)))
            return TRUE;

    }

    return FALSE;
}

static int
XineramaWarpPointer(ClientPtr client)
{
    WindowPtr	dest = NULL;
    int		x, y, rc;

    REQUEST(xWarpPointerReq);


    if (stuff->dstWid != None) {
	rc = dixLookupWindow(&dest, stuff->dstWid, client, DixReadAccess);
	if (rc != Success)
	    return rc;
    }
    x = sprite.hotPhys.x;
    y = sprite.hotPhys.y;

    if (stuff->srcWid != None)
    {
	int     winX, winY;
 	XID 	winID = stuff->srcWid;
        WindowPtr source;
	
	rc = dixLookupWindow(&source, winID, client, DixReadAccess);
	if (rc != Success)
	    return rc;

	winX = source->drawable.x;
	winY = source->drawable.y;
	if(source == WindowTable[0]) {
	    winX -= panoramiXdataPtr[0].x;
	    winY -= panoramiXdataPtr[0].y;
	}
	if (x < winX + stuff->srcX ||
	    y < winY + stuff->srcY ||
	    (stuff->srcWidth != 0 &&
	     winX + stuff->srcX + (int)stuff->srcWidth < x) ||
	    (stuff->srcHeight != 0 &&
	     winY + stuff->srcY + (int)stuff->srcHeight < y) ||
	    !XineramaPointInWindowIsVisible(source, x, y))
	    return Success;
    }
    if (dest) {
	x = dest->drawable.x;
	y = dest->drawable.y;
	if(dest == WindowTable[0]) {
	    x -= panoramiXdataPtr[0].x;
	    y -= panoramiXdataPtr[0].y;
	}
    } 

    x += stuff->dstX;
    y += stuff->dstY;

    if (x < sprite.physLimits.x1)
	x = sprite.physLimits.x1;
    else if (x >= sprite.physLimits.x2)
	x = sprite.physLimits.x2 - 1;
    if (y < sprite.physLimits.y1)
	y = sprite.physLimits.y1;
    else if (y >= sprite.physLimits.y2)
	y = sprite.physLimits.y2 - 1;
    if (sprite.hotShape)
	ConfineToShape(sprite.hotShape, &x, &y);

    XineramaSetCursorPosition(x, y, TRUE);

    return Success;
}

#endif


/**
 * Server-side protocol handling for WarpPointer request.
 * Warps the cursor position to the coordinates given in the request.
 */
int
ProcWarpPointer(ClientPtr client)
{
    WindowPtr	dest = NULL;
    int		x, y, rc;
    ScreenPtr	newScreen;
    DeviceIntPtr dev;
    REQUEST(xWarpPointerReq);
    REQUEST_SIZE_MATCH(xWarpPointerReq);

    for (dev = inputInfo.devices; dev; dev = dev->next) {
        if ((dev->coreEvents || dev == inputInfo.pointer) && dev->button) {
	    rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixWriteAccess);
	    if (rc != Success)
		return rc;
	}
    }
#ifdef PANORAMIX
    if(!noPanoramiXExtension)
	return XineramaWarpPointer(client);
#endif

    if (stuff->dstWid != None) {
	rc = dixLookupWindow(&dest, stuff->dstWid, client, DixGetAttrAccess);
	if (rc != Success)
	    return rc;
    }
    x = sprite.hotPhys.x;
    y = sprite.hotPhys.y;

    if (stuff->srcWid != None)
    {
	int     winX, winY;
 	XID 	winID = stuff->srcWid;
        WindowPtr source;
	
	rc = dixLookupWindow(&source, winID, client, DixGetAttrAccess);
	if (rc != Success)
	    return rc;

	winX = source->drawable.x;
	winY = source->drawable.y;
	if (source->drawable.pScreen != sprite.hotPhys.pScreen ||
	    x < winX + stuff->srcX ||
	    y < winY + stuff->srcY ||
	    (stuff->srcWidth != 0 &&
	     winX + stuff->srcX + (int)stuff->srcWidth < x) ||
	    (stuff->srcHeight != 0 &&
	     winY + stuff->srcY + (int)stuff->srcHeight < y) ||
	    !PointInWindowIsVisible(source, x, y))
	    return Success;
    }
    if (dest) 
    {
	x = dest->drawable.x;
	y = dest->drawable.y;
	newScreen = dest->drawable.pScreen;
    } else 
	newScreen = sprite.hotPhys.pScreen;

    x += stuff->dstX;
    y += stuff->dstY;

    if (x < 0)
	x = 0;
    else if (x >= newScreen->width)
	x = newScreen->width - 1;
    if (y < 0)
	y = 0;
    else if (y >= newScreen->height)
	y = newScreen->height - 1;

    if (newScreen == sprite.hotPhys.pScreen)
    {
	if (x < sprite.physLimits.x1)
	    x = sprite.physLimits.x1;
	else if (x >= sprite.physLimits.x2)
	    x = sprite.physLimits.x2 - 1;
	if (y < sprite.physLimits.y1)
	    y = sprite.physLimits.y1;
	else if (y >= sprite.physLimits.y2)
	    y = sprite.physLimits.y2 - 1;
#if defined(SHAPE)
	if (sprite.hotShape)
	    ConfineToShape(sprite.hotShape, &x, &y);
#endif
	(*newScreen->SetCursorPosition)(newScreen, x, y, TRUE);
    }
    else if (!PointerConfinedToScreen())
    {
	NewCurrentScreen(newScreen, x, y);
    }
    return Success;
}

static Bool 
BorderSizeNotEmpty(WindowPtr pWin)
{
     if(REGION_NOTEMPTY(sprite.hotPhys.pScreen, &pWin->borderSize))
	return TRUE;

#ifdef PANORAMIX
     if(!noPanoramiXExtension && XineramaSetWindowPntrs(pWin)) {
	int i;

	for(i = 1; i < PanoramiXNumScreens; i++) {
	    if(REGION_NOTEMPTY(sprite.screen, &sprite.windows[i]->borderSize))
		return TRUE;
	}
     }
#endif
     return FALSE;
}

/** 
 * "CheckPassiveGrabsOnWindow" checks to see if the event passed in causes a
 * passive grab set on the window to be activated. 
 * If a passive grab is activated, the event will be delivered to the client.
 * 
 * @param pWin The window that may be subject to a passive grab.
 * @param device Device that caused the event.
 * @param xE List of events (multiple ones for DeviceMotionNotify)
 * @count number of elements in xE.
 */

static Bool
CheckPassiveGrabsOnWindow(
    WindowPtr pWin,
    DeviceIntPtr device,
    xEvent *xE,
    int count)
{
    GrabPtr grab = wPassiveGrabs(pWin);
    GrabRec tempGrab;
    xEvent *dxE;

    if (!grab)
	return FALSE;
    tempGrab.window = pWin;
    tempGrab.device = device;
    tempGrab.type = xE->u.u.type;
    tempGrab.detail.exact = xE->u.u.detail;
    tempGrab.detail.pMask = NULL;
    tempGrab.modifiersDetail.pMask = NULL;
    for (; grab; grab = grab->next)
    {
#ifdef XKB
	DeviceIntPtr	gdev;
	XkbSrvInfoPtr	xkbi;

	gdev= grab->modifierDevice;
	xkbi= gdev->key->xkbInfo;
#endif
	tempGrab.modifierDevice = grab->modifierDevice;
	if ((device == grab->modifierDevice) &&
	    ((xE->u.u.type == KeyPress)
#if defined(XINPUT) && defined(XKB)
	     || (xE->u.u.type == DeviceKeyPress)
#endif
	     ))
	    tempGrab.modifiersDetail.exact =
#ifdef XKB
		(noXkbExtension?gdev->key->prev_state:xkbi->state.grab_mods);
#else
		grab->modifierDevice->key->prev_state;
#endif
	else
	    tempGrab.modifiersDetail.exact =
#ifdef XKB
		(noXkbExtension ? gdev->key->state : xkbi->state.grab_mods);
#else
		grab->modifierDevice->key->state;
#endif
	if (GrabMatchesSecond(&tempGrab, grab) &&
	    (!grab->confineTo ||
	     (grab->confineTo->realized && 
				BorderSizeNotEmpty(grab->confineTo))))
	{
#ifdef XKB
	    if (!noXkbExtension) {
		XE_KBPTR.state &= 0x1f00;
		XE_KBPTR.state |=
				tempGrab.modifiersDetail.exact&(~0x1f00);
	    }
#endif
	    (*device->ActivateGrab)(device, grab, currentTime, TRUE);
 
	    FixUpEventFromWindow(xE, grab->window, None, TRUE);

	    (void) TryClientEvents(rClient(grab), xE, count,
				   filters[xE->u.u.type],
				   filters[xE->u.u.type],  grab);

	    if (device->sync.state == FROZEN_NO_EVENT)
	    {
		if (device->sync.evcount < count)
		{
		    Must_have_memory = TRUE; /* XXX */
		    device->sync.event = (xEvent *)xrealloc(device->sync.event,
							    count*
							    sizeof(xEvent));
		    Must_have_memory = FALSE; /* XXX */
		}
		device->sync.evcount = count;
		for (dxE = device->sync.event; --count >= 0; dxE++, xE++)
		    *dxE = *xE;
	    	device->sync.state = FROZEN_WITH_EVENT;
            }	
	    return TRUE;
	}
    }
    return FALSE;
}

/**
 * CheckDeviceGrabs handles both keyboard and pointer events that may cause
 * a passive grab to be activated.  
 *
 * If the event is a keyboard event, the ancestors of the focus window are
 * traced down and tried to see if they have any passive grabs to be
 * activated.  If the focus window itself is reached and it's descendants
 * contain the pointer, the ancestors of the window that the pointer is in
 * are then traced down starting at the focus window, otherwise no grabs are
 * activated.  
 * If the event is a pointer event, the ancestors of the window that the
 * pointer is in are traced down starting at the root until CheckPassiveGrabs
 * causes a passive grab to activate or all the windows are
 * tried. PRH
 *
 * If a grab is activated, the event has been sent to the client already!
 *
 * @param device The device that caused the event.
 * @param xE The event to handle (most likely {Device}ButtonPress).
 * @param count Number of events in list.
 * @return TRUE if a grab has been activated or false otherwise.
*/

Bool
CheckDeviceGrabs(DeviceIntPtr device, xEvent *xE, 
                 int checkFirst, int count)
{
    int i;
    WindowPtr pWin = NULL;
    FocusClassPtr focus = device->focus;

    if (((xE->u.u.type == ButtonPress)
#if defined(XINPUT) && defined(XKB)
	 || (xE->u.u.type == DeviceButtonPress)
#endif
	 ) && (device->button->buttonsDown != 1))
	return FALSE;

    i = checkFirst;

    if (focus)
    {
	for (; i < focus->traceGood; i++)
	{
	    pWin = focus->trace[i];
	    if (pWin->optional &&
		CheckPassiveGrabsOnWindow(pWin, device, xE, count))
		return TRUE;
	}
  
	if ((focus->win == NoneWin) ||
	    (i >= spriteTraceGood) ||
	    ((i > checkFirst) && (pWin != spriteTrace[i-1])))
	    return FALSE;
    }

    for (; i < spriteTraceGood; i++)
    {
	pWin = spriteTrace[i];
	if (pWin->optional &&
	    CheckPassiveGrabsOnWindow(pWin, device, xE, count))
	    return TRUE;
    }

    return FALSE;
}

/**
 * Called for keyboard events to deliver event to whatever client owns the
 * focus. Event is delivered to the keyboard's focus window, the root window
 * or to the window owning the input focus.
 *
 * @param keybd The keyboard originating the event.
 * @param xE The event list.
 * @param window Window underneath the sprite.
 * @param count number of events in xE.
 */
void
DeliverFocusedEvent(DeviceIntPtr keybd, xEvent *xE, WindowPtr window, int count)
{
    WindowPtr focus = keybd->focus->win;
    int mskidx = 0;

    if (focus == FollowKeyboardWin)
	focus = inputInfo.keyboard->focus->win;
    if (!focus)
	return;
    if (focus == PointerRootWin)
    {
	DeliverDeviceEvents(window, xE, NullGrab, NullWindow, keybd, count);
	return;
    }
    if ((focus == window) || IsParent(focus, window))
    {
	if (DeliverDeviceEvents(window, xE, NullGrab, focus, keybd, count))
	    return;
    }
    if (XaceHook(XACE_SEND_ACCESS, NULL, keybd, focus, xE, count))
	return;
    /* just deliver it to the focus window */
    FixUpEventFromWindow(xE, focus, None, FALSE);
    if (xE->u.u.type & EXTENSION_EVENT_BASE)
	mskidx = keybd->id;
    (void)DeliverEventsToWindow(focus, xE, count, filters[xE->u.u.type],
				NullGrab, mskidx);
}

/**
 * Deliver an event from a device that is currently grabbed. Uses
 * DeliverDeviceEvents() for further delivery if a ownerEvents is set on the
 * grab. If not, TryClientEvents() is used.
 *
 * @param deactivateGrab True if the device's grab should be deactivated.
 */
void
DeliverGrabbedEvent(xEvent *xE, DeviceIntPtr thisDev, 
                    Bool deactivateGrab, int count)
{
    GrabPtr grab = thisDev->grab;
    int deliveries = 0;
    DeviceIntPtr dev;
    xEvent *dxE;

    if (grab->ownerEvents)
    {
	WindowPtr focus;

	if (thisDev->focus)
	{
	    focus = thisDev->focus->win;
	    if (focus == FollowKeyboardWin)
		focus = inputInfo.keyboard->focus->win;
	}
	else
	    focus = PointerRootWin;
	if (focus == PointerRootWin)
	    deliveries = DeliverDeviceEvents(sprite.win, xE, grab, NullWindow,
					     thisDev, count);
	else if (focus && (focus == sprite.win || IsParent(focus, sprite.win)))
	    deliveries = DeliverDeviceEvents(sprite.win, xE, grab, focus,
					     thisDev, count);
	else if (focus)
	    deliveries = DeliverDeviceEvents(focus, xE, grab, focus,
					     thisDev, count);
    }
    if (!deliveries)
    {
	FixUpEventFromWindow(xE, grab->window, None, TRUE);
	if (XaceHook(XACE_SEND_ACCESS, 0, thisDev, grab->window, xE, count) ||
	    XaceHook(XACE_RECEIVE_ACCESS, rClient(grab), grab->window, xE,
		     count))
	    deliveries = 1; /* don't send, but pretend we did */
	else
	    deliveries = TryClientEvents(rClient(grab), xE, count,
					 (Mask)grab->eventMask,
					 filters[xE->u.u.type], grab);
	if (deliveries && (xE->u.u.type == MotionNotify
#ifdef XINPUT
			   || xE->u.u.type == DeviceMotionNotify
#endif
			   ))
	    thisDev->valuator->motionHintWindow = grab->window;
    }
    if (deliveries && !deactivateGrab && (xE->u.u.type != MotionNotify
#ifdef XINPUT
					  && xE->u.u.type != DeviceMotionNotify
#endif
					  ))
	switch (thisDev->sync.state)
	{
	case FREEZE_BOTH_NEXT_EVENT:
	    for (dev = inputInfo.devices; dev; dev = dev->next)
	    {
		if (dev == thisDev)
		    continue;
		FreezeThaw(dev, TRUE);
		if ((dev->sync.state == FREEZE_BOTH_NEXT_EVENT) &&
		    (CLIENT_BITS(dev->grab->resource) ==
		     CLIENT_BITS(thisDev->grab->resource)))
		    dev->sync.state = FROZEN_NO_EVENT;
		else
		    dev->sync.other = thisDev->grab;
	    }
	    /* fall through */
	case FREEZE_NEXT_EVENT:
	    thisDev->sync.state = FROZEN_WITH_EVENT;
	    FreezeThaw(thisDev, TRUE);
	    if (thisDev->sync.evcount < count)
	    {
		Must_have_memory = TRUE; /* XXX */
		thisDev->sync.event = (xEvent *)xrealloc(thisDev->sync.event,
							 count*sizeof(xEvent));
		Must_have_memory = FALSE; /* XXX */
	    }
	    thisDev->sync.evcount = count;
	    for (dxE = thisDev->sync.event; --count >= 0; dxE++, xE++)
		*dxE = *xE;
	    break;
	}
}

/**
 * Main keyboard event processing function for core keyboard events. 
 * Updates the events fields from the current pointer state and delivers the
 * event.
 *
 * For key events, xE will always be a single event.
 *
 * @param xE Event list
 * @param keybd The device that caused an event.
 * @param count Number of elements in xE.
 */
void
#ifdef XKB
CoreProcessKeyboardEvent (xEvent *xE, DeviceIntPtr keybd, int count)
#else
ProcessKeyboardEvent (xEvent *xE, DeviceIntPtr keybd, int count)
#endif
{
    int             key, bit;
    BYTE   *kptr;
    int    i;
    CARD8  modifiers;
    CARD16 mask;
    GrabPtr         grab = keybd->grab;
    Bool            deactivateGrab = FALSE;
    KeyClassPtr keyc = keybd->key;
#ifdef XEVIE
    static Window           rootWin = 0;

    if(!xeviegrabState && xevieFlag && clients[xevieClientIndex] &&
          (xevieMask & xevieFilters[xE->u.u.type])) {
      key = xE->u.u.detail;
      kptr = &keyc->down[key >> 3];
      bit = 1 << (key & 7);
      if((xE->u.u.type == KeyPress &&  (*kptr & bit)) ||
         (xE->u.u.type == KeyRelease && !(*kptr & bit)))
      {} else {
#ifdef XKB
        if(!noXkbExtension)
	    xevieKBEventSent = 1;
#endif
        if(!xevieKBEventSent)
        {
          xeviekb = keybd;
          if(!rootWin) {
	      rootWin = GetCurrentRootWindow()->drawable.id;
          }
          xE->u.keyButtonPointer.event = xeviewin->drawable.id;
          xE->u.keyButtonPointer.root = rootWin;
          xE->u.keyButtonPointer.child = (xeviewin->firstChild) ? xeviewin->firstChild->
drawable.id:0;
          xE->u.keyButtonPointer.rootX = xeviehot.x;
          xE->u.keyButtonPointer.rootY = xeviehot.y;
          xE->u.keyButtonPointer.state = keyc->state;
          WriteToClient(clients[xevieClientIndex], sizeof(xEvent), (char *)xE);
#ifdef XKB
          if(noXkbExtension)
#endif
            return;
        } else {
	    xevieKBEventSent = 0;
        }
      }
    }
#endif

    if (!syncEvents.playingEvents)
    {
	NoticeTime(xE);
	if (DeviceEventCallback)
	{
	    DeviceEventInfoRec eventinfo;
	    eventinfo.events = xE;
	    eventinfo.count = count;
	    CallCallbacks(&DeviceEventCallback, (pointer)&eventinfo);
	}
    }
#ifdef XEVIE
    /* fix for bug5094030: don't change the state bit if the event is from XEvIE client */
    if(!(!xeviegrabState && xevieFlag && clients[xevieClientIndex] &&
	 (xevieMask & xevieFilters[xE->u.u.type]
#ifdef XKB
	  && !noXkbExtension
#endif
    )))
#endif
    XE_KBPTR.state = (keyc->state | inputInfo.pointer->button->state);
    XE_KBPTR.rootX = sprite.hot.x;
    XE_KBPTR.rootY = sprite.hot.y;
    key = xE->u.u.detail;
    kptr = &keyc->down[key >> 3];
    bit = 1 << (key & 7);
    modifiers = keyc->modifierMap[key];
#if defined(XKB) && defined(XEVIE)
    if(!noXkbExtension && !xeviegrabState &&
       xevieFlag && clients[xevieClientIndex] &&
       (xevieMask & xevieFilters[xE->u.u.type])) {
	switch(xE->u.u.type) {
	  case KeyPress: *kptr &= ~bit; break;
	  case KeyRelease: *kptr |= bit; break;
	}
    }
#endif

    switch (xE->u.u.type)
    {
	case KeyPress: 
	    if (*kptr & bit) /* allow ddx to generate multiple downs */
	    {   
		if (!modifiers)
		{
		    xE->u.u.type = KeyRelease;
		    (*keybd->public.processInputProc)(xE, keybd, count);
		    xE->u.u.type = KeyPress;
		    /* release can have side effects, don't fall through */
		    (*keybd->public.processInputProc)(xE, keybd, count);
		}
		return;
	    }
	    inputInfo.pointer->valuator->motionHintWindow = NullWindow;
	    *kptr |= bit;
	    keyc->prev_state = keyc->state;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
		if (mask & modifiers)
		{
		    /* This key affects modifier "i" */
		    keyc->modifierKeyCount[i]++;
		    keyc->state |= mask;
		    modifiers &= ~mask;
		}
	    }
	    if (!grab && CheckDeviceGrabs(keybd, xE, 0, count))
	    {
		keybd->activatingKey = key;
		return;
	    }
	    break;
	case KeyRelease: 
	    if (!(*kptr & bit)) /* guard against duplicates */
		return;
	    inputInfo.pointer->valuator->motionHintWindow = NullWindow;
	    *kptr &= ~bit;
	    keyc->prev_state = keyc->state;
	    for (i = 0, mask = 1; modifiers; i++, mask <<= 1)
	    {
		if (mask & modifiers) {
		    /* This key affects modifier "i" */
		    if (--keyc->modifierKeyCount[i] <= 0) {
			keyc->state &= ~mask;
			keyc->modifierKeyCount[i] = 0;
		    }
		    modifiers &= ~mask;
		}
	    }
	    if (keybd->fromPassiveGrab && (key == keybd->activatingKey))
		deactivateGrab = TRUE;
	    break;
	default: 
	    FatalError("Impossible keyboard event");
    }
    if (grab)
	DeliverGrabbedEvent(xE, keybd, deactivateGrab, count);
    else
	DeliverFocusedEvent(keybd, xE, sprite.win, count);
    if (deactivateGrab)
        (*keybd->DeactivateGrab)(keybd);

    XaceHook(XACE_KEY_AVAIL, xE, keybd, count);
}

#ifdef XKB
/* This function is used to set the key pressed or key released state -
   this is only used when the pressing of keys does not cause 
   the device's processInputProc to be called, as in for example Mouse Keys.
*/
void
FixKeyState (xEvent *xE, DeviceIntPtr keybd)
{
    int             key, bit;
    BYTE   *kptr;
    KeyClassPtr keyc = keybd->key;

    key = xE->u.u.detail;
    kptr = &keyc->down[key >> 3];
    bit = 1 << (key & 7);

    if (((xE->u.u.type==KeyPress)||(xE->u.u.type==KeyRelease)||
         (xE->u.u.type==DeviceKeyPress)||(xE->u.u.type==DeviceKeyRelease))
            ) {
	DebugF("FixKeyState: Key %d %s\n",key,
               (((xE->u.u.type==KeyPress)||(xE->u.u.type==DeviceKeyPress))?"down":"up"));
    }

    if (xE->u.u.type == KeyPress || xE->u.u.type == DeviceKeyPress)
	    *kptr |= bit;
    else if (xE->u.u.type == KeyRelease || xE->u.u.type == DeviceKeyRelease)
	    *kptr &= ~bit;
    else
        FatalError("Impossible keyboard event");
}
#endif

/** 
 * Main pointer event processing function for core pointer events. 
 * For motion events: update the sprite.
 * For all other events: Update the event fields based on the current sprite
 * state.
 *
 * For core pointer events, xE will always be a single event.
 *
 * @param xE Event list
 * @param mouse The device that caused an event.
 * @param count Number of elements in xE.
 */
void
#ifdef XKB
CoreProcessPointerEvent (xEvent *xE, DeviceIntPtr mouse, int count)
#else
ProcessPointerEvent (xEvent *xE, DeviceIntPtr mouse, int count)
#endif
{
    GrabPtr	grab = mouse->grab;
    Bool                deactivateGrab = FALSE;
    ButtonClassPtr butc = mouse->button;
#ifdef XKB
    XkbSrvInfoPtr xkbi= inputInfo.keyboard->key->xkbInfo;
#endif
#ifdef XEVIE
    if(xevieFlag && clients[xevieClientIndex] && !xeviegrabState &&
       (xevieMask & xevieFilters[xE->u.u.type])) {
      if(xevieEventSent)
        xevieEventSent = 0;
      else {
        xeviemouse = mouse;
        WriteToClient(clients[xevieClientIndex], sizeof(xEvent), (char *)xE);
        return;
      }
    }
#endif

    if (!syncEvents.playingEvents)
	NoticeTime(xE)
    XE_KBPTR.state = (butc->state | (
#ifdef XKB
			(noXkbExtension ?
				inputInfo.keyboard->key->state :
				xkbi->state.grab_mods)
#else
			inputInfo.keyboard->key->state
#endif
				    ));
    {
	NoticeTime(xE);
	if (DeviceEventCallback)
	{
	    DeviceEventInfoRec eventinfo;
	    /* see comment in EnqueueEvents regarding the next three lines */
	    if (xE->u.u.type == MotionNotify)
		XE_KBPTR.root =
		    WindowTable[sprite.hotPhys.pScreen->myNum]->drawable.id;
	    eventinfo.events = xE;
	    eventinfo.count = count;
	    CallCallbacks(&DeviceEventCallback, (pointer)&eventinfo);
	}
    }
    if (xE->u.u.type != MotionNotify)
    {
	int  key;
	BYTE *kptr;
	int           bit;

	XE_KBPTR.rootX = sprite.hot.x;
	XE_KBPTR.rootY = sprite.hot.y;

	key = xE->u.u.detail;
	kptr = &butc->down[key >> 3];
	bit = 1 << (key & 7);
	switch (xE->u.u.type)
	{
	case ButtonPress: 
	    mouse->valuator->motionHintWindow = NullWindow;
	    if (!(*kptr & bit))
		butc->buttonsDown++;
	    butc->motionMask = ButtonMotionMask;
	    *kptr |= bit;
	    if (xE->u.u.detail == 0)
		return;
	    if (xE->u.u.detail <= 5)
		butc->state |= (Button1Mask >> 1) << xE->u.u.detail;
	    filters[MotionNotify] = Motion_Filter(butc);
	    if (!grab)
		if (CheckDeviceGrabs(mouse, xE, 0, count))
		    return;
	    break;
	case ButtonRelease: 
	    mouse->valuator->motionHintWindow = NullWindow;
	    if (*kptr & bit)
		--butc->buttonsDown;
	    if (!butc->buttonsDown)
		butc->motionMask = 0;
	    *kptr &= ~bit;
	    if (xE->u.u.detail == 0)
		return;
	    if (xE->u.u.detail <= 5)
		butc->state &= ~((Button1Mask >> 1) << xE->u.u.detail);
	    filters[MotionNotify] = Motion_Filter(butc);
	    if (!butc->state && mouse->fromPassiveGrab)
		deactivateGrab = TRUE;
	    break;
	default: 
	    FatalError("bogus pointer event from ddx: %d", xE->u.u.type);
	}
    }
    else if (!CheckMotion(xE))
	return;
    if (grab)
	DeliverGrabbedEvent(xE, mouse, deactivateGrab, count);
    else
	DeliverDeviceEvents(sprite.win, xE, NullGrab, NullWindow,
			    mouse, count);
    if (deactivateGrab)
        (*mouse->DeactivateGrab)(mouse);
}

#define AtMostOneClient \
	(SubstructureRedirectMask | ResizeRedirectMask | ButtonPressMask)
#define ManagerMask \
	(SubstructureRedirectMask | ResizeRedirectMask)

/**
 * Recalculate which events may be deliverable for the given window.
 * Recalculated mask is used for quicker determination which events may be
 * delivered to a window.
 *
 * The otherEventMasks on a WindowOptional is the combination of all event
 * masks set by all clients on the window.
 * deliverableEventMask is the combination of the eventMask and the
 * otherEventMask.
 *
 * Traverses to siblings and parents of the window.
 */
void
RecalculateDeliverableEvents(pWin)
    WindowPtr pWin;
{
    OtherClients *others;
    WindowPtr pChild;

    pChild = pWin;
    while (1)
    {
	if (pChild->optional)
	{
	    pChild->optional->otherEventMasks = 0;
	    for (others = wOtherClients(pChild); others; others = others->next)
	    {
		pChild->optional->otherEventMasks |= others->mask;
	    }
	}
	pChild->deliverableEvents = pChild->eventMask|
				    wOtherEventMasks(pChild);
	if (pChild->parent)
	    pChild->deliverableEvents |=
		(pChild->parent->deliverableEvents &
		 ~wDontPropagateMask(pChild) & PropagateMask);
	if (pChild->firstChild)
	{
	    pChild = pChild->firstChild;
	    continue;
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    break;
	pChild = pChild->nextSib;
    }
}

/**
 *
 *  \param value must conform to DeleteType
 */
int
OtherClientGone(pointer value, XID id)
{
    OtherClientsPtr other, prev;
    WindowPtr pWin = (WindowPtr)value;

    prev = 0;
    for (other = wOtherClients(pWin); other; other = other->next)
    {
	if (other->resource == id)
	{
	    if (prev)
		prev->next = other->next;
	    else
	    {
		if (!(pWin->optional->otherClients = other->next))
		    CheckWindowOptionalNeed (pWin);
	    }
	    xfree(other);
	    RecalculateDeliverableEvents(pWin);
	    return(Success);
	}
	prev = other;
    }
    FatalError("client not on event list");
    /*NOTREACHED*/
    return -1; /* make compiler happy */
}

int
EventSelectForWindow(WindowPtr pWin, ClientPtr client, Mask mask)
{
    Mask check;
    OtherClients * others;
    int rc;

    if (mask & ~AllEventMasks)
    {
	client->errorValue = mask;
	return BadValue;
    }
    check = (mask & ManagerMask);
    if (check) {
	rc = XaceHook(XACE_RESOURCE_ACCESS, client, pWin->drawable.id,
		      RT_WINDOW, pWin, RT_NONE, NULL, DixManageAccess);
	if (rc != Success)
	    return rc;
    }
    check = (mask & AtMostOneClient);
    if (check & (pWin->eventMask|wOtherEventMasks(pWin)))
    {				       /* It is illegal for two different
				          clients to select on any of the
				          events for AtMostOneClient. However,
				          it is OK, for some client to
				          continue selecting on one of those
				          events.  */
	if ((wClient(pWin) != client) && (check & pWin->eventMask))
	    return BadAccess;
	for (others = wOtherClients (pWin); others; others = others->next)
	{
	    if (!SameClient(others, client) && (check & others->mask))
		return BadAccess;
	}
    }
    if (wClient (pWin) == client)
    {
	check = pWin->eventMask;
	pWin->eventMask = mask;
    }
    else
    {
	for (others = wOtherClients (pWin); others; others = others->next)
	{
	    if (SameClient(others, client))
	    {
		check = others->mask;
		if (mask == 0)
		{
		    FreeResource(others->resource, RT_NONE);
		    return Success;
		}
		else
		    others->mask = mask;
		goto maskSet;
	    }
	}
	check = 0;
	if (!pWin->optional && !MakeWindowOptional (pWin))
	    return BadAlloc;
	others = (OtherClients *) xalloc(sizeof(OtherClients));
	if (!others)
	    return BadAlloc;
	others->mask = mask;
	others->resource = FakeClientID(client->index);
	others->next = pWin->optional->otherClients;
	pWin->optional->otherClients = others;
	if (!AddResource(others->resource, RT_OTHERCLIENT, (pointer)pWin))
	    return BadAlloc;
    }
maskSet: 
    if ((inputInfo.pointer->valuator->motionHintWindow == pWin) &&
	(mask & PointerMotionHintMask) &&
	!(check & PointerMotionHintMask) &&
	!inputInfo.pointer->grab)
	inputInfo.pointer->valuator->motionHintWindow = NullWindow;
    RecalculateDeliverableEvents(pWin);
    return Success;
}

int
EventSuppressForWindow(WindowPtr pWin, ClientPtr client, 
                       Mask mask, Bool *checkOptional)
{
    int i, free;

    if (mask & ~PropagateMask)
    {
	client->errorValue = mask;
	return BadValue;
    }
    if (pWin->dontPropagate)
	DontPropagateRefCnts[pWin->dontPropagate]--;
    if (!mask)
	i = 0;
    else
    {
	for (i = DNPMCOUNT, free = 0; --i > 0; )
	{
	    if (!DontPropagateRefCnts[i])
		free = i;
	    else if (mask == DontPropagateMasks[i])
		break;
	}
	if (!i && free)
	{
	    i = free;
	    DontPropagateMasks[i] = mask;
	}
    }
    if (i || !mask)
    {
	pWin->dontPropagate = i;
	if (i)
	    DontPropagateRefCnts[i]++;
	if (pWin->optional)
	{
	    pWin->optional->dontPropagateMask = mask;
	    *checkOptional = TRUE;
	}
    }
    else
    {
	if (!pWin->optional && !MakeWindowOptional (pWin))
	{
	    if (pWin->dontPropagate)
		DontPropagateRefCnts[pWin->dontPropagate]++;
	    return BadAlloc;
	}
	pWin->dontPropagate = 0;
        pWin->optional->dontPropagateMask = mask;
    }
    RecalculateDeliverableEvents(pWin);
    return Success;
}

/**
 * @return The window that is the first ancestor of both a and b.
 */
static WindowPtr 
CommonAncestor(
    WindowPtr a,
    WindowPtr b)
{
    for (b = b->parent; b; b = b->parent)
	if (IsParent(b, a)) return b;
    return NullWindow;
}

/**
 * Assembles an EnterNotify or LeaveNotify and sends it event to the client. 
 * The core devices are used to fill in the event fields.
 */
static void
EnterLeaveEvent(
    int type,
    int mode,
    int detail,
    WindowPtr pWin,
    Window child)
{
    xEvent		event;
    DeviceIntPtr keybd = inputInfo.keyboard;
    WindowPtr		focus;
    DeviceIntPtr mouse = inputInfo.pointer;
    GrabPtr	grab = mouse->grab;
    Mask		mask;

    if ((pWin == mouse->valuator->motionHintWindow) &&
	(detail != NotifyInferior))
	mouse->valuator->motionHintWindow = NullWindow;
    if (grab)
    {
	mask = (pWin == grab->window) ? grab->eventMask : 0;
	if (grab->ownerEvents)
	    mask |= EventMaskForClient(pWin, rClient(grab));
    }
    else
    {
	mask = pWin->eventMask | wOtherEventMasks(pWin);
    }
    if (mask & filters[type])
    {
	event.u.u.type = type;
	event.u.u.detail = detail;
	event.u.enterLeave.time = currentTime.milliseconds;
	event.u.enterLeave.rootX = sprite.hot.x;
	event.u.enterLeave.rootY = sprite.hot.y;
	/* Counts on the same initial structure of crossing & button events! */
	FixUpEventFromWindow(&event, pWin, None, FALSE);
	/* Enter/Leave events always set child */
	event.u.enterLeave.child = child;
	event.u.enterLeave.flags = event.u.keyButtonPointer.sameScreen ?
					    ELFlagSameScreen : 0;
#ifdef XKB
	if (!noXkbExtension) {
	    event.u.enterLeave.state = mouse->button->state & 0x1f00;
	    event.u.enterLeave.state |= 
			XkbGrabStateFromRec(&keybd->key->xkbInfo->state);
	} else
#endif
	event.u.enterLeave.state = keybd->key->state | mouse->button->state;
	event.u.enterLeave.mode = mode;
	focus = keybd->focus->win;
	if ((focus != NoneWin) &&
	    ((pWin == focus) || (focus == PointerRootWin) ||
	     IsParent(focus, pWin)))
	    event.u.enterLeave.flags |= ELFlagFocus;
	if (grab)
	    (void)TryClientEvents(rClient(grab), &event, 1, mask,
				  filters[type], grab);
	else
	    (void)DeliverEventsToWindow(pWin, &event, 1, filters[type],
					NullGrab, 0);
    }
    if ((type == EnterNotify) && (mask & KeymapStateMask))
    {
	xKeymapEvent ke;
	ClientPtr client = grab ? rClient(grab)
				: clients[CLIENT_ID(pWin->drawable.id)];
	if (XaceHook(XACE_DEVICE_ACCESS, client, keybd, DixReadAccess))
	    bzero((char *)&ke.map[0], 31);
	else
	    memmove((char *)&ke.map[0], (char *)&keybd->key->down[1], 31);

	ke.type = KeymapNotify;
	if (grab)
	    (void)TryClientEvents(rClient(grab), (xEvent *)&ke, 1, mask,
				  KeymapStateMask, grab);
	else
	    (void)DeliverEventsToWindow(pWin, (xEvent *)&ke, 1,
					KeymapStateMask, NullGrab, 0);
    }
}

/**
 * Send enter notifies to all parent windows up to ancestor.
 * This function recurses.
 */
static void
EnterNotifies(WindowPtr ancestor, WindowPtr child, int mode, int detail)
{
    WindowPtr	parent = child->parent;

    if (ancestor == parent)
	return;
    EnterNotifies(ancestor, parent, mode, detail);
    EnterLeaveEvent(EnterNotify, mode, detail, parent, child->drawable.id);
}


/**
 * Send leave notifies to all parent windows up to ancestor.
 * This function recurses.
 */
static void
LeaveNotifies(WindowPtr child, WindowPtr ancestor, int mode, int detail)
{
    WindowPtr  pWin;

    if (ancestor == child)
	return;
    for (pWin = child->parent; pWin != ancestor; pWin = pWin->parent)
    {
	EnterLeaveEvent(LeaveNotify, mode, detail, pWin, child->drawable.id);
	child = pWin;
    }
}

/**
 * Figure out if enter/leave events are necessary and send them to the
 * appropriate windows.
 * 
 * @param fromWin Window the sprite moved out of.
 * @param toWin Window the sprite moved into.
 */
static void
DoEnterLeaveEvents(WindowPtr fromWin, WindowPtr toWin, int mode)
{
    if (fromWin == toWin)
	return;
    if (IsParent(fromWin, toWin))
    {
	EnterLeaveEvent(LeaveNotify, mode, NotifyInferior, fromWin, None);
	EnterNotifies(fromWin, toWin, mode, NotifyVirtual);
	EnterLeaveEvent(EnterNotify, mode, NotifyAncestor, toWin, None);
    }
    else if (IsParent(toWin, fromWin))
    {
	EnterLeaveEvent(LeaveNotify, mode, NotifyAncestor, fromWin, None);
	LeaveNotifies(fromWin, toWin, mode, NotifyVirtual);
	EnterLeaveEvent(EnterNotify, mode, NotifyInferior, toWin, None);
    }
    else
    { /* neither fromWin nor toWin is descendent of the other */
	WindowPtr common = CommonAncestor(toWin, fromWin);
	/* common == NullWindow ==> different screens */
	EnterLeaveEvent(LeaveNotify, mode, NotifyNonlinear, fromWin, None);
	LeaveNotifies(fromWin, common, mode, NotifyNonlinearVirtual);
	EnterNotifies(common, toWin, mode, NotifyNonlinearVirtual);
	EnterLeaveEvent(EnterNotify, mode, NotifyNonlinear, toWin, None);
    }
}

static void
FocusEvent(DeviceIntPtr dev, int type, int mode, int detail, WindowPtr pWin)
{
    xEvent event;

#ifdef XINPUT
    if (dev != inputInfo.keyboard)
    {
	DeviceFocusEvent(dev, type, mode, detail, pWin);
	return;
    }
#endif
    event.u.focus.mode = mode;
    event.u.u.type = type;
    event.u.u.detail = detail;
    event.u.focus.window = pWin->drawable.id;
    (void)DeliverEventsToWindow(pWin, &event, 1, filters[type], NullGrab,
				0);
    if ((type == FocusIn) &&
	((pWin->eventMask | wOtherEventMasks(pWin)) & KeymapStateMask))
    {
	xKeymapEvent ke;
	ClientPtr client = clients[CLIENT_ID(pWin->drawable.id)];
	if (XaceHook(XACE_DEVICE_ACCESS, client, dev, DixReadAccess))
	    bzero((char *)&ke.map[0], 31);
	else
	    memmove((char *)&ke.map[0], (char *)&dev->key->down[1], 31);

	ke.type = KeymapNotify;
	(void)DeliverEventsToWindow(pWin, (xEvent *)&ke, 1,
				    KeymapStateMask, NullGrab, 0);
    }
}

 /*
  * recursive because it is easier
  * no-op if child not descended from ancestor
  */
static Bool
FocusInEvents(
    DeviceIntPtr dev,
    WindowPtr ancestor, WindowPtr child, WindowPtr skipChild,
    int mode, int detail,
    Bool doAncestor)
{
    if (child == NullWindow)
	return ancestor == NullWindow;
    if (ancestor == child)
    {
	if (doAncestor)
	    FocusEvent(dev, FocusIn, mode, detail, child);
	return TRUE;
    }
    if (FocusInEvents(dev, ancestor, child->parent, skipChild, mode, detail,
		      doAncestor))
    {
	if (child != skipChild)
	    FocusEvent(dev, FocusIn, mode, detail, child);
	return TRUE;
    }
    return FALSE;
}

/* dies horribly if ancestor is not an ancestor of child */
static void
FocusOutEvents(
    DeviceIntPtr dev,
    WindowPtr child, WindowPtr ancestor,
    int mode, int detail,
    Bool doAncestor)
{
    WindowPtr  pWin;

    for (pWin = child; pWin != ancestor; pWin = pWin->parent)
	FocusEvent(dev, FocusOut, mode, detail, pWin);
    if (doAncestor)
	FocusEvent(dev, FocusOut, mode, detail, ancestor);
}

void
DoFocusEvents(DeviceIntPtr dev, WindowPtr fromWin, WindowPtr toWin, int mode)
{
    int     out, in;		       /* for holding details for to/from
				          PointerRoot/None */
    int     i;

    if (fromWin == toWin)
	return;
    out = (fromWin == NoneWin) ? NotifyDetailNone : NotifyPointerRoot;
    in = (toWin == NoneWin) ? NotifyDetailNone : NotifyPointerRoot;
 /* wrong values if neither, but then not referenced */

    if ((toWin == NullWindow) || (toWin == PointerRootWin))
    {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
   	{
	    if (fromWin == PointerRootWin)
		FocusOutEvents(dev, sprite.win, ROOT, mode, NotifyPointer,
			       TRUE);
	    /* Notify all the roots */
#ifdef PANORAMIX
 	    if ( !noPanoramiXExtension )
	        FocusEvent(dev, FocusOut, mode, out, WindowTable[0]);
	    else 
#endif
	        for (i=0; i<screenInfo.numScreens; i++)
	            FocusEvent(dev, FocusOut, mode, out, WindowTable[i]);
	}
	else
	{
	    if (IsParent(fromWin, sprite.win))
	      FocusOutEvents(dev, sprite.win, fromWin, mode, NotifyPointer,
			     FALSE);
	    FocusEvent(dev, FocusOut, mode, NotifyNonlinear, fromWin);
	    /* next call catches the root too, if the screen changed */
	    FocusOutEvents(dev, fromWin->parent, NullWindow, mode,
			   NotifyNonlinearVirtual, FALSE);
	}
	/* Notify all the roots */
#ifdef PANORAMIX
	if ( !noPanoramiXExtension )
	    FocusEvent(dev, FocusIn, mode, in, WindowTable[0]);
	else 
#endif
	    for (i=0; i<screenInfo.numScreens; i++)
	        FocusEvent(dev, FocusIn, mode, in, WindowTable[i]);
	if (toWin == PointerRootWin)
	    (void)FocusInEvents(dev, ROOT, sprite.win, NullWindow, mode,
				NotifyPointer, TRUE);
    }
    else
    {
	if ((fromWin == NullWindow) || (fromWin == PointerRootWin))
	{
	    if (fromWin == PointerRootWin)
		FocusOutEvents(dev, sprite.win, ROOT, mode, NotifyPointer,
			       TRUE);
#ifdef PANORAMIX
 	    if ( !noPanoramiXExtension )
	        FocusEvent(dev, FocusOut, mode, out, WindowTable[0]);
	    else 
#endif
	        for (i=0; i<screenInfo.numScreens; i++)
	            FocusEvent(dev, FocusOut, mode, out, WindowTable[i]);
	    if (toWin->parent != NullWindow)
	      (void)FocusInEvents(dev, ROOT, toWin, toWin, mode,
				  NotifyNonlinearVirtual, TRUE);
	    FocusEvent(dev, FocusIn, mode, NotifyNonlinear, toWin);
	    if (IsParent(toWin, sprite.win))
    	       (void)FocusInEvents(dev, toWin, sprite.win, NullWindow, mode,
				   NotifyPointer, FALSE);
	}
	else
	{
	    if (IsParent(toWin, fromWin))
	    {
		FocusEvent(dev, FocusOut, mode, NotifyAncestor, fromWin);
		FocusOutEvents(dev, fromWin->parent, toWin, mode,
			       NotifyVirtual, FALSE);
		FocusEvent(dev, FocusIn, mode, NotifyInferior, toWin);
		if ((IsParent(toWin, sprite.win)) &&
			(sprite.win != fromWin) &&
			(!IsParent(fromWin, sprite.win)) &&
			(!IsParent(sprite.win, fromWin)))
		    (void)FocusInEvents(dev, toWin, sprite.win, NullWindow,
					mode, NotifyPointer, FALSE);
	    }
	    else
		if (IsParent(fromWin, toWin))
		{
		    if ((IsParent(fromWin, sprite.win)) &&
			    (sprite.win != fromWin) &&
			    (!IsParent(toWin, sprite.win)) &&
			    (!IsParent(sprite.win, toWin)))
			FocusOutEvents(dev, sprite.win, fromWin, mode,
				       NotifyPointer, FALSE);
		    FocusEvent(dev, FocusOut, mode, NotifyInferior, fromWin);
		    (void)FocusInEvents(dev, fromWin, toWin, toWin, mode,
					NotifyVirtual, FALSE);
		    FocusEvent(dev, FocusIn, mode, NotifyAncestor, toWin);
		}
		else
		{
		/* neither fromWin or toWin is child of other */
		    WindowPtr common = CommonAncestor(toWin, fromWin);
		/* common == NullWindow ==> different screens */
		    if (IsParent(fromWin, sprite.win))
			FocusOutEvents(dev, sprite.win, fromWin, mode,
				       NotifyPointer, FALSE);
		    FocusEvent(dev, FocusOut, mode, NotifyNonlinear, fromWin);
		    if (fromWin->parent != NullWindow)
		      FocusOutEvents(dev, fromWin->parent, common, mode,
				     NotifyNonlinearVirtual, FALSE);
		    if (toWin->parent != NullWindow)
		      (void)FocusInEvents(dev, common, toWin, toWin, mode,
					  NotifyNonlinearVirtual, FALSE);
		    FocusEvent(dev, FocusIn, mode, NotifyNonlinear, toWin);
		    if (IsParent(toWin, sprite.win))
			(void)FocusInEvents(dev, toWin, sprite.win, NullWindow,
					    mode, NotifyPointer, FALSE);
		}
	}
    }
}

/**
 * Set the input focus to the given window. Subsequent keyboard events will be
 * delivered to the given window.
 * 
 * Usually called from ProcSetInputFocus as result of a client request. If so,
 * the device is the inputInfo.keyboard.
 * If called from ProcXSetInputFocus as result of a client xinput request, the
 * device is set to the device specified by the client.
 *
 * @param client Client that requested input focus change.
 * @param dev Focus device. 
 * @param focusID The window to obtain the focus. Can be PointerRoot or None.
 * @param revertTo Specifies where the focus reverts to when window becomes
 * unviewable.
 * @param ctime Specifies the time.
 * @param followOK True if pointer is allowed to follow the keyboard.
 */
int
SetInputFocus(
    ClientPtr client,
    DeviceIntPtr dev,
    Window focusID,
    CARD8 revertTo,
    Time ctime,
    Bool followOK)
{
    FocusClassPtr focus;
    WindowPtr focusWin;
    int mode, rc;
    TimeStamp time;

    UpdateCurrentTime();
    if ((revertTo != RevertToParent) &&
	(revertTo != RevertToPointerRoot) &&
	(revertTo != RevertToNone) &&
	((revertTo != RevertToFollowKeyboard) || !followOK))
    {
	client->errorValue = revertTo;
	return BadValue;
    }
    time = ClientTimeToServerTime(ctime);
    if ((focusID == None) || (focusID == PointerRoot))
	focusWin = (WindowPtr)(long)focusID;
    else if ((focusID == FollowKeyboard) && followOK)
	focusWin = inputInfo.keyboard->focus->win;
    else {
	rc = dixLookupWindow(&focusWin, focusID, client, DixSetAttrAccess);
	if (rc != Success)
	    return rc;
 	/* It is a match error to try to set the input focus to an 
	unviewable window. */
	if(!focusWin->realized)
	    return(BadMatch);
    }
    rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, DixSetFocusAccess);
    if (rc != Success)
	return Success;

    focus = dev->focus;
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	(CompareTimeStamps(time, focus->time) == EARLIER))
	return Success;
    mode = (dev->grab) ? NotifyWhileGrabbed : NotifyNormal;
    if (focus->win == FollowKeyboardWin)
	DoFocusEvents(dev, inputInfo.keyboard->focus->win, focusWin, mode);
    else
	DoFocusEvents(dev, focus->win, focusWin, mode);
    focus->time = time;
    focus->revert = revertTo;
    if (focusID == FollowKeyboard)
	focus->win = FollowKeyboardWin;
    else
	focus->win = focusWin;
    if ((focusWin == NoneWin) || (focusWin == PointerRootWin))
	focus->traceGood = 0;
    else
    {
        int depth = 0;
	WindowPtr pWin;

        for (pWin = focusWin; pWin; pWin = pWin->parent) depth++;
        if (depth > focus->traceSize)
        {
	    focus->traceSize = depth+1;
	    Must_have_memory = TRUE; /* XXX */
	    focus->trace = (WindowPtr *)xrealloc(focus->trace,
						 focus->traceSize *
						 sizeof(WindowPtr));
	    Must_have_memory = FALSE; /* XXX */
	}
	focus->traceGood = depth;
        for (pWin = focusWin, depth--; pWin; pWin = pWin->parent, depth--) 
	    focus->trace[depth] = pWin;
    }
    return Success;
}

/**
 * Server-side protocol handling for SetInputFocus request.
 *
 * Sets the input focus for the virtual core keyboard.
 */
int
ProcSetInputFocus(client)
    ClientPtr client;
{
    REQUEST(xSetInputFocusReq);

    REQUEST_SIZE_MATCH(xSetInputFocusReq);

    return SetInputFocus(client, inputInfo.keyboard, stuff->focus,
			 stuff->revertTo, stuff->time, FALSE);
}

/**
 * Server-side protocol handling for GetInputFocus request.
 * 
 * Sends the current input focus for the virtual core keyboard back to the
 * client.
 */
int
ProcGetInputFocus(ClientPtr client)
{
    xGetInputFocusReply rep;
    FocusClassPtr focus = inputInfo.keyboard->focus;
    int rc;
    /* REQUEST(xReq); */
    REQUEST_SIZE_MATCH(xReq);

    rc = XaceHook(XACE_DEVICE_ACCESS, client, inputInfo.keyboard,
		  DixGetFocusAccess);
    if (rc != Success)
	return rc;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (focus->win == NoneWin)
	rep.focus = None;
    else if (focus->win == PointerRootWin)
	rep.focus = PointerRoot;
    else rep.focus = focus->win->drawable.id;
    rep.revertTo = focus->revert;
    WriteReplyToClient(client, sizeof(xGetInputFocusReply), &rep);
    return Success;
}

/**
 * Server-side protocol handling for Grabpointer request.
 *
 * Sets an active grab on the inputInfo.pointer and returns success status to
 * client.
 */
int
ProcGrabPointer(ClientPtr client)
{
    xGrabPointerReply rep;
    DeviceIntPtr device = inputInfo.pointer;
    GrabPtr grab;
    WindowPtr pWin, confineTo;
    CursorPtr cursor, oldCursor;
    REQUEST(xGrabPointerReq);
    TimeStamp time;
    Mask access_mode = DixGrabAccess;
    int rc;

    REQUEST_SIZE_MATCH(xGrabPointerReq);
    UpdateCurrentTime();
    if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
        return BadValue;
    }
    if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
        return BadValue;
    }
    if ((stuff->ownerEvents != xFalse) && (stuff->ownerEvents != xTrue))
    {
	client->errorValue = stuff->ownerEvents;
        return BadValue;
    }
    if (stuff->eventMask & ~PointerGrabMask)
    {
	client->errorValue = stuff->eventMask;
        return BadValue;
    }
    rc = dixLookupWindow(&pWin, stuff->grabWindow, client, DixSetAttrAccess);
    if (rc != Success)
	return rc;
    if (stuff->confineTo == None)
	confineTo = NullWindow;
    else 
    {
	rc = dixLookupWindow(&confineTo, stuff->confineTo, client,
			     DixSetAttrAccess);
	if (rc != Success)
	    return rc;
    }
    if (stuff->cursor == None)
	cursor = NullCursor;
    else
    {
	rc = dixLookupResource((pointer *)&cursor, stuff->cursor, RT_CURSOR,
			       client, DixUseAccess);
	if (rc != Success)
	{
	    client->errorValue = stuff->cursor;
	    return (rc == BadValue) ? BadCursor : rc;
	}
	access_mode |= DixForceAccess;
    }
    if (stuff->pointerMode == GrabModeSync ||
	stuff->keyboardMode == GrabModeSync)
	access_mode |= DixFreezeAccess;
    rc = XaceHook(XACE_DEVICE_ACCESS, client, device, access_mode);
    if (rc != Success)
	return rc;

	/* at this point, some sort of reply is guaranteed. */
    time = ClientTimeToServerTime(stuff->time);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    grab = device->grab;
    if ((grab) && !SameClient(grab, client))
	rep.status = AlreadyGrabbed;
    else if ((!pWin->realized) ||
             (confineTo &&
                !(confineTo->realized && BorderSizeNotEmpty(confineTo))))
	rep.status = GrabNotViewable;
    else if (device->sync.frozen &&
	     device->sync.other && !SameClient(device->sync.other, client))
	rep.status = GrabFrozen;
    else if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (CompareTimeStamps(time, device->grabTime) == EARLIER))
	rep.status = GrabInvalidTime;
    else
    {
	GrabRec tempGrab;

	oldCursor = NullCursor;
	if (grab)
 	{
	    if (grab->confineTo && !confineTo)
		ConfineCursorToWindow(ROOT, FALSE, FALSE);
	    oldCursor = grab->cursor;
	}
	tempGrab.cursor = cursor;
	tempGrab.resource = client->clientAsMask;
	tempGrab.ownerEvents = stuff->ownerEvents;
	tempGrab.eventMask = stuff->eventMask;
	tempGrab.confineTo = confineTo;
	tempGrab.window = pWin;
	tempGrab.keyboardMode = stuff->keyboardMode;
	tempGrab.pointerMode = stuff->pointerMode;
	tempGrab.device = device;
	(*device->ActivateGrab)(device, &tempGrab, time, FALSE);
	if (oldCursor)
	    FreeCursor (oldCursor, (Cursor)0);
	rep.status = GrabSuccess;
    }
    WriteReplyToClient(client, sizeof(xGrabPointerReply), &rep);
    return Success;
}

/**
 * Server-side protocol handling for ChangeActivePointerGrab request.
 *
 * Changes properties of the grab hold by the client. If the client does not
 * hold an active grab on the device, nothing happens. 
 *
 * Works on the core pointer only.
 */
int
ProcChangeActivePointerGrab(ClientPtr client)
{
    DeviceIntPtr device = inputInfo.pointer;
    GrabPtr grab = device->grab;
    CursorPtr newCursor, oldCursor;
    REQUEST(xChangeActivePointerGrabReq);
    TimeStamp time;

    REQUEST_SIZE_MATCH(xChangeActivePointerGrabReq);
    if (stuff->eventMask & ~PointerGrabMask)
    {
	client->errorValue = stuff->eventMask;
        return BadValue;
    }
    if (stuff->cursor == None)
	newCursor = NullCursor;
    else
    {
	int rc = dixLookupResource((pointer *)&newCursor, stuff->cursor,
				   RT_CURSOR, client, DixUseAccess);
	if (rc != Success)
	{
	    client->errorValue = stuff->cursor;
	    return (rc == BadValue) ? BadCursor : rc;
	}
    }
    if (!grab)
	return Success;
    if (!SameClient(grab, client))
	return Success;
    time = ClientTimeToServerTime(stuff->time);
    if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (CompareTimeStamps(time, device->grabTime) == EARLIER))
	return Success;
    oldCursor = grab->cursor;
    grab->cursor = newCursor;
    if (newCursor)
	newCursor->refcnt++;
    PostNewCursor();
    if (oldCursor)
	FreeCursor(oldCursor, (Cursor)0);
    grab->eventMask = stuff->eventMask;
    return Success;
}

/**
 * Server-side protocol handling for UngrabPointer request.
 *
 * Deletes the pointer grab on the core pointer device.
 */
int
ProcUngrabPointer(ClientPtr client)
{
    DeviceIntPtr device = inputInfo.pointer;
    GrabPtr grab;
    TimeStamp time;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    UpdateCurrentTime();
    grab = device->grab;
    time = ClientTimeToServerTime(stuff->id);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	    (CompareTimeStamps(time, device->grabTime) != EARLIER) &&
	    (grab) && SameClient(grab, client))
	(*device->DeactivateGrab)(device);
    return Success;
}

/**
 * Sets a grab on the given device.
 * 
 * Called from ProcGrabKeyboard to work on the inputInfo.keyboard.
 * Called from ProcXGrabDevice to work on the device specified by the client.
 * 
 * The parameters this_mode and other_mode represent the keyboard_mode and
 * pointer_mode parameters of XGrabKeyboard(). 
 * See man page for details on all the parameters
 * 
 * @param client Client that owns the grab.
 * @param dev The device to grab. 
 * @param this_mode GrabModeSync or GrabModeAsync
 * @param other_mode GrabModeSync or GrabModeAsync
 * @param status Return code to be returned to the caller.
 * 
 * @returns Success or BadValue.
 */
int
GrabDevice(ClientPtr client, DeviceIntPtr dev, 
           unsigned this_mode, unsigned other_mode, Window grabWindow, 
           unsigned ownerEvents, Time ctime, Mask mask, CARD8 *status)
{
    WindowPtr pWin;
    GrabPtr grab;
    TimeStamp time;
    Mask access_mode = DixGrabAccess;
    int rc;

    UpdateCurrentTime();
    if ((this_mode != GrabModeSync) && (this_mode != GrabModeAsync))
    {
	client->errorValue = this_mode;
        return BadValue;
    }
    if ((other_mode != GrabModeSync) && (other_mode != GrabModeAsync))
    {
	client->errorValue = other_mode;
        return BadValue;
    }
    if ((ownerEvents != xFalse) && (ownerEvents != xTrue))
    {
	client->errorValue = ownerEvents;
        return BadValue;
    }

    rc = dixLookupWindow(&pWin, grabWindow, client, DixSetAttrAccess);
    if (rc != Success)
	return rc;
    if (this_mode == GrabModeSync || other_mode == GrabModeSync)
	access_mode |= DixFreezeAccess;
    rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, access_mode);
    if (rc != Success)
	return rc;

    time = ClientTimeToServerTime(ctime);
    grab = dev->grab;
    if (grab && !SameClient(grab, client))
	*status = AlreadyGrabbed;
    else if (!pWin->realized)
	*status = GrabNotViewable;
    else if ((CompareTimeStamps(time, currentTime) == LATER) ||
	     (CompareTimeStamps(time, dev->grabTime) == EARLIER))
	*status = GrabInvalidTime;
    else if (dev->sync.frozen &&
	     dev->sync.other && !SameClient(dev->sync.other, client))
	*status = GrabFrozen;
    else
    {
	GrabRec tempGrab;

	tempGrab.window = pWin;
	tempGrab.resource = client->clientAsMask;
	tempGrab.ownerEvents = ownerEvents;
	tempGrab.keyboardMode = this_mode;
	tempGrab.pointerMode = other_mode;
	tempGrab.eventMask = mask;
	tempGrab.device = dev;
	(*dev->ActivateGrab)(dev, &tempGrab, time, FALSE);
	*status = GrabSuccess;
    }
    return Success;
}

/**
 * Server-side protocol handling for GrabKeyboard request.
 *
 * Grabs the inputInfo.keyboad and returns success status to client.
 */
int
ProcGrabKeyboard(ClientPtr client)
{
    xGrabKeyboardReply rep;
    REQUEST(xGrabKeyboardReq);
    int result;

    REQUEST_SIZE_MATCH(xGrabKeyboardReq);

    result = GrabDevice(client, inputInfo.keyboard, stuff->keyboardMode,
			stuff->pointerMode, stuff->grabWindow,
			stuff->ownerEvents, stuff->time,
			KeyPressMask | KeyReleaseMask, &rep.status);

    if (result != Success)
	return result;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    WriteReplyToClient(client, sizeof(xGrabKeyboardReply), &rep);
    return Success;
}

/**
 * Server-side protocol handling for UngrabKeyboard request.
 *
 * Deletes a possible grab on the inputInfo.keyboard.
 */
int
ProcUngrabKeyboard(ClientPtr client)
{
    DeviceIntPtr device = inputInfo.keyboard;
    GrabPtr grab;
    TimeStamp time;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    UpdateCurrentTime();
    grab = device->grab;
    time = ClientTimeToServerTime(stuff->id);
    if ((CompareTimeStamps(time, currentTime) != LATER) &&
	(CompareTimeStamps(time, device->grabTime) != EARLIER) &&
	(grab) && SameClient(grab, client))
	(*device->DeactivateGrab)(device);
    return Success;
}

/**
 * Server-side protocol handling for QueryPointer request.
 *
 * Returns the current state and position of the core pointer to the client. 
 */
int
ProcQueryPointer(ClientPtr client)
{
    xQueryPointerReply rep;
    WindowPtr pWin, t;
    DeviceIntPtr mouse = inputInfo.pointer;
    int rc;
    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    rc = dixLookupWindow(&pWin, stuff->id, client, DixGetAttrAccess);
    if (rc != Success)
	return rc;
    rc = XaceHook(XACE_DEVICE_ACCESS, client, mouse, DixReadAccess);
    if (rc != Success)
	return rc;

    if (mouse->valuator->motionHintWindow)
	MaybeStopHint(mouse, client);
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.mask = mouse->button->state | inputInfo.keyboard->key->state;
    rep.length = 0;
    rep.root = (ROOT)->drawable.id;
    rep.rootX = sprite.hot.x;
    rep.rootY = sprite.hot.y;
    rep.child = None;
    if (sprite.hot.pScreen == pWin->drawable.pScreen)
    {
	rep.sameScreen = xTrue;
	rep.winX = sprite.hot.x - pWin->drawable.x;
	rep.winY = sprite.hot.y - pWin->drawable.y;
	for (t = sprite.win; t; t = t->parent)
	    if (t->parent == pWin)
	    {
		rep.child = t->drawable.id;
		break;
	    }
    }
    else
    {
	rep.sameScreen = xFalse;
	rep.winX = 0;
	rep.winY = 0;
    }

#ifdef PANORAMIX
    if(!noPanoramiXExtension) {
	rep.rootX += panoramiXdataPtr[0].x;
	rep.rootY += panoramiXdataPtr[0].y;
	if(stuff->id == rep.root) {
	    rep.winX += panoramiXdataPtr[0].x;
	    rep.winY += panoramiXdataPtr[0].y;
	}
    }
#endif

    WriteReplyToClient(client, sizeof(xQueryPointerReply), &rep);

    return(Success);    
}

/**
 * Initializes the device list and the DIX sprite to sane values. Allocates
 * trace memory used for quick window traversal.
 */
void
InitEvents(void)
{
    int i;

    sprite.hot.pScreen = sprite.hotPhys.pScreen = (ScreenPtr)NULL;
    inputInfo.numDevices = 0;
    inputInfo.devices = (DeviceIntPtr)NULL;
    inputInfo.off_devices = (DeviceIntPtr)NULL;
    inputInfo.keyboard = (DeviceIntPtr)NULL;
    inputInfo.pointer = (DeviceIntPtr)NULL;
    if (spriteTraceSize == 0)
    {
	spriteTraceSize = 32;
	spriteTrace = (WindowPtr *)xalloc(32*sizeof(WindowPtr));
	if (!spriteTrace)
	    FatalError("failed to allocate spriteTrace");
    }
    spriteTraceGood = 0;
    lastEventMask = OwnerGrabButtonMask;
    filters[MotionNotify] = PointerMotionMask;
#ifdef XEVIE
    xeviewin =
#endif
    sprite.win = NullWindow;
    sprite.current = NullCursor;
    sprite.hotLimits.x1 = 0;
    sprite.hotLimits.y1 = 0;
    sprite.hotLimits.x2 = 0;
    sprite.hotLimits.y2 = 0;
    sprite.confined = FALSE;
    syncEvents.replayDev = (DeviceIntPtr)NULL;
    syncEvents.replayWin = NullWindow;
    while (syncEvents.pending)
    {
	QdEventPtr next = syncEvents.pending->next;
	xfree(syncEvents.pending);
	syncEvents.pending = next;
    }
    syncEvents.pendtail = &syncEvents.pending;
    syncEvents.playingEvents = FALSE;
    syncEvents.time.months = 0;
    syncEvents.time.milliseconds = 0;	/* hardly matters */
    currentTime.months = 0;
    currentTime.milliseconds = GetTimeInMillis();
    lastDeviceEventTime = currentTime;
    for (i = 0; i < DNPMCOUNT; i++)
    {
	DontPropagateMasks[i] = 0;
	DontPropagateRefCnts[i] = 0;
    }
}

void
CloseDownEvents(void)
{
  xfree(spriteTrace);
  spriteTrace = NULL;
  spriteTraceSize = 0;
}

/**
 * Server-side protocol handling for SendEvent request.
 *
 * Locates the window to send the event to and forwards the event. 
 */
int
ProcSendEvent(ClientPtr client)
{
    WindowPtr pWin;
    WindowPtr effectiveFocus = NullWindow; /* only set if dest==InputFocus */
    REQUEST(xSendEventReq);

    REQUEST_SIZE_MATCH(xSendEventReq);

    /* The client's event type must be a core event type or one defined by an
	extension. */

    if ( ! ((stuff->event.u.u.type > X_Reply &&
	     stuff->event.u.u.type < LASTEvent) || 
	    (stuff->event.u.u.type >= EXTENSION_EVENT_BASE &&
	     stuff->event.u.u.type < (unsigned)lastEvent)))
    {
	client->errorValue = stuff->event.u.u.type;
	return BadValue;
    }
    if (stuff->event.u.u.type == ClientMessage &&
	stuff->event.u.u.detail != 8 &&
	stuff->event.u.u.detail != 16 &&
	stuff->event.u.u.detail != 32)
    {
	client->errorValue = stuff->event.u.u.detail;
	return BadValue;
    }
    if (stuff->eventMask & ~AllEventMasks)
    {
	client->errorValue = stuff->eventMask;
	return BadValue;
    }

    if (stuff->destination == PointerWindow)
	pWin = sprite.win;
    else if (stuff->destination == InputFocus)
    {
	WindowPtr inputFocus = inputInfo.keyboard->focus->win;

	if (inputFocus == NoneWin)
	    return Success;

	/* If the input focus is PointerRootWin, send the event to where
	the pointer is if possible, then perhaps propogate up to root. */
   	if (inputFocus == PointerRootWin)
	    inputFocus = ROOT;

	if (IsParent(inputFocus, sprite.win))
	{
	    effectiveFocus = inputFocus;
	    pWin = sprite.win;
	}
	else
	    effectiveFocus = pWin = inputFocus;
    }
    else
	dixLookupWindow(&pWin, stuff->destination, client, DixSendAccess);

    if (!pWin)
	return BadWindow;
    if ((stuff->propagate != xFalse) && (stuff->propagate != xTrue))
    {
	client->errorValue = stuff->propagate;
	return BadValue;
    }
    stuff->event.u.u.type |= 0x80;
    if (stuff->propagate)
    {
	for (;pWin; pWin = pWin->parent)
	{
	    if (XaceHook(XACE_SEND_ACCESS, client, NULL, pWin,
			 &stuff->event, 1))
		return Success;
	    if (DeliverEventsToWindow(pWin, &stuff->event, 1, stuff->eventMask,
				      NullGrab, 0))
		return Success;
	    if (pWin == effectiveFocus)
		return Success;
	    stuff->eventMask &= ~wDontPropagateMask(pWin);
	    if (!stuff->eventMask)
		break;
	}
    }
    else if (!XaceHook(XACE_SEND_ACCESS, client, NULL, pWin, &stuff->event, 1))
	(void)DeliverEventsToWindow(pWin, &stuff->event, 1, stuff->eventMask,
				    NullGrab, 0);
    return Success;
}

/**
 * Server-side protocol handling for UngrabKey request.
 *
 * Deletes a passive grab for the given key. Only works on the
 * inputInfo.keyboard.
 */
int
ProcUngrabKey(ClientPtr client)
{
    REQUEST(xUngrabKeyReq);
    WindowPtr pWin;
    GrabRec tempGrab;
    DeviceIntPtr keybd = inputInfo.keyboard;
    int rc;

    REQUEST_SIZE_MATCH(xUngrabKeyReq);
    rc = dixLookupWindow(&pWin, stuff->grabWindow, client, DixReadAccess);
    if (rc != Success)
	return rc;

    if (((stuff->key > keybd->key->curKeySyms.maxKeyCode) ||
	 (stuff->key < keybd->key->curKeySyms.minKeyCode))
	&& (stuff->key != AnyKey))
    {
	client->errorValue = stuff->key;
        return BadValue;
    }
    if ((stuff->modifiers != AnyModifier) &&
	(stuff->modifiers & ~AllModifiersMask))
    {
	client->errorValue = stuff->modifiers;
	return BadValue;
    }
    tempGrab.resource = client->clientAsMask;
    tempGrab.device = keybd;
    tempGrab.window = pWin;
    tempGrab.modifiersDetail.exact = stuff->modifiers;
    tempGrab.modifiersDetail.pMask = NULL;
    tempGrab.modifierDevice = inputInfo.keyboard;
    tempGrab.type = KeyPress;
    tempGrab.detail.exact = stuff->key;
    tempGrab.detail.pMask = NULL;

    if (!DeletePassiveGrabFromList(&tempGrab))
	return(BadAlloc);
    return(Success);
}

/**
 * Server-side protocol handling for GrabKey request.
 *
 * Creates a grab for the inputInfo.keyboard and adds it to the list of
 * passive grabs. 
 */
int
ProcGrabKey(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xGrabKeyReq);
    GrabPtr grab;
    DeviceIntPtr keybd = inputInfo.keyboard;
    int rc;

    REQUEST_SIZE_MATCH(xGrabKeyReq);
    if ((stuff->ownerEvents != xTrue) && (stuff->ownerEvents != xFalse))
    {
	client->errorValue = stuff->ownerEvents;
	return(BadValue);
    }
    if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
        return BadValue;
    }
    if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
        return BadValue;
    }
    if (((stuff->key > keybd->key->curKeySyms.maxKeyCode) ||
	 (stuff->key < keybd->key->curKeySyms.minKeyCode))
	&& (stuff->key != AnyKey))
    {
	client->errorValue = stuff->key;
        return BadValue;
    }
    if ((stuff->modifiers != AnyModifier) &&
	(stuff->modifiers & ~AllModifiersMask))
    {
	client->errorValue = stuff->modifiers;
	return BadValue;
    }
    rc = dixLookupWindow(&pWin, stuff->grabWindow, client, DixSetAttrAccess);
    if (rc != Success)
	return rc;

    grab = CreateGrab(client->index, keybd, pWin, 
	(Mask)(KeyPressMask | KeyReleaseMask), (Bool)stuff->ownerEvents,
	(Bool)stuff->keyboardMode, (Bool)stuff->pointerMode,
	keybd, stuff->modifiers, KeyPress, stuff->key, 
	NullWindow, NullCursor);
    if (!grab)
	return BadAlloc;
    return AddPassiveGrabToList(client, grab);
}


/**
 * Server-side protocol handling for GrabButton request.
 *
 * Creates a grab for the inputInfo.pointer and adds it as a passive grab to
 * the list.
 */
int
ProcGrabButton(ClientPtr client)
{
    WindowPtr pWin, confineTo;
    REQUEST(xGrabButtonReq);
    CursorPtr cursor;
    GrabPtr grab;
    Mask access_mode = DixGrabAccess;
    int rc;

    REQUEST_SIZE_MATCH(xGrabButtonReq);
    if ((stuff->pointerMode != GrabModeSync) &&
	(stuff->pointerMode != GrabModeAsync))
    {
	client->errorValue = stuff->pointerMode;
        return BadValue;
    }
    if ((stuff->keyboardMode != GrabModeSync) &&
	(stuff->keyboardMode != GrabModeAsync))
    {
	client->errorValue = stuff->keyboardMode;
        return BadValue;
    }
    if ((stuff->modifiers != AnyModifier) &&
	(stuff->modifiers & ~AllModifiersMask))
    {
	client->errorValue = stuff->modifiers;
	return BadValue;
    }
    if ((stuff->ownerEvents != xFalse) && (stuff->ownerEvents != xTrue))
    {
	client->errorValue = stuff->ownerEvents;
	return BadValue;
    }
    if (stuff->eventMask & ~PointerGrabMask)
    {
	client->errorValue = stuff->eventMask;
        return BadValue;
    }
    rc = dixLookupWindow(&pWin, stuff->grabWindow, client, DixSetAttrAccess);
    if (rc != Success)
	return rc;
    if (stuff->confineTo == None)
       confineTo = NullWindow;
    else {
	rc = dixLookupWindow(&confineTo, stuff->confineTo, client,
			     DixSetAttrAccess);
	if (rc != Success)
	    return rc;
    }
    if (stuff->cursor == None)
	cursor = NullCursor;
    else
    {
	rc = dixLookupResource((pointer *)&cursor, stuff->cursor, RT_CURSOR,
			       client, DixUseAccess);
	if (rc != Success)
	if (!cursor)
	{
	    client->errorValue = stuff->cursor;
	    return (rc == BadValue) ? BadCursor : rc;
	}
	access_mode |= DixForceAccess;
    }
    if (stuff->pointerMode == GrabModeSync ||
	stuff->keyboardMode == GrabModeSync)
	access_mode |= DixFreezeAccess;
    rc = XaceHook(XACE_DEVICE_ACCESS, client, inputInfo.pointer, access_mode);
    if (rc != Success)
	return rc;

    grab = CreateGrab(client->index, inputInfo.pointer, pWin, 
        (Mask)stuff->eventMask, (Bool)stuff->ownerEvents,
        (Bool) stuff->keyboardMode, (Bool)stuff->pointerMode,
        inputInfo.keyboard, stuff->modifiers, ButtonPress,
        stuff->button, confineTo, cursor);
    if (!grab)
	return BadAlloc;
    return AddPassiveGrabToList(client, grab);
}

/**
 * Server-side protocol handling for UngrabButton request.
 *
 * Deletes a passive grab on the inputInfo.pointer from the list.
 */
int
ProcUngrabButton(ClientPtr client)
{
    REQUEST(xUngrabButtonReq);
    WindowPtr pWin;
    GrabRec tempGrab;
    int rc;

    REQUEST_SIZE_MATCH(xUngrabButtonReq);
    if ((stuff->modifiers != AnyModifier) &&
	(stuff->modifiers & ~AllModifiersMask))
    {
	client->errorValue = stuff->modifiers;
	return BadValue;
    }
    rc = dixLookupWindow(&pWin, stuff->grabWindow, client, DixReadAccess);
    if (rc != Success)
	return rc;
    tempGrab.resource = client->clientAsMask;
    tempGrab.device = inputInfo.pointer;
    tempGrab.window = pWin;
    tempGrab.modifiersDetail.exact = stuff->modifiers;
    tempGrab.modifiersDetail.pMask = NULL;
    tempGrab.modifierDevice = inputInfo.keyboard;
    tempGrab.type = ButtonPress;
    tempGrab.detail.exact = stuff->button;
    tempGrab.detail.pMask = NULL;

    if (!DeletePassiveGrabFromList(&tempGrab))
	return(BadAlloc);
    return(Success);
}

/**
 * Deactivate any grab that may be on the window, remove the focus.
 * Delete any XInput extension events from the window too. Does not change the
 * window mask. Use just before the window is deleted.
 *
 * If freeResources is set, passive grabs on the window are deleted.
 *
 * @param pWin The window to delete events from.
 * @param freeResources True if resources associated with the window should be
 * deleted.
 */
void
DeleteWindowFromAnyEvents(WindowPtr pWin, Bool freeResources)
{
    WindowPtr		parent;
    DeviceIntPtr	mouse = inputInfo.pointer;
    DeviceIntPtr	keybd = inputInfo.keyboard;
    FocusClassPtr	focus = keybd->focus;
    OtherClientsPtr	oc;
    GrabPtr		passive;


    /* Deactivate any grabs performed on this window, before making any
	input focus changes. */

    if (mouse->grab &&
	((mouse->grab->window == pWin) || (mouse->grab->confineTo == pWin)))
	(*mouse->DeactivateGrab)(mouse);

    /* Deactivating a keyboard grab should cause focus events. */

    if (keybd->grab && (keybd->grab->window == pWin))
	(*keybd->DeactivateGrab)(keybd);

    /* If the focus window is a root window (ie. has no parent) then don't 
	delete the focus from it. */
    
    if ((pWin == focus->win) && (pWin->parent != NullWindow))
    {
	int focusEventMode = NotifyNormal;

 	/* If a grab is in progress, then alter the mode of focus events. */

	if (keybd->grab)
	    focusEventMode = NotifyWhileGrabbed;

	switch (focus->revert)
	{
	case RevertToNone:
	    DoFocusEvents(keybd, pWin, NoneWin, focusEventMode);
	    focus->win = NoneWin;
	    focus->traceGood = 0;
	    break;
	case RevertToParent:
	    parent = pWin;
	    do
	    {
		parent = parent->parent;
		focus->traceGood--;
	    } while (!parent->realized
/* This would be a good protocol change -- windows being reparented
   during SaveSet processing would cause the focus to revert to the
   nearest enclosing window which will survive the death of the exiting
   client, instead of ending up reverting to a dying window and thence
   to None
 */
#ifdef NOTDEF
 	      || clients[CLIENT_ID(parent->drawable.id)]->clientGone
#endif
		);
	    DoFocusEvents(keybd, pWin, parent, focusEventMode);
	    focus->win = parent;
	    focus->revert = RevertToNone;
	    break;
	case RevertToPointerRoot:
	    DoFocusEvents(keybd, pWin, PointerRootWin, focusEventMode);
	    focus->win = PointerRootWin;
	    focus->traceGood = 0;
	    break;
	}
    }

    if (mouse->valuator->motionHintWindow == pWin)
	mouse->valuator->motionHintWindow = NullWindow;

    if (freeResources)
    {
	if (pWin->dontPropagate)
	    DontPropagateRefCnts[pWin->dontPropagate]--;
	while ( (oc = wOtherClients(pWin)) )
	    FreeResource(oc->resource, RT_NONE);
	while ( (passive = wPassiveGrabs(pWin)) )
	    FreeResource(passive->resource, RT_NONE);
     }
#ifdef XINPUT
    DeleteWindowFromAnyExtEvents(pWin, freeResources);
#endif
}

/**
 * Call this whenever some window at or below pWin has changed geometry. If
 * there is a grab on the window, the cursor will be re-confined into the
 * window.
 */
_X_EXPORT void
CheckCursorConfinement(WindowPtr pWin)
{
    GrabPtr grab = inputInfo.pointer->grab;
    WindowPtr confineTo;

#ifdef PANORAMIX
    if(!noPanoramiXExtension && pWin->drawable.pScreen->myNum) return;
#endif

    if (grab && (confineTo = grab->confineTo))
    {
	if (!BorderSizeNotEmpty(confineTo))
	    (*inputInfo.pointer->DeactivateGrab)(inputInfo.pointer);
	else if ((pWin == confineTo) || IsParent(pWin, confineTo))
	    ConfineCursorToWindow(confineTo, TRUE, TRUE);
    }
}

Mask
EventMaskForClient(WindowPtr pWin, ClientPtr client)
{
    OtherClientsPtr	other;

    if (wClient (pWin) == client)
	return pWin->eventMask;
    for (other = wOtherClients(pWin); other; other = other->next)
    {
	if (SameClient(other, client))
	    return other->mask;
    }
    return 0;
}

/**
 * Server-side protocol handling for RecolorCursor request.
 */
int
ProcRecolorCursor(ClientPtr client)
{
    CursorPtr pCursor;
    int		rc, nscr;
    ScreenPtr	pscr;
    Bool 	displayed;
    REQUEST(xRecolorCursorReq);

    REQUEST_SIZE_MATCH(xRecolorCursorReq);
    rc = dixLookupResource((pointer *)&pCursor, stuff->cursor, RT_CURSOR,
			   client, DixWriteAccess);
    if (rc != Success)
    {
	client->errorValue = stuff->cursor;
	return (rc == BadValue) ? BadCursor : rc;
    }

    pCursor->foreRed = stuff->foreRed;
    pCursor->foreGreen = stuff->foreGreen;
    pCursor->foreBlue = stuff->foreBlue;

    pCursor->backRed = stuff->backRed;
    pCursor->backGreen = stuff->backGreen;
    pCursor->backBlue = stuff->backBlue;

    for (nscr = 0; nscr < screenInfo.numScreens; nscr++)
    {
	pscr = screenInfo.screens[nscr];
#ifdef PANORAMIX
	if(!noPanoramiXExtension)
	    displayed = (pscr == sprite.screen);
	else
#endif
	    displayed = (pscr == sprite.hotPhys.pScreen);
	( *pscr->RecolorCursor)(pscr, pCursor,
				(pCursor == sprite.current) && displayed);
    }
    return (Success);
}

/**
 * Write the given events to a client, swapping the byte order if necessary.
 * To swap the byte ordering, a callback is called that has to be set up for
 * the given event type.
 *
 * In the case of DeviceMotionNotify trailed by DeviceValuators, the events
 * can be more than one. Usually it's just one event. 
 *
 * Do not modify the event structure passed in. See comment below.
 * 
 * @param pClient Client to send events to.
 * @param count Number of events.
 * @param events The event list.
 */
_X_EXPORT void
WriteEventsToClient(ClientPtr pClient, int count, xEvent *events)
{
#ifdef PANORAMIX
    xEvent    eventCopy;
#endif
    xEvent    eventTo, *eventFrom;
    int       i;

#ifdef XKB
    if ((!noXkbExtension)&&(!XkbFilterEvents(pClient, count, events)))
	return;
#endif

#ifdef PANORAMIX
    if(!noPanoramiXExtension && 
       (panoramiXdataPtr[0].x || panoramiXdataPtr[0].y)) 
    {
	switch(events->u.u.type) {
	case MotionNotify:
	case ButtonPress:
	case ButtonRelease:
	case KeyPress:
	case KeyRelease:
	case EnterNotify:
	case LeaveNotify:
	/* 
	   When multiple clients want the same event DeliverEventsToWindow
	   passes the same event structure multiple times so we can't 
	   modify the one passed to us 
        */
	    count = 1;  /* should always be 1 */
	    memcpy(&eventCopy, events, sizeof(xEvent));
	    eventCopy.u.keyButtonPointer.rootX += panoramiXdataPtr[0].x;
	    eventCopy.u.keyButtonPointer.rootY += panoramiXdataPtr[0].y;
	    if(eventCopy.u.keyButtonPointer.event == 
	       eventCopy.u.keyButtonPointer.root) 
	    {
		eventCopy.u.keyButtonPointer.eventX += panoramiXdataPtr[0].x;
		eventCopy.u.keyButtonPointer.eventY += panoramiXdataPtr[0].y;
	    }
	    events = &eventCopy;
	    break;
	default: break;
	}
    }
#endif

    if (EventCallback)
    {
	EventInfoRec eventinfo;
	eventinfo.client = pClient;
	eventinfo.events = events;
	eventinfo.count = count;
	CallCallbacks(&EventCallback, (pointer)&eventinfo);
    }
#ifdef XSERVER_DTRACE
    if (XSERVER_SEND_EVENT_ENABLED()) {
	for (i = 0; i < count; i++)
	{
	    XSERVER_SEND_EVENT(pClient->index, events[i].u.u.type, &events[i]);
	}
    }
#endif	
    if(pClient->swapped)
    {
	for(i = 0; i < count; i++)
	{
	    eventFrom = &events[i];
	    /* Remember to strip off the leading bit of type in case
	       this event was sent with "SendEvent." */
	    (*EventSwapVector[eventFrom->u.u.type & 0177])
		(eventFrom, &eventTo);
	    (void)WriteToClient(pClient, sizeof(xEvent), (char *)&eventTo);
	}
    }
    else
    {
	(void)WriteToClient(pClient, count * sizeof(xEvent), (char *) events);
    }
}
