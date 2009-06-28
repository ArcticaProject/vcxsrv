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

RESTYPE	RRModeType;

static Bool
RRModeEqual (xRRModeInfo *a, xRRModeInfo *b)
{
    if (a->width != b->width) return FALSE;
    if (a->height != b->height) return FALSE;
    if (a->dotClock != b->dotClock) return FALSE;
    if (a->hSyncStart != b->hSyncStart) return FALSE;
    if (a->hSyncEnd != b->hSyncEnd) return FALSE;
    if (a->hTotal != b->hTotal) return FALSE;
    if (a->hSkew != b->hSkew) return FALSE;
    if (a->vSyncStart != b->vSyncStart) return FALSE;
    if (a->vSyncEnd != b->vSyncEnd) return FALSE;
    if (a->vTotal != b->vTotal) return FALSE;
    if (a->nameLength != b->nameLength) return FALSE;
    if (a->modeFlags != b->modeFlags) return FALSE;
    return TRUE;
}

/*
 * Keep a list so it's easy to find modes in the resource database.
 */
static int	    num_modes;
static RRModePtr    *modes;

static RRModePtr
RRModeCreate (xRRModeInfo   *modeInfo,
	      const char    *name,
	      ScreenPtr	    userScreen)
{
    RRModePtr	mode, *newModes;
    
    if (!RRInit ())
	return NULL;

    mode = xalloc (sizeof (RRModeRec) + modeInfo->nameLength + 1);
    if (!mode)
	return NULL;
    mode->refcnt = 1;
    mode->mode = *modeInfo;
    mode->name = (char *) (mode + 1);
    memcpy (mode->name, name, modeInfo->nameLength);
    mode->name[modeInfo->nameLength] = '\0';
    mode->userScreen = userScreen;

    if (num_modes)
	newModes = xrealloc (modes, (num_modes + 1) * sizeof (RRModePtr));
    else
	newModes = xalloc (sizeof (RRModePtr));

    if (!newModes)
    {
	xfree (mode);
	return NULL;
    }

    mode->mode.id = FakeClientID(0);
    if (!AddResource (mode->mode.id, RRModeType, (pointer) mode))
	return NULL;
    modes = newModes;
    modes[num_modes++] = mode;
    
    /*
     * give the caller a reference to this mode
     */
    ++mode->refcnt;
    return mode;
}

static RRModePtr
RRModeFindByName (const char	*name,
		  CARD16    	nameLength)
{
    int		i;
    RRModePtr	mode;

    for (i = 0; i < num_modes; i++)
    {
	mode = modes[i];
	if (mode->mode.nameLength == nameLength &&
	    !memcmp (name, mode->name, nameLength))
	{
	    return mode;
	}
    }
    return NULL;
}

RRModePtr
RRModeGet (xRRModeInfo	*modeInfo,
	   const char	*name)
{
    int	i;

    for (i = 0; i < num_modes; i++)
    {
	RRModePtr   mode = modes[i];
	if (RRModeEqual (&mode->mode, modeInfo) &&
	    !memcmp (name, mode->name, modeInfo->nameLength))
	{
	    ++mode->refcnt;
	    return mode;
	}
    }

    return RRModeCreate (modeInfo, name, NULL);
}

static RRModePtr
RRModeCreateUser (ScreenPtr	pScreen,
		  xRRModeInfo	*modeInfo,
		  const char	*name,
		  int		*error)
{
    RRModePtr	mode;

    mode = RRModeFindByName (name, modeInfo->nameLength);
    if (mode)
    {
	*error = BadName;
	return NULL;
    }
    
    mode = RRModeCreate (modeInfo, name, pScreen);
    if (!mode)
    {
	*error = BadAlloc;
	return NULL;
    }
    *error = Success;
    return mode;
}

RRModePtr *
RRModesForScreen (ScreenPtr pScreen, int *num_ret)
{
    rrScrPriv(pScreen);
    int		o, c, m;
    RRModePtr	*screen_modes;
    int		num_screen_modes = 0;

    screen_modes = xalloc ((num_modes ? num_modes : 1) * sizeof (RRModePtr));
    if (!screen_modes)
	return NULL;
    
    /*
     * Add modes from all outputs
     */
    for (o = 0; o < pScrPriv->numOutputs; o++)
    {
	RROutputPtr	output = pScrPriv->outputs[o];
	int		m, n;

	for (m = 0; m < output->numModes + output->numUserModes; m++)
	{
	    RRModePtr   mode = (m < output->numModes ? 
				output->modes[m] : 
				output->userModes[m-output->numModes]);
	    for (n = 0; n < num_screen_modes; n++)
		if (screen_modes[n] == mode)
		    break;
	    if (n == num_screen_modes)
		screen_modes[num_screen_modes++] = mode;
	}
    }
    /*
     * Add modes from all crtcs. The goal is to
     * make sure all available and active modes
     * are visible to the client
     */
    for (c = 0; c < pScrPriv->numCrtcs; c++)
    {
	RRCrtcPtr	crtc = pScrPriv->crtcs[c];
	RRModePtr	mode = crtc->mode;
	int		n;

	if (!mode) continue;
	for (n = 0; n < num_screen_modes; n++)
	    if (screen_modes[n] == mode)
		break;
	if (n == num_screen_modes)
	    screen_modes[num_screen_modes++] = mode;
    }
    /*
     * Add all user modes for this screen
     */
    for (m = 0; m < num_modes; m++)
    {
	RRModePtr	mode = modes[m];
	int		n;

	if (mode->userScreen != pScreen)
	    continue;
	for (n = 0; n < num_screen_modes; n++)
	    if (screen_modes[n] == mode)
		break;
	if (n == num_screen_modes)
	    screen_modes[num_screen_modes++] = mode;
    }
    
    *num_ret = num_screen_modes;
    return screen_modes;
}

void
RRModeDestroy (RRModePtr mode)
{
    int	m;
    
    if (--mode->refcnt > 0)
	return;
    for (m = 0; m < num_modes; m++)
    {
	if (modes[m] == mode)
	{
	    memmove (modes + m, modes + m + 1,
		     (num_modes - m - 1) * sizeof (RRModePtr));
	    num_modes--;
	    if (!num_modes)
	    {
		xfree (modes);
		modes = NULL;
	    }
	    break;
	}
    }
    
    xfree (mode);
}

static int
RRModeDestroyResource (pointer value, XID pid)
{
    RRModeDestroy ((RRModePtr) value);
    return 1;
}

Bool
RRModeInit (void)
{
    assert (num_modes == 0);
    assert (modes == NULL);
    RRModeType = CreateNewResourceType (RRModeDestroyResource);
    if (!RRModeType)
	return FALSE;
    RegisterResourceName (RRModeType, "MODE");
    return TRUE;
}

int
ProcRRCreateMode (ClientPtr client)
{
    REQUEST(xRRCreateModeReq);
    xRRCreateModeReply	rep;
    WindowPtr		pWin;
    ScreenPtr		pScreen;
    rrScrPrivPtr	pScrPriv;
    xRRModeInfo		*modeInfo;
    long		units_after;
    char		*name;
    int			error, rc;
    RRModePtr		mode;
    
    REQUEST_AT_LEAST_SIZE (xRRCreateModeReq);
    rc = dixLookupWindow(&pWin, stuff->window, client, DixReadAccess);
    if (rc != Success)
	return rc;

    pScreen = pWin->drawable.pScreen;
    pScrPriv = rrGetScrPriv(pScreen);
    
    modeInfo = &stuff->modeInfo;
    name = (char *) (stuff + 1);
    units_after = (stuff->length - (sizeof (xRRCreateModeReq) >> 2));

    /* check to make sure requested name fits within the data provided */
    if ((int) (modeInfo->nameLength + 3) >> 2 > units_after)
	return BadLength;

    mode = RRModeCreateUser (pScreen, modeInfo, name, &error);
    if (!mode)
	return error;

    rep.type = X_Reply;
    rep.pad0 = 0;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.mode = mode->mode.id;
    if (client->swapped)
    {
	int n;
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.mode, n);
    }
    WriteToClient(client, sizeof(xRRCreateModeReply), (char *)&rep);
    
    return client->noClientException;
}

int
ProcRRDestroyMode (ClientPtr client)
{
    REQUEST(xRRDestroyModeReq);
    RRModePtr	mode;
    
    REQUEST_SIZE_MATCH(xRRDestroyModeReq);
    mode = LookupIDByType (stuff->mode, RRModeType);
    if (!mode)
    {
	client->errorValue = stuff->mode;
	return RRErrorBase + BadRRMode;
    }
    if (!mode->userScreen)
	return BadMatch;
    if (mode->refcnt > 1)
	return BadAccess;
    FreeResource (stuff->mode, 0);
    return Success;
}

int
ProcRRAddOutputMode (ClientPtr client)
{
    REQUEST(xRRAddOutputModeReq);
    RRModePtr	mode;
    RROutputPtr	output;
    
    REQUEST_SIZE_MATCH(xRRAddOutputModeReq);
    output = LookupOutput(client, stuff->output, DixReadAccess);

    if (!output)
    {
	client->errorValue = stuff->output;
	return RRErrorBase + BadRROutput;
    }
    
    mode = LookupIDByType (stuff->mode, RRModeType);
    if (!mode)
    {
	client->errorValue = stuff->mode;
	return RRErrorBase + BadRRMode;
    }
    
    return RROutputAddUserMode (output, mode);
}

int
ProcRRDeleteOutputMode (ClientPtr client)
{
    REQUEST(xRRDeleteOutputModeReq);
    RRModePtr	mode;
    RROutputPtr	output;
    
    REQUEST_SIZE_MATCH(xRRDeleteOutputModeReq);
    output = LookupOutput(client, stuff->output, DixReadAccess);

    if (!output)
    {
	client->errorValue = stuff->output;
	return RRErrorBase + BadRROutput;
    }
    
    mode = LookupIDByType (stuff->mode, RRModeType);
    if (!mode)
    {
	client->errorValue = stuff->mode;
	return RRErrorBase + BadRRMode;
    }
    
    return RROutputDeleteUserMode (output, mode);
}
