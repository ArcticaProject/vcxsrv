/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86ShopwOpts.c,v 3.80 2003/10/08 14:58:27 dawes Exp $ */
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
 * Author:  Marcus Schaefer, ms@suse.de
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include "os.h"
#ifdef XFree86LOADER
#include "loaderProcs.h"
#endif
#include "xf86.h"
#include "xf86Config.h"
#include "xf86_OSlib.h"
#include "xf86Priv.h"
/* #include "xf86PciData.h" */
#define IN_XSERVER
#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"
#include "vbe.h"
#include "xf86DDC.h"
#if defined(__sparc__) && !defined(__OpenBSD__)
#include "xf86Bus.h"
#include "xf86Sbus.h"
#endif
#include "globals.h"

static const char* 
optionTypeToSting(OptionValueType type)
{
    switch (type) {
    case OPTV_NONE:
        return "";
    case OPTV_INTEGER:
        return "<int>";
    case OPTV_STRING:
        return "<str>";
    case OPTV_ANYSTR:
        return "<str>";
    case OPTV_REAL:
        return "<real>";
    case OPTV_BOOLEAN:
        return "<bool>";
    case OPTV_FREQ:
        return "<freq>";
    default:
        return "<undef>";
    }
}

void DoShowOptions (void) {
	int  i = 0;
	char **vlist  = 0;
	char *pSymbol = 0;
	XF86ModuleData *initData = 0;
	if (! (vlist = xf86DriverlistFromCompile())) {
		ErrorF("Missing output drivers\n");
		goto bail;
	}
	xf86LoadModules (vlist,0);
	xfree (vlist);
	for (i = 0; i < xf86NumDrivers; i++) {
		if (xf86DriverList[i]->AvailableOptions) {
			OptionInfoPtr pOption = (OptionInfoPtr)(*xf86DriverList[i]->AvailableOptions)(0,0);
			if (! pOption) {
				ErrorF ("(EE) Couldn't read option table for %s driver\n",
					xf86DriverList[i]->driverName
				);
				continue;                                                       
			}
			pSymbol = xalloc (
				strlen(xf86DriverList[i]->driverName) + strlen("ModuleData") + 1
			);
			strcpy (pSymbol, xf86DriverList[i]->driverName);
			strcat (pSymbol, "ModuleData");
			initData = LoaderSymbol (pSymbol);
			if (initData) {
				XF86ModuleVersionInfo *vers = initData->vers;
				ErrorF ("Driver[%d]:%s[%s] {\n",
					i,xf86DriverList[i]->driverName,vers->vendor
				);
				OptionInfoPtr p;
				for (p = pOption; p->name != NULL; p++) {
					const char *opttype = optionTypeToSting(p->type);
					char *optname = xalloc(strlen(p->name) + 2 + 1);
					if (!optname) {
						continue;                      
					}
					sprintf(optname, "%s", p->name);
					ErrorF ("\t%s:%s\n", optname,opttype);
				}
				ErrorF ("}\n");
			}
		}
	}
	bail:
	OsCleanup (TRUE);                             
	AbortDDX ();                                                           
	fflush (stderr);                        
	exit (0);
}
