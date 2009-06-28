/* $XFree86: xc/lib/font/stubs/stubs.h,v 1.3 1999/12/15 01:14:36 robin Exp $ */

#include <stdio.h>
#include <X11/fonts/fntfilst.h>
#include <X11/fonts/font.h>

#ifndef True
#define True (-1)
#endif
#ifndef False
#define False (0)
#endif

/* this probably works for Mach-O too, but probably not for PE */
#if defined(__ELF__) && defined(__GNUC__) && (__GNUC__ >= 3)
#define weak __attribute__((weak))
#else
#define weak
#endif

extern FontPtr find_old_font ( FSID id );
extern int set_font_authorizations ( char **authorizations, 
				     int *authlen, 
				     ClientPtr client );

extern unsigned long GetTimeInMillis (void);

extern void ErrorF(const char *format, ...);
extern void FatalError(const char *format, ...);

/* end of file */
