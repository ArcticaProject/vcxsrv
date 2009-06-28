
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
xf86ErrorF(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    LogVWrite(1, format, ap);
    va_end(ap);
}

_X_EXPORT void
ErrorF(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    LogVWrite(1, format, ap);
    va_end(ap);
}
