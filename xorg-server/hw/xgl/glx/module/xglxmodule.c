/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xglx.h"
#include "xglmodule.h"

#include <glitz-glx.h>

char *
moduleVersion (void)
{
    return VERSION;
}

Bool
moduleInit (const char *module)
{
    glitz_glx_init (module);

    return TRUE;
}

void
InitOutput (ScreenInfo *pScreenInfo,
	    int	       argc,
	    char       **argv)
{
    xglxInitOutput (pScreenInfo, argc, argv);
}

Bool
LegalModifier (unsigned int key,
	       DeviceIntPtr pDev)
{
    return xglxLegalModifier (key, pDev);
}

void
ProcessInputEvents (void)
{
    xglxProcessInputEvents ();
}

void
InitInput (int	argc,
	   char	**argv)
{
    xglxInitInput (argc, argv);
}

void
ddxUseMsg (void)
{
    ErrorF ("\nXglx usage:\n");
    xglxUseMsg ();
}

int
ddxProcessArgument (int	 argc,
		    char **argv,
		    int	 i)
{
    return xglxProcessArgument (argc, argv, i);
}

void
AbortDDX (void)
{
    xglxAbort ();
}

void
ddxGiveUp (void)
{
    xglxGiveUp ();
}

void
OsVendorInit (void)
{
    xglxOsVendorInit ();
}
