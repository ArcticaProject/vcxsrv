/*
 * Copyright 2007 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * them Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTIBILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif
#include "scrnintstr.h"
#include "mibstore.h"

/*
 * There is no longer an mi implementation of backing store.  This function
 * is only for source compatibility with old drivers.
 *
 * Note though that you do get backing store for free if your server has
 * Composite enabled, since the automatic redirection mechanism provides
 * essentially the same functionality.  See compChangeWindowAttributes()
 * for the implementation.
 */

_X_EXPORT void
miInitializeBackingStore (ScreenPtr pScreen)
{
    pScreen->SaveDoomedAreas = NULL;
    pScreen->RestoreAreas = NULL;
    pScreen->ExposeCopy = NULL;
    pScreen->TranslateBackingStore = NULL;
    pScreen->ClearBackingStore = NULL;
    pScreen->DrawGuarantee = NULL;
}
