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

#ifndef _CHIPS_H_
#define _CHIPS_H_
#include <vesa.h>

/*
 * offset from ioport beginning 
 */

#define HIQV
#ifdef HIQV
#define CHIPS_MMIO_BASE(c)	((c)->vesa.fb_phys + 0x400000)
#else
#define CHIPS_MMIO_BASE(c)	((c)->vesa.fb_phys + 0x200000)
#endif
#define CHIPS_MMIO_SIZE(c)	(0x20000)

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;

typedef struct _chipsSave {
    int	    dummy;
} ChipsSave;

typedef struct _chipsCardInfo {
    VesaCardPrivRec	vesa;
    CARD32		*window;
    Bool		mmio;
    ChipsSave		save;
} ChipsCardInfo;
    
#define getChipsCardInfo(kd)  ((ChipsCardInfo *) ((kd)->card->driver))
#define chipsCardInfo(kd)	    ChipsCardInfo	*chipsc = getChipsCardInfo(kd)

typedef struct _chipsCursor {
    int		width, height;
    int		xhot, yhot;
    Bool	has_cursor;
    CursorPtr	pCursor;
    Pixel	source, mask;
} ChipsCursor;

#define CHIPS_CURSOR_WIDTH	64
#define CHIPS_CURSOR_HEIGHT	64

typedef struct _chipsScreenInfo {
    VesaScreenPrivRec	vesa;
    CARD8	    *mmio_base;
    CARD8	    *cursor_base;
    CARD8	    *screen;
    CARD8	    *off_screen;
    int		    off_screen_size;
    ChipsCursor   cursor;
    KaaScreenInfoRec kaa;
} ChipsScreenInfo;

#define getChipsScreenInfo(kd) ((ChipsScreenInfo *) ((kd)->screen->driver))
#define chipsScreenInfo(kd)    ChipsScreenInfo *chipss = getChipsScreenInfo(kd)

Bool
chipsDrawInit (ScreenPtr pScreen);

void
chipsDrawEnable (ScreenPtr pScreen);

void
chipsDrawDisable (ScreenPtr pScreen);

void
chipsDrawFini (ScreenPtr pScreen);

CARD8
chipsReadXR (ChipsScreenInfo *chipsc, CARD8 index);

void
chipsWriteXR (ChipsScreenInfo *chipsc, CARD8 index, CARD8 value);

Bool
chipsCursorInit (ScreenPtr pScreen);

void
chipsCursorEnable (ScreenPtr pScreen);

void
chipsCursorDisable (ScreenPtr pScreen);

void
chipsCursorFini (ScreenPtr pScreen);

void
chipsRecolorCursor (ScreenPtr pScreen, int ndef, xColorItem *pdef);

extern KdCardFuncs  chipsFuncs;

#endif /* _CHIPS_H_ */
