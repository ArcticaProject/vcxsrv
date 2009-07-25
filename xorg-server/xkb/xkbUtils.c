/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/*

Copyright © 2008 Red Hat Inc.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "os.h"
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#define	XK_CYRILLIC
#include <X11/keysym.h>
#include "misc.h"
#include "inputstr.h"

#define	XKBSRV_NEED_FILE_FUNCS
#include <xkbsrv.h>
#include "xkbgeom.h"
#include "xkb.h"

int	XkbDisableLockActions = 0;

/***====================================================================***/

int
_XkbLookupAnyDevice(DeviceIntPtr *pDev, int id, ClientPtr client,
		    Mask access_mode, int *xkb_err)
{
    int rc = XkbKeyboardErrorCode;

    if (id == XkbUseCoreKbd)
        id = PickKeyboard(client)->id;
    else if (id == XkbUseCorePtr)
        id = PickPointer(client)->id;

    rc = dixLookupDevice(pDev, id, client, access_mode);
    if (rc != Success)
	*xkb_err = XkbErr_BadDevice;

    return rc;
}

int
_XkbLookupKeyboard(DeviceIntPtr *pDev, int id, ClientPtr client,
		   Mask access_mode, int *xkb_err)
{
    DeviceIntPtr dev;
    int rc;

    if (id == XkbDfltXIId)
        id = XkbUseCoreKbd;

    rc = _XkbLookupAnyDevice(pDev, id, client, access_mode, xkb_err);
    if (rc != Success)
	return rc;

    dev = *pDev;
    if (!dev->key || !dev->key->xkbInfo) {
	*pDev = NULL;
	*xkb_err= XkbErr_BadClass;
	return XkbKeyboardErrorCode;
    }
    return Success;
}

int
_XkbLookupBellDevice(DeviceIntPtr *pDev, int id, ClientPtr client,
		     Mask access_mode, int *xkb_err)
{
    DeviceIntPtr dev;
    int rc;

    rc = _XkbLookupAnyDevice(pDev, id, client, access_mode, xkb_err);
    if (rc != Success)
	return rc;

    dev = *pDev;
    if (!dev->kbdfeed && !dev->bell) {
	*pDev = NULL;
	*xkb_err= XkbErr_BadClass;
	return XkbKeyboardErrorCode;
    }
    return Success;
}

int
_XkbLookupLedDevice(DeviceIntPtr *pDev, int id, ClientPtr client,
		    Mask access_mode, int *xkb_err)
{
    DeviceIntPtr dev;
    int rc;

    if (id == XkbDfltXIId)
        id = XkbUseCorePtr;

    rc = _XkbLookupAnyDevice(pDev, id, client, access_mode, xkb_err);
    if (rc != Success)
	return rc;

    dev = *pDev;
    if (!dev->kbdfeed && !dev->leds) {
	*pDev = NULL;
	*xkb_err= XkbErr_BadClass;
	return XkbKeyboardErrorCode;
    }
    return Success;
}

int
_XkbLookupButtonDevice(DeviceIntPtr *pDev, int id, ClientPtr client,
		       Mask access_mode, int *xkb_err)
{
    DeviceIntPtr dev;
    int rc;

    rc = _XkbLookupAnyDevice(pDev, id, client, access_mode, xkb_err);
    if (rc != Success)
	return rc;

    dev = *pDev;
    if (!dev->button) {
	*pDev = NULL;
	*xkb_err= XkbErr_BadClass;
	return XkbKeyboardErrorCode;
    }
    return Success;
}

void
XkbSetActionKeyMods(XkbDescPtr xkb,XkbAction *act,unsigned mods)
{
register unsigned	tmp;

    switch (act->type) {
	case XkbSA_SetMods: case XkbSA_LatchMods: case XkbSA_LockMods:
	    if (act->mods.flags&XkbSA_UseModMapMods)
		act->mods.real_mods= act->mods.mask= mods;
	    if ((tmp= XkbModActionVMods(&act->mods))!=0)
		act->mods.mask|= XkbMaskForVMask(xkb,tmp);
	    break;
	case XkbSA_ISOLock:
	    if (act->iso.flags&XkbSA_UseModMapMods)
		act->iso.real_mods= act->iso.mask= mods;
	    if ((tmp= XkbModActionVMods(&act->iso))!=0)
		act->iso.mask|= XkbMaskForVMask(xkb,tmp);
	    break;
    }
    return;
}

unsigned
XkbMaskForVMask(XkbDescPtr xkb,unsigned vmask)
{
register int i,bit;
register unsigned mask;
    
    for (mask=i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	if (vmask&bit)
	    mask|= xkb->server->vmods[i];
    }
    return mask;
}

/***====================================================================***/

void
XkbUpdateKeyTypesFromCore(	DeviceIntPtr	pXDev,
				KeyCode	 	first,
				CARD8	 	num,
				XkbChangesPtr	changes)
{
XkbDescPtr		xkb;
unsigned		key,nG,explicit;
KeySymsPtr		pCore;
int			types[XkbNumKbdGroups];
KeySym			tsyms[XkbMaxSymsPerKey],*syms;
XkbMapChangesPtr	mc;

    xkb= pXDev->key->xkbInfo->desc;
    if (first+num-1>xkb->max_key_code) {
	/* 1/12/95 (ef) -- XXX! should allow XKB structures to grow */
	num= xkb->max_key_code-first+1;
    }

    mc= (changes?(&changes->map):NULL);

    pCore= &pXDev->key->curKeySyms;
    syms= &pCore->map[(first-xkb->min_key_code)*pCore->mapWidth];
    for (key=first; key<(first+num); key++,syms+= pCore->mapWidth) {
        explicit= xkb->server->explicit[key]&XkbExplicitKeyTypesMask;
        types[XkbGroup1Index]= XkbKeyKeyTypeIndex(xkb,key,XkbGroup1Index);
        types[XkbGroup2Index]= XkbKeyKeyTypeIndex(xkb,key,XkbGroup2Index);
        types[XkbGroup3Index]= XkbKeyKeyTypeIndex(xkb,key,XkbGroup3Index);
        types[XkbGroup4Index]= XkbKeyKeyTypeIndex(xkb,key,XkbGroup4Index);
        nG= XkbKeyTypesForCoreSymbols(xkb,pCore->mapWidth,syms,explicit,types,
									tsyms);
	XkbChangeTypesOfKey(xkb,key,nG,XkbAllGroupsMask,types,mc);
	memcpy((char *)XkbKeySymsPtr(xkb,key),(char *)tsyms,
					XkbKeyNumSyms(xkb,key)*sizeof(KeySym));
    }
    if (changes->map.changed&XkbKeySymsMask) {
	CARD8 oldLast,newLast;
	oldLast = changes->map.first_key_sym+changes->map.num_key_syms-1;
	newLast = first+num-1;

	if (first<changes->map.first_key_sym)
	    changes->map.first_key_sym = first;
	if (oldLast>newLast)
	    newLast= oldLast;
	changes->map.num_key_syms = newLast-changes->map.first_key_sym+1;
    }
    else {
	changes->map.changed|= XkbKeySymsMask;
	changes->map.first_key_sym = first;
	changes->map.num_key_syms = num;
    }
    return;
}

void
XkbUpdateDescActions(	XkbDescPtr		xkb,
			KeyCode		 	first,
			CARD8		 	num,
			XkbChangesPtr	 	changes)
{
register unsigned	key;

    for (key=first;key<(first+num);key++) {
	XkbApplyCompatMapToKey(xkb,key,changes);
    }

    if (changes->map.changed&(XkbVirtualModMapMask|XkbModifierMapMask)) {
        unsigned char           newVMods[XkbNumVirtualMods];
        register  unsigned      bit,i;
        unsigned                present;

        bzero(newVMods,XkbNumVirtualMods);
        present= 0;
        for (key=xkb->min_key_code;key<=xkb->max_key_code;key++) {
            if (xkb->server->vmodmap[key]==0)
                continue;
            for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
                if (bit&xkb->server->vmodmap[key]) {
                    present|= bit;
                    newVMods[i]|= xkb->map->modmap[key];
                }
            }
        }
        for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
            if ((bit&present)&&(newVMods[i]!=xkb->server->vmods[i])) {
                changes->map.changed|= XkbVirtualModsMask;
                changes->map.vmods|= bit;
                xkb->server->vmods[i]= newVMods[i];
            }
        }
    }
    if (changes->map.changed&XkbVirtualModsMask)
        XkbApplyVirtualModChanges(xkb,changes->map.vmods,changes);

    if (changes->map.changed&XkbKeyActionsMask) {
	CARD8 oldLast,newLast;
	oldLast= changes->map.first_key_act+changes->map.num_key_acts-1;
	newLast = first+num-1;

	if (first<changes->map.first_key_act)
	    changes->map.first_key_act = first;
	if (newLast>oldLast)
	    newLast= oldLast;
	changes->map.num_key_acts= newLast-changes->map.first_key_act+1;
    }
    else {
	changes->map.changed|= XkbKeyActionsMask;
	changes->map.first_key_act = first;
	changes->map.num_key_acts = num;
    }
    return;
}

void
XkbUpdateActions(	DeviceIntPtr	 	pXDev,
			KeyCode		 	first,
			CARD8		 	num,
			XkbChangesPtr	 	changes,
			unsigned *	 	needChecksRtrn,
			XkbEventCausePtr	cause)
{
XkbSrvInfoPtr		xkbi;
XkbDescPtr		xkb;
CARD8 *			repeat;

    if (needChecksRtrn)
	*needChecksRtrn= 0;
    xkbi= pXDev->key->xkbInfo;
    xkb= xkbi->desc;
    repeat= xkb->ctrls->per_key_repeat;

    if (pXDev->kbdfeed)
	memcpy(repeat,pXDev->kbdfeed->ctrl.autoRepeats,32);

    XkbUpdateDescActions(xkb,first,num,changes);

    if ((pXDev->kbdfeed)&&
	(changes->ctrls.enabled_ctrls_changes&XkbPerKeyRepeatMask)) {
        memcpy(pXDev->kbdfeed->ctrl.autoRepeats,repeat, 32);
	(*pXDev->kbdfeed->CtrlProc)(pXDev, &pXDev->kbdfeed->ctrl);
    }
    return;
}

void
XkbUpdateCoreDescription(DeviceIntPtr keybd,Bool resize)
{
register int		key,tmp;
int			maxSymsPerKey,maxKeysPerMod;
int			first,last,firstCommon,lastCommon;
XkbDescPtr		xkb;
KeyClassPtr		keyc;
CARD8			keysPerMod[XkbNumModifiers];

    if (!keybd || !keybd->key || !keybd->key->xkbInfo)
	return;
    xkb= keybd->key->xkbInfo->desc;
    keyc= keybd->key;
    maxSymsPerKey= maxKeysPerMod= 0;
    bzero(keysPerMod,sizeof(keysPerMod));
    memcpy(keyc->modifierMap,xkb->map->modmap,xkb->max_key_code+1);
    if ((xkb->min_key_code==keyc->curKeySyms.minKeyCode)&&
	(xkb->max_key_code==keyc->curKeySyms.maxKeyCode)) {
	first= firstCommon= xkb->min_key_code;
	last= lastCommon= xkb->max_key_code;
    }
    else if (resize) {
	keyc->curKeySyms.minKeyCode= xkb->min_key_code;
	keyc->curKeySyms.maxKeyCode= xkb->max_key_code;
	tmp= keyc->curKeySyms.mapWidth*_XkbCoreNumKeys(keyc);
	keyc->curKeySyms.map= _XkbTypedRealloc(keyc->curKeySyms.map,tmp,KeySym);
	if (!keyc->curKeySyms.map)
	   FatalError("Couldn't allocate keysyms\n");
	first= firstCommon= xkb->min_key_code;
	last= lastCommon= xkb->max_key_code;
    }
    else {
	if (xkb->min_key_code<keyc->curKeySyms.minKeyCode) {
	    first= xkb->min_key_code;
	    firstCommon= keyc->curKeySyms.minKeyCode;
	}
	else {
	    firstCommon= xkb->min_key_code;
	    first= keyc->curKeySyms.minKeyCode;
	}
	if (xkb->max_key_code>keyc->curKeySyms.maxKeyCode) {
	    lastCommon= keyc->curKeySyms.maxKeyCode;
	    last= xkb->max_key_code;
	}
	else {
	    lastCommon= xkb->max_key_code;
	    last= keyc->curKeySyms.maxKeyCode;
	}
    }

    /* determine sizes */
    for (key=first;key<=last;key++) {
	if (XkbKeycodeInRange(xkb,key)) {
	    int	nGroups;
	    int	w;
	    nGroups= XkbKeyNumGroups(xkb,key);
	    tmp= 0;
	    if (nGroups>0) {
		if ((w=XkbKeyGroupWidth(xkb,key,XkbGroup1Index))<=2)
		     tmp+= 2;
		else tmp+= w + 2;
	    }
	    if (nGroups>1) {
                if (tmp <= 2) {
		     if ((w=XkbKeyGroupWidth(xkb,key,XkbGroup2Index))<2)
		          tmp+= 2;
		     else tmp+= w;
                } else {
                     if ((w=XkbKeyGroupWidth(xkb,key,XkbGroup2Index))>2)
                          tmp+= w - 2;
                }
	    }
	    if (nGroups>2)
		tmp+= XkbKeyGroupWidth(xkb,key,XkbGroup3Index);
	    if (nGroups>3)
		tmp+= XkbKeyGroupWidth(xkb,key,XkbGroup4Index);
	    if (tmp>maxSymsPerKey)
		maxSymsPerKey= tmp;
	}
	if (_XkbCoreKeycodeInRange(keyc,key)) {
	    if (keyc->modifierMap[key]!=0) {
		register unsigned bit,i,mask;
		mask= keyc->modifierMap[key];
		for (i=0,bit=1;i<XkbNumModifiers;i++,bit<<=1) {
		    if (mask&bit) {
			keysPerMod[i]++;
			if (keysPerMod[i]>maxKeysPerMod)
			    maxKeysPerMod= keysPerMod[i];
		    }
		}
	    }
	}
    }

    if (maxKeysPerMod>0) {
	tmp= maxKeysPerMod*XkbNumModifiers;
	if (keyc->modifierKeyMap==NULL)
	    keyc->modifierKeyMap= (KeyCode *)_XkbCalloc(1, tmp);
	else if (keyc->maxKeysPerModifier<maxKeysPerMod)
	    keyc->modifierKeyMap= (KeyCode *)_XkbRealloc(keyc->modifierKeyMap,tmp);
	if (keyc->modifierKeyMap==NULL)
	    FatalError("Couldn't allocate modifierKeyMap in UpdateCore\n");
	bzero(keyc->modifierKeyMap,tmp);
    }
    else if ((keyc->maxKeysPerModifier>0)&&(keyc->modifierKeyMap!=NULL)) {
	_XkbFree(keyc->modifierKeyMap);
	keyc->modifierKeyMap= NULL;
    }
    keyc->maxKeysPerModifier= maxKeysPerMod;

    if (maxSymsPerKey>0) {
	tmp= maxSymsPerKey*_XkbCoreNumKeys(keyc);
	keyc->curKeySyms.map= _XkbTypedRealloc(keyc->curKeySyms.map,tmp,KeySym);
	if (keyc->curKeySyms.map==NULL)
	    FatalError("Couldn't allocate symbols map in UpdateCore\n");
    }
    else if ((keyc->curKeySyms.mapWidth>0)&&(keyc->curKeySyms.map!=NULL)) {
	_XkbFree(keyc->curKeySyms.map);
	keyc->curKeySyms.map= NULL;
    }
    keyc->curKeySyms.mapWidth= maxSymsPerKey;

    bzero(keysPerMod,sizeof(keysPerMod));
    for (key=firstCommon;key<=lastCommon;key++) {
	if (keyc->curKeySyms.map!=NULL) {
	    KeySym *pCore,*pXKB;
	    unsigned nGroups,groupWidth,n,nOut;

	    nGroups= XkbKeyNumGroups(xkb,key);
	    n= (key-keyc->curKeySyms.minKeyCode)*maxSymsPerKey;
	    pCore= &keyc->curKeySyms.map[n];
	    bzero(pCore,maxSymsPerKey*sizeof(KeySym));
	    pXKB= XkbKeySymsPtr(xkb,key);
	    nOut= 2;
	    if (nGroups>0) {
		groupWidth= XkbKeyGroupWidth(xkb,key,XkbGroup1Index);
		if (groupWidth>0)	pCore[0]= pXKB[0];
		if (groupWidth>1)	pCore[1]= pXKB[1];
		for (n=2;n<groupWidth;n++) {
		    pCore[2+n]= pXKB[n];
		}
		if (groupWidth>2)
		    nOut= groupWidth;
	    }

	    /* See XKB Protocol Sec, Section 12.4.
	       A 1-group key with ABCDE on a 2 group keyboard must be
	       duplicated across all groups as ABABCDECDE.
	     */
	    if (nGroups == 1)
	    {
		int idx;

		groupWidth = XkbKeyGroupWidth(xkb, key, XkbGroup1Index);

		/* AB..CDE... -> ABABCDE... */
		if (groupWidth > 0 && maxSymsPerKey >= 3)
		    pCore[2] = pCore[0];
		if (groupWidth > 1 && maxSymsPerKey >= 4)
		    pCore[3] = pCore[1];

		/* ABABCDE... -> ABABCDECDE */
		idx = 2 + groupWidth;
		while (groupWidth > 2 &&
			idx < maxSymsPerKey &&
			idx < groupWidth * 2)
		{
		    pCore[idx] = pCore[idx - groupWidth + 2];
		    idx++;
		}
		idx = 2 * groupWidth;
		if (idx < 4)
		    idx = 4;
		/* 3 or more groups: ABABCDECDEABCDEABCDE */
		for (n = 0; n < groupWidth && idx < maxSymsPerKey; n++)
		    pCore[idx++] = pXKB[n];
	    }

	    pXKB+= XkbKeyGroupsWidth(xkb,key);
	    nOut+= 2;
	    if (nGroups>1) {
		groupWidth= XkbKeyGroupWidth(xkb,key,XkbGroup2Index);
		if (groupWidth>0)	pCore[2]= pXKB[0];
		if (groupWidth>1)	pCore[3]= pXKB[1];
		for (n=2;n<groupWidth;n++) {
		    pCore[nOut+(n-2)]= pXKB[n];
		}
		if (groupWidth>2)
		    nOut+= (groupWidth-2);
	    }
	    pXKB+= XkbKeyGroupsWidth(xkb,key);
	    for (n=XkbGroup3Index;n<nGroups;n++) {
		register int s;
		groupWidth= XkbKeyGroupWidth(xkb,key,n);
		for (s=0;s<groupWidth;s++) {
		    pCore[nOut++]= pXKB[s];
		}
		pXKB+= XkbKeyGroupsWidth(xkb,key);
	    }
	}
	if (keyc->modifierMap[key]!=0) {
	    register unsigned bit,i,mask;
	    mask= keyc->modifierMap[key];
	    for (i=0,bit=1;i<XkbNumModifiers;i++,bit<<=1) {
		if (mask&bit) {
		    tmp= i*maxKeysPerMod+keysPerMod[i];
		    keyc->modifierKeyMap[tmp]= key;
		    keysPerMod[i]++;
		}
	    }
	}
    }
    return;
}

void
XkbSetRepeatKeys(DeviceIntPtr pXDev,int key,int onoff)
{
    if (pXDev && pXDev->key && pXDev->key->xkbInfo) {
	xkbControlsNotify	cn;
	XkbControlsPtr		ctrls = pXDev->key->xkbInfo->desc->ctrls;
	XkbControlsRec 		old;
	old = *ctrls;

	if (key== -1) {	/* global autorepeat setting changed */
	    if (onoff)	ctrls->enabled_ctrls |= XkbRepeatKeysMask;
	    else	ctrls->enabled_ctrls &= ~XkbRepeatKeysMask;
	}
	else if (pXDev->kbdfeed) {
	    ctrls->per_key_repeat[key/8] = 
		pXDev->kbdfeed->ctrl.autoRepeats[key/8];
	}
	
	if (XkbComputeControlsNotify(pXDev,&old,ctrls,&cn,True))
	    XkbSendControlsNotify(pXDev,&cn);
    }
    return;
}

void
XkbApplyMappingChange(	DeviceIntPtr	kbd,
			CARD8		 request,
			KeyCode		 firstKey,
			CARD8		 num,
			ClientPtr	 client)
{
XkbEventCauseRec	cause;
XkbChangesRec	 	changes;
unsigned	 	check;

    if (kbd->key->xkbInfo==NULL)
	XkbInitDevice(kbd);
    bzero(&changes,sizeof(XkbChangesRec));
    check= 0;
    if (request==MappingKeyboard) {
	XkbSetCauseCoreReq(&cause,X_ChangeKeyboardMapping,client);
	XkbUpdateKeyTypesFromCore(kbd,firstKey,num,&changes);
	XkbUpdateActions(kbd,firstKey,num,&changes,&check,&cause);
	if (check)
	    XkbCheckSecondaryEffects(kbd->key->xkbInfo,check,&changes,&cause);
    }
    else if (request==MappingModifier) {
	XkbDescPtr	xkb= kbd->key->xkbInfo->desc;

	XkbSetCauseCoreReq(&cause,X_SetModifierMapping,client);

	num = xkb->max_key_code-xkb->min_key_code+1;
	memcpy(xkb->map->modmap,kbd->key->modifierMap,xkb->max_key_code+1);

	changes.map.changed|= XkbModifierMapMask;
	changes.map.first_modmap_key= xkb->min_key_code;
	changes.map.num_modmap_keys= num;
	XkbUpdateActions(kbd,xkb->min_key_code,num,&changes,&check,&cause);
	if (check)
	    XkbCheckSecondaryEffects(kbd->key->xkbInfo,check,&changes,&cause);
    }
    /* 3/26/94 (ef) -- XXX! Doesn't deal with input extension requests */
    XkbSendNotification(kbd,&changes,&cause);
    return;
}

void
XkbDisableComputedAutoRepeats(DeviceIntPtr dev,unsigned key)
{
XkbSrvInfoPtr	xkbi = dev->key->xkbInfo;
xkbMapNotify	mn;

    xkbi->desc->server->explicit[key]|= XkbExplicitAutoRepeatMask;
    bzero(&mn,sizeof(mn));
    mn.changed= XkbExplicitComponentsMask;
    mn.firstKeyExplicit= key;
    mn.nKeyExplicit= 1;
    XkbSendMapNotify(dev,&mn);
    return;
}

unsigned
XkbStateChangedFlags(XkbStatePtr old,XkbStatePtr new)
{
int		changed;

    changed=(old->group!=new->group?XkbGroupStateMask:0);
    changed|=(old->base_group!=new->base_group?XkbGroupBaseMask:0);
    changed|=(old->latched_group!=new->latched_group?XkbGroupLatchMask:0);
    changed|=(old->locked_group!=new->locked_group?XkbGroupLockMask:0);
    changed|=(old->mods!=new->mods?XkbModifierStateMask:0);
    changed|=(old->base_mods!=new->base_mods?XkbModifierBaseMask:0);
    changed|=(old->latched_mods!=new->latched_mods?XkbModifierLatchMask:0);
    changed|=(old->locked_mods!=new->locked_mods?XkbModifierLockMask:0);
    changed|=(old->compat_state!=new->compat_state?XkbCompatStateMask:0);
    changed|=(old->grab_mods!=new->grab_mods?XkbGrabModsMask:0);
    if (old->compat_grab_mods!=new->compat_grab_mods)
	changed|= XkbCompatGrabModsMask;
    changed|=(old->lookup_mods!=new->lookup_mods?XkbLookupModsMask:0);
    if (old->compat_lookup_mods!=new->compat_lookup_mods)
	changed|= XkbCompatLookupModsMask;
    changed|=(old->ptr_buttons!=new->ptr_buttons?XkbPointerButtonMask:0);
    return changed;
}

static void
XkbComputeCompatState(XkbSrvInfoPtr xkbi)
{
CARD16 		grp_mask;
XkbStatePtr	state= &xkbi->state;
XkbCompatMapPtr	map;

    if (!state || !xkbi->desc || !xkbi->desc->ctrls || !xkbi->desc->compat)
        return;

    map= xkbi->desc->compat;
    grp_mask= map->groups[state->group].mask;
    state->compat_state = state->mods|grp_mask;
    state->compat_lookup_mods= state->lookup_mods|grp_mask;

    if (xkbi->desc->ctrls->enabled_ctrls&XkbIgnoreGroupLockMask)
	 grp_mask= map->groups[state->base_group].mask;
    state->compat_grab_mods= state->grab_mods|grp_mask;
    return;
}

unsigned
XkbAdjustGroup(int group,XkbControlsPtr ctrls)
{
unsigned	act;

    act= XkbOutOfRangeGroupAction(ctrls->groups_wrap);
    if (group<0) {
	while ( group < 0 )  {
	    if (act==XkbClampIntoRange) {
		group= XkbGroup1Index;
	    }
	    else if (act==XkbRedirectIntoRange) {
		int newGroup;
		newGroup= XkbOutOfRangeGroupNumber(ctrls->groups_wrap);
		if (newGroup>=ctrls->num_groups)
		     group= XkbGroup1Index;
		else group= newGroup;
	    }
	    else {
		group+= ctrls->num_groups;
	    }
	}
    }
    else if (group>=ctrls->num_groups) {
	if (act==XkbClampIntoRange) {
	    group= ctrls->num_groups-1;
	}
	else if (act==XkbRedirectIntoRange) {
	    int newGroup;
	    newGroup= XkbOutOfRangeGroupNumber(ctrls->groups_wrap);
	    if (newGroup>=ctrls->num_groups)
		 group= XkbGroup1Index;
	    else group= newGroup;
	}
	else {
	    group%= ctrls->num_groups;
	}
    }
    return group;
}

void
XkbComputeDerivedState(XkbSrvInfoPtr xkbi)
{
XkbStatePtr	state= &xkbi->state;
XkbControlsPtr	ctrls= xkbi->desc->ctrls;
unsigned char	grp;

    if (!state || !ctrls)
        return;

    state->mods= (state->base_mods|state->latched_mods);
    state->mods|= state->locked_mods;
    state->lookup_mods= state->mods&(~ctrls->internal.mask);
    state->grab_mods= state->lookup_mods&(~ctrls->ignore_lock.mask);
    state->grab_mods|= 
	((state->base_mods|state->latched_mods)&ctrls->ignore_lock.mask);


    grp= state->locked_group;
    if (grp>=ctrls->num_groups)
	state->locked_group= XkbAdjustGroup(XkbCharToInt(grp),ctrls);

    grp= state->locked_group+state->base_group+state->latched_group;
    if (grp>=ctrls->num_groups)
	 state->group= XkbAdjustGroup(XkbCharToInt(grp),ctrls);
    else state->group= grp;
    XkbComputeCompatState(xkbi);
    return;
}

/***====================================================================***/

void
XkbCheckSecondaryEffects(	XkbSrvInfoPtr		xkbi,
				unsigned		which,
				XkbChangesPtr 		changes,
				XkbEventCausePtr	cause)
{
    if (which&XkbStateNotifyMask) {
	XkbStateRec old;
	old= xkbi->state;
	changes->state_changes|= XkbStateChangedFlags(&old,&xkbi->state);
	XkbComputeDerivedState(xkbi);
    }
    if (which&XkbIndicatorStateNotifyMask)
	XkbUpdateIndicators(xkbi->device,XkbAllIndicatorsMask,True,changes,
									cause);
    return;
}

/***====================================================================***/

Bool
XkbEnableDisableControls(	XkbSrvInfoPtr		xkbi,
				unsigned long		change,
				unsigned long		newValues,
				XkbChangesPtr		changes,
				XkbEventCausePtr	cause)
{
XkbControlsPtr		ctrls;
unsigned 		old;
XkbSrvLedInfoPtr	sli;

    ctrls= xkbi->desc->ctrls;
    old= ctrls->enabled_ctrls;
    ctrls->enabled_ctrls&= ~change;
    ctrls->enabled_ctrls|= (change&newValues);
    if (old==ctrls->enabled_ctrls)
	return False;
    if (cause!=NULL) {
	xkbControlsNotify cn;
	cn.numGroups= ctrls->num_groups;
	cn.changedControls|= XkbControlsEnabledMask;
	cn.enabledControls= ctrls->enabled_ctrls;
	cn.enabledControlChanges= (ctrls->enabled_ctrls^old);
	cn.keycode= cause->kc;
	cn.eventType= cause->event;
	cn.requestMajor= cause->mjr;
	cn.requestMinor= cause->mnr;
	XkbSendControlsNotify(xkbi->device,&cn);
    }
    else {
	/* Yes, this really should be an XOR.  If ctrls->enabled_ctrls_changes*/
	/* is non-zero, the controls in question changed already in "this" */
	/* request and this change merely undoes the previous one.  By the */
	/* same token, we have to figure out whether or not ControlsEnabled */
	/* should be set or not in the changes structure */
	changes->ctrls.enabled_ctrls_changes^= (ctrls->enabled_ctrls^old);
	if (changes->ctrls.enabled_ctrls_changes)
	     changes->ctrls.changed_ctrls|= XkbControlsEnabledMask;
	else changes->ctrls.changed_ctrls&= ~XkbControlsEnabledMask;
    }
    sli= XkbFindSrvLedInfo(xkbi->device,XkbDfltXIClass,XkbDfltXIId,0);
    XkbUpdateIndicators(xkbi->device,sli->usesControls,True,changes,cause);
    return True;
}

/***====================================================================***/

#define	MAX_TOC	16

XkbGeometryPtr 
XkbLookupNamedGeometry(DeviceIntPtr dev,Atom name,Bool *shouldFree)
{
XkbSrvInfoPtr	xkbi=	dev->key->xkbInfo;
XkbDescPtr	xkb=	xkbi->desc;

    *shouldFree= 0;
    if (name==None) {
	if (xkb->geom!=NULL)
	    return xkb->geom;
	name= xkb->names->geometry;
    }
    if ((xkb->geom!=NULL)&&(xkb->geom->name==name))
	return xkb->geom;
    *shouldFree= 1;
    return NULL;
}

void
XkbConvertCase(register KeySym sym, KeySym *lower, KeySym *upper)
{
    *lower = sym;
    *upper = sym;
    switch(sym >> 8) {
    case 0: /* Latin 1 */
	if ((sym >= XK_A) && (sym <= XK_Z))
	    *lower += (XK_a - XK_A);
	else if ((sym >= XK_a) && (sym <= XK_z))
	    *upper -= (XK_a - XK_A);
	else if ((sym >= XK_Agrave) && (sym <= XK_Odiaeresis))
	    *lower += (XK_agrave - XK_Agrave);
	else if ((sym >= XK_agrave) && (sym <= XK_odiaeresis))
	    *upper -= (XK_agrave - XK_Agrave);
	else if ((sym >= XK_Ooblique) && (sym <= XK_Thorn))
	    *lower += (XK_oslash - XK_Ooblique);
	else if ((sym >= XK_oslash) && (sym <= XK_thorn))
	    *upper -= (XK_oslash - XK_Ooblique);
	break;
    case 1: /* Latin 2 */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym == XK_Aogonek)
	    *lower = XK_aogonek;
	else if (sym >= XK_Lstroke && sym <= XK_Sacute)
	    *lower += (XK_lstroke - XK_Lstroke);
	else if (sym >= XK_Scaron && sym <= XK_Zacute)
	    *lower += (XK_scaron - XK_Scaron);
	else if (sym >= XK_Zcaron && sym <= XK_Zabovedot)
	    *lower += (XK_zcaron - XK_Zcaron);
	else if (sym == XK_aogonek)
	    *upper = XK_Aogonek;
	else if (sym >= XK_lstroke && sym <= XK_sacute)
	    *upper -= (XK_lstroke - XK_Lstroke);
	else if (sym >= XK_scaron && sym <= XK_zacute)
	    *upper -= (XK_scaron - XK_Scaron);
	else if (sym >= XK_zcaron && sym <= XK_zabovedot)
	    *upper -= (XK_zcaron - XK_Zcaron);
	else if (sym >= XK_Racute && sym <= XK_Tcedilla)
	    *lower += (XK_racute - XK_Racute);
	else if (sym >= XK_racute && sym <= XK_tcedilla)
	    *upper -= (XK_racute - XK_Racute);
	break;
    case 2: /* Latin 3 */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Hstroke && sym <= XK_Hcircumflex)
	    *lower += (XK_hstroke - XK_Hstroke);
	else if (sym >= XK_Gbreve && sym <= XK_Jcircumflex)
	    *lower += (XK_gbreve - XK_Gbreve);
	else if (sym >= XK_hstroke && sym <= XK_hcircumflex)
	    *upper -= (XK_hstroke - XK_Hstroke);
	else if (sym >= XK_gbreve && sym <= XK_jcircumflex)
	    *upper -= (XK_gbreve - XK_Gbreve);
	else if (sym >= XK_Cabovedot && sym <= XK_Scircumflex)
	    *lower += (XK_cabovedot - XK_Cabovedot);
	else if (sym >= XK_cabovedot && sym <= XK_scircumflex)
	    *upper -= (XK_cabovedot - XK_Cabovedot);
	break;
    case 3: /* Latin 4 */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Rcedilla && sym <= XK_Tslash)
	    *lower += (XK_rcedilla - XK_Rcedilla);
	else if (sym >= XK_rcedilla && sym <= XK_tslash)
	    *upper -= (XK_rcedilla - XK_Rcedilla);
	else if (sym == XK_ENG)
	    *lower = XK_eng;
	else if (sym == XK_eng)
	    *upper = XK_ENG;
	else if (sym >= XK_Amacron && sym <= XK_Umacron)
	    *lower += (XK_amacron - XK_Amacron);
	else if (sym >= XK_amacron && sym <= XK_umacron)
	    *upper -= (XK_amacron - XK_Amacron);
	break;
    case 6: /* Cyrillic */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Serbian_DJE && sym <= XK_Serbian_DZE)
	    *lower -= (XK_Serbian_DJE - XK_Serbian_dje);
	else if (sym >= XK_Serbian_dje && sym <= XK_Serbian_dze)
	    *upper += (XK_Serbian_DJE - XK_Serbian_dje);
	else if (sym >= XK_Cyrillic_YU && sym <= XK_Cyrillic_HARDSIGN)
	    *lower -= (XK_Cyrillic_YU - XK_Cyrillic_yu);
	else if (sym >= XK_Cyrillic_yu && sym <= XK_Cyrillic_hardsign)
	    *upper += (XK_Cyrillic_YU - XK_Cyrillic_yu);
        break;
    case 7: /* Greek */
	/* Assume the KeySym is a legal value (ignore discontinuities) */
	if (sym >= XK_Greek_ALPHAaccent && sym <= XK_Greek_OMEGAaccent)
	    *lower += (XK_Greek_alphaaccent - XK_Greek_ALPHAaccent);
	else if (sym >= XK_Greek_alphaaccent && sym <= XK_Greek_omegaaccent &&
		 sym != XK_Greek_iotaaccentdieresis &&
		 sym != XK_Greek_upsilonaccentdieresis)
	    *upper -= (XK_Greek_alphaaccent - XK_Greek_ALPHAaccent);
	else if (sym >= XK_Greek_ALPHA && sym <= XK_Greek_OMEGA)
	    *lower += (XK_Greek_alpha - XK_Greek_ALPHA);
	else if (sym >= XK_Greek_alpha && sym <= XK_Greek_omega &&
		 sym != XK_Greek_finalsmallsigma)
	    *upper -= (XK_Greek_alpha - XK_Greek_ALPHA);
        break;
    }
}

static Bool
_XkbCopyClientMap(XkbDescPtr src, XkbDescPtr dst)
{
    void *tmp = NULL;
    int i;
    XkbKeyTypePtr stype = NULL, dtype = NULL;

    /* client map */
    if (src->map) {
        if (!dst->map) {
            tmp = xcalloc(1, sizeof(XkbClientMapRec));
            if (!tmp)
                return FALSE;
            dst->map = tmp;
        }

        if (src->map->syms) {
            if (src->map->size_syms != dst->map->size_syms) {
                if (dst->map->syms)
                    tmp = xrealloc(dst->map->syms,
                                   src->map->size_syms * sizeof(KeySym));
                else
                    tmp = xalloc(src->map->size_syms * sizeof(KeySym));
                if (!tmp)
                    return FALSE;
                dst->map->syms = tmp;

            }
            memcpy(dst->map->syms, src->map->syms,
                   src->map->size_syms * sizeof(KeySym));
        }
        else {
            if (dst->map->syms) {
                xfree(dst->map->syms);
                dst->map->syms = NULL;
            }
        }
        dst->map->num_syms = src->map->num_syms;
        dst->map->size_syms = src->map->size_syms;

        if (src->map->key_sym_map) {
            if (src->max_key_code != dst->max_key_code) {
                if (dst->map->key_sym_map)
                    tmp = xrealloc(dst->map->key_sym_map,
                                   (src->max_key_code + 1) *
                                     sizeof(XkbSymMapRec));
                else
                    tmp = xalloc((src->max_key_code + 1) *
                                 sizeof(XkbSymMapRec));
                if (!tmp)
                    return FALSE;
                dst->map->key_sym_map = tmp;
            }
            memcpy(dst->map->key_sym_map, src->map->key_sym_map,
                   (src->max_key_code + 1) * sizeof(XkbSymMapRec));
        }
        else {
            if (dst->map->key_sym_map) {
                xfree(dst->map->key_sym_map);
                dst->map->key_sym_map = NULL;
            }
        }

        if (src->map->types && src->map->num_types) {
            if (src->map->num_types > dst->map->size_types ||
                !dst->map->types || !dst->map->size_types) {
                if (dst->map->types && dst->map->size_types) {
                    tmp = xrealloc(dst->map->types,
                                   src->map->num_types * sizeof(XkbKeyTypeRec));
                    if (!tmp)
                        return FALSE;
                    dst->map->types = tmp;
                    bzero(dst->map->types + dst->map->num_types,
                          (src->map->num_types - dst->map->num_types) *
                            sizeof(XkbKeyTypeRec));
                }
                else {
                    tmp = xcalloc(src->map->num_types, sizeof(XkbKeyTypeRec));
                    if (!tmp)
                        return FALSE;
                    dst->map->types = tmp;
                }
            }
            else if (src->map->num_types < dst->map->num_types &&
                     dst->map->types) {
                for (i = src->map->num_types, dtype = (dst->map->types + i);
                     i < dst->map->num_types; i++, dtype++) {
                    if (dtype->level_names)
                        xfree(dtype->level_names);
                    dtype->level_names = NULL;
                    dtype->num_levels = 0;
                    if (dtype->map_count) {
                        if (dtype->map)
                            xfree(dtype->map);
                        if (dtype->preserve)
                            xfree(dtype->preserve);
                    }
                }
            }

            stype = src->map->types;
            dtype = dst->map->types;
            for (i = 0; i < src->map->num_types; i++, dtype++, stype++) {
                if (stype->num_levels && stype->level_names) {
                    if (stype->num_levels != dtype->num_levels &&
                        dtype->num_levels && dtype->level_names &&
                        i < dst->map->num_types) {
                        tmp = xrealloc(dtype->level_names,
                                       stype->num_levels * sizeof(Atom));
                        if (!tmp)
                            continue;
                        dtype->level_names = tmp;
                    }
                    else if (!dtype->num_levels || !dtype->level_names ||
                             i >= dst->map->num_types) {
                        tmp = xalloc(stype->num_levels * sizeof(Atom));
                        if (!tmp)
                            continue;
                        dtype->level_names = tmp;
                    }
                    dtype->num_levels = stype->num_levels;
                    memcpy(dtype->level_names, stype->level_names,
                           stype->num_levels * sizeof(Atom));
                }
                else {
                    if (dtype->num_levels && dtype->level_names &&
                        i < dst->map->num_types)
                        xfree(dtype->level_names);
                    dtype->num_levels = 0;
                    dtype->level_names = NULL;
                }

                dtype->name = stype->name;
                memcpy(&dtype->mods, &stype->mods, sizeof(XkbModsRec));

                if (stype->map_count) {
                    if (stype->map) {
                        if (stype->map_count != dtype->map_count &&
                            dtype->map_count && dtype->map &&
                            i < dst->map->num_types) {
                            tmp = xrealloc(dtype->map,
                                           stype->map_count *
                                             sizeof(XkbKTMapEntryRec));
                            if (!tmp)
                                return FALSE;
                            dtype->map = tmp;
                        }
                        else if (!dtype->map_count || !dtype->map ||
                                 i >= dst->map->num_types) {
                            tmp = xalloc(stype->map_count *
                                           sizeof(XkbKTMapEntryRec));
                            if (!tmp)
                                return FALSE;
                            dtype->map = tmp;
                        }

                        memcpy(dtype->map, stype->map,
                               stype->map_count * sizeof(XkbKTMapEntryRec));
                    }
                    else {
                        if (dtype->map && i < dst->map->num_types)
                            xfree(dtype->map);
                        dtype->map = NULL;
                    }

                    if (stype->preserve) {
                        if (stype->map_count != dtype->map_count &&
                            dtype->map_count && dtype->preserve &&
                            i < dst->map->num_types) {
                            tmp = xrealloc(dtype->preserve,
                                           stype->map_count *
                                             sizeof(XkbModsRec));
                            if (!tmp)
                                return FALSE;
                            dtype->preserve = tmp;
                        }
                        else if (!dtype->preserve || !dtype->map_count ||
                                 i >= dst->map->num_types) {
                            tmp = xalloc(stype->map_count *
                                         sizeof(XkbModsRec));
                            if (!tmp)
                                return FALSE;
                            dtype->preserve = tmp;
                        }

                        memcpy(dtype->preserve, stype->preserve,
                               stype->map_count * sizeof(XkbModsRec));
                    }
                    else {
                        if (dtype->preserve && i < dst->map->num_types)
                            xfree(dtype->preserve);
                        dtype->preserve = NULL;
                    }

                    dtype->map_count = stype->map_count;
                }
                else {
                    if (dtype->map_count && i < dst->map->num_types) {
                        if (dtype->map)
                            xfree(dtype->map);
                        if (dtype->preserve)
                            xfree(dtype->preserve);
                    }
                    dtype->map_count = 0;
                    dtype->map = NULL;
                    dtype->preserve = NULL;
                }
            }

            dst->map->size_types = src->map->num_types;
            dst->map->num_types = src->map->num_types;
        }
        else {
            if (dst->map->types) {
                for (i = 0, dtype = dst->map->types; i < dst->map->num_types;
                     i++, dtype++) {
                    if (dtype->level_names)
                        xfree(dtype->level_names);
                    if (dtype->map && dtype->map_count)
                        xfree(dtype->map);
                    if (dtype->preserve && dtype->map_count)
                        xfree(dtype->preserve);
                }
                xfree(dst->map->types);
                dst->map->types = NULL;
            }
            dst->map->num_types = 0;
            dst->map->size_types = 0;
        }

        if (src->map->modmap) {
            if (src->max_key_code != dst->max_key_code) {
                if (dst->map->modmap)
                    tmp = xrealloc(dst->map->modmap, src->max_key_code + 1);
                else
                    tmp = xalloc(src->max_key_code + 1);
                if (!tmp)
                    return FALSE;
                dst->map->modmap = tmp;
            }
            memcpy(dst->map->modmap, src->map->modmap, src->max_key_code + 1);
        }
        else {
            if (dst->map->modmap) {
                xfree(dst->map->modmap);
                dst->map->modmap = NULL;
            }
        }
    }
    else {
        if (dst->map)
            XkbFreeClientMap(dst, XkbAllClientInfoMask, True);
    }

    return TRUE;
}

static Bool
_XkbCopyServerMap(XkbDescPtr src, XkbDescPtr dst)
{
    void *tmp = NULL;

    /* server map */
    if (src->server) {
        if (!dst->server) {
            tmp = xcalloc(1, sizeof(XkbServerMapRec));
            if (!tmp)
                return FALSE;
            dst->server = tmp;
        }

        if (src->server->explicit) {
            if (src->max_key_code != dst->max_key_code) {
                if (dst->server->explicit)
                    tmp = xrealloc(dst->server->explicit, src->max_key_code + 1);
                else
                    tmp = xalloc(src->max_key_code + 1);
                if (!tmp)
                    return FALSE;
                dst->server->explicit = tmp;
            }
            memcpy(dst->server->explicit, src->server->explicit,
                   src->max_key_code + 1);
        }
        else {
            if (dst->server->explicit) {
                xfree(dst->server->explicit);
                dst->server->explicit = NULL;
            }
        }

        if (src->server->acts) {
            if (src->server->size_acts != dst->server->size_acts) {
                if (dst->server->acts)
                    tmp = xrealloc(dst->server->acts,
                                   src->server->size_acts * sizeof(XkbAction));
                else
                    tmp = xalloc(src->server->size_acts * sizeof(XkbAction));
                if (!tmp)
                    return FALSE;
                dst->server->acts = tmp;
            }
            memcpy(dst->server->acts, src->server->acts,
                   src->server->size_acts * sizeof(XkbAction));
        }
        else {
            if (dst->server->acts) {
                xfree(dst->server->acts);
                dst->server->acts = NULL;
            }
        }
       dst->server->size_acts = src->server->size_acts;
       dst->server->num_acts = src->server->num_acts;

        if (src->server->key_acts) {
            if (src->max_key_code != dst->max_key_code) {
                if (dst->server->key_acts)
                    tmp = xrealloc(dst->server->key_acts,
                                   (src->max_key_code + 1) *
                                     sizeof(unsigned short));
                else
                    tmp = xalloc((src->max_key_code + 1) *
                                 sizeof(unsigned short));
                if (!tmp)
                    return FALSE;
                dst->server->key_acts = tmp;
            }
            memcpy(dst->server->key_acts, src->server->key_acts,
                   (src->max_key_code + 1) * sizeof(unsigned short));
        }
        else {
            if (dst->server->key_acts) {
                xfree(dst->server->key_acts);
                dst->server->key_acts = NULL;
            }
        }

        if (src->server->behaviors) {
            if (src->max_key_code != dst->max_key_code) {
                if (dst->server->behaviors)
                    tmp = xrealloc(dst->server->behaviors,
                                   (src->max_key_code + 1) *
                                   sizeof(XkbBehavior));
                else
                    tmp = xalloc((src->max_key_code + 1) *
                                 sizeof(XkbBehavior));
                if (!tmp)
                    return FALSE;
                dst->server->behaviors = tmp;
            }
            memcpy(dst->server->behaviors, src->server->behaviors,
                   (src->max_key_code + 1) * sizeof(XkbBehavior));
        }
        else {
            if (dst->server->behaviors) {
                xfree(dst->server->behaviors);
                dst->server->behaviors = NULL;
            }
        }

        memcpy(dst->server->vmods, src->server->vmods, XkbNumVirtualMods);

        if (src->server->vmodmap) {
            if (src->max_key_code != dst->max_key_code) {
                if (dst->server->vmodmap)
                    tmp = xrealloc(dst->server->vmodmap,
                                   (src->max_key_code + 1) *
                                   sizeof(unsigned short));
                else
                    tmp = xalloc((src->max_key_code + 1) *
                                 sizeof(unsigned short));
                if (!tmp)
                    return FALSE;
                dst->server->vmodmap = tmp;
            }
            memcpy(dst->server->vmodmap, src->server->vmodmap,
                   (src->max_key_code + 1) * sizeof(unsigned short));
        }
        else {
            if (dst->server->vmodmap) {
                xfree(dst->server->vmodmap);
                dst->server->vmodmap = NULL;
            }
        }
    }
    else {
        if (dst->server)
            XkbFreeServerMap(dst, XkbAllServerInfoMask, True);
    }

    return TRUE;
}

static Bool
_XkbCopyNames(XkbDescPtr src, XkbDescPtr dst)
{
    void *tmp = NULL;

    /* names */
    if (src->names) {
        if (!dst->names) {
            dst->names = xcalloc(1, sizeof(XkbNamesRec));
            if (!dst->names)
                return FALSE;
        }

        if (src->names->keys) {
            if (src->max_key_code != dst->max_key_code) {
                if (dst->names->keys)
                    tmp = xrealloc(dst->names->keys, (src->max_key_code + 1) *
                                   sizeof(XkbKeyNameRec));
                else
                    tmp = xalloc((src->max_key_code + 1) *
                                 sizeof(XkbKeyNameRec));
                if (!tmp)
                    return FALSE;
                dst->names->keys = tmp;
            }
            memcpy(dst->names->keys, src->names->keys,
                   (src->max_key_code + 1) * sizeof(XkbKeyNameRec));
        }
        else {
            if (dst->names->keys) {
                xfree(dst->names->keys);
                dst->names->keys = NULL;
            }
        }

        if (src->names->num_key_aliases) {
            if (src->names->num_key_aliases != dst->names->num_key_aliases) {
                if (dst->names->key_aliases)
                    tmp = xrealloc(dst->names->key_aliases,
                                   src->names->num_key_aliases *
                                     sizeof(XkbKeyAliasRec));
                else
                    tmp = xalloc(src->names->num_key_aliases *
                                 sizeof(XkbKeyAliasRec));
                if (!tmp)
                    return FALSE;
                dst->names->key_aliases = tmp;
            }
            memcpy(dst->names->key_aliases, src->names->key_aliases,
                   src->names->num_key_aliases * sizeof(XkbKeyAliasRec));
        }
        else {
            if (dst->names->key_aliases) {
                xfree(dst->names->key_aliases);
                dst->names->key_aliases = NULL;
            }
        }
        dst->names->num_key_aliases = src->names->num_key_aliases;

        if (src->names->num_rg) {
            if (src->names->num_rg != dst->names->num_rg) {
                if (dst->names->radio_groups)
                    tmp = xrealloc(dst->names->radio_groups,
                                   src->names->num_rg * sizeof(Atom));
                else
                    tmp = xalloc(src->names->num_rg * sizeof(Atom));
                if (!tmp)
                    return FALSE;
                dst->names->radio_groups = tmp;
            }
            memcpy(dst->names->radio_groups, src->names->radio_groups,
                   src->names->num_rg * sizeof(Atom));
        }
        else {
            if (dst->names->radio_groups)
                xfree(dst->names->radio_groups);
        }
        dst->names->num_rg = src->names->num_rg;

        dst->names->keycodes = src->names->keycodes;
        dst->names->geometry = src->names->geometry;
        dst->names->symbols = src->names->symbols;
        dst->names->types = src->names->types;
        dst->names->compat = src->names->compat;
        dst->names->phys_symbols = src->names->phys_symbols;

        memcpy(dst->names->vmods, src->names->vmods,
               XkbNumVirtualMods * sizeof(Atom));
        memcpy(dst->names->indicators, src->names->indicators,
               XkbNumIndicators * sizeof(Atom));
        memcpy(dst->names->groups, src->names->groups,
               XkbNumKbdGroups * sizeof(Atom));
    }
    else {
        if (dst->names)
            XkbFreeNames(dst, XkbAllNamesMask, True);
    }

    return TRUE;
}

static Bool
_XkbCopyCompat(XkbDescPtr src, XkbDescPtr dst)
{
    void *tmp = NULL;

    /* compat */
    if (src->compat) {
        if (!dst->compat) {
            dst->compat = xcalloc(1, sizeof(XkbCompatMapRec));
            if (!dst->compat)
                return FALSE;
        }

        if (src->compat->sym_interpret && src->compat->num_si) {
            if (src->compat->num_si != dst->compat->size_si) {
                if (dst->compat->sym_interpret)
                    tmp = xrealloc(dst->compat->sym_interpret,
                                   src->compat->num_si *
                                     sizeof(XkbSymInterpretRec));
                else
                    tmp = xalloc(src->compat->num_si *
                                 sizeof(XkbSymInterpretRec));
                if (!tmp)
                    return FALSE;
                dst->compat->sym_interpret = tmp;
            }
            memcpy(dst->compat->sym_interpret, src->compat->sym_interpret,
                   src->compat->num_si * sizeof(XkbSymInterpretRec));

            dst->compat->num_si = src->compat->num_si;
            dst->compat->size_si = src->compat->num_si;
        }
        else {
            if (dst->compat->sym_interpret && dst->compat->size_si)
                xfree(dst->compat->sym_interpret);

            dst->compat->sym_interpret = NULL;
            dst->compat->num_si = 0;
            dst->compat->size_si = 0;
        }

        memcpy(dst->compat->groups, src->compat->groups,
               XkbNumKbdGroups * sizeof(XkbModsRec));
    }
    else {
        if (dst->compat)
            XkbFreeCompatMap(dst, XkbAllCompatMask, True);
    }

    return TRUE;
}

static Bool
_XkbCopyGeom(XkbDescPtr src, XkbDescPtr dst)
{
    void *tmp = NULL;
    int i = 0, j = 0, k = 0;
    XkbColorPtr scolor = NULL, dcolor = NULL;
    XkbDoodadPtr sdoodad = NULL, ddoodad = NULL;
    XkbOutlinePtr soutline = NULL, doutline = NULL;
    XkbPropertyPtr sprop = NULL, dprop = NULL;
    XkbRowPtr srow = NULL, drow = NULL;
    XkbSectionPtr ssection = NULL, dsection = NULL;
    XkbShapePtr sshape = NULL, dshape = NULL;

    /* geometry */
    if (src->geom) {
        if (!dst->geom) {
            dst->geom = xcalloc(sizeof(XkbGeometryRec), 1);
            if (!dst->geom)
                return FALSE;
        }

        /* properties */
        if (src->geom->num_properties) {
            if (src->geom->num_properties != dst->geom->sz_properties) {
                /* If we've got more properties in the destination than
                 * the source, run through and free all the excess ones
                 * first. */
                if (src->geom->num_properties < dst->geom->sz_properties) {
                    for (i = src->geom->num_properties,
                         dprop = dst->geom->properties + i;
                         i < dst->geom->num_properties;
                         i++, dprop++) {
                        xfree(dprop->name);
                        xfree(dprop->value);
                    }
                }

                if (dst->geom->sz_properties)
                    tmp = xrealloc(dst->geom->properties,
                                   src->geom->num_properties *
                                    sizeof(XkbPropertyRec));
                else
                    tmp = xalloc(src->geom->num_properties *
                                  sizeof(XkbPropertyRec));
                if (!tmp)
                    return FALSE;
                dst->geom->properties = tmp;
            }

            /* We don't set num_properties as we need it to try and avoid
             * too much reallocing. */
            dst->geom->sz_properties = src->geom->num_properties;

            if (dst->geom->sz_properties > dst->geom->num_properties) {
                bzero(dst->geom->properties + dst->geom->num_properties,
                      (dst->geom->sz_properties - dst->geom->num_properties) *
                      sizeof(XkbPropertyRec));
            }

            for (i = 0,
                  sprop = src->geom->properties,
                  dprop = dst->geom->properties;
                 i < src->geom->num_properties;
                 i++, sprop++, dprop++) {
                if (i < dst->geom->num_properties) {
                    if (strlen(sprop->name) != strlen(dprop->name)) {
                        tmp = xrealloc(dprop->name, strlen(sprop->name) + 1);
                        if (!tmp)
                            return FALSE;
                        dprop->name = tmp;
                    }
                    if (strlen(sprop->value) != strlen(dprop->value)) {
                        tmp = xrealloc(dprop->value, strlen(sprop->value) + 1);
                        if (!tmp)
                            return FALSE;
                        dprop->value = tmp;
                    }
                    strcpy(dprop->name, sprop->name);
                    strcpy(dprop->value, sprop->value);
                }
                else {
                    dprop->name = xstrdup(sprop->name);
                    dprop->value = xstrdup(sprop->value);
                }
            }

            /* ... which is already src->geom->num_properties. */
            dst->geom->num_properties = dst->geom->sz_properties;
        }
        else {
            if (dst->geom->sz_properties) {
                for (i = 0, dprop = dst->geom->properties;
                     i < dst->geom->num_properties;
                     i++, dprop++) {
                    xfree(dprop->name);
                    xfree(dprop->value);
                }
                xfree(dst->geom->properties);
                dst->geom->properties = NULL;
            }

            dst->geom->num_properties = 0;
            dst->geom->sz_properties = 0;
        }

        /* colors */
        if (src->geom->num_colors) {
            if (src->geom->num_colors != dst->geom->sz_colors) {
                if (src->geom->num_colors < dst->geom->sz_colors) {
                    for (i = src->geom->num_colors,
                         dcolor = dst->geom->colors + i;
                         i < dst->geom->num_colors;
                         i++, dcolor++) {
                        xfree(dcolor->spec);
                    }
                }

                if (dst->geom->sz_colors)
                    tmp = xrealloc(dst->geom->colors,
                                   src->geom->num_colors *
                                    sizeof(XkbColorRec));
                else
                    tmp = xalloc(src->geom->num_colors *
                                  sizeof(XkbColorRec));
                if (!tmp)
                    return FALSE;
                dst->geom->colors = tmp;
            }

            dst->geom->sz_colors = src->geom->num_colors;

            if (dst->geom->sz_colors > dst->geom->num_colors) {
                bzero(dst->geom->colors + dst->geom->num_colors,
                      (dst->geom->sz_colors - dst->geom->num_colors) *
                      sizeof(XkbColorRec));
            }

            for (i = 0,
                  scolor = src->geom->colors,
                  dcolor = dst->geom->colors;
                 i < src->geom->num_colors;
                 i++, scolor++, dcolor++) {
                if (i < dst->geom->num_colors) {
                    if (strlen(scolor->spec) != strlen(dcolor->spec)) {
                        tmp = xrealloc(dcolor->spec, strlen(scolor->spec) + 1);
                        if (!tmp)
                            return FALSE;
                        dcolor->spec = tmp;
                    }
                    strcpy(dcolor->spec, scolor->spec);
                }
                else {
                    dcolor->spec = xstrdup(scolor->spec);
                }
            }

            dst->geom->num_colors = dst->geom->sz_colors;
        }
        else {
            if (dst->geom->sz_colors) {
                for (i = 0, dcolor = dst->geom->colors;
                     i < dst->geom->num_colors;
                     i++, dcolor++) {
                    xfree(dcolor->spec);
                }
                xfree(dst->geom->colors);
                dst->geom->colors = NULL;
            }

            dst->geom->num_colors = 0;
            dst->geom->sz_colors = 0;
        }

        /* shapes */
        /* shapes break down into outlines, which break down into points. */
        if (dst->geom->num_shapes) {
            for (i = 0, dshape = dst->geom->shapes;
                 i < dst->geom->num_shapes;
                 i++, dshape++) {
                for (j = 0, doutline = dshape->outlines;
                     j < dshape->num_outlines;
                     j++, doutline++) {
                    if (doutline->sz_points)
                        xfree(doutline->points);
                }

                if (dshape->sz_outlines) {
                    xfree(dshape->outlines);
                    dshape->outlines = NULL;
                }

                dshape->num_outlines = 0;
                dshape->sz_outlines = 0;
            }
        }

        if (src->geom->num_shapes) {
            tmp = xcalloc(src->geom->num_shapes, sizeof(XkbShapeRec));
            if (!tmp)
                return FALSE;
            dst->geom->shapes = tmp;

            for (i = 0, sshape = src->geom->shapes, dshape = dst->geom->shapes;
                 i < src->geom->num_shapes;
                 i++, sshape++, dshape++) {
                if (sshape->num_outlines) {
                    tmp = xcalloc(sshape->num_outlines, sizeof(XkbOutlineRec));
                    if (!tmp)
                        return FALSE;
                    dshape->outlines = tmp;
                    
                    for (j = 0,
                          soutline = sshape->outlines,
                          doutline = dshape->outlines;
                         j < sshape->num_outlines;
                         j++, soutline++, doutline++) {
                        if (soutline->num_points) {
                            tmp = xalloc(soutline->num_points *
                                          sizeof(XkbPointRec));
                            if (!tmp)
                                return FALSE;
                            doutline->points = tmp;

                            memcpy(doutline->points, soutline->points,
                                   soutline->num_points * sizeof(XkbPointRec));
                        }

                        doutline->num_points = soutline->num_points;
                        doutline->sz_points = soutline->num_points;
                    }
                }

                dshape->num_outlines = sshape->num_outlines;
                dshape->sz_outlines = sshape->num_outlines;
            }

            dst->geom->num_shapes = src->geom->num_shapes;
            dst->geom->sz_shapes = src->geom->num_shapes;
        }
        else {
            if (dst->geom->sz_shapes) {
                xfree(dst->geom->shapes);
            }
            dst->geom->shapes = NULL;
            dst->geom->num_shapes = 0;
            dst->geom->sz_shapes = 0;
        }

        /* sections */
        /* sections break down into doodads, and also into rows, which break
         * down into keys. */
        if (dst->geom->num_sections) {
            for (i = 0, dsection = dst->geom->sections;
                 i < dst->geom->num_sections;
                 i++, dsection++) {
                for (j = 0, drow = dsection->rows;
                     j < dsection->num_rows;
                     j++, drow++) {
                    if (drow->num_keys)
                        xfree(drow->keys);
                }

                if (dsection->num_rows)
                    xfree(dsection->rows);

                /* cut and waste from geom/doodad below. */
                for (j = 0, ddoodad = dsection->doodads;
                     j < dsection->num_doodads;
                     j++, ddoodad++) {
                    if (ddoodad->any.type == XkbTextDoodad) {
                        if (ddoodad->text.text) {
                            xfree(ddoodad->text.text);
                            ddoodad->text.text = NULL;
                        }
                        if (ddoodad->text.font) {
                            xfree(ddoodad->text.font);
                            ddoodad->text.font = NULL;
                        }
                     }
                     else if (ddoodad->any.type == XkbLogoDoodad) {
                        if (ddoodad->logo.logo_name) {
                            xfree(ddoodad->logo.logo_name);
                            ddoodad->logo.logo_name = NULL;
                        }
                    }
                }

                if (dsection->num_doodads)
                    xfree(dsection->doodads);
            }

            dst->geom->num_sections = 0;
            dst->geom->sections = NULL;
        }

        if (src->geom->num_sections) {
            if (dst->geom->sz_sections)
                tmp = xrealloc(dst->geom->sections,
                               src->geom->num_sections *
                                sizeof(XkbSectionRec));
            else
                tmp = xalloc(src->geom->num_sections * sizeof(XkbSectionRec));
            if (!tmp)
                return FALSE;
            memset(tmp, 0, src->geom->num_sections * sizeof(XkbSectionRec));
            dst->geom->sections = tmp;
            dst->geom->num_sections = src->geom->num_sections;
            dst->geom->sz_sections = src->geom->num_sections;

            for (i = 0,
                  ssection = src->geom->sections,
                  dsection = dst->geom->sections;
                 i < src->geom->num_sections;
                 i++, ssection++, dsection++) {
                *dsection = *ssection;
                if (ssection->num_rows) {
                    tmp = xcalloc(ssection->num_rows, sizeof(XkbRowRec));
                    if (!tmp)
                        return FALSE;
                    dsection->rows = tmp;
                }
                dsection->num_rows = ssection->num_rows;
                dsection->sz_rows = ssection->num_rows;

                for (j = 0, srow = ssection->rows, drow = dsection->rows;
                     j < ssection->num_rows;
                     j++, srow++, drow++) {
                    if (srow->num_keys) {
                        tmp = xalloc(srow->num_keys * sizeof(XkbKeyRec));
                        if (!tmp)
                            return FALSE;
                        drow->keys = tmp;
                        memcpy(drow->keys, srow->keys,
                               srow->num_keys * sizeof(XkbKeyRec));
                    }
                    drow->num_keys = srow->num_keys;
                    drow->sz_keys = srow->num_keys;
                }

                if (ssection->num_doodads) {
                    tmp = xcalloc(ssection->num_doodads, sizeof(XkbDoodadRec));
                    if (!tmp)
                        return FALSE;
                    dsection->doodads = tmp;
                }
                else {
                    dsection->doodads = NULL;
                }

                dsection->sz_doodads = ssection->num_doodads;
                for (k = 0,
                      sdoodad = ssection->doodads,
                      ddoodad = dsection->doodads;
                     k < ssection->num_doodads;
                     k++, sdoodad++, ddoodad++) {
                    if (sdoodad->any.type == XkbTextDoodad) {
                        if (sdoodad->text.text)
                            ddoodad->text.text =
                             xstrdup(sdoodad->text.text);
                        if (sdoodad->text.font)
                            ddoodad->text.font =
                             xstrdup(sdoodad->text.font);
                    }
                    else if (sdoodad->any.type == XkbLogoDoodad) {
                        if (sdoodad->logo.logo_name)
                            ddoodad->logo.logo_name =
                             xstrdup(sdoodad->logo.logo_name);
                    }
                    ddoodad->any.type = sdoodad->any.type;
                }
                dsection->overlays = NULL;
                dsection->sz_overlays = 0;
                dsection->num_overlays = 0;
            }
        }
        else {
            if (dst->geom->sz_sections) {
                xfree(dst->geom->sections);
            }

            dst->geom->sections = NULL;
            dst->geom->num_sections = 0;
            dst->geom->sz_sections = 0;
        }

        /* doodads */
        if (dst->geom->num_doodads) {
            for (i = src->geom->num_doodads,
                  ddoodad = dst->geom->doodads +
                             src->geom->num_doodads;
                 i < dst->geom->num_doodads;
                 i++, ddoodad++) {
                 if (ddoodad->any.type == XkbTextDoodad) {
                    if (ddoodad->text.text) {
                        xfree(ddoodad->text.text);
                        ddoodad->text.text = NULL;
                    }
                    if (ddoodad->text.font) {
                        xfree(ddoodad->text.font);
                        ddoodad->text.font = NULL;
                    }
                 }
                 else if (ddoodad->any.type == XkbLogoDoodad) {
                    if (ddoodad->logo.logo_name) {
                        xfree(ddoodad->logo.logo_name);
                        ddoodad->logo.logo_name = NULL;
                    }
                }
            }
            dst->geom->num_doodads = 0;
            dst->geom->doodads = NULL;
        }

        if (src->geom->num_doodads) {
            if (dst->geom->sz_doodads)
                tmp = xrealloc(dst->geom->doodads,
                               src->geom->num_doodads *
                                sizeof(XkbDoodadRec));
            else
                tmp = xalloc(src->geom->num_doodads *
                              sizeof(XkbDoodadRec));
            if (!tmp)
                return FALSE;
            memset(tmp, 0, src->geom->num_doodads * sizeof(XkbDoodadRec));
            dst->geom->doodads = tmp;

            dst->geom->sz_doodads = src->geom->num_doodads;

            for (i = 0,
                  sdoodad = src->geom->doodads,
                  ddoodad = dst->geom->doodads;
                 i < src->geom->num_doodads;
                 i++, sdoodad++, ddoodad++) {
                ddoodad->any.type = sdoodad->any.type;
                if (sdoodad->any.type == XkbTextDoodad) {
                    if (sdoodad->text.text)
                        ddoodad->text.text = xstrdup(sdoodad->text.text);
                    if (sdoodad->text.font)
                        ddoodad->text.font = xstrdup(sdoodad->text.font);
                }
                else if (sdoodad->any.type == XkbLogoDoodad) {
                    if (sdoodad->logo.logo_name)
                        ddoodad->logo.logo_name =
                          xstrdup(sdoodad->logo.logo_name);
                }
            }

            dst->geom->num_doodads = dst->geom->sz_doodads;
        }
        else {
            if (dst->geom->sz_doodads) {
                xfree(dst->geom->doodads);
            }

            dst->geom->doodads = NULL;
            dst->geom->num_doodads = 0;
            dst->geom->sz_doodads = 0;
        }

        /* key aliases */
        if (src->geom->num_key_aliases) {
            if (src->geom->num_key_aliases != dst->geom->sz_key_aliases) {
                if (dst->geom->sz_key_aliases)
                    tmp = xrealloc(dst->geom->key_aliases,
                                   src->geom->num_key_aliases *
                                    2 * XkbKeyNameLength);
                else
                    tmp = xalloc(src->geom->num_key_aliases *
                                  2 * XkbKeyNameLength);
                if (!tmp)
                    return FALSE;
                dst->geom->key_aliases = tmp;

                dst->geom->sz_key_aliases = src->geom->num_key_aliases;
            }

            memcpy(dst->geom->key_aliases, src->geom->key_aliases,
                   src->geom->num_key_aliases * 2 * XkbKeyNameLength);

            dst->geom->num_key_aliases = dst->geom->sz_key_aliases;
        }
        else {
            if (dst->geom->key_aliases) {
                xfree(dst->geom->key_aliases);
            }
            dst->geom->key_aliases = NULL;
            dst->geom->num_key_aliases = 0;
            dst->geom->sz_key_aliases = 0;
        }
        
        /* font */
        if (src->geom->label_font) {
            if (!dst->geom->label_font) {
                tmp = xalloc(strlen(src->geom->label_font));
                if (!tmp)
                    return FALSE;
                dst->geom->label_font = tmp;
            }
            else if (strlen(src->geom->label_font) !=
                strlen(dst->geom->label_font)) {
                tmp = xrealloc(dst->geom->label_font,
                               strlen(src->geom->label_font));
                if (!tmp)
                    return FALSE;
                dst->geom->label_font = tmp;
            }

            strcpy(dst->geom->label_font, src->geom->label_font);
            i = XkbGeomColorIndex(src->geom, src->geom->label_color);
            dst->geom->label_color = &(dst->geom->colors[i]);
            i = XkbGeomColorIndex(src->geom, src->geom->base_color);
            dst->geom->base_color = &(dst->geom->colors[i]);
        }
        else {
            if (dst->geom->label_font) {
                xfree(dst->geom->label_font);
            }
            dst->geom->label_font = NULL;
            dst->geom->label_color = NULL;
            dst->geom->base_color = NULL;
        }

        dst->geom->name = src->geom->name;
        dst->geom->width_mm = src->geom->width_mm;
        dst->geom->height_mm = src->geom->height_mm;
    }
    else
    {
        if (dst->geom) {
            /* I LOVE THE DIFFERENT CALL SIGNATURE.  REALLY, I DO. */
            XkbFreeGeometry(dst->geom, XkbGeomAllMask, True);
            dst->geom = NULL;
        }
    }

    return TRUE;
}

static Bool
_XkbCopyIndicators(XkbDescPtr src, XkbDescPtr dst)
{
    /* indicators */
    if (src->indicators) {
        if (!dst->indicators) {
            dst->indicators = xalloc(sizeof(XkbIndicatorRec));
            if (!dst->indicators)
                return FALSE;
        }
        memcpy(dst->indicators, src->indicators, sizeof(XkbIndicatorRec));
    }
    else {
        if (dst->indicators) {
            xfree(dst->indicators);
            dst->indicators = NULL;
        }
    }
    return TRUE;
}

static Bool
_XkbCopyControls(XkbDescPtr src, XkbDescPtr dst)
{
    /* controls */
    if (src->ctrls) {
        if (!dst->ctrls) {
            dst->ctrls = xalloc(sizeof(XkbControlsRec));
            if (!dst->ctrls)
                return FALSE;
        }
        memcpy(dst->ctrls, src->ctrls, sizeof(XkbControlsRec));
    }
    else {
        if (dst->ctrls) {
            xfree(dst->ctrls);
            dst->ctrls = NULL;
        }
    }
    return TRUE;
}

/**
 * Copy an XKB map from src to dst, reallocating when necessary: if some
 * map components are present in one, but not in the other, the destination
 * components will be allocated or freed as necessary.
 *
 * Basic map consistency is assumed on both sides, so maps with random
 * uninitialised data (e.g. names->radio_grous == NULL, names->num_rg == 19)
 * _will_ cause failures.  You've been warned.
 *
 * Returns TRUE on success, or FALSE on failure.  If this function fails,
 * dst may be in an inconsistent state: all its pointers are guaranteed
 * to remain valid, but part of the map may be from src and part from dst.
 *
 */

Bool
XkbCopyKeymap(XkbDescPtr src, XkbDescPtr dst, Bool sendNotifies)
{
    DeviceIntPtr pDev = NULL, tmpDev = NULL;
    xkbMapNotify mn;
    xkbNewKeyboardNotify nkn;

    if (!src || !dst || src == dst)
        return FALSE;

    if (!_XkbCopyClientMap(src, dst))
        return FALSE;
    if (!_XkbCopyServerMap(src, dst))
        return FALSE;
    if (!_XkbCopyIndicators(src, dst))
        return FALSE;
    if (!_XkbCopyControls(src, dst))
        return FALSE;
    if (!_XkbCopyNames(src, dst))
        return FALSE;
    if (!_XkbCopyCompat(src, dst))
        return FALSE;
    if (!_XkbCopyGeom(src, dst))
        return FALSE;

    if (inputInfo.keyboard->key->xkbInfo &&
        inputInfo.keyboard->key->xkbInfo->desc == dst) {
        pDev = inputInfo.keyboard;
    }
    else {
        for (tmpDev = inputInfo.devices; tmpDev && !pDev;
             tmpDev = tmpDev->next) {
            if (tmpDev->key && tmpDev->key->xkbInfo &&
                tmpDev->key->xkbInfo->desc == dst) {
                pDev = tmpDev;
                break;
            }
        }
        for (tmpDev = inputInfo.off_devices; tmpDev && !pDev;
                tmpDev = tmpDev->next) {
            if (tmpDev->key && tmpDev->key->xkbInfo &&
                    tmpDev->key->xkbInfo->desc == dst) {
                pDev = tmpDev;
                break;
            }
        }
    }

    if (sendNotifies) {
        if (!pDev) {
            ErrorF("[xkb] XkbCopyKeymap: asked for notifies, but can't find device!\n");
        }
        else {
            /* send NewKeyboardNotify if the keycode range changed, else
             * just MapNotify.  we also need to send NKN if the geometry
             * changed (obviously ...). */
            if ((src->min_key_code != dst->min_key_code ||
                 src->max_key_code != dst->max_key_code)) {
                nkn.oldMinKeyCode = dst->min_key_code;
                nkn.oldMaxKeyCode = dst->max_key_code;
                nkn.deviceID = nkn.oldDeviceID = pDev->id;
                nkn.minKeyCode = src->min_key_code;
                nkn.maxKeyCode = src->max_key_code;
                nkn.requestMajor = XkbReqCode;
                nkn.requestMinor = X_kbSetMap; /* XXX bare-faced lie */
                nkn.changed = XkbAllNewKeyboardEventsMask;
                XkbSendNewKeyboardNotify(pDev, &nkn);
            } else
            {
                mn.deviceID = pDev->id;
                mn.minKeyCode = src->min_key_code;
                mn.maxKeyCode = src->max_key_code;
                mn.firstType = 0;
                mn.nTypes = src->map->num_types;
                mn.firstKeySym = src->min_key_code;
                mn.nKeySyms = XkbNumKeys(src);
                mn.firstKeyAct = src->min_key_code;
                mn.nKeyActs = XkbNumKeys(src);
                /* Cargo-culted from ProcXkbGetMap. */
                mn.firstKeyBehavior = src->min_key_code;
                mn.nKeyBehaviors = XkbNumKeys(src);
                mn.firstKeyExplicit = src->min_key_code;
                mn.nKeyExplicit = XkbNumKeys(src);
                mn.firstModMapKey = src->min_key_code;
                mn.nModMapKeys = XkbNumKeys(src);
                mn.firstVModMapKey = src->min_key_code;
                mn.nVModMapKeys = XkbNumKeys(src);
                mn.virtualMods = ~0; /* ??? */
                mn.changed = XkbAllMapComponentsMask;                
                XkbSendMapNotify(pDev, &mn);
            }
        }
    }

    dst->min_key_code = src->min_key_code;
    dst->max_key_code = src->max_key_code;

    return TRUE;
}
