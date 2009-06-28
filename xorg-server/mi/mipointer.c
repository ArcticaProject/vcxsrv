/*

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

# define NEED_EVENTS
# include   <X11/X.h>
# include   <X11/Xmd.h>
# include   <X11/Xproto.h>
# include   "misc.h"
# include   "windowstr.h"
# include   "pixmapstr.h"
# include   "mi.h"
# include   "scrnintstr.h"
# include   "mipointrst.h"
# include   "cursorstr.h"
# include   "dixstruct.h"
# include   "inputstr.h"

_X_EXPORT DevPrivateKey miPointerScreenKey = &miPointerScreenKey;

#define GetScreenPrivate(s) ((miPointerScreenPtr) \
    dixLookupPrivate(&(s)->devPrivates, miPointerScreenKey))
#define SetupScreen(s)	miPointerScreenPtr  pScreenPriv = GetScreenPrivate(s)

/*
 * until more than one pointer device exists.
 */

static miPointerRec miPointer;

static Bool miPointerRealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
static Bool miPointerUnrealizeCursor(ScreenPtr pScreen, CursorPtr pCursor);
static Bool miPointerDisplayCursor(ScreenPtr pScreen, CursorPtr pCursor);
static void miPointerConstrainCursor(ScreenPtr pScreen, BoxPtr pBox);
static void miPointerPointerNonInterestBox(ScreenPtr pScreen, BoxPtr pBox);
static void miPointerCursorLimits(ScreenPtr pScreen, CursorPtr pCursor,
				  BoxPtr pHotBox, BoxPtr pTopLeftBox);
static Bool miPointerSetCursorPosition(ScreenPtr pScreen, int x, int y,
				       Bool generateEvent);
static Bool miPointerCloseScreen(int index, ScreenPtr pScreen);
static void miPointerMove(ScreenPtr pScreen, int x, int y, unsigned long time);

static xEvent* events; /* for WarpPointer MotionNotifies */

_X_EXPORT Bool
miPointerInitialize (pScreen, spriteFuncs, screenFuncs, waitForUpdate)
    ScreenPtr		    pScreen;
    miPointerSpriteFuncPtr  spriteFuncs;
    miPointerScreenFuncPtr  screenFuncs;
    Bool		    waitForUpdate;
{
    miPointerScreenPtr	pScreenPriv;

    pScreenPriv = (miPointerScreenPtr) xalloc (sizeof (miPointerScreenRec));
    if (!pScreenPriv)
	return FALSE;
    pScreenPriv->spriteFuncs = spriteFuncs;
    pScreenPriv->screenFuncs = screenFuncs;
    /*
     * check for uninitialized methods
     */
    if (!screenFuncs->EnqueueEvent)
	screenFuncs->EnqueueEvent = mieqEnqueue;
    if (!screenFuncs->NewEventScreen)
	screenFuncs->NewEventScreen = mieqSwitchScreen;
    pScreenPriv->waitForUpdate = waitForUpdate;
    pScreenPriv->showTransparent = FALSE;
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = miPointerCloseScreen;
    dixSetPrivate(&pScreen->devPrivates, miPointerScreenKey, pScreenPriv);
    /*
     * set up screen cursor method table
     */
    pScreen->ConstrainCursor = miPointerConstrainCursor;
    pScreen->CursorLimits = miPointerCursorLimits;
    pScreen->DisplayCursor = miPointerDisplayCursor;
    pScreen->RealizeCursor = miPointerRealizeCursor;
    pScreen->UnrealizeCursor = miPointerUnrealizeCursor;
    pScreen->SetCursorPosition = miPointerSetCursorPosition;
    pScreen->RecolorCursor = miRecolorCursor;
    pScreen->PointerNonInterestBox = miPointerPointerNonInterestBox;
    /*
     * set up the pointer object
     */
    miPointer.pScreen = NULL;
    miPointer.pSpriteScreen = NULL;
    miPointer.pCursor = NULL;
    miPointer.pSpriteCursor = NULL;
    miPointer.limits.x1 = 0;
    miPointer.limits.x2 = 32767;
    miPointer.limits.y1 = 0;
    miPointer.limits.y2 = 32767;
    miPointer.confined = FALSE;
    miPointer.x = 0;
    miPointer.y = 0;

    events = NULL;

    return TRUE;
}

static Bool
miPointerCloseScreen (index, pScreen)
    int		index;
    ScreenPtr	pScreen;
{
    SetupScreen(pScreen);

    if (pScreen == miPointer.pScreen)
	miPointer.pScreen = 0;
    if (pScreen == miPointer.pSpriteScreen)
	miPointer.pSpriteScreen = 0;
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    xfree ((pointer) pScreenPriv);
    xfree ((pointer) events);
    events = NULL;
    return (*pScreen->CloseScreen) (index, pScreen);
}

/*
 * DIX/DDX interface routines
 */

static Bool
miPointerRealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupScreen(pScreen);

    return (*pScreenPriv->spriteFuncs->RealizeCursor) (pScreen, pCursor);
}

static Bool
miPointerUnrealizeCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    SetupScreen(pScreen);

    return (*pScreenPriv->spriteFuncs->UnrealizeCursor) (pScreen, pCursor);
}

static Bool
miPointerDisplayCursor (pScreen, pCursor)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
{
    miPointer.pCursor = pCursor;
    miPointer.pScreen = pScreen;
    miPointerUpdateSprite(inputInfo.pointer);
    return TRUE;
}

static void
miPointerConstrainCursor (pScreen, pBox)
    ScreenPtr	pScreen;
    BoxPtr	pBox;
{
    miPointer.limits = *pBox;
    miPointer.confined = PointerConfinedToScreen();
}

/*ARGSUSED*/
static void
miPointerPointerNonInterestBox (pScreen, pBox)
    ScreenPtr	pScreen;
    BoxPtr	pBox;
{
    /* until DIX uses this, this will remain a stub */
}

/*ARGSUSED*/
static void
miPointerCursorLimits(pScreen, pCursor, pHotBox, pTopLeftBox)
    ScreenPtr	pScreen;
    CursorPtr	pCursor;
    BoxPtr	pHotBox;
    BoxPtr	pTopLeftBox;
{
    *pTopLeftBox = *pHotBox;
}

static Bool GenerateEvent;

static Bool
miPointerSetCursorPosition(pScreen, x, y, generateEvent)
    ScreenPtr pScreen;
    int       x, y;
    Bool      generateEvent;
{
    SetupScreen (pScreen);

    GenerateEvent = generateEvent;
    /* device dependent - must pend signal and call miPointerWarpCursor */
    (*pScreenPriv->screenFuncs->WarpCursor) (pScreen, x, y);
    if (!generateEvent)
	miPointerUpdateSprite(inputInfo.pointer);
    return TRUE;
}

/* Once signals are ignored, the WarpCursor function can call this */

_X_EXPORT void
miPointerWarpCursor (pScreen, x, y)
    ScreenPtr	pScreen;
    int		x, y;
{
    BOOL changedScreen = FALSE;
    SetupScreen (pScreen);

    if (miPointer.pScreen != pScreen)
    {
	(*pScreenPriv->screenFuncs->NewEventScreen) (pScreen, TRUE);
        changedScreen = TRUE;
    }

    if (GenerateEvent)
    {
	miPointerMove (pScreen, x, y, GetTimeInMillis()); 
    }
    else
    {
	/* everything from miPointerMove except the event and history */

    	if (!pScreenPriv->waitForUpdate && pScreen == miPointer.pSpriteScreen)
    	{
	    miPointer.devx = x;
	    miPointer.devy = y;
	    if(!miPointer.pCursor->bits->emptyMask)
		(*pScreenPriv->spriteFuncs->MoveCursor) (pScreen, x, y);
    	}
	miPointer.x = x;
	miPointer.y = y;
	miPointer.pScreen = pScreen;
    }

    if (changedScreen)
        UpdateSpriteForScreen (pScreen) ;
}

/*
 * Pointer/CursorDisplay interface routines
 */

/*
 * miPointerUpdate
 *
 * Syncronize the sprite with the cursor - called from ProcessInputEvents
 */

void
miPointerUpdate ()
{
    miPointerUpdateSprite(inputInfo.pointer);
}

void
miPointerUpdateSprite (DeviceIntPtr pDev)
{
    ScreenPtr		pScreen;
    miPointerScreenPtr	pScreenPriv;
    CursorPtr		pCursor;
    int			x, y, devx, devy;

    if (!pDev || !(pDev->coreEvents || pDev == inputInfo.pointer))
        return;

    pScreen = miPointer.pScreen;
    if (!pScreen)
	return;

    x = miPointer.x;
    y = miPointer.y;
    devx = miPointer.devx;
    devy = miPointer.devy;

    pScreenPriv = GetScreenPrivate (pScreen);
    /*
     * if the cursor has switched screens, disable the sprite
     * on the old screen
     */
    if (pScreen != miPointer.pSpriteScreen)
    {
	if (miPointer.pSpriteScreen)
	{
	    miPointerScreenPtr  pOldPriv;
    	
	    pOldPriv = GetScreenPrivate (miPointer.pSpriteScreen);
	    if (miPointer.pCursor)
	    {
	    	(*pOldPriv->spriteFuncs->SetCursor)
			    	(miPointer.pSpriteScreen, NullCursor, 0, 0);
	    }
	    (*pOldPriv->screenFuncs->CrossScreen) (miPointer.pSpriteScreen, FALSE);
	}
	(*pScreenPriv->screenFuncs->CrossScreen) (pScreen, TRUE);
	(*pScreenPriv->spriteFuncs->SetCursor)
				(pScreen, miPointer.pCursor, x, y);
	miPointer.devx = x;
	miPointer.devy = y;
	miPointer.pSpriteCursor = miPointer.pCursor;
	miPointer.pSpriteScreen = pScreen;
    }
    /*
     * if the cursor has changed, display the new one
     */
    else if (miPointer.pCursor != miPointer.pSpriteCursor)
    {
	pCursor = miPointer.pCursor;
	if (pCursor->bits->emptyMask && !pScreenPriv->showTransparent)
	    pCursor = NullCursor;
	(*pScreenPriv->spriteFuncs->SetCursor) (pScreen, pCursor, x, y);

	miPointer.devx = x;
	miPointer.devy = y;
	miPointer.pSpriteCursor = miPointer.pCursor;
    }
    else if (x != devx || y != devy)
    {
	miPointer.devx = x;
	miPointer.devy = y;
	if(!miPointer.pCursor->bits->emptyMask)
	    (*pScreenPriv->spriteFuncs->MoveCursor) (pScreen, x, y);
    }
}

/*
 * miPointerDeltaCursor.  The pointer has moved dx,dy from it's previous
 * position.
 */

void
miPointerDeltaCursor (int dx, int dy, unsigned long time)
{
    int x = miPointer.x + dx, y = miPointer.y + dy;

    miPointerSetPosition(inputInfo.pointer, &x, &y, time);
}

void
miPointerSetNewScreen(int screen_no, int x, int y)
{
    miPointerSetScreen(inputInfo.pointer, screen_no, x, y);
}

void
miPointerSetScreen(DeviceIntPtr pDev, int screen_no, int x, int y)
{
	miPointerScreenPtr pScreenPriv;
	ScreenPtr pScreen;

	pScreen = screenInfo.screens[screen_no];
	pScreenPriv = GetScreenPrivate (pScreen);
	(*pScreenPriv->screenFuncs->NewEventScreen) (pScreen, FALSE);
	NewCurrentScreen (pScreen, x, y);
   	miPointer.limits.x2 = pScreen->width;
   	miPointer.limits.y2 = pScreen->height;
}

_X_EXPORT ScreenPtr
miPointerCurrentScreen ()
{
    return miPointerGetScreen(inputInfo.pointer);
}

_X_EXPORT ScreenPtr
miPointerGetScreen(DeviceIntPtr pDev)
{
    return miPointer.pScreen;
}

/* Move the pointer to x, y on the current screen, update the sprite, and
 * the motion history.  Generates no events.  Does not return changed x
 * and y if they are clipped; use miPointerSetPosition instead. */
_X_EXPORT void
miPointerAbsoluteCursor (int x, int y, unsigned long time)
{
    miPointerSetPosition(inputInfo.pointer, &x, &y, time);
}

/* Move the pointer on the current screen,  and update the sprite. */
static void
miPointerMoved (DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y,
                     unsigned long time)
{
    SetupScreen(pScreen);

    if (pDev && (pDev->coreEvents || pDev == inputInfo.pointer) &&
        !pScreenPriv->waitForUpdate && pScreen == miPointer.pSpriteScreen)
    {
	miPointer.devx = x;
	miPointer.devy = y;
	if(!miPointer.pCursor->bits->emptyMask)
	    (*pScreenPriv->spriteFuncs->MoveCursor) (pScreen, x, y);
    }

    miPointer.x = x;
    miPointer.y = y;
    miPointer.pScreen = pScreen;
}

_X_EXPORT void
miPointerSetPosition(DeviceIntPtr pDev, int *x, int *y, unsigned long time)
{
    miPointerScreenPtr	pScreenPriv;
    ScreenPtr		pScreen;
    ScreenPtr		newScreen;

    pScreen = miPointer.pScreen;
    if (!pScreen)
	return;	    /* called before ready */

    if (!pDev || !(pDev->coreEvents || pDev == inputInfo.pointer))
        return;

    if (*x < 0 || *x >= pScreen->width || *y < 0 || *y >= pScreen->height)
    {
	pScreenPriv = GetScreenPrivate (pScreen);
	if (!miPointer.confined)
	{
	    newScreen = pScreen;
	    (*pScreenPriv->screenFuncs->CursorOffScreen) (&newScreen, x, y);
	    if (newScreen != pScreen)
	    {
		pScreen = newScreen;
		(*pScreenPriv->screenFuncs->NewEventScreen) (pScreen, FALSE);
		pScreenPriv = GetScreenPrivate (pScreen);
	    	/* Smash the confine to the new screen */
	    	miPointer.limits.x2 = pScreen->width;
	    	miPointer.limits.y2 = pScreen->height;
	    }
	}
    }
    /* Constrain the sprite to the current limits. */
    if (*x < miPointer.limits.x1)
	*x = miPointer.limits.x1;
    if (*x >= miPointer.limits.x2)
	*x = miPointer.limits.x2 - 1;
    if (*y < miPointer.limits.y1)
	*y = miPointer.limits.y1;
    if (*y >= miPointer.limits.y2)
	*y = miPointer.limits.y2 - 1;

    if (miPointer.x == *x && miPointer.y == *y && miPointer.pScreen == pScreen)
	return;

    miPointerMoved(pDev, pScreen, *x, *y, time);
}

_X_EXPORT void
miPointerPosition (int *x, int *y)
{
    miPointerGetPosition(inputInfo.pointer, x, y);
}

_X_EXPORT void
miPointerGetPosition(DeviceIntPtr pDev, int *x, int *y)
{
    *x = miPointer.x;
    *y = miPointer.y;
}

void
miPointerMove (ScreenPtr pScreen, int x, int y, unsigned long time)
{
    int i, nevents;
    int valuators[2];

    miPointerMoved(inputInfo.pointer, pScreen, x, y, time);

    /* generate motion notify */
    valuators[0] = x;
    valuators[1] = y;

    if (!events)
    {
        events = (xEvent*)xcalloc(sizeof(xEvent), GetMaximumEventsNum());

        if (!events)
        {
            FatalError("Could not allocate event store.\n");
            return;
        }
    }

    nevents = GetPointerEvents(events, inputInfo.pointer, MotionNotify, 0,
                               POINTER_ABSOLUTE, 0, 2, valuators);

    for (i = 0; i < nevents; i++)
        mieqEnqueue(inputInfo.pointer, &events[i]);
}
