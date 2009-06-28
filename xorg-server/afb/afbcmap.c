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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "scrnintstr.h"
#include "colormapst.h"
#include "resource.h"
#include "micmap.h"
#include "afb.h"

int
afbListInstalledColormaps(ScreenPtr pScreen, Colormap *pmaps)
{
	return miListInstalledColormaps(pScreen, pmaps);
}


void
afbInstallColormap(ColormapPtr pmap)
{
	miInstallColormap(pmap);
}

void
afbUninstallColormap(ColormapPtr pmap)
{
	miUninstallColormap(pmap);
}

void
afbResolveColor(short unsigned int *pred, short unsigned int *pgreen, short unsigned int *pblue, register VisualPtr pVisual)
{
	miResolveColor(pred, pgreen, pblue, pVisual);
}

Bool
afbInitializeColormap(register ColormapPtr pmap)
{
	return miInitializeColormap(pmap);
}

/*
 * Given a list of formats for a screen, create a list
 * of visuals and depths for the screen which correspond to
 * the set which can be used with this version of afb.
 */

Bool
afbInitVisuals(VisualPtr *visualp, DepthPtr *depthp, int *nvisualp, int *ndepthp, int *rootDepthp, VisualID *defaultVisp, long unsigned int sizes, int bitsPerRGB)
{
	return miInitVisuals(visualp, depthp, nvisualp, ndepthp, rootDepthp,
				defaultVisp, sizes, bitsPerRGB, -1);
}
