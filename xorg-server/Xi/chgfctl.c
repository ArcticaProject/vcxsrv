/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/********************************************************************
 *
 *  Change feedback control attributes for an extension device.
 *
 */

#define	 NEED_EVENTS	/* for inputstr.h    */
#define	 NEED_REPLIES
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "inputstr.h"	/* DeviceIntPtr      */
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>	/* control constants */

#include "exglobals.h"

#include "chgfctl.h"

#define DO_ALL    (-1)

/***********************************************************************
 *
 * This procedure changes the control attributes for an extension device,
 * for clients on machines with a different byte ordering than the server.
 *
 */

int
SProcXChangeFeedbackControl(ClientPtr client)
{
    char n;

    REQUEST(xChangeFeedbackControlReq);
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xChangeFeedbackControlReq);
    swapl(&stuff->mask, n);
    return (ProcXChangeFeedbackControl(client));
}

/******************************************************************************
 *
 * This procedure changes KbdFeedbackClass data.
 *
 */

static int
ChangeKbdFeedback(ClientPtr client, DeviceIntPtr dev, long unsigned int mask,
		  KbdFeedbackPtr k, xKbdFeedbackCtl * f)
{
    char n;
    KeybdCtrl kctrl;
    int t;
    int key = DO_ALL;

    if (client->swapped) {
	swaps(&f->length, n);
	swaps(&f->pitch, n);
	swaps(&f->duration, n);
	swapl(&f->led_mask, n);
	swapl(&f->led_values, n);
    }

    kctrl = k->ctrl;
    if (mask & DvKeyClickPercent) {
	t = f->click;
	if (t == -1)
	    t = defaultKeyboardControl.click;
	else if (t < 0 || t > 100) {
	    client->errorValue = t;
	    return BadValue;
	}
	kctrl.click = t;
    }

    if (mask & DvPercent) {
	t = f->percent;
	if (t == -1)
	    t = defaultKeyboardControl.bell;
	else if (t < 0 || t > 100) {
	    client->errorValue = t;
	    return BadValue;
	}
	kctrl.bell = t;
    }

    if (mask & DvPitch) {
	t = f->pitch;
	if (t == -1)
	    t = defaultKeyboardControl.bell_pitch;
	else if (t < 0) {
	    client->errorValue = t;
	    return BadValue;
	}
	kctrl.bell_pitch = t;
    }

    if (mask & DvDuration) {
	t = f->duration;
	if (t == -1)
	    t = defaultKeyboardControl.bell_duration;
	else if (t < 0) {
	    client->errorValue = t;
	    return BadValue;
	}
	kctrl.bell_duration = t;
    }

    if (mask & DvLed) {
	kctrl.leds &= ~(f->led_mask);
	kctrl.leds |= (f->led_mask & f->led_values);
    }

    if (mask & DvKey) {
	key = (KeyCode) f->key;
	if (key < 8 || key > 255) {
	    client->errorValue = key;
	    return BadValue;
	}
	if (!(mask & DvAutoRepeatMode))
	    return BadMatch;
    }

    if (mask & DvAutoRepeatMode) {
	int inx = (key >> 3);
	int kmask = (1 << (key & 7));

	t = (CARD8) f->auto_repeat_mode;
	if (t == AutoRepeatModeOff) {
	    if (key == DO_ALL)
		kctrl.autoRepeat = FALSE;
	    else
		kctrl.autoRepeats[inx] &= ~kmask;
	} else if (t == AutoRepeatModeOn) {
	    if (key == DO_ALL)
		kctrl.autoRepeat = TRUE;
	    else
		kctrl.autoRepeats[inx] |= kmask;
	} else if (t == AutoRepeatModeDefault) {
	    if (key == DO_ALL)
		kctrl.autoRepeat = defaultKeyboardControl.autoRepeat;
	    else
		kctrl.autoRepeats[inx] &= ~kmask;
	    kctrl.autoRepeats[inx] =
		(kctrl.autoRepeats[inx] & ~kmask) |
		(defaultKeyboardControl.autoRepeats[inx] & kmask);
	} else {
	    client->errorValue = t;
	    return BadValue;
	}
    }

    k->ctrl = kctrl;
    (*k->CtrlProc) (dev, &k->ctrl);
    return Success;
}

/******************************************************************************
 *
 * This procedure changes PtrFeedbackClass data.
 *
 */

static int
ChangePtrFeedback(ClientPtr client, DeviceIntPtr dev, long unsigned int mask,
		  PtrFeedbackPtr p, xPtrFeedbackCtl * f)
{
    char n;
    PtrCtrl pctrl;	/* might get BadValue part way through */

    if (client->swapped) {
	swaps(&f->length, n);
	swaps(&f->num, n);
	swaps(&f->denom, n);
	swaps(&f->thresh, n);
    }

    pctrl = p->ctrl;
    if (mask & DvAccelNum) {
	int accelNum;

	accelNum = f->num;
	if (accelNum == -1)
	    pctrl.num = defaultPointerControl.num;
	else if (accelNum < 0) {
	    client->errorValue = accelNum;
	    return BadValue;
	} else
	    pctrl.num = accelNum;
    }

    if (mask & DvAccelDenom) {
	int accelDenom;

	accelDenom = f->denom;
	if (accelDenom == -1)
	    pctrl.den = defaultPointerControl.den;
	else if (accelDenom <= 0) {
	    client->errorValue = accelDenom;
	    return BadValue;
	} else
	    pctrl.den = accelDenom;
    }

    if (mask & DvThreshold) {
	int threshold;

	threshold = f->thresh;
	if (threshold == -1)
	    pctrl.threshold = defaultPointerControl.threshold;
	else if (threshold < 0) {
	    client->errorValue = threshold;
	    return BadValue;
	} else
	    pctrl.threshold = threshold;
    }

    p->ctrl = pctrl;
    (*p->CtrlProc) (dev, &p->ctrl);
    return Success;
}

/******************************************************************************
 *
 * This procedure changes IntegerFeedbackClass data.
 *
 */

static int
ChangeIntegerFeedback(ClientPtr client, DeviceIntPtr dev,
		      long unsigned int mask, IntegerFeedbackPtr i,
		      xIntegerFeedbackCtl * f)
{
    char n;

    if (client->swapped) {
	swaps(&f->length, n);
	swapl(&f->int_to_display, n);
    }

    i->ctrl.integer_displayed = f->int_to_display;
    (*i->CtrlProc) (dev, &i->ctrl);
    return Success;
}

/******************************************************************************
 *
 * This procedure changes StringFeedbackClass data.
 *
 */

static int
ChangeStringFeedback(ClientPtr client, DeviceIntPtr dev,
		     long unsigned int mask, StringFeedbackPtr s,
		     xStringFeedbackCtl * f)
{
    char n;
    int i, j;
    KeySym *syms, *sup_syms;

    syms = (KeySym *) (f + 1);
    if (client->swapped) {
	swaps(&f->length, n);	/* swapped num_keysyms in calling proc */
	SwapLongs((CARD32 *) syms, f->num_keysyms);
    }

    if (f->num_keysyms > s->ctrl.max_symbols)
	return BadValue;

    sup_syms = s->ctrl.symbols_supported;
    for (i = 0; i < f->num_keysyms; i++) {
	for (j = 0; j < s->ctrl.num_symbols_supported; j++)
	    if (*(syms + i) == *(sup_syms + j))
		break;
	if (j == s->ctrl.num_symbols_supported)
	    return BadMatch;
    }

    s->ctrl.num_symbols_displayed = f->num_keysyms;
    for (i = 0; i < f->num_keysyms; i++)
	*(s->ctrl.symbols_displayed + i) = *(syms + i);
    (*s->CtrlProc) (dev, &s->ctrl);
    return Success;
}

/******************************************************************************
 *
 * This procedure changes BellFeedbackClass data.
 *
 */

static int
ChangeBellFeedback(ClientPtr client, DeviceIntPtr dev,
		   long unsigned int mask, BellFeedbackPtr b,
		   xBellFeedbackCtl * f)
{
    char n;
    int t;
    BellCtrl bctrl;	/* might get BadValue part way through */

    if (client->swapped) {
	swaps(&f->length, n);
	swaps(&f->pitch, n);
	swaps(&f->duration, n);
    }

    bctrl = b->ctrl;
    if (mask & DvPercent) {
	t = f->percent;
	if (t == -1)
	    t = defaultKeyboardControl.bell;
	else if (t < 0 || t > 100) {
	    client->errorValue = t;
	    return BadValue;
	}
	bctrl.percent = t;
    }

    if (mask & DvPitch) {
	t = f->pitch;
	if (t == -1)
	    t = defaultKeyboardControl.bell_pitch;
	else if (t < 0) {
	    client->errorValue = t;
	    return BadValue;
	}
	bctrl.pitch = t;
    }

    if (mask & DvDuration) {
	t = f->duration;
	if (t == -1)
	    t = defaultKeyboardControl.bell_duration;
	else if (t < 0) {
	    client->errorValue = t;
	    return BadValue;
	}
	bctrl.duration = t;
    }
    b->ctrl = bctrl;
    (*b->CtrlProc) (dev, &b->ctrl);
    return Success;
}

/******************************************************************************
 *
 * This procedure changes LedFeedbackClass data.
 *
 */

static int
ChangeLedFeedback(ClientPtr client, DeviceIntPtr dev, long unsigned int mask,
		  LedFeedbackPtr l, xLedFeedbackCtl * f)
{
    char n;
    LedCtrl lctrl;	/* might get BadValue part way through */

    if (client->swapped) {
	swaps(&f->length, n);
	swapl(&f->led_values, n);
	swapl(&f->led_mask, n);
    }

    f->led_mask &= l->ctrl.led_mask;	/* set only supported leds */
    f->led_values &= l->ctrl.led_mask;	/* set only supported leds */
    if (mask & DvLed) {
	lctrl.led_mask = f->led_mask;
	lctrl.led_values = f->led_values;
	(*l->CtrlProc) (dev, &lctrl);
	l->ctrl.led_values &= ~(f->led_mask);	/* zero changed leds */
	l->ctrl.led_values |= (f->led_mask & f->led_values);	/* OR in set leds */
    }

    return Success;
}

/***********************************************************************
 *
 * Change the control attributes.
 *
 */

int
ProcXChangeFeedbackControl(ClientPtr client)
{
    unsigned len;
    DeviceIntPtr dev;
    KbdFeedbackPtr k;
    PtrFeedbackPtr p;
    IntegerFeedbackPtr i;
    StringFeedbackPtr s;
    BellFeedbackPtr b;
    LedFeedbackPtr l;
    int rc;

    REQUEST(xChangeFeedbackControlReq);
    REQUEST_AT_LEAST_SIZE(xChangeFeedbackControlReq);

    len = stuff->length - (sizeof(xChangeFeedbackControlReq) >> 2);
    rc = dixLookupDevice(&dev, stuff->deviceid, client, DixManageAccess);
    if (rc != Success)
	return rc;

    switch (stuff->feedbackid) {
    case KbdFeedbackClass:
	if (len != (sizeof(xKbdFeedbackCtl) >> 2))
	    return BadLength;

	for (k = dev->kbdfeed; k; k = k->next)
	    if (k->ctrl.id == ((xKbdFeedbackCtl *) & stuff[1])->id)
		return ChangeKbdFeedback(client, dev, stuff->mask, k,
					 (xKbdFeedbackCtl *) & stuff[1]);
	break;
    case PtrFeedbackClass:
	if (len != (sizeof(xPtrFeedbackCtl) >> 2))
	    return BadLength;

	for (p = dev->ptrfeed; p; p = p->next)
	    if (p->ctrl.id == ((xPtrFeedbackCtl *) & stuff[1])->id)
		return ChangePtrFeedback(client, dev, stuff->mask, p,
					 (xPtrFeedbackCtl *) & stuff[1]);
	break;
    case StringFeedbackClass:
    {
	char n;
	xStringFeedbackCtl *f = ((xStringFeedbackCtl *) & stuff[1]);

	if (client->swapped) {
	    swaps(&f->num_keysyms, n);
	}
	if (len != ((sizeof(xStringFeedbackCtl) >> 2) + f->num_keysyms))
	    return BadLength;

	for (s = dev->stringfeed; s; s = s->next)
	    if (s->ctrl.id == ((xStringFeedbackCtl *) & stuff[1])->id)
		return ChangeStringFeedback(client, dev, stuff->mask, s,
					    (xStringFeedbackCtl *) & stuff[1]);
	break;
    }
    case IntegerFeedbackClass:
	if (len != (sizeof(xIntegerFeedbackCtl) >> 2))
	    return BadLength;

	for (i = dev->intfeed; i; i = i->next)
	    if (i->ctrl.id == ((xIntegerFeedbackCtl *) & stuff[1])->id)
		return ChangeIntegerFeedback(client, dev, stuff->mask, i,
					     (xIntegerFeedbackCtl *)&stuff[1]);
	break;
    case LedFeedbackClass:
	if (len != (sizeof(xLedFeedbackCtl) >> 2))
	    return BadLength;

	for (l = dev->leds; l; l = l->next)
	    if (l->ctrl.id == ((xLedFeedbackCtl *) & stuff[1])->id)
		return ChangeLedFeedback(client, dev, stuff->mask, l,
					 (xLedFeedbackCtl *) & stuff[1]);
	break;
    case BellFeedbackClass:
	if (len != (sizeof(xBellFeedbackCtl) >> 2))
	    return BadLength;

	for (b = dev->bell; b; b = b->next)
	    if (b->ctrl.id == ((xBellFeedbackCtl *) & stuff[1])->id)
		return ChangeBellFeedback(client, dev, stuff->mask, b,
					  (xBellFeedbackCtl *) & stuff[1]);
	break;
    default:
	break;
    }

    return BadMatch;
}

