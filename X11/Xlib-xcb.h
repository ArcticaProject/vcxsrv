/* Copyright (C) 2003-2006 Jamey Sharp, Josh Triplett
 * This file is licensed under the MIT license. See the file COPYING. */

#ifndef XLIB_XCB_H
#define XLIB_XCB_H

#include <xcb/xcb.h>
#include <X11/Xlib.h>

xcb_connection_t *XGetXCBConnection(Display *dpy);

enum XEventQueueOwner { XlibOwnsEventQueue = 0, XCBOwnsEventQueue };
void XSetEventQueueOwner(Display *dpy, enum XEventQueueOwner owner);

#endif /* XLIB_XCB_H */
