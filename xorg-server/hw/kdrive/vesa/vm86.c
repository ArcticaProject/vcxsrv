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

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "vm86.h"

#define PUSHW(vi, i) \
{ vi->vms.regs.esp -= 2;\
  LMW(vi,MAKE_POINTER(vi->vms.regs.ss, vi->vms.regs.esp)) = i;}

static int vm86old(struct vm86_struct *vms);
static int vm86_loop(Vm86InfoPtr vi);

static const U8 rev_ints[32] =
{ 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0x80,
};

static const U8 retcode_data[2] =
{ 0xCD, 0xFF };

Vm86InfoPtr
Vm86Setup(int mapHoles)
{
    int devmem = -1, devzero = -1;
    void *magicMem, *loMem, *hiMem;
    void *hole1, *hole2;
    U32 stack_base, ret_code;
    Vm86InfoPtr vi = NULL;

    devmem = open("/dev/mem", O_RDWR);
    if(devmem < 0) {
	perror("open /dev/mem");
	goto fail;
    }

    devzero = open("/dev/zero", O_RDWR);
    if(devzero < 0) {
	perror("open /dev/zero");
	goto fail;
    }

    magicMem = MAP_FAILED;
    loMem = MAP_FAILED;
    hiMem = MAP_FAILED;
    hole1 = MAP_FAILED;
    hole2 = MAP_FAILED;


    magicMem = mmap((void*)MAGICMEM_BASE, MAGICMEM_SIZE,
		    PROT_READ | PROT_WRITE | PROT_EXEC,
		    MAP_PRIVATE | MAP_FIXED, devmem, MAGICMEM_BASE);
    
    if(magicMem == MAP_FAILED) {
	ErrorF("Couldn't map magic memory\n");
	goto unmapfail;
    }

    if(mapHoles) {
        hole1 = mmap((void*)HOLE1_BASE, HOLE1_SIZE,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_FIXED, devzero, HOLE1_BASE);
    
        if(hole1 == MAP_FAILED) {
            ErrorF("Couldn't map first hole\n");
            goto unmapfail;
        }
    }

    loMem = mmap((void*)LOMEM_BASE, LOMEM_SIZE,
		 PROT_READ | PROT_WRITE | PROT_EXEC,
		 MAP_PRIVATE | MAP_FIXED, devzero, LOMEM_BASE);
    if(loMem == MAP_FAILED) {
	ErrorF("Couldn't map low memory\n");
	munmap(magicMem, MAGICMEM_SIZE);
	goto unmapfail;
    }

    if(mapHoles) {
        hole2 = mmap((void*)HOLE2_BASE, HOLE2_SIZE,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_FIXED, devzero, HOLE2_BASE);
    
        if(hole2 == MAP_FAILED) {
            ErrorF("Couldn't map first hole\n");
            goto unmapfail;
        }
    }

    hiMem = mmap((void*)HIMEM_BASE, HIMEM_SIZE,
		 PROT_READ | PROT_WRITE | PROT_EXEC,
		 MAP_SHARED | MAP_FIXED,
		 devmem, HIMEM_BASE);
    if(hiMem == MAP_FAILED) {
	ErrorF("Couldn't map high memory\n");
	goto unmapfail;
    }

    vi = xalloc(sizeof(Vm86InfoRec));
    if (!vi)
	goto unmapfail;

    vi->magicMem = magicMem;
    vi->hole1 = hole1;
    vi->loMem = loMem;
    vi->hole2 = hole2;
    vi->hiMem = hiMem;
    vi->brk = LOMEM_BASE;

    stack_base = Vm86AllocateMemory(vi, STACK_SIZE);
    if(stack_base == ALLOC_FAIL)
	goto unmapfail;
    ret_code = Vm86AllocateMemory(vi, sizeof(retcode_data));
    if(ret_code == ALLOC_FAIL)
	goto unmapfail;

    vi->stack_base = stack_base;
    vi->ret_code = ret_code;

    memset(&vi->vms, 0, sizeof(struct vm86_struct));
    vi->vms.flags = 0;
    vi->vms.screen_bitmap = 0;
    vi->vms.cpu_type = CPU_586;
    memcpy(&vi->vms.int_revectored, rev_ints, sizeof(rev_ints));

    iopl(3);
    
    if(devmem >= 0)
	close(devmem);
    if(devzero >= 0)
	close(devzero);

    return vi;

unmapfail:
    if(magicMem != MAP_FAILED) munmap(magicMem, MAGICMEM_SIZE);
    if(hole1 != MAP_FAILED) munmap(hole1, HOLE1_SIZE);
    if(loMem != MAP_FAILED) munmap(loMem, LOMEM_SIZE);
    if(hole2 != MAP_FAILED) munmap(hole2, HOLE2_SIZE);
    if(hiMem != MAP_FAILED) munmap(hiMem, HIMEM_SIZE);
fail:
    if(devmem >= 0)
	close(devmem);
    if(devzero >= 0)
	close(devzero);
    if(vi)
	xfree(vi);
    return NULL;
}

void
Vm86Cleanup(Vm86InfoPtr vi)
{
    if(vi->magicMem != MAP_FAILED) munmap(vi->magicMem, MAGICMEM_SIZE);
    if(vi->hole1 != MAP_FAILED) munmap(vi->hole1, HOLE1_SIZE);
    if(vi->loMem != MAP_FAILED) munmap(vi->loMem, LOMEM_SIZE);
    if(vi->hole2 != MAP_FAILED) munmap(vi->hole2, HOLE2_SIZE);
    if(vi->hiMem != MAP_FAILED) munmap(vi->hiMem, HIMEM_SIZE);
    xfree(vi);
}

int
Vm86DoInterrupt(Vm86InfoPtr vi, int num)
{
    U16 seg, off;
    int code;

    if(num < 0 || num>256) {
	ErrorF("Interrupt %d doesn't exist\n");
	return -1;
    }
    seg = MMW(vi,num * 4 + 2);
    off = MMW(vi,num * 4);
    if(MAKE_POINTER(seg, off) < ROM_BASE ||
       MAKE_POINTER(seg, off) >= ROM_BASE + ROM_SIZE) {
	ErrorF("Interrupt pointer (seg %x off %x) doesn't point at ROM\n",
	       seg, off);
	return -1;
    }
    memcpy(&(LM(vi,vi->ret_code)), retcode_data, sizeof(retcode_data));
    vi->vms.regs.eflags = IF_MASK | IOPL_MASK;
    vi->vms.regs.ss = POINTER_SEGMENT(vi->stack_base);
    vi->vms.regs.esp = STACK_SIZE;
    PUSHW(vi, IF_MASK | IOPL_MASK);
    PUSHW(vi, POINTER_SEGMENT(vi->ret_code));
    PUSHW(vi, POINTER_OFFSET(vi->ret_code));
    vi->vms.regs.cs = seg;
    vi->vms.regs.eip = off;
    OsBlockSignals ();
    code = vm86_loop(vi);
    OsReleaseSignals ();
    if(code != 0)
	return -1;
    else
	return 0;
}

int
Vm86DoPOST(Vm86InfoPtr vi)
{
    U16 seg, off;
    int code;

    seg = 0xC000;
    off = 3;
    if(MAKE_POINTER(seg, off) < ROM_BASE ||
       MAKE_POINTER(seg, off) >= ROM_BASE + ROM_SIZE) {
	ErrorF("BIOS pointer (seg %x off %x) doesn't point at ROM\n",
	       seg, off);
	return -1;
    }
    memcpy(&(LM(vi,vi->ret_code)), retcode_data, sizeof(retcode_data));
    vi->vms.regs.ss = POINTER_SEGMENT(vi->stack_base);
    vi->vms.regs.esp = STACK_SIZE;
    PUSHW(vi, POINTER_SEGMENT(vi->ret_code));
    PUSHW(vi, POINTER_OFFSET(vi->ret_code));
    vi->vms.regs.cs = seg;
    vi->vms.regs.eip = off;
    OsBlockSignals ();
    code = vm86_loop(vi);
    OsReleaseSignals ();
    if(code != 0)
	return -1;
    else
        return 0;
}

#define DEBUG_VBE 0
#if DEBUG_VBE
#define DBG(x) ErrorF x; usleep(10*1000)
#else
#define DBG(x)
#endif

static inline U8 
vm86_inb(U16 port)
{
    U8 value;
    
    if (port != 0x3da)
    {
	DBG(("inb  0x%04x", port));
    }
    asm volatile ("inb %w1,%b0" : "=a" (value) : "d" (port));
    if (port != 0x3da)
    {
	DBG((" = 0x%02x\n", value));
    }
    return value;
}

static inline U16
vm86_inw(U16 port)
{
    U16 value;
    DBG(("inw  0x%04x", port));
    asm volatile ("inw %w1,%w0" : "=a" (value) : "d" (port));
    DBG((" = 0x%04x\n", value));
    return value;
}

static inline U32
vm86_inl(U16 port)
{
    U32 value;
    DBG(("inl  0x%04x", port));
    asm volatile ("inl %w1,%0" : "=a" (value) : "d" (port));
    DBG((" = 0x%08x\n", value));
    return value;
}

static inline void
vm86_outb(U16 port, U8 value)
{
#if 0
    static U8 CR;

    if (port == 0x3d4)
	CR = value;
    if (port == 0x3d5 && CR == 0xa4)
    {
	DBG(("outb 0x%04x = 0x%02x (skipped)\n", port, value));
	return;
    }
#endif
    DBG(("outb 0x%04x = 0x%02x\n", port, value));
    asm volatile ("outb %b0,%w1" : : "a" (value), "d" (port));
}

static inline void
vm86_outw(U16 port, U16 value)
{
    DBG(("outw 0x%04x = 0x%04x\n", port, value));
    asm volatile ("outw %w0,%w1" : : "a" (value), "d" (port));
}

static inline void
vm86_outl(U16 port, U32 value)
{
    DBG(("outl 0x%04x = 0x%08x\n", port, value));
    asm volatile ("outl %0,%w1" : : "a" (value), "d" (port));
}

#define SEG_CS 1
#define SEG_DS 2
#define SEG_ES 3
#define SEG_SS 4
#define SEG_GS 5
#define SEG_FS 6
#define REP 1
#define REPNZ 2
#define SET_8(_x, _y) (_x) = ((_x) & ~0xFF) | ((_y) & 0xFF);
#define SET_16(_x, _y) (_x) = ((_x) & ~0xFFFF) | ((_y) & 0xFFFF);
#define INC_IP(_i) SET_16(regs->eip, (regs->eip + _i))
#define AGAIN INC_IP(1); goto again;

static int
vm86_emulate(Vm86InfoPtr vi)
{
    struct vm86_regs *regs = &vi->vms.regs;
    U8 opcode;
    int size;
    int pref_seg = 0, pref_rep = 0, pref_66 = 0, pref_67 = 0;

  again:
    if(!Vm86IsMemory(vi, MAKE_POINTER(regs->cs, regs->eip))) {
        ErrorF("Trying to execute unmapped memory\n");
        return -1;
    }
    opcode = Vm86Memory(vi, MAKE_POINTER(regs->cs, regs->eip));
    switch(opcode) {
    case 0x2E: pref_seg = SEG_CS; AGAIN;
    case 0x3E: pref_seg = SEG_DS; AGAIN;
    case 0x26: pref_seg = SEG_ES; AGAIN;
    case 0x36: pref_seg = SEG_SS; AGAIN;
    case 0x65: pref_seg = SEG_GS; AGAIN;
    case 0x64: pref_seg = SEG_FS; AGAIN;
    case 0x66: pref_66 = 1; AGAIN;
    case 0x67: pref_67 = 1; AGAIN;
    case 0xF2: pref_rep = REPNZ; AGAIN;
    case 0xF3: pref_rep = REP; AGAIN;

    case 0xEC:                  /* IN AL, DX */
        SET_8(regs->eax, vm86_inb(regs->edx & 0xFFFF));
        INC_IP(1);
        break;
    case 0xED:                  /* IN AX, DX */
        if(pref_66)
            regs->eax = vm86_inl(regs->edx & 0xFFFF);
        else
            SET_16(regs->eax, vm86_inw(regs->edx & 0xFFFF));
	INC_IP(1);
        break;
    case 0xE4:                  /* IN AL, imm8 */
        SET_8(regs->eax, 
              vm86_inb(Vm86Memory(vi, MAKE_POINTER(regs->cs, regs->eip+1))));
        INC_IP(2);
        break;
    case 0xE5:                  /* IN AX, imm8 */
        if(pref_66)
            regs->eax =
                vm86_inl(Vm86Memory(vi, MAKE_POINTER(regs->cs, regs->eip+1)));
        else
            SET_16(regs->eax, 
                   vm86_inw(Vm86Memory(vi, MAKE_POINTER(regs->cs, regs->eip+1))));
        INC_IP(2);
        break;
    case 0x6C:                  /* INSB */
    case 0x6D:                  /* INSW */
        if(opcode == 0x6C) {
            Vm86WriteMemory(vi, MAKE_POINTER(regs->es, regs->edi),
                vm86_inb(regs->edx & 0xFFFF));
            size = 1;
        } else if(pref_66) {
            Vm86WriteMemoryL(vi, MAKE_POINTER(regs->es, regs->edi),
                vm86_inl(regs->edx & 0xFFFF));
            size = 4;
        } else {
            Vm86WriteMemoryW(vi, MAKE_POINTER(regs->es, regs->edi),
                vm86_inw(regs->edx & 0xFFFF));
            size = 2;
        }
        if(regs->eflags & (1<<10))
            regs->edi -= size;
        else
            regs->edi += size;
        if(pref_rep) {
            if(pref_66) {
                regs->ecx--;
                if(regs->ecx != 0)
                    goto again;
            } else {
                SET_16(regs->ecx, regs->ecx - 1);
                if((regs->ecx & 0xFFFF) != 0)
                    goto again;
            }
        }
        INC_IP(1);
        break;

    case 0xEE:                  /* OUT DX, AL */
        vm86_outb(regs->edx & 0xFFFF, regs->eax & 0xFF);
        INC_IP(1);
        break;
    case 0xEF:                  /* OUT DX, AX */
        if(pref_66)
            vm86_outl(regs->edx & 0xFFFF, regs->eax);
        else
            vm86_outw(regs->edx & 0xFFFF, regs->eax & 0xFFFF);
        INC_IP(1);
        break;
    case 0xE6:                  /* OUT imm8, AL */
        vm86_outb(Vm86Memory(vi, MAKE_POINTER(regs->cs, regs->eip+1)),
             regs->eax & 0xFF);
        INC_IP(2);
        break;
    case 0xE7:                  /* OUT imm8, AX */
        if(pref_66)
            vm86_outl(Vm86Memory(vi, MAKE_POINTER(regs->cs, regs->eip+1)),
                  regs->eax);
        else
            vm86_outw(Vm86Memory(vi, MAKE_POINTER(regs->cs, regs->eip+1)),
                 regs->eax & 0xFFFF);
        INC_IP(2);
        break;
    case 0x6E:                  /* OUTSB */
    case 0x6F:                  /* OUTSW */
        if(opcode == 0x6E) {
            vm86_outb(regs->edx & 0xFFFF, 
                 Vm86Memory(vi, MAKE_POINTER(regs->es, regs->edi)));
            size = 1;
        } else if(pref_66) {
            vm86_outl(regs->edx & 0xFFFF, 
                 Vm86Memory(vi, MAKE_POINTER(regs->es, regs->edi)));
            size = 4;
        } else {
            vm86_outw(regs->edx & 0xFFFF, 
                 Vm86Memory(vi, MAKE_POINTER(regs->es, regs->edi)));
            size = 2;
        }
        if(regs->eflags & (1<<10))
            regs->edi -= size;
        else
            regs->edi += size;
        if(pref_rep) {
            if(pref_66) {
                regs->ecx--;
                if(regs->ecx != 0)
                    goto again;
            } else {
                SET_16(regs->ecx, regs->ecx - 1);
                if((regs->ecx & 0xFFFF) != 0)
                    goto again;
            }
        }
        INC_IP(1);
        break;

    case 0x0F:
        ErrorF("Hit 0F trap in VM86 code\n");
        return -1;
    case 0xF0:
        ErrorF("Hit lock prefix in VM86 code\n");
        return -1;
    case 0xF4:
        ErrorF("Hit HLT in VM86 code\n");
        return -1;

    default:
        ErrorF("Unhandled GP fault in VM86 code (opcode = 0x%02X)\n",
               opcode);
        return -1;
    }
    return 0;
}
#undef SEG_CS
#undef SEG_DS
#undef SEG_ES
#undef SEG_SS
#undef SEG_GS
#undef SEG_FS
#undef REP
#undef REPNZ
#undef SET_8
#undef SET_16
#undef INC_IP
#undef AGAIN

static int
vm86_loop(Vm86InfoPtr vi)
{
    int code;
    
    while(1) {
        code = vm86old(&vi->vms);
        switch(VM86_TYPE(code)) {
        case VM86_SIGNAL:
            continue;
        case VM86_UNKNOWN:
            code = vm86_emulate(vi);
            if(code < 0) {
                Vm86Debug(vi);
                return -1;
            }
            break;
        case VM86_INTx:
            if(VM86_ARG(code) == 0xFF)
                return 0;
            else {
                PUSHW(vi, vi->vms.regs.eflags)
                PUSHW(vi, vi->vms.regs.cs);
                PUSHW(vi, vi->vms.regs.eip);
                vi->vms.regs.cs = MMW(vi,VM86_ARG(code) * 4 + 2);
                vi->vms.regs.eip = MMW(vi,VM86_ARG(code) * 4);
            }
            break;
        case VM86_STI:
            ErrorF("VM86 code enabled interrupts\n");
            Vm86Debug(vi);
            return -1;
        default:
            if(code < 0) {
                if(errno == ENOSYS) {
                    ErrorF("No vm86 support.  Are you running on AMD64?\n");
                } else {
                    ErrorF("vm86 failed (%s).\n", strerror(errno));
                    Vm86Debug(vi);
                }
            } else {
                ErrorF("Unexpected result code 0x%X from vm86\n", code);
                Vm86Debug(vi);
            }
            return -1;
        }
    }
}

int 
Vm86IsMemory(Vm86InfoPtr vi, U32 i) 
{
    if(i >= MAGICMEM_BASE && i< MAGICMEM_BASE + MAGICMEM_SIZE)
        return 1;
    else if(i >= LOMEM_BASE && i< LOMEM_BASE + LOMEM_SIZE)
        return 1;
    else if(i >= HIMEM_BASE && i< HIMEM_BASE + HIMEM_SIZE)
        return 1;
    else
        return 0;
}

U8 
Vm86Memory(Vm86InfoPtr vi, U32 i)
{
    if(i >= MAGICMEM_BASE && i< MAGICMEM_BASE + MAGICMEM_SIZE)
        return MM(vi, i);
    else if(i >= LOMEM_BASE && i< LOMEM_BASE + LOMEM_SIZE)
        return LM(vi, i);
    else if(i >= HIMEM_BASE && i< HIMEM_BASE + HIMEM_SIZE)
        return HM(vi, i);
    else {
        ErrorF("Reading unmapped memory at 0x%08X\n", i);
	return 0;
    }
}

U16
Vm86MemoryW(Vm86InfoPtr vi, U32 i)
{
    if(i >= MAGICMEM_BASE && i< MAGICMEM_BASE + MAGICMEM_SIZE)
        return MMW(vi, i);
    else if(i >= LOMEM_BASE && i< LOMEM_BASE + LOMEM_SIZE)
        return LMW(vi, i);
    else if(i >= HIMEM_BASE && i< HIMEM_BASE + HIMEM_SIZE)
        return HMW(vi, i);
    else {
        ErrorF("Reading unmapped memory at 0x%08X\n", i);
        return 0;
    }
}

U32
Vm86MemoryL(Vm86InfoPtr vi, U32 i)
{
    if(i >= MAGICMEM_BASE && i< MAGICMEM_BASE + MAGICMEM_SIZE)
        return MML(vi, i);
    else if(i >= LOMEM_BASE && i< LOMEM_BASE + LOMEM_SIZE)
        return LML(vi, i);
    else if(i >= HIMEM_BASE && i< HIMEM_BASE + HIMEM_SIZE)
        return HML(vi, i);
    else {
        ErrorF("Reading unmapped memory at 0x%08X\n", i);
        return 0;
    }
}

void
Vm86WriteMemory(Vm86InfoPtr vi, U32 i, U8 val)
{
    if(i >= MAGICMEM_BASE && i< MAGICMEM_BASE + MAGICMEM_SIZE)
        MM(vi, i) = val;
    else if(i >= LOMEM_BASE && i< LOMEM_BASE + LOMEM_SIZE)
        LM(vi, i) = val;
    else if(i >= HIMEM_BASE && i< HIMEM_BASE + HIMEM_SIZE)
        HM(vi, i) = val;
    else {
        ErrorF("Writing unmapped memory at 0x%08X\n", i);
    }
}

void
Vm86WriteMemoryW(Vm86InfoPtr vi, U32 i, U16 val)
{
    if(i >= MAGICMEM_BASE && i< MAGICMEM_BASE + MAGICMEM_SIZE)
        MMW(vi, i) = val;
    else if(i >= LOMEM_BASE && i< LOMEM_BASE + LOMEM_SIZE)
        LMW(vi, i) = val;
    else if(i >= HIMEM_BASE && i< HIMEM_BASE + HIMEM_SIZE)
        HMW(vi, i) = val;
    else {
        ErrorF("Writing unmapped memory at 0x%08X\n", i);
    }
}

void
Vm86WriteMemoryL(Vm86InfoPtr vi, U32 i, U32 val)
{
    if(i >= MAGICMEM_BASE && i< MAGICMEM_BASE + MAGICMEM_SIZE)
        MML(vi, i) = val;
    else if(i >= LOMEM_BASE && i< LOMEM_BASE + LOMEM_SIZE)
        LML(vi, i) = val;
    else if(i >= HIMEM_BASE && i< HIMEM_BASE + HIMEM_SIZE)
        HML(vi, i) = val;
    else {
        ErrorF("Writing unmapped memory at 0x%08X\n", i);
    }
}

int
Vm86AllocateMemory(Vm86InfoPtr vi, int n)
{
    int ret;
    if(n<0) {
        ErrorF("Asked to allocate negative amount of memory\n");
        return vi->brk;
    }
      
    n = (n + 15) & ~15;
    if(vi->brk + n > LOMEM_BASE + LOMEM_SIZE) {
        ErrorF("Out of low memory\n");
        exit(2);
    }
    ret = vi->brk;
    vi->brk += n;
    return ret;
}

int
Vm86MarkMemory (Vm86InfoPtr vi)
{
    return vi->brk;
}

void
Vm86ReleaseMemory (Vm86InfoPtr vi, int mark)
{
    vi->brk = mark;
}

static int
vm86old(struct vm86_struct *vm)
{
    int res;
    
    asm volatile (
	"pushl %%ebx\n\t"
	"movl %2, %%ebx\n\t"
	"movl %1,%%eax\n\t"
	"int $0x80\n\t"
	"popl %%ebx"
	: "=a" (res)  : "n" (113), "r" (vm));
    if(res < 0) {
	errno = -res;
	res = -1;
    } else 
	errno = 0;
    return res;
}

void
Vm86Debug(Vm86InfoPtr vi)
{
    struct vm86_regs *regs = &vi->vms.regs;
    int i;

    ErrorF("eax=0x%08lX ebx=0x%08lX ecx=0x%08lX edx=0x%08lX\n",
           regs->eax, regs->ebx, regs->ecx, regs->edx);
    ErrorF("esi=0x%08lX edi=0x%08lX ebp=0x%08lX\n",
           regs->esi, regs->edi, regs->ebp);
    ErrorF("eip=0x%08lX esp=0x%08lX eflags=0x%08lX\n",
           regs->eip, regs->esp, regs->eflags);
    ErrorF("cs=0x%04lX      ds=0x%04lX      es=0x%04lX      fs=0x%04lX      gs=0x%04lX\n",
           regs->cs, regs->ds, regs->es, regs->fs, regs->gs);
    for(i=-7; i<8; i++) {
        ErrorF(" %s%02X",
               i==0?"->":"",
               Vm86Memory(vi, MAKE_POINTER(regs->cs, regs->eip + i)));
    }
    ErrorF("\n");
}

#ifdef NOT_IN_X_SERVER
static void 
ErrorF(char *f, ...)
{
    va_list args;
    va_start(args, f);
    vfprintf(stderr, f, args);
    va_end(args);
}
#endif
