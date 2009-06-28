/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>

#include "gc.h"
#include "window.h"
#include "pixmap.h"
#include "region.h"

#include "afb.h"
#include "maskbits.h"

/* horizontal solid line
   abs(len) > 1
*/
void
afbHorzS(pbase, nlwidth, sizeDst, depthDst, x1, y1, len, rrops)
PixelType *pbase;		/* pointer to base of bitmap */
register int nlwidth;		/* width in longwords of bitmap */
int sizeDst;
int depthDst;
int x1;						/* initial point */
int y1;
int len;				/* length of line */
register unsigned char *rrops;
{
	register PixelType *addrl;
	register PixelType startmask;
	register PixelType endmask;
	register int nlmiddle;
	register int d;
	int saveNLmiddle;

	/* force the line to go left to right
	   but don't draw the last point
	*/
	if (len < 0) {
		x1 += len;
		x1 += 1;
		len = -len;
	}

	/* all bits inside same longword */
	if ( ((x1 & PIM) + len) < PPW) {
		maskpartialbits(x1, len, startmask);

		for (d = 0; d < depthDst; d++) {
			addrl = afbScanline(pbase, x1, y1, nlwidth);
			pbase += sizeDst;	/* @@@ NEXT PLANE @@@ */

			switch (rrops[d]) {
				case RROP_BLACK:
					*addrl &= ~startmask;
					break;
				case RROP_WHITE:
					*addrl |= startmask;
					break;
				case RROP_INVERT:
					*addrl ^= startmask;
					break;
				case RROP_NOP:
					break;
			} /* switch */
		} /* for (d = ...) */
	} else {
		maskbits(x1, len, startmask, endmask, nlmiddle);
		saveNLmiddle = nlmiddle;

		for (d = 0; d < depthDst; d++) {
			addrl = afbScanline(pbase, x1, y1, nlwidth);
			pbase += sizeDst;	/* @@@ NEXT PLANE @@@ */
			nlmiddle = saveNLmiddle;

			switch (rrops[d]) {
				case RROP_BLACK:
					if (startmask)
						*addrl++ &= ~startmask;
					Duff (nlmiddle, *addrl++ = 0x0);
					if (endmask)
						*addrl &= ~endmask;
					break;

				case RROP_WHITE:
					if (startmask)
						*addrl++ |= startmask;
					Duff (nlmiddle, *addrl++ = ~0);
					if (endmask)
						*addrl |= endmask;
					break;

				case RROP_INVERT:
					if (startmask)
						*addrl++ ^= startmask;
					Duff (nlmiddle, *addrl++ ^= ~0);
					if (endmask)
						*addrl ^= endmask;
					break;

				case RROP_NOP:
					break;
			} /* switch */
		} /* for (d = ... ) */
	}
}

/* vertical solid line
   this uses do loops because pcc (Ultrix 1.2, bsd 4.2) generates
   better code.  sigh.  we know that len will never be 0 or 1, so
   it's OK to use it.
*/
void
afbVertS(pbase, nlwidth, sizeDst, depthDst, x1, y1, len, rrops)
PixelType *pbase;		/* pointer to base of bitmap */
register int nlwidth;		/* width in longwords of bitmap */
int sizeDst;
int depthDst;
int x1, y1;				/* initial point */
register int len;		/* length of line */
unsigned char *rrops;
{
	register PixelType *addrl;
	register PixelType bitmask;
	int saveLen;
	int d;

	if (len < 0) {
		nlwidth = -nlwidth;
		len = -len;
	}

	saveLen = len;

	for (d = 0; d < depthDst; d++) {
		addrl = afbScanline(pbase, x1, y1, nlwidth);
		pbase += sizeDst;	/* @@@ NEXT PLANE @@@ */
		len = saveLen;

		switch (rrops[d]) {
			case RROP_BLACK:
				bitmask = mfbGetrmask(x1 & PIM);
				Duff(len, *addrl &= bitmask; afbScanlineInc(addrl, nlwidth) );
				break;

			case RROP_WHITE:
				bitmask = mfbGetmask(x1 & PIM);
				Duff(len, *addrl |= bitmask; afbScanlineInc(addrl, nlwidth) );
				break;

			case RROP_INVERT:
				bitmask = mfbGetmask(x1 & PIM);
				Duff(len, *addrl ^= bitmask; afbScanlineInc(addrl, nlwidth) );
				break;

			case RROP_NOP:
				break;
		} /* switch */
	} /* for (d = ...) */
}
