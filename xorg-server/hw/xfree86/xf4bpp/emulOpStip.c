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

/* ppc OpaqueStipple
 *
 * Based on the private stipple; does a foreground, and then an inverted
 * on the background 
 *
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf4bpp.h"
#include "OScompiler.h"
#include "mfbmap.h"
#include "mfb.h"

void 
xf4bppOpaqueStipple( pWin, pStipple, fg, bg, alu, planes, x, y, w, h, xSrc, ySrc )
WindowPtr pWin; /* GJA */
register PixmapPtr pStipple ;
unsigned long int fg ;
unsigned long int bg ;
int alu ;
unsigned long int planes ;
register int x, y, w, h ;
int xSrc, ySrc ;
{
    /* DO BACKGROUND */
    switch ( alu ) {
	/* Easy Cases -- i.e. Final Result Doesn't Depend On Initial Dest. */
	case GXclear:		/* 0x0 Zero 0 */
	case GXset:		/* 0xf 1 */
 	    /* Foreground And Background Are Both The Same !! */
	    xf4bppFillSolid( pWin, bg, alu, planes, x, y, w, h ) ;
	case GXnoop:		/* 0x5 dst */
	    break ;
	case GXcopy:		/* 0x3 src */
	case GXcopyInverted:	/* 0xc NOT src */
	    { /* Special Case Code */
 		register int vtarget, htarget ;

 		/* We Can Draw Just One Copy Then Blit The Rest !! */
		/* Draw The One Copy */
		htarget = MIN( w, pStipple->drawable.width ) ;
		vtarget = MIN( h, pStipple->drawable.height ) ;

		/* First The Background */
		xf4bppFillSolid( pWin, bg, alu, planes, x, y,
					htarget, vtarget ) ;
		/* Then The Foreground */
		xf4bppFillStipple( pWin, pStipple, fg, alu, planes,
				       x, y, htarget, vtarget,
				       xSrc, ySrc ) ;

		/* Here We Double The Size Of The BLIT Each Iteration */
		xf4bppReplicateArea(pWin, x, y, planes, w, h, htarget, vtarget);
	    }
	    break ;
	default:
	/* Hard Cases -- i.e. Final Result DOES Depend On Initial Dest. */
	    { /* Do The Background */
		register int i, j;
		register PixmapPtr pInvPixmap = xf4bppCopyPixmap( pStipple ) ;
		register unsigned char *data = pInvPixmap->devPrivate.ptr ;

		/* INVERT PIXMAP  OK, jeff, this is for you */
		for ( i = pInvPixmap->drawable.height ; i-- ; )
			for ( j = pInvPixmap->devKind ; j-- ; data++ )
				*data = ~ ( *data ) ;

	        xf4bppFillStipple( pWin, pInvPixmap, bg, alu, planes, x, y, w, h, xSrc, ySrc );
	        mfbDestroyPixmap( pInvPixmap ) ;
	        /* DO FOREGROUND */
	        xf4bppFillStipple( pWin, pStipple, fg, alu, planes, x, y, w, h, xSrc, ySrc );
	    }
	break ;
    }
    return;
}
