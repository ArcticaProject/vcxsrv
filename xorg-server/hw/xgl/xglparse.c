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

#include "xgl.h"

char *
xglParseFindNext (char *cur,
		  char *delim,
		  char *save,
		  char *last)
{
    while (*cur && !strchr (delim, *cur))
	*save++ = *cur++;

    *save = 0;
    *last = *cur;

    if (*cur)
	cur++;

    return cur;
}

void
xglParseScreen (char *arg)
{
    char delim;
    char save[1024];
    int	 i, pixels, mm;

    xglScreenInfo.width    = 0;
    xglScreenInfo.height   = 0;
    xglScreenInfo.widthMm  = 0;
    xglScreenInfo.heightMm = 0;

    if (!arg)
	return;

    if (strlen (arg) >= sizeof (save))
	return;

    for (i = 0; i < 2; i++)
    {
	arg = xglParseFindNext (arg, "x/", save, &delim);
	if (!save[0])
	    return;

	pixels = atoi (save);
	mm = 0;

	if (delim == '/')
	{
	    arg = xglParseFindNext (arg, "x", save, &delim);
	    if (!save[0])
		return;

	    mm = atoi (save);
	}

	if (i == 0)
	{
	    xglScreenInfo.width   = pixels;
	    xglScreenInfo.widthMm = mm;
	}
	else
	{
	    xglScreenInfo.height   = pixels;
	    xglScreenInfo.heightMm = mm;
	}

	if (delim != 'x')
	    return;
    }
}

static void
xglParseAccel (char *arg)
{
    xglAccelInfoPtr pAccel;
    char	    delim;
    char	    save[1024];

    if (!arg)
	return;

    if (strlen (arg) >= sizeof (save))
	return;

    arg = xglParseFindNext (arg, "@:", save, &delim);
    if (!save[0])
	return;

    if (strcasecmp (save, "pixmap") == 0)
	pAccel = &xglScreenInfo.accel.pixmap;
    else if (strcasecmp (save, "window") == 0)
	pAccel = &xglScreenInfo.accel.window;
    else if (strcasecmp (save, "glx") == 0)
	pAccel = &xglScreenInfo.accel.glx;
    else if (strcasecmp (save, "xv") == 0)
	pAccel = &xglScreenInfo.accel.xv;
    else
	return;

    if (delim == '@')
    {
	arg = xglParseFindNext (arg, "/x:", save, &delim);
	if (!save[0])
	    return;

	pAccel->size.aboveWidth = pAccel->size.minWidth = atoi (save);

	if (delim == '/')
	{
	    arg = xglParseFindNext (arg, "x:", save, &delim);
	    if (!save[0])
		return;

	    pAccel->size.aboveWidth = atoi (save);
	}

	if (delim == 'x')
	{
	    arg = xglParseFindNext (arg, "/:", save, &delim);
	    if (!save[0])
		return;

	    pAccel->size.aboveHeight = pAccel->size.minHeight = atoi (save);

	    if (delim == '/')
	    {
		arg = xglParseFindNext (arg, ":", save, &delim);
		if (!save[0])
		    return;

		pAccel->size.aboveHeight = atoi (save);
	    }
	}
    }

    pAccel->enabled = TRUE;
    pAccel->pbuffer = FALSE;

    if (delim == ':')
    {
	if (strcasecmp (arg, "fbo") == 0)
	    ;
	else if (strcasecmp  (arg, "off")  == 0 ||
		 strncasecmp (arg, "0", 1) == 0 ||
		 strncasecmp (arg, "f", 1) == 0 ||
		 strncasecmp (arg, "n", 1) == 0)
	{
	    pAccel->enabled = FALSE;
	    pAccel->pbuffer = FALSE;
	}
	else if (strcasecmp (arg, "pbuffer") == 0)
	{
	    pAccel->pbuffer = TRUE;
	}
    }
}

void
xglUseMsg (void)
{
    ErrorF ("-vertextype [short|float] set vertex data type\n");
    ErrorF ("-yinverted             Y is upside-down\n");
    ErrorF ("-lines                 "
	    "accelerate lines that are not vertical or horizontal\n");
    ErrorF ("-vbo                   "
	    "use vertex buffer objects for streaming of vertex data\n");
    ErrorF ("-pbomask [1|4|8|16|32] "
	    "set bpp's to use with pixel buffer objects\n");
    ErrorF ("-accel TYPE[@WIDTH[/MIN]xHEIGHT[/MIN]][:METHOD] "
	    "offscreen acceleration\n");
}

int
xglProcessArgument (int	 argc,
		    char **argv,
		    int	 i)
{
    if (!strcmp (argv[i], "-vertextype"))
    {
	if ((i + 1) < argc)
	{
	    if (!strcasecmp (argv[i + 1], "short"))
		xglScreenInfo.geometryDataType = GEOMETRY_DATA_TYPE_SHORT;
	    else if (!strcasecmp (argv[i + 1], "float"))
		xglScreenInfo.geometryDataType = GEOMETRY_DATA_TYPE_FLOAT;
	}
	else
	    return 1;

	return 2;
    }
    else if (!strcmp (argv[i], "-yinverted"))
    {
	xglScreenInfo.yInverted = TRUE;
	return 1;
    }
    else if (!strcmp (argv[i], "-lines"))
    {
	xglScreenInfo.lines = TRUE;
	return 1;
    }
    else if (!strcmp (argv[i], "-vbo"))
    {
	xglScreenInfo.geometryUsage = GEOMETRY_USAGE_STREAM;
	return 1;
    }
    else if (!strcmp (argv[i], "-pbomask"))
    {
	if ((i + 1) < argc)
	{
	    xglScreenInfo.pboMask = atoi (argv[i + 1]);
	}
	else
	    return 1;

	return 2;
    }
    else if (!strcmp (argv[i], "-accel"))
    {
	if ((i + 1) < argc)
	{
	    xglParseAccel (argv[i + 1]);
	}
	else
	    return 1;

	return 2;
    }

    return 0;
}
