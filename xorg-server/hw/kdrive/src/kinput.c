/*
 * Copyright © 1999 Keith Packard
 * Copyright © 2006 Nokia Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors make no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "inputstr.h"

#define XK_PUBLISHING
#include <X11/keysym.h>
#if HAVE_X11_XF86KEYSYM_H
#include <X11/XF86keysym.h>
#endif
#include <signal.h>
#include <stdio.h>
#ifdef sun
#include <sys/file.h> /* needed for FNONBLOCK & FASYNC */
#endif

#include "xkbsrv.h"

#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "XIstubs.h" /* even though we don't use stubs.  cute, no? */
#include "exevents.h"
#include "extinit.h"
#include "exglobals.h"
#include "eventstr.h"
#include "xserver-properties.h"
#include "inpututils.h"

#define AtomFromName(x) MakeAtom(x, strlen(x), 1)

struct KdConfigDevice {
    char *line;
    struct KdConfigDevice *next;
};

/* kdKeyboards and kdPointers hold all the real devices. */
static KdKeyboardInfo *kdKeyboards         = NULL;
static KdPointerInfo  *kdPointers          = NULL;
static struct KdConfigDevice *kdConfigKeyboards   = NULL;
static struct KdConfigDevice *kdConfigPointers    = NULL;

static KdKeyboardDriver *kdKeyboardDrivers = NULL;
static KdPointerDriver  *kdPointerDrivers  = NULL;

static Bool		kdInputEnabled;
static Bool		kdOffScreen;
static unsigned long	kdOffScreenTime;
static KdPointerMatrix	kdPointerMatrix = {
   { { 1, 0, 0 },
     { 0, 1, 0 } }
};

void KdResetInputMachine (void);

#define KD_MAX_INPUT_FDS    8

typedef struct _kdInputFd {
    int	        fd;
    void        (*read) (int fd, void *closure);
    int	        (*enable) (int fd, void *closure);
    void        (*disable) (int fd, void *closure);
    void        *closure;
} KdInputFd;

static KdInputFd kdInputFds[KD_MAX_INPUT_FDS];
static int	 kdNumInputFds;

extern Bool      kdRawPointerCoordinates;

static void
KdSigio (int sig)
{
    int	i;

    for (i = 0; i < kdNumInputFds; i++)
	(*kdInputFds[i].read) (kdInputFds[i].fd, kdInputFds[i].closure);
}

static void
KdBlockSigio (void)
{
    sigset_t	set;

    sigemptyset (&set);
    sigaddset (&set, SIGIO);
    sigprocmask (SIG_BLOCK, &set, 0);
}

static void
KdUnblockSigio (void)
{
    sigset_t	set;

    sigemptyset (&set);
    sigaddset (&set, SIGIO);
    sigprocmask (SIG_UNBLOCK, &set, 0);
}

#ifdef DEBUG_SIGIO

void
KdAssertSigioBlocked (char *where)
{
    sigset_t	set, old;

    sigemptyset (&set);
    sigprocmask (SIG_BLOCK, &set, &old);
    if (!sigismember (&old, SIGIO)) {
	ErrorF ("SIGIO not blocked at %s\n", where);
        KdBacktrace(0);
    }
}

#else

#define KdAssertSigioBlocked(s)

#endif

static int  kdnFds;

#ifdef FNONBLOCK
#define NOBLOCK FNONBLOCK
#else
#define NOBLOCK FNDELAY
#endif

void
KdResetInputMachine (void)
{
    KdPointerInfo *pi;

    for (pi = kdPointers; pi; pi = pi->next) {
        pi->mouseState = start;
        pi->eventHeld = FALSE;
    }
}

static void
KdNonBlockFd (int fd)
{
    int	flags;
    flags = fcntl (fd, F_GETFL);
    flags |= FASYNC|NOBLOCK;
    fcntl (fd, F_SETFL, flags);
}

static void
KdAddFd (int fd)
{
    struct sigaction	act;
    sigset_t		set;

    kdnFds++;
    fcntl (fd, F_SETOWN, getpid());
    KdNonBlockFd (fd);
    AddEnabledDevice (fd);
    memset (&act, '\0', sizeof act);
    act.sa_handler = KdSigio;
    sigemptyset (&act.sa_mask);
    sigaddset (&act.sa_mask, SIGIO);
    sigaddset (&act.sa_mask, SIGALRM);
    sigaddset (&act.sa_mask, SIGVTALRM);
    sigaction (SIGIO, &act, 0);
    sigemptyset (&set);
    sigprocmask (SIG_SETMASK, &set, 0);
}

static void
KdRemoveFd (int fd)
{
    struct sigaction	act;
    int			flags;

    kdnFds--;
    RemoveEnabledDevice (fd);
    flags = fcntl (fd, F_GETFL);
    flags &= ~(FASYNC|NOBLOCK);
    fcntl (fd, F_SETFL, flags);
    if (kdnFds == 0)
    {
	memset (&act, '\0', sizeof act);
	act.sa_handler = SIG_IGN;
	sigemptyset (&act.sa_mask);
	sigaction (SIGIO, &act, 0);
    }
}

Bool
KdRegisterFd (int fd, void (*read) (int fd, void *closure), void *closure)
{
    if (kdNumInputFds == KD_MAX_INPUT_FDS)
	return FALSE;
    kdInputFds[kdNumInputFds].fd = fd;
    kdInputFds[kdNumInputFds].read = read;
    kdInputFds[kdNumInputFds].enable = 0;
    kdInputFds[kdNumInputFds].disable = 0;
    kdInputFds[kdNumInputFds].closure = closure;
    kdNumInputFds++;
    if (kdInputEnabled)
	KdAddFd (fd);
    return TRUE;
}

void
KdUnregisterFd (void *closure, int fd, Bool do_close)
{
    int	i, j;

    for (i = 0; i < kdNumInputFds; i++) {
	if (kdInputFds[i].closure == closure &&
            (fd == -1 || kdInputFds[i].fd == fd)) {
	    if (kdInputEnabled)
		KdRemoveFd (kdInputFds[i].fd);
	    if (do_close)
		close (kdInputFds[i].fd);
	    kdNumInputFds--;
	    for (j = i; j < kdNumInputFds; j++)
		kdInputFds[j] = kdInputFds[j+1];
            break;
	}
    }
}

void
KdUnregisterFds (void *closure, Bool do_close)
{
    KdUnregisterFd(closure, -1, do_close);
}

void
KdDisableInput (void)
{
    KdKeyboardInfo *ki;
    KdPointerInfo *pi;
    int found = 0, i = 0;

    KdBlockSigio();

    for (ki = kdKeyboards; ki; ki = ki->next) {
        if (ki->driver && ki->driver->Disable)
            (*ki->driver->Disable) (ki);
    }

    for (pi = kdPointers; pi; pi = pi->next) {
        if (pi->driver && pi->driver->Disable)
            (*pi->driver->Disable) (pi);
    }

    if (kdNumInputFds) {
        ErrorF("[KdDisableInput] Buggy drivers: still %d input fds left!",
               kdNumInputFds);
        i = 0;
        while (i < kdNumInputFds) {
            found = 0;
            for (ki = kdKeyboards; ki; ki = ki->next) {
                if (ki == kdInputFds[i].closure) {
                    ErrorF("    fd %d belongs to keybd driver %s\n",
                           kdInputFds[i].fd,
                           ki->driver && ki->driver->name ?
                             ki->driver->name : "(unnamed!)");
                    found = 1;
                    break;
                }
            }

            if (found) {
                i++;
                continue;
            }

            for (pi = kdPointers; pi; pi = pi->next) {
                if (pi == kdInputFds[i].closure) {
                    ErrorF("    fd %d belongs to pointer driver %s\n",
                           kdInputFds[i].fd,
                           pi->driver && pi->driver->name ?
                             pi->driver->name : "(unnamed!)");
                    break;
                }
            }

            if (found) {
                i++;
                continue;
            }

            ErrorF("    fd %d not claimed by any active device!\n",
                   kdInputFds[i].fd);
            KdUnregisterFd(kdInputFds[i].closure, kdInputFds[i].fd, TRUE);
        }
    }

    kdInputEnabled = FALSE;
}

void
KdEnableInput (void)
{
    InternalEvent ev;
    KdKeyboardInfo *ki;
    KdPointerInfo *pi;

    kdInputEnabled = TRUE;

    for (ki = kdKeyboards; ki; ki = ki->next) {
        if (ki->driver && ki->driver->Enable)
            (*ki->driver->Enable) (ki);
    }

    for (pi = kdPointers; pi; pi = pi->next) {
        if (pi->driver && pi->driver->Enable)
            (*pi->driver->Enable) (pi);
    }

    /* reset screen saver */
    ev.any.time = GetTimeInMillis ();
    NoticeEventTime (&ev);

    KdUnblockSigio ();
}

static KdKeyboardDriver *
KdFindKeyboardDriver (char *name)
{
    KdKeyboardDriver *ret;

    /* ask a stupid question ... */
    if (!name)
        return NULL;

    for (ret = kdKeyboardDrivers; ret; ret = ret->next) {
        if (strcmp(ret->name, name) == 0)
            return ret;
    }

    return NULL;
}

static KdPointerDriver *
KdFindPointerDriver (char *name)
{
    KdPointerDriver *ret;

    /* ask a stupid question ... */
    if (!name)
        return NULL;

    for (ret = kdPointerDrivers; ret; ret = ret->next) {
        if (strcmp(ret->name, name) == 0)
            return ret;
    }

    return NULL;
}

static int
KdPointerProc(DeviceIntPtr pDevice, int onoff)
{
    DevicePtr       pDev = (DevicePtr)pDevice;
    KdPointerInfo   *pi;
    Atom            xiclass;
    Atom            *btn_labels;
    Atom            *axes_labels;

    if (!pDev)
	return BadImplementation;

    for (pi = kdPointers; pi; pi = pi->next) {
        if (pi->dixdev && pi->dixdev->id == pDevice->id)
            break;
    }

    if (!pi || !pi->dixdev || pi->dixdev->id != pDevice->id) {
        ErrorF("[KdPointerProc] Failed to find pointer for device %d!\n",
               pDevice->id);
        return BadImplementation;
    }

    switch (onoff)
    {
    case DEVICE_INIT:
#ifdef DEBUG
        ErrorF("initialising pointer %s ...\n", pi->name);
#endif
        if (!pi->driver) {
            if (!pi->driverPrivate) {
                ErrorF("no driver specified for %s\n", pi->name);
                return BadImplementation;
            }

            pi->driver = KdFindPointerDriver(pi->driverPrivate);
            if (!pi->driver) {
                ErrorF("Couldn't find pointer driver %s\n",
                       pi->driverPrivate ? (char *) pi->driverPrivate :
                       "(unnamed)");
                return !Success;
            }
            free(pi->driverPrivate);
            pi->driverPrivate = NULL;
        }

        if (!pi->driver->Init) {
            ErrorF("no init function\n");
            return BadImplementation;
        }

        if ((*pi->driver->Init) (pi) != Success) {
            return !Success;
        }

	btn_labels = calloc(pi->nButtons, sizeof(Atom));
	if (!btn_labels)
	    return BadAlloc;
	axes_labels = calloc(pi->nAxes, sizeof(Atom));
	if (!axes_labels) {
	    free(btn_labels);
	    return BadAlloc;
	}

	switch(pi->nAxes)
	{
	    default:
	    case 7:
		btn_labels[6] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_HWHEEL_RIGHT);
	    case 6:
		btn_labels[5] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_HWHEEL_LEFT);
	    case 5:
		btn_labels[4] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_DOWN);
	    case 4:
		btn_labels[3] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_WHEEL_UP);
	    case 3:
		btn_labels[2] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_RIGHT);
	    case 2:
		btn_labels[1] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_MIDDLE);
	    case 1:
		btn_labels[0] = XIGetKnownProperty(BTN_LABEL_PROP_BTN_LEFT);
	    case 0:
		break;
	}

	if (pi->nAxes >= 2) {
	    axes_labels[0] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_X);
	    axes_labels[1] = XIGetKnownProperty(AXIS_LABEL_PROP_REL_Y);
	}

	InitPointerDeviceStruct(pDev, pi->map, pi->nButtons, btn_labels,
	    (PtrCtrlProcPtr)NoopDDA,
	    GetMotionHistorySize(), pi->nAxes, axes_labels);

        free(btn_labels);
        free(axes_labels);

        if (pi->inputClass == KD_TOUCHSCREEN) {
            xiclass = AtomFromName(XI_TOUCHSCREEN);
        }
        else {
            xiclass = AtomFromName(XI_MOUSE);
        }

        AssignTypeAndName(pi->dixdev, xiclass,
                          pi->name ? pi->name : "Generic KDrive Pointer");

	return Success;

    case DEVICE_ON:
        if (pDev->on == TRUE)
            return Success;

        if (!pi->driver->Enable) {
            ErrorF("no enable function\n");
            return BadImplementation;
        }

        if ((*pi->driver->Enable) (pi) == Success) {
            pDev->on = TRUE;
            return Success;
        }
        else {
            return BadImplementation;
        }

	return Success;

    case DEVICE_OFF:
        if (pDev->on == FALSE) {
            return Success;
        }

        if (!pi->driver->Disable) {
            return BadImplementation;
        }
        else {
            (*pi->driver->Disable) (pi);
            pDev->on = FALSE;
            return Success;
        }

        return Success;

    case DEVICE_CLOSE:
	if (pDev->on) {
            if (!pi->driver->Disable) {
                return BadImplementation;
            }
            (*pi->driver->Disable) (pi);
            pDev->on = FALSE;
        }

        if (!pi->driver->Fini)
            return BadImplementation;

        (*pi->driver->Fini) (pi);

        KdRemovePointer(pi);

        return Success;
    }

    /* NOTREACHED */
    return BadImplementation;
}

Bool
LegalModifier(unsigned int key, DeviceIntPtr pDev)
{
    return TRUE;
}

static void
KdBell (int volume, DeviceIntPtr pDev, pointer arg, int something)
{
    KeybdCtrl *ctrl = arg;
    KdKeyboardInfo *ki = NULL;

    for (ki = kdKeyboards; ki; ki = ki->next) {
        if (ki->dixdev && ki->dixdev->id == pDev->id)
            break;
    }

    if (!ki || !ki->dixdev || ki->dixdev->id != pDev->id || !ki->driver)
        return;

    KdRingBell(ki, volume, ctrl->bell_pitch, ctrl->bell_duration);
}

void
DDXRingBell(int volume, int pitch, int duration)
{
    KdKeyboardInfo *ki = NULL;

    if (kdOsFuncs->Bell) {
        (*kdOsFuncs->Bell)(volume, pitch, duration);
    }
    else {
        for (ki = kdKeyboards; ki; ki = ki->next) {
            if (ki->dixdev->coreEvents)
                KdRingBell(ki, volume, pitch, duration);
        }
    }
}

void
KdRingBell(KdKeyboardInfo *ki, int volume, int pitch, int duration)
{
    if (!ki || !ki->driver || !ki->driver->Bell)
        return;

    if (kdInputEnabled)
        (*ki->driver->Bell) (ki, volume, pitch, duration);
}


static void
KdSetLeds (KdKeyboardInfo *ki, int leds)
{
    if (!ki || !ki->driver)
        return;

    if (kdInputEnabled) {
        if (ki->driver->Leds)
            (*ki->driver->Leds) (ki, leds);
    }
}

void
KdSetLed (KdKeyboardInfo *ki, int led, Bool on)
{
    if (!ki || !ki->dixdev || !ki->dixdev->kbdfeed)
        return;

    NoteLedState (ki->dixdev, led, on);
    KdSetLeds (ki, ki->dixdev->kbdfeed->ctrl.leds);
}

void
KdSetPointerMatrix (KdPointerMatrix *matrix)
{
    kdPointerMatrix = *matrix;
}

void
KdComputePointerMatrix (KdPointerMatrix *m, Rotation randr, int width,
                        int height)
{
    int		    x_dir = 1, y_dir = 1;
    int		    i, j;
    int		    size[2];

    size[0] = width; size[1] = height;
    if (randr & RR_Reflect_X)
	x_dir = -1;
    if (randr & RR_Reflect_Y)
	y_dir = -1;
    switch (randr & (RR_Rotate_All)) {
    case RR_Rotate_0:
	m->matrix[0][0] = x_dir; m->matrix[0][1] = 0;
	m->matrix[1][0] = 0; m->matrix[1][1] = y_dir;
	break;
    case RR_Rotate_90:
	m->matrix[0][0] = 0; m->matrix[0][1] = -x_dir;
	m->matrix[1][0] = y_dir; m->matrix[1][1] = 0;
	break;
    case RR_Rotate_180:
	m->matrix[0][0] = -x_dir; m->matrix[0][1] = 0;
	m->matrix[1][0] = 0; m->matrix[1][1] = -y_dir;
	break;
    case RR_Rotate_270:
	m->matrix[0][0] = 0; m->matrix[0][1] = x_dir;
	m->matrix[1][0] = -y_dir; m->matrix[1][1] = 0;
	break;
    }
    for (i = 0; i < 2; i++)
    {
	m->matrix[i][2] = 0;
	for (j = 0 ; j < 2; j++)
	    if (m->matrix[i][j] < 0)
		m->matrix[i][2] = size[j] - 1;
    }
}

void
KdScreenToPointerCoords (int *x, int *y)
{
    int	(*m)[3] = kdPointerMatrix.matrix;
    int div = m[0][1] * m[1][0] - m[1][1] * m[0][0];
    int sx = *x;
    int sy = *y;

    *x = (m[0][1] * sy - m[0][1] * m[1][2] + m[1][1] * m[0][2] - m[1][1] * sx) / div;
    *y = (m[1][0] * sx + m[0][0] * m[1][2] - m[1][0] * m[0][2] - m[0][0] * sy) / div;
}

static void
KdKbdCtrl (DeviceIntPtr pDevice, KeybdCtrl *ctrl)
{
    KdKeyboardInfo *ki;

    for (ki = kdKeyboards; ki; ki = ki->next) {
        if (ki->dixdev && ki->dixdev->id == pDevice->id)
            break;
    }

    if (!ki || !ki->dixdev || ki->dixdev->id != pDevice->id || !ki->driver)
        return;

    KdSetLeds(ki, ctrl->leds);
    ki->bellPitch = ctrl->bell_pitch;
    ki->bellDuration = ctrl->bell_duration;
}

extern KeybdCtrl defaultKeyboardControl;

static int
KdKeyboardProc(DeviceIntPtr pDevice, int onoff)
{
    Bool        ret;
    DevicePtr   pDev = (DevicePtr)pDevice;
    KdKeyboardInfo *ki;
    Atom xiclass;
    XkbRMLVOSet rmlvo;

    if (!pDev)
	return BadImplementation;

    for (ki = kdKeyboards; ki; ki = ki->next) {
        if (ki->dixdev && ki->dixdev->id == pDevice->id)
            break;
    }

    if (!ki || !ki->dixdev || ki->dixdev->id != pDevice->id) {
        return BadImplementation;
    }

    switch (onoff)
    {
    case DEVICE_INIT:
#ifdef DEBUG
        ErrorF("initialising keyboard %s\n", ki->name);
#endif
        if (!ki->driver) {
            if (!ki->driverPrivate) {
                ErrorF("no driver specified!\n");
                return BadImplementation;
            }

            ki->driver = KdFindKeyboardDriver(ki->driverPrivate);
            if (!ki->driver) {
                ErrorF("Couldn't find keyboard driver %s\n",
                       ki->driverPrivate ? (char *) ki->driverPrivate :
                       "(unnamed)");
                return !Success;
            }
            free(ki->driverPrivate);
            ki->driverPrivate = NULL;
        }

        if (!ki->driver->Init) {
            ErrorF("Keyboard %s: no init function\n", ki->name);
            return BadImplementation;
        }

        if ((*ki->driver->Init) (ki) != Success) {
            return !Success;
        }

        memset(&rmlvo, 0, sizeof(rmlvo));
        rmlvo.rules = ki->xkbRules;
        rmlvo.model = ki->xkbModel;
        rmlvo.layout = ki->xkbLayout;
        rmlvo.variant = ki->xkbVariant;
        rmlvo.options = ki->xkbOptions;
        ret = InitKeyboardDeviceStruct (pDevice, &rmlvo, KdBell, KdKbdCtrl);
	if (!ret) {
            ErrorF("Couldn't initialise keyboard %s\n", ki->name);
	    return BadImplementation;
        }

        xiclass = AtomFromName(XI_KEYBOARD);
        AssignTypeAndName(pDevice, xiclass,
                          ki->name ? ki->name : "Generic KDrive Keyboard");

        KdResetInputMachine();

        return Success;

    case DEVICE_ON:
        if (pDev->on == TRUE)
            return Success;

        if (!ki->driver->Enable)
            return BadImplementation;

        if ((*ki->driver->Enable) (ki) != Success) {
            return BadMatch;
        }

        pDev->on = TRUE;
        return Success;

    case DEVICE_OFF:
        if (pDev->on == FALSE)
            return Success;

        if (!ki->driver->Disable)
            return BadImplementation;

        (*ki->driver->Disable) (ki);
        pDev->on = FALSE;

        return Success;

        break;

    case DEVICE_CLOSE:
	if (pDev->on) {
            if (!ki->driver->Disable)
                return BadImplementation;

            (*ki->driver->Disable) (ki);
            pDev->on = FALSE;
	}

        if (!ki->driver->Fini)
            return BadImplementation;

        (*ki->driver->Fini) (ki);

        KdRemoveKeyboard(ki);

        return Success;
    }

    /* NOTREACHED */
    return BadImplementation;
}

void
KdAddPointerDriver (KdPointerDriver *driver)
{
    KdPointerDriver **prev;

    if (!driver)
        return;

    for (prev = &kdPointerDrivers; *prev; prev = &(*prev)->next) {
        if (*prev == driver)
            return;
    }
    *prev = driver;
}

void
KdRemovePointerDriver (KdPointerDriver *driver)
{
    KdPointerDriver *tmp;

    if (!driver)
        return;

    /* FIXME remove all pointers using this driver */
    for (tmp = kdPointerDrivers; tmp; tmp = tmp->next) {
        if (tmp->next == driver)
            tmp->next = driver->next;
    }
    if (tmp == driver)
        tmp = NULL;
}

void
KdAddKeyboardDriver (KdKeyboardDriver *driver)
{
    KdKeyboardDriver **prev;

    if (!driver)
        return;

    for (prev = &kdKeyboardDrivers; *prev; prev = &(*prev)->next) {
        if (*prev == driver)
            return;
    }
    *prev = driver;
}

void
KdRemoveKeyboardDriver (KdKeyboardDriver *driver)
{
    KdKeyboardDriver *tmp;

    if (!driver)
        return;

    /* FIXME remove all keyboards using this driver */
    for (tmp = kdKeyboardDrivers; tmp; tmp = tmp->next) {
        if (tmp->next == driver)
            tmp->next = driver->next;
    }
    if (tmp == driver)
        tmp = NULL;
}

KdKeyboardInfo *
KdNewKeyboard (void)
{
    KdKeyboardInfo *ki = calloc(sizeof(KdKeyboardInfo), 1);
    if (!ki)
        return NULL;

    ki->minScanCode = 0;
    ki->maxScanCode = 0;
    ki->leds = 0;
    ki->bellPitch = 1000;
    ki->bellDuration = 200;
    ki->next = NULL;
    ki->options = NULL;
    ki->xkbRules = strdup(XKB_DFLT_RULES);
    ki->xkbModel = strdup(XKB_DFLT_MODEL);
    ki->xkbLayout = strdup(XKB_DFLT_LAYOUT);
    ki->xkbVariant = strdup(XKB_DFLT_VARIANT);
    ki->xkbOptions = strdup(XKB_DFLT_OPTIONS);

    return ki;
}

int
KdAddConfigKeyboard (char *keyboard)
{
    struct KdConfigDevice **prev, *new;

    if (!keyboard)
        return Success;

    new = (struct KdConfigDevice *) calloc(sizeof(struct KdConfigDevice), 1);
    if (!new)
        return BadAlloc;

    new->line = strdup(keyboard);
    new->next = NULL;

    for (prev = &kdConfigKeyboards; *prev; prev = &(*prev)->next);
    *prev = new;

    return Success;
}

int
KdAddKeyboard (KdKeyboardInfo *ki)
{
    KdKeyboardInfo **prev;

    if (!ki)
        return !Success;

    ki->dixdev = AddInputDevice(serverClient, KdKeyboardProc, TRUE);
    if (!ki->dixdev) {
        ErrorF("Couldn't register keyboard device %s\n",
               ki->name ? ki->name : "(unnamed)");
        return !Success;
    }

#ifdef DEBUG
    ErrorF("added keyboard %s with dix id %d\n", ki->name, ki->dixdev->id);
#endif

    for (prev = &kdKeyboards; *prev; prev = &(*prev)->next);
    *prev = ki;

    return Success;
}

void
KdRemoveKeyboard (KdKeyboardInfo *ki)
{
    KdKeyboardInfo **prev;

    if (!ki)
        return;

    for (prev = &kdKeyboards; *prev; prev = &(*prev)->next) {
        if (*prev == ki) {
            *prev = ki->next;
            break;
        }
    }

    KdFreeKeyboard(ki);
}

int
KdAddConfigPointer (char *pointer)
{
    struct KdConfigDevice **prev, *new;

    if (!pointer)
        return Success;

    new = (struct KdConfigDevice *) calloc(sizeof(struct KdConfigDevice), 1);
    if (!new)
        return BadAlloc;

    new->line = strdup(pointer);
    new->next = NULL;

    for (prev = &kdConfigPointers; *prev; prev = &(*prev)->next);
    *prev = new;

    return Success;
}

int
KdAddPointer (KdPointerInfo *pi)
{
    KdPointerInfo **prev;

    if (!pi)
        return Success;

    pi->mouseState = start;
    pi->eventHeld = FALSE;

    pi->dixdev = AddInputDevice(serverClient, KdPointerProc, TRUE);
    if (!pi->dixdev) {
        ErrorF("Couldn't add pointer device %s\n",
               pi->name ? pi->name : "(unnamed)");
        return BadDevice;
    }

    for (prev = &kdPointers; *prev; prev = &(*prev)->next);
    *prev = pi;

    return Success;
}

void
KdRemovePointer (KdPointerInfo *pi)
{
    KdPointerInfo **prev;

    if (!pi)
        return;

    for (prev = &kdPointers; *prev; prev = &(*prev)->next) {
        if (*prev == pi) {
            *prev = pi->next;
            break;
        }
    }

    KdFreePointer(pi);
}

/*
 * You can call your kdriver server with something like:
 * $ ./hw/kdrive/yourserver/X :1 -mouse evdev,,device=/dev/input/event4 -keybd
 * evdev,,device=/dev/input/event1,xkbmodel=abnt2,xkblayout=br
 */
static Bool
KdGetOptions (InputOption **options, char *string)
{
    InputOption     *newopt = NULL, **tmpo = NULL;
    int             tam_key = 0;

    newopt = calloc(1, sizeof (InputOption));
    if (!newopt)
        return FALSE;

    for (tmpo = options; *tmpo; tmpo = &(*tmpo)->next)
        ; /* Hello, I'm here */
    *tmpo = newopt;

    if (strchr(string, '='))
    {
        tam_key = (strchr(string, '=') - string);
        newopt->key = (char *)malloc(tam_key);
        strncpy(newopt->key, string, tam_key);
        newopt->key[tam_key] = '\0';
        newopt->value = strdup(strchr(string, '=') + 1);
    }
    else
    {
        newopt->key = strdup(string);
        newopt->value = NULL;
    }
    newopt->next = NULL;

    return TRUE;
}

static void
KdParseKbdOptions (KdKeyboardInfo *ki)
{
    InputOption *option = NULL;

    for (option = ki->options; option; option = option->next)
    {
        if (strcasecmp(option->key, "XkbRules") == 0)
            ki->xkbRules = option->value;
        else if (strcasecmp(option->key, "XkbModel") == 0)
            ki->xkbModel = option->value;
        else if (strcasecmp(option->key, "XkbLayout") == 0)
            ki->xkbLayout = option->value;
        else if (strcasecmp(option->key, "XkbVariant") == 0)
            ki->xkbVariant = option->value;
        else if (strcasecmp(option->key, "XkbOptions") == 0)
            ki->xkbOptions = option->value;
        else if (!strcasecmp (option->key, "device"))
            ki->path = strdup(option->value);
        else
           ErrorF("Kbd option key (%s) of value (%s) not assigned!\n",
                    option->key, option->value);
    }
}

KdKeyboardInfo *
KdParseKeyboard (char *arg)
{
    char            save[1024];
    char            delim;
    InputOption     *options = NULL;
    KdKeyboardInfo     *ki = NULL;

    ki = KdNewKeyboard();
    if (!ki)
        return NULL;

    ki->name = strdup("Unknown KDrive Keyboard");
    ki->path = NULL;
    ki->driver = NULL;
    ki->driverPrivate = NULL;
    ki->next = NULL;

    if (!arg)
    {
        ErrorF("keybd: no arg\n");
        KdFreeKeyboard (ki);
        return NULL;
    }

    if (strlen (arg) >= sizeof (save))
    {
        ErrorF("keybd: arg too long\n");
        KdFreeKeyboard (ki);
        return NULL;
    }

    arg = KdParseFindNext (arg, ",", save, &delim);
    if (!save[0])
    {
        ErrorF("keybd: failed on save[0]\n");
        KdFreeKeyboard (ki);
        return NULL;
    }

    if (strcmp (save, "auto") == 0)
        ki->driverPrivate = NULL;
    else
        ki->driverPrivate = strdup(save);

    if (delim != ',')
    {
        return ki;
    }

    arg = KdParseFindNext (arg, ",", save, &delim);

    while (delim == ',')
    {
        arg = KdParseFindNext (arg, ",", save, &delim);

	if (!KdGetOptions(&options, save))
	{
	    KdFreeKeyboard(ki);
	    return NULL;
        }
    }

    if (options)
    {
        ki->options = options;
        KdParseKbdOptions(ki);
    }

    return ki;
}

static void
KdParsePointerOptions (KdPointerInfo *pi)
{
    InputOption *option = NULL;

    for (option = pi->options; option; option = option->next)
    {
        if (!strcmp (option->key, "emulatemiddle"))
            pi->emulateMiddleButton = TRUE;
        else if (!strcmp (option->key, "noemulatemiddle"))
            pi->emulateMiddleButton = FALSE;
        else if (!strcmp (option->key, "transformcoord"))
            pi->transformCoordinates = TRUE;
        else if (!strcmp (option->key, "rawcoord"))
            pi->transformCoordinates = FALSE;
        else if (!strcasecmp (option->key, "device"))
            pi->path = strdup(option->value);
        else if (!strcasecmp (option->key, "protocol"))
            pi->protocol = strdup(option->value);
        else
            ErrorF("Pointer option key (%s) of value (%s) not assigned!\n",
                    option->key, option->value);
    }
}

KdPointerInfo *
KdParsePointer (char *arg)
{
    char            save[1024];
    char            delim;
    KdPointerInfo   *pi = NULL;
    InputOption     *options = NULL;
    int             i = 0;

    pi = KdNewPointer();
    if (!pi)
        return NULL;
    pi->emulateMiddleButton = kdEmulateMiddleButton;
    pi->transformCoordinates = !kdRawPointerCoordinates;
    pi->protocol = NULL;
    pi->nButtons = 5; /* XXX should not be hardcoded */
    pi->inputClass = KD_MOUSE;

    if (!arg)
    {
        ErrorF("mouse: no arg\n");
        KdFreePointer (pi);
        return NULL;
    }

    if (strlen (arg) >= sizeof (save))
    {
        ErrorF("mouse: arg too long\n");
        KdFreePointer (pi);
        return NULL;
    }
    arg = KdParseFindNext (arg, ",", save, &delim);
    if (!save[0])
    {
        ErrorF("failed on save[0]\n");
        KdFreePointer (pi);
        return NULL;
    }

    if (strcmp(save, "auto") == 0)
        pi->driverPrivate = NULL;
    else
        pi->driverPrivate = strdup(save);

    if (delim != ',')
    {
        return pi;
    }

    arg = KdParseFindNext (arg, ",", save, &delim);

    while (delim == ',')
    {
        arg = KdParseFindNext (arg, ",", save, &delim);
        if (save[0] == '{')
        {
            char *s = save + 1;
             i = 0;
             while (*s && *s != '}')
             {
                if ('1' <= *s && *s <= '0' + pi->nButtons)
                    pi->map[i] = *s - '0';
                else
                    UseMsg ();
                s++;
             }
        }
        else
        {
            if (!KdGetOptions(&options, save))
            {
                KdFreePointer(pi);
                return NULL;
            }
        }
    }

    if (options)
    {
        pi->options = options;
        KdParsePointerOptions(pi);
    }

    return pi;
}


void
KdInitInput (void)
{
    KdPointerInfo *pi;
    KdKeyboardInfo *ki;
    struct KdConfigDevice *dev;

    kdInputEnabled = TRUE;

    for (dev = kdConfigPointers; dev; dev = dev->next) {
        pi = KdParsePointer(dev->line);
        if (!pi)
            ErrorF("Failed to parse pointer\n");
        if (KdAddPointer(pi) != Success)
            ErrorF("Failed to add pointer!\n");
    }
    for (dev = kdConfigKeyboards; dev; dev = dev->next) {
        ki = KdParseKeyboard(dev->line);
        if (!ki)
            ErrorF("Failed to parse keyboard\n");
        if (KdAddKeyboard(ki) != Success)
            ErrorF("Failed to add keyboard!\n");
    }

    mieqInit();
}

void
KdCloseInput (void)
{
    mieqFini();
}

/*
 * Middle button emulation state machine
 *
 *  Possible transitions:
 *	Button 1 press	    v1
 *	Button 1 release    ^1
 *	Button 2 press	    v2
 *	Button 2 release    ^2
 *	Button 3 press	    v3
 *	Button 3 release    ^3
 *	Button other press  vo
 *	Button other release ^o
 *	Mouse motion	    <>
 *	Keyboard event	    k
 *	timeout		    ...
 *	outside box	    <->
 *
 *  States:
 *	start
 *	button_1_pend
 *	button_1_down
 *	button_2_down
 *	button_3_pend
 *	button_3_down
 *	synthetic_2_down_13
 *	synthetic_2_down_3
 *	synthetic_2_down_1
 *
 *  Transition diagram
 *
 *  start
 *	v1  -> (hold) (settimeout) button_1_pend
 *	^1  -> (deliver) start
 *	v2  -> (deliver) button_2_down
 *	^2  -> (deliever) start
 *	v3  -> (hold) (settimeout) button_3_pend
 *	^3  -> (deliver) start
 *	vo  -> (deliver) start
 *	^o  -> (deliver) start
 *	<>  -> (deliver) start
 *	k   -> (deliver) start
 *
 *  button_1_pend	(button 1 is down, timeout pending)
 *	^1  -> (release) (deliver) start
 *	v2  -> (release) (deliver) button_1_down
 *	^2  -> (release) (deliver) button_1_down
 *	v3  -> (cleartimeout) (generate v2) synthetic_2_down_13
 *	^3  -> (release) (deliver) button_1_down
 *	vo  -> (release) (deliver) button_1_down
 *	^o  -> (release) (deliver) button_1_down
 *	<-> -> (release) (deliver) button_1_down
 *	<>  -> (deliver) button_1_pend
 *	k   -> (release) (deliver) button_1_down
 *	... -> (release) button_1_down
 *
 *  button_1_down	(button 1 is down)
 *	^1  -> (deliver) start
 *	v2  -> (deliver) button_1_down
 *	^2  -> (deliver) button_1_down
 *	v3  -> (deliver) button_1_down
 *	^3  -> (deliver) button_1_down
 *	vo  -> (deliver) button_1_down
 *	^o  -> (deliver) button_1_down
 *	<>  -> (deliver) button_1_down
 *	k   -> (deliver) button_1_down
 *
 *  button_2_down	(button 2 is down)
 *	v1  -> (deliver) button_2_down
 *	^1  -> (deliver) button_2_down
 *	^2  -> (deliver) start
 *	v3  -> (deliver) button_2_down
 *	^3  -> (deliver) button_2_down
 *	vo  -> (deliver) button_2_down
 *	^o  -> (deliver) button_2_down
 *	<>  -> (deliver) button_2_down
 *	k   -> (deliver) button_2_down
 *
 *  button_3_pend	(button 3 is down, timeout pending)
 *	v1  -> (generate v2) synthetic_2_down
 *	^1  -> (release) (deliver) button_3_down
 *	v2  -> (release) (deliver) button_3_down
 *	^2  -> (release) (deliver) button_3_down
 *	^3  -> (release) (deliver) start
 *	vo  -> (release) (deliver) button_3_down
 *	^o  -> (release) (deliver) button_3_down
 *	<-> -> (release) (deliver) button_3_down
 *	<>  -> (deliver) button_3_pend
 *	k   -> (release) (deliver) button_3_down
 *	... -> (release) button_3_down
 *
 *  button_3_down	(button 3 is down)
 *	v1  -> (deliver) button_3_down
 *	^1  -> (deliver) button_3_down
 *	v2  -> (deliver) button_3_down
 *	^2  -> (deliver) button_3_down
 *	^3  -> (deliver) start
 *	vo  -> (deliver) button_3_down
 *	^o  -> (deliver) button_3_down
 *	<>  -> (deliver) button_3_down
 *	k   -> (deliver) button_3_down
 *
 *  synthetic_2_down_13	(button 1 and 3 are down)
 *	^1  -> (generate ^2) synthetic_2_down_3
 *	v2  -> synthetic_2_down_13
 *	^2  -> synthetic_2_down_13
 *	^3  -> (generate ^2) synthetic_2_down_1
 *	vo  -> (deliver) synthetic_2_down_13
 *	^o  -> (deliver) synthetic_2_down_13
 *	<>  -> (deliver) synthetic_2_down_13
 *	k   -> (deliver) synthetic_2_down_13
 *
 *  synthetic_2_down_3 (button 3 is down)
 *	v1  -> (deliver) synthetic_2_down_3
 *	^1  -> (deliver) synthetic_2_down_3
 *	v2  -> synthetic_2_down_3
 *	^2  -> synthetic_2_down_3
 *	^3  -> start
 *	vo  -> (deliver) synthetic_2_down_3
 *	^o  -> (deliver) synthetic_2_down_3
 *	<>  -> (deliver) synthetic_2_down_3
 *	k   -> (deliver) synthetic_2_down_3
 *
 *  synthetic_2_down_1 (button 1 is down)
 *	^1  -> start
 *	v2  -> synthetic_2_down_1
 *	^2  -> synthetic_2_down_1
 *	v3  -> (deliver) synthetic_2_down_1
 *	^3  -> (deliver) synthetic_2_down_1
 *	vo  -> (deliver) synthetic_2_down_1
 *	^o  -> (deliver) synthetic_2_down_1
 *	<>  -> (deliver) synthetic_2_down_1
 *	k   -> (deliver) synthetic_2_down_1
 */

typedef enum _inputClass {
    down_1, up_1,
    down_2, up_2,
    down_3, up_3,
    down_o, up_o,
    motion, outside_box,
    keyboard, timeout,
    num_input_class
} KdInputClass;

typedef enum _inputAction {
    noop,
    hold,
    setto,
    deliver,
    release,
    clearto,
    gen_down_2,
    gen_up_2
} KdInputAction;

#define MAX_ACTIONS 2

typedef struct _inputTransition {
    KdInputAction  actions[MAX_ACTIONS];
    KdPointerState nextState;
} KdInputTransition;

static const
KdInputTransition  kdInputMachine[num_input_states][num_input_class] = {
    /* start */
    {
	{ { hold, setto },	    button_1_pend },	/* v1 */
	{ { deliver, noop },	    start },		/* ^1 */
	{ { deliver, noop },	    button_2_down },	/* v2 */
	{ { deliver, noop },	    start },		/* ^2 */
	{ { hold, setto },	    button_3_pend },	/* v3 */
	{ { deliver, noop },	    start },		/* ^3 */
	{ { deliver, noop },	    start },		/* vo */
	{ { deliver, noop },	    start },		/* ^o */
	{ { deliver, noop },	    start },		/* <> */
	{ { deliver, noop },	    start },		/* <-> */
	{ { noop, noop },	    start },		/* k */
	{ { noop, noop },	    start },		/* ... */
    },
    /* button_1_pend */
    {
	{ { noop, noop },	    button_1_pend },	/* v1 */
	{ { release, deliver },	    start },		/* ^1 */
	{ { release, deliver },	    button_1_down },	/* v2 */
	{ { release, deliver },	    button_1_down },	/* ^2 */
	{ { clearto, gen_down_2 },  synth_2_down_13 },	/* v3 */
	{ { release, deliver },	    button_1_down },	/* ^3 */
	{ { release, deliver },	    button_1_down },	/* vo */
	{ { release, deliver },	    button_1_down },	/* ^o */
	{ { deliver, noop },	    button_1_pend },	/* <> */
	{ { release, deliver },	    button_1_down },	/* <-> */
	{ { noop, noop },	    button_1_down },	/* k */
	{ { release, noop },	    button_1_down },	/* ... */
    },
    /* button_1_down */
    {
	{ { noop, noop },	    button_1_down },	/* v1 */
	{ { deliver, noop },	    start },		/* ^1 */
	{ { deliver, noop },	    button_1_down },	/* v2 */
	{ { deliver, noop },	    button_1_down },	/* ^2 */
	{ { deliver, noop },	    button_1_down },	/* v3 */
	{ { deliver, noop },	    button_1_down },	/* ^3 */
	{ { deliver, noop },	    button_1_down },	/* vo */
	{ { deliver, noop },	    button_1_down },	/* ^o */
	{ { deliver, noop },	    button_1_down },	/* <> */
	{ { deliver, noop },	    button_1_down },	/* <-> */
	{ { noop, noop },	    button_1_down },	/* k */
	{ { noop, noop },	    button_1_down },	/* ... */
    },
    /* button_2_down */
    {
	{ { deliver, noop },	    button_2_down },	/* v1 */
	{ { deliver, noop },	    button_2_down },	/* ^1 */
	{ { noop, noop },	    button_2_down },	/* v2 */
	{ { deliver, noop },	    start },		/* ^2 */
	{ { deliver, noop },	    button_2_down },	/* v3 */
	{ { deliver, noop },	    button_2_down },	/* ^3 */
	{ { deliver, noop },	    button_2_down },	/* vo */
	{ { deliver, noop },	    button_2_down },	/* ^o */
	{ { deliver, noop },	    button_2_down },	/* <> */
	{ { deliver, noop },	    button_2_down },	/* <-> */
	{ { noop, noop },	    button_2_down },	/* k */
	{ { noop, noop },	    button_2_down },	/* ... */
    },
    /* button_3_pend */
    {
	{ { clearto, gen_down_2 },  synth_2_down_13 },	/* v1 */
	{ { release, deliver },	    button_3_down },	/* ^1 */
	{ { release, deliver },	    button_3_down },	/* v2 */
	{ { release, deliver },	    button_3_down },	/* ^2 */
	{ { release, deliver },	    button_3_down },	/* v3 */
	{ { release, deliver },	    start },		/* ^3 */
	{ { release, deliver },	    button_3_down },	/* vo */
	{ { release, deliver },	    button_3_down },	/* ^o */
	{ { deliver, noop },	    button_3_pend },	/* <> */
	{ { release, deliver },	    button_3_down },	/* <-> */
	{ { release, noop },	    button_3_down },	/* k */
	{ { release, noop },	    button_3_down },	/* ... */
    },
    /* button_3_down */
    {
	{ { deliver, noop },	    button_3_down },	/* v1 */
	{ { deliver, noop },	    button_3_down },	/* ^1 */
	{ { deliver, noop },	    button_3_down },	/* v2 */
	{ { deliver, noop },	    button_3_down },	/* ^2 */
	{ { noop, noop },	    button_3_down },	/* v3 */
	{ { deliver, noop },	    start },		/* ^3 */
	{ { deliver, noop },	    button_3_down },	/* vo */
	{ { deliver, noop },	    button_3_down },	/* ^o */
	{ { deliver, noop },	    button_3_down },	/* <> */
	{ { deliver, noop },	    button_3_down },	/* <-> */
	{ { noop, noop },	    button_3_down },	/* k */
	{ { noop, noop },	    button_3_down },	/* ... */
    },
    /* synthetic_2_down_13 */
    {
	{ { noop, noop },	    synth_2_down_13 },	/* v1 */
	{ { gen_up_2, noop },	    synth_2_down_3 },	/* ^1 */
	{ { noop, noop },	    synth_2_down_13 },	/* v2 */
	{ { noop, noop },	    synth_2_down_13 },	/* ^2 */
	{ { noop, noop },	    synth_2_down_13 },	/* v3 */
	{ { gen_up_2, noop },	    synth_2_down_1 },	/* ^3 */
	{ { deliver, noop },	    synth_2_down_13 },	/* vo */
	{ { deliver, noop },	    synth_2_down_13 },	/* ^o */
	{ { deliver, noop },	    synth_2_down_13 },	/* <> */
	{ { deliver, noop },	    synth_2_down_13 },	/* <-> */
	{ { noop, noop },	    synth_2_down_13 },	/* k */
	{ { noop, noop },	    synth_2_down_13 },	/* ... */
    },
    /* synthetic_2_down_3 */
    {
	{ { deliver, noop },	    synth_2_down_3 },	/* v1 */
	{ { deliver, noop },	    synth_2_down_3 },	/* ^1 */
	{ { deliver, noop },	    synth_2_down_3 },	/* v2 */
	{ { deliver, noop },	    synth_2_down_3 },	/* ^2 */
	{ { noop, noop },	    synth_2_down_3 },	/* v3 */
	{ { noop, noop },	    start },		/* ^3 */
	{ { deliver, noop },	    synth_2_down_3 },	/* vo */
	{ { deliver, noop },	    synth_2_down_3 },	/* ^o */
	{ { deliver, noop },	    synth_2_down_3 },	/* <> */
	{ { deliver, noop },	    synth_2_down_3 },	/* <-> */
	{ { noop, noop },	    synth_2_down_3 },	/* k */
	{ { noop, noop },	    synth_2_down_3 },	/* ... */
    },
    /* synthetic_2_down_1 */
    {
	{ { noop, noop },	    synth_2_down_1 },	/* v1 */
	{ { noop, noop },	    start },		/* ^1 */
	{ { deliver, noop },	    synth_2_down_1 },	/* v2 */
	{ { deliver, noop },	    synth_2_down_1 },	/* ^2 */
	{ { deliver, noop },	    synth_2_down_1 },	/* v3 */
	{ { deliver, noop },	    synth_2_down_1 },	/* ^3 */
	{ { deliver, noop },	    synth_2_down_1 },	/* vo */
	{ { deliver, noop },	    synth_2_down_1 },	/* ^o */
	{ { deliver, noop },	    synth_2_down_1 },	/* <> */
	{ { deliver, noop },	    synth_2_down_1 },	/* <-> */
	{ { noop, noop },	    synth_2_down_1 },	/* k */
	{ { noop, noop },	    synth_2_down_1 },	/* ... */
    },
};

#define EMULATION_WINDOW    10
#define EMULATION_TIMEOUT   100

static int
KdInsideEmulationWindow (KdPointerInfo *pi, int x, int y, int z)
{
    pi->emulationDx = pi->heldEvent.x - x;
    pi->emulationDy = pi->heldEvent.y - y;

    return (abs (pi->emulationDx) < EMULATION_WINDOW &&
	    abs (pi->emulationDy) < EMULATION_WINDOW);
}

static KdInputClass
KdClassifyInput (KdPointerInfo *pi, int type, int x, int y, int z, int b)
{
    switch (type) {
    case ButtonPress:
	switch (b) {
	case 1: return down_1;
	case 2: return down_2;
	case 3: return down_3;
	default: return down_o;
	}
	break;
    case ButtonRelease:
	switch (b) {
	case 1: return up_1;
	case 2: return up_2;
	case 3: return up_3;
	default: return up_o;
	}
	break;
    case MotionNotify:
	if (pi->eventHeld && !KdInsideEmulationWindow(pi, x, y, z))
	    return outside_box;
	else
	    return motion;
    default:
	return keyboard;
    }
    return keyboard;
}

#ifdef DEBUG
char	*kdStateNames[] = {
    "start",
    "button_1_pend",
    "button_1_down",
    "button_2_down",
    "button_3_pend",
    "button_3_down",
    "synth_2_down_13",
    "synth_2_down_3",
    "synthetic_2_down_1",
    "num_input_states"
};

char	*kdClassNames[] = {
    "down_1", "up_1",
    "down_2", "up_2",
    "down_3", "up_3",
    "motion", "ouside_box",
    "keyboard", "timeout",
    "num_input_class"
};

char *kdActionNames[] = {
    "noop",
    "hold",
    "setto",
    "deliver",
    "release",
    "clearto",
    "gen_down_2",
    "gen_up_2",
};
#endif /* DEBUG */

static void
KdQueueEvent (DeviceIntPtr pDev, InternalEvent *ev)
{
    KdAssertSigioBlocked ("KdQueueEvent");
    mieqEnqueue (pDev, ev);
}

/* We return true if we're stealing the event. */
static Bool
KdRunMouseMachine (KdPointerInfo *pi, KdInputClass c, int type, int x, int y,
                   int z, int b, int absrel)
{
    const KdInputTransition *t;
    int	a;

    c = KdClassifyInput(pi, type, x, y, z, b);
    t = &kdInputMachine[pi->mouseState][c];
    for (a = 0; a < MAX_ACTIONS; a++)
    {
	switch (t->actions[a]) {
	case noop:
	    break;
	case hold:
	    pi->eventHeld = TRUE;
	    pi->emulationDx = 0;
	    pi->emulationDy = 0;
	    pi->heldEvent.type = type;
            pi->heldEvent.x = x;
            pi->heldEvent.y = y;
            pi->heldEvent.z = z;
            pi->heldEvent.flags = b;
            pi->heldEvent.absrel = absrel;
            return TRUE;
	    break;
	case setto:
	    pi->emulationTimeout = GetTimeInMillis () + EMULATION_TIMEOUT;
	    pi->timeoutPending = TRUE;
	    break;
	case deliver:
            _KdEnqueuePointerEvent (pi, pi->heldEvent.type, pi->heldEvent.x,
                                    pi->heldEvent.y, pi->heldEvent.z,
                                    pi->heldEvent.flags, pi->heldEvent.absrel,
                                    TRUE);
	    break;
	case release:
	    pi->eventHeld = FALSE;
	    pi->timeoutPending = FALSE;
            _KdEnqueuePointerEvent (pi, pi->heldEvent.type, pi->heldEvent.x,
                                    pi->heldEvent.y, pi->heldEvent.z,
                                    pi->heldEvent.flags, pi->heldEvent.absrel,
                                    TRUE);
            return TRUE;
	    break;
	case clearto:
	    pi->timeoutPending = FALSE;
	    break;
	case gen_down_2:
            _KdEnqueuePointerEvent (pi, ButtonPress, x, y, z, 2, absrel,
                                    TRUE);
	    pi->eventHeld = FALSE;
            return TRUE;
	    break;
	case gen_up_2:
            _KdEnqueuePointerEvent (pi, ButtonRelease, x, y, z, 2, absrel,
                                    TRUE);
            return TRUE;
	    break;
	}
    }
    pi->mouseState = t->nextState;
    return FALSE;
}

static int
KdHandlePointerEvent (KdPointerInfo *pi, int type, int x, int y, int z, int b,
                      int absrel)
{
    if (pi->emulateMiddleButton)
        return KdRunMouseMachine (pi, KdClassifyInput(pi, type, x, y, z, b),
                                  type, x, y, z, b, absrel);
    return FALSE;
}

static void
KdReceiveTimeout (KdPointerInfo *pi)
{
    KdRunMouseMachine (pi, timeout, 0, 0, 0, 0, 0, 0);
}

/*
 * kdCheckTermination
 *
 * This function checks for the key sequence that terminates the server.  When
 * detected, it sets the dispatchException flag and returns.  The key sequence
 * is:
 *	Control-Alt
 * It's assumed that the server will be waken up by the caller when this
 * function returns.
 */

extern int nClients;

void
KdReleaseAllKeys (void)
{
#if 0
    int	key;
    KdKeyboardInfo *ki;

    KdBlockSigio ();

    for (ki = kdKeyboards; ki; ki = ki->next) {
        for (key = ki->keySyms.minKeyCode; key < ki->keySyms.maxKeyCode;
             key++) {
            if (key_is_down(ki->dixdev, key, KEY_POSTED | KEY_PROCESSED)) {
                KdHandleKeyboardEvent(ki, KeyRelease, key);
                QueueGetKeyboardEvents(ki->dixdev, KeyRelease, key, NULL);
            }
        }
    }

    KdUnblockSigio ();
#endif
}

static void
KdCheckLock (void)
{
    KeyClassPtr	    keyc = NULL;
    Bool	    isSet = FALSE, shouldBeSet = FALSE;
    KdKeyboardInfo     *tmp = NULL;

    for (tmp = kdKeyboards; tmp; tmp = tmp->next) {
        if (tmp->LockLed && tmp->dixdev && tmp->dixdev->key) {
            keyc = tmp->dixdev->key;
            isSet = (tmp->leds & (1 << (tmp->LockLed-1))) != 0;
            /* FIXME: Just use XKB indicators! */
            shouldBeSet = !!(XkbStateFieldFromRec(&keyc->xkbInfo->state) & LockMask);
            if (isSet != shouldBeSet)
                KdSetLed (tmp, tmp->LockLed, shouldBeSet);
        }
    }
}

void
KdEnqueueKeyboardEvent(KdKeyboardInfo   *ki,
                       unsigned char scan_code,
		       unsigned char is_up)
{
    unsigned char key_code;
    KeyClassPtr	keyc = NULL;
    KeybdCtrl *ctrl = NULL;
    int type;

    if (!ki || !ki->dixdev || !ki->dixdev->kbdfeed || !ki->dixdev->key)
	return;

    keyc = ki->dixdev->key;
    ctrl = &ki->dixdev->kbdfeed->ctrl;

    if (scan_code >= ki->minScanCode && scan_code <= ki->maxScanCode)
    {
	key_code = scan_code + KD_MIN_KEYCODE - ki->minScanCode;

	/*
	 * Set up this event -- the type may be modified below
	 */
	if (is_up)
	    type = KeyRelease;
	else
	    type = KeyPress;

        QueueKeyboardEvents(ki->dixdev, type, key_code, NULL);
    }
    else {
        ErrorF("driver %s wanted to post scancode %d outside of [%d, %d]!\n",
               ki->name, scan_code, ki->minScanCode, ki->maxScanCode);
    }
}

/*
 * kdEnqueuePointerEvent
 *
 * This function converts hardware mouse event information into X event
 * information.  A mouse movement event is passed off to MI to generate
 * a MotionNotify event, if appropriate.  Button events are created and
 * passed off to MI for enqueueing.
 */

/* FIXME do something a little more clever to deal with multiple axes here */
void
KdEnqueuePointerEvent(KdPointerInfo *pi, unsigned long flags, int rx, int ry,
                      int rz)
{
    CARD32        ms;
    unsigned char buttons;
    int           x, y, z;
    int           (*matrix)[3] = kdPointerMatrix.matrix;
    unsigned long button;
    int           n;
    int           dixflags = 0;

    if (!pi)
	return;

    ms = GetTimeInMillis();

    /* we don't need to transform z, so we don't. */
    if (flags & KD_MOUSE_DELTA) {
	if (pi->transformCoordinates) {
	    x = matrix[0][0] * rx + matrix[0][1] * ry;
	    y = matrix[1][0] * rx + matrix[1][1] * ry;
	}
	else {
	    x = rx;
	    y = ry;
	}
    }
    else {
	if (pi->transformCoordinates) {
	    x = matrix[0][0] * rx + matrix[0][1] * ry + matrix[0][2];
	    y = matrix[1][0] * rx + matrix[1][1] * ry + matrix[1][2];
	}
	else {
	    x = rx;
	    y = ry;
	}
    }
    z = rz;

    if (flags & KD_MOUSE_DELTA)
    {
        if (x || y || z)
        {
            dixflags = POINTER_RELATIVE | POINTER_ACCELERATE;
            _KdEnqueuePointerEvent(pi, MotionNotify, x, y, z, 0, dixflags, FALSE);
        }
    } else
    {
        dixflags = POINTER_ABSOLUTE;
        if (x != pi->dixdev->last.valuators[0] ||
            y != pi->dixdev->last.valuators[1])
            _KdEnqueuePointerEvent(pi, MotionNotify, x, y, z, 0, dixflags, FALSE);
    }

    buttons = flags;

    for (button = KD_BUTTON_1, n = 1; n <= pi->nButtons;
         button <<= 1, n++) {
        if (((pi->buttonState & button) ^ (buttons & button)) &&
           !(buttons & button)) {
            _KdEnqueuePointerEvent(pi, ButtonRelease, x, y, z, n,
                                   dixflags, FALSE);
	}
    }
    for (button = KD_BUTTON_1, n = 1; n <= pi->nButtons;
         button <<= 1, n++) {
	if (((pi->buttonState & button) ^ (buttons & button)) &&
	    (buttons & button)) {
            _KdEnqueuePointerEvent(pi, ButtonPress, x, y, z, n,
                                   dixflags, FALSE);
        }
    }

    pi->buttonState = buttons;
}

void
_KdEnqueuePointerEvent (KdPointerInfo *pi, int type, int x, int y, int z,
                        int b, int absrel, Bool force)
{
    int valuators[3] = { x, y, z };
    ValuatorMask mask;

    /* TRUE from KdHandlePointerEvent, means 'we swallowed the event'. */
    if (!force && KdHandlePointerEvent(pi, type, x, y, z, b, absrel))
        return;

    valuator_mask_set_range(&mask, 0, 3, valuators);

    QueuePointerEvents(pi->dixdev, type, b, absrel, &mask);
}

void
KdBlockHandler (int		screen,
		pointer		blockData,
		pointer		timeout,
		pointer		readmask)
{
    KdPointerInfo		    *pi;
    int myTimeout=0;

    for (pi = kdPointers; pi; pi = pi->next)
    {
	if (pi->timeoutPending)
	{
	    int	ms;

	    ms = pi->emulationTimeout - GetTimeInMillis ();
	    if (ms < 1)
		ms = 1;
	    if(ms<myTimeout || myTimeout==0)
		    myTimeout=ms;
	}
    }
    /* if we need to poll for events, do that */
    if(kdOsFuncs->pollEvents)
    {
	    (*kdOsFuncs->pollEvents)();
	    myTimeout=20;
    }
    if(myTimeout>0)
    	AdjustWaitForDelay (timeout, myTimeout);
}

void
KdWakeupHandler (int		screen,
		 pointer    	data,
		 unsigned long	lresult,
		 pointer	readmask)
{
    int		result = (int) lresult;
    fd_set	*pReadmask = (fd_set *) readmask;
    int		i;
    KdPointerInfo	*pi;

    if (kdInputEnabled && result > 0)
    {
	for (i = 0; i < kdNumInputFds; i++)
	    if (FD_ISSET (kdInputFds[i].fd, pReadmask))
	    {
		KdBlockSigio ();
		(*kdInputFds[i].read) (kdInputFds[i].fd, kdInputFds[i].closure);
		KdUnblockSigio ();
	    }
    }
    for (pi = kdPointers; pi; pi = pi->next)
    {
	if (pi->timeoutPending)
	{
	    if ((long) (GetTimeInMillis () - pi->emulationTimeout) >= 0)
	    {
		pi->timeoutPending = FALSE;
		KdBlockSigio ();
		KdReceiveTimeout (pi);
		KdUnblockSigio ();
	    }
	}
    }
    if (kdSwitchPending)
	KdProcessSwitch ();
}

#define KdScreenOrigin(pScreen) (&(KdGetScreenPriv(pScreen)->screen->origin))

static Bool
KdCursorOffScreen(ScreenPtr *ppScreen, int *x, int *y)
{
    ScreenPtr	pScreen  = *ppScreen;
    ScreenPtr	pNewScreen;
    int		n;
    int		dx, dy;
    int		best_x, best_y;
    int		n_best_x, n_best_y;
    CARD32	ms;

    if (kdDisableZaphod || screenInfo.numScreens <= 1)
	return FALSE;

    if (0 <= *x && *x < pScreen->width && 0 <= *y && *y < pScreen->height)
	return FALSE;

    ms = GetTimeInMillis ();
    if (kdOffScreen && (int) (ms - kdOffScreenTime) < 1000)
	return FALSE;
    kdOffScreen = TRUE;
    kdOffScreenTime = ms;
    n_best_x = -1;
    best_x = 32767;
    n_best_y = -1;
    best_y = 32767;
    for (n = 0; n < screenInfo.numScreens; n++)
    {
	pNewScreen = screenInfo.screens[n];
	if (pNewScreen == pScreen)
	    continue;
	dx = KdScreenOrigin(pNewScreen)->x - KdScreenOrigin(pScreen)->x;
	dy = KdScreenOrigin(pNewScreen)->y - KdScreenOrigin(pScreen)->y;
	if (*x < 0)
	{
	    if (dx <= 0 && -dx < best_x)
	    {
		best_x = -dx;
		n_best_x = n;
	    }
	}
	else if (*x >= pScreen->width)
	{
	    if (dx >= 0 && dx < best_x)
	    {
		best_x = dx;
		n_best_x = n;
	    }
	}
	if (*y < 0)
	{
	    if (dy <= 0 && -dy < best_y)
	    {
		best_y = -dy;
		n_best_y = n;
	    }
	}
	else if (*y >= pScreen->height)
	{
	    if (dy >= 0 && dy < best_y)
	    {
		best_y = dy;
		n_best_y = n;
	    }
	}
    }
    if (best_y < best_x)
	n_best_x = n_best_y;
    if (n_best_x == -1)
	return FALSE;
    pNewScreen = screenInfo.screens[n_best_x];

    if (*x < 0)
	*x += pNewScreen->width;
    if (*y < 0)
	*y += pNewScreen->height;

    if (*x >= pScreen->width)
	*x -= pScreen->width;
    if (*y >= pScreen->height)
	*y -= pScreen->height;

    *ppScreen = pNewScreen;
    return TRUE;
}

static void
KdCrossScreen(ScreenPtr pScreen, Bool entering)
{
#ifndef XIPAQ
    if (entering)
	KdEnableScreen (pScreen);
    else
	KdDisableScreen (pScreen);
#endif
}

int KdCurScreen;	/* current event screen */

static void
KdWarpCursor (DeviceIntPtr pDev, ScreenPtr pScreen, int x, int y)
{
    KdBlockSigio ();
    KdCurScreen = pScreen->myNum;
    miPointerWarpCursor(pDev, pScreen, x, y);
    KdUnblockSigio ();
}

miPointerScreenFuncRec kdPointerScreenFuncs =
{
    KdCursorOffScreen,
    KdCrossScreen,
    KdWarpCursor
};

void
ProcessInputEvents (void)
{
    mieqProcessInputEvents();
    if (kdSwitchPending)
	KdProcessSwitch ();
    KdCheckLock ();
}

/* At the moment, absolute/relative is up to the client. */
int
SetDeviceMode(register ClientPtr client, DeviceIntPtr pDev, int mode)
{
    return BadMatch;
}

int
SetDeviceValuators(register ClientPtr client, DeviceIntPtr pDev,
                   int *valuators, int first_valuator, int num_valuators)
{
    return BadMatch;
}

int
ChangeDeviceControl(register ClientPtr client, DeviceIntPtr pDev,
                        xDeviceCtl *control)
{
    switch (control->control) {
    case DEVICE_RESOLUTION:
        /* FIXME do something more intelligent here */
        return BadMatch;

    case DEVICE_ABS_CALIB:
    case DEVICE_ABS_AREA:
    case DEVICE_CORE:
        return BadMatch;
    case DEVICE_ENABLE:
        return Success;

    default:
        return BadMatch;
    }

    /* NOTREACHED */
    return BadImplementation;
}

int
NewInputDeviceRequest(InputOption *options, InputAttributes *attrs,
                      DeviceIntPtr *pdev)
{
    InputOption *option = NULL;
    KdPointerInfo *pi = NULL;
    KdKeyboardInfo *ki = NULL;

    for (option = options; option; option = option->next) {
        if (strcmp(option->key, "type") == 0) {
            if (strcmp(option->value, "pointer") == 0) {
                pi = KdNewPointer();
                if (!pi)
                    return BadAlloc;
            }
            else if (strcmp(option->value, "keyboard") == 0) {
                ki = KdNewKeyboard();
                if (!ki)
                    return BadAlloc;
            }
            else {
                ErrorF("unrecognised device type!\n");
                return BadValue;
            }
        }
#ifdef CONFIG_HAL
        else if (strcmp(option->key, "_source") == 0 &&
                 strcmp(option->value, "server/hal") == 0)
        {
            ErrorF("Ignoring device from HAL.\n");
            return BadValue;
        }
#endif
#ifdef CONFIG_UDEV
        else if (strcmp(option->key, "_source") == 0 &&
                 strcmp(option->value, "server/udev") == 0)
        {
            ErrorF("Ignoring device from udev.\n");
            return BadValue;
        }
#endif
    }

    if (!ki && !pi) {
        ErrorF("unrecognised device identifier!\n");
        return BadValue;
    }

    /* FIXME: change this code below to use KdParseKbdOptions and
     * KdParsePointerOptions */
    for (option = options; option; option = option->next) {
        if (strcmp(option->key, "device") == 0) {
            if (pi && option->value)
                pi->path = strdup(option->value);
            else if (ki && option->value)
                ki->path = strdup(option->value);
        }
        else if (strcmp(option->key, "driver") == 0) {
            if (pi) {
                pi->driver = KdFindPointerDriver(option->value);
                if (!pi->driver) {
                    ErrorF("couldn't find driver!\n");
                    KdFreePointer(pi);
                    return BadValue;
                }
                pi->options = options;
            }
            else if (ki) {
                ki->driver = KdFindKeyboardDriver(option->value);
                if (!ki->driver) {
                    ErrorF("couldn't find driver!\n");
                    KdFreeKeyboard(ki);
                    return BadValue;
                }
                ki->options = options;
            }
        }
    }

    if (pi) {
        if (KdAddPointer(pi) != Success ||
            ActivateDevice(pi->dixdev, TRUE) != Success ||
            EnableDevice(pi->dixdev, TRUE) != TRUE) {
            ErrorF("couldn't add or enable pointer\n");
            return BadImplementation;
        }
    }
    else if (ki) {
        if (KdAddKeyboard(ki) != Success ||
            ActivateDevice(ki->dixdev, TRUE) != Success ||
            EnableDevice(ki->dixdev, TRUE) != TRUE) {
            ErrorF("couldn't add or enable keyboard\n");
            return BadImplementation;
        }
    }

    if (pi) {
        *pdev = pi->dixdev;
    } else if(ki) {
        *pdev = ki->dixdev;
    }

    return Success;
}

void
DeleteInputDeviceRequest(DeviceIntPtr pDev)
{
    RemoveDevice(pDev, TRUE);
}
