/*

Copyright 1996, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/*
 * (c) Copyright 1996 Hewlett-Packard Company
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc.
 * (c) Copyright 1996 Digital Equipment Corp.
 * (c) Copyright 1996 Fujitsu Limited
 * (c) Copyright 1996 Hitachi, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the names of the copyright holders
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from said copyright holders.
 */

/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PsText.c
**    *
**    *  Contents:	Character-drawing routines for the PS DDX
**    *
**    *  Created By:	Roger Helmendach (Liberty Systems)
**    *
**    *  Copyright:	Copyright 1996 The Open Group, Inc.
**    *
**    *********************************************************
** 
********************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "Ps.h"
#include "gcstruct.h"
#include "windowstr.h"
#include <X11/fonts/fntfil.h>
#include <X11/fonts/fntfilst.h>
#include <limits.h>

int
PsPolyText8(
  DrawablePtr pDrawable,
  GCPtr       pGC,
  int         x,
  int         y,
  int         count,
  char       *string)
{
  if( pDrawable->type==DRAWABLE_PIXMAP )
  {
    DisplayElmPtr   elm;
    PixmapPtr       pix  = (PixmapPtr)pDrawable;
    PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
    DisplayListPtr  disp;
    GCPtr           gc;

    if ((gc = PsCreateAndCopyGC(pDrawable, pGC)) == NULL) return x;

    disp = PsGetFreeDisplayBlock(priv);

    elm  = &disp->elms[disp->nelms];
    elm->type = Text8Cmd;
    elm->gc   = gc;
    elm->c.text8.x      = x;
    elm->c.text8.y      = y;
    elm->c.text8.count  = count;
    elm->c.text8.string = (char *)xalloc(count);
    memcpy(elm->c.text8.string, string, count);
    disp->nelms += 1;
    
    return x;
  }
  else
  {
    PsFontInfoRec *firec;

    /* We need a context for rendering... */
    if (PsGetPsContextPriv(pDrawable) == NULL)
      return x;

    firec = PsGetFontInfoRec(pDrawable, pGC->font);
    if (!firec)
        return x;

#ifdef XP_USE_FREETYPE    
    if (firec->ftir->downloadableFont && 
        (firec->ftir->font_type == PSFTI_FONT_TYPE_FREETYPE))
    {
        PsOutPtr       psOut;
        ColormapPtr    cMap;
        
	if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) 
	    return x; 

        if (firec->ftir->alreadyDownloaded[0] == False)
        {
            PsOut_DownloadFreeType(psOut,
                                   firec->ftir->ft_download_font_type,
                                   firec->ftir->download_ps_name, pGC->font, 0);              
            firec->ftir->alreadyDownloaded[0] = True;
        }

        PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
       	PsOut_Color(psOut, PsGetPixelColor(cMap, pGC->fgPixel)); 
       	if (!firec->size)
            PsOut_TextAttrsMtx(psOut, firec->ftir->download_ps_name, firec->mtx, firec->ftir->is_iso_encoding); 
        else
            PsOut_TextAttrs(psOut, firec->ftir->download_ps_name, firec->size, firec->ftir->is_iso_encoding); 
        PsOut_FreeType_Text(pGC->font, psOut, x, y, string, count);

	return x;	
    }
    else
#endif /* XP_USE_FREETYPE */
         if (firec->ftir->downloadableFont && 
             (firec->ftir->font_type != PSFTI_FONT_TYPE_FREETYPE))
    {
        PsOutPtr       psOut;
        ColormapPtr    cMap;
        
	if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) 
	    return x; 

        if (firec->ftir->alreadyDownloaded[0] == False)
        {
            PsOut_DownloadType1(psOut, "PsPolyText8",
                                firec->ftir->download_ps_name, firec->ftir->filename);
            firec->ftir->alreadyDownloaded[0] = True;
        }

        PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
       	PsOut_Color(psOut, PsGetPixelColor(cMap, pGC->fgPixel)); 
       	if (!firec->size)
            PsOut_TextAttrsMtx(psOut, firec->ftir->download_ps_name, firec->mtx, firec->ftir->is_iso_encoding); 
        else
            PsOut_TextAttrs(psOut, firec->ftir->download_ps_name, firec->size, firec->ftir->is_iso_encoding); 
        PsOut_Text(psOut, x, y, string, count, -1);

	return x;	
    }    
    
    /* Render glyphs as bitmaps */
    {
        unsigned long n, i;
        int w;
        CharInfoPtr charinfo[255];  

        GetGlyphs(pGC->font, (unsigned long)count, 
            (unsigned char *)string, Linear8Bit, &n, charinfo);
        w = 0;
        for (i=0; i < n; i++)
          w += charinfo[i]->metrics.characterWidth;

        if (n != 0)
            PsPolyGlyphBlt(pDrawable, pGC, x, y, n, 
                    charinfo, FONTGLYPHS(pGC->font));
        x += w;
        
        return x;
    }  
  }
  return x;
}

int
PsPolyText16(
  DrawablePtr     pDrawable,
  GCPtr           pGC,
  int             x,
  int             y,
  int             count,
  unsigned short *string)
{
  if( pDrawable->type==DRAWABLE_PIXMAP )
  {
    DisplayElmPtr   elm;
    PixmapPtr       pix  = (PixmapPtr)pDrawable;
    PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
    DisplayListPtr  disp;
    GCPtr           gc;

    if ((gc = PsCreateAndCopyGC(pDrawable, pGC)) == NULL) return x;

    disp = PsGetFreeDisplayBlock(priv);

    elm  = &disp->elms[disp->nelms];
    elm->type = Text16Cmd;
    elm->gc   = gc;
    elm->c.text16.x      = x;
    elm->c.text16.y      = y;
    elm->c.text16.count  = count;
    elm->c.text16.string =
      (unsigned short *)xalloc(count*sizeof(unsigned short));
    memcpy(elm->c.text16.string, string, count*sizeof(unsigned short));
    disp->nelms += 1;

    return x;
  }
  else
  {
    PsFontInfoRec *firec;

    /* We need a context for rendering... */
    if (PsGetPsContextPriv(pDrawable) == NULL)
      return x;

    firec = PsGetFontInfoRec(pDrawable, pGC->font);
    if (!firec)
        return x;

#ifdef XP_USE_FREETYPE    
    if (firec->ftir->downloadableFont &&
        (firec->ftir->font_type == PSFTI_FONT_TYPE_FREETYPE))
    {
        PsOutPtr       psOut;
        ColormapPtr    cMap;
        unsigned short c,
                       c_hiByte,
                       c_lowByte,
                       fontPage;
        int            i;
        
	if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) 
	    return x; 

        /* Scan the string we want to render and download all neccesary parts
         * of the font (one part(="font page") has 256 glyphs)
         */
        for( i = 0 ; i < count ; i++ )
        {
            c = string[i];
#if IMAGE_BYTE_ORDER == LSBFirst
            c_hiByte = c & 0x00FF;
            c_lowByte = (c >> 8) & 0x00FF;
#elif IMAGE_BYTE_ORDER == MSBFirst
            c_hiByte  = (c >> 8) & 0x00FF;
            c_lowByte = c & 0x00FF;
#else
#error Unsupported byte order
#endif
            fontPage  = c_hiByte;
          
            if (firec->ftir->alreadyDownloaded[fontPage] == False)
            {
                char        buffer[256];
                const char *ps_name;

                if (fontPage > 0)
                {
                    sprintf(buffer, "%s_%x", firec->ftir->download_ps_name, (int)fontPage);
                    ps_name = buffer;
                }
                else
                {
                    ps_name = firec->ftir->download_ps_name;
                }

                PsOut_DownloadFreeType(psOut,
                                       firec->ftir->ft_download_font_type,
                                       ps_name, pGC->font, (fontPage * 0x100)); /* same as (fontPage << 8) */    
                                 
                firec->ftir->alreadyDownloaded[fontPage] = True;
            }
        }


        PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
       	PsOut_Color(psOut, PsGetPixelColor(cMap, pGC->fgPixel)); 
       	if (!firec->size)
            PsOut_FreeType_TextAttrsMtx16(psOut, firec->ftir->download_ps_name, firec->mtx, firec->ftir->is_iso_encoding); 
        else
            PsOut_FreeType_TextAttrs16(psOut, firec->ftir->download_ps_name, firec->size, firec->ftir->is_iso_encoding); 
        PsOut_FreeType_Text16(pGC->font, psOut, x, y, string, count);
        
	return x;	
    }
    else
#endif /* XP_USE_FREETYPE */
         if (firec->ftir->downloadableFont &&
             (firec->ftir->font_type != PSFTI_FONT_TYPE_FREETYPE))
    {
        PsOutPtr       psOut;
        ColormapPtr    cMap;
        unsigned short fontPage;
        
	if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) 
	    return x; 

        PsOut_DownloadType1(psOut, "PsPolyText16",
                            firec->ftir->download_ps_name, firec->ftir->filename);
        firec->ftir->alreadyDownloaded[fontPage] = True;

        PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
       	PsOut_Color(psOut, PsGetPixelColor(cMap, pGC->fgPixel)); 
       	if (!firec->size)
            PsOut_TextAttrsMtx(psOut, firec->ftir->download_ps_name, firec->mtx, firec->ftir->is_iso_encoding); 
        else
            PsOut_TextAttrs(psOut, firec->ftir->download_ps_name, firec->size, firec->ftir->is_iso_encoding); 
        PsOut_Text16(psOut, x, y, string, count, -1);
        
	return x;	
    }
    
    /* Render glyphs as bitmaps */
    {
        unsigned long n, i;
        int w;
        CharInfoPtr charinfo[255];  /* encoding only has 1 byte for count */

        GetGlyphs(pGC->font, (unsigned long)count, (unsigned char *)string,
                  (FONTLASTROW(pGC->font) == 0) ? Linear16Bit : TwoD16Bit,
                  &n, charinfo);
        w = 0;
        for (i=0; i < n; i++)
          w += charinfo[i]->metrics.characterWidth;
        if (n != 0)
	    PsPolyGlyphBlt(pDrawable, pGC, x, y, n, charinfo, FONTGLYPHS(pGC->font));
        x += w;
        
        return x;
    }  
  }
  return x;
}

void
PsImageText8(
  DrawablePtr pDrawable,
  GCPtr       pGC,
  int         x,
  int         y,
  int         count,
  char       *string)
{
  if( pDrawable->type==DRAWABLE_PIXMAP )
  {
    DisplayElmPtr   elm;
    PixmapPtr       pix  = (PixmapPtr)pDrawable;
    PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
    DisplayListPtr  disp;
    GCPtr           gc;

    if ((gc = PsCreateAndCopyGC(pDrawable, pGC)) == NULL) return;

    disp = PsGetFreeDisplayBlock(priv);

    elm  = &disp->elms[disp->nelms];
    elm->type = TextI8Cmd;
    elm->gc   = gc;
    elm->c.text8.x      = x;
    elm->c.text8.y      = y;
    elm->c.text8.count  = count;
    elm->c.text8.string = (char *)xalloc(count);
    memcpy(elm->c.text8.string, string, count);
    disp->nelms += 1;
  }
  else
  {
    int          iso;
    int          siz;
    float        mtx[4];
    char        *fnam;
    PsOutPtr     psOut;
    ColormapPtr  cMap;

    if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) return;
    PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
    PsOut_Color(psOut, PsGetPixelColor(cMap, pGC->fgPixel));
    fnam = PsGetPSFontName(pGC->font);
    if( !fnam ) fnam = "Times-Roman";
    siz = PsGetFontSize(pGC->font, mtx);
    iso = PsIsISOLatin1Encoding(pGC->font);
    if( !siz ) PsOut_TextAttrsMtx(psOut, fnam, mtx, iso);
    else       PsOut_TextAttrs(psOut, fnam, siz, iso);
    PsOut_Text(psOut, x, y, string, count, PsGetPixelColor(cMap, pGC->bgPixel));
  }
}

void
PsImageText16(
  DrawablePtr     pDrawable,
  GCPtr           pGC,
  int             x,
  int             y,
  int             count,
  unsigned short *string)
{
  if( pDrawable->type==DRAWABLE_PIXMAP )
  {
    DisplayElmPtr   elm;
    PixmapPtr       pix  = (PixmapPtr)pDrawable;
    PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
    DisplayListPtr  disp;
    GCPtr           gc;

    if ((gc = PsCreateAndCopyGC(pDrawable, pGC)) == NULL) return;

    disp = PsGetFreeDisplayBlock(priv);

    elm  = &disp->elms[disp->nelms];
    elm->type = TextI16Cmd;
    elm->gc   = gc;
    elm->c.text16.x      = x;
    elm->c.text16.y      = y;
    elm->c.text16.count  = count;
    elm->c.text16.string =
      (unsigned short *)xalloc(count*sizeof(unsigned short));
    memcpy(elm->c.text16.string, string, count*sizeof(unsigned short));
    disp->nelms += 1;
  }
  else
  {
    int   i;
    char *str;
    if( !count ) return;
    str = (char *)xalloc(count);
    for( i=0 ; i<count ; i++ ) str[i] = string[i];
    PsImageText8(pDrawable, pGC, x, y, count, str);
    free(str);
  }
}

void
PsImageGlyphBlt(
  DrawablePtr   pDrawable,
  GCPtr         pGC,
  int           x,
  int           y,
  unsigned int  nGlyphs,
  CharInfoPtr  *pCharInfo,
  pointer       pGlyphBase)
{
  /* NOT TO BE IMPLEMENTED */
}

void
PsPolyGlyphBlt(
  DrawablePtr   pDrawable,
  GCPtr         pGC,
  int           x,
  int           y,
  unsigned int  nGlyphs,
  CharInfoPtr  *pCharInfo,
  pointer       pGlyphBase)
{
    int width, height;
    PixmapPtr pPixmap;
    int nbyLine;                        /* bytes per line of padded pixmap */
    FontPtr pfont;
    GCPtr pGCtmp;
    register int i;
    register int j;
    unsigned char *pbits;               /* buffer for PutImage */
    register unsigned char *pb;         /* temp pointer into buffer */
    register CharInfoPtr pci;           /* currect char info */
    register unsigned char *pglyph;     /* pointer bits in glyph */
    int gWidth, gHeight;                /* width and height of glyph */
    register int nbyGlyphWidth;         /* bytes per scanline of glyph */
    int nbyPadGlyph;                    /* server padded line of glyph */
    int w, tmpx;
    XID gcvals[3];

    pfont = pGC->font;
    width = FONTMAXBOUNDS(pfont,rightSideBearing) -
            FONTMINBOUNDS(pfont,leftSideBearing);
    height = FONTMAXBOUNDS(pfont,ascent) +
             FONTMAXBOUNDS(pfont,descent);

    if ((width == 0) || (height == 0) )
        return;
    {
        int i;
        w = 0;
        for (i=0; i < nGlyphs; i++) w += pCharInfo[i]->metrics.characterWidth;
    }
    pGCtmp = GetScratchGC(1, pDrawable->pScreen);
    if (!pGCtmp)
    {
        (*pDrawable->pScreen->DestroyPixmap)(pPixmap);
        return;
    }

    gcvals[0] = GXcopy;
    gcvals[1] = pGC->fgPixel;
    gcvals[2] = pGC->bgPixel; 

    DoChangeGC(pGCtmp, GCFunction|GCForeground|GCBackground, gcvals, 0);

    
    nbyLine = BitmapBytePad(width);
    pbits = (unsigned char *)xalloc(height*nbyLine);
    if (!pbits){
        PsDestroyPixmap(pPixmap);
        return;
    }
    tmpx = 0;
    while(nGlyphs--)
    {
        pci = *pCharInfo++;
        pglyph = FONTGLYPHBITS(pGlyphBase, pci);
        gWidth = GLYPHWIDTHPIXELS(pci);
        gHeight = GLYPHHEIGHTPIXELS(pci);
        if (gWidth && gHeight)
        {
            nbyGlyphWidth = GLYPHWIDTHBYTESPADDED(pci);
            nbyPadGlyph = BitmapBytePad(gWidth);

            if (nbyGlyphWidth == nbyPadGlyph
#if GLYPHPADBYTES != 4
                && (((int) pglyph) & 3) == 0
#endif
                )
            {
                pb = pglyph;
            }
            else
            {
                for (i=0, pb = pbits; i<gHeight; i++, pb = pbits+(i*nbyPadGlyph))
                    for (j = 0; j < nbyGlyphWidth; j++)
                        *pb++ = *pglyph++;
                pb = pbits;
            }

	    PsPutImageMask((DrawablePtr)pDrawable, pGCtmp, 
		   1, x + pci->metrics.leftSideBearing, 
		   y - pci->metrics.ascent, gWidth, gHeight,
                   0, XYBitmap, (char *)pb);
	}
        
        x  += pci->metrics.characterWidth;
    }
    xfree(pbits);
    FreeScratchGC(pGCtmp);
}
