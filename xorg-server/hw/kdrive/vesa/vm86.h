/*
 * Copyright © 2000 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* 
Copyright (c) 2000 by Juliusz Chroboczek
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions: 
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef _VM86_H_
#define _VM86_H_

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/vm86.h>
#include <sys/io.h>

#ifdef NOT_IN_X_SERVER
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
static void ErrorF(char*, ...);
#define xalloc(a) malloc(a)
#define xcalloc(a,b) calloc(a,b)
#define xfree(a) free(a)
#else
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xos.h>
#include "os.h"
#endif

#ifndef IF_MASK
#define IF_MASK X86_EFLAGS_IF
#endif
#ifndef IOPL_MASK
#define IOPL_MASK X86_EFLAGS_IOPL
#endif

typedef unsigned char	U8;
typedef unsigned short	U16;
typedef unsigned int	U32;

/* The whole addressable memory */
#define SYSMEM_BASE 0x00000
#define SYSMEM_SIZE 0x100000

/* Interrupt vectors and BIOS data area */
/* This is allocated privately from /dev/mem */
#define MAGICMEM_BASE 0x00000
#define MAGICMEM_SIZE 0x01000

/* The low memory, allocated privately from /dev/zero */
/* 64KB should be enough for anyone, as they used to say */
#define LOMEM_BASE 0x10000
#define LOMEM_SIZE 0x10000

/* The video memory and BIOS ROM, allocated shared from /dev/mem */
#define HIMEM_BASE 0xA0000
#define HIMEM_SIZE (SYSMEM_BASE + SYSMEM_SIZE - HIMEM_BASE)

#define HOLE1_BASE (MAGICMEM_BASE + MAGICMEM_SIZE)
#define HOLE1_SIZE (LOMEM_BASE - HOLE1_BASE)

#define HOLE2_BASE (LOMEM_BASE + LOMEM_SIZE)
#define HOLE2_SIZE (HIMEM_BASE - HOLE2_BASE)

/* The BIOS ROM */
#define ROM_BASE 0xC0000
#define ROM_SIZE 0x30000

#define STACK_SIZE 0x1000

#define POINTER_SEGMENT(ptr) (((unsigned int)ptr)>>4)
#define POINTER_OFFSET(ptr) (((unsigned int)ptr)&0x000F)
#define MAKE_POINTER(seg, off) (((((unsigned int)(seg))<<4) + (unsigned int)(off)))
#define MAKE_POINTER_1(lw) MAKE_POINTER(((lw)&0xFFFF0000)/0x10000, (lw)&0xFFFF)
#define ALLOC_FAIL ((U32)-1)

typedef struct _Vm86InfoRec {
    void		*magicMem, *loMem, *hiMem;
    void                *hole1, *hole2;
    U32			brk;
    struct vm86_struct	vms;
    U32			ret_code, stack_base;
} Vm86InfoRec, *Vm86InfoPtr;

#define LM(vi,i) (((char*)vi->loMem)[i-LOMEM_BASE])
#define LMW(vi,i) (*(U16*)(&LM(vi,i)))
#define LML(vi,i) (*(U32*)(&LM(vi,i)))
#define MM(vi,i) (((char*)vi->magicMem)[i-MAGICMEM_BASE])
#define MMW(vi,i) (*(U16*)(&MM(vi,i)))
#define MML(vi,i) (*(U32*)(&MM(vi,i)))
#define HM(vi,i) (((char*)vi->hiMem)[i-HIMEM_BASE])
#define HMW(vi,i) (*(U16*)(&MM(vi,i)))
#define HML(vi,i) (*(U32*)(&MM(vi,i)))

Vm86InfoPtr
Vm86Setup(int);
    
void
Vm86Cleanup(Vm86InfoPtr vi);

int
Vm86DoInterrupt(Vm86InfoPtr vi, int num);

int
Vm86DoPOST(Vm86InfoPtr vi);

int
Vm86IsMemory(Vm86InfoPtr vi, U32 i);

U8
Vm86Memory(Vm86InfoPtr, U32);

U16
Vm86MemoryW(Vm86InfoPtr, U32);

U32
Vm86MemoryL(Vm86InfoPtr, U32);

void
Vm86WriteMemory(Vm86InfoPtr, U32, U8);

void
Vm86WriteMemoryW(Vm86InfoPtr, U32, U16);

void
Vm86WriteMemoryL(Vm86InfoPtr, U32, U32);

int
Vm86AllocateMemory(Vm86InfoPtr, int);

int
Vm86MarkMemory (Vm86InfoPtr vi);

void
Vm86ReleaseMemory (Vm86InfoPtr vi, int mark);

void
Vm86Debug(Vm86InfoPtr vi);

#endif /* _VM86_H_ */
