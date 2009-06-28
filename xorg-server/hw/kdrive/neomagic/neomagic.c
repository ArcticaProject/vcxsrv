/*
 *
 * Copyright © 2004 Franco Catrin
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Franco Catrin not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Franco Catrin makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * FRANCO CATRIN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL FRANCO CATRIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "neomagic.h"
#include <sys/io.h>

struct NeoChipInfo neoChips[] = {
    {NEO_VENDOR, 0x0001, CAP_NM2070, "MagicGraph 128(NM2070)",
     896, 65000, 2048, 0x100, 1024, 1024, 1024},
    {NEO_VENDOR, 0x0002, CAP_NM2090, "MagicGraph 128V(NM2090)",
     1152, 80000, 2048, 0x100, 2048, 1024, 1024},
    {NEO_VENDOR, 0x0003, CAP_NM2090, "MagicGraph 128ZV(NM2093)",
     1152, 80000, 2048, 0x100, 2048, 1024, 1024},
    {NEO_VENDOR, 0x0083, CAP_NM2097, "MagicGraph 128ZV+(NM2097)",
     1152, 80000, 1024, 0x100, 2048, 1024, 1024},
    {NEO_VENDOR, 0x0004, CAP_NM2097, "MagicGraph 128XD(NM2160)",
     2048, 90000, 1024, 0x100, 2048, 1024, 1024},
    {NEO_VENDOR, 0x0005, CAP_NM2200, "MagicGraph 256AV(NM2200)",
     2560, 110000, 1024, 0x1000, 4096, 1280, 1024},
    {NEO_VENDOR, 0x0025, CAP_NM2200, "MagicGraph 256AV+(NM2230)",
     3008, 110000, 1024, 0x1000, 4096, 1280, 1024},
    {NEO_VENDOR, 0x0006, CAP_NM2200, "MagicGraph 256ZX(NM2360)",
     4096, 110000, 1024, 0x1000, 4096, 1280, 1024},
    {NEO_VENDOR, 0x0016, CAP_NM2200, "MagicGraph 256XL+(NM2380)",
     6144, 110000, 1024, 0x1000, 8192, 1280, 1024},
    {0, 0, 0, NULL},
};

static Bool
neoCardInit(KdCardInfo *card)
{
    NeoCardInfo    *neoc;
    struct NeoChipInfo *chip;

    neoc =(NeoCardInfo *) xalloc(sizeof(NeoCardInfo));
    if(!neoc) {
        return FALSE;
    }

    if(!vesaInitialize(card, &neoc->backendCard)) {
        xfree(neoc);
        return FALSE;
    }

    for(chip = neoChips; chip->name != NULL; ++chip) {
        if(chip->device == card->attr.deviceID) {
            neoc->chip = chip;
            break;
        }
    }

    ErrorF("Using Neomagic card: %s\n", neoc->chip->name);

    neoMapReg(card, neoc);

    card->driver = neoc;

    return TRUE;
}

static Bool
neoScreenInit(KdScreenInfo *screen)
{
    NeoScreenInfo *neos;
    int screen_size, memory;

    neos = xcalloc(sizeof(NeoScreenInfo), 1);
    if(neos == NULL) {
        return FALSE;
    }

	memset (neos, '\0', sizeof (NeoScreenInfo));


    if(!vesaScreenInitialize(screen, &neos->backendScreen)) {
        xfree(neos);
        return FALSE;
    }

    screen->softCursor = TRUE;    // no hardware color cursor available

	neos->screen = neos->backendScreen.fb;

	memory = neos->backendScreen.fb_size;
    screen_size = screen->fb[0].byteStride * screen->height;
    memory -= screen_size;

    if(memory > screen->fb[0].byteStride) {
        neos->off_screen = neos->screen + screen_size;
        neos->off_screen_size = memory;
    } else {
        neos->off_screen = 0;
        neos->off_screen_size = 0;
    }

    screen->driver = neos;

    return TRUE;
}

static Bool
neoInitScreen(ScreenPtr pScreen)
{
    return vesaInitScreen(pScreen);
}

static Bool
neoFinishInitScreen(ScreenPtr pScreen)
{
    return vesaFinishInitScreen(pScreen);
}

static Bool
neoCreateResources(ScreenPtr pScreen)
{
    return vesaCreateResources(pScreen);
}

void
neoPreserve(KdCardInfo *card)
{
	vesaPreserve(card);
}

CARD8
neoGetIndex(NeoCardInfo *nvidiac, CARD16 addr,  CARD8 index)
{
    outb(index, addr);
    
    return inb(addr+1);
}

void
neoSetIndex(NeoCardInfo *nvidiac, CARD16 addr,  CARD8 index, CARD8 val)
{
    outb(index, addr);
    outb(val, addr+1);
}

static void neoLock(NeoCardInfo *neoc){
    CARD8 cr11;
    neoSetIndex(neoc, 0x3ce,  0x09, 0x00);
    neoSetIndex(neoc, 0x3ce,  0x11, 0x0); // disable MMIO and linear mode
    cr11 = neoGetIndex(neoc, 0x3d4, 0x11);
    neoSetIndex(neoc, 0x3d4, 0x11, cr11 | 0x80);
}

static void neoUnlock(NeoCardInfo *neoc){
    CARD8 cr11;
    cr11 = neoGetIndex(neoc, 0x3d4, 0x11);
    neoSetIndex(neoc, 0x3d4, 0x11, cr11 & 0x7F);
    neoSetIndex(neoc, 0x3ce,  0x09, 0x26);
    neoSetIndex(neoc, 0x3ce,  0x11, 0xc0); // enable MMIO and linear mode
}


Bool
neoMapReg(KdCardInfo *card, NeoCardInfo *neoc)
{
    neoc->reg_base = card->attr.address[1] & 0xFFF80000;
    if(!neoc->reg_base) {
        return FALSE;
    }

    neoc->mmio = KdMapDevice(neoc->reg_base, NEO_REG_SIZE(card));
    if(!neoc->mmio) {
        return FALSE;
    }

    KdSetMappedMode(neoc->reg_base, NEO_REG_SIZE(card), KD_MAPPED_MODE_REGISTERS);

    return TRUE;
}

void
neoUnmapReg(KdCardInfo *card, NeoCardInfo *neoc)
{
    if(neoc->reg_base)
    {
        neoSetIndex(neoc, 0x3ce, 0x82,0);
        KdResetMappedMode(neoc->reg_base, NEO_REG_SIZE(card), KD_MAPPED_MODE_REGISTERS);
        KdUnmapDevice((void *)neoc->mmio, NEO_REG_SIZE(card));
        neoc->reg_base = 0;
    }
}

static void
neoSetMMIO(KdCardInfo *card, NeoCardInfo *neoc)
{
    if(!neoc->reg_base)
        neoMapReg(card, neoc);
        neoUnlock(neoc);
}

static void
neoResetMMIO(KdCardInfo *card, NeoCardInfo *neoc)
{
    neoUnmapReg(card, neoc);
    neoLock(neoc);
}


Bool
neoEnable(ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    neoCardInfo(pScreenPriv);

    if(!vesaEnable(pScreen)) {
        return FALSE;
    }

    neoSetMMIO(pScreenPriv->card, neoc);
    return TRUE;
}

void
neoDisable(ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    neoCardInfo(pScreenPriv);

    neoResetMMIO(pScreenPriv->card, neoc);

    vesaDisable(pScreen);
}

static void
neoGetColors(ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
    vesaGetColors(pScreen, fb, n, pdefs);
}

static void
neoPutColors(ScreenPtr pScreen, int fb, int n, xColorItem *pdefs)
{
    vesaPutColors(pScreen, fb, n, pdefs);
}

static Bool
neoDPMS(ScreenPtr pScreen, int mode)
{
    return vesaDPMS(pScreen, mode);
}

static void
neoRestore(KdCardInfo *card)
{
    NeoCardInfo *neoc = card->driver;

    neoResetMMIO(card, neoc);
    vesaRestore(card);
}

static void
neoScreenFini(KdScreenInfo *screen)
{
    NeoScreenInfo *neos =(NeoScreenInfo *) screen->driver;

    vesaScreenFini(screen);
    xfree(neos);
    screen->driver = 0;
}

static void
neoCardFini(KdCardInfo *card)
{
    NeoCardInfo *neoc = card->driver;

	neoUnmapReg(card, neoc);
	vesaCardFini(card);
}

#define neoCursorInit 0       // initCursor
#define neoCursorEnable 0     // enableCursor
#define neoCursorDisable 0    // disableCursor
#define neoCursorFini 0       // finiCursor */
#define neoRecolorCursor 0    // recolorCursor */
//#define     neoDrawInit 0              // initAccel
//#define     neoDrawEnable 0            // enableAccel
//#define     neoDrawSync 0          // syncAccel
//#define     neoDrawDisable 0          // disableAccel
//#define     neoDrawFini 0             // finiAccel

KdCardFuncs    neoFuncs = {
    neoCardInit,              // cardinit
    neoScreenInit,            // scrinit
    neoInitScreen,            // initScreen
    neoFinishInitScreen,      // finishInitScreen
    neoCreateResources,       // createRes
    neoPreserve,              // preserve
    neoEnable,                // enable
    neoDPMS,                  // dpms
    neoDisable,               // disable
    neoRestore,               // restore
    neoScreenFini,            // scrfini
    neoCardFini,              // cardfini

    neoCursorInit,            // initCursor
    neoCursorEnable,          // enableCursor
    neoCursorDisable,         // disableCursor
    neoCursorFini,            // finiCursor
    neoRecolorCursor,         // recolorCursor

    neoDrawInit,              // initAccel
    neoDrawEnable,            // enableAccel
    neoDrawDisable,           // disableAccel
    neoDrawFini,              // finiAccel

    neoGetColors,             // getColors
    neoPutColors,             // putColors
};
