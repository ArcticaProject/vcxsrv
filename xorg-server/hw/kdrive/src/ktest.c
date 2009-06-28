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

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"


static CARD8	memoryPatterns[] = { 0xff, 0x00, 0x5a, 0xa5, 0xaa, 0x55 };

#define NUM_PATTERNS	(sizeof (memoryPatterns) / sizeof (memoryPatterns[0]))

Bool
KdFrameBufferValid (CARD8 *base, int size)
{
    volatile CARD8  *b = (volatile CARD8 *) base;
    CARD8	    save, test, compare;
    int		    i, j;

    b = base + (size - 1);
    save = *b;

    for (i = 0; i < NUM_PATTERNS; i++)
    {
	test = memoryPatterns[i];
	*b = test;
	for (j = 0; j < 1000; j++)
	{
	    compare = *b;
	    if (compare != test)
		return FALSE;
	}
    }
    *b = save;
    return TRUE;
}

int
KdFrameBufferSize (CARD8 *base, int max)
{
    int	min, cur;
    
    min = 0;
    while (min + 1 < max)
    {
	cur = (max + min) / 2;
	if (KdFrameBufferValid (base, cur))
	    min = cur;
	else
	    max = cur;
    }
    if (KdFrameBufferValid (base, max))
	return max;
    else
	return min;
}
