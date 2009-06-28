/*
 * Copyright © 2001 Keith Packard
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

#ifndef _SMI_H_
#define _SMI_H_

#define SMI_VESA 0

#if SMI_VESA
#include <vesa.h>
#define subGetColors vesaGetColors
#define subPutColors vesaPutColors
#define subInitialize vesaInitialize
#define subScreenInitialize vesaScreenInitialize
#define subInitScreen vesaInitScreen
#define subFinishInitScreen vesaFinishInitScreen
#define subCreateResources vesaCreateResources
#define subRandRSetConfig vesaRandRSetConfig
#define subPreserve vesaPreserve
#define subEnable vesaEnable
#define subDPMS vesaDPMS
#define subRestore vesaRestore
#define subScreenFini vesaScreenFini
#define subCardFini vesaCardFini
#define subDisable vesaDisable
#define SubCardPrivRec	VesaCardPrivRec
#define SubScreenPrivRec    VesaScreenPrivRec
#define subUseMsg() vesaUseMsg()
#define subProcessArgument(c,v,i) vesaProcessArgument(c,v,i)
#else
#include <fbdev.h>
#define subGetColors fbdevGetColors
#define subPutColors fbdevPutColors
#define subInitialize fbdevInitialize
#define subScreenInitialize fbdevScreenInitialize
#define subInitScreen fbdevInitScreen
#define subFinishInitScreen fbdevFinishInitScreen
#define subCreateResources fbdevCreateResources
#define subRandRSetConfig fbdevRandRSetConfig
#define subPreserve fbdevPreserve
#define subEnable fbdevEnable
#define subDPMS fbdevDPMS
#define subRestore fbdevRestore
#define subScreenFini fbdevScreenFini
#define subCardFini fbdevCardFini
#define subDisable fbdevDisable
#define SubCardPrivRec	FbdevPriv
#define SubScreenPrivRec    FbdevScrPriv
#define subUseMsg()
#define subProcessArgument(c,v,i) 0
#endif

#include "kxv.h"

#define SMI_DEBUG 0
#if SMI_DEBUG
#define DBGOUT(fmt,a...) fprintf (stderr, fmt, ##a)
#else
#define DBGOUT(fmt,a...)
#endif

#define ENTER()	DBGOUT("Enter %s\n", __FUNCTION__)
#define LEAVE() DBGOUT("Leave %s\n", __FUNCTION__)

/*
 * offset from ioport beginning 
 */

#define SMI_IO_BASE(c)	    ((c)->attr.io)
#define SMI_REG_BASE(c)	    ((c)->attr.address[0])
#define SMI_REG_SIZE(c)	    (4096)

#define SMI_DPR_OFF(c)	    (0x00000)

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;

/* DPR reg */
typedef struct _DPR {
    VOL32   src_xy;			/* 0x00 */
    VOL32   dst_xy;			/* 0x04 */
    VOL32   dst_wh;			/* 0x08 */
    VOL32   accel_cmd;			/* 0x0c */
    VOL32   src_stride;			/* 0x10 */
    VOL32   fg;				/* 0x14 */
    VOL32   bg;				/* 0x18 */
    VOL32   data_format;		/* 0x1c */
    VOL32   transparent;		/* 0x20 */
    VOL32   mask1;			/* 0x24 ? */
    VOL32   mask2;			/* 0x28 ? */
    VOL32   scissors_ul;		/* 0x2c */
    VOL32   scissors_lr;		/* 0x30 */
    VOL32   mask3;			/* 0x34 */
    VOL32   mask4;			/* 0x38 */
    VOL32   dst_stride;			/* 0x3c */
    VOL32   unknown_40;			/* 0x40 */
    VOL32   unknown_44;			/* 0x44 */
} DPR;

#define SMI_XY(x,y)	    (((y) & 0x7fff) | (((x) & 0x7fff) << 16))

/* 2D Engine commands */
#define SMI_TRANSPARENT_SRC		0x00000100
#define SMI_TRANSPARENT_DEST		0x00000300

#define SMI_OPAQUE_PXL			0x00000000
#define SMI_TRANSPARENT_PXL		0x00000400

#define SMI_MONO_PACK_8			0x00001000
#define SMI_MONO_PACK_16		0x00002000
#define SMI_MONO_PACK_32		0x00003000

#define SMI_ROP2_SRC			0x00008000
#define SMI_ROP2_PAT			0x0000C000
#define SMI_ROP3			0x00000000

#define SMI_BITBLT			0x00000000
#define SMI_RECT_FILL			0x00010000
#define SMI_TRAPEZOID_FILL		0x00030000
#define SMI_SHORT_STROKE		0x00060000
#define SMI_BRESENHAM_LINE		0x00070000
#define SMI_HOSTBLT_WRITE		0x00080000
#define SMI_HOSTBLT_READ		0x00090000
#define SMI_ROTATE_BLT			0x000B0000

#define SMI_SRC_COLOR			0x00000000
#define SMI_SRC_MONOCHROME		0x00400000

#define SMI_GRAPHICS_STRETCH		0x00800000

#define SMI_ROTATE_CW			0x01000000
#define SMI_ROTATE_CCW			0x02000000

#define SMI_MAJOR_X			0x00000000
#define SMI_MAJOR_Y			0x04000000

#define SMI_LEFT_TO_RIGHT		0x00000000
#define SMI_RIGHT_TO_LEFT		0x08000000

#define SMI_COLOR_PATTERN		0x40000000
#define SMI_MONO_PATTERN		0x00000000

#define SMI_QUICK_START			0x10000000
#define SMI_START_ENGINE		0x80000000

#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5

typedef struct _smiCardInfo {
    SubCardPrivRec    	sub;
    CARD16		io_base;
    CARD8		*reg_base;
    DPR			*dpr;
    int			avail;
} SmiCardInfo;
    
#define getSmiCardInfo(kd)	((SmiCardInfo *) ((kd)->card->driver))
#define smiCardInfo(kd)	SmiCardInfo	*smic = getSmiCardInfo(kd)

typedef struct _smiScreenInfo {
    SubScreenPrivRec	sub;
    CARD8		*screen;
    CARD32		stride;
    CARD32		data_format;
    CARD8		dpr_vpr_enable;
    KaaScreenInfoRec kaa;
} SmiScreenInfo;

#define getSmiScreenInfo(kd) ((SmiScreenInfo *) ((kd)->screen->driver))
#define smiScreenInfo(kd)    SmiScreenInfo *smis = getSmiScreenInfo(kd)
    
void
smiPreserve (KdCardInfo *card);

Bool
smiMapReg (KdCardInfo *card, SmiCardInfo *smic);

void
smiUnmapReg (KdCardInfo *card, SmiCardInfo *smic);

void
smiOutb (CARD16 port, CARD8 val);

CARD8
smiInb (CARD16 port);

CARD8
smiGetIndex (SmiCardInfo *smic, CARD16 addr, CARD16 data, CARD8 id);

void
smiSetIndex (SmiCardInfo *smic, CARD16 addr, CARD16 data, CARD8 id, CARD8 val);

void
smiSetMMIO (KdCardInfo *card, SmiCardInfo *smic);

void
smiResetMMIO (KdCardInfo *card, SmiCardInfo *smic);

Bool
smiEnable (ScreenPtr pScreen);

void
smiDisable (ScreenPtr pScreen);

void
smiWaitAvail(SmiCardInfo *smic, int n);

void
smiWaitIdle (SmiCardInfo *smic);
    
Bool
smiDrawSetup (ScreenPtr pScreen);

Bool
smiDrawInit (ScreenPtr pScreen);

void
smiDrawReinit (ScreenPtr pScreen);

void
smiDrawEnable (ScreenPtr pScreen);

void
smiDrawDisable (ScreenPtr pScreen);

void
smiDrawFini (ScreenPtr pScreen);

CARD8
smiReadIndex (SmiCardInfo *smic, CARD16 port, CARD8 index);

void
smiWriteIndex (SmiCardInfo *smic, CARD16 port, CARD8 index, CARD8 value);

extern KdCardFuncs  smiFuncs;

#endif /* _SMI_H_ */
