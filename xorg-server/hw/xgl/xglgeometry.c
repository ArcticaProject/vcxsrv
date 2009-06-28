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
#include <X11/fonts/fontstruct.h>
#include "dixfontstr.h"

xglDataTypeInfoRec xglGeometryDataTypes[2] = {
    { GLITZ_DATA_TYPE_SHORT, sizeof (glitz_short_t) },
    { GLITZ_DATA_TYPE_FLOAT, sizeof (glitz_float_t) }
};

glitz_buffer_hint_t usageTypes[] = {
    GLITZ_BUFFER_HINT_STREAM_DRAW,
    GLITZ_BUFFER_HINT_STATIC_DRAW,
    GLITZ_BUFFER_HINT_DYNAMIC_DRAW
};

void
xglGeometryResize (ScreenPtr	  pScreen,
		   xglGeometryPtr pGeometry,
		   int		  size)
{
    XGL_SCREEN_PRIV (pScreen);

    if (size == pGeometry->size)
	return;

    if (pGeometry->broken)
	return;

    if (pGeometry->usage == GEOMETRY_USAGE_SYSMEM)
    {
	pGeometry->data = xrealloc (pGeometry->data, size);

	if (pGeometry->buffer)
	    glitz_buffer_destroy (pGeometry->buffer);

	pGeometry->buffer = NULL;

	if (pGeometry->data)
	{
	    pGeometry->buffer = glitz_buffer_create_for_data (pGeometry->data);
	    if (!pGeometry->buffer)
	    {
		pGeometry->broken = TRUE;
		return;
	    }
	}
	else if (size)
	{
	    pGeometry->broken = TRUE;
	    return;
	}
    }
    else
    {
	glitz_buffer_t *newBuffer;

	if (size)
	{
	    newBuffer =
		glitz_vertex_buffer_create (pScreenPriv->drawable, NULL, size,
					    usageTypes[pGeometry->usage]);
	    if (!newBuffer)
	    {
		pGeometry->broken = TRUE;
		return;
	    }
	} else
	    newBuffer = NULL;

	if (pGeometry->buffer && newBuffer)
	{
	    void *oldData, *newData;

	    oldData = glitz_buffer_map (pGeometry->buffer,
					GLITZ_BUFFER_ACCESS_READ_ONLY);
	    newData = glitz_buffer_map (newBuffer,
					GLITZ_BUFFER_ACCESS_WRITE_ONLY);

	    if (oldData && newData)
		memcpy (newData, oldData, MIN (size, pGeometry->size));

	    glitz_buffer_unmap (pGeometry->buffer);
	    glitz_buffer_unmap (newBuffer);

	    glitz_buffer_destroy (pGeometry->buffer);
	}
	pGeometry->buffer = newBuffer;
    }

    pGeometry->size = size;

    if (pGeometry->endOffset > size)
	pGeometry->endOffset = size;
}

#define MAP_GEOMETRY(pScreen, pGeometry, offset, units, ptr, _size)	  \
    if ((pGeometry)->broken)						  \
	return;								  \
    (_size) = (units) * xglGeometryDataTypes[(pGeometry)->dataType].size; \
    if (((pGeometry)->size - (offset)) < (_size))			  \
    {									  \
	xglGeometryResize (pScreen, pGeometry,				  \
			   (pGeometry)->endOffset + (_size) + 500);	  \
	if ((pGeometry)->broken)					  \
	    return;							  \
    }									  \
    (ptr) = glitz_buffer_map ((pGeometry)->buffer,			  \
			      GLITZ_BUFFER_ACCESS_WRITE_ONLY);		  \
    if (!(ptr))								  \
    {									  \
	(pGeometry)->broken = TRUE;					  \
	return;								  \
    }									  \
    (ptr) += (offset)

#define UNMAP_GEOMETRY(pGeometry, offset, _size)			   \
    if (glitz_buffer_unmap ((pGeometry)->buffer))			   \
    {									   \
	(pGeometry)->broken = TRUE;					   \
	return;								   \
    }									   \
    if (((offset) + (_size)) > (pGeometry)->endOffset)			   \
    {									   \
	(pGeometry)->endOffset = (offset) + (_size);			   \
	(pGeometry)->count = (pGeometry)->endOffset /			   \
	    (2 * xglGeometryDataTypes[(pGeometry)->dataType].size);	   \
    }

/*
 * Adds a number of boxes as GL_QUAD primitives
 */
void
xglGeometryAddBox (ScreenPtr	  pScreen,
		   xglGeometryPtr pGeometry,
		   BoxPtr	  pBox,
		   int		  nBox,
		   int		  offset)
{
    int  size;
    char *ptr;

    if (nBox < 1)
	return;

    MAP_GEOMETRY (pScreen, pGeometry, offset, nBox * 8, ptr, size);

    switch (pGeometry->dataType) {
    case GEOMETRY_DATA_TYPE_SHORT:
    {
	glitz_short_t *data = (glitz_short_t *) ptr;

	while (nBox--)
	{
	    *data++ = (glitz_short_t) pBox->x1;
	    *data++ = (glitz_short_t) pBox->y1;
	    *data++ = (glitz_short_t) pBox->x2;
	    *data++ = (glitz_short_t) pBox->y1;
	    *data++ = (glitz_short_t) pBox->x2;
	    *data++ = (glitz_short_t) pBox->y2;
	    *data++ = (glitz_short_t) pBox->x1;
	    *data++ = (glitz_short_t) pBox->y2;

	    pBox++;
	}
    } break;
    case GEOMETRY_DATA_TYPE_FLOAT:
    {
	glitz_float_t *data = (glitz_float_t *) ptr;

	while (nBox--)
	{
	    *data++ = (glitz_float_t) pBox->x1;
	    *data++ = (glitz_float_t) pBox->y1;
	    *data++ = (glitz_float_t) pBox->x2;
	    *data++ = (glitz_float_t) pBox->y1;
	    *data++ = (glitz_float_t) pBox->x2;
	    *data++ = (glitz_float_t) pBox->y2;
	    *data++ = (glitz_float_t) pBox->x1;
	    *data++ = (glitz_float_t) pBox->y2;

	    pBox++;
	}
    } break;
    }

    UNMAP_GEOMETRY (pGeometry, offset, size);
}

/*
 * Adds a number of spans as GL_LINE primitives
 */
void
xglGeometryAddSpan (ScreenPtr	   pScreen,
		    xglGeometryPtr pGeometry,
		    DDXPointPtr	   ppt,
		    int		   *pwidth,
		    int		   n,
		    int		   offset)
{
    int  size;
    char *ptr;

    if (n < 1)
	return;

    MAP_GEOMETRY (pScreen, pGeometry, offset, n * 4, ptr, size);

    switch (pGeometry->dataType) {
    case GEOMETRY_DATA_TYPE_SHORT:
    {
	glitz_short_t *data = (glitz_short_t *) ptr;

	while (n--)
	{
	    *data++ = (glitz_short_t) ppt->x;
	    *data++ = (glitz_short_t) ppt->y;
	    *data++ = (glitz_short_t) (ppt->x + *pwidth);
	    *data++ = (glitz_short_t) ppt->y;

	    ppt++;
	    pwidth++;
	}
    } break;
    case GEOMETRY_DATA_TYPE_FLOAT:
    {
	glitz_float_t *data = (glitz_float_t *) ptr;

	while (n--)
	{
	    *data++ = (glitz_float_t) ppt->x;
	    *data++ = (glitz_float_t) ppt->y;
	    *data++ = (glitz_float_t) (ppt->x + *pwidth);
	    *data++ = (glitz_float_t) ppt->y;

	    ppt++;
	    pwidth++;
	}
    } break;
    }

    UNMAP_GEOMETRY (pGeometry, offset, size);
}

/*
 * This macro is needed for end pixels to be rasterized correctly using
 * OpenGL as OpenGL line segments are half-opened.
 */
#define ADJUST_END_POINT(start, end, isPoint) \
    (((end) > (start)) ? (end) + 1:	      \
     ((end) < (start)) ? (end) - 1:	      \
     (isPoint)	       ? (end) + 1:	      \
     (end))

/*
 * Adds a number of connected lines as GL_LINE_STRIP primitives
 */
void
xglGeometryAddLine (ScreenPtr	   pScreen,
		    xglGeometryPtr pGeometry,
		    int		   loop,
		    int		   mode,
		    int		   npt,
		    DDXPointPtr    ppt,
		    int		   offset)
{
    DDXPointRec pt;
    int		size;
    char	*ptr;

    if (npt < 2)
	return;

    MAP_GEOMETRY (pScreen, pGeometry, offset, npt * 2, ptr, size);

    pt.x = 0;
    pt.y = 0;

    switch (pGeometry->dataType) {
    case GEOMETRY_DATA_TYPE_SHORT:
    {
	glitz_short_t *data = (glitz_short_t *) ptr;

	while (npt--)
	{
	    if (mode == CoordModePrevious)
	    {
		pt.x += ppt->x;
		pt.y += ppt->y;
	    }
	    else
	    {
		pt.x = ppt->x;
		pt.y = ppt->y;
	    }

	    if (npt || loop)
	    {
		*data++ = (glitz_short_t) pt.x;
		*data++ = (glitz_short_t) pt.y;
	    }
	    else
	    {
		ppt--;
		*data++ = (glitz_short_t)
		    ADJUST_END_POINT (ppt->x, pt.x, ppt->y == pt.y);
		*data++ = (glitz_short_t) ADJUST_END_POINT (ppt->y, pt.y, 0);
	    }

	    ppt++;
	}
    } break;
    case GEOMETRY_DATA_TYPE_FLOAT:
    {
	glitz_float_t *data = (glitz_float_t *) ptr;

	while (npt--)
	{
	    if (mode == CoordModePrevious)
	    {
		pt.x += ppt->x;
		pt.y += ppt->y;
	    }
	    else
	    {
		pt.x = ppt->x;
		pt.y = ppt->y;
	    }

	    if (npt || loop)
	    {
		*data++ = (glitz_float_t) pt.x;
		*data++ = (glitz_float_t) pt.y;
	    }
	    else
	    {
		ppt--;
		*data++ = (glitz_float_t)
		    ADJUST_END_POINT (ppt->x, pt.x, ppt->y == pt.y);
		*data++ = (glitz_float_t) ADJUST_END_POINT (ppt->y, pt.y, 0);
	    }

	    ppt++;
	}
    } break;
    }

    UNMAP_GEOMETRY (pGeometry, offset, size);
}

/*
 * Adds a number of line segments as GL_LINE primitives
 */
void
xglGeometryAddSegment (ScreenPtr      pScreen,
		       xglGeometryPtr pGeometry,
		       int	      nsegInit,
		       xSegment       *pSegInit,
		       int	      offset)
{
    int  size;
    char *ptr;

    if (nsegInit < 1)
	return;

    MAP_GEOMETRY (pScreen, pGeometry, offset, nsegInit * 4, ptr, size);

    switch (pGeometry->dataType) {
    case GEOMETRY_DATA_TYPE_SHORT:
    {
	glitz_short_t *data = (glitz_short_t *) ptr;

	while (nsegInit--)
	{
	    *data++ = (glitz_short_t) pSegInit->x1;
	    *data++ = (glitz_short_t) pSegInit->y1;
	    *data++ = (glitz_short_t)
		ADJUST_END_POINT (pSegInit->x1, pSegInit->x2,
				  pSegInit->y1 == pSegInit->y2);
	    *data++ = (glitz_short_t)
		ADJUST_END_POINT (pSegInit->y1, pSegInit->y2, 0);

	    pSegInit++;
	}
    } break;
    case GEOMETRY_DATA_TYPE_FLOAT:
    {
	glitz_float_t *data = (glitz_float_t *) ptr;

	while (nsegInit--)
	{
	    *data++ = (glitz_float_t) pSegInit->x1;
	    *data++ = (glitz_float_t) pSegInit->y1;
	    *data++ = (glitz_float_t)
		ADJUST_END_POINT (pSegInit->x1, pSegInit->x2,
				  pSegInit->y1 == pSegInit->y2);
	    *data++ = (glitz_float_t)
		ADJUST_END_POINT (pSegInit->y1, pSegInit->y2, 0);

	    pSegInit++;
	}
    } break;
    }

    UNMAP_GEOMETRY (pGeometry, offset, size);
}

void
xglGeometryForGlyph (ScreenPtr	    pScreen,
		     xglGeometryPtr pGeometry,
		     unsigned int   nGlyph,
		     CharInfoPtr    *ppciInit,
		     pointer	    pglyphBase)
{
    CharInfoPtr		*ppci;
    CharInfoPtr		pci;
    unsigned char	*glyphbase = (pointer) ~0;
    unsigned char	*pglyph;
    int			x = 0;
    int			gx, gy;
    int			gWidth, gHeight;
    int			n, lastX = 0, lastY = 0;
    glitz_multi_array_t *array;
    glitz_buffer_t	*buffer;

    ppci = ppciInit;
    n = nGlyph;

    while (n--)
    {
	pglyph = FONTGLYPHBITS (pglyphBase, *ppci++);
	if (pglyph < glyphbase)
	    glyphbase = pglyph;
    }

    buffer = glitz_buffer_create_for_data (glyphbase);
    if (!buffer)
    {
	pGeometry->broken = TRUE;
	return;
    }

    GEOMETRY_SET_BUFFER (pGeometry, buffer);

    array = glitz_multi_array_create (nGlyph);
    if (!array)
    {
	pGeometry->broken = TRUE;
	return;
    }

    GEOMETRY_SET_MULTI_ARRAY (pGeometry, array);

    ppci = ppciInit;
    while (nGlyph--)
    {
	pci = *ppci++;
	pglyph = FONTGLYPHBITS (pglyphBase, pci);
	gWidth = GLYPHWIDTHPIXELS (pci);
	gHeight = GLYPHHEIGHTPIXELS (pci);

	if (gWidth && gHeight)
	{
	    gx = x + pci->metrics.leftSideBearing;
	    gy = -pci->metrics.ascent;

	    glitz_multi_array_add (array,
				   (pglyph - glyphbase) * 8,
				   gWidth, gHeight,
				   (gx - lastX) << 16, (gy - lastY) << 16);
	    lastX = gx;
	    lastY = gy;
	}
	x += pci->metrics.characterWidth;
    }

    glitz_buffer_destroy (buffer);
    glitz_multi_array_destroy (array);
}

#define FIXED_LINE_X_TO_FLOAT(line, v)		  \
    (((glitz_float_t)				  \
	((line).p1.x + (xFixed_16_16)		  \
	 (((xFixed_32_32) ((v) - (line).p1.y) *   \
	   ((line).p2.x - (line).p1.x)) /	  \
	  ((line).p2.y - (line).p1.y)))) / 65536)

#define FIXED_LINE_X_CEIL_TO_FLOAT(line, v)	\
  (((glitz_float_t)				\
      ((line).p1.x + (xFixed_16_16)		\
       (((((line).p2.y - (line).p1.y) - 1) +	\
	 ((xFixed_32_32) ((v) - (line).p1.y) *	\
	  ((line).p2.x - (line).p1.x))) /	\
	((line).p2.y - (line).p1.y)))) / 65536)

/*
 * Adds a number of trapezoids as GL_QUAD primitives
 */
void
xglGeometryAddTrapezoid (ScreenPtr	pScreen,
			 xglGeometryPtr pGeometry,
			 xTrapezoid	*pTrap,
			 int		nTrap,
			 int		offset)
{
    int  size;
    char *ptr;

    if (nTrap < 1)
	return;

    MAP_GEOMETRY (pScreen, pGeometry, offset, nTrap * 8, ptr, size);

    switch (pGeometry->dataType) {
    case GEOMETRY_DATA_TYPE_SHORT:
	/* not supported */
	pGeometry->broken = TRUE;
	break;
    case GEOMETRY_DATA_TYPE_FLOAT:
    {
	glitz_float_t *data = (glitz_float_t *) ptr;
	glitz_float_t top, bottom;

	while (nTrap--)
	{
	    top    = FIXED_TO_FLOAT (pTrap->top);
	    bottom = FIXED_TO_FLOAT (pTrap->bottom);

	    *data++ = FIXED_LINE_X_TO_FLOAT (pTrap->left, pTrap->top);
	    *data++ = top;
	    *data++ = FIXED_LINE_X_CEIL_TO_FLOAT (pTrap->right, pTrap->top);
	    *data++ = top;
	    *data++ = FIXED_LINE_X_CEIL_TO_FLOAT (pTrap->right, pTrap->bottom);
	    *data++ = bottom;
	    *data++ = FIXED_LINE_X_TO_FLOAT (pTrap->left, pTrap->bottom);
	    *data++ = bottom;

	    pTrap++;
	}
    } break;
    }

    UNMAP_GEOMETRY (pGeometry, offset, size);
}

/*
 * Adds a number of traps as GL_QUAD primitives
 */
void
xglGeometryAddTrap (ScreenPtr	   pScreen,
		    xglGeometryPtr pGeometry,
		    xTrap	   *pTrap,
		    int		   nTrap,
		    int		   offset)
{
    int  size;
    char *ptr;

    if (nTrap < 1)
	return;

    MAP_GEOMETRY (pScreen, pGeometry, offset, nTrap * 8, ptr, size);

    switch (pGeometry->dataType) {
    case GEOMETRY_DATA_TYPE_SHORT:
	/* not supported */
	pGeometry->broken = TRUE;
	break;
    case GEOMETRY_DATA_TYPE_FLOAT:
    {
	glitz_float_t *data = (glitz_float_t *) ptr;
	glitz_float_t top, bottom;

	while (nTrap--)
	{
	    top    = FIXED_TO_FLOAT (pTrap->top.y);
	    bottom = FIXED_TO_FLOAT (pTrap->bot.y);

	    *data++ = FIXED_TO_FLOAT (pTrap->top.l);
	    *data++ = top;
	    *data++ = FIXED_TO_FLOAT (pTrap->top.r);
	    *data++ = top;
	    *data++ = FIXED_TO_FLOAT (pTrap->bot.r);
	    *data++ = bottom;
	    *data++ = FIXED_TO_FLOAT (pTrap->bot.l);
	    *data++ = bottom;

	    pTrap++;
	}
    } break;
    }

    UNMAP_GEOMETRY (pGeometry, offset, size);
}

/* XXX: scratch geometry size never shrinks, it just gets larger when
   required. this is not acceptable. */
xglGeometryPtr
xglGetScratchGeometryWithSize (ScreenPtr pScreen,
			       int	 size)
{
    xglGeometryPtr pGeometry;

    XGL_SCREEN_PRIV (pScreen);

    pGeometry = &pScreenPriv->scratchGeometry;

    if (pGeometry->broken || pGeometry->size < size)
    {
	GEOMETRY_UNINIT (pGeometry);
	GEOMETRY_INIT (pScreen, pGeometry, pGeometry->type,
		       pScreenPriv->geometryUsage, size);
    }
    else
    {
	if (pGeometry->array)
	{
	    glitz_multi_array_destroy (pGeometry->array);
	    pGeometry->array = NULL;
	}
	pGeometry->endOffset = 0;
	pGeometry->xOff      = 0;
	pGeometry->yOff      = 0;
	pGeometry->first     = 0;
	pGeometry->count     = 0;
	pGeometry->width     = 2;
    }

    return pGeometry;
}

xglGeometryPtr
xglGetScratchVertexGeometryWithType (ScreenPtr pScreen,
				     int       type,
				     int       count)
{
    xglGeometryPtr pGeometry;
    int		   stride;

    stride = 2 * xglGeometryDataTypes[type].size;

    pGeometry = xglGetScratchGeometryWithSize (pScreen, count * stride);

    pGeometry->type	= GLITZ_GEOMETRY_TYPE_VERTEX;
    pGeometry->dataType	= type;

    pGeometry->f.vertex.primitive	 = GLITZ_PRIMITIVE_QUADS;
    pGeometry->f.vertex.type		 = xglGeometryDataTypes[type].type;
    pGeometry->f.vertex.bytes_per_vertex = stride;
    pGeometry->f.vertex.attributes       = 0;

    return pGeometry;
}

xglGeometryPtr
xglGetScratchVertexGeometry (ScreenPtr pScreen,
			     int	 count)
{
    xglGeometryPtr pGeometry;
    int		   type, stride;

    XGL_SCREEN_PRIV (pScreen);

    type   = pScreenPriv->geometryDataType;
    stride = 2 * xglGeometryDataTypes[type].size;

    pGeometry = xglGetScratchGeometryWithSize (pScreen, count * stride);

    pGeometry->type	= GLITZ_GEOMETRY_TYPE_VERTEX;
    pGeometry->dataType	= type;

    pGeometry->f.vertex.primitive	 = GLITZ_PRIMITIVE_QUADS;
    pGeometry->f.vertex.type		 = xglGeometryDataTypes[type].type;
    pGeometry->f.vertex.bytes_per_vertex = stride;
    pGeometry->f.vertex.attributes       = 0;

    return pGeometry;
}

Bool
xglSetGeometry (xglGeometryPtr	pGeometry,
		glitz_surface_t *surface)
{
    if (pGeometry->broken)
	return FALSE;

    glitz_set_geometry (surface, pGeometry->type, &pGeometry->f,
			pGeometry->buffer);

    if (pGeometry->array)
	glitz_set_multi_array (surface, pGeometry->array,
			       pGeometry->xOff, pGeometry->yOff);
    else
	glitz_set_array (surface,
			 pGeometry->first, pGeometry->width, pGeometry->count,
			 pGeometry->xOff, pGeometry->yOff);

    return TRUE;
}
