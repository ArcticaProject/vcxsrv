#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86Module.h"

static MODULESETUPPROTO(xf8_32bppSetup);

static XF86ModuleVersionInfo VersRec =
{
        "xf8_32bpp",
        MODULEVENDORSTRING,
        MODINFOSTRING1,
        MODINFOSTRING2,
        XORG_VERSION_CURRENT,
        1, 0, 0,
        ABI_CLASS_ANSIC,                /* Only need the ansic layer */
        ABI_ANSIC_VERSION,
        MOD_CLASS_NONE,
        {0,0,0,0}       /* signature, to be patched into the file by a tool */
};

_X_EXPORT XF86ModuleData xf8_32bppModuleData = {
    &VersRec,
    xf8_32bppSetup,
    NULL
};

static pointer
xf8_32bppSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    if (!LoadSubModule(module, "cfb", NULL, NULL, NULL, NULL,
			errmaj, errmin))
        return NULL;
    if (!LoadSubModule(module, "cfb32", NULL, NULL, NULL, NULL,
			errmaj, errmin))
        return NULL;
    return (pointer)1;  /* non-NULL required to indicate success */
}
