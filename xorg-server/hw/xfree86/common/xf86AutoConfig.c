/*
 * Copyright 2003 by David H. Dawes.
 * Copyright 2003 by X-Oz Technologies.
 * All rights reserved.
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
 * 
 * Author: David Dawes <dawes@XFree86.Org>.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"
#include "xf86Parser.h"
#include "xf86tokens.h"
#include "xf86Config.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#ifdef __sparc__
# include "xf86sbusBus.h"
#endif
#include "dirent.h"

#ifdef sun
# include <sys/visual_io.h>
# include <ctype.h>
#endif

/* Sections for the default built-in configuration. */

#define BUILTIN_DEVICE_NAME \
	"\"Builtin Default %s Device %d\""

#define BUILTIN_DEVICE_SECTION_PRE \
	"Section \"Device\"\n" \
	"\tIdentifier\t" BUILTIN_DEVICE_NAME "\n" \
	"\tDriver\t\"%s\"\n"

#define BUILTIN_DEVICE_SECTION_POST \
	"EndSection\n\n"

#define BUILTIN_DEVICE_SECTION \
	BUILTIN_DEVICE_SECTION_PRE \
	BUILTIN_DEVICE_SECTION_POST

#define BUILTIN_SCREEN_NAME \
	"\"Builtin Default %s Screen %d\""

#define BUILTIN_SCREEN_SECTION \
	"Section \"Screen\"\n" \
	"\tIdentifier\t" BUILTIN_SCREEN_NAME "\n" \
	"\tDevice\t" BUILTIN_DEVICE_NAME "\n" \
	"EndSection\n\n"

#define BUILTIN_LAYOUT_SECTION_PRE \
	"Section \"ServerLayout\"\n" \
	"\tIdentifier\t\"Builtin Default Layout\"\n"

#define BUILTIN_LAYOUT_SCREEN_LINE \
	"\tScreen\t" BUILTIN_SCREEN_NAME "\n"

#define BUILTIN_LAYOUT_SECTION_POST \
	"EndSection\n\n"

static const char **builtinConfig = NULL;
static int builtinLines = 0;

static void listPossibleVideoDrivers(char *matches[], int nmatches);

/*
 * A built-in config file is stored as an array of strings, with each string
 * representing a single line.  AppendToConfig() breaks up the string "s"
 * into lines, and appends those lines it to builtinConfig.
 */

static void
AppendToList(const char *s, const char ***list, int *lines)
{
    char *str, *newstr, *p;

    str = xnfstrdup(s);
    for (p = strtok(str, "\n"); p; p = strtok(NULL, "\n")) {
	(*lines)++;
	*list = xnfrealloc(*list, (*lines + 1) * sizeof(**list));
	newstr = xnfalloc(strlen(p) + 2);
	strcpy(newstr, p);
	strcat(newstr, "\n");
	(*list)[*lines - 1] = newstr;
	(*list)[*lines] = NULL;
    }
    xfree(str);
}

static void
FreeList(const char ***list, int *lines)
{
    int i;

    for (i = 0; i < *lines; i++) {
	if ((*list)[i])
	    xfree((*list)[i]);
    }
    xfree(*list);
    *list = NULL;
    *lines = 0;
}

static void
FreeConfig(void)
{
    FreeList(&builtinConfig, &builtinLines);
}

static void
AppendToConfig(const char *s)
{
    AppendToList(s, &builtinConfig, &builtinLines);
}

static int
videoPtrToDriverList(struct pci_device *dev,
		     char *returnList[], int returnListMax)
{
    int i;
    /* Add more entries here if we ever return more than 4 drivers for
       any device */
    char *driverList[5] = { NULL, NULL, NULL, NULL, NULL };

    switch (dev->vendor_id)
    {
	/* AMD Geode LX */
	case 0x1022:
	    if (dev->device_id == 0x2081)
		driverList[0] = "geode";
	    break;
	/* older Geode products acquired by AMD still carry an NSC vendor_id */
	case 0x100b:
	    if (dev->device_id == 0x0030) {
		/* NSC Geode GX2 specifically */
		driverList[0] = "geode";
		/* GX2 support started its life in the NSC tree and was later 
		   forked by AMD for GEODE so we keep it as a backup */
		driverList[1] = "nsc";
	    } else 
		/* other NSC variant e.g. 0x0104 (SC1400), 0x0504 (SCx200) */
		driverList[0] = "nsc";
	    break;
	/* Cyrix Geode GX1 */
	case 0x1078:
	    if (dev->device_id == 0x0104)
		driverList[0] = "cyrix";
	    break;
	case 0x1142:		    driverList[0] = "apm"; break;
	case 0xedd8:		    driverList[0] = "ark"; break;
	case 0x1a03:		    driverList[0] = "ast"; break;
	case 0x1002:		    driverList[0] = "ati"; break;
	case 0x102c:		    driverList[0] = "chips"; break;
	case 0x1013:		    driverList[0] = "cirrus"; break;
	case 0x3d3d:		    driverList[0] = "glint"; break;
	case 0x105d:		    driverList[0] = "i128"; break;
	case 0x8086:
	    if ((dev->device_id == 0x00d1) || (dev->device_id == 0x7800)) {
		driverList[0] = "i740";
            } else if (dev->device_id == 0x8108) {
                break; /* "hooray" for poulsbo */
	    } else {
		driverList[0] = "intel";
	    }
	    break;
	case 0x102b:		    driverList[0] = "mga";	break;
	case 0x10c8:		    driverList[0] = "neomagic"; break;
	case 0x10de: case 0x12d2:   driverList[0] = "nv";	break;
	case 0x1106:		    driverList[0] = "openchrome"; break;
	case 0x1163:		    driverList[0] = "rendition"; break;
	case 0x5333:
	    switch (dev->device_id)
	    {
		case 0x88d0: case 0x88d1: case 0x88f0: case 0x8811:
		case 0x8812: case 0x8814: case 0x8901:
		    driverList[0] = "s3"; break;
		case 0x5631: case 0x883d: case 0x8a01: case 0x8a10:
		case 0x8c01: case 0x8c03: case 0x8904: case 0x8a13:
		    driverList[0] = "s3virge"; break;
		default:
		    driverList[0] = "savage"; break;
	    }
	    break;
	case 0x1039:		    driverList[0] = "sis";	break;
	case 0x126f:		    driverList[0] = "siliconmotion"; break;
	case 0x121a:
	    if (dev->device_id < 0x0003)
	        driverList[0] = "voodoo";
	    else
	        driverList[0] = "tdfx";
	    break;
	case 0x1011:		    driverList[0] = "tga"; break;
	case 0x1023:		    driverList[0] = "trident"; break;
	case 0x100c:		    driverList[0] = "tseng"; break;
	case 0x80ee:		    driverList[0] = "vboxvideo"; break;
	case 0x15ad:		    driverList[0] = "vmware"; break;
	case 0x18ca:
	    if (dev->device_id == 0x47)
		driverList[0] = "xgixp";
	    else
		driverList[0] = "xgi";
	    break;
	default: break;
    }
    for (i = 0; (i < returnListMax) && (driverList[i] != NULL); i++) {
	returnList[i] = xnfstrdup(driverList[i]);
    }
    return i;	/* Number of entries added */
}

Bool
xf86AutoConfig(void)
{
    char *deviceList[20];
    char **p;
    const char **cp;
    char buf[1024];
    ConfigStatus ret;

    listPossibleVideoDrivers(deviceList, 20);

    for (p = deviceList; *p; p++) {
	snprintf(buf, sizeof(buf), BUILTIN_DEVICE_SECTION, *p, 0, *p);
	AppendToConfig(buf);
	snprintf(buf, sizeof(buf), BUILTIN_SCREEN_SECTION, *p, 0, *p, 0);
	AppendToConfig(buf);
    }

    AppendToConfig(BUILTIN_LAYOUT_SECTION_PRE);
    for (p = deviceList; *p; p++) {
	snprintf(buf, sizeof(buf), BUILTIN_LAYOUT_SCREEN_LINE, *p, 0);
	AppendToConfig(buf);
    }
    AppendToConfig(BUILTIN_LAYOUT_SECTION_POST);

    for (p = deviceList; *p; p++) {
	xfree(*p);
    }

    xf86MsgVerb(X_DEFAULT, 0,
		"Using default built-in configuration (%d lines)\n",
		builtinLines);

    xf86MsgVerb(X_DEFAULT, 3, "--- Start of built-in configuration ---\n");
    for (cp = builtinConfig; *cp; cp++)
	xf86ErrorFVerb(3, "\t%s", *cp);
    xf86MsgVerb(X_DEFAULT, 3, "--- End of built-in configuration ---\n");
    
    xf86setBuiltinConfig(builtinConfig);
    ret = xf86HandleConfigFile(TRUE);
    FreeConfig();

    if (ret != CONFIG_OK)
	xf86Msg(X_ERROR, "Error parsing the built-in default configuration.\n");

    return (ret == CONFIG_OK);
}

static int
xchomp(char *line)
{
    size_t len = 0;

    if (!line) {
        return 1;
    }

    len = strlen(line);
    if (line[len - 1] == '\n' && len > 0) {
        line[len - 1] = '\0';
    }
    return 0;
}

#ifdef __linux__
/* This function is used to provide a workaround for binary drivers that
 * don't export their PCI ID's properly. If distros don't end up using this
 * feature it can and should be removed because the symbol-based resolution
 * scheme should be the primary one */
static void
matchDriverFromFiles (char** matches, uint16_t match_vendor, uint16_t match_chip)
{
    DIR *idsdir;
    FILE *fp;
    struct dirent *direntry;
    char *line = NULL;
    size_t len;
    ssize_t read;
    char path_name[256], vendor_str[5], chip_str[5];
    uint16_t vendor, chip;
    int i, j;

    idsdir = opendir(PCI_TXT_IDS_PATH);
    if (!idsdir)
        return;

    xf86Msg(X_INFO, "Scanning %s directory for additional PCI ID's supported by the drivers\n", PCI_TXT_IDS_PATH);
    direntry = readdir(idsdir);
    /* Read the directory */
    while (direntry) {
        if (direntry->d_name[0] == '.') {
            direntry = readdir(idsdir);
            continue;
        }
        len = strlen(direntry->d_name);
        /* A tiny bit of sanity checking. We should probably do better */
        if (strncmp(&(direntry->d_name[len-4]), ".ids", 4) == 0) {
            /* We need the full path name to open the file */
            strncpy(path_name, PCI_TXT_IDS_PATH, 256);
            strncat(path_name, "/", 1);
            strncat(path_name, direntry->d_name, (256 - strlen(path_name) - 1));
            fp = fopen(path_name, "r");
            if (fp == NULL) {
                xf86Msg(X_ERROR, "Could not open %s for reading. Exiting.\n", path_name);
                goto end;
            }
            /* Read the file */
#ifdef __GLIBC__
            while ((read = getline(&line, &len, fp)) != -1) {
#else
            while ((line = fgetln(fp, &len)) != (char *)NULL) {
#endif /* __GLIBC __ */
                xchomp(line);
                if (isdigit(line[0])) {
                    strncpy(vendor_str, line, 4);
                    vendor_str[4] = '\0';
                    vendor = (int)strtol(vendor_str, NULL, 16);
                    if ((strlen(&line[4])) == 0) {
                        chip_str[0] = '\0';
                        chip = -1;
                    } else {
                        /* Handle trailing whitespace */
                        if (isspace(line[4])) {
                            chip_str[0] = '\0';
                            chip = -1;
                        } else {
                            /* Ok, it's a real ID */
                            strncpy(chip_str, &line[4], 4);
                            chip_str[4] = '\0';
                            chip = (int)strtol(chip_str, NULL, 16);
                        }
                    }
                    if (vendor == match_vendor && chip == match_chip ) {
                        i = 0;
                        while (matches[i]) {
                            i++;
                        }
                        matches[i] = (char*)xalloc(sizeof(char) * strlen(direntry->d_name) -  3);
                        if (!matches[i]) {
                            xf86Msg(X_ERROR, "Could not allocate space for the module name. Exiting.\n");
                            goto end;
                        }
                        /* hack off the .ids suffix. This should guard
                         * against other problems, but it will end up
                         * taking off anything after the first '.' */
                        for (j = 0; j < (strlen(direntry->d_name) - 3) ; j++) {
                            if (direntry->d_name[j] == '.') {
                                matches[i][j] = '\0';
                                break;
                            } else {
                                matches[i][j] = direntry->d_name[j];
                            }
                        }
                        xf86Msg(X_INFO, "Matched %s from file name %s\n", matches[i], direntry->d_name);
                    }
                } else {
                    /* TODO Handle driver overrides here */
                }
            }
            fclose(fp);
        }
        direntry = readdir(idsdir);
    }
 end:
    xfree(line);
    closedir(idsdir);
}
#endif /* __linux__ */

static void
listPossibleVideoDrivers(char *matches[], int nmatches)
{
    struct pci_device * info = NULL;
    struct pci_device_iterator *iter;
    int i;
    
    for (i = 0 ; i < nmatches ; i++) {
        matches[i] = NULL;
    }
    i = 0;

#ifdef sun
    /* Check for driver type based on /dev/fb type and if valid, use
       it instead of PCI bus probe results */
    if (xf86Info.consoleFd >= 0) {
	struct vis_identifier   visid;
	const char *cp;
	extern char xf86SolarisFbDev[PATH_MAX];
	int iret;

	SYSCALL(iret = ioctl(xf86Info.consoleFd, VIS_GETIDENTIFIER, &visid));
	if (iret < 0) {
	    int fbfd;

	    fbfd = open(xf86SolarisFbDev, O_RDONLY);
	    if (fbfd >= 0) {
		SYSCALL(iret = ioctl(fbfd, VIS_GETIDENTIFIER, &visid));
		close(fbfd);
	    }
	}

	if (iret < 0) {
	    xf86Msg(X_WARNING,
		    "could not get frame buffer identifier from %s\n",
		    xf86SolarisFbDev);
	} else {
	    xf86Msg(X_PROBED, "console driver: %s\n", visid.name);

	    /* Special case from before the general case was set */
	    if (strcmp(visid.name, "NVDAnvda") == 0) {
		matches[i++] = xnfstrdup("nvidia");
	    }

	    /* General case - split into vendor name (initial all-caps
	       prefix) & driver name (rest of the string). */
	    if (strcmp(visid.name, "SUNWtext") != 0) {
		for (cp = visid.name; (*cp != '\0') && isupper(*cp); cp++) {
		    /* find end of all uppercase vendor section */
		}
		if ((cp != visid.name) && (*cp != '\0')) {
		    char *driverName = xnfstrdup(cp);
		    char *vendorName = xnfstrdup(visid.name);
		    vendorName[cp - visid.name] = '\0';

		    matches[i++] = vendorName;
		    matches[i++] = driverName;
		}
	    }
	}
    }
#endif
#ifdef __sparc__
    {
	char *sbusDriver = sparcDriverName();
	if (sbusDriver)
	    matches[i++] = xnfstrdup(sbusDriver);
    }
#endif

    /* Find the primary device, and get some information about it. */
    iter = pci_slot_match_iterator_create(NULL);
    while ((info = pci_device_next(iter)) != NULL) {
	if (xf86IsPrimaryPci(info)) {
	    break;
	}
    }

    pci_iterator_destroy(iter);

    if (!info) {
	ErrorF("Primary device is not PCI\n");
    }
#ifdef __linux__
    else {
	matchDriverFromFiles(matches, info->vendor_id, info->device_id);
    }
#endif /* __linux__ */

    for (i = 0; (i < nmatches) && (matches[i]); i++) {
	/* find end of matches list */
    }

    if ((info != NULL) && (i < nmatches)) {
	i += videoPtrToDriverList(info, &(matches[i]), nmatches - i);
    }

    /* Fallback to platform default hardware */
    if (i < (nmatches - 1)) {
#if defined(__i386__) || defined(__amd64__) || defined(__hurd__)
	matches[i++] = xnfstrdup("vesa");
#elif defined(__sparc__) && !defined(sun)
	matches[i++] = xnfstrdup("sunffb");
#endif
    }

    /* Fallback to platform default frame buffer driver */
    if (i < (nmatches - 1)) {
#if !defined(__linux__) && defined(__sparc__)
	matches[i++] = xnfstrdup("wsfb");
#else
	matches[i++] = xnfstrdup("fbdev");
#endif
    }
}

static char*
chooseVideoDriver(void)
{
    char *chosen_driver = NULL;
    int i;
    char *matches[20]; /* If we have more than 20 drivers we're in trouble */

    listPossibleVideoDrivers(matches, 20);

    /* TODO Handle multiple drivers claiming to support the same PCI ID */
    chosen_driver = matches[0];

    xf86Msg(X_DEFAULT, "Matched %s for the autoconfigured driver\n",
	    chosen_driver);

    for (i = 0; matches[i] ; i++) {
        if (matches[i] != chosen_driver) {
            xfree(matches[i]);
        }
    }

    return chosen_driver;
}

GDevPtr
autoConfigDevice(GDevPtr preconf_device)
{
    GDevPtr ptr = NULL;

    if (!xf86configptr) {
        return NULL;
    }

    /* If there's a configured section with no driver chosen, use it */
    if (preconf_device) {
        ptr = preconf_device;
    } else {
        ptr = xcalloc(1, sizeof(GDevRec));
        if (!ptr) {
            return NULL;
        }
        ptr->chipID = -1;
        ptr->chipRev = -1;
        ptr->irq = -1;

        ptr->active = TRUE;
        ptr->claimed = FALSE;
        ptr->identifier = "Autoconfigured Video Device";
        ptr->driver = NULL;
    }
    if (!ptr->driver) {
        ptr->driver = chooseVideoDriver();
    }

    /* TODO Handle multiple screen sections */
    if (xf86ConfigLayout.screens && !xf86ConfigLayout.screens->screen->device) {
        xf86ConfigLayout.screens->screen->device = ptr;
        ptr->myScreenSection = xf86ConfigLayout.screens->screen;
    }
    xf86Msg(X_DEFAULT, "Assigned the driver to the xf86ConfigLayout\n");

    return ptr;
}
