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
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

/* Bus-specific headers */

#include "xf86Bus.h"

#define XF86_OS_PRIVS
#include "xf86_OSproc.h"
#include "xf86VGAarbiter.h"

#include "Pci.h"

/* Entity data */
EntityPtr *xf86Entities = NULL;	/* Bus slots claimed by drivers */
int xf86NumEntities = 0;
static int xf86EntityPrivateCount = 0;

BusRec primaryBus = { BUS_NONE, { 0 } };

static Bool xf86ResAccessEnter = FALSE;

static Bool doFramebufferMode = FALSE;

/*
 * Call the bus probes relevant to the architecture.
 *
 * The only one available so far is for PCI and SBUS.
 */

void
xf86BusProbe(void)
{
    xf86PciProbe();
#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
    xf86SbusProbe();
#endif
}

/*
 * Determine what bus type the busID string represents.  The start of the
 * bus-dependent part of the string is returned as retID.
 */

BusType
StringToBusType(const char* busID, const char **retID)
{
    char *p, *s;
    BusType ret = BUS_NONE;

    /* If no type field, Default to PCI */
    if (isdigit(busID[0])) {
	if (retID)
	    *retID = busID;
	return BUS_PCI;
    }

    s = xstrdup(busID);
    p = strtok(s, ":");
    if (p == NULL || *p == 0) {
	xfree(s);
	return BUS_NONE;
    }
    if (!xf86NameCmp(p, "pci") || !xf86NameCmp(p, "agp"))
	ret = BUS_PCI; 
    if (!xf86NameCmp(p, "sbus"))
	ret = BUS_SBUS;
    if (ret != BUS_NONE)
	if (retID)
	    *retID = busID + strlen(p) + 1;
    xfree(s);
    return ret;
}

/*
 * Entity related code.
 */

void
xf86EntityInit(void)
{
    int i;
    
    for (i = 0; i < xf86NumEntities; i++)
	if (xf86Entities[i]->entityInit) {
	    xf86Entities[i]->entityInit(i,xf86Entities[i]->private);
	}
}

int
xf86AllocateEntity(void)
{
    xf86NumEntities++;
    xf86Entities = xnfrealloc(xf86Entities,
			      sizeof(EntityPtr) * xf86NumEntities);
    xf86Entities[xf86NumEntities - 1] = xnfcalloc(1,sizeof(EntityRec));
    xf86Entities[xf86NumEntities - 1]->entityPrivates =
               xnfcalloc(sizeof(DevUnion) * xf86EntityPrivateCount, 1);
    return (xf86NumEntities - 1);
}

static void
EntityEnter(void)
{
    int i;
    
    for (i = 0; i < xf86NumEntities; i++)
	if (xf86Entities[i]->entityEnter) {
	    xf86Entities[i]->entityEnter(i,xf86Entities[i]->private);
	}
}

static void
EntityLeave(void)
{
    int i;

    for (i = 0; i < xf86NumEntities; i++)
	if (xf86Entities[i]->entityLeave) {
	    xf86Entities[i]->entityLeave(i,xf86Entities[i]->private);
	}
}

Bool
xf86IsEntityPrimary(int entityIndex)
{
    EntityPtr pEnt = xf86Entities[entityIndex];
    
    if (primaryBus.type != pEnt->bus.type) return FALSE;

    switch (pEnt->bus.type) {
    case BUS_PCI:
	return (pEnt->bus.id.pci == primaryBus.id.pci);
    case BUS_SBUS:
	return (pEnt->bus.id.sbus.fbNum == primaryBus.id.sbus.fbNum);
    default:
	return FALSE;
    }
}
	
Bool
xf86SetEntityFuncs(int entityIndex, EntityProc init, EntityProc enter,
		   EntityProc leave, pointer private)
{
    if (entityIndex >= xf86NumEntities)
	return FALSE;
    xf86Entities[entityIndex]->entityInit = init;
    xf86Entities[entityIndex]->entityEnter = enter;
    xf86Entities[entityIndex]->entityLeave = leave;
    xf86Entities[entityIndex]->private = private;
    return TRUE;
}

Bool
xf86DriverHasEntities(DriverPtr drvp)
{
    int i;
    for (i = 0; i < xf86NumEntities; i++) {
	if (xf86Entities[i]->driver == drvp) 
	    return TRUE;
    }
    return FALSE;
}

void
xf86AddEntityToScreen(ScrnInfoPtr pScrn, int entityIndex)
{
    if (entityIndex == -1)
	return;
    if (xf86Entities[entityIndex]->inUse &&
	!(xf86Entities[entityIndex]->entityProp & IS_SHARED_ACCEL)) {
	ErrorF("Requested Entity already in use!\n");
	return;
    }

    pScrn->numEntities++;
    pScrn->entityList = xnfrealloc(pScrn->entityList,
				    pScrn->numEntities * sizeof(int));
    pScrn->entityList[pScrn->numEntities - 1] = entityIndex;
    xf86Entities[entityIndex]->inUse = TRUE;
    pScrn->entityInstanceList = xnfrealloc(pScrn->entityInstanceList,
				    pScrn->numEntities * sizeof(int));
    pScrn->entityInstanceList[pScrn->numEntities - 1] = 0;
    pScrn->domainIOBase = xf86Entities[entityIndex]->domainIO;
}

void
xf86SetEntityInstanceForScreen(ScrnInfoPtr pScrn, int entityIndex, int instance)
{
    int i;

    if (entityIndex == -1 || entityIndex >= xf86NumEntities)
	return;

    for (i = 0; i < pScrn->numEntities; i++) {
	if (pScrn->entityList[i] == entityIndex) {
	    pScrn->entityInstanceList[i] = instance;
	    break;
	}
    }
}

/*
 * XXX  This needs to be updated for the case where a single entity may have
 * instances associated with more than one screen.
 */
ScrnInfoPtr
xf86FindScreenForEntity(int entityIndex)
{
    int i,j;

    if (entityIndex == -1) return NULL;
    
    if (xf86Screens) {
	for (i = 0; i < xf86NumScreens; i++) {
	    for (j = 0; j < xf86Screens[i]->numEntities; j++) {
		if ( xf86Screens[i]->entityList[j] == entityIndex )
		    return (xf86Screens[i]);
	    }
	}
    }
    return NULL;
}

void
xf86RemoveEntityFromScreen(ScrnInfoPtr pScrn, int entityIndex)
{
    int i;
    
    for (i = 0; i < pScrn->numEntities; i++) {
	if (pScrn->entityList[i] == entityIndex) {
	    for (i++; i < pScrn->numEntities; i++)
		pScrn->entityList[i-1] = pScrn->entityList[i];
	    pScrn->numEntities--;
	    xf86Entities[entityIndex]->inUse = FALSE;
	    break;
	}
    }
}

/*
 * xf86ClearEntitiesForScreen() - called when a screen is deleted
 * to mark it's entities unused. Called by xf86DeleteScreen().
 */
void
xf86ClearEntityListForScreen(int scrnIndex)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    int i, entityIndex;
    
    if (pScrn->entityList == NULL || pScrn->numEntities == 0) return;
	
    for (i = 0; i < pScrn->numEntities; i++) {
	entityIndex = pScrn->entityList[i];
	xf86Entities[entityIndex]->inUse = FALSE;
	/* disable resource: call the disable function */
    }
    xfree(pScrn->entityList);
    xfree(pScrn->entityInstanceList);
    pScrn->entityList = NULL;
    pScrn->entityInstanceList = NULL;
}

/*
 * Add an extra device section (GDevPtr) to an entity.
 */

void
xf86AddDevToEntity(int entityIndex, GDevPtr dev)
{
    EntityPtr pEnt;
    
    if (entityIndex >= xf86NumEntities)
	return;
    
    pEnt = xf86Entities[entityIndex];
    pEnt->numInstances++;
    pEnt->devices = xnfrealloc(pEnt->devices,
				pEnt->numInstances * sizeof(GDevPtr));
    pEnt->devices[pEnt->numInstances - 1] = dev;
    dev->claimed = TRUE;
}

/*
 * xf86GetEntityInfo() -- This function hands information from the
 * EntityRec struct to the drivers. The EntityRec structure itself
 * remains invisible to the driver.
 */
EntityInfoPtr
xf86GetEntityInfo(int entityIndex)
{
    EntityInfoPtr pEnt;
    int i;
    
    if (entityIndex == -1)
	return NULL;

    if (entityIndex >= xf86NumEntities)
	return NULL;
    
    pEnt = xnfcalloc(1,sizeof(EntityInfoRec));
    pEnt->index = entityIndex;
    pEnt->location = xf86Entities[entityIndex]->bus;
    pEnt->active = xf86Entities[entityIndex]->active;
    pEnt->chipset = xf86Entities[entityIndex]->chipset;
    pEnt->driver = xf86Entities[entityIndex]->driver;
    if ( (xf86Entities[entityIndex]->devices) &&
         (xf86Entities[entityIndex]->devices[0]) ) {
	for (i = 0; i < xf86Entities[entityIndex]->numInstances; i++)
	    if (xf86Entities[entityIndex]->devices[i]->screen == 0)
	        break;
	pEnt->device = xf86Entities[entityIndex]->devices[i];
    } else
	pEnt->device = NULL;
    
    return pEnt;
}

int
xf86GetNumEntityInstances(int entityIndex)
{
    if (entityIndex >= xf86NumEntities)
	return -1;
    
    return xf86Entities[entityIndex]->numInstances;
}

GDevPtr
xf86GetDevFromEntity(int entityIndex, int instance)
{
    int i;
  
    /* We might not use AddDevtoEntity */
    if ( (!xf86Entities[entityIndex]->devices) ||
         (!xf86Entities[entityIndex]->devices[0]) ) 
	return NULL;

    if (entityIndex >= xf86NumEntities ||
	instance >= xf86Entities[entityIndex]->numInstances)
	return NULL;
    
    for (i = 0; i < xf86Entities[entityIndex]->numInstances; i++)
	if (xf86Entities[entityIndex]->devices[i]->screen == instance)
	    break;
    return xf86Entities[entityIndex]->devices[i];
}

/*
 * xf86AccessInit() - set up everything needed for access control
 * called only once on first server generation.
 */
void
xf86AccessInit(void)
{
    xf86ResAccessEnter = TRUE;
}

/*
 * xf86AccessEnter() -- gets called to save the text mode VGA IO 
 * resources when reentering the server after a VT switch.
 */
void
xf86AccessEnter(void)
{
    if (xf86ResAccessEnter) 
	return;

    /*
     * on enter we simply disable routing of special resources
     * to any bus and let the RAC code to "open" the right bridges.
     */
    EntityEnter();
    xf86EnterServerState(SETUP);
    xf86ResAccessEnter = TRUE;
}

/*
 * xf86AccessLeave() -- prepares access for and calls the
 * entityLeave() functions.
 * xf86AccessLeaveState() --- gets called to restore the
 * access to the VGA IO resources when switching VT or on
 * server exit.
 * This was split to call xf86AccessLeaveState() from
 * ddxGiveUp().
 */
void
xf86AccessLeave(void)
{
    if (!xf86ResAccessEnter)
	return;
    EntityLeave();
}

/*
 * xf86EnableAccess() -- enable access to controlled resources.
 * To reduce latency when switching access the ScrnInfoRec has
 * a linked list of the EntityAccPtr of all screen entities.
 */
/*
 * switching access needs to be done in te following oder:
 * disable
 * 1. disable old entity
 * 2. reroute bus
 * 3. enable new entity
 * Otherwise resources needed for access control might be shadowed
 * by other resources!
 */

void
xf86EnableAccess(ScrnInfoPtr pScrn)
{
    DebugF("Enable access %i\n",pScrn->scrnIndex);

    return;
}

/*
 * xf86EnterServerState() -- set state the server is in.
 */

typedef enum { TRI_UNSET, TRI_TRUE, TRI_FALSE } TriState;

static void
SetSIGIOForState(xf86State state)
{
    static int sigio_state;
    static TriState sigio_blocked = TRI_UNSET;

    if ((state == SETUP) && (sigio_blocked != TRI_TRUE)) {
        sigio_state = xf86BlockSIGIO();
	sigio_blocked = TRI_TRUE;
    } else if ((state == OPERATING) && (sigio_blocked != TRI_UNSET)) {
        xf86UnblockSIGIO(sigio_state);
        sigio_blocked = TRI_FALSE;
    }
}

void
xf86EnterServerState(xf86State state)
{
    /* 
     * This is a good place to block SIGIO during SETUP state.
     * SIGIO should be blocked in SETUP state otherwise (u)sleep()
     * might get interrupted early. 
     * We take care not to call xf86BlockSIGIO() twice. 
     */
    SetSIGIOForState(state);
    if (state == SETUP)
	DebugF("Entering SETUP state\n");
    else
	DebugF("Entering OPERATING state\n");

    return;
}

/*
 * xf86PostProbe() -- Allocate all non conflicting resources
 * This function gets called by xf86Init().
 */
void
xf86PostProbe(void)
{
    if (fbSlotClaimed) {
        if (pciSlotClaimed
#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
	    || sbusSlotClaimed
#endif
	    ) { 
	    FatalError("Cannot run in framebuffer mode. Please specify busIDs "
		       "       for all framebuffer devices\n");
	    return;
	} else  {
	    xf86Msg(X_INFO,"Running in FRAMEBUFFER Mode\n");
	    doFramebufferMode = TRUE;

	    return;
	}
    }
}

void
xf86PostScreenInit(void)
{
    if (doFramebufferMode) {
	SetSIGIOForState(OPERATING);
	return;
    }

    xf86VGAarbiterWrapFunctions();

    DebugF("PostScreenInit  generation: %i\n",serverGeneration);
    xf86EnterServerState(OPERATING);
}

/*
 * xf86FindPrimaryDevice() - Find the display device which
 * was active when the server was started.
 */
void
xf86FindPrimaryDevice(void)
{
    if (primaryBus.type != BUS_NONE) {
	char *bus;
	char loc[16];

	switch (primaryBus.type) {
	case BUS_PCI:
	    bus = "PCI";
	    snprintf(loc, sizeof(loc), " %2.2x@%2.2x:%2.2x:%1.1x",
		     primaryBus.id.pci->bus,
		     primaryBus.id.pci->domain,
		     primaryBus.id.pci->dev,
		     primaryBus.id.pci->func);
	    break;
	case BUS_SBUS:
	    bus = "SBUS";
	    snprintf(loc, sizeof(loc), " %2.2x", primaryBus.id.sbus.fbNum);
	    break;
	default:
	    bus = "";
	    loc[0] = '\0';
	}

	xf86MsgVerb(X_INFO, 2, "Primary Device is: %s%s\n",bus,loc);
    }
}

int
xf86GetLastScrnFlag(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        return(xf86Entities[entityIndex]->lastScrnFlag);
    } else {
        return -1;
    }
}

void
xf86SetLastScrnFlag(int entityIndex, int scrnIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->lastScrnFlag = scrnIndex;
    }
}

Bool
xf86IsEntityShared(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        if(xf86Entities[entityIndex]->entityProp & IS_SHARED_ACCEL) {
	    return TRUE;
	}
    }
    return FALSE;
}

void
xf86SetEntityShared(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->entityProp |= IS_SHARED_ACCEL;
    }
}

Bool
xf86IsEntitySharable(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        if(xf86Entities[entityIndex]->entityProp & ACCEL_IS_SHARABLE) {
	    return TRUE;
	}
    }
    return FALSE;
}

void
xf86SetEntitySharable(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->entityProp |= ACCEL_IS_SHARABLE;
    }
}

Bool
xf86IsPrimInitDone(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        if(xf86Entities[entityIndex]->entityProp & SA_PRIM_INIT_DONE) {
	    return TRUE;
	}
    }
    return FALSE;
}

void
xf86SetPrimInitDone(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->entityProp |= SA_PRIM_INIT_DONE;
    }
}

void
xf86ClearPrimInitDone(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->entityProp &= ~SA_PRIM_INIT_DONE;
    }
}


/*
 * Allocate a private in the entities.
 */

int
xf86AllocateEntityPrivateIndex(void)
{
    int idx, i;
    EntityPtr pEnt;
    DevUnion *nprivs;

    idx = xf86EntityPrivateCount++;
    for (i = 0; i < xf86NumEntities; i++) {
	pEnt = xf86Entities[i];
	nprivs = xnfrealloc(pEnt->entityPrivates,
			    xf86EntityPrivateCount * sizeof(DevUnion));
	/* Zero the new private */
	bzero(&nprivs[idx], sizeof(DevUnion));
	pEnt->entityPrivates = nprivs;
    }
    return idx;
}

DevUnion *
xf86GetEntityPrivate(int entityIndex, int privIndex)
{
    if (entityIndex >= xf86NumEntities || privIndex >= xf86EntityPrivateCount)
	return NULL;

    return &(xf86Entities[entityIndex]->entityPrivates[privIndex]);
}

