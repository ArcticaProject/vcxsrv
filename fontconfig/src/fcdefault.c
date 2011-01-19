/*
 * fontconfig/src/fcdefault.c
 *
 * Copyright © 2001 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "fcint.h"
#include <locale.h>

static const struct {
    FcObject	field;
    FcBool	value;
} FcBoolDefaults[] = {
    { FC_HINTING_OBJECT,	   FcTrue	},  /* !FT_LOAD_NO_HINTING */
    { FC_VERTICAL_LAYOUT_OBJECT,   FcFalse	},  /* FC_LOAD_VERTICAL_LAYOUT */
    { FC_AUTOHINT_OBJECT,	   FcFalse	},  /* FC_LOAD_FORCE_AUTOHINT */
    { FC_GLOBAL_ADVANCE_OBJECT,    FcTrue	},  /* !FC_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH */
    { FC_EMBEDDED_BITMAP_OBJECT,   FcTrue 	},  /* !FC_LOAD_NO_BITMAP */
    { FC_DECORATIVE_OBJECT,	   FcFalse	},
};

#define NUM_FC_BOOL_DEFAULTS	(int) (sizeof FcBoolDefaults / sizeof FcBoolDefaults[0])

FcChar8 *
FcGetDefaultLang (void)
{
    static char	lang_local [128] = {0};
    char        *ctype;
    char        *territory;
    char        *after;
    int         lang_len, territory_len;

    if (lang_local [0])
	return (FcChar8 *) lang_local;

    ctype = setlocale (LC_CTYPE, NULL);

    /*
     * Check if setlocale (LC_ALL, "") has been called
     */
    if (!ctype || !strcmp (ctype, "C"))
    {
	ctype = getenv ("LC_ALL");
	if (!ctype)
	{
	    ctype = getenv ("LC_CTYPE");
	    if (!ctype)
		ctype = getenv ("LANG");
	}
    }

    /* ignore missing or empty ctype */
    if (ctype && *ctype != '\0')
    {
	territory = strchr (ctype, '_');
	if (territory)
	{
	    lang_len = territory - ctype;
	    territory = territory + 1;
	    after = strchr (territory, '.');
	    if (!after)
	    {
		after = strchr (territory, '@');
		if (!after)
		    after = territory + strlen (territory);
	    }
	    territory_len = after - territory;
	    if (lang_len + 1 + territory_len + 1 <= (int) sizeof (lang_local))
	    {
		strncpy (lang_local, ctype, lang_len);
		lang_local[lang_len] = '-';
		strncpy (lang_local + lang_len + 1, territory, territory_len);
		lang_local[lang_len + 1 + territory_len] = '\0';
	    }
	}
	else
	{
	    after = strchr (ctype, '.');
	    if (!after)
	    {
		after = strchr (ctype, '@');
		if (!after)
		    after = ctype + strlen (ctype);
	    }
	    lang_len = after - ctype;
	    if (lang_len + 1 <= (int) sizeof (lang_local))
	    {
		strncpy (lang_local, ctype, lang_len);
		lang_local[lang_len] = '\0';
	    }
	}
    }

    /* set default lang to en */
    if (!lang_local [0])
	strcpy (lang_local, "en");

    return (FcChar8 *) lang_local;
}

void
FcDefaultSubstitute (FcPattern *pattern)
{
    FcValue v;
    int	    i;

    if (FcPatternObjectGet (pattern, FC_WEIGHT_OBJECT, 0, &v) == FcResultNoMatch )
	FcPatternObjectAddInteger (pattern, FC_WEIGHT_OBJECT, FC_WEIGHT_MEDIUM);

    if (FcPatternObjectGet (pattern, FC_SLANT_OBJECT, 0, &v) == FcResultNoMatch)
	FcPatternObjectAddInteger (pattern, FC_SLANT_OBJECT, FC_SLANT_ROMAN);

    if (FcPatternObjectGet (pattern, FC_WIDTH_OBJECT, 0, &v) == FcResultNoMatch)
	FcPatternObjectAddInteger (pattern, FC_WIDTH_OBJECT, FC_WIDTH_NORMAL);

    for (i = 0; i < NUM_FC_BOOL_DEFAULTS; i++)
	if (FcPatternObjectGet (pattern, FcBoolDefaults[i].field, 0, &v) == FcResultNoMatch)
	    FcPatternObjectAddBool (pattern, FcBoolDefaults[i].field, FcBoolDefaults[i].value);
    
    if (FcPatternObjectGet (pattern, FC_PIXEL_SIZE_OBJECT, 0, &v) == FcResultNoMatch)
    {
	double	dpi, size, scale;

	if (FcPatternObjectGetDouble (pattern, FC_SIZE_OBJECT, 0, &size) != FcResultMatch)
	{
	    size = 12.0;
	    (void) FcPatternObjectDel (pattern, FC_SIZE_OBJECT);
	    FcPatternObjectAddDouble (pattern, FC_SIZE_OBJECT, size);
	}
	if (FcPatternObjectGetDouble (pattern, FC_SCALE_OBJECT, 0, &scale) != FcResultMatch)
	{
	    scale = 1.0;
	    (void) FcPatternObjectDel (pattern, FC_SCALE_OBJECT);
	    FcPatternObjectAddDouble (pattern, FC_SCALE_OBJECT, scale);
	}
	size *= scale;
	if (FcPatternObjectGetDouble (pattern, FC_DPI_OBJECT, 0, &dpi) != FcResultMatch)
	{
	    dpi = 75.0;
	    (void) FcPatternObjectDel (pattern, FC_DPI_OBJECT);
	    FcPatternObjectAddDouble (pattern, FC_DPI_OBJECT, dpi);
	}
	size *= dpi / 72.0;
	FcPatternObjectAddDouble (pattern, FC_PIXEL_SIZE_OBJECT, size);
    }

    if (FcPatternObjectGet (pattern, FC_LANG_OBJECT, 0, &v) == FcResultNoMatch)
    {
 	FcPatternObjectAddString (pattern, FC_LANG_OBJECT, FcGetDefaultLang ());
    }
    if (FcPatternObjectGet (pattern, FC_FONTVERSION_OBJECT, 0, &v) == FcResultNoMatch)
    {
	FcPatternObjectAddInteger (pattern, FC_FONTVERSION_OBJECT, 0x7fffffff);
    }

    if (FcPatternObjectGet (pattern, FC_HINT_STYLE_OBJECT, 0, &v) == FcResultNoMatch)
    {
	FcPatternObjectAddInteger (pattern, FC_HINT_STYLE_OBJECT, FC_HINT_FULL);
    }
}
#define __fcdefault__
#include "fcaliastail.h"
#undef __fcdefault__
