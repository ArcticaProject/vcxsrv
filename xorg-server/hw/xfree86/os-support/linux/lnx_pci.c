
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <X11/X.h>
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#define XF86_OS_PRIVS
#include "xf86_OSproc.h"
#include "xf86Pci.h"
#include "Pci.h"

#ifdef __sparc__
#define PCIADDR_TYPE		long long
#define PCIADDR_IGNORE_FMT	"%*x"
#define PCIADDR_FMT		"%llx"
#else
#define PCIADDR_TYPE		long
#define PCIADDR_IGNORE_FMT	"%*x"
#define PCIADDR_FMT		"%lx"
#endif

struct pci_dev {
    unsigned int domain;
    unsigned int bus;
    unsigned int dev;
    unsigned int fn;
    PCIADDR_TYPE offset[7];
    PCIADDR_TYPE size[7];
    struct pci_dev *next;
};

struct pci_dev *xf86OSLinuxPCIDevs = NULL;
int xf86OSLinuxNumPciDevs = 0;

static struct pci_dev *xf86OSLinuxGetPciDevs(void) {
    char c[0x200];
    FILE *file = NULL;
    DIR  *dir;
    struct dirent *dirent;
    struct pci_dev *tmp, *ret = NULL;
    unsigned int i, num, devfn;
    unsigned PCIADDR_TYPE begin, end;
    char *res;
    
    /* Try 2.6 devices first, with domain support */
    if ( (dir = opendir ("/sys/bus/pci/devices")) ) {
	xf86OSLinuxNumPciDevs = 0;
	while ( (dirent = readdir (dir)) ) {
	    unsigned int domain, bus, dev, fn;
	    if (sscanf (dirent->d_name, "%04x:%02x:%02x.%01x",
			&domain, &bus, &dev, &fn) == 4) {
		tmp = xcalloc (sizeof(struct pci_dev), 1);
		tmp->domain = domain;
		tmp->bus    = bus;
		tmp->dev    = dev;
		tmp->fn     = fn;
		sprintf (c, "/sys/bus/pci/devices/%12s/resource",
			 dirent->d_name);
		i = 0;
		if ( (file = fopen (c, "r")) ) {
		    while (i < 7 && fgets (c, 0x200, file)) {
			if (sscanf (c, PCIADDR_FMT " " PCIADDR_FMT " "
				    PCIADDR_IGNORE_FMT, &begin, &end) == 2) {
			    tmp->offset[i] = begin;
			    tmp->size[i]   = begin ? end-begin+1 : 0;
			    i++;
			}
		    }
		    fclose (file);
		}
		if (i > 0) {
		    tmp->next = ret;
		    ret       = tmp;
		    xf86OSLinuxNumPciDevs++;
		} else
		    xfree (tmp);
	    }
	}
	closedir (dir);
    }

    if (ret)
	return ret;

    file = fopen("/proc/bus/pci/devices", "r");
    if (!file) return NULL;

    xf86OSLinuxNumPciDevs = 0;
    
    do {
        res = fgets(c, 0x1ff, file);
        if (res) {
            tmp = xcalloc(sizeof(struct pci_dev),1);
            num = sscanf(res,
                /*bus+dev vendorid deviceid irq */
                "%02x%02x\t%*04x%*04x\t%*x"
                /* 7 PCI resource base addresses */
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                /* 7 PCI resource sizes, and then optionally a driver name */
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT
                "\t" PCIADDR_FMT,
                &tmp->bus,&devfn,&tmp->offset[0],&tmp->offset[1],&tmp->offset[2],&tmp->offset[3],
                &tmp->offset[4],&tmp->offset[5],&tmp->offset[6], &tmp->size[0], &tmp->size[1], &tmp->size[2],
                &tmp->size[3], &tmp->size[4], &tmp->size[5], &tmp->size[6]);
            tmp->dev = devfn >> 3;
            tmp->fn  = devfn & 0x7;
            if (num != 16) {  /* apparantly not 2.3 style */
                xfree(tmp);
                fclose(file);
                return NULL;
            }
            if (ret) {
                tmp->next = ret;
            }
            ret = tmp;
            xf86OSLinuxNumPciDevs++;
        }
    } while (res);
    fclose(file);
    return ret;
}

/* Query the kvirt address (64bit) of a BAR range from size for a given TAG */
unsigned long
xf86GetOSOffsetFromPCI(PCITAG tag, int space, unsigned long base)
{
    unsigned int ndx;
    struct pci_dev *device;
    struct pci_device *dev;

    if (!xf86OSLinuxPCIDevs) {
        xf86OSLinuxPCIDevs = xf86OSLinuxGetPciDevs();
    }
    if (!xf86OSLinuxPCIDevs) {
        return FALSE;
    }

    for (device = xf86OSLinuxPCIDevs; device; device = device->next) {
	dev = pci_device_find_by_slot(device->domain, device->bus, 
				      device->dev, device->fn);
        if (dev != NULL) {
            /* ok now look through all the BAR values of this device */
            for (ndx=0; ndx<7; ndx++) {
                uint32_t savePtr;
	        uint32_t flagMask;

		/* The ROM BAR isn't with the other BARs.
		 */
		const pciaddr_t offset = (ndx == 6) 
		  ? (4 * 12) : (4 * ndx) + 16;

		pci_device_cfg_read_u32(dev, &savePtr, offset);

                /* Ignore unset base addresses. The kernel may have reported
		 * non-zero size and address even if they are disabled (e.g.,
		 * disabled ROM BAR).
                 */
                if (savePtr == 0)
                    continue;

                /* Remove memory attribute bits, different for IO
                 * and memory ranges. 
		 */
                flagMask = (savePtr & 0x1) ? ~0x3UL : ~0xFUL;
                savePtr &= flagMask;

                /* find the index of the incoming base */
                if (base >= savePtr && base < (savePtr + device->size[ndx])) {
                    return (device->offset[ndx] & flagMask) + (base - savePtr);
                }
            }
        }
    }

    return 0;
}
