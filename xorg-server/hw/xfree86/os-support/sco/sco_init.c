/*
 * Copyright 2001-2005 by J. Kean Johnston <jkj@sco.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name J. Kean Johnston not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  J. Kean Johnston makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * J. KEAN JOHNSTON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL J. KEAN JOHNSTON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* Re-written May 2001 to represent the current state of reality */

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
static int VTnum = -1;
static char vtdevice[48];
static int sco_console_mode = -1;

extern Bool mpxLock;

void
xf86OpenConsole(void)
{
  int i, ioctl_ret;
  struct vt_mode VT;
  struct vid_info vidinf;
  struct sigaction sigvtsw;
  char *ttn;

  if (serverGeneration == 1) {
    /* check if we're run with euid==0 */
    if (geteuid() != 0) {
      FatalError("xf86OpenConsole: Server must be setuid root\n");
    }

    /* If we are run in the background we will get SIGTTOU. Ignore it. */
    OsSignal (SIGTTOU, SIG_IGN);

    /*
     * Set up the virtual terminal (multiscreen in SCO parlance).
     * For the actual console itself, screens are numbered from
     * 1 to (usually) 16. However, it is possible to have a nested
     * server, and it is also possible to be on a multi-console
     * system such as MaxSpeed or SunRiver. Therefore, we should
     * not make any assumptions about the TTY name we are on, and
     * instead we rely on ttyname() to give us the real TTY name.
     * Previously, we tried to determine the TTY name manually.
     * This is wrong. The only time we need to futz with the TTY name
     * is if we were given the name of a TTY to run on explicity on
     * the command line.
     */

    if (VTnum == -1) {
      /*
       * No device was specified. We need to query the kernel to see which
       * console device we are on (and in fact if we are on a console at all).
       */
      ttn = ttyname (1);

      if (ttn == (char *)0) {
	FatalError ("xf86OpenConsole: Could not determine TTY name: %s\n",
	  strerror(errno));
      }
      strlcpy (vtdevice, ttn, sizeof(vtdevice));
    } else if (VTnum >= 0) {
      snprintf (vtdevice, sizeof(vtdevice), "/dev/tty%02d", VTnum);
    }

    /*
     * Now we can dispose of stdin/stdout
     */
    fclose (stdin);
    fclose (stdout);

    if ((xf86Info.consoleFd = open(vtdevice, O_RDWR | O_NDELAY, 0)) < 0) {
      FatalError("xf86OpenConsole: Cannot open %s: %s\n", vtdevice,
	strerror(errno));
    }

    /*
     * We make 100% sure we use the correct VT number. This can get ugly
     * where there are multi-consoles in use, so we make sure we query
     * the kernel for the correct VT number. It knows best, we don't.
     */
    vidinf.size = sizeof(vidinf);
    if (ioctl (xf86Info.consoleFd, CONS_GETINFO, &vidinf) < 0) {
      FatalError ("xf86OpenConsole: %s not a console device "
	"or error querying device: %s\n", vtdevice, strerror (errno));
    }
    xf86Info.vtno = vidinf.m_num;
    VTnum = vidinf.m_num + 1; /* 0-based */

    ErrorF("(using VT%02d device %s)\n\n", VTnum, vtdevice);

    /* We activate the console just in case its not the one we are on */
    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0) {
        ErrorF("xf86OpenConsole: VT_ACTIVATE failed (%s)\n", strerror(errno));
    }

    /* Disassociate from controling TTY */
    if (!KeepTty) {
      setpgrp();
    }

    /*
     * Now we get the current mode that the console device is in. We will
     * use this later when we close the console device to restore it to
     * that same mode.
     */
    if ((sco_console_mode = ioctl(xf86Info.consoleFd, CONS_GET, 0L)) < 0) {
      FatalError("xf86OpenConsole: CONS_GET failed on console (%s)\n",
          strerror(errno));
    }

    if (ioctl(xf86Info.consoleFd, VT_GETMODE, &VT) < 0) {
      FatalError("xf86OpenConsole: VT_GETMODE failed (%s)\n", strerror(errno));
    }

    sigvtsw.sa_handler = xf86VTRequest;
    sigfillset(&sigvtsw.sa_mask);
    sigvtsw.sa_flags = 0;

    /* NOTE: Using sigaction means we dont have to re-arm the signal */
    sigaction(SIGUSR1, &sigvtsw, NULL);

    VT.mode = VT_PROCESS;
    VT.relsig = SIGUSR1;
    VT.acqsig = SIGUSR1;
    VT.frsig = SIGINT;          /* Not implemented */
    VT.waitv = 0;

    /*
     * The SCO X server tries the following call 5 times. Lets do the same
     * thing. It shouldn't really be required but sometimes things take a
     * while to settle down when switching screens. *helpless shrug* I know
     * its sucks but ...
     */

    ioctl_ret = 0;
    for (i = 0; i < 5; i++) {
      ioctl_ret = ioctl(xf86Info.consoleFd, VT_SETMODE, &VT);
      if (ioctl_ret >= 0)
        break;
      usleep(999999); /* Dont use nap() - it forces linking with -lx */
    }

    if (ioctl_ret < 0) {
      FatalError("xf86OpenConsole: VT_SETMODE failed (%s)\n", strerror(errno));
    }

    /*
     * Convince the console driver we are in graphics mode.
     */
    if (ioctl(xf86Info.consoleFd, KDSETMODE, KD_GRAPHICS) < 0) {
        ErrorF("Failed to set graphics mode (%s)\n", strerror(errno));
    }
  } else { /* serverGeneration != 1 */
    if (ioctl(xf86Info.consoleFd, VT_ACTIVATE, xf86Info.vtno) != 0) {
      ErrorF("xf86OpenConsole: VT_ACTIVATE failed (%s)\n", strerror(errno));
    }
  }
}

/*
 * Restore the console to its previous state. This may cause flicker if
 * the screen was previous in a graphics mode, because we first set it
 * to text mode. This has the advantage of getting the console driver
 * to do a soft reset on the card, which really does help settle the
 * video card down again after coming out of Xfree86.
 */
void
xf86CloseConsole(void)
{
  struct vt_mode VT;
  struct sigaction sigvtsw;

  /* Set text mode (possibly briefly) */
  ioctl(xf86Info.consoleFd, KDSETMODE, KD_TEXT0);

  /* Restore the original mode */
  if (sco_console_mode != -1) {
      ioctl(xf86Info.consoleFd, MODESWITCH | sco_console_mode, 0L);
  }

  ioctl(xf86Info.consoleFd, VT_RELDISP, 1);     /* Release the display */

  sigvtsw.sa_handler = SIG_DFL;
  sigfillset(&sigvtsw.sa_mask);
  sigvtsw.sa_flags = 0;

  sigaction(SIGUSR1, &sigvtsw, NULL);

  VT.mode = VT_AUTO;
  VT.waitv = 0;
  VT.relsig = SIGUSR1;
  VT.acqsig = SIGUSR1;
  VT.frsig  = SIGINT;
  ioctl(xf86Info.consoleFd, VT_SETMODE, &VT); /* Revert to auto handling */

  close(xf86Info.consoleFd); /* We're done with the device */
}

int
xf86ProcessArgument(int argc, char *argv[], int i)
{
  /*
   * Keep server from detaching from controlling tty.  This is useful
   * when debugging (so the server can receive keyboard signals).
   */
  if (!strcmp(argv[i], "-keeptty")) {
    KeepTty = TRUE;
    return(1);
  }

  /*
   * By default, the X server wants to bind itself to CPU 0. This makes
   * sure that the server has full access to the I/O ports at IOPL 3.
   * Some SMP systems have trouble with I/O on CPU's other than 0. If,
   * however, you have a system that is well behaved, you can specify
   * this argument and let the scheduler decide which CPU the server
   * should run on.
   */
  if (!strcmp(argv[i], "-nompxlock")) {
    mpxLock = FALSE;
    return (1);
  }

  /*
   * Specify the VT number to run on (NOT the device).
   */
  if ((argv[i][0] == 'v') && (argv[i][1] == 't')) {
    if (sscanf(argv[i], "vt%2d", &VTnum) == 0) {
      UseMsg();
      VTnum = -1;
      return(0);
    }
    if (VTnum <= 0) {
      UseMsg();
      VTnum = -1;
      return(0);
    }
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
      VTnum = -2;
      strlcpy (vtdevice, argv[i], sizeof(vtdevice));
      return(2);
    }
  }
  return(0);
}

void
xf86UseMsg(void)
{
	ErrorF("vtXX                   use the specified VT number\n");
	ErrorF("-crt DEVICE            use the specified VT device\n");
        ErrorF("-nompxlock             dont bind X server to CPU 0\n");
	ErrorF("-keeptty               ");
	ErrorF("don't detach controlling tty (for debugging only)\n");
}
