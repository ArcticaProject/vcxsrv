/* WindowsWM extension is based on AppleWM extension */
/**************************************************************************

Copyright (c) 2002 Apple Computer, Inc. All Rights Reserved.
Copyright (c) 2003 Torrey T. Lyons. All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include "win.h"

#define NEED_REPLIES
#define NEED_EVENTS
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "swaprep.h"
#define _WINDOWSWM_SERVER_
#include "windowswmstr.h"

static int WMErrorBase;

static DISPATCH_PROC(ProcWindowsWMDispatch);
static DISPATCH_PROC(SProcWindowsWMDispatch);

static unsigned char WMReqCode = 0;
static int WMEventBase = 0;

static RESTYPE ClientType, EventType; /* resource types for event masks */
static XID eventResource;

/* Currently selected events */
static unsigned int eventMask = 0;

static int WMFreeClient (pointer data, XID id);
static int WMFreeEvents (pointer data, XID id);
static void SNotifyEvent(xWindowsWMNotifyEvent *from, xWindowsWMNotifyEvent *to);

typedef struct _WMEvent *WMEventPtr;
typedef struct _WMEvent {
  WMEventPtr      next;
  ClientPtr	    client;
  XID		    clientResource;
  unsigned int    mask;
} WMEventRec;

static inline BoxRec
make_box (int x, int y, int w, int h)
{
  BoxRec r;
  r.x1 = x;
  r.y1 = y;
  r.x2 = x + w;
  r.y2 = y + h;
  return r;
}

void
winWindowsWMExtensionInit ()
{
  ExtensionEntry* extEntry;

  ClientType = CreateNewResourceType(WMFreeClient);
  EventType = CreateNewResourceType(WMFreeEvents);
  eventResource = FakeClientID(0);

  if (ClientType && EventType &&
      (extEntry = AddExtension(WINDOWSWMNAME,
			       WindowsWMNumberEvents,
			       WindowsWMNumberErrors,
			       ProcWindowsWMDispatch,
			       SProcWindowsWMDispatch,
			       NULL,
			       StandardMinorOpcode)))
    {
      WMReqCode = (unsigned char)extEntry->base;
      WMErrorBase = extEntry->errorBase;
      WMEventBase = extEntry->eventBase;
      EventSwapVector[WMEventBase] = (EventSwapPtr) SNotifyEvent;
    }
}

static int
ProcWindowsWMQueryVersion(register ClientPtr client)
{
  xWindowsWMQueryVersionReply rep;
  register int n;

  REQUEST_SIZE_MATCH(xWindowsWMQueryVersionReq);
  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  rep.majorVersion = WINDOWS_WM_MAJOR_VERSION;
  rep.minorVersion = WINDOWS_WM_MINOR_VERSION;
  rep.patchVersion = WINDOWS_WM_PATCH_VERSION;
  if (client->swapped)
    {
      swaps(&rep.sequenceNumber, n);
      swapl(&rep.length, n);
    }
  WriteToClient(client, sizeof(xWindowsWMQueryVersionReply), (char *)&rep);
  return (client->noClientException);
}


/* events */

static inline void
updateEventMask (WMEventPtr *pHead)
{
  WMEventPtr pCur;

  eventMask = 0;
  for (pCur = *pHead; pCur != NULL; pCur = pCur->next)
    eventMask |= pCur->mask;
}

/*ARGSUSED*/
static int
WMFreeClient (pointer data, XID id)
{
  WMEventPtr   pEvent;
  WMEventPtr   *pHead, pCur, pPrev;

  pEvent = (WMEventPtr) data;
  pHead = (WMEventPtr *) LookupIDByType(eventResource, EventType);
  if (pHead)
    {
      pPrev = 0;
      for (pCur = *pHead; pCur && pCur != pEvent; pCur=pCur->next)
	pPrev = pCur;
      if (pCur)
	{
	  if (pPrev)
	    pPrev->next = pEvent->next;
	  else
	    *pHead = pEvent->next;
	}
      updateEventMask (pHead);
    }
  xfree ((pointer) pEvent);
  return 1;
}

/*ARGSUSED*/
static int
WMFreeEvents (pointer data, XID id)
{
  WMEventPtr   *pHead, pCur, pNext;
  
  pHead = (WMEventPtr *) data;
  for (pCur = *pHead; pCur; pCur = pNext)
    {
      pNext = pCur->next;
      FreeResource (pCur->clientResource, ClientType);
      xfree ((pointer) pCur);
    }
  xfree ((pointer) pHead);
  eventMask = 0;
  return 1;
}

static int
ProcWindowsWMSelectInput (register ClientPtr client)
{
  REQUEST(xWindowsWMSelectInputReq);
  WMEventPtr		pEvent, pNewEvent, *pHead;
  XID			clientResource;

  REQUEST_SIZE_MATCH (xWindowsWMSelectInputReq);
  pHead = (WMEventPtr *)SecurityLookupIDByType(client, eventResource,
					       EventType, DixWriteAccess);
  if (stuff->mask != 0)
    {
      if (pHead)
	{
	  /* check for existing entry. */
	  for (pEvent = *pHead; pEvent; pEvent = pEvent->next)
	    {
	      if (pEvent->client == client)
		{
		  pEvent->mask = stuff->mask;
		  updateEventMask (pHead);
		  return Success;
		}
	    }
	}
      
      /* build the entry */
      pNewEvent = (WMEventPtr) xalloc (sizeof (WMEventRec));
      if (!pNewEvent)
	return BadAlloc;
      pNewEvent->next = 0;
      pNewEvent->client = client;
      pNewEvent->mask = stuff->mask;
      /*
       * add a resource that will be deleted when
       * the client goes away
       */
      clientResource = FakeClientID (client->index);
      pNewEvent->clientResource = clientResource;
      if (!AddResource (clientResource, ClientType, (pointer)pNewEvent))
	return BadAlloc;
      /*
       * create a resource to contain a pointer to the list
       * of clients selecting input.  This must be indirect as
       * the list may be arbitrarily rearranged which cannot be
       * done through the resource database.
       */
      if (!pHead)
	{
	  pHead = (WMEventPtr *) xalloc (sizeof (WMEventPtr));
	  if (!pHead ||
	      !AddResource (eventResource, EventType, (pointer)pHead))
	    {
	      FreeResource (clientResource, RT_NONE);
	      return BadAlloc;
	    }
	  *pHead = 0;
	}
      pNewEvent->next = *pHead;
      *pHead = pNewEvent;
      updateEventMask (pHead);
    }
  else if (stuff->mask == 0)
    {
      /* delete the interest */
      if (pHead)
	{
	  pNewEvent = 0;
	  for (pEvent = *pHead; pEvent; pEvent = pEvent->next)
	    {
	      if (pEvent->client == client)
		break;
	      pNewEvent = pEvent;
	    }
	  if (pEvent)
	    {
	      FreeResource (pEvent->clientResource, ClientType);
	      if (pNewEvent)
		pNewEvent->next = pEvent->next;
	      else
		*pHead = pEvent->next;
	      xfree (pEvent);
	      updateEventMask (pHead);
	    }
	}
    }
  else
    {
      client->errorValue = stuff->mask;
      return BadValue;
    }
  return Success;
}

/*
 * deliver the event
 */

void
winWindowsWMSendEvent (int type, unsigned int mask, int which, int arg,
		       Window window, int x, int y, int w, int h)
{
  WMEventPtr		*pHead, pEvent;
  ClientPtr		client;
  xWindowsWMNotifyEvent se;
#if CYGMULTIWINDOW_DEBUG
  ErrorF ("winWindowsWMSendEvent %d %d %d %d,  %d %d - %d %d\n",
	  type, mask, which, arg, x, y, w, h);
#endif
  pHead = (WMEventPtr *) LookupIDByType(eventResource, EventType);
  if (!pHead)
    return;
  for (pEvent = *pHead; pEvent; pEvent = pEvent->next)
    {
      client = pEvent->client;
#if CYGMULTIWINDOW_DEBUG
      ErrorF ("winWindowsWMSendEvent - x%08x\n", (int) client);
#endif
      if ((pEvent->mask & mask) == 0
	  || client == serverClient || client->clientGone)
	{
	  continue;
	}
#if CYGMULTIWINDOW_DEBUG 
      ErrorF ("winWindowsWMSendEvent - send\n");
#endif
      se.type = type + WMEventBase;
      se.kind = which;
      se.window = window;
      se.arg = arg;
      se.x = x;
      se.y = y;
      se.w = w;
      se.h = h;
      se.sequenceNumber = client->sequence;
      se.time = currentTime.milliseconds;
      WriteEventsToClient (client, 1, (xEvent *) &se);
    }
}

/* Safe to call from any thread. */
unsigned int
WindowsWMSelectedEvents (void)
{
  return eventMask;
}


/* general utility functions */

static int
ProcWindowsWMDisableUpdate (register ClientPtr client)
{
  REQUEST_SIZE_MATCH(xWindowsWMDisableUpdateReq);

  //winDisableUpdate();

  return (client->noClientException);
}

static int
ProcWindowsWMReenableUpdate (register ClientPtr client)
{
  REQUEST_SIZE_MATCH(xWindowsWMReenableUpdateReq);

  //winEnableUpdate(); 

  return (client->noClientException);
}


/* window functions */

static int
ProcWindowsWMSetFrontProcess (register ClientPtr client)
{
  REQUEST_SIZE_MATCH(xWindowsWMSetFrontProcessReq);
  
  //QuartzMessageMainThread(kWindowsSetFrontProcess, NULL, 0);
  
  return (client->noClientException);
}


/* frame functions */

static int
ProcWindowsWMFrameGetRect (register ClientPtr client)
{
  xWindowsWMFrameGetRectReply rep;
  BoxRec ir;
  RECT rcNew;
  REQUEST(xWindowsWMFrameGetRectReq);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameGetRect %d %d\n",
	  (sizeof(xWindowsWMFrameGetRectReq) >> 2), (int) client->req_len);
#endif
  
  REQUEST_SIZE_MATCH(xWindowsWMFrameGetRectReq);
  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;

  ir = make_box (stuff->ix, stuff->iy, stuff->iw, stuff->ih);

  if (stuff->frame_rect != 0)
    {
      ErrorF ("ProcWindowsWMFrameGetRect - stuff->frame_rect != 0\n");
      return BadValue;
    }

  /* Store the origin, height, and width in a rectangle structure */
  SetRect (&rcNew, stuff->ix, stuff->iy,
	   stuff->ix + stuff->iw, stuff->iy + stuff->ih);
    
#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameGetRect - %d %d %d %d\n",
	  stuff->ix, stuff->iy, stuff->ix + stuff->iw, stuff->iy + stuff->ih);
#endif

  /*
   * Calculate the required size of the Windows window rectangle,
   * given the size of the Windows window client area.
   */
  AdjustWindowRectEx (&rcNew, stuff->frame_style, FALSE, stuff->frame_style_ex);
  rep.x = rcNew.left;
  rep.y = rcNew.top;
  rep.w = rcNew.right - rcNew.left;
  rep.h = rcNew.bottom - rcNew.top;
#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameGetRect - %d %d %d %d\n",
	  rep.x, rep.y, rep.w, rep.h);
#endif

  WriteToClient(client, sizeof(xWindowsWMFrameGetRectReply), (char *)&rep);
  return (client->noClientException);
}


static int
ProcWindowsWMFrameDraw (register ClientPtr client)
{
  REQUEST(xWindowsWMFrameDrawReq);
  WindowPtr pWin;
  win32RootlessWindowPtr pRLWinPriv;
  RECT rcNew;
  int nCmdShow, rc;
  RegionRec newShape;
  ScreenPtr pScreen;

  REQUEST_SIZE_MATCH (xWindowsWMFrameDrawReq);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameDraw\n");
#endif
  rc = dixLookupWindow(&pWin, stuff->window, client, DixReadAccess);
  if (rc != Success)
      return rc;
#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameDraw - Window found\n");
#endif

  pRLWinPriv = (win32RootlessWindowPtr) RootlessFrameForWindow (pWin, TRUE);
  if (pRLWinPriv == 0) return BadWindow;

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameDraw - HWND 0x%08x 0x%08x 0x%08x\n",
	  (int) pRLWinPriv->hWnd, (int) stuff->frame_style,
	  (int) stuff->frame_style_ex);
  ErrorF ("ProcWindowsWMFrameDraw - %d %d %d %d\n",
	  stuff->ix, stuff->iy, stuff->iw, stuff->ih);
#endif

  /* Store the origin, height, and width in a rectangle structure */
  SetRect (&rcNew, stuff->ix, stuff->iy,
	   stuff->ix + stuff->iw, stuff->iy + stuff->ih);

  /*
   * Calculate the required size of the Windows window rectangle,
   * given the size of the Windows window client area.
   */
  AdjustWindowRectEx (&rcNew, stuff->frame_style, FALSE, stuff->frame_style_ex);
  
  /* Set the window extended style flags */
  if (!SetWindowLongPtr (pRLWinPriv->hWnd, GWL_EXSTYLE, stuff->frame_style_ex))
    {
      return BadValue;
    }

  /* Set the window standard style flags */
  if (!SetWindowLongPtr (pRLWinPriv->hWnd, GWL_STYLE, stuff->frame_style))
    {
      return BadValue;
    }

  /* Flush the window style */
  if (!SetWindowPos (pRLWinPriv->hWnd, NULL,
		     rcNew.left, rcNew.top,
		     rcNew.right - rcNew.left, rcNew.bottom - rcNew.top,
		     SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE))
    {
      return BadValue;
    }
  if (!IsWindowVisible(pRLWinPriv->hWnd))
    nCmdShow = SW_HIDE;
  else 
    nCmdShow = SW_SHOWNA;

  ShowWindow (pRLWinPriv->hWnd, nCmdShow);

  winMWExtWMUpdateIcon (pWin->drawable.id);

  if (wBoundingShape(pWin) != NULL)
    {
      pScreen = pWin->drawable.pScreen;
      /* wBoundingShape is relative to *inner* origin of window.
	 Translate by borderWidth to get the outside-relative position. */
      
      REGION_NULL(pScreen, &newShape);
      REGION_COPY(pScreen, &newShape, wBoundingShape(pWin));
      REGION_TRANSLATE(pScreen, &newShape, pWin->borderWidth, pWin->borderWidth);
      winMWExtWMReshapeFrame (pRLWinPriv, &newShape);
      REGION_UNINIT(pScreen, &newShape);
    }
#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameDraw - done\n");
#endif

  return (client->noClientException);
}

static int
ProcWindowsWMFrameSetTitle(
			   register ClientPtr client
			   )
{
  unsigned int title_length, title_max;
  unsigned char *title_bytes;
  REQUEST(xWindowsWMFrameSetTitleReq);
  WindowPtr pWin;
  win32RootlessWindowPtr pRLWinPriv;
  int rc;

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameSetTitle\n");
#endif

  REQUEST_AT_LEAST_SIZE(xWindowsWMFrameSetTitleReq);

  rc = dixLookupWindow(&pWin, stuff->window, client, DixReadAccess);
  if (rc != Success)
      return rc;
#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameSetTitle - Window found\n");
#endif

  title_length = stuff->title_length;
  title_max = (stuff->length << 2) - sizeof(xWindowsWMFrameSetTitleReq);

  if (title_max < title_length)
    return BadValue;

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameSetTitle - length is valid\n");
#endif

  title_bytes = malloc (title_length+1);
  strncpy (title_bytes, (unsigned char *) &stuff[1], title_length);
  title_bytes[title_length] = '\0';

  pRLWinPriv = (win32RootlessWindowPtr) RootlessFrameForWindow (pWin, FALSE);

  if (pRLWinPriv == 0)
    {
      free (title_bytes);
      return BadWindow;
    }
    
  /* Flush the window style */
  SetWindowText (pRLWinPriv->hWnd, title_bytes);

  free (title_bytes);

#if CYGMULTIWINDOW_DEBUG
  ErrorF ("ProcWindowsWMFrameSetTitle - done\n");
#endif

  return (client->noClientException);
}


/* dispatch */

static int
ProcWindowsWMDispatch (register ClientPtr client)
{
  REQUEST(xReq);

  switch (stuff->data)
    {
    case X_WindowsWMQueryVersion:
      return ProcWindowsWMQueryVersion(client);
    }

  if (!LocalClient(client))
    return WMErrorBase + WindowsWMClientNotLocal;

  switch (stuff->data)
    {
    case X_WindowsWMSelectInput:
      return ProcWindowsWMSelectInput(client);
    case X_WindowsWMDisableUpdate:
      return ProcWindowsWMDisableUpdate(client);
    case X_WindowsWMReenableUpdate:
      return ProcWindowsWMReenableUpdate(client);
    case X_WindowsWMSetFrontProcess:
      return ProcWindowsWMSetFrontProcess(client);
    case X_WindowsWMFrameGetRect:
      return ProcWindowsWMFrameGetRect(client);
    case X_WindowsWMFrameDraw:
      return ProcWindowsWMFrameDraw(client);
    case X_WindowsWMFrameSetTitle:
      return ProcWindowsWMFrameSetTitle(client);
    default:
      return BadRequest;
    }
}

static void
SNotifyEvent (xWindowsWMNotifyEvent *from, xWindowsWMNotifyEvent *to)
{
  to->type = from->type;
  to->kind = from->kind;
  cpswaps (from->sequenceNumber, to->sequenceNumber);
  cpswapl (from->window, to->window);
  cpswapl (from->time, to->time);
  cpswapl (from->arg, to->arg);
}

static int
SProcWindowsWMQueryVersion (register ClientPtr client)
{
  register int n;
  REQUEST(xWindowsWMQueryVersionReq);
  swaps(&stuff->length, n);
  return ProcWindowsWMQueryVersion(client);
}

static int
SProcWindowsWMDispatch (register ClientPtr client)
{
  REQUEST(xReq);

  /* It is bound to be non-local when there is byte swapping */
  if (!LocalClient(client))
    return WMErrorBase + WindowsWMClientNotLocal;

  /* only local clients are allowed WM access */
  switch (stuff->data)
    {
    case X_WindowsWMQueryVersion:
      return SProcWindowsWMQueryVersion(client);
    default:
      return BadRequest;
    }
}
