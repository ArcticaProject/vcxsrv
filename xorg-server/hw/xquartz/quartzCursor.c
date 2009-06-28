/**************************************************************
 *
 * Support for using the Quartz Window Manager cursor
 *
 * Copyright (c) 2001-2003 Torrey T. Lyons and Greg Parker.
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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "quartzCommon.h"
#include "quartzCursor.h"
#include "darwin.h"

#include <pthread.h>

#include "mi.h"
#include "scrnintstr.h"
#include "cursorstr.h"
#include "mipointrst.h"
#include "globals.h"

// Size of the QuickDraw cursor
#define CURSORWIDTH 16
#define CURSORHEIGHT 16

typedef struct {
    int                     qdCursorMode;
    int                     qdCursorVisible;
    int                     useQDCursor;
    QueryBestSizeProcPtr    QueryBestSize;
    miPointerSpriteFuncPtr  spriteFuncs;
} QuartzCursorScreenRec, *QuartzCursorScreenPtr;

static DevPrivateKey darwinCursorScreenKey = &darwinCursorScreenKey;
static CursorPtr quartzLatentCursor = NULL;
static QD_Cursor gQDArrow; // QuickDraw arrow cursor

// Cursor for the main thread to set (NULL = arrow cursor).
static CCrsrHandle currentCursor = NULL;
static pthread_mutex_t cursorMutex;
static pthread_cond_t cursorCondition;

#define CURSOR_PRIV(pScreen) ((QuartzCursorScreenPtr) \
    dixLookupPrivate(&pScreen->devPrivates, darwinCursorScreenKey))

#define HIDE_QD_CURSOR(pScreen, visible)                                \
    if (visible) {                                                      \
        int ix;                                                         \
        for (ix = 0; ix < QUARTZ_PRIV(pScreen)->displayCount; ix++) {   \
            CGDisplayHideCursor(QUARTZ_PRIV(pScreen)->displayIDs[ix]);  \
        }                                                               \
        visible = FALSE;                                                \
    } ((void)0)

#define SHOW_QD_CURSOR(pScreen, visible)                                \
    {                                                                   \
        int ix;                                                         \
        for (ix = 0; ix < QUARTZ_PRIV(pScreen)->displayCount; ix++) {   \
            CGDisplayShowCursor(QUARTZ_PRIV(pScreen)->displayIDs[ix]);  \
        }                                                               \
        visible = TRUE;                                                 \
    } ((void)0)

#define CHANGE_QD_CURSOR(cursorH)                                       \
    if (!quartzServerQuitting) {                                        \
        /* Acquire lock and tell the main thread to change cursor */    \
        pthread_mutex_lock(&cursorMutex);                               \
        currentCursor = (CCrsrHandle) (cursorH);                        \
        QuartzMessageMainThread(kQuartzCursorUpdate, NULL, 0);          \
                                                                        \
        /* Wait for the main thread to change the cursor */             \
        pthread_cond_wait(&cursorCondition, &cursorMutex);              \
        pthread_mutex_unlock(&cursorMutex);                             \
    } ((void)0)


/*
 * MakeQDCursor helpers: CTAB_ENTER, interleave
 */

// Add a color entry to a ctab
#define CTAB_ENTER(ctab, index, r, g, b)                                \
    ctab->ctTable[index].value = index;                                 \
    ctab->ctTable[index].rgb.red = r;                                   \
    ctab->ctTable[index].rgb.green = g;                                 \
    ctab->ctTable[index].rgb.blue = b

// Make an unsigned short by interleaving the bits of bytes c1 and c2.
// High bit of c1 is first; low bit of c2 is last.
// Interleave is a built-in INTERCAL operator.
static unsigned short
interleave(
    unsigned char c1,
    unsigned char c2 )
{
    return
        ((c1 & 0x80) << 8) | ((c2 & 0x80) << 7) |
        ((c1 & 0x40) << 7) | ((c2 & 0x40) << 6) |
        ((c1 & 0x20) << 6) | ((c2 & 0x20) << 5) |
        ((c1 & 0x10) << 5) | ((c2 & 0x10) << 4) |
        ((c1 & 0x08) << 4) | ((c2 & 0x08) << 3) |
        ((c1 & 0x04) << 3) | ((c2 & 0x04) << 2) |
        ((c1 & 0x02) << 2) | ((c2 & 0x02) << 1) |
        ((c1 & 0x01) << 1) | ((c2 & 0x01) << 0) ;
}

/*
 * MakeQDCursor
 * Make a QuickDraw color cursor from the given X11 cursor.
 * Warning: This code is nasty. Color cursors were meant to be read
 * from resources; constructing the structures programmatically is messy.
 */
/*
    QuickDraw cursor representation:
    Our color cursor is a 2 bit per pixel pixmap.
    Each pixel's bits are (source<<1 | mask) from the original X cursor pixel.
    The cursor's color table maps the colors like this:
    (2-bit value | X result    | colortable | Mac result)
             00  | transparent | white      | transparent (white outside mask)
             01  | back color  | back color | back color
             10  | undefined   | black      | invert background (just for fun)
             11  | fore color  | fore color | fore color
*/
static CCrsrHandle
MakeQDCursor(
    CursorPtr pCursor )
{
    CCrsrHandle result;
    CCrsrPtr curs;
    int i, w, h;
    unsigned short rowMask;
    PixMap *pix;
    ColorTable *ctab;
    unsigned short *image;

    result = (CCrsrHandle) NewHandleClear(sizeof(CCrsr));
    if (!result) return NULL;
    HLock((Handle)result);
    curs = *result;

    // Initialize CCrsr
    curs->crsrType = 0x8001;     // 0x8000 = b&w, 0x8001 = color
    curs->crsrMap = (PixMapHandle) NewHandleClear(sizeof(PixMap));
    if (!curs->crsrMap) goto pixAllocFailed;
    HLock((Handle)curs->crsrMap);
    pix = *curs->crsrMap;
    curs->crsrData = NULL;       // raw cursor image data (set below)
    curs->crsrXData = NULL;      // QD's processed data
    curs->crsrXValid = 0;        // zero means QD must re-process cursor data
    curs->crsrXHandle = NULL;    // reserved
    memset(curs->crsr1Data, 0, CURSORWIDTH*CURSORHEIGHT/8); // b&w data
    memset(curs->crsrMask,  0, CURSORWIDTH*CURSORHEIGHT/8); // b&w & color mask
    curs->crsrHotSpot.h = min(CURSORWIDTH,  pCursor->bits->xhot); // hot spot
    curs->crsrHotSpot.v = min(CURSORHEIGHT, pCursor->bits->yhot); // hot spot
    curs->crsrXTable = 0;        // reserved
    curs->crsrID = GetCTSeed();  // unique ID from Color Manager

    // Set the b&w data and mask
    w = min(pCursor->bits->width,  CURSORWIDTH);
    h = min(pCursor->bits->height, CURSORHEIGHT);
    rowMask = ~((1 << (CURSORWIDTH - w)) - 1);
    for (i = 0; i < h; i++) {
        curs->crsr1Data[i] = rowMask &
        ((pCursor->bits->source[i*4]<<8) | pCursor->bits->source[i*4+1]);
        curs->crsrMask[i] = rowMask &
        ((pCursor->bits->mask[i*4]<<8)   | pCursor->bits->mask[i*4+1]);
    }

    // Set the color data and mask
    // crsrMap: defines bit depth and size and colortable only
    pix->rowBytes = (CURSORWIDTH * 2 / 8) | 0x8000; // last bit on means PixMap
    SetRect(&pix->bounds, 0, 0, CURSORWIDTH, CURSORHEIGHT); // see TN 1020
    pix->pixelSize = 2;
    pix->cmpCount = 1;
    pix->cmpSize = 2;
    // pix->pmTable set below

    // crsrData is the pixel data. crsrMap's baseAddr is not used.
    curs->crsrData = NewHandleClear(CURSORWIDTH*CURSORHEIGHT * 2 / 8);
    if (!curs->crsrData) goto imageAllocFailed;
    HLock((Handle)curs->crsrData);
    image = (unsigned short *) *curs->crsrData;
    // Pixel data is just 1-bit data and mask interleaved (see above)
    for (i = 0; i < h; i++) {
        unsigned char s, m;
        s = pCursor->bits->source[i*4] & (rowMask >> 8);
        m = pCursor->bits->mask[i*4] & (rowMask >> 8);
        image[2*i] = interleave(s, m);
        s = pCursor->bits->source[i*4+1] & (rowMask & 0x00ff);
        m = pCursor->bits->mask[i*4+1] & (rowMask & 0x00ff);
        image[2*i+1] = interleave(s, m);
    }

    // Build the color table (entries described above)
    // NewPixMap allocates a color table handle.
    pix->pmTable = (CTabHandle) NewHandleClear(sizeof(ColorTable) + 3
                    * sizeof(ColorSpec));
    if (!pix->pmTable) goto ctabAllocFailed;
    HLock((Handle)pix->pmTable);
    ctab = *pix->pmTable;
    ctab->ctSeed = GetCTSeed();
    ctab->ctFlags = 0;
    ctab->ctSize = 3; // color count - 1
    CTAB_ENTER(ctab, 0, 0xffff, 0xffff, 0xffff);
    CTAB_ENTER(ctab, 1, pCursor->backRed, pCursor->backGreen,
               pCursor->backBlue);
    CTAB_ENTER(ctab, 2, 0x0000, 0x0000, 0x0000);
    CTAB_ENTER(ctab, 3, pCursor->foreRed, pCursor->foreGreen,
               pCursor->foreBlue);

    HUnlock((Handle)pix->pmTable); // ctab
    HUnlock((Handle)curs->crsrData); // image data
    HUnlock((Handle)curs->crsrMap); // pix
    HUnlock((Handle)result); // cursor

    return result;

    // "What we have here is a failure to allocate"
ctabAllocFailed:
    HUnlock((Handle)curs->crsrData);
    DisposeHandle((Handle)curs->crsrData);
imageAllocFailed:
    HUnlock((Handle)curs->crsrMap);
    DisposeHandle((Handle)curs->crsrMap);
pixAllocFailed:
    HUnlock((Handle)result);
    DisposeHandle((Handle)result);
    return NULL;
}


/*
 * FreeQDCursor
 * Destroy a QuickDraw color cursor created with MakeQDCursor().
 * The cursor must not currently be on screen.
 */
static void FreeQDCursor(CCrsrHandle cursHandle)
{
    CCrsrPtr curs;
    PixMap *pix;

    HLock((Handle)cursHandle);
    curs = *cursHandle;
    HLock((Handle)curs->crsrMap);
    pix = *curs->crsrMap;
    DisposeHandle((Handle)pix->pmTable);
    HUnlock((Handle)curs->crsrMap);
    DisposeHandle((Handle)curs->crsrMap);
    DisposeHandle((Handle)curs->crsrData);
    HUnlock((Handle)cursHandle);
    DisposeHandle((Handle)cursHandle);
}


/*
===========================================================================

 Pointer sprite functions

===========================================================================
*/

/*
 * QuartzRealizeCursor
 * Convert the X cursor representation to QuickDraw format if possible.
 */
Bool
QuartzRealizeCursor(
    ScreenPtr pScreen,
    CursorPtr pCursor )
{
    CCrsrHandle qdCursor;
    QuartzCursorScreenPtr ScreenPriv = CURSOR_PRIV(pScreen);

    if(!pCursor || !pCursor->bits)
        return FALSE;

    // if the cursor is too big we use a software cursor
    if ((pCursor->bits->height > CURSORHEIGHT) ||
        (pCursor->bits->width > CURSORWIDTH) || !ScreenPriv->useQDCursor)
    {
        if (quartzRootless) {
            // rootless can't use a software cursor
            return TRUE;
        } else {
            return (*ScreenPriv->spriteFuncs->RealizeCursor)
                        (pScreen, pCursor);
        }
    }

    // make new cursor image
    qdCursor = MakeQDCursor(pCursor);
    if (!qdCursor) return FALSE;

    // save the result
    dixSetPrivate(&pCursor->devPrivates, pScreen, qdCursor);

    return TRUE;
}


/*
 * QuartzUnrealizeCursor
 * Free the storage space associated with a realized cursor.
 */
Bool
QuartzUnrealizeCursor(
    ScreenPtr pScreen,
    CursorPtr pCursor )
{
    QuartzCursorScreenPtr ScreenPriv = CURSOR_PRIV(pScreen);

    if ((pCursor->bits->height > CURSORHEIGHT) ||
        (pCursor->bits->width > CURSORWIDTH) || !ScreenPriv->useQDCursor)
    {
        if (quartzRootless) {
            return TRUE;
        } else {
            return (*ScreenPriv->spriteFuncs->UnrealizeCursor)
                        (pScreen, pCursor);
        }
    } else {
        CCrsrHandle oldCursor = dixLookupPrivate(&pCursor->devPrivates,
						 pScreen);
        if (currentCursor != oldCursor) {
            // This should only fail when quitting, in which case we just leak.
            FreeQDCursor(oldCursor);
        }
	dixSetPrivate(&pCursor->devPrivates, pScreen, NULL);
        return TRUE;
    }
}


/*
 * QuartzSetCursor
 * Set the cursor sprite and position.
 * Use QuickDraw cursor if possible.
 */
static void
QuartzSetCursor(
    ScreenPtr       pScreen,
    CursorPtr       pCursor,
    int             x,
    int             y)
{
    QuartzCursorScreenPtr ScreenPriv = CURSOR_PRIV(pScreen);

    quartzLatentCursor = pCursor;

    // Don't touch Mac OS cursor if X is hidden!
    if (!quartzServerVisible)
        return;

    if (!pCursor) {
        // Remove the cursor completely.
        HIDE_QD_CURSOR(pScreen, ScreenPriv->qdCursorVisible);
        if (! ScreenPriv->qdCursorMode)
            (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, 0, x, y);
    }
    else if ((pCursor->bits->height <= CURSORHEIGHT) &&
             (pCursor->bits->width <= CURSORWIDTH) && ScreenPriv->useQDCursor)
    {
        // Cursor is small enough to use QuickDraw directly.
        if (! ScreenPriv->qdCursorMode)    // remove the X cursor
            (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, 0, x, y);
        ScreenPriv->qdCursorMode = TRUE;

        CHANGE_QD_CURSOR(dixLookupPrivate(&pCursor->devPrivates, pScreen));
        SHOW_QD_CURSOR(pScreen, ScreenPriv->qdCursorVisible);
    }
    else if (quartzRootless) {
        // Rootless can't use a software cursor, so we just use Mac OS arrow.
        CHANGE_QD_CURSOR(NULL);
        SHOW_QD_CURSOR(pScreen, ScreenPriv->qdCursorVisible);
    }
    else {
        // Cursor is too big for QuickDraw. Use X software cursor.
        HIDE_QD_CURSOR(pScreen, ScreenPriv->qdCursorVisible);
        ScreenPriv->qdCursorMode = FALSE;
        (*ScreenPriv->spriteFuncs->SetCursor)(pScreen, pCursor, x, y);
    }
}


/*
 * QuartzReallySetCursor
 * Set the QuickDraw cursor. Called from the main thread since changing the
 * cursor with QuickDraw is not thread safe on dual processor machines.
 */
void
QuartzReallySetCursor()
{
    pthread_mutex_lock(&cursorMutex);

    if (currentCursor) {
        SetCCursor(currentCursor);
    } else {
        SetCursor(&gQDArrow);
    }

    pthread_cond_signal(&cursorCondition);
    pthread_mutex_unlock(&cursorMutex);
}


/*
 * QuartzMoveCursor
 * Move the cursor. This is a noop for QuickDraw.
 */
static void
QuartzMoveCursor(
    ScreenPtr   pScreen,
    int         x,
    int         y)
{
    QuartzCursorScreenPtr ScreenPriv = CURSOR_PRIV(pScreen);

    // only the X cursor needs to be explicitly moved
    if (!ScreenPriv->qdCursorMode)
        (*ScreenPriv->spriteFuncs->MoveCursor)(pScreen, x, y);
}


static miPointerSpriteFuncRec quartzSpriteFuncsRec = {
    QuartzRealizeCursor,
    QuartzUnrealizeCursor,
    QuartzSetCursor,
    QuartzMoveCursor
};


/*
===========================================================================

 Pointer screen functions

===========================================================================
*/

/*
 * QuartzCursorOffScreen
 */
static Bool QuartzCursorOffScreen(ScreenPtr *pScreen, int *x, int *y)
{
    return FALSE;
}


/*
 * QuartzCrossScreen
 */
static void QuartzCrossScreen(ScreenPtr pScreen, Bool entering)
{
    return;
}


/*
 * QuartzWarpCursor
 *  Change the cursor position without generating an event or motion history.
 *  The input coordinates (x,y) are in pScreen-local X11 coordinates.
 *
 */
static void
QuartzWarpCursor(
    ScreenPtr               pScreen,
    int                     x,
    int                     y)
{
    static int              neverMoved = TRUE;

    if (neverMoved) {
        // Don't move the cursor the first time. This is the jump-to-center
        // initialization, and it's annoying because we may still be in MacOS.
        neverMoved = FALSE;
        return;
    }

    if (quartzServerVisible) {
        CGDisplayErr        cgErr;
        CGPoint             cgPoint;
        // Only need to do this for one display. Any display will do.
        CGDirectDisplayID   cgID = QUARTZ_PRIV(pScreen)->displayIDs[0];
        CGRect              cgRect = CGDisplayBounds(cgID);

        // Convert (x,y) to CoreGraphics screen-local CG coordinates.
        // This is necessary because the X11 screen and CG screen may not
        // coincide. (e.g. X11 screen may be moved to dodge the menu bar)

        // Make point in X11 global coordinates
        cgPoint = CGPointMake(x + dixScreenOrigins[pScreen->myNum].x,
                              y + dixScreenOrigins[pScreen->myNum].y);
        // Shift to CoreGraphics global screen coordinates
        cgPoint.x += darwinMainScreenX;
        cgPoint.y += darwinMainScreenY;
        // Shift to CoreGraphics screen-local coordinates
        cgPoint.x -= cgRect.origin.x;
        cgPoint.y -= cgRect.origin.y;

        cgErr = CGDisplayMoveCursorToPoint(cgID, cgPoint);
        if (cgErr != CGDisplayNoErr) {
            ErrorF("Could not set cursor position with error code 0x%x.\n",
                    cgErr);
        }
    }

    miPointerWarpCursor(pScreen, x, y);
    miPointerUpdate();
}


static miPointerScreenFuncRec quartzScreenFuncsRec = {
    QuartzCursorOffScreen,
    QuartzCrossScreen,
    QuartzWarpCursor,
    DarwinEQPointerPost,
    DarwinEQSwitchScreen
};


/*
===========================================================================

 Other screen functions

===========================================================================
*/

/*
 * QuartzCursorQueryBestSize
 * Handle queries for best cursor size
 */
static void
QuartzCursorQueryBestSize(
   int              class,
   unsigned short   *width,
   unsigned short   *height,
   ScreenPtr        pScreen)
{
    QuartzCursorScreenPtr ScreenPriv = CURSOR_PRIV(pScreen);

    if (class == CursorShape) {
        *width = CURSORWIDTH;
        *height = CURSORHEIGHT;
    } else {
        (*ScreenPriv->QueryBestSize)(class, width, height, pScreen);
    }
}


/*
 * QuartzInitCursor
 * Initialize cursor support
 */
Bool
QuartzInitCursor(
    ScreenPtr   pScreen )
{
    QuartzCursorScreenPtr   ScreenPriv;
    miPointerScreenPtr      PointPriv;
    DarwinFramebufferPtr    dfb = SCREEN_PRIV(pScreen);

    // initialize software cursor handling (always needed as backup)
    if (!miDCInitialize(pScreen, &quartzScreenFuncsRec)) {
        return FALSE;
    }

    ScreenPriv = xcalloc( 1, sizeof(QuartzCursorScreenRec) );
    if (!ScreenPriv) return FALSE;

    CURSOR_PRIV(pScreen) = ScreenPriv;

    // override some screen procedures
    ScreenPriv->QueryBestSize = pScreen->QueryBestSize;
    pScreen->QueryBestSize = QuartzCursorQueryBestSize;

    // initialize QuickDraw cursor handling
    GetQDGlobalsArrow(&gQDArrow);
    PointPriv = (miPointerScreenPtr)
	dixLookupPrivate(&pScreen->devPrivates, miPointerScreenKey);

    ScreenPriv->spriteFuncs = PointPriv->spriteFuncs;
    PointPriv->spriteFuncs = &quartzSpriteFuncsRec;

    if (!quartzRootless)
        ScreenPriv->useQDCursor = QuartzFSUseQDCursor(dfb->colorBitsPerPixel);
    else
        ScreenPriv->useQDCursor = TRUE;
    ScreenPriv->qdCursorMode = TRUE;
    ScreenPriv->qdCursorVisible = TRUE;

    // initialize cursor mutex lock
    pthread_mutex_init(&cursorMutex, NULL);

    // initialize condition for waiting
    pthread_cond_init(&cursorCondition, NULL);

    return TRUE;
}


// X server is hiding. Restore the Aqua cursor.
void QuartzSuspendXCursor(
    ScreenPtr pScreen )
{
    QuartzCursorScreenPtr ScreenPriv = CURSOR_PRIV(pScreen);

    CHANGE_QD_CURSOR(NULL);
    SHOW_QD_CURSOR(pScreen, ScreenPriv->qdCursorVisible);
}


// X server is showing. Restore the X cursor.
void QuartzResumeXCursor(
    ScreenPtr pScreen,
    int x,
    int y )
{
    QuartzSetCursor(pScreen, quartzLatentCursor, x, y);
}
