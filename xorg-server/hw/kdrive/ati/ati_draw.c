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
#include "ati.h"
#include "ati_reg.h"
#include "ati_dma.h"
#include "ati_draw.h"
#include "kaa.h"

CARD8 ATISolidRop[16] = {
    /* GXclear      */      0x00,         /* 0 */
    /* GXand        */      0xa0,         /* src AND dst */
    /* GXandReverse */      0x50,         /* src AND NOT dst */
    /* GXcopy       */      0xf0,         /* src */
    /* GXandInverted*/      0x0a,         /* NOT src AND dst */
    /* GXnoop       */      0xaa,         /* dst */
    /* GXxor        */      0x5a,         /* src XOR dst */
    /* GXor         */      0xfa,         /* src OR dst */
    /* GXnor        */      0x05,         /* NOT src AND NOT dst */
    /* GXequiv      */      0xa5,         /* NOT src XOR dst */
    /* GXinvert     */      0x55,         /* NOT dst */
    /* GXorReverse  */      0xf5,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x0f,         /* NOT src */
    /* GXorInverted */      0xaf,         /* NOT src OR dst */
    /* GXnand       */      0x5f,         /* NOT src OR NOT dst */
    /* GXset        */      0xff,         /* 1 */
};

CARD8 ATIBltRop[16] = {
    /* GXclear      */      0x00,         /* 0 */
    /* GXand        */      0x88,         /* src AND dst */
    /* GXandReverse */      0x44,         /* src AND NOT dst */
    /* GXcopy       */      0xcc,         /* src */
    /* GXandInverted*/      0x22,         /* NOT src AND dst */
    /* GXnoop       */      0xaa,         /* dst */
    /* GXxor        */      0x66,         /* src XOR dst */
    /* GXor         */      0xee,         /* src OR dst */
    /* GXnor        */      0x11,         /* NOT src AND NOT dst */
    /* GXequiv      */      0x99,         /* NOT src XOR dst */
    /* GXinvert     */      0x55,         /* NOT dst */
    /* GXorReverse  */      0xdd,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x33,         /* NOT src */
    /* GXorInverted */      0xbb,         /* NOT src OR dst */
    /* GXnand       */      0x77,         /* NOT src OR NOT dst */
    /* GXset        */      0xff,         /* 1 */
};

int copydx, copydy;
ATIScreenInfo *accel_atis;
/* If is_24bpp is set, then we are using the accelerator in 8-bit mode due
 * to it being broken for 24bpp, so coordinates have to be multiplied by 3.
 */
Bool is_24bpp;
CARD32 settings, color, src_pitch_offset, dst_pitch_offset;

int sample_count;
float sample_offsets_x[255];
float sample_offsets_y[255];

#define DRAW_USING_PACKET3 0

void
ATIDrawSetup(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	RING_LOCALS;

	/* XXX: this shouldn't be necessary, but fixes some R128 composite
	 * issues.
	 */
	/*if (!atic->is_radeon) {
		char *mmio = atic->reg_base;
		ATIWaitIdle(atis);
		MMIO_OUT32(mmio, R128_REG_PC_GUI_MODE,
		    R128_PC_BYPASS_EN);
	}*/

	BEGIN_DMA(2);
	OUT_REG(ATI_REG_DEFAULT_SC_BOTTOM_RIGHT,
	    ATI_DEFAULT_SC_RIGHT_MAX | ATI_DEFAULT_SC_BOTTOM_MAX);
	END_DMA();

	if (!atic->is_radeon) {
		/* Setup for R128 Composite */
		BEGIN_DMA(12);
		OUT_REG(R128_REG_SCALE_3D_CNTL,
		    R128_SCALE_3D_TEXMAP_SHADE |
		    R128_SCALE_PIX_REPLICATE |
		    R128_TEX_CACHE_SPLIT |
		    R128_TEX_MAP_ALPHA_IN_TEXTURE |
		    R128_TEX_CACHE_LINE_SIZE_4QW);
		OUT_REG(R128_REG_SETUP_CNTL,
		    R128_COLOR_SOLID_COLOR |
		    R128_PRIM_TYPE_TRI |
		    R128_TEXTURE_ST_MULT_W |
		    R128_STARTING_VERTEX_1 |
		    R128_ENDING_VERTEX_3 |
		    R128_SUB_PIX_4BITS);
		OUT_REG(R128_REG_PM4_VC_FPU_SETUP,
		    R128_FRONT_DIR_CCW |
		    R128_BACKFACE_CULL |
		    R128_FRONTFACE_SOLID |
		    R128_FPU_COLOR_SOLID |
		    R128_FPU_SUB_PIX_4BITS |
		    R128_FPU_MODE_3D |
		    R128_TRAP_BITS_DISABLE |
		    R128_XFACTOR_2 |
		    R128_YFACTOR_2 |
		    R128_FLAT_SHADE_VERTEX_OGL |
		    R128_FPU_ROUND_TRUNCATE |
		    R128_WM_SEL_8DW);
		OUT_REG(R128_REG_PLANE_3D_MASK_C, 0xffffffff);
		OUT_REG(R128_REG_CONSTANT_COLOR_C, 0xff000000);
		OUT_REG(R128_REG_WINDOW_XY_OFFSET, 0x00000000);
		END_DMA();
	} else if (!atic->is_r300) {
		/* Setup for R100/R200 Composite */
		BEGIN_DMA(8);
		OUT_REG(RADEON_REG_RE_TOP_LEFT, 0);
		OUT_REG(RADEON_REG_RE_WIDTH_HEIGHT, 0xffffffff);
		OUT_REG(RADEON_REG_RB3D_PLANEMASK, 0xffffffff);
		OUT_REG(RADEON_REG_SE_CNTL,
		    RADEON_FFACE_CULL_CCW |
		    RADEON_FFACE_SOLID |
		    RADEON_VTX_PIX_CENTER_OGL);
		END_DMA();

		if (atic->is_r100) {
			BEGIN_DMA(6);
			OUT_REG(RADEON_REG_SE_CNTL_STATUS, RADEON_TCL_BYPASS);
			OUT_REG(RADEON_REG_SE_COORD_FMT,
			    RADEON_VTX_XY_PRE_MULT_1_OVER_W0 |
			    RADEON_VTX_ST0_NONPARAMETRIC |
			    RADEON_VTX_ST1_NONPARAMETRIC |
			    RADEON_TEX1_W_ROUTING_USE_W0);
			OUT_REG(RADEON_REG_RB3D_DSTCACHE_MODE, 
				RADEON_RB3D_DC_2D_CACHE_AUTOFLUSH |
				RADEON_RB3D_DC_3D_CACHE_AUTOFLUSH);
			END_DMA();
		} else {
			BEGIN_DMA(18);
			/* XXX: The 0 below should be RADEON_TCL_BYPASS on
			 * RS300s.
			 */
			OUT_REG(R200_REG_SE_VAP_CNTL_STATUS, 0);
			OUT_REG(R200_REG_PP_CNTL_X, 0);
			OUT_REG(R200_REG_PP_TXMULTI_CTL_0, 0);
			OUT_REG(R200_REG_SE_VTX_STATE_CNTL, 0);
			OUT_REG(R200_REG_RE_CNTL, 0);
			/* XXX: VTX_ST_DENORMALIZED is illegal for the case of
			 * repeating textures.
			 */
			OUT_REG(R200_REG_SE_VTE_CNTL, R200_VTX_ST_DENORMALIZED);
			OUT_REG(R200_REG_SE_VAP_CNTL,
			    R200_VAP_FORCE_W_TO_ONE |
			    R200_VAP_VF_MAX_VTX_NUM);
			OUT_REG(R200_REG_RE_AUX_SCISSOR_CNTL, 0);
			OUT_REG(RADEON_REG_RB3D_DSTCACHE_MODE, 
				RADEON_RB3D_DC_2D_CACHE_AUTOFLUSH |
				RADEON_RB3D_DC_3D_CACHE_AUTOFLUSH |
				R200_RB3D_DC_2D_CACHE_AUTOFREE |
				R200_RB3D_DC_3D_CACHE_AUTOFREE);
			END_DMA();
		}
	}
}

static void
ATIWaitMarker(ScreenPtr pScreen, int marker)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);

	ENTER_DRAW(0);
	ATIWaitIdle(atis);
	LEAVE_DRAW(0);
}

void
RadeonSwitchTo2D(ATIScreenInfo *atis)
{
	RING_LOCALS;

	ENTER_DRAW(0);
	BEGIN_DMA(4);
	OUT_REG(RADEON_REG_RB3D_DSTCACHE_CTLSTAT, RADEON_RB3D_DC_FLUSH);
	OUT_REG(ATI_REG_WAIT_UNTIL,
	    RADEON_WAIT_HOST_IDLECLEAN | RADEON_WAIT_3D_IDLECLEAN);
	END_DMA();
	LEAVE_DRAW(0);
}

void
RadeonSwitchTo3D(ATIScreenInfo *atis)
{
	RING_LOCALS;

	ENTER_DRAW(0);
	BEGIN_DMA(4);
	OUT_REG(RADEON_REG_RB3D_DSTCACHE_CTLSTAT, RADEON_RB3D_DC_FLUSH);
	/* We must wait for 3d to idle, in case source was just written as a dest. */
	OUT_REG(ATI_REG_WAIT_UNTIL,
	    RADEON_WAIT_HOST_IDLECLEAN | RADEON_WAIT_2D_IDLECLEAN | RADEON_WAIT_3D_IDLECLEAN);
	END_DMA();
	LEAVE_DRAW(0);
}

#if ATI_TRACE_DRAW
void
ATIEnterDraw (PixmapPtr pPix, char *function)
{
    if (pPix != NULL) {
	KdScreenPriv(pPix->drawable.pScreen);
	CARD32 offset;
    
	offset = ((CARD8 *)pPix->devPrivate.ptr -
		  pScreenPriv->screen->memory_base);
    
	ErrorF ("Enter %s 0x%x (%dx%dx%d/%d)\n", function, offset,
	    pPix->drawable.width, pPix->drawable.height, pPix->drawable.depth,
	    pPix->drawable.bitsPerPixel);
    } else
	ErrorF ("Enter %s\n", function);
}

void
ATILeaveDraw (PixmapPtr pPix, char *function)
{
    if (pPix != NULL) {
	KdScreenPriv(pPix->drawable.pScreen);
	CARD32 offset;
    
	offset = ((CARD8 *)pPix->devPrivate.ptr -
		  pScreenPriv->screen->memory_base);
    
	ErrorF ("Leave %s 0x%x\n", function, offset);
    } else
	ErrorF ("Leave %s\n", function);
}
#endif

/* Assumes that depth 15 and 16 can be used as depth 16, which is okay since we
 * require src and dest datatypes to be equal.
 */
static Bool
ATIGetDatatypeBpp(int bpp, CARD32 *type)
{
	switch (bpp) {
	case 8:
		*type = R128_DATATYPE_CI8;
		return TRUE;
	case 16:
		*type = R128_DATATYPE_RGB565;
		return TRUE;
	case 24:
		*type = R128_DATATYPE_CI8;
		return TRUE;
	case 32:
		*type = R128_DATATYPE_ARGB8888;
		return TRUE;
	default:
		ATI_FALLBACK(("Unsupported bpp: %d\n", bpp));
		return FALSE;
	}
}

Bool
ATIGetOffsetPitch(ATIScreenInfo *atis, int bpp, CARD32 *pitch_offset,
    int offset, int pitch)
{
	ATICardInfo *atic = atis->atic;

	/* On the R128, depending on the bpp the screen can be set up so that it
	 * doesn't meet the pitchAlign requirement but can still be
	 * accelerated, so we check the specific pitch requirement of alignment
	 * to 8 pixels.
	 */
	if (atic->is_radeon) {
		if (pitch % atis->kaa.pitchAlign != 0)
			ATI_FALLBACK(("Bad pitch 0x%08x\n", pitch));
		*pitch_offset = ((pitch >> 6) << 22) | (offset >> 10);

	} else {
		if (pitch % bpp != 0)
			ATI_FALLBACK(("Bad pitch 0x%08x\n", pitch));
		*pitch_offset = ((pitch / bpp) << 21) | (offset >> 5);
	}

	if (offset % atis->kaa.offsetAlign != 0)
		ATI_FALLBACK(("Bad offset 0x%08x\n", offset));

	return TRUE;
}

Bool
ATIGetPixmapOffsetPitch(PixmapPtr pPix, CARD32 *pitch_offset)
{
	KdScreenPriv(pPix->drawable.pScreen);
	ATIScreenInfo(pScreenPriv);
	CARD32 pitch, offset;
	int bpp;

	bpp = pPix->drawable.bitsPerPixel;
	if (bpp == 24)
		bpp = 8;

	offset = ((CARD8 *)pPix->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);
	pitch = pPix->devKind;

	return ATIGetOffsetPitch(atis, bpp, pitch_offset, offset, pitch);
}

static Bool
ATIPrepareSolid(PixmapPtr pPix, int alu, Pixel pm, Pixel fg)
{
	KdScreenPriv(pPix->drawable.pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	CARD32 datatype;
	RING_LOCALS;

	is_24bpp = (pPix->drawable.bitsPerPixel == 24);
	accel_atis = atis;

	if (is_24bpp) {
		/* Solid fills in fake-24bpp mode only work if the pixel color
		 * and planemask are all the same byte.
		 */
		if ((fg & 0xffffff) != (((fg & 0xff) << 16) | ((fg >> 8) &
		    0xffff)))
			ATI_FALLBACK(("Can't do solid color 0x%08x in 24bpp\n",
			    fg));
		if ((pm & 0xffffff) != (((pm & 0xff) << 16) | ((pm >> 8) &
		    0xffff)))
			ATI_FALLBACK(("Can't do planemask 0x%08x in 24bpp\n",
			    pm));
	}

	if (!ATIGetDatatypeBpp(pPix->drawable.bitsPerPixel, &datatype))
		return FALSE;
	if (!ATIGetPixmapOffsetPitch(pPix, &dst_pitch_offset))
		return FALSE;

	ENTER_DRAW(pPix);

	if (atic->is_radeon)
		RadeonSwitchTo2D(atis);

	settings =
	    ATI_GMC_DST_PITCH_OFFSET_CNTL |
	    ATI_GMC_BRUSH_SOLID_COLOR |
	    (datatype << 8) |
	    ATI_GMC_SRC_DATATYPE_COLOR |
	    (ATISolidRop[alu] << 16) |
	    ATI_GMC_CLR_CMP_CNTL_DIS |
	    R128_GMC_AUX_CLIP_DIS;
	color = fg;

#if DRAW_USING_PACKET3
	BEGIN_DMA(6);
	OUT_REG(ATI_REG_DEFAULT_SC_BOTTOM_RIGHT,
	    ATI_DEFAULT_SC_RIGHT_MAX | ATI_DEFAULT_SC_BOTTOM_MAX);
	OUT_REG(ATI_REG_DP_WRITE_MASK, pm);
	OUT_REG(ATI_REG_DP_CNTL, ATI_DST_X_LEFT_TO_RIGHT |
	    ATI_DST_Y_TOP_TO_BOTTOM);
	END_DMA();
#else
	BEGIN_DMA(12);
	OUT_REG(ATI_REG_DEFAULT_SC_BOTTOM_RIGHT,
	    ATI_DEFAULT_SC_RIGHT_MAX | ATI_DEFAULT_SC_BOTTOM_MAX);
	OUT_REG(ATI_REG_DST_PITCH_OFFSET, dst_pitch_offset);
	OUT_REG(ATI_REG_DP_GUI_MASTER_CNTL, settings);
	OUT_REG(ATI_REG_DP_BRUSH_FRGD_CLR, fg);
	OUT_REG(ATI_REG_DP_WRITE_MASK, pm);
	OUT_REG(ATI_REG_DP_CNTL, ATI_DST_X_LEFT_TO_RIGHT |
	    ATI_DST_Y_TOP_TO_BOTTOM);
	END_DMA();
#endif

	LEAVE_DRAW(pPix);
	return TRUE;
}

static void
ATISolid(int x1, int y1, int x2, int y2)
{
	ENTER_DRAW(0);
	ATIScreenInfo *atis = accel_atis;
	RING_LOCALS;
	
	if (is_24bpp) {
		x1 *= 3;
		x2 *= 3;
	}
#if DRAW_USING_PACKET3
	BEGIN_DMA(6);
	OUT_RING(DMA_PACKET3(ATI_CCE_PACKET3_PAINT_MULTI, 5));
	OUT_RING(settings);
	OUT_RING(dst_pitch_offset);
	OUT_RING(color);
	OUT_RING((x1 << 16) | y1);
	OUT_RING(((x2 - x1) << 16) | (y2 - y1));
	END_DMA();
#else
	BEGIN_DMA(3);
	OUT_RING(DMA_PACKET0(ATI_REG_DST_Y_X, 2));
	OUT_RING_REG(ATI_REG_DST_Y_X, (y1 << 16) | x1);
	OUT_RING_REG(ATI_REG_DST_HEIGHT_WIDTH, ((y2 - y1) << 16) | (x2 - x1));
	END_DMA();
#endif
	LEAVE_DRAW(0);
}

static void
ATIDoneSolid(void)
{
	ENTER_DRAW(0);
	LEAVE_DRAW(0);
}

static Bool
ATIPrepareCopy(PixmapPtr pSrc, PixmapPtr pDst, int dx, int dy, int alu, Pixel pm)
{
	KdScreenPriv(pDst->drawable.pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	CARD32 datatype;
	RING_LOCALS;

	copydx = dx;
	copydy = dy;
	is_24bpp = pDst->drawable.bitsPerPixel == 24;
	accel_atis = atis;

	if (is_24bpp && ((pm & 0xffffff) != (((pm & 0xff) << 16) | ((pm >> 8) &
	    0xffff))))
		ATI_FALLBACK(("Can't do planemask 0x%08x in 24bpp\n", pm));

	if (!ATIGetDatatypeBpp(pDst->drawable.bitsPerPixel, &datatype))
		return FALSE;
	if (!ATIGetPixmapOffsetPitch(pSrc, &src_pitch_offset))
		return FALSE;
	if (!ATIGetPixmapOffsetPitch(pDst, &dst_pitch_offset))
		return FALSE;

	ENTER_DRAW (pDst);
	if (atic->is_radeon)
		RadeonSwitchTo2D(atis);

	settings =
	    ATI_GMC_SRC_PITCH_OFFSET_CNTL |
	    ATI_GMC_DST_PITCH_OFFSET_CNTL |
	    ATI_GMC_BRUSH_NONE |
	    (datatype << 8) |
	    ATI_GMC_SRC_DATATYPE_COLOR |
	    (ATIBltRop[alu] << 16) |
	    ATI_DP_SRC_SOURCE_MEMORY |
	    ATI_GMC_CLR_CMP_CNTL_DIS |
	    R128_GMC_AUX_CLIP_DIS;

#if DRAW_USING_PACKET3
	BEGIN_DMA(6);
	OUT_REG(ATI_REG_DEFAULT_SC_BOTTOM_RIGHT,
	    ATI_DEFAULT_SC_RIGHT_MAX | ATI_DEFAULT_SC_BOTTOM_MAX);
	OUT_REG(ATI_REG_DP_WRITE_MASK, pm);
	OUT_REG(ATI_REG_DP_CNTL,
	    (dx >= 0 ? ATI_DST_X_LEFT_TO_RIGHT : 0) |
	    (dy >= 0 ? ATI_DST_Y_TOP_TO_BOTTOM : 0));
	END_DMA();

#else
	BEGIN_DMA(12);
	OUT_REG(ATI_REG_DEFAULT_SC_BOTTOM_RIGHT,
	    ATI_DEFAULT_SC_RIGHT_MAX | ATI_DEFAULT_SC_BOTTOM_MAX);
	OUT_REG(ATI_REG_SRC_PITCH_OFFSET, src_pitch_offset);
	OUT_REG(ATI_REG_DST_PITCH_OFFSET, dst_pitch_offset);
	OUT_REG(ATI_REG_DP_GUI_MASTER_CNTL, settings);
	OUT_REG(ATI_REG_DP_WRITE_MASK, pm);
	OUT_REG(ATI_REG_DP_CNTL,
	    (dx >= 0 ? ATI_DST_X_LEFT_TO_RIGHT : 0) |
	    (dy >= 0 ? ATI_DST_Y_TOP_TO_BOTTOM : 0));
	END_DMA();
#endif
	LEAVE_DRAW(pDst);

	return TRUE;
}

static void
ATICopy(int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	ATIScreenInfo *atis = accel_atis;
	RING_LOCALS;

	if (is_24bpp) {
		srcX *= 3;
		dstX *= 3;
		w *= 3;
	}

#if !DRAW_USING_PACKET3
	if (copydx < 0) {
		srcX += w - 1;
		dstX += w - 1;
	}

	if (copydy < 0)  {
		srcY += h - 1;
		dstY += h - 1;
	}
#endif

#if DRAW_USING_PACKET3
	BEGIN_DMA(7);
	OUT_RING(DMA_PACKET3(ATI_CCE_PACKET3_BITBLT_MULTI, 6));
	OUT_RING(settings);
	OUT_RING(src_pitch_offset);
	OUT_RING(dst_pitch_offset);
	OUT_RING((srcX << 16) | srcY);
	OUT_RING((dstX << 16) | dstY);
	OUT_RING((w << 16) | h);
	END_DMA();
#else
	BEGIN_DMA(4);
	OUT_RING(DMA_PACKET0(ATI_REG_SRC_Y_X, 3));
	OUT_RING_REG(ATI_REG_SRC_Y_X, (srcY << 16) | srcX);
	OUT_RING_REG(ATI_REG_DST_Y_X, (dstY << 16) | dstX);
	OUT_RING_REG(ATI_REG_DST_HEIGHT_WIDTH, (h << 16) | w);
	END_DMA();
#endif
}

static void
ATIDoneCopy(void)
{
}

static Bool
ATIUploadToScreen(PixmapPtr pDst, char *src, int src_pitch)
{
	ScreenPtr pScreen = pDst->drawable.pScreen;
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	int width, height, bpp, i, dwords;
	int dst_pitch, dst_offset;
	CARD32 dst_pitch_offset, datatype;
	Bool success;
	RING_LOCALS;

	ENTER_DRAW (pDst);

	LEAVE_DRAW (pDst);
	/* XXX: Hostdata uploads aren't working yet. */
	return FALSE;
	
	dst_offset = ((CARD8 *)pDst->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);
	dst_pitch = pDst->devKind;
	width = pDst->drawable.width;
	height = pDst->drawable.height;
	bpp =  pDst->drawable.bitsPerPixel;

	success = ATIGetDatatypeBpp(bpp, &datatype);

	if (bpp == 24) {
		is_24bpp = TRUE;
		bpp = 8;
	} else
		is_24bpp = FALSE;

	if (!ATIGetOffsetPitch(atis, bpp, &dst_pitch_offset, dst_offset,
	    dst_pitch))
		return FALSE;

	if (src_pitch != (width * bpp / 8))
		return FALSE;

	/* No PACKET3 packets when in PIO mode. */
	if (atis->using_pio)
		return FALSE;

	dwords = (width * height * (bpp / 8) + 3) / 4;

	/* Flush pixel cache so nothing being written to the destination
	 * previously gets mixed up with the hostdata blit.
	 */
	if (atic->is_radeon) {
		BEGIN_DMA(4);
		OUT_REG(RADEON_REG_RB3D_DSTCACHE_CTLSTAT, RADEON_RB3D_DC_FLUSH);
		OUT_REG(ATI_REG_WAIT_UNTIL,
		    RADEON_WAIT_2D_IDLECLEAN |
		    RADEON_WAIT_3D_IDLECLEAN |
		    RADEON_WAIT_HOST_IDLECLEAN);
		END_DMA();
	} else {
		BEGIN_DMA(2);
		OUT_REG(R128_REG_PC_GUI_CTLSTAT,
		    R128_PC_FLUSH_GUI | R128_PC_RI_GUI);
		END_DMA();
	}

	BEGIN_DMA(8);
	OUT_RING(DMA_PACKET3(ATI_CCE_PACKET3_HOSTDATA_BLT, 7 + dwords));
	OUT_RING(ATI_GMC_DST_PITCH_OFFSET_CNTL |
	    ATI_GMC_BRUSH_NONE |
	    (datatype << 8) |
	    ATI_GMC_SRC_DATATYPE_COLOR |
	    (ATISolidRop[GXcopy] << 16) |
	    ATI_DP_SRC_SOURCE_HOST_DATA |
	    ATI_GMC_CLR_CMP_CNTL_DIS |
	    R128_GMC_AUX_CLIP_DIS |
	    ATI_GMC_WR_MSK_DIS);
	OUT_RING(dst_pitch_offset);
	OUT_RING(0xffffffff);
	OUT_RING(0xffffffff);
	OUT_RING((0 << 16) | 0);
	OUT_RING((height << 16) | width);
	OUT_RING(dwords);
	END_DMA();

	for (i = 0; i < dwords; i++) {
		BEGIN_DMA(1);
		OUT_RING(((CARD32 *)src)[i]);
		END_DMA();
	}

	if (atic->is_radeon) {
		BEGIN_DMA(4);
		OUT_REG(RADEON_REG_RB3D_DSTCACHE_CTLSTAT,
		    RADEON_RB3D_DC_FLUSH_ALL);
		OUT_REG(ATI_REG_WAIT_UNTIL,
		    RADEON_WAIT_2D_IDLECLEAN |
		    RADEON_WAIT_HOST_IDLECLEAN);
		END_DMA();
	} else {
		BEGIN_DMA(2);
		OUT_REG(R128_REG_PC_GUI_CTLSTAT, R128_PC_FLUSH_GUI);
		END_DMA();
	}

	kaaMarkSync(pScreen);

	ErrorF("hostdata upload %d,%d %dbpp\n", width, height, bpp);

	return TRUE;
}


static Bool
ATIUploadToScratch(PixmapPtr pSrc, PixmapPtr pDst)
{
	KdScreenPriv(pSrc->drawable.pScreen);
	ATICardInfo(pScreenPriv);
	ATIScreenInfo(pScreenPriv);
	int dst_pitch, src_pitch, w, i, size, bytes;
	unsigned char *dst, *src;
	RING_LOCALS;

	ENTER_DRAW(pSrc);
	/* Align width to log 2, useful for R128 composite.  This should be a
	 * KAA flag we check for (and supported in kaa.c in general) since many
	 * older bits of hardware are going to want POT pitches.
	 */
	w = pSrc->drawable.width;
	if (atis->kaa.flags & KAA_OFFSCREEN_ALIGN_POT)
		w = 1 << (ATILog2(w - 1) + 1);
	dst_pitch = (w * pSrc->drawable.bitsPerPixel / 8 +
	    atis->kaa.pitchAlign - 1) & ~(atis->kaa.pitchAlign - 1);

	size = dst_pitch * pSrc->drawable.height;
	if (size > atis->scratch_area->size)
		ATI_FALLBACK(("Pixmap too large for scratch (%d,%d)\n",
		    pSrc->drawable.width, pSrc->drawable.height));

	atis->scratch_next = (atis->scratch_next + atis->kaa.offsetAlign - 1) &
	    ~(atis->kaa.offsetAlign - 1);
	if (atis->scratch_next + size > atis->scratch_area->offset +
	    atis->scratch_area->size) {
		/* Only sync when we've used all of the scratch area. */
		kaaWaitSync(pSrc->drawable.pScreen);
		atis->scratch_next = atis->scratch_area->offset;
	}
	memcpy(pDst, pSrc, sizeof(*pDst));
	pDst->devKind = dst_pitch;
	pDst->devPrivate.ptr = pScreenPriv->screen->memory_base +
	    atis->scratch_next;
	atis->scratch_next += size;

	src = pSrc->devPrivate.ptr;
	src_pitch = pSrc->devKind;
	dst = pDst->devPrivate.ptr;
	bytes = src_pitch < dst_pitch ? src_pitch : dst_pitch;

	i = pSrc->drawable.height;
	while (i--) {
		memcpy(dst, src, bytes);
		dst += dst_pitch;
		src += src_pitch;
	}

	/* Flush the pixel cache */
	if (atic->is_radeon) {
		BEGIN_DMA(4);
		OUT_REG(RADEON_REG_RB3D_DSTCACHE_CTLSTAT,
		    RADEON_RB3D_DC_FLUSH_ALL);
		OUT_REG(ATI_REG_WAIT_UNTIL, RADEON_WAIT_HOST_IDLECLEAN);
		END_DMA();
	} else {
		BEGIN_DMA(2);
		OUT_REG(R128_REG_PC_GUI_CTLSTAT, R128_PC_FLUSH_ALL);
		END_DMA();
	}

	LEAVE_DRAW(pSrc);
	return TRUE;
}

static void
ATIBlockHandler(pointer blockData, OSTimePtr timeout, pointer readmask)
{
	ScreenPtr pScreen = (ScreenPtr) blockData;
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);

	/* When the server is going to sleep, make sure that all DMA data has
	 * been flushed.
	 */
	if (atis->indirectBuffer)
		ATIFlushIndirect(atis, 1);
}

static void
ATIWakeupHandler(pointer blockData, int result, pointer readmask)
{
}

Bool
ATIDrawInit(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);

	ErrorF("Screen: %d/%d depth/bpp\n", pScreenPriv->screen->fb[0].depth,
	    pScreenPriv->screen->fb[0].bitsPerPixel);

	RegisterBlockAndWakeupHandlers(ATIBlockHandler, ATIWakeupHandler,
	    pScreen);

#ifdef USE_DRI
	atis->using_dri = ATIDRIScreenInit(pScreen);
#endif /* USE_DRI */

	memset(&atis->kaa, 0, sizeof(KaaScreenInfoRec));
	atis->kaa.waitMarker = ATIWaitMarker;
	atis->kaa.PrepareSolid = ATIPrepareSolid;
	atis->kaa.Solid = ATISolid;
	atis->kaa.DoneSolid = ATIDoneSolid;
	atis->kaa.PrepareCopy = ATIPrepareCopy;
	atis->kaa.Copy = ATICopy;
	atis->kaa.DoneCopy = ATIDoneCopy;
	/* Other acceleration will be hooked in in DrawEnable depending on
	 * what type of DMA gets initialized.
	 */

	atis->kaa.flags = KAA_OFFSCREEN_PIXMAPS;
	if (atic->is_radeon) {
		atis->kaa.offsetAlign = 1024;
		atis->kaa.pitchAlign = 64;
	} else {
		/* Rage 128 compositing wants power-of-two pitches. */
		atis->kaa.flags |= KAA_OFFSCREEN_ALIGN_POT;
		atis->kaa.offsetAlign = 32;
		/* Pitch alignment is in sets of 8 pixels, and we need to cover
		 * 32bpp, so 32 bytes.
		 */
		atis->kaa.pitchAlign = 32;
	}

	kaaInitTrapOffsets(8, sample_offsets_x, sample_offsets_y, 0.0, 0.0);
	sample_count = (1 << 8) - 1;

	if (!kaaDrawInit(pScreen, &atis->kaa))
		return FALSE;

	return TRUE;
}

static void
ATIScratchSave(ScreenPtr pScreen, KdOffscreenArea *area)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);

	atis->scratch_area = NULL;
}

void
ATIDrawEnable(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);

	ATIDMASetup(pScreen);
	ATIDrawSetup(pScreen);

	atis->scratch_area = NULL;
	atis->kaa.PrepareBlend = NULL;
	atis->kaa.Blend = NULL;
	atis->kaa.DoneBlend = NULL;
	atis->kaa.CheckComposite = NULL;
	atis->kaa.PrepareComposite = NULL;
	atis->kaa.Composite = NULL;
	atis->kaa.DoneComposite = NULL;
	atis->kaa.UploadToScreen = NULL;
    	atis->kaa.UploadToScratch = NULL;

	/* We can't dispatch 3d commands in PIO mode. */
	if (!atis->using_pio) {
		if (!atic->is_radeon) {
			atis->kaa.CheckComposite = R128CheckComposite;
			atis->kaa.PrepareComposite = R128PrepareComposite;
			atis->kaa.Composite = R128Composite;
			atis->kaa.DoneComposite = R128DoneComposite;
		} else if (atic->is_r100) {
			atis->kaa.CheckComposite = R100CheckComposite;
			atis->kaa.PrepareComposite = R100PrepareComposite;
			atis->kaa.Composite = RadeonComposite;
			atis->kaa.DoneComposite = RadeonDoneComposite;
		} else if (atic->is_r200) {
			atis->kaa.CheckComposite = R200CheckComposite;
			atis->kaa.PrepareComposite = R200PrepareComposite;
			atis->kaa.Composite = RadeonComposite;
			atis->kaa.DoneComposite = RadeonDoneComposite;
		}
	}
#ifdef USE_DRI
	if (atis->using_dri) {
		if (!atic->is_radeon) {
			/*atis->kaa.PrepareTrapezoids = R128PrepareTrapezoids;
			atis->kaa.Trapezoids = R128Trapezoids;
			atis->kaa.DoneTrapezoids = R128DoneTrapezoids;*/
		} else if (atic->is_r100 || atic->is_r200) {
			atis->kaa.PrepareTrapezoids = RadeonPrepareTrapezoids;
			atis->kaa.Trapezoids = RadeonTrapezoids;
			atis->kaa.DoneTrapezoids = RadeonDoneTrapezoids;
		}
	}
#endif /* USE_DRI */

	atis->kaa.UploadToScreen = ATIUploadToScreen;

	/* Reserve a scratch area.  It'll be used for storing glyph data during
	 * Composite operations, because glyphs aren't in real pixmaps and thus
	 * can't be migrated.
	 */
	atis->scratch_area = KdOffscreenAlloc(pScreen, 131072,
	    atis->kaa.offsetAlign, TRUE, ATIScratchSave, atis);
	if (atis->scratch_area != NULL) {
		atis->scratch_next = atis->scratch_area->offset;
		atis->kaa.UploadToScratch = ATIUploadToScratch;
	}

	kaaMarkSync(pScreen);
}

void
ATIDrawDisable(ScreenPtr pScreen)
{
	kaaWaitSync(pScreen);
	ATIDMATeardown(pScreen);
}

void
ATIDrawFini(ScreenPtr pScreen)
{
#ifdef USE_DRI
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	if (atis->using_dri) {
		ATIDRICloseScreen(pScreen);
		atis->using_dri = FALSE;
	}
#endif /* USE_DRI */

	RemoveBlockAndWakeupHandlers(ATIBlockHandler, ATIWakeupHandler,
	    pScreen);

	kaaDrawFini(pScreen);
}

