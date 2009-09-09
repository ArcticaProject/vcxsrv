/*
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifndef _FBPICT_H_
#define _FBPICT_H_

#include "renderedge.h"


#if defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

#define FbIntMult(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define FbIntDiv(a,b)	 (((CARD16) (a) * 255) / (b))

#define FbGet8(v,i)   ((CARD16) (CARD8) ((v) >> i))

/*
 * There are two ways of handling alpha -- either as a single unified value or
 * a separate value for each component, hence each macro must have two
 * versions.  The unified alpha version has a 'U' at the end of the name,
 * the component version has a 'C'.  Similarly, functions which deal with
 * this difference will have two versions using the same convention.
 */

#define FbOverU(x,y,i,a,t) ((t) = FbIntMult(FbGet8(y,i),(a),(t)) + FbGet8(x,i),\
			   (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))

#define FbOverC(x,y,i,a,t) ((t) = FbIntMult(FbGet8(y,i),FbGet8(a,i),(t)) + FbGet8(x,i),\
			    (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))

#define FbInU(x,i,a,t) ((CARD32) FbIntMult(FbGet8(x,i),(a),(t)) << (i))

#define FbInC(x,i,a,t) ((CARD32) FbIntMult(FbGet8(x,i),FbGet8(a,i),(t)) << (i))

#define FbGen(x,y,i,ax,ay,t,u,v) ((t) = (FbIntMult(FbGet8(y,i),ay,(u)) + \
					 FbIntMult(FbGet8(x,i),ax,(v))),\
				  (CARD32) ((CARD8) ((t) | \
						     (0 - ((t) >> 8)))) << (i))

#define FbAdd(x,y,i,t)	((t) = FbGet8(x,i) + FbGet8(y,i), \
			 (CARD32) ((CARD8) ((t) | (0 - ((t) >> 8)))) << (i))


#define Alpha(x) ((x) >> 24)
#define Red(x) (((x) >> 16) & 0xff)
#define Green(x) (((x) >> 8) & 0xff)
#define Blue(x) ((x) & 0xff)

/**
 * Returns TRUE if the fbComposeGetSolid can be used to get a single solid
 * color representing every source sampling location of the picture.
 */
static INLINE Bool
fbCanGetSolid(PicturePtr pict)
{
    if (pict->pDrawable == NULL ||
	pict->pDrawable->width != 1 ||
	pict->pDrawable->height != 1)
    {
	return FALSE;
    }
    if (pict->repeat != RepeatNormal)
	return FALSE;

    switch (pict->format) {
    case PICT_a8r8g8b8:
    case PICT_x8r8g8b8:
    case PICT_a8b8g8r8:
    case PICT_x8b8g8r8:
    case PICT_b8g8r8a8:
    case PICT_b8g8r8x8:
    case PICT_r8g8b8:
    case PICT_b8g8r8:
    case PICT_r5g6b5:
    case PICT_b5g6r5:
	return TRUE;
    default:
	return FALSE;
    }
}

#define fbComposeGetSolid(pict, bits, fmt) { \
    FbBits	*__bits__; \
    FbStride	__stride__; \
    int		__bpp__; \
    int		__xoff__,__yoff__; \
\
    fbGetDrawable((pict)->pDrawable,__bits__,__stride__,__bpp__,__xoff__,__yoff__); \
    switch (__bpp__) { \
    case 32: \
	(bits) = READ((CARD32 *) __bits__); \
	break; \
    case 24: \
	(bits) = Fetch24 ((CARD8 *) __bits__); \
	break; \
    case 16: \
	(bits) = READ((CARD16 *) __bits__); \
	(bits) = cvt0565to0888(bits); \
	break; \
    case 8: \
	(bits) = READ((CARD8 *) __bits__); \
	(bits) = (bits) << 24; \
	break; \
    case 1: \
	(bits) = READ((CARD32 *) __bits__);			\
	(bits) = FbLeftStipBits((bits),1) ? 0xff000000 : 0x00000000;\
	break; \
    default: \
	return; \
    } \
    /* If necessary, convert RGB <--> BGR. */ \
    if (PICT_FORMAT_TYPE((pict)->format) != PICT_FORMAT_TYPE(fmt)) \
    { \
	(bits) = (((bits) & 0xff000000) | \
		  (((bits) & 0x00ff0000) >> 16) | \
		  (((bits) & 0x0000ff00) >>  0) | \
		  (((bits) & 0x000000ff) << 16)); \
    } \
    /* manage missing src alpha */ \
    if ((pict)->pFormat->direct.alphaMask == 0) \
	(bits) |= 0xff000000; \
    fbFinishAccess ((pict)->pDrawable); \
}

#define fbComposeGetStart(pict,x,y,type,stride,line,mul) {\
    FbBits	*__bits__; \
    FbStride	__stride__; \
    int		__bpp__; \
    int		__xoff__,__yoff__; \
\
    fbGetDrawable((pict)->pDrawable,__bits__,__stride__,__bpp__,__xoff__,__yoff__); \
    (stride) = __stride__ * sizeof (FbBits) / sizeof (type); \
    (line) = ((type *) __bits__) + (stride) * ((y) + __yoff__) + (mul) * ((x) + __xoff__); \
}
#define cvt8888to0565(s)    ((((s) >> 3) & 0x001f) | \
			     (((s) >> 5) & 0x07e0) | \
			     (((s) >> 8) & 0xf800))
#define cvt0565to0888(s)    (((((s) << 3) & 0xf8) | (((s) >> 2) & 0x7)) | \
			     ((((s) << 5) & 0xfc00) | (((s) >> 1) & 0x300)) | \
			     ((((s) << 8) & 0xf80000) | (((s) << 3) & 0x70000)))

#if IMAGE_BYTE_ORDER == MSBFirst
#define Fetch24(a)  ((unsigned long) (a) & 1 ? \
		     ((READ(a) << 16) | READ((CARD16 *) ((a)+1))) : \
		     ((READ((CARD16 *) (a)) << 8) | READ((a)+2)))
#define Store24(a,v) ((unsigned long) (a) & 1 ? \
		      (WRITE(a, (CARD8) ((v) >> 16)), \
		       WRITE((CARD16 *) ((a)+1), (CARD16) (v))) : \
		      (WRITE((CARD16 *) (a), (CARD16) ((v) >> 8)), \
		       WRITE((a)+2, (CARD8) (v))))
#else
#define Fetch24(a)  ((unsigned long) (a) & 1 ? \
		     (READ(a) | (READ((CARD16 *) ((a)+1)) << 8)) : \
		     (READ((CARD16 *) (a)) | (READ((a)+2) << 16)))
#define Store24(a,v) ((unsigned long) (a) & 1 ? \
		      (WRITE(a, (CARD8) (v)), \
		       WRITE((CARD16 *) ((a)+1), (CARD16) ((v) >> 8))) : \
		      (WRITE((CARD16 *) (a), (CARD16) (v)),\
		       WRITE((a)+2, (CARD8) ((v) >> 16))))
#endif
		      
/*
   The methods below use some tricks to be able to do two color
   components at the same time.
*/

/*
  x_c = (x_c * a) / 255
*/
#define FbByteMul(x, a) do {                                      \
        CARD32 t = ((x & 0xff00ff) * a) + 0x800080;               \
        t = (t + ((t >> 8) & 0xff00ff)) >> 8;                     \
        t &= 0xff00ff;                                            \
                                                                  \
        x = (((x >> 8) & 0xff00ff) * a) + 0x800080;               \
        x = (x + ((x >> 8) & 0xff00ff));                          \
        x &= 0xff00ff00;                                          \
        x += t;                                                   \
    } while (0)

/*
  x_c = (x_c * a) / 255 + y
*/
#define FbByteMulAdd(x, a, y) do {                                \
        CARD32 t = ((x & 0xff00ff) * a) + 0x800080;               \
        t = (t + ((t >> 8) & 0xff00ff)) >> 8;                     \
        t &= 0xff00ff;                                            \
        t += y & 0xff00ff;                                        \
        t |= 0x1000100 - ((t >> 8) & 0xff00ff);                   \
        t &= 0xff00ff;                                            \
                                                                  \
        x = (((x >> 8) & 0xff00ff) * a) + 0x800080;                 \
        x = (x + ((x >> 8) & 0xff00ff)) >> 8;                       \
        x &= 0xff00ff;                                              \
        x += (y >> 8) & 0xff00ff;                                   \
        x |= 0x1000100 - ((x >> 8) & 0xff00ff);                     \
        x &= 0xff00ff;                                              \
        x <<= 8;                                                    \
        x += t;                                                     \
    } while (0)

/*
  x_c = (x_c * a + y_c * b) / 255
*/
#define FbByteAddMul(x, a, y, b) do {                                   \
        CARD32 t;                                                       \
        CARD32 r = (x >> 24) * a + (y >> 24) * b + 0x80;                \
        r += (r >> 8);                                                  \
        r >>= 8;                                                        \
                                                                        \
        t = (x & 0xff00) * a + (y & 0xff00) * b;                        \
        t += (t >> 8) + 0x8000;                                         \
        t >>= 16;                                                       \
                                                                        \
        t |= r << 16;                                                   \
        t |= 0x1000100 - ((t >> 8) & 0xff00ff);                         \
        t &= 0xff00ff;                                                  \
        t <<= 8;                                                        \
                                                                        \
        r = ((x >> 16) & 0xff) * a + ((y >> 16) & 0xff) * b + 0x80;     \
        r += (r >> 8);                                                  \
        r >>= 8;                                                        \
                                                                        \
        x = (x & 0xff) * a + (y & 0xff) * b + 0x80;                     \
        x += (x >> 8);                                                  \
        x >>= 8;                                                        \
        x |= r << 16;                                                   \
        x |= 0x1000100 - ((x >> 8) & 0xff00ff);                         \
        x &= 0xff00ff;                                                  \
        x |= t;                                                         \
} while (0)

/*
  x_c = (x_c * a + y_c *b) / 256
*/
#define FbByteAddMul_256(x, a, y, b) do {                               \
        CARD32 t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;             \
        t >>= 8;                                                        \
        t &= 0xff00ff;                                                  \
                                                                        \
        x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;      \
        x &= 0xff00ff00;                                                \
        x += t;                                                         \
} while (0)
/*
  x_c = (x_c * a_c) / 255
*/
#define FbByteMulC(x, a) do {                           \
        CARD32 t;                                       \
        CARD32 r = (x & 0xff) * (a & 0xff);             \
        r |= (x & 0xff0000) * ((a >> 16) & 0xff);       \
	r += 0x800080;					\
        r = (r + ((r >> 8) & 0xff00ff)) >> 8;           \
        r &= 0xff00ff;                                  \
                                                        \
        x >>= 8;                                        \
        t = (x & 0xff) * ((a >> 8) & 0xff);             \
        t |= (x & 0xff0000) * (a >> 24);                \
        t += 0x800080;                                  \
        t = t + ((t >> 8) & 0xff00ff);                  \
        x = r | (t & 0xff00ff00);                       \
                                                        \
    } while (0)

/*
  x_c = (x_c * a) / 255 + y
*/
#define FbByteMulAddC(x, a, y) do {                                 \
        CARD32 t;                                                   \
        CARD32 r = (x & 0xff) * (a & 0xff);                         \
        r |= (x & 0xff0000) * ((a >> 16) & 0xff);                   \
	r += 0x800080;						    \
	r = (r + ((r >> 8) & 0xff00ff)) >> 8;			    \
        r &= 0xff00ff;                                              \
        r += y & 0xff00ff;                                          \
        r |= 0x1000100 - ((r >> 8) & 0xff00ff);                     \
        r &= 0xff00ff;                                              \
                                                                    \
        x >>= 8;                                                       \
        t = (x & 0xff) * ((a >> 8) & 0xff);                            \
        t |= (x & 0xff0000) * (a >> 24);                               \
	t += 0x800080;                                                 \
        t = (t + ((t >> 8) & 0xff00ff)) >> 8;			       \
        t &= 0xff00ff;                                                 \
        t += (y >> 8) & 0xff00ff;                                      \
        t |= 0x1000100 - ((t >> 8) & 0xff00ff);                        \
        t &= 0xff00ff;                                                 \
        x = r | (t << 8);                                              \
    } while (0)

/*
  x_c = (x_c * a_c + y_c * b) / 255
*/
#define FbByteAddMulC(x, a, y, b) do {                                  \
        CARD32 t;                                                       \
        CARD32 r = (x >> 24) * (a >> 24) + (y >> 24) * b;               \
        r += (r >> 8) + 0x80;                                           \
        r >>= 8;                                                        \
                                                                        \
        t = (x & 0xff00) * ((a >> 8) & 0xff) + (y & 0xff00) * b;        \
        t += (t >> 8) + 0x8000;                                         \
        t >>= 16;                                                       \
                                                                        \
        t |= r << 16;                                                   \
        t |= 0x1000100 - ((t >> 8) & 0xff00ff);                         \
        t &= 0xff00ff;                                                  \
        t <<= 8;                                                        \
                                                                        \
        r = ((x >> 16) & 0xff) * ((a >> 16) & 0xff) + ((y >> 16) & 0xff) * b + 0x80; \
        r += (r >> 8);                                                  \
        r >>= 8;                                                        \
                                                                        \
        x = (x & 0xff) * (a & 0xff) + (y & 0xff) * b + 0x80;            \
        x += (x >> 8);                                                  \
        x >>= 8;                                                        \
        x |= r << 16;                                                   \
        x |= 0x1000100 - ((x >> 8) & 0xff00ff);                         \
        x &= 0xff00ff;                                                  \
        x |= t;                                                         \
    } while (0)
 
/*
  x_c = min(x_c + y_c, 255)
*/
#define FbByteAdd(x, y) do {                                            \
        CARD32 t;                                                       \
        CARD32 r = (x & 0xff00ff) + (y & 0xff00ff);                     \
        r |= 0x1000100 - ((r >> 8) & 0xff00ff);                         \
        r &= 0xff00ff;                                                  \
                                                                        \
        t = ((x >> 8) & 0xff00ff) + ((y >> 8) & 0xff00ff);              \
        t |= 0x1000100 - ((t >> 8) & 0xff00ff);                         \
        r |= (t & 0xff00ff) << 8;                                       \
        x = r;                                                          \
    } while (0)

#define div_255(x) (((x) + 0x80 + (((x) + 0x80) >> 8)) >> 8)

#if defined(__i386__) && defined(__GNUC__)
#define FASTCALL __attribute__((regparm(3)))
#else
#define FASTCALL
#endif

typedef struct _FbComposeData {
    CARD8	op;
    PicturePtr	src;
    PicturePtr	mask;
    PicturePtr	dest;
    INT16	xSrc;
    INT16	ySrc;
    INT16	xMask;
    INT16	yMask;
    INT16	xDest;
    INT16	yDest;
    CARD16	width;
    CARD16	height;
} FbComposeData;

extern _X_EXPORT void
fbCompositeRect (const FbComposeData *data, CARD32 *scanline_buffer);

typedef FASTCALL void (*CombineMaskU) (CARD32 *src, const CARD32 *mask, int width);
typedef FASTCALL void (*CombineFuncU) (CARD32 *dest, const CARD32 *src, int width);
typedef FASTCALL void (*CombineFuncC) (CARD32 *dest, CARD32 *src, CARD32 *mask, int width);

typedef struct _FbComposeFunctions {
    CombineFuncU *combineU;
    CombineFuncC *combineC;
    CombineMaskU combineMaskU;
} FbComposeFunctions;

/* fbcompose.c */

extern _X_EXPORT void
fbCompositeGeneral (CARD8	op,
		    PicturePtr	pSrc,
		    PicturePtr	pMask,
		    PicturePtr	pDst,
		    INT16	xSrc,
		    INT16	ySrc,
		    INT16	xMask,
		    INT16	yMask,
		    INT16	xDst,
		    INT16	yDst,
		    CARD16	width,
		    CARD16	height);

/* fbpict.c */
extern _X_EXPORT void
fbComposite (CARD8      op,
	     PicturePtr pSrc,
	     PicturePtr pMask,
	     PicturePtr pDst,
	     INT16      xSrc,
	     INT16      ySrc,
	     INT16      xMask,
	     INT16      yMask,
	     INT16      xDst,
	     INT16      yDst,
	     CARD16     width,
	     CARD16     height);

typedef void	(*CompositeFunc) (CARD8      op,
				  PicturePtr pSrc,
				  PicturePtr pMask,
				  PicturePtr pDst,
				  INT16      xSrc,
				  INT16      ySrc,
				  INT16      xMask,
                                  INT16      yMask,
				  INT16      xDst,
				  INT16      yDst,
				  CARD16     width,
				  CARD16     height);

extern _X_EXPORT void
fbWalkCompositeRegion (CARD8 op,
		       PicturePtr pSrc,
		       PicturePtr pMask,
		       PicturePtr pDst,
		       INT16 xSrc,
		       INT16 ySrc,
		       INT16 xMask,
		       INT16 yMask,
		       INT16 xDst,
		       INT16 yDst,
		       CARD16 width,
		       CARD16 height,
		       Bool srcRepeat,
		       Bool maskRepeat,
		       CompositeFunc compositeRect);

/* fbtrap.c */

extern _X_EXPORT void
fbAddTraps (PicturePtr	pPicture,
	    INT16	xOff,
	    INT16	yOff,
	    int		ntrap,
	    xTrap	*traps);

extern _X_EXPORT void
fbRasterizeTrapezoid (PicturePtr    alpha,
		      xTrapezoid    *trap,
		      int	    x_off,
		      int	    y_off);

extern _X_EXPORT void
fbAddTriangles (PicturePtr  pPicture,
		INT16	    xOff,
		INT16	    yOff,
		int	    ntri,
		xTriangle   *tris);

#endif /* _FBPICT_H_ */
