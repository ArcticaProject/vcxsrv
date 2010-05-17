/* $XConsortium: actions.h,v 1.6 94/04/17 20:43:31 rws Exp $ */
/*

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/* 
 * actions.h - action table declaring externally available procedures for xcalc
 *
 * Author:  Donna Converse, MIT X Consortium
 */

extern void
    add(Widget,XEvent*,String*,Cardinal*), back(Widget,XEvent*,String*,Cardinal*), bell(Widget,XEvent*,String*,Cardinal*), clearit(Widget,XEvent*,String*,Cardinal*), cosine(Widget,XEvent*,String*,Cardinal*), decimal(Widget,XEvent*,String*,Cardinal*),
    degree(Widget,XEvent*,String*,Cardinal*), digit(Widget,XEvent*,String*,Cardinal*), divide(Widget,XEvent*,String*,Cardinal*), e(Widget,XEvent*,String*,Cardinal*), enter(Widget,XEvent*,String*,Cardinal*), epower(Widget,XEvent*,String*,Cardinal*), equal(Widget,XEvent*,String*,Cardinal*),
    exchange(Widget,XEvent*,String*,Cardinal*), factorial(Widget,XEvent*,String*,Cardinal*), 
    inverse(Widget,XEvent*,String*,Cardinal*), leftParen(Widget,XEvent*,String*,Cardinal*), logarithm(Widget,XEvent*,String*,Cardinal*), multiply(Widget,XEvent*,String*,Cardinal*), naturalLog(Widget,XEvent*,String*,Cardinal*),
    negate(Widget,XEvent*,String*,Cardinal*), nop(Widget,XEvent*,String*,Cardinal*), off(Widget,XEvent*,String*,Cardinal*), pi(Widget,XEvent*,String*,Cardinal*), power(Widget,XEvent*,String*,Cardinal*), quit(Widget,XEvent*,String*,Cardinal*), recall(Widget,XEvent*,String*,Cardinal*),
    reciprocal(Widget,XEvent*,String*,Cardinal*), rightParen(Widget,XEvent*,String*,Cardinal*), roll(Widget,XEvent*,String*,Cardinal*), scientific(Widget,XEvent*,String*,Cardinal*), selection(Widget,XEvent*,String*,Cardinal*), sine(Widget,XEvent*,String*,Cardinal*),
    square(Widget,XEvent*,String*,Cardinal*), squareRoot(Widget,XEvent*,String*,Cardinal*), store(Widget,XEvent*,String*,Cardinal*), subtract(Widget,XEvent*,String*,Cardinal*), sum(Widget,XEvent*,String*,Cardinal*),
    tangent(Widget,XEvent*,String*,Cardinal*), tenpower(Widget,XEvent*,String*,Cardinal*), XexchangeY(Widget,XEvent*,String*,Cardinal*);

/*
 * 	calculator action table
 */

XtActionsRec	Actions[] = {
{"add",		add},		/* addition */
{"back",	back},		/* HP-specific backspace */
{"bell",	bell},		/* ring bell */
{"clear",	clearit},	/* TI-specific clear calculator state */
{"cosine",	cosine},	/* trigonometric function cosine */
{"decimal",	decimal},	/* decimal point */
{"degree",	degree},	/* degree, radian, grad switch */
{"digit",	digit},		/* numeric key */
{"divide",	divide},	/* division */
{"e",		e},		/* the natural number e */
{"enter",	enter},		/* HP-specific enter */
{"epower",	epower},	/* e raised to a power */
{"equal",	equal},		/* TI-specific = */
{"exchange",	exchange},	/* TI-specific exchange memory and display */
{"factorial",	factorial},	/* factorial function */
{"inverse", 	inverse},	/* inverse */
{"leftParen",	leftParen},	/* TI-specific left parenthesis */
{"logarithm",	logarithm},	/* logarithm base 10 */
{"multiply",	multiply},	/* multiplication */
{"naturalLog",	naturalLog},	/* natural logarithm base e */
{"negate",	negate},	/* change sign */
{"nop",		nop},		/* no operation, rings bell */
{"off",		off},		/* clear state */
{"pi",		pi},		/* the number pi */
{"power",	power},		/* raise to an arbitrary power */
{"quit",	quit},		/* quit */
{"recall",	recall},	/* memory recall */
{"reciprocal",  reciprocal},	/* reciprocal function */
{"rightParen",	rightParen},	/* TI-specific left parenthesis */
{"roll",	roll},		/* HP-specific roll stack */
{"scientific",	scientific},	/* scientfic notation (EE) */
{"selection",	selection},	/* copy selection */
{"sine",	sine},		/* trigonometric function sine */
{"square",	square},	/* square */
{"squareRoot",	squareRoot},	/* square root */
{"store",	store},		/* memory store */
{"subtract", 	subtract},	/* subtraction */
{"sum",		sum},		/* memory summation */
{"tangent",	tangent},	/* trigonometric function tangent */
{"tenpower",	tenpower},	/* 10 raised to to an arbitrary power */
{"XexchangeY",	XexchangeY}	/* HP-specific exchange X and Y registers */
};
