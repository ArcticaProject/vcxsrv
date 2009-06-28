/*
 * Copyright © 2003 Anders Carlsson
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Anders Carlsson not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Anders Carlsson makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ANDERS CARLSSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ANDERS CARLSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _R128_H_
#define _R128_H_
#include <vesa.h>

#define R128_REG_BASE(c)	    ((c)->attr.address[1])
#define R128_REG_SIZE(c)	    (0x4000)

#define R128_OUT32(mmio, a, v) (*(VOL32 *) ((mmio) + (a)) = (v))
#define R128_IN32(mmio, a) (*(VOL32 *) ((mmio) + (a)))

#define R128_REG_GUI_STAT		0x1740
#define R128_REG_DEFAULT_OFFSET 	0x16e0
#define R128_REG_DEFAULT_PITCH		0x16e4
#define R128_REG_DP_GUI_MASTER_CNTL	0x146c
#define R128_REG_DP_BRUSH_FRGD_CLR      0x147c
#define R128_REG_DP_WRITE_MASK          0x16cc
#define R128_REG_DP_CNTL                0x16c0
#define R128_REG_DST_WIDTH_HEIGHT       0x1598
#define R128_REG_DST_Y_X                0x1438
#define R128_REG_PC_NGUI_CTLSTAT	0x0184
#define R128_REG_DST_HEIGHT_WIDTH       0x143c
#define R128_REG_SRC_Y_X                0x1434
#define R128_DEFAULT_SC_BOTTOM_RIGHT    0x16e8
#define R128_AUX_SC_CNTL                0x1660
#define R128_SC_TOP_LEFT                0x16ec
#define R128_SC_BOTTOM_RIGHT            0x16f0

#define R128_GMC_DST_DATATYPE_SHIFT     8
#define R128_GMC_CLR_CMP_CNTL_DIS       (1 << 28)
#define R128_GMC_AUX_CLIP_DIS           (1 << 29)
#define R128_GMC_BRUSH_SOLID_COLOR      (13 << 4)
#define R128_GMC_SRC_DATATYPE_COLOR     (3 << 12)
#define R128_GMC_ROP3_SHIFT		16
#define R128_DST_X_LEFT_TO_RIGHT        (1 <<  0)
#define R128_DST_Y_TOP_TO_BOTTOM        (1 <<  1)
#define R128_GUI_ACTIVE			(1 << 31)
#define R128_PC_BUSY                    (1 << 31)
#define R128_DP_SRC_SOURCE_MEMORY       (2 << 24)
#define R128_DEFAULT_SC_RIGHT_MAX       (0x1fff <<  0)
#define R128_DEFAULT_SC_BOTTOM_MAX      (0x1fff << 16)

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;
			 
typedef struct _r128CardInfo {
    VesaCardPrivRec vesa;
    CARD8 *reg_base;
    int fifo_size;
} R128CardInfo;

#define getR128CardInfo(kd) ((R128CardInfo *) ((kd)->card->driver))
#define r128CardInfo(kd)	R128CardInfo *r128c = getR128CardInfo(kd)

typedef struct _r128ScreenInfo {
    VesaScreenPrivRec vesa;
    CARD8 *screen;
    CARD8 *off_screen;
    int off_screen_size;

    KaaScreenInfoRec kaa;

    int pitch;
    int datatype;

    int dp_gui_master_cntl;
} R128ScreenInfo;

#define getR128ScreenInfo(kd) ((R128ScreenInfo *) ((kd)->screen->driver))
#define r128ScreenInfo(kd)    R128ScreenInfo *r128s = getR128ScreenInfo(kd)

Bool
r128MapReg (KdCardInfo *card, R128CardInfo *r128c);

void
r128UnmapReg (KdCardInfo *card, R128CardInfo *r128c);

void
r128SetMMIO (KdCardInfo *card, R128CardInfo *r128c);

void
r128ResetMMIO (KdCardInfo *card, R128CardInfo *r128c);

Bool
r128DrawSetup (ScreenPtr pScreen);

Bool
r128DrawInit (ScreenPtr pScreen);

void
r128DrawEnable (ScreenPtr pScreen);

void
r128DrawDisable (ScreenPtr pScreen);

void
r128DrawFini (ScreenPtr pScreen);

extern KdCardFuncs  r128Funcs;

#endif /* _R128_H_ */
