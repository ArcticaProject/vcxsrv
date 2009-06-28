/*
 * Copyright (c) 1997-2000 by The XFree86 Project, Inc.
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
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Resources.h"

#include "xf86Bus.h"

#define XF86_OS_PRIVS
#define NEED_OS_RAC_PROTOS
#include "xf86_OSproc.h"

#include "xf86RAC.h"

Bool isaSlotClaimed = FALSE;

/*
 * If the slot requested is already in use, return FALSE.
 * Otherwise, claim the slot for the screen requesting it.
 */

_X_EXPORT int
xf86ClaimIsaSlot(DriverPtr drvp, int chipset, GDevPtr dev, Bool active)
{
    EntityPtr p;
    BusAccPtr pbap = xf86BusAccInfo;
    int num;
    
    num = xf86AllocateEntity();
    p = xf86Entities[num];
    p->driver = drvp;
    p->chipset = chipset;
    p->busType = BUS_ISA;
    p->active = active;
    p->inUse = FALSE;
    xf86AddDevToEntity(num, dev);
    p->access = xnfcalloc(1,sizeof(EntityAccessRec));
    p->access->fallback = &AccessNULL;
    p->access->pAccess = &AccessNULL;
    p->busAcc = NULL;
    while (pbap) {
	if (pbap->type == BUS_ISA)
	    p->busAcc = pbap;
	pbap = pbap->next;
    }
    isaSlotClaimed = TRUE;
    return num;
}

/*
 * Get the list of ISA "slots" claimed by a screen
 *
 * Note: The ISA implementation here assumes that only one ISA "slot" type
 * can be claimed by any one screen.  That means a return value other than
 * 0 or 1 isn't useful.
 */
int
xf86GetIsaInfoForScreen(int scrnIndex)
{
    int num = 0;
    int i;
    EntityPtr p;
    
    for (i = 0; i < xf86Screens[scrnIndex]->numEntities; i++) {
	p = xf86Entities[xf86Screens[scrnIndex]->entityList[i]];
  	if (p->busType == BUS_ISA) {
  	    num++;
  	}
    }
    return num;
}

/*
 * Parse a BUS ID string, and return True if it is a ISA bus id.
 */

_X_EXPORT Bool
xf86ParseIsaBusString(const char *busID)
{
    /*
     * The format assumed to be "isa" or "isa:"
     */
    return (StringToBusType(busID,NULL) == BUS_ISA);
}


/*
 * xf86IsPrimaryIsa() -- return TRUE if primary device
 * is ISA.
 */
 
_X_EXPORT Bool
xf86IsPrimaryIsa(void)
{
    return ( primaryBus.type == BUS_ISA );
}

void
isaConvertRange2Host(resRange *pRange)
{
    return;
}
