
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
#include <errno.h>
#include <sys/wait.h>

#include "os.h"
#define USE_PSOUT_PRIVATE 1
#include "psout.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <X11/Xproto.h>
#include <X11/fonts/font.h>
#include <X11/fonts/fontstruct.h>
#include <X11/fonts/fntfilst.h>
#include <X11/fonts/fontutil.h>
#include <X11/fonts/fontenc.h>
#include <X11/fonts/ft.h>
#define NOT_IN_FTFUNCS
#include <X11/fonts/ftfuncs.h>

int do_debug_ft2pt1             = FALSE;
int do_enable_ft2pt1_optimizer  = FALSE;

/* Defined in ttf2pt1.c */
int ft2pt1_main(int argc, char **argv,
                FTFontPtr tf, const char *download_psfontname, unsigned long download_font_block_offset);

/* Download FreeType outlines as PS Type1 font */
int PsOut_DownloadFreeType1(PsOutPtr self, const char *psfontname, FontPtr pFont, long block_offset)
{
  FTFontPtr tf;
  FT_Face   face;
  int       ft2pt1_numargs = 0;
  char     *ft2pt1_args[40];
  char     *pstype1filename_prefix;
  char      pstype1filename[PATH_MAX+1];
  int       ft2pt1_main_retval;
  pid_t     childpid;
  
  tf = (FTFontPtr)pFont->fontPrivate;
  face = tf->instance->face->face;
  
  /* Set debugging flags */
  do_debug_ft2pt1             = (getenv("XPRT_PSDDX_DO_DEBUG_FT2PT1") != NULL);
  do_enable_ft2pt1_optimizer  = (getenv("XPRT_PSDDX_DO_ENABLE_FT2PT1_OPTIMIZER") != NULL);

  if( do_debug_ft2pt1 )
  {
    fprintf(stderr, "# Converting FT2 font to PS Type1 filename='%s', ttface=%lx\n", tf->instance->face->filename, (long)face);
  }

  pstype1filename_prefix = tempnam(NULL, "Xprt_");

  ft2pt1_args[ft2pt1_numargs] = "ft2pt1";                        ft2pt1_numargs++;
  ft2pt1_args[ft2pt1_numargs] = "-Ob";                           ft2pt1_numargs++;
  ft2pt1_args[ft2pt1_numargs] = "-e";                            ft2pt1_numargs++;
  ft2pt1_args[ft2pt1_numargs] = "-a";                            ft2pt1_numargs++;
  ft2pt1_args[ft2pt1_numargs] = "-Ga";                           ft2pt1_numargs++;
  if( do_enable_ft2pt1_optimizer )
  {
    /* Scale fonts to a 1000x1000 matrix */
    ft2pt1_args[ft2pt1_numargs] = "-Ot";                         ft2pt1_numargs++;
  }
  else
  {
    /* Disable the ttf2pt1 optimisations */
    ft2pt1_args[ft2pt1_numargs] = "-Ou";                         ft2pt1_numargs++;
    ft2pt1_args[ft2pt1_numargs] = "-Oo";                         ft2pt1_numargs++;
    ft2pt1_args[ft2pt1_numargs] = "-Os";                         ft2pt1_numargs++;
    ft2pt1_args[ft2pt1_numargs] = "-Oh";                         ft2pt1_numargs++;
  }
  
  if( !do_debug_ft2pt1 )
  {
    ft2pt1_args[ft2pt1_numargs] = "-W 0";                        ft2pt1_numargs++;
  }
  ft2pt1_args[ft2pt1_numargs] = tf->instance->face->filename;    ft2pt1_numargs++;
  ft2pt1_args[ft2pt1_numargs] = pstype1filename_prefix;          ft2pt1_numargs++;
  ft2pt1_args[ft2pt1_numargs] = NULL;

/* XXX: ttf2pt1 has lots of leaks and global vars which are not cleaned-up
 * As long this problem exists we will simply fork() and call the converter
 * from the child process (all resources are free'ed when the child process
 * exists) as a workaround.
 */
#define FT2PT1_NEEDS_SEPERATE_PROCESS 1

#ifdef FT2PT1_NEEDS_SEPERATE_PROCESS
  /* Flush internal buffer and then the stdio stream before fork()! */
  S_Flush(self);
  fflush(self->Fp);

  childpid = fork();
  switch(childpid)
  {
    case -1:
        FatalError("PS DDX internal error: Cannot fork() converter child process, %s\n", strerror(errno));
        break;
    case 0: /* child */
        fclose(self->Fp);
        self->Fp = NULL;
            
        ft2pt1_main_retval = ft2pt1_main(ft2pt1_numargs, ft2pt1_args, tf, psfontname, block_offset);
        if( do_debug_ft2pt1 )
        {
          fprintf(stderr, "## ft2pt1_main returned %d (child)\n", ft2pt1_main_retval);
        }
        exit(ft2pt1_main_retval);
        break;
    default: /* parent */
        waitpid(childpid, &ft2pt1_main_retval, 0);
        break;
  }

  if( do_debug_ft2pt1 )
  {
    fprintf(stderr, "## ft2pt1_main returned %d (parent)\n", ft2pt1_main_retval);
  }
#else
  S_Flush(self);

  ft2pt1_main_retval = ft2pt1_main(ft2pt1_numargs, ft2pt1_args, tf, psfontname, block_offset);
  if( do_debug_ft2pt1 )
  {
    fprintf(stderr, "## ft2pt1_main returned %d (child)\n", ft2pt1_main_retval);
  }
#endif /* FT2PT1_NEEDS_SEPERATE_PROCESS */

  if( ft2pt1_main_retval != EXIT_SUCCESS )
  {
    FatalError("PS DDX internal error while converting FreeType font '%s' to PS Type1, error=%d\n",
               tf->instance->face->filename, ft2pt1_main_retval);
  }

  sprintf(pstype1filename, "%s.pfa", pstype1filename_prefix);
  if( do_debug_ft2pt1 )
  {
    fprintf(stderr, "# Downloading converted FT2/PS Type1 filename='%s'\n", pstype1filename);
  }

  PsOut_DownloadType1(self, "PsOut_DownloadFreeType1", psfontname, pstype1filename);
  
  if( !do_debug_ft2pt1 )
  {
    unlink(pstype1filename);
  }
  
  free(pstype1filename_prefix);

  S_Flush(self);

  return 0;
}


