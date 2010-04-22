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

static int miPointerScreenKeyIndex;
DevPrivateKey miPointerScreenKey = &miPointerScreenKeyIndex;

#define GetScreenPrivate(s) ((miPointerScreenPtr) \
    dixLookupPrivate(&(s)->devPrivates, miPointerScreenKey))
#define SetupScreen(s)	miPointerScreenPtr  pScreenPriv = GetScreenPrivate(s)

static int miPointerPrivKeyIndex;
static DevPrivateKey miPointerPrivKey = &miPointerPrivKeyIndex;

#define MIPOINTER(dev) \
    ((!IsMaster(dev) && !dev->u.master) ? \
        (miPointerPtr)dixLookupPrivate(&(dev)->devPrivates, miPointerPrivKey): \
        (miPointerPtr)dixLookupPrivate(&(GetMaster(dev, MASTER_POINTER))->devPrivates, miPointerPrivKey))

static Bool miPointerRealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                   CursorPtr pCursor);
static Bool miPointerUnrealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                     CursorPtr pCursor);
static Bool miPointerDisplayCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                   CursorPtr pCursor);
static void miPointerConstrainCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                     BoxPtr pBox); 
static void miPointerPointerNonInterestBox(DeviceIntPtr pDev, 
                                           ScreenPtr pScreen, BoxPtr pBox);
static void miPointerCursorLimits(DeviceIntPtr pDev, ScreenPtr pScreen,
                                  CursorPtr pCursor, BoxPtr pHotBox, 
                                  BoxPtr pTopLeftBox);
static Bool miPointerSetCursorPosition(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                       int x, int y,
				       Bool generateEvent);
static Bool miPointerCloseScreen(int index, ScreenPtr pScreen);
static void miPointerMove(DeviceIntPtr pDev, ScreenPtr pScreen, 
                          int x, int y);
static Bool miPointerDeviceInitialize(DeviceIntPtr pDev, ScreenPtr pScreen);
static void miPointerDeviceCleanup(DeviceIntPtr pDev,
                                   ScreenPtr pScreen);

static EventList* events; /* for WarpPointer MotionNotifies */

Bool
miPointerInitialize (ScreenPtr                  pScreen,
                     miPointerSpriteFuncPtr     spriteFuncs,
                     miPointerScreenFuncPtr     screenFuncs,
                     Bool                       waitForUpdate)
{
    miPointerScreenPtr	pScreenPriv;

    pScreenPriv = xalloc (sizeof (miPointerScreenRec));
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
    pScreen->DeviceCursorInitialize = miPointerDeviceInitialize;
    pScreen->DeviceCursorCleanup = miPointerDeviceCleanup;

    events = NULL;
    return TRUE;
}

static Bool
miPointerCloseScreen (int index, ScreenPtr pScreen)
{
#if 0
    miPointerPtr pPointer;
    DeviceIntPtr pDev;
#endif

    SetupScreen(pScreen);

#if 0
    for (pDev = inputInfo.devices; pDev; pDev = pDev->next)
    {
        if (DevHasCursor(pDev))
        {
            pPointer = MIPOINTER(pDev);

            if (pScreen == pPointer->pScreen)
                pPointer->pScreen = 0;
            if (pScreen == pPointer->pSpriteScreen)
                pPointer->pSpriteScreen = 0;
        }
    }

    if (MIPOINTER(inputInfo.pointer)->pScreen == pScreen)
        MIPOINTER(inputInfo.pointer)->pScreen = 0;
    if (MIPOINTER(inputInfo.pointer)->pSpriteScreen == pScreen)
        MIPOINTER(inputInfo.pointer)->pSpriteScreen = 0;
#endif

    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    xfree ((pointer) pScreenPriv);
    FreeEventList(events, GetMaximumEventsNum());
    events = NULL;
    return (*pScreen->CloseScreen) (index, pScreen);
}

/*
 * DIX/DDX interface routines
 */

static Bool
miPointerRealizeCursor (DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor)
{
    SetupScreen(pScreen);
    return (*pScreenPriv->spriteFuncs->RealizeCursor) (pDev, pScreen, pCursor);
}

static Bool
miPointerUnrealizeCursor (DeviceIntPtr  pDev,
                          ScreenPtr     pScreen,
                          CursorPtr     pCursor)
{
    SetupScreen(pScreen);
    return (*pScreenPriv->spriteFuncs->UnrealizeCursor) (pDev, pScreen, pCursor);
}

static Bool
miPointerDisplayCursor (DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor)
{
    miPointerPtr pPointer;

    /* return for keyboards */
    if ((IsMaster(pDev) && !DevHasCursor(pDev)) ||
        (!IsMaster(pDev) && pDev->u.master && !DevHasCursor(pDev->u.master)))
            return FALSE;

    pPointer = MIPOINTER(pDev);

    pPointer->pCursor = pCursor;
    pPointer->pScreen = pScreen;
    miPointerUpdateSprite(pDev);
    return TRUE;
}

static void
miPointerConstrainCursor (DeviceIntPtr pDev, ScreenPtr pScreen, BoxPtr pBox)
{
    miPointerPtr pPointer;

    pPointer = MIPOINTER(pDev);

    pPointer->limits = *pBox;
    pPointer->confined = PointerConfinedToScreen(pDev);
}

/*ARGSUSED*/
static void
miPointerPointerNonInterestBox (DeviceIntPtr    pDev,
                                ScreenPtr       pScreen,
                                BoxPtr          pBox)
{
    /* until DIX uses this, this will remain a stub */
}

/*ARGSUSED*/
static void
miPointerCursorLimits(DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor,
                      BoxPtr pHotBox, BoxPtr pTopLeftBox)
{
    *pTopLeftBox = *pHotBox;
}

static Bool GenerateEvent;

static Bool
miPointerSetCursorPosition(DeviceIntPtr pDev, ScreenPtr pScreen,
                           int x, int y, Bool generateEvent)
{
    SetupScreen (pScreen);

    GenerateEvent = generateEvent;
    /* device dependent - must pend signal and call miPointerWarpCursor */
    (*pScreenPriv->screenFuncs->WarpCursor) (pDev, pScreen, x, y);
    if (!generateEvent)
	miPointerUpdateSprite(pDev);
    return TRUE;
}

/* Set up sprite information for the device.
   This function will be called once for each device after it is initialized
   in the DIX.
 */
static Bool
miPointerDeviceInitialize(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    miPointerPtr pPointer;
    SetupScreen (pScreen);

    pPointer = xalloc(sizeof(miPointerRec));
    if (!pPointer)
        return FALSE;

    pPointer->pScreen = NULL;
    pPointer->pSpriteScreen = NULL;
    pPointer->pCursor = NULL;
    pPointer->pSpriteCursor = NULL;
    pPointer->limits.x1 = 0;
    pPointer->limits.x2 = 32767;
    pPointer->limits.y1 = 0;
    pPointer->limits.y2 = 32767;
    pPointer->confined = FALSE;
    pPointer->x = 0;
    pPointer->y = 0;

    if (!((*pScreenPriv->spriteFuncs->DeviceCursorInitialize)(pDev, pScreen)))
    {
        xfree(pPointer);
        return FALSE;
    }

    dixSetPrivate(&pDev->devPrivates, miPointerPrivKey, pPointer);
    return TRUE;
}

/* Clean up after device.
   This function will be called once before the device is freed in the DIX
 */
static void
miPointerDeviceCleanup(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    SetupScreen(pScreen);

    if (!IsMaster(pDev) && pDev->u.master)
        return;

    (*pScreenPriv->spriteFuncs->DeviceCursorCleanup)(pDev, pScreen);
    xfree(dixLookupPrivate(&pDev->devPrivates, miPointerPrivKey));
    dixSetPrivate(&pDev->devPrivates, miPointerPrivKey, NULL);
}


/* Once signals are ignored, the WarpCursor function can call this */

void
miPointerWarpCursor (DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y)
{
    miPointerPtr pPointer;
    BOOL changedScreen = FALSE;

    SetupScreen (pScreen);
    pPointer = MIPOINTER(pDev);

    if (pPointer->pScreen != pScreen)
    {
	(*pScreenPriv->screenFuncs->NewEventScreen) (pDev, pScreen, TRUE);
        changedScreen = TRUE;
    }

    if (GenerateEvent)
    {
	miPointerMove (pDev, pScreen, x, y);
    }
    else
    {
	/* everything from miPointerMove except the event and history */

    	if (!pScreenPriv->waitForUpdate && pScreen == pPointer->pSpriteScreen)
    	{
	    pPointer->devx = x;
	    pPointer->devy = y;
	    if(pPointer->pCursor && !pPointer->pCursor->bits->emptyMask)
		(*pScreenPriv->spriteFuncs->MoveCursor) (pDev, pScreen, x, y);
    	}
	pPointer->x = x;
	pPointer->y = y;
	pPointer->pScreen = pScreen;
    }

    /* Don't call USFS if we use Xinerama, otherwise the root window is
     * updated to the second screen, and we never receive any events.
     * (FDO bug #18668) */
    if (changedScreen
#ifdef PANORAMIX
            && noPanoramiXExtension
#endif
       )
        UpdateSpriteForScreen (pDev, pScreen) ;
}

/*
 * Pointer/CursorDisplay interface routines
 */

/*
 * miPointerUpdateSprite
 *
 * Syncronize the sprite with the cursor - called from ProcessInputEvents
 */

void
miPointerUpdateSprite (DeviceIntPtr pDev)
{
    ScreenPtr		pScreen;
    miPointerScreenPtr	pScreenPriv;
    CursorPtr		pCursor;
    int			x, y, devx, devy;
    miPointerPtr        pPointer;

    if (!pDev || !pDev->coreEvents)
        return;

    pPointer = MIPOINTER(pDev);

    if (!pPointer)
        return;

    pScreen = pPointer->pScreen;
    if (!pScreen)
	return;

    x = pPointer->x;
    y = pPointer->y;
    devx = pPointer->devx;
    devy = pPointer->devy;

    pScreenPriv = GetScreenPrivate (pScreen);
    /*
     * if the cursor has switched screens, disable the sprite
     * on the old screen
     */
    if (pScreen != pPointer->pSpriteScreen)
    {
	if (pPointer->pSpriteScreen)
	{
	    miPointerScreenPtr  pOldPriv;
    	
	    pOldPriv = GetScreenPrivate (pPointer->pSpriteScreen);
	    if (pPointer->pCursor)
	    {
	    	(*pOldPriv->spriteFuncs->SetCursor)
			    	(pDev, pPointer->pSpriteScreen, NullCursor, 0, 0);
	    }
	    (*pOldPriv->screenFuncs->CrossScreen) (pPointer->pSpriteScreen, FALSE);
	}
	(*pScreenPriv->screenFuncs->CrossScreen) (pScreen, TRUE);
	(*pScreenPriv->spriteFuncs->SetCursor)
				(pDev, pScreen, pPointer->pCursor, x, y);
	pPointer->devx = x;
	pPointer->devy = y;
	pPointer->pSpriteCursor = pPointer->pCursor;
	pPointer->pSpriteScreen = pScreen;
    }
    /*
     * if the cursor has changed, display the new one
     */
    else if (pPointer->pCursor != pPointer->pSpriteCursor)
    {
	pCursor = pPointer->pCursor;
	if (!pCursor || (pCursor->bits->emptyMask && !pScreenPriv->showTransparent))
	    pCursor = NullCursor;
	(*pScreenPriv->spriteFuncs->SetCursor) (pDev, pScreen, pCursor, x, y);

	pPointer->devx = x;
	pPointer->devy = y;
	pPointer->pSpriteCursor = pPointer->pCursor;
    }
    else if (x != devx || y != devy)
    {
	pPointer->devx = x;
	pPointer->devy = y;
	if(pPointer->pCursor && !pPointer->pCursor->bits->emptyMask)
	    (*pScreenPriv->spriteFuncs->MoveCursor) (pDev, pScreen, x, y);
    }
}

void
miPointerSetScreen(DeviceIntPtr pDev, int screen_no, int x, int y)
{
	miPointerScreenPtr pScreenPriv;
	ScreenPtr pScreen;
        miPointerPtr pPointer;

        pPointer = MIPOINTER(pDev);

	pScreen = screenInfo.screens[screen_no];
	pScreenPriv = GetScreenPrivate (pScreen);
	(*pScreenPriv->screenFuncs->NewEventScreen) (pDev, pScreen, FALSE);
	NewCurrentScreen (pDev, pScreen, x, y);

        pPointer->limits.x2 = pScreen->width;
        pPointer->limits.y2 = pScreen->height;
}

ScreenPtr
miPointerCurrentScreen (void)
{
    return miPointerGetScreen(inputInfo.pointer);
}

ScreenPtr
miPointerGetScreen(DeviceIntPtr pDev)
{
    miPointerPtr pPointer = MIPOINTER(pDev);
    return (pPointer) ? pPointer->pScreen : NULL;
}

/* Move the pointer on the current screen,  and update the sprite. */
static void
miPointerMoved (DeviceIntPtr pDev, ScreenPtr pScreen,
                int x, int y)
{
    miPointerPtr pPointer;
    SetupScreen(pScreen);

    pPointer = MIPOINTER(pDev);

    /* Hack: We mustn't call into ->MoveCursor for anything but the
     * VCP, as this may cause a non-HW rendered cursor to be rendered during
     * SIGIO. This again leads to allocs during SIGIO which leads to SIGABRT.
     */
    if ((pDev == inputInfo.pointer || (!IsMaster(pDev) && pDev->u.master == inputInfo.pointer))
        && !pScreenPriv->waitForUpdate && pScreen == pPointer->pSpriteScreen)
    {
	pPointer->devx = x;
	pPointer->devy = y;
	if(pPointer->pCursor && !pPointer->pCursor->bits->emptyMask)
	    (*pScreenPriv->spriteFuncs->MoveCursor) (pDev, pScreen, x, y);
    }

    pPointer->x = x;
    pPointer->y = y;
    pPointer->pScreen = pScreen;
}

void
miPointerSetPosition(DeviceIntPtr pDev, int *x, int *y)
{
    miPointerScreenPtr	pScreenPriv;
    ScreenPtr		pScreen;
    ScreenPtr		newScreen;

    miPointerPtr        pPointer; 

    if (!pDev || !pDev->coreEvents)
        return;

    pPointer = MIPOINTER(pDev);
    pScreen = pPointer->pScreen;
    if (!pScreen)
	return;	    /* called before ready */

    if (*x < 0 || *x >= pScreen->width || *y < 0 || *y >= pScreen->height)
    {
	pScreenPriv = GetScreenPrivate (pScreen);
	if (!pPointer->confined)
	{
	    newScreen = pScreen;
	    (*pScreenPriv->screenFuncs->CursorOffScreen) (&newScreen, x, y);
	    if (newScreen != pScreen)
	    {
		pScreen = newScreen;
		(*pScreenPriv->screenFuncs->NewEventScreen) (pDev, pScreen,
							     FALSE);
		pScreenPriv = GetScreenPrivate (pScreen);
	    	/* Smash the confine to the new screen */
                pPointer->limits.x2 = pScreen->width;
                pPointer->limits.y2 = pScreen->height;
	    }
	}
    }
    /* Constrain the sprite to the current limits. */
    if (*x < pPointer->limits.x1)
	*x = pPointer->limits.x1;
    if (*x >= pPointer->limits.x2)
	*x = pPointer->limits.x2 - 1;
    if (*y < pPointer->limits.y1)
	*y = pPointer->limits.y1;
    if (*y >= pPointer->limits.y2)
	*y = pPointer->limits.y2 - 1;

    if (pPointer->x == *x && pPointer->y == *y && 
            pPointer->pScreen == pScreen) 
        return;

    miPointerMoved(pDev, pScreen, *x, *y);
}

void
miPointerGetPosition(DeviceIntPtr pDev, int *x, int *y)
{
    *x = MIPOINTER(pDev)->x;
    *y = MIPOINTER(pDev)->y;
}

#ifdef XQUARTZ
#include <pthread.h>
void darwinEvents_lock(void);
void darwinEvents_unlock(void);
#endif

void
miPointerMove (DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y)
{
    int i, nevents;
    int valuators[2];

    miPointerMoved(pDev, pScreen, x, y);

    /* generate motion notify */
    valuators[0] = x;
    valuators[1] = y;

    if (!events)
    {
        events = InitEventList(GetMaximumEventsNum());

        if (!events)
        {
            FatalError("Could not allocate event store.\n");
            return;
        }
    }

    nevents = GetPointerEvents(events, pDev, MotionNotify, 0, POINTER_SCREEN | POINTER_ABSOLUTE, 0, 2, valuators);

    OsBlockSignals();
#ifdef XQUARTZ
    darwinEvents_lock();
#endif
    for (i = 0; i < nevents; i++)
        mieqEnqueue(pDev, (InternalEvent*)events[i].event);
#ifdef XQUARTZ
    darwinEvents_unlock();
#endif
    OsReleaseSignals();
}
