/*
 * misprite.c
 *
 * machine independent software sprite routines
 */

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

#include   <X11/X.h>
#include   <X11/Xproto.h>
#include   "misc.h"
#include   "pixmapstr.h"
#include   "input.h"
#include   "mi.h"
#include   "cursorstr.h"
#include   <X11/fonts/font.h>
#include   "scrnintstr.h"
#include   "colormapst.h"
#include   "windowstr.h"
#include   "gcstruct.h"
#include   "mipointer.h"
#include   "misprite.h"
#include   "dixfontstr.h"
#include   <X11/fonts/fontstruct.h>
#include   "inputstr.h"
#include   "damage.h"

typedef struct {
    CursorPtr	    pCursor;
    int		    x;			/* cursor hotspot */
    int		    y;
    BoxRec	    saved;		/* saved area from the screen */
    Bool	    isUp;		/* cursor in frame buffer */
    Bool	    shouldBeUp;		/* cursor should be displayed */
    WindowPtr	    pCacheWin;		/* window the cursor last seen in */
    Bool	    isInCacheWin;
    Bool	    checkPixels;	/* check colormap collision */
    ScreenPtr       pScreen;
} miCursorInfoRec, *miCursorInfoPtr;

/*
 * per screen information
 */

typedef struct {
    /* screen procedures */
    CloseScreenProcPtr			CloseScreen;
    GetImageProcPtr			GetImage;
    GetSpansProcPtr			GetSpans;
    SourceValidateProcPtr		SourceValidate;
    
    /* window procedures */
    CopyWindowProcPtr			CopyWindow;
    
    /* colormap procedures */
    InstallColormapProcPtr		InstallColormap;
    StoreColorsProcPtr			StoreColors;
    
    /* os layer procedures */
    ScreenBlockHandlerProcPtr		BlockHandler;
    
    /* device cursor procedures */
    DeviceCursorInitializeProcPtr       DeviceCursorInitialize;
    DeviceCursorCleanupProcPtr          DeviceCursorCleanup;

    xColorItem	    colors[2];
    ColormapPtr     pInstalledMap;
    ColormapPtr     pColormap;
    VisualPtr	    pVisual;
    miSpriteCursorFuncPtr    funcs;
    DamagePtr	    pDamage;		/* damage tracking structure */
    Bool            damageRegistered;
} miSpriteScreenRec, *miSpriteScreenPtr;

#define SOURCE_COLOR	0
#define MASK_COLOR	1

/*
 * Overlap BoxPtr and Box elements
 */
#define BOX_OVERLAP(pCbox,X1,Y1,X2,Y2) \
 	(((pCbox)->x1 <= (X2)) && ((X1) <= (pCbox)->x2) && \
	 ((pCbox)->y1 <= (Y2)) && ((Y1) <= (pCbox)->y2))

/*
 * Overlap BoxPtr, origins, and rectangle
 */
#define ORG_OVERLAP(pCbox,xorg,yorg,x,y,w,h) \
    BOX_OVERLAP((pCbox),(x)+(xorg),(y)+(yorg),(x)+(xorg)+(w),(y)+(yorg)+(h))

/*
 * Overlap BoxPtr, origins and RectPtr
 */
#define ORGRECT_OVERLAP(pCbox,xorg,yorg,pRect) \
    ORG_OVERLAP((pCbox),(xorg),(yorg),(pRect)->x,(pRect)->y, \
		(int)((pRect)->width), (int)((pRect)->height))
/*
 * Overlap BoxPtr and horizontal span
 */
#define SPN_OVERLAP(pCbox,y,x,w) BOX_OVERLAP((pCbox),(x),(y),(x)+(w),(y))

#define LINE_SORT(x1,y1,x2,y2) \
{ int _t; \
  if (x1 > x2) { _t = x1; x1 = x2; x2 = _t; } \
  if (y1 > y2) { _t = y1; y1 = y2; y2 = _t; } }

#define LINE_OVERLAP(pCbox,x1,y1,x2,y2,lw2) \
    BOX_OVERLAP((pCbox), (x1)-(lw2), (y1)-(lw2), (x2)+(lw2), (y2)+(lw2))


#define SPRITE_DEBUG_ENABLE 0
#if SPRITE_DEBUG_ENABLE
#define SPRITE_DEBUG(x)	ErrorF x
#else
#define SPRITE_DEBUG(x)
#endif

#define MISPRITE(dev) \
    ((!IsMaster(dev) && !dev->u.master) ? \
       (miCursorInfoPtr)dixLookupPrivate(&dev->devPrivates, miSpriteDevPrivatesKey) : \
       (miCursorInfoPtr)dixLookupPrivate(&(GetMaster(dev, MASTER_POINTER))->devPrivates, miSpriteDevPrivatesKey))

static void
miSpriteDisableDamage(ScreenPtr pScreen, miSpriteScreenPtr pScreenPriv)
{
    if (pScreenPriv->damageRegistered) {
	DamageUnregister (&(pScreen->GetScreenPixmap(pScreen)->drawable),
			  pScreenPriv->pDamage);
	pScreenPriv->damageRegistered = 0;
    }
}

static void
miSpriteEnableDamage(ScreenPtr pScreen, miSpriteScreenPtr pScreenPriv)
{
    if (!pScreenPriv->damageRegistered) {
	pScreenPriv->damageRegistered = 1;
	DamageRegister (&(pScreen->GetScreenPixmap(pScreen)->drawable),
			pScreenPriv->pDamage);
    }
}

static void
miSpriteIsUp(miCursorInfoPtr pDevCursor)
{
    pDevCursor->isUp = TRUE;
}

static void
miSpriteIsDown(miCursorInfoPtr pDevCursor)
{
    pDevCursor->isUp = FALSE;
}

/*
 * screen wrappers
 */

static int miSpriteScreenKeyIndex;
static DevPrivateKey miSpriteScreenKey = &miSpriteScreenKeyIndex;
static int miSpriteDevPrivatesKeyIndex;
static DevPrivateKey miSpriteDevPrivatesKey = &miSpriteDevPrivatesKeyIndex;

static Bool	    miSpriteCloseScreen(int i, ScreenPtr pScreen);
static void	    miSpriteGetImage(DrawablePtr pDrawable, int sx, int sy,
				     int w, int h, unsigned int format,
				     unsigned long planemask, char *pdstLine);
static void	    miSpriteGetSpans(DrawablePtr pDrawable, int wMax,
				     DDXPointPtr ppt, int *pwidth, int nspans,
				     char *pdstStart);
static void	    miSpriteSourceValidate(DrawablePtr pDrawable, int x, int y,
					   int width, int height);
static void	    miSpriteCopyWindow (WindowPtr pWindow,
					DDXPointRec ptOldOrg,
					RegionPtr prgnSrc);
static void	    miSpriteBlockHandler(int i, pointer blockData,
					 pointer pTimeout,
					 pointer pReadMask);
static void	    miSpriteInstallColormap(ColormapPtr pMap);
static void	    miSpriteStoreColors(ColormapPtr pMap, int ndef,
					xColorItem *pdef);

static void	    miSpriteComputeSaved(DeviceIntPtr pDev,
                                         ScreenPtr pScreen);

static Bool         miSpriteDeviceCursorInitialize(DeviceIntPtr pDev,
                                                   ScreenPtr pScreen);
static void         miSpriteDeviceCursorCleanup(DeviceIntPtr pDev,
                                                ScreenPtr pScreen);

#define SCREEN_PROLOGUE(pScreen, field) ((pScreen)->field = \
   ((miSpriteScreenPtr)dixLookupPrivate(&(pScreen)->devPrivates, \
					miSpriteScreenKey))->field)
#define SCREEN_EPILOGUE(pScreen, field)\
    ((pScreen)->field = miSprite##field)

/*
 * pointer-sprite method table
 */

static Bool miSpriteRealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                  CursorPtr pCursor);
static Bool miSpriteUnrealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                    CursorPtr pCursor);
static void miSpriteSetCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                              CursorPtr pCursor, int x, int y);
static void miSpriteMoveCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                               int x, int y);

miPointerSpriteFuncRec miSpritePointerFuncs = {
    miSpriteRealizeCursor,
    miSpriteUnrealizeCursor,
    miSpriteSetCursor,
    miSpriteMoveCursor,
    miSpriteDeviceCursorInitialize,
    miSpriteDeviceCursorCleanup,
};

/*
 * other misc functions
 */

static void miSpriteRemoveCursor(DeviceIntPtr pDev,
                                 ScreenPtr pScreen);
static void miSpriteSaveUnderCursor(DeviceIntPtr pDev,
                                 ScreenPtr pScreen);
static void miSpriteRestoreCursor(DeviceIntPtr pDev,
                                 ScreenPtr pScreen);

static void
miSpriteReportDamage (DamagePtr pDamage, RegionPtr pRegion, void *closure)
{
    ScreenPtr		    pScreen = closure;
    miSpriteScreenPtr	    pScreenPriv;
    miCursorInfoPtr         pCursorInfo;
    DeviceIntPtr            pDev;

    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);

    for (pDev = inputInfo.devices; pDev; pDev = pDev->next)
    {
        if (DevHasCursor(pDev))
        {
            pCursorInfo = MISPRITE(pDev);

            if (pCursorInfo->isUp &&
                pCursorInfo->pScreen == pScreen &&
                miRectIn(pRegion, &pCursorInfo->saved) != rgnOUT)
            {
                SPRITE_DEBUG(("Damage remove\n"));
                miSpriteRemoveCursor (pDev, pScreen);
            }
        }
    }
}

/*
 * miSpriteInitialize -- called from device-dependent screen
 * initialization proc after all of the function pointers have
 * been stored in the screen structure.
 */

Bool
miSpriteInitialize (ScreenPtr               pScreen,
                    miSpriteCursorFuncPtr   cursorFuncs,
                    miPointerScreenFuncPtr  screenFuncs)
{
    miSpriteScreenPtr	pScreenPriv;
    VisualPtr		pVisual;

    if (!DamageSetup (pScreen))
	return FALSE;

    pScreenPriv = xalloc (sizeof (miSpriteScreenRec));
    if (!pScreenPriv)
	return FALSE;

    pScreenPriv->pDamage = DamageCreate (miSpriteReportDamage,
					 NULL,
					 DamageReportRawRegion,
					 TRUE,
					 pScreen,
					 pScreen);

    if (!miPointerInitialize (pScreen, &miSpritePointerFuncs, screenFuncs,TRUE))
    {
	xfree (pScreenPriv);
	return FALSE;
    }
    for (pVisual = pScreen->visuals;
	 pVisual->vid != pScreen->rootVisual;
	 pVisual++)
	;
    pScreenPriv->pVisual = pVisual;
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreenPriv->GetImage = pScreen->GetImage;
    pScreenPriv->GetSpans = pScreen->GetSpans;
    pScreenPriv->SourceValidate = pScreen->SourceValidate;

    pScreenPriv->CopyWindow = pScreen->CopyWindow;

    pScreenPriv->InstallColormap = pScreen->InstallColormap;
    pScreenPriv->StoreColors = pScreen->StoreColors;

    pScreenPriv->BlockHandler = pScreen->BlockHandler;

    pScreenPriv->DeviceCursorInitialize = pScreen->DeviceCursorInitialize;
    pScreenPriv->DeviceCursorCleanup = pScreen->DeviceCursorCleanup;

    pScreenPriv->pInstalledMap = NULL;
    pScreenPriv->pColormap = NULL;
    pScreenPriv->funcs = cursorFuncs;
    pScreenPriv->colors[SOURCE_COLOR].red = 0;
    pScreenPriv->colors[SOURCE_COLOR].green = 0;
    pScreenPriv->colors[SOURCE_COLOR].blue = 0;
    pScreenPriv->colors[MASK_COLOR].red = 0;
    pScreenPriv->colors[MASK_COLOR].green = 0;
    pScreenPriv->colors[MASK_COLOR].blue = 0;
    pScreenPriv->damageRegistered = 0;

    dixSetPrivate(&pScreen->devPrivates, miSpriteScreenKey, pScreenPriv);

    pScreen->CloseScreen = miSpriteCloseScreen;
    pScreen->GetImage = miSpriteGetImage;
    pScreen->GetSpans = miSpriteGetSpans;
    pScreen->SourceValidate = miSpriteSourceValidate;

    pScreen->CopyWindow = miSpriteCopyWindow;
    pScreen->InstallColormap = miSpriteInstallColormap;
    pScreen->StoreColors = miSpriteStoreColors;

    pScreen->BlockHandler = miSpriteBlockHandler;

    return TRUE;
}

/*
 * Screen wrappers
 */

/*
 * CloseScreen wrapper -- unwrap everything, free the private data
 * and call the wrapped function
 */

static Bool
miSpriteCloseScreen (int i, ScreenPtr pScreen)
{
    miSpriteScreenPtr   pScreenPriv;

    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->GetImage = pScreenPriv->GetImage;
    pScreen->GetSpans = pScreenPriv->GetSpans;
    pScreen->SourceValidate = pScreenPriv->SourceValidate;
    pScreen->BlockHandler = pScreenPriv->BlockHandler;
    pScreen->InstallColormap = pScreenPriv->InstallColormap;
    pScreen->StoreColors = pScreenPriv->StoreColors;

    DamageDestroy (pScreenPriv->pDamage);

    xfree (pScreenPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}

static void
miSpriteGetImage (DrawablePtr pDrawable, int sx, int sy, int w, int h,
                  unsigned int format, unsigned long planemask,
                  char *pdstLine)
{
    ScreenPtr	    pScreen = pDrawable->pScreen;
    miSpriteScreenPtr    pScreenPriv;
    DeviceIntPtr    pDev;
    miCursorInfoPtr pCursorInfo;

    SCREEN_PROLOGUE (pScreen, GetImage);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
        pScreenPriv = dixLookupPrivate(&pScreen->devPrivates,miSpriteScreenKey);
        for(pDev = inputInfo.devices; pDev; pDev = pDev->next)
        {
            if (DevHasCursor(pDev))
            {
                 pCursorInfo = MISPRITE(pDev);
                 if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen &&
                      ORG_OVERLAP(&pCursorInfo->saved,pDrawable->x,pDrawable->y,
                                  sx, sy, w, h))
                 {
                     SPRITE_DEBUG (("GetImage remove\n"));
                     miSpriteRemoveCursor (pDev, pScreen);
                 }
            }
        }
    }

    (*pScreen->GetImage) (pDrawable, sx, sy, w, h,
			  format, planemask, pdstLine);

    SCREEN_EPILOGUE (pScreen, GetImage);
}

static void
miSpriteGetSpans (DrawablePtr pDrawable, int wMax, DDXPointPtr ppt,
                  int *pwidth, int nspans, char *pdstStart)
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    miSpriteScreenPtr	    pScreenPriv;
    DeviceIntPtr            pDev;
    miCursorInfoPtr         pCursorInfo;

    SCREEN_PROLOGUE (pScreen, GetSpans);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
        pScreenPriv = dixLookupPrivate(&pScreen->devPrivates,miSpriteScreenKey);

        for(pDev = inputInfo.devices; pDev; pDev = pDev->next)
        {
            if (DevHasCursor(pDev))
            {
                pCursorInfo = MISPRITE(pDev);

                if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen)
                {
                    DDXPointPtr    pts;
                    int    	       *widths;
                    int    	       nPts;
                    int    	       xorg,
                                   yorg;

                    xorg = pDrawable->x;
                    yorg = pDrawable->y;

                    for (pts = ppt, widths = pwidth, nPts = nspans;
                            nPts--;
                            pts++, widths++)
                    {
                        if (SPN_OVERLAP(&pCursorInfo->saved,pts->y+yorg,
                                    pts->x+xorg,*widths))
                        {
                            SPRITE_DEBUG (("GetSpans remove\n"));
                            miSpriteRemoveCursor (pDev, pScreen);
                            break;
                        }
                    }
                }
            }
        }
    }

    (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);

    SCREEN_EPILOGUE (pScreen, GetSpans);
}

static void
miSpriteSourceValidate (DrawablePtr pDrawable, int x, int y, int width,
                        int height)
{
    ScreenPtr		    pScreen = pDrawable->pScreen;
    miSpriteScreenPtr	    pScreenPriv;
    DeviceIntPtr            pDev;
    miCursorInfoPtr         pCursorInfo;

    SCREEN_PROLOGUE (pScreen, SourceValidate);

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	pScreenPriv = dixLookupPrivate(&pScreen->devPrivates,miSpriteScreenKey);

	for(pDev = inputInfo.devices; pDev; pDev = pDev->next)
	{
	    if (DevHasCursor(pDev))
	    {
		pCursorInfo = MISPRITE(pDev);
		if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen &&
		    ORG_OVERLAP(&pCursorInfo->saved, pDrawable->x, pDrawable->y,
				x, y, width, height))
		{
		    SPRITE_DEBUG (("SourceValidate remove\n"));
		    miSpriteRemoveCursor (pDev, pScreen);
		}
	    }
	}
    }

    if (pScreen->SourceValidate)
	(*pScreen->SourceValidate) (pDrawable, x, y, width, height);

    SCREEN_EPILOGUE (pScreen, SourceValidate);
}

static void
miSpriteCopyWindow (WindowPtr pWindow, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    ScreenPtr	pScreen = pWindow->drawable.pScreen;
    miSpriteScreenPtr	    pScreenPriv;
    DeviceIntPtr            pDev;
    miCursorInfoPtr         pCursorInfo;

    SCREEN_PROLOGUE (pScreen, CopyWindow);

    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);

    for(pDev = inputInfo.devices; pDev; pDev = pDev->next)
    {
        if (DevHasCursor(pDev))
        {
            pCursorInfo = MISPRITE(pDev);
            /*
             * Damage will take care of destination check
             */
            if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen &&
                    miRectIn(prgnSrc, &pCursorInfo->saved) != rgnOUT)
            {
                SPRITE_DEBUG (("CopyWindow remove\n"));
                miSpriteRemoveCursor (pDev, pScreen);
            }
        }
    }

    (*pScreen->CopyWindow) (pWindow, ptOldOrg, prgnSrc);
    SCREEN_EPILOGUE (pScreen, CopyWindow);
}

static void
miSpriteBlockHandler (int i, pointer blockData, pointer pTimeout,
                      pointer pReadmask)
{
    ScreenPtr		pScreen = screenInfo.screens[i];
    miSpriteScreenPtr	pPriv;
    DeviceIntPtr            pDev;
    miCursorInfoPtr         pCursorInfo;

    pPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    SCREEN_PROLOGUE(pScreen, BlockHandler);

    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);

    SCREEN_EPILOGUE(pScreen, BlockHandler);

    for(pDev = inputInfo.devices; pDev; pDev = pDev->next)
    {
        if (DevHasCursor(pDev))
        {
            pCursorInfo = MISPRITE(pDev);
            if (pCursorInfo && !pCursorInfo->isUp
                    && pCursorInfo->pScreen == pScreen
                    && pCursorInfo->shouldBeUp)
            {
                SPRITE_DEBUG (("BlockHandler restore\n"));
                miSpriteSaveUnderCursor (pDev, pScreen);
            }
        }
    }
    for(pDev = inputInfo.devices; pDev; pDev = pDev->next)
    {
        if (DevHasCursor(pDev))
        {
            pCursorInfo = MISPRITE(pDev);
            if (pCursorInfo && !pCursorInfo->isUp &&
                    pCursorInfo->pScreen == pScreen &&
                    pCursorInfo->shouldBeUp)
            {
                SPRITE_DEBUG (("BlockHandler restore\n"));
                miSpriteRestoreCursor (pDev, pScreen);
            }
        }
    }
}

static void
miSpriteInstallColormap (ColormapPtr pMap)
{
    ScreenPtr		pScreen = pMap->pScreen;
    miSpriteScreenPtr	pPriv;

    pPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    SCREEN_PROLOGUE(pScreen, InstallColormap);

    (*pScreen->InstallColormap) (pMap);

    SCREEN_EPILOGUE(pScreen, InstallColormap);

    /* InstallColormap can be called before devices are initialized. */
    pPriv->pInstalledMap = pMap;
    if (pPriv->pColormap != pMap)
    {
        DeviceIntPtr pDev;
        miCursorInfoPtr     pCursorInfo;
        for (pDev = inputInfo.devices; pDev; pDev = pDev->next)
        {
            if (DevHasCursor(pDev))
            {
                pCursorInfo = MISPRITE(pDev);
                pCursorInfo->checkPixels = TRUE;
                if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen)
                    miSpriteRemoveCursor(pDev, pScreen);
            }
        }

    }
}

static void
miSpriteStoreColors (ColormapPtr pMap, int ndef, xColorItem *pdef)
{
    ScreenPtr		pScreen = pMap->pScreen;
    miSpriteScreenPtr	pPriv;
    int			i;
    int			updated;
    VisualPtr		pVisual;
    DeviceIntPtr        pDev;
    miCursorInfoPtr     pCursorInfo;

    pPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    SCREEN_PROLOGUE(pScreen, StoreColors);

    (*pScreen->StoreColors) (pMap, ndef, pdef);

    SCREEN_EPILOGUE(pScreen, StoreColors);

    if (pPriv->pColormap == pMap)
    {
        updated = 0;
        pVisual = pMap->pVisual;
        if (pVisual->class == DirectColor)
        {
            /* Direct color - match on any of the subfields */

#define MaskMatch(a,b,mask) (((a) & (pVisual->mask)) == ((b) & (pVisual->mask)))

#define UpdateDAC(dev, plane,dac,mask) {\
    if (MaskMatch (dev->colors[plane].pixel,pdef[i].pixel,mask)) {\
	dev->colors[plane].dac = pdef[i].dac; \
	updated = 1; \
    } \
}

#define CheckDirect(dev, plane) \
	    UpdateDAC(dev, plane,red,redMask) \
	    UpdateDAC(dev, plane,green,greenMask) \
	    UpdateDAC(dev, plane,blue,blueMask)

            for (i = 0; i < ndef; i++)
            {
                CheckDirect (pPriv, SOURCE_COLOR)
                CheckDirect (pPriv, MASK_COLOR)
            }
        }
        else
        {
            /* PseudoColor/GrayScale - match on exact pixel */
            for (i = 0; i < ndef; i++)
            {
                if (pdef[i].pixel ==
                        pPriv->colors[SOURCE_COLOR].pixel)
                {
                    pPriv->colors[SOURCE_COLOR] = pdef[i];
                    if (++updated == 2)
                        break;
                }
                if (pdef[i].pixel ==
                        pPriv->colors[MASK_COLOR].pixel)
                {
                    pPriv->colors[MASK_COLOR] = pdef[i];
                    if (++updated == 2)
                        break;
                }
            }
        }
        if (updated)
        {
            for(pDev = inputInfo.devices; pDev; pDev = pDev->next)
            {
                if (DevHasCursor(pDev))
                {
                    pCursorInfo = MISPRITE(pDev);
                    pCursorInfo->checkPixels = TRUE;
                    if (pCursorInfo->isUp && pCursorInfo->pScreen == pScreen)
                        miSpriteRemoveCursor (pDev, pScreen);
                }
            }
        }
    }
}

static void
miSpriteFindColors (miCursorInfoPtr pDevCursor, ScreenPtr pScreen)
{
    miSpriteScreenPtr   pScreenPriv =
	dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    CursorPtr		pCursor;
    xColorItem		*sourceColor, *maskColor;

    pCursor = pDevCursor->pCursor;
    sourceColor = &pScreenPriv->colors[SOURCE_COLOR];
    maskColor = &pScreenPriv->colors[MASK_COLOR];
    if (pScreenPriv->pColormap != pScreenPriv->pInstalledMap ||
	!(pCursor->foreRed == sourceColor->red &&
	  pCursor->foreGreen == sourceColor->green &&
          pCursor->foreBlue == sourceColor->blue &&
	  pCursor->backRed == maskColor->red &&
	  pCursor->backGreen == maskColor->green &&
	  pCursor->backBlue == maskColor->blue))
    {
	pScreenPriv->pColormap = pScreenPriv->pInstalledMap;
	sourceColor->red = pCursor->foreRed;
	sourceColor->green = pCursor->foreGreen;
	sourceColor->blue = pCursor->foreBlue;
	FakeAllocColor (pScreenPriv->pColormap, sourceColor);
	maskColor->red = pCursor->backRed;
	maskColor->green = pCursor->backGreen;
	maskColor->blue = pCursor->backBlue;
	FakeAllocColor (pScreenPriv->pColormap, maskColor);
	/* "free" the pixels right away, don't let this confuse you */
	FakeFreeColor(pScreenPriv->pColormap, sourceColor->pixel);
	FakeFreeColor(pScreenPriv->pColormap, maskColor->pixel);
    }

    pDevCursor->checkPixels = FALSE;

}

/*
 * miPointer interface routines
 */

#define SPRITE_PAD  8

static Bool
miSpriteRealizeCursor (DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor)
{
    miSpriteScreenPtr	pScreenPriv;
    miCursorInfoPtr pCursorInfo;

    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    if (!IsMaster(pDev) && !pDev->u.master)
    {
        ErrorF("[mi] miSpriteRealizeCursor called for floating device.\n");
        return FALSE;
    }
    pCursorInfo = MISPRITE(pDev);

    if (pCursor == pCursorInfo->pCursor)
	pCursorInfo->checkPixels = TRUE;

    return (*pScreenPriv->funcs->RealizeCursor) (pScreen, pCursor);
}

static Bool
miSpriteUnrealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCursor)
{
    miSpriteScreenPtr	pScreenPriv;

    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    return (*pScreenPriv->funcs->UnrealizeCursor) (pScreen, pCursor);
}

static void
miSpriteSetCursor (DeviceIntPtr pDev, ScreenPtr pScreen,
                   CursorPtr pCursor, int x, int y)
{
    miSpriteScreenPtr	pScreenPriv;
    miCursorInfoPtr pPointer;

    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);

    if (!IsMaster(pDev) && !pDev->u.master)
    {
        ErrorF("[mi] miSpriteSetCursor called for floating device.\n");
        return;
    }
    pPointer = MISPRITE(pDev);

    if (!pCursor)
    {
    	pPointer->shouldBeUp = FALSE;
    	if (pPointer->isUp)
	    miSpriteRemoveCursor (pDev, pScreen);
	pPointer->pCursor = 0;
	return;
    }
    pPointer->shouldBeUp = TRUE;
    if (pPointer->x == x &&
	pPointer->y == y &&
	pPointer->pCursor == pCursor &&
	!pPointer->checkPixels)
    {
	return;
    }
    pPointer->x = x;
    pPointer->y = y;
    pPointer->pCacheWin = NullWindow;
    if (pPointer->checkPixels || pPointer->pCursor != pCursor)
    {
	pPointer->pCursor = pCursor;
	miSpriteFindColors (pPointer, pScreen);
    }
    if (pPointer->isUp) {
#if 0
        /* FIXME: Disabled for MPX, should be rewritten */
	int	sx, sy;
	/*
	 * check to see if the old saved region
	 * encloses the new sprite, in which case we use
	 * the flicker-free MoveCursor primitive.
	 */
	sx = pointer->x - (int)pCursor->bits->xhot;
	sy = pointer->y - (int)pCursor->bits->yhot;
	if (sx + (int) pCursor->bits->width >= pointer->saved.x1 &&
	    sx < pointer->saved.x2 &&
	    sy + (int) pCursor->bits->height >= pointer->saved.y1 &&
	    sy < pointer->saved.y2 &&
	    (int) pCursor->bits->width + (2 * SPRITE_PAD) ==
		pointer->saved.x2 - pointer->saved.x1 &&
	    (int) pCursor->bits->height + (2 * SPRITE_PAD) ==
		pointer->saved.y2 - pointer->saved.y1
	    )
	{
	    DamageDrawInternal (pScreen, TRUE);
	    miSpriteIsDown(pCursorInfo);
	    if (!(sx >= pointer->saved.x1 &&
                  sx + (int)pCursor->bits->width < pointer->saved.x2
                  && sy >= pointer->saved.y1 &&
                  sy + (int)pCursor->bits->height <
                                pointer->saved.y2))
            {
		int oldx1, oldy1, dx, dy;

		oldx1 = pointer->saved.x1;
		oldy1 = pointer->saved.y1;
		dx = oldx1 - (sx - SPRITE_PAD);
		dy = oldy1 - (sy - SPRITE_PAD);
		pointer->saved.x1 -= dx;
		pointer->saved.y1 -= dy;
		pointer->saved.x2 -= dx;
		pointer->saved.y2 -= dy;
		(void) (*pScreenPriv->funcs->ChangeSave) (pScreen,
				pointer->saved.x1,
 				pointer->saved.y1,
                                pointer->saved.x2 -
                                pointer->saved.x1,
                                pointer->saved.y2 -
                                pointer->saved.y1,
				dx, dy);
	    }
	    (void) (*pScreenPriv->funcs->MoveCursor) (pScreen, pCursor,
				  pointer->saved.x1,
 				  pointer->saved.y1,
                                  pointer->saved.x2 -
                                  pointer->saved.x1,
                                  pointer->saved.y2 -
                                  pointer->saved.y1,
				  sx - pointer->saved.x1,
				  sy - pointer->saved.y1,
				  pointer->colors[SOURCE_COLOR].pixel,
				  pointer->colors[MASK_COLOR].pixel);
	    miSpriteIsUp(pCursorInfo);
	    DamageDrawInternal (pScreen, FALSE);
	}
	else
#endif
	{
	    SPRITE_DEBUG (("SetCursor remove %d\n", pDev->id));
	    miSpriteRemoveCursor (pDev, pScreen);
	}
    }

    if (!pPointer->isUp && pPointer->pCursor)
    {
	SPRITE_DEBUG (("SetCursor restore %d\n", pDev->id));
        miSpriteSaveUnderCursor(pDev, pScreen);
	miSpriteRestoreCursor (pDev, pScreen);
    }

}

static void
miSpriteMoveCursor (DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y)
{
    miSpriteScreenPtr	pScreenPriv;
    CursorPtr pCursor;

    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    if (!IsMaster(pDev) && !pDev->u.master)
    {
        ErrorF("[mi] miSpriteMoveCursor called for floating device.\n");
        return;
    }
    pCursor = MISPRITE(pDev)->pCursor;

    miSpriteSetCursor (pDev, pScreen, pCursor, x, y);
}


static Bool
miSpriteDeviceCursorInitialize(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    miSpriteScreenPtr pScreenPriv;
    miCursorInfoPtr pCursorInfo;
    int ret = FALSE;

    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);

    pCursorInfo = xalloc(sizeof(miCursorInfoRec));
    if (!pCursorInfo)
        return FALSE;

    pCursorInfo->pCursor = NULL;
    pCursorInfo->x = 0;
    pCursorInfo->y = 0;
    pCursorInfo->isUp = FALSE;
    pCursorInfo->shouldBeUp = FALSE;
    pCursorInfo->pCacheWin = NullWindow;
    pCursorInfo->isInCacheWin = FALSE;
    pCursorInfo->checkPixels = TRUE;
    pCursorInfo->pScreen = FALSE;

    ret = (*pScreenPriv->funcs->DeviceCursorInitialize)(pDev, pScreen);
    if (!ret)
    {
        xfree(pCursorInfo);
        pCursorInfo = NULL;
    }
    dixSetPrivate(&pDev->devPrivates, miSpriteDevPrivatesKey, pCursorInfo);
    return ret;
}

static void
miSpriteDeviceCursorCleanup(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    if (DevHasCursor(pDev))
    {
        miSpriteScreenPtr pScreenPriv;
        pScreenPriv = dixLookupPrivate(&pScreen->devPrivates,
                                       miSpriteScreenKey);

        (*pScreenPriv->funcs->DeviceCursorCleanup)(pDev, pScreen);
    }
}

/*
 * undraw/draw cursor
 */

static void
miSpriteRemoveCursor (DeviceIntPtr pDev, ScreenPtr pScreen)
{
    miSpriteScreenPtr   pScreenPriv;
    miCursorInfoPtr     pCursorInfo;


    if (!IsMaster(pDev) && !pDev->u.master)
    {
        ErrorF("[mi] miSpriteRemoveCursor called for floating device.\n");
        return;
    }
    DamageDrawInternal (pScreen, TRUE);
    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    pCursorInfo = MISPRITE(pDev);

    miSpriteIsDown(pCursorInfo);
    pCursorInfo->pCacheWin = NullWindow;
    miSpriteDisableDamage(pScreen, pScreenPriv);
    if (!(*pScreenPriv->funcs->RestoreUnderCursor) (pDev,
                                         pScreen,
					 pCursorInfo->saved.x1,
                                         pCursorInfo->saved.y1,
                                         pCursorInfo->saved.x2 -
                                         pCursorInfo->saved.x1,
                                         pCursorInfo->saved.y2 -
                                         pCursorInfo->saved.y1))
    {
	miSpriteIsUp(pCursorInfo);
    }
    miSpriteEnableDamage(pScreen, pScreenPriv);
    DamageDrawInternal (pScreen, FALSE);
}

/*
 * Called from the block handler, saves area under cursor
 * before waiting for something to do.
 */

static void
miSpriteSaveUnderCursor(DeviceIntPtr pDev, ScreenPtr pScreen)
{
    miSpriteScreenPtr   pScreenPriv;
    int			x, y;
    CursorPtr		pCursor;
    miCursorInfoPtr     pCursorInfo;

    if (!IsMaster(pDev) && !pDev->u.master)
    {
        ErrorF("[mi] miSpriteSaveUnderCursor called for floating device.\n");
        return;
    }
    DamageDrawInternal (pScreen, TRUE);
    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    pCursorInfo = MISPRITE(pDev);

    miSpriteComputeSaved (pDev, pScreen);
    pCursor = pCursorInfo->pCursor;

    x = pCursorInfo->x - (int)pCursor->bits->xhot;
    y = pCursorInfo->y - (int)pCursor->bits->yhot;
    miSpriteDisableDamage(pScreen, pScreenPriv);

    (*pScreenPriv->funcs->SaveUnderCursor) (pDev,
                                      pScreen,
				      pCursorInfo->saved.x1,
				      pCursorInfo->saved.y1,
                                      pCursorInfo->saved.x2 -
                                      pCursorInfo->saved.x1,
                                      pCursorInfo->saved.y2 -
                                      pCursorInfo->saved.y1);
    SPRITE_DEBUG(("SaveUnderCursor %d\n", pDev->id));
    miSpriteEnableDamage(pScreen, pScreenPriv);
    DamageDrawInternal (pScreen, FALSE);
}


/*
 * Called from the block handler, restores the cursor
 * before waiting for something to do.
 */

static void
miSpriteRestoreCursor (DeviceIntPtr pDev, ScreenPtr pScreen)
{
    miSpriteScreenPtr   pScreenPriv;
    int			x, y;
    CursorPtr		pCursor;
    miCursorInfoPtr     pCursorInfo;

    if (!IsMaster(pDev) && !pDev->u.master)
    {
        ErrorF("[mi] miSpriteRestoreCursor called for floating device.\n");
        return;
    }

    DamageDrawInternal (pScreen, TRUE);
    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    pCursorInfo = MISPRITE(pDev);

    miSpriteComputeSaved (pDev, pScreen);
    pCursor = pCursorInfo->pCursor;

    x = pCursorInfo->x - (int)pCursor->bits->xhot;
    y = pCursorInfo->y - (int)pCursor->bits->yhot;
    miSpriteDisableDamage(pScreen, pScreenPriv);
    SPRITE_DEBUG(("RestoreCursor %d\n", pDev->id));
    if (pCursorInfo->checkPixels)
        miSpriteFindColors (pCursorInfo, pScreen);
    if ((*pScreenPriv->funcs->PutUpCursor) (pDev, pScreen,
                pCursor, x, y,
                pScreenPriv->colors[SOURCE_COLOR].pixel,
                pScreenPriv->colors[MASK_COLOR].pixel))
    {
        miSpriteIsUp(pCursorInfo);
        pCursorInfo->pScreen = pScreen;
    }
    miSpriteEnableDamage(pScreen, pScreenPriv);
    DamageDrawInternal (pScreen, FALSE);
}

/*
 * compute the desired area of the screen to save
 */

static void
miSpriteComputeSaved (DeviceIntPtr pDev, ScreenPtr pScreen)
{
    miSpriteScreenPtr   pScreenPriv;
    int		    x, y, w, h;
    int		    wpad, hpad;
    CursorPtr	    pCursor;
    miCursorInfoPtr pCursorInfo;

    if (!IsMaster(pDev) && !pDev->u.master)
    {
        ErrorF("[mi] miSpriteComputeSaved called for floating device.\n");
        return;
    }
    pScreenPriv = dixLookupPrivate(&pScreen->devPrivates, miSpriteScreenKey);
    pCursorInfo = MISPRITE(pDev);

    pCursor = pCursorInfo->pCursor;
    x = pCursorInfo->x - (int)pCursor->bits->xhot;
    y = pCursorInfo->y - (int)pCursor->bits->yhot;
    w = pCursor->bits->width;
    h = pCursor->bits->height;
    wpad = SPRITE_PAD;
    hpad = SPRITE_PAD;
    pCursorInfo->saved.x1 = x - wpad;
    pCursorInfo->saved.y1 = y - hpad;
    pCursorInfo->saved.x2 = pCursorInfo->saved.x1 + w + wpad * 2;
    pCursorInfo->saved.y2 = pCursorInfo->saved.y1 + h + hpad * 2;
}

