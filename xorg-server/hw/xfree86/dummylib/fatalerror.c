#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

/*
 * Utility functions required by libxf86_os. 
 */

_X_EXPORT void
FatalError(const char *f, ...)
{
    va_list args;

    va_start(args, f);
    fprintf(stderr, "Fatal Error:\n");
    vfprintf(stderr, f, args);
    va_end(args);
    exit(1);
}

