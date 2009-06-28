/*
 * Copyright © 2003 Eric Anholt
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

#include <sys/io.h>

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif

#include "sis.h"
#include "sis_reg.h"

#if 0
#define SIS_FALLBACK(x)		\
do {				\
	ErrorF x;		\
	return FALSE;		\
} while (0)
#else
#define SIS_FALLBACK(x) return FALSE
#endif

CARD8 SiSSolidRop[16] = {
    /* GXclear      */      0x00,         /* 0 */
    /* GXand        */      0xa0,         /* src AND dst */
    /* GXandReverse */      0x50,         /* src AND NOT dst */
    /* GXcopy       */      0xf0,         /* src */
    /* GXandInverted*/      0x0a,         /* NOT src AND dst */
    /* GXnoop       */      0xaa,         /* dst */
    /* GXxor        */      0x5a,         /* src XOR dst */
    /* GXor         */      0xfa,         /* src OR dst */
    /* GXnor        */      0x05,         /* NOT src AND NOT dst */
    /* GXequiv      */      0xa5,         /* NOT src XOR dst */
    /* GXinvert     */      0x55,         /* NOT dst */
    /* GXorReverse  */      0xf5,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x0f,         /* NOT src */
    /* GXorInverted */      0xaf,         /* NOT src OR dst */
    /* GXnand       */      0x5f,         /* NOT src OR NOT dst */
    /* GXset        */      0xff,         /* 1 */
};

CARD8 SiSBltRop[16] = {
    /* GXclear      */      0x00,         /* 0 */
    /* GXand        */      0x88,         /* src AND dst */
    /* GXandReverse */      0x44,         /* src AND NOT dst */
    /* GXcopy       */      0xcc,         /* src */
    /* GXandInverted*/      0x22,         /* NOT src AND dst */
    /* GXnoop       */      0xaa,         /* dst */
    /* GXxor        */      0x66,         /* src XOR dst */
    /* GXor         */      0xee,         /* src OR dst */
    /* GXnor        */      0x11,         /* NOT src AND NOT dst */
    /* GXequiv      */      0x99,         /* NOT src XOR dst */
    /* GXinvert     */      0x55,         /* NOT dst */
    /* GXorReverse  */      0xdd,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x33,         /* NOT src */
    /* GXorInverted */      0xbb,         /* NOT src OR dst */
    /* GXnand       */      0x77,         /* NOT src OR NOT dst */
    /* GXset        */      0xff,         /* 1 */
};

int copydx, copydy;
int fifo_size;
SiSScreenInfo *accel_siss;
char *mmio;
CARD32 sis_color = 0;
CARD32 blitCmd;

static void
SiSWaitAvailMMIO(int n)
{
	while (fifo_size < n) {
		fifo_size = MMIO_IN32(mmio, REG_CommandQueue) & MASK_QueueLen;
	}
	fifo_size -= n;
}

static void
SiSWaitIdle(void)
{
	CARD32 engineState;
	do {
		engineState = MMIO_IN32(mmio, REG_CommandQueue);
	} while ((engineState & SiS_EngIdle) != SiS_EngIdle);
}

static Bool
SiSPrepareSolid(PixmapPtr pPixmap, int alu, Pixel pm, Pixel fg)
{
	KdScreenPriv(pPixmap->drawable.pScreen);
	SiSScreenInfo(pScreenPriv);
	SiSCardInfo(pScreenPriv);

	/* No acceleration for other formats (yet) */
	if (pPixmap->drawable.bitsPerPixel !=
	    pScreenPriv->screen->fb[0].bitsPerPixel)
		return FALSE;

	if ((pm & 0x00ffffff) != 0x00ffffff)	/* XXX */
		SIS_FALLBACK(("Unsupported planemask 0x%x\n", pm));

	accel_siss = siss;
	mmio = sisc->reg_base;

	SiSWaitAvailMMIO(4);
	MMIO_OUT32(mmio, REG_BLT_PATFG, fg);
	MMIO_OUT32(mmio, REG_BLT_DSTRECT, (-1 << 16) | pPixmap->devKind);
	MMIO_OUT32(mmio, REG_BLT_SRCPITCH, siss->depthSet);
	MMIO_OUT32(mmio, REG_BLT_DSTBASE, ((CARD8 *)pPixmap->devPrivate.ptr -
	    pScreenPriv->screen->memory_base));

	blitCmd = BLT_CMD_BITBLT | BLT_PAT_FG | BLT_X_INC | BLT_Y_INC |
	    BLT_NOCLIP | (SiSSolidRop[alu] << 8);

	return TRUE;
}

static void
SiSSolid(int x1, int y1, int x2, int y2)
{
	SiSWaitAvailMMIO(3);
	MMIO_OUT32(mmio, REG_BLT_DSTXY, (x1 << 16) | y1);
	MMIO_OUT32(mmio, REG_BLT_H_W, ((y2 - y1) << 16) | (x2 - x1));
	MMIO_OUT32(mmio, REG_BLT_CMD, blitCmd);
}

static void
SiSDoneSolid(void)
{
}

static Bool
SiSPrepareCopy(PixmapPtr pSrc, PixmapPtr pDst, int dx, int dy, int alu,
    Pixel pm)
{
	KdScreenPriv(pDst->drawable.pScreen);
	SiSScreenInfo(pScreenPriv);
	SiSCardInfo(pScreenPriv);

	/* No acceleration for other formats (yet) */
	if (pDst->drawable.bitsPerPixel !=
	    pScreenPriv->screen->fb[0].bitsPerPixel)
		return FALSE;

	if ((pm & 0x00ffffff) != 0x00ffffff)	/* XXX */
		SIS_FALLBACK(("Unsupported pixel mask 0x%x\n", pm));

	accel_siss = siss;
	mmio = sisc->reg_base;

	SiSWaitAvailMMIO(4);
	MMIO_OUT32(mmio, REG_BLT_SRCPITCH, siss->depthSet | pSrc->devKind);
	MMIO_OUT32(mmio, REG_BLT_DSTRECT, (-1 << 16) | pDst->devKind);
	MMIO_OUT32(mmio, REG_BLT_SRCBASE, ((CARD8 *)pSrc->devPrivate.ptr -
	    pScreenPriv->screen->memory_base));
	MMIO_OUT32(mmio, REG_BLT_DSTBASE, ((CARD8 *)pDst->devPrivate.ptr -
	    pScreenPriv->screen->memory_base));

	blitCmd = BLT_CMD_BITBLT | BLT_PAT_FG | BLT_NOCLIP |
	    (SiSBltRop[alu] << 8);

	if (pSrc != pDst || dx >= 0)
		blitCmd |= BLT_X_INC;
	if (pSrc != pDst || dy >= 0)
		blitCmd |= BLT_Y_INC;

	return TRUE;
}

static void
SiSCopy(int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	if (!(blitCmd & BLT_X_INC)) {
		srcX += w - 1;
		dstX += w - 1;
	}

	if (!(blitCmd & BLT_Y_INC)) {
		srcY += h - 1;
		dstY += h - 1;
	}

	SiSWaitAvailMMIO(4);
	MMIO_OUT32(mmio, REG_BLT_H_W, (h << 16) | w);
	MMIO_OUT32(mmio, REG_BLT_SRCXY, (srcX << 16) | srcY);
	MMIO_OUT32(mmio, REG_BLT_DSTXY, (dstX << 16) | dstY);
	MMIO_OUT32(mmio, REG_BLT_CMD, blitCmd);
}

static void
SiSDoneCopy(void)
{
}

KaaScreenInfoRec SiSKaa = {
	SiSPrepareSolid,
	SiSSolid,
	SiSDoneSolid,
	SiSPrepareCopy,
	SiSCopy,
	SiSDoneCopy,
	KAA_OFFSCREEN_PIXMAPS,
	8,
	8
};

#define USE_TURBOQUEUE 0

Bool
SiSDrawInit(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	SiSScreenInfo(pScreenPriv);
	CARD8 tmp;
#if USE_TURBOQUEUE
	int tqsize;
#endif

	switch (pScreenPriv->screen->fb[0].depth)
	{
	case 8:
		siss->depthSet = 0x00000000;
		break;
	case 15:
		siss->depthSet = 0x40000000;
		break;
	case 16:
		siss->depthSet = 0x80000000;
		break;
	case 24:
		if (pScreenPriv->screen->fb[0].bitsPerPixel == 32) {
			siss->depthSet = 0xc0000000;
			break;
		}
		/* FALLTHROUGH*/
	default:
		ErrorF("Unsupported depth/bpp %d/%d\n",
		    pScreenPriv->screen->fb[0].depth,
		    pScreenPriv->screen->fb[0].bitsPerPixel);
		return FALSE;
	}

	outb(0x05, 0x3c4);
	outb(0x86, 0x3c5); /* unlock registers */

	outb(0x20, 0x3c4);
	outb(0xA1, 0x3c5); /* enable pci linear addressing, MMIO, PCI_IO */

	outb(0x1e, 0x3c4);
	tmp = inb(0x3c5);
	outb(tmp | 0x42 | 0x18, 0x3c5); /* Enable 2d and 3d */

#if USE_TURBOQUEUE
	tqsize = (pScreenPriv->screen->memory_size / 1024) / 64 - 8;
	/* Enable TQ */
	outb(0x26, 0x3c4);
	outb(tqsize & 0xff, 0x3c5);
	outb(0x27, 0x3c4);
	tmp = inb(0x3c5);
	outb(((tqsize >> 8) & 0x03) | (tmp & 0x0c) | 0xF0, 0x3c5);
	
	/* XXX: Adjust offscreen size to avoid TQ area (last 512k) */
#endif

	ErrorF("Screen: %d/%d depth/bpp\n", pScreenPriv->screen->fb[0].depth,
	    pScreenPriv->screen->fb[0].bitsPerPixel);

	if (!kaaDrawInit(pScreen, &SiSKaa))
		return FALSE;

	return TRUE;
}

void
SiSDrawEnable(ScreenPtr pScreen)
{
	KdMarkSync(pScreen);
}

void
SiSDrawDisable(ScreenPtr pScreen)
{
}

void
SiSDrawFini(ScreenPtr pScreen)
{
	kaaDrawFini (pScreen);
}

void
SiSDrawSync(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	SiSScreenInfo(pScreenPriv);
	SiSCardInfo(pScreenPriv);

	accel_siss = siss;
	mmio = sisc->reg_base;

	SiSWaitIdle();
}
