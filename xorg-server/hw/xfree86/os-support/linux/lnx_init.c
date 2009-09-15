/*
 * Copyright 1992 by Orest Zborowski <obz@Kodak.com>
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Orest Zborowski and David Wexelblat
 * not be used in advertising or publicity pertaining to distribution of
 * the software without specific, written prior permission.  Orest Zborowski
 * and David Wexelblat make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express or
 * implied warranty.
 *
 * OREST ZBOROWSKI AND DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL OREST ZBOROWSKI OR DAVID WEXELBLAT BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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
#include "lnx.h"

#include <sys/stat.h>

static Bool KeepTty = FALSE;
static int VTnum = -1;
static Bool VTSwitch = TRUE;
static Bool ShareVTs = FALSE;
static int activeVT = -1;

static int vtPermSave[4];
static char vtname[11];
static struct termios tty_attr; /* tty state to restore */
static int tty_mode; /* kbd mode to restore */

static int
saveVtPerms(void)
{
    /* We need to use stat to get permissions. */
    struct stat svtp;

    /* Do them numerically ordered, hard coded tty0 first. */
    if (stat("/dev/tty0", &svtp) != 0)
	return 0;
    vtPermSave[0] = (int)svtp.st_uid;
    vtPermSave[1] = (int)svtp.st_gid;

    /* Now check the console we are dealing with. */
    if (stat(vtname, &svtp) != 0)
	return 0;
    vtPermSave[2] = (int)svtp.st_uid;
    vtPermSave[3] = (int)svtp.st_gid;

    return 1;
}

static void
restoreVtPerms(void)
{
    if (geteuid() == 0) {
	 /* Set the terminal permissions back to before we started. */
	 (void)chown("/dev/tty0", vtPermSave[0], vtPermSave[1]);
	 (void)chown(vtname, vtPermSave[2], vtPermSave[3]);
    }
}

static void *console_handler;

static void
drain_console(int fd, void *closure)
{
    tcflush(fd, TCIOFLUSH);
}

void
xf86OpenConsole(void)
{
    int i, fd = -1;
    struct vt_mode VT;
    struct vt_stat vts;
    MessageType from = X_PROBED;
    char *tty0[] = { "/dev/tty0", "/dev/vc/0", NULL };
    char *vcs[] = { "/dev/vc/%d", "/dev/tty%d", NULL };

    if (serverGeneration == 1) {

	/* when KeepTty check if we're run with euid==0 */
	if (KeepTty && geteuid() != 0) 
	    FatalError("xf86OpenConsole:"
		       " Server must be suid root for option \"KeepTTY\"\n");

	/*
	 * setup the virtual terminal manager
	 */
	if (VTnum != -1) {
	    xf86Info.vtno = VTnum;
	    from = X_CMDLINE;
	} else {

	    i=0;
	    while (tty0[i] != NULL) {
		if ((fd = open(tty0[i],O_WRONLY,0)) >= 0)
		  break;
		i++;
	    }
	    
	    if (fd < 0)
		FatalError(
		    "xf86OpenConsole: Cannot open /dev/tty0 (%s)\n",
		    strerror(errno));

            if (ShareVTs)
            {
                if (ioctl(fd, VT_GETSTATE, &vts) == 0)
                    xf86Info.vtno = vts.v_active;
                else
                    FatalError("xf86OpenConsole: Cannot find the current"
                               " VT (%s)\n", strerror(errno));
            } else {
	        if ((ioctl(fd, VT_OPENQRY, &xf86Info.vtno) < 0) ||
		    (xf86Info.vtno == -1))
		    FatalError("xf86OpenConsole: Cannot find a free VT: %s\n",
                               strerror(errno));
            }
	    close(fd);
	}

	xf86Msg(from, "using VT number %d\n\n", xf86Info.vtno);

	if (!KeepTty) {
	    pid_t ppid = getppid();
	    pid_t ppgid;
	    ppgid = getpgid(ppid);

	    /*
	     * change to parent process group that pgid != pid so
	     * that setsid() doesn't fail and we become process
	     * group leader
	     */
	    if (setpgid(0,ppgid) < 0)
		xf86Msg(X_WARNING, "xf86OpenConsole: setpgid failed: %s\n",
			strerror(errno));

	    /* become process group leader */
	    if ((setsid() < 0))
		xf86Msg(X_WARNING, "xf86OpenConsole: setsid failed: %s\n",
			strerror(errno));
	}

        i=0;
        while (vcs[i] != NULL) {
            sprintf(vtname, vcs[i], xf86Info.vtno); /* /dev/tty1-64 */
     	    if ((xf86Info.consoleFd = open(vtname, O_RDWR|O_NDELAY, 0)) >= 0)
		break;
            i++;
        }

	if (xf86Info.consoleFd < 0)
	    FatalError("xf86OpenConsole: Cannot open virtual console"
		       " %d (%s)\n", xf86Info.vtno, strerror(errno));

        if (!ShareVTs)
        {
	    /*
	     * Grab the vt ownership before we overwrite it.
	     * Hard coded /dev/tty0 into this function as well for below.
	     */
	    if (!saveVtPerms())
	        xf86Msg(X_WARNING,
		        "xf86OpenConsole: Could not save ownership of VT\n");

	    if (geteuid() == 0) {
		    /* change ownership of the vt */
		    if (chown(vtname, getuid(), getgid()) < 0)
			    xf86Msg(X_WARNING,"xf86OpenConsole: chown %s failed: %s\n",
				    vtname, strerror(errno));

		    /*
		     * the current VT device we're running on is not
		     * "console", we want to grab all consoles too
		     *
		     * Why is this needed??
		     */
		    if (chown("/dev/tty0", getuid(), getgid()) < 0)
			    xf86Msg(X_WARNING,"xf86OpenConsole: chown /dev/tty0 failed: %s\n",
				    strerror(errno));
	    }
        }

	/*
	 * Linux doesn't switch to an active vt after the last close of a vt,
	 * so we do this ourselves by remembering which is active now.
	 */
	if (ioctl(xf86Info.consoleFd, VT_GETSTATE, &vts) < 0)
	    xf86Msg(X_WARNING,"xf86OpenConsole: VT_GETSTATE failed: %s\n",
		    strerror(errno));
	else
	    activeVT = vts.v_active;

#if 0
	if (!KeepTty) {
	    /*
	     * Detach from the controlling tty to avoid char loss
	     */
	    if ((i = open("/dev/tty",O_RDWR)) >= 0) {
		ioctl(i, TIOCNOTTY, 0);
		close(i);
	    }
	}
#endif

        if (!ShareVTs)
        {
            struct termios nTty;

#if defined(DO_OS_FONTRESTORE)
	    lnx_savefont();
#endif
	    /*
	     * now get the VT.  This _must_ succeed, or else fail completely.
	     */
	    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) < 0)
	        FatalError("xf86OpenConsole: VT_ACTIVATE failed: %s\n",
		           strerror(errno));

	    if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, xf86Info.vtno) < 0)
	        FatalError("xf86OpenConsole: VT_WAITACTIVE failed: %s\n",
			   strerror(errno));

	    if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) < 0)
	        FatalError("xf86OpenConsole: VT_GETMODE failed %s\n",
		           strerror(errno));

	    signal(SIGUSR1, xf86VTRequest);

	    VT.mode = VT_PROCESS;
	    VT.relsig = SIGUSR1;
	    VT.acqsig = SIGUSR1;

	    if (ioctl(xf86Info.consoleFd, VT_SETMODE, &VT) < 0)
	        FatalError("xf86OpenConsole: VT_SETMODE VT_PROCESS failed: %s\n",
		    strerror(errno));
	
	    if (ioctl(xf86Info.consoleFd, KDSETMODE, KD_GRAPHICS) < 0)
	        FatalError("xf86OpenConsole: KDSETMODE KD_GRAPHICS failed %s\n",
		           strerror(errno));

            tcgetattr(xf86Info.consoleFd, &tty_attr);
            ioctl(xf86Info.consoleFd, KDGKBMODE, &tty_mode);

            if (ioctl(xf86Info.consoleFd, KDSKBMODE, K_RAW) < 0)
                FatalError("xf86OpenConsole: KDSKBMODE K_RAW failed %s\n",
                        strerror(errno));

            nTty = tty_attr;
            nTty.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
            nTty.c_oflag = 0;
            nTty.c_cflag = CREAD | CS8;
            nTty.c_lflag = 0;
            nTty.c_cc[VTIME]=0;
            nTty.c_cc[VMIN]=1;
            cfsetispeed(&nTty, 9600);
            cfsetospeed(&nTty, 9600);
            tcsetattr(xf86Info.consoleFd, TCSANOW, &nTty);

            /* need to keep the buffer clean, else the kernel gets angry */
            console_handler = xf86AddGeneralHandler(xf86Info.consoleFd,
                    drain_console, NULL);

	    /* we really should have a InitOSInputDevices() function instead
	     * of Init?$#*&Device(). So I just place it here */
        }
    } else { 	/* serverGeneration != 1 */
        if (!ShareVTs && VTSwitch)
        {
	    /*
	     * now get the VT
	     */
	    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) < 0)
	        xf86Msg(X_WARNING, "xf86OpenConsole: VT_ACTIVATE failed %s\n",
		        strerror(errno));

	    if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, xf86Info.vtno) < 0)
	        xf86Msg(X_WARNING, "xf86OpenConsole: VT_WAITACTIVE failed %s\n",
		        strerror(errno));
        }
    }
    return;
}

void
xf86CloseConsole(void)
{
    struct vt_mode   VT;
#if defined(DO_OS_FONTRESTORE)
    struct vt_stat vts;
    int vtno = -1;
#endif

    if (ShareVTs) {
        close(xf86Info.consoleFd);
        return;
    }

    if (console_handler) {
	xf86RemoveGeneralHandler(console_handler);
	console_handler = NULL;
    };

#if defined(DO_OS_FONTRESTORE)
    if (ioctl(xf86Info.consoleFd, VT_GETSTATE, &vts) < 0)
	xf86Msg(X_WARNING, "xf86CloseConsole: VT_GETSTATE failed: %s\n",
		strerror(errno));
    else
	vtno = vts.v_active;
#endif

    /* Back to text mode ... */
    if (ioctl(xf86Info.consoleFd, KDSETMODE, KD_TEXT) < 0)
	xf86Msg(X_WARNING, "xf86CloseConsole: KDSETMODE failed: %s\n",
		strerror(errno));

    ioctl(xf86Info.consoleFd, KDSKBMODE, tty_mode);
    tcsetattr(xf86Info.consoleFd, TCSANOW, &tty_attr);

    if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) < 0) 
	xf86Msg(X_WARNING, "xf86CloseConsole: VT_GETMODE failed: %s\n",
		strerror(errno));
    else {
	/* set dflt vt handling */
	VT.mode = VT_AUTO;
	if (ioctl(xf86Info.consoleFd, VT_SETMODE, &VT) < 0) 
	    xf86Msg(X_WARNING, "xf86CloseConsole: VT_SETMODE failed: %s\n",
		    strerror(errno));
    }

    if (VTSwitch)
    {
        /*
         * Perform a switch back to the active VT when we were started
         */
        if (activeVT >= 0) {
	    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, activeVT) < 0)
	        xf86Msg(X_WARNING, "xf86CloseConsole: VT_ACTIVATE failed: %s\n",
		        strerror(errno));
	    if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, activeVT) < 0)
		xf86Msg(X_WARNING,
			"xf86CloseConsole: VT_WAITACTIVE failed: %s\n",
			strerror(errno));
	    activeVT = -1;
        }

#if defined(DO_OS_FONTRESTORE)
        if (xf86Info.vtno == vtno)	/* check if we are active */
	    lnx_restorefont();
        lnx_freefontdata();
#endif
    }
    close(xf86Info.consoleFd);	/* make the vt-manager happy */

    restoreVtPerms();		/* restore the permissions */

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
        if (!strcmp(argv[i], "-novtswitch"))
        {
                VTSwitch = FALSE;
                return(1);
        }
        if (!strcmp(argv[i], "-sharevts"))
        {
                ShareVTs = TRUE;
                return(1);
        }
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
xf86UseMsg(void)
{
	ErrorF("vtXX                   use the specified VT number\n");
	ErrorF("-keeptty               ");
	ErrorF("don't detach controlling tty (for debugging only)\n");
        ErrorF("-novtswitch            don't immediately switch to new VT\n");
        ErrorF("-sharevts              share VTs with another X server\n");
	return;
}
