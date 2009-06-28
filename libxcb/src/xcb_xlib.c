/* Copyright (C) 2005 Bart Massey and Jamey Sharp.
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

#include "xcbxlib.h"
#include "xcbint.h"

#include <assert.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#endif

static void xcb_xlib_printbt(void)
{
#ifdef HAVE_BACKTRACE
	void *array[20];
	int size;
	char **strings;
	int i;

	size = backtrace(array, 20);
	strings = backtrace_symbols(array, size);

	fprintf(stderr, "Locking assertion failure.  Backtrace:\n");

	for (i = 0; i < size; ++i)
		fprintf(stderr, "#%i %s\n", i, strings[i]);

	free(strings);
#endif
}

#ifndef NDEBUG
#define xcb_assert(c,x) do { if (!(x)) { xcb_xlib_printbt(); if (!(c)->xlib.sloppy_lock) assert(x); } } while(0)
#else
#define xcb_assert(c,x)
#endif

unsigned int xcb_get_request_sent(xcb_connection_t *c)
{
    if(c->has_error)
        return 0;
    return c->out.request;
}

void xcb_xlib_lock(xcb_connection_t *c)
{
    _xcb_lock_io(c);
    xcb_assert(c, !c->xlib.lock);
    c->xlib.lock = 1;
    c->xlib.thread = pthread_self();
    _xcb_unlock_io(c);
}

void xcb_xlib_unlock(xcb_connection_t *c)
{
    _xcb_lock_io(c);
    xcb_assert(c, c->xlib.lock);
    xcb_assert(c, pthread_equal(c->xlib.thread, pthread_self()));
    c->xlib.lock = 0;
    pthread_cond_broadcast(&c->xlib.cond);
    _xcb_unlock_io(c);
}
