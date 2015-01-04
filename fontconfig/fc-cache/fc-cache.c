/*
 * fontconfig/fc-cache/fc-cache.c
 *
 * Copyright © 2002 Keith Packard
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

#if defined (_WIN32)
#define STRICT
#include <windows.h>
#define sleep(x) Sleep((x) * 1000)
#undef STRICT
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

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
const struct option longopts[] = {
    {"error-on-no-fonts", 0, 0, 'E'},
    {"force", 0, 0, 'f'},
    {"really-force", 0, 0, 'r'},
    {"sysroot", required_argument, 0, 'y'},
    {"system-only", 0, 0, 's'},
    {"version", 0, 0, 'V'},
    {"verbose", 0, 0, 'v'},
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
    fprintf (file, "usage: %s [-EfrsvVh] [-y SYSROOT] [--error-on-no-fonts] [--force|--really-force] [--sysroot=SYSROOT] [--system-only] [--verbose] [--version] [--help] [dirs]\n",
	     program);
#else
    fprintf (file, "usage: %s [-EfrsvVh] [-y SYSROOT] [dirs]\n",
	     program);
#endif
    fprintf (file, "Build font information caches in [dirs]\n"
	     "(all directories in font configuration by default).\n");
    fprintf (file, "\n");
#if HAVE_GETOPT_LONG
    fprintf (file, "  -E, --error-on-no-fonts  raise an error if no fonts in a directory\n");
    fprintf (file, "  -f, --force              scan directories with apparently valid caches\n");
    fprintf (file, "  -r, --really-force       erase all existing caches, then rescan\n");
    fprintf (file, "  -s, --system-only        scan system-wide directories only\n");
    fprintf (file, "  -y, --sysroot=SYSROOT    prepend SYSROOT to all paths for scanning\n");
    fprintf (file, "  -v, --verbose            display status information while busy\n");
    fprintf (file, "  -V, --version            display font config version and exit\n");
    fprintf (file, "  -h, --help               display this help and exit\n");
#else
    fprintf (file, "  -E         (error-on-no-fonts)\n");
    fprintf (file, "                       raise an error if no fonts in a directory\n");
    fprintf (file, "  -f         (force)   scan directories with apparently valid caches\n");
    fprintf (file, "  -r,   (really force) erase all existing caches, then rescan\n");
    fprintf (file, "  -s         (system)  scan system-wide directories only\n");
    fprintf (file, "  -y SYSROOT (sysroot) prepend SYSROOT to all paths for scanning\n");
    fprintf (file, "  -v         (verbose) display status information while busy\n");
    fprintf (file, "  -V         (version) display font config version and exit\n");
    fprintf (file, "  -h         (help)    display this help and exit\n");
#endif
    exit (error);
}

static FcStrSet *processed_dirs;

static int
scanDirs (FcStrList *list, FcConfig *config, FcBool force, FcBool really_force, FcBool verbose, FcBool recursive, FcBool error_on_no_fonts, int *changed, FcStrSet *updateDirs)
{
    int		    ret = 0;
    const FcChar8   *dir;
    FcStrSet	    *subdirs;
    FcStrList	    *sublist;
    FcCache	    *cache;
    struct stat	    statb;
    FcBool	    was_valid, was_processed = FcFalse;
    int		    i;
    const FcChar8   *sysroot = FcConfigGetSysRoot (config);

    /*
     * Now scan all of the directories into separate databases
     * and write out the results
     */
    while ((dir = FcStrListNext (list)))
    {
	if (verbose)
	{
	    if (!recursive)
		printf ("Re-scanning ");
	    if (sysroot)
		printf ("[%s]", sysroot);
	    printf ("%s: ", dir);
	    fflush (stdout);
	}
	
	if (recursive && FcStrSetMember (processed_dirs, dir))
	{
	    if (verbose)
		printf ("skipping, looped directory detected\n");
	    continue;
	}

	if (stat ((char *) dir, &statb) == -1)
	{
	    switch (errno) {
	    case ENOENT:
	    case ENOTDIR:
		if (verbose)
		    printf ("skipping, no such directory\n");
		break;
	    default:
		fprintf (stderr, "\"%s\": ", dir);
		perror ("");
		ret++;
		break;
	    }
	    continue;
	}

	if (!S_ISDIR (statb.st_mode))
	{
	    fprintf (stderr, "\"%s\": not a directory, skipping\n", dir);
	    continue;
	}
	was_processed = FcTrue;

	if (really_force)
	    FcDirCacheUnlink (dir, config);

	cache = NULL;
	was_valid = FcFalse;
	if (!force) {
	    cache = FcDirCacheLoad (dir, config, NULL);
	    if (cache)
		was_valid = FcTrue;
	}
	
	if (!cache)
	{
	    if (!recursive)
		cache = FcDirCacheRescan (dir, config);
	    else
	    {
		(*changed)++;
		cache = FcDirCacheRead (dir, FcTrue, config);
	    }
	    if (!cache)
	    {
		fprintf (stderr, "%s: error scanning\n", dir);
		ret++;
		continue;
	    }
	}

	if (was_valid)
	{
	    if (verbose)
		printf ("skipping, existing cache is valid: %d fonts, %d dirs\n",
			FcCacheNumFont (cache), FcCacheNumSubdir (cache));
	}
	else
	{
	    if (verbose)
		printf ("caching, new cache contents: %d fonts, %d dirs\n", 
			FcCacheNumFont (cache), FcCacheNumSubdir (cache));

	    if (!FcDirCacheValid (dir))
	    {
		fprintf (stderr, "%s: failed to write cache\n", dir);
		(void) FcDirCacheUnlink (dir, config);
		ret++;
	    }
	}

	if (recursive)
	{
	    subdirs = FcStrSetCreate ();
	    if (!subdirs)
	    {
		fprintf (stderr, "%s: Can't create subdir set\n", dir);
		ret++;
		FcDirCacheUnload (cache);
		continue;
	    }
	    for (i = 0; i < FcCacheNumSubdir (cache); i++)
		FcStrSetAdd (subdirs, FcCacheSubdir (cache, i));
	    if (updateDirs && FcCacheNumSubdir (cache) > 0)
		FcStrSetAdd (updateDirs, dir);
	
	    FcDirCacheUnload (cache);
	
	    sublist = FcStrListCreate (subdirs);
	    FcStrSetDestroy (subdirs);
	    if (!sublist)
	    {
		fprintf (stderr, "%s: Can't create subdir list\n", dir);
		ret++;
		continue;
	    }
	    FcStrSetAdd (processed_dirs, dir);
	    ret += scanDirs (sublist, config, force, really_force, verbose, recursive, error_on_no_fonts, changed, updateDirs);
	    FcStrListDone (sublist);
	}
	else
	    FcDirCacheUnload (cache);
    }
    if (error_on_no_fonts && !was_processed)
	ret++;
    return ret;
}

static FcBool
cleanCacheDirectories (FcConfig *config, FcBool verbose)
{
    FcStrList	*cache_dirs = FcConfigGetCacheDirs (config);
    FcChar8	*cache_dir;
    FcBool	ret = FcTrue;

    if (!cache_dirs)
	return FcFalse;
    while ((cache_dir = FcStrListNext (cache_dirs)))
    {
	if (!FcDirCacheClean (cache_dir, verbose))
	{
	    ret = FcFalse;
	    break;
	}
    }
    FcStrListDone (cache_dirs);
    return ret;
}

int
main (int argc, char **argv)
{
    FcStrSet	*dirs, *updateDirs;
    FcStrList	*list;
    FcBool    	verbose = FcFalse;
    FcBool	force = FcFalse;
    FcBool	really_force = FcFalse;
    FcBool	systemOnly = FcFalse;
    FcBool	error_on_no_fonts = FcFalse;
    FcConfig	*config;
    FcChar8     *sysroot = NULL;
    int		i;
    int		changed;
    int		ret;
#if HAVE_GETOPT_LONG || HAVE_GETOPT
    int		c;

#if HAVE_GETOPT_LONG
    while ((c = getopt_long (argc, argv, "Efrsy:Vvh", longopts, NULL)) != -1)
#else
    while ((c = getopt (argc, argv, "Efrsy:Vvh")) != -1)
#endif
    {
	switch (c) {
	case 'E':
	    error_on_no_fonts = FcTrue;
	    break;
	case 'r':
	    really_force = FcTrue;
	    /* fall through */
	case 'f':
	    force = FcTrue;
	    break;
	case 's':
	    systemOnly = FcTrue;
	    break;
	case 'y':
	    sysroot = FcStrCopy ((const FcChar8 *)optarg);
	    break;
	case 'V':
	    fprintf (stderr, "fontconfig version %d.%d.%d\n", 
		     FC_MAJOR, FC_MINOR, FC_REVISION);
	    exit (0);
	case 'v':
	    verbose = FcTrue;
	    break;
	case 'h':
	    usage (argv[0], 0);
	default:
	    usage (argv[0], 1);
	}
    }
    i = optind;
#else
    i = 1;
#endif

    if (systemOnly)
	FcConfigEnableHome (FcFalse);
    if (sysroot)
    {
	FcConfigSetSysRoot (NULL, sysroot);
	FcStrFree (sysroot);
	config = FcConfigGetCurrent();
    }
    else
    {
	config = FcInitLoadConfig ();
    }
    if (!config)
    {
	fprintf (stderr, "%s: Can't init font config library\n", argv[0]);
	return 1;
    }
    FcConfigSetCurrent (config);

    if (argv[i])
    {
	dirs = FcStrSetCreate ();
	if (!dirs)
	{
	    fprintf (stderr, "%s: Can't create list of directories\n",
		     argv[0]);
	    return 1;
	}
	while (argv[i])
	{
	    if (!FcStrSetAddFilename (dirs, (FcChar8 *) argv[i]))
	    {
		fprintf (stderr, "%s: Can't add directory\n", argv[0]);
		return 1;
	    }
	    i++;
	}
	list = FcStrListCreate (dirs);
	FcStrSetDestroy (dirs);
    }
    else
	list = FcConfigGetConfigDirs (config);

    if ((processed_dirs = FcStrSetCreate()) == NULL) {
	fprintf(stderr, "Cannot malloc\n");
	return 1;
    }

    updateDirs = FcStrSetCreate ();
    changed = 0;
    ret = scanDirs (list, config, force, really_force, verbose, FcTrue, error_on_no_fonts, &changed, updateDirs);
    /* Update the directory cache again to avoid the race condition as much as possible */
    FcStrListDone (list);
    list = FcStrListCreate (updateDirs);
    if (list)
    {
	ret += scanDirs (list, config, FcTrue, FcFalse, verbose, FcFalse, error_on_no_fonts, &changed, NULL);
	FcStrListDone (list);
    }
    FcStrSetDestroy (updateDirs);

    /*
     * Try to create CACHEDIR.TAG anyway.
     * This expects the fontconfig cache directory already exists.
     * If it doesn't, it won't be simply created.
     */
    FcCacheCreateTagFile (config);

    FcStrSetDestroy (processed_dirs);

    cleanCacheDirectories (config, verbose);

    FcConfigDestroy (config);
    FcFini ();
    /* 
     * Now we need to sleep a second  (or two, to be extra sure), to make
     * sure that timestamps for changes after this run of fc-cache are later
     * then any timestamps we wrote.  We don't use gettimeofday() because
     * sleep(3) can't be interrupted by a signal here -- this isn't in the
     * library, and there aren't any signals flying around here.
     */
    /* the resolution of mtime on FAT is 2 seconds */
    if (changed)
	sleep (2);
    if (verbose)
	printf ("%s: %s\n", argv[0], ret ? "failed" : "succeeded");
    return ret;
}
