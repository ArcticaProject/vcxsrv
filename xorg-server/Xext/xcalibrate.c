/*
 * Copyright © 2003 Philip Blundell
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Philip Blundell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Philip Blundell makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * PHILIP BLUNDELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL PHILIP BLUNDELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_KDRIVE_CONFIG_H
#include <kdrive-config.h>
#endif


#include <X11/X.h>
#include <X11/Xproto.h>
#include "misc.h"
#include "os.h"
#include "dixstruct.h"
#include "extnsionst.h"
#include "swaprep.h"
#include "protocol-versions.h"

#include <X11/extensions/xcalibrateproto.h>
#include <X11/extensions/xcalibratewire.h>

extern void (*tslib_raw_event_hook)(int x, int y, int pressure, void *closure);
extern void *tslib_raw_event_closure;

static CARD8	XCalibrateReqCode;
int		XCalibrateEventBase;
int		XCalibrateReqBase;
int		XCalibrateErrorBase;

static ClientPtr xcalibrate_client;

static void
xcalibrate_event_hook (int x, int y, int pressure, void *closure)
{
  ClientPtr pClient = (ClientPtr) closure;
  xXCalibrateRawTouchscreenEvent	ev;

  ev.type = XCalibrateEventBase + X_XCalibrateRawTouchscreen;
  ev.sequenceNumber = pClient->sequence;
  ev.x = x;
  ev.y = y;
  ev.pressure = pressure;

  if (!pClient->clientGone)
    WriteEventsToClient (pClient, 1, (xEvent *) &ev);
}

static int
ProcXCalibrateQueryVersion (ClientPtr client)
{
  REQUEST(xXCalibrateQueryVersionReq);
  xXCalibrateQueryVersionReply rep;
  CARD16 client_major, client_minor;  /* not used */

  REQUEST_SIZE_MATCH (xXCalibrateQueryVersionReq);

  client_major = stuff->majorVersion;
  client_minor = stuff->minorVersion;

  fprintf(stderr, "%s(): called\n", __func__); 

  rep.type = X_Reply;
  rep.length = 0;
  rep.sequenceNumber = client->sequence;
  rep.majorVersion = SERVER_XCALIBRATE_MAJOR_VERSION;
  rep.minorVersion = SERVER_XCALIBRATE_MINOR_VERSION;
  if (client->swapped) { 
    int n;
    swaps(&rep.sequenceNumber, n);
    swapl(&rep.length, n);     
    swaps(&rep.majorVersion, n);
    swaps(&rep.minorVersion, n);
  }
  WriteToClient(client, sizeof (xXCalibrateQueryVersionReply), (char *)&rep);
  return (client->noClientException);
}

static int
SProcXCalibrateQueryVersion (ClientPtr client)
{
    REQUEST(xXCalibrateQueryVersionReq);
    int n;

    REQUEST_SIZE_MATCH (xXCalibrateQueryVersionReq);
    swaps(&stuff->majorVersion,n);
    swaps(&stuff->minorVersion,n);
    return ProcXCalibrateQueryVersion(client);
}

static int
ProcXCalibrateSetRawMode (ClientPtr client)
{
  REQUEST(xXCalibrateRawModeReq);
  xXCalibrateRawModeReply rep;

  REQUEST_SIZE_MATCH (xXCalibrateRawModeReq);

  memset (&rep, 0, sizeof (rep));
  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;

  if (stuff->on)
    {
      if (xcalibrate_client == NULL)
	{
	  /* Start calibrating.  */
	  xcalibrate_client = client;
	  tslib_raw_event_hook = xcalibrate_event_hook;
	  tslib_raw_event_closure = client;
	  rep.status = GrabSuccess;
	}
      else
	{
	  rep.status = AlreadyGrabbed;
	}
    }
  else
    {
      if (xcalibrate_client == client)
	{
	  /* Stop calibrating.  */
	  xcalibrate_client = NULL;
	  tslib_raw_event_hook = NULL;
	  tslib_raw_event_closure = NULL;
	  rep.status = GrabSuccess;

	  /* Cycle input off and on to reload configuration.  */
	  KdDisableInput ();
	  KdEnableInput ();
	}
      else
	{
	  rep.status = AlreadyGrabbed;
	}
    }

  if (client->swapped)
    {
      int n;

      swaps (&rep.sequenceNumber, n);
      swaps (&rep.status, n);
    }
  WriteToClient(client, sizeof (rep), (char *) &rep);
  return (client->noClientException);
}

static int
SProcXCalibrateSetRawMode (ClientPtr client)
{
  REQUEST(xXCalibrateRawModeReq);
  int n;

  REQUEST_SIZE_MATCH (xXCalibrateRawModeReq);

  swaps(&stuff->on, n);

  return ProcXCalibrateSetRawMode(client);
}

static int
ProcXCalibrateScreenToCoord (ClientPtr client)
{
  REQUEST(xXCalibrateScreenToCoordReq);
  xXCalibrateScreenToCoordReply rep;

  REQUEST_SIZE_MATCH (xXCalibrateScreenToCoordReq);

  memset (&rep, 0, sizeof (rep));
  rep.type = X_Reply;
  rep.sequenceNumber = client->sequence;
  rep.x = stuff->x;
  rep.y = stuff->y;

  KdScreenToPointerCoords(&rep.x, &rep.y);

  if (client->swapped)
    {
      int n;

      swaps (&rep.x, n);
      swaps (&rep.y, n);
    }
  WriteToClient(client, sizeof (rep), (char *) &rep);
  return (client->noClientException);
}

static int
SProcXCalibrateScreenToCoord (ClientPtr client)
{
  REQUEST(xXCalibrateScreenToCoordReq);
  int n;

  REQUEST_SIZE_MATCH (xXCalibrateScreenToCoordReq);

  swaps(&stuff->x, n);
  swaps(&stuff->y, n);

  return ProcXCalibrateScreenToCoord(client);
}

static int
ProcXCalibrateDispatch (ClientPtr client)
{
    REQUEST(xReq);
    switch (stuff->data) {
    case X_XCalibrateQueryVersion:
        return ProcXCalibrateQueryVersion(client);
    case X_XCalibrateRawMode:
        return ProcXCalibrateSetRawMode(client);
    case X_XCalibrateScreenToCoord:
        return ProcXCalibrateScreenToCoord(client);

    default: break;
    }

    return BadRequest;
}

static int
SProcXCalibrateDispatch (ClientPtr client)
{
    REQUEST(xReq);
    int n;

    swaps(&stuff->length,n);

    switch (stuff->data) {
    case X_XCalibrateQueryVersion:
        return SProcXCalibrateQueryVersion(client);
    case X_XCalibrateRawMode:
        return SProcXCalibrateSetRawMode(client);
    case X_XCalibrateScreenToCoord:
        return SProcXCalibrateScreenToCoord(client);

    default: break;
    }

    return BadRequest;
}

static void
XCalibrateClientCallback (CallbackListPtr	*list,
			  pointer		closure,
			  pointer		data)
{
    NewClientInfoRec	*clientinfo = (NewClientInfoRec *) data;
    ClientPtr		pClient = clientinfo->client;

    if (clientinfo->setup == NULL
	&& xcalibrate_client != NULL
	&& xcalibrate_client == pClient)
      {
	/* Stop calibrating.  */
	xcalibrate_client = NULL;
	tslib_raw_event_hook = NULL;
	tslib_raw_event_closure = NULL;
      }
}

void
XCalibrateExtensionInit(void)
{
  ExtensionEntry *extEntry;

  if (!AddCallback (&ClientStateCallback, XCalibrateClientCallback, 0))
    return;  

  extEntry = AddExtension(XCALIBRATE_NAME, XCalibrateNumberEvents, XCalibrateNumberErrors,
			  ProcXCalibrateDispatch, SProcXCalibrateDispatch,
			  NULL, StandardMinorOpcode);

  if (!extEntry)
    return;

  XCalibrateReqCode = (unsigned char)extEntry->base;
  XCalibrateEventBase = extEntry->eventBase;
  XCalibrateErrorBase = extEntry->errorBase;

  xcalibrate_client = 0;
}
