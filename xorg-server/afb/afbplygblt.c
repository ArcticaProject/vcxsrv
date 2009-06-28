/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


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

#include		<X11/X.h>
#include		<X11/Xmd.h>
#include		<X11/Xproto.h>
#include		"afb.h"
#include		<X11/fonts/fontstruct.h>
#include		"dixfontstr.h"
#include		"gcstruct.h"
#include		"windowstr.h"
#include		"scrnintstr.h"
#include		"pixmapstr.h"
#include		"regionstr.h"
#include		"maskbits.h"

/*
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

*/

void
afbPolyGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase)
	DrawablePtr pDrawable;
	GC				*pGC;
	int		x, y;
	unsigned int nglyph;
	CharInfoPtr *ppci;			/* array of character info */
	pointer		pglyphBase;		/* start of array of glyphs */
{
	ExtentInfoRec info;			/* used by QueryGlyphExtents() */
	BoxRec bbox;					/* string's bounding box */

	CharInfoPtr pci;
	int xorg, yorg;				/* origin of drawable in bitmap */
	int widthDst;					/* width of dst in longwords */

										/* these keep track of the character origin */
	PixelType *pdstBase;
										/* points to longword with character origin */
	int xchar;						/* xorigin of char (mod 32) */

										/* these are used for placing the glyph */
	register int xoff;			/* x offset of left edge of glyph (mod 32) */
	register PixelType *pdst;	/* pointer to current longword in dst */

	register int d;
	int depthDst;
	int sizeDst;
	int hSave;
	int w;							/* width of glyph in bits */
	int h;							/* height of glyph */
	int widthGlyph;				/* width of glyph, in bytes */
	unsigned char *rrops;
	register unsigned char *pglyph;
										/* pointer to current row of glyph */
	unsigned char *pglyphSave;

										/* used for putting down glyph */
	register PixelType tmpSrc;
										/* for getting bits from glyph */
	register PixelType startmask;
	register PixelType endmask;

	register int nFirst;			/* bits of glyph in current longword */
	PixelType *pdstSave;

	xorg = pDrawable->x;
	yorg = pDrawable->y;
	afbGetPixelWidthSizeDepthAndPointer(pDrawable, widthDst, sizeDst, depthDst,
													 pdstBase);

	QueryGlyphExtents(pGC->font, ppci, (unsigned long)nglyph, &info);

	x += xorg;
	y += yorg;

	bbox.x1 = x + info.overallLeft;
	bbox.x2 = x + info.overallRight;
	bbox.y1 = y - info.overallAscent;
	bbox.y2 = y + info.overallDescent;

	rrops = ((afbPrivGCPtr)dixLookupPrivate(&pGC->devPrivates,
						afbGCPrivateKey))->rrops;
	switch (RECT_IN_REGION(pGC->pScreen, pGC->pCompositeClip, &bbox)) {
		case rgnOUT:
			break;
		case rgnIN:
			pdstBase = afbScanlineNoBankSwitch(pdstBase, x, y, widthDst);
			xchar = x & PIM;

			while(nglyph--) {
				pci = *ppci;
				pglyphSave = FONTGLYPHBITS(pglyphBase, pci);
				w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
				hSave = pci->metrics.ascent + pci->metrics.descent;
				widthGlyph = GLYPHWIDTHBYTESPADDED(pci);
				/* start at top scanline of glyph */
				pdstSave = afbScanlineDelta(pdstBase, -pci->metrics.ascent,
											widthDst);

				/* find correct word in scanline and x offset within it
				   for left edge of glyph
				*/
				xoff = xchar + pci->metrics.leftSideBearing;
				if (xoff > PLST) {
					pdstSave++;
					xoff &= PIM;
				} else if (xoff < 0) {
					xoff += PPW;
					pdstSave--;
				}

				for (d = 0; d < depthDst; d++) {
					h = hSave;
					pdst = pdstSave;
					pdstSave += sizeDst;	/* @@@ NEXT PLANE @@@ */
					pglyph = pglyphSave;

					if ((xoff + w) <= PPW) {
						/* glyph all in one longword */
						maskpartialbits(xoff, w, startmask);
						switch (rrops[d]) {
							case RROP_WHITE:
								while (h--) {
									getleftbits(pglyph, w, tmpSrc);
									*pdst |= (SCRRIGHT(tmpSrc, xoff) & startmask);
									pglyph += widthGlyph;
									afbScanlineInc(pdst, widthDst);
								}
								break;
							case RROP_BLACK:
								while (h--) {
									getleftbits(pglyph, w, tmpSrc);
									*pdst &= ~(SCRRIGHT(tmpSrc, xoff) & startmask);
									pglyph += widthGlyph;
									afbScanlineInc(pdst, widthDst);
								}
								break;
							case RROP_INVERT:
								while (h--) {
									getleftbits(pglyph, w, tmpSrc);
									*pdst ^= (SCRRIGHT(tmpSrc, xoff) & startmask);
									pglyph += widthGlyph;
									afbScanlineInc(pdst, widthDst);
								}
								break;
						}
					} else {
						/* glyph crosses longword boundary */
						maskPPWbits(xoff, w, startmask, endmask);
						nFirst = PPW - xoff;
						switch (rrops[d]) {
							case RROP_WHITE:
								while (h--) {
									getleftbits(pglyph, w, tmpSrc);
									*pdst |= (SCRRIGHT(tmpSrc, xoff) & startmask);
									*(pdst+1) |= (SCRLEFT(tmpSrc, nFirst) & endmask);
									pglyph += widthGlyph;
									afbScanlineInc(pdst, widthDst);
								}
								break;
							case RROP_BLACK:
								while (h--) {
									getleftbits(pglyph, w, tmpSrc);
									*pdst &= ~(SCRRIGHT(tmpSrc, xoff) & startmask);
									*(pdst+1) &= ~(SCRLEFT(tmpSrc, nFirst) & endmask);
									pglyph += widthGlyph;
									afbScanlineInc(pdst, widthDst);
								}
								break;
							case RROP_INVERT:
								while (h--) {
									getleftbits(pglyph, w, tmpSrc);
									*pdst ^= (SCRRIGHT(tmpSrc, xoff) & startmask);
									*(pdst+1) ^= (SCRLEFT(tmpSrc, nFirst) & endmask);
									pglyph += widthGlyph;
									afbScanlineInc(pdst, widthDst);
								}
								break;
						}
					} /* glyph crosses longwords boundary */
				} /* depth loop */
				/* update character origin */
				x += pci->metrics.characterWidth;
				xchar += pci->metrics.characterWidth;
				if (xchar > PLST) {
					xchar -= PPW;
					pdstBase++;
				} else if (xchar < 0) {
					xchar += PPW;
					pdstBase--;
				}
				ppci++;
			} /* while nglyph-- */
			break;
		case rgnPART:
			{
				afbTEXTPOS *ppos;
				int nbox;
				BoxPtr pbox;
				RegionPtr cclip;
				int xpos;				/* x position of char origin */
				int i;
				BoxRec clip;
				int leftEdge, rightEdge;
				int topEdge, bottomEdge;
				int glyphRow;				/* first row of glyph not wholly
										   clipped out */
				int glyphCol;				/* leftmost visible column of glyph */
#if GETLEFTBITS_ALIGNMENT > 1
				int getWidth;				/* bits to get from glyph */
#endif

				if(!(ppos = (afbTEXTPOS *)xalloc(nglyph * sizeof(afbTEXTPOS))))
					return;

				pdstBase = afbScanlineNoBankSwitch(pdstBase, x, y, widthDst);
				xpos = x;
				xchar = xpos & PIM;

				for (i = 0; i < nglyph; i++) {
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
					if (xchar > PLST) {
						xchar &= PIM;
						pdstBase++;
					} else if (xchar < 0) {
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
				while(nbox--) {
					pbox++;
					clip.x1 = max(bbox.x1, pbox->x1);
					clip.y1 = max(bbox.y1, pbox->y1);
					clip.x2 = min(bbox.x2, pbox->x2);
					clip.y2 = min(bbox.y2, pbox->y2);
					if ((clip.x2<=clip.x1) || (clip.y2<=clip.y1))
						continue;

					for(i=0; i<nglyph; i++) {
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

						hSave = bottomEdge - topEdge;
						if (hSave <= 0)
							continue;

						glyphRow = (topEdge - y) + pci->metrics.ascent;
						widthGlyph = ppos[i].widthGlyph;
						pglyphSave = FONTGLYPHBITS(pglyphBase, pci);
						pglyphSave += (glyphRow * widthGlyph);

						glyphCol = (leftEdge - ppos[i].xpos) -
								   (pci->metrics.leftSideBearing);
#if GETLEFTBITS_ALIGNMENT > 1
						getWidth = w + glyphCol;
#endif

						pdstSave = afbScanlineDelta(ppos[i].pdstBase, -(y-topEdge),
													widthDst);
						xoff = xchar + (leftEdge - ppos[i].xpos);
						if (xoff > PLST) {
							xoff &= PIM;
							pdstSave++;
						} else if (xoff < 0) {
							xoff += PPW;
							pdstSave--;
						}

						for (d = 0; d < depthDst; d++) {
							h = hSave;
							pdst = pdstSave;
							pdstSave += sizeDst;	/* @@@ NEXT PLANE @@@ */
							pglyph = pglyphSave;

							if ((xoff + w) <= PPW) {
								maskpartialbits(xoff, w, startmask);
								switch (rrops[d]) {
									case RROP_WHITE:
										while (h--) {
											getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
											*pdst |= (SCRRIGHT(tmpSrc, xoff) & startmask);
											pglyph += widthGlyph;
											afbScanlineInc(pdst, widthDst);
										}
										break;
									case RROP_BLACK:
										while (h--) {
											getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
											*pdst &= ~(SCRRIGHT(tmpSrc, xoff) & startmask);
											pglyph += widthGlyph;
											afbScanlineInc(pdst, widthDst);
										}
										break;
									case RROP_INVERT:
										while (h--) {
											getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
											*pdst ^= (SCRRIGHT(tmpSrc, xoff) & startmask);
											pglyph += widthGlyph;
											afbScanlineInc(pdst, widthDst);
										}
										break;
								}
							} else {
								maskPPWbits(xoff, w, startmask, endmask);
								nFirst = PPW - xoff;
								switch (rrops[d]) {
									case RROP_WHITE:
										while (h--) {
											getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
											*pdst |= (SCRRIGHT(tmpSrc, xoff) & startmask);
											*(pdst+1) |= (SCRLEFT(tmpSrc, nFirst) & endmask);
											pglyph += widthGlyph;
											afbScanlineInc(pdst, widthDst);
										}
										break;
									case RROP_BLACK:
										while (h--) {
											getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
											*pdst &= ~(SCRRIGHT(tmpSrc, xoff) & startmask);
											*(pdst+1) &= ~(SCRLEFT(tmpSrc, nFirst) & endmask);
											pglyph += widthGlyph;
											afbScanlineInc(pdst, widthDst);
										}
										break;
									case RROP_INVERT:
										while (h--) {
											getshiftedleftbits(pglyph, glyphCol, getWidth, tmpSrc);
											*pdst ^= (SCRRIGHT(tmpSrc, xoff) & startmask);
											*(pdst+1) ^= (SCRLEFT(tmpSrc, nFirst) & endmask);
											pglyph += widthGlyph;
											afbScanlineInc(pdst, widthDst);
										}
										break;
								}
							}
						} /* depth */
					} /* for each glyph */
				} /* while nbox-- */
				xfree(ppos);
				break;
			}

		default:
			break;
	}
}
