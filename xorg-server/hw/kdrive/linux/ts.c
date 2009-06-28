/*
 * Derived from ps2.c by Jim Gettys
 *
 * Copyright © 1999 Keith Packard
 * Copyright © 2000 Compaq Computer Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard or Compaq not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard and Compaq makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD AND COMPAQ DISCLAIM ALL WARRANTIES WITH REGARD TO THIS 
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, 
 * IN NO EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#define NEED_EVENTS
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xpoll.h>
#include "inputstr.h"
#include "scrnintstr.h"
#include "kdrive.h"
#include <sys/ioctl.h>
#include <linux/h3600_ts.h>	/* touch screen events */

static long lastx = 0, lasty = 0;

int KdTsPhyScreen = 0;

static int
TsReadBytes (int fd, char *buf, int len, int min)
{
    int		    n, tot;
    fd_set	    set;
    struct timeval  tv;

    tot = 0;
    while (len)
    {
	n = read (fd, buf, len);
	if (n > 0)
	{
	    tot += n;
	    buf += n;
	    len -= n;
	}
	if (tot % min == 0)
	    break;
	FD_ZERO (&set);
	FD_SET (fd, &set);
	tv.tv_sec = 0;
	tv.tv_usec = 100 * 1000;
	n = select (fd + 1, &set, 0, 0, &tv);
	if (n <= 0)
	    break;
    }
    return tot;
}

static void
TsRead (int tsPort, void *closure)
{
    KdPointerInfo    *pi = closure;
    TS_EVENT	    event;
    int		    n;
    long	    x, y;
    unsigned long   flags;

    n = TsReadBytes (tsPort, (char *) &event, sizeof (event), sizeof (event));
    if (n == sizeof (event))  
    {
	if (event.pressure) 
	{
	    /* 
	     * HACK ATTACK.  (static global variables used !)
	     * Here we test for the touch screen driver actually being on the
	     * touch screen, if it is we send absolute coordinates. If not,
	     * then we send delta's so that we can track the entire vga screen.
	     */
	    if (KdCurScreen == KdTsPhyScreen) {
	    	flags = KD_BUTTON_1;
	    	x = event.x;
	    	y = event.y;
	    }
	    else
	      {
	    	flags = /* KD_BUTTON_1 |*/ KD_MOUSE_DELTA;
	    	if ((lastx == 0) || (lasty == 0)) {
	    	    x = 0;
	    	    y = 0;
	    	} else {
	    	    x = event.x - lastx;
	    	    y = event.y - lasty;
	    	}
	    	lastx = event.x;
	    	lasty = event.y;
	      }
	} else {
	    flags = KD_MOUSE_DELTA;
	    x = 0;
	    y = 0;
	    lastx = 0;
	    lasty = 0;
	}
	KdEnqueuePointerEvent (pi, flags, x, y, 0);
    }
}

char	*TsNames[] = {
  "/dev/ts",	
  "/dev/h3600_ts" /* temporary name; note this code can try
			   to open more than one device */
};

#define NUM_TS_NAMES	(sizeof (TsNames) / sizeof (TsNames[0]))

static Status
TsInit (KdPointerInfo *pi)
{
    int		i;
    int		fd;
    int		n = 0;

    if (!pi->path || strcmp(pi->path, "auto") == 0) {
        for (i = 0; i < NUM_TS_NAMES; i++)    {
            fd = open (TsNames[i], 0);
            if (fd >= 0) {
                pi->path = KdSaveString (TsNames[i]);
                break;
            }
	}
    }
    else {
        fd = open (pi->path, 0);
    }

    if (fd < 0) {
        ErrorF("TsInit: Couldn't open %s\n", pi->path);
        return BadMatch;
    }
    close(fd);

    pi->name = KdSaveString("H3600 Touchscreen");

    return Success;
}

static Status
TsEnable (KdPointerInfo *pi)
{
    int fd;

    if (!pi || !pi->path)
        return BadImplementation;

    fd = open(pi->path, 0);

    if (fd < 0) {
        ErrorF("TsInit: Couldn't open %s\n", pi->path);
        return BadMatch;
    }

    struct h3600_ts_calibration cal;
    /*
     * Check to see if this is a touch screen
     */
    if (ioctl (fd, TS_GET_CAL, &cal) != -1) {
	mi->driverPrivate = (void *) fd;
	if (!KdRegisterFd (fd, TsRead, (void *) mi)) {
            close(fd);
            return BadAlloc;
	}
    }
    else {
        ErrorF("TsEnable: %s is not a touchscreen\n", pi->path);
	close (fd);
        return BadMatch;
    }

    return Success;
}

static void
TsFini (KdPointerInfo *pi)
{
    KdUnregisterFds (pi, (int)pi->driverPrivate, TRUE);
    mi->driverPrivate = NULL;
}

KdPointerDriver TsDriver = {
    TsInit,
    TsEnable,
    TsDisable,
    TsFini,
    NULL,
};
