/*
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 * Copyright 1993 by David McCullough <davidm@stallion.oz.au>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of David Wexelblat not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  David Wexelblat makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * DAVID WEXELBLAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL DAVID WEXELBLAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
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

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

/* For the event driver prototypes */
#include <sys/event.h>
#include <mouse.h>

/*
 * Handle the VT-switching interface for SCO
 */

/*
 * This function is the signal handler for the VT-switching signal.  It
 * is only referenced inside the OS-support layer. NOTE: we do NOT need
 * to re-arm the signal here, since we used sigaction() to set the signal
 * disposition in sco_init.c. If we had used signal(), we would need to
 * re-arm the signal here. All we need to do now is record the fact that
 * we got the signal. XFree86 handles the rest.
 */
void
xf86VTRequest(int sig)
{
  xf86Info.vtRequestsPending = TRUE;
  return;
}

Bool
xf86VTSwitchPending(void)
{
  return(xf86Info.vtRequestsPending ? TRUE : FALSE);
}

/*
 * When we switch away, we need to flush and suspend the event driver
 * before the VT_RELDISP. We also need to get the current LED status
 * and preserve it, so that we can restore it when we come back.
 */
static int sco_ledstatus = -1;
static unsigned int sco_ledstate = 0;

Bool
xf86VTSwitchAway(void)
{
  ev_flush();
  ev_suspend();

  sco_ledstatus = ioctl(xf86Info.consoleFd, KDGETLED, &sco_ledstate);

  xf86Info.vtRequestsPending = FALSE;
  if (ioctl(xf86Info.consoleFd, VT_RELDISP, VT_TRUE) < 0) {
    return(FALSE);
  } else {
    return(TRUE);
  }
}

/*
 * When we come back to the X server, we need to resume the event driver,
 * and we need to restore the LED settings to what they were when we
 * switched away.
 */
Bool
xf86VTSwitchTo(void)
{
  ev_resume();

  xf86Info.vtRequestsPending = FALSE;
  if (ioctl(xf86Info.consoleFd, VT_RELDISP, VT_ACKACQ) < 0) {
    return(FALSE);
  } else {
    if (sco_ledstatus >= 0) {
      ioctl (xf86Info.consoleFd, KDSETLED, sco_ledstate);
    }
    sco_ledstatus = -1;

    /*
     * Convince the console driver this screen is in graphics mode,
     * otherwise it assumes it can do more to the screen than it should.
     */
    if (ioctl(xf86Info.consoleFd, KDSETMODE, KD_GRAPHICS) < 0) {
        ErrorF("Failed to set graphics mode (%s)\n", strerror(errno));
    }

    return TRUE;
  }
}
