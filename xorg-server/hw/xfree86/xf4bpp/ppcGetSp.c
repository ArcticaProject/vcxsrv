/*
 * Copyright IBM Corporation 1987,1988,1989
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that 
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <string.h>
#include "xf4bpp.h"
#include "OScompiler.h"
#include "mfbmap.h"
#include "mfb.h"
#include "servermd.h"
#include "ibmTrace.h"

/* GetSpans -- for each span, gets bits from drawable starting at ppt[i]
 * and continuing for pwidth[i] bits
 * Each scanline returned will be server scanline padded, i.e., it will come
 * out to an integral number of words.
 */
void
xf4bppGetSpans( pDrawable, wMax, ppt, pwidth, nspans, pdstStart )
    DrawablePtr		pDrawable ;	/* drawable from which to get bits */
    int			wMax ;		/* largest value of all *pwidths */
    DDXPointPtr		ppt ;		/* points to start copying from */
    int			*pwidth ;	/* list of number of bits to copy */
    int			nspans ;	/* number of scanlines to copy */
    char 		*pdstStart ;
{
    register int		j ;
    register unsigned char	*pdst ;	/* where to put the bits */
    register unsigned char	*psrc ;	/* where to get the bits */
    register int		pixmapStride ;


    TRACE( ( "xf4bppGetSpans(pDrawable=0x%x,wMax=%d,ppt=0x%x,pwidth=0x%x,nspans=%d)\n", 
	pDrawable, wMax, ppt, pwidth, nspans ) ) ;

    if ( ( pDrawable->depth == 1 ) && ( pDrawable->type == DRAWABLE_PIXMAP ) )
	{
	mfbGetSpans( pDrawable, wMax, ppt, pwidth, nspans, pdstStart ) ;
	return;
	}

    pixmapStride = PixmapBytePad( wMax, pDrawable->depth ) ;
    pdst = (unsigned char *) /* GJA */ pdstStart ;

    if ( pDrawable->type == DRAWABLE_WINDOW ) {
	for ( ; nspans-- ; ppt++, pwidth++ ) {
		xf4bppReadColorImage(  (WindowPtr)pDrawable,
			ppt->x, ppt->y, j = *pwidth, 1, pdst, pixmapStride ) ;
		pdst += j ; /* width is in 32 bit words */
		j = ( -j ) & 3 ;
		while ( j-- ) /* Pad out to 32-bit boundary */
		    *pdst++ = 0 ;
	}
    }
    else {  /* OK, if we are here, we had better be a DRAWABLE PIXMAP */
	register int widthSrc =  /* width of pixmap in bytes */
	 (int) ( (PixmapPtr) pDrawable )->devKind ;

	psrc = (unsigned char *) ( (PixmapPtr) pDrawable )->devPrivate.ptr ;
	for ( ; nspans-- ; ppt++, pwidth++ ) {
	    MOVE( psrc + ( ppt->y * widthSrc ) + ppt->x,
		  pdst, j = *pwidth ) ;
	    pdst += j ;
	    j = ( -j ) & 3 ;
	    while ( j-- ) /* Pad out to 32-bit boundary */
	    	*pdst++ = 0 ;
	}
    }
    return ;
}
