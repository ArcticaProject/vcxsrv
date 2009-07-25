/*
 * Copyright 2000-2002 by Alan Hourihane, Flint Mountain, North Wales.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Alan Hourihane, alanh@fairlite.demon.co.uk
 *
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <pciaccess.h>
#include "Pci.h"
#include "os.h"
#include "loaderProcs.h"
#include "xf86.h"
#include "xf86Config.h"
#include "xf86_OSlib.h"
#include "xf86Priv.h"
#define IN_XSERVER
#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"
#include "vbe.h"
#include "xf86DDC.h"
#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
#include "xf86Bus.h"
#include "xf86Sbus.h"
#endif
#include "globals.h"

typedef struct _DevToConfig {
    GDevRec GDev;
    struct pci_device * pVideo;
#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
    sbusDevicePtr sVideo;
#endif
    int iDriver;
} DevToConfigRec, *DevToConfigPtr;

static DevToConfigPtr DevToConfig = NULL;
static int nDevToConfig = 0, CurrentDriver;

_X_EXPORT xf86MonPtr ConfiguredMonitor;
Bool xf86DoConfigurePass1 = TRUE;
static Bool foundMouse = FALSE;

#if defined(__SCO__)
static char *DFLT_MOUSE_PROTO = "OSMouse";
#elif defined(__UNIXWARE__)
static char *DFLT_MOUSE_PROTO = "OSMouse";
static char *DFLT_MOUSE_DEV = "/dev/mouse";
#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
static char *DFLT_MOUSE_DEV = "/dev/sysmouse";
static char *DFLT_MOUSE_PROTO = "auto";
#elif defined(linux)
static char DFLT_MOUSE_DEV[] = "/dev/input/mice";
static char DFLT_MOUSE_PROTO[] = "auto";
#else
static char *DFLT_MOUSE_DEV = "/dev/mouse";
static char *DFLT_MOUSE_PROTO = "auto";
#endif

/*
 * This is called by the driver, either through xf86Match???Instances() or
 * directly.  We allocate a GDevRec and fill it in as much as we can, letting
 * the caller fill in the rest and/or change it as it sees fit.
 */
GDevPtr
xf86AddBusDeviceToConfigure(const char *driver, BusType bus, void *busData, int chipset)
{
    int i, j;
    struct pci_device * pVideo = NULL;
    Bool isPrimary = FALSE;

    if (xf86DoProbe || !xf86DoConfigure || !xf86DoConfigurePass1)
	return NULL;

    /* Check for duplicates */
    switch (bus) {
    case BUS_PCI:
	pVideo = (struct pci_device *) busData;
	for (i = 0;  i < nDevToConfig;  i++)
	    if (DevToConfig[i].pVideo &&
		(DevToConfig[i].pVideo->domain == pVideo->domain) &&
		(DevToConfig[i].pVideo->bus == pVideo->bus) &&
		(DevToConfig[i].pVideo->dev == pVideo->dev) &&
		(DevToConfig[i].pVideo->func == pVideo->func))
		return NULL;
	isPrimary = xf86IsPrimaryPci(pVideo);
	break;
#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
    case BUS_SBUS:
	for (i = 0;  i < nDevToConfig;  i++)
	    if (DevToConfig[i].sVideo &&
		DevToConfig[i].sVideo->fbNum == ((sbusDevicePtr) busData)->fbNum)
		return NULL;
	break;
#endif
    default:
	return NULL;
    }

    /* Allocate new structure occurrence */
    i = nDevToConfig++;
    DevToConfig =
	xnfrealloc(DevToConfig, nDevToConfig * sizeof(DevToConfigRec));
#if 1   /* Doesn't work when a driver detects more than one adapter */
    if ((i > 0) && isPrimary) {
        memmove(DevToConfig + 1,DevToConfig,
		(nDevToConfig - 1) * sizeof(DevToConfigRec));
	i = 0;
    } 
#endif
    memset(DevToConfig + i, 0, sizeof(DevToConfigRec));

#   define NewDevice DevToConfig[i]

    NewDevice.GDev.chipID = NewDevice.GDev.chipRev = NewDevice.GDev.irq = -1;

    NewDevice.iDriver = CurrentDriver;

    /* Fill in what we know, converting the driver name to lower case */
    NewDevice.GDev.driver = xnfalloc(strlen(driver) + 1);
    for (j = 0;  (NewDevice.GDev.driver[j] = tolower(driver[j]));  j++);

    switch (bus) {
    case BUS_PCI: {
	const char *VendorName;
	const char *CardName;
	char busnum[8];

	NewDevice.pVideo = pVideo;

	VendorName = pci_device_get_vendor_name( pVideo );
	CardName = pci_device_get_device_name( pVideo );

	if (!VendorName) {
	    VendorName = xnfalloc(15);
	    sprintf((char*)VendorName, "Unknown Vendor");
	}

	if (!CardName) {
	    CardName = xnfalloc(14);
	    sprintf((char*)CardName, "Unknown Board");
	}

	NewDevice.GDev.identifier =
	    xnfalloc(strlen(VendorName) + strlen(CardName) + 2);
	sprintf(NewDevice.GDev.identifier, "%s %s", VendorName, CardName);

	NewDevice.GDev.vendor = (char *)VendorName;
	NewDevice.GDev.board = (char *)CardName;

	NewDevice.GDev.busID = xnfalloc(16);
	xf86FormatPciBusNumber(pVideo->bus, busnum);
	sprintf(NewDevice.GDev.busID, "PCI:%s:%d:%d",
	    busnum, pVideo->dev, pVideo->func);

	NewDevice.GDev.chipID = pVideo->device_id;
	NewDevice.GDev.chipRev = pVideo->revision;

	if (chipset < 0)
	    chipset = (pVideo->vendor_id << 16) | pVideo->device_id;
	}
	break;
#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
    case BUS_SBUS: {
	char *promPath = NULL;
	NewDevice.sVideo = (sbusDevicePtr) busData;
	NewDevice.GDev.identifier = NewDevice.sVideo->descr;
	if (sparcPromInit() >= 0) {
	    promPath = sparcPromNode2Pathname(&NewDevice.sVideo->node);
	    sparcPromClose();
	}
	if (promPath) {
	    NewDevice.GDev.busID = xnfalloc(strlen(promPath) + 6);
	    sprintf(NewDevice.GDev.busID, "SBUS:%s", promPath);
	    xfree(promPath);
	} else {
	    NewDevice.GDev.busID = xnfalloc(12);
	    sprintf(NewDevice.GDev.busID, "SBUS:fb%d", NewDevice.sVideo->fbNum);
	}
	}
	break;
#endif
    default:
	break;
    }

    /* Get driver's available options */
    if (xf86DriverList[CurrentDriver]->AvailableOptions)
	NewDevice.GDev.options = (OptionInfoPtr)
	    (*xf86DriverList[CurrentDriver]->AvailableOptions)(chipset,
							       bus);

    return &NewDevice.GDev;

#   undef NewDevice
}

static XF86ConfInputPtr
configureInputSection (void)
{
    XF86ConfInputPtr mouse = NULL;
    parsePrologue (XF86ConfInputPtr, XF86ConfInputRec)

    ptr->inp_identifier = "Keyboard0";
    ptr->inp_driver = "kbd";
    ptr->list.next = NULL;

    /* Crude mechanism to auto-detect mouse (os dependent) */
    { 
	int fd;
#ifdef WSCONS_SUPPORT
	fd = open("/dev/wsmouse", 0);
	if (fd > 0) {
	    DFLT_MOUSE_DEV = "/dev/wsmouse";
	    DFLT_MOUSE_PROTO = "wsmouse";
	    close(fd);
	} else {
	    ErrorF("cannot open /dev/wsmouse\n");
	}
#endif

#ifndef __SCO__
	fd = open(DFLT_MOUSE_DEV, 0);
	if (fd != -1) {
	    foundMouse = TRUE;
	    close(fd);
	}
#else
	foundMouse = TRUE;
#endif
    }

    mouse = xf86confcalloc(1, sizeof(XF86ConfInputRec));
    mouse->inp_identifier = "Mouse0";
    mouse->inp_driver = "mouse";
    mouse->inp_option_lst = 
		xf86addNewOption(mouse->inp_option_lst, xstrdup("Protocol"),
				xstrdup(DFLT_MOUSE_PROTO));
#ifndef __SCO__
    mouse->inp_option_lst = 
		xf86addNewOption(mouse->inp_option_lst, xstrdup("Device"),
				xstrdup(DFLT_MOUSE_DEV));
#endif
    mouse->inp_option_lst = 
		xf86addNewOption(mouse->inp_option_lst, xstrdup("ZAxisMapping"),
				xstrdup("4 5 6 7"));
    ptr = (XF86ConfInputPtr)xf86addListItem((glp)ptr, (glp)mouse);
    return ptr;
}

static XF86ConfScreenPtr
configureScreenSection (int screennum)
{
    int i;
    int depths[] = { 1, 4, 8, 15, 16, 24/*, 32*/ };
    parsePrologue (XF86ConfScreenPtr, XF86ConfScreenRec)

    ptr->scrn_identifier = xf86confmalloc(18);
    sprintf(ptr->scrn_identifier, "Screen%d", screennum);
    ptr->scrn_monitor_str = xf86confmalloc(19);
    sprintf(ptr->scrn_monitor_str, "Monitor%d", screennum);
    ptr->scrn_device_str = xf86confmalloc(16);
    sprintf(ptr->scrn_device_str, "Card%d", screennum);

    for (i=0; i<sizeof(depths)/sizeof(depths[0]); i++)
    {
	XF86ConfDisplayPtr display;

	display = xf86confcalloc(1, sizeof(XF86ConfDisplayRec));
	display->disp_depth = depths[i];
	display->disp_black.red = display->disp_white.red = -1;
	display->disp_black.green = display->disp_white.green = -1;
	display->disp_black.blue = display->disp_white.blue = -1;
	ptr->scrn_display_lst = (XF86ConfDisplayPtr)xf86addListItem(
				     (glp)ptr->scrn_display_lst, (glp)display);
    }

    return ptr;
}

static const char* 
optionTypeToSting(OptionValueType type)
{
    switch (type) {
    case OPTV_NONE:
        return "";
    case OPTV_INTEGER:
        return "<i>";
    case OPTV_STRING:
        return "<str>";
    case OPTV_ANYSTR:
       return "[<str>]";
    case OPTV_REAL:
        return "<f>";
    case OPTV_BOOLEAN:
        return "[<bool>]";
    case OPTV_FREQ:
        return "<freq>";
    default:
        return "";
    }
}

static XF86ConfDevicePtr
configureDeviceSection (int screennum)
{
    char identifier[16];
    OptionInfoPtr p;
    int i = 0;
    parsePrologue (XF86ConfDevicePtr, XF86ConfDeviceRec)

    /* Move device info to parser structure */
    sprintf(identifier, "Card%d", screennum);
    ptr->dev_identifier = strdup(identifier);
/*    ptr->dev_identifier = DevToConfig[screennum].GDev.identifier;*/
    ptr->dev_vendor = DevToConfig[screennum].GDev.vendor;
    ptr->dev_board = DevToConfig[screennum].GDev.board;
    ptr->dev_chipset = DevToConfig[screennum].GDev.chipset;
    ptr->dev_busid = DevToConfig[screennum].GDev.busID;
    ptr->dev_driver = DevToConfig[screennum].GDev.driver;
    ptr->dev_ramdac = DevToConfig[screennum].GDev.ramdac;
    for (i = 0;  (i < MAXDACSPEEDS) && (i < CONF_MAXDACSPEEDS);  i++)
        ptr->dev_dacSpeeds[i] = DevToConfig[screennum].GDev.dacSpeeds[i];
    ptr->dev_videoram = DevToConfig[screennum].GDev.videoRam;
    ptr->dev_textclockfreq = DevToConfig[screennum].GDev.textClockFreq;
    ptr->dev_bios_base = DevToConfig[screennum].GDev.BiosBase;
    ptr->dev_mem_base = DevToConfig[screennum].GDev.MemBase;
    ptr->dev_io_base = DevToConfig[screennum].GDev.IOBase;
    ptr->dev_clockchip = DevToConfig[screennum].GDev.clockchip;
    for (i = 0;  (i < MAXCLOCKS) && (i < DevToConfig[screennum].GDev.numclocks);  i++)
        ptr->dev_clock[i] = DevToConfig[screennum].GDev.clock[i];
    ptr->dev_clocks = i;
    ptr->dev_chipid = DevToConfig[screennum].GDev.chipID;
    ptr->dev_chiprev = DevToConfig[screennum].GDev.chipRev;
    ptr->dev_irq = DevToConfig[screennum].GDev.irq;

    /* Make sure older drivers don't segv */
    if (DevToConfig[screennum].GDev.options) {
    	/* Fill in the available driver options for people to use */
	const char *descrip =
	    "        ### Available Driver options are:-\n"
	    "        ### Values: <i>: integer, <f>: float, "
			"<bool>: \"True\"/\"False\",\n"
	    "        ### <string>: \"String\", <freq>: \"<f> Hz/kHz/MHz\"\n"
	    "        ### [arg]: arg optional\n";
	ptr->dev_comment = xstrdup(descrip);
	if (ptr->dev_comment) {
    	    for (p = DevToConfig[screennum].GDev.options;
		 p->name != NULL; p++) {
		char *p_e;
		const char *prefix = "        #Option     ";
		const char *middle = " \t# ";
		const char *suffix = "\n";
		const char *opttype = optionTypeToSting(p->type);
		char *optname;
		int len = strlen(ptr->dev_comment) + strlen(prefix) +
			  strlen(middle) + strlen(suffix) + 1;
		
		optname = xalloc(strlen(p->name) + 2 + 1);
		if (!optname)
		    break;
		sprintf(optname, "\"%s\"", p->name);

		len += max(20, strlen(optname));
		len += strlen(opttype);

		ptr->dev_comment = xrealloc(ptr->dev_comment, len);
		if (!ptr->dev_comment)
		    break;
		p_e = ptr->dev_comment + strlen(ptr->dev_comment);
		sprintf(p_e, "%s%-20s%s%s%s", prefix, optname, middle,
			opttype, suffix);
		xfree(optname);
	    }
    	}
    }

    return ptr;
}

static XF86ConfLayoutPtr
configureLayoutSection (void)
{
    int scrnum = 0;
    parsePrologue (XF86ConfLayoutPtr, XF86ConfLayoutRec)

    ptr->lay_identifier = "X.org Configured";

    {
	XF86ConfInputrefPtr iptr;

	iptr = xf86confmalloc (sizeof (XF86ConfInputrefRec));
	iptr->list.next = NULL;
	iptr->iref_option_lst = NULL;
	iptr->iref_inputdev_str = "Mouse0";
	iptr->iref_option_lst =
		xf86addNewOption (iptr->iref_option_lst, xstrdup("CorePointer"), NULL);
	ptr->lay_input_lst = (XF86ConfInputrefPtr)
		xf86addListItem ((glp) ptr->lay_input_lst, (glp) iptr);
    }

    {
	XF86ConfInputrefPtr iptr;

	iptr = xf86confmalloc (sizeof (XF86ConfInputrefRec));
	iptr->list.next = NULL;
	iptr->iref_option_lst = NULL;
	iptr->iref_inputdev_str = "Keyboard0";
	iptr->iref_option_lst =
		xf86addNewOption (iptr->iref_option_lst, xstrdup("CoreKeyboard"), NULL);
	ptr->lay_input_lst = (XF86ConfInputrefPtr)
		xf86addListItem ((glp) ptr->lay_input_lst, (glp) iptr);
    }

    for (scrnum = 0;  scrnum < nDevToConfig;  scrnum++) {
	XF86ConfAdjacencyPtr aptr;

	aptr = xf86confmalloc (sizeof (XF86ConfAdjacencyRec));
	aptr->list.next = NULL;
	aptr->adj_x = 0;
	aptr->adj_y = 0;
	aptr->adj_scrnum = scrnum;
	aptr->adj_screen_str = xnfalloc(18);
	sprintf(aptr->adj_screen_str, "Screen%d", scrnum);
	if (scrnum == 0) {
	    aptr->adj_where = CONF_ADJ_ABSOLUTE;
	    aptr->adj_refscreen = NULL;
	}
	else {
	    aptr->adj_where = CONF_ADJ_RIGHTOF;
	    aptr->adj_refscreen = xnfalloc(18);
	    sprintf(aptr->adj_refscreen, "Screen%d", scrnum - 1);
	}
    	ptr->lay_adjacency_lst =
	    (XF86ConfAdjacencyPtr)xf86addListItem((glp)ptr->lay_adjacency_lst,
					      (glp)aptr);
    }

    return ptr;
}

static XF86ConfFlagsPtr
configureFlagsSection (void)
{
    parsePrologue (XF86ConfFlagsPtr, XF86ConfFlagsRec)

    return ptr;
}

static XF86ConfModulePtr
configureModuleSection (void)
{
    char **elist, **el;
    /* Find the list of extension modules. */
    const char *esubdirs[] = {
	"extensions",
	NULL
    };
    const char *fsubdirs[] = {
	"fonts",
	NULL
    };
    parsePrologue (XF86ConfModulePtr, XF86ConfModuleRec)

    elist = LoaderListDirs(esubdirs, NULL);
    if (elist) {
	for (el = elist; *el; el++) {
	    XF86LoadPtr module;

    	    module = xf86confcalloc(1, sizeof(XF86LoadRec));
    	    module->load_name = *el;
            ptr->mod_load_lst = (XF86LoadPtr)xf86addListItem(
                                (glp)ptr->mod_load_lst, (glp)module);
    	}
	xfree(elist);
    }

    /* Process list of font backends separately to include only required ones */
    elist = LoaderListDirs(fsubdirs, NULL);
    if (elist) {
	for (el = elist; *el; el++) {
	    XF86LoadPtr module;

    	    module = xf86confcalloc(1, sizeof(XF86LoadRec));
    	    module->load_name = *el;

            /* Add only those font backends which are referenced by fontpath */
            /* 'strstr(dFP,"/dir")' is meant as 'dFP =~ m(/dir\W)' */
    	    if (defaultFontPath && (
		(strcmp(*el, "freetype")  == 0 &&
		 strstr(defaultFontPath, "/TTF")) ||
    	        (strcmp(*el, "type1")  == 0 &&
		 strstr(defaultFontPath, "/Type1")))) 
	    	ptr->mod_load_lst = (XF86LoadPtr)xf86addListItem(
					(glp)ptr->mod_load_lst, (glp)module);
    	}
	xfree(elist);
    }

    return ptr;
}

static XF86ConfFilesPtr
configureFilesSection (void)
{
    parsePrologue (XF86ConfFilesPtr, XF86ConfFilesRec)

   if (xf86ModulePath)
       ptr->file_modulepath = strdup(xf86ModulePath);
   if (defaultFontPath)
       ptr->file_fontpath = strdup(defaultFontPath);
   
    return ptr;
}

static XF86ConfMonitorPtr
configureMonitorSection (int screennum)
{
    parsePrologue (XF86ConfMonitorPtr, XF86ConfMonitorRec)

    ptr->mon_identifier = xf86confmalloc(19);
    sprintf(ptr->mon_identifier, "Monitor%d", screennum);
    ptr->mon_vendor = strdup("Monitor Vendor");
    ptr->mon_modelname = strdup("Monitor Model");

    return ptr;
}

static XF86ConfMonitorPtr
configureDDCMonitorSection (int screennum)
{
    int i = 0;
    int len, mon_width, mon_height;
#define displaySizeMaxLen 80
    char displaySize_string[displaySizeMaxLen];
    int displaySizeLen;

    parsePrologue (XF86ConfMonitorPtr, XF86ConfMonitorRec)

    ptr->mon_identifier = xf86confmalloc(19);
    sprintf(ptr->mon_identifier, "Monitor%d", screennum);
    ptr->mon_vendor = strdup(ConfiguredMonitor->vendor.name);
    ptr->mon_modelname = xf86confmalloc(12);
    sprintf(ptr->mon_modelname, "%x", ConfiguredMonitor->vendor.prod_id);

    /* features in centimetres, we want millimetres */
    mon_width  = 10 * ConfiguredMonitor->features.hsize ;
    mon_height = 10 * ConfiguredMonitor->features.vsize ;

#ifdef CONFIGURE_DISPLAYSIZE
    ptr->mon_width  = mon_width;
    ptr->mon_height = mon_height;
#else
    if (mon_width && mon_height) {
      /* when values available add DisplaySize option AS A COMMENT */

      displaySizeLen = snprintf(displaySize_string, displaySizeMaxLen,
				"\t#DisplaySize\t%5d %5d\t# mm\n",
				mon_width, mon_height);

      if (displaySizeLen>0 && displaySizeLen<displaySizeMaxLen) {
	if (ptr->mon_comment) {
	  len = strlen(ptr->mon_comment);
	} else {
	  len = 0;
	}
	if ((ptr->mon_comment =
	     xf86confrealloc(ptr->mon_comment, 
			     len+strlen(displaySize_string)))) {
	  strcpy(ptr->mon_comment + len, displaySize_string);
	}
      }
    }
#endif /* def CONFIGURE_DISPLAYSIZE */

    for (i=0;i<4;i++) {
	switch (ConfiguredMonitor->det_mon[i].type) {
	    case DS_NAME:
		ptr->mon_modelname  = xf86confrealloc(ptr->mon_modelname, 
		  strlen((char*)(ConfiguredMonitor->det_mon[i].section.name))
		    + 1);
		strcpy(ptr->mon_modelname,
		       (char*)(ConfiguredMonitor->det_mon[i].section.name));
		break;
	    case DS_RANGES:
		ptr->mon_hsync[ptr->mon_n_hsync].lo =
		    ConfiguredMonitor->det_mon[i].section.ranges.min_h;
		ptr->mon_hsync[ptr->mon_n_hsync].hi =
		    ConfiguredMonitor->det_mon[i].section.ranges.max_h;
		ptr->mon_n_vrefresh = 1;
		ptr->mon_vrefresh[ptr->mon_n_hsync].lo =
		    ConfiguredMonitor->det_mon[i].section.ranges.min_v;
		ptr->mon_vrefresh[ptr->mon_n_hsync].hi =
		    ConfiguredMonitor->det_mon[i].section.ranges.max_v;
		ptr->mon_n_hsync++;
	    default:
		break;
	}
    }

    if (ConfiguredMonitor->features.dpms) {
      ptr->mon_option_lst = xf86addNewOption(ptr->mon_option_lst, xstrdup("DPMS"), NULL);
    }

    return ptr;
}

#if !defined(PATH_MAX)
# define PATH_MAX 1024
#endif

void
DoConfigure(void)
{
    int i,j, screennum = -1;
    char *home = NULL;
    char filename[PATH_MAX];
    char *addslash = "";
    XF86ConfigPtr xf86config = NULL;
    char **vlist, **vl;
    int *dev2screen;

    vlist = xf86DriverlistFromCompile();

    if (!vlist) {
	ErrorF("Missing output drivers.  Configuration failed.\n");
	goto bail;
    }

    ErrorF("List of video drivers:\n");
    for (vl = vlist; *vl; vl++)
	ErrorF("\t%s\n", *vl);

    /* Load all the drivers that were found. */
    xf86LoadModules(vlist, NULL);

    xfree(vlist);

    for (i = 0; i < xf86NumDrivers; i++) {
	xorgHWFlags flags;
	if (!xf86DriverList[i]->driverFunc
	    || !xf86DriverList[i]->driverFunc(NULL,
					      GET_REQUIRED_HW_INTERFACES,
					      &flags)
	    || NEED_IO_ENABLED(flags)) {
	    xorgHWAccess = TRUE;
	    break;
	}
    }
    /* Enable full I/O access */
    if (xorgHWAccess) {
	if(!xf86EnableIO())
	    /* oops, we have failed */
	    xorgHWAccess = FALSE;
    }

    /* Disable PCI devices */
    xf86ResourceBrokerInit();
    xf86AccessInit();
    xf86FindPrimaryDevice();
 
    /* Create XF86Config file structure */
    xf86config = calloc(1, sizeof(XF86ConfigRec));

    /* Call all of the probe functions, reporting the results. */
    for (CurrentDriver = 0;  CurrentDriver < xf86NumDrivers;  CurrentDriver++) {
	xorgHWFlags flags;
	Bool found_screen;
	DriverRec * const drv = xf86DriverList[CurrentDriver];

	if (!xorgHWAccess) {
	    if (!drv->driverFunc
		|| !drv->driverFunc( NULL, GET_REQUIRED_HW_INTERFACES, &flags )
		|| NEED_IO_ENABLED(flags)) 
		continue;
	}
	
	found_screen = xf86CallDriverProbe( drv, TRUE );
	if ( found_screen && drv->Identify ) {
	    (*drv->Identify)(0);
	}
    }

    if (nDevToConfig <= 0) {
	ErrorF("No devices to configure.  Configuration failed.\n");
	goto bail;
    }

    /* Add device, monitor and screen sections for detected devices */
    for (screennum = 0;  screennum < nDevToConfig;  screennum++) {
    	XF86ConfDevicePtr DevicePtr;
	XF86ConfMonitorPtr MonitorPtr;
	XF86ConfScreenPtr ScreenPtr;

	DevicePtr = configureDeviceSection(screennum);
    	xf86config->conf_device_lst = (XF86ConfDevicePtr)xf86addListItem(
			    (glp)xf86config->conf_device_lst, (glp)DevicePtr);
	MonitorPtr = configureMonitorSection(screennum);
    	xf86config->conf_monitor_lst = (XF86ConfMonitorPtr)xf86addListItem(
			    (glp)xf86config->conf_monitor_lst, (glp)MonitorPtr);
	ScreenPtr = configureScreenSection(screennum);
    	xf86config->conf_screen_lst = (XF86ConfScreenPtr)xf86addListItem(
			    (glp)xf86config->conf_screen_lst, (glp)ScreenPtr);
    }

    xf86config->conf_files = configureFilesSection();
    xf86config->conf_modules = configureModuleSection();
    xf86config->conf_flags = configureFlagsSection();
    xf86config->conf_videoadaptor_lst = NULL;
    xf86config->conf_modes_lst = NULL;
    xf86config->conf_vendor_lst = NULL;
    xf86config->conf_dri = NULL;
    xf86config->conf_input_lst = configureInputSection();
    xf86config->conf_layout_lst = configureLayoutSection();

    home = getenv("HOME");
    if ((home == NULL) || (home[0] == '\0')) {
    	home = "/";
    } else {
	/* Determine if trailing slash is present or needed */
	int l = strlen(home);

	if (home[l-1] != '/') {
	    addslash = "/";
	}
    }

    snprintf(filename, sizeof(filename), "%s%s" XF86CONFIGFILE ".new",
	     home, addslash);

    if (xf86writeConfigFile(filename, xf86config) == 0) {
	xf86Msg(X_ERROR, "Unable to write config file: \"%s\": %s\n",
		filename, strerror(errno));
	goto bail;
    }

    xf86DoConfigurePass1 = FALSE;
    /* Try to get DDC information filled in */
    xf86ConfigFile = filename;
    if (xf86HandleConfigFile(FALSE) != CONFIG_OK) {
	goto bail;
    }

    xf86DoConfigurePass1 = FALSE;
    
    dev2screen = xnfcalloc(1,xf86NumDrivers*sizeof(int));

    {
	Bool *driverProbed = xnfcalloc(1,xf86NumDrivers*sizeof(Bool));
	for (screennum = 0;  screennum < nDevToConfig;  screennum++) {
	    int k,l,n,oldNumScreens;

	    i = DevToConfig[screennum].iDriver;

	    if (driverProbed[i]) continue;
	    driverProbed[i] = TRUE;
	    
	    oldNumScreens = xf86NumScreens;

	    xf86CallDriverProbe( xf86DriverList[i], FALSE );

	    /* reorder */
	    k = screennum > 0 ? screennum : 1;
	    for (l = oldNumScreens; l < xf86NumScreens; l++) {
	        /* is screen primary? */
	        Bool primary = FALSE;
		for (n = 0; n<xf86Screens[l]->numEntities; n++) {
	            if (xf86IsEntityPrimary(xf86Screens[l]->entityList[n])) {
		        dev2screen[0] = l;
			primary = TRUE;
			break;
		    }
		}
		if (primary) continue;
		/* not primary: assign it to next device of same driver */
		/* 
		 * NOTE: we assume that devices in DevToConfig 
		 * and xf86Screens[] have the same order except
		 * for the primary device which always comes first.
		 */
		for (; k < nDevToConfig; k++) {
		    if (DevToConfig[k].iDriver == i) {
		        dev2screen[k++] = l;
			break;
		    }
		}
	    }
	}
	xfree(driverProbed);
    }
    

    if (nDevToConfig != xf86NumScreens) {
	ErrorF("Number of created screens does not match number of detected"
	       " devices.\n  Configuration failed.\n");
	goto bail;
    }

    xf86PostProbe();
    xf86EntityInit();

    for (j = 0; j < xf86NumScreens; j++) {
	xf86Screens[j]->scrnIndex = j;
    }

    xf86freeMonitorList(xf86config->conf_monitor_lst);
    xf86config->conf_monitor_lst = NULL;
    xf86freeScreenList(xf86config->conf_screen_lst);
    xf86config->conf_screen_lst = NULL;
    for (j = 0; j < xf86NumScreens; j++) {
	XF86ConfMonitorPtr MonitorPtr;
	XF86ConfScreenPtr ScreenPtr;

	ConfiguredMonitor = NULL;

	xf86EnableAccess(xf86Screens[dev2screen[j]]);
	if ((*xf86Screens[dev2screen[j]]->PreInit)(xf86Screens[dev2screen[j]], 
						   PROBE_DETECT) &&
	    ConfiguredMonitor) {
	    MonitorPtr = configureDDCMonitorSection(j);
	} else {
	    MonitorPtr = configureMonitorSection(j);
	}
	ScreenPtr = configureScreenSection(j);
	xf86config->conf_monitor_lst = (XF86ConfMonitorPtr)xf86addListItem(
		(glp)xf86config->conf_monitor_lst, (glp)MonitorPtr);
	xf86config->conf_screen_lst = (XF86ConfScreenPtr)xf86addListItem(
		(glp)xf86config->conf_screen_lst, (glp)ScreenPtr);
    }

    if (xf86writeConfigFile(filename, xf86config) == 0) {
	xf86Msg(X_ERROR, "Unable to write config file: \"%s\": %s\n",
		filename, strerror(errno));
	goto bail;
    }

    ErrorF("\n");

#ifdef __SCO__
    ErrorF("\n"__XSERVERNAME__
	   " is using the kernel event driver to access the mouse.\n"
	    "If you wish to use the internal "__XSERVERNAME__
	   " mouse drivers, please\n"
	    "edit the file and correct the Device.\n");
#else /* !__SCO__ */
    if (!foundMouse) {
	ErrorF("\n"__XSERVERNAME__" is not able to detect your mouse.\n"
		"Edit the file and correct the Device.\n");
    } else {
	ErrorF("\n"__XSERVERNAME__" detected your mouse at device %s.\n"
		"Please check your config if the mouse is still not\n"
		"operational, as by default "__XSERVERNAME__
	       " tries to autodetect\n"
		"the protocol.\n",DFLT_MOUSE_DEV);
    }
#endif /* !__SCO__ */

    if (xf86NumScreens > 1) {
	ErrorF("\n"__XSERVERNAME__
	       " has configured a multihead system, please check your config.\n");
    }

    ErrorF("\nYour %s file is %s\n\n", XF86CONFIGFILE ,filename);
    ErrorF("To test the server, run 'X -config %s'\n\n", filename);

bail:
    OsCleanup(TRUE);
    AbortDDX();
    fflush(stderr);
    exit(0);
}
