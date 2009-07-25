#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include "scrnintstr.h"
#include "gcstruct.h"
#include "glyphstr.h"
#include "window.h"
#include "windowstr.h"
#include "picture.h"
#include "picturestr.h"
#include "colormapst.h"
#include "xaa.h"
#include "xaalocal.h"
#include "xaaWrapper.h"

void XAASync(ScreenPtr pScreen);

/* #include "render.h" */

#if 1
#define COND(pDraw) \
        ((pDraw)->depth \
                      != (xaaWrapperGetScrPriv(((DrawablePtr)(pDraw))->pScreen))->depth)
#else
#define COND(pDraw) 1
#endif

static Bool xaaWrapperCreateGC(GCPtr pGC);
static void xaaWrapperValidateGC(GCPtr pGC, unsigned long changes, DrawablePtr pDraw);
static void xaaWrapperDestroyGC(GCPtr pGC);
static void xaaWrapperChangeGC (GCPtr pGC, unsigned long   mask);
static void xaaWrapperCopyGC (GCPtr pGCSrc, unsigned long   mask, GCPtr pGCDst);
static void xaaWrapperChangeClip (GCPtr pGC, int type, pointer pvalue, int nrects);

static void xaaWrapperCopyClip(GCPtr pgcDst, GCPtr pgcSrc);
static void xaaWrapperDestroyClip(GCPtr pGC);


static void
xaaWrapperComposite (CARD8 op, PicturePtr pSrc, PicturePtr pMask, PicturePtr pDst,
	     INT16 xSrc, INT16 ySrc, INT16 xMask, INT16 yMask,
	     INT16 xDst, INT16 yDst, CARD16 width, CARD16 height);
static void
xaaWrapperGlyphs (CARD8 op, PicturePtr pSrc, PicturePtr pDst,
	  PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int nlist,
	  GlyphListPtr list, GlyphPtr *glyphs);


typedef struct {
    CloseScreenProcPtr		CloseScreen;
    CreateScreenResourcesProcPtr CreateScreenResources;
    CreateWindowProcPtr		CreateWindow;
    CopyWindowProcPtr		CopyWindow;
    WindowExposuresProcPtr	WindowExposures;
    CreateGCProcPtr		CreateGC;
    CreateColormapProcPtr	CreateColormap;
    DestroyColormapProcPtr	DestroyColormap;
    InstallColormapProcPtr	InstallColormap;
    UninstallColormapProcPtr	UninstallColormap;
    ListInstalledColormapsProcPtr ListInstalledColormaps;
    StoreColorsProcPtr		StoreColors;
#ifdef RENDER
    CompositeProcPtr		Composite;
    GlyphsProcPtr		Glyphs;
#endif    

    CloseScreenProcPtr		wrapCloseScreen;
    CreateScreenResourcesProcPtr wrapCreateScreenResources;
    CreateWindowProcPtr		wrapCreateWindow;
    CopyWindowProcPtr		wrapCopyWindow;
    WindowExposuresProcPtr	wrapWindowExposures;
    CreateGCProcPtr		wrapCreateGC;
    CreateColormapProcPtr	wrapCreateColormap;
    DestroyColormapProcPtr	wrapDestroyColormap;
    InstallColormapProcPtr	wrapInstallColormap;
    UninstallColormapProcPtr	wrapUninstallColormap;
    ListInstalledColormapsProcPtr wrapListInstalledColormaps;
    StoreColorsProcPtr		wrapStoreColors;
#ifdef RENDER
    CompositeProcPtr		wrapComposite;
    GlyphsProcPtr		wrapGlyphs;
#endif    
    int depth;
} xaaWrapperScrPrivRec, *xaaWrapperScrPrivPtr;

#define xaaWrapperGetScrPriv(s)	((xaaWrapperScrPrivPtr) \
    dixLookupPrivate(&(s)->devPrivates, xaaWrapperScrPrivateKey))
#define xaaWrapperScrPriv(s)     xaaWrapperScrPrivPtr pScrPriv = xaaWrapperGetScrPriv(s)

#define wrap(priv,real,mem,func) {\
    priv->mem = real->mem; \
    real->mem = func; \
}

#define unwrap(priv,real,mem) {\
    real->mem = priv->mem; \
}

#define cond_wrap(priv,cond,real,mem,wrapmem,func) {\
    if (COND(cond)) \
        priv->wrapmem = real->mem; \
    else  \
	priv->mem = real->mem; \
    real->mem = func; \
}

#define cond_unwrap(priv,cond,real,mem,wrapmem) {\
    if (COND(cond)) \
        real->mem = priv->wrapmem; \
    else \
        real->mem = priv->mem; \
}

#define get(priv,real,func,wrap) \
    priv->wrap = real->func;

typedef struct _xaaWrapperGCPriv {
    GCOps   *ops;
    Bool  wrap;
    GCFuncs *funcs;
    GCOps   *wrapops;
} xaaWrapperGCPrivRec, *xaaWrapperGCPrivPtr;

#define xaaWrapperGetGCPriv(pGC) ((xaaWrapperGCPrivPtr) \
    dixLookupPrivate(&(pGC)->devPrivates, xaaWrapperGCPrivateKey))
#define xaaWrapperGCPriv(pGC)   xaaWrapperGCPrivPtr  pGCPriv = xaaWrapperGetGCPriv(pGC)


static int xaaWrapperScrPrivateKeyIndex;
static DevPrivateKey xaaWrapperScrPrivateKey = &xaaWrapperScrPrivateKeyIndex;
static int xaaWrapperGCPrivateKeyIndex;
static DevPrivateKey xaaWrapperGCPrivateKey = &xaaWrapperGCPrivateKeyIndex;

static Bool
xaaWrapperCreateScreenResources(ScreenPtr pScreen)
{
    xaaWrapperScrPriv(pScreen);
    Bool		ret;
    
    unwrap (pScrPriv,pScreen, CreateScreenResources);
    ret = pScreen->CreateScreenResources(pScreen);
    wrap(pScrPriv,pScreen,CreateScreenResources,xaaWrapperCreateScreenResources);
    return ret;
}

static Bool
xaaWrapperCloseScreen (int iScreen, ScreenPtr pScreen)
{
    xaaWrapperScrPriv(pScreen);
    Bool		ret;

    unwrap (pScrPriv,pScreen, CloseScreen);
    ret = pScreen->CloseScreen(iScreen,pScreen);
    return TRUE;
}

static Bool
xaaWrapperCreateWindow(WindowPtr pWin)
{
    xaaWrapperScrPriv(pWin->drawable.pScreen);
    Bool ret;
    
    cond_unwrap(pScrPriv, &pWin->drawable, pWin->drawable.pScreen,
		CreateWindow, wrapCreateWindow);
    ret = pWin->drawable.pScreen->CreateWindow(pWin);
    cond_wrap(pScrPriv, &pWin->drawable, pWin->drawable.pScreen, CreateWindow,
	      wrapCreateWindow, xaaWrapperCreateWindow);

    return ret;
}

static void
xaaWrapperCopyWindow(WindowPtr	pWin,
	     DDXPointRec	ptOldOrg,
	     RegionPtr	prgnSrc)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    xaaWrapperScrPriv(pScreen);
    
    unwrap (pScrPriv, pScreen, CopyWindow);
#if 0
    if (COND(&pWin->drawable))
	pWin->drawable.pScreen->CopyWindow = pScrPriv->wrapCopyWindow;
#endif
    pScreen->CopyWindow(pWin, ptOldOrg, prgnSrc);
    wrap(pScrPriv, pScreen, CopyWindow, xaaWrapperCopyWindow);
}

static void
xaaWrapperWindowExposures (WindowPtr	pWin,
			  RegionPtr	prgn,
			  RegionPtr	other_exposed)
{
    xaaWrapperScrPriv(pWin->drawable.pScreen);

    cond_unwrap(pScrPriv, &pWin->drawable, pWin->drawable.pScreen,
		WindowExposures, wrapWindowExposures);
    pWin->drawable.pScreen->WindowExposures(pWin, prgn, other_exposed);
    cond_wrap(pScrPriv, &pWin->drawable, pWin->drawable.pScreen,
	      WindowExposures, wrapWindowExposures, xaaWrapperWindowExposures);
}

static Bool
xaaWrapperCreateColormap(ColormapPtr pmap)
{
    xaaWrapperScrPriv(pmap->pScreen);
    Bool		ret;
    unwrap(pScrPriv,pmap->pScreen, CreateColormap);
    ret = pmap->pScreen->CreateColormap(pmap);
    wrap(pScrPriv,pmap->pScreen,CreateColormap,xaaWrapperCreateColormap);
    
    return ret;
}

static void
xaaWrapperDestroyColormap(ColormapPtr pmap)
{
    xaaWrapperScrPriv(pmap->pScreen);
    unwrap(pScrPriv,pmap->pScreen, DestroyColormap);
    pmap->pScreen->DestroyColormap(pmap);
    wrap(pScrPriv,pmap->pScreen,DestroyColormap,xaaWrapperDestroyColormap);
}

static void
xaaWrapperStoreColors(ColormapPtr pmap, int nColors, xColorItem *pColors)
{
    xaaWrapperScrPriv(pmap->pScreen);
    unwrap(pScrPriv,pmap->pScreen, StoreColors);
    pmap->pScreen->StoreColors(pmap,nColors,pColors);
    wrap(pScrPriv,pmap->pScreen,StoreColors,xaaWrapperStoreColors);
}

static void
xaaWrapperInstallColormap(ColormapPtr pmap)
{
    xaaWrapperScrPriv(pmap->pScreen);
    
    unwrap(pScrPriv,pmap->pScreen, InstallColormap);
    pmap->pScreen->InstallColormap(pmap);
    wrap(pScrPriv,pmap->pScreen,InstallColormap,xaaWrapperInstallColormap);
}

static void
xaaWrapperUninstallColormap(ColormapPtr pmap)
{
    xaaWrapperScrPriv(pmap->pScreen);

    unwrap(pScrPriv,pmap->pScreen, UninstallColormap);
    pmap->pScreen->UninstallColormap(pmap);
    wrap(pScrPriv,pmap->pScreen,UninstallColormap,xaaWrapperUninstallColormap);
}

static int
xaaWrapperListInstalledColormaps(ScreenPtr pScreen, Colormap *pCmapIds)
{
    int			n;
    xaaWrapperScrPriv(pScreen);

    unwrap(pScrPriv,pScreen, ListInstalledColormaps);
    n = pScreen->ListInstalledColormaps(pScreen, pCmapIds);
    wrap (pScrPriv,pScreen,ListInstalledColormaps,xaaWrapperListInstalledColormaps);
    return n;
}

Bool
xaaSetupWrapper(ScreenPtr pScreen, XAAInfoRecPtr infoPtr, int depth, SyncFunc *func)
{
    Bool ret;
    xaaWrapperScrPrivPtr pScrPriv;
#ifdef RENDER
    PictureScreenPtr	ps = GetPictureScreenIfSet(pScreen);
#endif

    if (!dixRequestPrivate(xaaWrapperGCPrivateKey, sizeof(xaaWrapperGCPrivRec)))
	return FALSE;

    pScrPriv = (xaaWrapperScrPrivPtr) xalloc (sizeof (xaaWrapperScrPrivRec));
    if (!pScrPriv)
	return FALSE;

    get (pScrPriv, pScreen, CloseScreen, wrapCloseScreen);
    get (pScrPriv, pScreen, CreateScreenResources, wrapCreateScreenResources);
    get (pScrPriv, pScreen, CreateWindow, wrapCreateWindow);
    get (pScrPriv, pScreen, CopyWindow, wrapCopyWindow);
    get (pScrPriv, pScreen, WindowExposures, wrapWindowExposures);
    get (pScrPriv, pScreen, CreateGC, wrapCreateGC);
    get (pScrPriv, pScreen, CreateColormap, wrapCreateColormap);
    get (pScrPriv, pScreen, DestroyColormap, wrapDestroyColormap);
    get (pScrPriv, pScreen, InstallColormap, wrapInstallColormap);
    get (pScrPriv, pScreen, UninstallColormap, wrapUninstallColormap);
    get (pScrPriv, pScreen, ListInstalledColormaps, wrapListInstalledColormaps);
    get (pScrPriv, pScreen, StoreColors, wrapStoreColors);
#ifdef RENDER
    if (ps) {
	get (pScrPriv, ps, Glyphs, wrapGlyphs);
	get (pScrPriv, ps, Composite, wrapComposite);
    }
#endif
    if (!(ret = XAAInit(pScreen,infoPtr)))
	return FALSE;
    
    wrap (pScrPriv, pScreen, CloseScreen, xaaWrapperCloseScreen);
    wrap (pScrPriv, pScreen, CreateScreenResources,
	  xaaWrapperCreateScreenResources);
    wrap (pScrPriv, pScreen, CreateWindow, xaaWrapperCreateWindow);
    wrap (pScrPriv, pScreen, CopyWindow, xaaWrapperCopyWindow);
    wrap (pScrPriv, pScreen, WindowExposures, xaaWrapperWindowExposures);
    wrap (pScrPriv, pScreen, CreateGC, xaaWrapperCreateGC);
    wrap (pScrPriv, pScreen, CreateColormap, xaaWrapperCreateColormap);
    wrap (pScrPriv, pScreen, DestroyColormap, xaaWrapperDestroyColormap);
    wrap (pScrPriv, pScreen, InstallColormap, xaaWrapperInstallColormap);
    wrap (pScrPriv, pScreen, UninstallColormap, xaaWrapperUninstallColormap);
    wrap (pScrPriv, pScreen, ListInstalledColormaps,
	  xaaWrapperListInstalledColormaps);
    wrap (pScrPriv, pScreen, StoreColors, xaaWrapperStoreColors);

#ifdef RENDER
    if (ps) {
	wrap (pScrPriv, ps, Glyphs, xaaWrapperGlyphs);
	wrap (pScrPriv, ps, Composite, xaaWrapperComposite);
    }
#endif
    pScrPriv->depth = depth;
    dixSetPrivate(&pScreen->devPrivates, xaaWrapperScrPrivateKey, pScrPriv);

    *func = XAASync;
    
    return ret;
}

GCFuncs xaaWrapperGCFuncs = {
    xaaWrapperValidateGC, xaaWrapperChangeGC, xaaWrapperCopyGC,
    xaaWrapperDestroyGC, xaaWrapperChangeClip, xaaWrapperDestroyClip,
    xaaWrapperCopyClip
};

#define XAAWRAPPER_GC_FUNC_PROLOGUE(pGC) \
    xaaWrapperGCPriv(pGC); \
    unwrap(pGCPriv, pGC, funcs); \
    if (pGCPriv->wrap) unwrap(pGCPriv, pGC, ops)

#define XAAWRAPPER_GC_FUNC_EPILOGUE(pGC) \
    wrap(pGCPriv, pGC, funcs, &xaaWrapperGCFuncs);  \
    if (pGCPriv->wrap) wrap(pGCPriv, pGC, ops, pGCPriv->wrapops)

static Bool
xaaWrapperCreateGC(GCPtr pGC)
{
    ScreenPtr		pScreen = pGC->pScreen;
    xaaWrapperScrPriv(pScreen);
    xaaWrapperGCPriv(pGC);
    Bool ret;

    unwrap (pScrPriv, pScreen, CreateGC);
    if((ret = (*pScreen->CreateGC) (pGC))) {
	pGCPriv->wrap = FALSE;
	pGCPriv->funcs = pGC->funcs;
	pGCPriv->wrapops = pGC->ops;
	pGC->funcs = &xaaWrapperGCFuncs;
    }
    wrap (pScrPriv, pScreen, CreateGC, xaaWrapperCreateGC);

    return ret;
}

static void
xaaWrapperValidateGC(
   GCPtr         pGC,
   unsigned long changes,
   DrawablePtr   pDraw 
){
    XAAWRAPPER_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ValidateGC)(pGC, changes, pDraw);

    if(COND(pDraw)) 
	pGCPriv->wrap = TRUE;
    
    XAAWRAPPER_GC_FUNC_EPILOGUE (pGC);
}

static void
xaaWrapperDestroyGC(GCPtr pGC)
{
    XAAWRAPPER_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->DestroyGC)(pGC);
    XAAWRAPPER_GC_FUNC_EPILOGUE (pGC);
}

static void
xaaWrapperChangeGC (
    GCPtr	    pGC,
    unsigned long   mask
){
    XAAWRAPPER_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeGC) (pGC, mask);
    XAAWRAPPER_GC_FUNC_EPILOGUE (pGC);
}

static void
xaaWrapperCopyGC (
    GCPtr	    pGCSrc, 
    unsigned long   mask,
    GCPtr	    pGCDst
){
    XAAWRAPPER_GC_FUNC_PROLOGUE (pGCDst);
    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    XAAWRAPPER_GC_FUNC_EPILOGUE (pGCDst);
}

static void
xaaWrapperChangeClip (
    GCPtr   pGC,
    int		type,
    pointer	pvalue,
    int		nrects 
){
    XAAWRAPPER_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);
    XAAWRAPPER_GC_FUNC_EPILOGUE (pGC);
}

static void
xaaWrapperCopyClip(GCPtr pgcDst, GCPtr pgcSrc)
{
    XAAWRAPPER_GC_FUNC_PROLOGUE (pgcDst);
    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);
    XAAWRAPPER_GC_FUNC_EPILOGUE (pgcDst);
}

static void
xaaWrapperDestroyClip(GCPtr pGC)
{
    XAAWRAPPER_GC_FUNC_PROLOGUE (pGC);
    (* pGC->funcs->DestroyClip)(pGC);
    XAAWRAPPER_GC_FUNC_EPILOGUE (pGC);
}

#ifdef RENDER
static void
xaaWrapperComposite (CARD8 op, PicturePtr pSrc, PicturePtr pMask, PicturePtr pDst,
	     INT16 xSrc, INT16 ySrc, INT16 xMask, INT16 yMask,
    INT16 xDst, INT16 yDst, CARD16 width, CARD16 height)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    xaaWrapperScrPriv(pScreen);

    unwrap (pScrPriv, ps, Composite);
    (*ps->Composite) (op, pSrc, pMask, pDst, xSrc, ySrc, xMask, yMask,
		      xDst, yDst, width, height);
    wrap (pScrPriv, ps, Composite, xaaWrapperComposite);
}


static void
xaaWrapperGlyphs (CARD8 op, PicturePtr pSrc, PicturePtr pDst,
	  PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int nlist,
	  GlyphListPtr list, GlyphPtr *glyphs)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    xaaWrapperScrPriv(pScreen);

    unwrap (pScrPriv, ps, Glyphs);
    (*ps->Glyphs) (op, pSrc, pDst, maskFormat, xSrc, ySrc,
		   nlist, list, glyphs);
    wrap (pScrPriv, ps, Glyphs, xaaWrapperGlyphs);

}
#endif

void
XAASync(ScreenPtr pScreen)
{
    XAAScreenPtr pScreenPriv = (XAAScreenPtr)
	dixLookupPrivate(&pScreen->devPrivates, XAAGetScreenKey());
    XAAInfoRecPtr infoRec = pScreenPriv->AccelInfoRec;

    if(infoRec->NeedToSync) {
        (*infoRec->Sync)(infoRec->pScrn);
        infoRec->NeedToSync = FALSE;
    }
}
