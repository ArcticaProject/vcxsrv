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
    SYMFUNC(miChangeClip)
    SYMFUNC(miChangeGC)
    SYMFUNC(miClearDrawable)
    SYMFUNC(miClearToBackground)
    SYMFUNC(miClearVisualTypes)
    SYMFUNC(miClipSpans)
    SYMFUNC(miComputeCompositeClip)
    SYMFUNC(miCopyClip)
    SYMFUNC(miCopyGC)
    SYMFUNC(miCreateDefColormap)
    SYMFUNC(miCreateScreenResources)
    SYMFUNC(miDCInitialize)
    SYMFUNC(miDestroyClip)
    SYMFUNC(miDestroyGC)
    SYMFUNC(miExpandDirectColors)
    SYMFUNC(miFillArcSetup)
    SYMFUNC(miFillArcSliceSetup)
    SYMFUNC(miFillPolygon)
    SYMFUNC(miGetDefaultVisualMask)
    SYMFUNC(miHandleExposures)
    SYMFUNC(miImageGlyphBlt)
    SYMFUNC(miImageText16)
    SYMFUNC(miImageText8)
    SYMFUNC(miInitOverlay)
    SYMFUNC(miInitVisuals)
    SYMFUNC(miInitializeBackingStore)
    SYMFUNC(miInitializeBanking)
    SYMFUNC(miInitializeColormap)
    SYMFUNC(miInstallColormap)
    SYMFUNC(miIntersect)
    SYMFUNC(miInverse)
    SYMFUNC(miListInstalledColormaps)
    SYMFUNC(miModifyPixmapHeader)
    SYMFUNC(miOverlayCollectUnderlayRegions)
    SYMFUNC(miOverlayComputeCompositeClip)
    SYMFUNC(miOverlayCopyUnderlay)
    SYMFUNC(miOverlayGetPrivateClips)
    SYMFUNC(miOverlaySetRootClip)
    SYMFUNC(miOverlaySetTransFunction)
    SYMFUNC(miPointInRegion)
    SYMFUNC(miPointerAbsoluteCursor)
    SYMFUNC(miPointerCurrentScreen)
    SYMFUNC(miPointerInitialize)
    SYMFUNC(miPointerWarpCursor)
    SYMFUNC(miPolyArc)
    SYMFUNC(miPolyBuildEdge)
    SYMFUNC(miPolyBuildPoly)
    SYMFUNC(miPolyFillArc)
    SYMFUNC(miPolyFillRect)
    SYMFUNC(miPolyGlyphBlt)
    SYMFUNC(miPolyPoint)
    SYMFUNC(miPolyRectangle)
    SYMFUNC(miPolySegment)
    SYMFUNC(miPolyText16)
    SYMFUNC(miPolyText8)
    SYMFUNC(miRectAlloc)
    SYMFUNC(miRectIn)
    SYMFUNC(miRectsToRegion)
    SYMFUNC(miRegionAppend)
    SYMFUNC(miRegionCopy)
    SYMFUNC(miRegionCreate)
    SYMFUNC(miRegionDestroy)
    SYMFUNC(miRegionEmpty)
    SYMFUNC(miRegionEqual)
    SYMFUNC(miRegionExtents)
    SYMFUNC(miRegionInit)
    SYMFUNC(miRegionNotEmpty)
    SYMFUNC(miRegionReset)
    SYMFUNC(miRegionUninit)
    SYMFUNC(miRegionValidate)
    SYMFUNC(miResolveColor)
    SYMFUNC(miRoundCapClip)
    SYMFUNC(miRoundJoinClip)
    SYMFUNC(miScreenInit)
    SYMFUNC(miSegregateChildren)
    SYMFUNC(miSetPixmapDepths)
    SYMFUNC(miSetVisualTypes)
    SYMFUNC(miSetVisualTypesAndMasks)
    SYMFUNC(miSetZeroLineBias)
    SYMFUNC(miSubtract)
    SYMFUNC(miTranslateRegion)
    SYMFUNC(miUninstallColormap)
    SYMFUNC(miUnion)
    SYMFUNC(miWideDash)
    SYMFUNC(miWideLine)
    SYMFUNC(miWindowExposures)
    SYMFUNC(miZeroArcSetup)
    SYMFUNC(miZeroClipLine)
    SYMFUNC(miZeroLine)
    SYMFUNC(miZeroPolyArc)
    SYMVAR(miEmptyBox)
    SYMVAR(miEmptyData)
    SYMVAR(miInstalledMaps)
    SYMVAR(miPointerScreenKey)
    SYMVAR(miSpritePointerFuncs)
    SYMVAR(miZeroLineScreenKey)
#ifdef DAMAGE
    SYMFUNC(DamageDamageRegion)
#endif
};
