/*
 * Accelerated rootless blit
 */
/*
 * This code is largely copied from fbBlt.c.
 *
 * Copyright © 1998 Keith Packard
 * Copyright (c) 2002 Apple Computer, Inc. All Rights Reserved.
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

#include <stddef.h> /* For NULL */
#include <string.h>
#include "fb.h"
#include "rootlessCommon.h"
#include "rlAccel.h"

#define InitializeShifts(sx,dx,ls,rs) { \
    if (sx != dx) { \
	if (sx > dx) { \
	    ls = sx - dx; \
	    rs = FB_UNIT - ls; \
	} else { \
	    rs = dx - sx; \
	    ls = FB_UNIT - rs; \
	} \
    } \
}

void
rlBlt (FbBits   *srcLine,
       FbStride	srcStride,
       int	srcX,

       ScreenPtr pDstScreen,
       FbBits   *dstLine,
       FbStride dstStride,
       int	dstX,

       int	width,
       int	height,

       int	alu,
       FbBits	pm,
       int	bpp,

       Bool	reverse,
       Bool	upsidedown)
{
    FbBits  *src, *dst;
    int	    leftShift, rightShift;
    FbBits  startmask, endmask;
    FbBits  bits, bits1;
    int	    n, nmiddle;
    Bool    destInvarient;
    int	    startbyte, endbyte;
    FbDeclareMergeRop ();

#ifdef FB_24BIT
    if (bpp == 24 && !FbCheck24Pix (pm))
    {
	fbBlt24 (srcLine, srcStride, srcX, dstLine, dstStride, dstX,
		 width, height, alu, pm, reverse, upsidedown);
	return;
    }
#endif

    if (alu == GXcopy && pm == FB_ALLONES && !reverse &&
            !(srcX & 7) && !(dstX & 7) && !(width & 7)) {
        int i;
        CARD8 *src = (CARD8 *) srcLine;
        CARD8 *dst = (CARD8 *) dstLine;
        
        srcStride *= sizeof(FbBits);
        dstStride *= sizeof(FbBits);
        width >>= 3;
        src += (srcX >> 3);
        dst += (dstX >> 3);

        if (!upsidedown)
            for (i = 0; i < height; i++)
                memcpy(dst + i * dstStride, src + i * srcStride, width);
        else
            for (i = height - 1; i >= 0; i--)
                memcpy(dst + i * dstStride, src + i * srcStride, width);

        return;
    }

    FbInitializeMergeRop(alu, pm);
    destInvarient = FbDestInvarientMergeRop();
    if (upsidedown)
    {
	srcLine += (height - 1) * (srcStride);
	dstLine += (height - 1) * (dstStride);
	srcStride = -srcStride;
	dstStride = -dstStride;
    }
    FbMaskBitsBytes (dstX, width, destInvarient, startmask, startbyte,
		     nmiddle, endmask, endbyte);

    /*
     * Beginning of the rootless acceleration code
     */
    if (!startmask && !endmask && alu == GXcopy &&
        height * nmiddle * sizeof(*dst) > rootless_CopyBytes_threshold)
    {
	if (pm == FB_ALLONES && SCREENREC(pDstScreen)->imp->CopyBytes)
	{
	    SCREENREC(pDstScreen)->imp->CopyBytes(
                            nmiddle * sizeof(*dst), height,
                            (char *) srcLine + (srcX >> 3),
                            srcStride * sizeof (*src),
                            (char *) dstLine + (dstX >> 3),
                            dstStride * sizeof (*dst));
	    return;
	}

	/* FIXME: the pm test here isn't super-wonderful - just because
	   we don't care about the top eight bits doesn't necessarily
	   mean we want them set to 255. But doing this does give a
	   factor of two performance improvement when copying from a
	   pixmap to a window, which is pretty common.. */

	else if (bpp == 32 && sizeof(FbBits) == 4 &&
                 pm == 0x00FFFFFFUL && !reverse &&
                 SCREENREC(pDstScreen)->imp->CompositePixels)
	{
	    /* need to copy XRGB to ARGB. */

	    void *src[2], *dest[2];
	    unsigned int src_rowbytes[2], dest_rowbytes[2];
            unsigned int fn;

	    src[0] = (char *) srcLine + (srcX >> 3);
	    src[1] = NULL;
	    src_rowbytes[0] = srcStride * sizeof(*src);
	    src_rowbytes[1] = 0;

	    dest[0] = (char *) dstLine + (dstX >> 3);
	    dest[1] = dest[0];
	    dest_rowbytes[0] = dstStride * sizeof(*dst);
	    dest_rowbytes[1] = dest_rowbytes[0];

	    fn = RL_COMPOSITE_FUNCTION(RL_COMPOSITE_SRC, RL_DEPTH_ARGB8888,
                                       RL_DEPTH_NIL, RL_DEPTH_ARGB8888);

            if (SCREENREC(pDstScreen)->imp->CompositePixels(
                                nmiddle, height,
                                fn, src, src_rowbytes,
                                NULL, 0, dest, dest_rowbytes) == Success)
            {
                return;
            }
	}
    }
    /* End of the rootless acceleration code */

    if (reverse)
    {
	srcLine += ((srcX + width - 1) >> FB_SHIFT) + 1;
	dstLine += ((dstX + width - 1) >> FB_SHIFT) + 1;
	srcX = (srcX + width - 1) & FB_MASK;
	dstX = (dstX + width - 1) & FB_MASK;
    }
    else
    {
	srcLine += srcX >> FB_SHIFT;
	dstLine += dstX >> FB_SHIFT;
	srcX &= FB_MASK;
	dstX &= FB_MASK;
    }
    if (srcX == dstX)
    {
	while (height--)
	{
	    src = srcLine;
	    srcLine += srcStride;
	    dst = dstLine;
	    dstLine += dstStride;
	    if (reverse)
	    {
		if (endmask)
		{
		    bits = *--src;
		    --dst;
		    FbDoRightMaskByteMergeRop(dst, bits, endbyte, endmask);
		}
		n = nmiddle;
		if (destInvarient)
		{
		    while (n--)
			*--dst = FbDoDestInvarientMergeRop(*--src);
		}
		else
		{
		    while (n--)
		    {
			bits = *--src;
			--dst;
			*dst = FbDoMergeRop (bits, *dst);
		    }
		}
		if (startmask)
		{
		    bits = *--src;
		    --dst;
		    FbDoLeftMaskByteMergeRop(dst, bits, startbyte, startmask);
		}
	    }
	    else
	    {
		if (startmask)
		{
		    bits = *src++;
		    FbDoLeftMaskByteMergeRop(dst, bits, startbyte, startmask);
		    dst++;
		}
		n = nmiddle;
		if (destInvarient)
		{
#if 0
		    /*
		     * This provides some speedup on screen->screen blts
		     * over the PCI bus, usually about 10%.  But fb
		     * isn't usually used for this operation...
		     */
		    if (_ca2 + 1 == 0 && _cx2 == 0)
		    {
			FbBits	t1, t2, t3, t4;
			while (n >= 4)
			{
			    t1 = *src++;
			    t2 = *src++;
			    t3 = *src++;
			    t4 = *src++;
			    *dst++ = t1;
			    *dst++ = t2;
			    *dst++ = t3;
			    *dst++ = t4;
			    n -= 4;
			}
		    }
#endif
		    while (n--)
			*dst++ = FbDoDestInvarientMergeRop(*src++);
		}
		else
		{
		    while (n--)
		    {
			bits = *src++;
			*dst = FbDoMergeRop (bits, *dst);
			dst++;
		    }
		}
		if (endmask)
		{
		    bits = *src;
		    FbDoRightMaskByteMergeRop(dst, bits, endbyte, endmask);
		}
	    }
	}
    }
    else
    {
	if (srcX > dstX)
	{
	    leftShift = srcX - dstX;
	    rightShift = FB_UNIT - leftShift;
	}
	else
	{
	    rightShift = dstX - srcX;
	    leftShift = FB_UNIT - rightShift;
	}
	while (height--)
	{
	    src = srcLine;
	    srcLine += srcStride;
	    dst = dstLine;
	    dstLine += dstStride;
	    
	    bits1 = 0;
	    if (reverse)
	    {
		if (srcX < dstX)
		    bits1 = *--src;
		if (endmask)
		{
		    bits = FbScrRight(bits1, rightShift);
		    if (FbScrRight(endmask, leftShift))
		    {
			bits1 = *--src;
			bits |= FbScrLeft(bits1, leftShift);
		    }
		    --dst;
		    FbDoRightMaskByteMergeRop(dst, bits, endbyte, endmask);
		}
		n = nmiddle;
		if (destInvarient)
		{
		    while (n--)
		    {
			bits = FbScrRight(bits1, rightShift);
			bits1 = *--src;
			bits |= FbScrLeft(bits1, leftShift);
			--dst;
			*dst = FbDoDestInvarientMergeRop(bits);
		    }
		}
		else
		{
		    while (n--)
		    {
			bits = FbScrRight(bits1, rightShift);
			bits1 = *--src;
			bits |= FbScrLeft(bits1, leftShift);
			--dst;
			*dst = FbDoMergeRop(bits, *dst);
		    }
		}
		if (startmask)
		{
		    bits = FbScrRight(bits1, rightShift);
		    if (FbScrRight(startmask, leftShift))
		    {
			bits1 = *--src;
			bits |= FbScrLeft(bits1, leftShift);
		    }
		    --dst;
		    FbDoLeftMaskByteMergeRop (dst, bits, startbyte, startmask);
		}
	    }
	    else
	    {
		if (srcX > dstX)
		    bits1 = *src++;
		if (startmask)
		{
		    bits = FbScrLeft(bits1, leftShift); 
		    if (FbScrLeft(startmask, rightShift))
		    {
			bits1 = *src++;
			bits |= FbScrRight(bits1, rightShift);
		    }
		    FbDoLeftMaskByteMergeRop (dst, bits, startbyte, startmask);
		    dst++;
		}
		n = nmiddle;
		if (destInvarient)
		{
		    while (n--)
		    {
			bits = FbScrLeft(bits1, leftShift);
			bits1 = *src++;
			bits |= FbScrRight(bits1, rightShift);
			*dst = FbDoDestInvarientMergeRop(bits);
			dst++;
		    }
		}
		else
		{
		    while (n--)
		    {
			bits = FbScrLeft(bits1, leftShift);
			bits1 = *src++;
			bits |= FbScrRight(bits1, rightShift);
			*dst = FbDoMergeRop(bits, *dst);
			dst++;
		    }
		}
		if (endmask)
		{
		    bits = FbScrLeft(bits1, leftShift);
		    if (FbScrLeft(endmask, rightShift))
		    {
			bits1 = *src;
			bits |= FbScrRight(bits1, rightShift);
		    }
		    FbDoRightMaskByteMergeRop (dst, bits, endbyte, endmask);
		}
	    }
	}
    }
}
