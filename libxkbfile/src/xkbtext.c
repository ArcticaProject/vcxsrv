/* $Xorg: xkbtext.c,v 1.3 2000/08/17 19:46:44 cpqbld Exp $ */
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
/* $XFree86: xc/lib/xkbfile/xkbtext.c,v 3.11 2002/12/21 18:49:02 paulo Exp $ */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#elif defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <X11/Xos.h>

#ifndef XKB_IN_SERVER

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>

#include "XKMformat.h"
#include "XKBfileInt.h"

#else

#include <X11/X.h>
#define	NEED_EVENTS
#include <X11/Xproto.h>
#include "misc.h"
#include "inputstr.h"
#include "dix.h"
#include <X11/extensions/XKBstr.h>
#define XKBSRV_NEED_FILE_FUNCS	1
#include <X11/extensions/XKBsrv.h>
#include <X11/extensions/XKBgeom.h>

#endif

/***====================================================================***/

#define	BUFFER_SIZE	512

static char textBuffer[BUFFER_SIZE];
static int  tbNext= 0;

static char *
tbGetBuffer(unsigned size)
{
char *rtrn;

    if (size>=BUFFER_SIZE)
	return NULL;
    if ((BUFFER_SIZE-tbNext)<=size)
	tbNext= 0;
    rtrn= &textBuffer[tbNext];
    tbNext+= size;
    return rtrn;
}

/***====================================================================***/

char *
XkbAtomText(Display *dpy,Atom atm,unsigned format)
{
char	*rtrn,*tmp;

    tmp= XkbAtomGetString(dpy,atm);
    if (tmp!=NULL) {
	int	len;
	len= strlen(tmp)+1;
	if (len>BUFFER_SIZE)
	    len= BUFFER_SIZE-2;
	rtrn= tbGetBuffer(len);
	strncpy(rtrn,tmp,len);
	rtrn[len]= '\0';
        _XkbFree(tmp);
    }
    else {
	rtrn= tbGetBuffer(1);
	rtrn[0]= '\0';
    }
    if (format==XkbCFile) {
	for (tmp=rtrn;*tmp!='\0';tmp++) {
	    if ((tmp==rtrn)&&(!isalpha(*tmp)))
		*tmp= '_';
	    else if (!isalnum(*tmp))
		*tmp= '_';
	}
    }
    return XkbStringText(rtrn,format);
}

/***====================================================================***/

char *
XkbVModIndexText(Display *dpy,XkbDescPtr xkb,unsigned ndx,unsigned format)
{
register int len;
register Atom *vmodNames;
char *rtrn,*tmp;

    if (xkb && xkb->names)
	 vmodNames= xkb->names->vmods;
    else vmodNames= NULL;

    tmp= NULL;
    if (ndx>=XkbNumVirtualMods)
	 tmp= strdup("illegal");
    else if (vmodNames&&(vmodNames[ndx]!=None))
	 tmp= XkbAtomGetString(dpy,vmodNames[ndx]);
    if (tmp==NULL) {
        tmp= (char *)_XkbAlloc(20 * sizeof(char));
	snprintf(tmp,20,"%d",ndx);
    }

    len= strlen(tmp)+1;
    if (format==XkbCFile)
	len+= 4;
    if (len>=BUFFER_SIZE)
	len= BUFFER_SIZE-1;
    rtrn= tbGetBuffer(len);
    if (format==XkbCFile) {
         snprintf(rtrn, len, "vmod_%s", tmp);
    }
    else strncpy(rtrn,tmp,len);
    _XkbFree(tmp);
    return rtrn;
}

char *
XkbVModMaskText(	Display *	dpy,
			XkbDescPtr	xkb,
			unsigned	modMask,
			unsigned	mask,
			unsigned	format)
{
register int i,bit;
int	 len;
char *mm,*rtrn;
char *str,buf[BUFFER_SIZE];

    if ((modMask==0)&&(mask==0)) {
	rtrn= tbGetBuffer(5);
	if (format==XkbCFile)
	     sprintf(rtrn,"0");
	else sprintf(rtrn,"none");
	return rtrn;
    }
    if (modMask!=0)
	 mm= XkbModMaskText(modMask,format);
    else mm= NULL;

    str= buf;
    buf[0]= '\0';
    if (mask) {
	char *tmp;
	for (i=0,bit=1;i<XkbNumVirtualMods;i++,bit<<=1) {
	    if (mask&bit) {
		tmp= XkbVModIndexText(dpy,xkb,i,format);
		len= strlen(tmp)+1+(str==buf?0:1);
		if (format==XkbCFile)
		    len+= 4;
		if ((str-(buf+len))<=BUFFER_SIZE) {
		    if (str!=buf) {
			if (format==XkbCFile)	*str++= '|';
			else			*str++= '+';
			len--;
		    }
		}
		if (format==XkbCFile)
		     sprintf(str,"%sMask",tmp);
		else strcpy(str,tmp);
		str= &str[len-1];
	    }
	}
	str= buf;
    }
    else str= NULL;
    if (mm)	
	len= strlen(mm);
    else	len= 0;
    if (str)
	len+= strlen(str)+(mm==NULL?0:1);
    if (len>=BUFFER_SIZE)
	len= BUFFER_SIZE-1;
    rtrn= tbGetBuffer(len+1);
    rtrn[0]= '\0';

    if (mm!=NULL) {
	i= strlen(mm);
	if (i>len)
	    i= len;
	strcpy(rtrn,mm);
    }
    else {
	i=0;
    }
    if (str!=NULL) {
	if (mm!=NULL) {
	    if (format==XkbCFile)	strcat(rtrn,"|");
	    else			strcat(rtrn,"+");
	}
	strncat(rtrn,str,len-i);
    }
    rtrn[len]= '\0';
    return rtrn;
}

static char *modNames[XkbNumModifiers] = {
    "Shift", "Lock", "Control", "Mod1", "Mod2", "Mod3", "Mod4", "Mod5"
};

char *
XkbModIndexText(unsigned ndx,unsigned format)
{
char *	rtrn;
char	buf[100];

    if (format==XkbCFile) {
	if (ndx<XkbNumModifiers)
	     sprintf(buf,"%sMapIndex",modNames[ndx]);
	else if (ndx==XkbNoModifier)
	     sprintf(buf,"XkbNoModifier");
	else sprintf(buf,"0x%02x",ndx);
    }
    else {
	if (ndx<XkbNumModifiers)
	     strcpy(buf,modNames[ndx]);
	else if (ndx==XkbNoModifier)
	     strcpy(buf,"none");
	else sprintf(buf,"ILLEGAL_%02x",ndx);
    }
    rtrn= tbGetBuffer(strlen(buf)+1);
    strcpy(rtrn,buf);
    return rtrn;
}

char *
XkbModMaskText(unsigned mask,unsigned format)
{
register int i,bit;
char buf[64],*rtrn;

    if ((mask&0xff)==0xff) {
	if (format==XkbCFile) 		strcpy(buf,"0xff");
	else				strcpy(buf,"all");
    }
    else if ((mask&0xff)==0) {
	if (format==XkbCFile)		strcpy(buf,"0");
	else				strcpy(buf,"none");
    }
    else {
	char *str= buf;
	buf[0]= '\0';
	for (i=0,bit=1;i<XkbNumModifiers;i++,bit<<=1) {
	    if (mask&bit) {
		if (str!=buf) {
		    if (format==XkbCFile)	*str++= '|';
		    else			*str++= '+';
		}
		strcpy(str,modNames[i]);
		str= &str[strlen(str)];
		if (format==XkbCFile) {
		    strcpy(str,"Mask");
		    str+= 4;
		}
	    }
	}
    }
    rtrn= tbGetBuffer(strlen(buf)+1);
    strcpy(rtrn,buf);
    return rtrn;
}

/***====================================================================***/

/*ARGSUSED*/
char *
XkbConfigText(unsigned config,unsigned format)
{
static char *buf;

    buf= tbGetBuffer(32);
    switch (config) {
	case XkmSemanticsFile:
	    strcpy(buf,"Semantics");
	    break;
	case XkmLayoutFile:
	    strcpy(buf,"Layout");
	    break;
	case XkmKeymapFile:
	    strcpy(buf,"Keymap");
	    break;
	case XkmGeometryFile:
	case XkmGeometryIndex:
	    strcpy(buf,"Geometry");
	    break;
	case XkmTypesIndex:
	    strcpy(buf,"Types");
	    break;
	case XkmCompatMapIndex:
	    strcpy(buf,"CompatMap");
	    break;
	case XkmSymbolsIndex:
	    strcpy(buf,"Symbols");
	    break;
	case XkmIndicatorsIndex:
	    strcpy(buf,"Indicators");
	    break;
	case XkmKeyNamesIndex:
	    strcpy(buf,"KeyNames");
	    break;
	case XkmVirtualModsIndex:
	    strcpy(buf,"VirtualMods");
	    break;
	default:
	    sprintf(buf,"unknown(%d)",config);
	    break;
    }
    return buf;
}

/***====================================================================***/

char *
XkbKeysymText(KeySym sym,unsigned format)
{
static char buf[32],*rtrn;

#ifndef XKB_IN_SERVER
    if (sym==NoSymbol)
	strcpy(rtrn=buf,"NoSymbol");
    else if ((rtrn=XKeysymToString(sym))==NULL)
	sprintf(rtrn=buf, "0x%lx", (long)sym);
    else if (format==XkbCFile) {
	sprintf(buf,"XK_%s",rtrn);
	rtrn= buf;
    }
    return rtrn;
#else /* def XKB_IN_SERVER */
    if (sym==NoSymbol)
	 strcpy(rtrn=buf,"NoSymbol");
    else sprintf(rtrn=buf, "0x%lx", (long)sym);
    return rtrn;
#endif /* XKB_IN_SERVER */
}

char *
XkbKeyNameText(char *name,unsigned format)
{
char *buf;

    if (format==XkbCFile) {
	buf= tbGetBuffer(5);
	memcpy(buf,name,4);
	buf[4]= '\0';
    }
    else {
	int len;
	buf= tbGetBuffer(7);
	buf[0]= '<';
	memcpy(&buf[1],name,4);
	buf[5]= '\0';
	len= strlen(buf);
	buf[len++]= '>';
	buf[len]= '\0';
    }
    return buf;
}

/***====================================================================***/

static char *siMatchText[5] = {
	"NoneOf", "AnyOfOrNone", "AnyOf", "AllOf", "Exactly"
};

char *
XkbSIMatchText(unsigned type,unsigned format)
{
static char buf[40];
char *rtrn;

    switch (type&XkbSI_OpMask) {
	case XkbSI_NoneOf:	rtrn= siMatchText[0]; break;
	case XkbSI_AnyOfOrNone:	rtrn= siMatchText[1]; break;
	case XkbSI_AnyOf:	rtrn= siMatchText[2]; break;
	case XkbSI_AllOf:	rtrn= siMatchText[3]; break;
	case XkbSI_Exactly:	rtrn= siMatchText[4]; break;
	default:		sprintf(buf,"0x%x",type&XkbSI_OpMask);
				return buf;
    }
    if (format==XkbCFile) {
	if (type&XkbSI_LevelOneOnly)
	     sprintf(buf,"XkbSI_LevelOneOnly|XkbSI_%s",rtrn);
	else sprintf(buf,"XkbSI_%s",rtrn);
	rtrn= buf;
    }
    return rtrn;
}

/***====================================================================***/

static char *imWhichNames[]= {
	"base",
	"latched",
	"locked",
	"effective",
	"compat"
};

char *
XkbIMWhichStateMaskText(unsigned use_which,unsigned format)
{
int		len;
unsigned	i,bit,tmp;
char *		buf;

    if (use_which==0) {
	buf= tbGetBuffer(2);
	strcpy(buf,"0");
	return buf;
    }
    tmp= use_which&XkbIM_UseAnyMods;
    for (len=i=0,bit=1;tmp!=0;i++,bit<<=1) {
	if (tmp&bit) {
	    tmp&= ~bit;
	    len+= strlen(imWhichNames[i])+1;
	    if (format==XkbCFile)
		len+= 9;
	}
    }
    buf= tbGetBuffer(len+1);
    tmp= use_which&XkbIM_UseAnyMods;
    for (len=i=0,bit=1;tmp!=0;i++,bit<<=1) {
	if (tmp&bit) {
	    tmp&= ~bit;
	    if (format==XkbCFile) {
		if (len!=0)
		    buf[len++]= '|';
		sprintf(&buf[len],"XkbIM_Use%s",imWhichNames[i]);
		buf[len+9]= toupper(buf[len+9]);
	    }
	    else {
		if (len!=0)
		    buf[len++]= '+';
		sprintf(&buf[len],"%s",imWhichNames[i]);
	    }
	    len+= strlen(&buf[len]);
	}
    }
    return buf;
}

char *
XkbAccessXDetailText(unsigned state,unsigned format)
{
char *buf,*prefix;

    buf= tbGetBuffer(32);
    if (format==XkbMessage)	prefix= "";
    else			prefix= "XkbAXN_";
    switch (state){
	case XkbAXN_SKPress:	sprintf(buf,"%sSKPress",prefix); break;
	case XkbAXN_SKAccept:	sprintf(buf,"%sSKAccept",prefix); break;
	case XkbAXN_SKRelease:	sprintf(buf,"%sSKRelease",prefix); break;
	case XkbAXN_SKReject:	sprintf(buf,"%sSKReject",prefix); break;
	case XkbAXN_BKAccept:	sprintf(buf,"%sBKAccept",prefix); break;
	case XkbAXN_BKReject:	sprintf(buf,"%sBKReject",prefix); break;
	case XkbAXN_AXKWarning:	sprintf(buf,"%sAXKWarning",prefix); break;
	default:		sprintf(buf,"ILLEGAL"); break;
    }
    return buf;
}

static char *nknNames[] = {
	"keycodes", "geometry", "deviceID"
};
#define	NUM_NKN	(sizeof(nknNames)/sizeof(char *))

char *
XkbNKNDetailMaskText(unsigned detail,unsigned format)
{
char *buf,*prefix,*suffix;
register int 		i;
register unsigned	bit;
int			len,plen,slen;


    if ((detail&XkbAllNewKeyboardEventsMask)==0) {
	char *tmp = "";
	if (format==XkbCFile)			tmp= "0";
	else if (format==XkbMessage)		tmp= "none";
	buf=  tbGetBuffer(strlen(tmp)+1);
	strcpy(buf,tmp);
	return buf;
    }
    else if ((detail&XkbAllNewKeyboardEventsMask)==XkbAllNewKeyboardEventsMask){
	char *	tmp;
	if (format==XkbCFile)		tmp= "XkbAllNewKeyboardEventsMask";
	else 				tmp= "all";
	buf=  tbGetBuffer(strlen(tmp)+1);
	strcpy(buf,tmp);
	return buf;
    }
    if (format==XkbMessage) {
	prefix= "";
	suffix= "";
	slen= plen= 0;
    }
    else {
	prefix= "XkbNKN_";
	plen= 7;
	if (format==XkbCFile)
	     suffix= "Mask";
	else suffix= "";
	slen= strlen(suffix);
    }
    for (len=0,i=0,bit=1;i<NUM_NKN;i++,bit<<=1) {
	if (detail&bit) {
	    if (len!=0)	len+= 1;	/* room for '+' or '|' */
	    len+= plen+slen+strlen(nknNames[i]);
	}
    }
    buf= tbGetBuffer(len+1);
    buf[0]= '\0';
    for (len=0,i=0,bit=1;i<NUM_NKN;i++,bit<<=1) {
	if (detail&bit) {
	    if (len!=0) {
		if (format==XkbCFile)	buf[len++]= '|';
		else			buf[len++]= '+';
	    }
	    if (plen) {
		strcpy(&buf[len],prefix);
		len+= plen;
	    }
	    strcpy(&buf[len],nknNames[i]);
	    len+= strlen(nknNames[i]);
	    if (slen) {
		strcpy(&buf[len],suffix);
		len+= slen;
	    }
	}
    }
    buf[len++]= '\0';
    return buf;
}

static char *ctrlNames[] = {
	"repeatKeys",
	"slowKeys",
	"bounceKeys",
	"stickyKeys",
	"mouseKeys",
	"mouseKeysAccel",
	"accessXKeys",
	"accessXTimeout",
	"accessXFeedback",
	"audibleBell",
	"overlay1",
	"overlay2",
	"ignoreGroupLock"
};

char *
XkbControlsMaskText(unsigned ctrls,unsigned format)
{
int		len;
unsigned	i,bit,tmp;
char *		buf;

    if (ctrls==0) {
	buf= tbGetBuffer(5);
	if (format==XkbCFile)
	     strcpy(buf,"0");
	else strcpy(buf,"none");
	return buf;
    }
    tmp= ctrls&XkbAllBooleanCtrlsMask;
    for (len=i=0,bit=1;tmp!=0;i++,bit<<=1) {
	if (tmp&bit) {
	    tmp&= ~bit;
	    len+= strlen(ctrlNames[i])+1;
	    if (format==XkbCFile)
		len+= 7;
	}
    }
    buf= tbGetBuffer(len+1);
    tmp= ctrls&XkbAllBooleanCtrlsMask;
    for (len=i=0,bit=1;tmp!=0;i++,bit<<=1) {
	if (tmp&bit) {
	    tmp&= ~bit;
	    if (format==XkbCFile) {
		if (len!=0)
		    buf[len++]= '|';
		sprintf(&buf[len],"Xkb%sMask",ctrlNames[i]);
		buf[len+3]= toupper(buf[len+3]);
	    }
	    else {
		if (len!=0)
		    buf[len++]= '+';
		sprintf(&buf[len],"%s",ctrlNames[i]);
	    }
	    len+= strlen(&buf[len]);
	}
    }
    return buf;
}

/***====================================================================***/

char *
XkbStringText(char *str,unsigned format)
{
char *	buf;
register char *in,*out;
int	len;
Bool	ok;

    if (str==NULL) {
	buf= tbGetBuffer(2);
	buf[0]='\0';
	return buf;
    }
    else if (format==XkbXKMFile)
	return str;
    for (ok= True,len=0,in=str;*in!='\0';in++,len++) {
	if (!isprint(*in)) {
	    ok= False;
	    switch (*in) {
		case '\n': case '\t': case '\v':
		case '\b': case '\r': case '\f':
		    len++;
		    break;
		default:
		    len+= 4;
		    break;
	    }
	}
    }
    if (ok)
	return str;
    buf= tbGetBuffer(len+1);
    for (in=str,out=buf;*in!='\0';in++) {
	if (isprint(*in))
	    *out++= *in;
	else {
	    *out++= '\\';
	    if (*in=='\n')	*out++= 'n';
	    else if (*in=='\t')	*out++= 't';
	    else if (*in=='\v')	*out++= 'v';
	    else if (*in=='\b')	*out++= 'b';
	    else if (*in=='\r')	*out++= 'r';
	    else if (*in=='\f')	*out++= 'f';
	    else if ((*in=='\033')&&(format==XkbXKMFile)) {
		*out++= 'e';
	    }
	    else {
		*out++= '0';
		sprintf(out,"%o",*in);
		while (*out!='\0')
		    out++;
	    }
	}
    }
    *out++= '\0';
    return buf;
}

/***====================================================================***/

char *
XkbGeomFPText(int val,unsigned format)
{
int	whole,frac;
char *	buf;

    buf= tbGetBuffer(12);
    if (format==XkbCFile) {
	sprintf(buf,"%d",val);
    }
    else {
	whole= val/XkbGeomPtsPerMM;
	frac= val%XkbGeomPtsPerMM;
	if (frac!=0)
	     sprintf(buf,"%d.%d",whole,frac);
	else sprintf(buf,"%d",whole);
    }
    return buf;
}

char *
XkbDoodadTypeText(unsigned type,unsigned format)
{
char *	buf;
    if (format==XkbCFile) {
	buf= tbGetBuffer(24);
	if (type==XkbOutlineDoodad)	   strcpy(buf,"XkbOutlineDoodad");
	else if (type==XkbSolidDoodad)	   strcpy(buf,"XkbSolidDoodad");
	else if (type==XkbTextDoodad)	   strcpy(buf,"XkbTextDoodad");
	else if (type==XkbIndicatorDoodad) strcpy(buf,"XkbIndicatorDoodad");
	else if (type==XkbLogoDoodad)	   strcpy(buf,"XkbLogoDoodad");
	else				   sprintf(buf,"UnknownDoodad%d",type);
    }
    else {
	buf= tbGetBuffer(12);
	if (type==XkbOutlineDoodad)	   strcpy(buf,"outline");
	else if (type==XkbSolidDoodad)	   strcpy(buf,"solid");
	else if (type==XkbTextDoodad)	   strcpy(buf,"text");
	else if (type==XkbIndicatorDoodad) strcpy(buf,"indicator");
	else if (type==XkbLogoDoodad)	   strcpy(buf,"logo");
	else				   sprintf(buf,"unknown%d",type);
    }
    return buf;
}

static char *actionTypeNames[XkbSA_NumActions]= {
    "NoAction", 
    "SetMods",      "LatchMods",    "LockMods", 
    "SetGroup",     "LatchGroup",   "LockGroup",
    "MovePtr",
    "PtrBtn",       "LockPtrBtn",
    "SetPtrDflt",
    "ISOLock",
    "Terminate",    "SwitchScreen", 
    "SetControls",  "LockControls",
    "ActionMessage",
    "RedirectKey",
    "DeviceBtn",    "LockDeviceBtn"
};

char *
XkbActionTypeText(unsigned type,unsigned format)
{
static char buf[32];
char *rtrn;

    if (type<=XkbSA_LastAction) {
	rtrn= actionTypeNames[type];
	if (format==XkbCFile) {
	    sprintf(buf,"XkbSA_%s",rtrn);
	    return buf;
	}
	return rtrn;
    }
    sprintf(buf,"Private");
    return buf;
}

/***====================================================================***/

static int
TryCopyStr(char *to,char *from,int *pLeft)
{
register int len;
    if (*pLeft>0) {
	len= strlen(from);
	if (len<((*pLeft)-3)) {
	    strcat(to,from);
	    *pLeft-= len;
	    return True;
	}
    }
    *pLeft= -1;
    return False;
}

/*ARGSUSED*/
static Bool
CopyNoActionArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,int*sz)
{
    return True;
}

static Bool
CopyModActionArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,
								int* sz)
{
XkbModAction *	act;
unsigned	tmp;

    act= &action->mods;
    tmp= XkbModActionVMods(act);
    TryCopyStr(buf,"modifiers=",sz);
    if (act->flags&XkbSA_UseModMapMods)
	  TryCopyStr(buf,"modMapMods",sz);
    else if (act->real_mods || tmp) {
	 TryCopyStr(buf,
		     XkbVModMaskText(dpy,xkb,act->real_mods,tmp,XkbXKBFile),
		     sz);
    }
    else TryCopyStr(buf,"none",sz);
    if (act->type==XkbSA_LockMods)
	return True;
    if (act->flags&XkbSA_ClearLocks)
	TryCopyStr(buf,",clearLocks",sz);
    if (act->flags&XkbSA_LatchToLock)
	TryCopyStr(buf,",latchToLock",sz);
    return True;
}

/*ARGSUSED*/
static Bool
CopyGroupActionArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,
								int *sz)
{
XkbGroupAction *	act;
char			tbuf[32];

    act= &action->group;
    TryCopyStr(buf,"group=",sz);
    if (act->flags&XkbSA_GroupAbsolute)
	 sprintf(tbuf,"%d",XkbSAGroup(act)+1);
    else if (XkbSAGroup(act)<0)
	 sprintf(tbuf,"%d",XkbSAGroup(act));
    else sprintf(tbuf,"+%d",XkbSAGroup(act));
    TryCopyStr(buf,tbuf,sz);
    if (act->type==XkbSA_LockGroup)
	return True;
    if (act->flags&XkbSA_ClearLocks)
	TryCopyStr(buf,",clearLocks",sz);
    if (act->flags&XkbSA_LatchToLock)
	TryCopyStr(buf,",latchToLock",sz);
    return True;
}

/*ARGSUSED*/
static Bool
CopyMovePtrArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,int *sz)
{
XkbPtrAction *	act;
int		x,y;
char		tbuf[32];

    act= &action->ptr;
    x= XkbPtrActionX(act);
    y= XkbPtrActionY(act);
    if ((act->flags&XkbSA_MoveAbsoluteX)||(x<0))
	 sprintf(tbuf,"x=%d",x);
    else sprintf(tbuf,"x=+%d",x);
    TryCopyStr(buf,tbuf,sz);

    if ((act->flags&XkbSA_MoveAbsoluteY)||(y<0))
	 sprintf(tbuf,",y=%d",y);
    else sprintf(tbuf,",y=+%d",y);
    TryCopyStr(buf,tbuf,sz);
    if (act->flags&XkbSA_NoAcceleration)
	TryCopyStr(buf,",!accel",sz);
    return True;
}

/*ARGSUSED*/
static Bool
CopyPtrBtnArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,int *sz)
{
XkbPtrBtnAction *	act;
char			tbuf[32];

    act= &action->btn;
    TryCopyStr(buf,"button=",sz);
    if ((act->button>0)&&(act->button<6)) {
	 sprintf(tbuf,"%d",act->button);
	 TryCopyStr(buf,tbuf,sz);
    }
    else TryCopyStr(buf,"default",sz);
    if (act->count>0) {
	sprintf(tbuf,",count=%d",act->count);
	TryCopyStr(buf,tbuf,sz);
    }
    if (action->type==XkbSA_LockPtrBtn) {
	switch (act->flags&(XkbSA_LockNoUnlock|XkbSA_LockNoLock)) {
	    case XkbSA_LockNoLock:
		sprintf(tbuf,",affect=unlock"); break;
	    case XkbSA_LockNoUnlock:
		sprintf(tbuf,",affect=lock"); break;
	    case XkbSA_LockNoUnlock|XkbSA_LockNoLock:
		sprintf(tbuf,",affect=neither"); break;
	    default:
		sprintf(tbuf,",affect=both"); break;
	}
	TryCopyStr(buf,tbuf,sz);
    }
    return True;
}

/*ARGSUSED*/
static Bool
CopySetPtrDfltArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,
								int *sz)
{
XkbPtrDfltAction *	act;
char			tbuf[32];

    act= &action->dflt;
    if (act->affect==XkbSA_AffectDfltBtn) {
	TryCopyStr(buf,"affect=button,button=",sz);
	if ((act->flags&XkbSA_DfltBtnAbsolute)||(XkbSAPtrDfltValue(act)<0))
	     sprintf(tbuf,"%d",XkbSAPtrDfltValue(act));
	else sprintf(tbuf,"+%d",XkbSAPtrDfltValue(act));
	TryCopyStr(buf,tbuf,sz);
    }
    return True;
}

static Bool
CopyISOLockArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,int *sz)
{
XkbISOAction *	act;
char		tbuf[64];

    act= &action->iso;
    if (act->flags&XkbSA_ISODfltIsGroup) {
	TryCopyStr(tbuf,"group=",sz);
	if (act->flags&XkbSA_GroupAbsolute)
	     sprintf(tbuf,"%d",XkbSAGroup(act)+1);
	else if (XkbSAGroup(act)<0)
	     sprintf(tbuf,"%d",XkbSAGroup(act));
	else sprintf(tbuf,"+%d",XkbSAGroup(act));
	TryCopyStr(buf,tbuf,sz);
    }
    else {
	unsigned tmp;
	tmp= XkbModActionVMods(act);
	TryCopyStr(buf,"modifiers=",sz);
	if (act->flags&XkbSA_UseModMapMods)
	     TryCopyStr(buf,"modMapMods",sz);
	else if (act->real_mods || tmp) {
	    if (act->real_mods) {
		TryCopyStr(buf,XkbModMaskText(act->real_mods,XkbXKBFile),sz);
		if (tmp)
		    TryCopyStr(buf,"+",sz);
	    }
	    if (tmp)
		TryCopyStr(buf,XkbVModMaskText(dpy,xkb,0,tmp,XkbXKBFile),sz);
	}
	else TryCopyStr(buf,"none",sz);
    }
    TryCopyStr(buf,",affect=",sz);
    if ((act->affect&XkbSA_ISOAffectMask)==0)
	TryCopyStr(buf,"all",sz);
    else {
	int nOut= 0;
	if ((act->affect&XkbSA_ISONoAffectMods)==0) {
	    TryCopyStr(buf,"mods",sz);
	    nOut++;
	}
	if ((act->affect&XkbSA_ISONoAffectGroup)==0) {
	    sprintf(tbuf,"%sgroups",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if ((act->affect&XkbSA_ISONoAffectPtr)==0) {
	    sprintf(tbuf,"%spointer",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if ((act->affect&XkbSA_ISONoAffectCtrls)==0) {
	    sprintf(tbuf,"%scontrols",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
    }
    return True;
}

/*ARGSUSED*/
static Bool
CopySwitchScreenArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,
								int *sz)
{
XkbSwitchScreenAction *	act;
char			tbuf[32];

    act= &action->screen;
    if ((act->flags&XkbSA_SwitchAbsolute)||(XkbSAScreen(act)<0))
	 sprintf(tbuf,"screen=%d",XkbSAScreen(act));
    else sprintf(tbuf,"screen=+%d",XkbSAScreen(act));
    TryCopyStr(buf,tbuf,sz);
    if (act->flags&XkbSA_SwitchApplication)
	 TryCopyStr(buf,",!same",sz);
    else TryCopyStr(buf,",same",sz);
    return True;
}

/*ARGSUSED*/
static Bool
CopySetLockControlsArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,
							char *buf,int *sz)
{
XkbCtrlsAction *	act;
unsigned		tmp;
char			tbuf[32];

    act= &action->ctrls;
    tmp= XkbActionCtrls(act);
    TryCopyStr(buf,"controls=",sz);
    if (tmp==0)
	TryCopyStr(buf,"none",sz);
    else if ((tmp&XkbAllBooleanCtrlsMask)==XkbAllBooleanCtrlsMask)
	TryCopyStr(buf,"all",sz);
    else {
	int nOut= 0;
	if (tmp&XkbRepeatKeysMask) {
	    sprintf(tbuf,"%sRepeatKeys",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbSlowKeysMask) {
	    sprintf(tbuf,"%sSlowKeys",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbBounceKeysMask) {
	    sprintf(tbuf,"%sBounceKeys",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbStickyKeysMask) {
	    sprintf(tbuf,"%sStickyKeys",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbMouseKeysMask) {
	    sprintf(tbuf,"%sMouseKeys",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbMouseKeysAccelMask) {
	    sprintf(tbuf,"%sMouseKeysAccel",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbAccessXKeysMask) {
	    sprintf(tbuf,"%sAccessXKeys",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbAccessXTimeoutMask) {
	    sprintf(tbuf,"%sAccessXTimeout",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbAccessXFeedbackMask) {
	    sprintf(tbuf,"%sAccessXFeedback",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbAudibleBellMask) {
	    sprintf(tbuf,"%sAudibleBell",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbOverlay1Mask) {
	    sprintf(tbuf,"%sOverlay1",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbOverlay2Mask) {
	    sprintf(tbuf,"%sOverlay2",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
	if (tmp&XkbIgnoreGroupLockMask) {
	    sprintf(tbuf,"%sIgnoreGroupLock",(nOut>0?"+":""));
	    TryCopyStr(buf,tbuf,sz);
	    nOut++;
	}
    }
    return True;
}

/*ARGSUSED*/
static Bool
CopyActionMessageArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,
								int *sz)
{
XkbMessageAction *	act;
unsigned		all;
char			tbuf[32];

    act= &action->msg;
    all= XkbSA_MessageOnPress|XkbSA_MessageOnRelease;
    TryCopyStr(buf,"report=",sz);
    if ((act->flags&all)==0)
	TryCopyStr(buf,"none",sz);
    else if ((act->flags&all)==all)
	TryCopyStr(buf,"all",sz);
    else if (act->flags&XkbSA_MessageOnPress)
	 TryCopyStr(buf,"KeyPress",sz);
    else TryCopyStr(buf,"KeyRelease",sz);
    sprintf(tbuf,",data[0]=0x%02x",act->message[0]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[1]=0x%02x",act->message[1]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[2]=0x%02x",act->message[2]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[3]=0x%02x",act->message[3]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[4]=0x%02x",act->message[4]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[5]=0x%02x",act->message[5]); TryCopyStr(buf,tbuf,sz);
    return True;
}

static Bool
CopyRedirectKeyArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,
								int *sz)
{
XkbRedirectKeyAction *	act;
char			tbuf[32],*tmp;
unsigned		kc;
unsigned		vmods,vmods_mask;

    act= &action->redirect;
    kc= act->new_key;
    vmods= XkbSARedirectVMods(act);
    vmods_mask= XkbSARedirectVModsMask(act);
    if (xkb && xkb->names && xkb->names->keys && (kc<=xkb->max_key_code) &&
				(xkb->names->keys[kc].name[0]!='\0')) {
	char *kn;
	kn= XkbKeyNameText(xkb->names->keys[kc].name,XkbXKBFile);
	sprintf(tbuf,"key=%s",kn);
    }
    else sprintf(tbuf,"key=%d",kc);
    TryCopyStr(buf,tbuf,sz);
    if ((act->mods_mask==0)&&(vmods_mask==0))
	return True;
    if ((act->mods_mask==XkbAllModifiersMask)&&
	(vmods_mask==XkbAllVirtualModsMask)) {
	tmp= XkbVModMaskText(dpy,xkb,act->mods,vmods,XkbXKBFile);
	TryCopyStr(buf,",mods=",sz);
	TryCopyStr(buf,tmp,sz);
    }
    else {
	if ((act->mods_mask&act->mods)||(vmods_mask&vmods)) {
	    tmp= XkbVModMaskText(dpy,xkb,act->mods_mask&act->mods,
					 vmods_mask&vmods,XkbXKBFile);
	    TryCopyStr(buf,",mods= ",sz);
	    TryCopyStr(buf,tmp,sz);
	}
	if ((act->mods_mask&(~act->mods))||(vmods_mask&(~vmods))) {
	    tmp= XkbVModMaskText(dpy,xkb,act->mods_mask&(~act->mods),
					 vmods_mask&(~vmods),XkbXKBFile);
	    TryCopyStr(buf,",clearMods= ",sz);
	    TryCopyStr(buf,tmp,sz);
	}
    }
    return True;
}

/*ARGSUSED*/
static Bool
CopyDeviceBtnArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,
								int *sz)
{
XkbDeviceBtnAction *	act;
char			tbuf[32];

    act= &action->devbtn;
    sprintf(tbuf,"device= %d",act->device); TryCopyStr(buf,tbuf,sz);
    TryCopyStr(buf,",button=",sz);
    sprintf(tbuf,"%d",act->button);
    TryCopyStr(buf,tbuf,sz);
    if (act->count>0) {
	sprintf(tbuf,",count=%d",act->count);
	TryCopyStr(buf,tbuf,sz);
    }
    if (action->type==XkbSA_LockDeviceBtn) {
	switch (act->flags&(XkbSA_LockNoUnlock|XkbSA_LockNoLock)) {
	    case XkbSA_LockNoLock:
		sprintf(tbuf,",affect=unlock"); break;
	    case XkbSA_LockNoUnlock:
		sprintf(tbuf,",affect=lock"); break;
	    case XkbSA_LockNoUnlock|XkbSA_LockNoLock:
		sprintf(tbuf,",affect=neither"); break;
	    default:
		sprintf(tbuf,",affect=both"); break;
	}
	TryCopyStr(buf,tbuf,sz);
    }
    return True;
}

/*ARGSUSED*/
static Bool
CopyOtherArgs(Display *dpy,XkbDescPtr xkb,XkbAction *action,char *buf,int *sz)
{
XkbAnyAction *	act;
char		tbuf[32];

    act= &action->any;
    sprintf(tbuf,"type=0x%02x",act->type); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[0]=0x%02x",act->data[0]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[1]=0x%02x",act->data[1]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[2]=0x%02x",act->data[2]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[3]=0x%02x",act->data[3]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[4]=0x%02x",act->data[4]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[5]=0x%02x",act->data[5]); TryCopyStr(buf,tbuf,sz);
    sprintf(tbuf,",data[6]=0x%02x",act->data[6]); TryCopyStr(buf,tbuf,sz);
    return True;
}

typedef	Bool	(*actionCopy)(
	Display *	/* dpy */,
	XkbDescPtr 	/* xkb */,
	XkbAction *	/* action */,
	char *		/* buf */,
	int*		/* sz */
);
static actionCopy	copyActionArgs[XkbSA_NumActions] = {
	CopyNoActionArgs		/* NoAction	*/,
	CopyModActionArgs		/* SetMods	*/,
	CopyModActionArgs		/* LatchMods	*/,
	CopyModActionArgs		/* LockMods	*/,
	CopyGroupActionArgs		/* SetGroup	*/,
	CopyGroupActionArgs		/* LatchGroup	*/,
	CopyGroupActionArgs		/* LockGroup	*/,
	CopyMovePtrArgs			/* MovePtr	*/,
	CopyPtrBtnArgs			/* PtrBtn	*/,
	CopyPtrBtnArgs			/* LockPtrBtn	*/,
	CopySetPtrDfltArgs		/* SetPtrDflt	*/,
	CopyISOLockArgs			/* ISOLock	*/,
	CopyNoActionArgs		/* Terminate	*/,
	CopySwitchScreenArgs		/* SwitchScreen	*/,
	CopySetLockControlsArgs		/* SetControls	*/,
	CopySetLockControlsArgs		/* LockControls	*/,
	CopyActionMessageArgs		/* ActionMessage*/,
	CopyRedirectKeyArgs		/* RedirectKey	*/,
	CopyDeviceBtnArgs		/* DeviceBtn	*/,
	CopyDeviceBtnArgs		/* LockDeviceBtn*/
};

#define	ACTION_SZ	256

char *
XkbActionText(Display *dpy,XkbDescPtr xkb,XkbAction *action,unsigned format)
{
char	buf[ACTION_SZ],*tmp;
int	sz;

    if (format==XkbCFile) {
	sprintf(buf,
	    "{ %20s, { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x } }",
	    XkbActionTypeText(action->type,XkbCFile),
	    action->any.data[0],action->any.data[1],action->any.data[2],
	    action->any.data[3],action->any.data[4],action->any.data[5],
	    action->any.data[6]);
    }
    else {
	sprintf(buf,"%s(",XkbActionTypeText(action->type,XkbXKBFile));
	sz= ACTION_SZ-strlen(buf)+2; /* room for close paren and NULL */
	if (action->type<(unsigned)XkbSA_NumActions)
	     (*copyActionArgs[action->type])(dpy,xkb,action,buf,&sz);
	else CopyOtherArgs(dpy,xkb,action,buf,&sz);
	TryCopyStr(buf,")",&sz);
    }
    tmp= tbGetBuffer(strlen(buf)+1);
    if (tmp!=NULL)
	strcpy(tmp,buf);
    return tmp;
}

char *
XkbBehaviorText(XkbDescPtr xkb,XkbBehavior *behavior,unsigned format)
{
char	buf[256],*tmp;

    if (format==XkbCFile) {
	if (behavior->type==XkbKB_Default)
	     sprintf(buf,"{   0,    0 }");
	else sprintf(buf,"{ %3d, 0x%02x }",behavior->type,behavior->data);
    }
    else {
	unsigned 	type,permanent;
	type= behavior->type&XkbKB_OpMask;
	permanent=((behavior->type&XkbKB_Permanent)!=0);

	if (type==XkbKB_Lock) {
	    sprintf(buf,"lock= %s",(permanent?"Permanent":"True"));
	}
	else if (type==XkbKB_RadioGroup) {
	    int 	g;
	    char	*tmp;
	    g= ((behavior->data)&(~XkbKB_RGAllowNone))+1;
	    if (XkbKB_RGAllowNone&behavior->data) {
		sprintf(buf,"allowNone,");
		tmp= &buf[strlen(buf)];
	    }
	    else tmp= buf;
	    if (permanent)
		 sprintf(tmp,"permanentRadioGroup= %d",g);
	    else sprintf(tmp,"radioGroup= %d",g);
	}
	else if ((type==XkbKB_Overlay1)||(type==XkbKB_Overlay2)) {
	    int ndx,kc;
	    char *kn;

	    ndx= ((type==XkbKB_Overlay1)?1:2);
	    kc= behavior->data;
	    if ((xkb)&&(xkb->names)&&(xkb->names->keys))
		kn= XkbKeyNameText(xkb->names->keys[kc].name,XkbXKBFile);
	    else {
		static char tbuf[8];
		sprintf(tbuf,"%d",kc);
		kn= tbuf;
	    }
	    if (permanent)
		 sprintf(buf,"permanentOverlay%d= %s",ndx,kn);
	    else sprintf(buf,"overlay%d= %s",ndx,kn);
	}
    }
    tmp= tbGetBuffer(strlen(buf)+1);
    if (tmp!=NULL)
	strcpy(tmp,buf);
    return tmp;
}

/***====================================================================***/

char *
XkbIndentText(unsigned size)
{
static char buf[32];
register int i;

    if (size>31)
	size= 31;

    for (i=0;i<size;i++) {
	buf[i]= ' ';
    }
    buf[size]= '\0';
    return buf;
}

#ifndef XKB_IN_SERVER

/***====================================================================***/

#define	PIXEL_MAX	65535

Bool
XkbLookupCanonicalRGBColor(char *def,XColor *color)
{
int     tmp;

    if (_XkbStrCaseEqual(def,"black")) {
	color->red= color->green= color->blue= 0;
	return True;
    }
    else if (_XkbStrCaseEqual(def,"white")) {
	color->red= color->green= color->blue= PIXEL_MAX;
	return True;
    }
    else if ((sscanf(def,"grey%d",&tmp)==1)||
        (sscanf(def,"gray%d",&tmp)==1)||
        (sscanf(def,"Grey%d",&tmp)==1)||
        (sscanf(def,"Gray%d",&tmp)==1)) {
	if ((tmp>0)&&(tmp<=100)) {
	    tmp= (PIXEL_MAX*tmp)/100;
	    color->red= color->green= color->blue= tmp;
	    return True;
	}
    }
    else if ((tmp=(_XkbStrCaseEqual(def,"red")*100))||
             (sscanf(def,"red%d",&tmp)==1)) {
	if ((tmp>0)&&(tmp<=100)) {
	    tmp= (PIXEL_MAX*tmp)/100;
	    color->red= tmp;
	    color->green= color->blue= 0;
	    return True;
	}
    }
    else if ((tmp=(_XkbStrCaseEqual(def,"green")*100))||
             (sscanf(def,"green%d",&tmp)==1)) {
	if ((tmp>0)&&(tmp<=100)) {
	    tmp= (PIXEL_MAX*tmp)/100;
	    color->green= tmp;
	    color->red= color->blue= 0;
	    return True;
	}
    }
    else if ((tmp=(_XkbStrCaseEqual(def,"blue")*100))||
             (sscanf(def,"blue%d",&tmp)==1)) {
	if ((tmp>0)&&(tmp<=100)) {
	    tmp= (PIXEL_MAX*tmp)/100;
	    color->blue= tmp;
	    color->red= color->green= 0;
	    return True;
	}
    }
    else if ((tmp=(_XkbStrCaseEqual(def,"magenta")*100))||
             (sscanf(def,"magenta%d",&tmp)==1)) {
	if ((tmp>0)&&(tmp<=100)) {
	    tmp= (PIXEL_MAX*tmp)/100;
	    color->green= 0;
	    color->red= color->blue= tmp;
	    return True;
	}
    }
    else if ((tmp=(_XkbStrCaseEqual(def,"cyan")*100))||
             (sscanf(def,"cyan%d",&tmp)==1)) {
	if ((tmp>0)&&(tmp<=100)) {
	    tmp= (PIXEL_MAX*tmp)/100;
	    color->red= 0;
	    color->green= color->blue= tmp;
	    return True;
	}
    }
    else if ((tmp=(_XkbStrCaseEqual(def,"yellow")*100))||
             (sscanf(def,"yellow%d",&tmp)==1)) {
	if ((tmp>0)&&(tmp<=100)) {
	    tmp= (PIXEL_MAX*tmp)/100;
	    color->blue= 0;
	    color->red= color->green= tmp;
	    return True;
	}
    }
    return False;
}

#endif
