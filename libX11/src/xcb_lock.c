/* Copyright (C) 2003-2006 Jamey Sharp, Josh Triplett
 * This file is licensed under the MIT license. See the file COPYING. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Xlibint.h"
#include "locking.h"
#include "Xxcbint.h"
#include <xcb/xcbext.h>
#include <xcb/xcbxlib.h>

#include <pthread.h>

static void _XCBLockDisplay(Display *dpy)
{
    if(dpy->xcb->lock_fns.lock_display)
	dpy->xcb->lock_fns.lock_display(dpy);
    if(!dpy->lock || dpy->lock->locking_level == 0)
	xcb_xlib_lock(dpy->xcb->connection);
    if(!(dpy->flags & XlibDisplayIOError))
	_XGetXCBBuffer(dpy);
}

/* XXX: If you change this function, update _XReply's copy of its guts! */
static void _XCBUnlockDisplay(Display *dpy)
{
    if(!(dpy->flags & XlibDisplayIOError))
    {
	_XPutXCBBuffer(dpy);
	assert(dpy->xcb->partial_request == 0);
	assert(xcb_get_request_sent(dpy->xcb->connection) == dpy->request);

	/* Traditional Xlib does this in _XSend; see the Xlib/XCB version
	 * of that function for why we do it here instead. */
	_XSetSeqSyncFunction(dpy);
    }

    if(!dpy->lock || dpy->lock->locking_level == 0)
	xcb_xlib_unlock(dpy->xcb->connection);
    if(dpy->xcb->lock_fns.unlock_display)
	dpy->xcb->lock_fns.unlock_display(dpy);
}

int _XCBInitDisplayLock(Display *dpy)
{
    if(!dpy->lock_fns && !(dpy->lock_fns = Xcalloc(1, sizeof(*dpy->lock_fns))))
	return 0;
    dpy->xcb->lock_fns.lock_display = dpy->lock_fns->lock_display;
    dpy->lock_fns->lock_display = _XCBLockDisplay;
    dpy->xcb->lock_fns.unlock_display = dpy->lock_fns->unlock_display;
    dpy->lock_fns->unlock_display = _XCBUnlockDisplay;
    return 1;
}

void _XCBShutdownDisplayLock(Display *dpy)
{
    if(dpy->lock_fns) {
	Xfree((char *)dpy->lock_fns);
	dpy->lock_fns = NULL;
    }
}

void _XGetXCBBuffer(Display *dpy)
{
    static const xReq dummy_request;
    unsigned int xcb_req = xcb_get_request_sent(dpy->xcb->connection);
    if(xcb_connection_has_error(dpy->xcb->connection))
	_XIOError(dpy);

    /* if Xlib has a partial request pending then XCB doesn't know about
     * the current request yet */
    if(dpy->xcb->partial_request)
	++xcb_req;

    assert(XCB_SEQUENCE_COMPARE(xcb_req, >=, dpy->request));
    dpy->request = xcb_req;

    dpy->last_req = (char *) &dummy_request;
}

static size_t request_length(struct iovec *vec)
{
    /* we have at least part of a request. dig out the length field.
     * note that length fields are always in vec[0]: Xlib doesn't split
     * fixed-length request parts. */
    size_t len;
    assert(vec[0].iov_len >= 4);
    len = ((uint16_t *) vec[0].iov_base)[1];
    if(len == 0)
    {
	/* it's a bigrequest. dig out the *real* length field. */
	assert(vec[0].iov_len >= 8);
	len = ((uint32_t *) vec[0].iov_base)[1];
    }
    return len << 2;
}

static inline int issue_complete_request(Display *dpy, int veclen, struct iovec *vec)
{
    xcb_protocol_request_t xcb_req = { 0 };
    unsigned int sequence;
    int flags = XCB_REQUEST_RAW;
    int i;
    size_t len;

    /* skip empty iovecs. if no iovecs remain, we're done. */
    assert(veclen >= 0);
    while(veclen > 0 && vec[0].iov_len == 0)
	--veclen, ++vec;
    if(!veclen)
	return 0;

    len = request_length(vec);

    /* do we have enough data for a complete request? how many iovec
     * elements does it span? */
    for(i = 0; i < veclen; ++i)
    {
	size_t oldlen = len;
	len -= vec[i].iov_len;
	/* if len is now 0 or has wrapped, we have enough data. */
	if((len - 1) > oldlen)
	    break;
    }
    if(i == veclen)
	return 0;

    /* we have enough data to issue one complete request. the remaining
     * code can't fail. */

    /* len says how far we overshot our data needs. (it's "negative" if
     * we actually overshot, or 0 if we're right on.) */
    vec[i].iov_len += len;
    xcb_req.count = i + 1;
    xcb_req.opcode = ((uint8_t *) vec[0].iov_base)[0];

    /* if we don't own the event queue, we have to ask XCB to set our
     * errors aside for us. */
    if(dpy->xcb->event_owner != XlibOwnsEventQueue)
	flags |= XCB_REQUEST_CHECKED;

    /* XCB will always skip request 0; account for that in the Xlib count */
    if (xcb_get_request_sent(dpy->xcb->connection) == 0xffffffff)
	dpy->request++;
    /* send the accumulated request. */
    sequence = xcb_send_request(dpy->xcb->connection, flags, vec, &xcb_req);
    if(!sequence)
	_XIOError(dpy);

    /* update the iovecs to refer only to data not yet sent. */
    vec[i].iov_len = -len;

    /* iff we asked XCB to set aside errors, we must pick those up
     * eventually. iff there are async handlers, we may have just
     * issued requests that will generate replies. in either case,
     * we need to remember to check later. */
    if(flags & XCB_REQUEST_CHECKED || dpy->async_handlers)
    {
	PendingRequest *req = malloc(sizeof(PendingRequest));
	assert(req);
	req->next = 0;
	req->waiters = -1;
	req->sequence = sequence;
	*dpy->xcb->pending_requests_tail = req;
	dpy->xcb->pending_requests_tail = &req->next;
    }
    return 1;
}

void _XPutXCBBuffer(Display *dpy)
{
    static char const pad[3];
    const int padsize = -dpy->xcb->request_extra_size & 3;
    xcb_connection_t *c = dpy->xcb->connection;
    _XExtension *ext;
    struct iovec iov[6];

    assert_sequence_less(dpy->last_request_read, dpy->request);
    assert_sequence_less(xcb_get_request_sent(c), dpy->request);

    for(ext = dpy->flushes; ext; ext = ext->next_flush)
    {
	ext->before_flush(dpy, &ext->codes, dpy->buffer, dpy->bufptr - dpy->buffer);
	if(dpy->xcb->request_extra)
	{
	    ext->before_flush(dpy, &ext->codes, dpy->xcb->request_extra, dpy->xcb->request_extra_size);
	    if(padsize)
		ext->before_flush(dpy, &ext->codes, pad, padsize);
	}
    }

    iov[2].iov_base = dpy->xcb->partial_request;
    iov[2].iov_len = dpy->xcb->partial_request_offset;
    iov[3].iov_base = dpy->buffer;
    iov[3].iov_len = dpy->bufptr - dpy->buffer;
    iov[4].iov_base = (caddr_t) dpy->xcb->request_extra;
    iov[4].iov_len = dpy->xcb->request_extra_size;
    iov[5].iov_base = (caddr_t) pad;
    iov[5].iov_len = padsize;

    while(issue_complete_request(dpy, 4, iov + 2))
	/* empty */;

    /* first discard any completed partial_request. */
    if(iov[2].iov_len == 0 && dpy->xcb->partial_request)
    {
	free(dpy->xcb->partial_request);
	dpy->xcb->partial_request = 0;
	dpy->xcb->partial_request_offset = 0;
    }

    /* is there anything to copy into partial_request? */
    if(iov[3].iov_len != 0 || iov[4].iov_len != 0 || iov[5].iov_len != 0)
    {
	int i;
	if(!dpy->xcb->partial_request)
	{
	    size_t len = request_length(iov + 3);
	    assert(!dpy->xcb->partial_request_offset);
	    dpy->xcb->partial_request = malloc(len);
	    assert(dpy->xcb->partial_request);
	}
	for(i = 3; i < sizeof(iov) / sizeof(*iov); ++i)
	{
	    memcpy(dpy->xcb->partial_request + dpy->xcb->partial_request_offset, iov[i].iov_base, iov[i].iov_len);
	    dpy->xcb->partial_request_offset += iov[i].iov_len;
	}
    }

    dpy->xcb->request_extra = 0;
    dpy->xcb->request_extra_size = 0;
    dpy->bufptr = dpy->buffer;
}
