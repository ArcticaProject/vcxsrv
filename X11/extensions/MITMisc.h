/************************************************************

Copyright 1989, 1998  The Open Group

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

********************************************************/

/* RANDOM CRUFT! THIS HAS NO OFFICIAL X CONSORTIUM OR X PROJECT TEAM BLESSING */

/* $Xorg: MITMisc.h,v 1.4 2001/02/09 02:03:23 xorgcvs Exp $ */

#ifndef _XMITMISC_H_
#define _XMITMISC_H_

#include <X11/Xfuncproto.h>

#define X_MITSetBugMode			0
#define X_MITGetBugMode			1

#define MITMiscNumberEvents		0

#define MITMiscNumberErrors		0

#ifndef _MITMISC_SERVER_

_XFUNCPROTOBEGIN

Bool XMITMiscQueryExtension(
    Display*		/* dpy */,
    int*		/* event_basep */,
    int*		/* error_basep */
);

Status XMITMiscSetBugMode(
    Display*		/* dpy */,
    Bool		/* onOff */
);

Bool XMITMiscGetBugMode(
    Display*		/* dpy */
);

_XFUNCPROTOEND

#endif

#endif
