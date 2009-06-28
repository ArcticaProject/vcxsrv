/*
 * Copyright (C) 2001-2004 Bart Massey and Jamey Sharp.
 * All Rights Reserved.
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

#ifndef __XCBINT_H
#define __XCBINT_H

#include "bigreq.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef GCC_HAS_VISIBILITY
#pragma GCC visibility push(hidden)
#endif

enum workarounds {
    WORKAROUND_NONE,
    WORKAROUND_GLX_GET_FB_CONFIGS_BUG
};

enum lazy_reply_tag
{
    LAZY_NONE = 0,
    LAZY_COOKIE,
    LAZY_FORCED
};

#define XCB_PAD(i) (-(i) & 3)

#define XCB_SEQUENCE_COMPARE(a,op,b)	((int) ((a) - (b)) op 0)

/* xcb_list.c */

typedef void (*xcb_list_free_func_t)(void *);

typedef struct _xcb_map _xcb_map;

_xcb_map *_xcb_map_new(void);
void _xcb_map_delete(_xcb_map *q, xcb_list_free_func_t do_free);
int _xcb_map_put(_xcb_map *q, unsigned int key, void *data);
void *_xcb_map_remove(_xcb_map *q, unsigned int key);


/* xcb_out.c */

typedef struct _xcb_out {
    pthread_cond_t cond;
    int writing;

    char queue[4096];
    int queue_len;

    unsigned int request;
    unsigned int request_written;

    pthread_mutex_t reqlenlock;
    enum lazy_reply_tag maximum_request_length_tag;
    union {
        xcb_big_requests_enable_cookie_t cookie;
        uint32_t value;
    } maximum_request_length;
} _xcb_out;

int _xcb_out_init(_xcb_out *out);
void _xcb_out_destroy(_xcb_out *out);

int _xcb_out_send(xcb_connection_t *c, struct iovec **vector, int *count);
int _xcb_out_flush_to(xcb_connection_t *c, unsigned int request);


/* xcb_in.c */

typedef struct _xcb_in {
    pthread_cond_t event_cond;
    int reading;

    char queue[4096];
    int queue_len;

    unsigned int request_expected;
    unsigned int request_read;
    unsigned int request_completed;
    struct reply_list *current_reply;
    struct reply_list **current_reply_tail;

    _xcb_map *replies;
    struct event_list *events;
    struct event_list **events_tail;
    struct reader_list *readers;

    struct pending_reply *pending_replies;
    struct pending_reply **pending_replies_tail;
} _xcb_in;

int _xcb_in_init(_xcb_in *in);
void _xcb_in_destroy(_xcb_in *in);

int _xcb_in_expect_reply(xcb_connection_t *c, unsigned int request, enum workarounds workaround, int flags);

int _xcb_in_read(xcb_connection_t *c);
int _xcb_in_read_block(xcb_connection_t *c, void *buf, int nread);


/* xcb_xlib.c */

typedef struct _xcb_xlib {
    int lock;
    int sloppy_lock;
    pthread_t thread;
    pthread_cond_t cond;
} _xcb_xlib;


/* xcb_xid.c */

typedef struct _xcb_xid {
    pthread_mutex_t lock;
    uint32_t last;
    uint32_t base;
    uint32_t max;
    uint32_t inc;
} _xcb_xid;

int _xcb_xid_init(xcb_connection_t *c);
void _xcb_xid_destroy(xcb_connection_t *c);


/* xcb_ext.c */

typedef struct _xcb_ext {
    pthread_mutex_t lock;
    struct lazyreply *extensions;
    int extensions_size;
} _xcb_ext;

int _xcb_ext_init(xcb_connection_t *c);
void _xcb_ext_destroy(xcb_connection_t *c);


/* xcb_conn.c */

struct xcb_connection_t {
    int has_error;

    /* constant data */
    xcb_setup_t *setup;
    int fd;

    /* I/O data */
    pthread_mutex_t iolock;
    _xcb_xlib xlib;
    _xcb_in in;
    _xcb_out out;

    /* misc data */
    _xcb_ext ext;
    _xcb_xid xid;
};

void _xcb_conn_shutdown(xcb_connection_t *c);
void _xcb_wait_io(xcb_connection_t *c, pthread_cond_t *cond);
int _xcb_conn_wait(xcb_connection_t *c, pthread_cond_t *cond, struct iovec **vector, int *count);


/* xcb_auth.c */

int _xcb_get_auth_info(int fd, xcb_auth_info_t *info, int display);

#ifdef GCC_HAS_VISIBILITY
#pragma GCC visibility pop
#endif


/* xcb_conn.c symbols visible to xcb-xlib */

void _xcb_lock_io(xcb_connection_t *c);
void _xcb_unlock_io(xcb_connection_t *c);

#endif
