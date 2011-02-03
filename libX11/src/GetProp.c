/*

Copyright 1986, 1998  The Open Group

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

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"

int
XGetWindowProperty(
    register Display *dpy,
    Window w,
    Atom property,
    long offset,
    long length,
    Bool delete,
    Atom req_type,
    Atom *actual_type,		/* RETURN */
    int *actual_format,  	/* RETURN  8, 16, or 32 */
    unsigned long *nitems, 	/* RETURN  # of 8-, 16-, or 32-bit entities */
    unsigned long *bytesafter,	/* RETURN */
    unsigned char **prop)	/* RETURN */
{
    xGetPropertyReply reply;
    register xGetPropertyReq *req;
    xError error = {0};

    LockDisplay(dpy);
    GetReq (GetProperty, req);
    req->window = w;
    req->property = property;
    req->type = req_type;
    req->delete = delete;
    req->longOffset = offset;
    req->longLength = length;
    error.sequenceNumber = dpy->request;

    if (!_XReply (dpy, (xReply *) &reply, 0, xFalse)) {
	UnlockDisplay(dpy);
	SyncHandle();
	return (1);	/* not Success */
	}

    *prop = (unsigned char *) NULL;
    if (reply.propertyType != None) {
	long nbytes, netbytes;
	switch (reply.format) {
      /*
       * One extra byte is malloced than is needed to contain the property
       * data, but this last byte is null terminated and convenient for
       * returning string properties, so the client doesn't then have to
       * recopy the string to make it null terminated.
       */
	  case 8:
	    nbytes = netbytes = reply.nItems;
	    if (nbytes + 1 > 0 &&
		(*prop = (unsigned char *) Xmalloc ((unsigned)nbytes + 1)))
		_XReadPad (dpy, (char *) *prop, netbytes);
	    break;

	  case 16:
	    nbytes = reply.nItems * sizeof (short);
	    netbytes = reply.nItems << 1;
	    if (nbytes + 1 > 0 &&
		(*prop = (unsigned char *) Xmalloc ((unsigned)nbytes + 1)))
		_XRead16Pad (dpy, (short *) *prop, netbytes);
	    break;

	  case 32:
	    nbytes = reply.nItems * sizeof (long);
	    netbytes = reply.nItems << 2;
	    if (nbytes + 1 > 0 &&
		(*prop = (unsigned char *) Xmalloc ((unsigned)nbytes + 1)))
		_XRead32 (dpy, (long *) *prop, netbytes);
	    break;

	  default:
	    /*
	     * This part of the code should never be reached.  If it is,
	     * the server sent back a property with an invalid format.
	     * This is a BadImplementation error.
	     */
	    {
		/* sequence number stored above */
		error.type = X_Error;
		error.majorCode = X_GetProperty;
		error.minorCode = 0;
		error.errorCode = BadImplementation;
		_XError(dpy, &error);
	    }
	    nbytes = netbytes = 0L;
	    break;
	}
	if (! *prop) {
	    _XEatData(dpy, (unsigned long) netbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return(BadAlloc);	/* not Success */
	}
	(*prop)[nbytes] = '\0';
    }
    *actual_type = reply.propertyType;
    *actual_format = reply.format;
    *nitems = reply.nItems;
    *bytesafter = reply.bytesAfter;
    UnlockDisplay(dpy);
    SyncHandle();
    return(Success);
}

