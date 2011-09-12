/*
 * fontconfig/src/fcblanks.c
 *
 * Copyright © 2002 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the author(s) not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
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

FcBlanks *
FcBlanksCreate (void)
{
    FcBlanks	*b;

    b = malloc (sizeof (FcBlanks));
    if (!b)
	return 0;
    FcMemAlloc (FC_MEM_BLANKS, sizeof (FcBlanks));
    b->nblank = 0;
    b->sblank = 0;
    b->blanks = 0;
    return b;
}

void
FcBlanksDestroy (FcBlanks *b)
{
    if (b->blanks)
    {
	FcMemFree (FC_MEM_BLANKS, b->sblank * sizeof (FcChar32));
	free (b->blanks);
    }
    FcMemFree (FC_MEM_BLANKS, sizeof (FcBlanks));
    free (b);
}

FcBool
FcBlanksAdd (FcBlanks *b, FcChar32 ucs4)
{
    FcChar32	*c;
    int		sblank;

    for (sblank = 0; sblank < b->nblank; sblank++)
	if (b->blanks[sblank] == ucs4)
	    return FcTrue;

    if (b->nblank == b->sblank)
    {
	sblank = b->sblank + 32;
	if (b->blanks)
	    c = (FcChar32 *) realloc (b->blanks, sblank * sizeof (FcChar32));
	else
	    c = (FcChar32 *) malloc (sblank * sizeof (FcChar32));
	if (!c)
	    return FcFalse;
	if (b->sblank)
	    FcMemFree (FC_MEM_BLANKS, b->sblank * sizeof (FcChar32));
	FcMemAlloc (FC_MEM_BLANKS, sblank * sizeof (FcChar32));
	b->sblank = sblank;
	b->blanks = c;
    }
    b->blanks[b->nblank++] = ucs4;
    return FcTrue;
}

FcBool
FcBlanksIsMember (FcBlanks *b, FcChar32 ucs4)
{
    int	i;

    for (i = 0; i < b->nblank; i++)
	if (b->blanks[i] == ucs4)
	    return FcTrue;
    return FcFalse;
}
#define __fcblanks__
#include "fcaliastail.h"
#undef __fcblanks__
