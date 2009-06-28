/*******************************************************************************
  Stub for Alpha Linux
*******************************************************************************/
 
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

/* 
 *   All we really need is a delay of about 40ns for I/O recovery for just
 *   about any occasion, but we'll be more conservative here:  On a
 *   100-MHz CPU, produce at least a delay of 1,000ns.
 */ 
_X_EXPORT void
xf86IODelay()
{
	xf86UDelay(1);
}

