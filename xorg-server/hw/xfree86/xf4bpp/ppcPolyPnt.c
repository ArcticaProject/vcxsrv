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
#include "scrnintstr.h"
#include "ppcGCstr.h"
#include "ibmTrace.h"

void
xf4bppPolyPoint( pDrawable, pGC, mode, npt, pptInit )
DrawablePtr	pDrawable ;
GCPtr		pGC ;
int		mode ;				/* Origin or Previous */
int		npt ;
xPoint		*pptInit ;
{
register xPoint *ppt ;
ppcPrivGC *devPriv ;
int alu ;
int nptTmp ;

TRACE( ("xf4bppPolyPoint(0x%x,0x%x,%d,%d,0x%x)\n",
	pDrawable, pGC, mode, npt, pptInit ) ) ;

if ( pDrawable->type == DRAWABLE_PIXMAP ) {
	if ( pGC->alu != GXnoop )
		miPolyPoint( pDrawable, pGC, mode, npt, pptInit ) ;
	return ;
}

devPriv = (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey());
if ( ( alu = devPriv->colorRrop.alu ) == GXnoop )
	return ;

/* make pointlist origin relative */
if ( mode == CoordModePrevious )
	for ( ppt = pptInit, nptTmp = npt ; --nptTmp ; ) {
		ppt++ ;
		ppt->x += (ppt-1)->x ;
		ppt->y += (ppt-1)->y ;
	}

if ( pGC->miTranslate ) {
	register int xorg = pDrawable->x ;
	register int yorg = pDrawable->y ;
	for ( ppt = pptInit, nptTmp = npt ; nptTmp-- ; ppt++ ) {
		ppt->x += xorg ;
		ppt->y += yorg ;
	}
}

{
	register RegionPtr pRegion = pGC->pCompositeClip ;
	register unsigned long int fg = devPriv->colorRrop.fgPixel ;
	register unsigned long int pm = devPriv->colorRrop.planemask ;
	BoxRec box ; /* Scratch Space */

	if ( ! REGION_NUM_RECTS(pRegion))
		return ;

	for ( ppt = pptInit ; npt-- ; ppt++ )
		if (POINT_IN_REGION(pDrawable->pScreen, pRegion,
				    ppt->x, ppt->y, &box))
			xf4bppFillSolid( (WindowPtr)pDrawable,
				fg, alu, pm, ppt->x, ppt->y, 1, 1 ) ;
}

return ;
}
