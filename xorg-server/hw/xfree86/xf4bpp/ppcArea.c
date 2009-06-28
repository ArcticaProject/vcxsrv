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

/* 
 * ppc solid area fill
 *
 * Tom Paquin 8/87 
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf4bpp.h"
#include "mfbmap.h"
#include "mfb.h"
#include "ppcGCstr.h"
#include "ibmTrace.h"

void
xf4bppFillArea( pWin, nboxes, pBox, pGC )
    register WindowPtr pWin ;
    register int nboxes ;
    register BoxPtr pBox ;
    GCPtr	pGC ;
{
register int x, y, w, h ;
int alu ;
unsigned long int fg, bg, pm ;
int xSrc, ySrc ;
PixmapPtr pPixmap ;
ppcPrivGC *pPrivGC = dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey());

TRACE( ( "xf4bppFillArea(0x%x,%d,0x%x,0x%x)\n", pWin, nboxes, pBox, pGC ) ) ;

if ( ( alu = pPrivGC->colorRrop.alu ) == GXnoop || !nboxes )
	return ;

xSrc = pGC->patOrg.x + pWin->drawable.x ;
ySrc = pGC->patOrg.y + pWin->drawable.y ;

pm = pPrivGC->colorRrop.planemask ;
fg = pPrivGC->colorRrop.fgPixel ;
bg = pPrivGC->colorRrop.bgPixel ;

nboxes++ ;
switch ( pPrivGC->colorRrop.fillStyle ) {
	case FillTiled:
		for ( pPixmap = pGC->tile.pixmap ; --nboxes ; pBox++ )
			if ( ( w = pBox->x2 - ( x = pBox->x1 ) )
			  && ( h = pBox->y2 - ( y = pBox->y1 ) ) )
				xf4bppTileRect( pWin, pPixmap, alu, pm,
					     x, y, w, h, xSrc, ySrc ) ;
		break ;
	case FillOpaqueStippled:
		for ( pPixmap = pGC->stipple ; --nboxes ; pBox++ )
			if ( ( w = pBox->x2 - ( x = pBox->x1 ) )
			  && ( h = pBox->y2 - ( y = pBox->y1 ) ) )
				xf4bppOpaqueStipple( pWin, pPixmap, fg, bg, alu, pm,
					     x, y, w, h, xSrc, ySrc ) ;
		break ;
	case FillStippled:
		for ( pPixmap = pGC->stipple ; --nboxes ; pBox++ )
			if ( ( w = pBox->x2 - ( x = pBox->x1 ) )
			  && ( h = pBox->y2 - ( y = pBox->y1 ) ) )
				xf4bppFillStipple( pWin, pPixmap, fg, alu, pm,
					     x, y, w, h, xSrc, ySrc ) ;
		break ;
	case FillSolid:
		for ( ; --nboxes ; pBox++ )
			if ( ( w = pBox->x2 - ( x = pBox->x1 ) )
			  && ( h = pBox->y2 - ( y = pBox->y1 ) ) )
				xf4bppFillSolid( pWin, fg, alu, pm, x, y, w, h ) ;
		break ;
}

}
