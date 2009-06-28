/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


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

#include "windowstr.h"
#include "regionstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "mfb.h"
#include "maskbits.h"

/* 
   the solid fillers are called for rectangles and window backgrounds.
   the boxes are already translated.
   maybe this should always take a pixmap instead of a drawable?

   NOTE:
   iy = ++iy < tileHeight ? iy : 0
is equivalent to iy%= tileheight, and saves a division.
*/

/*
	MFBSOLIDFILLAREA	OPEQ	EQWHOLEOWRD
	mfbSolidWhiteArea	|=	= ~0
	mfbSolidBlackArea	&=~	= 0
	mfbSolidInvertArea	^=	^= ~0

EQWHOLEWORD is used to write whole longwords.  it could use OPEQ,
but *p++ |= ~0 on at least two compilers generates much
worse code than *p++ = ~0.  similarly for *p++ &= ~~0
and *p++ = 0.

*/

/*ARGSUSED*/
void
MFBSOLIDFILLAREA(pDraw, nbox, pbox, alu, nop)
    DrawablePtr pDraw;
    int nbox;
    BoxPtr pbox;
    int alu;
    PixmapPtr nop;
{
    int nlwidth;	/* width in longwords of the drawable */
    int w;		/* width of current box */
    register int h;	/* height of current box */
    register PixelType *p;	/* pointer to bits we're writing */
    register int nlw;	/* loop version of nlwMiddle */
    register PixelType startmask;
    register PixelType endmask;/* masks for reggedy bits at either end of line */
    register int nlwExtra;	
		        /* to get from right of box to left of next span */
    int nlwMiddle;	/* number of longwords between sides of boxes */
    PixelType *pbits;	/* pointer to start of drawable */

    mfbGetPixelWidthAndPointer(pDraw, nlwidth, pbits);

    while (nbox--)
    {
	w = pbox->x2 - pbox->x1;
	h = pbox->y2 - pbox->y1;
	p = mfbScanline(pbits, pbox->x1, pbox->y1, nlwidth);

	if ( ((pbox->x1 & PIM) + w) < PPW)
	{
	    maskpartialbits(pbox->x1, w, startmask);
	    nlwExtra = nlwidth;
	    Duff(h, *p OPEQ startmask; mfbScanlineInc(p, nlwExtra));
	}
	else
	{
	    maskbits(pbox->x1, w, startmask, endmask, nlwMiddle);
	    nlwExtra = nlwidth - nlwMiddle;

	    if (startmask && endmask)
	    {
		nlwExtra -= 1;
		while (h--)
		{
		    nlw = nlwMiddle;
		    *p OPEQ startmask;
		    p++;
		    Duff(nlw, *p++ EQWHOLEWORD);
		    *p OPEQ endmask;
		    mfbScanlineInc(p, nlwExtra);
		}
	    }
	    else if (startmask && !endmask)
	    {
		nlwExtra -= 1;
		while (h--)
		{
		    nlw = nlwMiddle;
		    *p OPEQ startmask;
		    p++;
		    Duff(nlw, *p++ EQWHOLEWORD);
		    mfbScanlineInc(p, nlwExtra);
		}
	    }
	    else if (!startmask && endmask)
	    {
		while (h--)
		{
		    nlw = nlwMiddle;
		    Duff(nlw, *p++ EQWHOLEWORD);
		    *p OPEQ endmask;
		    mfbScanlineInc(p, nlwExtra);
		}
	    }
	    else /* no ragged bits at either end */
	    {
		while (h--)
		{
		    nlw = nlwMiddle;
		    Duff(nlw, *p++ EQWHOLEWORD);
		    mfbScanlineInc(p, nlwExtra);
		}
	    }
	}
        pbox++;
    }
}



/* stipple a list of boxes

you can use the reduced rasterop for stipples.  if rrop is
black, AND the destination with (not stipple pattern).  if rrop is
white OR the destination with the stipple pattern.  if rrop is invert,
XOR the destination with the stipple pattern.

	MFBSTIPPLEFILLAREA	OPEQ
	mfbStippleWhiteArea	|=
	mfbStippleBlackArea	&=~
	mfbStippleInveryArea	^=
*/

/*ARGSUSED*/
void
MFBSTIPPLEFILLAREA(pDraw, nbox, pbox, alu, pstipple)
    DrawablePtr pDraw;
    int nbox;
    BoxPtr pbox;
    int alu;
    PixmapPtr pstipple;
{
    register PixelType *psrc;
			/* pointer to bits in tile, if needed */
    int tileHeight;	/* height of the tile */
    register PixelType srcpix;	

    int nlwidth;	/* width in longwords of the drawable */
    int w;		/* width of current box */
    register int nlw;	/* loop version of nlwMiddle */
    register PixelType *p;	/* pointer to bits we're writing */
    register int h;	/* height of current box */
    PixelType startmask;
    PixelType endmask;	/* masks for reggedy bits at either end of line */
    int nlwMiddle;	/* number of longwords between sides of boxes */
    int nlwExtra;	/* to get from right of box to left of next span */
    register int iy;	/* index of current scanline in tile */
    PixelType *pbits;	/* pointer to start of drawable */

    mfbGetPixelWidthAndPointer(pDraw, nlwidth, pbits);

    tileHeight = pstipple->drawable.height;
    psrc = (PixelType *)(pstipple->devPrivate.ptr);

    while (nbox--)
    {
	w = pbox->x2 - pbox->x1;
	h = pbox->y2 - pbox->y1;
	iy = pbox->y1 % tileHeight;
	p = mfbScanline(pbits, pbox->x1, pbox->y1, nlwidth);

	if ( ((pbox->x1 & PIM) + w) < PPW)
	{
	    maskpartialbits(pbox->x1, w, startmask);
	    nlwExtra = nlwidth;
	    while (h--)
	    {
		srcpix = psrc[iy];
		iy = ++iy < tileHeight ? iy : 0;
		*p OPEQ (srcpix & startmask);
		mfbScanlineInc(p, nlwExtra);
	    }
	}
	else
	{
	    maskbits(pbox->x1, w, startmask, endmask, nlwMiddle);
	    nlwExtra = nlwidth - nlwMiddle;

	    if (startmask && endmask)
	    {
		nlwExtra -= 1;
		while (h--)
		{
		    srcpix = psrc[iy];
		    iy = ++iy < tileHeight ? iy : 0;
		    nlw = nlwMiddle;
		    *p OPEQ (srcpix & startmask);
		    p++;
		    Duff (nlw, *p++ OPEQ srcpix);
		    *p OPEQ (srcpix & endmask);
		    mfbScanlineInc(p, nlwExtra);
		}
	    }
	    else if (startmask && !endmask)
	    {
		nlwExtra -= 1;
		while (h--)
		{
		    srcpix = psrc[iy];
		    iy = ++iy < tileHeight ? iy : 0;
		    nlw = nlwMiddle;
		    *p OPEQ (srcpix & startmask);
		    p++;
		    Duff(nlw, *p++ OPEQ srcpix);
		    mfbScanlineInc(p, nlwExtra);
		}
	    }
	    else if (!startmask && endmask)
	    {
		while (h--)
		{
		    srcpix = psrc[iy];
		    iy = ++iy < tileHeight ? iy : 0;
		    nlw = nlwMiddle;
		    Duff(nlw, *p++ OPEQ srcpix);
		    *p OPEQ (srcpix & endmask);
		    mfbScanlineInc(p, nlwExtra);
		}
	    }
	    else /* no ragged bits at either end */
	    {
		while (h--)
		{
		    srcpix = psrc[iy];
		    iy = ++iy < tileHeight ? iy : 0;
		    nlw = nlwMiddle;
		    Duff(nlw, *p++ OPEQ srcpix);
		    mfbScanlineInc(p, nlwExtra);
		}
	    }
	}
        pbox++;
    }
}
