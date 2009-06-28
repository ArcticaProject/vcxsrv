
/*
Copyright (c) 2005 Roland Mainz <roland.mainz@nrubsig.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the copyright holders shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from said
copyright holders.
*/

/* Please do not beat me for this ugly code - most of it has been stolen from
 * xc/lib/X11/ImUtil.c */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "Ps.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "servermd.h"
#include "attributes.h"

static unsigned char const _reverse_byte[0x100] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

static
int XReverse_Bytes(
    register unsigned char *bpt,
    register int nb)
{
    do {
        *bpt = _reverse_byte[*bpt];
        bpt++;
    } while (--nb > 0);
    return 0;
}

/*
 * Data structure for "image" data, used by image manipulation routines.
 */
typedef struct {
    int width, height;          /* size of image */
    int xoffset;                /* number of pixels offset in X direction */
    int format;                 /* XYBitmap, XYPixmap, ZPixmap */
    char *data;                 /* pointer to image data */
    int byte_order;             /* data byte order, LSBFirst, MSBFirst */
    int bitmap_unit;            /* quant. of scanline 8, 16, 32 */
    int bitmap_bit_order;       /* LSBFirst, MSBFirst */
    int depth;                  /* depth of image */
    int bytes_per_line;         /* accelarator to next line */
    int bits_per_pixel;         /* bits per pixel (ZPixmap) */
} TmpImage;


static void xynormalizeimagebits (
    register unsigned char *bp,
    register TmpImage *img)
{
	register unsigned char c;

	if (img->byte_order != img->bitmap_bit_order) {
	    switch (img->bitmap_unit) {

		case 16:
		    c = *bp;
		    *bp = *(bp + 1);
		    *(bp + 1) = c;
		    break;

		case 32:
		    c = *(bp + 3);
		    *(bp + 3) = *bp;
		    *bp = c;
		    c = *(bp + 2);
		    *(bp + 2) = *(bp + 1);
		    *(bp + 1) = c;
		    break;
	    }
	}
	if (img->bitmap_bit_order == MSBFirst)
	    XReverse_Bytes (bp, img->bitmap_unit >> 3);
}

static void znormalizeimagebits (
    register unsigned char *bp,
    register TmpImage *img)
{
	register unsigned char c;
	switch (img->bits_per_pixel) {

	    case 4:
		*bp = ((*bp >> 4) & 0xF) | ((*bp << 4) & ~0xF);
		break;

	    case 16:
		c = *bp;
		*bp = *(bp + 1);
		*(bp + 1) = c;
		break;

	    case 24:
		c = *(bp + 2);
		*(bp + 2) = *bp;
		*bp = c;
		break;

	    case 32:
		c = *(bp + 3);
		*(bp + 3) = *bp;
		*bp = c;
		c = *(bp + 2);
		*(bp + 2) = *(bp + 1);
		*(bp + 1) = c;
		break;
	}
}

/*
 * Macros
 * 
 * The ROUNDUP macro rounds up a quantity to the specified boundary,
 * then truncates to bytes.
 *
 * The XYNORMALIZE macro determines whether XY format data requires 
 * normalization and calls a routine to do so if needed. The logic in
 * this module is designed for LSBFirst byte and bit order, so 
 * normalization is done as required to present the data in this order.
 *
 * The ZNORMALIZE macro performs byte and nibble order normalization if 
 * required for Z format data.
 *
 * The XYINDEX macro computes the index to the starting byte (char) boundary
 * for a bitmap_unit containing a pixel with coordinates x and y for image
 * data in XY format.
 * 
 * The ZINDEX macro computes the index to the starting byte (char) boundary 
 * for a pixel with coordinates x and y for image data in ZPixmap format.
 * 
 */

#if defined(Lynx) && defined(ROUNDUP)
#undef ROUNDUP
#endif

#define ROUNDUP(nbytes, pad) ((((nbytes) + ((pad)-1)) / (pad)) * ((pad)>>3))

#define XYNORMALIZE(bp, img) \
    if ((img->byte_order == MSBFirst) || (img->bitmap_bit_order == MSBFirst)) \
	xynormalizeimagebits((unsigned char *)(bp), img)

#define ZNORMALIZE(bp, img) \
    if (img->byte_order == MSBFirst) \
	znormalizeimagebits((unsigned char *)(bp), img)

#define XYINDEX(x, y, img) \
    ((y) * img->bytes_per_line) + \
    (((x) + img->xoffset) / img->bitmap_unit) * (img->bitmap_unit >> 3)

#define ZINDEX(x, y, img) ((y) * img->bytes_per_line) + \
    (((x) * img->bits_per_pixel) >> 3)

/*
 * GetPixel
 * 
 * Returns the specified pixel.  The X and Y coordinates are relative to 
 * the origin (upper left [0,0]) of the image.  The pixel value is returned
 * in normalized format, i.e. the LSB of the long is the LSB of the pixel.
 * The algorithm used is:
 *
 *	copy the source bitmap_unit or Zpixel into temp
 *	normalize temp if needed
 *	extract the pixel bits into return value
 *
 */

static unsigned long const low_bits_table[] = {
    0x00000000, 0x00000001, 0x00000003, 0x00000007,
    0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
    0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
    0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
    0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
    0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
    0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
    0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,
    0xffffffff
};

static unsigned long XGetPixel (TmpImage *ximage, int x, int y)
{
	unsigned long pixel, px;
	register char *src;
	register char *dst;
	register int i, j;
	int bits, nbytes;
	long plane;
     
	if ((ximage->bits_per_pixel | ximage->depth) == 1) {
		src = &ximage->data[XYINDEX(x, y, ximage)];
		dst = (char *)&pixel;
		pixel = 0;
		for (i = ximage->bitmap_unit >> 3; --i >= 0; ) *dst++ = *src++;
		XYNORMALIZE(&pixel, ximage);
          	bits = (x + ximage->xoffset) % ximage->bitmap_unit;
		pixel = ((((char *)&pixel)[bits>>3])>>(bits&7)) & 1;
	} else if (ximage->format == XYPixmap) {
		pixel = 0;
		plane = 0;
		nbytes = ximage->bitmap_unit >> 3;
		for (i = ximage->depth; --i >= 0; ) {
		    src = &ximage->data[XYINDEX(x, y, ximage)+ plane];
		    dst = (char *)&px;
		    px = 0;
		    for (j = nbytes; --j >= 0; ) *dst++ = *src++;
		    XYNORMALIZE(&px, ximage);
		    bits = (x + ximage->xoffset) % ximage->bitmap_unit;
		    pixel = (pixel << 1) |
			    (((((char *)&px)[bits>>3])>>(bits&7)) & 1);
		    plane = plane + (ximage->bytes_per_line * ximage->height);
		}
	} else if (ximage->format == ZPixmap) {
		src = &ximage->data[ZINDEX(x, y, ximage)];
		dst = (char *)&px;
		px = 0;
		for (i = (ximage->bits_per_pixel + 7) >> 3; --i >= 0; )
		    *dst++ = *src++;		
		ZNORMALIZE(&px, ximage);
		pixel = 0;
		for (i=sizeof(unsigned long); --i >= 0; )
		    pixel = (pixel << 8) | ((unsigned char *)&px)[i];
		if (ximage->bits_per_pixel == 4) {
		    if (x & 1)
			pixel >>= 4;
		    else
			pixel &= 0xf;
		}
	} else {
		return 0; /* bad image */
	}
	if (ximage->bits_per_pixel == ximage->depth)
	  return pixel;
	else
	  return (pixel & low_bits_table[ximage->depth]);
}

unsigned long
PsGetImagePixel(char *pImage, int depth, int w, int h, int leftPad, int format,
                int px, int py)
{
  TmpImage xi = {0};
  
  xi.width            = w;
  xi.height           = h;
  xi.xoffset          = 0/*leftPad*/;
  xi.format           = format;
  xi.data             = pImage;
  xi.byte_order       = IMAGE_BYTE_ORDER;
  xi.bitmap_bit_order = BITMAP_BIT_ORDER;
  xi.bitmap_unit      = ((depth > 16)?(32):
                        ((depth >  8)?(16):
                        ((depth >  1)? (8):
                                  (1))));
  xi.depth            = depth;
  xi.bits_per_pixel   = xi.bitmap_unit;

  /*
   * compute per line accelerator.
   */
  if (format == ZPixmap)
      xi.bytes_per_line = 
          ROUNDUP((xi.bits_per_pixel * xi.width), 32);
  else
      xi.bytes_per_line =
          ROUNDUP((xi.width + xi.xoffset), 32); 

  return XGetPixel(&xi, px, py);
}



