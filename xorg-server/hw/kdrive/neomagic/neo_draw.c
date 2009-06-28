/*
 *
 * Copyright © 2004 Franco Catrin
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Franco Catrin not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Franco Catrin makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * FRANCO CATRIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL FRANCO CATRIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "neomagic.h"

#include <X11/Xmd.h>
#include "gcstruct.h"
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "regionstr.h"
#include "mistruct.h"
#include "dixfontstr.h"
#include "fb.h"
#include "migc.h"
#include "miline.h"
#include "picturestr.h"

NeoMMIO *mmio;
NeoScreenInfo *screen;
NeoCardInfo   *card;
CARD32 fgColor;
CARD32 rop;

CARD32 neoRop[16] = {
    0x000000,    /* GXclear */
    0x080000,    /* GXand */
    0x040000,    /* GXandReverse */
    0x0c0000,    /* GXcopy */
    0x020000,    /* GXandInvert */
    0x0a0000,    /* GXnoop */
    0x060000,    /* GXxor */
    0x0e0000,    /* GXor */
    0x010000,    /* GXnor */
    0x090000,    /* GXequiv */
    0x050000,    /* GXinvert */
    0x0d0000,    /* GXorReverse */
    0x030000,    /* GXcopyInvert */
    0x0b0000,    /* GXorInverted */
    0x070000,    /* GXnand */
    0x0f0000     /* GXset */
};

static  void neoWaitIdle(NeoCardInfo *neoc)
{
    // if MMIO is not working it may halt the machine
    unsigned int i = 0;
    while ((mmio->bltStat & 1) && ++i<100000);
}

static void neoWaitMarker (ScreenPtr pScreen, int marker)
{
    KdScreenPriv(pScreen);
    neoCardInfo(pScreenPriv);

    neoWaitIdle(neoc);
}


static Bool neoPrepareSolid(PixmapPtr pPixmap,
                            int alu,
                            Pixel pm,
                            Pixel fg)
{
    FbBits depthMask = FbFullMask(pPixmap->drawable.depth);
    if ((pm & depthMask) != depthMask) {
        return FALSE;
    } else {
        fgColor = fg;
		if (alu!=3) DBGOUT("used ROP %i\n", alu);
		rop = neoRop[alu];
        return TRUE;
    }
}

static void neoSolid (int x1, int y1, int x2, int y2)
{
    int x, y, w, h;
    x = x1;
    y = y1;
    w = x2-x1;
    h = y2-y1;
	neoWaitIdle(card);
	mmio->fgColor = fgColor;
	mmio->bltCntl =
			NEO_BC3_FIFO_EN      |
			NEO_BC0_SRC_IS_FG    |
			NEO_BC3_SKIP_MAPPING | rop;		
    mmio->dstStart = y * screen->pitch + x * screen->depth;

    mmio->xyExt    = (unsigned long)(h << 16) | (w & 0xffff);
	
}


static void neoDoneSolid(void)
{
}

static Bool neoPrepareCopy (PixmapPtr pSrcPixpam, PixmapPtr pDstPixmap,
                     int dx, int dy, int alu, Pixel pm)
{
	rop = neoRop[alu];
    return TRUE;
}

static void neoCopy (int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	neoWaitIdle(card);
	
    if ((dstY < srcY) || ((dstY == srcY) && (dstX < srcX))) {
		mmio->bltCntl  = 
					NEO_BC3_FIFO_EN |
					NEO_BC3_SKIP_MAPPING |  rop;
		mmio->srcStart = srcY * screen->pitch + srcX * screen->depth;
		mmio->dstStart = dstY * screen->pitch + dstX * screen->depth;
		
		mmio->xyExt    = (unsigned long)(h << 16) | (w & 0xffff);
	} else {
		mmio->bltCntl  = NEO_BC0_X_DEC |
					NEO_BC0_DST_Y_DEC |
					NEO_BC0_SRC_Y_DEC |
					NEO_BC3_FIFO_EN |
					NEO_BC3_SKIP_MAPPING |  rop;
		srcX+=w-1;
		dstX+=w-1;
		srcY+=h-1;
		dstY+=h-1;
		mmio->srcStart = srcY * screen->pitch + srcX * screen->depth;
		mmio->dstStart = dstY * screen->pitch + dstX * screen->depth;
		mmio->xyExt    = (unsigned long)(h << 16) | (w & 0xffff);
	}	

}

static void neoDoneCopy (void)
{
}


Bool neoDrawInit (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    neoScreenInfo(pScreenPriv);

    ENTER();

    memset(&neos->kaa, 0, sizeof(KaaScreenInfoRec));
    neos->kaa.waitMarker	= neoWaitMarker;
    neos->kaa.PrepareSolid	= neoPrepareSolid;
    neos->kaa.Solid		= neoSolid;
    neos->kaa.DoneSolid		= neoDoneSolid;
    neos->kaa.PrepareCopy	= neoPrepareCopy;
    neos->kaa.Copy		= neoCopy;
    neos->kaa.DoneCopy		= neoDoneCopy;

    if (!kaaDrawInit (pScreen, &neos->kaa)) {
        return FALSE;
    }
    LEAVE();
    return TRUE;
}

void neoDrawEnable (ScreenPtr pScreen)
{
    ENTER();
    SetupNeo(pScreen);
    screen = neos;
    card = neoc;
    mmio = neoc->mmio;
    screen->depth = (screen->backendScreen.mode.BitsPerPixel+7)/8;
    screen->pitch = screen->backendScreen.mode.BytesPerScanLine;
    DBGOUT("NEO depth=%x, pitch=%x\n", screen->depth, screen->pitch);
    LEAVE();
}

void neoDrawDisable (ScreenPtr pScreen)
{
    ENTER();
    LEAVE();
}

void neoDrawFini (ScreenPtr pScreen)
{
    ENTER();
    LEAVE();
}

