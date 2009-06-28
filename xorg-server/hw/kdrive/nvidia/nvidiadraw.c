/*
 * Copyright © 2003 Keith Packard
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
#include "nvidia.h"
#include "nvidiadraw.h"

#include	<X11/Xmd.h>
#include	"gcstruct.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"mistruct.h"
#include	"dixfontstr.h"
#include	"fb.h"
#include	"migc.h"
#include	"miline.h"
#include	"picturestr.h"
#include	"kaa.h"

CARD8 nvidiaRop[16] = {
    /* GXclear      */      0x01,         /* 0 */
    /* GXand        */      0x0c,         /* src AND dst */
    /* GXandReverse */      0x0d,         /* src AND NOT dst */
    /* GXcopy       */      0x07,         /* src */
    /* GXandInverted*/      0x0e,         /* NOT src AND dst */
    /* GXnoop       */      0x03,         /* dst */
    /* GXxor        */      0x05,         /* src XOR dst */
    /* GXor         */      0x0b,         /* src OR dst */
    /* GXnor        */      0x0f,         /* NOT src AND NOT dst */
    /* GXequiv      */      0x06,         /* NOT src XOR dst */
    /* GXinvert     */      0x00,         /* NOT dst */
    /* GXorReverse  */      0x0a,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x04,         /* NOT src */
    /* GXorInverted */      0x09,         /* NOT src OR dst */
    /* GXnand       */      0x08,         /* NOT src OR NOT dst */
    /* GXset        */      0x02,         /* 1 */
};

static NvidiaCardInfo	*card;

void
nvidiaWait (NvidiaCardInfo *card, NvidiaFifoFree *free, int n)
{
    while (card->fifo_free < n)
    {
	card->fifo_free = free->FifoFree >> 2;
    }
    card->fifo_free -= n;
}

void
nvidiaWaitIdle (NvidiaCardInfo *card)
{
    while (card->fifo_free < card->fifo_size || (card->busy->busy & 1))
    {
	card->fifo_free = card->rop->FifoFree.FifoFree >> 2;
    }
}

static void
nvidiaWaitMarker (ScreenPtr pScreen, int marker)
{
    KdScreenPriv(pScreen);
    nvidiaCardInfo(pScreenPriv);
    
    nvidiaWaitIdle (nvidiac);
}

static Bool
nvidiaPrepareSolid (PixmapPtr   pPixmap,
		    int		alu,
		    Pixel	pm,
		    Pixel	fg)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    KdScreenPriv(pScreen);
    nvidiaCardInfo(pScreenPriv);
    
    card = nvidiac;
    if (~pm & FbFullMask(pPixmap->drawable.depth))
	return FALSE;
    nvidiaWait (nvidiac, &nvidiac->rop->FifoFree, 1);
    nvidiac->rop->Rop3 = nvidiaRop[alu];
    nvidiaWait (nvidiac, &nvidiac->rect->FifoFree, 1);
    nvidiac->rect->Color1A = fg;
    return TRUE;
}

static void
nvidiaSolid (int x1, int y1, int x2, int y2)
{
    nvidiaWait (card, &card->rect->FifoFree, 2);
    card->rect->TopLeft = NVIDIA_XY(x1,y1);
    card->rect->WidthHeight = NVIDIA_XY(x2-x1,y2-y1);
}

static void
nvidiaDoneSolid (void)
{
}


static Bool
nvidiaPrepareCopy (PixmapPtr	pSrcPixmap,
		   PixmapPtr	pDstPixmap,
		   int		dx,
		   int		dy,
		   int		alu,
		   Pixel	pm)
{
    ScreenPtr pScreen = pDstPixmap->drawable.pScreen;
    KdScreenPriv(pScreen);
    nvidiaCardInfo(pScreenPriv);
    
    card = nvidiac;
    if (~pm & FbFullMask(pDstPixmap->drawable.depth))
	return FALSE;
    nvidiaWait (nvidiac, &card->rop->FifoFree, 1);
    nvidiac->rop->Rop3 = nvidiaRop[alu];
    return TRUE;
}

static void
nvidiaCopy (int srcX,
	    int srcY,
	    int dstX,
	    int dstY,
	    int w,
	    int h)
{
    nvidiaWait (card, &card->blt->FifoFree, 3);
    card->blt->TopLeftSrc = NVIDIA_XY(srcX, srcY);
    card->blt->TopLeftDst = NVIDIA_XY(dstX, dstY);
    card->blt->WidthHeight = NVIDIA_XY(w, h);
}

static void
nvidiaDoneCopy (void)
{
}

Bool
nvidiaDrawInit (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    nvidiaCardInfo(pScreenPriv);
    nvidiaScreenInfo(pScreenPriv);
    Bool    ret = TRUE;
    
    ENTER ();
    if (pScreenPriv->screen->fb[0].depth == 4)
	ret = FALSE;
    
    memset(&nvidias->kaa, 0, sizeof(KaaScreenInfoRec));
    nvidias->kaa.waitMarker	= nvidiaWaitMarker;
    nvidias->kaa.PrepareSolid	= nvidiaPrepareSolid;
    nvidias->kaa.Solid		= nvidiaSolid;
    nvidias->kaa.DoneSolid	= nvidiaDoneSolid;
    nvidias->kaa.PrepareCopy	= nvidiaPrepareCopy;
    nvidias->kaa.Copy		= nvidiaCopy;
    nvidias->kaa.DoneCopy	= nvidiaDoneCopy;

    if (ret && !nvidiac->rop)
    {
	ErrorF ("Failed to map fifo registers\n");
	ret = FALSE;
    }
    if (ret && !nvidiac->rop->FifoFree.FifoFree)
    {
	ErrorF ("Fifo appears broken\n");
	ret = FALSE;
    }
    if (ret && !kaaDrawInit (pScreen, &nvidias->kaa))
    {
	ErrorF ("kaaDrawInit failed\n");
	ret = FALSE;
    }

    LEAVE ();
    return ret;
}

#define PIX_FORMAT_MONO	0
#define PIX_FORMAT_PSEUDO_8	2
#define PIX_FORMAT_TRUE_1555	3
#define PIX_FORMAT_TRUE_565	4
#define PIX_FORMAT_TRUE_8888    6
#define PIX_FORMAT_TRUE_332	7
#define PIX_FORMAT_GRAY_8	8
#define PIX_FORMAT_YUV_422	0xb
#define PIX_FORMAT_YUV_444	0xe
#define PIX_FORMAT_TRUE_4444	0xf

void
nvidiaDrawEnable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    nvidiaCardInfo(pScreenPriv);
    
    ENTER ();
    nvidiac->fifo_size = nvidiac->rop->FifoFree.FifoFree;
    nvidiac->fifo_free = 0;
    kaaMarkSync (pScreen);
    LEAVE ();
}

void
nvidiaDrawDisable (ScreenPtr pScreen)
{
}

void
nvidiaDrawFini (ScreenPtr pScreen)
{
}

