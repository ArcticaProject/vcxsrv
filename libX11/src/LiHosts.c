/* $Xorg: LiHosts.c,v 1.4 2001/02/09 02:03:34 xorgcvs Exp $ */
/* $XdotOrg: lib/X11/src/LiHosts.c,v 1.4 2005-07-03 07:00:55 daniels Exp $ */
/*

Copyright 1986, 1998  The Open Group
Copyright 2004 Sun Microsystems, Inc.

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

X Window System is a trademark of The Open Group.

*/

/* This can really be considered an os dependent routine */

#define NEED_REPLIES
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
/*
 * can be freed using XFree.
 */

XHostAddress *XListHosts (
    register Display *dpy,
    int *nhosts,	/* RETURN */
    Bool *enabled)	/* RETURN */
{
    register XHostAddress *outbuf = NULL, *op;
    xListHostsReply reply;
    long nbytes;
    unsigned char *buf, *bp;
    register unsigned i;
    register xListHostsReq *req;
    XServerInterpretedAddress *sip;

    *nhosts = 0;
    LockDisplay(dpy);
    GetReq (ListHosts, req);

    if (!_XReply (dpy, (xReply *) &reply, 0, xFalse)) {
       UnlockDisplay(dpy);
       SyncHandle();
       return (XHostAddress *) NULL;
    }

    if (reply.nHosts) {
	nbytes = reply.length << 2;	/* compute number of bytes in reply */

	op = outbuf = (XHostAddress *)
	    Xmalloc((unsigned) (nbytes +
	      (reply.nHosts * sizeof(XHostAddress)) +
	      (reply.nHosts * sizeof(XServerInterpretedAddress))));

	if (! outbuf) {
	    _XEatData(dpy, (unsigned long) nbytes);
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (XHostAddress *) NULL;
	}
	sip = (XServerInterpretedAddress *)
	 (((unsigned char  *) outbuf) + (reply.nHosts * sizeof(XHostAddress)));
	bp = buf = ((unsigned char  *) sip)
	  + (reply.nHosts * sizeof(XServerInterpretedAddress));

	_XRead (dpy, (char *) buf, nbytes);

	for (i = 0; i < reply.nHosts; i++) {
#ifdef WORD64
	    xHostEntry xhe;
	    memcpy((char *)&xhe, bp, SIZEOF(xHostEntry));
	    op->family = xhe.family;
	    op->length = xhe.length;
#else
	    op->family = ((xHostEntry *) bp)->family;
	    op->length =((xHostEntry *) bp)->length;
#endif
	    if (op->family == FamilyServerInterpreted) {
		char *tp = (char *) (bp + SIZEOF(xHostEntry));
		char *vp = memchr(tp, 0, op->length);

		if (vp != NULL) {
		    sip->type = tp;
		    sip->typelength = vp - tp;
		    sip->value = vp + 1;
		    sip->valuelength = op->length - (sip->typelength + 1);
		} else {
		    sip->type = sip->value = NULL;
		    sip->typelength = sip->valuelength = 0;
		}
		op->address = (char *) sip;
		sip++;
	    } else {
		op->address = (char *) (bp + SIZEOF(xHostEntry));
	    }
	    bp += SIZEOF(xHostEntry) + (((op->length + 3) >> 2) << 2);
	    op++;
	}
    }

    *enabled = reply.enabled;
    *nhosts = reply.nHosts;
    UnlockDisplay(dpy);
    SyncHandle();
    return (outbuf);
}





