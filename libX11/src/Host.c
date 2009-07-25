/* $Xorg: Host.c,v 1.4 2001/02/09 02:03:33 xorgcvs Exp $ */
/* $XdotOrg: lib/X11/src/Host.c,v 1.4 2005-07-03 07:00:55 daniels Exp $ */
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
/* $XFree86: xc/lib/X11/Host.c,v 1.3 2001/01/17 19:41:37 dawes Exp $ */

/* this might be rightly regarded an os dependent file */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"

int
XAddHost (
    register Display *dpy,
    XHostAddress *host)
{
    register xChangeHostsReq *req;
    register int length;
    XServerInterpretedAddress *siAddr;
    int addrlen;

    siAddr = host->family == FamilyServerInterpreted ?
	(XServerInterpretedAddress *)host->address : NULL;
    addrlen = siAddr ?
	siAddr->typelength + siAddr->valuelength + 1 : host->length;

    length = (addrlen + 3) & ~0x3;	/* round up */

    LockDisplay(dpy);
    GetReqExtra (ChangeHosts, length, req);
    req->mode = HostInsert;
    req->hostFamily = host->family;
    req->hostLength = addrlen;
    if (siAddr) {
	char *dest = (char *) NEXTPTR(req,xChangeHostsReq);
	memcpy(dest, siAddr->type, siAddr->typelength);
	dest[siAddr->typelength] = '\0';
	memcpy(dest + siAddr->typelength + 1,siAddr->value,siAddr->valuelength);
    } else {
	memcpy((char *) NEXTPTR(req,xChangeHostsReq), host->address, addrlen);
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

int
XRemoveHost (
    register Display *dpy,
    XHostAddress *host)
{
    register xChangeHostsReq *req;
    register int length;
    XServerInterpretedAddress *siAddr;
    int addrlen;

    siAddr = host->family == FamilyServerInterpreted ?
	(XServerInterpretedAddress *)host->address : NULL;
    addrlen = siAddr ?
	siAddr->typelength + siAddr->valuelength + 1 : host->length;

    length = (addrlen + 3) & ~0x3;	/* round up */

    LockDisplay(dpy);
    GetReqExtra (ChangeHosts, length, req);
    req->mode = HostDelete;
    req->hostFamily = host->family;
    req->hostLength = addrlen;
    if (siAddr) {
	char *dest = (char *) NEXTPTR(req,xChangeHostsReq);
	memcpy(dest, siAddr->type, siAddr->typelength);
	dest[siAddr->typelength] = '\0';
	memcpy(dest + siAddr->typelength + 1,siAddr->value,siAddr->valuelength);
    } else {
	memcpy((char *) NEXTPTR(req,xChangeHostsReq), host->address, addrlen);
    }
    UnlockDisplay(dpy);
    SyncHandle();
    return 1;
}

int
XAddHosts (
    register Display *dpy,
    XHostAddress *hosts,
    int n)
{
    register int i;
    for (i = 0; i < n; i++) {
	(void) XAddHost(dpy, &hosts[i]);
      }
    return 1;
}

int
XRemoveHosts (
    register Display *dpy,
    XHostAddress *hosts,
    int n)
{
    register int i;
    for (i = 0; i < n; i++) {
	(void) XRemoveHost(dpy, &hosts[i]);
      }
    return 1;
}
