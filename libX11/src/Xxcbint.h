/* Copyright (C) 2003-2006 Jamey Sharp, Josh Triplett
 * This file is licensed under the MIT license. See the file COPYING. */

#ifndef XXCBINT_H
#define XXCBINT_H

#include <assert.h>
#include <X11/Xlibint.h>
#include <X11/Xlib-xcb.h>
#include "locking.h" /* really just want X11/Xthreads.h but can't have it. */

#define XCB_SEQUENCE_COMPARE(a,op,b)	((int) ((a) - (b)) op 0)
#define assert_sequence_less(a,b) assert(XCB_SEQUENCE_COMPARE((a), <=, (b)))

typedef struct PendingRequest PendingRequest;
struct PendingRequest {
	PendingRequest *next;
	xcondition_rec condition;
	int waiters; /* Number of threads waiting; -1 if no wait needed */
	unsigned int sequence;
};

typedef struct _X11XCBPrivate {
	struct _XLockPtrs lock_fns;
	xcb_connection_t *connection;
	PendingRequest *pending_requests;
	PendingRequest **pending_requests_tail;
	xcb_generic_event_t *next_event;
	const char *request_extra;
	int request_extra_size;
	char *partial_request;
	int partial_request_offset;
	char *reply_data;
	int reply_length;
	int reply_consumed;
	enum XEventQueueOwner event_owner;
	XID next_xid;
} _X11XCBPrivate;

/* xcb_disp.c */

int _XConnectXCB(Display *dpy, _Xconst char *display, char **fullnamep, int *screenp);
void _XFreeX11XCBStructure(Display *dpy);

/* xcb_lock.c */

int _XCBInitDisplayLock(Display *dpy);
void _XCBShutdownDisplayLock(Display *dpy);

/* _XGetXCBBuffer and _XPutXCBBuffer calls must be paired and must not
 * be nested. */

void _XGetXCBBuffer(Display *dpy);
void _XPutXCBBuffer(Display *dpy);

#endif /* XXCBINT_H */
