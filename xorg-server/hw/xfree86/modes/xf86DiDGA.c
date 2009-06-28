/*
 * Copyright © 2006 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#endif

#include "xf86.h"
#include "xf86DDC.h"
#include "xf86_OSproc.h"
#include "dgaproc.h"
#include "xf86Crtc.h"
#include "xf86Modes.h"
#include "gcstruct.h"
#include "scrnintstr.h"
#include "windowstr.h"

static Bool
xf86_dga_get_modes (ScreenPtr pScreen)
{
    ScrnInfoPtr		scrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
    DGAModePtr		modes, mode;
    DisplayModePtr	display_mode;
    int			bpp = scrn->bitsPerPixel >> 3;
    int			num;

    num = 0;
    display_mode = scrn->modes;
    while (display_mode) 
    {
	num++;
	display_mode = display_mode->next;
	if (display_mode == scrn->modes)
	    break;
    }
    
    if (!num)
	return FALSE;
    
    modes = xalloc(num * sizeof(DGAModeRec));
    if (!modes)
	return FALSE;
    
    num = 0;
    display_mode = scrn->modes;
    while (display_mode) 
    {
	mode = modes + num++;

	mode->mode = display_mode;
	mode->flags = DGA_CONCURRENT_ACCESS | DGA_PIXMAP_AVAILABLE;
        mode->flags |= DGA_FILL_RECT | DGA_BLIT_RECT;
	if (display_mode->Flags & V_DBLSCAN)
	    mode->flags |= DGA_DOUBLESCAN;
	if (display_mode->Flags & V_INTERLACE)
	    mode->flags |= DGA_INTERLACED;
	mode->byteOrder = scrn->imageByteOrder;
	mode->depth = scrn->depth;
	mode->bitsPerPixel = scrn->bitsPerPixel;
	mode->red_mask = scrn->mask.red;
	mode->green_mask = scrn->mask.green;
	mode->blue_mask = scrn->mask.blue;
	mode->visualClass = (bpp == 1) ? PseudoColor : TrueColor;
	mode->viewportWidth = display_mode->HDisplay;
	mode->viewportHeight = display_mode->VDisplay;
	mode->xViewportStep = (bpp == 3) ? 2 : 1;
	mode->yViewportStep = 1;
	mode->viewportFlags = DGA_FLIP_RETRACE;
	mode->offset = 0;
	mode->address = (unsigned char *) xf86_config->dga_address;
	mode->bytesPerScanline = xf86_config->dga_stride;
	mode->imageWidth = xf86_config->dga_width;
	mode->imageHeight = xf86_config->dga_height;
	mode->pixmapWidth = mode->imageWidth;
	mode->pixmapHeight = mode->imageHeight;
	mode->maxViewportX = mode->imageWidth -	mode->viewportWidth;
	mode->maxViewportY = mode->imageHeight - mode->viewportHeight;

	display_mode = display_mode->next;
	if (display_mode == scrn->modes)
	    break;
    }
    if (xf86_config->dga_modes)
	xfree (xf86_config->dga_modes);
    xf86_config->dga_nmode = num;
    xf86_config->dga_modes = modes;
    return TRUE;
}

static Bool
xf86_dga_set_mode(ScrnInfoPtr scrn, DGAModePtr display_mode)
{
    ScreenPtr		pScreen = scrn->pScreen;
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(scrn);

    if (!display_mode) 
    {
	if (xf86_config->dga_save_mode)
	{
	    xf86SwitchMode(pScreen, xf86_config->dga_save_mode);
	    xf86_config->dga_save_mode = NULL;
	}
    }
    else
    {
	if (!xf86_config->dga_save_mode)
	{
	    xf86_config->dga_save_mode = scrn->currentMode;
	    xf86SwitchMode(pScreen, display_mode->mode);
	}
    }
    return TRUE;
}

static int
xf86_dga_get_viewport(ScrnInfoPtr scrn)
{
    return 0;
}

static void
xf86_dga_set_viewport(ScrnInfoPtr scrn, int x, int y, int flags)
{
   scrn->AdjustFrame(scrn->pScreen->myNum, x, y, flags);
}

static Bool
xf86_dga_get_drawable_and_gc (ScrnInfoPtr scrn, DrawablePtr *ppDrawable, GCPtr *ppGC)
{
    ScreenPtr		pScreen = scrn->pScreen;
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
    PixmapPtr		pPixmap;
    GCPtr		pGC;
    
    pPixmap = GetScratchPixmapHeader (pScreen, xf86_config->dga_width, xf86_config->dga_height,
				      scrn->depth, scrn->bitsPerPixel, xf86_config->dga_stride, 
				      (char *) scrn->memPhysBase + scrn->fbOffset);
    if (!pPixmap)
	return FALSE;
    pGC  = GetScratchGC (scrn->depth, pScreen);
    if (!pGC)
    {
	FreeScratchPixmapHeader (pPixmap);
	return FALSE;
    }
    *ppDrawable = &pPixmap->drawable;
    *ppGC = pGC;
    return TRUE;
}

static void
xf86_dga_release_drawable_and_gc (ScrnInfoPtr scrn, DrawablePtr pDrawable, GCPtr pGC)
{
    FreeScratchGC (pGC);
    FreeScratchPixmapHeader ((PixmapPtr) pDrawable);
}

static void
xf86_dga_fill_rect(ScrnInfoPtr scrn, int x, int y, int w, int h, unsigned long color)
{
    GCPtr		pGC;
    DrawablePtr		pDrawable;
    XID			vals[1];
    xRectangle		r;

    if (!xf86_dga_get_drawable_and_gc (scrn, &pDrawable, &pGC))
	return;
    vals[0] = color;
    ChangeGC (pGC, GCForeground, vals);
    ValidateGC (pDrawable, pGC);
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    pGC->ops->PolyFillRect (pDrawable, pGC, 1, &r);
    xf86_dga_release_drawable_and_gc (scrn, pDrawable, pGC);
}

static void
xf86_dga_sync(ScrnInfoPtr scrn)
{
    ScreenPtr	pScreen = scrn->pScreen;
    WindowPtr	pRoot = WindowTable [pScreen->myNum];
    char	buffer[4];

    pScreen->GetImage (&pRoot->drawable, 0, 0, 1, 1, ZPixmap, ~0L, buffer);
}

static void
xf86_dga_blit_rect(ScrnInfoPtr scrn, int srcx, int srcy, int w, int h, int dstx, int dsty)
{
    DrawablePtr	pDrawable;
    GCPtr	pGC;

    if (!xf86_dga_get_drawable_and_gc (scrn, &pDrawable, &pGC))
	return;
    ValidateGC (pDrawable, pGC);
    pGC->ops->CopyArea (pDrawable, pDrawable, pGC, srcx, srcy, w, h, dstx, dsty);
    xf86_dga_release_drawable_and_gc (scrn, pDrawable, pGC);
}

static Bool
xf86_dga_open_framebuffer(ScrnInfoPtr scrn,
			  char **name,
			  unsigned char **mem, int *size, int *offset, int *flags)
{
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
    
    *size = xf86_config->dga_stride * xf86_config->dga_height;
    *mem = (unsigned char *) (xf86_config->dga_address);
    *offset = 0;
    *flags = DGA_NEED_ROOT;

    return TRUE;
}

static void
xf86_dga_close_framebuffer(ScrnInfoPtr scrn)
{
}

static DGAFunctionRec xf86_dga_funcs = {
   xf86_dga_open_framebuffer,
   xf86_dga_close_framebuffer,
   xf86_dga_set_mode,
   xf86_dga_set_viewport,
   xf86_dga_get_viewport,
   xf86_dga_sync,
   xf86_dga_fill_rect,
   xf86_dga_blit_rect,
   NULL
};

_X_EXPORT Bool
xf86DiDGAReInit (ScreenPtr pScreen)
{
    ScrnInfoPtr		scrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(scrn);
    
    if (!xf86_dga_get_modes (pScreen))
	return FALSE;
    
    return DGAReInitModes (pScreen, xf86_config->dga_modes, xf86_config->dga_nmode);
}

_X_EXPORT Bool
xf86DiDGAInit (ScreenPtr pScreen, unsigned long dga_address)
{
    ScrnInfoPtr		scrn = xf86Screens[pScreen->myNum];
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(scrn);

    xf86_config->dga_flags = 0;
    xf86_config->dga_address = dga_address;
    xf86_config->dga_width = scrn->virtualX;
    xf86_config->dga_height = scrn->virtualY;
    xf86_config->dga_stride = scrn->displayWidth * scrn->bitsPerPixel >> 3;
    
    if (!xf86_dga_get_modes (pScreen))
	return FALSE;
    
    return DGAInit(pScreen, &xf86_dga_funcs, xf86_config->dga_modes, xf86_config->dga_nmode);
}
