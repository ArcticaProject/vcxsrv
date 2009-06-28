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

#ifdef RENDER
#include "gcstruct.h"
#include "picturestr.h"

#define BITMAP_CACHE_SIZE      256000
#define BITMAP_CACHE_MAX_LEVEL ~0
#define BITMAP_CACHE_MAX_SIZE  512

#define TEXTURE_CACHE_SIZE	 512
#define TEXTURE_CACHE_MAX_LEVEL	 64
#define TEXTURE_CACHE_MAX_HEIGHT 72
#define TEXTURE_CACHE_MAX_WIDTH  72

#define NEXT_GLYPH_SERIAL_NUMBER ((++glyphSerialNumber) > MAX_SERIAL_NUM ? \
	    (glyphSerialNumber = 1): glyphSerialNumber)

#define GLYPH_GET_AREA_PRIV(pArea)		\
    ((xglGlyphAreaPtr) (pArea)->devPrivate.ptr)

#define GLYPH_AREA_PRIV(pArea)				    \
    xglGlyphAreaPtr pAreaPriv = GLYPH_GET_AREA_PRIV (pArea)

#define NEEDS_COMPONENT(f) (PICT_FORMAT_A (f) != 0 && PICT_FORMAT_RGB (f) != 0)

#define WRITE_VEC2(ptr, _x, _y) \
    *(ptr)++ = (_x);		\
    *(ptr)++ = (_y)

#define WRITE_BOX(ptr, _vx1, _vy1, _vx2, _vy2, box) \
    WRITE_VEC2 (ptr, _vx1, _vy1);		    \
    WRITE_VEC2 (ptr, (box).x1, (box).y2);	    \
    WRITE_VEC2 (ptr, _vx2, _vy1);		    \
    WRITE_VEC2 (ptr, (box).x2, (box).y2);	    \
    WRITE_VEC2 (ptr, _vx2, _vy2);		    \
    WRITE_VEC2 (ptr, (box).x2, (box).y1);	    \
    WRITE_VEC2 (ptr, _vx1, _vy2);		    \
    WRITE_VEC2 (ptr, (box).x1, (box).y1)

typedef union _xglGlyphList {
    glitz_short_t *s;
    glitz_float_t *f;
} xglGlyphListRec, *xglGlyphListPtr;

typedef struct _xglGlyphArray {
    int	lastX, lastY;
} xglGlyphArrayRec, *xglGlyphArrayPtr;

typedef union _xglGlyphVertexData {
    xglGlyphArrayRec array;
    xglGlyphListRec  list;
} xglGlyphVertexDataRec, *xglGlyphVertexDataPtr;

typedef struct _xglGlyphOp {
    GlyphListPtr pLists;
    int		 listLen;
    GlyphPtr	 *ppGlyphs;
    int		 nGlyphs;
    int		 xOff;
    int		 yOff;
    Bool	 noCache;
} xglGlyphOpRec, *xglGlyphOpPtr;

unsigned long glyphSerialNumber = 0;

xglAreaRec zeroSizeArea = {
    0, 0,
    0, 0,
    0, 0,
    { NULL, NULL, NULL, NULL }, NULL,
    (pointer) 0,
    { 0 }
};

static Bool
xglGlyphCreate (xglAreaPtr pArea)
{
    return TRUE;
}

static Bool
xglGlyphMoveIn (xglAreaPtr pArea,
		pointer    closure)
{
    xglGlyphCachePtr pCache = (xglGlyphCachePtr) pArea->pRoot->closure;
    GlyphPtr	     pGlyph = (GlyphPtr) closure;

    XGL_GLYPH_PRIV (pCache->pScreen, pGlyph);

    pGlyphPriv->pArea = pArea;

    return TRUE;
}

static void
xglGlyphMoveOut (xglAreaPtr pArea,
		 pointer    closure)
{
    xglGlyphCachePtr pCache = (xglGlyphCachePtr) pArea->pRoot->closure;
    GlyphPtr	     pGlyph = (GlyphPtr) closure;

    XGL_GLYPH_PRIV (pCache->pScreen, pGlyph);

    pGlyphPriv->pArea = NULL;
}

static int
xglGlyphCompareScore (xglAreaPtr pArea,
		      pointer	 closure1,
		      pointer	 closure2)
{
    GLYPH_AREA_PRIV (pArea);

    if (pAreaPriv->serial == glyphSerialNumber)
	return 1;

    return -1;
}

static const xglAreaFuncsRec xglGlyphAreaFuncs = {
    xglGlyphCreate,
    xglGlyphMoveIn,
    xglGlyphMoveOut,
    xglGlyphCompareScore
};

Bool
xglRealizeGlyph (ScreenPtr pScreen,
		 GlyphPtr  pGlyph)
{
    PictureScreenPtr pPictureScreen = GetPictureScreen (pScreen);
    Bool	     ret;

    XGL_SCREEN_PRIV (pScreen);
    XGL_GLYPH_PRIV (pScreen, pGlyph);

    XGL_PICTURE_SCREEN_UNWRAP (RealizeGlyph);
    ret = (*pPictureScreen->RealizeGlyph) (pScreen, pGlyph);
    XGL_PICTURE_SCREEN_WRAP (RealizeGlyph, xglRealizeGlyph);

    pGlyphPriv->pArea = NULL;

    return ret;
}

void
xglUnrealizeGlyph (ScreenPtr pScreen,
		   GlyphPtr  pGlyph)
{
    PictureScreenPtr pPictureScreen = GetPictureScreen (pScreen);

    XGL_SCREEN_PRIV (pScreen);
    XGL_GLYPH_PRIV (pScreen, pGlyph);

    XGL_PICTURE_SCREEN_UNWRAP (UnrealizeGlyph);
    (*pPictureScreen->UnrealizeGlyph) (pScreen, pGlyph);
    XGL_PICTURE_SCREEN_WRAP (UnrealizeGlyph, xglUnrealizeGlyph);

    if (pGlyphPriv->pArea && pGlyphPriv->pArea->width)
	xglWithdrawArea (pGlyphPriv->pArea);
}

Bool
xglInitGlyphCache (xglGlyphCachePtr pCache,
		   ScreenPtr	    pScreen,
		   PictFormatPtr    format)
{
    XGL_SCREEN_PRIV (pScreen);

    pCache->depth = format->depth;

    if (!pScreenPriv->pSolidAlpha)
    {
	xglCreateSolidAlphaPicture (pScreen);
	if (!pScreenPriv->pSolidAlpha)
	    return FALSE;
    }

    if (pCache->depth == 1)
    {
	int stride;

	GEOMETRY_INIT (pScreen, &pCache->u.geometry,
		       GLITZ_GEOMETRY_TYPE_VERTEX,
		       GEOMETRY_USAGE_STATIC, BITMAP_CACHE_SIZE);
	GEOMETRY_SET_VERTEX_DATA_TYPE (&pCache->u.geometry,
				       pScreenPriv->geometryDataType);

	stride = pCache->u.geometry.f.vertex.bytes_per_vertex;
	if (!xglRootAreaInit (&pCache->rootArea,
			      BITMAP_CACHE_MAX_LEVEL,
			      BITMAP_CACHE_SIZE / (stride * 4),
			      0, sizeof (xglGlyphAreaRec),
			      (xglAreaFuncsPtr) &xglGlyphAreaFuncs,
			      (pointer) pCache))
	{
	    GEOMETRY_UNINIT (&pCache->u.geometry);
	    return FALSE;
	}
    }
    else
    {

	xglGlyphTexturePtr	   pTexture = &pCache->u.texture;
	glitz_surface_t		   *mask;
	glitz_surface_attributes_t attr;
	glitz_vertex_format_t	   *vertex;
	xglVisualPtr		   pVisual;

	pVisual = xglFindVisualWithDepth (pScreen, format->depth);
	if (!pVisual)
	    return FALSE;

	if (!xglRootAreaInit (&pCache->rootArea,
			      TEXTURE_CACHE_MAX_LEVEL,
			      TEXTURE_CACHE_SIZE, TEXTURE_CACHE_SIZE,
			      sizeof (xglGlyphAreaRec),
			      (xglAreaFuncsPtr) &xglGlyphAreaFuncs,
			      (pointer) pCache))
	    return FALSE;

	if (pScreenPriv->geometryDataType == GEOMETRY_DATA_TYPE_SHORT)
	{
	    attr.unnormalized = 1;
	    mask = glitz_surface_create (pScreenPriv->drawable,
					 pVisual->format.surface,
					 TEXTURE_CACHE_SIZE,
					 TEXTURE_CACHE_SIZE,
					 GLITZ_SURFACE_UNNORMALIZED_MASK,
					 &attr);
	}
	else
	    mask = NULL;

	if (!mask)
	{
	    mask = glitz_surface_create (pScreenPriv->drawable,
					 pVisual->format.surface,
					 TEXTURE_CACHE_SIZE,
					 TEXTURE_CACHE_SIZE,
					 0, NULL);
	    if (!mask)
		return FALSE;

	    pTexture->geometryDataType = GEOMETRY_DATA_TYPE_FLOAT;
	}
	else
	    pTexture->geometryDataType = GEOMETRY_DATA_TYPE_SHORT;

	if (NEEDS_COMPONENT (format->format))
	    glitz_surface_set_component_alpha (mask, 1);

	pTexture->pMask = xglCreateDevicePicture (mask);
	if (!pTexture->pMask)
	    return FALSE;

	vertex = &pCache->u.texture.format.vertex;
	vertex->primitive  = GLITZ_PRIMITIVE_QUADS;
	vertex->mask.size  = GLITZ_COORDINATE_SIZE_XY;
	vertex->attributes = GLITZ_VERTEX_ATTRIBUTE_MASK_COORD_MASK;

	if (pTexture->geometryDataType == GEOMETRY_DATA_TYPE_FLOAT)
	{
	    vertex->type	     = GLITZ_DATA_TYPE_FLOAT;
	    vertex->bytes_per_vertex = sizeof (glitz_float_t) * 4;
	    vertex->mask.offset	     = sizeof (glitz_float_t) * 2;
	    vertex->mask.type	     = GLITZ_DATA_TYPE_FLOAT;
	}
	else
	{
	    vertex->type	     = GLITZ_DATA_TYPE_SHORT;
	    vertex->bytes_per_vertex = sizeof (glitz_short_t) * 4;
	    vertex->mask.offset	     = sizeof (glitz_short_t) * 2;
	    vertex->mask.type	     = GLITZ_DATA_TYPE_SHORT;
	}

	pTexture->pixel.fourcc	       = GLITZ_FOURCC_RGB;
	pTexture->pixel.masks	       = pVisual->pPixel->masks;
	pTexture->pixel.scanline_order = GLITZ_PIXEL_SCANLINE_ORDER_BOTTOM_UP;
	pTexture->pixel.bytes_per_line = 0;
	pTexture->pixel.xoffset	       = 0;
	pTexture->pixel.skip_lines     = 0;
    }

    pCache->pScreen = pScreen;

    return TRUE;
}

void
xglFiniGlyphCache (xglGlyphCachePtr pCache)
{
    if (pCache->pScreen)
    {
	xglRootAreaFini (&pCache->rootArea);

	if (pCache->depth == 1)
	{
	    GEOMETRY_UNINIT (&pCache->u.geometry);
	}
	else
	{
	    if (pCache->u.texture.pMask)
		FreePicture ((pointer) pCache->u.texture.pMask, 0);
	}

	pCache->pScreen = NULL;
    }
}

static xglAreaPtr
xglCacheGlyph (xglGlyphCachePtr pCache,
	       GlyphPtr		pGlyph)
{
    ScreenPtr pScreen = pCache->pScreen;

    XGL_GLYPH_PRIV (pScreen, pGlyph);

    if (pCache->depth == 1)
    {
	PixmapPtr pPixmap;
	RegionPtr pRegion;
	int	  nBox;

	pPixmap = GetScratchPixmapHeader (pScreen,
					  pGlyph->info.width,
					  pGlyph->info.height,
					  pCache->depth, pCache->depth, 0,
					  (pointer) (pGlyph + 1));
	if (!pPixmap)
	    return NULL;

	(*pScreen->ModifyPixmapHeader) (pPixmap,
					pGlyph->info.width,
					pGlyph->info.height,
					0, 0, -1, (pointer) (pGlyph + 1));

	pRegion = (*pScreen->BitmapToRegion) (pPixmap);
	FreeScratchPixmapHeader (pPixmap);

	if (!pRegion)
	    return NULL;

	nBox = REGION_NUM_RECTS (pRegion);
	if (nBox > BITMAP_CACHE_MAX_SIZE)
	{
	    REGION_DESTROY (pScreen, pRegion);
	    return NULL;
	}

	if (nBox > 0)
	{
	    /* Find available area */
	    if (!xglFindArea (pCache->rootArea.pArea, nBox, 0,
			      FALSE, (pointer) pGlyph))
	    {
		/* Kicking out area with lower score */
		xglFindArea (pCache->rootArea.pArea, nBox, 0,
			     TRUE, (pointer) pGlyph);
	    }

	    if (pGlyphPriv->pArea)
	    {
		int stride;

		GLYPH_AREA_PRIV (pGlyphPriv->pArea);

		pAreaPriv->serial = glyphSerialNumber;
		pAreaPriv->u.range.first = pGlyphPriv->pArea->x * 4;
		pAreaPriv->u.range.count = nBox * 4;

		stride = pCache->u.geometry.f.vertex.bytes_per_vertex;
		GEOMETRY_ADD_REGION_AT (pScreen, &pCache->u.geometry, pRegion,
					pGlyphPriv->pArea->x * stride * 4);
	    }
	} else
	    pGlyphPriv->pArea = &zeroSizeArea;

	REGION_DESTROY (pScreen, pRegion);
    }
    else
    {
	xglGlyphTexturePtr pTexture = &pCache->u.texture;

	if (pGlyph->info.width  > TEXTURE_CACHE_MAX_WIDTH ||
	    pGlyph->info.height > TEXTURE_CACHE_MAX_HEIGHT)
	    return NULL;

	if (pGlyph->info.width > 0 && pGlyph->info.height > 0)
	{
	    glitz_buffer_t *buffer;

	    buffer = glitz_buffer_create_for_data (pGlyph + 1);
	    if (!buffer)
		return NULL;

	    /* Find available area */
	    if (!xglFindArea (pCache->rootArea.pArea,
			      pGlyph->info.width, pGlyph->info.height,
			      FALSE, (pointer) pGlyph))
	    {
		/* Kicking out area with lower score */
		xglFindArea (pCache->rootArea.pArea,
			     pGlyph->info.width, pGlyph->info.height,
			     TRUE, (pointer) pGlyph);
	    }

	    if (pGlyphPriv->pArea)
	    {
		glitz_surface_t	     *surface;
		glitz_point_fixed_t  p1, p2;
		glitz_pixel_format_t pixel;

		GLYPH_AREA_PRIV (pGlyphPriv->pArea);

		pixel = pTexture->pixel;
		pixel.bytes_per_line =
		    PixmapBytePad (pGlyph->info.width, pCache->depth);

		surface = pTexture->pMask->pSourcePict->source.devPrivate.ptr;

		glitz_set_pixels (surface,
				  pGlyphPriv->pArea->x,
				  pGlyphPriv->pArea->y,
				  pGlyph->info.width,
				  pGlyph->info.height,
				  &pixel,
				  buffer);

		p1.x = pGlyphPriv->pArea->x << 16;
		p1.y = pGlyphPriv->pArea->y << 16;
		p2.x = (pGlyphPriv->pArea->x + pGlyph->info.width)  << 16;
		p2.y = (pGlyphPriv->pArea->y + pGlyph->info.height) << 16;

		glitz_surface_translate_point (surface, &p1, &p1);
		glitz_surface_translate_point (surface, &p2, &p2);

		pAreaPriv->serial = glyphSerialNumber;
		if (pTexture->geometryDataType)
		{
		    pAreaPriv->u.box.fBox.x1 = FIXED_TO_FLOAT (p1.x);
		    pAreaPriv->u.box.fBox.y1 = FIXED_TO_FLOAT (p1.y);
		    pAreaPriv->u.box.fBox.x2 = FIXED_TO_FLOAT (p2.x);
		    pAreaPriv->u.box.fBox.y2 = FIXED_TO_FLOAT (p2.y);
		}
		else
		{
		    pAreaPriv->u.box.sBox.x1 = p1.x >> 16;
		    pAreaPriv->u.box.sBox.y1 = p1.y >> 16;
		    pAreaPriv->u.box.sBox.x2 = p2.x >> 16;
		    pAreaPriv->u.box.sBox.y2 = p2.y >> 16;
		}
	    }
	    glitz_buffer_destroy (buffer);
	} else
	    pGlyphPriv->pArea = &zeroSizeArea;
    }

    return pGlyphPriv->pArea;
}

static void
xglUncachedGlyphs (CARD8	 op,
		   PicturePtr    pSrc,
		   PicturePtr    pDst,
		   INT16	 xSrc,
		   INT16	 ySrc,
		   xglGlyphOpPtr pOp)
{
    ScreenPtr	     pScreen = pDst->pDrawable->pScreen;
    PicturePtr	     pPicture = NULL;
    PixmapPtr	     pPixmap = NULL;
    xglGlyphCachePtr pCache;
    int		     depth = pOp->pLists->format->depth;
    GlyphPtr	     glyph;
    INT16	     xOff, yOff;
    xglGlyphPtr	     pGlyphPriv;
    xglAreaPtr	     pArea;
    Bool	     usingCache = !pOp->noCache;

    XGL_SCREEN_PRIV (pScreen);

    pCache = &pScreenPriv->glyphCache[depth];
    if (usingCache)
    {
	if (!pCache->pScreen)
	{
	    if (!xglInitGlyphCache (pCache, pScreen, pOp->pLists->format))
		usingCache = FALSE;
	}
    }

    while (pOp->nGlyphs)
    {
	glyph = *pOp->ppGlyphs;

	if (!pOp->listLen)
	{
	    pOp->pLists++;
	    pOp->listLen = pOp->pLists->len;
	    pOp->xOff   += pOp->pLists->xOff;
	    pOp->yOff   += pOp->pLists->yOff;
	}

	xOff = pOp->xOff;
	yOff = pOp->yOff;

	if (usingCache)
	{
	    pGlyphPriv = XGL_GET_GLYPH_PRIV (pScreen, glyph);
	    pArea = pGlyphPriv->pArea;
	    if (pSrc)
	    {
		if (!pArea)
		    pArea = xglCacheGlyph (pCache, glyph);

		if (pArea)
		    break;
	    }
	} else
	    pArea = NULL;

	pOp->listLen--;
	pOp->nGlyphs--;
	pOp->ppGlyphs++;

	pOp->xOff += glyph->info.xOff;
	pOp->yOff += glyph->info.yOff;

	if (pArea)
	    continue;

	if (!pPicture)
	{
	    XID componentAlpha;
	    int	error;

	    pPixmap = GetScratchPixmapHeader (pScreen,
					      glyph->info.width,
					      glyph->info.height,
					      depth, depth,
					      0, (pointer) (glyph + 1));
	    if (!pPixmap)
		return;

	    componentAlpha = NEEDS_COMPONENT (pOp->pLists->format->format);
	    pPicture = CreatePicture (0, &pPixmap->drawable,
				      pOp->pLists->format,
				      CPComponentAlpha, &componentAlpha,
				      serverClient, &error);
	    if (!pPicture)
	    {
		FreeScratchPixmapHeader (pPixmap);
		return;
	    }
	}

	(*pScreen->ModifyPixmapHeader) (pPixmap,
					glyph->info.width, glyph->info.height,
					0, 0, -1, (pointer) (glyph + 1));
	pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;

	if (pSrc)
	    CompositePicture (op,
			      pSrc,
			      pPicture,
			      pDst,
			      xSrc + (xOff - glyph->info.x),
			      ySrc + (yOff - glyph->info.y),
			      0, 0,
			      xOff - glyph->info.x,
			      yOff - glyph->info.y,
			      glyph->info.width,
			      glyph->info.height);
	else
	    CompositePicture (PictOpAdd,
			      pPicture,
			      NULL,
			      pDst,
			      0, 0,
			      0, 0,
			      xOff - glyph->info.x,
			      yOff - glyph->info.y,
			      glyph->info.width,
			      glyph->info.height);
    }

    if (pPicture)
    {
	FreeScratchPixmapHeader (pPixmap);
	FreePicture ((pointer) pPicture, 0);
    }
}

static Bool
xglCachedGlyphs (CARD8	       op,
		 PicturePtr    pSrc,
		 PicturePtr    pDst,
		 INT16	       xSrc,
		 INT16	       ySrc,
		 xglGlyphOpPtr pOp)
{
    ScreenPtr		  pScreen = pDst->pDrawable->pScreen;
    xglGlyphOpRec	  opSave = *pOp;
    xglGlyphCachePtr	  pCache;
    xglGlyphVertexDataRec vData;
    xglGeometryPtr	  pGeometry;
    GlyphPtr		  glyph;
    xglGlyphPtr		  pGlyphPriv;
    xglAreaPtr		  pArea;
    xglGlyphAreaPtr	  pGlyphArea;
    BoxRec		  extents;
    INT16		  xOff, yOff, x1, x2, y1, y2;
    int			  depth = pOp->pLists->format->depth;
    int			  i, remaining = pOp->nGlyphs;
    int			  nGlyph = 0;
    PicturePtr		  pMaskPicture = NULL;

    XGL_SCREEN_PRIV (pScreen);

    pCache = &pScreenPriv->glyphCache[depth];
    if (!pCache->pScreen)
    {
	if (!xglInitGlyphCache (pCache, pScreen, pOp->pLists->format))
	{
	    pOp->noCache = TRUE;
	    return 1;
	}
    }

    /* update serial number for all glyphs already in cache so that
       we don't accidentally replace one. */
    for (i = 0; i < pOp->nGlyphs; i++)
    {
	pGlyphPriv = XGL_GET_GLYPH_PRIV (pScreen, pOp->ppGlyphs[i]);
	pArea = pGlyphPriv->pArea;
	if (pArea && pArea->width)
	    GLYPH_GET_AREA_PRIV (pArea)->serial = glyphSerialNumber;
    }

    for (i = 0; i < pOp->nGlyphs; i++)
    {
	pGlyphPriv = XGL_GET_GLYPH_PRIV (pScreen, pOp->ppGlyphs[i]);
	pArea = pGlyphPriv->pArea;
	if (!pArea)
	    pArea = xglCacheGlyph (pCache, pOp->ppGlyphs[i]);

	if (pArea)
	{
	    if (pArea->width)
		nGlyph++;
	}
	else if (pSrc)
	    break;
    }

    if (nGlyph)
    {
	if (depth == 1)
	{
	    glitz_multi_array_t *multiArray;

	    pGeometry = &pCache->u.geometry;
	    pGeometry->xOff = pGeometry->yOff = 0;

	    multiArray = glitz_multi_array_create (nGlyph);
	    if (!multiArray)
		return 1;

	    GEOMETRY_SET_MULTI_ARRAY (pGeometry, multiArray);
	    glitz_multi_array_destroy (multiArray);

	    vData.array.lastX = 0;
	    vData.array.lastY = 0;
	}
	else
	{
	    i = 4 * pCache->u.texture.format.vertex.bytes_per_vertex * nGlyph;
	    pGeometry = xglGetScratchGeometryWithSize (pScreen, i);

	    pGeometry->f = pCache->u.texture.format;
	    pGeometry->type = GLITZ_GEOMETRY_TYPE_VERTEX;
	    pMaskPicture = pCache->u.texture.pMask;

	    vData.list.s = glitz_buffer_map (pGeometry->buffer,
					     GLITZ_BUFFER_ACCESS_WRITE_ONLY);
	}
    } else
	pGeometry = NULL;

    extents.x1 = MAXSHORT;
    extents.y1 = MAXSHORT;
    extents.x2 = MINSHORT;
    extents.y2 = MINSHORT;

    while (pOp->nGlyphs)
    {
	glyph = *pOp->ppGlyphs;

	if (!pOp->listLen)
	{
	    pOp->pLists++;
	    pOp->listLen = pOp->pLists->len;
	    pOp->xOff   += pOp->pLists->xOff;
	    pOp->yOff   += pOp->pLists->yOff;
	}

	xOff = pOp->xOff;
	yOff = pOp->yOff;

	pGlyphPriv = XGL_GET_GLYPH_PRIV (pScreen, glyph);
	pArea = pGlyphPriv->pArea;
	if (!pArea && pSrc)
	    break;

	pOp->listLen--;
	pOp->nGlyphs--;
	pOp->ppGlyphs++;

	pOp->xOff += glyph->info.xOff;
	pOp->yOff += glyph->info.yOff;

	if (!pArea)
	    continue;

	x1 = xOff - glyph->info.x;
	x2 = x1 + glyph->info.width;
	if (x1 < extents.x1)
	    extents.x1 = x1;
	if (x2 > extents.x2)
	    extents.x2 = x2;

	y1 = yOff - glyph->info.y;
	y2 = y1 + glyph->info.height;
	if (y1 < extents.y1)
	    extents.y1 = y1;
	if (y2 > extents.y2)
	    extents.y2 = y2;

	if (pArea->width)
	{
	    pGlyphArea = GLYPH_GET_AREA_PRIV (pArea);
	    if (depth == 1)
	    {
		glitz_multi_array_add (pGeometry->array,
				       pGlyphArea->u.range.first, 2,
				       pGlyphArea->u.range.count,
				       (x1 - vData.array.lastX) << 16,
				       (y1 - vData.array.lastY) << 16);
		vData.array.lastX = x1;
		vData.array.lastY = y1;
	    }
	    else
	    {
		if (pCache->u.texture.geometryDataType)
		{
		    WRITE_BOX (vData.list.f, x1, y1, x2, y2,
			       pGlyphArea->u.box.fBox);
		}
		else
		{
		    WRITE_BOX (vData.list.s, x1, y1, x2, y2,
			       pGlyphArea->u.box.sBox);
		}
	    }
	}
	remaining--;
    }

    NEXT_GLYPH_SERIAL_NUMBER;

    if (nGlyph)
    {
	if (depth != 1)
	{
	    glitz_buffer_unmap (pGeometry->buffer);
	    pGeometry->count = nGlyph * 4;
	}

	xSrc += extents.x1;
	ySrc += extents.y1;

	if (!pSrc)
	{
	    op = PictOpAdd;
	    pSrc = pScreenPriv->pSolidAlpha;

	    if (remaining)
		*pOp = opSave;
	}

	GEOMETRY_TRANSLATE (pGeometry,
			    pDst->pDrawable->x,
			    pDst->pDrawable->y);

	if (xglCompositeGeneral (op,
				 pSrc,
				 pMaskPicture,
				 pDst,
				 pGeometry,
				 xSrc, ySrc,
				 0, 0,
				 pDst->pDrawable->x + extents.x1,
				 pDst->pDrawable->y + extents.y1,
				 extents.x2 - extents.x1,
				 extents.y2 - extents.y1))
	{
	    xglAddCurrentBitDamage (pDst->pDrawable);
	    return remaining;
	}

	remaining = ~0;
	*pOp = opSave;
	pOp->noCache = TRUE;
    }
    else
    {
	if (remaining)
	{
	    *pOp = opSave;
	    pOp->noCache = TRUE;
	}
    }

    return remaining;
}

static Bool
xglGlyphExtents (PicturePtr   pDst,
		 int	      nlist,
		 GlyphListPtr list,
		 GlyphPtr     *glyphs,
		 BoxPtr	      extents)
{
    GlyphPtr glyph;
    BoxRec   line;
    int	     x1, x2, y1, y2;
    int	     n;
    int	     x;
    int	     y;
    Bool     overlap = FALSE;

    x = 0;
    y = 0;

    extents->x1 = MAXSHORT;
    extents->x2 = MINSHORT;
    extents->y1 = MAXSHORT;
    extents->y2 = MINSHORT;

    while (!list->len)
    {
	if (--nlist)
	{
	    x += list->xOff;
	    y += list->yOff;
	    list++;
	}
	else
	{
	    return FALSE;
	}
    }

    glyph = *glyphs;
    x1 = (x + list->xOff) - glyph->info.x;
    if (x1 < MINSHORT)
	x1 = MINSHORT;
    y1 = (y  + list->yOff) - glyph->info.y;
    if (y1 < MINSHORT)
	y1 = MINSHORT;

    line.x1 = x1;
    line.x2 = x1;
    line.y1 = y1;
    line.y2 = y1;

    while (nlist--)
    {
	x += list->xOff;
	y += list->yOff;
	n = list->len;
	list++;

	while (n--)
	{
	    glyph = *glyphs++;
	    x1 = x - glyph->info.x;
	    if (x1 < MINSHORT)
		x1 = MINSHORT;
	    y1 = y - glyph->info.y;
	    if (y1 < MINSHORT)
		y1 = MINSHORT;
	    x2 = x1 + glyph->info.width;
	    if (x2 > MAXSHORT)
		x2 = MAXSHORT;
	    y2 = y1 + glyph->info.height;
	    if (y2 > MAXSHORT)
		y2 = MAXSHORT;

	    if (x1 >= line.x2)
	    {
		line.x2 = x2;
		if (y1 < line.y1)
		    line.y1 = y1;
		if (y2 > line.y2)
		    line.y2 = y2;
	    }
	    else if (x2 <= line.x1)
	    {
		line.x1 = x1;
		if (y1 < line.y1)
		    line.y1 = y1;
		if (y2 > line.y2)
		    line.y2 = y2;
	    }
	    else
	    {
		if (line.y1 >= extents->y2)
		{
		    extents->y2 = line.y2;
		    if (line.y1 < extents->y1)
			extents->y1 = line.y1;
		}
		else if (line.y2 <= extents->y1)
		{
		    extents->y1 = line.y1;
		    if (line.y2 > extents->y2)
			extents->y2 = line.y2;
		}
		else
		{
		    if (line.y1 < extents->y1)
			extents->y1 = line.y1;
		    if (line.y2 > extents->y2)
			extents->y2 = line.y2;

		    overlap = TRUE;
		}

		if (line.x1 < extents->x1)
		    extents->x1 = line.x1;
		if (line.x2 > extents->x2)
		    extents->x2 = line.x2;

		line.x1 = x1;
		line.y1 = y1;
		line.x2 = x2;
		line.y2 = y2;
	    }

	    x += glyph->info.xOff;
	    y += glyph->info.yOff;
	}
    }

    if (line.y1 >= extents->y2)
    {
	extents->y2 = line.y2;
	if (line.y1 < extents->y1)
	    extents->y1 = line.y1;
    }
    else if (line.y2 <= extents->y1)
    {
	extents->y1 = line.y1;
	if (line.y2 > extents->y2)
	    extents->y2 = line.y2;
    }
    else
    {
	if (line.y1 < extents->y1)
	    extents->y1 = line.y1;
	if (line.y2 > extents->y2)
	    extents->y2 = line.y2;

	overlap = TRUE;
    }

    if (line.x1 < extents->x1)
	extents->x1 = line.x1;
    if (line.x2 > extents->x2)
	extents->x2 = line.x2;

    xglPictureClipExtents (pDst, extents);

    return overlap;
}

/* returns 0 if all glyph lists don't have the same format */
static CARD32
xglGlyphListFormatId (GlyphListPtr list,
		      int	   nlist)
{
    CARD32 id = list->format->id;

    nlist--;
    list++;

    while (nlist--)
    {
	if (list->format->id != id)
	    return 0;

	list++;
    }

    return id;
}

void
xglGlyphs (CARD8	 op,
	   PicturePtr	 pSrc,
	   PicturePtr	 pDst,
	   PictFormatPtr maskFormat,
	   INT16	 xSrc,
	   INT16	 ySrc,
	   int		 nlist,
	   GlyphListPtr	 list,
	   GlyphPtr	 *glyphs)
{
    ScreenPtr	  pScreen = pDst->pDrawable->pScreen;
    PicturePtr	  pMask = NULL, pSrcPicture, pDstPicture;
    BoxRec	  extents;
    xglGlyphOpRec glyphOp;
    int		  xDst = list->xOff, yDst = list->yOff;
    int		  overlap;
    int		  target;

    overlap = xglGlyphExtents (pDst, nlist, list, glyphs, &extents);
    if (extents.x2 <= extents.x1 || extents.y2 <= extents.y1)
	return;

    target = xglPrepareTarget (pDst->pDrawable);

    if (op != PictOpAdd && maskFormat &&
	(!target || overlap || op != PictOpOver ||
	 xglGlyphListFormatId (list, nlist) != maskFormat->id))
    {
	PixmapPtr  pPixmap;
	XID	   componentAlpha;
	GCPtr	   pGC;
	xRectangle rect;
	int	   error;

	rect.x = 0;
	rect.y = 0;
	rect.width = extents.x2 - extents.x1;
	rect.height = extents.y2 - extents.y1;

	pPixmap = (*pScreen->CreatePixmap) (pScreen,
					    rect.width, rect.height,
					    maskFormat->depth,
					    CREATE_PIXMAP_USAGE_SCRATCH);
	if (!pPixmap)
	    return;

	componentAlpha = NEEDS_COMPONENT (maskFormat->format);
	pMask = CreatePicture (0, &pPixmap->drawable,
			       maskFormat, CPComponentAlpha, &componentAlpha,
			       serverClient, &error);
	if (!pMask)
	{
	    (*pScreen->DestroyPixmap) (pPixmap);
	    return;
	}

	if (!target)
	{
	    /* make sure we don't do accelerated drawing to mask */
	    xglSetPixmapVisual (pPixmap, NULL);
	}

	ValidatePicture (pMask);
	pGC = GetScratchGC (pPixmap->drawable.depth, pScreen);
	ValidateGC (&pPixmap->drawable, pGC);
	(*pGC->ops->PolyFillRect) (&pPixmap->drawable, pGC, 1, &rect);
	FreeScratchGC (pGC);

	(*pScreen->DestroyPixmap) (pPixmap);

	target = xglPrepareTarget (pMask->pDrawable);

	glyphOp.xOff = -extents.x1;
	glyphOp.yOff = -extents.y1;
	pSrcPicture = NULL;
	pDstPicture = pMask;
    }
    else
    {
	glyphOp.xOff = 0;
	glyphOp.yOff = 0;
	pSrcPicture = pSrc;
	pDstPicture = pDst;
    }

    glyphOp.ppGlyphs = glyphs;
    glyphOp.noCache  = !target;

    while (nlist--)
    {
	glyphOp.xOff   += list->xOff;
	glyphOp.yOff   += list->yOff;
	glyphOp.listLen = list->len;
	glyphOp.nGlyphs = list->len;
	glyphOp.pLists  = list++;

	for (; nlist; nlist--, list++)
	{
	    if (list->format->id != glyphOp.pLists->format->id)
		break;

	    glyphOp.nGlyphs += list->len;
	}

	while (glyphOp.nGlyphs)
	{
	    if (glyphOp.noCache || xglCachedGlyphs (op,
						    pSrcPicture,
						    pDstPicture,
						    xSrc - xDst, ySrc - yDst,
						    &glyphOp))
		xglUncachedGlyphs (op,
				   pSrcPicture,
				   pDstPicture,
				   xSrc - xDst, ySrc - yDst,
				   &glyphOp);
	}
    }

    if (pMask)
    {
	CompositePicture (op, pSrc, pMask, pDst,
			  xSrc + extents.x1 - xDst,
			  ySrc + extents.y1 - yDst,
			  0, 0,
			  extents.x1, extents.y1,
			  extents.x2 - extents.x1,
			  extents.y2 - extents.y1);

	FreePicture ((pointer) pMask, (XID) 0);
    }
}

#endif
