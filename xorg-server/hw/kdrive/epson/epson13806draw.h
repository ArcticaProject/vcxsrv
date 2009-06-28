/*
 * Copyright 2004 by Costas Stylianou <costas.stylianou@psion.com> +44(0)7850 394095
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Costas Sylianou not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission. Costas Stylianou makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * COSTAS STYLIANOU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL COSTAS STYLIANOU BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* 
 * epson13806draw.h - Implementation of hard ware accelerated functions for epson S1D13806
 *               Graphic controller.
 *
 * History:
 * 28-Jan-04  C.Stylianou       PRJ NBL: Created from chipsdraw.h
 *
 */

#ifndef _EPSON13806DRAW_H_
#define _EPSON13806DRAW_H_


/*
 * offset from ioport beginning 
 */


#define SetupEpson(s)  KdScreenPriv(s); \
		    epsonCardInfo(pScreenPriv); \
		    EpsonPtr epson = epsonc->epson


typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;

typedef struct _epsonSave {
    int	    dummy;
} EpsonSave;

typedef struct _epsonCardInfo {
    EpsonPriv    epson;
    CARD32		*window;
    Bool		mmio;
    EpsonSave		save;
} epsonCardInfo;
    
#define getEpsonCardInfo(kd)  ((epsonCardInfo *) ((kd)->card->driver))
#define epsonCardInfo(kd)	    epsonCardInfo	*epsonc = getEpsonCardInfo(kd)

typedef struct _epsonCursor {
    int		width, height;
    int		xhot, yhot;
    Bool	has_cursor;
    CursorPtr	pCursor;
    Pixel	source, mask;
} EpsonCursor;

#define epson_CURSOR_WIDTH	64
#define epson_CURSOR_HEIGHT	64

typedef struct _epsonScreenInfo {
    EpsonScrPriv    epson;
    CARD8	    *cursor_base;
    CARD8	    *screen;
    CARD8	    *off_screen;
    int		    off_screen_size;
    EpsonCursor   cursor;
    void       *regbase_virt;
} EpsonScreenInfo;

#define getEpsonScreenInfo(kd) ((EpsonScreenInfo *) ((kd)->screen->driver))
#define epsonScreenInfo(kd)    EpsonScreenInfo *epsons = getEpsonScreenInfo(kd)

Bool
epsonDrawInit (ScreenPtr pScreen);

void
epsonDrawEnable (ScreenPtr pScreen);

void
epsonDrawSync (ScreenPtr pScreen);

void
epsonDrawDisable (ScreenPtr pScreen);

void
epsonDrawFini (ScreenPtr pScreen);

Bool
epsonCursorInit (ScreenPtr pScreen);

void
epsonCursorEnable (ScreenPtr pScreen);

void
epsonCursorDisable (ScreenPtr pScreen);

void
epsonCursorFini (ScreenPtr pScreen);

void
epsonRecolorCursor (ScreenPtr pScreen, int ndef, xColorItem *pdef);

extern KdCardFuncs  epsonFuncs;

#endif
