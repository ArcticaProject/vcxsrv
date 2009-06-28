/*
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of David Wexelblat not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  David Wexelblat makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

/*
 * This file is for utility functions that will be shared by other pieces
 * of the system.  Putting things here ensure that all the linking order
 * dependencies are dealt with, as this library will be linked in last.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <ctype.h>

/* To prevent empty source file warnings */
int _xf86misc;

#if 0
/* For use only with gcc */
#ifdef __GNUC__

#include "os.h"

char *
debug_alloca(char *file, int line, int size)
{
	char *ptr;

	ptr = Xalloc(size);
	ErrorF("Alloc: %s line %d; ptr = 0x%x, length = %d\n", file, line,
	       ptr, size);
	return ptr;
}

void
debug_dealloca(char *file, int line, char *ptr)
{
	ErrorF("Dealloc: %s line %d; ptr = 0x%x\n", file, line, ptr);
	Xfree(ptr);
}
#endif
#endif

#if defined(ISC) || defined(Lynx)

#include <math.h>

/* Needed for apm_driver.c */
/* These functions are modeled after the functions inside gnu's libc */

static double
copysign(double x, double y)
{
	x = fabs(x);
	return y < 0 ? - x : x;
}

double
RInt(double x)
{
	double s,t;
	const double one = 1.0;
	const static double L = 4503599627370496.0E0;

	if (x!=x)
		return(x);
	if (copysign(x,one) >= L)
		return(x);
	s = copysign(L,x);
	t = x + s;
	return (t - s);
}
#endif
