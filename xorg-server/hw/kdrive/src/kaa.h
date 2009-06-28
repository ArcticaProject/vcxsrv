/*
 * Copyright © 2001 Keith Packard
 *
 * Partly based on code that is Copyright © The XFree86 Project Inc.
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
 
#ifndef _KAA_H_
#define _KAA_H_

#include "picturestr.h"

#define KaaGetScreenPriv(s) ((KaaScreenPrivPtr) \
    dixLookupPrivate(&(s)->devPrivates, kaaScreenPrivateKey))
#define KaaScreenPriv(s)	KaaScreenPrivPtr    pKaaScr = KaaGetScreenPriv(s)

#define KaaGetPixmapPriv(p) ((KaaPixmapPrivPtr) \
    dixLookupPrivate(&(p)->devPrivates, kaaPixmapPrivateKey))
#define KaaSetPixmapPriv(p,a) \
    dixSetPrivate(&(p)->devPrivates, kaaPixmapPrivateKey, a)
#define KaaPixmapPriv(p)	KaaPixmapPrivPtr pKaaPixmap = KaaGetPixmapPriv(p)

typedef struct {
    KaaScreenInfoPtr info;
} KaaScreenPrivRec, *KaaScreenPrivPtr;

typedef struct {
    KdOffscreenArea *area;
    int		    score;
    int		    devKind;
    DevUnion	    devPrivate;
    Bool	    dirty;
} KaaPixmapPrivRec, *KaaPixmapPrivPtr;

extern DevPrivateKey kaaScreenPrivateKey;
extern DevPrivateKey kaaPixmapPrivateKey;


void
kaaPixmapUseScreen (PixmapPtr pPixmap);

void
kaaPixmapUseMemory (PixmapPtr pPixmap);

void
kaaDrawableDirty(DrawablePtr pDrawable);

Bool
kaaDrawableIsOffscreen (DrawablePtr pDrawable);

Bool
kaaPixmapIsOffscreen(PixmapPtr p);

PixmapPtr
kaaGetOffscreenPixmap (DrawablePtr pDrawable, int *xp, int *yp);

void
kaaMoveInPixmap (PixmapPtr pPixmap);

void
kaaMarkSync (ScreenPtr pScreen);

void
kaaWaitSync (ScreenPtr pScreen);

void
kaaCopyNtoN (DrawablePtr    pSrcDrawable,
	     DrawablePtr    pDstDrawable,
	     GCPtr	    pGC,
	     BoxPtr	    pbox,
	     int	    nbox,
	     int	    dx,
	     int	    dy,
	     Bool	    reverse,
	     Bool	    upsidedown,
	     Pixel	    bitplane,
	     void	    *closure);

void
kaaComposite(CARD8	op,
	     PicturePtr pSrc,
	     PicturePtr pMask,
	     PicturePtr pDst,
	     INT16	xSrc,
	     INT16	ySrc,
	     INT16	xMask,
	     INT16	yMask,
	     INT16	xDst,
	     INT16	yDst,
	     CARD16	width,
	     CARD16	height);

void
kaaRasterizeTrapezoid(PicturePtr pPict,
		      xTrapezoid *trap,
		      int xoff,
		      int yoff);

void
kaaInitTrapOffsets(int grid_order, float *x_offsets, float *y_offsets,
		   float x_offset, float y_offset);

#endif /* _KAA_H_ */
