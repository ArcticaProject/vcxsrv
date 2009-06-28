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
#include "r128.h"

static Bool
r128CardInit (KdCardInfo *card)
{
    R128CardInfo *r128c;

    r128c = (R128CardInfo *) xalloc (sizeof (R128CardInfo));
    if (!r128c)
	return FALSE;

    r128MapReg (card, r128c);

    if (!vesaInitialize (card, &r128c->vesa))
    {
	xfree (r128c);
	return FALSE;
    }

    r128c->fifo_size = 0;
    
    card->driver = r128c;

    return TRUE;
}

static Bool
r128ScreenInit (KdScreenInfo *screen)
{
    R128ScreenInfo *r128s;
    int screen_size, memory;

    r128s = (R128ScreenInfo *) xalloc (sizeof (R128ScreenInfo));
    if (!r128s)
	return FALSE;
    memset (r128s, '\0', sizeof (R128ScreenInfo));
    if (!vesaScreenInitialize (screen, &r128s->vesa))
    {
	xfree (r128s);
	return FALSE;
    }
#if 0    
    /* if (!r128c->reg)
       screen->dumb = TRUE; */

    if (r128s->vesa.mapping != VESA_LINEAR)
	screen->dumb = TRUE;

    fprintf (stderr, "vesa mapping is %d\n", r128s->vesa.mapping);
#endif    
    r128s->screen = r128s->vesa.fb;

    memory = r128s->vesa.fb_size;
    screen_size = screen->fb[0].byteStride * screen->height;

    memory -= screen_size;
    if (memory > screen->fb[0].byteStride)
    {
	r128s->off_screen = r128s->screen + screen_size;
	r128s->off_screen_size = memory;
    }
    else
    {
	r128s->off_screen = 0;
	r128s->off_screen_size = 0;
    }
    screen->driver = r128s;
    return TRUE;
}

static Bool
r128InitScreen (ScreenPtr pScreen)
{
    return vesaInitScreen (pScreen);
}

static Bool
r128FinishInitScreen (ScreenPtr pScreen)
{
    Bool ret;

    ret = vesaFinishInitScreen (pScreen);

    return ret;
}

static void
r128Preserve (KdCardInfo *card)
{
    vesaPreserve (card);
}

Bool
r128MapReg (KdCardInfo *card, R128CardInfo *r128c)
{
    r128c->reg_base = (CARD8 *) KdMapDevice (R128_REG_BASE (card),
					     R128_REG_SIZE (card));

    if (!r128c->reg_base)
    {
	return FALSE;
    }

    KdSetMappedMode (R128_REG_BASE (card),
		     R128_REG_SIZE (card),
		     KD_MAPPED_MODE_REGISTERS);

    return TRUE;
}

void
r128UnmapReg (KdCardInfo *card, R128CardInfo *r128c)
{
    if (r128c->reg_base)
    {
	KdResetMappedMode (R128_REG_BASE (card),
			   R128_REG_SIZE (card),
			   KD_MAPPED_MODE_REGISTERS);
	KdUnmapDevice ((void *) r128c->reg_base, R128_REG_SIZE (card));
	r128c->reg_base = 0;
    }
}

void
r128SetMMIO (KdCardInfo *card, R128CardInfo *r128c)
{
    if (!r128c->reg_base)
	r128MapReg (card, r128c);
}

void
r128ResetMMIO (KdCardInfo *card, R128CardInfo *r128c)
{
    r128UnmapReg (card, r128c);
}


static Bool
r128DPMS (ScreenPtr pScreen, int mode)
{
    /* XXX */
    return TRUE;
}

static Bool
r128Enable (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    R128CardInfo *r128c = pScreenPriv->card->driver;

    if (!vesaEnable (pScreen))
	return FALSE;
    
    r128SetMMIO (pScreenPriv->card, r128c);
    r128DPMS (pScreen, KD_DPMS_NORMAL);
    
    return TRUE;
}

static void
r128Disable (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    R128CardInfo *r128c = pScreenPriv->card->driver;

    r128ResetMMIO (pScreenPriv->card, r128c);
    vesaDisable (pScreen);
}

static void
r128Restore (KdCardInfo *card)
{
    R128CardInfo *r128c = card->driver;
    
    r128ResetMMIO (card, r128c);
    vesaRestore (card);
}

static void
r128ScreenFini (KdScreenInfo *screen)
{
    R128ScreenInfo *r128s = (R128ScreenInfo *) screen->driver;

    vesaScreenFini (screen);
    xfree (r128s);
    screen->driver = 0;
}

static void
r128CardFini (KdCardInfo *card)
{
    R128CardInfo *r128c = (R128CardInfo *)card->driver;

    r128UnmapReg (card, r128c);
    vesaCardFini (card);
}

KdCardFuncs r128Funcs = {
    r128CardInit,	/* cardinit */
    r128ScreenInit,	/* scrinit */
    r128InitScreen,	/* initScreen */
    r128FinishInitScreen, /* finishInitScreen */
    vesaCreateResources,/* createRes */
    r128Preserve,	/* preserve */
    r128Enable,		/* enable */
    r128DPMS,		/* dpms */
    r128Disable,	/* disable */
    r128Restore,	/* restore */
    r128ScreenFini,	/* scrfini */
    r128CardFini,	/* cardfini */
    
    0,			/* initCursor */
    0,			/* enableCursor */
    0,			/* disableCursor */
    0,			/* finiCursor */
    0,			/* recolorCursor */
    
    r128DrawInit,	/* initAccel */
    r128DrawEnable,	/* enableAccel */
    r128DrawDisable,	/* disableAccel */
    r128DrawFini,	/* finiAccel */
    
    vesaGetColors,    	 /* getColors */
    vesaPutColors,	 /* putColors */
};

