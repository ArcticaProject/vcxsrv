
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
#include FT_TRUETYPE_TABLES_H
#include FT_BBOX_H
#include FT_GLYPH_H

#include FT_CONFIG_CONFIG_H
#include FT_CONFIG_OPTIONS_H
#include FT_ERRORS_H
#include FT_SYSTEM_H
#include FT_IMAGE_H
#include FT_TYPES_H
#include FT_OUTLINE_H
#include FT_MODULE_H
#include FT_RENDER_H
#include FT_TYPE1_TABLES_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TAGS_H
#include FT_CACHE_H
#include FT_CACHE_IMAGE_H
#include FT_CACHE_SMALL_BITMAPS_H
#include FT_MULTIPLE_MASTERS_H
#include FT_SFNT_NAMES_H

#include <X11/Xproto.h>
#include <X11/fonts/font.h>
#include <X11/fonts/fontstruct.h>
#include <X11/fonts/fntfilst.h>
#include <X11/fonts/fontutil.h>
#include <X11/fonts/fontenc.h>
#include <X11/fonts/ft.h>
#define NOT_IN_FTFUNCS
#include <X11/fonts/ftfuncs.h>

struct ft2info
{
  FontPtr         pFont;
  FTFontPtr       tf;
  FT_Face         ttface;
  struct
  {
    char *full_name;
    char *copyright;
    char *family;
    char *subfamily;
    char *version;
  } nameid;
  TT_Postscript  *ttpostscript;
  TT_Header      *ttheader;
};

/* Local prototypes */
static FT_Error PSType3_createOutlineGlyphs(FILE *out, struct ft2info *ti, unsigned long unicode, const char *psglyphname);
static int      PSType3_generateOutlineFont(FILE *out, const char *psfontname, struct ft2info *ti, long block_offset);

extern FT_Library ftypeLibrary; /* defined in xc/lib/font/FreeType/ftfuncs.c */

#define USE_FT_PS_NAMES 1

static
FT_Error PSType3_createOutlineGlyphs( FILE *out, struct ft2info *ti, unsigned long x11fontindex, const char *psglyphname )
{
  unsigned long  ftindex;
  FT_BBox        bbox;
  FT_Error       error;
  FT_Outline     outline;

  /* Remap X11 font index to FreeType font index */
  ftindex = FTRemap(ti->ttface, &ti->tf->mapping, x11fontindex);

  error = FT_Load_Glyph(ti->ttface, ftindex, (FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING));
  if( error )
  {
    fprintf(stderr, "PSType3_createOutlineGlyphs: FT_Load_Glyph() failure, error=%d\n", (int)error);
    return error;
  }

  outline = ti->ttface->glyph->outline;

  FT_Outline_Get_CBox(&outline, &bbox);

  fprintf(out, "/%s {\n", psglyphname);
  fprintf(out, "%ld 0 %ld %ld %ld %ld setcachedevice\n",
          (signed long)ti->ttface->glyph->metrics.horiAdvance,
          (long)bbox.xMin,
          (long)bbox.yMin,
          (long)bbox.xMax,
          (long)bbox.yMax);

  if( outline.n_contours > 0 )
  {
    long            i,
                    j,
                    k, k1,
                    cs, ce,
                    nguide,
                    contour_start,
                    contour_end,
                    last_point;
    Bool            first;
    FT_Vector      *vec;

    contour_start = ce = 0;

    vec        = outline.points;
    last_point = outline.n_points;

    i = j = k = 0;
    first = TRUE;

    while( i <= outline.contours[outline.n_contours - 1] )
    {
      contour_end = outline.contours[j];

      if( first )
      {
        fprintf(out, "%ld %ld moveto\n", vec[i].x, vec[i].y);
        contour_start = i;
        first = FALSE;
      }
      else if( outline.tags[i] & FT_CURVE_TAG_ON )
      {
        fprintf(out, "%ld %ld lineto\n", vec[i].x, vec[i].y);
      }
      else
      {
        Bool finished = FALSE;

        cs       = i-1;
        nguide   = 0;
        while( !finished )
        {
          if( i == contour_end+1 )
          {
            ce = contour_start;
            finished = TRUE;
          }
          else if( outline.tags[i] & FT_CURVE_TAG_ON )
          {
            ce = i;
            finished = TRUE;
          }
          else
          {
            i++;
            nguide++;
          }
        }

        switch( nguide )
        {
          case 0: 
              fprintf(out, "%ld %ld lineto\n", vec[ce].x, vec[ce].y);
              break;

          case 1:
              fprintf(out, "%ld %ld %ld %ld %ld %ld curveto\n",
                      (vec[cs].x+2*vec[cs+1].x)/3,
                      (vec[cs].y+2*vec[cs+1].y)/3,
                      (2*vec[cs+1].x+vec[ce].x)/3,
                      (2*vec[cs+1].y+vec[ce].y)/3,
                      vec[ce].x, vec[ce].y);
              break;

          case 2:
              fprintf(out, "%ld %ld %ld %ld %ld %ld curveto\n",
                      (-vec[cs].x+4*vec[cs+1].x)/3,
                      (-vec[cs].y+4*vec[cs+1].y)/3,
                      (4*vec[cs+2].x-vec[ce].x)/3,
                      (4*vec[cs+2].y-vec[ce].y)/3,
                      vec[ce].x, vec[ce].y);
              break;

          case 3:
              fprintf(out, "%ld %ld %ld %ld %ld %ld curveto\n",
                      (vec[cs].x+2*vec[cs+1].x)/3,
                      (vec[cs].y+2*vec[cs+1].y)/3,
                      (5*vec[cs+1].x+vec[cs+2].x)/6,
                      (5*vec[cs+1].y+vec[cs+2].y)/6,
                      (vec[cs+1].x+vec[cs+2].x)/2,
                      (vec[cs+1].y+vec[cs+2].y)/2);

              fprintf(out, "%ld %ld %ld %ld %ld %ld curveto\n",
                      (vec[cs+1].x+5*vec[cs+2].x)/6,
                      (vec[cs+1].y+5*vec[cs+2].y)/6,
                      (5*vec[cs+2].x+vec[cs+3].x)/6,
                      (5*vec[cs+2].y+vec[cs+3].y)/6,
                      (vec[cs+3].x+vec[cs+2].x)/2,
                      (vec[cs+3].y+vec[cs+2].y)/2);

              fprintf(out, "%ld %ld %ld %ld %ld %ld curveto\n",
                      (vec[cs+2].x+5*vec[cs+3].x)/6,
                      (vec[cs+2].y+5*vec[cs+3].y)/6,
                      (2*vec[cs+3].x+vec[ce].x)/3,
                      (2*vec[cs+3].y+vec[ce].y)/3,
                      vec[ce].x, vec[ce].y);
              break;

          default: /* anything |nguide > 3| */
              k1 = cs + nguide;

              fprintf(out, "%ld %ld %ld %ld %ld %ld curveto\n",
                      (vec[cs].x+2*vec[cs+1].x)/3,
                      (vec[cs].y+2*vec[cs+1].y)/3,
                      (5*vec[cs+1].x+vec[cs+2].x)/6,
                      (5*vec[cs+1].y+vec[cs+2].y)/6,
                      (vec[cs+1].x+vec[cs+2].x)/2,
                      (vec[cs+1].y+vec[cs+2].y)/2);

              for( k = cs+2 ; k <= k1-1 ; k++ )
              {
                fprintf(out, "%ld %ld %ld %ld %ld %ld curveto\n",
                        (vec[k-1].x+5*vec[k].x)/6,
                        (vec[k-1].y+5*vec[k].y)/6,
                        (5*vec[k].x+vec[k+1].x)/6,
                        (5*vec[k].y+vec[k+1].y)/6,
                        (vec[k].x+vec[k+1].x)/2,
                        (vec[k].y+vec[k+1].y)/2);
              }

              fprintf(out, "%ld %ld %ld %ld %ld %ld curveto\n",
                      (vec[k1-1].x+5*vec[k1].x)/6,
                      (vec[k1-1].y+5*vec[k1].y)/6,
                      (2*vec[k1].x+vec[ce].x)/3,
                      (2*vec[k1].y+vec[ce].y)/3,
                      vec[ce].x, vec[ce].y);
              break;
        }
      }

      if( i >= contour_end )
      {
        fprintf(out, "closepath\n");
        first = TRUE;
        i = contour_end + 1;
        j++;
      }
      else
      {
        i++;
      }
    }
  }

  fprintf(out, "fill } bind def\n");

  return 0;
}

static
int PSType3_generateOutlineFont(FILE *out, const char *psfontname, struct ft2info *ti, long block_offset)
{
        long   i;
        double scaler;
  const int    numchars = 256;
#ifdef USE_FT_PS_NAMES
        int    linewidth = 0;
#endif /* USE_FT_PS_NAMES */

  fprintf(out, "%%%%BeginFont: %s\n", psfontname);
  fprintf(out, "22 dict begin\n");
  fprintf(out, "/FontType 3 def\n");
  fprintf(out, "/StrokeWidth 0 def\n");
  fprintf(out, "/PaintType 0 def\n");
  fprintf(out, "/FontName (%s) def\n",     psfontname);
  fprintf(out, "/FontInfo 9 dict dup begin\n");
  fprintf(out, "  /FullName (%s) def\n",   ti->nameid.full_name?ti->nameid.full_name:psfontname);
  fprintf(out, "  /Notice (%s) def\n",     ti->nameid.copyright?ti->nameid.copyright:"nothing here");
  fprintf(out, "  /FamilyName (%s) def\n", ti->nameid.family?ti->nameid.family:psfontname);
  fprintf(out, "  /Weight (%s) def\n",     ti->nameid.subfamily?ti->nameid.subfamily:"Regular");
  fprintf(out, "  /version (%s) def\n",    ti->nameid.version?ti->nameid.version:"0.1");

  if( ti->ttpostscript )
  {
    fprintf(out, "  /italicAngle %.9g def\n",     (double)ti->ttpostscript->italicAngle);
    fprintf(out, "  /underlineThickness %d def\n", (int)ti->ttpostscript->underlineThickness);
    fprintf(out, "  /underlinePosition %d def\n",  (int)ti->ttpostscript->underlinePosition);
    fprintf(out, "  /isFixedPitch %s def\n",       ((ti->ttpostscript->isFixedPitch)?("true"):("false")));
  }
  else
  {
    fprintf(out, "  /italicAngle %.9g def\n",      0.0);
    fprintf(out, "  /underlineThickness %d def\n", 100);
    fprintf(out, "  /underlinePosition %d def\n",  0);
    fprintf(out, "  /isFixedPitch false def\n");
  }

  fprintf(out, "end def\n");

  scaler = (1000.0 / (double)ti->ttface->units_per_EM) / 1000.0;
  fprintf(out, "/FontMatrix [%.9g 0 0 %.9g 0 0] def\n", scaler, scaler);

  if( ti->ttheader )
  {
    fprintf(out, "/FontBBox [%d %d %d %d] def\n",
            (int)ti->ttheader->xMin,
            (int)ti->ttheader->yMin,
            (int)ti->ttheader->xMax,
            (int)ti->ttheader->yMax);
  }
  else
  {
    fprintf(out, "/FontBBox [%ld %ld %ld %ld] def\n",
                 ti->ttface->bbox.xMin,
                 ti->ttface->bbox.yMin,
                 ti->ttface->bbox.xMax,
                 ti->ttface->bbox.yMax);
		 
  }

  fprintf(out, "/Encoding [\n");
  for( i = 0 ; i < 256 ; i++ )
  {
#ifdef USE_FT_PS_NAMES
    char namebuf[256];
    PsOut_Get_FreeType_Glyph_Name(namebuf, ti->pFont, i+block_offset);
    linewidth += strlen(namebuf) + 2;
    fprintf(out, "/%s%s", namebuf, (linewidth > 70)?(linewidth = 0, "\n"):(" "));
#else
    fprintf(out, "/ch%02x%s", i, (((i % 10) == 9)?("\n"):(" ")));
#endif /* USE_FT_PS_NAMES */
  }
  fprintf(out, "] def\n");

  fprintf(out, "/CharProcs %d dict def CharProcs begin\n", (int)(numchars + 1));
  fprintf(out, "/.notdef {\n"
               "1000 0 0 0 0 0 setcachedevice\n"
               "fill } bind def\n");
  for( i = 0 ; i < numchars ; i++ )
  {
    char buf[32];
#ifdef USE_FT_PS_NAMES
    char namebuf[256];
    PsOut_Get_FreeType_Glyph_Name(namebuf, ti->pFont, i+block_offset);
    sprintf(buf, "%s ", namebuf);
#else
    sprintf(buf, "ch%02lx ", i);
#endif /* USE_FT_PS_NAMES */
    PSType3_createOutlineGlyphs(out, ti, i+block_offset, buf);
  }
  fprintf(out, "end\n"
               "/BuildGlyph {\n"
               "  exch /CharProcs get exch\n"
               "  2 copy known not {pop /.notdef} if get exec } bind def\n"
               "/BuildChar { 1 index /Encoding get exch get\n"
               "  1 index /Encoding get exec } bind def\n");
  fprintf(out, "currentdict end /%s exch definefont pop\n", psfontname);
  fprintf(out, "%%EndFont\n");

  return 0;
}

static
char *FT_Get_TT_NAME_ID(FT_Face ttface, int index)
{
  FT_SfntName  name;
  char        *s;

  if( index >= FT_Get_Sfnt_Name_Count(ttface) )
    return NULL;

  FT_Get_Sfnt_Name(ttface, index, &name);
  s = (char *)malloc(name.string_len+2);
  if( !s )
    return NULL;
  memcpy(s, (char *)name.string, name.string_len);
  s[name.string_len] = '\0';
  return s;
}

int PsOut_DownloadFreeType3(PsOutPtr self, const char *psfontname, FontPtr pFont, long block_offset)
{
  struct ft2info  cft2info = { 0 };
  struct ft2info *ti       = &cft2info;

  S_Flush(self);

  ti->tf     = (FTFontPtr)pFont->fontPrivate;
  ti->ttface = ti->tf->instance->face->face;
  ti->pFont  = pFont;
#ifdef DEBUG_gisburn
  fprintf(stderr, "# Downloading FT2 font filename='%s', ttface=%lx\n", ti->tf->instance->face->filename, (long)ti->ttface);
#endif /* DEBUG_gisburn */

  ti->nameid.full_name = FT_Get_TT_NAME_ID(ti->ttface, TT_NAME_ID_FULL_NAME);
  ti->nameid.copyright = FT_Get_TT_NAME_ID(ti->ttface, TT_NAME_ID_COPYRIGHT);
  ti->nameid.family    = FT_Get_TT_NAME_ID(ti->ttface, TT_NAME_ID_FONT_FAMILY);
  ti->nameid.subfamily = FT_Get_TT_NAME_ID(ti->ttface, TT_NAME_ID_FONT_SUBFAMILY);
  ti->nameid.version   = FT_Get_TT_NAME_ID(ti->ttface, TT_NAME_ID_VERSION_STRING);

  ti->ttheader     =     (TT_Header *)FT_Get_Sfnt_Table(ti->ttface, ft_sfnt_head);
  ti->ttpostscript = (TT_Postscript *)FT_Get_Sfnt_Table(ti->ttface, ft_sfnt_post);

  PSType3_generateOutlineFont(self->Fp, psfontname, ti, block_offset);

  free(ti->nameid.full_name);
  free(ti->nameid.copyright);
  free(ti->nameid.family);
  free(ti->nameid.subfamily);
  free(ti->nameid.version);

  S_Flush(self);

  return 0;
}

