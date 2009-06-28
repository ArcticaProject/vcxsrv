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

#include "xgl.h"
#include "xglmodule.h"

#ifdef XGL_MODULAR

#include <dlfcn.h>

#define SYM(ptr, name) { (void **) &(ptr), (name) }

void *
xglLoadModule (const char *name,
	       int	  flag)
{
    ModuleVersionProcPtr moduleVersion;
    ModuleInitProcPtr    moduleInit;
    void	         *handle = 0;
    char	         *module;
    xglSymbolRec         mSym[] = {
	SYM (moduleVersion, "moduleVersion"),
	SYM (moduleInit,    "moduleInit")
    };

    module = malloc (strlen (XGL_MODULE_PATH "/lib.so") + strlen (name) + 1);
    if (!module)
	return 0;

    sprintf (module, XGL_MODULE_PATH "/lib%s.so", name);

    handle = dlopen (module, flag);
    if (handle)
    {
	if (xglLookupSymbols (handle, mSym, sizeof (mSym) / sizeof (mSym[0])))
	{
	    const char *version;

	    version = (*moduleVersion) ();
	    if (strcmp (VERSION, version) == 0)
	    {
		if (!(*moduleInit) (module))
		{
		    dlclose (handle);
		    handle = 0;
		}
	    }
	    else
	    {
		ErrorF ("Module version mismatch. "
			"%s is %s Xserver is" VERSION "\n",
			module, version);

		dlclose (handle);
		handle = 0;
	    }
	}
	else
	{
	    dlclose (handle);
	    handle = 0;
	}
    }
    else
	ErrorF ("dlopen: %s\n", dlerror ());

    free (module);

    return handle;
}

void
xglUnloadModule (void *handle)
{
    dlclose (handle);
}

Bool
xglLookupSymbols (void         *handle,
		  xglSymbolPtr sym,
		  int	       nSym)
{
    void *symbol;
    char *error;
    int  i;

    /* avoid previous error */
    dlerror ();

    for (i = 0; i < nSym; i++)
    {
	symbol = dlsym (handle, sym[i].name);
	if (!symbol)
	{
	    error = dlerror ();
	    if (error != 0)
		ErrorF ("dlsym: %s: %s\n", sym[i].name, error);

	    return FALSE;
	}

	*(sym[i].ptr) = symbol;
    }

    return TRUE;
}

#endif
