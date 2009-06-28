/* $Xorg: t1funcs.c,v 1.5 2001/02/09 02:04:01 xorgcvs Exp $ */
/* Copyright International Business Machines,Corp. 1991
 * All Rights Reserved
 *
 * License, subject to the license given below, to use,
 * copy, modify, and distribute this software * and its
 * documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear
 * in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation,
 * and that the name of IBM not be used in advertising or
 * publicity pertaining to distribution of the software
 * without specific, written prior permission.
 *
 * IBM PROVIDES THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES
 * OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT
 * LIMITED TO ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE QUALITY AND
 * PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF
 * THE SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM) ASSUMES
 * THE ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN
 * NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Author: Jeffrey B. Lotspiech, IBM Almaden Research Center
 *   Modeled on spfuncs.c by Dave Lemke, Network Computing Devices, Inc
 *   which contains the following copyright and permission notices:
 *
 * Copyright 1990, 1991 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices
 * or Digital not be used in advertising or publicity pertaining to 
 * distribution of the software without specific, written prior permission.  
 * Network Computing Devices or Digital make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * NETWORK COMPUTING DEVICES AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES OR DIGITAL BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* $XFree86: xc/lib/font/Type1/t1funcs.c,v 3.33 2003/07/19 13:16:40 tsi Exp $ */

/*

Copyright 1987, 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef FONTMODULE
#include <string.h>
#ifdef _XOPEN_SOURCE
#include <math.h>
#else
#define _XOPEN_SOURCE	/* to get prototype for hypot on some systems */
#include <math.h>
#undef _XOPEN_SOURCE
#endif
#include "X11/Xfuncs.h"
#ifdef USE_MMAP
#include <sys/types.h>
#include <sys/mman.h>
#endif
#else
#include "Xmd.h"
#include "Xdefs.h"
#endif

#ifdef FONTMODULE
#include "os.h"
#include "xf86_ansic.h"
#endif

#include <X11/fonts/fntfilst.h>
#include <X11/fonts/fontutil.h>
#include <X11/fonts/FSproto.h>
#include <X11/fonts/fontenc.h>
#include "t1unicode.h"
 
#include "objects.h"
#include "spaces.h"
#include "paths.h"
#include "regions.h"
#include "t1stdio.h"
#include "util.h"
#include "fontfcn.h"
#include "t1intf.h"


static int Type1GetGlyphs ( FontPtr pFont, unsigned long count,
			    unsigned char *chars, FontEncoding charEncoding, 
			    unsigned long *glyphCount, CharInfoPtr *glyphs );

static int Type1GetMetrics ( FontPtr pFont, unsigned long count, 
				    unsigned char *chars, 
				    FontEncoding charEncoding, 
				    unsigned long *glyphCount, 
				    xCharInfo **glyphs );

 
#define minchar(p) ((p).min_char_low + ((p).min_char_high << 8))
#define maxchar(p) ((p).max_char_low + ((p).max_char_high << 8))

static void fillrun ( char *p, pel x0, pel x1, int bit );
 
extern psfont *FontP;
extern psobj *ISOLatin1EncArrayP;

static void fill ( char *dest, int h, int w, struct region *area, int byte, 
		   int bit, int wordsize );

static int 
Type1ReturnCodeToXReturnCode(int rc)
{
    switch(rc) {
    case SCAN_OK:
	return Successful;
    case SCAN_FILE_EOF:
	/* fall through to BadFontFormat */
    case SCAN_ERROR:
	return BadFontFormat;
    case SCAN_OUT_OF_MEMORY:
	return AllocError;
    case SCAN_FILE_OPEN_ERROR:
	return BadFontName;
    case SCAN_TRUE:
    case SCAN_FALSE:
    case SCAN_END:
	/* fall through */
    default:
	/* this should not happen */
	ErrorF("Type1 return code not convertable to X return code: %d\n", rc);
	return rc;
    }
}

/*ARGSUSED*/
int 
Type1OpenScalable (FontPathElementPtr fpe, 
		   FontPtr *ppFont, 
		   int flags, 
		   FontEntryPtr entry, 
		   char *fileName, 
		   FontScalablePtr vals, 
		   fsBitmapFormat format,
		   fsBitmapFormatMask fmask, 
		   FontPtr non_cachable_font) 	/* We don't do licensing */
{
       FontPtr     pFont;
       int         bit,
                   byte,
                   glyph,
                   scan,
                   image;
       int pad,wordsize;     /* scan & image in bits                         */
       long *pool;           /* memory pool for ximager objects              */
       int size;             /* for memory size calculations                 */
       struct XYspace *S;    /* coordinate space for character               */
       struct region *area;
       CharInfoRec *glyphs;
       int len, rc, count = 0, i = 0;
       struct type1font *type1;
       char *p;
       FontMapPtr mapping = NULL;
       int no_mapping;
       psobj *fontmatrix;
       long x0, total_width = 0, total_raw_width = 0;
       double x1, y1, t1 = .001, t2 = 0.0, t3 = 0.0, t4 = .001;
       double sxmult;

       /* Reject ridiculously small font sizes that will blow up the math */
       if (hypot(vals->pixel_matrix[0], vals->pixel_matrix[1]) < 1.0 ||
	   hypot(vals->pixel_matrix[2], vals->pixel_matrix[3]) < 1.0)
	   return BadFontName;

       /* set up default values */
       FontDefaultFormat(&bit, &byte, &glyph, &scan);
       /* get any changes made from above */
       rc = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);
       if (rc != Successful)
               return rc;
 
       pad                = glyph * 8;
       wordsize           = scan * 8;
 
#define  PAD(bits, pad)  (((bits)+(pad)-1)&-(pad))
 
       pFont = CreateFontRec();
       if (pFont == NULL)
           return AllocError;

       type1 = (struct type1font *)xalloc(sizeof(struct type1font));
       if (type1 == NULL) {
               DestroyFontRec(pFont);
               return AllocError;
       }
       bzero(type1, sizeof(struct type1font));
 
       /* heuristic for "maximum" size of pool we'll need: */
       size = 200000 + 600 *
	      (int)hypot(vals->pixel_matrix[2], vals->pixel_matrix[3])
	      * sizeof(short);
       if (size < 0 || NULL == (pool = (long *) xalloc(size))) {
               xfree(type1);
               DestroyFontRec(pFont);
               return AllocError;
       }
 
       addmemory(pool, size);
 
 
       glyphs = type1->glyphs;
 
       /* load font if not already loaded */
       if (!fontfcnA(fileName, &rc)) {
         delmemory();
	 xfree(type1);
	 DestroyFontRec(pFont);
         xfree(pool);
         return Type1ReturnCodeToXReturnCode(rc);
       }

       fontmatrix = &FontP->fontInfoP[FONTMATRIX].value;
       if (objPIsArray(fontmatrix) && fontmatrix->len == 6)
       {
#define assign(n,d,f) if (objPIsInteger(fontmatrix->data.arrayP + n)) \
			  d = fontmatrix->data.arrayP[n].data.integer; \
		      else if (objPIsReal(fontmatrix->data.arrayP + n)) \
			  d = fontmatrix->data.arrayP[n].data.real; \
		      else d = f;

	   assign(0, t1, .001);
	   assign(1, t2, 0.0);
	   assign(2, t3, 0.0);
	   assign(3, t4, .001);
       }

       S = (struct XYspace *) t1_Transform((struct xobject *)IDENTITY,
					   t1, t2, t3, t4);

       S = (struct XYspace *) Permanent(t1_Transform((struct xobject *)S, 
						     vals->pixel_matrix[0],
						     -vals->pixel_matrix[1],
						     vals->pixel_matrix[2],
						     -vals->pixel_matrix[3]));


       /* multiplier for computation of raw values */
       sxmult = hypot(vals->pixel_matrix[0], vals->pixel_matrix[1]);
       if (sxmult > EPS) sxmult = 1000.0 / sxmult;

       no_mapping=0;
       p = FontEncFromXLFD(entry->name.name, entry->name.length);

       if(p==0) {               /* XLFD does not specify an encoding */
           mapping=0;
           no_mapping=2;        /* ISO 8859-1 */
       }

       if(!strcmp(p, "adobe-fontspecific")) {
           mapping=0;
           no_mapping=1;        /* font's native encoding vector */
       }

       pFont->info.firstCol = 255;
       pFont->info.lastCol  = 0;

       if(!no_mapping) {
           mapping = FontEncMapFind(p, 
                                    FONT_ENCODING_POSTSCRIPT, -1, -1,
                                    fileName);
           if(!mapping)
               mapping = FontEncMapFind(p, 
                                        FONT_ENCODING_UNICODE, -1, -1,
                                        fileName);
           if(!mapping)
	       goto NoEncoding;
           else
               no_mapping=0;
       }

       for (i=0; i < 256; i++) {
               long h,w;
               long paddedW;
	       int j;
	       char *codename;

               if(no_mapping == 1) {
                   codename = FontP->fontInfoP[ENCODING].
                       value.data.arrayP[i].data.valueP;
                   len = FontP->fontInfoP[ENCODING].
                       value.data.arrayP[i].len;
               } else if(no_mapping) {
                   codename = unicodetoPSname(i);
                 len = codename ? strlen(codename) : 0;
               } else {
                 if(mapping->type == FONT_ENCODING_UNICODE) {
                     codename = unicodetoPSname(FontEncRecode(i, mapping));
                 } else
                     codename = FontEncName(i, mapping);
                 len=codename?strlen(codename):0;
               }

               /* Avoid multiply rasterising the undefined glyph */
               if(len==7 && !strncmp(codename, ".notdef", 7)) {
                   len=0;
                   codename=0;
               }

               /* But do rasterise it at least once */
               if(len==0) {
                   if(i==0) {
                       codename=".notdef";
                       len=7;
                   } else
                       continue;
               }

	       /* See if this character is in the list of ranges specified
		  in the XLFD name */
               if(i!=0) {
                   for (j = 0; j < vals->nranges; j++)
                       if (i >= minchar(vals->ranges[j]) &&
                           i <= maxchar(vals->ranges[j]))
                           break;

                   /* If not, don't realize it. */
                   if (vals->nranges && j == vals->nranges)
                       continue;
               }

               rc = 0;
               area = (struct region *)fontfcnB(S, (unsigned char *)codename,
                                                &len, &rc);
               if (rc < 0) {
                       rc = Type1ReturnCodeToXReturnCode(rc);
                       break;
               }
               else if (rc > 0)
                       continue;
 
               if (area == NULL)
                       continue;
 
	       if (pFont->info.firstCol > i)
		   pFont->info.firstCol = i;
	       if (pFont->info.lastCol < i)
		   pFont->info.lastCol = i;

               h       = area->ymax - area->ymin;
               w       = area->xmax - area->xmin;
               paddedW = PAD(w, pad);
 
               if (h > 0 && w > 0) {
                       size = h * paddedW / 8;
                       glyphs[i].bits = (char *)xalloc(size);
                       if (glyphs[i].bits == NULL) {
                               rc = AllocError;
                               break;
                       }
               }
               else {
		       size = 0;
                       h = w = 0;
                       area->xmin = area->xmax = 0;
                       area->ymax = area->ymax = 0;
               }
 
               glyphs[i].metrics.leftSideBearing  = area->xmin;
	       x1 = (double)(x0 = area->ending.x - area->origin.x);
	       y1 = (double)(area->ending.y - area->origin.y);
               glyphs[i].metrics.characterWidth   =
		   (x0 + (x0 > 0 ? FPHALF : -FPHALF)) / (1 << FRACTBITS);
               if (!glyphs[i].metrics.characterWidth && size == 0)
	       {
		   /* Zero size and zero extents: presumably caused by
		      the choice of transformation.  Let's create a
		      small bitmap so we're not mistaken for an undefined
		      character. */
		   h = w = 1;
		   size = paddedW = PAD(w, pad);
		   glyphs[i].bits = (char *)xalloc(size);
                   if (glyphs[i].bits == NULL) {
                       rc = AllocError;
                       break;
                   }
	       }
               glyphs[i].metrics.attributes =
		   NEARESTPEL((long)(hypot(x1, y1) * sxmult));
	       total_width += glyphs[i].metrics.attributes;
	       total_raw_width += abs((int)(INT16)glyphs[i].metrics.attributes);
	       count++;
               glyphs[i].metrics.rightSideBearing = w + area->xmin;
               glyphs[i].metrics.descent          = area->ymax - NEARESTPEL(area->origin.y);
               glyphs[i].metrics.ascent           = h - glyphs[i].metrics.descent;
 
 
               bzero(glyphs[i].bits, size);
               if (h > 0 && w > 0) {
                   fill(glyphs[i].bits, h, paddedW, area, byte, bit, wordsize );
               }
 
               Destroy(area);
       }
 NoEncoding:
       
       delmemory();
       xfree(pool);
 
       if (pFont->info.firstCol > pFont->info.lastCol)
       {
               xfree(type1);
               DestroyFontRec(pFont);
               return BadFontName;
       }
 
       if (i != 256) {
               for (i--; i >= 0; i--)
                       if (glyphs[i].bits != NULL)
                               xfree(glyphs[i].bits);
               xfree(type1);
               DestroyFontRec(pFont);
               return rc;
       }
       type1->pDefault    = NULL;
 
       pFont->format      = format;
 
       pFont->bit         = bit;
       pFont->byte        = byte;
       pFont->glyph       = glyph;
       pFont->scan        = scan;
 
       pFont->info.firstRow = 0;
       pFont->info.lastRow  = 0;
 
       pFont->get_metrics = Type1GetMetrics;
       pFont->get_glyphs  = Type1GetGlyphs;
       pFont->unload_font = Type1CloseFont;
       pFont->unload_glyphs = NULL;
       pFont->refcnt = 0;
 
       pFont->fontPrivate = (unsigned char *) type1;

       if (count)
       {
	   total_raw_width = (total_raw_width * 10 + count / 2) / count;
	   if (total_width < 0)
	   {
	       /* Predominant direction is R->L */
	       total_raw_width = -total_raw_width;
	   }
	   vals->width = (int)((double)total_raw_width *
			       vals->pixel_matrix[0] / 1000.0 +
			       (vals->pixel_matrix[0] > 0 ? .5 : -.5));
       }

       T1FillFontInfo(pFont, vals, fileName, entry->name.name, total_raw_width);
 
       *ppFont = pFont;
       return Successful;
}

static int
Type1GetGlyphs(FontPtr pFont, 
	       unsigned long count, 
	       unsigned char *chars, 
	       FontEncoding charEncoding, 
	       unsigned long *glyphCount,  /* RETURN */
	       CharInfoPtr *glyphs)	   /* RETURN */
{
    unsigned int firstRow;
    unsigned int numRows;
    CharInfoPtr *glyphsBase;
    register unsigned int c;
    register CharInfoPtr pci;
    unsigned int r;
    CharInfoPtr pDefault;
    register struct type1font *type1Font;
    register int firstCol;
 
    type1Font  = (struct type1font *) pFont->fontPrivate;
    firstCol   = pFont->info.firstCol;
    pDefault   = type1Font->pDefault;
    glyphsBase = glyphs;
 
    switch (charEncoding) {

#define EXIST(pci) \
    ((pci)->metrics.attributes || \
     (pci)->metrics.ascent != -(pci)->metrics.descent || \
     (pci)->metrics.leftSideBearing != (pci)->metrics.rightSideBearing)
 
    case Linear8Bit:
    case TwoD8Bit:
        if (pFont->info.firstRow > 0)
            break;
        while (count--) {
                c = (*chars++);
                if (c >= firstCol &&
                       (pci = &type1Font->glyphs[c]) &&
		       EXIST(pci))
                    *glyphs++ = pci;
                else if (pDefault)
                    *glyphs++ = pDefault;
        }
        break;
    case Linear16Bit:
        while (count--) {
                c = *chars++ << 8;
                c = (c | *chars++);
                if (c < 256 && c >= firstCol &&
                        (pci = &type1Font->glyphs[c]) &&
			EXIST(pci))
                    *glyphs++ = pci;
                else if (pDefault)
                    *glyphs++ = pDefault;
        }
        break;
 
    case TwoD16Bit:
        firstRow = pFont->info.firstRow;
        numRows = pFont->info.lastRow - firstRow + 1;
        while (count--) {
            r = (*chars++) - firstRow;
            c = (*chars++);
            if (r < numRows && c < 256 && c >= firstCol &&
                    (pci = &type1Font->glyphs[(r << 8) + c]) &&
		    EXIST(pci))
                *glyphs++ = pci;
            else if (pDefault)
                *glyphs++ = pDefault;
        }
        break;
    }
    *glyphCount = glyphs - glyphsBase;
    return Successful;

#undef EXIST
}

static int
Type1GetMetrics(FontPtr pFont, 
		unsigned long count, 
		unsigned char *chars, 
		FontEncoding charEncoding, 
		unsigned long *glyphCount, /* RETURN */
		xCharInfo **glyphs) 	   /* RETURN */
{
    static CharInfoRec nonExistantChar;
 
    int         ret;
    struct type1font *type1Font;
    CharInfoPtr oldDefault;
 
    type1Font = (struct type1font *) pFont->fontPrivate;
    oldDefault = type1Font->pDefault;
    type1Font->pDefault = &nonExistantChar;
    ret = Type1GetGlyphs(pFont, count, chars, charEncoding, glyphCount, (CharInfoPtr *) glyphs);
    type1Font->pDefault = oldDefault;
    return ret;
}

void 
Type1CloseFont(FontPtr pFont)
{
       register int i;
       struct type1font *type1;
 
       type1 = (struct type1font *) pFont->fontPrivate;
       for (i=0; i < 256; i++)
               if (type1->glyphs[i].bits != NULL)
                        xfree(type1->glyphs[i].bits);
       xfree(type1);

       if (pFont->info.props)
	   xfree(pFont->info.props);

       if (pFont->info.isStringProp)
	   xfree(pFont->info.isStringProp);

       DestroyFontRec(pFont);
}

static void 
fill(char *dest,             /* destination bitmap                           */
     int h, int w,           /* dimensions of 'dest', w padded               */
     struct region *area,    /* region to write to 'dest'                    */
     int byte, int bit,      /* flags; LSBFirst or MSBFirst                  */
     int wordsize)           /* number of bits per word for LSB/MSB purposes */
{
       register struct edgelist *edge;  /* for looping through edges         */
       register char *p;     /* current scan line in 'dest'                  */
       register int y;       /* for looping through scans                    */
       register int wbytes = w / 8;  /* number of bytes in width             */
       register pel *leftP,*rightP;  /* pointers to X values, left and right */
       int xmin = area->xmin;  /* upper left X                               */
       int ymin = area->ymin;  /* upper left Y                               */
 
       for (edge = area->anchor; VALIDEDGE(edge); edge = edge->link->link) {
 
               p = dest + (edge->ymin - ymin) * wbytes;
               leftP = edge->xvalues;
               rightP = edge->link->xvalues;
 
               for (y = edge->ymin; y < edge->ymax; y++) {
                       fillrun(p, *leftP++ - xmin, *rightP++ - xmin, bit);
                       p += wbytes;
               }
       }
/*
Now, as an afterthought, we'll go reorganize if odd byte order requires
it:
*/
       if (byte == LSBFirst && wordsize != 8) {
               register int i;
 
               switch (wordsize) {
                   case 16:
                   {
                       register unsigned short data,*p;
 
                       p = (unsigned short *) dest;
 
                       for (i = h * w /16; --i >= 0;) {
                               data = *p;
                               *p++ = (data << 8) + (data >> 8);
                       }
                       break;
                   }
                   case 64:
                   case 32:
                   {
                       register unsigned long data,*p;
 
                       p = (unsigned long *) dest;
 
                       for (i = h * w / 32; --i >= 0;) {
                               data = *p;
                               *p++ = (data << 24) + (data >> 24)
                                      + (0xFF00 & (data >> 8))
                                      + (0xFF0000 & (data << 8));
                       }
                       if (wordsize == 64) {
 
                               p = (unsigned long *) dest;
 
                               for (i = h * w / 64; --i >= 0;) {
                                       data = *p++;
                                       p[-1] = p[0];
                                       *p++ = data;
                               }
                       }
                       break;
                   }
                   default:
                       Abort("xiFill: unknown format");
               }
       }
 
}
 
#define  ALLONES  0xFF
 
static void 
fillrun(char *p,             /* address of this scan line                    */
	pel x0, pel x1,      /* left and right X                             */
	int bit)             /* format:  LSBFirst or MSBFirst                */
{
       register int startmask,endmask;  /* bits to set in first and last char*/
       register int middle;  /* number of chars between start and end + 1    */
 
       if (x1 <= x0)
               return;
       middle = x1/8 - x0/8;
       p += x0/8;
       x0 &= 7;  x1 &= 7;
       if (bit == LSBFirst) {
               startmask = ALLONES << x0;
               endmask = ~(ALLONES << x1);
       }
       else {
               startmask = ALLONES >> x0;
               endmask = ~(ALLONES >> x1);
       }
       if (middle == 0)
               *p++ |= startmask & endmask;
       else {
               *p++ |= startmask;
               while (--middle > 0)
                       *p++ = (char)ALLONES;
               *p |= endmask;
       }
}
 
#define CAPABILITIES (CAP_MATRIX | CAP_CHARSUBSETTING)

static FontRendererRec renderers[] = {
  { ".pfa", 4, NULL, Type1OpenScalable,
        NULL, Type1GetInfoScalable, 0, CAPABILITIES },
  { ".pfb", 4, NULL, Type1OpenScalable,
        NULL, Type1GetInfoScalable, 0, CAPABILITIES }
};

void
Type1RegisterFontFileFunctions(void)
{
    int i;
 
    T1InitStdProps();
    for (i=0; i < sizeof(renderers) / sizeof(FontRendererRec); i++)
            FontFilePriorityRegisterRenderer(&renderers[i], -10);
}
