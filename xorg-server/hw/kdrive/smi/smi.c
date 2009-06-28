/*
 * Copyright © 2001 Keith Packard
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
#include "smi.h"
#include "kaa.h"
#include <sys/io.h>

static Bool
smiCardInit (KdCardInfo *card)
{
    SmiCardInfo	*smic;

    ENTER ();
    smic = (SmiCardInfo *) xalloc (sizeof (SmiCardInfo));
    if (!smic)
	return FALSE;
    memset (smic, '\0', sizeof (SmiCardInfo));
    
    (void) smiMapReg (card, smic);

    if (!subInitialize (card, &smic->sub))
    {
	xfree (smic);
	return FALSE;
    }

    card->driver = smic;
    LEAVE();
    return TRUE;
}

static Bool
smiScreenInit (KdScreenInfo *screen)
{
    SmiCardInfo		*smic = screen->card->driver;
    SmiScreenInfo	*smis;

    ENTER();
    smis = (SmiScreenInfo *) xalloc (sizeof (SmiScreenInfo));
    if (!smis)
	return FALSE;
    memset (smis, '\0', sizeof (SmiScreenInfo));
    screen->driver = smis;
    if (!subScreenInitialize (screen, &smis->sub))
    {
	xfree (smis);
	return FALSE;
    }
    if (!smic->reg_base)
	screen->dumb = TRUE;
    screen->softCursor = TRUE;
#if SMI_VESA
    smis->screen = smis->sub.fb;
#else
    smis->screen = smic->sub.fb;
#endif
    LEAVE();
    return TRUE;
}

static Bool
smiInitScreen (ScreenPtr pScreen)
{
    Bool    ret;
    ENTER ();
#if 0
#ifdef XV
    KdScreenPriv(pScreen);
    SmiCardInfo	*smic = pScreenPriv->screen->card->driver;
    if (smic->media_reg && smic->reg)
	smiInitVideo(pScreen);
#endif
#endif
    ret = subInitScreen (pScreen);
    LEAVE();
    return ret;
}

#ifdef RANDR
static Bool
smiRandRSetConfig (ScreenPtr		pScreen,
		   Rotation		randr,
		   int			rate,
		   RRScreenSizePtr	pSize)
{
    Bool    ret;
    
    ENTER ();
    kaaWaitSync (pScreen);

    ret = subRandRSetConfig (pScreen, randr, rate, pSize);
    LEAVE();
    return ret;
}

static Bool
smiRandRInit (ScreenPtr pScreen)
{
    rrScrPriv(pScreen);

    ENTER ();
    pScrPriv->rrSetConfig = smiRandRSetConfig;
    LEAVE ();
    return TRUE;
}
#endif

static Bool
smiFinishInitScreen (ScreenPtr pScreen)
{
    Bool    ret;
    ret = subFinishInitScreen (pScreen);
#ifdef RANDR
    smiRandRInit (pScreen);
#endif
    return ret;
}

void
smiPreserve (KdCardInfo *card)
{
    ENTER ();
    subPreserve(card);
    LEAVE();
}

Bool
smiMapReg (KdCardInfo *card, SmiCardInfo *smic)
{
    ENTER ();
    smic->io_base = 0;	/* only handles one SMI card at standard VGA address */
    smic->reg_base = (CARD8 *) KdMapDevice (SMI_REG_BASE(card),
					    SMI_REG_SIZE(card));
    
    if (!smic->reg_base)
    {
	smic->dpr = 0;
	return FALSE;
    }
    
    KdSetMappedMode (SMI_REG_BASE(card),
		     SMI_REG_SIZE(card),
		     KD_MAPPED_MODE_REGISTERS);
    smic->dpr = (DPR *) (smic->reg_base + SMI_DPR_OFF(card));
    LEAVE ();
    return TRUE;
}

void
smiUnmapReg (KdCardInfo *card, SmiCardInfo *smic)
{
    ENTER ();
    if (smic->reg_base)
    {
	KdResetMappedMode (SMI_REG_BASE(card),
			   SMI_REG_SIZE(card),
			   KD_MAPPED_MODE_REGISTERS);
	KdUnmapDevice ((void *) smic->reg_base, SMI_REG_SIZE(card));
	smic->reg_base = 0;
	smic->dpr = 0;
    }
    LEAVE ();
}

void
smiOutb (CARD16 port, CARD8 val)
{
    asm volatile ("outb %b0,%w1" : : "a" (val), "d" (port));
}

CARD8
smiInb (CARD16 port)
{
    CARD8   v;
    asm volatile ("inb %w1,%b0" : "=a" (v) : "d" (port));
    return v;
}

CARD8
smiGetIndex (SmiCardInfo *smic, CARD16 addr, CARD16 data, CARD8 id)
{
    smiOutb (smic->io_base + addr, id);
    return smiInb (smic->io_base + data);
}

void
smiSetIndex (SmiCardInfo *smic, CARD16 addr, CARD16 data, CARD8 id, CARD8 val)
{
    smiOutb (smic->io_base + addr, id);
    smiOutb (smic->io_base + data, val);
}

void
smiSetMMIO (KdCardInfo *card, SmiCardInfo *smic)
{
    ENTER ();
    if (!smic->reg_base)
	smiMapReg (card, smic);
    LEAVE();
}

void
smiResetMMIO (KdCardInfo *card, SmiCardInfo *smic)
{
    smiUnmapReg (card, smic);
}

static Bool
smiDPMS (ScreenPtr pScreen, int mode)
{
    Bool    ret;
    ENTER ();
    ret = subDPMS (pScreen, mode);
    LEAVE ();
    return ret;
}

Bool
smiEnable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    SmiCardInfo	*smic = pScreenPriv->card->driver;

    ENTER ();
    iopl (3);
    if (!subEnable (pScreen))
	return FALSE;
    
    smiSetMMIO (pScreenPriv->card, smic);
    smiDPMS (pScreen, KD_DPMS_NORMAL);
#if 0
#ifdef XV
    KdXVEnable (pScreen);
#endif
#endif
    LEAVE ();
    return TRUE;
}

void
smiDisable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    SmiCardInfo	*smic = pScreenPriv->card->driver;

    ENTER ();
#if 0
#ifdef XV
    KdXVDisable (pScreen);
#endif
#endif
    smiResetMMIO (pScreenPriv->card, smic);
    subDisable (pScreen);
    LEAVE ();
}

static void
smiRestore (KdCardInfo *card)
{
    ENTER ();
    subRestore (card);
    LEAVE();
}

static void
smiScreenFini (KdScreenInfo *screen)
{
    SmiScreenInfo	*smis = (SmiScreenInfo *) screen->driver;

    ENTER ();
    subScreenFini (screen);
    xfree (smis);
    screen->driver = 0;
    LEAVE ();
}

static void
smiCardFini (KdCardInfo *card)
{
    SmiCardInfo	*smic = card->driver;

    ENTER ();
    smiUnmapReg (card, smic);
    subCardFini (card);
    LEAVE ();
}

#define smiCursorInit 0       /* initCursor */
#define smiCursorEnable 0    /* enableCursor */
#define smiCursorDisable 0   /* disableCursor */
#define smiCursorFini 0       /* finiCursor */
#define smiRecolorCursor 0   /* recolorCursor */

KdCardFuncs	smiFuncs = {
    smiCardInit,	    /* cardinit */
    smiScreenInit,	    /* scrinit */
    smiInitScreen,	    /* initScreen */
    smiFinishInitScreen,    /* finishInitScreen */
    subCreateResources,	    /* createRes */
    smiPreserve,	    /* preserve */
    smiEnable,		    /* enable */
    smiDPMS,		    /* dpms */
    smiDisable,		    /* disable */
    smiRestore,		    /* restore */
    smiScreenFini,	    /* scrfini */
    smiCardFini,	    /* cardfini */
    
    smiCursorInit,	    /* initCursor */
    smiCursorEnable,	    /* enableCursor */
    smiCursorDisable,	    /* disableCursor */
    smiCursorFini,	    /* finiCursor */
    smiRecolorCursor,	    /* recolorCursor */
    
    smiDrawInit,	    /* initAccel */
    smiDrawEnable,	    /* enableAccel */
    smiDrawDisable,	    /* disableAccel */
    smiDrawFini,	    /* finiAccel */
    
    subGetColors,    	    /* getColors */
    subPutColors,	    /* putColors */
};
