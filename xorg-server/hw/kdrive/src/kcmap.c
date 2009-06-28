/*
 * Copyright © 1999 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"

/*
 * Put the entire colormap into the DAC
 */

void
KdSetColormap (ScreenPtr pScreen, int fb)
{
    KdScreenPriv(pScreen);
    ColormapPtr	pCmap = pScreenPriv->pInstalledmap[fb];
    Pixel	pixels[KD_MAX_PSEUDO_SIZE];
    xrgb	colors[KD_MAX_PSEUDO_SIZE];
    xColorItem	defs[KD_MAX_PSEUDO_SIZE];
    int		i;

    if (!pScreenPriv->card->cfuncs->putColors)
	return;
    if (pScreenPriv->screen->fb[fb].depth > KD_MAX_PSEUDO_DEPTH)
	return;
    
    if (!pScreenPriv->enabled)
	return;
    
    if (!pCmap)
	return;

    /*
     * Make DIX convert pixels into RGB values -- this handles
     * true/direct as well as pseudo/static visuals
     */
    
    for (i = 0; i < (1 << pScreenPriv->screen->fb[fb].depth); i++)
	pixels[i] = i;

    QueryColors (pCmap, (1 << pScreenPriv->screen->fb[fb].depth), pixels, colors);
    
    for (i = 0; i < (1 << pScreenPriv->screen->fb[fb].depth); i++)
    {
	defs[i].pixel = i;
	defs[i].red = colors[i].red;
	defs[i].green = colors[i].green;
	defs[i].blue = colors[i].blue;
	defs[i].flags = DoRed|DoGreen|DoBlue;
    }

    (*pScreenPriv->card->cfuncs->putColors) (pCmap->pScreen, fb,
					     (1 << pScreenPriv->screen->fb[fb].depth),
					     defs);

    /* recolor hardware cursor */
    if (pScreenPriv->card->cfuncs->recolorCursor)
	(*pScreenPriv->card->cfuncs->recolorCursor) (pCmap->pScreen, 0, 0);
}

/*
 * When the hardware is enabled, save the hardware colors and store
 * the current colormap
 */
void
KdEnableColormap (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    int	    i;
    int	    fb;
    Bool    done = FALSE;

    if (!pScreenPriv->card->cfuncs->putColors)
	return;
    for (fb = 0; fb < KD_MAX_FB && pScreenPriv->screen->fb[fb].depth; fb++)
    {
	if (pScreenPriv->screen->fb[fb].depth <= KD_MAX_PSEUDO_DEPTH && !done)
	{
	    for (i = 0; i < (1 << pScreenPriv->screen->fb[fb].depth); i++)
		pScreenPriv->systemPalette[i].pixel = i;
	    (*pScreenPriv->card->cfuncs->getColors) (pScreen, fb,
						   (1 << pScreenPriv->screen->fb[fb].depth),
						   pScreenPriv->systemPalette);
	    done = TRUE;
	}
	KdSetColormap (pScreen, fb);
    }
}

void
KdDisableColormap (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    int	    fb;

    if (!pScreenPriv->card->cfuncs->putColors)
	return;
    for (fb = 0; fb < KD_MAX_FB && pScreenPriv->screen->fb[fb].depth; fb++)
    {
	if (pScreenPriv->screen->fb[fb].depth <= KD_MAX_PSEUDO_DEPTH)
	{
	    (*pScreenPriv->card->cfuncs->putColors) (pScreen, fb,
						     (1 << pScreenPriv->screen->fb[fb].depth),
						     pScreenPriv->systemPalette);
	    break;
	}
    }
}

static int
KdColormapFb (ColormapPtr   pCmap)
{
    ScreenPtr	pScreen = pCmap->pScreen;
    KdScreenPriv (pScreen);
    KdScreenInfo    *screen = pScreenPriv->screen;
    int		d;
    DepthPtr	depth;
    int		v;
    VisualID	vid = pCmap->pVisual->vid;
    int		fb;

    if (screen->fb[1].depth)
    {
	for (d = 0; d < pScreen->numDepths; d++)
	{
	    depth = &pScreen->allowedDepths[d];
	    for (v = 0; v < depth->numVids; v++)
	    {
		if (depth->vids[v] == vid)
		{
		    for (fb = 0; fb < KD_MAX_FB && screen->fb[fb].depth; fb++)
		    {
			if (depth->depth == screen->fb[fb].depth)
			    return fb;
		    }
		}
	    }
	}
    }
    return 0;
}

/*
 * KdInstallColormap
 *
 * This function is called when the server receives a request to install a
 * colormap or when the server needs to install one on its own, like when
 * there's no window manager running and the user has moved the pointer over
 * an X client window.  It needs to build an identity Windows palette for the
 * colormap and realize it into the Windows system palette.
 */
void
KdInstallColormap (ColormapPtr pCmap)
{
    KdScreenPriv(pCmap->pScreen);
    int		fb = KdColormapFb (pCmap);

    if (pCmap == pScreenPriv->pInstalledmap[fb])
	return;

    /* Tell X clients that the installed colormap is going away. */
    if (pScreenPriv->pInstalledmap[fb])
	WalkTree(pScreenPriv->pInstalledmap[fb]->pScreen, TellLostMap,
		 (pointer) &(pScreenPriv->pInstalledmap[fb]->mid));

    /* Take note of the new installed colorscreen-> */
    pScreenPriv->pInstalledmap[fb] = pCmap;

    KdSetColormap (pCmap->pScreen, fb);
    
    /* Tell X clients of the new colormap */
    WalkTree(pCmap->pScreen, TellGainedMap, (pointer) &(pCmap->mid));
}

/*
 * KdUninstallColormap
 *
 * This function uninstalls a colormap by either installing 
 * the default X colormap or erasing the installed colormap pointer.
 * The default X colormap itself cannot be uninstalled.
 */
void
KdUninstallColormap (ColormapPtr pCmap)
{
    KdScreenPriv(pCmap->pScreen);
    int		fb = KdColormapFb (pCmap);
    Colormap	defMapID;
    ColormapPtr defMap;

    /* ignore if not installed */
    if (pCmap != pScreenPriv->pInstalledmap[fb])
	return;

    /* ignore attempts to uninstall default colormap */
    defMapID = pCmap->pScreen->defColormap;
    if ((Colormap) pCmap->mid == defMapID)
	return;

    /* install default if on same fb */
    defMap = (ColormapPtr) LookupIDByType(defMapID, RT_COLORMAP);
    if (defMap && KdColormapFb (defMap) == fb)
	(*pCmap->pScreen->InstallColormap)(defMap);
    else
    {
	/* uninstall and clear colormap pointer */
	WalkTree(pCmap->pScreen, TellLostMap,
		 (pointer) &(pCmap->mid));
	pScreenPriv->pInstalledmap[fb] = 0;
    }
}

int
KdListInstalledColormaps (ScreenPtr pScreen, Colormap *pCmaps)
{
    KdScreenPriv(pScreen);
    int		fb;
    int		n = 0;
    
    for (fb = 0; fb < KD_MAX_FB && pScreenPriv->screen->fb[fb].depth; fb++)
    {
	if (pScreenPriv->pInstalledmap[fb])
	{
	    *pCmaps++ = pScreenPriv->pInstalledmap[fb]->mid;
	    n++;
	}
    }
    return n;
}

/*
 * KdStoreColors
 *
 * This function is called whenever the server receives a request to store
 * color values into one or more entries in the currently installed X
 * colormap; it can be either the default colormap or a private colorscreen->
 */
void
KdStoreColors (ColormapPtr pCmap, int ndef, xColorItem *pdefs)
{
    KdScreenPriv(pCmap->pScreen);
    VisualPtr           pVisual;
    xColorItem          expanddefs[KD_MAX_PSEUDO_SIZE];
    int			fb = KdColormapFb (pCmap);

    if (pCmap != pScreenPriv->pInstalledmap[fb])
	return;
    
    if (!pScreenPriv->card->cfuncs->putColors)
	return;
    
    if (pScreenPriv->screen->fb[fb].depth > KD_MAX_PSEUDO_DEPTH)
	return;
    
    if (!pScreenPriv->enabled)
	return;
    
    /* Check for DirectColor or TrueColor being simulated on a PseudoColor device. */
    pVisual = pCmap->pVisual;
    if ((pVisual->class | DynamicClass) == DirectColor)
    {
	/*
	 * Expand DirectColor or TrueColor color values into a PseudoColor
	 * format.  Defer to the Color Framebuffer (CFB) code to do that.
	 */
	ndef = fbExpandDirectColors(pCmap, ndef, pdefs, expanddefs);
	pdefs = expanddefs;
    }

    (*pScreenPriv->card->cfuncs->putColors) (pCmap->pScreen, fb, ndef, pdefs);
    
    /* recolor hardware cursor */
    if (pScreenPriv->card->cfuncs->recolorCursor)
	(*pScreenPriv->card->cfuncs->recolorCursor) (pCmap->pScreen, ndef, pdefs);
}
