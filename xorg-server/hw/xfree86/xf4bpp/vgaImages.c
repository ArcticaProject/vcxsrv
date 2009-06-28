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

void
xf4bppDrawColorImage( pWin, x, y, w, h, data, RowIncrement, alu, planes )
WindowPtr pWin; /* GJA */
int x, y ;
register int w, h ;
unsigned char *data ;
register int RowIncrement ;
const int alu ;
const unsigned long int planes ;
{
IOADDRESS REGBASE;
register unsigned long int tmp ;
register const unsigned char *src ;
register volatile unsigned char *dst ;
register int Pixel_Count ;
register unsigned int currMask ;
register unsigned int InitialMask ;
register volatile unsigned char *StartByte ;
unsigned int invert_source_data = FALSE ;
#ifdef	PC98_EGC	/* new EGC test */
register unsigned char tmp1;
#endif

{	/* Start GJA */
	if ( !xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->vtSema ) {
		xf4bppOffDrawColorImage( pWin, x, y, w, h, data, RowIncrement, alu, planes );
		return;
	}
}	/* End GJA */

{
	unsigned int invert_existing_data = FALSE ;
	unsigned int data_rotate_value = VGA_COPY_MODE ;
#ifdef	PC98_EGC
	unsigned short ROP_value;
#endif

	REGBASE = 0x300 +
	    xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->domainIOBase;

	switch ( alu ) {
		case GXclear:		/* 0x0 Zero 0 */
		case GXinvert:		/* 0xa NOT dst */
		case GXset:		/* 0xf 1 */
			xf4bppFillSolid( pWin, VGA_ALLPLANES, alu, planes, x, y, w, h ) ;
		case GXnoop:		/* 0x5 dst */
			return ;
		case GXnor:		/* 0x8 NOT src AND NOT dst */
			invert_existing_data = TRUE ;
		case GXandInverted:	/* 0x4 NOT src AND dst */
			invert_source_data = TRUE ;
		case GXand:		/* 0x1 src AND dst */
			data_rotate_value = VGA_AND_MODE ;
			break ;
		case GXequiv:		/* 0x9 NOT src XOR dst */
			invert_source_data = TRUE ;
		case GXxor:		/* 0x6 src XOR dst */
			data_rotate_value = VGA_XOR_MODE ;
			break ;
		case GXandReverse:	/* 0x2 src AND NOT dst */
			invert_existing_data = TRUE ;
			data_rotate_value = VGA_AND_MODE ;
			break ;
		case GXnand:		/* 0xe NOT src OR NOT dst */
			invert_source_data = TRUE ;
		case GXorReverse:	/* 0xb src OR NOT dst */
			invert_existing_data = TRUE ;
			/* GJA -- moved this here */
			data_rotate_value = VGA_OR_MODE ;
			break ;
		case GXorInverted:	/* 0xd NOT src OR dst */
			invert_source_data = TRUE ;
		case GXor:		/* 0x7 src OR dst */
			data_rotate_value = VGA_OR_MODE ;
			break ;
		case GXcopyInverted:	/* 0xc NOT src */
			invert_source_data = TRUE ;
		case GXcopy:		/* 0x3 src */
		default:
			break ;
	}

#ifdef	PC98_EGC
	/* Setup EGC Registers */
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
	outw(EGC_PLANE, ~(planes & VGA_ALLPLANES));
	outw(EGC_MODE, ROP_value);
	outw(EGC_FGC, 0x0000);
	tmp1 = 0;
#else
	if ( invert_existing_data )
		xf4bppFillSolid( pWin, VGA_ALLPLANES, GXinvert, planes, x, y, w, h ) ;
	/* Setup VGA Registers */
	SetVideoSequencer( Mask_MapIndex, planes & VGA_ALLPLANES ) ;
	/* Set Raster Op */
	SetVideoGraphics( Data_RotateIndex, data_rotate_value ) ;
	SetVideoGraphics( Graphics_ModeIndex, VGA_WRITE_MODE_2 ) ;
#endif
}

StartByte = SCREENADDRESS(pWin, x, y);
InitialMask = SCRRIGHT8( LeftmostBit, BIT_OFFSET( x ) ) ;
if ( invert_source_data )
#ifdef	PC98_EGC
#if 0 /* New EGC version */
	egc_image_invert ( StartByte, data, InitialMask, w, h,
					 RowIncrement ) ;
#else	/* new EGC c version */
	for ( ;
	      h-- ;
	      data += RowIncrement, StartByte += BYTES_PER_LINE(pWin) ) {
		dst = StartByte;
		for ( src = data,
		      Pixel_Count = w, currMask = InitialMask ;
		      Pixel_Count-- ;
		      src++ ) {
			if (tmp1 != (~*src & VGA_ALLPLANES)) {
				tmp1 = ~*src & VGA_ALLPLANES;
				/* set FGC */
				outw(EGC_FGC, ~*src & VGA_ALLPLANES);
			}
			*((VgaMemoryPtr) dst) = currMask;
			if ( currMask & RightmostBit ) {
				currMask = LeftmostBit ;
				dst++;
			}
 			else
				currMask = SCRRIGHT8( currMask, 1 ) ;
		}
	}
#endif	/* new EGC  */
#else	/* original */
	for ( ;
	      h-- ;
	      data += RowIncrement, StartByte += BYTES_PER_LINE(pWin) ) {
		dst = StartByte;
		for ( src = data,
		      Pixel_Count = w, currMask = InitialMask ;
		      Pixel_Count-- ;
		      src++ ) {
			/* Set The Bit Mask Reg */
			SetVideoGraphics( Bit_MaskIndex, currMask ) ;
			/* Read To Load vga Data Latches */
			tmp = *( (VgaMemoryPtr) dst ) ;
			(void) tmp;
			*( (VgaMemoryPtr) dst ) = ~ *src ;
			if ( currMask & RightmostBit ) {
				currMask = LeftmostBit ;
				dst++;
			}
			else
				currMask = SCRRIGHT8( currMask, 1 ) ;
		}
	}
#endif
else /* invert_source_data == FALSE */
#ifdef	PC98_EGC
#if 0	/* new EGC version */
		egc_image ( StartByte, data, InitialMask, w, h,
				RowIncrement );
#else	/* new EGC c version */
	for ( ;
	      h-- ;
	      data += RowIncrement, StartByte += BYTES_PER_LINE(pWin) ) {
		dst = StartByte;
		for ( src = data,
		      Pixel_Count = w, currMask = InitialMask ;
		      Pixel_Count-- ;
		      src++ ) {
			if (tmp1 != *src & VGA_ALLPLANES) {
				tmp1 = *src & VGA_ALLPLANES;
				outw(EGC_FGC, tmp1);	/* set FGC */
			}
			*((VgaMemoryPtr) dst) = currMask;	/* write with mask */
			if ( currMask & RightmostBit ) {
				currMask = LeftmostBit ;
				dst++;
			}
			else
				currMask = SCRRIGHT8( currMask, 1 ) ;
		}
	}
#endif	/* new EGC version */
#else	/* original */
	for ( ;
	      h-- ;
	      data += RowIncrement, StartByte += BYTES_PER_LINE(pWin) ) {
		dst = StartByte;
		for ( src = data,
		      Pixel_Count = w, currMask = InitialMask ;
		      Pixel_Count-- ;
		      src++ ) {
			/* Set The Bit Mask Reg */
			SetVideoGraphics( Bit_MaskIndex, currMask ) ; /* GJA */
			/* Read To Load vga Data Latches */
			tmp = *( (VgaMemoryPtr) dst ) ;
			(void) tmp;
			*( (VgaMemoryPtr) dst ) = *src ;
			if ( currMask & RightmostBit ) {
				currMask = LeftmostBit ;
				dst++;
			}
			else
				currMask = SCRRIGHT8( currMask, 1 ) ;
		}
	}
#endif	/* original */

return ;
}

#ifndef	PC98_EGC
static unsigned long int
read8Z
(
	IOADDRESS REGBASE,
	register volatile unsigned char *screen_ptr
)
{
register unsigned long int i ;
register unsigned long int j ;

/* Read One Byte At A Time to get
 *	i ==	[ Plane 3 ] [ Plane 2 ] [ Plane 1 ] [ Plane 0 ]
 * into a single register
 */
SetVideoGraphicsData( 3 ) ;
i = *( (VgaMemoryPtr) screen_ptr ) << 8 ;
SetVideoGraphicsData( 2 ) ;
i |= *( (VgaMemoryPtr) screen_ptr ) ;
i <<= 8 ;
SetVideoGraphicsData( 1 ) ;
i |= *( (VgaMemoryPtr) screen_ptr ) ;
i <<= 8 ;
SetVideoGraphicsData( 0 ) ;
i |= *( (VgaMemoryPtr) screen_ptr ) ;

/* Push Bits To Get
 * j ==	[Pixel 7][Pixel 6][Pixel 5][Pixel 4][Pixel 3][Pixel 2][Pixel 1][Pixel 0]
 * into one register
 */

j = ( i & 0x1 ) << 4 ;
j |= ( i >>= 1 ) & 0x1 ;
j <<= 4 ;
j |= ( i >>= 1 ) & 0x1 ;
j <<= 4 ;
j |= ( i >>= 1 ) & 0x1 ;
j <<= 4 ;
j |= ( i >>= 1 ) & 0x1 ;
j <<= 4 ;
j |= ( i >>= 1 ) & 0x1 ;
j <<= 4 ;
j |= ( i >>= 1 ) & 0x1 ;
j <<= 4 ;
j |= ( i >>= 1 ) & 0x1 ;

j |= ( i & 0x2 ) << 28 ;
j |= ( ( i >>= 1 ) & 0x2 ) << 24 ;
j |= ( ( i >>= 1 ) & 0x2 ) << 20 ;
j |= ( ( i >>= 1 ) & 0x2 ) << 16 ;
j |= ( ( i >>= 1 ) & 0x2 ) << 12 ;
j |= ( ( i >>= 1 ) & 0x2 ) <<  8 ;
j |= ( ( i >>= 1 ) & 0x2 ) <<  4 ;
j |= ( i >>= 1 ) & 0x2 ;

j |= ( i & 0x4 ) << 28 ;
j |= ( ( i >>= 1 ) & 0x4 ) << 24 ;
j |= ( ( i >>= 1 ) & 0x4 ) << 20 ;
j |= ( ( i >>= 1 ) & 0x4 ) << 16 ;
j |= ( ( i >>= 1 ) & 0x4 ) << 12 ;
j |= ( ( i >>= 1 ) & 0x4 ) <<  8 ;
j |= ( ( i >>= 1 ) & 0x4 ) <<  4 ;
j |= ( i >>= 1 ) & 0x4 ;

j |= ( i & 0x8 ) << 28 ;
j |= ( ( i >>= 1 ) & 0x8 ) << 24 ;
j |= ( ( i >>= 1 ) & 0x8 ) << 20 ;
j |= ( ( i >>= 1 ) & 0x8 ) << 16 ;
j |= ( ( i >>= 1 ) & 0x8 ) << 12 ;
j |= ( ( i >>= 1 ) & 0x8 ) <<  8 ;
j |= ( ( i >>= 1 ) & 0x8 ) <<  4 ;
j |= ( i >>= 1 ) & 0x8 ;

return j ;
}
#endif		/* not PC98_EGC */

void
xf4bppReadColorImage( pWin, x, y, lx, ly, data, RowIncrement )
WindowPtr pWin; /* GJA */
int x, y ;
int lx, ly ;
register unsigned char *data ;
int RowIncrement ;
{
IOADDRESS REGBASE;
register unsigned long int tmp ;
register volatile unsigned char *src ;
volatile unsigned char *masterSrc ;
int savCenterWidth ;
int dx ;
int skip ;
int center_width ;
int ignore ;
int pad ;
unsigned char tmpc;

{	/* Start GJA */
	if ( !xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->vtSema ) {
		xf4bppOffReadColorImage( pWin, x, y, lx, ly, data, RowIncrement );
		return;
	}
}	/* End GJA */

if ( ( lx <= 0 ) || ( ly <= 0 ) )
	return ;

	REGBASE = 0x300 +
	    xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->domainIOBase;

/* Setup VGA Registers */
#ifndef	PC98_EGC
SetVideoGraphicsIndex( Graphics_ModeIndex ) ;
tmpc = inb( GraphicsDataRegister );
SetVideoGraphicsData( tmpc & ~0x8 ) ; /* Clear the bit */
SetVideoGraphicsIndex( Read_Map_SelectIndex ) ;
#else
outw(EGC_MODE, 0x0800);
#endif

skip = BIT_OFFSET( x ) ;
pad = RowIncrement - lx ;
ignore = BIT_OFFSET( x + lx ) ;
masterSrc = SCREENADDRESS( pWin, x, y ) ;
center_width = ROW_OFFSET( x + lx ) - ROW_OFFSET( ( x + 0x7 ) & ~0x7 ) ;

#define SINGLE_STEP 	*data++ = tmp & VGA_ALLPLANES ; tmp >>= 4


if ( center_width < 0 ) {
	src = masterSrc;
	for ( ; ly-- ; ) {
		tmp = read8Z( REGBASE, src ) >> ( skip << 2 ) ;
		for ( dx = lx + 1 ; --dx ; ) {
			SINGLE_STEP ;
		}
		data += pad ;
		src += BYTES_PER_LINE(pWin);
	}
} else
	for ( savCenterWidth = center_width ;
	      ly-- ;
	      center_width = savCenterWidth,
	      masterSrc += BYTES_PER_LINE(pWin) ) {
		src = masterSrc ;
		tmp = read8Z( REGBASE, src ) ; src++;
		if ((dx = skip))
			tmp >>= ( dx << 2 ) ;
		else
		if ( lx < 8 ) {			/* kludge -- GJA */
			--center_width ;	/* kludge -- GJA */
			dx = 8 - lx ;		/* kludge -- GJA */
		} else				/* kludge -- GJA */
			--center_width ;
	BranchPoint:
		switch ( dx ) {
		LoopTop:
		case 0x0: SINGLE_STEP ;
		case 0x1: SINGLE_STEP ;
		case 0x2: SINGLE_STEP ;
		case 0x3: SINGLE_STEP ;
		case 0x4: SINGLE_STEP ;
		case 0x5: SINGLE_STEP ;
		case 0x6: SINGLE_STEP ;
		case 0x7: *data++ = tmp & VGA_ALLPLANES ;

			/* Fall Through To End Of Inner Loop */
			if ( center_width > 0 ) {
				tmp = read8Z( REGBASE, src ) ; src++;
				center_width-- ;
				goto LoopTop ;
			}
			else if ( ( center_width == 0 )
			       && ( dx = ( - ignore ) & 07 ) ) {
				tmp = read8Z( REGBASE, src ) ; src++;
				center_width-- ;
				goto BranchPoint ; /* Do Mod 8 edge */
			}
			else /* End of this line */
				data += pad ;
		}
	}

return ;
}
