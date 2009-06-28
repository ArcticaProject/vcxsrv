/*
 * Copyright 2001-2005 by Kean Johnston <jkj@sco.com>
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell, David Wexelblat 
 * and Kean Johnston not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Thomas Roell, David Wexelblat and Kean Johnston make no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL, DAVID WEXELBLAT AND KEAN JOHNSTON DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THOMAS ROELLm DAVID WEXELBLAT
 * OR KEAN JOHNSTON BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 */

#include "X.h"
#include "Xmd.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#include <sys/utsname.h>

static Bool KeepTty = FALSE;
static Bool Protect0 = FALSE;
static Bool CRTSpecified = FALSE;
static int VTnum = -1;
static char vtdevice[48];

int usl_is_osr6 = -1;

static Bool
IsConsoleDevice(const char *dev)
{
  if ((!strcmp (dev, "/dev/console")) ||
      (!strcmp (dev, "/dev/syscon")) ||
      (!strcmp (dev, "/dev/systty")))
    return TRUE;

  return FALSE;
}

static int
is_osr6 (void)
{
  struct utsname uts;

  if (usl_is_osr6 == -1) {
    if (uname (&uts) < 0) {
      FatalError ("get_usl_ver: Failed to determine UNIX name (%s)\n",
	strerror (errno));
    }

    if (uts.version[0] == '6')
      usl_is_osr6 = 1;
    else
      usl_is_osr6 = 0;
  }

  return usl_is_osr6;
}


void
xf86OpenConsole(void)
{
  int fd, i, ioctl_ret;
  struct vt_mode VT;
  struct vt_stat vts;
  MessageType from = X_PROBED;
  struct sigaction sigvtsw;
  char *ttn;

  if (serverGeneration == 1) {
    int isconsole = 0, consdev = 0;

    /* check if we're run with euid==0 */
    if (geteuid() != 0) {
      FatalError("xf86OpenConsole: Server must be suid root\n");
    }

    /* If we are run in the background we will get SIGTTOU. Ignore it. */
    OsSignal (SIGTTOU, SIG_IGN);

    /* Protect page 0 to help find NULL dereferencing */
    /* mprotect() doesn't seem to work */
    if (Protect0) {
      int fd = -1;

      if ((fd = open("/dev/zero", O_RDONLY, 0)) < 0) {
	xf86Msg(X_WARNING, "xf86OpenConsole: cannot open /dev/zero (%s)\n",
	  strerror(errno));
      } else {
	if ((int)mmap(0, 0x1000, PROT_NONE,
	    MAP_FIXED | MAP_SHARED, fd, 0) == -1) {
	  xf86Msg(X_WARNING, "xf86OpenConsole: failed to protect page 0 (%s)\n",
	    strerror(errno));
	}
	close(fd);
      }
    }

    /*
     * setup the virtual terminal manager
     */
    if (VTnum == -1) {
      /*
       * No device was specified. We need to query the kernel to see which
       * console device we are on (and in fact if we are on a console at all).
       */
      if (ioctl (0, VT_GETSTATE, &vts) < 0) {
	FatalError("xf86OpenConsole: Could not query active VT: %s\n",
	  strerror(errno));
      }
      VTnum = vts.v_active;
      if (is_osr6())
	snprintf (vtdevice, sizeof(vtdevice), "/dev/tty%02d", VTnum + 1);
      else
	snprintf (vtdevice, sizeof(vtdevice), "/dev/vt%02d", VTnum);
    } else {
      from = X_CMDLINE;
      if (is_osr6())
	snprintf (vtdevice, sizeof(vtdevice), "/dev/tty%02d", VTnum + 1);
      else
	snprintf (vtdevice, sizeof(vtdevice), "/dev/vt%02d", VTnum);
    }

    if (IsConsoleDevice(vtdevice)) {
      isconsole = 1;
      CRTSpecified = FALSE;	/* Dont honour -crt /dev/console */
    }

    if (ioctl (0, KIOCINFO, 0) >= 0)
      consdev = 1 + isconsole;

    if ((!CRTSpecified) && (isconsole || (!consdev))) {
      /*
       * Need to find a free VT
       */
      if ((fd = open ("/dev/console", O_WRONLY | O_NOCTTY)) < 0) {
	FatalError ("xf86OpenConsole: Could not open /dev/console: %s\n",
	  strerror (errno));
      }

      if (ioctl (fd, VT_OPENQRY, &VTnum) < 0) {
	FatalError ("xf86OpenConsole: Cannot find a free VT: %s\n",
	  strerror(errno));
      }
      close (fd);
      if (usl_is_osr6)
	snprintf (vtdevice, sizeof(vtdevice), "/dev/tty%02d", VTnum + 1);
      else
	snprintf (vtdevice, sizeof(vtdevice), "/dev/vt%02d", VTnum);
    }

    /*
     * Now we can dispose of stdin/stdout
     */
    fclose (stdin);
    fclose (stdout);

    if (CRTSpecified || isconsole || consdev != 1) {
      if (!KeepTty) {
	setpgrp();
      }
    }

    if ((xf86Info.consoleFd = open(vtdevice, O_RDWR | O_NONBLOCK, 0)) < 0) {
      FatalError("xf86OpenConsole: Cannot open %s: %s\n", vtdevice,
	strerror(errno));
    }

    xf86Msg (from, "using VT number %d (%s)\n\n", VTnum, vtdevice);
    xf86Info.vtno = VTnum;

    /* change ownership of the vt */
    chown(vtdevice, getuid(), getgid());

    /*
     * now get the VT
     */
    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0) {
      FatalError("xf86OpenConsole: VT_ACTIVATE failed: %s\n",
	strerror(errno));
    }
    if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, xf86Info.vtno) != 0) {
      FatalError("xf86OpenConsole: VT_WAITACTIVE failed: %s\n",strerror(errno));
    }

    if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) < 0) {
      FatalError("xf86OpenConsole: VT_GETMODE failed: %s\n", strerror(errno));
    }

    sigvtsw.sa_handler = xf86VTRequest;
    sigfillset(&sigvtsw.sa_mask);
    sigvtsw.sa_flags = 0;
    sigaction(SIGUSR1, &sigvtsw, NULL);

    VT.mode = VT_PROCESS;
    VT.relsig = SIGUSR1;
    VT.acqsig = SIGUSR1;

    ioctl_ret = ioctl(xf86Info.consoleFd, VT_SETMODE, &VT);
    if (ioctl_ret < 0) {
      FatalError("xf86OpenConsole: VT_SETMODE failed: %s\n", strerror(errno));
    }

    if (ioctl(xf86Info.consoleFd, KDSETMODE, KD_GRAPHICS) < 0) {
      FatalError("xf86OpenConsole: KD_GRAPHICS failed: %s\n", strerror(errno));
    }
  } else { /* serverGeneration != 1 */
    /*
     * now get the VT
     */
    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0) {
      FatalError("xf86OpenConsole: VT_ACTIVATE failed: %s\n", strerror(errno));
    }
    if (ioctl(xf86Info.consoleFd, VT_WAITACTIVE, xf86Info.vtno) != 0) {
      FatalError("xf86OpenConsole: VT_WAITACTIVE failed: %s\n",strerror(errno));
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
xf86CloseConsole(void)
{
  struct vt_mode   VT;
  struct sigaction sigvtsw;

  ioctl(xf86Info.consoleFd, KDSETMODE, KD_TEXT);  /* Back to text mode ... */

  sigvtsw.sa_handler = SIG_DFL;
  sigfillset(&sigvtsw.sa_mask);
  sigvtsw.sa_flags = 0;
  sigaction(SIGUSR1, &sigvtsw, NULL);

  if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) != -1) {
    VT.mode = VT_AUTO;
    VT.waitv = 0;
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
  if (!strcmp(argv[i], "-keeptty")) {
    KeepTty = TRUE;
    return(1);
  }

  /*
   * Undocumented flag to protect page 0 from read/write to help
   * catch NULL pointer dereferences.  This is purely a debugging
   * flag.
   */
  if (!strcmp(argv[i], "-protect0")) {
    Protect0 = TRUE;
    return(1);
  }

  if ((argv[i][0] == 'v') && (argv[i][1] == 't')) {
    if (sscanf(argv[i], "vt%2d", &VTnum) == 0) {
      UseMsg();
      VTnum = -1;
      return(0);
    }
    VTnum -= is_osr6();
    CRTSpecified = TRUE;
    return(1);
  }

  /*
   * Use a device the user specifies.
   */
  if (!strcmp(argv[i], "-crt")) {
    if (++i > argc) {
      UseMsg();
      VTnum = -1;
      return(0);
    } else {
      char *mytty = ttyname(0);
      char *arg = argv[i];

      if (!mytty)
	mytty = "\1";
      if (!arg[0])
	arg = "\2";	/* Prevent idiots from using -crt "" */

      if (strcmp (mytty, arg) != 0) {
	char *fmt;

	if (is_osr6())
	  fmt = "/dev/tty%02d";
	else
	  fmt = "/dev/vt%02d";

	if (sscanf(arg, fmt, &VTnum) == 0) {
	  UseMsg();
	  VTnum = -1;
	  return(0);
	}

	/* OSR6 devices start names at 1, UW7 starts at 0 */
	VTnum -= is_osr6();
	CRTSpecified = TRUE;
      }
      return(2);
    }
  }
  return(0);
}

void
xf86UseMsg(void)
{
  if (is_osr6()) {
    ErrorF("-crt /dev/ttyXX        use the specified VT device\n");
    ErrorF("vtXX                   use the specified VT number (01-16)\n");
  } else {
    ErrorF("-crt /dev/vtXX         use the specified VT device\n");
    ErrorF("vtXX                   use the specified VT number (00-15)\n");
  }

  ErrorF("-keeptty               ");
  ErrorF("don't detach controlling tty (for debugging only)\n");
  return;
}
