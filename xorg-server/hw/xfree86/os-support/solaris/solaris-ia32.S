/ Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
/ 
/ Permission is hereby granted, free of charge, to any person obtaining a
/ copy of this software and associated documentation files (the
/ "Software"), to deal in the Software without restriction, including
/ without limitation the rights to use, copy, modify, merge, publish,
/ distribute, and/or sell copies of the Software, and to permit persons
/ to whom the Software is furnished to do so, provided that the above
/ copyright notice(s) and this permission notice appear in all copies of
/ the Software and that both the above copyright notice(s) and this
/ permission notice appear in supporting documentation.
/ 
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
/ OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
/ MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
/ OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
/ HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
/ INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
/ FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
/ NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
/ WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
/ 
/ Except as contained in this notice, the name of a copyright holder
/ shall not be used in advertising or otherwise to promote the sale, use
/ or other dealings in this Software without prior written authorization
/ of the copyright holder.

#ifdef INLINE_ASM
#define FUNCTION_START(f,n)	.inline f,n
#define FUNCTION_END(f)		.end
#else
#define _ASM
#include <sys/asm_linkage.h>
#define FUNCTION_START(f,n)	ENTRY(f)
#define FUNCTION_END(f)		SET_SIZE(f)
#endif

	FUNCTION_START(inb,4)
	movl	(%esp), %edx
	xorl    %eax, %eax
	inb	(%dx)
	FUNCTION_END(inb)

	FUNCTION_START(inw,4)
	movl	(%esp), %edx
	xorl    %eax, %eax
	inw	(%dx)
	FUNCTION_END(inw)

	FUNCTION_START(inl,4)
	movl	(%esp), %edx
	xorl    %eax, %eax
	inl	(%dx)
	FUNCTION_END(inl)

	FUNCTION_START(outb,8)
	movl	(%esp), %edx
	movl    4(%esp), %eax
	outb	(%dx)
	FUNCTION_END(outb)

	FUNCTION_START(outw,8)
	movl	(%esp), %edx
	movl    4(%esp), %eax
	outw	(%dx)
	FUNCTION_END(outw)

	FUNCTION_START(outl,8)
	movl	(%esp), %edx
	movl    4(%esp), %eax
	outl	(%dx)
	FUNCTION_END(outl)

