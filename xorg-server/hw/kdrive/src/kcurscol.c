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
#include "cursorstr.h"

static int
KdComputeCmapShift (unsigned long mask)
{
    int	shift;
    unsigned long   bit;
    
    shift = 16;
    bit = 0x80000000;
    while (!(mask & bit))
    {
	shift--;
	bit >>= 1;
    }
    return shift;
}

#define Shift(v,d)  ((d) < 0 ? ((v) >> (-d)) : ((v) << (d)))

void
KdAllocateCursorPixels (ScreenPtr	pScreen,
			int		fb,
			CursorPtr	pCursor, 
			Pixel		*source,
			Pixel		*mask)
{
    xColorItem	    sourceColor, maskColor;
    int		    r, g, b;
    KdScreenPriv(pScreen);

    if (pScreenPriv->screen->fb[fb].redMask)
    {

	r = KdComputeCmapShift (pScreenPriv->screen->fb[fb].redMask);
	g = KdComputeCmapShift (pScreenPriv->screen->fb[fb].greenMask);
	b = KdComputeCmapShift (pScreenPriv->screen->fb[fb].blueMask);
	*source = ((Shift(pCursor->foreRed,r) & pScreenPriv->screen->fb[fb].redMask) |
			    (Shift(pCursor->foreGreen,g) & pScreenPriv->screen->fb[fb].greenMask) |
			    (Shift(pCursor->foreBlue,b) & pScreenPriv->screen->fb[fb].blueMask));
	*mask = ((Shift(pCursor->backRed,r) & pScreenPriv->screen->fb[fb].redMask) |
			  (Shift(pCursor->backGreen,g) & pScreenPriv->screen->fb[fb].greenMask) |
			  (Shift(pCursor->backBlue,b) & pScreenPriv->screen->fb[fb].blueMask));
    }
    else
    {
	/*
	 * Set these to an invalid pixel value so that
	 * when the store colors comes through, the cursor
	 * won't get recolored
	 */
	*source = ~0;
	*mask = ~0;
	
	sourceColor.red = pCursor->foreRed;
	sourceColor.green = pCursor->foreGreen;
	sourceColor.blue = pCursor->foreBlue;
	FakeAllocColor(pScreenPriv->pInstalledmap[fb], &sourceColor);
	maskColor.red = pCursor->backRed;
	maskColor.green = pCursor->backGreen;
	maskColor.blue = pCursor->backBlue;
	FakeAllocColor(pScreenPriv->pInstalledmap[fb], &maskColor);
	FakeFreeColor(pScreenPriv->pInstalledmap[fb], sourceColor.pixel);
	FakeFreeColor(pScreenPriv->pInstalledmap[fb], maskColor.pixel);
	*source = sourceColor.pixel;
	*mask = maskColor.pixel;
    }
}
