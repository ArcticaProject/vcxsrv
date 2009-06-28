/* 
 * 
 * Copyright (c) 1997  Metro Link Incorporated
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 * 
 */

/* View/edit this file with tab stops set to 4 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "xf86Parser.h"
#include "configProcs.h"

#ifdef MALLOC_FUNCTIONS
void
xfree (void *p)
{
	free (p);
	return;
}

void *
xalloc (int size)
{
	return malloc (size);
}

void *
xrealloc (void *p, int size)
{
	return realloc (p, size);
}

#endif

#define CONFPATH "%A,%R,/etc/X11/%R,%P/etc/X11/%R,%E,%F,/etc/X11/%F," \
		 "%P/etc/X11/%F,/etc/X11/%X,/etc/%X,%P/etc/X11/%X.%H," \
		 "%P/etc/X11/%X,%P/lib/X11/%X.%H,%P/lib/X11/%X"

int
main (int argc, char *argv[])
{
	const char *filename;
	char *cmdline = NULL;
	XF86ConfigPtr conf;

	if (argc > 1)
	{
		cmdline = argv[1];
	}
	if ((filename = xf86openConfigFile (CONFPATH, cmdline, NULL)))
	{
		fprintf (stderr, "Opened %s for the config file\n", filename);
	}
	else
	{
		fprintf (stderr, "Unable to open config file\n");
		exit (1);
	}

	if ((conf = xf86readConfigFile ()) == NULL)
	{
		fprintf (stderr, "Problem when parsing config file\n");
	}
	else
	{
		fprintf (stderr, "Config file parsed OK\n");
	}
	xf86closeConfigFile ();

	if (argc > 2) {
		fprintf(stderr, "Writing config file to `%s'\n", argv[2]);
		xf86writeConfigFile (argv[2], conf);
	}
	exit(0);
}

/* Functions that the parser requires */

_X_EXPORT void
VErrorF(const char *f, va_list args)
{
	vfprintf(stderr, f, args);
}

_X_EXPORT void
ErrorF(const char *f, ...)
{
	va_list args;

	va_start(args, f);
	vfprintf(stderr, f, args);
	va_end(args);
}
