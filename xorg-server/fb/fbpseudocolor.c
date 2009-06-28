#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <string.h>

#include <X11/X.h>
#include <X11/Xproto.h>
#include "scrnintstr.h"
#include "colormapst.h"
#include "glyphstr.h"
#include "resource.h"
#include <X11/fonts/font.h>
#include "dixfontstr.h"
#include <X11/fonts/fontstruct.h>
#include "micmap.h"
#include "fb.h"
#include "fbpseudocolor.h"

static Bool xxCreateGC(GCPtr pGC);
static void xxValidateGC(GCPtr pGC, unsigned long changes, DrawablePtr pDraw);
static void xxDestroyGC(GCPtr pGC);
static void xxChangeGC (GCPtr pGC, unsigned long   mask);
static void xxCopyGC (GCPtr pGCSrc, unsigned long   mask, GCPtr pGCDst);
static void xxChangeClip (GCPtr pGC, int type, pointer pvalue, int nrects);

static void xxCopyClip(GCPtr pgcDst, GCPtr pgcSrc);
static void xxDestroyClip(GCPtr pGC);
static void xxFillSpans(DrawablePtr pDraw, GC *pGC, int nInit,
			DDXPointPtr pptInit, int *pwidthInit, int fSorted);
static void xxSetSpans(DrawablePtr pDraw, GCPtr	pGC, char *pcharsrc,
		       DDXPointPtr pptInit, int	*pwidthInit, int nspans,
		       int fSorted);
static void xxPutImage(DrawablePtr pDraw, GCPtr	pGC, int depth, int x, int y,
		       int w, int h,int	leftPad, int format, char *pImage);
static RegionPtr xxCopyPlane(DrawablePtr pSrc,
			     DrawablePtr pDst, GCPtr pGC,int srcx, int srcy,
			     int width, int height, int	dstx, int dsty,
			     unsigned long bitPlane);
static void xxPolyPoint(DrawablePtr pDraw, GCPtr pGC, int mode, int npt,
			xPoint *pptInit);
static void xxPolylines(DrawablePtr pDraw, GCPtr pGC, int mode,
			int npt, DDXPointPtr pptInit);
static void xxPolySegment(DrawablePtr pDraw, GCPtr pGC, int nseg,
			  xSegment *pSeg);
static void xxPolyRectangle(DrawablePtr  pDraw, GCPtr pGC, int nRects,
			    xRectangle  *pRects);
static void xxPolyArc( DrawablePtr pDraw, GCPtr	pGC, int narcs, xArc *parcs);
static void xxFillPolygon(DrawablePtr pDraw, GCPtr pGC, int shape,
			  int mode, int count, DDXPointPtr pptInit);
static void xxPolyFillRect(DrawablePtr pDraw, GCPtr pGC, int nRectsInit, 
			   xRectangle *pRectsInit);
static RegionPtr xxCopyArea(DrawablePtr pSrc, DrawablePtr pDst, GC *pGC,
			    int srcx, int srcy, int width, int height,
			    int dstx, int dsty);
static void xxPolyFillArc(DrawablePtr pDraw, GCPtr pGC, int narcs,
			  xArc *parcs);
static int xxPolyText8(DrawablePtr pDraw, GCPtr	pGC, int x, int	y, int count,
		       char *chars);
static int xxPolyText16(DrawablePtr pDraw, GCPtr pGC, int x, int y,
			int count, unsigned short *chars);
static void xxImageText8(DrawablePtr pDraw, GCPtr pGC, int x, 
			 int y, int count, char	*chars);
static void xxImageText16(DrawablePtr pDraw, GCPtr pGC, int x, int y,
			  int count, unsigned short *chars);
static void xxImageGlyphBlt(DrawablePtr pDraw, GCPtr pGC, int x, int y,
			    unsigned int nglyph, CharInfoPtr *ppci,
			    pointer pglyphBase);
static void xxPolyGlyphBlt(DrawablePtr pDraw, GCPtr pGC, int x, int y,
			   unsigned int nglyph, CharInfoPtr *ppci,
			   pointer pglyphBase);
static void xxPushPixels(GCPtr pGC, PixmapPtr pBitMap, DrawablePtr pDraw,
			 int	dx, int dy, int xOrg, int yOrg);
static void
xxComposite (CARD8 op, PicturePtr pSrc, PicturePtr pMask, PicturePtr pDst,
	     INT16 xSrc, INT16 ySrc, INT16 xMask, INT16 yMask,
	     INT16 xDst, INT16 yDst, CARD16 width, CARD16 height);
static void
xxGlyphs (CARD8 op, PicturePtr pSrc, PicturePtr pDst,
	  PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int nlist,
	  GlyphListPtr list, GlyphPtr *glyphs);


typedef struct _xxCmapPrivRec {
    CARD32* cmap;
    ColormapPtr pmap;
    Bool dirty;
    struct _xxCmapPrivRec *next;
} xxCmapPrivRec, *xxCmapPrivPtr;


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
    PixmapPtr			pPixmap;
    char *			addr;
    pointer			pBits;
    RegionRec			region;
    VisualPtr			bVisual;
    RegionRec			bRegion;
    int				myDepth;
    int				depth;
    ColormapPtr			baseCmap;
    ColormapPtr*		InstalledCmaps;
    xxCmapPrivPtr		Cmaps;
    int				numInstalledColormaps;
    int				colormapDirty;
    xxSyncFunc			sync;
} xxScrPrivRec, *xxScrPrivPtr;

#define xxGetScrPriv(s)	((xxScrPrivPtr) \
    dixLookupPrivate(&(s)->devPrivates, xxScrPrivateKey))
#define xxScrPriv(s)     xxScrPrivPtr pScrPriv = xxGetScrPriv(s)

#define xxGetCmapPriv(s) ((xxCmapPrivPtr) \
    dixLookupPrivate(&(s)->devPrivates, xxColormapPrivateKey))
#define xxCmapPriv(s)    xxCmapPrivPtr pCmapPriv = xxGetCmapPriv(s);

typedef struct _xxGCPriv {
    GCOps   *ops;
    GCFuncs *funcs;
} xxGCPrivRec, *xxGCPrivPtr;

#define xxGetGCPriv(pGC) ((xxGCPrivPtr) \
    dixLookupPrivate(&(pGC)->devPrivates, xxGCPrivateKey))
#define xxGCPriv(pGC)   xxGCPrivPtr  pGCPriv = xxGetGCPriv(pGC)

static DevPrivateKey xxScrPrivateKey = &xxScrPrivateKey;
static DevPrivateKey xxGCPrivateKey = &xxGCPrivateKey;
static DevPrivateKey xxColormapPrivateKey = &xxColormapPrivateKey;


#define wrap(priv,real,mem,func) {\
    priv->mem = real->mem; \
    real->mem = func; \
}

#define unwrap(priv,real,mem) {\
    real->mem = priv->mem; \
}

#define MARK_DIRTY (1 << 31)

#define MAX_NUM_XX_INSTALLED_CMAPS 255
/* #define DEBUG  */
#ifdef DEBUG
# define DBG ErrorF
# define DBG_ARGS(x) ErrorF x
# define PRINT_RECTS(rec) {\
       int i;\
       BoxPtr box;\
       ErrorF("RECTS: %i\n",REGION_NUM_RECTS(&rec));\
       if (REGION_NUM_RECTS(&rec) > 1)  { \
          for (i = 0; i < REGION_NUM_RECTS(&rec); i++ ) {\
             box = REGION_BOX(&rec,i);\
	     ErrorF("x1: %hi x2: %hi y1: %hi y2: %hi\n", \
             box->x1,box->x2,box->y1,box->y2);\
          }\
       } else { \
             box = &(rec.extents); \
	     ErrorF("x1: %hi x2: %hi y1: %hi y2: %hi\n", \
             box->x1,box->x2,box->y1,box->y2);\
       } \
}
#else
# define DBG(x)
# define DBG_ARGS(x)
# define PRINT_RECTS(rec)
#endif

#if 0
static void xxCopyPseudocolorRegion(ScreenPtr pScreen, RegionPtr pReg,
				    xxCmapPrivPtr pCmapPriv);
static void xxUpdateFb(ScreenPtr pScreen);


static void
xxUpdateWindowImmediately(WindowPtr pWin)
{
    xxScrPriv(pWin->drawable.pScreen);
    xxCmapPrivPtr pCmapPriv;
    ColormapPtr pmap;
	    
    pmap = (ColormapPtr)LookupIDByType(wColormap(pWin),RT_COLORMAP);
    
    if (pmap && (pCmapPriv = xxGetCmapPriv(pmap)) != (pointer)-1) {
	xxCopyPseudocolorRegion(pWin->drawable.pScreen,
				&pScrPriv->region, pCmapPriv);
    }
}
#else
# define xxUpdateWindowImmediately(x)
#endif

static ColormapPtr
xxGetBaseColormap(ScreenPtr pScreen)
{
    xxScrPriv(pScreen);
    DepthPtr pDepth = pScreen->allowedDepths;
    int i,j,k;
    ColormapPtr pDefMap
	=  (ColormapPtr) LookupIDByType(pScreen->defColormap,RT_COLORMAP);
    ColormapPtr cmap = NULL;
    VisualPtr pVisual = NULL;
	
    for (i = 0; i < pScreen->numDepths; i++, pDepth++)
	if (pDepth->depth == pScrPriv->depth) {
	    for (j = 0; j < pDepth->numVids; j++) {
		if (pDefMap->pVisual->vid == pDepth->vids[j]
		    && pDefMap->pVisual->class == TrueColor) {
		    cmap = pDefMap;
		    break;
		}
		if (!pVisual) {
		    for (k = 0; k < pScreen->numVisuals; k++) {
			if (pScreen->visuals[k].class == TrueColor
			    && pScreen->visuals[k].vid
			    == pDepth->vids[j]) {
			    pVisual = &pScreen->visuals[k];
			    break;
			}
		    }
		}
	    }
	    if (cmap)
		break;
	}
	    
    if (!cmap) {
	CreateColormap(FakeClientID(0),pScreen,pVisual,&cmap,AllocNone,0);
    }
    
    return cmap;
}

static Bool
xxCreateScreenResources(ScreenPtr pScreen)
{
    PixmapPtr		pPix;
    xxScrPriv(pScreen);
    Bool		ret;
    PixmapPtr		pPixmap;
    BoxRec		box;
    int			depth = pScrPriv->myDepth;
    pointer		pBits;
    
    unwrap (pScrPriv,pScreen, CreateScreenResources);
    ret = pScreen->CreateScreenResources(pScreen);
    wrap(pScrPriv,pScreen,CreateScreenResources,xxCreateScreenResources);

    if (!ret) return FALSE;
    
    pScrPriv->pBits = NULL;
    if (pScrPriv->addr)
	pBits = pScrPriv->addr;
    else
	pBits = xalloc(pScreen->width * pScreen->height
		       * (BitsPerPixel(depth) >> 3));
    if (!pBits) return FALSE;
    
    pPixmap = (*pScreen->CreatePixmap)(pScreen, 0, 0, depth, 0);
    if (!pPixmap) {
	xfree(pBits);
	return FALSE;
    }
    if (!(*pScreen->ModifyPixmapHeader)(pPixmap, pScreen->width,
					pScreen->height, depth,
					BitsPerPixel(depth),
					PixmapBytePad(pScreen->width, depth),
					pBits)) {
	xfree(pBits);
	return FALSE;
    }
    if (pScreen->rootDepth == pScrPriv->myDepth) {
	pPix = (PixmapPtr)pScreen->devPrivate;    
	if (!(*pScreen->ModifyPixmapHeader)(pPix, 0,0, pScrPriv->depth,
					    BitsPerPixel(pScrPriv->depth),
					    PixmapBytePad(pScreen->width,
							  pScrPriv->depth),
					    0)) {
	    xfree(pBits);
	    return FALSE;
	}
    }

    pScrPriv->baseCmap = xxGetBaseColormap(pScreen);
    
    pScrPriv->pBits = pBits;
    pScrPriv->pPixmap = pPixmap;
    box.x1 = 0;
    box.y1 = 0;
    box.x2 = pScreen->width;
    box.y2 = pScreen->height;
    REGION_NULL(pScreen, &pScrPriv->region);
    REGION_INIT(pScreen, &pScrPriv->bRegion, &box, 0);
    
    return TRUE;
}

static Bool
xxCloseScreen (int iScreen, ScreenPtr pScreen)
{
    xxScrPriv(pScreen);
    Bool		ret;

    (*pScreen->DestroyPixmap)(pScrPriv->pPixmap);
    /* We don't need to free the baseColormap as FreeClientResourcess
       will have taken care of it. */
    REGION_UNINIT (pScreen, &pScrPriv->region);
    
    unwrap (pScrPriv,pScreen, CloseScreen);
    ret = pScreen->CloseScreen(iScreen,pScreen);

    xfree(pScrPriv->pBits);
    xfree(pScrPriv->InstalledCmaps);
    xfree(pScrPriv);
    
    return TRUE;
}

static Bool
xxMyVisual(ScreenPtr pScreen, VisualID vid)
{
    xxScrPriv(pScreen);
    DepthPtr pDepth = pScreen->allowedDepths;
    int i,j;
    
    for (i = 0; i < pScreen->numDepths; i++, pDepth++)
	if (pDepth->depth == pScrPriv->myDepth) {
	    for (j = 0; j < pDepth->numVids; j++) {
		if (vid == pDepth->vids[j]) {
		    return TRUE;
		}
	    }
	}
    return FALSE;
}

static Bool
xxInitColormapPrivate(ColormapPtr pmap)
{
    xxScrPriv(pmap->pScreen);
    xxCmapPrivPtr	pCmapPriv;
    pointer		cmap;

    dixSetPrivate(&pmap->devPrivates, xxColormapPrivateKey, (pointer) -1);
    
    if (xxMyVisual(pmap->pScreen,pmap->pVisual->vid)) {
	DBG("CreateColormap\n");
	pCmapPriv = (xxCmapPrivPtr) xalloc (sizeof (xxCmapPrivRec));
	if (!pCmapPriv)
	    return FALSE;
	dixSetPrivate(&pmap->devPrivates, xxColormapPrivateKey, pCmapPriv);
	cmap = xalloc(sizeof (CARD32) * (1 << pScrPriv->myDepth));
	if (!cmap)
	return FALSE;

	memset(cmap,0,sizeof (CARD32) * (1 << pScrPriv->myDepth));
	
	pCmapPriv->cmap = cmap;
	pCmapPriv->dirty = FALSE;
	pCmapPriv->pmap = pmap;
	pCmapPriv->next = pScrPriv->Cmaps;
	pScrPriv->Cmaps = pCmapPriv;
    }
    return TRUE;
}


static Bool
xxCreateColormap(ColormapPtr pmap)
{
    xxScrPriv(pmap->pScreen);
    Bool		ret;
    
    if (!xxInitColormapPrivate(pmap)) return FALSE;
    
    unwrap(pScrPriv,pmap->pScreen, CreateColormap);
    ret = pmap->pScreen->CreateColormap(pmap);
    wrap(pScrPriv,pmap->pScreen,CreateColormap,xxCreateColormap);
    
    return ret;
}

static int
xxCmapInstalled(ColormapPtr pmap)
{
    xxScrPriv(pmap->pScreen);
    int i;
    
    for (i = 0; i < pScrPriv->numInstalledColormaps; i++)
	if (pScrPriv->InstalledCmaps[i] == pmap)
	    break;
	if (i == pScrPriv->numInstalledColormaps) /* not installed */
	    return -1;
	return i;
}

static void
xxInstalledCmapDelete(ScreenPtr pScreen, int num)
{
    xxScrPriv(pScreen);
    int i;

    pScrPriv->numInstalledColormaps--;
    
    for (i = num; i < pScrPriv->numInstalledColormaps; i++)
	pScrPriv->InstalledCmaps[i] = pScrPriv->InstalledCmaps[i+1];
}

static void
xxDestroyColormap(ColormapPtr pmap)
{
    xxScrPriv(pmap->pScreen);
    xxCmapPriv(pmap);

    if (pCmapPriv != (pointer) -1) {
	xxCmapPrivPtr tmpCmapPriv = pScrPriv->Cmaps;
	xxCmapPrivPtr *prevCmapPriv = &pScrPriv->Cmaps;
	int n;
	
	DBG("DestroyColormap\n");

	if ((n = xxCmapInstalled(pmap)) != -1)
	    xxInstalledCmapDelete(pmap->pScreen,n);

	while (tmpCmapPriv) {
	    if (tmpCmapPriv->pmap == pmap) {
		*prevCmapPriv = tmpCmapPriv->next;
		break;
	    }
	    prevCmapPriv = &tmpCmapPriv->next;
	    tmpCmapPriv = tmpCmapPriv->next;
	}
	
	xfree(pCmapPriv->cmap);
	xfree(pCmapPriv);
    }

    unwrap(pScrPriv,pmap->pScreen, DestroyColormap);
    pmap->pScreen->DestroyColormap(pmap);
    wrap(pScrPriv,pmap->pScreen,DestroyColormap,xxDestroyColormap);
}

#define Shift(v,d)  ((d) < 0 ? ((v) >> (-d)) : ((v) << (d)))

static int
xxComputeCmapShift (unsigned long mask)
{
    int	shift;
    unsigned long   bit;
    
    shift = 16;
    bit = 0x80000000;
    while (!(mask & bit))
    {
	shift--;
	bit >>= 1;
    }
    return shift;
}

static void
xxStoreColors(ColormapPtr pmap, int nColors, xColorItem *pColors)
{
    xxScrPriv(pmap->pScreen);
    xxCmapPriv(pmap);

    if (pCmapPriv != (pointer) -1) {

	xColorItem	*expanddefs;
	int		i;
	VisualPtr	bVisual;
	int		rs, gs, bs;

	if (nColors == 0) return;

	DBG("StoreColors\n");
	
	expanddefs = xalloc(sizeof(xColorItem)
				    * (1 <<  pScrPriv->myDepth));
	if (!expanddefs) return;
	
	bVisual = pScrPriv->bVisual;

	DBG("StoreColors\n");

	rs = xxComputeCmapShift(bVisual->redMask);
	gs = xxComputeCmapShift(bVisual->greenMask);
	bs = xxComputeCmapShift(bVisual->blueMask);
	
	if ((pmap->pVisual->class | DynamicClass) == DirectColor) {
	    nColors = miExpandDirectColors(pmap, nColors, pColors, expanddefs);
	    pColors = expanddefs;
	}

	for (i = 0; i < nColors; i++) {
	    DBG_ARGS(("index: %i r 0x%x g 0x%x b 0x%x\n", pColors->pixel,
		   pColors->red, pColors->green, pColors->blue));
	    pCmapPriv->cmap[pColors->pixel] = MARK_DIRTY
		| (Shift(pColors->red, rs) & bVisual->redMask)
		| (Shift(pColors->green, gs) & bVisual->greenMask)
		| (Shift(pColors->blue, bs)  & bVisual->blueMask);
	    pColors++;
	}

	xfree(expanddefs);

	pCmapPriv->dirty = TRUE;
	pScrPriv->colormapDirty = TRUE;
	
	return;
    }
    
    unwrap(pScrPriv,pmap->pScreen, StoreColors);
    pmap->pScreen->StoreColors(pmap,nColors,pColors);
    wrap(pScrPriv,pmap->pScreen,StoreColors,xxStoreColors);
}

static void
xxInstallColormap(ColormapPtr pmap)
{
    int i;
    xxScrPriv(pmap->pScreen);
    xxCmapPriv(pmap);
    
    if (pCmapPriv != (pointer) -1) {
	Pixel		*pixels;
	xrgb		*colors;
	int		i;
	VisualPtr	pVisual;
	xColorItem	*defs;

	DBG("InstallColormap\n");

	if (xxCmapInstalled(pmap) != -1)
	    return;

	if (!pScrPriv->numInstalledColormaps) {
	    unwrap(pScrPriv,pmap->pScreen, InstallColormap);
	    pmap->pScreen->InstallColormap(pScrPriv->baseCmap);
	    wrap(pScrPriv,pmap->pScreen,InstallColormap,xxInstallColormap);
	}
	    
	pixels = xalloc(sizeof(Pixel) * (1 <<  pScrPriv->myDepth));
	colors = xalloc(sizeof(xrgb) * (1 <<  pScrPriv->myDepth));
	defs = xalloc(sizeof(xColorItem) * (1 << pScrPriv->myDepth));
	
	if (!pixels || !colors)
	    return;

	/* if we have more than max installed delete the oldest */
	if (pScrPriv->numInstalledColormaps == MAX_NUM_XX_INSTALLED_CMAPS)
	    xxInstalledCmapDelete(pmap->pScreen,0);
	
	pScrPriv->InstalledCmaps[pScrPriv->numInstalledColormaps] = pmap;
	pScrPriv->numInstalledColormaps++;
	
	pVisual = pScrPriv->bVisual;
	
	for (i = 0; i < (1 <<  pScrPriv->myDepth); i++)
	    pixels[i] = i;
	
	QueryColors (pmap, (1 << pScrPriv->myDepth), pixels, colors);

	for (i = 0; i < (1 <<  pScrPriv->myDepth); i++) {
	    defs[i].pixel = pixels[i];
            defs[i].red = colors[i].red;
            defs[i].green = colors[i].green;
            defs[i].blue = colors[i].blue;
            defs[i].flags =  DoRed|DoGreen|DoBlue;
        }
	xxStoreColors(pmap,(1 <<  pScrPriv->myDepth),defs);

	xfree(pixels);
	xfree(colors);
	xfree(defs);

	return;
    } 

    for (i = pScrPriv->numInstalledColormaps; i ; i--)
	WalkTree(pmap->pScreen, TellLostMap,
		 (char *)&pScrPriv->InstalledCmaps[i-1]->mid);
    
    pScrPriv->numInstalledColormaps = 0;
     
    unwrap(pScrPriv,pmap->pScreen, InstallColormap);
    pmap->pScreen->InstallColormap(pmap);
    wrap(pScrPriv,pmap->pScreen,InstallColormap,xxInstallColormap);
}

static void
xxUninstallColormap(ColormapPtr pmap)
{
    xxScrPriv(pmap->pScreen);
    xxCmapPriv(pmap);

    if (pCmapPriv != (pointer) -1) {
	int num;
	
	if ((num = xxCmapInstalled(pmap)) == -1)
	    return;
	
	DBG("UninstallColormap\n");
	xxInstalledCmapDelete(pmap->pScreen,num);

	return;
    } 
    
    unwrap(pScrPriv,pmap->pScreen, UninstallColormap);
    pmap->pScreen->UninstallColormap(pmap);
    wrap(pScrPriv,pmap->pScreen,UninstallColormap,xxUninstallColormap);
	
}

static int
xxListInstalledColormaps(ScreenPtr pScreen, Colormap *pCmapIds)
{
    int			n,i;
    xxScrPriv(pScreen);

    unwrap(pScrPriv,pScreen, ListInstalledColormaps);
    n = pScreen->ListInstalledColormaps(pScreen, pCmapIds);
    wrap (pScrPriv,pScreen,ListInstalledColormaps,xxListInstalledColormaps);

    pCmapIds += n;

    for (i = 0; i < pScrPriv->numInstalledColormaps; i++) {
	*pCmapIds++ = pScrPriv->InstalledCmaps[i]->mid;
	n++;
    }

    return n;
}

static Bool
xxCreateWindow(WindowPtr pWin)
{
    xxScrPriv(pWin->drawable.pScreen);
    
    if (pWin->drawable.class != InputOutput
	|| pScrPriv->myDepth != pWin->drawable.depth) {
	Bool ret;
	DBG("CreateWindow NoPseudo\n");
	unwrap (pScrPriv, pWin->drawable.pScreen, CreateWindow);
	ret = pWin->drawable.pScreen->CreateWindow(pWin);
	wrap(pScrPriv, pWin->drawable.pScreen, CreateWindow, xxCreateWindow);

	return ret;
    }
    
    DBG("CreateWindow\n");

    dixSetPrivate(&pWin->devPrivates, fbGetWinPrivateKey(), pScrPriv->pPixmap);
    PRINT_RECTS(pScrPriv->region);
	if (!pWin->parent) {
	REGION_EMPTY (pWin->drawable.pScreen, &pScrPriv->region);
    }
    PRINT_RECTS(pScrPriv->region);
    
    return TRUE;
}

static void
xxWalkChildren(WindowPtr pWin, RegionPtr pReg, PixmapPtr pPixmap)
{
    
    WindowPtr		pCurWin = pWin;
    
    do {
	if (fbGetWindowPixmap(pCurWin) == pPixmap) {
	    DBG("WalkWindow Add\n");
	    REGION_UNION(pWin->drawable.pScreen,pReg,pReg,
			 &pCurWin->borderClip);
	} else {
	    DBG("WalkWindow Sub\n");
	    REGION_SUBTRACT(pWin->drawable.pScreen,pReg,pReg,
			    &pCurWin->borderClip);
	}
	if (pCurWin->lastChild)
	    xxWalkChildren(pCurWin->lastChild,pReg, pPixmap);
    } while ((pCurWin = pCurWin->prevSib));
}

static void
xxPickMyWindows(WindowPtr pWin, RegionPtr pRgn)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    xxScrPriv(pScreen);

    if (fbGetWindowPixmap(pWin) == pScrPriv->pPixmap) {
	REGION_UNION(pWin->drawable.pScreen,pRgn,pRgn,&pWin->borderClip);
    }
    if (pWin->lastChild)
	xxWalkChildren(pWin->lastChild,pRgn,pScrPriv->pPixmap);
}

static void
xxCopyWindow(WindowPtr	pWin,
	     DDXPointRec	ptOldOrg,
	     RegionPtr	prgnSrc)
{
    ScreenPtr		pScreen = pWin->drawable.pScreen;
    xxScrPriv(pScreen);
    RegionRec		rgn;
    RegionRec		rgn_new;
    int			dx, dy;
    PixmapPtr pPixmap = fbGetWindowPixmap(pWin);

    DBG("xxCopyWindow\n");

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;

    REGION_NULL(pScreen, &rgn_new);
    REGION_UNION(pScreen, &rgn_new,&rgn_new,prgnSrc);
    REGION_TRANSLATE(pScreen,&rgn_new,-dx,-dy);

    REGION_NULL(pScreen, &rgn);
    xxPickMyWindows(pWin,&rgn);

    unwrap (pScrPriv, pScreen, CopyWindow);
    dixSetPrivate(&pWin->devPrivates, fbGetWinPrivateKey(),
		  fbGetScreenPixmap(pScreen));
    pScreen->CopyWindow(pWin, ptOldOrg, prgnSrc);
    dixSetPrivate(&pWin->devPrivates, fbGetWinPrivateKey(), pPixmap);
    wrap(pScrPriv, pScreen, CopyWindow, xxCopyWindow);

    REGION_INTERSECT(pScreen,&rgn,&rgn,&rgn_new);
    if (REGION_NOTEMPTY (pScreen,&rgn)) {
	fbCopyRegion(&pScrPriv->pPixmap->drawable,&pScrPriv->pPixmap->drawable,
		     0,&rgn,dx,dy,fbCopyWindowProc,0,(void*)0);
	REGION_TRANSLATE(pScreen,&rgn,dx,dy);
	REGION_INTERSECT(pScreen,&rgn_new,&pScrPriv->region,&rgn);
	REGION_SUBTRACT(pScreen,&pScrPriv->region,&pScrPriv->region,&rgn);
	REGION_TRANSLATE(pScreen,&rgn_new,-dx,-dy);
	REGION_UNION(pScreen,&pScrPriv->region,&pScrPriv->region,&rgn_new);
    }
#if 1
    REGION_UNINIT(pScreen,&rgn_new);
    REGION_UNINIT(pScreen,&rgn);
#endif
}

static void
xxWindowExposures (WindowPtr	pWin,
			  RegionPtr	prgn,
			  RegionPtr	other_exposed)
{
    xxScrPriv(pWin->drawable.pScreen);

    if (fbGetWindowPixmap(pWin) == pScrPriv->pPixmap) {
	DBG("WindowExposures\n");
	PRINT_RECTS(pScrPriv->region);
	REGION_UNION(pWin->drawable.pScreen,&pScrPriv->region,
		     &pScrPriv->region,
		     prgn);
	PRINT_RECTS(pScrPriv->region);
    } else {
	DBG("WindowExposures NonPseudo\n");
	PRINT_RECTS(pScrPriv->region);
	REGION_SUBTRACT(pWin->drawable.pScreen,&pScrPriv->region,
		     &pScrPriv->region,
		     prgn);
	PRINT_RECTS(pScrPriv->region);
    }
    unwrap (pScrPriv, pWin->drawable.pScreen, WindowExposures);
    pWin->drawable.pScreen->WindowExposures(pWin, prgn, other_exposed);
    wrap(pScrPriv, pWin->drawable.pScreen, WindowExposures, xxWindowExposures);
}

static void
xxCopyPseudocolorRegion(ScreenPtr pScreen, RegionPtr pReg,
			xxCmapPrivPtr pCmapPriv)
{
    xxScrPriv(pScreen);
    CARD32		mask = (1 << pScrPriv->myDepth) - 1;
    int			num = REGION_NUM_RECTS(pReg);
    BoxPtr		pbox = REGION_RECTS(pReg);
    int			width, height;
    CARD8		*src;
    CARD16		*dst, *dst_base;
    int			dst_stride;
    register CARD32	*cmap = pCmapPriv->cmap;
    register CARD8      *s;
    register CARD16     *d;
    int w;

    fbPrepareAccess((DrawablePtr)pScreen->devPrivate);

    dst_base = (CARD16*) ((PixmapPtr)pScreen->devPrivate)->devPrivate.ptr;
    dst_stride = (int)((PixmapPtr)pScreen->devPrivate)->devKind
	/ sizeof (CARD16);

    while (num--) {
	height = pbox->y2 - pbox->y1;
	width = pbox->x2 - pbox->x1;
	
	src = (unsigned char *) pScrPriv->pBits
	    + (pbox->y1 * pScreen->width) + pbox->x1;
	dst = dst_base + (pbox->y1 * dst_stride) + pbox->x1;
	while (height--) {
	    w = width;
	    s = src;
	    d = dst;

	    while(w--) {
		*(d++) = (CARD16)*(cmap + ((*(s++)) & mask));
	    }
	    src += pScreen->width;
	    dst += dst_stride;
	}
	pbox++;
    }

    fbFinishAccess(&((PixmapPtr)pScreen->devPrivate)->drawable);
}

static void
xxUpdateCmapPseudocolorRegion(ScreenPtr pScreen, RegionPtr pReg,
			xxCmapPrivPtr pCmapPriv)
{
    xxScrPriv(pScreen);
    CARD32		mask = (1 << pScrPriv->myDepth) - 1;
    int			num = REGION_NUM_RECTS(pReg);
    BoxPtr		pbox = REGION_RECTS(pReg);
    int			width, height;
    CARD8		*src;
    CARD16		*dst, *dst_base;
    int			dst_stride;
    register CARD32	val;
    register CARD32	*cmap = pCmapPriv->cmap;
    register CARD8      *s;
    register CARD16     *d;
    int w;

    dst_base = (CARD16*) ((PixmapPtr)pScreen->devPrivate)->devPrivate.ptr;
    dst_stride = (int)((PixmapPtr)pScreen->devPrivate)->devKind
	/ sizeof (CARD16);

    while (num--) {

	height = pbox->y2 - pbox->y1;
	width = pbox->x2 - pbox->x1;
	
	src = (unsigned char *) pScrPriv->pBits
	    + (pbox->y1 * pScreen->width) + pbox->x1;
	dst = dst_base + (pbox->y1 * dst_stride) + pbox->x1;
	while (height--) {
	    w = width;
	    s = src;
	    d = dst;
	    while(w--) {
		val = *(cmap + ((*(s++)) & mask));
      		if (val & MARK_DIRTY) {
		    *d = (CARD16) val;
		}
		d++;
	    }
	    src += pScreen->width;
	    dst += dst_stride;
	}
	pbox++;
    }
}

static void
xxGetWindowRegion(WindowPtr pWin,RegionPtr winreg)
{
    REGION_NULL(pWin->drawable.pScreen,winreg);
    /* get visible part of the border ...Argh */
    REGION_SUBTRACT(pWin->drawable.pScreen,winreg,&pWin->borderSize,
		    &pWin->winSize);
    REGION_INTERSECT(pWin->drawable.pScreen,winreg,winreg,
		     &pWin->borderClip);
    /* add window interior excluding children */
    REGION_UNION(pWin->drawable.pScreen,winreg,winreg,
		 &pWin->clipList);
}

static int
xxUpdateRegion(WindowPtr pWin, pointer unused)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    xxScrPriv(pScreen);
    ColormapPtr pmap = (pointer) -1;
    RegionRec		winreg, rgni;
    
    if (pScrPriv->myDepth == pWin->drawable.depth) {
	xxCmapPrivPtr pCmapPriv = (pointer)-1;
	xxGetWindowRegion(pWin,&winreg);

	if (pScrPriv->colormapDirty) {

	    pmap = (ColormapPtr)LookupIDByType(wColormap(pWin),RT_COLORMAP);
	    if (!pmap)
		goto CONTINUE; /* return ? */

	    pCmapPriv = xxGetCmapPriv(pmap);
	    if (pCmapPriv == (pointer) -1)
		return WT_WALKCHILDREN;
	    if (!pCmapPriv->dirty)
		goto CONTINUE;

	    REGION_NULL (pScreen, &rgni);
	    /* This will be taken care of when damaged regions are updated */
	    REGION_SUBTRACT(pScreen, &rgni, &winreg, &pScrPriv->region);
	    if (REGION_NOTEMPTY (pScreen,&rgni))
		xxUpdateCmapPseudocolorRegion(pScreen,&rgni, pCmapPriv);
	}
    CONTINUE:

	REGION_NULL (pScreen, &rgni);
	REGION_INTERSECT (pScreen, &rgni, &winreg, &pScrPriv->region);
	
	if (REGION_NOTEMPTY (pScreen,&rgni)) {
	    if (pmap == (pointer) -1) {
		pmap =
		    (ColormapPtr)LookupIDByType(wColormap(pWin),RT_COLORMAP);
		if (!pmap) /* return ? */
		    pmap = (ColormapPtr)LookupIDByType(pScreen->defColormap,
						       RT_COLORMAP);
		pCmapPriv = xxGetCmapPriv(pmap);
	    }
	    
	    if (pCmapPriv != (pointer)-1)
		xxCopyPseudocolorRegion(pScreen,&rgni, pCmapPriv);
	    REGION_SUBTRACT(pScreen, &pScrPriv->region, &pScrPriv->region,
			    &rgni);
	}
#if 1
	REGION_UNINIT(pScreen,&rgni);
	REGION_UNINIT(pScreen,&winreg);
#endif
    }
    return WT_WALKCHILDREN;
}


static void
xxUpdateFb(ScreenPtr pScreen)
{
    xxScrPriv(pScreen);

    DBG("Update FB\n");
    PRINT_RECTS(pScrPriv->region);

    if (pScrPriv->sync)
	pScrPriv->sync(pScreen); /*@!@*/
    
    WalkTree(pScreen,xxUpdateRegion,NULL);
#if 0
    if (REGION_NOTEMPTY (pScreen,&pScrPriv->region)) {
	ColormapPtr pmap = (pointer) -1;
	xxCmapPrivPtr pCmapPriv;
	
	pmap = (ColormapPtr)LookupIDByType(pScreen->defColormap,
					   RT_COLORMAP);
	pCmapPriv = xxGetCmapPriv(pmap);
	if (pCmapPriv != (pointer)-1)
	    xxCopyPseudocolorRegion(pScreen,&pScrPriv->region, pCmapPriv);
	REGION_SUBTRACT(pScreen, &pScrPriv->region, &pScrPriv->region,
			&pScrPriv->region);
    }
#endif
    if (pScrPriv->colormapDirty) {
	xxCmapPrivPtr pCmap = pScrPriv->Cmaps;

	while (pCmap) {
	    int j;

	    if (pCmap->dirty) {
		for (j = 0; j < (1 <<  pScrPriv->myDepth); j++) 
		    pCmap->cmap[j] &= ~MARK_DIRTY;
		pCmap->dirty = FALSE;
	    }
	    pCmap = pCmap->next;
	}
	pScrPriv->colormapDirty = FALSE;
    }
}

static void
xxBlockHandler (pointer	data,
		OSTimePtr pTimeout,
		pointer pRead)
{
    ScreenPtr	pScreen = (ScreenPtr) data;
    xxScrPriv(pScreen);

    if (REGION_NOTEMPTY (pScreen,&pScrPriv->region) || pScrPriv->colormapDirty)
	xxUpdateFb (pScreen);
}

static void
xxWakeupHandler (pointer data, int i, pointer LastSelectMask)
{
}

Bool
xxSetup(ScreenPtr pScreen, int myDepth, int baseDepth, char* addr, xxSyncFunc sync)
{
    xxScrPrivPtr	pScrPriv;
    DepthPtr		pDepths;
    ColormapPtr		pDefMap;
    int i,j,k;
    
#ifdef RENDER
    PictureScreenPtr	ps = GetPictureScreenIfSet(pScreen);
#endif

    if (!dixRequestPrivate(xxGCPrivateKey, sizeof (xxGCPrivRec)))
	return FALSE;

    pScrPriv = (xxScrPrivPtr) xalloc (sizeof (xxScrPrivRec));
    if (!pScrPriv)
	return FALSE;
    
    if (baseDepth)
	pScrPriv->depth = baseDepth;
    else {
	pDepths = pScreen->allowedDepths;
        for (i = 0; i < pScreen->numDepths; i++, pDepths++)
	    if (pDepths->depth != myDepth)
		pScrPriv->depth = pDepths->depth;
    }
    if (!pScrPriv->depth)
	return FALSE;
    
    pDepths = pScreen->allowedDepths;
    for (i = 0; i < pScreen->numDepths; i++, pDepths++)
	if (pDepths->depth == pScrPriv->depth) {
	    for (j = 0; i < pDepths->numVids; j++) {
		for (k = 0; k < pScreen->numVisuals; k++) {
		    if (pScreen->visuals[k].vid
			== pDepths[i].vids[j]
			&& pScreen->visuals[k].class == TrueColor) {
			pScrPriv->bVisual =  &pScreen->visuals[k];
			goto DONE;
		    }
		}
	    }
	}
    
 DONE:
    if (!pScrPriv->bVisual)
	return FALSE;

    pScrPriv->myDepth = myDepth;
    pScrPriv->numInstalledColormaps = 0;
    pScrPriv->colormapDirty = FALSE;
    pScrPriv->Cmaps = NULL;
    pScrPriv->sync = sync;
    
    pScreen->maxInstalledCmaps += MAX_NUM_XX_INSTALLED_CMAPS;
    pScrPriv->InstalledCmaps = xcalloc(MAX_NUM_XX_INSTALLED_CMAPS,
				       sizeof(ColormapPtr));
    if (!pScrPriv->InstalledCmaps)
	return FALSE;

    
    if (!RegisterBlockAndWakeupHandlers (xxBlockHandler,
					 xxWakeupHandler,
					 (pointer) pScreen))
	return FALSE;

    wrap (pScrPriv, pScreen, CloseScreen, xxCloseScreen);
    wrap (pScrPriv, pScreen, CreateScreenResources, xxCreateScreenResources);
    wrap (pScrPriv, pScreen, CreateWindow, xxCreateWindow);
    wrap (pScrPriv, pScreen, CopyWindow, xxCopyWindow);
#if 0 /* can we leave this out even with backing store enabled ? */
    wrap (pScrPriv, pScreen, WindowExposures, xxWindowExposures);
#endif
    wrap (pScrPriv, pScreen, CreateGC, xxCreateGC);
    wrap (pScrPriv, pScreen, CreateColormap, xxCreateColormap);
    wrap (pScrPriv, pScreen, DestroyColormap, xxDestroyColormap);
    wrap (pScrPriv, pScreen, InstallColormap, xxInstallColormap);
    wrap (pScrPriv, pScreen, UninstallColormap, xxUninstallColormap);
    wrap (pScrPriv, pScreen, ListInstalledColormaps, xxListInstalledColormaps);
    wrap (pScrPriv, pScreen, StoreColors, xxStoreColors);
#ifdef RENDER
    if (ps) {
	wrap (pScrPriv, ps, Glyphs, xxGlyphs);
	wrap (pScrPriv, ps, Composite, xxComposite);
    }
#endif
    pScrPriv->addr = addr;
    dixSetPrivate(&pScreen->devPrivates, xxScrPrivateKey, pScrPriv);

    pDefMap = (ColormapPtr) LookupIDByType(pScreen->defColormap, RT_COLORMAP);
    if (!xxInitColormapPrivate(pDefMap))
	return FALSE;
    
    return TRUE;
}

GCFuncs xxGCFuncs = {
    xxValidateGC, xxChangeGC, xxCopyGC, xxDestroyGC,
    xxChangeClip, xxDestroyClip, xxCopyClip
};

static GCOps xxGCOps = {
    xxFillSpans, xxSetSpans, 
    xxPutImage, xxCopyArea, 
    xxCopyPlane, xxPolyPoint, 
    xxPolylines, xxPolySegment, 
    xxPolyRectangle, xxPolyArc, 
    xxFillPolygon, xxPolyFillRect, 
    xxPolyFillArc, xxPolyText8, 
    xxPolyText16, xxImageText8, 
    xxImageText16, xxImageGlyphBlt, 
    xxPolyGlyphBlt, xxPushPixels,
    {NULL}		/* devPrivate */
};

#define IS_VISIBLE(pDraw) (pDraw->type == DRAWABLE_WINDOW \
	   && (fbGetWindowPixmap((WindowPtr) pDraw) == pScrPriv->pPixmap))

#define TRANSLATE_BOX(box, pDraw) { \
    box.x1 += pDraw->x; \
    box.x2 += pDraw->x; \
    box.y1 += pDraw->y; \
    box.y2 += pDraw->y; \
    }

#define TRIM_BOX(box, pGC) { \
    BoxPtr extents = &pGC->pCompositeClip->extents;\
    if(box.x1 < extents->x1) box.x1 = extents->x1; \
    if(box.x2 > extents->x2) box.x2 = extents->x2; \
    if(box.y1 < extents->y1) box.y1 = extents->y1; \
    if(box.y2 > extents->y2) box.y2 = extents->y2; \
    }

#define BOX_NOT_EMPTY(box) \
    (((box.x2 - box.x1) > 0) && ((box.y2 - box.y1) > 0))


#define _ADD_BOX(box,pGC) {\
    if (BOX_NOT_EMPTY(box)) { \
       RegionRec region; \
       ScreenPtr pScreen = pGC->pScreen;\
       REGION_INIT (pScreen, &region, &box, 1); \
       REGION_INTERSECT(pScreen,&region,&region,\
                                 (pGC)->pCompositeClip);\
       if (REGION_NOTEMPTY(pScreen,&region)) { \
           xxScrPriv(pScreen);\
	   PRINT_RECTS(pScrPriv->region);\
           REGION_UNION(pScreen,&pScrPriv->region,&pScrPriv->region,&region);\
	   PRINT_RECTS(pScrPriv->region);\
           REGION_UNINIT(pScreen,&region);\
       }\
   }\
}

#define TRANSLATE_AND_ADD_BOX(box,pGC) {\
         TRANSLATE_BOX(box,pDraw); \
         TRIM_BOX(box,pGC); \
         _ADD_BOX(box,pGC); \
}

#define ADD_BOX(box,pGC) { \
        TRIM_BOX(box,pGC); \
        _ADD_BOX(box,pGC); \
}

#define XX_GC_FUNC_PROLOGUE(pGC) \
    xxGCPriv(pGC); \
    unwrap(pGCPriv, pGC, funcs); \
    if (pGCPriv->ops) unwrap(pGCPriv, pGC, ops)

#define XX_GC_FUNC_EPILOGUE(pGC) \
    wrap(pGCPriv, pGC, funcs, &xxGCFuncs);  \
    if (pGCPriv->ops) wrap(pGCPriv, pGC, ops, &xxGCOps)

static Bool
xxCreateGC(GCPtr pGC)
{
    ScreenPtr		pScreen = pGC->pScreen;
    xxScrPriv(pScreen);
    xxGCPriv(pGC);
    Bool ret;

    unwrap (pScrPriv, pScreen, CreateGC);
    if((ret = (*pScreen->CreateGC) (pGC))) {
	pGCPriv->ops = NULL;
	pGCPriv->funcs = pGC->funcs;
	pGC->funcs = &xxGCFuncs;
    }
    wrap (pScrPriv, pScreen, CreateGC, xxCreateGC);

    return ret;
}

static void
xxValidateGC(
   GCPtr         pGC,
   unsigned long changes,
   DrawablePtr   pDraw 
){
    XX_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ValidateGC)(pGC, changes, pDraw);
    if(pDraw->type == DRAWABLE_WINDOW)
	pGCPriv->ops = pGC->ops;  /* just so it's not NULL */
    else 
	pGCPriv->ops = NULL;
    XX_GC_FUNC_EPILOGUE (pGC);
}

static void
xxDestroyGC(GCPtr pGC)
{
    XX_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->DestroyGC)(pGC);
    XX_GC_FUNC_EPILOGUE (pGC);
}

static void
xxChangeGC (
    GCPtr	    pGC,
    unsigned long   mask
){
    XX_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeGC) (pGC, mask);
    XX_GC_FUNC_EPILOGUE (pGC);
}

static void
xxCopyGC (
    GCPtr	    pGCSrc, 
    unsigned long   mask,
    GCPtr	    pGCDst
){
    XX_GC_FUNC_PROLOGUE (pGCDst);
    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    XX_GC_FUNC_EPILOGUE (pGCDst);
}

static void
xxChangeClip (
    GCPtr   pGC,
    int		type,
    pointer	pvalue,
    int		nrects 
){
    XX_GC_FUNC_PROLOGUE (pGC);
    (*pGC->funcs->ChangeClip) (pGC, type, pvalue, nrects);
    XX_GC_FUNC_EPILOGUE (pGC);
}

static void
xxCopyClip(GCPtr pgcDst, GCPtr pgcSrc)
{
    XX_GC_FUNC_PROLOGUE (pgcDst);
    (* pgcDst->funcs->CopyClip)(pgcDst, pgcSrc);
    XX_GC_FUNC_EPILOGUE (pgcDst);
}

static void
xxDestroyClip(GCPtr pGC)
{
    XX_GC_FUNC_PROLOGUE (pGC);
    (* pGC->funcs->DestroyClip)(pGC);
    XX_GC_FUNC_EPILOGUE (pGC);
}

#define XX_GC_OP_PROLOGUE(pGC,pDraw) \
    xxScrPriv(pDraw->pScreen); \
    xxGCPriv(pGC);  \
    GCFuncs *oldFuncs = pGC->funcs; \
    unwrap(pGCPriv, pGC, funcs);  \
    unwrap(pGCPriv, pGC, ops); \
	
#define XX_GC_OP_EPILOGUE(pGC,pDraw) \
    wrap(pGCPriv, pGC, funcs, oldFuncs); \
    wrap(pGCPriv, pGC, ops, &xxGCOps)

static void
xxFillSpans(
    DrawablePtr pDraw,
    GC		*pGC,
    int		nInit,	
    DDXPointPtr pptInit,	
    int 	*pwidthInit,		
    int 	fSorted 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);    

    if(IS_VISIBLE(pDraw) && nInit) {
	DDXPointPtr ppt = pptInit;
	int *pwidth = pwidthInit;
	int i = nInit;
	BoxRec box;

	DBG("FillSpans\n");
	box.x1 = ppt->x;
	box.x2 = box.x1 + *pwidth;
	box.y2 = box.y1 = ppt->y;

	while(--i) {
	   ppt++;
	   pwidthInit++;
	   if(box.x1 > ppt->x) box.x1 = ppt->x;
	   if(box.x2 < (ppt->x + *pwidth)) 
		box.x2 = ppt->x + *pwidth;
	   if(box.y1 > ppt->y) box.y1 = ppt->y;
	   else if(box.y2 < ppt->y) box.y2 = ppt->y;
	}

	box.y2++;

	(*pGC->ops->FillSpans)(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted);

	
	TRANSLATE_AND_ADD_BOX(box, pGC);
    } else
	(*pGC->ops->FillSpans)(pDraw, pGC, nInit, pptInit, pwidthInit, fSorted);

    XX_GC_OP_EPILOGUE(pGC, pDraw);
}

static void
xxSetSpans(
    DrawablePtr		pDraw,
    GCPtr		pGC,
    char		*pcharsrc,
    DDXPointPtr 	pptInit,
    int			*pwidthInit,
    int			nspans,
    int			fSorted 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && nspans) {
	DDXPointPtr ppt = pptInit;
	int *pwidth = pwidthInit;
	int i = nspans;
	BoxRec box;

	DBG("SetSpans\n");
	box.x1 = ppt->x;
	box.x2 = box.x1 + *pwidth;
	box.y2 = box.y1 = ppt->y;

	while(--i) {
	   ppt++;
	   pwidth++;
	   if(box.x1 > ppt->x) box.x1 = ppt->x;
	   if(box.x2 < (ppt->x + *pwidth)) 
		box.x2 = ppt->x + *pwidth;
	   if(box.y1 > ppt->y) box.y1 = ppt->y;
	   else if(box.y2 < ppt->y) box.y2 = ppt->y;
	}

	box.y2++;

	(*pGC->ops->SetSpans)(pDraw, pGC, pcharsrc, pptInit, 
				pwidthInit, nspans, fSorted);

	TRANSLATE_AND_ADD_BOX(box, pGC);
    } else
	(*pGC->ops->SetSpans)(pDraw, pGC, pcharsrc, pptInit, 
				pwidthInit, nspans, fSorted);

    XX_GC_OP_EPILOGUE(pGC, pDraw);
}

static void
xxPutImage(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		depth, 
    int x, int y, int w, int h,
    int		leftPad,
    int		format,
    char 	*pImage 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->PutImage)(pDraw, pGC, depth, x, y, w, h, 
		leftPad, format, pImage);
    XX_GC_OP_EPILOGUE(pGC, pDraw);
    if(IS_VISIBLE(pDraw)) {
	BoxRec box;

	DBG("PutImage\n");
	box.x1 = x + pDraw->x;
	box.x2 = box.x1 + w;
	box.y1 = y + pDraw->y;
	box.y2 = box.y1 + h;

	ADD_BOX(box, pGC);
    }
}

static RegionPtr
xxCopyArea(
    DrawablePtr pSrc,
    DrawablePtr pDst,
    GC *pGC,
    int srcx, int srcy,
    int width, int height,
    int dstx, int dsty 
){
    RegionPtr ret;
    XX_GC_OP_PROLOGUE(pGC, pDst);
    DBG("xxCopyArea\n");
    ret = (*pGC->ops->CopyArea)(pSrc, pDst,
            pGC, srcx, srcy, width, height, dstx, dsty);
    XX_GC_OP_EPILOGUE(pGC, pDst);

    if(IS_VISIBLE(pDst)) {
	BoxRec box;

	DBG("CopyArea\n");
	box.x1 = dstx + pDst->x;
	box.x2 = box.x1 + width;
	box.y1 = dsty + pDst->y;
	box.y2 = box.y1 + height;

	ADD_BOX(box, pGC);
    }

    return ret;
}

static RegionPtr
xxCopyPlane(
    DrawablePtr	pSrc,
    DrawablePtr	pDst,
    GCPtr pGC,
    int	srcx, int srcy,
    int	width, int height,
    int	dstx, int dsty,
    unsigned long bitPlane 
){
    RegionPtr ret;
    XX_GC_OP_PROLOGUE(pGC, pDst);
    ret = (*pGC->ops->CopyPlane)(pSrc, pDst,
	       pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    XX_GC_OP_EPILOGUE(pGC, pDst);

    if(IS_VISIBLE(pDst)) {
	BoxRec box;

	DBG("CopyPlane\n");
	box.x1 = dstx + pDst->x;
	box.x2 = box.x1 + width;
	box.y1 = dsty + pDst->y;
	box.y2 = box.y1 + height;

	ADD_BOX(box, pGC);
    }

    return ret;
}

static void
xxPolyPoint(
    DrawablePtr pDraw,
    GCPtr pGC,
    int mode,
    int npt,
    xPoint *pptInit 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->PolyPoint)(pDraw, pGC, mode, npt, pptInit);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && npt) {
	BoxRec box;

	DBG("PolyPoint\n");
	box.x2 = box.x1 = pptInit->x;
	box.y2 = box.y1 = pptInit->y;

	/* this could be slow if the points were spread out */

	while(--npt) {
	   pptInit++;
	   if(box.x1 > pptInit->x) box.x1 = pptInit->x;
	   else if(box.x2 < pptInit->x) box.x2 = pptInit->x;
	   if(box.y1 > pptInit->y) box.y1 = pptInit->y;
	   else if(box.y2 < pptInit->y) box.y2 = pptInit->y;
	}

	box.x2++;
	box.y2++;

	TRANSLATE_AND_ADD_BOX(box, pGC);
    }
}

static void
xxPolylines(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		mode,		
    int		npt,		
    DDXPointPtr pptInit 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->Polylines)(pDraw, pGC, mode, npt, pptInit);
    XX_GC_OP_EPILOGUE(pGC, pDraw);


    if(IS_VISIBLE(pDraw) && npt) {
	BoxRec box;
	int extra = pGC->lineWidth >> 1;

	DBG("PolyLine\n");
	box.x2 = box.x1 = pptInit->x;
	box.y2 = box.y1 = pptInit->y;

	if(npt > 1) {
	   if(pGC->joinStyle == JoinMiter)
		extra = 6 * pGC->lineWidth;
	   else if(pGC->capStyle == CapProjecting)
		extra = pGC->lineWidth;
        }

	if(mode == CoordModePrevious) {
	   int x = box.x1;
	   int y = box.y1;
	   while(--npt) {
		pptInit++;
		x += pptInit->x;
		y += pptInit->y;
		if(box.x1 > x) box.x1 = x;
		else if(box.x2 < x) box.x2 = x;
		if(box.y1 > y) box.y1 = y;
		else if(box.y2 < y) box.y2 = y;
	    }
	} else {
	   while(--npt) {
		pptInit++;
		if(box.x1 > pptInit->x) box.x1 = pptInit->x;
		else if(box.x2 < pptInit->x) box.x2 = pptInit->x;
		if(box.y1 > pptInit->y) box.y1 = pptInit->y;
		else if(box.y2 < pptInit->y) box.y2 = pptInit->y;
	    }
	}

	box.x2++;
	box.y2++;

	if(extra) {
	   box.x1 -= extra;
	   box.x2 += extra;
	   box.y1 -= extra;
	   box.y2 += extra;
        }

	TRANSLATE_AND_ADD_BOX(box, pGC);
    }
}

static void 
xxPolySegment(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nseg,
    xSegment	*pSeg
    ){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->PolySegment)(pDraw, pGC, nseg, pSeg);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && nseg) {
	BoxRec box;
	int extra = pGC->lineWidth;

	DBG("PolySegment\n");
        if(pGC->capStyle != CapProjecting)	
	   extra >>= 1;

	if(pSeg->x2 > pSeg->x1) {
	    box.x1 = pSeg->x1;
	    box.x2 = pSeg->x2;
	} else {
	    box.x2 = pSeg->x1;
	    box.x1 = pSeg->x2;
	}

	if(pSeg->y2 > pSeg->y1) {
	    box.y1 = pSeg->y1;
	    box.y2 = pSeg->y2;
	} else {
	    box.y2 = pSeg->y1;
	    box.y1 = pSeg->y2;
	}

	while(--nseg) {
	    pSeg++;
	    if(pSeg->x2 > pSeg->x1) {
		if(pSeg->x1 < box.x1) box.x1 = pSeg->x1;
		if(pSeg->x2 > box.x2) box.x2 = pSeg->x2;
	    } else {
		if(pSeg->x2 < box.x1) box.x1 = pSeg->x2;
		if(pSeg->x1 > box.x2) box.x2 = pSeg->x1;
	    }
	    if(pSeg->y2 > pSeg->y1) {
		if(pSeg->y1 < box.y1) box.y1 = pSeg->y1;
		if(pSeg->y2 > box.y2) box.y2 = pSeg->y2;
	    } else {
		if(pSeg->y2 < box.y1) box.y1 = pSeg->y2;
		if(pSeg->y1 > box.y2) box.y2 = pSeg->y1;
	    }
	}

	box.x2++;
	box.y2++;

	if(extra) {
	   box.x1 -= extra;
	   box.x2 += extra;
	   box.y1 -= extra;
	   box.y2 += extra;
        }
	
	TRANSLATE_AND_ADD_BOX(box, pGC);
    }
}

static void
xxPolyRectangle(
    DrawablePtr  pDraw,
    GCPtr        pGC,
    int	         nRects,
    xRectangle  *pRects 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->PolyRectangle)(pDraw, pGC, nRects, pRects);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && nRects) 
    {
	BoxRec box;
	int offset1, offset2, offset3;

	DBG("PolyRectangle\n");
	offset2 = pGC->lineWidth;
	if(!offset2) offset2 = 1;
	offset1 = offset2 >> 1;
	offset3 = offset2 - offset1;

	while(nRects--) 
	{
	    box.x1 = pRects->x - offset1;
	    box.y1 = pRects->y - offset1;
	    box.x2 = box.x1 + pRects->width + offset2;
	    box.y2 = box.y1 + offset2;		
	    TRANSLATE_AND_ADD_BOX(box, pGC);
	    box.x1 = pRects->x - offset1;
	    box.y1 = pRects->y + offset3;
	    box.x2 = box.x1 + offset2;
	    box.y2 = box.y1 + pRects->height - offset2;		
	    TRANSLATE_AND_ADD_BOX(box, pGC);
	    box.x1 = pRects->x + pRects->width - offset1;
	    box.y1 = pRects->y + offset3;
	    box.x2 = box.x1 + offset2;
	    box.y2 = box.y1 + pRects->height - offset2;		
	    TRANSLATE_AND_ADD_BOX(box, pGC);
	    box.x1 = pRects->x - offset1;
	    box.y1 = pRects->y + pRects->height - offset1;
	    box.x2 = box.x1 + pRects->width + offset2;
	    box.y2 = box.y1 + offset2;		
	    TRANSLATE_AND_ADD_BOX(box, pGC);

	    pRects++;
	}
    }
}

static void
xxPolyArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->PolyArc)(pDraw, pGC, narcs, parcs);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && narcs) {
	int extra = pGC->lineWidth >> 1;
	BoxRec box;

	DBG("PolyArc\n");
	box.x1 = parcs->x;
	box.x2 = box.x1 + parcs->width;
	box.y1 = parcs->y;
	box.y2 = box.y1 + parcs->height;

	/* should I break these up instead ? */

	while(--narcs) {
	   parcs++;
	   if(box.x1 > parcs->x) box.x1 = parcs->x;
	   if(box.x2 < (parcs->x + parcs->width))
		box.x2 = parcs->x + parcs->width;
	   if(box.y1 > parcs->y) box.y1 = parcs->y;
	   if(box.y2 < (parcs->y + parcs->height))
		box.y2 = parcs->y + parcs->height;
        }

	if(extra) {
	   box.x1 -= extra;
	   box.x2 += extra;
	   box.y1 -= extra;
	   box.y2 += extra;
        }

	box.x2++;
	box.y2++;

	TRANSLATE_AND_ADD_BOX(box, pGC);
    }
}

static void
xxFillPolygon(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		shape,
    int		mode,
    int		count,
    DDXPointPtr	pptInit 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && (count > 2)) {
	DDXPointPtr ppt = pptInit;
	int i = count;
	BoxRec box;

	DBG("FillPolygon\n");
	box.x2 = box.x1 = ppt->x;
	box.y2 = box.y1 = ppt->y;

	if(mode != CoordModeOrigin) {
	   int x = box.x1;
	   int y = box.y1;
	   while(--i) {
		ppt++;
		x += ppt->x;
		y += ppt->y;
		if(box.x1 > x) box.x1 = x;
		else if(box.x2 < x) box.x2 = x;
		if(box.y1 > y) box.y1 = y;
		else if(box.y2 < y) box.y2 = y;
	    }
	} else {
	   while(--i) {
		ppt++;
		if(box.x1 > ppt->x) box.x1 = ppt->x;
		else if(box.x2 < ppt->x) box.x2 = ppt->x;
		if(box.y1 > ppt->y) box.y1 = ppt->y;
		else if(box.y2 < ppt->y) box.y2 = ppt->y;
	    }
	}

	box.x2++;
	box.y2++;

	(*pGC->ops->FillPolygon)(pDraw, pGC, shape, mode, count, pptInit);

	TRANSLATE_AND_ADD_BOX(box, pGC);
    } else
	(*pGC->ops->FillPolygon)(pDraw, pGC, shape, mode, count, pptInit);

    XX_GC_OP_EPILOGUE(pGC, pDraw);
}

static void 
xxPolyFillRect(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		nRectsInit, 
    xRectangle	*pRectsInit 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && nRectsInit) {
	BoxRec box;
	xRectangle *pRects = pRectsInit;
	int nRects = nRectsInit;

	DBG("PolyFillRect\n");
	box.x1 = pRects->x;
	box.x2 = box.x1 + pRects->width;
	box.y1 = pRects->y;
	box.y2 = box.y1 + pRects->height;

	while(--nRects) {
	    pRects++;
	    if(box.x1 > pRects->x) box.x1 = pRects->x;
	    if(box.x2 < (pRects->x + pRects->width))
		box.x2 = pRects->x + pRects->width;
	    if(box.y1 > pRects->y) box.y1 = pRects->y;
	    if(box.y2 < (pRects->y + pRects->height))
		box.y2 = pRects->y + pRects->height;
	}

	/* cfb messes with the pRectsInit so we have to do our
	   calculations first */

	(*pGC->ops->PolyFillRect)(pDraw, pGC, nRectsInit, pRectsInit);

	TRANSLATE_AND_ADD_BOX(box, pGC);
    } else
	(*pGC->ops->PolyFillRect)(pDraw, pGC, nRectsInit, pRectsInit);

    XX_GC_OP_EPILOGUE(pGC, pDraw);
}

static void
xxPolyFillArc(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->PolyFillArc)(pDraw, pGC, narcs, parcs);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && narcs) {
	BoxRec box;

	DBG("PolyFillArc\n");
	box.x1 = parcs->x;
	box.x2 = box.x1 + parcs->width;
	box.y1 = parcs->y;
	box.y2 = box.y1 + parcs->height;

	/* should I break these up instead ? */

	while(--narcs) {
	   parcs++;
	   if(box.x1 > parcs->x) box.x1 = parcs->x;
	   if(box.x2 < (parcs->x + parcs->width))
		box.x2 = parcs->x + parcs->width;
	   if(box.y1 > parcs->y) box.y1 = parcs->y;
	   if(box.y2 < (parcs->y + parcs->height))
		box.y2 = parcs->y + parcs->height;
        }

	TRANSLATE_AND_ADD_BOX(box, pGC);
    }
}

static int
xxPolyText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int 	y,
    int 	count,
    char	*chars 
){
    int width;

    XX_GC_OP_PROLOGUE(pGC, pDraw);
    width = (*pGC->ops->PolyText8)(pDraw, pGC, x, y, count, chars);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    width -= x;

    if(IS_VISIBLE(pDraw) && (width > 0)) {
	BoxRec box;

	DBG("PolyText8\n");
	/* ugh */
	box.x1 = pDraw->x + x + FONTMINBOUNDS(pGC->font, leftSideBearing);
	box.x2 = pDraw->x + x + FONTMAXBOUNDS(pGC->font, rightSideBearing);

	if(count > 1) {
	   if(width > 0) box.x2 += width;
	   else box.x1 += width;
	}

	box.y1 = pDraw->y + y - FONTMAXBOUNDS(pGC->font, ascent);
	box.y2 = pDraw->y + y + FONTMAXBOUNDS(pGC->font, descent);

	ADD_BOX(box,  pGC);
    }

    return (width + x);
}

static int
xxPolyText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars 
){
    int width;

    XX_GC_OP_PROLOGUE(pGC, pDraw);
    width = (*pGC->ops->PolyText16)(pDraw, pGC, x, y, count, chars);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    width -= x;

    if(IS_VISIBLE(pDraw) && (width > 0)) {
	BoxRec box;

	DBG("PolyText16\n");
	/* ugh */
	box.x1 = pDraw->x + x + FONTMINBOUNDS(pGC->font, leftSideBearing);
	box.x2 = pDraw->x + x + FONTMAXBOUNDS(pGC->font, rightSideBearing);

	if(count > 1) {
	   if(width > 0) box.x2 += width;
	   else box.x1 += width;
	}

	box.y1 = pDraw->y + y - FONTMAXBOUNDS(pGC->font, ascent);
	box.y2 = pDraw->y + y + FONTMAXBOUNDS(pGC->font, descent);

	ADD_BOX(box, pGC);
    }

    return (width + x);
}

static void
xxImageText8(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x, 
    int		y,
    int 	count,
    char	*chars 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->ImageText8)(pDraw, pGC, x, y, count, chars);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && count) {
	int top, bot, Min, Max;
	BoxRec box;

	DBG("ImageText8\n");
	top = max(FONTMAXBOUNDS(pGC->font, ascent), FONTASCENT(pGC->font));
	bot = max(FONTMAXBOUNDS(pGC->font, descent), FONTDESCENT(pGC->font));

	Min = count * FONTMINBOUNDS(pGC->font, characterWidth);
	if(Min > 0) Min = 0;
	Max = count * FONTMAXBOUNDS(pGC->font, characterWidth);	
	if(Max < 0) Max = 0;

	/* ugh */
	box.x1 = pDraw->x + x + Min +
		FONTMINBOUNDS(pGC->font, leftSideBearing);
	box.x2 = pDraw->x + x + Max + 
		FONTMAXBOUNDS(pGC->font, rightSideBearing);

	box.y1 = pDraw->y + y - top;
	box.y2 = pDraw->y + y + bot;

	ADD_BOX(box, pGC);
    }
}

static void
xxImageText16(
    DrawablePtr pDraw,
    GCPtr	pGC,
    int		x,
    int		y,
    int 	count,
    unsigned short *chars 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->ImageText16)(pDraw, pGC, x, y, count, chars);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && count) {
	int top, bot, Min, Max;
	BoxRec box;

	DBG("ImageText16\n");
	top = max(FONTMAXBOUNDS(pGC->font, ascent), FONTASCENT(pGC->font));
	bot = max(FONTMAXBOUNDS(pGC->font, descent), FONTDESCENT(pGC->font));

	Min = count * FONTMINBOUNDS(pGC->font, characterWidth);
	if(Min > 0) Min = 0;
	Max = count * FONTMAXBOUNDS(pGC->font, characterWidth);	
	if(Max < 0) Max = 0;

	/* ugh */
	box.x1 = pDraw->x + x + Min +
		FONTMINBOUNDS(pGC->font, leftSideBearing);
	box.x2 = pDraw->x + x + Max + 
		FONTMAXBOUNDS(pGC->font, rightSideBearing);

	box.y1 = pDraw->y + y - top;
	box.y2 = pDraw->y + y + bot;

	ADD_BOX(box, pGC);
    }
}

static void
xxImageGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int x, int y,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->ImageGlyphBlt)(pDraw, pGC, x, y, nglyph, 
					ppci, pglyphBase);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && nglyph) {
	int top, bot, width = 0;
	BoxRec box;

	DBG("ImageGlyphBlt\n");
	top = max(FONTMAXBOUNDS(pGC->font, ascent), FONTASCENT(pGC->font));
	bot = max(FONTMAXBOUNDS(pGC->font, descent), FONTDESCENT(pGC->font));

	box.x1 = ppci[0]->metrics.leftSideBearing;
	if(box.x1 > 0) box.x1 = 0;
	box.x2 = ppci[nglyph - 1]->metrics.rightSideBearing - 
		ppci[nglyph - 1]->metrics.characterWidth;
	if(box.x2 < 0) box.x2 = 0;

	box.x2 += pDraw->x + x;
	box.x1 += pDraw->x + x;
	   
	while(nglyph--) {
	    width += (*ppci)->metrics.characterWidth;
	    ppci++;
	}

	if(width > 0) 
	   box.x2 += width;
	else 
	   box.x1 += width;

	box.y1 = pDraw->y + y - top;
	box.y2 = pDraw->y + y + bot;

	ADD_BOX(box, pGC);
    }
}

static void
xxPolyGlyphBlt(
    DrawablePtr pDraw,
    GCPtr pGC,
    int x, int y,
    unsigned int nglyph,
    CharInfoPtr *ppci,
    pointer pglyphBase 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->PolyGlyphBlt)(pDraw, pGC, x, y, nglyph, 
				ppci, pglyphBase);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw) && nglyph) {
	BoxRec box;

	DBG("PolyGlyphBlt\n");
	/* ugh */
	box.x1 = pDraw->x + x + ppci[0]->metrics.leftSideBearing;
	box.x2 = pDraw->x + x + ppci[nglyph - 1]->metrics.rightSideBearing;

	if(nglyph > 1) {
	    int width = 0;

	    while(--nglyph) { 
		width += (*ppci)->metrics.characterWidth;
		ppci++;
	    }
	
	    if(width > 0) box.x2 += width;
	    else box.x1 += width;
	}

	box.y1 = pDraw->y + y - FONTMAXBOUNDS(pGC->font, ascent);
	box.y2 = pDraw->y + y + FONTMAXBOUNDS(pGC->font, descent);

	ADD_BOX(box, pGC);
    }
}

static void
xxPushPixels(
    GCPtr	pGC,
    PixmapPtr	pBitMap,
    DrawablePtr pDraw,
    int	dx, int dy, int xOrg, int yOrg 
){
    XX_GC_OP_PROLOGUE(pGC, pDraw);
    (*pGC->ops->PushPixels)(pGC, pBitMap, pDraw, dx, dy, xOrg, yOrg);
    XX_GC_OP_EPILOGUE(pGC, pDraw);

    if(IS_VISIBLE(pDraw)) {
	BoxRec box;

	DBG("PushPixels\n");
	box.x1 = xOrg + pDraw->x;
	box.x2 = box.x1 + dx;
	box.y1 = yOrg + pDraw->y;
	box.y2 = box.y1 + dy;

	ADD_BOX(box, pGC);
    }
}


#ifdef RENDER
#define RENDER_MAKE_BOX(pDrawable,X,Y,W,H) { \
    box.x1 = X + pDrawable->x; \
    box.x2 = X + pDrawable->x + W; \
    box.y1 = Y + pDrawable->y; \
    box.y2 = Y + pDrawable->y + H; \
}

#define RENDER_ADD_BOX(pScreen,box) {\
    if (BOX_NOT_EMPTY(box)) { \
       RegionRec region; \
       xxScrPriv(pScreen);\
       ScreenPtr pScreen = pScreen;\
       REGION_INIT (pScreen, &region, &box, 1); \
       PRINT_RECTS(pScrPriv->region);\
       REGION_UNION(pScreen,&pScrPriv->region,&pScrPriv->region,&region);\
       PRINT_RECTS(pScrPriv->region);\
       REGION_UNINIT(pScreen,&region);\
   }\
}

static void
xxComposite (CARD8 op, PicturePtr pSrc, PicturePtr pMask, PicturePtr pDst,
	     INT16 xSrc, INT16 ySrc, INT16 xMask, INT16 yMask,
    INT16 xDst, INT16 yDst, CARD16 width, CARD16 height)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    xxScrPriv(pScreen);
    BoxRec		box;

    unwrap (pScrPriv, ps, Composite);
    (*ps->Composite) (op, pSrc, pMask, pDst, xSrc, ySrc, xMask, yMask,
		      xDst, yDst, width, height);
    wrap (pScrPriv, ps, Composite, xxComposite);
    if (pDst->pDrawable->type == DRAWABLE_WINDOW) {
	RENDER_MAKE_BOX(pDst->pDrawable, xDst, yDst, width, height);
	RENDER_ADD_BOX(pScreen,box);
    }
}


static void
xxGlyphs (CARD8 op, PicturePtr pSrc, PicturePtr pDst,
	  PictFormatPtr maskFormat, INT16 xSrc, INT16 ySrc, int nlist,
	  GlyphListPtr list, GlyphPtr *glyphs)
{
    ScreenPtr		pScreen = pDst->pDrawable->pScreen;
    PictureScreenPtr	ps = GetPictureScreen(pScreen);
    xxScrPriv(pScreen);
    int			x, y;
    int			n;
    GlyphPtr		glyph;
    BoxRec		box;

    unwrap (pScrPriv, ps, Glyphs);
    (*ps->Glyphs) (op, pSrc, pDst, maskFormat, xSrc, ySrc,
		   nlist, list, glyphs);
    wrap (pScrPriv, ps, Glyphs, xxGlyphs);
    if (pDst->pDrawable->type == DRAWABLE_WINDOW)
    {
	x = xSrc;
	y = ySrc;
	while (nlist--)
	{
	    x += list->xOff;
	    y += list->yOff;
	    n = list->len;
	    while (n--)
	    {
		glyph = *glyphs++;
		RENDER_MAKE_BOX(pDst->pDrawable,
				x - glyph->info.x, y - glyph->info.y,
				glyph->info.width, glyph->info.height);
		RENDER_ADD_BOX(pScreen,box);
		x += glyph->info.xOff;
		y += glyph->info.yOff;
	    }
	    list++;
	}
    }
}
#endif

void
xxPrintVisuals(void)
{
    int k,i,j;
    DepthPtr pDepth;
    VisualPtr pVisual;

    for (k = 0; k < screenInfo.numScreens; k++) {
	ScreenPtr pScreen = screenInfo.screens[k];
	
	pDepth = pScreen->allowedDepths;
	for (i = 0; i < pScreen->numDepths; i++, pDepth++)
	    for (j = 0; j < pDepth->numVids; j++) {
		ErrorF("depth: %i vid: 0x%lx\n",
		       pDepth->depth, pDepth->vids[j]);
	    }
	
	pVisual = pScreen->visuals;
	for (i = 0; i < pScreen->numVisuals; i++, pVisual++)
	    ErrorF("vid: 0x%x rm: 0x%lx gm: 0x%lx bm: 0x%lx\n",
		   (unsigned int)pVisual->vid,
		   pVisual->redMask,
		   pVisual->greenMask,
		   pVisual->blueMask);
    }
}


