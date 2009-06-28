/*
 * Copyright © 2004 Keith Packard
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
#define NEED_EVENTS
#include <errno.h>
#include <linux/input.h>
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xpoll.h>
#define XK_PUBLISHING
#include <X11/keysym.h>
#include "inputstr.h"
#include "kkeymap.h"
#include "scrnintstr.h"
#include "xegl.h"

#define NUM_EVENTS  128
#define ABS_UNSET   -65535

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define ISBITSET(x,y) ((x)[LONG(y)] & BIT(y))
#define OFF(x)   ((x)%BITS_PER_LONG)
#define LONG(x)  ((x)/BITS_PER_LONG)
#define BIT(x)	 (1 << OFF(x))
#define SETBIT(x,y) ((x)[LONG(y)] |= BIT(y))
#define CLRBIT(x,y) ((x)[LONG(y)] &= ~BIT(y))
#define ASSIGNBIT(x,y,z)    ((x)[LONG(y)] = ((x)[LONG(y)] & ~BIT(y)) | (z << OFF(y)))

typedef struct _kevdevMouse {
    /* current device state */
    int			    rel[REL_MAX + 1];
    int			    abs[ABS_MAX + 1];
    int			    prevabs[ABS_MAX + 1];
    long		    key[NBITS(KEY_MAX + 1)];

    /* supported device info */
    long		    relbits[NBITS(REL_MAX + 1)];
    long		    absbits[NBITS(ABS_MAX + 1)];
    long		    keybits[NBITS(KEY_MAX + 1)];
    struct input_absinfo    absinfo[ABS_MAX + 1];
    int			    max_rel;
    int			    max_abs;
} Kevdev;

static int flags = 0;

static void
EvdevMotion (KdMouseInfo    *mi)
{
    Kevdev		*ke = mi->driver;
    int			i;

    for (i = 0; i <= ke->max_rel; i++)
	if (ke->rel[i])
	{
            KdEnqueueMouseEvent (mi, flags | KD_MOUSE_DELTA, ke->rel[0], ke->rel[1]);
	    int a;
//	    ErrorF ("rel");
	    for (a = 0; a <= ke->max_rel; a++)
	    {
//		if (ISBITSET (ke->relbits, a))
//		    ErrorF (" %d=%d", a, ke->rel[a]);
		ke->rel[a] = 0;
	    }
//	    ErrorF ("\n");
	    break;
	}
    for (i = 0; i < ke->max_abs; i++)
	if (ke->abs[i] != ke->prevabs[i])
	{
            KdEnqueueMouseEvent (mi, flags, ke->abs[0], ke->abs[1]);
	    int a;
//	    ErrorF ("abs");
	    for (a = 0; a <= ke->max_abs; a++)
	    {
//		if (ISBITSET (ke->absbits, a))
//		    ErrorF (" %d=%d", a, ke->abs[a]);
		ke->prevabs[a] = ke->abs[a];
	    }
//	    ErrorF ("\n");
	    break;
	}
}

static void
EvdevRead (int evdevPort, void *closure)
{
    KdMouseInfo		*mi = closure;
    Kevdev		*ke = mi->driver;
    int			i, n;
    struct input_event	events[NUM_EVENTS];

    n = read (evdevPort, &events, NUM_EVENTS * sizeof (struct input_event));
    if (n <= 0)
	return;
    n /= sizeof (struct input_event);
    for (i = 0; i < n; i++)
    {
	switch (events[i].type) {
	case EV_SYN:
	    break;
	case EV_KEY:
	    EvdevMotion (mi);
	    ASSIGNBIT(ke->key,events[i].code, events[i].value);
	    if (events[i].code < 0x100)
		ErrorF ("key %d %d\n", events[i].code, events[i].value);
	    else
		ErrorF ("key 0x%x %d\n", events[i].code, events[i].value);
	    
	    if (events[i].value==1) {
	      switch (events[i].code) {
	      case BTN_LEFT:
                flags |= KD_BUTTON_1;
                break;
	      case BTN_RIGHT:
		flags |= KD_BUTTON_3;
                break;
	      case BTN_MIDDLE:
                flags |= KD_BUTTON_2;
                break;
	      case BTN_FORWARD:
                flags |= KD_BUTTON_4;
                break;
	      case BTN_BACK:
                flags |= KD_BUTTON_5;
                break;
	      }
	    }
	    else if (events[i].value==0) {
	      switch (events[i].code) {
	      case BTN_LEFT:
                flags &= ~KD_BUTTON_1;
                break;
	      case BTN_RIGHT:
		flags &= ~KD_BUTTON_3;
                break;
	      case BTN_MIDDLE:
                flags &= ~KD_BUTTON_2;
                break;
	      case BTN_FORWARD:
                flags &= ~KD_BUTTON_4;
                break;
	      case BTN_BACK:
                flags &= ~KD_BUTTON_5;
                break;
	      }
	    }
	    KdEnqueueMouseEvent (mi, KD_MOUSE_DELTA | flags, 0, 0);
	    break;
	case EV_REL:
	    ke->rel[events[i].code] += events[i].value;
	    break;
	case EV_ABS:
	    ke->abs[events[i].code] = events[i].value;
	    break;
	}
    }
    EvdevMotion (mi);
}

int EvdevInputType;

char *kdefaultEvdev[] =  {
  //    "/dev/input/event0",
    "/dev/input/event1",
    //   "/dev/input/event2",
    // "/dev/input/event3",
    //    "/dev/input/event4",
    //   "/dev/input/event5",
};

#define NUM_DEFAULT_EVDEV    (sizeof (kdefaultEvdev) / sizeof (kdefaultEvdev[0]))

static Bool
EvdevInit (void)
{
    int		i;
    int		fd;
    KdMouseInfo	*mi, *next;
    int		n = 0;
    char	*prot;
    char        name[100];

    if (!EvdevInputType)
	EvdevInputType = KdAllocInputType ();

    for (mi = kdMouseInfo; mi; mi = next)
    {
	next = mi->next;
	prot = mi->prot;
	if (mi->inputType)
	    continue;
	if (!mi->name)
	{
	    for (i = 0; i < NUM_DEFAULT_EVDEV; i++)
	    {
		fd = open (kdefaultEvdev[i], 2);
		if (fd >= 0)
		{
                    ioctl(fd, EVIOCGRAB, 1);

                    ioctl(fd, EVIOCGNAME(sizeof(name)), name);
                    ErrorF("Name is %s\n", name);
                    ioctl(fd, EVIOCGPHYS(sizeof(name)), name);
                    ErrorF("Phys Loc is %s\n", name);
                    ioctl(fd, EVIOCGUNIQ(sizeof(name)), name);
                    ErrorF("Unique is %s\n", name);

		    mi->name = KdSaveString (kdefaultEvdev[i]);
		    break;
		}
	    }
	}
	else
	    fd = open (mi->name, 2);

	if (fd >= 0)
	{
	    unsigned long   ev[NBITS(EV_MAX)];
	    Kevdev	    *ke;

 	    if (ioctl (fd, EVIOCGBIT(0 /*EV*/, sizeof (ev)), ev) < 0)
	    {
		perror ("EVIOCGBIT 0");
		close (fd);
		continue;
	    }
	    ke = xalloc (sizeof (Kevdev));
	    if (!ke)
	    {
		close (fd);
		continue;
	    }
	    memset (ke, '\0', sizeof (Kevdev));
	    if (ISBITSET (ev, EV_KEY))
	    {
		if (ioctl (fd, EVIOCGBIT (EV_KEY, sizeof (ke->keybits)),
			   ke->keybits) < 0)
		{
		    perror ("EVIOCGBIT EV_KEY");
		    xfree (ke);
		    close (fd);
		    continue;
		}
	    }
	    if (ISBITSET (ev, EV_REL))
	    {
		if (ioctl (fd, EVIOCGBIT (EV_REL, sizeof (ke->relbits)),
			   ke->relbits) < 0)
		{
		    perror ("EVIOCGBIT EV_REL");
		    xfree (ke);
		    close (fd);
		    continue;
		}
		for (ke->max_rel = REL_MAX; ke->max_rel >= 0; ke->max_rel--)
		    if (ISBITSET(ke->relbits, ke->max_rel))
			break;
	    }
	    if (ISBITSET (ev, EV_ABS))
	    {
		int i;

		if (ioctl (fd, EVIOCGBIT (EV_ABS, sizeof (ke->absbits)),
			   ke->absbits) < 0)
		{
		    perror ("EVIOCGBIT EV_ABS");
		    xfree (ke);
		    close (fd);
		    continue;
		}
		for (ke->max_abs = ABS_MAX; ke->max_abs >= 0; ke->max_abs--)
		    if (ISBITSET(ke->absbits, ke->max_abs))
			break;
		for (i = 0; i <= ke->max_abs; i++)
		{
		    if (ISBITSET (ke->absbits, i))
			if (ioctl (fd, EVIOCGABS(i), &ke->absinfo[i]) < 0)
			{
			    perror ("EVIOCGABS");
			    break;
			}
		    ke->prevabs[i] = ABS_UNSET;
		}
		if (i <= ke->max_abs)
		{
		    xfree (ke);
		    close (fd);
		    continue;
		}
	    }
	    mi->driver = ke;
	    mi->inputType = EvdevInputType;
	    if (KdRegisterFd (EvdevInputType, fd, EvdevRead, (void *) mi))
		n++;
	}
    }
    return TRUE;
}

static void
EvdevFini (void)
{
    KdMouseInfo	*mi;

    KdUnregisterFds (EvdevInputType, TRUE);
    for (mi = kdMouseInfo; mi; mi = mi->next)
    {
	if (mi->inputType == EvdevInputType)
	{
	    xfree (mi->driver);
	    mi->driver = 0;
	    mi->inputType = 0;
	}
    }
}

KdMouseFuncs LinuxEvdevMouseFuncs = {
    EvdevInit,
    EvdevFini,
};


KeySym evdevKeymap[(112 - 1 + 1) * 2] = {
/* These are directly mapped from DOS scanset 0 */
/*      1     8 */       XK_Escape, NoSymbol,
/*      2     9 */       XK_1,  XK_exclam,
/*      3    10 */       XK_2,  XK_at,
/*      4    11 */       XK_3,  XK_numbersign,
/*      5    12 */       XK_4,  XK_dollar,
/*      6    13 */       XK_5,  XK_percent,
/*      7    14 */       XK_6,  XK_asciicircum,
/*      8    15 */       XK_7,  XK_ampersand,
/*      9    16 */       XK_8,  XK_asterisk,
/*     10    17 */       XK_9,  XK_parenleft,
/*     11    18 */       XK_0,  XK_parenright,
/*     12    19 */       XK_minus,      XK_underscore,
/*     13    20 */       XK_equal,      XK_plus,
/*     14    21 */       XK_BackSpace,  NoSymbol,
/*     15    22 */       XK_Tab,        NoSymbol,
/*     16    23 */       XK_Q,  NoSymbol,
/*     17    24 */       XK_W,  NoSymbol,
/*     18    25 */       XK_E,  NoSymbol,
/*     19    26 */       XK_R,  NoSymbol,
/*     20    27 */       XK_T,  NoSymbol,
/*     21    28 */       XK_Y,  NoSymbol,
/*     22    29 */       XK_U,  NoSymbol,
/*     23    30 */       XK_I,  NoSymbol,
/*     24    31 */       XK_O,  NoSymbol,
/*     25    32 */       XK_P,  NoSymbol,
/*     26    33 */       XK_bracketleft,        XK_braceleft,
/*     27    34 */       XK_bracketright,       XK_braceright,
/*     28    35 */       XK_Return,     NoSymbol,
/*     29    36 */       XK_Control_L,  NoSymbol,
/*     30    37 */       XK_A,  NoSymbol,
/*     31    38 */       XK_S,  NoSymbol,
/*     32    39 */       XK_D,  NoSymbol,
/*     33    40 */       XK_F,  NoSymbol,
/*     34    41 */       XK_G,  NoSymbol,
/*     35    42 */       XK_H,  NoSymbol,
/*     36    43 */       XK_J,  NoSymbol,
/*     37    44 */       XK_K,  NoSymbol,
/*     38    45 */       XK_L,  NoSymbol,
/*     39    46 */       XK_semicolon,  XK_colon,
/*     40    47 */       XK_apostrophe, XK_quotedbl,
/*     41    48 */       XK_grave,      XK_asciitilde,
/*     42    49 */       XK_Shift_L,    NoSymbol,
/*     43    50 */       XK_backslash,  XK_bar,
/*     44    51 */       XK_Z,  NoSymbol,
/*     45    52 */       XK_X,  NoSymbol,
/*     46    53 */       XK_C,  NoSymbol,
/*     47    54 */       XK_V,  NoSymbol,
/*     48    55 */       XK_B,  NoSymbol,
/*     49    56 */       XK_N,  NoSymbol,
/*     50    57 */       XK_M,  NoSymbol,
/*     51    58 */       XK_comma,      XK_less,
/*     52    59 */       XK_period,     XK_greater,
/*     53    60 */       XK_slash,      XK_question,
/*     54    61 */       XK_Shift_R,    NoSymbol,
/*     55    62 */       XK_KP_Multiply,        NoSymbol,
/*     56    63 */       XK_Alt_L,      XK_Meta_L,
/*     57    64 */       XK_space,      NoSymbol,
/*     58    65 */       XK_Caps_Lock,  NoSymbol,
/*     59    66 */       XK_F1, NoSymbol,
/*     60    67 */       XK_F2, NoSymbol,
/*     61    68 */       XK_F3, NoSymbol,
/*     62    69 */       XK_F4, NoSymbol,
/*     63    70 */       XK_F5, NoSymbol,
/*     64    71 */       XK_F6, NoSymbol,
/*     65    72 */       XK_F7, NoSymbol,
/*     66    73 */       XK_F8, NoSymbol,
/*     67    74 */       XK_F9, NoSymbol,
/*     68    75 */       XK_F10,        NoSymbol,
/*     69    76 */       XK_Break,      XK_Pause,
/*     70    77 */       XK_Scroll_Lock,        NoSymbol,
/*     71    78 */       XK_KP_Home,    XK_KP_7,
/*     72    79 */       XK_KP_Up,      XK_KP_8,
/*     73    80 */       XK_KP_Page_Up, XK_KP_9,
/*     74    81 */       XK_KP_Subtract,        NoSymbol,
/*     75    82 */       XK_KP_Left,    XK_KP_4,
/*     76    83 */       XK_KP_5,       NoSymbol,
/*     77    84 */       XK_KP_Right,   XK_KP_6,
/*     78    85 */       XK_KP_Add,     NoSymbol,
/*     79    86 */       XK_KP_End,     XK_KP_1,
/*     80    87 */       XK_KP_Down,    XK_KP_2,
/*     81    88 */       XK_KP_Page_Down,       XK_KP_3,
/*     82    89 */       XK_KP_Insert,  XK_KP_0,
/*     83    90 */       XK_KP_Delete,  XK_KP_Decimal,
/*     84    91 */     NoSymbol,        NoSymbol,
/*     85    92 */     NoSymbol,        NoSymbol,
/*     86    93 */     NoSymbol,        NoSymbol,
/*     87    94 */       XK_F11,        NoSymbol,
/*     88    95 */       XK_F12,        NoSymbol,

/* These are remapped from the extended set (using ExtendMap) */

/*     89    96 */       XK_Control_R,  NoSymbol,
/*     90    97 */       XK_KP_Enter,   NoSymbol,
/*     91    98 */       XK_KP_Divide,  NoSymbol,
/*     92    99 */       XK_Sys_Req,    XK_Print,
/*     93   100 */       XK_Alt_R,      XK_Meta_R,
/*     94   101 */       XK_Num_Lock,   NoSymbol,
/*     95   102 */       XK_Home,       NoSymbol,
/*     96   103 */       XK_Up,         NoSymbol,
/*     97   104 */       XK_Page_Up,    NoSymbol,
/*     98   105 */       XK_Left,       NoSymbol,
/*     99   106 */       XK_Right,      NoSymbol,
/*    100   107 */       XK_End,        NoSymbol,
/*    101   108 */       XK_Down,       NoSymbol,
/*    102   109 */       XK_Page_Down,  NoSymbol,
/*    103   110 */       XK_Insert,     NoSymbol,
/*    104   111 */       XK_Delete,     NoSymbol,
/*    105   112 */       XK_Super_L,    NoSymbol,
/*    106   113 */       XK_Super_R,    NoSymbol,
/*    107   114 */       XK_Menu,       NoSymbol,
/*    108   115 */       NoSymbol,      NoSymbol,
/*    109   116 */       NoSymbol,      NoSymbol,
/*    110   117 */       NoSymbol,      NoSymbol,
/*    111   118 */       NoSymbol,      NoSymbol,
/*    112   119 */       NoSymbol,      NoSymbol,
};

static void
EvdevRead1 (int evdevPort, void *closure)
{
    int i, n, xk;
    struct input_event  events[NUM_EVENTS];

    n = read (evdevPort, &events, NUM_EVENTS * sizeof (struct input_event));
    if (n <= 0)
        return;
    n /= sizeof (struct input_event);
    for (i = 0; i < n; i++)
    {
        switch (events[i].type) {
        case EV_SYN:
            break;
        case EV_KEY:
            xk = events[i].code;
            if (events[i].code < 0x100)
                ErrorF ("key %d %d xk %d\n", events[i].code, events[i].value, xk);
            else
                ErrorF ("key 0x%x %d xk %d\n", events[i].code, events[i].value, xk);
            if (events[i].value == 2) {
                //KdEnqueueKeyboardEvent (xk, 0);
                KdEnqueueKeyboardEvent (xk, 0);
            } else
                KdEnqueueKeyboardEvent (xk, !events[i].value);
            break;
        }
    }
}

char *kdefaultEvdev1[] =  {
    "/dev/input/event0",
    //    "/dev/input/event1",
    //    "/dev/input/event2",
    //    "/dev/input/event3",
    //    "/dev/input/event4",
    //    "/dev/input/event5",
};

#define NUM_DEFAULT_EVDEV1    (sizeof (kdefaultEvdev1) / sizeof (kdefaultEvdev1[0]))

static Bool
EvdevKbdInit (void)
{
    int         i;
    int         fd;
    int         n = 0;
    char        name[100];

    if (!EvdevInputType)
        EvdevInputType = KdAllocInputType ();

    for (i = 0; i < NUM_DEFAULT_EVDEV; i++)
    {
        fd = open (kdefaultEvdev1[i], 2);
        if (fd >= 0)
        {
            ioctl(fd, EVIOCGRAB, 1);

            ioctl(fd, EVIOCGNAME(sizeof(name)), name);
            ErrorF("Name is %s\n", name);
            ioctl(fd, EVIOCGPHYS(sizeof(name)), name);
            ErrorF("Phys Loc is %s\n", name);
            ioctl(fd, EVIOCGUNIQ(sizeof(name)), name);
            ErrorF("Unique is %s\n", name);

        }

        if (fd >= 0)
        {
            unsigned long   ev[NBITS(EV_MAX)];
            Kevdev          *ke;

            if (ioctl (fd, EVIOCGBIT(0 /*EV*/, sizeof (ev)), ev) < 0)
            {
                perror ("EVIOCGBIT 0");
                close (fd);
                continue;
            }
            ke = xalloc (sizeof (Kevdev));
            if (!ke)
            {
                close (fd);
                continue;
            }
            memset (ke, '\0', sizeof (Kevdev));
            if (ISBITSET (ev, EV_KEY))
            {
                if (ioctl (fd, EVIOCGBIT (EV_KEY, sizeof (ke->keybits)),
                           ke->keybits) < 0)
                {
                    perror ("EVIOCGBIT EV_KEY");
                    xfree (ke);
                    close (fd);
                    continue;
                }
            }
            if (ISBITSET (ev, EV_REL))
            {
                if (ioctl (fd, EVIOCGBIT (EV_REL, sizeof (ke->relbits)),
                           ke->relbits) < 0)
                {
                    perror ("EVIOCGBIT EV_REL");
                    xfree (ke);
                    close (fd);
                    continue;
                }
                for (ke->max_rel = REL_MAX; ke->max_rel >= 0; ke->max_rel--)
                    if (ISBITSET(ke->relbits, ke->max_rel))
                        break;
            }
            if (ISBITSET (ev, EV_ABS))
            {
                int i;

                if (ioctl (fd, EVIOCGBIT (EV_ABS, sizeof (ke->absbits)),
                           ke->absbits) < 0)
                {
                    perror ("EVIOCGBIT EV_ABS");
                    xfree (ke);
                    close (fd);
                    continue;
                }
                for (ke->max_abs = ABS_MAX; ke->max_abs >= 0; ke->max_abs--)
                    if (ISBITSET(ke->absbits, ke->max_abs))
                        break;
                for (i = 0; i <= ke->max_abs; i++)
                {
                    if (ISBITSET (ke->absbits, i))
                        if (ioctl (fd, EVIOCGABS(i), &ke->absinfo[i]) < 0)
                        {
                            perror ("EVIOCGABS");
                            break;
                        }
                    ke->prevabs[i] = ABS_UNSET;
                }
                if (i <= ke->max_abs)
                {
                    xfree (ke);
                    close (fd);
                    continue;
                }
            }
            if (KdRegisterFd (EvdevInputType, fd, EvdevRead1, NULL))
                n++;
        }
    }
    return TRUE;
}

static void EvdevKbdLoad(void)
{
    kdMinScanCode = 1;
    kdMaxScanCode = 112;
    kdKeymapWidth = 2;
    memcpy (kdKeymap, evdevKeymap, sizeof (evdevKeymap));
}

static void
EvdevKbdFini (void)
{
}

static void
EvdevKbdLeds (int leds)
{
}


static void EvdevKbdBell(int volume, int pitch, int duration)
{
}


KdKeyboardFuncs LinuxEvdevKeyboardFuncs = {
    EvdevKbdLoad,
    EvdevKbdInit,
    EvdevKbdLeds,
    EvdevKbdBell,
    EvdevKbdFini,
    0,
};
