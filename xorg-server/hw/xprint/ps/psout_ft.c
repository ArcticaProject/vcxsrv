
/*
Copyright (c) 2003-2004 Roland Mainz <roland.mainz@nrubsig.org>

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
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "os.h"
#define USE_PSOUT_PRIVATE 1
#include "psout.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H

#include <X11/Xproto.h>
#include <X11/fonts/font.h>
#include <X11/fonts/fontstruct.h>
#include <X11/fonts/fntfilst.h>
#include <X11/fonts/fontutil.h>
#include <X11/fonts/fontenc.h>
#include <X11/fonts/ft.h>
#define NOT_IN_FTFUNCS
#include <X11/fonts/ftfuncs.h>
#include "servermd.h" /* needed for endian test (IMAGE_BYTE_ORDER) */

#define USE_FT_PS_NAMES 1

#ifdef USE_FT_PS_NAMES
void PsOut_Get_FreeType_Glyph_Name( char *destbuf, FontPtr pFont, unsigned long x11fontindex)
{
  FTFontPtr      tf     = (FTFontPtr)pFont->fontPrivate;
  FT_Face        ttface = tf->instance->face->face;
  FT_Error       error;
  char           buf[256];
  unsigned long  ftindex;

  /* Remap X11 font index to FreeType font index */
  ftindex = FTRemap(ttface, &tf->mapping, x11fontindex);

  if( FT_Has_PS_Glyph_Names(ttface) )
  {
    error = FT_Get_Glyph_Name(ttface, ftindex, buf, 64);
  }
  else
  {
    error = 1;
  }
  
  if( error )
  {
    /* Check for unicode mapping
     * See Adobe document "Unicode and Glyph Names"
     * (http://partners.adobe.com/asn/tech/type/unicodegn.jsp)
     */
    if( (tf->mapping.mapping->type == FONT_ENCODING_UNICODE) &&
        (ftindex < 0xFFFE) )
    {
      sprintf(buf, "uni%04lx", ftindex);
    }
    else
    {
      sprintf(buf, "ch%02lx", ftindex);
    }
  }
  
  strcpy(destbuf, buf);
}
#endif /* USE_FT_PS_NAMES */

int PsOut_DownloadFreeType(PsOutPtr self, PsFTDownloadFontType downloadfonttype, const char *psfontname, FontPtr pFont, long block_offset)
{
  switch(downloadfonttype)
  {
    case PsFontType3:
        return PsOut_DownloadFreeType3(self, psfontname, pFont, block_offset);
    case PsFontType1:
        return PsOut_DownloadFreeType1(self, psfontname, pFont, block_offset);
    default:
        FatalError("PS DDX: PsOut_DownloadFreeType(downloadfonttype='%d' not implemented\n",
                   (int)downloadfonttype);
        return 0; /* NO-OP, FatalError() will call |exit()| */
  }
}

/* cloned from |PsOut_TextAttrs16| */
void
PsOut_FreeType_TextAttrs16(PsOutPtr self, char *fnam, int siz, int iso)
{
  int i;
  if( self->FontName && strcmp(fnam, self->FontName)==0 &&
      siz==self->FontSize ) return;
  if( self->FontName ) xfree(self->FontName);
  self->FontName = (char *)xalloc(strlen(fnam)+1);
  strcpy(self->FontName, fnam);
  self->FontSize = siz;
  for( i=0 ; i<4 ; i++ ) self->FontMtx[i] = -1.;
}

/* cloned from |PsOut_TextAttrsMtx16| */
void
PsOut_FreeType_TextAttrsMtx16(PsOutPtr self, char *fnam, float *mtx, int iso)
{
  int i;
  if( self->FontName && strcmp(fnam, self->FontName)==0 &&
      mtx[0]==self->FontMtx[0] && mtx[1]==self->FontMtx[1] &&
      mtx[2]==self->FontMtx[2] && mtx[3]==self->FontMtx[3] ) return;
  if( self->FontName ) xfree(self->FontName);
  self->FontName = (char *)xalloc(strlen(fnam)+1);
  strcpy(self->FontName, fnam);
  for( i=0 ; i<4 ; i++ ) self->FontMtx[i] = mtx[i];
  self->FontSize = -1;
}

static
int FT_Get_CharcellMetricsCharacterHeight(FontPtr pFont)
{
  FTFontPtr ftfont = (FTFontPtr)pFont->fontPrivate;

  return ftfont->instance->charcellMetrics->ascent + 
         ftfont->instance->charcellMetrics->descent;
}

static
int FT_Get_CharcellMetricsCharacterWidth(FontPtr pFont)
{
  FTFontPtr ftfont = (FTFontPtr)pFont->fontPrivate;

  if( ftfont->instance->spacing != FT_PROPORTIONAL )
  {
    int width = ftfont->instance->charcellMetrics->characterWidth;
    
    /* If the font uses a matrix make sure we transform the |characterWidth|
     * back to it's original value since we download the untransformed font
     * and use a PostScript transformation matrix to transform the font when
     * rendering the text
     */
    if( ftfont->instance->transformation.nonIdentity )
    {
      FT_Vector v;
    
      FT_Matrix m = ftfont->instance->transformation.matrix;
      (void)FT_Matrix_Invert(&m); /* FixMe: We should check the return code */
      v.x = width;
      v.y = FT_Get_CharcellMetricsCharacterHeight(pFont);
      FT_Vector_Transform(&v, &m);
      width = v.x;
    }
    
    return width;
  }

  return 0;
}

void
PsOut_FreeType_Text(FontPtr pFont, PsOutPtr self, int x, int y, char *text, int textl)
{
  int i;
  int xo = self->XOff,
      yo = self->YOff;
  char buf[256];
  int  cwidth = FT_Get_CharcellMetricsCharacterWidth(pFont);

  if( self->InFrame || self->InTile ) xo = yo = 0;
  x += xo; y += yo;

  S_OutNum(self, (float)x);
  S_OutNum(self, (float)y);
  S_OutTok(self, "moveto", 1);
  
  S_OutTok(self, "[ ", 0);
  
  for( i = 0 ; i < textl ; i++ )
  {
#ifdef USE_FT_PS_NAMES
    char namebuf[256];
    unsigned int  ch           = text[i]&0xFF;
    unsigned long block_offset = 0;
    PsOut_Get_FreeType_Glyph_Name(namebuf, pFont, ch+block_offset);

    sprintf(buf, "/%s ", namebuf);
#else
    sprintf(buf, "/ch%02x ", text[i]&0xFF);
#endif /* USE_FT_PS_NAMES */
    S_OutTok(self, buf, 0);
  }

  /* Check whether we have any special spacing requirements (e.g. non-proportional fonts) ... */
  if( cwidth != 0 )
  {
    /* If the we use a matrix to render the font (instead of using |self->FontSize|)
     * we must apply the matrix to the "rmoveto" which is used to force the exact
     * character width. The "trmoveto" macro will do that for us...
     */
    if( self->FontSize == -1 )
    {  
      sprintf(buf, "]{gs glyphshow gr %d 0 trmoveto}fa",  cwidth);
    }
    else
    {
      sprintf(buf, "]{gs glyphshow gr %d 0 rm}fa", cwidth);
    }
  }
  else
  {
    sprintf(buf, "]{glyphshow}fa");
  }
  S_OutTok(self, buf, 0);
}

/* XXX: |PsOut_FreeType_Text16| should be rewritten - currently it uses lame,
 * slow hacks and makes some risky assumtions about how |PsOut_Text16|
 * allocates memory */
void
PsOut_FreeType_Text16(FontPtr pFont, PsOutPtr self, int x, int y, unsigned short *text, int textl)
{
  int i;
  int xo = self->XOff,
      yo = self->YOff;
  unsigned short c,
                 c_hiByte,
                 c_lowByte,
                 fontPage;
  long           lastFontPage = -1;
  char           baseFontName[256];
  char           buf[256];

  if( self->InFrame || self->InTile ) xo = yo = 0;
  x += xo; y += yo;

  strcpy(baseFontName, self->FontName);

  S_OutNum(self, (float)x);
  S_OutNum(self, (float)y);
  S_OutTok(self, "moveto", 1);
  
  for( i = 0 ; i < textl ; i++ )
  {
    c = text[i];
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

    if( fontPage != lastFontPage )
    {
      if( fontPage > 0 )
      {
        sprintf(buf, "%s_%x", baseFontName, fontPage);
      }
      else
      {
        sprintf(buf, "%s", baseFontName);
        xfree(self->FontName);
        self->FontName = NULL;
      }

      if( self->FontSize == -1 )
      {
        PsOut_TextAttrsMtx(self, buf, self->FontMtx, FALSE);
      }
      else
      {
        PsOut_TextAttrs(self, buf, self->FontSize, FALSE);
      }
      lastFontPage = fontPage;
    }

#ifdef USE_FT_PS_NAMES
    {
      char namebuf[256];
      unsigned int  ch           = c_lowByte;
      unsigned long block_offset = c_hiByte * 0x100 /* same as c_hiByte << 8 */;
      int           cwidth       = FT_Get_CharcellMetricsCharacterWidth(pFont);
      PsOut_Get_FreeType_Glyph_Name(namebuf, pFont, ch+block_offset);
      
      /* Check whether we have any special spacing requirements (e.g. non-proportional fonts) ... */
      if( cwidth != 0 )
      {
        /* If the we use a matrix to render the font (instead of using |self->FontSize|)
         * we must apply the matrix to the "rmoveto" which is used to force the exact
         * character width. The "trmoveto" macro will do that for us...
         */
        if( self->FontSize == -1 )
        {
          sprintf(buf, "gs /%s glyphshow gr %d 0 trmoveto", namebuf, cwidth);
        }
        else
        {
          sprintf(buf, "gs /%s glyphshow gr %d 0 rm",  namebuf, cwidth);
        }
      }
      else
      {
        sprintf(buf, "/%s glyphshow", namebuf);
      }
    }
#else
    sprintf(buf, "/ch%02x glyphshow", c_lowByte);
#endif /* USE_FT_PS_NAMES */    
    S_OutTok(self, buf, 1); 
  }

  if( self->FontName ) xfree(self->FontName);
  self->FontName = (char *)xalloc(strlen(baseFontName)+1);
  strcpy(self->FontName, baseFontName);
}

