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
#include "swaprep.h"
#include "registry.h"

RESTYPE	RRCrtcType;

/*
 * Notify the CRTC of some change
 */
void
RRCrtcChanged (RRCrtcPtr crtc, Bool layoutChanged)
{
    ScreenPtr	pScreen = crtc->pScreen;

    crtc->changed = TRUE;
    if (pScreen)
    {
	rrScrPriv(pScreen);
    
	pScrPriv->changed = TRUE;
	/*
	 * Send ConfigureNotify on any layout change
	 */
	if (layoutChanged)
	    pScrPriv->layoutChanged = TRUE;
    }
}

/*
 * Create a CRTC
 */
RRCrtcPtr
RRCrtcCreate (ScreenPtr pScreen, void *devPrivate)
{
    RRCrtcPtr	    crtc;
    RRCrtcPtr	    *crtcs;
    rrScrPrivPtr    pScrPriv;

    if (!RRInit())
	return NULL;
    
    pScrPriv = rrGetScrPriv(pScreen);

    /* make space for the crtc pointer */
    if (pScrPriv->numCrtcs)
	crtcs = xrealloc (pScrPriv->crtcs, 
			  (pScrPriv->numCrtcs + 1) * sizeof (RRCrtcPtr));
    else
	crtcs = xalloc (sizeof (RRCrtcPtr));
    if (!crtcs)
	return FALSE;
    pScrPriv->crtcs = crtcs;
    
    crtc = xcalloc (1, sizeof (RRCrtcRec));
    if (!crtc)
	return NULL;
    crtc->id = FakeClientID (0);
    crtc->pScreen = pScreen;
    crtc->mode = NULL;
    crtc->x = 0;
    crtc->y = 0;
    crtc->rotation = RR_Rotate_0;
    crtc->rotations = RR_Rotate_0;
    crtc->outputs = NULL;
    crtc->numOutputs = 0;
    crtc->gammaSize = 0;
    crtc->gammaRed = crtc->gammaBlue = crtc->gammaGreen = NULL;
    crtc->changed = FALSE;
    crtc->devPrivate = devPrivate;
    RRTransformInit (&crtc->client_pending_transform);
    RRTransformInit (&crtc->client_current_transform);
    pixman_transform_init_identity (&crtc->transform);
    pixman_f_transform_init_identity (&crtc->f_transform);
    pixman_f_transform_init_identity (&crtc->f_inverse);

    if (!AddResource (crtc->id, RRCrtcType, (pointer) crtc))
	return NULL;

    /* attach the screen and crtc together */
    crtc->pScreen = pScreen;
    pScrPriv->crtcs[pScrPriv->numCrtcs++] = crtc;
    
    return crtc;
}

/*
 * Set the allowed rotations on a CRTC
 */
void
RRCrtcSetRotations (RRCrtcPtr crtc, Rotation rotations)
{
    crtc->rotations = rotations;
}

/*
 * Set whether transforms are allowed on a CRTC
 */
void
RRCrtcSetTransformSupport (RRCrtcPtr crtc, Bool transforms)
{
    crtc->transforms = transforms;
}

/*
 * Notify the extension that the Crtc has been reconfigured,
 * the driver calls this whenever it has updated the mode
 */
Bool
RRCrtcNotify (RRCrtcPtr	    crtc,
	      RRModePtr	    mode,
	      int	    x,
	      int	    y,
	      Rotation	    rotation,
	      RRTransformPtr transform,
	      int	    numOutputs,
	      RROutputPtr   *outputs)
{
    int	    i, j;
    
    /*
     * Check to see if any of the new outputs were
     * not in the old list and mark them as changed
     */
    for (i = 0; i < numOutputs; i++)
    {
	for (j = 0; j < crtc->numOutputs; j++)
	    if (outputs[i] == crtc->outputs[j])
		break;
	if (j == crtc->numOutputs)
	{
	    outputs[i]->crtc = crtc;
	    RROutputChanged (outputs[i], FALSE);
	    RRCrtcChanged (crtc, FALSE);
	}
    }
    /*
     * Check to see if any of the old outputs are
     * not in the new list and mark them as changed
     */
    for (j = 0; j < crtc->numOutputs; j++)
    {
	for (i = 0; i < numOutputs; i++)
	    if (outputs[i] == crtc->outputs[j])
		break;
	if (i == numOutputs)
	{
	    if (crtc->outputs[j]->crtc == crtc)
		crtc->outputs[j]->crtc = NULL;
	    RROutputChanged (crtc->outputs[j], FALSE);
	    RRCrtcChanged (crtc, FALSE);
	}
    }
    /*
     * Reallocate the crtc output array if necessary
     */
    if (numOutputs != crtc->numOutputs)
    {
	RROutputPtr *newoutputs;
	
	if (numOutputs)
	{
	    if (crtc->numOutputs)
		newoutputs = xrealloc (crtc->outputs,
				    numOutputs * sizeof (RROutputPtr));
	    else
		newoutputs = xalloc (numOutputs * sizeof (RROutputPtr));
	    if (!newoutputs)
		return FALSE;
	}
	else
	{
	    if (crtc->outputs)
		xfree (crtc->outputs);
	    newoutputs = NULL;
	}
	crtc->outputs = newoutputs;
	crtc->numOutputs = numOutputs;
    }
    /*
     * Copy the new list of outputs into the crtc
     */
    memcpy (crtc->outputs, outputs, numOutputs * sizeof (RROutputPtr));
    /*
     * Update remaining crtc fields
     */
    if (mode != crtc->mode)
    {
	if (crtc->mode)
	    RRModeDestroy (crtc->mode);
	crtc->mode = mode;
	if (mode != NULL)
	    mode->refcnt++;
	RRCrtcChanged (crtc, TRUE);
    }
    if (x != crtc->x)
    {
	crtc->x = x;
	RRCrtcChanged (crtc, TRUE);
    }
    if (y != crtc->y)
    {
	crtc->y = y;
	RRCrtcChanged (crtc, TRUE);
    }
    if (rotation != crtc->rotation)
    {
	crtc->rotation = rotation;
	RRCrtcChanged (crtc, TRUE);
    }
    if (!RRTransformEqual (transform, &crtc->client_current_transform)) {
	RRTransformCopy (&crtc->client_current_transform, transform);
	RRCrtcChanged (crtc, TRUE);
    }
    if (crtc->changed && mode)
    {
	RRTransformCompute (x, y,
			    mode->mode.width, mode->mode.height,
			    rotation,
			    &crtc->client_current_transform,
			    &crtc->transform, &crtc->f_transform,
			    &crtc->f_inverse);
    }
    return TRUE;
}

void
RRDeliverCrtcEvent (ClientPtr client, WindowPtr pWin, RRCrtcPtr crtc)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    rrScrPriv (pScreen);
    xRRCrtcChangeNotifyEvent	ce;
    RRModePtr	mode = crtc->mode;
    
    ce.type = RRNotify + RREventBase;
    ce.subCode = RRNotify_CrtcChange;
    ce.sequenceNumber = client->sequence;
    ce.timestamp = pScrPriv->lastSetTime.milliseconds;
    ce.window = pWin->drawable.id;
    ce.crtc = crtc->id;
    ce.rotation = crtc->rotation;
    if (mode)
    {
	ce.mode = mode->mode.id;
	ce.x = crtc->x;
	ce.y = crtc->y;
	ce.width = mode->mode.width;
	ce.height = mode->mode.height;
    }
    else
    {
	ce.mode = None;
	ce.x = 0;
	ce.y = 0;
	ce.width = 0;
	ce.height = 0;
    }
    WriteEventsToClient (client, 1, (xEvent *) &ce);
}

static Bool
RRCrtcPendingProperties (RRCrtcPtr crtc)
{
    ScreenPtr	pScreen = crtc->pScreen;
    rrScrPriv(pScreen);
    int		o;

    for (o = 0; o < pScrPriv->numOutputs; o++)
    {
	RROutputPtr output = pScrPriv->outputs[o];
	if (output->crtc == crtc && output->pendingProperties)
	    return TRUE;
    }
    return FALSE;
}

/*
 * Request that the Crtc be reconfigured
 */
Bool
RRCrtcSet (RRCrtcPtr    crtc,
	   RRModePtr	mode,
	   int		x,
	   int		y,
	   Rotation	rotation,
	   int		numOutputs,
	   RROutputPtr  *outputs)
{
    ScreenPtr	pScreen = crtc->pScreen;
    Bool	ret = FALSE;
    rrScrPriv(pScreen);

    /* See if nothing changed */
    if (crtc->mode == mode &&
	crtc->x == x &&
	crtc->y == y &&
	crtc->rotation == rotation &&
	crtc->numOutputs == numOutputs &&
	!memcmp (crtc->outputs, outputs, numOutputs * sizeof (RROutputPtr)) &&
	!RRCrtcPendingProperties (crtc) &&
	!RRCrtcPendingTransform (crtc))
    {
	ret = TRUE;
    }
    else
    {
#if RANDR_12_INTERFACE
	if (pScrPriv->rrCrtcSet)
	{
	    ret = (*pScrPriv->rrCrtcSet) (pScreen, crtc, mode, x, y, 
					  rotation, numOutputs, outputs);
	}
	else
#endif
	{
#if RANDR_10_INTERFACE
	    if (pScrPriv->rrSetConfig)
	    {
		RRScreenSize	    size;
		RRScreenRate	    rate;

		if (!mode)
		{
		    RRCrtcNotify (crtc, NULL, x, y, rotation, NULL, 0, NULL);
		    ret = TRUE;
		}
		else
		{
		    size.width = mode->mode.width;
		    size.height = mode->mode.height;
		    if (outputs[0]->mmWidth && outputs[0]->mmHeight)
		    {
			size.mmWidth = outputs[0]->mmWidth;
			size.mmHeight = outputs[0]->mmHeight;
		    }
		    else
		    {
			size.mmWidth = pScreen->mmWidth;
			size.mmHeight = pScreen->mmHeight;
		    }
		    size.nRates = 1;
		    rate.rate = RRVerticalRefresh (&mode->mode);
		    size.pRates = &rate;
		    ret = (*pScrPriv->rrSetConfig) (pScreen, rotation, rate.rate, &size);
		    /*
		     * Old 1.0 interface tied screen size to mode size
		     */
		    if (ret)
		    {
			RRCrtcNotify (crtc, mode, x, y, rotation, NULL, 1, outputs);
			RRScreenSizeNotify (pScreen);
		    }
		}
	    }
#endif
	}
	if (ret)
	{
	    int	o;
	    RRTellChanged (pScreen);

	    for (o = 0; o < numOutputs; o++)
		RRPostPendingProperties (outputs[o]);
	}
    }
    return ret;
}

/*
 * Return crtc transform
 */
RRTransformPtr
RRCrtcGetTransform (RRCrtcPtr crtc)
{
    RRTransformPtr  transform = &crtc->client_pending_transform;

    if (pixman_transform_is_identity (&transform->transform))
	return NULL;
    return transform;
}

/*
 * Check whether the pending and current transforms are the same
 */
Bool
RRCrtcPendingTransform (RRCrtcPtr crtc)
{
    return memcmp (&crtc->client_current_transform.transform,
		   &crtc->client_pending_transform.transform,
		   sizeof (PictTransform)) != 0;
}

/*
 * Destroy a Crtc at shutdown
 */
void
RRCrtcDestroy (RRCrtcPtr crtc)
{
    FreeResource (crtc->id, 0);
}

static int
RRCrtcDestroyResource (pointer value, XID pid)
{
    RRCrtcPtr	crtc = (RRCrtcPtr) value;
    ScreenPtr	pScreen = crtc->pScreen;

    if (pScreen)
    {
	rrScrPriv(pScreen);
	int		i;
    
	for (i = 0; i < pScrPriv->numCrtcs; i++)
	{
	    if (pScrPriv->crtcs[i] == crtc)
	    {
		memmove (pScrPriv->crtcs + i, pScrPriv->crtcs + i + 1,
			 (pScrPriv->numCrtcs - (i + 1)) * sizeof (RRCrtcPtr));
		--pScrPriv->numCrtcs;
		break;
	    }
	}
    }
    if (crtc->gammaRed)
	xfree (crtc->gammaRed);
    if (crtc->mode)
	RRModeDestroy (crtc->mode);
    xfree (crtc);
    return 1;
}

/*
 * Request that the Crtc gamma be changed
 */

Bool
RRCrtcGammaSet (RRCrtcPtr   crtc,
		CARD16	    *red,
		CARD16	    *green,
		CARD16	    *blue)
{
    Bool	ret = TRUE;
#if RANDR_12_INTERFACE
    ScreenPtr	pScreen = crtc->pScreen;
#endif
    
    memcpy (crtc->gammaRed, red, crtc->gammaSize * sizeof (CARD16));
    memcpy (crtc->gammaGreen, green, crtc->gammaSize * sizeof (CARD16));
    memcpy (crtc->gammaBlue, blue, crtc->gammaSize * sizeof (CARD16));
#if RANDR_12_INTERFACE
    if (pScreen)
    {
	rrScrPriv(pScreen);
	if (pScrPriv->rrCrtcSetGamma)
	    ret = (*pScrPriv->rrCrtcSetGamma) (pScreen, crtc);
    }
#endif
    return ret;
}

/*
 * Notify the extension that the Crtc gamma has been changed
 * The driver calls this whenever it has changed the gamma values
 * in the RRCrtcRec
 */

Bool
RRCrtcGammaNotify (RRCrtcPtr	crtc)
{
    return TRUE;    /* not much going on here */
}

static void
RRModeGetScanoutSize (RRModePtr mode, PictTransformPtr transform,
		      int *width, int *height)
{
    BoxRec  box;

    if (mode == NULL) {
	*width = 0;
	*height = 0;
	return;
    }

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = mode->mode.width;
    box.y2 = mode->mode.height;

    pixman_transform_bounds (transform, &box);
    *width = box.x2 - box.x1;
    *height = box.y2 - box.y1;
}

/**
 * Returns the width/height that the crtc scans out from the framebuffer
 */
void
RRCrtcGetScanoutSize(RRCrtcPtr crtc, int *width, int *height)
{
    return RRModeGetScanoutSize (crtc->mode, &crtc->transform, width, height);
}

/*
 * Set the size of the gamma table at server startup time
 */

Bool
RRCrtcGammaSetSize (RRCrtcPtr	crtc,
		    int		size)
{
    CARD16  *gamma;

    if (size == crtc->gammaSize)
	return TRUE;
    if (size)
    {
	gamma = xalloc (size * 3 * sizeof (CARD16));
	if (!gamma)
	    return FALSE;
    }
    else
	gamma = NULL;
    if (crtc->gammaRed)
	xfree (crtc->gammaRed);
    crtc->gammaRed = gamma;
    crtc->gammaGreen = gamma + size;
    crtc->gammaBlue = gamma + size*2;
    crtc->gammaSize = size;
    return TRUE;
}

/*
 * Set the pending CRTC transformation
 */

int
RRCrtcTransformSet (RRCrtcPtr		crtc,
		    PictTransformPtr	transform,
		    struct pixman_f_transform *f_transform,
		    struct pixman_f_transform *f_inverse,
		    char		*filter_name,
		    int			filter_len,
		    xFixed		*params,
		    int			nparams)
{
    PictFilterPtr   filter = NULL;
    int		    width = 0, height = 0;

    if (!crtc->transforms)
	return BadValue;

    if (filter_len)
    {
	filter = PictureFindFilter (crtc->pScreen,
				    filter_name,
				    filter_len);
	if (!filter)
	    return BadName;
	if (filter->ValidateParams)
	{
	    if (!filter->ValidateParams (crtc->pScreen, filter->id,
					 params, nparams, &width, &height))
		return BadMatch;
	}
	else {
	    width = filter->width;
	    height = filter->height;
	}
    }
    else
    {
	if (nparams)
	    return BadMatch;
    }
    if (!RRTransformSetFilter (&crtc->client_pending_transform,
			       filter, params, nparams, width, height))
	return BadAlloc;

    crtc->client_pending_transform.transform = *transform;
    crtc->client_pending_transform.f_transform = *f_transform;
    crtc->client_pending_transform.f_inverse = *f_inverse;
    return Success;
}

/*
 * Initialize crtc type
 */
Bool
RRCrtcInit (void)
{
    RRCrtcType = CreateNewResourceType (RRCrtcDestroyResource);
    if (!RRCrtcType)
	return FALSE;
    RegisterResourceName (RRCrtcType, "CRTC");
    return TRUE;
}

int
ProcRRGetCrtcInfo (ClientPtr client)
{
    REQUEST(xRRGetCrtcInfoReq);
    xRRGetCrtcInfoReply	rep;
    RRCrtcPtr			crtc;
    CARD8			*extra;
    unsigned long		extraLen;
    ScreenPtr			pScreen;
    rrScrPrivPtr		pScrPriv;
    RRModePtr			mode;
    RROutput			*outputs;
    RROutput			*possible;
    int				i, j, k, n;
    int				width, height;
    BoxRec			panned_area;
    
    REQUEST_SIZE_MATCH(xRRGetCrtcInfoReq);
    crtc = LookupCrtc(client, stuff->crtc, DixReadAccess);

    if (!crtc)
	return RRErrorBase + BadRRCrtc;

    /* All crtcs must be associated with screens before client
     * requests are processed
     */
    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);

    mode = crtc->mode;
    
    rep.type = X_Reply;
    rep.status = RRSetConfigSuccess;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.timestamp = pScrPriv->lastSetTime.milliseconds;
    if (pScrPriv->rrGetPanning &&
	pScrPriv->rrGetPanning (pScreen, crtc, &panned_area, NULL, NULL) &&
	(panned_area.x2 > panned_area.x1) && (panned_area.y2 > panned_area.y1))
    {
 	rep.x = panned_area.x1;
	rep.y = panned_area.y1;
	rep.width = panned_area.x2 - panned_area.x1;
	rep.height = panned_area.y2 - panned_area.y1;
    }
    else
    {
	RRCrtcGetScanoutSize (crtc, &width, &height);
	rep.x = crtc->x;
	rep.y = crtc->y;
	rep.width = width;
	rep.height = height;
    }
    rep.mode = mode ? mode->mode.id : 0;
    rep.rotation = crtc->rotation;
    rep.rotations = crtc->rotations;
    rep.nOutput = crtc->numOutputs;
    k = 0;
    for (i = 0; i < pScrPriv->numOutputs; i++)
	for (j = 0; j < pScrPriv->outputs[i]->numCrtcs; j++)
	    if (pScrPriv->outputs[i]->crtcs[j] == crtc)
		k++;
    rep.nPossibleOutput = k;
    
    rep.length = rep.nOutput + rep.nPossibleOutput;

    extraLen = rep.length << 2;
    if (extraLen)
    {
	extra = xalloc (extraLen);
	if (!extra)
	    return BadAlloc;
    }
    else
	extra = NULL;

    outputs = (RROutput *) extra;
    possible = (RROutput *) (outputs + rep.nOutput);
    
    for (i = 0; i < crtc->numOutputs; i++)
    {
	outputs[i] = crtc->outputs[i]->id;
	if (client->swapped)
	    swapl (&outputs[i], n);
    }
    k = 0;
    for (i = 0; i < pScrPriv->numOutputs; i++)
	for (j = 0; j < pScrPriv->outputs[i]->numCrtcs; j++)
	    if (pScrPriv->outputs[i]->crtcs[j] == crtc)
	    {
		possible[k] = pScrPriv->outputs[i]->id;
		if (client->swapped)
		    swapl (&possible[k], n);
		k++;
	    }
    
    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swapl(&rep.timestamp, n);
	swaps(&rep.x, n);
	swaps(&rep.y, n);
	swaps(&rep.width, n);
	swaps(&rep.height, n);
	swapl(&rep.mode, n);
	swaps(&rep.rotation, n);
	swaps(&rep.rotations, n);
	swaps(&rep.nOutput, n);
	swaps(&rep.nPossibleOutput, n);
    }
    WriteToClient(client, sizeof(xRRGetCrtcInfoReply), (char *)&rep);
    if (extraLen)
    {
	WriteToClient (client, extraLen, (char *) extra);
	xfree (extra);
    }
    
    return client->noClientException;
}

int
ProcRRSetCrtcConfig (ClientPtr client)
{
    REQUEST(xRRSetCrtcConfigReq);
    xRRSetCrtcConfigReply   rep;
    ScreenPtr		    pScreen;
    rrScrPrivPtr	    pScrPriv;
    RRCrtcPtr		    crtc;
    RRModePtr		    mode;
    int			    numOutputs;
    RROutputPtr		    *outputs = NULL;
    RROutput		    *outputIds;
    TimeStamp		    configTime;
    TimeStamp		    time;
    Rotation		    rotation;
    int			    i, j;
    
    REQUEST_AT_LEAST_SIZE(xRRSetCrtcConfigReq);
    numOutputs = (stuff->length - (SIZEOF (xRRSetCrtcConfigReq) >> 2));
    
    crtc = LookupIDByType (stuff->crtc, RRCrtcType);
    if (!crtc)
    {
	client->errorValue = stuff->crtc;
	return RRErrorBase + BadRRCrtc;
    }
    if (stuff->mode == None)
    {
	mode = NULL;
	if (numOutputs > 0)
	    return BadMatch;
    }
    else
    {
	mode = LookupIDByType (stuff->mode, RRModeType);
	if (!mode)
	{
	    client->errorValue = stuff->mode;
	    return RRErrorBase + BadRRMode;
	}
	if (numOutputs == 0)
	    return BadMatch;
    }
    if (numOutputs)
    {
	outputs = xalloc (numOutputs * sizeof (RROutputPtr));
	if (!outputs)
	    return BadAlloc;
    }
    else
	outputs = NULL;
    
    outputIds = (RROutput *) (stuff + 1);
    for (i = 0; i < numOutputs; i++)
    {
	outputs[i] = (RROutputPtr) LookupIDByType (outputIds[i], RROutputType);
	if (!outputs[i])
	{
	    client->errorValue = outputIds[i];
	    if (outputs)
		xfree (outputs);
	    return RRErrorBase + BadRROutput;
	}
	/* validate crtc for this output */
	for (j = 0; j < outputs[i]->numCrtcs; j++)
	    if (outputs[i]->crtcs[j] == crtc)
		break;
	if (j == outputs[i]->numCrtcs)
	{
	    if (outputs)
		xfree (outputs);
	    return BadMatch;
	}
	/* validate mode for this output */
	for (j = 0; j < outputs[i]->numModes + outputs[i]->numUserModes; j++)
	{
	    RRModePtr	m = (j < outputs[i]->numModes ? 
			     outputs[i]->modes[j] :
			     outputs[i]->userModes[j - outputs[i]->numModes]);
	    if (m == mode)
		break;
	}
	if (j == outputs[i]->numModes + outputs[i]->numUserModes)
	{
	    if (outputs)
		xfree (outputs);
	    return BadMatch;
	}
    }
    /* validate clones */
    for (i = 0; i < numOutputs; i++)
    {
	for (j = 0; j < numOutputs; j++)
	{
	    int k;
	    if (i == j)
		continue;
	    for (k = 0; k < outputs[i]->numClones; k++)
	    {
		if (outputs[i]->clones[k] == outputs[j])
		    break;
	    }
	    if (k == outputs[i]->numClones)
	    {
		if (outputs)
		    xfree (outputs);
		return BadMatch;
	    }
	}
    }

    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);
    
    time = ClientTimeToServerTime(stuff->timestamp);
    configTime = ClientTimeToServerTime(stuff->configTimestamp);
    
    if (!pScrPriv)
    {
	time = currentTime;
	rep.status = RRSetConfigFailed;
	goto sendReply;
    }
    
#if 0
    /*
     * if the client's config timestamp is not the same as the last config
     * timestamp, then the config information isn't up-to-date and
     * can't even be validated
     */
    if (CompareTimeStamps (configTime, pScrPriv->lastConfigTime) != 0)
    {
	rep.status = RRSetConfigInvalidConfigTime;
	goto sendReply;
    }
#endif
    
    /*
     * Validate requested rotation
     */
    rotation = (Rotation) stuff->rotation;

    /* test the rotation bits only! */
    switch (rotation & 0xf) {
    case RR_Rotate_0:
    case RR_Rotate_90:
    case RR_Rotate_180:
    case RR_Rotate_270:
	break;
    default:
	/*
	 * Invalid rotation
	 */
	client->errorValue = stuff->rotation;
	if (outputs)
	    xfree (outputs);
	return BadValue;
    }

    if (mode)
    {
	if ((~crtc->rotations) & rotation)
	{
	    /*
	     * requested rotation or reflection not supported by screen
	     */
	    client->errorValue = stuff->rotation;
	    if (outputs)
		xfree (outputs);
	    return BadMatch;
	}
    
#ifdef RANDR_12_INTERFACE
	/*
	 * Check screen size bounds if the DDX provides a 1.2 interface
	 * for setting screen size. Else, assume the CrtcSet sets
	 * the size along with the mode. If the driver supports transforms,
	 * then it must allow crtcs to display a subset of the screen, so
	 * only do this check for drivers without transform support.
	 */
	if (pScrPriv->rrScreenSetSize && !crtc->transforms)
	{
	    int source_width;
	    int	source_height;
	    PictTransform transform;
	    struct pixman_f_transform f_transform, f_inverse;

	    RRTransformCompute (stuff->x, stuff->y,
				mode->mode.width, mode->mode.height,
				rotation,
				&crtc->client_pending_transform,
				&transform, &f_transform, &f_inverse);

	    RRModeGetScanoutSize (mode, &transform, &source_width, &source_height);
	    if (stuff->x + source_width > pScreen->width)
	    {
		client->errorValue = stuff->x;
		if (outputs)
		    xfree (outputs);
		return BadValue;
	    }
	    
	    if (stuff->y + source_height > pScreen->height)
	    {
		client->errorValue = stuff->y;
		if (outputs)
		    xfree (outputs);
		return BadValue;
	    }
	}
#endif
    }
    
    /*
     * Make sure the requested set-time is not older than
     * the last set-time
     */
    if (CompareTimeStamps (time, pScrPriv->lastSetTime) < 0)
    {
	rep.status = RRSetConfigInvalidTime;
	goto sendReply;
    }

    if (!RRCrtcSet (crtc, mode, stuff->x, stuff->y,
		   rotation, numOutputs, outputs))
    {
	rep.status = RRSetConfigFailed;
	goto sendReply;
    }
    rep.status = RRSetConfigSuccess;
    pScrPriv->lastSetTime = time;
    
sendReply:
    if (outputs)
	xfree (outputs);
    
    rep.type = X_Reply;
    /* rep.status has already been filled in */
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.newTimestamp = pScrPriv->lastSetTime.milliseconds;

    if (client->swapped) 
    {
	int n;
    	swaps(&rep.sequenceNumber, n);
    	swapl(&rep.length, n);
	swapl(&rep.newTimestamp, n);
    }
    WriteToClient(client, sizeof(xRRSetCrtcConfigReply), (char *)&rep);
    
    return client->noClientException;
}

int
ProcRRGetPanning (ClientPtr client)
{
    REQUEST(xRRGetPanningReq);
    xRRGetPanningReply	rep;
    RRCrtcPtr		crtc;
    ScreenPtr		pScreen;
    rrScrPrivPtr	pScrPriv;
    BoxRec		total;
    BoxRec		tracking;
    INT16		border[4];
    int			n;
    
    REQUEST_SIZE_MATCH(xRRGetPanningReq);
    crtc = LookupCrtc(client, stuff->crtc, DixReadAccess);

    if (!crtc)
	return RRErrorBase + BadRRCrtc;

    /* All crtcs must be associated with screens before client
     * requests are processed
     */
    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);

    if (!pScrPriv)
	return RRErrorBase + BadRRCrtc;

    memset(&rep, 0, sizeof(rep));
    rep.type = X_Reply;
    rep.status = RRSetConfigSuccess;
    rep.sequenceNumber = client->sequence;
    rep.length = 1;
    rep.timestamp = pScrPriv->lastSetTime.milliseconds;

    if (pScrPriv->rrGetPanning &&
	pScrPriv->rrGetPanning (pScreen, crtc, &total, &tracking, border)) {
	rep.left          = total.x1;
	rep.top           = total.y1;
	rep.width         = total.x2 - total.x1;
	rep.height        = total.y2 - total.y1;
	rep.track_left    = tracking.x1;
	rep.track_top     = tracking.y1;
	rep.track_width   = tracking.x2 - tracking.x1;
	rep.track_height  = tracking.y2 - tracking.y1;
	rep.border_left   = border[0];
	rep.border_top    = border[1];
	rep.border_right  = border[2];
	rep.border_bottom = border[3];
    }

    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swaps(&rep.timestamp, n);
	swaps(&rep.left, n);
	swaps(&rep.top, n);
	swaps(&rep.width, n);
	swaps(&rep.height, n);
	swaps(&rep.track_left, n);
	swaps(&rep.track_top, n);
	swaps(&rep.track_width, n);
	swaps(&rep.track_height, n);
	swaps(&rep.border_left, n);
	swaps(&rep.border_top, n);
	swaps(&rep.border_right, n);
	swaps(&rep.border_bottom, n);
    }
    WriteToClient(client, sizeof(xRRGetPanningReply), (char *)&rep);
    return client->noClientException;
}

int
ProcRRSetPanning (ClientPtr client)
{
    REQUEST(xRRSetPanningReq);
    xRRSetPanningReply	rep;
    RRCrtcPtr		crtc;
    ScreenPtr		pScreen;
    rrScrPrivPtr	pScrPriv;
    TimeStamp		time;
    BoxRec		total;
    BoxRec		tracking;
    INT16		border[4];
    int			n;
    
    REQUEST_SIZE_MATCH(xRRSetPanningReq);
    crtc = LookupCrtc(client, stuff->crtc, DixReadAccess);

    if (!crtc)
	return RRErrorBase + BadRRCrtc;


    /* All crtcs must be associated with screens before client
     * requests are processed
     */
    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);

    if (!pScrPriv) {
	time = currentTime;
	rep.status = RRSetConfigFailed;
	goto sendReply;
    }
    
    time = ClientTimeToServerTime(stuff->timestamp);
    
    /*
     * Make sure the requested set-time is not older than
     * the last set-time
     */
    if (CompareTimeStamps (time, pScrPriv->lastSetTime) < 0)
    {
	rep.status = RRSetConfigInvalidTime;
	goto sendReply;
    }

    if (!pScrPriv->rrGetPanning)
	return RRErrorBase + BadRRCrtc;

    total.x1    = stuff->left;
    total.y1    = stuff->top;
    total.x2    = total.x1 + stuff->width;
    total.y2    = total.y1 + stuff->height;
    tracking.x1 = stuff->track_left;
    tracking.y1 = stuff->track_top;
    tracking.x2 = tracking.x1 + stuff->track_width;
    tracking.y2 = tracking.y1 + stuff->track_height;
    border[0]   = stuff->border_left;
    border[1]   = stuff->border_top;
    border[2]   = stuff->border_right;
    border[3]   = stuff->border_bottom;

    if (! pScrPriv->rrSetPanning (pScreen, crtc, &total, &tracking, border))
	return BadMatch;

    pScrPriv->lastSetTime = time;

    rep.status = RRSetConfigSuccess;

sendReply:
    rep.type = X_Reply;
    rep.sequenceNumber = client->sequence;
    rep.length = 0;
    rep.newTimestamp = pScrPriv->lastSetTime.milliseconds;

    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
	swaps(&rep.newTimestamp, n);
    }
    WriteToClient(client, sizeof(xRRSetPanningReply), (char *)&rep);
    return client->noClientException;
}

int
ProcRRGetCrtcGammaSize (ClientPtr client)
{
    REQUEST(xRRGetCrtcGammaSizeReq);
    xRRGetCrtcGammaSizeReply	reply;
    RRCrtcPtr			crtc;
    int				n;

    REQUEST_SIZE_MATCH(xRRGetCrtcGammaSizeReq);
    crtc = LookupCrtc (client, stuff->crtc, DixReadAccess);
    if (!crtc)
	return RRErrorBase + BadRRCrtc;
    
    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = 0;
    reply.size = crtc->gammaSize;
    if (client->swapped) {
	swaps (&reply.sequenceNumber, n);
	swapl (&reply.length, n);
	swaps (&reply.size, n);
    }
    WriteToClient (client, sizeof (xRRGetCrtcGammaSizeReply), (char *) &reply);
    return client->noClientException;
}

int
ProcRRGetCrtcGamma (ClientPtr client)
{
    REQUEST(xRRGetCrtcGammaReq);
    xRRGetCrtcGammaReply	reply;
    RRCrtcPtr			crtc;
    int				n;
    unsigned long		len;
    char			*extra = NULL;
    
    REQUEST_SIZE_MATCH(xRRGetCrtcGammaReq);
    crtc = LookupCrtc (client, stuff->crtc, DixReadAccess);
    if (!crtc)
	return RRErrorBase + BadRRCrtc;
    
    len = crtc->gammaSize * 3 * 2;
    
    if (crtc->gammaSize) {
	extra = xalloc(len);
	if (!extra)
	    return BadAlloc;
    }

    reply.type = X_Reply;
    reply.sequenceNumber = client->sequence;
    reply.length = (len + 3) >> 2;
    reply.size = crtc->gammaSize;
    if (client->swapped) {
	swaps (&reply.sequenceNumber, n);
	swapl (&reply.length, n);
	swaps (&reply.size, n);
    }
    WriteToClient (client, sizeof (xRRGetCrtcGammaReply), (char *) &reply);
    if (crtc->gammaSize)
    {
	memcpy(extra, crtc->gammaRed, len);
	client->pSwapReplyFunc = (ReplySwapPtr)CopySwap16Write;
	WriteSwappedDataToClient (client, len, extra);
	xfree(extra);
    }
    return client->noClientException;
}

int
ProcRRSetCrtcGamma (ClientPtr client)
{
    REQUEST(xRRSetCrtcGammaReq);
    RRCrtcPtr			crtc;
    unsigned long		len;
    CARD16			*red, *green, *blue;
    
    REQUEST_AT_LEAST_SIZE(xRRSetCrtcGammaReq);
    crtc = LookupCrtc (client, stuff->crtc, DixWriteAccess);
    if (!crtc)
	return RRErrorBase + BadRRCrtc;
    
    len = client->req_len - (sizeof (xRRSetCrtcGammaReq) >> 2);
    if (len < (stuff->size * 3 + 1) >> 1)
	return BadLength;

    if (stuff->size != crtc->gammaSize)
	return BadMatch;
    
    red = (CARD16 *) (stuff + 1);
    green = red + crtc->gammaSize;
    blue = green + crtc->gammaSize;
    
    RRCrtcGammaSet (crtc, red, green, blue);

    return Success;
}

/* Version 1.3 additions */

int
ProcRRSetCrtcTransform (ClientPtr client)
{
    REQUEST(xRRSetCrtcTransformReq);
    RRCrtcPtr		    crtc;
    PictTransform	    transform;
    struct pixman_f_transform f_transform, f_inverse;
    char		    *filter;
    int			    nbytes;
    xFixed		    *params;
    int			    nparams;

    REQUEST_AT_LEAST_SIZE(xRRSetCrtcTransformReq);
    crtc = LookupCrtc (client, stuff->crtc, DixWriteAccess);
    if (!crtc)
	return RRErrorBase + BadRRCrtc;

    PictTransform_from_xRenderTransform (&transform, &stuff->transform);
    pixman_f_transform_from_pixman_transform (&f_transform, &transform);
    if (!pixman_f_transform_invert (&f_inverse, &f_transform))
	return BadMatch;

    filter = (char *) (stuff + 1);
    nbytes = stuff->nbytesFilter;
    params = (xFixed *) (filter + ((nbytes + 3) & ~3));
    nparams = ((xFixed *) stuff + client->req_len) - params;
    if (nparams < 0)
	return BadLength;

    return RRCrtcTransformSet (crtc, &transform, &f_transform, &f_inverse,
			       filter, nbytes, params, nparams);
}


#define CrtcTransformExtra	(SIZEOF(xRRGetCrtcTransformReply) - 32)
				
static int
transform_filter_length (RRTransformPtr transform)
{
    int	nbytes, nparams;

    if (transform->filter == NULL)
	return 0;
    nbytes = strlen (transform->filter->name);
    nparams = transform->nparams;
    return ((nbytes + 3) & ~3) + (nparams * sizeof (xFixed));
}

static int
transform_filter_encode (ClientPtr client, char *output,
			 CARD16	*nbytesFilter,
			 CARD16	*nparamsFilter,
			 RRTransformPtr transform)
{
    int	    nbytes, nparams;
    int	    n;

    if (transform->filter == NULL) {
	*nbytesFilter = 0;
	*nparamsFilter = 0;
	return 0;
    }
    nbytes = strlen (transform->filter->name);
    nparams = transform->nparams;
    *nbytesFilter = nbytes;
    *nparamsFilter = nparams;
    memcpy (output, transform->filter->name, nbytes);
    while ((nbytes & 3) != 0)
	output[nbytes++] = 0;
    memcpy (output + nbytes, transform->params, nparams * sizeof (xFixed));
    if (client->swapped) {
	swaps (nbytesFilter, n);
	swaps (nparamsFilter, n);
	SwapLongs ((CARD32 *) (output + nbytes), nparams);
    }
    nbytes += nparams * sizeof (xFixed);
    return nbytes;
}

static void
transform_encode (ClientPtr client, xRenderTransform *wire, PictTransform *pict)
{
    xRenderTransform_from_PictTransform (wire, pict);
    if (client->swapped)
	SwapLongs ((CARD32 *) wire, sizeof (xRenderTransform) >> 2);
}

int
ProcRRGetCrtcTransform (ClientPtr client)
{
    REQUEST(xRRGetCrtcTransformReq);
    xRRGetCrtcTransformReply	*reply;
    RRCrtcPtr			crtc;
    int				n, nextra;
    RRTransformPtr		current, pending;
    char			*extra;

    REQUEST_SIZE_MATCH (xRRGetCrtcTransformReq);
    crtc = LookupCrtc (client, stuff->crtc, DixWriteAccess);
    if (!crtc)
	return RRErrorBase + BadRRCrtc;

    pending = &crtc->client_pending_transform;
    current = &crtc->client_current_transform;

    nextra = (transform_filter_length (pending) +
	      transform_filter_length (current));

    reply = xalloc (sizeof (xRRGetCrtcTransformReply) + nextra);
    if (!reply)
	return BadAlloc;

    extra = (char *) (reply + 1);
    reply->type = X_Reply;
    reply->sequenceNumber = client->sequence;
    reply->length = (CrtcTransformExtra + nextra) >> 2;

    reply->hasTransforms = crtc->transforms;

    transform_encode (client, &reply->pendingTransform, &pending->transform);
    extra += transform_filter_encode (client, extra,
				      &reply->pendingNbytesFilter,
				      &reply->pendingNparamsFilter,
				      pending);

    transform_encode (client, &reply->currentTransform, &current->transform);
    extra += transform_filter_encode (client, extra,
				      &reply->currentNbytesFilter,
				      &reply->currentNparamsFilter,
				      current);

    if (client->swapped) {
	swaps (&reply->sequenceNumber, n);
	swapl (&reply->length, n);
    }
    WriteToClient (client, sizeof (xRRGetCrtcTransformReply) + nextra, (char *) reply);
    xfree(reply);
    return client->noClientException;
}
