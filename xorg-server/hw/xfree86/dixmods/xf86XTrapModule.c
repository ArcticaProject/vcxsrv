/*  This is the xf86 module code for the DEC_XTRAP extension. */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "xf86Module.h"

#include <X11/extensions/xtrapdi.h>

extern void DEC_XTRAPInit(INITARGS);

static MODULESETUPPROTO(xtrapSetup);

static ExtensionModule xtrapExt =
{
    DEC_XTRAPInit,
    XTrapExtName,
    NULL,
    NULL,
    NULL
};

static XF86ModuleVersionInfo xtrapVersRec =
{
    "xtrap",
    MODULEVENDORSTRING,
    MODINFOSTRING1,
    MODINFOSTRING2,
    XORG_VERSION_CURRENT,
    1, 0, 0,
    ABI_CLASS_EXTENSION,         /* needs the server extension ABI */
    ABI_EXTENSION_VERSION,
    MOD_CLASS_EXTENSION,
    {0,0,0,0}
};

_X_EXPORT XF86ModuleData xtrapModuleData = { &xtrapVersRec, xtrapSetup, NULL };

static pointer
xtrapSetup(pointer module, pointer opts, int *errmaj, int *errmin) {
    LoadExtension(&xtrapExt, FALSE);
    /* Need a non-NULL return value to indicate success */
    return (pointer)1;
}
