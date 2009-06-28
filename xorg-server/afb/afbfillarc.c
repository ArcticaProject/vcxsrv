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
#include "mifillarc.h"
#include "mi.h"

static void
afbFillEllipseSolid(DrawablePtr pDraw, xArc *arc, register unsigned char *rrops)
{
	int x, y, e;
	int yk, xk, ym, xm, dx, dy, xorg, yorg;
	register int slw;
	miFillArcRec info;
	PixelType *addrlt, *addrlb;
	register PixelType *pdst;
	PixelType *addrl;
	register int n;
	register int d;
	int nlwidth;
	register int xpos;
	PixelType startmask, endmask;
	int nlmiddle;
	int depthDst;
	int sizeDst;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 addrlt);
	miFillArcSetup(arc, &info);
	MIFILLARCSETUP();
	xorg += pDraw->x;
	yorg += pDraw->y;
	addrlb = addrlt;
	addrlt += nlwidth * (yorg - y);
	addrlb += nlwidth * (yorg + y + dy);
	while (y) {
		addrlt += nlwidth;
		addrlb -= nlwidth;
		MIFILLARCSTEP(slw);
		if (!slw)
			continue;
		xpos = xorg - x;
		pdst = addrl = afbScanlineOffset(addrlt, (xpos >> PWSH));
		if (((xpos & PIM) + slw) < PPW) {
			maskpartialbits(xpos, slw, startmask);
			for (d = 0; d < depthDst; d++, pdst += sizeDst) {	/* @@@ NEXT PLANE @@@ */
				switch (rrops[d]) {
					case RROP_BLACK:
						*pdst &= ~startmask;
						break;
					case RROP_WHITE:
						*pdst |= startmask;
						break;
					case RROP_INVERT:
						*pdst ^= startmask;
						break;
					case RROP_NOP:
						break;
				}
			}
			if (miFillArcLower(slw)) {
				pdst = afbScanlineOffset(addrlb, (xpos >> PWSH));

				for (d = 0; d < depthDst; d++, pdst += sizeDst) {	/* @@@ NEXT PLANE @@@ */
					switch (rrops[d]) {
						case RROP_BLACK:
							*pdst &= ~startmask;
							break;
						case RROP_WHITE:
							*pdst |= startmask;
							break;
						case RROP_INVERT:
							*pdst ^= startmask;
							break;
						case RROP_NOP:
							break;
					}
				}
			}
			continue;
		}
		maskbits(xpos, slw, startmask, endmask, nlmiddle);
		for (d = 0; d < depthDst; d++, addrl += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			n = nlmiddle;
			pdst = addrl;

			switch (rrops[d]) {
				case RROP_BLACK:
					if (startmask)
						*pdst++ &= ~startmask;
					while (n--)
						*pdst++ = 0;
					if (endmask)
						*pdst &= ~endmask;
					break;

				case RROP_WHITE:
					if (startmask)
						*pdst++ |= startmask;
					while (n--)
						*pdst++ = ~0;
					if (endmask)
						*pdst |= endmask;
					break;

				case RROP_INVERT:
					if (startmask)
						*pdst++ ^= startmask;
					while (n--)
						*pdst++ ^= ~0;
					if (endmask)
						*pdst ^= endmask;
					break;

				case RROP_NOP:
					break;
			}
		}
		if (!miFillArcLower(slw))
			continue;
		addrl = afbScanlineOffset(addrlb, (xpos >> PWSH));
		for (d = 0; d < depthDst; d++, addrl += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			n = nlmiddle;
			pdst = addrl;

			switch (rrops[d]) {
				case RROP_BLACK:
					if (startmask)
						*pdst++ &= ~startmask;
					while (n--)
						*pdst++ = 0;
					if (endmask)
						*pdst &= ~endmask;
					break;

				case RROP_WHITE:
					if (startmask)
						*pdst++ |= startmask;
					while (n--)
						*pdst++ = ~0;
					if (endmask)
						*pdst |= endmask;
					break;

				case RROP_INVERT:
					if (startmask)
						*pdst++ ^= startmask;
					while (n--)
						*pdst++ ^= ~0;
					if (endmask)
						*pdst ^= endmask;
					break;

				case RROP_NOP:
					break;
			}
		}
	}
}

#define FILLSPAN(xl,xr,addr) \
	if (xr >= xl) { \
		width = xr - xl + 1; \
		addrl = afbScanlineOffset(addr, (xl >> PWSH)); \
		if (((xl & PIM) + width) < PPW) { \
			maskpartialbits(xl, width, startmask); \
			for (pdst = addrl, d = 0; d < depthDst; d++, pdst += sizeDst) { /* @@@ NEXT PLANE @@@ */ \
				switch (rrops[d]) { \
					case RROP_BLACK: \
						*pdst &= ~startmask; \
						break; \
					case RROP_WHITE: \
						*pdst |= startmask; \
						break; \
					case RROP_INVERT: \
						*pdst ^= startmask; \
						break; \
					case RROP_NOP: \
						break; \
				} \
			} \
		} else { \
			maskbits(xl, width, startmask, endmask, nlmiddle); \
			for (d = 0; d < depthDst; d++, addrl += sizeDst) {	/* @@@ NEXT PLANE @@@ */ \
				n = nlmiddle; \
				pdst = addrl; \
				switch (rrops[d]) { \
					case RROP_BLACK: \
						if (startmask) \
							*pdst++ &= ~startmask; \
						while (n--) \
							*pdst++ = 0; \
						if (endmask) \
							*pdst &= ~endmask; \
						break; \
					case RROP_WHITE: \
						if (startmask) \
							*pdst++ |= startmask; \
						while (n--) \
							*pdst++ = ~0; \
						if (endmask) \
							*pdst |= endmask; \
						break; \
					case RROP_INVERT: \
						if (startmask) \
							*pdst++ ^= startmask; \
						while (n--) \
							*pdst++ ^= ~0; \
						if (endmask) \
							*pdst ^= endmask; \
						break; \
					case RROP_NOP: \
						break; \
				} \
			} \
		} \
	}

#define FILLSLICESPANS(flip,addr) \
	if (!flip) { \
		FILLSPAN(xl, xr, addr); \
	} else { \
		xc = xorg - x; \
		FILLSPAN(xc, xr, addr); \
		xc += slw - 1; \
		FILLSPAN(xl, xc, addr); \
	}

static void
afbFillArcSliceSolidCopy(DrawablePtr pDraw, GCPtr pGC, xArc *arc, register unsigned char *rrops)
{
	PixelType *addrl;
	register PixelType *pdst;
	register int n;
	register int d;
	int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
	register int x, y, e;
	miFillArcRec info;
	miArcSliceRec slice;
	int xl, xr, xc;
	PixelType *addrlt, *addrlb;
	int nlwidth;
	int width;
	PixelType startmask, endmask;
	int nlmiddle;
	int sizeDst;
	int depthDst;

	afbGetPixelWidthSizeDepthAndPointer(pDraw, nlwidth, sizeDst, depthDst,
													 addrlt);
	miFillArcSetup(arc, &info);
	miFillArcSliceSetup(arc, &slice, pGC);
	MIFILLARCSETUP();
	xorg += pDraw->x;
	yorg += pDraw->y;
	addrlb = addrlt;
	addrlt = afbScanlineDeltaNoBankSwitch(addrlt, yorg - y, nlwidth);
	addrlb = afbScanlineDeltaNoBankSwitch(addrlb, yorg + y + dy, nlwidth);
	slice.edge1.x += pDraw->x;
	slice.edge2.x += pDraw->x;
	while (y > 0) {
		afbScanlineIncNoBankSwitch(addrlt, nlwidth);
		afbScanlineIncNoBankSwitch(addrlb, -nlwidth);
		MIFILLARCSTEP(slw);
		MIARCSLICESTEP(slice.edge1);
		MIARCSLICESTEP(slice.edge2);
		if (miFillSliceUpper(slice)) {
			MIARCSLICEUPPER(xl, xr, slice, slw);
			FILLSLICESPANS(slice.flip_top, addrlt);
		}
		if (miFillSliceLower(slice)) {
			MIARCSLICELOWER(xl, xr, slice, slw);
			FILLSLICESPANS(slice.flip_bot, addrlb);
		}
	}
}

void
afbPolyFillArcSolid(register DrawablePtr pDraw, GCPtr pGC, int narcs, xArc *parcs)
{
	afbPrivGC *priv;
	register xArc *arc;
	register int i;
	BoxRec box;
	RegionPtr cclip;
	unsigned char *rrops;

	priv = (afbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					     afbGCPrivateKey);
	rrops = priv->rrops;
	cclip = pGC->pCompositeClip;
	for (arc = parcs, i = narcs; --i >= 0; arc++) {
		if (miFillArcEmpty(arc))
			continue;
		if (miCanFillArc(arc)) {
			box.x1 = arc->x + pDraw->x;
			box.y1 = arc->y + pDraw->y;
			box.x2 = box.x1 + (int)arc->width + 1;
			box.y2 = box.y1 + (int)arc->height + 1;
			if (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN) {
				if ((arc->angle2 >= FULLCIRCLE) ||
					(arc->angle2 <= -FULLCIRCLE))
					afbFillEllipseSolid(pDraw, arc, rrops);
				else
					afbFillArcSliceSolidCopy(pDraw, pGC, arc, rrops);
				continue;
			}
		}
		miPolyFillArc(pDraw, pGC, 1, arc);
	}
}
