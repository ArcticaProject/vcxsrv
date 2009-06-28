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

#ifndef _SMIDRAW_H_
#define _SMIDRAW_H_

#define SetupSmi(s) KdScreenPriv(s); \
		    smiCardInfo(pScreenPriv); \
		    Cop	    *cop = smic->cop

#define SmiAlpha	(COP_MULTI_ALPHA|COP_ALPHA_WRITE_ENABLE)

#define _smiInit(cop,smic) { \
    if ((cop)->status == 0xffffffff) smiSetMMIO(smic); \
    (cop)->multi = (smic)->cop_depth; \
    (cop)->multi = (smic)->cop_stride; \
    (cop)->multi = SmiAlpha; \
} \

#define _smiSetSolidRect(cop,pix,alu,cmd) {\
    cop->multi = COP_MULTI_PATTERN; \
    cop->multi = COP_MULTI_ROP | smiRop[alu]; \
    cop->fg = (pix); \
    cmd = COP_OP_BLT | COP_SCL_OPAQUE | COP_OP_ROP | COP_OP_FG; \
}

#define _smiRect(cop,x1,y1,x2,y2,cmd) { \
    (cop)->dst_start_xy = TRI_XY (x1,y1); \
    (cop)->dst_end_xy = TRI_XY(x2,y2); \
    _smiWaitDone(cop); \
    (cop)->command = (cmd); \
}

#define COP_STATUS_BUSY	(COP_STATUS_BE_BUSY | \
			 COP_STATUS_DPE_BUSY | \
			 COP_STATUS_MI_BUSY)

#define _smiWaitDone(cop)   { \
    int __q__ = 500000; \
    while (__q__-- && (cop)->status & COP_STATUS_BUSY) \
	; \
    if (!__q__) \
	(cop)->status = 0;  \
}

#define _smiWaitIdleEmpty(cop)	_smiWaitDone(cop)

#define sourceInvarient(alu)	(((alu) & 3) == (((alu) >> 2) & 3))

#endif
