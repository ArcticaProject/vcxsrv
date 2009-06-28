#ifndef _PM2_H_
#define _PM2_H_
#include <vesa.h>
#include "kxv.h"
#include "klinux.h"

#include "glint_regs.h"

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;

#define PM2_REG_BASE(c)		((c)->attr.address[0] & 0xFFFFC000)
#define PM2_REG_SIZE(c)		(0x10000)

typedef struct _PM2CardInfo {
    VesaCardPrivRec vesa;
    CARD8 *reg_base;

    int InFifoSpace;
    int	FIFOSize;

    int	pprod;
    int	bppalign;

    int	ClippingOn;

    int	ROP;

    int x;
    int	y;
    int	w;
    int	h;

    int	FrameBufferReadMode;
    int	BppShift;
    int	BltScanDirection;

    int	RasterizerSwap;
    int	PixelWidth;
    int	TexMapFormat;
    int	startxdom;
    int	startxsub;
    int	starty;
    int	count;
    int	dy;
    int	dxdom;

    int	planemask;
} PM2CardInfo;

#define getPM2CardInfo(kd)	((PM2CardInfo *) ((kd)->card->driver))
#define pmCardInfo(kd)	PM2CardInfo	*pm2c = getPM2CardInfo(kd)

typedef struct _PM2ScreenInfo {
    VesaScreenPrivRec vesa;
    CARD8 *cursor_base;
    CARD8 *screen;
    CARD8 *off_screen;
    int	 off_screen_size;
    KdVideoAdaptorPtr pAdaptor;
    KaaScreenInfoRec kaa;
} PM2ScreenInfo;

#define getPM2ScreenInfo(kd) ((PM2ScreenInfo *) ((kd)->screen->driver))
#define pmScreenInfo(kd)    PM2ScreenInfo *pm2s = getPM2ScreenInfo(kd)

Bool 
pmCardInit (KdCardInfo *card);

Bool 
pmScreenInit (KdScreenInfo *screen);

Bool        
pmDrawInit(ScreenPtr);

void
pmDrawEnable (ScreenPtr);

void
pmDrawDisable (ScreenPtr);

void
pmDrawFini (ScreenPtr);


extern KdCardFuncs  PM2Funcs;

#define MMIO_OUT32(base, offset, val) 				\
do { 								\
	*(volatile CARD32 *)(void *)(((CARD8*)(base)) + (offset)) = (val); \
} while (0)

#  define MMIO_IN32(base, offset) 				\
	*(volatile CARD32 *)(void *)(((CARD8*)(base)) + (offset))

#define GLINT_WRITE_REG(v,r) 					\
	MMIO_OUT32(mmio,(unsigned long)(r), (v))

#define GLINT_READ_REG(r) 					\
	MMIO_IN32(mmio,(unsigned long)(r))

#define GLINT_SLOW_WRITE_REG(v,r)				\
do{								\
	GLINT_WAIT(card->FIFOSize);		     		\
        GLINT_WRITE_REG(v,r);					\
}while(0)

#define REPLICATE(r)						\
{								\
	if (pScreenPriv->screen->fb[0].bitsPerPixel == 16) {	\
		r &= 0xFFFF;					\
		r |= (r<<16);					\
	} else							\
	if (pScreenPriv->screen->fb[0].bitsPerPixel == 8) { 	\
		r &= 0xFF;					\
		r |= (r<<8);					\
		r |= (r<<16);					\
	}							\
}

#define DO_PLANEMASK(planemask)					\
{ 								\
	if (planemask != card->planemask) {			\
		card->planemask = planemask;			\
		REPLICATE(planemask); 				\
		GLINT_WRITE_REG(planemask, FBHardwareWriteMask);\
	}							\
} 

#define LOADROP(rop)						\
{								\
	if (card->ROP != rop)	{				\
		GLINT_WRITE_REG(rop<<1|UNIT_ENABLE, LogicalOpMode);	\
		card->ROP = rop;				\
	}							\
}

#define GLINT_WAIT(n)						\
do{								\
	if (card->InFifoSpace>=(n))				\
	    card->InFifoSpace -= (n);				\
	else {							\
	    int tmp;						\
	    while((tmp=GLINT_READ_REG(InFIFOSpace))<(n));	\
	    /* Clamp value due to bugs in PM3 */		\
	    if (tmp > card->FIFOSize)				\
		tmp = card->FIFOSize;				\
	    card->InFifoSpace = tmp - (n);			\
	}							\
}while(0)

#define CHECKCLIPPING						\
{								\
	if (card->ClippingOn) {					\
		card->ClippingOn = FALSE;			\
		GLINT_WAIT(1);					\
		GLINT_WRITE_REG(0, ScissorMode);		\
	}							\
}

#endif /* _PM2_H_ */
