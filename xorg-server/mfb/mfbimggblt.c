/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include	<X11/X.h>
#include	<X11/Xmd.h>
#include	<X11/Xproto.h>
#include	"mfb.h"
#include	<X11/fonts/fontstruct.h>
#include	"dixfontstr.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"maskbits.h"

/*
    we should eventually special-case fixed-width fonts for ImageText.

    this works for fonts with glyphs <= 32 bits wide.

    the clipping calculations are done for worst-case fonts.
we make no assumptions about the heights, widths, or bearings
of the glyphs.  if we knew that the glyphs are all the same height,
we could clip the tops and bottoms per clipping box, rather
than per character per clipping box.  if we knew that the glyphs'
left and right bearings were wlle-behaved, we could clip a single
character at the start, output until the last unclipped
character, and then clip the last one.  this is all straightforward
to determine based on max-bounds and min-bounds from the font.
    there is some inefficiency introduced in the per-character
clipping to make what's going on clearer.

    (it is possible, for example, for a font to be defined in which the
next-to-last character in a font would be clipped out, but the last
one wouldn't.  the code below deals with this.)

    Image text looks at the bits in the glyph and the fg and bg in the
GC.  it paints a rectangle, as defined in the protocol dcoument,
and the paints the characters.

   to avoid source proliferation, this file is compiled
three times:
	MFBIMAGEGLYPHBLT	OPEQ
	mfbImageGlyphBltWhite	|=
	mfbImageGlyphBltBlack	&=~

    the register allocations for startmask and endmask may not
be the right thing.  are there two other deserving candidates?
xoff, pdst, pglyph, and tmpSrc seem like the right things, though.
*/

void
MFBIMAGEGLYPHBLT(pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
    DrawablePtr pDrawable;
    GC 		*pGC;
    int 	x, y;
    unsigned int nglyph;
    CharInfoPtr *ppci;		/* array of character info */
    pointer	pglyphBase;	/* start of array of glyphs */
{
    ExtentInfoRec info;	/* used by QueryGlyphExtents() */
    BoxRec bbox;	/* string's bounding box */
    xRectangle backrect;/* backing rectangle to paint.
			   in the general case, NOT necessarily
			   the same as the string's bounding box
			*/

    CharInfoPtr pci;
    int xorg, yorg;	/* origin of drawable in bitmap */
    int widthDst;	/* width of dst in longwords */

			/* these keep track of the character origin */
    PixelType *pdstBase;
			/* points to longword with character origin */
    int xchar;		/* xorigin of char (mod 32) */

			/* these are used for placing the glyph */
    register int xoff;	/* x offset of left edge of glyph (mod 32) */
    register PixelType *pdst;
			/* pointer to current longword in dst */

    int w;		/* width of glyph in bits */
    int h;		/* height of glyph */
    int widthGlyph;	/* width of glyph, in bytes */
    register unsigned char *pglyph;
			/* pointer to current row of glyph */

			/* used for putting down glyph */    
    register PixelType tmpSrc;
			/* for getting bits from glyph */
    register PixelType startmask;
    register PixelType endmask;

    register int nFirst;/* bits of glyph in current longword */
    mfbPrivGC *pPrivGC;
    mfbFillAreaProcPtr oldFillArea;
			/* we might temporarily usurp this
			   field in devPriv */

    if (!(pGC->planemask & 1))
	return;

    xorg = pDrawable->x;
    yorg = pDrawable->y;
    mfbGetPixelWidthAndPointer(pDrawable, widthDst, pdstBase);

    QueryGlyphExtents(pGC->font, ppci, (unsigned long)nglyph, &info);

    backrect.x = x;
    backrect.y = y - FONTASCENT(pGC->font);
    backrect.width = info.overallWidth;
    backrect.height = FONTASCENT(pGC->font) + FONTDESCENT(pGC->font);

    x += xorg;
    y += yorg;

    bbox.x1 = x + info.overallLeft;
    bbox.x2 = x + info.overallRight;
    bbox.y1 = y - info.overallAscent;
    bbox.y2 = y + info.overallDescent;

    /* UNCLEAN CODE
       we know the mfbPolyFillRect uses only two fields in
       devPrivate[mfbGCPrivateIndex].ptr, one of which (ropFillArea) is
       irrelevant for solid filling, so we just poke the FillArea
       field.  the GC is now in an inconsistent state, but we'll fix
       it as soon as PolyFillRect returns.  fortunately, the server
       is single threaded.

    NOTE:
       if you are not using the standard mfbFillRectangle code, you
       need to poke any fields in the GC the rectangle stuff need
       (probably alu, fgPixel, and fillStyle) and in devPrivate[mfbGCPrivateIndex].ptr
       (probably rop or ropFillArea.)  You could just call ValidateGC,
       but that is usually not a cheap thing to do.
    */

    pPrivGC = (mfbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					    mfbGetGCPrivateKey());
    oldFillArea = pPrivGC->FillArea;

    if (pGC->bgPixel & 1)
        pPrivGC->FillArea = mfbSolidWhiteArea;
    else
        pPrivGC->FillArea = mfbSolidBlackArea;

    mfbPolyFillRect(pDrawable, pGC, 1, &backrect);
    pPrivGC->FillArea = oldFillArea;

    /* the faint-hearted can open their eyes now */
    switch (RECT_IN_REGION(pGC->pScreen, pGC->pCompositeClip, &bbox))
    {
      case rgnOUT:
	break;
      case rgnIN:
	pdstBase = mfbScanlineNoBankSwitch(pdstBase, x, y, widthDst);
        xchar = x & PIM;

        while(nglyph--)
        {
	    pci = *ppci;
	    pglyph = FONTGLYPHBITS(pglyphBase, pci);
	    w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
	    h = pci->metrics.ascent + pci->metrics.descent;
	    widthGlyph = GLYPHWIDTHBYTESPADDED(pci);

	    /* start at top scanline of glyph */
	    pdst = pdstBase;

	    /* find correct word in scanline and x offset within it
	       for left edge of glyph
	    */
	    xoff = xchar + pci->metrics.leftSideBearing;
	    if (xoff > PLST)
	    {
	        pdst++;
	        xoff &= PIM;
	    }
	    else if (xoff < 0)
	    {
	        xoff += PPW;
	        pdst--;
	    }

	    pdst = mfbScanlineDelta(pdst, -pci->metrics.ascent, widthDst);

	    if ((xoff + w) <= PPW)
	    {
	        /* glyph all in one longword */
	        maskpartialbits(xoff, w, startmask);
	        while (h--)
	        {
		    getleftbits(pglyph, w, tmpSrc);
		    *pdst OPEQ (SCRRIGHT(tmpSrc, xoff) & startmask);
		    pglyph += widthGlyph;
		    mfbScanlineInc(pdst, widthDst);
	        }
	    }
	    else
	    {
	        /* glyph crosses longword boundary */
	        maskPPWbits(xoff, w, startmask, endmask);
	        nFirst = PPW - xoff;
	        while (h--)
	        {
		    getleftbits(pglyph, w, tmpSrc);
		    *pdst OPEQ (SCRRIGHT(tmpSrc, xoff) & startmask);
		    *(pdst+1) OPEQ (SCRLEFT(tmpSrc, nFirst) & endmask);
		    pglyph += widthGlyph;
		    mfbScanlineInc(pdst, widthDst);
	        }
	    } /* glyph crosses longwords boundary */

	    /* update character origin */
	    x += pci->metrics.characterWidth;
	    xchar += pci->metrics.characterWidth;
	    if (xchar > PLST)
	    {
	        xchar -= PPW;
	        pdstBase++;
	    }
	    else if (xchar < 0)
	    {
	        xchar += PPW;
	        pdstBase--;
	    }
	    ppci++;
        } /* while nglyph-- */
	break;
      case rgnPART:
      {
	TEXTPOS *ppos;
	int nbox;
	BoxPtr pbox;
	RegionPtr cclip;
	int xpos;		/* x position of char origin */
	int i;
	BoxRec clip;
	int leftEdge, rightEdge;
	int topEdge, bottomEdge;
	int glyphRow;		/* first row of glyph not wholly
				   clipped out */
	int glyphCol;		/* leftmost visible column of glyph */
#if GETLEFTBITS_ALIGNMENT > 1
	int getWidth;		/* bits to get from glyph */
#endif

	if(!(ppos = (TEXTPOS *)xalloc(nglyph * sizeof(TEXTPOS))))
	    return;

	pdstBase = mfbScanlineNoBankSwitch(pdstBase, x, y, widthDst);
        xpos = x;
	xchar = xpos & PIM;

	for (i=0; i<nglyph; i++)
	{
	    pci = ppci[i];

	    ppos[i].xpos = xpos;
	    ppos[i].xchar = xchar;
	    ppos[i].leftEdge = xpos + pci->metrics.leftSideBearing;
	    ppos[i].rightEdge = xpos + pci->metrics.rightSideBearing;
	    ppos[i].topEdge = y - pci->metrics.ascent;
	    ppos[i].bottomEdge = y + pci->metrics.descent;
	    ppos[i].pdstBase = pdstBase;
	    ppos[i].widthGlyph = GLYPHWIDTHBYTESPADDED(pci);

	    xpos += pci->metrics.characterWidth;
	    xchar += pci->metrics.characterWidth;
	    if (xchar > PLST)
	    {
		xchar &= PIM;
		pdstBase++;
	    }
	    else if (xchar < 0)
	    {
		xchar += PPW;
		pdstBase--;
	    }
	}

	cclip = pGC->pCompositeClip;
	pbox = REGION_RECTS(cclip);
	nbox = REGION_NUM_RECTS(cclip);

	/* HACK ALERT
	   since we continue out of the loop below so often, it
	   is easier to increment pbox at the  top than at the end.
	   don't try this at home.
	*/
	pbox--;
	while(nbox--)
	{
	    pbox++;
	    clip.x1 = max(bbox.x1, pbox->x1);
	    clip.y1 = max(bbox.y1, pbox->y1);
	    clip.x2 = min(bbox.x2, pbox->x2);
	    clip.y2 = min(bbox.y2, pbox->y2);
	    if ((clip.x2<=clip.x1) || (clip.y2<=clip.y1))
		continue;

	    for(i=0; i<nglyph; i++)
	    {
		pci = ppci[i];
		xchar = ppos[i].xchar;

		/* clip the left and right edges */
		if (ppos[i].leftEdge < clip.x1)
		    leftEdge = clip.x1;
		else
		    leftEdge = ppos[i].leftEdge;

		if (ppos[i].rightEdge > clip.x2)
		    rightEdge = clip.x2;
		else
		    rightEdge = ppos[i].rightEdge;

		w = rightEdge - leftEdge;
		if (w <= 0)
		    continue;

		/* clip the top and bottom edges */
		if (ppos[i].topEdge < clip.y1)
		    topEdge = clip.y1;
		else
		    topEdge = ppos[i].topEdge;

		if (ppos[i].bottomEdge > clip.y2)
		    bottomEdge = clip.y2;
		else
		    bottomEdge = ppos[i].bottomEdge;

		h = bottomEdge - topEdge;
		if (h <= 0)
		    continue;

		glyphRow = (topEdge - y) + pci->metrics.ascent;
		widthGlyph = ppos[i].widthGlyph;
		pglyph = FONTGLYPHBITS(pglyphBase, pci);
		pglyph += (glyphRow * widthGlyph);

		pdst = ppos[i].pdstBase;

		glyphCol = (leftEdge - ppos[i].xpos) -
			   (pci->metrics.leftSideBearing);
#if GETLEFTBITS_ALIGNMENT > 1
		getWidth = w + glyphCol;
#endif
		xoff = xchar + (leftEdge - ppos[i].xpos);
		if (xoff > PLST)
		{
		    xoff &= PIM;
		    pdst++;
		}
		else if (xoff < 0)
		{
		    xoff += PPW;
		    pdst--;
		}

		pdst = mfbScanlineDelta(pdst, -(y-topEdge), widthDst);

		if ((xoff + w) <= PPW)
		{
		    maskpartialbits(xoff, w, startmask);
		    while (h--)
		    {
			getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
			*pdst OPEQ (SCRRIGHT(tmpSrc, xoff) & startmask);
			pglyph += widthGlyph;
			mfbScanlineInc(pdst, widthDst);
		    }
		}
		else
		{
		    maskPPWbits(xoff, w, startmask, endmask);
		    nFirst = PPW - xoff;
		    while (h--)
		    {
			getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
			*pdst OPEQ (SCRRIGHT(tmpSrc, xoff) & startmask);
			*(pdst+1) OPEQ (SCRLEFT(tmpSrc, nFirst) & endmask);
			pglyph += widthGlyph;
			mfbScanlineInc(pdst, widthDst);
		    }
		}
	    } /* for each glyph */
	} /* while nbox-- */
	xfree(ppos);
	break;
      }
      default:
	break;
    }
}
