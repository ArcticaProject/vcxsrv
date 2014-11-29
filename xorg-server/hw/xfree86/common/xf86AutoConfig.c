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
#include "xf86platformBus.h"
#include "xf86pciBus.h"
#ifdef __sparc__
#include "xf86sbusBus.h"
#endif

#ifdef sun
#include <sys/visual_io.h>
#include <ctype.h>
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
    free(str);
}

static void
FreeList(const char ***list, int *lines)
{
    int i;

    for (i = 0; i < *lines; i++) {
        free((char *) ((*list)[i]));
    }
    free(*list);
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
        free(*p);
    }

    xf86MsgVerb(X_DEFAULT, 0,
                "Using default built-in configuration (%d lines)\n",
                builtinLines);

    xf86MsgVerb(X_DEFAULT, 3, "--- Start of built-in configuration ---\n");
    for (cp = builtinConfig; *cp; cp++)
        xf86ErrorFVerb(3, "\t%s", *cp);
    xf86MsgVerb(X_DEFAULT, 3, "--- End of built-in configuration ---\n");

    xf86initConfigFiles();
    xf86setBuiltinConfig(builtinConfig);
    ret = xf86HandleConfigFile(TRUE);
    FreeConfig();

    if (ret != CONFIG_OK)
        xf86Msg(X_ERROR, "Error parsing the built-in default configuration.\n");

    return ret == CONFIG_OK;
}

static void
listPossibleVideoDrivers(char *matches[], int nmatches)
{
    int i;

    for (i = 0; i < nmatches; i++) {
        matches[i] = NULL;
    }
    i = 0;

#ifdef XSERVER_PLATFORM_BUS
    i = xf86PlatformMatchDriver(matches, nmatches);
#endif
#ifdef sun
    /* Check for driver type based on /dev/fb type and if valid, use
       it instead of PCI bus probe results */
    if (xf86Info.consoleFd >= 0 && (i < (nmatches - 1))) {
        struct vis_identifier visid;
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
        }
        else {
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
    if (i < (nmatches - 1))
    {
        char *sbusDriver = sparcDriverName();

        if (sbusDriver)
            matches[i++] = xnfstrdup(sbusDriver);
    }
#endif
#ifdef XSERVER_LIBPCIACCESS
    if (i < (nmatches - 1))
        i += xf86PciMatchDriver(&matches[i], nmatches - i);
#endif

#if defined(__linux__)
    matches[i++] = xnfstrdup("modesetting");
#endif

#if !defined(sun)
    /* Fallback to platform default frame buffer driver */
    if (i < (nmatches - 1)) {
#if !defined(__linux__) && defined(__sparc__)
        matches[i++] = xnfstrdup("wsfb");
#else
        matches[i++] = xnfstrdup("fbdev");
#endif
    }
#endif                          /* !sun */

    /* Fallback to platform default hardware */
    if (i < (nmatches - 1)) {
#if defined(__i386__) || defined(__amd64__) || defined(__hurd__)
        matches[i++] = xnfstrdup("vesa");
#elif defined(__sparc__) && !defined(sun)
        matches[i++] = xnfstrdup("sunffb");
#endif
    }
}

/* copy a screen section and enter the desired driver
 * and insert it at i in the list of screens */
static Bool
copyScreen(confScreenPtr oscreen, GDevPtr odev, int i, char *driver)
{
    confScreenPtr nscreen;
    GDevPtr cptr = NULL;
    char *identifier;

    nscreen = malloc(sizeof(confScreenRec));
    if (!nscreen)
        return FALSE;
    memcpy(nscreen, oscreen, sizeof(confScreenRec));

    cptr = malloc(sizeof(GDevRec));
    if (!cptr) {
        free(nscreen);
        return FALSE;
    }
    memcpy(cptr, odev, sizeof(GDevRec));

    if (asprintf(&identifier, "Autoconfigured Video Device %s", driver)
        == -1) {
        free(cptr);
        free(nscreen);
        return FALSE;
    }
    cptr->driver = driver;
    cptr->identifier = identifier;

    xf86ConfigLayout.screens[i].screen = nscreen;

    /* now associate the new driver entry with the new screen entry */
    xf86ConfigLayout.screens[i].screen->device = cptr;
    cptr->myScreenSection = xf86ConfigLayout.screens[i].screen;

    return TRUE;
}

GDevPtr
autoConfigDevice(GDevPtr preconf_device)
{
    GDevPtr ptr = NULL;
    char *matches[20];          /* If we have more than 20 drivers we're in trouble */
    int num_matches = 0, num_screens = 0, i;
    screenLayoutPtr slp;

    if (!xf86configptr) {
        return NULL;
    }

    /* If there's a configured section with no driver chosen, use it */
    if (preconf_device) {
        ptr = preconf_device;
    }
    else {
        ptr = calloc(1, sizeof(GDevRec));
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
        /* get all possible video drivers and count them */
        listPossibleVideoDrivers(matches, 20);
        for (; matches[num_matches]; num_matches++) {
            xf86Msg(X_DEFAULT, "Matched %s as autoconfigured driver %d\n",
                    matches[num_matches], num_matches);
        }

        slp = xf86ConfigLayout.screens;
        if (slp) {
            /* count the number of screens and make space for
             * a new screen for each additional possible driver
             * minus one for the already existing first one
             * plus one for the terminating NULL */
            for (; slp[num_screens].screen; num_screens++);
            xf86ConfigLayout.screens = xnfcalloc(num_screens + num_matches,
                                                 sizeof(screenLayoutRec));
            xf86ConfigLayout.screens[0] = slp[0];

            /* do the first match and set that for the original first screen */
            ptr->driver = matches[0];
            if (!xf86ConfigLayout.screens[0].screen->device) {
                xf86ConfigLayout.screens[0].screen->device = ptr;
                ptr->myScreenSection = xf86ConfigLayout.screens[0].screen;
            }

            /* for each other driver found, copy the first screen, insert it
             * into the list of screens and set the driver */
            i = 0;
            while (i++ < num_matches) {
                if (!copyScreen(slp[0].screen, ptr, i, matches[i]))
                    return NULL;
            }

            /* shift the rest of the original screen list
             * to the end of the current screen list
             *
             * TODO Handle rest of multiple screen sections */
            for (i = 1; i < num_screens; i++) {
                xf86ConfigLayout.screens[i + num_matches] = slp[i];
            }
            xf86ConfigLayout.screens[num_screens + num_matches - 1].screen =
                NULL;
            free(slp);
        }
        else {
            /* layout does not have any screens, not much to do */
            ptr->driver = matches[0];
            for (i = 1; matches[i]; i++) {
                if (matches[i] != matches[0]) {
                    free(matches[i]);
                }
            }
        }
    }

    xf86Msg(X_DEFAULT, "Assigned the driver to the xf86ConfigLayout\n");

    return ptr;
}
