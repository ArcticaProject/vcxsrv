/* $Xorg: PutBEvent.c,v 1.4 2001/02/09 02:03:35 xorgcvs Exp $ */
/*

Copyright 1986, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/X11/PutBEvent.c,v 1.3 2001/01/17 19:41:41 dawes Exp $ */

/* XPutBackEvent puts an event back at the head of the queue. */
#define NEED_EVENTS
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"

int
_XPutBackEvent (
    register Display *dpy, 
    register XEvent *event)
	{
	register _XQEvent *qelt;

	if (!dpy->qfree) {
    	    if ((dpy->qfree = (_XQEvent *) Xmalloc (sizeof (_XQEvent))) == NULL) {
		return 0;
	    }
	    dpy->qfree->next = NULL;
	}
	qelt = dpy->qfree;
	dpy->qfree = qelt->next;
	qelt->qserial_num = dpy->next_event_serial_num++;
	qelt->next = dpy->head;
	qelt->event = *event;
	dpy->head = qelt;
	if (dpy->tail == NULL)
	    dpy->tail = qelt;
	dpy->qlen++;
	return 0;
	}

int
XPutBackEvent (
    register Display * dpy, 
    register XEvent *event)
	{
	int ret;

	LockDisplay(dpy);
	ret = _XPutBackEvent(dpy, event);
	UnlockDisplay(dpy);
	return ret;
	}
