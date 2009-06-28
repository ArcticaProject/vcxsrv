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

/* ppc Replicate Area -- A Divide & Conquer Algorithm
 * a "ppc" Helper Function For Stipples And Tiling
 * P. Shupak 1/88
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf4bpp.h"

void xf4bppReplicateArea( pWin, x, y, planeMask, goalWidth, goalHeight,
			currentHoriz, currentVert)
WindowPtr pWin; /* GJA */
register int x, y, planeMask ;
int goalWidth, goalHeight ;
int currentHoriz, currentVert ;
{
	for ( ;
	      currentHoriz <= ( goalWidth >> 1 ) ;
	      currentHoriz <<= 1 ) {
		xf4bppBitBlt( pWin, GXcopy, planeMask,
			x, y,
			x + currentHoriz, y,
			currentHoriz, currentVert ) ;
	}
	if ( goalWidth - currentHoriz )
		xf4bppBitBlt( pWin, GXcopy, planeMask,
			x, y,
			x + currentHoriz, y,
			goalWidth - currentHoriz, currentVert ) ;
	for ( ;
	      currentVert <= ( goalHeight >> 1 ) ;
	      currentVert <<= 1 ) {
		xf4bppBitBlt( pWin, GXcopy, planeMask,
			x, y,
			x, y + currentVert,
			goalWidth, currentVert ) ;
	}
	if ( goalHeight - currentVert )
		xf4bppBitBlt( pWin, GXcopy, planeMask,
			x, y,
			x, y + currentVert,
			goalWidth, goalHeight - currentVert ) ;
return ;
}
