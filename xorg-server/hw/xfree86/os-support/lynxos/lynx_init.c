/*
 * Copyright 1993 by Thomas Mueller
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Mueller not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Mueller makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS MUELLER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS MUELLER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */



#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

static int VTnum = -1;

void
xf86OpenConsole()
{
    struct vt_mode VT;
    char vtname1[11];
    int fd, pgrp;
    MessageType from = X_PROBED;

    if (serverGeneration == 1) 
    {
	/* check if we're run with euid==0 */
	if (geteuid() != 0)
	{
	    FatalError("xf86OpenConsole: Server must be suid root\n");
	}

    	/*
     	 * setup the virtual terminal manager
     	 * NOTE:
     	 *   We use the out-of-the-box atc terminal driver,
     	 *   not the GE contributed vdt driver.
     	 *   Also, we do setup signals for VT switching which
     	 *   is not really necessary because we don't feed the
     	 *   VT switch keystrokes to the kernel in xf86Events.c
     	 *   (it bombs occasionally...)
     	 */
    	if (VTnum != -1) 
	{
      	    xf86Info.vtno = VTnum;
	    from = X_CMDLINE;
    	}
    	else 
	{
	    /* We could use /dev/con which is usually a symlink
	     * to /dev/atc0 but one could configure the system
	     * to use a serial line as console device, so to 
	     * be sure we take /dev/atc0.
	     */
      	    if ((fd = open("/dev/atc0",O_WRONLY,0)) < 0) 
	    {
        	FatalError(
		    "xf86OpenConsole: Cannot open /dev/atc0 (%s)\n",
		    strerror(errno));
	    }
      	    if ((ioctl(fd, VT_OPENQRY, &xf86Info.vtno) < 0) || 
		(xf86Info.vtno == -1))
	    {
        	FatalError("xf86OpenConsole: Cannot find a free VT\n");
	    }
           close(fd);
        }
	xf86Msg(from, "using VT number %d\n", xf86Info.vtno);

	sprintf(vtname1,"/dev/atc%d",xf86Info.vtno);

	pgrp = getpgrp();		/* POSIX version ! */
	ioctl(xf86Info.consoleFd, TIOCSPGRP, &pgrp);

	if ((xf86Info.consoleFd = open(vtname1,O_RDWR|O_NDELAY,0)) < 0) 
	{
		FatalError(
		    "xf86OpenConsole: Cannot open %s (%s)\n",
		    vtname1, strerror(errno));
	}
	/* change ownership of the vt */
	chown(vtname1, getuid(), getgid());

	/*
	 * now get the VT
	 */
	if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0)
	{
    	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_ACTIVATE failed\n");
	}
	if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) < 0) 
	{
	    FatalError("xf86OpenConsole: VT_GETMODE failed\n");
	}

	/* for future use... */
	signal(SIGUSR1, xf86VTRequest);

	VT.mode = VT_PROCESS;
	VT.relsig = SIGUSR1;
	VT.acqsig = SIGUSR1;
	if (ioctl(xf86Info.consoleFd, VT_SETMODE, &VT) < 0) 
	{
	    FatalError("xf86OpenConsole: VT_SETMODE VT_PROCESS failed\n");
	}
    }
    else
    {
	/* serverGeneration != 1 */
	/*
	 * now get the VT
	 */
	if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0)
	{
	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_ACTIVATE failed\n");
	}
	/*
	 * If the server doesn't have the VT when the reset occurs,
	 * this is to make sure we don't continue until the activate
	 * signal is received.
	 */
	if (!xf86Screens[0]->vtSema)
	    sleep(5);
    }
    return;
}

void
xf86CloseConsole()
{
    struct vt_mode   VT;

#if 0
    ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno);
    ioctl(xf86Info.consoleFd, VT_WAITACTIVE, 0);
#endif
    if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) != -1)
    {
	VT.mode = VT_AUTO;
	ioctl(xf86Info.consoleFd, VT_SETMODE, &VT); /* set dflt vt handling */
    }
    close(xf86Info.consoleFd);                 /* make the vt-manager happy */
    return;
}

int
xf86ProcessArgument(int argc, char *argv[], int i)
{
	if ((argv[i][0] == 'v') && (argv[i][1] == 't'))
	{
		if (sscanf(argv[i], "vt%2d", &VTnum) == 0)
		{
			UseMsg();
			VTnum = -1;
			return(0);
		}
		return(1);
	}
	return(0);
}

void
xf86UseMsg()
{
	ErrorF("vtXX                   use the specified VT number\n");
	return;
}

