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
*/

#ifndef _VIA_H_
#define _VIA_H_

#include <vesa.h>
#include <klinux.h>

/*
** Define DebugF so that we can spot debug statements easily.
*/
#define DebugF ErrorF

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;

/*
** These macros provide access to data on the card. The VIA graphics chips
** are only available on IA-32 architecture, so PCI address space and CPU
** address space are always the same (hence accesses can be performed by
** dereferencing a pointer into PCI space).
*/
#define MMIO_OUT32( mmio, a, v )	(*(VOL32 *)((mmio) + (a)) = (v))
#define MMIO_IN32( mmio, a )		(*(VOL32 *)((mmio) + (a)))
#define MMIO_OUT16( mmio, a, v )	(*(VOL16 *)((mmio) + (a)) = (v))
#define MMIO_IN16( mmio, a )		(*(VOL16 *)((mmio) + (a)))
#define MMIO_OUT8( mmio, a, v )		(*(VOL8 *)((mmio) + (a)) = (v))
#define MMIO_IN8( mmio, a, v )		(*(VOL8 *)((mmio) + (a)))

/*
** VGA regisers are offset 0x8000 from the beginning of the mmap'd register
** space.
*/
#define VIA_MMIO_VGABASE		0x8000

/*
** The size of the register space, used when we mmap the registers. The
** argument "c" should be a KdCardInfo*.
*/
#define VIA_REG_SIZE(c)			(0x9000)

/*
** The base of the register space, used when we mmap the registers. The
** argument "c" should be a KdCardInfo*.
*/
#define VIA_REG_BASE(c)			((c)->attr.address[1])

/*
** Access to the mmap'd VGA registers. The VGA registers are offset from the
** beginning of the 16M pci space by 0x8000. These macros get used just like
** outb/inb would be used to access VGA.
*/
#define VGAOUT32( addr, v )	MMIO_OUT32( viac->mapBase + VIA_MMIO_VGABASE, addr, v )
#define VGAIN32( addr )		MMIO_IN32( viac->mapBase + VIA_MMIO_VGABASE, addr )
#define VGAOUT16( addr, v )	MMIO_OUT16( viac->mapBase + VIA_MMIO_VGABASE, addr, v )
#define VGAIN16( addr )		MIIO_IN16( viac->mapBase + VIA_MMIO_VGABASE, addr )
#define VGAOUT8( addr, v )	MMIO_OUT8( viac->mapBase + VIA_MMIO_VGABASE, addr, v )
#define VGAIN8( addr )		MMIO_IN8( viac->mapBase + VIA_MMIO_VGABASE, addr )

/*
** Access to any of the registers on the chip.
*/
#define OUTREG32( addr, v )	MMIO_OUT32( viac->mapBase, addr, v )
#define INREG32( addr )		MMIO_IN32( viac->mapBase, addr )
#define OUTREG16( addr, v )	MMIO_OUT16( viac->mapBase, addr, v )
#define INREG16( addr )		MMIO_IN16( viac->mapBase, addr )

/*
** We keep all of our chip specific data in a ViaCardInfo.
*/
typedef struct _viaCardInfo {
	VesaCardPrivRec		vesa;		/* card info for VESA driver */
	VOL8*			mapBase;	/* mmap'd registers */
	CARD32			savedCommand;	/* command to issue to GE */
	CARD32			savedFgColor;	/* color to issue to GE */
} ViaCardInfo;

/*
** We keep all of our screen specific data in a ViaScreenInfo.
*/
typedef struct _viaScreenInfo {
	VesaScreenPrivRec	vesa;
	KaaScreenInfoRec	kaa;
} ViaScreenInfo;

/*
** These function prototypes are for support functions. More infomation on each
** function is available at the place the function is implemented, in via.c.
*/
Bool viaMapReg( KdCardInfo* card, ViaCardInfo* viac );
void viaUnmapReg( KdCardInfo* card, ViaCardInfo* viac );
void viaSetMMIO( KdCardInfo* card, ViaCardInfo* viac );
void viaResetMMIO( KdCardInfo* card, ViaCardInfo* viac );

/*
** The viaFuncs structure gets filled with the addresses of the functions
** that we use to talk to the graphics chip.
*/
extern KdCardFuncs	viaFuncs;

#endif

