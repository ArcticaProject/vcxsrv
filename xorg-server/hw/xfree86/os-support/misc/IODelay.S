/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/misc/IODelay.S,v 1.1 1999/07/10 07:24:50 dawes Exp $ */
/*******************************************************************************
                        Copyright 1994 by Glenn G. Lai

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyr notice appear in all copies and that
both that copyr notice and this permission notice appear in
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
glenn@cs.utexas.edu)
7/21/94
*******************************************************************************/
/* $XConsortium: IODelay.s /main/4 1996/02/21 17:40:21 kaleb $ */
 
/* 
 *   All we really need is a delay of about 40ns for I/O recovery for just
 *   about any occasion, but we'll be more conservative here:  On a
 *   100-MHz CPU, produce at least a delay of 1,000ns.
 */ 

#include "assyntax.h"

	FILE("DACDelay.s")

	AS_BEGIN

	GLOBL	GLNAME(xf86IODelay)

	SEG_TEXT
	ALIGNTEXT4
GLNAME(xf86IODelay):
	MOV_L 	(CONST(100), EAX)
delay_it:
	DEC_L	(EAX)
	JNE	(delay_it)
	RET

