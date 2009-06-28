/*
 * Copyright  1999 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <xgl-config.h>

#include <signal.h>
#include <stdio.h>

#include "xegl.h"
#include "mipointer.h"
#include "inputstr.h"

#define XK_PUBLISHING
#include <X11/keysym.h>
#if HAVE_X11_XF86KEYSYM_H
#include <X11/XF86keysym.h>
#endif
#include "kkeymap.h"

#ifdef XKB
#define XKB_IN_SERVER
#include <xkbsrv.h>
#endif

static DeviceIntPtr	pKdKeyboard, pKdPointer;

#define MAX_MOUSE_DRIVERS   6

static KdMouseFuncs	*kdMouseFuncs[MAX_MOUSE_DRIVERS];
static int		kdNMouseFuncs;
static KdKeyboardFuncs	*kdKeyboardFuncs;
static int		kdBellPitch;
static int		kdBellDuration;
static int		kdLeds;
static Bool		kdInputEnabled;
static Bool		kdOffScreen;
static unsigned long	kdOffScreenTime;
static KdMouseMatrix	kdMouseMatrix = {
   { { 1, 0, 0 },
     { 0, 1, 0 } }
};

int		kdMouseButtonCount;
int		kdMinScanCode;
int		kdMaxScanCode;
int		kdMinKeyCode;
int		kdMaxKeyCode;
int		kdKeymapWidth = KD_MAX_WIDTH;
KeySym		kdKeymap[KD_MAX_LENGTH * KD_MAX_WIDTH];
CARD8		kdModMap[MAP_LENGTH];
KeySymsRec	kdKeySyms;


void
KdResetInputMachine (void);

#define KD_KEY_COUNT		248

CARD8		kdKeyState[KD_KEY_COUNT/8];

#define IsKeyDown(key) ((kdKeyState[(key) >> 3] >> ((key) & 7)) & 1)

#define KD_MAX_INPUT_FDS    8

typedef struct _kdInputFd {
    int	    type;
    int	    fd;
    void    (*read) (int fd, void *closure);
    int	    (*enable) (int fd, void *closure);
    void    (*disable) (int fd, void *closure);
    void    *closure;
} KdInputFd;

KdInputFd    	kdInputFds[KD_MAX_INPUT_FDS];
int		kdNumInputFds;
int		kdInputTypeSequence;

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

#undef VERIFY_SIGIO
#ifdef VERIFY_SIGIO

void
KdAssertSigioBlocked (char *where)
{
    sigset_t	set, old;

    sigemptyset (&set);
    sigprocmask (SIG_BLOCK, &set, &old);
    if (!sigismember (&old, SIGIO))
	ErrorF ("SIGIO not blocked at %s\n", where);
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

int
KdAllocInputType (void)
{
    return ++kdInputTypeSequence;
}

Bool
KdRegisterFd (int type, int fd, void (*read) (int fd, void *closure), void *closure)
{
    if (kdNumInputFds == KD_MAX_INPUT_FDS)
	return FALSE;
    kdInputFds[kdNumInputFds].type = type;
    kdInputFds[kdNumInputFds].fd = fd;
    kdInputFds[kdNumInputFds].read = read;
    kdInputFds[kdNumInputFds].enable = 0;
    kdInputFds[kdNumInputFds].disable = 0;
    kdInputFds[kdNumInputFds].closure = closure;
    ++kdNumInputFds;
    if (kdInputEnabled)
	KdAddFd (fd);
    return TRUE;
}

void
KdRegisterFdEnableDisable (int fd,
			   int (*enable) (int fd, void *closure),
			   void (*disable) (int fd, void *closure))
{
    int	i;

    for (i = 0; i < kdNumInputFds; i++)
	if (kdInputFds[i].fd == fd)
	{
	    kdInputFds[i].enable = enable;
	    kdInputFds[i].disable = disable;
	    break;
	}
}

void
KdUnregisterFds (int type, Bool do_close)
{
    int	i, j;

    for (i = 0; i < kdNumInputFds;)
    {
	if (kdInputFds[i].type == type)
	{
	    if (kdInputEnabled)
		KdRemoveFd (kdInputFds[i].fd);
	    if (do_close)
		close (kdInputFds[i].fd);
	    --kdNumInputFds;
	    for (j = i; j < kdNumInputFds; j++)
		kdInputFds[j] = kdInputFds[j+1];
	}
	else
	    i++;
    }
}

static void
KdDisableInput (void)
{
    int	i;

    KdBlockSigio ();

    for (i = 0; i < kdNumInputFds; i++)
    {
	KdRemoveFd (kdInputFds[i].fd);
	if (kdInputFds[i].disable)
	    (*kdInputFds[i].disable) (kdInputFds[i].fd, kdInputFds[i].closure);
    }
    kdInputEnabled = FALSE;
}

static void
KdEnableInput (void)
{
    xEvent	xE;
    int	i;

    kdInputEnabled = TRUE;
    for (i = 0; i < kdNumInputFds; i++)
    {
	KdNonBlockFd (kdInputFds[i].fd);
	if (kdInputFds[i].enable)
	    kdInputFds[i].fd = (*kdInputFds[i].enable) (kdInputFds[i].fd, kdInputFds[i].closure);
	KdAddFd (kdInputFds[i].fd);
    }

    /* reset screen saver */
    xE.u.keyButtonPointer.time = GetTimeInMillis ();
    NoticeEventTime (&xE);

    KdUnblockSigio ();
}

static int
KdMouseProc(DeviceIntPtr pDevice, int onoff)
{
    BYTE	map[KD_MAX_BUTTON];
    DevicePtr	pDev = (DevicePtr)pDevice;
    int		i;

    if (!pDev)
	return BadImplementation;

    switch (onoff)
    {
    case DEVICE_INIT:
	for (i = 1; i <= kdMouseButtonCount; i++)
	    map[i] = i;
	InitPointerDeviceStruct(pDev, map, kdMouseButtonCount,
	    miPointerGetMotionEvents,
	    (PtrCtrlProcPtr)NoopDDA,
	    miPointerGetMotionBufferSize());
	break;

    case DEVICE_ON:
	pDev->on = TRUE;
	pKdPointer = pDevice;
	for (i = 0; i < kdNMouseFuncs; i++)
	    (*kdMouseFuncs[i]->Init)();
	break;
    case DEVICE_OFF:
    case DEVICE_CLOSE:
	if (pDev->on)
	{
	    pDev->on = FALSE;
	    pKdPointer = 0;
	    for (i = 0; i < kdNMouseFuncs; i++)
		(*kdMouseFuncs[i]->Fini) ();
	}
	break;
    }
    return Success;
}

Bool
KdLegalModifier(unsigned int key, DevicePtr pDev)
{
    return TRUE;
}

static void
KdBell (int volume, DeviceIntPtr pDev, pointer ctrl, int something)
{
    if (kdInputEnabled)
	(*kdKeyboardFuncs->Bell) (volume, kdBellPitch, kdBellDuration);
}


static void
KdSetLeds (void)
{
    if (kdInputEnabled)
	(*kdKeyboardFuncs->Leds) (kdLeds);
}

static void
KdSetLed (int led, Bool on)
{
    NoteLedState (pKdKeyboard, led, on);
    kdLeds = pKdKeyboard->kbdfeed->ctrl.leds;
    KdSetLeds ();
}

static void
KdSetMouseMatrix (KdMouseMatrix *matrix)
{
    kdMouseMatrix = *matrix;
}

static void
KdComputeMouseMatrix (KdMouseMatrix *m, Rotation randr, int width, int height)
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

static void
KdKbdCtrl (DeviceIntPtr pDevice, KeybdCtrl *ctrl)
{
    kdLeds = ctrl->leds;
    kdBellPitch = ctrl->bell_pitch;
    kdBellDuration = ctrl->bell_duration;
    KdSetLeds ();
}

static int
KdKeybdProc(DeviceIntPtr pDevice, int onoff)
{
    Bool        ret;
    DevicePtr   pDev = (DevicePtr)pDevice;
#ifdef XKB
    XkbComponentNamesRec names;
#endif

    if (!pDev)
	return BadImplementation;

    switch (onoff)
    {
    case DEVICE_INIT:
	if (pDev != (DevicePtr)inputInfo.keyboard)
	{
	    return !Success;
	}
#ifndef XKB
	ret = InitKeyboardDeviceStruct(pDev,
				       &kdKeySyms,
				       kdModMap,
				       KdBell, KdKbdCtrl);
#else
	memset(&names, 0, sizeof(XkbComponentNamesRec));

	XkbSetRulesDflts ("base", "pc101", "us", NULL, NULL);
	ret = XkbInitKeyboardDeviceStruct (pDev,
					   &names,
					   &kdKeySyms,
					   kdModMap,
					   KdBell, KdKbdCtrl);
#endif
	if (!ret)
	    return BadImplementation;
	break;
    case DEVICE_ON:
	pDev->on = TRUE;
	pKdKeyboard = pDevice;
	if (kdKeyboardFuncs)
	    (*kdKeyboardFuncs->Init) ();
	break;
    case DEVICE_OFF:
    case DEVICE_CLOSE:
	pKdKeyboard = 0;
	if (pDev->on)
	{
	    pDev->on = FALSE;
	    if (kdKeyboardFuncs)
		(*kdKeyboardFuncs->Fini) ();
	}
	break;
    }
    return Success;
}

extern KeybdCtrl defaultKeyboardControl;

static void
KdInitAutoRepeats (void)
{
    int		    key_code;
    unsigned char   mask;
    int		    i;
    unsigned char   *repeats;

    repeats = defaultKeyboardControl.autoRepeats;
    memset (repeats, '\0', 32);
    for (key_code = KD_MIN_KEYCODE; key_code <= KD_MAX_KEYCODE; key_code++)
    {
	if (!kdModMap[key_code])
	{
	    i = key_code >> 3;
	    mask = 1 << (key_code & 7);
	    repeats[i] |= mask;
	}
    }
}

const KdKeySymModsRec kdKeySymMods[] = {
  {  XK_Control_L,	ControlMask },
  {  XK_Control_R, ControlMask },
  {  XK_Shift_L,	ShiftMask },
  {  XK_Shift_R,	ShiftMask },
  {  XK_Caps_Lock,	LockMask },
  {  XK_Shift_Lock, LockMask },
  {  XK_Alt_L,	Mod1Mask },
  {  XK_Alt_R,	Mod1Mask },
  {  XK_Meta_L,	Mod1Mask },
  {  XK_Meta_R,	Mod1Mask },
  {  XK_Num_Lock,	Mod2Mask },
  {  XK_Super_L,	Mod3Mask },
  {  XK_Super_R,	Mod3Mask },
  {  XK_Hyper_L,	Mod3Mask },
  {  XK_Hyper_R,	Mod3Mask },
  {  XK_Mode_switch, Mod4Mask },
#ifdef TOUCHSCREEN
  /* PDA specific hacks */
#ifdef XF86XK_Start
  {  XF86XK_Start, ControlMask },
#endif
  {  XK_Menu, ShiftMask },
  {  XK_telephone, Mod1Mask },
#ifdef XF86XK_AudioRecord
  {  XF86XK_AudioRecord, Mod2Mask },
#endif
#ifdef XF86XK_Calendar
  {  XF86XK_Calendar, Mod3Mask }
#endif
#endif
};

#define NUM_SYM_MODS (sizeof(kdKeySymMods) / sizeof(kdKeySymMods[0]))

static void
KdInitModMap (void)
{
    int	    key_code;
    int	    row;
    int	    width;
    KeySym  *syms;
    int	    i;

    width = kdKeySyms.mapWidth;
    for (key_code = kdMinKeyCode; key_code <= kdMaxKeyCode; key_code++)
    {
	kdModMap[key_code] = 0;
	syms = kdKeymap + (key_code - kdMinKeyCode) * width;
	for (row = 0; row < width; row++, syms++)
	{
	    for (i = 0; i < NUM_SYM_MODS; i++)
	    {
		if (*syms == kdKeySymMods[i].modsym)
		    kdModMap[key_code] |= kdKeySymMods[i].modbit;
	    }
	}
    }
}

static void
KdAddMouseDriver(KdMouseFuncs *pMouseFuncs)
{
    if (kdNMouseFuncs < MAX_MOUSE_DRIVERS)
	kdMouseFuncs[kdNMouseFuncs++] = pMouseFuncs;
}

void
eglInitInput(KdMouseFuncs    *pMouseFuncs,
	    KdKeyboardFuncs *pKeyboardFuncs)
{
    DeviceIntPtr	pKeyboard, pPointer;
    KdMouseInfo		*mi;

    if (!kdMouseInfo)
	KdParseMouse (0);
    kdMouseButtonCount = 0;
    for (mi = kdMouseInfo; mi; mi = mi->next)
    {
	if (mi->nbutton > kdMouseButtonCount)
	    kdMouseButtonCount = mi->nbutton;
    }

    kdNMouseFuncs = 0;
    KdAddMouseDriver (pMouseFuncs);
    kdKeyboardFuncs = pKeyboardFuncs;
    memset (kdKeyState, '\0', sizeof (kdKeyState));
    if (kdKeyboardFuncs)
	(*kdKeyboardFuncs->Load) ();
    kdMinKeyCode = kdMinScanCode + KD_KEY_OFFSET;
    kdMaxKeyCode = kdMaxScanCode + KD_KEY_OFFSET;
    kdKeySyms.map = kdKeymap;
    kdKeySyms.minKeyCode = kdMinKeyCode;
    kdKeySyms.maxKeyCode = kdMaxKeyCode;
    kdKeySyms.mapWidth = kdKeymapWidth;
    kdLeds = 0;
    kdBellPitch = 1000;
    kdBellDuration = 200;
    kdInputEnabled = TRUE;
    KdInitModMap ();
    KdInitAutoRepeats ();
    KdResetInputMachine ();
    pPointer  = AddInputDevice(KdMouseProc, TRUE);
    pKeyboard = AddInputDevice(KdKeybdProc, TRUE);
    RegisterPointerDevice(pPointer);
    RegisterKeyboardDevice(pKeyboard);
    miRegisterPointerDevice(screenInfo.screens[0], pPointer);
    mieqInit(&pKeyboard->public, &pPointer->public);
#ifdef XINPUT
    {
	static long zero1, zero2;

	//SetExtInputCheck (&zero1, &zero2);
	ErrorF("Extended Input Devices not yet supported. Impelement it at line %d in %s\n",
	       __LINE__, __FILE__);
    }
#endif
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
    KdMouseState   nextState;
} KdInputTransition;

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

#define EventX(e)   ((e)->u.keyButtonPointer.rootX)
#define EventY(e)   ((e)->u.keyButtonPointer.rootY)

static int
KdInsideEmulationWindow (KdMouseInfo *mi, xEvent *ev)
{
    if (ev->u.keyButtonPointer.pad1)
    {
	mi->emulationDx += EventX(ev);
	mi->emulationDy += EventY(ev);
    }
    else
    {
	mi->emulationDx = EventX(&mi->heldEvent) - EventX(ev);
	mi->emulationDy = EventY(&mi->heldEvent) - EventY(ev);
    }
    return (abs (mi->emulationDx) < EMULATION_WINDOW &&
	    abs (mi->emulationDy) < EMULATION_WINDOW);
}

static KdInputClass
KdClassifyInput (KdMouseInfo *mi, xEvent *ev)
{
    switch (ev->u.u.type) {
    case ButtonPress:
	switch (ev->u.u.detail) {
	case 1: return down_1;
	case 2: return down_2;
	case 3: return down_3;
	default: return down_o;
	}
	break;
    case ButtonRelease:
	switch (ev->u.u.detail) {
	case 1: return up_1;
	case 2: return up_2;
	case 3: return up_3;
	default: return up_o;
	}
	break;
    case MotionNotify:
	if (mi->eventHeld && !KdInsideEmulationWindow(mi, ev))
	    return outside_box;
	else
	    return motion;
    default:
	return keyboard;
    }
    return keyboard;
}

#ifndef NDEBUG
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
#endif

static void
KdQueueEvent (xEvent *ev)
{
    KdAssertSigioBlocked ("KdQueueEvent");
    if (ev->u.u.type == MotionNotify)
    {
	if (ev->u.keyButtonPointer.pad1)
	{
	    ev->u.keyButtonPointer.pad1 = 0;
	    miPointerDeltaCursor (ev->u.keyButtonPointer.rootX,
				  ev->u.keyButtonPointer.rootY,
				  ev->u.keyButtonPointer.time);
	}
	else
	{
	    miPointerAbsoluteCursor(ev->u.keyButtonPointer.rootX,
				    ev->u.keyButtonPointer.rootY,
				    ev->u.keyButtonPointer.time);
	}
    }
    else
    {
	mieqEnqueue (ev);
    }
}

static void
KdRunMouseMachine (KdMouseInfo *mi, KdInputClass c, xEvent *ev)
{
    KdInputTransition	*t;
    int			a;

    t = &kdInputMachine[mi->mouseState][c];
    for (a = 0; a < MAX_ACTIONS; a++)
    {
	switch (t->actions[a]) {
	case noop:
	    break;
	case hold:
	    mi->eventHeld = TRUE;
	    mi->emulationDx = 0;
	    mi->emulationDy = 0;
	    mi->heldEvent = *ev;
	    break;
	case setto:
	    mi->emulationTimeout = GetTimeInMillis () + EMULATION_TIMEOUT;
	    mi->timeoutPending = TRUE;
	    break;
	case deliver:
	    KdQueueEvent (ev);
	    break;
	case release:
	    mi->eventHeld = FALSE;
	    mi->timeoutPending = FALSE;
	    KdQueueEvent (&mi->heldEvent);
	    break;
	case clearto:
	    mi->timeoutPending = FALSE;
	    break;
	case gen_down_2:
	    ev->u.u.detail = 2;
	    mi->eventHeld = FALSE;
	    KdQueueEvent (ev);
	    break;
	case gen_up_2:
	    ev->u.u.detail = 2;
	    KdQueueEvent (ev);
	    break;
	}
    }
    mi->mouseState = t->nextState;
}

void
KdResetInputMachine (void)
{
    KdMouseInfo	*mi;

    for (mi = kdMouseInfo; mi; mi = mi->next)
    {
	mi->mouseState = start;
	mi->eventHeld = FALSE;
    }
}

static void
KdHandleMouseEvent (KdMouseInfo *mi, xEvent *ev)
{
    if (mi->emulateMiddleButton)
	KdRunMouseMachine (mi, KdClassifyInput (mi, ev), ev);
    else
	KdQueueEvent (ev);
}

static void
KdReceiveTimeout (KdMouseInfo *mi)
{
    KdRunMouseMachine (mi, timeout, 0);
}

#define KILL_SEQUENCE ((1L << KK_CONTROL)|(1L << KK_ALT)|(1L << KK_F8)|(1L << KK_F10))
#define SPECIAL_SEQUENCE ((1L << KK_CONTROL) | (1L << KK_ALT))
#define SETKILLKEY(b) (KdSpecialKeys |= (1L << (b)))
#define CLEARKILLKEY(b) (KdSpecialKeys &= ~(1L << (b)))
#define KEYMAP	    (pKdKeyboard->key->curKeySyms)
#define KEYCOL1(k) (KEYMAP.map[((k)-kdMinKeyCode)*KEYMAP.mapWidth])

CARD32	KdSpecialKeys = 0;

extern char dispatchException;

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

static void
KdCheckSpecialKeys(xEvent *xE)
{
    KeySym	sym = KEYCOL1(xE->u.u.detail);

    if (!pKdKeyboard) return;

    /*
     * Ignore key releases
     */

    if (xE->u.u.type == KeyRelease) return;

#ifdef XIPAQ
    /*
     * Check for buttons 1, 2 and 3 on the iPAQ
     */
    if (sym == XK_Pointer_Button1 && kdMouseInfo) {
	KdEnqueueMouseEvent(kdMouseInfo, KD_MOUSE_DELTA | KD_BUTTON_1, 0, 0);
	return;
    }
    if (sym == XK_Pointer_Button2 && kdMouseInfo) {
	KdEnqueueMouseEvent(kdMouseInfo, KD_MOUSE_DELTA | KD_BUTTON_2, 0, 0);
	return;
    }
    if (sym == XK_Pointer_Button3 && kdMouseInfo) {
	KdEnqueueMouseEvent(kdMouseInfo, KD_MOUSE_DELTA | KD_BUTTON_3, 0, 0);
	return;
    }
#endif

    /*
     * Check for control/alt pressed
     */
    if ((pKdKeyboard->key->state & (ControlMask|Mod1Mask)) !=
	(ControlMask|Mod1Mask))
	return;


    /*
     * Let OS function see keysym first
     */

    if (kdOsFuncs->SpecialKey)
	if ((*kdOsFuncs->SpecialKey) (sym))
	    return;

    /*
     * Now check for backspace or delete; these signal the
     * X server to terminate
     */
    switch (sym) {
    case XK_BackSpace:
    case XK_Delete:
    case XK_KP_Delete:
	/*
	 * Set the dispatch exception flag so the server will terminate the
	 * next time through the dispatch loop.
	 */
	if (kdDontZap == FALSE)
	    dispatchException |= DE_TERMINATE;
	break;
    }
}

/*
 * kdEnqueueKeyboardEvent
 *
 * This function converts hardware keyboard event information into an X event
 * and enqueues it using MI.  It wakes up the server before returning so that
 * the event will be processed normally.
 *
 */

static void
KdHandleKeyboardEvent (xEvent *ev)
{
    int		key = ev->u.u.detail;
    int		byte;
    CARD8	bit;
    KdMouseInfo	*mi;

    byte = key >> 3;
    bit = 1 << (key & 7);
    switch (ev->u.u.type) {
    case KeyPress:
	kdKeyState[byte] |= bit;
	break;
    case KeyRelease:
	kdKeyState[byte] &= ~bit;
	break;
    }
    for (mi = kdMouseInfo; mi; mi = mi->next)
	KdRunMouseMachine (mi, keyboard, 0);
    KdQueueEvent (ev);
}

static void
KdReleaseAllKeys (void)
{
    xEvent  xE;
    int	    key;

    KdBlockSigio ();
    for (key = 0; key < KD_KEY_COUNT; key++)
	if (IsKeyDown(key))
	{
	    xE.u.keyButtonPointer.time = GetTimeInMillis();
	    xE.u.u.type = KeyRelease;
	    xE.u.u.detail = key;
	    KdHandleKeyboardEvent (&xE);
	}
    KdUnblockSigio ();
}

static void
KdCheckLock (void)
{
    KeyClassPtr	    keyc = pKdKeyboard->key;
    Bool	    isSet, shouldBeSet;

    if (kdKeyboardFuncs->LockLed)
    {
	isSet = (kdLeds & (1 << (kdKeyboardFuncs->LockLed-1))) != 0;
	shouldBeSet = (keyc->state & LockMask) != 0;
	if (isSet != shouldBeSet)
	{
	    KdSetLed (kdKeyboardFuncs->LockLed, shouldBeSet);
	}
    }
}

void
KdEnqueueKeyboardEvent(unsigned char	scan_code,
		       unsigned char	is_up)
{
    unsigned char   key_code;
    xEvent	    xE;
    KeyClassPtr	    keyc;

    if (!pKdKeyboard)
	return;
    keyc = pKdKeyboard->key;

    xE.u.keyButtonPointer.time = GetTimeInMillis();

    if (kdMinScanCode <= scan_code && scan_code <= kdMaxScanCode)
    {
	key_code = scan_code + KD_MIN_KEYCODE - kdMinScanCode;

	/*
	 * Set up this event -- the type may be modified below
	 */
	if (is_up)
	    xE.u.u.type = KeyRelease;
	else
	    xE.u.u.type = KeyPress;
	xE.u.u.detail = key_code;

	switch (KEYCOL1(key_code))
	{
	case XK_Num_Lock:
	case XK_Scroll_Lock:
	case XK_Shift_Lock:
	case XK_Caps_Lock:
	    if (xE.u.u.type == KeyRelease)
		return;
	    if (IsKeyDown (key_code))
		xE.u.u.type = KeyRelease;
	    else
		xE.u.u.type = KeyPress;
	}

	/*
	 * Check pressed keys which are already down
	 */
	if (IsKeyDown (key_code) && xE.u.u.type == KeyPress)
	{
	    KeybdCtrl	*ctrl = &pKdKeyboard->kbdfeed->ctrl;

	    /*
	     * Check auto repeat
	     */
	    if (!ctrl->autoRepeat || keyc->modifierMap[key_code] ||
		!(ctrl->autoRepeats[key_code >> 3] & (1 << (key_code & 7))))
	    {
		return;
	    }
	    /*
	     * X delivers press/release even for autorepeat
	     */
	    xE.u.u.type = KeyRelease;
	    KdHandleKeyboardEvent (&xE);
	    xE.u.u.type = KeyPress;
	}
	/*
	 * Check released keys which are already up
	 */
	else if (!IsKeyDown (key_code) && xE.u.u.type == KeyRelease)
	{
	    return;
	}
	KdCheckSpecialKeys (&xE);
	KdHandleKeyboardEvent (&xE);
    }
}

#define SetButton(mi, b, v, s) \
{\
    xE.u.u.detail = mi->map[b]; \
    xE.u.u.type = v; \
    KdHandleMouseEvent (mi, &xE); \
}

#define Press(mi, b)         SetButton(mi, b, ButtonPress, "Down")
#define Release(mi, b)       SetButton(mi, b, ButtonRelease, "Up")

/*
 * kdEnqueueMouseEvent
 *
 * This function converts hardware mouse event information into X event
 * information.  A mouse movement event is passed off to MI to generate
 * a MotionNotify event, if appropriate.  Button events are created and
 * passed off to MI for enqueueing.
 */

static void
KdMouseAccelerate (DeviceIntPtr	device, int *dx, int *dy)
{
    PtrCtrl *pCtrl = &device->ptrfeed->ctrl;
    double  speed = sqrt (*dx * *dx + *dy * *dy);
    double  accel;
    double  m;

    /*
     * Ok, so we want it moving num/den times faster at threshold*2
     *
     * accel = m *threshold + b
     * 1 = m * 0 + b	-> b = 1
     *
     * num/den = m * (threshold * 2) + 1
     *
     * num / den - 1 = m * threshold * 2
     * (num / den - 1) / threshold * 2 = m
     */
    m = (((double) pCtrl->num / (double) pCtrl->den - 1.0) /
	 ((double) pCtrl->threshold * 2.0));
    accel = m * speed + 1;
    *dx = accel * *dx;
    *dy = accel * *dy;
}

void
KdEnqueueMouseEvent(KdMouseInfo *mi, unsigned long flags, int rx, int ry)
{
    CARD32	    ms;
    xEvent	    xE;
    unsigned char   buttons;
    int		    x, y;
    int		    (*matrix)[3] = kdMouseMatrix.matrix;
    unsigned long   button;
    int		    n;

    if (!pKdPointer)
	return;

    ms = GetTimeInMillis();

    if (flags & KD_MOUSE_DELTA)
    {
	if (mi->transformCoordinates)
	{
	    x = matrix[0][0] * rx + matrix[0][1] * ry;
	    y = matrix[1][0] * rx + matrix[1][1] * ry;
	}
	else
	{
	    x = rx;
	    y = ry;
	}
	KdMouseAccelerate (pKdPointer, &x, &y);
	xE.u.keyButtonPointer.pad1 = 1;
    }
    else
    {
	if (mi->transformCoordinates)
	{
	    x = matrix[0][0] * rx + matrix[0][1] * ry + matrix[0][2];
	    y = matrix[1][0] * rx + matrix[1][1] * ry + matrix[1][2];
	}
	else
	{
	    x = rx;
	    y = ry;
	}
	xE.u.keyButtonPointer.pad1 = 0;
    }
    xE.u.keyButtonPointer.time = ms;
    xE.u.keyButtonPointer.rootX = x;
    xE.u.keyButtonPointer.rootY = y;

    xE.u.u.type = MotionNotify;
    xE.u.u.detail = 0;
    KdHandleMouseEvent (mi, &xE);

    buttons = flags;

    for (button = KD_BUTTON_1, n = 0; button <= KD_BUTTON_5; button <<= 1, n++)
    {
	if ((mi->buttonState & button) ^ (buttons & button))
	{
	    if (buttons & button)
	    {
		Press(mi, n);
	    }
	    else
	    {
		Release(mi, n);
	    }
	}
    }
    mi->buttonState = buttons;
}

static void
KdEnqueueMotionEvent (KdMouseInfo *mi, int x, int y)
{
    xEvent  xE;
    CARD32  ms;

    ms = GetTimeInMillis();

    xE.u.u.type = MotionNotify;
    xE.u.keyButtonPointer.time = ms;
    xE.u.keyButtonPointer.rootX = x;
    xE.u.keyButtonPointer.rootY = y;

    KdHandleMouseEvent (mi, &xE);
}

static void
KdBlockHandler (int		screen,
		pointer		blockData,
		pointer		timeout,
		pointer		readmask)
{
    KdMouseInfo		    *mi;
    int myTimeout=0;

    for (mi = kdMouseInfo; mi; mi = mi->next)
    {
	if (mi->timeoutPending)
	{
	    int	ms;

	    ms = mi->emulationTimeout - GetTimeInMillis ();
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
KdWakeupHandler (pointer data,
		 int	 result,
		 pointer readmask)
{
    fd_set	*pReadmask = (fd_set *) readmask;
    int		i;
    KdMouseInfo	*mi;

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
    for (mi = kdMouseInfo; mi; mi = mi->next)
    {
	if (mi->timeoutPending)
	{
	    if ((long) (GetTimeInMillis () - mi->emulationTimeout) >= 0)
	    {
		mi->timeoutPending = FALSE;
		KdBlockSigio ();
		KdReceiveTimeout (mi);
		KdUnblockSigio ();
	    }
	}
    }
//  if (kdSwitchPending)
//      kdProcessSwitch ();
}

#define KdScreenOrigin(pScreen) (&(KdGetScreenPriv (pScreen)->origin))

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
//  if (entering)
//      KdEnableScreen (pScreen);
//  else
//      KdDisableScreen (pScreen);
#endif
}

int KdCurScreen;	/* current event screen */

static void
KdWarpCursor (ScreenPtr pScreen, int x, int y)
{
    KdBlockSigio ();
    KdCurScreen = pScreen->myNum;
    miPointerWarpCursor (pScreen, x, y);
    KdUnblockSigio ();
}

miPointerScreenFuncRec kdPointerScreenFuncs =
{
    KdCursorOffScreen,
    KdCrossScreen,
    KdWarpCursor
};

void
KdProcessInputEvents (void)
{
    mieqProcessInputEvents();
    miPointerUpdate();
//  if (kdSwitchPending)
//      KdProcessSwitch ();
    KdCheckLock ();
}
