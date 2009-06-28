
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "misc.h"
#include "xf86.h"
#include "xf86_OSproc.h"

#include <X11/X.h>
#include "scrnintstr.h"
#include "windowstr.h"
#include "xf86str.h"
#include "xaa.h"
#include "xaalocal.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "xaawrap.h"

/*
    Written by Harm Hanemaayer (H.Hanemaayer@inter.nl.net).
*/

void 
XAACopyWindow(
    WindowPtr pWin,
    DDXPointRec ptOldOrg,
    RegionPtr prgnSrc )
{
    DDXPointPtr pptSrc, ppt;
    RegionRec rgnDst;
    BoxPtr pbox;
    int dx, dy, nbox;
    WindowPtr pwinRoot;
    ScreenPtr pScreen = pWin->drawable.pScreen;
    XAAInfoRecPtr infoRec = 
	GET_XAAINFORECPTR_FROM_DRAWABLE((&pWin->drawable));

    if (!infoRec->pScrn->vtSema || !infoRec->ScreenToScreenBitBlt) { 
	XAA_SCREEN_PROLOGUE (pScreen, CopyWindow);
	if(infoRec->pScrn->vtSema && infoRec->NeedToSync) {
	    (*infoRec->Sync)(infoRec->pScrn);
	    infoRec->NeedToSync = FALSE;
	}
        (*pScreen->CopyWindow) (pWin, ptOldOrg, prgnSrc);
	XAA_SCREEN_EPILOGUE (pScreen, CopyWindow, XAACopyWindow);
    	return;
    }

    pwinRoot = WindowTable[pScreen->myNum];

    REGION_NULL(pScreen, &rgnDst);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    REGION_TRANSLATE(pScreen, prgnSrc, -dx, -dy);
    REGION_INTERSECT(pScreen, &rgnDst, &pWin->borderClip, prgnSrc);

    pbox = REGION_RECTS(&rgnDst);
    nbox = REGION_NUM_RECTS(&rgnDst);
    if(!nbox || 
      !(pptSrc = (DDXPointPtr )xalloc(nbox * sizeof(DDXPointRec)))) {
	REGION_UNINIT(pScreen, &rgnDst);
	return;
    }
    ppt = pptSrc;

    while(nbox--) {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
	ppt++; pbox++;
    }
    
    infoRec->ScratchGC.planemask = ~0L;
    infoRec->ScratchGC.alu = GXcopy;

    XAADoBitBlt((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
        		&(infoRec->ScratchGC), &rgnDst, pptSrc);

    xfree(pptSrc);
    REGION_UNINIT(pScreen, &rgnDst);
}
