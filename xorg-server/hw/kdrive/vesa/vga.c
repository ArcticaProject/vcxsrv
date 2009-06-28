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

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "vesa.h"

static const VesaModeRec vgaModes[] = {
    {
	6,  0,
	MODE_SUPPORTED | MODE_GRAPHICS | MODE_VGA | MODE_LINEAR,
	1, 1, MEMORY_PLANAR, 
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	640, 200, 80,
    },
    {
	0xd, 0,
	MODE_SUPPORTED | MODE_GRAPHICS | MODE_VGA | MODE_COLOUR,
	4, 4, MEMORY_PLANAR, 
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	320, 200, 40,
    },
    {
	0xe, 0,
	MODE_SUPPORTED | MODE_GRAPHICS | MODE_VGA | MODE_COLOUR,
	4, 4, MEMORY_PLANAR, 
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	640, 200, 80,
    },
    {
	0x10, 0,
	MODE_SUPPORTED | MODE_GRAPHICS | MODE_VGA | MODE_COLOUR,
	4, 4, MEMORY_PLANAR, 
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	640, 350, 80,
    },
    {
	0x11, 0,
	MODE_SUPPORTED | MODE_GRAPHICS | MODE_VGA | MODE_LINEAR,
	1, 1, MEMORY_PLANAR, 
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	640, 480, 80,
    },
    {
	0x12, 0,
	MODE_SUPPORTED | MODE_GRAPHICS | MODE_VGA | MODE_COLOUR,
	4, 4, MEMORY_PLANAR, 
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	640, 480, 80,
    },
    {
	0x13, 0,
	MODE_SUPPORTED | MODE_GRAPHICS | MODE_VGA | MODE_COLOUR | MODE_LINEAR,
	8, 8, MEMORY_PSEUDO,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	320, 200, 320,
    },
};

#define NUM_VGA_MODE	(sizeof vgaModes / sizeof vgaModes[0])

int
VgaGetNmode (Vm86InfoPtr vi)
{
    return NUM_VGA_MODE;
}

int
VgaGetModes (Vm86InfoPtr vi, VesaModePtr mode, int nmode)
{
    if (nmode > NUM_VGA_MODE)
	nmode = NUM_VGA_MODE;
    memcpy (mode, vgaModes, nmode * sizeof (VesaModeRec));
    return nmode;
}

int
VgaSetMode(Vm86InfoPtr vi, int mode)
{
    int code;
    
    vi->vms.regs.eax = mode & 0x7f;
    code = Vm86DoInterrupt (vi, 0x10);
    if(code < 0)
        return -1;
    return 0;
}

int
VgaGetMode (Vm86InfoPtr vi, int *mode)
{
    *mode = Vm86Memory (vi, 0x449);
    return 0;
}

void
VgaSetWritePlaneMask(Vm86InfoPtr vi, int mask)
{
    asm volatile ("outb %b0,%w1" : : "a" (2), "d" (0x3c4));
    asm volatile ("outb %b0,%w1" : : "a" (mask), "d" (0x3c5));
}

void
VgaSetReadPlaneMap(Vm86InfoPtr vi, int map)
{
    asm volatile ("outb %b0,%w1" : : "a" (4), "d" (0x3ce));
    asm volatile ("outb %b0,%w1" : : "a" (map), "d" (0x3cf));
}

int 
VgaSetPalette(Vm86InfoPtr vi, int first, int number, U8 *entries)
{
    U8	    *palette_scratch;
    int	    mark;
    int	    palette_base;
    int	    i, j, code;

    if(number == 0)
        return 0;

    if(first < 0 || number < 0 || first + number > 256) {
        ErrorF("Cannot set %d, %d palette entries\n", first, number);
        return -1;
    }

    mark = Vm86MarkMemory (vi);
    palette_base = Vm86AllocateMemory (vi, 3 * 256);
    
    palette_scratch = &LM(vi, palette_base);

    vi->vms.regs.eax = 0x1012;
    vi->vms.regs.ebx = first;
    vi->vms.regs.ecx = number;
    vi->vms.regs.es = POINTER_SEGMENT(palette_base);
    vi->vms.regs.edx = POINTER_OFFSET(palette_base);
    j = 0;
    i = 0;
    while (number--)
    {
	palette_scratch[j++] = entries[i++] >> 2;
	palette_scratch[j++] = entries[i++] >> 2;
	palette_scratch[j++] = entries[i++] >> 2;
	i++;
    }
    code = Vm86DoInterrupt(vi, 0x10);
    Vm86ReleaseMemory (vi, mark);
    
    if(code < 0)
	return -1;
    return 0;
}
        
int 
VgaGetPalette(Vm86InfoPtr vi, int first, int number, U8 *entries)
{
    U8	    *palette_scratch;
    int	    mark;
    int	    palette_base;
    int	    i, j, code;

    if(number == 0)
        return 0;

    if(first < 0 || number < 0 || first + number > 256) {
        ErrorF("Cannot get %d, %d palette entries\n", first, number);
        return -1;
    }

    mark = Vm86MarkMemory (vi);
    palette_base = Vm86AllocateMemory (vi, 3 * 256);
    
    palette_scratch = &LM(vi, palette_base);

    vi->vms.regs.eax = 0x1017;
    vi->vms.regs.ebx = first;
    vi->vms.regs.ecx = number;
    vi->vms.regs.es = POINTER_SEGMENT(palette_base);
    vi->vms.regs.edx = POINTER_OFFSET(palette_base);
    
    code = VbeDoInterrupt10(vi);
    if(code < 0)
	return -1;
    
    j = 0;
    i = 0;
    while (number--)
    {
	entries[i++] = palette_scratch[j++] << 2;
	entries[i++] = palette_scratch[j++] << 2;
	entries[i++] = palette_scratch[j++] << 2;
	entries[i++] = 0;
    }
    
    Vm86ReleaseMemory (vi, mark);

    return 0;
}   
        
#define VGA_FB(vm)  ((vm) < 8 ? 0xb8000 : 0xa0000)

void *
VgaSetWindow (Vm86InfoPtr vi, int vmode, int bytes, int mode, int *size)
{
    *size = 0x10000 - bytes;
    return &LM(vi,VGA_FB(vmode) + bytes);
}

void *
VgaMapFramebuffer (Vm86InfoPtr vi, int vmode, int *size, CARD32 *ret_phys)
{
    if (VGA_FB(vmode) == 0xa0000)
	*size = 0x10000;
    else
	*size = 0x4000;
    *ret_phys = VGA_FB(vmode);
    return &LM(vi,VGA_FB(vmode));
}

void
VgaUnmapFramebuffer (Vm86InfoPtr vi)
{
}
