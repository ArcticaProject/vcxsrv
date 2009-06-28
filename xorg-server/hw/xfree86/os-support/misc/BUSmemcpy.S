/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/misc/BUSmemcpy.s,v 1.2 1998/07/25 16:56:49 dawes Exp $ */
/******************************************************************************
                        Copyright 1993 by Glenn G. Lai

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Glenn G. Lai not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

Glenn G. Lai DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

Glenn G. Lai
P.O. Box 4314
Austin, Tx 78765
(glenn@cs.utexas.edu)
8/9/93
******************************************************************************/
/* $XConsortium: BUSmemcpy.s /main/4 1996/02/21 17:39:34 kaleb $ */

/*
 * Modified to use long-alignment of video memory rather than word-alignment
 * to improve performance for LocalBus video cards.  Function names changed
 * from ISAToMem and MemToISA to BusToMem and MemToBus.
 *
 * David Dawes <dawes@XFree86.org>, 25 August 1993.
 */


#include "assyntax.h"

	FILE("BUSmemcpy.s")

	AS_BEGIN

/* BusToMem copies from video memory to main memory
   MemToBus copies from main memory to video memory

	void xf86BusToMem(unsigned char *dst, unsigned char *src, int len);
	void xf86MemToBus(unsigned char *dst, unsigned char *src, int len);
*/

#define dst     REGOFF(4,ESP)
#define src     REGOFF(8,ESP)
#define len     REGOFF(12,ESP)

	GLOBL	GLNAME(xf86BusToMem)
	GLOBL	GLNAME(xf86MemToBus)

	SEG_DATA
copyright:
	STRING("Copyright 8/9/1993 by Glenn G. Lai")

	SEG_TEXT
	ALIGNTEXT4
GLNAME(xf86BusToMem):
	CLD
	PUSH_L	(ESI)
	MOV_L	(EDI, EDX)
	
	MOV_L	(src, ESI)
	MOV_L	(dst, EDI)
	MOV_L	(len, ECX)
	
	CMP_L	(CONST(7), ECX)
	JC	(quickBM)

	TEST_L	(CONST(1), ESI)
	JZ	(BwM)

	MOVS_B
	DEC_L	(ECX)

BwM:	
	TEST_L	(CONST(2), ESI)
	JZ	(BlM)

	MOVS_W
	DEC_L	(ECX)
	DEC_L	(ECX)

BlM:
	MOV_L	(ECX, EAX)
	AND_L	(CONST(3), EAX)
	SHR_L	(CONST(2), ECX)
	REP
	MOVS_L
	MOV_L	(EAX, ECX)
quickBM:
	OR_L	(ECX, ECX)
	JZ	(return)
	REP
	MOVS_B
return:
	POP_L	(ESI)
	MOV_L	(EDX, EDI)
	RET
/************************/

	ALIGNTEXT4
GLNAME(xf86MemToBus):
	CLD
	PUSH_L	(ESI)
	MOV_L	(EDI, EDX)
	
	MOV_L	(src, ESI)
	MOV_L	(dst, EDI)
	MOV_L	(len, ECX)
	
	CMP_L	(CONST(7), ECX)
	JC	(quickMB)

	TEST_L	(CONST(1), EDI)
	JZ	(MwB)

	MOVS_B
	DEC_L	(ECX)

MwB:	
	TEST_L	(CONST(2), EDI)
	JZ	(MlB)

	MOVS_W
	DEC_L	(ECX)
	DEC_L	(ECX)

MlB:
	MOV_L	(ECX, EAX)
	AND_L	(CONST(3), EAX)
	SHR_L	(CONST(2), ECX)
	REP
	MOVS_L
	MOV_L	(EAX, ECX)
quickMB:
	OR_L	(ECX, ECX)
	JZ	(return)
	REP
	MOVS_B

	POP_L	(ESI)
	MOV_L	(EDX, EDI)
	RET

