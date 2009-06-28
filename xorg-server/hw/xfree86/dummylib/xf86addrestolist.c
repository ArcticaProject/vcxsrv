
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"

_X_EXPORT resPtr
xf86AddResToList(resPtr rlist, resRange *Range, int entityIndex)
{
    return rlist;
}

_X_EXPORT void
xf86FreeResList(resPtr rlist)
{
    return;
}

_X_EXPORT resPtr
xf86DupResList(const resPtr rlist)
{
    return rlist;
}
