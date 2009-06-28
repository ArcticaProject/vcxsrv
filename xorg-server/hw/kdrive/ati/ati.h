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

#ifndef _ATI_H_
#define _ATI_H_

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif

#ifdef KDRIVEFBDEV
#include <fbdev.h>
#endif
#ifdef KDRIVEVESA
#include <vesa.h>
#endif

#include "kxv.h"

#undef XF86DRI
#ifdef XF86DRI
#define USE_DRI
#include "xf86drm.h"
#include "dri.h"
#ifdef GLXEXT
#include "GL/glxint.h"
#include "GL/glxtokens.h"
#include "ati_dripriv.h"
#endif
#endif

#define ATI_REG_BASE(c)		((c)->attr.address[1])
#define ATI_REG_SIZE(c)		(0x4000)

#ifdef __powerpc__

static __inline__ void
MMIO_OUT32(__volatile__ void *base, const unsigned long offset,
	   const unsigned int val)
{
	__asm__ __volatile__(
			"stwbrx %1,%2,%3\n\t"
			"eieio"
			: "=m" (*((volatile unsigned char *)base+offset))
			: "r" (val), "b" (base), "r" (offset));
}

static __inline__ CARD32
MMIO_IN32(__volatile__ void *base, const unsigned long offset)
{
	register unsigned int val;
	__asm__ __volatile__(
			"lwbrx %0,%1,%2\n\t"
			"eieio"
			: "=r" (val)
			: "b" (base), "r" (offset),
			"m" (*((volatile unsigned char *)base+offset)));
	return val;
}

#else

#define MMIO_OUT32(mmio, a, v)		(*(VOL32 *)((mmio) + (a)) = (v))
#define MMIO_IN32(mmio, a)		(*(VOL32 *)((mmio) + (a)))

#endif

#define MMIO_OUT8(mmio, a, v)		(*(VOL8 *)((mmio) + (a)) = (v))
#define MMIO_IN8(mmio, a, v)		(*(VOL8 *)((mmio) + (a)))

#define INPLL(mmio, addr) \
	(MMIO_OUT8(mmio, ATI_REG_CLOCK_CNTL_INDEX, addr),		\
	 MMIO_IN32(mmio, ATI_REG_CLOCK_CNTL_DATA))

#define OUTPLL(mmio, addr, val) do {					\
	MMIO_OUT8(mmio, ATI_REG_CLOCK_CNTL_INDEX, (addr) | ATI_PLL_WR_EN); \
	MMIO_OUT32(mmio, ATI_REG_CLOCK_CNTL_DATA, val);			\
} while (0)

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;

struct pci_id_entry {
	CARD16 vendor;
	CARD16 device;
	CARD8 caps;
	char *name;
};

struct backend_funcs {
	void    (*cardfini)(KdCardInfo *);
	void    (*scrfini)(KdScreenInfo *);
	Bool    (*initScreen)(ScreenPtr);
	Bool    (*finishInitScreen)(ScreenPtr pScreen);
	Bool	(*createRes)(ScreenPtr);
	void    (*preserve)(KdCardInfo *);
	void    (*restore)(KdCardInfo *);
	Bool    (*dpms)(ScreenPtr, int);
	Bool    (*enable)(ScreenPtr);
	void    (*disable)(ScreenPtr);
	void    (*getColors)(ScreenPtr, int, int, xColorItem *);
	void    (*putColors)(ScreenPtr, int, int, xColorItem *);
#ifdef RANDR
	Bool	(*randrSetConfig) (ScreenPtr, Rotation, int, RRScreenSizePtr);
#endif
};

typedef struct _ATICardInfo {
	union {
#ifdef KDRIVEFBDEV
		FbdevPriv fbdev;
#endif
#ifdef KDRIVEVESA
		VesaCardPrivRec vesa;
#endif
	} backend_priv;
	struct backend_funcs backend_funcs;

	struct pci_id_entry *pci_id;
	char *reg_base;
	Bool is_radeon;
	Bool is_r100;
	Bool is_r200;
	Bool is_r300;
	Bool is_agp;
	char *busid;
	CARD32 crtc_pitch;
	CARD32 crtc2_pitch;
#ifdef USE_DRI
	int drmFd;
#endif /* USE_DRI */
	Bool use_fbdev, use_vesa;
} ATICardInfo;

#define getATICardInfo(kd)	((ATICardInfo *) ((kd)->card->driver))
#define ATICardInfo(kd)		ATICardInfo *atic = getATICardInfo(kd)

typedef struct _ATICursor {
	int		width, height;
	int		xhot, yhot;
	
	Bool		has_cursor;
	CursorPtr	pCursor;
	Pixel		source, mask;
	KdOffscreenArea *area;
} ATICursor;

typedef struct _ATIPortPriv {
	int brightness;
	int saturation;
	RegionRec clip;
	CARD32 size;
	KdOffscreenArea *off_screen;
	DrawablePtr pDraw;
	PixmapPtr pPixmap;

	CARD32 src_offset;
	CARD32 src_pitch;
	CARD8 *src_addr;

	int id;
	int src_x1, src_y1, src_x2, src_y2;
	int dst_x1, dst_y1, dst_x2, dst_y2;
	int src_w, src_h, dst_w, dst_h;
} ATIPortPrivRec, *ATIPortPrivPtr;

typedef struct _dmaBuf {
	int size;
	int used;
	void *address;
#ifdef USE_DRI
	drmBufPtr drmBuf;
#endif
} dmaBuf;

typedef struct _ATIScreenInfo {
	union {
#ifdef KDRIVEFBDEV
		FbdevScrPriv fbdev;
#endif
#ifdef KDRIVEVESA
		VesaScreenPrivRec vesa;
#endif
	} backend_priv;
	KaaScreenInfoRec kaa;

	ATICardInfo *atic;
	KdScreenInfo *screen;

	int		scratch_offset;
	int		scratch_next;
	KdOffscreenArea *scratch_area;

	ATICursor	cursor;

	KdVideoAdaptorPtr pAdaptor;
	int		num_texture_ports;

	Bool		using_pio;	/* If we use decode DMA packets to MMIO. */
	Bool		using_pseudo;	/* If we use MMIO to submit DMA packets. */
	Bool		using_dma;	/* If we use non-DRI DMA to submit packets. */
	Bool		using_dri;	/* If we use the DRM for DMA. */
	Bool		using_agp;	/* If we are using AGP or not for DMA. */

	KdOffscreenArea *dma_space;	/* For "DMA" from framebuffer. */
	void		*agp_addr;	/* Mapped AGP aperture */
	int		agp_size;
	int		agp_key;	/* Key of AGP memory for DMA */
	CARD32		*ring_addr;	/* Beginning of ring buffer. */
	int		ring_write;	/* Index of write ptr in ring. */
	int		ring_read;	/* Index of read ptr in ring. */
	int		ring_len;


	dmaBuf		*indirectBuffer;
	int		indirectStart;

	int		mmio_avail;
	int		cce_pri_size;
	int		cce_pri_avail;

#ifdef USE_DRI
	Bool		dma_started;

	drmSize         registerSize;
	drmHandle       registerHandle;
	drmHandle       fbHandle;

	drmSize		gartSize;
	drmHandle	agpMemHandle;		/* Handle from drmAgpAlloc */
	unsigned long	gartOffset;
	unsigned char	*AGP;			/* Map */
	int		agpMode;
	drmSize         pciSize;
	drmHandle       pciMemHandle;

	/* ring buffer data */
	unsigned long	ringStart;		/* Offset into AGP space */
	drmHandle	ringHandle;		/* Handle from drmAddMap */
	drmSize		ringMapSize;		/* Size of map */
	int		ringSize;		/* Size of ring (MB) */
	unsigned char	*ring;			/* Map */

	unsigned long	ringReadOffset;		/* Offset into AGP space */
	drmHandle	ringReadPtrHandle;	/* Handle from drmAddMap */
	drmSize		ringReadMapSize;	/* Size of map */
	unsigned char	*ringReadPtr;		/* Map */

	/* vertex/indirect buffer data */
	unsigned long	bufStart;		/* Offset into AGP space */
	drmHandle	bufHandle;		/* Handle from drmAddMap */
	drmSize		bufMapSize;		/* Size of map */
	int		bufSize;		/* Size of buffers (MB) */
	unsigned char	*buf;			/* Map */
	int		bufNumBufs;		/* Number of buffers */
	drmBufMapPtr	buffers;		/* Buffer map */
	
	/* AGP Texture data */
	unsigned long	gartTexStart;		/* Offset into AGP space */
	drmHandle	gartTexHandle;		/* Handle from drmAddMap */
	drmSize		gartTexMapSize;		/* Size of map */
	int		gartTexSize;		/* Size of AGP tex space (MB) */
	unsigned char	*gartTex;		/* Map */
	int		log2GARTTexGran;

	int		DMAusecTimeout;   /* CCE timeout in usecs */

	/* DRI screen private data */	
	int		frontOffset;
	int		frontPitch;
	int		backOffset;
	int		backPitch;
	int		depthOffset;
	int		depthPitch;
	int		spanOffset;
	int		textureOffset;
	int		textureSize;
	int		log2TexGran;

	int		irqEnabled;

	int		serverContext;

	DRIInfoPtr	pDRIInfo;
#ifdef GLXEXT
	int		numVisualConfigs;
	__GLXvisualConfig *pVisualConfigs;
	ATIConfigPrivPtr pVisualConfigsPriv;
#endif /* GLXEXT */
#endif /* USE_DRI */
} ATIScreenInfo;

#define getATIScreenInfo(kd)	((ATIScreenInfo *) ((kd)->screen->driver))
#define ATIScreenInfo(kd)	ATIScreenInfo *atis = getATIScreenInfo(kd)

typedef union { float f; CARD32 i; } fi_type;

/* Surely there's a better way to go about this */
static inline CARD32
ATIFloatAsInt(float val)
{
	fi_type fi;

	fi.f = val;
	return fi.i;
}

#define GET_FLOAT_BITS(x) ATIFloatAsInt(x)

/* ati.c */
Bool
ATIMapReg(KdCardInfo *card, ATICardInfo *atic);

void
ATIUnmapReg(KdCardInfo *card, ATICardInfo *atic);

void
R300CGWorkaround(ATIScreenInfo *atis);

/* ati_draw.c */
void
ATIDrawSetup(ScreenPtr pScreen);

Bool
ATIDrawInit(ScreenPtr pScreen);

void
ATIDrawEnable(ScreenPtr pScreen);

void
ATIDrawDisable(ScreenPtr pScreen);

void
ATIDrawFini(ScreenPtr pScreen);

/* ati_dri.c */
#ifdef USE_DRI
Bool
ATIDRIScreenInit(ScreenPtr pScreen);

void
ATIDRICloseScreen(ScreenPtr pScreen);

void
ATIDRIDMAStart(ATIScreenInfo *atis);

void
ATIDRIDMAStop(ATIScreenInfo *atis);

void
ATIDRIDMAReset(ATIScreenInfo *atis);

void
ATIDRIDispatchIndirect(ATIScreenInfo *atis, Bool discard);

drmBufPtr
ATIDRIGetBuffer(ATIScreenInfo *atis);

#endif /* USE_DRI */

/* ati_cursor.c */
Bool
ATICursorInit(ScreenPtr pScreen);

void
ATICursorEnable(ScreenPtr pScreen);

void
ATICursorDisable(ScreenPtr pScreen);

void
ATICursorFini(ScreenPtr pScreen);

void
ATIRecolorCursor(ScreenPtr pScreen, int ndef, xColorItem *pdef);

int
ATILog2(int val);

/* ati_video.c */
Bool
ATIInitVideo(ScreenPtr pScreen);

void
ATIFiniVideo(ScreenPtr pScreen);

extern KdCardFuncs ATIFuncs;

#endif /* _ATI_H_ */
