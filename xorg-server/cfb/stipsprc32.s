/*
 * $Xorg: stipsprc32.s,v 1.4 2001/02/09 02:04:39 xorgcvs Exp $
 * $XdotOrg:	$
 *
Copyright 1990, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.
 *
 * Author:  Keith Packard, MIT X Consortium
 */
/* $XFree86: xc/programs/Xserver/cfb/stipsprc32.s,v 1.4 2001/01/17 22:36:38 dawes Exp $ */

/*
 * SPARC assembly code for optimized text rendering.
 *
 * Other stippling could be done in assembly, but the payoff is
 * not nearly as large.  Mostly because large areas are heavily
 * optimized already.
 */

/* not that I expect to ever see an LSB SPARC, but ... */
#ifdef LITTLE_ENDIAN
# define BitsR		sll
# define BitsL		srl
# define WO(o)		3-o
# define FourBits(dest,bits)	and	bits, 0xf, dest
#else
# define BitsR		srl
# define BitsL		sll
# define WO(o)		o
# define FourBits(dest,bits)	srl	bits, 28, dest
#endif

/*
 * cfb32StippleStack(addr, stipple, value, stride, Count, Shift)
 *               4       5       6      7     16(sp) 20(sp)
 *
 *  Apply successive 32-bit stipples starting at addr, addr+stride, ...
 *
 *  Used for text rendering, but only when no data could be lost
 *  when the stipple is shifted left by Shift bits
 */
/* arguments */
#define addr	%i0
#define stipple	%i1
#define value	%i2
#define stride	%i3
#define count	%i4
#define shift	%i5

/* local variables */
#define atemp	%l0
#define bits	%l1
#define lshift	%l2
#define sbase	%l3
#define stemp	%l4

#define CASE_SIZE	5	/* case blocks are 2^5 bytes each */
#define CASE_MASK	0x1e0	/* first case mask */

#define ForEachLine	LY1
#define NextLine	LY2
#define CaseBegin	LY3
#define ForEachBits	LY4
#define NextBits	LY5

#if defined(SVR4) || defined(__ELF__)
#ifdef TETEXT
#define	_cfb32StippleStack	cfb32StippleStackTE
#else
#define	_cfb32StippleStack	cfb32StippleStack
#endif
#else
#ifdef TETEXT
#define	_cfb32StippleStack	_cfb32StippleStackTE
#endif
#endif

	.seg	"text"
	.proc	16
	.globl	_cfb32StippleStack
_cfb32StippleStack:
	save	%sp,-64,%sp
#ifdef SHAREDCODE
1:
        call    2f
        nop
2:
        mov     %o7,sbase                       /* sbase = 1b(1:) */
        add     sbase, CaseBegin-1b, sbase
#else /* !SHAREDCODE */
	sethi	%hi(CaseBegin),sbase		/* load up switch table */
	or	sbase,%lo(CaseBegin),sbase
#endif /* !SHAREDCODE */
	mov	4,lshift			/* compute offset within */
	sub	lshift, shift, lshift		/*  stipple of remaining bits */
#ifdef LITTLE_ENDIAN
	inc	CASE_SIZE, shift		/* first shift for LSB */
#else
	inc	28-CASE_SIZE, shift		/* first shift for MSB */
#endif
	/* do ... while (--count > 0); */
ForEachLine:
	ld	[stipple],bits			/* get stipple bits */
	mov	addr,atemp			/* set up for this line */
#ifdef TETEXT
	/* Terminal emulator fonts are expanded and have many 0 rows */
	tst	bits
	bz	NextLine			/* skip out early on 0 */
#endif
	add	addr, stride, addr		/* step for the loop */
	BitsR	bits, shift, stemp		/* get first bits */
	and	stemp, CASE_MASK, stemp		/* compute first jump */
	BitsL	bits, lshift, bits		/* set remaining bits */
	jmp	sbase+stemp			/*  ... */
	tst	bits

ForEachBits:
	inc	16, atemp
ForEachBits1:
	FourBits(stemp, bits)			/* compute jump for */
	sll	stemp, CASE_SIZE, stemp		/*  these four bits */
	BitsL	bits, 4, bits			/* step for remaining bits */
	jmp	sbase+stemp			/* jump */
	tst	bits
CaseBegin:
	bnz,a	ForEachBits1			/* 0 */
	inc	16, atemp
NextLine:
	deccc	1, count
NextLine1:
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop

	bnz	ForEachBits			/* 1 */
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	bnz	ForEachBits			/* 2 */
	st	value, [atemp+WO(8)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	st	value, [atemp+WO(8)]		/* 3 */
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	bnz	ForEachBits			/* 4 */
	st	value, [atemp+WO(4)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	st	value, [atemp+WO(4)]		/* 5 */
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(4)]		/* 6 */
	bnz	ForEachBits
	st	value, [atemp+WO(8)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(4)]		/* 7 */
	st	value, [atemp+WO(8)]
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	b	NextLine1
	deccc	1, count
	nop
	nop
					
	bnz	ForEachBits			/* 8 */
	st	value, [atemp+WO(0)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
	nop
					
	st	value, [atemp+WO(0)]		/* 9 */
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(0)]		/* a */
	bnz	ForEachBits
	st	value, [atemp+WO(8)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(0)]		/* b */
	st	value, [atemp+WO(8)]
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	b	NextLine1
	deccc	1, count
	nop
	nop
					
	st	value, [atemp+WO(0)]		/* c */
	bnz	ForEachBits
	st	value, [atemp+WO(4)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
					
	st	value, [atemp+WO(0)]		/* d */
	st	value, [atemp+WO(4)]
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	b	NextLine1
	deccc	1, count
	nop
	nop
					
	st	value, [atemp+WO(0)]		/* e */
	st	value, [atemp+WO(4)]
	bnz	ForEachBits
	st	value, [atemp+WO(8)]
	b	NextLine1
	deccc	1, count
	nop
	nop
					
	st	value, [atemp+WO(0)]		/* f */
	st	value, [atemp+WO(4)]
	st	value, [atemp+WO(8)]
	bnz	ForEachBits
	st	value, [atemp+WO(12)]
	deccc	1, count
	bnz,a	ForEachLine
	inc	4, stipple
	ret
	restore
