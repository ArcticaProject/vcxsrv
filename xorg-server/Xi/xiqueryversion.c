/*
 * Copyright © 2009 Red Hat, Inc.
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
 * Authors: Peter Hutterer
 *
 */

/**
 * @file xiqueryversion.c
 * Protocol handling for the XIQueryVersion request/reply.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif


#include "inputstr.h"

#include <X11/Xmd.h>
#include <X11/X.h>
#include <X11/extensions/XI2proto.h>

#include "exglobals.h"
#include "exevents.h"
#include "xiqueryversion.h"
#include "misc.h"

extern XExtensionVersion XIVersion; /* defined in getvers.c */
/**
 * Return the supported XI version.
 *
 * Saves the version the client claims to support as well, for future
 * reference.
 */
int
ProcXIQueryVersion(ClientPtr client)
{
    xXIQueryVersionReply rep;
    XIClientPtr pXIClient;
    int major, minor;
    unsigned int sversion, cversion;

    REQUEST(xXIQueryVersionReq);
    REQUEST_SIZE_MATCH(xXIQueryVersionReq);

    /* This request only exists after XI2 */
    if (stuff->major_version < 2)
    {
        client->errorValue = stuff->major_version;
        return BadValue;
    }

    pXIClient = dixLookupPrivate(&client->devPrivates, XIClientPrivateKey);

    sversion = XIVersion.major_version * 1000 + XIVersion.minor_version;
    cversion = stuff->major_version * 1000 + stuff->minor_version;

    if (sversion > cversion)
    {
        major = stuff->major_version;
        minor = stuff->minor_version;
    } else
    {
        major = XIVersion.major_version;
        minor = XIVersion.minor_version;
    }

    pXIClient->major_version = major;
    pXIClient->minor_version = minor;

    memset(&rep, 0, sizeof(xXIQueryVersionReply));
    rep.repType = X_Reply;
    rep.RepType = X_XIQueryVersion;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.major_version = major;
    rep.minor_version = minor;

    WriteReplyToClient(client, sizeof(xXIQueryVersionReply), &rep);

    return Success;
}

/* Swapping routines */

int
SProcXIQueryVersion(ClientPtr client)
{
    char n;

    REQUEST(xXIQueryVersionReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xXIQueryVersionReq);
    swaps(&stuff->major_version, n);
    swaps(&stuff->minor_version, n);
    return (ProcXIQueryVersion(client));
}

void
SRepXIQueryVersion(ClientPtr client, int size, xXIQueryVersionReply *rep)
{
    char n;
    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    swaps(&rep->major_version, n);
    swaps(&rep->minor_version, n);
    WriteToClient(client, size, (char *)rep);
}
