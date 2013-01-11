/*
 * fontconfig/fc-validate/fc-validate.c
 *
 * Copyright © 2003 Keith Packard
 * Copyright © 2012 Red Hat, Inc.
 * Red Hat Author(s): Akira TAGOH
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#ifdef linux
#define HAVE_GETOPT_LONG 1
#endif
#define HAVE_GETOPT 1
#endif

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#ifndef HAVE_GETOPT
#define HAVE_GETOPT 0
#endif
#ifndef HAVE_GETOPT_LONG
#define HAVE_GETOPT_LONG 0
#endif

#if HAVE_GETOPT_LONG
#undef  _GNU_SOURCE
#define _GNU_SOURCE
#include <getopt.h>
static const struct option longopts[] = {
    {"index", 1, 0, 'i'},
    {"lang", 1, 0, 'l'},
    {"verbose", 0, 0, 'v'},
    {"version", 0, 0, 'V'},
    {"help", 0, 0, 'h'},
    {NULL,0,0,0},
};
#else
#if HAVE_GETOPT
extern char *optarg;
extern int optind, opterr, optopt;
#endif
#endif

static void
usage (char *program, int error)
{
    FILE *file = error ? stderr : stdout;
#if HAVE_GETOPT_LONG
    fprintf (file, "usage: %s [-Vhv] [-i index] [-l LANG] [--index index] [--lang LANG] [--verbose] [--version] [--help] font-file...\n",
	     program);
#else
    fprintf (file, "usage: %s [-Vhv] [-i index] [-l LANG] font-file...\n",
	     program);
#endif
    fprintf (file, "Validate font files and print result\n");
    fprintf (file, "\n");
#if HAVE_GETOPT_LONG
    fprintf (file, "  -i, --index INDEX    display the INDEX face of each font file only\n");
    fprintf (file, "  -l, --lang=LANG      set LANG instead of current locale\n");
    fprintf (file, "  -v, --verbose        show more detailed information\n");
    fprintf (file, "  -V, --version        display font config version and exit\n");
    fprintf (file, "  -h, --help           display this help and exit\n");
#else
    fprintf (file, "  -i INDEX   (index)        display the INDEX face of each font file only\n");
    fprintf (file, "  -l LANG    (lang)         set LANG instead of current locale\n");
    fprintf (file, "  -v         (verbose)      show more detailed information\n");
    fprintf (file, "  -V         (version)      display font config version and exit\n");
    fprintf (file, "  -h         (help)         display this help and exit\n");
#endif
    exit (error);
}

int
main (int argc, char **argv)
{
    int		index_set = 0;
    int		set_index = 0;
    FcChar8     *lang = NULL;
    const FcCharSet *fcs_lang = NULL;
    int		err = 0;
    int		i;
    FT_Library  ftlib;
    FcBool      verbose = FcFalse;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
    int		c;

    setlocale (LC_ALL, "");

#if HAVE_GETOPT_LONG
    while ((c = getopt_long (argc, argv, "i:l:mVhv", longopts, NULL)) != -1)
#else
    while ((c = getopt (argc, argv, "i:l:mVhv")) != -1)
#endif
    {
	switch (c) {
	case 'i':
	    index_set = 1;
	    set_index = atoi (optarg);
	    break;
	case 'l':
	    lang = (FcChar8 *) FcLangNormalize ((const FcChar8 *) optarg);
	    break;
	case 'v':
	    verbose = FcTrue;
	    break;
	case 'V':
	    fprintf (stderr, "fontconfig version %d.%d.%d\n",
		     FC_MAJOR, FC_MINOR, FC_REVISION);
	    exit (0);
	case 'h':
	    usage (argv[0], 0);
	default:
	    usage (argv[0], 1);
	}
    }
    i = optind;
#else
    i = 1;
    verbose = FcTrue;
#endif

    if (i == argc)
	usage (argv[0], 1);

    if (!lang)
	lang = FcLangNormalize ((const FcChar8 *) setlocale (LC_CTYPE, NULL));

    if (lang)
	fcs_lang = FcLangGetCharSet (lang);

    if (FT_Init_FreeType (&ftlib))
    {
	fprintf (stderr, "Can't initalize FreeType library\n");
	return 1;
    }

    for (; i < argc; i++)
    {
	int index;

	index = set_index;

	do {
	    FT_Face face;
	    FcCharSet *fcs, *fcs_sub;

	    if (FT_New_Face (ftlib, argv[i], index, &face))
	    {
		if (!index_set && index > 0)
		    break;
		fprintf (stderr, "Unable to open %s\n", argv[i]);
		err = 1;
	    }
	    else
	    {
		FcChar32 count;

		fcs = FcFreeTypeCharSet (face, NULL);
		fcs_sub = FcCharSetSubtract (fcs_lang, fcs);

		count = FcCharSetCount (fcs_sub);
		if (count > 0)
		{
		    FcChar32 ucs4, pos, map[FC_CHARSET_MAP_SIZE];

		    printf ("%s:%d Missing %d glyph(s) to satisfy the coverage for %s language\n",
			    argv[i], index, count, lang);

		    if (verbose)
		    {
			for (ucs4 = FcCharSetFirstPage (fcs_sub, map, &pos);
			     ucs4 != FC_CHARSET_DONE;
			     ucs4 = FcCharSetNextPage (fcs_sub, map, &pos))
			{
			    int j;

			    for (j = 0; j < FC_CHARSET_MAP_SIZE; j++)
			    {
				FcChar32 bits = map[j];
				FcChar32 base = ucs4 + j * 32;
				int b = 0;

				while (bits)
				{
				    if (bits & 1)
					printf ("  0x%04x\n", base + b);
				    bits >>= 1;
				    b++;
				}
			    }
			}
		    }
		}
		else
		{
		    printf ("%s:%d Satisfy the coverage for %s language\n", argv[i], index, lang);
		}

		FcCharSetDestroy (fcs);
		FcCharSetDestroy (fcs_sub);

		FT_Done_Face (face);
	    }

	    index++;
	} while (index_set == 0);
    }

    FT_Done_FreeType (ftlib);

    if (lang)
	FcStrFree (lang);

    FcFini ();
    return err;
}
