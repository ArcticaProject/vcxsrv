#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdlib.h>

#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xproto.h>
#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb32.h"
#include "cfb8_32.h"
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mistruct.h"
#include "mibstore.h"
#include "migc.h"


static void cfb8_32ValidateGC(GCPtr, unsigned long, DrawablePtr);
static void cfb8_32DestroyGC(GCPtr pGC);
static void cfb32DestroyGC_Underlay(GCPtr pGC);

static
GCFuncs cfb8_32GCFuncs = {
    cfb8_32ValidateGC,
    miChangeGC,
    miCopyGC,
    cfb8_32DestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip,
};


static
GCFuncs cfb32GCFuncs_Underlay = {
    cfb32ValidateGC_Underlay,
    miChangeGC,
    miCopyGC,
    cfb32DestroyGC_Underlay,
    miChangeClip,
    miDestroyClip,
    miCopyClip,
};

static void
cfb32DestroyGC_Underlay(GCPtr pGC)
{
    if (pGC->freeCompClip)
        REGION_DESTROY(pGC->pScreen, pGC->pCompositeClip);

    if(pGC->ops)
	miDestroyGCOps(pGC->ops);
}


static void
cfb8_32DestroyGC(GCPtr pGC)
{
    cfb8_32GCPtr pGCPriv = CFB8_32_GET_GC_PRIVATE(pGC);

    if (pGC->freeCompClip)
        REGION_DESTROY(pGC->pScreen, pGC->pCompositeClip);
    if(pGCPriv->Ops8bpp)
	miDestroyGCOps(pGCPriv->Ops8bpp);
    if(pGCPriv->Ops32bpp)
	miDestroyGCOps(pGCPriv->Ops32bpp);
}

Bool
cfb8_32CreateGC(GCPtr pGC)
{
    cfb8_32GCPtr pGCPriv;
    cfbPrivGC *pPriv;

    if (PixmapWidthPaddingInfo[pGC->depth].padPixelsLog2 == LOG2_BITMAP_PAD)
        return (mfbCreateGC(pGC));

    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;
    pGC->miTranslate = 1;
    pGC->fExpose = TRUE;
    pGC->freeCompClip = FALSE;
    pGC->pRotatedPixmap = (PixmapPtr) NULL;

    pPriv = cfbGetGCPrivate(pGC);
    pPriv->rop = pGC->alu;
    pPriv->oneRect = FALSE;

    pGC->ops = NULL;

    if (pGC->depth == 8) {
	pGC->funcs = &cfb8_32GCFuncs;

	pGCPriv = CFB8_32_GET_GC_PRIVATE(pGC);
	pGCPriv->Ops8bpp = NULL;
	pGCPriv->Ops32bpp = NULL;
	pGCPriv->OpsAre8bpp = FALSE;
	pGCPriv->changes = 0;
    } else
	pGC->funcs = &cfb32GCFuncs_Underlay;
	
    return TRUE;
}


static void
cfb8_32ValidateGC(
    GCPtr pGC,
    unsigned long changes,
    DrawablePtr pDraw
){
    cfb8_32GCPtr pGCPriv = CFB8_32_GET_GC_PRIVATE(pGC);

    if(pDraw->bitsPerPixel == 32) {
	if(pGCPriv->OpsAre8bpp) {
	    int origChanges = changes;
	    pGC->ops = pGCPriv->Ops32bpp;
	    changes |= pGCPriv->changes;
	    pGCPriv->changes = origChanges;
	    pGCPriv->OpsAre8bpp = FALSE;
	} else 
	    pGCPriv->changes |= changes;

	cfb8_32ValidateGC32(pGC, changes, pDraw);
	pGCPriv->Ops32bpp = pGC->ops;
    } else {  /* bitsPerPixel == 8 */
	if(!pGCPriv->OpsAre8bpp) {
	    int origChanges = changes;
	    pGC->ops = pGCPriv->Ops8bpp;
	    changes |= pGCPriv->changes;
	    pGCPriv->changes = origChanges;
	    pGCPriv->OpsAre8bpp = TRUE;
	} else 
	    pGCPriv->changes |= changes;

	cfb8_32ValidateGC8(pGC, changes, pDraw);
	pGCPriv->Ops8bpp = pGC->ops;
    }
}

