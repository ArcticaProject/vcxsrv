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
 *  Routines to register and initialize extension input devices.
 *  This also contains ProcessOtherEvent, the routine called from DDX
 *  to route extension events.
 *
 */

#define	 NEED_EVENTS
#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include <X11/extensions/geproto.h>
#include "inputstr.h"
#include "windowstr.h"
#include "miscstruct.h"
#include "region.h"
#include "exevents.h"
#include "extnsionst.h"
#include "exglobals.h"
#include "dixevents.h"	/* DeliverFocusedEvent */
#include "dixgrabs.h"	/* CreateGrab() */
#include "scrnintstr.h"
#include "listdev.h" /* for CopySwapXXXClass */
#include "xace.h"

#ifdef XKB
#include <X11/extensions/XKBproto.h>
#include "xkbsrv.h"
extern Bool XkbCopyKeymap(XkbDescPtr src, XkbDescPtr dst, Bool sendNotifies);
#endif

#define WID(w) ((w) ? ((w)->drawable.id) : 0)
#define AllModifiersMask ( \
	ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask | \
	Mod3Mask | Mod4Mask | Mod5Mask )
#define AllButtonsMask ( \
	Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )
#define Motion_Filter(class) (DevicePointerMotionMask | \
			      (class)->state | (class)->motionMask)

Bool ShouldFreeInputMasks(WindowPtr /* pWin */ ,
				 Bool	/* ignoreSelectedEvents */
    );
static Bool MakeInputMasks(WindowPtr	/* pWin */
    );

/* Used to sture classes currently not in use by an MD */
extern DevPrivateKey UnusedClassesPrivateKey;


void
RegisterOtherDevice(DeviceIntPtr device)
{
    device->public.processInputProc = ProcessOtherEvent;
    device->public.realInputProc = ProcessOtherEvent;
}

Bool
IsPointerEvent(xEvent* xE)
{
    switch(xE->u.u.type)
    {
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
        case EnterNotify:
        case LeaveNotify:
            return TRUE;
        default:
            if (xE->u.u.type == DeviceButtonPress ||
                xE->u.u.type == DeviceButtonRelease ||
                xE->u.u.type == DeviceMotionNotify ||
                xE->u.u.type == ProximityIn ||
                xE->u.u.type == ProximityOut)
            {
                return TRUE;
            }
    }
    return FALSE;
}

/**
 * @return the device matching the deviceid of the device set in the event, or
 * NULL if the event is not an XInput event.
 */
DeviceIntPtr
XIGetDevice(xEvent* xE)
{
    DeviceIntPtr pDev = NULL;

    if (xE->u.u.type == DeviceButtonPress ||
        xE->u.u.type == DeviceButtonRelease ||
        xE->u.u.type == DeviceMotionNotify ||
        xE->u.u.type == ProximityIn ||
        xE->u.u.type == ProximityOut ||
        xE->u.u.type == DevicePropertyNotify)
    {
        int rc;
        int id;

        id = ((deviceKeyButtonPointer*)xE)->deviceid & ~MORE_EVENTS;

        rc = dixLookupDevice(&pDev, id, serverClient, DixUnknownAccess);
        if (rc != Success)
            ErrorF("[dix] XIGetDevice failed on XACE restrictions (%d)\n", rc);
    }
    return pDev;
}


/**
 * Copy the device->key into master->key and send a mapping notify to the
 * clients if appropriate.
 * master->key needs to be allocated by the caller.
 *
 * Device is the slave device. If it is attached to a master device, we may
 * need to send a mapping notify to the client because it causes the MD
 * to change state.
 *
 * Mapping notify needs to be sent in the following cases:
 *      - different slave device on same master
 *      - different master
 *
 * XXX: They way how the code is we also send a map notify if the slave device
 * stays the same, but the master changes. This isn't really necessary though.
 *
 * XXX: this gives you funny behaviour with the ClientPointer. When a
 * MappingNotify is sent to the client, the client usually responds with a
 * GetKeyboardMapping. This will retrieve the ClientPointer's keyboard
 * mapping, regardless of which keyboard sent the last mapping notify request.
 * So depending on the CP setting, your keyboard may change layout in each
 * app...
 *
 * This code is basically the old SwitchCoreKeyboard.
 */

void
CopyKeyClass(DeviceIntPtr device, DeviceIntPtr master)
{
    static DeviceIntPtr lastMapNotifyDevice = NULL;
    KeyClassPtr mk, dk; /* master, device */
    BOOL sendNotify = FALSE;
    int i;

    if (device == master)
        return;

    dk = device->key;
    mk = master->key;

    if (device != dixLookupPrivate(&master->devPrivates,
                                   CoreDevicePrivateKey)) {
        memcpy(mk->modifierMap, dk->modifierMap, MAP_LENGTH);

        if (dk->maxKeysPerModifier)
        {
            mk->modifierKeyMap = xrealloc(mk->modifierKeyMap,
                                          8 * dk->maxKeysPerModifier);
            if (!mk->modifierKeyMap)
                FatalError("[Xi] no memory for class shift.\n");
            memcpy(mk->modifierKeyMap, dk->modifierKeyMap,
                    (8 * dk->maxKeysPerModifier));
        } else
        {
            xfree(mk->modifierKeyMap);
            mk->modifierKeyMap = NULL;
        }

        mk->maxKeysPerModifier = dk->maxKeysPerModifier;
        mk->curKeySyms.minKeyCode = dk->curKeySyms.minKeyCode;
        mk->curKeySyms.maxKeyCode = dk->curKeySyms.maxKeyCode;
        SetKeySymsMap(&mk->curKeySyms, &dk->curKeySyms);

        /*
         * Copy state from the extended keyboard to core.  If you omit this,
         * holding Ctrl on keyboard one, and pressing Q on keyboard two, will
         * cause your app to quit.  This feels wrong to me, hence the below
         * code.
         *
         * XXX: If you synthesise core modifier events, the state will get
         *      clobbered here.  You'll have to work out something sensible
         *      to fix that.  Good luck.
         */

#define KEYBOARD_MASK (ShiftMask | LockMask | ControlMask | Mod1Mask | \
        Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)
        mk->state &= ~(KEYBOARD_MASK);
        mk->state |= (dk->state & KEYBOARD_MASK);
#undef KEYBOARD_MASK
        for (i = 0; i < 8; i++)
            mk->modifierKeyCount[i] = dk->modifierKeyCount[i];

#ifdef XKB
        if (!noXkbExtension && dk->xkbInfo && dk->xkbInfo->desc) {
            if (!mk->xkbInfo || !mk->xkbInfo->desc)
            {
                XkbInitDevice(master);
                XkbFinishDeviceInit(master);
            }
            if (!XkbCopyKeymap(dk->xkbInfo->desc, mk->xkbInfo->desc, True))
                FatalError("Couldn't pivot keymap from device to core!\n");
        }
#endif

        dixSetPrivate(&master->devPrivates, CoreDevicePrivateKey, device);
        sendNotify = TRUE;
    } else if (lastMapNotifyDevice != master)
        sendNotify = TRUE;

    if (sendNotify)
    {
        SendMappingNotify(master, MappingKeyboard,
                           mk->curKeySyms.minKeyCode,
                          (mk->curKeySyms.maxKeyCode -
                           mk->curKeySyms.minKeyCode),
                          serverClient);
        lastMapNotifyDevice = master;
    }
}

/**
 * Copies the feedback classes from device "from" into device "to". Classes
 * are duplicated (not just flipping the pointers). All feedback classes are
 * linked lists, the full list is duplicated.
 */
static void
DeepCopyFeedbackClasses(DeviceIntPtr from, DeviceIntPtr to)
{
    ClassesPtr classes;

    if (from->kbdfeed)
    {
        KbdFeedbackPtr *k, it;

        if (!to->kbdfeed)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->kbdfeed = classes->kbdfeed;
        }

        k = &to->kbdfeed;
        for(it = from->kbdfeed; it; it = it->next)
        {
            if (!(*k))
            {
                *k = xcalloc(1, sizeof(KbdFeedbackClassRec));
                if (!*k)
                {
                    ErrorF("[Xi] Cannot alloc memory for class copy.");
                    return;
                }
            }
            (*k)->BellProc = it->BellProc;
            (*k)->CtrlProc = it->CtrlProc;
            (*k)->ctrl     = it->ctrl;
#ifdef XKB
            if ((*k)->xkb_sli)
                XkbFreeSrvLedInfo((*k)->xkb_sli);
            (*k)->xkb_sli = XkbCopySrvLedInfo(from, it->xkb_sli, *k, NULL);
#endif

            k = &(*k)->next;
        }
    } else if (to->kbdfeed && !from->kbdfeed)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->kbdfeed = to->kbdfeed;
        to->kbdfeed      = NULL;
    }

    if (from->ptrfeed)
    {
        PtrFeedbackPtr *p, it;
        if (!to->ptrfeed)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->ptrfeed = classes->ptrfeed;
        }

        p = &to->ptrfeed;
        for (it = from->ptrfeed; it; it = it->next)
        {
            if (!(*p))
            {
                *p = xcalloc(1, sizeof(PtrFeedbackClassRec));
                if (!*p)
                {
                    ErrorF("[Xi] Cannot alloc memory for class copy.");
                    return;
                }
            }
            (*p)->CtrlProc = it->CtrlProc;
            (*p)->ctrl     = it->ctrl;

            p = &(*p)->next;
        }
    } else if (to->ptrfeed && !from->ptrfeed)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->ptrfeed = to->ptrfeed;
        to->ptrfeed      = NULL;
    }

    if (from->intfeed)
    {
        IntegerFeedbackPtr *i, it;

        if (!to->intfeed)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->intfeed = classes->intfeed;
        }

        i = &to->intfeed;
        for (it = from->intfeed; it; it = it->next)
        {
            if (!(*i))
            {
                *i = xcalloc(1, sizeof(IntegerFeedbackClassRec));
                if (!(*i))
                {
                    ErrorF("[Xi] Cannot alloc memory for class copy.");
                    return;
                }
            }
            (*i)->CtrlProc = it->CtrlProc;
            (*i)->ctrl     = it->ctrl;

            i = &(*i)->next;
        }
    } else if (to->intfeed && !from->intfeed)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->intfeed = to->intfeed;
        to->intfeed      = NULL;
    }

    if (from->stringfeed)
    {
        StringFeedbackPtr *s, it;

        if (!to->stringfeed)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->stringfeed = classes->stringfeed;
        }

        s = &to->stringfeed;
        for (it = from->stringfeed; it; it = it->next)
        {
            if (!(*s))
            {
                *s = xcalloc(1, sizeof(StringFeedbackClassRec));
                if (!(*s))
                {
                    ErrorF("[Xi] Cannot alloc memory for class copy.");
                    return;
                }
            }
            (*s)->CtrlProc = it->CtrlProc;
            (*s)->ctrl     = it->ctrl;

            s = &(*s)->next;
        }
    } else if (to->stringfeed && !from->stringfeed)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->stringfeed = to->stringfeed;
        to->stringfeed      = NULL;
    }

    if (from->bell)
    {
        BellFeedbackPtr *b, it;

        if (!to->bell)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->bell = classes->bell;
        }

        b = &to->bell;
        for (it = from->bell; it; it = it->next)
        {
            if (!(*b))
            {
                *b = xcalloc(1, sizeof(BellFeedbackClassRec));
                if (!(*b))
                {
                    ErrorF("[Xi] Cannot alloc memory for class copy.");
                    return;
                }
            }
            (*b)->BellProc = it->BellProc;
            (*b)->CtrlProc = it->CtrlProc;
            (*b)->ctrl     = it->ctrl;

            b = &(*b)->next;
        }
    } else if (to->bell && !from->bell)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->bell = to->bell;
        to->bell      = NULL;
    }

    if (from->leds)
    {
        LedFeedbackPtr *l, it;

        if (!to->leds)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->leds = classes->leds;
        }

        l = &to->leds;
        for (it = from->leds; it; it = it->next)
        {
            if (!(*l))
            {
                *l = xcalloc(1, sizeof(LedFeedbackClassRec));
                if (!(*l))
                {
                    ErrorF("[Xi] Cannot alloc memory for class copy.");
                    return;
                }
            }
            (*l)->CtrlProc = it->CtrlProc;
            (*l)->ctrl     = it->ctrl;
#ifdef XKB
            if ((*l)->xkb_sli)
                XkbFreeSrvLedInfo((*l)->xkb_sli);
            (*l)->xkb_sli = XkbCopySrvLedInfo(from, it->xkb_sli, NULL, *l);
#endif

            l = &(*l)->next;
        }
    } else if (to->leds && !from->leds)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->leds = to->leds;
        to->leds      = NULL;
    }
}

/**
 * Copies the CONTENT of the classes of device from into the classes in device
 * to. From and to are identical after finishing.
 *
 * If to does not have classes from currenly has, the classes are stored in
 * to's devPrivates system. Later, we recover it again from there if needed.
 * Saves a few memory allocations.
 */

void
DeepCopyDeviceClasses(DeviceIntPtr from, DeviceIntPtr to)
{
    ClassesPtr classes;

    /* XkbInitDevice (->XkbInitIndicatorMap->XkbFindSrvLedInfo) relies on the
     * kbdfeed to be set up properly, so let's do the feedback classes first.
     */
    DeepCopyFeedbackClasses(from, to);

    if (from->key)
    {
        KeyCode             *oldModKeyMap;
        KeySym              *oldMap;
#ifdef XKB
        struct _XkbSrvInfo  *oldXkbInfo;
#endif
        if (!to->key)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->key = classes->key;
            if (!to->key)
            {
                to->key = xcalloc(1, sizeof(KeyClassRec));
                if (!to->key)
                    FatalError("[Xi] no memory for class shift.\n");
            } else
                classes->key = NULL;
        }

        oldModKeyMap    = to->key->modifierKeyMap;
        oldMap          = to->key->curKeySyms.map;
#ifdef XKB
        oldXkbInfo      = to->key->xkbInfo;
#endif

        if (!oldMap) /* newly created key struct */
        {
            int bytes = (to->key->curKeySyms.maxKeyCode -
                         to->key->curKeySyms.minKeyCode + 1) *
                         to->key->curKeySyms.mapWidth;
            oldMap = (KeySym *)xcalloc(sizeof(KeySym), bytes);
            memcpy(oldMap, from->key->curKeySyms.map, bytes);
        }

        to->key->modifierKeyMap = oldModKeyMap;
        to->key->curKeySyms.map = oldMap;
#ifdef XKB
        to->key->xkbInfo        = oldXkbInfo;
#endif

        CopyKeyClass(from, to);
    } else if (to->key && !from->key)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->key = to->key;
        to->key      = NULL;
    }

    if (from->valuator)
    {
        ValuatorClassPtr v;
        if (!to->valuator)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->valuator = classes->valuator;
            if (to->valuator)
                classes->valuator = NULL;
        }

        to->valuator = xrealloc(to->valuator, sizeof(ValuatorClassRec) +
                from->valuator->numAxes * sizeof(AxisInfo) +
                from->valuator->numAxes * sizeof(unsigned int));
        v = to->valuator;
        if (!v)
            FatalError("[Xi] no memory for class shift.\n");

        v->numAxes = from->valuator->numAxes;
        v->axes = (AxisInfoPtr)&v[1];
        memcpy(v->axes, from->valuator->axes, v->numAxes * sizeof(AxisInfo));

        v->axisVal = (int*)(v->axes + from->valuator->numAxes);
    } else if (to->valuator && !from->valuator)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->valuator = to->valuator;
        to->valuator      = NULL;
    }

    if (from->button)
    {
        if (!to->button)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->button = classes->button;
            if (!to->button)
            {
                to->button = xcalloc(1, sizeof(ButtonClassRec));
                if (!to->button)
                    FatalError("[Xi] no memory for class shift.\n");
            } else
                classes->button = NULL;
        }

#ifdef XKB
        if (from->button->xkb_acts)
        {
            if (!to->button->xkb_acts)
            {
                to->button->xkb_acts = xcalloc(1, sizeof(XkbAction));
                if (!to->button->xkb_acts)
                    FatalError("[Xi] not enough memory for xkb_acts.\n");
            }
            memcpy(to->button->xkb_acts, from->button->xkb_acts,
                    sizeof(XkbAction));
        } else
            xfree(to->button->xkb_acts);
#endif
    } else if (to->button && !from->button)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->button = to->button;
        to->button      = NULL;
    }


    /* We can't just copy over the focus class. When an app sets the focus,
     * it'll do so on the master device. Copying the SDs focus means losing
     * the focus.
     * So we only copy the focus class if the device didn't have one,
     * otherwise we leave it as it is.
     */
    if (from->focus)
    {
        if (!to->focus)
        {
            WindowPtr *oldTrace;

            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->focus = classes->focus;
            if (!to->focus)
            {
                to->focus = xcalloc(1, sizeof(FocusClassRec));
                if (!to->focus)
                    FatalError("[Xi] no memory for class shift.\n");
            } else
                classes->focus = NULL;

            oldTrace = to->focus->trace;
            memcpy(to->focus, from->focus, sizeof(FocusClassRec));
            to->focus->trace = xrealloc(oldTrace,
                                  to->focus->traceSize * sizeof(WindowPtr));
            if (!to->focus->trace && to->focus->traceSize)
                FatalError("[Xi] no memory for trace.\n");
            memcpy(to->focus->trace, from->focus->trace,
                    from->focus->traceSize * sizeof(WindowPtr));
        }
    } else if (to->focus)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->focus = to->focus;
        to->focus      = NULL;
    }

    if (from->proximity)
    {
        if (!to->proximity)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->proximity = classes->proximity;
            if (!to->proximity)
            {
                to->proximity = xcalloc(1, sizeof(ProximityClassRec));
                if (!to->proximity)
                    FatalError("[Xi] no memory for class shift.\n");
            } else
                classes->proximity = NULL;
        }
        memcpy(to->proximity, from->proximity, sizeof(ProximityClassRec));
    } else if (to->proximity)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->proximity = to->proximity;
        to->proximity      = NULL;
    }

    if (from->absolute)
    {
        if (!to->absolute)
        {
            classes = dixLookupPrivate(&to->devPrivates,
                                       UnusedClassesPrivateKey);
            to->absolute = classes->absolute;
            if (!to->absolute)
            {
                to->absolute = xcalloc(1, sizeof(AbsoluteClassRec));
                if (!to->absolute)
                    FatalError("[Xi] no memory for class shift.\n");
            } else
                classes->absolute = NULL;
        }
        memcpy(to->absolute, from->absolute, sizeof(AbsoluteClassRec));
    } else if (to->absolute)
    {
        ClassesPtr classes;
        classes = dixLookupPrivate(&to->devPrivates, UnusedClassesPrivateKey);
        classes->absolute = to->absolute;
        to->absolute      = NULL;
    }
}


/**
 * Update the device state according to the data in the event.
 *
 * return values are
 *   DEFAULT ... process as normal
 *   DONT_PROCESS ... return immediately from caller
 *   IS_REPEAT .. event is a repeat event.
 */
#define DEFAULT 0
#define DONT_PROCESS 1
#define IS_REPEAT 2
int
UpdateDeviceState(DeviceIntPtr device, xEvent* xE, int count)
{
    int i;
    int key = 0,
        bit = 0;

    KeyClassPtr k       = NULL;
    ButtonClassPtr b    = NULL;
    ValuatorClassPtr v  = NULL;
    deviceValuator *xV  = (deviceValuator *) xE;
    BYTE *kptr          = NULL;
    CARD16 modifiers    = 0,
           mask         = 0;

    /* currently no other generic event modifies the device */
    if (xE->u.u.type == GenericEvent)
        return DEFAULT;

    k = device->key;
    v = device->valuator;
    b = device->button;


    if (xE->u.u.type != DeviceValuator)
    {
        key = xE->u.u.detail;
        bit = 1 << (key & 7);
    }

    /* Update device axis */
    /* Don't update valuators for the VCP, it never sends XI events anyway */
    for (i = 1; !device->isMaster && i < count; i++) {
	if ((++xV)->type == DeviceValuator) {
	    int *axisvals;
            int first = xV->first_valuator;
            BOOL change = FALSE;


	    if (xV->num_valuators &&
                (!v || (xV->num_valuators &&
                      (first + xV->num_valuators > v->numAxes))))
		FatalError("Bad valuators reported for device %s\n",
			   device->name);
	    if (v && v->axisVal) {
                /* v->axisVal is always in absolute coordinates. Only the
                 * delivery mode changes.
                 * If device is mode Absolute
                 *     dev = event
                 * If device is mode Relative
                 *      swap = (event - device)
                 *      dev = event
                 *      event = delta
                 */
                int delta;
                axisvals = v->axisVal;
                if (v->mode == Relative) /* device reports relative */
                    change = TRUE;

                switch (xV->num_valuators) {
                    case 6:
                        if (change) delta = xV->valuator5 - *(axisvals + first + 5);
                        *(axisvals + first + 5) = xV->valuator5;
                        if (change) xV->valuator5 = delta;
                    case 5:
                        if (change) delta = xV->valuator4 - *(axisvals + first + 4);
                        *(axisvals + first + 4) = xV->valuator4;
                        if (change) xV->valuator4 = delta;
                    case 4:
                        if (change) delta = xV->valuator3 - *(axisvals + first + 3);
                        *(axisvals + first + 3) = xV->valuator3;
                        if (change) xV->valuator3 = delta;
                    case 3:
                        if (change) delta = xV->valuator2 - *(axisvals + first + 2);
                        *(axisvals + first + 2) = xV->valuator2;
                        if (change) xV->valuator2 = delta;
                    case 2:
                        if (change) delta = xV->valuator1 - *(axisvals + first + 1);
                        *(axisvals + first + 1) = xV->valuator1;
                        if (change) xV->valuator1 = delta;
                    case 1:
                        if (change) delta = xV->valuator0 - *(axisvals + first);
                        *(axisvals + first) = xV->valuator0;
                        if (change) xV->valuator0 = delta;
                    case 0:
                    default:
                        break;
                }
	    }
	}
    }

    if (xE->u.u.type == DeviceKeyPress) {
        if (!k)
            return DONT_PROCESS;

	modifiers = k->modifierMap[key];
	kptr = &k->down[key >> 3];
	if (*kptr & bit) {	/* allow ddx to generate multiple downs */
	    return IS_REPEAT;
	}
	if (device->valuator)
	    device->valuator->motionHintWindow = NullWindow;
	*kptr |= bit;
	k->prev_state = k->state;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1) {
	    if (mask & modifiers) {
		/* This key affects modifier "i" */
		k->modifierKeyCount[i]++;
		k->state |= mask;
		modifiers &= ~mask;
	    }
	}
    } else if (xE->u.u.type == DeviceKeyRelease) {
        if (!k)
            return DONT_PROCESS;

	kptr = &k->down[key >> 3];
	if (!(*kptr & bit))	/* guard against duplicates */
	    return DONT_PROCESS;
	modifiers = k->modifierMap[key];
	if (device->valuator)
	    device->valuator->motionHintWindow = NullWindow;
	*kptr &= ~bit;
	k->prev_state = k->state;
	for (i = 0, mask = 1; modifiers; i++, mask <<= 1) {
	    if (mask & modifiers) {
		/* This key affects modifier "i" */
		if (--k->modifierKeyCount[i] <= 0) {
		    k->modifierKeyCount[i] = 0;
		    k->state &= ~mask;
		}
		modifiers &= ~mask;
	    }
	}
    } else if (xE->u.u.type == DeviceButtonPress) {
        if (!b)
            return DONT_PROCESS;

        kptr = &b->down[key >> 3];
        if ((*kptr & bit) != 0)
            return DONT_PROCESS;
        *kptr |= bit;
	if (device->valuator)
	    device->valuator->motionHintWindow = NullWindow;
        if (!b->map[key])
            return DONT_PROCESS;
        b->buttonsDown++;
	b->motionMask = DeviceButtonMotionMask;
        if (b->map[key] <= 5)
	    b->state |= (Button1Mask >> 1) << b->map[key];
	SetMaskForEvent(device->id, Motion_Filter(b), DeviceMotionNotify);
    } else if (xE->u.u.type == DeviceButtonRelease) {
        if (!b)
            return DONT_PROCESS;

        kptr = &b->down[key>>3];
        if (!(*kptr & bit))
            return DONT_PROCESS;
        if (device->isMaster) {
            DeviceIntPtr sd;

            /*
             * Leave the button down if any slave has the
             * button still down. Note that this depends on the
             * event being delivered through the slave first
             */
            for (sd = inputInfo.devices; sd; sd = sd->next) {
                if (sd->isMaster || sd->u.master != device)
                    continue;
                if ((sd->button->down[key>>3] & bit) != 0)
                    return DONT_PROCESS;
            }
        }
        *kptr &= ~bit;
	if (device->valuator)
	    device->valuator->motionHintWindow = NullWindow;
        if (!b->map[key])
            return DONT_PROCESS;
        if (b->buttonsDown >= 1 && !--b->buttonsDown)
	    b->motionMask = 0;
	if (b->map[key] <= 5)
	    b->state &= ~((Button1Mask >> 1) << b->map[key]);
	SetMaskForEvent(device->id, Motion_Filter(b), DeviceMotionNotify);
    } else if (xE->u.u.type == ProximityIn)
	device->valuator->mode &= ~OutOfProximity;
    else if (xE->u.u.type == ProximityOut)
	device->valuator->mode |= OutOfProximity;

    return DEFAULT;
}

/**
 * Main device event processing function.
 * Called from when processing the events from the event queue.
 *
 */
void
ProcessOtherEvent(xEventPtr xE, DeviceIntPtr device, int count)
{
    int i;
    CARD16 modifiers;
    GrabPtr grab = device->deviceGrab.grab;
    Bool deactivateDeviceGrab = FALSE;
    int key = 0, rootX, rootY;
    ButtonClassPtr b;
    KeyClassPtr k;
    ValuatorClassPtr v;
    deviceValuator *xV  = (deviceValuator *) xE;
    int ret = 0;
    int state;
    DeviceIntPtr mouse = NULL, kbd = NULL;

    if (IsPointerDevice(device))
    {
        kbd = GetPairedDevice(device);
        mouse = device;
        if (!kbd->key) /* can happen with floating SDs */
            kbd = NULL;
    } else
    {
        mouse = GetPairedDevice(device);
        kbd = device;
        if (!mouse->valuator || !mouse->button) /* may be float. SDs */
            mouse = NULL;
    }

    /* State needs to be assembled BEFORE the device is updated. */
    state = (kbd) ? kbd->key->state : 0;
    state |= (mouse) ? (mouse->button->state) : 0;

    ret = UpdateDeviceState(device, xE, count);
    if (ret == DONT_PROCESS)
        return;

    v = device->valuator;
    b = device->button;
    k = device->key;

    if (device->isMaster)
        CheckMotion(xE, device);

    if (xE->u.u.type != DeviceValuator && xE->u.u.type != GenericEvent) {
	GetSpritePosition(device, &rootX, &rootY);
	xE->u.keyButtonPointer.rootX = rootX;
	xE->u.keyButtonPointer.rootY = rootY;
	NoticeEventTime(xE);

        xE->u.keyButtonPointer.state = state;

        key = xE->u.u.detail;
    }
    if (DeviceEventCallback) {
	DeviceEventInfoRec eventinfo;

	eventinfo.events = (xEventPtr) xE;
	eventinfo.count = count;
	CallCallbacks(&DeviceEventCallback, (pointer) & eventinfo);
    }

    /* Valuator event handling */
    /* Don't care about valuators for the VCP, it never sends XI events */

    for (i = 1; !device->isMaster && i < count; i++) {
	if ((++xV)->type == DeviceValuator) {
	    int first = xV->first_valuator;
	    if (xV->num_valuators
		&& (!v
		    || (xV->num_valuators
			&& (first + xV->num_valuators > v->numAxes))))
		FatalError("Bad valuators reported for device %s\n",
			   device->name);
	    xV->device_state = 0;
	    if (k)
		xV->device_state |= k->state;
	    if (b)
		xV->device_state |= b->state;
	}
    }

    if (xE->u.u.type == DeviceKeyPress) {
        if (ret == IS_REPEAT) {	/* allow ddx to generate multiple downs */
            modifiers = k->modifierMap[key];
	    if (!modifiers) {
		xE->u.u.type = DeviceKeyRelease;
		ProcessOtherEvent(xE, device, count);
		xE->u.u.type = DeviceKeyPress;
		/* release can have side effects, don't fall through */
		ProcessOtherEvent(xE, device, count);
	    }
	    return;
	}
	if (!grab && CheckDeviceGrabs(device, xE, 0, count)) {
	    device->deviceGrab.activatingKey = key;
	    return;
	}
    } else if (xE->u.u.type == DeviceKeyRelease) {
	if (device->deviceGrab.fromPassiveGrab &&
            (key == device->deviceGrab.activatingKey))
	    deactivateDeviceGrab = TRUE;
    } else if (xE->u.u.type == DeviceButtonPress) {
	xE->u.u.detail = b->map[key];
	if (xE->u.u.detail == 0) {
	    xE->u.u.detail = key;
	    return;
	}
        if (!grab && CheckDeviceGrabs(device, xE, 0, count))
        {
            /* if a passive grab was activated, the event has been sent
             * already */
            return;
        }

    } else if (xE->u.u.type == DeviceButtonRelease) {
	xE->u.u.detail = b->map[key];
	if (xE->u.u.detail == 0) {
	    xE->u.u.detail = key;
	    return;
	}
        if (!b->buttonsDown && device->deviceGrab.fromPassiveGrab)
            deactivateDeviceGrab = TRUE;
    }

    if (grab)
        DeliverGrabbedEvent(xE, device, deactivateDeviceGrab, count);
    else if (device->focus && !IsPointerEvent(xE))
	DeliverFocusedEvent(device, xE, GetSpriteWindow(device), count);
    else
	DeliverDeviceEvents(GetSpriteWindow(device), xE, NullGrab, NullWindow,
			    device, count);

    if (deactivateDeviceGrab == TRUE)
	(*device->deviceGrab.DeactivateGrab) (device);
    xE->u.u.detail = key;
}

_X_EXPORT int
InitProximityClassDeviceStruct(DeviceIntPtr dev)
{
    ProximityClassPtr proxc;

    proxc = (ProximityClassPtr) xalloc(sizeof(ProximityClassRec));
    if (!proxc)
	return FALSE;
    dev->proximity = proxc;
    return TRUE;
}

/**
 * Initialise the device's valuators. The memory must already be allocated,
 * this function merely inits the matching axis (specified through axnum) to
 * sane values.
 *
 * It is a condition that (minval < maxval).
 *
 * @see InitValuatorClassDeviceStruct
 */
_X_EXPORT void
InitValuatorAxisStruct(DeviceIntPtr dev, int axnum, int minval, int maxval,
		       int resolution, int min_res, int max_res)
{
    AxisInfoPtr ax;

    if (!dev || !dev->valuator || minval > maxval)
        return;

    ax = dev->valuator->axes + axnum;

    ax->min_value = minval;
    ax->max_value = maxval;
    ax->resolution = resolution;
    ax->min_resolution = min_res;
    ax->max_resolution = max_res;
}

static void
FixDeviceStateNotify(DeviceIntPtr dev, deviceStateNotify * ev, KeyClassPtr k,
		     ButtonClassPtr b, ValuatorClassPtr v, int first)
{
    ev->type = DeviceStateNotify;
    ev->deviceid = dev->id;
    ev->time = currentTime.milliseconds;
    ev->classes_reported = 0;
    ev->num_keys = 0;
    ev->num_buttons = 0;
    ev->num_valuators = 0;

    if (b) {
	ev->classes_reported |= (1 << ButtonClass);
	ev->num_buttons = b->numButtons;
	memcpy((char*)ev->buttons, (char*)b->down, 4);
    } else if (k) {
	ev->classes_reported |= (1 << KeyClass);
	ev->num_keys = k->curKeySyms.maxKeyCode - k->curKeySyms.minKeyCode;
	memmove((char *)&ev->keys[0], (char *)k->down, 4);
    }
    if (v) {
	int nval = v->numAxes - first;

	ev->classes_reported |= (1 << ValuatorClass);
	ev->classes_reported |= (dev->valuator->mode << ModeBitsShift);
	ev->num_valuators = nval < 3 ? nval : 3;
	switch (ev->num_valuators) {
	case 3:
	    ev->valuator2 = v->axisVal[first + 2];
	case 2:
	    ev->valuator1 = v->axisVal[first + 1];
	case 1:
	    ev->valuator0 = v->axisVal[first];
	    break;
	}
    }
}

static void
FixDeviceValuator(DeviceIntPtr dev, deviceValuator * ev, ValuatorClassPtr v,
		  int first)
{
    int nval = v->numAxes - first;

    ev->type = DeviceValuator;
    ev->deviceid = dev->id;
    ev->num_valuators = nval < 3 ? nval : 3;
    ev->first_valuator = first;
    switch (ev->num_valuators) {
    case 3:
	ev->valuator2 = v->axisVal[first + 2];
    case 2:
	ev->valuator1 = v->axisVal[first + 1];
    case 1:
	ev->valuator0 = v->axisVal[first];
	break;
    }
    first += ev->num_valuators;
}

void
DeviceFocusEvent(DeviceIntPtr dev, int type, int mode, int detail,
		 WindowPtr pWin)
{
    deviceFocus event;

    if (type == FocusIn)
	type = DeviceFocusIn;
    else
	type = DeviceFocusOut;

    event.deviceid = dev->id;
    event.mode = mode;
    event.type = type;
    event.detail = detail;
    event.window = pWin->drawable.id;
    event.time = currentTime.milliseconds;

    (void)DeliverEventsToWindow(dev, pWin, (xEvent *) & event, 1,
				DeviceFocusChangeMask, NullGrab, dev->id);

    if ((type == DeviceFocusIn) &&
	(wOtherInputMasks(pWin)) &&
	(wOtherInputMasks(pWin)->inputEvents[dev->id] & DeviceStateNotifyMask))
    {
	int evcount = 1;
	deviceStateNotify *ev, *sev;
	deviceKeyStateNotify *kev;
	deviceButtonStateNotify *bev;

	KeyClassPtr k;
	ButtonClassPtr b;
	ValuatorClassPtr v;
	int nval = 0, nkeys = 0, nbuttons = 0, first = 0;

	if ((b = dev->button) != NULL) {
	    nbuttons = b->numButtons;
	    if (nbuttons > 32)
		evcount++;
	}
	if ((k = dev->key) != NULL) {
	    nkeys = k->curKeySyms.maxKeyCode - k->curKeySyms.minKeyCode;
	    if (nkeys > 32)
		evcount++;
	    if (nbuttons > 0) {
		evcount++;
	    }
	}
	if ((v = dev->valuator) != NULL) {
	    nval = v->numAxes;

	    if (nval > 3)
		evcount++;
	    if (nval > 6) {
		if (!(k && b))
		    evcount++;
		if (nval > 9)
		    evcount += ((nval - 7) / 3);
	    }
	}

	sev = ev = (deviceStateNotify *) xalloc(evcount * sizeof(xEvent));
	FixDeviceStateNotify(dev, ev, NULL, NULL, NULL, first);

	if (b != NULL) {
	    FixDeviceStateNotify(dev, ev++, NULL, b, v, first);
	    first += 3;
	    nval -= 3;
	    if (nbuttons > 32) {
		(ev - 1)->deviceid |= MORE_EVENTS;
		bev = (deviceButtonStateNotify *) ev++;
		bev->type = DeviceButtonStateNotify;
		bev->deviceid = dev->id;
		memcpy((char*)&bev->buttons[4], (char*)&b->down[4], DOWN_LENGTH - 4);
	    }
	    if (nval > 0) {
		(ev - 1)->deviceid |= MORE_EVENTS;
		FixDeviceValuator(dev, (deviceValuator *) ev++, v, first);
		first += 3;
		nval -= 3;
	    }
	}

	if (k != NULL) {
	    FixDeviceStateNotify(dev, ev++, k, NULL, v, first);
	    first += 3;
	    nval -= 3;
	    if (nkeys > 32) {
		(ev - 1)->deviceid |= MORE_EVENTS;
		kev = (deviceKeyStateNotify *) ev++;
		kev->type = DeviceKeyStateNotify;
		kev->deviceid = dev->id;
		memmove((char *)&kev->keys[0], (char *)&k->down[4], 28);
	    }
	    if (nval > 0) {
		(ev - 1)->deviceid |= MORE_EVENTS;
		FixDeviceValuator(dev, (deviceValuator *) ev++, v, first);
		first += 3;
		nval -= 3;
	    }
	}

	while (nval > 0) {
	    FixDeviceStateNotify(dev, ev++, NULL, NULL, v, first);
	    first += 3;
	    nval -= 3;
	    if (nval > 0) {
		(ev - 1)->deviceid |= MORE_EVENTS;
		FixDeviceValuator(dev, (deviceValuator *) ev++, v, first);
		first += 3;
		nval -= 3;
	    }
	}

	(void)DeliverEventsToWindow(dev, pWin, (xEvent *) sev, evcount,
				    DeviceStateNotifyMask, NullGrab, dev->id);
	xfree(sev);
    }
}

int
GrabButton(ClientPtr client, DeviceIntPtr dev, BYTE this_device_mode,
	   BYTE other_devices_mode, CARD16 modifiers,
	   DeviceIntPtr modifier_device, CARD8 button, Window grabWindow,
	   BOOL ownerEvents, Cursor rcursor, Window rconfineTo, Mask eventMask)
{
    WindowPtr pWin, confineTo;
    CursorPtr cursor;
    GrabPtr grab;
    Mask access_mode = DixGrabAccess;
    int rc;

    if ((this_device_mode != GrabModeSync) &&
	(this_device_mode != GrabModeAsync)) {
	client->errorValue = this_device_mode;
	return BadValue;
    }
    if ((other_devices_mode != GrabModeSync) &&
	(other_devices_mode != GrabModeAsync)) {
	client->errorValue = other_devices_mode;
	return BadValue;
    }
    if ((modifiers != AnyModifier) && (modifiers & ~AllModifiersMask)) {
	client->errorValue = modifiers;
	return BadValue;
    }
    if ((ownerEvents != xFalse) && (ownerEvents != xTrue)) {
	client->errorValue = ownerEvents;
	return BadValue;
    }
    rc = dixLookupWindow(&pWin, grabWindow, client, DixSetAttrAccess);
    if (rc != Success)
	return rc;
    if (rconfineTo == None)
	confineTo = NullWindow;
    else {
	rc = dixLookupWindow(&confineTo, rconfineTo, client, DixSetAttrAccess);
	if (rc != Success)
	    return rc;
    }
    if (rcursor == None)
	cursor = NullCursor;
    else {
	rc = dixLookupResourceByType((pointer *)&cursor, rcursor, RT_CURSOR,
			       client, DixUseAccess);
	if (rc != Success)
	{
	    client->errorValue = rcursor;
	    return (rc == BadValue) ? BadCursor : rc;
	}
	access_mode |= DixForceAccess;
    }
    if (this_device_mode == GrabModeSync || other_devices_mode == GrabModeSync)
	access_mode |= DixFreezeAccess;
    rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, access_mode);
    if (rc != Success)
	return rc;

    grab = CreateGrab(client->index, dev, pWin, eventMask,
		      (Bool) ownerEvents, (Bool) this_device_mode,
		      (Bool) other_devices_mode, modifier_device, modifiers,
		      DeviceButtonPress, button, confineTo, cursor);
    if (!grab)
	return BadAlloc;
    return AddPassiveGrabToList(client, grab);
}

int
GrabKey(ClientPtr client, DeviceIntPtr dev, BYTE this_device_mode,
	BYTE other_devices_mode, CARD16 modifiers,
	DeviceIntPtr modifier_device, CARD8 key, Window grabWindow,
	BOOL ownerEvents, Mask mask)
{
    WindowPtr pWin;
    GrabPtr grab;
    KeyClassPtr k = dev->key;
    Mask access_mode = DixGrabAccess;
    int rc;

    if (k == NULL)
	return BadMatch;
    if ((other_devices_mode != GrabModeSync) &&
	(other_devices_mode != GrabModeAsync)) {
	client->errorValue = other_devices_mode;
	return BadValue;
    }
    if ((this_device_mode != GrabModeSync) &&
	(this_device_mode != GrabModeAsync)) {
	client->errorValue = this_device_mode;
	return BadValue;
    }
    if (((key > k->curKeySyms.maxKeyCode) || (key < k->curKeySyms.minKeyCode))
	&& (key != AnyKey)) {
	client->errorValue = key;
	return BadValue;
    }
    if ((modifiers != AnyModifier) && (modifiers & ~AllModifiersMask)) {
	client->errorValue = modifiers;
	return BadValue;
    }
    if ((ownerEvents != xTrue) && (ownerEvents != xFalse)) {
	client->errorValue = ownerEvents;
	return BadValue;
    }
    rc = dixLookupWindow(&pWin, grabWindow, client, DixSetAttrAccess);
    if (rc != Success)
	return rc;
    if (this_device_mode == GrabModeSync || other_devices_mode == GrabModeSync)
	access_mode |= DixFreezeAccess;
    rc = XaceHook(XACE_DEVICE_ACCESS, client, dev, access_mode);
    if (rc != Success)
	return rc;

    grab = CreateGrab(client->index, dev, pWin,
		      mask, ownerEvents, this_device_mode, other_devices_mode,
		      modifier_device, modifiers, DeviceKeyPress, key,
		      NullWindow, NullCursor);
    if (!grab)
	return BadAlloc;
    return AddPassiveGrabToList(client, grab);
}

int
SelectForWindow(DeviceIntPtr dev, WindowPtr pWin, ClientPtr client,
		Mask mask, Mask exclusivemasks, Mask validmasks)
{
    int mskidx = dev->id;
    int i, ret;
    Mask check;
    InputClientsPtr others;

    if (mask & ~validmasks) {
	client->errorValue = mask;
	return BadValue;
    }
    check = (mask & exclusivemasks);
    if (wOtherInputMasks(pWin)) {
	if (check & wOtherInputMasks(pWin)->inputEvents[mskidx]) {	/* It is illegal for two different
									 * clients to select on any of the
									 * events for maskcheck. However,
									 * it is OK, for some client to
									 * continue selecting on one of those
									 * events.  */
	    for (others = wOtherInputMasks(pWin)->inputClients; others;
		 others = others->next) {
		if (!SameClient(others, client) && (check &
						    others->mask[mskidx]))
		    return BadAccess;
	    }
	}
	for (others = wOtherInputMasks(pWin)->inputClients; others;
	     others = others->next) {
	    if (SameClient(others, client)) {
		check = others->mask[mskidx];
		others->mask[mskidx] = mask;
		if (mask == 0) {
		    for (i = 0; i < EMASKSIZE; i++)
			if (i != mskidx && others->mask[i] != 0)
			    break;
		    if (i == EMASKSIZE) {
			RecalculateDeviceDeliverableEvents(pWin);
			if (ShouldFreeInputMasks(pWin, FALSE))
			    FreeResource(others->resource, RT_NONE);
			return Success;
		    }
		}
		goto maskSet;
	    }
	}
    }
    check = 0;
    if ((ret = AddExtensionClient(pWin, client, mask, mskidx)) != Success)
	return ret;
  maskSet:
    if (dev->valuator)
	if ((dev->valuator->motionHintWindow == pWin) &&
	    (mask & DevicePointerMotionHintMask) &&
	    !(check & DevicePointerMotionHintMask) && !dev->deviceGrab.grab)
	    dev->valuator->motionHintWindow = NullWindow;
    RecalculateDeviceDeliverableEvents(pWin);
    return Success;
}

int
AddExtensionClient(WindowPtr pWin, ClientPtr client, Mask mask, int mskidx)
{
    InputClientsPtr others;

    if (!pWin->optional && !MakeWindowOptional(pWin))
	return BadAlloc;
    others = xcalloc(1, sizeof(InputClients));
    if (!others)
	return BadAlloc;
    if (!pWin->optional->inputMasks && !MakeInputMasks(pWin))
	return BadAlloc;
    others->mask[mskidx] = mask;
    others->resource = FakeClientID(client->index);
    others->next = pWin->optional->inputMasks->inputClients;
    pWin->optional->inputMasks->inputClients = others;
    if (!AddResource(others->resource, RT_INPUTCLIENT, (pointer) pWin))
	return BadAlloc;
    return Success;
}

static Bool
MakeInputMasks(WindowPtr pWin)
{
    struct _OtherInputMasks *imasks;

    imasks = xcalloc(1, sizeof(struct _OtherInputMasks));
    if (!imasks)
	return FALSE;
    pWin->optional->inputMasks = imasks;
    return TRUE;
}

void
RecalculateDeviceDeliverableEvents(WindowPtr pWin)
{
    InputClientsPtr others;
    struct _OtherInputMasks *inputMasks;	/* default: NULL */
    WindowPtr pChild, tmp;
    int i;

    pChild = pWin;
    while (1) {
	if ((inputMasks = wOtherInputMasks(pChild)) != 0) {
	    for (others = inputMasks->inputClients; others;
		 others = others->next) {
		for (i = 0; i < EMASKSIZE; i++)
		    inputMasks->inputEvents[i] |= others->mask[i];
	    }
	    for (i = 0; i < EMASKSIZE; i++)
		inputMasks->deliverableEvents[i] = inputMasks->inputEvents[i];
	    for (tmp = pChild->parent; tmp; tmp = tmp->parent)
		if (wOtherInputMasks(tmp))
		    for (i = 0; i < EMASKSIZE; i++)
			inputMasks->deliverableEvents[i] |=
			    (wOtherInputMasks(tmp)->deliverableEvents[i]
			     & ~inputMasks->
			     dontPropagateMask[i] & PropagateMask[i]);
	}
	if (pChild->firstChild) {
	    pChild = pChild->firstChild;
	    continue;
	}
	while (!pChild->nextSib && (pChild != pWin))
	    pChild = pChild->parent;
	if (pChild == pWin)
	    break;
	pChild = pChild->nextSib;
    }
}

int
InputClientGone(WindowPtr pWin, XID id)
{
    InputClientsPtr other, prev;

    if (!wOtherInputMasks(pWin))
	return (Success);
    prev = 0;
    for (other = wOtherInputMasks(pWin)->inputClients; other;
	 other = other->next) {
	if (other->resource == id) {
	    if (prev) {
		prev->next = other->next;
		xfree(other);
	    } else if (!(other->next)) {
		if (ShouldFreeInputMasks(pWin, TRUE)) {
		    wOtherInputMasks(pWin)->inputClients = other->next;
		    xfree(wOtherInputMasks(pWin));
		    pWin->optional->inputMasks = (OtherInputMasks *) NULL;
		    CheckWindowOptionalNeed(pWin);
		    xfree(other);
		} else {
		    other->resource = FakeClientID(0);
		    if (!AddResource(other->resource, RT_INPUTCLIENT,
				     (pointer) pWin))
			return BadAlloc;
		}
	    } else {
		wOtherInputMasks(pWin)->inputClients = other->next;
		xfree(other);
	    }
	    RecalculateDeviceDeliverableEvents(pWin);
	    return (Success);
	}
	prev = other;
    }
    FatalError("client not on device event list");
}

int
SendEvent(ClientPtr client, DeviceIntPtr d, Window dest, Bool propagate,
	  xEvent * ev, Mask mask, int count)
{
    WindowPtr pWin;
    WindowPtr effectiveFocus = NullWindow;	/* only set if dest==InputFocus */
    WindowPtr spriteWin = GetSpriteWindow(d);

    if (dest == PointerWindow)
	pWin = spriteWin;
    else if (dest == InputFocus) {
	WindowPtr inputFocus;

	if (!d->focus)
	    inputFocus = spriteWin;
	else
	    inputFocus = d->focus->win;

	if (inputFocus == FollowKeyboardWin)
	    inputFocus = inputInfo.keyboard->focus->win;

	if (inputFocus == NoneWin)
	    return Success;

	/* If the input focus is PointerRootWin, send the event to where
	 * the pointer is if possible, then perhaps propogate up to root. */
	if (inputFocus == PointerRootWin)
	    inputFocus = GetCurrentRootWindow(d);

	if (IsParent(inputFocus, spriteWin)) {
	    effectiveFocus = inputFocus;
	    pWin = spriteWin;
	} else
	    effectiveFocus = pWin = inputFocus;
    } else
	dixLookupWindow(&pWin, dest, client, DixSendAccess);
    if (!pWin)
	return BadWindow;
    if ((propagate != xFalse) && (propagate != xTrue)) {
	client->errorValue = propagate;
	return BadValue;
    }
    ev->u.u.type |= 0x80;
    if (propagate) {
	for (; pWin; pWin = pWin->parent) {
	    if (DeliverEventsToWindow(d, pWin, ev, count, mask, NullGrab, d->id))
		return Success;
	    if (pWin == effectiveFocus)
		return Success;
	    if (wOtherInputMasks(pWin))
		mask &= ~wOtherInputMasks(pWin)->dontPropagateMask[d->id];
	    if (!mask)
		break;
	}
    } else if (!XaceHook(XACE_SEND_ACCESS, client, NULL, pWin, ev, count))
	(void)(DeliverEventsToWindow(d, pWin, ev, count, mask, NullGrab, d->id));
    return Success;
}

int
SetButtonMapping(ClientPtr client, DeviceIntPtr dev, int nElts, BYTE * map)
{
    int i;
    ButtonClassPtr b = dev->button;

    if (b == NULL)
	return BadMatch;

    if (nElts != b->numButtons) {
	client->errorValue = nElts;
	return BadValue;
    }
    if (BadDeviceMap(&map[0], nElts, 1, 255, &client->errorValue))
	return BadValue;
    for (i = 0; i < nElts; i++)
	if ((b->map[i + 1] != map[i]) && BitIsOn(b->down, i + 1))
	    return MappingBusy;
    for (i = 0; i < nElts; i++)
	b->map[i + 1] = map[i];
    return Success;
}

int
SetModifierMapping(ClientPtr client, DeviceIntPtr dev, int len, int rlen,
		   int numKeyPerModifier, KeyCode * inputMap, KeyClassPtr * k)
{
    KeyCode *map = NULL;
    int inputMapLen;
    int i;

    *k = dev->key;
    if (*k == NULL)
	return BadMatch;
    if (len != ((numKeyPerModifier << 1) + rlen))
	return BadLength;

    inputMapLen = 8 * numKeyPerModifier;

    /*
     *  Now enforce the restriction that "all of the non-zero keycodes must be
     *  in the range specified by min-keycode and max-keycode in the
     *  connection setup (else a Value error)"
     */
    i = inputMapLen;
    while (i--) {
	if (inputMap[i]
	    && (inputMap[i] < (*k)->curKeySyms.minKeyCode
		|| inputMap[i] > (*k)->curKeySyms.maxKeyCode)) {
	    client->errorValue = inputMap[i];
	    return -1;	/* BadValue collides with MappingFailed */
	}
    }

    /*
     *  Now enforce the restriction that none of the old or new
     *  modifier keys may be down while we change the mapping,  and
     *  that the DDX layer likes the choice.
     */
    if (!AllModifierKeysAreUp(dev, (*k)->modifierKeyMap,
			      (int)(*k)->maxKeysPerModifier, inputMap,
			      (int)numKeyPerModifier)
	|| !AllModifierKeysAreUp(dev, inputMap, (int)numKeyPerModifier,
				 (*k)->modifierKeyMap,
				 (int)(*k)->maxKeysPerModifier)) {
	return MappingBusy;
    } else {
	for (i = 0; i < inputMapLen; i++) {
	    if (inputMap[i] && !LegalModifier(inputMap[i], dev)) {
		return MappingFailed;
	    }
	}
    }

    /*
     *  Now build the keyboard's modifier bitmap from the
     *  list of keycodes.
     */
    if (inputMapLen) {
	map = (KeyCode *) xalloc(inputMapLen);
	if (!map)
	    return BadAlloc;
    }
    if ((*k)->modifierKeyMap)
	xfree((*k)->modifierKeyMap);
    if (inputMapLen) {
	(*k)->modifierKeyMap = map;
	memmove((char *)(*k)->modifierKeyMap, (char *)inputMap, inputMapLen);
    } else
	(*k)->modifierKeyMap = NULL;

    (*k)->maxKeysPerModifier = numKeyPerModifier;
    for (i = 0; i < MAP_LENGTH; i++)
	(*k)->modifierMap[i] = 0;
    for (i = 0; i < inputMapLen; i++)
	if (inputMap[i]) {
	    (*k)->modifierMap[inputMap[i]]
		|= (1 << (i / (*k)->maxKeysPerModifier));
	}

    return (MappingSuccess);
}

void
SendDeviceMappingNotify(ClientPtr client, CARD8 request,
			KeyCode firstKeyCode, CARD8 count, DeviceIntPtr dev)
{
    xEvent event;
    deviceMappingNotify *ev = (deviceMappingNotify *) & event;

    ev->type = DeviceMappingNotify;
    ev->request = request;
    ev->deviceid = dev->id;
    ev->time = currentTime.milliseconds;
    if (request == MappingKeyboard) {
	ev->firstKeyCode = firstKeyCode;
	ev->count = count;
    }

#ifdef XKB
    if (!noXkbExtension && (request == MappingKeyboard ||
                            request == MappingModifier))
        XkbApplyMappingChange(dev, request, firstKeyCode, count, client);
#endif

    SendEventToAllWindows(dev, DeviceMappingNotifyMask, (xEvent *) ev, 1);
}

int
ChangeKeyMapping(ClientPtr client,
		 DeviceIntPtr dev,
		 unsigned len,
		 int type,
		 KeyCode firstKeyCode,
		 CARD8 keyCodes, CARD8 keySymsPerKeyCode, KeySym * map)
{
    KeySymsRec keysyms;
    KeyClassPtr k = dev->key;

    if (k == NULL)
	return (BadMatch);

    if (len != (keyCodes * keySymsPerKeyCode))
	return BadLength;

    if ((firstKeyCode < k->curKeySyms.minKeyCode) ||
	(firstKeyCode + keyCodes - 1 > k->curKeySyms.maxKeyCode)) {
	client->errorValue = firstKeyCode;
	return BadValue;
    }
    if (keySymsPerKeyCode == 0) {
	client->errorValue = 0;
	return BadValue;
    }
    keysyms.minKeyCode = firstKeyCode;
    keysyms.maxKeyCode = firstKeyCode + keyCodes - 1;
    keysyms.mapWidth = keySymsPerKeyCode;
    keysyms.map = map;
    if (!SetKeySymsMap(&k->curKeySyms, &keysyms))
	return BadAlloc;
    SendDeviceMappingNotify(client, MappingKeyboard, firstKeyCode, keyCodes, dev);
    return client->noClientException;
}

static void
DeleteDeviceFromAnyExtEvents(WindowPtr pWin, DeviceIntPtr dev)
{
    WindowPtr parent;

    /* Deactivate any grabs performed on this window, before making
     * any input focus changes.
     * Deactivating a device grab should cause focus events. */

    if (dev->deviceGrab.grab && (dev->deviceGrab.grab->window == pWin))
	(*dev->deviceGrab.DeactivateGrab) (dev);

    /* If the focus window is a root window (ie. has no parent)
     * then don't delete the focus from it. */

    if (dev->focus && (pWin == dev->focus->win) && (pWin->parent != NullWindow)) {
	int focusEventMode = NotifyNormal;

	/* If a grab is in progress, then alter the mode of focus events. */

	if (dev->deviceGrab.grab)
	    focusEventMode = NotifyWhileGrabbed;

	switch (dev->focus->revert) {
	case RevertToNone:
	    DoFocusEvents(dev, pWin, NoneWin, focusEventMode);
	    dev->focus->win = NoneWin;
	    dev->focus->traceGood = 0;
	    break;
	case RevertToParent:
	    parent = pWin;
	    do {
		parent = parent->parent;
		dev->focus->traceGood--;
	    }
	    while (!parent->realized);
	    DoFocusEvents(dev, pWin, parent, focusEventMode);
	    dev->focus->win = parent;
	    dev->focus->revert = RevertToNone;
	    break;
	case RevertToPointerRoot:
	    DoFocusEvents(dev, pWin, PointerRootWin, focusEventMode);
	    dev->focus->win = PointerRootWin;
	    dev->focus->traceGood = 0;
	    break;
	case RevertToFollowKeyboard:
	    if (inputInfo.keyboard->focus->win) {
		DoFocusEvents(dev, pWin, inputInfo.keyboard->focus->win,
			      focusEventMode);
		dev->focus->win = FollowKeyboardWin;
		dev->focus->traceGood = 0;
	    } else {
		DoFocusEvents(dev, pWin, NoneWin, focusEventMode);
		dev->focus->win = NoneWin;
		dev->focus->traceGood = 0;
	    }
	    break;
	}
    }

    if (dev->valuator)
	if (dev->valuator->motionHintWindow == pWin)
	    dev->valuator->motionHintWindow = NullWindow;
}

void
DeleteWindowFromAnyExtEvents(WindowPtr pWin, Bool freeResources)
{
    int i;
    DeviceIntPtr dev;
    InputClientsPtr ic;
    struct _OtherInputMasks *inputMasks;

    for (dev = inputInfo.devices; dev; dev = dev->next) {
	if (dev == inputInfo.pointer || dev == inputInfo.keyboard)
	    continue;
	DeleteDeviceFromAnyExtEvents(pWin, dev);
    }

    for (dev = inputInfo.off_devices; dev; dev = dev->next)
	DeleteDeviceFromAnyExtEvents(pWin, dev);

    if (freeResources)
	while ((inputMasks = wOtherInputMasks(pWin)) != 0) {
	    ic = inputMasks->inputClients;
	    for (i = 0; i < EMASKSIZE; i++)
		inputMasks->dontPropagateMask[i] = 0;
	    FreeResource(ic->resource, RT_NONE);
	}
}

int
MaybeSendDeviceMotionNotifyHint(deviceKeyButtonPointer * pEvents, Mask mask)
{
    DeviceIntPtr dev;

    dixLookupDevice(&dev, pEvents->deviceid & DEVICE_BITS, serverClient,
		    DixReadAccess);
    if (!dev)
        return 0;

    if (pEvents->type == DeviceMotionNotify) {
	if (mask & DevicePointerMotionHintMask) {
	    if (WID(dev->valuator->motionHintWindow) == pEvents->event) {
		return 1;	/* don't send, but pretend we did */
	    }
	    pEvents->detail = NotifyHint;
	} else {
	    pEvents->detail = NotifyNormal;
	}
    }
    return (0);
}

void
CheckDeviceGrabAndHintWindow(WindowPtr pWin, int type,
			     deviceKeyButtonPointer * xE, GrabPtr grab,
			     ClientPtr client, Mask deliveryMask)
{
    DeviceIntPtr dev;

    dixLookupDevice(&dev, xE->deviceid & DEVICE_BITS, serverClient,
		    DixReadAccess);
    if (!dev)
        return;

    if (type == DeviceMotionNotify)
	dev->valuator->motionHintWindow = pWin;
    else if ((type == DeviceButtonPress) && (!grab) &&
	     (deliveryMask & DeviceButtonGrabMask)) {
	GrabRec tempGrab;

	tempGrab.device = dev;
	tempGrab.resource = client->clientAsMask;
	tempGrab.window = pWin;
	tempGrab.ownerEvents =
	    (deliveryMask & DeviceOwnerGrabButtonMask) ? TRUE : FALSE;
	tempGrab.eventMask = deliveryMask;
	tempGrab.keyboardMode = GrabModeAsync;
	tempGrab.pointerMode = GrabModeAsync;
	tempGrab.confineTo = NullWindow;
	tempGrab.cursor = NullCursor;
        tempGrab.genericMasks = NULL;
        tempGrab.next = NULL;
	(*dev->deviceGrab.ActivateGrab) (dev, &tempGrab, currentTime, TRUE);
    }
}

static Mask
DeviceEventMaskForClient(DeviceIntPtr dev, WindowPtr pWin, ClientPtr client)
{
    InputClientsPtr other;

    if (!wOtherInputMasks(pWin))
	return 0;
    for (other = wOtherInputMasks(pWin)->inputClients; other;
	 other = other->next) {
	if (SameClient(other, client))
	    return other->mask[dev->id];
    }
    return 0;
}

void
MaybeStopDeviceHint(DeviceIntPtr dev, ClientPtr client)
{
    WindowPtr pWin;
    GrabPtr grab = dev->deviceGrab.grab;

    pWin = dev->valuator->motionHintWindow;

    if ((grab && SameClient(grab, client) &&
	 ((grab->eventMask & DevicePointerMotionHintMask) ||
	  (grab->ownerEvents &&
	   (DeviceEventMaskForClient(dev, pWin, client) &
	    DevicePointerMotionHintMask)))) ||
	(!grab &&
	 (DeviceEventMaskForClient(dev, pWin, client) &
	  DevicePointerMotionHintMask)))
	dev->valuator->motionHintWindow = NullWindow;
}

int
DeviceEventSuppressForWindow(WindowPtr pWin, ClientPtr client, Mask mask,
			     int maskndx)
{
    struct _OtherInputMasks *inputMasks = wOtherInputMasks(pWin);

    if (mask & ~PropagateMask[maskndx]) {
	client->errorValue = mask;
	return BadValue;
    }

    if (mask == 0) {
	if (inputMasks)
	    inputMasks->dontPropagateMask[maskndx] = mask;
    } else {
	if (!inputMasks)
	    AddExtensionClient(pWin, client, 0, 0);
	inputMasks = wOtherInputMasks(pWin);
	inputMasks->dontPropagateMask[maskndx] = mask;
    }
    RecalculateDeviceDeliverableEvents(pWin);
    if (ShouldFreeInputMasks(pWin, FALSE))
	FreeResource(inputMasks->inputClients->resource, RT_NONE);
    return Success;
}

Bool
ShouldFreeInputMasks(WindowPtr pWin, Bool ignoreSelectedEvents)
{
    int i;
    Mask allInputEventMasks = 0;
    struct _OtherInputMasks *inputMasks = wOtherInputMasks(pWin);

    for (i = 0; i < EMASKSIZE; i++)
	allInputEventMasks |= inputMasks->dontPropagateMask[i];
    if (!ignoreSelectedEvents)
	for (i = 0; i < EMASKSIZE; i++)
	    allInputEventMasks |= inputMasks->inputEvents[i];
    if (allInputEventMasks == 0)
	return TRUE;
    else
	return FALSE;
}

/***********************************************************************
 *
 * Walk through the window tree, finding all clients that want to know
 * about the Event.
 *
 */

static void
FindInterestedChildren(DeviceIntPtr dev, WindowPtr p1, Mask mask,
                       xEvent * ev, int count)
{
    WindowPtr p2;

    while (p1) {
        p2 = p1->firstChild;
        (void)DeliverEventsToWindow(dev, p1, ev, count, mask, NullGrab, dev->id);
        FindInterestedChildren(dev, p2, mask, ev, count);
        p1 = p1->nextSib;
    }
}

/***********************************************************************
 *
 * Send an event to interested clients in all windows on all screens.
 *
 */

void
SendEventToAllWindows(DeviceIntPtr dev, Mask mask, xEvent * ev, int count)
{
    int i;
    WindowPtr pWin, p1;

    for (i = 0; i < screenInfo.numScreens; i++) {
        pWin = WindowTable[i];
        if (!pWin)
            continue;
        (void)DeliverEventsToWindow(dev, pWin, ev, count, mask, NullGrab, dev->id);
        p1 = pWin->firstChild;
        FindInterestedChildren(dev, p1, mask, ev, count);
    }
}

