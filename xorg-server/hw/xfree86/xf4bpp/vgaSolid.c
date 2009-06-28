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
#include "OScompiler.h"
#include "vgaReg.h"
#include "vgaVideo.h"

#include "xf86str.h" /* for pScrn->vtSema */
extern ScrnInfoPtr *xf86Screens;

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#ifndef	PC98_EGC
#ifdef USE_ASM
extern void fastFill();
extern void fastFillRMW();
#else

static void fastFill
(
	register volatile unsigned char *destination,
	register const unsigned int bytes_per_line,
	register const unsigned int bytewidth,	/* MUST BE > 0 !! */
	register unsigned int height		/* MUST BE > 0 !! */
)
{
int stop_count = bytewidth ;
register int row_jump = bytes_per_line - bytewidth ;
#if !defined(OLDHC) && defined(BSDrt) && !defined(__i386__)
register const unsigned int notZero = ((unsigned char)(~0x0));
#else
#define notZero ((unsigned char)(~0))
#endif

#define SINGLE_STORE \
    ( *( (VgaMemoryPtr) destination ) = notZero ); \
    destination++; stop_count--;

/* TOP OF FIRST LOOP */
BranchPoint:

switch ( bytewidth & 0xF ) { /* Jump into loop at mod 16 remainder */
	LoopTop :
	case 0x0 : SINGLE_STORE ;
	case 0xF : SINGLE_STORE ;
	case 0xE : SINGLE_STORE ;
	case 0xD : SINGLE_STORE ;
	case 0xC : SINGLE_STORE ;
	case 0xB : SINGLE_STORE ;
	case 0xA : SINGLE_STORE ;
	case 0x9 : SINGLE_STORE ;
	case 0x8 : SINGLE_STORE ;
	case 0x7 : SINGLE_STORE ;
	case 0x6 : SINGLE_STORE ;
	case 0x5 : SINGLE_STORE ;
	case 0x4 : SINGLE_STORE ;
	case 0x3 : SINGLE_STORE ;
	case 0x2 : SINGLE_STORE ;
	case 0x1 : SINGLE_STORE ;
/* FIRST LOOP */
		if ( stop_count )
			goto LoopTop ;
/* SECOND LOOP */
		if ( --height ) {
			destination += row_jump ;
			stop_count = bytewidth ;
			goto BranchPoint ;
		}
		else
			return ;
#undef SINGLE_STORE
}
/*NOTREACHED*/
}

/* For Read-Modify-Write Case */
static void fastFillRMW
(
	register volatile unsigned char *destination,
	register const unsigned int bytes_per_line,
	register const unsigned int bytewidth,	/* MUST BE > 0 !! */
	register unsigned int height		/* MUST BE > 0 !! */
)
{
int stop_count = bytewidth ;
register int row_jump = bytes_per_line - bytewidth ;
#if !defined(OLDHC) && defined(BSDrt) && !defined(__i386__)
register const unsigned int notZero = ((unsigned char)(~0x0));
#endif
register int tmp ;

#define SINGLE_STORE \
    tmp = *( (VgaMemoryPtr) destination ) ;  (void)tmp; \
    ( *( (VgaMemoryPtr) destination ) = notZero ) ; \
    destination++; stop_count-- ;

/* TOP OF FIRST LOOP */
BranchPoint:

switch ( bytewidth & 0xF ) { /* Jump into loop at mod 16 remainder */
	LoopTop :
	case 0x0 : SINGLE_STORE ;
	case 0xF : SINGLE_STORE ;
	case 0xE : SINGLE_STORE ;
	case 0xD : SINGLE_STORE ;
	case 0xC : SINGLE_STORE ;
	case 0xB : SINGLE_STORE ;
	case 0xA : SINGLE_STORE ;
	case 0x9 : SINGLE_STORE ;
	case 0x8 : SINGLE_STORE ;
	case 0x7 : SINGLE_STORE ;
	case 0x6 : SINGLE_STORE ;
	case 0x5 : SINGLE_STORE ;
	case 0x4 : SINGLE_STORE ;
	case 0x3 : SINGLE_STORE ;
	case 0x2 : SINGLE_STORE ;
	case 0x1 : SINGLE_STORE ;
/* FIRST LOOP */
		if ( stop_count )
			goto LoopTop ;
/* SECOND LOOP */
		if ( --height ) {
			destination += row_jump ;
			stop_count = bytewidth ;
			goto BranchPoint ;
		}
		else
			return ;
}
#undef SINGLE_STORE
/*NOTREACHED*/
}
#endif


void xf4bppFillSolid( pWin, color, alu, planes, x0, y0, lx, ly )
WindowPtr pWin; /* GJA */
unsigned long int color ;
const int alu ;
unsigned long int planes ;
register int x0 ;
register const int y0 ;
register int lx ;
register const int ly ;		/* MUST BE > 0 !! */
{
IOADDRESS REGBASE;
register volatile unsigned char *dst ;
register int tmp ;
register int tmp2 ;
register int tmp3 ;
unsigned int data_rotate_value = VGA_COPY_MODE ;
unsigned int read_write_modify = FALSE ;
unsigned int invert_existing_data = FALSE ;

{	/* Start GJA */
	if ( !xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->vtSema ) {
		xf4bppOffFillSolid( pWin, color, alu, planes, x0, y0, lx, ly );
		return;
	}
}	/* End GJA */

if ( ( lx == 0 ) || ( ly == 0 ) )
	return;

switch ( alu ) {
	case GXclear:		/* 0x0 Zero 0 */
		color = 0 ;
		break ;
	case GXnor:		/* 0x8 NOT src AND NOT dst */
		invert_existing_data = TRUE ;
	case GXandInverted:	/* 0x4 NOT src AND dst */
		color = ~color ;
	case GXand:		/* 0x1 src AND dst */
		data_rotate_value = VGA_AND_MODE ;
		read_write_modify = TRUE ;
	case GXcopy:		/* 0x3 src */
		break ;
	case GXnoop:		/* 0x5 dst */
		return ;
	case GXequiv:		/* 0x9 NOT src XOR dst */
		color = ~color ;
	case GXxor:		/* 0x6 src XOR dst */
		data_rotate_value = VGA_XOR_MODE ;
		read_write_modify = TRUE ;
		planes &= color ;
		break ;
	case GXandReverse:	/* 0x2 src AND NOT dst */
		invert_existing_data = TRUE ;
		data_rotate_value = VGA_AND_MODE ;
		read_write_modify = TRUE ;
		break ;
	case GXorReverse:	/* 0xb src OR NOT dst */
		invert_existing_data = TRUE ;
		data_rotate_value = VGA_OR_MODE ;
		read_write_modify = TRUE ;
		break ;
	case GXnand:		/* 0xe NOT src OR NOT dst */
		invert_existing_data = TRUE ;
	case GXorInverted:	/* 0xd NOT src OR dst */
		color = ~color ;
	case GXor:		/* 0x7 src OR dst */
		data_rotate_value = VGA_OR_MODE ;
		read_write_modify = TRUE ;
		break ;
	case GXcopyInverted:	/* 0xc NOT src */
		color = ~color ;
		break ;
	case GXinvert:		/* 0xa NOT dst */
		data_rotate_value = VGA_XOR_MODE ;
		read_write_modify = TRUE ;
	case GXset:		/* 0xf 1 */
		color = VGA_ALLPLANES ;
	default:
		break ;
}

if ( !( planes &= VGA_ALLPLANES ) )
	return ;

REGBASE =
    xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->domainIOBase + 0x300;

/*
 * Set The Plane-Enable
 */
SetVideoSequencer( Mask_MapIndex, planes ) ;
SetVideoGraphics( Enb_Set_ResetIndex, planes ) ;
/*
 * Put Display Into SET/RESET Write Mode
 */
SetVideoGraphics( Graphics_ModeIndex, VGA_WRITE_MODE_3 ) ;
/*
 * Set The Color in The Set/Reset Register
 */
SetVideoGraphics( Set_ResetIndex, color & VGA_ALLPLANES ) ;
/*
 * Set The Function-Select In The Data Rotate Register
 */
SetVideoGraphics( Data_RotateIndex, data_rotate_value ) ;

/* Do Left Edge */
if ((tmp = x0 & 07)) {
	tmp2 = SCRRIGHT8( ( (unsigned) 0xFF ), tmp ) ;
	/* Catch The Cases Where The Entire Region Is Within One Byte */
	if ( ( lx -= 8 - tmp ) < 0 ) {
		tmp2 &= SCRLEFT8( 0xFF, -lx ) ;
		lx = 0 ;
	}
	/* Set The Bit Mask Reg */
        SetVideoGraphics(Bit_MaskIndex, tmp2 ) ;
	if ( invert_existing_data == TRUE ) {
                SetVideoGraphics( Set_ResetIndex, VGA_ALLPLANES ) ;
		SetVideoGraphics( Data_RotateIndex, VGA_XOR_MODE ) ;
		dst = SCREENADDRESS( pWin, x0, y0 );
		for ( tmp = ly;
		      tmp-- ; ) {
			tmp3 = *( (VgaMemoryPtr) dst ) ;
			(void)tmp3;
			*( (VgaMemoryPtr) dst ) = tmp2 ;
			dst += BYTES_PER_LINE(pWin);
		}
                SetVideoGraphics( Set_ResetIndex, color & VGA_ALLPLANES ) ;
		SetVideoGraphics( Data_RotateIndex, data_rotate_value ) ;
			/* Un-Set XOR */
	}
	dst = SCREENADDRESS( pWin, x0, y0 );
	for ( tmp = ly;
	      tmp-- ; ) {
		tmp3 = *( (VgaMemoryPtr) dst ) ;
		(void)tmp3;
		*( (VgaMemoryPtr) dst ) = tmp2 ;
		dst += BYTES_PER_LINE(pWin);
	}
	if ( !lx ) { /* All Handled In This Byte */
		return ;
	}
	x0 = ( x0 + 8 ) & ~07 ;
}

/* Fill The Center Of The Box */
if ( ROW_OFFSET( lx ) ) {
	SetVideoGraphics(Bit_MaskIndex, 0xFF ) ;
	if ( invert_existing_data == TRUE ) {
                SetVideoGraphics( Set_ResetIndex, VGA_ALLPLANES ) ;
		SetVideoGraphics( Data_RotateIndex, VGA_XOR_MODE ) ;
		fastFillRMW( SCREENADDRESS( pWin, x0, y0 ),
			     BYTES_PER_LINE(pWin),
			     ROW_OFFSET( lx ), ly ) ;
                SetVideoGraphics( Set_ResetIndex, color & VGA_ALLPLANES ) ;
		SetVideoGraphics( Data_RotateIndex, data_rotate_value ) ;
			/* Un-Set XOR */
		/* Point At The Bit Mask Reg */
	}
	(* ( ( read_write_modify == FALSE ) ? fastFill : fastFillRMW ) )
		( SCREENADDRESS( pWin, x0, y0 ), BYTES_PER_LINE(pWin),
		  ROW_OFFSET( lx ), ly ) ;
}

/* Do Right Edge */
if ((tmp = BIT_OFFSET(lx))) { /* x0 Now Is Byte Aligned */
	/* Set The Bit Mask */
	SetVideoGraphics( Bit_MaskIndex,
		(tmp2 = SCRLEFT8( 0xFF, ( 8 - tmp ) ) ) ) ;
	if ( invert_existing_data == TRUE ) {
                SetVideoGraphics( Set_ResetIndex, VGA_ALLPLANES ) ;
		SetVideoGraphics( Data_RotateIndex, VGA_XOR_MODE ) ;
		dst = SCREENADDRESS( pWin, ( x0 + lx ), y0 );
		for ( tmp = ly; 
		      tmp-- ; ) {
			tmp3 = *( (VgaMemoryPtr) dst ) ;
			(void)tmp3;
			*( (VgaMemoryPtr) dst ) = tmp2 ;
			dst += BYTES_PER_LINE(pWin);
		}
                SetVideoGraphics( Set_ResetIndex, color & VGA_ALLPLANES ) ;
		SetVideoGraphics( Data_RotateIndex, data_rotate_value ) ;
			/* Un-Set XOR */
	}
	dst = SCREENADDRESS( pWin, ( x0 + lx ), y0 );
	for ( tmp = ly; 
	      tmp-- ; ) {
		tmp3 = *( (VgaMemoryPtr) dst ) ;
		(void)tmp3;
		*( (VgaMemoryPtr) dst ) = tmp2 ;
		dst += BYTES_PER_LINE(pWin) ;
	}
}
/* Disable Set/Reset Register */
SetVideoGraphics( Enb_Set_ResetIndex, 0 ) ;


return ;
}

#else	/* for PC98 EGC */
static void WordfastFill( destination, bytes_per_line, wordwidth, height )
register volatile unsigned char *destination ;
register const unsigned int bytes_per_line ;
register const unsigned int wordwidth ;	/* MUST BE > 0 !! */
register unsigned int height ;		/* MUST BE > 0 !! */
{
int stop_count = wordwidth ;
register int row_jump = bytes_per_line - wordwidth*2 ;
#if !defined(OLDHC) && defined(BSDrt) && !defined(__i386__) && 0
register const int notZero = ~0x0 ;
#else
#define notZero ( ~0 )
#endif

#define SINGLE_STORE \
    ( *( (unsigned short *) destination++ ) = notZero ); \
    destination++; stop_count--;

/* TOP OF FIRST LOOP */
BranchPoint:

switch ( wordwidth & 0xF ) { /* Jump into loop at mod 16 remainder */
	LoopTop :
	case 0x0 : SINGLE_STORE ;
	case 0xF : SINGLE_STORE ;
	case 0xE : SINGLE_STORE ;
	case 0xD : SINGLE_STORE ;
	case 0xC : SINGLE_STORE ;
	case 0xB : SINGLE_STORE ;
	case 0xA : SINGLE_STORE ;
	case 0x9 : SINGLE_STORE ;
	case 0x8 : SINGLE_STORE ;
	case 0x7 : SINGLE_STORE ;
	case 0x6 : SINGLE_STORE ;
	case 0x5 : SINGLE_STORE ;
	case 0x4 : SINGLE_STORE ;
	case 0x3 : SINGLE_STORE ;
	case 0x2 : SINGLE_STORE ;
	case 0x1 : SINGLE_STORE ;
/* FIRST LOOP */
		if ( stop_count )
			goto LoopTop ;
/* SECOND LOOP */
		if ( --height ) {
			destination += row_jump ;
			stop_count = wordwidth ;
			goto BranchPoint ;
		}
		else
			return ;
#undef SINGLE_STORE
}
/*NOTREACHED*/
}

void xf4bppFillSolid( pWin, color, alu, planes, x0, y0, lx, ly )
WindowPtr pWin; /* GJA */
unsigned long int color ;
const int alu ;
unsigned long int planes ;
register int x0 ;
register const int y0 ;
register int lx ;
register const int ly ;		/* MUST BE > 0 !! */
{
register volatile unsigned char *dst ;
register tmp ;
register tmp2 ;
register unsigned short tmp3 ;
unsigned short ROP_value;
unsigned int data_rotate_value = VGA_COPY_MODE ;
unsigned int read_write_modify = FALSE ;
unsigned int invert_existing_data = FALSE ;

{	/* Start GJA */
	if ( !xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->vtSema ) {
		xf4bppOffFillSolid( pWin, color, alu, planes, x0, y0, lx, ly );
		return;
	}
}	/* End GJA */

if ( ( lx == 0 ) || ( ly == 0 ) )
	return;

switch ( alu ) {
	case GXclear:		/* 0x0 Zero 0 */
		color = 0 ;
		break ;
	case GXnor:		/* 0x8 NOT src AND NOT dst */
		invert_existing_data = TRUE ;
	case GXandInverted:	/* 0x4 NOT src AND dst */
		color = ~color ;
	case GXand:		/* 0x1 src AND dst */
		data_rotate_value = VGA_AND_MODE ;
		read_write_modify = TRUE ;
	case GXcopy:		/* 0x3 src */
		break ;
	case GXnoop:		/* 0x5 dst */
		return ;
	case GXequiv:		/* 0x9 NOT src XOR dst */
		color = ~color ;
	case GXxor:		/* 0x6 src XOR dst */
		data_rotate_value = VGA_XOR_MODE ;
		read_write_modify = TRUE ;
		planes &= color ;
		break ;
	case GXandReverse:	/* 0x2 src AND NOT dst */
		invert_existing_data = TRUE ;
		data_rotate_value = VGA_AND_MODE ;
		read_write_modify = TRUE ;
		break ;
	case GXorReverse:	/* 0xb src OR NOT dst */
		invert_existing_data = TRUE ;
		data_rotate_value = VGA_OR_MODE ;
		read_write_modify = TRUE ;
		break ;
	case GXnand:		/* 0xe NOT src OR NOT dst */
		invert_existing_data = TRUE ;
	case GXorInverted:	/* 0xd NOT src OR dst */
		color = ~color ;
	case GXor:		/* 0x7 src OR dst */
		data_rotate_value = VGA_OR_MODE ;
		read_write_modify = TRUE ;
		break ;
	case GXcopyInverted:	/* 0xc NOT src */
		color = ~color ;
		break ;
	case GXinvert:		/* 0xa NOT dst */
		data_rotate_value = VGA_XOR_MODE ;
		read_write_modify = TRUE ;
	case GXset:		/* 0xf 1 */
		color = VGA_ALLPLANES ;
	default:
		break ;
}

if ( !( planes &= VGA_ALLPLANES ) )
	return ;

/* Set Access Planes */
outw(EGC_PLANE, ~planes);
switch(data_rotate_value) {
/* EGC MODE.. Cmp Read: Flase, WriteSource=ROP, ReadSource=CPU */
 case VGA_AND_MODE:
    if (invert_existing_data)
	ROP_value = EGC_AND_INV_MODE;
    else
	ROP_value = EGC_AND_MODE;
    break;
 case VGA_OR_MODE:
    if (invert_existing_data)
	ROP_value = EGC_OR_INV_MODE;
    else
	ROP_value = EGC_OR_MODE;
    break;
 case VGA_XOR_MODE:
    if (invert_existing_data)
	ROP_value = EGC_XOR_INV_MODE;
    else
	ROP_value = EGC_XOR_MODE;
    break;
 case VGA_COPY_MODE:
 default:
    ROP_value = EGC_COPY_MODE;
    break;
}
outw(EGC_MODE, ROP_value);
outw(EGC_FGC, color & VGA_ALLPLANES);
/* Do Left Edge */
if ( tmp = x0 & 0x0f ) {
	dst = (unsigned char *)((int)(SCREENADDRESS(pWin,x0,y0)) & ~0x01);
	tmp3 = (unsigned)0xffff >>tmp;
	/* Catch The Cases Where The Entire Region Is Within One Word */
	if ( ( lx -= 16 - tmp ) < 0 ) {
		tmp3 &= (unsigned)0xffff << -lx;
		lx = 0 ;
	}
	tmp3 = (unsigned short)(tmp3 >> 8 | tmp3 << 8);
	for ( tmp = ly;
	      tmp-- ; ) {
		*((unsigned short *) dst ) = tmp3 ;
		dst += BYTES_PER_LINE(pWin);
	}
	if ( !lx ) { /* All Handled In This Word */
		return ;
	}
	x0 = ( x0 + 0x0f ) & ~0x0f ;
}

/* Fill The Center Of The Box */
if (lx >> 4) {
	WordfastFill( SCREENADDRESS( pWin, x0, y0 ), BYTES_PER_LINE(pWin), 
		     (lx >> 4), ly ) ;
}

/* Do Right Edge */
if ( tmp = lx & 0x0f ) { /* x0 Now Is Word Aligned */
	/* Set The Bit Mask */
	tmp3 = (unsigned)0xffff << ( 16 - tmp );
	dst = (unsigned char*)((int)SCREENADDRESS(pWin,(x0+lx),y0) & ~0x01);
	tmp3 = (unsigned short)(tmp3 >> 8 | tmp3 << 8);
	for ( tmp = ly;
	      tmp-- ; ) {
		*( (unsigned short *) dst ) = tmp3 ;
		dst += BYTES_PER_LINE(pWin);
	}
}

return ;
}
#endif
