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

#include <X11/X.h>

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "afb.h"
#include "maskbits.h"

/*
   the solid fillers are called for rectangles and window backgrounds.
   the boxes are already translated.
   maybe this should always take a pixmap instead of a drawable?

   NOTE:
   iy = ++iy < tileHeight ? iy : 0
is equivalent to iy%= tileheight, and saves a division.
*/

/*ARGSUSED*/
void
afbSolidFillArea (pDraw, nbox, pbox, rrops)
	DrawablePtr pDraw;
	int nbox;
	BoxPtr pbox;
	register unsigned char *rrops;
{
	int nlwidth;				/* width in longwords of the drawable */
	int w;						/* width of current box */
	register int h;			/* height of current box */
	register PixelType *p;	/* pointer to bits we're writing */
	register int nlw;			/* loop version of nlwMiddle */
	register PixelType startmask;
	register PixelType endmask;
									/* masks for reggedy bits at either end of line */
	register int nlwExtra;
									/* to get from right of box to left of next span */
	int nlwMiddle;				/* number of longwords between sides of boxes */
	PixelType *pbits;			/* pointer to start of drawable */
	PixelType *saveP;
	int saveH;
	int depthDst;
	int sizeDst;
	register int d;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 pbits);

	while (nbox--) {
		w = pbox->x2 - pbox->x1;
		saveH = pbox->y2 - pbox->y1;

		saveP = afbScanline(pbits, pbox->x1, pbox->y1, nlwidth);

		if ( ((pbox->x1 & PIM) + w) < PPW) {
			for (d = 0; d < depthDst; d++) {
				h = saveH;
				p = saveP;
				saveP += sizeDst;	/* @@@ NEXT PLANE @@@ */
				maskpartialbits(pbox->x1, w, startmask);
				nlwExtra = nlwidth;

				switch (rrops[d]) {
					case RROP_BLACK:
						Duff(h, *p &= ~startmask; afbScanlineInc(p, nlwExtra));
						break;
					case RROP_WHITE:
						Duff(h, *p |= startmask; afbScanlineInc(p, nlwExtra));
						break;
					case RROP_INVERT:
						Duff(h, *p ^= startmask; afbScanlineInc(p, nlwExtra));
						break;
					case RROP_NOP:
						break;
				} /* switch */
			} /* for (d = ..) */
		} else {
			maskbits(pbox->x1, w, startmask, endmask, nlwMiddle);

			for (d = 0; d < depthDst; d++) {
				h = saveH;
				p = saveP;
				saveP += sizeDst;	/* @@@ NEXT PLANE @@@ */
				nlwExtra = nlwidth - nlwMiddle;

				if (startmask && endmask) {
					nlwExtra -= 1;
					switch (rrops[d]) {
						case RROP_BLACK:
							while (h--) {
								nlw = nlwMiddle;
								*p &= ~startmask;
								p++;
								Duff(nlw, *p++ = 0);
								*p &= ~endmask;
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_WHITE:
							while (h--) {
								nlw = nlwMiddle;
								*p |= startmask;
								p++;
								Duff(nlw, *p++ = ~0);
								*p |= endmask;
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_INVERT:
							while (h--) {
								nlw = nlwMiddle;
								*p ^= startmask;
								p++;
								Duff(nlw, *p++ ^= ~0);
								*p ^= endmask;
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_NOP:
							break;
					}
				} else if (startmask && !endmask) {
					nlwExtra -= 1;
					switch (rrops[d]) {
						case RROP_BLACK:
							while (h--) {
								nlw = nlwMiddle;
								*p &= ~startmask;
								p++;
								Duff(nlw, *p++ = 0);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_WHITE:
							while (h--) {
								nlw = nlwMiddle;
								*p |= startmask;
								p++;
								Duff(nlw, *p++ = ~0);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_INVERT:
							while (h--) {
								nlw = nlwMiddle;
								*p ^= startmask;
								p++;
								Duff(nlw, *p++ ^= ~0);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_NOP:
							break;
					}
				} else if (!startmask && endmask) {
					switch (rrops[d]) {
						case RROP_BLACK:
							while (h--) {
								nlw = nlwMiddle;
								Duff(nlw, *p++ = 0);
								*p &= ~endmask;
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_WHITE:
							while (h--) {
								nlw = nlwMiddle;
								Duff(nlw, *p++ = ~0);
								*p |= endmask;
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_INVERT:
							while (h--) {
								nlw = nlwMiddle;
								Duff(nlw, *p++ ^= ~0);
								*p ^= endmask;
								afbScanlineInc(p, nlwExtra);
							}
						case RROP_NOP:
							break;
					}
				} else { /* no ragged bits at either end */
					switch (rrops[d]) {
						case RROP_BLACK:
							while (h--) {
								nlw = nlwMiddle;
								Duff(nlw, *p++ = 0);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_WHITE:
							while (h--) {
								nlw = nlwMiddle;
								Duff(nlw, *p++ = ~0);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_INVERT:
							while (h--) {
								nlw = nlwMiddle;
								Duff(nlw, *p++ ^= ~0);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_NOP:
							break;
					} /* switch */
				}
			} /* for (d = 0 ... ) */
		}
		pbox++;
	}
}

/* stipple a list of boxes -

you can use the reduced rasterop for stipples.  if rrop is
black, AND the destination with (not stipple pattern).  if rrop is
white OR the destination with the stipple pattern.  if rrop is invert,
XOR the destination with the stipple pattern.
*/

/*ARGSUSED*/
void
afbStippleAreaPPW (pDraw, nbox, pbox, pstipple, rrops)
	DrawablePtr pDraw;
	int nbox;
	BoxPtr pbox;
	PixmapPtr pstipple;
	unsigned char *rrops;
{
	register PixelType *psrc;
						/* pointer to bits in tile, if needed */
	int tileHeight;		/* height of the tile */
	register PixelType srcpix;

	int nlwidth;		/* width in longwords of the drawable */
	int w;				/* width of current box */
	register int nlw;		/* loop version of nlwMiddle */
	register PixelType *p;		/* pointer to bits we're writing */
	register int h;		/* height of current box */
	PixelType startmask;
	PixelType endmask;		/* masks for reggedy bits at either end of line */
	int nlwMiddle;		/* number of longwords between sides of boxes */
	int nlwExtra;		/* to get from right of box to left of next span */
	int sizeDst;
	int depthDst;
	int d;
	int saveIy;
	register int iy;		/* index of current scanline in tile */
	PixelType *pbits;		/* pointer to start of drawable */
	PixelType *pBase;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 pBase);

	tileHeight = pstipple->drawable.height;
	psrc = (PixelType *)(pstipple->devPrivate.ptr);

	while (nbox--) {
		w = pbox->x2 - pbox->x1;
		saveIy = pbox->y1 % tileHeight;
		pbits = pBase;

		if ( ((pbox->x1 & PIM) + w) < PPW) {
			maskpartialbits(pbox->x1, w, startmask);
			nlwExtra = nlwidth;
			for (d = 0; d < depthDst; d++) {
				p = afbScanline(pbits, pbox->x1, pbox->y1, nlwidth);
				pbits += sizeDst;	/* @@@ NEXT PLANE @@@ */
				iy = saveIy;
				h = pbox->y2 - pbox->y1;

				switch (rrops[d]) {
					case RROP_BLACK:
						while (h--) {
							srcpix = psrc[iy];
							iy = ++iy < tileHeight ? iy : 0;
							*p &= ~(srcpix & startmask);
							afbScanlineInc(p, nlwExtra);
						}
						break;
					case RROP_WHITE:
						while (h--) {
							srcpix = psrc[iy];
							iy = ++iy < tileHeight ? iy : 0;
							*p |= (srcpix & startmask);
							afbScanlineInc(p, nlwExtra);
						}
						break;
					case RROP_INVERT:
						while (h--) {
							srcpix = psrc[iy];
							iy = ++iy < tileHeight ? iy : 0;
							*p ^= (srcpix & startmask);
							afbScanlineInc(p, nlwExtra);
						}
						break;
					case RROP_NOP:
						break;
				} /* switch */
			} /* for (d = ...) */

		} else {
			maskbits(pbox->x1, w, startmask, endmask, nlwMiddle);

			for (d = 0; d < depthDst; d++) {
				nlwExtra = nlwidth - nlwMiddle;
				p = afbScanline(pbits, pbox->x1, pbox->y1, nlwidth);
				pbits += sizeDst;	/* @@@ NEXT PLANE @@@ */
				iy = saveIy;
				h = pbox->y2 - pbox->y1;

				if (startmask && endmask) {
					nlwExtra -= 1;
					switch (rrops[d]) {
						case RROP_BLACK:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								*p &= ~(srcpix & startmask);
								p++;
								Duff (nlw, *p++ &= ~srcpix);
								*p &= ~(srcpix & endmask);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_WHITE:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								*p |= (srcpix & startmask);
								p++;
								Duff (nlw, *p++ |= srcpix);
								*p |= (srcpix & endmask);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_INVERT:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								*p ^= (srcpix & startmask);
								p++;
								Duff (nlw, *p++ ^= srcpix);
								*p ^= (srcpix & endmask);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_NOP:
							break;
					} /* switch */
				} else if (startmask && !endmask) {
					nlwExtra -= 1;
					switch (rrops[d]) {
						case RROP_BLACK:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								*p &= ~(srcpix & startmask);
								p++;
								Duff(nlw, *p++ &= ~srcpix);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_WHITE:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								*p |= (srcpix & startmask);
								p++;
								Duff(nlw, *p++ |= srcpix);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_INVERT:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								*p ^= (srcpix & startmask);
								p++;
								Duff(nlw, *p++ ^= srcpix);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_NOP:
							break;
					} /* switch */
				} else if (!startmask && endmask) {
					switch (rrops[d]) {
						case RROP_BLACK:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								Duff(nlw, *p++ &= ~srcpix);
								*p &= ~(srcpix & endmask);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_WHITE:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								Duff(nlw, *p++ |= srcpix);
								*p |= (srcpix & endmask);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_INVERT:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								Duff(nlw, *p++ ^= srcpix);
								*p ^= (srcpix & endmask);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_NOP:
							break;
					} /* switch */
				} else { /* no ragged bits at either end */
					switch (rrops[d]) {
						case RROP_BLACK:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								Duff(nlw, *p++ &= ~srcpix);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_WHITE:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								Duff(nlw, *p++ |= srcpix);
								afbScanlineInc(p, nlwExtra);
							}
							break;
						case RROP_INVERT:
							while (h--) {
								srcpix = psrc[iy];
								iy = ++iy < tileHeight ? iy : 0;
								nlw = nlwMiddle;
								Duff(nlw, *p++ ^= srcpix);
								afbScanlineInc(p, nlwExtra);
							}
							break;
					} /* switch */
				}
			} /* for (d = ...) */
		}
		pbox++;
	}
}

void
afbStippleArea (pDraw, nbox, pbox, pTile, xOff, yOff, rrops)
	DrawablePtr pDraw;
	int nbox;
	BoxPtr pbox;
	PixmapPtr pTile;
	int xOff;
	int yOff;
	unsigned char *rrops;
{
	register PixelType *psrc;	/* pointer to bits in tile, if needed */
	int nlwidth;					/* width in longwords of the drawable */
	register int h;				/* height of current box */
	register PixelType *pdst;	/* pointer to bits we're writing */
	int sizeDst;
	int depthDst;
	int tileLine;
	int iline;
	int w, width, x, xSrc, ySrc, srcStartOver, nend;
	int tlwidth, rem, tileWidth, tileHeight, endinc;
	int saveW;
	register int rop;
	PixelType *psrcT;
	int d;
	int nstart;
	PixelType startmask;
	PixelType endmask;		/* masks for reggedy bits at either end of line */
	int nlMiddle;				/* number of longwords between sides of boxes */
	int iy;
	PixelType *pBase;			/* pointer to start of drawable */
	PixelType *saveP;
	PixelType *pStartDst;
	PixelType *pStartTile;
	int saveH;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 pBase);

	tileHeight = pTile->drawable.height;
	tileWidth = pTile->drawable.width;
	tlwidth = pTile->devKind / sizeof (PixelType);

	xSrc = pDraw->x + ((xOff % tileWidth) - tileWidth);
	ySrc = pDraw->y + ((yOff % tileHeight) - tileHeight);

	while (nbox--) {
		saveW = pbox->x2 - pbox->x1;
		iline = (pbox->y1 - ySrc) % tileHeight;
		psrcT = (PixelType *) pTile->devPrivate.ptr;
		tileLine = iline * tlwidth;
		saveH = pbox->y2 - pbox->y1;
		saveP = afbScanline(pBase, pbox->x1, pbox->y1, nlwidth);

		for (d = 0; d < depthDst; d++, saveP += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			h = saveH;
			pStartDst = saveP;
			pStartTile = psrcT + tileLine;
			iy = iline;

			while (h--) {
				x = pbox->x1;
				width = saveW;
				pdst = pStartDst;
				rop = rrops[d];

				while(width > 0) {
					psrc = pStartTile;
					w = min(tileWidth, width);
					if((rem = (x - xSrc) % tileWidth) != 0) {
						/* if we're in the middle of the tile, get
						   as many bits as will finish the span, or
						   as many as will get to the left edge of the tile,
						   or a longword worth, starting at the appropriate
						   offset in the tile.
						*/
						w = min(min(tileWidth - rem, width), BITMAP_SCANLINE_PAD);
						endinc = rem / BITMAP_SCANLINE_PAD;
						getandputrrop((psrc + endinc), (rem & PIM), (x & PIM),
									 w, pdst, rop)
						if((x & PIM) + w >= PPW)
							pdst++;
					} else if(((x & PIM) + w) < PPW) {
						/* doing < PPW bits is easy, and worth special-casing */
						putbitsrrop(*psrc, x & PIM, w, pdst, rop);
					} else {
						/* start at the left edge of the tile,
						   and put down as much as we can
						*/
						maskbits(x, w, startmask, endmask, nlMiddle);

						if (startmask)
							nstart = PPW - (x & PIM);
						else
							nstart = 0;
						if (endmask)
							nend = (x + w)  & PIM;
						else
							nend = 0;

						srcStartOver = nstart > PLST;

						if(startmask) {
							putbitsrrop(*psrc, (x & PIM), nstart, pdst, rop);
							pdst++;
							if(srcStartOver)
								psrc++;
						}

						while(nlMiddle--) {
							getandputrrop0(psrc, nstart, PPW, pdst, rop);
							pdst++;
							psrc++;
						}

						if(endmask) {
							getandputrrop0(psrc, nstart, nend, pdst, rop);
						}
					 }
					 x += w;
					 width -= w;
				} /* while (width > 0) */

				pStartDst += nlwidth;
				if (++iy >= tileHeight) {
					iy = 0;
					pStartTile = psrcT;
				} else
					pStartTile += tlwidth;

			} /* while (h) */
		} /* for (d = ... ) */
		pbox++;
	} /* for each box */
}
