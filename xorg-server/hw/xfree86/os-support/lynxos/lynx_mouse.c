
/*
 * Copyright 1999 by The XFree86 Project, Inc.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Xinput.h"
#include "xf86OSmouse.h"

static int
SupportedInterfaces(void)
{
    /* XXX Need to check this. */
    return MSE_SERIAL | MSE_BUS | MSE_PS2 | MSE_AUTO;
}

_X_EXPORT OSMouseInfoPtr
xf86OSMouseInit(int flags)
{
    OSMouseInfoPtr p;

    p = xcalloc(sizeof(OSMouseInfoRec), 1);
    if (!p)
	return NULL;
    p->SupportedInterfaces = SupportedInterfaces;
    return p;
}

