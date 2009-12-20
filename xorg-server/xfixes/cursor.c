/*
 * Copyright © 2006 Sun Microsystems, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Copyright © 2002 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "xfixesint.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "dixevents.h"
#include "servermd.h"
#include "inputstr.h"
#include "windowstr.h"
#include "xace.h"

static RESTYPE		CursorClientType;
static RESTYPE		CursorHideCountType;
static RESTYPE		CursorWindowType;
static CursorPtr	CursorCurrent[MAXDEVICES];
static CursorPtr        pInvisibleCursor = NULL;

static int CursorScreenPrivateKeyIndex;
static DevPrivateKey CursorScreenPrivateKey = &CursorScreenPrivateKeyIndex;

static void deleteCursorHideCountsForScreen (ScreenPtr pScreen);

#define VERIFY_CURSOR(pCursor, cursor, client, access)			\
    do {								\
	int err;							\
	err = dixLookupResourceByType((pointer *) &pCursor, cursor,	\
				      RT_CURSOR, client, access);	\
	if (err == BadValue) {						\
	    client->errorValue = cursor;				\
	    return BadCursor;						\
	} else if (err != Success) {					\
	    client->errorValue = cursor;				\
	    return err;							\
	}								\
    } while (0)

/*
 * There is a global list of windows selecting for cursor events
 */

typedef struct _CursorEvent *CursorEventPtr;

typedef struct _CursorEvent {
    CursorEventPtr	next;
    CARD32		eventMask;
    ClientPtr		pClient;
    WindowPtr		pWindow;
    XID			clientResource;
} CursorEventRec;

static CursorEventPtr	    cursorEvents;

/*
 * Each screen has a list of clients which have requested
 * that the cursor be hid, and the number of times each
 * client has requested.
*/

typedef struct _CursorHideCountRec *CursorHideCountPtr;

typedef struct _CursorHideCountRec {
    CursorHideCountPtr   pNext;  
    ClientPtr            pClient;
    ScreenPtr            pScreen;
    int                  hideCount;
    XID			 resource;
} CursorHideCountRec;

/*
 * Wrap DisplayCursor to catch cursor change events
 */

typedef struct _CursorScreen {
    DisplayCursorProcPtr	DisplayCursor;
    CloseScreenProcPtr		CloseScreen;
    CursorHideCountPtr          pCursorHideCounts;
} CursorScreenRec, *CursorScreenPtr;

#define GetCursorScreen(s) ((CursorScreenPtr)dixLookupPrivate(&(s)->devPrivates, CursorScreenPrivateKey))
#define GetCursorScreenIfSet(s) GetCursorScreen(s)
#define SetCursorScreen(s,p) dixSetPrivate(&(s)->devPrivates, CursorScreenPrivateKey, p)
#define Wrap(as,s,elt,func)	(((as)->elt = (s)->elt), (s)->elt = func)
#define Unwrap(as,s,elt,backup)	(((backup) = (s)->elt), (s)->elt = (as)->elt)

/* The cursor doesn't show up until the first XDefineCursor() */
static Bool CursorVisible = FALSE;

Bool EnableCursor = TRUE;

static Bool
CursorDisplayCursor (DeviceIntPtr pDev,
                     ScreenPtr pScreen,
		     CursorPtr pCursor)
{
    CursorScreenPtr	cs = GetCursorScreen(pScreen);
    Bool		ret;
    DisplayCursorProcPtr backupProc;

    Unwrap (cs, pScreen, DisplayCursor, backupProc);

    /*
     * Have to check ConnectionInfo to distinguish client requests from
     * initial root window setup.  Not a great way to do it, I admit.
     */
    if (ConnectionInfo)
	CursorVisible = EnableCursor;

    if (cs->pCursorHideCounts != NULL || !CursorVisible) {
        ret = ((*pScreen->RealizeCursor)(pDev, pScreen, pInvisibleCursor) &&
	       (*pScreen->DisplayCursor) (pDev, pScreen, pInvisibleCursor));
    } else {
	ret = (*pScreen->DisplayCursor) (pDev, pScreen, pCursor);
    }

    if (pCursor != CursorCurrent[pDev->id])
    {
	CursorEventPtr	e;

	CursorCurrent[pDev->id] = pCursor;
	for (e = cursorEvents; e; e = e->next)
	{
	    if ((e->eventMask & XFixesDisplayCursorNotifyMask) &&
		!e->pClient->clientGone)
	    {
		xXFixesCursorNotifyEvent	ev;
		ev.type = XFixesEventBase + XFixesCursorNotify;
		ev.subtype = XFixesDisplayCursorNotify;
		ev.sequenceNumber = e->pClient->sequence;
		ev.window = e->pWindow->drawable.id;
		ev.cursorSerial = pCursor->serialNumber;
		ev.timestamp = currentTime.milliseconds;
		ev.name = pCursor->name;
		WriteEventsToClient (e->pClient, 1, (xEvent *) &ev);
	    }
	}
    }
    Wrap (cs, pScreen, DisplayCursor, backupProc);

    return ret;
}

static Bool
CursorCloseScreen (int index, ScreenPtr pScreen)
{
    CursorScreenPtr	cs = GetCursorScreen (pScreen);
    Bool		ret;
    CloseScreenProcPtr	close_proc;
    DisplayCursorProcPtr display_proc;

    Unwrap (cs, pScreen, CloseScreen, close_proc);
    Unwrap (cs, pScreen, DisplayCursor, display_proc);
    deleteCursorHideCountsForScreen(pScreen);
    ret = (*pScreen->CloseScreen) (index, pScreen);
    xfree (cs);
    return ret;
}

#define CursorAllEvents (XFixesDisplayCursorNotifyMask)

static int
XFixesSelectCursorInput (ClientPtr	pClient,
			 WindowPtr	pWindow,
			 CARD32		eventMask)
{
    CursorEventPtr	*prev, e;
    pointer val;
    int rc;

    for (prev = &cursorEvents; (e = *prev); prev = &e->next)
    {
	if (e->pClient == pClient &&
	    e->pWindow == pWindow)
	{
	    break;
	}
    }
    if (!eventMask)
    {
	if (e)
	{
	    FreeResource (e->clientResource, 0);
	}
	return Success;
    }
    if (!e)
    {
	e = (CursorEventPtr) xalloc (sizeof (CursorEventRec));
	if (!e)
	    return BadAlloc;

	e->next = 0;
	e->pClient = pClient;
	e->pWindow = pWindow;
	e->clientResource = FakeClientID(pClient->index);

	/*
	 * Add a resource hanging from the window to
	 * catch window destroy
	 */
	rc = dixLookupResourceByType( &val, pWindow->drawable.id,
				      CursorWindowType, serverClient,
				      DixGetAttrAccess);
	if (rc != Success)
	    if (!AddResource (pWindow->drawable.id, CursorWindowType,
			      (pointer) pWindow))
	    {
		xfree (e);
		return BadAlloc;
	    }

	if (!AddResource (e->clientResource, CursorClientType, (pointer) e))
	    return BadAlloc;

	*prev = e;
    }
    e->eventMask = eventMask;
    return Success;
}

int
ProcXFixesSelectCursorInput (ClientPtr client)
{
    REQUEST (xXFixesSelectCursorInputReq);
    WindowPtr	pWin;
    int		rc;

    REQUEST_SIZE_MATCH (xXFixesSelectCursorInputReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixGetAttrAccess);
    if (rc != Success)
        return rc;
    if (stuff->eventMask & ~CursorAllEvents)
    {
	client->errorValue = stuff->eventMask;
	return( BadValue );
    }
    return XFixesSelectCursorInput (client, pWin, stuff->eventMask);
}

static int
GetBit (unsigned char *line, int x)
{
    unsigned char   mask;
    
    if (screenInfo.bitmapBitOrder == LSBFirst)
	mask = (1 << (x & 7));
    else
	mask = (0x80 >> (x & 7));
    /* XXX assumes byte order is host byte order */
    line += (x >> 3);
    if (*line & mask)
	return 1;
    return 0;
}

int
SProcXFixesSelectCursorInput (ClientPtr client)
{
    register int n;
    REQUEST(xXFixesSelectCursorInputReq);

    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swapl(&stuff->eventMask, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}
    
void
SXFixesCursorNotifyEvent (xXFixesCursorNotifyEvent *from,
			  xXFixesCursorNotifyEvent *to)
{
    to->type = from->type;
    cpswaps (from->sequenceNumber, to->sequenceNumber);
    cpswapl (from->window, to->window);
    cpswapl (from->cursorSerial, to->cursorSerial);
    cpswapl (from->timestamp, to->timestamp);
    cpswapl (from->name, to->name);
}

static void
CopyCursorToImage (CursorPtr pCursor, CARD32 *image)
{
    int width = pCursor->bits->width;
    int height = pCursor->bits->height;
    int npixels = width * height;
    
#ifdef ARGB_CURSOR
    if (pCursor->bits->argb)
	memcpy (image, pCursor->bits->argb, npixels * sizeof (CARD32));
    else
#endif
    {
	unsigned char	*srcLine = pCursor->bits->source;
	unsigned char	*mskLine = pCursor->bits->mask;
	int		stride = BitmapBytePad (width);
	int		x, y;
	CARD32		fg, bg;
	
	fg = (0xff000000 | 
	      ((pCursor->foreRed & 0xff00) << 8) |
	      (pCursor->foreGreen & 0xff00) |
	      (pCursor->foreBlue >> 8));
	bg = (0xff000000 | 
	      ((pCursor->backRed & 0xff00) << 8) |
	      (pCursor->backGreen & 0xff00) |
	      (pCursor->backBlue >> 8));
	for (y = 0; y < height; y++)
	{
	    for (x = 0; x < width; x++)
	    {
		if (GetBit (mskLine, x))
		{
		    if (GetBit (srcLine, x))
			*image++ = fg;
		    else
			*image++ = bg;
		}
		else
		    *image++ = 0;
	    }
	    srcLine += stride;
	    mskLine += stride;
	}
    }
}

int
ProcXFixesGetCursorImage (ClientPtr client)
{
/*    REQUEST(xXFixesGetCursorImageReq); */
    xXFixesGetCursorImageReply	*rep;
    CursorPtr			pCursor;
    CARD32			*image;
    int				npixels, width, height, rc, x, y;

    REQUEST_SIZE_MATCH(xXFixesGetCursorImageReq);
    pCursor = CursorCurrent[PickPointer(client)->id];
    if (!pCursor)
	return BadCursor;
    rc = XaceHook(XACE_RESOURCE_ACCESS, client, pCursor->id, RT_CURSOR,
		  pCursor, RT_NONE, NULL, DixReadAccess);
    if (rc != Success)
	return rc;
    GetSpritePosition (PickPointer(client), &x, &y);
    width = pCursor->bits->width;
    height = pCursor->bits->height;
    npixels = width * height;
    rep = xalloc (sizeof (xXFixesGetCursorImageReply) +
		  npixels * sizeof (CARD32));
    if (!rep)
	return BadAlloc;

    rep->type = X_Reply;
    rep->sequenceNumber = client->sequence;
    rep->length = npixels;
    rep->width = width;
    rep->height = height;
    rep->x = x;
    rep->y = y;
    rep->xhot = pCursor->bits->xhot;
    rep->yhot = pCursor->bits->yhot; 
    rep->cursorSerial = pCursor->serialNumber;

    image = (CARD32 *) (rep + 1);
    CopyCursorToImage (pCursor, image);
    if (client->swapped)
    {
	int n;
	swaps (&rep->sequenceNumber, n);
	swapl (&rep->length, n);
	swaps (&rep->x, n);
	swaps (&rep->y, n);
	swaps (&rep->width, n);
	swaps (&rep->height, n);
	swaps (&rep->xhot, n);
	swaps (&rep->yhot, n);
	swapl (&rep->cursorSerial, n);
	SwapLongs (image, npixels);
    }
    WriteToClient(client, sizeof (xXFixesGetCursorImageReply) +
			 (npixels << 2), (char *) rep);
    xfree (rep);
    return client->noClientException;
}

int
SProcXFixesGetCursorImage (ClientPtr client)
{
    int n;
    REQUEST(xXFixesGetCursorImageReq);
    swaps (&stuff->length, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesSetCursorName (ClientPtr client)
{
    CursorPtr pCursor;
    char *tchar;
    REQUEST(xXFixesSetCursorNameReq);
    Atom atom;

    REQUEST_AT_LEAST_SIZE(xXFixesSetCursorNameReq);
    VERIFY_CURSOR(pCursor, stuff->cursor, client, DixSetAttrAccess);
    tchar = (char *) &stuff[1];
    atom = MakeAtom (tchar, stuff->nbytes, TRUE);
    if (atom == BAD_RESOURCE)
	return BadAlloc;
    
    pCursor->name = atom;
    return(client->noClientException);
}

int
SProcXFixesSetCursorName (ClientPtr client)
{
    int n;
    REQUEST(xXFixesSetCursorNameReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xXFixesSetCursorNameReq);
    swapl (&stuff->cursor, n);
    swaps (&stuff->nbytes, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesGetCursorName (ClientPtr client)
{
    CursorPtr			pCursor;
    xXFixesGetCursorNameReply	reply;
    REQUEST(xXFixesGetCursorNameReq);
    const char *str;
    int len;

    REQUEST_SIZE_MATCH(xXFixesGetCursorNameReq);
    VERIFY_CURSOR(pCursor, stuff->cursor, client, DixGetAttrAccess);
    if (pCursor->name)
	str = NameForAtom (pCursor->name);
    else
	str = "";
    len = strlen (str);
    
    reply.type = X_Reply;
    reply.length = bytes_to_int32(len);
    reply.sequenceNumber = client->sequence;
    reply.atom = pCursor->name;
    reply.nbytes = len;
    if (client->swapped)
    {
	int n;
	swaps (&reply.sequenceNumber, n);
	swapl (&reply.length, n);
	swapl (&reply.atom, n);
	swaps (&reply.nbytes, n);
    }
    WriteReplyToClient(client, sizeof(xXFixesGetCursorNameReply), &reply);
    WriteToClient(client, len, str);
    
    return(client->noClientException);
}

int
SProcXFixesGetCursorName (ClientPtr client)
{
    int n;
    REQUEST(xXFixesGetCursorNameReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesGetCursorNameReq);
    swapl (&stuff->cursor, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesGetCursorImageAndName (ClientPtr client)
{
/*    REQUEST(xXFixesGetCursorImageAndNameReq); */
    xXFixesGetCursorImageAndNameReply	*rep;
    CursorPtr			pCursor;
    CARD32			*image;
    int				npixels;
    const char			*name;
    int				nbytes, nbytesRound;
    int				width, height;
    int				rc, x, y;

    REQUEST_SIZE_MATCH(xXFixesGetCursorImageAndNameReq);
    pCursor = CursorCurrent[PickPointer(client)->id];
    if (!pCursor)
	return BadCursor;
    rc = XaceHook(XACE_RESOURCE_ACCESS, client, pCursor->id, RT_CURSOR,
		  pCursor, RT_NONE, NULL, DixReadAccess|DixGetAttrAccess);
    if (rc != Success)
	return rc;
    GetSpritePosition (PickPointer(client), &x, &y);
    width = pCursor->bits->width;
    height = pCursor->bits->height;
    npixels = width * height;
    name = pCursor->name ? NameForAtom (pCursor->name) : "";
    nbytes = strlen (name);
    nbytesRound = pad_to_int32(nbytes);
    rep = xalloc (sizeof (xXFixesGetCursorImageAndNameReply) +
		  npixels * sizeof (CARD32) + nbytesRound);
    if (!rep)
	return BadAlloc;

    rep->type = X_Reply;
    rep->sequenceNumber = client->sequence;
    rep->length = npixels + bytes_to_int32(nbytesRound);
    rep->width = width;
    rep->height = height;
    rep->x = x;
    rep->y = y;
    rep->xhot = pCursor->bits->xhot;
    rep->yhot = pCursor->bits->yhot; 
    rep->cursorSerial = pCursor->serialNumber;
    rep->cursorName = pCursor->name;
    rep->nbytes = nbytes;

    image = (CARD32 *) (rep + 1);
    CopyCursorToImage (pCursor, image);
    memcpy ((image + npixels), name, nbytes);
    if (client->swapped)
    {
	int n;
	swaps (&rep->sequenceNumber, n);
	swapl (&rep->length, n);
	swaps (&rep->x, n);
	swaps (&rep->y, n);
	swaps (&rep->width, n);
	swaps (&rep->height, n);
	swaps (&rep->xhot, n);
	swaps (&rep->yhot, n);
	swapl (&rep->cursorSerial, n);
	swapl (&rep->cursorName, n);
	swaps (&rep->nbytes, n);
	SwapLongs (image, npixels);
    }
    WriteToClient(client, sizeof (xXFixesGetCursorImageAndNameReply) +
			 (npixels << 2) + nbytesRound, (char *) rep);
    xfree (rep);
    return client->noClientException;
}

int
SProcXFixesGetCursorImageAndName (ClientPtr client)
{
    int n;
    REQUEST(xXFixesGetCursorImageAndNameReq);
    swaps (&stuff->length, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

/*
 * Find every cursor reference in the system, ask testCursor
 * whether it should be replaced with a reference to pCursor.
 */

typedef Bool (*TestCursorFunc) (CursorPtr pOld, pointer closure);

typedef struct {
    RESTYPE type;
    TestCursorFunc testCursor;
    CursorPtr pNew;
    pointer closure;
} ReplaceCursorLookupRec, *ReplaceCursorLookupPtr;

static const RESTYPE    CursorRestypes[] = {
    RT_WINDOW, RT_PASSIVEGRAB, RT_CURSOR
};

#define NUM_CURSOR_RESTYPES (sizeof (CursorRestypes) / sizeof (CursorRestypes[0]))

static Bool
ReplaceCursorLookup (pointer value, XID id, pointer closure)
{
    ReplaceCursorLookupPtr  rcl = (ReplaceCursorLookupPtr) closure;
    WindowPtr		    pWin;
    GrabPtr		    pGrab;
    CursorPtr		    pCursor = 0, *pCursorRef = 0;
    XID			    cursor = 0;

    switch (rcl->type) {
    case RT_WINDOW:
	pWin = (WindowPtr) value;
	if (pWin->optional)
	{
	    pCursorRef = &pWin->optional->cursor;
	    pCursor = *pCursorRef;
	}
	break;
    case RT_PASSIVEGRAB:
	pGrab = (GrabPtr) value;
	pCursorRef = &pGrab->cursor;
	pCursor = *pCursorRef;
	break;
    case RT_CURSOR:
	pCursorRef = 0;
	pCursor = (CursorPtr) value;
	cursor = id;
	break;
    }
    if (pCursor && pCursor != rcl->pNew)
    {
	if ((*rcl->testCursor) (pCursor, rcl->closure))
	{
	    rcl->pNew->refcnt++;
	    /* either redirect reference or update resource database */
	    if (pCursorRef)
		*pCursorRef = rcl->pNew;
	    else
		ChangeResourceValue (id, RT_CURSOR, rcl->pNew);
	    FreeCursor (pCursor, cursor);
	}
    }
    return FALSE;   /* keep walking */
}

static void
ReplaceCursor (CursorPtr pCursor,
	       TestCursorFunc testCursor,
	       pointer closure)
{
    int	clientIndex;
    int resIndex;
    ReplaceCursorLookupRec  rcl;
    
    /* 
     * Cursors exist only in the resource database, windows and grabs.
     * All of these are always pointed at by the resource database.  Walk
     * the whole thing looking for cursors
     */
    rcl.testCursor = testCursor;
    rcl.pNew = pCursor;
    rcl.closure = closure;

    /* for each client */
    for (clientIndex = 0; clientIndex < currentMaxClients; clientIndex++)
    {
	if (!clients[clientIndex])
	    continue;
	for (resIndex = 0; resIndex < NUM_CURSOR_RESTYPES; resIndex++)
	{
	    rcl.type = CursorRestypes[resIndex];
	    /*
	     * This function walks the entire client resource database
	     */
	    LookupClientResourceComplex (clients[clientIndex], 
					 rcl.type, 
					 ReplaceCursorLookup,
					 (pointer) &rcl);
	}
    }
    /* this "knows" that WindowHasNewCursor doesn't depend on it's argument */
    WindowHasNewCursor (WindowTable[0]);
}

static Bool 
TestForCursor (CursorPtr pCursor, pointer closure)
{
    return (pCursor == (CursorPtr) closure);
}

int
ProcXFixesChangeCursor (ClientPtr client)
{
    CursorPtr	pSource, pDestination;
    REQUEST(xXFixesChangeCursorReq);

    REQUEST_SIZE_MATCH(xXFixesChangeCursorReq);
    VERIFY_CURSOR (pSource, stuff->source, client,
		   DixReadAccess|DixGetAttrAccess);
    VERIFY_CURSOR (pDestination, stuff->destination, client,
		   DixWriteAccess|DixSetAttrAccess);

    ReplaceCursor (pSource, TestForCursor, (pointer) pDestination);
    return (client->noClientException);
}

int
SProcXFixesChangeCursor (ClientPtr client)
{
    int n;
    REQUEST(xXFixesChangeCursorReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesChangeCursorReq);
    swapl (&stuff->source, n);
    swapl (&stuff->destination, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

static Bool
TestForCursorName (CursorPtr pCursor, pointer closure)
{
    Atom *pName = closure;
    return (pCursor->name == *pName);
}

int
ProcXFixesChangeCursorByName (ClientPtr client)
{
    CursorPtr	pSource;
    Atom	name;
    char	*tchar;
    REQUEST(xXFixesChangeCursorByNameReq);

    REQUEST_FIXED_SIZE(xXFixesChangeCursorByNameReq, stuff->nbytes);
    VERIFY_CURSOR(pSource, stuff->source, client,
		  DixReadAccess|DixGetAttrAccess);
    tchar = (char *) &stuff[1];
    name = MakeAtom (tchar, stuff->nbytes, FALSE);
    if (name)
	ReplaceCursor (pSource, TestForCursorName, &name);
    return (client->noClientException);
}

int
SProcXFixesChangeCursorByName (ClientPtr client)
{
    int n;
    REQUEST(xXFixesChangeCursorByNameReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE (xXFixesChangeCursorByNameReq);
    swapl (&stuff->source, n);
    swaps (&stuff->nbytes, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

/*
 * Routines for manipulating the per-screen hide counts list.
 * This list indicates which clients have requested cursor hiding 
 * for that screen.
 */

/* Return the screen's hide-counts list element for the given client */
static CursorHideCountPtr
findCursorHideCount (ClientPtr pClient, ScreenPtr pScreen) 
{
    CursorScreenPtr    cs = GetCursorScreen(pScreen);
    CursorHideCountPtr pChc;

    for (pChc = cs->pCursorHideCounts; pChc != NULL; pChc = pChc->pNext) {
	if (pChc->pClient == pClient) {
	    return pChc;
	}
    }

    return NULL;           
}

static int
createCursorHideCount (ClientPtr pClient, ScreenPtr pScreen)
{
    CursorScreenPtr    cs = GetCursorScreen(pScreen);
    CursorHideCountPtr pChc;

    pChc = (CursorHideCountPtr) xalloc(sizeof(CursorHideCountRec));
    if (pChc == NULL) {
	return BadAlloc;
    }
    pChc->pClient = pClient;
    pChc->pScreen = pScreen;
    pChc->hideCount = 1;
    pChc->resource = FakeClientID(pClient->index);
    pChc->pNext = cs->pCursorHideCounts;
    cs->pCursorHideCounts = pChc;
    
    /* 
     * Create a resource for this element so it can be deleted
     * when the client goes away.
     */
    if (!AddResource (pChc->resource, CursorHideCountType, 
		      (pointer) pChc)) {
	xfree(pChc);
	return BadAlloc;
    }

    return Success;
}

/* 
 * Delete the given hide-counts list element from its screen list.
 */
static void
deleteCursorHideCount (CursorHideCountPtr pChcToDel, ScreenPtr pScreen)
{
    CursorScreenPtr    cs = GetCursorScreen(pScreen);
    CursorHideCountPtr pChc, pNext;
    CursorHideCountPtr pChcLast = NULL;

    pChc = cs->pCursorHideCounts;
    while (pChc != NULL) {
	pNext = pChc->pNext;
	if (pChc == pChcToDel) {
	    xfree(pChc);
	    if (pChcLast == NULL) {
		cs->pCursorHideCounts = pNext;
	    } else {
		pChcLast->pNext = pNext;
	    }
	    return;
	}
	pChcLast = pChc;
	pChc = pNext;
    }
}

/* 
 * Delete all the hide-counts list elements for this screen.
 */
static void
deleteCursorHideCountsForScreen (ScreenPtr pScreen)
{
    CursorScreenPtr    cs = GetCursorScreen(pScreen);
    CursorHideCountPtr pChc, pTmp;

    pChc = cs->pCursorHideCounts;
    while (pChc != NULL) {
	pTmp = pChc->pNext;
	FreeResource(pChc->resource, 0);
	pChc = pTmp;
    }
    cs->pCursorHideCounts = NULL;   
}

int 
ProcXFixesHideCursor (ClientPtr client) 
{
    WindowPtr pWin;
    CursorHideCountPtr pChc;
    REQUEST(xXFixesHideCursorReq);
    int ret;

    REQUEST_SIZE_MATCH (xXFixesHideCursorReq);

    ret = dixLookupResourceByType((pointer *)&pWin, stuff->window, RT_WINDOW,
			    client, DixGetAttrAccess);
    if (ret != Success) {
	client->errorValue = stuff->window;
	return (ret == BadValue) ? BadWindow : ret;
    }

    /* 
     * Has client hidden the cursor before on this screen? 
     * If so, just increment the count. 
     */

    pChc = findCursorHideCount(client, pWin->drawable.pScreen);
    if (pChc != NULL) {
	pChc->hideCount++;
	return client->noClientException;
    }

    /* 
     * This is the first time this client has hid the cursor 
     * for this screen.
     */
    ret = XaceHook(XACE_SCREEN_ACCESS, client, pWin->drawable.pScreen,
		   DixHideAccess);
    if (ret != Success)
	return ret;

    ret = createCursorHideCount(client, pWin->drawable.pScreen);

    if (ret == Success) {
	DeviceIntPtr dev;
	for (dev = inputInfo.devices; dev; dev = dev->next)
	{
	    if (IsMaster(dev) && IsPointerDevice(dev))
		CursorDisplayCursor(dev, pWin->drawable.pScreen, CursorCurrent[dev->id]);
	}
    }

    return ret;
}

int 
SProcXFixesHideCursor (ClientPtr client) 
{
    int n;
    REQUEST(xXFixesHideCursorReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesHideCursorReq);
    swapl (&stuff->window, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int 
ProcXFixesShowCursor (ClientPtr client) 
{
    WindowPtr pWin;
    CursorHideCountPtr pChc;
    int rc;
    REQUEST(xXFixesShowCursorReq);

    REQUEST_SIZE_MATCH (xXFixesShowCursorReq);

    rc = dixLookupResourceByType((pointer *)&pWin, stuff->window, RT_WINDOW,
			   client, DixGetAttrAccess);
    if (rc != Success) {
	client->errorValue = stuff->window;
	return (rc == BadValue) ? BadWindow : rc;
    }

    /* 
     * Has client hidden the cursor on this screen?
     * If not, generate an error.
     */
    pChc = findCursorHideCount(client, pWin->drawable.pScreen);
    if (pChc == NULL) {
	return BadMatch;
    }

    rc = XaceHook(XACE_SCREEN_ACCESS, client, pWin->drawable.pScreen,
		  DixShowAccess);
    if (rc != Success)
	return rc;

    pChc->hideCount--;
    if (pChc->hideCount <= 0) {
	FreeResource(pChc->resource, 0);
    }

    return (client->noClientException);
}

int 
SProcXFixesShowCursor (ClientPtr client) 
{
    int n;
    REQUEST(xXFixesShowCursorReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesShowCursorReq);
    swapl (&stuff->window, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

static int
CursorFreeClient (pointer data, XID id)
{
    CursorEventPtr	old = (CursorEventPtr) data;
    CursorEventPtr	*prev, e;
    
    for (prev = &cursorEvents; (e = *prev); prev = &e->next)
    {
	if (e == old)
	{
	    *prev = e->next;
	    xfree (e);
	    break;
	}
    }
    return 1;
}

static int
CursorFreeHideCount (pointer data, XID id)
{
    CursorHideCountPtr pChc = (CursorHideCountPtr) data;
    ScreenPtr pScreen = pChc->pScreen;
    DeviceIntPtr dev;

    deleteCursorHideCount(pChc, pChc->pScreen);
    for (dev = inputInfo.devices; dev; dev = dev->next)
    {
        if (IsMaster(dev) && IsPointerDevice(dev))
            CursorDisplayCursor(dev, pScreen, CursorCurrent[dev->id]);
    }

    return 1;
}

static int
CursorFreeWindow (pointer data, XID id)
{
    WindowPtr		pWindow = (WindowPtr) data;
    CursorEventPtr	e, next;

    for (e = cursorEvents; e; e = next)
    {
	next = e->next;
	if (e->pWindow == pWindow)
	{
	    FreeResource (e->clientResource, 0);
	}
    }
    return 1;
}

static CursorPtr
createInvisibleCursor (void)
{
    CursorPtr pCursor;
    unsigned char *psrcbits, *pmaskbits;
    CursorMetricRec cm;

    psrcbits = (unsigned char *) xcalloc(4, 1);
    pmaskbits = (unsigned char *) xcalloc(4, 1);
    if (psrcbits == NULL || pmaskbits == NULL) {
	return NULL;
    }

    cm.width = 1;
    cm.height = 1;
    cm.xhot = 0;
    cm.yhot = 0;

    AllocARGBCursor(psrcbits, pmaskbits,
		NULL, &cm,
		0, 0, 0,
		0, 0, 0,
		&pCursor, serverClient, (XID)0);

    return pCursor;
}

Bool
XFixesCursorInit (void)
{
    int	i;

    if (party_like_its_1989)
	CursorVisible = EnableCursor;
    
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	ScreenPtr	pScreen = screenInfo.screens[i];
	CursorScreenPtr	cs;

	cs = (CursorScreenPtr) xalloc (sizeof (CursorScreenRec));
	if (!cs)
	    return FALSE;
	Wrap (cs, pScreen, CloseScreen, CursorCloseScreen);
	Wrap (cs, pScreen, DisplayCursor, CursorDisplayCursor);
	cs->pCursorHideCounts = NULL;
	SetCursorScreen (pScreen, cs);
    }
    CursorClientType = CreateNewResourceType(CursorFreeClient,
					     "XFixesCursorClient");
    CursorHideCountType = CreateNewResourceType(CursorFreeHideCount,
						"XFixesCursorHideCount");
    CursorWindowType = CreateNewResourceType(CursorFreeWindow,
					     "XFixesCursorWindow");

    if (pInvisibleCursor == NULL) {
	pInvisibleCursor = createInvisibleCursor();
	if (pInvisibleCursor == NULL) {
	    return BadAlloc;
	}
    }

    return CursorClientType && CursorHideCountType && CursorWindowType;
}

