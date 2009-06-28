/*
 * Support for accelerated rootless code
 */
/*
 * Copyright (c) 2004 Torrey T. Lyons. All Rights Reserved.
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

/*
 * The accelerated rootless code replaces some GC operations from fb with
 * versions that call the rootless acceleration functions where appropriate.
 * To work properly, this must be wrapped directly on top of fb. Nothing
 * underneath this layer besides fb will get called.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "rootless.h"
#include "rlAccel.h"

typedef struct _rlAccelScreenRec {
    CreateGCProcPtr CreateGC;
    CloseScreenProcPtr CloseScreen;
} rlAccelScreenRec, *rlAccelScreenPtr;

static DevPrivateKey rlAccelScreenPrivateKey = &rlAccelScreenPrivateKey;

#define RLACCELREC(pScreen) ((rlAccelScreenRec *) \
    dixLookupPrivate(&(pScreen)->devPrivates, rlAccelScreenPrivateKey))

#define SETRLACCELREC(pScreen, v) \
    dixSetPrivate(&(pScreen)->devPrivates, rlAccelScreenPrivateKey, v)

/* This is mostly identical to fbGCOps. */
static GCOps rlAccelOps = {
    rlFillSpans,
    fbSetSpans,
    fbPutImage,
    rlCopyArea,
    fbCopyPlane,
    fbPolyPoint,
    fbPolyLine,
    fbPolySegment,
    fbPolyRectangle,
    fbPolyArc,
    miFillPolygon,
    rlPolyFillRect,
    fbPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    rlImageGlyphBlt,
    fbPolyGlyphBlt,
    fbPushPixels
};


/*
 * Screen function to create a graphics context
 */
static Bool
rlCreateGC(GCPtr pGC)
{
    ScreenPtr pScreen = pGC->pScreen;
    rlAccelScreenRec *s = RLACCELREC(pScreen);
    Bool result;

    // Unwrap and call
    pScreen->CreateGC = s->CreateGC;
    result = s->CreateGC(pGC);

    // Accelerated GC ops replace some fb GC ops
    pGC->ops = &rlAccelOps;

    // Rewrap
    s->CreateGC = pScreen->CreateGC;
    pScreen->CreateGC = rlCreateGC;

    return result;
}


/*
 * Clean up when closing a screen on server reset
 */
static Bool
rlCloseScreen (int iScreen, ScreenPtr pScreen)
{
    rlAccelScreenRec *s = RLACCELREC(pScreen);
    Bool result;

    // Unwrap
    pScreen->CloseScreen = s->CloseScreen;
    result = pScreen->CloseScreen(iScreen, pScreen);

    xfree(s);

    return result;
}


/*
 * RootlessAccelInit
 *  Called by the rootless implementation to initialize accelerated
 *  rootless drawing.
 */
Bool
RootlessAccelInit(ScreenPtr pScreen)
{
    rlAccelScreenRec *s;

    s = xalloc(sizeof(rlAccelScreenRec));
    if (!s) return FALSE;
    SETRLACCELREC(pScreen, s);

    // Wrap the screen functions we need
    s->CreateGC = pScreen->CreateGC;
    pScreen->CreateGC = rlCreateGC;
    s->CloseScreen = pScreen->CloseScreen;
    pScreen->CloseScreen = rlCloseScreen;

    return TRUE;
}
