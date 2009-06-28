/* $Xorg: miscutil.c,v 1.4 2001/02/09 02:04:04 xorgcvs Exp $ */

/*

Copyright 1991, 1994, 1998  The Open Group

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
/* $XFree86: xc/lib/font/util/miscutil.c,v 1.7 2001/07/25 15:04:57 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <X11/Xosdefs.h>
#include <stdlib.h>
#include <X11/fonts/fontmisc.h>
#include "stubs.h"

#define XK_LATIN1
#include    <X11/keysymdef.h>


#ifdef __SUNPRO_C
#pragma weak serverGeneration
#pragma weak Xalloc
#pragma weak Xrealloc
#pragma weak Xfree
#pragma weak Xcalloc
#pragma weak CopyISOLatin1Lowered
#pragma weak register_fpe_functions
#endif

/* make sure everything initializes themselves at least once */
weak long serverGeneration = 1;

weak void *
Xalloc (unsigned long m)
{
    return malloc (m);
}

weak void *
Xrealloc (void *n, unsigned long m)
{
    if (!n)
	return malloc (m);
    else
	return realloc (n, m);
}

weak void
Xfree (void *n)
{
    if (n)
	free (n);
}

weak void *
Xcalloc (unsigned long n)
{
    return calloc (n, 1);
}

weak void
CopyISOLatin1Lowered (char *dst, char *src, int len)
{
    register unsigned char *dest, *source;

    for (dest = (unsigned char *)dst, source = (unsigned char *)src;
	 *source && len > 0;
	 source++, dest++, len--)
    {
	if ((*source >= XK_A) && (*source <= XK_Z))
	    *dest = *source + (XK_a - XK_A);
	else if ((*source >= XK_Agrave) && (*source <= XK_Odiaeresis))
	    *dest = *source + (XK_agrave - XK_Agrave);
	else if ((*source >= XK_Ooblique) && (*source <= XK_Thorn))
	    *dest = *source + (XK_oslash - XK_Ooblique);
	else
	    *dest = *source;
    }
    *dest = '\0';
}

weak void
register_fpe_functions (void)
{
}
