/***********************************************************

Copyright 1987, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/


/*

PSZ		8	16	24	32
PIXEL_ADDR	True	True	True	True
NO_ONE_RECT	False	False	False	False
WriteBitGroup	True	True	True	True
FOUR_BIT_CODE	True	False	False	False
LOWMEMFTPT	False	False	False	False

*/


/* This gets built twice.  Once for 8bpp and another for 32bpp */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xproto.h>
#include "cfb.h"
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "region.h"

#include "mistruct.h"
#include "mibstore.h"
#include "migc.h"
#include "mioverlay.h"

#include "cfb8_32.h"
#include "cfbmskbits.h"
#include "cfb8bit.h"


#if PSZ == 8
# define useTEGlyphBlt  cfbTEGlyphBlt8
#else
# ifdef WriteBitGroup
#  define useTEGlyphBlt cfbImageGlyphBlt8
# else
#  define useTEGlyphBlt cfbTEGlyphBlt
# endif
#endif

#ifdef WriteBitGroup
# define useImageGlyphBlt       cfbImageGlyphBlt8
# define usePolyGlyphBlt        cfbPolyGlyphBlt8
#else
# define useImageGlyphBlt       miImageGlyphBlt
# define usePolyGlyphBlt        miPolyGlyphBlt
#endif

#ifdef FOUR_BIT_CODE
# define usePushPixels  cfbPushPixels8
#else
# define usePushPixels  mfbPushPixels
#endif

#ifdef PIXEL_ADDR
# define ZeroPolyArc    cfbZeroPolyArcSS8Copy
#else
# define ZeroPolyArc    miZeroPolyArc
#endif


static GCOps cfb8_32TEOps1Rect = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfb8_32PutImage,
    cfb8_32CopyArea,
    cfb8_32CopyPlane,
    cfbPolyPoint,
#ifdef PIXEL_ADDR
    cfb8LineSS1Rect,
    cfb8SegmentSS1Rect,
#else
    cfbLineSS,
    cfbSegmentSS,
#endif
    miPolyRectangle,
    ZeroPolyArc,
    cfbFillPoly1RectCopy,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    useTEGlyphBlt,
    usePolyGlyphBlt,
    usePushPixels
};

static GCOps cfb8_32NonTEOps1Rect = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfb8_32PutImage,
    cfb8_32CopyArea,
    cfb8_32CopyPlane,
    cfbPolyPoint,
#ifdef PIXEL_ADDR
    cfb8LineSS1Rect,
    cfb8SegmentSS1Rect,
#else
    cfbLineSS,
    cfbSegmentSS,
#endif
    miPolyRectangle,
    ZeroPolyArc,
    cfbFillPoly1RectCopy,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    useImageGlyphBlt,
    usePolyGlyphBlt,
    usePushPixels
};

static GCOps cfb8_32TEOps = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfb8_32PutImage,
    cfb8_32CopyArea,
    cfb8_32CopyPlane,
    cfbPolyPoint,
    cfbLineSS,
    cfbSegmentSS,
    miPolyRectangle,
    ZeroPolyArc,
    miFillPolygon,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    useTEGlyphBlt,
    usePolyGlyphBlt,
    usePushPixels
};

static GCOps cfb8_32NonTEOps = {
    cfbSolidSpansCopy,
    cfbSetSpans,
    cfb8_32PutImage,
    cfb8_32CopyArea,
    cfb8_32CopyPlane,
    cfbPolyPoint,
    cfbLineSS,
    cfbSegmentSS,
    miPolyRectangle,
#ifdef PIXEL_ADDR
    cfbZeroPolyArcSS8Copy,
#else
    miZeroPolyArc,
#endif
    miFillPolygon,
    cfbPolyFillRect,
    cfbPolyFillArcSolidCopy,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    useImageGlyphBlt,
    usePolyGlyphBlt,
    usePushPixels
};

static GCOps *
cfb8_32MatchCommon (GCPtr pGC, cfbPrivGCPtr devPriv)
{
    if (pGC->lineWidth != 0)
	return 0;
    if (pGC->lineStyle != LineSolid)
	return 0;
    if (pGC->fillStyle != FillSolid)
	return 0;
    if (devPriv->rop != GXcopy)
	return 0;
    if (pGC->font &&
	FONTMAXBOUNDS(pGC->font,rightSideBearing) -
        FONTMINBOUNDS(pGC->font,leftSideBearing) <= 32 &&
	FONTMINBOUNDS(pGC->font,characterWidth) >= 0)
    {
	if (TERMINALFONT(pGC->font)
#ifdef FOUR_BIT_CODE
	    && FONTMAXBOUNDS(pGC->font,characterWidth) >= PGSZB
#endif
	)
#ifdef NO_ONE_RECT
            return &cfb8_32TEOps1Rect;
#else
	    if (devPriv->oneRect)
		return &cfb8_32TEOps1Rect;
	    else
		return &cfb8_32TEOps;
#endif
	else
#ifdef NO_ONE_RECT
	    return &cfb8_32NonTEOps1Rect;
#else
	    if (devPriv->oneRect)
		return &cfb8_32NonTEOps1Rect;
	    else
		return &cfb8_32NonTEOps;
#endif
    }
    return 0;
}


/* Clipping conventions
	if the drawable is a window
	    CT_REGION ==> pCompositeClip really is the composite
	    CT_other ==> pCompositeClip is the window clip region
	if the drawable is a pixmap
	    CT_REGION ==> pCompositeClip is the translated client region
		clipped to the pixmap boundary
	    CT_other ==> pCompositeClip is the pixmap bounding box
*/

void
#if PSZ == 8
cfb8_32ValidateGC8(
#else
cfb8_32ValidateGC32(
#endif
    GCPtr  		pGC,
    unsigned long 	changes,
    DrawablePtr		pDrawable
){
    int         mask;		/* stateChanges */
    int         index;		/* used for stepping through bitfields */
    int		new_rrop;
    int         new_line, new_text, new_fillspans, new_fillarea;
    /* flags for changing the proc vector */
    cfbPrivGCPtr devPriv;
    int		oneRect;

    pGC->lastWinOrg.x = pDrawable->x;
    pGC->lastWinOrg.y = pDrawable->y;
    devPriv = cfbGetGCPrivate(pGC);

    new_rrop = FALSE;
    new_line = FALSE;
    new_text = FALSE;
    new_fillspans = FALSE;
    new_fillarea = FALSE;

    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */

    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS)))
    {
	miComputeCompositeClip (pGC, pDrawable);
#ifdef NO_ONE_RECT
	devPriv->oneRect = FALSE;
#else
	oneRect = REGION_NUM_RECTS(pGC->pCompositeClip) == 1;
	if (oneRect != devPriv->oneRect)
	    new_line = TRUE;
	devPriv->oneRect = oneRect;
#endif
    }

    mask = changes;
    while (mask) {
	index = lowbit (mask);
	mask &= ~index;

	switch (index) {
	case GCFunction:
	case GCForeground:
	    new_rrop = TRUE;
	    break;
	case GCPlaneMask:
	    new_rrop = TRUE;
	    new_text = TRUE;
	    break;
	case GCBackground:
	    break;
	case GCLineStyle:
	case GCLineWidth:
	    new_line = TRUE;
	    break;
	case GCJoinStyle:
	case GCCapStyle:
	    break;
	case GCFillStyle:
	    new_text = TRUE;
	    new_fillspans = TRUE;
	    new_line = TRUE;
	    new_fillarea = TRUE;
	    break;
	case GCFillRule:
	    break;
	case GCTile:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;
	case GCStipple:
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	    break;
	case GCTileStipXOrigin:
	case GCTileStipYOrigin:
	    break;
	case GCFont:
	    new_text = TRUE;
	    break;
	case GCSubwindowMode:
	case GCGraphicsExposures:
	case GCClipXOrigin:
	case GCClipYOrigin:
	case GCClipMask:
	case GCDashOffset:
	case GCDashList:
	case GCArcMode:
	default:
	    break;
	}
    }

    /*
     * If the drawable has changed,  ensure suitable
     * entries are in the proc vector. 
     */
    if (pDrawable->serialNumber != (pGC->serialNumber & (DRAWABLE_SERIAL_BITS)))
	new_fillspans = TRUE;	/* deal with FillSpans later */

    if (new_rrop)
    {
	int old_rrop;

	old_rrop = devPriv->rop;
	devPriv->rop = cfbReduceRasterOp (pGC->alu, pGC->fgPixel,
					   pGC->planemask,
					   &devPriv->and, &devPriv->xor);
	if (old_rrop == devPriv->rop)
	    new_rrop = FALSE;
	else
	{
#ifdef PIXEL_ADDR
	    new_line = TRUE;
#endif
#ifdef WriteBitGroup
	    new_text = TRUE;
#endif
	    new_fillspans = TRUE;
	    new_fillarea = TRUE;
	}
    }

    if(!pGC->ops)
	pGC->ops = & cfb8_32NonTEOps;

    if (new_rrop || new_fillspans || new_text || new_fillarea || new_line)
    {
	GCOps	*newops;

	if ((newops = cfb8_32MatchCommon (pGC, devPriv)))
 	{
	    if (pGC->ops->devPrivate.val)
		miDestroyGCOps (pGC->ops);
	    pGC->ops = newops;
	    new_rrop = new_line = new_fillspans = new_text = new_fillarea = 0;
	}
 	else
 	{
	    if (!pGC->ops->devPrivate.val)
	    {
		pGC->ops = miCreateGCOps (pGC->ops);
		pGC->ops->devPrivate.val = 1;
	    }
	}
    }

    /* deal with the changes we've collected */
    if (new_line)
    {
	pGC->ops->FillPolygon = miFillPolygon;
#ifdef NO_ONE_RECT
	if (pGC->fillStyle == FillSolid)
	{
	    switch (devPriv->rop) {
	    case GXcopy:
		pGC->ops->FillPolygon = cfbFillPoly1RectCopy;
		break;
	    default:
		pGC->ops->FillPolygon = cfbFillPoly1RectGeneral;
		break;
	    }
	}
#else
	if (devPriv->oneRect && pGC->fillStyle == FillSolid)
	{
	    switch (devPriv->rop) {
	    case GXcopy:
		pGC->ops->FillPolygon = cfbFillPoly1RectCopy;
		break;
	    default:
		pGC->ops->FillPolygon = cfbFillPoly1RectGeneral;
		break;
	    }
	}
#endif
	if (pGC->lineWidth == 0)
	{
#ifdef PIXEL_ADDR
	    if ((pGC->lineStyle == LineSolid) && (pGC->fillStyle == FillSolid))
	    {
		switch (devPriv->rop)
		{
		case GXxor:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8Xor;
		    break;
		case GXcopy:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8Copy;
		    break;
		default:
		    pGC->ops->PolyArc = cfbZeroPolyArcSS8General;
		    break;
		}
	    }
	    else
#endif
		pGC->ops->PolyArc = miZeroPolyArc;
	}
	else
	    pGC->ops->PolyArc = miPolyArc;
	pGC->ops->PolySegment = miPolySegment;
	switch (pGC->lineStyle)
	{
	case LineSolid:
	    if(pGC->lineWidth == 0)
	    {
		if (pGC->fillStyle == FillSolid)
		{
#if defined(PIXEL_ADDR) && !defined(NO_ONE_RECT)
		    if (devPriv->oneRect &&
			((pDrawable->x >= pGC->pScreen->width - 32768) &&
			 (pDrawable->y >= pGC->pScreen->height - 32768)))
		    {
			pGC->ops->Polylines = cfb8LineSS1Rect;
			pGC->ops->PolySegment = cfb8SegmentSS1Rect;
		    } else
#endif
#ifdef NO_ONE_RECT
		    {
			pGC->ops->Polylines = cfb8LineSS1Rect;
			pGC->ops->PolySegment = cfb8SegmentSS1Rect;
		    }
#else
		    {
		    	pGC->ops->Polylines = cfbLineSS;
		    	pGC->ops->PolySegment = cfbSegmentSS;
		    }
#endif
		}
 		else
		    pGC->ops->Polylines = miZeroLine;
	    }
	    else
		pGC->ops->Polylines = miWideLine;
	    break;
	case LineOnOffDash:
	case LineDoubleDash:
	    if (pGC->lineWidth == 0 && pGC->fillStyle == FillSolid)
	    {
		pGC->ops->Polylines = cfbLineSD;
		pGC->ops->PolySegment = cfbSegmentSD;
	    } else
		pGC->ops->Polylines = miWideDash;
	    break;
	}
    }

    if (new_text && (pGC->font))
    {
        if (FONTMAXBOUNDS(pGC->font,rightSideBearing) -
            FONTMINBOUNDS(pGC->font,leftSideBearing) > 32 ||
	    FONTMINBOUNDS(pGC->font,characterWidth) < 0)
        {
            pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
        }
        else
        {
#ifdef WriteBitGroup
	    if (pGC->fillStyle == FillSolid)
	    {
		if (devPriv->rop == GXcopy)
		    pGC->ops->PolyGlyphBlt = cfbPolyGlyphBlt8;
		else
#ifdef FOUR_BIT_CODE
		    pGC->ops->PolyGlyphBlt = cfbPolyGlyphRop8;
#else
		    pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
#endif
	    }
	    else
#endif
		pGC->ops->PolyGlyphBlt = miPolyGlyphBlt;
            /* special case ImageGlyphBlt for terminal emulator fonts */
#if !defined(WriteBitGroup) || PSZ == 8
	    if (TERMINALFONT(pGC->font) &&
		(pGC->planemask & PMSK) == PMSK
#ifdef FOUR_BIT_CODE
		&& FONTMAXBOUNDS(pGC->font,characterWidth) >= PGSZB
#endif
		)
	    {
		pGC->ops->ImageGlyphBlt = useTEGlyphBlt;
	    }
            else
#endif
	    {
#ifdef WriteBitGroup
		if (devPriv->rop == GXcopy &&
		    pGC->fillStyle == FillSolid &&
		    (pGC->planemask & PMSK) == PMSK)
		    pGC->ops->ImageGlyphBlt = cfbImageGlyphBlt8;
		else
#endif
		    pGC->ops->ImageGlyphBlt = miImageGlyphBlt;
	    }
        }
    }    


    if (new_fillspans) {
	switch (pGC->fillStyle) {
	case FillSolid:
	    switch (devPriv->rop) {
	    case GXcopy:
		pGC->ops->FillSpans = cfbSolidSpansCopy;
		break;
	    case GXxor:
		pGC->ops->FillSpans = cfbSolidSpansXor;
		break;
	    default:
		pGC->ops->FillSpans = cfbSolidSpansGeneral;
		break;
	    }
	    break;
	case FillTiled:
	    pGC->ops->FillSpans = cfbUnnaturalTileFS;
	    break;
	case FillStippled:
	case FillOpaqueStippled:
	    pGC->ops->FillSpans = cfbUnnaturalStippleFS;
	    break;
	default:
	    FatalError("cfbValidateGC: illegal fillStyle\n");
	}
    } /* end of new_fillspans */

    if (new_fillarea) {
#ifndef FOUR_BIT_CODE
	pGC->ops->PolyFillRect = miPolyFillRect;
	if (pGC->fillStyle == FillSolid || pGC->fillStyle == FillTiled)
	{
	    pGC->ops->PolyFillRect = cfbPolyFillRect;
	}
#endif
#ifdef FOUR_BIT_CODE
	pGC->ops->PushPixels = mfbPushPixels;
	if (pGC->fillStyle == FillSolid && devPriv->rop == GXcopy)
	    pGC->ops->PushPixels = cfbPushPixels8;
#endif
	pGC->ops->PolyFillArc = miPolyFillArc;
	if (pGC->fillStyle == FillSolid)
	{
	    switch (devPriv->rop)
	    {
	    case GXcopy:
		pGC->ops->PolyFillArc = cfbPolyFillArcSolidCopy;
		break;
	    default:
		pGC->ops->PolyFillArc = cfbPolyFillArcSolidGeneral;
		break;
	    }
	}
    }
}
