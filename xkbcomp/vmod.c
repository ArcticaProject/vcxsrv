/* $Xorg: vmod.c,v 1.3 2000/08/17 19:54:33 cpqbld Exp $ */
/************************************************************
 Copyright (c) 1994 by Silicon Graphics Computer Systems, Inc.

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
/* $XFree86: xc/programs/xkbcomp/vmod.c,v 3.3 2001/01/17 23:45:45 dawes Exp $ */

#define	DEBUG_VAR_NOT_LOCAL
#define	DEBUG_VAR debugFlags
#include <stdio.h>
#include "xkbcomp.h"
#include "tokens.h"
#include "expr.h"
#include "misc.h"

#include <X11/extensions/XKB.h>
#include <X11/extensions/XKBstr.h>

#include "vmod.h"

void
InitVModInfo(VModInfo *info,XkbDescPtr xkb)
{
    ClearVModInfo(info,xkb);
    info->errorCount= 0;
    return;
}

void
ClearVModInfo(VModInfo *info,XkbDescPtr xkb)
{
register int i;

    if (XkbAllocNames(xkb,XkbVirtualModNamesMask,0,0)!=Success)
	return;
    if (XkbAllocServerMap(xkb,XkbVirtualModsMask,0)!=Success)
	return;
    info->xkb= xkb;
    info->newlyDefined= info->defined= info->available= 0;
    if (xkb && xkb->names) {
	register int bit;
	for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	    if (xkb->names->vmods[i]!=None)
		info->defined|= bit;
	}
    }
    return;
}

/***====================================================================***/

Bool
HandleVModDef(VModDef *stmt,unsigned mergeMode,VModInfo *info)
{
register int 	i,bit,nextFree;
ExprResult 	mod;
XkbServerMapPtr	srv;
XkbNamesPtr	names;
Atom		stmtName;
    
    srv= info->xkb->server;
    names= info->xkb->names;
    stmtName= XkbInternAtom(info->xkb->dpy,XkbAtomGetString(NULL,stmt->name),
    									False);
    for (i=0,bit=1,nextFree= -1;i<XkbNumVirtualMods;i++,bit<<=1) {
	if (info->defined&bit) {
	    if (names->vmods[i]==stmtName) {	/* already defined */
		info->available|= bit;
		if (stmt->value==NULL)
		    return True;
		else {
		    char *str1;
		    const char *str2 = "";
		    if (!ExprResolveModMask(stmt->value,&mod,NULL,NULL)) {
			str1= XkbAtomText(NULL,stmt->name,XkbMessage);
			ACTION1("Declaration of %s ignored\n",str1);
			return False;
		    }
		    if (mod.uval==srv->vmods[i])
			return True;

		    str1= XkbAtomText(NULL,stmt->name,XkbMessage);
		    WARN1("Virtual modifier %s multiply defined\n",str1);
		    str1= XkbModMaskText(srv->vmods[i],XkbCFile);
		    if (mergeMode==MergeOverride) {
			str2= str1;
			str1= XkbModMaskText(mod.uval,XkbCFile);
		    }
		    ACTION2("Using %s, ignoring %s\n",str1,str2);
		    if (mergeMode==MergeOverride)
			srv->vmods[i]= mod.uval;
		    return True;
		}
	    }
	}
	else if (nextFree<0)
	    nextFree= i;
    }
    if (nextFree<0) {
	ERROR1("Too many virtual modifiers defined (maximum %d)\n",
						XkbNumVirtualMods);
	ACTION("Exiting\n");
	return False;
    }
    info->defined|= (1<<nextFree);
    info->newlyDefined|= (1<<nextFree);
    info->available|= (1<<nextFree);
    names->vmods[nextFree]= stmtName;
    if (stmt->value==NULL)
	return True;
    if (ExprResolveModMask(stmt->value,&mod,NULL,NULL)) {
	srv->vmods[nextFree]= mod.uval;
	return True;
    }
    ACTION1("Declaration of %s ignored\n",
    				XkbAtomText(NULL,stmt->name,XkbMessage));
    return False;
}

int
LookupVModIndex(	XPointer	priv,
			Atom 		elem,
			Atom 		field,
			unsigned 	type,
			ExprResult *	val_rtrn)
{
register int	i;
register char *	fieldStr;
register char *	modStr;
XkbDescPtr	xkb;

    xkb= (XkbDescPtr)priv;
    if ((xkb==NULL)||(xkb->names==NULL)||(elem!=None)||(type!=TypeInt)) {
	return False;
    }
    fieldStr= XkbAtomGetString(xkb->dpy,field);
    if (fieldStr==NULL)
	return False;
    for (i=0;i<XkbNumVirtualMods;i++) {
	modStr= XkbAtomGetString(xkb->dpy,xkb->names->vmods[i]);
	if ((modStr!=NULL)&&(uStrCaseCmp(fieldStr,modStr)==0)) {
	    val_rtrn->uval= i;
	    return True;
	}
    }
    return False;
}

int
LookupVModMask(	XPointer 	priv,
		Atom 		elem,
		Atom 		field,
		unsigned 	type,
		ExprResult *	val_rtrn)
{
    if (LookupVModIndex(priv,elem,field,type,val_rtrn)) {
	register unsigned ndx= val_rtrn->uval;
	val_rtrn->uval= (1<<(XkbNumModifiers+ndx));
	return True;
    }
    return False;
}

int
FindKeypadVMod(XkbDescPtr xkb)
{
Atom name;
ExprResult rtrn;

    name= XkbInternAtom(xkb->dpy,"NumLock",False);
    if ((xkb)&&
	LookupVModIndex((XPointer)xkb,None,name,TypeInt,&rtrn)) {
	return rtrn.ival;
    }
    return -1;
}

Bool
ResolveVirtualModifier(ExprDef *def,ExprResult *val_rtrn,VModInfo *info)
{
XkbNamesPtr	names;

    names= info->xkb->names;
    if (def->op==ExprIdent) {
	register int i,bit;
	for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	    char *str1,*str2;
	    str1= XkbAtomGetString(info->xkb->dpy,names->vmods[i]);
	    str2= XkbAtomGetString(NULL,def->value.str);
	    if ((info->available&bit)&&
		(uStrCaseCmp(str1,str2)==Equal)) {
		val_rtrn->uval= i;
		return True;
	    }
	}
    }
    if (ExprResolveInteger(def,val_rtrn,NULL,NULL)) {
	if (val_rtrn->uval<XkbNumVirtualMods)
	    return True;
	ERROR2("Illegal virtual modifier %d (must be 0..%d inclusive)\n",
					val_rtrn->uval,XkbNumVirtualMods-1);
    }
    return False;
}
