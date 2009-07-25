#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "misc.h"
#include "xf86.h"
#include "xf86_OSproc.h"

#include <X11/X.h>
#include "colormapst.h"
#include "scrnintstr.h"
#include "screenint.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "pixmap.h"
#include "windowstr.h"
#include "window.h"
#include "xf86str.h"
#include "xf86RAC.h"
#include "mipointer.h"
#include "mipointrst.h"
#ifdef RENDER
# include "picturestr.h"
#endif

#ifdef DEBUG
#define DPRINT_S(x,y) ErrorF(x ": %i\n",y);
#define DPRINT(x) ErrorF(x "\n");
#else
#define DPRINT_S(x,y)
#define DPRINT(x)
#endif

#define WRAP_SCREEN(x,y)     {pScreenPriv->x = pScreen->x;\
                              pScreen->x = y;}
#define WRAP_SCREEN_COND(x,y,cond) \
                             {pScreenPriv->x = pScreen->x;\
                                if (flag & (cond))\
                                  pScreen->x = y;}
#define UNWRAP_SCREEN(x)    pScreen->x = pScreenPriv->x

#define SCREEN_PROLOG(x) pScreen->x = ((RACScreenPtr) \
    dixLookupPrivate(&(pScreen)->devPrivates, RACScreenKey))->x
#define SCREEN_EPILOG(x,y) pScreen->x = y;

#define WRAP_PICT_COND(x,y,cond) if (ps)\
				{pScreenPriv->x = ps->x;\
					if (flag & (cond))\
					ps->x = y;}
#define UNWRAP_PICT(x) 	if (ps) {ps->x = pScreenPriv->x;}

#define PICTURE_PROLOGUE(field) ps->field = \
	((RACScreenPtr)dixLookupPrivate(&(pScreen)->devPrivates, RACScreenKey))->field
#define PICTURE_EPILOGUE(field, wrap) \
	ps->field = wrap

#define WRAP_SCREEN_INFO(x,y) {pScreenPriv->x = pScrn->x;\
                                   pScrn->x = y;}
#define WRAP_SCREEN_INFO_COND(x,y,cond) \
                              {pScreenPriv->x = pScrn->x;\
	                          if (flag & (cond))\
                                     pScrn->x = y;}
#define UNWRAP_SCREEN_INFO(x)    pScrn->x = pScreenPriv->x

#define SPRITE_PROLOG     miPointerScreenPtr PointPriv = \
    (miPointerScreenPtr)dixLookupPrivate(&pScreen->devPrivates, miPointerScreenKey); \
    RACScreenPtr pScreenPriv = \
    ((RACScreenPtr)dixLookupPrivate(&(pScreen)->devPrivates, RACScreenKey));\
			PointPriv->spriteFuncs = pScreenPriv->miSprite;
#define SPRITE_EPILOG pScreenPriv->miSprite = PointPriv->spriteFuncs;\
	              PointPriv->spriteFuncs  = &RACSpriteFuncs;
#define WRAP_SPRITE_COND(cond){pScreenPriv->miSprite = PointPriv->spriteFuncs;\
	                      if(flag & (cond))\
	                      PointPriv->spriteFuncs  = &RACSpriteFuncs;}
#define UNWRAP_SPRITE PointPriv->spriteFuncs = pScreenPriv->miSprite
    
	    
#define GC_WRAP(x) pGCPriv->wrapOps = (x)->ops;\
		 pGCPriv->wrapFuncs = (x)->funcs;\
                           (x)->ops = &RACGCOps;\
                         (x)->funcs = &RACGCFuncs;
#define GC_UNWRAP(x)\
    RACGCPtr  pGCPriv = (RACGCPtr)dixLookupPrivate(&(x)->devPrivates, RACGCKey);\
                    (x)->ops = pGCPriv->wrapOps;\
	          (x)->funcs = pGCPriv->wrapFuncs;

#define GC_SCREEN register ScrnInfoPtr pScrn \
                           = xf86Screens[pGC->pScreen->myNum]

#define ENABLE xf86EnableAccess(xf86Screens[pScreen->myNum])
#define ENABLE_GC  xf86EnableAccess(xf86Screens[pGC->pScreen->myNum])

typedef struct _RACScreen {
    CreateGCProcPtr 		CreateGC;
    CloseScreenProcPtr 		CloseScreen;
    GetImageProcPtr 		GetImage;
    GetSpansProcPtr 		GetSpans;
    SourceValidateProcPtr 	SourceValidate;
    CopyWindowProcPtr 		CopyWindow;
    ClearToBackgroundProcPtr 	ClearToBackground;
    CreatePixmapProcPtr         CreatePixmap;
    SaveScreenProcPtr           SaveScreen;
    /* Colormap */
    StoreColorsProcPtr          StoreColors;
    /* Cursor */
    DisplayCursorProcPtr         DisplayCursor;
    RealizeCursorProcPtr         RealizeCursor;
    UnrealizeCursorProcPtr       UnrealizeCursor;
    RecolorCursorProcPtr         RecolorCursor;
    SetCursorPositionProcPtr     SetCursorPosition;
    void                         (*AdjustFrame)(int,int,int,int);
    Bool                         (*SwitchMode)(int, DisplayModePtr,int);
    Bool                         (*EnterVT)(int, int);
    void                         (*LeaveVT)(int, int);
    void                         (*FreeScreen)(int, int);
    miPointerSpriteFuncPtr       miSprite;
#ifdef RENDER
    CompositeProcPtr			Composite;
    GlyphsProcPtr			Glyphs;
    CompositeRectsProcPtr		CompositeRects;
#endif
} RACScreenRec, *RACScreenPtr;

typedef struct _RACGC {
    GCOps 	*wrapOps;
    GCFuncs 	*wrapFuncs;
} RACGCRec, *RACGCPtr;

/* Screen funcs */		     
static Bool RACCloseScreen (int i, ScreenPtr pScreen);
static void RACGetImage (DrawablePtr pDrawable, int sx, int sy,
			 int w, int h, unsigned int format,
			 unsigned long planemask, char *pdstLine);
static void RACGetSpans (DrawablePtr pDrawable, int wMax, DDXPointPtr	ppt,
			 int *pwidth, int nspans, char	*pdstStart);
static void RACSourceValidate (DrawablePtr	pDrawable,
			       int x, int y, int width, int height );
static void RACCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg,
			  RegionPtr prgnSrc );
static void RACClearToBackground (WindowPtr pWin, int x, int y,
				  int w, int h, Bool generateExposures );
static PixmapPtr RACCreatePixmap(ScreenPtr pScreen, int w, int h, int depth,
				 unsigned usage_hint);
static Bool  RACCreateGC(GCPtr pGC);
static Bool RACSaveScreen(ScreenPtr pScreen, Bool unblank);
static void RACStoreColors (ColormapPtr pmap, int ndef, xColorItem *pdefs);
static void RACRecolorCursor (DeviceIntPtr pDev, ScreenPtr pScreen, 
                              CursorPtr pCurs, Bool displayed);
static Bool RACRealizeCursor (DeviceIntPtr pDev, ScreenPtr   pScreen, 
                              CursorPtr pCursor);
static Bool RACUnrealizeCursor (DeviceIntPtr pDev, ScreenPtr   pScreen, 
                                CursorPtr pCursor);
static Bool RACDisplayCursor (DeviceIntPtr pDev, ScreenPtr pScreen, 
                              CursorPtr pCursor);
static Bool RACSetCursorPosition (DeviceIntPtr pDev, ScreenPtr   pScreen, 
                                  int x, int y, Bool generateEvent);
static void RACAdjustFrame(int index, int x, int y, int flags);
static Bool RACSwitchMode(int index, DisplayModePtr mode, int flags);
static Bool RACEnterVT(int index, int flags);
static void RACLeaveVT(int index, int flags);
static void RACFreeScreen(int index, int flags);
/* GC funcs */
static void RACValidateGC(GCPtr pGC, unsigned long changes, DrawablePtr pDraw);
static void RACChangeGC(GCPtr pGC, unsigned long mask);
static void RACCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst);
static void RACDestroyGC(GCPtr pGC);
static void RACChangeClip(GCPtr pGC, int type, pointer pvalue, int nrects);
static void RACDestroyClip(GCPtr pGC);
static void RACCopyClip(GCPtr pgcDst, GCPtr pgcSrc);
/* GC ops */
static void RACFillSpans( DrawablePtr pDraw, GC *pGC, int nInit,	
			  DDXPointPtr pptInit, int *pwidthInit, int fSorted );
static void RACSetSpans(DrawablePtr pDraw, GCPtr pGC, char *pcharsrc,
			register DDXPointPtr ppt, int *pwidth, int nspans,
			int fSorted );
static void RACPutImage(DrawablePtr pDraw, GCPtr pGC, int depth, 
			int x, int y, int w, int h, int	leftPad,
			int format, char *pImage );
static RegionPtr RACCopyArea(DrawablePtr pSrc, DrawablePtr pDst,
			     GC *pGC, int srcx, int srcy,
			     int width, int height,
			     int dstx, int dsty );
static RegionPtr RACCopyPlane(DrawablePtr pSrc, DrawablePtr pDst,
			      GCPtr pGC, int srcx, int srcy,
			      int width, int height, int dstx, int dsty,
			      unsigned long bitPlane );
static void RACPolyPoint(DrawablePtr pDraw, GCPtr pGC, int mode,
			 int npt, xPoint *pptInit );
static void RACPolylines(DrawablePtr pDraw, GCPtr pGC, int mode,
			 int npt, DDXPointPtr pptInit );
static void RACPolySegment(DrawablePtr	pDraw, GCPtr pGC, int nseg,
			   xSegment	*pSeg );
static void RACPolyRectangle(DrawablePtr  pDraw, GCPtr pGC, int  nRectsInit,
			     xRectangle  *pRectsInit );
static void RACPolyArc(DrawablePtr pDraw, GCPtr	pGC, int narcs,
		       xArc *parcs );
static void RACFillPolygon(DrawablePtr	pDraw, GCPtr pGC, int shape, int mode,
			   int count, DDXPointPtr ptsIn );
static void RACPolyFillRect( DrawablePtr pDraw, GCPtr	pGC, int nrectFill, 
			     xRectangle	*prectInit );
static void RACPolyFillArc(DrawablePtr	pDraw, GCPtr pGC, int	narcs,
			   xArc	*parcs );
static int RACPolyText8(DrawablePtr pDraw, GCPtr pGC, int x, int y,
			int count,  char *chars );
static int RACPolyText16(DrawablePtr pDraw, GCPtr pGC, int x, int y,
			 int count, unsigned short *chars );
static void RACImageText8(DrawablePtr pDraw, GCPtr pGC, int x, int y,
			  int 	count, char *chars );
static void RACImageText16(DrawablePtr pDraw, GCPtr pGC, int x, int y,
			   int 	count, unsigned short *chars );
static void RACImageGlyphBlt(DrawablePtr pDraw, GCPtr pGC, int xInit,
			     int yInit, unsigned int nglyph,
			     CharInfoPtr *ppci, pointer pglyphBase );
static void RACPolyGlyphBlt(DrawablePtr pDraw, GCPtr pGC, int xInit,
			    int yInit, unsigned int nglyph,
			    CharInfoPtr *ppci, pointer pglyphBase );
static void RACPushPixels(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDraw,
			  int dx, int dy, int xOrg, int yOrg );
/* miSpriteFuncs */
static Bool RACSpriteRealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen,
                                   CursorPtr pCur); 
static Bool RACSpriteUnrealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                     CursorPtr pCur);
static void RACSpriteSetCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                               CursorPtr pCur, int x, int y);
static void RACSpriteMoveCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                                int x, int y); 
#ifdef RENDER
static void RACComposite(CARD8 op, PicturePtr pSrc,  PicturePtr pMask,
			 PicturePtr pDst, INT16 xSrc, INT16 ySrc,
			 INT16 xMask, INT16 yMask, INT16 xDst,
			 INT16 yDst, CARD16 width, CARD16 height);
static void RACGlyphs(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
		      PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc,
		      int nlist, GlyphListPtr list, GlyphPtr *glyphs);
static void RACCompositeRects(CARD8 op, PicturePtr pDst, xRenderColor *color,
			      int nRect, xRectangle *rects);
#endif

static GCFuncs RACGCFuncs = {
    RACValidateGC, RACChangeGC, RACCopyGC, RACDestroyGC,
    RACChangeClip, RACDestroyClip, RACCopyClip
};

static GCOps RACGCOps = {
    RACFillSpans, RACSetSpans, RACPutImage, RACCopyArea, 
    RACCopyPlane, RACPolyPoint, RACPolylines, RACPolySegment, 
    RACPolyRectangle, RACPolyArc, RACFillPolygon, RACPolyFillRect, 
    RACPolyFillArc, RACPolyText8, RACPolyText16, RACImageText8, 
    RACImageText16, RACImageGlyphBlt, RACPolyGlyphBlt, RACPushPixels,
    {NULL}		/* devPrivate */
};

static miPointerSpriteFuncRec RACSpriteFuncs = {
    RACSpriteRealizeCursor, RACSpriteUnrealizeCursor, RACSpriteSetCursor,
    RACSpriteMoveCursor
};

static int RACScreenKeyIndex;
static DevPrivateKey RACScreenKey = &RACScreenKeyIndex;
static int RACGCKeyIndex;
static DevPrivateKey RACGCKey = &RACGCKeyIndex;


Bool 
xf86RACInit(ScreenPtr pScreen, unsigned int flag)
{
    ScrnInfoPtr pScrn;
    RACScreenPtr pScreenPriv;
    miPointerScreenPtr PointPriv;
#ifdef RENDER
    PictureScreenPtr	ps = GetPictureScreenIfSet(pScreen);
#endif

    pScrn = xf86Screens[pScreen->myNum];
    PointPriv = (miPointerScreenPtr)dixLookupPrivate(&pScreen->devPrivates,
						     miPointerScreenKey);
    DPRINT_S("RACInit",pScreen->myNum);

    if (!dixRequestPrivate(RACGCKey, sizeof(RACGCRec)))
	return FALSE;

    if (!(pScreenPriv = xalloc(sizeof(RACScreenRec))))
	return FALSE;

    dixSetPrivate(&pScreen->devPrivates, RACScreenKey, pScreenPriv);
    
    WRAP_SCREEN(CloseScreen, RACCloseScreen);
    WRAP_SCREEN(SaveScreen, RACSaveScreen);
    WRAP_SCREEN_COND(CreateGC, RACCreateGC, RAC_FB);
    WRAP_SCREEN_COND(GetImage, RACGetImage, RAC_FB);
    WRAP_SCREEN_COND(GetSpans, RACGetSpans, RAC_FB);
    WRAP_SCREEN_COND(SourceValidate, RACSourceValidate, RAC_FB);
    WRAP_SCREEN_COND(CopyWindow, RACCopyWindow, RAC_FB);
    WRAP_SCREEN_COND(ClearToBackground, RACClearToBackground, RAC_FB);
    WRAP_SCREEN_COND(CreatePixmap, RACCreatePixmap, RAC_FB);
    WRAP_SCREEN_COND(StoreColors, RACStoreColors, RAC_COLORMAP);
    WRAP_SCREEN_COND(DisplayCursor, RACDisplayCursor, RAC_CURSOR);
    WRAP_SCREEN_COND(RealizeCursor, RACRealizeCursor, RAC_CURSOR);
    WRAP_SCREEN_COND(UnrealizeCursor, RACUnrealizeCursor, RAC_CURSOR);
    WRAP_SCREEN_COND(RecolorCursor, RACRecolorCursor, RAC_CURSOR);
    WRAP_SCREEN_COND(SetCursorPosition, RACSetCursorPosition, RAC_CURSOR);
#ifdef RENDER
    WRAP_PICT_COND(Composite,RACComposite,RAC_FB);
    WRAP_PICT_COND(Glyphs,RACGlyphs,RAC_FB);    
    WRAP_PICT_COND(CompositeRects,RACCompositeRects,RAC_FB);    
#endif
    WRAP_SCREEN_INFO_COND(AdjustFrame, RACAdjustFrame, RAC_VIEWPORT);
    WRAP_SCREEN_INFO(SwitchMode, RACSwitchMode);
    WRAP_SCREEN_INFO(EnterVT, RACEnterVT);
    WRAP_SCREEN_INFO(LeaveVT, RACLeaveVT);
    WRAP_SCREEN_INFO(FreeScreen, RACFreeScreen);
    WRAP_SPRITE_COND(RAC_CURSOR);

    return TRUE;
}

/* Screen funcs */
static Bool
RACCloseScreen (int i, ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RACScreenPtr pScreenPriv = (RACScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, RACScreenKey);
    miPointerScreenPtr PointPriv = (miPointerScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, miPointerScreenKey);
#ifdef RENDER
    PictureScreenPtr	ps = GetPictureScreenIfSet(pScreen);
#endif

    DPRINT_S("RACCloseScreen",pScreen->myNum);
    UNWRAP_SCREEN(CreateGC);
    UNWRAP_SCREEN(CloseScreen);
    UNWRAP_SCREEN(GetImage);
    UNWRAP_SCREEN(GetSpans);
    UNWRAP_SCREEN(SourceValidate);
    UNWRAP_SCREEN(CopyWindow);
    UNWRAP_SCREEN(ClearToBackground);
    UNWRAP_SCREEN(SaveScreen);
    UNWRAP_SCREEN(StoreColors);
    UNWRAP_SCREEN(DisplayCursor);
    UNWRAP_SCREEN(RealizeCursor);
    UNWRAP_SCREEN(UnrealizeCursor);
    UNWRAP_SCREEN(RecolorCursor);
    UNWRAP_SCREEN(SetCursorPosition);
#ifdef RENDER
    UNWRAP_PICT(Composite);
    UNWRAP_PICT(Glyphs);    
    UNWRAP_PICT(CompositeRects);    
#endif
    UNWRAP_SCREEN_INFO(AdjustFrame);
    UNWRAP_SCREEN_INFO(SwitchMode);
    UNWRAP_SCREEN_INFO(EnterVT);
    UNWRAP_SCREEN_INFO(LeaveVT);
    UNWRAP_SCREEN_INFO(FreeScreen);
    UNWRAP_SPRITE;
        
    xfree ((pointer) pScreenPriv);

    if (xf86Screens[pScreen->myNum]->vtSema) {
	xf86EnterServerState(SETUP);
	ENABLE;
    }
    return (*pScreen->CloseScreen) (i, pScreen);
}

static void
RACGetImage (
    DrawablePtr pDrawable,
    int	sx, int sy, int w, int h,
    unsigned int    format,
    unsigned long   planemask,
    char	    *pdstLine 
    )
{
    ScreenPtr pScreen = pDrawable->pScreen;
    DPRINT_S("RACGetImage",pScreen->myNum);
    SCREEN_PROLOG(GetImage);
    if (xf86Screens[pScreen->myNum]->vtSema) {
	ENABLE;
    }
    (*pScreen->GetImage) (pDrawable, sx, sy, w, h,
			  format, planemask, pdstLine);
    SCREEN_EPILOG (GetImage, RACGetImage);
}

static void
RACGetSpans (
    DrawablePtr	pDrawable,
    int		wMax,
    DDXPointPtr	ppt,
    int		*pwidth,
    int		nspans,
    char	*pdstStart
    )
{
    ScreenPtr	    pScreen = pDrawable->pScreen;

    DPRINT_S("RACGetSpans",pScreen->myNum);
    SCREEN_PROLOG (GetSpans);
    ENABLE;
    (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
    SCREEN_EPILOG (GetSpans, RACGetSpans);
}

static void
RACSourceValidate (
    DrawablePtr	pDrawable,
    int	x, int y, int width, int height )
{
    ScreenPtr	pScreen = pDrawable->pScreen;
    DPRINT_S("RACSourceValidate",pScreen->myNum);
    SCREEN_PROLOG (SourceValidate);
    ENABLE;
    if (pScreen->SourceValidate)
	(*pScreen->SourceValidate) (pDrawable, x, y, width, height);
    SCREEN_EPILOG (SourceValidate, RACSourceValidate);
}

static void
RACCopyWindow(
    WindowPtr pWin,
    DDXPointRec ptOldOrg,
    RegionPtr prgnSrc )
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    DPRINT_S("RACCopyWindow",pScreen->myNum);
    SCREEN_PROLOG (CopyWindow);
    ENABLE;
    (*pScreen->CopyWindow) (pWin, ptOldOrg, prgnSrc);
    SCREEN_EPILOG (CopyWindow, RACCopyWindow);
}

static void
RACClearToBackground (
    WindowPtr pWin,
    int x, int y,
    int w, int h,
    Bool generateExposures )
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    DPRINT_S("RACClearToBackground",pScreen->myNum);
    SCREEN_PROLOG ( ClearToBackground);
    ENABLE;
    (*pScreen->ClearToBackground) (pWin, x, y, w, h, generateExposures);
    SCREEN_EPILOG (ClearToBackground, RACClearToBackground);
}

static PixmapPtr 
RACCreatePixmap(ScreenPtr pScreen, int w, int h, int depth, unsigned usage_hint)
{
    PixmapPtr pPix;

    DPRINT_S("RACCreatePixmap",pScreen->myNum);
    SCREEN_PROLOG ( CreatePixmap);
    ENABLE;
    pPix = (*pScreen->CreatePixmap) (pScreen, w, h, depth, usage_hint);
    SCREEN_EPILOG (CreatePixmap, RACCreatePixmap);

    return pPix;
}

static Bool 
RACSaveScreen(ScreenPtr pScreen, Bool unblank)
{
    Bool val;

    DPRINT_S("RACSaveScreen",pScreen->myNum);
    SCREEN_PROLOG (SaveScreen);
    ENABLE;
    val = (*pScreen->SaveScreen) (pScreen, unblank);
    SCREEN_EPILOG (SaveScreen, RACSaveScreen);

    return val;
}

static void
RACStoreColors (
    ColormapPtr        pmap,
    int                ndef,
    xColorItem         *pdefs)
{
    ScreenPtr pScreen = pmap->pScreen;

    DPRINT_S("RACStoreColors",pScreen->myNum);
    SCREEN_PROLOG (StoreColors);
    ENABLE;
    (*pScreen->StoreColors) (pmap,ndef,pdefs);

    SCREEN_EPILOG ( StoreColors, RACStoreColors);
}

static void
RACRecolorCursor (    
    DeviceIntPtr pDev,
    ScreenPtr pScreen,
    CursorPtr pCurs,
    Bool displayed
    )
{
    DPRINT_S("RACRecolorCursor",pScreen->myNum);
    SCREEN_PROLOG (RecolorCursor);
    ENABLE;
    (*pScreen->RecolorCursor) (pDev, pScreen,pCurs,displayed);
    
    SCREEN_EPILOG ( RecolorCursor, RACRecolorCursor);
}

static Bool
RACRealizeCursor (    
    DeviceIntPtr pDev,
    ScreenPtr    pScreen,
    CursorPtr    pCursor
    )
{
    Bool val;

    DPRINT_S("RACRealizeCursor",pScreen->myNum);
    SCREEN_PROLOG (RealizeCursor);
    ENABLE;
    val = (*pScreen->RealizeCursor) (pDev, pScreen,pCursor);
    
    SCREEN_EPILOG ( RealizeCursor, RACRealizeCursor);
    return val;
}

static Bool
RACUnrealizeCursor (    
    DeviceIntPtr pDev,
    ScreenPtr    pScreen,
    CursorPtr    pCursor
    )
{
    Bool val;

    DPRINT_S("RACUnrealizeCursor",pScreen->myNum);
    SCREEN_PROLOG (UnrealizeCursor);
    ENABLE;
    val = (*pScreen->UnrealizeCursor) (pDev, pScreen,pCursor);
    
    SCREEN_EPILOG ( UnrealizeCursor, RACUnrealizeCursor);
    return val;
}

static Bool
RACDisplayCursor (    
    DeviceIntPtr pDev,
    ScreenPtr    pScreen,
    CursorPtr    pCursor
    )
{
    Bool val;

    DPRINT_S("RACDisplayCursor",pScreen->myNum);
    SCREEN_PROLOG (DisplayCursor);
    ENABLE;
    val = (*pScreen->DisplayCursor) (pDev, pScreen,pCursor);
    
    SCREEN_EPILOG ( DisplayCursor, RACDisplayCursor);
    return val;
}

static Bool
RACSetCursorPosition (    
    DeviceIntPtr pDev,
    ScreenPtr   pScreen,
    int x, int y,
    Bool generateEvent)
{
    Bool val;

    DPRINT_S("RACSetCursorPosition",pScreen->myNum);
    SCREEN_PROLOG (SetCursorPosition);
    ENABLE;
    val = (*pScreen->SetCursorPosition) (pDev, pScreen,x,y,generateEvent);
    
    SCREEN_EPILOG ( SetCursorPosition, RACSetCursorPosition);
    return val;
}

static void
RACAdjustFrame(int index, int x, int y, int flags)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    RACScreenPtr pScreenPriv = (RACScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, RACScreenKey);

    DPRINT_S("RACAdjustFrame",index);
    xf86EnableAccess(xf86Screens[index]);

    (*pScreenPriv->AdjustFrame)(index, x, y, flags);
}

static Bool
RACSwitchMode(int index, DisplayModePtr mode, int flags)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    RACScreenPtr pScreenPriv = (RACScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, RACScreenKey);

    DPRINT_S("RACSwitchMode",index);
    xf86EnableAccess(xf86Screens[index]);

    return (*pScreenPriv->SwitchMode)(index, mode, flags);
}

static Bool
RACEnterVT(int index, int flags)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    RACScreenPtr pScreenPriv = (RACScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, RACScreenKey);

    DPRINT_S("RACEnterVT",index);
    xf86EnableAccess(xf86Screens[index]);

    return (*pScreenPriv->EnterVT)(index, flags);
}

static void
RACLeaveVT(int index, int flags)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    RACScreenPtr pScreenPriv = (RACScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, RACScreenKey);

    DPRINT_S("RACLeaveVT",index);
    xf86EnableAccess(xf86Screens[index]);

    (*pScreenPriv->LeaveVT)(index, flags);
}

static void
RACFreeScreen(int index, int flags)
{
    ScreenPtr pScreen = screenInfo.screens[index];
    RACScreenPtr pScreenPriv = (RACScreenPtr)dixLookupPrivate(
	&pScreen->devPrivates, RACScreenKey);

    DPRINT_S("RACFreeScreen",index);
    xf86EnableAccess(xf86Screens[index]);

    (*pScreenPriv->FreeScreen)(index, flags);
}

static Bool
RACCreateGC(GCPtr pGC)
{
    ScreenPtr    pScreen = pGC->pScreen;
    RACGCPtr pGCPriv = (RACGCPtr)dixLookupPrivate(&pGC->devPrivates, RACGCKey);
    Bool         ret;

    DPRINT_S("RACCreateGC",pScreen->myNum);
    SCREEN_PROLOG(CreateGC);

    ret = (*pScreen->CreateGC)(pGC);
	
    GC_WRAP(pGC);
    SCREEN_EPILOG(CreateGC,RACCreateGC);

    return ret;
}

/* GC funcs */
static void
RACValidateGC(
   GCPtr         pGC,
   unsigned long changes,
   DrawablePtr   pDraw )
{
    GC_UNWRAP(pGC);
    DPRINT("RACValidateGC");
    (*pGC->funcs->ValidateGC)(pGC, changes, pDraw);
    GC_WRAP(pGC);
}


static void
RACDestroyGC(GCPtr pGC)
{
    GC_UNWRAP (pGC);
    DPRINT("RACDestroyGC");
    (*pGC->funcs->DestroyGC)(pGC);
    GC_WRAP (pGC);
}

static void
RACChangeGC (
    GCPtr	    pGC,
    unsigned long   mask)
{
    GC_UNWRAP (pGC);
    DPRINT("RACChangeGC");
    (*pGC->funcs->ChangeGC) (pGC, mask);
    GC_WRAP (pGC);
}

static void
RACCopyGC (
    GCPtr	    pGCSrc, 
    unsigned long   mask,
    GCPtr	    pGCDst)
{
    GC_UNWRAP (pGCDst);
    DPRINT("RACCopyGC");
    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    GC_WRAP (pGCDst);
}

static void
RACChangeClip (
    GCPtr   pGC,
    int		type,
    pointer	pvalue,
    int		nrects )
{
    GC_UNWRAP (pGC);
    DPRINT("RACChangeClip");
    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);
    GC_WRAP (pGC);
}

static void
RACCopyClip(GCPtr pgcDst, GCPtr pgcSrc)
{
    GC_UNWRAP (pgcDst);
    DPRINT("RACCopyClip");
    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);
    GC_WRAP (pgcDst);
}

static void
RACDestroyClip(GCPtr pGC)
{
    GC_UNWRAP (pGC);
    DPRINT("RACDestroyClip");
    (* pGC->funcs->DestroyClip)(pGC);
    GC_WRAP (pGC);
}

/* GC Ops */
static void
RACFillSpans(
    DrawablePtr pDraw,
    GC		*pGC,
    int		nInit,	
    DDXPointPtr pptInit,	
    int *pwidthInit,		
    int fSorted )
{
    GC_UNWRAP(pGC);
    DPRINT("RACFillSpans");
    ENABLE_GC;
    (*pGC->ops->FillSpans)(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted);
    GC_WRAP(pGC);
}

static void
RACSetSpans(
    DrawablePtr		pDraw,
    GCPtr		pGC,
    char		*pcharsrc,
    register DDXPointPtr ppt,
    int			*pwidth,
    int			nspans,
    int			fSorted )
{
    GC_UNWRAP(pGC);
    DPRINT("RACSetSpans");
    ENABLE_GC;
    (*pGC->ops->SetSpans)(pDraw, pGC, pcharsrc, ppt, pwidth, nspans, fSorted);
    GC_WRAP(pGC);
}

static void
RACPutImage(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		depth, 
    int x, int y, int w, int h,
    int		leftPad,
    int		format,
    char 	*pImage )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPutImage");
    ENABLE_GC;
    (*pGC->ops->PutImage)(pDraw, pGC, depth, x, y, w, h, 
			  leftPad, format, pImage);
    GC_WRAP(pGC);
}

static RegionPtr
RACCopyArea(
    DrawablePtr pSrc,
    DrawablePtr pDst,
    GC *pGC,
    int srcx, int srcy,
    int width, int height,
    int dstx, int dsty )
{
    RegionPtr ret;

    GC_UNWRAP(pGC);
    DPRINT("RACCopyArea");
    ENABLE_GC;
    ret = (*pGC->ops->CopyArea)(pSrc, pDst,
				pGC, srcx, srcy, width, height, dstx, dsty);
    GC_WRAP(pGC);
    return ret;
}

static RegionPtr
RACCopyPlane(
    DrawablePtr	pSrc,
    DrawablePtr	pDst,
    GCPtr pGC,
    int	srcx, int srcy,
    int	width, int height,
    int	dstx, int dsty,
    unsigned long bitPlane )
{
    RegionPtr ret;

    GC_UNWRAP(pGC);
    DPRINT("RACCopyPlane");
    ENABLE_GC;
    ret = (*pGC->ops->CopyPlane)(pSrc, pDst, pGC, srcx, srcy,
				 width, height, dstx, dsty, bitPlane);
    GC_WRAP(pGC);
    return ret;
}

static void
RACPolyPoint(
    DrawablePtr pDraw,
    GCPtr pGC,
    int mode,
    int npt,
    xPoint *pptInit )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPolyPoint");
    ENABLE_GC;
    (*pGC->ops->PolyPoint)(pDraw, pGC, mode, npt, pptInit);
    GC_WRAP(pGC);
}


static void
RACPolylines(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		mode,		
    int		npt,		
    DDXPointPtr pptInit )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPolylines");
    ENABLE_GC;
    (*pGC->ops->Polylines)(pDraw, pGC, mode, npt, pptInit);
    GC_WRAP(pGC);
}

static void 
RACPolySegment(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nseg,
    xSegment	*pSeg )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPolySegment");
    ENABLE_GC;
    (*pGC->ops->PolySegment)(pDraw, pGC, nseg, pSeg);
    GC_WRAP(pGC);
}

static void
RACPolyRectangle(
    DrawablePtr  pDraw,
    GCPtr        pGC,
    int	         nRectsInit,
    xRectangle  *pRectsInit )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPolyRectangle");
    ENABLE_GC;
    (*pGC->ops->PolyRectangle)(pDraw, pGC, nRectsInit, pRectsInit);
    GC_WRAP(pGC);
}

static void
RACPolyArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPolyArc");
    ENABLE_GC;
    (*pGC->ops->PolyArc)(pDraw, pGC, narcs, parcs);
    GC_WRAP(pGC);
}

static void
RACFillPolygon(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		shape,
    int		mode,
    int		count,
    DDXPointPtr	ptsIn )
{
    GC_UNWRAP(pGC);
    DPRINT("RACFillPolygon");
    ENABLE_GC;
    (*pGC->ops->FillPolygon)(pDraw, pGC, shape, mode, count, ptsIn);
    GC_WRAP(pGC);
}


static void 
RACPolyFillRect(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nrectFill, 
    xRectangle	*prectInit )  
{
    GC_UNWRAP(pGC);
    DPRINT("RACPolyFillRect");
    ENABLE_GC;
    (*pGC->ops->PolyFillRect)(pDraw, pGC, nrectFill, prectInit);
    GC_WRAP(pGC);
}


static void
RACPolyFillArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPolyFillArc");
    ENABLE_GC;
    (*pGC->ops->PolyFillArc)(pDraw, pGC, narcs, parcs);
    GC_WRAP(pGC);
}

static int
RACPolyText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int 	y,
    int 	count,
    char	*chars )
{
    int ret;

    GC_UNWRAP(pGC);
    DPRINT("RACPolyText8");
    ENABLE_GC;
    ret = (*pGC->ops->PolyText8)(pDraw, pGC, x, y, count, chars);
    GC_WRAP(pGC);
    return ret;
}

static int
RACPolyText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars )
{
    int ret;

    GC_UNWRAP(pGC);
    DPRINT("RACPolyText16");
    ENABLE_GC;
    ret = (*pGC->ops->PolyText16)(pDraw, pGC, x, y, count, chars);
    GC_WRAP(pGC);
    return ret;
}

static void
RACImageText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int		y,
    int 	count,
    char	*chars )
{
    GC_UNWRAP(pGC);
    DPRINT("RACImageText8");
    ENABLE_GC;
    (*pGC->ops->ImageText8)(pDraw, pGC, x, y, count, chars);
    GC_WRAP(pGC);
}

static void
RACImageText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars )
{
    GC_UNWRAP(pGC);
    DPRINT("RACImageText16");
    ENABLE_GC;
    (*pGC->ops->ImageText16)(pDraw, pGC, x, y, count, chars);
    GC_WRAP(pGC);
}


static void
RACImageGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int xInit, int yInit,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase )
{
    GC_UNWRAP(pGC);
    DPRINT("RACImageGlyphBlt");
    ENABLE_GC;
    (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, xInit, yInit,
			       nglyph, ppci, pglyphBase);
    GC_WRAP(pGC);
}

static void
RACPolyGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int xInit, int yInit,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPolyGlyphBlt");
    ENABLE_GC;
    (*pGC->ops->PolyGlyphBlt)(pDraw, pGC, xInit, yInit,
			      nglyph, ppci, pglyphBase);
    GC_WRAP(pGC);
}

static void
RACPushPixels(
    GCPtr	pGC,
    PixmapPtr	pBitMap,
    DrawablePtr pDraw,
    int	dx, int dy, int xOrg, int yOrg )
{
    GC_UNWRAP(pGC);
    DPRINT("RACPushPixels");
    ENABLE_GC;
    (*pGC->ops->PushPixels)(pGC, pBitMap, pDraw, dx, dy, xOrg, yOrg);
    GC_WRAP(pGC);
}


/* miSpriteFuncs */
static Bool
RACSpriteRealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCur)
{
    Bool val;
    SPRITE_PROLOG;
    DPRINT_S("RACSpriteRealizeCursor",pScreen->myNum);
    ENABLE;
    val = PointPriv->spriteFuncs->RealizeCursor(pDev, pScreen, pCur);
    SPRITE_EPILOG;
    return val;
}

static Bool
RACSpriteUnrealizeCursor(DeviceIntPtr pDev, ScreenPtr pScreen, CursorPtr pCur)
{
    Bool val;
    SPRITE_PROLOG;
    DPRINT_S("RACSpriteUnrealizeCursor",pScreen->myNum);
    ENABLE;
    val = PointPriv->spriteFuncs->UnrealizeCursor(pDev, pScreen, pCur);
    SPRITE_EPILOG;
    return val;
}

static void
RACSpriteSetCursor(DeviceIntPtr pDev, ScreenPtr pScreen, 
                   CursorPtr pCur, int x, int y) 
{
    SPRITE_PROLOG;
    DPRINT_S("RACSpriteSetCursor",pScreen->myNum);
    ENABLE;
    PointPriv->spriteFuncs->SetCursor(pDev, pScreen, pCur, x, y);
    SPRITE_EPILOG;
}

static void
RACSpriteMoveCursor(DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y)
{
    SPRITE_PROLOG;
    DPRINT_S("RACSpriteMoveCursor",pScreen->myNum);
    ENABLE;
    PointPriv->spriteFuncs->MoveCursor(pDev, pScreen, x, y);
    SPRITE_EPILOG;
}

#ifdef RENDER
static void
RACComposite(CARD8 op, PicturePtr pSrc, PicturePtr pMask,
	     PicturePtr pDst, INT16 xSrc, INT16 ySrc, INT16 xMask,
	     INT16 yMask, INT16 xDst, INT16 yDst, CARD16 width,
	     CARD16 height)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);

    PICTURE_PROLOGUE(Composite);

    ENABLE;
    (*ps->Composite) (op, pSrc, pMask, pDst, xSrc, ySrc, xMask, yMask, xDst,
		      yDst, width, height);
    
    PICTURE_EPILOGUE(Composite, RACComposite);
}

static void
RACGlyphs(CARD8 op, PicturePtr pSrc, PicturePtr pDst,
	  PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int nlist,
	  GlyphListPtr list, GlyphPtr *glyphs)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);

    PICTURE_PROLOGUE(Glyphs);

    ENABLE;
    (*ps->Glyphs)(op, pSrc, pDst, maskFormat, xSrc, ySrc, nlist, list, glyphs);
    
    PICTURE_EPILOGUE (Glyphs, RACGlyphs);
}

static void
RACCompositeRects(CARD8 op, PicturePtr pDst, xRenderColor *color, int nRect,
		  xRectangle *rects)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);

    PICTURE_PROLOGUE(CompositeRects);

    ENABLE;
    (*ps->CompositeRects)(op, pDst, color, nRect, rects);
    
    PICTURE_EPILOGUE (CompositeRects, RACCompositeRects);
}
#endif

