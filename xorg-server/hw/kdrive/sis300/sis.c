/*
 * Copyright © 2003 Eric Anholt
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Eric Anholt not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Eric Anholt makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ERIC ANHOLT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ERIC ANHOLT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "sis.h"
#include "sis_reg.h"

struct pci_id_entry sis_pci_ids[] = {
	{0x1039, 0x0300, 0x1, "SiS 300/305"},
	{0x1039, 0x5300, 0x1, "SiS 540"},
	{0x1039, 0x6300, 0x1, "SiS 630"},
	{0x1039, 0x7300, 0x1, "SiS 730"},
	{0, 0, 0, NULL}
};

static Bool
SiSCardInit(KdCardInfo *card)
{
	SiSCardInfo *sisc;
	Bool initialized = FALSE;

	sisc = xcalloc(sizeof(SiSCardInfo), 1);
	if (sisc == NULL)
		return FALSE;

#ifdef KDRIVEFBDEV
	if (!initialized && fbdevInitialize(card, &sisc->backend_priv.fbdev)) {
		sisc->use_fbdev = TRUE;
		initialized = TRUE;
		sisc->backend_funcs.cardfini = fbdevCardFini;
		sisc->backend_funcs.scrfini = fbdevScreenFini;
		sisc->backend_funcs.initScreen = fbdevInitScreen;
		sisc->backend_funcs.finishInitScreen = fbdevFinishInitScreen;
		sisc->backend_funcs.createRes = fbdevCreateResources;
		sisc->backend_funcs.preserve = fbdevPreserve;
		sisc->backend_funcs.restore = fbdevRestore;
		sisc->backend_funcs.dpms = fbdevDPMS;
		sisc->backend_funcs.enable = fbdevEnable;
		sisc->backend_funcs.disable = fbdevDisable;
		sisc->backend_funcs.getColors = fbdevGetColors;
		sisc->backend_funcs.putColors = fbdevPutColors;
	}
#endif
#ifdef KDRIVEVESA
	if (!initialized && vesaInitialize(card, &sisc->backend_priv.vesa)) {
		sisc->use_vesa = TRUE;
		initialized = TRUE;
		sisc->backend_funcs.cardfini = vesaCardFini;
		sisc->backend_funcs.scrfini = vesaScreenFini;
		sisc->backend_funcs.initScreen = vesaInitScreen;
		sisc->backend_funcs.finishInitScreen = vesaFinishInitScreen;
		sisc->backend_funcs.createRes = vesaCreateResources;
		sisc->backend_funcs.preserve = vesaPreserve;
		sisc->backend_funcs.restore = vesaRestore;
		sisc->backend_funcs.dpms = vesaDPMS;
		sisc->backend_funcs.enable = vesaEnable;
		sisc->backend_funcs.disable = vesaDisable;
		sisc->backend_funcs.getColors = vesaGetColors;
		sisc->backend_funcs.putColors = vesaPutColors;
	}
#endif

	if (!initialized || !SiSMapReg(card, sisc)) {
		xfree(sisc);
		return FALSE;
	}

	card->driver = sisc;

	return TRUE;
}

static void
SiSCardFini(KdCardInfo *card)
{
	SiSCardInfo *sisc = (SiSCardInfo *)card->driver;

	SiSUnmapReg(card, sisc);
	sisc->backend_funcs.cardfini(card);
}

static Bool
SiSScreenInit(KdScreenInfo *screen)
{
	SiSScreenInfo *siss;
	SiSCardInfo(screen);
	int success = FALSE;

	siss = xcalloc(sizeof(SiSScreenInfo), 1);
	if (siss == NULL)
		return FALSE;

	siss->sisc = sisc;

	screen->driver = siss;

#ifdef KDRIVEFBDEV
	if (sisc->use_fbdev) {
		success = fbdevScreenInitialize(screen,
		    &siss->backend_priv.fbdev);
		screen->memory_size = sisc->backend_priv.fbdev.fix.smem_len;
		screen->off_screen_base =
		    sisc->backend_priv.fbdev.var.yres_virtual *
		    screen->fb[0].byteStride;
	}
#endif
#ifdef KDRIVEVESA
	if (sisc->use_vesa) {
		if (screen->fb[0].depth == 0)
			screen->fb[0].depth = 16;
		success = vesaScreenInitialize(screen,
		    &siss->backend_priv.vesa);
	}
#endif
	if (!success) {
		screen->driver = NULL;
		xfree(siss);
		return FALSE;
	}

	return TRUE;
}

static void
SiSScreenFini(KdScreenInfo *screen)
{
	SiSScreenInfo *siss = (SiSScreenInfo *)screen->driver;
	SiSCardInfo *sisc = screen->card->driver;

	sisc->backend_funcs.scrfini(screen);
	xfree(siss);
	screen->driver = 0;
}

Bool
SiSMapReg(KdCardInfo *card, SiSCardInfo *sisc)
{
	sisc->reg_base = (CARD8 *)KdMapDevice(SIS_REG_BASE(card),
	    SIS_REG_SIZE(card));

	if (sisc->reg_base == NULL)
		return FALSE;

	KdSetMappedMode(SIS_REG_BASE(card), SIS_REG_SIZE(card),
	    KD_MAPPED_MODE_REGISTERS);

	return TRUE;
}

void
SiSUnmapReg(KdCardInfo *card, SiSCardInfo *sisc)
{
	if (sisc->reg_base) {
		KdResetMappedMode(SIS_REG_BASE(card), SIS_REG_SIZE(card),
		    KD_MAPPED_MODE_REGISTERS);
		KdUnmapDevice((void *)sisc->reg_base, SIS_REG_SIZE(card));
		sisc->reg_base = 0;
	}
}

static Bool
SiSInitScreen(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	SiSCardInfo(pScreenPriv);

	return sisc->backend_funcs.initScreen(pScreen);
}

static Bool
SiSFinishInitScreen(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	SiSCardInfo(pScreenPriv);

	return sisc->backend_funcs.finishInitScreen(pScreen);
}

static Bool
SiSCreateResources(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	SiSCardInfo(pScreenPriv);

	return sisc->backend_funcs.createRes(pScreen);
}

static void
SiSPreserve(KdCardInfo *card)
{
	SiSCardInfo *sisc = card->driver;

	sisc->backend_funcs.preserve(card);
}

static void
SiSRestore(KdCardInfo *card)
{
	SiSCardInfo *sisc = card->driver;

	SiSUnmapReg(card, sisc);

	sisc->backend_funcs.restore(card);
}

static Bool
SiSDPMS(ScreenPtr pScreen, int mode)
{
	KdScreenPriv(pScreen);
	SiSCardInfo(pScreenPriv);

	return sisc->backend_funcs.dpms(pScreen, mode);
}

static Bool
SiSEnable(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	SiSCardInfo(pScreenPriv);

	if (!sisc->backend_funcs.enable(pScreen))
		return FALSE;

	if ((sisc->reg_base == NULL) && !SiSMapReg(pScreenPriv->screen->card,
	    sisc))
		return FALSE;

	SiSDPMS(pScreen, KD_DPMS_NORMAL);

	return TRUE;
}

static void
SiSDisable(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	SiSCardInfo(pScreenPriv);

	SiSUnmapReg(pScreenPriv->card, sisc);

	sisc->backend_funcs.disable(pScreen);
}

static void
SiSGetColors(ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
	KdScreenPriv(pScreen);
	SiSCardInfo(pScreenPriv);

	sisc->backend_funcs.getColors(pScreen, fb, n, pdefs);
}

static void
SiSPutColors(ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
	KdScreenPriv(pScreen);
	SiSCardInfo(pScreenPriv);

	sisc->backend_funcs.putColors(pScreen, fb, n, pdefs);
}

KdCardFuncs SiSFuncs = {
	SiSCardInit,		/* cardinit */
	SiSScreenInit,		/* scrinit */
	SiSInitScreen,		/* initScreen */
	SiSFinishInitScreen,	/* finishInitScreen */
	SiSCreateResources,	/* createRes */
	SiSPreserve,		/* preserve */
	SiSEnable,		/* enable */
	SiSDPMS,		/* dpms */
	SiSDisable,		/* disable */
	SiSRestore,		/* restore */
	SiSScreenFini,		/* scrfini */
	SiSCardFini,		/* cardfini */

	0,			/* initCursor */
	0,			/* enableCursor */
	0,			/* disableCursor */
	0,			/* finiCursor */
	0,			/* recolorCursor */

	SiSDrawInit,		/* initAccel */
	SiSDrawEnable,		/* enableAccel */
	SiSDrawSync,		/* syncAccel */
	SiSDrawDisable,		/* disableAccel */
	SiSDrawFini,		/* finiAccel */

	SiSGetColors,		/* getColors */
	SiSPutColors,		/* putColors */
};
