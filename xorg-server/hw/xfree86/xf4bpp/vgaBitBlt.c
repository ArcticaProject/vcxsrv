/* GJA -- span move routines */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf4bpp.h"
#include "OScompiler.h"
#include "vgaReg.h"
#include "vgaVideo.h"

#include "xf86str.h" /* for pScrn->vtSema */
extern ScrnInfoPtr *xf86Screens;

#ifndef	PC98_EGC	/* not PC98_EGC */
/* NOTE: It seems that there is no way to program the VGA to copy just
 * a part of a byte in the smarter modes. Therefore we copy the boundaries
 * plane by plane.
 */
#define WORDSZ 8
 /* The fast blit code requires WORDSZ = 8 for its read-modify write cycle.
  * Therefore, we do not fully implement the other options.
  */
#define HIGHPLANEMASK 0x08
#define HIGHPLANEINDEX 3

/* Of course, we want the following anyway:
 * (Yes, they're identical now.)
 */
#define SMEM(x,y) ( VIDBASE(pWin) + (y) * BYTES_PER_LINE(pWin) + (x) )
#define DMEM(x,y) ( VIDBASE(pWin) + (y) * BYTES_PER_LINE(pWin) + (x) )

#define WORD8 unsigned char
#define LW8 BYTES_PER_LINE(pWin) /* Line width */
#define WSHIFT8 0x3
#define WMASK8 0x07
/* NOTE: lmask[8] matters. It must be different from lmask[0] */
static unsigned char lmasktab[] = {
	0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF
} ;
static unsigned char rmasktab[] = {
	0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00
} ;

#define LMASK8(n) lmasktab[n]
#define RMASK8(n) rmasktab[n]
#define SWAPB8(x) (x)

#if (WORDSZ == 8)

#define WORD WORD8
#define LW LW8
#define WSHIFT WSHIFT8
#define WMASK WMASK8

#define LMASK(n) LMASK8(n)
#define RMASK(n) RMASK8(n)
#define SWAPB(x) SWAPB8(x)

#endif /* WORDSZ == 8 */

#define DO_ALU(dst,src,mask,alu) {\
	int _ndst, _odst; _odst = dst; \
	switch ( alu ) { \
	case GXclear: \
		_ndst = 0;			break; \
	case GXand: \
		_ndst = src & _odst;		break; \
	case GXandReverse: \
		_ndst = src & ~ _odst;		break; \
	case GXcopy: \
		_ndst = src;			break; \
	case GXandInverted: \
		_ndst = ~ src & _odst;		break; \
	default: \
	case GXnoop: \
		_ndst = _odst;			break; \
	case GXxor: \
		_ndst = src ^ _odst;		break; \
	case GXor: \
		_ndst = src | _odst;		break; \
	case GXnor: \
		_ndst = ~ src & ~ _odst;	break; \
	case GXequiv: \
		_ndst = ~ src ^ _odst;		break; \
	case GXinvert: \
		_ndst = ~ _odst;		break; \
	case GXorReverse: \
		_ndst = src | ~ _odst;		break; \
	case GXcopyInverted: \
		_ndst = ~ src;			break; \
	case GXorInverted: \
		_ndst = ~ src | _odst;		break; \
	case GXnand: \
		_ndst = ~ src | ~ _odst;	break; \
	case GXset: \
		_ndst = ~0;			break; \
	} \
	dst = (_odst & ~(mask)) | (_ndst & (mask)); \
	}

static void aligned_blit(
    WindowPtr, int, int, int, int, int, int, int, int
);

static void aligned_blit_center(
    WindowPtr, int, int, int, int, int, int
);

static void shift(
    WindowPtr, int, int, int, int, int, int, int
);

static void shift_thin_rect(
    WindowPtr, int, int, int, int, int, int, int
);

static void shift_center(
    WindowPtr, int, int, int, int, int, int, int
);

void xf4bppBitBlt(pWin,alu,writeplanes,x0,y0,x1,y1,w,h)
WindowPtr pWin; /* GJA */
int alu;
int writeplanes; /* planes */
int x0, y0, x1, y1, w, h;
{
    IOADDRESS REGBASE;
    int plane, bit;

    if ( !w || !h ) return;

    if ( ! xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->vtSema ) {
        xf4bppOffBitBlt(pWin,alu,writeplanes,x0,y0,x1,y1,w,h);
        return;
    }

    REGBASE =
	xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->domainIOBase + 0x300;

    /* 0x7, not WMASK: it is hardware dependant */
    if ( ((x0 - x1) & 0x7) || (alu != GXcopy) ) {
	/* Use slow copy */
	SetVideoGraphics(Enb_Set_ResetIndex, 0); /* All from CPU */
	SetVideoGraphics(Bit_MaskIndex, 0xFF); /* All bits */
	SetVideoGraphics(Graphics_ModeIndex, 0); /* Write mode 0 */
	SetVideoGraphics(Data_RotateIndex, 0); /* Don't rotate, replace */

	for ( plane = HIGHPLANEMASK, bit = HIGHPLANEINDEX ;
		plane ; plane >>= 1, bit-- )
	{

		if ( writeplanes & plane) {
			SetVideoGraphics(Read_Map_SelectIndex, bit);
			SetVideoSequencer(Mask_MapIndex, plane);

			shift(pWin,x0,x1,y0,y1,w,h,alu);
		}
	}
    } else {
        aligned_blit(pWin,x0,x1,y0,y1,w,h,alu,writeplanes);
    }
}

/* Copy a span a number of places to the right.
 */
static void
shift(pWin,x0,x1,y0,y1,w,h,alu)
WindowPtr pWin; /* GJA */
int x0;  /* left edge of source */
int x1;  /* left edge of target */
int y0;
int y1;
int w; /* length of source, and of target */
int h;
int alu;
{
  if ( ((x1 & WMASK) + w) <= WORDSZ ) {
     shift_thin_rect(pWin,x0,x1,y0,y1,w,h,alu);
  } else if ( x1 > x0 ) { /* Shift right: start right */
     int l1 = x1 & WMASK, r1 = (x1 + w) & WMASK;

     if ( r1 ) /* right edge */
        shift_thin_rect(pWin,x0+w-r1,x1+w-r1,y0,y1,r1,h,alu);
     shift_center(pWin,x0,x1,y0,y1,w,h,alu);
     if ( l1 ) /* left edge */
        shift_thin_rect(pWin,x0,x1,y0,y1,(WORDSZ-l1),h,alu);
  } else {
     int l1 = x1 & WMASK, r1 = (x1 + w) & WMASK;

     if ( l1 ) /* left edge */
        shift_thin_rect(pWin,x0,x1,y0,y1,(WORDSZ-l1),h,alu);
     shift_center(pWin,x0,x1,y0,y1,w,h,alu);
     if ( r1 ) /* right edge */
        shift_thin_rect(pWin,x0+w-r1,x1+w-r1,y0,y1,r1,h,alu);
  }
}

/* The whole rectangle is so thin that it fits in one byte written */
static void
shift_thin_rect(pWin,x0,x1,y0,y1,w,h,alu)
WindowPtr pWin; /* GJA */
int x0;  /* left edge of source */
int x1;  /* left edge of target */
int y0;
int y1;
int w; /* length of source, and of target */
int h;
int alu;
{
  int l0 = x0 & WMASK; /* Left edge of source, as bit */
  int l1 = x1 & WMASK; /* Left edge of target, as bit */
  int L0 = x0 >> WSHIFT; /* Left edge of source, as byte */
  int L1 = x1 >> WSHIFT; /* Left edge of target, as byte */
  int pad;
  int htmp;
  int mask;
  int tmp;
  int bs;
  
  volatile unsigned char *sp, *dp;

  mask = RMASK(l1) & LMASK(l1+w);
  bs = (x1 - x0) & WMASK;

  if ( y1 > y0 ) { /* Move down, start at the bottom */
    pad = - BYTES_PER_LINE(pWin);
    sp = SMEM(L0,y0+h-1);
    dp = DMEM(L1,y1+h-1);
  } else { /* Move up, start at the top */
    pad = BYTES_PER_LINE(pWin);
    sp = SMEM(L0,y0);
    dp = DMEM(L1,y1);
  }

  if ( l0+w > WORDSZ ) {
    /* Need two bytes */
    for ( htmp = h ; htmp ; htmp-- ) {
      tmp = (sp[0] << (WORDSZ - bs));
      sp++;
      tmp |= (sp[0] >> bs);
      sp--;
      DO_ALU(dp[0],tmp,mask,alu);
      dp += pad;
      sp += pad;
    }
  } else if ( l0 <= l1 ) {
    /* Need one byte, shifted right */
    for ( htmp = h ; htmp ; htmp-- ) {
      tmp = (sp[0] >> bs);
      DO_ALU(dp[0],tmp,mask,alu);
      dp += pad;
      sp += pad;
    }
  } else {
    /* Need one byte, shifted left */
    for ( htmp = h ; htmp ; htmp-- ) {
      tmp = (sp[0] << (WORDSZ - bs));
      DO_ALU(dp[0],tmp,mask,alu);
      dp += pad;
      sp += pad;
    }
  }
}

static void
shift_center(pWin,x0,x1,y0,y1,w,h,alu)
WindowPtr pWin; /* GJA */
int x0;  /* left edge of source */
int x1;  /* left edge of target */
int y0;
int y1;
int w; /* length of source, and of target */
int h;
int alu;
{
  int l1 = x1 & WMASK; /* Left edge of target, as bit */
  int r1 = (x1 + w) & WMASK; /* Right edge of target, as bit */
  int pad;
  int htmp, wtmp; /* Temporaries for indices over height and width */
  volatile unsigned char tmp; /* Temporary result of the shifts */
  int bs;
  int rem; /* Remaining bits; temporary in loop */
  int bytecnt;
  
  volatile unsigned char *sp, *dp;

  bs = (x1 - x0) & WMASK;

  if ( l1 ) {
     bytecnt = (w - (WORDSZ - l1) - r1) >> WSHIFT;
     sp = SMEM( ((x0 + (WORDSZ - l1)) >> WSHIFT), y0);
     dp = DMEM( ((x1 + (WORDSZ - l1)) >> WSHIFT), y1);
  } else {
     bytecnt = (w - r1) >> WSHIFT;
     sp = SMEM( (x0 >> WSHIFT), y0);
     dp = DMEM( (x1 >> WSHIFT), y1);
  }

  if ( y1 > y0 ) { /* Move down, start at the bottom */
    if ( x1 > x0 ) { /* Move right, start right */
       pad = - BYTES_PER_LINE(pWin) + bytecnt;
       sp += BYTES_PER_LINE(pWin) * (h - 1) + bytecnt - 1;
       dp += BYTES_PER_LINE(pWin) * (h - 1) + bytecnt - 1;
    } else { /* Move left, start left */
       pad = - BYTES_PER_LINE(pWin) - bytecnt;
       sp += BYTES_PER_LINE(pWin) * (h - 1);
       dp += BYTES_PER_LINE(pWin) * (h - 1);
    }
  } else { /* Move up, start at the top */
    if ( x1 > x0 ) { /* Move right, start right */
       pad = BYTES_PER_LINE(pWin) + bytecnt;
       sp += bytecnt - 1;
       dp += bytecnt - 1;
    } else { /* Move left, start left */
       pad = BYTES_PER_LINE(pWin) - bytecnt;
       sp += 0;
       dp += 0;
    }
  }

  if ( x1 > x0 ) { /* Move right, start right */
    if ( bs == 0 ) { /* No shift. Need one byte only */
      for ( htmp = h ; htmp ; htmp-- ) {
        for ( wtmp = bytecnt ; wtmp ; wtmp-- ) {
          tmp = sp[0];
          DO_ALU(dp[0],tmp,~0,alu); 
	  dp--;
          sp--;
        }
        dp += pad;
        sp += pad;
      } 
    } else {
      for ( htmp = h ; htmp ; htmp-- ) {
	if ( bytecnt ) {
	   sp++;
   	   rem = sp[0];
	   sp--;
           for ( wtmp = bytecnt ; wtmp ; wtmp-- ) {
             tmp = (rem >> bs);
             rem = sp[0];
             tmp |= (rem << (WORDSZ - bs)) ;
             DO_ALU(dp[0],tmp,~0,alu); 
	     dp--;
             sp--;
           }
        }
        dp += pad;
        sp += pad;
      } 
    }
  } else { /* x1 <= x0 */ /* Move left, start left */
    if ( bs == 0 ) { /* No shift. Need one byte only */
      for ( htmp = h ; htmp ; htmp-- ) {
        for ( wtmp = bytecnt ; wtmp ; wtmp-- ) {
          tmp = sp[0];
          DO_ALU(dp[0],tmp,~0,alu); 
	  dp++;
          sp++;
        }
        dp += pad;
        sp += pad;
      } 
    } else {
      for ( htmp = h ; htmp ; htmp-- ) {
        if ( bytecnt ) {
          rem = sp[0];
          for ( wtmp = bytecnt ; wtmp ; wtmp-- ) {
            tmp = (rem << (WORDSZ - bs));
	    sp++;
	    rem = sp[0];
	    sp--;
            tmp |= (rem >> bs);
            DO_ALU(dp[0],tmp,~0,alu); 
	    dp++;
            sp++;
          }
        }
        dp += pad;
        sp += pad;
      } 
    }
  }
}

/* Copy a rectangle.
 */
static void
aligned_blit(pWin,x0,x1,y0,y1,w,h,alu,planes)
WindowPtr pWin; /* GJA */
int x0;  /* left edge of source */
int x1;  /* left edge of target */
int y0;
int y1;
int w; /* length of source, and of target */
int h;
int alu;
int planes;
{
  IOADDRESS REGBASE =
	xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->domainIOBase + 0x300;
  int plane, bit;

  if ( ((x1 & WMASK) + w) <= WORDSZ ) {
	SetVideoGraphics(Enb_Set_ResetIndex, 0); /* All from CPU */
	SetVideoGraphics(Bit_MaskIndex, 0xFF); /* All bits */
	SetVideoGraphics(Graphics_ModeIndex, 0); /* Write mode 0 */
	SetVideoGraphics(Data_RotateIndex, 0); /* Don't rotate, replace */

	for ( plane = HIGHPLANEMASK, bit = HIGHPLANEINDEX;
		plane ; plane >>= 1, bit-- )
	{
		if ( planes & plane) {
			SetVideoGraphics(Read_Map_SelectIndex, bit);
			SetVideoSequencer(Mask_MapIndex, plane);

     			shift_thin_rect(pWin,x0,x1,y0,y1,w,h,alu);
		}
	}
  } else if ( x1 > x0 ) { /* Shift right: start right */
     int l1 = x1 & WMASK, r1 = (x1 + w) & WMASK;

     if ( r1 ) { /* right edge */
	SetVideoGraphics(Enb_Set_ResetIndex, 0); /* All from CPU */
	SetVideoGraphics(Bit_MaskIndex, 0xFF); /* All bits */
	SetVideoGraphics(Graphics_ModeIndex, 0); /* Write mode 0 */
	SetVideoGraphics(Data_RotateIndex, 0); /* Don't rotate, replace */

	for ( plane = HIGHPLANEMASK, bit = HIGHPLANEINDEX;
		plane ; plane >>= 1, bit-- )
	{
		if ( planes & plane) {
			SetVideoGraphics(Read_Map_SelectIndex, bit);
			SetVideoSequencer(Mask_MapIndex, plane);

		        shift_thin_rect(pWin,x0+w-r1,x1+w-r1,y0,y1,r1,h,alu);
		}
	}
     }

     /* Center */
     SetVideoGraphics(Graphics_ModeIndex, 1); /* Write mode 1 */
     SetVideoSequencer(Mask_MapIndex, planes);

     aligned_blit_center(pWin,x0,x1,y0,y1,w,h);

     if ( l1 ) { /* left edge */
	SetVideoGraphics(Enb_Set_ResetIndex, 0); /* All from CPU */
	SetVideoGraphics(Bit_MaskIndex, 0xFF); /* All bits */
	SetVideoGraphics(Graphics_ModeIndex, 0); /* Write mode 0 */
	SetVideoGraphics(Data_RotateIndex, 0); /* Don't rotate, replace */

	for ( plane = HIGHPLANEMASK, bit = HIGHPLANEINDEX;
		plane ; plane >>= 1, bit-- )
	{
		if ( planes & plane) {
			SetVideoGraphics(Read_Map_SelectIndex, bit);
			SetVideoSequencer(Mask_MapIndex, plane);

        		shift_thin_rect(pWin,x0,x1,y0,y1,(WORDSZ-l1),h,alu);
		}
	}
     }
  } else {
     int l1 = x1 & WMASK, r1 = (x1 + w) & WMASK;

     if ( l1 ) { /* left edge */
	SetVideoGraphics(Enb_Set_ResetIndex, 0); /* All from CPU */
	SetVideoGraphics(Bit_MaskIndex, 0xFF); /* All bits */
	SetVideoGraphics(Graphics_ModeIndex, 0); /* Write mode 0 */
	SetVideoGraphics(Data_RotateIndex, 0); /* Don't rotate, replace */

	for ( plane = HIGHPLANEMASK, bit = HIGHPLANEINDEX;
		plane ; plane >>= 1, bit-- )
	{
		if ( planes & plane) {
			SetVideoGraphics(Read_Map_SelectIndex, bit);
			SetVideoSequencer(Mask_MapIndex, plane);

        		shift_thin_rect(pWin,x0,x1,y0,y1,(WORDSZ-l1),h,alu);
		}
	}
     }

     /* Center */
     SetVideoGraphics(Graphics_ModeIndex, 1); /* Write mode 1 */
     SetVideoSequencer(Mask_MapIndex, planes);

     aligned_blit_center(pWin,x0,x1,y0,y1,w,h);

     if ( r1 ) { /* right edge */
	SetVideoGraphics(Enb_Set_ResetIndex, 0); /* All from CPU */
	SetVideoGraphics(Bit_MaskIndex, 0xFF); /* All bits */
	SetVideoGraphics(Graphics_ModeIndex, 0); /* Write mode 0 */
	SetVideoGraphics(Data_RotateIndex, 0); /* Don't rotate, replace */

	for ( plane = HIGHPLANEMASK, bit = HIGHPLANEINDEX ;
		plane ; plane >>= 1, bit-- )
	{
		if ( planes & plane) {
			SetVideoGraphics(Read_Map_SelectIndex, bit);
			SetVideoSequencer(Mask_MapIndex, plane);

        		shift_thin_rect(pWin,x0+w-r1,x1+w-r1,y0,y1,r1,h,alu);
		}
	}
     }
  }
}

static void
aligned_blit_center(pWin,x0,x1,y0,y1,w,h)
WindowPtr pWin; /* GJA */
int x0;  /* left edge of source */
int x1;  /* left edge of target */
int y0;
int y1;
int w; /* length of source, and of target */
int h;
{
  int l1 = x1 & WMASK; /* Left edge of target, as bit */
  int r1 = (x1 + w) & WMASK; /* Right edge of target, as bit */
  int pad;
  int htmp, wtmp; /* Temporaries for indices over height and width */
  volatile unsigned char tmp; /* Temporary result of the shifts */
  int bytecnt;
  
  volatile unsigned char *sp, *dp;

  if ( l1 ) {
     bytecnt = (w - (WORDSZ - l1) - r1) >> WSHIFT;
     sp = SMEM( ((x0 + (WORDSZ - l1)) >> WSHIFT), y0);
     dp = DMEM( ((x1 + (WORDSZ - l1)) >> WSHIFT), y1);
  } else {
     bytecnt = (w - r1) >> WSHIFT;
     sp = SMEM( (x0 >> WSHIFT), y0);
     dp = DMEM( (x1 >> WSHIFT), y1);
  }

  if ( y1 > y0 ) { /* Move down, start at the bottom */
    if ( x1 > x0 ) { /* Move right, start right */
       pad = - BYTES_PER_LINE(pWin) + bytecnt;
       sp += BYTES_PER_LINE(pWin) * (h - 1) + bytecnt - 1;
       dp += BYTES_PER_LINE(pWin) * (h - 1) + bytecnt - 1;
    } else { /* Move left, start left */
       pad = - BYTES_PER_LINE(pWin) - bytecnt;
       sp += BYTES_PER_LINE(pWin) * (h - 1);
       dp += BYTES_PER_LINE(pWin) * (h - 1);
    }
  } else { /* Move up, start at the top */
    if ( x1 > x0 ) { /* Move right, start right */
       pad = BYTES_PER_LINE(pWin) + bytecnt;
       sp += bytecnt - 1;
       dp += bytecnt - 1;
    } else { /* Move left, start left */
       pad = BYTES_PER_LINE(pWin) - bytecnt;
       sp += 0;
       dp += 0;
    }
  }

  if ( x1 > x0 ) { /* Move right, start right */
      for ( htmp = h ; htmp ; htmp-- ) {
        for ( wtmp = bytecnt ; wtmp ; wtmp-- ) {
          tmp = sp[0];
	  dp[0] = tmp;
	  dp--;
          sp--;
        }
        dp += pad;
        sp += pad;
      } 
  } else { /* x1 <= x0 */ /* Move left, start left */
      for ( htmp = h ; htmp ; htmp-- ) {
        for ( wtmp = bytecnt ; wtmp ; wtmp-- ) {
          tmp = sp[0];
          dp[0] = tmp;
	  dp++;
          sp++;
        }
        dp += pad;
        sp += pad;
      } 
  }
}
#else	/* PC98_EGC */

static void
egc_fast_blt (pWin, alu, writeplanes, x0, y0, x1, y1, w, h)
WindowPtr pWin;	
const	int alu, writeplanes ;
register int x0, x1 ;
int	     y0, y1 ;
register int w, h ;
{
register volatile unsigned char *src ;
register volatile unsigned char *dst ;
unsigned short *src_x ;
unsigned short *dst_x ;
int x_direction, y_interval ;
int	src_off, dst_off ;
register int k, i ;
unsigned short ROP_value;

src = (unsigned char *)SCREENADDRESS( pWin, 0, y0);
dst = (unsigned char *)SCREENADDRESS( pWin, 0, y1);

/* Set Map Mask */
outw(EGC_PLANE, ~(writeplanes & VGA_ALLPLANES));
switch(alu) {
case GXnor:		/* ~(S|D) */
    ROP_value = 0x2903;
    break;
case GXandInverted:	/* ~S&D */
    ROP_value = 0x290c;
    break;
case GXand:		/* S&D */
    ROP_value = 0x29c0;
    break;
case GXequiv:		/* ~S ^ D */
    ROP_value = 0x29c3;
    break;
case GXxor:		/* S^D */
    ROP_value = 0x293c;
    break;
case GXandReverse:	/* S&~D */
    ROP_value = 0x2930;
    break;
case GXorReverse:	/* S|~D */
    ROP_value = 0x29f3;
    break;
case GXnand:		/* ~(S&D) */
    ROP_value = 0x293f;
    break;
case GXorInverted:	/* ~S|D */
    ROP_value = 0x29cf;
    break;
case GXor:		/* S|D */
    ROP_value = 0x29fa;
    break;
case GXcopyInverted:	/* ~S */
    ROP_value = 0x290f;
    break;
case GXcopy:		/* S */
default:
    ROP_value = 0x29f0;
}
outw(EGC_MODE, ROP_value);
if ( y1 > y0 ) {
	y_interval = - BYTES_PER_LINE(pWin) * 8 ;
	src += BYTES_PER_LINE(pWin) * ( h - 1 ) ;
	dst += BYTES_PER_LINE(pWin) * ( h - 1 ) ;
}
else {
	y_interval = BYTES_PER_LINE(pWin) * 8 ;
}

src = (unsigned char *)((int)src << 3) ;
dst = (unsigned char *)((int)dst << 3) ;

if ( y1 > y0) {
	x_direction = 0x1000 ;
	src += x0 + w - 1 ;
	dst += x1 + w - 1 ;
} else if ( y1 < y0 ) {
	x_direction = 0 ;
	src += x0 ;
	dst += x1 ;
} else {
	if ( x1 < x0 ) {
		x_direction = 0 ;
		src += x0 ;
		dst += x1 ;
	} else {
		x_direction = 0x1000 ;
		src += x0 + w - 1 ;
		dst += x1 + w - 1 ;
	}
}
	outw ( EGC_LENGTH , w - 1 ) ;

for ( ; h-- ; ) {
	if ( x_direction ) {
		src_off = 15 - (int)src & 0xf ;
		dst_off = 15 - (int)dst & 0xf ;
	} else {
		src_off = (int)src & 0xf ;
		dst_off = (int)dst & 0xf ;
	}
#if defined(__NetBSD__) || defined(__OpenBSD__)
	src_x   = (unsigned short *)(((unsigned int)src >> 4 ) << 1) ;
	dst_x   = (unsigned short *)(((unsigned int)dst >> 4 ) << 1) ;
#else
	src_x   = (unsigned short *)(((int)src >> 4 ) << 1) ;
	dst_x   = (unsigned short *)(((int)dst >> 4 ) << 1) ;
#endif
	k = ( src_off + w + 15 ) >> 4 ;
	if ( src_off < dst_off ) {
		if ( ((src_off + w - 1 ) >> 4) < ((dst_off + w - 1) >> 4)) k++ ;
	}
	if ( src_off > dst_off ) {
		if ( ((src_off + w - 1) >> 4 ) == ((dst_off + w - 1) >> 4) ) k++ ;
		if ( x_direction ) dst_x ++ ;
			else	   dst_x -- ;
	}
	outw ( EGC_ADD , x_direction | src_off | dst_off << 4 );
	if ( x_direction ) {
		wcopyl ( src_x, dst_x, k, VIDBASE(pWin) ) ;
	} else {
		wcopyr ( src_x, dst_x, k, VIDBASE(pWin) ) ;
	}
src += y_interval ;
dst += y_interval ;
}
outw ( EGC_ADD, 0 ) ;
outw ( EGC_LENGTH , 0xf );
return;
}

void
xf4bppBitBlt( pWin,alu, writeplanes, x0, y0, x1, y1, w, h )
WindowPtr pWin; /* GJA */
int alu;
int writeplanes; /* planes */
int x0, y0, x1, y1, w, h;
{
	if ( ! xf86Screens[((DrawablePtr)pWin)->pScreen->myNum]->vtSema ) {
		xf4bppOffBitBlt( pWin, alu, writeplanes,
			   x0, y0, x1, y1, w, h );
		return;
	}

switch ( alu ) {
	case GXclear:		/* 0x0 Zero 0 */
	case GXinvert:		/* 0xa NOT dst */
	case GXset:		/* 0xf 1 */
		xf4bppFillSolid( pWin, VGA_ALLPLANES, alu, writeplanes, x1, y1, w, h ) ;
			/* x1, y1, GJA */
	case GXnoop:		/* 0x5 dst */
		return ;
	default:
		break ;
}

egc_fast_blt ( pWin, alu, writeplanes, x0, y0, x1, y1, w, h);
return;
}
#endif
