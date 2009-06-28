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
#include "inputstr.h"
#include "mipointer.h"
#include "damage.h"
#include "fb.h"
#ifdef MITSHM
#include "shmint.h"
static ShmFuncs shmFuncs = { NULL, xglShmPutImage };
#endif
#ifdef RENDER
#include "glyphstr.h"
#endif
#ifdef COMPOSITE
#include "compint.h"
#endif

int xglScreenGeneration = -1;
int xglScreenPrivateIndex;
int xglGCPrivateIndex;
int xglPixmapPrivateIndex;
int xglWinPrivateIndex;

#ifdef RENDER
int xglGlyphPrivateIndex;
#endif

#define xglQueryBestSize	  (void *) NoopDDA
#define xglSaveScreen		  (void *) NoopDDA

#define xglConstrainCursor	  (void *) NoopDDA
#define xglCursorLimits		  (void *) NoopDDA
#define xglDisplayCursor	  (void *) NoopDDA
#define xglRealizeCursor	  (void *) NoopDDA
#define xglUnrealizeCursor	  (void *) NoopDDA
#define xglRecolorCursor	  (void *) NoopDDA
#define xglSetCursorPosition	  (void *) NoopDDA

static Bool
xglAllocatePrivates (ScreenPtr pScreen)
{
    xglScreenPtr pScreenPriv;

    if (xglScreenGeneration != serverGeneration)
    {
	xglScreenPrivateIndex = AllocateScreenPrivateIndex ();
	if (xglScreenPrivateIndex < 0)
	    return FALSE;

	xglGCPrivateIndex = AllocateGCPrivateIndex ();
	if (xglGCPrivateIndex < 0)
	    return FALSE;

	xglPixmapPrivateIndex = AllocatePixmapPrivateIndex ();
	if (xglPixmapPrivateIndex < 0)
	    return FALSE;

	xglWinPrivateIndex = AllocateWindowPrivateIndex ();
	if (xglWinPrivateIndex < 0)
	    return FALSE;

#ifdef RENDER
	xglGlyphPrivateIndex = AllocateGlyphPrivateIndex ();
	if (xglGlyphPrivateIndex < 0)
	    return FALSE;
#endif

	xglScreenGeneration = serverGeneration;
    }

    if (!AllocateGCPrivate (pScreen, xglGCPrivateIndex, sizeof (xglGCRec)))
	return FALSE;

    if (!AllocatePixmapPrivate (pScreen, xglPixmapPrivateIndex,
				sizeof (xglPixmapRec)))
	return FALSE;

    if (!AllocateWindowPrivate (pScreen, xglWinPrivateIndex,
				sizeof (xglWinRec)))
	return FALSE;

    pScreenPriv = xalloc (sizeof (xglScreenRec));
    if (!pScreenPriv)
	return FALSE;

    XGL_SET_SCREEN_PRIV (pScreen, pScreenPriv);

    return TRUE;
}

Bool
xglScreenInit (ScreenPtr pScreen)
{
    xglScreenPtr pScreenPriv;
    xglVisualPtr v;
    int		 i, depth, bpp = 0;

#ifdef RENDER
    PictureScreenPtr pPictureScreen;
#endif

    depth = xglScreenInfo.depth;

    for (v = xglVisuals; v; v = v->next)
    {
	if (v->pPixel->depth == depth)
	{
	    bpp = v->pPixel->masks.bpp;
	    break;
	}
    }

    if (!bpp)
	return FALSE;

    if (!xglAllocatePrivates (pScreen))
	return FALSE;

    pScreenPriv = XGL_GET_SCREEN_PRIV (pScreen);

    pScreenPriv->pScreenPixmap = NULL;

    /* Add any unlisted depths from the pixmap formats */
    for (i = 0; i < screenInfo.numPixmapFormats; i++)
    {
	if (!xglHasVisualTypes (xglVisuals, screenInfo.formats[i].depth))
	    xglSetVisualTypes (screenInfo.formats[i].depth, 0, 0, 0, 0);
    }

    pScreenPriv->pVisual = 0;

#ifdef GLXEXT
    pScreenPriv->pGlxVisual = 0;
#endif

    pScreenPriv->rootVisual = 0;

    pScreenPriv->drawable = xglScreenInfo.drawable;
    pScreenPriv->features =
	glitz_drawable_get_features (xglScreenInfo.drawable);

    GEOMETRY_INIT (pScreen, &pScreenPriv->scratchGeometry,
		   GLITZ_GEOMETRY_TYPE_VERTEX,
		   pScreenPriv->geometryUsage, 0);

    pScreenPriv->geometryDataType = xglScreenInfo.geometryDataType;
    pScreenPriv->geometryUsage    = xglScreenInfo.geometryUsage;
    pScreenPriv->yInverted	  = xglScreenInfo.yInverted;
    pScreenPriv->pboMask	  = xglScreenInfo.pboMask;
    pScreenPriv->lines		  = xglScreenInfo.lines;
    pScreenPriv->accel		  = xglScreenInfo.accel;

    if (monitorResolution == 0)
	monitorResolution = XGL_DEFAULT_DPI;

    if (!fbSetupScreen (pScreen, NULL,
			xglScreenInfo.width, xglScreenInfo.height,
			monitorResolution, monitorResolution,
			xglScreenInfo.width, bpp))
	return FALSE;

    pScreen->SaveScreen = xglSaveScreen;

    pScreen->CreatePixmap  = xglCreatePixmap;
    pScreen->DestroyPixmap = xglDestroyPixmap;

    if (!fbFinishScreenInit (pScreen, NULL,
			     xglScreenInfo.width, xglScreenInfo.height,
			     monitorResolution, monitorResolution,
			     xglScreenInfo.width, bpp))
	return FALSE;

#ifdef MITSHM
    ShmRegisterFuncs (pScreen, &shmFuncs);
#endif

#ifdef RENDER
    if (!xglPictureInit (pScreen))
	return FALSE;
#endif

    XGL_SCREEN_WRAP (GetImage, xglGetImage);
    XGL_SCREEN_WRAP (GetSpans, xglGetSpans);

    XGL_SCREEN_WRAP (CopyWindow, xglCopyWindow);
    XGL_SCREEN_WRAP (CreateWindow, xglCreateWindow);
    XGL_SCREEN_WRAP (DestroyWindow, xglDestroyWindow);
    XGL_SCREEN_WRAP (ChangeWindowAttributes, xglChangeWindowAttributes);

    XGL_SCREEN_WRAP (CreateGC, xglCreateGC);

    pScreen->ConstrainCursor   = xglConstrainCursor;
    pScreen->CursorLimits      = xglCursorLimits;
    pScreen->DisplayCursor     = xglDisplayCursor;
    pScreen->RealizeCursor     = xglRealizeCursor;
    pScreen->UnrealizeCursor   = xglUnrealizeCursor;
    pScreen->RecolorCursor     = xglRecolorCursor;
    pScreen->SetCursorPosition = xglSetCursorPosition;

    pScreen->ModifyPixmapHeader = xglModifyPixmapHeader;

    XGL_SCREEN_WRAP (BitmapToRegion, xglPixmapToRegion);

    pScreen->GetWindowPixmap = xglGetWindowPixmap;

    XGL_SCREEN_WRAP (SetWindowPixmap, xglSetWindowPixmap);

#ifdef RENDER
    pPictureScreen = GetPictureScreenIfSet (pScreen);
    if (pPictureScreen)
    {
	if (!AllocateGlyphPrivate (pScreen, xglGlyphPrivateIndex,
				   sizeof (xglGlyphRec)))
	    return FALSE;

	XGL_PICTURE_SCREEN_WRAP (RealizeGlyph, xglRealizeGlyph);
	XGL_PICTURE_SCREEN_WRAP (UnrealizeGlyph, xglUnrealizeGlyph);
	XGL_PICTURE_SCREEN_WRAP (Composite, xglComposite);
	XGL_PICTURE_SCREEN_WRAP (Glyphs, xglGlyphs);
	XGL_PICTURE_SCREEN_WRAP (Trapezoids, xglTrapezoids);
	XGL_PICTURE_SCREEN_WRAP (AddTraps, xglAddTraps);
	XGL_PICTURE_SCREEN_WRAP (AddTriangles, xglAddTriangles);
	XGL_PICTURE_SCREEN_WRAP (ChangePicture, xglChangePicture);
	XGL_PICTURE_SCREEN_WRAP (ChangePictureTransform,
				 xglChangePictureTransform);
	XGL_PICTURE_SCREEN_WRAP (ChangePictureFilter, xglChangePictureFilter);
    }
#endif

    if (!fbCreateDefColormap (pScreen))
	return FALSE;

#ifdef COMPOSITE
#warning "composite building"
    if (!compScreenInit (pScreen))
	return FALSE;
#endif

    /* Damage is required */
    DamageSetup (pScreen);

    XGL_SCREEN_WRAP (CloseScreen, xglCloseScreen);

    return TRUE;
}

Bool
xglFinishScreenInit (ScreenPtr pScreen)
{
    xglVisualPtr v;

#ifdef RENDER
    glitz_vertex_format_t *format;
    static glitz_color_t  clearBlack = { 0x0, 0x0, 0x0, 0x0 };
    static glitz_color_t  solidWhite = { 0xffff, 0xffff, 0xffff, 0xffff };
    int			  i;
#endif

    XGL_SCREEN_PRIV (pScreen);

    xglInitVisuals (pScreen);

    for (v = pScreenPriv->pVisual; v; v = v->next)
    {
	if (v->vid == pScreen->rootVisual)
	    pScreenPriv->rootVisual = v;
    }

    if (!pScreenPriv->rootVisual || !pScreenPriv->rootVisual->format.surface)
	return FALSE;

    pScreenPriv->surface =
	glitz_surface_create (pScreenPriv->drawable,
			      pScreenPriv->rootVisual->format.surface,
			      xglScreenInfo.width, xglScreenInfo.height,
			      0, NULL);
    if (!pScreenPriv->surface)
	return FALSE;

    glitz_surface_attach (pScreenPriv->surface,
			  pScreenPriv->drawable,
			  GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);

#ifdef RENDER
    for (i = 0; i < 33; i++)
	pScreenPriv->glyphCache[i].pScreen = NULL;

    for (v = pScreenPriv->pVisual; v; v = v->next)
    {
	if (v->pPixel->depth == 8)
	    break;
    }

    pScreenPriv->pSolidAlpha    = 0;
    pScreenPriv->trapInfo.pMask = 0;

    /* An accelerated alpha only Xgl visual is required for trapezoid
       acceleration */
    if (v && v->format.surface)
    {
	glitz_surface_t *mask;

	mask = glitz_surface_create (pScreenPriv->drawable,
				     v->format.surface,
				     2, 1, 0, NULL);
	if (mask)
	{
	    glitz_set_rectangle (mask, &clearBlack, 0, 0, 1, 1);
	    glitz_set_rectangle (mask, &solidWhite, 1, 0, 1, 1);

	    glitz_surface_set_fill (mask, GLITZ_FILL_NEAREST);
	    glitz_surface_set_filter (mask, GLITZ_FILTER_BILINEAR, NULL, 0);

	    pScreenPriv->trapInfo.pMask = xglCreateDevicePicture (mask);
	    if (!pScreenPriv->trapInfo.pMask)
		return FALSE;
	}
    }

    format = &pScreenPriv->trapInfo.format.vertex;
    format->primitive  = GLITZ_PRIMITIVE_QUADS;
    format->attributes = GLITZ_VERTEX_ATTRIBUTE_MASK_COORD_MASK;

    format->mask.type	     = GLITZ_DATA_TYPE_FLOAT;
    format->mask.size	     = GLITZ_COORDINATE_SIZE_X;
    format->bytes_per_vertex = sizeof (glitz_float_t);

    if (pScreenPriv->geometryDataType)
    {
	format->type		  = GLITZ_DATA_TYPE_FLOAT;
	format->bytes_per_vertex += 2 * sizeof (glitz_float_t);
	format->mask.offset	  = 2 * sizeof (glitz_float_t);
    }
    else
    {
	format->type		  = GLITZ_DATA_TYPE_SHORT;
	format->bytes_per_vertex += 2 * sizeof (glitz_short_t);
	format->mask.offset	  = 2 * sizeof (glitz_short_t);
    }
#endif

#ifdef XV
    if (!xglXvScreenInit (pScreen))
       return FALSE;
#endif

    return TRUE;
}

Bool
xglCloseScreen (int	  index,
		ScreenPtr pScreen)
{
    xglVisualPtr v;

    XGL_SCREEN_PRIV (pScreen);
    XGL_PIXMAP_PRIV (pScreenPriv->pScreenPixmap);

#ifdef RENDER
    int i;

    for (i = 0; i < 33; i++)
	xglFiniGlyphCache (&pScreenPriv->glyphCache[i]);

    if (pScreenPriv->pSolidAlpha)
	FreePicture ((pointer) pScreenPriv->pSolidAlpha, 0);

    if (pScreenPriv->trapInfo.pMask)
	FreePicture ((pointer) pScreenPriv->trapInfo.pMask, 0);
#endif

    xglFiniPixmap (pScreenPriv->pScreenPixmap);
    if (pPixmapPriv->pDamage)
	DamageDestroy (pPixmapPriv->pDamage);

    if (pScreenPriv->surface)
	glitz_surface_destroy (pScreenPriv->surface);

    GEOMETRY_UNINIT (&pScreenPriv->scratchGeometry);

    while (pScreenPriv->pVisual)
    {
	v = pScreenPriv->pVisual;
	pScreenPriv->pVisual = v->next;
	xfree (v);
    }

#ifdef GLXEXT
    while (pScreenPriv->pGlxVisual)
    {
	v = pScreenPriv->pGlxVisual;
	pScreenPriv->pGlxVisual = v->next;
	xfree (v);
    }
#endif

    XGL_SCREEN_UNWRAP (CloseScreen);
    xfree (pScreenPriv);

    return (*pScreen->CloseScreen) (index, pScreen);
}

#ifdef RENDER
void
xglCreateSolidAlphaPicture (ScreenPtr pScreen)
{
    static xRenderColor	solidWhite = { 0xffff, 0xffff, 0xffff, 0xffff };
    static xRectangle	one = { 0, 0, 1, 1 };
    PixmapPtr		pPixmap;
    PictFormatPtr	pFormat;
    int			error;
    Pixel		pixel;
    GCPtr		pGC;
    XID			tmpval[2];

    XGL_SCREEN_PRIV (pScreen);

    pFormat = PictureMatchFormat (pScreen, 32, PICT_a8r8g8b8);
    if (!pFormat)
	return;

    pGC = GetScratchGC (pFormat->depth, pScreen);
    if (!pGC)
	return;

    pPixmap = (*pScreen->CreatePixmap) (pScreen, 1, 1, pFormat->depth, 0);
    if (!pPixmap)
	return;

    miRenderColorToPixel (pFormat, &solidWhite, &pixel);

    tmpval[0] = GXcopy;
    tmpval[1] = pixel;

    ChangeGC (pGC, GCFunction | GCForeground, tmpval);
    ValidateGC (&pPixmap->drawable, pGC);
    (*pGC->ops->PolyFillRect) (&pPixmap->drawable, pGC, 1, &one);
    FreeScratchGC (pGC);

    tmpval[0] = xTrue;
    pScreenPriv->pSolidAlpha = CreatePicture (0, &pPixmap->drawable, pFormat,
					      CPRepeat, tmpval,
					      serverClient, &error);
    (*pScreen->DestroyPixmap) (pPixmap);

    if (pScreenPriv->pSolidAlpha)
	ValidatePicture (pScreenPriv->pSolidAlpha);
}
#endif
