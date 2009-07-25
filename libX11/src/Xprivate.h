/* Copyright (C) 2008 Jamey Sharp, Josh Triplett
 * This file is licensed under the MIT license. See the file COPYING.
 *
 * As Xlibint.h has long become effectively public API, this header exists
 * for new private functions that nothing outside of libX11 should call.
 */

#ifndef XPRIVATE_H
#define XPRIVATE_H

extern int _XIDHandler(Display *dpy);
extern void _XSetPrivSyncFunction(Display *dpy);
extern void _XSetSeqSyncFunction(Display *dpy);

#endif /* XPRIVATE_H */
