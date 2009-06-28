#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86Module.h"

static MODULESETUPPROTO(uda1380Setup);

static XF86ModuleVersionInfo uda1380VersRec =
{
        "uda1380",
        MODULEVENDORSTRING,
        MODINFOSTRING1,
        MODINFOSTRING2,
        XORG_VERSION_CURRENT,
        1, 0, 0,
        ABI_CLASS_VIDEODRV,             /* This needs the video driver ABI */
        ABI_VIDEODRV_VERSION,
        MOD_CLASS_NONE,
        {0,0,0,0}
};
 
_X_EXPORT XF86ModuleData uda1380ModuleData = {
        &uda1380VersRec,
        uda1380Setup,
        NULL
}; 

static pointer
uda1380Setup(pointer module, pointer opts, int *errmaj, int *errmin) {
   return (pointer)1;
}
