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
#include "xf86Resources.h"

/* Bus-specific headers */

#include "xf86Bus.h"

#define XF86_OS_PRIVS
#define NEED_OS_RAC_PROTOS
#include "xf86_OSproc.h"

#include "xf86RAC.h"
#include "Pci.h"

/* Entity data */
EntityPtr *xf86Entities = NULL;	/* Bus slots claimed by drivers */
int xf86NumEntities = 0;
static int xf86EntityPrivateCount = 0;
BusAccPtr xf86BusAccInfo = NULL;

static void
noopEnableDisable(void *arg) { }

xf86AccessRec AccessNULL = { noopEnableDisable, noopEnableDisable, NULL };

xf86CurrentAccessRec xf86CurrentAccess = {NULL,NULL};

BusRec primaryBus = { BUS_NONE, {{0}}};

static Bool xf86ResAccessEnter = FALSE;


/* resource lists */
resPtr Acc = NULL;

/* predefined special resources */
_X_EXPORT resRange resVgaExclusive[] = {_VGA_EXCLUSIVE, _END};
_X_EXPORT resRange resVgaShared[] = {_VGA_SHARED, _END};
_X_EXPORT resRange resVgaMemShared[] = {_VGA_SHARED_MEM,_END};
_X_EXPORT resRange resVgaIoShared[] = {_VGA_SHARED_IO,_END};
_X_EXPORT resRange resVgaUnusedExclusive[] = {_VGA_EXCLUSIVE_UNUSED, _END};
_X_EXPORT resRange resVgaUnusedShared[] = {_VGA_SHARED_UNUSED, _END};
_X_EXPORT resRange resVgaSparseExclusive[] = {_VGA_EXCLUSIVE_SPARSE, _END};
_X_EXPORT resRange resVgaSparseShared[] = {_VGA_SHARED_SPARSE, _END};
_X_EXPORT resRange res8514Exclusive[] = {_8514_EXCLUSIVE, _END};
_X_EXPORT resRange res8514Shared[] = {_8514_SHARED, _END};

/* Flag: do we need RAC ? */
static Bool needRAC = FALSE;
static Bool doFramebufferMode = FALSE;

/* state change notification callback list */
static StateChangeNotificationPtr StateChangeNotificationList;
static void notifyStateChange(xf86NotifyState state);

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
    xf86AccessPtr pacc;
    
    for (i = 0; i < xf86NumEntities; i++)
	if (xf86Entities[i]->entityInit) {
	    if (xf86Entities[i]->access->busAcc)
		((BusAccPtr)xf86Entities[i]->access->busAcc)->set_f
		    (xf86Entities[i]->access->busAcc);
	    pacc = xf86Entities[i]->access->fallback;
	    pacc->AccessEnable(pacc->arg);
	    xf86Entities[i]->entityInit(i,xf86Entities[i]->private);
	    pacc->AccessDisable(pacc->arg);
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
    xf86AccessPtr pacc;
    
    for (i = 0; i < xf86NumEntities; i++)
	if (xf86Entities[i]->entityEnter) {
	    if (xf86Entities[i]->access->busAcc)
		((BusAccPtr)xf86Entities[i]->access->busAcc)->set_f
		    (xf86Entities[i]->access->busAcc);
	    pacc = xf86Entities[i]->access->fallback;
	    pacc->AccessEnable(pacc->arg);
	    xf86Entities[i]->entityEnter(i,xf86Entities[i]->private);
	    pacc->AccessDisable(pacc->arg);
	}
}

static void
EntityLeave(void)
{
    int i;
    xf86AccessPtr pacc;

    for (i = 0; i < xf86NumEntities; i++)
	if (xf86Entities[i]->entityLeave) {
	    if (xf86Entities[i]->access->busAcc)
		((BusAccPtr)xf86Entities[i]->access->busAcc)->set_f
		    (xf86Entities[i]->access->busAcc);
	    pacc = xf86Entities[i]->access->fallback;
	    pacc->AccessEnable(pacc->arg);
	    xf86Entities[i]->entityLeave(i,xf86Entities[i]->private);
	    pacc->AccessDisable(pacc->arg);
	}
}

_X_EXPORT Bool
xf86IsEntityPrimary(int entityIndex)
{
    EntityPtr pEnt = xf86Entities[entityIndex];
    
    if (primaryBus.type != pEnt->busType) return FALSE;

    switch (pEnt->busType) {
    case BUS_PCI:
	return (pEnt->bus.id.pci == primaryBus.id.pci);
    case BUS_SBUS:
	return (pEnt->sbusBusId.fbNum == primaryBus.id.sbus.fbNum);
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

_X_EXPORT void
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
    xf86Entities[entityIndex]->access->next = pScrn->access;
    pScrn->access = xf86Entities[entityIndex]->access;
    xf86Entities[entityIndex]->inUse = TRUE;
    pScrn->entityInstanceList = xnfrealloc(pScrn->entityInstanceList,
				    pScrn->numEntities * sizeof(int));
    pScrn->entityInstanceList[pScrn->numEntities - 1] = 0;
    pScrn->domainIOBase = xf86Entities[entityIndex]->domainIO;
}

_X_EXPORT void
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
_X_EXPORT ScrnInfoPtr
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

_X_EXPORT void
xf86RemoveEntityFromScreen(ScrnInfoPtr pScrn, int entityIndex)
{
    int i;
    EntityAccessPtr *ptr = (EntityAccessPtr *)&pScrn->access;
    EntityAccessPtr peacc;
    
    for (i = 0; i < pScrn->numEntities; i++) {
	if (pScrn->entityList[i] == entityIndex) {
	    peacc = xf86Entities[pScrn->entityList[i]]->access;
	    (*ptr) = peacc->next;
	    /* disable entity: call disable func */
	    if (peacc->pAccess)
		peacc->pAccess->AccessDisable(peacc->pAccess->arg);
	    /* also disable fallback - just in case */
	    if (peacc->fallback)
		peacc->fallback->AccessDisable(peacc->fallback->arg);
	    for (i++; i < pScrn->numEntities; i++)
		pScrn->entityList[i-1] = pScrn->entityList[i];
	    pScrn->numEntities--;
	    xf86Entities[entityIndex]->inUse = FALSE;
	    break;
	}
	ptr = &(xf86Entities[pScrn->entityList[i]]->access->next);
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
    EntityAccessPtr peacc;
    int i, entityIndex;
    
    if (pScrn->entityList == NULL || pScrn->numEntities == 0) return;
	
    for (i = 0; i < pScrn->numEntities; i++) {
	entityIndex = pScrn->entityList[i];
	xf86Entities[entityIndex]->inUse = FALSE;
	/* disable resource: call the disable function */
	peacc = xf86Entities[entityIndex]->access;
	if (peacc->pAccess)
	    peacc->pAccess->AccessDisable(peacc->pAccess->arg);
	/* and the fallback function */
	if (peacc->fallback)
	    peacc->fallback->AccessDisable(peacc->fallback->arg);
	/* shared resources are only needed when entity is active: remove */
	xf86DeallocateResourcesForEntity(entityIndex, ResShared);
    }
    xfree(pScrn->entityList);
    xfree(pScrn->entityInstanceList);
    if (pScrn->CurrentAccess->pIoAccess == (EntityAccessPtr)pScrn->access)
	pScrn->CurrentAccess->pIoAccess = NULL;
    if (pScrn->CurrentAccess->pMemAccess == (EntityAccessPtr)pScrn->access)
	pScrn->CurrentAccess->pMemAccess = NULL;
    pScrn->entityList = NULL;
    pScrn->entityInstanceList = NULL;
}

void
xf86DeallocateResourcesForEntity(int entityIndex, unsigned long type)
{
    resPtr *pprev_next = &Acc;
    resPtr res = Acc;

    while (res) {
	if (res->entityIndex == entityIndex &&
	    (type & ResAccMask & res->res_type))
	{
	    (*pprev_next) = res->next;
	    xfree(res);
	} else 
	    pprev_next = &(res->next);
	res = (*pprev_next);
    }
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
_X_EXPORT EntityInfoPtr
xf86GetEntityInfo(int entityIndex)
{
    EntityInfoPtr pEnt;
    int i;
    
    if (entityIndex >= xf86NumEntities)
	return NULL;
    
    pEnt = xnfcalloc(1,sizeof(EntityInfoRec));
    pEnt->index = entityIndex;
    pEnt->location = xf86Entities[entityIndex]->bus;
    pEnt->active = xf86Entities[entityIndex]->active;
    pEnt->chipset = xf86Entities[entityIndex]->chipset;
    pEnt->resources = xf86Entities[entityIndex]->resources;
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

_X_EXPORT int
xf86GetNumEntityInstances(int entityIndex)
{
    if (entityIndex >= xf86NumEntities)
	return -1;
    
    return xf86Entities[entityIndex]->numInstances;
}

_X_EXPORT GDevPtr
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
 * general generic disable function.
 */
static void
disableAccess(void)
{
    int i;
    xf86AccessPtr pacc;
    EntityAccessPtr peacc;
    
    /* call disable funcs and reset current access pointer */
    /* the entity specific access funcs are in an enabled  */
    /* state - driver must restore their state explicitely */
    for (i = 0; i < xf86NumScreens; i++) {
	peacc = xf86Screens[i]->CurrentAccess->pIoAccess;
	while (peacc) {
	    if (peacc->pAccess)
		peacc->pAccess->AccessDisable(peacc->pAccess->arg);
	    peacc = peacc->next;
	}
	xf86Screens[i]->CurrentAccess->pIoAccess = NULL;
	peacc = xf86Screens[i]->CurrentAccess->pMemAccess;
	while (peacc) {
	    if (peacc->pAccess)
		peacc->pAccess->AccessDisable(peacc->pAccess->arg);
	    peacc = peacc->next;
	}
	xf86Screens[i]->CurrentAccess->pMemAccess = NULL;
    }
    /* then call the generic entity disable funcs */
    for (i = 0; i < xf86NumEntities; i++) {
	pacc = xf86Entities[i]->access->fallback; 
	pacc->AccessDisable(pacc->arg);
    }
}

static void
clearAccess(void)
{
    int i;
    
    /* call disable funcs and reset current access pointer */
    /* the entity specific access funcs are in an enabled  */
    /* state - driver must restore their state explicitely */
    for (i = 0; i < xf86NumScreens; i++) {
	xf86Screens[i]->CurrentAccess->pIoAccess = NULL;
	xf86Screens[i]->CurrentAccess->pMemAccess = NULL;
    }

}

/*
 * Generic interface to bus specific code - add other buses here
 */

/*
 * xf86AccessInit() - set up everything needed for access control
 * called only once on first server generation.
 */
void
xf86AccessInit(void)
{
    initPciState();
    initPciBusState();
    DisablePciBusAccess();
    DisablePciAccess();
    
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
    PciBusStateEnter();
    DisablePciBusAccess();
    PciStateEnter();
    disableAccess();
    EntityEnter();
    notifyStateChange(NOTIFY_ENTER);
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
    notifyStateChange(NOTIFY_LEAVE);
    disableAccess();
    DisablePciBusAccess();
    EntityLeave();
}

void
xf86AccessLeaveState(void)
{
    if (!xf86ResAccessEnter)
	return;
    xf86ResAccessEnter = FALSE;
    PciStateLeave();
    PciBusStateLeave();
}

/*
 * xf86AccessRestoreState() - Restore the access registers to the
 * state before X was started. This is handy for framebuffers.
 */
static void 
xf86AccessRestoreState(void)
{
    if (!xf86ResAccessEnter)
	return;
    PciStateLeave();
    PciBusStateLeave();
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

_X_EXPORT void
xf86EnableAccess(ScrnInfoPtr pScrn)
{
    register EntityAccessPtr peAcc = (EntityAccessPtr) pScrn->access;
    register EntityAccessPtr pceAcc;
    register xf86AccessPtr pAcc;
    EntityAccessPtr tmp;

#ifdef DEBUG
    ErrorF("Enable access %i\n",pScrn->scrnIndex);
#endif

    /* Entity is not under access control or currently enabled */
    if (!pScrn->access) {
	if (pScrn->busAccess) {
	    ((BusAccPtr)pScrn->busAccess)->set_f(pScrn->busAccess);
	}
	return;
    }
    
    switch (pScrn->resourceType) {
    case IO:
	pceAcc = pScrn->CurrentAccess->pIoAccess;
	if (peAcc == pceAcc) {
	    return;
	}
	if (pScrn->CurrentAccess->pMemAccess == pceAcc)
	    pScrn->CurrentAccess->pMemAccess = NULL;
	while (pceAcc) {
	    pAcc = pceAcc->pAccess;
	    if (pAcc)
		pAcc->AccessDisable(pAcc->arg);
	    pceAcc = pceAcc->next;
	}
	if (pScrn->busAccess)
	    ((BusAccPtr)pScrn->busAccess)->set_f(pScrn->busAccess);
	while (peAcc) {
	    pAcc = peAcc->pAccess;
	    if (pAcc) 
		pAcc->AccessEnable(pAcc->arg);
	    peAcc = peAcc->next;
	}
	pScrn->CurrentAccess->pIoAccess = (EntityAccessPtr) pScrn->access;
	return;
	
    case MEM_IO:
	pceAcc = pScrn->CurrentAccess->pIoAccess;
	if (peAcc != pceAcc) { /* current Io != pAccess */
	    tmp = pceAcc;
	    while (pceAcc) {
		pAcc = pceAcc->pAccess;
		if (pAcc)
		    pAcc->AccessDisable(pAcc->arg);
		pceAcc = pceAcc->next;
	    }
	    pceAcc = pScrn->CurrentAccess->pMemAccess;
	    if (peAcc != pceAcc /* current Mem != pAccess */
		&& tmp !=pceAcc) {
		while (pceAcc) {
		    pAcc = pceAcc->pAccess;
		    if (pAcc)
			pAcc->AccessDisable(pAcc->arg);
		    pceAcc = pceAcc->next;
		}
	    }
	} else {    /* current Io == pAccess */
	    pceAcc = pScrn->CurrentAccess->pMemAccess;
	    if (pceAcc == peAcc) { /* current Mem == pAccess */
		return;
	    }
	    while (pceAcc) {  /* current Mem != pAccess */
		pAcc = pceAcc->pAccess;
		if (pAcc)
		    pAcc->AccessDisable(pAcc->arg);
		pceAcc = pceAcc->next;
	    }
	}
	if (pScrn->busAccess)
	    ((BusAccPtr)pScrn->busAccess)->set_f(pScrn->busAccess);
	while (peAcc) {
	    pAcc = peAcc->pAccess;
	    if (pAcc) 
		pAcc->AccessEnable(pAcc->arg);
		peAcc = peAcc->next;
	}
	pScrn->CurrentAccess->pMemAccess =
	    pScrn->CurrentAccess->pIoAccess = (EntityAccessPtr) pScrn->access;
	return;
	
    case MEM:
	pceAcc = pScrn->CurrentAccess->pMemAccess;
	if (peAcc == pceAcc) {
	    return;
	}
	if (pScrn->CurrentAccess->pIoAccess == pceAcc)
	    pScrn->CurrentAccess->pIoAccess = NULL;
	while (pceAcc) {
	    pAcc = pceAcc->pAccess;
	    if (pAcc)
		pAcc->AccessDisable(pAcc->arg);
	    pceAcc = pceAcc->next;
	}
	if (pScrn->busAccess)
	    ((BusAccPtr)pScrn->busAccess)->set_f(pScrn->busAccess);
	while (peAcc) {
	    pAcc = peAcc->pAccess;
	    if (pAcc)
		pAcc->AccessEnable(pAcc->arg);
	    peAcc = peAcc->next;
	}
	pScrn->CurrentAccess->pMemAccess = (EntityAccessPtr) pScrn->access;
	return;

    case NONE:
	if (pScrn->busAccess) {
	    ((BusAccPtr)pScrn->busAccess)->set_f(pScrn->busAccess);
	}
	return;
    }
}

_X_EXPORT void
xf86SetCurrentAccess(Bool Enable, ScrnInfoPtr pScrn)
{
    EntityAccessPtr pceAcc2 = NULL;
    register EntityAccessPtr pceAcc = NULL;
    register xf86AccessPtr pAcc;

    
    switch(pScrn->resourceType) {
    case IO:
	pceAcc = pScrn->CurrentAccess->pIoAccess;
	break;
    case MEM:
	pceAcc = pScrn->CurrentAccess->pMemAccess;
	break;
    case MEM_IO:
	pceAcc = pScrn->CurrentAccess->pMemAccess;
	pceAcc2 = pScrn->CurrentAccess->pIoAccess;
	break;
    default:
	break;
    }

    while (pceAcc) {
	pAcc = pceAcc->pAccess;
	if (pAcc) {
	    if (Enable) {
		pAcc->AccessEnable(pAcc->arg);
	    } else {
		pAcc->AccessDisable(pAcc->arg);
	    }
	}
	pceAcc = pceAcc->next;
	if (!pceAcc) {
	    pceAcc = pceAcc2;
	    pceAcc2 = NULL;
	}
    }
}

_X_EXPORT void
xf86SetAccessFuncs(EntityInfoPtr pEnt, xf86SetAccessFuncPtr funcs,
		   xf86SetAccessFuncPtr oldFuncs)
{
    AccessFuncPtr rac;

    if (!xf86Entities[pEnt->index]->rac)
	xf86Entities[pEnt->index]->rac = xnfcalloc(1,sizeof(AccessFuncRec));

    rac = xf86Entities[pEnt->index]->rac;

    rac->mem_new = funcs->mem;
    rac->io_new = funcs->io;
    rac->io_mem_new = funcs->io_mem;
    
    rac->old = oldFuncs;
}

/*
 * Conflict checking
 */

static memType
getMask(memType val)
{
    memType mask = 0;
    memType tmp = 0;
    
    mask=~mask;
    tmp = ~((~tmp) >> 1);
    
    while (!(val & tmp)) {
	mask = mask >> 1;
	val = val << 1;
    }
    return mask;
}

/*
 * checkConflictBlock() -- check for conflicts of a block resource range.
 * If conflict is found return end of conflicting range. Else return 0.
 */
static memType
checkConflictBlock(resRange *range, resPtr pRes)
{
    memType val,tmp,prev;
    int i;
    
    switch (pRes->res_type & ResExtMask) {
    case ResBlock:
	if (range->rBegin < pRes->block_end &&
	    range->rEnd > pRes->block_begin) {
#ifdef DEBUG
	    ErrorF("b-b conflict w: %lx %lx\n",
		   pRes->block_begin,pRes->block_end);
#endif
	    return pRes->block_end < range->rEnd ?
		pRes->block_end : range->rEnd;
	}
	return 0;
    case ResSparse:
	if (pRes->sparse_base > range->rEnd) return 0;
	
	val = (~pRes->sparse_mask | pRes->sparse_base) & getMask(range->rEnd);
#ifdef DEBUG
	ErrorF("base = 0x%lx, mask = 0x%lx, begin = 0x%lx, end = 0x%lx ,"
	       "val = 0x%lx\n",
		pRes->sparse_base, pRes->sparse_mask, range->rBegin,
		range->rEnd, val);
#endif
	i = sizeof(memType) * 8;
	tmp = prev = pRes->sparse_base;
	
	while (i) {
	    tmp |= 1<< (--i) & val;
	    if (tmp > range->rEnd)
		tmp = prev;
	    else
		prev = tmp;
	}
	if (tmp >= range->rBegin) {
#ifdef DEBUG
	    ErrorF("conflict found at: 0x%lx\n",tmp);
	    ErrorF("b-d conflict w: %lx %lx\n",
		   pRes->sparse_base,pRes->sparse_mask);
#endif
	    return tmp;
	}
	else
	    return 0;
    }
    return 0;
}

/*
 * checkConflictSparse() -- check for conflicts of a sparse resource range.
 * If conflict is found return base of conflicting region. Else return 0.
 */
#define mt_max ~(memType)0
#define length sizeof(memType) * 8
static memType
checkConflictSparse(resRange *range, resPtr pRes)
{
    memType val, tmp, prev;
    int i;
    
    switch (pRes->res_type & ResExtMask) {
    case ResSparse:
	tmp = pRes->sparse_mask & range->rMask;
	if ((tmp & pRes->sparse_base) == (tmp & range->rBase)) {
#ifdef DEBUG
	    ErrorF("s-b conflict w: %lx %lx\n",
		   pRes->sparse_base,pRes->sparse_mask);
#endif
	    return pRes->sparse_mask;
	}
	return 0;

    case ResBlock:
	if (pRes->block_end < range->rBase) return 0;
	
	val = (~range->rMask | range->rBase) & getMask(pRes->block_end);
	i = length;
	tmp = prev = range->rBase;
	
	while (i) {
#ifdef DEBUG
	    ErrorF("tmp = 0x%lx\n",tmp);
#endif
	    tmp |= 1<< (--i) & val;
	    if (tmp > pRes->block_end)
		tmp = prev;
	    else
		prev = tmp;
	}
	if (tmp < pRes->block_begin) 
	    return 0;
	else {
	    /*
	     * now we subdivide the block region in sparse regions
	     * with base values = 2^n and find the smallest mask.
	     * This might be done in a simpler way....
	     */
	    memType mask, m_mask = 0, base = pRes->block_begin;
	    int i;	    
	    while (base < pRes->block_end) {
		for (i = 1; i < length; i++)
		    if ( base != (base & (mt_max << i))) break;
		mask = mt_max >> (length - i);
		do mask >>= 1;
		while ((mask + base + 1) > pRes->block_end);
		/* m_mask and are _inverted_ sparse masks */ 
		m_mask = mask > m_mask ? mask : m_mask;
		base = base + mask + 1;
	    }
#ifdef DEBUG
	    ErrorF("conflict found at: 0x%lx\n",tmp);
	    ErrorF("b-b conflict w: %lx %lx\n",
		   pRes->block_begin,pRes->block_end);
#endif
	    return ~m_mask; 
	}
    }
    return 0;
}
#undef mt_max
#undef length

/*
 * needCheck() -- this function decides whether to check for conflicts
 * depending on the types of the resource ranges and their locations
 */
static Bool
needCheck(resPtr pRes, unsigned long type, int entityIndex, xf86State state)
{
    /* the same entity shouldn't conflict with itself */
    ScrnInfoPtr pScrn;
    int i;
    BusType loc = BUS_NONE;
    BusType r_loc = BUS_NONE;

    if ((pRes->res_type & ResTypeMask) != (type & ResTypeMask))
        return FALSE;

    /*
     * Resources set by BIOS (ResBios) are allowed to conflict
     * with resources marked (ResBios).
     */
    if (pRes->res_type & type & ResBios)
	return FALSE;
    
    if (type & pRes->res_type & ResUnused)
 	return FALSE;

    if (state == OPERATING) {
	if (type & ResDisableOpr || pRes->res_type & ResDisableOpr)
	    return FALSE;
	if (type & pRes->res_type & ResUnusedOpr) return FALSE;
	/*
	 * Maybe we should have ResUnused set The resUnusedOpr
	 * bit, too. This way we could avoid this confusion
	 */
	if ((type & ResUnusedOpr && pRes->res_type & ResUnused) ||
	    (type & ResUnused && pRes->res_type & ResUnusedOpr))
	    return FALSE;
    }
    
    if (entityIndex > -1)
	loc = xf86Entities[entityIndex]->busType;
    if (pRes->entityIndex > -1)
	r_loc = xf86Entities[pRes->entityIndex]->busType;

    if ((type & ResAccMask) == ResShared &&
	(pRes->res_type & ResAccMask) == ResShared)
	return FALSE;

    if (pRes->entityIndex == entityIndex) return FALSE;

    if (pRes->entityIndex > -1 &&
	(pScrn = xf86FindScreenForEntity(entityIndex))) {
	for (i = 0; i < pScrn->numEntities; i++)
	    if (pScrn->entityList[i] == pRes->entityIndex) return FALSE;
    }
    return TRUE;
}

/*
 * checkConflict() - main conflict checking function which all other
 * function call.
 */
static memType
checkConflict(resRange *rgp, resPtr pRes, int entityIndex,
	      xf86State state, Bool ignoreIdentical)
{
    memType ret;
    
    while(pRes) {
	if (!needCheck(pRes,rgp->type, entityIndex ,state)) { 
	    pRes = pRes->next;                    
	    continue;                             
	}
	switch (rgp->type & ResExtMask) {
	case ResBlock:
	    if (rgp->rEnd < rgp->rBegin) {
		xf86Msg(X_ERROR,"end of block range 0x%lx < begin 0x%lx\n",
			(long)rgp->rEnd, (long)rgp->rBegin);
		return 0;
	    }
	    if ((ret = checkConflictBlock(rgp, pRes))) {
		if (!ignoreIdentical || (rgp->rBegin != pRes->block_begin)
		    || (rgp->rEnd != pRes->block_end))
		    return ret;
	    }
    break;
	case ResSparse:
	    if ((rgp->rBase & rgp->rMask) != rgp->rBase) {
		xf86Msg(X_ERROR,"sparse io range (base: 0x%lx  mask: 0x%lx)"
			"doesn't satisfy (base & mask = mask)\n",
			(long)rgp->rBase, (long)rgp->rMask);
		return 0;
	    }
	    if ((ret = checkConflictSparse(rgp, pRes))) {
		if (!ignoreIdentical || (rgp->rBase != pRes->sparse_base)
		    || (rgp->rMask != pRes->sparse_mask))
		    return ret;
	    }
	    break;
	}
	pRes = pRes->next;
    }
    return 0;
}

/*
 * xf86ChkConflict() - This function is the low level interface to
 * the resource broker that gets exported. Tests all resources ie.
 * performs test with SETUP flag.
 */
static memType
xf86ChkConflict(resRange *rgp, int entityIndex)
{
    return checkConflict(rgp, Acc, entityIndex, SETUP, FALSE);
}

/*
 * Resources List handling
 */

static resPtr
xf86JoinResLists(resPtr rlist1, resPtr rlist2)
{
    resPtr pRes;

    if (!rlist1)
	return rlist2;

    if (!rlist2)
	return rlist1;

    for (pRes = rlist1; pRes->next; pRes = pRes->next)
	;
    pRes->next = rlist2;
    return rlist1;
}

resPtr
xf86AddResToList(resPtr rlist, resRange *range, int entityIndex)
{
    resPtr new;

    switch (range->type & ResExtMask) {
    case ResBlock:
	if (range->rEnd < range->rBegin) {
		xf86Msg(X_ERROR,"end of block range 0x%lx < begin 0x%lx\n",
			(long)range->rEnd, (long)range->rBegin);
		return rlist;
	}
	break;
    case ResSparse:
	if ((range->rBase & range->rMask) != range->rBase) {
	    xf86Msg(X_ERROR,"sparse io range (base: 0x%lx  mask: 0x%lx)"
		    "doesn't satisfy (base & mask = mask)\n",
		    (long)range->rBase, (long)range->rMask);
	    return rlist;
	}
	break;
    }
    
    new = xnfalloc(sizeof(resRec));
    /* 
     * Only background resources may be registered with ResBios 
     * and ResEstimated set. Other resources only set it for
     * testing.
     */
    if (entityIndex != (-1)) 
        range->type &= ~(ResBios | ResEstimated);
    new->val = *range;
    new->entityIndex = entityIndex;
    new->next = rlist;
    return new;
}

_X_EXPORT void
xf86FreeResList(resPtr rlist)
{
    resPtr pRes;

    if (!rlist)
	return;

    for (pRes = rlist->next; pRes; rlist = pRes, pRes = pRes->next)
	xfree(rlist);
    xfree(rlist);
}

static resPtr
xf86DupResList(const resPtr rlist)
{
    resPtr pRes, ret, prev, new;

    if (!rlist)
	return NULL;

    ret = xnfalloc(sizeof(resRec));
    *ret = *rlist;
    prev = ret;
    for (pRes = rlist->next; pRes; pRes = pRes->next) {
	new = xnfalloc(sizeof(resRec));
	*new = *pRes;
	prev->next = new;
	prev = new;
    }
    return ret;
}

static void
xf86PrintResList(int verb, resPtr list)
{
    int i = 0;
    const char *s, *r;
    resPtr tmp = list;
    unsigned long type;
    
    if (!list)
	return;

    type = ResMem;
    r = "M";
    while (1) {
	while (list) {
	    if ((list->res_type & ResPhysMask) == type) {
		switch (list->res_type & ResExtMask) {
		case ResBlock:
		    xf86ErrorFVerb(verb,
				   "\t[%d] %d\t%ld\t0x%08lx - 0x%08lx (0x%lx)",
				   i, list->entityIndex,
				   (list->res_type & ResDomain) >> 24,
				   (long)list->block_begin,
				   (long)list->block_end,
				   (long)(list->block_end -
					  list->block_begin + 1));
		    break;
		case ResSparse:
		    xf86ErrorFVerb(verb, "\t[%d] %d\t%ld\t0x%08lx - 0x%08lx ",
				   i, list->entityIndex,
				   (list->res_type & ResDomain) >> 24,
				   (long)list->sparse_base,
				   (long)list->sparse_mask);
		    break;
		default:
		    list = list->next;
		    continue;
		}
		xf86ErrorFVerb(verb, " %s", r);
		switch (list->res_type & ResAccMask) {
		case ResExclusive:
		    if (list->res_type & ResUnused)
			s = "x";
		    else
			s = "X";
		    break;
		case ResShared:
		    if (list->res_type & ResUnused)
			s = "s";
		    else
			s = "S";
		    break;
		default:
		    s = "?";
		}
		xf86ErrorFVerb(verb, "%s", s);
		switch (list->res_type & ResExtMask) {
		case ResBlock:
		    s = "[B]";
		    break;
		case ResSparse:
		    s = "[S]";
		    break;
		default:
		    s = "[?]";
		}
		xf86ErrorFVerb(verb, "%s", s);
		if (list->res_type & ResInit)
		    xf86ErrorFVerb(verb, "t");
		if (list->res_type & ResBios)
		    xf86ErrorFVerb(verb, "(B)");
		if (list->res_type & ResBus)
		    xf86ErrorFVerb(verb, "(b)");
		if (list->res_type & ResOprMask) {
		    switch (list->res_type & ResOprMask) {
		    case ResUnusedOpr:
			s = "(OprU)";
			break;
		    case ResDisableOpr:
			s = "(OprD)";
			break;
		    default:
			s = "(Opr?)";
			break;
		    }
		    xf86ErrorFVerb(verb, "%s", s);
		}
		xf86ErrorFVerb(verb, "\n");
		i++;
	    }
	    list = list->next;
	}
	if (type == ResIo) break;
	type = ResIo;
	r = "I";
	list = tmp;
    }
}

resPtr
xf86AddRangesToList(resPtr list, resRange *pRange, int entityIndex)
{
    while(pRange && pRange->type != ResEnd) {
	list = xf86AddResToList(list,pRange,entityIndex);
	pRange++;
    }
    return list;
}

void
xf86ResourceBrokerInit(void)
{
    Acc = NULL;

    /* Get the ranges used exclusively by the system */
    Acc = xf86AccResFromOS(Acc);
    xf86MsgVerb(X_INFO, 3, "System resource ranges:\n");
    xf86PrintResList(3, Acc);
}

/*
 * Resource registration
 */

static void
convertRange2Host(int entityIndex, resRange *pRange)
{
    if (pRange->type & ResBus) {
	switch (xf86Entities[entityIndex]->busType) {
	case BUS_PCI:
	    pciConvertRange2Host(entityIndex,pRange);
	    break;
	default:
	    break;
	}

	pRange->type &= ~ResBus;
    }
}

static void
xf86ConvertListToHost(int entityIndex, resPtr list)
{
    while (list) {
	convertRange2Host(entityIndex, &list->val);
	list = list->next;
    }
}

/*
 * xf86RegisterResources() -- attempts to register listed resources.
 * Returns a resPtr listing all resources not successfully registered, by
 * which we mean, NULL.
 */

_X_EXPORT resPtr
xf86RegisterResources(int entityIndex, resList list, unsigned long access)
{
    resRange range;
    resList list_f = NULL;

    if (!list)
	return NULL;

    while(list->type != ResEnd) {
	range = *list;

	convertRange2Host(entityIndex,&range);

	if ((access != ResNone) && (access & ResAccMask)) {
	    range.type = (range.type & ~ResAccMask) | (access & ResAccMask);
	}
 	range.type &= ~ResEstimated;	/* Not allowed for drivers */
	Acc = xf86AddResToList(Acc,&range,entityIndex);
	list++;
    }
    if (list_f)
      xfree(list_f);

#ifdef DEBUG
    xf86MsgVerb(X_INFO, 3,"Resources after driver initialization\n");
    xf86PrintResList(3, Acc);
#endif
    return NULL;
    
}

static void
busTypeSpecific(EntityPtr pEnt, xf86AccessPtr *acc_mem,
		xf86AccessPtr *acc_io, xf86AccessPtr *acc_mem_io)
{
    switch (pEnt->bus.type) {
    case BUS_SBUS:
	*acc_mem = *acc_io = *acc_mem_io = &AccessNULL;
	break;
    case BUS_PCI: {
	struct pci_device *const dev = pEnt->bus.id.pci;

	if ((dev != NULL) && ((void *)dev->user_data != NULL)) {
	    pciAccPtr const paccp = (pciAccPtr) dev->user_data;
	    
	    *acc_io = & paccp->ioAccess;
	    *acc_mem = & paccp->memAccess;
	    *acc_mem_io = & paccp->io_memAccess;
	}
	else {
	    /* FIXME: This is an error path.  We should probably have an
	     * FIXME: assertion here or something.
	     */
	    *acc_io = NULL;
	    *acc_mem = NULL;
	    *acc_mem_io = NULL;
	}
	break;
    }
    default:
	*acc_mem = *acc_io = *acc_mem_io = NULL;
	break;
    }
    return;
}

static void
setAccess(EntityPtr pEnt, xf86State state)
{

    xf86AccessPtr acc_mem, acc_io, acc_mem_io;
    xf86AccessPtr org_mem = NULL, org_io = NULL, org_mem_io = NULL;
    int prop;
    
    busTypeSpecific(pEnt, &acc_mem, &acc_io, &acc_mem_io);

    /* The replacement function needs to handle _all_ shared resources */
    /* unless they are handeled locally and disabled otherwise         */
    if (pEnt->rac) {
	if (pEnt->rac->io_new) {
	    org_io = acc_io;
	    acc_io = pEnt->rac->io_new;
	}
	if (pEnt->rac->mem_new) {
	    org_mem = acc_mem;
	    acc_mem = pEnt->rac->mem_new;
	}	
	if (pEnt->rac->io_mem_new) {
	    org_mem_io = acc_mem_io;
	    acc_mem_io = pEnt->rac->io_mem_new;
	}   
    }
    
    if (state == OPERATING) {
	prop = pEnt->entityProp;
	switch(pEnt->entityProp & NEED_SHARED) {
	case NEED_SHARED:
	    pEnt->access->rt = MEM_IO;
	    break;
	case NEED_IO_SHARED:
	    pEnt->access->rt = IO;
	    break;
	case NEED_MEM_SHARED:
	    pEnt->access->rt = MEM;
	    break;
	default:
	    pEnt->access->rt = NONE;
	}
    } else {
	prop = NEED_SHARED | NEED_MEM | NEED_IO;
	pEnt->access->rt = MEM_IO;
    }
    
    switch(pEnt->access->rt) {
    case IO:
	pEnt->access->pAccess = acc_io;
	break;
    case MEM:
	pEnt->access->pAccess = acc_mem;
	break;
    case MEM_IO:
	pEnt->access->pAccess = acc_mem_io;
	break;
    default: /* no conflicts at all */
	pEnt->access->pAccess =  NULL; /* remove from RAC */
	break;
    }

    if (org_io) {
	/* does the driver want the old access func? */
	if (pEnt->rac->old) {
	    /* give it to the driver, leave state disabled */
	    pEnt->rac->old->io = org_io;
	} else {
	    /* driver doesn't want it - enable generic access */
	    org_io->AccessEnable(org_io->arg);
	}
    }

    if (org_mem_io) {
	/* does the driver want the old access func? */
	if (pEnt->rac->old) {
	    /* give it to the driver, leave state disabled */
	    pEnt->rac->old->io_mem = org_mem_io;
	} else {
	    /* driver doesn't want it - enable generic access */
	    org_mem_io->AccessEnable(org_mem_io->arg);
	}
    }

    if (org_mem) {
	/* does the driver want the old access func? */
	if (pEnt->rac->old) {
	    /* give it to the driver, leave state disabled */
	    pEnt->rac->old->mem = org_mem;
	} else {
	    /* driver doesn't want it - enable generic access */
	    org_mem->AccessEnable(org_mem->arg);
	}
    }

    if (!(prop & NEED_MEM_SHARED)){
	if (prop & NEED_MEM) {
	    if (acc_mem)
		acc_mem->AccessEnable(acc_mem->arg);
	} else {
	    if (acc_mem)
		acc_mem->AccessDisable(acc_mem->arg);
	}
    }

    if (!(prop & NEED_IO_SHARED)) {
	if (prop & NEED_IO) {
	    if (acc_io)
		acc_io->AccessEnable(acc_io->arg);
	} else {
	    if (acc_io)
		acc_io->AccessDisable(acc_io->arg);
	}
    }

    /* disable shared resources */
    if (pEnt->access->pAccess)
	pEnt->access->pAccess->AccessDisable(pEnt->access->pAccess->arg);

    /*
     * If device is not under access control it is enabled.
     * If it needs bus routing do it here as it isn't bus
     * type specific. Any conflicts should be checked at this
     * stage
     */
    if (!pEnt->access->pAccess
	&& (pEnt->entityProp & (state == SETUP ? NEED_VGA_ROUTED_SETUP :
				NEED_VGA_ROUTED)))
	((BusAccPtr)pEnt->busAcc)->set_f(pEnt->busAcc);
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
    EntityPtr pEnt;
    ScrnInfoPtr pScrn;
    int i,j;
    int needVGA = 0;
    resType rt;
    /* 
     * This is a good place to block SIGIO during SETUP state.
     * SIGIO should be blocked in SETUP state otherwise (u)sleep()
     * might get interrupted early. 
     * We take care not to call xf86BlockSIGIO() twice. 
     */
    SetSIGIOForState(state);
#ifdef DEBUG
    if (state == SETUP)
	ErrorF("Entering SETUP state\n");
    else
	ErrorF("Entering OPERATING state\n");
#endif

    /* When servicing a dumb framebuffer we don't need to do anything */
    if (doFramebufferMode) return;

    for (i=0; i<xf86NumScreens; i++) {
	pScrn = xf86Screens[i];
	j = pScrn->entityList[pScrn->numEntities - 1];
	pScrn->access = xf86Entities[j]->access;
	
 	for (j = 0; j<xf86Screens[i]->numEntities; j++) {
 	    pEnt = xf86Entities[xf86Screens[i]->entityList[j]];
 	    if (pEnt->entityProp & (state == SETUP ? NEED_VGA_ROUTED_SETUP
 				    : NEED_VGA_ROUTED)) 
		xf86Screens[i]->busAccess = pEnt->busAcc;
 	}
	if (xf86Screens[i]->busAccess)
	    needVGA ++;
    }
    
    /*
     * if we just have one screen we don't have RAC.
     * Therefore just enable the screen and return.
     */
    if (!needRAC) {
	xf86EnableAccess(xf86Screens[0]);
	notifyStateChange(NOTIFY_ENABLE);
	return;
    }
    
    if (state == SETUP)
	notifyStateChange(NOTIFY_SETUP_TRANSITION);
    else
	notifyStateChange(NOTIFY_OPERATING_TRANSITION);
    
    clearAccess();
    for (i=0; i<xf86NumScreens;i++) {

	rt = NONE;
	
	for (j = 0; j<xf86Screens[i]->numEntities; j++) {
	    pEnt = xf86Entities[xf86Screens[i]->entityList[j]];
	    setAccess(pEnt,state);

	    if (pEnt->access->rt != NONE) {
		if (rt != NONE && rt != pEnt->access->rt)
		    rt = MEM_IO;
		else
		    rt = pEnt->access->rt;
	    }
	}
	xf86Screens[i]->resourceType = rt;
	if (rt == NONE) {
	    xf86Screens[i]->access = NULL;
	    if (needVGA < 2)
		xf86Screens[i]->busAccess = NULL;
	}
	
#ifdef DEBUG
	if (xf86Screens[i]->busAccess)
	    ErrorF("Screen %i setting vga route\n",i);
#endif
	switch (rt) {
	case MEM_IO:
	    xf86MsgVerb(X_INFO, 3, "Screen %i shares mem & io resources\n",i);
	    break;
	case IO:
	    xf86MsgVerb(X_INFO, 3, "Screen %i shares io resources\n",i);
	    break;
	case MEM:
	    xf86MsgVerb(X_INFO, 3, "Screen %i shares mem resources\n",i);
	    break;
	default:
	    xf86MsgVerb(X_INFO, 3, "Entity %i shares no resources\n",i);
	    break;
	}
    }
    if (state == SETUP)
	notifyStateChange(NOTIFY_SETUP);
    else
	notifyStateChange(NOTIFY_OPERATING);
}

/*
 * xf86SetOperatingState() -- Set ResOperMask for resources listed.
 */
_X_EXPORT resPtr
xf86SetOperatingState(resList list, int entityIndex, int mask)
{
    resPtr acc;
    resPtr r_fail = NULL;
    resRange range;
    
    while (list->type != ResEnd) {
	range = *list;
	convertRange2Host(entityIndex,&range);

	acc = Acc;
	while (acc) {
#define MASK (ResTypeMask | ResExtMask)
	    if ((acc->entityIndex == entityIndex) 
		&& (acc->val.a == range.a) && (acc->val.b == range.b)
		&& ((acc->val.type & MASK) == (range.type & MASK)))
		break;
#undef MASK
	    acc = acc->next;
	}
	if (acc)
	    acc->val.type = (acc->val.type & ~ResOprMask)
		| (mask & ResOprMask);
	else {
	    r_fail = xf86AddResToList(r_fail,&range,entityIndex);
	}
	list ++;
    }
    
     return r_fail;
}

/*
 * Stage specific code
 */

/*
 * xf86ClaimFixedResources() is used to allocate non-relocatable resources.
 * This should only be done by a driver's Probe() function.
 */
_X_EXPORT void
xf86ClaimFixedResources(resList list, int entityIndex)
{
    resPtr ptr = NULL;
    resRange range;	

    if (!list) return;
    
    while (list->type !=ResEnd) {
 	range = *list;

	convertRange2Host(entityIndex,&range);

 	range.type &= ~ResEstimated;	/* Not allowed for drivers */
 	switch (range.type & ResAccMask) {
  	case ResExclusive:
 	    if (!xf86ChkConflict(&range, entityIndex)) {
 		Acc = xf86AddResToList(Acc, &range, entityIndex);
	    } else FatalError("xf86ClaimFixedResources conflict\n");
	    break;
	case ResShared:
	    /* at this stage the resources are just added to the
	     * EntityRec. After the Probe() phase this list is checked by
	     * xf86PostProbe(). All resources which don't
	     * conflict with already allocated ones are allocated
	     * and removed from the EntityRec. Thus a non-empty resource
	     * list in the EntityRec indicates resource conflicts the
	     * driver should either handle or fail.
	     */
	    if (xf86Entities[entityIndex]->active)
		ptr = xf86AddResToList(ptr,&range,entityIndex);
	    break;
	}
	list++;
    }
    xf86Entities[entityIndex]->resources =
	xf86JoinResLists(xf86Entities[entityIndex]->resources,ptr);
    xf86MsgVerb(X_INFO, 3,
	"resource ranges after xf86ClaimFixedResources() call:\n");
    xf86PrintResList(3,Acc);
#ifdef DEBUG
    if (ptr) {
	xf86MsgVerb(X_INFO, 3, "to be registered later:\n");
	xf86PrintResList(3,ptr);
    }
#endif
}

static void
checkRoutingForScreens(xf86State state)
{
    resList list = resVgaUnusedExclusive;
    resPtr pResVGA = NULL;
    resPtr pResVGAHost;
    pointer vga = NULL;
    int i,j;
    int entityIndex;
    EntityPtr pEnt;
    resPtr pAcc;
    resRange range;

    /*
     * find devices that need VGA routed: ie the ones that have
     * registered VGA resources without ResUnused. ResUnused
     * doesn't conflict with itself therefore use it here.
     */
    while (list->type != ResEnd) { /* create resPtr from resList for VGA */
	range = *list;
	range.type &= ~(ResBios | ResEstimated); /* if set remove them */
	pResVGA = xf86AddResToList(pResVGA, &range, -1);
	list++;
    }

    for (i = 0; i < xf86NumScreens; i++) {
	for (j = 0; j < xf86Screens[i]->numEntities; j++) {
	    entityIndex = xf86Screens[i]->entityList[j];
	    pEnt = xf86Entities[entityIndex];
	    pAcc = Acc;
	    vga = NULL;
	    pResVGAHost = xf86DupResList(pResVGA);
	    xf86ConvertListToHost(entityIndex,pResVGAHost);
	    while (pAcc) {
		if (pAcc->entityIndex == entityIndex)
		    if (checkConflict(&pAcc->val, pResVGAHost,
				      entityIndex, state, FALSE)) {
			if (vga && vga != pEnt->busAcc) {
			    xf86Msg(X_ERROR, "Screen %i needs vga routed to"
				    "different buses - deleting\n",i);
			    xf86DeleteScreen(i--,0);
			}
#ifdef DEBUG
			{
			    resPtr rlist = xf86AddResToList(NULL,&pAcc->val,
							    pAcc->entityIndex);
			    xf86MsgVerb(X_INFO,3,"====== %s\n",
					state == OPERATING ? "OPERATING"
					: "SETUP");
			    xf86MsgVerb(X_INFO,3,"%s Resource:\n",
					(pAcc->val.type) & ResMem ? "Mem" :"Io");
			    xf86PrintResList(3,rlist);
			    xf86FreeResList(rlist);
			    xf86MsgVerb(X_INFO,3,"Conflicts with:\n");
			    xf86PrintResList(3,pResVGAHost);
			    xf86MsgVerb(X_INFO,3,"=====\n");
			}
#endif
			vga = pEnt->busAcc;
			pEnt->entityProp |= (state == SETUP
			    ? NEED_VGA_ROUTED_SETUP : NEED_VGA_ROUTED);
			if (state == OPERATING) {
			    if (pAcc->val.type & ResMem)
				pEnt->entityProp |= NEED_VGA_MEM;
			    else
				pEnt->entityProp |= NEED_VGA_IO;
			}
		    }
		pAcc = pAcc->next;
	    }
	    if (vga)
		xf86MsgVerb(X_INFO, 3,"Setting vga for screen %i.\n",i);
	    xf86FreeResList(pResVGAHost);
	}
    }
    xf86FreeResList(pResVGA);
}

/*
 * xf86PostProbe() -- Allocate all non conflicting resources
 * This function gets called by xf86Init().
 */
void
xf86PostProbe(void)
{
    memType val;
    int i,j;
    resPtr resp, acc, tmp, resp_x;

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
	    xf86AccessRestoreState();
	    notifyStateChange(NOTIFY_ENABLE);
	    doFramebufferMode = TRUE;

	    return;
	}
    }
    acc = tmp = xf86DupResList(Acc);

    for (i=0; i<xf86NumEntities; i++) {
	resp = xf86Entities[i]->resources;
	xf86Entities[i]->resources = NULL;
	resp_x = NULL;
	while (resp) {
	    if (! (val = checkConflict(&resp->val,acc,i,SETUP,FALSE)))  {
 	        resp->res_type &= ~(ResBios); /* just used for chkConflict() */
		tmp = resp_x;
		resp_x = resp;
		resp = resp->next;
		resp_x->next = tmp;
		} else {
		    xf86MsgVerb(X_INFO, 3, "Found conflict at: 0x%lx\n",
				(long)val);
 		    resp->res_type &= ~ResEstimated;
		    tmp = xf86Entities[i]->resources;
		    xf86Entities[i]->resources = resp;
		    resp = resp->next;
		    xf86Entities[i]->resources->next = tmp;
		}
	}
	xf86JoinResLists(Acc,resp_x);
    }
    xf86FreeResList(acc);
    
    xf86MsgVerb(X_INFO, 3, "resource ranges after probing:\n");
    xf86PrintResList(3, Acc);
    checkRoutingForScreens(SETUP);

    for (i = 0; i < xf86NumScreens; i++) {
	for (j = 0; j<xf86Screens[i]->numEntities; j++) {
	    EntityPtr pEnt = xf86Entities[xf86Screens[i]->entityList[j]];
 	    if ((pEnt->entityProp & NEED_VGA_ROUTED_SETUP) &&
 		((xf86Screens[i]->busAccess = pEnt->busAcc)))
		break;
	}
    }
}

static void
checkRequiredResources(int entityIndex)
{
    resRange range;
    resPtr pAcc = Acc;
    const EntityPtr pEnt = xf86Entities[entityIndex];
    while (pAcc) {
	if (pAcc->entityIndex == entityIndex) {
	    range = pAcc->val;
	    /*  ResAny to find conflicts with anything. */
	    range.type = (range.type & ~ResAccMask) | ResAny | ResBios;
	    if (checkConflict(&range,Acc,entityIndex,OPERATING,FALSE))
		switch (pAcc->res_type & ResPhysMask) {
		case ResMem:
		    pEnt->entityProp |= NEED_MEM_SHARED;
		    break;
		case ResIo:
		    pEnt->entityProp |= NEED_IO_SHARED;
		    break;
		}
	    if (!(pAcc->res_type & ResOprMask)) {
		switch (pAcc->res_type & ResPhysMask) {
		case ResMem:
		    pEnt->entityProp |= NEED_MEM;
		    break;
		case ResIo:
		    pEnt->entityProp |= NEED_IO;
		    break;
		}
	    }
	}
	pAcc = pAcc->next;
    }
    
    /*
     * After we have checked all resources of an entity agains any
     * other resource we know if the entity need this resource type
     * (ie. mem/io) at all. if not we can disable this type completely,
     * so no need to share it either. 
     */
    if ((pEnt->entityProp & NEED_MEM_SHARED)
	&& (!(pEnt->entityProp & NEED_MEM)))
	pEnt->entityProp &= ~(unsigned long)NEED_MEM_SHARED;

    if ((pEnt->entityProp & NEED_IO_SHARED)
	&& (!(pEnt->entityProp & NEED_IO)))
	pEnt->entityProp &= ~(unsigned long)NEED_IO_SHARED;
}

void
xf86PostPreInit()
{
  if (doFramebufferMode) return;

    if (xf86NumScreens > 1)
	needRAC = TRUE;

    xf86MsgVerb(X_INFO, 3, "do I need RAC?");
    
    if (needRAC) {
	xf86ErrorFVerb(3, "  Yes, I do.\n");
    } else {
	xf86ErrorFVerb(3, "  No, I don't.\n");
    }
 	
    xf86MsgVerb(X_INFO, 3, "resource ranges after preInit:\n");
    xf86PrintResList(3, Acc);
}

void
xf86PostScreenInit(void)
{
    int i,j;
    ScreenPtr pScreen;
    unsigned int flags;
    int nummem = 0, numio = 0;

    if (doFramebufferMode) {
	SetSIGIOForState(OPERATING);
	return;
    }

#ifdef DEBUG
    ErrorF("PostScreenInit  generation: %i\n",serverGeneration);
#endif
    if (serverGeneration == 1) {
	checkRoutingForScreens(OPERATING);
	for (i=0; i<xf86NumEntities; i++) {
	    checkRequiredResources(i);
	}
	
	/*
	 * after removing NEED_XXX_SHARED from entities that
	 * don't need need XXX resources at all we might have
	 * a single entity left that has NEED_XXX_SHARED set.
	 * In this case we can delete that, too.
	 */
	for (i = 0; i < xf86NumEntities; i++) {
	    if (xf86Entities[i]->entityProp & NEED_MEM_SHARED)
		nummem++;
	    if (xf86Entities[i]->entityProp & NEED_IO_SHARED)
		numio++;
	}
	for (i = 0; i < xf86NumEntities; i++) {
	    if (nummem < 2)
		xf86Entities[i]->entityProp &= ~NEED_MEM_SHARED;
	    if (numio < 2)
		xf86Entities[i]->entityProp &= ~NEED_IO_SHARED;
	}
    }
    
    if (xf86Screens && needRAC) {
	int needRACforVga = 0;

	for (i = 0; i < xf86NumScreens; i++) {
	    for (j = 0; j < xf86Screens[i]->numEntities; j++) {
		if (xf86Entities[xf86Screens[i]->entityList[j]]->entityProp
		    & NEED_VGA_ROUTED) {
		    needRACforVga ++;
		    break; /* only count each screen once */
		}
	    }
	}
	
	for (i = 0; i < xf86NumScreens; i++) {
	    Bool needRACforMem = FALSE, needRACforIo = FALSE;
	    
	    for (j = 0; j < xf86Screens[i]->numEntities; j++) {
		if (xf86Entities[xf86Screens[i]->entityList[j]]->entityProp
		    & NEED_MEM_SHARED)
		    needRACforMem = TRUE;
		if (xf86Entities[xf86Screens[i]->entityList[j]]->entityProp
		    & NEED_IO_SHARED)
		    needRACforIo = TRUE;
		/*
		 * We may need RAC although we don't share any resources
		 * as we need to route VGA to the correct bus. This can
		 * only be done simultaniously for MEM and IO.
		 */
		if (needRACforVga > 1) {
		    if (xf86Entities[xf86Screens[i]->entityList[j]]->entityProp
			& NEED_VGA_MEM)
			needRACforMem = TRUE;
		    if (xf86Entities[xf86Screens[i]->entityList[j]]->entityProp
			& NEED_VGA_IO)
			needRACforIo = TRUE;		
		}
	    }
		
	    pScreen = xf86Screens[i]->pScreen;
	    flags = 0;
	    if (needRACforMem) {
		flags |= xf86Screens[i]->racMemFlags;
		xf86ErrorFVerb(3, "Screen %d is using RAC for mem\n", i);
	    }
	    if (needRACforIo) {
		flags |= xf86Screens[i]->racIoFlags;
		xf86ErrorFVerb(3, "Screen %d is using RAC for io\n", i);
	    }
	    
	    xf86RACInit(pScreen,flags);
	}
    }
    
    xf86EnterServerState(OPERATING);
    
}

/*
 * Sets
 */


static resPtr
decomposeSparse(resRange range)
{
    resRange new;
    resPtr ret = NULL;
    memType val = range.rBegin;
    int i = 0;
    
    new.type = (range.type & ~ResExtMask) | ResSparse;

    while (1) {
	if (val & 0x01) {
	    new.rBase = (val << i);
	    new.rMask = ~((1 << i) - 1);
	    ret = xf86AddResToList(ret,&new,-1);
	    val ++;
	}
	i++;
	val >>= 1;
	if ((((val + 1) << i) - 1) > range.rEnd)
	    break;
    }
    i--;
    val <<= 1;
    
    while (1) {
	if((((val + 1) << i) - 1)> range.rEnd) {
	    if (--i < 0) break;
	    val <<= 1;
	} else {
	    new.rBase = (val << i);
	    new.rMask = ~((1 << i) - 1);
	    val++;
	    ret = xf86AddResToList(ret,&new,-1);
	}
    }
    return ret;
}
    
static Bool
x_isSubsetOf(resRange range, resPtr list1, resPtr list2)
{
    resRange range1, range2;
    memType m1_A_m2;
    Bool ret;
    resPtr list;
    
    if (list1) {
	list = list1;
	if ((range.type & ResTypeMask) == (list->res_type & ResTypeMask)) {
	    switch (range.type & ResExtMask) {
	    case ResBlock:
		if ((list->res_type & ResExtMask) == ResBlock) {
		    if (range.rBegin >= list->block_begin
			&& range.rEnd <= list->block_end)
			return TRUE;
		    else if (range.rBegin < list->block_begin
			     && range.rEnd > list->block_end) {
			RANGE(range1, range.rBegin, list->block_begin - 1,
			      range.type);
			RANGE(range2, list->block_end + 1, range.rEnd,
			      range.type);
			return (x_isSubsetOf(range1,list->next,list2) &&
				x_isSubsetOf(range2,list->next,list2));
		    }
		    else if (range.rBegin >= list->block_begin
			     && range.rBegin <= list->block_end) {
			RANGE(range1, list->block_end + 1, range.rEnd,
			      range.type);
			return (x_isSubsetOf(range1,list->next,list2));
		    } else if (range.rEnd >= list->block_begin
			       && range.rEnd <= list->block_end) {
			RANGE(range1,range.rBegin, list->block_begin - 1,
			      range.type);
			return (x_isSubsetOf(range1,list->next,list2));
		    } 
		}
		break;
	    case ResSparse:
		if ((list->res_type & ResExtMask) == ResSparse) {
		    memType test;
		    int i;
		    
		    m1_A_m2 = range.rMask & list->sparse_mask;
		    if ((range.rBase ^ list->sparse_base) & m1_A_m2)
			break;
		    /*
		     * We use the following system:
		     * let 0 ^= mask:1 base:0, 1 ^= mask:1 base:1,
		     * X mask:0 ; S: set TSS: test set for subset
		     * NTSS: new test set after test
		     *    S: 1   0   1   0   X   X   0   1   X
		     *  TSS: 1   0   0   1   1   0   X   X   X
		     *    T: 0   0   1   1   0   0   0   0   0
		     * NTSS: 1   0  0/X  1/X 1   0   1   0   X
		     *    R: 0   0   0   0   0   0   1   1   0
		     * If R != 0 TSS and S are disjunct
		     * If R == 0 TSS is subset of S
		     * If R != 0 NTSS contains elements from TSS
		     * which are not also members of S.
		     * If a T is set one of the correspondig bits
		     * in NTSS must be set to the specified value
		     * all other are X
		     */
		    test = list->sparse_mask & ~range.rMask;
		    if (test == 0)
			return TRUE;
		    for (i = 0; i < sizeof(memType); i++) {
			if ((test >> i) & 0x1) {
			    RANGE(range1, ((range.rBase & list->sparse_base)
				  | (range.rBase & ~list->sparse_mask)
				  | ((~list->sparse_base & list->sparse_mask)
				     & ~range.rMask)) & range1.rMask,
				  ((range.rMask | list->sparse_mask) & ~test)
				  | (1 << i), range.type);
			    return (x_isSubsetOf(range1,list->next,list2));
			}
		    }
		}
		break;
	    }
	}
	return (x_isSubsetOf(range,list->next,list2));
    } else if (list2) {
	resPtr tmpList = NULL;
	switch (range.type & ResExtMask) {
	case ResBlock:
	    tmpList = decomposeSparse(range);
	    while (tmpList) {
		if (!x_isSubsetOf(tmpList->val,list2,NULL)) {
		    xf86FreeResList(tmpList);
		    return FALSE;
		}
		tmpList = tmpList->next;
	    }
	    xf86FreeResList(tmpList);
	    return TRUE;
	    break;
	case ResSparse:
	    while (list2) {
		tmpList = xf86JoinResLists(tmpList,decomposeSparse(list2->val));
		list2 = list2->next;
	    }
	    ret = x_isSubsetOf(range,tmpList,NULL);
	    xf86FreeResList(tmpList);
	    return ret;
	    break;
	}
    } else
	return FALSE;

    return FALSE;
}

Bool
xf86IsSubsetOf(resRange range, resPtr list)
{
    resPtr dup = xf86DupResList(list);
    resPtr r_sp = NULL, r = NULL, tmp = NULL;
    Bool ret = FALSE;
    
    while (dup) {
	tmp = dup;
	dup = dup->next;
	switch (tmp->res_type & ResExtMask) {
	case ResBlock:
	    tmp->next = r;
	    r = tmp;
	    break;
	case ResSparse:
	    tmp->next = r_sp;
	    r_sp = tmp;
	    break;
	}
    }
    
    switch (range.type & ResExtMask) {
    case ResBlock:
	ret = x_isSubsetOf(range,r,r_sp);
	break;
    case ResSparse:
	ret = x_isSubsetOf(range,r_sp,r);
	break;
    }
    xf86FreeResList(r);
    xf86FreeResList(r_sp);
    
    return ret;
}

static resPtr
findIntersect(resRange Range, resPtr list)
{
    resRange range;
    resPtr new = NULL;
    
    while (list) {
	    if ((Range.type & ResTypeMask) == (list->res_type & ResTypeMask)) {
		switch (Range.type & ResExtMask) {
		case ResBlock:
		    switch (list->res_type & ResExtMask) {
		    case ResBlock:
			if (Range.rBegin >= list->block_begin)
			    range.rBegin = Range.rBegin;
			else
			    range.rBegin = list->block_begin;
			if (Range.rEnd <= list->block_end)
			    range.rEnd = Range.rEnd;
			else 
			    range.rEnd = list->block_end;
			if (range.rEnd > range.rBegin) {
			    range.type = Range.type;
			    new = xf86AddResToList(new,&range,-1);
			}
			break;
		    case ResSparse:
			new = xf86JoinResLists(new,xf86FindIntersectOfLists(new,decomposeSparse(list->val)));
			break;
		    }
		    break;
		case ResSparse:
		    switch (list->res_type & ResExtMask) {
		    case ResSparse:
			if (!((~(range.rBase ^ list->sparse_base)
			    & (range.rMask & list->sparse_mask)))) {
			    RANGE(range, (range.rBase & list->sparse_base)
				  | (~range.rMask & list->sparse_base)
				  | (~list->sparse_mask & range.rBase),
				  range.rMask | list->sparse_mask,
				  Range.type);
			    new = xf86AddResToList(new,&range,-1);
			}
			break;
		    case ResBlock:
			new = xf86JoinResLists(new,xf86FindIntersectOfLists(
			    decomposeSparse(range),list));
			break;
		    }
		}
	    }
	list = list->next;
    }
    return new;
}
    
resPtr
xf86FindIntersectOfLists(resPtr l1, resPtr l2)
{
    resPtr ret = NULL;

    while (l1) {
	ret = xf86JoinResLists(ret,findIntersect(l1->val,l2));
	l1 = l1->next;
    }
    return ret;
}

#if 0	/* Not used */
static resPtr
xf86FindComplement(resRange Range)
{
    resRange range;
    memType tmp;
    resPtr new = NULL;
    int i;
    
    switch (Range.type & ResExtMask) {
    case ResBlock:
	if (Range.rBegin > 0) {
	    RANGE(range, 0, Range.rBegin - 1, Range.type);
	    new = xf86AddResToList(new,&range,-1);
	}
	if (Range.rEnd < (memType)~0) {
	    RANGE(range,Range.rEnd + 1, (memType)~0, Range.type);
	    new = xf86AddResToList(new,&range,-1);
	}
	break;
    case ResSparse:
	tmp = Range.rMask;
	for (i = 0; i < sizeof(memType); i++) {
	    if (tmp & 0x1) {
		RANGE(range,(~Range.rMask & range.rMask),(1 << i), Range.type);
		new = xf86AddResToList(new,&range,-1);
	    }
	}
	break;
    default:
	break;
    }
    return new;
}
#endif

resPtr
xf86ExtractTypeFromList(resPtr list, unsigned long type)
{
    resPtr ret = NULL;
    
    while (list) {
	if ((list->res_type & ResTypeMask) == type)
	    ret = xf86AddResToList(ret,&(list->val),list->entityIndex);
	list = list->next;
    }
    return ret;
}

/*
 * xf86FindPrimaryDevice() - Find the display device which
 * was active when the server was started.
 */
void
xf86FindPrimaryDevice()
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

Bool
xf86NoSharedResources(int screenIndex,resType res)
{
    int j;
    
    if (screenIndex > xf86NumScreens)
	return TRUE;

    for (j = 0; j < xf86Screens[screenIndex]->numEntities; j++) {
      switch (res) {
      case IO:
	if ( xf86Entities[xf86Screens[screenIndex]->entityList[j]]->entityProp
	     & NEED_IO_SHARED)
	  return FALSE;
	break;
      case MEM:
	if ( xf86Entities[xf86Screens[screenIndex]->entityList[j]]->entityProp
	     & NEED_MEM_SHARED)
	  return FALSE;
	break;
      case MEM_IO:
	if ( xf86Entities[xf86Screens[screenIndex]->entityList[j]]->entityProp
	     & NEED_SHARED)
	  return FALSE;
	break;
      case NONE:
	break;
      }
    }
    return TRUE;
}

_X_EXPORT void
xf86RegisterStateChangeNotificationCallback(xf86StateChangeNotificationCallbackFunc func, pointer arg)
{
    StateChangeNotificationPtr ptr =
	(StateChangeNotificationPtr)xnfalloc(sizeof(StateChangeNotificationRec));

    ptr->func = func;
    ptr->arg = arg;
    ptr->next = StateChangeNotificationList;
    StateChangeNotificationList = ptr;
}

_X_EXPORT Bool
xf86DeregisterStateChangeNotificationCallback(xf86StateChangeNotificationCallbackFunc func)
{
    StateChangeNotificationPtr *ptr = &StateChangeNotificationList;
    StateChangeNotificationPtr tmp;
    
    while (*ptr) {
	if ((*ptr)->func == func) {
	    tmp = (*ptr);
	    (*ptr) = (*ptr)->next;
	    xfree(tmp);
	    return TRUE;
	}
	ptr = &((*ptr)->next);
    }
    return FALSE;
}

static void
notifyStateChange(xf86NotifyState state)
{
    StateChangeNotificationPtr ptr = StateChangeNotificationList;
    while (ptr) {
	ptr->func(state,ptr->arg);
	ptr = ptr->next;
    }
}

/* Multihead accel sharing accessor functions and entity Private handling */

_X_EXPORT int
xf86GetLastScrnFlag(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        return(xf86Entities[entityIndex]->lastScrnFlag);
    } else {
        return -1;
    }
}

_X_EXPORT void
xf86SetLastScrnFlag(int entityIndex, int scrnIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->lastScrnFlag = scrnIndex;
    }
}

_X_EXPORT Bool
xf86IsEntityShared(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        if(xf86Entities[entityIndex]->entityProp & IS_SHARED_ACCEL) {
	    return TRUE;
	}
    }
    return FALSE;
}

_X_EXPORT void
xf86SetEntityShared(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->entityProp |= IS_SHARED_ACCEL;
    }
}

_X_EXPORT Bool
xf86IsEntitySharable(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        if(xf86Entities[entityIndex]->entityProp & ACCEL_IS_SHARABLE) {
	    return TRUE;
	}
    }
    return FALSE;
}

_X_EXPORT void
xf86SetEntitySharable(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->entityProp |= ACCEL_IS_SHARABLE;
    }
}

_X_EXPORT Bool
xf86IsPrimInitDone(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        if(xf86Entities[entityIndex]->entityProp & SA_PRIM_INIT_DONE) {
	    return TRUE;
	}
    }
    return FALSE;
}

_X_EXPORT void
xf86SetPrimInitDone(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->entityProp |= SA_PRIM_INIT_DONE;
    }
}

_X_EXPORT void
xf86ClearPrimInitDone(int entityIndex)
{
    if(entityIndex < xf86NumEntities) {
        xf86Entities[entityIndex]->entityProp &= ~SA_PRIM_INIT_DONE;
    }
}


/*
 * Allocate a private in the entities.
 */

_X_EXPORT int
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

_X_EXPORT DevUnion *
xf86GetEntityPrivate(int entityIndex, int privIndex)
{
    if (entityIndex >= xf86NumEntities || privIndex >= xf86EntityPrivateCount)
	return NULL;

    return &(xf86Entities[entityIndex]->entityPrivates[privIndex]);
}

