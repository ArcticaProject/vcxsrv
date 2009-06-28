/*
 * Copyright © 2003 Keith Packard
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
#include "nvidia.h"
#include "kaa.h"
#include <sys/io.h>

static Bool
nvidiaCardInit (KdCardInfo *card)
{
    NvidiaCardInfo	*nvidiac;

    nvidiac = (NvidiaCardInfo *) xalloc (sizeof (NvidiaCardInfo));
    if (!nvidiac)
	return FALSE;
    
    (void) nvidiaMapReg (card, nvidiac);
    
    if (!vesaInitialize (card, &nvidiac->vesa))
    {
	xfree (nvidiac);
	return FALSE;
    }

    card->driver = nvidiac;
    
    return TRUE;
}

static Bool
nvidiaScreenInit (KdScreenInfo *screen)
{
    NvidiaCardInfo	*nvidiac = screen->card->driver;
    NvidiaScreenInfo	*nvidias;
    int			screen_size, memory;

    nvidias = (NvidiaScreenInfo *) xalloc (sizeof (NvidiaScreenInfo));
    if (!nvidias)
	return FALSE;
    memset (nvidias, '\0', sizeof (NvidiaScreenInfo));
    if (!vesaScreenInitialize (screen, &nvidias->vesa))
    {
	xfree (nvidias);
	return FALSE;
    }
    if (!nvidiac->reg_base)
	screen->dumb = TRUE;
    if (nvidias->vesa.mapping != VESA_LINEAR)
	screen->dumb = TRUE;
    nvidias->screen = nvidias->vesa.fb;
    memory = nvidias->vesa.fb_size;
    screen_size = screen->fb[0].byteStride * screen->height;
    if (nvidias->screen && memory >= screen_size + 2048)
    {
	memory -= 2048;
	nvidias->cursor_base = nvidias->screen + memory - 2048;
    }
    else
	nvidias->cursor_base = 0;
    screen->softCursor = TRUE;	/* XXX for now */
    memory -= screen_size;
    if (memory > screen->fb[0].byteStride)
    {
	nvidias->off_screen = nvidias->screen + screen_size;
	nvidias->off_screen_size = memory;
    }
    else
    {
	nvidias->off_screen = 0;
	nvidias->off_screen_size = 0;
    }
    screen->driver = nvidias;
    return TRUE;
}

static Bool
nvidiaInitScreen (ScreenPtr pScreen)
{
#if 0
#ifdef XV
    KdScreenPriv(pScreen);
    NvidiaCardInfo	*nvidiac = pScreenPriv->screen->card->driver;
    if (nvidiac->media_reg && nvidiac->reg)
	nvidiaInitVideo(pScreen);
#endif
#endif
    return vesaInitScreen (pScreen);
}

#ifdef RANDR
static Bool
nvidiaRandRSetConfig (ScreenPtr		pScreen,
		      Rotation		rotation,
		      int		rate,
		      RRScreenSizePtr	pSize)
{
    kaaWaitSync (pScreen);

    if (!vesaRandRSetConfig (pScreen, rotation, rate, pSize))
	return FALSE;
    
    return TRUE;
}

static void
nvidiaRandRInit (ScreenPtr pScreen)
{
    rrScrPriv(pScreen);

    pScrPriv->rrSetConfig = nvidiaRandRSetConfig;
}
#endif

static Bool
nvidiaFinishInitScreen (ScreenPtr pScreen)
{
    Bool    ret;
    ret = vesaFinishInitScreen (pScreen);
#ifdef RANDR
    nvidiaRandRInit (pScreen);
#endif
    return ret;
}

void
nvidiaPreserve (KdCardInfo *card)
{
    vesaPreserve(card);
}

void
nvidiaOutb (NvidiaCardInfo *nvidiac, CARD16 port, CARD8 val)
{
    asm volatile ("outb %b0,%w1" : : "a" (val), "d" (port));
}

CARD8
nvidiaInb (NvidiaCardInfo *nvidiac, CARD16 port)
{
    CARD8   v;
    asm volatile ("inb %w1,%b0" : "=a" (v) : "d" (port));
    return v;
}

CARD8
nvidiaGetIndex (NvidiaCardInfo *nvidiac, CARD16 addr, CARD16 data, CARD8 id)
{
    CARD8   ret;
    DBGOUT ("nvidiaGetIndex(0x%x,0x%x)\n", addr, id);
    nvidiaOutb (nvidiac, addr, id);
    ret = nvidiaInb (nvidiac, data);
    DBGOUT ("    -> 0x%x\n", ret);
    return ret;
}

void
nvidiaSetIndex (NvidiaCardInfo *nvidiac, CARD16 addr, CARD16 data, CARD8 id, CARD8 val)
{
    DBGOUT ("nvidiaSetIndex(0x%x,0x%x) = 0x%x\n", addr, id, val);
    nvidiaOutb (nvidiac, addr, id);
    nvidiaOutb (nvidiac, data, val);
}

static void vgaLockUnlock (NvidiaCardInfo *nvidiac, Bool lock)
{
    CARD8 cr11;
    ENTER ();
    cr11 = nvidiaGetIndex (nvidiac, 0x3d4, 0x3d5, 0x11);
    if (lock) cr11 |= 0x80;
    else cr11 &= ~0x80;
    nvidiaSetIndex (nvidiac, 0x3d4, 0x3d5, 0x11, cr11);
    LEAVE ();
}

static void nvidiaLockUnlock (NvidiaCardInfo *nvidiac, Bool lock)
{
    if (NVIDIA_IS_3(nvidiac))
	nvidiaSetIndex (nvidiac, 0x3c4, 0x3c5, 0x06, lock ? 0x99 : 0x57);
    else
	nvidiaSetIndex (nvidiac, 0x3c4, 0x3c5, 0x1f, lock ? 0x99 : 0x57);
    vgaLockUnlock(nvidiac, lock);
}

Bool
nvidiaMapReg (KdCardInfo *card, NvidiaCardInfo *nvidiac)
{
    nvidiac->reg_base = (CARD8 *) KdMapDevice (NVIDIA_REG_BASE(card),
						NVIDIA_REG_SIZE(card));
    
    if (!nvidiac->reg_base)
    {
	nvidiac->mmio = 0;
	nvidiac->rop = 0;
	nvidiac->blt = 0;
	nvidiac->rect = 0;
	return FALSE;
    }
    
    nvidiac->mmio = (CARD8 *) (nvidiac->reg_base + NVIDIA_MMIO_OFF(card));
    nvidiac->rop = (NvidiaRop *) (nvidiac->reg_base + NVIDIA_ROP_OFF(card));
    nvidiac->rect = (NvidiaRectangle *) (nvidiac->reg_base + NVIDIA_RECTANGLE_OFF(card));
    nvidiac->blt = (NvidiaScreenBlt *) (nvidiac->reg_base + NVIDIA_BLT_OFF(card));
    nvidiac->busy = (NvidiaBusy *) (nvidiac->reg_base + NVIDIA_BUSY_OFF(card));
    KdSetMappedMode (NVIDIA_REG_BASE(card),
		     NVIDIA_REG_SIZE(card),
		     KD_MAPPED_MODE_REGISTERS);
    return TRUE;
}

void
nvidiaUnmapReg (KdCardInfo *card, NvidiaCardInfo *nvidiac)
{
    if (nvidiac->reg_base)
    {
	KdResetMappedMode (NVIDIA_REG_BASE(card),
			   NVIDIA_REG_SIZE(card),
			   KD_MAPPED_MODE_REGISTERS);
	KdUnmapDevice ((void *) nvidiac->reg_base, NVIDIA_REG_SIZE(card));
	nvidiac->reg_base = 0;
	nvidiac->rop = 0;
	nvidiac->blt = 0;
	nvidiac->rect = 0;
    }
}

void
nvidiaSetMMIO (KdCardInfo *card, NvidiaCardInfo *nvidiac)
{
    if (!nvidiac->reg_base)
	nvidiaMapReg (card, nvidiac);
    nvidiaLockUnlock (nvidiac, FALSE);
    nvidiac->fifo_free = 0;
    nvidiac->fifo_size = nvidiac->rop->FifoFree.FifoFree;
}

void
nvidiaResetMMIO (KdCardInfo *card, NvidiaCardInfo *nvidiac)
{
    nvidiaUnmapReg (card, nvidiac);
    nvidiaLockUnlock (nvidiac, TRUE);
}

Bool
nvidiaEnable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    NvidiaCardInfo	*nvidiac = pScreenPriv->card->driver;

    if (!vesaEnable (pScreen))
	return FALSE;
    
    nvidiaSetMMIO (pScreenPriv->card, nvidiac);
#ifdef XV
    KdXVEnable (pScreen);
#endif
    return TRUE;
}

void
nvidiaDisable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    NvidiaCardInfo	*nvidiac = pScreenPriv->card->driver;

#ifdef XV
    KdXVDisable (pScreen);
#endif
    nvidiaResetMMIO (pScreenPriv->card, nvidiac);
    vesaDisable (pScreen);
}

static Bool
nvidiaDPMS (ScreenPtr pScreen, int mode)
{
    return vesaDPMS (pScreen, mode);
}

static void
nvidiaRestore (KdCardInfo *card)
{
    NvidiaCardInfo	*nvidiac = card->driver;

    nvidiaResetMMIO (card, nvidiac);
    vesaRestore (card);
}

static void
nvidiaScreenFini (KdScreenInfo *screen)
{
    NvidiaScreenInfo	*nvidias = (NvidiaScreenInfo *) screen->driver;

    vesaScreenFini (screen);
    xfree (nvidias);
    screen->driver = 0;
}

static void
nvidiaCardFini (KdCardInfo *card)
{
    NvidiaCardInfo	*nvidiac = card->driver;

    nvidiaUnmapReg (card, nvidiac);
    vesaCardFini (card);
}

#define nvidiaCursorInit 0       /* initCursor */
#define nvidiaCursorEnable 0    /* enableCursor */
#define nvidiaCursorDisable 0   /* disableCursor */
#define nvidiaCursorFini 0       /* finiCursor */
#define nvidiaRecolorCursor 0   /* recolorCursor */

KdCardFuncs	nvidiaFuncs = {
    nvidiaCardInit,	    /* cardinit */
    nvidiaScreenInit,	    /* scrinit */
    nvidiaInitScreen,	    /* initScreen */
    nvidiaFinishInitScreen, /* finishInitScreen */
    vesaCreateResources,    /* createRes */
    nvidiaPreserve,	    /* preserve */
    nvidiaEnable,	    /* enable */
    nvidiaDPMS,		    /* dpms */
    nvidiaDisable,	    /* disable */
    nvidiaRestore,	    /* restore */
    nvidiaScreenFini,	    /* scrfini */
    nvidiaCardFini,	    /* cardfini */
    
    nvidiaCursorInit,	    /* initCursor */
    nvidiaCursorEnable,	    /* enableCursor */
    nvidiaCursorDisable,    /* disableCursor */
    nvidiaCursorFini,	    /* finiCursor */
    nvidiaRecolorCursor,    /* recolorCursor */
    
    nvidiaDrawInit,	    /* initAccel */
    nvidiaDrawEnable,	    /* enableAccel */
    nvidiaDrawDisable,	    /* disableAccel */
    nvidiaDrawFini,	    /* finiAccel */
    
    vesaGetColors,    	    /* getColors */
    vesaPutColors,	    /* putColors */
};
