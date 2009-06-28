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

#ifndef _VGA_H_
#define _VGA_H_

typedef unsigned long	VGA32;
typedef unsigned short	VGA16;
typedef unsigned char	VGA8;
typedef int		VGABOOL;
typedef volatile VGA8	VGAVOL8;

#define VGATRUE	    1
#define VGAFALSE    0

typedef struct _vgaReg {
    VGA16	id;
    VGA8	base;
    VGA8	len;
} VgaReg;

#define VGA_REG_NONE	    0xffff
#define VGA_REG_END	    VGA_REG_NONE, 0, 0

typedef struct _vgaValue {
    VGA8	save;
    VGA8	cur;
    VGA16	flags;
} VgaValue;

#define VGA_VALUE_VALID	    1	/* value ever fetched */
#define VGA_VALUE_MODIFIED  2	/* value ever changed */
#define VGA_VALUE_DIRTY	    4	/* value needs syncing */
#define VGA_VALUE_SAVED	    8	/* value preserved */

typedef enum _vgaAccess {
    VgaAccessMem, VgaAccessIo, VgaAccessIndMem, VgaAccessIndIo,
    VgaAccessDone
} VgaAccess;

typedef struct _vgaMap {
    VgaAccess	access;
    VGA32	port;
    VGA8	addr;	    /* for Ind access; addr offset from port */
    VGA8	value;	    /* for Ind access; value offset from port */
    VGA8	index;	    /* for Ind access; index value */
} VgaMap;

#define VGA_UNLOCK_FIXED    1	/* dont save current value */
#define VGA_UNLOCK_LOCK	    2	/* execute only on relock */
#define VGA_UNLOCK_UNLOCK   4	/* execute only on unlock */

typedef struct _vgaSave {
    VGA16	first;
    VGA16	last;
} VgaSave;

#define VGA_SAVE_END	VGA_REG_NONE, VGA_REG_NONE

typedef struct _vgaCard {
    void	(*map) (struct _vgaCard *card, VGA16 reg, VgaMap *map, VGABOOL write);
    void	*closure;
    int		max;
    VgaValue	*values;
    VgaSave	*saves;
} VgaCard;

VGA8
VgaInb (VGA16 r);

void
VgaOutb (VGA8 v, VGA16 r);
    
VGA8
VgaReadMemb (VGA32 addr);

void
VgaWriteMemb (VGA8 v, VGA32 addr);

void
VgaSetImm (VgaCard *card, VgaReg *reg, VGA32 value);

VGA32
VgaGetImm (VgaCard *card, VgaReg *reg);

void
VgaSet (VgaCard *card, VgaReg *reg, VGA32 value);

VGA32
VgaGet (VgaCard *card, VgaReg *reg);

void
VgaFlush (VgaCard *card);

void
VgaFill (VgaCard *card, VGA16 low, VGA16 high);

void
VgaPreserve (VgaCard *card);

void
VgaInvalidate (VgaCard *card);

void
VgaRestore (VgaCard *card);

void
VgaFinish (VgaCard *card);

void
VgaFlushReg (VgaCard *card, VgaReg *reg);

VGA8
VgaFetch (VgaCard *card, VGA16 id);

void
VgaStore (VgaCard *card, VGA16 id, VGA8 value);

VGA8
_VgaFetchInd (VGA16 port, VGA8 reg);

void
_VgaStoreInd (VGA16 port, VGA8 reg, VGA8 value);

#endif /* _VGA_H_ */
