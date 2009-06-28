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

#ifndef _NEOMAGIC_H_
#define _NEOMAGIC_H_
#include <backend.h>
#include "kxv.h"
#include "klinux.h"
#include "vesa.h"


#define DBGOUT DebugF

#define ENTER()    DBGOUT("Enter %s\n", __FUNCTION__)
#define LEAVE() DBGOUT("Leave %s\n", __FUNCTION__)

#define NEO_VENDOR 0x10c8
#define CAP_NM2070  0x01 /* If it's a NM2070 series */
#define CAP_NM2090  0x02 /* If it's a NM2090 series */
#define CAP_NM2097  0x03 /* If it's a NM2097 series */
#define CAP_NM2200  0x04 /* If it's a NM2200 series */

#define NEO_BS0_BLT_BUSY        0x00000001
#define NEO_BS0_FIFO_AVAIL      0x00000002
#define NEO_BS0_FIFO_PEND       0x00000004

#define NEO_BC0_DST_Y_DEC       0x00000001
#define NEO_BC0_X_DEC           0x00000002
#define NEO_BC0_SRC_TRANS       0x00000004
#define NEO_BC0_SRC_IS_FG       0x00000008
#define NEO_BC0_SRC_Y_DEC       0x00000010
#define NEO_BC0_FILL_PAT        0x00000020
#define NEO_BC0_SRC_MONO        0x00000040
#define NEO_BC0_SYS_TO_VID      0x00000080

#define NEO_BC1_DEPTH8          0x00000100
#define NEO_BC1_DEPTH16         0x00000200
#define NEO_BC1_X_320           0x00000400
#define NEO_BC1_X_640           0x00000800
#define NEO_BC1_X_800           0x00000c00
#define NEO_BC1_X_1024          0x00001000
#define NEO_BC1_X_1152          0x00001400
#define NEO_BC1_X_1280          0x00001800
#define NEO_BC1_X_1600          0x00001c00
#define NEO_BC1_DST_TRANS       0x00002000
#define NEO_BC1_MSTR_BLT        0x00004000
#define NEO_BC1_FILTER_Z        0x00008000

#define NEO_BC2_WR_TR_DST       0x00800000

#define NEO_BC3_SRC_XY_ADDR     0x01000000
#define NEO_BC3_DST_XY_ADDR     0x02000000
#define NEO_BC3_CLIP_ON         0x04000000
#define NEO_BC3_FIFO_EN         0x08000000
#define NEO_BC3_BLT_ON_ADDR     0x10000000
#define NEO_BC3_SKIP_MAPPING    0x80000000

#define NEO_MODE1_DEPTH8        0x0100
#define NEO_MODE1_DEPTH16       0x0200
#define NEO_MODE1_DEPTH24       0x0300
#define NEO_MODE1_X_320         0x0400
#define NEO_MODE1_X_640         0x0800
#define NEO_MODE1_X_800         0x0c00
#define NEO_MODE1_X_1024        0x1000
#define NEO_MODE1_X_1152        0x1400
#define NEO_MODE1_X_1280        0x1800
#define NEO_MODE1_X_1600        0x1c00
#define NEO_MODE1_BLT_ON_ADDR   0x2000

typedef volatile CARD8    VOL8;
typedef volatile CARD16    VOL16;
typedef volatile CARD32    VOL32;

#define NEO_REG_SIZE(c)        (0x200000L)

typedef volatile struct {
    CARD32 bltStat;
    CARD32 bltCntl;
    CARD32 xpColor;
    CARD32 fgColor;
    CARD32 bgColor;
    CARD32 pitch;
    CARD32 clipLT;
    CARD32 clipRB;
    CARD32 srcBitOffset;
    CARD32 srcStart;
    CARD32 reserved0;
    CARD32 dstStart;
    CARD32 xyExt;

    CARD32 reserved1[19];

    CARD32 pageCntl;
    CARD32 pageBase;
    CARD32 postBase;
    CARD32 postPtr;
    CARD32 dataPtr;
} NeoMMIO;

typedef struct _neoCardInfo {
    VesaCardPrivRec backendCard;

    CARD32 reg_base;
    NeoMMIO *mmio;
    int dstOrg;
    int dstPitch;
    int dstPixelWidth;

    int srcOrg;
    int srcPitch;
    int srcPixelWidth;

    struct NeoChipInfo *chip;

    CARD32 bltCntl;

} NeoCardInfo;

struct NeoChipInfo {
    CARD16 vendor;
    CARD16 device;
    CARD8 caps;
    char *name;
    int videoRam;
    int maxClock;
    int cursorMem;
    int cursorOff;
    int linearSize;
    int maxWidth;
    int maxHeight;
};

#define getNeoCardInfo(kd) ((NeoCardInfo *) ((kd)->card->driver))
#define neoCardInfo(kd) NeoCardInfo *neoc = getNeoCardInfo(kd)

typedef struct _neoScreenInfo {
    VesaScreenPrivRec backendScreen;

    CARD8 *screen;
    CARD8 *off_screen;
    int off_screen_size;
    int pitch;
    int depth;
    KdVideoAdaptorPtr pAdaptor;
    KaaScreenInfoRec kaa;
} NeoScreenInfo;

#define getNeoScreenInfo(kd) ((NeoScreenInfo *) ((kd)->screen->driver))
#define neoScreenInfo(kd) NeoScreenInfo *neos = getNeoScreenInfo(kd)

#define SetupNeo(s) KdScreenPriv(s); \
                    neoCardInfo(pScreenPriv); \
                    neoScreenInfo(pScreenPriv);

void
neoPreserve (KdCardInfo *card);

Bool
neoEnable (ScreenPtr pScreen);

void
neoDisable (ScreenPtr pScreen);

Bool
neoMapReg (KdCardInfo *card, NeoCardInfo *nvidiac);

void
neoUnmapReg (KdCardInfo *card, NeoCardInfo *nvidiac);

CARD8
neoGetIndex (NeoCardInfo *nvidiac, CARD16 addr,  CARD8 id);

void
neoSetIndex (NeoCardInfo *nvidiac, CARD16 addr,  CARD8 id, CARD8 val);

Bool
neoDrawInit (ScreenPtr pScreen);

void
neoDrawEnable (ScreenPtr pScreen);

void
neoDrawDisable (ScreenPtr pScreen);

void
neoDrawFini (ScreenPtr pScreen);

extern KdCardFuncs  neoFuncs;

#endif /* _NEOMAGIC_H_ */
