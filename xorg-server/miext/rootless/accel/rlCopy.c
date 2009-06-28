/*
 * This code is largely copied from fbcopy.c.
 *
 * Copyright © 1998 Keith Packard
 * Copyright (c) 2003 Torrey T. Lyons. All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "fb.h"
#include "rlAccel.h"


void
rlCopyNtoN (DrawablePtr	pSrcDrawable,
	    DrawablePtr	pDstDrawable,
	    GCPtr	pGC,
	    BoxPtr	pbox,
	    int		nbox,
	    int		dx,
	    int		dy,
	    Bool	reverse,
	    Bool	upsidedown,
	    Pixel	bitplane,
	    void	*closure)
{
    CARD8	alu = pGC ? pGC->alu : GXcopy;
    FbBits	pm = pGC ? fbGetGCPrivate(pGC)->pm : FB_ALLONES;
    FbBits	*src;
    FbStride	srcStride;
    int		srcBpp;
    int		srcXoff, srcYoff;
    FbBits	*dst;
    FbStride	dstStride;
    int		dstBpp;
    int		dstXoff, dstYoff;
    
    fbGetDrawable (pSrcDrawable, src, srcStride, srcBpp, srcXoff, srcYoff);
    fbGetDrawable (pDstDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
    
    while (nbox--)
    {
	rlBlt (src + (pbox->y1 + dy + srcYoff) * srcStride,
	       srcStride,
	       (pbox->x1 + dx + srcXoff) * srcBpp,
    
	       pDstDrawable->pScreen,
               dst + (pbox->y1 + dstYoff) * dstStride,
	       dstStride,
	       (pbox->x1 + dstXoff) * dstBpp,
    
	       (pbox->x2 - pbox->x1) * dstBpp,
	       (pbox->y2 - pbox->y1),
    
	       alu,
	       pm,
	       dstBpp,
    
	       reverse,
	       upsidedown);
	pbox++;
    }
}

RegionPtr
rlCopyArea (DrawablePtr	pSrcDrawable,
	    DrawablePtr	pDstDrawable,
	    GCPtr	pGC,
	    int		xIn, 
	    int		yIn,
	    int		widthSrc, 
	    int		heightSrc,
	    int		xOut, 
	    int		yOut)
{
    fbCopyProc	copy;
    
#ifdef FB_24_32BIT
    if (pSrcDrawable->bitsPerPixel != pDstDrawable->bitsPerPixel)
	copy = fb24_32CopyMtoN;
    else
#endif
	copy = rlCopyNtoN;
    return fbDoCopy (pSrcDrawable, pDstDrawable, pGC, xIn, yIn,
		     widthSrc, heightSrc, xOut, yOut, copy, 0, 0);
}
