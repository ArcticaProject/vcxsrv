

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "misc.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "resource.h"
#include "colormap.h"
#include "colormapst.h"
#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb32.h"
#include "cfb8_32.h"
#include "mi.h"
#include "micmap.h"
#include "mistruct.h"
#include "dix.h"
#include "mibstore.h"
#include "mioverlay.h"
#include "xf86.h"
#include "xf86str.h"
#include "globals.h"

/* CAUTION:  We require that cfb8 and cfb32 were NOT 
	compiled with CFB_NEED_SCREEN_PRIVATE */

static DevPrivateKey cfb8_32GCPrivateKey = &cfb8_32GCPrivateKey;
DevPrivateKey cfb8_32GetGCPrivateKey(void)
{
    return cfb8_32GCPrivateKey;
}

static DevPrivateKey cfb8_32ScreenPrivateKey = &cfb8_32ScreenPrivateKey;
DevPrivateKey cfb8_32GetScreenPrivateKey(void)
{
    return cfb8_32ScreenPrivateKey;
}

static Bool
cfb8_32AllocatePrivates(ScreenPtr pScreen)
{
   cfb8_32ScreenPtr pScreenPriv;

   if (!(pScreenPriv = xalloc(sizeof(cfb8_32ScreenRec))))
        return FALSE;

   dixSetPrivate(&pScreen->devPrivates, cfb8_32ScreenPrivateKey, pScreenPriv);
   
   
   /* All cfb will have the same GC and Window private indicies */
   if(!mfbAllocatePrivates(pScreen, &cfbGCPrivateKey))
	return FALSE;

   if(!dixRequestPrivate(cfbGCPrivateKey, sizeof(cfbPrivGC)))
        return FALSE;

   if(!dixRequestPrivate(cfb8_32GCPrivateKey, sizeof(cfb8_32GCRec)))
        return FALSE;

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

static Bool
cfb8_32SetupScreen(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize, int ysize,	/* in pixels */
    int dpix, int dpiy,		/* dots per inch */
    int width			/* pixel width of frame buffer */
){
    if (!cfb8_32AllocatePrivates(pScreen))
	return FALSE;
    pScreen->defColormap = FakeClientID(0);
    /* let CreateDefColormap do whatever it wants for pixels */ 
    pScreen->blackPixel = pScreen->whitePixel = (Pixel) 0;
    pScreen->QueryBestSize = mfbQueryBestSize;
    /* SaveScreen */
    pScreen->GetImage = cfb8_32GetImage;
    pScreen->GetSpans = cfb8_32GetSpans;
    pScreen->CreateWindow = cfb8_32CreateWindow;
    pScreen->DestroyWindow = cfb8_32DestroyWindow;	
    pScreen->PositionWindow = cfb8_32PositionWindow;
    pScreen->ChangeWindowAttributes = cfb8_32ChangeWindowAttributes;
    pScreen->RealizeWindow = cfb32MapWindow;			/* OK */
    pScreen->UnrealizeWindow = cfb32UnmapWindow;		/* OK */
    pScreen->CopyWindow = cfb8_32CopyWindow;
    pScreen->CreatePixmap = cfb32CreatePixmap;			/* OK */
    pScreen->DestroyPixmap = cfb32DestroyPixmap; 		/* OK */
    pScreen->RealizeFont = mfbRealizeFont;
    pScreen->UnrealizeFont = mfbUnrealizeFont;
    pScreen->CreateGC = cfb8_32CreateGC;
    pScreen->CreateColormap = miInitializeColormap;
    pScreen->DestroyColormap = DestroyColormapNoop;
    pScreen->InstallColormap = miInstallColormap;
    pScreen->UninstallColormap = miUninstallColormap;
    pScreen->ListInstalledColormaps = miListInstalledColormaps;
    pScreen->StoreColors = StoreColorsNoop;
    pScreen->ResolveColor = miResolveColor;
    pScreen->BitmapToRegion = mfbPixmapToRegion;

    mfbRegisterCopyPlaneProc (pScreen, cfb8_32CopyPlane);
    return TRUE;
}

typedef struct {
    pointer pbits; 
    int width;   
} miScreenInitParmsRec, *miScreenInitParmsPtr;

static Bool
cfb8_32CreateScreenResources(ScreenPtr pScreen)
{
    miScreenInitParmsPtr pScrInitParms;
    int pitch;
    Bool retval;

    /* get the pitch before mi destroys it */
    pScrInitParms = (miScreenInitParmsPtr)pScreen->devPrivate;
    pitch = pScrInitParms->width << 2;

    if((retval = miCreateScreenResources(pScreen))) {
	/* fix the screen pixmap */
	PixmapPtr pPix = (PixmapPtr)pScreen->devPrivate;
	pPix->drawable.bitsPerPixel = 32;
	pPix->drawable.depth = 8;
	pPix->devKind = pitch;
    }

    return retval;
}


static Bool
cfb8_32CloseScreen (int i, ScreenPtr pScreen)
{
    cfb8_32ScreenPtr pScreenPriv = CFB8_32_GET_SCREEN_PRIVATE(pScreen);
    if(pScreenPriv->visualData)
	xfree(pScreenPriv->visualData);

    xfree((pointer) pScreenPriv);
    dixSetPrivate(&pScreen->devPrivates, cfb8_32ScreenPrivateKey, NULL);

    return(cfb32CloseScreen(i, pScreen));
}

static void
cfb8_32TransFunc(
    ScreenPtr pScreen,
    int nbox,
    BoxPtr pbox
){
    cfb8_32FillBoxSolid8(&(WindowTable[pScreen->myNum]->drawable), 
			nbox, pbox, xf86Screens[pScreen->myNum]->colorKey);
}

static Bool
cfb8_32InOverlayFunc(WindowPtr pWin)
{
   return (pWin->drawable.depth == 8);
}

static Bool
cfb8_32FinishScreenInit(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize, int ysize,	/* in pixels */
    int dpix, int dpiy,		/* dots per inch */
    int width			/* pixel width of frame buffer */
){
    VisualPtr	visuals;
    DepthPtr	depths;
    int		nvisuals;
    int		ndepths;
    int		rootdepth;
    VisualID	defaultVisual;

    rootdepth = 0;
    if (!miInitVisuals (&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
			 &defaultVisual,((unsigned long)1<<(32-1)), 8, -1))
	return FALSE;
    if (! miScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, width,
			rootdepth, ndepths, depths,
			defaultVisual, nvisuals, visuals))
	return FALSE;

    pScreen->CreateScreenResources = cfb8_32CreateScreenResources;
    pScreen->CloseScreen = cfb8_32CloseScreen;
    pScreen->GetScreenPixmap = cfb32GetScreenPixmap; 	/* OK */
    pScreen->SetScreenPixmap = cfb32SetScreenPixmap;	/* OK */

    if (! miInitOverlay(pScreen, cfb8_32InOverlayFunc, cfb8_32TransFunc))
	return FALSE;

    return TRUE;
}

static void
cfb8_32EnableDisableFBAccess (
    int index,
    Bool enable
){
    ScreenPtr pScreen = screenInfo.screens[index];
    cfb8_32ScreenPtr pScreenPriv = CFB8_32_GET_SCREEN_PRIVATE(pScreen);
    
    miOverlaySetRootClip(pScreen, enable);

    (*pScreenPriv->EnableDisableFBAccess) (index, enable);
}

static Atom overlayVisualsAtom;

typedef struct {
    CARD32 overlay_visual;
    CARD32 transparent_type;
    CARD32 value;
    CARD32 layer;
} overlayVisualRec;

static void
cfb8_32SetupVisuals (ScreenPtr pScreen)
{
    cfb8_32ScreenPtr pScreenPriv = CFB8_32_GET_SCREEN_PRIVATE(pScreen);
    char atomString[] = {"SERVER_OVERLAY_VISUALS"};
    overlayVisualRec *overlayVisuals;
    VisualID *visuals = NULL;
    int numVisuals = 0;
    DepthPtr pDepth = pScreen->allowedDepths;
    int numDepths = pScreen->numDepths;
    int i;

    /* find depth 8 visuals */
    for(i = 0; i < numDepths; i++, pDepth++) {
	if(pDepth->depth == 8) {
	    numVisuals = pDepth->numVids;
	    visuals = pDepth->vids;
	    break;
	}
    }
    
    if(!numVisuals || !visuals) {
	ErrorF("No overlay visuals found!\n");
	return;
    }

    if(!(overlayVisuals = xalloc(numVisuals * sizeof(overlayVisualRec))))
	return;

    for(i = 0; i < numVisuals; i++) {
	overlayVisuals[i].overlay_visual = visuals[i];
	overlayVisuals[i].transparent_type = 1; /* transparent pixel */
	overlayVisuals[i].value = pScreenPriv->key;
	overlayVisuals[i].layer = 1;
    }    

    overlayVisualsAtom = MakeAtom(atomString, sizeof(atomString) - 1, TRUE);
    xf86RegisterRootWindowProperty(pScreen->myNum, overlayVisualsAtom,
			overlayVisualsAtom, 32, numVisuals * 4, overlayVisuals);
    pScreenPriv->visualData = (pointer)overlayVisuals;
}

Bool
cfb8_32ScreenInit(
    ScreenPtr pScreen,
    pointer pbits,		/* pointer to screen bitmap */
    int xsize, int ysize,	/* in pixels */
    int dpix, int dpiy,		/* dots per inch */
    int w			/* pixel width of frame buffer */
){
    cfb8_32ScreenPtr pScreenPriv;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

    if (!cfb8_32SetupScreen(pScreen, pbits, xsize, ysize, dpix, dpiy, w))
	return FALSE;

    pScreenPriv = CFB8_32_GET_SCREEN_PRIVATE(pScreen);
    pScreenPriv->key = pScrn->colorKey;
    pScreenPriv->visualData = NULL;


    pScreenPriv->EnableDisableFBAccess = pScrn->EnableDisableFBAccess;
    pScrn->EnableDisableFBAccess = cfb8_32EnableDisableFBAccess;


    if(cfb8_32FinishScreenInit(pScreen, pbits, xsize, ysize, dpix, dpiy, w))
    { 
	cfb8_32SetupVisuals(pScreen); 
	return TRUE;
    }
    return FALSE;
}
