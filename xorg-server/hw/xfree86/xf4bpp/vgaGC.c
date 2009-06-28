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

#include "xf4bpp.h"
#include "mfbmap.h"
#include "mfb.h"
#include "mi.h"
#include "ppcGCstr.h"

void
xf4bppChangeGCtype( pGC, devPriv )
register GC *pGC ;
register ppcPrivGCPtr devPriv ;
{
	if ( devPriv->lastDrawableType == DRAWABLE_PIXMAP ) {
	    pGC->ops->CopyArea	= miCopyArea ;
	    pGC->ops->PolyFillRect	= miPolyFillRect ;
	    pGC->ops->PushPixels	= miPushPixels ;
	    pGC->ops->PolyArc	= miPolyArc ;
	    pGC->ops->PolyFillArc	= miPolyFillArc ;
	    pGC->ops->PolySegment	= miPolySegment ;
	}
	else {
	    pGC->ops->CopyArea	= xf4bppCopyArea ;
	    pGC->ops->PolyFillRect	= xf4bppPolyFillRect ;
	    pGC->ops->PushPixels	= miPushPixels ; /* GJA */
	    pGC->ops->PolyArc	= xf4bppZeroPolyArc ;
	    pGC->ops->PolyFillArc	= xf4bppPolyFillArc ;
	    pGC->ops->PolySegment	= xf4bppSegmentSS ;
	}
	return;
}

Mask
xf4bppChangeWindowGC( pGC, changes )
register GC *pGC ;
register Mask changes ;
{
register ppcPrivGCPtr devPriv = (ppcPrivGCPtr)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey());
register unsigned long int idx ; /* used for stepping through bitfields */

#define LOWBIT( x ) ( x & - x ) /* Two's complement */
    while ((idx = LOWBIT(changes))) {
	switch ( idx ) {

	  case GCLineStyle:
	  case GCLineWidth:
	    pGC->ops->PolyArc = ( ( pGC->lineStyle == LineSolid )
		? ( ( pGC->lineWidth == 0 ) ? xf4bppZeroPolyArc
					    : miPolyArc )
		: miPolyArc ) ;
	    pGC->ops->PolySegment = ( ( pGC->lineStyle == LineSolid )
		? ( ( pGC->lineWidth == 0 )
			? ( ( pGC->fillStyle == FillSolid ) ?
				xf4bppSegmentSS : miPolySegment )
			: miPolySegment )
		: ( ( pGC->lineWidth == 0 )
			? ( ( pGC->fillStyle == FillSolid ) ?
				xf4bppSegmentSD : miPolySegment )
			: miPolySegment ) ) ;
	    pGC->ops->Polylines = ( ( pGC->lineStyle == LineSolid )
		? ( ( pGC->lineWidth == 0 )
			? ( (pGC->fillStyle == FillSolid ) ?
				xf4bppLineSS : miZeroLine )
			: miWideLine )
		: ( ( pGC->lineWidth == 0 )
			? ( (pGC->fillStyle == FillSolid ) ?
				xf4bppLineSD : miWideDash )
			: miWideDash ) ) ;
	    /*
	     * If these are just square boxes with no funny business
	     * going on we can call the fast routine that draws
	     * rectangles without floating point.
	     */
/* too buggy */
#if 0
	    if ( ( pGC->lineStyle == LineSolid ) 
			&& ( pGC->joinStyle == JoinMiter ) 
			&& ( pGC->lineWidth != 0 ) )
		pGC->ops->PolyRectangle = xf4bppPolyRectangle;
	    else
#endif
		pGC->ops->PolyRectangle = miPolyRectangle;

	    changes &= ~( GCLineStyle | GCLineWidth ) ;
	    break ;
	  case GCJoinStyle:
	    /*
	     * If these are just square boxes with no funny business
	     * going on we can call the fast routine that draws
	     * rectangles without floating point.
	     */
/* too buggy */
#if 0
	    if ( ( pGC->lineStyle == LineSolid ) 
			&& ( pGC->joinStyle == JoinMiter ) 
			&& ( pGC->lineWidth != 0 ) )
		pGC->ops->PolyRectangle = xf4bppPolyRectangle;
	    else
#endif
		pGC->ops->PolyRectangle = miPolyRectangle;
	    changes &= ~ idx ; /* i.e. changes &= ~ GCJoinStyle */
	    break ;

	  case GCBackground:
	    if ( pGC->fillStyle != FillOpaqueStippled ) {
		changes &= ~ idx ; /* i.e. changes &= ~GCBackground */
		break ;
	    } /* else Fall Through */
	  case GCForeground:
	    if ( pGC->fillStyle == FillTiled ) {
		changes &= ~ idx ; /* i.e. changes &= ~GCForeground */
		break ;
	    } /* else Fall Through */
	  case GCFunction:
	  case GCPlaneMask:
	  case GCFillStyle:
	    { /* new_fill */
		int fillStyle = devPriv->colorRrop.fillStyle ;
		/* install a suitable fillspans */
		if ( fillStyle == FillSolid )
		    pGC->ops->FillSpans = xf4bppSolidWindowFS ;
		else if ( fillStyle == FillStippled )
		    pGC->ops->FillSpans = xf4bppStippleWindowFS ;
		else if ( fillStyle == FillOpaqueStippled )
		    pGC->ops->FillSpans = xf4bppOpStippleWindowFS ;
		else /*  fillStyle == FillTiled */
		    pGC->ops->FillSpans = xf4bppTileWindowFS ;
	    } /* end of new_fill */
	    changes &= ~( GCBackground | GCForeground
		     | GCFunction
		     | GCPlaneMask | GCFillStyle ) ;
	    break ;

	default:
	    ErrorF("xf4bppChangeWindowGC: Unexpected GC Change\n") ;
	    changes &= ~ idx ; /* Remove it anyway */
	    break ;
	}
    }
    return changes;
}
