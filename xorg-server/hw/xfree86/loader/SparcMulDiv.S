/*
 * Hardware integer division and multiplication routines for SPARC v8 and higher.
 *
 * Copyright (C) 1996 David S. Miller (davem@redhat.com)
 * Copyright (C) 1999 Jakub Jelinek (jj@ultra.linux.cz)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JAKUB JELINEK OR DAVID MILLER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/misc/SparcMulDiv.S,v 1.1 1999/07/10 07:24:51 dawes Exp $ */

	.globl	urem_sparcv89, umul_sparcv89, udiv_sparcv89
	.globl	rem_sparcv89, mul_sparcv89, div_sparcv89

	.align	32
urem_sparcv89:
	wr	%g0, 0x0, %y
	nop
	nop
	nop
	udiv	%o0, %o1, %o2
	umul	%o2, %o1, %o2
	retl
	 sub	%o0, %o2, %o0

	.align	32
umul_sparcv89:
	umul	%o0, %o1, %o0
	retl
	 rd	%y, %o1

	.align	32
udiv_sparcv89:
	wr	%g0, 0x0, %y
	nop
	nop
	retl
	 udiv	%o0, %o1, %o0

	.align	32
rem_sparcv89:
	sra	%o0, 0x1f, %o4
	wr	%o4, 0x0, %y
	nop
	nop
	nop
	sdivcc	%o0, %o1, %o2
	bvs,a	1f
	 xnor	%o2, %g0, %o2
1:	smul	%o2, %o1, %o2
	retl
	 sub	%o0, %o2, %o0

	.align	32
mul_sparcv89:
	smul	%o0, %o1, %o0
	retl
	 rd	%y, %o1
	nop

	.align	32
div_sparcv89:
	sra	%o0, 0x1f, %o2
	wr	%o2, 0x0, %y
	nop
	nop
	nop
	sdivcc	%o0, %o1, %o0
	bvs,a	1f
	 xnor	%o0, %g0, %o0
1:	retl
	 nop
