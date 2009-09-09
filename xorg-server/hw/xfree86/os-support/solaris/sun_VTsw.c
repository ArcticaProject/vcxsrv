/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * of the copyright holder.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#include <door.h>
#include <sys/vtdaemon.h>

/*
 * Handle the VT-switching interface for Solaris/OpenSolaris
 */

void
xf86VTRequest(int sig)
{
	if (xf86Info.vtPendingNum != -1)
	{
		ioctl(xf86Info.consoleFd, VT_RELDISP, 1);
		xf86Info.vtPendingNum = -1;

		return;
	}

	xf86Info.vtRequestsPending = TRUE;
	return;
}

Bool
xf86VTSwitchPending(void)
{
    return(xf86Info.vtRequestsPending ? TRUE : FALSE);
}

Bool
xf86VTSwitchAway(void)
{
	int door_fd;
	vt_cmd_arg_t vt_door_arg;
	door_arg_t door_arg;

	xf86Info.vtRequestsPending = FALSE;

	vt_door_arg.vt_ev = VT_EV_HOTKEYS;
	vt_door_arg.vt_num = xf86Info.vtPendingNum;
	door_arg.data_ptr = (char *)&vt_door_arg;
	door_arg.data_size = sizeof (vt_cmd_arg_t);
	door_arg.rbuf = NULL;
	door_arg.rsize = 0;
	door_arg.desc_ptr = NULL;
	door_arg.desc_num = 0;

	if ((door_fd = open(VT_DAEMON_DOOR_FILE, O_RDONLY)) < 0)
		return (FALSE);

	if (door_call(door_fd, &door_arg) != 0) {
		close(door_fd);
		return (FALSE);
	}

	close(door_fd);
	return (TRUE);
}

Bool
xf86VTSwitchTo(void)
{
	xf86Info.vtRequestsPending = FALSE;
	if (ioctl(xf86Info.consoleFd, VT_RELDISP, VT_ACKACQ) < 0)
	{
		return(FALSE);
	}
	else
	{
		return(TRUE);
	}
}
