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

#ifndef _NVIDIA_H_
#define _NVIDIA_H_
#include <vesa.h>
#include "kxv.h"
#include "klinux.h"

/*
 * offset from ioport beginning 
 */

#define DEBUG
#ifdef DEBUG
#define DBGOUT(fmt,a...) fprintf (stderr, fmt, ##a)
#else
#define DBGOUT(fmt,a...)
#endif

#define ENTER()	DBGOUT("Enter %s\n", __FUNCTION__)
#define LEAVE() DBGOUT("Leave %s\n", __FUNCTION__)

#define NVIDIA_REG_BASE(c)	    ((c)->attr.address[0])
#define NVIDIA_REG_SIZE(c)	    (16 * 1024 * 1024)

#define NVIDIA_PCIO_OFF(c)	    (0x601000)
#define NVIDIA_MMIO_OFF(c)	    (NVIDIA_PCIO_OFF(c) + 0)
#define NVIDIA_FIFO_OFF(c)	    (0x800000)
#define NVIDIA_ROP_OFF(c)	    (NVIDIA_FIFO_OFF(c) + 0)
#define NVIDIA_CLIP_OFF(c)	    (NVIDIA_FIFO_OFF(c) + 0x2000)
#define NVIDIA_PATT_OFF(c)	    (NVIDIA_FIFO_OFF(c) + 0x4000)
#define NVIDIA_PIXMAP_OFF(c)	    (NVIDIA_FIFO_OFF(c) + 0x6000)
#define NVIDIA_BLT_OFF(c)	    (NVIDIA_FIFO_OFF(c) + 0x8000)
#define NVIDIA_RECTANGLE_OFF(c)	    (NVIDIA_FIFO_OFF(c) + 0xa000)
#define NVIDIA_LINE_OFF(c)	    (NVIDIA_FIFO_OFF(c) + 0xc000)
#define NVIDIA_IS_3(c)		    (0)
#define NVIDIA_BUSY(c)		    (NVIDIA_IS_3(c) ? 0x6b0 : 0x700)
#define NVIDIA_BUSY_OFF(c)	    (0x400000 + NVIDIA_BUSY(c))

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;

#define NVIDIA_XY(x,y)	    ((x) | ((y) << 16))

typedef struct {
#if X_BYTE_ORDER == X_BIG_ENDIAN
    VOL32   FifoFree;
#else
    VOL16   FifoFree;
    VOL16   Nop;
#endif
} NvidiaFifoFree;

/*
 * Raster OPeration. Windows style ROP3.
 */
typedef struct {
    VOL32 reserved00[4];
    NvidiaFifoFree  FifoFree;
    VOL32 reserved01[0x0BB];
    VOL32 Rop3;
} NvidiaRop;

/*
 * 2D filled rectangle.
 */
typedef struct {
    VOL32 reserved00[4];
    NvidiaFifoFree  FifoFree;
    VOL32 reserved01[0x0BB];
    VOL32 reserved03[(0x040)-1];
    VOL32 Color1A;
    VOL32 TopLeft;
    VOL32 WidthHeight;
} NvidiaRectangle;

/*
 * 2D screen-screen BLT.
 */
typedef struct {
    VOL32 reserved00[4];
    NvidiaFifoFree  FifoFree;
    VOL32 reserved01[0x0BB];
    VOL32 TopLeftSrc;
    VOL32 TopLeftDst;
    VOL32 WidthHeight;
} NvidiaScreenBlt;

typedef struct {
    VOL32		busy;
} NvidiaBusy;

typedef struct _nvidiaCardInfo {
    VesaCardPrivRec	vesa;
    CARD8		*reg_base;
    int			fifo_free;
    int			fifo_size;
    CARD8		*mmio;
    NvidiaRop		*rop;
    NvidiaRectangle    	*rect;
    NvidiaScreenBlt	*blt;
    NvidiaBusy		*busy;
} NvidiaCardInfo;
    
#define getNvidiaCardInfo(kd)	((NvidiaCardInfo *) ((kd)->card->driver))
#define nvidiaCardInfo(kd)	NvidiaCardInfo	*nvidiac = getNvidiaCardInfo(kd)

/*
 * Xv information, optional
 */
typedef struct _nvidiaPortPriv {
    CARD32      YBuf0Offset;

    CARD32      YBuf1Offset;

    CARD8	currentBuf;

    int		brightness;
    int		saturation;

    RegionRec   clip;
    CARD32      colorKey;

    Bool	videoOn;
    Time        offTime;
    Time        freeTime;
    CARD32	size;
    CARD32	offset;
} NvidiaPortPrivRec, *NvidiaPortPrivPtr;

Bool nvidiaInitVideo(ScreenPtr pScreen);

typedef struct _nvidiaScreenInfo {
    VesaScreenPrivRec		vesa;
    CARD8			*cursor_base;
    CARD8			*screen;
    CARD8			*off_screen;
    int				off_screen_size;
    KdVideoAdaptorPtr		pAdaptor;
    KaaScreenInfoRec		kaa;
} NvidiaScreenInfo;

#define getNvidiaScreenInfo(kd) ((NvidiaScreenInfo *) ((kd)->screen->driver))
#define nvidiaScreenInfo(kd)    NvidiaScreenInfo *nvidias = getNvidiaScreenInfo(kd)
    
void
nvidiaPreserve (KdCardInfo *card);

void
nvidiaOutb (NvidiaCardInfo *nvidiac, CARD16 port, CARD8 val);

CARD8
nvidiaInb (NvidiaCardInfo *nvidiac, CARD16 port);

CARD8
nvidiaGetIndex (NvidiaCardInfo *nvidiac, CARD16 addr, CARD16 data, CARD8 id);

void
nvidiaSetIndex (NvidiaCardInfo *nvidiac, CARD16 addr, CARD16 data, CARD8 id, CARD8 val);

Bool
nvidiaMapReg (KdCardInfo *card, NvidiaCardInfo *nvidiac);

void
nvidiaUnmapReg (KdCardInfo *card, NvidiaCardInfo *nvidiac);

void
nvidiaSetMMIO (KdCardInfo *card, NvidiaCardInfo *nvidiac);

void
nvidiaResetMMIO (KdCardInfo *card, NvidiaCardInfo *nvidiac);

Bool
nvidiaEnable (ScreenPtr pScreen);

void
nvidiaDisable (ScreenPtr pScreen);

void
nvidiaWait (NvidiaCardInfo *card, NvidiaFifoFree *free, int n);

void
nvidiaWaitIdle (NvidiaCardInfo *card);
    
Bool
nvidiaDrawSetup (ScreenPtr pScreen);

Bool
nvidiaDrawInit (ScreenPtr pScreen);

void
nvidiaDrawReinit (ScreenPtr pScreen);

void
nvidiaDrawEnable (ScreenPtr pScreen);

void
nvidiaDrawDisable (ScreenPtr pScreen);

void
nvidiaDrawFini (ScreenPtr pScreen);

CARD8
nvidiaReadIndex (NvidiaCardInfo *nvidiac, CARD16 port, CARD8 index);

void
nvidiaWriteIndex (NvidiaCardInfo *nvidiac, CARD16 port, CARD8 index, CARD8 value);

Bool
nvidiaCursorInit (ScreenPtr pScreen);

void
nvidiaCursorEnable (ScreenPtr pScreen);

void
nvidiaCursorDisable (ScreenPtr pScreen);

void
nvidiaCursorFini (ScreenPtr pScreen);

void
nvidiaRecolorCursor (ScreenPtr pScreen, int ndef, xColorItem *pdef);

extern KdCardFuncs  nvidiaFuncs;

#endif /* _NVIDIA_H_ */
