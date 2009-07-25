/*
 * quartzCommon.h
 *
 * Common definitions used internally by all Quartz modes
 *
 * This file should be included before any X11 or IOKit headers
 * so that it can avoid symbol conflicts.
 *
 * Copyright (c) 2001-2004 Torrey T. Lyons and Greg Parker.
 *                 All Rights Reserved.
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

#ifndef _QUARTZCOMMON_H
#define _QUARTZCOMMON_H

#include <X11/Xdefs.h>
#include "privates.h"

// Quartz specific per screen storage structure
typedef struct {
    // List of CoreGraphics displays that this X11 screen covers.
    // This is more than one CG display for video mirroring and
    // rootless PseudoramiX mode.
    // No CG display will be covered by more than one X11 screen.
    int displayCount;
    CGDirectDisplayID *displayIDs;
} QuartzScreenRec, *QuartzScreenPtr;

#define QUARTZ_PRIV(pScreen) \
    ((QuartzScreenPtr)dixLookupPrivate(&pScreen->devPrivates, quartzScreenKey))

// Data stored at startup for Cocoa front end
extern int              quartzEventWriteFD;

// User preferences used by Quartz modes
extern int              quartzUseSysBeep;
extern int              focusOnNewWindow;
extern int              quartzUseAGL;
extern int              quartzEnableKeyEquivalents;
extern int              quartzFullscreenDisableHotkeys;

// Other shared data
extern int              quartzServerVisible;
extern int              quartzServerQuitting;
extern DevPrivateKey    quartzScreenKey;
extern int              aquaMenuBarHeight;

// Name of GLX bundle for native OpenGL
extern const char      *quartzOpenGLBundle;

void QuartzReadPreferences(void);
void QuartzMessageMainThread(unsigned msg, void *data, unsigned length);
void QuartzMessageServerThread(int type, int argc, ...);
void QuartzSetWindowMenu(int nitems, const char **items,
                         const char *shortcuts);
void QuartzFSCapture(void);
void QuartzFSRelease(void);
int  QuartzFSUseQDCursor(int depth);
void QuartzBlockHandler(pointer blockData, OSTimePtr pTimeout, pointer pReadmask);
void QuartzWakeupHandler(pointer blockData, int result, pointer pReadmask);

#endif  /* _QUARTZCOMMON_H */
