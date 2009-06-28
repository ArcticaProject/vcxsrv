/*
 * Copyright © 2004 Ralph Thomas
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Ralph Thomas not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Ralph Thomas makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * RALPH THOMAS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL RALPH THOMAS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
** VIA CLE266 driver
** Copyright 2004 (C) Ralph Thomas <ralpht@gmail.com>
**
** http://www.viatech.com.tw/
**
** This header has some function prototypes for the hardware
** accelerated drawing code in viadraw.c.
*/

#ifndef _VIA_DRAW_H_
#define _VIA_DRAW_H_

/*
** More information on these functions is in viadraw.c.
*/
void viaWaitIdle( ViaCardInfo* card );
Bool viaDrawInit( ScreenPtr pScreen );
void viaDrawEnable( ScreenPtr pScreen );
void viaDrawDisable( ScreenPtr pScreen );
void viaDrawFini( ScreenPtr pScreen );
void viaDrawSync( ScreenPtr pScreen );

#endif
