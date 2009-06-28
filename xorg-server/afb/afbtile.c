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

#include "mergerop.h"
/*

   the boxes are already translated.

   NOTE:
   iy = ++iy < tileHeight ? iy : 0
is equivalent to iy%= tileheight, and saves a division.
*/

/*
	tile area with a PPW bit wide pixmap
*/
void
MROP_NAME(afbTileAreaPPW)(pDraw, nbox, pbox, alu, ptile, planemask)
	DrawablePtr pDraw;
	int nbox;
	BoxPtr pbox;
	int alu;
	PixmapPtr ptile;
	unsigned long planemask;
{
	register PixelType *psrc;
						/* pointer to bits in tile, if needed */
	int tileHeight;		/* height of the tile */
	register PixelType srcpix;
	int nlwidth;		/* width in longwords of the drawable */
	int w;				/* width of current box */
	MROP_DECLARE_REG ()
	register int h;		/* height of current box */
	register int nlw;		/* loop version of nlwMiddle */
	register PixelType *p;		/* pointer to bits we're writing */
	int sizeDst;
	int depthDst;
	register int d;
	PixelType startmask;
	PixelType endmask;		/* masks for reggedy bits at either end of line */
	int nlwMiddle;		/* number of longwords between sides of boxes */
	int nlwExtra;		/* to get from right of box to left of next span */
	register int iy;		/* index of current scanline in tile */
	PixelType *pbits;		/* pointer to start of drawable */
	PixelType *saveP;
	PixelType *pSaveSrc;
	int saveH;
	int saveIY;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 pbits);

	MROP_INITIALIZE(alu,~0)

	tileHeight = ptile->drawable.height;
	pSaveSrc = (PixelType *)(ptile->devPrivate.ptr);

	while (nbox--) {
		w = pbox->x2 - pbox->x1;
		saveH = pbox->y2 - pbox->y1;
		saveIY = pbox->y1 % tileHeight;
		saveP = afbScanline(pbits, pbox->x1, pbox->y1, nlwidth);
		psrc = pSaveSrc;

		if (((pbox->x1 & PIM) + w) < PPW) {
			maskpartialbits(pbox->x1, w, startmask);
			nlwExtra = nlwidth;
			for (d = 0; d < depthDst; d++, saveP += sizeDst, psrc += tileHeight) {	/* @@@ NEXT PLANE @@@ */
				if (!(planemask & (1 << d)))
					continue;

				p = saveP;
				h = saveH;
				iy = saveIY;

				while (h--) {
					srcpix = psrc[iy];
					iy++;
					if (iy == tileHeight)
						iy = 0;
					*p = MROP_MASK(srcpix,*p,startmask);
					afbScanlineInc(p, nlwExtra);
				}
			}
		} else {
			maskbits(pbox->x1, w, startmask, endmask, nlwMiddle);

			for (d = 0; d < depthDst; d++, saveP += sizeDst, psrc += tileHeight) {	/* @@@ NEXT PLANE @@@ */
				if (!(planemask & (1 << d)))
					continue;

				p = saveP;
				h = saveH;
				iy = saveIY;
				nlwExtra = nlwidth - nlwMiddle;

				if (startmask && endmask) {
					nlwExtra -= 1;
					while (h--) {
						srcpix = psrc[iy];
						iy++;
						if (iy == tileHeight)
							iy = 0;
						nlw = nlwMiddle;
						*p = MROP_MASK (srcpix,*p,startmask);
						p++;
						while (nlw--) {
							*p = MROP_SOLID(srcpix,*p);
							p++;
						}

						*p = MROP_MASK(srcpix,*p,endmask);
						afbScanlineInc(p, nlwExtra);
					}
				} else if (startmask && !endmask) {
					nlwExtra -= 1;
					while (h--) {
						srcpix = psrc[iy];
						iy++;
						if (iy == tileHeight)
							iy = 0;
						nlw = nlwMiddle;
						*p = MROP_MASK(srcpix,*p,startmask);
						p++;
						while (nlw--) {
							*p = MROP_SOLID(srcpix,*p);
							p++;
						}
						afbScanlineInc(p, nlwExtra);
					}
				} else if (!startmask && endmask) {
					while (h--) {
						srcpix = psrc[iy];
						iy++;
						if (iy == tileHeight)
							iy = 0;
						nlw = nlwMiddle;
						while (nlw--) {
							*p = MROP_SOLID(srcpix,*p);
							p++;
						}

						*p = MROP_MASK(srcpix,*p,endmask);
						afbScanlineInc(p, nlwExtra);
					}
				} else { /* no ragged bits at either end */
					while (h--) {
						srcpix = psrc[iy];
						iy++;
						if (iy == tileHeight)
							iy = 0;
						nlw = nlwMiddle;
						while (nlw--) {
							*p = MROP_SOLID (srcpix,*p);
							p++;
						}
						afbScanlineInc(p, nlwExtra);
					}
				}
			} /* for (d = ...) */
		}
		pbox++;
	}
}

void
MROP_NAME(afbTileArea)(pDraw, nbox, pbox, alu, pTile, xOff, yOff, planemask)
	DrawablePtr pDraw;
	int nbox;
	BoxPtr pbox;
	int alu;
	PixmapPtr pTile;
	int xOff;
	int yOff;
	unsigned long planemask;
{
	register PixelType *psrc;
						/* pointer to bits in tile, if needed */
	int nlwidth;		/* width in longwords of the drawable */
	MROP_DECLARE_REG ()
	register int h;		/* height of current box */
	register PixelType *pdst;		/* pointer to bits we're writing */
	register PixelType tmpsrc;
#if (MROP) != Mcopy
	register PixelType tmpdst;
#endif
	int sizeDst;
	int depthDst;
	int sizeTile;
	int tileLine;
	int iline;
	int w, width, x, xSrc, ySrc, srcStartOver, nend;
	int tlwidth, rem, tileWidth, tileHeight, endinc;
	int saveW;
	PixelType *psrcT;
	int d;
	int nstart;
	PixelType startmask;
	PixelType endmask;		/* masks for reggedy bits at either end of line */
	int nlMiddle;		/* number of longwords between sides of boxes */
	int iy;
	PixelType *pBase;		/* pointer to start of drawable */
	PixelType *saveP;
	PixelType *pStartDst;
	PixelType *pStartTile;
	int saveH;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 pBase);

	MROP_INITIALIZE(alu,~0)

	tileHeight = pTile->drawable.height;
	tileWidth = pTile->drawable.width;
	tlwidth = pTile->devKind / sizeof (PixelType);
	sizeTile = tlwidth * tileHeight;

	xSrc = pDraw->x + ((xOff % tileWidth) - tileWidth);
	ySrc = pDraw->y + ((yOff % tileHeight) - tileHeight);

	while (nbox--) {
		saveW = pbox->x2 - pbox->x1;
		iline = (pbox->y1 - ySrc) % tileHeight;
		psrcT = (PixelType *) pTile->devPrivate.ptr;
		tileLine = iline * tlwidth;
		saveH = pbox->y2 - pbox->y1;
		saveP = afbScanline(pBase, pbox->x1, pbox->y1, nlwidth);

		for (d = 0; d < depthDst; d++, psrcT += sizeTile, saveP += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			if (!(planemask & (1 << d)))
				continue;

			h = saveH;
			pStartDst = saveP;
			pStartTile = psrcT + tileLine;
			iy = iline;

			while (h--) {
				x = pbox->x1;
				width = saveW;
				pdst = pStartDst;
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

						getbits ((psrc+endinc), (rem&PIM), w, tmpsrc);
#if (MROP) != Mcopy
						getbits (pdst, (x & PIM), w, tmpdst);
						tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
						putbits (tmpsrc, (x & PIM), w, pdst);

						if((x & PIM) + w >= PPW)
							pdst++;
					} else if(((x & PIM) + w) < PPW) {
						/* doing < PPW bits is easy, and worth special-casing */
						tmpsrc = *psrc;
#if (MROP) != Mcopy
						getbits (pdst, (x & PIM), w, tmpdst);
						tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
						putbits (tmpsrc, (x & PIM), w, pdst);
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
							tmpsrc = *psrc;
#if (MROP) != Mcopy
							getbits (pdst, (x & PIM), nstart, tmpdst);
							tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
							putbits (tmpsrc, (x & PIM), nstart, pdst);
							pdst++;
							if(srcStartOver)
								psrc++;
						}

						while(nlMiddle--) {
							getbits (psrc, nstart, PPW, tmpsrc);
#if (MROP) != Mcopy
							tmpdst = *pdst;
							tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
							*pdst++ = tmpsrc;
							/*putbits (tmpsrc, 0, PPW, pdst);
							pdst++;*/
							psrc++;
						}

						if(endmask) {
							getbits (psrc, nstart, nend, tmpsrc);
#if (MROP) != Mcopy
							tmpdst = *pdst;
							tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
							putbits (tmpsrc, 0, nend, pdst);
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

void
MROP_NAME(afbOpaqueStippleAreaPPW)(pDraw, nbox, pbox, alu, ptile,
								   rropsOS, planemask)
	DrawablePtr pDraw;
	int nbox;
	BoxPtr pbox;
	int alu;
	PixmapPtr ptile;
	register unsigned char *rropsOS;
	unsigned long planemask;
{
	register PixelType *psrc;
						/* pointer to bits in tile, if needed */
	int tileHeight;		/* height of the tile */
	register PixelType srcpix = 0;
	int nlwidth;		/* width in longwords of the drawable */
	int w;				/* width of current box */
	MROP_DECLARE_REG ()
	register int h;		/* height of current box */
	register int nlw;		/* loop version of nlwMiddle */
	register PixelType *p;		/* pointer to bits we're writing */
	int sizeDst;
	int depthDst;
	register int d;
	PixelType startmask;
	PixelType endmask;		/* masks for reggedy bits at either end of line */
	int nlwMiddle;		/* number of longwords between sides of boxes */
	int nlwExtra;		/* to get from right of box to left of next span */
	register int iy;		/* index of current scanline in tile */
	PixelType *pbits;		/* pointer to start of drawable */
	PixelType *saveP;
	int saveH;
	int saveIY;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 pbits);

	MROP_INITIALIZE(alu,~0)

	tileHeight = ptile->drawable.height;
	psrc = (PixelType *)(ptile->devPrivate.ptr);

	while (nbox--) {
		w = pbox->x2 - pbox->x1;
		saveH = pbox->y2 - pbox->y1;
		saveIY = pbox->y1 % tileHeight;
		saveP = afbScanline(pbits, pbox->x1, pbox->y1, nlwidth);

		if ( ((pbox->x1 & PIM) + w) < PPW) {
			maskpartialbits(pbox->x1, w, startmask);
			nlwExtra = nlwidth;
			for (d = 0; d < depthDst; d++, saveP += sizeDst) {	/* @@@ NEXT PLANE @@@ */
				if (!(planemask & (1 << d)))
					continue;

				p = saveP;
				h = saveH;
				iy = saveIY;

				while (h--) {
					switch (rropsOS[d]) {
						case RROP_BLACK:
							srcpix = 0;
							break;
						case RROP_WHITE:
							srcpix = ~0;
							break;
						case RROP_COPY:
							srcpix = psrc[iy];
							break;
						case RROP_INVERT:
							srcpix = ~psrc[iy];
							break;
					}
					iy++;
					if (iy == tileHeight)
						iy = 0;
					*p = MROP_MASK(srcpix,*p,startmask);
					afbScanlineInc(p, nlwExtra);
				}
			}
		} else {
			maskbits(pbox->x1, w, startmask, endmask, nlwMiddle);

			for (d = 0; d < depthDst; d++, saveP += sizeDst) {	/* @@@ NEXT PLANE @@@ */
				if (!(planemask & (1 << d)))
					continue;

				p = saveP;
				h = saveH;
				iy = saveIY;
				nlwExtra = nlwidth - nlwMiddle;

				if (startmask && endmask) {
					nlwExtra -= 1;
					while (h--) {
						switch (rropsOS[d]) {
							case RROP_BLACK:
								srcpix = 0;
								break;
							case RROP_WHITE:
								srcpix = ~0;
								break;
							case RROP_COPY:
								srcpix = psrc[iy];
								break;
							case RROP_INVERT:
								srcpix = ~psrc[iy];
								break;
						}
						iy++;
						if (iy == tileHeight)
							iy = 0;
						nlw = nlwMiddle;
						*p = MROP_MASK (srcpix,*p,startmask);
						p++;
						while (nlw--) {
							*p = MROP_SOLID(srcpix,*p);
							p++;
						}

						*p = MROP_MASK(srcpix,*p,endmask);
						afbScanlineInc(p, nlwExtra);
					}
				} else if (startmask && !endmask) {
					nlwExtra -= 1;
					while (h--) {
						switch (rropsOS[d]) {
							case RROP_BLACK:
								srcpix = 0;
								break;
							case RROP_WHITE:
								srcpix = ~0;
								break;
							case RROP_COPY:
								srcpix = psrc[iy];
								break;
							case RROP_INVERT:
								srcpix = ~psrc[iy];
								break;
						}
						iy++;
						if (iy == tileHeight)
							iy = 0;
						nlw = nlwMiddle;
						*p = MROP_MASK(srcpix,*p,startmask);
						p++;
						while (nlw--) {
							*p = MROP_SOLID(srcpix,*p);
							p++;
						}
						afbScanlineInc(p, nlwExtra);
					}
				} else if (!startmask && endmask) {
					while (h--) {
						switch (rropsOS[d]) {
							case RROP_BLACK:
								srcpix = 0;
								break;
							case RROP_WHITE:
								srcpix = ~0;
								break;
							case RROP_COPY:
								srcpix = psrc[iy];
								break;
							case RROP_INVERT:
								srcpix = ~psrc[iy];
								break;
						}
						iy++;
						if (iy == tileHeight)
							iy = 0;
						nlw = nlwMiddle;
						while (nlw--) {
							*p = MROP_SOLID(srcpix,*p);
							p++;
						}

						*p = MROP_MASK(srcpix,*p,endmask);
						afbScanlineInc(p, nlwExtra);
					}
				} else { /* no ragged bits at either end */
					while (h--) {
						switch (rropsOS[d]) {
							case RROP_BLACK:
								srcpix = 0;
								break;
							case RROP_WHITE:
								srcpix = ~0;
								break;
							case RROP_COPY:
								srcpix = psrc[iy];
								break;
							case RROP_INVERT:
								srcpix = ~psrc[iy];
								break;
						}
						iy++;
						if (iy == tileHeight)
							iy = 0;
						nlw = nlwMiddle;
						while (nlw--) {
							*p = MROP_SOLID (srcpix,*p);
							p++;
						}
						afbScanlineInc(p, nlwExtra);
					}
				}
			} /* for (d = ...) */
		}
		pbox++;
	}
}

void
MROP_NAME(afbOpaqueStippleArea)(pDraw, nbox, pbox, alu, pTile, xOff, yOff,
								rropsOS, planemask)
	DrawablePtr pDraw;
	int nbox;
	BoxPtr pbox;
	int alu;
	PixmapPtr pTile;
	int xOff;
	int yOff;
	register unsigned char *rropsOS;
	unsigned long planemask;
{
	register PixelType *psrc;
						/* pointer to bits in tile, if needed */
	int nlwidth;		/* width in longwords of the drawable */
	MROP_DECLARE_REG ()
	register int h;		/* height of current box */
	register PixelType *pdst;		/* pointer to bits we're writing */
	register PixelType tmpsrc = 0;
#if (MROP) != Mcopy
	register PixelType tmpdst;
#endif
	int sizeDst;
	int depthDst;
	int tileLine;
	int iline;
	int w, width, x, xSrc, ySrc, srcStartOver, nend;
	int tlwidth, rem, tileWidth, tileHeight, endinc;
	int saveW;
	PixelType *psrcT;
	int d;
	int nstart;
	PixelType startmask;
	PixelType endmask;		/* masks for reggedy bits at either end of line */
	int nlMiddle;		/* number of longwords between sides of boxes */
	int iy;
	PixelType *pBase;		/* pointer to start of drawable */
	PixelType *saveP;
	PixelType *pStartDst;
	PixelType *pStartTile;
	int saveH;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 pBase);

	MROP_INITIALIZE(alu,~0)

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
			if (!(planemask & (1 << d)))
				continue;

			h = saveH;
			pStartDst = saveP;
			pStartTile = psrcT + tileLine;
			iy = iline;

			while (h--) {
				x = pbox->x1;
				width = saveW;
				pdst = pStartDst;
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

						switch (rropsOS[d]) {
							case RROP_BLACK:
								tmpsrc = 0;
								break;
							case RROP_WHITE:
								tmpsrc = ~0;
								break;

							case RROP_COPY:
								getbits ((psrc+endinc), (rem&PIM), w, tmpsrc);
								break;

							case RROP_INVERT:
								getbits ((psrc+endinc), (rem&PIM), w, tmpsrc);
								tmpsrc = ~tmpsrc;
								break;
						}
#if (MROP) != Mcopy
						getbits (pdst, (x & PIM), w, tmpdst);
						tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
						putbits (tmpsrc, (x & PIM), w, pdst);

						if((x & PIM) + w >= PPW)
							pdst++;
					} else if(((x & PIM) + w) < PPW) {
						/* doing < PPW bits is easy, and worth special-casing */
						switch (rropsOS[d]) {
							case RROP_BLACK:
								tmpsrc = 0;
								break;
							case RROP_WHITE:
								tmpsrc = ~0;
								break;
							case RROP_COPY:
								tmpsrc = *psrc;
								break;
							case RROP_INVERT:
								tmpsrc = ~*psrc;
								break;
						}
#if (MROP) != Mcopy
						getbits (pdst, (x & PIM), w, tmpdst);
						tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
						putbits (tmpsrc, (x & PIM), w, pdst);
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
						switch (rropsOS[d]) {
							case RROP_BLACK:
								tmpsrc = 0;
								break;
							case RROP_WHITE:
								tmpsrc = ~0;
								break;
							case RROP_COPY:
								tmpsrc = *psrc;
								break;
							case RROP_INVERT:
								tmpsrc = ~*psrc;
								break;
						}
#if (MROP) != Mcopy
							getbits (pdst, (x & PIM), nstart, tmpdst);
							tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
							putbits (tmpsrc, (x & PIM), nstart, pdst);
							pdst++;
							if(srcStartOver)
								psrc++;
						}

						while(nlMiddle--) {
						switch (rropsOS[d]) {
							case RROP_BLACK:
								tmpsrc = 0;
								break;
							case RROP_WHITE:
								tmpsrc = ~0;
								break;
							case RROP_COPY:
								getbits (psrc, nstart, PPW, tmpsrc);
								break;
							case RROP_INVERT:
								getbits (psrc, nstart, PPW, tmpsrc);
								tmpsrc = ~tmpsrc;
								break;
						}
#if (MROP) != Mcopy
							tmpdst = *pdst;
							tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
							*pdst++ = tmpsrc;
							/*putbits (tmpsrc, 0, PPW, pdst);
							pdst++; */
							psrc++;
						}

						if(endmask) {
						switch (rropsOS[d]) {
							case RROP_BLACK:
								tmpsrc = 0;
								break;
							case RROP_WHITE:
								tmpsrc = ~0;
								break;

							case RROP_COPY:
								getbits (psrc, nstart, nend, tmpsrc);
								break;

							case RROP_INVERT:
								getbits (psrc, nstart, nend, tmpsrc);
								tmpsrc = ~tmpsrc;
								break;
						}
#if (MROP) != Mcopy
							tmpdst = *pdst;
							tmpsrc = DoMergeRop (tmpsrc, tmpdst);
#endif
							putbits (tmpsrc, 0, nend, pdst);
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
