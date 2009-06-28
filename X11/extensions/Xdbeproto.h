/* $Xorg: Xdbeproto.h,v 1.3 2000/08/18 04:05:45 coskrey Exp $ */
/******************************************************************************
 * 
 * Copyright (c) 1994, 1995  Hewlett-Packard Company
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Hewlett-Packard
 * Company shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the Hewlett-Packard Company.
 * 
 *     Header file for Xlib-related DBE
 *
 *****************************************************************************/


#ifndef XDBE_PROTO_H
#define XDBE_PROTO_H

/* INCLUDES */


/* DEFINES */

/* Values for swap_action field of XdbeSwapInfo structure */
#define XdbeUndefined    0
#define XdbeBackground   1
#define XdbeUntouched    2
#define XdbeCopied       3

#ifdef NEED_DBE_PROTOCOL

#define DBE_PROTOCOL_NAME "DOUBLE-BUFFER"

/* Current version numbers */
#define DBE_MAJOR_VERSION       1
#define DBE_MINOR_VERSION       0

/* Used when adding extension; also used in Xdbe macros */
#define DbeNumberEvents			0
#define DbeBadBuffer			0
#define DbeNumberErrors			(DbeBadBuffer + 1)

/* Request values used in (S)ProcDbeDispatch() */
#define X_DbeGetVersion                 0
#define X_DbeAllocateBackBufferName     1
#define X_DbeDeallocateBackBufferName   2
#define X_DbeSwapBuffers                3
#define X_DbeBeginIdiom                 4
#define X_DbeEndIdiom                   5
#define X_DbeGetVisualInfo              6
#define X_DbeGetBackBufferAttributes    7

typedef CARD8  xDbeSwapAction;
typedef CARD32 xDbeBackBuffer;

#endif /* NEED_DBE_PROTOCOL */


/* TYPEDEFS */

/* Client data types */

/* XdbeVisualInfo and XdbeScreenVisualInfo are defined in this file,
 * "Xdbeproto.h", rather than "Xdbe.h" because the server uses these data
 * types.
 */

typedef struct
{
    VisualID    visual;    /* one visual ID that supports double-buffering */
    int         depth;     /* depth of visual in bits                      */
    int         perflevel; /* performance level of visual                  */
}
XdbeVisualInfo;

typedef struct
{
    int                 count;          /* number of items in visual_depth   */
    XdbeVisualInfo      *visinfo;       /* list of visuals & depths for scrn */
}
XdbeScreenVisualInfo;

/* Protocol data types */

#ifdef NEED_DBE_PROTOCOL

typedef struct
{
    CARD32		window B32;	/* window      */
    xDbeSwapAction	swapAction;	/* swap action */
    CARD8		pad1;		/* unused      */
    CARD16		pad2 B16;

} xDbeSwapInfo;

typedef struct
{
    CARD32	visualID B32;	/* associated visual      */
    CARD8	depth;		/* depth of visual        */
    CARD8	perfLevel;	/* performance level hint */
    CARD16	pad1 B16;

} xDbeVisInfo;
#define sz_xDbeVisInfo	8

typedef struct
{
    CARD32	n B32;	/* number of visual info items in list  */

} xDbeScreenVisInfo;	/* followed by n xDbeVisInfo items */

typedef struct
{
    CARD32	window B32;	/* window */

} xDbeBufferAttributes;


/* Requests and replies */

typedef struct
{
    CARD8	reqType;	/* major-opcode: always codes->major_opcode */
    CARD8	dbeReqType;	/* minor-opcode: always X_DbeGetVersion (0) */
    CARD16	length B16;	/* request length: (2)                      */
    CARD8	majorVersion;	/* client-major-version                     */
    CARD8	minorVersion;	/* client-minor-version                     */
    CARD16	unused B16;	/* unused                                   */

} xDbeGetVersionReq;
#define sz_xDbeGetVersionReq	8

typedef struct
{
    BYTE	type;			/* Reply: X_Reply (1)   */
    CARD8	unused;			/* unused               */
    CARD16	sequenceNumber B16;	/* sequence number      */
    CARD32	length B32;		/* reply length: (0)    */
    CARD8	majorVersion;		/* server-major-version */
    CARD8	minorVersion;		/* server-minor-version */
    CARD16	pad1 B16;		/* unused               */
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;

} xDbeGetVersionReply;
#define sz_xDbeGetVersionReply	32

typedef struct
{
    CARD8		reqType;	/* major-opcode: codes->major_opcode */
    CARD8		dbeReqType;	/* X_DbeAllocateBackBufferName (1)   */
    CARD16		length B16;	/* request length: (4)               */
    CARD32		window B32;	/* window                            */
    xDbeBackBuffer	buffer B32;	/* back buffer name                  */
    xDbeSwapAction	swapAction;	/* swap action hint                  */
    CARD8		pad1;		/* unused                            */
    CARD16		pad2 B16;

} xDbeAllocateBackBufferNameReq;
#define sz_xDbeAllocateBackBufferNameReq	16

typedef struct
{
    CARD8		reqType;	/* major-opcode: codes->major_opcode */
    CARD8		dbeReqType;	/* X_DbeDeallocateBackBufferName (2) */
    CARD16		length B16;	/* request length: (2)               */
    xDbeBackBuffer	buffer B32;	/* back buffer name                  */

} xDbeDeallocateBackBufferNameReq;
#define sz_xDbeDeallocateBackBufferNameReq	8

typedef struct
{
    CARD8	reqType;	/* major-opcode: always codes->major_opcode  */
    CARD8	dbeReqType;	/* minor-opcode: always X_DbeSwapBuffers (3) */
    CARD16	length B16;	/* request length: (2+2n)                    */
    CARD32	n B32;		/* n, number of window/swap action pairs     */

} xDbeSwapBuffersReq;		/* followed by n window/swap action pairs    */
#define sz_xDbeSwapBuffersReq	8

typedef struct
{
    CARD8	reqType;	/* major-opcode: always codes->major_opcode */
    CARD8	dbeReqType;	/* minor-opcode: always X_DbeBeginIdom (4)  */
    CARD16	length B16;	/* request length: (1)                      */

} xDbeBeginIdiomReq;
#define sz_xDbeBeginIdiomReq	4

typedef struct
{
    CARD8	reqType;	/* major-opcode: always codes->major_opcode */
    CARD8	dbeReqType;	/* minor-opcode: always X_DbeEndIdom (5)    */
    CARD16	length B16;	/* request length: (1)                      */

} xDbeEndIdiomReq;
#define sz_xDbeEndIdiomReq	4

typedef struct
{
    CARD8	reqType;	/* always codes->major_opcode     */
    CARD8	dbeReqType;	/* always X_DbeGetVisualInfo (6)  */
    CARD16	length B16;	/* request length: (2+n)          */
    CARD32	n B32;		/* n, number of drawables in list */

} xDbeGetVisualInfoReq;		/* followed by n drawables        */
#define sz_xDbeGetVisualInfoReq	8

typedef struct
{
    BYTE	type;			/* Reply: X_Reply (1)                */
    CARD8	unused;			/* unused                            */
    CARD16	sequenceNumber B16;	/* sequence number                   */
    CARD32	length B32;		/* reply length                      */
    CARD32	m;			/* m, number of visual infos in list */
    CARD32	pad1 B32;		/* unused                            */
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;

} xDbeGetVisualInfoReply;		/* followed by m visual infos        */
#define sz_xDbeGetVisualInfoReply	32

typedef struct
{
    CARD8		reqType;	/* always codes->major_opcode       */
    CARD8		dbeReqType;	/* X_DbeGetBackBufferAttributes (7) */
    CARD16		length B16;	/* request length: (2)              */
    xDbeBackBuffer	buffer B32;	/* back buffer name                 */

} xDbeGetBackBufferAttributesReq;
#define sz_xDbeGetBackBufferAttributesReq	8

typedef struct
{
    BYTE	type;			/* Reply: X_Reply (1) */
    CARD8	unused;			/* unused             */
    CARD16	sequenceNumber B16;	/* sequence number    */
    CARD32	length B32;		/* reply length: (0)  */
    CARD32	attributes;		/* attributes         */
    CARD32	pad1 B32;		/* unused             */
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;

} xDbeGetBackBufferAttributesReply;
#define sz_xDbeGetBackBufferAttributesReply	32

#endif /* NEED_DBE_PROTOCOL */

#endif /* XDBE_PROTO_H */

