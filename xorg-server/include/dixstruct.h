/***********************************************************
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

#ifndef DIXSTRUCT_H
#define DIXSTRUCT_H

#include "dix.h"
#include "resource.h"
#include "cursor.h"
#include "gc.h"
#include "pixmap.h"
#include "privates.h"
#include <X11/Xmd.h>

/*
 * 	direct-mapped hash table, used by resource manager to store
 *      translation from client ids to server addresses.
 */

#ifdef DEBUG
#define MAX_REQUEST_LOG 100
#endif

extern CallbackListPtr ClientStateCallback;

typedef struct {
    ClientPtr 		client;
    xConnSetupPrefix 	*prefix; 
    xConnSetup  	*setup;
} NewClientInfoRec;

typedef void (*ReplySwapPtr) (
		ClientPtr	/* pClient */,
		int		/* size */,
		void *		/* pbuf */);

extern void ReplyNotSwappd (
		ClientPtr	/* pClient */,
		int		/* size */,
		void *		/* pbuf */);

typedef enum {ClientStateInitial,
	      ClientStateAuthenticating,
	      ClientStateRunning,
	      ClientStateRetained,
	      ClientStateGone,
	      ClientStateCheckingSecurity,
	      ClientStateCheckedSecurity} ClientState;

#ifdef XFIXES
typedef struct _saveSet {
    struct _Window  *windowPtr;
    Bool	    toRoot;
    Bool	    map;
} SaveSetElt;
#define SaveSetWindow(ss)   ((ss).windowPtr)
#define SaveSetToRoot(ss)   ((ss).toRoot)
#define SaveSetShouldMap(ss)	    ((ss).map)
#define SaveSetAssignWindow(ss,w)   ((ss).windowPtr = (w))
#define SaveSetAssignToRoot(ss,tr)  ((ss).toRoot = (tr))
#define SaveSetAssignMap(ss,m)      ((ss).map = (m))
#else
typedef struct _Window *SaveSetElt;
#define SaveSetWindow(ss)   (ss)
#define SaveSetToRoot(ss)   FALSE
#define SaveSetShouldMap(ss)	    TRUE
#define SaveSetAssignWindow(ss,w)   ((ss) = (w))
#define SaveSetAssignToRoot(ss,tr)
#define SaveSetAssignMap(ss,m)
#endif

typedef struct _Client {
    int         index;
    Mask        clientAsMask;
    pointer     requestBuffer;
    pointer     osPrivate;	/* for OS layer, including scheduler */
    Bool        swapped;
    ReplySwapPtr pSwapReplyFunc;
    XID         errorValue;
    int         sequence;
    int         closeDownMode;
    int         clientGone;
    int         noClientException;	/* this client died or needs to be
					 * killed */
    SaveSetElt	*saveSet;
    int         numSaved;
    pointer     screenPrivate[MAXSCREENS];
    int         (**requestVector) (
		ClientPtr /* pClient */);
    CARD32	req_len;		/* length of current request */
    Bool	big_requests;		/* supports large requests */
    int		priority;
    ClientState clientState;
    PrivateRec	*devPrivates;
#ifdef XKB
    unsigned short	xkbClientFlags;
    unsigned short	mapNotifyMask;
    unsigned short	newKeyboardNotifyMask;
    unsigned short	vMajor,vMinor;
    KeyCode		minKC,maxKC;
#endif

#ifdef DEBUG
    unsigned char requestLog[MAX_REQUEST_LOG];
    int         requestLogIndex;
#endif
    unsigned long replyBytesRemaining;
    void *appgroup; /* Can't remove, ABI */
    struct _FontResolution * (*fontResFunc) (    /* no need for font.h */
		ClientPtr	/* pClient */,
		int *		/* num */);
    int	    smart_priority;
    long    smart_start_tick;
    long    smart_stop_tick;
    long    smart_check_tick;
    
    DeviceIntPtr clientPtr;
}           ClientRec;

/*
 * Scheduling interface
 */
extern long SmartScheduleTime;
extern long SmartScheduleInterval;
extern long SmartScheduleSlice;
extern long SmartScheduleMaxSlice;
extern Bool SmartScheduleDisable;
extern void SmartScheduleStartTimer(void);
extern void SmartScheduleStopTimer(void);
#define SMART_MAX_PRIORITY  (20)
#define SMART_MIN_PRIORITY  (-20)

extern Bool SmartScheduleInit(void);


/* This prototype is used pervasively in Xext, dix */
#define DISPATCH_PROC(func) int func(ClientPtr /* client */)

typedef struct _WorkQueue {
    struct _WorkQueue *next;
    Bool        (*function) (
		ClientPtr	/* pClient */,
		pointer		/* closure */
);
    ClientPtr   client;
    pointer     closure;
}           WorkQueueRec;

extern TimeStamp currentTime;
extern TimeStamp lastDeviceEventTime;

extern int CompareTimeStamps(
    TimeStamp /*a*/,
    TimeStamp /*b*/);

extern TimeStamp ClientTimeToServerTime(CARD32 /*c*/);

typedef struct _CallbackRec {
  CallbackProcPtr proc;
  pointer data;
  Bool deleted;
  struct _CallbackRec *next;
} CallbackRec, *CallbackPtr;

typedef struct _CallbackList {
  int inCallback;
  Bool deleted;
  int numDeleted;
  CallbackPtr list;
} CallbackListRec;

/* proc vectors */

extern int (* InitialVector[3]) (ClientPtr /*client*/);

extern int (* ProcVector[256]) (ClientPtr /*client*/);

extern int (* SwappedProcVector[256]) (ClientPtr /*client*/);

extern ReplySwapPtr ReplySwapVector[256];

extern int ProcBadRequest(ClientPtr /*client*/);

#endif				/* DIXSTRUCT_H */
