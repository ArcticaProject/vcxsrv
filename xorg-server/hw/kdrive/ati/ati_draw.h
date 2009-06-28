/*
 * Copyright © 2004 Eric Anholt
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Eric Anholt not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Eric Anholt makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ERIC ANHOLT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ERIC ANHOLT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _ATI_DRAW_H_
#define _ATI_DRAW_H_

Bool ATIGetOffsetPitch(ATIScreenInfo *atis, int bpp, CARD32 *pitch_offset,
    int offset, int pitch);
Bool ATIGetPixmapOffsetPitch(PixmapPtr pPix, CARD32 *pitch_offset);

Bool R128CheckComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture);
Bool R128PrepareComposite(int op, PicturePtr pSrcPicture,
    PicturePtr pMaskPicture, PicturePtr pDstPicture, PixmapPtr pSrc,
    PixmapPtr pMask, PixmapPtr pDst);
void R128Composite(int srcX, int srcY, int maskX, int maskY, int dstX, int dstY,
    int w, int h);
void R128DoneComposite(void);

Bool R128PrepareTrapezoids(PicturePtr pDstPicture, PixmapPtr pDst);
void R128Trapezoids(KaaTrapezoid *traps, int ntraps);
void R128DoneTrapezoids(void);

Bool R100CheckComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture);
Bool R100PrepareComposite(int op, PicturePtr pSrcPicture,
    PicturePtr pMaskPicture, PicturePtr pDstPicture, PixmapPtr pSrc,
    PixmapPtr pMask, PixmapPtr pDst);
Bool R200CheckComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture);
Bool R200PrepareComposite(int op, PicturePtr pSrcPicture,
    PicturePtr pMaskPicture, PicturePtr pDstPicture, PixmapPtr pSrc,
    PixmapPtr pMask, PixmapPtr pDst);
void RadeonComposite(int srcX, int srcY, int maskX, int maskY, int dstX,
    int dstY, int w, int h);
void RadeonDoneComposite(void);

Bool RadeonPrepareTrapezoids(PicturePtr pDstPicture, PixmapPtr pDst);
void RadeonTrapezoids(KaaTrapezoid *traps, int ntraps);
void RadeonDoneTrapezoids(void);

void RadeonSwitchTo2D(ATIScreenInfo *atis);
void RadeonSwitchTo3D(ATIScreenInfo *atis);
void ATIWaitIdle(ATIScreenInfo *atis);

#define ATI_TRACE_FALL 0
#define ATI_TRACE_DRAW 0

#if ATI_TRACE_FALL
#define ATI_FALLBACK(x)			\
do {					\
	ErrorF("%s: ", __FUNCTION__);	\
	ErrorF x;			\
	return FALSE;			\
} while (0)
#else
#define ATI_FALLBACK(x) return FALSE
#endif

#if ATI_TRACE_DRAW
#define ENTER_DRAW(pix) ATIEnterDraw(pix, __FUNCTION__)
#define LEAVE_DRAW(pix) ATILeaveDraw(pix, __FUNCTION__)

void
ATIEnterDraw (PixmapPtr pPixmap, char *function);

void
ATILeaveDraw (PixmapPtr pPixmap, char *function);
#else /* ATI_TRACE */
#define ENTER_DRAW(pix)
#define LEAVE_DRAW(pix)
#endif /* !ATI_TRACE */

#endif /* _ATI_DRAW_H_ */
