/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xegl.h"

static xglScreenInfoRec xglScreenInfo = {
    NULL, 0, 0, 0, 0,
    DEFAULT_GEOMETRY_DATA_TYPE,
    DEFAULT_GEOMETRY_USAGE,
    FALSE,
    XGL_DEFAULT_PBO_MASK,
    FALSE,
    FALSE
};

#ifdef GLXEXT
static Bool loadGlx = TRUE;
#endif

void
InitOutput (ScreenInfo *pScreenInfo,
	    int	       argc,
	    char       **argv)
{

#ifdef GLXEXT
    if (loadGlx)
    {
	if (!xglLoadGLXModules ())
	    FatalError ("No GLX modules loaded");
    }
#endif

    xeglInitOutput (pScreenInfo, argc, argv);
}

Bool
LegalModifier (unsigned int key,
	       DeviceIntPtr pDev)
{
    return xeglLegalModifier (key, pDev);
}

void
ProcessInputEvents (void)
{
    xeglProcessInputEvents ();
}

void
InitInput (int  argc,
	   char **argv)
{
    xeglInitInput (argc, argv);
}

void
ddxUseMsg (void)
{
    ErrorF ("\nXgl usage:\n");

#ifdef GLXEXT
    ErrorF ("-noglx                 don't load glx extension\n");
#endif

    xglUseMsg ();
    ErrorF ("\nXegl usage:\n");
    xeglUseMsg ();
}

int
ddxProcessArgument (int  argc,
		    char **argv,
		    int  i)
{
    int skip;

#ifdef GLXEXT
    if (!strcmp (argv[i], "-noglx"))
    {
	loadGlx = FALSE;
	return 1;
    }
#endif

    skip = xglProcessArgument (argc, argv, i);
    if (skip)
	return skip;

    return xeglProcessArgument (argc, argv, i);
}

void
AbortDDX (void)
{
    xeglAbort ();
}

void
ddxGiveUp (void)
{
    xeglGiveUp ();
}

void
OsVendorInit (void)
{
    xeglOsVendorInit ();
}
