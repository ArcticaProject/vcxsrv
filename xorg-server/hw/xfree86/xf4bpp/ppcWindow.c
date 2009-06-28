/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


Copyright IBM Corporation 1987,1988,1989
All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that 
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
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

*/

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdlib.h>

#include "xf4bpp.h"
#include "mfbmap.h"
#include "mfb.h"
#include "scrnintstr.h"
#include "ibmTrace.h"

/*
   xf4bppCopyWindow copies only the parts of the destination that are
visible in the source.
*/

void 
xf4bppCopyWindow(pWin, ptOldOrg, prgnSrc)
    register WindowPtr pWin ;
    DDXPointRec ptOldOrg ;
    RegionPtr prgnSrc ;
{
    RegionPtr prgnDst ;
    register BoxPtr pbox ;
    register int dx, dy ;
    register int nbox ;
    register int pm ;

    BoxPtr pboxTmp, pboxNext, pboxBase, pboxNew ;
				/* temporaries for shuffling rectangles */

    TRACE(("xf4bppCopyWindow(pWin= 0x%x, ptOldOrg= 0x%x, prgnSrc= 0x%x)\n", pWin, ptOldOrg, prgnSrc)) ;


    dx = ptOldOrg.x - pWin->drawable.x ;
    dy = ptOldOrg.y - pWin->drawable.y ;
    REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);

    prgnDst = REGION_CREATE(pWin->drawable.pScreen, NULL, 1);
    REGION_INTERSECT(pWin->drawable.pScreen, prgnDst,
		     &pWin->borderClip, prgnSrc);

    if ( !( nbox = REGION_NUM_RECTS(prgnDst) ) )
	return;

    pbox = REGION_RECTS(prgnDst);

    pboxNew = 0 ;
    if ( nbox > 1 ) {
	if ( dy < 0 ) {
	    if ( dx > 0 ) {
		/* walk source bottom to top */
		/* keep ordering in each band, reverse order of bands */
		if ( !( pboxNew =
			(BoxPtr) xalloc( sizeof( BoxRec ) * nbox ) ) )
			return ;
		pboxBase = pboxNext = pbox+nbox - 1 ;
		while ( pboxBase >= pbox ) {
			while ( ( pboxNext >= pbox )
			     && ( pboxBase->y1 == pboxNext->y1 ) )
				pboxNext-- ;
			pboxTmp = pboxNext + 1 ;
			while ( pboxTmp <= pboxBase )
				*pboxNew++ = *pboxTmp++ ;
			pboxBase = pboxNext ;
		}
		pboxNew -= nbox ;
		pbox = pboxNew ;
	    }
	    else { /* dx <= 0 */
		/* we can just reverse the entire list in place */
		/* Do three-position swaps */
		BoxRec tmpBox ;

		pboxBase = pbox ;
		pboxNext = pbox + nbox - 1 ;
		while ( pboxBase < pboxNext ) {
			/* ****** Warning Structure Assignment !! ****** */
			tmpBox    = *pboxBase ;
			*pboxBase = *pboxNext ;
			*pboxNext = tmpBox ;
			pboxBase++ ;
			pboxNext-- ;
		}
	    }
	}
        else if ( dx < 0 ) {
	/* walk source right to left */
	    /* reverse order of rects in each band */
	    if ( !( pboxNew = (BoxPtr)xalloc(sizeof(BoxRec) * nbox) ) )
		return ;
	    pboxBase = pboxNext = pbox ;
	    while (pboxBase < pbox+nbox)
	    {
		while ((pboxNext < pbox+nbox) &&
		       (pboxNext->y1 == pboxBase->y1))
		    pboxNext++ ;
		pboxTmp = pboxNext ;
		while (pboxTmp != pboxBase)
		    *pboxNew++ = *--pboxTmp ;
		pboxBase = pboxNext ;
	    }
	    pboxNew -= nbox ;
	    pbox = pboxNew ;
	}
    } /* END if nbox > 1 */

    /*
     * call blit several times, the parms are:
     *   blit( alu,rplanes, wplanes, srcx, srcy, destx, desty, width, height ) ;
     */

    pm = ( 1 << pWin->drawable.depth ) - 1 ;
    for ( ; nbox-- ; pbox++ )
	xf4bppBitBlt( pWin, GXcopy, pm,
		 pbox->x1 + dx, pbox->y1 + dy, 
		 pbox->x1, pbox->y1, 
		 pbox->x2 - pbox->x1, pbox->y2 - pbox->y1) ;

    /* free up stuff */
    if ( pboxNew )
	xfree( pboxNew ) ;

    REGION_DESTROY(pWin->drawable.pScreen, prgnDst);
}

Bool xf4bppPositionWindow(pWin, x, y)
register WindowPtr pWin ;
register int x, y ;
{
    return TRUE ;
}

Bool 
xf4bppDestroyWindow(pWin)
register WindowPtr pWin ;
{
return pWin ? TRUE : FALSE ;
}

/* As The Name Says -- Used For ega, vga and apa8c */
Bool
xf4bppCreateWindowForXYhardware(pWin)
register WindowPtr pWin ;
{
    TRACE(("xf4bppCreateWindowForXYhardware (pWin= 0x%x)\n", pWin));

    return TRUE;
}
