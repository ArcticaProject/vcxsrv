/* $Xorg: lbxstr.h,v 1.4 2001/02/13 20:14:04 pookie Exp $ */
/*
 * Copyright 1992 Network Computing Devices
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of NCD. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  NCD. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NCD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NCD.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XFree86: xc/include/extensions/lbxstr.h,v 1.2 2001/08/01 00:44:35 tsi Exp $ */
 
#ifndef _LBXSTR_H_
#define _LBXSTR_H_

#include <X11/extensions/XLbx.h>

#define LBXNAME "LBX"

#define LBX_MAJOR_VERSION	1
#define LBX_MINOR_VERSION	0

/*
 * Redefine some basic types used by structures defined herein.  This removes
 * any possibility on 64-bit architectures of one entity viewing communicated
 * data as 32-bit quantities and another entity viewing the same data as 64-bit
 * quantities.
 */
#define XID CARD32
#define Atom CARD32
#define Colormap CARD32
#define Drawable CARD32
#define VisualID CARD32
#define Window CARD32

typedef struct {
    BOOL	success;		/* TRUE */
    BOOL	changeType;
    CARD16	majorVersion B16,
		minorVersion B16;
    CARD16	length B16;		/* 1/4 additional bytes in setup info */
    CARD32	tag B32;
} xLbxConnSetupPrefix;

typedef struct _LbxQueryVersion {
    CARD8	reqType;		/* always LbxReqCode */
    CARD8	lbxReqType;		/* always X_LbxQueryVersion */
    CARD16	length B16;
} xLbxQueryVersionReq;
#define sz_xLbxQueryVersionReq	4

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	unused;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD16	majorVersion B16;	/* major version of LBX protocol */
    CARD16	minorVersion B16;	/* minor version of LBX protocol */
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xLbxQueryVersionReply;
#define sz_xLbxQueryVersionReply	32

typedef struct _LbxStartProxy {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxStartProxy */
    CARD16	length B16;
} xLbxStartProxyReq;
#define sz_xLbxStartProxyReq	    4

typedef struct _LbxStopProxy {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxStopProxy */
    CARD16	length B16;
} xLbxStopProxyReq;
#define sz_xLbxStopProxyReq	    4

typedef struct _LbxSwitch {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxSwitch */
    CARD16	length B16;
    CARD32	client B32;	/* new client */
} xLbxSwitchReq;
#define sz_xLbxSwitchReq	8

typedef struct _LbxNewClient {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxNewClient */
    CARD16	length B16;
    CARD32	client B32;	/* new client */
} xLbxNewClientReq;
#define sz_xLbxNewClientReq	8

typedef struct _LbxCloseClient {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxCloseClient */
    CARD16	length B16;
    CARD32	client B32;	/* new client */
} xLbxCloseClientReq;
#define sz_xLbxCloseClientReq	8

typedef struct _LbxModifySequence {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxModifySequence */
    CARD16	length B16;
    CARD32	adjust B32;
} xLbxModifySequenceReq;
#define sz_xLbxModifySequenceReq    8
    
typedef struct _LbxAllowMotion {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxAllowMotion */
    CARD16	length B16;
    CARD32	num B32;
} xLbxAllowMotionReq;
#define sz_xLbxAllowMotionReq    8
    
typedef struct {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxGrabCmap */
    CARD16	length B16;
    Colormap	cmap B32;
} xLbxGrabCmapReq;    
#define sz_xLbxGrabCmapReq	8

#define LBX_SMART_GRAB		0x80
#define LBX_AUTO_RELEASE	0x40
#define LBX_3CHANNELS		0x20
#define LBX_2BYTE_PIXELS	0x10
#define LBX_RGB_BITS_MASK	0x0f

#define LBX_LIST_END		0
#define LBX_PIXEL_PRIVATE	1
#define LBX_PIXEL_SHARED	2
#define LBX_PIXEL_RANGE_PRIVATE	3
#define LBX_PIXEL_RANGE_SHARED	4
#define LBX_NEXT_CHANNEL	5

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	flags;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	pad0 B16;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B16;
} xLbxGrabCmapReply;
#define sz_xLbxGrabCmapReply	32
#define sz_xLbxGrabCmapReplyHdr	8


typedef struct {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxReleaseCmap */
    CARD16	length B16;
    Colormap	cmap B32;
} xLbxReleaseCmapReq;    
#define sz_xLbxReleaseCmapReq	8

typedef struct {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxAllocColor */
    CARD16	length B16;
    Colormap	cmap B32;
    CARD32	pixel B32;
    CARD16	red B16, green B16, blue B16;
    CARD16	pad B16;
} xLbxAllocColorReq;    
#define sz_xLbxAllocColorReq	20

typedef struct _LbxIncrementPixel {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxIncrementPixel */
    CARD16	length B16;
    CARD32	cmap B32;
    CARD32	pixel B32;
} xLbxIncrementPixelReq;
#define sz_xLbxIncrementPixelReq    12

typedef struct _LbxDelta {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxDelta */
    CARD16	length B16;
    CARD8	diffs;		/* number of diffs */
    CARD8	cindex;		/* cache index */
				/* list of diffs follows */
} xLbxDeltaReq;
#define sz_xLbxDeltaReq    6

typedef struct _LbxGetModifierMapping {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxGetModifierMapping */
    CARD16	length B16;
} xLbxGetModifierMappingReq;
#define	sz_xLbxGetModifierMappingReq	4

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	keyspermod;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	tag B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xLbxGetModifierMappingReply;
#define sz_xLbxGetModifierMappingReply	32

typedef struct _LbxGetKeyboardMapping {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxGetKeyboardMapping */
    CARD16	length B16;
    KeyCode	firstKeyCode;
    CARD8	count;
    CARD16	pad1 B16;
} xLbxGetKeyboardMappingReq;
#define	sz_xLbxGetKeyboardMappingReq	8

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	keysperkeycode;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	tag B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xLbxGetKeyboardMappingReply;
#define sz_xLbxGetKeyboardMappingReply	32

typedef struct _LbxQueryFont {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxQueryFont */
    CARD16	length B16;
    CARD32	fid B32;
} xLbxQueryFontReq;
#define	sz_xLbxQueryFontReq	8

typedef struct _LbxInternAtoms {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxInternAtoms */
    CARD16	length B16;
    CARD16	num B16;
} xLbxInternAtomsReq;
#define sz_xLbxInternAtomsReq	6  

typedef struct {
    BYTE	type;		/* X_Reply */
    CARD8	unused;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	atomsStart B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xLbxInternAtomsReply;
#define sz_xLbxInternAtomsReply		32
#define sz_xLbxInternAtomsReplyHdr	8


typedef struct _LbxGetWinAttrAndGeom {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxGetWinAttrAndGeom */
    CARD16	length B16;
    CARD32	id B32;		/* window id */
} xLbxGetWinAttrAndGeomReq;
#define sz_xLbxGetWinAttrAndGeomReq 8

typedef struct {
    BYTE type;  /* X_Reply */
    CARD8 backingStore;
    CARD16 sequenceNumber B16;
    CARD32 length B32;	/* NOT 0; this is an extra-large reply */
    VisualID visualID B32;
#if defined(__cplusplus) || defined(c_plusplus)
    CARD16 c_class B16;
#else
    CARD16 class B16;
#endif
    CARD8 bitGravity;
    CARD8 winGravity;
    CARD32 backingBitPlanes B32;
    CARD32 backingPixel B32;
    BOOL saveUnder;
    BOOL mapInstalled;
    CARD8 mapState;
    BOOL override;
    Colormap colormap B32;
    CARD32 allEventMasks B32;
    CARD32 yourEventMask B32;
    CARD16 doNotPropagateMask B16;
    CARD16 pad1 B16;
    Window root B32;
    INT16 x B16, y B16;
    CARD16 width B16, height B16;
    CARD16 borderWidth B16;
    CARD8 depth;
    CARD8 pad2;
} xLbxGetWinAttrAndGeomReply;
#define sz_xLbxGetWinAttrAndGeomReply 60


typedef struct {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxSync */
    CARD16	length B16;
} xLbxSyncReq;
#define sz_xLbxSyncReq	4

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	pad0;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xLbxSyncReply;
#define sz_xLbxSyncReply 32


/* an LBX squished charinfo packs the data in a CARD32 as follows */
#define	LBX_WIDTH_SHIFT		26
#define	LBX_LEFT_SHIFT		20
#define	LBX_RIGHT_SHIFT		13
#define	LBX_ASCENT_SHIFT	7
#define	LBX_DESCENT_SHIFT	0

#define	LBX_WIDTH_BITS		6
#define	LBX_LEFT_BITS		6
#define	LBX_RIGHT_BITS		7
#define	LBX_ASCENT_BITS		6
#define	LBX_DESCENT_BITS	7

#define	LBX_WIDTH_MASK		0xfc000000
#define	LBX_LEFT_MASK		0x03f00000
#define	LBX_RIGHT_MASK		0x000fe000
#define	LBX_ASCENT_MASK		0x00001f80
#define	LBX_DESCENT_MASK	0x0000007f

#define	LBX_MASK_BITS(val, n)	((unsigned int) ((val) & ((1 << (n)) - 1)))

typedef struct {
    CARD32	metrics B32;
} xLbxCharInfo;

/* note that this is identical to xQueryFontReply except for missing 
 * first 2 words
 */
typedef struct {
    xCharInfo minBounds; 
/* XXX do we need to leave this gunk? */
#ifndef WORD64
    CARD32 walign1 B32;
#endif
    xCharInfo maxBounds; 
#ifndef WORD64
    CARD32 walign2 B32;
#endif
    CARD16 minCharOrByte2 B16, maxCharOrByte2 B16;
    CARD16 defaultChar B16;
    CARD16 nFontProps B16;  /* followed by this many xFontProp structures */
    CARD8 drawDirection;
    CARD8 minByte1, maxByte1;
    BOOL allCharsExist;
    INT16 fontAscent B16, fontDescent B16;
    CARD32 nCharInfos B32; /* followed by this many xLbxCharInfo structures */
} xLbxFontInfo;

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	compression;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	tag B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    /* X_QueryFont sticks much of the data in the base reply packet,
     * but we hope that it won't be needed, (and it won't fit in 32 bytes
     * with the tag anyways)
     *
     * if any additional data is needed, its sent in a xLbxFontInfo
     */
} xLbxQueryFontReply;
#define sz_xLbxQueryFontReply	32

typedef struct _LbxChangeProperty {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxChangeProperty */
    CARD16	length B16;
    Window	window B32;
    Atom	property B32;
    Atom	type B32;
    CARD8	format;
    CARD8	mode;
    BYTE	pad[2];
    CARD32	nUnits B32;
} xLbxChangePropertyReq;
#define	sz_xLbxChangePropertyReq	24

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	pad;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	tag B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xLbxChangePropertyReply;
#define sz_xLbxChangePropertyReply	32

typedef struct _LbxGetProperty {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxGetProperty */
    CARD16	length B16;
    Window	window B32;
    Atom	property B32;
    Atom	type B32;
    CARD8	delete;
    BYTE	pad[3];
    CARD32	longOffset B32;
    CARD32	longLength B32;
} xLbxGetPropertyReq;
#define	sz_xLbxGetPropertyReq	28

typedef struct {
    BYTE	type;			/* X_Reply */
    CARD8	format;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    Atom	propertyType B32;
    CARD32	bytesAfter B32;
    CARD32	nItems B32;
    CARD32	tag B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
} xLbxGetPropertyReply;
#define sz_xLbxGetPropertyReply	32

typedef struct _LbxTagData {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxTagData */
    CARD16	length B16;
    XID		tag B32;
    CARD32	real_length B32;
    /* data */
} xLbxTagDataReq;
#define	sz_xLbxTagDataReq	12

typedef struct _LbxInvalidateTag {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxInvalidateTag */
    CARD16	length B16;
    CARD32	tag B32;
} xLbxInvalidateTagReq;
#define	sz_xLbxInvalidateTagReq	8

typedef struct _LbxPutImage {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxPutImage */
    CARD16	length B16;
    CARD8	compressionMethod;
    CARD8	cacheEnts;
    CARD8	bitPacked;
    /* rest is variable */
} xLbxPutImageReq;
#define sz_xLbxPutImageReq	7

typedef struct {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxGetImage */
    CARD16	length B16;
    Drawable	drawable B32;
    INT16	x B16, y B16;
    CARD16	width B16, height B16;
    CARD32	planeMask B32;
    CARD8	format;
    CARD8	pad1;
    CARD16	pad2 B16;
} xLbxGetImageReq;    

#define sz_xLbxGetImageReq 24

typedef struct {
    BYTE type;			/* X_Reply */
    CARD8 depth;
    CARD16 sequenceNumber B16;
    CARD32 lbxLength B32;
    CARD32 xLength B32;
    VisualID visual B32;
    CARD8 compressionMethod;
    CARD8 pad1;
    CARD16 pad2 B16;
    CARD32 pad3 B32;
    CARD32 pad4 B32;
    CARD32 pad5 B32;
} xLbxGetImageReply;

#define sz_xLbxGetImageReply 32
  
/* Following used for LbxPolyPoint, LbxPolyLine, LbxPolySegment,
   LbxPolyRectangle, LbxPolyArc, LbxPolyFillRectangle and LbxPolyFillArc */

#define GFX_CACHE_SIZE  15

#define GFXdCacheEnt(e)	    ((e) & 0xf)
#define GFXgCacheEnt(e)	    (((e) >> 4) & 0xf)
#define GFXCacheEnts(d,g)   (((d) & 0xf) | (((g) & 0xf) << 4))

#define GFXCacheNone   0xf

typedef struct _LbxPolyPoint {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;
    CARD16	length B16;
    CARD8	cacheEnts;
    CARD8	padBytes;
} xLbxPolyPointReq;

#define sz_xLbxPolyPointReq	6

typedef xLbxPolyPointReq xLbxPolyLineReq;
typedef xLbxPolyPointReq xLbxPolySegmentReq;
typedef xLbxPolyPointReq xLbxPolyRectangleReq;
typedef xLbxPolyPointReq xLbxPolyArcReq;
typedef xLbxPolyPointReq xLbxPolyFillRectangleReq;
typedef xLbxPolyPointReq xLbxPolyFillArcReq;

#define sz_xLbxPolyLineReq		sz_xLbxPolyPointReq
#define sz_xLbxPolySegmentReq		sz_xLbxPolyPointReq
#define sz_xLbxPolyRectangleReq		sz_xLbxPolyPointReq
#define sz_xLbxPolyArcReq		sz_xLbxPolyPointReq
#define sz_xLbxPolyFillRectangleReq	sz_xLbxPolyPointReq
#define sz_xLbxPolyFillArc		sz_xLbxPolyPointReq

typedef struct _LbxFillPoly {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;
    CARD16	length B16;
    CARD8	cacheEnts;
    BYTE	shape;
    CARD8	padBytes;
} xLbxFillPolyReq;
#define sz_xLbxFillPolyReq	7

typedef struct _LbxCopyArea {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;
    CARD16	length B16;
    CARD8	srcCache;	/* source drawable */
    CARD8	cacheEnts;	/* dest drawable and gc */
    /* followed by encoded src x, src y, dst x, dst y, width, height */
} xLbxCopyAreaReq;
    
#define sz_xLbxCopyAreaReq  6

typedef struct _LbxCopyPlane {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;
    CARD16	length B16;
    CARD32	bitPlane B32;
    CARD8	srcCache;	/* source drawable */
    CARD8	cacheEnts;	/* dest drawable and gc */
    /* followed by encoded src x, src y, dst x, dst y, width, height */
} xLbxCopyPlaneReq;
    
#define sz_xLbxCopyPlaneReq  10

typedef struct _LbxPolyText {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;
    CARD16	length B16;
    CARD8	cacheEnts;
    /* followed by encoded src x, src y coordinates and text elts */
} xLbxPolyTextReq;

#define sz_xLbxPolyTextReq  5

typedef xLbxPolyTextReq xLbxPolyText8Req;
typedef xLbxPolyTextReq xLbxPolyText16Req;
    
#define sz_xLbxPolyTextReq	5
#define sz_xLbxPolyText8Req	5
#define sz_xLbxPolyText16Req	5

typedef struct _LbxImageText {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;
    CARD16	length B16;
    CARD8	cacheEnts;
    CARD8	nChars;
    /* followed by encoded src x, src y coordinates and string */
} xLbxImageTextReq;
    
typedef xLbxImageTextReq xLbxImageText8Req;
typedef xLbxImageTextReq xLbxImageText16Req;
    
#define sz_xLbxImageTextReq	6
#define sz_xLbxImageText8Req	6
#define sz_xLbxImageText16Req	6

typedef struct {
    CARD8       offset;
    CARD8       diff;
} xLbxDiffItem;
#define sz_xLbxDiffItem    2

typedef struct {
    BYTE	type;		/* X_Reply */
    CARD8	nOpts;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    CARD32	optDataStart B32;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xLbxStartReply;
#define sz_xLbxStartReply	32
#define sz_xLbxStartReplyHdr	8

typedef struct _LbxQueryExtension {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxQueryExtension */
    CARD16	length B16;
    CARD32	nbytes B32;
} xLbxQueryExtensionReq;
#define	sz_xLbxQueryExtensionReq	8

typedef struct _LbxQueryExtensionReply {
    BYTE	type;			/* X_Reply */
    CARD8	numReqs;
    CARD16	sequenceNumber B16;
    CARD32	length B32;
    BOOL	present;
    CARD8	major_opcode;
    CARD8	first_event;
    CARD8	first_error;
    CARD32	pad0 B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;

    /* reply & event generating requests */
} xLbxQueryExtensionReply;
#define sz_xLbxQueryExtensionReply	32


typedef struct _LbxBeginLargeRequest {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxBeginLargeRequest */
    CARD16	length B16;
    CARD32	largeReqLength B32;
} xLbxBeginLargeRequestReq;
#define	sz_BeginLargeRequestReq 8

typedef struct _LbxLargeRequestData {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxLargeRequestData */
    CARD16	length B16;
    /* followed by LISTofCARD8 data */
} xLbxLargeRequestDataReq;
#define	sz_LargeRequestDataReq 4

typedef struct _LbxEndLargeRequest {
    CARD8	reqType;	/* always LbxReqCode */
    CARD8	lbxReqType;	/* always X_LbxEndLargeRequest */
    CARD16	length B16;
} xLbxEndLargeRequestReq;
#define	sz_EndLargeRequestReq 4



typedef struct _LbxSwitchEvent {
    BYTE	type;		/* always eventBase + LbxEvent */
    BYTE	lbxType;	/* LbxSwitchEvent */
    CARD16	pad B16;
    CARD32	client B32;
} xLbxSwitchEvent;
#define sz_xLbxSwitchEvent	8

typedef struct _LbxCloseEvent {
    BYTE	type;		/* always eventBase + LbxEvent */
    BYTE	lbxType;	/* LbxCloseEvent */
    CARD16	sequenceNumber B16;
    CARD32	client B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xLbxCloseEvent;
#define sz_xLbxCloseEvent	32

typedef struct _LbxInvalidateTagEvent {
    BYTE	type;		/* always eventBase + LbxEvent */
    BYTE	lbxType;	/* LbxInvalidateTagEvent */
    CARD16	sequenceNumber B16;
    CARD32	tag B32;
    CARD32	tagType B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xLbxInvalidateTagEvent;
#define sz_xLbxInvalidateTagEvent 32

typedef struct _LbxSendTagDataEvent {
    BYTE	type;		/* always eventBase + LbxEvent */
    BYTE	lbxType;	/* LbxSendTagDataEvent */
    CARD16	sequenceNumber B16;
    CARD32	tag B32;
    CARD32	tagType B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
} xLbxSendTagDataEvent;
#define sz_xLbxSendTagDataEvent 32

typedef struct _LbxListenToOneEvent {
    BYTE	type;		/* always eventBase + LbxEvent */
    BYTE	lbxType;	/* LbxListenToOneEvent */
    CARD16	sequenceNumber B16;
    CARD32	client B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xLbxListenToOneEvent;
#define sz_xLbxListenToOneEvent 32

typedef struct _LbxListenToAllEvent {
    BYTE	type;		/* always eventBase + LbxEvent */
    BYTE	lbxType;	/* LbxListenToAllEvent */
    CARD16	sequenceNumber B16;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
    CARD32	pad7 B32;
} xLbxListenToAllEvent;
#define sz_xLbxListenToOneEvent 32

typedef struct _LbxReleaseCmapEvent {
    BYTE	type;		/* always eventBase + LbxEvent */
    BYTE	lbxType;	/* LbxReleaseCmapEvent */
    CARD16	sequenceNumber B16;
    Colormap	colormap B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
    CARD32	pad5 B32;
    CARD32	pad6 B32;
} xLbxReleaseCmapEvent;
#define sz_xLbxReleaseCmapEvent	32


typedef struct _LbxFreeCellsEvent {
    BYTE	type;		/* always eventBase + LbxEvent */
    BYTE	lbxType;	/* LbxFreeCellsEvent */
    CARD16	sequenceNumber B16;
    Colormap	colormap B32;
    CARD32	pixelStart B32;
    CARD32	pixelEnd B32;
    CARD32	pad1 B32;
    CARD32	pad2 B32;
    CARD32	pad3 B32;
    CARD32	pad4 B32;
} xLbxFreeCellsEvent;
#define sz_xLbxFreeCellsEvent	32


/*
 * squished X event sizes.  If these change, be sure to update lbxquish.c
 * and unsquish.c appropriately
 *
 * lbxsz_* is the padded squished length
 * lbxupsz_* is the unpadded squished length
 */

#define	  lbxsz_KeyButtonEvent		32
#define	lbxupsz_KeyButtonEvent		31

#define	  lbxsz_EnterLeaveEvent		32
#define	lbxupsz_EnterLeaveEvent		32

#define	  lbxsz_FocusEvent		12
#define	lbxupsz_FocusEvent		9

#define	  lbxsz_KeymapEvent		32
#define	lbxupsz_KeymapEvent		32

#define	  lbxsz_ExposeEvent		20
#define	lbxupsz_ExposeEvent		18

#define	  lbxsz_GfxExposeEvent		24
#define	lbxupsz_GfxExposeEvent		21

#define	  lbxsz_NoExposeEvent		12
#define	lbxupsz_NoExposeEvent		11

#define	  lbxsz_VisibilityEvent		12
#define	lbxupsz_VisibilityEvent		9

#define	  lbxsz_CreateNotifyEvent	24
#define	lbxupsz_CreateNotifyEvent	23

#define	  lbxsz_DestroyNotifyEvent	12
#define	lbxupsz_DestroyNotifyEvent	12

#define	  lbxsz_UnmapNotifyEvent	16
#define	lbxupsz_UnmapNotifyEvent	13

#define	  lbxsz_MapNotifyEvent		16
#define	lbxupsz_MapNotifyEvent		13

#define	  lbxsz_MapRequestEvent		12
#define	lbxupsz_MapRequestEvent		12

#define	  lbxsz_ReparentEvent		24
#define	lbxupsz_ReparentEvent		21

#define	  lbxsz_ConfigureNotifyEvent	28
#define	lbxupsz_ConfigureNotifyEvent	27

#define	  lbxsz_ConfigureRequestEvent	28
#define	lbxupsz_ConfigureRequestEvent	28

#define	  lbxsz_GravityEvent		16
#define	lbxupsz_GravityEvent		16

#define	  lbxsz_ResizeRequestEvent	12
#define	lbxupsz_ResizeRequestEvent	12

#define	  lbxsz_CirculateEvent		20
#define	lbxupsz_CirculateEvent		17

#define	  lbxsz_PropertyEvent		20
#define	lbxupsz_PropertyEvent		17

#define	  lbxsz_SelectionClearEvent	16
#define	lbxupsz_SelectionClearEvent	16

#define	  lbxsz_SelectionRequestEvent	28
#define	lbxupsz_SelectionRequestEvent	28

#define	  lbxsz_SelectionNotifyEvent	24
#define	lbxupsz_SelectionNotifyEvent	24

#define	  lbxsz_ColormapEvent		16
#define	lbxupsz_ColormapEvent		14

#define	  lbxsz_MappingNotifyEvent	8
#define	lbxupsz_MappingNotifyEvent	7

#define	  lbxsz_ClientMessageEvent	32
#define	lbxupsz_ClientMessageEvent	32

#define	lbxsz_UnknownEvent		32

#ifdef DEBUG

#define DBG_SWITCH	0x00000001
#define DBG_CLOSE	0x00000002
#define DBG_IO		0x00000004
#define DBG_READ_REQ	0x00000008
#define DBG_LEN		0x00000010
#define DBG_BLOCK	0x00000020
#define DBG_CLIENT	0x00000040
#define DBG_DELTA	0x00000080

extern int lbxDebug;

#define DBG(n,m)    if (lbxDebug & (n)) { fprintf m; } else
#else
#define DBG(n,m)
#endif

/*
 * Cancel the previous redefinition of the basic types, thus restoring their
 * X.h definitions.
 */

#undef XID
#undef Atom
#undef Colormap
#undef Drawable
#undef VisualID
#undef Window

#endif	/* _LBXSTR_H_ */
