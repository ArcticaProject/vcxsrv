

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdlib.h>

#include <X11/X.h>
#include "scrnintstr.h"
#include "windowstr.h"
#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb32.h"
#include "cfb8_32.h"
#include "mistruct.h"
#include "regionstr.h"
#include "cfbmskbits.h"
#include "mioverlay.h"


/* We don't bother with cfb's fastBackground/Border so we don't
   need to use the Window privates */


Bool
cfb8_32CreateWindow(WindowPtr pWin)
{
    pWin->drawable.bitsPerPixel = 32;
    return TRUE;
}


Bool
cfb8_32DestroyWindow(WindowPtr pWin)
{
    return TRUE;
}

Bool
cfb8_32PositionWindow(
    WindowPtr pWin,
    int x, int y
){
    return TRUE;
}

void 
cfb8_32CopyWindow(pWin, ptOldOrg, prgnSrc)
    WindowPtr pWin;
    DDXPointRec ptOldOrg;
    RegionPtr prgnSrc;
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    DDXPointPtr ppt, pptSrc;
    RegionRec rgnDst;
    RegionPtr borderClip = &pWin->borderClip;
    BoxPtr pbox;
    int dx, dy, i, nbox;
    WindowPtr pwinRoot;
    Bool doUnderlay = miOverlayCopyUnderlay(pScreen);
    Bool freeReg = FALSE;

    pwinRoot = WindowTable[pScreen->myNum];

    if(doUnderlay)
	freeReg = miOverlayCollectUnderlayRegions(pWin, &borderClip);

    REGION_NULL(pScreen, &rgnDst);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    REGION_TRANSLATE(pScreen, prgnSrc, -dx, -dy);
    REGION_INTERSECT(pScreen, &rgnDst, borderClip, prgnSrc);

    pbox = REGION_RECTS(&rgnDst);
    nbox = REGION_NUM_RECTS(&rgnDst);
    if(!nbox || 
       !(pptSrc = (DDXPointPtr )xalloc(nbox * sizeof(DDXPointRec))))
    {
	REGION_UNINIT(pScreen, &rgnDst);
	return;
    }
    ppt = pptSrc;

    for (i = nbox; --i >= 0; ppt++, pbox++)
    {
	ppt->x = pbox->x1 + dx;
	ppt->y = pbox->y1 + dy;
    }

    if(doUnderlay)
	cfbDoBitblt24To24GXcopy((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
			GXcopy, &rgnDst, pptSrc, ~0);
    else
	cfbDoBitblt8To8GXcopy((DrawablePtr)pwinRoot, (DrawablePtr)pwinRoot,
			GXcopy, &rgnDst, pptSrc, ~0);

    xfree(pptSrc);
    REGION_UNINIT(pScreen, &rgnDst);
    if(freeReg) 
	REGION_DESTROY(pScreen, borderClip);
}

Bool
cfb8_32ChangeWindowAttributes(
    WindowPtr pWin,
    unsigned long mask
){
    return TRUE;
}




