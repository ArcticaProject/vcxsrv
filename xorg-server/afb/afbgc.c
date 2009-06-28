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

#include <string.h>

#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xproto.h>
#include "afb.h"
#include "dixfontstr.h"
#include <X11/fonts/fontstruct.h>
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mistruct.h"
#include "migc.h"

#include "maskbits.h"

static void afbDestroyGC(GCPtr);
static void afbValidateGC(GCPtr, unsigned long, DrawablePtr);

static GCFuncs afbFuncs = {
		afbValidateGC,
		miChangeGC,
		miCopyGC,
		afbDestroyGC,
		miChangeClip,
		miDestroyClip,
		miCopyClip
};

static GCOps afbGCOps = {
		afbSolidFS,
		afbSetSpans,
		afbPutImage,
		afbCopyArea,
		miCopyPlane,
		afbPolyPoint,
		afbLineSS,
		afbSegmentSS,
		miPolyRectangle,
		afbZeroPolyArcSS,
		afbFillPolygonSolid,
		afbPolyFillRect,
		afbPolyFillArcSolid,
		miPolyText8,
		miPolyText16,
		miImageText8,
		miImageText16,
		afbTEGlyphBlt,
		afbPolyGlyphBlt,
		afbPushPixels
};

static void
afbReduceOpaqueStipple(PixelType fg, PixelType bg, unsigned long planemask,
		       int depth, unsigned char *rop)
{
	register int d;
	register Pixel mask = 1;

	bg ^= fg;

	for (d = 0; d < depth; d++, mask <<= 1) {
		if (!(planemask & mask))
			rop[d] = RROP_NOP;
		else if (!(bg & mask)) {
			/* Both fg and bg have a 0 or 1 in this plane */
			if (fg & mask)
				rop[d] = RROP_WHITE;
			else
				rop[d] = RROP_BLACK;
		} else {
			/* Both fg and bg have different bits on this plane */
			if (fg & mask)
				rop[d] = RROP_COPY;
			else
				rop[d] = RROP_INVERT;
		}
	}
}

Bool
afbCreateGC(pGC)
	register GCPtr pGC;
{
	afbPrivGC 		*pPriv;

	pGC->clientClip = NULL;
	pGC->clientClipType = CT_NONE;

	/* some of the output primitives aren't really necessary, since
	   they will be filled in ValidateGC because of dix/CreateGC()
	   setting all the change bits.  Others are necessary because although
	   they depend on being a monochrome frame buffer, they don't change
	*/

	pGC->ops = &afbGCOps;
	pGC->funcs = &afbFuncs;

	/* afb wants to translate before scan convesion */
	pGC->miTranslate = 1;

	pPriv = (afbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					      afbGCPrivateKey);
	afbReduceRop(pGC->alu, pGC->fgPixel, pGC->planemask, pGC->depth,
		pPriv->rrops);
	afbReduceOpaqueStipple(pGC->fgPixel, pGC->bgPixel, pGC->planemask,
		pGC->depth, pPriv->rropOS);

	pGC->fExpose = TRUE;
	pGC->pRotatedPixmap = NullPixmap;
	pGC->freeCompClip = FALSE;
	return TRUE;
}

static void
afbComputeCompositeClip(GCPtr pGC, DrawablePtr pDrawable)
{
	if (pDrawable->type == DRAWABLE_WINDOW) {
		WindowPtr pWin = (WindowPtr) pDrawable;
		RegionPtr pregWin;
		Bool freeTmpClip, freeCompClip;

	if (pGC->subWindowMode == IncludeInferiors) {
		pregWin = NotClippedByChildren(pWin);
		freeTmpClip = TRUE;
	} else {
		pregWin = &pWin->clipList;
		freeTmpClip = FALSE;
	}
	freeCompClip = pGC->freeCompClip;

	/*
	 * if there is no client clip, we can get by with just keeping the
	 * pointer we got, and remembering whether or not should destroy (or
	 * maybe re-use) it later.  this way, we avoid unnecessary copying of
	 * regions.  (this wins especially if many clients clip by children
	 * and have no client clip.)
	 */
	if (pGC->clientClipType == CT_NONE) {
		if (freeCompClip)
			REGION_DESTROY(pGC->pScreen, pGC->pCompositeClip);
			pGC->pCompositeClip = pregWin;
			pGC->freeCompClip = freeTmpClip;
		} else {
			/*
			 * we need one 'real' region to put into the composite clip. if
			 * pregWin the current composite clip are real, we can get rid of
			 * one. if pregWin is real and the current composite clip isn't,
			 * use pregWin for the composite clip. if the current composite
			 * clip is real and pregWin isn't, use the current composite
			 * clip. if neither is real, create a new region.
			 */

			REGION_TRANSLATE(pGC->pScreen, pGC->clientClip,
			pDrawable->x + pGC->clipOrg.x,
			pDrawable->y + pGC->clipOrg.y);

			if (freeCompClip) {
				REGION_INTERSECT(pGC->pScreen, pGC->pCompositeClip, pregWin,
									  pGC->clientClip);
				if (freeTmpClip)
					REGION_DESTROY(pGC->pScreen, pregWin);
			} else if (freeTmpClip) {
				REGION_INTERSECT(pGC->pScreen, pregWin, pregWin, pGC->clientClip);
				pGC->pCompositeClip = pregWin;
			} else {
				pGC->pCompositeClip = REGION_CREATE(pGC->pScreen, NullBox, 0);
				REGION_INTERSECT(pGC->pScreen, pGC->pCompositeClip,
				pregWin, pGC->clientClip);
			}
			pGC->freeCompClip = TRUE;
			REGION_TRANSLATE(pGC->pScreen, pGC->clientClip,
			-(pDrawable->x + pGC->clipOrg.x),
			-(pDrawable->y + pGC->clipOrg.y));
		}
	}	/* end of composite clip for a window */
	else {
		BoxRec pixbounds;

		/* XXX should we translate by drawable.x/y here ? */
		pixbounds.x1 = 0;
		pixbounds.y1 = 0;
		pixbounds.x2 = pDrawable->width;
		pixbounds.y2 = pDrawable->height;

		if (pGC->freeCompClip) {
			REGION_RESET(pGC->pScreen, pGC->pCompositeClip, &pixbounds);
		} else {
			pGC->freeCompClip = TRUE;
			pGC->pCompositeClip = REGION_CREATE(pGC->pScreen, &pixbounds, 1);
		}

		if (pGC->clientClipType == CT_REGION) {
			REGION_TRANSLATE(pGC->pScreen, pGC->pCompositeClip, -pGC->clipOrg.x,
								  -pGC->clipOrg.y);
			REGION_INTERSECT(pGC->pScreen, pGC->pCompositeClip,
								  pGC->pCompositeClip, pGC->clientClip);
			REGION_TRANSLATE(pGC->pScreen, pGC->pCompositeClip, pGC->clipOrg.x,
								  pGC->clipOrg.y);
		}
	}	/* end of composite clip for pixmap */
} /* end afbComputeCompositeClip */

/* Clipping conventions
		if the drawable is a window
			CT_REGION ==> pCompositeClip really is the composite
			CT_other ==> pCompositeClip is the window clip region
		if the drawable is a pixmap
			CT_REGION ==> pCompositeClip is the translated client region
				clipped to the pixmap boundary
			CT_other ==> pCompositeClip is the pixmap bounding box
*/

/*ARGSUSED*/
static void
afbValidateGC(pGC, changes, pDrawable)
	register GCPtr 		pGC;
	unsigned long		changes;
	DrawablePtr 		pDrawable;
{
	register afbPrivGCPtr devPriv;
	int mask;		/* stateChanges */
	int index;		/* used for stepping through bitfields */
	int xrot, yrot;		/* rotations for tile and stipple pattern */
				/* flags for changing the proc vector
				   and updating things in devPriv
				*/
	int new_rotate, new_rrop,  new_line, new_text, new_fill;
	DDXPointRec		oldOrg;				/* origin of thing GC was last used with */

	oldOrg = pGC->lastWinOrg;

	pGC->lastWinOrg.x = pDrawable->x;
	pGC->lastWinOrg.y = pDrawable->y;

	/* we need to re-rotate the tile if the previous window/pixmap
	   origin (oldOrg) differs from the new window/pixmap origin
	   (pGC->lastWinOrg)
	*/
	new_rotate = (oldOrg.x != pGC->lastWinOrg.x) ||
					 (oldOrg.y != pGC->lastWinOrg.y);


	devPriv = (afbPrivGCPtr)dixLookupPrivate(&pGC->devPrivates,
						 afbGCPrivateKey);

	/*
		if the client clip is different or moved OR
		the subwindowMode has changed OR
		the window's clip has changed since the last validation
		we need to recompute the composite clip
	*/
	if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
		 (pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS)))
		afbComputeCompositeClip(pGC, pDrawable);

	new_rrop = FALSE;
	new_line = FALSE;
	new_text = FALSE;
	new_fill = FALSE;

	mask = changes;
	while (mask) {
		index = lowbit(mask);
		mask &= ~index;

		/* this switch acculmulates a list of which procedures
		   might have to change due to changes in the GC.  in
		   some cases (e.g. changing one 16 bit tile for another)
		   we might not really need a change, but the code is
		   being paranoid.
		   this sort of batching wins if, for example, the alu
		   and the font have been changed, or any other pair
		   of items that both change the same thing.
		*/
		switch (index) {
			case GCPlaneMask:
			case GCFunction:
			case GCForeground:
				new_rrop = TRUE;
				break;
			case GCBackground:
				new_rrop = TRUE;		/* for opaque stipples */
				break;
			case GCLineStyle:
			case GCLineWidth:
			case GCJoinStyle:
					new_line = TRUE;
					break;
			case GCCapStyle:
				break;
			case GCFillStyle:
				new_fill = TRUE;
				break;
			case GCFillRule:
				break;
			case GCTile:
				if(pGC->tileIsPixel)
					break;
				new_rotate = TRUE;
				new_fill = TRUE;
				break;

			case GCStipple:
				if(pGC->stipple == (PixmapPtr)NULL)
					break;
				new_rotate = TRUE;
				new_fill = TRUE;
				break;

			case GCTileStipXOrigin:
				new_rotate = TRUE;
				break;

			case GCTileStipYOrigin:
				new_rotate = TRUE;
				break;

			case GCFont:
				new_text = TRUE;
				break;
			case GCSubwindowMode:
				break;
			case GCGraphicsExposures:
				break;
			case GCClipXOrigin:
				break;
			case GCClipYOrigin:
				break;
			case GCClipMask:
				break;
			case GCDashOffset:
				break;
			case GCDashList:
				break;
			case GCArcMode:
				break;
			default:
					break;
		}
	}

	/* deal with the changes we've collected .
	   new_rrop must be done first because subsequent things
	   depend on it.
	*/

	if(new_rotate || new_fill) {
		Bool new_pix = FALSE;

		/* figure out how much to rotate */
		xrot = pGC->patOrg.x;
		yrot = pGC->patOrg.y;
		xrot += pDrawable->x;
		yrot += pDrawable->y;

		switch (pGC->fillStyle) {
			case FillTiled:
				/* copy current tile and stipple */
				if (!pGC->tileIsPixel &&
					 (pGC->tile.pixmap->drawable.width <= PPW) &&
					 !(pGC->tile.pixmap->drawable.width &
						(pGC->tile.pixmap->drawable.width - 1))) {
					afbCopyRotatePixmap(pGC->tile.pixmap, &pGC->pRotatedPixmap,
												xrot, yrot);
					new_pix = TRUE;
				}
				break;
			case FillStippled:
			case FillOpaqueStippled:
				if (pGC->stipple && (pGC->stipple->drawable.width <= PPW) &&
					 !(pGC->stipple->drawable.width &
						(pGC->stipple->drawable.width - 1))) {
					afbCopyRotatePixmap(pGC->stipple, &pGC->pRotatedPixmap,
												xrot, yrot);
					new_pix = TRUE;
				}
		}
		/* destroy any previously rotated tile or stipple */
		if (!new_pix && pGC->pRotatedPixmap) {
			(*pDrawable->pScreen->DestroyPixmap)(pGC->pRotatedPixmap);
			pGC->pRotatedPixmap = (PixmapPtr)NULL;
		}
	}

	/*
	 * duck out here when the GC is unchanged
	 */

	if (!changes)
		return;

	if (new_rrop || new_fill) {
		afbReduceRop(pGC->alu, pGC->fgPixel, pGC->planemask, pDrawable->depth,
						  devPriv->rrops);
		afbReduceOpaqueStipple(pGC->fgPixel, pGC->bgPixel, pGC->planemask,
										pGC->depth, devPriv->rropOS);
		new_fill = TRUE;
	}

	if (new_line || new_fill || new_text) {
		if (!pGC->ops->devPrivate.val) {
			pGC->ops = miCreateGCOps(pGC->ops);
			pGC->ops->devPrivate.val = 1;
		}
	}

	if (new_line || new_fill) {
		if (pGC->lineWidth == 0) {
			if (pGC->lineStyle == LineSolid && pGC->fillStyle == FillSolid)
				pGC->ops->PolyArc = afbZeroPolyArcSS;
			else
				pGC->ops->PolyArc = miZeroPolyArc;
		} else
			pGC->ops->PolyArc = miPolyArc;
		if (pGC->lineStyle == LineSolid) {
			if(pGC->lineWidth == 0) {
				if (pGC->fillStyle == FillSolid) {
					pGC->ops->PolySegment = afbSegmentSS;
					pGC->ops->Polylines = afbLineSS;
				}
 				else
				{
					pGC->ops->PolySegment = miPolySegment;
					pGC->ops->Polylines = miZeroLine;
				}
			} else {
				pGC->ops->PolySegment = miPolySegment;
				pGC->ops->Polylines = miWideLine;
			}
		} else {
			if(pGC->lineWidth == 0 && pGC->fillStyle == FillSolid) {
				pGC->ops->PolySegment = afbSegmentSD;
				pGC->ops->Polylines = afbLineSD;
			} else {
				pGC->ops->PolySegment = miPolySegment;
				pGC->ops->Polylines = miWideDash;
			}
		}
	}

	if (new_text || new_fill) {
		if ((pGC->font) &&
			(FONTMAXBOUNDS(pGC->font,rightSideBearing) -
			 FONTMINBOUNDS(pGC->font,leftSideBearing) > 32 ||
			 FONTMINBOUNDS(pGC->font,characterWidth) < 0)) {
			pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
			pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
		} else {
			/* special case ImageGlyphBlt for terminal emulator fonts */
			if ((pGC->font) &&
				TERMINALFONT(pGC->font)) {
				pGC->ops->ImageGlyphBlt = afbTEGlyphBlt;
			} else {
				pGC->ops->ImageGlyphBlt = afbImageGlyphBlt;
			}

			/* now do PolyGlyphBlt */
			if (pGC->fillStyle == FillSolid) {
				pGC->ops->PolyGlyphBlt = afbPolyGlyphBlt;
			} else {
				pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
			}
		}
	}

	if (new_fill) {
		/* install a suitable fillspans and pushpixels */
		pGC->ops->PushPixels = afbPushPixels;
		pGC->ops->FillPolygon = miFillPolygon;
		pGC->ops->PolyFillArc = miPolyFillArc;

		switch (pGC->fillStyle) {
			case FillSolid:
				pGC->ops->FillSpans = afbSolidFS;
				pGC->ops->FillPolygon = afbFillPolygonSolid;
				pGC->ops->PolyFillArc = afbPolyFillArcSolid;
				break;
			case FillTiled:
				if (pGC->pRotatedPixmap)
					pGC->ops->FillSpans = afbTileFS;
				else
					pGC->ops->FillSpans = afbUnnaturalTileFS;
				break;
			case FillOpaqueStippled:
				if (pGC->pRotatedPixmap)
					pGC->ops->FillSpans = afbOpaqueStippleFS;
				else
					pGC->ops->FillSpans = afbUnnaturalOpaqueStippleFS;
				break;

			case FillStippled:
				if (pGC->pRotatedPixmap)
					pGC->ops->FillSpans = afbStippleFS;
				else
					pGC->ops->FillSpans = afbUnnaturalStippleFS;
				break;
		}
	} /* end of new_fill */
}

static void
afbDestroyGC(pGC)
	GCPtr pGC;
{
	if (pGC->pRotatedPixmap)
		(*pGC->pScreen->DestroyPixmap)(pGC->pRotatedPixmap);
	if (pGC->freeCompClip)
		REGION_DESTROY(pGC->pScreen, pGC->pCompositeClip);
	miDestroyGCOps(pGC->ops);
}

void
afbReduceRop(alu, src, planemask, depth, rop)
	register int alu;
	register Pixel src;
	register unsigned long planemask;
	int depth;
	register unsigned char *rop;
{
	register int d;
	register Pixel mask = 1;

	for (d = 0; d < depth; d++, mask <<= 1) {
		if (!(planemask & mask))
			rop[d] = RROP_NOP;
		else if ((src & mask) == 0)		/* src is black */
			switch (alu) {
				case GXclear:
					rop[d] = RROP_BLACK;
					break;
				case GXand:
					rop[d] = RROP_BLACK;
					break;
				case GXandReverse:
					rop[d] = RROP_BLACK;
					break;
				case GXcopy:
					rop[d] = RROP_BLACK;
					break;
				case GXandInverted:
					rop[d] = RROP_NOP;
					break;
				case GXnoop:
					rop[d] = RROP_NOP;
					break;
				case GXxor:
					rop[d] = RROP_NOP;
					break;
				case GXor:
					rop[d] = RROP_NOP;
					break;
				case GXnor:
					rop[d] = RROP_INVERT;
					break;
				case GXequiv:
					rop[d] = RROP_INVERT;
					break;
				case GXinvert:
					rop[d] = RROP_INVERT;
					break;
				case GXorReverse:
					rop[d] = RROP_INVERT;
					break;
				case GXcopyInverted:
					rop[d] = RROP_WHITE;
					break;
				case GXorInverted:
					rop[d] = RROP_WHITE;
					break;
				case GXnand:
					rop[d] = RROP_WHITE;
					break;
				case GXset:
					rop[d] = RROP_WHITE;
					break;
			}
		else /* src is white */
			switch (alu) {
				case GXclear:
					rop[d] = RROP_BLACK;
					break;
				case GXand:
					rop[d] = RROP_NOP;
					break;
				case GXandReverse:
					rop[d] = RROP_INVERT;
					break;
				case GXcopy:
					rop[d] = RROP_WHITE;
					break;
				case GXandInverted:
					rop[d] = RROP_BLACK;
					break;
				case GXnoop:
					rop[d] = RROP_NOP;
					break;
				case GXxor:
					rop[d] = RROP_INVERT;
					break;
				case GXor:
					rop[d] = RROP_WHITE;
					break;
				case GXnor:
					rop[d] = RROP_BLACK;
					break;
				case GXequiv:
					rop[d] = RROP_NOP;
					break;
				case GXinvert:
					rop[d] = RROP_INVERT;
					break;
				case GXorReverse:
					rop[d] = RROP_WHITE;
					break;
				case GXcopyInverted:
					rop[d] = RROP_BLACK;
					break;
				case GXorInverted:
					rop[d] = RROP_NOP;
					break;
				case GXnand:
					rop[d] = RROP_INVERT;
					break;
				case GXset:
					rop[d] = RROP_WHITE;
					break;
			}
	}
}
