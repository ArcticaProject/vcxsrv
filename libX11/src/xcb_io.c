/* Copyright (C) 2003-2006 Jamey Sharp, Josh Triplett
 * This file is licensed under the MIT license. See the file COPYING. */

#include "Xlibint.h"
#include "locking.h"
#include "Xprivate.h"
#include "Xxcbint.h"
#include <xcb/xcbext.h>

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void return_socket(void *closure)
{
	Display *dpy = closure;
	LockDisplay(dpy);
	_XSend(dpy, NULL, 0);
	dpy->bufmax = dpy->buffer;
	UnlockDisplay(dpy);
}

static void require_socket(Display *dpy)
{
	if(dpy->bufmax == dpy->buffer)
	{
		uint64_t sent;
		int flags = 0;
		/* if we don't own the event queue, we have to ask XCB
		 * to set our errors aside for us. */
		if(dpy->xcb->event_owner != XlibOwnsEventQueue)
			flags = XCB_REQUEST_CHECKED;
		if(!xcb_take_socket(dpy->xcb->connection, return_socket, dpy,
		                    flags, &sent))
			_XIOError(dpy);
		/* Xlib uses unsigned long for sequence numbers.  XCB
		 * uses 64-bit internally, but currently exposes an
		 * unsigned int API.  If these differ, Xlib cannot track
		 * the full 64-bit sequence number if 32-bit wrap
		 * happens while Xlib does not own the socket.  A
		 * complete fix would be to make XCB's public API use
		 * 64-bit sequence numbers. */
		assert(!(sizeof(unsigned long) > sizeof(unsigned int)
		         && dpy->xcb->event_owner == XlibOwnsEventQueue
		         && (sent - dpy->last_request_read >= (UINT64_C(1) << 32))));
		dpy->xcb->last_flushed = dpy->request = sent;
		dpy->bufmax = dpy->xcb->real_bufmax;
	}
}

/* Call internal connection callbacks for any fds that are currently
 * ready to read. This function will not block unless one of the
 * callbacks blocks.
 *
 * This code borrowed from _XWaitForReadable. Inverse call tree:
 * _XRead
 *  _XWaitForWritable
 *   _XFlush
 *   _XSend
 *  _XEventsQueued
 *  _XReadEvents
 *  _XRead[0-9]+
 *   _XAllocIDs
 *  _XReply
 *  _XEatData
 * _XReadPad
 */
static void check_internal_connections(Display *dpy)
{
	struct _XConnectionInfo *ilist;
	fd_set r_mask;
	struct timeval tv;
	int result;
	int highest_fd = -1;

	if(dpy->flags & XlibDisplayProcConni || !dpy->im_fd_info)
		return;

	FD_ZERO(&r_mask);
	for(ilist = dpy->im_fd_info; ilist; ilist = ilist->next)
	{
		assert(ilist->fd >= 0);
		FD_SET(ilist->fd, &r_mask);
		if(ilist->fd > highest_fd)
			highest_fd = ilist->fd;
	}
	assert(highest_fd >= 0);

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	result = select(highest_fd + 1, &r_mask, NULL, NULL, &tv);

	if(result == -1)
	{
		if(errno == EINTR)
			return;
		_XIOError(dpy);
	}

	for(ilist = dpy->im_fd_info; result && ilist; ilist = ilist->next)
		if(FD_ISSET(ilist->fd, &r_mask))
		{
			_XProcessInternalConnection(dpy, ilist);
			--result;
		}
}

static void call_handlers(Display *dpy, xcb_generic_reply_t *buf)
{
	_XAsyncHandler *async, *next;
	for(async = dpy->async_handlers; async; async = next)
	{
		next = async->next;
		if(async->handler(dpy, (xReply *) buf, (char *) buf, sizeof(xReply) + (buf->length << 2), async->data))
			return;
	}
	if(buf->response_type == 0) /* unhandled error */
	    _XError(dpy, (xError *) buf);
}

static xcb_generic_event_t * wait_or_poll_for_event(Display *dpy, int wait)
{
	xcb_connection_t *c = dpy->xcb->connection;
	xcb_generic_event_t *event;
	if(wait)
	{
		if(dpy->xcb->event_waiter)
		{
			ConditionWait(dpy, dpy->xcb->event_notify);
			event = xcb_poll_for_event(c);
		}
		else
		{
			dpy->xcb->event_waiter = 1;
			UnlockDisplay(dpy);
			event = xcb_wait_for_event(c);
			LockDisplay(dpy);
			dpy->xcb->event_waiter = 0;
			ConditionBroadcast(dpy, dpy->xcb->event_notify);
		}
	}
	else
		event = xcb_poll_for_event(c);
	return event;
}

/* Widen a 32-bit sequence number into a native-word-size (unsigned long)
 * sequence number.  Treating the comparison as a 1 and shifting it avoids a
 * conditional branch, and shifting by 16 twice avoids a compiler warning when
 * sizeof(unsigned long) == 4. */
static void widen(unsigned long *wide, unsigned int narrow)
{
	unsigned long new = (*wide & ~0xFFFFFFFFUL) | narrow;
	*wide = new + ((unsigned long) (new < *wide) << 16 << 16);
}

static void process_responses(Display *dpy, int wait_for_first_event, xcb_generic_error_t **current_error, unsigned long current_request)
{
	void *reply;
	xcb_generic_event_t *event = dpy->xcb->next_event;
	xcb_generic_error_t *error;
	xcb_connection_t *c = dpy->xcb->connection;
	if(!event && dpy->xcb->event_owner == XlibOwnsEventQueue)
		event = wait_or_poll_for_event(dpy, wait_for_first_event);

	require_socket(dpy);

	while(1)
	{
		PendingRequest *req = dpy->xcb->pending_requests;
		unsigned long event_sequence = dpy->last_request_read;
		if(event)
			widen(&event_sequence, event->full_sequence);
		assert(!(req && current_request && !XLIB_SEQUENCE_COMPARE(req->sequence, <=, current_request)));
		if(event && (!req || XLIB_SEQUENCE_COMPARE(event_sequence, <=, req->sequence)))
		{
			dpy->last_request_read = event_sequence;
			if(event->response_type != X_Error)
			{
				/* GenericEvents may be > 32 bytes. In this
				 * case, the event struct is trailed by the
				 * additional bytes. the xcb_generic_event_t
				 * struct uses 4 bytes for internal numbering,
				 * so we need to shift the trailing data to be
				 * after the first 32 bytes.  */
                                if (event->response_type == GenericEvent &&
                                        ((xcb_ge_event_t*)event)->length)
				{
					memmove(&event->full_sequence,
                                                &event[1],
						((xcb_ge_event_t*)event)->length * 4);
				}
				_XEnq(dpy, (xEvent *) event);
				wait_for_first_event = 0;
			}
			else if(current_error && event_sequence == current_request)
			{
				/* This can only occur when called from
				 * _XReply, which doesn't need a new event. */
				*current_error = (xcb_generic_error_t *) event;
				event = NULL;
				break;
			}
			else
				_XError(dpy, (xError *) event);
			free(event);
			event = wait_or_poll_for_event(dpy, wait_for_first_event);
		}
		else if(req && req->sequence == current_request)
		{
			break;
		}
		else if(req && xcb_poll_for_reply(dpy->xcb->connection, req->sequence, &reply, &error))
		{
			uint64_t sequence = req->sequence;
			if(!reply)
			{
				dpy->xcb->pending_requests = req->next;
				if(!dpy->xcb->pending_requests)
					dpy->xcb->pending_requests_tail = &dpy->xcb->pending_requests;
				free(req);
				reply = error;
			}
			if(reply)
			{
				dpy->last_request_read = sequence;
				call_handlers(dpy, reply);
				free(reply);
			}
		}
		else
			break;
	}

	dpy->xcb->next_event = event;

	if(xcb_connection_has_error(c))
		_XIOError(dpy);

	assert(XLIB_SEQUENCE_COMPARE(dpy->last_request_read, <=, dpy->request));
}

int _XEventsQueued(Display *dpy, int mode)
{
	if(dpy->flags & XlibDisplayIOError)
		return 0;
	if(dpy->xcb->event_owner != XlibOwnsEventQueue)
		return 0;

	if(mode == QueuedAfterFlush)
		_XSend(dpy, NULL, 0);
	else
		check_internal_connections(dpy);
	process_responses(dpy, 0, NULL, 0);
	return dpy->qlen;
}

/* _XReadEvents - Flush the output queue,
 * then read as many events as possible (but at least 1) and enqueue them
 */
void _XReadEvents(Display *dpy)
{
	if(dpy->flags & XlibDisplayIOError)
		return;
	_XSend(dpy, NULL, 0);
	if(dpy->xcb->event_owner != XlibOwnsEventQueue)
		return;
	check_internal_connections(dpy);
	do {
		process_responses(dpy, 1, NULL, 0);
	} while (dpy->qlen == 0);
}

/*
 * _XSend - Flush the buffer and send the client data. 32 bit word aligned
 * transmission is used, if size is not 0 mod 4, extra bytes are transmitted.
 *
 * Note that the connection must not be read from once the data currently
 * in the buffer has been written.
 */
void _XSend(Display *dpy, const char *data, long size)
{
	static const xReq dummy_request;
	static char const pad[3];
	struct iovec vec[3];
	uint64_t requests;
	_XExtension *ext;
	xcb_connection_t *c = dpy->xcb->connection;
	if(dpy->flags & XlibDisplayIOError)
		return;

	if(dpy->bufptr == dpy->buffer && !size)
		return;

	/* iff we asked XCB to set aside errors, we must pick those up
	 * eventually. iff there are async handlers, we may have just
	 * issued requests that will generate replies. in either case,
	 * we need to remember to check later. */
	if(dpy->xcb->event_owner != XlibOwnsEventQueue || dpy->async_handlers)
	{
		uint64_t sequence;
		for(sequence = dpy->xcb->last_flushed; sequence < dpy->request; ++sequence)
		{
			PendingRequest *req = malloc(sizeof(PendingRequest));
			assert(req);
			req->next = NULL;
			req->sequence = sequence;
			*dpy->xcb->pending_requests_tail = req;
			dpy->xcb->pending_requests_tail = &req->next;
		}
	}
	requests = dpy->request - dpy->xcb->last_flushed;
	dpy->xcb->last_flushed = dpy->request;

	vec[0].iov_base = dpy->buffer;
	vec[0].iov_len = dpy->bufptr - dpy->buffer;
	vec[1].iov_base = (caddr_t) data;
	vec[1].iov_len = size;
	vec[2].iov_base = (caddr_t) pad;
	vec[2].iov_len = -size & 3;

	for(ext = dpy->flushes; ext; ext = ext->next_flush)
	{
		int i;
		for(i = 0; i < 3; ++i)
			if(vec[i].iov_len)
				ext->before_flush(dpy, &ext->codes, vec[i].iov_base, vec[i].iov_len);
	}

	if(xcb_writev(c, vec, 3, requests) < 0)
		_XIOError(dpy);
	dpy->bufptr = dpy->buffer;
	dpy->last_req = (char *) &dummy_request;

	check_internal_connections(dpy);

	_XSetSeqSyncFunction(dpy);
}

/*
 * _XFlush - Flush the X request buffer.  If the buffer is empty, no
 * action is taken.
 */
void _XFlush(Display *dpy)
{
	require_socket(dpy);
	_XSend(dpy, NULL, 0);

	_XEventsQueued(dpy, QueuedAfterReading);
}

static const XID inval_id = ~0UL;

int _XIDHandler(Display *dpy)
{
	XID next;

	if (dpy->xcb->next_xid != inval_id)
	    return 0;

	next = xcb_generate_id(dpy->xcb->connection);
	LockDisplay(dpy);
	dpy->xcb->next_xid = next;
#ifdef XTHREADS
	if (dpy->lock)
		(*dpy->lock->user_unlock_display)(dpy);
#endif
	UnlockDisplay(dpy);
	return 0;
}

/* _XAllocID - resource ID allocation routine. */
XID _XAllocID(Display *dpy)
{
	XID ret = dpy->xcb->next_xid;
	assert (ret != inval_id);
#ifdef XTHREADS
	if (dpy->lock)
		(*dpy->lock->user_lock_display)(dpy);
#endif
	dpy->xcb->next_xid = inval_id;
	_XSetPrivSyncFunction(dpy);
	return ret;
}

/* _XAllocIDs - multiple resource ID allocation routine. */
void _XAllocIDs(Display *dpy, XID *ids, int count)
{
	int i;
#ifdef XTHREADS
	if (dpy->lock)
		(*dpy->lock->user_lock_display)(dpy);
	UnlockDisplay(dpy);
#endif
	for (i = 0; i < count; i++)
		ids[i] = xcb_generate_id(dpy->xcb->connection);
#ifdef XTHREADS
	LockDisplay(dpy);
	if (dpy->lock)
		(*dpy->lock->user_unlock_display)(dpy);
#endif
}

static void _XFreeReplyData(Display *dpy, Bool force)
{
	if(!force && dpy->xcb->reply_consumed < dpy->xcb->reply_length)
		return;
	free(dpy->xcb->reply_data);
	dpy->xcb->reply_data = NULL;
}

static PendingRequest * insert_pending_request(Display *dpy)
{
	PendingRequest **cur = &dpy->xcb->pending_requests;
	while(*cur && XLIB_SEQUENCE_COMPARE((*cur)->sequence, <, dpy->request))
		cur = &((*cur)->next);
	if(!*cur || (*cur)->sequence != dpy->request)
	{
		PendingRequest *node = malloc(sizeof(PendingRequest));
		assert(node);
		node->next = *cur;
		node->sequence = dpy->request;
		if(cur == dpy->xcb->pending_requests_tail)
			dpy->xcb->pending_requests_tail = &(node->next);
		*cur = node;
	}
	return *cur;
}

/*
 * _XReply - Wait for a reply packet and copy its contents into the
 * specified rep.
 * extra: number of 32-bit words expected after the reply
 * discard: should I discard data following "extra" words?
 */
Status _XReply(Display *dpy, xReply *rep, int extra, Bool discard)
{
	xcb_generic_error_t *error;
	xcb_connection_t *c = dpy->xcb->connection;
	char *reply;
	PendingRequest *current;

	assert(!dpy->xcb->reply_data);

	if(dpy->flags & XlibDisplayIOError)
		return 0;

	_XSend(dpy, NULL, 0);
	current = insert_pending_request(dpy);
	/* FIXME: drop the Display lock while waiting?
	 * Complicates process_responses. */
	reply = xcb_wait_for_reply(c, current->sequence, &error);

	check_internal_connections(dpy);
	process_responses(dpy, 0, &error, current->sequence);

	if(error)
	{
		_XExtension *ext;
		xError *err = (xError *) error;
		int ret_code;

		dpy->last_request_read = error->full_sequence;

		/* Xlib is evil and assumes that even errors will be
		 * copied into rep. */
		memcpy(rep, error, 32);

		/* do not die on "no such font", "can't allocate",
		   "can't grab" failures */
		switch(err->errorCode)
		{
			case BadName:
				switch(err->majorCode)
				{
					case X_LookupColor:
					case X_AllocNamedColor:
						free(error);
						return 0;
				}
				break;
			case BadFont:
				if(err->majorCode == X_QueryFont) {
					free(error);
					return 0;
				}
				break;
			case BadAlloc:
			case BadAccess:
				free(error);
				return 0;
		}

		/*
		 * we better see if there is an extension who may
		 * want to suppress the error.
		 */
		for(ext = dpy->ext_procs; ext; ext = ext->next)
			if(ext->error && ext->error(dpy, err, &ext->codes, &ret_code)) {
				free(error);
				return ret_code;
			}

		_XError(dpy, err);
		free(error);
		return 0;
	}

	/* it's not an error, but we don't have a reply, so it's an I/O
	 * error. */
	if(!reply)
	{
		_XIOError(dpy);
		return 0;
	}

	dpy->last_request_read = current->sequence;

	/* there's no error and we have a reply. */
	dpy->xcb->reply_data = reply;
	dpy->xcb->reply_consumed = sizeof(xReply) + (extra * 4);
	dpy->xcb->reply_length = sizeof(xReply);
	if(dpy->xcb->reply_data[0] == 1)
		dpy->xcb->reply_length += (((xcb_generic_reply_t *) dpy->xcb->reply_data)->length * 4);

	/* error: Xlib asks too much. give them what we can anyway. */
	if(dpy->xcb->reply_length < dpy->xcb->reply_consumed)
		dpy->xcb->reply_consumed = dpy->xcb->reply_length;

	memcpy(rep, dpy->xcb->reply_data, dpy->xcb->reply_consumed);
	_XFreeReplyData(dpy, discard);
	return 1;
}

int _XRead(Display *dpy, char *data, long size)
{
	assert(size >= 0);
	if(size == 0)
		return 0;
	assert(dpy->xcb->reply_data != NULL);
	assert(dpy->xcb->reply_consumed + size <= dpy->xcb->reply_length);
	memcpy(data, dpy->xcb->reply_data + dpy->xcb->reply_consumed, size);
	dpy->xcb->reply_consumed += size;
	_XFreeReplyData(dpy, False);
	return 0;
}

/*
 * _XReadPad - Read bytes from the socket taking into account incomplete
 * reads.  If the number of bytes is not 0 mod 4, read additional pad
 * bytes.
 */
void _XReadPad(Display *dpy, char *data, long size)
{
	_XRead(dpy, data, size);
	dpy->xcb->reply_consumed += -size & 3;
	_XFreeReplyData(dpy, False);
}

/* Read and discard "n" 8-bit bytes of data */
void _XEatData(Display *dpy, unsigned long n)
{
	dpy->xcb->reply_consumed += n;
	_XFreeReplyData(dpy, False);
}
