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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdlib.h>

#include "xf4bpp.h"
#include "OScompiler.h"
#include "mi.h"
#include "scrnintstr.h"
#include "servermd.h"

/* Was MIGETIMAGE -- public entry for the GetImage Request
 * We're getting the image into a memory buffer. While we have to use GetSpans
 * to read a line from the device ( since we don't know what that looks like ) ,
 * we can just write into the destination buffer
 *
 * two different strategies are used, depending on whether we're getting the
 * image in Z format or XY format
 * Z format:
 * Line at a time, GetSpans a line and bcopy it to the destination
 * buffer, except that if the planemask is not all ones, we create a
 * temporary pixmap and do a SetSpans into it ( to get bits turned off ) 
 * and then another GetSpans to get stuff back ( because pixmaps are
 * opaque, and we are passed in the memory to write into ) .  This is
 * completely ugly and slow but works, but the interfaces just aren't
 * designed for this case.  Life is hard.
 * XY format:
 * get the single plane specified in planemask
 */
void
xf4bppGetImage( pDraw, sx, sy, w, h, format, planeMask, pdstLine ) 
    DrawablePtr pDraw ;
    int	sx, sy, w, h ;
    unsigned int format ;
    unsigned long planeMask ;
    char *	     pdstLine ;
{
#if 1
    int	    depth, i, linelength, width ;
    DDXPointRec	pt ;
    char *pbits ;
    XID gcv[2] ;
    PixmapPtr	pPixmap = (PixmapPtr) NULL ;
    GCPtr	pGC ;
    char *pDst = pdstLine ;

    depth = pDraw->depth ;
    if ( format == ZPixmap ) {
	linelength = PixmapBytePad( w, depth ) ;
/*	if ( pDraw->type == DRAWABLE_WINDOW ) { */
	    sx += pDraw->x ;
	    sy += pDraw->y ;
/*	} */
	if ( ( ( ( 1 << pDraw->depth ) - 1 ) & planeMask )
		 != (unsigned)( 1 << pDraw->depth ) - 1 ) {
	    pGC = GetScratchGC( depth, pDraw->pScreen ) ;
	    pPixmap = (PixmapPtr)
	      (* pDraw->pScreen->CreatePixmap)( pDraw->pScreen, w, h, depth,
						CREATE_PIXMAP_USAGE_SCRATCH) ;
	    gcv[0] = GXcopy ;
	    gcv[1] = planeMask ;
	    DoChangeGC( pGC, GCPlaneMask | GCFunction, gcv, 0 ) ;
	    ValidateGC( (DrawablePtr)pPixmap, pGC ) ;

	    pbits = (char *)xalloc(w);

	    for ( i = 0 ; i < h ; i++ ) {
		pt.x = sx ;
		pt.y = sy + i ;
		width = w ;
		(* pDraw->pScreen->GetSpans)( pDraw, w, &pt, &width, 1, pbits ) ;
		pt.x = 0 ;
		pt.y = i ;
		width = w ;
		if ( planeMask & ((1 << depth) - 1) ) /* GJA -- mfb bug */ 
		  (* pGC->ops->SetSpans)( (DrawablePtr)pPixmap, pGC, pbits, &pt, &width, 1, TRUE ) ;
		(* pDraw->pScreen->GetSpans)( (DrawablePtr)pPixmap, w, &pt, &width, 1, pDst ) ;
		pDst += linelength ;
	    }

	    xfree(pbits) ;
	    (* pGC->pScreen->DestroyPixmap)( pPixmap ) ;
	    FreeScratchGC( pGC ) ;
	    return ;
	}

	for ( i = 0 ; i < h ; i++ ) {
	    pt.x = sx ;
	    pt.y = sy + i ;
	    width = w ;
	    (* pDraw->pScreen->GetSpans)( pDraw, w, &pt, &width, 1, pDst ) ;
	    pDst += linelength ;
	}
    }
    else
#endif
       miGetImage( pDraw, sx, sy, w, h, format, planeMask, pdstLine ) ;
}
