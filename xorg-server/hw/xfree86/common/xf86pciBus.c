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

/* Bus-specific headers */
#include "xf86Bus.h"

#define XF86_OS_PRIVS
#include "xf86_OSproc.h"


/* Bus-specific globals */
Bool pciSlotClaimed = FALSE;

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

void
xf86FormatPciBusNumber(int busnum, char *buffer)
{
    /* 'buffer' should be at least 8 characters long */
    if (busnum < 256)
	sprintf(buffer, "%d", busnum);
    else
	sprintf(buffer, "%d@%d", busnum & 0x00ff, busnum >> 8);
}

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
    struct pci_device ** xf86PciVideoInfo = NULL;


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
#ifdef HAVE_PCI_DEVICE_IS_BOOT_VGA
	    if (pci_device_is_boot_vga(info)) {
                primaryBus.type = BUS_PCI;
                primaryBus.id.pci = info;
            }
#endif
	    info->user_data = 0;
	}
    }
    free(iter);

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
    free(xf86PciVideoInfo);
}

/*
 * If the slot requested is already in use, return -1.
 * Otherwise, claim the slot for the screen requesting it.
 */

int
xf86ClaimPciSlot(struct pci_device * d, DriverPtr drvp,
		 int chipset, GDevPtr dev, Bool active)
{
    EntityPtr p = NULL;
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
 * Unclaim PCI slot, e.g. if probing failed, so that a different driver can claim.
 */
void
xf86UnclaimPciSlot(struct pci_device *d)
{
    int i;

    for (i = 0; i < xf86NumEntities; i++) {
	const EntityPtr p = xf86Entities[i];

	if ((p->bus.type == BUS_PCI) && (p->bus.id.pci == d)) {
	    /* Probably the slot should be deallocated? */
	    p->bus.type = BUS_NONE;
	    return;
	}
    }
}

/*
 * Parse a BUS ID string, and return the PCI bus parameters if it was
 * in the correct format for a PCI bus id.
 */

Bool
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
	free(s);
	return FALSE;
    }
    d = strpbrk(p, "@");
    if (d != NULL) {
	*(d++) = 0;
	for (i = 0; d[i] != 0; i++) {
	    if (!isdigit(d[i])) {
		free(s);
		return FALSE;
	    }
	}
    }
    for (i = 0; p[i] != 0; i++) {
	if (!isdigit(p[i])) {
	    free(s);
	    return FALSE;
	}
    }
    *bus = atoi(p);
    if (d != NULL && *d != 0)
	*bus += atoi(d) << 8;
    p = strtok(NULL, ":");
    if (p == NULL || *p == 0) {
	free(s);
	return FALSE;
    }
    for (i = 0; p[i] != 0; i++) {
	if (!isdigit(p[i])) {
	    free(s);
	    return FALSE;
	}
    }
    *device = atoi(p);
    *func = 0;
    p = strtok(NULL, ":");
    if (p == NULL || *p == 0) {
	free(s);
	return TRUE;
    }
    for (i = 0; p[i] != 0; i++) {
	if (!isdigit(p[i])) {
	    free(s);
	    return FALSE;
	}
    }
    *func = atoi(p);
    free(s);
    return TRUE;
}

/*
 * Compare a BUS ID string with a PCI bus id.  Return TRUE if they match.
 */

Bool
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
 
Bool
xf86IsPrimaryPci(struct pci_device *pPci)
{
    return ((primaryBus.type == BUS_PCI) && (pPci == primaryBus.id.pci));
}

/*
 * xf86GetPciInfoForEntity() -- Get the pciVideoRec of entity.
 */
struct pci_device *
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
Bool
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

Bool
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

#define END_OF_MATCHES(m) \
    (((m).vendor_id == 0) && ((m).device_id == 0) && ((m).subvendor_id == 0))

Bool
xf86PciAddMatchingDev(DriverPtr drvp)
{
    const struct pci_id_match * const devices = drvp->supported_devices;
    int j;
    struct pci_device *pPci;
    struct pci_device_iterator *iter;
    int numFound = 0;


    iter = pci_id_match_iterator_create(NULL);
    while ((pPci = pci_device_next(iter)) != NULL) {
    /* Determine if this device is supported by the driver.  If it is,
     * add it to the list of devices to configure.
     */
    for (j = 0 ; ! END_OF_MATCHES(devices[j]) ; j++) {
        if ( PCI_ID_COMPARE( devices[j].vendor_id, pPci->vendor_id )
         && PCI_ID_COMPARE( devices[j].device_id, pPci->device_id )
         && ((devices[j].device_class_mask & pPci->device_class)
             == devices[j].device_class) ) {
        if (xf86CheckPciSlot(pPci)) {
            GDevPtr pGDev = xf86AddBusDeviceToConfigure(
                    drvp->driverName, BUS_PCI, pPci, -1);
            if (pGDev != NULL) {
            /* After configure pass 1, chipID and chipRev are
             * treated as over-rides, so clobber them here.
             */
            pGDev->chipID = -1;
            pGDev->chipRev = -1;
            }

            numFound++;
        }

        break;
        }
    }
    }

    pci_iterator_destroy(iter);

    return (numFound != 0);
}

Bool
xf86PciProbeDev(DriverPtr drvp)
{
    int i, j;
    struct pci_device * pPci;
    Bool foundScreen = FALSE;
    const struct pci_id_match * const devices = drvp->supported_devices;
    GDevPtr *devList;
    const unsigned numDevs = xf86MatchDevice(drvp->driverName, & devList);

    for ( i = 0 ; i < numDevs ; i++ ) {
       struct pci_device_iterator *iter;
       unsigned device_id;


       /* Find the pciVideoRec associated with this device section.
        */
       iter = pci_id_match_iterator_create(NULL);
       while ((pPci = pci_device_next(iter)) != NULL) {
           if (devList[i]->busID && *devList[i]->busID) {
               if (xf86ComparePciBusString(devList[i]->busID,
                                           ((pPci->domain << 8)
                                            | pPci->bus),
                                           pPci->dev,
                                           pPci->func)) {
                   break;
               }
           }
           else if (xf86IsPrimaryPci(pPci)) {
               break;
           }
       }

       pci_iterator_destroy(iter);

       if (pPci == NULL) {
           continue;
       }
       device_id = (devList[i]->chipID > 0)
         ? devList[i]->chipID : pPci->device_id;


       /* Once the pciVideoRec is found, determine if the device is supported
        * by the driver.  If it is, probe it!
        */
       for ( j = 0 ; ! END_OF_MATCHES( devices[j] ) ; j++ ) {
           if ( PCI_ID_COMPARE( devices[j].vendor_id, pPci->vendor_id )
                && PCI_ID_COMPARE( devices[j].device_id, device_id )
                && ((devices[j].device_class_mask & pPci->device_class)
                     == devices[j].device_class) ) {
               int  entry;

               /* Allow the same entity to be used more than once for
                * devices with multiple screens per entity.  This assumes
                * implicitly that there will be a screen == 0 instance.
                *
                * FIXME Need to make sure that two different drivers don't
                * FIXME claim the same screen > 0 instance.
                */
               if ((devList[i]->screen == 0) && !xf86CheckPciSlot(pPci))
                   continue;

               DebugF("%s: card at %d:%d:%d is claimed by a Device section\n",
                      drvp->driverName, pPci->bus, pPci->dev, pPci->func);

               /* Allocate an entry in the lists to be returned */
               entry = xf86ClaimPciSlot(pPci, drvp, device_id,
                                         devList[i], devList[i]->active);

               if ((entry == -1) && (devList[i]->screen > 0)) {
                   unsigned k;

                   for (k = 0; k < xf86NumEntities; k++ ) {
                       EntityPtr pEnt = xf86Entities[k];
                       if (pEnt->bus.type != BUS_PCI)
                           continue;
                       if (pEnt->bus.id.pci == pPci) {
                           entry = k;
                           xf86AddDevToEntity(k, devList[i]);
                           break;
                       }
                   }
               }

               if (entry != -1) {
                   if ((*drvp->PciProbe)(drvp, entry, pPci,
                                         devices[j].match_data)) {
                       foundScreen = TRUE;
                   } else
                       xf86UnclaimPciSlot(pPci);
               }

               break;
           }
       }
    }
    free(devList);

    return foundScreen;
}

void
xf86PciIsolateDevice(char *argument)
{
    int bus, device, func;

    if (sscanf(argument, "PCI:%d:%d:%d", &bus, &device, &func) == 3) {
        xf86IsolateDevice.domain = PCI_DOM_FROM_BUS(bus);
        xf86IsolateDevice.bus = PCI_BUS_NO_DOMAIN(bus);
        xf86IsolateDevice.dev = device;
        xf86IsolateDevice.func = func;
    } else
        FatalError("Invalid isolated device specification\n");
}
