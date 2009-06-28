/*
 * Copyright © 2006 Nokia Corporation
 * Copyright © 2006-2007 Daniel Stone
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Daniel Stone <daniel@fooishbar.org>
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <X11/X.h>
#include <X11/keysym.h>
#define NEED_EVENTS
#define NEED_REPLIES
#include <X11/Xproto.h>

#include "misc.h"
#include "resource.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "dixstruct.h"
#include "globals.h"
#include "dixevents.h"
#include "mipointer.h"

#ifdef XKB
#include <X11/extensions/XKBproto.h>
#include <xkbsrv.h>
extern Bool XkbCopyKeymap(XkbDescPtr src, XkbDescPtr dst, Bool sendNotifies);
#endif

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"
#endif

#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "exglobals.h"
#include "exevents.h"
#include "exglobals.h"
#include "extnsionst.h"


/* Maximum number of valuators, divided by six, rounded up, to get number
 * of events. */
#define MAX_VALUATOR_EVENTS 6

/* Number of motion history events to store. */
#define MOTION_HISTORY_SIZE 256


/**
 * Pick some arbitrary size for Xi motion history.
 */
_X_EXPORT int
GetMotionHistorySize(void)
{
    return MOTION_HISTORY_SIZE;
}

static void
set_key_down(DeviceIntPtr pDev, int key_code)
{
    pDev->key->postdown[key_code >> 3] |= (1 << (key_code & 7));
}

static void
set_key_up(DeviceIntPtr pDev, int key_code)
{
    pDev->key->postdown[key_code >> 3] &= ~(1 << (key_code & 7));
}

static Bool
key_is_down(DeviceIntPtr pDev, int key_code)
{
    return !!(pDev->key->postdown[key_code >> 3] & (1 << (key_code & 7)));
}

static Bool
key_autorepeats(DeviceIntPtr pDev, int key_code)
{
    return !!(pDev->kbdfeed->ctrl.autoRepeats[key_code >> 3] &
              (1 << (key_code & 7)));
}

/**
 * Allocate the motion history buffer.
 */
_X_EXPORT void
AllocateMotionHistory(DeviceIntPtr pDev)
{
    if (pDev->valuator->motion)
        xfree(pDev->valuator->motion);

    if (pDev->valuator->numMotionEvents < 1)
        return;

    pDev->valuator->motion = xalloc(((sizeof(INT32) * pDev->valuator->numAxes) +
                                     sizeof(Time)) *
                                    pDev->valuator->numMotionEvents);
    pDev->valuator->first_motion = 0;
    pDev->valuator->last_motion = 0;
}


/**
 * Dump the motion history between start and stop into the supplied buffer.
 * Only records the event for a given screen in theory, but in practice, we
 * sort of ignore this.
 */
_X_EXPORT int
GetMotionHistory(DeviceIntPtr pDev, xTimecoord *buff, unsigned long start,
                 unsigned long stop, ScreenPtr pScreen)
{
    char *ibuff = NULL, *obuff = (char *) buff;
    int i = 0, ret = 0;
    Time current;
    /* The size of a single motion event. */
    int size = (sizeof(INT32) * pDev->valuator->numAxes) + sizeof(Time);

    if (!pDev->valuator || !pDev->valuator->numMotionEvents)
        return 0;

    for (i = pDev->valuator->first_motion;
         i != pDev->valuator->last_motion;
         i = (i + 1) % pDev->valuator->numMotionEvents) {
        /* We index the input buffer by which element we're accessing, which
         * is not monotonic, and the output buffer by how many events we've
         * written so far. */
        ibuff = (char *) pDev->valuator->motion + (i * size);
        memcpy(&current, ibuff, sizeof(Time));

        if (current > stop) {
            return ret;
        }
        else if (current >= start) {
            memcpy(obuff, ibuff, size);
            obuff += size;
            ret++;
        }
    }

    return ret;
}


/**
 * Update the motion history for a specific device, with the list of
 * valuators.
 */
static void
updateMotionHistory(DeviceIntPtr pDev, CARD32 ms, int first_valuator,
                    int num_valuators, int *valuators)
{
    char *buff = (char *) pDev->valuator->motion;

    if (!pDev->valuator->numMotionEvents)
        return;

    buff += ((sizeof(INT32) * pDev->valuator->numAxes) + sizeof(CARD32)) *
            pDev->valuator->last_motion;
    memcpy(buff, &ms, sizeof(Time));

    buff += sizeof(Time);
    bzero(buff, sizeof(INT32) * pDev->valuator->numAxes);

    buff += sizeof(INT32) * first_valuator;
    memcpy(buff, valuators, sizeof(INT32) * num_valuators);

    pDev->valuator->last_motion = (pDev->valuator->last_motion + 1) %
                                  pDev->valuator->numMotionEvents;

    /* If we're wrapping around, just keep the circular buffer going. */
    if (pDev->valuator->first_motion == pDev->valuator->last_motion)
        pDev->valuator->first_motion = (pDev->valuator->first_motion + 1) %
                                       pDev->valuator->numMotionEvents;

    return;
}


/**
 * Returns the maximum number of events GetKeyboardEvents,
 * GetKeyboardValuatorEvents, and GetPointerEvents will ever return.
 *
 * Should be used in DIX as:
 * xEvent *events = xcalloc(sizeof(xEvent), GetMaximumEventsNum());
 *
 * This MUST be absolutely constant, from init until exit.
 */
_X_EXPORT int
GetMaximumEventsNum(void) {
    /* Two base events -- core and device, plus valuator events.  Multiply
     * by two if we're doing non-XKB key repeats. */
    int ret = 2 + MAX_VALUATOR_EVENTS;

#ifdef XKB
    if (noXkbExtension)
#endif
        ret *= 2;

    return ret;
}


/* Originally a part of xf86PostMotionEvent; modifies valuators
 * in-place. */
static void
acceleratePointer(DeviceIntPtr pDev, int first_valuator, int num_valuators,
                  int *valuators)
{
    float mult = 0.0;
    int dx = 0, dy = 0;
    int *px = NULL, *py = NULL;

    if (!num_valuators || !valuators)
        return;

    if (first_valuator == 0) {
        dx = valuators[0];
        px = &valuators[0];
    }
    if (first_valuator <= 1 && num_valuators >= (2 - first_valuator)) {
        dy = valuators[1 - first_valuator];
        py = &valuators[1 - first_valuator];
    }

    if (!dx && !dy)
        return;

    if (pDev->ptrfeed && pDev->ptrfeed->ctrl.num) {
        /* modeled from xf86Events.c */
        if (pDev->ptrfeed->ctrl.threshold) {
            if ((abs(dx) + abs(dy)) >= pDev->ptrfeed->ctrl.threshold) {
                pDev->valuator->dxremaind = ((float)dx *
                                             (float)(pDev->ptrfeed->ctrl.num)) /
                                             (float)(pDev->ptrfeed->ctrl.den) +
                                            pDev->valuator->dxremaind;
                if (px) {
                    *px = (int)pDev->valuator->dxremaind;
                    pDev->valuator->dxremaind = pDev->valuator->dxremaind -
                                                (float)(*px);
                }

                pDev->valuator->dyremaind = ((float)dy *
                                             (float)(pDev->ptrfeed->ctrl.num)) /
                                             (float)(pDev->ptrfeed->ctrl.den) +
                                            pDev->valuator->dyremaind;
                if (py) {
                    *py = (int)pDev->valuator->dyremaind;
                    pDev->valuator->dyremaind = pDev->valuator->dyremaind -
                                                (float)(*py);
                }
            }
        }
        else {
	    mult = pow((float)dx * (float)dx + (float)dy * (float)dy,
                       ((float)(pDev->ptrfeed->ctrl.num) /
                        (float)(pDev->ptrfeed->ctrl.den) - 1.0) /
                       2.0) / 2.0;
            if (dx) {
                pDev->valuator->dxremaind = mult * (float)dx +
                                            pDev->valuator->dxremaind;
                *px = (int)pDev->valuator->dxremaind;
                pDev->valuator->dxremaind = pDev->valuator->dxremaind -
                                            (float)(*px);
            }
            if (dy) {
                pDev->valuator->dyremaind = mult * (float)dy +
                                            pDev->valuator->dyremaind;
                *py = (int)pDev->valuator->dyremaind;
                pDev->valuator->dyremaind = pDev->valuator->dyremaind -
                                            (float)(*py);
            }
        }
    }
}


/**
 * Clip an axis to its bounds, which are declared in the call to
 * InitValuatorAxisClassStruct.
 */
static void
clipAxis(DeviceIntPtr pDev, int axisNum, int *val)
{
    AxisInfoPtr axes = pDev->valuator->axes + axisNum;

    /* No clipping if the value-range <= 0 */
    if(axes->min_value < axes->min_value) {
        if (*val < axes->min_value)
            *val = axes->min_value;
        if (*val > axes->max_value)
            *val = axes->max_value;
    }
}

/**
 * Clip every axis in the list of valuators to its bounds.
 */
static void
clipValuators(DeviceIntPtr pDev, int first_valuator, int num_valuators,
              int *valuators)
{
    AxisInfoPtr axes = pDev->valuator->axes + first_valuator;
    int i;

    for (i = 0; i < num_valuators; i++, axes++)
        clipAxis(pDev, i + first_valuator, &(valuators[i]));
}


/**
 * Fills events with valuator events for pDev, as given by the other
 * parameters.
 *
 * FIXME: Need to fix ValuatorClassRec to store all the valuators as
 *        last posted, not just x and y; otherwise relative non-x/y
 *        valuators, though a very narrow use case, will be broken.
 */
static xEvent *
getValuatorEvents(xEvent *events, DeviceIntPtr pDev, int first_valuator,
                  int num_valuators, int *valuators) {
    deviceValuator *xv = (deviceValuator *) events;
    int i = 0, final_valuator = first_valuator + num_valuators;

    for (i = first_valuator; i < final_valuator; i += 6, xv++, events++) {
        xv->type = DeviceValuator;
        xv->first_valuator = i;
        xv->num_valuators = ((final_valuator - i) > 6) ? 6 : (final_valuator - i);
        xv->deviceid = pDev->id;
        switch (final_valuator - i) {
        case 6:
            xv->valuator5 = valuators[i + 5];
        case 5:
            xv->valuator4 = valuators[i + 4];
        case 4:
            xv->valuator3 = valuators[i + 3];
        case 3:
            xv->valuator2 = valuators[i + 2];
        case 2:
            xv->valuator1 = valuators[i + 1];
        case 1:
            xv->valuator0 = valuators[i];
        }

        if (i + 6 < final_valuator)
            xv->deviceid |= MORE_EVENTS;
    }

    return events;
}


/**
 * Convenience wrapper around GetKeyboardValuatorEvents, that takes no
 * valuators.
 */
_X_EXPORT int
GetKeyboardEvents(xEvent *events, DeviceIntPtr pDev, int type, int key_code) {
    return GetKeyboardValuatorEvents(events, pDev, type, key_code, 0, 0, NULL);
}


/**
 * Returns a set of keyboard events for KeyPress/KeyRelease, optionally
 * also with valuator events.  Handles Xi and XKB.
 *
 * events is not NULL-terminated; the return value is the number of events.
 * The DDX is responsible for allocating the event structure in the first
 * place via GetMaximumEventsNum(), and for freeing it.
 *
 * This function does not change the core keymap to that of the device;
 * that is done by SwitchCoreKeyboard, which is called from
 * mieqProcessInputEvents.  If replacing that function, take care to call
 * SetCoreKeyboard before processInputProc, so keymaps are altered to suit.
 *
 * Note that this function recurses!  If called for non-XKB, a repeating
 * key press will trigger a matching KeyRelease, as well as the
 * KeyPresses.
 */
_X_EXPORT int
GetKeyboardValuatorEvents(xEvent *events, DeviceIntPtr pDev, int type,
                          int key_code, int first_valuator,
                          int num_valuators, int *valuators) {
    int numEvents = 0;
    CARD32 ms = 0;
    KeySym *map = pDev->key->curKeySyms.map;
    KeySym sym;
    deviceKeyButtonPointer *kbp = NULL;

    if (!events)
        return 0;

    /* DO NOT WANT */
    if (type != KeyPress && type != KeyRelease)
        return 0;

    if (!pDev->key || !pDev->focus || !pDev->kbdfeed ||
        (pDev->coreEvents && !inputInfo.keyboard->key))
        return 0;

    if (key_code < 8 || key_code > 255)
        return 0;

    sym = map[(key_code - pDev->key->curKeySyms.minKeyCode)
              * pDev->key->curKeySyms.mapWidth];

    if (pDev->coreEvents)
        numEvents = 2;
    else
        numEvents = 1;

    if (num_valuators) {
        if ((num_valuators / 6) + 1 > MAX_VALUATOR_EVENTS)
            num_valuators = MAX_VALUATOR_EVENTS;
        numEvents += (num_valuators / 6) + 1;
    }

#ifdef XKB
    if (noXkbExtension)
#endif
    {
        switch (sym) {
            case XK_Num_Lock:
            case XK_Caps_Lock:
            case XK_Scroll_Lock:
            case XK_Shift_Lock:
                if (type == KeyRelease)
                    return 0;
                else if (type == KeyPress && key_is_down(pDev, key_code))
                    type = KeyRelease;
        }
    }

    /* Handle core repeating, via press/release/press/release.
     * FIXME: In theory, if you're repeating with two keyboards in non-XKB,
     *        you could get unbalanced events here. */
    if (type == KeyPress && key_is_down(pDev, key_code)) {
        /* If autorepeating is disabled either globally or just for that key,
         * or we have a modifier, don't generate a repeat event. */
        if (!pDev->kbdfeed->ctrl.autoRepeat ||
            !key_autorepeats(pDev, key_code) ||
            pDev->key->modifierMap[key_code])
            return 0;

#ifdef XKB
        if (noXkbExtension)
#endif
        {
            numEvents += GetKeyboardValuatorEvents(events, pDev,
                                                   KeyRelease, key_code,
                                                   first_valuator, num_valuators,
                                                   valuators);
            events += numEvents;
        }
    }

    ms = GetTimeInMillis();

    if (pDev->coreEvents) {
        events->u.keyButtonPointer.time = ms;
        events->u.u.type = type;
        events->u.u.detail = key_code;
        if (type == KeyPress)
	    set_key_down(inputInfo.keyboard, key_code);
        else if (type == KeyRelease)
	    set_key_up(inputInfo.keyboard, key_code);
        events++;
    }

    kbp = (deviceKeyButtonPointer *) events;
    kbp->time = ms;
    kbp->deviceid = pDev->id;
    kbp->detail = key_code;
    if (type == KeyPress) {
        kbp->type = DeviceKeyPress;
	set_key_down(pDev, key_code);
    }
    else if (type == KeyRelease) {
        kbp->type = DeviceKeyRelease;
	set_key_up(pDev, key_code);
    }

    events++;
    if (num_valuators) {
        kbp->deviceid |= MORE_EVENTS;
        clipValuators(pDev, first_valuator, num_valuators, valuators);
        events = getValuatorEvents(events, pDev, first_valuator,
                                   num_valuators, valuators);
    }

    return numEvents;
}


/**
 * Generate a series of xEvents (returned in xE) representing pointer
 * motion, or button presses.  Xi and XKB-aware.
 *
 * events is not NULL-terminated; the return value is the number of events.
 * The DDX is responsible for allocating the event structure in the first
 * place via GetMaximumEventsNum(), and for freeing it.
 */
_X_EXPORT int
GetPointerEvents(xEvent *events, DeviceIntPtr pDev, int type, int buttons,
                 int flags, int first_valuator, int num_valuators,
                 int *valuators) {
    int num_events = 0, final_valuator = 0;
    CARD32 ms = 0;
    deviceKeyButtonPointer *kbp = NULL;
    DeviceIntPtr cp = inputInfo.pointer;
    int x = 0, y = 0;
    Bool coreOnly = (pDev == inputInfo.pointer);
    ScreenPtr scr = miPointerGetScreen(pDev);

    /* Sanity checks. */
    if (type != MotionNotify && type != ButtonPress && type != ButtonRelease)
        return 0;

    if ((type == ButtonPress || type == ButtonRelease) && !pDev->button)
        return 0;

    /* FIXME: I guess it should, in theory, be possible to post button events
     *        from devices without valuators. */
    if (!pDev->valuator)
        return 0;

    if (!coreOnly && pDev->coreEvents)
        num_events = 2;
    else
        num_events = 1;

    if (type == MotionNotify && num_valuators <= 0)
        return 0;

    /* Do we need to send a DeviceValuator event? */
    if (!coreOnly && num_valuators) {
        if ((((num_valuators - 1) / 6) + 1) > MAX_VALUATOR_EVENTS)
            num_valuators = MAX_VALUATOR_EVENTS * 6;
        num_events += ((num_valuators - 1) / 6) + 1;
    }

    final_valuator = num_valuators + first_valuator;

    /* You fail. */
    if (first_valuator < 0 || final_valuator > pDev->valuator->numAxes)
        return 0;

    ms = GetTimeInMillis();

    /* Set x and y based on whether this is absolute or relative, and
     * accelerate if we need to. */
    if (flags & POINTER_ABSOLUTE) {
        if (num_valuators >= 1 && first_valuator == 0) {
            x = valuators[0];
        }
        else {
            /* If we're sending core events but didn't provide a value,
             * translate the core value (but use the device coord if
             * it translates to the same coord to preserve sub-pixel
             * coord information). If we're not sending core events use
             * whatever value we have */
            x = pDev->valuator->lastx;
            if(pDev->coreEvents) {
                int min = pDev->valuator->axes[0].min_value;
                int max = pDev->valuator->axes[0].max_value;
                if(min < max) {
                    if((int)((float)(x-min)*scr->width/(max-min+1)) != cp->valuator->lastx)
                        x = (int)((float)(cp->valuator->lastx)*(max-min+1)/scr->width)+min;
                }
                else
                    x = cp->valuator->lastx;
            }
        }

        if (first_valuator <= 1 && num_valuators >= (2 - first_valuator)) {
            y = valuators[1 - first_valuator];
        }
        else {
            y = pDev->valuator->lasty;
            if(pDev->coreEvents) {
                int min = pDev->valuator->axes[1].min_value;
                int max = pDev->valuator->axes[1].max_value;
                if(min < max) {
                    if((int)((float)(y-min)*scr->height/(max-min+1)) != cp->valuator->lasty)
                        y = (int)((float)(cp->valuator->lasty)*(max-min+1)/scr->height)+min;
                }
                else
                    y = cp->valuator->lasty;
            }
        }

        /* Clip both x and y to the defined limits (usually co-ord space limit). */
        clipAxis(pDev, 0, &x);
        clipAxis(pDev, 1, &y);
    }
    else {
        if (flags & POINTER_ACCELERATE)
            acceleratePointer(pDev, first_valuator, num_valuators,
                              valuators);

        if (pDev->coreEvents) {
            /* Get and convert the core pointer coordinate space into
             * device coordinates. Use the device coords if it translates
             * into the same position as the core to preserve relative sub-
             * pixel movements from the device. */
            int min = pDev->valuator->axes[0].min_value;
            int max = pDev->valuator->axes[0].max_value;
            if(min < max) {
                x = pDev->valuator->lastx;
                if((int)((float)(x-min)*scr->width/(max-min+1)) != cp->valuator->lastx)
                    x = (int)((float)(cp->valuator->lastx)*(max-min+1)/scr->width)+min;
            }
            else
                x = cp->valuator->lastx;

            min = pDev->valuator->axes[1].min_value;
            max = pDev->valuator->axes[1].max_value;
            if(min < max) {
                y = pDev->valuator->lasty;
                if((int)((float)(y-min)*scr->height/(max-min+1)) != cp->valuator->lasty)
                    y = (int)((float)(cp->valuator->lasty)*(max-min+1)/scr->height)+min;
            }
            else
                y = cp->valuator->lasty;

            /* Add relative movement */
            if (first_valuator == 0 && num_valuators >= 1)
                x += valuators[0];
            if (first_valuator <= 1 && num_valuators >= (2 - first_valuator))
                y += valuators[1 - first_valuator];
        }
        else {
            x = pDev->valuator->lastx;
            y = pDev->valuator->lasty;
            if (first_valuator == 0 && num_valuators >= 1)
                x += valuators[0];
            if (first_valuator <= 1 && num_valuators >= (2 - first_valuator))
                y += valuators[1 - first_valuator];

            if(!coreOnly) {
                /* Since we're not sending core-events we must clip both x and y
                 * to the defined limits so we don't run outside the box. */
                clipAxis(pDev, 0, &x);
                clipAxis(pDev, 1, &y);
            }
        }
    }

    pDev->valuator->lastx = x;
    pDev->valuator->lasty = y;
    /* Convert the dev coord back to screen coord if we're
     * sending core events */
    if (pDev->coreEvents) {
        int min = pDev->valuator->axes[0].min_value;
        int max = pDev->valuator->axes[0].max_value;
        if(min < max)
            x = (int)((float)(x-min)*scr->width/(max-min+1));
        cp->valuator->lastx = x;
        min = pDev->valuator->axes[1].min_value;
        max = pDev->valuator->axes[1].max_value;
        if(min < max)
            y = (int)((float)(y-min)*scr->height/(max-min+1));
        cp->valuator->lasty = y;
    }

    /* This takes care of crossing screens for us, as well as clipping
     * to the current screen.  Right now, we only have one history buffer,
     * so we don't set this for both the device and core.*/
    miPointerSetPosition(pDev, &x, &y, ms);

    if (pDev->coreEvents) {
        /* miPointerSetPosition may have changed screen */
        scr = miPointerGetScreen(pDev);
        if(x != cp->valuator->lastx) {
            int min = pDev->valuator->axes[0].min_value;
            int max = pDev->valuator->axes[0].max_value;
            cp->valuator->lastx = pDev->valuator->lastx = x;
            if(min < max)
                pDev->valuator->lastx = (int)((float)(x)*(max-min+1)/scr->width)+min;
        }
        if(y != cp->valuator->lasty) {
            int min = pDev->valuator->axes[1].min_value;
            int max = pDev->valuator->axes[1].max_value;
            cp->valuator->lasty = pDev->valuator->lasty = y;
            if(min < max)
                pDev->valuator->lasty = (int)((float)(y)*(max-min+1)/scr->height)+min;
        }
    }
    else if (coreOnly) {
        cp->valuator->lastx = x;
        cp->valuator->lasty = y;
    }

    /* Drop x and y back into the valuators list, if they were originally
     * present. */
    if (first_valuator == 0 && num_valuators >= 1)
        valuators[0] = pDev->valuator->lastx;
    if (first_valuator <= 1 && num_valuators >= (2 - first_valuator))
        valuators[1 - first_valuator] = pDev->valuator->lasty;

    updateMotionHistory(pDev, ms, first_valuator, num_valuators, valuators);

    /* for some reason inputInfo.pointer does not have coreEvents set */
    if (coreOnly || pDev->coreEvents) {
        events->u.u.type = type;
        events->u.keyButtonPointer.time = ms;
        events->u.keyButtonPointer.rootX = x;
        events->u.keyButtonPointer.rootY = y;

        if (type == ButtonPress || type == ButtonRelease) {
            /* We hijack SetPointerMapping to work on all core-sending
             * devices, so we use the device-specific map here instead of
             * the core one. */
            events->u.u.detail = pDev->button->map[buttons];
        }
        else {
            events->u.u.detail = 0;
        }

        events++;
    }

    if (!coreOnly) {
        kbp = (deviceKeyButtonPointer *) events;
        kbp->time = ms;
        kbp->deviceid = pDev->id;

        if (type == MotionNotify) {
            kbp->type = DeviceMotionNotify;
        }
        else {
            if (type == ButtonPress)
                kbp->type = DeviceButtonPress;
            else if (type == ButtonRelease)
                kbp->type = DeviceButtonRelease;
            kbp->detail = pDev->button->map[buttons];
        }

        kbp->root_x = pDev->valuator->lastx;
        kbp->root_y = pDev->valuator->lasty;

        events++;
        if (num_valuators) {
            kbp->deviceid |= MORE_EVENTS;
            clipValuators(pDev, first_valuator, num_valuators, valuators);
            events = getValuatorEvents(events, pDev, first_valuator,
                                       num_valuators, valuators);
        }
    }

    return num_events;
}


/**
 * Post ProximityIn/ProximityOut events, accompanied by valuators.
 *
 * events is not NULL-terminated; the return value is the number of events.
 * The DDX is responsible for allocating the event structure in the first
 * place via GetMaximumEventsNum(), and for freeing it.
 */
_X_EXPORT int
GetProximityEvents(xEvent *events, DeviceIntPtr pDev, int type,
                   int first_valuator, int num_valuators, int *valuators)
{
    int num_events = 1;
    deviceKeyButtonPointer *kbp = (deviceKeyButtonPointer *) events;

    /* Sanity checks. */
    if (type != ProximityIn && type != ProximityOut)
        return 0;

    if (!pDev->valuator)
        return 0;

    /* Do we need to send a DeviceValuator event? */
    if ((pDev->valuator->mode & 1) == Relative)
        num_valuators = 0;

    if (num_valuators) {
        if ((((num_valuators - 1) / 6) + 1) > MAX_VALUATOR_EVENTS)
            num_valuators = MAX_VALUATOR_EVENTS * 6;
        num_events += ((num_valuators - 1) / 6) + 1;
    }

    /* You fail. */
    if (first_valuator < 0 ||
        (num_valuators + first_valuator) > pDev->valuator->numAxes)
        return 0;

    kbp->type = type;
    kbp->deviceid = pDev->id;
    kbp->detail = 0;
    kbp->time = GetTimeInMillis();

    if (num_valuators) {
        kbp->deviceid |= MORE_EVENTS;
        events++;
        clipValuators(pDev, first_valuator, num_valuators, valuators);
        events = getValuatorEvents(events, pDev, first_valuator,
                                   num_valuators, valuators);
    }

    return num_events;
}


/**
 * Note that pDev was the last device to send a core event.  This function
 * copies the complete keymap from the originating device to the core
 * device, and makes sure the appropriate notifications are generated.
 *
 * Call this just before processInputProc.
 */
_X_EXPORT void
SwitchCoreKeyboard(DeviceIntPtr pDev)
{
    KeyClassPtr ckeyc = inputInfo.keyboard->key;
    int i = 0;

    if (pDev != dixLookupPrivate(&inputInfo.keyboard->devPrivates,
				 CoreDevicePrivateKey)) {
        memcpy(ckeyc->modifierMap, pDev->key->modifierMap, MAP_LENGTH);
        if (ckeyc->modifierKeyMap)
            xfree(ckeyc->modifierKeyMap);
        ckeyc->modifierKeyMap = xalloc(8 * pDev->key->maxKeysPerModifier);
        memcpy(ckeyc->modifierKeyMap, pDev->key->modifierKeyMap,
                (8 * pDev->key->maxKeysPerModifier));

        ckeyc->maxKeysPerModifier = pDev->key->maxKeysPerModifier;
        ckeyc->curKeySyms.minKeyCode = pDev->key->curKeySyms.minKeyCode;
        ckeyc->curKeySyms.maxKeyCode = pDev->key->curKeySyms.maxKeyCode;
        SetKeySymsMap(&ckeyc->curKeySyms, &pDev->key->curKeySyms);

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
        ckeyc->state &= ~(KEYBOARD_MASK);
        ckeyc->state |= (pDev->key->state & KEYBOARD_MASK);
#undef KEYBOARD_MASK
        for (i = 0; i < 8; i++)
            ckeyc->modifierKeyCount[i] = pDev->key->modifierKeyCount[i];

#ifdef XKB
        if (!noXkbExtension && pDev->key->xkbInfo && pDev->key->xkbInfo->desc) {
            if (!XkbCopyKeymap(pDev->key->xkbInfo->desc, ckeyc->xkbInfo->desc,
                               True))
                FatalError("Couldn't pivot keymap from device to core!\n");
        }
#endif

        SendMappingNotify(MappingKeyboard, ckeyc->curKeySyms.minKeyCode,
                          (ckeyc->curKeySyms.maxKeyCode -
                           ckeyc->curKeySyms.minKeyCode),
                          serverClient);
	dixSetPrivate(&inputInfo.keyboard->devPrivates, CoreDevicePrivateKey,
		      pDev);
    }
}


/**
 * Note that pDev was the last function to send a core pointer event.
 * Currently a no-op.
 *
 * Call this just before processInputProc.
 */
_X_EXPORT void
SwitchCorePointer(DeviceIntPtr pDev)
{
    if (pDev != dixLookupPrivate(&inputInfo.pointer->devPrivates,
				 CoreDevicePrivateKey))
	dixSetPrivate(&inputInfo.pointer->devPrivates,
		      CoreDevicePrivateKey, pDev);
}


/**
 * Synthesize a single motion event for the core pointer.
 *
 * Used in cursor functions, e.g. when cursor confinement changes, and we need
 * to shift the pointer to get it inside the new bounds.
 */
void
PostSyntheticMotion(int x, int y, int screen, unsigned long time)
{
    xEvent xE;

#ifdef PANORAMIX
    /* Translate back to the sprite screen since processInputProc
       will translate from sprite screen to screen 0 upon reentry
       to the DIX layer. */
    if (!noPanoramiXExtension) {
        x += panoramiXdataPtr[0].x - panoramiXdataPtr[screen].x;
        y += panoramiXdataPtr[0].y - panoramiXdataPtr[screen].y;
    }
#endif

    memset(&xE, 0, sizeof(xEvent));
    xE.u.u.type = MotionNotify;
    xE.u.keyButtonPointer.rootX = x;
    xE.u.keyButtonPointer.rootY = y;
    xE.u.keyButtonPointer.time = time;

    (*inputInfo.pointer->public.processInputProc)(&xE, inputInfo.pointer, 1);
}
