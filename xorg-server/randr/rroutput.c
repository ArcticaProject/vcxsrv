/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "randrstr.h"
#include "registry.h"

RESTYPE	RROutputType;

/*
 * Notify the output of some change
 */
void
RROutputChanged (RROutputPtr output, Bool configChanged)
{
    ScreenPtr	pScreen = output->pScreen;
    
    output->changed = TRUE;
    if (pScreen)
    {
	rrScrPriv (pScreen);
	pScrPriv->changed = TRUE;
	if (configChanged)
	    pScrPriv->configChanged = TRUE;
    }
}

/*
 * Create an output
 */

RROutputPtr
RROutputCreate (ScreenPtr   pScreen,
		const char  *name,
		int	    nameLength,
		void	    *devPrivate)
{
    RROutputPtr	    output;
    RROutputPtr	    *outputs;
    rrScrPrivPtr    pScrPriv;

    if (!RRInit())
	return NULL;
    
    pScrPriv = rrGetScrPriv(pScreen);

    if (pScrPriv->numOutputs)
	outputs = xrealloc (pScrPriv->outputs, 
			    (pScrPriv->numOutputs + 1) * sizeof (RROutputPtr));
    else
	outputs = xalloc (sizeof (RROutputPtr));
    if (!outputs)
	return FALSE;

    pScrPriv->outputs = outputs;
    
    output = xalloc (sizeof (RROutputRec) + nameLength + 1);
    if (!output)
	return NULL;
    output->id = FakeClientID (0);
    output->pScreen = pScreen;
    output->name = (char *) (output + 1);
    output->nameLength = nameLength;
    memcpy (output->name, name, nameLength);
    output->name[nameLength] = '\0';
    output->connection = RR_UnknownConnection;
    output->subpixelOrder = SubPixelUnknown;
    output->mmWidth = 0;
    output->mmHeight = 0;
    output->crtc = NULL;
    output->numCrtcs = 0;
    output->crtcs = NULL;
    output->numClones = 0;
    output->clones = NULL;
    output->numModes = 0;
    output->numPreferred = 0;
    output->modes = NULL;
    output->numUserModes = 0;
    output->userModes = NULL;
    output->properties = NULL;
    output->pendingProperties = FALSE;
    output->changed = FALSE;
    output->devPrivate = devPrivate;
    
    if (!AddResource (output->id, RROutputType, (pointer) output))
	return NULL;

    pScrPriv->outputs[pScrPriv->numOutputs++] = output;
    return output;
}

/*
 * Notify extension that output parameters have been changed
 */
Bool
RROutputSetClones (RROutputPtr  output,
		   RROutputPtr  *clones,
		   int		numClones)
{
    RROutputPtr	*newClones;
    int		i;

    if (numClones == output->numClones)
    {
	for (i = 0; i < numClones; i++)
	    if (output->clones[i] != clones[i])
		break;
	if (i == numClones)
	    return TRUE;
    }
    if (numClones)
    {
	newClones = xalloc (numClones * sizeof (RROutputPtr));
	if (!newClones)
	    return FALSE;
    }
    else
	newClones = NULL;
    if (output->clones)
	xfree (output->clones);
    memcpy (newClones, clones, numClones * sizeof (RROutputPtr));
    output->clones = newClones;
    output->numClones = numClones;
    RROutputChanged (output, TRUE);
    return TRUE;
}

Bool
RROutputSetModes (RROutputPtr	output,
		  RRModePtr	*modes,
		  int		numModes,
		  int		numPreferred)
{
    RRModePtr	*newModes;
    int		i;

    if (numModes == output->numModes && numPreferred == output->numPreferred)
    {
	for (i = 0; i < numModes; i++)
	    if (output->modes[i] != modes[i])
		break;
	if (i == numModes)
	{
	    for (i = 0; i < numModes; i++)
		RRModeDestroy (modes[i]);
	    return TRUE;
	}
    }

    if (numModes)
    {
	newModes = xalloc (numModes * sizeof (RRModePtr));
	if (!newModes)
	    return FALSE;
    }
    else
	newModes = NULL;
    if (output->modes)
    {
	for (i = 0; i < output->numModes; i++)
	    RRModeDestroy (output->modes[i]);
	xfree (output->modes);
    }
    memcpy (newModes, modes, numModes * sizeof (RRModePtr));
    output->modes = newModes;
    output->numModes = numModes;
    output->numPreferred = numPreferred;
    RROutputChanged (output, TRUE);
    return TRUE;
}

int
RROutputAddUserMode (RROutputPtr    output,
		     RRModePtr	    mode)
{
    int		m;
    ScreenPtr	pScreen = output->pScreen;
    rrScrPriv(pScreen);
    RRModePtr	*newModes;

    /* Check to see if this mode is already listed for this output */
    for (m = 0; m < output->numModes + output->numUserModes; m++)
    {
	RRModePtr   e = (m < output->numModes ?
			 output->modes[m] :
			 output->userModes[m - output->numModes]);
	if (mode == e)
	    return Success;
    }

    /* Check with the DDX to see if this mode is OK */
    if (pScrPriv->rrOutputValidateMode)
	if (!pScrPriv->rrOutputValidateMode (pScreen, output, mode))
	    return BadMatch;

    if (output->userModes)
	newModes = xrealloc (output->userModes,
			     (output->numUserModes + 1) * sizeof (RRModePtr));
    else
	newModes = xalloc (sizeof (RRModePtr));
    if (!newModes)
	return BadAlloc;

    output->userModes = newModes;
    output->userModes[output->numUserModes++] = mode;
    ++mode->refcnt;
    RROutputChanged (output, TRUE);
    RRTellChanged (pScreen);
    return Success;
}

int
RROutputDeleteUserMode (RROutputPtr output,
			RRModePtr   mode)
{
    int		m;
    
    /* Find this mode in the user mode list */
    for (m = 0; m < output->numUserModes; m++)
    {
	RRModePtr   e = output->userModes[m];

	if (mode == e)
	    break;
    }
    /* Not there, access error */
    if (m == output->numUserModes)
	return BadAccess;

    /* make sure the mode isn't active for this output */
    if (output->crtc && output->crtc->mode == mode)
	return BadMatch;

    memmove (output->userModes + m, output->userModes + m + 1,
	     (output->numUserModes - m - 1) * sizeof (RRModePtr));
    output->numUserModes--;
    RRModeDestroy (mode);
    return Success;
}

Bool
RROutputSetCrtcs (RROutputPtr	output,
		  RRCrtcPtr	*crtcs,
		  int		numCrtcs)
{
    RRCrtcPtr	*newCrtcs;
    int		i;

    if (numCrtcs == output->numCrtcs)
    {
	for (i = 0; i < numCrtcs; i++)
	    if (output->crtcs[i] != crtcs[i])
		break;
	if (i == numCrtcs)
	    return TRUE;
    }
    if (numCrtcs)
    {
	newCrtcs = xalloc (numCrtcs * sizeof (RRCrtcPtr));
	if (!newCrtcs)
	    return FALSE;
    }
    else
	newCrtcs = NULL;
    if (output->crtcs)
	xfree (output->crtcs);
    memcpy (newCrtcs, crtcs, numCrtcs * sizeof (RRCrtcPtr));
    output->crtcs = newCrtcs;
    output->numCrtcs = numCrtcs;
    RROutputChanged (output, TRUE);
    return TRUE;
}

Bool
RROutputSetConnection (RROutputPtr  output,
		       CARD8	    connection)
{
    if (output->connection == connection)
	return TRUE;
    output->connection = connection;
    RROutputChanged (output, TRUE);
    return TRUE;
}

Bool
RROutputSetSubpixelOrder (RROutputPtr output,
			  int	      subpixelOrder)
{
    if (output->subpixelOrder == subpixelOrder)
	return TRUE;

    output->subpixelOrder = subpixelOrder;
    RROutputChanged (output, FALSE);
    return TRUE;
}

Bool
RROutputSetPhysicalSize (RROutputPtr	output,
			 int		mmWidth,
			 int		mmHeight)
{
    if (output->mmWidth == mmWidth && output->mmHeight == mmHeight)
	return TRUE;
    output->mmWidth = mmWidth;
    output->mmHeight = mmHeight;
    RROutputChanged (output, FALSE);
    return TRUE;
}


void
RRDeliverOutputEvent(ClientPtr client, WindowPtr pWin, RROutputPtr output)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    rrScrPriv (pScreen);
    xRROutputChangeNotifyEvent	oe;
    RRCrtcPtr	crtc = output->crtc;
    RRModePtr	mode = crtc ? crtc->mode : 0;
    
    oe.type = RRNotify + RREventBase;
    oe.subCode = RRNotify_OutputChange;
    oe.sequenceNumber = client->sequence;
    oe.timestamp = pScrPriv->lastSetTime.milliseconds;
    oe.configTimestamp = pScrPriv->lastConfigTime.milliseconds;
    oe.window = pWin->drawable.id;
    oe.output = output->id;
    if (crtc)
    {
	oe.crtc = crtc->id;
	oe.mode = mode ? mode->mode.id : None;
	oe.rotation = crtc->rotation;
    }
    else
    {
	oe.crtc = None;
	oe.mode = None;
	oe.rotation = RR_Rotate_0;
    }
    oe.connection = output->connection;
    oe.subpixelOrder = output->subpixelOrder;
    WriteEventsToClient (client, 1, (xEvent *) &oe);
}

/*
 * Destroy a Output at shutdown
 */
void
RROutputDestroy (RROutputPtr output)
{
    FreeResource (output->id, 0);
}

static int
RROutputDestroyResource (pointer value, XID pid)
{
    RROutputPtr	output = (RROutputPtr) value;
    ScreenPtr	pScreen = output->pScreen;
    int		m;

    if (pScreen)
    {
	rrScrPriv(pScreen);
	int		i;
    
	for (i = 0; i < pScrPriv->numOutputs; i++)
	{
	    if (pScrPriv->outputs[i] == output)
	    {
		memmove (pScrPriv->outputs + i, pScrPriv->outputs + i + 1,
			 (pScrPriv->numOutputs - (i + 1)) * sizeof (RROutputPtr));
		--pScrPriv->numOutputs;
		break;
	    }
	}
    }
    if (output->modes)
    {
	for (m = 0; m < output->numModes; m++)
	    RRModeDestroy (output->modes[m]);
	xfree (output->modes);
    }
    
    for (m = 0; m < output->numUserModes; m++)
	RRModeDestroy (output->userModes[m]);
    if (output->userModes)
	xfree (output->userModes);

    if (output->crtcs)
	xfree (output->crtcs);
    if (output->clones)
	xfree (output->clones);
    RRDeleteAllOutputProperties (output);
    xfree (output);
    return 1;
}

/*
 * Initialize output type
 */
Bool
RROutputInit (void)
{
    RROutputType = CreateNewResourceType (RROutputDestroyResource);
    if (!RROutputType)
	return FALSE;
    RegisterResourceName (RROutputType, "OUTPUT");
    return TRUE;
}

#define OutputInfoExtra	(SIZEOF(xRRGetOutputInfoReply) - 32)
				
int
ProcRRGetOutputInfo (ClientPtr client)
{
    REQUEST(xRRGetOutputInfoReq);
    xRRGetOutputInfoReply	rep;
    RROutputPtr			output;
    CARD8			*extra;
    unsigned long		extraLen;
    ScreenPtr			pScreen;
    rrScrPrivPtr		pScrPriv;
    RRCrtc			*crtcs;
    RRMode			*modes;
    RROutput			*clones;
    char			*name;
    int				i, n;
    
    REQUEST_SIZE_MATCH(xRRGetOutputInfoReq);
    output = LookupOutput(client, stuff->output, DixReadAccess);

    if (!output)
    {
	client->errorValue = stuff->output;
	return RRErrorBase + BadRROutput;
    }

    pScreen = output->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);

    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = OutputInfoExtra >> 2;
    rep.timestamp = pScrPriv->lastSetTime.milliseconds;
    rep.crtc = output->crtc ? output->crtc->id : None;
    rep.mmWidth = output->mmWidth;
    rep.mmHeight = output->mmHeight;
    rep.connection = output->connection;
    rep.subpixelOrder = output->subpixelOrder;
    rep.nCrtcs = output->numCrtcs;
    rep.nModes = output->numModes + output->numUserModes;
    rep.nPreferred = output->numPreferred;
    rep.nClones = output->numClones;
    rep.nameLength = output->nameLength;
    
    extraLen = ((output->numCrtcs + 
		 output->numModes + output->numUserModes +
		 output->numClones +
		 ((rep.nameLength + 3) >> 2)) << 2);

    if (extraLen)
    {
	rep.length += extraLen >> 2;
	extra = xalloc (extraLen);
	if (!extra)
	    return BadAlloc;
    }
    else
	extra = NULL;

    crtcs = (RRCrtc *) extra;
    modes = (RRMode *) (crtcs + output->numCrtcs);
    clones = (RROutput *) (modes + output->numModes + output->numUserModes);
    name = (char *) (clones + output->numClones);
    
    for (i = 0; i < output->numCrtcs; i++)
    {
	crtcs[i] = output->crtcs[i]->id;
	if (client->swapped)
	    swapl (&crtcs[i], n);
    }
    for (i = 0; i < output->numModes + output->numUserModes; i++)
    {
	if (i < output->numModes)
	    modes[i] = output->modes[i]->mode.id;
	else
	    modes[i] = output->userModes[i - output->numModes]->mode.id;
	if (client->swapped)
	    swapl (&modes[i], n);
    }
    for (i = 0; i < output->numClones; i++)
    {
	clones[i] = output->clones[i]->id;
	if (client->swapped)
	    swapl (&clones[i], n);
    }
    memcpy (name, output->name, output->nameLength);
    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.timestamp, n);
	swapl(&rep.crtc, n);
	swapl(&rep.mmWidth, n);
	swapl(&rep.mmHeight, n);
	swaps(&rep.nCrtcs, n);
	swaps(&rep.nModes, n);
	swaps(&rep.nClones, n);
	swaps(&rep.nameLength, n);
    }
    WriteToClient(client, sizeof(xRRGetOutputInfoReply), (char *)&rep);
    if (extraLen)
    {
	WriteToClient (client, extraLen, (char *) extra);
	xfree (extra);
    }
    
    return client->noClientException;
}
