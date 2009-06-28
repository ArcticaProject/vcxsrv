/*
 * Copyright © 2004 Keith Packard
 * Copyright © 2005 Eric Anholt
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
 *
 * Based on mach64video.c by Keith Packard.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "ati.h"
#include "ati_dma.h"
#include "ati_draw.h"
#include "ati_reg.h"
#include "kaa.h"

#include <X11/extensions/Xv.h>
#include "fourcc.h"

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

static Atom xvBrightness, xvSaturation;

extern CARD8 ATIBltRop[16];

#define IMAGE_MAX_WIDTH		2048
#define IMAGE_MAX_HEIGHT	2048

static void
ATIStopVideo(KdScreenInfo *screen, pointer data, Bool exit)
{
	ScreenPtr pScreen = screen->pScreen;
	ATIPortPrivPtr pPortPriv = (ATIPortPrivPtr)data;

	REGION_EMPTY(screen->pScreen, &pPortPriv->clip);   

	if (pPortPriv->off_screen) {
		KdOffscreenFree (pScreen, pPortPriv->off_screen);
		pPortPriv->off_screen = 0;
	}
}

static int
ATISetPortAttribute(KdScreenInfo *screen, Atom attribute, int value,
    pointer data)
{
	return BadMatch;
}

static int
ATIGetPortAttribute(KdScreenInfo *screen, Atom attribute, int *value,
    pointer data)
{
	return BadMatch;
}

static void
ATIQueryBestSize(KdScreenInfo *screen, Bool motion, short vid_w, short vid_h,
    short drw_w, short drw_h, unsigned int *p_w, unsigned int *p_h,
    pointer data)
{
	*p_w = drw_w;
	*p_h = drw_h;
}

/* ATIClipVideo -  

   Takes the dst box in standard X BoxRec form (top and left
   edges inclusive, bottom and right exclusive).  The new dst
   box is returned.  The source boundaries are given (x1, y1 
   inclusive, x2, y2 exclusive) and returned are the new source 
   boundaries in 16.16 fixed point. 
*/

static void
ATIClipVideo(BoxPtr dst, INT32 *x1, INT32 *x2, INT32 *y1, INT32 *y2,
    BoxPtr extents, INT32 width, INT32 height)
{
	INT32 vscale, hscale, delta;
	int diff;

	hscale = ((*x2 - *x1) << 16) / (dst->x2 - dst->x1);
	vscale = ((*y2 - *y1) << 16) / (dst->y2 - dst->y1);

	*x1 <<= 16; *x2 <<= 16;
	*y1 <<= 16; *y2 <<= 16;

	diff = extents->x1 - dst->x1;
	if (diff > 0) {
		dst->x1 = extents->x1;
		*x1 += diff * hscale;
	}
	diff = dst->x2 - extents->x2;
	if (diff > 0) {
		dst->x2 = extents->x2;
		*x2 -= diff * hscale;
	}
	diff = extents->y1 - dst->y1;
	if (diff > 0) {
		dst->y1 = extents->y1;
		*y1 += diff * vscale;
	}
	diff = dst->y2 - extents->y2;
	if (diff > 0) {
		dst->y2 = extents->y2;
		*y2 -= diff * vscale;
	}

	if (*x1 < 0) {
		diff =  (- *x1 + hscale - 1)/ hscale;
		dst->x1 += diff;
		*x1 += diff * hscale;
	}
	delta = *x2 - (width << 16);
	if (delta > 0) {
		diff = (delta + hscale - 1)/ hscale;
		dst->x2 -= diff;
		*x2 -= diff * hscale;
	}
	if (*y1 < 0) {
		diff =  (- *y1 + vscale - 1)/ vscale;
		dst->y1 += diff;
		*y1 += diff * vscale;
	}
	delta = *y2 - (height << 16);
	if (delta > 0) {
		diff = (delta + vscale - 1)/ vscale;
		dst->y2 -= diff;
		*y2 -= diff * vscale;
	}
}

static void
R128DisplayVideo(KdScreenInfo *screen, ATIPortPrivPtr pPortPriv)
{
	ScreenPtr pScreen = screen->pScreen;
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	CARD32 dstDatatype, srcDatatype;
	CARD32 dst_offset, dst_pitch;
	int dstxoff, dstyoff;
	PixmapPtr pPixmap = pPortPriv->pPixmap;
	int bpp = pPixmap->drawable.bitsPerPixel;
	RING_LOCALS;

	BoxPtr pBox = REGION_RECTS(&pPortPriv->clip);
	int nBox = REGION_NUM_RECTS(&pPortPriv->clip);

	if (pPortPriv->id == FOURCC_UYVY)
		srcDatatype = R128_DATATYPE_YVYU_422;
	else
		srcDatatype = R128_DATATYPE_VYUY_422;

	switch (bpp)
	{
	case 16:
		if (pPixmap->drawable.depth == 15)
			dstDatatype = R128_DATATYPE_ARGB1555;
		else
			dstDatatype = R128_DATATYPE_RGB565;
		break;
	case 32:
		dstDatatype = R128_DATATYPE_ARGB8888;
		break;
	default:
		return;
	}

	dst_offset = ((CARD8 *)pPixmap->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);
	dst_pitch = pPixmap->devKind;
#ifdef COMPOSITE
	dstxoff = -pPixmap->screen_x + pPixmap->drawable.x;
	dstyoff = -pPixmap->screen_y + pPixmap->drawable.y;
#else
	dstxoff = 0;
	dstyoff = 0;
#endif

	BEGIN_DMA(18);
	OUT_REG(ATI_REG_DST_PITCH_OFFSET,
	    ((dst_pitch / bpp) << 21) | (dst_offset >> 5));
	OUT_REG(ATI_REG_DP_GUI_MASTER_CNTL,
	    ATI_GMC_DST_PITCH_OFFSET_CNTL |
	    ATI_GMC_BRUSH_NONE |
	    (dstDatatype << 8) |
	    ATI_GMC_SRC_DATATYPE_COLOR |
	    (ATIBltRop[GXcopy] << 16) |
	    R128_GMC_3D_FCN_EN |
	    ATI_GMC_CLR_CMP_CNTL_DIS |
	    R128_GMC_AUX_CLIP_DIS);
	OUT_REG(ATI_REG_DP_CNTL, 
	    ATI_DST_X_LEFT_TO_RIGHT | ATI_DST_Y_TOP_TO_BOTTOM );
	OUT_REG(R128_REG_SCALE_3D_CNTL,
	    R128_SCALE_3D_SCALE |
	    R128_SBLEND_ONE |
	    R128_DBLEND_ZERO);
	OUT_REG(R128_REG_TEX_CNTL_C, R128_TEX_CACHE_FLUSH);
	OUT_REG(R128_REG_SCALE_3D_DATATYPE, srcDatatype);

	OUT_RING(DMA_PACKET0(R128_REG_SCALE_PITCH, 5));
	OUT_RING_REG(R128_REG_SCALE_PITCH, pPortPriv->src_pitch / 16);
	OUT_RING_REG(R128_REG_SCALE_X_INC,
	    (pPortPriv->src_w << 16) / pPortPriv->dst_w);
	OUT_RING_REG(R128_REG_SCALE_Y_INC,
	    (pPortPriv->src_h << 16) / pPortPriv->dst_h);
	OUT_RING_REG(R128_REG_SCALE_HACC, 0x0);
	OUT_RING_REG(R128_REG_SCALE_VACC, 0x0);

	END_DMA();

	while (nBox--) {
		int srcX, srcY, dstX, dstY, srcw, srch, dstw, dsth;

		dstX = pBox->x1 + dstxoff;
		dstY = pBox->y1 + dstyoff;
		dstw = pBox->x2 - pBox->x1;
		dsth = pBox->y2 - pBox->y1;
		srcX = (pBox->x1 - pPortPriv->dst_x1) *
		    pPortPriv->src_w / pPortPriv->dst_w;
		srcY = (pBox->y1 - pPortPriv->dst_y1) *
		    pPortPriv->src_h / pPortPriv->dst_h;
		srcw = pPortPriv->src_w - srcX;
		srch = pPortPriv->src_h - srcY;

		BEGIN_DMA(6);
		OUT_RING(DMA_PACKET0(R128_REG_SCALE_SRC_HEIGHT_WIDTH, 2));
		OUT_RING_REG(R128_REG_SCALE_SRC_HEIGHT_WIDTH,
		    (srch << 16) | srcw);
		OUT_RING_REG(R128_REG_SCALE_OFFSET_0, pPortPriv->src_offset +
		    srcY * pPortPriv->src_pitch + srcX * 2);

		OUT_RING(DMA_PACKET0(R128_REG_SCALE_DST_X_Y, 2));
		OUT_RING_REG(R128_REG_SCALE_DST_X_Y, (dstX << 16) | dstY);
		OUT_RING_REG(R128_REG_SCALE_DST_HEIGHT_WIDTH,
		    (dsth << 16) | dstw);
		END_DMA();
		pBox++;
	}
#ifdef DAMAGEEXT
	/* XXX: Shouldn't this be in kxv.c instead? */
	DamageDamageRegion(pPortPriv->pDraw, &pPortPriv->clip);
#endif
	kaaMarkSync(pScreen);
}

union intfloat {
	float f;
	CARD32 i;
};

struct blend_vertex {
	union intfloat x, y;
	union intfloat s0, t0;
};

#define VTX_DWORD_COUNT 4

#define VTX_OUT(vtx)		\
do {				\
	OUT_RING(vtx.x.i);	\
	OUT_RING(vtx.y.i);	\
	OUT_RING(vtx.s0.i);	\
	OUT_RING(vtx.t0.i);	\
} while (0)

static void
RadeonDisplayVideo(KdScreenInfo *screen, ATIPortPrivPtr pPortPriv)
{
	ScreenPtr pScreen = screen->pScreen;
	KdScreenPriv(pScreen);
	ATICardInfo(pScreenPriv);
	ATIScreenInfo(pScreenPriv);
	struct blend_vertex vtx[4];
	PixmapPtr pPixmap = pPortPriv->pPixmap;
	CARD32 txformat;
	CARD32 dst_offset, dst_pitch, dst_format;
	int dstxoff, dstyoff, pixel_shift;
	RING_LOCALS;

	BoxPtr pBox = REGION_RECTS(&pPortPriv->clip);
	int nBox = REGION_NUM_RECTS(&pPortPriv->clip);

	switch (pPixmap->drawable.bitsPerPixel) {
	case 16:
		if (pPixmap->drawable.depth == 15)
			dst_format = RADEON_COLOR_FORMAT_ARGB1555;
		else
			dst_format = RADEON_COLOR_FORMAT_RGB565;
		pixel_shift = 1;
		break;
	case 32:
		dst_format = RADEON_COLOR_FORMAT_ARGB8888;
		pixel_shift = 2;
		break;
	default:
		return;
	}

	dst_offset = ((CARD8 *)pPixmap->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);
	dst_pitch = pPixmap->devKind;

#ifdef COMPOSITE
	dstxoff = -pPixmap->screen_x + pPixmap->drawable.x;
	dstyoff = -pPixmap->screen_y + pPixmap->drawable.y;
#else
	dstxoff = 0;
	dstyoff = 0;
#endif

	/* Same for R100/R200 */
	if (pPortPriv->id == FOURCC_UYVY)
		txformat = RADEON_TXFORMAT_YVYU422;
	else
		txformat = RADEON_TXFORMAT_VYUY422;

	txformat |= RADEON_TXFORMAT_NON_POWER2;

	RadeonSwitchTo3D(atis);

	BEGIN_DMA(8);

	OUT_RING(DMA_PACKET0(RADEON_REG_PP_CNTL, 3));
	OUT_RING_REG(RADEON_REG_PP_CNTL,
	    RADEON_TEX_0_ENABLE | RADEON_TEX_BLEND_0_ENABLE);
	OUT_RING_REG(RADEON_REG_RB3D_CNTL,
	    dst_format | RADEON_ALPHA_BLEND_ENABLE);
	OUT_RING_REG(RADEON_REG_RB3D_COLOROFFSET, dst_offset);

	OUT_REG(RADEON_REG_RB3D_COLORPITCH, dst_pitch >> pixel_shift);

	OUT_REG(RADEON_REG_RB3D_BLENDCNTL,
	    RADEON_SBLEND_GL_ONE | RADEON_DBLEND_GL_ZERO);

	END_DMA();

	if (atic->is_r200) {
		BEGIN_DMA(17);

		OUT_REG(R200_REG_SE_VTX_FMT_0, R200_VTX_XY);
		OUT_REG(R200_REG_SE_VTX_FMT_1,
		    (2 << R200_VTX_TEX0_COMP_CNT_SHIFT));

		OUT_RING(DMA_PACKET0(R200_REG_PP_TXFILTER_0, 5));
		OUT_RING_REG(R200_REG_PP_TXFILTER_0,
		    R200_MAG_FILTER_LINEAR |
		    R200_MIN_FILTER_LINEAR |
		    R200_YUV_TO_RGB);
		OUT_RING_REG(R200_REG_PP_TXFORMAT_0, txformat);
		OUT_RING_REG(R200_REG_PP_TXFORMAT_X_0, 0);
		OUT_RING_REG(R200_REG_PP_TXSIZE_0,
		    (pPixmap->drawable.width - 1) |
		    ((pPixmap->drawable.height - 1) << RADEON_TEX_VSIZE_SHIFT));
		OUT_RING_REG(R200_REG_PP_TXPITCH_0, pPortPriv->src_pitch - 32);

		OUT_REG(R200_PP_TXOFFSET_0, pPortPriv->src_offset);

		OUT_RING(DMA_PACKET0(R200_REG_PP_TXCBLEND_0, 4));
		OUT_RING_REG(R200_REG_PP_TXCBLEND_0,
		    R200_TXC_ARG_A_ZERO |
		    R200_TXC_ARG_B_ZERO |
		    R200_TXC_ARG_C_R0_COLOR |
		    R200_TXC_OP_MADD);
		OUT_RING_REG(R200_REG_PP_TXCBLEND2_0,
		    R200_TXC_CLAMP_0_1 | R200_TXC_OUTPUT_REG_R0);
		OUT_RING_REG(R200_REG_PP_TXABLEND_0,
		    R200_TXA_ARG_A_ZERO |
		    R200_TXA_ARG_B_ZERO |
		    R200_TXA_ARG_C_R0_ALPHA |
		    R200_TXA_OP_MADD);
		OUT_RING_REG(R200_REG_PP_TXABLEND2_0,
		    R200_TXA_CLAMP_0_1 | R200_TXA_OUTPUT_REG_R0);

		END_DMA();
	} else {
//		BEGIN_DMA(11);
		BEGIN_DMA(9);

		OUT_RING(DMA_PACKET0(RADEON_REG_PP_TXFILTER_0, 5));
		OUT_RING_REG(RADEON_REG_PP_TXFILTER_0, RADEON_MAG_FILTER_LINEAR |
		    RADEON_MIN_FILTER_LINEAR |
		    RADEON_YUV_TO_RGB);
		OUT_RING_REG(RADEON_REG_PP_TXFORMAT_0, txformat);
		OUT_RING_REG(RADEON_REG_PP_TXOFFSET_0, pPortPriv->src_offset);
		OUT_RING_REG(RADEON_REG_PP_TXCBLEND_0,
		    RADEON_COLOR_ARG_A_ZERO |
		    RADEON_COLOR_ARG_B_ZERO |
		    RADEON_COLOR_ARG_C_T0_COLOR |
		    RADEON_BLEND_CTL_ADD |
		    RADEON_CLAMP_TX);
		OUT_RING_REG(RADEON_REG_PP_TXABLEND_0,
		    RADEON_ALPHA_ARG_A_ZERO |
		    RADEON_ALPHA_ARG_B_ZERO |
		    RADEON_ALPHA_ARG_C_T0_ALPHA |
		    RADEON_BLEND_CTL_ADD |
		    RADEON_CLAMP_TX);

		OUT_RING(DMA_PACKET0(RADEON_REG_PP_TEX_SIZE_0, 2));
		OUT_RING_REG(RADEON_REG_PP_TEX_SIZE_0,
		    (pPixmap->drawable.width - 1) |
		    ((pPixmap->drawable.height - 1) << RADEON_TEX_VSIZE_SHIFT));
		OUT_RING_REG(RADEON_REG_PP_TEX_PITCH_0,
		    pPortPriv->src_pitch - 32);

//	        OUT_RING_REG(ATI_REG_WAIT_UNTIL, ATI_WAIT_CRTC_VLINE);

		END_DMA();
	}

	while (nBox--) {
		float srcX, srcY, dstX, dstY, srcw, srch, dstw, dsth;

		dstX = pBox->x1 + dstxoff;
		dstY = pBox->y1 + dstyoff;
		dstw = pBox->x2 - pBox->x1;
		dsth = pBox->y2 - pBox->y1;
		srcX = (pBox->x1 - pPortPriv->dst_x1) *
		    pPortPriv->src_w / pPortPriv->dst_w;
		srcY = (pBox->y1 - pPortPriv->dst_y1) *
		    pPortPriv->src_h / pPortPriv->dst_h;
		srcw = pPortPriv->src_w * (dstw / pPortPriv->dst_w);
		srch = pPortPriv->src_h * (dsth / pPortPriv->dst_h);

		/*
		 * rectangle:
		 *
		 *  +---------2
		 *  |         |
		 *  |         |
		 *  0---------1
		 */
		
		vtx[0].x.f = dstX;
		vtx[0].y.f = dstY + dsth;
		vtx[0].s0.f = srcX;
		vtx[0].t0.f = srcY + srch;

		vtx[1].x.f = dstX + dstw;
		vtx[1].y.f = dstY + dsth;
		vtx[1].s0.f = srcX + srcw;
		vtx[1].t0.f = srcY + srch;

		vtx[2].x.f = dstX + dstw;
		vtx[2].y.f = dstY;
		vtx[2].s0.f = srcX + srcw;
		vtx[2].t0.f = srcY;

		if (atic->is_r100) {
			BEGIN_DMA(3 * VTX_DWORD_COUNT + 3);
			OUT_RING(DMA_PACKET3(RADEON_CP_PACKET3_3D_DRAW_IMMD,
			    3 * VTX_DWORD_COUNT + 2));
			OUT_RING(RADEON_CP_VC_FRMT_XY |
			    RADEON_CP_VC_FRMT_ST0);
			OUT_RING(RADEON_CP_VC_CNTL_PRIM_TYPE_RECT_LIST |
			    RADEON_CP_VC_CNTL_PRIM_WALK_RING |
			    RADEON_CP_VC_CNTL_MAOS_ENABLE |
			    RADEON_CP_VC_CNTL_VTX_FMT_RADEON_MODE |
			    (3 << RADEON_CP_VC_CNTL_NUM_SHIFT));
		} else {
			BEGIN_DMA(3 * VTX_DWORD_COUNT + 2);
			OUT_RING(DMA_PACKET3(R200_CP_PACKET3_3D_DRAW_IMMD_2,
			    3 * VTX_DWORD_COUNT + 1));
			OUT_RING(RADEON_CP_VC_CNTL_PRIM_TYPE_RECT_LIST |
			    RADEON_CP_VC_CNTL_PRIM_WALK_RING |
			    (3 << RADEON_CP_VC_CNTL_NUM_SHIFT));
		}

		VTX_OUT(vtx[0]);
		VTX_OUT(vtx[1]);
		VTX_OUT(vtx[2]);
		END_DMA();

		pBox++;
	}
#ifdef DAMAGEEXT
	/* XXX: Shouldn't this be in kxv.c instead? */
	DamageDamageRegion(pPortPriv->pDraw, &pPortPriv->clip);
#endif
	kaaMarkSync(pScreen);
}

static void
ATIVideoSave(ScreenPtr pScreen, KdOffscreenArea *area)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATIPortPrivPtr pPortPriv = atis->pAdaptor->pPortPrivates[0].ptr;

	if (pPortPriv->off_screen == area)
		pPortPriv->off_screen = 0;
}

static int
ATIPutImage(KdScreenInfo *screen, DrawablePtr pDraw,
	       short src_x, short src_y,
	       short drw_x, short drw_y,
	       short src_w, short src_h,
	       short drw_w, short drw_h,
	       int id,
	       unsigned char *buf,
	       short width,
	       short height,
	       Bool sync,
	       RegionPtr clipBoxes,
	       pointer data)
{
	ScreenPtr pScreen = screen->pScreen;
	KdScreenPriv(pScreen);
	ATICardInfo(pScreenPriv);
	ATIScreenInfo(pScreenPriv);
	ATIPortPrivPtr pPortPriv = (ATIPortPrivPtr)data;
	char *mmio = atic->reg_base;
	INT32 x1, x2, y1, y2;
	int randr = RR_Rotate_0 /* XXX */;
	int srcPitch, srcPitch2, dstPitch;
	int top, left, npixels, nlines, size;
	BoxRec dstBox;
	int dst_width = width, dst_height = height;
	int rot_x1, rot_y1, rot_x2, rot_y2;
	int dst_x1, dst_y1, dst_x2, dst_y2;
	int rot_src_w, rot_src_h, rot_drw_w, rot_drw_h;

	/* Clip */
	x1 = src_x;
	x2 = src_x + src_w;
	y1 = src_y;
	y2 = src_y + src_h;

	dstBox.x1 = drw_x;
	dstBox.x2 = drw_x + drw_w;
	dstBox.y1 = drw_y;
	dstBox.y2 = drw_y + drw_h;

	ATIClipVideo(&dstBox, &x1, &x2, &y1, &y2,
	    REGION_EXTENTS(pScreen, clipBoxes), width, height);

	src_w = (x2 - x1) >> 16;
	src_h = (y2 - y1) >> 16;
	drw_w = dstBox.x2 - dstBox.x1;
	drw_h = dstBox.y2 - dstBox.y1;

	if ((x1 >= x2) || (y1 >= y2))
		return Success;

	if (mmio == NULL)
		return BadAlloc;

	if (randr & (RR_Rotate_0|RR_Rotate_180)) {
		dst_width = width;
		dst_height = height;
		rot_src_w = src_w;
		rot_src_h = src_h;
		rot_drw_w = drw_w;
		rot_drw_h = drw_h;
	} else {
		dst_width = height;
		dst_height = width;
		rot_src_w = src_h;
		rot_src_h = src_w;
		rot_drw_w = drw_h;
		rot_drw_h = drw_w;
	}

	switch (randr & RR_Rotate_All) {
	case RR_Rotate_0:
	default:
		dst_x1 = dstBox.x1;
		dst_y1 = dstBox.y1;
		dst_x2 = dstBox.x2;
		dst_y2 = dstBox.y2;
		rot_x1 = x1;
		rot_y1 = y1;
		rot_x2 = x2;
		rot_y2 = y2;
		break;
	case RR_Rotate_90:
		dst_x1 = dstBox.y1;
		dst_y1 = screen->height - dstBox.x2;
		dst_x2 = dstBox.y2;
		dst_y2 = screen->height - dstBox.x1;
		rot_x1 = y1;
		rot_y1 = (src_w << 16) - x2;
		rot_x2 = y2;
		rot_y2 = (src_w << 16) - x1;
		break;
	case RR_Rotate_180:
		dst_x1 = screen->width - dstBox.x2;
		dst_y1 = screen->height - dstBox.y2;
		dst_x2 = screen->width - dstBox.x1;
		dst_y2 = screen->height - dstBox.y1;
		rot_x1 = (src_w << 16) - x2;
		rot_y1 = (src_h << 16) - y2;
		rot_x2 = (src_w << 16) - x1;
		rot_y2 = (src_h << 16) - y1;
		break;
	case RR_Rotate_270:
		dst_x1 = screen->width - dstBox.y2;
		dst_y1 = dstBox.x1;
		dst_x2 = screen->width - dstBox.y1;
		dst_y2 = dstBox.x2;
		rot_x1 = (src_h << 16) - y2;
		rot_y1 = x1;
		rot_x2 = (src_h << 16) - y1;
		rot_y2 = x2;
		break;
	}

	switch(id) {
	case FOURCC_YV12:
	case FOURCC_I420:
		dstPitch = ((dst_width << 1) + 15) & ~15;
		srcPitch = (width + 3) & ~3;
		srcPitch2 = ((width >> 1) + 3) & ~3;
		size = dstPitch * dst_height;
		break;
	case FOURCC_UYVY:
	case FOURCC_YUY2:
	default:
		dstPitch = ((dst_width << 1) + 15) & ~15;
		srcPitch = (width << 1);
		srcPitch2 = 0;
		size = dstPitch * dst_height;
		break;
	}

	if (pPortPriv->off_screen != NULL && size != pPortPriv->size) {
		KdOffscreenFree(screen->pScreen, pPortPriv->off_screen);
		pPortPriv->off_screen = 0;
	}

	if (pPortPriv->off_screen == NULL) {
		pPortPriv->off_screen = KdOffscreenAlloc(screen->pScreen,
		    size * 2, 64, TRUE, ATIVideoSave, pPortPriv);
		if (pPortPriv->off_screen == NULL)
			return BadAlloc;
	}


	if (pDraw->type == DRAWABLE_WINDOW)
		pPortPriv->pPixmap =
		    (*pScreen->GetWindowPixmap)((WindowPtr)pDraw);
	else
		pPortPriv->pPixmap = (PixmapPtr)pDraw;

	/* Migrate the pixmap to offscreen if necessary. */
	if (!kaaPixmapIsOffscreen(pPortPriv->pPixmap))
		kaaMoveInPixmap(pPortPriv->pPixmap);

	if (!kaaPixmapIsOffscreen(pPortPriv->pPixmap)) {
		return BadAlloc;
	}

	pPortPriv->src_offset = pPortPriv->off_screen->offset;
	pPortPriv->src_addr = (CARD8 *)(pScreenPriv->screen->memory_base +
	    pPortPriv->src_offset);
	pPortPriv->src_pitch = dstPitch;
	pPortPriv->size = size;
	pPortPriv->pDraw = pDraw;

	/* copy data */
	top = rot_y1 >> 16;
	left = (rot_x1 >> 16) & ~1;
	npixels = ((((rot_x2 + 0xffff) >> 16) + 1) & ~1) - left;

	/* Since we're probably overwriting the area that might still be used
	 * for the last PutImage request, wait for idle.
	 */
	ATIWaitIdle(atis);

	switch(id) {
	case FOURCC_YV12:
	case FOURCC_I420:
		top &= ~1;
		nlines = ((((rot_y2 + 0xffff) >> 16) + 1) & ~1) - top;
		KdXVCopyPlanarData(screen, buf, pPortPriv->src_addr, randr,
		    srcPitch, srcPitch2, dstPitch, rot_src_w, rot_src_h,
		    height, top, left, nlines, npixels, id);
		break;
	case FOURCC_UYVY:
	case FOURCC_YUY2:
	default:
		nlines = ((rot_y2 + 0xffff) >> 16) - top;
		KdXVCopyPackedData(screen, buf, pPortPriv->src_addr, randr,
		    srcPitch, dstPitch, rot_src_w, rot_src_h, top, left,
		    nlines, npixels);
		break;
	}

	/* update cliplist */
	if (!REGION_EQUAL(screen->pScreen, &pPortPriv->clip, clipBoxes)) {
		REGION_COPY(screen->pScreen, &pPortPriv->clip, clipBoxes);
	}

	pPortPriv->id = id;
	pPortPriv->src_x1 = rot_x1;
	pPortPriv->src_y1 = rot_y1;
	pPortPriv->src_x2 = rot_x2;
	pPortPriv->src_y2 = rot_y2;
	pPortPriv->src_w = rot_src_w;
	pPortPriv->src_h = rot_src_h;
	pPortPriv->dst_x1 = dst_x1;
	pPortPriv->dst_y1 = dst_y1;
	pPortPriv->dst_x2 = dst_x2;
	pPortPriv->dst_y2 = dst_y2;
	pPortPriv->dst_w = rot_drw_w;
	pPortPriv->dst_h = rot_drw_h;

	if (atic->is_radeon)
		RadeonDisplayVideo(screen, pPortPriv);
	else
		R128DisplayVideo(screen, pPortPriv);

	return Success;
}

static int
ATIReputImage(KdScreenInfo *screen, DrawablePtr pDraw, short drw_x, short drw_y,
    RegionPtr clipBoxes, pointer data)
{
	ScreenPtr pScreen = screen->pScreen;
	KdScreenPriv(pScreen);
	ATICardInfo(pScreenPriv);
	ATIPortPrivPtr	pPortPriv = (ATIPortPrivPtr)data;
	BoxPtr pOldExtents = REGION_EXTENTS(screen->pScreen, &pPortPriv->clip);
	BoxPtr pNewExtents = REGION_EXTENTS(screen->pScreen, clipBoxes);

	if (pOldExtents->x1 != pNewExtents->x1 ||
	    pOldExtents->x2 != pNewExtents->x2 ||
	    pOldExtents->y1 != pNewExtents->y1 ||
	    pOldExtents->y2 != pNewExtents->y2)
		return BadMatch;

	if (pDraw->type == DRAWABLE_WINDOW)
		pPortPriv->pPixmap =
		    (*pScreen->GetWindowPixmap)((WindowPtr)pDraw);
	else
		pPortPriv->pPixmap = (PixmapPtr)pDraw;

	if (!kaaPixmapIsOffscreen(pPortPriv->pPixmap))
		kaaMoveInPixmap(pPortPriv->pPixmap);

	if (!kaaPixmapIsOffscreen(pPortPriv->pPixmap)) {
		ErrorF("err\n");
		return BadAlloc;
	}


	/* update cliplist */
	if (!REGION_EQUAL(screen->pScreen, &pPortPriv->clip, clipBoxes))
		REGION_COPY(screen->pScreen, &pPortPriv->clip, clipBoxes);

	/* XXX: What do the drw_x and drw_y here mean for us? */

	if (atic->is_radeon)
		RadeonDisplayVideo(screen, pPortPriv);
	else
		R128DisplayVideo(screen, pPortPriv);

	return Success;
}

static int
ATIQueryImageAttributes(KdScreenInfo *screen, int id, unsigned short *w,
    unsigned short *h, int *pitches, int *offsets)
{
	int size, tmp;

	if (*w > IMAGE_MAX_WIDTH) 
		*w = IMAGE_MAX_WIDTH;
	if (*h > IMAGE_MAX_HEIGHT) 
		*h = IMAGE_MAX_HEIGHT;

	*w = (*w + 1) & ~1;
	if (offsets)
		offsets[0] = 0;

	switch (id)
	{
	case FOURCC_YV12:
	case FOURCC_I420:
		*h = (*h + 1) & ~1;
		size = (*w + 3) & ~3;
		if (pitches) 
			pitches[0] = size;
		size *= *h;
		if (offsets) 
			offsets[1] = size;
		tmp = ((*w >> 1) + 3) & ~3;
		if (pitches) 
			pitches[1] = pitches[2] = tmp;
		tmp *= (*h >> 1);
		size += tmp;
		if (offsets) 
			offsets[2] = size;
		size += tmp;
		break;
	case FOURCC_UYVY:
	case FOURCC_YUY2:
	default:
		size = *w << 1;
		if (pitches) 
			pitches[0] = size;
		size *= *h;
		break;
	}

	return size;
}


/* client libraries expect an encoding */
static KdVideoEncodingRec DummyEncoding[1] =
{
	{
		0,
		"XV_IMAGE",
		IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT,
		{1, 1}
	}
};

#define NUM_FORMATS 3

static KdVideoFormatRec Formats[NUM_FORMATS] = 
{
	{15, TrueColor}, {16, TrueColor}, {24, TrueColor}
};

#define NUM_ATTRIBUTES 0

static KdAttributeRec Attributes[NUM_ATTRIBUTES] =
{
};

#define NUM_IMAGES 4

static KdImageRec Images[NUM_IMAGES] =
{
	XVIMAGE_YUY2,
	XVIMAGE_YV12,
	XVIMAGE_I420,
	XVIMAGE_UYVY
};

static KdVideoAdaptorPtr 
ATISetupImageVideo(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	KdVideoAdaptorPtr adapt;
	ATIPortPrivPtr pPortPriv;
	int i;

	atis->num_texture_ports = 16;

	adapt = xcalloc(1, sizeof(KdVideoAdaptorRec) + atis->num_texture_ports *
	    (sizeof(ATIPortPrivRec) + sizeof(DevUnion)));
	if (adapt == NULL)
		return NULL;

	adapt->type = XvWindowMask | XvInputMask | XvImageMask;
	adapt->flags = VIDEO_CLIP_TO_VIEWPORT;
	adapt->name = "ATI Texture Video";
	adapt->nEncodings = 1;
	adapt->pEncodings = DummyEncoding;
	adapt->nFormats = NUM_FORMATS;
	adapt->pFormats = Formats;
	adapt->nPorts = atis->num_texture_ports;
	adapt->pPortPrivates = (DevUnion*)(&adapt[1]);

	pPortPriv =
	    (ATIPortPrivPtr)(&adapt->pPortPrivates[atis->num_texture_ports]);

	for (i = 0; i < atis->num_texture_ports; i++)
		adapt->pPortPrivates[i].ptr = &pPortPriv[i];

	adapt->nAttributes = NUM_ATTRIBUTES;
	adapt->pAttributes = Attributes;
	adapt->pImages = Images;
	adapt->nImages = NUM_IMAGES;
	adapt->PutVideo = NULL;
	adapt->PutStill = NULL;
	adapt->GetVideo = NULL;
	adapt->GetStill = NULL;
	adapt->StopVideo = ATIStopVideo;
	adapt->SetPortAttribute = ATISetPortAttribute;
	adapt->GetPortAttribute = ATIGetPortAttribute;
	adapt->QueryBestSize = ATIQueryBestSize;
	adapt->PutImage = ATIPutImage;
	adapt->ReputImage = ATIReputImage;
	adapt->QueryImageAttributes = ATIQueryImageAttributes;

	/* gotta uninit this someplace */
	REGION_INIT(pScreen, &pPortPriv->clip, NullBox, 0); 

	atis->pAdaptor = adapt;

	xvBrightness = MAKE_ATOM("XV_BRIGHTNESS");
	xvSaturation = MAKE_ATOM("XV_SATURATION");

	return adapt;
}

Bool ATIInitVideo(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	KdScreenInfo *screen = pScreenPriv->screen;
	KdVideoAdaptorPtr *adaptors, *newAdaptors = NULL;
	KdVideoAdaptorPtr newAdaptor = NULL;
	int num_adaptors;
    
	atis->pAdaptor = NULL;

	if (atic->reg_base == NULL)
		return FALSE;
	if (atic->is_r300)
		return FALSE;

	num_adaptors = KdXVListGenericAdaptors(screen, &adaptors);

	newAdaptor = ATISetupImageVideo(pScreen);

	if (newAdaptor)  {
		if (!num_adaptors) {
			num_adaptors = 1;
			adaptors = &newAdaptor;
		} else {
			newAdaptors = xalloc((num_adaptors + 1) * 
			    sizeof(KdVideoAdaptorPtr *));
			if (newAdaptors) {
				memcpy(newAdaptors, adaptors, num_adaptors *
				    sizeof(KdVideoAdaptorPtr));
				newAdaptors[num_adaptors] = newAdaptor;
				adaptors = newAdaptors;
				num_adaptors++;
			}
		}
	}

	if (num_adaptors)
		KdXVScreenInit(pScreen, adaptors, num_adaptors);

	if (newAdaptors)
		xfree(newAdaptors);

	return TRUE;
}

void
ATIFiniVideo(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	KdVideoAdaptorPtr adapt = atis->pAdaptor;
	ATIPortPrivPtr pPortPriv;
	int i;

	if (!adapt)
		return;

	for (i = 0; i < atis->num_texture_ports; i++) {
		pPortPriv = (ATIPortPrivPtr)(&adapt->pPortPrivates[i].ptr);
		REGION_UNINIT(pScreen, &pPortPriv->clip);
	}
	xfree(adapt);
	atis->pAdaptor = NULL;
}
