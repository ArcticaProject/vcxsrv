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

static int
SProcRRQueryVersion (ClientPtr client)
{
    register int n;
    REQUEST(xRRQueryVersionReq);

    swaps(&stuff->length, n);
    swapl(&stuff->majorVersion, n);
    swapl(&stuff->minorVersion, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetScreenInfo (ClientPtr client)
{
    register int n;
    REQUEST(xRRGetScreenInfoReq);

    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRSetScreenConfig (ClientPtr client)
{
    register int n;
    REQUEST(xRRSetScreenConfigReq);

    if (RRClientKnowsRates (client))
    {
	REQUEST_SIZE_MATCH (xRRSetScreenConfigReq);
	swaps (&stuff->rate, n);
    }
    else
    {
	REQUEST_SIZE_MATCH (xRR1_0SetScreenConfigReq);
    }
    
    swaps(&stuff->length, n);
    swapl(&stuff->drawable, n);
    swapl(&stuff->timestamp, n);
    swaps(&stuff->sizeID, n);
    swaps(&stuff->rotation, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRSelectInput (ClientPtr client)
{
    register int n;
    REQUEST(xRRSelectInputReq);

    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swaps(&stuff->enable, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetScreenSizeRange (ClientPtr client)
{
    int n;
    REQUEST(xRRGetScreenSizeRangeReq);

    REQUEST_SIZE_MATCH(xRRGetScreenSizeRangeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRSetScreenSize (ClientPtr client)
{
    int n;
    REQUEST(xRRSetScreenSizeReq);

    REQUEST_SIZE_MATCH(xRRSetScreenSizeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swapl(&stuff->widthInMillimeters, n);
    swapl(&stuff->heightInMillimeters, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetScreenResources (ClientPtr client)
{
    int n;
    REQUEST(xRRGetScreenResourcesReq);

    REQUEST_SIZE_MATCH(xRRGetScreenResourcesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetOutputInfo (ClientPtr client)
{
    int n;
    REQUEST(xRRGetOutputInfoReq);

    REQUEST_SIZE_MATCH(xRRGetOutputInfoReq);
    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    swapl(&stuff->configTimestamp, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRListOutputProperties (ClientPtr client)
{
    int n;
    REQUEST(xRRListOutputPropertiesReq);

    REQUEST_SIZE_MATCH(xRRListOutputPropertiesReq);
    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRQueryOutputProperty (ClientPtr client)
{
    int n;
    REQUEST(xRRQueryOutputPropertyReq);

    REQUEST_SIZE_MATCH(xRRQueryOutputPropertyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    swapl(&stuff->property, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRConfigureOutputProperty (ClientPtr client)
{
    int n;
    REQUEST(xRRConfigureOutputPropertyReq);

    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    swapl(&stuff->property, n);
    SwapRestL(stuff);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRChangeOutputProperty (ClientPtr client)
{
    int n;
    REQUEST(xRRChangeOutputPropertyReq);

    REQUEST_AT_LEAST_SIZE (xRRChangeOutputPropertyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    swapl(&stuff->property, n);
    swapl(&stuff->type, n);
    swapl(&stuff->nUnits, n);
    switch(stuff->format) {
	case 8:
	    break;
	case 16:
	    SwapRestS(stuff);
	    break;
	case 32:
	    SwapRestL(stuff);
	    break;
	default:
	    client->errorValue = stuff->format;
	    return BadValue;
    }
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRDeleteOutputProperty (ClientPtr client)
{
    int n;
    REQUEST(xRRDeleteOutputPropertyReq);
    
    REQUEST_SIZE_MATCH(xRRDeleteOutputPropertyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    swapl(&stuff->property, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetOutputProperty (ClientPtr client)
{
    int n;
    REQUEST(xRRGetOutputPropertyReq);

    REQUEST_SIZE_MATCH(xRRGetOutputPropertyReq);
    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    swapl(&stuff->property, n);
    swapl(&stuff->type, n);
    swapl(&stuff->longOffset, n);
    swapl(&stuff->longLength, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRCreateMode (ClientPtr client)
{
    int n;
    xRRModeInfo *modeinfo;
    REQUEST(xRRCreateModeReq);

    REQUEST_AT_LEAST_SIZE(xRRCreateModeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);

    modeinfo = &stuff->modeInfo;
    swapl(&modeinfo->id, n);
    swaps(&modeinfo->width, n);
    swaps(&modeinfo->height, n);
    swapl(&modeinfo->dotClock, n);
    swaps(&modeinfo->hSyncStart, n);
    swaps(&modeinfo->hSyncEnd, n);
    swaps(&modeinfo->hTotal, n);
    swaps(&modeinfo->vSyncStart, n);
    swaps(&modeinfo->vSyncEnd, n);
    swaps(&modeinfo->vTotal, n);
    swaps(&modeinfo->nameLength, n);
    swapl(&modeinfo->modeFlags, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRDestroyMode (ClientPtr client)
{
    int n;
    REQUEST(xRRDestroyModeReq);

    REQUEST_SIZE_MATCH(xRRDestroyModeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->mode, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRAddOutputMode (ClientPtr client)
{
    int n;
    REQUEST(xRRAddOutputModeReq);

    REQUEST_SIZE_MATCH(xRRAddOutputModeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    swapl(&stuff->mode, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRDeleteOutputMode (ClientPtr client)
{
    int n;
    REQUEST(xRRDeleteOutputModeReq);

    REQUEST_SIZE_MATCH(xRRDeleteOutputModeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->output, n);
    swapl(&stuff->mode, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetCrtcInfo (ClientPtr client)
{
    int n;
    REQUEST(xRRGetCrtcInfoReq);

    REQUEST_SIZE_MATCH(xRRGetCrtcInfoReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    swapl(&stuff->configTimestamp, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRSetCrtcConfig (ClientPtr client)
{
    int n;
    REQUEST(xRRSetCrtcConfigReq);

    REQUEST_AT_LEAST_SIZE(xRRSetCrtcConfigReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    swapl(&stuff->timestamp, n);
    swapl(&stuff->configTimestamp, n);
    swaps(&stuff->x, n);
    swaps(&stuff->y, n);
    swapl(&stuff->mode, n);
    swaps(&stuff->rotation, n);
    SwapRestL(stuff);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetCrtcGammaSize (ClientPtr client)
{
    int n;
    REQUEST(xRRGetCrtcGammaSizeReq);

    REQUEST_SIZE_MATCH(xRRGetCrtcGammaSizeReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetCrtcGamma (ClientPtr client)
{
    int n;
    REQUEST(xRRGetCrtcGammaReq);

    REQUEST_SIZE_MATCH(xRRGetCrtcGammaReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRSetCrtcGamma (ClientPtr client)
{
    int n;
    REQUEST(xRRSetCrtcGammaReq);

    REQUEST_AT_LEAST_SIZE(xRRSetCrtcGammaReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    swaps(&stuff->size, n);
    SwapRestS(stuff);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRSetCrtcTransform (ClientPtr client)
{
    int n, nparams;
    char *filter;
    CARD32 *params;
    REQUEST(xRRSetCrtcTransformReq);

    REQUEST_AT_LEAST_SIZE(xRRSetCrtcTransformReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    SwapLongs((CARD32 *)&stuff->transform, (sizeof(xRenderTransform)) >> 2);
    swaps(&stuff->nbytesFilter, n);
    filter = (char *)(stuff + 1);
    params = (CARD32 *) (filter + ((stuff->nbytesFilter + 3) & ~3));
    nparams = ((CARD32 *) stuff + client->req_len) - params;
    if (nparams < 0)
	return BadLength;

    SwapLongs(params, nparams);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetCrtcTransform (ClientPtr client)
{
    int n;
    REQUEST(xRRGetCrtcTransformReq);

    REQUEST_SIZE_MATCH(xRRGetCrtcTransformReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRGetPanning (ClientPtr client)
{
    int n;
    REQUEST(xRRGetPanningReq);
    
    REQUEST_SIZE_MATCH(xRRGetPanningReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRSetPanning (ClientPtr client)
{
    int n;
    REQUEST(xRRSetPanningReq);
    
    REQUEST_SIZE_MATCH(xRRSetPanningReq);
    swaps(&stuff->length, n);
    swapl(&stuff->crtc, n);
    swapl(&stuff->timestamp, n);
    swaps(&stuff->left, n);
    swaps(&stuff->top, n);
    swaps(&stuff->width, n);
    swaps(&stuff->height, n);
    swaps(&stuff->track_left, n);
    swaps(&stuff->track_top, n);
    swaps(&stuff->track_width, n);
    swaps(&stuff->track_height, n);
    swaps(&stuff->border_left, n);
    swaps(&stuff->border_top, n);
    swaps(&stuff->border_right, n);
    swaps(&stuff->border_bottom, n);
    return (*ProcRandrVector[stuff->randrReqType]) (client);
}

static int
SProcRRSetOutputPrimary (ClientPtr client)
{
    int n;
    REQUEST(xRRSetOutputPrimaryReq);

    REQUEST_SIZE_MATCH(xRRSetOutputPrimaryReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    swapl(&stuff->output, n);
    return ProcRandrVector[stuff->randrReqType](client);
}

static int
SProcRRGetOutputPrimary (ClientPtr client)
{
    int n;
    REQUEST(xRRSetOutputPrimaryReq);

    REQUEST_SIZE_MATCH(xRRGetOutputPrimaryReq);
    swaps(&stuff->length, n);
    swapl(&stuff->window, n);
    return ProcRandrVector[stuff->randrReqType](client);
}

int (*SProcRandrVector[RRNumberRequests])(ClientPtr) = {
    SProcRRQueryVersion,	/* 0 */
/* we skip 1 to make old clients fail pretty immediately */
    NULL,			/* 1 SProcRandrOldGetScreenInfo */
/* V1.0 apps share the same set screen config request id */
    SProcRRSetScreenConfig,	/* 2 */
    NULL,			/* 3 SProcRandrOldScreenChangeSelectInput */
/* 3 used to be ScreenChangeSelectInput; deprecated */
    SProcRRSelectInput,		/* 4 */
    SProcRRGetScreenInfo,    	/* 5 */
/* V1.2 additions */
    SProcRRGetScreenSizeRange,	/* 6 */
    SProcRRSetScreenSize,	/* 7 */
    SProcRRGetScreenResources,	/* 8 */
    SProcRRGetOutputInfo,	/* 9 */
    SProcRRListOutputProperties,/* 10 */
    SProcRRQueryOutputProperty,	/* 11 */
    SProcRRConfigureOutputProperty,  /* 12 */
    SProcRRChangeOutputProperty,/* 13 */
    SProcRRDeleteOutputProperty,/* 14 */
    SProcRRGetOutputProperty,	/* 15 */
    SProcRRCreateMode,		/* 16 */
    SProcRRDestroyMode,		/* 17 */
    SProcRRAddOutputMode,	/* 18 */
    SProcRRDeleteOutputMode,	/* 19 */
    SProcRRGetCrtcInfo,		/* 20 */
    SProcRRSetCrtcConfig,	/* 21 */
    SProcRRGetCrtcGammaSize,	/* 22 */
    SProcRRGetCrtcGamma,	/* 23 */
    SProcRRSetCrtcGamma,	/* 24 */
/* V1.3 additions */
    SProcRRGetScreenResources,	/* 25 GetScreenResourcesCurrent */
    SProcRRSetCrtcTransform,	/* 26 */
    SProcRRGetCrtcTransform,	/* 27 */
    SProcRRGetPanning,		/* 28 */
    SProcRRSetPanning,		/* 29 */
    SProcRRSetOutputPrimary,	/* 30 */
    SProcRRGetOutputPrimary,	/* 31 */
};

