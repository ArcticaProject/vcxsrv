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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#ifdef HAVE_XKB_CONFIG_H
#include <xkb-config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include "misc.h"
#include "inputstr.h"
#include "opaque.h"
#include "property.h"
#define	XKBSRV_NEED_FILE_FUNCS
#include <xkbsrv.h>
#include "xkbgeom.h"
#include <X11/extensions/XKMformat.h>
#include "xkbfile.h"
#include "xkb.h"

#define	CREATE_ATOM(s)	MakeAtom(s,sizeof(s)-1,1)

#if defined(__alpha) || defined(__alpha__)
#define	LED_COMPOSE	2
#define LED_CAPS	3
#define	LED_SCROLL	4
#define	LED_NUM		5
#define	PHYS_LEDS	0x1f
#else
#ifdef sun
#define LED_NUM		1
#define	LED_SCROLL	2
#define	LED_COMPOSE	3
#define LED_CAPS	4
#define	PHYS_LEDS	0x0f
#else
#define	LED_CAPS	1
#define	LED_NUM		2
#define	LED_SCROLL	3
#define	PHYS_LEDS	0x07
#endif
#endif

#define	MAX_TOC	16
typedef struct	_SrvXkmInfo {
	DeviceIntPtr	dev;
	FILE *		file;
	XkbDescPtr	xkb;
} SrvXkmInfo;


/***====================================================================***/

#ifndef XKB_BASE_DIRECTORY
#define	XKB_BASE_DIRECTORY	"/usr/lib/X11/xkb"
#endif
#ifndef XKB_BIN_DIRECTORY
#define	XKB_BIN_DIRECTORY	XKB_BASE_DIRECTORY
#endif
#ifndef XKB_DFLT_RULES_FILE
#define	XKB_DFLT_RULES_FILE	"base"
#endif
#ifndef XKB_DFLT_KB_LAYOUT
#define	XKB_DFLT_KB_LAYOUT	"us"
#endif
#ifndef XKB_DFLT_KB_MODEL
#define	XKB_DFLT_KB_MODEL	"dflt"
#endif
#ifndef XKB_DFLT_KB_VARIANT
#define	XKB_DFLT_KB_VARIANT	NULL
#endif
#ifndef XKB_DFLT_KB_OPTIONS
#define	XKB_DFLT_KB_OPTIONS	NULL
#endif
#ifndef XKB_DFLT_DISABLED
#define	XKB_DFLT_DISABLED	True
#endif
#ifndef XKB_DFLT_RULES_PROP
#define	XKB_DFLT_RULES_PROP	True
#endif

char	*		XkbBaseDirectory=	XKB_BASE_DIRECTORY;
char	*		XkbBinDirectory=	XKB_BIN_DIRECTORY;
static int	 	XkbWantAccessX=		0;	

static Bool		rulesDefined=		False;
static char *		XkbRulesFile=		NULL;
static char *		XkbModelDflt=		NULL;
static char *		XkbLayoutDflt=		NULL;
static char *		XkbVariantDflt=		NULL;
static char *		XkbOptionsDflt=		NULL;

static char *		XkbModelUsed=	NULL;
static char *		XkbLayoutUsed=	NULL;
static char *		XkbVariantUsed=	NULL;
static char *		XkbOptionsUsed=	NULL;

static XkbDescPtr       xkb_cached_map = NULL;

_X_EXPORT Bool		noXkbExtension=		XKB_DFLT_DISABLED;
static Bool		XkbWantRulesProp=	XKB_DFLT_RULES_PROP;

/***====================================================================***/

static char *
XkbGetRulesDflts(XkbRF_VarDefsPtr defs)
{
    if (XkbModelDflt)	defs->model= XkbModelDflt;
    else		defs->model= XKB_DFLT_KB_MODEL;
    if (XkbLayoutDflt)	defs->layout= XkbLayoutDflt;
    else		defs->layout= XKB_DFLT_KB_LAYOUT;
    if (XkbVariantDflt)	defs->variant= XkbVariantDflt;
    else		defs->variant= XKB_DFLT_KB_VARIANT;
    if (XkbOptionsDflt)	defs->options= XkbOptionsDflt;
    else		defs->options= XKB_DFLT_KB_OPTIONS;
    return (rulesDefined?XkbRulesFile:XKB_DFLT_RULES_FILE);
}

static Bool
XkbWriteRulesProp(ClientPtr client, pointer closure)
{
int 			len,out;
Atom			name;
char *			pval;

    if (rulesDefined && (!XkbRulesFile))
	return False;
    len= (XkbRulesFile?strlen(XkbRulesFile):strlen(XKB_DFLT_RULES_FILE));
    len+= (XkbModelUsed?strlen(XkbModelUsed):0);
    len+= (XkbLayoutUsed?strlen(XkbLayoutUsed):0);
    len+= (XkbVariantUsed?strlen(XkbVariantUsed):0);
    len+= (XkbOptionsUsed?strlen(XkbOptionsUsed):0);
    if (len<1)
	return True;

    len+= 5; /* trailing NULs */

    name= MakeAtom(_XKB_RF_NAMES_PROP_ATOM,strlen(_XKB_RF_NAMES_PROP_ATOM),1);
    if (name==None) {
	ErrorF("[xkb] Atom error: %s not created\n",_XKB_RF_NAMES_PROP_ATOM);
	return True;
    }
    pval= (char*) xalloc(len);
    if (!pval) {
	ErrorF("[xkb] Allocation error: %s proprerty not created\n",
						_XKB_RF_NAMES_PROP_ATOM);
	return True;
    }
    out= 0;
    if (XkbRulesFile) {
	strcpy(&pval[out],XkbRulesFile);
	out+= strlen(XkbRulesFile);
    } else {
	strcpy(&pval[out],XKB_DFLT_RULES_FILE);
	out+= strlen(XKB_DFLT_RULES_FILE);
    }
    pval[out++]= '\0';
    if (XkbModelUsed) {
	strcpy(&pval[out],XkbModelUsed);
	out+= strlen(XkbModelUsed);
    } 
    pval[out++]= '\0';
    if (XkbLayoutUsed) {
	strcpy(&pval[out],XkbLayoutUsed);
	out+= strlen(XkbLayoutUsed);
    }
    pval[out++]= '\0';
    if (XkbVariantUsed) {
	strcpy(&pval[out],XkbVariantUsed);
	out+= strlen(XkbVariantUsed);
    }
    pval[out++]= '\0';
    if (XkbOptionsUsed) {
	strcpy(&pval[out],XkbOptionsUsed);
	out+= strlen(XkbOptionsUsed);
    }
    pval[out++]= '\0';
    if (out!=len) {
	ErrorF("[xkb] Internal Error! bad size (%d!=%d) for _XKB_RULES_NAMES\n",
								out,len);
    }
    dixChangeWindowProperty(serverClient, WindowTable[0], name, XA_STRING, 8,
			    PropModeReplace, len, pval, True);
    xfree(pval);
    return True;
}

static void
XkbSetRulesUsed(XkbRF_VarDefsPtr defs)
{
    if (XkbModelUsed)
	_XkbFree(XkbModelUsed);
    XkbModelUsed= (defs->model?_XkbDupString(defs->model):NULL);
    if (XkbLayoutUsed)
	_XkbFree(XkbLayoutUsed);
    XkbLayoutUsed= (defs->layout?_XkbDupString(defs->layout):NULL);
    if (XkbVariantUsed)
	_XkbFree(XkbVariantUsed);
    XkbVariantUsed= (defs->variant?_XkbDupString(defs->variant):NULL);
    if (XkbOptionsUsed)
	_XkbFree(XkbOptionsUsed);
    XkbOptionsUsed= (defs->options?_XkbDupString(defs->options):NULL);
    if (XkbWantRulesProp)
	QueueWorkProc(XkbWriteRulesProp,NULL,NULL);
    return;
}

/**
 * Set the default RMLVO for the next device to be initialised.
 * If a parameter is NULL, the previous setting will be used. Use empty
 * strings if you want to delete a previous setting.
 *
 * If @rulesFile is NULL and no previous @rulesFile has been set, the
 * built-in default is chosen as default.
 */
_X_EXPORT void
XkbSetRulesDflts(char *rulesFile,char *model,char *layout,
					char *variant,char *options)
{
    if (!rulesFile && !XkbRulesFile)
    {
	LogMessage(X_WARNING, "[xkb] No rule given, and no previous rule "
		              "defined. Defaulting to '%s'.\n",
                              XKB_DFLT_RULES_FILE);
	rulesFile = XKB_DFLT_RULES_FILE;
    }

    if (rulesFile) {
	if (XkbRulesFile)
	    _XkbFree(XkbRulesFile);
	XkbRulesFile= _XkbDupString(rulesFile);
	rulesDefined= True;
    }

    if (model) {
	if (XkbModelDflt)
	    _XkbFree(XkbModelDflt);
	XkbModelDflt= _XkbDupString(model);
    }
    if (layout) {
	if (XkbLayoutDflt)
	    _XkbFree(XkbLayoutDflt);
	XkbLayoutDflt= _XkbDupString(layout);
    }
    if (variant) {
	if (XkbVariantDflt)
	    _XkbFree(XkbVariantDflt);
	XkbVariantDflt= _XkbDupString(variant);
    }
    if (options) {
	if (XkbOptionsDflt)
	    _XkbFree(XkbOptionsDflt);
	XkbOptionsDflt= _XkbDupString(options);
    }
    return;
}

void
XkbDeleteRulesDflts()
{
    _XkbFree(XkbRulesFile);
    XkbRulesFile = NULL;
    _XkbFree(XkbModelDflt);
    XkbModelDflt = NULL;
    _XkbFree(XkbLayoutDflt);
    XkbLayoutDflt = NULL;
    _XkbFree(XkbVariantDflt);
    XkbVariantDflt = NULL;
    _XkbFree(XkbOptionsDflt);
    XkbOptionsDflt = NULL;

    XkbFreeKeyboard(xkb_cached_map, XkbAllComponentsMask, True);
    xkb_cached_map = NULL;
}

/***====================================================================***/

#include "xkbDflts.h"

static Bool
XkbInitKeyTypes(XkbDescPtr xkb)
{
    if (xkb->defined & XkmTypesMask)
        return True;

    initTypeNames(NULL);
    if (XkbAllocClientMap(xkb,XkbKeyTypesMask,num_dflt_types)!=Success)
	return False;
    if (XkbCopyKeyTypes(dflt_types,xkb->map->types,num_dflt_types)!=
    								 Success) {
	return False;
    }
    xkb->map->size_types= xkb->map->num_types= num_dflt_types;
    return True;
}

static void
XkbInitRadioGroups(XkbSrvInfoPtr xkbi)
{
    xkbi->nRadioGroups = 0;
    xkbi->radioGroups = NULL;
    return;
}


static Status
XkbInitCompatStructs(XkbDescPtr xkb)
{
register int 	i;
XkbCompatMapPtr	compat;

    if (xkb->defined & XkmCompatMapMask)
        return True;

    if (XkbAllocCompatMap(xkb,XkbAllCompatMask,num_dfltSI)!=Success)
	return BadAlloc;
    compat = xkb->compat;
    if (compat->sym_interpret) {
	compat->num_si = num_dfltSI;
	memcpy((char *)compat->sym_interpret,(char *)dfltSI,sizeof(dfltSI));
    }
    for (i=0;i<XkbNumKbdGroups;i++) {
	compat->groups[i]= compatMap.groups[i];
	if (compat->groups[i].vmods!=0) {
	    unsigned mask;
	    mask= XkbMaskForVMask(xkb,compat->groups[i].vmods);
	    compat->groups[i].mask= compat->groups[i].real_mods|mask;
	}
	else compat->groups[i].mask= compat->groups[i].real_mods;
    }
    return Success;
}

static void
XkbInitSemantics(XkbDescPtr xkb)
{
    XkbInitKeyTypes(xkb);
    XkbInitCompatStructs(xkb);
    return;
}

/***====================================================================***/

static Status
XkbInitNames(XkbSrvInfoPtr xkbi)
{
XkbDescPtr	xkb;
XkbNamesPtr	names;
Status		rtrn;
Atom		unknown;

    xkb= xkbi->desc;
    if ((rtrn=XkbAllocNames(xkb,XkbAllNamesMask,0,0))!=Success)
	return rtrn;
    unknown= CREATE_ATOM("unknown");
    names = xkb->names;
    if (names->keycodes==None)		names->keycodes= unknown;
    if (names->geometry==None)		names->geometry= unknown;
    if (names->phys_symbols==None)	names->phys_symbols= unknown;
    if (names->symbols==None)		names->symbols= unknown;
    if (names->types==None)		names->types= unknown;
    if (names->compat==None)		names->compat= unknown;
    if (!(xkb->defined & XkmVirtualModsMask)) {
        if (names->vmods[vmod_NumLock]==None)
            names->vmods[vmod_NumLock]= CREATE_ATOM("NumLock");
        if (names->vmods[vmod_Alt]==None)
            names->vmods[vmod_Alt]= CREATE_ATOM("Alt");
        if (names->vmods[vmod_AltGr]==None)
            names->vmods[vmod_AltGr]= CREATE_ATOM("ModeSwitch");
    }

    if (!(xkb->defined & XkmIndicatorsMask) ||
        !(xkb->defined & XkmGeometryMask)) {
        initIndicatorNames(NULL,xkb);
        if (names->indicators[LED_CAPS-1]==None)
            names->indicators[LED_CAPS-1] = CREATE_ATOM("Caps Lock");
        if (names->indicators[LED_NUM-1]==None)
            names->indicators[LED_NUM-1] = CREATE_ATOM("Num Lock");
        if (names->indicators[LED_SCROLL-1]==None)
            names->indicators[LED_SCROLL-1] = CREATE_ATOM("Scroll Lock");
#ifdef LED_COMPOSE
        if (names->indicators[LED_COMPOSE-1]==None)
            names->indicators[LED_COMPOSE-1] = CREATE_ATOM("Compose");
#endif
    }

    if (xkb->geom!=NULL)
	 names->geometry= xkb->geom->name;
    else names->geometry= unknown;

    return Success;
}

static Status
XkbInitIndicatorMap(XkbSrvInfoPtr xkbi)
{
XkbDescPtr		xkb;
XkbIndicatorPtr		map;
XkbSrvLedInfoPtr	sli;

    xkb= xkbi->desc;
    if (XkbAllocIndicatorMaps(xkb)!=Success)
	return BadAlloc;

    if (!(xkb->defined & XkmIndicatorsMask)) {
        map= xkb->indicators;
        map->phys_indicators = PHYS_LEDS;
        map->maps[LED_CAPS-1].flags= XkbIM_NoExplicit;
        map->maps[LED_CAPS-1].which_mods= XkbIM_UseLocked;
        map->maps[LED_CAPS-1].mods.mask= LockMask;
        map->maps[LED_CAPS-1].mods.real_mods= LockMask;

        map->maps[LED_NUM-1].flags= XkbIM_NoExplicit;
        map->maps[LED_NUM-1].which_mods= XkbIM_UseLocked;
        map->maps[LED_NUM-1].mods.mask= 0;
        map->maps[LED_NUM-1].mods.real_mods= 0;
        map->maps[LED_NUM-1].mods.vmods= vmod_NumLockMask;

        map->maps[LED_SCROLL-1].flags= XkbIM_NoExplicit;
        map->maps[LED_SCROLL-1].which_mods= XkbIM_UseLocked;
        map->maps[LED_SCROLL-1].mods.mask= Mod3Mask;
        map->maps[LED_SCROLL-1].mods.real_mods= Mod3Mask;
    }

    sli= XkbFindSrvLedInfo(xkbi->device,XkbDfltXIClass,XkbDfltXIId,0);
    if (sli)
	XkbCheckIndicatorMaps(xkbi->device,sli,XkbAllIndicatorsMask);

    return Success;
}

static Status
XkbInitControls(DeviceIntPtr pXDev,XkbSrvInfoPtr xkbi)
{
XkbDescPtr	xkb;
XkbControlsPtr	ctrls;

    xkb= xkbi->desc;
    /* 12/31/94 (ef) -- XXX! Should check if controls loaded from file */
    if (XkbAllocControls(xkb,XkbAllControlsMask)!=Success)
	FatalError("Couldn't allocate keyboard controls\n");
    ctrls= xkb->ctrls;
    if (!(xkb->defined & XkmSymbolsMask))
        ctrls->num_groups = 1;
    ctrls->groups_wrap = XkbSetGroupInfo(1,XkbWrapIntoRange,0);
    ctrls->internal.mask = 0;
    ctrls->internal.real_mods = 0;
    ctrls->internal.vmods = 0;
    ctrls->ignore_lock.mask = 0;
    ctrls->ignore_lock.real_mods = 0;
    ctrls->ignore_lock.vmods = 0;
    ctrls->enabled_ctrls = XkbAccessXTimeoutMask|XkbRepeatKeysMask|
				XkbMouseKeysAccelMask|XkbAudibleBellMask|
				XkbIgnoreGroupLockMask;
    if (XkbWantAccessX)
	ctrls->enabled_ctrls|= XkbAccessXKeysMask;
    AccessXInit(pXDev);
    return Success;
}

void
XkbInitDevice(DeviceIntPtr pXDev)
{
int			i;
XkbSrvInfoPtr		xkbi;
XkbChangesRec		changes;
unsigned		check;
XkbEventCauseRec	cause;

    bzero(&changes,sizeof(XkbChangesRec));
    pXDev->key->xkbInfo= xkbi= _XkbTypedCalloc(1,XkbSrvInfoRec);
    if ( xkbi ) {
	XkbDescPtr	xkb;

        if (xkb_cached_map) {
            xkbi->desc = xkb_cached_map;
            xkb_cached_map = NULL;
        }
        else {
            xkbi->desc= XkbAllocKeyboard();
            if (!xkbi->desc)
                FatalError("Couldn't allocate keyboard description\n");
            xkbi->desc->min_key_code = pXDev->key->curKeySyms.minKeyCode;
            xkbi->desc->max_key_code = pXDev->key->curKeySyms.maxKeyCode;
        }
	xkb= xkbi->desc;
	if (xkb->min_key_code == 0)
	    xkb->min_key_code = pXDev->key->curKeySyms.minKeyCode;
	if (xkb->max_key_code == 0)
	    xkb->max_key_code = pXDev->key->curKeySyms.maxKeyCode;
	if ((pXDev->key->curKeySyms.minKeyCode!=xkbi->desc->min_key_code)||
	    (pXDev->key->curKeySyms.maxKeyCode!=xkbi->desc->max_key_code)) {
	    /* 12/9/95 (ef) -- XXX! Maybe we should try to fix up one or */
	    /*                 the other here, but for now just complain */
	    /*                 can't just update the core range without */
	    /*                 reallocating the KeySymsRec (pain)       */
	    ErrorF("[xkb] Internal Error!! XKB and core keymap have different range\n");
	}
	if (XkbAllocClientMap(xkb,XkbAllClientInfoMask,0)!=Success)
	    FatalError("Couldn't allocate client map in XkbInitDevice\n");
	i= XkbNumKeys(xkb)/3+1;
	if (XkbAllocServerMap(xkb,XkbAllServerInfoMask,i)!=Success)
	    FatalError("Couldn't allocate server map in XkbInitDevice\n");

	xkbi->dfltPtrDelta=1;
	xkbi->device = pXDev;

	XkbInitSemantics(xkb);
	XkbInitNames(xkbi);
	XkbInitRadioGroups(xkbi);

	/* 12/31/94 (ef) -- XXX! Should check if state loaded from file */
	bzero(&xkbi->state,sizeof(XkbStateRec));

	XkbInitControls(pXDev,xkbi);

        if (xkb->defined & XkmSymbolsMask)
            memcpy(pXDev->key->modifierMap, xkb->map->modmap,
                   xkb->max_key_code + 1);
        else
            memcpy(xkb->map->modmap, pXDev->key->modifierMap,
                   xkb->max_key_code + 1);

	XkbInitIndicatorMap(xkbi);

	XkbDDXInitDevice(pXDev);

        if (xkb->defined & XkmSymbolsMask)
            XkbUpdateCoreDescription(pXDev, True);
        else
            XkbUpdateKeyTypesFromCore(pXDev, xkb->min_key_code,
                                      XkbNumKeys(xkb), &changes);

	XkbSetCauseUnknown(&cause);
	XkbUpdateActions(pXDev,xkb->min_key_code, XkbNumKeys(xkb),&changes,
								&check,&cause);
        /* For sanity.  The first time the connection
         * is opened, the client side min and max are set
         * using QueryMinMaxKeyCodes() which grabs them 
	 * from pXDev.
	 */
	pXDev->key->curKeySyms.minKeyCode = xkb->min_key_code;
	pXDev->key->curKeySyms.maxKeyCode = xkb->max_key_code;
    }
    return;
}

#if MAP_LENGTH > XkbMaxKeyCount
#undef  XkbMaxKeyCount
#define XkbMaxKeyCount MAP_LENGTH
#endif

_X_EXPORT Bool
XkbInitKeyboardDeviceStruct(
    DeviceIntPtr		dev,
    XkbComponentNamesPtr	names,
    KeySymsPtr                  pSymsIn,
    CARD8                       pModsIn[],
    void                        (*bellProc)(
        int /*percent*/,
        DeviceIntPtr /*device*/,
        pointer /*ctrl*/,
        int),
    void                        (*ctrlProc)(
        DeviceIntPtr /*device*/,
        KeybdCtrl * /*ctrl*/))
{
KeySymsRec		tmpSyms,*pSyms;
CARD8			tmpMods[XkbMaxLegalKeyCode+1],*pMods;
char			name[PATH_MAX],*rules;
Bool			ok=False;
XkbRF_VarDefsRec	defs;
XkbDescPtr              xkb;

    if ((dev->key!=NULL)||(dev->kbdfeed!=NULL))
	return False;
    pSyms= pSymsIn;
    pMods= pModsIn;
    bzero(&defs,sizeof(XkbRF_VarDefsRec));
    rules= XkbGetRulesDflts(&defs);

    /*
     * The strings are duplicated because it is not guaranteed that
     * they are allocated, or that they are allocated for every server
     * generation. Eventually they will be freed at the end of this
     * function.
     */
    names->keymap = NULL;
    if (names->keycodes) names->keycodes = _XkbDupString(names->keycodes);
    if (names->types) names->types = _XkbDupString(names->types);
    if (names->compat) names->compat = _XkbDupString(names->compat);
    if (names->geometry) names->geometry = _XkbDupString(names->geometry);
    if (names->symbols) names->symbols = _XkbDupString(names->symbols);

    if (defs.model && defs.layout && rules) {
	XkbComponentNamesRec	rNames;
	bzero(&rNames,sizeof(XkbComponentNamesRec));
	if (XkbDDXNamesFromRules(dev,rules,&defs,&rNames)) {
	    if (rNames.keycodes) {
		if (!names->keycodes)
		    names->keycodes =  rNames.keycodes;
		else
		    _XkbFree(rNames.keycodes);
	    }
	    if (rNames.types) {
		if (!names->types)
		    names->types = rNames.types;
		else  _XkbFree(rNames.types);
	    }
	    if (rNames.compat) {
		if (!names->compat) 
		    names->compat =  rNames.compat;
		else  _XkbFree(rNames.compat);
	    }
	    if (rNames.symbols) {
		if (!names->symbols)
		    names->symbols =  rNames.symbols;
		else _XkbFree(rNames.symbols);
	    }
	    if (rNames.geometry) {
		if (!names->geometry)
		    names->geometry = rNames.geometry;
		else _XkbFree(rNames.geometry);
	    }
	    XkbSetRulesUsed(&defs);
	}
    }

    ok = (Bool) XkbDDXLoadKeymapByNames(dev,names,XkmAllIndicesMask,0,
                                        &xkb,name,PATH_MAX);

    if (ok && (xkb!=NULL)) {
	KeyCode		minKC,maxKC;

	minKC= xkb->min_key_code;
	maxKC= xkb->max_key_code;
	if (XkbIsLegalKeycode(minKC)&&XkbIsLegalKeycode(maxKC)&&(minKC<=maxKC)&&
	    ((minKC!=pSyms->minKeyCode)||(maxKC!=pSyms->maxKeyCode))) {
	    if (xkb->map!=NULL) {
		KeySym	*inSym,*outSym;
		int	width= pSymsIn->mapWidth;

		tmpSyms.minKeyCode= minKC;
		tmpSyms.maxKeyCode= maxKC;

		if (minKC<pSymsIn->minKeyCode)
		    minKC= pSymsIn->minKeyCode;
		if (maxKC>pSymsIn->maxKeyCode)
		    maxKC= pSymsIn->maxKeyCode;

		tmpSyms.mapWidth= width;
		tmpSyms.map= _XkbTypedCalloc(width*XkbNumKeys(xkb),KeySym);
		inSym= &pSymsIn->map[(minKC-pSymsIn->minKeyCode)*width];
		outSym= &tmpSyms.map[(minKC-tmpSyms.minKeyCode)*width];
		memcpy(outSym,inSym,((maxKC-minKC+1)*width)*sizeof(KeySym));
		pSyms= &tmpSyms;
	    }
	    if ((xkb->map!=NULL)&&(xkb->map->modmap!=NULL)) {
		bzero(tmpMods,XkbMaxKeyCount);
		memcpy(tmpMods,xkb->map->modmap,maxKC+1);
		pMods= tmpMods;
	    }
	}
        /* Store the map here so we can pick it back up in XkbInitDevice.
         * Sigh. */
        xkb_cached_map = xkb;
    }
    else {
	LogMessage(X_WARNING, "Couldn't load XKB keymap, falling back to pre-XKB keymap\n");
    }
    ok= InitKeyboardDeviceStruct((DevicePtr)dev,pSyms,pMods,bellProc,ctrlProc);
    xkb_cached_map = NULL;
    if ((pSyms==&tmpSyms)&&(pSyms->map!=NULL)) {
	_XkbFree(pSyms->map);
	pSyms->map= NULL;
    }

    if (names->keycodes) _XkbFree(names->keycodes);
    names->keycodes = NULL;
    if (names->types) _XkbFree(names->types);
    names->types = NULL;
    if (names->compat) _XkbFree(names->compat);
    names->compat = NULL;
    if (names->geometry) _XkbFree(names->geometry);
    names->geometry = NULL;
    if (names->symbols) _XkbFree(names->symbols);
    names->symbols = NULL;

    return ok;
}

/***====================================================================***/

	/*
	 * InitKeyClassDeviceStruct initializes the key class before it
	 * initializes the keyboard feedback class for a device. 
	 * UpdateActions can't set up the correct autorepeat for keyboard 
	 * initialization because the keyboard feedback isn't created yet.   
	 * Instead, UpdateActions notes the "correct" autorepeat in the 
	 * SrvInfo structure and InitKbdFeedbackClass calls UpdateAutoRepeat 
	 * to apply the computed autorepeat once the feedback class exists.
	 *
	 * DIX will apply the changed autorepeat, so there's no need to
	 * do so here.   This function returns True if both RepeatKeys and
	 * the core protocol autorepeat ctrls are set (i.e. should use 
	 * software autorepeat), false otherwise.
	 *
	 * This function also computes the autorepeat accelerators for the
	 * default indicator feedback.
	 */
int
XkbFinishDeviceInit(DeviceIntPtr pXDev)
{
XkbSrvInfoPtr		xkbi;
XkbDescPtr		xkb;
int			softRepeat;
XkbSrvLedInfoPtr	sli;

    xkbi = NULL;
    if (pXDev && pXDev->key && pXDev->key->xkbInfo && pXDev->kbdfeed) {
	xkbi= pXDev->key->xkbInfo;
	xkb= xkbi->desc;
        /* If we come from DeepCopyDeviceClasses, the CtrlProc was already set
         * to XkbDDXKeybdCtrlProc, overwriting it leads to happy recursion.
         */
	if (pXDev->kbdfeed && pXDev->kbdfeed->CtrlProc != XkbDDXKeybdCtrlProc) {
	    xkbi->kbdProc= pXDev->kbdfeed->CtrlProc;
	    pXDev->kbdfeed->CtrlProc= XkbDDXKeybdCtrlProc;
	}
	if (pXDev->kbdfeed->ctrl.autoRepeat)
	    xkb->ctrls->enabled_ctrls|= XkbRepeatKeysMask;
	softRepeat= (xkb->ctrls->enabled_ctrls&XkbRepeatKeysMask)!=0;
	if (pXDev->kbdfeed) {
	    memcpy(pXDev->kbdfeed->ctrl.autoRepeats,
		   xkb->ctrls->per_key_repeat,XkbPerKeyBitArraySize);
	    softRepeat= softRepeat&&pXDev->kbdfeed->ctrl.autoRepeat;
	}
    }
    else softRepeat= 0;
    sli= XkbFindSrvLedInfo(pXDev,XkbDfltXIClass,XkbDfltXIId,0);
    if (sli && xkbi)
	XkbCheckIndicatorMaps(xkbi->device,sli,XkbAllIndicatorsMask);
    else DebugF("[xkb] No indicator feedback in XkbFinishInit (shouldn't happen)!\n");
    return softRepeat;
}

	/*
	 * Be very careful about what does and doesn't get freed by this 
	 * function.  To reduce fragmentation, XkbInitDevice allocates a 
	 * single huge block per device and divides it up into most of the 
	 * fixed-size structures for the device.   Don't free anything that
	 * is part of this larger block.
	 */
void
XkbFreeInfo(XkbSrvInfoPtr xkbi)
{
    if (xkbi->radioGroups) {
	_XkbFree(xkbi->radioGroups);
	xkbi->radioGroups= NULL;
    }
    if (xkbi->mouseKeyTimer) {
	TimerFree(xkbi->mouseKeyTimer);
	xkbi->mouseKeyTimer= NULL;
    }
    if (xkbi->slowKeysTimer) {
	TimerFree(xkbi->slowKeysTimer);
	xkbi->slowKeysTimer= NULL;
    }
    if (xkbi->bounceKeysTimer) {
	TimerFree(xkbi->bounceKeysTimer);
	xkbi->bounceKeysTimer= NULL;
    }
    if (xkbi->repeatKeyTimer) {
	TimerFree(xkbi->repeatKeyTimer);
	xkbi->repeatKeyTimer= NULL;
    }
    if (xkbi->krgTimer) {
	TimerFree(xkbi->krgTimer);
	xkbi->krgTimer= NULL;
    }
    xkbi->beepType= _BEEP_NONE;
    if (xkbi->beepTimer) {
	TimerFree(xkbi->beepTimer);
	xkbi->beepTimer= NULL;
    }
    if (xkbi->desc) {
	XkbFreeKeyboard(xkbi->desc,XkbAllComponentsMask,True);
	xkbi->desc= NULL;
    }
    _XkbFree(xkbi);
    return;
}

/***====================================================================***/

extern int	XkbDfltRepeatDelay;
extern int	XkbDfltRepeatInterval;

extern unsigned short	XkbDfltAccessXTimeout;
extern unsigned int	XkbDfltAccessXTimeoutMask;
extern unsigned int	XkbDfltAccessXFeedback;
extern unsigned char	XkbDfltAccessXOptions;

int
XkbProcessArguments(int argc,char *argv[],int i)
{
    if (strcmp(argv[i],"-kb")==0) {
	noXkbExtension= True;
	return 1;
    }
    else if (strcmp(argv[i],"+kb")==0) {
	noXkbExtension= False;
	return 1;
    }
    else if (strncmp(argv[i], "-xkbdir", 7) == 0) {
	if(++i < argc) {
#if !defined(WIN32) && !defined(__CYGWIN__)
	    if (getuid() != geteuid()) {
		LogMessage(X_WARNING, "-xkbdir is not available for setuid X servers\n");
		return -1;
	    } else
#endif
	    {
		if (strlen(argv[i]) < PATH_MAX) {
		    XkbBaseDirectory= argv[i];
		    return 2;
	        } else {
		    LogMessage(X_ERROR, "-xkbdir pathname too long\n");
		    return -1;
		}
	    }
	}
	else {
	    return -1;
	}
    }
    else if ((strncmp(argv[i],"-accessx",8)==0)||
                 (strncmp(argv[i],"+accessx",8)==0)) {
	int j=1;	    
	if (argv[i][0]=='-')        
	    XkbWantAccessX= 0;
	else {
	    XkbWantAccessX= 1;
	    
	    if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		XkbDfltAccessXTimeout = atoi(argv[++i]);
		j++;

		if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		    /*
		     * presumption that the reasonably useful range of
		     * values fits in 0..MAXINT since SunOS 4 doesn't
		     * have strtoul.
		     */
		    XkbDfltAccessXTimeoutMask=(unsigned int)
					      strtol(argv[++i],NULL,16); 
		    j++;
		}
		if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		    if (argv[++i][0] == '1' ) 
			XkbDfltAccessXFeedback=XkbAccessXFeedbackMask;
		    else
			XkbDfltAccessXFeedback=0;
		    j++;
		}
		if ( ((i+1)<argc) && (isdigit(argv[i+1][0])) ) {
		    XkbDfltAccessXOptions=(unsigned char)
					   strtol(argv[++i],NULL,16);
		    j++;
		}
	    }
	}
	return j;
    }
    if ((strcmp(argv[i], "-ardelay") == 0) ||
        (strcmp (argv[i], "-ar1") == 0)) {	/* -ardelay int */
	if (++i >= argc) UseMsg ();
	XkbDfltRepeatDelay = (long)atoi(argv[i]);
	return 2;
    }
    if ((strcmp(argv[i], "-arinterval") == 0) ||
        (strcmp (argv[i], "-ar2") == 0)) {	/* -arinterval int */
	if (++i >= argc) UseMsg ();
	XkbDfltRepeatInterval = (long)atoi(argv[i]);
	return 2;
    }
    return 0;
}

void
XkbUseMsg(void)
{
    ErrorF("-kb                    disable the X Keyboard Extension\n");
    ErrorF("+kb                    enable the X Keyboard Extension\n");
    ErrorF("[+-]accessx [ timeout [ timeout_mask [ feedback [ options_mask] ] ] ]\n");
    ErrorF("                       enable/disable accessx key sequences\n");
    ErrorF("-ardelay               set XKB autorepeat delay\n");
    ErrorF("-arinterval            set XKB autorepeat interval\n");
}
