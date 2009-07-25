/*

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  
DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN action OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include "misc.h"
#define NEED_EVENTS
#include <X11/Xproto.h>
#include "windowstr.h"
#include "inputstr.h"
#include "cursorstr.h"
#include "dixgrabs.h"
#include "xace.h"

#define BITMASK(i) (((Mask)1) << ((i) & 31))
#define MASKIDX(i) ((i) >> 5)
#define MASKWORD(buf, i) buf[MASKIDX(i)]
#define BITSET(buf, i) MASKWORD(buf, i) |= BITMASK(i)
#define BITCLEAR(buf, i) MASKWORD(buf, i) &= ~BITMASK(i)
#define GETBIT(buf, i) (MASKWORD(buf, i) & BITMASK(i))

GrabPtr
CreateGrab(
    int client,
    DeviceIntPtr device,
    WindowPtr window,
    Mask eventMask,
    Bool ownerEvents, Bool keyboardMode, Bool pointerMode,
    DeviceIntPtr modDevice,
    unsigned short modifiers,
    int type,
    KeyCode keybut,	/* key or button */
    WindowPtr confineTo,
    CursorPtr cursor)
{
    GrabPtr grab;

    grab = (GrabPtr)xalloc(sizeof(GrabRec));
    if (!grab)
	return (GrabPtr)NULL;
    grab->resource = FakeClientID(client);
    grab->device = device;
    grab->coreGrab = (type < LASTEvent);
    grab->window = window;
    grab->eventMask = eventMask;
    grab->deviceMask = 0;
    grab->ownerEvents = ownerEvents;
    grab->keyboardMode = keyboardMode;
    grab->pointerMode = pointerMode;
    grab->modifiersDetail.exact = modifiers;
    grab->modifiersDetail.pMask = NULL;
    grab->modifierDevice = modDevice;
    grab->coreMods = ((modDevice == inputInfo.keyboard) ||
		      (modDevice == inputInfo.pointer));
    grab->type = type;
    grab->detail.exact = keybut;
    grab->detail.pMask = NULL;
    grab->confineTo = confineTo;
    grab->cursor = cursor;
    grab->genericMasks = NULL;
    grab->next = NULL;
    if (cursor)
	cursor->refcnt++;
    return grab;

}

static void
FreeGrab(GrabPtr pGrab)
{
    if (pGrab->modifiersDetail.pMask != NULL)
	xfree(pGrab->modifiersDetail.pMask);

    if (pGrab->detail.pMask != NULL)
	xfree(pGrab->detail.pMask);

    if (pGrab->cursor)
	FreeCursor(pGrab->cursor, (Cursor)0);

    xfree(pGrab);
}

int
DeletePassiveGrab(pointer value, XID id)
{
    GrabPtr g, prev;
    GrabPtr pGrab = (GrabPtr)value;

    /* it is OK if the grab isn't found */
    prev = 0;
    for (g = (wPassiveGrabs (pGrab->window)); g; g = g->next)
    {
	if (pGrab == g)
	{
	    if (prev)
		prev->next = g->next;
	    else
		if (!(pGrab->window->optional->passiveGrabs = g->next))
		    CheckWindowOptionalNeed (pGrab->window);
	    break;
	}
	prev = g;
    }
    FreeGrab(pGrab);
    return Success;
}

static Mask *
DeleteDetailFromMask(Mask *pDetailMask, unsigned short detail)
{
    Mask *mask;
    int i;

    mask = (Mask *)xalloc(sizeof(Mask) * MasksPerDetailMask);
    if (mask)
    {
	if (pDetailMask)
	    for (i = 0; i < MasksPerDetailMask; i++)
		mask[i]= pDetailMask[i];
	else
	    for (i = 0; i < MasksPerDetailMask; i++)
		mask[i]= ~0L;
	BITCLEAR(mask, detail);
    }
    return mask; 
}

static Bool
IsInGrabMask(
    DetailRec firstDetail,
    DetailRec secondDetail,
    unsigned short exception)
{
    if (firstDetail.exact == exception)
    {
	if (firstDetail.pMask == NULL)
	    return TRUE;
	
	/* (at present) never called with two non-null pMasks */
	if (secondDetail.exact == exception)
	    return FALSE;

 	if (GETBIT(firstDetail.pMask, secondDetail.exact))
	    return TRUE;
    }
    
    return FALSE;
}

static Bool 
IdenticalExactDetails(
    unsigned short firstExact,
    unsigned short secondExact,
    unsigned short exception)
{
    if ((firstExact == exception) || (secondExact == exception))
	return FALSE;
   
    if (firstExact == secondExact)
	return TRUE;

    return FALSE;
}

static Bool 
DetailSupersedesSecond(
    DetailRec firstDetail,
    DetailRec secondDetail,
    unsigned short exception)
{
    if (IsInGrabMask(firstDetail, secondDetail, exception))
	return TRUE;

    if (IdenticalExactDetails(firstDetail.exact, secondDetail.exact,
			      exception))
	return TRUE;
  
    return FALSE;
}

static Bool
GrabSupersedesSecond(GrabPtr pFirstGrab, GrabPtr pSecondGrab)
{
    if (!DetailSupersedesSecond(pFirstGrab->modifiersDetail,
				pSecondGrab->modifiersDetail, 
				(unsigned short)AnyModifier))
	return FALSE;

    if (DetailSupersedesSecond(pFirstGrab->detail,
			       pSecondGrab->detail, (unsigned short)AnyKey))
	return TRUE;
 
    return FALSE;
}

/**
 * Compares two grabs and returns TRUE if the first grab matches the second
 * grab. 
 * 
 * A match is when 
 *  - the devices set for the grab are equal (this is optional).
 *  - the event types for both grabs are equal.
 *  - XXX
 *
 * @param ignoreDevice TRUE if the device settings on the grabs are to be
 * ignored.
 * @return TRUE if the grabs match or FALSE otherwise.
 */
Bool
GrabMatchesSecond(GrabPtr pFirstGrab, GrabPtr pSecondGrab, Bool ignoreDevice)
{
    if (!ignoreDevice &&
            ((pFirstGrab->device != pSecondGrab->device) ||
             (pFirstGrab->modifierDevice != pSecondGrab->modifierDevice)))
            return FALSE;

    if (pFirstGrab->type != pSecondGrab->type)
	return FALSE;

    if (GrabSupersedesSecond(pFirstGrab, pSecondGrab) ||
	GrabSupersedesSecond(pSecondGrab, pFirstGrab))
	return TRUE;
 
    if (DetailSupersedesSecond(pSecondGrab->detail, pFirstGrab->detail,
			       (unsigned short)AnyKey) 
	&& 
	DetailSupersedesSecond(pFirstGrab->modifiersDetail,
			       pSecondGrab->modifiersDetail,
			       (unsigned short)AnyModifier))
	return TRUE;

    if (DetailSupersedesSecond(pFirstGrab->detail, pSecondGrab->detail,
			       (unsigned short)AnyKey)
	&& 
	DetailSupersedesSecond(pSecondGrab->modifiersDetail,
			       pFirstGrab->modifiersDetail,
			       (unsigned short)AnyModifier))
	return TRUE;

    return FALSE;
}

static Bool
GrabsAreIdentical(GrabPtr pFirstGrab, GrabPtr pSecondGrab)
{
    if (pFirstGrab->device != pSecondGrab->device || 
	(pFirstGrab->modifierDevice != pSecondGrab->modifierDevice) ||
	(pFirstGrab->type != pSecondGrab->type))
	return FALSE;

    if (!(DetailSupersedesSecond(pFirstGrab->detail, 
                               pSecondGrab->detail, 
                               (unsigned short)AnyKey) && 
        DetailSupersedesSecond(pSecondGrab->detail,
                               pFirstGrab->detail,
                               (unsigned short)AnyKey)))
        return FALSE;

    if (!(DetailSupersedesSecond(pFirstGrab->modifiersDetail, 
                               pSecondGrab->modifiersDetail, 
                               (unsigned short)AnyModifier) && 
        DetailSupersedesSecond(pSecondGrab->modifiersDetail,
                               pFirstGrab->modifiersDetail,
                               (unsigned short)AnyModifier)))
        return FALSE;

    return TRUE;
}


/**
 * Prepend the new grab to the list of passive grabs on the window.
 * Any previously existing grab that matches the new grab will be removed.
 * Adding a new grab that would override another client's grab will result in
 * a BadAccess.
 * 
 * @return Success or X error code on failure.
 */
int
AddPassiveGrabToList(ClientPtr client, GrabPtr pGrab)
{
    GrabPtr grab;
    Mask access_mode = DixGrabAccess;
    int rc;

    for (grab = wPassiveGrabs(pGrab->window); grab; grab = grab->next)
    {
	if (GrabMatchesSecond(pGrab, grab, FALSE))
	{
	    if (CLIENT_BITS(pGrab->resource) != CLIENT_BITS(grab->resource))
	    {
		FreeGrab(pGrab);
		return BadAccess;
	    }
	}
    }

    if (pGrab->keyboardMode == GrabModeSync||pGrab->pointerMode == GrabModeSync)
	access_mode |= DixFreezeAccess;
    rc = XaceHook(XACE_DEVICE_ACCESS, client, pGrab->device, access_mode);
    if (rc != Success)
	return rc;

    /* Remove all grabs that match the new one exactly */
    for (grab = wPassiveGrabs(pGrab->window); grab; grab = grab->next)
    {
	if (GrabsAreIdentical(pGrab, grab))
	{
            DeletePassiveGrabFromList(grab);
            break;
	} 
    }

    if (!pGrab->window->optional && !MakeWindowOptional (pGrab->window))
    {
	FreeGrab(pGrab);
	return BadAlloc;
    }

    pGrab->next = pGrab->window->optional->passiveGrabs;
    pGrab->window->optional->passiveGrabs = pGrab;
    if (AddResource(pGrab->resource, RT_PASSIVEGRAB, (pointer)pGrab))
	return Success;
    return BadAlloc;
}

/* the following is kinda complicated, because we need to be able to back out
 * if any allocation fails
 */

Bool
DeletePassiveGrabFromList(GrabPtr pMinuendGrab)
{
    GrabPtr grab;
    GrabPtr *deletes, *adds;
    Mask ***updates, **details;
    int i, ndels, nadds, nups;
    Bool ok;

#define UPDATE(mask,exact) \
	if (!(details[nups] = DeleteDetailFromMask(mask, exact))) \
	  ok = FALSE; \
	else \
	  updates[nups++] = &(mask)

    i = 0;
    for (grab = wPassiveGrabs(pMinuendGrab->window); grab; grab = grab->next)
	i++;
    if (!i)
	return TRUE;
    deletes = (GrabPtr *)xalloc(i * sizeof(GrabPtr));
    adds = (GrabPtr *)xalloc(i * sizeof(GrabPtr));
    updates = (Mask ***)xalloc(i * sizeof(Mask **));
    details = (Mask **)xalloc(i * sizeof(Mask *));
    if (!deletes || !adds || !updates || !details)
    {
	if (details) xfree(details);
	if (updates) xfree(updates);
	if (adds) xfree(adds);
	if (deletes) xfree(deletes);
	return FALSE;
    }
    ndels = nadds = nups = 0;
    ok = TRUE;
    for (grab = wPassiveGrabs(pMinuendGrab->window);
	 grab && ok;
	 grab = grab->next)
    {
	if ((CLIENT_BITS(grab->resource) != CLIENT_BITS(pMinuendGrab->resource)) ||
	    !GrabMatchesSecond(grab, pMinuendGrab, (grab->coreGrab)))
	    continue;
	if (GrabSupersedesSecond(pMinuendGrab, grab))
	{
	    deletes[ndels++] = grab;
	}
	else if ((grab->detail.exact == AnyKey)
		 && (grab->modifiersDetail.exact != AnyModifier))
	{
	    UPDATE(grab->detail.pMask, pMinuendGrab->detail.exact);
	}
	else if ((grab->modifiersDetail.exact == AnyModifier) 
		 && (grab->detail.exact != AnyKey))
	{
	    UPDATE(grab->modifiersDetail.pMask,
		   pMinuendGrab->modifiersDetail.exact);
	}
	else if ((pMinuendGrab->detail.exact != AnyKey)
		 && (pMinuendGrab->modifiersDetail.exact != AnyModifier))
	{
	    GrabPtr pNewGrab;

	    UPDATE(grab->detail.pMask, pMinuendGrab->detail.exact);

	    pNewGrab = CreateGrab(CLIENT_ID(grab->resource), grab->device,
				  grab->window, (Mask)grab->eventMask,
				  (Bool)grab->ownerEvents,
				  (Bool)grab->keyboardMode,
				  (Bool)grab->pointerMode,
				  grab->modifierDevice,
				  AnyModifier, (int)grab->type,
				  pMinuendGrab->detail.exact,
				  grab->confineTo, grab->cursor);
	    if (!pNewGrab)
		ok = FALSE;
	    else if (!(pNewGrab->modifiersDetail.pMask =
		       DeleteDetailFromMask(grab->modifiersDetail.pMask,
					 pMinuendGrab->modifiersDetail.exact))
		     ||
		     (!pNewGrab->window->optional &&
		      !MakeWindowOptional(pNewGrab->window)))
	    {
		FreeGrab(pNewGrab);
		ok = FALSE;
	    }
	    else if (!AddResource(pNewGrab->resource, RT_PASSIVEGRAB,
				  (pointer)pNewGrab))
		ok = FALSE;
	    else
		adds[nadds++] = pNewGrab;
	}   
	else if (pMinuendGrab->detail.exact == AnyKey)
	{
	    UPDATE(grab->modifiersDetail.pMask,
		   pMinuendGrab->modifiersDetail.exact);
	}
	else
	{
	    UPDATE(grab->detail.pMask, pMinuendGrab->detail.exact);
	}
    }

    if (!ok)
    {
	for (i = 0; i < nadds; i++)
	    FreeResource(adds[i]->resource, RT_NONE);
	for (i = 0; i < nups; i++)
	    xfree(details[i]);
    }
    else
    {
	for (i = 0; i < ndels; i++)
	    FreeResource(deletes[i]->resource, RT_NONE);
	for (i = 0; i < nadds; i++)
	{
	    grab = adds[i];
	    grab->next = grab->window->optional->passiveGrabs;
	    grab->window->optional->passiveGrabs = grab;
	}
	for (i = 0; i < nups; i++)
	{
	    xfree(*updates[i]);
	    *updates[i] = details[i];
	}
    }
    xfree(details);
    xfree(updates);
    xfree(adds);
    xfree(deletes);
    return ok;

#undef UPDATE
}
