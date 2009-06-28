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

#include "xf4bpp.h"
#include "ppcGCstr.h"

/* xf4bppGetReducedColorRrop( pGC, drawableDepth, returnLoc )
 * An attempt to do "strength reduction" on color raster-ops
 * P. Shupak 1/88
 */

static void 
ppcReduceGeneral
(
	register int		alu,
	register unsigned long	pm,
	register unsigned long	fg,
	register unsigned long	bg,
	register int		fillStyle,
	int			drawableDepth,
	ppcReducedRrop		*returnLoc
)
{

if ( ( alu == GXnoop )
  || !( pm &= ( ( 1 << drawableDepth ) - 1 ) ) ) {
	returnLoc->alu = GXnoop ;
	return ;
}

#ifdef DELETE_THIS
switch ( fillStyle ) {
	case FillTiled:
		switch ( alu ) {
			case GXclear:		/* 0x0 Zero 0 */
			case GXinvert:		/* 0xa NOT dst */
			case GXset:		/* 0xf 1 */
				fillStyle = FillSolid ;
			default: /* We Can't Do Much Here */
				break ;
		}
		break ;
	case FillOpaqueStippled:
		if ( ( fg & pm ) != ( bg & pm ) ) { /* else FillSolid */
			switch ( alu ) {
				case GXclear:	/* 0x0 Zero 0 */
				case GXset:	/* 0xf 1 */
				case GXinvert:	/* 0xa NOT dst */
					fillStyle = FillSolid ;
					break ;
				case GXnor:	/* 0x8 NOT src AND NOT dst */
				case GXnand:	/* 0xe NOT src OR NOT dst */
				case GXcopy:	/* 0x3 src */
					break ;
				case GXandReverse: /* 0x2 src AND NOT dst */
					fg = ~fg ;
					bg = ~bg ;
					alu = GXnor ;
					break ;
				case GXandInverted: /* 0x4 NOT src AND dst */
					fg = ~fg ;
					bg = ~bg ;
					alu = GXand ; /* Fall Through */
				case GXand:	/* 0x1 src AND dst */
					pm &= ~( fg & bg ) ;
					if ( ( bg & pm ) == pm ) {
						fillStyle = FillStippled ;
						alu = GXclear ;
					}
					break ;
				case GXequiv:	/* 0x9 NOT src XOR dst */
					fg = ~fg ;
					bg = ~bg ;
					alu = GXxor ; /* Fall Through */
				case GXxor:	/* 0x6 src XOR dst */
					pm &= ( fg | bg ) ;
					if ( !( bg & pm ) ) {
						fillStyle = FillStippled ;
						alu = GXinvert ;
					}
					break ;
				case GXorReverse:	/* 0xb src OR NOT dst */
					fg = ~fg ;
					bg = ~bg ;
					alu = GXnand ;
					break ;
				case GXcopyInverted:	/* 0xc NOT src */
					fg = ~fg ;
					bg = ~bg ;
					alu = GXcopy ;
					break ;
				case GXorInverted:	/* 0xd NOT src OR dst */
					fg = ~fg ;
					bg = ~bg ;
					alu = GXor ; /* Fall Through */
				case GXor:	/* 0x7 src OR dst */
					pm &= ( fg | bg ) ;
					if ( !( bg & pm ) ) {
						fillStyle = FillStippled ;
						alu = GXset ;
					}
					break ;
				default:
					ErrorF(
			 "xf4bppGetReducedColorRrop: Unknown Alu Raster-Op" ) ;
					break ;
			}
			break ; /* Don't Fall Through */
		}
		else
			fillStyle = FillSolid ;
			/* Fall Through */
	case FillStippled:
	case FillSolid:
		switch ( alu ) {
			case GXclear:		/* 0x0 Zero 0 */
			case GXset:		/* 0xf 1 */
			case GXinvert:		/* 0xa NOT dst */
				break ;
			case GXand:		/* 0x1 src AND dst */
				pm &= ~fg ;
				alu = GXclear ;
				break ;
			case GXandReverse:	/* 0x2 src AND NOT dst */
				fg = ~fg ;
				alu = GXnor ; /* Fall Through */
			case GXnor:		/* 0x8 NOT src AND NOT dst */
				if ( !( fg & pm ) )
					alu = GXclear ;
				else if ( ( fg & pm ) == pm )
					alu = GXinvert ;
				break ;
			case GXandInverted:	/* 0x4 NOT src AND dst */
				pm &= fg ;
				alu = GXclear ;
				break ;
			case GXxor:		/* 0x6 src XOR dst */
				pm &= fg ;
				alu = GXinvert ;
				break ;
			case GXor:		/* 0x7 src OR dst */
				pm &= fg ;
				alu = GXset ;
				break ;
			case GXequiv:		/* 0x9 NOT src XOR dst */
				pm &= ~fg ;
				alu = GXinvert ;
				break ;
			case GXorReverse:	/* 0xb src OR NOT dst */
				fg = ~fg ;
				alu = GXnand ; /* Fall Through */
			case GXnand:		/* 0xe NOT src OR NOT dst */
				if ( !( fg & pm ) )
					alu = GXset ;
				else if ( ( fg & pm ) == pm )
					alu = GXinvert ;
				break ;
			case GXcopyInverted:	/* 0xc NOT src */
				fg = ~fg ;
				alu = GXcopy ; /* Fall Through */
			case GXcopy:		/* 0x3 src */
				if ( !( fg & pm ) )
					alu = GXclear ;
				else if ( ( fg & pm ) == pm )
					alu = GXset ;
				break ;
			case GXorInverted:	/* 0xd NOT src OR dst */
				pm &= ~fg ;
				alu = GXset ;
				break ;
			default:
				ErrorF(
			 "xf4bppGetReducedColorRrop: Unknown Alu Raster-Op" ) ;
				break ;
		}
		break;
	default:
		ErrorF("xf4bppGetReducedColorRrop: Bad Fillstyle\n");
		break;
}
#endif

/* Final Test On Restricted Plane Mask */
if ( !pm )
	alu = GXnoop ;

/* Set Actual Returned Values */
returnLoc->planemask = pm ;
returnLoc->fgPixel   = fg ;
returnLoc->bgPixel   = bg ;
returnLoc->alu       = alu ;
returnLoc->fillStyle = fillStyle ;

return ;
}

void 
xf4bppGetReducedColorRrop( pGC, drawableDepth, returnLoc )
GC		*pGC ;
int		drawableDepth ;
ppcReducedRrop	*returnLoc ;
{

ppcReduceGeneral( pGC->alu,
		  pGC->planemask,
		  pGC->fgPixel,
		  pGC->bgPixel,
 		  pGC->fillStyle,
		  drawableDepth,
		  returnLoc ) ;

return ;
}
