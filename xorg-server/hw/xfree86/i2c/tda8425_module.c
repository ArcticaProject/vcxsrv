#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86Module.h"

static MODULESETUPPROTO(tda8425Setup);

static XF86ModuleVersionInfo tda8425VersRec =
{
        "tda8425",
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
 
_X_EXPORT XF86ModuleData tda8425ModuleData = {
        &tda8425VersRec,
        tda8425Setup,
        NULL
}; 

static pointer
tda8425Setup(pointer module, pointer opts, int *errmaj, int *errmin) {
   return (pointer)1;
}
