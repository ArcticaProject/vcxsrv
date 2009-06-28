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

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "kaa.h"
#include "picturestr.h"
#include "mipict.h"
#include "fbpict.h"

/*
 * These functions wrap the low-level fb rendering functions and
 * synchronize framebuffer/accelerated drawing by stalling until
 * the accelerator is idle
 */

void
KdCheckFillSpans  (DrawablePtr pDrawable, GCPtr pGC, int nspans,
		   DDXPointPtr ppt, int *pwidth, int fSorted)
{
    kaaWaitSync (pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbFillSpans (pDrawable, pGC, nspans, ppt, pwidth, fSorted);
}

void
KdCheckSetSpans (DrawablePtr pDrawable, GCPtr pGC, char *psrc,
		 DDXPointPtr ppt, int *pwidth, int nspans, int fSorted)
{
    kaaWaitSync (pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbSetSpans (pDrawable, pGC, psrc, ppt, pwidth, nspans, fSorted);
}

void
KdCheckPutImage (DrawablePtr pDrawable, GCPtr pGC, int depth,
		 int x, int y, int w, int h, int leftPad, int format,
		 char *bits)
{
    kaaWaitSync (pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbPutImage (pDrawable, pGC, depth, x, y, w, h, leftPad, format, bits);
}

RegionPtr
KdCheckCopyArea (DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
		 int srcx, int srcy, int w, int h, int dstx, int dsty)
{
    kaaWaitSync (pSrc->pScreen);
    kaaDrawableDirty (pDst);
    return fbCopyArea (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty);
}

RegionPtr
KdCheckCopyPlane (DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
		  int srcx, int srcy, int w, int h, int dstx, int dsty,
		  unsigned long bitPlane)
{
    kaaWaitSync (pSrc->pScreen);
    kaaDrawableDirty (pDst);
    return fbCopyPlane (pSrc, pDst, pGC, srcx, srcy, w, h, dstx, dsty,
			bitPlane);
}

void
KdCheckPolyPoint (DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
		  DDXPointPtr pptInit)
{
    kaaWaitSync (pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbPolyPoint (pDrawable, pGC, mode, npt, pptInit);
}

void
KdCheckPolylines (DrawablePtr pDrawable, GCPtr pGC,
		  int mode, int npt, DDXPointPtr ppt)
{

    if (pGC->lineWidth == 0) {
	kaaWaitSync(pDrawable->pScreen);
	kaaDrawableDirty (pDrawable);
    }
    kaaDrawableDirty (pDrawable);
    fbPolyLine (pDrawable, pGC, mode, npt, ppt);
}

void
KdCheckPolySegment (DrawablePtr pDrawable, GCPtr pGC, 
		    int nsegInit, xSegment *pSegInit)
{
    if (pGC->lineWidth == 0) {
	kaaWaitSync(pDrawable->pScreen);
	kaaDrawableDirty (pDrawable);
    }
    kaaDrawableDirty (pDrawable);
    fbPolySegment (pDrawable, pGC, nsegInit, pSegInit);
}

void
KdCheckPolyRectangle (DrawablePtr pDrawable, GCPtr pGC, 
		      int nrects, xRectangle *prect)
{
    if (pGC->lineWidth == 0) {
	kaaWaitSync(pDrawable->pScreen);
	kaaDrawableDirty (pDrawable);
    }
    fbPolyRectangle (pDrawable, pGC, nrects, prect);
}

void
KdCheckPolyArc (DrawablePtr pDrawable, GCPtr pGC, 
		int narcs, xArc *pArcs)
{
    if (pGC->lineWidth == 0)
    {
	kaaWaitSync(pDrawable->pScreen);
	kaaDrawableDirty (pDrawable);
	fbPolyArc (pDrawable, pGC, narcs, pArcs);
    }
    else
	miPolyArc (pDrawable, pGC, narcs, pArcs);
}

#if 0
void
KdCheckFillPolygon (DrawablePtr pDrawable, GCPtr pGC, 
		    int shape, int mode, int count, DDXPointPtr pPts)
{
    kaaWaitSync(pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbFillPolygon (pDrawable, pGC, mode, count, pPts);
}
#endif

void
KdCheckPolyFillRect (DrawablePtr pDrawable, GCPtr pGC,
		     int nrect, xRectangle *prect)
{
    kaaWaitSync(pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbPolyFillRect (pDrawable, pGC, nrect, prect);
}

void
KdCheckPolyFillArc (DrawablePtr pDrawable, GCPtr pGC, 
		    int narcs, xArc *pArcs)
{
    kaaWaitSync(pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbPolyFillArc (pDrawable, pGC, narcs, pArcs);
}

void
KdCheckImageGlyphBlt (DrawablePtr pDrawable, GCPtr pGC,
		      int x, int y, unsigned int nglyph,
		      CharInfoPtr *ppci, pointer pglyphBase)
{
    kaaWaitSync(pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbImageGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}

void
KdCheckPolyGlyphBlt (DrawablePtr pDrawable, GCPtr pGC,
		     int x, int y, unsigned int nglyph,
		     CharInfoPtr *ppci, pointer pglyphBase)
{
    kaaWaitSync(pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbPolyGlyphBlt (pDrawable, pGC, x, y, nglyph, ppci, pglyphBase);
}

void
KdCheckPushPixels (GCPtr pGC, PixmapPtr pBitmap,
		   DrawablePtr pDrawable,
		   int w, int h, int x, int y)
{
    kaaWaitSync(pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbPushPixels (pGC, pBitmap, pDrawable, w, h, x, y);
}

void
KdCheckGetImage (DrawablePtr pDrawable,
		 int x, int y, int w, int h,
		 unsigned int format, unsigned long planeMask,
		 char *d)
{
    kaaWaitSync(pDrawable->pScreen);
    fbGetImage (pDrawable, x, y, w, h, format, planeMask, d);
}

void
KdCheckGetSpans (DrawablePtr pDrawable,
		 int wMax,
		 DDXPointPtr ppt,
		 int *pwidth,
		 int nspans,
		 char *pdstStart)
{
    kaaWaitSync(pDrawable->pScreen);
    fbGetSpans (pDrawable, wMax, ppt, pwidth, nspans, pdstStart);
}

void
KdCheckCopyWindow (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    kaaWaitSync (pWin->drawable.pScreen);
    kaaDrawableDirty ((DrawablePtr)pWin);
    fbCopyWindow (pWin, ptOldOrg, prgnSrc);
}

#if KD_MAX_FB > 1
void
KdCheckPaintKey(DrawablePtr  pDrawable,
		RegionPtr    pRegion,
		CARD32       pixel,
		int          layer)
{
    kaaWaitSync (pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    fbOverlayPaintKey (pDrawable,  pRegion, pixel, layer);
}

void
KdCheckOverlayCopyWindow  (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    kaaWaitSync (pWin->drawable.pScreen);
    kaaDrawableDirty ((DrawablePtr)pWin);
    fbOverlayCopyWindow (pWin, ptOldOrg, prgnSrc);
}
#endif

void
KdScreenInitAsync (ScreenPtr pScreen)
{
    pScreen->GetImage = KdCheckGetImage;
    pScreen->GetSpans = KdCheckGetSpans;
    pScreen->CopyWindow = KdCheckCopyWindow;
#ifdef RENDER
    KdPictureInitAsync (pScreen);
#endif
}

void
KdCheckComposite (CARD8      op,
		  PicturePtr pSrc,
		  PicturePtr pMask,
		  PicturePtr pDst,
		  INT16      xSrc,
		  INT16      ySrc,
		  INT16      xMask,
		  INT16      yMask,
		  INT16      xDst,
		  INT16      yDst,
		  CARD16     width,
		  CARD16     height)
{
    kaaWaitSync (pDst->pDrawable->pScreen);
    kaaDrawableDirty (pDst->pDrawable);
    fbComposite (op,
		 pSrc,
		 pMask,
		 pDst,
		 xSrc,
		 ySrc,
		 xMask,
		 yMask,
		 xDst,
		 yDst,
		 width,
		 height);
}

void
KdCheckRasterizeTrapezoid(PicturePtr	pMask,
			  xTrapezoid	*trap,
			  int		x_off,
			  int		y_off)
{
    kaaWaitSync (pMask->pDrawable->pScreen);
    kaaDrawableDirty (pMask->pDrawable);
    fbRasterizeTrapezoid (pMask, trap, x_off, y_off);
}

/*
 * Only need to stall for copyarea/copyplane
 */
const GCOps kdAsyncPixmapGCOps = {
    fbFillSpans,
    fbSetSpans,
    fbPutImage,
    KdCheckCopyArea,
    KdCheckCopyPlane,
    fbPolyPoint,
    fbPolyLine,
    fbPolySegment,
    fbPolyRectangle,
    fbPolyArc,
    fbFillPolygon,
    fbPolyFillRect,
    fbPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    fbImageGlyphBlt,
    fbPolyGlyphBlt,
    fbPushPixels
};

void
KdPictureInitAsync (ScreenPtr pScreen)
{
    PictureScreenPtr    ps;

    ps = GetPictureScreen(pScreen);
    ps->Composite = KdCheckComposite;
    ps->RasterizeTrapezoid = KdCheckRasterizeTrapezoid;
}
