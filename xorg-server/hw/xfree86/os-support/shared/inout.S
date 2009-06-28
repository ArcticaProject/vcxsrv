/* $XConsortium: inout.s /main/6 1996/02/21 17:53:35 kaleb $ */





/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/shared/inout.S,v 1.1 1999/07/10 07:24:52 dawes Exp $ */

#include "assyntax.h"

/*
 *	Make i80386 io primitives available at C-level.
 */

	FILE("inout.s")
	AS_BEGIN
	SEG_TEXT

/*
 *-----------------------------------------------------------------------
 * inb ---
 *	Input one byte.
 *
 * Results:
 *      Byte in al.
 *-----------------------------------------------------------------------
 */
	GLOBL	GLNAME(inb)
GLNAME(inb):
	MOV_L	(REGOFF(4,ESP),EDX)
	SUB_L	(EAX,EAX)
	IN_B
	RET

/*
 *-----------------------------------------------------------------------
 * outb ---
 *	Output one byte.
 *
 * Results:
 *      None.
 *-----------------------------------------------------------------------
 */
	GLOBL	GLNAME(outb)
GLNAME(outb):
	MOV_L	(REGOFF(4,sp),EDX)
	MOV_L	(REGOFF(8,sp),EAX)
	OUT_B
	RET
/*
 *-----------------------------------------------------------------------
 * inw ---
 *	Input one 16-bit word.
 *
 * Results:
 *      Word in ax.
 *-----------------------------------------------------------------------
 */
	GLOBL	GLNAME(inw)
GLNAME(inw):
	MOV_L	(REGOFF(4,ESP),EDX)
	IN_W
	RET

/*
 *-----------------------------------------------------------------------
 * outw ---
 *	Output one 16-bit word.
 *
 * Results:
 *      None.
 *-----------------------------------------------------------------------
 */
	GLOBL	GLNAME(outw)
GLNAME(outw):
	MOV_L	(REGOFF(4,ESP),EDX)
	MOV_L	(REGOFF(8,ESP),EAX)
	OUT_W
	RET

/*
 *-----------------------------------------------------------------------
 * inl ---
 *	Input one 32-bit longword.
 *
 * Results:
 *      Word in eax.
 *-----------------------------------------------------------------------
 */
	GLOBL	GLNAME(inl)
GLNAME(inl):
	MOV_L	(REGOFF(4,ESP),EDX)
	IN_L
	RET

/*
 *-----------------------------------------------------------------------
 * outl ---
 *	Output one 32-bit longword.
 *
 * Results:
 *      None.
 *-----------------------------------------------------------------------
 */
	GLOBL	GLNAME(outl)
GLNAME(outl):
	MOV_L	(REGOFF(4,ESP),EDX)
	MOV_L	(REGOFF(8,ESP),EAX)
	OUT_L
	RET

