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
#include "dirent.h"

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
static const char *deviceList[] = {
	"fbdev",
	"vesa",
	NULL
};

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

static const char *
videoPtrToDriverName(struct pci_device *dev)
{
    /*
     * things not handled yet:
     * cyrix/nsc.  should be merged into geode anyway.
     * xgi.
     */

    switch (dev->vendor_id)
    {
	case 0x1022:
		if (dev->device_id == 0x2081)
			return "geode";
		else
			return NULL;
	case 0x1142:		    return "apm";
	case 0xedd8:		    return "ark";
	case 0x1a03:		    return "ast";
	case 0x1002:		    return "ati";
	case 0x102c:		    return "chips";
	case 0x1013:		    return "cirrus";
	case 0x8086:
	    if ((dev->device_id == 0x00d1) || (dev->device_id == 0x7800))
		return "i740";
	    else return "intel";
	case 0x102b:		    return "mga";
	case 0x10c8:		    return "neomagic";
	case 0x105d:		    return "i128";
	case 0x10de: case 0x12d2:   return "nv";
	case 0x1163:		    return "rendition";
	case 0x5333:
	    switch (dev->device_id)
	    {
		case 0x88d0: case 0x88d1: case 0x88f0: case 0x8811:
		case 0x8812: case 0x8814: case 0x8901:
		    return "s3";
		case 0x5631: case 0x883d: case 0x8a01: case 0x8a10:
		case 0x8c01: case 0x8c03: case 0x8904: case 0x8a13:
		    return "s3virge";
		default:
		    return "savage";
	    }
	case 0x1039:		    return "sis";
	case 0x126f:		    return "siliconmotion";
	case 0x121a:
	    if (dev->device_id < 0x0003)
	        return "voodoo";
	    else
	        return "tdfx";
	case 0x3d3d:		    return "glint";
	case 0x1023:		    return "trident";
	case 0x100c:		    return "tseng";
	case 0x1106:		    return "openchrome";
	case 0x15ad:		    return "vmware";
	default: break;
    }
    return NULL;
}

Bool
xf86AutoConfig(void)
{
    const char **p;
    char buf[1024];
    const char *driver = NULL;
    ConfigStatus ret;

    driver = chooseVideoDriver();

    if (driver) {
	snprintf(buf, sizeof(buf), BUILTIN_DEVICE_SECTION_PRE,
		 driver, 0, driver);
	AppendToConfig(buf);
	ErrorF("New driver is \"%s\"\n", driver);
	buf[0] = '\t';
	AppendToConfig(BUILTIN_DEVICE_SECTION_POST);
	snprintf(buf, sizeof(buf), BUILTIN_SCREEN_SECTION,
		 driver, 0, driver, 0);
	AppendToConfig(buf);
    }

    for (p = deviceList; *p; p++) {
	snprintf(buf, sizeof(buf), BUILTIN_DEVICE_SECTION, *p, 0, *p);
	AppendToConfig(buf);
	snprintf(buf, sizeof(buf), BUILTIN_SCREEN_SECTION, *p, 0, *p, 0);
	AppendToConfig(buf);
    }

    AppendToConfig(BUILTIN_LAYOUT_SECTION_PRE);
    if (driver) {
	snprintf(buf, sizeof(buf), BUILTIN_LAYOUT_SCREEN_LINE, driver, 0);
	AppendToConfig(buf);
    }
    for (p = deviceList; *p; p++) {
	snprintf(buf, sizeof(buf), BUILTIN_LAYOUT_SCREEN_LINE, *p, 0);
	AppendToConfig(buf);
    }
    AppendToConfig(BUILTIN_LAYOUT_SECTION_POST);

    xf86MsgVerb(X_DEFAULT, 0,
		"Using default built-in configuration (%d lines)\n",
		builtinLines);

    xf86MsgVerb(X_DEFAULT, 3, "--- Start of built-in configuration ---\n");
    for (p = builtinConfig; *p; p++)
	xf86ErrorFVerb(3, "\t%s", *p);
    xf86MsgVerb(X_DEFAULT, 3, "--- End of built-in configuration ---\n");
    
    xf86setBuiltinConfig(builtinConfig);
    ret = xf86HandleConfigFile(TRUE);
    FreeConfig();

    if (ret != CONFIG_OK)
	xf86Msg(X_ERROR, "Error parsing the built-in default configuration.\n");

    return (ret == CONFIG_OK);
}

int 
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
        ptr = (GDevPtr)xalloc(sizeof(GDevRec));
        if (!ptr) {
            return NULL;
        }
        memset((GDevPtr)ptr, 0, sizeof(GDevRec));
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

char*
chooseVideoDriver(void)
{
    struct pci_device * info = NULL;
    struct pci_device_iterator *iter;
    char *chosen_driver = NULL;
    int i;
    char *matches[20]; /* If we have more than 20 drivers we're in trouble */
    
    for (i=0 ; i<20 ; i++)
        matches[i] = NULL;

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

    /* TODO Handle multiple drivers claiming to support the same PCI ID */
    if (matches[0]) {
        chosen_driver = matches[0];
    } else {
	if (info != NULL)
	    chosen_driver = videoPtrToDriverName(info);
	if (chosen_driver == NULL) {
#if defined  __i386__ || defined __amd64__ || defined __x86_64__ || defined __hurd__
	    chosen_driver = "vesa";
#elif defined __sparc__
	    chosen_driver = "sunffb";
#else
	    chosen_driver = "fbdev";
#endif
	}
    }

    xf86Msg(X_DEFAULT, "Matched %s for the autoconfigured driver\n", chosen_driver);

    i = 0;
    while (matches[i]) {
        if (matches[i] != chosen_driver) {
            xfree(matches[i]);
        }
        i++;
    }

    return chosen_driver;
}
