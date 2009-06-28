
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
    return MSE_SERIAL | MSE_AUTO;
}

#ifndef ISC
static const char *internalNames[] = {
	NULL
};

static const char **
BuiltinNames(void)
{
    return internalNames;
}

static Bool
CheckProtocol(const char *protocol)
{
    int i;

    for (i = 0; internalNames[i]; i++)
	if (xf86NameCmp(protocol, internalNames[i]) == 0)
	    return TRUE;
    return FALSE;
}
#endif

_X_EXPORT OSMouseInfoPtr
xf86OSMouseInit(int flags)
{
    OSMouseInfoPtr p;

    p = xcalloc(sizeof(OSMouseInfoRec), 1);
    if (!p)
	return NULL;
    p->SupportedInterfaces = SupportedInterfaces;
#ifndef ISC
    p->BuiltinNames = BuiltinNames;
    p->CheckProtocol = CheckProtocol;
#endif
    return p;
}

