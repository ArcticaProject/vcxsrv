/*
Copyright (c) 1992  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _SAVERPROTO_H_
#define _SAVERPROTO_H_

#include <X11/extensions/saver.h>

#define Window CARD32
#define Drawable CARD32
#define Font CARD32
#define Pixmap CARD32
#define Cursor CARD32
#define Colormap CARD32
#define GContext CARD32
#define Atom CARD32
#define VisualID CARD32
#define Time CARD32
#define KeyCode CARD8
#define KeySym CARD32

#define X_ScreenSaverQueryVersion   0

typedef struct _ScreenSaverQueryVersion {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverQueryVersion */
    CARD16 length B16;
    CARD8 clientMajor;
    CARD8 clientMinor;
    CARD16 unused B16;	
} xScreenSaverQueryVersionReq;
#define sz_xScreenSaverQueryVersionReq	8

typedef struct {
    CARD8 type;			/* X_Reply */
    CARD8 unused;			/* not used */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    CARD16 majorVersion B16;	/* major version of protocol */
    CARD16 minorVersion B16;	/* minor version of protocol */
    CARD32 pad0 B32;
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
} xScreenSaverQueryVersionReply;
#define sz_xScreenSaverQueryVersionReply	32

#define X_ScreenSaverQueryInfo   1

typedef struct _ScreenSaverQueryInfo {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverQueryInfo */
    CARD16 length B16;
    Drawable drawable B32;
} xScreenSaverQueryInfoReq;
#define sz_xScreenSaverQueryInfoReq	8

typedef struct {
    CARD8 type;			/* X_Reply */
    BYTE state;			/* Off, On */
    CARD16 sequenceNumber B16;
    CARD32 length B32;
    Window window B32;
    CARD32 tilOrSince B32;
    CARD32 idle B32;
    CARD32 eventMask B32;
    BYTE kind;			/* Blanked, Internal, External */
    CARD8 pad0;
    CARD16 pad1 B16;
    CARD32 pad2 B32;
} xScreenSaverQueryInfoReply;
#define sz_xScreenSaverQueryInfoReply	32

#define X_ScreenSaverSelectInput   2

typedef struct _ScreenSaverSelectInput {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverSelectInput */
    CARD16 length B16;
    Drawable drawable B32;
    CARD32 eventMask B32;
} xScreenSaverSelectInputReq;
#define sz_xScreenSaverSelectInputReq	12

#define X_ScreenSaverSetAttributes   3

typedef struct _ScreenSaverSetAttributes {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverSetAttributes */
    CARD16 length B16;
    Drawable drawable B32;
    INT16 x B16, y B16;
    CARD16 width B16, height B16, borderWidth B16;
    BYTE c_class;
    CARD8 depth;
    VisualID visualID B32;
    CARD32 mask B32;
} xScreenSaverSetAttributesReq;
#define sz_xScreenSaverSetAttributesReq	28

#define X_ScreenSaverUnsetAttributes   4

typedef struct _ScreenSaverUnsetAttributes {
    CARD8 reqType;		/* always ScreenSaverReqCode */
    CARD8 saverReqType;		/* always X_ScreenSaverUnsetAttributes */
    CARD16 length B16;
    Drawable drawable B32;
} xScreenSaverUnsetAttributesReq;
#define sz_xScreenSaverUnsetAttributesReq	8

#define X_ScreenSaverSuspend   5

typedef struct _ScreenSaverSuspend {
    CARD8 reqType;
    CARD8 saverReqType;
    CARD16 length B16;
    Bool suspend B32;
} xScreenSaverSuspendReq;
#define sz_xScreenSaverSuspendReq	8

typedef struct _ScreenSaverNotify {
    CARD8 type;			/* always eventBase + ScreenSaverNotify */
    BYTE state;			/* off, on, cycle */
    CARD16 sequenceNumber B16;
    Time timestamp B32;
    Window root B32;
    Window window B32;		/* screen saver window */
    BYTE kind;			/* blanked, internal, external */
    BYTE forced;
    CARD16 pad0 B16;
    CARD32 pad1 B32;
    CARD32 pad2 B32;
    CARD32 pad3 B32;
} xScreenSaverNotifyEvent;
#define sz_xScreenSaverNotifyEvent	32

#undef Window
#undef Drawable
#undef Font
#undef Pixmap
#undef Cursor
#undef Colormap
#undef GContext
#undef Atom
#undef VisualID
#undef Time
#undef KeyCode
#undef KeySym

#endif /* _SAVERPROTO_H_ */
