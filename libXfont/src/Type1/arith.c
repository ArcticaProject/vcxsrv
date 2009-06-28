/* $Xorg: arith.c,v 1.3 2000/08/17 19:46:29 cpqbld Exp $ */
/* Copyright International Business Machines, Corp. 1991
 * All Rights Reserved
 * Copyright Lexmark International, Inc. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM or Lexmark not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM AND LEXMARK PROVIDE THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES OF
 * ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE
 * QUALITY AND PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF THE
 * SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM OR LEXMARK) ASSUMES THE
 * ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN NO EVENT SHALL
 * IBM OR LEXMARK BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/* $XFree86: xc/lib/font/Type1/arith.c,v 1.6tsi Exp $ */

 /* ARITH    CWEB         V0006 ********                             */
/*
:h1.ARITH Module - Portable Module for Multiple Precision Fixed Point Arithmetic
 
This module provides division and multiplication of 64-bit fixed point
numbers.  (To be more precise, the module works on numbers that take
two 'longs' to store.  That is almost always equivalent to saying 64-bit
numbers.)
 
Note: it is frequently easy and desirable to recode these functions in
assembly language for the particular processor being used, because
assembly language, unlike C, will have 64-bit multiply products and
64-bit dividends.  This module is offered as a portable version.
 
&author. Jeffrey B. Lotspiech (lotspiech@almaden.ibm.com) and Sten F. Andler
 
 
:h3.Include Files
 
The included files are:
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef FONTMODULE
# include "os.h"
#endif
#include "objects.h"
#include "spaces.h"
#include "arith.h"

 
/*
:h3.
*/
/*SHARED LINE(S) ORIGINATED HERE*/
/*
Reference for all algorithms:  Donald E. Knuth, "The Art of Computer
Programming, Volume 2, Semi-Numerical Algorithms," Addison-Wesley Co.,
Massachusetts, 1969, pp. 229-279.
 
Knuth talks about a 'digit' being an arbitrary sized unit and a number
being a sequence of digits.  We'll take a digit to be a 'short'.
The following assumption must be valid for these algorithms to work:
:ol.
:li.A 'long' is two 'short's.
:eol.
The following code is INDEPENDENT of:
:ol.
:li.The actual size of a short.
:li.Whether shorts and longs are stored most significant byte
first or least significant byte first.
:eol.
 
SHORTSIZE is the number of bits in a short; LONGSIZE is the number of
bits in a long; MAXSHORT is the maximum unsigned short:
*/
/*SHARED LINE(S) ORIGINATED HERE*/
/*
ASSEMBLE concatenates two shorts to form a long:
*/
#define     ASSEMBLE(hi,lo)   ((((unsigned long)hi)<<SHORTSIZE)+(lo))
/*
HIGHDIGIT extracts the most significant short from a long; LOWDIGIT
extracts the least significant short from a long:
*/
#define     HIGHDIGIT(u)      ((u)>>SHORTSIZE)
#define     LOWDIGIT(u)       ((u)&MAXSHORT)
 
/*
SIGNBITON tests the high order bit of a long 'w':
*/
#define    SIGNBITON(w)   (((long)w)<0)
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
:h2.Double Long Arithmetic
 
:h3.DLmult() - Multiply Two Longs to Yield a Double Long
 
The two multiplicands must be positive.
*/
 
static void 
DLmult(doublelong *product, unsigned long u, unsigned long v)
{
#ifdef LONG64
/* printf("DLmult(? ?, %lx, %lx)\n", u, v); */
    *product = u*v;
/* printf("DLmult returns %lx\n", *product); */
#else
  register unsigned long u1, u2; /* the digits of u */
  register unsigned long v1, v2; /* the digits of v */
  register unsigned int w1, w2, w3, w4; /* the digits of w */
  register unsigned long t; /* temporary variable */
/* printf("DLmult(? ?, %x, %x)\n", u, v); */
  u1 = HIGHDIGIT(u);
  u2 = LOWDIGIT(u);
  v1 = HIGHDIGIT(v);
  v2 = LOWDIGIT(v);
 
  if (v2 == 0) w4 = w3 = w2 = 0;
  else
    {
    t = u2 * v2;
    w4 = LOWDIGIT(t);
    t = u1 * v2 + HIGHDIGIT(t);
    w3 = LOWDIGIT(t);
    w2 = HIGHDIGIT(t);
    }
 
  if (v1 == 0) w1 = 0;
  else
    {
    t = u2 * v1 + w3;
    w3 = LOWDIGIT(t);
    t = u1 * v1 + w2 + HIGHDIGIT(t);
    w2 = LOWDIGIT(t);
    w1 = HIGHDIGIT(t);
    }
 
  product->high = ASSEMBLE(w1, w2);
  product->low  = ASSEMBLE(w3, w4);
#endif /* LONG64 else */
}
 
/*
:h3.DLrightshift() - Macro to Shift Double Long Right by N
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
:h2.Fractional Pel Arithmetic
*/
/*
:h3.FPmult() - Multiply Two Fractional Pel Values
 
This funtion first calculates w = u * v to "doublelong" precision.
It then shifts w right by FRACTBITS bits, and checks that no
overflow will occur when the resulting value is passed back as
a fractpel.
*/
 
fractpel 
FPmult(fractpel u, fractpel v)
{
  doublelong w;
  register int negative = FALSE; /* sign flag */
#ifdef LONG64
  register fractpel ret;
#endif
 
  if ((u == 0) || (v == 0)) return (0);
 
 
  if (u < 0) {u = -u; negative = TRUE;}
  if (v < 0) {v = -v; negative = !negative;}
 
  if (u == TOFRACTPEL(1)) return ((negative) ? -v : v);
  if (v == TOFRACTPEL(1)) return ((negative) ? -u : u);
 
  DLmult(&w, u, v);
  DLrightshift(w, FRACTBITS);
#ifndef LONG64
  if (w.high != 0 || SIGNBITON(w.low)) {
        w.low = TOFRACTPEL(MAXSHORT);
  }
 
  return ((negative) ? -w.low : w.low);
#else
  if (w & 0xffffffff80000000L ) {
        ret = TOFRACTPEL(MAXSHORT);
  }
  else
        ret = (fractpel)w;
 
  return ((negative) ? -ret : ret);
#endif
}
