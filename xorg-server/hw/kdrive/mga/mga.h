/*
 * Copyright © 2003-2004 Anders Carlsson
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

#ifndef _MGA_H_
#define _MGA_H_
#include <vesa.h>
#include <klinux.h>

#define MGA_REG_BASE(c)	    ((c)->attr.address[1])
#define MGA_REG_SIZE(c)	    (0x4000)

#define MGA_OUT32(mmio, a, v) (*(VOL32 *) ((mmio) + (a)) = (v))
#define MGA_IN32(mmio, a) (*(VOL32 *) ((mmio) + (a)))

#define MGA_REG_EXEC		(0x0100)
#define MGA_REG_DWGCTL		(0x1c00)
#define MGA_REG_PLNWT		(0x1c1c)
#define MGA_REG_FCOL		(0x1c24)
#define MGA_REG_MACCESS 	(0x1c04)
#define MGA_REG_SGN		(0x1c58)
#define MGA_REG_AR0		(0x1c60)
#define MGA_REG_AR1		(0x1c64)
#define MGA_REG_AR2		(0x1c68)
#define MGA_REG_AR3		(0x1c6C)
#define MGA_REG_AR4		(0x1c70)
#define MGA_REG_AR5		(0x1c74)
#define MGA_REG_AR6		(0x1c78)

#define MGA_REG_CXBNDRY		(0x1c80)
#define MGA_REG_FXBNDRY		(0x1c84)
#define MGA_REG_YDSTLEN		(0x1c88)
#define MGA_REG_PITCH 		(0x1c8c)
#define MGA_REG_YTOP		(0x1c98)
#define MGA_REG_YBOT		(0x1c9c)
#define MGA_REG_FIFOSTATUS 	(0x1e10)
#define MGA_REG_STATUS	 	(0x1e14)
#define MGA_REG_CACHEFLUSH 	(0x1fff)
#define MGA_REG_SRCORG 		(0x2cb4)
#define MGA_REG_DSTORG 		(0x2cb8)

#define MGA_G4XX_DEVICE_ID	(0x0525)

#define MGA_PW8 	(0)
#define MGA_PW16 	(1)
#define MGA_PW24 	(2)
#define MGA_PW32 	(3)

/* Drawing opcodes */
#define MGA_OPCOD_TRAP	 (4)
#define MGA_OPCOD_TEXTURE_TRAP	 (6)
#define MGA_OPCOD_BITBLT (8)

#define MGA_DWGCTL_SOLID	(1 << 11)
#define MGA_DWGCTL_ARZERO	(1 << 12)
#define MGA_DWGCTL_SGNZERO	(1 << 13)
#define MGA_DWGCTL_SHIFTZERO	(1 << 14)

#define MGA_DWGCTL_BFCOL        (2 << 25)

#define MGA_ATYPE_RPL		(0 << 4)
#define MGA_ATYPE_RSTR		(1 << 4)
#define MGA_ATYPE_ZI		(3 << 4)
#define MGA_ATYPE_BLK 		(4 << 4)
#define MGA_ATYPE_I		(7 << 4)

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;
			 
typedef struct _mgaCardInfo {
    VesaCardPrivRec vesa;
    CARD8 *reg_base;
    int fifo_size;
} MgaCardInfo;

#define getMgaCardInfo(kd) ((MgaCardInfo *) ((kd)->card->driver))
#define mgaCardInfo(kd)	MgaCardInfo *mgac = getMgaCardInfo(kd)

typedef struct _mgaScreenInfo {
    VesaScreenPrivRec vesa;

    KaaScreenInfoRec kaa;

    int pitch;
    int pw;
} MgaScreenInfo;

#define getMgaScreenInfo(kd) ((MgaScreenInfo *) ((kd)->screen->driver))
#define mgaScreenInfo(kd)    MgaScreenInfo *mgas = getMgaScreenInfo(kd)


VOL8 *mmio;


Bool
mgaMapReg (KdCardInfo *card, MgaCardInfo *mgac);

void
mgaUnmapReg (KdCardInfo *card, MgaCardInfo *mgac);

void
mgaSetMMIO (KdCardInfo *card, MgaCardInfo *mgac);

void
mgaResetMMIO (KdCardInfo *card, MgaCardInfo *mgac);

Bool
mgaDrawSetup (ScreenPtr pScreen);

Bool
mgaDrawInit (ScreenPtr pScreen);

void
mgaDrawEnable (ScreenPtr pScreen);

void
mgaDrawDisable (ScreenPtr pScreen);

void
mgaDrawFini (ScreenPtr pScreen);

extern KdCardFuncs  mgaFuncs;


void
mgaWaitAvail (int n);

void
mgaWaitIdle (void);

Bool
mgaSetup (ScreenPtr pScreen, int dest_bpp, int wait);


#if 0
#define MGA_FALLBACK(x)		\
do {				\
	ErrorF x;		\
	return FALSE;		\
} while (0);
#else
#define MGA_FALLBACK(x) return FALSE;
#endif

#endif /* _MGA_H_ */
