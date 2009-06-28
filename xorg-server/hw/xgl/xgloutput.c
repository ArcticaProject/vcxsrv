/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xgl.h"

#include <mivalidate.h>
#include <dixstruct.h>

typedef struct _xglDepth {
    CARD8 depth;
    CARD8 bpp;
} xglDepthRec, *xglDepthPtr;

static xglDepthRec xglDepths[] = {
    {  1,  1 },
    {  4,  4 },
    {  8,  8 },
    { 15, 16 },
    { 16, 16 },
    { 24, 32 },
    { 32, 32 }
};

#define NUM_XGL_DEPTHS (sizeof (xglDepths) / sizeof (xglDepths[0]))

void
xglSetPixmapFormats (ScreenInfo *pScreenInfo)
{
    int i;

    pScreenInfo->imageByteOrder	    = IMAGE_BYTE_ORDER;
    pScreenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
    pScreenInfo->bitmapScanlinePad  = BITMAP_SCANLINE_PAD;
    pScreenInfo->bitmapBitOrder	    = BITMAP_BIT_ORDER;
    pScreenInfo->numPixmapFormats   = 0;

    for (i = 0; i < NUM_XGL_DEPTHS; i++)
    {
	PixmapFormatRec *format;

	format = &pScreenInfo->formats[pScreenInfo->numPixmapFormats++];

	format->depth	     = xglDepths[i].depth;
	format->bitsPerPixel = xglDepths[i].bpp;
	format->scanlinePad  = BITMAP_SCANLINE_PAD;
    }
}

void
xglSetRootClip (ScreenPtr pScreen,
		Bool	  enable)
{
    WindowPtr	pWin = WindowTable[pScreen->myNum];
    WindowPtr	pChild;
    Bool	wasViewable;
    Bool	anyMarked = FALSE;
    RegionPtr	pOldClip = 0;

#ifdef DO_SAVE_UNDERS
    Bool	dosave = FALSE;
#endif

    WindowPtr   pLayerWin;
    BoxRec	box;

    if (!pWin)
	return;

    wasViewable = (Bool) (pWin->viewable);
    if (wasViewable)
    {
	for (pChild = pWin->firstChild; pChild; pChild = pChild->nextSib)
	{
	    (void) (*pScreen->MarkOverlappedWindows) (pChild,
						      pChild,
						      &pLayerWin);
	}

	(*pScreen->MarkWindow) (pWin);
	anyMarked = TRUE;

	if (pWin->valdata)
	{
	    if (HasBorder (pWin))
	    {
		RegionPtr borderVisible;

		borderVisible = REGION_CREATE (pScreen, NullBox, 1);
		REGION_SUBTRACT (pScreen, borderVisible,
				 &pWin->borderClip, &pWin->winSize);
		pWin->valdata->before.borderVisible = borderVisible;
	    }
	    pWin->valdata->before.resized = TRUE;
	}
    }

    if (enable)
    {
	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pScreen->width;
	box.y2 = pScreen->height;

	pWin->drawable.width  = pScreen->width;
	pWin->drawable.height = pScreen->height;

	REGION_INIT (pScreen, &pWin->winSize, &box, 1);
	REGION_INIT (pScreen, &pWin->borderSize, &box, 1);
	REGION_RESET (pScreen, &pWin->borderClip, &box);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }
    else
    {
	REGION_EMPTY (pScreen, &pWin->borderClip);
	REGION_BREAK (pWin->drawable.pScreen, &pWin->clipList);
    }

    ResizeChildrenWinSize (pWin, 0, 0, 0, 0);

    if (wasViewable)
    {
	if (pWin->firstChild)
	{
	    anyMarked |= (*pScreen->MarkOverlappedWindows) (pWin->firstChild,
							    pWin->firstChild,
							    (WindowPtr *) 0);
	}
	else
	{
	    (*pScreen->MarkWindow) (pWin);
	    anyMarked = TRUE;
	}

#ifdef DO_SAVE_UNDERS
	if (DO_SAVE_UNDERS (pWin))
	    dosave = (*pScreen->ChangeSaveUnder) (pLayerWin, pLayerWin);
#endif

	if (anyMarked)
	    (*pScreen->ValidateTree)(pWin, NullWindow, VTOther);
    }

    if (wasViewable)
    {
	if (anyMarked)
	    (*pScreen->HandleExposures) (pWin);

#ifdef DO_SAVE_UNDERS
	if (dosave)
	    (*pScreen->PostChangeSaveUnder) (pLayerWin, pLayerWin);
#endif

	if (anyMarked && pScreen->PostValidateTree)
	    (*pScreen->PostValidateTree) (pWin, NullWindow, VTOther);
    }

    if (pWin->realized)
	WindowsRestructured ();
}
