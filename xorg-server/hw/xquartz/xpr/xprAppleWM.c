/*
 * Xplugin rootless implementation functions for AppleWM extension
 *
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 * Copyright (c) 2003 Torrey T. Lyons. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "xpr.h"
#include "applewmExt.h"
#include "rootless.h"
#include "Xplugin.h"
#include <X11/X.h>

static int xprSetWindowLevel(
    WindowPtr pWin,
    int level)
{
    xp_window_id wid;
    xp_window_changes wc;

    wid = (xp_window_id) RootlessFrameForWindow (pWin, TRUE);
    if (wid == 0)
        return BadWindow;

    RootlessStopDrawing (pWin, FALSE);

    wc.window_level = level;
    if (xp_configure_window (wid, XP_WINDOW_LEVEL, &wc) != Success) {
        return BadValue;
    }

    return Success;
}


static int xprFrameDraw(
    WindowPtr pWin,
    int class,
    unsigned int attr,
    const BoxRec *outer,
    const BoxRec *inner,
    unsigned int title_len,
    const unsigned char *title_bytes)
{
    xp_window_id wid;

    wid = (xp_window_id) RootlessFrameForWindow (pWin, FALSE);
    if (wid == 0)
        return BadWindow;

    if (xp_frame_draw (wid, class, attr, outer, inner,
                       title_len, title_bytes) != Success)
    {
        return BadValue;
    }

    return Success;
}


static AppleWMProcsRec xprAppleWMProcs = {
    xp_disable_update,
    xp_reenable_update,
    xprSetWindowLevel,
    xp_frame_get_rect,
    xp_frame_hit_test,
    xprFrameDraw
};


void xprAppleWMInit(void)
{
    AppleWMExtensionInit(&xprAppleWMProcs);
}
