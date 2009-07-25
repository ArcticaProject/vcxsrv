/*
 * Copyright © 2006 Sun Microsystems
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Sun Microsystems not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Sun Microsystems makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Copyright © 2003 Keith Packard
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "compint.h"

static int CompScreenPrivateKeyIndex;
DevPrivateKey CompScreenPrivateKey = &CompScreenPrivateKeyIndex;
static int CompWindowPrivateKeyIndex;
DevPrivateKey CompWindowPrivateKey = &CompWindowPrivateKeyIndex;
static int CompSubwindowsPrivateKeyIndex;
DevPrivateKey CompSubwindowsPrivateKey = &CompSubwindowsPrivateKeyIndex;


static Bool
compCloseScreen (int index, ScreenPtr pScreen)
{
    CompScreenPtr   cs = GetCompScreen (pScreen);
    Bool	    ret;

    xfree (cs->alternateVisuals);

    pScreen->CloseScreen = cs->CloseScreen;
    pScreen->BlockHandler = cs->BlockHandler;
    pScreen->InstallColormap = cs->InstallColormap;
    pScreen->ChangeWindowAttributes = cs->ChangeWindowAttributes;
    pScreen->ReparentWindow = cs->ReparentWindow;
    pScreen->MoveWindow = cs->MoveWindow;
    pScreen->ResizeWindow = cs->ResizeWindow;
    pScreen->ChangeBorderWidth = cs->ChangeBorderWidth;
    
    pScreen->ClipNotify = cs->ClipNotify;
    pScreen->UnrealizeWindow = cs->UnrealizeWindow;
    pScreen->RealizeWindow = cs->RealizeWindow;
    pScreen->DestroyWindow = cs->DestroyWindow;
    pScreen->CreateWindow = cs->CreateWindow;
    pScreen->CopyWindow = cs->CopyWindow;
    pScreen->PositionWindow = cs->PositionWindow;

    xfree (cs);
    dixSetPrivate(&pScreen->devPrivates, CompScreenPrivateKey, NULL);
    ret = (*pScreen->CloseScreen) (index, pScreen);

    return ret;
}

static void
compInstallColormap (ColormapPtr pColormap)
{
    VisualPtr	    pVisual = pColormap->pVisual;
    ScreenPtr	    pScreen = pColormap->pScreen;
    CompScreenPtr   cs = GetCompScreen (pScreen);
    int		    a;

    for (a = 0; a < cs->numAlternateVisuals; a++)
	if (pVisual->vid == cs->alternateVisuals[a])
	    return;
    pScreen->InstallColormap = cs->InstallColormap;
    (*pScreen->InstallColormap) (pColormap);
    cs->InstallColormap = pScreen->InstallColormap;
    pScreen->InstallColormap = compInstallColormap;
}

/* Fake backing store via automatic redirection */
static Bool
compChangeWindowAttributes(WindowPtr pWin, unsigned long mask)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    CompScreenPtr cs = GetCompScreen (pScreen);
    Bool ret;

    pScreen->ChangeWindowAttributes = cs->ChangeWindowAttributes;
    ret = pScreen->ChangeWindowAttributes(pWin, mask);

    if (ret && (mask & CWBackingStore) &&
	    pScreen->backingStoreSupport != NotUseful) {
	if (pWin->backingStore != NotUseful) {
	    compRedirectWindow(serverClient, pWin, CompositeRedirectAutomatic);
	    pWin->backStorage = (pointer) (intptr_t) 1;
	} else {
	    compUnredirectWindow(serverClient, pWin,
				 CompositeRedirectAutomatic);
	    pWin->backStorage = NULL;
	}
    }

    pScreen->ChangeWindowAttributes = compChangeWindowAttributes;

    return ret;
}

static void
compScreenUpdate (ScreenPtr pScreen)
{
    CompScreenPtr   cs = GetCompScreen (pScreen);

    compCheckTree (pScreen);
    if (cs->damaged)
    {
	compWindowUpdate (WindowTable[pScreen->myNum]);
	cs->damaged = FALSE;
    }
}

static void
compBlockHandler (int	    i,
		  pointer   blockData,
		  pointer   pTimeout,
		  pointer   pReadmask)
{
    ScreenPtr	    pScreen = screenInfo.screens[i];
    CompScreenPtr   cs = GetCompScreen (pScreen);

    pScreen->BlockHandler = cs->BlockHandler;
    compScreenUpdate (pScreen);
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);
    cs->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = compBlockHandler;
}

/*
 * Add alternate visuals -- always expose an ARGB32 and RGB24 visual
 */

static DepthPtr
compFindVisuallessDepth (ScreenPtr pScreen, int d)
{
    int		i;

    for (i = 0; i < pScreen->numDepths; i++)
    {
	DepthPtr    depth = &pScreen->allowedDepths[i];
	if (depth->depth == d)
	{
	    /*
	     * Make sure it doesn't have visuals already
	     */
	    if (depth->numVids)
		return 0;
	    /*
	     * looks fine
	     */
	    return depth;
	}
    }
    /*
     * If there isn't one, then it's gonna be hard to have 
     * an associated visual
     */
    return 0;
}

/*
 * Add a list of visual IDs to the list of visuals to implicitly redirect.
 */
static Bool
compRegisterAlternateVisuals (CompScreenPtr cs, VisualID *vids, int nVisuals)
{
    VisualID *p;

    p = xrealloc(cs->alternateVisuals,
		 sizeof(VisualID) * (cs->numAlternateVisuals + nVisuals));
    if(p == NULL)
	return FALSE;

    memcpy(&p[cs->numAlternateVisuals], vids, sizeof(VisualID) * nVisuals);

    cs->alternateVisuals = p;
    cs->numAlternateVisuals += nVisuals;

    return TRUE;
}

_X_EXPORT
Bool CompositeRegisterAlternateVisuals (ScreenPtr pScreen, VisualID *vids,
					int nVisuals)
{
    CompScreenPtr cs = GetCompScreen (pScreen);
    return compRegisterAlternateVisuals(cs, vids, nVisuals);
}

typedef struct _alternateVisual {
    int		depth;
    CARD32	format;
} CompAlternateVisual;

static CompAlternateVisual  altVisuals[] = {
#if COMP_INCLUDE_RGB24_VISUAL
    {	24,	PICT_r8g8b8 },
#endif
    {	32,	PICT_a8r8g8b8 },
};

static const int NUM_COMP_ALTERNATE_VISUALS = sizeof(altVisuals) /
					      sizeof(CompAlternateVisual);

static Bool
compAddAlternateVisual(ScreenPtr pScreen, CompScreenPtr cs,
		       CompAlternateVisual *alt)
{
    VisualPtr	    visual, visuals;
    int		    i;
    int		    numVisuals;
    XID		    *installedCmaps;
    ColormapPtr	    installedCmap;
    int		    numInstalledCmaps;
    DepthPtr	    depth;
    PictFormatPtr   pPictFormat;
    VisualID	    *vid;
    unsigned long   alphaMask;

    /*
     * The ARGB32 visual is always available.  Other alternate depth visuals
     * are only provided if their depth is less than the root window depth.
     * There's no deep reason for this.
     */
    if (alt->depth >= pScreen->rootDepth && alt->depth != 32)
	return FALSE;

    depth = compFindVisuallessDepth (pScreen, alt->depth);
    if (!depth)
	/* alt->depth doesn't exist or already has alternate visuals. */
	return TRUE;

    pPictFormat = PictureMatchFormat (pScreen, alt->depth, alt->format);
    if (!pPictFormat)
	return FALSE;

    vid = xalloc(sizeof(VisualID));
    if (!vid)
	return FALSE;

    /* Find the installed colormaps */
    installedCmaps = xalloc (pScreen->maxInstalledCmaps * sizeof (XID));
    if (!installedCmaps) {
	xfree(vid);
	return FALSE;
    }
    numInstalledCmaps = pScreen->ListInstalledColormaps(pScreen, 
	    installedCmaps);

    /* realloc the visual array to fit the new one in place */
    numVisuals = pScreen->numVisuals;
    visuals = xrealloc(pScreen->visuals, (numVisuals + 1) * sizeof(VisualRec));
    if (!visuals) {
	xfree(vid);
	xfree(installedCmaps);
	return FALSE;
    }

    /*
     * Fix up any existing installed colormaps -- we'll assume that
     * the only ones created so far have been installed.  If this
     * isn't true, we'll have to walk the resource database looking
     * for all colormaps.
     */
    for (i = 0; i < numInstalledCmaps; i++) {
	int j;

	installedCmap = LookupIDByType (installedCmaps[i], RT_COLORMAP);
	if (!installedCmap)
	    continue;
	j = installedCmap->pVisual - pScreen->visuals;
	installedCmap->pVisual = &visuals[j];
    }

    xfree(installedCmaps);

    pScreen->visuals = visuals;
    visual = visuals + pScreen->numVisuals; /* the new one */
    pScreen->numVisuals++;

    /* Initialize the visual */
    visual->vid = FakeClientID (0);
    visual->bitsPerRGBValue = 8;
    if (PICT_FORMAT_TYPE(alt->format) == PICT_TYPE_COLOR) {
	visual->class = PseudoColor;
	visual->nplanes = PICT_FORMAT_BPP(alt->format);
	visual->ColormapEntries = 1 << visual->nplanes;
    } else {
	DirectFormatRec *direct = &pPictFormat->direct;
	visual->class = TrueColor;
	visual->redMask   = ((unsigned long)direct->redMask) << direct->red;
	visual->greenMask = ((unsigned long)direct->greenMask) << direct->green;
	visual->blueMask  = ((unsigned long)direct->blueMask) << direct->blue;
	alphaMask = ((unsigned long)direct->alphaMask) << direct->alpha;
	visual->offsetRed   = direct->red;
	visual->offsetGreen = direct->green;
	visual->offsetBlue  = direct->blue;
	/*
	 * Include A bits in this (unlike GLX which includes only RGB)
	 * This lets DIX compute suitable masks for colormap allocations
	 */
	visual->nplanes = Ones (visual->redMask |
		visual->greenMask |
		visual->blueMask |
		alphaMask);
	/* find widest component */
	visual->ColormapEntries = (1 << max (Ones (visual->redMask),
		    max (Ones (visual->greenMask),
			Ones (visual->blueMask))));
    }

    /* remember the visual ID to detect auto-update windows */
    compRegisterAlternateVisuals(cs, &visual->vid, 1);

    /* Fix up the depth */
    *vid = visual->vid;
    depth->numVids = 1;
    depth->vids = vid;
    return TRUE;
}

static Bool
compAddAlternateVisuals (ScreenPtr pScreen, CompScreenPtr cs)
{
    int alt, ret = 0;

    for (alt = 0; alt < NUM_COMP_ALTERNATE_VISUALS; alt++)
	ret |= compAddAlternateVisual(pScreen, cs, altVisuals + alt);

    return !!ret;
}

Bool
compScreenInit (ScreenPtr pScreen)
{
    CompScreenPtr   cs;

    if (GetCompScreen (pScreen))
	return TRUE;
    cs = (CompScreenPtr) xalloc (sizeof (CompScreenRec));
    if (!cs)
	return FALSE;

    cs->damaged = FALSE;
    cs->overlayWid = FakeClientID(0);
    cs->pOverlayWin = NULL;
    cs->pOverlayClients = NULL;

    cs->numAlternateVisuals = 0;
    cs->alternateVisuals = NULL;

    if (!compAddAlternateVisuals (pScreen, cs))
    {
	xfree (cs);
	return FALSE;
    }

    cs->PositionWindow = pScreen->PositionWindow;
    pScreen->PositionWindow = compPositionWindow;

    cs->CopyWindow = pScreen->CopyWindow;
    pScreen->CopyWindow = compCopyWindow;

    cs->CreateWindow = pScreen->CreateWindow;
    pScreen->CreateWindow = compCreateWindow;

    cs->DestroyWindow = pScreen->DestroyWindow;
    pScreen->DestroyWindow = compDestroyWindow;

    cs->RealizeWindow = pScreen->RealizeWindow;
    pScreen->RealizeWindow = compRealizeWindow;

    cs->UnrealizeWindow = pScreen->UnrealizeWindow;
    pScreen->UnrealizeWindow = compUnrealizeWindow;

    cs->ClipNotify = pScreen->ClipNotify;
    pScreen->ClipNotify = compClipNotify;

    cs->MoveWindow = pScreen->MoveWindow;
    pScreen->MoveWindow = compMoveWindow;

    cs->ResizeWindow = pScreen->ResizeWindow;
    pScreen->ResizeWindow = compResizeWindow;

    cs->ChangeBorderWidth = pScreen->ChangeBorderWidth;
    pScreen->ChangeBorderWidth = compChangeBorderWidth;

    cs->ReparentWindow = pScreen->ReparentWindow;
    pScreen->ReparentWindow = compReparentWindow;

    cs->InstallColormap = pScreen->InstallColormap;
    pScreen->InstallColormap = compInstallColormap;

    cs->ChangeWindowAttributes = pScreen->ChangeWindowAttributes;
    pScreen->ChangeWindowAttributes = compChangeWindowAttributes;

    cs->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = compBlockHandler;

    cs->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = compCloseScreen;

    dixSetPrivate(&pScreen->devPrivates, CompScreenPrivateKey, cs);

    RegisterRealChildHeadProc(CompositeRealChildHead);

    return TRUE;
}
