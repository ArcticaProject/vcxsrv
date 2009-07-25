/*
 * Copyright (c) 1999-2003 by The XFree86 Project, Inc.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

/* Standard resource information code */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Privstr.h"
#include "xf86Pci.h"
#define NEED_OS_RAC_PROTOS
#include "xf86_OSlib.h"
#include "xf86Resources.h"

/* Avoid Imakefile changes */
#include "bus/Pci.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
	defined(__DragonFly__) || defined(__sun)
#define xf86StdAccResFromOS xf86AccResFromOS
#endif

resPtr
xf86StdAccResFromOS(resPtr ret)
{
    resRange range;

    /*
     * Fallback is to claim the following areas:
     *
     * 0x00000000 - 0x0009ffff	low 640k host memory
     * 0x000c0000 - 0x000effff  location of VGA and other extensions ROMS
     * 0x000f0000 - 0x000fffff	system BIOS
     * 0x00100000 - 0x3fffffff	low 1G - 1MB host memory
     * 0xfec00000 - 0xfecfffff	default I/O APIC config space
     * 0xfee00000 - 0xfeefffff	default Local APIC config space
     * 0xffe00000 - 0xffffffff	high BIOS area (should this be included?)
     *
     * reference: Intel 440BX AGP specs
     *
     * The two APIC spaces appear to be BX-specific and should be dealt with
     * elsewhere.
     */

    /* Fallback is to claim 0x0 - 0x9ffff and 0x100000 - 0x7fffffff */
    RANGE(range, 0x00000000, 0x0009ffff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range, 0x000c0000, 0x000effff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range, 0x000f0000, 0x000fffff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
#if 0
    RANGE(range, 0xfec00000, 0xfecfffff, ResExcMemBlock | ResBios);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range, 0xfee00000, 0xfeefffff, ResExcMemBlock | ResBios);
    ret = xf86AddResToList(ret, &range, -1);
    /* airlied - remove BIOS range it shouldn't be here 
       this should use E820 - or THE OS */
    RANGE(range, 0xffe00000, 0xffffffff, ResExcMemBlock | ResBios);
    ret = xf86AddResToList(ret, &range, -1);
#endif
    /*
     * Fallback would be to claim well known ports in the 0x0 - 0x3ff range
     * along with their sparse I/O aliases, but that's too imprecise.  Instead
     * claim a bare minimum here.
     */
    RANGE(range, 0x00000000, 0x000000ff, ResExcIoBlock); /* For mainboard */
    ret = xf86AddResToList(ret, &range, -1);

    /*
     * At minimum, the top and bottom resources must be claimed, so that
     * resources that are (or appear to be) unallocated can be relocated.
     */
/*  RANGE(range, 0x00000000, 0x00000000, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range, 0xffffffff, 0xffffffff, ResExcMemBlock);
    ret = xf86AddResToList(ret, &range, -1);
    RANGE(range, 0x00000000, 0x00000000, ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1); */
    RANGE(range, 0x0000ffff, 0x0000ffff, ResExcIoBlock);
    ret = xf86AddResToList(ret, &range, -1);

    /* XXX add others */
    return ret;
}
