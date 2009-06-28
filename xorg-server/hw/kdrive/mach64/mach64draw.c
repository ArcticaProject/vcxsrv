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
#include "mach64.h"
#include "mach64draw.h"

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

CARD8 mach64Rop[16] = {
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

#define MACH64_DRAW_COMBO_SOLID	0x1
#define MACH64_DRAW_COMBO_COPY	0x8

#define SYNC_ALWAYS 0
static Reg	*reg;
static CARD32	avail;
static CARD32	triple;

#define IDX(reg,n)  (&(reg)->n - &(reg)->CRTC_H_TOTAL_DISP)

void
mach64WaitAvail(Reg *reg, int n)
{
    if (avail < n)
    {
	while ((avail = ((reg->GUI_STAT) >> 16) & 0x3ff) < n)
	    ;
    }
    avail -= n;
}

void
mach64WaitIdle (Reg *reg)
{
    while (reg->GUI_STAT & 1)
	;
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

typedef struct _mach64AccelReg {
    int		depth;
    int		bitsPerPixel;
    CARD32	DP_PIX_WIDTH;
    CARD32	DP_SET_GUI_ENGINE;    
} Mach64AccelReg;

static const Mach64AccelReg	mach64AccelReg[] = {
    { 1, 1,
	/* DP_PIX_WIDTH */
	((PIX_FORMAT_MONO << 0) |	/* DP_DST_PIX_WIDTH */
	 (PIX_FORMAT_MONO << 4) |	/* COMPOSITE_PIX_WIDTH */
	 (PIX_FORMAT_MONO << 8) |	/* DP_SRC_PIX_WIDTH */
	 (0 << 13) |			/* DP_HOST_TRIPLE_EN */
	 (0 << 14) |			/* DP_PALETTE_TYPE */
	 (PIX_FORMAT_MONO << 16) |	/* DP_HOST_PIX_WIDTH */
	 (0 << 20) |			/* DP_C14_RGB_INDEX */
	 (0 << 24) |			/* DP_BYTE_PIX_ORDER */
	 (0 << 25) |			/* DP_CONVERSION_TEMP */
	 (0 << 26) |			/* DP_C14_RGB_LOW_NIBBLE */
	 (0 << 27) |			/* DP_C14_RGB_HIGH_NIBBLE */
	 (PIX_FORMAT_MONO << 28) |	/* DP_SCALE_PIX_WIDTH */
	 0),
	/* DP_SET_GUI_ENGINE */
	((PIX_FORMAT_MONO << 3) |
	 (1 << 6) |		/* SET_DP_SRC_PIX_WIDTH */
	 (6 << 7) |		/* SET_DST_OFFSET */
	 (0 << 10) |		/* SET_DST_PITCH */
	 (0 << 14) |		/* SET_DST_PITCH_BY_2 */
	 (0 << 15) |		/* SET_SRC_OFFPITCH_COPY */
	 (0 << 16) |		/* SET_SRC_HGTWID1_2 */
	 (0 << 20) |		/* SET_DRAWING_COMBO */
	 (1 << 24) |		/* SET_BUS_MASTER_OP */
	 (0 << 26) |		/* SET_BUS_MASTER_EN */
	 (0 << 27) |		/* SET_BUS_MASTER_SYNC */
	 (0 << 28) |		/* DP_HOST_TRIPLE_EN */
	 (0 << 29) |		/* FAST_FILL_EN */
	 (0 << 30) |		/* BLOCK_WRITE_EN */
	 0)
    },
    { 8, 8,
	/* DP_PIX_WIDTH */
	((PIX_FORMAT_PSEUDO_8 << 0) |	/* DP_DST_PIX_WIDTH */
	 (PIX_FORMAT_PSEUDO_8 << 4) |	/* COMPOSITE_PIX_WIDTH */
	 (PIX_FORMAT_PSEUDO_8 << 8) |	/* DP_SRC_PIX_WIDTH */
	 (0 << 13) |			/* DP_HOST_TRIPLE_EN */
	 (0 << 14) |			/* DP_PALETTE_TYPE */
	 (PIX_FORMAT_PSEUDO_8 << 16) |	/* DP_HOST_PIX_WIDTH */
	 (0 << 20) |			/* DP_C14_RGB_INDEX */
	 (0 << 24) |			/* DP_BYTE_PIX_ORDER */
	 (0 << 25) |			/* DP_CONVERSION_TEMP */
	 (0 << 26) |			/* DP_C14_RGB_LOW_NIBBLE */
	 (0 << 27) |			/* DP_C14_RGB_HIGH_NIBBLE */
	 (PIX_FORMAT_PSEUDO_8 << 28) |	/* DP_SCALE_PIX_WIDTH */
	 0),
	/* DP_SET_GUI_ENGINE */
	((PIX_FORMAT_PSEUDO_8 << 3) |
	 (1 << 6) |		/* SET_DP_SRC_PIX_WIDTH */
	 (6 << 7) |		/* SET_DST_OFFSET */
	 (0 << 10) |		/* SET_DST_PITCH */
	 (0 << 14) |		/* SET_DST_PITCH_BY_2 */
	 (0 << 15) |		/* SET_SRC_OFFPITCH_COPY */
	 (0 << 16) |		/* SET_SRC_HGTWID1_2 */
	 (0 << 20) |		/* SET_DRAWING_COMBO */
	 (1 << 24) |		/* SET_BUS_MASTER_OP */
	 (0 << 26) |		/* SET_BUS_MASTER_EN */
	 (0 << 27) |		/* SET_BUS_MASTER_SYNC */
	 (0 << 28) |		/* DP_HOST_TRIPLE_EN */
	 (0 << 29) |		/* FAST_FILL_EN */
	 (0 << 30) |		/* BLOCK_WRITE_EN */
	 0)
    },
    { 15, 16,
	/* DP_PIX_WIDTH */
	((PIX_FORMAT_TRUE_1555 << 0) |	/* DP_DST_PIX_WIDTH */
	 (PIX_FORMAT_TRUE_1555 << 4) |	/* COMPOSITE_PIX_WIDTH */
	 (PIX_FORMAT_TRUE_1555 << 8) |	/* DP_SRC_PIX_WIDTH */
	 (0 << 13) |			/* DP_HOST_TRIPLE_EN */
	 (0 << 14) |			/* DP_PALETTE_TYPE */
	 (PIX_FORMAT_TRUE_1555 << 16) |	/* DP_HOST_PIX_WIDTH */
	 (0 << 20) |			/* DP_C14_RGB_INDEX */
	 (0 << 24) |			/* DP_BYTE_PIX_ORDER */
	 (0 << 25) |			/* DP_CONVERSION_TEMP */
	 (0 << 26) |			/* DP_C14_RGB_LOW_NIBBLE */
	 (0 << 27) |			/* DP_C14_RGB_HIGH_NIBBLE */
	 (PIX_FORMAT_TRUE_1555 << 28) |	/* DP_SCALE_PIX_WIDTH */
	 0),
	/* DP_SET_GUI_ENGINE */
	((PIX_FORMAT_TRUE_1555 << 3) |
	 (1 << 6) |		/* SET_DP_SRC_PIX_WIDTH */
	 (6 << 7) |		/* SET_DST_OFFSET */
	 (0 << 10) |		/* SET_DST_PITCH */
	 (0 << 14) |		/* SET_DST_PITCH_BY_2 */
	 (0 << 15) |		/* SET_SRC_OFFPITCH_COPY */
	 (0 << 16) |		/* SET_SRC_HGTWID1_2 */
	 (0 << 20) |		/* SET_DRAWING_COMBO */
	 (1 << 24) |		/* SET_BUS_MASTER_OP */
	 (0 << 26) |		/* SET_BUS_MASTER_EN */
	 (0 << 27) |		/* SET_BUS_MASTER_SYNC */
	 (0 << 28) |		/* DP_HOST_TRIPLE_EN */
	 (0 << 29) |		/* FAST_FILL_EN */
	 (0 << 30) |		/* BLOCK_WRITE_EN */
	 0)
    },
    { 16, 16,
	/* DP_PIX_WIDTH */
	((PIX_FORMAT_TRUE_565 << 0) |	/* DP_DST_PIX_WIDTH */
	 (PIX_FORMAT_TRUE_565 << 4) |	/* COMPOSITE_PIX_WIDTH */
	 (PIX_FORMAT_TRUE_565 << 8) |	/* DP_SRC_PIX_WIDTH */
	 (0 << 13) |			/* DP_HOST_TRIPLE_EN */
	 (0 << 14) |			/* DP_PALETTE_TYPE */
	 (PIX_FORMAT_TRUE_565 << 16) |	/* DP_HOST_PIX_WIDTH */
	 (0 << 20) |			/* DP_C14_RGB_INDEX */
	 (0 << 24) |			/* DP_BYTE_PIX_ORDER */
	 (0 << 25) |			/* DP_CONVERSION_TEMP */
	 (0 << 26) |			/* DP_C14_RGB_LOW_NIBBLE */
	 (0 << 27) |			/* DP_C14_RGB_HIGH_NIBBLE */
	 (PIX_FORMAT_TRUE_565 << 28) |	/* DP_SCALE_PIX_WIDTH */
	 0),
	/* DP_SET_GUI_ENGINE */
	((PIX_FORMAT_TRUE_565 << 3) |
	 (1 << 6) |		/* SET_DP_SRC_PIX_WIDTH */
	 (6 << 7) |		/* SET_DST_OFFSET */
	 (0 << 10) |		/* SET_DST_PITCH */
	 (0 << 14) |		/* SET_DST_PITCH_BY_2 */
	 (0 << 15) |		/* SET_SRC_OFFPITCH_COPY */
	 (0 << 16) |		/* SET_SRC_HGTWID1_2 */
	 (0 << 20) |		/* SET_DRAWING_COMBO */
	 (1 << 24) |		/* SET_BUS_MASTER_OP */
	 (0 << 26) |		/* SET_BUS_MASTER_EN */
	 (0 << 27) |		/* SET_BUS_MASTER_SYNC */
	 (0 << 28) |		/* DP_HOST_TRIPLE_EN */
	 (0 << 29) |		/* FAST_FILL_EN */
	 (0 << 30) |		/* BLOCK_WRITE_EN */
	 0)
    },
    { 24, 24,
	/* DP_PIX_WIDTH */
	((PIX_FORMAT_PSEUDO_8 << 0) |    /* DP_DST_PIX_WIDTH */
	 (PIX_FORMAT_PSEUDO_8 << 4) |    /* COMPOSITE_PIX_WIDTH */
	 (PIX_FORMAT_PSEUDO_8 << 8) |    /* DP_SRC_PIX_WIDTH */
	 (0 << 13) |			    /* DP_HOST_TRIPLE_EN */
	 (0 << 14) |			    /* DP_PALETTE_TYPE */
	 (PIX_FORMAT_PSEUDO_8 << 16) |   /* DP_HOST_PIX_WIDTH */
	 (0 << 20) |			    /* DP_C14_RGB_INDEX */
	 (0 << 24) |			    /* DP_BYTE_PIX_ORDER */
	 (0 << 25) |			    /* DP_CONVERSION_TEMP */
	 (0 << 26) |			    /* DP_C14_RGB_LOW_NIBBLE */
	 (0 << 27) |			    /* DP_C14_RGB_HIGH_NIBBLE */
	 (PIX_FORMAT_PSEUDO_8 << 28) |  /* DP_SCALE_PIX_WIDTH */
	 0),
	/* DP_SET_GUI_ENGINE */
	((PIX_FORMAT_PSEUDO_8 << 3) |
	 (1 << 6) |		/* SET_DP_SRC_PIX_WIDTH */
	 (6 << 7) |		/* SET_DST_OFFSET */
	 (0 << 10) |		/* SET_DST_PITCH */
	 (0 << 14) |		/* SET_DST_PITCH_BY_2 */
	 (0 << 15) |		/* SET_SRC_OFFPITCH_COPY */
	 (0 << 16) |		/* SET_SRC_HGTWID1_2 */
	 (0 << 20) |		/* SET_DRAWING_COMBO */
	 (1 << 24) |		/* SET_BUS_MASTER_OP */
	 (0 << 26) |		/* SET_BUS_MASTER_EN */
	 (0 << 27) |		/* SET_BUS_MASTER_SYNC */
	 (0 << 28) |		/* DP_HOST_TRIPLE_EN */
	 (0 << 29) |		/* FAST_FILL_EN */
	 (0 << 30) |		/* BLOCK_WRITE_EN */
	 0)
    },
    { 24, 32,
	/* DP_PIX_WIDTH */
	((PIX_FORMAT_TRUE_8888 << 0) |   /* DP_DST_PIX_WIDTH */
	 (PIX_FORMAT_TRUE_8888 << 4) |   /* COMPOSITE_PIX_WIDTH */
	 (PIX_FORMAT_TRUE_8888 << 8) |   /* DP_SRC_PIX_WIDTH */
	 (0 << 13) |			    /* DP_HOST_TRIPLE_EN */
	 (0 << 14) |			    /* DP_PALETTE_TYPE */
	 (PIX_FORMAT_TRUE_8888 << 16) |  /* DP_HOST_PIX_WIDTH */
	 (0 << 20) |			    /* DP_C14_RGB_INDEX */
	 (0 << 24) |			    /* DP_BYTE_PIX_ORDER */
	 (0 << 25) |			    /* DP_CONVERSION_TEMP */
	 (0 << 26) |			    /* DP_C14_RGB_LOW_NIBBLE */
	 (0 << 27) |			    /* DP_C14_RGB_HIGH_NIBBLE */
	 (PIX_FORMAT_TRUE_8888 << 28) |  /* DP_SCALE_PIX_WIDTH */
	 0),
	/* DP_SET_GUI_ENGINE */
	((PIX_FORMAT_TRUE_8888 << 3) |
	 (1 << 6) |		/* SET_DP_SRC_PIX_WIDTH */
	 (6 << 7) |		/* SET_DST_OFFSET */
	 (0 << 10) |		/* SET_DST_PITCH */
	 (0 << 14) |		/* SET_DST_PITCH_BY_2 */
	 (0 << 15) |		/* SET_SRC_OFFPITCH_COPY */
	 (0 << 16) |		/* SET_SRC_HGTWID1_2 */
	 (0 << 20) |		/* SET_DRAWING_COMBO */
	 (1 << 24) |		/* SET_BUS_MASTER_OP */
	 (0 << 26) |		/* SET_BUS_MASTER_EN */
	 (0 << 27) |		/* SET_BUS_MASTER_SYNC */
	 (0 << 28) |		/* DP_HOST_TRIPLE_EN */
	 (0 << 29) |		/* FAST_FILL_EN */
	 (0 << 30) |		/* BLOCK_WRITE_EN */
	 0)
    },
    { 32, 32,
	/* DP_PIX_WIDTH */
	((PIX_FORMAT_TRUE_8888 << 0) |   /* DP_DST_PIX_WIDTH */
	 (PIX_FORMAT_TRUE_8888 << 4) |   /* COMPOSITE_PIX_WIDTH */
	 (PIX_FORMAT_TRUE_8888 << 8) |   /* DP_SRC_PIX_WIDTH */
	 (0 << 13) |			    /* DP_HOST_TRIPLE_EN */
	 (0 << 14) |			    /* DP_PALETTE_TYPE */
	 (PIX_FORMAT_TRUE_8888 << 16) |  /* DP_HOST_PIX_WIDTH */
	 (0 << 20) |			    /* DP_C14_RGB_INDEX */
	 (0 << 24) |			    /* DP_BYTE_PIX_ORDER */
	 (0 << 25) |			    /* DP_CONVERSION_TEMP */
	 (0 << 26) |			    /* DP_C14_RGB_LOW_NIBBLE */
	 (0 << 27) |			    /* DP_C14_RGB_HIGH_NIBBLE */
	 (PIX_FORMAT_TRUE_8888 << 28) |  /* DP_SCALE_PIX_WIDTH */
	 0),
	/* DP_SET_GUI_ENGINE */
	((PIX_FORMAT_TRUE_8888 << 3) |
	 (1 << 6) |		/* SET_DP_SRC_PIX_WIDTH */
	 (6 << 7) |		/* SET_DST_OFFSET */
	 (0 << 10) |		/* SET_DST_PITCH */
	 (0 << 14) |		/* SET_DST_PITCH_BY_2 */
	 (0 << 15) |		/* SET_SRC_OFFPITCH_COPY */
	 (0 << 16) |		/* SET_SRC_HGTWID1_2 */
	 (0 << 20) |		/* SET_DRAWING_COMBO */
	 (1 << 24) |		/* SET_BUS_MASTER_OP */
	 (0 << 26) |		/* SET_BUS_MASTER_EN */
	 (0 << 27) |		/* SET_BUS_MASTER_SYNC */
	 (0 << 28) |		/* DP_HOST_TRIPLE_EN */
	 (0 << 29) |		/* FAST_FILL_EN */
	 (0 << 30) |		/* BLOCK_WRITE_EN */
	 0)
    }
};

#define NACCELREG (sizeof mach64AccelReg / sizeof mach64AccelReg[0])

static void
mach64WaitMarker (ScreenPtr pScreen, int marker)
{
    KdScreenPriv(pScreen);
    mach64CardInfo(pScreenPriv);
    reg = mach64c->reg;
    
    mach64WaitIdle (reg);
}

static Bool
mach64Setup (PixmapPtr pDst, PixmapPtr pSrc, CARD32 combo, int wait)
{
    ScreenPtr pScreen = pDst->drawable.pScreen;
    KdScreenPriv(pScreen);
    mach64CardInfo(pScreenPriv);
    CARD32  DST_PITCH;
    CARD32  DST_OFFSET;
    CARD32  SRC_PITCH;
    CARD32  SRC_OFFSET;
    CARD32  DP_PIX_WIDTH;
    CARD32  DP_SET_GUI_ENGINE;
    int	    i;

    for (i = 0; i < NACCELREG; i++)
	if (mach64AccelReg[i].depth == pDst->drawable.depth &&
	    mach64AccelReg[i].bitsPerPixel == pDst->drawable.bitsPerPixel)
	    break;
    if (i == NACCELREG)
	return FALSE;
    DP_PIX_WIDTH = mach64AccelReg[i].DP_PIX_WIDTH;
    DP_SET_GUI_ENGINE = mach64AccelReg[i].DP_SET_GUI_ENGINE;

    reg = mach64c->reg;
    if (!reg)
	return FALSE;
    
    triple = (pDst->drawable.bitsPerPixel == 24);
    
    if (triple)
    {
	/* bytes / 8 = bytes >> 3 */
	DST_PITCH = pDst->devKind >> 3;
    }
    else
    {
	/* pixels / 8 = ((bytes * 8) / bpp) / 8 = bytes / bpp */
	DST_PITCH = pDst->devKind / pDst->drawable.bitsPerPixel;
    }
    
    /* bytes / 8 */
    DST_OFFSET = ((CARD8 *) pDst->devPrivate.ptr - pScreenPriv->screen->memory_base) >> 3;
    
    mach64WaitAvail(reg, wait + (pSrc ? 5 : 4));
    reg->DP_SET_GUI_ENGINE = DP_SET_GUI_ENGINE | (combo << 20);
    reg->DP_PIX_WIDTH = DP_PIX_WIDTH;
    reg->DST_OFF_PITCH = ((DST_OFFSET << 0) |	/* USR1_DST_OFFSET */
			  (DST_PITCH << 22) |	/* USR1_DST_PITCH */
			  0);
    if (pSrc)
    {
	if (triple)
	{
	    SRC_PITCH = pSrc->devKind >> 3;
	}
        else
	{
	    /* pixels / 8 = ((bytes * 8) / bpp) / 8 = bytes / bpp */
	    SRC_PITCH = pSrc->devKind / pSrc->drawable.bitsPerPixel;
	}
	/* bytes / 8 */
	SRC_OFFSET = ((CARD8 *) pSrc->devPrivate.ptr - pScreenPriv->screen->memory_base) >> 3;
	
	reg->SRC_OFF_PITCH = ((SRC_OFFSET << 0) |
			      (SRC_PITCH << 22) |
			      0);
    }
    return TRUE;
}

static Bool
mach64PrepareSolid (PixmapPtr   pPixmap,
		    int		alu,
		    Pixel	pm,
		    Pixel	fg)
{
    if (!mach64Setup (pPixmap, 0, 1, 3))
	return FALSE;
    reg->DP_MIX = (mach64Rop[alu] << 16) | 0;
    reg->DP_WRITE_MSK = pm;
    reg->DP_FRGD_CLR = fg;
    return TRUE;
}

static void
mach64Solid (int x1, int y1, int x2, int y2)
{
    if (triple)
    {
	CARD32	traj;

	x1 *= 3;
	x2 *= 3;

	traj = (DST_X_DIR | 
		DST_Y_DIR |
		DST_24_ROT_EN | 
		DST_24_ROT((x1 / 4) % 6));
	mach64WaitAvail (reg, 1);
	reg->GUI_TRAJ_CNTL = traj;
    }
    mach64WaitAvail(reg,2);
    reg->DST_X_Y = MACH64_XY(x1,y1);
    reg->DST_WIDTH_HEIGHT = MACH64_XY(x2-x1,y2-y1);
}

static void
mach64DoneSolid (void)
{
#if SYNC_ALWAYS
    mach64WaitIdle (reg);
#endif
}

static int copyDx;
static int copyDy;
static CARD32	copyCombo;

static Bool
mach64PrepareCopy (PixmapPtr	pSrcPixmap,
		   PixmapPtr	pDstPixmap,
		   int		dx,
		   int		dy,
		   int		alu,
		   Pixel	pm)
{
    copyCombo = 8 | 2 | 1;
    copyDx = dx;
    copyDy = dy;
    
    /*
     * Avoid going backwards when copying pixmaps to the screen.
     * This should reduce tearing somewhat
     */
    if (pSrcPixmap == pDstPixmap)
    {
        if (dx <= 0)
	    copyCombo &= ~1;
	if (dy <= 0)
	    copyCombo &= ~2;
    }

    if (!mach64Setup (pDstPixmap, pSrcPixmap, copyCombo, 2))
	return FALSE;
    
    reg->DP_MIX = (mach64Rop[alu] << 16) | 0;
    reg->DP_WRITE_MSK = pm;
    return TRUE;
}

static void
mach64Copy (int srcX,
	    int srcY,
	    int dstX,
	    int dstY,
	    int w,
	    int h)
{
    if (triple)
    {
	CARD32	traj;

	srcX *= 3;
	dstX *= 3;
	w *= 3;

	traj = DST_24_ROT_EN | DST_24_ROT((dstX / 4) % 6);
	
	if (copyCombo & 1)
	    traj |= 1;
	if (copyCombo & 2)
	    traj |= 2;
	
	mach64WaitAvail (reg, 1);
	reg->GUI_TRAJ_CNTL = traj;
    }
    if ((copyCombo & 1) == 0)
    {
	srcX += w - 1;
	dstX += w - 1;
    }
    if ((copyCombo & 2) == 0)
    {
	srcY += h - 1;
	dstY += h - 1;
    }
    mach64WaitAvail (reg, 4);
    reg->SRC_Y_X = MACH64_YX(srcX, srcY);
    reg->SRC_WIDTH1 = w;
    reg->DST_Y_X = MACH64_YX(dstX, dstY);
    reg->DST_HEIGHT_WIDTH = MACH64_YX(w,h);
}

static void
mach64DoneCopy (void)
{
#if SYNC_ALWAYS
    mach64WaitIdle (reg);
#endif
}


Bool
mach64DrawInit (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    mach64ScreenInfo(pScreenPriv);

    memset(&mach64s->kaa, 0, sizeof(KaaScreenInfoRec));
    mach64s->kaa.waitMarker	= mach64WaitMarker;
    mach64s->kaa.PrepareSolid	= mach64PrepareSolid;
    mach64s->kaa.Solid		= mach64Solid;
    mach64s->kaa.DoneSolid	= mach64DoneSolid;
    mach64s->kaa.PrepareCopy	= mach64PrepareCopy;
    mach64s->kaa.Copy		= mach64Copy;
    mach64s->kaa.DoneCopy	= mach64DoneCopy;
    mach64s->kaa.offsetAlign	= 64;
    mach64s->kaa.pitchAlign	= 64;
    mach64s->kaa.flags		= KAA_OFFSCREEN_PIXMAPS;

    if (pScreenPriv->screen->fb[0].depth == 4)
	return FALSE;
    
    if (!kaaDrawInit (pScreen, &mach64s->kaa))
	return FALSE;

    return TRUE;
}

void
mach64DrawEnable (ScreenPtr pScreen)
{
    kaaMarkSync (pScreen);
}

void
mach64DrawDisable (ScreenPtr pScreen)
{
}

void
mach64DrawFini (ScreenPtr pScreen)
{
    kaaDrawFini (pScreen);
}
