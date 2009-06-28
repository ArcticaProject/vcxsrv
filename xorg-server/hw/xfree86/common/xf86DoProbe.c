/*
 * Copyright (c) 1999-2002 by The XFree86 Project, Inc.
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
 * finish setting up the server
 * Load the driver modules and call their probe functions.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include "os.h"
#include "loaderProcs.h"
#include "xf86Config.h"
#include "xf86_OSlib.h"
#include "xf86.h"
#include "xf86Priv.h"

void
DoProbe()
{
    int i;
    Bool probeResult;
    Bool ioEnableFailed = FALSE;
    
    /* Find the list of video driver modules. */
    char **list = xf86DriverlistFromCompile();
    char **l;

    if (list) {
	ErrorF("List of video driver modules:\n");
	for (l = list; *l; l++)
	    ErrorF("\t%s\n", *l);
    } else {
	ErrorF("No video driver modules found\n");
    }

    /* Load all the drivers that were found. */
    xf86LoadModules(list, NULL);

    /* Disable PCI devices */
    xf86AccessInit();

    /* Call all of the probe functions, reporting the results. */
    for (i = 0; i < xf86NumDrivers; i++) {
	DriverRec * const drv = xf86DriverList[i];

	if (!xorgHWAccess) {
	    xorgHWFlags flags;
	    if (!drv->driverFunc
		|| !drv->driverFunc( NULL, GET_REQUIRED_HW_INTERFACES, &flags )
		|| NEED_IO_ENABLED(flags)) {
		if (ioEnableFailed)
		    continue;
		if (!xf86EnableIO()) {
		    ioEnableFailed = TRUE;
		    continue;
		}
		xorgHWAccess = TRUE;
	    }
	}
	    

	xf86MsgVerb(X_INFO, 3, "Probing in driver %s\n",  drv->driverName);

	probeResult = xf86CallDriverProbe( drv, TRUE );
	if (!probeResult) {
	    xf86ErrorF("Probe in driver `%s' returns FALSE\n",
		drv->driverName);
	} else {
	    xf86ErrorF("Probe in driver `%s' returns TRUE\n",
		drv->driverName);

	    /* If we have a result, then call driver's Identify function */
	    if (drv->Identify != NULL) {
		const int verbose = xf86SetVerbosity(1);
		(*drv->Identify)(0);
		xf86SetVerbosity(verbose);
	    }
	}
    }

    OsCleanup(TRUE);
    AbortDDX();
    fflush(stderr);
    exit(0);
}
