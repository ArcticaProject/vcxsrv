/* Copyright (c) 2008 Apple Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 * HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above
 * copyright holders shall not be used in advertising or otherwise to
 * promote the sale, use or other dealings in this Software without
 * prior written authorization.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "xpr.h"

#define NEED_EVENTS
#include   <X11/X.h>
#include   <X11/Xmd.h>
#include   <X11/Xproto.h>
#include   "misc.h"
#include   "windowstr.h"
#include   "pixmapstr.h"
#include   "inputstr.h"
#include   "mi.h"
#include   "scrnintstr.h"
#include   "mipointer.h"

#include "darwin.h"
#include "quartz.h"
#include "quartzKeyboard.h"
#include "darwinEvents.h"

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "rootlessWindow.h"
#include "xprEvent.h"

static void xprEventHandler(int screenNum, xEventPtr xe, DeviceIntPtr dev, int nevents) {
    int i;
    
    TA_SERVER();
    
    DEBUG_LOG("DarwinEventHandler(%d, %p, %p, %d)\n", screenNum, xe, dev, nevents);
    for (i=0; i<nevents; i++) {
        switch(xe[i].u.u.type) {
                
            case kXquartzWindowState:
                DEBUG_LOG("kXquartzWindowState\n");
                RootlessNativeWindowStateChanged(xprGetXWindow(xe[i].u.clientMessage.u.l.longs0),
                                                 xe[i].u.clientMessage.u.l.longs1);
                break;
                
            case kXquartzWindowMoved:
                DEBUG_LOG("kXquartzWindowMoved\n");
                RootlessNativeWindowMoved(xprGetXWindow(xe[i].u.clientMessage.u.l.longs0));
                break;
                
            case kXquartzBringAllToFront:
                DEBUG_LOG("kXquartzBringAllToFront\n");
                RootlessOrderAllWindows();
                break;
        }
    }
}

void QuartzModeEQInit(void) {
    mieqSetHandler(kXquartzWindowState, xprEventHandler);
    mieqSetHandler(kXquartzWindowMoved, xprEventHandler);
    mieqSetHandler(kXquartzBringAllToFront, xprEventHandler);
}
