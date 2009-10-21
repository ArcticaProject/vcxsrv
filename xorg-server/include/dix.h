/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef DIX_H
#define DIX_H

#include "gc.h"
#include "window.h"
#include "input.h"
#include "cursor.h"
#include "geext.h"
#include "events.h"
#include <X11/extensions/XI.h>

#define EARLIER -1
#define SAMETIME 0
#define LATER 1

#define NullClient ((ClientPtr) 0)
#define REQUEST(type) \
	type *stuff = (type *)client->requestBuffer


#define REQUEST_SIZE_MATCH(req)\
    if ((sizeof(req) >> 2) != client->req_len)\
         return(BadLength)

#define REQUEST_AT_LEAST_SIZE(req) \
    if ((sizeof(req) >> 2) > client->req_len )\
         return(BadLength)

#define REQUEST_FIXED_SIZE(req, n)\
    if (((sizeof(req) >> 2) > client->req_len) || \
        (((sizeof(req) + (n) + 3) >> 2) != client->req_len)) \
         return(BadLength)

#define LEGAL_NEW_RESOURCE(id,client)\
    if (!LegalNewID(id,client)) \
    {\
	client->errorValue = id;\
        return(BadIDChoice);\
    }

#define VALIDATE_DRAWABLE_AND_GC(drawID, pDraw, mode)\
    {\
	int rc = dixLookupDrawable(&(pDraw), drawID, client, M_ANY, mode);\
	if (rc != Success)\
	    return rc;\
	rc = dixLookupGC(&(pGC), stuff->gc, client, DixUseAccess);\
	if (rc != Success)\
	    return rc;\
	if ((pGC->depth != pDraw->depth) || (pGC->pScreen != pDraw->pScreen))\
	    return (BadMatch);\
    }\
    if (pGC->serialNumber != pDraw->serialNumber)\
	ValidateGC(pDraw, pGC);


#define WriteReplyToClient(pClient, size, pReply) { \
   if ((pClient)->swapped) \
      (*ReplySwapVector[((xReq *)(pClient)->requestBuffer)->reqType]) \
           (pClient, (int)(size), pReply); \
      else (void) WriteToClient(pClient, (int)(size), (char *)(pReply)); }

#define WriteSwappedDataToClient(pClient, size, pbuf) \
   if ((pClient)->swapped) \
      (*(pClient)->pSwapReplyFunc)(pClient, (int)(size), pbuf); \
   else (void) WriteToClient (pClient, (int)(size), (char *)(pbuf));

typedef struct _TimeStamp *TimeStampPtr;

#ifndef _XTYPEDEF_CLIENTPTR
typedef struct _Client *ClientPtr; /* also in misc.h */
#define _XTYPEDEF_CLIENTPTR
#endif

typedef struct _WorkQueue	*WorkQueuePtr;

extern _X_EXPORT ClientPtr clients[MAXCLIENTS];
extern _X_EXPORT ClientPtr serverClient;
extern _X_EXPORT int currentMaxClients;
extern _X_EXPORT char dispatchExceptionAtReset;

typedef int HWEventQueueType;
typedef HWEventQueueType* HWEventQueuePtr;

extern _X_EXPORT HWEventQueuePtr checkForInput[2];

typedef struct _TimeStamp {
    CARD32 months;	/* really ~49.7 days */
    CARD32 milliseconds;
}           TimeStamp;

/* dispatch.c */

extern _X_EXPORT void SetInputCheck(
    HWEventQueuePtr /*c0*/,
    HWEventQueuePtr /*c1*/);

extern _X_EXPORT void CloseDownClient(
    ClientPtr /*client*/);

extern _X_EXPORT void UpdateCurrentTime(void);

extern _X_EXPORT void UpdateCurrentTimeIf(void);

extern _X_EXPORT int dixDestroyPixmap(
    pointer /*value*/,
    XID /*pid*/);

extern _X_EXPORT void InitClient(
    ClientPtr /*client*/,
    int /*i*/,
    pointer /*ospriv*/);

extern _X_EXPORT ClientPtr NextAvailableClient(
    pointer /*ospriv*/);

extern _X_EXPORT void SendErrorToClient(
    ClientPtr /*client*/,
    unsigned int /*majorCode*/,
    unsigned int /*minorCode*/,
    XID /*resId*/,
    int /*errorCode*/);

extern _X_EXPORT void MarkClientException(
    ClientPtr /*client*/);

extern _X_HIDDEN Bool CreateConnectionBlock(void);
/* dixutils.c */

extern _X_EXPORT int CompareISOLatin1Lowered(
    unsigned char * /*a*/,
    int alen,
    unsigned char * /*b*/,
    int blen);

extern _X_EXPORT int dixLookupWindow(
    WindowPtr *result,
    XID id,
    ClientPtr client,
    Mask access_mode);

extern _X_EXPORT int dixLookupDrawable(
    DrawablePtr *result,
    XID id,
    ClientPtr client,
    Mask type_mask,
    Mask access_mode);

extern _X_EXPORT int dixLookupGC(
    GCPtr *result,
    XID id,
    ClientPtr client,
    Mask access_mode);

extern _X_EXPORT int dixLookupClient(
    ClientPtr *result,
    XID id,
    ClientPtr client,
    Mask access_mode);

extern _X_EXPORT void NoopDDA(void);

extern _X_EXPORT int AlterSaveSetForClient(
    ClientPtr /*client*/,
    WindowPtr /*pWin*/,
    unsigned /*mode*/,
    Bool /*toRoot*/,
    Bool /*map*/);
  
extern _X_EXPORT void DeleteWindowFromAnySaveSet(
    WindowPtr /*pWin*/);

extern _X_EXPORT void BlockHandler(
    pointer /*pTimeout*/,
    pointer /*pReadmask*/);

extern _X_EXPORT void WakeupHandler(
    int /*result*/,
    pointer /*pReadmask*/);

void
EnableLimitedSchedulingLatency(void);

void
DisableLimitedSchedulingLatency(void);

typedef void (* WakeupHandlerProcPtr)(
    pointer /* blockData */,
    int /* result */,
    pointer /* pReadmask */);

extern _X_EXPORT Bool RegisterBlockAndWakeupHandlers(
    BlockHandlerProcPtr /*blockHandler*/,
    WakeupHandlerProcPtr /*wakeupHandler*/,
    pointer /*blockData*/);

extern _X_EXPORT void RemoveBlockAndWakeupHandlers(
    BlockHandlerProcPtr /*blockHandler*/,
    WakeupHandlerProcPtr /*wakeupHandler*/,
    pointer /*blockData*/);

extern _X_EXPORT void InitBlockAndWakeupHandlers(void);

extern _X_EXPORT void ProcessWorkQueue(void);

extern _X_EXPORT void ProcessWorkQueueZombies(void);

extern _X_EXPORT Bool QueueWorkProc(
    Bool (* /*function*/)(
        ClientPtr /*clientUnused*/,
        pointer /*closure*/),
    ClientPtr /*client*/,
    pointer /*closure*/
);

typedef Bool (* ClientSleepProcPtr)(
    ClientPtr /*client*/,
    pointer /*closure*/);

extern _X_EXPORT Bool ClientSleep(
    ClientPtr /*client*/,
    ClientSleepProcPtr /* function */,
    pointer /*closure*/);

#ifndef ___CLIENTSIGNAL_DEFINED___
#define ___CLIENTSIGNAL_DEFINED___
extern _X_EXPORT Bool ClientSignal(
    ClientPtr /*client*/);
#endif /* ___CLIENTSIGNAL_DEFINED___ */

extern _X_EXPORT void ClientWakeup(
    ClientPtr /*client*/);

extern _X_EXPORT Bool ClientIsAsleep(
    ClientPtr /*client*/);

/* atom.c */

extern _X_EXPORT Atom MakeAtom(
    const char * /*string*/,
    unsigned /*len*/,
    Bool /*makeit*/);

extern _X_EXPORT Bool ValidAtom(
    Atom /*atom*/);

extern _X_EXPORT const char *NameForAtom(
    Atom /*atom*/);

extern _X_EXPORT void AtomError(void);

extern _X_EXPORT void FreeAllAtoms(void);

extern _X_EXPORT void InitAtoms(void);

/* main.c */

extern _X_EXPORT void SetVendorRelease(int release);

extern _X_EXPORT void SetVendorString(char *string);

/* events.c */

extern void SetMaskForEvent(
    int /* deviceid */,
    Mask /* mask */,
    int /* event */);

extern _X_EXPORT void ConfineToShape(
    DeviceIntPtr /* pDev */, 
    RegionPtr /* shape */, 
    int*      /* px */,
    int*      /* py */);

extern _X_EXPORT Bool IsParent(
    WindowPtr /* maybeparent */,
    WindowPtr /* child */);

extern _X_EXPORT WindowPtr GetCurrentRootWindow(DeviceIntPtr pDev);

extern _X_EXPORT WindowPtr GetSpriteWindow(DeviceIntPtr pDev);


extern _X_EXPORT void NoticeEventTime(InternalEvent *ev);

extern void EnqueueEvent(
    InternalEvent * /* ev */,
    DeviceIntPtr  /* device */);

extern void ActivatePointerGrab(
    DeviceIntPtr /* mouse */,
    GrabPtr /* grab */,
    TimeStamp /* time */,
    Bool /* autoGrab */);

extern void DeactivatePointerGrab(
    DeviceIntPtr /* mouse */);

extern void ActivateKeyboardGrab(
    DeviceIntPtr /* keybd */,
    GrabPtr /* grab */,
    TimeStamp /* time */,
    Bool /* passive */);

extern void DeactivateKeyboardGrab(
    DeviceIntPtr /* keybd */);

extern BOOL ActivateFocusInGrab(
    DeviceIntPtr /* dev */,
    WindowPtr /* old */,
    WindowPtr /* win */);

extern void AllowSome(
    ClientPtr	/* client */,
    TimeStamp /* time */,
    DeviceIntPtr /* thisDev */,
    int /* newState */);

extern void ReleaseActiveGrabs(
    ClientPtr client);

extern _X_EXPORT int DeliverEventsToWindow(
    DeviceIntPtr /* pWin */,
    WindowPtr /* pWin */,
    xEventPtr /* pEvents */,
    int /* count */,
    Mask /* filter */,
    GrabPtr /* grab */);

extern int DeliverDeviceEvents(
    WindowPtr /* pWin */,
    InternalEvent* /* event */,
    GrabPtr /* grab */,
    WindowPtr /* stopAt */,
    DeviceIntPtr /* dev */);

extern void InitializeSprite(
    DeviceIntPtr /* pDev */,
    WindowPtr    /* pWin */);

extern void UpdateSpriteForScreen(
    DeviceIntPtr /* pDev */,
    ScreenPtr /* pScreen */);

extern _X_EXPORT void WindowHasNewCursor(
    WindowPtr /* pWin */);

extern Bool CheckDeviceGrabs(
    DeviceIntPtr /* device */,
    DeviceEvent* /* event */,
    int /* checkFirst */);

extern void DeliverFocusedEvent(
    DeviceIntPtr /* keybd */,
    InternalEvent* /* event */,
    WindowPtr /* window */);

extern void DeliverGrabbedEvent(
    InternalEvent* /* event */,
    DeviceIntPtr /* thisDev */,
    Bool /* deactivateGrab */);

extern void FixKeyState(
    DeviceEvent* /* event */,
    DeviceIntPtr /* keybd */);

extern void RecalculateDeliverableEvents(
    WindowPtr /* pWin */);

extern _X_EXPORT int OtherClientGone(
    pointer /* value */,
    XID /* id */);

extern void DoFocusEvents(
    DeviceIntPtr /* dev */,
    WindowPtr /* fromWin */,
    WindowPtr /* toWin */,
    int /* mode */);

extern int SetInputFocus(
    ClientPtr /* client */,
    DeviceIntPtr /* dev */,
    Window /* focusID */,
    CARD8 /* revertTo */,
    Time /* ctime */,
    Bool /* followOK */);

extern int GrabDevice(
    ClientPtr /* client */,
    DeviceIntPtr /* dev */,
    unsigned /* this_mode */,
    unsigned /* other_mode */,
    Window /* grabWindow */,
    unsigned /* ownerEvents */,
    Time /* ctime */,
    GrabMask* /* mask */,
    int /* grabtype */,
    Cursor /* curs */,
    Window /* confineToWin */,
    CARD8 * /* status */);

extern void InitEvents(void);

extern void CloseDownEvents(void);

extern void DeleteWindowFromAnyEvents(
    WindowPtr	/* pWin */,
    Bool /* freeResources */);


extern Mask EventMaskForClient(
    WindowPtr /* pWin */,
    ClientPtr /* client */);



extern _X_EXPORT int DeliverEvents(
    WindowPtr /*pWin*/,
    xEventPtr /*xE*/,
    int /*count*/,
    WindowPtr /*otherParent*/);

extern Bool CheckMotion(
    DeviceEvent* /* ev */,
    DeviceIntPtr /* pDev */);

extern _X_EXPORT void WriteEventsToClient(
    ClientPtr /*pClient*/,
    int	     /*count*/,
    xEventPtr /*events*/);

extern _X_EXPORT int TryClientEvents(
    ClientPtr /*client*/,
    DeviceIntPtr /* device */,
    xEventPtr /*pEvents*/,
    int /*count*/,
    Mask /*mask*/,
    Mask /*filter*/,
    GrabPtr /*grab*/);

extern _X_EXPORT void WindowsRestructured(void);

extern int SetClientPointer(
        ClientPtr /* client */,
        DeviceIntPtr /* device */);

extern _X_EXPORT DeviceIntPtr PickPointer(
    ClientPtr /* client */);

extern _X_EXPORT DeviceIntPtr PickKeyboard(
    ClientPtr /* client */);

extern Bool IsInterferingGrab(
        ClientPtr /* client */,
        DeviceIntPtr /* dev */,
        xEvent* /* events */);

#ifdef PANORAMIX
extern _X_EXPORT void ReinitializeRootWindow(WindowPtr win, int xoff, int yoff);
#endif

#ifdef RANDR
extern _X_EXPORT void
ScreenRestructured (ScreenPtr pScreen);
#endif

extern _X_EXPORT int ffs(int i);

/*
 *  callback manager stuff
 */

#ifndef _XTYPEDEF_CALLBACKLISTPTR
typedef struct _CallbackList *CallbackListPtr; /* also in misc.h */
#define _XTYPEDEF_CALLBACKLISTPTR
#endif

typedef void (*CallbackProcPtr) (
    CallbackListPtr *, pointer, pointer);

extern _X_EXPORT Bool AddCallback(
    CallbackListPtr * /*pcbl*/,
    CallbackProcPtr /*callback*/,
    pointer /*data*/);

extern _X_EXPORT Bool DeleteCallback(
    CallbackListPtr * /*pcbl*/,
    CallbackProcPtr /*callback*/,
    pointer /*data*/);

extern _X_EXPORT void CallCallbacks(
    CallbackListPtr * /*pcbl*/,
    pointer /*call_data*/);

extern _X_EXPORT void DeleteCallbackList(
    CallbackListPtr * /*pcbl*/);

extern _X_EXPORT void InitCallbackManager(void);

/*
 *  ServerGrabCallback stuff
 */

extern _X_EXPORT CallbackListPtr ServerGrabCallback;

typedef enum {SERVER_GRABBED, SERVER_UNGRABBED,
	      CLIENT_PERVIOUS, CLIENT_IMPERVIOUS } ServerGrabState;

typedef struct {
    ClientPtr client;
    ServerGrabState grabstate;
} ServerGrabInfoRec;

/*
 *  EventCallback stuff
 */

extern _X_EXPORT CallbackListPtr EventCallback;

typedef struct {
    ClientPtr client;
    xEventPtr events;
    int count;
} EventInfoRec;

/*
 *  DeviceEventCallback stuff
 */

extern _X_EXPORT CallbackListPtr DeviceEventCallback;

typedef struct {
    xEventPtr events;
    int count;
} DeviceEventInfoRec;

extern int XItoCoreType(int xi_type);
extern Bool DevHasCursor(DeviceIntPtr pDev);
extern Bool _X_EXPORT IsPointerDevice( DeviceIntPtr dev);
extern Bool _X_EXPORT IsKeyboardDevice(DeviceIntPtr dev);
extern Bool IsPointerEvent(InternalEvent *event);
extern Bool IsMaster(DeviceIntPtr dev);

extern _X_HIDDEN void CopyKeyClass(DeviceIntPtr device, DeviceIntPtr master);
extern _X_HIDDEN int CorePointerProc(DeviceIntPtr dev, int what);
extern _X_HIDDEN int CoreKeyboardProc(DeviceIntPtr dev, int what);


/*
 * These are deprecated compatibility functions and will be removed soon!
 * Please use the noted replacements instead.
 */
/* replaced by dixLookupWindow */
extern _X_EXPORT WindowPtr SecurityLookupWindow(
    XID id,
    ClientPtr client,
    Mask access_mode);
/* replaced by dixLookupWindow */
extern _X_EXPORT WindowPtr LookupWindow(
    XID id,
    ClientPtr client);

/* replaced by dixLookupDrawable */
extern _X_EXPORT pointer SecurityLookupDrawable(
    XID id,
    ClientPtr client,
    Mask access_mode);

/* replaced by dixLookupDrawable */
extern _X_EXPORT pointer LookupDrawable(
    XID id,
    ClientPtr client);

/* replaced by dixLookupClient */
extern _X_EXPORT ClientPtr LookupClient(
    XID id,
    ClientPtr client);

#endif /* DIX_H */
