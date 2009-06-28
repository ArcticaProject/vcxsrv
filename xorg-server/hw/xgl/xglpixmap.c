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

static glitz_buffer_hint_t xglPixmapUsageHints[] = {
    (glitz_buffer_hint_t) 0,	    /* reserved for system memory */
    GLITZ_BUFFER_HINT_STREAM_DRAW,
    GLITZ_BUFFER_HINT_STREAM_READ,
    GLITZ_BUFFER_HINT_STREAM_COPY,
    GLITZ_BUFFER_HINT_STATIC_DRAW,
    GLITZ_BUFFER_HINT_STATIC_READ,
    GLITZ_BUFFER_HINT_STATIC_COPY,
    GLITZ_BUFFER_HINT_DYNAMIC_DRAW,
    GLITZ_BUFFER_HINT_DYNAMIC_READ,
    GLITZ_BUFFER_HINT_DYNAMIC_COPY
};

#define NUM_XGL_PIXMAP_USAGE_HINTS				     \
    (sizeof (xglPixmapUsageHints) / sizeof (xglPixmapUsageHints[0]))

#define XGL_PIXMAP_USAGE_HINT(hint) (xglPixmapUsageHints[hint])

static void
xglPixmapDamageReport (DamagePtr pDamage,
		       RegionPtr pRegion,
		       void	 *closure)
{
    PixmapPtr pPixmap = (PixmapPtr) closure;
    BoxPtr    pExt;

    XGL_PIXMAP_PRIV (pPixmap);

    pExt = REGION_EXTENTS (pPixmap->drawable.pScreen, pRegion);

    if (BOX_NOTEMPTY (&pPixmapPriv->damageBox))
    {
	if (pExt->x1 < pPixmapPriv->damageBox.x1)
	    pPixmapPriv->damageBox.x1 = pExt->x1;

	if (pExt->y1 < pPixmapPriv->damageBox.y1)
	    pPixmapPriv->damageBox.y1 = pExt->y1;

	if (pExt->x2 > pPixmapPriv->damageBox.x2)
	    pPixmapPriv->damageBox.x2 = pExt->x2;

	if (pExt->y2 > pPixmapPriv->damageBox.y2)
	    pPixmapPriv->damageBox.y2 = pExt->y2;
    }
    else
	pPixmapPriv->damageBox = *pExt;
}


static Bool
xglPixmapCreateDamage (PixmapPtr pPixmap)
{
    XGL_PIXMAP_PRIV (pPixmap);

    pPixmapPriv->pDamage =
	DamageCreate (xglPixmapDamageReport, (DamageDestroyFunc) 0,
		      DamageReportRawRegion, TRUE,
		      pPixmap->drawable.pScreen,
		      (void *) pPixmap);
    if (!pPixmapPriv->pDamage)
	return FALSE;

    DamageRegister (&pPixmap->drawable, pPixmapPriv->pDamage);

    return TRUE;
}

void
xglSetPixmapVisual (PixmapPtr    pPixmap,
		    xglVisualPtr pVisual)
{
    xglVisualPtr pOldVisual;

    XGL_PIXMAP_PRIV (pPixmap);

    pOldVisual = pPixmapPriv->pVisual;
    if (pOldVisual && pVisual)
    {
	glitz_surface_t *surface;

	if (pOldVisual->vid != pVisual->vid)
	{
	    surface = pPixmapPriv->surface;
	    if (surface)
	    {
		glitz_drawable_t *drawable;

		drawable = glitz_surface_get_attached_drawable (surface);
		if (drawable)
		{
		    if (pOldVisual->format.drawable->id !=
			pVisual->format.drawable->id)
		    {
			glitz_surface_detach (pPixmapPriv->surface);
			pPixmapPriv->target = xglPixmapTargetOut;
		    }
		}

		if (pOldVisual->format.surface->id != pVisual->format.surface->id)
		{
		    xglSyncBits (&pPixmap->drawable, NULL);
		    glitz_surface_destroy (pPixmapPriv->surface);
		    pPixmapPriv->surface = 0;
		}
	    }
	}
    }
    else if (pOldVisual)
    {
	if (pPixmapPriv->surface)
	{
	    xglSyncBits (&pPixmap->drawable, NULL);
	    glitz_surface_destroy (pPixmapPriv->surface);
	    pPixmapPriv->surface = 0;
	}
	pPixmapPriv->target = xglPixmapTargetNo;
    }

    pPixmapPriv->pVisual = pVisual;

    if (pPixmapPriv->pVisual && pPixmapPriv->pVisual->format.surface)
    {
	if (!pPixmapPriv->pDamage)
	{
	    if (!xglPixmapCreateDamage (pPixmap))
		FatalError (XGL_SW_FAILURE_STRING);
	}
    }
}

static Bool
xglPixmapSurfaceInit (PixmapPtr	    pPixmap,
		      unsigned long features,
		      int	    width,
		      int	    height)
{
    BoxRec box;

    XGL_PIXMAP_PRIV (pPixmap);

    pPixmapPriv->surface = NULL;
    pPixmapPriv->drawable = NULL;
    pPixmapPriv->acceleratedTile = FALSE;
    pPixmapPriv->pictureMask = ~0;
    pPixmapPriv->target = xglPixmapTargetNo;

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = width;
    box.y2 = height;

    REGION_INIT (pScreen, &pPixmapPriv->bitRegion, &box, 1);

    pPixmapPriv->pVisual = xglFindVisualWithDepth (pPixmap->drawable.pScreen,
						   pPixmap->drawable.depth);
    if (pPixmapPriv->pVisual)
    {
	XGL_SCREEN_PRIV (pPixmap->drawable.pScreen);

	/* general pixmap acceleration */
	if (pPixmapPriv->pVisual->format.drawable &&
	    pScreenPriv->accel.pixmap.enabled &&
	    xglCheckPixmapSize (pPixmap, &pScreenPriv->accel.pixmap.size))
	    pPixmapPriv->target = xglPixmapTargetOut;
    }

    if (pPixmapPriv->pVisual && pPixmapPriv->pVisual->format.surface)
    {
	if (!pPixmapPriv->pDamage)
	{
	    if (!xglPixmapCreateDamage (pPixmap))
		FatalError (XGL_SW_FAILURE_STRING);
	}

	if (width && height)
	{
	    if (width == 1 && height == 1)
	    {
		pPixmapPriv->acceleratedTile = TRUE;
	    }
	    else if (features & GLITZ_FEATURE_TEXTURE_BORDER_CLAMP_MASK)
	    {
		if ((features & GLITZ_FEATURE_TEXTURE_NON_POWER_OF_TWO_MASK) ||
		    (POWER_OF_TWO (width) && POWER_OF_TWO (height)))
		    pPixmapPriv->acceleratedTile = TRUE;
	    }
	}
    }

    return TRUE;
}

PixmapPtr
xglCreatePixmap (ScreenPtr  pScreen,
		 int	    width,
		 int	    height,
		 int	    depth,
		 unsigned   usage_hint)
{
    xglPixmapPtr pPixmapPriv;
    PixmapPtr	 pPixmap;

    XGL_SCREEN_PRIV (pScreen);

    pPixmap = AllocatePixmap (pScreen, 0);
    if (!pPixmap)
	return NullPixmap;

    pPixmap->drawable.type = DRAWABLE_PIXMAP;
    pPixmap->drawable.class = 0;
    pPixmap->drawable.pScreen = pScreen;
    pPixmap->drawable.depth = depth;
    pPixmap->drawable.bitsPerPixel = BitsPerPixel (depth);
    pPixmap->drawable.id = 0;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pPixmap->drawable.x = 0;
    pPixmap->drawable.y = 0;
    pPixmap->drawable.width = width;
    pPixmap->drawable.height = height;

#ifdef COMPOSITE
    pPixmap->screen_x = 0;
    pPixmap->screen_y = 0;
#endif

    pPixmap->devKind = 0;
    pPixmap->refcnt = 1;
    pPixmap->devPrivate.ptr = 0;
    pPixmap->usage_hint = usage_hint;

    pPixmapPriv = XGL_GET_PIXMAP_PRIV (pPixmap);

    pPixmapPriv->pVisual = NULL;
    pPixmapPriv->pDamage = NULL;

    if (!xglPixmapSurfaceInit (pPixmap, pScreenPriv->features, width, height))
	return NullPixmap;

    pPixmapPriv->buffer = NULL;
    pPixmapPriv->bits = (pointer) 0;
    pPixmapPriv->stride = 0;
    pPixmapPriv->pGeometry = NULL;
    pPixmapPriv->allBits = TRUE;

    pPixmapPriv->damageBox = miEmptyBox;

    return pPixmap;
}

void
xglFiniPixmap (PixmapPtr pPixmap)
{
    XGL_PIXMAP_PRIV (pPixmap);

    if (pPixmap->devPrivate.ptr)
    {
	if (pPixmapPriv->buffer)
	    glitz_buffer_unmap (pPixmapPriv->buffer);
    }

    if (pPixmapPriv->pGeometry)
	GEOMETRY_UNINIT (pPixmapPriv->pGeometry);

    if (pPixmapPriv->buffer)
	glitz_buffer_destroy (pPixmapPriv->buffer);

    if (pPixmapPriv->bits)
	xfree (pPixmapPriv->bits);

    REGION_UNINIT (pPixmap->drawable.pScreen, &pPixmapPriv->bitRegion);

    if (pPixmapPriv->drawable)
	glitz_drawable_destroy (pPixmapPriv->drawable);

    if (pPixmapPriv->surface)
	glitz_surface_destroy (pPixmapPriv->surface);
}

Bool
xglDestroyPixmap (PixmapPtr pPixmap)
{
    if (--pPixmap->refcnt)
	return TRUE;

    xglFiniPixmap (pPixmap);

    dixFreePrivates(pPixmap->devPrivates);
    xfree (pPixmap);

    return TRUE;
}

Bool
xglModifyPixmapHeader (PixmapPtr pPixmap,
		       int	 width,
		       int	 height,
		       int	 depth,
		       int	 bitsPerPixel,
		       int	 devKind,
		       pointer	 pPixData)
{
    xglScreenPtr pScreenPriv;
    xglPixmapPtr pPixmapPriv;
    int		 oldWidth, oldHeight;

    if (!pPixmap)
	return FALSE;

    pScreenPriv = XGL_GET_SCREEN_PRIV (pPixmap->drawable.pScreen);
    pPixmapPriv = XGL_GET_PIXMAP_PRIV (pPixmap);

    oldWidth  = pPixmap->drawable.width;
    oldHeight = pPixmap->drawable.height;

    if ((width > 0) && (height > 0) && (depth > 0) && (bitsPerPixel > 0) &&
	(devKind > 0) && pPixData)
    {
	pPixmap->drawable.depth = depth;
	pPixmap->drawable.bitsPerPixel = bitsPerPixel;
	pPixmap->drawable.id = 0;
	pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
	pPixmap->drawable.x = 0;
	pPixmap->drawable.y = 0;
	pPixmap->drawable.width = width;
	pPixmap->drawable.height = height;
	pPixmapPriv->stride = devKind;
	pPixmap->refcnt = 1;
    }
    else
    {
	if (width > 0)
	    pPixmap->drawable.width = width;

	if (height > 0)
	    pPixmap->drawable.height = height;

	if (depth > 0)
	    pPixmap->drawable.depth = depth;

	if (bitsPerPixel > 0)
	    pPixmap->drawable.bitsPerPixel = bitsPerPixel;
	else if ((bitsPerPixel < 0) && (depth > 0))
	    pPixmap->drawable.bitsPerPixel = BitsPerPixel (depth);

	if (devKind > 0)
	    pPixmapPriv->stride = devKind;
	else if ((devKind < 0) && ((width > 0) || (depth > 0)))
	    pPixmapPriv->stride = PixmapBytePad (pPixmap->drawable.width,
						 pPixmap->drawable.depth);
    }

    if (pPixmap->drawable.width  != oldWidth ||
	pPixmap->drawable.height != oldHeight)
    {
	pPixmapPriv->pVisual = NULL;
	pPixmapPriv->target  = xglPixmapTargetNo;

	if (pPixmapPriv->drawable)
	    glitz_drawable_destroy (pPixmapPriv->drawable);

	if (pPixmapPriv->surface)
	    glitz_surface_destroy (pPixmapPriv->surface);

	REGION_UNINIT (pPixmap->drawable.pScreen, &pPixmapPriv->bitRegion);

	if (!xglPixmapSurfaceInit (pPixmap,
				   pScreenPriv->features,
				   pPixmap->drawable.width,
				   pPixmap->drawable.height))
	    return FALSE;
    }

    if (pPixData)
    {
	BoxRec box;

	if (pPixmap->devPrivate.ptr)
	{
	    if (pPixmapPriv->buffer)
		glitz_buffer_unmap (pPixmapPriv->buffer);

	    pPixmap->devPrivate.ptr = 0;
	}

	if (pPixmapPriv->pGeometry)
	{
	    GEOMETRY_UNINIT (pPixmapPriv->pGeometry);
	    pPixmapPriv->pGeometry = NULL;
	}

	if (pPixmapPriv->buffer)
	    glitz_buffer_destroy (pPixmapPriv->buffer);

	if (pPixmapPriv->bits)
	    xfree (pPixmapPriv->bits);

	pPixmapPriv->bits = (pointer) 0;
	pPixmapPriv->buffer = glitz_buffer_create_for_data (pPixData);
	if (!pPixmapPriv->buffer)
	    return FALSE;

	pPixmapPriv->allBits = TRUE;

	box.x1 = 0;
	box.y1 = 0;
	box.x2 = pPixmap->drawable.width;
	box.y2 = pPixmap->drawable.height;

	REGION_UNINIT (pPixmap->drawable.pScreen, &pPixmapPriv->bitRegion);
	REGION_INIT (pPixmap->drawable.pScreen, &pPixmapPriv->bitRegion,
		     &box, 1);

	if (pPixmapPriv->pDamage)
	{
	    RegionPtr pRegion;

	    pRegion = DamageRegion (pPixmapPriv->pDamage);

	    REGION_UNINIT (pPixmap->drawable.pScreen, pRegion);
	    REGION_INIT (pPixmap->drawable.pScreen, pRegion, NullBox, 0);
	    REGION_SUBTRACT (pPixmap->drawable.pScreen, pRegion,
			     &pPixmapPriv->bitRegion, pRegion);

	}
    }

    /*
     * Screen pixmap
     */
    if (!pScreenPriv->pScreenPixmap || pScreenPriv->pScreenPixmap == pPixmap)
    {
	if (!pPixmapPriv->drawable)
	{
	    glitz_drawable_reference (pScreenPriv->drawable);
	    pPixmapPriv->drawable = pScreenPriv->drawable;
	}

	if (!pPixmapPriv->surface)
	{
	    glitz_surface_reference (pScreenPriv->surface);
	    pPixmapPriv->surface = pScreenPriv->surface;
	}

	pPixmapPriv->pVisual = pScreenPriv->rootVisual;
	pPixmapPriv->target  = xglPixmapTargetIn;

	if (!pScreenPriv->pScreenPixmap)
	    pScreenPriv->pScreenPixmap = pPixmap;
    }

    return TRUE;
}

RegionPtr
xglPixmapToRegion (PixmapPtr pPixmap)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    RegionPtr pRegion;

    XGL_SCREEN_PRIV (pScreen);

    if (!xglSyncBits (&pPixmap->drawable, NullBox))
	FatalError (XGL_SW_FAILURE_STRING);

    XGL_SCREEN_UNWRAP (BitmapToRegion);
    pRegion = (*pScreen->BitmapToRegion) (pPixmap);
    XGL_SCREEN_WRAP (BitmapToRegion, xglPixmapToRegion);

    return pRegion;
}

xglGeometryPtr
xglPixmapToGeometry (PixmapPtr pPixmap,
		     int       xOff,
		     int       yOff)
{
    XGL_PIXMAP_PRIV (pPixmap);

    if (pPixmap->devPrivate.ptr)
	xglUnmapPixmapBits (pPixmap);

    if (!pPixmapPriv->pGeometry)
    {
	xglGeometryPtr pGeometry;

	if (!pPixmapPriv->buffer)
	{
	    if (!xglAllocatePixmapBits (pPixmap,
					XGL_PIXMAP_USAGE_HINT_DEFAULT))
		return NULL;
	}

	pGeometry = xalloc (sizeof (xglGeometryRec));
	if (!pGeometry)
	    return NULL;

	GEOMETRY_INIT (pPixmap->drawable.pScreen, pGeometry,
		       GLITZ_GEOMETRY_TYPE_BITMAP,
		       GEOMETRY_USAGE_DYNAMIC, 0);

	GEOMETRY_SET_BUFFER (pGeometry, pPixmapPriv->buffer);

	if (pPixmapPriv->stride < 0)
	{
	    pGeometry->f.bitmap.bytes_per_line = -pPixmapPriv->stride;
	    pGeometry->f.bitmap.scanline_order =
		GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP;
	}
	else
	{
	    pGeometry->f.bitmap.bytes_per_line = pPixmapPriv->stride;
	    pGeometry->f.bitmap.scanline_order =
		GLITZ_PIXEL_SCANLINE_ORDER_TOP_DOWN;
	}

	pGeometry->f.bitmap.pad = ((1 + FB_MASK) >> FB_SHIFT) *
	    sizeof (FbBits);
	pGeometry->width = pPixmap->drawable.width;
	pGeometry->count = pPixmap->drawable.height;

	pPixmapPriv->pGeometry = pGeometry;
    }

    pPixmapPriv->pGeometry->xOff = xOff << 16;
    pPixmapPriv->pGeometry->yOff = yOff << 16;

    return pPixmapPriv->pGeometry;
}

Bool
xglCreatePixmapSurface (PixmapPtr pPixmap)
{
    XGL_PIXMAP_PRIV (pPixmap);

    if (!pPixmapPriv->surface)
    {
	XGL_SCREEN_PRIV (pPixmap->drawable.pScreen);

	if (!pPixmapPriv->pVisual || !pPixmapPriv->pVisual->format.surface)
	    return FALSE;

	pPixmapPriv->surface =
	    glitz_surface_create (pScreenPriv->drawable,
				  pPixmapPriv->pVisual->format.surface,
				  pPixmap->drawable.width,
				  pPixmap->drawable.height,
				  0, NULL);
	if (!pPixmapPriv->surface)
	{
	    pPixmapPriv->pVisual = NULL;
	    pPixmapPriv->target  = xglPixmapTargetNo;

	    return FALSE;
	}
    }

    return TRUE;
}

Bool
xglAllocatePixmapBits (PixmapPtr pPixmap, int hint)
{
    int width, height, bpp, stride;

    XGL_PIXMAP_PRIV (pPixmap);
    XGL_SCREEN_PRIV (pPixmap->drawable.pScreen);

    width  = pPixmap->drawable.width;
    height = pPixmap->drawable.height;
    bpp    = pPixmap->drawable.bitsPerPixel;

    stride = ((width * bpp + FB_MASK) >> FB_SHIFT) * sizeof (FbBits);

    if (stride)
    {
	glitz_buffer_t *buffer;

	if ((pScreenPriv->pboMask & bpp) && hint)
	{
	    buffer = glitz_pixel_buffer_create (pScreenPriv->drawable,
						NULL, height * stride,
						XGL_PIXMAP_USAGE_HINT (hint));
	}
	else
	{
	    pPixmapPriv->bits = xalloc (height * stride);
	    if (!pPixmapPriv->bits)
		return FALSE;

	    buffer = glitz_buffer_create_for_data (pPixmapPriv->bits);
	}

	if (!buffer)
	{
	    if (pPixmapPriv->bits)
		xfree (pPixmapPriv->bits);
	    pPixmapPriv->bits = NULL;
	    return FALSE;
	}
	pPixmapPriv->buffer = buffer;
    }

    if (pScreenPriv->yInverted)
	pPixmapPriv->stride = stride;
    else
	pPixmapPriv->stride = -stride;

    return TRUE;
}

Bool
xglMapPixmapBits (PixmapPtr pPixmap)
{
    if (!pPixmap->devPrivate.ptr)
    {
	CARD8 *bits;

	XGL_PIXMAP_PRIV (pPixmap);

	if (!pPixmapPriv->buffer)
	    if (!xglAllocatePixmapBits (pPixmap,
					XGL_PIXMAP_USAGE_HINT_DEFAULT))
		return FALSE;

	bits = glitz_buffer_map (pPixmapPriv->buffer,
				 GLITZ_BUFFER_ACCESS_READ_WRITE);
	if (!bits)
	    return FALSE;

	pPixmap->devKind = pPixmapPriv->stride;
	if (pPixmapPriv->stride < 0)
	{
	    pPixmap->devPrivate.ptr = bits +
		(pPixmap->drawable.height - 1) * -pPixmapPriv->stride;
	}
	else
	{
	    pPixmap->devPrivate.ptr = bits;
	}
    }

    return TRUE;
}

Bool
xglUnmapPixmapBits (PixmapPtr pPixmap)
{
    XGL_PIXMAP_PRIV (pPixmap);

    pPixmap->devKind = 0;
    pPixmap->devPrivate.ptr = 0;

    if (pPixmapPriv->buffer)
	if (glitz_buffer_unmap (pPixmapPriv->buffer))
	    return FALSE;

    return TRUE;
}

Bool
xglCheckPixmapSize (PixmapPtr		 pPixmap,
		    xglSizeConstraintPtr pSize)
{
    if (pPixmap->drawable.width  < pSize->minWidth ||
	pPixmap->drawable.height < pSize->minHeight)
	return FALSE;

    if (pPixmap->drawable.width  > pSize->aboveWidth ||
	pPixmap->drawable.height > pSize->aboveHeight)
	return TRUE;

    return FALSE;
}

void
xglEnablePixmapAccel (PixmapPtr	      pPixmap,
		      xglAccelInfoPtr pAccel)
{
    XGL_SCREEN_PRIV (pPixmap->drawable.pScreen);
    XGL_PIXMAP_PRIV (pPixmap);

    if (pAccel->enabled && xglCheckPixmapSize (pPixmap, &pAccel->size))
    {
	xglVisualPtr v;

	if (pAccel->pbuffer)
	{
	    for (v = pScreenPriv->pVisual; v; v = v->next)
	    {
		if (v->pPixel->depth != pPixmap->drawable.depth)
		    continue;

		if (v->format.drawable && v->pbuffer)
		    break;
	    }
	}
	else
	{
	    for (v = pScreenPriv->pVisual; v; v = v->next)
	    {
		if (v->pPixel->depth != pPixmap->drawable.depth)
		    continue;

		if (v->format.drawable && !v->pbuffer)
		    break;
	    }
	}

	if (v)
	{
	    xglSetPixmapVisual (pPixmap, v);
	    if (!pPixmapPriv->target)
		pPixmapPriv->target = xglPixmapTargetOut;
	}
    }
}
