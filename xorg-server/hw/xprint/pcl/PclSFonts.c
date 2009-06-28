/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:          PclSFonts.c
**    *
**    *  Contents:
**    *                 Send Soft Font Download data to the specified
**    *                 file pointer.
**    *
**    *  Created:       3/4/96
**    *
**    *********************************************************
**
********************************************************************/
/*
(c) Copyright 1996 Hewlett-Packard Company
(c) Copyright 1996 International Business Machines Corp.
(c) Copyright 1996 Sun Microsystems, Inc.
(c) Copyright 1996 Novell, Inc.
(c) Copyright 1996 Digital Equipment Corp.
(c) Copyright 1996 Fujitsu Limited
(c) Copyright 1996 Hitachi, Ltd.

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


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include "Pcl.h"

static char tmp1;
static short tmp2;
#define Put1byte(fp, x)		tmp1=x; fwrite((char *)&tmp1, 1, 1, fp)
#define Put2bytes(fp, x)	tmp2=x; fwrite((char *)&tmp2, 2, 1, fp)

#define ESC 0x1b
#define SYMBOL_SET 277

static unsigned int PclDownloadChar(FILE *,PclCharDataPtr,unsigned short,unsigned char);
static unsigned int PclDownloadHeader(FILE *, PclFontDescPtr, unsigned short);

#ifdef PCL_FONT_COMPRESS
static unsigned char *compress_bitmap_data(PclCharDataPtr, unsigned int *);
#endif /* PCL_FONT_COMPRESS */

/* -*- PclDownloadSoftFont8 -*-
 * Send the Character Definition Command for 8-bit font
 * **************************************************************************/
void
PclDownloadSoftFont8(
    FILE *fp,
    PclSoftFontInfoPtr pSoftFontInfo,
    PclFontHead8Ptr pfh,
    PclCharDataPtr pcd,
    unsigned char *code
)
{
    /*
     * Check whether the font header has already been downloaded.
     * If not, download it.
     */

    if ( !pfh->fid ) {
	pfh->fid = pSoftFontInfo->cur_max_fid++;
	PclDownloadHeader(fp, &(pfh->fd), pfh->fid);
    }
    pfh->index[*code] = *code;
    PclDownloadChar(fp, pcd, pfh->fid, pfh->index[*code]);

}

/* -*- PclDownloadSoftFont16 -*-
 * Send the Character Definition Command for 16 bit font
 * **************************************************************************/
void
PclDownloadSoftFont16(
    FILE *fp,
    PclSoftFontInfoPtr pSoftFontInfo,
    PclFontHead16Ptr pfh,
    PclCharDataPtr pcd,
    unsigned char row,
    unsigned char col
)
{
    /*
     * Check whether the font header is already downloaded.
     * If not, download it.
     */

    if ( !pfh->cur_cindex ) {
	pfh->cur_fid = pSoftFontInfo->cur_max_fid++;
	PclDownloadHeader(fp, &(pfh->fd), pfh->cur_fid);
    }
    pfh->index[row][col].fid = pfh->cur_fid;
    pfh->index[row][col].cindex = pfh->cur_cindex++;

    PclDownloadChar(fp, pcd, pfh->index[row][col].fid, pfh->index[row][col].cindex);
}

/* -*- PclCreateSoftFontInfo -*-
 * Create and Initialize the structure for storing the information
 * of the downloaded soft font.
 * **************************************************************************/
PclSoftFontInfoPtr
PclCreateSoftFontInfo(void)
{
PclSoftFontInfoPtr pSoftFontInfo;

    pSoftFontInfo = (PclSoftFontInfoPtr)xalloc(sizeof(PclSoftFontInfoRec));
    if ( pSoftFontInfo == (PclSoftFontInfoPtr) NULL)
	return (PclSoftFontInfoPtr) NULL;
    pSoftFontInfo->phead8 = (PclFontHead8Ptr)NULL;
    pSoftFontInfo->phead16 = (PclFontHead16Ptr)NULL;
    pSoftFontInfo->pinfont = (PclInternalFontPtr)NULL;
    pSoftFontInfo->cur_max_fid = 1;
    return pSoftFontInfo;
}

/* -*- PclDestroySoftFontInfo -*-
 * Destroy the soft font information structure
 * **************************************************************************/
void
PclDestroySoftFontInfo( PclSoftFontInfoPtr pSoftFontInfo )
{
PclFontHead8Ptr  pfh8,  pfh8_next;
PclFontHead16Ptr pfh16, pfh16_next;
PclInternalFontPtr pin, pin_next;
unsigned char nindex_row;
int i;

    if ( pSoftFontInfo == (PclSoftFontInfoPtr) NULL )
	return;

    pfh8  = pSoftFontInfo->phead8;
    while (pfh8 != (PclFontHead8Ptr) NULL) {
	xfree(pfh8->fontname);
	xfree(pfh8->index);
	pfh8_next = pfh8->next;
	xfree(pfh8);
	pfh8 = pfh8_next;
    }

    pfh16 = pSoftFontInfo->phead16;
    while (pfh16 != (PclFontHead16Ptr) NULL) {
	xfree(pfh16->fontname);
	nindex_row = pfh16->lastRow - pfh16->firstRow + 1;
	for (i=0; i<nindex_row; i++)
	    xfree(pfh16->index[i]);
	xfree(pfh16->index);
	pfh16_next = pfh16->next;
	xfree(pfh16);
	pfh16 = pfh16_next;
    }

    pin = pSoftFontInfo->pinfont;
    while (pin != (PclInternalFontPtr) NULL) {
	xfree(pin->fontname);
	pin_next = pin->next;
	xfree(pin);
	pin = pin_next;
    }

    xfree(pSoftFontInfo);
}

/* -*- PclDownloadHeader -*-
 * Send the Font Header Commnad. 
 * 	Format 0  : Font Header for Pcl Bitmapped Fonts
 * 	Format 20 : Font Header for Resolution Specified Bitmapped Fonts
 * **************************************************************************/
static unsigned int
PclDownloadHeader(
    FILE *fp,
    PclFontDescPtr fd,
    unsigned short fid
)
{
int nbytes;

#ifdef XP_PCL_LJ3
    nbytes = 64;
#else
    nbytes = 68;
#endif /* XP_PCL_LJ3 */
    /*
     * Font ID Command : Esc *c#D
     *		(Default = 0, Range = 0 - 32767)
     */
    fprintf(fp, "%c*c%dD", ESC, fid);

    /*
     * Font Header Commnad : Esc )s#W[font header data]
     *		(Default = 0, Range = 0 - 32767)
     */
    fprintf(fp, "%c)s%dW", ESC, nbytes);

    Put2bytes(fp, nbytes);			/* Font Description Size */
#ifdef XP_PCL_LJ3
    Put1byte(fp, 0);				/* Header Format */
#else
    Put1byte(fp, 20);				/* Header Format */
#endif /* XP_PCL_LJ3 */
    Put1byte(fp, 2);				/* Font Type */
    Put2bytes(fp, 0);				/* Style MSB */
    Put2bytes(fp, fd->ascent);			/* BaseLine Position */
    Put2bytes(fp, fd->cellwidth);		/* Cell Width */
    Put2bytes(fp, fd->cellheight);		/* Cell Height */
    Put1byte(fp, 0);				/* Orienation */
    Put1byte(fp, fd->spacing);			/* Spacing */
    Put2bytes(fp, SYMBOL_SET);			/* Symbol Set */
    Put2bytes(fp, fd->pitch*4);			/* font pitch */
    Put2bytes(fp, fd->cellheight * 4);		/* Height */
    Put2bytes(fp, 0);				/* x-Height */
    Put1byte(fp, 0);				/* width type (normal) */
    Put1byte(fp, 0);				/* Style LSB */
    Put1byte(fp, 0);				/* Stroke Weight */ 
    Put1byte(fp, 5);				/* Typeface LSB */
    Put1byte(fp, 0);				/* Typeface MSB */
    Put1byte(fp, 0);				/* Serif Style */
    Put1byte(fp, 0);				/* Quality */
    Put1byte(fp, 0);				/* Placement */
    Put1byte(fp, 0);				/* Underline Position */
    Put1byte(fp, 0);				/* Underline Thickness */
    Put2bytes(fp, fd->cellheight*1.2);		/* Text Height */
    Put2bytes(fp, fd->cellwidth * 4);		/* Text Width */
    Put2bytes(fp, 0);				/* First Code */
    Put2bytes(fp, 255);				/* Last Code */
    Put1byte(fp, 0);				/* Pitch Extend */
    Put1byte(fp, 0);				/* Height Extend */
    Put2bytes(fp, 0);				/* Cap Height */
    Put2bytes(fp, 0);				/* Font Number 1 */
    Put2bytes(fp, 0);				/* Font Number 2 */
    Put2bytes(fp, 0);				/* Font Name */
    Put2bytes(fp, 0);				/* Font Name */
    Put2bytes(fp, 0);				/* Font Name */
    Put2bytes(fp, 0);				/* Font Name */
    Put2bytes(fp, 0);				/* Font Name */
    Put2bytes(fp, 0);				/* Font Name */
    Put2bytes(fp, 0);				/* Font Name */
    Put2bytes(fp, 0);				/* Font Name */

#ifdef XP_PCL_LJ3
    return 64;
#else
    Put2bytes(fp, 300);				/* X Resolution */
    Put2bytes(fp, 300);				/* Y Resolution */
    return 68;
#endif /* XP_PCL_LJ3 */

}

/* -*- PclDownloadCharacter -*-
 * Send the Character Definition Command.
 * **************************************************************************/
static unsigned int
PclDownloadChar(
    FILE *fp,
    PclCharDataPtr cd,
    unsigned short fid,
    unsigned char code
)
{
unsigned int nbytes, n;
unsigned char *raster;

    /*
     * Font ID Command : Esc *c#D
     *		(Default = 0, Range = 0 - 32767)
     * Character Code Command : Esc *c#E
     *		(Default = 0, Range = 0 - 65535)
     */
    fprintf(fp, "%c*c%dd%dE", ESC, fid, code);

    /*
     * Character Definition Command : Esc (s#W[character descriptor and data]
     *		(Default = N/A, Range = 0 - 32767)
     */

    nbytes = n = cd->height * ((cd->width + 7) / 8);
#ifdef PCL_FONT_COMPRESS
    raster = compress_bitmap_data(cd, &nbytes);
#else
    raster = (unsigned char *)NULL;
#endif /* PCL_FONT_COMPRESS */
    fprintf(fp, "%c(s%dW", ESC, nbytes + 16);

    Put1byte(fp, 4);				/* Format */
    Put1byte(fp, 0);				/* Continuation */
    Put1byte(fp, 14);				/* Descriptor Size */
    if (raster) {				/* Class */
	Put1byte(fp, 2);
    } else {
	Put1byte(fp, 1);			/* Class */
    }
    Put2bytes(fp, 0);				/* Orientation */
    Put2bytes(fp, cd->h_offset);		/* left offset */
    Put2bytes(fp, cd->v_offset);		/* top offset */
    Put2bytes(fp, cd->width);			/* character width */
    Put2bytes(fp, cd->height);			/* character height */
    Put2bytes(fp, cd->font_pitch*4);		/* delta X */

    /*
     * Raster Character Data
     */
    if (raster) {
	fwrite(raster, nbytes, 1, fp);
	xfree(raster);
    } else
	fwrite(cd->raster_top, nbytes, 1, fp);

    return n + 16;
}


#ifdef PCL_FONT_COMPRESS
/* -*- compress_bitmap_data -*-
 * Compress Bitmap data
 * **************************************************************************/
static unsigned char *
compress_bitmap_data(
    PclCharDataPtr cd,
    unsigned int *nbytes
)
{
unsigned int  byte_width;
unsigned char *raster, *rptr_s, *rptr_e, *rptr_end;
unsigned char *tmp_s, *tmp_ptr;
unsigned char *p;
unsigned char cur, pixel;
unsigned int num;

int i, j, k, w;

    byte_width = (cd->width + 7) / 8;
    *nbytes = cd->height * byte_width;

    /* Create buffer for storing compress bitmap glyph  */
    raster = (unsigned char *)xalloc(*nbytes);
    rptr_s = raster;
    rptr_e = raster;
    rptr_end = raster + *nbytes;

    tmp_s = (unsigned char *)xalloc(cd->width * 8 + 2);

    p = cd->raster_top;
    for (i=0; i<cd->height; i++) {
	tmp_ptr = tmp_s;
	*tmp_ptr++ = 0;
	if ( (*p>>7)&0x1 == 1 ) {
	    *tmp_ptr++ = 0;
	    cur = 1;
	} else {
	    cur = 0;
	}
	num = 0;
	for (j=0, w=0; j<byte_width; j++, p++) {
	    for (k=0; k<8 && w<cd->width; k++, w++) {
		pixel = (*p>>(7-k))&0x1;
		if ( pixel == cur ) {
		    num++;
		} else {
		    cur = pixel;
		    while (num > 255) {
			*tmp_ptr++ = 255;
			*tmp_ptr++ = 0;
			num -= 255;
		    }
		    *tmp_ptr++ = num;
		    num = 1;
		}
	    }
	}
	if ( pixel == cur ) {
	    while (num > 255) {
		*tmp_ptr++ = 255;
		*tmp_ptr++ = 0;
		num -= 255;
	    }
	    *tmp_ptr++ = num&0xff;
	} else
	    *tmp_ptr++ = num;

	if ( ((rptr_e - rptr_s) == (tmp_ptr - tmp_s)) &&
			!memcmp(rptr_s+1, tmp_s+1, (tmp_ptr - tmp_s) - 1) )
	    *rptr_s += 1;
	else {
	    if ( rptr_e + (tmp_ptr - tmp_s) > rptr_end ) {
		xfree(raster);
		xfree(tmp_s);
		return (unsigned char *)NULL;
	    }
	    memcpy (rptr_e, tmp_s, tmp_ptr - tmp_s);
	    rptr_s = rptr_e;
	    rptr_e = rptr_s + (tmp_ptr - tmp_s);
	}
    }
    xfree(tmp_s);
    *nbytes = rptr_e - raster;

    return raster;
}
#endif /* PCL_FONT_COMPRESS */
