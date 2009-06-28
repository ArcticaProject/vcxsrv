/****************************************************************************
Copyright 1987, 1988, 1989, 1990, 1991, 1992 by 

	Digital Equipment Corp., Maynard, MA

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
 *      This module is the device independent module responsible for all
 *      routines required for proper communication in a heterogeneous
 *      networking environment (i.e. client & server on different endian
 *      machines).  The bulk of this module is patterned after X11/R4's
 *      server/dix/swapreq.c ; however, they infact swap fields
 *      in the exact opposite order since XTrap requires "unswapped" data
 *      to become "swapped" before sending it to a "swapped" client.
 *
 *  CONTRIBUTORS:
 *
 *      Ken Miller
 *      Marc Evans
 *
 * CHANGES:
 *
 *	Robert Chesler - added client arg for X11R6 port in many spots
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#define NEED_REPLIES
#define NEED_EVENTS
#include <X11/Xproto.h>
#include <X11/Xprotostr.h>
#include <X11/extensions/xtrapdi.h>
#include "input.h"          /* Server DevicePtr definitions */
#include "misc.h"
#include "dixstruct.h"
#ifdef PC
# include "extnsist.h"
#else
# include "extnsionst.h"        /* Server ExtensionEntry definitions */
#endif
# include "swapreq.h"        /* Server SwapColorItem definition */
#include <X11/extensions/xtrapddmi.h>
#include <X11/extensions/xtrapproto.h>

/* In-coming XTrap requests needing to be swapped to native format */

int sXETrapReset(xXTrapReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapReset(request,client));
}

int sXETrapGetAvailable(xXTrapGetReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    swaps(&(request->protocol),n);
    return(XETrapGetAvailable(request,client));
}

int sXETrapConfig(xXTrapConfigReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    swaps(&(request->config_max_pkt_size),n);
    return(XETrapConfig(request,client));
}

int sXETrapStartTrap(xXTrapReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapStartTrap(request,client));
}

int sXETrapStopTrap(xXTrapReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapStopTrap(request,client));
}

int sXETrapGetCurrent(xXTrapReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapGetCurrent(request,client));
}

int sXETrapGetStatistics(xXTrapReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapGetStatistics(request,client));
}

#ifndef _XINPUT
int sXETrapSimulateXEvent(xXTrapInputReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->input.x),n);
    swaps(&(request->input.y),n);
    return(XETrapSimulateXEvent(request,client));
}
#endif

int sXETrapGetVersion(xXTrapGetReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    swaps(&(request->protocol),n);
    return(XETrapGetVersion(request,client));
}

int sXETrapGetLastInpTime(xXTrapReq *request, ClientPtr client)
{
    register char n;
    swaps(&(request->length),n);
    return(XETrapGetLastInpTime(request,client));
}


/* Out-going XTrap replies needing to be swapped *from* native format */

void sReplyXETrapGetAvail(ClientPtr client, int size, char *reply)
{
    xXTrapGetAvailReply *rep = (xXTrapGetAvailReply *)reply;
    register char n;
    swaps(&(rep->hdr.sequenceNumber),n);
    swapl(&(rep->hdr.length),n);
    swapl(&(rep->data.pf_ident),n);
    swaps(&(rep->data.xtrap_release),n);
    swaps(&(rep->data.xtrap_version),n);
    swaps(&(rep->data.xtrap_revision),n);
    swaps(&(rep->data.max_pkt_size),n);
    swapl(&(rep->data.major_opcode),n);
    swapl(&(rep->data.event_base),n);
    swaps(&(rep->data.cur_x),n);
    swaps(&(rep->data.cur_y),n);
    (void)WriteToClient(client,size,reply);
    return;
}
void sReplyXETrapGetVers(ClientPtr client, int size, char *reply)
{
    xXTrapGetVersReply *rep = (xXTrapGetVersReply *)reply;
    register char n;
    swaps(&(rep->hdr.sequenceNumber),n);
    swapl(&(rep->hdr.length),n);
    swaps(&(rep->data.xtrap_release),n);
    swaps(&(rep->data.xtrap_version),n);
    swaps(&(rep->data.xtrap_revision),n);
    (void)WriteToClient(client,size,reply);
    return;
}
void sReplyXETrapGetLITim(ClientPtr client, int size, char *reply)
{
    xXTrapGetLITimReply *rep = (xXTrapGetLITimReply *)reply;
    register char n;
    swaps(&(rep->hdr.sequenceNumber),n);
    swapl(&(rep->hdr.length),n);
    swapl(&(rep->data_last_time),n);
    (void)WriteToClient(client,size,reply);
    return;
}
void sReplyXETrapGetCur(ClientPtr client, int size, char *reply)
{
    xXTrapGetCurReply *rep = (xXTrapGetCurReply *)reply;
    register char n;
    swaps(&(rep->hdr.sequenceNumber),n);
    swapl(&(rep->hdr.length),n);
    swaps(&(rep->data_config_max_pkt_size),n);
    (void)WriteToClient(client,size,reply);
    return;
}
void sReplyXETrapGetStats(ClientPtr client, int size, char *reply)
{
    xXTrapGetStatsReply *rep = (xXTrapGetStatsReply *)reply;
    register char n;
    register int i;
    long *p;

    swaps(&(rep->sequenceNumber),n);
    swapl(&(rep->length),n);
    for (i=0L, p = (long *)rep->data.requests; i<256L; i++, p++)
    { 
        swapl(p,n);
    }
    for (i=0L, p = (long *)rep->data.events; i<XETrapCoreEvents; i++, p++)
    {
        swapl(p,n); 
    }
    (void)WriteToClient(client,size,reply);
    return;
}

/* Out-going XTrap I/O header needing to be swapped *from* native format */

void sXETrapHeader(XETrapHeader *hdr)
{
    register char n;

    swapl(&(hdr->count), n);
    swapl(&(hdr->timestamp), n);
    swaps(&(hdr->win_x), n);
    swaps(&(hdr->win_y), n);
    swaps(&(hdr->client), n);
}

    /* Out-going requests needing to be swapped *from* native format
     * aka swapreq.c "equivalents" 
     */

/* The following is used for all requests that have
   no fields to be swapped (except "length") */
void XETSwSimpleReq(register xReq *data)
{
    register char n;
    swaps(&(data->length), n);
}

/* The following is used for all requests that have
   only a single 32-bit field to be swapped, coming
   right after the "length" field */

void XETSwResourceReq(register xResourceReq *data)
{
    register char n;

    swaps(&(data->length), n);
    swapl(&(data->id), n);
}

void XETSwCreateWindow(register xCreateWindowReq *data,ClientPtr client)
{
    register char n;

    swapl(&(data->wid), n);
    swapl(&(data->parent), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
    swaps(&(data->borderWidth), n);
    swaps(&(data->class), n);
    swapl(&(data->visual), n);
    swapl(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

void XETSwChangeWindowAttributes(register xChangeWindowAttributesReq *data,
ClientPtr client)
{
    register char n;

    swapl(&(data->window), n);
    swapl(&(data->valueMask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

void XETSwReparentWindow(register xReparentWindowReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->parent), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
}

void XETSwConfigureWindow(xConfigureWindowReq *data, ClientPtr client)
{
    register char n;
    swapl(&(data->window), n);
    swaps(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}


void XETSwInternAtom(register xInternAtomReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->nbytes), n);
}

void XETSwChangeProperty(register xChangePropertyReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->property), n);
    swapl(&(data->type), n);
    switch ( data->format ) {
        case 8L : break;
        case 16L:
            SwapShorts((short *)(data + 1), data->nUnits);
        break;
    case 32L:
            SwapLongs((CARD32 *)(data + 1), data->nUnits);
        break;
    }
    swapl(&(data->nUnits), n);
}

void XETSwDeleteProperty(register xDeletePropertyReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->property), n);
              
}
void XETSwGetProperty(register xGetPropertyReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->property), n);
    swapl(&(data->type), n);
    swapl(&(data->longOffset), n);
    swapl(&(data->longLength), n);
}

void XETSwSetSelectionOwner(register xSetSelectionOwnerReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->selection), n);
    swapl(&(data->time), n);
}

void XETSwConvertSelection(register xConvertSelectionReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->requestor), n);
    swapl(&(data->selection), n);
    swapl(&(data->target), n);
    swapl(&(data->property), n);
    swapl(&(data->time), n);
}

void XETSwSendEvent(register xSendEventReq *data)
{
    register char n;
    xEvent eventT;
    EventSwapPtr proc;
    swapl(&(data->destination), n);
    swapl(&(data->eventMask), n);

    /* Swap event */
    proc = EventSwapVector[data->event.u.u.type & 0177];
    if (!proc || proc == NotImplemented)
        (*proc)(&(data->event), &eventT);
    data->event = eventT;
    swaps(&(data->length), n);
}

void XETSwGrabPointer(register xGrabPointerReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->eventMask), n);
    swapl(&(data->confineTo), n);
    swapl(&(data->cursor), n);
    swapl(&(data->time), n);
}

void XETSwGrabButton(register xGrabButtonReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->eventMask), n);
    swapl(&(data->confineTo), n);
    swapl(&(data->cursor), n);
    swaps(&(data->modifiers), n);
}

void XETSwUngrabButton(register xUngrabButtonReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->modifiers), n);
}

void XETSwChangeActivePointerGrab(register xChangeActivePointerGrabReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cursor), n);
    swapl(&(data->time), n);
    swaps(&(data->eventMask), n);
}

void XETSwGrabKeyboard(register xGrabKeyboardReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swapl(&(data->time), n);
}

void XETSwGrabKey(register xGrabKeyReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->modifiers), n);
}

void XETSwUngrabKey(register xUngrabKeyReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->grabWindow), n);
    swaps(&(data->modifiers), n);
}

void XETSwGetMotionEvents(register xGetMotionEventsReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swapl(&(data->start), n);
    swapl(&(data->stop), n);
}

void XETSwTranslateCoords(register xTranslateCoordsReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcWid), n);
    swapl(&(data->dstWid), n);
    swaps(&(data->srcX), n);
    swaps(&(data->srcY), n);
}

void XETSwWarpPointer(register xWarpPointerReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcWid), n);
    swapl(&(data->dstWid), n);
    swaps(&(data->srcX), n);
    swaps(&(data->srcY), n);
    swaps(&(data->srcWidth), n);
    swaps(&(data->srcHeight), n);
    swaps(&(data->dstX), n);
    swaps(&(data->dstY), n);
}

void XETSwSetInputFocus(register xSetInputFocusReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->focus), n);
    swapl(&(data->time), n);
}

void XETSwOpenFont(register xOpenFontReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->fid), n);
    swaps(&(data->nbytes), n);
}

void XETSwListFonts(register xListFontsReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->maxNames), n);
    swaps(&(data->nbytes), n);
}

void XETSwListFontsWithInfo(register xListFontsWithInfoReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->maxNames), n);
    swaps(&(data->nbytes), n);
}

void XETSwSetFontPath(register xSetFontPathReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->nFonts), n);
}

void XETSwCreatePixmap(register xCreatePixmapReq *data)
{
    register char n;

    swaps(&(data->length), n);
    swapl(&(data->pid), n);
    swapl(&(data->drawable), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
}

void XETSwCreateGC(register xCreateGCReq *data, ClientPtr client)
{
    register char n;
    swapl(&(data->gc), n);
    swapl(&(data->drawable), n);
    swapl(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

void XETSwChangeGC(register xChangeGCReq *data, ClientPtr client)
{
    register char n;
    swapl(&(data->gc), n);
    swapl(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

void XETSwCopyGC(register xCopyGCReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcGC), n);
    swapl(&(data->dstGC), n);
    swapl(&(data->mask), n);
}

void XETSwSetDashes(register xSetDashesReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->gc), n);
    swaps(&(data->dashOffset), n);
    swaps(&(data->nDashes), n);
}

void XETSwSetClipRectangles(register xSetClipRectanglesReq *data, ClientPtr
client)
{
    register char n;
    swapl(&(data->gc), n);
    swaps(&(data->xOrigin), n);
    swaps(&(data->yOrigin), n);
    SwapRestS(data);
    swaps(&(data->length), n);
}

void XETSwClearToBackground(register xClearAreaReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->window), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
}

void XETSwCopyArea(register xCopyAreaReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcDrawable), n);
    swapl(&(data->dstDrawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->srcX), n);
    swaps(&(data->srcY), n);
    swaps(&(data->dstX), n);
    swaps(&(data->dstY), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
}

void XETSwCopyPlane(register xCopyPlaneReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->srcDrawable), n);
    swapl(&(data->dstDrawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->srcX), n);
    swaps(&(data->srcY), n);
    swaps(&(data->dstX), n);
    swaps(&(data->dstY), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
    swapl(&(data->bitPlane), n);
}

/* The following routine is used for all Poly drawing requests
   (except FillPoly, which uses a different request format) */
void XETSwPoly(register xPolyPointReq *data, ClientPtr client)
{
    register char n;

    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    SwapRestS(data);
    swaps(&(data->length), n);
}
     /* cannot use XETSwPoly for this one, because xFillPolyReq
      * is longer than xPolyPointReq, and we don't want to swap
      * the difference as shorts! 
      */
void XETSwFillPoly(register xFillPolyReq *data, ClientPtr client)
{
    register char n;

    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    SwapRestS(data);
    swaps(&(data->length), n);
}

void XETSwPutImage(register xPutImageReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
    swaps(&(data->dstX), n);
    swaps(&(data->dstY), n);
    /* Image should already be swapped */
}

void XETSwGetImage(register xGetImageReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);
    swapl(&(data->planeMask), n);
}

/* ProcPolyText used for both PolyText8 and PolyText16 */

void XETSwPolyText(register xPolyTextReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
}

/* ProcImageText used for both ImageText8 and ImageText16 */

void XETSwImageText(register xImageTextReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swapl(&(data->gc), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
}

void XETSwCreateColormap(register xCreateColormapReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->mid), n);
    swapl(&(data->window), n);
    swapl(&(data->visual), n);
}


void XETSwCopyColormapAndFree(register xCopyColormapAndFreeReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->mid), n);
    swapl(&(data->srcCmap), n);

}

void XETSwAllocColor                (register xAllocColorReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->red), n);
    swaps(&(data->green), n);
    swaps(&(data->blue), n);
}

void XETSwAllocNamedColor           (register xAllocNamedColorReq *data)
{
    register char n;

    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->nbytes), n);
}

void XETSwAllocColorCells           (register xAllocColorCellsReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->colors), n);
    swaps(&(data->planes), n);
}

void XETSwAllocColorPlanes(register xAllocColorPlanesReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->colors), n);
    swaps(&(data->red), n);
    swaps(&(data->green), n);
    swaps(&(data->blue), n);
}

void XETSwFreeColors          (register xFreeColorsReq *data, ClientPtr
client)
{
    register char n;
    swapl(&(data->cmap), n);
    swapl(&(data->planeMask), n);
    SwapRestL(data);
    swaps(&(data->length), n);

}

void XETSwStoreColors               (register xStoreColorsReq *data,ClientPtr
client)
{
    register char n;
    unsigned long count;
    xColorItem     *pItem;

    swapl(&(data->cmap), n);
    pItem = (xColorItem *) &(data[1]);
    for(count = LengthRestB(data)/sizeof(xColorItem); count != 0; count--)
        SwapColorItem(pItem++);
    swaps(&(data->length), n);
}

void XETSwStoreNamedColor           (register xStoreNamedColorReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swapl(&(data->pixel), n);
    swaps(&(data->nbytes), n);
}

void XETSwQueryColors(register xQueryColorsReq *data, ClientPtr client)
{
    register char n;
    swapl(&(data->cmap), n);
    SwapRestL(data);
    swaps(&(data->length), n);
} 

void XETSwLookupColor(register xLookupColorReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cmap), n);
    swaps(&(data->nbytes), n);
}

void XETSwCreateCursor(register xCreateCursorReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cid), n);
    swapl(&(data->source), n);
    swapl(&(data->mask), n);
    swaps(&(data->foreRed), n);
    swaps(&(data->foreGreen), n);
    swaps(&(data->foreBlue), n);
    swaps(&(data->backRed), n);
    swaps(&(data->backGreen), n);
    swaps(&(data->backBlue), n);
    swaps(&(data->x), n);
    swaps(&(data->y), n);
}

void XETSwCreateGlyphCursor(register xCreateGlyphCursorReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cid), n);
    swapl(&(data->source), n);
    swapl(&(data->mask), n);
    swaps(&(data->sourceChar), n);
    swaps(&(data->maskChar), n);
    swaps(&(data->foreRed), n);
    swaps(&(data->foreGreen), n);
    swaps(&(data->foreBlue), n);
    swaps(&(data->backRed), n);
    swaps(&(data->backGreen), n);
    swaps(&(data->backBlue), n);
}


void XETSwRecolorCursor(register xRecolorCursorReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->cursor), n);
    swaps(&(data->foreRed), n);
    swaps(&(data->foreGreen), n);
    swaps(&(data->foreBlue), n);
    swaps(&(data->backRed), n);
    swaps(&(data->backGreen), n);
    swaps(&(data->backBlue), n);
}

void XETSwQueryBestSize   (register xQueryBestSizeReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swapl(&(data->drawable), n);
    swaps(&(data->width), n);
    swaps(&(data->height), n);

}

void XETSwQueryExtension (register xQueryExtensionReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->nbytes), n);
}

void XETSwChangeKeyboardMapping   (register xChangeKeyboardMappingReq *data)
{
    register char n;
    register long *p;
    register int i, count;

    swaps(&(data->length), n);
    p = (long *)&(data[1]);
    count = data->keyCodes * data->keySymsPerKeyCode;
    for(i = 0; i < count; i++)
    {
        swapl(p, n);
        p++;
    }
}


void XETSwChangeKeyboardControl   (register xChangeKeyboardControlReq *data,
    ClientPtr client)
{
    register char n;
    swapl(&(data->mask), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

void XETSwChangePointerControl   (register xChangePointerControlReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->accelNum), n);
    swaps(&(data->accelDenum), n);
    swaps(&(data->threshold), n);
}


void XETSwSetScreenSaver            (register xSetScreenSaverReq *data)
{
    register char n;
    swaps(&(data->length), n);
    swaps(&(data->timeout), n);
    swaps(&(data->interval), n);
}

void XETSwChangeHosts(register xChangeHostsReq *data)
{
    register char n;

    swaps(&(data->length), n);
    swaps(&(data->hostLength), n);

}
void XETSwRotateProperties(register xRotatePropertiesReq *data, ClientPtr client)
{
    register char n;
    swapl(&(data->window), n);
    swaps(&(data->nAtoms), n);
    swaps(&(data->nPositions), n);
    SwapRestL(data);
    swaps(&(data->length), n);
}

/*ARGSUSED*/
void XETSwNoOperation(xReq *data)
{
    /* noop -- don't do anything */
}
