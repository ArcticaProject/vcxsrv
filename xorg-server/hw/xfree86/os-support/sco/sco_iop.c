/*
 * Copyright 2001 by J. Kean Johnston <jkj@caldera.com>
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


#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>

#include "compiler.h"

#define _NEED_SYSI86
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86OSpriv.h"
#include "xf86_OSlib.h"


/***************************************************************************/
/* I/O Permissions section                                                 */
/***************************************************************************/

/*
 * There is a right way and a wrong way of doing this. Unfortunately, we
 * are forced to do it the wrong way. The right way is to be told the range
 * or ranges of I/O ports the driver(s) need access to, in order to use the
 * CONS_IOPERM ioctl() to grant access only to those ports we care about.
 * This way we can guarantee some small level of stability because a driver
 * does not have access to all ports (which would mean it could play with
 * the PIT and thus affect scheduling times, or a whole slew of other
 * nasty things). However, because XFree86 currently only enables or disables
 * ALL port access, we need to run at IOPL 3, which basically means the
 * X Server runs at the same level as the kernel. You can image why this is
 * unsafe. Oh, and this is not a problem unique to OSR5, other OSes are
 * affected by this as well.
 *
 * So, for the time being, we change our IOPL until such time as the XFree86
 * architecture is changed to allow for tighter control of I/O ports. If and
 * when it is, then the CONS_ADDIOP/DELIOP ioctl() should be used to enable 
 * or disable access to the desired ports.
 */

extern long sysi86 (int cmd, ...);

static Bool IOEnabled = FALSE;

_X_EXPORT Bool
xf86EnableIO(void)
{
	if (IOEnabled)
		return TRUE;

	if (sysi86(SI86V86, V86SC_IOPL, PS_IOPL) < 0) {
		xf86Msg(X_WARNING,"Failed to set IOPL for extended I/O\n");
		return FALSE;
	}
	
	IOEnabled = TRUE;
	return TRUE;
}

_X_EXPORT void
xf86DisableIO(void)
{
	if (!IOEnabled)
		return;

	sysi86(SI86V86, V86SC_IOPL, 0);
	IOEnabled = FALSE;
}
