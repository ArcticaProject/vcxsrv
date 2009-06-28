/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Thomas Roell and
 * David Wexelblat makes no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THOMAS ROELL AND DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL THOMAS ROELL OR DAVID WEXELBLAT BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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

static Bool KeepTty = FALSE;
#ifdef SVR4
static Bool Protect0 = FALSE;
#endif
static int VTnum = -1;


void
xf86OpenConsole()
{
    int fd;
    struct vt_mode VT;
    char vtname1[10],vtname2[10];
    MessageType from = X_PROBED;

    if (serverGeneration == 1) 
    {
    	/* check if we're run with euid==0 */
    	if (geteuid() != 0)
	{
      	    FatalError("xf86OpenConsole: Server must be suid root\n");
	}

#ifdef SVR4
	/* Protect page 0 to help find NULL dereferencing */
	/* mprotect() doesn't seem to work */
	if (Protect0)
	{
	    int fd = -1;

	    if ((fd = open("/dev/zero", O_RDONLY, 0)) < 0)
	    {
		xf86Msg(X_WARNING,
			"xf86OpenConsole: cannot open /dev/zero (%s)\n",
			strerror(errno));
	    }
	    else
	    {
		if ((int)mmap(0, 0x1000, PROT_NONE,
			      MAP_FIXED | MAP_SHARED, fd, 0) == -1)
		{
		    xf86Msg(X_WARNING,
			"xf86OpenConsole: failed to protect page 0 (%s)\n",
			strerror(errno));
		}
		close(fd);
	    }
	}
#endif
    	/*
     	 * setup the virtual terminal manager
     	 */
    	if (VTnum != -1) 
	{
      	    xf86Info.vtno = VTnum;
	    from = X_CMDLINE;
    	}
    	else 
	{
      	    if ((fd = open("/dev/console",O_WRONLY,0)) < 0) 
	    {
        	FatalError(
		    "xf86OpenConsole: Cannot open /dev/console (%s)\n",
		    strerror(errno));
	    }
      	    if ((ioctl(fd, VT_OPENQRY, &xf86Info.vtno) < 0) || 
		(xf86Info.vtno == -1))
	    {
        	FatalError("xf86OpenConsole: Cannot find a free VT\n");
	    }
           close(fd);
        }
	xf86Msg(from, "using VT number %d\n\n", xf86Info.vtno);

	sprintf(vtname1,"/dev/vc%02d",xf86Info.vtno); /* ESIX */
	sprintf(vtname2,"/dev/vt%02d",xf86Info.vtno); /* rest of the world */

	if (!KeepTty)
    	{
    	    setpgrp();
	}

	if (((xf86Info.consoleFd = open(vtname1, O_RDWR|O_NDELAY, 0)) < 0) &&
	    ((xf86Info.consoleFd = open(vtname2, O_RDWR|O_NDELAY, 0)) < 0))
	{
            FatalError("xf86OpenConsole: Cannot open %s (%s) (%s)\n",
		       vtname2, vtname1, strerror(errno));
	}

	/* change ownership of the vt */
	if (chown(vtname1, getuid(), getgid()) < 0)
	{
            chown(vtname2, getuid(), getgid());
	}

	/*
	 * now get the VT
	 */
	if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0)
	{
    	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_ACTIVATE failed\n");
	}
	if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, xf86Info.vtno) != 0)
	{
	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_WAITACTIVE failed\n");
	}
	if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) < 0) 
	{
	    FatalError("xf86OpenConsole: VT_GETMODE failed\n");
	}

	signal(SIGUSR1, xf86VTRequest);

	VT.mode = VT_PROCESS;
	VT.relsig = SIGUSR1;
	VT.acqsig = SIGUSR1;
	if (ioctl(xf86Info.consoleFd, VT_SETMODE, &VT) < 0) 
	{
	    FatalError("xf86OpenConsole: VT_SETMODE VT_PROCESS failed\n");
	}
	if (ioctl(xf86Info.consoleFd, KDSETMODE, KD_GRAPHICS) < 0)
	{
	    FatalError("xf86OpenConsole: KDSETMODE KD_GRAPHICS failed\n");
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
	if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, xf86Info.vtno) != 0)
	{
      	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_WAITACTIVE failed\n");
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
    ioctl(xf86Info.consoleFd, KDSETMODE, KD_TEXT);  /* Back to text mode ... */
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
	/*
	 * Keep server from detaching from controlling tty.  This is useful 
	 * when debugging (so the server can receive keyboard signals.
	 */
	if (!strcmp(argv[i], "-keeptty"))
	{
		KeepTty = TRUE;
		return(1);
	}
#ifdef SVR4
	/*
	 * Undocumented flag to protect page 0 from read/write to help
	 * catch NULL pointer dereferences.  This is purely a debugging
	 * flag.
	 */
	if (!strcmp(argv[i], "-protect0"))
	{
		Protect0 = TRUE;
		return(1);
	}
#endif
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
	ErrorF("-keeptty               ");
	ErrorF("don't detach controlling tty (for debugging only)\n");
	return;
}
