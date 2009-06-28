/*
 *
 * Copyright 1999-2003 by The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of The XFree86 Project, Inc. not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission. The XFree86 Project, Inc. makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE XFREE86 PROJECT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL THE XFREE86 PROJECT, INC. BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "resource.h"
#include "sym.h"
#include "misc.h"
#ifdef PANORAMIX
#include "panoramiX.h"
#endif
#include "sleepuntil.h"

#ifdef HAS_SHM
extern int ShmCompletionCode;
extern int BadShmSegCode;
extern RESTYPE ShmSegType, ShmPixType;
#endif

#ifdef PANORAMIX
extern Bool noPanoramiXExtension;
extern int PanoramiXNumScreens;
extern PanoramiXData *panoramiXdataPtr;
extern unsigned long XRT_WINDOW;
extern unsigned long XRT_PIXMAP;
extern unsigned long XRT_GC;
extern unsigned long XRT_COLORMAP;
extern unsigned long XRC_DRAWABLE;
extern Bool XineramaRegisterConnectionBlockCallback(void (*func) (void));
extern int XineramaDeleteResource(pointer, XID);
#endif

_X_HIDDEN void *extLookupTab[] = {

    SYMFUNC(ClientSleepUntil)

#ifdef HAS_SHM
    SYMVAR(ShmCompletionCode)
    SYMVAR(BadShmSegCode)
    SYMVAR(ShmSegType)
#endif

#ifdef PANORAMIX
    SYMFUNC(XineramaRegisterConnectionBlockCallback)
    SYMFUNC(XineramaDeleteResource)
    SYMVAR(PanoramiXNumScreens)
    SYMVAR(panoramiXdataPtr)
    SYMVAR(XRT_WINDOW)
    SYMVAR(XRT_PIXMAP)
    SYMVAR(XRT_GC)
    SYMVAR(XRT_COLORMAP)
    SYMVAR(XRC_DRAWABLE)
#endif
};
