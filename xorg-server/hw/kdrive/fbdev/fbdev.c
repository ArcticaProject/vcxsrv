/*
 * Copyright � 1999 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "fbdev.h"
#include <sys/ioctl.h>

#include <errno.h>

extern int KdTsPhyScreen;

char *fbdevDevicePath = NULL;

Bool
fbdevInitialize (KdCardInfo *card, FbdevPriv *priv)
{
    int		    k;
    unsigned long   off;

    if (fbdevDevicePath == NULL) 
      fbdevDevicePath = "/dev/fb0";

    if ((priv->fd = open(fbdevDevicePath, O_RDWR)) < 0)
      {
	ErrorF("Error opening framebuffer %s: %s\n", 
	       fbdevDevicePath, strerror(errno));
        return FALSE;
      }

    /* quiet valgrind */
    memset (&priv->fix, '\0', sizeof (priv->fix));
    if ((k=ioctl(priv->fd, FBIOGET_FSCREENINFO, &priv->fix)) < 0) {
	perror("Error with /dev/fb ioctl FIOGET_FSCREENINFO");
	close (priv->fd);
	return FALSE;
    }
    /* quiet valgrind */
    memset (&priv->var, '\0', sizeof (priv->var));
    if ((k=ioctl(priv->fd, FBIOGET_VSCREENINFO, &priv->var)) < 0) {
	perror("Error with /dev/fb ioctl FIOGET_VSCREENINFO");
	close (priv->fd);
	return FALSE;
    }

    priv->fb_base = (char *) mmap ((caddr_t) NULL,
				   priv->fix.smem_len,
				   PROT_READ|PROT_WRITE,
				   MAP_SHARED,
				   priv->fd, 0);
    
    if (priv->fb_base == (char *)-1) 
    {
        perror("ERROR: mmap framebuffer fails!");
	close (priv->fd);
	return FALSE;
    }
    off = (unsigned long) priv->fix.smem_start % (unsigned long) getpagesize();
    priv->fb = priv->fb_base + off;
    return TRUE;
}

Bool
fbdevCardInit (KdCardInfo *card)
{
    FbdevPriv	*priv;

    priv = (FbdevPriv *) xalloc (sizeof (FbdevPriv));
    if (!priv)
	return FALSE;
    
    if (!fbdevInitialize (card, priv))
    {
	xfree (priv);
	return FALSE;
    }
    card->driver = priv;
    
    return TRUE;
}

static Pixel
fbdevMakeContig (Pixel orig, Pixel others)
{
    Pixel   low;

    low = lowbit (orig) >> 1;
    while (low && (others & low) == 0)
    {
	orig |= low;
	low >>= 1;
    }
    return orig;
}

static Bool
fbdevModeSupported (KdScreenInfo		*screen,
		    const KdMonitorTiming	*t)
{
    return TRUE;
}

static void
fbdevConvertMonitorTiming (const KdMonitorTiming *t, struct fb_var_screeninfo *var)
{
    memset (var, 0, sizeof (struct fb_var_screeninfo));
    
    var->xres = t->horizontal;
    var->yres = t->vertical;
    var->xres_virtual = t->horizontal;
    var->yres_virtual = t->vertical;
    var->xoffset = 0;
    var->yoffset = 0;
    var->pixclock = t->clock ? 1000000000 / t->clock : 0;
    var->left_margin = t->hbp;
    var->right_margin = t->hfp;
    var->upper_margin = t->vbp;
    var->lower_margin = t->vfp;
    var->hsync_len = t->hblank - t->hfp - t->hbp;
    var->vsync_len = t->vblank - t->vfp - t->vbp;

    var->sync = 0;
    var->vmode = 0;

    if (t->hpol == KdSyncPositive)
      var->sync |= FB_SYNC_HOR_HIGH_ACT;
    if (t->vpol == KdSyncPositive)
      var->sync |= FB_SYNC_VERT_HIGH_ACT;
}

Bool
fbdevScreenInitialize (KdScreenInfo *screen, FbdevScrPriv *scrpriv)
{
    FbdevPriv	*priv = screen->card->driver;
    Pixel	allbits;
    int		depth;
    Bool	gray;
    struct fb_var_screeninfo var;
    const KdMonitorTiming *t;
    int k;

    k = ioctl (priv->fd, FBIOGET_VSCREENINFO, &var);
    
    if (!screen->width || !screen->height)
    {
	if (k >= 0) 
	{
	    screen->width = var.xres;
	    screen->height = var.yres;
	}
	else
	{
	    screen->width = 1024;
	    screen->height = 768;
	}
	screen->rate = 103; /* FIXME: should get proper value from fb driver */
    }
    if (!screen->fb[0].depth)
    {
	if (k >= 0) 
	    screen->fb[0].depth = var.bits_per_pixel;
	else
	    screen->fb[0].depth = 16;
    }

    if ((screen->width != var.xres) || (screen->height != var.yres))
    {
      t = KdFindMode (screen, fbdevModeSupported);
      screen->rate = t->rate;
      screen->width = t->horizontal;
      screen->height = t->vertical;

      /* Now try setting the mode */
      if (k < 0 || (t->horizontal != var.xres || t->vertical != var.yres))
          fbdevConvertMonitorTiming (t, &var);
    }

    var.activate = FB_ACTIVATE_NOW;
    var.bits_per_pixel = screen->fb[0].depth;
    var.nonstd = 0;
    var.grayscale = 0;

    k = ioctl (priv->fd, FBIOPUT_VSCREENINFO, &var);

    if (k < 0)
    {
	fprintf (stderr, "error: %s\n", strerror (errno));
	return FALSE;
    }

    /* Re-get the "fixed" parameters since they might have changed */
    k = ioctl (priv->fd, FBIOGET_FSCREENINFO, &priv->fix);
    if (k < 0)
        perror ("FBIOGET_FSCREENINFO");

    /* Now get the new screeninfo */
    ioctl (priv->fd, FBIOGET_VSCREENINFO, &priv->var);
    depth = priv->var.bits_per_pixel;
    gray = priv->var.grayscale;
    
    switch (priv->fix.visual) {
    case FB_VISUAL_PSEUDOCOLOR:
	if (gray)
	{
	    screen->fb[0].visuals = (1 << StaticGray);
	    /* could also support GrayScale, but what's the point? */
	}
	else
	{
	    screen->fb[0].visuals = ((1 << StaticGray) |
			       (1 << GrayScale) |
			       (1 << StaticColor) |
			       (1 << PseudoColor) |
			       (1 << TrueColor) |
			       (1 << DirectColor));
	}
	screen->fb[0].blueMask  = 0x00;
	screen->fb[0].greenMask = 0x00;
	screen->fb[0].redMask   = 0x00;
	break;
    case FB_VISUAL_STATIC_PSEUDOCOLOR:
	if (gray)
	{
	    screen->fb[0].visuals = (1 << StaticGray);
	}
	else
	{
	    screen->fb[0].visuals = (1 << StaticColor);
	}
	screen->fb[0].blueMask  = 0x00;
	screen->fb[0].greenMask = 0x00;
	screen->fb[0].redMask   = 0x00;
	break;
    case FB_VISUAL_TRUECOLOR:
    case FB_VISUAL_DIRECTCOLOR:
	screen->fb[0].visuals = (1 << TrueColor);
#define Mask(o,l)   (((1 << l) - 1) << o)
	screen->fb[0].redMask = Mask (priv->var.red.offset, priv->var.red.length);
	screen->fb[0].greenMask = Mask (priv->var.green.offset, priv->var.green.length);
	screen->fb[0].blueMask = Mask (priv->var.blue.offset, priv->var.blue.length);

	/*
	 * This is a kludge so that Render will work -- fill in the gaps
	 * in the pixel
	 */
	screen->fb[0].redMask = fbdevMakeContig (screen->fb[0].redMask,
						 screen->fb[0].greenMask|
						 screen->fb[0].blueMask);

	screen->fb[0].greenMask = fbdevMakeContig (screen->fb[0].greenMask,
						   screen->fb[0].redMask|
						   screen->fb[0].blueMask);

	screen->fb[0].blueMask = fbdevMakeContig (screen->fb[0].blueMask,
						  screen->fb[0].redMask|
						  screen->fb[0].greenMask);

	allbits = screen->fb[0].redMask | screen->fb[0].greenMask | screen->fb[0].blueMask;
	depth = 32;
	while (depth && !(allbits & (1 << (depth - 1))))
	    depth--;
	break;
    default:
	return FALSE;
	break;
    }
    screen->fb[0].depth = depth;
    screen->fb[0].bitsPerPixel = priv->var.bits_per_pixel;

    scrpriv->randr = screen->randr;

    return fbdevMapFramebuffer (screen);
}

Bool
fbdevScreenInit (KdScreenInfo *screen)
{
    FbdevScrPriv *scrpriv;

    scrpriv = xcalloc (1, sizeof (FbdevScrPriv));
    if (!scrpriv)
	return FALSE;
    screen->driver = scrpriv;
    if (!fbdevScreenInitialize (screen, scrpriv))
    {
	screen->driver = 0;
	xfree (scrpriv);
	return FALSE;
    }
    return TRUE;
}
    
void *
fbdevWindowLinear (ScreenPtr	pScreen,
		   CARD32	row,
		   CARD32	offset,
		   int		mode,
		   CARD32	*size,
		   void		*closure)
{
    KdScreenPriv(pScreen);
    FbdevPriv	    *priv = pScreenPriv->card->driver;

    if (!pScreenPriv->enabled)
	return 0;
    *size = priv->fix.line_length;
    return (CARD8 *) priv->fb + row * priv->fix.line_length + offset;
}

Bool
fbdevMapFramebuffer (KdScreenInfo *screen)
{
    FbdevScrPriv	*scrpriv = screen->driver;
    KdPointerMatrix	m;
    FbdevPriv		*priv = screen->card->driver;

    if (scrpriv->randr != RR_Rotate_0)
	scrpriv->shadow = TRUE;
    else
	scrpriv->shadow = FALSE;
    
    KdComputePointerMatrix (&m, scrpriv->randr, screen->width, screen->height);
    
    KdSetPointerMatrix (&m);
    
    screen->width = priv->var.xres;
    screen->height = priv->var.yres;
    screen->memory_base = (CARD8 *) (priv->fb);
    screen->memory_size = priv->fix.smem_len;
    
    if (scrpriv->shadow)
    {
	if (!KdShadowFbAlloc (screen, 0, 
			      scrpriv->randr & (RR_Rotate_90|RR_Rotate_270)))
	    return FALSE;
	screen->off_screen_base = screen->memory_size;
    }
    else
    {
        screen->fb[0].byteStride = priv->fix.line_length;
        screen->fb[0].pixelStride = (priv->fix.line_length * 8 / 
    				 priv->var.bits_per_pixel);
        screen->fb[0].frameBuffer = (CARD8 *) (priv->fb);
	screen->off_screen_base = screen->fb[0].byteStride * screen->height;
    }
    
    return TRUE;
}

void
fbdevSetScreenSizes (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KdScreenInfo	*screen = pScreenPriv->screen;
    FbdevScrPriv	*scrpriv = screen->driver;
    FbdevPriv		*priv = screen->card->driver;

    if (scrpriv->randr & (RR_Rotate_0|RR_Rotate_180))
    {
	pScreen->width = priv->var.xres;
	pScreen->height = priv->var.yres;
	pScreen->mmWidth = screen->width_mm;
	pScreen->mmHeight = screen->height_mm;
    }
    else
    {
	pScreen->width = priv->var.yres;
	pScreen->height = priv->var.xres;
	pScreen->mmWidth = screen->height_mm;
	pScreen->mmHeight = screen->width_mm;
    }
}

Bool
fbdevUnmapFramebuffer (KdScreenInfo *screen)
{
    KdShadowFbFree (screen, 0);
    return TRUE;
}

Bool
fbdevSetShadow (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KdScreenInfo	*screen = pScreenPriv->screen;
    FbdevScrPriv	*scrpriv = screen->driver;
    FbdevPriv		*priv = screen->card->driver;
    ShadowUpdateProc	update;
    ShadowWindowProc	window;
    int			useYX = 0;

#ifdef __arm__
    /* Use variant copy routines that always read left to right in the
       shadow framebuffer.  Reading vertical strips is exceptionally
       slow on XScale due to cache effects.  */
    useYX = 1;
#endif

    window = fbdevWindowLinear;
    update = 0;
    if (scrpriv->randr)
	if (priv->var.bits_per_pixel == 16) {
	    switch (scrpriv->randr) {
	    case RR_Rotate_90:
		if (useYX)
		    update = shadowUpdateRotate16_90YX;
		else
		    update =  shadowUpdateRotate16_90;
		break;
	    case RR_Rotate_180:
		update = shadowUpdateRotate16_180;
		break;
	    case RR_Rotate_270:
		if (useYX)
		    update = shadowUpdateRotate16_270YX;
		else
		    update =  shadowUpdateRotate16_270;
		break;
	    default:
		update = shadowUpdateRotate16;
		break;
	    }
	} else
	    update = shadowUpdateRotatePacked;
    else
	update = shadowUpdatePacked;
    return KdShadowSet (pScreen, scrpriv->randr, update, window);
}


#ifdef RANDR
Bool
fbdevRandRGetInfo (ScreenPtr pScreen, Rotation *rotations)
{
    KdScreenPriv(pScreen);
    KdScreenInfo	    *screen = pScreenPriv->screen;
    FbdevScrPriv	    *scrpriv = screen->driver;
    RRScreenSizePtr	    pSize;
    Rotation		    randr;
    int			    n;
    
    *rotations = RR_Rotate_All|RR_Reflect_All;
    
    for (n = 0; n < pScreen->numDepths; n++)
	if (pScreen->allowedDepths[n].numVids)
	    break;
    if (n == pScreen->numDepths)
	return FALSE;
    
    pSize = RRRegisterSize (pScreen,
			    screen->width,
			    screen->height,
			    screen->width_mm,
			    screen->height_mm);
    
    randr = KdSubRotation (scrpriv->randr, screen->randr);
    
    RRSetCurrentConfig (pScreen, randr, 0, pSize);
    
    return TRUE;
}

Bool
fbdevRandRSetConfig (ScreenPtr		pScreen,
		     Rotation		randr,
		     int		rate,
		     RRScreenSizePtr	pSize)
{
    KdScreenPriv(pScreen);
    KdScreenInfo	*screen = pScreenPriv->screen;
    FbdevScrPriv	*scrpriv = screen->driver;
    Bool		wasEnabled = pScreenPriv->enabled;
    FbdevScrPriv	oldscr;
    int			oldwidth;
    int			oldheight;
    int			oldmmwidth;
    int			oldmmheight;
    int			newwidth, newheight;

    if (screen->randr & (RR_Rotate_0|RR_Rotate_180))
    {
	newwidth = pSize->width;
	newheight = pSize->height;
    }
    else
    {
	newwidth = pSize->height;
	newheight = pSize->width;
    }

    if (wasEnabled)
	KdDisableScreen (pScreen);

    oldscr = *scrpriv;
    
    oldwidth = screen->width;
    oldheight = screen->height;
    oldmmwidth = pScreen->mmWidth;
    oldmmheight = pScreen->mmHeight;
    
    /*
     * Set new configuration
     */
    
    scrpriv->randr = KdAddRotation (screen->randr, randr);

    fbdevUnmapFramebuffer (screen);
    
    if (!fbdevMapFramebuffer (screen))
	goto bail4;

    KdShadowUnset (screen->pScreen);

    if (!fbdevSetShadow (screen->pScreen))
	goto bail4;

    fbdevSetScreenSizes (screen->pScreen);

    /*
     * Set frame buffer mapping
     */
    (*pScreen->ModifyPixmapHeader) (fbGetScreenPixmap (pScreen),
				    pScreen->width,
				    pScreen->height,
				    screen->fb[0].depth,
				    screen->fb[0].bitsPerPixel,
				    screen->fb[0].byteStride,
				    screen->fb[0].frameBuffer);
    
    /* set the subpixel order */
    
    KdSetSubpixelOrder (pScreen, scrpriv->randr);
    if (wasEnabled)
	KdEnableScreen (pScreen);

    return TRUE;

bail4:
    fbdevUnmapFramebuffer (screen);
    *scrpriv = oldscr;
    (void) fbdevMapFramebuffer (screen);
    pScreen->width = oldwidth;
    pScreen->height = oldheight;
    pScreen->mmWidth = oldmmwidth;
    pScreen->mmHeight = oldmmheight;
    
    if (wasEnabled)
	KdEnableScreen (pScreen);
    return FALSE;
}

Bool
fbdevRandRInit (ScreenPtr pScreen)
{
    rrScrPrivPtr    pScrPriv;
    
    if (!RRScreenInit (pScreen))
	return FALSE;

    pScrPriv = rrGetScrPriv(pScreen);
    pScrPriv->rrGetInfo = fbdevRandRGetInfo;
    pScrPriv->rrSetConfig = fbdevRandRSetConfig;
    return TRUE;
}
#endif

Bool
fbdevCreateColormap (ColormapPtr pmap)
{
    ScreenPtr		pScreen = pmap->pScreen;
    KdScreenPriv(pScreen);
    FbdevPriv		*priv = pScreenPriv->card->driver;
    VisualPtr		pVisual;
    int			i;
    int			nent;
    xColorItem		*pdefs;
    
    switch (priv->fix.visual) {
    case FB_VISUAL_STATIC_PSEUDOCOLOR:
	pVisual = pmap->pVisual;
	nent = pVisual->ColormapEntries;
	pdefs = xalloc (nent * sizeof (xColorItem));
	if (!pdefs)
	    return FALSE;
	for (i = 0; i < nent; i++)
	    pdefs[i].pixel = i;
	fbdevGetColors (pScreen, 0, nent, pdefs);
	for (i = 0; i < nent; i++)
	{
	    pmap->red[i].co.local.red = pdefs[i].red;
	    pmap->red[i].co.local.green = pdefs[i].green;
	    pmap->red[i].co.local.blue = pdefs[i].blue;
	}
	xfree (pdefs);
	return TRUE;
    default:
	return fbInitializeColormap (pmap);
    }
}

Bool
fbdevInitScreen (ScreenPtr pScreen)
{
#ifdef TOUCHSCREEN
    KdTsPhyScreen = pScreen->myNum;
#endif

    pScreen->CreateColormap = fbdevCreateColormap;
    return TRUE;
}

Bool
fbdevFinishInitScreen (ScreenPtr pScreen)
{
    if (!shadowSetup (pScreen))
	return FALSE;

#ifdef RANDR
    if (!fbdevRandRInit (pScreen))
	return FALSE;
#endif
    
    return TRUE;
}


Bool
fbdevCreateResources (ScreenPtr pScreen)
{
    return fbdevSetShadow (pScreen);
}

void
fbdevPreserve (KdCardInfo *card)
{
}

Bool
fbdevEnable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    FbdevPriv		*priv = pScreenPriv->card->driver;
    int			k;

    priv->var.activate = FB_ACTIVATE_NOW|FB_CHANGE_CMAP_VBL;
    
    /* display it on the LCD */
    k = ioctl (priv->fd, FBIOPUT_VSCREENINFO, &priv->var);
    if (k < 0)
    {
	perror ("FBIOPUT_VSCREENINFO");
	return FALSE;
    }
    
    if (priv->fix.visual == FB_VISUAL_DIRECTCOLOR)
    {
	struct fb_cmap	cmap;
	int		i;

	for (i = 0; 
	     i < (1 << priv->var.red.length) ||
	     i < (1 << priv->var.green.length) ||
	     i < (1 << priv->var.blue.length); i++)
	{
	    priv->red[i] = i * 65535 / ((1 << priv->var.red.length) - 1);
	    priv->green[i] = i * 65535 / ((1 << priv->var.green.length) - 1);
	    priv->blue[i] = i * 65535 / ((1 << priv->var.blue.length) - 1);
	}
	cmap.start = 0;
	cmap.len = i;
	cmap.red = &priv->red[0];
	cmap.green = &priv->green[0];
	cmap.blue = &priv->blue[0];
	cmap.transp = 0;
	ioctl (priv->fd, FBIOPUTCMAP, &cmap);
    }
    return TRUE;
}

Bool
fbdevDPMS (ScreenPtr pScreen, int mode)
{
    KdScreenPriv(pScreen);
    FbdevPriv	*priv = pScreenPriv->card->driver;
    static int oldmode = -1;

    if (mode == oldmode)
	return TRUE;
#ifdef FBIOPUT_POWERMODE
    if (ioctl (priv->fd, FBIOPUT_POWERMODE, &mode) >= 0)
    {
	oldmode = mode;
	return TRUE;
    }
#endif
#ifdef FBIOBLANK
    if (ioctl (priv->fd, FBIOBLANK, mode ? mode + 1 : 0) >= 0)
    {
	oldmode = mode;
	return TRUE;
    }
#endif
    return FALSE;
}

void
fbdevDisable (ScreenPtr pScreen)
{
}

void
fbdevRestore (KdCardInfo *card)
{
}

void
fbdevScreenFini (KdScreenInfo *screen)
{
}

void
fbdevCardFini (KdCardInfo *card)
{
    FbdevPriv	*priv = card->driver;
    
    munmap (priv->fb_base, priv->fix.smem_len);
    close (priv->fd);
    xfree (priv);
}

void
fbdevGetColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
    KdScreenPriv(pScreen);
    FbdevPriv	    *priv = pScreenPriv->card->driver;
    struct fb_cmap  cmap;
    int		    p;
    int		    k;
    int		    min, max;

    min = 256;
    max = 0;
    for (k = 0; k < n; k++)
    {
	if (pdefs[k].pixel < min)
	    min = pdefs[k].pixel;
	if (pdefs[k].pixel > max)
	    max = pdefs[k].pixel;
    }
    cmap.start = min;
    cmap.len = max - min + 1;
    cmap.red = &priv->red[min];
    cmap.green = &priv->green[min];
    cmap.blue = &priv->blue[min];
    cmap.transp = 0;
    k = ioctl (priv->fd, FBIOGETCMAP, &cmap);
    if (k < 0)
    {
	perror ("can't get colormap");
	return;
    }
    while (n--)
    {
	p = pdefs->pixel;
	pdefs->red = priv->red[p];
	pdefs->green = priv->green[p];
	pdefs->blue = priv->blue[p];
	pdefs++;
    }
}

void
fbdevPutColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
    KdScreenPriv(pScreen);
    FbdevPriv	*priv = pScreenPriv->card->driver;
    struct fb_cmap  cmap;
    int		    p;
    int		    min, max;

    min = 256;
    max = 0;
    while (n--)
    {
	p = pdefs->pixel;
	priv->red[p] = pdefs->red;
	priv->green[p] = pdefs->green;
	priv->blue[p] = pdefs->blue;
	if (p < min)
	    min = p;
	if (p > max)
	    max = p;
	pdefs++;
    }
    cmap.start = min;
    cmap.len = max - min + 1;
    cmap.red = &priv->red[min];
    cmap.green = &priv->green[min];
    cmap.blue = &priv->blue[min];
    cmap.transp = 0;
    ioctl (priv->fd, FBIOPUTCMAP, &cmap);
}
