/* $Xorg: register.c,v 1.4 2001/02/09 02:04:03 xorgcvs Exp $ */

/*

Copyright 1994, 1998  The Open Group

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
/* $XFree86: xc/lib/font/fontfile/register.c,v 1.14 2001/01/17 19:43:30 dawes Exp $ */

/*
 * This is in a separate source file so that small programs
 * such as mkfontdir that want to use the fontfile utilities don't
 * end up dragging in code from all the renderers, which is not small.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#define XFONT_BITMAP 1
#endif

#include <X11/fonts/fontmisc.h>
#include <X11/fonts/fntfilst.h>
#include <X11/fonts/bitmap.h>
#include <X11/fonts/fontmod.h>

/*
 * Translate monolithic build symbols to modular build symbols.
 * I chose to make the modular symbols 'canonical' because they
 * are prefixed with XFONT_, neatly avoiding name collisions
 * with other packages.
 */

#ifndef CRAY
# ifdef BUILD_SPEEDO
#  define XFONT_SPEEDO 1
# endif
# ifdef BUILD_TYPE1
#  define XFONT_TYPE1 1
# endif
#endif

#ifdef BUILD_FREETYPE
# define XFONT_FREETYPE 1
#endif

/* Font renderers to initialize when not linked into something like
   Xorg that provides its own module configuration options */
static const FontModule builtinFontModuleList[] = {
#ifdef XFONT_SPEEDO
    {
	SpeedoRegisterFontFileFunctions,
	"speedo",
	NULL
    },
#endif
#ifdef XFONT_TYPE1
    {
	Type1RegisterFontFileFunctions,
	"type1",
	NULL
    },
#endif
#ifdef XFONT_FREETYPE    
    {
	FreeTypeRegisterFontFileFunctions,
	"freetype",
	NULL
    },
#endif
    /* List terminator - must be last entry */
    {	NULL, NULL, NULL }
};

void
FontFileRegisterFpeFunctions(void)
{
    const FontModule *fmlist = builtinFontModuleList;

#ifdef XFONT_BITMAP
    /* bitmap is always builtin to libXfont now */
    BitmapRegisterFontFileFunctions ();
#endif

#ifdef LOADABLEFONTS
    if (FontModuleList) {
	fmlist = FontModuleList;
    }
#endif    

    if (fmlist) {
	int i;

	for (i = 0; fmlist[i].name; i++) {
	    if (fmlist[i].initFunc) {
		fmlist[i].initFunc();
	    }
	}
    }
    
    FontFileRegisterLocalFpeFunctions ();
    CatalogueRegisterLocalFpeFunctions ();
}

