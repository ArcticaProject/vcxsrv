#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "kaa.h"

#include "pm2.h"

static PM2CardInfo	*card;
static VOL8	*mmio;

static void Permedia2LoadCoord(int x, int y, int w, int h);

static void
pmWaitMarker (ScreenPtr pScreen, int marker)
{
    CHECKCLIPPING;

    while (GLINT_READ_REG(DMACount) != 0);
    GLINT_WAIT(2);
    GLINT_WRITE_REG(0x400, FilterMode);
    GLINT_WRITE_REG(0, GlintSync);
    do {
   	while(GLINT_READ_REG(OutFIFOWords) == 0);
    } while (GLINT_READ_REG(OutputFIFO) != Sync_tag);
}

static Bool
pmPrepareSolid (PixmapPtr   	pPixmap,
		int		rop,
		Pixel		planemask,
		Pixel		color)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    KdScreenPriv(pScreen);
    pmCardInfo(pScreenPriv);

    card = pm2c;
    mmio = pm2c->reg_base;

    if (~planemask & FbFullMask(pPixmap->drawable.depth))
	return FALSE;

    REPLICATE(color);

    GLINT_WAIT(6);
    DO_PLANEMASK(planemask);
    if (rop == GXcopy) {
	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	GLINT_WRITE_REG(card->pprod, FBReadMode);
	GLINT_WRITE_REG(color, FBBlockColor);
    } else {
	GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
      	GLINT_WRITE_REG(color, ConstantColor);
	/* We can use Packed mode for filling solid non-GXcopy rasters */
	GLINT_WRITE_REG(card->pprod|FBRM_DstEnable|FBRM_Packed, FBReadMode);
    }
    LOADROP(rop);

    return TRUE;
}

static void
pmSolid (int x1, int y1, int x2, int y2)
{
    int speed = 0;

    if (card->ROP == GXcopy) {
	GLINT_WAIT(3);
        Permedia2LoadCoord(x1, y1, x2-x1, y2-y1);
  	speed = FastFillEnable;
    } else {
	GLINT_WAIT(4);
        Permedia2LoadCoord(x1>>card->BppShift, y1, 
			    ((x2-x1)+7)>>card->BppShift, y2-y1);
  	GLINT_WRITE_REG(x1<<16|(x1+(x2-x1)), PackedDataLimits);
  	speed = 0;
    }
    GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive | speed, Render);
}

static void
pmDoneSolid (void)
{
}

static Bool
pmPrepareCopy (PixmapPtr	pSrcPixmap,
	       PixmapPtr	pDstPixmap,
	       int		dx,
	       int		dy,
	       int		rop,
	       Pixel		planemask)
{
    ScreenPtr pScreen = pDstPixmap->drawable.pScreen;
    KdScreenPriv(pScreen);
    pmCardInfo(pScreenPriv);

    card = pm2c;
    mmio = pm2c->reg_base;

    if (~planemask & FbFullMask(pDstPixmap->drawable.depth))
	return FALSE;

    card->BltScanDirection = ((dx >= 0 ? XPositive : 0) | (dy >= 0 ? YPositive : 0));

    GLINT_WAIT(4);
    DO_PLANEMASK(planemask);

    GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
    if ((rop == GXset) || (rop == GXclear)) {
	card->FrameBufferReadMode = card->pprod;
    } else
    if ((rop == GXcopy) || (rop == GXcopyInverted)) {
	card->FrameBufferReadMode = card->pprod |FBRM_SrcEnable;
    } else {
	card->FrameBufferReadMode = card->pprod | FBRM_SrcEnable |
							FBRM_DstEnable;
    }
    LOADROP(rop);

    return TRUE;
}


static void
pmCopy (int x1,
        int y1,
        int x2,
        int y2,
        int w,
        int h)
{
    char align;

    /* We can only use GXcopy for Packed modes */
    if (card->ROP != GXcopy) {
	GLINT_WAIT(5);
	GLINT_WRITE_REG(card->FrameBufferReadMode, FBReadMode);
        Permedia2LoadCoord(x2, y2, w, h);
        GLINT_WRITE_REG(((y1-y2)&0x0FFF)<<16 | ((x1-x2)&0x0FFF), FBSourceDelta);
    } else {
  	align = (x2 & card->bppalign) - (x1 & card->bppalign);
	GLINT_WAIT(6);
	GLINT_WRITE_REG(card->FrameBufferReadMode|FBRM_Packed, FBReadMode);
        Permedia2LoadCoord(x2>>card->BppShift, y2, 
				(w+7)>>card->BppShift, h);
  	GLINT_WRITE_REG(align<<29|x2<<16|(x2+w), PackedDataLimits);
        GLINT_WRITE_REG(((y1-y2)&0x0FFF)<<16 | (((x1 & ~card->bppalign)-(x2 & ~card->bppalign))&0x0FFF), FBSourceDelta);
    }

    GLINT_WRITE_REG(PrimitiveRectangle | card->BltScanDirection, Render);
}


static void
pmDoneCopy (void)
{
}

static void
Permedia2LoadCoord(int x, int y,
		   int w, int h)
{
    if ((h != card->h) || (w != card->w)) {
	card->w = w;
	card->h = h;
	GLINT_WRITE_REG(((h&0x0FFF)<<16)|(w&0x0FFF), RectangleSize);
    }
    if ((y != card->y) || (x != card->x)) {
	card->x = x;
	card->y = y;
	GLINT_WRITE_REG(((y&0x0FFF)<<16)|(x&0x0FFF), RectangleOrigin);
    }
}


Bool
pmDrawInit (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    pmCardInfo(pScreenPriv);
    pmScreenInfo(pScreenPriv);
    Bool    ret = TRUE;

    card = pm2c;
    mmio = pm2c->reg_base;

    memset(&pm2s->kaa, 0, sizeof(KaaScreenInfoRec));
    pm2s->kaa.waitMarker	= pmWaitMarker;
    pm2s->kaa.PrepareSolid	= pmPrepareSolid;
    pm2s->kaa.Solid		= pmSolid;
    pm2s->kaa.DoneSolid		= pmDoneSolid;
    pm2s->kaa.PrepareCopy	= pmPrepareCopy;
    pm2s->kaa.Copy		= pmCopy;
    pm2s->kaa.DoneCopy		= pmDoneCopy;

    if (ret && !kaaDrawInit (pScreen, &pm2s->kaa))
    {
	ErrorF ("kaaDrawInit failed\n");
	ret = FALSE;
    }

    return ret;
}


void
pmDrawEnable (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    pmCardInfo(pScreenPriv);

    card = pm2c;
    mmio = pm2c->reg_base;

    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	ScissorMode);
    GLINT_SLOW_WRITE_REG(UNIT_ENABLE,	FBWriteMode);
    GLINT_SLOW_WRITE_REG(0, 		dXSub);
    GLINT_SLOW_WRITE_REG(GWIN_DisableLBUpdate,   GLINTWindow);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DitherMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaBlendMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	ColorDDAMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureColorMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureAddressMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	PMTextureReadMode);
    GLINT_SLOW_WRITE_REG(card->pprod,	LBReadMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaBlendMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TexelLUTMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	YUVMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DepthMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RouterMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FogMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AntialiasMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaTestMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StencilMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AreaStippleMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LogicalOpMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DepthMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StatisticMode);
    GLINT_SLOW_WRITE_REG(0x400,		FilterMode);
    GLINT_SLOW_WRITE_REG(0xffffffff,	FBHardwareWriteMask);
    GLINT_SLOW_WRITE_REG(0xffffffff,	FBSoftwareWriteMask);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RasterizerMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	GLINTDepth);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBSourceOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBPixelOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBSourceOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	WindowOrigin);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBWindowBase);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBSourceBase);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBWindowBase);

#if X_BYTE_ORDER == X_BIG_ENDIAN
    card->RasterizerSwap = 1;
#else
    card->RasterizerSwap = 0;
#endif

    switch (pScreenPriv->screen->fb[0].bitsPerPixel) {
	case 8:
	    card->PixelWidth = 0x0; /* 8 Bits */
	    card->TexMapFormat = card->pprod;
#if X_BYTE_ORDER == X_BIG_ENDIAN
	    card->RasterizerSwap |= 3<<15;	/* Swap host data */
#endif
	    break;
	case 16:
	    card->PixelWidth = 0x1; /* 16 Bits */
	    card->TexMapFormat = card->pprod | 1<<19;
#if X_BYTE_ORDER == X_BIG_ENDIAN
	    card->RasterizerSwap |= 2<<15;	/* Swap host data */
#endif
	    break;
	case 24:
 	    card->PixelWidth = 0x4; /* 24 Bits */
	    card->TexMapFormat = card->pprod | 2<<19;
	    break;
	case 32:
	    card->PixelWidth = 0x2; /* 32 Bits */
	    card->TexMapFormat = card->pprod | 2<<19;
  	    break;
    }
    card->ClippingOn = FALSE;
    card->startxdom = 0;
    card->startxsub = 0;
    card->starty = 0;
    card->count = 0;
    card->dy = 1<<16;
    card->dxdom = 0;
    card->x = 0;
    card->y = 0;
    card->h = 0;
    card->w = 0;
    card->ROP = 0xFF;
    GLINT_SLOW_WRITE_REG(card->PixelWidth, FBReadPixel);
    GLINT_SLOW_WRITE_REG(card->TexMapFormat, PMTextureMapFormat);
    GLINT_SLOW_WRITE_REG(0, RectangleSize);
    GLINT_SLOW_WRITE_REG(0, RectangleOrigin);
    GLINT_SLOW_WRITE_REG(0, dXDom);
    GLINT_SLOW_WRITE_REG(1<<16, dY);
    GLINT_SLOW_WRITE_REG(0, StartXDom);
    GLINT_SLOW_WRITE_REG(0, StartXSub);
    GLINT_SLOW_WRITE_REG(0, StartY);
    GLINT_SLOW_WRITE_REG(0, GLINTCount);

    kaaMarkSync (pScreen);
}

void
pmDrawDisable (ScreenPtr pScreen)
{
}

void
pmDrawFini (ScreenPtr pScreen)
{
}
