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
#include "colormapst.h"
#include "micmap.h"
#include "fb.h"

static xglPixelFormatRec xglPixelFormats[] = {
    {
	8, 8,
	{
	    8,
	    0x000000ff,
	    0x00000000,
	    0x00000000,
	    0x00000000
	}
    }, {
	15, 5,
	{
	    16,
	    0x00000000,
	    0x00007c00,
	    0x000003e0,
	    0x0000001f
	}
    }, {
	16, 6,
	{
	    16,
	    0x00000000,
	    0x0000f800,
	    0x000007e0,
	    0x0000001f
	}
    }, {
	24, 8,
	{
	    32,
	    0x00000000,
	    0x00ff0000,
	    0x0000ff00,
	    0x000000ff
	}
    }, {
	32, 8,
	{
	    32,
	    0xff000000,
	    0x00ff0000,
	    0x0000ff00,
	    0x000000ff
	}
    }
};

#define NUM_XGL_PIXEL_FORMATS				     \
    (sizeof (xglPixelFormats) / sizeof (xglPixelFormats[0]))

xglVisualPtr xglVisuals = NULL;

void
xglSetVisualTypes (int depth,
		   int visuals,
		   int redSize,
		   int greenSize,
		   int blueSize)
{
    xglPixelFormatPtr pBestFormat = 0;
    int		      i, rs, gs, bs, diff, bestDiff = 0;

    for (i = 0; i < NUM_XGL_PIXEL_FORMATS; i++)
    {
	if (xglPixelFormats[i].depth == depth)
	{
	    if (visuals)
	    {
		rs = Ones (xglPixelFormats[i].masks.red_mask);
		gs = Ones (xglPixelFormats[i].masks.green_mask);
		bs = Ones (xglPixelFormats[i].masks.blue_mask);

		if (redSize   >= rs &&
		    greenSize >= gs &&
		    blueSize  >= bs)
		{
		    diff = (redSize - rs) + (greenSize - gs) + (blueSize - bs);
		    if (pBestFormat)
		    {
			if (diff < bestDiff)
			{
			    pBestFormat = &xglPixelFormats[i];
			    bestDiff = diff;
			}
		    }
		    else
		    {
			pBestFormat = &xglPixelFormats[i];
			bestDiff = diff;
		    }
		}
	    }
	    else
	    {
		pBestFormat = &xglPixelFormats[i];
		break;
	    }
	}
    }

    if (pBestFormat)
    {
	xglVisualPtr new, *prev, v;
	unsigned int bitsPerRGB;
	Pixel	     rm, gm, bm;

	new = xalloc (sizeof (xglVisualRec));
	if (!new)
	    return;

	new->next = 0;

	new->format.surface  = 0;
	new->format.drawable = 0;
	new->pPixel	     = pBestFormat;
	new->vid	     = 0;

	bitsPerRGB = pBestFormat->bitsPerRGB;

	rm = pBestFormat->masks.red_mask;
	gm = pBestFormat->masks.green_mask;
	bm = pBestFormat->masks.blue_mask;

	fbSetVisualTypesAndMasks (depth, visuals, bitsPerRGB, rm, gm, bm);

	for (prev = &xglVisuals; (v = *prev); prev = &v->next);
	*prev = new;
    }
    else
    {
	fbSetVisualTypesAndMasks (depth, 0, 0, 0, 0, 0);
    }
}

Bool
xglHasVisualTypes (xglVisualPtr pVisual,
		   int		depth)
{
    xglVisualPtr v;

    for (v = pVisual; v; v = v->next)
	if (v->pPixel->depth == depth)
	    return TRUE;

    return FALSE;
}

glitz_format_t *
xglFindBestSurfaceFormat (ScreenPtr         pScreen,
			  xglPixelFormatPtr pPixel)
{
    glitz_format_t templ, *format, *best = 0;
    unsigned int   mask;
    unsigned short rs, gs, bs, as;
    int	           i = 0;

    XGL_SCREEN_PRIV (pScreen);

    rs = Ones (pPixel->masks.red_mask);
    gs = Ones (pPixel->masks.green_mask);
    bs = Ones (pPixel->masks.blue_mask);
    as = Ones (pPixel->masks.alpha_mask);

    templ.color.fourcc = GLITZ_FOURCC_RGB;
    mask = GLITZ_FORMAT_FOURCC_MASK;

    do {
	format = glitz_find_format (pScreenPriv->drawable, mask, &templ, i++);
	if (format)
	{
	    if (format->color.red_size   >= rs &&
		format->color.green_size >= gs &&
		format->color.blue_size  >= bs &&
		format->color.alpha_size >= as)
	    {
		if (best)
		{
		    if (((format->color.red_size   - rs) +
			 (format->color.green_size - gs) +
			 (format->color.blue_size  - bs)) <
			((best->color.red_size   - rs) +
			 (best->color.green_size - gs) +
			 (best->color.blue_size  - bs)))
			best = format;
		}
		else
		{
		    best = format;
		}
	    }
	}
    } while (format);

    return best;
}

static Bool
xglInitVisual (ScreenPtr	 pScreen,
	       xglVisualPtr	 pVisual,
	       xglPixelFormatPtr pPixel,
	       VisualID		 vid)
{
    glitz_format_t *format;

    XGL_SCREEN_PRIV (pScreen);

    format = xglFindBestSurfaceFormat (pScreen, pPixel);
    if (format)
    {
	glitz_drawable_format_t templ;
	unsigned long	        mask;

	templ.color        = format->color;
	templ.depth_size   = 0;
	templ.stencil_size = 0;
	templ.doublebuffer = 0;
	templ.samples      = 1;

	mask =
	    GLITZ_FORMAT_FOURCC_MASK       |
	    GLITZ_FORMAT_RED_SIZE_MASK     |
	    GLITZ_FORMAT_GREEN_SIZE_MASK   |
	    GLITZ_FORMAT_BLUE_SIZE_MASK    |
	    GLITZ_FORMAT_ALPHA_SIZE_MASK   |
	    GLITZ_FORMAT_DEPTH_SIZE_MASK   |
	    GLITZ_FORMAT_STENCIL_SIZE_MASK |
	    GLITZ_FORMAT_DOUBLEBUFFER_MASK |
	    GLITZ_FORMAT_SAMPLES_MASK;

	pVisual->next	 = 0;
	pVisual->vid	 = vid;
	pVisual->pPixel	 = pPixel;
	pVisual->pbuffer = FALSE;

	pVisual->format.surface  = format;
	pVisual->format.drawable =
	    glitz_find_drawable_format (pScreenPriv->drawable,
					mask, &templ, 0);

	return TRUE;
    }

    return FALSE;
}

static Bool
xglInitPbufferVisual (ScreenPtr	        pScreen,
		      xglVisualPtr	pVisual,
		      xglPixelFormatPtr pPixel,
		      VisualID		vid)
{
    glitz_format_t *format;

    XGL_SCREEN_PRIV (pScreen);

    format = xglFindBestSurfaceFormat (pScreen, pPixel);
    if (format)
    {
	glitz_drawable_format_t templ, *screenFormat;
	unsigned long	        mask;

	/* use same drawable format as screen for pbuffers */
	screenFormat = glitz_drawable_get_format (pScreenPriv->drawable);
	templ.id = screenFormat->id;

	templ.color   = format->color;
	templ.samples = 1;

	mask =
	    GLITZ_FORMAT_ID_MASK	 |
	    GLITZ_FORMAT_FOURCC_MASK     |
	    GLITZ_FORMAT_RED_SIZE_MASK   |
	    GLITZ_FORMAT_GREEN_SIZE_MASK |
	    GLITZ_FORMAT_BLUE_SIZE_MASK  |
	    GLITZ_FORMAT_SAMPLES_MASK;

	pVisual->next	 = 0;
	pVisual->vid	 = vid;
	pVisual->pPixel	 = pPixel;
	pVisual->pbuffer = TRUE;

	pVisual->format.surface  = format;
	pVisual->format.drawable =
	    glitz_find_pbuffer_format (pScreenPriv->drawable,
				       mask, &templ, 0);

	if (pVisual->format.drawable)
	    return TRUE;
    }

    return FALSE;
}

void
xglInitVisuals (ScreenPtr pScreen)
{
    xglVisualPtr pVisual, v, new, *prev;
    int		 i;

    XGL_SCREEN_PRIV (pScreen);

    for (i = 0; i < pScreen->numVisuals; i++)
    {
	for (pVisual = xglVisuals; pVisual; pVisual = pVisual->next)
	    if (pVisual->pPixel->depth == pScreen->visuals[i].nplanes)
		break;

	if (pVisual)
	{
	    new = xalloc (sizeof (xglVisualRec));
	    if (new)
	    {
		if (xglInitVisual (pScreen, new, pVisual->pPixel,
				   pScreen->visuals[i].vid))
		{
		    new->next = 0;

		    prev = &pScreenPriv->pVisual;
		    while ((v = *prev))
			prev = &v->next;

		    *prev = new;
		}
		else
		{
		    xfree (new);
		}
	    }

	    new = xalloc (sizeof (xglVisualRec));
	    if (new)
	    {
		if (xglInitPbufferVisual (pScreen, new, pVisual->pPixel,
					  pScreen->visuals[i].vid))
		{
		    new->next = 0;

		    prev = &pScreenPriv->pVisual;
		    while ((v = *prev))
			prev = &v->next;

		    *prev = new;
		}
		else
		{
		    xfree (new);
		}
	    }
	}
    }

    /* Add additional Xgl visuals for pixmap formats */
    for (i = 0; i < screenInfo.numPixmapFormats; i++)
    {
	if (!xglHasVisualTypes (pScreenPriv->pVisual,
				screenInfo.formats[i].depth))
	{
	    for (v = xglVisuals; v; v = v->next)
		if (v->pPixel->depth == screenInfo.formats[i].depth)
		    break;

	    if (v)
	    {
		new = xalloc (sizeof (xglVisualRec));
		if (new)
		{
		    if (xglInitVisual (pScreen, new, v->pPixel, 0))
		    {
			new->next = 0;

			prev = &pScreenPriv->pVisual;
			while ((v = *prev))
			    prev = &v->next;

			*prev = new;
		    }
		    else
		    {
			xfree (new);
		    }
		}
	    }
	}
    }
}

xglVisualPtr
xglFindVisualWithDepth (ScreenPtr pScreen,
			int       depth)
{
    xglVisualPtr v;

    XGL_SCREEN_PRIV (pScreen);

    for (v = pScreenPriv->pVisual; v; v = v->next)
    {
	if (v->pPixel->depth == depth)
	    return v;
    }

    return 0;
}

xglVisualPtr
xglFindVisualWithId (ScreenPtr pScreen,
		     int       vid)
{
    xglVisualPtr v;

    XGL_SCREEN_PRIV (pScreen);

    for (v = pScreenPriv->pVisual; v; v = v->next)
    {
	if (v->vid == vid)
	    return v;
    }

    return 0;
}

void
xglClearVisualTypes (void)
{
    xglVisualPtr v;

    while (xglVisuals)
    {
	v = xglVisuals;
	xglVisuals = v->next;
	xfree (v);
    }

    miClearVisualTypes ();
}
