/*
 * Accelerated rootless fill
 */
/*
 * This code is largely copied from fbsolid.c.
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

#include "fb.h"
#include "rootlessCommon.h"


void
rlSolid (ScreenPtr  pScreen,
         FbBits	    *dst,
	 FbStride   dstStride,
	 int	    dstX,
	 int	    bpp,

	 int	    width,
	 int	    height,

	 FbBits	    and,
	 FbBits	    xor)
{
    FbBits  startmask, endmask;
    int	    n, nmiddle;
    int	    startbyte, endbyte;

#ifdef FB_24BIT
    if (bpp == 24 && (!FbCheck24Pix(and) || !FbCheck24Pix(xor)))
    {
	fbSolid24 (dst, dstStride, dstX, width, height, and, xor);
	return;
    }
#endif
	
    dst += dstX >> FB_SHIFT;
    dstX &= FB_MASK;
    FbMaskBitsBytes(dstX, width, and == 0, startmask, startbyte, 
		    nmiddle, endmask, endbyte);

    /*
     * Beginning of the rootless acceleration code
     */
    if (!startmask && !endmask && !and &&
        height * nmiddle * sizeof (*dst) > rootless_FillBytes_threshold &&
        SCREENREC(pScreen)->imp->FillBytes)
    {
	if (bpp <= 8)
	    xor |= xor << 8;
	if (bpp <= 16)
	    xor |= xor << 16;

	SCREENREC(pScreen)->imp->FillBytes(nmiddle * sizeof (*dst), height,
                                           xor, (char *) dst + (dstX >> 3),
                                           dstStride * sizeof (*dst));
	return;
    }
    /* End of the rootless acceleration code */

    if (startmask)
	dstStride--;
    dstStride -= nmiddle;
    while (height--)
    {
	if (startmask)
	{
	    FbDoLeftMaskByteRRop(dst,startbyte,startmask,and,xor);
	    dst++;
	}
	n = nmiddle;
	if (!and)
	    while (n--)
		*dst++ = xor;
	else
	    while (n--)
	    {
		*dst = FbDoRRop (*dst, and, xor);
		dst++;
	    }
	if (endmask)
	    FbDoRightMaskByteRRop(dst,endbyte,endmask,and,xor);
	dst += dstStride;
    }
}
