
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdlib.h>

#include <X11/X.h>
#include <X11/Xmd.h>
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
#include "mistruct.h"
#include "dix.h"
#include "mibstore.h"


RegionPtr
cfb8_32CopyArea(
    DrawablePtr pSrcDraw,
    DrawablePtr pDstDraw,
    GC *pGC,
    int srcx, int srcy,
    int width, int height,
    int dstx, int dsty 
){

   if(pSrcDraw->bitsPerPixel == 32) {
	if(pDstDraw->bitsPerPixel == 32) {
	    if((pGC->alu == GXcopy) && (pGC->planemask == 0xff000000)) {
		return cfb32BitBlt (pSrcDraw, pDstDraw,
            		pGC, srcx, srcy, width, height, dstx, dsty, 
			cfbDoBitblt8To8GXcopy, 0L);
	    }
	    return(cfb32CopyArea(pSrcDraw, pDstDraw, pGC, srcx, srcy,
		    			width, height, dstx, dsty));
	} else {
	    /* have to translate 32 -> 8 copies */
	    return cfb32BitBlt (pSrcDraw, pDstDraw,
            		pGC, srcx, srcy, width, height, dstx, dsty, 
			cfbDoBitblt32To8, 0L);
	}
   } else {
	if(pDstDraw->bitsPerPixel == 32) {
	    /* have to translate 8 -> 32 copies */
	    return cfb32BitBlt (pSrcDraw, pDstDraw,
            		pGC, srcx, srcy, width, height, dstx, dsty, 
			cfbDoBitblt8To32, 0L);
	} else {
	    return(cfbCopyArea(pSrcDraw, pDstDraw, pGC, srcx, srcy,
		    width, height, dstx, dsty));
	}
   }
}




void 
cfbDoBitblt8To32(
    DrawablePtr pSrc, 
    DrawablePtr pDst, 
    int rop,
    RegionPtr prgnDst, 
    DDXPointPtr pptSrc,
    unsigned long pm
){
    BoxPtr pbox = REGION_RECTS(prgnDst);
    int nbox = REGION_NUM_RECTS(prgnDst);
    unsigned char *ptr8, *ptr32;
    unsigned char *data8, *data32;
    int pitch8, pitch32;
    int height, width, i;

    cfbGetByteWidthAndPointer(pSrc, pitch8, ptr8);
    cfbGetByteWidthAndPointer(pDst, pitch32, ptr32);
    ptr32 += 3; /* point to the top byte */

    pm >>= 24;

    if((pm == 0xff) && (rop == GXcopy)) {
	for(;nbox; pbox++, pptSrc++, nbox--) {
	    data8 = ptr8 + (pptSrc->y * pitch8) + pptSrc->x;
	    data32 = ptr32 + (pbox->y1 * pitch32) + (pbox->x1 << 2);
	    width = pbox->x2 - pbox->x1;
	    height = pbox->y2 - pbox->y1;

	    while(height--) {
		for(i = 0; i < width; i++)
		    data32[i << 2] = data8[i];
		data8 += pitch8;
		data32 += pitch32;
	    }
	}
    } else {  /* it ain't pretty, but hey */
	for(;nbox; pbox++, pptSrc++, nbox--) {
	    data8 = ptr8 + (pptSrc->y * pitch8) + pptSrc->x;
	    data32 = ptr32 + (pbox->y1 * pitch32) + (pbox->x1 << 2);
	    width = pbox->x2 - pbox->x1;
	    height = pbox->y2 - pbox->y1;

	    while(height--) {	
		switch(rop) {
		case GXcopy:
		    for(i = 0; i < width; i++)
			data32[i<<2] = (data8[i] & pm) | (data32[i<<2] & ~pm);
		    break;
		case GXor:
		    for(i = 0; i < width; i++)
			data32[i<<2] |= data8[i] & pm;
		    break;
		case GXclear:
		    for(i = 0; i < width; i++)
			data32[i<<2] &= ~pm;
		    break;
		case GXand:
		    for(i = 0; i < width; i++) 
			data32[i<<2] &= data8[i] | ~pm;
		    break;
		case GXandReverse:
		    for(i = 0; i < width; i++) 
			data32[i<<2] = ~data32[i<<2] & (data8[i] | ~pm);
		    break;
		case GXandInverted:
		    for(i = 0; i < width; i++) 
			data32[i<<2] &= ~data8[i] | ~pm;
		    break;
		case GXnoop:
		    return;
		case GXxor:
		    for(i = 0; i < width; i++) 
			data32[i<<2] ^= data8[i] & pm;
		    break;
		case GXnor:
		    for(i = 0; i < width; i++)
			data32[i<<2] =  ~(data32[i<<2] | (data8[i] & pm));
		    break;
		case GXequiv:
		    for(i = 0; i < width; i++)
			data32[i<<2] = ~(data32[i<<2] ^ (data8[i] & pm));
		    break;
		case GXinvert:
		    for(i = 0; i < width; i++)
			data32[i<<2] ^= pm;
		    break;
		case GXorReverse:
		    for(i = 0; i < width; i++)
			data32[i<<2] = ~data32[i<<2] | (data8[i] & pm);
		    break;
		case GXcopyInverted:
		    for(i = 0; i < width; i++)
			data32[i<<2] = (~data8[i] & pm) | (data32[i<<2] & ~pm);
		    break;
		case GXorInverted:
		    for(i = 0; i < width; i++)
			data32[i<<2] |= ~data8[i] & pm;
		    break;
		case GXnand:
		    for(i = 0; i < width; i++) 
			data32[i<<2] = ~(data32[i<<2] & (data8[i] | ~pm));
		    break;
		case GXset:
		    for(i = 0; i < width; i++)
			data32[i<<2] |= pm;
		    break;
		}
	        data8 += pitch8;
	        data32 += pitch32;
	    }
	}
    }
}


void 
cfbDoBitblt32To8(
    DrawablePtr pSrc, 
    DrawablePtr pDst, 
    int rop,
    RegionPtr prgnDst, 
    DDXPointPtr pptSrc,
    unsigned long pm
){
    BoxPtr pbox = REGION_RECTS(prgnDst);
    int nbox = REGION_NUM_RECTS(prgnDst);
    unsigned char *ptr8, *ptr32;
    unsigned char *data8, *data32;
    int pitch8, pitch32;
    int height, width, i;

    cfbGetByteWidthAndPointer(pDst, pitch8, ptr8);
    cfbGetByteWidthAndPointer(pSrc, pitch32, ptr32);
    ptr32 += 3; /* point to the top byte */

    if(((pm & 0xff) == 0xff) && (rop == GXcopy)) {
	for(;nbox; pbox++, pptSrc++, nbox--) {
	    data8 = ptr8 + (pbox->y1 * pitch8) + pbox->x1;
	    data32 = ptr32 + (pptSrc->y * pitch32) + (pptSrc->x << 2);

	    width = pbox->x2 - pbox->x1;
	    height = pbox->y2 - pbox->y1;

	    while(height--) {
		for(i = 0; i < width; i++)
		    data8[i] = data32[i << 2];
		data8 += pitch8;
		data32 += pitch32;
	    }
	}
    } else {
	for(;nbox; pbox++, pptSrc++, nbox--) {
	    data8 = ptr8 + (pbox->y1 * pitch8) + pbox->x1;
	    data32 = ptr32 + (pptSrc->y * pitch32) + (pptSrc->x << 2);

	    width = pbox->x2 - pbox->x1;
	    height = pbox->y2 - pbox->y1;

	    while(height--) {	
		switch(rop) {
		case GXcopy:
		    for(i = 0; i < width; i++)
			data8[i] = (data32[i<<2] & pm) | (data8[i] & ~pm);
		    break;
		case GXor:
		    for(i = 0; i < width; i++)
			data8[i] |= data32[i<<2] & pm;
		    break;
		case GXclear:
		    for(i = 0; i < width; i++)
			data8[i] &= ~pm;
		    break;
		case GXand:
		    for(i = 0; i < width; i++)
			data8[i] &= data32[i<<2] | ~pm;
		    break;
		case GXandReverse:
		    for(i = 0; i < width; i++)
			data8[i] = ~data8[i] & (data32[i<<2] | ~pm);
		    break;
		case GXandInverted:
		    for(i = 0; i < width; i++)
			data8[i] &= ~data32[i<<2] | ~pm;
		    break;
		case GXnoop:
		    return;
		case GXxor:
		    for(i = 0; i < width; i++) 
			data8[i] ^= data32[i<<2] & pm;
		    break;
		case GXnor:
		    for(i = 0; i < width; i++)
			data8[i] = ~(data8[i] | (data32[i<<2] & pm));
		    break;
		case GXequiv:
		    for(i = 0; i < width; i++)
			data8[i] = ~(data8[i] ^ (data32[i<<2] & pm));
		    break;
		case GXinvert:
		    for(i = 0; i < width; i++)
			data8[i] ^= pm;
		    break;
		case GXorReverse:
		    for(i = 0; i < width; i++)
			data8[i] = ~data8[i] | (data32[i<<2] & pm);
		    break;
		case GXcopyInverted:
		    for(i = 0; i < width; i++)
			data8[i] = (~data32[i<<2] & pm) | (data8[i] & ~pm);
		    break;
		case GXorInverted:
		    for(i = 0; i < width; i++)
			data8[i] |= ~data32[i<<2] & pm;
		    break;
		case GXnand:
		    for(i = 0; i < width; i++)
			data8[i] = ~(data8[i] & (data32[i<<2] | ~pm));
		    break;
		case GXset:
		    for(i = 0; i < width; i++)
			data8[i] |= pm;
		    break;
		}
	        data8 += pitch8;
	        data32 += pitch32;
	    }
	}
    }
}



static void 
Do8To8Blt(
   unsigned char *SrcPtr,
   int SrcPitch,
   unsigned char *DstPtr,
   int DstPitch,
   int nbox,
   DDXPointPtr pptSrc,
   BoxPtr pbox,
   int xdir, int ydir
){
   int i, j, width, height, ydir2;
   CARD8 *src, *dst;

   SrcPtr += 3;
   DstPtr += 3;
   xdir *= 4;
   ydir2 = ydir * DstPitch;
   ydir *= SrcPitch;

   for(;nbox; pbox++, pptSrc++, nbox--) {
	src = SrcPtr + (pptSrc->y * SrcPitch) + (pptSrc->x << 2);
	dst = DstPtr + (pbox->y1 * DstPitch) + (pbox->x1 << 2);
	width = pbox->x2 - pbox->x1;
	height = pbox->y2 - pbox->y1;

	if(ydir < 0) {
	    src += (height - 1) * SrcPitch;
	    dst += (height - 1) * DstPitch;
	}

	if(xdir < 0) {
	    register int tmp = (width - 1) << 2;
	    src += tmp;
	    dst += tmp;
	}

	while(height--) {
	   for(i = width, j = 0; i--; j+=xdir)
		dst[j] = src[j];
	   src += ydir;
	   dst += ydir2;
	}
    }
}

static void 
Do24To24Blt(
   unsigned char *SrcPtr,
   int SrcPitch,
   unsigned char *DstPtr,
   int DstPitch,
   int nbox,
   DDXPointPtr pptSrc,
   BoxPtr pbox,
   int xdir, int ydir
){
   int i, j, width, height, ydir2;
   CARD8 *src, *dst;

   xdir *= 4;
   ydir2 = ydir * DstPitch;
   ydir *= SrcPitch;

   for(;nbox; pbox++, pptSrc++, nbox--) {
	src = SrcPtr + (pptSrc->y * SrcPitch) + (pptSrc->x << 2);
	dst = DstPtr + (pbox->y1 * DstPitch) + (pbox->x1 << 2);
	width = pbox->x2 - pbox->x1;
	height = pbox->y2 - pbox->y1;

	if(ydir < 0) {
	    src += (height - 1) * SrcPitch;
	    dst += (height - 1) * DstPitch;
	}

	if(xdir < 0) {
	    register int tmp = (width - 1) << 2;
	    src += tmp;
	    dst += tmp;
	}

	while(height--) {
	   for(i = width, j = 0; i--; j+=xdir) {
		*((CARD16*)(dst + j)) = *((CARD32*)(src + j));
		dst[j + 2] = src[j + 2];
	   }
	   src += ydir;
	   dst += ydir2;
	}
    }
}


static void
cfb8_32DoBitBlt(
    DrawablePtr	    pSrc, 
    DrawablePtr	    pDst,
    RegionPtr	    prgnDst,
    DDXPointPtr	    pptSrc,
    void	    (*DoBlt)(
        unsigned char *SrcPtr,
        int SrcPitch,
        unsigned char *DstPtr,
        int DstPitch,
        int nbox,
        DDXPointPtr pptSrc,
        BoxPtr pbox,
        int xdir, int ydir)
){
    int nbox, careful, SrcPitch, DstPitch;
    BoxPtr pbox, pboxTmp, pboxNext, pboxBase, pboxNew1, pboxNew2;
    DDXPointPtr pptTmp, pptNew1, pptNew2;
    int xdir, ydir;
    unsigned char *SrcPtr, *DstPtr;

    /* XXX we have to err on the side of safety when both are windows,
     * because we don't know if IncludeInferiors is being used.
     */
    careful = ((pSrc == pDst) ||
               ((pSrc->type == DRAWABLE_WINDOW) &&
                (pDst->type == DRAWABLE_WINDOW)));

    pbox = REGION_RECTS(prgnDst);
    nbox = REGION_NUM_RECTS(prgnDst);

    pboxNew1 = NULL;
    pptNew1 = NULL;
    pboxNew2 = NULL;
    pptNew2 = NULL;
    if (careful && (pptSrc->y < pbox->y1)) {
        /* walk source botttom to top */
	ydir = -1;

	if (nbox > 1) {
	    /* keep ordering in each band, reverse order of bands */
	    pboxNew1 = (BoxPtr)xalloc(sizeof(BoxRec) * nbox);
	    if(!pboxNew1)
		return;
	    pptNew1 = (DDXPointPtr)xalloc(sizeof(DDXPointRec) * nbox);
	    if(!pptNew1) {
	        xfree(pboxNew1);
	        return;
	    }
	    pboxBase = pboxNext = pbox+nbox-1;
	    while (pboxBase >= pbox) {
	        while ((pboxNext >= pbox) &&
		       (pboxBase->y1 == pboxNext->y1))
		    pboxNext--;
	        pboxTmp = pboxNext+1;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp <= pboxBase) {
		    *pboxNew1++ = *pboxTmp++;
		    *pptNew1++ = *pptTmp++;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNew1 -= nbox;
	    pbox = pboxNew1;
	    pptNew1 -= nbox;
	    pptSrc = pptNew1;
        }
    } else {
	/* walk source top to bottom */
	ydir = 1;
    }

    if (careful && (pptSrc->x < pbox->x1)) {
	/* walk source right to left */
        xdir = -1;

	if (nbox > 1) {
	    /* reverse order of rects in each band */
	    pboxNew2 = (BoxPtr)xalloc(sizeof(BoxRec) * nbox);
	    pptNew2 = (DDXPointPtr)xalloc(sizeof(DDXPointRec) * nbox);
	    if(!pboxNew2 || !pptNew2) {
		if (pptNew2) xfree(pptNew2);
		if (pboxNew2) xfree(pboxNew2);
		if (pboxNew1) {
		    xfree(pptNew1);
		    xfree(pboxNew1);
		}
	        return;
	    }
	    pboxBase = pboxNext = pbox;
	    while (pboxBase < pbox+nbox) {
	        while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++;
	        pboxTmp = pboxNext;
	        pptTmp = pptSrc + (pboxTmp - pbox);
	        while (pboxTmp != pboxBase) {
		    *pboxNew2++ = *--pboxTmp;
		    *pptNew2++ = *--pptTmp;
	        }
	        pboxBase = pboxNext;
	    }
	    pboxNew2 -= nbox;
	    pbox = pboxNew2;
	    pptNew2 -= nbox;
	    pptSrc = pptNew2;
	}
    } else {
	/* walk source left to right */
        xdir = 1;
    }

    cfbGetByteWidthAndPointer(pSrc, SrcPitch, SrcPtr);
    cfbGetByteWidthAndPointer(pDst, DstPitch, DstPtr);

    (*DoBlt)(SrcPtr,SrcPitch,DstPtr,DstPitch,nbox,pptSrc,pbox,xdir,ydir);
 
    if (pboxNew2) {
	xfree(pptNew2);
	xfree(pboxNew2);
    }
    if (pboxNew1) {
	xfree(pptNew1);
	xfree(pboxNew1);
    }

}


/* A couple routines to speed up full planemask copies */

void 
cfbDoBitblt8To8GXcopy(
    DrawablePtr pSrc, 
    DrawablePtr pDst, 
    int rop,
    RegionPtr prgnDst, 
    DDXPointPtr pptSrc,
    unsigned long pm
){
    cfb8_32DoBitBlt(pSrc, pDst, prgnDst, pptSrc, Do8To8Blt);
}


void 
cfbDoBitblt24To24GXcopy(
    DrawablePtr pSrc, 
    DrawablePtr pDst, 
    int rop,
    RegionPtr prgnDst, 
    DDXPointPtr pptSrc,
    unsigned long pm
){
    cfb8_32DoBitBlt(pSrc, pDst, prgnDst, pptSrc, Do24To24Blt);
}
