/*
 * Copyright © 2003 Anders Carlsson
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Anders Carlsson not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Anders Carlsson makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ANDERS CARLSSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ANDERS CARLSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "mga.h"

static Bool
mgaCardInit (KdCardInfo *card)
{
    MgaCardInfo *mgac;

    mgac = (MgaCardInfo *) xalloc (sizeof (MgaCardInfo));
    if (!mgac)
	return FALSE;

    mgaMapReg (card, mgac);

    if (!vesaInitialize (card, &mgac->vesa))
    {
	xfree (mgac);
	return FALSE;
    }

    mgac->fifo_size = 0;
    
    card->driver = mgac;

    return TRUE;
}

static Bool
mgaScreenInit (KdScreenInfo *screen)
{
    MgaScreenInfo *mgas;

    mgas = (MgaScreenInfo *) xalloc (sizeof (MgaScreenInfo));
    if (!mgas)
	return FALSE;
    memset (mgas, '\0', sizeof (MgaScreenInfo));
    if (!vesaScreenInitialize (screen, &mgas->vesa))
    {
	xfree (mgas);
	return FALSE;
    }
#if 0    
    /* if (!mgac->reg)
       screen->dumb = TRUE; */

    if (mgas->vesa.mapping != VESA_LINEAR)
	screen->dumb = TRUE;

    fprintf (stderr, "vesa mapping is %d\n", mgas->vesa.mapping);
#endif
    screen->driver = mgas;
    return TRUE;
}

static Bool
mgaInitScreen (ScreenPtr pScreen)
{
    return vesaInitScreen (pScreen);
}

static Bool
mgaFinishInitScreen (ScreenPtr pScreen)
{
    Bool ret;

    ret = vesaFinishInitScreen (pScreen);

    return ret;
}

static Bool
mgaCreateResources (ScreenPtr pScreen)
{
    return vesaCreateResources (pScreen);
}

static void
mgaPreserve (KdCardInfo *card)
{
    vesaPreserve (card);
}

Bool
mgaMapReg (KdCardInfo *card, MgaCardInfo *mgac)
{
    mgac->reg_base = (CARD8 *) KdMapDevice (MGA_REG_BASE (card),
					    MGA_REG_SIZE (card));

    if (!mgac->reg_base)
    {
	return FALSE;
    }

    KdSetMappedMode (MGA_REG_BASE (card),
		     MGA_REG_SIZE (card),
		     KD_MAPPED_MODE_REGISTERS);

    return TRUE;
}

void
mgaUnmapReg (KdCardInfo *card, MgaCardInfo *mgac)
{
    if (mgac->reg_base)
    {
	KdResetMappedMode (MGA_REG_BASE (card),
			   MGA_REG_SIZE (card),
			   KD_MAPPED_MODE_REGISTERS);
	KdUnmapDevice ((void *) mgac->reg_base, MGA_REG_SIZE (card));
	mgac->reg_base = 0;
	/* mgac->reg = 0; */
    }
}

void
mgaSetMMIO (KdCardInfo *card, MgaCardInfo *mgac)
{
    if (!mgac->reg_base)
	mgaMapReg (card, mgac);
}

void
mgaResetMMIO (KdCardInfo *card, MgaCardInfo *mgac)
{
    mgaUnmapReg (card, mgac);
}

static Bool
mgaDPMS (ScreenPtr pScreen, int mode)
{
    /* XXX */
    return TRUE;
}

static Bool
mgaEnable (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    MgaCardInfo *mgac = pScreenPriv->card->driver;

    if (!vesaEnable (pScreen))
	return FALSE;
    
    mgaSetMMIO (pScreenPriv->card, mgac);
    mgaDPMS (pScreen, KD_DPMS_NORMAL);
    
    return TRUE;
}

static void
mgaDisable (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    MgaCardInfo *mgac = pScreenPriv->card->driver;

    mgaResetMMIO (pScreenPriv->card, mgac);

    vesaDisable (pScreen);
}

static void
mgaRestore (KdCardInfo *card)
{
    MgaCardInfo *mgac = card->driver;
    
    mgaResetMMIO (card, mgac);
    vesaRestore (card);
}

static void
mgaScreenFini (KdScreenInfo *screen)
{
    MgaScreenInfo *mgas = (MgaScreenInfo *) screen->driver;

    vesaScreenFini (screen);
    xfree (mgas);
    screen->driver = 0;
}

static void
mgaCardFini (KdCardInfo *card)
{
    MgaCardInfo *mgac = (MgaCardInfo *)card->driver;

    mgaUnmapReg (card, mgac);
    vesaCardFini (card);
}

KdCardFuncs mgaFuncs = {
    mgaCardInit,	/* cardinit */
    mgaScreenInit,	/* scrinit */
    mgaInitScreen,	/* initScreen */
    mgaFinishInitScreen, /* finishInitScreen */
    mgaCreateResources,	/* createRes */
    mgaPreserve,	/* preserve */
    mgaEnable,		/* enable */
    mgaDPMS,		/* dpms */
    mgaDisable,		/* disable */
    mgaRestore,		/* restore */
    mgaScreenFini,	/* scrfini */
    mgaCardFini,	/* cardfini */
    
    0,			/* initCursor */
    0,			/* enableCursor */
    0,			/* disableCursor */
    0,			/* finiCursor */
    0,			/* recolorCursor */
    
    mgaDrawInit,	/* initAccel */
    mgaDrawEnable,	/* enableAccel */
    mgaDrawDisable,	/* disableAccel */
    mgaDrawFini,	/* finiAccel */
    
    vesaGetColors,    	 /* getColors */
    vesaPutColors,	 /* putColors */
};

