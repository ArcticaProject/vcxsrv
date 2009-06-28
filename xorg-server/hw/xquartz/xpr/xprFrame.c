/*
 * Xplugin rootless implementation frame functions
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
#include "rootlessCommon.h"
#include "Xplugin.h"
#include "x-hash.h"
#include "x-list.h"
#include "applewmExt.h"

#include "propertyst.h"
#include "dix.h"
#include <X11/Xatom.h>
#include "windowstr.h"

#include <pthread.h>

#define DEFINE_ATOM_HELPER(func,atom_name)                      \
static Atom func (void) {                                       \
    static int generation;                                      \
    static Atom atom;                                           \
    if (generation != serverGeneration) {                       \
        generation = serverGeneration;                          \
        atom = MakeAtom (atom_name, strlen (atom_name), TRUE);  \
    }                                                           \
    return atom;                                                \
}

DEFINE_ATOM_HELPER(xa_native_window_id, "_NATIVE_WINDOW_ID")

/* Maps xp_window_id -> RootlessWindowRec */
static x_hash_table *window_hash;
static pthread_mutex_t window_hash_mutex;

static Bool no_configure_window;


static inline xp_error
xprConfigureWindow(xp_window_id id, unsigned int mask,
                   const xp_window_changes *values)
{
  //  ErrorF("xprConfigureWindow()\n");
    if (!no_configure_window)
        return xp_configure_window(id, mask, values);
    else
        return XP_Success;
}


static void
xprSetNativeProperty(RootlessWindowPtr pFrame)
{
    xp_error err;
    unsigned int native_id;
    long data;

    err = xp_get_native_window((xp_window_id) pFrame->wid, &native_id);
    if (err == Success)
    {
        /* FIXME: move this to AppleWM extension */

        data = native_id;
        dixChangeWindowProperty(serverClient, pFrame->win, xa_native_window_id(),
				XA_INTEGER, 32, PropModeReplace, 1, &data, TRUE);
    }
}


/*
 * Create and display a new frame.
 */
Bool
xprCreateFrame(RootlessWindowPtr pFrame, ScreenPtr pScreen,
               int newX, int newY, RegionPtr pShape)
{
    WindowPtr pWin = pFrame->win;
    xp_window_changes wc;
    unsigned int mask = 0;
    xp_error err;

    wc.x = newX;
    wc.y = newY;
    wc.width = pFrame->width;
    wc.height = pFrame->height;
    wc.bit_gravity = XP_GRAVITY_NONE;
    mask |= XP_BOUNDS;

    if (pWin->drawable.depth == 8)
    {
        wc.depth = XP_DEPTH_INDEX8;
#if 0
        wc.colormap = xprColormapCallback;
        wc.colormap_data = pScreen;
        mask |= XP_COLORMAP;
#endif
    }
    else if (pWin->drawable.depth == 15)
        wc.depth = XP_DEPTH_RGB555;
    else if (pWin->drawable.depth == 24)
        wc.depth = XP_DEPTH_ARGB8888;
    else
        wc.depth = XP_DEPTH_NIL;
    mask |= XP_DEPTH;

    if (pShape != NULL)
    {
        wc.shape_nrects = REGION_NUM_RECTS(pShape);
        wc.shape_rects = REGION_RECTS(pShape);
        wc.shape_tx = wc.shape_ty = 0;
        mask |= XP_SHAPE;
    }

    err = xp_create_window(mask, &wc, (xp_window_id *) &pFrame->wid);

    if (err != Success)
    {
        return FALSE;
    }

    if (window_hash == NULL)
    {
        window_hash = x_hash_table_new(NULL, NULL, NULL, NULL);
        pthread_mutex_init(&window_hash_mutex, NULL);
    }

    pthread_mutex_lock(&window_hash_mutex);
    x_hash_table_insert(window_hash, pFrame->wid, pFrame);
    pthread_mutex_unlock(&window_hash_mutex);

    xprSetNativeProperty(pFrame);

    return TRUE;
}


/*
 * Destroy a frame.
 */
void
xprDestroyFrame(RootlessFrameID wid)
{
    pthread_mutex_lock(&window_hash_mutex);
    x_hash_table_remove(window_hash, wid);
    pthread_mutex_unlock(&window_hash_mutex);

    xp_destroy_window((xp_window_id) wid);
}


/*
 * Move a frame on screen.
 */
void
xprMoveFrame(RootlessFrameID wid, ScreenPtr pScreen, int newX, int newY)
{
    xp_window_changes wc;

    wc.x = newX;
    wc.y = newY;
    //    ErrorF("xprMoveFrame(%d, %p, %d, %d)\n", wid, pScreen, newX, newY);
    xprConfigureWindow((xp_window_id) wid, XP_ORIGIN, &wc);
}


/*
 * Resize and move a frame.
 */
void
xprResizeFrame(RootlessFrameID wid, ScreenPtr pScreen,
               int newX, int newY, unsigned int newW, unsigned int newH,
               unsigned int gravity)
{
    xp_window_changes wc;

    wc.x = newX;
    wc.y = newY;
    wc.width = newW;
    wc.height = newH;
    wc.bit_gravity = gravity;

    /* It's unlikely that being async will save us anything here.
       But it can't hurt. */

    xprConfigureWindow((xp_window_id) wid, XP_BOUNDS, &wc);
}


/*
 * Change frame stacking.
 */
void
xprRestackFrame(RootlessFrameID wid, RootlessFrameID nextWid)
{
    xp_window_changes wc;

    /* Stack frame below nextWid it if it exists, or raise
       frame above everything otherwise. */

    if (nextWid == NULL)
    {
        wc.stack_mode = XP_MAPPED_ABOVE;
        wc.sibling = 0;
    }
    else
    {
        wc.stack_mode = XP_MAPPED_BELOW;
        wc.sibling = (xp_window_id) nextWid;
    }

    xprConfigureWindow((xp_window_id) wid, XP_STACKING, &wc);
}


/*
 * Change the frame's shape.
 */
void
xprReshapeFrame(RootlessFrameID wid, RegionPtr pShape)
{
    xp_window_changes wc;

    if (pShape != NULL)
    {
        wc.shape_nrects = REGION_NUM_RECTS(pShape);
        wc.shape_rects = REGION_RECTS(pShape);
    }
    else
    {
        wc.shape_nrects = -1;
        wc.shape_rects = NULL;
    }

    wc.shape_tx = wc.shape_ty = 0;

    xprConfigureWindow((xp_window_id) wid, XP_SHAPE, &wc);
}


/*
 * Unmap a frame.
 */
void
xprUnmapFrame(RootlessFrameID wid)
{
    xp_window_changes wc;

    wc.stack_mode = XP_UNMAPPED;
    wc.sibling = 0;

    xprConfigureWindow((xp_window_id) wid, XP_STACKING, &wc);
}


/*
 * Start drawing to a frame.
 *  Prepare for direct access to its backing buffer.
 */
void
xprStartDrawing(RootlessFrameID wid, char **pixelData, int *bytesPerRow)
{
    void *data[2];
    unsigned int rowbytes[2];
    xp_error err;

    err = xp_lock_window((xp_window_id) wid, NULL, NULL, data, rowbytes, NULL);
    if (err != Success)
        FatalError("Could not lock window %i for drawing.", (int) wid);

    *pixelData = data[0];
    *bytesPerRow = rowbytes[0];
}


/*
 * Stop drawing to a frame.
 */
void
xprStopDrawing(RootlessFrameID wid, Bool flush)
{
    xp_unlock_window((xp_window_id) wid, flush);
}


/*
 * Flush drawing updates to the screen.
 */
void
xprUpdateRegion(RootlessFrameID wid, RegionPtr pDamage)
{
    xp_flush_window((xp_window_id) wid);
}


/*
 * Mark damaged rectangles as requiring redisplay to screen.
 */
void
xprDamageRects(RootlessFrameID wid, int nrects, const BoxRec *rects,
               int shift_x, int shift_y)
{
    xp_mark_window((xp_window_id) wid, nrects, rects, shift_x, shift_y);
}


/*
 * Called after the window associated with a frame has been switched
 * to a new top-level parent.
 */
void
xprSwitchWindow(RootlessWindowPtr pFrame, WindowPtr oldWin)
{
    DeleteProperty(serverClient, oldWin, xa_native_window_id());

    xprSetNativeProperty(pFrame);
}


/*
 * Called to check if the frame should be reordered when it is restacked.
 */
Bool xprDoReorderWindow(RootlessWindowPtr pFrame)
{
    WindowPtr pWin = pFrame->win;

    return AppleWMDoReorderWindow(pWin);
}


/*
 * Copy area in frame to another part of frame.
 *  Used to accelerate scrolling.
 */
void
xprCopyWindow(RootlessFrameID wid, int dstNrects, const BoxRec *dstRects,
              int dx, int dy)
{
    xp_copy_window((xp_window_id) wid, (xp_window_id) wid,
                   dstNrects, dstRects, dx, dy);
}


static RootlessFrameProcsRec xprRootlessProcs = {
    xprCreateFrame,
    xprDestroyFrame,
    xprMoveFrame,
    xprResizeFrame,
    xprRestackFrame,
    xprReshapeFrame,
    xprUnmapFrame,
    xprStartDrawing,
    xprStopDrawing,
    xprUpdateRegion,
    xprDamageRects,
    xprSwitchWindow,
    xprDoReorderWindow,
    xp_copy_bytes,
    xp_fill_bytes,
    xp_composite_pixels,
    xprCopyWindow
};


/*
 * Initialize XPR implementation
 */
Bool
xprInit(ScreenPtr pScreen)
{
    RootlessInit(pScreen, &xprRootlessProcs);

    rootless_CopyBytes_threshold = xp_copy_bytes_threshold;
    rootless_FillBytes_threshold = xp_fill_bytes_threshold;
    rootless_CompositePixels_threshold = xp_composite_area_threshold;
    rootless_CopyWindow_threshold = xp_scroll_area_threshold;

    no_configure_window = FALSE;

    return TRUE;
}


/*
 * Given the id of a physical window, try to find the top-level (or root)
 * X window that it represents.
 */
WindowPtr
xprGetXWindow(xp_window_id wid)
{
    RootlessWindowRec *winRec;

    if (window_hash == NULL)
        return NULL;

    winRec = x_hash_table_lookup(window_hash, (void *) wid, NULL);

    return winRec != NULL ? winRec->win : NULL;
}

/*
 * Given the id of a physical window, try to find the top-level (or root)
 * X window that it represents.
 */
WindowPtr
xprGetXWindowFromAppKit(int windowNumber)
{
    RootlessWindowRec *winRec;
    Bool ret;
    xp_window_id wid;

    if (window_hash == NULL)
        return FALSE;

    /* need to lock, since this function can be called by any thread */

    pthread_mutex_lock(&window_hash_mutex);

    if (xp_lookup_native_window(windowNumber, &wid))
        ret = xprGetXWindow(wid) != NULL;
    else
        ret = FALSE;

    pthread_mutex_unlock(&window_hash_mutex);

    if (!ret) return NULL;
    winRec = x_hash_table_lookup(window_hash, (void *) wid, NULL);

    return winRec != NULL ? winRec->win : NULL;
}


/*
 * The windowNumber is an AppKit window number. Returns TRUE if xpr is
 * displaying a window with that number.
 */
Bool
xprIsX11Window(void *nsWindow, int windowNumber)
{
    Bool ret;
    xp_window_id wid;

    if (window_hash == NULL)
        return FALSE;

    /* need to lock, since this function can be called by any thread */

    pthread_mutex_lock(&window_hash_mutex);

    if (xp_lookup_native_window(windowNumber, &wid))
        ret = xprGetXWindow(wid) != NULL;
    else
        ret = FALSE;

    pthread_mutex_unlock(&window_hash_mutex);

    return ret;
}


/*
 * xprHideWindows
 *  Hide or unhide all top level windows. This is called for application hide/
 *  unhide events if the window manager is not Apple-WM aware. Xplugin windows
 *  do not hide or unhide themselves.
 */
void
xprHideWindows(Bool hide)
{
    int screen;
    WindowPtr pRoot, pWin;

    for (screen = 0; screen < screenInfo.numScreens; screen++) {
        pRoot = WindowTable[screenInfo.screens[screen]->myNum];
        RootlessFrameID prevWid = NULL;

        for (pWin = pRoot->firstChild; pWin; pWin = pWin->nextSib) {
            RootlessWindowRec *winRec = WINREC(pWin);

            if (winRec != NULL) {
                if (hide) {
                    xprUnmapFrame(winRec->wid);
                } else {
                    BoxRec box;

                    xprRestackFrame(winRec->wid, prevWid);
                    prevWid = winRec->wid;

                    box.x1 = 0;
                    box.y1 = 0;
                    box.x2 = winRec->width;
                    box.y2 = winRec->height;

                    xprDamageRects(winRec->wid, 1, &box, 0, 0);
                    RootlessQueueRedisplay(screenInfo.screens[screen]);
                }
            }
        }
    }
}
