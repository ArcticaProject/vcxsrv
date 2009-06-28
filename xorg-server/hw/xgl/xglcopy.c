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

Bool
xglCopy (DrawablePtr pSrc,
	 DrawablePtr pDst,
	 int	     dx,
	 int	     dy,
	 BoxPtr	     pBox,
	 int	     nBox)
{
    glitz_surface_t *src, *dst;
    int		    srcXoff, srcYoff;
    int		    dstXoff, dstYoff;

    XGL_DRAWABLE_PIXMAP (pDst);

    if (!nBox)
	return TRUE;

    if (xglPrepareTarget (pDst))
    {
	if (!xglSyncSurface (pSrc))
	    return FALSE;
    }
    else
    {
	if (!xglPrepareTarget (pSrc))
	    return FALSE;

	if (!xglSyncSurface (pDst))
	    return FALSE;
    }

    XGL_GET_DRAWABLE (pSrc, src, srcXoff, srcYoff);
    XGL_GET_DRAWABLE (pDst, dst, dstXoff, dstYoff);

    glitz_surface_set_clip_region (dst,
				   dstXoff, dstYoff,
				   (glitz_box_t *) pBox, nBox);

    glitz_copy_area (src,
		     dst,
		     srcXoff + dx,
		     srcYoff + dy,
		     pPixmap->drawable.width - dstXoff,
		     pPixmap->drawable.height - dstYoff,
		     dstXoff,
		     dstYoff);

    glitz_surface_set_clip_region (dst, 0, 0, NULL, 0);

    if (glitz_surface_get_status (dst))
	return FALSE;

    return TRUE;
}

void
xglCopyProc (DrawablePtr pSrc,
	     DrawablePtr pDst,
	     GCPtr	 pGC,
	     BoxPtr	 pBox,
	     int	 nBox,
	     int	 dx,
	     int	 dy,
	     Bool	 reverse,
	     Bool	 upsidedown,
	     Pixel	 bitplane,
	     void	 *closure)
{
    BoxPtr pSrcBox = (BoxPtr) closure;

    if (!xglCopy (pSrc, pDst, dx, dy, pBox, nBox))
    {
	RegionRec region;

	XGL_DRAWABLE_PIXMAP (pDst);
	XGL_PIXMAP_PRIV (pPixmap);

	if (!xglSyncBits (pSrc, pSrcBox))
	    FatalError (XGL_SW_FAILURE_STRING);

	if (!xglMapPixmapBits (pPixmap))
	    FatalError (XGL_SW_FAILURE_STRING);

	fbCopyNtoN (pSrc, pDst, pGC,
		    pBox, nBox,
		    dx, dy,
		    reverse, upsidedown, bitplane,
		    (void *) 0);

	pPixmapPriv->damageBox = miEmptyBox;

	while (nBox--)
	{
	    REGION_INIT (pDst->pScreen, &region, pBox, 1);
	    xglAddSurfaceDamage (pDst, &region);
	    REGION_UNINIT (pDst->pScreen, &region);

	    pBox++;
	}
    } else
	xglAddCurrentBitDamage (pDst);
}
