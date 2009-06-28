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
#include "chips.h"

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
#include	"kaa.h"

CARD8 chipsBltRop[16] = {
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

CARD8 chipsSolidRop[16] = {
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

/* Definitions for the Chips and Technology BitBLT engine communication. */
/* These are done using Memory Mapped IO, of the registers */
/* BitBLT modes for register 93D0. */

#ifdef HIQV
#define ctPATCOPY               0xF0
#define ctLEFT2RIGHT            0x000
#define ctRIGHT2LEFT            0x100
#define ctTOP2BOTTOM            0x000
#define ctBOTTOM2TOP            0x200
#define ctSRCSYSTEM             0x400
#define ctDSTSYSTEM             0x800
#define ctSRCMONO               0x1000
#define ctBGTRANSPARENT         0x22000
#define ctCOLORTRANSENABLE      0x4000
#define ctCOLORTRANSDISABLE     0x0
#define ctCOLORTRANSDST         0x8000
#define ctCOLORTRANSROP         0x0
#define ctCOLORTRANSEQUAL       0x10000L
#define ctCOLORTRANSNEQUAL      0x0
#define ctPATMONO               0x40000L
#define ctPATSOLID              0x80000L
#define ctPATSTART0             0x000000L
#define ctPATSTART1             0x100000L
#define ctPATSTART2             0x200000L
#define ctPATSTART3             0x300000L
#define ctPATSTART4             0x400000L
#define ctPATSTART5             0x500000L
#define ctPATSTART6             0x600000L
#define ctPATSTART7             0x700000L
#define ctSRCFG                 0x000000L	/* Where is this for the 65550?? */
#else
#define ctPATCOPY               0xF0
#define ctTOP2BOTTOM            0x100
#define ctBOTTOM2TOP            0x000
#define ctLEFT2RIGHT            0x200
#define ctRIGHT2LEFT            0x000
#define ctSRCFG                 0x400
#define ctSRCMONO               0x800
#define ctPATMONO               0x1000
#define ctBGTRANSPARENT         0x2000
#define ctSRCSYSTEM             0x4000
#define ctPATSOLID              0x80000L
#define ctPATSTART0             0x00000L
#define ctPATSTART1             0x10000L
#define ctPATSTART2             0x20000L
#define ctPATSTART3             0x30000L
#define ctPATSTART4             0x40000L
#define ctPATSTART5             0x50000L
#define ctPATSTART6             0x60000L
#define ctPATSTART7             0x70000L
#endif

#define chipsFillPix(bpp,pixel) {\
    if (bpp == 8) \
    { \
	pixel = pixel & 0xff; \
    } \
    else if (bpp == 16) \
    { \
	pixel = pixel & 0xffff; \
    } \
}

static VOL8	*mmio;
static CARD32	byteStride;
static CARD32	bytesPerPixel;
static CARD32	pixelStride;

static void
chipsSet (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    chipsScreenInfo(pScreenPriv);
    
    mmio = chipss->mmio_base;
    byteStride = pScreenPriv->screen->fb[0].byteStride;
    bytesPerPixel = pScreenPriv->screen->fb[0].bitsPerPixel >> 3;
    pixelStride = pScreenPriv->screen->fb[0].pixelStride;
}

#ifdef HIQV
#define CHIPS_BR0	0x00	/* offset */
#define CHIPS_BR1    	0x04	/* bg */
#define CHIPS_BR2    	0x08	/* fg */
#define CHIPS_BR3    	0x0c	/* monochrome */
#define CHIPS_BR4	0x10	/* bitblt */
#define CHIPS_BR5	0x14	/* pattern addr */
#define CHIPS_BR6	0x18	/* source addr */
#define CHIPS_BR7	0x1c	/* dst addr */
#define CHIPS_BR8	0x20	/* dst w/h */
#else
#define CHIPS_DR0	0x83d0
#define CHIPS_DR1    	0x87d0
#define CHIPS_DR2    	0x8bd0
#define CHIPS_DR3    	0x8fd0
#define CHIPS_DR4	0x93d0
#define CHIPS_DR5	0x97d0
#define CHIPS_DR6	0x9bd0
#define CHIPS_DR7	0x9fd0
#endif

#define DBG(x)

static void
chipsPitch (int src, int dst)
{
    CARD32  p;

    p = ((dst & 0xffff) << 16) | (src & 0xffff);
    DBG(ErrorF ("\tpitch 0x%x\n", p));
#ifdef HIQV
    *(VOL32 *) (mmio + CHIPS_BR0) = p;
#else
    *(VOL32 *) (mmio + CHIPS_DR0) = p;
#endif
}

static void
chipsBg (Pixel   bg)
{
    DBG(ErrorF ("\tbg 0x%x\n", bg));
#ifdef HIQV
    *(VOL32 *) (mmio + CHIPS_BR1) = bg & 0xffff;
#else
    *(VOL32 *) (mmio + CHIPS_DR2) = bg;
#endif
}

static void
chipsFg (Pixel   fg)
{
    DBG(ErrorF ("\tfg 0x%x\n", fg));
#ifdef HIQV
    *(VOL32 *) (mmio + CHIPS_BR2) = fg;
#else
    *(VOL32 *) (mmio + CHIPS_DR3) = fg;
#endif
}

static void
chipsOp (CARD32 op)
{
    DBG(ErrorF ("\top 0x%x\n", op));
#ifdef HIQV
    *(VOL32 *) (mmio + CHIPS_BR4) = op;
#else
    *(VOL32 *) (mmio + CHIPS_DR4) = op;
#endif
}
    
static void
chipsRopSolid (int rop)
{
    CARD32  op;
    
    op = chipsSolidRop[rop] | ctTOP2BOTTOM | ctLEFT2RIGHT | ctPATSOLID | ctPATMONO;
    chipsOp (op);
}

static void
chipsSrc (int addr)
{
    DBG(ErrorF ("\tsrc 0x%x\n", addr));
#ifdef HIQV
    *(VOL32 *) (mmio + CHIPS_BR6) = addr;
#else
    *(VOL32 *) (mmio + CHIPS_DR5) = addr;
#endif
}

static void
chipsDst (int addr)
{
    DBG(ErrorF ("\tdst 0x%x\n", addr));
#ifdef HIQV
    *(VOL32 *) (mmio + CHIPS_BR7) = addr;
#else
    *(VOL32 *) (mmio + CHIPS_DR6) = addr;
#endif
}

static void
chipsWidthHeightGo (int w, int h)
{
    DBG(ErrorF ("\twidth height %d/%d\n", w, h));
#ifdef HIQV
    *(VOL32 *) (mmio + CHIPS_BR8) = ((h & 0xffff) << 16) | (w & 0xffff);
#else
    *(VOL32 *) (mmio + CHIPS_DR7) = ((h & 0xffff) << 16) | (w & 0xffff);
#endif
}

static void
chipsWaitIdle (void)
{
#ifdef HIQV
    int	timeout = 0;
    CARD8   tmp;
    VOL32   *br4 = (VOL32 *) (mmio + CHIPS_BR4);
    
    DBG(ErrorF ("\tBR4 0x%x 0x%x\n", mmio + CHIPS_BR4, *br4));
    DBG(ErrorF ("\tXR20 0x%x\n", chipsReadXR (0, 0x20)));
    for (;;)
    {
	if ((*br4 & 0x80000000) == 0)
	    break;
	tmp = chipsReadXR (0, 0x20);
	if ((tmp & 1) == 0)
	    break;
	if (++timeout > 1000000)
	{
	    ErrorF ("timeout\n");
	    tmp = chipsReadXR (0, 0x20);
	    chipsWriteXR (0, 0x20, tmp | 2);
	    sleep (1);
	    chipsWriteXR (0, 0x20, tmp);
	    sleep (1);
	}
    }
#else
    while (*(VOL32 *) (mmio + CHIPS_DR4) & 0x00100000)
	;
#endif
}

static void
chipsWaitMarker (ScreenPtr pScreen, int marker)
{
    chipsSet (pScreen);
    chipsWaitIdle ();
}

static Bool
chipsPrepareSolid (PixmapPtr	pPixmap,
		   int		alu,
		   Pixel	pm,
		   Pixel	fg)
{
    FbBits  depthMask;
    
    DBG(ErrorF ("PrepareSolid %d 0x%x\n", alu, fg));
    depthMask = FbFullMask(pPixmap->drawable.depth);
    if ((pm & depthMask) != depthMask)
	return FALSE;
    else
    {
	chipsSet (pPixmap->drawable.pScreen);
	chipsWaitIdle ();
	chipsFillPix(pPixmap->drawable.bitsPerPixel,fg);
	chipsFg (fg);
	chipsBg (fg);
	chipsRopSolid (alu);
	chipsPitch (byteStride, byteStride);
	return TRUE;
    }
}

static void
chipsSolid (int x1, int y1, int x2, int y2)
{
    CARD32  dst;
    int	    w, h;

    DBG(ErrorF ("    Solid %dx%d %dx%d\n", x1, y1, x2, y2));
    dst = y1 * byteStride + x1 * bytesPerPixel;
    w = (x2 - x1) * bytesPerPixel;
    h = (y2 - y1);
    chipsWaitIdle ();
    chipsDst (dst);
    chipsWidthHeightGo (w, h);
}

static void
chipsDoneSolid (void)
{
}

static CARD32	copyOp;

static Bool
chipsPrepareCopy (PixmapPtr	pSrcPixmap,
		  PixmapPtr	pDstPixmap,
		  int		dx,
		  int		dy,
		  int		alu,
		  Pixel		pm)
{
    FbBits  depthMask;
    
    DBG(ErrorF ("PrepareSolid %d 0x%x\n", alu, fg));
    depthMask = FbFullMask(pDstPixmap->drawable.depth);
    if ((pm & depthMask) != depthMask)
	return FALSE;
    else
    {
	copyOp = chipsBltRop[alu];
	if (dy >= 0)
	    copyOp |= ctTOP2BOTTOM;
	else
	    copyOp |= ctBOTTOM2TOP;
	if (dx >= 0)
	    copyOp |= ctLEFT2RIGHT;
	else
	    copyOp |= ctRIGHT2LEFT;
	chipsSet (pDstPixmap->drawable.pScreen);
	chipsWaitIdle ();
	chipsOp (copyOp);
	chipsPitch (byteStride, byteStride);
	return TRUE;
    }
}

static void
chipsCopy (int srcX,
	   int srcY,
	   int dstX,
	   int dstY,
	   int w,
	   int h)
{
    int	src, dst;
    if ((copyOp & (ctTOP2BOTTOM|ctBOTTOM2TOP)) == ctBOTTOM2TOP)
    {
	src = (srcY + h - 1) * byteStride;
	dst = (dstY + h - 1) * byteStride;
    }
    else
    {
	src = srcY * byteStride;
	dst = dstY * byteStride;
    }
    if ((copyOp & (ctLEFT2RIGHT|ctRIGHT2LEFT)) == ctRIGHT2LEFT)
    {
	src = src + (srcX + w) * bytesPerPixel - 1;
	dst = dst + (dstX + w) * bytesPerPixel - 1;
    }
    else
    {
	src = src + srcX * bytesPerPixel;
	dst = dst + dstX * bytesPerPixel;
    }
    chipsWaitIdle ();
    chipsSrc (src);
    chipsDst (dst);
    chipsWidthHeightGo (w * bytesPerPixel, h);
}

static void
chipsDoneCopy (void)
{
}

Bool
chipsDrawInit (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    chipsScreenInfo(pScreenPriv);

    switch (pScreenPriv->screen->fb[0].bitsPerPixel) {
    case 8:
    case 16:
	break;
    default:
	return FALSE;
    }
	
    memset(&chipss->kaa, 0, sizeof(KaaScreenInfoRec));
    chipss->kaa.waitMarker	= chipsWaitMarker;
    chipss->kaa.PrepareSolid	= chipsPrepareSolid;
    chipss->kaa.Solid		= chipsSolid;
    chipss->kaa.DoneSolid	= chipsDoneSolid;
    chipss->kaa.PrepareCopy	= chipsPrepareCopy;
    chipss->kaa.Copy		= chipsCopy;
    chipss->kaa.DoneCopy	= chipsDoneCopy;

    if (!kaaDrawInit (pScreen, &chipss->kaa))
	return FALSE;
    
    return TRUE;
}

void
chipsDrawEnable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    chipsScreenInfo(pScreenPriv);
    CARD8 mode = 0x00;
    
    switch (pScreenPriv->screen->fb[0].bitsPerPixel) {
    case 8:
	mode = 0x00;
	break;
    case 16:
	mode = 0x10;
	break;
    }
    chipsSet (pScreen);
    chipsWaitIdle ();
    chipsWriteXR (chipss, 0x20, mode);
    
    kaaMarkSync (pScreen);
}

void
chipsDrawDisable (ScreenPtr pScreen)
{
}

void
chipsDrawFini (ScreenPtr pScreen)
{
}

