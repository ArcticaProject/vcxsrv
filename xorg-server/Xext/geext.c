/*
 * Copyright 2007-2008 Peter Hutterer
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
 * Author: Peter Hutterer, University of South Australia, NICTA
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif
#include "windowstr.h"
#include <X11/extensions/ge.h>
#include "registry.h"

#include "geint.h"
#include "geext.h"

/* Currently supported XGE version */
#define SERVER_GE_MAJOR 1
#define SERVER_GE_MINOR 0

#define rClient(obj) (clients[CLIENT_ID((obj)->resource)])

int GEEventBase;
int GEErrorBase;
static int GEClientPrivateKeyIndex;
DevPrivateKey GEClientPrivateKey = &GEClientPrivateKeyIndex;
int GEEventType; /* The opcode for all GenericEvents will have. */

int RT_GECLIENT  = 0;


GEExtension GEExtensions[MAXEXTENSIONS];

/* Major available requests */
static const int version_requests[] = {
    X_GEQueryVersion,	/* before client sends QueryVersion */
    X_GEQueryVersion,	/* must be set to last request in version 1 */
};

/* Forward declarations */
static void SGEGenericEvent(xEvent* from, xEvent* to);
static void GERecalculateWinMask(WindowPtr pWin);

#define NUM_VERSION_REQUESTS	(sizeof (version_requests) / sizeof (version_requests[0]))

/************************************************************/
/*                request handlers                          */
/************************************************************/

static int
ProcGEQueryVersion(ClientPtr client)
{
    int n;
    GEClientInfoPtr pGEClient = GEGetClient(client);
    xGEQueryVersionReply rep;
    REQUEST(xGEQueryVersionReq);

    REQUEST_SIZE_MATCH(xGEQueryVersionReq);

    rep.repType = X_Reply;
    rep.RepType = X_GEQueryVersion;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    /* return the supported version by the server */
    rep.majorVersion = SERVER_GE_MAJOR;
    rep.minorVersion = SERVER_GE_MINOR;

    /* Remember version the client requested */
    pGEClient->major_version = stuff->majorVersion;
    pGEClient->minor_version = stuff->minorVersion;

    if (client->swapped)
    {
	swaps(&rep.sequenceNumber, n);
        swapl(&rep.length, n);
        swaps(&rep.majorVersion, n);
        swaps(&rep.minorVersion, n);
    }

    WriteToClient(client, sizeof(xGEQueryVersionReply), (char*)&rep);
    return(client->noClientException);
}

int (*ProcGEVector[GENumberRequests])(ClientPtr) = {
    /* Version 1.0 */
    ProcGEQueryVersion
};

/************************************************************/
/*                swapped request handlers                  */
/************************************************************/
static int
SProcGEQueryVersion(ClientPtr client)
{
    int n;
    REQUEST(xGEQueryVersionReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH(xGEQueryVersionReq);
    swaps(&stuff->majorVersion, n);
    swaps(&stuff->minorVersion, n);
    return(*ProcGEVector[stuff->ReqType])(client);
}

int (*SProcGEVector[GENumberRequests])(ClientPtr) = {
    /* Version 1.0 */
    SProcGEQueryVersion
};


/************************************************************/
/*                callbacks                                 */
/************************************************************/

/* dispatch requests */
static int
ProcGEDispatch(ClientPtr client)
{
    GEClientInfoPtr pGEClient = GEGetClient(client);
    REQUEST(xGEReq);

    if (pGEClient->major_version >= NUM_VERSION_REQUESTS)
        return BadRequest;
    if (stuff->ReqType > version_requests[pGEClient->major_version])
        return BadRequest;

    return (ProcGEVector[stuff->ReqType])(client);
}

/* dispatch swapped requests */
static int
SProcGEDispatch(ClientPtr client)
{
    REQUEST(xGEReq);
    if (stuff->ReqType >= GENumberRequests)
        return BadRequest;
    return (*SProcGEVector[stuff->ReqType])(client);
}

/**
 * Called when a new client inits a connection to the X server.
 *
 * We alloc a simple struct to store the client's major/minor version. Can be
 * used in the furture for versioning support.
 */
static void
GEClientCallback(CallbackListPtr *list,
                 pointer closure,
                 pointer data)
{
    NewClientInfoRec	*clientinfo = (NewClientInfoRec *) data;
    ClientPtr		pClient = clientinfo->client;
    GEClientInfoPtr     pGEClient = GEGetClient(pClient);

    if (pGEClient == NULL)
    {
        pGEClient = xcalloc(1, sizeof(GEClientInfoRec));
        dixSetPrivate(&pClient->devPrivates, GEClientPrivateKey, pGEClient);
    }

    pGEClient->major_version = 0;
    pGEClient->minor_version = 0;
}

/* Reset extension. Called on server shutdown. */
static void
GEResetProc(ExtensionEntry *extEntry)
{
    DeleteCallback(&ClientStateCallback, GEClientCallback, 0);
    EventSwapVector[GenericEvent] = NotImplemented;

    GEEventBase = 0;
    GEErrorBase = 0;
    GEEventType = 0;
}

/*  Calls the registered event swap function for the extension.
 *
 *  Each extension can register a swap function to handle GenericEvents being
 *  swapped properly. The server calls SGEGenericEvent() before the event is
 *  written on the wire, this one calls the registered swap function to do the
 *  work.
 */
static void
SGEGenericEvent(xEvent* from, xEvent* to)
{
    xGenericEvent* gefrom = (xGenericEvent*)from;
    xGenericEvent* geto = (xGenericEvent*)to;

    if (gefrom->extension > MAXEXTENSIONS)
    {
        ErrorF("GE: Invalid extension offset for event.\n");
        return;
    }

    if (GEExtensions[gefrom->extension & 0x7F].evswap)
        GEExtensions[gefrom->extension & 0x7F].evswap(gefrom, geto);
}

/**
 * Resource callback, invoked when the client disconnects and the associated
 * GE masks must be destroyed.
 */
static int
GEClientGone(WindowPtr pWin, XID id)
{
    GenericClientMasksPtr gclmask;
    GenericMaskPtr        gmask, prev = NULL;

    if (!pWin || !pWin->optional)
        return Success;

    gclmask = pWin->optional->geMasks;
    for (gmask = gclmask->geClients; gmask; gmask = gmask->next)
    {
        if (gmask->resource == id)
        {
            if (prev)
            {
                prev->next = gmask->next;
                xfree(gmask);
            } else {
                gclmask->geClients = NULL;
                CheckWindowOptionalNeed(pWin);
                GERecalculateWinMask(pWin);
                xfree(gmask);
            }
            return Success;
        }
        prev = gmask;
    }

    FatalError("Client not a GE client");
    return BadImplementation;
}

/* Init extension, register at server.
 * Since other extensions may rely on XGE (XInput does already), it is a good
 * idea to init XGE first, before any other extension.
 */
void
GEExtensionInit(void)
{
    ExtensionEntry *extEntry;

    if(!AddCallback(&ClientStateCallback, GEClientCallback, 0))
    {
        FatalError("GEExtensionInit: register client callback failed.\n");
    }

    if((extEntry = AddExtension(GE_NAME,
                        GENumberEvents, GENumberErrors,
                        ProcGEDispatch, SProcGEDispatch,
                        GEResetProc, StandardMinorOpcode)) != 0)
    {
        GEEventBase = extEntry->eventBase;
        GEErrorBase = extEntry->errorBase;
        GEEventType = GEEventBase;

        RT_GECLIENT = CreateNewResourceType((DeleteType)GEClientGone);
        RegisterResourceName(RT_GECLIENT, "GECLIENT");

        memset(GEExtensions, 0, sizeof(GEExtensions));

        EventSwapVector[GenericEvent] = (EventSwapPtr) SGEGenericEvent;
    } else {
        FatalError("GEInit: AddExtensions failed.\n");
    }

}

/************************************************************/
/*                interface for extensions                  */
/************************************************************/

/* Register an extension with GE. The given swap function will be called each
 * time an event is sent to a client with different byte order.
 * @param extension The extensions major opcode
 * @param ev_swap The event swap function.
 * @param ev_fill Called for an event before delivery. The extension now has
 * the chance to fill in necessary fields for the event.
 */
void
GERegisterExtension(int extension,
                    void (*ev_swap)(xGenericEvent* from, xGenericEvent* to),
                    void (*ev_fill)(xGenericEvent* ev, DeviceIntPtr pDev,
                                    WindowPtr pWin, GrabPtr pGrab))
{
    if ((extension & 0x7F) >=  MAXEXTENSIONS)
        FatalError("GE: extension > MAXEXTENSIONS. This should not happen.\n");

    /* extension opcodes are > 128, might as well save some space here */
    GEExtensions[extension & 0x7f].evswap = ev_swap;
    GEExtensions[extension & 0x7f].evfill = ev_fill;
}


/* Sets type and extension field for a generic event. This is just an
 * auxiliary function, extensions could do it manually too.
 */
void
GEInitEvent(xGenericEvent* ev, int extension)
{
    ev->type = GenericEvent;
    ev->extension = extension;
    ev->length = 0;
}

/* Recalculates the summary mask for the window. */
static void
GERecalculateWinMask(WindowPtr pWin)
{
    int i;
    GenericMaskPtr it;
    GenericClientMasksPtr evmasks;

    if (!pWin->optional)
        return;

    evmasks = pWin->optional->geMasks;

    for (i = 0; i < MAXEXTENSIONS; i++)
    {
        evmasks->eventMasks[i] = 0;
    }

    it = pWin->optional->geMasks->geClients;
    while(it)
    {
        for (i = 0; i < MAXEXTENSIONS; i++)
        {
            evmasks->eventMasks[i] |= it->eventMask[i];
        }
        it = it->next;
    }
}

/* Set generic event mask for given window. */
void
GEWindowSetMask(ClientPtr pClient, DeviceIntPtr pDev,
                WindowPtr pWin, int extension, Mask mask)
{
    GenericMaskPtr cli;

    extension = (extension & 0x7F);

    if (extension >= MAXEXTENSIONS)
    {
        ErrorF("Invalid extension number.\n");
        return;
    }

    if (!pWin->optional && !MakeWindowOptional(pWin))
    {
        ErrorF("GE: Could not make window optional.\n");
        return;
    }

    if (mask)
    {
        GenericClientMasksPtr evmasks = pWin->optional->geMasks;

        /* check for existing client */
        cli = evmasks->geClients;
        while(cli)
        {
            if (rClient(cli) == pClient && cli->dev == pDev)
                break;
            cli = cli->next;
        }
        if (!cli)
        {
            /* new client and/or new device */
            cli  = (GenericMaskPtr)xcalloc(1, sizeof(GenericMaskRec));
            if (!cli)
            {
                ErrorF("GE: Insufficient memory to alloc client.\n");
                return;
            }
            cli->next = evmasks->geClients;
            cli->resource = FakeClientID(pClient->index);
            cli->dev = pDev;
            evmasks->geClients = cli;
            AddResource(cli->resource, RT_GECLIENT, (pointer)pWin);
        }
        cli->eventMask[extension] = mask;
    } else
    {
        /* remove client. */
        cli = pWin->optional->geMasks->geClients;
        if (rClient(cli) == pClient && cli->dev == pDev)
        {
            pWin->optional->geMasks->geClients = cli->next;
            xfree(cli);
        } else
        {
            GenericMaskPtr prev = cli;
            cli = cli->next;

            while(cli)
            {
                if (rClient(cli) == pClient && cli->dev == pDev)
                {
                    prev->next = cli->next;
                    xfree(cli);
                    break;
                }
                prev = cli;
                cli = cli->next;
            }
        }
        if (!cli)
            return;
    }

    GERecalculateWinMask(pWin);
}

/**
 * Return TRUE if the mask for the given device is set.
 * @param pWin Window the event may be delivered to.
 * @param pDev Device the device originating the event. May be NULL.
 * @param extension Extension ID
 * @param mask Event mask
 */
BOOL
GEDeviceMaskIsSet(WindowPtr pWin, DeviceIntPtr pDev,
                  int extension, Mask mask)
{
    GenericMaskPtr gemask;

    if (!pWin->optional || !pWin->optional->geMasks)
        return FALSE;

    extension &= 0x7F;

    if (!pWin->optional->geMasks->eventMasks[extension] & mask)
        return FALSE;


    gemask = pWin->optional->geMasks->geClients;

    while(gemask)
    {
        if ((!gemask->dev || gemask->dev == pDev) &&
                (gemask->eventMask[extension] & mask))
            return TRUE;

        gemask = gemask->next;
    }

    return FALSE;
}

