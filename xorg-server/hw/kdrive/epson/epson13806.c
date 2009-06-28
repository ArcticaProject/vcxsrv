/*
 * Copyright 2004 by Costas Stylianou <costas.stylianou@psion.com> +44(0)7850 394095
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Costas Sylianou not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission. Costas Stylianou makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * COSTAS STYLIANOU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL COSTAS STYLIANOU BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * epson13806.c - Implementation of hardware accelerated functions for
 *                Epson S1D13806 graphics controller.
 *
 * History:
 * 28-Jan-04  C.Stylianou                     PRJ NBL: Created from fbdev.c.
 * 30-Mar-04  Phil Blundell/Peter Naulls      Integration with XFree 4.3
 *  
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif

#include <sys/ioctl.h>

#include "epson13806.h"
#include "epson13806reg.h"

extern int KdTsPhyScreen;

Bool
epsonInitialize (KdCardInfo *card, EpsonPriv *priv)
{
    int k;
    unsigned long off;
    if ((priv->fd = open("/dev/fb0", O_RDWR)) < 0) {
        perror("Error opening /dev/fb0\n");
        return FALSE;
        }

    if ((k=ioctl(priv->fd, FBIOGET_FSCREENINFO, &priv->fix)) < 0) {
        perror("Error with /dev/fb ioctl FIOGET_FSCREENINFO");
        close (priv->fd);
        return FALSE;
        }

    if ((k=ioctl(priv->fd, FBIOGET_VSCREENINFO, &priv->var)) < 0) {
        perror("Error with /dev/fb ioctl FIOGET_VSCREENINFO");
        close (priv->fd);
        return FALSE;
        }
    
    priv->fb_base = KdMapDevice (EPSON13806_PHYSICAL_VMEM_ADDR, EPSON13806_VMEM_SIZE);
        
        
    if (priv->fb_base == (char *)-1) {
        perror("ERROR: mmap framebuffer fails!");
        close (priv->fd);
        return FALSE;
        }

    off = (unsigned long) priv->fix.smem_start % (unsigned long) getpagesize();
    priv->fb = priv->fb_base + off;
    return TRUE;
}

Bool
epsonCardInit (KdCardInfo *card)
{
    EpsonPriv    *priv;

    priv = (EpsonPriv *) xalloc (sizeof (EpsonPriv));
    if (!priv)
    return FALSE;
    
    if (!epsonInitialize (card, priv))
    {
    	xfree (priv);
    	return FALSE;
    }
    card->driver = priv;
    
    // Call InitEpson to map onto Epson registers
    initEpson13806();
    
    return TRUE;
}

#define FBDEV_KLUDGE_FORMAT
#ifdef FBDEV_KLUDGE_FORMAT
static Pixel
epsonMakeContig (Pixel orig, Pixel others)
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
#endif

Bool
epsonScreenInitialize (KdScreenInfo *screen, EpsonScrPriv *scrpriv)
{
    EpsonPriv    *priv = screen->card->driver;
    Pixel    allbits;
    int        depth;
    Bool    gray;
    depth = priv->var.bits_per_pixel;
    gray = priv->var.grayscale;
    
    
    screen->fb[0].visuals = (1 << TrueColor);
#define Mask(o,l)   (((1 << l) - 1) << o)
    screen->fb[0].redMask = Mask (priv->var.red.offset, priv->var.red.length);
    screen->fb[0].greenMask = Mask (priv->var.green.offset, priv->var.green.length);
    screen->fb[0].blueMask = Mask (priv->var.blue.offset, priv->var.blue.length);
#ifdef FBDEV_KLUDGE_FORMAT
    /*
     * This is a kludge so that Render will work -- fill in the gaps
     * in the pixel
     */
    screen->fb[0].redMask = epsonMakeContig (screen->fb[0].redMask,
                         screen->fb[0].greenMask|
                         screen->fb[0].blueMask);

    screen->fb[0].greenMask = epsonMakeContig (screen->fb[0].greenMask,
                           screen->fb[0].redMask|
                           screen->fb[0].blueMask);

    screen->fb[0].blueMask = epsonMakeContig (screen->fb[0].blueMask,
                          screen->fb[0].redMask|
                          screen->fb[0].greenMask);

#endif
    allbits = screen->fb[0].redMask | screen->fb[0].greenMask | screen->fb[0].blueMask;
    depth = 32;
    while (depth && !(allbits & (1 << (depth - 1))))
        depth--;
    
    screen->rate = 60;
    scrpriv->randr = screen->randr;
    
    {
        screen->fb[0].depth = depth;
        screen->fb[0].bitsPerPixel = priv->var.bits_per_pixel;
        screen->width = priv->var.xres;
        screen->height = priv->var.yres;
        screen->fb[0].byteStride = priv->fix.line_length;
        screen->fb[0].pixelStride = (priv->fix.line_length * 8 / 
                     priv->var.bits_per_pixel);
        screen->fb[0].frameBuffer = (CARD8 *) (priv->fb);
        screen->off_screen_base = screen->fb[0].byteStride * screen->height;
        screen->memory_base = priv->fb;
        screen->memory_size = EPSON13806_VMEM_SIZE;
    }
    return TRUE;
}

Bool
epsonScreenInit (KdScreenInfo *screen)
{
    EpsonScrPriv *scrpriv;

    scrpriv = xalloc (sizeof (EpsonScrPriv));
    if (!scrpriv)
        return FALSE;
    memset (scrpriv, '\0', sizeof (EpsonScrPriv));
    screen->driver = scrpriv;
    if (!epsonScreenInitialize (screen, scrpriv)) {
        screen->driver = 0;
    xfree (scrpriv);
    return FALSE;
    }
    return TRUE;
}
    
static void *
epsonWindowLinear (ScreenPtr    pScreen,
           CARD32    row,
           CARD32    offset,
           int        mode,
           CARD32    *size,
           void        *closure)
{
    KdScreenPriv(pScreen);
    EpsonPriv        *priv = pScreenPriv->card->driver;

    if (!pScreenPriv->enabled)
        return 0;
    *size = priv->fix.line_length;
    return (CARD8 *) priv->fb + row * priv->fix.line_length + offset;
}


#ifdef RANDR
static Bool
epsonRandRGetInfo (ScreenPtr pScreen, Rotation *rotations)
{
    KdScreenPriv(pScreen);
    KdScreenInfo        *screen = pScreenPriv->screen;
    EpsonScrPriv        *scrpriv = screen->driver;
#if 0
    RRVisualGroupPtr        pVisualGroup;
    RRGroupOfVisualGroupPtr pGroupOfVisualGroup;
#endif
    RRScreenSizePtr        pSize;
    Rotation            randr;
    int                n;
    
    *rotations = RR_Rotate_0|RR_Rotate_90|RR_Rotate_180|RR_Rotate_270;
    
    for (n = 0; n < pScreen->numDepths; n++)
        if (pScreen->allowedDepths[n].numVids)
            break;
    if (n == pScreen->numDepths)
        return FALSE;

#if 0    
    pVisualGroup = RRCreateVisualGroup (pScreen);
    if (!pVisualGroup)
        return FALSE;
    
    if (!RRAddDepthToVisualGroup (pScreen, pVisualGroup, &pScreen->allowedDepths[n])) {
        RRDestroyVisualGroup (pScreen, pVisualGroup);
        return FALSE;
        }

    pVisualGroup = RRRegisterVisualGroup (pScreen, pVisualGroup);
    if (!pVisualGroup)
        return FALSE;
    
    pGroupOfVisualGroup = RRCreateGroupOfVisualGroup (pScreen);

    if (!RRAddVisualGroupToGroupOfVisualGroup (pScreen,
                     pGroupOfVisualGroup,
                     pVisualGroup))
    {
    RRDestroyGroupOfVisualGroup (pScreen, pGroupOfVisualGroup);
    /* pVisualGroup left until screen closed */
    return FALSE;
    }

    pGroupOfVisualGroup = RRRegisterGroupOfVisualGroup (pScreen, pGroupOfVisualGroup);
    if (!pGroupOfVisualGroup)
    return FALSE;
#endif

    pSize = RRRegisterSize (pScreen,
                screen->width,
                screen->height,
                screen->width_mm,
                screen->height_mm);
    
    randr = KdSubRotation (scrpriv->randr, screen->randr);
    
    RRSetCurrentConfig (pScreen, randr, RR_Rotate_0, pSize);
    
    return TRUE;
}

static Bool
epsonRandRSetConfig (ScreenPtr        pScreen,
             Rotation        randr,
             int        rate,
             RRScreenSizePtr    pSize)
{
    KdScreenPriv(pScreen);
    KdScreenInfo    *screen = pScreenPriv->screen;
    EpsonScrPriv    *scrpriv = screen->driver;
    Bool        wasEnabled = pScreenPriv->enabled;

    randr = KdAddRotation (randr, screen->randr);

    if (scrpriv->randr != randr)
    {
        if (wasEnabled)
            KdDisableScreen (pScreen);
    
        scrpriv->randr = randr;

        if (wasEnabled)
            KdEnableScreen (pScreen);
    }
    return TRUE;
}

static Bool
epsonRandRInit (ScreenPtr pScreen)
{
    rrScrPrivPtr    pScrPriv;
    
    if (!RRScreenInit (pScreen))
    return FALSE;

    pScrPriv = rrGetScrPriv(pScreen);
    pScrPriv->rrGetInfo = epsonRandRGetInfo;
    pScrPriv->rrSetConfig = epsonRandRSetConfig;
    return TRUE;
}
#endif

static Bool
epsonCreateColormap (ColormapPtr pmap)
{
    ScreenPtr        pScreen = pmap->pScreen;
    KdScreenPriv(pScreen);
    EpsonPriv        *priv = pScreenPriv->card->driver;
    VisualPtr        pVisual;
    int            i;
    int            nent;
    xColorItem        *pdefs;
    
    switch (priv->fix.visual) {
        case FB_VISUAL_STATIC_PSEUDOCOLOR:
            pVisual = pmap->pVisual;
            nent = pVisual->ColormapEntries;
            pdefs = xalloc (nent * sizeof (xColorItem));
            if (!pdefs)
                return FALSE;
            for (i = 0; i < nent; i++)
                pdefs[i].pixel = i;
            epsonGetColors (pScreen, 0, nent, pdefs);
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
epsonInitScreen (ScreenPtr pScreen)
{
#ifdef TOUCHSCREEN
    KdTsPhyScreen = pScreen->myNum;
#endif

    pScreen->CreateColormap = epsonCreateColormap;

    return TRUE;
}

static Bool
epsonFinishInitScreen (ScreenPtr pScreen)
{
    if (!shadowSetup (pScreen))
    return FALSE;

#ifdef RANDR
    if (!epsonRandRInit (pScreen))
    return FALSE;
#endif

    return TRUE;
}

static Bool
epsonSetShadow (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KdScreenInfo    *screen = pScreenPriv->screen;
    EpsonScrPriv    *scrpriv = screen->driver;
    ShadowUpdateProc    update;
    ShadowWindowProc    window;

    window = epsonWindowLinear;
    update = shadowUpdatePacked;

    return KdShadowSet (pScreen, scrpriv->randr, update, window);
}

static Bool
epsonCreateResources (ScreenPtr pScreen)
{
    return epsonSetShadow (pScreen);
}

void
epsonPreserve (KdCardInfo *card)
{
}

Bool
epsonEnable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    EpsonPriv        *priv = pScreenPriv->card->driver;
    int            k;

    priv->var.activate = FB_ACTIVATE_NOW|FB_CHANGE_CMAP_VBL;
    
    /* display it on the LCD */
    k = ioctl (priv->fd, FBIOPUT_VSCREENINFO, &priv->var);
    if (k < 0) {
        perror ("FBIOPUT_VSCREENINFO");
        return FALSE;
        }

    k = ioctl (priv->fd, FBIOGET_FSCREENINFO, &priv->fix);
    if (k < 0) {
        perror ("FBIOGET_FSCREENINFO");
        return FALSE;
    }

    if (priv->fix.visual == FB_VISUAL_DIRECTCOLOR) {
        struct fb_cmap    cmap;
        int        i;

        for (i = 0; 
             i < (1 << priv->var.red.length) ||
             i < (1 << priv->var.green.length) ||
             i < (1 << priv->var.blue.length); i++) {
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
epsonDPMS (ScreenPtr pScreen, int mode)
{
    KdScreenPriv(pScreen);
    EpsonPriv    *priv = pScreenPriv->card->driver;
    static int oldmode = -1;

    if (mode == oldmode)
        return TRUE;
#ifdef FBIOPUT_POWERMODE
    if (ioctl (priv->fd, FBIOPUT_POWERMODE, &mode) >= 0) {
        oldmode = mode;
        return TRUE;
        }
#endif
#ifdef FBIOBLANK
    if (ioctl (priv->fd, FBIOBLANK, mode ? mode + 1 : 0) >= 0) {
        oldmode = mode;
        return TRUE;
        }
#endif
    return FALSE;
}

void
epsonDisable (ScreenPtr pScreen)
{
}

void
epsonRestore (KdCardInfo *card)
{
}

void
epsonScreenFini (KdScreenInfo *screen)
{
}

void
epsonCardFini (KdCardInfo *card)
{
    EpsonPriv    *priv = card->driver;
    
    munmap (priv->fb_base, priv->fix.smem_len);
    close (priv->fd);
    xfree (priv);
}

void
epsonGetColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
    KdScreenPriv(pScreen);
    EpsonPriv        *priv = pScreenPriv->card->driver;
    struct fb_cmap  cmap;
    int            p;
    int            k;
    int            min, max;

    min = 256;
    max = 0;
    for (k = 0; k < n; k++) {
        if (pdefs[k].pixel < min)
            min = pdefs[k].pixel;
        if (pdefs[k].pixel > max)
            max = pdefs[k].pixel;
        }
    cmap.start = min;
    cmap.len = max - min + 1;
    cmap.red = &priv->red[min];
    cmap.green = &priv->green[min];;
    cmap.blue = &priv->blue[min];
    cmap.transp = 0;
    k = ioctl (priv->fd, FBIOGETCMAP, &cmap);
    if (k < 0) {
        perror ("can't get colormap");
        return;
        }
    while (n--) {
    p = pdefs->pixel;
    pdefs->red = priv->red[p];
    pdefs->green = priv->green[p];
    pdefs->blue = priv->blue[p];
    pdefs++;
    }
}

void
epsonPutColors (ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
    KdScreenPriv(pScreen);
    EpsonPriv    *priv = pScreenPriv->card->driver;
    struct fb_cmap  cmap;
    int            p;
    int            min, max;

    min = 256;
    max = 0;
    while (n--) {
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



KdCardFuncs epsonFuncs = {
    epsonCardInit,          /* cardinit */
    epsonScreenInit,        /* scrinit */
    epsonInitScreen,        /* initScreen */
    epsonFinishInitScreen,
    epsonCreateResources,
    epsonPreserve,          /* preserve */
    epsonEnable,            /* enable */
    epsonDPMS,              /* dpms */
    epsonDisable,           /* disable */
    epsonRestore,           /* restore */
    epsonScreenFini,        /* scrfini */
    epsonCardFini,          /* cardfini */
    
    0,                      /* initCursor */
    0,                      /* enableCursor */
    0,                      /* disableCursor */
    0,                      /* finiCursor */
    0,                      /* recolorCursor */
    
    /* 
     * History:
     * 28-Jan-04  C.Stylianou       NBL: Added the following for h/w accel.
     *
     */

    epsonDrawInit,          /* initAccel */
    epsonDrawEnable,        /* enableAccel */
    epsonDrawDisable,       /* disableAccel */
    epsonDrawFini,          /* finiAccel */
    
    epsonGetColors,         /* getColors */
    epsonPutColors,         /* putColors */
};
