
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb32.h"
#include "cfb8_32.h"
#include "servermd.h"
#include "mi.h"


void
cfb8_32GetImage (
    DrawablePtr pDraw,
    int sx, int sy, int w, int h,
    unsigned int format,
    unsigned long planemask,
    char *pdstLine
){
    if(!w || !h) return;

    if (!cfbDrawableEnabled (pDraw))
	    return;

    if(pDraw->depth == 24){
	cfb32GetImage(pDraw, sx, sy, w, h, format, planemask, pdstLine);
	return;
    }
    
    if((pDraw->bitsPerPixel == 8) || (pDraw->bitsPerPixel == 1)){
	cfbGetImage(pDraw, sx, sy, w, h, format, planemask, pdstLine);
	return;
    }

    /* source is depth 8, 32 bpp */
    if(format != ZPixmap) {
	miGetImage(pDraw, sx, sy, w, h, format, planemask, pdstLine);
	return;
    } else {
	BoxRec box;
	DDXPointRec ptSrc;
	RegionRec rgnDst;
	ScreenPtr pScreen;
	PixmapPtr pPixmap;

	pScreen = pDraw->pScreen;
        pPixmap = GetScratchPixmapHeader(pScreen, w, h, 8, 8,
                        PixmapBytePad(w,8), (pointer)pdstLine);
        if (!pPixmap)
            return;
        if ((planemask & 0xff) != 0xff)
            memset((char *)pdstLine, 0, pPixmap->devKind * h);
        ptSrc.x = sx + pDraw->x;
        ptSrc.y = sy + pDraw->y;
        box.x1 = 0;
        box.y1 = 0;
        box.x2 = w;
        box.y2 = h;
        REGION_INIT(pScreen, &rgnDst, &box, 1);
        cfbDoBitblt32To8(pDraw, (DrawablePtr)pPixmap, GXcopy, &rgnDst,
                    &ptSrc, planemask);
        REGION_UNINIT(pScreen, &rgnDst);
        FreeScratchPixmapHeader(pPixmap);
    }
}

void
cfb8_32PutImage (
    DrawablePtr pDraw,
    GCPtr pGC,
    int depth, 
    int x, int y, int w, int h,
    int leftPad,
    int format,
    char *pImage
){
    if(!w || !h) return;

    if((pDraw->bitsPerPixel == 8) || (format != XYPixmap)){
	cfbPutImage(pDraw, pGC, depth, x, y, w, h, leftPad, format, pImage);
	return;
    } else {  /* moving an 8bpp XYPixmap to a 32bpp screen */
        unsigned long   oldFg, oldBg;
        XID             gcv[3];
        unsigned long   oldPlanemask;
        unsigned long   i;
        long            bytesPer;

        oldPlanemask = pGC->planemask;
        oldFg = pGC->fgPixel;
        oldBg = pGC->bgPixel;
        gcv[0] = ~0L;
        gcv[1] = 0;
        DoChangeGC(pGC, GCForeground | GCBackground, gcv, 0);
        bytesPer = (long)h * BitmapBytePad(w + leftPad);

        for (i = 0x80000000; i & 0xff000000; i >>= 1, pImage += bytesPer)
        {
            if (i & oldPlanemask)
            {
                gcv[0] = i;
                DoChangeGC(pGC, GCPlaneMask, gcv, 0);
                ValidateGC(pDraw, pGC);
                (*pGC->ops->PutImage)(pDraw, pGC, 1, x, y, w, h, leftPad,
                                 XYBitmap, pImage);
            }
        }
        gcv[0] = oldPlanemask;
        gcv[1] = oldFg;
        gcv[2] = oldBg;
        DoChangeGC(pGC, GCPlaneMask | GCForeground | GCBackground, gcv, 0);
        ValidateGC(pDraw, pGC);
    }
}




void
cfb8_32GetSpans(
   DrawablePtr pDraw,
   int wMax,
   DDXPointPtr ppt,
   int *pwidth,
   int nspans,
   char *pDst
){
   int pitch, i;
   CARD8 *ptr, *ptrBase;

   if (!cfbDrawableEnabled (pDraw))
        return;

   if(pDraw->bitsPerPixel == 1) {
	mfbGetSpans(pDraw, wMax, ppt, pwidth, nspans, pDst);
	return;
   }

   if(pDraw->depth == 24) {
	cfb32GetSpans(pDraw, wMax, ppt, pwidth, nspans, pDst);
	return;
   } else if(pDraw->bitsPerPixel == 8) {
	cfbGetSpans(pDraw, wMax, ppt, pwidth, nspans, pDst);
	return;
   }

   /* gotta get spans from a depth 8 window */
   cfbGetByteWidthAndPointer(pDraw, pitch, ptrBase);
   ptrBase += 3;  /* point to top byte */

   while(nspans--) {
	ptr = ptrBase + (ppt->y * pitch) + (ppt->x << 2);
	
	for(i = *pwidth; i--; ptr += 4) 
	   *(pDst++) = *ptr;
	
	pDst = (char*)((long)(pDst + 3) & ~3L);

	ppt++; pwidth++;
   }
}


