/*
 * Copyright 2006 Adam Jackson.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include "misc.h"
#include "servermd.h"
#include "scrnintstr.h"
#include "resource.h"
#include "fb.h"
#include "fboverlay.h"
#include "cfb8_16.h"

Bool
cfb8_16ScreenInit(ScreenPtr pScreen, pointer pbits16, pointer pbits8,
                  int xsize, int ysize, int dpix, int dpiy,
                  int width16, int width8)
{
    return
        (fbOverlaySetupScreen(pScreen, pbits16, pbits8, xsize, ysize,
                              dpix, dpiy, width16, width8, 16, 8) &&
         fbOverlayFinishScreenInit(pScreen, pbits16, pbits8, xsize, ysize,
                                   dpix, dpiy, width16, width8, 16, 8, 16, 8));
}

#include "xf86Module.h"

static MODULESETUPPROTO(xf8_16bppSetup);

static XF86ModuleVersionInfo VersRec = {
        "xf8_16bpp",
        MODULEVENDORSTRING,
        MODINFOSTRING1,
        MODINFOSTRING2,
        XORG_VERSION_CURRENT,
        2, 0, 0,
        ABI_CLASS_ANSIC,                /* Only need the ansic layer */
        ABI_ANSIC_VERSION,
        NULL,
        {0,0,0,0}       /* signature, to be patched into the file by a tool */
};

_X_EXPORT XF86ModuleData xf8_16bppModuleData = {
    &VersRec,
    xf8_16bppSetup,
    NULL
};

static pointer
xf8_16bppSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
    return (pointer)LoadSubModule(module, "fb", NULL, NULL, NULL, NULL,
			          errmaj, errmin);
}
