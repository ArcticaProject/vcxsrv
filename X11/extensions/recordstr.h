/*
#ifndef lint
$Xorg: recordstr.h,v 1.3 2000/08/18 04:05:46 coskrey Exp $
static char sccsid[ ] = "@(#) recordstr.h 1.5 6/5/95 12:37:44";
#endif
*/

/***************************************************************************
 * Copyright 1995 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Network Computing Devices
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES DISCLAIMs ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************/
/* $XFree86$ */

#ifndef _RECORDSTR_H_
#define _RECORDSTR_H_

#include <X11/extensions/record.h>

#define RECORD_NAME			"RECORD"
#define RECORD_MAJOR_VERSION		1
#define RECORD_MINOR_VERSION		13
#define RECORD_LOWEST_MAJOR_VERSION	1
#define RECORD_LOWEST_MINOR_VERSION	12
/* only difference between 1.12 and 1.13 is byte order of device events,
   which the library doesn't deal with. */

/*********************************************************
 *
 * Protocol request constants
 *
 */
#define X_RecordQueryVersion    0     /* First request from client */
#define X_RecordCreateContext   1     /* Create client RC */
#define X_RecordRegisterClients 2     /* Add to client RC */
#define X_RecordUnregisterClients 3   /* Delete from client RC */
#define X_RecordGetContext      4     /* Query client RC */
#define X_RecordEnableContext   5     /* Enable interception and reporting */
#define X_RecordDisableContext  6     /* Disable interception and reporting */
#define X_RecordFreeContext     7     /* Free client RC */

#define RecordNumErrors         (XRecordBadContext + 1) 
#define RecordNumEvents      	0L

#define sz_XRecordRange		32
#define sz_XRecordClientInfo 	12
#define sz_XRecordState 	16
#define sz_XRecordDatum 	32


#define XRecordGlobaldef
#define XRecordGlobalref extern

#define RecordMaxEvent     	(128L-1L)
#define RecordMinDeviceEvent	(2L)
#define RecordMaxDeviceEvent	(6L)
#define RecordMaxError          (256L-1L)
#define RecordMaxCoreRequest    (128L-1L)
#define RecordMaxExtRequest     (256L-1L)
#define RecordMinExtRequest     (129L-1L)

#define RECORD_RC 		CARD32
#define RECORD_XIDBASE		CARD32
#define RECORD_CLIENTSPEC	CARD32
#define RECORD_ELEMENT_HEADER	CARD8

typedef RECORD_CLIENTSPEC RecordClientSpec, *RecordClientSpecPtr;

typedef struct
{
    CARD8	first;
    CARD8	last;
} RECORD_RANGE8;

typedef struct
{
    CARD16	first B16;
    CARD16	last B16;
} RECORD_RANGE16;

typedef struct
{
    RECORD_RANGE8	majorCode;
    RECORD_RANGE16	minorCode;
} RECORD_EXTRANGE;

typedef struct
{
    RECORD_RANGE8	coreRequests;
    RECORD_RANGE8	coreReplies;
    RECORD_EXTRANGE	extRequests;
    RECORD_EXTRANGE	extReplies;
    RECORD_RANGE8	deliveredEvents;
    RECORD_RANGE8	deviceEvents;
    RECORD_RANGE8	errors;
    BOOL		clientStarted;
    BOOL		clientDied;
} RECORDRANGE;
#define sz_RECORDRANGE 	24

/* typedef RECORDRANGE xRecordRange, *xRecordRangePtr;
#define sz_xRecordRange 24 */

/* Cannot have structures within structures going over the wire */
typedef struct
{
    CARD8       	coreRequestsFirst;
    CARD8       	coreRequestsLast;
    CARD8       	coreRepliesFirst;
    CARD8       	coreRepliesLast;
    CARD8  		extRequestsMajorFirst;
    CARD8		extRequestsMajorLast;
    CARD16  		extRequestsMinorFirst B16;
    CARD16  		extRequestsMinorLast B16;
    CARD8  		extRepliesMajorFirst;
    CARD8		extRepliesMajorLast;
    CARD16  		extRepliesMinorFirst B16;
    CARD16  		extRepliesMinorLast B16;
    CARD8       	deliveredEventsFirst;
    CARD8       	deliveredEventsLast;
    CARD8		deviceEventsFirst;
    CARD8		deviceEventsLast;
    CARD8       	errorsFirst;
    CARD8       	errorsLast;
    BOOL                clientStarted;
    BOOL		clientDied;
} xRecordRange;
#define sz_xRecordRange 24

typedef struct
{
    RECORD_CLIENTSPEC	clientResource B32;
    CARD32		nRanges B32;
/* LISTofRECORDRANGE */
} RECORD_CLIENT_INFO;

typedef RECORD_CLIENT_INFO xRecordClientInfo;

/*
 * Initialize
 */
typedef struct {
    CARD8       reqType;
    CARD8       recordReqType;
    CARD16      length B16;
    CARD16      majorVersion B16;
    CARD16      minorVersion B16;
} xRecordQueryVersionReq;
#define sz_xRecordQueryVersionReq 	8

typedef struct
{
    CARD8   type;
    CARD8   pad0;
    CARD16  sequenceNumber B16;
    CARD32  length	 B32;
    CARD16  majorVersion B16;
    CARD16  minorVersion B16;
    CARD32  pad1	 B32;
    CARD32  pad2	 B32;
    CARD32  pad3	 B32;
    CARD32  pad4	 B32;
    CARD32  pad5	 B32;
 } xRecordQueryVersionReply;
#define sz_xRecordQueryVersionReply  	32

/*
 * Create RC
 */
typedef struct
{
    CARD8     		reqType;
    CARD8     		recordReqType;
    CARD16    		length B16;
    RECORD_RC		context B32;
    RECORD_ELEMENT_HEADER elementHeader;
    CARD8		pad;
    CARD16		pad0 B16;
    CARD32		nClients B32;
    CARD32              nRanges B32;
/* LISTofRECORD_CLIENTSPEC */
/* LISTofRECORDRANGE */
} xRecordCreateContextReq;
#define sz_xRecordCreateContextReq 	20

/*
 * Add to  RC
 */
typedef struct
{
    CARD8     		reqType;
    CARD8     		recordReqType;
    CARD16    		length B16;
    RECORD_RC		context B32;
    RECORD_ELEMENT_HEADER elementHeader;
    CARD8		pad;
    CARD16		pad0 B16;
    CARD32		nClients B32;
    CARD32              nRanges B32;
/* LISTofRECORD_CLIENTSPEC */
/* LISTofRECORDRANGE */
} xRecordRegisterClientsReq;
#define sz_xRecordRegisterClientsReq 	20

/*
 * Delete from RC
 */
typedef struct
{
    CARD8     		reqType;
    CARD8     		recordReqType;
    CARD16    		length B16;
    RECORD_RC		context B32;
    CARD32		nClients B32;
/* LISTofRECORD_CLIENTSPEC */
} xRecordUnregisterClientsReq;
#define sz_xRecordUnregisterClientsReq 	12

/*
 * Query RC
 */
typedef struct
{
    CARD8     	reqType;
    CARD8     	recordReqType;
    CARD16    	length B16;
    RECORD_RC	context B32;
} xRecordGetContextReq;
#define sz_xRecordGetContextReq 		8

typedef struct
{
    CARD8   	type;
    BOOL    	enabled;
    CARD16  	sequenceNumber B16;
    CARD32  	length	 B32;
    RECORD_ELEMENT_HEADER  elementHeader;
    CARD8	pad;
    CARD16	pad0 B16;
    CARD32  	nClients B32;
    CARD32  	pad1 B32;
    CARD32  	pad2 B32;
    CARD32  	pad3 B32;
    CARD32  	pad4 B32;
/* LISTofCLIENT_INFO */ 		/* intercepted-clients */
} xRecordGetContextReply;
#define sz_xRecordGetContextReply  	32

/*
 * Enable data interception
 */
typedef struct
{
    CARD8     	reqType;
    CARD8     	recordReqType;
    CARD16    	length B16;
    RECORD_RC	context B32;
} xRecordEnableContextReq;
#define sz_xRecordEnableContextReq 	8

typedef struct
{
    CARD8		type;
    CARD8		category;
    CARD16		sequenceNumber B16;
    CARD32		length B32;
    RECORD_ELEMENT_HEADER  elementHeader;
    BOOL		clientSwapped;
    CARD16		pad1 B16;
    RECORD_XIDBASE 	idBase B32;
    CARD32		serverTime B32;
    CARD32		recordedSequenceNumber B32;
    CARD32		pad3 B32;
    CARD32		pad4 B32;
    /* BYTE		data; */
} xRecordEnableContextReply;
#define sz_xRecordEnableContextReply 	32

/*
 * Disable data interception
 */
typedef struct
{
    CARD8     	reqType;
    CARD8     	recordReqType;
    CARD16    	length B16;
    RECORD_RC 	context B32;
} xRecordDisableContextReq;
#define sz_xRecordDisableContextReq	8

/*
 * Free RC
 */
typedef struct
{
    CARD8     	reqType;
    CARD8     	recordReqType;
    CARD16    	length B16;
    RECORD_RC 	context B32;
} xRecordFreeContextReq;
#define sz_xRecordFreeContextReq 	8

#undef RECORD_RC
#undef RECORD_XIDBASE
#undef RECORD_ELEMENT_HEADER
#undef RECORD_CLIENTSPEC

#endif
