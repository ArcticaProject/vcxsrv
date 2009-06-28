
/*
   Copyright (C) 1998.  The XFree86 Project Inc.

   Written by Mark Vojkovich (mvojkovi@ucsd.edu)
*/

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "misc.h"
#include "xf86.h"
#include "xf86_OSproc.h"

#include <X11/X.h>
#include "scrnintstr.h"
#include "regionstr.h"
#include "windowstr.h"
#include "xf86str.h"
#include "migc.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "colormapst.h"
#include "cfb8_32.h"

#define IS_DIRTY 	1
#define IS_SHARED	2

/** Screen Functions **/

static Bool OverlayCloseScreen (int, ScreenPtr);
static Bool OverlayCreateGC(GCPtr pGC);
static Bool OverlayDestroyPixmap(PixmapPtr);
static PixmapPtr OverlayCreatePixmap(ScreenPtr, int, int, int, unsigned);
static Bool OverlayChangeWindowAttributes(WindowPtr, unsigned long);

/** Funcs **/
static void OverlayValidateGC(GCPtr, unsigned long, DrawablePtr);
static void OverlayChangeGC(GCPtr, unsigned long);
static void OverlayCopyGC(GCPtr, unsigned long, GCPtr);
static void OverlayDestroyGC(GCPtr);
static void OverlayChangeClip(GCPtr, int, pointer, int);
static void OverlayDestroyClip(GCPtr);
static void OverlayCopyClip(GCPtr, GCPtr);


static PixmapPtr OverlayRefreshPixmap(PixmapPtr);

static GCFuncs OverlayGCFuncs = {
   OverlayValidateGC, OverlayChangeGC, 
   OverlayCopyGC, OverlayDestroyGC,
   OverlayChangeClip, OverlayDestroyClip, 
   OverlayCopyClip
};


/** Pixmap Ops */
static void	 PixmapFillSpans(DrawablePtr, GCPtr, int, DDXPointPtr, int *,
				  int);
static void	 PixmapSetSpans(DrawablePtr, GCPtr, char *, DDXPointPtr,
				 int *, int, int);
static void	 PixmapPutImage(DrawablePtr, GCPtr, int, int, int, int, int,
				 int, int, char *);
static void	 PixmapPushPixels(GCPtr, PixmapPtr, DrawablePtr, int, int,
				   int, int);
static RegionPtr PixmapCopyArea(DrawablePtr, DrawablePtr, GCPtr, int, int,
				 int, int, int, int);
static RegionPtr PixmapCopyPlane(DrawablePtr, DrawablePtr, GCPtr, int, int,
				  int, int, int, int, unsigned long);
static void	 PixmapPolyPoint(DrawablePtr, GCPtr, int, int, xPoint *);
static void	 PixmapPolylines(DrawablePtr, GCPtr, int, int, DDXPointPtr);
static void	 PixmapPolySegment(DrawablePtr, GCPtr, int, xSegment *);
static void	 PixmapPolyRectangle(DrawablePtr, GCPtr, int, xRectangle *);
static void	 PixmapPolyArc(DrawablePtr, GCPtr, int, xArc *);
static void	 PixmapFillPolygon(DrawablePtr, GCPtr, int, int, int,
				    DDXPointPtr);
static void	 PixmapPolyFillRect(DrawablePtr, GCPtr, int, xRectangle *);
static void	 PixmapPolyFillArc(DrawablePtr, GCPtr, int, xArc *);
static int	 PixmapPolyText8(DrawablePtr, GCPtr, int, int, int, char *);
static int	 PixmapPolyText16(DrawablePtr, GCPtr, int, int, int,
				   unsigned short *);
static void	 PixmapImageText8(DrawablePtr, GCPtr, int, int, int, char *);
static void	 PixmapImageText16(DrawablePtr, GCPtr, int, int, int,
				    unsigned short *);
static void	 PixmapImageGlyphBlt(DrawablePtr, GCPtr, int, int,
				      unsigned int, CharInfoPtr *, pointer);
static void	 PixmapPolyGlyphBlt(DrawablePtr, GCPtr, int, int,
				     unsigned int, CharInfoPtr *, pointer);

static GCOps PixmapGCOps = {
    PixmapFillSpans, PixmapSetSpans, 
    PixmapPutImage, PixmapCopyArea, 
    PixmapCopyPlane, PixmapPolyPoint, 
    PixmapPolylines, PixmapPolySegment, 
    PixmapPolyRectangle, PixmapPolyArc, 
    PixmapFillPolygon, PixmapPolyFillRect, 
    PixmapPolyFillArc, PixmapPolyText8, 
    PixmapPolyText16, PixmapImageText8, 
    PixmapImageText16, PixmapImageGlyphBlt, 
    PixmapPolyGlyphBlt, PixmapPushPixels,
    {NULL}		/* devPrivate */
};


/** Window Ops **/
static void	 WindowFillSpans(DrawablePtr, GCPtr, int, DDXPointPtr, int *,
				  int);
static void	 WindowSetSpans(DrawablePtr, GCPtr, char *, DDXPointPtr,
				 int *, int, int);
static void	 WindowPutImage(DrawablePtr, GCPtr, int, int, int, int, int,
				 int, int, char *);
static void	 WindowPushPixels(GCPtr, PixmapPtr, DrawablePtr, int, int,
				   int, int);
static RegionPtr WindowCopyArea(DrawablePtr, DrawablePtr, GCPtr, int, int,
				 int, int, int, int);
static RegionPtr WindowCopyPlane(DrawablePtr, DrawablePtr, GCPtr, int, int,
				  int, int, int, int, unsigned long);
static void	 WindowPolyPoint(DrawablePtr, GCPtr, int, int, xPoint *);
static void	 WindowPolylines(DrawablePtr, GCPtr, int, int, DDXPointPtr);
static void	 WindowPolySegment(DrawablePtr, GCPtr, int, xSegment *);
static void	 WindowPolyRectangle(DrawablePtr, GCPtr, int, xRectangle *);
static void	 WindowPolyArc(DrawablePtr, GCPtr, int, xArc *);
static void	 WindowFillPolygon(DrawablePtr, GCPtr, int, int, int,
				    DDXPointPtr);
static void	 WindowPolyFillRect(DrawablePtr, GCPtr, int, xRectangle *);
static void	 WindowPolyFillArc(DrawablePtr, GCPtr, int, xArc *);
static int	 WindowPolyText8(DrawablePtr, GCPtr, int, int, int, char *);
static int	 WindowPolyText16(DrawablePtr, GCPtr, int, int, int,
				   unsigned short *);
static void	 WindowImageText8(DrawablePtr, GCPtr, int, int, int, char *);
static void	 WindowImageText16(DrawablePtr, GCPtr, int, int, int,
				    unsigned short *);
static void	 WindowImageGlyphBlt(DrawablePtr, GCPtr, int, int,
				      unsigned int, CharInfoPtr *, pointer);
static void	 WindowPolyGlyphBlt(DrawablePtr, GCPtr, int, int,
				     unsigned int, CharInfoPtr *, pointer);

static GCOps WindowGCOps = {
    WindowFillSpans, WindowSetSpans, 
    WindowPutImage, WindowCopyArea, 
    WindowCopyPlane, WindowPolyPoint, 
    WindowPolylines, WindowPolySegment, 
    WindowPolyRectangle, WindowPolyArc, 
    WindowFillPolygon, WindowPolyFillRect, 
    WindowPolyFillArc, WindowPolyText8, 
    WindowPolyText16, WindowImageText8, 
    WindowImageText16, WindowImageGlyphBlt, 
    WindowPolyGlyphBlt, WindowPushPixels,
    {NULL}		/* devPrivate */
};

/** privates **/

typedef struct {
   CloseScreenProcPtr		CloseScreen;
   CreateGCProcPtr		CreateGC;
   CreatePixmapProcPtr		CreatePixmap;
   DestroyPixmapProcPtr		DestroyPixmap;
   ChangeWindowAttributesProcPtr ChangeWindowAttributes;
   int				LockPrivate;
} OverlayScreenRec, *OverlayScreenPtr;

typedef struct {
   GCFuncs		*wrapFuncs;
   GCOps		*wrapOps;
   GCOps		*overlayOps;
   unsigned long 	fg;
   unsigned long 	bg;
   unsigned long 	pm;
   PixmapPtr		tile;
} OverlayGCRec, *OverlayGCPtr;

typedef struct {
   PixmapPtr		pix32;
   CARD32		dirty;
} OverlayPixmapRec, *OverlayPixmapPtr;


static DevPrivateKey OverlayScreenKey = &OverlayScreenKey;
static DevPrivateKey OverlayGCKey = &OverlayGCKey;
static DevPrivateKey OverlayPixmapKey = &OverlayPixmapKey;

/** Macros **/

#define TILE_EXISTS(pGC) (!(pGC)->tileIsPixel && (pGC)->tile.pixmap)

#define OVERLAY_GET_PIXMAP_PRIVATE(pPix) ((OverlayPixmapPtr) \
    dixLookupPrivate(&(pPix)->devPrivates, OverlayPixmapKey))

#define OVERLAY_GET_SCREEN_PRIVATE(pScreen) ((OverlayScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, OverlayScreenKey))

#define OVERLAY_GET_GC_PRIVATE(pGC) ((OverlayGCPtr) \
    dixLookupPrivate(&(pGC)->devPrivates, OverlayGCKey))

#define OVERLAY_GC_FUNC_PROLOGUE(pGC)\
    OverlayGCPtr pGCPriv = OVERLAY_GET_GC_PRIVATE(pGC);\
    (pGC)->funcs = pGCPriv->wrapFuncs;\
    if(pGCPriv->overlayOps) \
	(pGC)->ops = pGCPriv->wrapOps

#define OVERLAY_GC_FUNC_EPILOGUE(pGC)\
    pGCPriv->wrapFuncs = (pGC)->funcs;\
    (pGC)->funcs = &OverlayGCFuncs;\
    if(pGCPriv->overlayOps) { \
	pGCPriv->wrapOps = (pGC)->ops;\
	(pGC)->ops = pGCPriv->overlayOps;\
    }

#define WINDOW_GC_OP_PROLOGUE(pGC)\
    OverlayScreenPtr pScreenPriv = OVERLAY_GET_SCREEN_PRIVATE((pGC)->pScreen);\
    OverlayGCPtr pGCPriv = OVERLAY_GET_GC_PRIVATE(pGC);\
    unsigned long oldfg = (pGC)->fgPixel;\
    unsigned long oldbg = (pGC)->bgPixel;\
    unsigned long oldpm = (pGC)->planemask;\
    PixmapPtr oldtile = (pGC)->tile.pixmap;\
    (pGC)->fgPixel = pGCPriv->fg;\
    (pGC)->bgPixel = pGCPriv->bg;\
    (pGC)->planemask = pGCPriv->pm;\
    if(pGCPriv->tile) (pGC)->tile.pixmap = pGCPriv->tile;\
    (pGC)->funcs = pGCPriv->wrapFuncs;\
    (pGC)->ops = pGCPriv->wrapOps;\
    pScreenPriv->LockPrivate++
    

#define WINDOW_GC_OP_EPILOGUE(pGC)\
    pGCPriv->wrapOps = (pGC)->ops;\
    pGCPriv->wrapFuncs = (pGC)->funcs;\
    (pGC)->fgPixel = oldfg;\
    (pGC)->bgPixel = oldbg;\
    (pGC)->planemask = oldpm;\
    (pGC)->tile.pixmap = oldtile;\
    (pGC)->funcs = &OverlayGCFuncs;\
    (pGC)->ops   = &WindowGCOps;\
    pScreenPriv->LockPrivate--


#define PIXMAP_GC_OP_PROLOGUE(pGC)\
    OverlayGCPtr pGCPriv = OVERLAY_GET_GC_PRIVATE(pGC);\
    OverlayPixmapPtr pPixPriv = OVERLAY_GET_PIXMAP_PRIVATE((PixmapPtr)pDraw);\
    pGC->funcs = pGCPriv->wrapFuncs;\
    pGC->ops = pGCPriv->wrapOps
    
#define PIXMAP_GC_OP_EPILOGUE(pGC)\
    pGCPriv->wrapOps = pGC->ops;\
    pGC->funcs = &OverlayGCFuncs;\
    pGC->ops = &PixmapGCOps;\
    pPixPriv->dirty |= IS_DIRTY
    

Bool
xf86Overlay8Plus32Init (ScreenPtr pScreen)
{
    OverlayScreenPtr pScreenPriv;

    if (!dixRequestPrivate(OverlayGCKey, sizeof(OverlayGCRec)))
	return FALSE;

    if (!dixRequestPrivate(OverlayPixmapKey, sizeof(OverlayPixmapRec)))
	return FALSE;

    if (!(pScreenPriv = xalloc(sizeof(OverlayScreenRec))))
	return FALSE;

    dixSetPrivate(&pScreen->devPrivates, OverlayScreenKey, pScreenPriv);

    pScreenPriv->CreateGC = pScreen->CreateGC;
    pScreenPriv->CloseScreen = pScreen->CloseScreen;
    pScreenPriv->CreatePixmap = pScreen->CreatePixmap;
    pScreenPriv->DestroyPixmap = pScreen->DestroyPixmap;
    pScreenPriv->ChangeWindowAttributes = pScreen->ChangeWindowAttributes;

    pScreen->CreateGC = OverlayCreateGC;
    pScreen->CloseScreen = OverlayCloseScreen;
    pScreen->CreatePixmap = OverlayCreatePixmap; 
    pScreen->DestroyPixmap = OverlayDestroyPixmap; 
    pScreen->ChangeWindowAttributes = OverlayChangeWindowAttributes; 

    pScreenPriv->LockPrivate = 0; 

    /* allocate the key in the default map */
    if(pScreen->defColormap) {
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	ColormapPtr pmap;
	xColorItem color;

	pmap = (ColormapPtr)LookupIDByType(pScreen->defColormap, RT_COLORMAP);

	pmap->red[pScrn->colorKey].refcnt = AllocPrivate;
	pmap->red[pScrn->colorKey].fShared = FALSE;
	pmap->freeRed--;
	
	color.red = color.blue = color.green = 0;
	color.pixel = pScrn->colorKey;
	color.flags = DoRed | DoGreen | DoBlue;

	StoreColors(pmap, 1, &color);
    }

    return TRUE;
}


/*********************** Screen Funcs ***********************/

Bool
OverlayCreateGC(GCPtr pGC)
{
    ScreenPtr    pScreen = pGC->pScreen;
    OverlayGCPtr pGCPriv = OVERLAY_GET_GC_PRIVATE(pGC);
    OverlayScreenPtr pScreenPriv = OVERLAY_GET_SCREEN_PRIVATE(pScreen);
    Bool ret;

    pScreen->CreateGC = pScreenPriv->CreateGC;

    if((ret = (*pScreen->CreateGC)(pGC)) && (pGC->depth != 1)) {
	pGCPriv->wrapFuncs = pGC->funcs;
	pGC->funcs = &OverlayGCFuncs;
	pGCPriv->wrapOps = NULL;
	pGCPriv->overlayOps = NULL;
	pGCPriv->tile = NULL;
    }

    pScreen->CreateGC = OverlayCreateGC;

    return ret;
}

static PixmapPtr 
OverlayCreatePixmap(ScreenPtr pScreen, int w, int h, int depth,
		    unsigned usage_hint)
{
    OverlayScreenPtr pScreenPriv = OVERLAY_GET_SCREEN_PRIVATE(pScreen);
    PixmapPtr pPix;
    
    pScreen->CreatePixmap = pScreenPriv->CreatePixmap;
    pPix = (*pScreen->CreatePixmap) (pScreen, w, h, depth, usage_hint);
    pScreen->CreatePixmap = OverlayCreatePixmap;

    /* We initialize all the privates */
    if(pPix) {
	OverlayPixmapPtr pPriv = OVERLAY_GET_PIXMAP_PRIVATE(pPix);
        pPriv->pix32 = NULL;
        pPriv->dirty = IS_DIRTY;
	if(!w || !h)
	   pPriv->dirty |= IS_SHARED;
    }

    return pPix;
}

static Bool
OverlayDestroyPixmap(PixmapPtr pPix)
{
    ScreenPtr pScreen = pPix->drawable.pScreen;
    OverlayScreenPtr pScreenPriv = OVERLAY_GET_SCREEN_PRIVATE(pScreen);
    Bool result;

    pScreen->DestroyPixmap = pScreenPriv->DestroyPixmap;

    if((pPix->refcnt == 1) && (pPix->drawable.bitsPerPixel == 8)) {
	OverlayPixmapPtr pPriv = OVERLAY_GET_PIXMAP_PRIVATE(pPix);
	if(pPriv->pix32) {
	   if(pPriv->pix32->refcnt != 1)
	     ErrorF("Warning! private pix refcnt = %i\n", pPriv->pix32->refcnt);
	   (*pScreen->DestroyPixmap)(pPriv->pix32);
	}
	pPriv->pix32 = NULL;
    }

    result = (*pScreen->DestroyPixmap) (pPix);
    pScreen->DestroyPixmap = OverlayDestroyPixmap;

    return result;
}

static Bool
OverlayCloseScreen (int i, ScreenPtr pScreen)
{
    OverlayScreenPtr pScreenPriv = OVERLAY_GET_SCREEN_PRIVATE(pScreen);

    pScreen->CreateGC = pScreenPriv->CreateGC;
    pScreen->CloseScreen = pScreenPriv->CloseScreen;
    pScreen->CreatePixmap = pScreenPriv->CreatePixmap;
    pScreen->DestroyPixmap = pScreenPriv->DestroyPixmap;
    pScreen->ChangeWindowAttributes = pScreenPriv->ChangeWindowAttributes;

    xfree ((pointer) pScreenPriv);

    return (*pScreen->CloseScreen) (i, pScreen);
}



static Bool
OverlayChangeWindowAttributes (WindowPtr pWin, unsigned long mask)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    OverlayScreenPtr pScreenPriv = OVERLAY_GET_SCREEN_PRIVATE(pScreen);
    Bool result;    

    if(pWin->drawable.depth == 8) {
	if((mask & CWBackPixmap) && 
	   (pWin->backgroundState == BackgroundPixmap)) 	
		OverlayRefreshPixmap(pWin->background.pixmap);

	if((mask & CWBorderPixmap) && !pWin->borderIsPixel)
		OverlayRefreshPixmap(pWin->border.pixmap);
    }

    pScreen->ChangeWindowAttributes = pScreenPriv->ChangeWindowAttributes;
    result = (*pScreen->ChangeWindowAttributes) (pWin, mask);
    pScreen->ChangeWindowAttributes = OverlayChangeWindowAttributes;

    return result;
}

/*********************** GC Funcs *****************************/


static PixmapPtr
OverlayRefreshPixmap(PixmapPtr pix8) 
{
    OverlayPixmapPtr pixPriv = OVERLAY_GET_PIXMAP_PRIVATE(pix8);
    ScreenPtr pScreen = pix8->drawable.pScreen;

    if(!pixPriv->pix32) {
	PixmapPtr newPix;

	newPix = (*pScreen->CreatePixmap)(pScreen, pix8->drawable.width,
		pix8->drawable.height, 24, 0);
	newPix->drawable.depth = 8;  /* Bad Mark! Bad Mark! */
        pixPriv->pix32 = newPix;
    }

    if(pixPriv->dirty) {
	OverlayScreenPtr pScreenPriv = OVERLAY_GET_SCREEN_PRIVATE(pScreen);
	GCPtr pGC;

	pGC = GetScratchGC(8, pScreen);

	pScreenPriv->LockPrivate++;  /* don't modify this one */
	ValidateGC((DrawablePtr)pixPriv->pix32, pGC);

	(*pGC->ops->CopyArea)((DrawablePtr)pix8, (DrawablePtr)pixPriv->pix32,
		pGC, 0, 0, pix8->drawable.width, pix8->drawable.height, 0, 0);  
	pScreenPriv->LockPrivate--;
	FreeScratchGC(pGC);

	pixPriv->dirty &= ~IS_DIRTY;
	pixPriv->pix32->drawable.serialNumber = NEXT_SERIAL_NUMBER;    
    }

    return pixPriv->pix32;
}


static void
OverlayValidateGC(
   GCPtr         pGC,
   unsigned long changes,
   DrawablePtr   pDraw 
){
    OverlayScreenPtr pScreenPriv = OVERLAY_GET_SCREEN_PRIVATE(pGC->pScreen);
    OVERLAY_GC_FUNC_PROLOGUE (pGC);

    if(pScreenPriv->LockPrivate < 0) {
	ErrorF("Something is wrong in OverlayValidateGC!\n");
	pScreenPriv->LockPrivate = 0;
    }

    if(pGC->depth == 24) {
	unsigned long oldpm = pGC->planemask;
	pGCPriv->overlayOps = NULL;

	if(pDraw->type == DRAWABLE_WINDOW)
	   pGC->planemask &= 0x00ffffff;
	else
	   pGC->planemask |= 0xff000000; 

        if(oldpm != pGC->planemask) changes |= GCPlaneMask;

	(*pGC->funcs->ValidateGC)(pGC, changes, pDraw);

    } else { /* depth == 8 */
	unsigned long newChanges = 0;

	if(pDraw->bitsPerPixel == 32) {
	
	    if(pGC->fillStyle == FillTiled)
		pGCPriv->tile = OverlayRefreshPixmap(pGC->tile.pixmap);
	    else pGCPriv->tile = NULL;

	    if(pGCPriv->overlayOps != &WindowGCOps) {
		newChanges = GCForeground | GCBackground | GCPlaneMask;
		if(pGCPriv->tile) 
		    newChanges |= GCTile;
	    }
	    pGCPriv->overlayOps = &WindowGCOps;

	    if(!pScreenPriv->LockPrivate) {
		unsigned long oldfg = pGC->fgPixel;
		unsigned long oldbg = pGC->bgPixel;
		unsigned long oldpm = pGC->planemask;
		PixmapPtr oldtile = pGC->tile.pixmap;

		pGC->fgPixel = pGCPriv->fg = oldfg << 24;
		pGC->bgPixel = pGCPriv->bg = oldbg << 24;
		pGC->planemask = pGCPriv->pm = oldpm << 24;
		if(pGCPriv->tile)
		    pGC->tile.pixmap = pGCPriv->tile;

		(*pGC->funcs->ValidateGC)(pGC, changes | newChanges, pDraw);

		pGC->fgPixel = oldfg;
		pGC->bgPixel = oldbg;
		pGC->planemask = oldpm;
		pGC->tile.pixmap = oldtile;
	    } else {
		pGCPriv->fg = pGC->fgPixel;
		pGCPriv->bg = pGC->bgPixel;
		pGCPriv->pm = pGC->planemask;

		(*pGC->funcs->ValidateGC)(pGC, changes | newChanges, pDraw);
	    }

	} else {  /* bitsPerPixel == 8 */
	    if(pGCPriv->overlayOps == &WindowGCOps) {
		newChanges = GCForeground | GCBackground | GCPlaneMask;
		if(pGCPriv->tile) 
		    newChanges |= GCTile;
	    }
	    pGCPriv->overlayOps = &PixmapGCOps;

	    (*pGC->funcs->ValidateGC)(pGC, changes | newChanges, pDraw);
	}
    }

    OVERLAY_GC_FUNC_EPILOGUE (pGC);
}


static void
OverlayDestroyGC(GCPtr pGC)
{
    OVERLAY_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->DestroyGC)(pGC);
    OVERLAY_GC_FUNC_EPILOGUE (pGC);
}

static void
OverlayChangeGC (
    GCPtr	    pGC,
    unsigned long   mask
){
    OVERLAY_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeGC) (pGC, mask);
    OVERLAY_GC_FUNC_EPILOGUE (pGC);
}

static void
OverlayCopyGC (
    GCPtr	    pGCSrc, 
    unsigned long   mask,
    GCPtr	    pGCDst
){
    OVERLAY_GC_FUNC_PROLOGUE (pGCDst);
    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    OVERLAY_GC_FUNC_EPILOGUE (pGCDst);
}
static void
OverlayChangeClip (
    GCPtr   pGC,
    int		type,
    pointer	pvalue,
    int		nrects
){
    OVERLAY_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);
    OVERLAY_GC_FUNC_EPILOGUE (pGC);
}

static void
OverlayCopyClip(GCPtr pgcDst, GCPtr pgcSrc)
{
    OVERLAY_GC_FUNC_PROLOGUE (pgcDst);
    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);
    OVERLAY_GC_FUNC_EPILOGUE (pgcDst);
}

static void
OverlayDestroyClip(GCPtr pGC)
{
    OVERLAY_GC_FUNC_PROLOGUE (pGC);
    (* pGC->funcs->DestroyClip)(pGC);
    OVERLAY_GC_FUNC_EPILOGUE (pGC);
}



/******************* Window GC ops ***********************/

static void
WindowFillSpans(
    DrawablePtr pDraw,
    GC		*pGC,
    int		nInit,	
    DDXPointPtr pptInit,	
    int *pwidthInit,		
    int fSorted 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->FillSpans)(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowSetSpans(
    DrawablePtr		pDraw,
    GCPtr		pGC,
    char		*pcharsrc,
    register DDXPointPtr ppt,
    int			*pwidth,
    int			nspans,
    int			fSorted 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->SetSpans)(pDraw, pGC, pcharsrc, ppt, pwidth, nspans, fSorted);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowPutImage(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		depth, 
    int x, int y, int w, int h,
    int		leftPad,
    int		format,
    char 	*pImage 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PutImage)(pDraw, pGC, depth, x, y, w, h, 
						leftPad, format, pImage);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static RegionPtr
WindowCopyArea(
    DrawablePtr pSrc,
    DrawablePtr pDst,
    GC *pGC,
    int srcx, int srcy,
    int width, int height,
    int dstx, int dsty 
){
    RegionPtr ret;

    WINDOW_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->CopyArea)(pSrc, pDst,
            pGC, srcx, srcy, width, height, dstx, dsty);
    WINDOW_GC_OP_EPILOGUE(pGC);
    return ret;
}

static RegionPtr
WindowCopyPlane(
    DrawablePtr	pSrc,
    DrawablePtr	pDst,
    GCPtr pGC,
    int	srcx, int srcy,
    int	width, int height,
    int	dstx, int dsty,
    unsigned long bitPlane 
){
    RegionPtr ret;

    WINDOW_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->CopyPlane)(pSrc, pDst,
	       pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    WINDOW_GC_OP_EPILOGUE(pGC);
    return ret;
}

static void
WindowPolyPoint(
    DrawablePtr pDraw,
    GCPtr pGC,
    int mode,
    int npt,
    xPoint *pptInit
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyPoint)(pDraw, pGC, mode, npt, pptInit);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowPolylines(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		mode,		
    int		npt,		
    DDXPointPtr pptInit 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->Polylines)(pDraw, pGC, mode, npt, pptInit);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void 
WindowPolySegment(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nseg,
    xSegment	*pSeg 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolySegment)(pDraw, pGC, nseg, pSeg);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowPolyRectangle(
    DrawablePtr  pDraw,
    GCPtr        pGC,
    int	         nRectsInit,
    xRectangle  *pRectsInit 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyRectangle)(pDraw, pGC, nRectsInit, pRectsInit);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowPolyArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyArc)(pDraw, pGC, narcs, parcs);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowFillPolygon(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		shape,
    int		mode,
    int		count,
    DDXPointPtr	ptsIn 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->FillPolygon)(pDraw, pGC, shape, mode, count, ptsIn);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void 
WindowPolyFillRect(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nrectFill, 
    xRectangle	*prectInit 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyFillRect)(pDraw, pGC, nrectFill, prectInit);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowPolyFillArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyFillArc)(pDraw, pGC, narcs, parcs);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static int
WindowPolyText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int 	y,
    int 	count,
    char	*chars 
){
    int ret;

    WINDOW_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->PolyText8)(pDraw, pGC, x, y, count, chars);
    WINDOW_GC_OP_EPILOGUE(pGC);
    return ret;
}

static int
WindowPolyText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars 
){
    int ret;

    WINDOW_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->PolyText16)(pDraw, pGC, x, y, count, chars);
    WINDOW_GC_OP_EPILOGUE(pGC);
    return ret;
}

static void
WindowImageText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int		y,
    int 	count,
    char	*chars 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageText8)(pDraw, pGC, x, y, count, chars);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowImageText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageText16)(pDraw, pGC, x, y, count, chars);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowImageGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int xInit, int yInit,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, xInit, yInit, nglyph, 
					ppci, pglyphBase);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowPolyGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int xInit, int yInit,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyGlyphBlt)(pDraw, pGC, xInit, yInit, nglyph, 
						ppci, pglyphBase);
    WINDOW_GC_OP_EPILOGUE(pGC);
}

static void
WindowPushPixels(
    GCPtr	pGC,
    PixmapPtr	pBitMap,
    DrawablePtr pDraw,
    int	dx, int dy, int xOrg, int yOrg 
){
    WINDOW_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PushPixels)(pGC, pBitMap, pDraw, dx, dy, xOrg, yOrg);
    WINDOW_GC_OP_EPILOGUE(pGC);
}


/******************* Pixmap GC ops ***********************/

static void
PixmapFillSpans(
    DrawablePtr pDraw,
    GC		*pGC,
    int		nInit,	
    DDXPointPtr pptInit,	
    int *pwidthInit,		
    int fSorted 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->FillSpans)(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapSetSpans(
    DrawablePtr		pDraw,
    GCPtr		pGC,
    char		*pcharsrc,
    register DDXPointPtr ppt,
    int			*pwidth,
    int			nspans,
    int			fSorted 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->SetSpans)(pDraw, pGC, pcharsrc, ppt, pwidth, nspans, fSorted);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapPutImage(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		depth, 
    int x, int y, int w, int h,
    int		leftPad,
    int		format,
    char 	*pImage 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PutImage)(pDraw, pGC, depth, x, y, w, h, 
						leftPad, format, pImage);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static RegionPtr
PixmapCopyArea(
    DrawablePtr pSrc,
    DrawablePtr pDraw,
    GC *pGC,
    int srcx, int srcy,
    int width, int height,
    int dstx, int dsty 
){
    RegionPtr ret;

    PIXMAP_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->CopyArea)(pSrc, pDraw,
            pGC, srcx, srcy, width, height, dstx, dsty);
    PIXMAP_GC_OP_EPILOGUE(pGC);
    return ret;
}

static RegionPtr
PixmapCopyPlane(
    DrawablePtr	pSrc,
    DrawablePtr	pDraw,
    GCPtr pGC,
    int	srcx, int srcy,
    int	width, int height,
    int	dstx, int dsty,
    unsigned long bitPlane 
){
    RegionPtr ret;

    PIXMAP_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->CopyPlane)(pSrc, pDraw,
	       pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    PIXMAP_GC_OP_EPILOGUE(pGC);
    return ret;
}

static void
PixmapPolyPoint(
    DrawablePtr pDraw,
    GCPtr pGC,
    int mode,
    int npt,
    xPoint *pptInit
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyPoint)(pDraw, pGC, mode, npt, pptInit);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapPolylines(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		mode,		
    int		npt,		
    DDXPointPtr pptInit 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->Polylines)(pDraw, pGC, mode, npt, pptInit);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void 
PixmapPolySegment(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nseg,
    xSegment	*pSeg 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolySegment)(pDraw, pGC, nseg, pSeg);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapPolyRectangle(
    DrawablePtr  pDraw,
    GCPtr        pGC,
    int	         nRectsInit,
    xRectangle  *pRectsInit 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyRectangle)(pDraw, pGC, nRectsInit, pRectsInit);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapPolyArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyArc)(pDraw, pGC, narcs, parcs);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapFillPolygon(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		shape,
    int		mode,
    int		count,
    DDXPointPtr	ptsIn 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->FillPolygon)(pDraw, pGC, shape, mode, count, ptsIn);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void 
PixmapPolyFillRect(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nrectFill, 
    xRectangle	*prectInit 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyFillRect)(pDraw, pGC, nrectFill, prectInit);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapPolyFillArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyFillArc)(pDraw, pGC, narcs, parcs);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static int
PixmapPolyText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int 	y,
    int 	count,
    char	*chars 
){
    int ret;

    PIXMAP_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->PolyText8)(pDraw, pGC, x, y, count, chars);
    PIXMAP_GC_OP_EPILOGUE(pGC);
    return ret;
}

static int
PixmapPolyText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars 
){
    int ret;

    PIXMAP_GC_OP_PROLOGUE(pGC);
    ret = (*pGC->ops->PolyText16)(pDraw, pGC, x, y, count, chars);
    PIXMAP_GC_OP_EPILOGUE(pGC);
    return ret;
}

static void
PixmapImageText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int		y,
    int 	count,
    char	*chars 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageText8)(pDraw, pGC, x, y, count, chars);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapImageText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageText16)(pDraw, pGC, x, y, count, chars);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapImageGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int xInit, int yInit,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, xInit, yInit, nglyph, 
					ppci, pglyphBase);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapPolyGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int xInit, int yInit,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PolyGlyphBlt)(pDraw, pGC, xInit, yInit, nglyph, 
						ppci, pglyphBase);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}

static void
PixmapPushPixels(
    GCPtr	pGC,
    PixmapPtr	pBitMap,
    DrawablePtr pDraw,
    int	dx, int dy, int xOrg, int yOrg 
){
    PIXMAP_GC_OP_PROLOGUE(pGC);
    (*pGC->ops->PushPixels)(pGC, pBitMap, pDraw, dx, dy, xOrg, yOrg);
    PIXMAP_GC_OP_EPILOGUE(pGC);
}


