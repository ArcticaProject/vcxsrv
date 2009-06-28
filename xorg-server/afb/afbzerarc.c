/************************************************************

Copyright (c) 1989  X Consortium

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

********************************************************/


/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xprotostr.h>
#include "regionstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "afb.h"
#include "maskbits.h"
#include "mizerarc.h"
#include "mi.h"

/*
 * Note: LEFTMOST must be the bit leftmost in the actual screen
 * representation.  This depends also on the IMAGE_BYTE_ORDER.
 * LONG2CHARS() takes care of the re-ordering as required. (DHD)
 */
#if (BITMAP_BIT_ORDER == MSBFirst)
#define LEFTMOST		((PixelType) LONG2CHARS((1 << PLST)))
#else
#define LEFTMOST		((PixelType) LONG2CHARS(1))
#endif

#define Pixelate(base,yoff,xoff) \
{ \
	paddr = afbScanlineOffset(base, (yoff) + ((xoff)>>PWSH)); \
	pmask = SCRRIGHT(LEFTMOST, (xoff) & PIM); \
	for (de = 0; de < depthDst; de++, paddr += sizeDst) /* @@@ NEXT PLANE @@@ */ \
		switch (rrops[de]) { \
			case RROP_BLACK: \
				*paddr &= ~pmask; \
				break; \
			case RROP_WHITE: \
				*paddr |= pmask; \
				break; \
			case RROP_INVERT: \
				*paddr ^= pmask; \
				break; \
			case RROP_NOP: \
				break; \
		} \
}

#define DoPix(bit,base,yoff,xoff) if (mask & bit) Pixelate(base,yoff,xoff);

static void
afbZeroArcSS(DrawablePtr pDraw, GCPtr pGC, xArc *arc)
{
	miZeroArcRec info;
	Bool do360;
	register int de;
	register int x, y, a, b, d, mask;
	register int k1, k3, dx, dy;
	PixelType *addrl;
	PixelType *yorgl, *yorgol;
	int nlwidth, yoffset, dyoffset;
	int sizeDst, depthDst;
	PixelType pmask;
	register PixelType *paddr;
	register unsigned char *rrops;

	rrops = ((afbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					       afbGCPrivateKey))->rrops;
	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 addrl);
	do360 = miZeroArcSetup(arc, &info, TRUE);
	yorgl = addrl + ((info.yorg + pDraw->y) * nlwidth);
	yorgol = addrl + ((info.yorgo + pDraw->y) * nlwidth);
	info.xorg += pDraw->x;
	info.xorgo += pDraw->x;
	MIARCSETUP();
	yoffset = y ? nlwidth : 0;
	dyoffset = 0;
	mask = info.initialMask;
	if (!(arc->width & 1)) {
		DoPix(2, yorgl, 0, info.xorgo);
		DoPix(8, yorgol, 0, info.xorgo);
	}
	if (!info.end.x || !info.end.y) {
		mask = info.end.mask;
		info.end = info.altend;
	}
	if (do360 && (arc->width == arc->height) && !(arc->width & 1)) {
		int xoffset = nlwidth;
		PixelType *yorghl = afbScanlineDeltaNoBankSwitch(yorgl, info.h, nlwidth);
		int xorghp = info.xorg + info.h;
		int xorghn = info.xorg - info.h;

		while (1) {
			Pixelate(yorgl, yoffset, info.xorg + x);
			Pixelate(yorgl, yoffset, info.xorg - x);
			Pixelate(yorgol, -yoffset, info.xorg - x);
			Pixelate(yorgol, -yoffset, info.xorg + x);
			if (a < 0)
				break;
			Pixelate(yorghl, -xoffset, xorghp - y);
			Pixelate(yorghl, -xoffset, xorghn + y);
			Pixelate(yorghl, xoffset, xorghn + y);
			Pixelate(yorghl, xoffset, xorghp - y);
			xoffset += nlwidth;
			MIARCCIRCLESTEP(yoffset += nlwidth;);
		}
		x = info.w;
		yoffset = info.h * nlwidth;
	} else if (do360) {
		while (y < info.h || x < info.w) {
			MIARCOCTANTSHIFT(dyoffset = nlwidth;);
			Pixelate(yorgl, yoffset, info.xorg + x);
			Pixelate(yorgl, yoffset, info.xorgo - x);
			Pixelate(yorgol, -yoffset, info.xorgo - x);
			Pixelate(yorgol, -yoffset, info.xorg + x);
			MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
		}
	} else {
		while (y < info.h || x < info.w) {
			MIARCOCTANTSHIFT(dyoffset = nlwidth;);
			if ((x == info.start.x) || (y == info.start.y)) {
				mask = info.start.mask;
				info.start = info.altstart;
			}
			DoPix(1, yorgl, yoffset, info.xorg + x);
			DoPix(2, yorgl, yoffset, info.xorgo - x);
			DoPix(4, yorgol, -yoffset, info.xorgo - x);
			DoPix(8, yorgol, -yoffset, info.xorg + x);
			if ((x == info.end.x) || (y == info.end.y)) {
				mask = info.end.mask;
				info.end = info.altend;
			}
			MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
		}
	}
	if ((x == info.start.x) || (y == info.start.y))
		mask = info.start.mask;
	DoPix(1, yorgl, yoffset, info.xorg + x);
	DoPix(4, yorgol, -yoffset, info.xorgo - x);
	if (arc->height & 1) {
		DoPix(2, yorgl, yoffset, info.xorgo - x);
		DoPix(8, yorgol, -yoffset, info.xorg + x);
	}
}

void
afbZeroPolyArcSS(DrawablePtr pDraw, GCPtr pGC, int narcs, xArc *parcs)
{
	register xArc *arc;
	register int i;
	BoxRec box;
	RegionPtr cclip;

	cclip = pGC->pCompositeClip;
	for (arc = parcs, i = narcs; --i >= 0; arc++) {
		if (miCanZeroArc(arc)) {
			box.x1 = arc->x + pDraw->x;
			box.y1 = arc->y + pDraw->y;
			box.x2 = box.x1 + (int)arc->width + 1;
			box.y2 = box.y1 + (int)arc->height + 1;
			if (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN)
				afbZeroArcSS(pDraw, pGC, arc);
			else
				miZeroPolyArc(pDraw, pGC, 1, arc);
		} else
			miPolyArc(pDraw, pGC, 1, arc);
	}
}
