/*
   Copyright (c) 2002  XFree86 Inc
*/

#define NEED_EVENTS
#define NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "swaprep.h"
#include "registry.h"
#include <X11/extensions/XResproto.h>
#include "pixmapstr.h"
#include "windowstr.h"
#include "gcstruct.h"
#include "modinit.h"

static int
ProcXResQueryVersion (ClientPtr client)
{
    REQUEST(xXResQueryVersionReq);
    xXResQueryVersionReply rep;
    CARD16 client_major, client_minor;  /* not used */

    REQUEST_SIZE_MATCH (xXResQueryVersionReq);

    client_major = stuff->client_major;
    client_minor = stuff->client_minor;
    (void) client_major;
    (void) client_minor;

    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.server_major = XRES_MAJOR_VERSION;
    rep.server_minor = XRES_MINOR_VERSION;   
    if (client->swapped) { 
        int n;
        swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);     
        swaps(&rep.server_major, n);
        swaps(&rep.server_minor, n);
    }
    WriteToClient(client, sizeof (xXResQueryVersionReply), (char *)&rep);
    return (client->noClientException);
}

static int
ProcXResQueryClients (ClientPtr client)
{
    /* REQUEST(xXResQueryClientsReq); */
    xXResQueryClientsReply rep;
    int *current_clients;
    int i, num_clients;

    REQUEST_SIZE_MATCH(xXResQueryClientsReq);

    current_clients = xalloc(currentMaxClients * sizeof(int));

    num_clients = 0;
    for(i = 0; i < currentMaxClients; i++) {
       if(clients[i]) {
           current_clients[num_clients] = i;
           num_clients++;   
       }
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.num_clients = num_clients;
    rep.length = rep.num_clients * sz_xXResClient >> 2;
    if (client->swapped) {
        int n;
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swapl (&rep.num_clients, n);
    }   
    WriteToClient (client, sizeof (xXResQueryClientsReply), (char *) &rep);

    if(num_clients) {
        xXResClient scratch;

        for(i = 0; i < num_clients; i++) {
            scratch.resource_base = clients[current_clients[i]]->clientAsMask;
            scratch.resource_mask = RESOURCE_ID_MASK;
        
            if(client->swapped) {
                int n;
                swapl (&scratch.resource_base, n);
                swapl (&scratch.resource_mask, n);
            }
            WriteToClient (client, sz_xXResClient, (char *) &scratch);
        }
    }

    xfree(current_clients);

    return (client->noClientException);
}


static void
ResFindAllRes (pointer value, XID id, RESTYPE type, pointer cdata)
{
    int *counts = (int *)cdata;

    counts[(type & TypeMask) - 1]++;
}

static int
ProcXResQueryClientResources (ClientPtr client)
{
    REQUEST(xXResQueryClientResourcesReq);
    xXResQueryClientResourcesReply rep;
    int i, clientID, num_types;
    int *counts;

    REQUEST_SIZE_MATCH(xXResQueryClientResourcesReq);

    clientID = CLIENT_ID(stuff->xid);

    if((clientID >= currentMaxClients) || !clients[clientID]) {
        client->errorValue = stuff->xid;
        return BadValue;
    }

    counts = xcalloc(lastResourceType + 1, sizeof(int));

    FindAllClientResources(clients[clientID], ResFindAllRes, counts);

    num_types = 0;

    for(i = 0; i <= lastResourceType; i++) {
       if(counts[i]) num_types++;
    }

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.num_types = num_types;
    rep.length = rep.num_types * sz_xXResType >> 2;
    if (client->swapped) {
        int n;
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swapl (&rep.num_types, n);
    }   

    WriteToClient (client,sizeof(xXResQueryClientResourcesReply),(char*)&rep);

    if(num_types) {
        xXResType scratch;
	char *name;

        for(i = 0; i < lastResourceType; i++) {
            if(!counts[i]) continue;

	    name = (char *)LookupResourceName(i + 1);
            if (strcmp(name, XREGISTRY_UNKNOWN))
		scratch.resource_type = MakeAtom(name, strlen(name), TRUE);
	    else {
                char buf[40];
                snprintf(buf, sizeof(buf), "Unregistered resource %i", i + 1);
		scratch.resource_type = MakeAtom(buf, strlen(buf), TRUE);
            }

            scratch.count = counts[i];

            if(client->swapped) {
                int n;
                swapl (&scratch.resource_type, n);
                swapl (&scratch.count, n);
            }
            WriteToClient (client, sz_xXResType, (char *) &scratch);
        }
    }

    xfree(counts);
    
    return (client->noClientException);
}

static unsigned long
ResGetApproxPixmapBytes (PixmapPtr pix)
{
   unsigned long nPixels;
   int           bytesPerPixel; 

   bytesPerPixel = pix->drawable.bitsPerPixel>>3;
   nPixels       = pix->drawable.width * pix->drawable.height;

   /* Divide by refcnt as pixmap could be shared between clients,  
    * so total pixmap mem is shared between these. 
   */
   return ( nPixels * bytesPerPixel ) / pix->refcnt;
}

static void 
ResFindPixmaps (pointer value, XID id, pointer cdata)
{
   unsigned long *bytes = (unsigned long *)cdata;
   PixmapPtr pix = (PixmapPtr)value;

   *bytes += ResGetApproxPixmapBytes(pix);
}

static void
ResFindWindowPixmaps (pointer value, XID id, pointer cdata)
{
   unsigned long *bytes = (unsigned long *)cdata;
   WindowPtr pWin = (WindowPtr)value;

   if (pWin->backgroundState == BackgroundPixmap)
     *bytes += ResGetApproxPixmapBytes(pWin->background.pixmap);

   if (pWin->border.pixmap != NULL && !pWin->borderIsPixel)
     *bytes += ResGetApproxPixmapBytes(pWin->border.pixmap);
}

static void
ResFindGCPixmaps (pointer value, XID id, pointer cdata)
{
   unsigned long *bytes = (unsigned long *)cdata;
   GCPtr pGC = (GCPtr)value;

   if (pGC->stipple != NULL)
     *bytes += ResGetApproxPixmapBytes(pGC->stipple);

   if (pGC->tile.pixmap != NULL && !pGC->tileIsPixel)
     *bytes += ResGetApproxPixmapBytes(pGC->tile.pixmap);
}

static int
ProcXResQueryClientPixmapBytes (ClientPtr client)
{
    REQUEST(xXResQueryClientPixmapBytesReq);
    xXResQueryClientPixmapBytesReply rep;
    int clientID;
    unsigned long bytes;

    REQUEST_SIZE_MATCH(xXResQueryClientPixmapBytesReq);

    clientID = CLIENT_ID(stuff->xid);

    if((clientID >= currentMaxClients) || !clients[clientID]) {
        client->errorValue = stuff->xid;
        return BadValue;
    }

    bytes = 0;

    FindClientResourcesByType(clients[clientID], RT_PIXMAP, ResFindPixmaps, 
                              (pointer)(&bytes));

    /* 
     * Make sure win background pixmaps also held to account. 
     */
    FindClientResourcesByType(clients[clientID], RT_WINDOW, 
			      ResFindWindowPixmaps, 
                              (pointer)(&bytes));

    /* 
     * GC Tile & Stipple pixmaps too.
    */
    FindClientResourcesByType(clients[clientID], RT_GC, 
			      ResFindGCPixmaps, 
                              (pointer)(&bytes));

#ifdef COMPOSITE
    /* FIXME: include composite pixmaps too */
#endif

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.bytes = bytes;
#ifdef _XSERVER64
    rep.bytes_overflow = bytes >> 32;
#else
    rep.bytes_overflow = 0;
#endif
    if (client->swapped) {
        int n;
        swaps (&rep.sequenceNumber, n);
        swapl (&rep.length, n);
        swapl (&rep.bytes, n);
        swapl (&rep.bytes_overflow, n);
    }
    WriteToClient (client,sizeof(xXResQueryClientPixmapBytesReply),(char*)&rep);

    return (client->noClientException);
}

static int
ProcResDispatch (ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_XResQueryVersion:
        return ProcXResQueryVersion(client);
    case X_XResQueryClients:
        return ProcXResQueryClients(client);
    case X_XResQueryClientResources:
        return ProcXResQueryClientResources(client);
    case X_XResQueryClientPixmapBytes:
        return ProcXResQueryClientPixmapBytes(client);
    default: break;
    }

    return BadRequest;
}

static int
SProcXResQueryVersion (ClientPtr client)
{
    REQUEST(xXResQueryVersionReq);
    int n;

    REQUEST_SIZE_MATCH (xXResQueryVersionReq);
    swaps(&stuff->client_major,n);
    swaps(&stuff->client_minor,n);
    return ProcXResQueryVersion(client);
}

static int
SProcXResQueryClientResources (ClientPtr client)
{
    REQUEST(xXResQueryClientResourcesReq);
    int n;

    REQUEST_SIZE_MATCH (xXResQueryClientResourcesReq);
    swaps(&stuff->xid,n);
    return ProcXResQueryClientResources(client);
}

static int
SProcXResQueryClientPixmapBytes (ClientPtr client)
{
    REQUEST(xXResQueryClientPixmapBytesReq);
    int n;

    REQUEST_SIZE_MATCH (xXResQueryClientPixmapBytesReq);
    swaps(&stuff->xid,n);
    return ProcXResQueryClientPixmapBytes(client);
}

static int
SProcResDispatch (ClientPtr client)
{
    REQUEST(xReq);
    int n;

    swaps(&stuff->length,n);

    switch (stuff->data) {
    case X_XResQueryVersion:
        return SProcXResQueryVersion(client);
    case X_XResQueryClients:  /* nothing to swap */
        return ProcXResQueryClients(client);
    case X_XResQueryClientResources:
        return SProcXResQueryClientResources(client);
    case X_XResQueryClientPixmapBytes:
        return SProcXResQueryClientPixmapBytes(client);
    default: break;
    }

    return BadRequest;
}

void
ResExtensionInit(INITARGS)
{
    (void) AddExtension(XRES_NAME, 0, 0,
                            ProcResDispatch, SProcResDispatch,
                            NULL, StandardMinorOpcode);
}
