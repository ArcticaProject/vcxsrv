/*
(c) Copyright 1996 Hewlett-Packard Company
(c) Copyright 1996 International Business Machines Corp.
(c) Copyright 1996 Sun Microsystems, Inc.
(c) Copyright 1996 Novell, Inc.
(c) Copyright 1996 Digital Equipment Corp.
(c) Copyright 1996 Fujitsu Limited
(c) Copyright 1996 Hitachi, Ltd.

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
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the copyright holders shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from said
copyright holders.
*/
/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:          xprint.c
**    *
**    *  Copyright:     Copyright 1993, 1995 Hewlett-Packard Company
**    *
**    *		Copyright 1989 by The Massachusetts Institute of Technology
**    *
**    *		Permission to use, copy, modify, and distribute this
**    *		software and its documentation for any purpose and without
**    *		fee is hereby granted, provided that the above copyright
**    *		notice appear in all copies and that both that copyright
**    *		notice and this permission notice appear in supporting
**    *		documentation, and that the name of MIT not be used in
**    *		advertising or publicity pertaining to distribution of the
**    *		software without specific prior written permission.
**    *		M.I.T. makes no representation about the suitability of
**    *		this software for any purpose. It is provided "as is"
**    *		without any express or implied warranty.
**    *
**    *		MIT DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
**    *		INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
**    *		NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL MIT BE  LI-
**    *		ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
**    *		ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
**    *		PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
**    *		OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
**    *		THE USE OR PERFORMANCE OF THIS SOFTWARE.
**    *
**    *********************************************************
**
********************************************************************/

#define _XP_PRINT_SERVER_
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xos.h>
#define NEED_EVENTS
#include <X11/Xproto.h>
#undef NEED_EVENTS
#include "misc.h"
#include "windowstr.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "extnsionst.h"
#include "dixstruct.h"
#include <X11/Xatom.h>
#include <X11/extensions/Print.h>
#include <X11/extensions/Printstr.h>
#include "../hw/xprint/DiPrint.h"
#include "../hw/xprint/attributes.h"
#include "modinit.h"

static void XpResetProc(ExtensionEntry *);

static int ProcXpDispatch(ClientPtr);
static int ProcXpSwappedDispatch(ClientPtr);

static int ProcXpQueryVersion(ClientPtr);
static int ProcXpGetPrinterList(ClientPtr);
static int ProcXpCreateContext(ClientPtr);
static int ProcXpSetContext(ClientPtr);
static int ProcXpGetContext(ClientPtr);
static int ProcXpDestroyContext(ClientPtr);
static int ProcXpGetContextScreen(ClientPtr);
static int ProcXpStartJob(ClientPtr);
static int ProcXpEndJob(ClientPtr);
static int ProcXpStartDoc(ClientPtr);
static int ProcXpEndDoc(ClientPtr);
static int ProcXpStartPage(ClientPtr);
static int ProcXpEndPage(ClientPtr);
static int ProcXpSelectInput(ClientPtr);
static int ProcXpInputSelected(ClientPtr);
static int ProcXpPutDocumentData(ClientPtr);
static int ProcXpGetDocumentData(ClientPtr);
static int ProcXpGetAttributes(ClientPtr);
static int ProcXpGetOneAttribute(ClientPtr);
static int ProcXpSetAttributes(ClientPtr);
static int ProcXpRehashPrinterList(ClientPtr);
static int ProcXpQueryScreens(ClientPtr);
static int ProcXpGetPageDimensions(ClientPtr);
static int ProcXpSetImageResolution(ClientPtr);
static int ProcXpGetImageResolution(ClientPtr);

static void SwapXpNotifyEvent(xPrintPrintEvent *, xPrintPrintEvent *);
static void SwapXpAttributeEvent(xPrintAttributeEvent *, xPrintAttributeEvent *);

static int SProcXpGetPrinterList(ClientPtr);
static int SProcXpCreateContext(ClientPtr);
static int SProcXpSetContext(ClientPtr);
static int SProcXpGetContext(ClientPtr);
static int SProcXpDestroyContext(ClientPtr);
static int SProcXpGetContextScreen(ClientPtr);
static int SProcXpStartJob(ClientPtr);
static int SProcXpEndJob(ClientPtr);
static int SProcXpStartDoc(ClientPtr);
static int SProcXpEndDoc(ClientPtr);
static int SProcXpStartPage(ClientPtr);
static int SProcXpEndPage(ClientPtr);
static int SProcXpSelectInput(ClientPtr);
static int SProcXpInputSelected(ClientPtr);
static int SProcXpPutDocumentData(ClientPtr);
static int SProcXpGetDocumentData(ClientPtr);
static int SProcXpGetAttributes(ClientPtr);
static int SProcXpGetOneAttribute(ClientPtr);
static int SProcXpSetAttributes(ClientPtr);
static int SProcXpRehashPrinterList(ClientPtr);
static int SProcXpGetPageDimensions(ClientPtr);
static int SProcXpSetImageResolution(ClientPtr);
static int SProcXpGetImageResolution(ClientPtr);

static void SendXpNotify(XpContextPtr, int, int);
static void SendAttributeNotify(XpContextPtr, int);
static int XpFreeClient(pointer, XID);
static int XpFreeContext(pointer, XID);
static int XpFreePage(pointer, XID);
static Bool XpCloseScreen(int, ScreenPtr);
static CARD32 GetAllEventMasks(XpContextPtr);
static struct _XpClient *CreateXpClient(ClientPtr);
static struct _XpClient *FindClient(XpContextPtr, ClientPtr);
static struct _XpClient *AcquireClient(XpContextPtr, ClientPtr);

typedef struct _driver {
    struct _driver *next;
    char *name;
    int (* CreateContext)(XpContextPtr);
} XpDriverRec, *XpDriverPtr;

typedef struct  _xpScreen {
    Bool (* CloseScreen)(int, ScreenPtr);
    struct _driver *drivers;
} XpScreenRec, *XpScreenPtr;

/*
 * Each context has a list of XpClients indicating which clients have
 * associated this context with their connection.
 * Each such client has a RTclient resource allocated for it,
 * and this per-client
 * resource is used to delete the XpClientRec if/when the client closes
 * its connection.
 * The list of XpClients is also walked if/when the context is destroyed
 * so that the ContextPtr can be removed from the client's devPrivates.
 */
typedef struct _XpClient {
	struct _XpClient *pNext;
	ClientPtr	client;
	XpContextPtr	context;
	CARD32		eventMask;
	XID		contextClientID; /* unneeded sanity check? */
} XpClientRec, *XpClientPtr;

static void FreeXpClient(XpClientPtr, Bool);

/*
 * Each StartPage request specifies a window which forms the top level
 * window of the page.  One of the following structs is created as a
 * RTpage resource with the same ID as the window itself.  This enables 
 * us to clean up when/if the window is destroyed, and to prevent the
 * same window from being simultaneously referenced in multiple contexts.
 * The page resource is created at the first StartPage on a given window,
 * and is only destroyed when/if the window is destroyed.  When the
 * EndPage is recieved (or an EndDoc or EndJob) the context field is
 * set to NULL, but the resource remains alive.
 */
typedef struct _XpPage {
	XpContextPtr	context;
} XpPageRec, *XpPagePtr;

typedef struct _XpStPageRec {
    XpContextPtr pContext;
    Bool slept;
    XpPagePtr pPage;
    WindowPtr pWin;
} XpStPageRec, *XpStPagePtr;

typedef struct _XpStDocRec {
    XpContextPtr pContext;
    Bool slept;
    CARD8 type;
} XpStDocRec, *XpStDocPtr;

#define QUADPAD(x) ((((x)+3)>>2)<<2)

/*
 * Possible bit-mask values in the "state" field of a XpContextRec.
 */
#define JOB_STARTED (1 << 0)
#define DOC_RAW_STARTED (1 << 1)
#define DOC_COOKED_STARTED (1 << 2)
#define PAGE_STARTED (1 << 3)
#define GET_DOC_DATA_STARTED (1 << 4)
#define JOB_GET_DATA (1 << 5)
    
static XpScreenPtr XpScreens[MAXSCREENS];
static unsigned char XpReqCode;
static int XpEventBase;
static int XpErrorBase;
static DevPrivateKey XpClientPrivateKey = &XpClientPrivateKey;

#define XP_GETPRIV(pClient) ((XpContextPtr) \
    dixLookupPrivate(&(pClient)->devPrivates, XpClientPrivateKey))
#define XP_SETPRIV(pClient, p) \
    dixSetPrivate(&(pClient)->devPrivates, XpClientPrivateKey, p)

/*
 * There are three types of resources involved.  One is the resource associated
 * with the context itself, with an ID specified by a printing client.  The
 * next is a resource created by us on the client's behalf (and unknown to
 * the client) when a client inits or sets a context which allows us to 
 * track each client's interest in events
 * on a particular context, and also allows us to clean up this interest
 * record when/if the client's connection is closed.  Finally, there is
 * a resource created for each window that's specified in a StartPage.  This
 * resource carries the same ID as the window itself, and enables us to
 * easily prevent the same window being referenced in multiple contexts
 * simultaneously, and enables us to clean up if the window is destroyed
 * before the EndPage.
 */
static RESTYPE RTclient, RTcontext, RTpage;

/*
 * allEvents is the OR of all the legal event mask bits.
 */
static CARD32 allEvents = XPPrintMask | XPAttributeMask;


/*******************************************************************************
 *
 * ExtensionInit, Driver Init functions, QueryVersion, and Dispatch procs
 *
 ******************************************************************************/

/*
 * XpExtensionInit
 *
 * Called from InitExtensions in main() usually through miinitextension
 *
 */

void
XpExtensionInit(INITARGS)
{
    ExtensionEntry *extEntry;
    int i;

    RTclient = CreateNewResourceType(XpFreeClient);
    RTcontext = CreateNewResourceType(XpFreeContext);
    RTpage = CreateNewResourceType(XpFreePage);
    if (RTclient && RTcontext && RTpage &&
        (extEntry = AddExtension(XP_PRINTNAME, XP_EVENTS, XP_ERRORS,
                               ProcXpDispatch, ProcXpSwappedDispatch,
                               XpResetProc, StandardMinorOpcode)))
    {
        XpReqCode = (unsigned char)extEntry->base;
        XpEventBase = extEntry->eventBase;
        XpErrorBase = extEntry->errorBase;
        EventSwapVector[XpEventBase] = (EventSwapPtr) SwapXpNotifyEvent;
        EventSwapVector[XpEventBase+1] = (EventSwapPtr) SwapXpAttributeEvent;
    }

    for(i = 0; i < MAXSCREENS; i++)
    {
	/*
	 * If a screen has registered with our extension, then we
	 * wrap the screen's CloseScreen function to allow us to
	 * reset our ContextPrivate stuff.  Note that this
	 * requires a printing DDX to call XpRegisterInitFunc
	 * _before_ this extension is initialized - i.e. at screen init
	 * time, _not_ at root window creation time.
	 */
	if(XpScreens[i] != (XpScreenPtr)NULL)
	{
	    XpScreens[i]->CloseScreen = screenInfo.screens[i]->CloseScreen;
	    screenInfo.screens[i]->CloseScreen = XpCloseScreen;
	}
    }
}

static void
XpResetProc(ExtensionEntry *extEntry)
{
    /*
     * We can't free up the XpScreens recs here, because extensions are
     * closed before screens, and our CloseScreen function uses the XpScreens
     * recs.

    int i;

    for(i = 0; i < MAXSCREENS; i++)
    {
	if(XpScreens[i] != (XpScreenPtr)NULL)
	    Xfree(XpScreens[i]);
	XpScreens[i] = (XpScreenPtr)NULL;
    }
    */
}

static Bool
XpCloseScreen(int index, ScreenPtr pScreen)
{
    Bool (* CloseScreen)(int, ScreenPtr);

    CloseScreen = XpScreens[index]->CloseScreen;
    if(XpScreens[index] != (XpScreenPtr)NULL)
    {
	XpDriverPtr pDriv, nextDriv;

	pDriv = XpScreens[index]->drivers;
	while(pDriv != (XpDriverPtr)NULL)
	{
	    nextDriv = pDriv->next;
            Xfree(pDriv);
	    pDriv = nextDriv;
	}
	Xfree(XpScreens[index]);
    }
    XpScreens[index] = (XpScreenPtr)NULL;

    return (*CloseScreen)(index, pScreen);
}

/*
 * XpRegisterInitFunc tells the print extension which screens
 * are printers as opposed to displays, and what drivers are
 * supported on each screen.  This eliminates the need of
 * allocating print-related private structures on windows on _all_ screens.
 * It also hands the extension a pointer to the routine to be called
 * whenever a context gets created for a particular driver on this screen.
 */
void
XpRegisterInitFunc(ScreenPtr pScreen, char *driverName, int (*initContext)(struct _XpContext *))
{
    XpDriverPtr pDriver;

    if(XpScreens[pScreen->myNum] == 0)
    {
        if((XpScreens[pScreen->myNum] =
           (XpScreenPtr) Xalloc(sizeof(XpScreenRec))) == 0)
            return;
	XpScreens[pScreen->myNum]->CloseScreen = 0;
	XpScreens[pScreen->myNum]->drivers = 0;
    }

    if((pDriver = (XpDriverPtr)Xalloc(sizeof(XpDriverRec))) == 0)
	return;
    pDriver->next = XpScreens[pScreen->myNum]->drivers;
    pDriver->name = driverName;
    pDriver->CreateContext = initContext;
    XpScreens[pScreen->myNum]->drivers = pDriver;
}

static int 
ProcXpDispatch(ClientPtr client)
{
    REQUEST(xReq);

    switch(stuff->data)
    {
	case X_PrintQueryVersion:
            return ProcXpQueryVersion(client);
	case X_PrintGetPrinterList:
	    return ProcXpGetPrinterList(client);
	case X_PrintCreateContext:
	    return ProcXpCreateContext(client);
	case X_PrintSetContext:
	    return ProcXpSetContext(client);
	case X_PrintGetContext:
	    return ProcXpGetContext(client);
	case X_PrintDestroyContext:
	    return ProcXpDestroyContext(client);
	case X_PrintGetContextScreen:
	    return ProcXpGetContextScreen(client);
	case X_PrintStartJob:
            return ProcXpStartJob(client);
	case X_PrintEndJob:
            return ProcXpEndJob(client);
	case X_PrintStartDoc:
            return ProcXpStartDoc(client);
	case X_PrintEndDoc:
            return ProcXpEndDoc(client);
	case X_PrintStartPage:
            return ProcXpStartPage(client);
	case X_PrintEndPage:
            return ProcXpEndPage(client);
	case X_PrintSelectInput:
            return ProcXpSelectInput(client);
	case X_PrintInputSelected:
            return ProcXpInputSelected(client);
	case X_PrintPutDocumentData:
            return ProcXpPutDocumentData(client);
	case X_PrintGetDocumentData:
            return ProcXpGetDocumentData(client);
	case X_PrintSetAttributes:
	    return ProcXpSetAttributes(client);
	case X_PrintGetAttributes:
	    return ProcXpGetAttributes(client);
	case X_PrintGetOneAttribute:
	    return ProcXpGetOneAttribute(client);
	case X_PrintRehashPrinterList:
	    return ProcXpRehashPrinterList(client);
	case X_PrintQueryScreens:
            return ProcXpQueryScreens(client);
	case X_PrintGetPageDimensions:
            return ProcXpGetPageDimensions(client);
	case X_PrintSetImageResolution:
            return ProcXpSetImageResolution(client);
	case X_PrintGetImageResolution:
            return ProcXpGetImageResolution(client);
	default:
	    return BadRequest;
    }
}

static int 
ProcXpSwappedDispatch(ClientPtr client)
{
    int temp;
    REQUEST(xReq);

    switch(stuff->data)
    {
	case X_PrintQueryVersion:
	    swaps(&stuff->length, temp);
            return ProcXpQueryVersion(client);
	case X_PrintGetPrinterList:
	    return SProcXpGetPrinterList(client);
	case X_PrintCreateContext:
	    return SProcXpCreateContext(client);
	case X_PrintSetContext:
	    return SProcXpSetContext(client);
	case X_PrintGetContext:
	    return SProcXpGetContext(client);
	case X_PrintDestroyContext:
	    return SProcXpDestroyContext(client);
	case X_PrintGetContextScreen:
	    return SProcXpGetContextScreen(client);
	case X_PrintStartJob:
            return SProcXpStartJob(client);
	case X_PrintEndJob:
            return SProcXpEndJob(client);
	case X_PrintStartDoc:
            return SProcXpStartDoc(client);
	case X_PrintEndDoc:
            return SProcXpEndDoc(client);
	case X_PrintStartPage:
            return SProcXpStartPage(client);
	case X_PrintEndPage:
            return SProcXpEndPage(client);
	case X_PrintSelectInput:
	    return SProcXpSelectInput(client);
	case X_PrintInputSelected:
	    return SProcXpInputSelected(client);
	case X_PrintPutDocumentData:
            return SProcXpPutDocumentData(client);
	case X_PrintGetDocumentData:
            return SProcXpGetDocumentData(client);
	case X_PrintSetAttributes:
	    return SProcXpSetAttributes(client);
	case X_PrintGetAttributes:
	    return SProcXpGetAttributes(client);
	case X_PrintGetOneAttribute:
	    return SProcXpGetOneAttribute(client);
	case X_PrintRehashPrinterList:
	    return SProcXpRehashPrinterList(client);
	case X_PrintQueryScreens:
	    swaps(&stuff->length, temp);
            return ProcXpQueryScreens(client);
	case X_PrintGetPageDimensions:
            return SProcXpGetPageDimensions(client);
	case X_PrintSetImageResolution:
            return SProcXpSetImageResolution(client);
	case X_PrintGetImageResolution:
            return SProcXpGetImageResolution(client);
	default:
	    return BadRequest;
    }
}

static int
ProcXpQueryVersion(ClientPtr client)
{
    /* REQUEST(xPrintQueryVersionReq); */
    xPrintQueryVersionReply rep;
    register int n;
    long l;

    REQUEST_SIZE_MATCH(xPrintQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.majorVersion = XP_MAJOR_VERSION;
    rep.minorVersion = XP_MINOR_VERSION;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, l);
        swaps(&rep.majorVersion, n);
        swaps(&rep.minorVersion, n);
    }
    WriteToClient(client, sz_xPrintQueryVersionReply, (char *)&rep);
    return client->noClientException;
}

/*******************************************************************************
 *
 * GetPrinterList : Return a list of all printers associated with this
 *                  server.  Calls XpDiGetPrinterList, which is defined in
 *		    the device-independent code in Xserver/Xprint.
 *
 ******************************************************************************/

static int
ProcXpGetPrinterList(ClientPtr client)
{
    REQUEST(xPrintGetPrinterListReq);
    int totalSize;
    int numEntries;
    XpDiListEntry **pList;
    xPrintGetPrinterListReply *rep;
    int n, i, totalBytes;
    long l;
    char *curByte;

    REQUEST_AT_LEAST_SIZE(xPrintGetPrinterListReq);

    totalSize = ((sz_xPrintGetPrinterListReq) >> 2) +
                ((stuff->printerNameLen + 3) >> 2) +
                ((stuff->localeLen + 3) >> 2);
    if(totalSize != client->req_len)
	 return BadLength;

    pList = XpDiGetPrinterList(stuff->printerNameLen, (char *)(stuff + 1), 
			       stuff->localeLen, (char *)((stuff + 1) + 
			       QUADPAD(stuff->printerNameLen)));

    for(numEntries = 0, totalBytes = sz_xPrintGetPrinterListReply;
	pList[numEntries] != (XpDiListEntry *)NULL;
	numEntries++)
    {
	totalBytes += 2 * sizeof(CARD32); 
	totalBytes += QUADPAD(strlen(pList[numEntries]->name));
	totalBytes += QUADPAD(strlen(pList[numEntries]->description));
    }

    if((rep = (xPrintGetPrinterListReply *)xalloc(totalBytes)) == 
       (xPrintGetPrinterListReply *)NULL)
	return BadAlloc;

    rep->type = X_Reply;
    rep->length = (totalBytes - sz_xPrintGetPrinterListReply) >> 2;
    rep->sequenceNumber = client->sequence;
    rep->listCount = numEntries;
    if (client->swapped) {
        swaps(&rep->sequenceNumber, n);
        swapl(&rep->length, l);
        swapl(&rep->listCount, l);
    }

    for(i = 0, curByte = (char *)(rep + 1); i < numEntries; i++)
    {
	CARD32 *pCrd;
	int len;

	pCrd = (CARD32 *)curByte;
	len = strlen(pList[i]->name);
	*pCrd = len;
        if (client->swapped)
            swapl((long *)curByte, l);
	curByte += sizeof(CARD32);
	strncpy(curByte, pList[i]->name, len);
	curByte += QUADPAD(len);

	pCrd = (CARD32 *)curByte;
	len = strlen(pList[i]->description);
	*pCrd = len;
        if (client->swapped)
            swapl((long *)curByte, l);
	curByte += sizeof(CARD32);
	strncpy(curByte, pList[i]->description, len);
	curByte += QUADPAD(len);
    }

    XpDiFreePrinterList(pList);

    WriteToClient(client, totalBytes, (char *)rep);
    xfree(rep);
    return client->noClientException;
}

/*******************************************************************************
 *
 * QueryScreens: Returns the list of screens which are associated with
 *               print drivers.
 *
 ******************************************************************************/

static int
ProcXpQueryScreens(ClientPtr client)
{
    /* REQUEST(xPrintQueryScreensReq); */
    int i, numPrintScreens, totalSize;
    WINDOW *pWinId;
    xPrintQueryScreensReply *rep;
    long l;

    REQUEST_SIZE_MATCH(xPrintQueryScreensReq);

    rep = (xPrintQueryScreensReply *)xalloc(sz_xPrintQueryScreensReply);
    pWinId = (WINDOW *)(rep + 1);

    for(i = 0, numPrintScreens = 0, totalSize = sz_xPrintQueryScreensReply; 
	i < MAXSCREENS; i++)
    {
	/*
	 * If a screen has registered with our extension, then it's
	 * a printer screen.
	 */
	if(XpScreens[i] != (XpScreenPtr)NULL)
	{
	    numPrintScreens++;
	    totalSize += sizeof(WINDOW);
	    rep = (xPrintQueryScreensReply *)xrealloc(rep, totalSize);
	    /* fix of bug: pWinId should be set again after reallocate rep */
	    pWinId = (WINDOW *)(rep + 1);
	    *pWinId = WindowTable[i]->drawable.id;
            if (client->swapped)
                swapl((long *)pWinId, l);
	}
    }

    rep->type = X_Reply;
    rep->sequenceNumber = client->sequence;
    rep->length = (totalSize - sz_xPrintQueryScreensReply) >> 2;
    rep->listCount = numPrintScreens;
    if (client->swapped)
    {
	int n;

        swaps(&rep->sequenceNumber, n);
        swapl(&rep->length, l);
        swapl(&rep->listCount, l);
    }

    WriteToClient(client, totalSize, (char *)rep);
    xfree(rep);
    return client->noClientException;
}

static int 
ProcXpGetPageDimensions(ClientPtr client)
{
    REQUEST(xPrintGetPageDimensionsReq);
    CARD16 width, height;
    xRectangle rect;
    xPrintGetPageDimensionsReply rep;
    XpContextPtr pContext;
    int result;

    REQUEST_SIZE_MATCH(xPrintGetPageDimensionsReq);

    if((pContext =(XpContextPtr)SecurityLookupIDByType(client,
						       stuff->printContext,
						       RTcontext,
						       DixReadAccess))
       == (XpContextPtr)NULL)
    {
	client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    if((pContext->funcs.GetMediumDimensions == 0) ||
       (pContext->funcs.GetReproducibleArea == 0))
        return BadImplementation;

    result = pContext->funcs.GetMediumDimensions(pContext, &width, &height);
    if(result != Success)
        return result;

        result = pContext->funcs.GetReproducibleArea(pContext, &rect);
    if(result != Success)
        return result;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.width = width;
    rep.height = height;
    rep.rx = rect.x;
    rep.ry = rect.y;
    rep.rwidth = rect.width;
    rep.rheight = rect.height;

    if(client->swapped)
    {
	int n;
	long l;

        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, l);
        swaps(&rep.width, n);
        swaps(&rep.height, n);
        swaps(&rep.rx, n);
        swaps(&rep.ry, n);
        swaps(&rep.rwidth, n);
        swaps(&rep.rheight, n);
    }

    WriteToClient(client, sz_xPrintGetPageDimensionsReply, (char *)&rep);
    return client->noClientException;
}

static int 
ProcXpSetImageResolution(ClientPtr client)
{
    REQUEST(xPrintSetImageResolutionReq);
    xPrintSetImageResolutionReply rep;
    XpContextPtr pContext;
    Bool status;
    int result;

    REQUEST_SIZE_MATCH(xPrintSetImageResolutionReq);

    if((pContext =(XpContextPtr)SecurityLookupIDByType(client,
						       stuff->printContext,
						       RTcontext,
						       DixWriteAccess))
       == (XpContextPtr)NULL)
    {
	client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    rep.prevRes = pContext->imageRes;
    if(pContext->funcs.SetImageResolution != 0) {
        result = pContext->funcs.SetImageResolution(pContext,
						    (int)stuff->imageRes,
						    &status);
	if(result != Success)
	    status = FALSE;
    } else
        status = FALSE;

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.status = status;

    if(client->swapped)
    {
	int n;
	long l;

        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, l);
        swaps(&rep.prevRes, n);
    }

    WriteToClient(client, sz_xPrintSetImageResolutionReply, (char *)&rep);
    return client->noClientException;
}

static int 
ProcXpGetImageResolution(ClientPtr client)
{
    REQUEST(xPrintGetImageResolutionReq);
    xPrintGetImageResolutionReply rep;
    XpContextPtr pContext;

    REQUEST_SIZE_MATCH(xPrintGetImageResolutionReq);

    if((pContext =(XpContextPtr)SecurityLookupIDByType(client,
						       stuff->printContext,
						       RTcontext,
						       DixReadAccess))
       == (XpContextPtr)NULL)
    {
	client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.imageRes = pContext->imageRes;

    if(client->swapped)
    {
	int n;
	long l;

        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, l);
        swaps(&rep.imageRes, n);
    }

    WriteToClient(client, sz_xPrintGetImageResolutionReply, (char *)&rep);
    return client->noClientException;
}

/*******************************************************************************
 *
 * RehashPrinterList : Cause the server's list of printers to be rebuilt.
 *                     This allows new printers to be added, or old ones
 *		       deleted without needing to restart the server.
 *
 ******************************************************************************/

static int
ProcXpRehashPrinterList(ClientPtr client)
{
    /* REQUEST(xPrintRehashPrinterListReq); */

    REQUEST_SIZE_MATCH(xPrintRehashPrinterListReq);

    return XpRehashPrinterList();
}

/******************************************************************************
 *
 * Context functions: Init, Set, Destroy, FreeContext
 *			AllocateContextPrivateIndex, AllocateContextPrivate
 *			and supporting functions.
 *
 *     Init creates a context, creates a XpClientRec for the calling
 *     client, and stores the contextPtr in the client's devPrivates.
 *
 *     Set creates a XpClientRec for the calling client, and stores the
 *     contextPtr in the client's devPrivates unless the context is None.
 *     If the context is None, then the client's connection association
 *     with any context is removed.
 *
 *     Destroy frees any and all XpClientRecs associated with the context,
 *     frees the context itself, and removes the contextPtr from any
 *     relevant client devPrivates.
 *
 *     FreeContext is called by FreeResource to free up a context.
 *
 ******************************************************************************/

/*
 * CreateContext creates and initializes the memory for the context itself.
 * The driver's CreateContext function
 * is then called.
 */
static int
ProcXpCreateContext(ClientPtr client)
{
    REQUEST(xPrintCreateContextReq);
    XpScreenPtr pPrintScreen;
    WindowPtr pRoot;
    char *driverName;
    XpContextPtr pContext;
    int result = Success;
    XpDriverPtr pDriver;

    REQUEST_AT_LEAST_SIZE(xPrintCreateContextReq);

    LEGAL_NEW_RESOURCE(stuff->contextID, client);

    /*
     * Check to see if the printer name is valid.
     */
    if((pRoot = XpDiValidatePrinter((char *)(stuff + 1), stuff->printerNameLen)) == 
       (WindowPtr)NULL)
	return BadMatch;

    pPrintScreen = XpScreens[pRoot->drawable.pScreen->myNum];

    /*
     * Allocate and add the context resource.
     */
    if((pContext = (XpContextPtr) xalloc(sizeof(XpContextRec))) == 
       (XpContextPtr) NULL)
	return BadAlloc;

    if(AddResource(stuff->contextID, RTcontext, (pointer) pContext)
       != TRUE)
    {
       xfree(pContext);
       return BadAlloc;
    }

    pContext->contextID = stuff->contextID;
    pContext->clientHead = (XpClientPtr)NULL;
    pContext->screenNum = pRoot->drawable.pScreen->myNum;
    pContext->state = 0;
    pContext->clientSlept = (ClientPtr)NULL;
    pContext->imageRes = 0;
    pContext->devPrivates = NULL;

    pContext->funcs.DestroyContext = 0;
    pContext->funcs.StartJob = 0;
    pContext->funcs.EndJob = 0;
    pContext->funcs.StartDoc = 0;
    pContext->funcs.EndDoc = 0;
    pContext->funcs.StartPage = 0;
    pContext->funcs.EndPage = 0;
    pContext->funcs.PutDocumentData = 0;
    pContext->funcs.GetDocumentData = 0;
    pContext->funcs.GetAttributes = 0;
    pContext->funcs.GetOneAttribute = 0;
    pContext->funcs.SetAttributes = 0;
    pContext->funcs.AugmentAttributes = 0;
    pContext->funcs.GetMediumDimensions = 0;
    pContext->funcs.GetReproducibleArea = 0;
    pContext->funcs.SetImageResolution = 0;

    if((pContext->printerName = (char *)xalloc(stuff->printerNameLen + 1)) == 
       (char *)NULL)
    {
	/* Freeing the context also causes the XpClients to be freed. */
	FreeResource(stuff->contextID, RT_NONE);
	return BadAlloc;
    }
    strncpy(pContext->printerName, (char *)(stuff + 1), stuff->printerNameLen);
    pContext->printerName[stuff->printerNameLen] = (char)'\0';

    driverName = XpDiGetDriverName(pRoot->drawable.pScreen->myNum, 
				   pContext->printerName);
    
    for(pDriver = pPrintScreen->drivers; 
	pDriver != (XpDriverPtr)NULL;
	pDriver = pDriver->next)
    {
	if(!strcmp(driverName, pDriver->name))
	{
	    if(pDriver->CreateContext != 0)
	        pDriver->CreateContext(pContext);
	    else
	        return BadImplementation;
	    break;
	}
    }

    if (client->noClientException != Success)
        return client->noClientException;
    else
	return result;
}

/*
 * SetContext creates the calling client's contextClient resource,
 * and stashes the contextID in the client's devPrivate.
 */
static int
ProcXpSetContext(ClientPtr client)
{
    REQUEST(xPrintSetContextReq);

    XpContextPtr pContext;
    XpClientPtr pPrintClient;
    int result = Success;

    REQUEST_AT_LEAST_SIZE(xPrintSetContextReq);

    if((pContext = XP_GETPRIV(client)) != (pointer)NULL)
    {
	/*
	 * Erase this client's knowledge of its old context, if any.
	 */
        if((pPrintClient = FindClient(pContext, client)) != (XpClientPtr)NULL)
        {
	    XpUnsetFontResFunc(client);
	    
	    if(pPrintClient->eventMask == 0)
		FreeXpClient(pPrintClient, TRUE);
        }

	XP_SETPRIV(client, NULL);
    }
    if(stuff->printContext == None)
        return Success;

    /*
     * Check to see that the supplied XID is really a valid print context
     * in this server.
     */
    if((pContext =(XpContextPtr)SecurityLookupIDByType(client,
						       stuff->printContext,
						       RTcontext,
						       DixWriteAccess))
       == (XpContextPtr)NULL)
    {
	client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    if((pPrintClient = AcquireClient(pContext, client)) == (XpClientPtr)NULL)
        return BadAlloc;

    XP_SETPRIV(client, pContext);

    XpSetFontResFunc(client);

    if (client->noClientException != Success)
        return client->noClientException;
    else
	return result;
}

XpContextPtr
XpGetPrintContext(ClientPtr client)
{
    return XP_GETPRIV(client);
}

static int
ProcXpGetContext(ClientPtr client)
{
    /* REQUEST(xPrintGetContextReq); */
    xPrintGetContextReply rep;

    XpContextPtr pContext;
    register int n;
    register long l;

    REQUEST_SIZE_MATCH(xPrintGetContextReq);

    if((pContext = XP_GETPRIV(client)) == (pointer)NULL)
	rep.printContext = None;
    else
        rep.printContext = pContext->contextID;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, l);
        swapl(&rep.printContext, l);
    }
    WriteToClient(client, sz_xPrintGetContextReply, (char *)&rep);
    return client->noClientException;
}


/*
 * DestroyContext frees the context associated with the calling client.
 * It operates by freeing the context resource ID, thus causing XpFreeContext
 * to be called.
 */
static int
ProcXpDestroyContext(ClientPtr client)
{
    REQUEST(xPrintDestroyContextReq);

    XpContextPtr pContext;

    REQUEST_SIZE_MATCH(xPrintDestroyContextReq);

    if((pContext =(XpContextPtr)SecurityLookupIDByType(client,
						       stuff->printContext,
						       RTcontext,
						       DixDestroyAccess))
       == (XpContextPtr)NULL)
    {
	client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    XpUnsetFontResFunc(client);
	    
    FreeResource(pContext->contextID, RT_NONE);

    return Success;
}

static int
ProcXpGetContextScreen(ClientPtr client)
{
    REQUEST(xPrintGetContextScreenReq);
    xPrintGetContextScreenReply rep;
    XpContextPtr pContext;
    int n;
    long l;

    if((pContext =(XpContextPtr)SecurityLookupIDByType(client,
						       stuff->printContext,
						       RTcontext,
						       DixReadAccess))
       == (XpContextPtr)NULL)
        return XpErrorBase+XPBadContext;
    
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.rootWindow = WindowTable[pContext->screenNum]->drawable.id;

    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, l);
        swapl(&rep.rootWindow, l);
    }

    WriteToClient(client, sz_xPrintGetContextScreenReply, (char *)&rep);
    return client->noClientException;
}

/*
 * XpFreeContext is the routine called by dix:FreeResource when a context
 * resource ID is freed.
 * It checks to see if there's a partial job pending on the context, and
 * if so it calls the appropriate End procs with the cancel flag set.
 * It calls the driver's DestroyContext routine to allow the driver to clean
 * up any context-related memory or state.
 * It calls FreeXpClient to free all the 
 * associated XpClientRecs and to set all the client->devPrivates to NULL.
 * It frees the printer name string, and frees the context
 * itself.
 */
static int
XpFreeContext(pointer data, XID id)
{
    XpContextPtr pContext = (XpContextPtr)data;

    /* Clean up any pending job on this context */
    if(pContext->state != 0)
    {
	if(pContext->state & PAGE_STARTED)
	{
	    WindowPtr pWin = (WindowPtr )LookupIDByType(
				       pContext->pageWin, RT_WINDOW);
	    XpPagePtr pPage = (XpPagePtr)LookupIDByType(
				       pContext->pageWin, RTpage);

	    pContext->funcs.EndPage(pContext, pWin);
	    SendXpNotify(pContext, XPEndPageNotify, TRUE);
	    pContext->state &= ~PAGE_STARTED;
	    if(pPage)
	        pPage->context = (XpContextPtr)NULL;
	}
	if((pContext->state & DOC_RAW_STARTED) || 
	   (pContext->state & DOC_COOKED_STARTED))
	{
	    pContext->funcs.EndDoc(pContext, TRUE);
	    SendXpNotify(pContext, XPEndDocNotify, TRUE);
	    pContext->state &= ~DOC_RAW_STARTED;
	    pContext->state &= ~DOC_COOKED_STARTED;
	}
	if(pContext->funcs.EndJob != 0)
	{
	    pContext->funcs.EndJob(pContext, TRUE);
	    SendXpNotify(pContext, XPEndJobNotify, TRUE);
	    pContext->state &= ~JOB_STARTED;
	    pContext->state &= ~GET_DOC_DATA_STARTED;
	}
    }

    /* 
     * Tell the driver we're destroying the context
     * This allows the driver to free and ContextPrivate data
     */
    if(pContext->funcs.DestroyContext != 0)
	pContext->funcs.DestroyContext(pContext);

    /* Free up all the XpClientRecs */
    while(pContext->clientHead != (XpClientPtr)NULL)
    {
	FreeXpClient(pContext->clientHead, TRUE);
    }

    xfree(pContext->printerName);
    dixFreePrivates(pContext->devPrivates);
    xfree(pContext);
    return Success; /* ??? */
}

/*
 * XpFreeClient is the routine called by dix:FreeResource when a RTclient
 * is freed.  It simply calls the FreeXpClient routine to do the work.
 */
static int
XpFreeClient(pointer data, XID id)
{
    FreeXpClient((XpClientPtr)data, TRUE);
    return Success;
}

/*
 * FreeXpClient 
 * frees the ClientRec passed in, and sets the client->devPrivates to NULL
 * if the client->devPrivates points to the same context as the XpClient.
 * Called from XpFreeContext(from FreeResource), and 
 * XpFreeClient.  The boolean freeResource specifies whether or not to call
 * FreeResource for the XpClientRec's XID.  We should free it except if we're
 * called from XpFreeClient (which is itself called from FreeResource for the
 * XpClientRec's XID).
 */
static void
FreeXpClient(XpClientPtr pXpClient, Bool freeResource)
{
    XpClientPtr pCurrent, pPrev;
    XpContextPtr pContext = pXpClient->context;

    /*
     * If we're freeing the clientRec associated with the context tied
     * to the client's devPrivates, then we need to clear the devPrivates.
     */
    if(XP_GETPRIV(pXpClient->client) == pXpClient->context)
    {
	XP_SETPRIV(pXpClient->client, NULL);
    }

    for(pPrev = (XpClientPtr)NULL, pCurrent = pContext->clientHead; 
	pCurrent != (XpClientPtr)NULL; 
	pCurrent = pCurrent->pNext)
    {
	if(pCurrent == pXpClient)
	{
	    if(freeResource == TRUE)
                FreeResource (pCurrent->contextClientID, RTclient);

            if (pPrev != (XpClientPtr)NULL)
                pPrev->pNext = pCurrent->pNext;
            else
                pContext->clientHead = pCurrent->pNext;

            xfree (pCurrent);
	    break;
	}
	pPrev = pCurrent;
    }
}

/*
 * CreateXpClient takes a ClientPtr and returns a pointer to a
 * XpClientRec which it allocates.  It also initializes the Rec,
 * including adding a resource on behalf of the client to enable the
 * freeing of the Rec when the client's connection is closed.
 */
static XpClientPtr
CreateXpClient(ClientPtr client)
{
    XpClientPtr pNewPrintClient;
    XID clientResource;

    if((pNewPrintClient = (XpClientPtr)xalloc(sizeof(XpClientRec))) ==
      (XpClientPtr)NULL)
        return (XpClientPtr)NULL;

    clientResource = FakeClientID(client->index);
    if(!AddResource(clientResource, RTclient, (pointer)pNewPrintClient))
    {
        xfree (pNewPrintClient);
        return (XpClientPtr)NULL;
    }

    pNewPrintClient->pNext = (XpClientPtr)NULL;
    pNewPrintClient->client = client;
    pNewPrintClient->context = (XpContextPtr)NULL;
    pNewPrintClient->eventMask = 0;
    pNewPrintClient->contextClientID = clientResource;

    return pNewPrintClient;
}

/*
 * XpFreePage is the routine called by dix:FreeResource to free the page
 * resource built with the same ID as a page window.  It checks to see
 * if we're in the middle of a page, and if so calls the driver's EndPage
 * function with 'cancel' set TRUE.  It frees the memory associated with
 * the page resource.
 */
static int
XpFreePage(pointer data, XID id)
{
    XpPagePtr page = (XpPagePtr)data;
    int result = Success;
    WindowPtr pWin = (WindowPtr )LookupIDByType(id, RT_WINDOW);

    /* Check to see if the window's being deleted in the middle of a page */
    if(page->context != (XpContextPtr)NULL && 
       page->context->state & PAGE_STARTED)
    {
	if(page->context->funcs.EndPage != 0)
	    result = page->context->funcs.EndPage(page->context, pWin);
        SendXpNotify(page->context, XPEndPageNotify, (int)TRUE);
	page->context->pageWin = 0; /* None, NULL??? XXX */
    }

    xfree(page);
    return result;
}

static XpClientPtr
AcquireClient(XpContextPtr pContext, ClientPtr client)
{
    XpClientPtr pXpClient;

    if((pXpClient = FindClient(pContext, client)) != (XpClientPtr)NULL)
	return pXpClient;

    if((pXpClient = CreateXpClient(client)) == (XpClientPtr)NULL)
	    return (XpClientPtr)NULL;

    pXpClient->context = pContext;
    pXpClient->pNext = pContext->clientHead;
    pContext->clientHead = pXpClient;

    return pXpClient;
}

static XpClientPtr
FindClient(XpContextPtr pContext, ClientPtr client)
{
    XpClientPtr pXpClient;

    for(pXpClient = pContext->clientHead; pXpClient != (XpClientPtr)NULL;
	pXpClient = pXpClient->pNext)
    {
	if(pXpClient->client == client)  return pXpClient;
    }
    return (XpClientPtr)NULL;
}


/******************************************************************************
 *
 * Start/End Functions: StartJob, EndJob, StartDoc, EndDoc, StartPage, EndPage
 *
 ******************************************************************************/

static int
ProcXpStartJob(ClientPtr client)
{
    REQUEST(xPrintStartJobReq);
    XpContextPtr pContext;
    int result = Success;

    REQUEST_SIZE_MATCH(xPrintStartJobReq);

    /* Check to see that a context has been established by this client. */
    if((pContext = XP_GETPRIV(client)) == (XpContextPtr)NULL)
        return XpErrorBase+XPBadContext;

    if(pContext->state != 0)
	return XpErrorBase+XPBadSequence;

    if(stuff->saveData != XPSpool && stuff->saveData != XPGetData)
    {
	client->errorValue = stuff->saveData;
	return BadValue;
    }

    if(pContext->funcs.StartJob != 0)
        result = pContext->funcs.StartJob(pContext, 
			 (stuff->saveData == XPGetData)? TRUE:FALSE,
			 client);
    else
        return BadImplementation;

    pContext->state = JOB_STARTED;
    if(stuff->saveData == XPGetData)
	pContext->state |= JOB_GET_DATA;

    SendXpNotify(pContext, XPStartJobNotify, FALSE);

    if (client->noClientException != Success)
        return client->noClientException;
    else
        return result;
}

static int
ProcXpEndJob(ClientPtr client)
{
    REQUEST(xPrintEndJobReq);
    int result = Success;
    XpContextPtr pContext;

    REQUEST_SIZE_MATCH(xPrintEndJobReq);

    if((pContext = XP_GETPRIV(client)) == (XpContextPtr)NULL)
        return XpErrorBase+XPBadSequence;

    if(!(pContext->state & JOB_STARTED))
	return XpErrorBase+XPBadSequence;
    
    /* Check for missing EndDoc */
    if((pContext->state & DOC_RAW_STARTED) || 
       (pContext->state & DOC_COOKED_STARTED))
    {
	if(pContext->state & PAGE_STARTED)
	{
	    WindowPtr pWin = (WindowPtr )LookupIDByType(
					   pContext->pageWin, RT_WINDOW);
	    XpPagePtr pPage = (XpPagePtr)LookupIDByType(
				       pContext->pageWin, RTpage);

	    if(stuff->cancel != TRUE)
	        return XpErrorBase+XPBadSequence;

            if(pContext->funcs.EndPage != 0)
                result = pContext->funcs.EndPage(pContext, pWin);
            else
	        return BadImplementation;

	    SendXpNotify(pContext, XPEndPageNotify, TRUE);

	    pContext->state &= ~PAGE_STARTED;

	    if(pPage)
	        pPage->context = (XpContextPtr)NULL;

	    if(result != Success) return result;
	}

        if(pContext->funcs.EndDoc != 0)
            result = pContext->funcs.EndDoc(pContext, stuff->cancel);
        else
	    return BadImplementation;

        SendXpNotify(pContext, XPEndDocNotify, stuff->cancel);
    }

    if(pContext->funcs.EndJob != 0)
        result = pContext->funcs.EndJob(pContext, stuff->cancel);
    else
	return BadImplementation;

    pContext->state = 0;

    SendXpNotify(pContext, XPEndJobNotify, stuff->cancel);

    if (client->noClientException != Success)
        return client->noClientException;
    else
        return result;
}

static Bool
DoStartDoc(ClientPtr client, XpStDocPtr c)
{
    XpContextPtr pContext = c->pContext;

    if(c->pContext->state & JOB_GET_DATA && 
       !(c->pContext->state & GET_DOC_DATA_STARTED))
    {
	if(!c->slept)
	{
	    c->slept = TRUE;
	    ClientSleep(client, (ClientSleepProcPtr)DoStartDoc, (pointer) c);
	    c->pContext->clientSlept = client;
	}
	return TRUE;
    }
    
    if(pContext->funcs.StartDoc != 0)
        (void) pContext->funcs.StartDoc(pContext, c->type);
    else
    {
	    SendErrorToClient(client, XpReqCode, X_PrintStartPage, 0, 
			      BadImplementation);
	    return TRUE;
    }

    if(c->type == XPDocNormal)
        pContext->state |= DOC_COOKED_STARTED;
    else
	pContext->state |= DOC_RAW_STARTED;

    SendXpNotify(pContext, XPStartDocNotify, (int)FALSE);

    xfree(c);
    return TRUE;
}

static int
ProcXpStartDoc(ClientPtr client)
{
    REQUEST(xPrintStartDocReq);
    int result = Success;
    XpContextPtr pContext;
    XpStDocPtr c;

    REQUEST_SIZE_MATCH(xPrintStartDocReq);

    if((pContext = XP_GETPRIV(client)) == (XpContextPtr)NULL)
        return XpErrorBase+XPBadSequence;

    if(!(pContext->state & JOB_STARTED) || 
       pContext->state & DOC_RAW_STARTED ||
       pContext->state & DOC_COOKED_STARTED)
	return XpErrorBase+XPBadSequence;

    if(stuff->type != XPDocNormal && stuff->type != XPDocRaw)
    {
	client->errorValue = stuff->type;
	return BadValue;
    }

    c = (XpStDocPtr)xalloc(sizeof(XpStDocRec));
    c->pContext = pContext;
    c->type = stuff->type;
    c->slept = FALSE;
    (void)DoStartDoc(client, c);

    if (client->noClientException != Success)
        return client->noClientException;
    else
        return result;
}

static int
ProcXpEndDoc(ClientPtr client)
{
    REQUEST(xPrintEndDocReq);
    XpContextPtr pContext;
    int result = Success;

    REQUEST_SIZE_MATCH(xPrintEndDocReq);

    if((pContext = XP_GETPRIV(client)) == (XpContextPtr)NULL)
        return XpErrorBase+XPBadSequence;

    if(!(pContext->state & DOC_RAW_STARTED) &&
       !(pContext->state & DOC_COOKED_STARTED))
	return XpErrorBase+XPBadSequence;
    
    if(pContext->state & PAGE_STARTED)
    {
	if(stuff->cancel == TRUE)
	{
	    WindowPtr pWin = (WindowPtr )LookupIDByType(
					   pContext->pageWin, RT_WINDOW);
	    XpPagePtr pPage = (XpPagePtr)LookupIDByType(
				       pContext->pageWin, RTpage);

            if(pContext->funcs.EndPage != 0)
                result = pContext->funcs.EndPage(pContext, pWin);
            else
	        return BadImplementation;

	    SendXpNotify(pContext, XPEndPageNotify, TRUE);

	    if(pPage)
	        pPage->context = (XpContextPtr)NULL;
	}
	else
	    return XpErrorBase+XPBadSequence;
	if(result != Success)
	    return result;
    }

    if(pContext->funcs.EndDoc != 0)
        result = pContext->funcs.EndDoc(pContext, stuff->cancel);
    else
	return BadImplementation;

    pContext->state &= ~DOC_RAW_STARTED;
    pContext->state &= ~DOC_COOKED_STARTED;

    SendXpNotify(pContext, XPEndDocNotify, stuff->cancel);

    if (client->noClientException != Success)
        return client->noClientException;
    else
        return result;
}

static Bool
DoStartPage(
    ClientPtr client,
    XpStPagePtr c)
{
    WindowPtr pWin = c->pWin;
    int result = Success;
    XpContextPtr pContext = c->pContext;
    XpPagePtr pPage;

    if(c->pContext->state & JOB_GET_DATA && 
       !(c->pContext->state & GET_DOC_DATA_STARTED))
    {
	if(!c->slept)
	{
	    c->slept = TRUE;
	    ClientSleep(client, (ClientSleepProcPtr)DoStartPage, (pointer) c);
	    c->pContext->clientSlept = client;
	}
	return TRUE;
    }

    if(!(pContext->state & DOC_COOKED_STARTED))
    {
	/* Implied StartDoc if it was omitted */
        if(pContext->funcs.StartDoc != 0)
            result = pContext->funcs.StartDoc(pContext, XPDocNormal);
        else
	{
	    SendErrorToClient(client, XpReqCode, X_PrintStartPage, 0, 
			      BadImplementation);
	    return TRUE;
	}

	if(result != Success) 
	{
	    SendErrorToClient(client, XpReqCode, X_PrintStartPage, 0, result);
	    return TRUE;
	}

        pContext->state |= DOC_COOKED_STARTED;
        SendXpNotify(pContext, XPStartDocNotify, (int)FALSE);
    }

    /* ensure the window's not already being used as a page */
    if((pPage = (XpPagePtr)LookupIDByType(c->pWin->drawable.id, RTpage)) != 
       (XpPagePtr)NULL)
    {
        if(pPage->context != (XpContextPtr)NULL)
	{
	    SendErrorToClient(client, XpReqCode, X_PrintStartPage, 0, 
			      BadWindow);
	    return TRUE;
	}
    }
    else
    {
        if((pPage = (XpPagePtr)xalloc(sizeof(XpPageRec))) == (XpPagePtr)NULL)
	{
	    SendErrorToClient(client, XpReqCode, X_PrintStartPage, 0, 
			      BadAlloc);
	    return TRUE;
	}
        if(AddResource(c->pWin->drawable.id, RTpage, pPage) == FALSE)
        {
	    xfree(pPage);
	    SendErrorToClient(client, XpReqCode, X_PrintStartPage, 0, 
			      BadAlloc);
	    return TRUE;
        }
    }

    pPage->context = pContext;
    pContext->pageWin = c->pWin->drawable.id;

    if(pContext->funcs.StartPage != 0)
        result = pContext->funcs.StartPage(pContext, pWin);
    else
    {
	SendErrorToClient(client, XpReqCode, X_PrintStartPage, 0, 
			  BadImplementation);
	return TRUE;
    }

    pContext->state |= PAGE_STARTED;

    (void)MapWindow(pWin, client);

    SendXpNotify(pContext, XPStartPageNotify, (int)FALSE);

    return TRUE;
}

static int
ProcXpStartPage(ClientPtr client)
{
    REQUEST(xPrintStartPageReq);
    WindowPtr pWin;
    int result = Success;
    XpContextPtr pContext;
    XpStPagePtr c;

    REQUEST_SIZE_MATCH(xPrintStartPageReq);

    if((pContext = XP_GETPRIV(client)) == (XpContextPtr)NULL)
        return XpErrorBase+XPBadSequence;

    if(!(pContext->state & JOB_STARTED))
	return XpErrorBase+XPBadSequence;

    /* can't have pages in a raw documented */
    if(pContext->state & DOC_RAW_STARTED)
	return XpErrorBase+XPBadSequence;
    
    if(pContext->state & PAGE_STARTED)
	return XpErrorBase+XPBadSequence;

    result = dixLookupWindow(&pWin, stuff->window, client, DixWriteAccess);
    if (result != Success)
	return result;
    if (pWin->drawable.pScreen->myNum != pContext->screenNum)
	return BadWindow;

    if((c = (XpStPagePtr)xalloc(sizeof(XpStPageRec))) == (XpStPagePtr)NULL)
	return BadAlloc;
    c->pContext = pContext;
    c->slept = FALSE;
    c->pWin = pWin;

    (void)DoStartPage(client, c);

    if (client->noClientException != Success)
        return client->noClientException;
    else
        return result;
}

static int
ProcXpEndPage(ClientPtr client)
{
    REQUEST(xPrintEndPageReq);
    int result = Success;
    XpContextPtr pContext;
    XpPagePtr page;
    WindowPtr pWin;

    REQUEST_SIZE_MATCH(xPrintEndPageReq);

    if((pContext = XP_GETPRIV(client)) == (XpContextPtr)NULL)
        return XpErrorBase+XPBadSequence;

    if(!(pContext->state & PAGE_STARTED))
	return XpErrorBase+XPBadSequence;

    pWin = (WindowPtr )LookupIDByType(pContext->pageWin, RT_WINDOW);

    /* Call the ddx's EndPage proc. */
    if(pContext->funcs.EndPage != 0)
        result = pContext->funcs.EndPage(pContext, pWin);
    else
	return BadImplementation;

    if((page = (XpPagePtr)LookupIDByType(pContext->pageWin, RTpage)) !=
       (XpPagePtr)NULL)
	page->context = (XpContextPtr)NULL;

    pContext->state &= ~PAGE_STARTED;
    pContext->pageWin = 0; /* None, NULL??? XXX */

    (void)UnmapWindow(pWin, FALSE);

    SendXpNotify(pContext, XPEndPageNotify, stuff->cancel);

    if (client->noClientException != Success)
        return client->noClientException;
    else
        return result;
}

/*******************************************************************************
 *
 * Document Data Functions: PutDocumentData, GetDocumentData
 *
 ******************************************************************************/

static int
ProcXpPutDocumentData(ClientPtr client)
{
    REQUEST(xPrintPutDocumentDataReq);
    XpContextPtr pContext;
    DrawablePtr pDraw;
    int result = Success;
    unsigned totalSize;
    char *pData, *pDoc_fmt, *pOptions;

    REQUEST_AT_LEAST_SIZE(xPrintPutDocumentDataReq);

    if((pContext = XP_GETPRIV(client)) == (XpContextPtr)NULL)
        return XpErrorBase+XPBadSequence;

    if(!(pContext->state & DOC_RAW_STARTED) &&
       !(pContext->state & DOC_COOKED_STARTED))
        return XpErrorBase+XPBadSequence;

    if (stuff->drawable) {
	if (pContext->state & DOC_RAW_STARTED)
	    return BadDrawable;
	result = dixLookupDrawable(&pDraw, stuff->drawable, client, 0,
				   DixWriteAccess);
	if (result != Success)
	    return result;
	if (pDraw->pScreen->myNum != pContext->screenNum)
	    return BadDrawable;
    } else {
	if (pContext->state & DOC_COOKED_STARTED)
	    return BadDrawable;
	pDraw = NULL;
    }

    pData = (char *)(&stuff[1]);

    totalSize = (stuff->len_data + 3) >> 2;
    pDoc_fmt = pData + (totalSize << 2);

    totalSize += (stuff->len_fmt + 3) >> 2;
    pOptions = pData + (totalSize << 2);

    totalSize += (stuff->len_options + 3) >> 2;
    if((totalSize + (sz_xPrintPutDocumentDataReq >> 2)) != client->req_len)
	 return BadLength;
    
    if(pContext->funcs.PutDocumentData != 0)
    {
        result = (*pContext->funcs.PutDocumentData)(pContext, pDraw,
					  pData, stuff->len_data,
				          pDoc_fmt, stuff->len_fmt,
				          pOptions, stuff->len_options,
					  client);
    }
    else
	return BadImplementation;

    if (client->noClientException != Success)
        return client->noClientException;
    else
        return result;
}

static int
ProcXpGetDocumentData(ClientPtr client)
{
    REQUEST(xPrintGetDocumentDataReq);
    xPrintGetDocumentDataReply rep;
    XpContextPtr pContext;
    int result = Success;

    REQUEST_SIZE_MATCH(xPrintGetDocumentDataReq);

    if((pContext = (XpContextPtr)SecurityLookupIDByType(client,
							stuff->printContext, 
							RTcontext,
							DixWriteAccess))
       == (XpContextPtr)NULL)
    {
        client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    if(pContext->funcs.GetDocumentData == 0)
	return BadImplementation;

    if(!(pContext->state & JOB_GET_DATA) || 
       pContext->state & GET_DOC_DATA_STARTED)
	return XpErrorBase+XPBadSequence;

    if(stuff->maxBufferSize <= 0)
    {
	client->errorValue = stuff->maxBufferSize;
        return BadValue; /* gotta have a positive buffer size */
    }

    result = (*pContext->funcs.GetDocumentData)(pContext, client, 
						stuff->maxBufferSize);
    if(result != Success)
    {
	rep.type = X_Reply;
	rep.sequenceNumber = client->sequence;
	rep.length = 0;
	rep.dataLen = 0;
	rep.statusCode = 1;
	rep.finishedFlag = TRUE;
        if (client->swapped) {
            int n;
            long l;

            swaps(&rep.sequenceNumber, n);
            swapl(&rep.statusCode, l); /* XXX Why are these longs??? */
            swapl(&rep.finishedFlag, l); /* XXX Why are these longs??? */
        }
	(void)WriteToClient(client,sz_xPrintGetDocumentDataReply,(char *)&rep);
    }
    else
        pContext->state |= GET_DOC_DATA_STARTED;

    if(pContext->clientSlept != (ClientPtr)NULL)
    {
	ClientSignal(pContext->clientSlept);
	ClientWakeup(pContext->clientSlept);
	pContext->clientSlept = (ClientPtr)NULL;
    }

    return result;
}

/*******************************************************************************
 *
 * Attribute requests: GetAttributes, SetAttributes, GetOneAttribute
 *
 ******************************************************************************/

static int 
ProcXpGetAttributes(ClientPtr client)
{
    REQUEST(xPrintGetAttributesReq);
    XpContextPtr pContext;
    char *attrs;
    xPrintGetAttributesReply *pRep;
    int totalSize, n;
    unsigned long l;

    REQUEST_SIZE_MATCH(xPrintGetAttributesReq);

    if(stuff->type < XPJobAttr || stuff->type > XPServerAttr)
    {
	client->errorValue = stuff->type;
	return BadValue;
    }

    if(stuff->type != XPServerAttr)
    {
        if((pContext = (XpContextPtr)SecurityLookupIDByType(
						client,
						stuff->printContext,
						RTcontext,
						DixReadAccess))
	   == (XpContextPtr)NULL)
        {
	    client->errorValue = stuff->printContext;
            return XpErrorBase+XPBadContext;
        }

        if(pContext->funcs.GetAttributes == 0)
	    return BadImplementation;
        if((attrs = (*pContext->funcs.GetAttributes)(pContext, stuff->type)) == 
           (char *)NULL) 
	    return BadAlloc;
    }
    else
    {
	if((attrs = XpGetAttributes((XpContextPtr)NULL, XPServerAttr)) ==
	   (char *)NULL)
	    return BadAlloc;
    }

    totalSize = sz_xPrintGetAttributesReply + QUADPAD(strlen(attrs));
    if((pRep = (xPrintGetAttributesReply *)malloc(totalSize)) ==
       (xPrintGetAttributesReply *)NULL)
	return BadAlloc;

    pRep->type = X_Reply;
    pRep->length = (totalSize - sz_xPrintGetAttributesReply) >> 2;
    pRep->sequenceNumber = client->sequence;
    pRep->stringLen = strlen(attrs);

    if (client->swapped) {
        swaps(&pRep->sequenceNumber, n);
        swapl(&pRep->length, l);
        swapl(&pRep->stringLen, l);
    }

    strncpy((char*)(pRep + 1), attrs, strlen(attrs));
    xfree(attrs);

    WriteToClient(client, totalSize, (char *)pRep);

    xfree(pRep);

    return client->noClientException;
}

static int 
ProcXpSetAttributes(ClientPtr client)
{
    REQUEST(xPrintSetAttributesReq);
    int result = Success;
    XpContextPtr pContext;
    char *attr;

    REQUEST_AT_LEAST_SIZE(xPrintSetAttributesReq);

    if(stuff->type < XPJobAttr || stuff->type > XPServerAttr)
    {
	client->errorValue = stuff->type;
	return BadValue;
    }

    /*
     * Disallow changing of read-only attribute pools
     */
    if(stuff->type == XPPrinterAttr || stuff->type == XPServerAttr)
	return BadMatch;

    if((pContext = (XpContextPtr)SecurityLookupIDByType(
					client,
					stuff->printContext,
					RTcontext,
					DixWriteAccess))
       == (XpContextPtr)NULL)
    {
        client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    if(pContext->funcs.SetAttributes == 0)
	return BadImplementation;
    
    /* 
     * Check for attributes being set after their relevant phase
     * has already begun (e.g. Job attributes set after StartJob).
     */
    if((pContext->state & JOB_STARTED) && stuff->type == XPJobAttr)
	return XpErrorBase+XPBadSequence;
    if(((pContext->state & DOC_RAW_STARTED) || 
       (pContext->state & DOC_COOKED_STARTED)) && stuff->type == XPDocAttr)
	return XpErrorBase+XPBadSequence;
    if((pContext->state & PAGE_STARTED) && stuff->type == XPPageAttr)
	return XpErrorBase+XPBadSequence;

    if((attr = (char *)malloc(stuff->stringLen + 1)) == (char *)NULL)
	return BadAlloc;

    strncpy(attr, (char *)(stuff + 1), stuff->stringLen);
    attr[stuff->stringLen] = (char)'\0';

    if(stuff->rule == XPAttrReplace)
        (*pContext->funcs.SetAttributes)(pContext, stuff->type, attr);
    else if(stuff->rule == XPAttrMerge)
        (*pContext->funcs.AugmentAttributes)(pContext, stuff->type, attr);
    else
    {
	client->errorValue = stuff->rule;
	result = BadValue;
    }

    xfree(attr);

    SendAttributeNotify(pContext, stuff->type);

    return result;
}

static int 
ProcXpGetOneAttribute(ClientPtr client)
{
    REQUEST(xPrintGetOneAttributeReq);
    XpContextPtr pContext;
    char *value, *attrName;
    xPrintGetOneAttributeReply *pRep;
    int totalSize;
    int n;
    unsigned long l;

    REQUEST_AT_LEAST_SIZE(xPrintGetOneAttributeReq);

    totalSize = ((sz_xPrintGetOneAttributeReq) >> 2) +
                ((stuff->nameLen + 3) >> 2);
    if(totalSize != client->req_len)
	 return BadLength;

    if(stuff->type < XPJobAttr || stuff->type > XPServerAttr)
    {
	client->errorValue = stuff->type;
	return BadValue;
    }
    
    if((attrName = (char *)malloc(stuff->nameLen + 1)) == (char *)NULL)
	return BadAlloc;
    strncpy(attrName, (char *)(stuff+1), stuff->nameLen);
    attrName[stuff->nameLen] = (char)'\0';

    if(stuff->type != XPServerAttr)
    {
        if((pContext = (XpContextPtr)SecurityLookupIDByType(
						client,
						stuff->printContext, 
						RTcontext,
						DixReadAccess))
	   == (XpContextPtr)NULL)
        {
	    client->errorValue = stuff->printContext;
            return XpErrorBase+XPBadContext;
        }

        if(pContext->funcs.GetOneAttribute == 0)
	    return BadImplementation;
        if((value = (*pContext->funcs.GetOneAttribute)(pContext, stuff->type,
           attrName)) == (char *)NULL) 
	    return BadAlloc;
    }
    else
    {
	if((value = XpGetOneAttribute((XpContextPtr)NULL, XPServerAttr,
	    attrName)) == (char *)NULL)
	    return BadAlloc;
    }

    free(attrName);

    totalSize = sz_xPrintGetOneAttributeReply + QUADPAD(strlen(value));
    if((pRep = (xPrintGetOneAttributeReply *)malloc(totalSize)) ==
       (xPrintGetOneAttributeReply *)NULL)
	return BadAlloc;

    pRep->type = X_Reply;
    pRep->length = (totalSize - sz_xPrintGetOneAttributeReply) >> 2;
    pRep->sequenceNumber = client->sequence;
    pRep->valueLen = strlen(value);

    if (client->swapped) {
        swaps(&pRep->sequenceNumber, n);
        swapl(&pRep->length, l);
        swapl(&pRep->valueLen, l);
    }

    strncpy((char*)(pRep + 1), value, strlen(value));

    WriteToClient(client, totalSize, (char *)pRep);

    xfree(pRep);

    return client->noClientException;
}

/*******************************************************************************
 *
 * Print Event requests: SelectInput InputSelected, SendXpNotify
 *
 ******************************************************************************/


static int
ProcXpSelectInput(ClientPtr client)
{
    REQUEST(xPrintSelectInputReq);
    int result = Success;
    XpContextPtr pContext;
    XpClientPtr pPrintClient;

    REQUEST_SIZE_MATCH(xPrintSelectInputReq);

    /*
     * Check to see that the supplied XID is really a valid print context
     * in this server.
     */
    if((pContext=(XpContextPtr)SecurityLookupIDByType(client,
						      stuff->printContext,
						      RTcontext,
						      DixWriteAccess))
       == (XpContextPtr)NULL)
    {
	client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    if(stuff->eventMask & ~allEvents)
    {
	client->errorValue = stuff->eventMask;
        return BadValue; /* bogus event mask bits */
    }

    if((pPrintClient = AcquireClient(pContext, client)) == (XpClientPtr)NULL)
	return BadAlloc;

    pPrintClient->eventMask = stuff->eventMask;

    return result;
}

static int
ProcXpInputSelected(ClientPtr client)
{
    REQUEST(xPrintInputSelectedReq);
    xPrintInputSelectedReply rep;
    register int n;
    long l;
    XpClientPtr pXpClient;
    XpContextPtr pContext;

    REQUEST_SIZE_MATCH(xPrintInputSelectedReq);

    if((pContext=(XpContextPtr)SecurityLookupIDByType(client,
						      stuff->printContext,
						      RTcontext,
						      DixReadAccess))
       == (XpContextPtr)NULL)
    {
	client->errorValue = stuff->printContext;
        return XpErrorBase+XPBadContext;
    }

    pXpClient = FindClient(pContext, client);

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.eventMask = (pXpClient != (XpClientPtr)NULL)? pXpClient->eventMask : 0;
    rep.allEventsMask = GetAllEventMasks(pContext);

    if (client->swapped) {
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, l);
        swapl(&rep.eventMask, l);
        swapl(&rep.allEventsMask, l);
    }

    WriteToClient(client, sz_xPrintInputSelectedReply, (char *)&rep);
    return client->noClientException;
}

static void
SendAttributeNotify(XpContextPtr pContext, int which)
{
    XpClientPtr        pXpClient;
    xPrintAttributeEvent   ae;
    ClientPtr	client;

    pXpClient = pContext->clientHead;
    if(pXpClient == (XpClientPtr)NULL) 
        return; /* Nobody's interested in the events (or this context). */

    for (pXpClient = pContext->clientHead; 
         pXpClient != (XpClientPtr)NULL; 
         pXpClient = pXpClient->pNext)
    {
        client = pXpClient->client;
        if (client == serverClient || client->clientGone || 
	    !(pXpClient->eventMask & XPAttributeMask))
            continue;
        ae.type = XPAttributeNotify + XpEventBase;
        ae.detail = which;
        ae.printContext = pContext->contextID;
        ae.sequenceNumber = client->sequence;
        WriteEventsToClient (client, 1, (xEvent *) &ae);
    }
}

static void
SendXpNotify(XpContextPtr pContext, int which, int val)
{
    XpClientPtr        pXpClient;
    xPrintPrintEvent   pe;
    ClientPtr	client;

    pXpClient = pContext->clientHead;
    if(pXpClient == (XpClientPtr)NULL) 
        return; /* Nobody's interested in the events (or this context). */

    for (pXpClient = pContext->clientHead; 
         pXpClient != (XpClientPtr)NULL; 
         pXpClient = pXpClient->pNext)
    {
        client = pXpClient->client;
        if (client == serverClient || client->clientGone || 
	    !(pXpClient->eventMask & XPPrintMask))
            continue;
        pe.type = XPPrintNotify + XpEventBase;
        pe.detail = which;
        pe.printContext = pContext->contextID;
	pe.cancel = (Bool)val;
        pe.sequenceNumber = client->sequence;
        WriteEventsToClient (client, 1, (xEvent *) &pe);
    }
}

static CARD32
GetAllEventMasks(XpContextPtr pContext)
{
    XpClientPtr pPrintClient;
    CARD32 totalMask = (CARD32)0;
    
    for (pPrintClient = pContext->clientHead;
         pPrintClient != (XpClientPtr)NULL;
         pPrintClient = pPrintClient->pNext)
    {
        totalMask |= pPrintClient->eventMask;
    }
    return totalMask;
}

/*
 * XpContextOfClient - returns the XpContextPtr to the context
 * associated with the specified client, or NULL if the client
 * does not currently have a context set.
 */
XpContextPtr
XpContextOfClient(ClientPtr client)
{
    return XP_GETPRIV(client);
}


/*******************************************************************************
 *
 * Swap-request functions
 *
 ******************************************************************************/

static int
SProcXpCreateContext(ClientPtr client)
{
    int i;
    long n;

    REQUEST(xPrintCreateContextReq);

    swaps(&stuff->length, i);
    swapl(&stuff->contextID, n);
    swapl(&stuff->printerNameLen, n);
    swapl(&stuff->localeLen, n);
    return ProcXpCreateContext(client);
}

static int
SProcXpGetPrinterList(ClientPtr client)
{
    int i;
    long n;

    REQUEST(xPrintGetPrinterListReq);

    swaps(&stuff->length, i);
    swapl(&stuff->printerNameLen, n);
    swapl(&stuff->localeLen, n);
    return ProcXpGetPrinterList(client);
}

static int
SProcXpRehashPrinterList(ClientPtr client)
{
    int i;

    REQUEST(xPrintRehashPrinterListReq);
    swaps(&stuff->length, i);
    return ProcXpRehashPrinterList(client);
}

static int
SProcXpSetContext(ClientPtr client)
{
    int i;

    REQUEST(xPrintSetContextReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, i);
    return ProcXpSetContext(client);
}

static int
SProcXpGetContext(ClientPtr client)
{
    int i;

    REQUEST(xPrintGetContextReq);
    swaps(&stuff->length, i);
    return ProcXpGetContext(client);
}

static int
SProcXpDestroyContext(ClientPtr client)
{
    int i;
    long n;

    REQUEST(xPrintDestroyContextReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    return ProcXpDestroyContext(client);
}

static int
SProcXpGetContextScreen(ClientPtr client)
{
    int i;
    long n;

    REQUEST(xPrintGetContextScreenReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    return ProcXpGetContextScreen(client);
}

static int
SProcXpInputSelected(ClientPtr client)
{
    int i;
    long n;

    REQUEST(xPrintInputSelectedReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    return ProcXpInputSelected(client);
}

static int
SProcXpStartJob(ClientPtr client)
{
    int i;

    REQUEST(xPrintStartJobReq);
    swaps(&stuff->length, i);
    return ProcXpStartJob(client);
}

static int
SProcXpEndJob(ClientPtr client)
{
    int i;

    REQUEST(xPrintEndJobReq);
    swaps(&stuff->length, i);
    return ProcXpEndJob(client);
}

static int
SProcXpStartDoc(ClientPtr client)
{
    int i;

    REQUEST(xPrintStartDocReq);
    swaps(&stuff->length, i);
    return ProcXpStartDoc(client);
}

static int
SProcXpEndDoc(ClientPtr client)
{
    int i;

    REQUEST(xPrintEndDocReq);
    swaps(&stuff->length, i);
    return ProcXpEndDoc(client);
}

static int
SProcXpStartPage(ClientPtr client)
{
    int i;
    long n;

    REQUEST(xPrintStartPageReq);
    swaps(&stuff->length, i);
    swapl(&stuff->window, n);
    return ProcXpStartPage(client);
}

static int
SProcXpEndPage(ClientPtr client)
{
    int i;

    REQUEST(xPrintEndPageReq);
    swaps(&stuff->length, i);
    return ProcXpEndPage(client);
}

static int
SProcXpPutDocumentData(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintPutDocumentDataReq);
    swaps(&stuff->length, i);
    swapl(&stuff->drawable, n);
    swapl(&stuff->len_data, n);
    swaps(&stuff->len_fmt, i);
    swaps(&stuff->len_options, i);
    return ProcXpPutDocumentData(client);
}

static int
SProcXpGetDocumentData(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintGetDocumentDataReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    swapl(&stuff->maxBufferSize, n);
    return ProcXpGetDocumentData(client);
}

static int
SProcXpGetAttributes(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintGetAttributesReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    return ProcXpGetAttributes(client);
}

static int
SProcXpSetAttributes(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintSetAttributesReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    swapl(&stuff->stringLen, n);
    return ProcXpSetAttributes(client);
}

static int
SProcXpGetOneAttribute(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintGetOneAttributeReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    swapl(&stuff->nameLen, n);
    return ProcXpGetOneAttribute(client);
}

static int
SProcXpSelectInput(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintSelectInputReq);
    swaps(&stuff->length, i);
    swapl(&stuff->eventMask, n);
    swapl(&stuff->printContext, n);
    return ProcXpSelectInput(client);
}

static int 
SProcXpGetPageDimensions(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintGetPageDimensionsReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    return ProcXpGetPageDimensions(client);
}

static int 
SProcXpSetImageResolution(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintSetImageResolutionReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    swaps(&stuff->imageRes, i);
    return ProcXpSetImageResolution(client);
}

static int 
SProcXpGetImageResolution(ClientPtr client)
{
    long n;
    int i;

    REQUEST(xPrintGetImageResolutionReq);
    swaps(&stuff->length, i);
    swapl(&stuff->printContext, n);
    return ProcXpGetImageResolution(client);
}

static void
SwapXpNotifyEvent(xPrintPrintEvent *src, xPrintPrintEvent *dst)
{
    /*
     * Swap the sequence number and context fields.
     */
    cpswaps(src->sequenceNumber, dst->sequenceNumber);
    cpswapl(src->printContext, dst->printContext);

    /*
     * Copy the byte-long fields.
     */
    dst->type = src->type;
    dst->detail = src->detail;
    dst->cancel = src->cancel;
}

static void
SwapXpAttributeEvent(xPrintAttributeEvent *src, xPrintAttributeEvent *dst)
{
    /*
     * Swap the sequence number and context fields.
     */
    cpswaps(src->sequenceNumber, dst->sequenceNumber);
    cpswapl(src->printContext, dst->printContext);

    /*
     * Copy the byte-long fields.
     */
    dst->type = src->type;
    dst->detail = src->detail;
}
