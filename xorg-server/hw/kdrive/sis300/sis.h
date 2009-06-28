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

#ifndef _SIS_H_
#define _SIS_H_

#include "kdrive-config.h"

#ifdef KDRIVEFBDEV
#include <fbdev.h>
#endif
#ifdef KDRIVEVESA
#include <vesa.h>
#endif

/* XXX */
#define SIS_REG_BASE(c)		((c)->attr.address[1])
#define SIS_REG_SIZE(c)		(0x10000)

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
};

typedef struct _SiSCardInfo {
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
	CARD8 *reg_base;
	Bool use_fbdev, use_vesa;
} SiSCardInfo;

#define getSiSCardInfo(kd)	((SiSCardInfo *) ((kd)->card->driver))
#define SiSCardInfo(kd)		SiSCardInfo *sisc = getSiSCardInfo(kd)

typedef struct _SiSScreenInfo {
	union {
#ifdef KDRIVEFBDEV
		FbdevScrPriv fbdev;
#endif
#ifdef KDRIVEVESA
		VesaScreenPrivRec vesa;
#endif
	} backend_priv;
	CARD32 depthSet;	/* depth value for REG_BLT_SRCPITCH */
	KaaScreenInfoRec kaa;
	SiSCardInfo *sisc;
} SiSScreenInfo;

#define getSiSScreenInfo(kd)	((SiSScreenInfo *) ((kd)->screen->driver))
#define SiSScreenInfo(kd)	SiSScreenInfo *siss = getSiSScreenInfo(kd)

Bool
SiSMapReg(KdCardInfo *card, SiSCardInfo *sisc);

void
SiSUnmapReg(KdCardInfo *card, SiSCardInfo *sisc);

Bool
SiSDrawSetup(ScreenPtr pScreen);

Bool
SiSDrawInit(ScreenPtr pScreen);

void
SiSDrawEnable(ScreenPtr pScreen);

void
SiSDrawSync(ScreenPtr pScreen);

void
SiSDrawDisable(ScreenPtr pScreen);

void
SiSDrawFini(ScreenPtr pScreen);

extern KdCardFuncs SiSFuncs;

#endif /* _SIS_H_ */
