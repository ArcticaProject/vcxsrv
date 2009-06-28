/***********************************************************

Copyright 1987, 1998  The Open Group

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/keysym.h>
#include "os.h"

typedef struct _builtinColor {
    unsigned char	red;
    unsigned char	green;
    unsigned char	blue;
    unsigned short	name;
} BuiltinColor;

/* These have to come after the struct definition because despair. */
#include "oscolor.h"
#define NUM_BUILTIN_COLORS  (sizeof (BuiltinColors) / sizeof (BuiltinColors[0]))

static unsigned char
OsToLower (unsigned char a)
{
    if ((a >= XK_A) && (a <= XK_Z))
	return a + (XK_a - XK_A);
    else if ((a >= XK_Agrave) && (a <= XK_Odiaeresis))
	return a + (XK_agrave - XK_Agrave);
    else if ((a >= XK_Ooblique) && (a <= XK_Thorn))
	return a + (XK_oslash - XK_Ooblique);
    else
	return a;
}

static int
OsStrCaseCmp (const unsigned char *s1, const unsigned char *s2, int l2)
{
    unsigned char   c1, c2;

    for (;;)
    {
	c1 = OsToLower (*s1++);
	if (l2 == 0)
	    c2 = '\0';
	else
	    c2 = OsToLower (*s2++);
	if (!c1 || !c2)
	    break;
	if (c1 != c2)
	    break;
	l2--;
    }
    return c2 - c1;
}

Bool
OsInitColors(void)
{
    return TRUE;
}

Bool
OsLookupColor(int		screen, 
	      char		*s_name,
	      unsigned int	len, 
	      unsigned short	*pred,
	      unsigned short	*pgreen,
	      unsigned short	*pblue)
{
    const BuiltinColor	*c;
    unsigned char	*name = (unsigned char *) s_name;
    int			low, mid, high;
    int			r;

    low = 0;
    high = NUM_BUILTIN_COLORS - 1;
    while (high >= low)
    {
	mid = (low + high) / 2;
	c = &BuiltinColors[mid];
	r = OsStrCaseCmp (&BuiltinColorNames[c->name], name, len);
	if (r == 0)
	{
	    *pred = c->red * 0x101;
	    *pgreen = c->green * 0x101;
	    *pblue = c->blue * 0x101;
	    return TRUE;
	}
	if (r < 0)
	    high = mid - 1;
	else
	    low = mid + 1;
    }
    return FALSE;
}
