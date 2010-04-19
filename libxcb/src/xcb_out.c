/* Copyright (C) 2001-2004 Bart Massey and Jamey Sharp.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the names of the authors or their
 * institutions shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the authors.
 */

/* Stuff that sends stuff to the server. */

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "xcb.h"
#include "xcbext.h"
#include "xcbint.h"
#include "bigreq.h"

static int write_block(xcb_connection_t *c, struct iovec *vector, int count)
{
    while(count && c->out.queue_len + vector[0].iov_len <= sizeof(c->out.queue))
    {
        memcpy(c->out.queue + c->out.queue_len, vector[0].iov_base, vector[0].iov_len);
        c->out.queue_len += vector[0].iov_len;
        vector[0].iov_base = (char *) vector[0].iov_base + vector[0].iov_len;
        vector[0].iov_len = 0;
        ++vector, --count;
    }
    if(!count)
        return 1;

    --vector, ++count;
    vector[0].iov_base = c->out.queue;
    vector[0].iov_len = c->out.queue_len;
    c->out.queue_len = 0;
    return _xcb_out_send(c, vector, count);
}

static void get_socket_back(xcb_connection_t *c)
{
    while(c->out.return_socket && c->out.socket_moving)
        pthread_cond_wait(&c->out.socket_cond, &c->iolock);
    if(!c->out.return_socket)
        return;

    c->out.socket_moving = 1;
    pthread_mutex_unlock(&c->iolock);
    c->out.return_socket(c->out.socket_closure);
    pthread_mutex_lock(&c->iolock);
    c->out.socket_moving = 0;

    pthread_cond_broadcast(&c->out.socket_cond);
    c->out.return_socket = 0;
    c->out.socket_closure = 0;
    _xcb_in_replies_done(c);
}

/* Public interface */

void xcb_prefetch_maximum_request_length(xcb_connection_t *c)
{
    if(c->has_error)
        return;
    pthread_mutex_lock(&c->out.reqlenlock);
    if(c->out.maximum_request_length_tag == LAZY_NONE)
    {
        const xcb_query_extension_reply_t *ext;
        ext = xcb_get_extension_data(c, &xcb_big_requests_id);
        if(ext && ext->present)
        {
            c->out.maximum_request_length_tag = LAZY_COOKIE;
            c->out.maximum_request_length.cookie = xcb_big_requests_enable(c);
        }
        else
        {
            c->out.maximum_request_length_tag = LAZY_FORCED;
            c->out.maximum_request_length.value = c->setup->maximum_request_length;
        }
    }
    pthread_mutex_unlock(&c->out.reqlenlock);
}

uint32_t xcb_get_maximum_request_length(xcb_connection_t *c)
{
    if(c->has_error)
        return 0;
    xcb_prefetch_maximum_request_length(c);
    pthread_mutex_lock(&c->out.reqlenlock);
    if(c->out.maximum_request_length_tag == LAZY_COOKIE)
    {
        xcb_big_requests_enable_reply_t *r = xcb_big_requests_enable_reply(c, c->out.maximum_request_length.cookie, 0);
        c->out.maximum_request_length_tag = LAZY_FORCED;
        if(r)
        {
            c->out.maximum_request_length.value = r->maximum_request_length;
            free(r);
        }
        else
            c->out.maximum_request_length.value = c->setup->maximum_request_length;
    }
    pthread_mutex_unlock(&c->out.reqlenlock);
    return c->out.maximum_request_length.value;
}

unsigned int xcb_send_request(xcb_connection_t *c, int flags, struct iovec *vector, const xcb_protocol_request_t *req)
{
    static const union {
        struct {
            uint8_t major;
            uint8_t pad;
            uint16_t len;
        } fields;
        uint32_t packet;
    } sync_req = { { /* GetInputFocus */ 43, 0, 1 } };
    uint64_t request;
    uint32_t prefix[3] = { 0 };
    int veclen = req->count;
    enum workarounds workaround = WORKAROUND_NONE;

    if(c->has_error)
        return 0;

    assert(c != 0);
    assert(vector != 0);
    assert(req->count > 0);

    if(!(flags & XCB_REQUEST_RAW))
    {
        static const char pad[3];
        unsigned int i;
        uint16_t shortlen = 0;
        size_t longlen = 0;
        assert(vector[0].iov_len >= 4);
        /* set the major opcode, and the minor opcode for extensions */
        if(req->ext)
        {
            const xcb_query_extension_reply_t *extension = xcb_get_extension_data(c, req->ext);
            if(!(extension && extension->present))
            {
                _xcb_conn_shutdown(c);
                return 0;
            }
            ((uint8_t *) vector[0].iov_base)[0] = extension->major_opcode;
            ((uint8_t *) vector[0].iov_base)[1] = req->opcode;
        }
        else
            ((uint8_t *) vector[0].iov_base)[0] = req->opcode;

        /* put together the length field, possibly using BIGREQUESTS */
        for(i = 0; i < req->count; ++i)
        {
            longlen += vector[i].iov_len;
            if(!vector[i].iov_base)
            {
                vector[i].iov_base = (char *) pad;
                assert(vector[i].iov_len <= sizeof(pad));
            }
        }
        assert((longlen & 3) == 0);
        longlen >>= 2;

        if(longlen <= c->setup->maximum_request_length)
        {
            /* we don't need BIGREQUESTS. */
            shortlen = longlen;
            longlen = 0;
        }
        else if(longlen > xcb_get_maximum_request_length(c))
        {
            _xcb_conn_shutdown(c);
            return 0; /* server can't take this; maybe need BIGREQUESTS? */
        }

        /* set the length field. */
        ((uint16_t *) vector[0].iov_base)[1] = shortlen;
        if(!shortlen)
            prefix[2] = ++longlen;
    }
    flags &= ~XCB_REQUEST_RAW;

    /* do we need to work around the X server bug described in glx.xml? */
    /* XXX: GetFBConfigs won't use BIG-REQUESTS in any sane
     * configuration, but that should be handled here anyway. */
    if(req->ext && !req->isvoid && !strcmp(req->ext->name, "GLX") &&
            ((req->opcode == 17 && ((uint32_t *) vector[0].iov_base)[1] == 0x10004) ||
             req->opcode == 21))
        workaround = WORKAROUND_GLX_GET_FB_CONFIGS_BUG;

    /* get a sequence number and arrange for delivery. */
    pthread_mutex_lock(&c->iolock);
    /* wait for other writing threads to get out of my way. */
    while(c->out.writing)
        pthread_cond_wait(&c->out.cond, &c->iolock);
    get_socket_back(c);

    request = ++c->out.request;
    /* send GetInputFocus (sync_req) when 64k-2 requests have been sent without
     * a reply.
     * Also send sync_req (could use NoOp) at 32-bit wrap to avoid having
     * applications see sequence 0 as that is used to indicate
     * an error in sending the request */
    while((req->isvoid &&
	c->out.request == c->in.request_expected + (1 << 16) - 1) ||
       request == 0)
    {
        prefix[0] = sync_req.packet;
        _xcb_in_expect_reply(c, request, WORKAROUND_NONE, XCB_REQUEST_DISCARD_REPLY);
        c->in.request_expected = c->out.request;
	request = ++c->out.request;
    }

    if(workaround != WORKAROUND_NONE || flags != 0)
        _xcb_in_expect_reply(c, request, workaround, flags);
    if(!req->isvoid)
        c->in.request_expected = c->out.request;

    if(prefix[0] || prefix[2])
    {
        --vector, ++veclen;
        if(prefix[2])
        {
            prefix[1] = ((uint32_t *) vector[1].iov_base)[0];
            vector[1].iov_base = (uint32_t *) vector[1].iov_base + 1;
            vector[1].iov_len -= sizeof(uint32_t);
        }
        vector[0].iov_len = sizeof(uint32_t) * ((prefix[0] ? 1 : 0) + (prefix[2] ? 2 : 0));
        vector[0].iov_base = prefix + !prefix[0];
    }

    if(!write_block(c, vector, veclen))
    {
        _xcb_conn_shutdown(c);
        request = 0;
    }
    pthread_mutex_unlock(&c->iolock);
    return request;
}

int xcb_take_socket(xcb_connection_t *c, void (*return_socket)(void *closure), void *closure, int flags, uint64_t *sent)
{
    int ret;
    if(c->has_error)
        return 0;
    pthread_mutex_lock(&c->iolock);
    get_socket_back(c);
    ret = _xcb_out_flush_to(c, c->out.request);
    if(ret)
    {
        c->out.return_socket = return_socket;
        c->out.socket_closure = closure;
        if(flags)
            _xcb_in_expect_reply(c, c->out.request, WORKAROUND_EXTERNAL_SOCKET_OWNER, flags);
        assert(c->out.request == c->out.request_written);
        *sent = c->out.request;
    }
    pthread_mutex_unlock(&c->iolock);
    return ret;
}

int xcb_writev(xcb_connection_t *c, struct iovec *vector, int count, uint64_t requests)
{
    int ret;
    if(c->has_error)
        return 0;
    pthread_mutex_lock(&c->iolock);
    c->out.request += requests;
    ret = _xcb_out_send(c, vector, count);
    pthread_mutex_unlock(&c->iolock);
    return ret;
}

int xcb_flush(xcb_connection_t *c)
{
    int ret;
    if(c->has_error)
        return 0;
    pthread_mutex_lock(&c->iolock);
    ret = _xcb_out_flush_to(c, c->out.request);
    pthread_mutex_unlock(&c->iolock);
    return ret;
}

/* Private interface */

int _xcb_out_init(_xcb_out *out)
{
    if(pthread_cond_init(&out->socket_cond, 0))
        return 0;
    out->return_socket = 0;
    out->socket_closure = 0;
    out->socket_moving = 0;

    if(pthread_cond_init(&out->cond, 0))
        return 0;
    out->writing = 0;

    out->queue_len = 0;

    out->request = 0;
    out->request_written = 0;

    if(pthread_mutex_init(&out->reqlenlock, 0))
        return 0;
    out->maximum_request_length_tag = LAZY_NONE;

    return 1;
}

void _xcb_out_destroy(_xcb_out *out)
{
    pthread_cond_destroy(&out->cond);
    pthread_mutex_destroy(&out->reqlenlock);
}

int _xcb_out_send(xcb_connection_t *c, struct iovec *vector, int count)
{
    int ret = 1;
    while(ret && count)
        ret = _xcb_conn_wait(c, &c->out.cond, &vector, &count);
    c->out.request_written = c->out.request;
    pthread_cond_broadcast(&c->out.cond);
    _xcb_in_wake_up_next_reader(c);
    return ret;
}

int _xcb_out_flush_to(xcb_connection_t *c, uint64_t request)
{
    assert(XCB_SEQUENCE_COMPARE(request, <=, c->out.request));
    if(XCB_SEQUENCE_COMPARE(c->out.request_written, >=, request))
        return 1;
    if(c->out.queue_len)
    {
        struct iovec vec;
        vec.iov_base = c->out.queue;
        vec.iov_len = c->out.queue_len;
        c->out.queue_len = 0;
        return _xcb_out_send(c, &vec, 1);
    }
    while(c->out.writing)
        pthread_cond_wait(&c->out.cond, &c->iolock);
    assert(XCB_SEQUENCE_COMPARE(c->out.request_written, >=, request));
    return 1;
}
