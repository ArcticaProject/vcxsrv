/*****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991 by Digital Equipment Corp., Maynard, MA
X11R6 Changes Copyright (c) 1994 by Robert Chesler of Absol-Puter, Hudson, NH.

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL AND ABSOL-PUTER DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL DIGITAL OR ABSOL-PUTER BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*****************************************************************************/
/*
 *  ABSTRACT:
 *
 *      This module is the main module for extension initialization and setup.
 *      It is called by the server and by clients using the extension.
 *      This is shared code and is subject to change only by team approval.
 *
 *  CONTRIBUTORS:
 *
 *      Dick Annicchiarico
 *      Robert Chesler
 *      Gene Durso
 *      Marc Evans
 *      Alan Jamison
 *      Mark Henry
 *      Ken Miller
 *
 *  CHANGES:
 *
 *	Robert Chesler - grab-impreviousness patch to improve grab behavior
 *	Robert Chesler - add client arg to swapping routines for X11R6 port
 *
 */

/*-----------------*
 *  Include Files  *
 *-----------------*/

#define NEED_REPLIES
#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include "inputstr.h"           /* Server DevicePtr definitions */
#include "misc.h"               /* Server swapping macros */
#include "dixstruct.h"          /* Server ClientRec definitions */
#include "resource.h"           /* Used with the MakeAtom call */
#ifdef PC
# include "scrintst.h"          /* Screen struct */
# include "extnsist.h"
#else
# include "extnsionst.h"        /* Server ExtensionEntry definitions */
# include "scrnintstr.h"        /* Screen struct */
#endif
#include "pixmapstr.h"          /* DrawableRec */
#include "windowstr.h"          /* Drawable Lookup structures */
#include <X11/extensions/xtrapdi.h>
#include <X11/extensions/xtrapddmi.h>
#include <X11/extensions/xtrapproto.h>
#include "colormapst.h"
#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#include "cursor.h"
#endif


/*----------------------------*
 *  Global Data Declarations  *
 *----------------------------*/

DevicePtr XETrapKbdDev                       = NULL;
DevicePtr XETrapPtrDev                       = NULL;
int    XETrapErrorBase                       = 0L;
xXTrapGetAvailReply XETrap_avail;            /* What's available to clients */

globalref int_function XETrapDispatchVector[10L]; /* Vector of XTrap Rtns */
globalref int_function XETSwDispatchVector[10L];  /* Swapped XTrap Rtns */

globalref int_function XETrapProcVector[256L]; /* The "shadowed" ProcVector */
    /* The "real" EventVector (XTrap creates it till events 
     * truly become vectored
     */
#ifndef VECTORED_EVENTS
globalref int_function EventProcVector[XETrapCoreEvents];
#else
extern WindowPtr GetCurrentRootWindow();
globalref int_function EventProcVector[128L];
#endif
static int_function keybd_process_inp = NULL;  /* Used for VECTORED_EVENTS */
    /* The "shadowed" Event Vector */
globalref int_function XETrapEventProcVector[XETrapCoreEvents];

globalref void_function XETSwProcVector[256L];/* Vector of Req swapping rtns */

/* This macro returns a true/false indicator based on whether it changes the
 * environment state bits local to the server extension. This is based on the
 * idea that if a valid flag is set and the corresponding data bit is not in
 * the same state as desired, then true is returned, otherwise false is
 * returned.
 */
#define _CheckChangeBit(valid,data,local,bit) \
    (BitIsFalse(valid,bit) ? 0L :                                \
        (((BitIsTrue(data,bit) && BitIsTrue(local,bit)) ||       \
         (BitIsFalse(data,bit) && BitIsFalse(local,bit))) ? 0L : \
         (BitToggle(local,bit), 1L)))

/* This macro is designed to return the number of elements in an
 * automatically allocated array.
 */
#ifndef ASIZE
#define ASIZE(array)    (sizeof(array)/sizeof(array[0L]))
#endif

/* This macro is designed to return the number of long words beyond
 * XETrapMinRepSize contained in a data structure.
 */
#ifndef XEXTRA
#define XEXTRA(s) \
    ((sizeof(s)+(sizeof(CARD32)-1L)-XETrapMinRepSize)/sizeof(CARD32))
#endif

/* Static Declarations known to XTrap Only
 * All XTrap clients refer to these single copies!
 */
/* This carries all of the information XTrap uses for internal information */
static XETrapEnv *XETenv[MAXCLIENTS]         = {NULL};
#ifndef RESTYPE
#define RESTYPE unsigned long
#endif
static RESTYPE   XETrapClass = 0L;      /* Resource class for this extension */
static RESTYPE   XETrapType  = 0L;      /* Resource type for this extension */
static Bool      gate_closed = False;   /* The global "gatekeeper" */
static Bool      key_ignore  = False;   /* The global "keymaster" */
static Bool      ignore_grabs = False;
static CARD8     next_key = XEKeyIsClear; /* Echo, Clear, or Other */
#ifdef VECTORED_EVENTS
static INT16     current_screen = -1L;   /* Current screen for events */
#endif
static INT16     vectored_requests[256L] = {0L}; /* cnt of vectoring clients */
static INT16     vectored_events[KeyPress+MotionNotify]  = {0L};
typedef struct _client_list
{
    struct _client_list    *next;
    ClientPtr              client;
} ClientList;
static ClientList io_clients;  /* Linked-list of clients currently doing I/O */
static ClientList stats_clients;  /* Linked-list of clients collecting stats */
static ClientList cmd_clients;   /* Linked-list of clients using command key */

/*----------------------------*
 *  Forward Declarations
 *----------------------------*/
static void _SwapProc (int (**f1 )(void), int (**f2 )(void));
static void sXETrapEvent (xETrapDataEvent *from , xETrapDataEvent *to );
static int add_accelerator_node (ClientPtr client , ClientList *accel );
static void remove_accelerator_node (ClientPtr client , ClientList *accel );
static void update_protocol (xXTrapGetReq *reqptr , ClientPtr client );
#ifdef COLOR_REPLIES
static void GetSendColorRep (ClientPtr client , xResourceReq *req );
static void GetSendNamedColorRep (ClientPtr client , xResourceReq *req );
static void GetSendColorCellsRep (ClientPtr client , xResourceReq *req );
static void GetSendColorPlanesRep (ClientPtr client , xResourceReq *req );
#endif

/*
 *  DESCRIPTION:
 *
 *      This routine is called by the server when a given client
 *      goes away (as identified by the first argument). All
 *      memory allocations, misc. values specific to a given
 *      client would be reset here.
 *
 */
int XETrapDestroyEnv(pointer value, XID id)
{
    xXTrapReq request;
    XETrapEnv *penv = XETenv[(long)value];

    XETrapReset(&request,penv->client);
    /* Free any memory malloc'd for a particular client here */
    /* In case stop I/O wasn't received for this client */
    if (penv->stats)
    {   /* Remove from client accelerator list */
        remove_accelerator_node(penv->client, &stats_clients);
        Xfree(penv->stats);     /* Free the stats buckets */
    }
    if (cmd_clients.next == NULL)
    { /* No more command key clients!  Let's reset the gate */
        gate_closed = False;
        key_ignore  = False;
        next_key = XEKeyIsClear;
    }

#ifdef VECTORED_EVENTS
    current_screen = -1L;       /* Invalidate current screen */
#endif

#ifdef VERBOSE
    ErrorF("%s:  Client '%d' Disconnected\n", XTrapExtName, 
        penv->client->index);
#endif

    Xfree(penv);
    XETenv[(long)value] = NULL;

    return 0;
}                       

/*
 *  DESCRIPTION:
 *
 *      This routine is called by the server when the last client
 *      (the session manager in most cases) goes away. This is server
 *      reset. When the server comes back up, this extension will not
 *      be loaded unless this routine makes the proper arrangements.
 *
 *      The real concern here is to unload the extension
 *      and possibly make arragements to be called upon
 *      server restart.
 *
 */
void XETrapCloseDown(ExtensionEntry *extEntry)
{                                           
    long i;

    for (i=0L; i<MAXCLIENTS; i++)
    {
        if (XETenv[i] != NULL)
        {
            XETrapDestroyEnv((pointer)i,0L);
        }
    }
    ignore_grabs = False;
    return;
}                       

/*
 *
 *  DESCRIPTION:
 *
 *      This routine has been created because of the initialization
 *      order that X uses, such that extensions are initialized before
 *      devices. This means that this extension must perform a second
 *      level of initialization to obtain the device references at some
 *      point after they have been initialized. It is assumed that when
 *      a client establishes communication with the extension that the
 *      devices havae been initialized, and therefore this function can
 *      obtain the information it needs.
 *
 *      In obtaining the information, this function also places its own
 *      functions in place of the *standard* functions. The original
 *      functions are retained for vectoring purposes.
 */

Bool XETrapRedirectDevices()
{
    Bool retval = True;

    /* Do we need to redirect the keyboard device? */
    if (XETrapKbdDev == NULL)
    {
        if ((XETrapKbdDev = (DevicePtr)inputInfo.keyboard) == NULL)
        {
            retval = False;
        }
        else
        {
            EventProcVector[KeyPress] =
                (int_function)XETrapKbdDev->realInputProc;
            EventProcVector[KeyRelease] =
                (int_function)XETrapKbdDev->realInputProc;
        }
#ifdef VECTORED_EVENTS
        keybd_process_inp = EventProcVector[KeyPress];
        EventProcVector[KeyPress] = EventProcVector[KeyRelease] = NULL;
        XETrapEventProcVector[KeyPress]   = XETrapEventVector;
        XETrapEventProcVector[KeyRelease] = XETrapEventVector;
#else   /* !VECTORED_EVENTS */
        XETrapEventProcVector[KeyPress]   = XETrapKeyboard;
        XETrapEventProcVector[KeyRelease] = XETrapKeyboard;
#endif /* !VECTORED_EVENTS */
    }
    /* Do we need to redirect the pointer device? */
#ifndef VECTORED_EVENTS
    if (XETrapPtrDev == NULL)
    {
        if ((XETrapPtrDev = (DevicePtr)inputInfo.pointer) == 0L)
        {
            retval = False;
        }
        else
        {
            EventProcVector[ButtonPress] = 
                (int_function)XETrapPtrDev->realInputProc;
            EventProcVector[ButtonRelease] = 
                (int_function)XETrapPtrDev->realInputProc;
            EventProcVector[MotionNotify] = 
                (int_function)XETrapPtrDev->realInputProc;
        }
        XETrapEventProcVector[ButtonPress]   = XETrapPointer;
        XETrapEventProcVector[ButtonRelease] = XETrapPointer;
        XETrapEventProcVector[MotionNotify]  = XETrapPointer;
    }
#endif /* !VECTORED_EVENTS */
    return(retval);
}

/*
 *
 *  DESCRIPTION:
 *
 *      This routine is the main entry point for the Xtrap extension. It is
 *      called by the server to inititalize the Xtrap extension.  Once the
 *      extension is initialized, life is controlled by the XtrapDispatch
 *      routine by the requests it will handle.
 *
 *      Initializes all the XTrap data structures with the proper
 *      addresses of defined routines that will help control the extension.
 *      It is vital that the extension state be kept accurate so that only
 *      one call to this routine be made.
 *
 */

void DEC_XTRAPInit()
{
    register ExtensionEntry *extEntry;
    unsigned int i;
    Atom a;

    /* Make the extension known to the server. Must be done every time
     * DEC_XTRAPInit is called, else server will think it failed.
     */
    if ((extEntry = AddExtension(XTrapExtName,XETrapNumEvents,
        XETrapNumErrors,XETrapDispatch,sXETrapDispatch,XETrapCloseDown
        ,StandardMinorOpcode)) == NULL)
    {                                        
        ErrorF("%s:  AddExtension Failed!\n", XTrapExtName); 
        return;
    }
#ifdef VERBOSE
        ErrorF("%s:  AddExtension assigned Major Opcode '%d'\n",
            XTrapExtName, extEntry->base);
#endif
    XETrap_avail.data.major_opcode = extEntry->base;
    XETrapErrorBase                = extEntry->errorBase;
    XETrap_avail.data.event_base   = extEntry->eventBase;

    /* Set up our swapped reply vector */
    ReplySwapVector[XETrap_avail.data.major_opcode] = 
	(void_function) sReplyXTrapDispatch;

    /* Set up our swapped event vector */
    EventSwapVector[extEntry->eventBase + XETrapData] = 
	(EventSwapPtr) sXETrapEvent;

    /* make an atom saying that the extension is present.  The 
     * adding of the resource occurs during XETrapCreateEnv().
     */
    if ((a = MakeAtom(XTrapExtName,strlen(XTrapExtName),1L)) == None ||
        (XETrapType  = CreateNewResourceType(XETrapDestroyEnv)) == 0L)
    {
        ErrorF("%s:  Setup can't create new resource type (%d,%d,%d)\n",
          XTrapExtName, (int)a,(int)XETrapClass,(int)XETrapType);
        return;
    }
    /* initialize the GetAvailable info reply here */
    XETrap_avail.hdr.type        = X_Reply;
    XETrap_avail.hdr.length      = XEXTRA(xXTrapGetAvailReply);
    XETrap_avail.data.xtrap_release  = XETrapRelease;
    XETrap_avail.data.xtrap_version  = XETrapVersion;
    XETrap_avail.data.xtrap_revision = XETrapRevision;
    XETrap_avail.data.pf_ident       = XETrapPlatform;
    XETrap_avail.data.max_pkt_size   = 0xFFFF;    /* very large number */
    for (i=0L; i<ASIZE(XETrap_avail.data.valid); i++)
    {
        XETrap_avail.data.valid[i] = 0L; /* Clear bits initially */
    }
    BitTrue(XETrap_avail.data.valid,XETrapTimestamp);
    BitTrue(XETrap_avail.data.valid,XETrapCmd);
    BitTrue(XETrap_avail.data.valid,XETrapCmdKeyMod);
    BitTrue(XETrap_avail.data.valid,XETrapRequest);
    BitTrue(XETrap_avail.data.valid,XETrapEvent);
    BitTrue(XETrap_avail.data.valid,XETrapMaxPacket);
    BitTrue(XETrap_avail.data.valid,XETrapStatistics);
    BitTrue(XETrap_avail.data.valid,XETrapWinXY);
    /* Not yet implemented */
    BitFalse(XETrap_avail.data.valid,XETrapCursor);
#ifndef _XINPUT
    BitFalse(XETrap_avail.data.valid,XETrapXInput);
#else
    BitTrue(XETrap_avail.data.valid,XETrapXInput);
#endif
#ifndef VECTORED_EVENTS
    BitFalse(XETrap_avail.data.valid,XETrapVectorEvents);
#else
    BitTrue(XETrap_avail.data.valid,XETrapVectorEvents);
#endif  /* VECTORED_EVENTS */
#ifndef COLOR_REPLIES
    BitFalse(XETrap_avail.data.valid,XETrapColorReplies);
#else
    BitTrue(XETrap_avail.data.valid,XETrapColorReplies);
#endif  /* COLOR_REPLIES */
    BitTrue(XETrap_avail.data.valid,XETrapGrabServer);
    /* initialize multi-client accelerator lists */
    io_clients.next = NULL;
    stats_clients.next = NULL;
    cmd_clients.next = NULL;
    for (i=0L; i<256L; i++)
    {
        vectored_requests[i] = 0L;
    }
    for (i=KeyPress; i<=MotionNotify; i++)
    {
        vectored_events[i] = 0L;
    }
    gate_closed = False;
    key_ignore  = False;
    next_key = XEKeyIsClear;
        
    XETrapPlatformSetup();
    /* Initialize any local memory we use */
    for (i=0L; i<ASIZE(EventProcVector); i++)
    {
        EventProcVector[i] = NULL;
#ifndef VECTORED_EVENTS
        XETrapEventProcVector[i] = NULL;
#else
        XETrapEventProcVector[i] = XETrapEventVector;
#endif
    }
    XETrapKbdDev = NULL;
    XETrapPtrDev = NULL;
    for (i=0L; i<ASIZE(XETrapProcVector); i++)
    {
        XETrapProcVector[i] = XETrapRequestVector;
    }
    for (i=128L; i<=255L; i++)
    {   /* Extension "swapped" requests are not implemented */
        XETSwProcVector[i] = NotImplemented;
    }
#ifdef VERBOSE
    ErrorF("%s:  Vers. %d.%d-%d successfully loaded\n", XTrapExtName,
        XETrap_avail.data.xtrap_release, 
        XETrap_avail.data.xtrap_version,
        XETrap_avail.data.xtrap_revision);
#endif

    return;
}

/*
 *  DESCRIPTION:
 *
 *      This procedure is called upon dispatch to allocate an
 *      environment structure for a new XTrap client.  The XETenv[]
 *      entry is allocated and initialized with default values.
 *      XETrapDestroyEnv() is responsible for deallocating this memory
 *      upon client termination.
 *
 *      Note: the status of this routine is returned to the caller of
 *      the Dispatch routine which will in turn SendErrorToClient if
 *      necessary.
 *
 */

int XETrapCreateEnv(ClientPtr client)
{
    XETrapEnv *penv = NULL;
    int status = Success;

    if (client->index >= MAXCLIENTS)
    {
        status = BadImplementation;
    }
    else if ((XETenv[client->index] = (XETrapEnv *)Xcalloc(sizeof(XETrapEnv)))
        == NULL)
    {
        status = BadAlloc;
    }
    if (status == Success)
    {
        penv = XETenv[client->index];
        penv->client = client;
        penv->protocol = 31;    /* default to backwards compatibility */
        /* prep for client's departure (for memory dealloc, cleanup) */
        AddResource(FakeClientID(client->index),XETrapType,
            (pointer)(long)(client->index));
        if (XETrapRedirectDevices() == False)
        {
            status = XETrapErrorBase + BadDevices;
        }
        /* Initialize the current state */
        if (status == Success)
        {
            status = XETrapReset(NULL, penv->client);
        }
    }

#ifdef VECTORED_EVENTS
    current_screen = -1L;       /* Invalidate current screen */
#endif

#ifdef VERBOSE
    if (status == Success)
    {
        ErrorF("%s:  Client '%d' Connection Accepted\n", XTrapExtName, 
            penv->client->index);
    }
#endif

    return(status);
}

/*
 *  DESCRIPTION:
 *
 *      This procedure is defined for the call to AddExtension()
 *      in which it is expected to be a parameter of the call.
 *
 *      This routine will be called by the server dispatcher
 *      when a client makes a request that is handled
 *      by the extension and the byte ordering of the client is the
 *      SAME as that of the extension.
 *
 *      Note: the status of the requests is returned to the caller of
 *      the Dispatch routine which will in turn SendErrorToClient if
 *      necessary.
 */

int XETrapDispatch(ClientPtr client)
{

    REQUEST(xXTrapReq);
    register int status = Success;

    REQUEST_AT_LEAST_SIZE(xXTrapReq);

    /* Have we seen this client before? */
    if (XETenv[client->index] == NULL)
    {
        status = XETrapCreateEnv(client);
    }
    /* Do we have a valid request? */
    if (status == Success)
    {
        if (stuff->minor_opcode < ASIZE(XETrapDispatchVector))
        {
            /* Then vector to the pointed to function */
            status = 
                (*(XETrapDispatchVector[stuff->minor_opcode]))(stuff,client);
        }
        else
        {
            status = BadRequest;
        }
    }
    return(status);
}

/*
 *  DESCRIPTION:
 *                     
 *        This procedure is defined for the call to AddExtension()
 *        in which it is expected to be a parameter of the call.
 *
 *        This routine would ordinarily be called by the server
 *        dispatcher when a client makes a request that is handled
 *        by the extension and the byte ordering of the client is
 *        DIFFERENT than that of the extension.
 */

int sXETrapDispatch(ClientPtr client)
{

    REQUEST(xXTrapReq);
    register int status = Success;

    REQUEST_AT_LEAST_SIZE(xXTrapReq);

    /* Have we seen this client before? */
    if (XETenv[client->index] == NULL)
    {
        status = XETrapCreateEnv(client);
    }
    /* Do we have a valid request? */
    if (status == Success)
    {
        if (stuff->minor_opcode < ASIZE(XETSwDispatchVector))
        {
            /* Then vector to the pointed to function */
            status = 
                (*(XETSwDispatchVector[stuff->minor_opcode]))(stuff,client);
        }
        else
        {
            status = BadRequest;
        }
    }
    return(status);
}

/*
 *  DESCRIPTION:
 *
 *      This routine will place the extension in a steady and known 
 *      state.  Any current state will be reset.  This is called either
 *      by a client request (dispatched) or when a new client environment
 *      is created.
 *
 */
int XETrapReset(xXTrapReq *request, ClientPtr client)
{
    static xXTrapConfigReq DummyReq;
    register int i;
    register int status = Success;
    XETrapEnv *penv = XETenv[client->index];

    /* in case any i/o's pending */
    (void)XETrapStopTrap((xXTrapReq *)NULL, client);
    penv->cur.hdr.type        = X_Reply;
    penv->cur.hdr.length      = XEXTRA(xXTrapGetCurReply);
    /* Fill in a dummy config request to clear all elements */
    for (i=0L; i<ASIZE(DummyReq.config_flags_valid); i++)
    {
        DummyReq.config_flags_valid[i]  = 0xFFL;  /* set all the valid flags */
        DummyReq.config_flags_data[i]   = 0L;     /* clear all data flags */
    }
    /* Don't reset grab server arbitrarily, it must be explicitly 
     * de-configured.
     */
    BitSet(DummyReq.config_flags_data, XETrapGrabServer, ignore_grabs);
    for (i=0L; i< ASIZE(DummyReq.config_flags_req); i++)
    {
       DummyReq.config_flags_req[i] = 0xFF; /* Clear all protocol requests */
    }
    for (i=0L; i< ASIZE(DummyReq.config_flags_event); i++)
    {
        DummyReq.config_flags_event[i] = 0xFF;  /* Clear all protocol events */
    }
    /* Call config routine to clear all configurable fields */
    status = XETrapConfig(&DummyReq, client);
    /* reset the environment */
    for (i=0L; i<ASIZE(penv->cur.data_state_flags); i++)
    {
        penv->cur.data_state_flags[i] = 0L; /* Clear all env flags */
    }
    penv->cur.data_config_max_pkt_size  = XETrap_avail.data.max_pkt_size;
   
    return(status);
}

/*
 *  DESCRIPTION:
 *
 *      This function sends a reply back to the requesting client indicating
 *      the available states of the extension can be configured for.
 */
int XETrapGetAvailable(xXTrapGetReq *request, ClientPtr client)
{
    XETrapEnv *penv = XETenv[client->index];
    update_protocol(request, client);
    /* Initialize the reply as needed */
    XETrap_avail.data.xtrap_protocol = penv->protocol;
    XETrap_avail.hdr.detail = XETrap_GetAvailable;
    XETrap_avail.hdr.sequenceNumber = client->sequence;
    WriteReplyToClient(client, sizeof(xXTrapGetAvailReply), &XETrap_avail);
    return(Success);
}

/*
 *  DESCRIPTION:
 *
 *      This function sends a reply back to the requesting client indicating
 *      the current state of the extension.
 */
int XETrapGetCurrent(xXTrapReq *request, ClientPtr client)
{
    XETrapEnv *penv = XETenv[client->index];
    int rep_size = (penv->protocol == 31 ? 284 : sz_xXTrapGetCurReply);
    penv->cur.hdr.length      = (rep_size - 32L) / SIZEOF(CARD32);

    /* Initialize the reply as needed */
    penv->cur.hdr.detail = XETrap_GetCurrent;
    penv->cur.hdr.sequenceNumber  = client->sequence;
    WriteReplyToClient(client, rep_size, &(penv->cur));

    return(Success);
}

/*
 *  DESCRIPTION:
 *
 *      This function sends a reply back to the requesting client dumping
 *      statistics (counts) of requests and events.  If stat's isn't
 *      configured, return failure.
 */
int XETrapGetStatistics(xXTrapReq *request, ClientPtr client)
{
    int status = Success;
    XETrapEnv *penv = XETenv[client->index];

    if ((BitIsTrue(penv->cur.data_config_flags_data, XETrapStatistics)) &&
        (penv->stats))
    {
        /* Initialize the reply as needed */
        int rep_size = sizeof(xXTrapGetStatsReply);
        penv->stats->detail = XETrap_GetStatistics;
        penv->stats->sequenceNumber = client->sequence;
        if (penv->protocol == 31)
        {
            xXTrapGetStatsReply  rep_stats;
            rep_stats = *penv->stats;
#ifndef VECTORED_EVENTS
            rep_size         = 1060;
#else
            rep_size         = 1544;
#endif
            rep_stats.length = (rep_size - 32L) / SIZEOF(CARD32);
            /* 
             * Now we need to shift the data *into* the header area 
             * for bug compatibility.
             */
            memcpy(&(rep_stats.pad0),&(penv->stats->data), 
                sizeof(XETrapGetStatsRep));
            WriteReplyToClient(client, rep_size, &rep_stats);
        }
        else
        {
            WriteReplyToClient(client, rep_size, penv->stats);
        }
    }
    else
    {
        status = XETrapErrorBase + BadStatistics;
    }
    return(status);
}

/*
 *  DESCRIPTION:
 *
 *      This function is dispatched when a client requests the extension to
 *      be configured in some manner.
 */
int XETrapConfig(xXTrapConfigReq *request, ClientPtr client)
{
    UByteP vflags       = request->config_flags_valid;
    UByteP dflags       = request->config_flags_data;
    UByteP req_flags    = request->config_flags_req;
    UByteP event_flags  = request->config_flags_event;
    XETrapEnv *penv     = XETenv[client->index];
    UByteP bit_flags    = penv->cur.data_config_flags_data;
    int status          = Success;
    CARD32 i            = 0L;

    /* Check events and swap if desired */
    if (BitIsTrue(vflags,XETrapEvent))
    {   /* Loop through all of the events */
        for (i=0L; i<ASIZE(EventProcVector); i++)
        {
            if (BitIsTrue(event_flags,i) &&    /* Do we care about this one? */
                (BitValue(dflags,XETrapEvent) ^           /* Exclusive Or */
                (BitValue(penv->cur.data_config_flags_event,i))))
            {   /* At this point we *know* there's a change.  The
                 * only question remaining is are there any more
                 * clients interested in this specific event.  If
                 * so, *don't* swap this process!
                 */
                if (BitIsTrue(dflags,XETrapEvent))
                {   /* Client wants the XTrap rtn */
                    if (++(vectored_events[i]) <= 1L)
                    {   /* first client, so do it */
                        _SwapProc(&(XETrapEventProcVector[i]), 
                            &(EventProcVector[i]));
                    }
                }
                else
                {   /* Client wants the *real* rtn */
                    if (--(vectored_events[i]) <= 0L)
                    {   /* No more clients using, so do it */
                        _SwapProc(&(XETrapEventProcVector[i]), 
                            &(EventProcVector[i]));
                    }
                }
                switch(i)
                {
                    case KeyPress:  /* needed for command key processing */
                    case KeyRelease:
                        XETrapKbdDev->processInputProc = 
                            (void_function)(EventProcVector[i] ? 
                            (void_function)EventProcVector[i] : 
                            (void_function)keybd_process_inp);
                        XETrapKbdDev->realInputProc = 
                            (void_function)(EventProcVector[i] ?
                            (void_function)EventProcVector[i] : 
                            (void_function)keybd_process_inp);
                        break;
#ifndef VECTORED_EVENTS
                    case ButtonPress: /* hack until events become vectored */
                    case ButtonRelease:
                    case MotionNotify:
                        XETrapPtrDev->processInputProc = 
                            (void_function)EventProcVector[i];
                        XETrapPtrDev->realInputProc = 
                            (void_function)EventProcVector[i];
                        break;
                    default:
                        status = BadImplementation;
                        break;
#endif /* !VECTORED_EVENTS */
                }
                BitToggle(penv->cur.data_config_flags_event,i);
            }
        }
    }
    if ((status == Success) && 
        (_CheckChangeBit(vflags,dflags,bit_flags,XETrapCmd)))
    {
        if (BitIsTrue(dflags, XETrapCmd))
        {   /* Add accelerator entry to cmd_clients list iff necessary */
            penv->cur.data_config_cmd_key = request->config_cmd_key;
            status = add_accelerator_node(penv->client, &cmd_clients);
        }
        else
        {
            penv->cur.data_config_cmd_key = 0L; /* default no KeyCode */
            remove_accelerator_node(penv->client, &cmd_clients);
        }
    }
    if ((status == Success) &&
        (_CheckChangeBit(vflags,dflags,bit_flags,XETrapMaxPacket)))
    {
        if (BitIsTrue(dflags,XETrapMaxPacket))
        {   /* Set size to what's passed in */
            if (request->config_max_pkt_size < XETrapMinPktSize)
            {   /* Tell them the value is too small */
                status = BadValue;
            }
            else
            {
                penv->cur.data_config_max_pkt_size = 
                    request->config_max_pkt_size;
            }
        }
        else
        {   /* Set it to the default (a *very* big number) */
            penv->cur.data_config_max_pkt_size = 0xFFFF;
        }
    }
    /* If the valid flag is set for requests, then each of the 
     * requests is swapped if it's different from current state.
     */
    if (BitIsTrue(vflags,XETrapRequest) && status == Success)
    {   /* Loop through all of the core requests */
        for (i=0L; i<ASIZE(XETrapProcVector); i++)
        {
            if (BitIsTrue(req_flags,i) &&     /* Do we care about this one? */
                (BitValue(dflags,XETrapRequest) ^          /* Exclusive Or */
                (BitValue(penv->cur.data_config_flags_req,i))))
            {   /* At this point we *know* there's a change.  The
                 * only question remaining is are there any more
                 * clients interested in this specific request.  If
                 * so, *don't* swap this process!
                 */
                if (BitIsTrue(dflags,XETrapRequest))
                {   /* Client wants the XTrap rtn */
                    if (++(vectored_requests[i]) <= 1L)
                    {   /* first client, so do it */
                        _SwapProc(&(XETrapProcVector[i]), (int_function *)&(ProcVector[i]));
                    }
                }
                else
                {   /* Client wants the *real* rtn */
                    if (--(vectored_requests[i]) <= 0L)
                    {   /* No more clients using, so do it */
                        _SwapProc(&(XETrapProcVector[i]), (int_function *)&(ProcVector[i]));
                    }
                }
                if (status == Success)
                {
                    BitToggle(penv->cur.data_config_flags_req,i);
                }
            }
        }
    }
    /* Check & Set the boolean flags */
    if (status == Success)
    {
        _CheckChangeBit(vflags,dflags,bit_flags,XETrapCmdKeyMod);
        _CheckChangeBit(vflags,dflags,bit_flags,XETrapTimestamp);
        _CheckChangeBit(vflags,dflags,bit_flags,XETrapWinXY);
/*        _CheckChangeBit(vflags,dflags,bit_flags,XETrapCursor); */
#ifdef COLOR_REPLIES
        _CheckChangeBit(vflags,dflags,bit_flags,XETrapColorReplies);
#endif /* COLOR_REPLIES */
        if (_CheckChangeBit(vflags,dflags,bit_flags,XETrapGrabServer))
        {   /* Let any client uncoditionally set/clear Grabs */
            ignore_grabs = BitValue(dflags, XETrapGrabServer);
        }
    }
    /* The statistics vflag/dflag mechanism is a little different
     * from most.  The dflag is initially set to 0 to indicate no
     * statistics.  When a config request comes in to request
     * statistics, memory's allocated and the dflag is set.
     * Thereafter, whenever a client wants to clear the counters, he
     * simply sets the vflag and clears the dflag.  Multiple requests
     * for statistics configuration are ignored, and the stats memory is
     * free'd only when the client disconnects.
     */
    if (status == Success)
    {
        if (_CheckChangeBit(vflags,dflags,bit_flags,XETrapStatistics))
        {
            if (BitIsTrue(dflags,XETrapStatistics))
            {   /* Do we need to allocate memory? */
                if (penv->stats == NULL && (penv->stats =
                    (xXTrapGetStatsReply *)Xcalloc(sizeof(xXTrapGetStatsReply)))
                    != NULL)
                {   /* Set up the reply header  */
                    penv->stats->type  = X_Reply;
                    penv->stats->length = XEXTRA(xXTrapGetStatsReply);
                    /* add accelerator node for stats clients list */
                    status = add_accelerator_node(penv->client, &stats_clients);
                }
                else if (penv->stats == NULL)
                {   /* No Memory! */
                    status = BadAlloc;
                }
            }
            else
            {   /* Zero out counters */
                (void)memset(penv->stats->data.requests, 0L, 
                    sizeof(penv->stats->data.requests));
                (void)memset(penv->stats->data.events, 0L,
                    sizeof(penv->stats->data.events));
                /* Re-cock the Stat's flag so that it'll
                 * sense a change for next zero'ing out
                 * of the counters.
                 */
                BitTrue(penv->cur.data_config_flags_data, XETrapStatistics);
            }
        }
    }    
    return(status);
}

/*
 *  DESCRIPTION:
 *
 *      This function sets the XETrapTrapActive bit to indicate that Trapping
 *      of requests and/or core events to the client may take place.
 *
 */
int XETrapStartTrap(xXTrapReq *request, ClientPtr client)
{
    XETrapEnv *penv = XETenv[client->index];
    int status = add_accelerator_node(penv->client, &io_clients);
    if (status == Success)
    {
        BitTrue(penv->cur.data_state_flags, XETrapTrapActive);
    }
    return(status);
}
/*
 *  DESCRIPTION:
 *
 *      This function clears the XETrapTrapActive bit to indicate that Trapping
 *      of requests and/or core events to the client may *not* take place.
 *
 */
int XETrapStopTrap(xXTrapReq *request, ClientPtr client)
{
    XETrapEnv *penv = XETenv[client->index];

    remove_accelerator_node(penv->client, &io_clients);
    BitFalse(penv->cur.data_state_flags, XETrapTrapActive);
    return(Success);
}

/*
 *  DESCRIPTION:
 *
 *      This function sends a reply back to the requesting client indicating
 *      the specific XTrap version of this extension.
 */
int XETrapGetVersion(xXTrapGetReq *request, ClientPtr client)
{
    xXTrapGetVersReply ver_rep;
    XETrapEnv *penv = XETenv[client->index];

    update_protocol(request,client);    /* to agree on protocol version */
    /* Initialize the reply as needed */
    ver_rep.hdr.type = X_Reply;
    ver_rep.hdr.detail = XETrap_GetVersion;
    ver_rep.hdr.sequenceNumber = client->sequence;
    ver_rep.hdr.length = XEXTRA(xXTrapGetVersReply);
    ver_rep.data.xtrap_release = XETrap_avail.data.xtrap_release;
    ver_rep.data.xtrap_version = XETrap_avail.data.xtrap_version;
    ver_rep.data.xtrap_revision = XETrap_avail.data.xtrap_revision;
    ver_rep.data.xtrap_protocol = penv->protocol; /* return agreed protocol */
    WriteReplyToClient(client, sizeof(xXTrapGetVersReply), &ver_rep);
    return(Success);
}

/*
 *  DESCRIPTION:
 *
 *      This function sends a reply back to the requesting client indicating
 *      the specific XTrap version of this extension.
 */
int XETrapGetLastInpTime(xXTrapReq *request, ClientPtr client)
{
    xXTrapGetLITimReply tim_rep;
    XETrapEnv *penv = XETenv[client->index];

    /* Initialize the reply as needed */
    tim_rep.hdr.type = X_Reply;
    tim_rep.hdr.detail = XETrap_GetLastInpTime;
    tim_rep.hdr.sequenceNumber = client->sequence;
    tim_rep.hdr.length = XEXTRA(xXTrapGetLITimReply);
    tim_rep.data_last_time = penv->last_input_time;
    WriteReplyToClient(client, sizeof(xXTrapGetLITimReply), &tim_rep);
    return(Success);
}

/*
 *  DESCRIPTION:
 *
 *  This routine is swapped in for the server's output request vectors.   
 *  After writing the request to one (or more) XTrap client(s), this
 *  routine ALWAYS returns by calling the REAL output request vector rtn.
 * 
 *  Note: Swapped Requests are handled automatically since the unswapped
 *        vectored routine is called after the request has been swapped.
 *        IOW, all requests are directed through ProcVector eventually and are
 *        "unswapped" at that point.  It is necessary to swap the data
 *        back if writing to a swapped client, however, and this is done
 *        by calling the appropriate XETSwProcVector[] routine.
 */
int XETrapRequestVector(ClientPtr client)
{
    int status = True;
    XETrapDatum *pdata, *spdata = NULL;
    REQUEST(xResourceReq);
    WindowPtr window_ptr;
    XETrapEnv *penv;
    BYTE *tptr;
    ClientList *ioc = &io_clients;
    ClientList *stc = &stats_clients;
    INT32 asize = sizeof(pdata->hdr) + stuff->length * sizeof(CARD32);
    INT32 size = MAX(asize,XETrapMinPktSize); /* Must be at least */
    INT32 csize;    /* size of request to send to the XTrap client */

    /* Get memory for the data to be sent */
    if ((pdata = (XETrapDatum *)Xcalloc(size)) == NULL)
    {   /* Can't do anything accept set a flag since we don't
         * know who to send the error to yet.
         */
        status = False;
    }

    while (ioc->next != NULL)
    {
        ioc = ioc->next;
        penv = XETenv[ioc->client->index];
        if (status == False)
        {   /* We didn't get the memory! Complain */
            SendErrorToClient(penv->client,XETrap_avail.data.major_opcode,
                stuff->reqType, 0L, BadAlloc);
            break;
        }
        if (BitIsTrue(penv->cur.data_config_flags_req,stuff->reqType))
        {   /* This particular client is interested in *this* request */
            pdata->hdr.client = client->index;  /* stuff client index in hdr */
            if (BitIsTrue(penv->cur.data_config_flags_data,XETrapWinXY))
            {
		if (Success != dixLookupDrawable(&window_ptr, stuff->id,
						 client, 0, DixUnknownAccess))
                {   /* Failed...invalidate the X and Y coordinate data. */
                    pdata->hdr.win_x = -1L;
                    pdata->hdr.win_y = -1L;
                }
                else
                {
                    pdata->hdr.screen = window_ptr->drawable.pScreen->myNum;
                    pdata->hdr.win_x = window_ptr->drawable.x;
                    pdata->hdr.win_y = window_ptr->drawable.y;
                }
            }
            if (BitIsTrue(penv->cur.data_config_flags_data,XETrapTimestamp))
            {
                pdata->hdr.timestamp = GetTimeInMillis();
            }
            /* Copy the information to a location we can write it from */
            (void) memcpy(&(pdata->u.req),stuff,stuff->length*sizeof(CARD32));
            pdata->hdr.count = MIN(penv->cur.data_config_max_pkt_size,asize);
            XETrapSetHeaderRequest(&(pdata->hdr));
    
            /* Perform any needed byte/word swapping. NOTE: This is not
             * the "normal" technique that should be used to perform the
             * swapping. The reason that we do it here is to be sure to
             * do it only once in a controlled manner, which we can not
             * guarentee in the case of the Xlib transport. Notice that
             * we don't swap the XTRAP EVENT information.  This is done
             * in the XETrapWriteXLib() routine.
             */

            if (penv->client->swapped)
            {   /* need to deal with swapped clients */
                if (spdata == NULL)
                {   /* Get memory for the swapped data to be sent */
                    if ((spdata = (XETrapDatum *)Xcalloc(size)) == NULL)
                    {
                        SendErrorToClient(penv->client,
                            XETrap_avail.data.major_opcode,
                            stuff->reqType, 0L, BadAlloc);
                        break;
                    }

                    memcpy(spdata,pdata,size);  /* fill in the info */
                    /* Now call the request-specific rtn to swap the request */
                    if (stuff->reqType < 128)
                    {   /* a core request, good */
                        (*XETSwProcVector[stuff->reqType])(&(spdata->u.req),
			    penv->client);	/* RTC X11R6 */
                    }
                    else if (penv->cur.data_config_max_pkt_size == 
                        XETrapMinPktSize)
                    {   /* Minimum size, so swap it as an ResourceReq */
                        XETSwResourceReq(&(spdata->u.req));
                    }
                    else
                    {   /* trying to swap an extension request! */
                        SendErrorToClient(penv->client,
                            XETrap_avail.data.major_opcode,
                            stuff->reqType, 0L, XETrapErrorBase + BadSwapReq);
                    }
                }
                /* need to stow in the latest header (count) */
                memcpy(spdata,pdata,SIZEOF(XETrapHeader));
                sXETrapHeader(&(spdata->hdr));  /* swap the XTrap Header */
            }
            /* Write as many bytes of information as the client wants */
            tptr = (BYTE *)(penv->client->swapped ? spdata : pdata);
            csize = MAX(pdata->hdr.count, XETrapMinPktSize);
            if (XETrapWriteXLib(penv, tptr, csize) != csize)
            {
                SendErrorToClient(penv->client,XETrap_avail.data.major_opcode,
                    stuff->reqType, 0L, XETrapErrorBase + BadIO);
            }
#ifdef COLOR_REPLIES
            /* Process Color Replies, if desired, and applicable */
            if (BitIsTrue(penv->cur.data_config_flags_data,XETrapColorReplies))
            {    /* wants color replies */
                switch(stuff->reqType)
                {
                    case X_AllocColor:
                        GetSendColorRep(client, stuff);
                        break;
                    case X_AllocNamedColor:
                        GetSendNamedColorRep(client, stuff);
                        break;
                    case X_AllocColorCells:
                        GetSendColorCellsRep(client, stuff);
                        break;
                    case X_AllocColorPlanes:
                        GetSendColorPlanesRep(client, stuff);
                        break;
                    default:
                        break;
                }
            }
#endif /* COLOR_REPLIES */
        }
    }
    while (stc->next != NULL)
    {   /* increment appropriate stats bucket for each interested client */
        stc = stc->next;
        penv = XETenv[stc->client->index];
        if (BitIsTrue(penv->cur.data_config_flags_req,stuff->reqType))
        {   /* This particular client would like this particular stat */
            penv->stats->data.requests[stuff->reqType]++;
        }
    }

    if (pdata)
    {
        Xfree(pdata);
    }
    if (spdata)
    {
        Xfree(spdata);
    }
    if (ignore_grabs == True &&
        (stuff->reqType == X_GrabServer || stuff->reqType == X_UngrabServer))
    {    /* doesn't want Grab's! Note: this is a "last configured" setting */
#ifndef NO_NEW_XTRAP
	int status;

	if (stuff->reqType == X_GrabServer)
	{
	    ClientList *pclient;

	    /* first call grab server procedure */
	    status = (*XETrapProcVector[stuff->reqType])(client);

	    /* then add XTrap controlling clients */
	    for (pclient = &io_clients; pclient; pclient = pclient->next)
		if (pclient->client)
		    MakeClientGrabImpervious(pclient->client);
	}
	else
	{
	    ClientList *pclient;

	    /* first drop XTrap controlling clients */
	    for (pclient = &io_clients; pclient; pclient = pclient->next)
		if (pclient->client)
		    MakeClientGrabPervious(pclient->client);

	    /* then call ungrab server procedure */
	    status = (*XETrapProcVector[stuff->reqType])(client);
	}
	return status;
#else /* NO_NEW_XTRAP */
        return(Success);
#endif /* NO_NEW_XTRAP */
    }
    else
    {
        return((*XETrapProcVector[stuff->reqType])(client));
    }
}
/*
 *
 *  DESCRIPTION:
 *
 *      This routine intercepts input xEvents from the keyboard.
 *      if XETrapTrapActive, will write record to client(s)
 *      and then pass the event to the server iff not command
 *      key and gate is open.  If it's a command key, then twiddle
 *      the gate state as required (optional, see below).
 *
 *      This routine implements an optional user specified command key
 *      that can be used to close the input pipe into the server
 *      while a client command is generated.  The keypress of the
 *      command key places this routine in command mode, the keyrelease
 *      exits command mode.  
 *
 *      A keypress of the command key followed by the
 *      optionally specified lock key will place this routine in continuous
 *      command mode until the command key and lock key are pressed again
 *      to exit command mode.  In the locked state, the client interprets
 *      keystrokes as it wishes, as commands or as input to a prior command.
 *
 *      Both mechanisms can be used alternately.
 *
 *  IMPLICIT INPUTS :
 *
 *      penv->cur.data_config_cmd_key :
 *                    This is the keycode of the key that is used to stop
 *                    and restart the transmission of intercepted input 
 *                    events to the server.  If specified, the gate_state
 *                    flag will be set or cleared depending on the state of
 *                    the command_key.
 *
 *      penv->cur.data_config_flags_data.XETrapCmdKeyMod:
 *                    This is the value of the mode in which the command_key
 *                    will operate.  It currently has two values: MODIFIER and
 *                    COMMAND_LOCK.  MODIFIER mode clears gate_state on
 *                    keypress, and sets gate_state on keyrelease.
 *                    COMMAND_LOCK mode toggles gate_state on
 *                    or off.
 *
 *      gate_closed:
 *                    A flag that is set/cleared in the xtrap_keyboard
 *                    routine that indicates whether intercepted input
 *                    should be passed to the server at any particular
 *                    instance.
 *
 *
 *      next_key:
 *                    This variable tracks the state of the next key to be
 *                    pressed or released.  It allows the checking of double
 *                    presses of the command key to be sent to the server and
 *                    keeps good state order when the command key is used.
 *
 *      key_ignore:
 *                   This variable indicates whether or not the specific
 *                   key should be ignored for subsequent server processing.
 *
 */
int XETrapKeyboard(xEvent *x_event, DevicePtr keybd, int count)
{                     
    register BYTE  type   = x_event->u.u.type;
    register BYTE  detail = x_event->u.u.detail;
    XETrapEnv *penv;
    ClientList *stc = &stats_clients;
    ClientList *cmc = &cmd_clients;
    int_function cur_func = XETrapKeyboard;

#ifdef VERBOSE
    if (count != 1L)
    {   /* We haven't coded for this situation yet! */
        ErrorF("Warning! Event count != 1 (%d)\n", count);
    }
#endif
    while (stc->next != NULL)
    {   /* increment appropriate stats bucket for each interested client */
        stc = stc->next;
        penv = XETenv[stc->client->index];
        if (BitIsTrue(penv->cur.data_config_flags_event,type))
        {   /* This particular client would like this particular stat */
            penv->stats->data.events[type]++;
        }
    }
#ifndef VECTORED_EVENTS
    /* We *only* StampAndMail command keys with vectored events since
     * we get much more data by waiting till we get called in XETrapEventVector
     */
    XETrapStampAndMail(x_event);  /* send to XTrap client if necessry */
#endif
    while (cmc->next != NULL)
    {
        cmc = cmc->next;
        penv = XETenv[cmc->client->index];
        key_ignore = False;
        if (detail == penv->cur.data_config_cmd_key)
        {
            if (BitIsTrue(penv->cur.data_config_flags_data, XETrapCmdKeyMod))
            {
                switch (type) 
                {
                    case KeyPress:
                        if (next_key == XEKeyIsEcho)
                        {
                            break;
                        }
                        gate_closed = True;
                        next_key = XEKeyIsClear;
                        break;
    
                    case KeyRelease:
                        if (next_key == XEKeyIsEcho)
                        {
                            next_key = XEKeyIsClear;
                            break;
                        }
                        if (next_key == XEKeyIsClear)
                        {
                            next_key = XEKeyIsEcho;
                        }
                        else
                        {   /* it's Other, so Clear it */
                            next_key = XEKeyIsClear;
                        }
                        gate_closed = False;
                        key_ignore = True;
                        break;
    
                    default: break;
                }
            }
            else
            {
                switch (type)
                {
                    case KeyPress:
                        if (next_key == XEKeyIsEcho)
                        {
                            gate_closed = False;
                            break;
                        }
                        /* Open gate on cmd key release */
                        if ((next_key == XEKeyIsOther) && 
                            gate_closed == True)
                        {
                            break;
                        }
                        gate_closed = True;
                        next_key = XEKeyIsClear;
                        break;
    
                    case KeyRelease:
                        if (next_key == XEKeyIsClear)
                        {
                            next_key = XEKeyIsEcho;
                            break;
                        }
    
                        if (next_key == XEKeyIsEcho)
                        {
                            next_key = XEKeyIsClear;
                            break;
                        }
    
                        gate_closed = False;
                        key_ignore = True;
                        next_key = XEKeyIsClear;
                        break;
    
                    default: 
                        break;
                }
            }
        }
        else
        {
            next_key = XEKeyIsOther;
        }
    }            

    /*
     *  If the gate to the server is open, 
     *  and we are not ignoring a keyrelease,
     *  pass the event to the server for normal processing.
     */
#ifndef VECTORED_EVENTS
    if ((gate_closed == False) && (key_ignore == False))
    {
        if (XETrapEventProcVector[type] != cur_func)
        {   /* to protect us from infinite loops */
            (void)(*XETrapEventProcVector[type])(x_event,keybd,count);
        }
        else
        {
            (void)(*EventProcVector[type])(x_event,keybd,count);
        }
    }
#else  /* VECTORED_EVENTS */
    if ((gate_closed == False) && (key_ignore == False))
    {   /* send event on to server to be trapped again in XETrapEventVector */
        (void)(*keybd_process_inp)(x_event,keybd,count);
    }
    else
    {
        XETrapStampAndMail(x_event);  /* send to XTrap client if necessry */
    }
#endif
    key_ignore = False; /* reset for next time around */
    return 0;
}

/*
 *  DESCRIPTION:
 *
 *      This routine intercepts input xEvents from the pointer device
 *      and passes the input event back to the server for normal processing.
 *
 *      This routine is sensitive to whether input is being passed
 *      up to the server or not.  This state is set by the keyboard
 *      input routine.
 *
 *
 */
#ifndef VECTORED_EVENTS
int XETrapPointer(xEvent *x_event, DevicePtr ptrdev, int count)
{
    XETrapEnv *penv;
    ClientList *stc = &stats_clients;
    int_function cur_func = XETrapPointer;

#ifdef VERBOSE
    if (count != 1L)
    {   /* We haven't coded for this situation yet! */
        ErrorF("Warning! Event count != 1 (%d)\n", count);
    }
#endif
    while (stc->next != NULL)
    {   /* increment appropriate stats bucket for each interested client */
        stc = stc->next;
        penv = XETenv[stc->client->index];
        if (BitIsTrue(penv->cur.data_config_flags_event,x_event->u.u.type))
        {   /* This particular client would like this particular stat */
            penv->stats->data.events[x_event->u.u.type]++;
        }
    }
    XETrapStampAndMail(x_event);  /* send to XTrap client if necessry */
    /*
     *  If the gate to the server is open,
     *  pass the event up like nothing has happened.
     */
    if (gate_closed == False)
    {
        if (XETrapEventProcVector[x_event->u.u.type] != cur_func)
        {   /* to protect us from infinite loops */
            (void)(*XETrapEventProcVector[x_event->u.u.type])(x_event,ptrdev,
                count);
        }
        else
        {
            (void)(*EventProcVector[x_event->u.u.type])(x_event,ptrdev,count);
        }
    }
    return 0;
}
#endif /* !VECTORED_EVENTS */


/*
 *  DESCRIPTION:
 *
 *      This routine determines whether it needs to send event data
 *      to the XTrap Client(s).  If so, it timestamps it appropriately
 *      and writes out both the header and detail information.
 *
 */
void XETrapStampAndMail(xEvent *x_event)
{
    XETrapDatum data;
    register CARD32 size;
    XETrapEnv *penv;
    ClientList *ioc = &io_clients;

    /* Currently, we're intercepting core events *before* most
     * of the event information's filled in.  Specifically, the
     * only fields that are valid at this level are: type, detail,
     * time, rootX, rootY, and state.
     */
    /* Loop through all clients wishing I/O */
    while (ioc->next != NULL)
    {
        ioc = ioc->next;
        penv = XETenv[ioc->client->index];
        /* Do we have a valid fd? Do we care about this event? */
        if (BitIsTrue(penv->cur.data_config_flags_event, x_event->u.u.type))
        {
            XETrapSetHeaderEvent(&(data.hdr));
            data.hdr.win_x = data.hdr.win_y = -1L; /* Invalidate req draw */
            data.hdr.screen = 0L;   /* not till Events are vectored! */
            data.hdr.client = 0L;   /* not till Events are vectored! */
            if (BitIsTrue(penv->cur.data_config_flags_data,
                XETrapTimestamp))
            {
                data.hdr.timestamp = GetTimeInMillis();
            }
            size = data.hdr.count = XETrapMinPktSize; /* Always for evts */
            penv->last_input_time = x_event->u.keyButtonPointer.time;
            /* Copy the event information into our local memory */
            (void)memcpy(&(data.u.event),x_event,sizeof(xEvent));

#ifdef PANORAMIX
	    if (!noPanoramiXExtension &&
                (data.u.event.u.u.type == MotionNotify ||
                data.u.event.u.u.type == ButtonPress ||
                data.u.event.u.u.type == ButtonRelease ||
                data.u.event.u.u.type == KeyPress ||
                data.u.event.u.u.type == KeyRelease)) {
		    int scr = XineramaGetCursorScreen();
		    data.u.event.u.keyButtonPointer.rootX +=
			panoramiXdataPtr[scr].x - panoramiXdataPtr[0].x;
		    data.u.event.u.keyButtonPointer.rootY +=
			panoramiXdataPtr[scr].y - panoramiXdataPtr[0].y;
	    }
#endif

            if (penv->client->swapped)
            {   /* 
                 * Notice that we don't swap the XTRAP EVENT information.  
                 * This is done in the XETrapWriteXLib() routine.
                 */
                xEvent ToEvent;
                (*EventSwapVector[data.u.event.u.u.type & 0177])
                    (&data.u.event,&ToEvent);
                (void)memcpy(&(data.u.event),&ToEvent,sizeof(ToEvent));
                sXETrapHeader(&(data.hdr));  /* swap the XTrap Header */
            }
            /* From this point on, the contents of data is swapped and
             * therefore we should not refer to it for information.
             */
            if (XETrapWriteXLib(penv, (BYTE *)&data, size) != size)
            {
                SendErrorToClient(penv->client,
                    XETrap_avail.data.major_opcode,
                    x_event->u.u.type, 0L, XETrapErrorBase + BadIO);
            }
        }
    }
    return;
}
#ifdef VECTORED_EVENTS
int XETrapEventVector(ClientPtr client, xEvent *x_event)
{
    XETrapDatum data;
    register CARD32 size;
    XETrapEnv *penv;
    ClientList *ioc = &io_clients;

    /* Loop through all clients wishing I/O */
    while (ioc->next != NULL)
    {
        ioc = ioc->next;
        penv = XETenv[ioc->client->index];
        /* Do we care about this event? */
        if (BitIsTrue(penv->cur.data_config_flags_event, x_event->u.u.type))
        {
            XETrapSetHeaderEvent(&(data.hdr));
            data.hdr.client = client->index;
            data.hdr.win_x = data.hdr.win_y = -1L; /* Invalidate req draw */
            if ((current_screen < 0L) || ((x_event->u.u.type >= KeyPress) && 
                (x_event->u.u.type <= MotionNotify) && 
                (!x_event->u.keyButtonPointer.sameScreen)))
            {   /* we've moved/warped to another screen */
                WindowPtr root_win = GetCurrentRootWindow();
                current_screen = root_win->drawable.pScreen->myNum;
            }
            data.hdr.screen = current_screen;
            if (BitIsTrue(penv->cur.data_config_flags_data,
                XETrapTimestamp))
            {
                data.hdr.timestamp = GetTimeInMillis();
            }
            size = data.hdr.count = XETrapMinPktSize; /* Always for evts */
            penv->last_input_time = x_event->u.keyButtonPointer.time;
            /* Copy the event information into our local memory */
            (void)memcpy(&(data.u.event),x_event,sizeof(xEvent));
    
            if (penv->client->swapped)
            {
                xEvent ToEvent;
                (*EventSwapVector[data.u.event.u.u.type & 0177])
                    (&data.u.event,&ToEvent);
                (void)memcpy(&(data.u.event),&ToEvent,sizeof(ToEvent));
                sXETrapHeader(&(data.hdr));  /* swap the XTrap Header */
            }
            /* From this point on, the contents of pdata is swapped and
             * therefore we should not refer to it for information.
             */
            if (XETrapWriteXLib(penv, (BYTE *)&data, size) != size)
            {
                SendErrorToClient(penv->client,
                    XETrap_avail.data.major_opcode,
                    x_event->u.u.type, 0L, XETrapErrorBase + BadIO);
            }
        }
    }
    return;
}
#endif /* VECTORED_EVENTS */
void sReplyXTrapDispatch(ClientPtr client, int size, char *reply)
{
    register XETrapRepHdr *rep = (XETrapRepHdr *)reply;

    switch(rep->detail)
    {   
        case XETrap_GetAvailable:
            {   
                xXTrapGetAvailReply lrep;
                (void)memcpy((char *)&lrep,reply,sizeof(lrep));
                sReplyXETrapGetAvail(client,size,(char *)&lrep);
            }
            break;
        case XETrap_GetCurrent:
            {   
                xXTrapGetCurReply lrep;
                (void)memcpy((char *)&lrep,reply,sizeof(lrep));
                sReplyXETrapGetCur(client,size,(char *)&lrep);
            }
            break;
        case XETrap_GetStatistics:
            {   
                xXTrapGetStatsReply lrep;
                (void)memcpy((char *)&lrep,reply,sizeof(lrep));
                sReplyXETrapGetStats(client,size,(char *)&lrep);
            }
            break;
        case XETrap_GetVersion:
            {   
                xXTrapGetVersReply lrep;
                (void)memcpy((char *)&lrep,reply,sizeof(lrep));
                sReplyXETrapGetVers(client,size,(char *)&lrep);
            }
            break;
        case XETrap_GetLastInpTime:
            {   
                xXTrapGetLITimReply lrep;
                (void)memcpy((char *)&lrep,reply,sizeof(lrep));
                sReplyXETrapGetLITim(client,size,(char *)&lrep);
            }
            break;
        default:
            SendErrorToClient(client,XETrap_avail.data.major_opcode,
                rep->detail, 0L, BadImplementation);
            break;
    }
    return;
}

/* 
 * XLib communications routines 
 */

/*
 *  DESCRIPTION:
 *
 *      This function performs the transport specific functions required
 *      for writing data back to an XTrap client over XLib.  The trick is
 *      packaging the data into <=32 byte packets to conform to the sizeof
 *      an X Event.  nbytes must be at least equal to XETrapMinPktSize
 *
 */
int XETrapWriteXLib(XETrapEnv *penv, BYTE *data, CARD32 nbytes)
{
    CARD32 size, total = 0L;
    xETrapDataEvent event;

    /* Initialize the detail field to show the beginning of a datum */
    event.detail = XETrapDataStart;
    event.idx = 0L;

    /* This loop could be optimized by not calling Write until after all
     * of the events are packaged. However, this would require memory
     * games, and may not therefore be a win.
     */
    while (nbytes > 0L)
    {   /* How many bytes can we send in this packet */
        size = (nbytes > sz_EventData) ? sz_EventData : nbytes;

        /* Initialize the event */
        event.type = XETrapData + XETrap_avail.data.event_base;
        event.sequenceNumber = penv->client->sequence;

        /* Copy the data we are sending */
        (void)memcpy(event.data,data,size);
        if (size < sz_EventData)
            (void)memset(event.data+size,0L,sz_EventData-size);
        data += size;
        nbytes -= size;
        total += size;

        /* Set the detail field to show the continuation of datum */
        if (total != size)
        {   /* this is not the first one */
            event.detail = (nbytes > 0) ? XETrapDataContinued : XETrapDataLast;
        }

        /* Send this part to the client */
        WriteEventsToClient(penv->client, 1L, (xEvent *) &event);
        event.idx++;      /* Bump the index for the next event */
    }
    return(total);
}

/*----------------------------*
 *  Static Functions
 *----------------------------*/

static void update_protocol(xXTrapGetReq *reqptr, ClientPtr client)
{
    XETrapEnv *penv = XETenv[client->index];
    /* update protocol number */
    switch (reqptr->protocol)
    {
        /* known acceptable protocols */
        case 31:
        case XETrapProtocol:
            penv->protocol = reqptr->protocol;
            break;
        /* all else */
        default:    /* stay backwards compatible */
            penv->protocol = 31;
            break;
    }
}

/* Swap 2 functions. This is a function instead of a macro to help to keep
 * lint from complaining about mixed types. It seems to work, but I would
 * probably classify this as a hack.
 */
static void _SwapProc( register int (**f1)(void), register int (**f2)(void))
{
    register int (*t1)(void) = *f1;
    *f1 = *f2;
    *f2 = t1;

    return;
}

/*
 *  DESCRIPTION:
 *
 *      This function swaps the byte order of fields within
 *      the XTrap Event Header.  It assumes the data will be
 *	swapped by code in XETrapRequestVector().
 *
 */
static void sXETrapEvent(xETrapDataEvent *from, xETrapDataEvent *to)
{
    to->type = from->type;
    to->detail = from->detail;
    cpswaps(from->sequenceNumber,to->sequenceNumber);
    cpswapl(from->idx,to->idx);
    /* Assumes that the data's already been swapped by XETrapRequestVector */
    memcpy(to->data, from->data, SIZEOF(EventData));
}

/*
 *  DESCRIPTION:
 *
 *      This function adds a node from an accelerator linked-list
 *      (either io_clients, stats_clients, or cmd_clients).
 *
 */
static int add_accelerator_node(ClientPtr client, ClientList *accel)
{
    Bool found = False;
    int status = Success;

    while (accel->next != NULL)
    {
        if (accel->client == client)
        {
            found = True;   /* Client's already known */
            break;
        }
        else
        {
            accel = accel->next;
        }
    }
    if (found == False)
    {
        if ((accel->next = (ClientList *)Xcalloc(sizeof(ClientList))) == NULL)
        {
            status = BadAlloc;
        }
        else
        {   /* fill in the node */
            accel = accel->next;
            accel->next   = NULL;
            accel->client = client;
        }
    }
    return(status);
}
/*
 *  DESCRIPTION:
 *
 *      This function removes a node from an accelerator linked-list
 *      (either io_clients, stats_clients, or cmd_clients).
 *
 */
static void remove_accelerator_node(ClientPtr client, ClientList *accel)
{
    while (accel->next != NULL)
    {
        if (accel->next->client == client)
        {
            ClientList *tmp = accel->next->next;
            Xfree(accel->next);
            accel->next = tmp;
            break;
        }
        else
        {
            accel = accel->next;
        }
    }

    return;
}

#ifdef COLOR_REPLIES
static void GetSendColorRep(ClientPtr client, xResourceReq *req)
{   /* adapted from ProcAllocColor() in dispatch.c */
    XETrapDatum data;
    int retval;
    XETrapEnv *penv = XETenv[client->index];
    xAllocColorReply *crep = (xAllocColorReply *)&(data.u.reply);
    xAllocColorReq   *creq = (xAllocColorReq *)req;
    ColormapPtr pmap = (ColormapPtr )LookupIDByType(creq->cmap, RT_COLORMAP);

    /* Fill in the header fields */
    data.hdr.count = XETrapMinPktSize; /* The color replies are 32 bytes */
    XETrapSetHeaderReply(&(data.hdr));
    /* Hack alert:
     * We need to pass the "reply" type in the header since replies don't
     * contain the id's themselves.  However, we're not changing the
     * protocol to support this until we decide exactly how we want to
     * do *all* replies (e.g. not just ColorReplies).  So until then, stow
     * the reply id in the screen field which wouldn't normally be used in
     * this context.
     */
    data.hdr.screen = req->reqType;
    if (!pmap)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadColor);
        return;
    }
    crep->red   = creq->red;
    crep->green = creq->green;
    crep->blue  = creq->blue;
    crep->pixel = 0;
    if ((retval = AllocColor(pmap, &(crep->red), &(crep->green),
        &(crep->blue), &(crep->pixel), client->index)) != Success)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, retval);
        return;
    }
    /* Swap data if necessary */
    if (client->swapped)
    {
        INT32 n;
        swaps(&(crep->red), n);
        swaps(&(crep->green), n);
        swaps(&(crep->blue), n);
        swapl(&(crep->pixel), n);
    }
    /* Send data to client */
    if (XETrapWriteXLib(penv, (BYTE *)&data, XETrapMinPktSize) 
        != XETrapMinPktSize)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, XETrapErrorBase + BadIO);
    }
}

static void GetSendNamedColorRep(ClientPtr client, xResourceReq *req)
{   /* adapted from ProcAllocNamedColor() in dispatch.c */
    XETrapDatum data;
    XETrapEnv *penv = XETenv[client->index];
    int retval;
    xAllocNamedColorReply *nrep = (xAllocNamedColorReply *)&(data.u.reply);
    xAllocNamedColorReq   *nreq = (xAllocNamedColorReq *)req;
    ColormapPtr pcmp = (ColormapPtr )LookupIDByType(nreq->cmap, RT_COLORMAP);

    data.hdr.count = XETrapMinPktSize; /* The color replies are 32 bytes */
    XETrapSetHeaderReply(&(data.hdr));
    /* Hack alert:
     * We need to pass the "reply" type in the header since replies don't
     * contain the id's themselves.  However, we're not changing the
     * protocol to support this until we decide exactly how we want to
     * do *all* replies (e.g. not just ColorReplies).  So until then, stow
     * the reply id in the screen field which wouldn't normally be used in
     * this context.
     */
    data.hdr.screen = req->reqType;
    if (!pcmp)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadColor);
        return;
    }
    if (!OsLookupColor(pcmp->pScreen->myNum, (char *)&nreq[1], 
        nreq->nbytes, &(nrep->exactRed), &(nrep->exactGreen), 
        &(nrep->exactBlue)))
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadName);
        return;
    }
    nrep->screenRed     = nrep->exactRed;
    nrep->screenGreen   = nrep->exactGreen;
    nrep->screenBlue    = nrep->exactBlue;
    nrep->pixel         = 0;
    if ((retval = AllocColor(pcmp, &(nrep->screenRed), 
        &(nrep->screenGreen), &(nrep->screenBlue), &(nrep->pixel), 
        client->index)) != Success)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, retval);
        return;
    }
    /* Swap data if necessary */
    if (client->swapped)
    {
        INT32 n;
        swapl(&(nrep->pixel), n);
        swaps(&(nrep->exactRed), n);
        swaps(&(nrep->exactGreen), n);
        swaps(&(nrep->exactBlue), n);
        swaps(&(nrep->screenRed), n);
        swaps(&(nrep->screenGreen), n);
        swaps(&(nrep->screenBlue), n);
    }

    /* Send data to client */
    if (XETrapWriteXLib(penv, (BYTE *)&data, XETrapMinPktSize) 
        != XETrapMinPktSize)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, XETrapErrorBase + BadIO);
    }
}

static void GetSendColorCellsRep(ClientPtr client, xResourceReq *req)
{   /* adapted from ProcAllocColorCells() in dispatch.c */
    int                   retval;
    int                   npixels, nmasks;
    unsigned long         *ppixels, *pmasks;
    long                  length;
    XETrapDatum           *data;
    XETrapEnv             *penv = XETenv[client->index];
    xAllocColorCellsReply *crep;
    xAllocColorCellsReq   *creq = (xAllocColorCellsReq *)req;
    ColormapPtr pmap = (ColormapPtr )LookupIDByType(creq->cmap, RT_COLORMAP);

    if (!pmap)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadColor);
        return;
    }
    npixels = creq->colors;
    if (!npixels)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadValue);
        return;
    }
    nmasks = creq->planes;
    length = ((long)npixels + (long)nmasks) * sizeof(Pixel);
    data = (XETrapDatum *)xalloc(sizeof(XETrapDatum)+length);
    if (!data)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadAlloc);
        return;
    }
    data->hdr.count = MIN(penv->cur.data_config_max_pkt_size,
        sizeof(XETrapDatum)+length);
    XETrapSetHeaderReply(&(data->hdr));
    data->hdr.screen = req->reqType;    /* hack! but necessary */
    ppixels = (unsigned long *)((char *)data + sizeof(XETrapDatum));
    pmasks = ppixels + npixels;
    if ((retval = AllocColorCells(client->index, pmap, npixels, 
        nmasks, (Bool)creq->contiguous, ppixels, pmasks)) != Success)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, retval);
        xfree(data);
        return;
    }
    crep = (xAllocColorCellsReply *)&(data->u.reply);
    crep->nPixels = npixels;
    crep->nMasks  = nmasks;
    /* Swap data if necessary */
    if (client->swapped)
    {
        INT32 n, i, *ptr;
        ptr=(INT32 *)ppixels;
        swaps(&(crep->nPixels), n);
        swaps(&(crep->nMasks), n);
        for (i=0; i<length; i++)
        {
            swapl(&(ptr[i]), n);
        }
    }
    /* Send data to client */
    if (XETrapWriteXLib(penv, (BYTE *)&data, data->hdr.count) 
        != data->hdr.count)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, XETrapErrorBase + BadIO);
    }
    xfree(data);
}
static void GetSendColorPlanesRep(ClientPtr client, xResourceReq *req)
{   /* adapted from ProcAllocColorPlanes() in dispatch.c */
    int                   retval;
    int                   npixels, nmasks;
    unsigned long         *ppixels, *pmasks;
    long                  length;
    XETrapDatum           *data;
    XETrapEnv             *penv = XETenv[client->index];
    xAllocColorPlanesReply *crep;
    xAllocColorPlanesReq   *creq = (xAllocColorPlanesReq *)req;
    ColormapPtr pmap = (ColormapPtr )LookupIDByType(creq->cmap, RT_COLORMAP);

    if (!pmap)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadColor);
        return;
    }
    npixels = creq->colors;
    if (!npixels)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadValue);
        return;
    }
    length = (long)npixels * sizeof(Pixel);
    data = (XETrapDatum *)xalloc(sizeof(XETrapDatum)+length);
    if (!data)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, BadAlloc);
        return;
    }
    data->hdr.count = MIN(penv->cur.data_config_max_pkt_size,
        sizeof(XETrapDatum)+length);
    XETrapSetHeaderReply(&(data->hdr));
    data->hdr.screen = req->reqType;    /* hack! but necessary */
    ppixels = (unsigned long *)((char *)data + sizeof(XETrapDatum));
    crep = (xAllocColorPlanesReply *)&(data->u.reply);
    if ((retval = AllocColorPlanes(client->index, pmap, npixels, 
        (int)creq->red, (int)creq->green, (int)creq->blue, 
        (int)creq->contiguous, ppixels, &(crep->redMask), &(crep->greenMask),
        &(crep->blueMask))) != Success)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, retval);
        xfree(data);
        return;
    }
    crep->nPixels = npixels;
    /* Swap data if necessary */
    if (client->swapped)
    {
        INT32 n, i, *ptr;
        ptr=(INT32 *)ppixels;
        swaps(&(crep->nPixels), n);
        swapl(&(crep->redMask), n);
        swapl(&(crep->greenMask), n);
        swapl(&(crep->blueMask), n);
        for (i=0; i<length; i++)
        {
            swapl(&(ptr[i]), n);
        }
    }
    /* Send data to client */
    if (XETrapWriteXLib(penv, (BYTE *)&data, data->hdr.count) 
        != data->hdr.count)
    {
        SendErrorToClient(penv->client, XETrap_avail.data.major_opcode,
            req->reqType, 0L, XETrapErrorBase + BadIO);
    }
    xfree(data);
}
#endif /* COLOR_REPLIES */
