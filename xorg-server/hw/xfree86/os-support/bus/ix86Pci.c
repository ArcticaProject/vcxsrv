/*
 * ix86Pci.c - x86 PCI driver
 *
 * The XFree86 server PCI access functions have been reimplemented as a
 * framework that allows each supported platform/OS to have their own
 * platform/OS specific PCI driver.
 *
 * Most of the code of these functions was simply lifted from the
 * Intel architecture specifric portion of the original Xfree86
 * PCI code in hw/xfree86/common_hw/xf86_PCI.C...
 *
 * Gary Barton
 * Concurrent Computer Corporation
 * garyb@gate.net
 */

/*
 * Copyright 1998 by Concurrent Computer Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Concurrent Computer
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Concurrent Computer Corporation makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * CONCURRENT COMPUTER CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CONCURRENT COMPUTER CORPORATION BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Copyright 1998 by Metro Link Incorporated
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Metro Link
 * Incorporated not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Metro Link Incorporated makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * METRO LINK INCORPORATED DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL METRO LINK INCORPORATED BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * This software is derived from the original XFree86 PCI code
 * which includes the following copyright notices as well:
 *
 * Copyright 1995 by Robin Cutshaw <robin@XFree86.Org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the above listed copyright holder(s)
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.  The above listed
 * copyright holder(s) make(s) no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM(S) ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * This code is also based heavily on the code in FreeBSD-current, which was
 * written by Wolfgang Stanglmeier, and contains the following copyright:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdio.h>
#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "Pci.h"

#ifdef PC98
#define outb(port,data) _outb(port,data)
#define outl(port,data) _outl(port,data)
#define inb(port) _inb(port)
#define inl(port) _inl(port)
#endif

#define	PCI_CFGMECH2_ENABLE_REG		0xCF8
#ifdef PC98
#define	PCI_CFGMECH2_FORWARD_REG	0xCF9
#else
#define	PCI_CFGMECH2_FORWARD_REG	0xCFA
#endif

#define PCI_CFGMECH2_MAXDEV	16

#define PCI_ADDR_FROM_TAG_CFG1(tag,reg) (PCI_EN | tag | (reg & 0xfc))
#define PCI_FORWARD_FROM_TAG(tag)       PCI_BUS_FROM_TAG(tag)
#define PCI_ENABLE_FROM_TAG(tag)        (0xf0 | (((tag) & 0x00000700) >> 7))
#define PCI_ADDR_FROM_TAG_CFG2(tag,reg) (0xc000 | (((tag) & 0x0000f800) >> 3) \
                                        | (reg & 0xfc))

/*
 * Intel x86 platform specific PCI access functions
 */
#if 0
static CARD32 ix86PciReadLongSetup(PCITAG tag, int off);
static void ix86PciWriteLongSetup(PCITAG, int off, CARD32 val);
static void ix86PciSetBitsLongSetup(PCITAG, int off, CARD32 mask, CARD32 val);
static CARD32 ix86PciReadLongCFG1(PCITAG tag, int off);
static void ix86PciWriteLongCFG1(PCITAG, int off, CARD32 val);
static void ix86PciSetBitsLongCFG1(PCITAG, int off, CARD32 mask, CARD32 val);
static CARD32 ix86PciReadLongCFG2(PCITAG tag, int off);
static void ix86PciWriteLongCFG2(PCITAG, int off, CARD32 val);
static void ix86PciSetBitsLongCFG2(PCITAG, int off, CARD32 mask, CARD32 val);
#endif

static pciBusFuncs_t ix86Funcs0 = {
#if 0
/* pciReadLong      */	ix86PciReadLongSetup,
/* pciWriteLong     */	ix86PciWriteLongSetup,
/* pciSetBitsLong   */	ix86PciSetBitsLongSetup,
/* pciAddrHostToBus */	pciAddrNOOP,
#endif
/* pciAddrBusToHost */	pciAddrNOOP
};

static pciBusFuncs_t ix86Funcs1 = {
#if 0
/* pciReadLong      */	ix86PciReadLongCFG1,
/* pciWriteLong     */	ix86PciWriteLongCFG1,
/* pciSetBitsLong   */	ix86PciSetBitsLongCFG1,
/* pciAddrHostToBus */	pciAddrNOOP,
#endif
/* pciAddrBusToHost */	pciAddrNOOP
};

static pciBusFuncs_t ix86Funcs2 = {
#if 0
/* pciReadLong      */	ix86PciReadLongCFG2,
/* pciWriteLong     */	ix86PciWriteLongCFG2,
/* pciSetBitsLong   */	ix86PciSetBitsLongCFG2,
/* pciAddrHostToBus */	pciAddrNOOP,
#endif
/* pciAddrBusToHost */	pciAddrNOOP
};

static pciBusInfo_t ix86Pci0 = {
/* configMech  */	PCI_CFG_MECH_UNKNOWN,	/* Set by ix86PciInit() */
/* numDevices  */	0,			/* Set by ix86PciInit() */
/* secondary   */	FALSE,
/* primary_bus */	0,
/* funcs       */	&ix86Funcs0,		/* Set by ix86PciInit() */
/* pciBusPriv  */	NULL,
/* bridge      */	NULL
};

_X_EXPORT pointer
xf86MapDomainMemory(int ScreenNum, int Flags, struct pci_device *dev,
                    ADDRESS Base, unsigned long Size)
{
    return xf86MapVidMem(ScreenNum, Flags, Base, Size);
}

IOADDRESS
xf86MapLegacyIO(struct pci_device *dev)
{
    (void)dev;
    return 0;
}

static Bool
ix86PciBusCheck(void)
{
#if 0
    PCITAG tag;
    CARD32 id, class;
    CARD8 device;

    for (device = 0; device < ix86Pci0.numDevices; device++) {
	tag = PCI_MAKE_TAG(0, device, 0);
	id = (*ix86Pci0.funcs->pciReadLong)(tag, PCI_ID_REG);

	if ((CARD16)(id + 1U) <= (CARD16)1UL)
		continue;

	/* The rest of this is inspired by the Linux kernel */
	class = (*ix86Pci0.funcs->pciReadLong)(tag, PCI_CLASS_REG);

	/* Ignore revision id and programming interface */
	switch (class >> 16) {
	case (PCI_CLASS_PREHISTORIC << 8) | PCI_SUBCLASS_PREHISTORIC_MISC:
	    /* Check for vendors of known buggy chipsets */
	    id &= 0x0000ffff;
	    if ((id == PCI_VENDOR_INTEL) || (id == PCI_VENDOR_COMPAQ))
		return TRUE;
	    continue;

	case (PCI_CLASS_PREHISTORIC << 8) | PCI_SUBCLASS_PREHISTORIC_VGA:
	case (PCI_CLASS_DISPLAY << 8) | PCI_SUBCLASS_DISPLAY_VGA:
	case (PCI_CLASS_BRIDGE << 8) | PCI_SUBCLASS_BRIDGE_HOST:
	    return TRUE;

	default:
	    break;
	}
    }
#endif
    return FALSE;
}

static
void ix86PciSelectCfgmech(void)
{
    static Bool beenhere = FALSE;
    CARD32 mode1Res1 = 0, mode1Res2 = 0, oldVal1 = 0;
    CARD8  mode2Res1 = 0, mode2Res2 = 0, oldVal2 = 0;
    int stages = 0;

    if (beenhere)
	return; /* Been there, done that */

    beenhere = TRUE;

    /*
     * Determine if motherboard chipset supports PCI Config Mech 1 or 2
     * We rely on xf86Info.pciFlags to tell which mechanisms to try....
     */
    switch (xf86Info.pciFlags) {
	case PCIOsConfig:
	case PCIProbe1:
	    if (!xf86EnableIO())
		return;

	    xf86MsgVerb(X_INFO, 2,
			"PCI: Probing config type using method 1\n");
	    oldVal1 = inl(PCI_CFGMECH1_ADDRESS_REG);

#ifdef DEBUGPCI
      if (xf86Verbose > 2) {
	ErrorF("Checking config type 1:\n"
		"\tinitial value of MODE1_ADDR_REG is 0x%08x\n", oldVal1);
	ErrorF("\tChecking that all bits in mask 0x7f000000 are clear\n");
      }
#endif

      /* Assuming config type 1 to start with */
      if ((oldVal1 & 0x7f000000) == 0) {

	stages |= 0x01;

#ifdef DEBUGPCI
	if (xf86Verbose > 2) {
	    ErrorF("\tValue indicates possibly config type 1\n");
	    ErrorF("\tWriting 32-bit value 0x%08x to MODE1_ADDR_REG\n", PCI_EN);
#if 0
	    ErrorF("\tWriting 8-bit value 0x00 to MODE1_ADDR_REG + 3\n");
#endif
	}
#endif

	ix86Pci0.configMech = PCI_CFG_MECH_1;
	ix86Pci0.numDevices = PCI_CFGMECH1_MAXDEV;
	ix86Pci0.funcs = &ix86Funcs1;

	outl(PCI_CFGMECH1_ADDRESS_REG, PCI_EN);

#if 0
	/*
	 * This seems to cause some Neptune-based PCI machines to switch
	 * from config type 1 to config type 2
	 */
	outb(PCI_CFGMECH1_ADDRESS_REG + 3, 0);
#endif
	mode1Res1 = inl(PCI_CFGMECH1_ADDRESS_REG);

#ifdef DEBUGPCI
	if (xf86Verbose > 2) {
	    ErrorF("\tValue read back from MODE1_ADDR_REG is 0x%08x\n",
			mode1Res1);
	    ErrorF("\tRestoring original contents of MODE1_ADDR_REG\n");
	}
#endif

	outl(PCI_CFGMECH1_ADDRESS_REG, oldVal1);

	if (mode1Res1) {

	    stages |= 0x02;

#ifdef DEBUGPCI
	    if (xf86Verbose > 2) {
		ErrorF("\tValue read back is non-zero, and indicates possible"
			" config type 1\n");
	    }
#endif

	    if (ix86PciBusCheck()) {

#ifdef DEBUGPCI
		if (xf86Verbose > 2)
		    ErrorF("\tBus check Confirms this: ");
#endif

		xf86MsgVerb(X_INFO, 2, "PCI: Config type is 1\n");
		xf86MsgVerb(X_INFO, 3,
			"PCI: stages = 0x%02x, oldVal1 = 0x%08lx, mode1Res1"
			" = 0x%08lx\n", stages, (unsigned long)oldVal1,
			(unsigned long)mode1Res1);
		return;
	    }

#ifdef DEBUGPCI
	    if (xf86Verbose > 2) {
		ErrorF("\tBus check fails to confirm this, continuing type 1"
			" check ...\n");
	    }
#endif

	}

	stages |= 0x04;

#ifdef DEBUGPCI
	if (xf86Verbose > 2) {
	    ErrorF("\tWriting 0xff000001 to MODE1_ADDR_REG\n");
	}
#endif
	outl(PCI_CFGMECH1_ADDRESS_REG, 0xff000001);
	mode1Res2 = inl(PCI_CFGMECH1_ADDRESS_REG);

#ifdef DEBUGPCI
	if (xf86Verbose > 2) {
	    ErrorF("\tValue read back from MODE1_ADDR_REG is 0x%08x\n",
			mode1Res2);
	    ErrorF("\tRestoring original contents of MODE1_ADDR_REG\n");
	}
#endif

	outl(PCI_CFGMECH1_ADDRESS_REG, oldVal1);

	if ((mode1Res2 & 0x80000001) == 0x80000000) {

	    stages |= 0x08;

#ifdef DEBUGPCI
	    if (xf86Verbose > 2) {
		ErrorF("\tValue read back has only the msb set\n"
			"\tThis indicates possible config type 1\n");
	    }
#endif

	    if (ix86PciBusCheck()) {

#ifdef DEBUGPCI
		if (xf86Verbose > 2)
		    ErrorF("\tBus check Confirms this: ");
#endif

		xf86MsgVerb(X_INFO, 2, "PCI: Config type is 1\n");
		xf86MsgVerb(X_INFO, 3,
			"PCI: stages = 0x%02x, oldVal1 = 0x%08lx,\n"
			"\tmode1Res1 = 0x%08lx, mode1Res2 = 0x%08lx\n",
			stages, (unsigned long)oldVal1,
			(unsigned long)mode1Res1, (unsigned long)mode1Res2);
		return;
	    }

#ifdef DEBUGPCI
	    if (xf86Verbose > 2) {
		ErrorF("\tBus check fails to confirm this.\n");
	    }
#endif

	}
      }

      xf86MsgVerb(X_INFO, 3, "PCI: Standard check for type 1 failed.\n");
      xf86MsgVerb(X_INFO, 3, "PCI: stages = 0x%02x, oldVal1 = 0x%08lx,\n"
	       "\tmode1Res1 = 0x%08lx, mode1Res2 = 0x%08lx\n",
	       stages, (unsigned long)oldVal1, (unsigned long)mode1Res1,
	       (unsigned long)mode1Res2);

      /* Try config type 2 */
      oldVal2 = inb(PCI_CFGMECH2_ENABLE_REG);
      if ((oldVal2 & 0xf0) == 0) {
	ix86Pci0.configMech = PCI_CFG_MECH_2;
	ix86Pci0.numDevices = PCI_CFGMECH2_MAXDEV;
	ix86Pci0.funcs = &ix86Funcs2;

	outb(PCI_CFGMECH2_ENABLE_REG, 0x0e);
	mode2Res1 = inb(PCI_CFGMECH2_ENABLE_REG);
	outb(PCI_CFGMECH2_ENABLE_REG, oldVal2);

	if (mode2Res1 == 0x0e) {
	    if (ix86PciBusCheck()) {
		xf86MsgVerb(X_INFO, 2, "PCI: Config type is 2\n");
		return;
	    }
	}
      }
      break; /* } */

    case PCIProbe2: /* { */
	if (!xf86EnableIO())
	    return;

      /* The scanpci-style detection method */

      xf86MsgVerb(X_INFO, 2, "PCI: Probing config type using method 2\n");

      outb(PCI_CFGMECH2_ENABLE_REG, 0x00);
      outb(PCI_CFGMECH2_FORWARD_REG, 0x00);
      mode2Res1 = inb(PCI_CFGMECH2_ENABLE_REG);
      mode2Res2 = inb(PCI_CFGMECH2_FORWARD_REG);

      if (mode2Res1 == 0 && mode2Res2 == 0) {
	xf86MsgVerb(X_INFO, 2, "PCI: Config type is 2\n");
	ix86Pci0.configMech = PCI_CFG_MECH_2;
	ix86Pci0.numDevices = PCI_CFGMECH2_MAXDEV;
	ix86Pci0.funcs = &ix86Funcs2;
	return;
      }

      oldVal1 = inl(PCI_CFGMECH1_ADDRESS_REG);
      outl(PCI_CFGMECH1_ADDRESS_REG, PCI_EN);
      mode1Res1 = inl(PCI_CFGMECH1_ADDRESS_REG);
      outl(PCI_CFGMECH1_ADDRESS_REG, oldVal1);
      if (mode1Res1 == PCI_EN) {
	xf86MsgVerb(X_INFO, 2, "PCI: Config type is 1\n");
	ix86Pci0.configMech = PCI_CFG_MECH_1;
	ix86Pci0.numDevices = PCI_CFGMECH1_MAXDEV;
	ix86Pci0.funcs = &ix86Funcs1;
	return;
      }
      break; /* } */

    case PCIForceConfig1:
	if (!xf86EnableIO())
	    return;

      xf86MsgVerb(X_INFO, 2, "PCI: Forcing config type 1\n");

      ix86Pci0.configMech = PCI_CFG_MECH_1;
      ix86Pci0.numDevices = PCI_CFGMECH1_MAXDEV;
      ix86Pci0.funcs = &ix86Funcs1;
      return;

    case PCIForceConfig2:
	if (!xf86EnableIO())
	    return;

      xf86MsgVerb(X_INFO, 2, "PCI: Forcing config type 2\n");

      ix86Pci0.configMech = PCI_CFG_MECH_2;
      ix86Pci0.numDevices = PCI_CFGMECH2_MAXDEV;
      ix86Pci0.funcs = &ix86Funcs2;
      return;

    case PCIForceNone:
	break;
    }

    /* No PCI found */
    ix86Pci0.configMech = PCI_CFG_MECH_UNKNOWN;
    xf86MsgVerb(X_INFO, 2, "PCI: No PCI bus found or probed for\n");
}

#if 0
static pciTagRec
ix86PcibusTag(CARD8 bus, CARD8 cardnum, CARD8 func)
{
    pciTagRec tag;

    tag.cfg1 = 0;

    if (func > 7 || cardnum >= pciBusInfo[bus]->numDevices)
	return tag;

    switch (ix86Pci0.configMech) {
    case PCI_CFG_MECH_1:
	    tag.cfg1 = PCI_EN | ((CARD32)bus << 16) |
		       ((CARD32)cardnum << 11) |
		       ((CARD32)func << 8);
	    break;

    case PCI_CFG_MECH_2:
	    tag.cfg2.port    = 0xc000 | ((CARD16)cardnum << 8);
	    tag.cfg2.enable  = 0xf0 | (func << 1);
	    tag.cfg2.forward = bus;
	    break;
    }

    return tag;
}
#endif

#if 0
static CARD32
ix86PciReadLongSetup(PCITAG Tag, int reg)
{
    ix86PciSelectCfgmech();
    return (*ix86Pci0.funcs->pciReadLong)(Tag,reg);
}

static CARD32
ix86PciReadLongCFG1(PCITAG Tag, int reg)
{
    CARD32    addr, data = 0;

#ifdef DEBUGPCI
    ErrorF("ix86PciReadLong 0x%lx, %d\n", Tag, reg);
#endif

    addr = PCI_ADDR_FROM_TAG_CFG1(Tag,reg);
    outl(PCI_CFGMECH1_ADDRESS_REG, addr);
    data = inl(PCI_CFGMECH1_DATA_REG);
    outl(PCI_CFGMECH1_ADDRESS_REG, 0);

#ifdef DEBUGPCI
    ErrorF("ix86PciReadLong 0x%lx\n", data);
#endif

    return data;
}

static CARD32
ix86PciReadLongCFG2(PCITAG Tag, int reg)
{
    CARD32    addr, data = 0;
    CARD8     forward, enable;

#ifdef DEBUGPCI
    ErrorF("ix86PciReadLong 0x%lx, %d\n", Tag, reg);
#endif

    forward  = PCI_FORWARD_FROM_TAG(Tag);
    enable   = PCI_ENABLE_FROM_TAG(Tag);
    addr     = PCI_ADDR_FROM_TAG_CFG2(Tag,reg);

    outb(PCI_CFGMECH2_ENABLE_REG, enable);
    outb(PCI_CFGMECH2_FORWARD_REG, forward);
    data = inl((CARD16)addr);
    outb(PCI_CFGMECH2_ENABLE_REG, 0);
    outb(PCI_CFGMECH2_FORWARD_REG, 0);

#ifdef DEBUGPCI
    ErrorF("ix86PciReadLong 0x%lx\n", data);
#endif

    return data;
}

static void
ix86PciWriteLongSetup(PCITAG Tag, int reg, CARD32 data)
{
    ix86PciSelectCfgmech();
    (*ix86Pci0.funcs->pciWriteLong)(Tag,reg,data);
}

static void
ix86PciWriteLongCFG1(PCITAG Tag, int reg, CARD32 data)
{
    CARD32    addr;

    addr = PCI_ADDR_FROM_TAG_CFG1(Tag,reg);
    outl(PCI_CFGMECH1_ADDRESS_REG, addr);
    outl(PCI_CFGMECH1_DATA_REG, data);
    outl(PCI_CFGMECH1_ADDRESS_REG, 0);
}

static void
ix86PciWriteLongCFG2(PCITAG Tag, int reg, CARD32 data)
{
    CARD32    addr;
    CARD8 forward, enable;

    forward  = PCI_FORWARD_FROM_TAG(Tag);
    enable   = PCI_ENABLE_FROM_TAG(Tag);
    addr     = PCI_ADDR_FROM_TAG_CFG2(Tag,reg);

    outb(PCI_CFGMECH2_ENABLE_REG, enable);
    outb(PCI_CFGMECH2_FORWARD_REG, forward);
    outl((CARD16)addr, data);
    outb(PCI_CFGMECH2_ENABLE_REG, 0);
    outb(PCI_CFGMECH2_FORWARD_REG, 0);
}

static void
ix86PciSetBitsLongSetup(PCITAG Tag, int reg, CARD32 mask, CARD32 val)
{
    ix86PciSelectCfgmech();
    (*ix86Pci0.funcs->pciSetBitsLong)(Tag,reg,mask,val);
}

static void
ix86PciSetBitsLongCFG1(PCITAG Tag, int reg, CARD32 mask, CARD32 val)
{
    CARD32    addr, data = 0;

#ifdef DEBUGPCI
    ErrorF("ix86PciSetBitsLong 0x%lx, %d\n", Tag, reg);
#endif

    addr = PCI_ADDR_FROM_TAG_CFG1(Tag,reg);
    outl(PCI_CFGMECH1_ADDRESS_REG, addr);
    data = inl(PCI_CFGMECH1_DATA_REG);
    data = (data & ~mask) | (val & mask);
    outl(PCI_CFGMECH1_DATA_REG, data);
    outl(PCI_CFGMECH1_ADDRESS_REG, 0);
}

static void
ix86PciSetBitsLongCFG2(PCITAG Tag, int reg, CARD32 mask, CARD32 val)
{
    CARD32    addr, data = 0;
    CARD8 enable, forward;

#ifdef DEBUGPCI
    ErrorF("ix86PciSetBitsLong 0x%lx, %d\n", Tag, reg);
#endif

    forward  = PCI_FORWARD_FROM_TAG(Tag);
    enable   = PCI_ENABLE_FROM_TAG(Tag);
    addr     = PCI_ADDR_FROM_TAG_CFG2(Tag,reg);

    outb(PCI_CFGMECH2_ENABLE_REG, enable);
    outb(PCI_CFGMECH2_FORWARD_REG, forward);
    data = inl((CARD16)addr);
    data = (data & ~mask) | (val & mask);
    outl((CARD16)addr, data);
    outb(PCI_CFGMECH2_ENABLE_REG, 0);
    outb(PCI_CFGMECH2_FORWARD_REG, 0);
}
#endif

void
ix86PciInit()
{
    /* Initialize pciBusInfo[] array and function pointers */
    pciNumBuses    = 1;
    pciBusInfo[0]  = &ix86Pci0;

    /* Make sure that there is a PCI bus present. */
    ix86PciSelectCfgmech();
    if (ix86Pci0.configMech == PCI_CFG_MECH_UNKNOWN) {
	pciNumBuses    = 0;
	pciBusInfo[0]  = NULL;
    }
}
