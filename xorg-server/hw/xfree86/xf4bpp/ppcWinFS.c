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

/******************************************************************
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

#include "xf4bpp.h"
#include "mfbmap.h"
#include "mfb.h"
#include "mi.h"
#include "mispans.h"
#include "ppcGCstr.h"
#include "ppcSpMcro.h"
#include "ibmTrace.h"

#define LeftMostBitInScreenLongWord SCRLEFT( 0xFFFFFFFF, 31 )
/*
********** ********** ********** ********** ********** ********** **********
   these routines all clip.  they assume that anything that has called
them has already translated the points (i.e. pGC->miTranslate is
non-zero, which is howit gets set in mfbCreateGC().)

   the number of new scanlines created by clipping ==
MaxRectsPerBand * nSpans.
********** ********** ********** ********** ********** ********** **********
*/

void
xf4bppSolidWindowFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
    DrawablePtr pDrawable ;
    GCPtr	pGC ;
    int		nInit ;			/* number of spans to fill */
    DDXPointPtr pptInit ;		/* pointer to list of start points */
    int		*pwidthInit ;		/* pointer to list of n widths */
    int 	fSorted ;
{
    register unsigned long int pm ;
    register unsigned long int fg ;
    register int alu ;
				/* next three parameters are post-clip */
    int n ;			/* number of spans to fill */
    register DDXPointPtr ppt ;	/* pointer to list of start points */
    register int *pwidth ;	/* pointer to list of n widths */
    int *pwidthFree ;		/* copies of the pointers to free */
    DDXPointPtr pptFree ;

    TRACE( ( "xf4bppSolidWindowFS(pDrawable=0x%x,pGC=0x%x,nInit=%d,pptInit=0x%x,pwidthInit=0x%x,fSorted=%d)\n",
	    pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted ) ) ;

    if ( pDrawable->type != DRAWABLE_WINDOW ) {
	ErrorF( "xf4bppSolidWindowFS: drawable is not a window\n") ;
	return ;
    }

    if ( ( alu = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.alu ) == GXnoop )
	return ;

    n = nInit * miFindMaxBand( pGC->pCompositeClip ) ;
    if ( !( pwidthFree = (int *) xalloc( n * sizeof( int ) ) ) )
	return ;
    pwidth = pwidthFree ;

    if ( !( pptFree = (DDXPointRec *)
			xalloc( n * sizeof( DDXPointRec ) ) ) ) {
	xfree( pwidth ) ;
	return ;
    }
    ppt = pptFree ;

    n = miClipSpans( pGC->pCompositeClip, pptInit, pwidthInit, nInit,
	ppt, pwidth, fSorted ) ;

    pm = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.planemask ;
    fg = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.fgPixel ;

    for ( ; n-- ; ppt++, pwidth++ )
	if ( *pwidth )
	    xf4bppFillSolid( (WindowPtr)pDrawable,
		           fg, alu, pm, ppt->x, ppt->y, *pwidth, 1 ) ;

    xfree( pptFree ) ;
    xfree( pwidthFree ) ;
    return ;
}

void
xf4bppStippleWindowFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
DrawablePtr pDrawable ;
register GC *pGC ;
int nInit ;			/* number of spans to fill */
DDXPointPtr pptInit ;		/* pointer to list of start points */
int *pwidthInit ;		/* pointer to list of n widths */
int fSorted ;
{
    register unsigned long int pm ;
    register unsigned long int fg ;
    register int alu ;
				/* next three parameters are post-clip */
    int n ;			/* number of spans to fill */
    register DDXPointPtr ppt ;	/* pointer to list of start points */
    register int *pwidth ;	/* pointer to list of n widths */
    PixmapPtr	pTile ;		/* pointer to tile we want to fill with */
    int xSrc ;
    int ySrc ;
    int *pwidthFree ;		/* copies of the pointers to free */
    DDXPointPtr pptFree ;

    TRACE( ( "xf4bppStippleWindowFS(pDrawable=0x%x,pGC=0x%x,nInit=%d,pptInit=0x%x,pwidthInit=0x%x,fSorted=%d)\n",
	   pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted ) ) ;

    if ( pDrawable->type != DRAWABLE_WINDOW ) {
	ErrorF( "xf4bppStippleWindowFS: drawable is not a window\n" ) ;
	return ;
    }

    if ( pGC->stipple->drawable.depth != 1 ) {
	ErrorF("ppcStippleFS: bad depth\ntype = %d, depth = %d\n",
		pDrawable->type, pGC->stipple->drawable.depth ) ;
	return ;
    }

    if ( ( alu = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.alu ) == GXnoop )
	return ;

    SETSPANPTRS( nInit, n, pwidthInit, pwidthFree, pptInit,
		 pptFree, pwidth, ppt, fSorted ) ;

    pm = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.planemask ;
    fg = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.fgPixel ;

    xSrc = pGC->patOrg.x + pDrawable->x ;
    ySrc = pGC->patOrg.y + pDrawable->y ;
    pTile = pGC->stipple ;

    for ( ; n-- ; ppt++, pwidth++ )
	xf4bppFillStipple( (WindowPtr)pDrawable, pTile, fg, alu, pm,
			ppt->x, ppt->y, *pwidth, 1, xSrc, ySrc ) ;

    xfree( pptFree ) ;
    xfree( pwidthFree ) ;

    return ;
}

void
xf4bppOpStippleWindowFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
DrawablePtr pDrawable ;
register GC *pGC ;
int nInit ;			/* number of spans to fill */
DDXPointPtr pptInit ;		/* pointer to list of start points */
int *pwidthInit ;		/* pointer to list of n widths */
int fSorted ;
{
    register DDXPointPtr ppt ;	/* pointer to list of start points */
    register int *pwidth ;	/* pointer to list of n widths */
    int n ;			/* number of spans to fill */
    int xSrc ;
    int ySrc ;
    unsigned long int pm ;
    unsigned long int fg, bg ;
    int alu ;
    int *pwidthFree ;		/* copies of the pointers to free */
    DDXPointPtr pptFree ;

    TRACE( ( "xf4bppOpStippleWindowFS(pDrawable=0x%x,pGC=0x%x,nInit=%d,pptInit=0x%x,pwidthInit=0x%x,fSorted=%d)\n",
	    pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted ) ) ;

    if ( pGC->stipple->drawable.depth != 1 ) {
	ErrorF( "xf4bppOpStippleWindowFS: bad depth\ntype = %d, depth = %d\n",
		pDrawable->type, pGC->stipple->drawable.depth ) ;
	return ;
    }

    if ( ( alu = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.alu ) == GXnoop )
	return ;

    SETSPANPTRS( nInit, n, pwidthInit, pwidthFree, pptInit,
		 pptFree, pwidth, ppt, fSorted ) ;

    pm = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.planemask ;
    fg = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.fgPixel ;
    bg = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.bgPixel ;

    xSrc = pGC->patOrg.x + pDrawable->x ;
    ySrc = pGC->patOrg.y + pDrawable->y ;

    for ( ; n-- ; ppt++, pwidth++ )
	xf4bppOpaqueStipple( (WindowPtr)pDrawable, pGC->stipple, fg, bg, alu, pm,
		ppt->x, ppt->y, *pwidth, 1, xSrc, ySrc ) ;

    xfree( pptFree ) ;
    xfree( pwidthFree ) ;
    return ;
}

void
xf4bppTileWindowFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
DrawablePtr pDrawable ;
register GC *pGC ;
int nInit ;			/* number of spans to fill */
DDXPointPtr pptInit ;		/* pointer to list of start points */
int *pwidthInit ;		/* pointer to list of n widths */
int fSorted ;
{
				/* next three parameters are post-clip */
    register DDXPointPtr ppt ;	/* pointer to list of start points */
    register int *pwidth ;	/* pointer to list of n widths */
    int n ;			/* number of spans to fill */
    unsigned char pm ;
    int alu ;
    int xSrc ;
    int ySrc ;
    int *pwidthFree ;		/* copies of the pointers to free */
    DDXPointPtr pptFree ;

    TRACE( ( "xf4bppTileWindowFS(pDrawable=0x%x,pGC=0x%x,nInit=%d,pptInit=0x%x,pwidthInit=0x%x,fSorted=%d)\n",
	    pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted ) ) ;

    if ( ( alu = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.alu ) == GXnoop )
	return ;

    SETSPANPTRS( nInit, n, pwidthInit, pwidthFree, pptInit,
		 pptFree, pwidth, ppt, fSorted ) ;

    xSrc = pGC->patOrg.x + pDrawable->x ;
    ySrc = pGC->patOrg.y + pDrawable->y ;
    pm = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.planemask ;

    for ( ; n-- ; ppt++, pwidth++ )
	    xf4bppTileRect( (WindowPtr)pDrawable, pGC->tile.pixmap, alu, pm,
		     ppt->x, ppt->y, *pwidth, 1, xSrc, ySrc ) ;

    xfree( pptFree ) ;
    xfree( pwidthFree ) ;
    return ;
}
