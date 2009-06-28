/*
 * Copyright 2005 by Kean Johnston <jkj@sco.com>
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

#include "X.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

/*
 * Handle the VT-switching interface for SCO UnixWare / OpenServer 6
 */

/*
 * This function is the signal handler for the VT-switching signal.  It
 * is only referenced inside the OS-support layer. NOTE: we do NOT need
 * to re-arm the signal here, since we used sigaction() to set the signal
 * disposition in usl_init.c. If we had used signal(), we would need to
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

static int usl_ledstatus = -1;
static unsigned int usl_ledstate = 0;

Bool
xf86VTSwitchAway(void)
{
  usl_ledstatus = ioctl(xf86Info.consoleFd, KDGETLED, &usl_ledstate);

  xf86Info.vtRequestsPending = FALSE;
  if (ioctl(xf86Info.consoleFd, VT_RELDISP, 1) < 0) {
    return(FALSE);
  } else {
    return(TRUE);
  }
}

Bool
xf86VTSwitchTo(void)
{
  xf86Info.vtRequestsPending = FALSE;
  if (ioctl(xf86Info.consoleFd, VT_RELDISP, VT_ACKACQ) < 0) {
    return(FALSE);
  } else {
    if (usl_ledstatus >= 0) {
      ioctl (xf86Info.consoleFd, KDSETLED, usl_ledstate);
    }
    usl_ledstatus = -1;

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
