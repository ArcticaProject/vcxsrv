/************************************************************

Copyright 2003-2005 Sun Microsystems, Inc.

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

************************************************************/

#define NEED_REPLIES
#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "colormapst.h"
#include "scrnintstr.h"
#include "servermd.h"
#define  _XEVIE_SERVER_
#include <X11/extensions/Xeviestr.h>
#include <X11/Xfuncproto.h>
#include "input.h"
#include "inputstr.h"
#include "windowstr.h"
#include "cursorstr.h"
#include <xkbsrv.h>

#include "../os/osdep.h"

#define NoSuchEvent 0x80000000

#ifdef XKB
extern Bool noXkbExtension;
#endif
extern int    xeviegrabState;

static DISPATCH_PROC(ProcXevieDispatch);
static DISPATCH_PROC(SProcXevieDispatch);

static void		XevieResetProc (ExtensionEntry *extEntry);

static unsigned char	XevieReqCode = 0;
static int		XevieErrorBase;

int			xevieFlag = 0;
int	 		xevieClientIndex = 0;
DeviceIntPtr		xeviekb = NULL;
DeviceIntPtr		xeviemouse = NULL;
Mask			xevieMask = 0;
int       		xevieEventSent = 0;
int			xevieKBEventSent = 0;
static DevPrivateKey    xevieDevicePrivateKey = &xevieDevicePrivateKey;
static Bool             xevieModifiersOn = FALSE;

#define XEVIEINFO(dev)  ((xevieDeviceInfoPtr) \
    dixLookupPrivate(&(dev)->devPrivates, xevieDevicePrivateKey))

Mask xevieFilters[128] = 
{
        NoSuchEvent,                   /* 0 */
        NoSuchEvent,                   /* 1 */
        KeyPressMask,                  /* KeyPress */
        KeyReleaseMask,                /* KeyRelease */
        ButtonPressMask,               /* ButtonPress */
        ButtonReleaseMask,             /* ButtonRelease */
        PointerMotionMask              /* MotionNotify (initial state) */
};

typedef struct {
    ProcessInputProc processInputProc;
    ProcessInputProc realInputProc;
    DeviceUnwrapProc unwrapProc;
} xevieDeviceInfoRec, *xevieDeviceInfoPtr;

typedef struct {
    CARD32 time;
    KeyClassPtr keyc;
} xevieKeycQueueRec, *xevieKeycQueuePtr;

#define KEYC_QUEUE_SIZE	    100
static xevieKeycQueueRec keycq[KEYC_QUEUE_SIZE] = {{0, NULL}};
static int keycqHead = 0, keycqTail = 0;

static Bool XevieStart(void);
static void XevieEnd(int clientIndex);
static void XevieClientStateCallback(CallbackListPtr *pcbl, pointer nulldata,
                                    pointer calldata);
static void XevieServerGrabStateCallback(CallbackListPtr *pcbl,
                                         pointer nulldata,
                                         pointer calldata);

static Bool XevieAdd(DeviceIntPtr device, pointer data);
static void XevieWrap(DeviceIntPtr device, ProcessInputProc proc);
static Bool XevieRemove(DeviceIntPtr device, pointer data);
static void doSendEvent(xEvent *xE, DeviceIntPtr device);
static void XeviePointerProcessInputProc(xEvent *xE, DeviceIntPtr dev,
                                         int count);
static void XevieKbdProcessInputProc(xEvent *xE, DeviceIntPtr dev, int count);

void
XevieExtensionInit (void)
{
    ExtensionEntry* extEntry;

    if (!AddCallback(&ServerGrabCallback,XevieServerGrabStateCallback,NULL))
       return;

    if ((extEntry = AddExtension (XEVIENAME,
				0,
				XevieNumberErrors,
				ProcXevieDispatch,
				SProcXevieDispatch,
				XevieResetProc,
				StandardMinorOpcode))) {
	XevieReqCode = (unsigned char)extEntry->base;
	XevieErrorBase = extEntry->errorBase;
    }
}

/*ARGSUSED*/
static 
void XevieResetProc (ExtensionEntry *extEntry)
{
}

static 
int ProcXevieQueryVersion (register ClientPtr client)
{
    xXevieQueryVersionReply rep;
    int n;

    REQUEST_SIZE_MATCH (xXevieQueryVersionReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequence_number = client->sequence;
    rep.server_major_version = XEVIE_MAJOR_VERSION;
    rep.server_minor_version = XEVIE_MINOR_VERSION;
    if (client->swapped) {
	swaps(&rep.sequence_number, n);
	swapl(&rep.length, n);
	swaps(&rep.server_major_version, n);
	swaps(&rep.server_minor_version, n);
    }
    WriteToClient (client, sizeof (xXevieQueryVersionReply), (char *)&rep);
    return client->noClientException;
}

static
int ProcXevieStart (register ClientPtr client)
{
    xXevieStartReply rep;
    int n;

    REQUEST_SIZE_MATCH (xXevieStartReq);
    rep.pad1 = 0;

    if(!xevieFlag){
        if (AddCallback(&ClientStateCallback,XevieClientStateCallback,NULL)) {
           xevieFlag = 1;
           rep.pad1 = 1;
           xevieClientIndex = client->index;
	   if(!keycq[0].time ) {
		int i;
		for(i=0; i<KEYC_QUEUE_SIZE; i++) {
		    keycq[i].keyc = xalloc(sizeof(KeyClassRec));	
		    keycq[i].keyc->xkbInfo = xalloc(sizeof(XkbSrvInfoRec));
		}
	   }
        } else
           return BadAlloc;
    } else
        return BadAccess;
#ifdef XKB
    if (!noXkbExtension) {
	if (!XevieStart()) {
            DeleteCallback(&ClientStateCallback,XevieClientStateCallback,NULL);
            return BadAlloc;
        }
    }
#endif
    
    xevieModifiersOn = FALSE;

    rep.length = 0;
    rep.type = X_Reply;
    rep.sequence_number = client->sequence;
    if (client->swapped) {
	swaps(&rep.sequence_number, n);
	swapl(&rep.length, n);
    }
    WriteToClient (client, sizeof (xXevieStartReply), (char *)&rep);
    return client->noClientException;
}

static
int ProcXevieEnd (register ClientPtr client)
{
    xXevieEndReply rep;
    int n;

    REQUEST_SIZE_MATCH (xXevieEndReq);
    
    if (xevieFlag) {
        if (client->index != xevieClientIndex)
            return BadAccess;

        DeleteCallback(&ClientStateCallback,XevieClientStateCallback,NULL);
        XevieEnd(xevieClientIndex);
    }

    rep.length = 0;
    rep.type = X_Reply;
    rep.sequence_number = client->sequence;
    if (client->swapped) {
	swaps(&rep.sequence_number, n);
	swapl(&rep.length, n);
    }
    WriteToClient (client, sizeof (xXevieEndReply), (char *)&rep);
    return client->noClientException;
}

static
int ProcXevieSend (register ClientPtr client)
{
    REQUEST (xXevieSendReq);
    xXevieSendReply rep;
    xEvent *xE;
    static unsigned char lastDetail = 0, lastType = 0;
    int n;

    REQUEST_SIZE_MATCH (xXevieSendReq);
    
    if (client->index != xevieClientIndex)
        return BadAccess;

    xE = (xEvent *)&stuff->event;
    rep.length = 0;
    rep.type = X_Reply;
    rep.sequence_number = client->sequence;
    if (client->swapped) {
	swaps(&rep.sequence_number, n);
	swapl(&rep.length, n);
    }
    WriteToClient (client, sizeof (xXevieSendReply), (char *)&rep);

    switch(xE->u.u.type) {
	case KeyPress:
        case KeyRelease:
	  xevieKBEventSent = 1;
#ifdef XKB
          if(!noXkbExtension)
	    doSendEvent(xE, inputInfo.keyboard);
	  else 
#endif
            CoreProcessKeyboardEvent (xE, xeviekb, 1);
	  break;
	case ButtonPress:
	case ButtonRelease:
	case MotionNotify:
	  xevieEventSent = 1;
#ifdef XKB
	  if(!noXkbExtension)
	    doSendEvent(xE, inputInfo.pointer);
	  else
#endif
	    CoreProcessPointerEvent(xE, xeviemouse, 1); 
	  break; 
	default:
	  break;
    }
    lastType = xE->u.u.type;
    lastDetail = xE->u.u.detail;
    return client->noClientException;
}

static
int ProcXevieSelectInput (register ClientPtr client)
{
    REQUEST (xXevieSelectInputReq);
    xXevieSelectInputReply rep;
    int n;

    REQUEST_SIZE_MATCH (xXevieSelectInputReq);

    if (client->index != xevieClientIndex)
        return BadAccess;

    xevieMask = stuff->event_mask;
    rep.length = 0;
    rep.type = X_Reply;
    rep.sequence_number = client->sequence;
    if (client->swapped) {
	swaps(&rep.sequence_number, n);
	swapl(&rep.length, n);
    }
    WriteToClient (client, sizeof (xXevieSelectInputReply), (char *)&rep);
    return client->noClientException;
}

static 
int ProcXevieDispatch (register ClientPtr client)
{
    REQUEST (xReq);
    switch (stuff->data)
    {
    case X_XevieQueryVersion:
	return ProcXevieQueryVersion (client);
    case X_XevieStart:
	return ProcXevieStart (client);
    case X_XevieEnd:
	return ProcXevieEnd (client);
    case X_XevieSend:
	return ProcXevieSend (client);
    case X_XevieSelectInput:
	return ProcXevieSelectInput(client);
    default:
	return BadRequest;
    }
}

static 
int SProcXevieQueryVersion (register ClientPtr client)
{
    register int n;

    REQUEST(xXevieQueryVersionReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXevieQueryVersionReq);
    swaps (&stuff->client_major_version, n);
    swaps (&stuff->client_minor_version, n);
    return ProcXevieQueryVersion(client);
}

static 
int SProcXevieStart (ClientPtr client)
{
    register int n;

    REQUEST (xXevieStartReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXevieStartReq);
    swapl (&stuff->screen, n);
    return ProcXevieStart (client);
}

static 
int SProcXevieEnd (ClientPtr client)
{
    register int n;

    REQUEST (xXevieEndReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXevieEndReq);
    swapl (&stuff->cmap, n);
    return ProcXevieEnd (client);
}

static
int SProcXevieSend (ClientPtr client)
{
    register int n;
    xEvent eventT;
    EventSwapPtr proc;

    REQUEST (xXevieSendReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXevieSendReq);
    swapl (&stuff->dataType, n);

    /* Swap event */
    proc = EventSwapVector[stuff->event.u.u.type & 0177];
    if (!proc ||  proc == NotImplemented) /* no swapping proc; invalid event type? */
	return (BadValue);
    (*proc)(&stuff->event, &eventT);
    stuff->event = eventT;
    
    return ProcXevieSend (client);
}

static
int SProcXevieSelectInput (ClientPtr client)
{
    register int n;

    REQUEST (xXevieSelectInputReq);
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXevieSelectInputReq);
    swapl (&stuff->event_mask, n);
    return ProcXevieSelectInput (client);
}


static 
int SProcXevieDispatch (register ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_XevieQueryVersion:
	return SProcXevieQueryVersion (client);
    case X_XevieStart:
	return SProcXevieStart (client);
    case X_XevieEnd:
	return SProcXevieEnd (client);
    case X_XevieSend:
	return SProcXevieSend (client);
    case X_XevieSelectInput:
	return SProcXevieSelectInput(client);
    default:
	return BadRequest;
    }
}
/*======================================================*/

#define WRAP_INPUTPROC(dev,store,inputProc) \
   store->processInputProc = dev->public.processInputProc; \
   dev->public.processInputProc = inputProc; \
   store->realInputProc = dev->public.realInputProc; \
   dev->public.realInputProc = inputProc;

#define COND_WRAP_INPUTPROC(dev,store,inputProc) \
   if (dev->public.processInputProc == dev->public.realInputProc) \
          dev->public.processInputProc = inputProc; \
   store->processInputProc =  \
   store->realInputProc = dev->public.realInputProc; \
   dev->public.realInputProc = inputProc;

#define UNWRAP_INPUTPROC(dev,restore) \
   dev->public.processInputProc = restore->processInputProc; \
   dev->public.realInputProc = restore->realInputProc;

#define UNWRAP_INPUTPROC(dev,restore) \
   dev->public.processInputProc = restore->processInputProc; \
   dev->public.realInputProc = restore->realInputProc;

#define XEVIE_EVENT(xE) \
      (xevieFlag \
       && !xeviegrabState \
       && clients[xevieClientIndex] \
       && (xevieMask & xevieFilters[xE->u.u.type]))


static void
sendEvent(ClientPtr pClient, xEvent *xE)
{
    if(pClient->swapped) {
        xEvent    eventTo;

        /* Remember to strip off the leading bit of type in case
           this event was sent with "SendEvent." */
        (*EventSwapVector[xE->u.u.type & 0177]) (xE, &eventTo);
        (void)WriteToClient(pClient, sizeof(xEvent), (char *)&eventTo);
    } else {
        (void)WriteToClient(pClient, sizeof(xEvent), (char *) xE);
    }
}

static void
XevieKbdProcessInputProc(xEvent *xE, DeviceIntPtr dev, int count)
{
    int             key, bit;
    BYTE   *kptr;
    ProcessInputProc tmp;
    KeyClassPtr keyc = dev->key;
    xevieDeviceInfoPtr xeviep = XEVIEINFO(dev);

    if(XEVIE_EVENT(xE)) {
        key = xE->u.u.detail;
        kptr = &keyc->down[key >> 3];
        bit = 1 << (key & 7);

	if (dev->key->modifierMap[xE->u.u.detail])
            xevieModifiersOn = TRUE;

        xE->u.keyButtonPointer.event = xeviewin->drawable.id;
        xE->u.keyButtonPointer.root = GetCurrentRootWindow()->drawable.id;
        xE->u.keyButtonPointer.child = (xeviewin->firstChild)
            ? xeviewin->firstChild->drawable.id:0;
        xE->u.keyButtonPointer.rootX = xeviehot.x;
        xE->u.keyButtonPointer.rootY = xeviehot.y;
        xE->u.keyButtonPointer.state = keyc->state | inputInfo.pointer->button->state;
        /* fix bug: sequence lost in Xlib */
        xE->u.u.sequenceNumber = clients[xevieClientIndex]->sequence;
#ifdef XKB
	/* fix for bug5092586 */
	if(!noXkbExtension) {
          switch(xE->u.u.type) {
	    case KeyPress: *kptr |= bit; break;
	    case KeyRelease: *kptr &= ~bit; break;
	  }
	}
#endif
	keycq[keycqHead].time = xE->u.keyButtonPointer.time;
	memcpy(keycq[keycqHead].keyc, keyc, sizeof(KeyClassRec) - sizeof(KeyClassPtr));
	memcpy(keycq[keycqHead].keyc->xkbInfo, keyc->xkbInfo, sizeof(XkbSrvInfoRec));
	if(++keycqHead >=KEYC_QUEUE_SIZE)
	    keycqHead = 0;
        sendEvent(clients[xevieClientIndex], xE);
        return;
    }

    tmp = dev->public.realInputProc;
    UNWRAP_INPUTPROC(dev,xeviep);
    dev->public.processInputProc(xE,dev,count);
    COND_WRAP_INPUTPROC(dev,xeviep,tmp);
}

static void
XeviePointerProcessInputProc(xEvent *xE, DeviceIntPtr dev, int count)
{
    xevieDeviceInfoPtr xeviep = XEVIEINFO(dev);
    ProcessInputProc tmp;

    if (XEVIE_EVENT(xE)) {
        /* fix bug: sequence lost in Xlib */
        xE->u.u.sequenceNumber = clients[xevieClientIndex]->sequence;
        sendEvent(clients[xevieClientIndex], xE);
        return;
    }

    tmp = dev->public.realInputProc;
    UNWRAP_INPUTPROC(dev,xeviep);
    dev->public.processInputProc(xE,dev,count);
    COND_WRAP_INPUTPROC(dev,xeviep,tmp);
}

static Bool
XevieStart(void)
{
    ProcessInputProc prp;
    prp = XevieKbdProcessInputProc;
    if (!XevieAdd(inputInfo.keyboard,&prp))
        return FALSE;
    prp = XeviePointerProcessInputProc;
    if (!XevieAdd(inputInfo.pointer,&prp))
        return FALSE;

    return TRUE;
}


static void
XevieEnd(int clientIndex)
{
    if (!clientIndex || clientIndex == xevieClientIndex) {

#ifdef XKB
       if(!noXkbExtension) {

	   XevieRemove(inputInfo.keyboard,NULL);

	   inputInfo.keyboard->public.processInputProc = CoreProcessKeyboardEvent;
           inputInfo.keyboard->public.realInputProc = CoreProcessKeyboardEvent;
           XkbSetExtension(inputInfo.keyboard,ProcessKeyboardEvent);


           XevieRemove(inputInfo.pointer,NULL);

	   inputInfo.pointer->public.processInputProc = CoreProcessPointerEvent;
           inputInfo.pointer->public.realInputProc = CoreProcessPointerEvent;
           XkbSetExtension(inputInfo.pointer,ProcessPointerEvent);
       }
#endif

       xevieFlag = 0;
       xevieClientIndex = 0;
       DeleteCallback (&ClientStateCallback, XevieClientStateCallback, NULL);
    }
}

static void
XevieClientStateCallback(CallbackListPtr *pcbl, pointer nulldata,
                        pointer calldata)
{
    NewClientInfoRec *pci = (NewClientInfoRec *)calldata;
    ClientPtr client = pci->client;
    if (client->clientState == ClientStateGone
       || client->clientState == ClientStateRetained)
       XevieEnd(client->index);
}

static void
XevieServerGrabStateCallback(CallbackListPtr *pcbl, pointer nulldata,
                            pointer calldata)
{
    ServerGrabInfoRec *grbinfo = (ServerGrabInfoRec *)calldata;
    if (grbinfo->grabstate == SERVER_GRABBED)
       xeviegrabState = TRUE;
    else
       xeviegrabState = FALSE;
}

#define UNWRAP_UNWRAPPROC(device,proc_store) \
    device->unwrapProc = proc_store;

#define WRAP_UNWRAPPROC(device,proc_store,proc) \
    proc_store = device->unwrapProc; \
    device->unwrapProc = proc;

static void
xevieUnwrapProc(DeviceIntPtr device, DeviceHandleProc proc, pointer data)
{
    xevieDeviceInfoPtr xeviep = XEVIEINFO(device);
    ProcessInputProc tmp = device->public.processInputProc;

    UNWRAP_INPUTPROC(device,xeviep);
    UNWRAP_UNWRAPPROC(device,xeviep->unwrapProc);
    proc(device,data);
    WRAP_INPUTPROC(device,xeviep,tmp);
    WRAP_UNWRAPPROC(device,xeviep->unwrapProc,xevieUnwrapProc);
}

static Bool
XevieUnwrapAdd(DeviceIntPtr device, void* data)
{
    if (device->unwrapProc)
        device->unwrapProc(device,XevieUnwrapAdd,data);
    else {
        ProcessInputProc *ptr = data;
        XevieWrap(device,*ptr);
    }

    return TRUE;
}

static Bool
XevieAdd(DeviceIntPtr device, void* data)
{
    xevieDeviceInfoPtr xeviep;

    xeviep = xalloc (sizeof (xevieDeviceInfoRec));
    if (!xeviep)
            return FALSE;

    dixSetPrivate(&device->devPrivates, xevieDevicePrivateKey, xeviep);
    XevieUnwrapAdd(device, data);

    return TRUE;
}

static Bool
XevieRemove(DeviceIntPtr device,pointer data)
{
    xevieDeviceInfoPtr xeviep = XEVIEINFO(device);

    if (!xeviep) return TRUE;

    UNWRAP_INPUTPROC(device,xeviep);
    UNWRAP_UNWRAPPROC(device,xeviep->unwrapProc);

    xfree(xeviep);
    dixSetPrivate(&device->devPrivates, xevieDevicePrivateKey, NULL);
    return TRUE;
}

static void
XevieWrap(DeviceIntPtr device, ProcessInputProc proc)
{
    xevieDeviceInfoPtr xeviep = XEVIEINFO(device);

    WRAP_INPUTPROC(device,xeviep,proc);
    WRAP_UNWRAPPROC(device,xeviep->unwrapProc,xevieUnwrapProc);
}

static void
doSendEvent(xEvent *xE, DeviceIntPtr dev)
{
    xevieDeviceInfoPtr xeviep = XEVIEINFO(dev);
    ProcessInputProc tmp = dev->public.realInputProc;
    if (((xE->u.u.type==KeyPress)||(xE->u.u.type==KeyRelease))
        && !xevieModifiersOn) {
	KeyClassPtr keyc =  dev->key;
        CARD8 realModes = dev->key->modifierMap[xE->u.u.detail];
	int notFound = 0;
	/* if some events are consumed by client, move the queue tail pointer to the current 
           event which just comes back from Xevie client . 
	*/
        if(keycq[keycqTail].time != xE->u.keyButtonPointer.time) {
	    while(keycq[keycqTail].time != xE->u.keyButtonPointer.time) {
		if(++keycqTail >= KEYC_QUEUE_SIZE)
		    keycqTail = 0;
		if(keycqTail == keycqHead) {
		    notFound = 1;
		    break;
		}
	    }
	}
	if(!notFound) {
	    dev->key = keycq[keycqTail].keyc;
	    if(++keycqTail >= KEYC_QUEUE_SIZE)
	        keycqTail = 0;
	}
        dev->key->modifierMap[xE->u.u.detail] = 0;  

	if(dev->key->xkbInfo->repeatKey != 0 && xE->u.u.type != KeyPress)
            XkbLastRepeatEvent=     (pointer)xE;
        UNWRAP_INPUTPROC(dev,xeviep);
        dev->public.processInputProc(xE,dev,1);
        COND_WRAP_INPUTPROC(dev,xeviep,tmp);
        XkbLastRepeatEvent= NULL;

        dev->key->modifierMap[xE->u.u.detail] = realModes;
	dev->key = keyc;
	if(notFound) {
	    DeleteCallback(&ClientStateCallback,XevieClientStateCallback,NULL);
            XevieEnd(xevieClientIndex);
	    ErrorF("Error: Xevie keyc queue size is not enough, disable Xevie\n");
	}	
    } else {
        UNWRAP_INPUTPROC(dev,xeviep);
        dev->public.processInputProc(xE,dev,1);
        COND_WRAP_INPUTPROC(dev,xeviep,tmp);
    }
}

