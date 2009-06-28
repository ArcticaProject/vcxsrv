/*
 *
 * Copyright 1995,96 by Metro Link, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Metro Link, Inc. not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Metro Link, Inc. makes no
 * representations about the suitability of this software for any purpose.
 *  It is provided "as is" without express or implied warranty.
 *
 * METRO LINK, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL METRO LINK, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "sym.h"
#include "misc.h"
#include "mi.h"
#include "mibank.h"
#include "miwideline.h"
#include "mibstore.h"
#include "cursor.h"
#include "mipointer.h"
#include "migc.h"
#include "miline.h"
#include "mizerarc.h"
#include "mifillarc.h"
#include "micmap.h"
#include "mioverlay.h"
#ifdef PANORAMIX
#include "resource.h"
#include "panoramiX.h"
#endif
#ifdef RENDER
#include "mipict.h"
#endif
#ifdef COMPOSITE
#include "cw.h"
#endif
#ifdef DAMAGE
#include "damage.h"
#endif

/* mi things */

extern miPointerSpriteFuncRec miSpritePointerFuncs;

_X_HIDDEN void *miLookupTab[] = {
    SYMFUNC(miClearToBackground)
    SYMFUNC(miSendGraphicsExpose)
    SYMFUNC(miModifyPixmapHeader)
    SYMFUNC(miHandleValidateExposures)
    SYMFUNC(miSetShape)
    SYMFUNC(miChangeBorderWidth)
    SYMFUNC(miShapedWindowIn)
    SYMFUNC(miRectIn)
    SYMFUNC(miZeroClipLine)
    SYMFUNC(miZeroDashLine)
    SYMFUNC(miClearDrawable)
    SYMFUNC(miPolyPoint)
    SYMFUNC(miStepDash)
    SYMFUNC(miIntersect)
    SYMFUNC(miRegionAppend)
    SYMFUNC(miRegionCopy)
    SYMFUNC(miRegionDestroy)
    SYMFUNC(miRegionEmpty)
    SYMFUNC(miRegionExtents)
    SYMFUNC(miRegionInit)
    SYMFUNC(miRegionNotEmpty)
    SYMFUNC(miRegionEqual)
    SYMFUNC(miRegionReset)
    SYMFUNC(miRegionUninit)
    SYMFUNC(miRegionValidate)
    SYMFUNC(miTranslateRegion)
    SYMFUNC(miHandleExposures)
    SYMFUNC(miPolyFillRect)
    SYMFUNC(miPolyFillArc)
    SYMFUNC(miImageGlyphBlt)
    SYMFUNC(miPolyGlyphBlt)
    SYMFUNC(miFillPolygon)
    SYMFUNC(miFillConvexPoly)
    SYMFUNC(miPolySegment)
    SYMFUNC(miZeroLine)
    SYMFUNC(miWideLine)
    SYMFUNC(miWideDash)
    SYMFUNC(miZeroPolyArc)
    SYMFUNC(miPolyArc)
    SYMFUNC(miCreateGCOps)
    SYMFUNC(miDestroyGCOps)
    SYMFUNC(miComputeCompositeClip)
    SYMFUNC(miChangeGC)
    SYMFUNC(miCopyGC)
    SYMFUNC(miDestroyGC)
    SYMFUNC(miChangeClip)
    SYMFUNC(miDestroyClip)
    SYMFUNC(miCopyClip)
    SYMFUNC(miPolyRectangle)
    SYMFUNC(miPolyText8)
    SYMFUNC(miPolyText16)
    SYMFUNC(miImageText8)
    SYMFUNC(miImageText16)
    SYMFUNC(miRegionCreate)
    SYMFUNC(miPaintWindow)
    SYMFUNC(miZeroArcSetup)
    SYMFUNC(miFillArcSetup)
    SYMFUNC(miFillArcSliceSetup)
    SYMFUNC(miFindMaxBand)
    SYMFUNC(miClipSpans)
    SYMFUNC(miAllocateGCPrivateIndex)
    SYMFUNC(miScreenInit)
    SYMFUNC(miGetScreenPixmap)
    SYMFUNC(miSetScreenPixmap)
    SYMFUNC(miPointerCurrentScreen)
    SYMFUNC(miRectAlloc)
    SYMFUNC(miInitializeBackingStore)
    SYMFUNC(miInitializeBanking)
    SYMFUNC(miModifyBanking)
    SYMFUNC(miCopyPlane)
    SYMFUNC(miCopyArea)
    SYMFUNC(miCreateScreenResources)
    SYMFUNC(miGetImage)
    SYMFUNC(miPutImage)
    SYMFUNC(miPushPixels)
    SYMFUNC(miPointerInitialize)
    SYMFUNC(miPointerPosition)
    SYMFUNC(miRecolorCursor)
    SYMFUNC(miPointerWarpCursor)
    SYMFUNC(miDCInitialize)
    SYMFUNC(miRectsToRegion)
    SYMFUNC(miPointInRegion)
    SYMFUNC(miInverse)
    SYMFUNC(miSubtract)
    SYMFUNC(miUnion)
    SYMFUNC(miPolyBuildEdge)
    SYMFUNC(miPolyBuildPoly)
    SYMFUNC(miRoundJoinClip)
    SYMFUNC(miRoundCapClip)
    SYMFUNC(miSetZeroLineBias)
    SYMFUNC(miResolveColor)
    SYMFUNC(miInitializeColormap)
    SYMFUNC(miInstallColormap)
    SYMFUNC(miUninstallColormap)
    SYMFUNC(miListInstalledColormaps)
    SYMFUNC(miExpandDirectColors)
    SYMFUNC(miCreateDefColormap)
    SYMFUNC(miClearVisualTypes)
    SYMFUNC(miSetVisualTypes)
    SYMFUNC(miSetVisualTypesAndMasks)
    SYMFUNC(miGetDefaultVisualMask)
    SYMFUNC(miSetPixmapDepths)
    SYMFUNC(miInitVisuals)
    SYMFUNC(miWindowExposures)
    SYMFUNC(miSegregateChildren)
    SYMFUNC(miHookInitVisuals)
    SYMFUNC(miPointerAbsoluteCursor)
    SYMFUNC(miOverlayCopyUnderlay)
    SYMFUNC(miOverlaySetTransFunction)
    SYMFUNC(miOverlayCollectUnderlayRegions)
    SYMFUNC(miInitOverlay)
    SYMFUNC(miOverlayComputeCompositeClip)
    SYMFUNC(miOverlayGetPrivateClips)
    SYMFUNC(miOverlaySetRootClip)
    SYMVAR(miEmptyBox)
    SYMVAR(miEmptyData)
    SYMVAR(miZeroLineScreenKey)
    SYMVAR(miSpritePointerFuncs)
    SYMVAR(miPointerScreenKey)
    SYMVAR(miInstalledMaps)
    SYMVAR(miInitVisualsProc)
#ifdef DAMAGE
    SYMFUNC(DamageDamageRegion)
#endif
};
