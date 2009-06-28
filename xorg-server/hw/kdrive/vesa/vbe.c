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
#include "vesa.h"

int
VbeGetVib (Vm86InfoPtr vi, VbeInfoBlock *vib)
{
    int		    code;
    int		    mark;
    int		    vib_base;
    VbeInfoBlock    *vib_low;
    
    mark = Vm86MarkMemory (vi);
    vib_base = Vm86AllocateMemory (vi, sizeof (VbeInfoBlock));
    vib_low = (VbeInfoBlock*)&(LM(vi, vib_base));
    vi->vms.regs.eax = 0x4F00;
    vi->vms.regs.es = POINTER_SEGMENT(vib_base);
    vi->vms.regs.edi = POINTER_OFFSET(vib_base);
    memcpy(vib_low->VbeSignature, "VBE2", 4);
    code = VbeDoInterrupt10(vi);
    if(code >= 0)
    {
	if(memcmp(vib_low->VbeSignature, "VESA", 4) == 0) 
	    *vib = *vib_low;
	else
	    code = -1;
    }
    Vm86ReleaseMemory (vi, mark);
    return code;
}

int
VbeGetVmib (Vm86InfoPtr vi, int mode, VbeModeInfoBlock *vmib)
{
    int			code;
    int			mark;
    int			vmib_base;
    VbeModeInfoBlock	*vmib_low;
    
    mark = Vm86MarkMemory (vi);
    
    vmib_base = Vm86AllocateMemory (vi, sizeof (VbeModeInfoBlock));
    vmib_low = (VbeModeInfoBlock*)&(LM(vi, vmib_base));
    
    vi->vms.regs.eax = 0x4F01;
    vi->vms.regs.ecx = mode&0xFFFF;
    vi->vms.regs.es = POINTER_SEGMENT(vmib_base);
    vi->vms.regs.edi = POINTER_OFFSET(vmib_base);
    code = VbeDoInterrupt10(vi);
    
    if(code >= 0)
	*vmib = *vmib_low;
    Vm86ReleaseMemory (vi, mark);
    return code;
}

static int 
VbeReportVib(Vm86InfoPtr vi, VbeInfoBlock *vib)
{
    U32 i, p;
    unsigned char c;
    int error = 0;
    
    ErrorF("VBE version %c.%c (", 
           ((vib->VbeVersion >> 8) & 0xFF) + '0',
           (vib->VbeVersion & 0xFF)+'0');
    p = vib->OemStringPtr;
    for(i = 0; 1; i++) {
        c = Vm86Memory(vi, MAKE_POINTER_1(p+i));
        if(!c) break;
	if (c >= ' ')
	    ErrorF("%c", c);
        if (i > 32000) {
            error = 1;
            break;
        }
    }
    ErrorF(")\n");
    ErrorF("DAC is %s, controller is %sVGA compatible%s\n",
           (vib->Capabilities[0]&1)?"switchable":"fixed",
           (vib->Capabilities[0]&2)?"not ":"",
           (vib->Capabilities[0]&3)?", RAMDAC causes snow":"");
    ErrorF("Total memory: %lu kilobytes\n", 64L*vib->TotalMemory);
    if(error)
        return -1;
    return 0;
}

#if 0
static int 
VbeReportModeInfo(Vm86InfoPtr vi, U16 mode, VbeModeInfoBlock *vmib)
{
    int supported = (vmib->ModeAttributes&0x1)?1:0;
    int colour = (vmib->ModeAttributes&0x8)?1:0;
    int graphics = (vmib->ModeAttributes&0x10)?1:0;
    int vga_compatible = !((vmib->ModeAttributes&0x20)?1:0);
    int linear_fb = (vmib->ModeAttributes&0x80)?1:0;

    ErrorF("0x%04X: %dx%dx%d%s",
           (unsigned)mode, 
           (int)vmib->XResolution, (int)vmib->YResolution,
           (int)vmib->BitsPerPixel,
           colour?"":" (monochrome)",
	   graphics?"":" (graphics)",
	   vga_compatible?"":" (vga compatible)",
	   linear_fb?"":" (linear frame buffer)");
    switch(vmib->MemoryModel) {
    case 0:
        ErrorF(" text mode (%dx%d)",
               (int)vmib->XCharSize, (int)vmib->YCharSize);
        break;
    case 1:
        ErrorF(" CGA graphics");
        break;
    case 2:
        ErrorF(" Hercules graphics");
        break;
    case 3:
        ErrorF(" Planar (%d planes)", vmib->NumberOfPlanes);
        break;
    case 4:
        ErrorF(" PseudoColor");
        break;
    case 5:
        ErrorF(" Non-chain 4, 256 colour");
        break;
    case 6:
        if(vmib->DirectColorModeInfo & 1)
            ErrorF(" DirectColor");
        else
            ErrorF(" TrueColor");
        ErrorF(" [%d:%d:%d:%d]",
               vmib->RedMaskSize, vmib->GreenMaskSize, vmib->BlueMaskSize,
               vmib->RsvdMaskSize);
        if(vmib->DirectColorModeInfo & 2)
            ErrorF(" (reserved bits are reserved)");
        break;
    case 7: ErrorF("YUV");
        break;
    default:
        ErrorF("unknown MemoryModel 0x%X ", vmib->MemoryModel);
    }
    if(!supported)
        ErrorF(" (unsupported)");
    else if(!linear_fb)
        ErrorF(" (no linear framebuffer)");
    ErrorF("\n");
    return 0;
}
#endif

void
VbeReportInfo (Vm86InfoPtr vi)
{
    VbeInfoBlock    vib;
    int		    code;

    code = VbeGetVib (vi, &vib);
    if (code >= 0)
	VbeReportVib(vi, &vib);
}

int
VbeGetNmode (Vm86InfoPtr vi)
{
    VbeInfoBlock    vib;
    int		    code;
    unsigned int    p;
    int		    n;
    int		    mode;

    code = VbeGetVib (vi, &vib);
    if (code >= 0)
    {
	p = MAKE_POINTER_1(vib.VideoModePtr);
	for (n = 0; ; n++)
	{
	    mode = Vm86MemoryW(vi, p);
	    if (mode == 0xffff)
	        break;
	    p += 2;
	}
	code = n;
    }
    return code;
}

int
VbeGetModes (Vm86InfoPtr vi, VesaModePtr modes, int nmode)
{
    VbeInfoBlock	vib;
    int			code;
    unsigned int	p;
    int			n;
    int			mode;
    VbeModeInfoBlock	vmib;

    code = VbeGetVib (vi, &vib);
    if (code < 0)
	return code;

    memset (modes, '\0', nmode * sizeof (VesaModeRec));
    
    p = MAKE_POINTER_1(vib.VideoModePtr);
    for (n = 0; n < nmode; n++)
    {
	mode = Vm86MemoryW(vi, p);
	if (mode == 0xffff)
	    break;
	modes[n].mode = mode;
	modes[n].vbe = 1;
	p += 2;
    }
    
    nmode = n;
    
    for (n = 0; n < nmode; n++)
    {
	code = VbeGetVmib (vi, modes[n].mode, &vmib);
	if (code >= 0)
	{
	    modes[n].ModeAttributes = vmib.ModeAttributes;
	    modes[n].NumberOfPlanes = vmib.NumberOfPlanes;
	    modes[n].BitsPerPixel = vmib.BitsPerPixel;
	    modes[n].MemoryModel = vmib.MemoryModel;
	    modes[n].RedMaskSize = vmib.RedMaskSize;
	    modes[n].RedFieldPosition = vmib.RedFieldPosition;
	    modes[n].GreenMaskSize = vmib.GreenMaskSize;
	    modes[n].GreenFieldPosition = vmib.GreenFieldPosition;
	    modes[n].BlueMaskSize = vmib.BlueMaskSize;
	    modes[n].BlueFieldPosition = vmib.BlueFieldPosition;
	    modes[n].RsvdMaskSize = vmib.RsvdMaskSize;
	    modes[n].RsvdFieldPosition = vmib.RsvdFieldPosition;
	    modes[n].DirectColorModeInfo = vmib.DirectColorModeInfo;
	    modes[n].XResolution = vmib.XResolution;
	    modes[n].YResolution = vmib.YResolution;
	    modes[n].BytesPerScanLine = vmib.BytesPerScanLine;
	}
    }
    
    return nmode;
}

VbeInfoPtr
VbeInit (Vm86InfoPtr vi)
{
    VbeInfoPtr	    vbe;
    int		    code;
    VbeInfoBlock    vib;

    code = VbeGetVib (vi, &vib);
    if (code < 0)
	return 0;
    
    vbe = xalloc (sizeof (VbeInfoRec));
    if (!vbe)
	return 0;
    vbe->palette_format = 6;
    vbe->palette_wait = TRUE;
    return vbe;
}

void
VbeCleanup (Vm86InfoPtr vi, VbeInfoPtr vbe)
{
    xfree (vbe);
}

int
VbeSetMode (Vm86InfoPtr vi, VbeInfoPtr vbe, int mode, int linear, int direct)
{
    int			code;
    VbeInfoBlock	vib;
    int			palette_wait = 0, palette_hi = 0;

    code = VbeGetVib (vi, &vib);
    if (code < 0)
	return -1;
    
    code = VbeGetVmib (vi, mode, &vbe->vmib);
    if (code < 0)
	return -1;
    
    mode = (mode & 0xffff) &~ 0x8000;
    if (linear)
	mode |= 0x4000;
    
    vi->vms.regs.eax = 0x4F02;
    vi->vms.regs.ebx = mode;
    code = VbeDoInterrupt10(vi);
    if(code < 0)
        return -1;
    
    vbe->windowA_offset = vbe->windowB_offset = -1;
    vbe->last_window = 1;
    
    if (!direct)
    {
	if(vib.Capabilities[0] & 1)
	    palette_hi = 1;
	if(vib.Capabilities[0] & 4)
	    palette_wait = 1;
	
	if(palette_hi || palette_wait)
	    VbeSetPaletteOptions(vi, vbe, palette_hi?8:6, palette_wait);
    }

    return 0;
}

int 
VbeGetMode(Vm86InfoPtr vi, int *mode)
{
    int code;
    
    vi->vms.regs.eax = 0x4F03;
    code = VbeDoInterrupt10(vi);
    if(code < 0)
        return - 1;
    *mode = vi->vms.regs.ebx & 0xFFFF;
    return 0;
}

void *
VbeMapFramebuffer(Vm86InfoPtr vi, VbeInfoPtr vbe, int mode, int *ret_size, CARD32 *ret_phys)
{
    U8			*fb;
    VbeInfoBlock	vib;
    VbeModeInfoBlock	vmib;
    int			size;
    int			pagesize = getpagesize();
    int			before, after;

    if (VbeGetVib (vi, &vib) < 0)
	return 0;
    
    if (VbeGetVmib (vi, mode, &vmib) < 0)
	return 0;
    
    size = 1024 * 64L * vib.TotalMemory;
    
    *ret_size = size;
    *ret_phys = vmib.PhysBasePtr;

    before = vmib.PhysBasePtr % pagesize;
    after = pagesize - ((vmib.PhysBasePtr + size) % pagesize);
    if(after == pagesize)
        after = 0;

    fb = KdMapDevice (vmib.PhysBasePtr - before, before + size + after);
    
    if(fb == 0)
    {
        ErrorF("Failed to map framebuffer\n");
        return NULL;
    }

    KdSetMappedMode (vmib.PhysBasePtr - before, before + size + after,
		     KD_MAPPED_MODE_FRAMEBUFFER);

    return fb + before;
}

void
VbeUnmapFramebuffer(Vm86InfoPtr vi, VbeInfoPtr vbe, int mode, void *fb)
{
    VbeInfoBlock	vib;
    VbeModeInfoBlock	vmib;
    int			size;
    int			pagesize = getpagesize();
    int			before, after;

    if (VbeGetVib (vi, &vib) < 0)
	return;
    
    if (VbeGetVmib (vi, mode, &vmib) < 0)
	return;
    
    size = 1024 * 64L * vib.TotalMemory;

    before = vmib.PhysBasePtr % pagesize;
    after = pagesize - ((vmib.PhysBasePtr + size) % pagesize);
    if(after == pagesize)
        after = 0;

    fb = (void *) ((char *) fb - before);
    
    KdUnmapDevice (fb, before + size + after);
    KdResetMappedMode (vmib.PhysBasePtr - before, before + size + after,
		     KD_MAPPED_MODE_FRAMEBUFFER);
}

int 
VbeSetPalette(Vm86InfoPtr vi, VbeInfoPtr vbe, int first, int number, U8 *entries)
{
    U8	    *palette_scratch;
    int	    mark;
    int	    palette_base;
    int	    i, code;

    if(number == 0)
        return 0;

    if(first < 0 || number < 0 || first + number > 256) {
        ErrorF("Cannot set %d, %d palette entries\n", first, number);
        return -1;
    }
    if(vbe->palette_format < 6 || vbe->palette_format > 8) {
        ErrorF("Impossible palette format %d\n", vbe->palette_format);
        return -1;
    }

    mark = Vm86MarkMemory (vi);
    palette_base = Vm86AllocateMemory (vi, 4 * 256);

    palette_scratch = &LM(vi, palette_base);

    for(i=0; i<number*4; i++)
	palette_scratch[i] = entries[i] >> (8 - vbe->palette_format);

    vi->vms.regs.eax = 0x4F09;
    if(vbe->palette_wait)
	vi->vms.regs.ebx = 0x80;
    else
	vi->vms.regs.ebx = 0x00;
    vi->vms.regs.ecx = number;
    vi->vms.regs.edx = first;
    vi->vms.regs.es = POINTER_SEGMENT(palette_base);
    vi->vms.regs.edi = POINTER_OFFSET(palette_base);
    code = VbeDoInterrupt10(vi);
    Vm86ReleaseMemory (vi, mark);
    
    if(code < 0)
	return -1;
    return 0;
}
        
int 
VbeGetPalette(Vm86InfoPtr vi, VbeInfoPtr vbe, int first, int number, U8 *entries)
{
    U8	    *palette_scratch;
    int	    mark;
    int	    palette_base;
    int	    i, code;

    if(number == 0)
        return 0;

    if(first < 0 || number < 0 || first + number > 256) {
        ErrorF("Cannot get %d, %d palette entries\n", first, number);
        return -1;
    }
    if(vbe->palette_format < 6 || vbe->palette_format > 8) {
        ErrorF("Impossible palette format %d\n", vbe->palette_format);
        return -1;
    }

    mark = Vm86MarkMemory (vi);
    palette_base = Vm86AllocateMemory (vi, 4 * 256);
    
    palette_scratch = &LM(vi, palette_base);

    vi->vms.regs.eax = 0x4F09;
    vi->vms.regs.ebx = 0x01;
    vi->vms.regs.ecx = number;
    vi->vms.regs.edx = first;
    vi->vms.regs.es = POINTER_SEGMENT(palette_base);
    vi->vms.regs.edi = POINTER_OFFSET(palette_base);
    code = VbeDoInterrupt10(vi);
    if(code >= 0)
    {
	for(i=0; i<number*4; i++)
	    entries[i] = palette_scratch[i] << (8-vbe->palette_format);
    }
    Vm86ReleaseMemory (vi, mark);

    return 0;
}   
        
int 
VbeSetPaletteOptions(Vm86InfoPtr vi, VbeInfoPtr vbe, U8 bits, int wait)
{
    int code;
    
    if(bits < 6 || bits > 8) {
        ErrorF("Impossible palette format %d\n", bits);
        return -1;
    }
    if(bits != vbe->palette_format) 
    {
        vbe->palette_format = 0;
        vi->vms.regs.eax = 0x4F08;
        vi->vms.regs.ebx = bits << 8;
        code = VbeDoInterrupt10(vi);
        if(code < 0)
            return -1;
        vbe->palette_format = bits;
    }
    vbe->palette_wait = wait;
    return 0;
}

static int
VbeReallySetWindow(Vm86InfoPtr vi, U8 window, U16 winnum)
{
    int code;
    vi->vms.regs.eax = 0x4F05;
    vi->vms.regs.ebx = window;
    vi->vms.regs.edx = winnum;
    code = VbeDoInterrupt10(vi);
    if(code < 0)
        return -1;
    return 0;
}

void *
VbeSetWindow(Vm86InfoPtr vi, VbeInfoPtr vbe, int offset, int purpose, int *size_return)
{
    int window_size = vbe->vmib.WinSize * 1024;
    int code;
    int winnum;

    if(vbe->windowA_offset >= 0)
	if(vbe->windowA_offset <= offset && vbe->windowA_offset + window_size > offset)
	    if(vbe->vmib.WinAAttributes & purpose)
		goto windowA;

    if(vbe->windowB_offset >= 0)
	if(vbe->windowB_offset <= offset && vbe->windowB_offset + window_size > offset)
	    if(vbe->vmib.WinBAttributes & purpose)
		goto windowB;

    if(!(vbe->vmib.WinBAttributes & purpose) || 
       !(vbe->vmib.WinBAttributes & VBE_WINDOW_RELOCATE))
	goto set_windowA;

    if(!(vbe->vmib.WinAAttributes & purpose) || 
       !(vbe->vmib.WinAAttributes & VBE_WINDOW_RELOCATE))
	goto set_windowB;

    if(vbe->last_window)
	goto set_windowA;
    else
	goto set_windowB;

set_windowA:
    winnum = offset / (vbe->vmib.WinGranularity * 1024);
    code = VbeReallySetWindow(vi, 0, winnum);
    if(code < 0) {
	ErrorF("Couldn't set window A to %d*%d\n", 
	       (int)winnum, (int)vbe->vmib.WinGranularity);
	return NULL;
    }
    vbe->windowA_offset = winnum * vbe->vmib.WinGranularity * 1024;
windowA:
    vbe->last_window = 0;
    *size_return = vbe->vmib.WinSize * 1024 - (offset - vbe->windowA_offset);
    return ((U8*)&(LM(vi, MAKE_POINTER(vbe->vmib.WinASegment, 0)))) + offset - vbe->windowA_offset;

set_windowB:
    winnum = offset / (vbe->vmib.WinGranularity * 1024);
    code = VbeReallySetWindow(vi, 1, winnum);
    if(code < 0) {
	ErrorF("Couldn't set window B to %d*%d\n", 
	       (int)winnum, (int)vbe->vmib.WinGranularity);
	return NULL;
    }
    vbe->windowB_offset = winnum * vbe->vmib.WinGranularity * 1024;
windowB:
    vbe->last_window = 1;
    *size_return = vbe->vmib.WinSize * 1024 - (offset - vbe->windowB_offset);
    return ((U8*)&(LM(vi, MAKE_POINTER(vbe->vmib.WinBSegment, 0)))) + offset - vbe->windowB_offset;
}

static const int VbeDPMSModes[4] = {
    0x00,	    /* KD_DPMS_NORMAL */
    0x01,	    /* KD_DPMS_STANDBY */
    0x02,	    /* KD_DPMS_SUSPEND */
    0x04,	    /* KD_DPMS_POWERDOWN */
};

Bool
VbeDPMS(Vm86InfoPtr vi, int mode)
{
    int		    code;

    /*
     * Check which modes are supported
     */
    vi->vms.regs.eax = 0x4f10;
    vi->vms.regs.ebx = 0x0000;
    vi->vms.regs.es = 0;
    vi->vms.regs.edi = 0;
    code = VbeDoInterrupt10 (vi);
    if (code < 0)
    {
	ErrorF ("No DPMS Support %d\n", code);
	return FALSE;
    }
    /* Skip this stage if it's not supported */
    if (((vi->vms.regs.ebx >> 4) & VbeDPMSModes[mode]) != VbeDPMSModes[mode])
	return FALSE;
    
    /* Select this mode */
    vi->vms.regs.eax = 0x4f10;
    vi->vms.regs.ebx = (VbeDPMSModes[mode] << 8) | 0x01;
    code = VbeDoInterrupt10 (vi);
    if (code < 0)
    {
	ErrorF ("DPMS failed %d\n", code);
	return FALSE;
    }
    
    return TRUE;
}

Bool
VbeBoot(Vm86InfoPtr vi)
{
    int	code;
    int bus = 1;
    int device = 0;
    int function = 0;

    vi->vms.regs.eax = (bus << 8) | (device << 3) | (function & 0x7);
    code = VbeDoInterruptE6 (vi);
    ErrorF ("Boot: %d\n", code);
    return TRUE;
}

int
VbeDoInterrupt10(Vm86InfoPtr vi)
{
    int code;
    int oldax;

    oldax = vi->vms.regs.eax & 0xFFFF;

    code = Vm86DoInterrupt(vi, 0x10);
    if(code < 0)
	return -1;

    if((vi->vms.regs.eax & 0xFFFF) != 0x4F && (oldax & 0xFF00) == 0x4F00) {
	ErrorF("Int 10h (0x%04X) failed: 0x%04X",
	       oldax, vi->vms.regs.eax & 0xFFFF);
	if((oldax & 0xFF00) == 0x4F00) {
	    switch((vi->vms.regs.eax & 0xFF00)>>8) {
	    case 0: 
		ErrorF(" (success)\n"); 
		return 0;
	    case 1: 
		ErrorF(" (function call failed)\n"); 
		break;
	    case 2: 
		ErrorF(" (function not supported on this hardware)\n"); 
		break;
	    case 3: 
		ErrorF(" (function call invalid in this video mode)\n"); 
		break;
	    default: 
		ErrorF(" (unknown error)\n"); 
		break;
	    }	    return -1;
	} else {
	    ErrorF("\n");
	}
    }
    return code;
}

int
VbeDoInterruptE6(Vm86InfoPtr vi)
{
    int code;
    int oldax;

    oldax = vi->vms.regs.eax & 0xffff;
    
    code = Vm86DoPOST (vi);
    ErrorF("POST (0x%04X): 0x%04X\n",
	   oldax, vi->vms.regs.eax & 0xffff);
    return code;
}
