/*
 * This code is largely copied from fbglyph.c.
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
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"
#include "rlAccel.h"

      
void
rlImageGlyphBlt (DrawablePtr	pDrawable,
		 GCPtr		pGC,
		 int		x, 
		 int		y,
		 unsigned int	nglyph,
		 CharInfoPtr	*ppciInit,
		 pointer	pglyphBase)
{
    FbGCPrivPtr	    pPriv = fbGetGCPrivate(pGC);
    CharInfoPtr	    *ppci;
    CharInfoPtr	    pci;
    unsigned char   *pglyph;		/* pointer bits in glyph */
    int		    gWidth, gHeight;	/* width and height of glyph */
    FbStride	    gStride;		/* stride of glyph */
    Bool	    opaque;
    int		    n;
    int		    gx, gy;
#ifndef FBNOPIXADDR
    void	    (*glyph) (FbBits *,
			      FbStride,
			      int,
			      FbStip *,
			      FbBits,
			      int,
			      int);
    FbBits	    *dst = 0;
    FbStride	    dstStride = 0;
    int		    dstBpp = 0;
    int		    dstXoff = 0, dstYoff = 0;
    
    glyph = 0;
    if (pPriv->and == 0)
    {
	fbGetDrawable (pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
	switch (dstBpp) {
	case 8:	    glyph = fbGlyph8; break;
	case 16:    glyph = fbGlyph16; break;
#ifdef FB_24BIT
	case 24:    glyph = fbGlyph24; break;
#endif
	case 32:    glyph = fbGlyph32; break;
	}
    }
#endif
    
    x += pDrawable->x;
    y += pDrawable->y;

    if (TERMINALFONT (pGC->font)
#ifndef FBNOPIXADDR
	&& !glyph
#endif
	)
    {
	opaque = TRUE;
    }
    else
    {
	int		xBack, widthBack;
	int		yBack, heightBack;
	
	ppci = ppciInit;
	n = nglyph;
	widthBack = 0;
	while (n--)
	    widthBack += (*ppci++)->metrics.characterWidth;
	
        xBack = x;
	if (widthBack < 0)
	{
	    xBack += widthBack;
	    widthBack = -widthBack;
	}
	yBack = y - FONTASCENT(pGC->font);
	heightBack = FONTASCENT(pGC->font) + FONTDESCENT(pGC->font);
	rlSolidBoxClipped (pDrawable,
			   fbGetCompositeClip(pGC),
			   xBack,
			   yBack,
			   xBack + widthBack,
			   yBack + heightBack,
			   fbAnd(GXcopy,pPriv->bg,pPriv->pm),
			   fbXor(GXcopy,pPriv->bg,pPriv->pm));
	opaque = FALSE;
    }

    ppci = ppciInit;
    while (nglyph--)
    {
	pci = *ppci++;
	pglyph = FONTGLYPHBITS(pglyphBase, pci);
	gWidth = GLYPHWIDTHPIXELS(pci);
	gHeight = GLYPHHEIGHTPIXELS(pci);
	if (gWidth && gHeight)
	{
	    gx = x + pci->metrics.leftSideBearing;
	    gy = y - pci->metrics.ascent; 
#ifndef FBNOPIXADDR
	    if (glyph && gWidth <= sizeof (FbStip) * 8 &&
		fbGlyphIn (fbGetCompositeClip(pGC), gx, gy, gWidth, gHeight))
	    {
		(*glyph) (dst + (gy + dstYoff) * dstStride,
			  dstStride,
			  dstBpp,
			  (FbStip *) pglyph,
			  pPriv->fg,
			  gx + dstXoff,
			  gHeight);
	    }
	    else
#endif
	    {
		gStride = GLYPHWIDTHBYTESPADDED(pci) / sizeof (FbStip);
		fbPutXYImage (pDrawable,
			      fbGetCompositeClip(pGC),
			      pPriv->fg,
			      pPriv->bg,
			      pPriv->pm,
			      GXcopy,
			      opaque,
    
			      gx,
			      gy,
			      gWidth, gHeight,
    
			      (FbStip *) pglyph,
			      gStride,
			      0);
	    }
	}
	x += pci->metrics.characterWidth;
    }
}
