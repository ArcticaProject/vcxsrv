/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 * Copyright 1999 by David Holland <davidh@iquest.net>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the names of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, AND IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#ifdef HAVE_SYS_KD_H
# include <sys/kd.h>
#endif

static Bool KeepTty = FALSE;
static Bool Protect0 = FALSE;
#ifdef HAS_USL_VTS
static int VTnum = -1;
static int xf86StartVT = -1;
#endif

#if defined(__SOL8__) || (!defined(__i386__) && !defined(__i386))
static char fb_dev[PATH_MAX] = "/dev/fb";
#else
static char fb_dev[PATH_MAX] = "/dev/console";
#endif

void
xf86OpenConsole(void)
{
    int i;
#ifdef HAS_USL_VTS
    int fd;
    struct vt_mode VT;
    struct vt_stat vtinfo;
    int FreeVTslot;
    MessageType from = X_PROBED;
#endif

    if (serverGeneration == 1)
    {
	/* Check if we're run with euid==0 */
	if (geteuid() != 0)
	    FatalError("xf86OpenConsole: Server must be suid root\n");

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
		    xf86Msg(X_WARNING,
			"xf86OpenConsole: failed to protect page 0 (%s)\n",
			strerror(errno));

		close(fd);
	    }
	}

#ifdef HAS_USL_VTS

	/*
	 * Setup the virtual terminal manager
	 */
	if (VTnum != -1)
	{
	    xf86Info.vtno = VTnum;
	    from = X_CMDLINE;
	}
	else
	{
	    if ((fd = open("/dev/vt00",O_RDWR,0)) < 0)
		FatalError("xf86OpenConsole: Cannot open /dev/vt00 (%s)\n",
		    strerror(errno));

	    if (ioctl(fd, VT_GETSTATE, &vtinfo) < 0)
		FatalError("xf86OpenConsole: Cannot determine current VT\n");

	    xf86StartVT = vtinfo.v_active;

	    /*
	     * There is a SEVERE problem with x86's VT's.  The VT_OPENQRY
	     * ioctl() will panic the entire system if all 8 (7 VT's+Console)
	     * terminals are used.  The only other way I've found to determine
	     * if there is a free VT is to try activating all the the available
	     * VT's and see if they all succeed - if they do, there there is no
	     * free VT, and the Xserver cannot continue without panic'ing the
	     * system.  (It's ugly, but it seems to work.)  Note there is a
	     * possible race condition here.
	     *
	     * David Holland 2/23/94
	     */

	    FreeVTslot = 0;
	    for (i = 7; (i >= 0) && !FreeVTslot; i--)
		if (ioctl(fd, VT_ACTIVATE, i) != 0)
		    FreeVTslot = 1;

	    if (!FreeVTslot ||
	        (ioctl(fd, VT_OPENQRY, &xf86Info.vtno) < 0) ||
		(xf86Info.vtno == -1))
		FatalError("xf86OpenConsole: Cannot find a free VT\n");

	    close(fd);
	}

	xf86Msg(from, "using VT number %d\n\n", xf86Info.vtno);

	sprintf(fb_dev, "/dev/vt%02d", xf86Info.vtno); /* Solaris 2.1 x86 */

#endif /* HAS_USL_VTS */

	if (!KeepTty)
	    setpgrp();

	if (((xf86Info.consoleFd = open(fb_dev, O_RDWR | O_NDELAY, 0)) < 0))
	    FatalError("xf86OpenConsole: Cannot open %s (%s)\n",
		       fb_dev, strerror(errno));

#ifdef HAS_USL_VTS

	/* Change ownership of the vt */
	chown(fb_dev, getuid(), getgid());

	/*
	 * Now get the VT
	 */
	if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0)
	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_ACTIVATE failed\n");

	if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, xf86Info.vtno) != 0)
	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_WAITACTIVE failed\n");

	if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) < 0)
	    FatalError("xf86OpenConsole: VT_GETMODE failed\n");

	signal(SIGUSR1, xf86VTRequest);

	VT.mode = VT_PROCESS;
	VT.relsig = SIGUSR1;
	VT.acqsig = SIGUSR1;

	if (ioctl(xf86Info.consoleFd, VT_SETMODE, &VT) < 0)
	    FatalError("xf86OpenConsole: VT_SETMODE VT_PROCESS failed\n");
#endif

#ifdef KDSETMODE
	SYSCALL(i = ioctl(xf86Info.consoleFd, KDSETMODE, KD_GRAPHICS));
	if (i < 0) {
	    xf86Msg(X_WARNING,
		    "xf86OpenConsole: KDSETMODE KD_GRAPHICS failed on %s (%s)\n",
		    fb_dev, strerror(errno));
	}
#endif
    }
    else /* serverGeneration != 1 */
    {
#ifdef HAS_USL_VTS
	/*
	 * Now re-get the VT
	 */
	if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0)
	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_ACTIVATE failed\n");

	if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, xf86Info.vtno) != 0)
	    xf86Msg(X_WARNING, "xf86OpenConsole: VT_WAITACTIVE failed\n");

	/*
	 * If the server doesn't have the VT when the reset occurs,
	 * this is to make sure we don't continue until the activate
	 * signal is received.
	 */
	if (!xf86Screens[0]->vtSema)
	    sleep(5);

#endif /* HAS_USL_VTS */

    }
}

void
xf86CloseConsole(void)
{
#ifdef HAS_USL_VTS
    struct vt_mode VT;
#endif

#if !defined(__i386__) && !defined(__i386) && !defined(__x86)

    if (!xf86DoProbe && !xf86DoConfigure) {
	int fd;

	/*
	 * Wipe out framebuffer just like the non-SI Xsun server does.  This
	 * could be improved by saving framebuffer contents in
	 * xf86OpenConsole() above and restoring them here.  Also, it's unclear
	 * at this point whether this should be done for all framebuffers in
	 * the system, rather than only the console.
	 */
	if ((fd = open("/dev/fb", O_RDWR, 0)) < 0) {
	    xf86Msg(X_WARNING,
		    "xf86CloseConsole():  unable to open framebuffer (%s)\n",
		    strerror(errno));
	} else {
	    struct fbgattr fbattr;

	    if ((ioctl(fd, FBIOGATTR, &fbattr) < 0) &&
		(ioctl(fd, FBIOGTYPE, &fbattr.fbtype) < 0)) {
		xf86Msg(X_WARNING,
			"xf86CloseConsole():  unable to retrieve framebuffer"
			" attributes (%s)\n", strerror(errno));
	    } else {
		pointer fbdata;

		fbdata = mmap(NULL, fbattr.fbtype.fb_size,
			      PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (fbdata == MAP_FAILED) {
		    xf86Msg(X_WARNING,
			    "xf86CloseConsole():  unable to mmap framebuffer"
			    " (%s)\n", strerror(errno));
		} else {
		    memset(fbdata, 0, fbattr.fbtype.fb_size);
		    munmap(fbdata, fbattr.fbtype.fb_size);
		}
	    }

	    close(fd);
	}
    }

#endif

#ifdef KDSETMODE
    /* Reset the display back to text mode */
    SYSCALL(ioctl(xf86Info.consoleFd, KDSETMODE, KD_TEXT));
#endif

#ifdef HAS_USL_VTS

    /*
     * Solaris 2.1 x86 doesn't seem to "switch" back to the console when the VT
     * is relinquished and its mode is reset to auto.  Also, Solaris 2.1 seems
     * to associate vt00 with the console so I've opened the "console" back up
     * and made it the active vt again in text mode and then closed it.  There
     * must be a better hack for this but I'm not aware of one at this time.
     *
     * Doug Anson 11/6/93
     * danson@lgc.com
     *
     * Fixed - 12/5/93 - David Holland - davidh@dorite.use.com
     * Did the whole thing similarly to the way linux does it
     */

    if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) != -1)
    {
	VT.mode = VT_AUTO;		/* Set default vt handling */
	ioctl(xf86Info.consoleFd, VT_SETMODE, &VT);
    }

    /* Activate the VT that X was started on */
    ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86StartVT);

#endif /* HAS_USL_VTS */

    close(xf86Info.consoleFd);
}

int
xf86ProcessArgument(int argc, char **argv, int i)
{
    /*
     * Keep server from detaching from controlling tty.  This is useful when
     * debugging, so the server can receive keyboard signals.
     */
    if (!strcmp(argv[i], "-keeptty"))
    {
	KeepTty = TRUE;
	return 1;
    }

    /*
     * Undocumented flag to protect page 0 from read/write to help catch NULL
     * pointer dereferences.  This is purely a debugging flag.
     */
    if (!strcmp(argv[i], "-protect0"))
    {
	Protect0 = TRUE;
	return 1;
    }

#ifdef HAS_USL_VTS

    if ((argv[i][0] == 'v') && (argv[i][1] == 't'))
    {
	if (sscanf(argv[i], "vt%2d", &VTnum) == 0)
	{
	    UseMsg();
	    VTnum = -1;
	    return 0;
	}

	return 1;
    }

#endif /* HAS_USL_VTS */

    if ((i + 1) < argc) {
	if (!strcmp(argv[i], "-dev")) {
	    strncpy(fb_dev, argv[i+1], PATH_MAX);
	    fb_dev[PATH_MAX - 1] = '\0';
	    return 2;
	}
    }

    return 0;
}

void xf86UseMsg()
{
#ifdef HAS_USL_VTS
    ErrorF("vtXX                   Use the specified VT number\n");
#endif
    ErrorF("-dev <fb>              Framebuffer device\n");
    ErrorF("-keeptty               Don't detach controlling tty\n");
    ErrorF("                       (for debugging only)\n");
}
