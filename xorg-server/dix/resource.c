/************************************************************

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
 * Copyright (c) 2005-2006, Oracle and/or its affiliates. All rights reserved.
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
 */

/*	Routines to manage various kinds of resources:
 *
 *	CreateNewResourceType, CreateNewResourceClass, InitClientResources,
 *	FakeClientID, AddResource, FreeResource, FreeClientResources,
 *	FreeAllResources, LookupIDByType, LookupIDByClass, GetXIDRange
 */

/* 
 *      A resource ID is a 32 bit quantity, the upper 2 bits of which are
 *	off-limits for client-visible resources.  The next 8 bits are
 *      used as client ID, and the low 22 bits come from the client.
 *	A resource ID is "hashed" by extracting and xoring subfields
 *      (varying with the size of the hash table).
 *
 *      It is sometimes necessary for the server to create an ID that looks
 *      like it belongs to a client.  This ID, however,  must not be one
 *      the client actually can create, or we have the potential for conflict.
 *      The 31st bit of the ID is reserved for the server's use for this
 *      purpose.  By setting CLIENT_ID(id) to the client, the SERVER_BIT to
 *      1, and an otherwise arbitrary ID in the low 22 bits, we can create a
 *      resource "owned" by the client.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "misc.h"
#include "os.h"
#include "resource.h"
#include "dixstruct.h" 
#include "opaque.h"
#include "windowstr.h"
#include "dixfont.h"
#include "colormap.h"
#include "inputstr.h"
#include "dixevents.h"
#include "dixgrabs.h"
#include "cursor.h"
#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif
#include "xace.h"
#include <assert.h>
#include "registry.h"

#ifdef XSERVER_DTRACE
#include <sys/types.h>
typedef const char *string;
#include "Xserver-dtrace.h"

#define TypeNameString(t) LookupResourceName(t)
#endif

static void RebuildTable(
    int /*client*/
);

#define SERVER_MINID 32

#define INITBUCKETS 64
#define INITHASHSIZE 6
#define MAXHASHSIZE 11

typedef struct _Resource {
    struct _Resource	*next;
    XID			id;
    RESTYPE		type;
    pointer		value;
} ResourceRec, *ResourcePtr;

typedef struct _ClientResource {
    ResourcePtr *resources;
    int		elements;
    int		buckets;
    int		hashsize;	/* log(2)(buckets) */
    XID		fakeID;
    XID		endFakeID;
} ClientResourceRec;

RESTYPE lastResourceType;
static RESTYPE lastResourceClass;
RESTYPE TypeMask;

struct ResourceType {
    DeleteType deleteFunc;
    int errorValue;
};

static struct ResourceType *resourceTypes;
static const struct ResourceType predefTypes[] = {
    [RT_NONE & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = (DeleteType)NoopDDA,
	.errorValue = BadValue,
    },
    [RT_WINDOW & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = DeleteWindow,
	.errorValue = BadWindow,
    },
    [RT_PIXMAP & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = dixDestroyPixmap,
	.errorValue = BadPixmap,
    },
    [RT_GC & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = FreeGC,
	.errorValue = BadGC,
    },
    [RT_FONT & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = CloseFont,
	.errorValue = BadFont,
    },
    [RT_CURSOR & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = FreeCursor,
	.errorValue = BadCursor,
    },
    [RT_COLORMAP & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = FreeColormap,
	.errorValue = BadColor,
    },
    [RT_CMAPENTRY & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = FreeClientPixels,
	.errorValue = BadColor,
    },
    [RT_OTHERCLIENT & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = OtherClientGone,
	.errorValue = BadValue,
    },
    [RT_PASSIVEGRAB & (RC_LASTPREDEF - 1)] = {
	.deleteFunc = DeletePassiveGrab,
	.errorValue = BadValue,
    },
};

CallbackListPtr ResourceStateCallback;

static _X_INLINE void
CallResourceStateCallback(ResourceState state, ResourceRec *res)
{
    if (ResourceStateCallback) {
	ResourceStateInfoRec rsi = { state, res->id, res->type, res->value };
	CallCallbacks(&ResourceStateCallback, &rsi);
    }
}

RESTYPE
CreateNewResourceType(DeleteType deleteFunc, char *name)
{
    RESTYPE next = lastResourceType + 1;
    struct ResourceType *types;

    if (next & lastResourceClass)
	return 0;
    types = realloc(resourceTypes, (next + 1) * sizeof(*resourceTypes));
    if (!types)
	return 0;

    lastResourceType = next;
    resourceTypes = types;
    resourceTypes[next].deleteFunc = deleteFunc;
    resourceTypes[next].errorValue = BadValue;

    /* Called even if name is NULL, to remove any previous entry */
    RegisterResourceName(next, name);

    return next;
}

void
SetResourceTypeErrorValue(RESTYPE type, int errorValue)
{
    resourceTypes[type & TypeMask].errorValue = errorValue;
}

RESTYPE
CreateNewResourceClass(void)
{
    RESTYPE next = lastResourceClass >> 1;

    if (next & lastResourceType)
	return 0;
    lastResourceClass = next;
    TypeMask = next - 1;
    return next;
}

static ClientResourceRec clientTable[MAXCLIENTS];

/*****************
 * InitClientResources
 *    When a new client is created, call this to allocate space
 *    in resource table
 *****************/

Bool
InitClientResources(ClientPtr client)
{
    int i, j;
 
    if (client == serverClient)
    {
	lastResourceType = RT_LASTPREDEF;
	lastResourceClass = RC_LASTPREDEF;
	TypeMask = RC_LASTPREDEF - 1;
	free(resourceTypes);
	resourceTypes = malloc(sizeof(predefTypes));
	if (!resourceTypes)
	    return FALSE;
	memcpy(resourceTypes, predefTypes, sizeof(predefTypes));
    }
    clientTable[i = client->index].resources =
	malloc(INITBUCKETS*sizeof(ResourcePtr));
    if (!clientTable[i].resources)
	return FALSE;
    clientTable[i].buckets = INITBUCKETS;
    clientTable[i].elements = 0;
    clientTable[i].hashsize = INITHASHSIZE;
    /* Many IDs allocated from the server client are visible to clients,
     * so we don't use the SERVER_BIT for them, but we have to start
     * past the magic value constants used in the protocol.  For normal
     * clients, we can start from zero, with SERVER_BIT set.
     */
    clientTable[i].fakeID = client->clientAsMask |
			    (client->index ? SERVER_BIT : SERVER_MINID);
    clientTable[i].endFakeID = (clientTable[i].fakeID | RESOURCE_ID_MASK) + 1;
    for (j=0; j<INITBUCKETS; j++) 
    {
        clientTable[i].resources[j] = NULL;
    }
    return TRUE;
}


static int
Hash(int client, XID id)
{
    id &= RESOURCE_ID_MASK;
    switch (clientTable[client].hashsize)
    {
	case 6:
	    return ((int)(0x03F & (id ^ (id>>6) ^ (id>>12))));
	case 7:
	    return ((int)(0x07F & (id ^ (id>>7) ^ (id>>13))));
	case 8:
	    return ((int)(0x0FF & (id ^ (id>>8) ^ (id>>16))));
	case 9:
	    return ((int)(0x1FF & (id ^ (id>>9))));
	case 10:
	    return ((int)(0x3FF & (id ^ (id>>10))));
	case 11:
	    return ((int)(0x7FF & (id ^ (id>>11))));
    }
    return -1;
}

static XID
AvailableID(
    int client,
    XID id,
    XID maxid,
    XID goodid)
{
    ResourcePtr res;

    if ((goodid >= id) && (goodid <= maxid))
	return goodid;
    for (; id <= maxid; id++)
    {
	res = clientTable[client].resources[Hash(client, id)];
	while (res && (res->id != id))
	    res = res->next;
	if (!res)
	    return id;
    }
    return 0;
}

void
GetXIDRange(int client, Bool server, XID *minp, XID *maxp)
{
    XID id, maxid;
    ResourcePtr *resp;
    ResourcePtr res;
    int i;
    XID goodid;

    id = (Mask)client << CLIENTOFFSET;
    if (server)
	id |= client ? SERVER_BIT : SERVER_MINID;
    maxid = id | RESOURCE_ID_MASK;
    goodid = 0;
    for (resp = clientTable[client].resources, i = clientTable[client].buckets;
	 --i >= 0;)
    {
	for (res = *resp++; res; res = res->next)
	{
	    if ((res->id < id) || (res->id > maxid))
		continue;
	    if (((res->id - id) >= (maxid - res->id)) ?
		(goodid = AvailableID(client, id, res->id - 1, goodid)) :
		!(goodid = AvailableID(client, res->id + 1, maxid, goodid)))
		maxid = res->id - 1;
	    else
		id = res->id + 1;
	}
    }
    if (id > maxid)
	id = maxid = 0;
    *minp = id;
    *maxp = maxid;
}

/**
 *  GetXIDList is called by the XC-MISC extension's MiscGetXIDList function.
 *  This function tries to find count unused XIDs for the given client.  It 
 *  puts the IDs in the array pids and returns the number found, which should
 *  almost always be the number requested.
 *
 *  The circumstances that lead to a call to this function are very rare.
 *  Xlib must run out of IDs while trying to generate a request that wants
 *  multiple ID's, like the Multi-buffering CreateImageBuffers request.
 *
 *  No rocket science in the implementation; just iterate over all
 *  possible IDs for the given client and pick the first count IDs
 *  that aren't in use.  A more efficient algorithm could probably be
 *  invented, but this will be used so rarely that this should suffice.
 */

unsigned int
GetXIDList(ClientPtr pClient, unsigned count, XID *pids)
{
    unsigned int found = 0;
    XID rc, id = pClient->clientAsMask;
    XID maxid;
    pointer val;

    maxid = id | RESOURCE_ID_MASK;
    while ( (found < count) && (id <= maxid) )
    {
	rc = dixLookupResourceByClass(&val, id, RC_ANY, serverClient,
				      DixGetAttrAccess);
	if (rc == BadValue)
	{
	    pids[found++] = id;
	}
	id++;
    }
    return found;
}

/*
 * Return the next usable fake client ID.
 *
 * Normally this is just the next one in line, but if we've used the last
 * in the range, we need to find a new range of safe IDs to avoid
 * over-running another client.
 */

XID
FakeClientID(int client)
{
    XID id, maxid;

    id = clientTable[client].fakeID++;
    if (id != clientTable[client].endFakeID)
	return id;
    GetXIDRange(client, TRUE, &id, &maxid);
    if (!id) {
	if (!client)
	    FatalError("FakeClientID: server internal ids exhausted\n");
	MarkClientException(clients[client]);
	id = ((Mask)client << CLIENTOFFSET) | (SERVER_BIT * 3);
	maxid = id | RESOURCE_ID_MASK;
    }
    clientTable[client].fakeID = id + 1;
    clientTable[client].endFakeID = maxid + 1;
    return id;
}

Bool
AddResource(XID id, RESTYPE type, pointer value)
{
    int client;
    ClientResourceRec *rrec;
    ResourcePtr res, *head;
    	
#ifdef XSERVER_DTRACE
    XSERVER_RESOURCE_ALLOC(id, type, value, TypeNameString(type));
#endif
    client = CLIENT_ID(id);
    rrec = &clientTable[client];
    if (!rrec->buckets)
    {
	ErrorF("[dix] AddResource(%lx, %lx, %lx), client=%d \n",
		(unsigned long)id, type, (unsigned long)value, client);
        FatalError("client not in use\n");
    }
    if ((rrec->elements >= 4*rrec->buckets) &&
	(rrec->hashsize < MAXHASHSIZE))
	RebuildTable(client);
    head = &rrec->resources[Hash(client, id)];
    res = malloc(sizeof(ResourceRec));
    if (!res)
    {
	(*resourceTypes[type & TypeMask].deleteFunc)(value, id);
	return FALSE;
    }
    res->next = *head;
    res->id = id;
    res->type = type;
    res->value = value;
    *head = res;
    rrec->elements++;
    CallResourceStateCallback(ResourceStateAdding, res);
    return TRUE;
}

static void
RebuildTable(int client)
{
    int j;
    ResourcePtr res, next;
    ResourcePtr **tails, *resources;
    ResourcePtr **tptr, *rptr;

    /*
     * For now, preserve insertion order, since some ddx layers depend
     * on resources being free in the opposite order they are added.
     */

    j = 2 * clientTable[client].buckets;
    tails = malloc(j * sizeof(ResourcePtr *));
    if (!tails)
	return;
    resources = malloc(j * sizeof(ResourcePtr));
    if (!resources)
    {
	free(tails);
	return;
    }
    for (rptr = resources, tptr = tails; --j >= 0; rptr++, tptr++)
    {
	*rptr = NULL;
	*tptr = rptr;
    }
    clientTable[client].hashsize++;
    for (j = clientTable[client].buckets,
	 rptr = clientTable[client].resources;
	 --j >= 0;
	 rptr++)
    {
	for (res = *rptr; res; res = next)
	{
	    next = res->next;
	    res->next = NULL;
	    tptr = &tails[Hash(client, res->id)];
	    **tptr = res;
	    *tptr = &res->next;
	}
    }
    free(tails);
    clientTable[client].buckets *= 2;
    free(clientTable[client].resources);
    clientTable[client].resources = resources;
}

void
FreeResource(XID id, RESTYPE skipDeleteFuncType)
{
    int		cid;
    ResourcePtr res;
    ResourcePtr *prev, *head;
    int *eltptr;
    int		elements;

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) && clientTable[cid].buckets)
    {
	head = &clientTable[cid].resources[Hash(cid, id)];
	eltptr = &clientTable[cid].elements;

	prev = head;
	while ( (res = *prev) )
	{
	    if (res->id == id)
	    {
		RESTYPE rtype = res->type;

#ifdef XSERVER_DTRACE
		XSERVER_RESOURCE_FREE(res->id, res->type,
			      res->value, TypeNameString(res->type));
#endif		    
		*prev = res->next;
		elements = --*eltptr;

		CallResourceStateCallback(ResourceStateFreeing, res);

		if (rtype != skipDeleteFuncType)
		    (*resourceTypes[rtype & TypeMask].deleteFunc)(res->value, res->id);
		free(res);
		if (*eltptr != elements)
		    prev = head; /* prev may no longer be valid */
	    }
	    else
		prev = &res->next;
        }
    }
}


void
FreeResourceByType(XID id, RESTYPE type, Bool skipFree)
{
    int		cid;
    ResourcePtr res;
    ResourcePtr *prev, *head;
    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) && clientTable[cid].buckets)
    {
	head = &clientTable[cid].resources[Hash(cid, id)];

	prev = head;
	while ( (res = *prev) )
	{
	    if (res->id == id && res->type == type)
	    {
#ifdef XSERVER_DTRACE
		XSERVER_RESOURCE_FREE(res->id, res->type,
			      res->value, TypeNameString(res->type));
#endif		    		    
		*prev = res->next;
		clientTable[cid].elements--;

		CallResourceStateCallback(ResourceStateFreeing, res);

		if (!skipFree)
		    (*resourceTypes[type & TypeMask].deleteFunc)(res->value, res->id);
		free(res);
		break;
	    }
	    else
		prev = &res->next;
        }
    }
}

/*
 * Change the value associated with a resource id.  Caller
 * is responsible for "doing the right thing" with the old
 * data
 */

Bool
ChangeResourceValue (XID id, RESTYPE rtype, pointer value)
{
    int    cid;
    ResourcePtr res;

    if (((cid = CLIENT_ID(id)) < MAXCLIENTS) && clientTable[cid].buckets)
    {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if ((res->id == id) && (res->type == rtype))
	    {
		res->value = value;
		return TRUE;
	    }
    }
    return FALSE;
}

/* Note: if func adds or deletes resources, then func can get called
 * more than once for some resources.  If func adds new resources,
 * func might or might not get called for them.  func cannot both
 * add and delete an equal number of resources!
 */

void
FindClientResourcesByType(
    ClientPtr client,
    RESTYPE type,
    FindResType func,
    pointer cdata
){
    ResourcePtr *resources;
    ResourcePtr this, next;
    int i, elements;
    int *eltptr;

    if (!client)
	client = serverClient;

    resources = clientTable[client->index].resources;
    eltptr = &clientTable[client->index].elements;
    for (i = 0; i < clientTable[client->index].buckets; i++) 
    {
        for (this = resources[i]; this; this = next)
	{
	    next = this->next;
	    if (!type || this->type == type) {
		elements = *eltptr;
		(*func)(this->value, this->id, cdata);
		if (*eltptr != elements)
		    next = resources[i]; /* start over */
	    }
	}
    }
}

void
FindAllClientResources(
    ClientPtr client,
    FindAllRes func,
    pointer cdata
){
    ResourcePtr *resources;
    ResourcePtr this, next;
    int i, elements;
    int *eltptr;

    if (!client)
        client = serverClient;

    resources = clientTable[client->index].resources;
    eltptr = &clientTable[client->index].elements;
    for (i = 0; i < clientTable[client->index].buckets; i++)
    {
        for (this = resources[i]; this; this = next)
        {
            next = this->next;
            elements = *eltptr;
            (*func)(this->value, this->id, this->type, cdata);
            if (*eltptr != elements)
                next = resources[i]; /* start over */
        }
    }
}


pointer
LookupClientResourceComplex(
    ClientPtr client,
    RESTYPE type,
    FindComplexResType func,
    pointer cdata
){
    ResourcePtr *resources;
    ResourcePtr this, next;
    pointer value;
    int i;

    if (!client)
	client = serverClient;

    resources = clientTable[client->index].resources;
    for (i = 0; i < clientTable[client->index].buckets; i++) {
        for (this = resources[i]; this; this = next) {
	    next = this->next;
	    if (!type || this->type == type) {
		/* workaround func freeing the type as DRI1 does */
		value = this->value;
		if((*func)(value, this->id, cdata))
		    return value;
	    }
	}
    }
    return NULL;
}


void
FreeClientNeverRetainResources(ClientPtr client)
{
    ResourcePtr *resources;
    ResourcePtr this;
    ResourcePtr *prev;
    int j, elements;
    int *eltptr;

    if (!client)
	return;

    resources = clientTable[client->index].resources;
    eltptr = &clientTable[client->index].elements;
    for (j=0; j < clientTable[client->index].buckets; j++) 
    {
	prev = &resources[j];
        while ( (this = *prev) )
	{
	    RESTYPE rtype = this->type;
	    if (rtype & RC_NEVERRETAIN)
	    {
#ifdef XSERVER_DTRACE
		XSERVER_RESOURCE_FREE(this->id, this->type,
			      this->value, TypeNameString(this->type));
#endif		    
		*prev = this->next;
		clientTable[client->index].elements--;

		CallResourceStateCallback(ResourceStateFreeing, this);

		elements = *eltptr;
		(*resourceTypes[rtype & TypeMask].deleteFunc)(this->value, this->id);
		free(this);
		if (*eltptr != elements)
		    prev = &resources[j]; /* prev may no longer be valid */
	    }
	    else
		prev = &this->next;
	}
    }
}

void
FreeClientResources(ClientPtr client)
{
    ResourcePtr *resources;
    ResourcePtr this;
    int j;

    /* This routine shouldn't be called with a null client, but just in
	case ... */

    if (!client)
	return;

    HandleSaveSet(client);

    resources = clientTable[client->index].resources;
    for (j=0; j < clientTable[client->index].buckets; j++) 
    {
        /* It may seem silly to update the head of this resource list as
	we delete the members, since the entire list will be deleted any way, 
	but there are some resource deletion functions "FreeClientPixels" for 
	one which do a LookupID on another resource id (a Colormap id in this
	case), so the resource list must be kept valid up to the point that
	it is deleted, so every time we delete a resource, we must update the
	head, just like in FreeResource. I hope that this doesn't slow down
	mass deletion appreciably. PRH */

	ResourcePtr *head;

	head = &resources[j];

        for (this = *head; this; this = *head)
	{
	    RESTYPE rtype = this->type;
#ifdef XSERVER_DTRACE
	    XSERVER_RESOURCE_FREE(this->id, this->type,
			  this->value, TypeNameString(this->type));
#endif		    
	    *head = this->next;
	    clientTable[client->index].elements--;

	    CallResourceStateCallback(ResourceStateFreeing, this);

	    (*resourceTypes[rtype & TypeMask].deleteFunc)(this->value, this->id);
	    free(this);
	}
    }
    free(clientTable[client->index].resources);
    clientTable[client->index].resources = NULL;
    clientTable[client->index].buckets = 0;
}

void
FreeAllResources(void)
{
    int	i;

    for (i = currentMaxClients; --i >= 0; ) 
    {
        if (clientTable[i].buckets) 
	    FreeClientResources(clients[i]);
    }
}

Bool
LegalNewID(XID id, ClientPtr client)
{
    pointer val;
    int rc;

#ifdef PANORAMIX
    XID 	minid, maxid;

    if (!noPanoramiXExtension) {
        minid = client->clientAsMask | (client->index ?
                                        SERVER_BIT : SERVER_MINID);
        maxid = (clientTable[client->index].fakeID | RESOURCE_ID_MASK) + 1;
        if ((id >= minid) && (id <= maxid))
            return TRUE;
    }
#endif /* PANORAMIX */
    if (client->clientAsMask == (id & ~RESOURCE_ID_MASK))
    {
        rc = dixLookupResourceByClass(&val, id, RC_ANY, serverClient,
                                      DixGetAttrAccess);
        return rc == BadValue;
    }
    return FALSE;
}

int
dixLookupResourceByType(pointer *result, XID id, RESTYPE rtype,
			ClientPtr client, Mask mode)
{
    int cid = CLIENT_ID(id);
    ResourcePtr res = NULL;

    *result = NULL;
    if ((rtype & TypeMask) > lastResourceType)
	return BadImplementation;

    if ((cid < MAXCLIENTS) && clientTable[cid].buckets) {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if (res->id == id && res->type == rtype)
		break;
    }
    if (!res)
	return resourceTypes[rtype & TypeMask].errorValue;

    if (client) {
	client->errorValue = id;
	cid = XaceHook(XACE_RESOURCE_ACCESS, client, id, res->type,
		       res->value, RT_NONE, NULL, mode);
	if (cid == BadValue)
	    return resourceTypes[rtype & TypeMask].errorValue;
	if (cid != Success)
	    return cid;
    }

    *result = res->value;
    return Success;
}

int
dixLookupResourceByClass(pointer *result, XID id, RESTYPE rclass,
			 ClientPtr client, Mask mode)
{
    int cid = CLIENT_ID(id);
    ResourcePtr res = NULL;

    *result = NULL;

    if ((cid < MAXCLIENTS) && clientTable[cid].buckets) {
	res = clientTable[cid].resources[Hash(cid, id)];

	for (; res; res = res->next)
	    if (res->id == id && (res->type & rclass))
		break;
    }
    if (!res)
	return BadValue;

    if (client) {
	client->errorValue = id;
	cid = XaceHook(XACE_RESOURCE_ACCESS, client, id, res->type,
		       res->value, RT_NONE, NULL, mode);
	if (cid != Success)
	    return cid;
    }

    *result = res->value;
    return Success;
}
