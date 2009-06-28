/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xgl.h"
#include "gcstruct.h"

#ifdef MITSHM

void
xglShmPutImage (DrawablePtr  pDrawable,
		GCPtr	     pGC,
		int	     depth,
		unsigned int format,
		int	     w,
		int	     h,
		int	     sx,
		int	     sy,
		int	     sw,
		int	     sh,
		int	     dx,
		int	     dy,
		char	     *data)
{
    ScreenPtr pScreen = pDrawable->pScreen;
    PixmapPtr pPixmapHeader = NULL;
    PixmapPtr pPixmap;
    int	      saveTarget;

    XGL_DRAWABLE_PIXMAP_PRIV (pDrawable);

    if ((format == ZPixmap) || (depth == 1))
    {
	pPixmap = pPixmapHeader =
	    GetScratchPixmapHeader (pScreen, w, h, depth,
				    BitsPerPixel (depth),
				    PixmapBytePad (w, depth),
				    (pointer) data);

	/* disable any possible acceleration of this pixmap */
	if (pPixmap)
	    xglSetPixmapVisual (pPixmap, 0);
    }
    else
    {
	pPixmap = (*pScreen->CreatePixmap) (pScreen, sw, sh, depth,
					    CREATE_PIXMAP_USAGE_SCRATCH);
	if (pPixmap)
	{
	    GCPtr pScratchGC;

	    if (!xglAllocatePixmapBits (pPixmap,
					XGL_PIXMAP_USAGE_HINT_DEFAULT))
	    {
		(*pScreen->DestroyPixmap) (pPixmap);
		return;
	    }

	    xglSetPixmapVisual (pPixmap, 0);

	    pScratchGC = GetScratchGC (depth, pScreen);
	    if (!pScratchGC)
	    {
		(*pScreen->DestroyPixmap) (pPixmap);
		return;
	    }

	    ValidateGC ((DrawablePtr) pPixmap, pScratchGC);
	    (*pGC->ops->PutImage) ((DrawablePtr) pPixmap, pScratchGC, depth,
				   -sx, -sy, w, h, 0,
				   (format == XYPixmap) ? XYPixmap : ZPixmap,
				   data);

	    FreeScratchGC (pScratchGC);

	    sx = sy = 0;
	}
    }

    if (!pPixmap)
	return;

    /* CopyArea should always be done in software */
    saveTarget = pPixmapPriv->target;
    pPixmapPriv->target = xglPixmapTargetNo;

    if (format == XYBitmap)
	(*pGC->ops->CopyPlane) ((DrawablePtr) pPixmap, pDrawable, pGC,
				sx, sy, sw, sh, dx, dy, 1L);
    else
	(*pGC->ops->CopyArea) ((DrawablePtr) pPixmap, pDrawable, pGC,
			       sx, sy, sw, sh, dx, dy);

    pPixmapPriv->target = saveTarget;

    if (pPixmapHeader)
	FreeScratchPixmapHeader (pPixmapHeader);
    else
	(*pScreen->DestroyPixmap) (pPixmap);
}

#endif
