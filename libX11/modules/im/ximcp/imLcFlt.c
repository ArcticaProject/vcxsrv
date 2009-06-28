/* $Xorg: imLcFlt.c,v 1.3 2000/08/17 19:45:13 cpqbld Exp $ */
/******************************************************************

              Copyright 1992 by Fuji Xerox Co., Ltd.
              Copyright 1992, 1994 by FUJITSU LIMITED

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Fuji Xerox, 
FUJITSU LIMITED not be used in advertising or publicity pertaining
to distribution of the software without specific, written prior
permission. Fuji Xerox, FUJITSU LIMITED make no representations
about the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

FUJI XEROX, FUJITSU LIMITED DISCLAIM ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL FUJI XEROX,
FUJITSU LIMITED BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

  Author   : Kazunori Nishihara	Fuji Xerox
  Modifier : Takashi Fujiwara   FUJITSU LIMITED 
                                fujiwara@a80.tech.yk.fujitsu.co.jp

******************************************************************/

#define NEED_EVENTS
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <X11/keysym.h>
#include "Xlcint.h"
#include "Ximint.h"

Bool
_XimLocalFilter(d, w, ev, client_data)
    Display	*d;
    Window	 w;
    XEvent	*ev;
    XPointer	 client_data;
{
    Xic		 ic = (Xic)client_data;
    KeySym	 keysym;
    static char	 buf[256];
    DefTree	*b = ic->private.local.base.tree;
    DTIndex	 t;

    if(ev->xkey.keycode == 0)
	return (False);

    XLookupString((XKeyEvent *)ev, buf, sizeof(buf), &keysym, NULL);

    if(IsModifierKey(keysym))
	return (False);

    if(keysym >= XK_braille_dot_1 && keysym <= XK_braille_dot_8) {
	if(ev->type == KeyPress) {
	    ic->private.local.brl_pressed |=
		1<<(keysym-XK_braille_dot_1);
	} else {
	    if(!ic->private.local.brl_committing
		    || ev->xkey.time - ic->private.local.brl_release_start > 300) {
	    	ic->private.local.brl_committing = ic->private.local.brl_pressed;
		ic->private.local.brl_release_start = ev->xkey.time;
	    }
	    ic->private.local.brl_pressed &= ~(1<<(keysym-XK_braille_dot_1));
	    if(!ic->private.local.brl_pressed) {
		if(ic->private.local.brl_committing) {
		    ic->private.local.brl_committed =
			ic->private.local.brl_committing;
		    ic->private.local.composed = 0;
		    ev->type = KeyPress;
		    ev->xkey.keycode = 0;
		    _XPutBackEvent(d, ev);
		}
	    }
	}
	return(True);
    }

    if(   (ev->type != KeyPress)
       || (((Xim)ic->core.im)->private.local.top == 0 ) )
	return(False);

    for(t = ic->private.local.context; t; t = b[t].next) {
	if(((ev->xkey.state & b[t].modifier_mask) == b[t].modifier) &&
	   (keysym == b[t].keysym))
	    break;
    }

    if(t) { /* Matched */
	if(b[t].succession) { /* Intermediate */
	    ic->private.local.context = b[t].succession;
	    return(True);
	} else { /* Terminate (reached to leaf) */
	    ic->private.local.composed = t;
	    ic->private.local.brl_committed = 0;
	    /* return back to client KeyPressEvent keycode == 0 */
	    ev->xkey.keycode = 0;
	    _XPutBackEvent(d, ev);
	    /* initialize internal state for next key sequence */
	    ic->private.local.context = ((Xim)ic->core.im)->private.local.top;
	    return(True);
	}
    } else { /* Unmatched */
	if(ic->private.local.context == ((Xim)ic->core.im)->private.local.top) {
	    return(False);
	}
	/* Error (Sequence Unmatch occured) */
	/* initialize internal state for next key sequence */
	ic->private.local.context = ((Xim)ic->core.im)->private.local.top;
	return(True);
    }
}
