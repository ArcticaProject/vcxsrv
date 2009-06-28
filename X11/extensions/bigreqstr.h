/* $Xorg: bigreqstr.h,v 1.4 2001/02/09 02:03:24 xorgcvs Exp $ */
/*

Copyright 1992, 1998  The Open Group

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
/* $XFree86$ */

#ifndef _BIGREQSTR_H_
#define _BIGREQSTR_H_

#define X_BigReqEnable		0

#define XBigReqNumberEvents	0

#define XBigReqNumberErrors	0

#define XBigReqExtensionName	"BIG-REQUESTS"

typedef struct {
    CARD8	reqType;	/* always XBigReqCode */
    CARD8	brReqType;	/* always X_BigReqEnable */
    CARD16	length B16;
} xBigReqEnableReq;
#define sz_xBigReqEnableReq 4

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	pad0;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	max_request_size B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xBigReqEnableReply;
#define sz_xBigReqEnableReply 32


typedef struct {
	CARD8 reqType;
	CARD8 data;
	CARD16 zero B16;
        CARD32 length B32;
} xBigReq;

#endif /* _BIGREQSTR_H_ */
