/*

Copyright 1993 by Davor Matic

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation.  Davor Matic makes no representations about
the suitability of this software for any purpose.  It is provided "as
is" without express or implied warranty.

*/

#ifdef HAVE_XNEST_CONFIG_H
#include <xnest-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "screenint.h"
#include "input.h"
#include "misc.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "servermd.h"
#include "inputstr.h"

#include "mi.h"

#include "Xnest.h"

#include "Args.h"
#include "Color.h"
#include "Display.h"
#include "Screen.h"
#include "XNWindow.h"
#include "Events.h"
#include "Keyboard.h"
#include "Pointer.h"
#include "mipointer.h"

CARD32 lastEventTime = 0;

extern EventList *xnestEvents;

void
ProcessInputEvents(void)
{
  mieqProcessInputEvents();
}

int
TimeSinceLastInputEvent(void)
{
    if (lastEventTime == 0)
        lastEventTime = GetTimeInMillis();
    return GetTimeInMillis() - lastEventTime;
}

void
SetTimeSinceLastInputEvent(void)
{
  lastEventTime = GetTimeInMillis();
}

static Bool
xnestExposurePredicate(Display *display, XEvent *event, char *args)
{
  return (event->type == Expose || event->type == ProcessedExpose);
}

static Bool
xnestNotExposurePredicate(Display *display, XEvent *event, char *args)
{
  return !xnestExposurePredicate(display, event, args);
}

void
xnestCollectExposures(void)
{
  XEvent X;
  WindowPtr pWin;
  RegionRec Rgn;
  BoxRec Box;
  
  while (XCheckIfEvent(xnestDisplay, &X, xnestExposurePredicate, NULL)) {
    pWin = xnestWindowPtr(X.xexpose.window);
    
    if (pWin && X.xexpose.width && X.xexpose.height) {
      Box.x1 = pWin->drawable.x + wBorderWidth(pWin) + X.xexpose.x;
      Box.y1 = pWin->drawable.y + wBorderWidth(pWin) + X.xexpose.y;
      Box.x2 = Box.x1 + X.xexpose.width;
      Box.y2 = Box.y1 + X.xexpose.height;
      
      REGION_INIT(pWin->drawable.pScreen, &Rgn, &Box, 1);
      
      miSendExposures(pWin, &Rgn, Box.x2, Box.y2);
    }
  }
}

void
xnestQueueKeyEvent(int type, unsigned int keycode)
{
  int i, n;

  GetEventList(&xnestEvents);
  lastEventTime = GetTimeInMillis();
  n = GetKeyboardEvents(xnestEvents, xnestKeyboardDevice, type, keycode);
  for (i = 0; i < n; i++)
    mieqEnqueue(xnestKeyboardDevice, (InternalEvent*)(xnestEvents + i)->event);
}

void
xnestCollectEvents(void)
{
  XEvent X;
  int i, n, valuators[2];
  ScreenPtr pScreen;
  GetEventList(&xnestEvents);

  while (XCheckIfEvent(xnestDisplay, &X, xnestNotExposurePredicate, NULL)) {
    switch (X.type) {
    case KeyPress:
      xnestUpdateModifierState(X.xkey.state);
      xnestQueueKeyEvent(KeyPress, X.xkey.keycode);
      break;
      
    case KeyRelease:
      xnestUpdateModifierState(X.xkey.state);
      xnestQueueKeyEvent(KeyRelease, X.xkey.keycode);
      break;
      
    case ButtonPress:
      xnestUpdateModifierState(X.xkey.state);
      lastEventTime = GetTimeInMillis();
      n = GetPointerEvents(xnestEvents, xnestPointerDevice, ButtonPress,
                           X.xbutton.button, POINTER_RELATIVE, 0, 0, NULL);
      for (i = 0; i < n; i++)
        mieqEnqueue(xnestPointerDevice, (InternalEvent*)(xnestEvents + i)->event);
      break;
      
    case ButtonRelease:
      xnestUpdateModifierState(X.xkey.state);
      lastEventTime = GetTimeInMillis();
      n = GetPointerEvents(xnestEvents, xnestPointerDevice, ButtonRelease,
                           X.xbutton.button, POINTER_RELATIVE, 0, 0, NULL);
      for (i = 0; i < n; i++)
        mieqEnqueue(xnestPointerDevice, (InternalEvent*)(xnestEvents + i)->event);
      break;
      
    case MotionNotify:
      valuators[0] = X.xmotion.x;
      valuators[1] = X.xmotion.y;
      lastEventTime = GetTimeInMillis();
      n = GetPointerEvents(xnestEvents, xnestPointerDevice, MotionNotify,
                           0, POINTER_ABSOLUTE, 0, 2, valuators);
      for (i = 0; i < n; i++)
        mieqEnqueue(xnestPointerDevice, (InternalEvent*)(xnestEvents + i)->event);
      break;
      
    case FocusIn:
      if (X.xfocus.detail != NotifyInferior) {
	pScreen = xnestScreen(X.xfocus.window);
	if (pScreen)
	  xnestDirectInstallColormaps(pScreen);
      }
      break;
   
    case FocusOut:
      if (X.xfocus.detail != NotifyInferior) {
	pScreen = xnestScreen(X.xfocus.window);
	if (pScreen)
	  xnestDirectUninstallColormaps(pScreen);
      }
      break;

    case KeymapNotify:
      break;

    case EnterNotify:
      if (X.xcrossing.detail != NotifyInferior) {
	pScreen = xnestScreen(X.xcrossing.window);
	if (pScreen) {
	  NewCurrentScreen(inputInfo.pointer, pScreen, X.xcrossing.x, X.xcrossing.y);
          valuators[0] = X.xcrossing.x;
          valuators[1] = X.xcrossing.y;
          lastEventTime = GetTimeInMillis();
          n = GetPointerEvents(xnestEvents, xnestPointerDevice, MotionNotify,
                               0, POINTER_ABSOLUTE, 0, 2, valuators);
          for (i = 0; i < n; i++)
            mieqEnqueue(xnestPointerDevice, (InternalEvent*)(xnestEvents + i)->event);
	  xnestDirectInstallColormaps(pScreen);
	}
      }
      break;
      
    case LeaveNotify:
      if (X.xcrossing.detail != NotifyInferior) {
	pScreen = xnestScreen(X.xcrossing.window);
	if (pScreen) {
	  xnestDirectUninstallColormaps(pScreen);
	}	
      }
      break;
      
    case DestroyNotify:
      if (xnestParentWindow != (Window) 0 &&
	  X.xdestroywindow.window == xnestParentWindow)
	exit (0);
      break;

    case CirculateNotify:
    case ConfigureNotify:
    case GravityNotify:
    case MapNotify:
    case ReparentNotify:
    case UnmapNotify:
      break;
      
    default:
      ErrorF("xnest warning: unhandled event\n");
      break;
    }
  }
}
