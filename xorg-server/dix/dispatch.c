/************************************************************

Copyright 1987, 1989, 1998  The Open Group

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


Copyright 1987, 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

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

********************************************************/

/* The panoramix components contained the following notice */
/*****************************************************************

Copyright (c) 1991, 1997 Digital Equipment Corporation, Maynard, Massachusetts.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
DIGITAL EQUIPMENT CORPORATION BE LIABLE FOR ANY CLAIM, DAMAGES, INCLUDING,
BUT NOT LIMITED TO CONSEQUENTIAL OR INCIDENTAL DAMAGES, OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Digital Equipment Corporation
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Digital
Equipment Corporation.

******************************************************************/

/* XSERVER_DTRACE additions:
 * Copyright 2005-2006 Sun Microsystems, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * of the copyright holder.
 */



#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef PANORAMIX_DEBUG
#include <stdio.h>
int ProcInitialConnection();
#endif

#include "windowstr.h"
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"
#include "gcstruct.h"
#include "selection.h"
#include "colormapst.h"
#include "cursorstr.h"
#include "scrnintstr.h"
#include "opaque.h"
#include "input.h"
#include "servermd.h"
#include "extnsionst.h"
#include "dixfont.h"
#include "dispatch.h"
#include "swaprep.h"
#include "swapreq.h"
#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif
#include "privates.h"
#include "xace.h"
#ifdef XKB
#ifndef XKB_IN_SERVER
#define XKB_IN_SERVER
#endif
#include "inputstr.h"
#include <xkbsrv.h>
#endif

#ifdef XSERVER_DTRACE
#include "registry.h"
#include <sys/types.h>
typedef const char *string;
#include "Xserver-dtrace.h"
#endif

#define mskcnt ((MAXCLIENTS + 31) / 32)
#define BITMASK(i) (1U << ((i) & 31))
#define MASKIDX(i) ((i) >> 5)
#define MASKWORD(buf, i) buf[MASKIDX(i)]
#define BITSET(buf, i) MASKWORD(buf, i) |= BITMASK(i)
#define BITCLEAR(buf, i) MASKWORD(buf, i) &= ~BITMASK(i)
#define GETBIT(buf, i) (MASKWORD(buf, i) & BITMASK(i))

extern xConnSetupPrefix connSetupPrefix;

static ClientPtr grabClient;
#define GrabNone 0
#define GrabActive 1
#define GrabKickout 2
static int grabState = GrabNone;
static long grabWaiters[mskcnt];
_X_EXPORT CallbackListPtr ServerGrabCallback = NULL;
HWEventQueuePtr checkForInput[2];
extern int connBlockScreenStart;

static void KillAllClients(void);

static int nextFreeClientID; /* always MIN free client ID */

static int	nClients;	/* number of authorized clients */

_X_EXPORT CallbackListPtr ClientStateCallback;

/* dispatchException & isItTimeToYield must be declared volatile since they
 * are modified by signal handlers - otherwise optimizer may assume it doesn't
 * need to actually check value in memory when used and may miss changes from
 * signal handlers.
 */
_X_EXPORT volatile char dispatchException = 0;
_X_EXPORT volatile char isItTimeToYield;

/* Various of the DIX function interfaces were not designed to allow
 * the client->errorValue to be set on BadValue and other errors.
 * Rather than changing interfaces and breaking untold code we introduce
 * a new global that dispatch can use.
 */
XID clientErrorValue;   /* XXX this is a kludge */

#define SAME_SCREENS(a, b) (\
    (a.pScreen == b.pScreen))

void
SetInputCheck(HWEventQueuePtr c0, HWEventQueuePtr c1)
{
    checkForInput[0] = c0;
    checkForInput[1] = c1;
}

_X_EXPORT void
UpdateCurrentTime(void)
{
    TimeStamp systime;

    /* To avoid time running backwards, we must call GetTimeInMillis before
     * calling ProcessInputEvents.
     */
    systime.months = currentTime.months;
    systime.milliseconds = GetTimeInMillis();
    if (systime.milliseconds < currentTime.milliseconds)
	systime.months++;
    if (*checkForInput[0] != *checkForInput[1])
	ProcessInputEvents();
    if (CompareTimeStamps(systime, currentTime) == LATER)
	currentTime = systime;
}

/* Like UpdateCurrentTime, but can't call ProcessInputEvents */
_X_EXPORT void
UpdateCurrentTimeIf(void)
{
    TimeStamp systime;

    systime.months = currentTime.months;
    systime.milliseconds = GetTimeInMillis();
    if (systime.milliseconds < currentTime.milliseconds)
	systime.months++;
    if (*checkForInput[0] == *checkForInput[1])
	currentTime = systime;
}


#undef SMART_DEBUG

#define SMART_SCHEDULE_DEFAULT_INTERVAL	20	    /* ms */
#define SMART_SCHEDULE_MAX_SLICE	200	    /* ms */

Bool	    SmartScheduleDisable = FALSE;
long	    SmartScheduleSlice = SMART_SCHEDULE_DEFAULT_INTERVAL;
long	    SmartScheduleInterval = SMART_SCHEDULE_DEFAULT_INTERVAL;
long	    SmartScheduleMaxSlice = SMART_SCHEDULE_MAX_SLICE;
long	    SmartScheduleTime;
static ClientPtr   SmartLastClient;
static int	   SmartLastIndex[SMART_MAX_PRIORITY-SMART_MIN_PRIORITY+1];

#ifdef SMART_DEBUG
long	    SmartLastPrint;
#endif

void        Dispatch(void);
void        InitProcVectors(void);

static int
SmartScheduleClient (int *clientReady, int nready)
{
    ClientPtr	pClient;
    int		i;
    int		client;
    int		bestPrio, best = 0;
    int		bestRobin, robin;
    long	now = SmartScheduleTime;
    long	idle;

    bestPrio = -0x7fffffff;
    bestRobin = 0;
    idle = 2 * SmartScheduleSlice;
    for (i = 0; i < nready; i++)
    {
	client = clientReady[i];
	pClient = clients[client];
	/* Praise clients which are idle */
	if ((now - pClient->smart_check_tick) >= idle)
	{
	    if (pClient->smart_priority < 0)
		pClient->smart_priority++;
	}
	pClient->smart_check_tick = now;
	
	/* check priority to select best client */
	robin = (pClient->index - SmartLastIndex[pClient->smart_priority-SMART_MIN_PRIORITY]) & 0xff;
	if (pClient->smart_priority > bestPrio ||
	    (pClient->smart_priority == bestPrio && robin > bestRobin))
	{
	    bestPrio = pClient->smart_priority;
	    bestRobin = robin;
	    best = client;
	}
#ifdef SMART_DEBUG
	if ((now - SmartLastPrint) >= 5000)
	    fprintf (stderr, " %2d: %3d", client, pClient->smart_priority);
#endif
    }
#ifdef SMART_DEBUG
    if ((now - SmartLastPrint) >= 5000)
    {
	fprintf (stderr, " use %2d\n", best);
	SmartLastPrint = now;
    }
#endif
    pClient = clients[best];
    SmartLastIndex[bestPrio-SMART_MIN_PRIORITY] = pClient->index;
    /*
     * Set current client pointer
     */
    if (SmartLastClient != pClient)
    {
	pClient->smart_start_tick = now;
	SmartLastClient = pClient;
    }
    /*
     * Adjust slice
     */
    if (nready == 1)
    {
	/*
	 * If it's been a long time since another client
	 * has run, bump the slice up to get maximal
	 * performance from a single client
	 */
	if ((now - pClient->smart_start_tick) > 1000 &&
	    SmartScheduleSlice < SmartScheduleMaxSlice)
	{
	    SmartScheduleSlice += SmartScheduleInterval;
	}
    }
    else
    {
	SmartScheduleSlice = SmartScheduleInterval;
    }
    return best;
}

#define MAJOROP ((xReq *)client->requestBuffer)->reqType

void
Dispatch(void)
{
    int        *clientReady;     /* array of request ready clients */
    int	result;
    ClientPtr	client;
    int	nready;
    HWEventQueuePtr* icheck = checkForInput;
    long			start_tick;

    nextFreeClientID = 1;
    nClients = 0;

    clientReady = (int *) xalloc(sizeof(int) * MaxClients);
    if (!clientReady)
	return;

    while (!dispatchException)
    {
        if (*icheck[0] != *icheck[1])
	{
	    ProcessInputEvents();
	    FlushIfCriticalOutputPending();
	}

	nready = WaitForSomething(clientReady);

	if (nready && !SmartScheduleDisable)
	{
	    clientReady[0] = SmartScheduleClient (clientReady, nready);
	    nready = 1;
	}
       /***************** 
	*  Handle events in round robin fashion, doing input between 
	*  each round 
	*****************/

	while (!dispatchException && (--nready >= 0))
	{
	    client = clients[clientReady[nready]];
	    if (! client)
	    {
		/* KillClient can cause this to happen */
		continue;
	    }
	    /* GrabServer activation can cause this to be true */
	    if (grabState == GrabKickout)
	    {
		grabState = GrabActive;
		break;
	    }
	    isItTimeToYield = FALSE;
 
	    start_tick = SmartScheduleTime;
	    while (!isItTimeToYield)
	    {
	        if (*icheck[0] != *icheck[1])
		    ProcessInputEvents();
		
		FlushIfCriticalOutputPending();
		if (!SmartScheduleDisable && 
		    (SmartScheduleTime - start_tick) >= SmartScheduleSlice)
		{
		    /* Penalize clients which consume ticks */
		    if (client->smart_priority > SMART_MIN_PRIORITY)
			client->smart_priority--;
		    break;
		}
		/* now, finally, deal with client requests */

	        result = ReadRequestFromClient(client);
	        if (result <= 0) 
	        {
		    if (result < 0)
			CloseDownClient(client);
		    break;
	        }

		client->sequence++;
#ifdef DEBUG
		if (client->requestLogIndex == MAX_REQUEST_LOG)
		    client->requestLogIndex = 0;
		client->requestLog[client->requestLogIndex] = MAJOROP;
		client->requestLogIndex++;
#endif
#ifdef XSERVER_DTRACE
		XSERVER_REQUEST_START(LookupMajorName(MAJOROP), MAJOROP,
			      ((xReq *)client->requestBuffer)->length,
			      client->index, client->requestBuffer);
#endif
		if (result > (maxBigRequestSize << 2))
		    result = BadLength;
		else {
		    result = XaceHookDispatch(client, MAJOROP);
		    if (result == Success)
			result = (* client->requestVector[MAJOROP])(client);
		    XaceHookAuditEnd(client, result);
		}
#ifdef XSERVER_DTRACE
		XSERVER_REQUEST_DONE(LookupMajorName(MAJOROP), MAJOROP,
			      client->sequence, client->index, result);
#endif

		if (result != Success) 
		{
		    if (client->noClientException != Success)
                        CloseDownClient(client);
                    else
		        SendErrorToClient(client, MAJOROP,
					  MinorOpcodeOfRequest(client),
					  client->errorValue, result);
		    break;
	        }
	    }
	    FlushAllOutput();
	    client = clients[clientReady[nready]];
	    if (client)
		client->smart_stop_tick = SmartScheduleTime;
	}
	dispatchException &= ~DE_PRIORITYCHANGE;
    }
#if defined(DDXBEFORERESET)
    ddxBeforeReset ();
#endif
    KillAllClients();
    xfree(clientReady);
    dispatchException &= ~DE_RESET;
}

#undef MAJOROP

_X_EXPORT int
ProcBadRequest(ClientPtr client)
{
    return (BadRequest);
}

int
ProcCreateWindow(ClientPtr client)
{
    WindowPtr pParent, pWin;
    REQUEST(xCreateWindowReq);
    int len, rc;

    REQUEST_AT_LEAST_SIZE(xCreateWindowReq);
    
    LEGAL_NEW_RESOURCE(stuff->wid, client);
    rc = dixLookupWindow(&pParent, stuff->parent, client, DixAddAccess);
    if (rc != Success)
        return rc;
    len = client->req_len - (sizeof(xCreateWindowReq) >> 2);
    if (Ones(stuff->mask) != len)
        return BadLength;
    if (!stuff->width || !stuff->height)
    {
	client->errorValue = 0;
        return BadValue;
    }
    pWin = CreateWindow(stuff->wid, pParent, stuff->x,
			      stuff->y, stuff->width, stuff->height, 
			      stuff->borderWidth, stuff->class,
			      stuff->mask, (XID *) &stuff[1], 
			      (int)stuff->depth, 
			      client, stuff->visual, &rc);
    if (pWin)
    {
	Mask mask = pWin->eventMask;

	pWin->eventMask = 0; /* subterfuge in case AddResource fails */
	if (!AddResource(stuff->wid, RT_WINDOW, (pointer)pWin))
	    return BadAlloc;
	pWin->eventMask = mask;
    }
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return rc;
}

int
ProcChangeWindowAttributes(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xChangeWindowAttributesReq);
    int result, len, rc;
    Mask access_mode = 0;

    REQUEST_AT_LEAST_SIZE(xChangeWindowAttributesReq);
    access_mode |= (stuff->valueMask & CWEventMask) ? DixReceiveAccess : 0;
    access_mode |= (stuff->valueMask & ~CWEventMask) ? DixSetAttrAccess : 0;
    rc = dixLookupWindow(&pWin, stuff->window, client, access_mode);
    if (rc != Success)
        return rc;
    len = client->req_len - (sizeof(xChangeWindowAttributesReq) >> 2);
    if (len != Ones(stuff->valueMask))
        return BadLength;
    result =  ChangeWindowAttributes(pWin, 
				  stuff->valueMask, 
				  (XID *) &stuff[1], 
				  client);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

int
ProcGetWindowAttributes(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xResourceReq);
    xGetWindowAttributesReply wa;
    int rc;

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupWindow(&pWin, stuff->id, client, DixGetAttrAccess);
    if (rc != Success)
	return rc;
    GetWindowAttributes(pWin, client, &wa);
    WriteReplyToClient(client, sizeof(xGetWindowAttributesReply), &wa);
    return(client->noClientException);
}

int
ProcDestroyWindow(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xResourceReq);
    int rc;

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupWindow(&pWin, stuff->id, client, DixDestroyAccess);
    if (rc != Success)
	return rc;
    if (pWin->parent) {
	rc = dixLookupWindow(&pWin, pWin->parent->drawable.id, client,
			     DixRemoveAccess);
	if (rc != Success)
	    return rc;
	FreeResource(stuff->id, RT_NONE);
    }
    return(client->noClientException);
}

int
ProcDestroySubwindows(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xResourceReq);
    int rc;

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupWindow(&pWin, stuff->id, client, DixRemoveAccess);
    if (rc != Success)
	return rc;
    DestroySubwindows(pWin, client);
    return(client->noClientException);
}

int
ProcChangeSaveSet(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xChangeSaveSetReq);
    int result, rc;
		  
    REQUEST_SIZE_MATCH(xChangeSaveSetReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixManageAccess);
    if (rc != Success)
        return rc;
    if (client->clientAsMask == (CLIENT_BITS(pWin->drawable.id)))
        return BadMatch;
    if ((stuff->mode == SetModeInsert) || (stuff->mode == SetModeDelete))
    {
        result = AlterSaveSetForClient(client, pWin, stuff->mode, FALSE, TRUE);
	if (client->noClientException != Success)
	    return(client->noClientException);
	else
            return(result);
    }
    else
    {
	client->errorValue = stuff->mode;
	return( BadValue );
    }
}

int
ProcReparentWindow(ClientPtr client)
{
    WindowPtr pWin, pParent;
    REQUEST(xReparentWindowReq);
    int result, rc;

    REQUEST_SIZE_MATCH(xReparentWindowReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixManageAccess);
    if (rc != Success)
        return rc;
    rc = dixLookupWindow(&pParent, stuff->parent, client, DixAddAccess);
    if (rc != Success)
        return rc;
    if (SAME_SCREENS(pWin->drawable, pParent->drawable))
    {
        if ((pWin->backgroundState == ParentRelative) &&
            (pParent->drawable.depth != pWin->drawable.depth))
            return BadMatch;
	if ((pWin->drawable.class != InputOnly) &&
	    (pParent->drawable.class == InputOnly))
	    return BadMatch;
        result =  ReparentWindow(pWin, pParent, 
			 (short)stuff->x, (short)stuff->y, client);
	if (client->noClientException != Success)
            return(client->noClientException);
	else
            return(result);
    }
    else 
        return (BadMatch);
}

int
ProcMapWindow(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xResourceReq);
    int rc;

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupWindow(&pWin, stuff->id, client, DixShowAccess);
    if (rc != Success)
        return rc;
    MapWindow(pWin, client);
           /* update cache to say it is mapped */
    return(client->noClientException);
}

int
ProcMapSubwindows(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xResourceReq);
    int rc;

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupWindow(&pWin, stuff->id, client, DixListAccess);
    if (rc != Success)
        return rc;
    MapSubwindows(pWin, client);
           /* update cache to say it is mapped */
    return(client->noClientException);
}

int
ProcUnmapWindow(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xResourceReq);
    int rc;

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupWindow(&pWin, stuff->id, client, DixHideAccess);
    if (rc != Success)
        return rc;
    UnmapWindow(pWin, FALSE);
           /* update cache to say it is mapped */
    return(client->noClientException);
}

int
ProcUnmapSubwindows(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xResourceReq);
    int rc;

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupWindow(&pWin, stuff->id, client, DixListAccess);
    if (rc != Success)
        return rc;
    UnmapSubwindows(pWin);
    return(client->noClientException);
}

int
ProcConfigureWindow(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xConfigureWindowReq);
    int result;
    int len, rc;

    REQUEST_AT_LEAST_SIZE(xConfigureWindowReq);
    rc = dixLookupWindow(&pWin, stuff->window, client,
			 DixManageAccess|DixSetAttrAccess);
    if (rc != Success)
        return rc;
    len = client->req_len - (sizeof(xConfigureWindowReq) >> 2);
    if (Ones((Mask)stuff->mask) != len)
        return BadLength;
    result =  ConfigureWindow(pWin, (Mask)stuff->mask, (XID *) &stuff[1], 
			      client);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

int
ProcCirculateWindow(ClientPtr client)
{
    WindowPtr pWin;
    REQUEST(xCirculateWindowReq);
    int rc;

    REQUEST_SIZE_MATCH(xCirculateWindowReq);
    if ((stuff->direction != RaiseLowest) &&
	(stuff->direction != LowerHighest))
    {
	client->errorValue = stuff->direction;
        return BadValue;
    }
    rc = dixLookupWindow(&pWin, stuff->window, client, DixManageAccess);
    if (rc != Success)
        return rc;
    CirculateWindow(pWin, (int)stuff->direction, client);
    return(client->noClientException);
}

static int
GetGeometry(ClientPtr client, xGetGeometryReply *rep)
{
    DrawablePtr pDraw;
    int rc;
    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    rc = dixLookupDrawable(&pDraw, stuff->id, client, M_ANY, DixGetAttrAccess);
    if (rc != Success)
	return rc;

    rep->type = X_Reply;
    rep->length = 0;
    rep->sequenceNumber = client->sequence;
    rep->root = WindowTable[pDraw->pScreen->myNum]->drawable.id;
    rep->depth = pDraw->depth;
    rep->width = pDraw->width;
    rep->height = pDraw->height;

    /* XXX - Because the pixmap-implementation of the multibuffer extension 
     *       may have the buffer-id's drawable resource value be a pointer
     *       to the buffer's window instead of the buffer itself
     *       (this happens if the buffer is the displayed buffer),
     *       we also have to check that the id matches before we can
     *       truly say that it is a DRAWABLE_WINDOW.
     */

    if ((pDraw->type == UNDRAWABLE_WINDOW) ||
        ((pDraw->type == DRAWABLE_WINDOW) && (stuff->id == pDraw->id)))
    {
        WindowPtr pWin = (WindowPtr)pDraw;
	rep->x = pWin->origin.x - wBorderWidth (pWin);
	rep->y = pWin->origin.y - wBorderWidth (pWin);
	rep->borderWidth = pWin->borderWidth;
    }
    else /* DRAWABLE_PIXMAP or DRAWABLE_BUFFER */
    {
	rep->x = rep->y = rep->borderWidth = 0;
    }

    return Success;
}


int
ProcGetGeometry(ClientPtr client)
{
    xGetGeometryReply rep;
    int status;

    if ((status = GetGeometry(client, &rep)) != Success)
	return status;

    WriteReplyToClient(client, sizeof(xGetGeometryReply), &rep);
    return(client->noClientException);
}


int
ProcQueryTree(ClientPtr client)
{
    xQueryTreeReply reply;
    int rc, numChildren = 0;
    WindowPtr pChild, pWin, pHead;
    Window  *childIDs = (Window *)NULL;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupWindow(&pWin, stuff->id, client, DixListAccess);
    if (rc != Success)
        return rc;
    reply.type = X_Reply;
    reply.root = WindowTable[pWin->drawable.pScreen->myNum]->drawable.id;
    reply.sequenceNumber = client->sequence;
    if (pWin->parent)
	reply.parent = pWin->parent->drawable.id;
    else
        reply.parent = (Window)None;
    pHead = RealChildHead(pWin);
    for (pChild = pWin->lastChild; pChild != pHead; pChild = pChild->prevSib)
	numChildren++;
    if (numChildren)
    {
	int curChild = 0;

	childIDs = (Window *) xalloc(numChildren * sizeof(Window));
	if (!childIDs)
	    return BadAlloc;
	for (pChild = pWin->lastChild; pChild != pHead; pChild = pChild->prevSib)
	    childIDs[curChild++] = pChild->drawable.id;
    }
    
    reply.nChildren = numChildren;
    reply.length = (numChildren * sizeof(Window)) >> 2;
    
    WriteReplyToClient(client, sizeof(xQueryTreeReply), &reply);
    if (numChildren)
    {
    	client->pSwapReplyFunc = (ReplySwapPtr) Swap32Write;
	WriteSwappedDataToClient(client, numChildren * sizeof(Window), childIDs);
	xfree(childIDs);
    }

    return(client->noClientException);
}

int
ProcInternAtom(ClientPtr client)
{
    Atom atom;
    char *tchar;
    REQUEST(xInternAtomReq);

    REQUEST_FIXED_SIZE(xInternAtomReq, stuff->nbytes);
    if ((stuff->onlyIfExists != xTrue) && (stuff->onlyIfExists != xFalse))
    {
	client->errorValue = stuff->onlyIfExists;
        return(BadValue);
    }
    tchar = (char *) &stuff[1];
    atom = MakeAtom(tchar, stuff->nbytes, !stuff->onlyIfExists);
    if (atom != BAD_RESOURCE)
    {
	xInternAtomReply reply;
	reply.type = X_Reply;
	reply.length = 0;
	reply.sequenceNumber = client->sequence;
	reply.atom = atom;
	WriteReplyToClient(client, sizeof(xInternAtomReply), &reply);
	return(client->noClientException);
    }
    else
	return (BadAlloc);
}

int
ProcGetAtomName(ClientPtr client)
{
    char *str;
    xGetAtomNameReply reply;
    int len;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    if ( (str = NameForAtom(stuff->id)) )
    {
	len = strlen(str);
	reply.type = X_Reply;
	reply.length = (len + 3) >> 2;
	reply.sequenceNumber = client->sequence;
	reply.nameLength = len;
	WriteReplyToClient(client, sizeof(xGetAtomNameReply), &reply);
	(void)WriteToClient(client, len, str);
	return(client->noClientException);
    }
    else 
    { 
	client->errorValue = stuff->id;
	return (BadAtom);
    }
}

int
ProcGrabServer(ClientPtr client)
{
    int rc;
    REQUEST_SIZE_MATCH(xReq);
    if (grabState != GrabNone && client != grabClient)
    {
	ResetCurrentRequest(client);
	client->sequence--;
	BITSET(grabWaiters, client->index);
	IgnoreClient(client);
	return(client->noClientException);
    }
    rc = OnlyListenToOneClient(client);
    if (rc != Success)
	return rc;
    grabState = GrabKickout;
    grabClient = client;

    if (ServerGrabCallback)
    {
	ServerGrabInfoRec grabinfo;
	grabinfo.client = client;
	grabinfo.grabstate  = SERVER_GRABBED;
	CallCallbacks(&ServerGrabCallback, (pointer)&grabinfo);
    }

    return(client->noClientException);
}

static void
UngrabServer(ClientPtr client)
{
    int i;

    grabState = GrabNone;
    ListenToAllClients();
    for (i = mskcnt; --i >= 0 && !grabWaiters[i]; )
	;
    if (i >= 0)
    {
	i <<= 5;
	while (!GETBIT(grabWaiters, i))
	    i++;
	BITCLEAR(grabWaiters, i);
	AttendClient(clients[i]);
    }

    if (ServerGrabCallback)
    {
	ServerGrabInfoRec grabinfo;
	grabinfo.client = client;
	grabinfo.grabstate  = SERVER_UNGRABBED;
	CallCallbacks(&ServerGrabCallback, (pointer)&grabinfo);
    }
}

int
ProcUngrabServer(ClientPtr client)
{
    REQUEST_SIZE_MATCH(xReq);
    UngrabServer(client);
    return(client->noClientException);
}

int
ProcTranslateCoords(ClientPtr client)
{
    REQUEST(xTranslateCoordsReq);

    WindowPtr pWin, pDst;
    xTranslateCoordsReply rep;
    int rc;

    REQUEST_SIZE_MATCH(xTranslateCoordsReq);
    rc = dixLookupWindow(&pWin, stuff->srcWid, client, DixGetAttrAccess);
    if (rc != Success)
        return rc;
    rc = dixLookupWindow(&pDst, stuff->dstWid, client, DixGetAttrAccess);
    if (rc != Success)
        return rc;
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    if (!SAME_SCREENS(pWin->drawable, pDst->drawable))
    {
	rep.sameScreen = xFalse;
        rep.child = None;
	rep.dstX = rep.dstY = 0;
    }
    else
    {
	INT16 x, y;
	rep.sameScreen = xTrue;
	rep.child = None;
	/* computing absolute coordinates -- adjust to destination later */
	x = pWin->drawable.x + stuff->srcX;
	y = pWin->drawable.y + stuff->srcY;
	pWin = pDst->firstChild;
	while (pWin)
	{
	    BoxRec  box;
	    if ((pWin->mapped) &&
		(x >= pWin->drawable.x - wBorderWidth (pWin)) &&
		(x < pWin->drawable.x + (int)pWin->drawable.width +
		 wBorderWidth (pWin)) &&
		(y >= pWin->drawable.y - wBorderWidth (pWin)) &&
		(y < pWin->drawable.y + (int)pWin->drawable.height +
		 wBorderWidth (pWin))
		/* When a window is shaped, a further check
		 * is made to see if the point is inside
		 * borderSize
		 */
		&& (!wBoundingShape(pWin) ||
		    POINT_IN_REGION(pWin->drawable.pScreen, 
					&pWin->borderSize, x, y, &box))
		
		&& (!wInputShape(pWin) ||
		    POINT_IN_REGION(pWin->drawable.pScreen,
				    wInputShape(pWin),
				    x - pWin->drawable.x,
				    y - pWin->drawable.y, &box))
		)
            {
		rep.child = pWin->drawable.id;
		pWin = (WindowPtr) NULL;
	    }
	    else
		pWin = pWin->nextSib;
	}
	/* adjust to destination coordinates */
	rep.dstX = x - pDst->drawable.x;
	rep.dstY = y - pDst->drawable.y;
    }
    WriteReplyToClient(client, sizeof(xTranslateCoordsReply), &rep);
    return(client->noClientException);
}

int
ProcOpenFont(ClientPtr client)
{
    int	err;
    REQUEST(xOpenFontReq);

    REQUEST_FIXED_SIZE(xOpenFontReq, stuff->nbytes);
    client->errorValue = stuff->fid;
    LEGAL_NEW_RESOURCE(stuff->fid, client);
    err = OpenFont(client, stuff->fid, (Mask) 0,
		stuff->nbytes, (char *)&stuff[1]);
    if (err == Success)
    {
	return(client->noClientException);
    }
    else
	return err;
}

int
ProcCloseFont(ClientPtr client)
{
    FontPtr pFont;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    pFont = (FontPtr)SecurityLookupIDByType(client, stuff->id, RT_FONT,
					    DixDestroyAccess);
    if ( pFont != (FontPtr)NULL)	/* id was valid */
    {
        FreeResource(stuff->id, RT_NONE);
	return(client->noClientException);
    }
    else
    {
	client->errorValue = stuff->id;
        return (BadFont);
    }
}

int
ProcQueryFont(ClientPtr client)
{
    xQueryFontReply	*reply;
    FontPtr pFont;
    GC *pGC;
    int rc;
    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    client->errorValue = stuff->id;		/* EITHER font or gc */
    rc = dixLookupResource((pointer *)&pFont, stuff->id, RT_FONT, client,
			   DixGetAttrAccess);
    if (rc == BadValue) {
	rc = dixLookupResource((pointer *)&pGC, stuff->id, RT_GC, client,
			       DixGetAttrAccess);
	if (rc == Success)
	    pFont = pGC->font;
    }
    if (rc != Success)
	return (rc == BadValue) ? BadFont: rc;

    {
	xCharInfo	*pmax = FONTINKMAX(pFont);
	xCharInfo	*pmin = FONTINKMIN(pFont);
	int		nprotoxcistructs;
	int		rlength;

	nprotoxcistructs = (
	   pmax->rightSideBearing == pmin->rightSideBearing &&
	   pmax->leftSideBearing == pmin->leftSideBearing &&
	   pmax->descent == pmin->descent &&
	   pmax->ascent == pmin->ascent &&
	   pmax->characterWidth == pmin->characterWidth) ?
		0 : N2dChars(pFont);

	rlength = sizeof(xQueryFontReply) +
	             FONTINFONPROPS(FONTCHARSET(pFont)) * sizeof(xFontProp)  +
		     nprotoxcistructs * sizeof(xCharInfo);
	reply = (xQueryFontReply *)xalloc(rlength);
	if(!reply)
	{
	    return(BadAlloc);
	}

	reply->type = X_Reply;
	reply->length = (rlength - sizeof(xGenericReply)) >> 2;
	reply->sequenceNumber = client->sequence;
	QueryFont( pFont, reply, nprotoxcistructs);

        WriteReplyToClient(client, rlength, reply);
	xfree(reply);
	return(client->noClientException);
    }
}

int
ProcQueryTextExtents(ClientPtr client)
{
    xQueryTextExtentsReply reply;
    FontPtr pFont;
    GC *pGC;
    ExtentInfoRec info;
    unsigned long length;
    int rc;
    REQUEST(xQueryTextExtentsReq);
    REQUEST_AT_LEAST_SIZE(xQueryTextExtentsReq);
        
    client->errorValue = stuff->fid;		/* EITHER font or gc */
    rc = dixLookupResource((pointer *)&pFont, stuff->fid, RT_FONT, client,
			   DixGetAttrAccess);
    if (rc == BadValue) {
	rc = dixLookupResource((pointer *)&pGC, stuff->fid, RT_GC, client,
			       DixGetAttrAccess);
	if (rc == Success)
	    pFont = pGC->font;
    }
    if (rc != Success)
	return (rc == BadValue) ? BadFont: rc;

    length = client->req_len - (sizeof(xQueryTextExtentsReq) >> 2);
    length = length << 1;
    if (stuff->oddLength)
    {
	if (length == 0)
	    return(BadLength);
        length--;
    }
    if (!QueryTextExtents(pFont, length, (unsigned char *)&stuff[1], &info))
	return(BadAlloc);
    reply.type = X_Reply;
    reply.length = 0;
    reply.sequenceNumber = client->sequence;
    reply.drawDirection = info.drawDirection;
    reply.fontAscent = info.fontAscent;
    reply.fontDescent = info.fontDescent;
    reply.overallAscent = info.overallAscent;
    reply.overallDescent = info.overallDescent;
    reply.overallWidth = info.overallWidth;
    reply.overallLeft = info.overallLeft;
    reply.overallRight = info.overallRight;
    WriteReplyToClient(client, sizeof(xQueryTextExtentsReply), &reply);
    return(client->noClientException);
}

int
ProcListFonts(ClientPtr client)
{
    REQUEST(xListFontsReq);

    REQUEST_FIXED_SIZE(xListFontsReq, stuff->nbytes);

    return ListFonts(client, (unsigned char *) &stuff[1], stuff->nbytes, 
	stuff->maxNames);
}

int
ProcListFontsWithInfo(ClientPtr client)
{
    REQUEST(xListFontsWithInfoReq);

    REQUEST_FIXED_SIZE(xListFontsWithInfoReq, stuff->nbytes);

    return StartListFontsWithInfo(client, stuff->nbytes,
				  (unsigned char *) &stuff[1], stuff->maxNames);
}

/**
 *
 *  \param value must conform to DeleteType
 */
int
dixDestroyPixmap(pointer value, XID pid)
{
    PixmapPtr pPixmap = (PixmapPtr)value;
    return (*pPixmap->drawable.pScreen->DestroyPixmap)(pPixmap);
}

int
ProcCreatePixmap(ClientPtr client)
{
    PixmapPtr pMap;
    DrawablePtr pDraw;
    REQUEST(xCreatePixmapReq);
    DepthPtr pDepth;
    int i, rc;

    REQUEST_SIZE_MATCH(xCreatePixmapReq);
    client->errorValue = stuff->pid;
    LEGAL_NEW_RESOURCE(stuff->pid, client);
    
    rc = dixLookupDrawable(&pDraw, stuff->drawable, client, M_ANY,
			   DixGetAttrAccess);
    if (rc != Success)
	return rc;

    if (!stuff->width || !stuff->height)
    {
	client->errorValue = 0;
        return BadValue;
    }
    if (stuff->width > 32767 || stuff->height > 32767)
    {
	/* It is allowed to try and allocate a pixmap which is larger than
	 * 32767 in either dimension. However, all of the framebuffer code
	 * is buggy and does not reliably draw to such big pixmaps, basically
	 * because the Region data structure operates with signed shorts
	 * for the rectangles in it.
	 *
	 * Furthermore, several places in the X server computes the
	 * size in bytes of the pixmap and tries to store it in an
	 * integer. This integer can overflow and cause the allocated size
	 * to be much smaller.
	 *
	 * So, such big pixmaps are rejected here with a BadAlloc
	 */
	return BadAlloc;
    }
    if (stuff->depth != 1)
    {
        pDepth = pDraw->pScreen->allowedDepths;
        for (i=0; i<pDraw->pScreen->numDepths; i++, pDepth++)
	   if (pDepth->depth == stuff->depth)
               goto CreatePmap;
	client->errorValue = stuff->depth;
        return BadValue;
    }
CreatePmap:
    pMap = (PixmapPtr)(*pDraw->pScreen->CreatePixmap)
		(pDraw->pScreen, stuff->width,
		 stuff->height, stuff->depth, 0);
    if (pMap)
    {
	pMap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	pMap->drawable.id = stuff->pid;
	/* security creation/labeling check */
	rc = XaceHook(XACE_RESOURCE_ACCESS, client, stuff->pid, RT_PIXMAP,
		      pMap, RT_NONE, NULL, DixCreateAccess);
	if (rc != Success) {
	    (*pDraw->pScreen->DestroyPixmap)(pMap);
	    return rc;
	}
	if (AddResource(stuff->pid, RT_PIXMAP, (pointer)pMap))
	    return(client->noClientException);
	(*pDraw->pScreen->DestroyPixmap)(pMap);
    }
    return (BadAlloc);
}

int
ProcFreePixmap(ClientPtr client)
{
    PixmapPtr pMap;
    int rc;
    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    rc = dixLookupResource((pointer *)&pMap, stuff->id, RT_PIXMAP, client,
			   DixDestroyAccess);
    if (rc == Success)
    {
	FreeResource(stuff->id, RT_NONE);
	return(client->noClientException);
    }
    else 
    {
	client->errorValue = stuff->id;
	return (rc == BadValue) ? BadPixmap : rc;
    }
}

int
ProcCreateGC(ClientPtr client)
{
    int error, rc;
    GC *pGC;
    DrawablePtr pDraw;
    unsigned len;
    REQUEST(xCreateGCReq);

    REQUEST_AT_LEAST_SIZE(xCreateGCReq);
    client->errorValue = stuff->gc;
    LEGAL_NEW_RESOURCE(stuff->gc, client);
    rc = dixLookupDrawable(&pDraw, stuff->drawable, client, 0,
			   DixGetAttrAccess);
    if (rc != Success)
	return rc;

    len = client->req_len -  (sizeof(xCreateGCReq) >> 2);
    if (len != Ones(stuff->mask))
        return BadLength;
    pGC = (GC *)CreateGC(pDraw, stuff->mask, (XID *) &stuff[1], &error,
			 stuff->gc, client);
    if (error != Success)
        return error;
    if (!AddResource(stuff->gc, RT_GC, (pointer)pGC))
	return (BadAlloc);
    return(client->noClientException);
}

int
ProcChangeGC(ClientPtr client)
{
    GC *pGC;
    int result;
    unsigned len;
    REQUEST(xChangeGCReq);
    REQUEST_AT_LEAST_SIZE(xChangeGCReq);

    result = dixLookupGC(&pGC, stuff->gc, client, DixSetAttrAccess);
    if (result != Success)
	return result;

    len = client->req_len -  (sizeof(xChangeGCReq) >> 2);
    if (len != Ones(stuff->mask))
        return BadLength;

    result = dixChangeGC(client, pGC, stuff->mask, (CARD32 *) &stuff[1], 0);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
    {
	client->errorValue = clientErrorValue;
        return(result);
    }
}

int
ProcCopyGC(ClientPtr client)
{
    GC *dstGC;
    GC *pGC;
    int result;
    REQUEST(xCopyGCReq);
    REQUEST_SIZE_MATCH(xCopyGCReq);

    result = dixLookupGC(&pGC, stuff->srcGC, client, DixGetAttrAccess);
    if (result != Success)
	return result;
    result = dixLookupGC(&dstGC, stuff->dstGC, client, DixSetAttrAccess);
    if (result != Success)
	return result;
    if ((dstGC->pScreen != pGC->pScreen) || (dstGC->depth != pGC->depth))
        return (BadMatch);    
    result = CopyGC(pGC, dstGC, stuff->mask);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
    {
	client->errorValue = clientErrorValue;
        return(result);
    }
}

int
ProcSetDashes(ClientPtr client)
{
    GC *pGC;
    int result;
    REQUEST(xSetDashesReq);

    REQUEST_FIXED_SIZE(xSetDashesReq, stuff->nDashes);
    if (stuff->nDashes == 0)
    {
	 client->errorValue = 0;
         return BadValue;
    }

    result = dixLookupGC(&pGC,stuff->gc, client, DixSetAttrAccess);
    if (result != Success)
	return result;

    result = SetDashes(pGC, stuff->dashOffset, stuff->nDashes,
		       (unsigned char *)&stuff[1]);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
    {
	client->errorValue = clientErrorValue;
        return(result);
    }
}

int
ProcSetClipRectangles(ClientPtr client)
{
    int	nr, result;
    GC *pGC;
    REQUEST(xSetClipRectanglesReq);

    REQUEST_AT_LEAST_SIZE(xSetClipRectanglesReq);
    if ((stuff->ordering != Unsorted) && (stuff->ordering != YSorted) &&
	(stuff->ordering != YXSorted) && (stuff->ordering != YXBanded))
    {
	client->errorValue = stuff->ordering;
        return BadValue;
    }
    result = dixLookupGC(&pGC,stuff->gc, client, DixSetAttrAccess);
    if (result != Success)
	return result;
		 
    nr = (client->req_len << 2) - sizeof(xSetClipRectanglesReq);
    if (nr & 4)
	return(BadLength);
    nr >>= 3;
    result = SetClipRects(pGC, stuff->xOrigin, stuff->yOrigin,
			  nr, (xRectangle *)&stuff[1], (int)stuff->ordering);
    if (client->noClientException != Success)
        return(client->noClientException);
    else
        return(result);
}

int
ProcFreeGC(ClientPtr client)
{
    GC *pGC;
    int rc;
    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    rc = dixLookupGC(&pGC, stuff->id, client, DixDestroyAccess);
    if (rc != Success)
	return rc;

    FreeResource(stuff->id, RT_NONE);
    return(client->noClientException);
}

int
ProcClearToBackground(ClientPtr client)
{
    REQUEST(xClearAreaReq);
    WindowPtr pWin;
    int rc;

    REQUEST_SIZE_MATCH(xClearAreaReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixWriteAccess);
    if (rc != Success)
        return rc;
    if (pWin->drawable.class == InputOnly)
    {
	client->errorValue = stuff->window;
	return (BadMatch);
    }		    
    if ((stuff->exposures != xTrue) && (stuff->exposures != xFalse))
    {
	client->errorValue = stuff->exposures;
        return(BadValue);
    }
    (*pWin->drawable.pScreen->ClearToBackground)(pWin, stuff->x, stuff->y,
			       stuff->width, stuff->height,
			       (Bool)stuff->exposures);
    return(client->noClientException);
}

int
ProcCopyArea(ClientPtr client)
{
    DrawablePtr pDst;
    DrawablePtr pSrc;
    GC *pGC;
    REQUEST(xCopyAreaReq);
    RegionPtr pRgn;
    int rc;

    REQUEST_SIZE_MATCH(xCopyAreaReq);

    VALIDATE_DRAWABLE_AND_GC(stuff->dstDrawable, pDst, DixWriteAccess); 
    if (stuff->dstDrawable != stuff->srcDrawable)
    {
	rc = dixLookupDrawable(&pSrc, stuff->srcDrawable, client, 0,
				 DixReadAccess);
	if (rc != Success)
	    return rc;
	if ((pDst->pScreen != pSrc->pScreen) || (pDst->depth != pSrc->depth))
	{
	    client->errorValue = stuff->dstDrawable;
	    return (BadMatch);
	}
    }
    else
        pSrc = pDst;

    pRgn = (*pGC->ops->CopyArea)(pSrc, pDst, pGC, stuff->srcX, stuff->srcY,
				 stuff->width, stuff->height, 
				 stuff->dstX, stuff->dstY);
    if (pGC->graphicsExposures)
    {
	(*pDst->pScreen->SendGraphicsExpose)
 		(client, pRgn, stuff->dstDrawable, X_CopyArea, 0);
	if (pRgn)
	    REGION_DESTROY(pDst->pScreen, pRgn);
    }

    return(client->noClientException);
}

int
ProcCopyPlane(ClientPtr client)
{
    DrawablePtr psrcDraw, pdstDraw;
    GC *pGC;
    REQUEST(xCopyPlaneReq);
    RegionPtr pRgn;
    int rc;

    REQUEST_SIZE_MATCH(xCopyPlaneReq);

    VALIDATE_DRAWABLE_AND_GC(stuff->dstDrawable, pdstDraw, DixWriteAccess);
    if (stuff->dstDrawable != stuff->srcDrawable)
    {
	rc = dixLookupDrawable(&psrcDraw, stuff->srcDrawable, client, 0,
			       DixReadAccess);
	if (rc != Success)
	    return rc;

	if (pdstDraw->pScreen != psrcDraw->pScreen)
	{
	    client->errorValue = stuff->dstDrawable;
	    return (BadMatch);
	}
    }
    else
        psrcDraw = pdstDraw;

    /* Check to see if stuff->bitPlane has exactly ONE good bit set */
    if(stuff->bitPlane == 0 || (stuff->bitPlane & (stuff->bitPlane - 1)) ||
       (stuff->bitPlane > (1L << (psrcDraw->depth - 1))))
    {
       client->errorValue = stuff->bitPlane;
       return(BadValue);
    }

    pRgn = (*pGC->ops->CopyPlane)(psrcDraw, pdstDraw, pGC, stuff->srcX, stuff->srcY,
				 stuff->width, stuff->height, 
				 stuff->dstX, stuff->dstY, stuff->bitPlane);
    if (pGC->graphicsExposures)
    {
	(*pdstDraw->pScreen->SendGraphicsExpose)
 		(client, pRgn, stuff->dstDrawable, X_CopyPlane, 0);
	if (pRgn)
	    REGION_DESTROY(pdstDraw->pScreen, pRgn);
    }
    return(client->noClientException);
}

int
ProcPolyPoint(ClientPtr client)
{
    int npoint;
    GC *pGC;
    DrawablePtr pDraw;
    REQUEST(xPolyPointReq);

    REQUEST_AT_LEAST_SIZE(xPolyPointReq);
    if ((stuff->coordMode != CoordModeOrigin) && 
	(stuff->coordMode != CoordModePrevious))
    {
	client->errorValue = stuff->coordMode;
        return BadValue;
    }
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess); 
    npoint = ((client->req_len << 2) - sizeof(xPolyPointReq)) >> 2;
    if (npoint)
        (*pGC->ops->PolyPoint)(pDraw, pGC, stuff->coordMode, npoint,
			  (xPoint *) &stuff[1]);
    return (client->noClientException);
}

int
ProcPolyLine(ClientPtr client)
{
    int npoint;
    GC *pGC;
    DrawablePtr pDraw;
    REQUEST(xPolyLineReq);

    REQUEST_AT_LEAST_SIZE(xPolyLineReq);
    if ((stuff->coordMode != CoordModeOrigin) && 
	(stuff->coordMode != CoordModePrevious))
    {
	client->errorValue = stuff->coordMode;
        return BadValue;
    }
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);
    npoint = ((client->req_len << 2) - sizeof(xPolyLineReq)) >> 2;
    if (npoint > 1)
	(*pGC->ops->Polylines)(pDraw, pGC, stuff->coordMode, npoint, 
			      (DDXPointPtr) &stuff[1]);
    return(client->noClientException);
}

int
ProcPolySegment(ClientPtr client)
{
    int nsegs;
    GC *pGC;
    DrawablePtr pDraw;
    REQUEST(xPolySegmentReq);

    REQUEST_AT_LEAST_SIZE(xPolySegmentReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);
    nsegs = (client->req_len << 2) - sizeof(xPolySegmentReq);
    if (nsegs & 4)
	return(BadLength);
    nsegs >>= 3;
    if (nsegs)
        (*pGC->ops->PolySegment)(pDraw, pGC, nsegs, (xSegment *) &stuff[1]);
    return (client->noClientException);
}

int
ProcPolyRectangle (ClientPtr client)
{
    int nrects;
    GC *pGC;
    DrawablePtr pDraw;
    REQUEST(xPolyRectangleReq);

    REQUEST_AT_LEAST_SIZE(xPolyRectangleReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);
    nrects = (client->req_len << 2) - sizeof(xPolyRectangleReq);
    if (nrects & 4)
	return(BadLength);
    nrects >>= 3;
    if (nrects)
        (*pGC->ops->PolyRectangle)(pDraw, pGC, 
		    nrects, (xRectangle *) &stuff[1]);
    return(client->noClientException);
}

int
ProcPolyArc(ClientPtr client)
{
    int		narcs;
    GC *pGC;
    DrawablePtr pDraw;
    REQUEST(xPolyArcReq);

    REQUEST_AT_LEAST_SIZE(xPolyArcReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);
    narcs = (client->req_len << 2) - sizeof(xPolyArcReq);
    if (narcs % sizeof(xArc))
	return(BadLength);
    narcs /= sizeof(xArc);
    if (narcs)
        (*pGC->ops->PolyArc)(pDraw, pGC, narcs, (xArc *) &stuff[1]);
    return (client->noClientException);
}

int
ProcFillPoly(ClientPtr client)
{
    int          things;
    GC *pGC;
    DrawablePtr pDraw;
    REQUEST(xFillPolyReq);

    REQUEST_AT_LEAST_SIZE(xFillPolyReq);
    if ((stuff->shape != Complex) && (stuff->shape != Nonconvex) &&  
	(stuff->shape != Convex))
    {
	client->errorValue = stuff->shape;
        return BadValue;
    }
    if ((stuff->coordMode != CoordModeOrigin) && 
	(stuff->coordMode != CoordModePrevious))
    {
	client->errorValue = stuff->coordMode;
        return BadValue;
    }

    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);
    things = ((client->req_len << 2) - sizeof(xFillPolyReq)) >> 2;
    if (things)
        (*pGC->ops->FillPolygon) (pDraw, pGC, stuff->shape,
			 stuff->coordMode, things,
			 (DDXPointPtr) &stuff[1]);
    return(client->noClientException);
}

int
ProcPolyFillRectangle(ClientPtr client)
{
    int             things;
    GC *pGC;
    DrawablePtr pDraw;
    REQUEST(xPolyFillRectangleReq);

    REQUEST_AT_LEAST_SIZE(xPolyFillRectangleReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);
    things = (client->req_len << 2) - sizeof(xPolyFillRectangleReq);
    if (things & 4)
	return(BadLength);
    things >>= 3;

    if (things)
        (*pGC->ops->PolyFillRect) (pDraw, pGC, things,
		      (xRectangle *) &stuff[1]);
    return (client->noClientException);
}

int
ProcPolyFillArc(ClientPtr client)
{
    int		narcs;
    GC *pGC;
    DrawablePtr pDraw;
    REQUEST(xPolyFillArcReq);

    REQUEST_AT_LEAST_SIZE(xPolyFillArcReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);
    narcs = (client->req_len << 2) - sizeof(xPolyFillArcReq);
    if (narcs % sizeof(xArc))
	return(BadLength);
    narcs /= sizeof(xArc);
    if (narcs)
        (*pGC->ops->PolyFillArc) (pDraw, pGC, narcs, (xArc *) &stuff[1]);
    return (client->noClientException);
}

#ifdef MATCH_CLIENT_ENDIAN

int
ServerOrder (void)
{
    int	    whichbyte = 1;

    if (*((char *) &whichbyte))
	return LSBFirst;
    return MSBFirst;
}

#define ClientOrder(client) ((client)->swapped ? !ServerOrder() : ServerOrder())

void
ReformatImage (char *base, int nbytes, int bpp, int order)
{
    switch (bpp) {
    case 1:	/* yuck */
	if (BITMAP_BIT_ORDER != order)
	    BitOrderInvert ((unsigned char *) base, nbytes);
#if IMAGE_BYTE_ORDER != BITMAP_BIT_ORDER && BITMAP_SCANLINE_UNIT != 8
	ReformatImage (base, nbytes, BITMAP_SCANLINE_UNIT, order);
#endif
	break;
    case 4:
	break;  /* yuck */
    case 8:
	break;
    case 16:
	if (IMAGE_BYTE_ORDER != order)
	    TwoByteSwap ((unsigned char *) base, nbytes);
	break;
    case 32:
	if (IMAGE_BYTE_ORDER != order)
	    FourByteSwap ((unsigned char *) base, nbytes);
	break;
    }
}
#else
#define ReformatImage(b,n,bpp,o)
#endif

/* 64-bit server notes: the protocol restricts padding of images to
 * 8-, 16-, or 32-bits. We would like to have 64-bits for the server
 * to use internally. Removes need for internal alignment checking.
 * All of the PutImage functions could be changed individually, but
 * as currently written, they call other routines which require things
 * to be 64-bit padded on scanlines, so we changed things here.
 * If an image would be padded differently for 64- versus 32-, then
 * copy each scanline to a 64-bit padded scanline.
 * Also, we need to make sure that the image is aligned on a 64-bit
 * boundary, even if the scanlines are padded to our satisfaction.
 */
int
ProcPutImage(ClientPtr client)
{
    GC *pGC;
    DrawablePtr pDraw;
    long	length; 	/* length of scanline server padded */
    long 	lengthProto; 	/* length of scanline protocol padded */
    char	*tmpImage;
    REQUEST(xPutImageReq);

    REQUEST_AT_LEAST_SIZE(xPutImageReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);
    if (stuff->format == XYBitmap)
    {
        if ((stuff->depth != 1) ||
	    (stuff->leftPad >= (unsigned int)screenInfo.bitmapScanlinePad))
            return BadMatch;
        length 	    = BitmapBytePad(stuff->width + stuff->leftPad);
    }
    else if (stuff->format == XYPixmap)
    {
        if ((pDraw->depth != stuff->depth) || 
	    (stuff->leftPad >= (unsigned int)screenInfo.bitmapScanlinePad))
            return BadMatch;
        length      = BitmapBytePad(stuff->width + stuff->leftPad);
	length      *= stuff->depth;
    }
    else if (stuff->format == ZPixmap)
    {
        if ((pDraw->depth != stuff->depth) || (stuff->leftPad != 0))
            return BadMatch;
        length      = PixmapBytePad(stuff->width, stuff->depth);
    }
    else
    {
	client->errorValue = stuff->format;
        return BadValue;
    }

    tmpImage = (char *)&stuff[1];
    lengthProto = length;
	
    if (((((lengthProto * stuff->height) + (unsigned)3) >> 2) + 
	(sizeof(xPutImageReq) >> 2)) != client->req_len)
	return BadLength;

    ReformatImage (tmpImage, lengthProto * stuff->height, 
		   stuff->format == ZPixmap ? BitsPerPixel (stuff->depth) : 1,
		   ClientOrder(client));
    
    (*pGC->ops->PutImage) (pDraw, pGC, stuff->depth, stuff->dstX, stuff->dstY,
		  stuff->width, stuff->height, 
		  stuff->leftPad, stuff->format, tmpImage);

     return (client->noClientException);
}

static int
DoGetImage(ClientPtr client, int format, Drawable drawable, 
           int x, int y, int width, int height, 
           Mask planemask, xGetImageReply **im_return)
{
    DrawablePtr		pDraw;
    int			nlines, linesPerBuf, rc;
    int	linesDone;
    long		widthBytesLine, length;
    Mask		plane = 0;
    char		*pBuf;
    xGetImageReply	xgi;
    RegionPtr pVisibleRegion = NULL;

    if ((format != XYPixmap) && (format != ZPixmap))
    {
	client->errorValue = format;
        return(BadValue);
    }
    rc = dixLookupDrawable(&pDraw, drawable, client, 0, DixReadAccess);
    if (rc != Success)
	return rc;

    if(pDraw->type == DRAWABLE_WINDOW)
    {
      if( /* check for being viewable */
	 !((WindowPtr) pDraw)->realized ||
	  /* check for being on screen */
         pDraw->x + x < 0 ||
 	 pDraw->x + x + width > pDraw->pScreen->width ||
         pDraw->y + y < 0 ||
         pDraw->y + y + height > pDraw->pScreen->height ||
          /* check for being inside of border */
         x < - wBorderWidth((WindowPtr)pDraw) ||
         x + width > wBorderWidth((WindowPtr)pDraw) + (int)pDraw->width ||
         y < -wBorderWidth((WindowPtr)pDraw) ||
         y + height > wBorderWidth ((WindowPtr)pDraw) + (int)pDraw->height
        )
	    return(BadMatch);
	xgi.visual = wVisual (((WindowPtr) pDraw));
    }
    else
    {
      if(x < 0 ||
         x+width > (int)pDraw->width ||
         y < 0 ||
         y+height > (int)pDraw->height
        )
	    return(BadMatch);
	xgi.visual = None;
    }

    xgi.type = X_Reply;
    xgi.sequenceNumber = client->sequence;
    xgi.depth = pDraw->depth;
    if(format == ZPixmap)
    {
	widthBytesLine = PixmapBytePad(width, pDraw->depth);
	length = widthBytesLine * height;

    }
    else 
    {
	widthBytesLine = BitmapBytePad(width);
	plane = ((Mask)1) << (pDraw->depth - 1);
	/* only planes asked for */
	length = widthBytesLine * height *
		 Ones(planemask & (plane | (plane - 1)));

    }

    xgi.length = length;

    if (im_return) {
	pBuf = (char *)xalloc(sz_xGetImageReply + length);
	if (!pBuf)
	    return (BadAlloc);
	if (widthBytesLine == 0)
	    linesPerBuf = 0;
	else
	    linesPerBuf = height;
	*im_return = (xGetImageReply *)pBuf;
	*(xGetImageReply *)pBuf = xgi;
	pBuf += sz_xGetImageReply;
    } else {
	xgi.length = (xgi.length + 3) >> 2;
	if (widthBytesLine == 0 || height == 0)
	    linesPerBuf = 0;
	else if (widthBytesLine >= IMAGE_BUFSIZE)
	    linesPerBuf = 1;
	else
	{
	    linesPerBuf = IMAGE_BUFSIZE / widthBytesLine;
	    if (linesPerBuf > height)
		linesPerBuf = height;
	}
	length = linesPerBuf * widthBytesLine;
	if (linesPerBuf < height)
	{
	    /* we have to make sure intermediate buffers don't need padding */
	    while ((linesPerBuf > 1) &&
		   (length & ((1L << LOG2_BYTES_PER_SCANLINE_PAD)-1)))
	    {
		linesPerBuf--;
		length -= widthBytesLine;
	    }
	    while (length & ((1L << LOG2_BYTES_PER_SCANLINE_PAD)-1))
	    {
		linesPerBuf++;
		length += widthBytesLine;
	    }
	}
	if(!(pBuf = (char *) xalloc(length)))
	    return (BadAlloc);
	WriteReplyToClient(client, sizeof (xGetImageReply), &xgi);
    }

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	pVisibleRegion = NotClippedByChildren((WindowPtr)pDraw);
	if (pVisibleRegion)
	{
	    REGION_TRANSLATE(pDraw->pScreen, pVisibleRegion,
			     -pDraw->x, -pDraw->y);
	}
    }

    if (linesPerBuf == 0)
    {
	/* nothing to do */
    }
    else if (format == ZPixmap)
    {
        linesDone = 0;
        while (height - linesDone > 0)
        {
	    nlines = min(linesPerBuf, height - linesDone);
	    (*pDraw->pScreen->GetImage) (pDraw,
	                                 x,
				         y + linesDone,
				         width, 
				         nlines,
				         format,
				         planemask,
				         (pointer) pBuf);
	    if (pVisibleRegion)
		XaceCensorImage(client, pVisibleRegion, widthBytesLine,
			pDraw, x, y + linesDone, width, 
			nlines, format, pBuf);

	    /* Note that this is NOT a call to WriteSwappedDataToClient,
               as we do NOT byte swap */
	    if (!im_return)
	    {
		ReformatImage (pBuf, (int)(nlines * widthBytesLine),
			       BitsPerPixel (pDraw->depth),
			       ClientOrder(client));

/* Don't split me, gcc pukes when you do */
		(void)WriteToClient(client,
				    (int)(nlines * widthBytesLine),
				    pBuf);
	    }
	    linesDone += nlines;
        }
    }
    else /* XYPixmap */
    {
        for (; plane; plane >>= 1)
	{
	    if (planemask & plane)
	    {
	        linesDone = 0;
	        while (height - linesDone > 0)
	        {
		    nlines = min(linesPerBuf, height - linesDone);
	            (*pDraw->pScreen->GetImage) (pDraw,
	                                         x,
				                 y + linesDone,
				                 width, 
				                 nlines,
				                 format,
				                 plane,
				                 (pointer)pBuf);
		    if (pVisibleRegion)
			XaceCensorImage(client, pVisibleRegion,
				widthBytesLine,
				pDraw, x, y + linesDone, width, 
				nlines, format, pBuf);

		    /* Note: NOT a call to WriteSwappedDataToClient,
		       as we do NOT byte swap */
		    if (im_return) {
			pBuf += nlines * widthBytesLine;
		    } else {
			ReformatImage (pBuf, 
				       (int)(nlines * widthBytesLine), 
				       1,
				       ClientOrder (client));

/* Don't split me, gcc pukes when you do */
			(void)WriteToClient(client,
					(int)(nlines * widthBytesLine),
					pBuf);
		    }
		    linesDone += nlines;
		}
            }
	}
    }
    if (pVisibleRegion)
	REGION_DESTROY(pDraw->pScreen, pVisibleRegion);
    if (!im_return)
	xfree(pBuf);
    return (client->noClientException);
}

int
ProcGetImage(ClientPtr client)
{
    REQUEST(xGetImageReq);

    REQUEST_SIZE_MATCH(xGetImageReq);

    return DoGetImage(client, stuff->format, stuff->drawable,
		      stuff->x, stuff->y,
		      (int)stuff->width, (int)stuff->height,
		      stuff->planeMask, (xGetImageReply **)NULL);
}

int
ProcPolyText(ClientPtr client)
{
    int	err;
    REQUEST(xPolyTextReq);
    DrawablePtr pDraw;
    GC *pGC;

    REQUEST_AT_LEAST_SIZE(xPolyTextReq);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);

    err = PolyText(client,
		   pDraw,
		   pGC,
		   (unsigned char *)&stuff[1],
		   ((unsigned char *) stuff) + (client->req_len << 2),
		   stuff->x,
		   stuff->y,
		   stuff->reqType,
		   stuff->drawable);

    if (err == Success)
    {
	return(client->noClientException);
    }
    else
	return err;
}

int
ProcImageText8(ClientPtr client)
{
    int	err;
    DrawablePtr pDraw;
    GC *pGC;

    REQUEST(xImageTextReq);

    REQUEST_FIXED_SIZE(xImageTextReq, stuff->nChars);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);

    err = ImageText(client,
		    pDraw,
		    pGC,
		    stuff->nChars,
		    (unsigned char *)&stuff[1],
		    stuff->x,
		    stuff->y,
		    stuff->reqType,
		    stuff->drawable);

    if (err == Success)
    {
	return(client->noClientException);
    }
    else
	return err;
}

int
ProcImageText16(ClientPtr client)
{
    int	err;
    DrawablePtr pDraw;
    GC *pGC;

    REQUEST(xImageTextReq);

    REQUEST_FIXED_SIZE(xImageTextReq, stuff->nChars << 1);
    VALIDATE_DRAWABLE_AND_GC(stuff->drawable, pDraw, DixWriteAccess);

    err = ImageText(client,
		    pDraw,
		    pGC,
		    stuff->nChars,
		    (unsigned char *)&stuff[1],
		    stuff->x,
		    stuff->y,
		    stuff->reqType,
		    stuff->drawable);

    if (err == Success)
    {
	return(client->noClientException);
    }
    else
	return err;
}


int
ProcCreateColormap(ClientPtr client)
{
    VisualPtr	pVisual;
    ColormapPtr	pmap;
    Colormap	mid;
    WindowPtr   pWin;
    ScreenPtr pScreen;
    REQUEST(xCreateColormapReq);
    int i, result;

    REQUEST_SIZE_MATCH(xCreateColormapReq);

    if ((stuff->alloc != AllocNone) && (stuff->alloc != AllocAll))
    {
	client->errorValue = stuff->alloc;
        return(BadValue);
    }
    mid = stuff->mid;
    LEGAL_NEW_RESOURCE(mid, client);
    result = dixLookupWindow(&pWin, stuff->window, client, DixGetAttrAccess);
    if (result != Success)
        return result;

    pScreen = pWin->drawable.pScreen;
    for (i = 0, pVisual = pScreen->visuals;
	 i < pScreen->numVisuals;
	 i++, pVisual++)
    {
	if (pVisual->vid != stuff->visual)
	    continue;
	result =  CreateColormap(mid, pScreen, pVisual, &pmap,
				 (int)stuff->alloc, client->index);
	if (client->noClientException != Success)
	    return(client->noClientException);
	else
	    return(result);
    }
    client->errorValue = stuff->visual;
    return(BadMatch);
}

int
ProcFreeColormap(ClientPtr client)
{
    ColormapPtr pmap;
    int rc;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupResource((pointer *)&pmap, stuff->id, RT_COLORMAP, client,
			   DixDestroyAccess);
    if (rc == Success)
    {
	/* Freeing a default colormap is a no-op */
	if (!(pmap->flags & IsDefault))
	    FreeResource(stuff->id, RT_NONE);
	return (client->noClientException);
    }
    else 
    {
	client->errorValue = stuff->id;
	return (rc == BadValue) ? BadColor : rc;
    }
}


int
ProcCopyColormapAndFree(ClientPtr client)
{
    Colormap	mid;
    ColormapPtr	pSrcMap;
    REQUEST(xCopyColormapAndFreeReq);
    int rc;

    REQUEST_SIZE_MATCH(xCopyColormapAndFreeReq);
    mid = stuff->mid;
    LEGAL_NEW_RESOURCE(mid, client);
    rc = dixLookupResource((pointer *)&pSrcMap, stuff->srcCmap, RT_COLORMAP,
			   client, DixReadAccess|DixRemoveAccess);
    if (rc == Success)
    {
	rc = CopyColormapAndFree(mid, pSrcMap, client->index);
	if (client->noClientException != Success)
            return(client->noClientException);
	else
            return rc;
    }
    else
    {
	client->errorValue = stuff->srcCmap;
	return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcInstallColormap(ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    rc = dixLookupResource((pointer *)&pcmp, stuff->id, RT_COLORMAP, client,
			   DixInstallAccess);
    if (rc != Success)
	goto out;

    rc = XaceHook(XACE_SCREEN_ACCESS, client, pcmp->pScreen, DixSetAttrAccess);
    if (rc != Success)
	goto out;

    (*(pcmp->pScreen->InstallColormap)) (pcmp);

    rc = client->noClientException;
out:
    client->errorValue = stuff->id;
    return (rc == BadValue) ? BadColor : rc;
}

int
ProcUninstallColormap(ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    rc = dixLookupResource((pointer *)&pcmp, stuff->id, RT_COLORMAP, client,
			   DixUninstallAccess);
    if (rc != Success)
	goto out;

    rc = XaceHook(XACE_SCREEN_ACCESS, client, pcmp->pScreen, DixSetAttrAccess);
    if (rc != Success)
	goto out;

    if(pcmp->mid != pcmp->pScreen->defColormap)
	(*(pcmp->pScreen->UninstallColormap)) (pcmp);

    rc = client->noClientException;
out:
    client->errorValue = stuff->id;
    return (rc == BadValue) ? BadColor : rc;
}

int
ProcListInstalledColormaps(ClientPtr client)
{
    xListInstalledColormapsReply *preply; 
    int nummaps, rc;
    WindowPtr pWin;
    REQUEST(xResourceReq);
    REQUEST_SIZE_MATCH(xResourceReq);

    rc = dixLookupWindow(&pWin, stuff->id, client, DixGetAttrAccess);
    if (rc != Success)
	goto out;

    rc = XaceHook(XACE_SCREEN_ACCESS, client, pWin->drawable.pScreen,
		  DixGetAttrAccess);
    if (rc != Success)
	goto out;

    preply = (xListInstalledColormapsReply *) 
		xalloc(sizeof(xListInstalledColormapsReply) +
		     pWin->drawable.pScreen->maxInstalledCmaps *
		     sizeof(Colormap));
    if(!preply)
        return(BadAlloc);

    preply->type = X_Reply;
    preply->sequenceNumber = client->sequence;
    nummaps = (*pWin->drawable.pScreen->ListInstalledColormaps)
        (pWin->drawable.pScreen, (Colormap *)&preply[1]);
    preply->nColormaps = nummaps;
    preply->length = nummaps;
    WriteReplyToClient(client, sizeof (xListInstalledColormapsReply), preply);
    client->pSwapReplyFunc = (ReplySwapPtr) Swap32Write;
    WriteSwappedDataToClient(client, nummaps * sizeof(Colormap), &preply[1]);
    xfree(preply);
    rc = client->noClientException;
out:
    return rc;
}

int
ProcAllocColor (ClientPtr client)
{
    ColormapPtr pmap;
    int rc;
    xAllocColorReply acr;
    REQUEST(xAllocColorReq);

    REQUEST_SIZE_MATCH(xAllocColorReq);
    rc = dixLookupResource((pointer *)&pmap, stuff->cmap, RT_COLORMAP, client,
			   DixAddAccess);
    if (rc == Success)
    {
	acr.type = X_Reply;
	acr.length = 0;
	acr.sequenceNumber = client->sequence;
	acr.red = stuff->red;
	acr.green = stuff->green;
	acr.blue = stuff->blue;
	acr.pixel = 0;
	if( (rc = AllocColor(pmap, &acr.red, &acr.green, &acr.blue,
	                       &acr.pixel, client->index)) )
	{
            if (client->noClientException != Success)
                return(client->noClientException);
	    else
	        return rc;
	}
#ifdef PANORAMIX
	if (noPanoramiXExtension || !pmap->pScreen->myNum)
#endif
        WriteReplyToClient(client, sizeof(xAllocColorReply), &acr);
	return (client->noClientException);

    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcAllocNamedColor (ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xAllocNamedColorReq);

    REQUEST_FIXED_SIZE(xAllocNamedColorReq, stuff->nbytes);
    rc = dixLookupResource((pointer *)&pcmp, stuff->cmap, RT_COLORMAP, client,
			   DixAddAccess);
    if (rc == Success)
    {
	xAllocNamedColorReply ancr;

	ancr.type = X_Reply;
	ancr.length = 0;
	ancr.sequenceNumber = client->sequence;

	if(OsLookupColor(pcmp->pScreen->myNum, (char *)&stuff[1], stuff->nbytes,
	                 &ancr.exactRed, &ancr.exactGreen, &ancr.exactBlue))
	{
	    ancr.screenRed = ancr.exactRed;
	    ancr.screenGreen = ancr.exactGreen;
	    ancr.screenBlue = ancr.exactBlue;
	    ancr.pixel = 0;
	    if( (rc = AllocColor(pcmp,
	                 &ancr.screenRed, &ancr.screenGreen, &ancr.screenBlue,
			 &ancr.pixel, client->index)) )
	    {
                if (client->noClientException != Success)
                    return(client->noClientException);
                else
		    return rc;
	    }
#ifdef PANORAMIX
	    if (noPanoramiXExtension || !pcmp->pScreen->myNum)
#endif
            WriteReplyToClient(client, sizeof (xAllocNamedColorReply), &ancr);
	    return (client->noClientException);
	}
	else
	    return(BadName);
	
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcAllocColorCells (ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xAllocColorCellsReq);

    REQUEST_SIZE_MATCH(xAllocColorCellsReq);
    rc = dixLookupResource((pointer *)&pcmp, stuff->cmap, RT_COLORMAP, client,
			   DixAddAccess);
    if (rc == Success)
    {
	xAllocColorCellsReply	accr;
	int			npixels, nmasks;
	long			length;
	Pixel			*ppixels, *pmasks;

	npixels = stuff->colors;
	if (!npixels)
	{
	    client->errorValue = npixels;
	    return (BadValue);
	}
	if (stuff->contiguous != xTrue && stuff->contiguous != xFalse)
	{
	    client->errorValue = stuff->contiguous;
	    return (BadValue);
	}
	nmasks = stuff->planes;
	length = ((long)npixels + (long)nmasks) * sizeof(Pixel);
	ppixels = (Pixel *)xalloc(length);
	if(!ppixels)
            return(BadAlloc);
	pmasks = ppixels + npixels;

	if( (rc = AllocColorCells(client->index, pcmp, npixels, nmasks, 
				    (Bool)stuff->contiguous, ppixels, pmasks)) )
	{
	    xfree(ppixels);
            if (client->noClientException != Success)
                return(client->noClientException);
	    else
	        return rc;
	}
#ifdef PANORAMIX
	if (noPanoramiXExtension || !pcmp->pScreen->myNum)
#endif
	{
	    accr.type = X_Reply;
	    accr.length = length >> 2;
	    accr.sequenceNumber = client->sequence;
	    accr.nPixels = npixels;
	    accr.nMasks = nmasks;
	    WriteReplyToClient(client, sizeof (xAllocColorCellsReply), &accr);
	    client->pSwapReplyFunc = (ReplySwapPtr) Swap32Write;
	    WriteSwappedDataToClient(client, length, ppixels);
	}
	xfree(ppixels);
        return (client->noClientException);        
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcAllocColorPlanes(ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xAllocColorPlanesReq);

    REQUEST_SIZE_MATCH(xAllocColorPlanesReq);
    rc = dixLookupResource((pointer *)&pcmp, stuff->cmap, RT_COLORMAP, client,
			   DixAddAccess);
    if (rc == Success)
    {
	xAllocColorPlanesReply	acpr;
	int			npixels;
	long			length;
	Pixel			*ppixels;

	npixels = stuff->colors;
	if (!npixels)
	{
	    client->errorValue = npixels;
	    return (BadValue);
	}
	if (stuff->contiguous != xTrue && stuff->contiguous != xFalse)
	{
	    client->errorValue = stuff->contiguous;
	    return (BadValue);
	}
	acpr.type = X_Reply;
	acpr.sequenceNumber = client->sequence;
	acpr.nPixels = npixels;
	length = (long)npixels * sizeof(Pixel);
	ppixels = (Pixel *)xalloc(length);
	if(!ppixels)
            return(BadAlloc);
	if( (rc = AllocColorPlanes(client->index, pcmp, npixels,
	    (int)stuff->red, (int)stuff->green, (int)stuff->blue,
	    (Bool)stuff->contiguous, ppixels,
	    &acpr.redMask, &acpr.greenMask, &acpr.blueMask)) )
	{
            xfree(ppixels);
            if (client->noClientException != Success)
                return(client->noClientException);
	    else
	        return rc;
	}
	acpr.length = length >> 2;
#ifdef PANORAMIX
	if (noPanoramiXExtension || !pcmp->pScreen->myNum)
#endif
	{
	    WriteReplyToClient(client, sizeof(xAllocColorPlanesReply), &acpr);
	    client->pSwapReplyFunc = (ReplySwapPtr) Swap32Write;
	    WriteSwappedDataToClient(client, length, ppixels);
	}
	xfree(ppixels);
        return (client->noClientException);        
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcFreeColors(ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xFreeColorsReq);

    REQUEST_AT_LEAST_SIZE(xFreeColorsReq);
    rc = dixLookupResource((pointer *)&pcmp, stuff->cmap, RT_COLORMAP, client,
			   DixRemoveAccess);
    if (rc == Success)
    {
	int	count;

	if(pcmp->flags & AllAllocated)
	    return(BadAccess);
	count = ((client->req_len << 2)- sizeof(xFreeColorsReq)) >> 2;
	rc = FreeColors(pcmp, client->index, count,
	    (Pixel *)&stuff[1], (Pixel)stuff->planeMask);
        if (client->noClientException != Success)
            return(client->noClientException);
        else
	{
	    client->errorValue = clientErrorValue;
            return rc;
	}

    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcStoreColors (ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xStoreColorsReq);

    REQUEST_AT_LEAST_SIZE(xStoreColorsReq);
    rc = dixLookupResource((pointer *)&pcmp, stuff->cmap, RT_COLORMAP, client,
			   DixWriteAccess);
    if (rc == Success)
    {
	int	count;

        count = (client->req_len << 2) - sizeof(xStoreColorsReq);
	if (count % sizeof(xColorItem))
	    return(BadLength);
	count /= sizeof(xColorItem);
	rc = StoreColors(pcmp, count, (xColorItem *)&stuff[1]);
        if (client->noClientException != Success)
            return(client->noClientException);
        else
	{
	    client->errorValue = clientErrorValue;
            return rc;
	}
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcStoreNamedColor (ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xStoreNamedColorReq);

    REQUEST_FIXED_SIZE(xStoreNamedColorReq, stuff->nbytes);
    rc = dixLookupResource((pointer *)&pcmp, stuff->cmap, RT_COLORMAP, client,
			   DixWriteAccess);
    if (rc == Success)
    {
	xColorItem	def;

	if(OsLookupColor(pcmp->pScreen->myNum, (char *)&stuff[1],
	                 stuff->nbytes, &def.red, &def.green, &def.blue))
	{
	    def.flags = stuff->flags;
	    def.pixel = stuff->pixel;
	    rc = StoreColors(pcmp, 1, &def);
            if (client->noClientException != Success)
                return(client->noClientException);
	    else
		return rc;
	}
        return (BadName);        
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcQueryColors(ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xQueryColorsReq);

    REQUEST_AT_LEAST_SIZE(xQueryColorsReq);
    rc = dixLookupResource((pointer *)&pcmp, stuff->cmap, RT_COLORMAP, client,
			   DixReadAccess);
    if (rc == Success)
    {
	int			count;
	xrgb 			*prgbs;
	xQueryColorsReply	qcr;

	count = ((client->req_len << 2) - sizeof(xQueryColorsReq)) >> 2;
	prgbs = (xrgb *)xalloc(count * sizeof(xrgb));
	if(!prgbs && count)
            return(BadAlloc);
	if( (rc = QueryColors(pcmp, count, (Pixel *)&stuff[1], prgbs)) )
	{
   	    if (prgbs) xfree(prgbs);
	    if (client->noClientException != Success)
                return(client->noClientException);
	    else
	    {
		client->errorValue = clientErrorValue;
	        return rc;
	    }
	}
	qcr.type = X_Reply;
	qcr.length = (count * sizeof(xrgb)) >> 2;
	qcr.sequenceNumber = client->sequence;
	qcr.nColors = count;
	WriteReplyToClient(client, sizeof(xQueryColorsReply), &qcr);
	if (count)
	{
	    client->pSwapReplyFunc = (ReplySwapPtr) SQColorsExtend;
	    WriteSwappedDataToClient(client, count * sizeof(xrgb), prgbs);
	}
	if (prgbs) xfree(prgbs);
	return(client->noClientException);
	
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
} 

int
ProcLookupColor(ClientPtr client)
{
    ColormapPtr pcmp;
    int rc;
    REQUEST(xLookupColorReq);

    REQUEST_FIXED_SIZE(xLookupColorReq, stuff->nbytes);
    rc = dixLookupResource((pointer *)&pcmp, stuff->cmap, RT_COLORMAP, client,
			   DixReadAccess);
    if (rc == Success)
    {
	xLookupColorReply lcr;

	if(OsLookupColor(pcmp->pScreen->myNum, (char *)&stuff[1], stuff->nbytes,
	                 &lcr.exactRed, &lcr.exactGreen, &lcr.exactBlue))
	{
	    lcr.type = X_Reply;
	    lcr.length = 0;
	    lcr.sequenceNumber = client->sequence;
	    lcr.screenRed = lcr.exactRed;
	    lcr.screenGreen = lcr.exactGreen;
	    lcr.screenBlue = lcr.exactBlue;
	    (*pcmp->pScreen->ResolveColor)(&lcr.screenRed,
	                                   &lcr.screenGreen,
					   &lcr.screenBlue,
					   pcmp->pVisual);
	    WriteReplyToClient(client, sizeof(xLookupColorReply), &lcr);
	    return(client->noClientException);
	}
        return (BadName);        
    }
    else
    {
        client->errorValue = stuff->cmap;
        return (rc == BadValue) ? BadColor : rc;
    }
}

int
ProcCreateCursor (ClientPtr client)
{
    CursorPtr		pCursor;
    PixmapPtr 		src;
    PixmapPtr 		msk;
    unsigned char *	srcbits;
    unsigned char *	mskbits;
    unsigned short	width, height;
    long		n;
    CursorMetricRec 	cm;
    int rc;

    REQUEST(xCreateCursorReq);

    REQUEST_SIZE_MATCH(xCreateCursorReq);
    LEGAL_NEW_RESOURCE(stuff->cid, client);

    rc = dixLookupResource((pointer *)&src, stuff->source, RT_PIXMAP, client,
			   DixReadAccess);
    if (rc != Success) {
	client->errorValue = stuff->source;
	return (rc == BadValue) ? BadPixmap : rc;
    }

    rc = dixLookupResource((pointer *)&msk, stuff->mask, RT_PIXMAP, client,
			   DixReadAccess);
    if (rc != Success)
    {
	if (stuff->mask != None)
	{
	    client->errorValue = stuff->mask;
	    return (rc == BadValue) ? BadPixmap : rc;
	}
    }
    else if (  src->drawable.width != msk->drawable.width
	    || src->drawable.height != msk->drawable.height
	    || src->drawable.depth != 1
	    || msk->drawable.depth != 1)
	return (BadMatch);

    width = src->drawable.width;
    height = src->drawable.height;

    if ( stuff->x > width 
      || stuff->y > height )
	return (BadMatch);

    n = BitmapBytePad(width)*height;
    srcbits = xcalloc(1, n);
    if (!srcbits)
	return (BadAlloc);
    mskbits = xalloc(n);
    if (!mskbits)
    {
	xfree(srcbits);
	return (BadAlloc);
    }

    (* src->drawable.pScreen->GetImage)( (DrawablePtr)src, 0, 0, width, height,
					 XYPixmap, 1, (pointer)srcbits);
    if ( msk == (PixmapPtr)NULL)
    {
	unsigned char *bits = mskbits;
	while (--n >= 0)
	    *bits++ = ~0;
    }
    else
    {
	/* zeroing the (pad) bits helps some ddx cursor handling */
	bzero((char *)mskbits, n);
	(* msk->drawable.pScreen->GetImage)( (DrawablePtr)msk, 0, 0, width,
					height, XYPixmap, 1, (pointer)mskbits);
    }
    cm.width = width;
    cm.height = height;
    cm.xhot = stuff->x;
    cm.yhot = stuff->y;
    rc = AllocARGBCursor(srcbits, mskbits, NULL, &cm,
			 stuff->foreRed, stuff->foreGreen, stuff->foreBlue,
			 stuff->backRed, stuff->backGreen, stuff->backBlue,
			 &pCursor, client, stuff->cid);

    if (rc != Success)
	return rc;
    if (!AddResource(stuff->cid, RT_CURSOR, (pointer)pCursor))
	return BadAlloc;

    return client->noClientException;
}

int
ProcCreateGlyphCursor (ClientPtr client)
{
    CursorPtr pCursor;
    int res;

    REQUEST(xCreateGlyphCursorReq);

    REQUEST_SIZE_MATCH(xCreateGlyphCursorReq);
    LEGAL_NEW_RESOURCE(stuff->cid, client);

    res = AllocGlyphCursor(stuff->source, stuff->sourceChar,
			   stuff->mask, stuff->maskChar,
			   stuff->foreRed, stuff->foreGreen, stuff->foreBlue,
			   stuff->backRed, stuff->backGreen, stuff->backBlue,
			   &pCursor, client, stuff->cid);
    if (res != Success)
	return res;
    if (AddResource(stuff->cid, RT_CURSOR, (pointer)pCursor))
	return client->noClientException;
    return BadAlloc;
}


int
ProcFreeCursor (ClientPtr client)
{
    CursorPtr pCursor;
    int rc;
    REQUEST(xResourceReq);

    REQUEST_SIZE_MATCH(xResourceReq);
    rc = dixLookupResource((pointer *)&pCursor, stuff->id, RT_CURSOR, client,
			   DixDestroyAccess);
    if (rc == Success) 
    {
	FreeResource(stuff->id, RT_NONE);
	return (client->noClientException);
    }
    else 
    {
	client->errorValue = stuff->id;
	return (rc == BadValue) ? BadCursor : rc;
    }
}

int
ProcQueryBestSize (ClientPtr client)
{
    xQueryBestSizeReply	reply;
    DrawablePtr pDraw;
    ScreenPtr pScreen;
    int rc;
    REQUEST(xQueryBestSizeReq);
    REQUEST_SIZE_MATCH(xQueryBestSizeReq);

    if ((stuff->class != CursorShape) && 
	(stuff->class != TileShape) && 
	(stuff->class != StippleShape))
    {
	client->errorValue = stuff->class;
        return(BadValue);
    }

    rc = dixLookupDrawable(&pDraw, stuff->drawable, client, M_ANY,
			   DixGetAttrAccess);
    if (rc != Success)
	return rc;
    if (stuff->class != CursorShape && pDraw->type == UNDRAWABLE_WINDOW)
	return (BadMatch);
    pScreen = pDraw->pScreen;
    rc = XaceHook(XACE_SCREEN_ACCESS, client, pScreen, DixGetAttrAccess);
    if (rc != Success)
	return rc;
    (* pScreen->QueryBestSize)(stuff->class, &stuff->width,
			       &stuff->height, pScreen);
    reply.type = X_Reply;
    reply.length = 0;
    reply.sequenceNumber = client->sequence;
    reply.width = stuff->width;
    reply.height = stuff->height;
    WriteReplyToClient(client, sizeof(xQueryBestSizeReply), &reply);
    return (client->noClientException);
}


int
ProcSetScreenSaver (ClientPtr client)
{
    int rc, i, blankingOption, exposureOption;
    REQUEST(xSetScreenSaverReq);
    REQUEST_SIZE_MATCH(xSetScreenSaverReq);

    for (i = 0; i < screenInfo.numScreens; i++) {
	rc = XaceHook(XACE_SCREENSAVER_ACCESS, client, screenInfo.screens[i],
		      DixSetAttrAccess);
	if (rc != Success)
	    return rc;
    }

    blankingOption = stuff->preferBlank;
    if ((blankingOption != DontPreferBlanking) &&
        (blankingOption != PreferBlanking) &&
        (blankingOption != DefaultBlanking))
    {
	client->errorValue = blankingOption;
        return BadValue;
    }
    exposureOption = stuff->allowExpose;
    if ((exposureOption != DontAllowExposures) &&
        (exposureOption != AllowExposures) &&
        (exposureOption != DefaultExposures))
    {
	client->errorValue = exposureOption;
        return BadValue;
    }
    if (stuff->timeout < -1)
    {
	client->errorValue = stuff->timeout;
        return BadValue;
    }
    if (stuff->interval < -1)
    {
	client->errorValue = stuff->interval;
        return BadValue;
    }

    if (blankingOption == DefaultBlanking)
	ScreenSaverBlanking = defaultScreenSaverBlanking;
    else
	ScreenSaverBlanking = blankingOption; 
    if (exposureOption == DefaultExposures)
	ScreenSaverAllowExposures = defaultScreenSaverAllowExposures;
    else
	ScreenSaverAllowExposures = exposureOption;

    if (stuff->timeout >= 0)
	ScreenSaverTime = stuff->timeout * MILLI_PER_SECOND;
    else 
	ScreenSaverTime = defaultScreenSaverTime;
    if (stuff->interval >= 0)
	ScreenSaverInterval = stuff->interval * MILLI_PER_SECOND;
    else
	ScreenSaverInterval = defaultScreenSaverInterval;

    SetScreenSaverTimer();
    return (client->noClientException);
}

int
ProcGetScreenSaver(ClientPtr client)
{
    xGetScreenSaverReply rep;
    int rc, i;
    REQUEST_SIZE_MATCH(xReq);

    for (i = 0; i < screenInfo.numScreens; i++) {
	rc = XaceHook(XACE_SCREENSAVER_ACCESS, client, screenInfo.screens[i],
		      DixGetAttrAccess);
	if (rc != Success)
	    return rc;
    }

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.timeout = ScreenSaverTime / MILLI_PER_SECOND;
    rep.interval = ScreenSaverInterval / MILLI_PER_SECOND;
    rep.preferBlanking = ScreenSaverBlanking;
    rep.allowExposures = ScreenSaverAllowExposures;
    WriteReplyToClient(client, sizeof(xGetScreenSaverReply), &rep);
    return (client->noClientException);
}

int
ProcChangeHosts(ClientPtr client)
{
    REQUEST(xChangeHostsReq);
    int result;

    REQUEST_FIXED_SIZE(xChangeHostsReq, stuff->hostLength);

    if(stuff->mode == HostInsert)
	result = AddHost(client, (int)stuff->hostFamily,
			 stuff->hostLength, (pointer)&stuff[1]);
    else if (stuff->mode == HostDelete)
	result = RemoveHost(client, (int)stuff->hostFamily, 
			    stuff->hostLength, (pointer)&stuff[1]);  
    else
    {
	client->errorValue = stuff->mode;
        return BadValue;
    }
    if (!result)
	result = client->noClientException;
    return (result);
}

int
ProcListHosts(ClientPtr client)
{
    xListHostsReply reply;
    int	len, nHosts, result;
    pointer	pdata;
    /* REQUEST(xListHostsReq); */

    REQUEST_SIZE_MATCH(xListHostsReq);

    /* untrusted clients can't list hosts */
    result = XaceHook(XACE_SERVER_ACCESS, client, DixReadAccess);
    if (result != Success)
	return result;

    result = GetHosts(&pdata, &nHosts, &len, &reply.enabled);
    if (result != Success)
	return(result);
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.nHosts = nHosts;
    reply.length = len >> 2;
    WriteReplyToClient(client, sizeof(xListHostsReply), &reply);
    if (nHosts)
    {
	client->pSwapReplyFunc = (ReplySwapPtr) SLHostsExtend;
	WriteSwappedDataToClient(client, len, pdata);
    }
    xfree(pdata);
    return (client->noClientException);
}

int
ProcChangeAccessControl(ClientPtr client)
{
    int result;
    REQUEST(xSetAccessControlReq);

    REQUEST_SIZE_MATCH(xSetAccessControlReq);
    if ((stuff->mode != EnableAccess) && (stuff->mode != DisableAccess))
    {
	client->errorValue = stuff->mode;
        return BadValue;
    }
    result = ChangeAccessControl(client, stuff->mode == EnableAccess);
    if (!result)
	result = client->noClientException;
    return (result);
}

/*********************
 * CloseDownRetainedResources
 *
 *    Find all clients that are gone and have terminated in RetainTemporary 
 *    and destroy their resources.
 *********************/

static void
CloseDownRetainedResources(void)
{
    int i;
    ClientPtr client;

    for (i=1; i<currentMaxClients; i++)
    {
        client = clients[i];
        if (client && (client->closeDownMode == RetainTemporary)
	    && (client->clientGone))
	    CloseDownClient(client);
    }
}

int
ProcKillClient(ClientPtr client)
{
    REQUEST(xResourceReq);
    ClientPtr killclient;
    int rc;

    REQUEST_SIZE_MATCH(xResourceReq);
    if (stuff->id == AllTemporary)
    {
	CloseDownRetainedResources();
        return (client->noClientException);
    }

    rc = dixLookupClient(&killclient, stuff->id, client, DixDestroyAccess);
    if (rc == Success) {
	CloseDownClient(killclient);
	/* if an LBX proxy gets killed, isItTimeToYield will be set */
	if (isItTimeToYield || (client == killclient))
	{
	    /* force yield and return Success, so that Dispatch()
	     * doesn't try to touch client
	     */
	    isItTimeToYield = TRUE;
	    return (Success);
	}
	return (client->noClientException);
    }
    else
	return rc;
}

int
ProcSetFontPath(ClientPtr client)
{
    unsigned char *ptr;
    unsigned long nbytes, total;
    long nfonts;
    int n, result;
    int error;
    REQUEST(xSetFontPathReq);
    
    REQUEST_AT_LEAST_SIZE(xSetFontPathReq);
    
    nbytes = (client->req_len << 2) - sizeof(xSetFontPathReq);
    total = nbytes;
    ptr = (unsigned char *)&stuff[1];
    nfonts = stuff->nFonts;
    while (--nfonts >= 0)
    {
	if ((total == 0) || (total < (n = (*ptr + 1))))
	    return(BadLength);
	total -= n;
	ptr += n;
    }
    if (total >= 4)
	return(BadLength);
    result = SetFontPath(client, stuff->nFonts, (unsigned char *)&stuff[1],
			 &error);
    if (!result)
    {
	result = client->noClientException;
	client->errorValue = error;
    }
    return (result);
}

int
ProcGetFontPath(ClientPtr client)
{
    xGetFontPathReply reply;
    int rc, stringLens, numpaths;
    unsigned char *bufferStart;
    /* REQUEST (xReq); */

    REQUEST_SIZE_MATCH(xReq);
    rc = GetFontPath(client, &numpaths, &stringLens, &bufferStart);
    if (rc != Success)
	return rc;

    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = (stringLens + numpaths + 3) >> 2;
    reply.nPaths = numpaths;

    WriteReplyToClient(client, sizeof(xGetFontPathReply), &reply);
    if (stringLens || numpaths)
	(void)WriteToClient(client, stringLens + numpaths, (char *)bufferStart);
    return(client->noClientException);
}

int
ProcChangeCloseDownMode(ClientPtr client)
{
    int rc;
    REQUEST(xSetCloseDownModeReq);
    REQUEST_SIZE_MATCH(xSetCloseDownModeReq);

    rc = XaceHook(XACE_CLIENT_ACCESS, client, client, DixManageAccess);
    if (rc != Success)
	return rc;

    if ((stuff->mode == AllTemporary) ||
	(stuff->mode == RetainPermanent) ||
	(stuff->mode == RetainTemporary))
    {
	client->closeDownMode = stuff->mode;
	return (client->noClientException);
    }
    else   
    {
	client->errorValue = stuff->mode;
	return (BadValue);
    }
}

int ProcForceScreenSaver(ClientPtr client)
{    
    int rc;
    REQUEST(xForceScreenSaverReq);

    REQUEST_SIZE_MATCH(xForceScreenSaverReq);
    
    if ((stuff->mode != ScreenSaverReset) && 
	(stuff->mode != ScreenSaverActive))
    {
	client->errorValue = stuff->mode;
        return BadValue;
    }
    rc = dixSaveScreens(client, SCREEN_SAVER_FORCER, (int)stuff->mode);
    if (rc != Success)
	return rc;
    return client->noClientException;
}

int ProcNoOperation(ClientPtr client)
{
    REQUEST_AT_LEAST_SIZE(xReq);
    
    /* noop -- don't do anything */
    return(client->noClientException);
}

void
InitProcVectors(void)
{
    int i;
    for (i = 0; i<256; i++)
    {
	if(!ProcVector[i])
	{
            ProcVector[i] = SwappedProcVector[i] = ProcBadRequest;
	    ReplySwapVector[i] = ReplyNotSwappd;
	}
    }
    for(i = LASTEvent; i < 128; i++)
    {
	EventSwapVector[i] = NotImplemented;
    }
    
}

/**********************
 * CloseDownClient
 *
 *  Client can either mark his resources destroy or retain.  If retained and
 *  then killed again, the client is really destroyed.
 *********************/

char dispatchExceptionAtReset = DE_RESET;

void
CloseDownClient(ClientPtr client)
{
    Bool really_close_down = client->clientGone ||
			     client->closeDownMode == DestroyAll;

    if (!client->clientGone)
    {
	/* ungrab server if grabbing client dies */
	if (grabState != GrabNone && grabClient == client)
	{
	    UngrabServer(client);
	}
	BITCLEAR(grabWaiters, client->index);
	DeleteClientFromAnySelections(client);
	ReleaseActiveGrabs(client);
	DeleteClientFontStuff(client);
	if (!really_close_down)
	{
	    /*  This frees resources that should never be retained
	     *  no matter what the close down mode is.  Actually we
	     *  could do this unconditionally, but it's probably
	     *  better not to traverse all the client's resources
	     *  twice (once here, once a few lines down in
	     *  FreeClientResources) in the common case of
	     *  really_close_down == TRUE.
	     */
	    FreeClientNeverRetainResources(client);
	    client->clientState = ClientStateRetained;
  	    if (ClientStateCallback)
            {
		NewClientInfoRec clientinfo;

		clientinfo.client = client; 
		clientinfo.prefix = (xConnSetupPrefix *)NULL;  
		clientinfo.setup = (xConnSetup *) NULL;
		CallCallbacks((&ClientStateCallback), (pointer)&clientinfo);
            } 
	}
	client->clientGone = TRUE;  /* so events aren't sent to client */
	if (ClientIsAsleep(client))
	    ClientSignal (client);
	ProcessWorkQueueZombies();
	CloseDownConnection(client);

	/* If the client made it to the Running stage, nClients has
	 * been incremented on its behalf, so we need to decrement it
	 * now.  If it hasn't gotten to Running, nClients has *not*
	 * been incremented, so *don't* decrement it.
	 */
	if (client->clientState != ClientStateInitial &&
	    client->clientState != ClientStateAuthenticating )
	{
	    --nClients;
	}
    }

    if (really_close_down)
    {
	if (client->clientState == ClientStateRunning && nClients == 0)
	    dispatchException |= dispatchExceptionAtReset;

	client->clientState = ClientStateGone;
	if (ClientStateCallback)
	{
	    NewClientInfoRec clientinfo;

	    clientinfo.client = client; 
	    clientinfo.prefix = (xConnSetupPrefix *)NULL;  
	    clientinfo.setup = (xConnSetup *) NULL;
	    CallCallbacks((&ClientStateCallback), (pointer)&clientinfo);
	} 	    
	FreeClientResources(client);
#ifdef XSERVER_DTRACE
	XSERVER_CLIENT_DISCONNECT(client->index);
#endif	
	if (client->index < nextFreeClientID)
	    nextFreeClientID = client->index;
	clients[client->index] = NullClient;
	SmartLastClient = NullClient;
	dixFreePrivates(client->devPrivates);
	xfree(client);

	while (!clients[currentMaxClients-1])
	    currentMaxClients--;
    }
}

static void
KillAllClients(void)
{
    int i;
    for (i=1; i<currentMaxClients; i++)
        if (clients[i]) {
            /* Make sure Retained clients are released. */
            clients[i]->closeDownMode = DestroyAll;
            CloseDownClient(clients[i]);     
        }
}

void InitClient(ClientPtr client, int i, pointer ospriv)
{
    client->index = i;
    client->sequence = 0; 
    client->clientAsMask = ((Mask)i) << CLIENTOFFSET;
    client->clientGone = FALSE;
    client->closeDownMode = i ? DestroyAll : RetainPermanent;
    client->numSaved = 0;
    client->saveSet = (SaveSetElt *)NULL;
    client->noClientException = Success;
#ifdef DEBUG
    client->requestLogIndex = 0;
#endif
    client->requestVector = InitialVector;
    client->osPrivate = ospriv;
    client->swapped = FALSE;
    client->big_requests = FALSE;
    client->priority = 0;
    client->clientState = ClientStateInitial;
    client->devPrivates = NULL;
#ifdef XKB
    if (!noXkbExtension) {
	client->xkbClientFlags = 0;
	client->mapNotifyMask = 0;
	client->newKeyboardNotifyMask = 0;
	client->vMinor = client->vMajor = 0;
	QueryMinMaxKeyCodes(&client->minKC,&client->maxKC);
    }
#endif
    client->replyBytesRemaining = 0;
    client->fontResFunc = NULL;
    client->smart_priority = 0;
    client->smart_start_tick = SmartScheduleTime;
    client->smart_stop_tick = SmartScheduleTime;
    client->smart_check_tick = SmartScheduleTime;

    client->clientPtr = NULL;
}

/************************
 * int NextAvailableClient(ospriv)
 *
 * OS dependent portion can't assign client id's because of CloseDownModes.
 * Returns NULL if there are no free clients.
 *************************/

ClientPtr NextAvailableClient(pointer ospriv)
{
    int i;
    ClientPtr client;
    xReq data;

    i = nextFreeClientID;
    if (i == MAXCLIENTS)
	return (ClientPtr)NULL;
    clients[i] = client = (ClientPtr)xalloc(sizeof(ClientRec));
    if (!client)
	return (ClientPtr)NULL;
    InitClient(client, i, ospriv);
    if (!InitClientResources(client))
    {
	xfree(client);
	return (ClientPtr)NULL;
    }
    data.reqType = 1;
    data.length = (sz_xReq + sz_xConnClientPrefix) >> 2;
    if (!InsertFakeRequest(client, (char *)&data, sz_xReq))
    {
	FreeClientResources(client);
	xfree(client);
	return (ClientPtr)NULL;
    }
    if (i == currentMaxClients)
	currentMaxClients++;
    while ((nextFreeClientID < MAXCLIENTS) && clients[nextFreeClientID])
	nextFreeClientID++;
    if (ClientStateCallback)
    {
	NewClientInfoRec clientinfo;

        clientinfo.client = client; 
        clientinfo.prefix = (xConnSetupPrefix *)NULL;  
        clientinfo.setup = (xConnSetup *) NULL;
	CallCallbacks((&ClientStateCallback), (pointer)&clientinfo);
    } 	
    return(client);
}

int
ProcInitialConnection(ClientPtr client)
{
    REQUEST(xReq);
    xConnClientPrefix *prefix;
    int whichbyte = 1;

    prefix = (xConnClientPrefix *)((char *)stuff + sz_xReq);
    if ((prefix->byteOrder != 'l') && (prefix->byteOrder != 'B'))
	return (client->noClientException = -1);
    if (((*(char *) &whichbyte) && (prefix->byteOrder == 'B')) ||
	(!(*(char *) &whichbyte) && (prefix->byteOrder == 'l')))
    {
	client->swapped = TRUE;
	SwapConnClientPrefix(prefix);
    }
    stuff->reqType = 2;
    stuff->length += ((prefix->nbytesAuthProto + (unsigned)3) >> 2) +
		     ((prefix->nbytesAuthString + (unsigned)3) >> 2);
    if (client->swapped)
    {
	swaps(&stuff->length, whichbyte);
    }
    ResetCurrentRequest(client);
    return (client->noClientException);
}

static int
SendConnSetup(ClientPtr client, char *reason)
{
    xWindowRoot *root;
    int i;
    int numScreens;
    char* lConnectionInfo;
    xConnSetupPrefix* lconnSetupPrefix;

    if (reason)
    {
	xConnSetupPrefix csp;

	csp.success = xFalse;
	csp.lengthReason = strlen(reason);
	csp.length = (csp.lengthReason + (unsigned)3) >> 2;
	csp.majorVersion = X_PROTOCOL;
	csp.minorVersion = X_PROTOCOL_REVISION;
	if (client->swapped)
	    WriteSConnSetupPrefix(client, &csp);
	else
	    (void)WriteToClient(client, sz_xConnSetupPrefix, (char *) &csp);
        (void)WriteToClient(client, (int)csp.lengthReason, reason);
	return (client->noClientException = -1);
    }

    numScreens = screenInfo.numScreens;
    lConnectionInfo = ConnectionInfo;
    lconnSetupPrefix = &connSetupPrefix;

    /* We're about to start speaking X protocol back to the client by
     * sending the connection setup info.  This means the authorization
     * step is complete, and we can count the client as an
     * authorized one.
     */
    nClients++;

    client->requestVector = client->swapped ? SwappedProcVector : ProcVector;
    client->sequence = 0;
    ((xConnSetup *)lConnectionInfo)->ridBase = client->clientAsMask;
    ((xConnSetup *)lConnectionInfo)->ridMask = RESOURCE_ID_MASK;
#ifdef MATCH_CLIENT_ENDIAN
    ((xConnSetup *)lConnectionInfo)->imageByteOrder = ClientOrder (client);
    ((xConnSetup *)lConnectionInfo)->bitmapBitOrder = ClientOrder (client);
#endif
    /* fill in the "currentInputMask" */
    root = (xWindowRoot *)(lConnectionInfo + connBlockScreenStart);
#ifdef PANORAMIX
    if (noPanoramiXExtension)
	numScreens = screenInfo.numScreens;
    else 
        numScreens = ((xConnSetup *)ConnectionInfo)->numRoots;
#endif

    for (i=0; i<numScreens; i++) 
    {
	unsigned int j;
	xDepth *pDepth;

        root->currentInputMask = WindowTable[i]->eventMask |
			         wOtherEventMasks (WindowTable[i]);
	pDepth = (xDepth *)(root + 1);
	for (j = 0; j < root->nDepths; j++)
	{
	    pDepth = (xDepth *)(((char *)(pDepth + 1)) +
				pDepth->nVisuals * sizeof(xVisualType));
	}
	root = (xWindowRoot *)pDepth;
    }

    if (client->swapped)
    {
	WriteSConnSetupPrefix(client, lconnSetupPrefix);
	WriteSConnectionInfo(client,
			     (unsigned long)(lconnSetupPrefix->length << 2),
			     lConnectionInfo);
    }
    else
    {
	(void)WriteToClient(client, sizeof(xConnSetupPrefix),
			    (char *) lconnSetupPrefix);
	(void)WriteToClient(client, (int)(lconnSetupPrefix->length << 2),
			    lConnectionInfo);
    }
    client->clientState = ClientStateRunning;
    if (ClientStateCallback)
    {
	NewClientInfoRec clientinfo;

        clientinfo.client = client; 
        clientinfo.prefix = lconnSetupPrefix;  
        clientinfo.setup = (xConnSetup *)lConnectionInfo;
	CallCallbacks((&ClientStateCallback), (pointer)&clientinfo);
    } 	
    return (client->noClientException);
}

int
ProcEstablishConnection(ClientPtr client)
{
    char *reason, *auth_proto, *auth_string;
    xConnClientPrefix *prefix;
    REQUEST(xReq);

    prefix = (xConnClientPrefix *)((char *)stuff + sz_xReq);
    auth_proto = (char *)prefix + sz_xConnClientPrefix;
    auth_string = auth_proto + ((prefix->nbytesAuthProto + 3) & ~3);
    if ((prefix->majorVersion != X_PROTOCOL) ||
	(prefix->minorVersion != X_PROTOCOL_REVISION))
	reason = "Protocol version mismatch";
    else
	reason = ClientAuthorized(client,
				  (unsigned short)prefix->nbytesAuthProto,
				  auth_proto,
				  (unsigned short)prefix->nbytesAuthString,
				  auth_string);
    /*
     * If Kerberos is being used for this client, the clientState
     * will be set to ClientStateAuthenticating at this point.
     * More messages need to be exchanged among the X server, Kerberos
     * server, and client to figure out if everyone is authorized.
     * So we don't want to send the connection setup info yet, since
     * the auth step isn't really done.
     */
    if (client->clientState == ClientStateCheckingSecurity)
	client->clientState = ClientStateCheckedSecurity;
    else if (client->clientState != ClientStateAuthenticating)
	return(SendConnSetup(client, reason));
    return(client->noClientException);
}

_X_EXPORT void
SendErrorToClient(ClientPtr client, unsigned majorCode, unsigned minorCode, 
                  XID resId, int errorCode)
{
    xError rep;

    rep.type = X_Error;
    rep.sequenceNumber = client->sequence;
    rep.errorCode = errorCode;
    rep.majorCode = majorCode;
    rep.minorCode = minorCode;
    rep.resourceID = resId;

    WriteEventsToClient (client, 1, (xEvent *)&rep);
}

void
MarkClientException(ClientPtr client)
{
    client->noClientException = -1;
}
