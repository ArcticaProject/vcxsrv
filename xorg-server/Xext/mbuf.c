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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "window.h"
#include "os.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "gcstruct.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include "resource.h"
#include "opaque.h"
#include "sleepuntil.h"
#include "inputstr.h"
#include <X11/extensions/multibufconst.h>
#include <X11/extensions/multibufproto.h>

#include <stdio.h>
#if !defined(WIN32)
#include <sys/time.h>
#endif

/* given an OtherClientPtr obj, get the ClientPtr */
#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])

/* given a MultibufferPtr b, get the ClientPtr */
#define bClient(b)   (clients[CLIENT_ID(b->pPixmap->drawable.id)])

#define ValidEventMasks (ExposureMask|MultibufferClobberNotifyMask|MultibufferUpdateNotifyMask)

/* The _Multibuffer and _Multibuffers structures below refer to each other,
 * so we need this forward declaration
 */
typedef struct _Multibuffers	*MultibuffersPtr;

/*
 * per-Multibuffer data
 */
typedef struct _Multibuffer {
    MultibuffersPtr pMultibuffers;  /* associated window data */
    Mask	    eventMask;	    /* MultibufferClobberNotifyMask|ExposureMask|MultibufferUpdateNotifyMask */
    Mask	    otherEventMask; /* mask of all other clients event masks */
    OtherClients    *otherClients;  /* other clients that want events */
    int		    number;	    /* index of this buffer into array */
    int		    side;	    /* always Mono */
    int		    clobber;	    /* Unclobbered, PartiallyClobbered, FullClobbered */
    PixmapPtr	    pPixmap;	    /* associated pixmap */
} MultibufferRec, *MultibufferPtr;

/*
 * per-window data
 */

typedef struct _Multibuffers {
    WindowPtr	pWindow;		/* associated window */
    int		numMultibuffer;		/* count of buffers */
    int		refcnt;			/* ref count for delete */
    int		displayedMultibuffer;	/* currently active buffer */
    int		updateAction;		/* Undefined, Background, Untouched, Copied */
    int		updateHint;		/* Frequent, Intermittent, Static */
    int		windowMode;		/* always Mono */

    TimeStamp	lastUpdate;		/* time of last update */

    unsigned short	width, height;	/* last known window size */
    short		x, y;		/* for static gravity */

    MultibufferPtr	buffers;        /* array of numMultibuffer buffers */
} MultibuffersRec;

/*
 * per-screen data
 */
typedef struct _MultibufferScreen {
    PositionWindowProcPtr PositionWindow;		/* pWin, x,y */
} MultibufferScreenRec, *MultibufferScreenPtr;

/*
 * per display-image-buffers request data.
 */

typedef struct _DisplayRequest {
    struct _DisplayRequest	*next;
    TimeStamp			activateTime;
    ClientPtr			pClient;
    XID				id;
} DisplayRequestRec, *DisplayRequestPtr;

#define DestroyWindowMask		(1L<<0)
#define PositionWindowMask		(1L<<1)
#define PostValidateTreeMask		(1L<<2)
#define ClipNotifyMask			(1L<<3)
#define WindowExposuresMask		(1L<<4)
#define CopyWindowMask			(1L<<5)
#define ClearToBackgroundMask		(1L<<6)
#define ChangeWindowAttributesMask	(1L<<7)

extern int		MultibufferScreenIndex;
extern int		MultibufferWindowIndex;

extern RESTYPE		MultibufferDrawableResType;

extern void		MultibufferUpdate(	/* pMbuffer, time */
				MultibufferPtr /* pMultibuffer */,
				CARD32 /* time */
				);
extern void		MultibufferExpose(	/* pMbuffer, pRegion */
				MultibufferPtr /* pMultibuffer */,
				RegionPtr /* pRegion */
				);
extern void		MultibufferClobber(	/* pMbuffer */
				MultibufferPtr /* pMultibuffer */
				);

typedef struct _mbufWindow	*mbufWindowPtr;

void DestroyImageBuffers (WindowPtr	pWin);

/*
 * per-buffer data
 */

#define MB_DISPLAYED_BUFFER(pMBWindow) \
    ((pMBWindow)->buffers + (pMBWindow)->displayedMultibuffer)

typedef struct _mbufBuffer {
    mbufWindowPtr   pMBWindow;	    /* associated window data */
    Mask	    eventMask;	    /* client event mask */
    Mask	    otherEventMask; /* union of other clients' event masks */
    OtherClientsPtr otherClients;   /* other clients that want events */
    int		    number;	    /* index of this buffer into array */
    int		    side;	    /* stero side: always Mono */
    int		    clobber;	    /* clober state */
    DrawablePtr	    pDrawable;	    /* associated drawable */
} mbufBufferRec, *mbufBufferPtr;


/*
 * per-window data
 */

#define MB_WINDOW_PRIV(pWin) \
    ((mbufWindowPtr)((pWin)->devPrivates[MultibufferWindowIndex].ptr))

typedef struct _mbufWindow {
    WindowPtr	pWindow;		/* associated window */
    int		numMultibuffer;		/* count of buffers */
    mbufBufferPtr buffers;		/* array of (numMultibuffer) buffers */
    int		displayedMultibuffer;	/* currently active buffer */
    int		updateAction;		/* Undefined, Background,
					   Untouched, Copied */
    int		updateHint;		/* Frequent, Intermittent, Static */
    int		windowMode;		/* always Mono */
    TimeStamp	lastUpdate;		/* time of last update */
    short		x, y;		/* for static gravity */
    unsigned short	width, height;	/* last known window size */
    DevUnion		devPrivate;
} mbufWindowRec;


/*
 * per-screen data
 */

#define MB_SCREEN_PRIV(pScreen) \
    ((mbufScreenPtr)((pScreen)->devPrivates[MultibufferScreenIndex].ptr))

typedef struct _mbufScreen {
    long mbufWindowCount;		/* count of multibuffered windows */

    /* Wrap pScreen->DestroyWindow */
    DestroyWindowProcPtr DestroyWindow;
    long funcsWrapped;			/* flags which functions are wrapped */

    /* Initialized by device-dependent section */
    int  nInfo;				/* number of buffer info rec's */
    xMbufBufferInfo *pInfo;		/* buffer info (for Normal buffers) */

    int  (* CreateImageBuffers)(
		WindowPtr		/* pWin */,
		int			/* nbuf */,
		XID *			/* ids */,
		int			/* action */,
		int			/* hint */
		);
    void (* DestroyImageBuffers)(
		WindowPtr		/* pWin */
		);
    void (* DisplayImageBuffers)(
		ScreenPtr		/* pScreen */,
		mbufBufferPtr *		/* ppMBBuffer */,
		mbufWindowPtr *		/* ppMBWindow */,
		int			/* nbuf */
		);
    void (* ClearImageBufferArea)(
		mbufBufferPtr		/* pMBBuffer */,
		short			/* x */,
		short			/* y */,
		unsigned short		/* width */,
		unsigned short		/* height */,
		Bool			/* exposures */
		);
    Bool (* ChangeMBufferAttributes)(	/* pMBWindow, vmask */
		/* FIXME */
		);
    Bool (* ChangeBufferAttributes)(	/* pMBBuffer, vmask */
		/* FIXME */
		);
    void (* DeleteBufferDrawable)(
		DrawablePtr		/* pDrawable */
		);
    void (* WrapScreenFuncs)(
		ScreenPtr		/* pScreen */
		);
    void (* ResetProc)(
		ScreenPtr		/* pScreen */
		);
    DevUnion	devPrivate;
} mbufScreenRec, *mbufScreenPtr;


/* Privates to mbufScreenRec */

#ifdef _MULTIBUF_PIXMAP_
#define MB_SCREEN_PRIV_PIXMAP(pScreen) \
    ((mbufPixmapPrivPtr) MB_SCREEN_PRIV((pScreen))->devPrivate.ptr)

typedef struct _mbufPixmapPriv
{
    /* Pointers to wrapped functions */
    PositionWindowProcPtr PositionWindow;		/* pWin, x,y */
    long funcsWrapped;			/* flags which functions are wrapped */
} mbufPixmapPrivRec, *mbufPixmapPrivPtr;
#endif /* _MULTIBUF_PIXMAP_ */


#ifdef _MULTIBUF_BUFFER_

extern int frameWindowPrivateIndex;

#define MB_SCREEN_PRIV_BUFFER(pScreen) \
    ((mbufBufferPrivPtr) MB_SCREEN_PRIV((pScreen))->devPrivate.ptr)

typedef struct _mbufBufferPriv
{
    DevUnion	*frameBuffer;	/* Array of screen framebuffers */
    DevUnion	selectPlane;	/* Plane(s) that select displayed buffer */

    /*
     * Note: subtractRgn and unionRgn may overlap. subtractRgn is a union
     * of all the old clipLists of the windows that are displaying
     * the backbuffer. unionRgn is the union of all the new clipLists
     * of the same windows.
     */

    RegionRec	backBuffer;	/* Area of screen displaying back buffer */
    RegionRec   subtractRgn;	/* Regions lost to backBuffer   */
    RegionRec   unionRgn;	/* Regions gained by backBuffer */
    Bool	rgnChanged;	/* TRUE if "backBuffer" needs to be updated */

    void (* CopyBufferBits)();	/* pMBWindow, srcBufferNum, dstBufferNum */
    void (* DrawSelectPlane)();	/* pScreen, selectPlane, pRegion, bufferNum */

    /* Pointers to wrapped functions */
    PostValidateTreeProcPtr	PostValidateTree; /* pParent, pChild, kind */
    ClipNotifyProcPtr		ClipNotify;       /* pWin, dx, dy */
    WindowExposuresProcPtr	WindowExposures;  /* pWin, pRegion */
    CopyWindowProcPtr		CopyWindow;       /* pWin, oldPt, pOldRegion */
    ClearToBackgroundProcPtr	ClearToBackground; /* pWin, x,y,w,h, sendExpose */
    ChangeWindowAttributesProcPtr ChangeWindowAttributes; /* pWin, vmask */
    long funcsWrapped;			/* flags which functions are wrapped */
    unsigned  inClearToBackground:1;	/* used by WindowExposure */
} mbufBufferPrivRec, *mbufBufferPrivPtr;
#endif /* _MULTIBUF_BUFFER_ */

static int		MultibufferEventBase;
static int		MultibufferErrorBase;
static int MultibufferScreenPrivKeyIndex;
static DevPrivateKey MultibufferScreenPrivKey = &MultibufferScreenPrivKeyIndex;
static int MultibufferWindowPrivKeyIndex;
static DevPrivateKey MultibufferWindowPrivKey = &MultibufferWindowPrivKeyIndex;

static void		PerformDisplayRequest (
				MultibuffersPtr * /* ppMultibuffers */,
				MultibufferPtr * /* pMultibuffer */,
				int /* nbuf */
				);
static Bool		QueueDisplayRequest (
				ClientPtr /* client */,
				TimeStamp /* activateTime */
				);

static void		BumpTimeStamp (
				TimeStamp * /* ts */,
				CARD32 /* inc */
				);

static void		AliasMultibuffer (
				MultibuffersPtr /* pMultibuffers */,
				int /* i */
				);
static void		RecalculateMultibufferOtherEvents (
				MultibufferPtr /* pMultibuffer */
				);
static int		EventSelectForMultibuffer(
				MultibufferPtr /* pMultibuffer */,
				ClientPtr /* client */,
				Mask /* mask */
				);

/*
 * The Pixmap associated with a buffer can be found as a resource
 * with this type
 */
RESTYPE			MultibufferDrawableResType;
static int		MultibufferDrawableDelete (
				pointer /* value */,
				XID /* id */
				);
/*
 * The per-buffer data can be found as a resource with this type.
 * the resource id of the per-buffer data is the same as the resource
 * id of the pixmap
 */
static RESTYPE		MultibufferResType;
static int		MultibufferDelete (
				pointer /* value */,
				XID /* id */
				);

/*
 * The per-window data can be found as a resource with this type,
 * using the window resource id
 */
static RESTYPE		MultibuffersResType;
static int		MultibuffersDelete (
				pointer /* value */,
				XID /* id */
				);

/*
 * Clients other than the buffer creator attach event masks in
 * OtherClient structures; each has a resource of this type.
 */
static RESTYPE		OtherClientResType;
static int		OtherClientDelete (
				pointer /* value */,
				XID /* id */
				);

/****************
 * MultibufferExtensionInit
 *
 * Called from InitExtensions in main()
 *
 ****************/

extern DISPATCH_PROC(ProcGetBufferAttributes);

static DISPATCH_PROC(ProcClearImageBufferArea);
static DISPATCH_PROC(ProcCreateImageBuffers);
static DISPATCH_PROC(ProcDestroyImageBuffers);
static DISPATCH_PROC(ProcDisplayImageBuffers);
static DISPATCH_PROC(ProcGetBufferInfo);
static DISPATCH_PROC(ProcGetBufferVersion);
static DISPATCH_PROC(ProcGetMBufferAttributes);
static DISPATCH_PROC(ProcMultibufferDispatch);
static DISPATCH_PROC(ProcSetBufferAttributes);
static DISPATCH_PROC(ProcSetMBufferAttributes);
static DISPATCH_PROC(SProcClearImageBufferArea);
static DISPATCH_PROC(SProcCreateImageBuffers);
static DISPATCH_PROC(SProcDestroyImageBuffers);
static DISPATCH_PROC(SProcDisplayImageBuffers);
static DISPATCH_PROC(SProcGetBufferAttributes);
static DISPATCH_PROC(SProcGetBufferInfo);
static DISPATCH_PROC(SProcGetBufferVersion);
static DISPATCH_PROC(SProcGetMBufferAttributes);
static DISPATCH_PROC(SProcMultibufferDispatch);
static DISPATCH_PROC(SProcSetBufferAttributes);
static DISPATCH_PROC(SProcSetMBufferAttributes);

static void		MultibufferResetProc(
				ExtensionEntry * /* extEntry */
				);
static void		SClobberNotifyEvent(
				xMbufClobberNotifyEvent * /* from */,
				xMbufClobberNotifyEvent	* /* to */
				);
static void		SUpdateNotifyEvent(
				xMbufUpdateNotifyEvent * /* from */,
				xMbufUpdateNotifyEvent * /* to */
				);
static Bool		MultibufferPositionWindow(
				WindowPtr /* pWin */,
				int /* x */,
				int /* y */
				);

static void		SetupBackgroundPainter (
				WindowPtr /* pWin */,
				GCPtr /* pGC */
				);

static int		DeliverEventsToMultibuffer (
				MultibufferPtr /* pMultibuffer */,
				xEvent * /* pEvents */,
				int /* count */,
				Mask /* filter */
				);

void
MultibufferExtensionInit()
{
    ExtensionEntry	    *extEntry;
    int			    i, j;
    ScreenPtr		    pScreen;
    MultibufferScreenPtr    pMultibufferScreen;

    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (!(pMultibufferScreen = xalloc (sizeof (MultibufferScreenRec))))
	{
	    for (j = 0; j < i; j++)
		xfree (dixLookupPrivate(&screenInfo.screens[j]->devPrivates, MultibufferScreenPrivKey));
	    return;
	}
	dixSetPrivate(&pScreen->devPrivates, MultibufferScreenPrivKey, pMultibufferScreen);
	/*
 	 * wrap PositionWindow to resize the pixmap when the window
	 * changes size
 	 */
	pMultibufferScreen->PositionWindow = pScreen->PositionWindow;
	pScreen->PositionWindow = MultibufferPositionWindow;
    }
    /*
     * create the resource types
     */
    MultibufferDrawableResType =
	CreateNewResourceType(MultibufferDrawableDelete)|RC_DRAWABLE;
    MultibufferResType = CreateNewResourceType(MultibufferDelete);
    MultibuffersResType = CreateNewResourceType(MultibuffersDelete);
    OtherClientResType = CreateNewResourceType(OtherClientDelete);
    if (MultibufferDrawableResType && MultibufferResType &&
	MultibuffersResType && 	OtherClientResType &&
	(extEntry = AddExtension(MULTIBUFFER_PROTOCOL_NAME,
				 MultibufferNumberEvents, 
				 MultibufferNumberErrors,
				 ProcMultibufferDispatch, SProcMultibufferDispatch,
				 MultibufferResetProc, StandardMinorOpcode)))
    {
	MultibufferEventBase = extEntry->eventBase;
	MultibufferErrorBase = extEntry->errorBase;
	EventSwapVector[MultibufferEventBase + MultibufferClobberNotify] = (EventSwapPtr) SClobberNotifyEvent;
	EventSwapVector[MultibufferEventBase + MultibufferUpdateNotify] = (EventSwapPtr) SUpdateNotifyEvent;
    }
}

/*ARGSUSED*/
static void
MultibufferResetProc (extEntry)
ExtensionEntry	*extEntry;
{
    int			    i;
    ScreenPtr		    pScreen;
    MultibufferScreenPtr    pMultibufferScreen;
    
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if ((pMultibufferScreen = (MultibufferScreenPtr)dixLookupPrivate(&pScreen->devPrivates, MultibufferScreenPrivKey)))
	{
	    pScreen->PositionWindow = pMultibufferScreen->PositionWindow;
	    xfree (pMultibufferScreen);
	}
    }
}

static int
ProcGetBufferVersion (client)
    ClientPtr	client;
{
    xMbufGetBufferVersionReply	rep;
    int		n;

    REQUEST_SIZE_MATCH (xMbufGetBufferVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = MULTIBUFFER_MAJOR_VERSION;
    rep.minorVersion = MULTIBUFFER_MINOR_VERSION;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof (xMbufGetBufferVersionReply), (char *)&rep);
    return (client->noClientException);
}

static void
SetupBackgroundPainter (pWin, pGC)
    WindowPtr	pWin;
    GCPtr	pGC;
{
    pointer	    gcvalues[4];
    int		    ts_x_origin, ts_y_origin;
    PixUnion	    background;
    int		    backgroundState;
    Mask	    gcmask;

    /*
     * First take care of any ParentRelative stuff by altering the
     * tile/stipple origin to match the coordinates of the upper-left
     * corner of the first ancestor without a ParentRelative background.
     * This coordinate is, of course, negative.
     */

    ts_x_origin = ts_y_origin = 0;
    while (pWin->backgroundState == ParentRelative) {
	ts_x_origin -= pWin->origin.x;
	ts_y_origin -= pWin->origin.y;
	pWin = pWin->parent;
    }
    backgroundState = pWin->backgroundState;
    background = pWin->background;

    switch (backgroundState)
    {
    case BackgroundPixel:
	gcvalues[0] = (pointer) background.pixel;
	gcvalues[1] = (pointer) FillSolid;
	gcmask = GCForeground|GCFillStyle;
	break;

    case BackgroundPixmap:
	gcvalues[0] = (pointer) FillTiled;
	gcvalues[1] = (pointer) background.pixmap;
	gcvalues[2] = (pointer)(long) ts_x_origin;
	gcvalues[3] = (pointer)(long) ts_y_origin;
	gcmask = GCFillStyle|GCTile|GCTileStipXOrigin|GCTileStipYOrigin;
	break;

    default:
	gcvalues[0] = (pointer) GXnoop;
	gcmask = GCFunction;
    }
    DoChangeGC(pGC, gcmask, (XID *)gcvalues, TRUE);
}

int
CreateImageBuffers (pWin, nbuf, ids, action, hint)
    WindowPtr	pWin;
    int		nbuf;
    XID		*ids;
    int		action;
    int		hint;
{
    MultibuffersPtr	pMultibuffers;
    MultibufferPtr	pMultibuffer;
    ScreenPtr		pScreen;
    int			width, height, depth;
    int			i;
    GCPtr		pClearGC = NULL;
    xRectangle		clearRect;

    DestroyImageBuffers(pWin);
    pMultibuffers = xalloc (sizeof (MultibuffersRec) + nbuf * sizeof (MultibufferRec));
    if (!pMultibuffers)
	return BadAlloc;
    pMultibuffers->pWindow = pWin;
    pMultibuffers->buffers = (MultibufferPtr) (pMultibuffers + 1);
    pMultibuffers->refcnt = pMultibuffers->numMultibuffer = 0;
    if (!AddResource (pWin->drawable.id, MultibuffersResType, (pointer) pMultibuffers))
	return BadAlloc;
    width = pWin->drawable.width;
    height = pWin->drawable.height;
    depth = pWin->drawable.depth;
    pScreen = pWin->drawable.pScreen;

    if (pWin->backgroundState != None)
    {
	pClearGC = GetScratchGC (pWin->drawable.depth, pScreen);
	SetupBackgroundPainter (pWin, pClearGC);
	clearRect.x = clearRect.y = 0;
	clearRect.width = width;
	clearRect.height = height;
    }

    for (i = 0; i < nbuf; i++)
    {
	pMultibuffer = &pMultibuffers->buffers[i];
	pMultibuffer->eventMask = 0L;
	pMultibuffer->otherEventMask = 0L;
	pMultibuffer->otherClients = (OtherClientsPtr) NULL;
	pMultibuffer->number = i;
	pMultibuffer->side = MultibufferSideMono;
	pMultibuffer->clobber = MultibufferUnclobbered;
	pMultibuffer->pMultibuffers = pMultibuffers;
	if (!AddResource (ids[i], MultibufferResType, (pointer) pMultibuffer))
	    break;
	pMultibuffer->pPixmap = (*pScreen->CreatePixmap) (pScreen, width, height, depth, 0);
	if (!pMultibuffer->pPixmap)
	    break;
	if (!AddResource (ids[i], MultibufferDrawableResType, (pointer) pMultibuffer->pPixmap))
	{
	    FreeResource (ids[i], MultibufferResType);
	    (*pScreen->DestroyPixmap) (pMultibuffer->pPixmap);
	    break;
	}
	pMultibuffer->pPixmap->drawable.id = ids[i];

	if (i > 0 && pClearGC)
	{
	    ValidateGC((DrawablePtr)pMultibuffer->pPixmap, pClearGC);
	    (*pClearGC->ops->PolyFillRect)((DrawablePtr)pMultibuffer->pPixmap,
					   pClearGC, 1, &clearRect);
	}
    }
    pMultibuffers->numMultibuffer = i;
    pMultibuffers->refcnt = i;
    pMultibuffers->displayedMultibuffer = -1;
    if (i > 0)
	AliasMultibuffer (pMultibuffers, 0);
    pMultibuffers->updateAction = action;
    pMultibuffers->updateHint = hint;
    pMultibuffers->windowMode = MultibufferModeMono;
    pMultibuffers->lastUpdate.months = 0;
    pMultibuffers->lastUpdate.milliseconds = 0;
    pMultibuffers->width = width;
    pMultibuffers->height = height;
    dixSetPrivate(&pWin->devPrivates, MultibufferWindowPrivKey, pMultibuffers);
    if (pClearGC) FreeScratchGC(pClearGC);
    return Success;
}


static int
ProcCreateImageBuffers (client)
    ClientPtr	client;
{
    REQUEST(xMbufCreateImageBuffersReq);
    xMbufCreateImageBuffersReply	rep;
    int		n;
    WindowPtr			pWin;
    XID				*ids;
    int				len, nbuf, i, err, rc;

    REQUEST_AT_LEAST_SIZE (xMbufCreateImageBuffersReq);
    len = stuff->length - bytes_to_int32(sizeof(xMbufCreateImageBuffersReq));
    if (len == 0)
	return BadLength;
    rc = dixLookupWindow(&pWin, stuff->window, client, DixUnknownAccess);
    if (rc != Success)
	return rc;
    if (pWin->drawable.class == InputOnly)
	return BadMatch;
    switch (stuff->updateAction)
    {
    case MultibufferUpdateActionUndefined:
    case MultibufferUpdateActionBackground:
    case MultibufferUpdateActionUntouched:
    case MultibufferUpdateActionCopied:
	break;
    default:
	client->errorValue = stuff->updateAction;
	return BadValue;
    }
    switch (stuff->updateHint)
    {
    case MultibufferUpdateHintFrequent:
    case MultibufferUpdateHintIntermittent:
    case MultibufferUpdateHintStatic:
	break;
    default:
	client->errorValue = stuff->updateHint;
	return BadValue;
    }
    nbuf = len;
    ids = (XID *) &stuff[1];
    for (i = 0; i < nbuf; i++)
    {
	LEGAL_NEW_RESOURCE(ids[i], client);
    }
    err = CreateImageBuffers (pWin, nbuf, ids,
			      stuff->updateAction, stuff->updateHint);
    if (err != Success)
	return err;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.numberBuffer = ((MultibuffersPtr) (dixLookupPrivate(&pWin->devPrivates, MultibufferWindowPrivKey)))->numMultibuffer;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.numberBuffer, n);
    }
    WriteToClient(client, sizeof (xMbufCreateImageBuffersReply), (char*)&rep);
    return (client->noClientException);
}

static int
ProcDisplayImageBuffers (client)
    ClientPtr	client;
{
    REQUEST(xMbufDisplayImageBuffersReq);
    MultibufferPtr	    *pMultibuffer;
    MultibuffersPtr	    *ppMultibuffers;
    int		    nbuf;
    XID		    *ids;
    int		    i, j;
    CARD32	    minDelay;
    TimeStamp	    activateTime, bufferTime;
    int		    rc;
    

    REQUEST_AT_LEAST_SIZE (xMbufDisplayImageBuffersReq);
    nbuf = stuff->length - bytes_to_int32(sizeof (xMbufDisplayImageBuffersReq));
    if (!nbuf)
	return Success;
    minDelay = stuff->minDelay;
    ids = (XID *) &stuff[1];
    ppMultibuffers = xalloc(nbuf * sizeof (MultibuffersPtr));
    pMultibuffer = xalloc(nbuf * sizeof (MultibufferPtr));
    if (!ppMultibuffers || !pMultibuffer)
    {
	if (ppMultibuffers) xfree(ppMultibuffers);
	if (pMultibuffer)   xfree(pMultibuffer);
	client->errorValue = 0;
	return BadAlloc;
    }
    activateTime.months = 0;
    activateTime.milliseconds = 0;
    for (i = 0; i < nbuf; i++)
    {
	rc = dixLookupResourceByType(&pMultibuffer[i], ids[i],
		MultibufferResType, client, DixUseAccess);
	if (rc != Success)
	{
	    xfree(ppMultibuffers);
	    xfree(pMultibuffer);
	    client->errorValue = ids[i];
	    return MultibufferErrorBase + MultibufferBadBuffer;
	}
	ppMultibuffers[i] = pMultibuffer[i]->pMultibuffers;
	for (j = 0; j < i; j++)
	{
	    if (ppMultibuffers[i] == ppMultibuffers[j])
	    {
	    	xfree(ppMultibuffers);
	    	xfree(pMultibuffer);
		client->errorValue = ids[i];
	    	return BadMatch;
	    }
	}
	bufferTime = ppMultibuffers[i]->lastUpdate;
	BumpTimeStamp (&bufferTime, minDelay);
	if (CompareTimeStamps (bufferTime, activateTime) == LATER)
	    activateTime = bufferTime;
    }
    UpdateCurrentTime ();
    if (CompareTimeStamps (activateTime, currentTime) == LATER &&
	QueueDisplayRequest (client, activateTime))
    {
	;
    }
    else
	PerformDisplayRequest (ppMultibuffers, pMultibuffer, nbuf);

    xfree(ppMultibuffers);
    xfree(pMultibuffer);
    return Success;
}


static int
ProcDestroyImageBuffers (client)
    ClientPtr	client;
{
    REQUEST (xMbufDestroyImageBuffersReq);
    WindowPtr	pWin;
    int rc;

    REQUEST_SIZE_MATCH (xMbufDestroyImageBuffersReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixUnknownAccess);
    if (rc != Success)
	return rc;
    DestroyImageBuffers (pWin);
    return Success;
}

static int
ProcSetMBufferAttributes (client)
    ClientPtr	client;
{
    REQUEST (xMbufSetMBufferAttributesReq);
    WindowPtr	pWin;
    MultibuffersPtr	pMultibuffers;
    int		len, rc;
    Mask	vmask;
    Mask	index2;
    CARD32	updateHint;
    XID		*vlist;

    REQUEST_AT_LEAST_SIZE (xMbufSetMBufferAttributesReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixUnknownAccess);
    if (rc != Success)
	return rc;
    rc = dixLookupResourceByType(&pMultibuffers, pWin->drawable.id,
	    MultibufferResType, client, DixSetAttrAccess);
    if (rc != Success)
	return BadMatch;
    len = stuff->length - bytes_to_int32(sizeof (xMbufSetMBufferAttributesReq));
    vmask = stuff->valueMask;
    if (len != Ones (vmask))
	return BadLength;
    vlist = (XID *) &stuff[1];
    while (vmask)
    {
	index2 = (Mask) lowbit (vmask);
	vmask &= ~index2;
	switch (index2)
	{
	case MultibufferWindowUpdateHint:
	    updateHint = (CARD32) *vlist;
	    switch (updateHint)
	    {
	    case MultibufferUpdateHintFrequent:
	    case MultibufferUpdateHintIntermittent:
	    case MultibufferUpdateHintStatic:
		pMultibuffers->updateHint = updateHint;
		break;
	    default:
		client->errorValue = updateHint;
		return BadValue;
	    }
	    vlist++;
	    break;
	default:
	    client->errorValue = stuff->valueMask;
	    return BadValue;
	}
    }
    return Success;
}

static int
ProcGetMBufferAttributes (client)
    ClientPtr	client;
{
    REQUEST (xMbufGetMBufferAttributesReq);
    WindowPtr	pWin;
    MultibuffersPtr	pMultibuffers;
    XID		*ids;
    xMbufGetMBufferAttributesReply  rep;
    int		i, n, rc;

    REQUEST_SIZE_MATCH (xMbufGetMBufferAttributesReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixUnknownAccess);
    if (rc != Success)
	return rc;
    rc = dixLookupResourceByType(&pMultibuffers, pWin->drawable.id,
	    MultibufferResType, client, DixGetAttrAccess);
    if (rc != Success)
	return BadAccess;
    ids = xalloc (pMultibuffers->numMultibuffer * sizeof (XID));
    if (!ids)
	return BadAlloc;
    for (i = 0; i < pMultibuffers->numMultibuffer; i++)
	ids[i] = pMultibuffers->buffers[i].pPixmap->drawable.id;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = pMultibuffers->numMultibuffer;
    rep.displayedBuffer = pMultibuffers->displayedMultibuffer;
    rep.updateAction = pMultibuffers->updateAction;
    rep.updateHint = pMultibuffers->updateHint;
    rep.windowMode = pMultibuffers->windowMode;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swaps(&rep.displayedBuffer, n);
	SwapLongs (ids, pMultibuffers->numMultibuffer);
    }
    WriteToClient (client, sizeof(xMbufGetMBufferAttributesReply),
		   (char *)&rep);
    WriteToClient (client, (int)(pMultibuffers->numMultibuffer * sizeof (XID)),
		   (char *)ids);
    xfree((pointer) ids);
    return client->noClientException;
}

static int
ProcSetBufferAttributes (client)
    ClientPtr	client;
{
    REQUEST(xMbufSetBufferAttributesReq);
    MultibufferPtr	pMultibuffer;
    int		len;
    Mask	vmask, index2;
    XID		*vlist;
    Mask	eventMask;
    int		result;
    int		rc;

    REQUEST_AT_LEAST_SIZE (xMbufSetBufferAttributesReq);

    rc = dixLookupResourceByType(&pMultibuffer, stuff->buffer,
	    MultibufferResType, client, DixSetAttrAccess);
    if (rc != Success)
	return MultibufferErrorBase + MultibufferBadBuffer;
    len = stuff->length - bytes_to_int32(sizeof (xMbufSetBufferAttributesReq));
    vmask = stuff->valueMask;
    if (len != Ones (vmask))
	return BadLength;
    vlist = (XID *) &stuff[1];
    while (vmask)
    {
	index2 = (Mask) lowbit (vmask);
	vmask &= ~index2;
	switch (index2)
	{
	case MultibufferBufferEventMask:
	    eventMask = (Mask) *vlist;
	    vlist++;
	    result = EventSelectForMultibuffer (pMultibuffer, client, eventMask);
	    if (result != Success)
		return result;
	    break;
	default:
	    client->errorValue = stuff->valueMask;
	    return BadValue;
	}
    }
    return Success;
}

int
ProcGetBufferAttributes (client)
    ClientPtr	client;
{
    REQUEST(xMbufGetBufferAttributesReq);
    MultibufferPtr	pMultibuffer;
    xMbufGetBufferAttributesReply	rep;
    OtherClientsPtr		other;
    int				n;
    int				rc;

    REQUEST_SIZE_MATCH (xMbufGetBufferAttributesReq);
    rc = dixLookupResourceByType(&pMultibuffer, stuff->buffer,
	    MultibufferResType, client, DixGetAttrAccess);
    if (rc != Success)
	return MultibufferErrorBase + MultibufferBadBuffer;
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.window = pMultibuffer->pMultibuffers->pWindow->drawable.id;
    if (bClient (pMultibuffer) == client)
	rep.eventMask = pMultibuffer->eventMask;
    else
    {
	rep.eventMask = (Mask) 0L;
	for (other = pMultibuffer->otherClients; other; other = other->next)
	    if (SameClient (other, client))
	    {
		rep.eventMask = other->mask;
		break;
	    }
    }
    rep.bufferIndex = pMultibuffer->number;
    rep.side = pMultibuffer->side;
    if (client->swapped)
    {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.window, n);
	swapl(&rep.eventMask, n);
	swaps(&rep.bufferIndex, n);
    }
    WriteToClient(client, sizeof (xMbufGetBufferAttributesReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcGetBufferInfo (client)
    ClientPtr	client;
{
    REQUEST (xMbufGetBufferInfoReq);
    DrawablePtr		    pDrawable;
    xMbufGetBufferInfoReply rep;
    ScreenPtr		    pScreen;
    int			    i, j, k, n, rc;
    xMbufBufferInfo	    *pInfo;
    int			    nInfo;
    DepthPtr		    pDepth;

    rc = dixLookupDrawable(&pDrawable, stuff->drawable, client, 0,
			   DixUnknownAccess);
    if (rc != Success)
	return rc;
    pScreen = pDrawable->pScreen;
    nInfo = 0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	pDepth = &pScreen->allowedDepths[i];
	nInfo += pDepth->numVids;
    }
    pInfo = xalloc (nInfo * sizeof (xMbufBufferInfo));
    if (!pInfo)
	return BadAlloc;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = nInfo * bytes_to_int32(sizeof (xMbufBufferInfo));
    rep.normalInfo = nInfo;
    rep.stereoInfo = 0;
    if (client->swapped)
    {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swaps(&rep.normalInfo, n);
	swaps(&rep.stereoInfo, n);
    }

    k = 0;
    for (i = 0; i < pScreen->numDepths; i++)
    {
	pDepth = &pScreen->allowedDepths[i];
	for (j = 0; j < pDepth->numVids; j++)
	{
	    pInfo[k].visualID = pDepth->vids[j];
	    pInfo[k].maxBuffers = 0;
	    pInfo[k].depth = pDepth->depth;
	    if (client->swapped)
	    {
		swapl (&pInfo[k].visualID, n);
		swaps (&pInfo[k].maxBuffers, n);
	    }
	    k++;
	}
    }
    WriteToClient (client, sizeof (xMbufGetBufferInfoReply), (pointer) &rep);
    WriteToClient (client, (int) nInfo * sizeof (xMbufBufferInfo), (pointer) pInfo);
    xfree ((pointer) pInfo);
    return client->noClientException;
}

static int
ProcClearImageBufferArea (client)
    ClientPtr	client;
{
    REQUEST (xMbufClearImageBufferAreaReq);
    MultibufferPtr	pMultibuffer;
    WindowPtr pWin;
    xRectangle clearRect;
    int width, height;
    DrawablePtr pDrawable;
    ScreenPtr pScreen;
    int rc;

    REQUEST_SIZE_MATCH (xMbufClearImageBufferAreaReq);
    rc = dixLookupResourceByType(&pMultibuffer, stuff->buffer,
	    MultibufferResType, client, DixWriteAccess);
    if (rc != Success)
	return MultibufferErrorBase + MultibufferBadBuffer;
    if ((stuff->exposures != xTrue) && (stuff->exposures != xFalse))
    {
	client->errorValue = stuff->exposures;
        return(BadValue);
    }
    pWin = pMultibuffer->pMultibuffers->pWindow;
    width  = pWin->drawable.width;
    height = pWin->drawable.height;
    pScreen = pWin->drawable.pScreen;

    clearRect.x = stuff->x;
    clearRect.y = stuff->y;
    clearRect.width  = stuff->width  ? stuff->width  : width;
    clearRect.height = stuff->height ? stuff->height : height;

    if (pWin->backgroundState != None)
    {
	GCPtr pClearGC;
	pClearGC = GetScratchGC (pWin->drawable.depth, pScreen);
	SetupBackgroundPainter (pWin, pClearGC);

	if (pMultibuffer->number == pMultibuffer->pMultibuffers->displayedMultibuffer)
	    pDrawable = (DrawablePtr)pWin;
	else
	    pDrawable = (DrawablePtr)pMultibuffer->pPixmap;

	ValidateGC(pDrawable, pClearGC);
	(*pClearGC->ops->PolyFillRect) (pDrawable, pClearGC, 1, &clearRect);
	FreeScratchGC(pClearGC);
    }

    if (stuff->exposures)
    {
	RegionRec region;
	BoxRec box;
	box.x1 = clearRect.x;
	box.y1 = clearRect.y;
	box.x2 = clearRect.x + clearRect.width;
	box.y2 = clearRect.y + clearRect.height;
	REGION_INIT(pScreen, &region, &box, 1);
	MultibufferExpose(pMultibuffer, &region);
	REGION_UNINIT(pScreen, &region);
    }
    return Success;
}

static int
ProcMultibufferDispatch (client)
    ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_MbufGetBufferVersion:
	return ProcGetBufferVersion (client);
    case X_MbufCreateImageBuffers:
	return ProcCreateImageBuffers (client);
    case X_MbufDisplayImageBuffers:
	return ProcDisplayImageBuffers (client);
    case X_MbufDestroyImageBuffers:
	return ProcDestroyImageBuffers (client);
    case X_MbufSetMBufferAttributes:
	return ProcSetMBufferAttributes (client);
    case X_MbufGetMBufferAttributes:
	return ProcGetMBufferAttributes (client);
    case X_MbufSetBufferAttributes:
	return ProcSetBufferAttributes (client);
    case X_MbufGetBufferAttributes:
	return ProcGetBufferAttributes (client);
    case X_MbufGetBufferInfo:
	return ProcGetBufferInfo (client);
    case X_MbufClearImageBufferArea:
	return ProcClearImageBufferArea (client);
    default:
	return BadRequest;
    }
}

static int
SProcGetBufferVersion (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufGetBufferVersionReq);

    swaps (&stuff->length, n);
    return ProcGetBufferVersion (client);
}

static int
SProcCreateImageBuffers (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufCreateImageBuffersReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xMbufCreateImageBuffersReq);
    swapl (&stuff->window, n);
    SwapRestL(stuff);
    return ProcCreateImageBuffers (client);
}

static int
SProcDisplayImageBuffers (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufDisplayImageBuffersReq);
    
    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xMbufDisplayImageBuffersReq);
    swaps (&stuff->minDelay, n);
    swaps (&stuff->maxDelay, n);
    SwapRestL(stuff);
    return ProcDisplayImageBuffers (client);
}

static int
SProcDestroyImageBuffers (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufDestroyImageBuffersReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xMbufDestroyImageBuffersReq);
    swapl (&stuff->window, n);
    return ProcDestroyImageBuffers (client);
}

static int
SProcSetMBufferAttributes (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufSetMBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufSetMBufferAttributesReq);
    swapl (&stuff->window, n);
    swapl (&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSetMBufferAttributes (client);
}

static int
SProcGetMBufferAttributes (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufGetMBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufGetMBufferAttributesReq);
    swapl (&stuff->window, n);
    return ProcGetMBufferAttributes (client);
}

static int
SProcSetBufferAttributes (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufSetBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufSetBufferAttributesReq);
    swapl (&stuff->buffer, n);
    swapl (&stuff->valueMask, n);
    SwapRestL(stuff);
    return ProcSetBufferAttributes (client);
}

static int
SProcGetBufferAttributes (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufGetBufferAttributesReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xMbufGetBufferAttributesReq);
    swapl (&stuff->buffer, n);
    return ProcGetBufferAttributes (client);
}

static int
SProcGetBufferInfo (client)
    ClientPtr	client;
{
    int    n;
    REQUEST (xMbufGetBufferInfoReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xMbufGetBufferInfoReq);
    swapl (&stuff->drawable, n);
    return ProcGetBufferInfo (client);
}

static int
SProcClearImageBufferArea(client)
    ClientPtr client;
{
    char n;
    REQUEST(xMbufClearImageBufferAreaReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xMbufClearImageBufferAreaReq);
    swapl(&stuff->buffer, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    return ProcClearImageBufferArea(client);
}

static int
SProcMultibufferDispatch (client)
    ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_MbufGetBufferVersion:
	return SProcGetBufferVersion (client);
    case X_MbufCreateImageBuffers:
	return SProcCreateImageBuffers (client);
    case X_MbufDisplayImageBuffers:
	return SProcDisplayImageBuffers (client);
    case X_MbufDestroyImageBuffers:
	return SProcDestroyImageBuffers (client);
    case X_MbufSetMBufferAttributes:
	return SProcSetMBufferAttributes (client);
    case X_MbufGetMBufferAttributes:
	return SProcGetMBufferAttributes (client);
    case X_MbufSetBufferAttributes:
	return SProcSetBufferAttributes (client);
    case X_MbufGetBufferAttributes:
	return SProcGetBufferAttributes (client);
    case X_MbufGetBufferInfo:
	return SProcGetBufferInfo (client);
    case X_MbufClearImageBufferArea:
	return SProcClearImageBufferArea (client);
    default:
	return BadRequest;
    }
}

static void
SUpdateNotifyEvent (from, to)
    xMbufUpdateNotifyEvent	*from, *to;
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->buffer, to->buffer);
    cpswapl (from->timeStamp, to->timeStamp);
}

static void
SClobberNotifyEvent (from, to)
    xMbufClobberNotifyEvent	*from, *to;
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->buffer, to->buffer);
    to->state = from->state;
}

RegionPtr CreateUnclippedWinSize(WindowPtr pWin);

RegionPtr
CreateUnclippedWinSize (WindowPtr pWin)
{
    RegionPtr	pRgn;
    BoxRec	box;

    box.x1 = pWin->drawable.x;
    box.y1 = pWin->drawable.y;
    box.x2 = pWin->drawable.x + (int) pWin->drawable.width;
    box.y2 = pWin->drawable.y + (int) pWin->drawable.height;
    pRgn = REGION_CREATE(pWin->drawable.pScreen, &box, 1);
    if (wBoundingShape (pWin) || wClipShape (pWin)) {
	ScreenPtr pScreen;
        pScreen = pWin->drawable.pScreen;

	REGION_TRANSLATE(pScreen, pRgn, - pWin->drawable.x,
			 - pWin->drawable.y);
	if (wBoundingShape (pWin))
	    REGION_INTERSECT(pScreen, pRgn, pRgn, wBoundingShape (pWin));
	if (wClipShape (pWin))
	    REGION_INTERSECT(pScreen, pRgn, pRgn, wClipShape (pWin));
	REGION_TRANSLATE(pScreen, pRgn, pWin->drawable.x, pWin->drawable.y);
    }
    return pRgn;
}

static void
PerformDisplayRequest (ppMultibuffers, pMultibuffer, nbuf)
    MultibufferPtr	    *pMultibuffer;
    MultibuffersPtr	    *ppMultibuffers;
    int		    nbuf;
{
    GCPtr	    pGC;
    PixmapPtr	    pPrevPixmap, pNewPixmap;
    xRectangle	    clearRect;
    WindowPtr	    pWin;
    RegionPtr	    pExposed;
    int		    i;
    MultibufferPtr  pPrevMultibuffer;
    XID		    graphicsExpose;

    UpdateCurrentTime ();
    for (i = 0; i < nbuf; i++)
    {
	pWin = ppMultibuffers[i]->pWindow;
	pGC = GetScratchGC (pWin->drawable.depth, pWin->drawable.pScreen);
	pPrevMultibuffer =
	   &ppMultibuffers[i]->buffers[ppMultibuffers[i]->displayedMultibuffer];
	pPrevPixmap = pPrevMultibuffer->pPixmap;
	pNewPixmap = pMultibuffer[i]->pPixmap;
	switch (ppMultibuffers[i]->updateAction)
	{
	case MultibufferUpdateActionUndefined:
	    break;
	case MultibufferUpdateActionBackground:
	    SetupBackgroundPainter (pWin, pGC);
	    ValidateGC ((DrawablePtr)pPrevPixmap, pGC);
	    clearRect.x = 0;
	    clearRect.y = 0;
	    clearRect.width = pPrevPixmap->drawable.width;
	    clearRect.height = pPrevPixmap->drawable.height;
	    (*pGC->ops->PolyFillRect) ((DrawablePtr)pPrevPixmap, pGC,
				       1, &clearRect);
	    break;
	case MultibufferUpdateActionUntouched:
	    /* copy the window to the pixmap that represents the
	     * currently displayed buffer
	     */
	    if (pPrevMultibuffer->eventMask & ExposureMask)
	    {
	    	graphicsExpose = TRUE;
	    	DoChangeGC (pGC, GCGraphicsExposures, &graphicsExpose, FALSE);
	    }
	    ValidateGC ((DrawablePtr)pPrevPixmap, pGC);
	    pExposed = (*pGC->ops->CopyArea)
			    ((DrawablePtr) pWin,
			     (DrawablePtr) pPrevPixmap,
			     pGC,
			     0, 0,
			     pWin->drawable.width, pWin->drawable.height,
			     0, 0);

	    /* if we couldn't copy the whole window to the buffer,
	     * send expose events (if any client wants them)
	     */
	    if (pPrevMultibuffer->eventMask & ExposureMask)
	    { /* some client wants expose events */
	    	if (pExposed)
	    	{
		    RegionPtr	pWinSize;

		    pWinSize = CreateUnclippedWinSize (pWin);
		    /* pExposed is window-relative, but at this point
		     * pWinSize is screen-relative.  Make pWinSize be
		     * window-relative so that region ops involving
		     * pExposed and pWinSize behave sensibly.
		     */
		    REGION_TRANSLATE(pWin->drawable.pScreen, pWinSize,
				     -pWin->drawable.x, -pWin->drawable.y);
		    REGION_INTERSECT(pWin->drawable.pScreen, pExposed,
				     pExposed, pWinSize);
		    REGION_DESTROY(pWin->drawable.pScreen, pWinSize);
	    	    MultibufferExpose (pPrevMultibuffer, pExposed);
	    	    REGION_DESTROY(pWin->drawable.pScreen, pExposed);
	    	}
	    	graphicsExpose = FALSE;
	    	DoChangeGC (pGC, GCGraphicsExposures, &graphicsExpose, FALSE);
	    }
	    break; /* end case MultibufferUpdateActionUntouched */

	case MultibufferUpdateActionCopied:
	    ValidateGC ((DrawablePtr)pPrevPixmap, pGC);
	    (*pGC->ops->CopyArea) ((DrawablePtr)pNewPixmap,
				   (DrawablePtr)pPrevPixmap, pGC,
				   0, 0,
				   pWin->drawable.width, pWin->drawable.height,
				   0, 0);
	    break;
	} /* end switch on update action */

	/* display the new buffer */
	ValidateGC ((DrawablePtr)pWin, pGC);
	(*pGC->ops->CopyArea) ((DrawablePtr)pNewPixmap, (DrawablePtr)pWin, pGC,
			       0, 0,
			       pWin->drawable.width, pWin->drawable.height,
			       0, 0);
	ppMultibuffers[i]->lastUpdate = currentTime;
	MultibufferUpdate (pMultibuffer[i],
			   ppMultibuffers[i]->lastUpdate.milliseconds);
	AliasMultibuffer (ppMultibuffers[i],
			  pMultibuffer[i] - ppMultibuffers[i]->buffers);
	FreeScratchGC (pGC);
    }
}

DrawablePtr
GetBufferPointer (pWin, i)
    WindowPtr	pWin;
    int		i;
{
    MultibuffersPtr pMultibuffers;

    if (!(pMultibuffers = (MultibuffersPtr) dixLookupPrivate(&pWin->devPrivates, MultibufferWindowPrivKey)))
	return NULL;
    return (DrawablePtr) pMultibuffers->buffers[i].pPixmap;
}

static Bool
QueueDisplayRequest (client, activateTime)
    ClientPtr	    client;
    TimeStamp	    activateTime;
{
    /* see xtest.c:ProcXTestFakeInput for code similar to this */

    if (!ClientSleepUntil(client, &activateTime, NULL, NULL))
    {
	return FALSE;
    }
    /* swap the request back so we can simply re-execute it */
    if (client->swapped)
    {
    	int    n;
    	REQUEST (xMbufDisplayImageBuffersReq);
    	
    	SwapRestL(stuff);
    	swaps (&stuff->length, n);
    	swaps (&stuff->minDelay, n);
    	swaps (&stuff->maxDelay, n);
    }
    ResetCurrentRequest (client);
    client->sequence--;
    return TRUE;
}


/*
 * Deliver events to a buffer
 */

static int
DeliverEventsToMultibuffer (pMultibuffer, pEvents, count, filter)
    MultibufferPtr	pMultibuffer;
    xEvent	*pEvents;
    int		count;
    Mask	filter;
{
    int deliveries = 0, nondeliveries = 0;
    int attempt;
    OtherClients *other;

    /* if nobody wants the event, we're done */
    if (!((pMultibuffer->otherEventMask|pMultibuffer->eventMask) & filter))
	return 0;

    /* maybe send event to owner */
    if ((attempt = TryClientEvents(
	bClient(pMultibuffer), NULL, pEvents, count, pMultibuffer->eventMask, filter, (GrabPtr) 0)) != 0)
    {
	if (attempt > 0)
	    deliveries++;
	else
	    nondeliveries--;
    }

    /* maybe send event to other clients */
    for (other = pMultibuffer->otherClients; other; other=other->next)
    {
	if ((attempt = TryClientEvents(
	      rClient(other), NULL, pEvents, count, other->mask, filter, (GrabPtr) 0)) != 0)
	{
	    if (attempt > 0)
		deliveries++;
	    else
		nondeliveries--;
	}
    }
    if (deliveries)
	return deliveries;
    return nondeliveries;
}

/*
 * Send Expose events to interested clients
 */

void
MultibufferExpose (pMultibuffer, pRegion)
    MultibufferPtr	pMultibuffer;
    RegionPtr	pRegion;
{
    if (pRegion && !REGION_NIL(pRegion))
    {
	xEvent *pEvent;
	PixmapPtr   pPixmap;
	xEvent *pe;
	BoxPtr pBox;
	int i;
	int numRects;

	pPixmap = pMultibuffer->pPixmap;
	REGION_TRANSLATE(pPixmap->drawable.pScreen, pRegion,
		    -pPixmap->drawable.x, -pPixmap->drawable.y);
	/* XXX MultibufferExpose "knows" the region representation */
	numRects = REGION_NUM_RECTS(pRegion);
	pBox = REGION_RECTS(pRegion);

	pEvent = xalloc(numRects * sizeof(xEvent));
	if (pEvent) {
	    pe = pEvent;

	    for (i=1; i<=numRects; i++, pe++, pBox++)
	    {
		pe->u.u.type = Expose;
		pe->u.expose.window = pPixmap->drawable.id;
		pe->u.expose.x = pBox->x1;
		pe->u.expose.y = pBox->y1;
		pe->u.expose.width = pBox->x2 - pBox->x1;
		pe->u.expose.height = pBox->y2 - pBox->y1;
		pe->u.expose.count = (numRects - i);
	    }
	    (void) DeliverEventsToMultibuffer (pMultibuffer, pEvent, numRects,
					       ExposureMask);
	    xfree(pEvent);
	}
    }
}

/* send UpdateNotify event */
void
MultibufferUpdate (pMultibuffer, time2)
    MultibufferPtr	pMultibuffer;
    CARD32	time2;
{
    xMbufUpdateNotifyEvent	event;

    event.type = MultibufferEventBase + MultibufferUpdateNotify;
    event.buffer = pMultibuffer->pPixmap->drawable.id;
    event.timeStamp = time2;
    (void) DeliverEventsToMultibuffer (pMultibuffer, (xEvent *)&event,
				1, (Mask)MultibufferUpdateNotifyMask);
}

/*
 * The sample implementation will never generate MultibufferClobberNotify
 * events
 */

void
MultibufferClobber (pMultibuffer)
    MultibufferPtr	pMultibuffer;
{
    xMbufClobberNotifyEvent	event;

    event.type = MultibufferEventBase + MultibufferClobberNotify;
    event.buffer = pMultibuffer->pPixmap->drawable.id;
    event.state = pMultibuffer->clobber;
    (void) DeliverEventsToMultibuffer (pMultibuffer, (xEvent *)&event,
				1, (Mask)MultibufferClobberNotifyMask);
}

/*
 * make the resource id for buffer i refer to the window
 * drawable instead of the pixmap;
 */

static void
AliasMultibuffer (pMultibuffers, i)
    MultibuffersPtr	pMultibuffers;
    int		i;
{
    MultibufferPtr	pMultibuffer;

    if (i == pMultibuffers->displayedMultibuffer)
	return;
    /*
     * remove the old association
     */
    if (pMultibuffers->displayedMultibuffer >= 0)
    {
	pMultibuffer = &pMultibuffers->buffers[pMultibuffers->displayedMultibuffer];
	ChangeResourceValue (pMultibuffer->pPixmap->drawable.id,
			     MultibufferDrawableResType,
 			     (pointer) pMultibuffer->pPixmap);
    }
    /*
     * make the new association
     */
    pMultibuffer = &pMultibuffers->buffers[i];
    ChangeResourceValue (pMultibuffer->pPixmap->drawable.id,
			 MultibufferDrawableResType,
			 (pointer) pMultibuffers->pWindow);
    pMultibuffers->displayedMultibuffer = i;
}

/*
 * free everything associated with multibuffering for this
 * window
 */

void
DestroyImageBuffers (pWin)
    WindowPtr	pWin;
{
    FreeResourceByType (pWin->drawable.id, MultibuffersResType, FALSE);
    /* Zero out the window's pointer to the buffers so they won't be reused */
    dixSetPrivate(&pWin->devPrivates, MultibufferWindowPrivKey, NULL);
}

/*
 * resize the buffers when the window is resized
 */ 

static Bool
MultibufferPositionWindow (pWin, x, y)
    WindowPtr	pWin;
    int		x, y;
{
    ScreenPtr	    pScreen;
    MultibufferScreenPtr pMultibufferScreen;
    MultibuffersPtr	    pMultibuffers;
    MultibufferPtr	    pMultibuffer;
    int		    width, height;
    int		    i;
    int		    dx, dy, dw, dh;
    int		    sourcex, sourcey;
    int		    destx, desty;
    PixmapPtr	    pPixmap;
    GCPtr	    pGC;
    int		    savewidth, saveheight;
    xRectangle	    clearRect;
    Bool	    clear;

    pScreen = pWin->drawable.pScreen;
    pMultibufferScreen = (MultibufferScreenPtr) dixLookupPrivate(&pScreen->devPrivates, MultibufferScreenPrivKey);
    (*pMultibufferScreen->PositionWindow) (pWin, x, y);

    /* if this window is not multibuffered, we're done */
    if (!(pMultibuffers = (MultibuffersPtr) dixLookupPrivate(&pWin->devPrivates, MultibufferWindowPrivKey)))
	return TRUE;

    /* if new size is same as old, we're done */
    if (pMultibuffers->width == pWin->drawable.width &&
        pMultibuffers->height == pWin->drawable.height)
	return TRUE;

    width = pWin->drawable.width;
    height = pWin->drawable.height;
    dx = pWin->drawable.x - pMultibuffers->x;
    dy = pWin->drawable.x - pMultibuffers->y;
    dw = width - pMultibuffers->width;
    dh = height - pMultibuffers->height;
    GravityTranslate (0, 0, -dx, -dy, dw, dh,
		      pWin->bitGravity, &destx, &desty);

    /* if the window grew, remember to paint the window background,
     * and maybe send expose events, for the new areas of the buffers
     */
    clear = pMultibuffers->width < width || pMultibuffers->height < height ||
		pWin->bitGravity == ForgetGravity;

    sourcex = 0;
    sourcey = 0;
    savewidth = pMultibuffers->width;
    saveheight = pMultibuffers->height;
    /* clip rectangle to source and destination */
    if (destx < 0)
    {
	savewidth += destx;
	sourcex -= destx;
	destx = 0;
    }
    if (destx + savewidth > width)
	savewidth = width - destx;
    if (desty < 0)
    {
	saveheight += desty;
	sourcey -= desty;
	desty = 0;
    }
    if (desty + saveheight > height)
	saveheight = height - desty;

    pMultibuffers->width = width;
    pMultibuffers->height = height;
    pMultibuffers->x = pWin->drawable.x;
    pMultibuffers->y = pWin->drawable.y;

    pGC = GetScratchGC (pWin->drawable.depth, pScreen);
    if (clear)
    {
	SetupBackgroundPainter (pWin, pGC);
	clearRect.x = 0;
	clearRect.y = 0;
	clearRect.width = width;
	clearRect.height = height;
    }
    for (i = 0; i < pMultibuffers->numMultibuffer; i++)
    {
	pMultibuffer = &pMultibuffers->buffers[i];
	pPixmap = (*pScreen->CreatePixmap) (pScreen, width, height,
					    pWin->drawable.depth,
					    CREATE_PIXMAP_USAGE_SCRATCH);
	if (!pPixmap)
	{
	    DestroyImageBuffers (pWin);
	    break;
	}
	ValidateGC ((DrawablePtr)pPixmap, pGC);
	/*
	 * I suppose this could avoid quite a bit of work if
	 * it computed the minimal area required.
	 */
	if (clear)
	    (*pGC->ops->PolyFillRect) ((DrawablePtr)pPixmap, pGC, 1, &clearRect);
	if (pWin->bitGravity != ForgetGravity)
	{
	    (*pGC->ops->CopyArea) ((DrawablePtr)pMultibuffer->pPixmap,
				   (DrawablePtr)pPixmap, pGC,
				    sourcex, sourcey, savewidth, saveheight,
				    destx, desty);
	}
	pPixmap->drawable.id = pMultibuffer->pPixmap->drawable.id;
	(*pScreen->DestroyPixmap) (pMultibuffer->pPixmap);
	pMultibuffer->pPixmap = pPixmap;
	if (i != pMultibuffers->displayedMultibuffer)
	{
	    ChangeResourceValue (pPixmap->drawable.id,
				 MultibufferDrawableResType,
				 (pointer) pPixmap);
	}
    }
    FreeScratchGC (pGC);
    return TRUE;
}

/* Resource delete func for MultibufferDrawableResType */
/*ARGSUSED*/
static int
MultibufferDrawableDelete (value, id)
    pointer	value;
    XID		id;
{
    DrawablePtr	pDrawable = (DrawablePtr)value;
    WindowPtr	pWin;
    MultibuffersPtr	pMultibuffers;
    PixmapPtr	pPixmap;

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pWin = (WindowPtr) pDrawable;
	pMultibuffers = (MultibuffersPtr) dixLookupPrivate(&pWin->devPrivates, MultibufferWindowPrivKey);
	pPixmap = pMultibuffers->buffers[pMultibuffers->displayedMultibuffer].pPixmap;
    }
    else
    {
	pPixmap = (PixmapPtr) pDrawable;
    }
    (*pPixmap->drawable.pScreen->DestroyPixmap) (pPixmap);
    return Success;
}

/* Resource delete func for MultibufferResType */
/*ARGSUSED*/
static int
MultibufferDelete (value, id)
    pointer	value;
    XID		id;
{
    MultibufferPtr	pMultibuffer = (MultibufferPtr)value;
    MultibuffersPtr	pMultibuffers;

    pMultibuffers = pMultibuffer->pMultibuffers;
    if (--pMultibuffers->refcnt == 0)
    {
	FreeResourceByType (pMultibuffers->pWindow->drawable.id,
			    MultibuffersResType, TRUE);
	xfree (pMultibuffers);
    }
    return Success;
}

/* Resource delete func for MultibuffersResType */
/*ARGSUSED*/
static int
MultibuffersDelete (value, id)
    pointer	value;
    XID		id;
{
    MultibuffersPtr	pMultibuffers = (MultibuffersPtr)value;
    int	i;

    if (pMultibuffers->refcnt == pMultibuffers->numMultibuffer)
    {
	for (i = pMultibuffers->numMultibuffer; --i >= 0; )
	    FreeResource (pMultibuffers->buffers[i].pPixmap->drawable.id, 0);
    }
    return Success;
}

/* Resource delete func for OtherClientResType */
static int
OtherClientDelete (value, id)
    pointer	value;
    XID		id;
{
    MultibufferPtr	pMultibuffer = (MultibufferPtr)value;
    OtherClientsPtr	other, prev;

    prev = 0;
    for (other = pMultibuffer->otherClients; other; other = other->next)
    {
	if (other->resource == id)
	{
	    if (prev)
		prev->next = other->next;
	    else
		pMultibuffer->otherClients = other->next;
	    xfree (other);
	    RecalculateMultibufferOtherEvents (pMultibuffer);
	    break;
	}
	prev = other;
    }
    return Success;
}

static int
EventSelectForMultibuffer (pMultibuffer, client, mask)
    MultibufferPtr	pMultibuffer;
    ClientPtr	client;
    Mask	mask;
{
    OtherClientsPtr	other;

    if (mask & ~ValidEventMasks)
    {
	client->errorValue = mask;
	return BadValue;
    }
    if (bClient (pMultibuffer) == client)
    {
	pMultibuffer->eventMask = mask;
    }
    else /* some other client besides the creator wants events */
    {
	for (other = pMultibuffer->otherClients; other; other = other->next)
	{
	    if (SameClient (other, client))
	    {
		if (mask == 0)
		{
		    FreeResource (other->resource, RT_NONE);
		    break;
		}
		other->mask = mask;
		break;
	    }
	}
	if (!other)
	{ /* new client that never selected events on this buffer before */
	    other = xalloc (sizeof (OtherClients));
	    if (!other)
		return BadAlloc;
	    other->mask = mask;
	    other->resource = FakeClientID (client->index);
	    if (!AddResource (other->resource, OtherClientResType, (pointer) pMultibuffer))
	    {
		xfree (other);
		return BadAlloc;
	    }
	    other->next = pMultibuffer->otherClients;
	    pMultibuffer->otherClients = other;
	}
	RecalculateMultibufferOtherEvents (pMultibuffer);
    }
    return (client->noClientException);
}

/* or together all the otherClients event masks */
static void
RecalculateMultibufferOtherEvents (pMultibuffer)
    MultibufferPtr	pMultibuffer;
{
    Mask	    otherEventMask;
    OtherClients    *other;

    otherEventMask = 0L;
    for (other = pMultibuffer->otherClients; other; other = other->next)
	otherEventMask |= other->mask;
    pMultibuffer->otherEventMask = otherEventMask;
}

/* add milliseconds to a timestamp, handling overflow */
static void
BumpTimeStamp (ts, inc)
TimeStamp   *ts;
CARD32	    inc;
{
    CARD32  newms;

    newms = ts->milliseconds + inc;
    if (newms < ts->milliseconds)
	ts->months++;
    ts->milliseconds = newms;
}
