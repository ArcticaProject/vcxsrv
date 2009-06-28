
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"
#include "xf86CursorPriv.h"
#include "colormapst.h"
#include "cursorstr.h"

DevPrivateKey xf86CursorScreenKey = &xf86CursorScreenKey;

/* sprite functions */

static Bool xf86CursorRealizeCursor(ScreenPtr, CursorPtr);
static Bool xf86CursorUnrealizeCursor(ScreenPtr, CursorPtr);
static void xf86CursorSetCursor(ScreenPtr, CursorPtr, int, int);
static void xf86CursorMoveCursor(ScreenPtr, int, int);

static miPointerSpriteFuncRec xf86CursorSpriteFuncs = {
   xf86CursorRealizeCursor,
   xf86CursorUnrealizeCursor,
   xf86CursorSetCursor,
   xf86CursorMoveCursor
};

/* Screen functions */

static void xf86CursorInstallColormap(ColormapPtr);
static void xf86CursorRecolorCursor(ScreenPtr, CursorPtr, Bool);
static Bool xf86CursorCloseScreen(int, ScreenPtr);
static void xf86CursorQueryBestSize(int, unsigned short*, unsigned short*,
				    ScreenPtr);

/* ScrnInfoRec functions */

static void xf86CursorEnableDisableFBAccess(int, Bool);
static Bool xf86CursorSwitchMode(int, DisplayModePtr,int);

Bool
xf86InitCursor(
   ScreenPtr pScreen,
   xf86CursorInfoPtr infoPtr
)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    xf86CursorScreenPtr ScreenPriv;
    miPointerScreenPtr PointPriv;

    if (!xf86InitHardwareCursor(pScreen, infoPtr))
	return FALSE;

    ScreenPriv = xcalloc(1, sizeof(xf86CursorScreenRec));
    if (!ScreenPriv)
	return FALSE;

    dixSetPrivate(&pScreen->devPrivates, xf86CursorScreenKey, ScreenPriv);

    ScreenPriv->SWCursor = TRUE;
    ScreenPriv->isUp = FALSE;
    ScreenPriv->CurrentCursor = NULL;
    ScreenPriv->CursorInfoPtr = infoPtr;
    ScreenPriv->PalettedCursor = FALSE;
    ScreenPriv->pInstalledMap = NULL;

    ScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = xf86CursorCloseScreen;
    ScreenPriv->QueryBestSize = pScreen->QueryBestSize;
    pScreen->QueryBestSize = xf86CursorQueryBestSize;
    ScreenPriv->RecolorCursor = pScreen->RecolorCursor;
    pScreen->RecolorCursor = xf86CursorRecolorCursor;

    if ((infoPtr->pScrn->bitsPerPixel == 8) &&
	!(infoPtr->Flags & HARDWARE_CURSOR_TRUECOLOR_AT_8BPP)) {
	ScreenPriv->InstallColormap = pScreen->InstallColormap;
	pScreen->InstallColormap = xf86CursorInstallColormap;
	ScreenPriv->PalettedCursor = TRUE;
    }

    PointPriv = dixLookupPrivate(&pScreen->devPrivates, miPointerScreenKey);

    ScreenPriv->showTransparent = PointPriv->showTransparent;
    if (infoPtr->Flags & HARDWARE_CURSOR_SHOW_TRANSPARENT)
	PointPriv->showTransparent = TRUE;
    else
	PointPriv->showTransparent = FALSE;
    ScreenPriv->spriteFuncs = PointPriv->spriteFuncs;
    PointPriv->spriteFuncs = &xf86CursorSpriteFuncs;

    ScreenPriv->EnableDisableFBAccess = pScrn->EnableDisableFBAccess;
    ScreenPriv->SwitchMode = pScrn->SwitchMode;
    
    ScreenPriv->ForceHWCursorCount = 0;
    ScreenPriv->HWCursorForced = FALSE;

    pScrn->EnableDisableFBAccess = xf86CursorEnableDisableFBAccess;
    if (pScrn->SwitchMode)
	pScrn->SwitchMode = xf86CursorSwitchMode;

    return TRUE;
}

/***** Screen functions *****/

static Bool
xf86CursorCloseScreen(int i, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    miPointerScreenPtr PointPriv = (miPointerScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, miPointerScreenKey);
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);

    if (ScreenPriv->isUp && pScrn->vtSema)
	xf86SetCursor(pScreen, NullCursor, ScreenPriv->x, ScreenPriv->y);

    pScreen->CloseScreen = ScreenPriv->CloseScreen;
    pScreen->QueryBestSize = ScreenPriv->QueryBestSize;
    pScreen->RecolorCursor = ScreenPriv->RecolorCursor;
    if (ScreenPriv->InstallColormap)
	pScreen->InstallColormap = ScreenPriv->InstallColormap;

    PointPriv->spriteFuncs = ScreenPriv->spriteFuncs;
    PointPriv->showTransparent = ScreenPriv->showTransparent;

    pScrn->EnableDisableFBAccess = ScreenPriv->EnableDisableFBAccess;
    pScrn->SwitchMode = ScreenPriv->SwitchMode;

    xfree(ScreenPriv->transparentData);
    xfree(ScreenPriv);

    return (*pScreen->CloseScreen)(i, pScreen);
}

static void
xf86CursorQueryBestSize(
   int class,
   unsigned short *width,
   unsigned short *height,
   ScreenPtr pScreen)
{
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);

    if (class == CursorShape) {
	if(*width > ScreenPriv->CursorInfoPtr->MaxWidth)
	   *width = ScreenPriv->CursorInfoPtr->MaxWidth;
	if(*height > ScreenPriv->CursorInfoPtr->MaxHeight)
	   *height = ScreenPriv->CursorInfoPtr->MaxHeight;
    } else
	(*ScreenPriv->QueryBestSize)(class, width, height, pScreen);
}

static void
xf86CursorInstallColormap(ColormapPtr pMap)
{
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pMap->pScreen->devPrivates, xf86CursorScreenKey);

    ScreenPriv->pInstalledMap = pMap;

    (*ScreenPriv->InstallColormap)(pMap);
}

static void
xf86CursorRecolorCursor(
    ScreenPtr pScreen,
    CursorPtr pCurs,
    Bool displayed)
{
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);

    if (!displayed)
	return;

    if (ScreenPriv->SWCursor)
	(*ScreenPriv->RecolorCursor)(pScreen, pCurs, displayed);
    else
	xf86RecolorCursor(pScreen, pCurs, displayed);
}

/***** ScrnInfoRec functions *********/

static void
xf86CursorEnableDisableFBAccess(
    int index,
    Bool enable)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);

    if (!enable && ScreenPriv->CurrentCursor != NullCursor) {
	CursorPtr   currentCursor = ScreenPriv->CurrentCursor;
	xf86CursorSetCursor(pScreen, NullCursor, ScreenPriv->x, ScreenPriv->y);
	ScreenPriv->isUp = FALSE;
	ScreenPriv->SWCursor = TRUE;
	ScreenPriv->SavedCursor = currentCursor;
    }

    if (ScreenPriv->EnableDisableFBAccess)
	(*ScreenPriv->EnableDisableFBAccess)(index, enable);

    if (enable && ScreenPriv->SavedCursor)
    {
	/*
	 * Re-set current cursor so drivers can react to FB access having been
	 * temporarily disabled.
	 */
	xf86CursorSetCursor(pScreen, ScreenPriv->SavedCursor,
			    ScreenPriv->x, ScreenPriv->y);
	ScreenPriv->SavedCursor = NULL;
    }
}

static Bool
xf86CursorSwitchMode(int index, DisplayModePtr mode, int flags)
{
    Bool ret;
    ScreenPtr pScreen = screenInfo.screens[index];
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);
    miPointerScreenPtr PointPriv = (miPointerScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, miPointerScreenKey);

    if (ScreenPriv->isUp) {
	xf86SetCursor(pScreen, NullCursor, ScreenPriv->x, ScreenPriv->y);
	ScreenPriv->isUp = FALSE;
    }

    ret = (*ScreenPriv->SwitchMode)(index, mode, flags);

    /*
     * Cannot restore cursor here because the new frame[XY][01] haven't been
     * calculated yet.  However, because the hardware cursor was removed above,
     * ensure the cursor is repainted by miPointerWarpCursor().
     */
    ScreenPriv->CursorToRestore = ScreenPriv->CurrentCursor;
    PointPriv->waitForUpdate = FALSE;	/* Force cursor repaint */

    return ret;
}

/****** miPointerSpriteFunctions *******/

static Bool
xf86CursorRealizeCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);

    if (pCurs->refcnt <= 1)
	dixSetPrivate(&pCurs->devPrivates, pScreen, NULL);

    return (*ScreenPriv->spriteFuncs->RealizeCursor)(pScreen, pCurs);
}

static Bool
xf86CursorUnrealizeCursor(ScreenPtr pScreen, CursorPtr pCurs)
{
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);

    if (pCurs->refcnt <= 1) {
	xfree(dixLookupPrivate(&pCurs->devPrivates, pScreen));
	dixSetPrivate(&pCurs->devPrivates, pScreen, NULL);
    }

    return (*ScreenPriv->spriteFuncs->UnrealizeCursor)(pScreen, pCurs);
}

static void
xf86CursorSetCursor(ScreenPtr pScreen, CursorPtr pCurs, int x, int y)
{
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);
    xf86CursorInfoPtr infoPtr = ScreenPriv->CursorInfoPtr;
    miPointerScreenPtr PointPriv;

    ScreenPriv->CurrentCursor = pCurs;
    ScreenPriv->x = x;
    ScreenPriv->y = y;
    ScreenPriv->CursorToRestore = NULL;

    if (!infoPtr->pScrn->vtSema)
	 ScreenPriv->SavedCursor = pCurs;

    if (pCurs == NullCursor) {	/* means we're supposed to remove the cursor */
	if (ScreenPriv->SWCursor)
	    (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, NullCursor, x, y);
	else if (ScreenPriv->isUp) {
	    xf86SetCursor(pScreen, NullCursor, x, y);
	    ScreenPriv->isUp = FALSE;
	}
	return;
    }

    ScreenPriv->HotX = pCurs->bits->xhot;
    ScreenPriv->HotY = pCurs->bits->yhot;

    PointPriv = (miPointerScreenPtr)dixLookupPrivate(&pScreen->devPrivates,
						     miPointerScreenKey);
    if (infoPtr->pScrn->vtSema && (ScreenPriv->ForceHWCursorCount || ((
#ifdef ARGB_CURSOR
	pCurs->bits->argb && infoPtr->UseHWCursorARGB &&
	 (*infoPtr->UseHWCursorARGB) (pScreen, pCurs) ) || (
	pCurs->bits->argb == 0 &&
#endif
	(pCurs->bits->height <= infoPtr->MaxHeight) &&
	(pCurs->bits->width <= infoPtr->MaxWidth) &&
	(!infoPtr->UseHWCursor || (*infoPtr->UseHWCursor)(pScreen, pCurs))))))
    {

	if (ScreenPriv->SWCursor)	/* remove the SW cursor */
	      (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, NullCursor, x, y);

	xf86SetCursor(pScreen, pCurs, x, y);
	ScreenPriv->SWCursor = FALSE;
	ScreenPriv->isUp = TRUE;
	PointPriv->waitForUpdate = !infoPtr->pScrn->silkenMouse;
	return;
    }

    PointPriv->waitForUpdate = TRUE;

    if (ScreenPriv->isUp) {
	/* Remove the HW cursor, or make it transparent */
	if (infoPtr->Flags & HARDWARE_CURSOR_SHOW_TRANSPARENT) {
	    xf86SetTransparentCursor(pScreen);
	} else {
	    xf86SetCursor(pScreen, NullCursor, x, y);
	    ScreenPriv->isUp = FALSE;
	}
    }

    ScreenPriv->SWCursor = TRUE;

    if (pCurs->bits->emptyMask && !ScreenPriv->showTransparent)
	pCurs = NullCursor;
    (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, pCurs, x, y);
}

static void
xf86CursorMoveCursor(ScreenPtr pScreen, int x, int y)
{
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);

    ScreenPriv->x = x;
    ScreenPriv->y = y;

    if (ScreenPriv->CursorToRestore)
	xf86CursorSetCursor(pScreen, ScreenPriv->CursorToRestore,
			    ScreenPriv->x, ScreenPriv->y);
    else if (ScreenPriv->SWCursor)
	(*ScreenPriv->spriteFuncs->MoveCursor)(pScreen, x, y);
    else if (ScreenPriv->isUp)
	xf86MoveCursor(pScreen, x, y);
}

void
xf86ForceHWCursor (ScreenPtr pScreen, Bool on)
{
    xf86CursorScreenPtr ScreenPriv = (xf86CursorScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, xf86CursorScreenKey);

    if (on)
    {
	if (ScreenPriv->ForceHWCursorCount++ == 0)
	{
	    if (ScreenPriv->SWCursor && ScreenPriv->CurrentCursor)
	    {
		ScreenPriv->HWCursorForced = TRUE;
		xf86CursorSetCursor (pScreen, ScreenPriv->CurrentCursor,
				     ScreenPriv->x, ScreenPriv->y);
	    }
	    else
		ScreenPriv->HWCursorForced = FALSE;
	}
    }
    else
    {
	if (--ScreenPriv->ForceHWCursorCount == 0)
	{
	    if (ScreenPriv->HWCursorForced && ScreenPriv->CurrentCursor)
		xf86CursorSetCursor (pScreen, ScreenPriv->CurrentCursor,
				     ScreenPriv->x, ScreenPriv->y);
	}
    }
}

xf86CursorInfoPtr
xf86CreateCursorInfoRec(void)
{
    return xcalloc(1, sizeof(xf86CursorInfoRec));
}

void
xf86DestroyCursorInfoRec(xf86CursorInfoPtr infoPtr)
{
    xfree(infoPtr);
}
