/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/lynxos/lynx_ppc.S,v 1.1 1999/07/10 07:24:49 dawes Exp $ */
/*
 * Copyright 1998 by Metro Link Incorporated
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Metro Link
 * Incorporated not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Metro Link Incorporated makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * METRO LINK INCORPORATED DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL METRO LINK INCORPORATED BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

	.file	"ppc.s"
.toc
	.csect .text[PR]
	.balign 4
	.globl ppc_flush_icache
	.globl .ppc_flush_icache
.csect ppc_flush_icache[DS]
ppc_flush_icache:
	.long .ppc_flush_icache, TOC[tc0], 0
.csect .text[PR]
.ppc_flush_icache:
	mflr 0
	stw 31,-4(1)
	stw 0,8(1)
	stwu 1,-64(1)
	mr 31,1
	stw 3,88(31)
	li 6, 0		/* __inst_dcbf (addr, 0); */
	dcbf 3, 6
	li 5, 32	/* __inst_dcbf (addr, LINESIZE); */
	dcbf 3, 5
	sync		/* __inst_sync (); */
	li 4,0		/* __inst_icbi (addr, 0); */
	icbi  3,4
	li 7,32		/* __inst_icbi (addr, LINESIZE); */
	icbi  3,7
	sync		/* __inst_sync (); */
	isync		/* __inst_isync (); */
L..1:
	lwz 1,0(1)
	lwz 0,8(1)
	mtlr 0
	lwz 31,-4(1)
	blr
LT..ppc_flush_icache:
	.long 0
	.byte 0,0,32,97,128,1,1,1
	.long 0
	.long LT..ppc_flush_icache-.ppc_flush_icache
	.short 16
	.byte "ppc_flush_icache"
	.byte 31
_section_.text:
	.csect .data[RW]
	.long _section_.text
