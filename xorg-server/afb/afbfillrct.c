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
#include <X11/Xprotostr.h>
#include "pixmapstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "regionstr.h"
#include "scrnintstr.h"

#include "afb.h"
#include "maskbits.h"

#define MODEQ(a, b) ((a) %= (b))

/*
	filled rectangles.
	translate the rectangles, clip them, and call the
helper function in the GC.
*/

#define NUM_STACK_RECTS		1024

void
afbPolyFillRect(DrawablePtr pDrawable, GCPtr pGC, int nrectFill, xRectangle *prectInit)
	                      
	          
	              				/* number of rectangles to fill */
	                      			/* Pointer to first rectangle to fill */
{
	xRectangle *prect;
	RegionPtr prgnClip;
	register BoxPtr pbox;
	register BoxPtr pboxClipped;
	BoxPtr pboxClippedBase;
	BoxPtr pextent;
	BoxRec stackRects[NUM_STACK_RECTS];
	int numRects;
	int n;
	int xorg, yorg;
	afbPrivGC *priv;
	unsigned char *rrops;
	unsigned char *rropsOS;

	priv = (afbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					     afbGCPrivateKey);
	prgnClip = pGC->pCompositeClip;
	rrops = priv->rrops;
	rropsOS = priv->rropOS;

	prect = prectInit;
	xorg = pDrawable->x;
	yorg = pDrawable->y;
	if (xorg || yorg) {
		prect = prectInit;
		n = nrectFill;
		Duff(n, prect->x += xorg; prect->y += yorg; prect++);
	}

	prect = prectInit;

	numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
	if (numRects > NUM_STACK_RECTS) {
		pboxClippedBase = (BoxPtr)xalloc(numRects * sizeof(BoxRec));
		if (!pboxClippedBase)
			return;
	}
	else
		pboxClippedBase = stackRects;

	pboxClipped = pboxClippedBase;

	if (REGION_NUM_RECTS(prgnClip) == 1) {
		int x1, y1, x2, y2, bx2, by2;

		pextent = REGION_RECTS(prgnClip);
		x1 = pextent->x1;
		y1 = pextent->y1;
		x2 = pextent->x2;
		y2 = pextent->y2;
			while (nrectFill--) {
			if ((pboxClipped->x1 = prect->x) < x1)
				pboxClipped->x1 = x1;

			if ((pboxClipped->y1 = prect->y) < y1)
				pboxClipped->y1 = y1;

			bx2 = (int) prect->x + (int) prect->width;
			if (bx2 > x2)
				bx2 = x2;
			pboxClipped->x2 = bx2;

			by2 = (int) prect->y + (int) prect->height;
			if (by2 > y2)
				by2 = y2;
			pboxClipped->y2 = by2;

			prect++;
			if ((pboxClipped->x1 < pboxClipped->x2) &&
				(pboxClipped->y1 < pboxClipped->y2)) {
				pboxClipped++;
			}
			}
	} else {
		int x1, y1, x2, y2, bx2, by2;

		pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
		x1 = pextent->x1;
		y1 = pextent->y1;
		x2 = pextent->x2;
		y2 = pextent->y2;
			while (nrectFill--) {
			BoxRec box;

			if ((box.x1 = prect->x) < x1)
				box.x1 = x1;

			if ((box.y1 = prect->y) < y1)
				box.y1 = y1;

			bx2 = (int) prect->x + (int) prect->width;
			if (bx2 > x2)
				bx2 = x2;
			box.x2 = bx2;

			by2 = (int) prect->y + (int) prect->height;
			if (by2 > y2)
				by2 = y2;
			box.y2 = by2;

			prect++;

			if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
					continue;

			n = REGION_NUM_RECTS (prgnClip);
			pbox = REGION_RECTS(prgnClip);

			/* clip the rectangle to each box in the clip region
			   this is logically equivalent to calling Intersect()
			*/
			while(n--) {
				pboxClipped->x1 = max(box.x1, pbox->x1);
				pboxClipped->y1 = max(box.y1, pbox->y1);
				pboxClipped->x2 = min(box.x2, pbox->x2);
				pboxClipped->y2 = min(box.y2, pbox->y2);
				pbox++;

				/* see if clipping left anything */
				if(pboxClipped->x1 < pboxClipped->x2 &&
				   pboxClipped->y1 < pboxClipped->y2) {
					pboxClipped++;
				}
			}
			}
	}
	if (pboxClipped != pboxClippedBase) {
		switch (pGC->fillStyle) {
			case FillSolid:
				afbSolidFillArea(pDrawable, pboxClipped-pboxClippedBase,
										pboxClippedBase, rrops);
				break;
			case FillTiled:
				switch (pGC->alu) {
					case GXcopy:
						if (pGC->pRotatedPixmap)
							afbTileAreaPPWCopy(pDrawable, pboxClipped-pboxClippedBase,
												pboxClippedBase, GXcopy,
												pGC->pRotatedPixmap, pGC->planemask);
						else
							afbTileAreaCopy (pDrawable, pboxClipped-pboxClippedBase,
												pboxClippedBase, GXcopy,
												pGC->tile.pixmap,
												pGC->patOrg.x, pGC->patOrg.y,
												pGC->planemask);
						break;

					default:
						if (pGC->pRotatedPixmap)
							afbTileAreaPPWGeneral(pDrawable, pboxClipped-pboxClippedBase,
												   pboxClippedBase, pGC->alu,
												   pGC->pRotatedPixmap,
												   pGC->planemask);
						else
							afbTileAreaGeneral(pDrawable, pboxClipped-pboxClippedBase,
												   pboxClippedBase, pGC->alu,
												   pGC->tile.pixmap,
												   pGC->patOrg.x, pGC->patOrg.y,
												   pGC->planemask);
						break;
				} /* switch (alu) */
				break;

			case FillStippled:
				if (pGC->pRotatedPixmap)
					afbStippleAreaPPW(pDrawable, pboxClipped-pboxClippedBase,
									   pboxClippedBase, pGC->pRotatedPixmap, rrops);
				else
					afbStippleArea(pDrawable, pboxClipped-pboxClippedBase,
									pboxClippedBase, pGC->stipple,
									pGC->patOrg.x, pGC->patOrg.y, rrops);
				break;

			case FillOpaqueStippled:
				switch (pGC->alu) {
					case GXcopy:
						if (pGC->pRotatedPixmap)
							afbOpaqueStippleAreaPPWCopy(pDrawable, pboxClipped-pboxClippedBase,
												pboxClippedBase, GXcopy,
												pGC->pRotatedPixmap,
												rropsOS, pGC->planemask);
						else
							afbOpaqueStippleAreaCopy(pDrawable, pboxClipped-pboxClippedBase,
												pboxClippedBase, GXcopy,
												pGC->stipple,
												pGC->patOrg.x, pGC->patOrg.y, rropsOS,
												pGC->planemask);
						break;

					default:
						if (pGC->pRotatedPixmap)
							afbOpaqueStippleAreaPPWGeneral(pDrawable, pboxClipped-pboxClippedBase,
													pboxClippedBase, pGC->alu,
													pGC->pRotatedPixmap,
													rropsOS,
													pGC->planemask);
						else
							afbOpaqueStippleAreaGeneral(pDrawable, pboxClipped-pboxClippedBase,
													pboxClippedBase, pGC->alu,
													pGC->stipple,
													pGC->patOrg.x, pGC->patOrg.y,
													rropsOS,
													pGC->planemask);
						break;
				} /* switch (alu) */
				break;
		}
	}
	if (pboxClippedBase != stackRects)
			xfree(pboxClippedBase);
}
