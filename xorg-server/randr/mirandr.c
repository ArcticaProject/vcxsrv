/*
 * Copyright © 2000 Compaq Computer Corporation
 * Copyright © 2002 Hewlett-Packard Company
 * Copyright © 2006 Intel Corporation
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
 *
 * Author:  Jim Gettys, Hewlett-Packard Company, Inc.
 *	    Keith Packard, Intel Corporation
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "scrnintstr.h"
#include "mi.h"
#include "randrstr.h"
#include <stdio.h>

Bool
miRRGetInfo (ScreenPtr pScreen, Rotation *rotations)
{
    return TRUE;
}

/*
 * Any hardware that can actually change anything will need something
 * different here
 */
Bool
miRRCrtcSet (ScreenPtr	pScreen,
	     RRCrtcPtr	crtc,
	     RRModePtr	mode,
	     int	x,
	     int	y,
	     Rotation	rotation,
	     int	numOutput,
	     RROutputPtr *outputs)
{
    return TRUE;
}

static Bool
miRRCrtcSetGamma (ScreenPtr	pScreen,
		  RRCrtcPtr	crtc)
{
    return TRUE;
}

Bool
miRROutputSetProperty (ScreenPtr	    pScreen,
		       RROutputPtr	    output,
		       Atom		    property,
		       RRPropertyValuePtr   value)
{
    return TRUE;
}

Bool
miRROutputValidateMode (ScreenPtr	    pScreen,
			RROutputPtr	    output,
			RRModePtr	    mode)
{
    return FALSE;
}

void
miRRModeDestroy (ScreenPtr  pScreen,
		 RRModePtr  mode)
{
}

/*
 * This function assumes that only a single depth can be
 * displayed at a time, but that all visuals of that depth
 * can be displayed simultaneously.  It further assumes that
 * only a single size is available.  Hardware providing
 * additional capabilties should use different code.
 * XXX what to do here....
 */

Bool
miRandRInit (ScreenPtr pScreen)
{
    rrScrPrivPtr    pScrPriv;
#if RANDR_12_INTERFACE
    RRModePtr	mode;
    RRCrtcPtr	crtc;
    RROutputPtr	output;
    xRRModeInfo modeInfo;
    char	name[64];
#endif
    
    if (!RRScreenInit (pScreen))
	return FALSE;
    pScrPriv = rrGetScrPriv(pScreen);
    pScrPriv->rrGetInfo = miRRGetInfo;
#if RANDR_12_INTERFACE
    pScrPriv->rrCrtcSet = miRRCrtcSet;
    pScrPriv->rrCrtcSetGamma = miRRCrtcSetGamma;
    pScrPriv->rrOutputSetProperty = miRROutputSetProperty;
    pScrPriv->rrOutputValidateMode = miRROutputValidateMode;
    pScrPriv->rrModeDestroy = miRRModeDestroy;
    
    RRScreenSetSizeRange (pScreen,
			  pScreen->width, pScreen->height,
			  pScreen->width, pScreen->height);

    sprintf (name, "%dx%d", pScreen->width, pScreen->height);
    memset (&modeInfo, '\0', sizeof (modeInfo));
    modeInfo.width = pScreen->width;
    modeInfo.height = pScreen->height;
    modeInfo.nameLength = strlen (name);
    
    mode = RRModeGet (&modeInfo, name);
    if (!mode)
	return FALSE;
    
    crtc = RRCrtcCreate (pScreen, NULL);
    if (!crtc)
	return FALSE;
    
    output = RROutputCreate (pScreen, "screen", 6, NULL);
    if (!output)
	return FALSE;
    if (!RROutputSetClones (output, NULL, 0))
	return FALSE;
    if (!RROutputSetModes (output, &mode, 1, 0))
	return FALSE;
    if (!RROutputSetCrtcs (output, &crtc, 1))
	return FALSE;
    if (!RROutputSetConnection (output, RR_Connected))
	return FALSE;
    RRCrtcNotify (crtc, mode, 0, 0, RR_Rotate_0, 1, &output);
#endif
    return TRUE;
}
