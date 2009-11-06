/* $XFree86: xc/lib/Xaw/OS.c,v 1.1 1998/12/06 10:44:34 dawes Exp $ */

/* Some OS-dependent utility code */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xosdefs.h>
#include <X11/IntrinsicP.h>
#include "Private.h"

#ifndef X_NOT_POSIX
#include <unistd.h>	/* for sysconf(), and getpagesize() */
#endif

#if defined(linux)
/* kernel header doesn't work with -ansi */
/* #include <asm/page.h> *//* for PAGE_SIZE */
#define HAS_GETPAGESIZE
#define HAS_SC_PAGESIZE	/* _SC_PAGESIZE may be an enum for Linux */
#endif

#if defined(CSRG_BASED)
#define HAS_GETPAGESIZE
#endif

#if defined(sun)
#define HAS_GETPAGESIZE
#endif

int
_XawGetPageSize(void)
{
    static int pagesize = -1;

    if (pagesize != -1)
	return pagesize;

    /* Try each supported method in the preferred order */

#if defined(_SC_PAGESIZE) || defined(HAS_SC_PAGESIZE)
    pagesize = sysconf(_SC_PAGESIZE);
#endif

#ifdef _SC_PAGE_SIZE
    if (pagesize == -1)
	pagesize = sysconf(_SC_PAGE_SIZE);
#endif

#ifdef HAS_GETPAGESIZE
    if (pagesize == -1)
	pagesize = getpagesize();
#endif

#ifdef PAGE_SIZE
    if (pagesize == -1)
	pagesize = PAGE_SIZE;
#endif

    if (pagesize == -1)
	pagesize = 0;

    return pagesize;
}
