/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or The Open Group
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and The Open Group make no 
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
#include <X11/Xmd.h>
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mistruct.h"
#include "dix.h"
#include "cfbmskbits.h"
#include "mibstore.h"

Bool
cfbCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
    int	    d;
    DepthPtr	depths = pScreen->allowedDepths;

    for (d = 0; d < pScreen->numDepths; d++)
	xfree (depths[d].vids);
    xfree (depths);
    xfree (pScreen->visuals);
#ifdef CFB_NEED_SCREEN_PRIVATE
    xfree (dixLookupPrivate(&pScreen->devPrivates, cfbScreenPrivateKey));
#else
    xfree (pScreen->devPrivate);
#endif
    return TRUE;
}

static void DestroyColormapNoop(
        ColormapPtr pColormap)
{
    /* NOOP */
}

static void StoreColorsNoop(
        ColormapPtr pColormap,
        int ndef,
        xColorItem * pdef)
{
    /* NOOP */
}

Bool
cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    if (!cfbAllocatePrivates(pScreen, NULL))
	return FALSE;
    pScreen->defColormap = FakeClientID(0);
    /* let CreateDefColormap do whatever it wants for pixels */ 
    pScreen->blackPixel = pScreen->whitePixel = (Pixel) 0;
    pScreen->QueryBestSize = mfbQueryBestSizeWeak();
    /* SaveScreen */
    pScreen->GetImage = cfbGetImage;
    pScreen->GetSpans = cfbGetSpans;
    pScreen->CreateWindow = cfbCreateWindow;
    pScreen->DestroyWindow = cfbDestroyWindow;
    pScreen->PositionWindow = cfbPositionWindow;
    pScreen->ChangeWindowAttributes = cfbChangeWindowAttributes;
    pScreen->RealizeWindow = cfbMapWindow;
    pScreen->UnrealizeWindow = cfbUnmapWindow;
    pScreen->CopyWindow = cfbCopyWindow;
    pScreen->CreatePixmap = cfbCreatePixmap;
    pScreen->DestroyPixmap = cfbDestroyPixmap;
    pScreen->RealizeFont = mfbRealizeFontWeak();
    pScreen->UnrealizeFont = mfbUnrealizeFontWeak();
    pScreen->CreateGC = cfbCreateGC;
    pScreen->CreateColormap = cfbInitializeColormap;
    pScreen->DestroyColormap = DestroyColormapNoop;
    pScreen->InstallColormap = cfbInstallColormap;
    pScreen->UninstallColormap = cfbUninstallColormap;
    pScreen->ListInstalledColormaps = cfbListInstalledColormaps;
    pScreen->StoreColors = StoreColorsNoop;
    pScreen->ResolveColor = cfbResolveColor;
    pScreen->BitmapToRegion = mfbPixmapToRegionWeak();

    mfbRegisterCopyPlaneProc (pScreen, cfbCopyPlane);
    return TRUE;
}

#ifdef CFB_NEED_SCREEN_PRIVATE
Bool
cfbCreateScreenResources(pScreen)
    ScreenPtr pScreen;
{
    Bool retval;

    pointer oldDevPrivate = pScreen->devPrivate;
    pScreen->devPrivate = dixLookupPrivate(&pScreen->devPrivates,
					   cfbScreenPrivateKey);
    retval = miCreateScreenResources(pScreen);
    dixSetPrivate(&pScreen->devPrivates, cfbScreenPrivateKey,
		  pScreen->devPrivate);
    pScreen->devPrivate = oldDevPrivate;
    return retval;
}
#endif

Bool
cfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
#ifdef CFB_NEED_SCREEN_PRIVATE
    pointer oldDevPrivate;
#endif
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual;

    rootdepth = 0;
    if (!cfbInitVisuals (&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
			 &defaultVisual,((unsigned long)1<<(PSZ-1)), 8))
	return FALSE;
#ifdef CFB_NEED_SCREEN_PRIVATE
    oldDevPrivate = pScreen->devPrivate;
#endif
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals))
	return FALSE;
    /* overwrite miCloseScreen with our own */
    pScreen->CloseScreen = cfbCloseScreen;
#ifdef CFB_NEED_SCREEN_PRIVATE
    pScreen->CreateScreenResources = cfbCreateScreenResources;
    dixSetPrivate(&pScreen->devPrivates, cfbScreenPrivateKey,
		  pScreen->devPrivate);
    pScreen->devPrivate = oldDevPrivate;
#endif
    pScreen->GetScreenPixmap = cfbGetScreenPixmap;
    pScreen->SetScreenPixmap = cfbSetScreenPixmap;
    return TRUE;
}

/* dts * (inch/dot) * (25.4 mm / inch) = mm */
Bool
cfbScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width)
    register ScreenPtr pScreen;
    pointer pbits;		/* pointer to screen bitmap */
    int xsize, ysize;		/* in pixels */
    int dpix, dpiy;		/* dots per inch */
    int width;			/* pixel width of frame buffer */
{
    if (!cfbSetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, width))
	return FALSE;
    return cfbFinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width);
}

PixmapPtr
cfbGetScreenPixmap(pScreen)
    ScreenPtr pScreen;
{
#ifdef CFB_NEED_SCREEN_PRIVATE
    return (PixmapPtr)dixLookupPrivate(&pScreen->devPrivates,
				       cfbScreenPrivateKey);
#else
    return (PixmapPtr)pScreen->devPrivate;
#endif
}

void
cfbSetScreenPixmap(pPix)
    PixmapPtr pPix;
{
#ifdef CFB_NEED_SCREEN_PRIVATE
    if (pPix)
	dixSetPrivate(&pPix->drawable.pScreen->devPrivates,
		      cfbScreenPrivateKey, pPix);
#else
    if (pPix)
	pPix->drawable.pScreen->devPrivate = (pointer)pPix;
#endif
}
