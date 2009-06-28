/*
 * Support for RENDER extension while protecting the alpha channel
 */
/*
 * Copyright (c) 2002-2003 Torrey T. Lyons. All Rights Reserved.
 * Copyright (c) 2002 Apple Computer, Inc. All Rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */
/* This file is largely based on fbcompose.c and fbpict.c, which contain
 * the following copyright:
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 */


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif
#ifdef RENDER

#include <stddef.h> /* For NULL */
#include "fb.h"
#include "picturestr.h"
#include "mipict.h"
#include "fbpict.h"
#include "safeAlpha.h"
#include "rootlessCommon.h"
# define mod(a,b)	((b) == 1 ? 0 : (a) >= 0 ? (a) % (b) : (b) - (-a) % (b))

/* Optimized version of fbCompositeSolidMask_nx8x8888 */
void
SafeAlphaCompositeSolidMask_nx8x8888(
    CARD8      op,
    PicturePtr pSrc,
    PicturePtr pMask,
    PicturePtr pDst,
    INT16      xSrc,
    INT16      ySrc,
    INT16      xMask,
    INT16      yMask,
    INT16      xDst,
    INT16      yDst,
    CARD16     width,
    CARD16     height)
{
    CARD32	src, srca;
    CARD32	*dstLine, *dst, d, dstMask;
    CARD8	*maskLine, *mask, m;
    FbStride	dstStride, maskStride;
    CARD16	w;

    fbComposeGetSolid(pSrc, src, pDst->format);

    dstMask = FbFullMask (pDst->pDrawable->depth);
    srca = src >> 24;
    if (src == 0)
	return;

    fbComposeGetStart (pDst, xDst, yDst, CARD32, dstStride, dstLine, 1);
    fbComposeGetStart (pMask, xMask, yMask, CARD8, maskStride, maskLine, 1);

    if (dstMask == FB_ALLONES && pDst->pDrawable->bitsPerPixel == 32 &&
        width * height > rootless_CompositePixels_threshold &&
        SCREENREC(pDst->pDrawable->pScreen)->imp->CompositePixels)
    {
	void *srcp[2], *destp[2];
	unsigned int dest_rowbytes[2];
	unsigned int fn;

	srcp[0] = &src; srcp[1] = &src;
	/* null rowbytes pointer means use first value as a constant */
	destp[0] = dstLine; destp[1] = dstLine;
	dest_rowbytes[0] = dstStride * 4; dest_rowbytes[1] = dest_rowbytes[0];
	fn = RL_COMPOSITE_FUNCTION(RL_COMPOSITE_OVER, RL_DEPTH_ARGB8888,
                                   RL_DEPTH_A8, RL_DEPTH_ARGB8888);

	if (SCREENREC(pDst->pDrawable->pScreen)->imp->CompositePixels(
                width, height, fn, srcp, NULL,
                maskLine, maskStride,
                destp, dest_rowbytes) == Success)
	{
	    return;
	}
    }

    while (height--)
    {
	dst = dstLine;
	dstLine += dstStride;
	mask = maskLine;
	maskLine += maskStride;
	w = width;

	while (w--)
	{
	    m = *mask++;
	    if (m == 0xff)
	    {
		if (srca == 0xff)
		    *dst = src & dstMask;
		else
		    *dst = fbOver (src, *dst) & dstMask;
	    }
	    else if (m)
	    {
		d = fbIn (src, m);
		*dst = fbOver (d, *dst) & dstMask;
	    }
	    dst++;
	}
    }
}

void
SafeAlphaComposite (CARD8           op,
                    PicturePtr      pSrc,
                    PicturePtr      pMask,
                    PicturePtr      pDst,
                    INT16           xSrc,
                    INT16           ySrc,
                    INT16           xMask,
                    INT16           yMask,
                    INT16           xDst,
                    INT16           yDst,
                    CARD16          width,
                    CARD16          height)
{
  if (!pSrc) {
    ErrorF("SafeAlphaComposite: pSrc must not be null!\n");
    return;
  }

  if (!pDst) {
    ErrorF("SafeAlphaComposite: pDst must not be null!\n");
    return;
  }
  
  int oldDepth = pDst->pDrawable->depth;
  int oldFormat = pDst->format;
    
  /*
   * We can use the more optimized fbpict code, but it sets bits above
   * the depth to zero. Temporarily adjust destination depth if needed.
   */
  if (pDst->pDrawable->type == DRAWABLE_WINDOW
        && pDst->pDrawable->depth == 24
      && pDst->pDrawable->bitsPerPixel == 32)
    {
      pDst->pDrawable->depth = 32;
    }
    
  /* For rootless preserve the alpha in x8r8g8b8 which really is
   * a8r8g8b8
   */
  if (oldFormat == PICT_x8r8g8b8)
    {
      pDst->format = PICT_a8r8g8b8;
    }
    
  if (pSrc->pDrawable && pMask && pMask->pDrawable &&
        !pSrc->transform && !pMask->transform &&
        !pSrc->alphaMap && !pMask->alphaMap &&
        !pMask->repeat && !pMask->componentAlpha && !pDst->alphaMap &&
        pMask->format == PICT_a8 &&
       pSrc->repeatType == RepeatNormal && 
        pSrc->pDrawable->width == 1 &&
        pSrc->pDrawable->height == 1 &&
      (pDst->format == PICT_a8r8g8b8 ||
         pDst->format == PICT_x8r8g8b8 ||
         pDst->format == PICT_a8b8g8r8 ||
       pDst->format == PICT_x8b8g8r8))
    {
      fbWalkCompositeRegion (op, pSrc, pMask, pDst,
			     xSrc, ySrc, xMask, yMask, xDst, yDst,
			     width, height,
			     TRUE /* srcRepeat */,
			     FALSE /* maskRepeat */,
			     SafeAlphaCompositeSolidMask_nx8x8888);
    }
  else
    {
      fbComposite (op, pSrc, pMask, pDst,
		   xSrc, ySrc, xMask, yMask, xDst, yDst, width, height);
    }

  pDst->pDrawable->depth = oldDepth;
  pDst->format = oldFormat;
}

#endif /* RENDER */
