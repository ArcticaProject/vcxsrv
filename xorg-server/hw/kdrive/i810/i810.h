/* COPYRIGHT AND PERMISSION NOTICE

Copyright (c) 2000, 2001 Nokia Home Communications

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, provided that the above
copyright notice(s) and this permission notice appear in all copies of
the Software and that both the above copyright notice(s) and this
permission notice appear in supporting documentation.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY
SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder
shall not be used in advertising or otherwise to promote the sale, use
or other dealings in this Software without prior written authorization
of the copyright holder.

X Window System is a trademark of The Open Group */

/*
 * Author:
 *   Pontus Lidman <pontus.lidman@nokia.com>
 */

#ifndef _I810_H_
#define _I810_H_

#include "i810_reg.h"

/* Globals */

typedef struct _I810Rec *I810Ptr;

/* Linear region allocated in framebuffer.
 */
typedef struct {
   unsigned long Start;
   unsigned long End;
   unsigned long Size;
} I810MemRange;

typedef struct {
   int tail_mask;
   I810MemRange mem;
   unsigned char *virtual_start;
   int head;
   int tail;
   int space;
} I810RingBuffer;

typedef struct {
   unsigned char DisplayControl;
   unsigned char PixelPipeCfg0;
   unsigned char PixelPipeCfg1;
   unsigned char PixelPipeCfg2;
   unsigned short VideoClk2_M;
   unsigned short VideoClk2_N;
   unsigned char VideoClk2_DivisorSel;
   unsigned char AddressMapping;
   unsigned char IOControl;
   unsigned char BitBLTControl;
   unsigned char ExtVertTotal;
   unsigned char ExtVertDispEnd;
   unsigned char ExtVertSyncStart;
   unsigned char ExtVertBlankStart;
   unsigned char ExtHorizTotal;
   unsigned char ExtHorizBlank;
   unsigned char ExtOffset;
   unsigned char InterlaceControl;
   unsigned int  LMI_FIFO_Watermark;

   unsigned int  LprbTail;
   unsigned int  LprbHead;
   unsigned int  LprbStart;
   unsigned int  LprbLen;

   unsigned int Fence[8];

   unsigned short OverlayActiveStart;
   unsigned short OverlayActiveEnd;


} I810RegRec, *I810RegPtr;

#define minb(p) *(volatile CARD8 *)(i810c->MMIOBase + (p))
#define moutb(p,v) *(volatile CARD8 *)(i810c->MMIOBase + (p)) = (v)

#define OUT_RING(n) {					\
   if (I810_DEBUG & DEBUG_VERBOSE_RING)			\
      ErrorF( "OUT_RING %x: %x\n", outring, n);	\
   *(volatile unsigned int *)(virt + outring) = n;	\
   outring += 4;					\
   outring &= ringmask;					\
}

#define ADVANCE_LP_RING() {					\
    i810c->LpRing.tail = outring;					\
    OUTREG(LP_RING + RING_TAIL, outring);	\
}

#ifdef __GNUC__
#define LP_RING_MESSAGE(n) \
   ErrorF("BEGIN_LP_RING %d in %s\n", n, __FUNCTION__)
#else
#define LP_RING_MESSAGE(n) \
   ErrorF("BEGIN_LP_RING %d in %s:%d\n", n, __FILE__, __LINE__)
#endif

#define LP_RING_LOCALS \
    unsigned int outring, ringmask;					\
    volatile unsigned char *virt

#define BEGIN_LP_RING(n)						\
    if (n>2 && (I810_DEBUG&DEBUG_ALWAYS_SYNC))				\
	i810Sync(i810s);	\
    if (i810c->LpRing.space < n*4) i810WaitLpRing(i810s, n*4, 0);	\
    i810c->LpRing.space -= n*4;						\
    if (I810_DEBUG & DEBUG_VERBOSE_RING) 				\
	LP_RING_MESSAGE(n);						\
    outring = i810c->LpRing.tail;					\
    ringmask = i810c->LpRing.tail_mask;					\
    virt = i810c->LpRing.virtual_start;			

/* Memory mapped register access macros */
#define INREG8(addr)        *(volatile CARD8  *)(i810c->MMIOBase + (addr))
#define INREG16(addr)       *(volatile CARD16 *)(i810c->MMIOBase + (addr))
#define INREG(addr)         *(volatile CARD32 *)(i810c->MMIOBase + (addr))

#define OUTREG8(addr, val) do {				\
   *(volatile CARD8 *)(i810c->MMIOBase  + (addr)) = (val);	\
   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG)			\
     ErrorF( "OUTREG8(%x, %x)\n", addr, val);	\
} while (0)

#define OUTREG16(addr, val) do {			\
   *(volatile CARD16 *)(i810c->MMIOBase + (addr)) = (val);	\
   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG)			\
     ErrorF( "OUTREG16(%x, %x)\n", addr, val);	\
} while (0)

#define OUTREG(addr, val) do {				\
   *(volatile CARD32 *)(i810c->MMIOBase + (addr)) = (val);	\
   if (I810_DEBUG&DEBUG_VERBOSE_OUTREG)			\
     ErrorF( "OUTREG(%x, %x)\n", addr, val);	\
} while (0)

/* To remove all debugging, make sure I810_DEBUG is defined as a
 * preprocessor symbol, and equal to zero.  
 */

#define I810_DEBUG 0

#ifndef I810_DEBUG
#warning "Debugging enabled - expect reduced performance"
extern int I810_DEBUG;
#endif

#define DEBUG_VERBOSE_ACCEL  0x1
#define DEBUG_VERBOSE_SYNC   0x2
#define DEBUG_VERBOSE_VGA    0x4
#define DEBUG_VERBOSE_RING   0x8
#define DEBUG_VERBOSE_OUTREG 0x10
#define DEBUG_VERBOSE_MEMORY 0x20
#define DEBUG_VERBOSE_CURSOR 0x40
#define DEBUG_ALWAYS_SYNC    0x80
#define DEBUG_VERBOSE_DRI    0x100


/* Size of the mmio region.
 */
#define I810_REG_SIZE 0x80000

/* PCI identifiers */
#ifndef PCI_CHIP_I810
#define PCI_CHIP_I810              0x7121
#define PCI_CHIP_I810_DC100        0x7123
#define PCI_CHIP_I810_E            0x7125 
#define PCI_CHIP_I815              0x1132 
#define PCI_CHIP_I810_BRIDGE       0x7120
#define PCI_CHIP_I810_DC100_BRIDGE 0x7122
#define PCI_CHIP_I810_E_BRIDGE     0x7124
#define PCI_CHIP_I815_BRIDGE       0x1130
#define PCI_CHIP_I845G             0x2562
#endif


#define IS_I810(i810c) (i810c->PciInfo->chipType == PCI_CHIP_I810 ||	\
			i810c->PciInfo->chipType == PCI_CHIP_I810_DC100 || \
			i810c->PciInfo->chipType == PCI_CHIP_I810_E)
#define IS_I815(i810c) (i810c->PciInfo->chipType == PCI_CHIP_I815)


/* default number of VGA registers stored internally */
#define VGA_NUM_CRTC 25 /* 0x19 */
#define VGA_NUM_SEQ 5
#define VGA_NUM_GFX 9
#define VGA_NUM_ATTR 21

/*
 * Settings of standard VGA registers.
 */
typedef struct {
    unsigned char MiscOutReg;     /* */
    unsigned char CRTC[VGA_NUM_CRTC];       /* Crtc Controller */
    unsigned char Sequencer[VGA_NUM_SEQ];   /* Video Sequencer */
    unsigned char Graphics[VGA_NUM_GFX];    /* Video Graphics */
    unsigned char Attribute[VGA_NUM_ATTR];  /* Video Atribute */
    unsigned char DAC[768];       /* Internal Colorlookuptable */
} vgaRegRec, *vgaRegPtr;


typedef struct _i810VGARec *i810VGAPtr;

/* VGA registers */
typedef struct _i810VGARec {
    int				IOBase;		/* I/O Base address */
    CARD8 * 			MMIOBase;	/* Pointer to MMIO start */
    vgaRegRec			SavedReg;	/* saved registers */
    vgaRegRec			ModeReg;	/* register settings for
							current mode */
    Bool			ShowOverscan;
    Bool			paletteEnabled;
    Bool			cmapSaved;
} i810VGARec;

typedef struct _i810CardInfo {
    int videoRam;
    int MaxClock;    
    long FbMapSize;
    int cpp;                    /* chars per pixel */

    unsigned long LinearAddr;
    unsigned long MMIOAddr;
    
    unsigned char *MMIOBase;
    unsigned char *FbBase;

    Bool GttBound;
    Bool agpAcquired2d;
    int VramKey;
    unsigned long VramOffset;
    int DcacheKey;
    unsigned long DcacheOffset;
    int HwcursKey; 
    unsigned long HwcursOffset;

    I810MemRange DcacheMem;
    I810MemRange SysMem;

    I810MemRange SavedDcacheMem;
    I810MemRange SavedSysMem;

    unsigned int bufferOffset;	/* for I810SelectBuffer */
    Bool DoneFrontAlloc;
    BoxRec FbMemBox;
    I810MemRange FrontBuffer;
    I810MemRange Scratch;
    I810MemRange XvMem;

    int  LmFreqSel;
    
    i810VGARec vga;

    I810RegRec SavedReg;
    I810RegRec ModeReg;
    I810RingBuffer LpRing;

    unsigned int BR[20]; 

    int CursorOffset;
    unsigned long CursorPhysical;
    unsigned long CursorStart;
    unsigned long OverlayPhysical;
    unsigned long OverlayStart;
    int colorKey;

    int nextColorExpandBuf;

    ScreenBlockHandlerProcPtr BlockHandler;

#ifdef XV
    KdVideoAdaptorPtr adaptor;
#endif

} i810CardInfo;

typedef struct _i810CardInfo I810CardInfo;	/* compatibility */

#define getI810CardInfo(kd)	((I810CardInfo *) ((kd)->card->driver))
#define i810CardInfo(kd)	I810CardInfo *i810c = getI810CardInfo(kd)

#define getI810ScreenInfo(kd)	((I810ScreenInfo *) ((kd)->screen->driver))
#define i810ScreenInfo(kd)	I810ScreenInfo *i810s = getI810ScreenInfo(kd)

typedef struct _i810Cursor {
    int		width, height;
    int		xhot, yhot;
    Bool	has_cursor;
    CursorPtr	pCursor;
} i810Cursor, *i810CursorPtr;

typedef struct _i810ScreenInfo {
    i810CardInfo *i810c;
    i810Cursor cursor;

    int pitch;
    KaaScreenInfoRec kaa;
} i810ScreenInfo;

typedef struct _i810ScreenInfo I810ScreenInfo;	/* compatibility */

#define I810_CURSOR_HEIGHT 64
#define I810_CURSOR_WIDTH 64

/* init functions (i810.c) */

Bool 
i810CardInit (KdCardInfo *card);

Bool 
i810ScreenInit (KdScreenInfo *screen);

/* The cursor functions (i810_cursor.c) */

Bool
i810CursorInit(ScreenPtr pScreen);

void
i810CursorEnable (ScreenPtr pScreen);

void
i810CursorDisable (ScreenPtr pScreen);

void
i810CursorFini (ScreenPtr pScreen);

/* Accel functions (i810draw.c) */

Bool        
i810InitAccel(ScreenPtr);

void        
i810EnableAccel (ScreenPtr);

void        
i810DisableAccel (ScreenPtr);

void        
i810FiniAccel (ScreenPtr);

void
i810FillBoxSolid (KdScreenInfo *screen, int nBox, BoxPtr pBox, 
                  unsigned long pixel, int alu, unsigned long planemask);


extern KdCardFuncs  i810Funcs;

/* Standard VGA registers */

#define VGA_ATTR_INDEX		0x3C0
#define VGA_ATTR_DATA_W		0x3C0
#define VGA_ATTR_DATA_R		0x3C1
#define VGA_IN_STAT_0		0x3C2		/* read */
#define VGA_MISC_OUT_W		0x3C2		/* write */
#define VGA_ENABLE		0x3C3
#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5
#define VGA_DAC_MASK		0x3C6
#define VGA_DAC_READ_ADDR	0x3C7
#define VGA_DAC_WRITE_ADDR	0x3C8
#define VGA_DAC_DATA		0x3C9
#define VGA_FEATURE_R		0x3CA		/* read */
#define VGA_MISC_OUT_R		0x3CC		/* read */
#define VGA_GRAPH_INDEX		0x3CE
#define VGA_GRAPH_DATA		0x3CF

#define VGA_IOBASE_MONO		0x3B0
#define VGA_IOBASE_COLOR	0x3D0

#define VGA_CRTC_INDEX_OFFSET	0x04
#define VGA_CRTC_DATA_OFFSET	0x05
#define VGA_IN_STAT_1_OFFSET	0x0A		/* read */
#define VGA_FEATURE_W_OFFSET	0x0A		/* write */

/* VGA stuff */
#define BIT_PLANE 3		/* Which plane we write to in mono mode */

/* DAC indices for white and black */
#define WHITE_VALUE 0x3F
#define BLACK_VALUE 0x00
#define OVERSCAN_VALUE 0x01

#define OVERSCAN 0x11		/* Index of OverScan register */

void 
i810VGAUnlock(i810VGAPtr vgap);

void 
i810VGALock(i810VGAPtr vgap);

Bool 
i810VGAInit(KdScreenInfo *scrninfp, const KdMonitorTiming *t);

void
i810VGABlankScreen(KdCardInfo *card, Bool on);

void
i810AdjustFrame(KdScreenInfo *screen, int x, int y, int flags);

Bool
i810VGAMapMem(KdCardInfo *card);

void
i810VGASave(KdCardInfo *card, vgaRegPtr save, int flags);

void 
i810PrintErrorState(i810CardInfo *i810c);

void
i810VGAGetIOBase(i810VGAPtr vgap);

Bool
i810InitVideo(ScreenPtr pScreen);

/*
 * MMIO versions of the register access functions.  These require
 * hwp->MemBase to be set in such a way that when the standard VGA port
 * address is added the correct memory address results.
 */

#define Vminb(p) ( *(volatile CARD8 *)(vgap->MMIOBase + (p)))
#define Vmoutb(p,v) ( *(volatile CARD8 *)(vgap->MMIOBase + (p)) = (v))

#define mmioWriteCrtc(vgap, index, value) { \
    Vmoutb(vgap->IOBase + VGA_CRTC_INDEX_OFFSET, index); \
    Vmoutb(vgap->IOBase + VGA_CRTC_DATA_OFFSET, value); \
}

#define mmioReadCrtc(vgap, index) ( \
    Vmoutb(vgap->IOBase + VGA_CRTC_INDEX_OFFSET, index), \
    Vminb(vgap->IOBase + VGA_CRTC_DATA_OFFSET) \
)

#define mmioWriteGr(vgap, index, value) { \
    Vmoutb(VGA_GRAPH_INDEX, index); \
    Vmoutb(VGA_GRAPH_DATA, value); \
}

#define mmioReadGr(vgap, index) ( \
    Vmoutb(VGA_GRAPH_INDEX, index), \
    Vminb(VGA_GRAPH_DATA) \
)

#define mmioWriteSeq(vgap, index, value) {\
    Vmoutb(VGA_SEQ_INDEX, index); \
    Vmoutb(VGA_SEQ_DATA, value); \
}

#define mmioReadSeq(vgap, index) ( \
    Vmoutb(VGA_SEQ_INDEX, index), \
    Vminb(VGA_SEQ_DATA) \
)

#define mmioWriteAttr(vgap, index, value) { \
    (void) Vminb(vgap->IOBase + VGA_IN_STAT_1_OFFSET); \
    Vmoutb(VGA_ATTR_INDEX, index); \
    Vmoutb(VGA_ATTR_DATA_W, value); \
}

#define mmioReadAttr(vgap, index) ( \
    (void) Vminb(vgap->IOBase + VGA_IN_STAT_1_OFFSET), \
    Vmoutb(VGA_ATTR_INDEX, index), \
    Vminb(VGA_ATTR_DATA_R) \
)

#define mmioWriteMiscOut(vgap, value) Vmoutb(VGA_MISC_OUT_W, value)


#define mmioReadMiscOut(vgap) Vminb(VGA_MISC_OUT_R)

#define mmioEnablePalette(vgap) { \
    (void) Vminb(vgap->IOBase + VGA_IN_STAT_1_OFFSET); \
    Vmoutb(VGA_ATTR_INDEX, 0x00); \
    vgap->paletteEnabled = TRUE; \
}

#define mmioDisablePalette(vgap) { \
    (void) Vminb(vgap->IOBase + VGA_IN_STAT_1_OFFSET); \
    Vmoutb(VGA_ATTR_INDEX, 0x20); \
    vgap->paletteEnabled = FALSE; \
}

#define mmioWriteDacWriteAddr(vgap, value) Vmoutb(VGA_DAC_WRITE_ADDR, value)

#define mmioWriteDacData(vgap, value) Vmoutb(VGA_DAC_DATA, value)

#endif /* _I810_H_ */
