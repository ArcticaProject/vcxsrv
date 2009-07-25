/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
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

/*
 * This file contains the interfaces to the bus-specific code
 */
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/X.h>
#include <pciaccess.h>
#include "os.h"
#include "Pci.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Resources.h"

/* Bus-specific headers */
#include "xf86Bus.h"

#define XF86_OS_PRIVS
#define NEED_OS_RAC_PROTOS
#include "xf86_OSproc.h"

#include "xf86RAC.h"

/* Bus-specific globals */
Bool pciSlotClaimed = FALSE;
static struct pci_device ** xf86PciVideoInfo = NULL;	/* PCI probe for video hw */


/* PCI classes that get included in xf86PciVideoInfo */
#define PCIINFOCLASSES(c) \
    ( (((c) & 0x00ff0000) == (PCI_CLASS_PREHISTORIC << 16)) \
      || (((c) & 0x00ff0000) == (PCI_CLASS_DISPLAY << 16)) \
      || ((((c) & 0x00ffff00) \
	   == ((PCI_CLASS_MULTIMEDIA << 16) | (PCI_SUBCLASS_MULTIMEDIA_VIDEO << 8)))) \
      || ((((c) & 0x00ffff00) \
	   == ((PCI_CLASS_PROCESSOR << 16) | (PCI_SUBCLASS_PROCESSOR_COPROC << 8)))) )

/*
 * PCI classes that have messages printed always.  The others are only
 * have a message printed when the vendor/dev IDs are recognised.
 */
#define PCIALWAYSPRINTCLASSES(c) \
    ( (((c) & 0x00ffff00) \
       == ((PCI_CLASS_PREHISTORIC << 16) | (PCI_SUBCLASS_PREHISTORIC_VGA << 8))) \
      || (((c) & 0x00ff0000) == (PCI_CLASS_DISPLAY << 16)) \
      || ((((c) & 0x00ffff00) \
	   == ((PCI_CLASS_MULTIMEDIA << 16) | (PCI_SUBCLASS_MULTIMEDIA_VIDEO << 8)))) )

#define IS_VGA(c) \
    (((c) & 0x00ffff00) \
	 == ((PCI_CLASS_DISPLAY << 16) | (PCI_SUBCLASS_DISPLAY_VGA << 8)))

/*
 * PCI classes for which potentially destructive checking of the map sizes
 * may be done.  Any classes where this may be unsafe should be omitted
 * from this list.
 */
#define PCINONSYSTEMCLASSES(c) PCIALWAYSPRINTCLASSES(c)

/* 
 * PCI classes that use RAC 
 */
#define PCISHAREDIOCLASSES(c) \
    ( (((c) & 0x00ffff00) \
       == ((PCI_CLASS_PREHISTORIC << 16) | (PCI_SUBCLASS_PREHISTORIC_VGA << 8))) \
      || IS_VGA(c) )


_X_EXPORT void
xf86FormatPciBusNumber(int busnum, char *buffer)
{
    /* 'buffer' should be at least 8 characters long */
    if (busnum < 256)
	sprintf(buffer, "%d", busnum);
    else
	sprintf(buffer, "%d@%d", busnum & 0x00ff, busnum >> 8);
}

/*
 * IO enable/disable related routines for PCI
 */
#define pArg ((pciArg*)arg)
#define SETBITS PCI_CMD_IO_ENABLE
static void
pciIoAccessEnable(void* arg)
{
#if 0
#ifdef DEBUG
    ErrorF("pciIoAccessEnable: 0x%05lx\n", *(PCITAG *)arg);
#endif
    pArg->ctrl |= SETBITS | PCI_CMD_MASTER_ENABLE;
    pci_device_cfg_write_u32(pArg->dev, pArg->ctrl, PCI_CMD_STAT_REG);
#endif
}

static void
pciIoAccessDisable(void* arg)
{
#if 0
#ifdef DEBUG
    ErrorF("pciIoAccessDisable: 0x%05lx\n", *(PCITAG *)arg);
#endif
    pArg->ctrl &= ~SETBITS;
    pci_device_cfg_write_u32(pArg->dev, pArg->ctrl, PCI_CMD_STAT_REG);
#endif
}

#undef SETBITS
#define SETBITS (PCI_CMD_IO_ENABLE | PCI_CMD_MEM_ENABLE)
static void
pciIo_MemAccessEnable(void* arg)
{
#if 0
#ifdef DEBUG
    ErrorF("pciIo_MemAccessEnable: 0x%05lx\n", *(PCITAG *)arg);
#endif
    pArg->ctrl |= SETBITS | PCI_CMD_MASTER_ENABLE;
    pci_device_cfg_write_u32(pArg->dev, pArg->ctrl, PCI_CMD_STAT_REG);
#endif
}

static void
pciIo_MemAccessDisable(void* arg)
{
#if 0
#ifdef DEBUG
    ErrorF("pciIo_MemAccessDisable: 0x%05lx\n", *(PCITAG *)arg);
#endif
    pArg->ctrl &= ~SETBITS;
    pci_device_cfg_write_u32(pArg->dev, pArg->ctrl, PCI_CMD_STAT_REG);
#endif
}

#undef SETBITS
#define SETBITS (PCI_CMD_MEM_ENABLE)
static void
pciMemAccessEnable(void* arg)
{
#if 0
#ifdef DEBUG
    ErrorF("pciMemAccessEnable: 0x%05lx\n", *(PCITAG *)arg);
#endif
    pArg->ctrl |= SETBITS | PCI_CMD_MASTER_ENABLE;
    pci_device_cfg_write_u32(pArg->dev, pArg->ctrl, PCI_CMD_STAT_REG);
#endif
}

static void
pciMemAccessDisable(void* arg)
{
#if 0
#ifdef DEBUG
    ErrorF("pciMemAccessDisable: 0x%05lx\n", *(PCITAG *)arg);
#endif
    pArg->ctrl &= ~SETBITS;
    pci_device_cfg_write_u32(pArg->dev, pArg->ctrl, PCI_CMD_STAT_REG);
#endif
}
#undef SETBITS
#undef pArg


/* move to OS layer */
#define MASKBITS (PCI_PCI_BRIDGE_VGA_EN | PCI_PCI_BRIDGE_MASTER_ABORT_EN)
static void
pciBusAccessEnable(BusAccPtr ptr)
{
#if 0
    struct pci_device * const dev = ptr->busdep.pci.dev;
    uint16_t ctrl;

#ifdef DEBUG
    ErrorF("pciBusAccessEnable: bus=%d\n", ptr->busdep.pci.bus);
#endif
    pci_device_cfg_read_u16( dev, & ctrl, PCI_PCI_BRIDGE_CONTROL_REG );
    if ((ctrl & MASKBITS) != PCI_PCI_BRIDGE_VGA_EN) {
	ctrl = (ctrl | PCI_PCI_BRIDGE_VGA_EN) &
	    ~(PCI_PCI_BRIDGE_MASTER_ABORT_EN | PCI_PCI_BRIDGE_SECONDARY_RESET);
	pci_device_cfg_write_u16(dev, ctrl, PCI_PCI_BRIDGE_CONTROL_REG);
    }
#endif
}

/* move to OS layer */
static void
pciBusAccessDisable(BusAccPtr ptr)
{
#if 0
    struct pci_device * const dev = ptr->busdep.pci.dev;
    uint16_t ctrl;

#ifdef DEBUG
    ErrorF("pciBusAccessDisable: bus=%d\n", ptr->busdep.pci.bus);
#endif
    pci_device_cfg_read_u16( dev, & ctrl, PCI_PCI_BRIDGE_CONTROL_REG );
    if (ctrl & MASKBITS) {
	ctrl &= ~(MASKBITS | PCI_PCI_BRIDGE_SECONDARY_RESET);
	pci_device_cfg_write_u16(dev, ctrl, PCI_PCI_BRIDGE_CONTROL_REG);
    }
#endif
}
#undef MASKBITS

static void
pciSetBusAccess(BusAccPtr ptr)
{
#if 0
#ifdef DEBUG
    ErrorF("pciSetBusAccess: route VGA to bus %d\n", ptr->busdep.pci.bus);
#endif

    if (!ptr->primary && !ptr->current)
	return;
    
    if (ptr->current && ptr->current->disable_f)
	(*ptr->current->disable_f)(ptr->current);
    ptr->current = NULL;
    
    /* walk down */
    while (ptr->primary) {	/* No enable for root bus */
	if (ptr != ptr->primary->current) {
	    if (ptr->primary->current && ptr->primary->current->disable_f)
		(*ptr->primary->current->disable_f)(ptr->primary->current);
	    if (ptr->enable_f)
		(*ptr->enable_f)(ptr);
	    ptr->primary->current = ptr;
	}
	ptr = ptr->primary;
    }
#endif
}

/* move to OS layer */
static void
savePciState( struct pci_device * dev, pciSavePtr ptr )
{
#if 0
    int i;

    pci_device_cfg_read_u32( dev, & ptr->command, PCI_CMD_STAT_REG );

    for ( i = 0; i < 6; i++ ) {
	pci_device_cfg_read_u32( dev, & ptr->base[i], 
				 PCI_CMD_BASE_REG + (i * 4) );
    }

    pci_device_cfg_read_u32( dev, & ptr->biosBase, PCI_CMD_BIOS_REG );
#endif
}

/* move to OS layer */
#if 0
static void
restorePciState( struct pci_device * dev, pciSavePtr ptr)
{
    int i;
    
    /* disable card before setting anything */
    pci_device_cfg_write_bits(dev, PCI_CMD_MEM_ENABLE | PCI_CMD_IO_ENABLE, 0,
			      PCI_CMD_STAT_REG);

    pci_device_cfg_write_u32(dev, ptr->biosBase, PCI_CMD_BIOS_REG);

    for ( i = 0; i < 6; i++ ) {
	pci_device_cfg_write_u32(dev, ptr->base[i],
				 PCI_CMD_BASE_REG + (i * 4));
    }

    pci_device_cfg_write_u32(dev, ptr->command, PCI_CMD_STAT_REG);
}
#endif

/* move to OS layer */
static void
savePciBusState(BusAccPtr ptr)
{
#if 0
    struct pci_device * const dev = ptr->busdep.pci.dev;
    uint16_t temp;

    pci_device_cfg_read_u16( dev, & temp, PCI_PCI_BRIDGE_CONTROL_REG );
    ptr->busdep.pci.save.control = temp & ~PCI_PCI_BRIDGE_SECONDARY_RESET;

    /* Allow master aborts to complete normally on non-root buses */
    if ( ptr->busdep.pci.save.control & PCI_PCI_BRIDGE_MASTER_ABORT_EN ) {
	temp = ptr->busdep.pci.save.control & ~PCI_PCI_BRIDGE_MASTER_ABORT_EN;
	pci_device_cfg_read_u16( dev, & temp, PCI_PCI_BRIDGE_CONTROL_REG );
    }
#endif
}

/* move to OS layer */
#define MASKBITS (PCI_PCI_BRIDGE_VGA_EN | PCI_PCI_BRIDGE_MASTER_ABORT_EN)
static void
restorePciBusState(BusAccPtr ptr)
{
#if 0
    struct pci_device * const dev = ptr->busdep.pci.dev;
    uint16_t ctrl;

    /* Only restore the bits we've changed (and don't cause resets) */
    pci_device_cfg_read_u16( dev, & ctrl, PCI_PCI_BRIDGE_CONTROL_REG );

    if ((ctrl ^ ptr->busdep.pci.save.control) & MASKBITS) {
	ctrl &= ~(MASKBITS | PCI_PCI_BRIDGE_SECONDARY_RESET);
	ctrl |= ptr->busdep.pci.save.control & MASKBITS;
	pci_device_cfg_write_u16(dev, ctrl, PCI_PCI_BRIDGE_CONTROL_REG);
    }
#endif
}
#undef MASKBITS


/*
 * xf86Bus.c interface
 */

void
xf86PciProbe(void)
{
    int i = 0, k;
    int num = 0;
    struct pci_device *info;
    struct pci_device_iterator *iter;


    if (!xf86scanpci()) {
	xf86PciVideoInfo = NULL;
	return;
    }

    iter = pci_slot_match_iterator_create(& xf86IsolateDevice);
    while ((info = pci_device_next(iter)) != NULL) {
	if (PCIINFOCLASSES(info->device_class)) {
	    num++;
	    xf86PciVideoInfo = xnfrealloc(xf86PciVideoInfo,
					  (sizeof(struct pci_device *)
					   * (num + 1)));
	    xf86PciVideoInfo[num] = NULL;
	    xf86PciVideoInfo[num - 1] = info;

	    pci_device_probe(info);
	    info->user_data = 0;
	}
    }


    /* If we haven't found a primary device try a different heuristic */
    if (primaryBus.type == BUS_NONE && num) {
	for (i = 0; i < num; i++) {
	    uint16_t  command;

	    info = xf86PciVideoInfo[i];
	    pci_device_cfg_read_u16(info, & command, 4);

	    if ((command & PCI_CMD_MEM_ENABLE) 
		&& ((num == 1) || IS_VGA(info->device_class))) {
		if (primaryBus.type == BUS_NONE) {
		    primaryBus.type = BUS_PCI;
		    primaryBus.id.pci = info;
		} else {
		    xf86Msg(X_NOTICE,
			    "More than one possible primary device found\n");
		    primaryBus.type ^= (BusType)(-1);
		}
	    }
	}
    }
    
    /* Print a summary of the video devices found */
    for (k = 0; k < num; k++) {
	const char *vendorname = NULL, *chipname = NULL;
	const char *prim = " ";
	Bool memdone = FALSE, iodone = FALSE;


	info = xf86PciVideoInfo[k];

	vendorname = pci_device_get_vendor_name( info );
	chipname = pci_device_get_device_name( info );

	if ((!vendorname || !chipname) &&
	    !PCIALWAYSPRINTCLASSES(info->device_class))
	    continue;

	if (xf86IsPrimaryPci(info))
	    prim = "*";

	xf86Msg(X_PROBED, "PCI:%s(%u:%u:%u:%u) %04x:%04x:%04x:%04x ", prim,
		info->domain, info->bus, info->dev, info->func,
		info->vendor_id, info->device_id,
		info->subvendor_id, info->subdevice_id);

	if (vendorname)
	    xf86ErrorF("%s ", vendorname);

	if (chipname)
	    xf86ErrorF("%s ", chipname);

	xf86ErrorF("rev %d", info->revision);

	for (i = 0; i < 6; i++) {
	    struct pci_mem_region * r = & info->regions[i];

	    if ( r->size && ! r->is_IO ) {
		if (!memdone) {
		    xf86ErrorF(", Mem @ ");
		    memdone = TRUE;
		} else
		    xf86ErrorF(", ");
		xf86ErrorF("0x%08lx/%ld", (long)r->base_addr, (long)r->size);
	    }
	}

	for (i = 0; i < 6; i++) {
	    struct pci_mem_region * r = & info->regions[i];

	    if ( r->size && r->is_IO ) {
		if (!iodone) {
		    xf86ErrorF(", I/O @ ");
		    iodone = TRUE;
		} else
		    xf86ErrorF(", ");
		xf86ErrorF("0x%08lx/%ld", (long)r->base_addr, (long)r->size);
	    }
	}

	if ( info->rom_size ) {
	    xf86ErrorF(", BIOS @ 0x\?\?\?\?\?\?\?\?/%ld", (long)info->rom_size);
	}

	xf86ErrorF("\n");
    }
}

void
initPciState(void)
{
    unsigned i;
    pciAccPtr pcaccp;

    if (xf86PciVideoInfo == NULL) {
	return;
    }

    for (i = 0 ; xf86PciVideoInfo[i] != NULL ; i++) {
	struct pci_device * const pvp = xf86PciVideoInfo[i];

	if (pvp->user_data == 0) {
	    pcaccp = xnfalloc( sizeof( pciAccRec ) );
	    pvp->user_data = (intptr_t) pcaccp;

	    pcaccp->arg.dev = pvp;
	    pcaccp->ioAccess.AccessDisable = pciIoAccessDisable;
	    pcaccp->ioAccess.AccessEnable = pciIoAccessEnable;
	    pcaccp->ioAccess.arg = &pcaccp->arg;
	    pcaccp->io_memAccess.AccessDisable = pciIo_MemAccessDisable;
	    pcaccp->io_memAccess.AccessEnable = pciIo_MemAccessEnable;
	    pcaccp->io_memAccess.arg = &pcaccp->arg;
	    pcaccp->memAccess.AccessDisable = pciMemAccessDisable;
	    pcaccp->memAccess.AccessEnable = pciMemAccessEnable;
	    pcaccp->memAccess.arg = &pcaccp->arg;

	    pcaccp->ctrl = PCISHAREDIOCLASSES(pvp->device_class);

	    savePciState(pvp, &pcaccp->save);
	    pcaccp->arg.ctrl = pcaccp->save.command;
	}
    }
}

/*
 * initPciBusState() - fill out the BusAccRec for a PCI bus.
 * Theory: each bus is associated with one bridge connecting it
 * to its parent bus. The address of a bridge is therefore stored
 * in the BusAccRec of the bus it connects to. Each bus can
 * have several bridges connecting secondary buses to it. Only one
 * of these bridges can be open. Therefore the status of a bridge
 * associated with a bus is stored in the BusAccRec of the parent
 * the bridge connects to. The first member of the structure is
 * a pointer to a function that open access to this bus. This function
 * receives a pointer to the structure itself as argument. This
 * design should be common to BusAccRecs of any type of buses we
 * support. The remeinder of the structure is bus type specific.
 * In this case it contains a pointer to the structure of the
 * parent bus. Thus enabling access to a specific bus is simple:
 * 1. Close any bridge going to secondary buses.
 * 2. Climb down the ladder and enable any bridge on buses
 *    on the path from the CPU to this bus.
 */
 
void
initPciBusState(void)
{
    static const struct pci_id_match bridge_match = {
	PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY,
	(PCI_CLASS_BRIDGE << 16), 0x0000ff0000, 0
    };
    struct pci_device *dev;
    struct pci_device_iterator *iter;
    BusAccPtr pbap, pbap_tmp;

    iter = pci_id_match_iterator_create(& bridge_match);
    while((dev = pci_device_next(iter)) != NULL) {
	const uint8_t subclass = (dev->device_class >> 8) & 0x0ff;
	int primary;
	int secondary;
	int subordinate;


	pci_device_get_bridge_buses(dev, &primary, &secondary, &subordinate);

	pbap = xnfcalloc(1,sizeof(BusAccRec));
	pbap->busdep.pci.bus = secondary;
	pbap->busdep.pci.primary_bus = primary;
	pbap->busdep_type = BUS_PCI;
	pbap->busdep.pci.dev = dev;

	pbap->set_f = pciSetBusAccess;
	
	switch (subclass) {
	case PCI_SUBCLASS_BRIDGE_HOST:
	    pbap->type = BUS_PCI;
	    break;
	case PCI_SUBCLASS_BRIDGE_PCI:
	case PCI_SUBCLASS_BRIDGE_CARDBUS:
	    pbap->type = BUS_PCI;
	    pbap->save_f = savePciBusState;
	    pbap->restore_f = restorePciBusState;
	    pbap->enable_f = pciBusAccessEnable;
	    pbap->disable_f = pciBusAccessDisable;
	    savePciBusState(pbap);
	    break;
	}
	pbap->next = xf86BusAccInfo;
	xf86BusAccInfo = pbap;
    }

    pci_iterator_destroy(iter);

    for (pbap = xf86BusAccInfo; pbap; pbap = pbap->next) {
	pbap->primary = NULL;

	if (pbap->busdep_type == BUS_PCI
	    && pbap->busdep.pci.primary_bus > -1) {
	    pbap_tmp = xf86BusAccInfo;
	    while (pbap_tmp) {
		if (pbap_tmp->busdep_type == BUS_PCI &&
		    pbap_tmp->busdep.pci.bus == pbap->busdep.pci.primary_bus) {
		    /* Don't create loops */
		    if (pbap == pbap_tmp)
			break;
		    pbap->primary = pbap_tmp;
		    break;
		}
		pbap_tmp = pbap_tmp->next;
	    }
	}
    }
}

void 
PciStateEnter(void)
{
#if 0
    unsigned i;

    if (xf86PciVideoInfo == NULL)
	return;

    for ( i = 0 ; xf86PciVideoInfo[i] != NULL ; i++ ) {
	pciAccPtr paccp = (pciAccPtr) xf86PciVideoInfo[i]->user_data;

 	if ( (paccp != NULL) && paccp->ctrl ) {
	    savePciState(paccp->arg.dev, &paccp->save);
	    restorePciState(paccp->arg.dev, &paccp->restore);
	    paccp->arg.ctrl = paccp->restore.command;
	}
    }
#endif
}

void
PciBusStateEnter(void)
{
#if 0
    BusAccPtr pbap = xf86BusAccInfo;

    while (pbap) {
	if (pbap->save_f)
	    pbap->save_f(pbap);
	pbap = pbap->next;
    }
#endif
}

void 
PciStateLeave(void)
{
#if 0
    unsigned i;

    if (xf86PciVideoInfo == NULL)
	return;

    for ( i = 0 ; xf86PciVideoInfo[i] != NULL ; i++ ) {
	pciAccPtr paccp = (pciAccPtr) xf86PciVideoInfo[i]->user_data;

 	if ( (paccp != NULL) && paccp->ctrl ) {
	    savePciState(paccp->arg.dev, &paccp->restore);
	    restorePciState(paccp->arg.dev, &paccp->save);
	}
    }
#endif
}

void
PciBusStateLeave(void)
{
#if 0
    BusAccPtr pbap = xf86BusAccInfo;

    while (pbap) {
	if (pbap->restore_f)
	    pbap->restore_f(pbap);
	pbap = pbap->next;
    }
#endif
}

void 
DisablePciAccess(void)
{
#if 0
    unsigned i;

    if (xf86PciVideoInfo == NULL)
	return;

    for ( i = 0 ; xf86PciVideoInfo[i] != NULL ; i++ ) {
	pciAccPtr paccp = (pciAccPtr) xf86PciVideoInfo[i]->user_data;

 	if ( (paccp != NULL) && paccp->ctrl ) {
	    pciIo_MemAccessDisable(paccp->io_memAccess.arg);
	}
    }
#endif
}

void
DisablePciBusAccess(void)
{
#if 0
    BusAccPtr pbap = xf86BusAccInfo;

    while (pbap) {
	if (pbap->disable_f)
	    pbap->disable_f(pbap);
	if (pbap->primary)
	    pbap->primary->current = NULL;
	pbap = pbap->next;
    }
#endif
}

/*
 * If the slot requested is already in use, return -1.
 * Otherwise, claim the slot for the screen requesting it.
 */

_X_EXPORT int
xf86ClaimPciSlot(struct pci_device * d, DriverPtr drvp,
		 int chipset, GDevPtr dev, Bool active)
{
    EntityPtr p = NULL;
    pciAccPtr paccp = (pciAccPtr) d->user_data;
    BusAccPtr pbap = xf86BusAccInfo;
    const unsigned bus = PCI_MAKE_BUS(d->domain, d->bus);
    
    int num;
    
    if (xf86CheckPciSlot(d)) {
	num = xf86AllocateEntity();
	p = xf86Entities[num];
	p->driver = drvp;
	p->chipset = chipset;
	p->bus.type = BUS_PCI;
	p->bus.id.pci = d;
	p->active = active;
	p->inUse = FALSE;
	if (dev)
            xf86AddDevToEntity(num, dev);
	/* Here we initialize the access structure */
	p->access = xnfcalloc(1,sizeof(EntityAccessRec));
	if (paccp != NULL) {
	    p->access->fallback = & paccp->io_memAccess;
	    p->access->pAccess = & paccp->io_memAccess;
	    paccp->ctrl = TRUE; /* mark control if not already */
	}
	else {
	    p->access->fallback = &AccessNULL;
	    p->access->pAccess = &AccessNULL;
	}
	
	p->busAcc = NULL;
	while (pbap) {
	    if (pbap->type == BUS_PCI && pbap->busdep.pci.bus == bus)
		p->busAcc = pbap;
	    pbap = pbap->next;
	}

	pciSlotClaimed = TRUE;

	if (active) {
	    /* Map in this domain's I/O space */
	   p->domainIO = xf86MapLegacyIO(d);
	}
	
 	return num;
    } else
 	return -1;
}

/*
 * Parse a BUS ID string, and return the PCI bus parameters if it was
 * in the correct format for a PCI bus id.
 */

_X_EXPORT Bool
xf86ParsePciBusString(const char *busID, int *bus, int *device, int *func)
{
    /*
     * The format is assumed to be "bus[@domain]:device[:func]", where domain,
     * bus, device and func are decimal integers.  domain and func may be
     * omitted and assumed to be zero, although doing this isn't encouraged.
     */

    char *p, *s, *d;
    const char *id;
    int i;

    if (StringToBusType(busID, &id) != BUS_PCI)
	return FALSE;

    s = xstrdup(id);
    p = strtok(s, ":");
    if (p == NULL || *p == 0) {
	xfree(s);
	return FALSE;
    }
    d = strpbrk(p, "@");
    if (d != NULL) {
	*(d++) = 0;
	for (i = 0; d[i] != 0; i++) {
	    if (!isdigit(d[i])) {
		xfree(s);
		return FALSE;
	    }
	}
    }
    for (i = 0; p[i] != 0; i++) {
	if (!isdigit(p[i])) {
	    xfree(s);
	    return FALSE;
	}
    }
    *bus = atoi(p);
    if (d != NULL && *d != 0)
	*bus += atoi(d) << 8;
    p = strtok(NULL, ":");
    if (p == NULL || *p == 0) {
	xfree(s);
	return FALSE;
    }
    for (i = 0; p[i] != 0; i++) {
	if (!isdigit(p[i])) {
	    xfree(s);
	    return FALSE;
	}
    }
    *device = atoi(p);
    *func = 0;
    p = strtok(NULL, ":");
    if (p == NULL || *p == 0) {
	xfree(s);
	return TRUE;
    }
    for (i = 0; p[i] != 0; i++) {
	if (!isdigit(p[i])) {
	    xfree(s);
	    return FALSE;
	}
    }
    *func = atoi(p);
    xfree(s);
    return TRUE;
}

/*
 * Compare a BUS ID string with a PCI bus id.  Return TRUE if they match.
 */

_X_EXPORT Bool
xf86ComparePciBusString(const char *busID, int bus, int device, int func)
{
    int ibus, idevice, ifunc;

    if (xf86ParsePciBusString(busID, &ibus, &idevice, &ifunc)) {
	return bus == ibus && device == idevice && func == ifunc;
    } else {
	return FALSE;
    }
}

/*
 * xf86IsPrimaryPci() -- return TRUE if primary device
 * is PCI and bus, dev and func numbers match.
 */
 
_X_EXPORT Bool
xf86IsPrimaryPci(struct pci_device *pPci)
{
    return ((primaryBus.type == BUS_PCI) && (pPci == primaryBus.id.pci));
}

/*
 * xf86GetPciInfoForEntity() -- Get the pciVideoRec of entity.
 */
_X_EXPORT struct pci_device *
xf86GetPciInfoForEntity(int entityIndex)
{
    EntityPtr p;
    
    if (entityIndex >= xf86NumEntities)
	return NULL;

    p = xf86Entities[entityIndex];
    return (p->bus.type == BUS_PCI) ? p->bus.id.pci : NULL;
}

/*
 * xf86CheckPciMemBase() checks that the memory base value matches one of the
 * PCI base address register values for the given PCI device.
 */
_X_EXPORT Bool
xf86CheckPciMemBase( struct pci_device * pPci, memType base )
{
    int i;

    for (i = 0; i < 6; i++)
	if (base == pPci->regions[i].base_addr)
	    return TRUE;
    return FALSE;
}

/*
 * Check if the slot requested is free.  If it is already in use, return FALSE.
 */

_X_EXPORT Bool
xf86CheckPciSlot(const struct pci_device *d)
{
    int i;

    for (i = 0; i < xf86NumEntities; i++) {
	const EntityPtr p = xf86Entities[i];

	if ((p->bus.type == BUS_PCI) && (p->bus.id.pci == d)) {
	    return FALSE;
	}
    }
    return TRUE;
}


void
pciConvertRange2Host(int entityIndex, resRange *pRange)
{
    struct pci_device *const pvp = xf86GetPciInfoForEntity(entityIndex);
    const PCITAG tag = PCI_MAKE_TAG(PCI_MAKE_BUS(pvp->domain, pvp->bus),
				    pvp->dev, pvp->func);

    if (pvp == NULL) {
	return;
    }

    if (!(pRange->type & ResBus))
	return;

    switch(pRange->type & ResPhysMask) {
    case ResMem:
	switch(pRange->type & ResExtMask) {
	case ResBlock:
	    pRange->rBegin = pciBusAddrToHostAddr(tag,PCI_MEM, pRange->rBegin);
	    pRange->rEnd = pciBusAddrToHostAddr(tag,PCI_MEM, pRange->rEnd);
	    break;
	case ResSparse:
	    pRange->rBase = pciBusAddrToHostAddr(tag,PCI_MEM_SPARSE_BASE,
						  pRange->rBegin);
	    pRange->rMask = pciBusAddrToHostAddr(tag,PCI_MEM_SPARSE_MASK,
						pRange->rEnd);
	    break;
	}
	break;
    case ResIo:
	switch(pRange->type & ResExtMask) {
	case ResBlock:
	    pRange->rBegin = pciBusAddrToHostAddr(tag,PCI_IO, pRange->rBegin);
	    pRange->rEnd = pciBusAddrToHostAddr(tag,PCI_IO, pRange->rEnd);
	    break;
	case ResSparse:
	    pRange->rBase = pciBusAddrToHostAddr(tag,PCI_IO_SPARSE_BASE
						  , pRange->rBegin);
	    pRange->rMask = pciBusAddrToHostAddr(tag,PCI_IO_SPARSE_MASK
						, pRange->rEnd);
	    break;
	}
	break;
    }

    /* Set domain number */
    pRange->type &= ~(ResDomain | ResBus);
    pRange->type |= pvp->domain << 24;
}
