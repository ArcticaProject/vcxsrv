/*
 * Copyright  1999 Keith Packard
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
/*
 *  All global variables and functions pertaining to keyboard key mapping
 *  live in this header file.
 */

#ifndef _KKEYMAP_H
#define _KKEYMAP_H

/* Offset of MIN_SCANCODE to 8 (X minimum scancode value) */
#define KD_KEY_OFFSET	(8 - kdMinScanCode)

#define KD_MIN_KEYCODE	8
#define KD_MAX_KEYCODE	254
#define KD_MAX_WIDTH	4
#define KD_MAX_LENGTH	(KD_MAX_KEYCODE - KD_MIN_KEYCODE + 1)

extern int		kdMinScanCode;
extern int		kdMaxScanCode;
extern int		kdMinKeyCode;
extern int		kdMaxKeyCode;
extern int		kdKeymapWidth;

extern KeySym		kdKeymap[KD_MAX_LENGTH * KD_MAX_WIDTH];

extern CARD8		kdModMap[MAP_LENGTH];

extern KeySymsRec	kdKeySyms;

typedef struct {
    KeySym  modsym;
    int	    modbit;
} KdKeySymModsRec;

#endif /* _KKEYMAP_H */
