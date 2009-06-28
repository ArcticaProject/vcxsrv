
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <string.h>

#include <X11/X.h>
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "gcstruct.h"
#include "afb.h"
#include "maskbits.h"
#include "servermd.h"
#include "mfb.h"

void
afbPutImage(pDraw, pGC, depth, x, y, width, height, leftPad, format, pImage)
	DrawablePtr pDraw;
	GCPtr pGC;
	int depth, x, y, width, height;
	int leftPad;
	int format;
	char *pImage;
{
	PixmapPtr pPixmap;

	if ((width == 0) || (height == 0))
		return;

	if (format != ZPixmap || depth == 1 || pDraw->depth == 1) {
		pPixmap = GetScratchPixmapHeader(pDraw->pScreen, width+leftPad, height,
													depth, depth,
													BitmapBytePad(width+leftPad),
													(pointer)pImage);
		if (!pPixmap)
			return;

		pGC->fExpose = FALSE;
		if (format == XYBitmap)
			(void)(*pGC->ops->CopyPlane)((DrawablePtr)pPixmap, pDraw, pGC, leftPad,
												  0, width, height, x, y, 1);
		else {
			(void)(*pGC->ops->CopyArea)((DrawablePtr)pPixmap, pDraw, pGC, leftPad,
												 0, width, height, x, y);
		}

		pGC->fExpose = TRUE;
		FreeScratchPixmapHeader(pPixmap);
	} else {
		/* Chunky to planar conversion required */

		PixmapPtr pPixmap;
		ScreenPtr pScreen = pDraw->pScreen;
		int widthSrc;
		int start_srcshift;
		register int b;
		register int dstshift;
		register int shift_step;
		register PixelType dst;
		register PixelType srcbits;
		register PixelType *pdst;
		register PixelType *psrc;
		int start_bit;
		register int nl;
		register int h;
		register int d;
		int sizeDst;
		PixelType *pdstBase;
		int widthDst;
		int depthDst;

		/* Create a tmp pixmap */
		pPixmap = (pScreen->CreatePixmap)(pScreen, width, height, depth,
						  CREATE_PIXMAP_USAGE_SCRATCH);
		if (!pPixmap)
			return;

		afbGetPixelWidthSizeDepthAndPointer((DrawablePtr)pPixmap, widthDst,
														 sizeDst, depthDst, pdstBase);

		widthSrc = PixmapWidthInPadUnits(width, depth);
		/* XXX: if depth == 8, use fast chunky to planar assembly function.*/
		if (depth > 4) {
			start_srcshift = 24;
			shift_step = 8;
		} else {
			start_srcshift = 28;
			shift_step = 4;
		}

		for (d = 0; d < depth; d++, pdstBase += sizeDst) {	/* @@@ NEXT PLANE @@@ */
			start_bit = start_srcshift + d;
			psrc = (PixelType *)pImage;
			pdst = pdstBase;
			h = height;

			while (h--) {
				dstshift = PPW - 1;
				dst = 0;
				nl = widthSrc;
				while (nl--) {
					srcbits = *psrc++;
					for (b = start_bit; b >= 0; b -= shift_step) {
						dst |= ((srcbits >> b) & 1) << dstshift;
						if (--dstshift < 0) {
							dstshift = PPW - 1;
							*pdst++ = dst;
							dst = 0;
						}
					}
				}
				if (dstshift != PPW - 1)
					*pdst++ = dst;
			}
		} /* for (d = ...) */

		pGC->fExpose = FALSE;
		(void)(*pGC->ops->CopyArea)((DrawablePtr)pPixmap, pDraw, pGC, leftPad, 0,
											 width, height, x, y);
		pGC->fExpose = TRUE;
		(*pScreen->DestroyPixmap)(pPixmap);
	}
}

void
afbGetImage(pDrawable, sx, sy, width, height, format, planemask, pdstLine)
	DrawablePtr pDrawable;
	int sx, sy, width, height;
	unsigned int format;
	unsigned long planemask;
	char *pdstLine;
{
	BoxRec box;
	DDXPointRec ptSrc;
	RegionRec rgnDst;
	ScreenPtr pScreen;
	PixmapPtr pPixmap;

	if ((width == 0) || (height == 0))
		return;

	pScreen = pDrawable->pScreen;
	sx += pDrawable->x;
	sy += pDrawable->y;

	if (format == XYPixmap || pDrawable->depth == 1) {
		pPixmap = GetScratchPixmapHeader(pScreen, width, height, 1, 1,
													BitmapBytePad(width), (pointer)pdstLine);
		if (!pPixmap)
			return;

		ptSrc.x = sx;
		ptSrc.y = sy;
		box.x1 = 0;
		box.y1 = 0;
		box.x2 = width;
		box.y2 = height;
		REGION_INIT(pScreen, &rgnDst, &box, 1);

		pPixmap->drawable.depth = 1;
		pPixmap->drawable.bitsPerPixel = 1;
		/* dix layer only ever calls GetImage with 1 bit set in planemask
		 * when format is XYPixmap.
		 */
		afbDoBitblt(pDrawable, (DrawablePtr)pPixmap, GXcopy, &rgnDst, &ptSrc,
						 planemask);

		FreeScratchPixmapHeader(pPixmap);
		REGION_UNINIT(pScreen, &rgnDst);
	} else {
		/* Planar to chunky conversion required */

		PixelType *psrcBits;
		PixelType *psrcLine;
		PixelType startmask, endmask;
		int depthSrc;
		int widthSrc;
		int sizeSrc;
		int sizeDst;
		int widthDst;
		register PixelType *psrc;
		register PixelType *pdst;
		register PixelType dst;
		register PixelType srcbits;
		register int d;
		register int b;
		register int dstshift;
		register int shift_step;
		register int start_endbit;
		int start_startbit;
		register int end_endbit = 0;
		register int start_dstshift;
		register int nl;
		register int h;
		int nlmiddle;

		widthDst = PixmapWidthInPadUnits(width, pDrawable->depth);
		sizeDst = widthDst * height;

		/* Clear the dest image */
		bzero(pdstLine, sizeDst << 2);

		afbGetPixelWidthSizeDepthAndPointer(pDrawable, widthSrc, sizeSrc,
														 depthSrc, psrcBits);

		psrcBits = afbScanline(psrcBits, sx, sy, widthSrc);

		start_startbit = PPW - 1 - (sx & PIM);
		if ((sx & PIM) + width < PPW) {
			maskpartialbits(sx, width, startmask);
			nlmiddle = 0;
			endmask = 0;
			start_endbit = PPW - ((sx + width) & PIM);
		} else {
			maskbits(sx, width, startmask, endmask, nlmiddle);
			start_endbit = 0;
			end_endbit = PPW - ((sx + width) & PIM);
		}
		/* ZPixmap images have either 4 or 8 bits per pixel dependent on
		 * depth.
		 */
		if (depthSrc > 4) {
			start_dstshift = 24;
			shift_step = 8;
		} else {
			start_dstshift = 28;
			shift_step = 4;
		}
#define SHIFT_BITS(start_bit,end_bit) \
for (b = (start_bit); b >= (end_bit); b--) { \
	dst |= ((srcbits >> b) & 1) << dstshift; \
	if ((dstshift -= shift_step) < 0) { \
		dstshift = start_dstshift + d; \
		*pdst++ = dst; \
		dst = *pdst; \
	} \
} \

		for (d = 0; d < depthSrc; d++, psrcBits += sizeSrc) {	/* @@@ NEXT PLANE @@@ */
			psrcLine = psrcBits;
			pdst = (PixelType *)pdstLine;
			h = height;

			while (h--) {
				psrc = psrcLine;
				psrcLine += widthSrc;
				dst = *pdst;
				dstshift = start_dstshift + d;

				if (startmask) {
					srcbits = *psrc++ & startmask;
					SHIFT_BITS(start_startbit, start_endbit);
				}

				nl = nlmiddle;
				while (nl--) {
					srcbits = *psrc++;
					SHIFT_BITS(PPW - 1, 0);
				}
				if (endmask) {
					srcbits = *psrc & endmask;
					SHIFT_BITS(PPW - 1, end_endbit);
				}

				if (dstshift != start_dstshift + d)
					*pdst++ = dst;
			} /* while (h--) */
		} /* for (d = ...) */
	}
}
