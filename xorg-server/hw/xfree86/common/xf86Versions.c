/*
 * Copyright (c) 2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"
#include "xf86OSmouse.h"

static CARD32 registeredVersions[NUM_BUILTIN_IFS];

_X_EXPORT CARD32
xf86GetBuiltinInterfaceVersion(BuiltinInterface iface, int flags)
{
    if (iface < 0 || iface >= NUM_BUILTIN_IFS) {
	xf86Msg(X_ERROR, "xf86GetBuiltinInterfaceVersion: Unexpected interface"
		"query: %d\n", iface);
	return 0;
    }

    if (registeredVersions[iface])
	return registeredVersions[iface];

    /* Most built-in interfaces are handled this way. */
    switch (iface) {
    case BUILTIN_IF_OSMOUSE:
	return OS_MOUSE_VERSION_CURRENT;
    default:
	xf86Msg(X_ERROR, "xf86GetBuiltinInterfaceVersion: internal error: "
		"interface %d not handled\n", iface);
	return 0;
    }
}

_X_EXPORT Bool
xf86RegisterBuiltinInterfaceVersion(BuiltinInterface iface, CARD32 version,
				    int flags)
{
    if (iface < 0 || iface >= NUM_BUILTIN_IFS) {
	xf86Msg(X_ERROR, "xf86RegisterBuiltinInterfaceVersion: "
		"unexpected interface number: %d\n", iface);
	return FALSE;
    }
    if (version == 0) {
	xf86Msg(X_ERROR, "xf86RegisterBuiltinInterfaceVersion: "
		"versions must be greater than zero\n");
	return FALSE;
    }
    registeredVersions[iface] = version;
    return TRUE;
}

