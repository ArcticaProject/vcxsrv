/*
 * Copyright 1999 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include    <X11/fonts/fntfilst.h>
#include "builtin.h"

static int  font_file_type;

static const char builtin_fonts[] = "built-ins";

static int
BuiltinNameCheck (char *name)
{
    return (strcmp (name, builtin_fonts) == 0);
}

static int
BuiltinInitFPE (FontPathElementPtr fpe)
{
    int			status;
    FontDirectoryPtr	dir;

    status = BuiltinReadDirectory (fpe->name, &dir);

    if (status == Successful)
	fpe->private = (pointer) dir;
    return status;
}

/* ARGSUSED */
static int
BuiltinResetFPE (FontPathElementPtr fpe)
{
    FontDirectoryPtr	dir;

    dir = (FontDirectoryPtr) fpe->private;
    /* builtins can't change! */
    return Successful;
}

static int
BuiltinFreeFPE (FontPathElementPtr fpe)
{
    FontFileFreeDir ((FontDirectoryPtr) fpe->private);
    return Successful;
}

void
BuiltinRegisterFpeFunctions(void)
{
    BuiltinRegisterFontFileFunctions ();

    font_file_type = RegisterFPEFunctions(BuiltinNameCheck,
					  BuiltinInitFPE,
					  BuiltinFreeFPE,
					  BuiltinResetFPE,
					  FontFileOpenFont,
					  FontFileCloseFont,
					  FontFileListFonts,
					  FontFileStartListFontsWithInfo,
					  FontFileListNextFontWithInfo,
					  (WakeupFpeFunc) 0,
					  (ClientDiedFunc) 0,
					  (LoadGlyphsFunc) 0,
					  (StartLaFunc) 0,
					  (NextLaFunc) 0,
					  (SetPathFunc) 0);
}
