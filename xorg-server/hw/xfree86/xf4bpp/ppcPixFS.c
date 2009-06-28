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
#include "maskbits.h"
#include "mi.h"
#include "mispans.h"
#include "ppcGCstr.h"
#include "ppcSpMcro.h"
#include "vgaVideo.h"
#include "ibmTrace.h"

#define LeftMostBitInScreenLongWord SCRLEFT( 0xFFFFFFFF, 31 )

/* GJA -- copied this from VGA */
#define SCRLEFT8(lw, n)	( (unsigned char) (((unsigned char) lw) << (n)) )
#define SCRRIGHT8(lw, n)	( (unsigned char) (((unsigned char)lw) >> (n)) )
/*
********** ********** ********** ********** ********** ********** **********
   these routines all clip.  they assume that anything that has called
them has already translated the points (i.e. pGC->miTranslate is
non-zero, which is howit gets set in mfbCreateGC().)

   the number of new scnalines created by clipping ==
MaxRectsPerBand * nSpans.
********** ********** ********** ********** ********** ********** **********
*/
/* A mod definition that goes smoothly into the negative.
 */
static int
modulo
(
      int n1,
      int n2
)
{
      int tmp;
      if ( n1 < 0 ) {
              tmp = (-n1) % n2;
              if ( tmp == 0 ) {
                      return 0;
              } else {
                      return n2 - tmp;
              }
      } else {
              return n1 % n2;
      }
}

void
xf4bppSolidPixmapFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
    DrawablePtr pDrawable ;
    GCPtr	pGC ;
    int		nInit ;			/* number of spans to fill */
    DDXPointPtr pptInit ;		/* pointer to list of start points */
    int		*pwidthInit ;		/* pointer to list of n widths */
    int 	fSorted ;
{
    register unsigned long int pm, npm ;
    register unsigned long int fg ;
    register int alu ;
				/* next three parameters are post-clip */
    int n ;			/* number of spans to fill */
    register DDXPointPtr ppt ;	/* pointer to list of start points */
    register int *pwidth ;	/* pointer to list of n widths */
    register unsigned char *addrl ;	/* pointer to current longword in bitmap */
    int i ;
    int *pwidthFree ;		/* copies of the pointers to free */
    DDXPointPtr pptFree ;

    TRACE(("xf4bppSolidPixmapFS(pDrawable=0x%x, pGC=0x%x, nInit=%d, pptInit=0x%x, pwidthInit=0x%x, fSorted=%d)\n", pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)) ;

    if ( pDrawable->type != DRAWABLE_PIXMAP ) {
	ErrorF("xf4bppSolidPixmapFS: drawable is not a pixmap\n") ;
	return ;
    }

    if ( ( alu = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()))->colorRrop.alu ) == GXnoop )
	return ;

    n = nInit * miFindMaxBand(pGC->pCompositeClip) ;
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
    npm = ( ~ pm ) & ( ( 1 << pDrawable->depth ) - 1 ) ;

    for ( ; n-- ; ppt++, pwidth++ ) {
        addrl = ( (unsigned char *) ( ( (PixmapPtr) pDrawable )->devPrivate.ptr ) )
	      + ( ppt->y * ( (int) ( ( (PixmapPtr) pDrawable )->devKind ) ) )
	      + ppt->x ;
	for ( i = *pwidth ; i-- ; addrl++ )
	    {
	    unsigned _p;
	    DoRop( _p, alu, fg, *addrl );
	    *addrl = ( *addrl & npm ) | ( pm & _p ) ;
	    }
#ifdef notdef /* PURDUE */
	    *addrl = ( *addrl & npm ) | ( pm & DoRop( alu, fg, *addrl ) ) ;
#endif /* PURDUE */
    }
    xfree( pptFree ) ;
    xfree( pwidthFree ) ;
    return ;
}

/* GJA -- copied from vgaStipple.c */
static unsigned char
vgagetbits
(
	register const int x,
	register const unsigned int patternWidth,
	register const unsigned char * const lineptr
)
{
register unsigned char bits ;
register const unsigned char *cptr ;
register int shift ;
register int wrap ;

cptr = lineptr + ( x >> 3 ) ;
bits = *cptr ;
if ((shift = x & 7))
      bits = SCRLEFT8( bits, shift ) | SCRRIGHT8( cptr[1], ( 8 - shift ) ) ;
if ( ( wrap = x + 8 - patternWidth ) > 0 ) {
      bits &= SCRLEFT8( 0xFF, wrap ) ;
      bits |= SCRRIGHT8( *lineptr, ( 8 - wrap ) ) ;
}

/* GJA -- Handle extraction of 8 bits from < 8 bits wide stipple.
 * I duplicated case 4,5,6,7 to give the compiler a chance to optimize.
 */
switch (patternWidth) {
case 1:	/* Not really useful. */
	bits &= ~SCRRIGHT8(0xFF,1);
	bits |= SCRRIGHT8(bits,1); 
	bits |= SCRRIGHT8(bits,2);
	bits |= SCRRIGHT8(bits,4);
	break;
case 2:
	bits &= ~SCRRIGHT8(0xFF,2);
	bits |= SCRRIGHT8(bits,2); bits |= SCRRIGHT8(bits,4); break;
case 3:
	bits &= ~SCRRIGHT8(0xFF,3);
	bits |= (SCRRIGHT8(bits,3) | SCRRIGHT8(bits,6)); break;
case 4:
	bits = (bits & ~SCRRIGHT8(0xFF,4)) | SCRRIGHT8(bits,4); break;
case 5:
	bits = (bits & ~SCRRIGHT8(0xFF,5)) | SCRRIGHT8(bits,5); break;
case 6:
	bits = (bits & ~SCRRIGHT8(0xFF,6)) | SCRRIGHT8(bits,6); break;
case 7:
	bits = (bits & ~SCRRIGHT8(0xFF,7)) | SCRRIGHT8(bits,7); break;
default:
	;
	/* Do nothing, of course */
}

return bits ;
}

void
xf4bppStipplePixmapFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
register DrawablePtr pDrawable ;
GC *pGC ;
int nInit ;			/* number of spans to fill */
DDXPointPtr pptInit ;		/* pointer to list of start points */
int *pwidthInit ;		/* pointer to list of n widths */
int fSorted ;
{
    register unsigned char *pdst ; /* pointer to current word in bitmap */
    register unsigned long int pm, npm ;
    register unsigned long int fg ;
    register int alu ;
				/* next three parameters are post-clip */
    int n ;			/* number of spans to fill */
    register DDXPointPtr ppt ;	/* pointer to list of start points */
    register int *pwidth ;	/* pointer to list of n widths */
    PixmapPtr	pTile ;		/* pointer to tile we want to fill with */
    int		width,  x, xSrc, ySrc ;
    int 	tlwidth, tileWidth ;
    unsigned char *psrcT ;
    int *pwidthFree ;		/* copies of the pointers to free */
    DDXPointPtr pptFree ;
    int xoff, count, stip, i ;

    TRACE(("xf4bppStipplePixmapFS(pDrawable=0x%x, pGC=0x%x, nInit=%d, pptInit=0x%x, pwidthInit=0x%x, fSorted=%d)\n",
		 pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted)) ;

    if ( pDrawable->type != DRAWABLE_PIXMAP ) {
	ErrorF( "xf4bppStippleWindowFS: drawable is not a pixmap\n") ;
	return ;
    }
    if ( pGC->stipple->drawable.depth != 1 ) {
	ErrorF( "ppcStippleFS: bad depth\ntype = %d, depth = %d\n",
		pDrawable->type, pGC->stipple->drawable.depth ) ;
	return ;
    }

    if ( ( alu = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.alu ) == GXnoop )
	return ;

    SETSPANPTRS( nInit, n, pwidthInit, pwidthFree, pptInit,
		pptFree, pwidth, ppt, fSorted ) ;

    pm = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.planemask ;
    fg = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.fgPixel ;

    pTile = pGC->stipple ;
    tlwidth = pTile->devKind ;

    tileWidth = pTile->drawable.width ;

    npm = ( ~ pm ) & ( ( 1 << pDrawable->depth ) - 1 ) ;

    /* this replaces rotating the stipple.  Instead, we just adjust the offset
     * at which we start grabbing bits from the stipple */
    xSrc = pGC->patOrg.x + pDrawable->x;
    ySrc = pGC->patOrg.y + pDrawable->y;

    while ( n-- ) {
        pdst = ( (unsigned char *) ( (PixmapPtr) pDrawable )->devPrivate.ptr )
	     + ( ppt->y * ( (int) ( ( (PixmapPtr) pDrawable )->devKind ) ) )
	     + ppt->x ;
        psrcT = (unsigned char *)pTile->devPrivate.ptr
	      + ( modulo( ppt->y - ySrc, pTile->drawable.height ) * tlwidth ) ;
	x = ppt->x ;

        xoff = modulo( x - xSrc, tileWidth) ;
        for ( width = *pwidth ; width ; width -= count, xoff+=count ) {
 
            if ( xoff >= tileWidth ) xoff -= tileWidth;
 
            if ( width < 8 )
                count = width;
            else
                count = 8;
 
            stip = vgagetbits( xoff, tileWidth, psrcT ) ;
 
            for ( i = count ; i-- ; ) {
                if ( stip & 128 )
		    {
		    unsigned _p;
		    DoRop( _p, alu, fg, *pdst ) ;
		    *pdst = ( *pdst & npm ) | ( pm & _p ) ;
		    }
#ifdef notdef /* PURDUE */
		    *pdst = ( *pdst & npm ) | ( pm & DoRop( alu, fg, *pdst ) ) ;
#endif /* PURDUE */
		pdst++ ;
		stip = SCRLEFT( stip, 1 ) ;
	    }
	}
	ppt++ ;
	pwidth++ ;
    }
    xfree( pptFree ) ;
    xfree( pwidthFree ) ;
    return ;
}

void
xf4bppOpStipplePixmapFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
DrawablePtr pDrawable ;
GC *pGC ;
int nInit ;			/* number of spans to fill */
DDXPointPtr pptInit ;		/* pointer to list of start points */
int *pwidthInit ;		/* pointer to list of n widths */
int fSorted ;
{
    register unsigned char *pdst ;	/* pointer to current word in bitmap */
    register unsigned long int pm, npm ;
    register unsigned long int fg, bg ;
    register int alu ;
				/* next three parameters are post-clip */
    int n ;			/* number of spans to fill */
    register DDXPointPtr ppt ;	/* pointer to list of start points */
    register int *pwidth ;	/* pointer to list of n widths */
    PixmapPtr pTile ;		/* pointer to tile we want to fill with */
    int	width ;
    int xSrc, ySrc ;
    int tlwidth, tileWidth ;
    unsigned char *psrcT ;
    int *pwidthFree ;		/* copies of the pointers to free */
    DDXPointPtr pptFree ;
    int xoff, count, stip, i ;

    TRACE( ( "xf4bppOpStipplePixmapFS(pDrawable=0x%x,pGC=0x%x,nInit=%d,pptInit=0x%x,pwidthInit=0x%x,fSorted=%d)\n",
	   pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted ) ) ;

    if ( pGC->stipple->drawable.depth != 1 ) {
	ErrorF( "xf4bppOpStipplePixmapFS: bad depth\ntype = %d, depth = %d\n",
		pDrawable->type, pGC->stipple->drawable.depth ) ;
	return ;
    }

    if ( ( alu = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.alu ) == GXnoop )
	return ;

    SETSPANPTRS( nInit, n, pwidthInit, pwidthFree, pptInit,
		 pptFree, pwidth, ppt, fSorted ) ;

    fg = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.fgPixel ;
    bg = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.bgPixel ;
    pm = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.planemask ;
    npm = ( ~ pm ) & ( ( 1 << pDrawable->depth ) - 1 ) ;

    pTile = pGC->stipple ;
    tlwidth = pTile->devKind ;
    tileWidth = pTile->drawable.width ;

    xSrc = pGC->patOrg.x + pDrawable->x;
    ySrc = pGC->patOrg.y + pDrawable->y;

    /* this replaces rotating the stipple.  Instead, we just adjust the offset
     * at which we start grabbing bits from the stipple */
    for ( ; n-- ; ppt++, pwidth++ ) {
        pdst = ( (unsigned char *) ( (PixmapPtr) pDrawable )->devPrivate.ptr )
	     + ( ppt->y * ( (int) ( (PixmapPtr) pDrawable )->devKind ) )
	     + ppt->x ;
        psrcT = (unsigned char *)pTile->devPrivate.ptr
            + ( modulo( ppt->y - ySrc, pTile->drawable.height ) * tlwidth ) ;
 
        xoff = modulo( ppt->x - xSrc, tileWidth) ;
 
        for ( width = *pwidth ; width ; width -= count, xoff+=count ) {
 
            if ( xoff >= tileWidth ) xoff -= tileWidth;
 
            if ( width < 8 )
                count = width;
            else
                count = 8;
 
            stip = vgagetbits( xoff, tileWidth, psrcT ) ;
            for ( i = count ; i-- ; pdst++, stip = SCRLEFT( stip, 1 ) )
                if ( stip & 128 )
		    {
		    unsigned _p;
		    DoRop( _p, alu, fg, *pdst ) ;
		    *pdst = ( *pdst & npm ) | ( pm & _p ) ;
		    }
#ifdef notdef /* PURDUE */
		    *pdst = ( *pdst & npm ) | ( pm & DoRop( alu, fg, *pdst ) ) ;
#endif /* PURDUE */
		else
		    {
		    unsigned _p;
		    DoRop( _p, alu, bg, *pdst ) ;
		    *pdst = ( *pdst & npm ) | ( pm & _p ) ;
		    }
#ifdef notdef /* PURDUE */
		    *pdst = ( *pdst & npm ) | ( pm & DoRop( alu, bg, *pdst ) ) ;
#endif /* PURDUE */
	}
    }
    xfree( pptFree ) ;
    xfree( pwidthFree ) ;
    return ;
}

void
xf4bppTilePixmapFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted )
register DrawablePtr pDrawable ;
GC *pGC ;
int nInit ;			/* number of spans to fill */
DDXPointPtr pptInit ;		/* pointer to list of start points */
int *pwidthInit ;		/* pointer to list of n widths */
int fSorted ;
{
    register DDXPointPtr ppt ;	/* pointer to list of start points */
    register int *pwidth ;	/* pointer to list of n widths */
    register unsigned char *pdst ;	/* pointer to current word in bitmap */
    register unsigned char *psrc ;	/* pointer to current word in tile */
    register PixmapPtr pTile ;	/* pointer to tile we want to fill with */
    int i ;
    int alu ;
    unsigned char pm, npm ;
				/* next three parameters are post-clip */
    int n ;			/* number of spans to fill */
    int tileWidth ;
    int xSrc, ySrc;
    unsigned char *psrcT ;
    int *pwidthFree ;		/* copies of the pointers to free */
    DDXPointPtr pptFree ;

    TRACE( ( "ppcTileFS(pDrawable=0x%x,pGC=0x%x,nInit=%d,pptInit=0x%x,pwidthInit=0x%x,fSorted=%d)\n",
	    pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted ) ) ;

    if ( ( pDrawable->depth == 1 ) && ( pDrawable->type == DRAWABLE_PIXMAP ) ) {
	mfbTileFS( pDrawable, pGC, nInit, pptInit, pwidthInit, fSorted ) ;
	return ;
    }
    if ( !xf4bppDepthOK( pDrawable, pGC->tile.pixmap->drawable.depth ) ) {
	ErrorF( "ppcTileFS: bad depth\ntype = %d, depth = %d\n",
		pDrawable->type, pDrawable->depth) ;
	return ;
    }

    if ( ( alu = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.alu ) == GXnoop )
	return ;

    SETSPANPTRS( nInit, n, pwidthInit, pwidthFree, pptInit,
		 pptFree, pwidth, ppt, fSorted ) ;

    /* the following code is for 8 bits per pixel addressable memory only */
    pm = ( (ppcPrivGC *)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()) )->colorRrop.planemask ;
    npm = ( ~ pm ) & ( ( 1 << pDrawable->depth ) - 1 ) ;
    pTile = pGC->tile.pixmap ;
    tileWidth = pTile->drawable.width ;

     xSrc = pGC->patOrg.x + pDrawable->x;
     ySrc = pGC->patOrg.y + pDrawable->y;
    /* this replaces rotating the tile. Instead we just adjust the offset
     * at which we start grabbing bits from the tile */
    for ( ; n-- ; ppt++, pwidth++ ) {
        pdst = ( (unsigned char *) ( (PixmapPtr) pDrawable )->devPrivate.ptr )
	     + ( ppt->y * ( (int) ( (PixmapPtr) pDrawable )->devKind ) )
	     + ppt->x ;
        psrcT = (unsigned char *) pTile->devPrivate.ptr
        + ( modulo( ppt->y - ySrc, pTile->drawable.height) * pTile->devKind ) ;
 
        psrc = psrcT + modulo( ppt->x - xSrc, tileWidth ) ;
	for ( i = *pwidth ; i-- ; pdst++, psrc++ ) {
	    if ( psrc >= ( psrcT + tileWidth ) )
		psrc = psrcT ;
	    {
	    unsigned _p;
	    DoRop( _p, alu, *psrc, *pdst ) ;
	    *pdst = ( *pdst & npm ) | ( pm & _p ) ;
	    }
#ifdef notdef /* PURDUE */
	    *pdst = ( *pdst & npm ) | ( pm & DoRop( alu, *psrc, *pdst ) ) ;
#endif /* PURDUE */
	}
    }
    xfree( pptFree ) ;
    xfree( pwidthFree ) ;
    return ;
}
