/*
 * Copyright 1996  The XFree86 Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * HARM HANEMAAYER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 * 
 * Written by Harm Hanemaayer (H.Hanemaayer@inter.nl.net).
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/xaa/xaaTEGlyphBlt.S,v 1.1 1999/07/10 07:24:54 dawes Exp $ */


/*
 * Intel Pentium-optimized versions of "terminal emulator font" text
 * bitmap transfer routines.
 *
 * SCANLINE_PAD_DWORD.
 *
 * Only for glyphs with a fixed width of 6 pixels or 8 pixels.
 */

#include "assyntax.h"

#ifndef QNX4
 	FILE("xaaTEGlyphBlt.s")
#else
 	FILE( __FILE__ )
#endif

	AS_BEGIN

/*
 * Definition of stack frame function arguments.
 * All functions have the same arguments (some don't have glyphwidth,
 * but that's OK, since it comes last and doesn't affect the offset
 * of the other arguments).
 */

#define base_arg	REGOFF(20,ESP)
#define glyphp_arg	REGOFF(24,ESP)
#define line_arg	REGOFF(28,ESP)
#define width_arg	REGOFF(32,ESP)
#define glyphwidth_arg	REGOFF(36,ESP)

#define BYTE_REVERSED	GLNAME(byte_reversed)

/* I assume %eax and %edx can be trashed. */

	SEG_TEXT

	ALIGNTEXT4

#ifdef FIXEDBASE
# ifdef MSBFIRST
	GLOBL GLNAME(DrawTETextScanlineWidth6PMSBFirstFixedBase)
GLNAME(DrawTETextScanlineWidth6PMSBFirstFixedBase):
# else
	GLOBL GLNAME(DrawTETextScanlineWidth6PLSBFirstFixedBase)
GLNAME(DrawTETextScanlineWidth6PLSBFirstFixedBase):
# endif
#else
# ifdef MSBFIRST
	GLOBL GLNAME(DrawTETextScanlineWidth6PMSBFirst)
GLNAME(DrawTETextScanlineWidth6PMSBFirst):
# else
	GLOBL GLNAME(DrawTETextScanlineWidth6PLSBFirst)
GLNAME(DrawTETextScanlineWidth6PLSBFirst):
# endif
#endif

/* Definition of stack frame function arguments. */

#define base_arg	REGOFF(20,ESP)
#define glyphp_arg	REGOFF(24,ESP)
#define line_arg	REGOFF(28,ESP)
#define width_arg	REGOFF(32,ESP)

	SUB_L	(CONST(16),ESP)
	MOV_L	(EBP,REGOFF(12,ESP))	/* PUSH EBP */
	MOV_L	(EBX,REGOFF(8,ESP))	/* PUSH EBX */
	MOV_L	(ESI,REGOFF(4,ESP))	/* PUSH ESI */
	MOV_L	(EDI,REGOFF(0,ESP))	/* PUSH EDI */

	MOV_L	(line_arg,EBP)
	MOV_L	(base_arg,EDI)
	MOV_L	(glyphp_arg,ESI)

	ALIGNTEXT4

.L6_1:
	/* Pentium-optimized instruction pairing. */
	/* EBX = bits = glyph[0][line] */
	MOV_L	(REGOFF(4,ESI),EDX)		/* glyphp[1] */
	MOV_L	(REGIND(ESI),EBX)		/* glyphp[0] */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[1][line] */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[0][line] */
	SAL_L	(CONST(6),EDX)			/* glyphp[1][line] << 6 */
	MOV_L	(REGOFF(8,ESI),ECX)		/* glyphp[2] */
	MOV_L	(REGOFF(12,ESI),EAX)		/* glyphp[3] */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[2][line] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[3][line] */
	SAL_L	(CONST(12),ECX)			/* glyphp[2][line] << 12 */
	OR_L	(EDX,EBX)			/* bits |= ..[1].. << 6 */
	SAL_L	(CONST(18),EAX)			/* glyphp[3][line] << 18 */
	OR_L	(ECX,EBX)			/* bits |= ..[2].. << 12 */

	MOV_L	(REGOFF(16,ESI),EDX)		/* glyphp[4] */
	MOV_L	(REGOFF(20,ESI),ECX)		/* glyphp[5] */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[4][line] */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[5][line] */
	SAL_L	(CONST(24),EDX)			/* glyphp[4][line] << 24 */
	OR_L	(EAX,EBX)			/* bits |= ..[3].. << 18 */
	SAL_L	(CONST(30),ECX)			/* glyphp[5][line] << 30 */
	OR_L	(EDX,EBX)			/* bits |= ..[4].. << 24 */

#ifndef MSBFIRST
	MOV_L	(REGOFF(20,ESI),EAX)		/* glyphp[5] */
	OR_L	(ECX,EBX)			/* bits |= ..[5].. << 30 */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[5][line] */
	MOV_L	(EBX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(CONST(0),EAX)
	OR_L	(ECX,EBX)			/* bits |= ..[5].. << 30 */
	MOV_L	(CONST(0),EDX)
	MOV_B	(BL,AL)
	MOV_B	(BH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),BH)
	ROL_L	(CONST(16),EBX)
	MOV_B	(BL,AL)
	MOV_B	(BH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),BH)
	ROL_L	(CONST(16),EBX)
	MOV_L	(REGOFF(20,ESI),EAX)		/* glyphp[5] */
	MOV_L	(EBX,REGIND(EDI))
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[5][line] */
#endif

	CMP_L   (CONST(32),width_arg)
        JG      (.L6_2)
#ifndef FIXEDBASE
	ADD_L	(CONST(4),EDI)		/* base++ */
#endif
	JMP	(.L6_4)
.L6_2:
	/* Note that glyphp[5][line] is already read again. */
	/* EAX = bits = glyphp[5][line] >> 2 */
	MOV_L	(REGOFF(24,ESI),EDX)		/* glyphp[6] */
	MOV_L	(REGOFF(28,ESI),EBX)		/* glyphp[7] */
	SHR_L	(CONST(2),EAX)			/* glyphp[5][line] >> 2 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[6][line] */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[7][line] */
	MOV_L	(REGOFF(32,ESI),ECX)		/* glyphp[8] */
	SAL_L	(CONST(4),EDX)			/* glyphp[6][line] << 4 */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[8][line] */
	SAL_L	(CONST(10),EBX)			/* glyphp[7][line] << 10 */
	OR_L	(EDX,EAX)			/* bits |= ..[6].. << 4 */
	SAL_L	(CONST(16),ECX)			/* glyphp[8][line] << 16 */
	MOV_L	(REGOFF(36,ESI),EDX)		/* glyphp[9] */
	OR_L	(EBX,EAX)			/* bits |= ..[7].. << 10 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[9][line] */
	OR_L	(ECX,EAX)			/* bits |= ..[8].. << 16 */
	MOV_L	(REGOFF(40,ESI),EBX)		/* glyphp[10] */
	SAL_L	(CONST(22),EDX)			/* glyphp[9][line] << 22 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[10][line] */
	OR_L	(EDX,EAX)			/* bits |= ..[9].. << 22 */

#ifndef MSBFIRST
	MOV_L	(REGOFF(40,ESI),ECX)		/* glyphp[10] */
	SAL_L	(CONST(28),EBX)			/* glyphp[10][line] << 28 */
	MOV_L	(REGOFF(44,ESI),EDX)		/* glyphp[11] */
	OR_L	(EBX,EAX)			/* bits |= ..[10].. << 28 */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[10][line] */
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(4,EDI)) /* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[11][line] */
#else
	MOV_L	(CONST(0),ECX)
	SAL_L	(CONST(28),EBX)			/* glyphp[10][line] << 28 */
	MOV_L	(CONST(0),EDX)
	OR_L	(EBX,EAX)			/* bits |= ..[10].. << 28 */
	MOV_B	(AL,CL)
	MOV_B	(AH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),AH)
	ROL_L	(CONST(16),EAX)
	MOV_B	(AL,CL)
	MOV_B	(AH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),AH)
	MOV_L	(REGOFF(40,ESI),ECX)		/* glyphp[10] */
	ROL_L	(CONST(16),EAX)
	MOV_L	(REGOFF(44,ESI),EDX)		/* glyphp[11] */
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(4,EDI)) /* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[10][line] */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[11][line] */
#endif

	CMP_L   (CONST(64),width_arg)
        JG      (.L6_3)
#ifndef FIXEDBASE
	ADD_L	(CONST(8),EDI)		/* base+=2 */
#endif
	JMP	(.L6_4)
.L6_3:
	/* Note that glyphp[10][line] is read again. */
	/* EAX = bits = glyphp[10][line] >> 4 */
	SHR_L	(CONST(4),ECX)			/* glyphp[10][line] >> 4 */
	MOV_L	(REGOFF(48,ESI),EBX)		/* glyphp[12] */
	SAL_L	(CONST(2),EDX)			/* glyphp[11][line] << 2 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[12][line] */
	OR_L	(EDX,ECX)			/* bits |= ..[11].. << 2 */
	MOV_L	(REGOFF(52,ESI),EAX)		/* glyphp[13] */
	SAL_L	(CONST(8),EBX)			/* glyphp[12][line] << 8 */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[13][line] */
	OR_L	(EBX,ECX)			/* bits |= ..[12].. << 8 */
	MOV_L	(REGOFF(56,ESI),EDX)		/* glyphp[14] */
	SAL_L	(CONST(14),EAX)			/* glyphp[13][line] << 14 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[14][line] */
	OR_L	(EAX,ECX)			/* bits |= ..[13].. << 14 */
	MOV_L	(REGOFF(60,ESI),EBX)		/* glyphp[15] */
	SAL_L	(CONST(20),EDX)			/* glyphp[14][line] << 20 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[15][line] */
	OR_L	(EDX,ECX)			/* bits |= ..[14].. << 20 */

#ifndef MSBFIRST
	SAL_L	(CONST(26),EBX)			/* glyphp[15][line] << 26 */
	OR_L	(EBX,ECX)			/* bits |= ..[15].. << 26 */
#ifndef FIXEDBASE
	MOV_L	(ECX,REGOFF(8,EDI)) /* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(ECX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),EAX)
	SAL_L	(CONST(26),EBX)			/* glyphp[15][line] << 26 */
	MOV_L	(CONST(0),EDX)
	OR_L	(EBX,ECX)			/* bits |= ..[15].. << 26 */
	MOV_B	(CL,AL)
	MOV_B	(CH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),CH)
	ROL_L	(CONST(16),ECX)
	MOV_B	(CL,AL)
	MOV_B	(CH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),CH)
	ROL_L	(CONST(16),ECX)
#ifndef FIXEDBASE
	MOV_L	(ECX,REGOFF(8,EDI)) /* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(ECX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#endif

#ifndef FIXEDBASE
	ADD_L	(CONST(12),EDI)		/* base += 3*/
#endif
	CMP_L   (CONST(96),width_arg)
        JLE     (.L6_4)
	ADD_L	(CONST(64),ESI)		/* glyphp += 16 */
	SUB_L	(CONST(96),width_arg)
        JMP     (.L6_1)

.L6_4:

#ifndef FIXEDBASE
	MOV_L	(EDI,EAX)		/* return base */
#else
	MOV_L	(base_arg,EAX)		/* return base */
#endif
	MOV_L	(REGOFF(0,ESP),EDI)	/* POPL EDI */
	MOV_L	(REGOFF(4,ESP),ESI)	/* POPL ESI */
	MOV_L	(REGOFF(8,ESP),EBX)	/* POPL EBX */
	MOV_L	(REGOFF(12,ESP),EBP)	/* POPL EBP */
	ADD_L	(CONST(16),ESP)
	RET
	

	ALIGNTEXT4

#ifdef FIXEDBASE
# ifdef MSBFIRST
	GLOBL GLNAME(DrawTETextScanlineWidth8PMSBFirstFixedBase)
GLNAME(DrawTETextScanlineWidth8PMSBFirstFixedBase):
# else
	GLOBL GLNAME(DrawTETextScanlineWidth8PLSBFirstFixedBase)
GLNAME(DrawTETextScanlineWidth8PLSBFirstFixedBase):
# endif
#else
# ifdef MSBFIRST
	GLOBL GLNAME(DrawTETextScanlineWidth8PMSBFirst)
GLNAME(DrawTETextScanlineWidth8PMSBFirst):
# else
	GLOBL GLNAME(DrawTETextScanlineWidth8PLSBFirst)
GLNAME(DrawTETextScanlineWidth8PLSBFirst):
# endif
#endif

	SUB_L	(CONST(16),ESP)
	MOV_L	(EBP,REGOFF(12,ESP))	/* PUSH EBP */
	MOV_L	(EBX,REGOFF(8,ESP))	/* PUSH EBX */
	MOV_L	(ESI,REGOFF(4,ESP))	/* PUSH ESI */
	MOV_L	(EDI,REGOFF(0,ESP))	/* PUSH EDI */

	MOV_L	(line_arg,EBP)
	MOV_L	(base_arg,EDI)
	MOV_L	(glyphp_arg,ESI)

	ALIGNTEXT4

.L8_1:
	/* Pentium-optimized instruction pairing. */
	/* EBX = bits */
	MOV_L	(REGIND(ESI),EAX)		/* glyphp[0] */
	MOV_L	(REGOFF(4,ESI),EDX)		/* glyphp[1] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[0][line] */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[1][line] */
#ifdef MSBFIRST
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),BH)
#else
	MOV_L	(EAX,EBX)			/* bits = glyph[0][line] */
	MOV_B   (DL,BH)				/* bits |= ..[1].. << 8 */
#endif

	ROL_L	(CONST(16),EBX)
	MOV_L	(REGOFF(8,ESI),EAX)		/* glyphp[2] */
	MOV_L	(REGOFF(12,ESI),ECX)		/* glyphp[3] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[2][line] */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[3][line] */
#ifdef MSBFIRST
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),BL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),BH)
#else
	MOV_B   (AL,BL)				/* bits |= ..[2].. << 16 */
	MOV_B   (CL,BH)				/* bits |= ..[3].. << 24 */
#endif
	ROL_L	(CONST(16),EBX)
	MOV_L	(EBX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
	CMP_L	(CONST(32),width_arg)
#ifndef FIXEDBASE
	JLE	(.L8_2)
#else
	JLE	(.L8_3)
#endif

	MOV_L	(REGOFF(16,ESI),EAX)		/* glyphp[4] */
	MOV_L	(REGOFF(20,ESI),EDX)		/* glyphp[5] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[4][line] */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[5][line] */
#ifdef MSBFIRST
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),BH)
#else
	MOV_L    (EAX,EBX)			/* bits = glyph[4][line] */
	MOV_B    (DL,BH)			/* nits |= ..[5].. << 8 */
#endif

	ROL_L	(CONST(16),EBX)
	MOV_L	(REGOFF(24,ESI),EAX)		/* glyphp[6] */
	MOV_L	(REGOFF(28,ESI),ECX)		/* glyphp[7] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[6][line] */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[7][line] */
#ifdef MSBFIRST
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),BL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),BH)
#else
	MOV_B   (AL,BL)				/* bits |= ..[6].. << 16 */
	MOV_B   (CL,BH)				/* bits |= ..[7].. << 24 */
#endif
	ROL_L	(CONST(16),EBX)
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(4,EDI))	/* WRITE_IN_BIT_ORDER(base+1, bits) */
	ADD_L	(CONST(8),EDI)		/* base += 2 */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
	CMP_L	(CONST(64),width_arg)
	JLE	(.L8_3)
	ADD_L	(CONST(32),ESI)		/* glyphp += 8 */
	SUB_L	(CONST(64),width_arg)
	JMP	(.L8_1)

#ifndef FIXEDBASE
.L8_2:
	ADD_L	(CONST(4),EDI)		/* base++ */
.L8_3:
	MOV_L	(EDI,EAX)		/* return base */
#else
.L8_3:
	MOV_L	(base_arg,EAX)		/* return base */
#endif
	MOV_L	(REGOFF(0,ESP),EDI)	/* POPL EDI */
	MOV_L	(REGOFF(4,ESP),ESI)	/* POPL ESI */
	MOV_L	(REGOFF(8,ESP),EBX)	/* POPL EBX */
	MOV_L	(REGOFF(12,ESP),EBP)	/* POPL EBP */
	ADD_L	(CONST(16),ESP)
	RET

	ALIGNTEXT4

#ifdef FIXEDBASE
# ifdef MSBFIRST
	GLOBL GLNAME(DrawTETextScanlineWidth9PMSBFirstFixedBase)
GLNAME(DrawTETextScanlineWidth9PMSBFirstFixedBase):
# else
	GLOBL GLNAME(DrawTETextScanlineWidth9PLSBFirstFixedBase)
GLNAME(DrawTETextScanlineWidth9PLSBFirstFixedBase):
# endif
#else
# ifdef MSBFIRST
	GLOBL GLNAME(DrawTETextScanlineWidth9PMSBFirst)
GLNAME(DrawTETextScanlineWidth9PMSBFirst):
# else
	GLOBL GLNAME(DrawTETextScanlineWidth9PLSBFirst)
GLNAME(DrawTETextScanlineWidth9PLSBFirst):
# endif
#endif

	SUB_L	(CONST(16),ESP)
	MOV_L	(EBP,REGOFF(12,ESP))	/* PUSH EBP */
	MOV_L	(EBX,REGOFF(8,ESP))	/* PUSH EBX */
	MOV_L	(ESI,REGOFF(4,ESP))	/* PUSH ESI */
	MOV_L	(EDI,REGOFF(0,ESP))	/* PUSH EDI */

	MOV_L	(line_arg,EBP)
	MOV_L	(base_arg,EDI)
	MOV_L	(glyphp_arg,ESI)

	ALIGNTEXT4

.L9_1:
	/* Pentium-optimized instruction pairing. */
	/* EAX = bits */
	MOV_L	(REGOFF(4,ESI),EBX)		/* glyphp[1] */
	MOV_L	(REGIND(ESI),EAX)		/* glyphp[0] */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[1][line] */
	MOV_L	(REGOFF(8,ESI),ECX)		/* glyphp[2] */
	SAL_L	(CONST(9),EBX)			/* glyphp[1][line] << 9 */
	MOV_L	(REGOFF(12,ESI),EDX)		/* glyphp[3] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[0][line] */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[3][line] */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[2][line] */
	OR_L	(EBX,EAX)			/* bits |= ..[1].. << 9 */
	SAL_L	(CONST(18),ECX)			/* glyphp[2][line] << 18 */
	OR_L	(ECX,EAX)			/* bits |= ..[2].. << 18 */
	SAL_L	(CONST(27),EDX)			/* glyphp[3][line << 27 */
			
#ifndef MSBFIRST
	MOV_L	(REGOFF(12,ESI),EBX)		/* glyphp[3] */
	OR_L	(EDX,EAX)			/* bits |= ..[3].. << 27 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[3][line] */
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(CONST(0),EBX)
	OR_L	(EDX,EAX)			/* bits |= ..[3].. << 27 */
	MOV_L	(CONST(0),ECX)
	MOV_B	(AL,BL)
	MOV_B	(AH,CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),AH)
	ROL_L	(CONST(16),EAX)
	MOV_B	(AL,BL)
	MOV_B	(AH,CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),AH)
	MOV_L	(REGOFF(12,ESI),EBX)		/* glyphp[3] */
	ROL_L	(CONST(16),EAX)
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[3][line] */
	MOV_L	(EAX,REGIND(EDI))
#endif

	CMP_L   (CONST(32),width_arg)
        JG      (.L9_2)
#ifndef FIXEDBASE
	ADD_L	(CONST(4),EDI)		/* base++ */
#endif
	JMP	(.L9_11)
.L9_2:
	/* Note that glyphp[3][line] is read again. */
	/* EAX = bits, EBX = glyphp[3][line] >> 5 */
	SHR_L	(CONST(5),EBX)			/* glyphp[3][line] >> 5 */
	MOV_L	(REGOFF(16,ESI),EAX)		/* glyphp[4] */
	MOV_L	(REGOFF(20,ESI),ECX)		/* glyphp[5] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[4][line] */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[5][line] */
	MOV_L	(REGOFF(24,ESI),EDX)		/* glyphp[6] */
	SAL_L	(CONST(4),EAX)			/* glyphp[4][line] << 4 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[6][line] */
	SAL_L	(CONST(13),ECX)			/* glyphp[5][line] << 13 */
	OR_L	(EBX,EAX)			/* bits |= ..[4].. << 4 */
	SAL_L	(CONST(22),EDX)			/* glyphp[6][line] << 22 */
	MOV_L	(REGOFF(28,ESI),EBX)		/* glyphp[7] */
	OR_L	(ECX,EAX)			/* bits |= ..[5].. << 13 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[7][line] */
	OR_L	(EDX,EAX)			/* bits |= ..[6].. << 22 */

#ifndef MSBFIRST
	MOV_L	(REGOFF(28,ESI),ECX)		/* glyphp[7] */
	SAL_L	(CONST(31),EBX)			/* glyphp[7][line] << 31 */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[7][line] */
	OR_L	(EBX,EAX)			/* bits |= ..[7].. << 31 */
	MOV_L	(REGOFF(32,ESI),EDX)		/* glyphp[8] */
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(4,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),ECX)
	SAL_L	(CONST(31),EBX)			/* glyphp[7][line] << 31 */
	MOV_L	(CONST(0),EDX)
	OR_L	(EBX,EAX)			/* bits |= ..[7].. << 31 */
	MOV_B	(AL,CL)
	MOV_B	(AH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),AH)
	ROL_L	(CONST(16),EAX)
	MOV_B	(AL,CL)
	MOV_B	(AH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),AH)
	MOV_L	(REGOFF(28,ESI),ECX)		/* glyphp[7] */
	ROL_L	(CONST(16),EAX)
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[7][line] */
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(4,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
	MOV_L	(REGOFF(32,ESI),EDX)		/* glyphp[8] */
#endif

	CMP_L   (CONST(64),width_arg)
        JG      (.L9_3)
#ifndef FIXEDBASE
	ADD_L	(CONST(8),EDI)		/* base+=2 */
#endif
	JMP	(.L9_11)
.L9_3:

	/* Note that glyphp[7][line] is read again. */
	/* ECX = bits = glyphp[7][line] >> 1 */
	SHR_L	(CONST(1),ECX)			/* glyphp[7][line] >> 1 */
	MOV_L	(REGOFF(36,ESI),EBX)		/* glyphp[9] */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[8][line] */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[9][line] */
	SAL_L	(CONST(8),EDX)			/* glyphp[8][line] << 8 */
	MOV_L	(REGOFF(40,ESI),EAX)		/* glyphp[10] */
	SAL_L	(CONST(17),EBX)			/* glyphp[9][line] << 17 */
	OR_L	(EDX,ECX)			/* bits |= ..[8].. << 8 */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[10][line] */
	OR_L	(EBX,ECX)			/* bits |= ..[9].. << 17 */
	SAL_L	(CONST(26),EAX)			/* glyphp[10][line] << 26 */

#ifndef MSBFIRST
	MOV_L	(REGOFF(40,ESI),EDX)		/* glyphp[10] */
	OR_L	(EAX,ECX)			/* bits |= ..[10].. << 26 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[10][line] */
#ifndef FIXEDBASE
	MOV_L	(ECX,REGOFF(8,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(ECX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),EDX)
	OR_L	(EAX,ECX)			/* bits |= ..[10].. << 26 */
	MOV_L	(CONST(0),EBX)
	MOV_B	(CL,DL)
	MOV_B	(CH,BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),CH)
	ROL_L	(CONST(16),ECX)
	MOV_B	(CL,DL)
	MOV_B	(CH,BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),CH)
	MOV_L	(REGOFF(40,ESI),EDX)		/* glyphp[10] */
	ROL_L	(CONST(16),ECX)
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[10][line] */
#ifndef FIXEDBASE
	MOV_L	(ECX,REGOFF(8,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(ECX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#endif

	CMP_L   (CONST(96),width_arg)
        JG      (.L9_4)
#ifndef FIXEDBASE
	ADD_L	(CONST(12),EDI)		/* base+=3 */
#endif
	JMP	(.L9_11)
.L9_4:
	/* Note that glyphp[10][line] is read again. */
	/* EDX = bits = glyphp[10][line] >> 6 */
	SHR_L	(CONST(6),EDX)			/* glyphp[10][line] >> 6 */
	MOV_L	(REGOFF(44,ESI),EBX)		/* glyphp[11] */
	MOV_L	(REGOFF(48,ESI),EAX)		/* glyphp[12] */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[11][line] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[12][line] */
	MOV_L	(REGOFF(52,ESI),ECX)		/* glyphp[13] */
	SAL_L	(CONST(3),EBX)			/* glyphp[11][line] << 3 */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[13][line] */
	SAL_L	(CONST(12),EAX)			/* glyphp[12][line] << 12 */
	OR_L	(EBX,EDX)			/* bits |= ..[11].. << 3 */
	SAL_L	(CONST(21),ECX)			/* glyphp[13][line] << 21 */
	MOV_L	(REGOFF(56,ESI),EBX)		/* glyphp[14] */
	OR_L	(EAX,EDX)			/* bits |= ..[12].. << 17 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[14][line] */
	SAL_L	(CONST(30),EBX)			/* glyphp[14][line] << 30 */
	OR_L	(ECX,EDX)			/* bits |= ..[13].. << 21 */

#ifndef MSBFIRST
	MOV_L	(REGOFF(56,ESI),EAX)		/* glyphp[14] */
	OR_L	(EBX,EDX)			/* bits |= ..[14].. << 30 */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[14][line] */
#ifndef FIXEDBASE
	MOV_L	(EDX,REGOFF(12,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EDX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),EAX)
	OR_L	(EBX,EDX)			/* bits |= ..[14].. << 30 */
	MOV_L	(CONST(0),ECX)
	MOV_B	(DL,AL)
	MOV_B	(DH,CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),DL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),DH)
	ROL_L	(CONST(16),EDX)
	MOV_B	(DL,AL)
	MOV_B	(DH,CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),DL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),DH)
	MOV_L	(REGOFF(56,ESI),EAX)		/* glyphp[14] */
	ROL_L	(CONST(16),EDX)
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[14][line] */
#ifndef FIXEDBASE
	MOV_L	(EDX,REGOFF(12,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EDX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#endif

	CMP_L   (CONST(128),width_arg)
        JG      (.L9_5)
#ifndef FIXEDBASE
	ADD_L	(CONST(16),EDI)		/* base+=4 */
#endif
	JMP	(.L9_11)
.L9_5:	
	/* Note that glyphp[14][line] is read again. */
	/* EAX = bits = glyphp[14][line] >> 2 */
	SHR_L	(CONST(2),EAX)			/* glyphp[14][line] >> 2 */
	MOV_L	(REGOFF(60,ESI),ECX)		/* glyphp[15] */
	MOV_L	(REGOFF(64,ESI),EBX)		/* glyphp[16] */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[15][line] */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[16][line] */
	MOV_L	(REGOFF(68,ESI),EDX)		/* glyphp[17] */
	SAL_L	(CONST(7),ECX)			/* glyphp[15][line] << 7 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[17][line] */
	SAL_L	(CONST(16),EBX)			/* glyphp[16][line] << 16 */
	OR_L	(ECX,EAX)			/* bits |= ..[15].. << 7 */
	SAL_L	(CONST(25),EDX)			/* glyphp[17][line] << 25 */
	OR_L	(EBX,EAX)			/* bits |= ..[16].. << 16 */
	
#ifndef MSBFIRST
	MOV_L	(REGOFF(68,ESI),ECX)		/* glyphp[17] */
	OR_L	(EDX,EAX)			/* bits |= ..[17].. << 25 */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[17][line] */
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(16,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),ECX)
	OR_L	(EDX,EAX)			/* bits |= ..[17].. << 25 */
	MOV_L	(CONST(0),EBX)
	MOV_B	(AL,CL)
	MOV_B	(AH,BL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),AH)
	ROL_L	(CONST(16),EAX)
	MOV_B	(AL,CL)
	MOV_B	(AH,BL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),AH)
	MOV_L	(REGOFF(68,ESI),ECX)		/* glyphp[17] */
	ROL_L	(CONST(16),EAX)
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[17][line] */
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(16,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#endif

	CMP_L   (CONST(160),width_arg)
        JG      (.L9_6)
#ifndef FIXEDBASE
	ADD_L	(CONST(20),EDI)		/* base+=5 */
#endif
	JMP	(.L9_11)
.L9_6:	
	/* Note that glyphp[17][line] is read again. */
	/* ECX = bits = glyphp[17][line] >> 7 */
	SHR_L	(CONST(7),ECX)			/* glyphp[17][line] >> 7 */
	MOV_L	(REGOFF(72,ESI),EBX)		/* glyphp[18] */
	MOV_L	(REGOFF(76,ESI),EAX)		/* glyphp[19] */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[18][line] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[19][line] */
	MOV_L	(REGOFF(80,ESI),EDX)		/* glyphp[20] */
	SAL_L	(CONST(2),EBX)			/* glyphp[18][line] << 2 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[20][line] */
	SAL_L	(CONST(11),EAX)			/* glyphp[19][line] << 11 */
	OR_L	(EBX,ECX)			/* bits |= ..[18].. << 2 */
	SAL_L	(CONST(20),EDX)			/* glyphp[20][line] << 20 */
	MOV_L	(REGOFF(84,ESI),EBX)		/* glyphp[21] */
	OR_L	(EAX,ECX)			/* bits |= ..[19].. << 11 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[21][line] */
	OR_L	(EDX,ECX)			/* bits |= ..[20].. << 20 */
	
#ifndef MSBFIRST
	MOV_L	(REGOFF(84,ESI),EAX)		/* glyphp[21] */
	SAL_L	(CONST(29),EBX)			/* glyphp[21][line] << 29 */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[21][line] */
	OR_L	(EBX,ECX)			/* bits |= ..[14].. << 30 */
	MOV_L	(REGOFF(88,ESI),EDX)		/* glyphp[22] */
#ifndef FIXEDBASE
	MOV_L	(ECX,REGOFF(20,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(ECX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),EAX)
	SAL_L	(CONST(29),EBX)			/* glyphp[21][line] << 29 */
	MOV_L	(CONST(0),EDX)
	OR_L	(EBX,ECX)			/* bits |= ..[14].. << 30 */
	MOV_B	(CL,AL)
	MOV_B	(CH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),CH)
	ROL_L	(CONST(16),ECX)
	MOV_B	(CL,AL)
	MOV_B	(CH,DL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),CH)
	MOV_L	(REGOFF(84,ESI),EAX)		/* glyphp[21] */
	ROL_L	(CONST(16),ECX)
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[21][line] */
#ifndef FIXEDBASE
	MOV_L	(ECX,REGOFF(20,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(ECX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
	MOV_L	(REGOFF(88,ESI),EDX)		/* glyphp[22] */
#endif

	CMP_L   (CONST(192),width_arg)
        JG      (.L9_7)
#ifndef FIXEDBASE
	ADD_L	(CONST(24),EDI)		/* base+=6 */
#endif
	JMP	(.L9_11)
.L9_7:
	/* Note that glyphp[21][line] is read again. */
	/* EAX = bits = glyphp[21][line] >> 3 */
	SHR_L	(CONST(3),EAX)			/* glyphp[21][line] >> 3 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[22][line] */
	MOV_L	(REGOFF(92,ESI),EBX)		/* glyphp[23] */
	MOV_L	(REGOFF(96,ESI),ECX)		/* glyphp[24] */
	SAL_L	(CONST(6),EDX)			/* glyphp[22][line] << 6 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[23][line] */
	OR_L	(EDX,EAX)			/* bits |= ..[22].. << 6 */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[24][line] */
	SAL_L	(CONST(15),EBX)			/* glyphp[23][line] << 15 */
	OR_L	(EBX,EAX)			/* bits |= ..[23].. << 15 */
	SAL_L	(CONST(24),ECX)			/* glyphp[24][line] << 24 */

#ifndef MSBFIRST
	MOV_L	(REGOFF(96,ESI),EDX)		/* glyphp[24] */
	OR_L	(ECX,EAX)			/* bits |= ..[24].. << 24 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[24][line] */
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(24,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),EDX)
	OR_L	(ECX,EAX)			/* bits |= ..[24].. << 24 */
	MOV_L	(CONST(0),EBX)
	MOV_B	(AL,DL)
	MOV_B	(AH,BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),AH)
	ROL_L	(CONST(16),EAX)
	MOV_B	(AL,DL)
	MOV_B	(AH,BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EDX),AL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),AH)
	MOV_L	(REGOFF(96,ESI),EDX)		/* glyphp[24] */
	ROL_L	(CONST(16),EAX)
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[24][line] */
#ifndef FIXEDBASE
	MOV_L	(EAX,REGOFF(24,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EAX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#endif

	CMP_L   (CONST(224),width_arg)
        JG      (.L9_8)
#ifndef FIXEDBASE
	ADD_L	(CONST(28),EDI)		/* base+=7 */
#endif
	JMP	(.L9_11)
.L9_8:
	/* Note that glyphp[24][line] is read again. */
	/* EDX = bits = glyphp[24][line] >> 8 */

	SHR_L	(CONST(8),EDX)			/* glyphp[24][line] >> 8 */
	MOV_L	(REGOFF(100,ESI),EAX)		/* glyphp[25] */
	MOV_L	(REGOFF(104,ESI),EBX)		/* glyphp[26] */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[25][line] */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[26][line] */
	MOV_L	(REGOFF(108,ESI),ECX)		/* glyphp[27] */
	SAL_L	(CONST(1),EAX)			/* glyphp[25][line] << 1 */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[27][line] */
	SAL_L	(CONST(10),EBX)			/* glyphp[26][line] << 10 */
	OR_L	(EAX,EDX)			/* bits |= ..[25].. << 1 */
	SAL_L	(CONST(19),ECX)			/* glyphp[27][line] << 19 */
	OR_L	(EBX,EDX)			/* bits |= ..[26].. << 10 */
	MOV_L	(REGOFF(112,ESI),EAX)		/* glyphp[28] */
	OR_L	(ECX,EDX)			/* bits |= ..[27].. << 19 */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[28][line] */

#ifndef MSBFIRST
	MOV_L	(REGOFF(112,ESI),EBX)		/* glyphp[28] */
	SAL_L	(CONST(28),EAX)			/* glyphp[28][line] << 28 */
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[28][line] */
	OR_L	(EAX,EDX)			/* bits |= ..[28].. << 28 */
	MOV_L	(REGOFF(116,ESI),ECX)		/* glyphp[29] */
#ifndef FIXEDBASE
	MOV_L	(EDX,REGOFF(28,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EDX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),EBX)
	SAL_L	(CONST(28),EAX)			/* glyphp[28][line] << 28 */
	MOV_L	(CONST(0),ECX)
	OR_L	(EAX,EDX)			/* bits |= ..[28].. << 28 */
	MOV_B	(DL,BL)
	MOV_B	(DH,CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),DL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),DH)
	ROL_L	(CONST(16),EDX)
	MOV_B	(DL,BL)
	MOV_B	(DH,CL)
	MOV_B	(REGOFF(BYTE_REVERSED,EBX),DL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),DH)
	MOV_L	(REGOFF(112,ESI),EBX)		/* glyphp[28] */
	ROL_L	(CONST(16),EDX)
	MOV_L	(REGOFF(116,ESI),ECX)		/* glyphp[29] */
#ifndef FIXEDBASE
	MOV_L	(EDX,REGOFF(28,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EDX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
	MOV_L	(REGBISD(EBX,EBP,4,0),EBX)	/* glyphp[28][line] */
#endif
	
	CMP_L   (CONST(256),width_arg)
        JG      (.L9_9)
#ifndef FIXEDBASE
	ADD_L	(CONST(32),EDI)		/* base+=8 */
#endif
	JMP	(.L9_11)
.L9_9:
	/* Note that glyphp[28][line] is read again. */
	/* EBX = bits = glyphp[28][line] >> 4 */
	SHR_L	(CONST(4),EBX)			/* glyphp[28][line] >> 4 */
	MOV_L	(REGBISD(ECX,EBP,4,0),ECX)	/* glyphp[29][line] */
	MOV_L	(REGOFF(120,ESI),EAX)		/* glyphp[30] */
	MOV_L	(REGOFF(124,ESI),EDX)		/* glyphp[31] */
	SAL_L	(CONST(5),ECX)			/* glyphp[29][line] << 5 */
	MOV_L	(REGBISD(EAX,EBP,4,0),EAX)	/* glyphp[30][line] */
	OR_L	(ECX,EBX)			/* bits |= ..[29].. << 5 */
	MOV_L	(REGBISD(EDX,EBP,4,0),EDX)	/* glyphp[31][line] */
	SAL_L	(CONST(14),EAX)			/* glyphp[30][line] << 14 */
	ADD_L	(CONST(128),ESI)		/* glyphp+=32 */
	SAL_L	(CONST(23),EDX)			/* glyphp[31][line] << 23 */
	OR_L	(EAX,EBX)			/* bits |= ..[30].. << 14 */
	SUB_L	(CONST(288),width_arg)		/* width-=288 */
	OR_L	(EDX,EBX)			/* bits |= ..[31].. << 23 */

#ifndef MSBFIRST
#ifndef FIXEDBASE
	MOV_L	(EBX,REGOFF(32,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EBX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#else
	MOV_L	(CONST(0),ECX)
	MOV_L	(CONST(0),EAX)
	MOV_B	(BL,CL)
	MOV_B	(BH,AL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),BH)
	ROL_L	(CONST(16),EBX)
	MOV_B	(BL,CL)
	MOV_B	(BH,AL)
	MOV_B	(REGOFF(BYTE_REVERSED,ECX),BL)
	MOV_B	(REGOFF(BYTE_REVERSED,EAX),BH)
	ROL_L	(CONST(16),EBX)
#ifndef FIXEDBASE
	MOV_L	(EBX,REGOFF(32,EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#else
	MOV_L	(EBX,REGIND(EDI))	/* WRITE_IN_BIT_ORDER(base, bits) */
#endif
#endif
	
	ADD_L	(CONST(36),EDI)		/* base+=9 */
	CMP_L   (CONST(0),width_arg)
        JG      (.L9_1)
			
.L9_11:
#ifndef FIXEDBASE
	MOV_L	(EDI,EAX)		/* return base */
#else
	MOV_L	(base_arg,EAX)		/* return base */
#endif
	MOV_L	(REGOFF(0,ESP),EDI)	/* POPL EDI */
	MOV_L	(REGOFF(4,ESP),ESI)	/* POPL ESI */
	MOV_L	(REGOFF(8,ESP),EBX)	/* POPL EBX */
	MOV_L	(REGOFF(12,ESP),EBP)	/* POPL EBP */
	ADD_L	(CONST(16),ESP)
	RET
