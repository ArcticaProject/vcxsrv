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

void
xglGetImage (DrawablePtr   pDrawable,
	     int	   x,
	     int	   y,
	     int	   w,
	     int	   h,
	     unsigned int  format,
	     unsigned long planeMask,
	     char	   *d)
{
    ScreenPtr	    pScreen = pDrawable->pScreen;
    glitz_surface_t *surface;
    int             xOff, yOff;
    BoxRec	    box;

    XGL_SCREEN_PRIV (pScreen);

    /* Many apps use GetImage to sync with the visible frame buffer */
    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	if (!xglSyncSurface (&pScreenPriv->pScreenPixmap->drawable))
	    FatalError (XGL_SW_FAILURE_STRING);

	glitz_surface_flush (pScreenPriv->surface);
	glitz_drawable_finish (pScreenPriv->drawable);
    }

    XGL_GET_DRAWABLE (pDrawable, surface, xOff, yOff);

    box.x1 = pDrawable->x + xOff + x;
    box.y1 = pDrawable->y + yOff + y;
    box.x2 = box.x1 + w;
    box.y2 = box.y1 + h;

    if (!xglSyncBits (pDrawable, &box))
	FatalError (XGL_SW_FAILURE_STRING);

    XGL_SCREEN_UNWRAP (GetImage);
    (*pScreen->GetImage) (pDrawable, x, y, w, h, format, planeMask, d);
    XGL_SCREEN_WRAP (GetImage, xglGetImage);
}

void
xglGetSpans (DrawablePtr pDrawable,
	     int	 wMax,
	     DDXPointPtr ppt,
	     int	 *pwidth,
	     int	 nspans,
	     char	 *pchardstStart)
{
    ScreenPtr pScreen = pDrawable->pScreen;

    XGL_SCREEN_PRIV (pScreen);

    if (!xglSyncBits (pDrawable, NullBox))
	FatalError (XGL_SW_FAILURE_STRING);

    XGL_SCREEN_UNWRAP (GetSpans);
    (*pScreen->GetSpans) (pDrawable, wMax, ppt, pwidth, nspans, pchardstStart);
    XGL_SCREEN_WRAP (GetSpans, xglGetSpans);
}
