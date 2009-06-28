/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/


/**
 * This version of fbcmap.c is implemented in terms of mi functions.
 * These functions used to be in fbcmap.c and depended upon the symbol
 * XFree86Server being defined.
 */


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "fb.h"
#include "micmap.h"

int
fbListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps)
{
    return miListInstalledColormaps(pScreen, pmaps);
}

void
fbInstallColormap(ColormapPtr pmap)
{
    miInstallColormap(pmap);
}

void
fbUninstallColormap(ColormapPtr pmap)
{
    miUninstallColormap(pmap);
}

void
fbResolveColor(unsigned short   *pred,
	       unsigned short   *pgreen,
	       unsigned short   *pblue,
	       VisualPtr	pVisual)
{
    miResolveColor(pred, pgreen, pblue, pVisual);
}

Bool
fbInitializeColormap(ColormapPtr pmap)
{
    return miInitializeColormap(pmap);
}

int
fbExpandDirectColors (ColormapPtr   pmap,
		      int	    ndef,
		      xColorItem    *indefs,
		      xColorItem    *outdefs)
{
    return miExpandDirectColors(pmap, ndef, indefs, outdefs);
}

Bool
fbCreateDefColormap(ScreenPtr pScreen)
{
    return miCreateDefColormap(pScreen);
}

void
fbClearVisualTypes(void)
{
    miClearVisualTypes();
}

Bool
fbSetVisualTypes (int depth, int visuals, int bitsPerRGB)
{
    return miSetVisualTypes(depth, visuals, bitsPerRGB, -1);
}

Bool
fbSetVisualTypesAndMasks (int depth, int visuals, int bitsPerRGB,
                          Pixel redMask, Pixel greenMask, Pixel blueMask)
{
    return miSetVisualTypesAndMasks(depth, visuals, bitsPerRGB, -1,
                                    redMask, greenMask, blueMask);
}

/*
 * Given a list of formats for a screen, create a list
 * of visuals and depths for the screen which coorespond to
 * the set which can be used with this version of fb.
 */
Bool
fbInitVisuals (VisualPtr    *visualp, 
	       DepthPtr	    *depthp,
	       int	    *nvisualp,
	       int	    *ndepthp,
	       int	    *rootDepthp,
	       VisualID	    *defaultVisp,
	       unsigned long	sizes,
	       int	    bitsPerRGB)
{
    return miInitVisuals(visualp, depthp, nvisualp, ndepthp, rootDepthp,
			 defaultVisp, sizes, bitsPerRGB, -1);
}
