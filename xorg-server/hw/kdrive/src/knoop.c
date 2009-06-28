/*
 * Copyright © 1999 Keith Packard
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

/*
 * GC ops that don't do anything
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include <gcstruct.h>

typedef void	(* typeFillSpans)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*nInit*/,
		DDXPointPtr /*pptInit*/,
		int * /*pwidthInit*/,
		int /*fSorted*/
);

typedef void	(* typeSetSpans)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		char * /*psrc*/,
		DDXPointPtr /*ppt*/,
		int * /*pwidth*/,
		int /*nspans*/,
		int /*fSorted*/
);

typedef void	(* typePutImage)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*depth*/,
		int /*x*/,
		int /*y*/,
		int /*w*/,
		int /*h*/,
		int /*leftPad*/,
		int /*format*/,
		char * /*pBits*/
);

typedef RegionPtr	(* typeCopyArea)(
		DrawablePtr /*pSrc*/,
		DrawablePtr /*pDst*/,
		GCPtr /*pGC*/,
		int /*srcx*/,
		int /*srcy*/,
		int /*w*/,
		int /*h*/,
		int /*dstx*/,
		int /*dsty*/
);

typedef RegionPtr	(* typeCopyPlane)(
		DrawablePtr /*pSrcDrawable*/,
		DrawablePtr /*pDstDrawable*/,
		GCPtr /*pGC*/,
		int /*srcx*/,
		int /*srcy*/,
		int /*width*/,
		int /*height*/,
		int /*dstx*/,
		int /*dsty*/,
		unsigned long /*bitPlane*/
);
typedef void	(* typePolyPoint)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*mode*/,
		int /*npt*/,
		DDXPointPtr /*pptInit*/
);

typedef void	(* typePolylines)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*mode*/,
		int /*npt*/,
		DDXPointPtr /*pptInit*/
);

typedef void	(* typePolySegment)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*nseg*/,
		xSegment * /*pSegs*/
);

typedef void	(* typePolyRectangle)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*nrects*/,
		xRectangle * /*pRects*/
);

typedef void	(* typePolyArc)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*narcs*/,
		xArc * /*parcs*/
);

typedef void	(* typeFillPolygon)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*shape*/,
		int /*mode*/,
		int /*count*/,
		DDXPointPtr /*pPts*/
);

typedef void	(* typePolyFillRect)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*nrectFill*/,
		xRectangle * /*prectInit*/
);

typedef void	(* typePolyFillArc)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*narcs*/,
		xArc * /*parcs*/
);

typedef int		(* typePolyText8)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*x*/,
		int /*y*/,
		int /*count*/,
		char * /*chars*/
);

typedef int		(* typePolyText16)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*x*/,
		int /*y*/,
		int /*count*/,
		unsigned short * /*chars*/
);

typedef void	(* typeImageText8)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*x*/,
		int /*y*/,
		int /*count*/,
		char * /*chars*/
);

typedef void	(* typeImageText16)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*x*/,
		int /*y*/,
		int /*count*/,
		unsigned short * /*chars*/
);

typedef void	(* typeImageGlyphBlt)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*x*/,
		int /*y*/,
		unsigned int /*nglyph*/,
		CharInfoPtr * /*ppci*/,
		pointer /*pglyphBase*/
);

typedef void	(* typePolyGlyphBlt)(
		DrawablePtr /*pDrawable*/,
		GCPtr /*pGC*/,
		int /*x*/,
		int /*y*/,
		unsigned int /*nglyph*/,
		CharInfoPtr * /*ppci*/,
		pointer /*pglyphBase*/
);

typedef void	(* typePushPixels)(
		GCPtr /*pGC*/,
		PixmapPtr /*pBitMap*/,
		DrawablePtr /*pDst*/,
		int /*w*/,
		int /*h*/,
		int /*x*/,
		int /*y*/
);

static RegionPtr
KdNoopCopyArea(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GCPtr pGC,
	    int srcx, int srcy, int width, int height, int dstx, int dsty)
{
    return NullRegion;
}

static RegionPtr 
KdNoopCopyPlane(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GCPtr pGC,
		 int srcx, int srcy, int width, int height, 
		 int dstx, int dsty, unsigned long bitPlane)
{
    return NullRegion;
}

GCOps	kdNoopOps = {
    (typeFillSpans)	    NoopDDA,	/* fill spans */
    (typeSetSpans)	    NoopDDA,	/* set spans */
    (typePutImage)	    NoopDDA,	/* put image */
    KdNoopCopyArea,		/* copy area */
    KdNoopCopyPlane,		/* copy plane */
    (typePolyPoint)	    NoopDDA,	/* poly point */
    (typePolylines)	    NoopDDA,	/* poly lines */
    (typePolySegment)   NoopDDA,	/* poly segment */
    (typePolyRectangle) NoopDDA,	/* poly rectangle */
    (typePolyArc)	    NoopDDA,	/* poly arc */
    (typeFillPolygon)   NoopDDA,	/* fill polygon */
    (typePolyFillRect)  NoopDDA,	/* poly fillrect */
    (typePolyFillArc)   NoopDDA,	/* poly fillarc */
    (typePolyText8)	    NoopDDA,	/* text 8 */
    (typePolyText16)    NoopDDA,	/* text 16 */
    (typeImageText8)    NoopDDA,	/* itext 8 */
    (typeImageText16)   NoopDDA,	/* itext 16 */
    (typePolyGlyphBlt)  NoopDDA,	/* glyph blt */
    (typeImageGlyphBlt) NoopDDA,	/* iglyph blt */
    (typePushPixels)    NoopDDA,	/* push pixels */
};
