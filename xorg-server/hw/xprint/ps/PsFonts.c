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
**    *  File:		PsFonts.c
**    *
**    *  Contents:	Font code for PS driver.
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

#include "regionstr.h"
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"
#include "scrnintstr.h"
#include <X11/fonts/fontxlfd.h>
#include <X11/fonts/fntfil.h>
#include <X11/fonts/fntfilst.h>

#include "Ps.h"

#include <ctype.h>
#include <limits.h>
#include <sys/stat.h>

Bool
PsRealizeFont(
  ScreenPtr  pscr,
  FontPtr    pFont)
{
  return TRUE;
}

Bool
PsUnrealizeFont(
  ScreenPtr  pscr,
  FontPtr    pFont)
{
  return TRUE;
}

char *
PsGetFontName(FontPtr pFont)
{
  int         i;
  int         nprops = pFont->info.nprops;
  FontPropPtr props  = pFont->info.props;
  Atom        name   = MakeAtom("FONT", 4, True);
  Atom        value  = (Atom)0;

  for( i=0 ; i<nprops ; i++ )
  {
    if( (Atom)props[i].name==name )
      { value = props[i].value; break; }
  }
  if( !value ) return (char *)0;
  return NameForAtom(value);
}

int
PsGetFontSize(FontPtr pFont, float *mtx)
{
  FontScalableRec   vals;
  char             *name = PsGetFontName(pFont);
  int               value = 0;

  FontParseXLFDName(name, &vals, FONT_XLFD_REPLACE_NONE);
  if( vals.values_supplied&PIXELSIZE_ARRAY )
  {
    int  i;
    for( i=0 ; i<4 ; i++ )
      mtx[i] = (float)vals.pixel_matrix[i];
  }
  else
  {
    value = vals.pixel;
    if( !value ) value = 20;
  }
  return value;
}

char *
PsGetPSFontName(FontPtr pFont)
{
  int         i;
  int         nprops = pFont->info.nprops;
  FontPropPtr props  = pFont->info.props;
  /* "_ADOBE_POSTSCRIPT_FONTNAME" maps directly to a PMF OBJ_NAME attribute
   * name - changing the name will break printer-builtin fonts. */
  Atom        name   = MakeAtom("_ADOBE_POSTSCRIPT_FONTNAME", 26, True); 
  Atom        value  = (Atom)0;

  for( i=0 ; i<nprops ; i++ )
  {
    if( (Atom)props[i].name==name )
      { value = props[i].value; break; }
  }
  if( !value ) return (char *)0; 
  return NameForAtom(value);
}

int
PsIsISOLatin1Encoding(FontPtr pFont)
{
  int          i;
  int          nprops = pFont->info.nprops;
  FontPropPtr  props  = pFont->info.props;
  Atom         reg = MakeAtom("CHARSET_REGISTRY", 16, True);
  Atom         enc = MakeAtom("CHARSET_ENCODING", 16, True);
  Atom         rv = 0, ev = 0;
  char        *rp = 0;
  char        *ep = 0;

  for( i=0 ; i<nprops ; i++ )
  {
    if( (Atom)props[i].name==reg ) rv = props[i].value;
    if( (Atom)props[i].name==enc ) ev = props[i].value;
  }
  if( rv ) rp = NameForAtom(rv);
  if( ev ) ep = NameForAtom(ev);
  if( (!rp) || (!ep) ) return(0);
  if( (char)tolower(rp[0])!='i' ||
      (char)tolower(rp[1])!='s' ||
      (char)tolower(rp[2])!='o' ||
      memcmp(&rp[3], "8859", 4)!=0 ||
      ep[0]!='1' ) return(0);
  return(1);
}

/* Return the encoding part of the XLFD (e.g. "*-iso8859-6.8x" etc.)*/
char *PsGetEncodingName(FontPtr pFont)
{
  int          i;
  int          nprops = pFont->info.nprops;
  FontPropPtr  props  = pFont->info.props;
  Atom         fnt = MakeAtom("FONT",              4, True);
  Atom         reg = MakeAtom("CHARSET_REGISTRY", 16, True);
  Atom         enc = MakeAtom("CHARSET_ENCODING", 16, True);
  Atom         fv = 0, rv = 0, ev = 0;
  char        *fp = 0;
  char        *rp = 0;
  char        *ep = 0;
  char        *encname;

  for( i=0 ; i<nprops ; i++ )
  {
    if( props[i].name==fnt ) fv = props[i].value;
    if( props[i].name==reg ) rv = props[i].value;
    if( props[i].name==enc ) ev = props[i].value;
  }
  if( fv ) fp = NameForAtom(fv);
  if( rv ) rp = NameForAtom(rv);
  if( ev ) ep = NameForAtom(ev);

  if( (!rp) || (!ep) || (!fp))
    return(0);
  
  encname  = fp;
  encname += strlen(encname) - (strlen(rp) + strlen(ep) + 1);
  
  return encname;
}

/* strstr(), case-insensitive */
static 
char *str_case_str(const char *s, const char *find)
{
  size_t len;
  char   c, 
         sc;

  if ((c = tolower(*find++)) != '\0')
  {
    len = strlen(find);
    do 
    {
      do
      {
        if ((sc = tolower(*s++)) == '\0')
          return NULL;
      } while (sc != c);
    } while (strncasecmp(s, find, len) != 0);
    s--;
  }
  return ((char *)s);
}

/* Check if the font path element is a directory which can be examined
 * (for example the font may be from a font server
 * (e.g. pFont->fpe->name == "tcp/:7100"))
 */
static
Bool IsFPEaReadableDir(FontPtr pFont)
{
  const char *fpe_name = pFont->fpe->name;
  if (!fpe_name)
    return False;

#define MODEL_FONTPATH_PREFIX     "PRINTER:"
#define MODEL_FONTPATH_PREFIX_LEN 8
  /* Strip model-specific font path prefix if there is one... */
  if (!strncmp(fpe_name, MODEL_FONTPATH_PREFIX, MODEL_FONTPATH_PREFIX_LEN))
    fpe_name += MODEL_FONTPATH_PREFIX_LEN;
    
  if (access(fpe_name, F_OK) == 0)
  {
    return True;
  }

  return False;
}

static
char *getFontFilename(FontPtr pFont)
{
  FontDirectoryPtr   dir;
  const char        *dlfnam;
  FILE              *file;
  struct stat        statb;
  char               buf[512];
  char              *front, *fn;
  char               font_dir_fname[PATH_MAX],  /* Full path of fonts.dir */
                     font_file_fname[PATH_MAX]; /* Name of font file (excluding path) */

#ifdef XP_USE_FREETYPE
  if( PsIsFreeTypeFont(pFont) )
  {
    const char  *fontname = PsGetFTFontFileName(pFont);

#ifdef DEBUG_gisburn
    fprintf(stderr, "getFontFilename: freetype font, file='%s'\n", fontname?fontname:"<NULL>");
#endif /* DEBUG_gisburn */  

    if( !fontname )
      return NULL;
      
    return strdup(fontname);
  }
#endif /* XP_USE_FREETYPE */

  if (!IsFPEaReadableDir(pFont))
  {
#ifdef DEBUG_gisburn
    fprintf(stderr, "getFontFilename: '%s' no valid font path on disk\n", pFont->fpe->name);
#endif /* DEBUG_gisburn */  
    return NULL;
  }

  dir = pFont->fpe->private;
  sprintf(font_dir_fname, "%s%s", dir->directory, "fonts.dir");  
  
  if (!(dlfnam = PsGetFontName(pFont)))
    return NULL;
  
  file = fopen(font_dir_fname, "r");
  if (file)
  {
    if (fstat (fileno(file), &statb) == -1)
      return NULL;

    while( fgets(buf, sizeof(buf)-1, file) )
    {             
      if ((fn = strstr(buf, " -")))
      {
        strcpy(font_file_fname, buf);
        font_file_fname[fn - buf] = '\0';
        fn++;
        if ((front = str_case_str(fn, "normal-")))
        {
          fn[front - fn] = '\0';
          if (str_case_str(dlfnam, fn))        
          {
            char full_font_file_path[PATH_MAX];

            fclose(file);
            
            sprintf(full_font_file_path, "%s%s", dir->directory, font_file_fname);

#ifdef xDEBUG_gisburn
            fprintf(stderr, "getFontFilename: returning '%s'\n", full_font_file_path);
#endif /* DEBUG_gisburn */
            return strdup(full_font_file_path); 
          }
        }
      }
    }
  }
  font_file_fname[0] = '\0';
  fclose(file);

#ifdef DEBUG_gisburn
  fprintf(stderr, "getFontFilename: returning NULL\n");
#endif /* DEBUG_gisburn */

  return NULL;
}

static
PsFontTypeInfoRec *PsFindFontTypeInfoRec(DrawablePtr pDrawable, FontPtr pFont)
{
  PsContextPrivRec  *cPriv = PsGetPsContextPriv(pDrawable);
  PsFontTypeInfoRec *rec;
  const char        *psname;
  char              *font_filename;
  char              *encname;
#ifdef XP_USE_FREETYPE
  Bool               is_freetypefont;
#endif /* XP_USE_FREETYPE */
  
#ifdef XP_USE_FREETYPE
  is_freetypefont = PsIsFreeTypeFont(pFont);
#endif /* XP_USE_FREETYPE */
  encname         = PsGetEncodingName(pFont);

  /* First try: Search by PostScript font name */
  psname = PsGetPSFontName(pFont);
  if (psname)
  {
    for( rec = cPriv->fontTypeInfoRecords ; rec != NULL ; rec = rec->next )
    {
#ifdef XP_USE_FREETYPE
      if (is_freetypefont)
      {
        if (rec->adobe_ps_name)
        {
          if ((rec->font_type == PSFTI_FONT_TYPE_FREETYPE) &&
              (!strcmp(rec->adobe_ps_name, psname)) &&
              (!strcmp(rec->ft_download_encoding, encname)))
          {
            return rec;
          }
        }
      }
      else
#endif /* XP_USE_FREETYPE */
      {
        if (rec->adobe_ps_name)
        {
          if ((rec->font_type != PSFTI_FONT_TYPE_FREETYPE) &&
              (!strcmp(rec->adobe_ps_name, psname)))
          {
            return rec;
          }
        }
      }
    }
  }

  /* Last attempt: Search by filename */
  font_filename = getFontFilename(pFont);
  if (font_filename)
  {
    for( rec = cPriv->fontTypeInfoRecords ; rec != NULL ; rec = rec->next )
    {
      if (rec->filename)
      {
#ifdef XP_USE_FREETYPE
        if (is_freetypefont)
        {
          if ( (rec->font_type == PSFTI_FONT_TYPE_FREETYPE) &&
               (!strcasecmp(rec->filename, font_filename)) && 
               (!strcasecmp(rec->ft_download_encoding, encname)) )
          {
            free(font_filename);
            return rec;
          }
        }
        else
#endif /* XP_USE_FREETYPE */
        {
          if ( (rec->font_type != PSFTI_FONT_TYPE_FREETYPE) &&
               (!strcasecmp(rec->filename, font_filename)) )
          {
            free(font_filename);
            return rec;
          }
        }
      }
    }
    
    free(font_filename);
  }
  
  return NULL;
}

static
void PsAddFontTypeInfoRec(DrawablePtr pDrawable, PsFontTypeInfoRec *add_rec)
{ 
  PsContextPrivRec *cPriv = PsGetPsContextPriv(pDrawable);

  /* ToDO: Always move the last used entry to the top that the list get's
   * sorted in an efficient order... :-) */
  add_rec->next = cPriv->fontTypeInfoRecords;
  cPriv->fontTypeInfoRecords = add_rec;
}

static
Bool strcaseendswith(const char *str, const char *suffix)
{
  const char *s;

  s = str + strlen(str) - strlen(suffix);

  if (!strcasecmp(s, suffix))
    return True;

  return False;
}


static
int getFontFileType( const char *filename )
{
  int type;

  /* Is this a Adobe PostScript Type 1 binary font (PFB) ? */
  if( strcaseendswith(filename, ".pfb") )
  {
    type = PSFTI_FONT_TYPE_PS_TYPE1_PFB;
  }
  /* Is this a Adobe PostScript ASCII font (PFA) ? */
  else if( strcaseendswith(filename, ".pfa") )
  {
    type = PSFTI_FONT_TYPE_PS_TYPE1_PFA;
  }
  /* Is this a PMF(=Printer Metrics File) ? */
  else if( strcaseendswith(filename, ".pmf") )
  {
    type = PSFTI_FONT_TYPE_PMF;
  }
  /* Is this a TrueType font file ? */
  else if( strcaseendswith(filename, ".ttf") ||
           strcaseendswith(filename, ".ttc") ||
           strcaseendswith(filename, ".otf") ||
           strcaseendswith(filename, ".otc") )
  {
    type = PSFTI_FONT_TYPE_TRUETYPE;
  }
  else
  {
    type = PSFTI_FONT_TYPE_OTHER;
  }

#ifdef XP_USE_FREETYPE
  {
    XpContextPtr  pCon;
    char         *downloadfonts;
    pCon = XpGetPrintContext(requestingClient);
    downloadfonts = XpGetOneAttribute(pCon, XPPrinterAttr, "xp-psddx-download-fonts");
    if( downloadfonts )
    {
      /* Should we download PS Type1 fonts as PS Type1||Type3 ? */
      if( (type == PSFTI_FONT_TYPE_PS_TYPE1_PFA) &&
          (strstr(downloadfonts, "pfa") != NULL) )
      {
        type = PSFTI_FONT_TYPE_FREETYPE;
      }

      if( (type == PSFTI_FONT_TYPE_PS_TYPE1_PFB) &&
          (strstr(downloadfonts, "pfb") != NULL) )
      {
        type = PSFTI_FONT_TYPE_FREETYPE;
      }

      /* Should we download TrueType fonts as PS Type1||Type3 ? */
      if( (type == PSFTI_FONT_TYPE_TRUETYPE) &&
          ((strstr(downloadfonts, "ttf") != NULL) || 
           (strstr(downloadfonts, "ttc") != NULL) || 
           (strstr(downloadfonts, "otf") != NULL) || 
           (strstr(downloadfonts, "otc") != NULL)) )
      {
        type = PSFTI_FONT_TYPE_FREETYPE;
      }
    }
  }
#endif /* XP_USE_FREETYPE */

#ifdef DEBUG_gisburn
  fprintf(stderr, "getFontFileType: '%s' is %d\n", filename, (int)type);
#endif /* DEBUG_gisburn */
  return type;
}

PsFTDownloadFontType PsGetFTDownloadFontType(void)
{
  PsFTDownloadFontType  downloadfonttype;
  XpContextPtr          pCon;
  char                 *psfonttype;

  pCon       = XpGetPrintContext(requestingClient);
  psfonttype = XpGetOneAttribute(pCon, XPPrinterAttr, "xp-psddx-download-font-type");

  if( !psfonttype || !strlen(psfonttype) )
  {
    return PsFontType1; /* Default download font type is PS Type1 */
  }

  if( !strcmp(psfonttype, "bitmap") )
  {
    downloadfonttype = PsFontBitmap;
  }
  else if( !strcmp(psfonttype, "pstype3") )
  {
    downloadfonttype = PsFontType3;
  }
  else if( !strcmp(psfonttype, "pstype1") )
  { 
    downloadfonttype = PsFontType1;
  }
  else
  {
     FatalError("PS DDX: XPPrinterAttr/xp-psddx-download-freetype-font-type='%s' not implemented\n", psfonttype);
     return 0; /* NO-OP, FatalError() will call |exit()| */
  }
  
  return downloadfonttype;
}

static
PsFontTypeInfoRec *PsCreateFontTypeInfoRec(DrawablePtr pDrawable, FontPtr pFont)
{
  char              *dlfnam;
  PsFontTypeInfoRec *rec;
  
  if (!(dlfnam = PsGetFontName(pFont)))
    return NULL;

  if (!(rec = (PsFontTypeInfoRec *)xalloc(sizeof(PsFontTypeInfoRec))))
    return NULL;
  memset(rec, 0, sizeof(PsFontTypeInfoRec));

  rec->next              = NULL;

  if ((rec->filename = getFontFilename(pFont)))
  {
    rec->font_type = getFontFileType(rec->filename);   
  }
  else
  {
    rec->filename  = NULL;
    rec->font_type = PSFTI_FONT_TYPE_OTHER;
  }

  rec->adobe_ps_name         = PsGetPSFontName(pFont);
#ifdef XP_USE_FREETYPE
  rec->ft_download_encoding  = PsGetEncodingName(pFont);
  rec->ft_download_font_type = PsGetFTDownloadFontType();
#endif /* XP_USE_FREETYPE */
  rec->download_ps_name      = NULL;

#define SET_FONT_DOWNLOAD_STATUS(rec, downloaded) { int i; for (i = 0 ; i < 256 ; i++) { (rec)->alreadyDownloaded[i]=(downloaded); } }

  /* Set some flags based on the font type */
  switch( rec->font_type )
  {
    case PSFTI_FONT_TYPE_PS_TYPE1_PFA:
    case PSFTI_FONT_TYPE_PS_TYPE1_PFB:
      rec->downloadableFont  = True;
      SET_FONT_DOWNLOAD_STATUS(rec, False);
      rec->is_iso_encoding = PsIsISOLatin1Encoding(pFont);
      break;

    case PSFTI_FONT_TYPE_PMF:
      rec->downloadableFont  = True; /* This font is in printer's ROM */
      SET_FONT_DOWNLOAD_STATUS(rec, True);
      rec->is_iso_encoding = PsIsISOLatin1Encoding(pFont);
      break;
    
    case PSFTI_FONT_TYPE_TRUETYPE:
      /* Note: TrueType font download not implemented */
      rec->downloadableFont  = False;
      SET_FONT_DOWNLOAD_STATUS(rec, False);
      rec->is_iso_encoding = PsIsISOLatin1Encoding(pFont);
      break;

#ifdef XP_USE_FREETYPE
    case PSFTI_FONT_TYPE_FREETYPE:
      if( rec->ft_download_font_type == PsFontType1 ||
          rec->ft_download_font_type == PsFontType3 )
      {
        rec->downloadableFont = True;
      }
      else
      {
        rec->downloadableFont = False;
      }

      SET_FONT_DOWNLOAD_STATUS(rec, False);
      rec->is_iso_encoding   = False; /* Freetype--->PS Type1/Type3 uses always non-iso PS encoding for now */
      break;
#endif /* XP_USE_FREETYPE */

    case PSFTI_FONT_TYPE_OTHER:
    default:
      rec->downloadableFont  = False;
      SET_FONT_DOWNLOAD_STATUS(rec, False);
      rec->is_iso_encoding = PsIsISOLatin1Encoding(pFont);
      break;
  }
  
#ifdef XP_USE_FREETYPE
  if( (rec->font_type == PSFTI_FONT_TYPE_FREETYPE) )
  {
    char *s;
    register int c;

    if( rec->adobe_ps_name )
    {
      rec->download_ps_name = malloc(strlen(rec->adobe_ps_name) + strlen(rec->ft_download_encoding) + 2);
      sprintf(rec->download_ps_name, "%s_%s", rec->adobe_ps_name, rec->ft_download_encoding);
    }
    else
    {
      /* Unfortunately not all TTF fonts have a PostScript font name (like
       * Solaris TTF fonts in /usr/openwin/lib/locale/ko.UTF-8/X11/fonts/TrueType,
       * /usr/openwin/lib/locale/ko/X11/fonts/TrueType) - in this case we
       * have to generate a font name
       */
      char        ftfontname[64];
      static long myfontindex = 0L;
      sprintf(ftfontname, "psfont_%lx", myfontindex++);
      
      rec->download_ps_name = malloc(strlen(ftfontname) + strlen(rec->ft_download_encoding) + 2);
      sprintf(rec->download_ps_name, "%s_%s", ftfontname, rec->ft_download_encoding);
      
      fprintf(stderr, "PsCreateFontTypeInfoRec: Note: '%s' has no PS font name, using '%s' for now.\n", dlfnam, rec->download_ps_name);
    }
    
    /* Make sure the font name we use for download is a valid PS font name */
    for( s = rec->download_ps_name ; *s != '\0'; s++ )
    {
      c = *s;
      
      /* Check for allowed chars, invalid ones are replaced with a '_'
       * (and check that the first char is not a digit) */
      if( !(isalnum(c) || c == '.' || c == '_' || c == '-') || (s==rec->download_ps_name && isdigit(c)) )
      {
        *s = '_';
      }
    }
  }
  else
#endif /* XP_USE_FREETYPE */
  {
    if( rec->adobe_ps_name )
    {
      rec->download_ps_name = strdup(rec->adobe_ps_name);
    }
    else
    {
      rec->download_ps_name = NULL;
    }
  }

  /* Safeguard - only treat font as downloadable when we have a PS font name!! */
  if (!rec->download_ps_name && rec->downloadableFont)
  {
    /* XXX: Log this message to the log when the logging service has been hook'ed up */
    fprintf(stderr, "PsCreateFontTypeInfoRec: Safeguard: No PS font name for '%s'!\n", dlfnam);
    rec->downloadableFont = False;
  }

#ifdef DEBUG_gisburn
  fprintf(stderr, "PsCreateFontTypeInfoRec: Created PsFontTypeInfoRec '%s' ('%s'/'%s')\n",
          ((rec->filename)        ?(rec->filename)     :("<null>")),
          ((rec->adobe_ps_name)   ?(rec->adobe_ps_name):("<null>")),
          ((rec->download_ps_name)?(rec->download_ps_name):("<null>")));
#endif /* DEBUG_gisburn */

  return rec;
}

static
PsFontTypeInfoRec *PsGetFontTypeInfoRec(DrawablePtr pDrawable, FontPtr pFont)
{
  PsFontTypeInfoRec *rec;
  char              *dlfnam;

  if(!(dlfnam = PsGetFontName(pFont)))
    return NULL;
  
  rec = PsFindFontTypeInfoRec(pDrawable, pFont);
  if (rec)
    return rec;
    
  rec = PsCreateFontTypeInfoRec(pDrawable, pFont);
  if (!rec)
    return NULL;
  
  PsAddFontTypeInfoRec(pDrawable, rec);

  return rec;
}

static
void PsFreeFontTypeInfoRecords( PsContextPrivPtr priv )
{
  PsFontTypeInfoRec *curr, *next;
  curr = priv->fontTypeInfoRecords;
  while( curr != NULL )
  {
    if (curr->filename)
      free(curr->filename); /* Free memory allocated by |strdup()| */

    if (curr->download_ps_name)
      free(curr->download_ps_name);
      
    next = curr->next;
    xfree(curr);
    curr = next;
  }
}

static
PsFontInfoRec *PsFindFontInfoRec(DrawablePtr pDrawable, FontPtr pFont)
{
  PsContextPrivRec *cPriv = PsGetPsContextPriv(pDrawable);
  PsFontInfoRec    *rec;

  if (!pFont)
    return NULL;

  for( rec = cPriv->fontInfoRecords ; rec != NULL ; rec = rec->next )
  {
    if ((rec->font == pFont) && 
        (rec->font_fontPrivate == pFont->fontPrivate))
      return rec;
  }
  
  return NULL;
}

static
void PsAddFontInfoRec(DrawablePtr pDrawable, PsFontInfoRec *add_rec)
{ 
  PsContextPrivRec *cPriv = PsGetPsContextPriv(pDrawable);

  /* ToDO: Always move the last used entry to the top that the list get's
   * sorted in an efficient order... :-) */
  add_rec->next = cPriv->fontInfoRecords;
  cPriv->fontInfoRecords = add_rec;
}

static
PsFontInfoRec *PsCreateFontInfoRec(DrawablePtr pDrawable, FontPtr pFont)
{
  PsFontInfoRec     *rec;
  PsFontTypeInfoRec *ftir;
  
  if (!(ftir = PsGetFontTypeInfoRec(pDrawable, pFont)))
    return NULL;

  if (!(rec = (PsFontInfoRec *)xalloc(sizeof(PsFontInfoRec))))
    return NULL;
  memset(rec, 0, sizeof(PsFontInfoRec));

  rec->font     = pFont;
  rec->font_fontPrivate = pFont->fontPrivate;
  rec->ftir     = ftir;
  rec->next     = NULL;
  rec->dfl_name = PsGetFontName(pFont);
  rec->size     = PsGetFontSize(pFont, rec->mtx);

#ifdef DEBUG_gisburn
  fprintf(stderr, "PsCreateFontInfoRec: Created PsFontInfoRec '%s'\n",
          ((rec->dfl_name)?(rec->dfl_name):("<null>")));
#endif /* DEBUG_gisburn */

  return rec;
}

PsFontInfoRec *PsGetFontInfoRec(DrawablePtr pDrawable, FontPtr pFont)
{
  PsFontInfoRec *rec;

  rec = PsFindFontInfoRec(pDrawable, pFont);
  if (rec)
    return rec;
    
  rec = PsCreateFontInfoRec(pDrawable, pFont);
  if (!rec)
    return NULL;
  
  PsAddFontInfoRec(pDrawable, rec);

  return rec;
}

void PsFreeFontInfoRecords( PsContextPrivPtr priv )
{
  PsFontInfoRec *curr, *next;
  curr = priv->fontInfoRecords;
  while( curr != NULL )
  {  
    next = curr->next;
    xfree(curr);
    curr = next;
  }
  
  PsFreeFontTypeInfoRecords(priv);

  priv->fontTypeInfoRecords = NULL;
  priv->fontInfoRecords     = NULL;
}
