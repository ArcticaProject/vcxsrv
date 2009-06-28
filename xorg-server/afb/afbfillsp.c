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
#include <X11/Xmd.h>
#include "gcstruct.h"
#include "window.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "afb.h"

#include "maskbits.h"

#include "mergerop.h"

#include "servermd.h"
#include "mi.h"
#include "mispans.h"

/* scanline filling for monochrome frame buffer
   written by drewry, oct 1986

   these routines all clip.  they assume that anything that has called
them has already translated the points (i.e. pGC->miTranslate is
non-zero, which is howit gets set in afbCreateGC().)

   the number of new scnalines created by clipping ==
MaxRectsPerBand * nSpans.

*/


void
afbSolidFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
	DrawablePtr pDrawable;
	GCPtr		pGC;
	int				nInit;				/* number of spans to fill */
	DDXPointPtr pptInit;		/* pointer to list of start points */
	int				*pwidthInit;		/* pointer to list of n widths */
	int 		fSorted;
{
								/* next three parameters are post-clip */
	int n;						/* number of spans to fill */
	register DDXPointPtr ppt;		/* pointer to list of start points */
	register int *pwidth;		/* pointer to list of n widths */
	PixelType *addrlBase;		/* pointer to start of bitmap */
	PixelType *pBase;
	int nlwidth;				/* width in longwords of bitmap */
	register PixelType *addrl;/* pointer to current longword in bitmap */
	register int nlmiddle;
	register PixelType startmask;
	register PixelType endmask;
	int *pwidthFree;				/* copies of the pointers to free */
	DDXPointPtr pptFree;
	int depthDst;
	int sizeDst;
	int		d;
	unsigned char *rrops;

	n = nInit * miFindMaxBand(pGC->pCompositeClip);
	pwidthFree = (int *)xalloc(n * sizeof(int));
	pptFree = (DDXPointRec *)xalloc(n * sizeof(DDXPointRec));
	if(!pptFree || !pwidthFree) {
		if (pptFree) xfree(pptFree);
		if (pwidthFree) xfree(pwidthFree);
		return;
	}
	pwidth = pwidthFree;
	ppt = pptFree;
	n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit,
					ppt, pwidth, fSorted);

	afbGetPixelWidthSizeDepthAndPointer(pDrawable, nlwidth, sizeDst, depthDst,
													 pBase);
	rrops = ((afbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					       afbGCPrivateKey))->rrops;
	while (n--) {
		addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);

		for (d = 0; d < depthDst; d++, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			if (*pwidth) {
				addrl = addrlBase;

				switch (rrops[d]) {
					case RROP_BLACK:
						if ( ((ppt->x & PIM) + *pwidth) < PPW) {
							/* all bits inside same longword */
							maskpartialbits(ppt->x, *pwidth, startmask);
								*addrl &= ~startmask;
						} else {
							maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
							if (startmask)
								*addrl++ &= ~startmask;
							Duff (nlmiddle, *addrl++ = 0x0);
							if (endmask)
								*addrl &= ~endmask;
						}
						break;

					case RROP_WHITE:
						if ( ((ppt->x & PIM) + *pwidth) < PPW) {
							/* all bits inside same longword */
							maskpartialbits(ppt->x, *pwidth, startmask);
							*addrl |= startmask;
						} else {
							maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
							if (startmask)
								*addrl++ |= startmask;
							Duff (nlmiddle, *addrl++ = ~0);
							if (endmask)
								*addrl |= endmask;
						}
						break;
					case RROP_INVERT:
						if ( ((ppt->x & PIM) + *pwidth) < PPW) {
							/* all bits inside same longword */
							maskpartialbits(ppt->x, *pwidth, startmask);
							*addrl ^= startmask;
						} else {
							maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
							if (startmask)
								*addrl++ ^= startmask;
							Duff (nlmiddle, *addrl++ ^= ~0);
							if (endmask)
								*addrl ^= endmask;
						}
						break;
					case RROP_NOP:
						break;
				}
			}
		}
		pwidth++;
		ppt++;
	}
	xfree(pptFree);
	xfree(pwidthFree);
}

void 
afbStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
	DrawablePtr pDrawable;
	GC *pGC;
	int nInit;						/* number of spans to fill */
	DDXPointPtr pptInit;		/* pointer to list of start points */
	int *pwidthInit;				/* pointer to list of n widths */
	int fSorted;
{
								/* next three parameters are post-clip */
	int n;						/* number of spans to fill */
	register DDXPointPtr ppt;		/* pointer to list of start points */
	register int *pwidth;		/* pointer to list of n widths */
	PixelType *addrlBase;		/* pointer to start of bitmap */
	PixelType *pBase;
	int nlwidth;				/* width in longwords of bitmap */
	register PixelType *addrl;/* pointer to current longword in bitmap */
	register PixelType src;
	register int nlmiddle;
	register PixelType startmask;
	register PixelType endmask;
	PixmapPtr pStipple;
	PixelType *psrc;
	int tileHeight;
	int *pwidthFree;				/* copies of the pointers to free */
	DDXPointPtr pptFree;
	int d;
	int depthDst;
	int sizeDst;
	unsigned char *rrops;

	n = nInit * miFindMaxBand(pGC->pCompositeClip);
	pwidthFree = (int *)xalloc(n * sizeof(int));
	pptFree = (DDXPointRec *)xalloc(n * sizeof(DDXPointRec));
	if(!pptFree || !pwidthFree) {
		if (pptFree) xfree(pptFree);
		if (pwidthFree) xfree(pwidthFree);
		return;
	}
	pwidth = pwidthFree;
	ppt = pptFree;
	n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit, 
					ppt, pwidth, fSorted);

	afbGetPixelWidthSizeDepthAndPointer(pDrawable, nlwidth, sizeDst, depthDst,
													 pBase);

	pStipple = pGC->pRotatedPixmap;
	tileHeight = pStipple->drawable.height;
	psrc = (PixelType *)(pStipple->devPrivate.ptr);

	rrops = ((afbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					       afbGCPrivateKey))->rrops;
	while (n--) {
		src = psrc[ppt->y % tileHeight];
		addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);
		for (d = 0; d < depthDst; d++, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			addrl = addrlBase;

			switch (rrops[d]) {
				case RROP_BLACK:
					/* all bits inside same longword */
					if ( ((ppt->x & PIM) + *pwidth) < PPW) {
						maskpartialbits(ppt->x, *pwidth, startmask);
						*addrl &= ~(src & startmask);
					} else {
						maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
						if (startmask)
							*addrl++ &= ~(src & startmask);
						Duff (nlmiddle, *addrl++ &= ~src);
						if (endmask)
							*addrl &= ~(src & endmask);
					}
					break;
				case RROP_WHITE:
					/* all bits inside same longword */
					if ( ((ppt->x & PIM) + *pwidth) < PPW) {
						maskpartialbits(ppt->x, *pwidth, startmask);
						*addrl |= (src & startmask);
					} else {
						maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
						if (startmask)
							*addrl++ |= (src & startmask);
						Duff (nlmiddle, *addrl++ |= src);
						if (endmask)
							*addrl |= (src & endmask);
					}
					break;
				case RROP_INVERT:
					/* all bits inside same longword */
					if ( ((ppt->x & PIM) + *pwidth) < PPW) {
						maskpartialbits(ppt->x, *pwidth, startmask);
						*addrl ^= (src & startmask);
					} else {
						maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
						if (startmask)
							*addrl++ ^= (src & startmask);
						Duff (nlmiddle, *addrl++ ^= src);
						if (endmask)
							*addrl ^= (src & endmask);
					}
					break;

				case RROP_NOP:
					break;
			}
		}
		pwidth++;
		ppt++;
	}
	xfree(pptFree);
	xfree(pwidthFree);
}

void
afbTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
	DrawablePtr pDrawable;
	GC *pGC;
	int nInit;						/* number of spans to fill */
	DDXPointPtr pptInit;		/* pointer to list of start points */
	int *pwidthInit;				/* pointer to list of n widths */
	int fSorted;
{
								/* next three parameters are post-clip */
	int n;						/* number of spans to fill */
	register DDXPointPtr ppt;		/* pointer to list of start points */
	register int *pwidth;		/* pointer to list of n widths */
	PixelType *addrlBase;		/* pointer to start of bitmap */
	PixelType *pBase;
	int nlwidth;				/* width in longwords of bitmap */
	register PixelType *addrl;		/* pointer to current longword in bitmap */
	register PixelType src;
	register int nlmiddle;
	register PixelType startmask;
	register PixelType endmask;
	PixmapPtr pTile;
	PixelType *psrc;
	int tileHeight;
	int rop;
	int *pwidthFree;				/* copies of the pointers to free */
	DDXPointPtr pptFree;
	int sizeDst;
	int depthDst;
	int d;

	n = nInit * miFindMaxBand(pGC->pCompositeClip);
	pwidthFree = (int *)xalloc(n * sizeof(int));
	pptFree = (DDXPointRec *)xalloc(n * sizeof(DDXPointRec));
	if(!pptFree || !pwidthFree) {
		if (pptFree) xfree(pptFree);
		if (pwidthFree) xfree(pwidthFree);
		return;
	}
	pwidth = pwidthFree;
	ppt = pptFree;
	n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit, 
					ppt, pwidth, fSorted);

	afbGetPixelWidthSizeDepthAndPointer(pDrawable, nlwidth, sizeDst, depthDst,
													 pBase);

	pTile = pGC->pRotatedPixmap;
	tileHeight = pTile->drawable.height;
	psrc = (PixelType *)(pTile->devPrivate.ptr);
	rop = pGC->alu;

	switch(rop) {
		case GXcopy:
#define DoMaskCopyRop(src,dst,mask)		(((dst) & ~(mask)) | ((src) & (mask)))
			while (n--) {
				if (*pwidth) {
					addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);

					for (d = 0; d < depthDst; d++, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
						if (!(pGC->planemask & (1 << d)))
							continue;

						addrl = addrlBase;
						src = psrc[ppt->y % tileHeight + tileHeight * d]; 
						if ( ((ppt->x & PIM) + *pwidth) < PPW) {
							maskpartialbits(ppt->x, *pwidth, startmask);
							*addrl = DoMaskCopyRop (src, *addrl, startmask);
						} else {
							maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
							if (startmask) {
								*addrl = DoMaskCopyRop (src, *addrl, startmask);
								addrl++;
							}
							while (nlmiddle--) {
								*addrl = src;
								addrl++;
							}
							if (endmask)
								*addrl = DoMaskCopyRop (src, *addrl, endmask);
						}
					}
				}
				pwidth++;
				ppt++;
			}
			break;

		default:
			{
				register DeclareMergeRop ();

				InitializeMergeRop(rop,~0);
				while (n--) {
					if (*pwidth) {
						addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);
						for (d = 0; d < depthDst; d++, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
							if (!(pGC->planemask & (1 << d)))
								continue;

							addrl = addrlBase;

							src = psrc[ppt->y % tileHeight + tileHeight * d];
							if ( ((ppt->x & PIM) + *pwidth) < PPW) {
								maskpartialbits(ppt->x, *pwidth, startmask);
								*addrl = DoMaskMergeRop (src, *addrl, startmask);
							} else {
								maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
								if (startmask) {
									*addrl = DoMaskMergeRop (src, *addrl, startmask);
									addrl++;
								}
								while (nlmiddle--) {
									*addrl = DoMergeRop (src, *addrl);
									addrl++;
								}
								if (endmask)
									*addrl = DoMaskMergeRop (src, *addrl, endmask);
							}
						}
					}
					pwidth++;
					ppt++;
				}
				break;
			}
	}
	xfree(pptFree);
	xfree(pwidthFree);
}

void
afbOpaqueStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
	DrawablePtr pDrawable;
	GC *pGC;
	int nInit;						/* number of spans to fill */
	DDXPointPtr pptInit;		/* pointer to list of start points */
	int *pwidthInit;				/* pointer to list of n widths */
	int fSorted;
{
								/* next three parameters are post-clip */
	int n;						/* number of spans to fill */
	register DDXPointPtr ppt;		/* pointer to list of start points */
	register int *pwidth;		/* pointer to list of n widths */
	PixelType *addrlBase;		/* pointer to start of bitmap */
	PixelType *pBase;
	int nlwidth;				/* width in longwords of bitmap */
	register PixelType *addrl;		/* pointer to current longword in bitmap */
	register PixelType src = 0;
	register int nlmiddle;
	register PixelType startmask;
	register PixelType endmask;
	PixmapPtr pTile;
	PixelType *psrc;
	int tileHeight;
	int rop;
	unsigned char *rropsOS;
	int *pwidthFree;				/* copies of the pointers to free */
	DDXPointPtr pptFree;
	int sizeDst;
	int depthDst;
	int d;

	n = nInit * miFindMaxBand(pGC->pCompositeClip);
	pwidthFree = (int *)xalloc(n * sizeof(int));
	pptFree = (DDXPointRec *)xalloc(n * sizeof(DDXPointRec));
	if(!pptFree || !pwidthFree) {
		if (pptFree) xfree(pptFree);
		if (pwidthFree) xfree(pwidthFree);
		return;
	}
	pwidth = pwidthFree;
	ppt = pptFree;
	n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit, 
					ppt, pwidth, fSorted);

	afbGetPixelWidthSizeDepthAndPointer(pDrawable, nlwidth, sizeDst, depthDst,
													 pBase);

	pTile = pGC->pRotatedPixmap;
	tileHeight = pTile->drawable.height;
	psrc = (PixelType *)(pTile->devPrivate.ptr);
	rop = pGC->alu;
	rropsOS = ((afbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
						 afbGCPrivateKey))->rropOS;
	switch(rop) {
		case GXcopy:
			while (n--) {
				addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);
				if (*pwidth) {
					for (d = 0; d < depthDst; d++, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
						switch (rropsOS[d]) {
							case RROP_BLACK:
								src = 0;
								break;
							case RROP_WHITE:
								src = ~0;
								break;
							case RROP_INVERT:
								src = ~psrc[ppt->y % tileHeight];
								break;
							case RROP_COPY:
								src = psrc[ppt->y % tileHeight];
								break;
							case RROP_NOP:
								continue;
						}

						addrl = addrlBase;

						if ( ((ppt->x & PIM) + *pwidth) < PPW) {
							maskpartialbits(ppt->x, *pwidth, startmask);
							*addrl = DoMaskCopyRop (src, *addrl, startmask);
						} else {
							maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
							if (startmask) {
								*addrl = DoMaskCopyRop (src, *addrl, startmask);
								addrl++;
							}
							while (nlmiddle--) {
								*addrl = src;
								addrl++;
							}
							if (endmask)
								*addrl = DoMaskCopyRop (src, *addrl, endmask);
						}
					} /* for (d = ...) */
				}

				pwidth++;
				ppt++;
			}
			break;

		default:
			{
				register DeclareMergeRop ();

				InitializeMergeRop(rop,~0);
				while (n--) {
					addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);
					if (*pwidth) {
						for (d = 0; d < depthDst; d++, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
							switch (rropsOS[d]) {
								case RROP_BLACK:
									src = 0;
									break;
								case RROP_WHITE:
									src = ~0;
									break;
								case RROP_INVERT:
									src = ~psrc[ppt->y % tileHeight];
									break;
								case RROP_COPY:
									src = psrc[ppt->y % tileHeight];
									break;
								case RROP_NOP:
									continue;
							}

							addrl = addrlBase;

							if ( ((ppt->x & PIM) + *pwidth) < PPW) {
								maskpartialbits(ppt->x, *pwidth, startmask);
								*addrl = DoMaskMergeRop (src, *addrl, startmask);
							} else {
								maskbits(ppt->x, *pwidth, startmask, endmask, nlmiddle);
								if (startmask) {
									*addrl = DoMaskMergeRop (src, *addrl, startmask);
									addrl++;
								}
								while (nlmiddle--) {
									*addrl = DoMergeRop (src, *addrl);
									addrl++;
								}
								if (endmask)
									*addrl = DoMaskMergeRop (src, *addrl, endmask);
							}
						} /* for (d = ...) */
					}
					pwidth++;
					ppt++;
				} /* while (n) */
				break;
			}
	} /* switch (rop) */
	xfree(pptFree);
	xfree(pwidthFree);
}

/* Fill spans with tiles that aren't PPW bits wide */
void
afbUnnaturalTileFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
	DrawablePtr pDrawable;
	GC				*pGC;
	int				nInit;				/* number of spans to fill */
	DDXPointPtr pptInit;		/* pointer to list of start points */
	int *pwidthInit;				/* pointer to list of n widths */
	int fSorted;
{
	int				iline;				/* first line of tile to use */
								/* next three parameters are post-clip */
	int n;						/* number of spans to fill */
	register DDXPointPtr ppt;		/* pointer to list of start points */
	register int *pwidth;		/* pointer to list of n widths */
	PixelType *addrlBase;		/* pointer to start of bitmap */
	PixelType *pBase;
	int				 nlwidth;		/* width in longwords of bitmap */
	register PixelType *pdst;/* pointer to current word in bitmap */
	register PixelType *psrc;/* pointer to current word in tile */
	register int nlMiddle;
	register int rop, nstart;
	PixelType startmask;
	PixmapPtr		pTile;				/* pointer to tile we want to fill with */
	int				w, width, x, xSrc, ySrc, srcStartOver, nend;
	int 		tlwidth, rem, tileWidth, tileHeight, endinc;
	PixelType	  endmask, *psrcT;
	int *pwidthFree;				/* copies of the pointers to free */
	DDXPointPtr pptFree;
	int sizeDst;
	int sizeTile;
	int depthDst;
	register int d;

	n = nInit * miFindMaxBand(pGC->pCompositeClip);
	pwidthFree = (int *)xalloc(n * sizeof(int));
	pptFree = (DDXPointRec *)xalloc(n * sizeof(DDXPointRec));
	if(!pptFree || !pwidthFree) {
		if (pptFree) xfree(pptFree);
		if (pwidthFree) xfree(pwidthFree);
		return;
	}
	pwidth = pwidthFree;
	ppt = pptFree;
	n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit, 
					ppt, pwidth, fSorted);

	pTile = pGC->tile.pixmap;
	tlwidth = pTile->devKind / PGSZB;
	rop = pGC->alu;

	xSrc = pDrawable->x;
	ySrc = pDrawable->y;

	afbGetPixelWidthSizeDepthAndPointer(pDrawable, nlwidth, sizeDst, depthDst,
													 pBase);

	tileWidth = pTile->drawable.width;
	tileHeight = pTile->drawable.height;
	sizeTile = tlwidth * tileHeight;

	/* this replaces rotating the tile. Instead we just adjust the offset
	 * at which we start grabbing bits from the tile.
	 * Ensure that ppt->x - xSrc >= 0 and ppt->y - ySrc >= 0,
	 * so that iline and rem always stay within the tile bounds.
	 */
	xSrc += (pGC->patOrg.x % tileWidth) - tileWidth;
	ySrc += (pGC->patOrg.y % tileHeight) - tileHeight;

	while (n--) {
		iline = (ppt->y - ySrc) % tileHeight;
		psrcT = (PixelType *) pTile->devPrivate.ptr + (iline * tlwidth);
		addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);

		for (d = 0; d < depthDst; d++, psrcT += sizeTile, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			if (!(pGC->planemask & (1 << d)))
				continue;

			if (*pwidth) {
				x = ppt->x;
				pdst = addrlBase;
				width = *pwidth;
				while(width > 0) {
					psrc = psrcT;
					w = min(tileWidth, width);
					if((rem = (x - xSrc)  % tileWidth) != 0) {
						/* if we're in the middle of the tile, get
						   as many bits as will finish the span, or
						   as many as will get to the left edge of the tile,
						   or a longword worth, starting at the appropriate
						   offset in the tile.
						*/
						w = min(min(tileWidth - rem, width), BITMAP_SCANLINE_PAD);
						endinc = rem / BITMAP_SCANLINE_PAD;
						getandputrop((psrc+endinc), (rem&PIM), (x & PIM), w, pdst, rop);
						if((x & PIM) + w >= PPW)
							pdst++;
					} else if(((x & PIM) + w) < PPW) {
						/* doing < PPW bits is easy, and worth special-casing */
						putbitsrop(*psrc, x & PIM, w, pdst, rop);
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
							putbitsrop(*psrc, (x & PIM), nstart, pdst, rop);
							pdst++;
							if(srcStartOver)
								psrc++;
						}
						 
						while(nlMiddle--) {
								getandputrop0(psrc, nstart, PPW, pdst, rop);
								pdst++;
								psrc++;
						}
						if(endmask) {
							getandputrop0(psrc, nstart, nend, pdst, rop);
						}
					 }
					 x += w;
					 width -= w;
				}
			}
		}
		ppt++;
		pwidth++;
	}
	xfree(pptFree);
	xfree(pwidthFree);
}

/* Fill spans with stipples that aren't PPW bits wide */
void
afbUnnaturalStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
	DrawablePtr pDrawable;
	GC				*pGC;
	int				nInit;				/* number of spans to fill */
	DDXPointPtr pptInit;		/* pointer to list of start points */
	int *pwidthInit;				/* pointer to list of n widths */
	int fSorted;
{
								/* next three parameters are post-clip */
	int n;						/* number of spans to fill */
	register DDXPointPtr ppt;		/* pointer to list of start points */
	register int *pwidth;		/* pointer to list of n widths */
	int				iline;				/* first line of tile to use */
	PixelType		*addrlBase;		/* pointer to start of bitmap */
	PixelType		*pBase;
	int				 nlwidth;		/* width in longwords of bitmap */
	register PixelType *pdst;				/* pointer to current word in bitmap */
	register PixelType *psrc;				/* pointer to current word in tile */
	register int nlMiddle;
	register int rop, nstart;
	PixelType startmask;
	PixmapPtr		pTile;				/* pointer to tile we want to fill with */
	int				w, width,  x, xSrc, ySrc, srcStartOver, nend;
	PixelType 		endmask, *psrcT;
	int 		tlwidth, rem, tileWidth, endinc;
	int				tileHeight;
	int *pwidthFree;				/* copies of the pointers to free */
	DDXPointPtr pptFree;
	unsigned char *rrops;
	register int d;
	int sizeDst;
	int depthDst;

	n = nInit * miFindMaxBand(pGC->pCompositeClip);
	pwidthFree = (int *)xalloc(n * sizeof(int));
	pptFree = (DDXPointRec *)xalloc(n * sizeof(DDXPointRec));
	if(!pptFree || !pwidthFree) {
		if (pptFree) xfree(pptFree);
		if (pwidthFree) xfree(pwidthFree);
		return;
	}
	pwidth = pwidthFree;
	ppt = pptFree;
	n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit, 
					ppt, pwidth, fSorted);

	pTile = pGC->stipple;
	tlwidth = pTile->devKind / PGSZB;
	xSrc = pDrawable->x;
	ySrc = pDrawable->y;
	afbGetPixelWidthSizeDepthAndPointer(pDrawable, nlwidth, sizeDst, depthDst,
													 pBase);

	tileWidth = pTile->drawable.width;
	tileHeight = pTile->drawable.height;
	rrops = ((afbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					       afbGCPrivateKey))->rrops;
	/* this replaces rotating the stipple.  Instead, we just adjust the offset
	 * at which we start grabbing bits from the stipple.
	 * Ensure that ppt->x - xSrc >= 0 and ppt->y - ySrc >= 0,
	 * so that iline and rem always stay within the tile bounds.
	 */
	xSrc += (pGC->patOrg.x % tileWidth) - tileWidth;
	ySrc += (pGC->patOrg.y % tileHeight) - tileHeight;
	while (n--) {
		iline = (ppt->y - ySrc) % tileHeight;
		psrcT = (PixelType *) pTile->devPrivate.ptr + (iline * tlwidth);
		addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);

		for (d = 0; d < depthDst; d++, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			rop = rrops[d];
			if (rop == RROP_NOP)
				continue;

			pdst = addrlBase;
			x = ppt->x;

			if (*pwidth) {
				width = *pwidth;
				while(width > 0) {
					psrc = psrcT;
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
				}
			}
		}
		ppt++;
		pwidth++;
	}
	xfree(pptFree);
	xfree(pwidthFree);
}

/* Fill spans with OpaqueStipples that aren't PPW bits wide */
void
afbUnnaturalOpaqueStippleFS(pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)
	DrawablePtr pDrawable;
	GC				*pGC;
	int				nInit;				/* number of spans to fill */
	DDXPointPtr pptInit;		/* pointer to list of start points */
	int *pwidthInit;				/* pointer to list of n widths */
	int fSorted;
{
	int				iline;				/* first line of tile to use */
								/* next three parameters are post-clip */
	int n;						/* number of spans to fill */
	register DDXPointPtr ppt;		/* pointer to list of start points */
	register int *pwidth;		/* pointer to list of n widths */
	PixelType *addrlBase;		/* pointer to start of bitmap */
	PixelType *pBase;
	int				 nlwidth;		/* width in longwords of bitmap */
	register PixelType *pdst;/* pointer to current word in bitmap */
	register PixelType *psrc;/* pointer to current word in tile */
	register int nlMiddle;
	register int d;
	register PixelType tmpsrc = 0;
	register PixelType tmpdst;
	register int alu, nstart;
	register unsigned char *rropsOS;
	PixelType startmask;
	PixmapPtr		pTile;				/* pointer to tile we want to fill with */
	int				w, width, x, xSrc, ySrc, srcStartOver, nend;
	int 		tlwidth, rem, tileWidth, tileHeight, endinc;
	PixelType	  endmask, *psrcT;
	int *pwidthFree;				/* copies of the pointers to free */
	DDXPointPtr pptFree;
	int sizeDst;
	int depthDst;

	n = nInit * miFindMaxBand(pGC->pCompositeClip);
	pwidthFree = (int *)xalloc(n * sizeof(int));
	pptFree = (DDXPointRec *)xalloc(n * sizeof(DDXPointRec));
	if(!pptFree || !pwidthFree) {
		if (pptFree) xfree(pptFree);
		if (pwidthFree) xfree(pwidthFree);
		return;
	}
	pwidth = pwidthFree;
	ppt = pptFree;
	n = miClipSpans(pGC->pCompositeClip, pptInit, pwidthInit, nInit, 
					ppt, pwidth, fSorted);

	pTile = pGC->stipple;
	tlwidth = pTile->devKind / PGSZB;
	alu = pGC->alu;

	xSrc = pDrawable->x;
	ySrc = pDrawable->y;

	afbGetPixelWidthSizeDepthAndPointer(pDrawable, nlwidth, sizeDst, depthDst,
													 pBase);

	tileWidth = pTile->drawable.width;
	tileHeight = pTile->drawable.height;
	rropsOS = afbGetGCPrivate(pGC)->rropOS;

	/* this replaces rotating the tile. Instead we just adjust the offset
	 * at which we start grabbing bits from the tile.
	 * Ensure that ppt->x - xSrc >= 0 and ppt->y - ySrc >= 0,
	 * so that iline and rem always stay within the tile bounds.
	 */
	xSrc += (pGC->patOrg.x % tileWidth) - tileWidth;
	ySrc += (pGC->patOrg.y % tileHeight) - tileHeight;

	while (n--) {
		iline = (ppt->y - ySrc) % tileHeight;
		psrcT = (PixelType *) pTile->devPrivate.ptr + (iline * tlwidth);
		addrlBase = afbScanline(pBase, ppt->x, ppt->y, nlwidth);

		for (d = 0; d < depthDst; d++, addrlBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			if (!(pGC->planemask & (1 << d)))
				continue;

			if (*pwidth) {
				x = ppt->x;
				pdst = addrlBase;
				width = *pwidth;
				while(width > 0) {
					psrc = psrcT;
					w = min(tileWidth, width);
					if((rem = (x - xSrc)  % tileWidth) != 0) {
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

						if (alu != GXcopy) {
							getbits (pdst, (x & PIM), w, tmpdst);
							DoRop (tmpsrc, alu, tmpsrc, tmpdst);
						}

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
						if (alu != GXcopy) {
							getbits (pdst, (x & PIM), w, tmpdst);
							DoRop (tmpsrc, alu, tmpsrc, tmpdst);
						}
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
							if (alu != GXcopy) {
								getbits (pdst, (x & PIM), nstart, tmpdst);
								DoRop (tmpsrc, alu, tmpsrc, tmpdst);
							}
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
							if (alu != GXcopy) {
								tmpdst = *pdst;
								DoRop (tmpsrc, alu, tmpsrc, tmpdst);
							}
							*pdst++ = tmpsrc;
							/*putbits (tmpsrc, 0, PPW, pdst);
								pdst++;*/
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
							if (alu != GXcopy) {
								tmpdst = *pdst;
								DoRop (tmpsrc, alu, tmpsrc, tmpdst);
							}
							putbits (tmpsrc, 0, nend, pdst);
						}
					}
					 x += w;
					 width -= w;
				}
			}
		}
		ppt++;
		pwidth++;
	}
	xfree(pptFree);
	xfree(pwidthFree);
}
