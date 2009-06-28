/************************************************************

Copyright 1989, 1998  The Open Group

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

********************************************************/

/* RANDOM CRUFT! THIS HAS NO OFFICIAL X CONSORTIUM OR X PROJECT TEAM  BLESSING */


#define NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#define _MITMISC_SERVER_
#include <X11/extensions/mitmiscstr.h>
#include "modinit.h"

static void MITResetProc(
    ExtensionEntry * /* extEntry */
);

static DISPATCH_PROC(ProcMITDispatch);
static DISPATCH_PROC(ProcMITGetBugMode);
static DISPATCH_PROC(ProcMITSetBugMode);
static DISPATCH_PROC(SProcMITDispatch);
static DISPATCH_PROC(SProcMITGetBugMode);
static DISPATCH_PROC(SProcMITSetBugMode);

void
MITMiscExtensionInit(INITARGS)
{
    AddExtension(MITMISCNAME, 0, 0,
		 ProcMITDispatch, SProcMITDispatch,
		 MITResetProc, StandardMinorOpcode);
}

/*ARGSUSED*/
static void
MITResetProc (extEntry)
ExtensionEntry	*extEntry;
{
}

static int
ProcMITSetBugMode(client)
    register ClientPtr client;
{
    REQUEST(xMITSetBugModeReq);

    REQUEST_SIZE_MATCH(xMITSetBugModeReq);
    if (stuff->onOff != xFalse)
        return BadRequest;
    return(client->noClientException);
}

static int
ProcMITGetBugMode(client)
    register ClientPtr client;
{
    xMITGetBugModeReply rep;
    register int n;

    REQUEST_SIZE_MATCH(xMITGetBugModeReq);
    rep.type = X_Reply;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.onOff = FALSE;
    if (client->swapped) {
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
    }
    WriteToClient(client, sizeof(xMITGetBugModeReply), (char *)&rep);
    return(client->noClientException);
}

static int
ProcMITDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_MITSetBugMode:
	return ProcMITSetBugMode(client);
    case X_MITGetBugMode:
	return ProcMITGetBugMode(client);
    default:
	return BadRequest;
    }
}

static int
SProcMITSetBugMode(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xMITSetBugModeReq);

    swaps(&stuff->length, n);
    return ProcMITSetBugMode(client);
}

static int
SProcMITGetBugMode(client)
    register ClientPtr	client;
{
    register int n;
    REQUEST(xMITGetBugModeReq);

    swaps(&stuff->length, n);
    return ProcMITGetBugMode(client);
}

static int
SProcMITDispatch (client)
    register ClientPtr	client;
{
    REQUEST(xReq);
    switch (stuff->data)
    {
    case X_MITSetBugMode:
	return SProcMITSetBugMode(client);
    case X_MITGetBugMode:
	return SProcMITGetBugMode(client);
    default:
	return BadRequest;
    }
}
