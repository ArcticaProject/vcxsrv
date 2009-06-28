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
 * 28-Jan-04  C.Stylianou       PRJ NBL: Created from fbdev.h
 *
 */

#ifndef _EPSON13806_H_
#define _EPSON13806_H_

#include <stdio.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>

#include "kdrive.h"

#ifdef RANDR
#include "randrstr.h"
#endif

typedef struct _epsonPriv {
    struct fb_var_screeninfo	var;
    struct fb_fix_screeninfo	fix;
    __u16			red[256];
    __u16			green[256];
    __u16			blue[256];
    int				fd;
    char			*fb;
    char			*fb_base;
} EpsonPriv;
    
typedef struct _epsonScrPriv {
    Rotation		randr;
    Bool			shadow;
    KaaScreenInfoRec	kaa;
} EpsonScrPriv;

extern KdCardFuncs  epsonFuncs;

Bool
epsonInitialize (KdCardInfo *card, EpsonPriv *priv);

Bool
epsonCardInit (KdCardInfo *card);

Bool
epsonScreenInit (KdScreenInfo *screen);

Bool
epsonScreenInitialize (KdScreenInfo *screen, EpsonScrPriv *scrpriv);
    
Bool
epsonInitScreen (ScreenPtr pScreen);

void
epsonPreserve (KdCardInfo *card);

Bool
epsonEnable (ScreenPtr pScreen);

Bool
epsonDPMS (ScreenPtr pScreen, int mode);

void
epsonDisable (ScreenPtr pScreen);

void
epsonRestore (KdCardInfo *card);

void
epsonScreenFini (KdScreenInfo *screen);

void
epsonCardFini (KdCardInfo *card);

void
epsonGetColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs);

void
epsonPutColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs);

/* 
 * History:
 * 28-Jan-04  C.Stylianou       NBL: Added the following prototypes for h/w accel.
 *
 */
Bool
epsonDrawInit (ScreenPtr pScreen);

void
epsonDrawEnable (ScreenPtr pScreen);

void
epsonDrawDisable (ScreenPtr pScreen);

void
epsonDrawFini (ScreenPtr pScreen);

/* 
 * History:
 * 28-Jan-04  C.Stylianou       NBL: Maps to Epson registers
 *
 */
void
initEpson13806(void);


#endif /* __EPSON13806_H_ */
