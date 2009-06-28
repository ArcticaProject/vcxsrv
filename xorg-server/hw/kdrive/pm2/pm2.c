#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "kaa.h"

#include "pm2.h"

#define PARTPROD(a,b,c) (((a)<<6) | ((b)<<3) | (c))

char bppand[4] = { 0x03, /* 8bpp */
		   0x01, /* 16bpp */
		   0x00, /* 24bpp */
		   0x00  /* 32bpp */};

int partprodPermedia[] = {
	-1,
	PARTPROD(0,0,1), PARTPROD(0,1,1), PARTPROD(1,1,1), PARTPROD(1,1,2),
	PARTPROD(1,2,2), PARTPROD(2,2,2), PARTPROD(1,2,3), PARTPROD(2,2,3),
	PARTPROD(1,3,3), PARTPROD(2,3,3), PARTPROD(1,2,4), PARTPROD(3,3,3),
	PARTPROD(1,3,4), PARTPROD(2,3,4),              -1, PARTPROD(3,3,4), 
	PARTPROD(1,4,4), PARTPROD(2,4,4),              -1, PARTPROD(3,4,4), 
	             -1, PARTPROD(2,3,5),              -1, PARTPROD(4,4,4), 
	PARTPROD(1,4,5), PARTPROD(2,4,5), PARTPROD(3,4,5),              -1,
	             -1,              -1,              -1, PARTPROD(4,4,5), 
	PARTPROD(1,5,5), PARTPROD(2,5,5),              -1, PARTPROD(3,5,5), 
	             -1,              -1,              -1, PARTPROD(4,5,5), 
	             -1,              -1,              -1, PARTPROD(3,4,6),
	             -1,              -1,              -1, PARTPROD(5,5,5), 
	PARTPROD(1,5,6), PARTPROD(2,5,6),              -1, PARTPROD(3,5,6),
	             -1,              -1,              -1, PARTPROD(4,5,6),
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1, PARTPROD(5,5,6),
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
	             -1,              -1,              -1,              -1,
		     0};

static Bool
pmMapReg(KdCardInfo *card, PM2CardInfo *pm2c)
{
	pm2c->reg_base = (CARD8 *)KdMapDevice(PM2_REG_BASE(card),
	    PM2_REG_SIZE(card));

	if (pm2c->reg_base == NULL)
		return FALSE;

	KdSetMappedMode(PM2_REG_BASE(card), PM2_REG_SIZE(card),
	    KD_MAPPED_MODE_REGISTERS);

	return TRUE;
}

static void
pmUnmapReg(KdCardInfo *card, PM2CardInfo *pm2c)
{
	if (pm2c->reg_base) {
		KdResetMappedMode(PM2_REG_BASE(card), PM2_REG_SIZE(card),
		    KD_MAPPED_MODE_REGISTERS);
		KdUnmapDevice((void *)pm2c->reg_base, PM2_REG_SIZE(card));
		pm2c->reg_base = 0;
	}
}

Bool
pmCardInit (KdCardInfo *card)
{
    PM2CardInfo	*pm2c;

    pm2c = (PM2CardInfo *) xalloc (sizeof (PM2CardInfo));
    if (!pm2c)
	return FALSE;
    memset (pm2c, '\0', sizeof (PM2CardInfo));

    (void) pmMapReg (card, pm2c);

    if (!vesaInitialize (card, &pm2c->vesa))
    {
	xfree (pm2c);
	return FALSE;
    }
    
    pm2c->InFifoSpace = 0;

    card->driver = pm2c;    

    return TRUE;
}

static  void
pmCardFini (KdCardInfo *card)
{
    PM2CardInfo	*pm2c = (PM2CardInfo *) card->driver;
    
    pmUnmapReg (card, pm2c);
    vesaCardFini (card);
}

Bool
pmScreenInit (KdScreenInfo *screen)
{
    PM2CardInfo	*pm2c = screen->card->driver;
    PM2ScreenInfo	*pm2s;
    int			screen_size, memory;

    pm2s = (PM2ScreenInfo *) xalloc (sizeof (PM2ScreenInfo));
    if (!pm2s)
	return FALSE;
    memset (pm2s, '\0', sizeof (PM2ScreenInfo));

    if (!vesaScreenInitialize (screen, &pm2s->vesa))
    {
	xfree (pm2s);
	return FALSE;
    }

    pm2c->pprod = partprodPermedia[screen->width >> 5];
    pm2c->bppalign = bppand[(screen->fb[0].bitsPerPixel>>3)-1];

    pm2s->screen = pm2s->vesa.fb;
    memory = pm2s->vesa.fb_size;
    
    screen_size = screen->fb[0].byteStride * screen->height;
    
    if (pm2s->screen && memory >= screen_size + 2048)
    {
	memory -= 2048;
	pm2s->cursor_base = pm2s->screen + memory - 2048;
    }
    else
	pm2s->cursor_base = 0;
    memory -= screen_size;
    if (memory > screen->fb[0].byteStride)
    {
	pm2s->off_screen = pm2s->screen + screen_size;
	pm2s->off_screen_size = memory;
    }
    else
    {
	pm2s->off_screen = 0;
	pm2s->off_screen_size = 0;
    }

    switch (screen->fb[0].bitsPerPixel) {
    case 8:
	pm2c->BppShift = 2;
	break;
    case 16:
	pm2c->BppShift = 1;
	break;
    case 24:
	pm2c->BppShift = 2;
	break;
    case 32:
	pm2c->BppShift = 0;
	break;
    }

    screen->driver = pm2s;

    return TRUE;
}

static void
pmScreenFini (KdScreenInfo *screen)
{
    PM2ScreenInfo	*pm2s = (PM2ScreenInfo *) screen->driver;

    vesaScreenFini (screen);
    xfree (pm2s);
    screen->driver = 0;
}

static Bool
pmInitScreen (ScreenPtr pScreen)
{
    return vesaInitScreen (pScreen);
}

#ifdef RANDR
static Bool
pmRandRSetConfig (ScreenPtr		pScreen,
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
pmRandRInit (ScreenPtr pScreen)
{
    rrScrPriv(pScreen);

    pScrPriv->rrSetConfig = pmRandRSetConfig;
}
#endif

static Bool
pmFinishInitScreen (ScreenPtr pScreen)
{
    Bool    ret;
    ret = vesaFinishInitScreen (pScreen);
#ifdef RANDR
    pmRandRInit (pScreen);
#endif
    return ret;
}

static void
pmPreserve(KdCardInfo *card)
{
    vesaPreserve(card);
}

static void
pmRestore(KdCardInfo *card)
{
    vesaRestore (card);
}

static Bool
pmEnable (ScreenPtr pScreen)
{
    if (!vesaEnable (pScreen))
	return FALSE;
    
#ifdef XV
    KdXVEnable (pScreen);
#endif

    return TRUE;
}

static void
pmDisable(ScreenPtr pScreen)
{
#ifdef XV
    KdXVDisable (pScreen);
#endif
    vesaDisable (pScreen);
}

static Bool
pmDPMS(ScreenPtr pScreen, int mode) 
{
    return vesaDPMS (pScreen, mode);
}

KdCardFuncs	PM2Funcs = {
    pmCardInit,               /* cardinit */
    pmScreenInit,             /* scrinit */
    pmInitScreen,             /* initScreen */
    pmFinishInitScreen, /* finishInitScreen */
    vesaCreateResources,    /* createRes */
    pmPreserve,               /* preserve */
    pmEnable,                 /* enable */
    pmDPMS,                   /* dpms */
    pmDisable,                /* disable */
    pmRestore,                /* restore */
    pmScreenFini,             /* scrfini */
    pmCardFini,               /* cardfini */
    
    0,             		/* initCursor */
    0,           		/* enableCursor */
    0,          		/* disableCursor */
    0,             		/* finiCursor */
    NULL,                       /* recolorCursor */

    pmDrawInit,              /* initAccel */
    pmDrawEnable,            /* enableAccel */
    pmDrawDisable,           /* disableAccel */
    pmDrawFini,              /* finiAccel */
    
    vesaGetColors,    	    /* getColors */
    vesaPutColors,	    /* putColors */
};
