/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>

#include <X11/X.h>
#include <X11/Xproto.h>		/* for xColorItem */
#include <X11/Xmd.h>
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "resource.h"
#include "colormap.h"
#include "afb.h"
#include "mistruct.h"
#include "dix.h"
#include "mi.h"
#include "mibstore.h"
#include "migc.h"
#include "servermd.h"

#ifdef PIXMAP_PER_WINDOW
DevPrivateKey frameWindowPrivateKey = &frameWindowPrivateKey;
#endif
DevPrivateKey afbGCPrivateKey = &afbGCPrivateKey;
DevPrivateKey afbScreenPrivateKey = &afbScreenPrivateKey;

static Bool
afbCloseScreen(int index, ScreenPtr pScreen)
{
	int d;
	DepthPtr depths = pScreen->allowedDepths;

	for (d = 0; d < pScreen->numDepths; d++)
		xfree(depths[d].vids);
	xfree(depths);
	xfree(pScreen->visuals);
	xfree(dixLookupPrivate(&pScreen->devPrivates, afbScreenPrivateKey));
	return(TRUE);
}

static Bool
afbCreateScreenResources(ScreenPtr pScreen)
{
	Bool retval;

	pointer oldDevPrivate = pScreen->devPrivate;

	pScreen->devPrivate = dixLookupPrivate(&pScreen->devPrivates,
					       afbScreenPrivateKey);
	retval = miCreateScreenResources(pScreen);

	/* Modify screen's pixmap devKind value stored off devPrivate to
	 * be the width of a single plane in longs rather than the width
	 * of a chunky screen in longs as incorrectly setup by the mi routine.
	 */
	((PixmapPtr)pScreen->devPrivate)->devKind = BitmapBytePad(pScreen->width);
	dixSetPrivate(&pScreen->devPrivates, afbScreenPrivateKey,
		      pScreen->devPrivate);
	pScreen->devPrivate = oldDevPrivate;
	return(retval);
}

static PixmapPtr
afbGetWindowPixmap(WindowPtr pWin)
{
#ifdef PIXMAP_PER_WINDOW
    return (PixmapPtr)dixLookupPrivate(&pWin->devPrivates,
				       frameWindowPrivateKey);
#else
    ScreenPtr pScreen = pWin->drawable.pScreen;

    return (* pScreen->GetScreenPixmap)(pScreen);
#endif
}

static void
afbSetWindowPixmap(WindowPtr pWin, PixmapPtr pPix)
{
#ifdef PIXMAP_PER_WINDOW
    dixSetPrivate(&pWin->devPrivates, frameWindowPrivateKey, pPix);
#else
    (* pWin->drawable.pScreen->SetScreenPixmap)(pPix);
#endif
}

static Bool
afbAllocatePrivates(ScreenPtr pScreen, DevPrivateKey *pGCKey)
{
	if (pGCKey)
		*pGCKey = afbGCPrivateKey;

	pScreen->GetWindowPixmap = afbGetWindowPixmap;
	pScreen->SetWindowPixmap = afbSetWindowPixmap;
	return dixRequestPrivate(afbGCPrivateKey, sizeof(afbPrivGC));
}

/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
afbScreenInit(register ScreenPtr pScreen, pointer pbits, int xsize, int ysize, int dpix, int dpiy, int width)
	                           
	              			/* pointer to screen bitmap */
	                 		/* in pixels */
	               			/* dots per inch */
	          			/* pixel width of frame buffer */
{
	VisualPtr visuals;
	DepthPtr depths;
	int nvisuals;
	int ndepths;
	int rootdepth;
	VisualID defaultVisual;
	pointer oldDevPrivate;

	rootdepth = 0;
	if (!afbInitVisuals(&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
								&defaultVisual, 256, 8)) {
		ErrorF("afbInitVisuals: FALSE\n");
		return FALSE;
	}
	if (!afbAllocatePrivates(pScreen, NULL)) {
		ErrorF("afbAllocatePrivates: FALSE\n");
		return FALSE;
	}

	pScreen->defColormap = (Colormap)FakeClientID(0);
	/* whitePixel, blackPixel */
	pScreen->blackPixel = 0;
	pScreen->whitePixel = 0;
	pScreen->QueryBestSize = afbQueryBestSize;
	/* SaveScreen */
	pScreen->GetImage = afbGetImage;
	pScreen->GetSpans = afbGetSpans;
	pScreen->CreateWindow = afbCreateWindow;
	pScreen->DestroyWindow = afbDestroyWindow;
	pScreen->PositionWindow = afbPositionWindow;
	pScreen->ChangeWindowAttributes = afbChangeWindowAttributes;
	pScreen->RealizeWindow = afbMapWindow;
	pScreen->UnrealizeWindow = afbUnmapWindow;
	pScreen->CopyWindow = afbCopyWindow;
	pScreen->CreatePixmap = afbCreatePixmap;
	pScreen->DestroyPixmap = afbDestroyPixmap;
	pScreen->RealizeFont = afbRealizeFont;
	pScreen->UnrealizeFont = afbUnrealizeFont;
	pScreen->CreateGC = afbCreateGC;
	pScreen->CreateColormap = afbInitializeColormap;
	pScreen->DestroyColormap = (DestroyColormapProcPtr)NoopDDA;
	pScreen->InstallColormap = afbInstallColormap;
	pScreen->UninstallColormap = afbUninstallColormap;
	pScreen->ListInstalledColormaps = afbListInstalledColormaps;
	pScreen->StoreColors = (StoreColorsProcPtr)NoopDDA;
	pScreen->ResolveColor = afbResolveColor;
	pScreen->BitmapToRegion = afbPixmapToRegion;
	oldDevPrivate = pScreen->devPrivate;
	if (!miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width, rootdepth,
		ndepths, depths, defaultVisual, nvisuals, visuals)) {
		ErrorF("miScreenInit: FALSE\n");
		return FALSE;
	}

	pScreen->CloseScreen = afbCloseScreen;
	pScreen->CreateScreenResources = afbCreateScreenResources;

	dixSetPrivate(&pScreen->devPrivates, afbScreenPrivateKey,
		      pScreen->devPrivate);
	pScreen->devPrivate = oldDevPrivate;

	return TRUE;
}
