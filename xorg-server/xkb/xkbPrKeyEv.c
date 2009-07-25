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

#include <stdio.h>
#include <math.h>
#define NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "misc.h"
#include "inputstr.h"
#include "exevents.h"
#include <xkbsrv.h>
#include <ctype.h>

/***====================================================================***/

void
XkbProcessKeyboardEvent(xEvent *xE,DeviceIntPtr keybd,int count)
{
KeyClassPtr	keyc = keybd->key;
XkbSrvInfoPtr	xkbi;
int		key;
XkbBehavior	behavior;
unsigned        ndx;
int             xiEvent;

    xkbi= keyc->xkbInfo;
    key= xE->u.u.detail;
    xiEvent= (xE->u.u.type & EXTENSION_EVENT_BASE);
    if (xkbDebugFlags&0x8) {
	DebugF("[xkb] XkbPKE: Key %d %s\n",key,(xE->u.u.type==KeyPress?"down":"up"));
    }

    if ( (xkbi->repeatKey==key) && (xE->u.u.type==KeyRelease) &&
	 ((xkbi->desc->ctrls->enabled_ctrls&XkbRepeatKeysMask)==0) ) {
	AccessXCancelRepeatKey(xkbi,key);
    }

    behavior= xkbi->desc->server->behaviors[key];
    /* The "permanent" flag indicates a hard-wired behavior that occurs */
    /* below XKB, such as a key that physically locks.   XKB does not   */
    /* do anything to implement the behavior, but it *does* report that */
    /* key is hardwired */

    if ((behavior.type&XkbKB_Permanent)==0) {
	switch (behavior.type) {
	    case XkbKB_Default:
		if (( xE->u.u.type == KeyPress || 
                            xE->u.u.type == DeviceKeyPress) && 
		    (keyc->down[key>>3] & (1<<(key&7)))) {
		    XkbLastRepeatEvent=	(pointer)xE;

                    if (xiEvent)
                        xE->u.u.type = DeviceKeyRelease;
                    else
                        xE->u.u.type = KeyRelease;
		    XkbHandleActions(keybd,keybd,xE,count);

                    if (xiEvent)
                        xE->u.u.type = DeviceKeyPress;
                    else
                        xE->u.u.type = KeyPress;
		    XkbHandleActions(keybd,keybd,xE,count);
		    XkbLastRepeatEvent= NULL;
		    return;
		}
		else if ((xE->u.u.type==KeyRelease || 
                            xE->u.u.type == DeviceKeyRelease) &&
			(!(keyc->down[key>>3]&(1<<(key&7))))) {
		    XkbLastRepeatEvent=	(pointer)&xE;
                    if (xiEvent)
                        xE->u.u.type = DeviceKeyPress;
                    else
                        xE->u.u.type = KeyPress;
		    XkbHandleActions(keybd,keybd,xE,count);
                    if (xiEvent)
                        xE->u.u.type = DeviceKeyRelease;
                    else
                        xE->u.u.type = KeyRelease;
		    XkbHandleActions(keybd,keybd,xE,count);
		    XkbLastRepeatEvent= NULL;
		    return;
		}
		break;
	    case XkbKB_Lock:
		if ( xE->u.u.type == KeyRelease || 
                        xE->u.u.type == DeviceKeyRelease) {
		    return;
                }
		else {
		    int	bit= 1<<(key&7);
		    if ( keyc->down[key>>3]&bit ) {
                        if (xiEvent)
                            xE->u.u.type = DeviceKeyRelease;
                        else
                            xE->u.u.type= KeyRelease;
                    }
                }
		break;
	    case XkbKB_RadioGroup:
		ndx= (behavior.data&(~XkbKB_RGAllowNone));
		if ( ndx<xkbi->nRadioGroups ) {
		    XkbRadioGroupPtr	rg;

		    if ( xE->u.u.type == KeyRelease ||
                            xE->u.u.type == DeviceKeyRelease)
		        return;

		    rg = &xkbi->radioGroups[ndx];
		    if ( rg->currentDown == xE->u.u.detail ) {
		        if (behavior.data&XkbKB_RGAllowNone) {
		            xE->u.u.type = KeyRelease;
			    XkbHandleActions(keybd,keybd,xE,count);
			    rg->currentDown= 0;
		        }
		        return;
		    }
		    if ( rg->currentDown!=0 ) {
			int key = xE->u.u.detail;
                        if (xiEvent)
                            xE->u.u.type = DeviceKeyRelease;
                        else
                            xE->u.u.type= KeyRelease;
			xE->u.u.detail= rg->currentDown;
		        XkbHandleActions(keybd,keybd,xE,count);
                        if (xiEvent)
                            xE->u.u.type = DeviceKeyPress;
                        else
                            xE->u.u.type= KeyPress;
		        xE->u.u.detail= key;
		    }
		    rg->currentDown= key;
		}
		else ErrorF("[xkb] InternalError! Illegal radio group %d\n",ndx);
		break;
	    case XkbKB_Overlay1: case XkbKB_Overlay2:
		{
		    unsigned	which;
		    if (behavior.type==XkbKB_Overlay1)	which= XkbOverlay1Mask;
		    else				which= XkbOverlay2Mask;
		    if ( (xkbi->desc->ctrls->enabled_ctrls&which)==0 )
			break;
		    if ((behavior.data>=xkbi->desc->min_key_code)&&
			(behavior.data<=xkbi->desc->max_key_code)) {
			xE->u.u.detail= behavior.data;
			/* 9/11/94 (ef) -- XXX! need to match release with */
			/*                 press even if the state of the  */
			/*                 corresponding overlay control   */
			/*                 changes while the key is down   */
		    }
		}
		break;
	    default:
		ErrorF("[xkb] unknown key behavior 0x%04x\n",behavior.type);
		break;
	}
    }
    XkbHandleActions(keybd,keybd,xE,count);
    return;
}

void
ProcessKeyboardEvent(xEvent *xE,DeviceIntPtr keybd,int count)
{

    KeyClassPtr keyc = keybd->key;
    XkbSrvInfoPtr xkbi = NULL;
    ProcessInputProc backup_proc;
    xkbDeviceInfoPtr xkb_priv = XKBDEVICEINFO(keybd);
    int is_press = (xE->u.u.type == KeyPress || xE->u.u.type == DeviceKeyPress);
    int is_release = (xE->u.u.type == KeyRelease ||
                      xE->u.u.type == DeviceKeyRelease);

    if (keyc)
        xkbi = keyc->xkbInfo;

    /* We're only interested in key events. */
    if (!is_press && !is_release) {
        UNWRAP_PROCESS_INPUT_PROC(keybd, xkb_priv, backup_proc);
        keybd->public.processInputProc(xE, keybd, count);
        COND_WRAP_PROCESS_INPUT_PROC(keybd, xkb_priv, backup_proc,
                                     xkbUnwrapProc);
        return;
    }

    /* If AccessX filters are active, then pass it through to
     * AccessXFilter{Press,Release}Event; else, punt to
     * XkbProcessKeyboardEvent.
     *
     * If AXF[PK]E don't intercept anything (which they probably won't),
     * they'll punt through XPKE anyway. */
    if ((xkbi->desc->ctrls->enabled_ctrls & XkbAllFilteredEventsMask)) {
        if (is_press)
            AccessXFilterPressEvent(xE, keybd, count);
        else if (is_release)
            AccessXFilterReleaseEvent(xE, keybd, count);

    } else {
        XkbProcessKeyboardEvent(xE, keybd, count);
    }
    
    return;
}
