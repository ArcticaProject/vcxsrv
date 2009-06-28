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
#include "fb.h"

#define XGL_WINDOW_FALLBACK_PROLOGUE(pWin, func)		       \
    if (!xglMapPixmapBits (XGL_GET_DRAWABLE_PIXMAP (&pWin->drawable))) \
	FatalError (XGL_SW_FAILURE_STRING);			       \
    XGL_SCREEN_UNWRAP (func)

#define XGL_WINDOW_FALLBACK_EPILOGUE(pWin, pRegion, func, xglfunc) \
    XGL_SCREEN_WRAP (func, xglfunc);				   \
    xglAddSurfaceDamage (&pWin->drawable, pRegion)

Bool
xglCreateWindow (WindowPtr pWin)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    Bool      ret;

    XGL_SCREEN_PRIV (pScreen);
    XGL_WINDOW_PRIV (pWin);

    XGL_SCREEN_UNWRAP (CreateWindow);
    ret = (*pScreen->CreateWindow) (pWin);
    XGL_SCREEN_WRAP (CreateWindow, xglCreateWindow);

    pWinPriv->pPixmap = pWin->drawable.pScreen->devPrivate;

    return ret;
}

Bool
xglDestroyWindow (WindowPtr pWin)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    Bool      ret;

    XGL_SCREEN_PRIV (pScreen);

    XGL_SCREEN_UNWRAP (DestroyWindow);
    ret = (*pScreen->DestroyWindow) (pWin);
    XGL_SCREEN_WRAP (DestroyWindow, xglDestroyWindow);

    return ret;
}

Bool
xglChangeWindowAttributes (WindowPtr	 pWin,
			   unsigned long mask)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;
    PixmapPtr pPixmap;
    Bool      ret;

    XGL_SCREEN_PRIV (pScreen);

    if (mask & CWBackPixmap)
    {
	if (pWin->backgroundState == BackgroundPixmap)
	{
	    pPixmap = pWin->background.pixmap;

	    if (FbEvenTile (pPixmap->drawable.width *
			    pPixmap->drawable.bitsPerPixel))
		xglSyncBits (&pPixmap->drawable, NULL);
	}
    }

    if (mask & CWBorderPixmap)
    {
	if (pWin->borderIsPixel == FALSE)
	{
	    pPixmap = pWin->border.pixmap;

	    if (FbEvenTile (pPixmap->drawable.width *
			    pPixmap->drawable.bitsPerPixel))
		xglSyncBits (&pPixmap->drawable, NULL);
	}
    }

    XGL_SCREEN_UNWRAP (ChangeWindowAttributes);
    ret = (*pScreen->ChangeWindowAttributes) (pWin, mask);
    XGL_SCREEN_WRAP (ChangeWindowAttributes, xglChangeWindowAttributes);

    return ret;
}

void
xglCopyWindow (WindowPtr   pWin,
	       DDXPointRec ptOldOrg,
	       RegionPtr   prgnSrc)
{
    PixmapPtr pPixmap;
    RegionRec rgnDst;
    int	      dx, dy;
    BoxPtr    pExtent = REGION_EXTENTS (pWin->drawable.pScreen, prgnSrc);
    BoxRec    box;

    pPixmap = XGL_GET_WINDOW_PIXMAP (pWin);

    box.x1 = pExtent->x1;
    box.y1 = pExtent->y1;
    box.x2 = pExtent->x2;
    box.y2 = pExtent->y2;

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;

    REGION_TRANSLATE (pWin->drawable.pScreen, prgnSrc, -dx, -dy);
    REGION_INIT (pWin->drawable.pScreen, &rgnDst, NullBox, 0);
    REGION_INTERSECT (pWin->drawable.pScreen,
		      &rgnDst, &pWin->borderClip, prgnSrc);

    fbCopyRegion (&pWin->drawable, &pWin->drawable,
		  0, &rgnDst, dx, dy, xglCopyProc, 0, (void *) &box);

    REGION_UNINIT (pWin->drawable.pScreen, &rgnDst);
}

PixmapPtr
xglGetWindowPixmap (WindowPtr pWin)
{
    return XGL_GET_WINDOW_PIXMAP (pWin);
}

void
xglSetWindowPixmap (WindowPtr pWin,
		    PixmapPtr pPixmap)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    XGL_SCREEN_PRIV (pScreen);

    XGL_SCREEN_UNWRAP (SetWindowPixmap);
    (*pScreen->SetWindowPixmap) (pWin, pPixmap);
    XGL_SCREEN_WRAP (SetWindowPixmap, xglSetWindowPixmap);

    XGL_GET_WINDOW_PRIV (pWin)->pPixmap = pPixmap;

    if (pPixmap != pScreenPriv->pScreenPixmap)
	xglEnablePixmapAccel (pPixmap, &pScreenPriv->accel.window);
}
