/*
 * Copyright © 2003 Eric Anholt, Anders Carlsson
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

#include "ati.h"
#include "ati_reg.h"
#include "ati_dma.h"
#include "ati_draw.h"

extern ATIScreenInfo *accel_atis;
extern int sample_count;
extern float sample_offsets_x[255];
extern float sample_offsets_y[255];
extern CARD8 ATIBltRop[16];

static int widths[2] = {1,1};
static int heights[2] = {1,1};
static Bool is_transform[2];
static PictTransform *transform[2];

struct blendinfo {
	Bool dst_alpha;
	Bool src_alpha;
	CARD32 blendctl;
};

static struct blendinfo R128BlendOp[] = {
	/* Clear */
	{0, 0, R128_SBLEND_ZERO		 | R128_DBLEND_ZERO},
	/* Src */
	{0, 0, R128_SBLEND_ONE		 | R128_DBLEND_ZERO},
	/* Dst */
	{0, 0, R128_SBLEND_ZERO		 | R128_DBLEND_ONE},
	/* Over */
	{0, 1, R128_SBLEND_ONE		 | R128_DBLEND_INV_SRC_ALPHA},
	/* OverReverse */
	{1, 0, R128_SBLEND_INV_DST_ALPHA | R128_DBLEND_ONE},
	/* In */
	{1, 0, R128_SBLEND_DST_ALPHA	 | R128_DBLEND_ZERO},
	/* InReverse */
	{0, 1, R128_SBLEND_ZERO		 | R128_DBLEND_SRC_ALPHA},
	/* Out */
	{1, 0, R128_SBLEND_INV_DST_ALPHA | R128_DBLEND_ZERO},
	/* OutReverse */
	{0, 1, R128_SBLEND_ZERO		 | R128_DBLEND_INV_SRC_ALPHA},
	/* Atop */
	{1, 1, R128_SBLEND_DST_ALPHA	 | R128_DBLEND_INV_SRC_ALPHA},
	/* AtopReverse */
	{1, 1, R128_SBLEND_INV_DST_ALPHA | R128_DBLEND_SRC_ALPHA},
	/* Xor */
	{1, 1, R128_SBLEND_INV_DST_ALPHA | R128_DBLEND_INV_SRC_ALPHA},
	/* Add */
	{0, 0, R128_SBLEND_ONE		 | R128_DBLEND_ONE},
};

static Bool
R128GetDatatypePict(CARD32 format, CARD32 *type)
{
	switch (format) {
	case PICT_a1r5g5b5:
	case PICT_x1r5g5b5:
		*type = R128_DATATYPE_ARGB1555;
		return TRUE;
	case PICT_r5g6b5:
		*type = R128_DATATYPE_RGB565;
		return TRUE;
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
		*type = R128_DATATYPE_ARGB8888;
		return TRUE;
	default:
		return FALSE;
	}

}

static Bool
R128CheckCompositeTexture(PicturePtr pPict)
{
	int w = pPict->pDrawable->width;
	int h = pPict->pDrawable->height;

	if (w > (1 << 10) || h > (1 << 10))
		ATI_FALLBACK(("Picture w/h too large (%dx%d)\n", w, h));
	if (pPict->repeat && ((w & (w - 1)) != 0 || (h & (h - 1)) != 0))
		ATI_FALLBACK(("NPOT repeat unsupported (%dx%d)\n", w, h));

	switch (pPict->format) {
	case PICT_a8:
	case PICT_a1r5g5b5:
	case PICT_a4r4g4b4:
	case PICT_r5g6b5:
	case PICT_a8r8g8b8:
		break;
	default:
		ATI_FALLBACK(("Unsupported picture format 0x%x\n",
		    pPict->format));
	}

	return TRUE;
}

Bool
R128CheckComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture)
{
	CARD32 dstDatatype;

	if (op >= sizeof(R128BlendOp)/sizeof(R128BlendOp[0]))
		ATI_FALLBACK(("Unsupported op 0x%x\n", op));
	if (pDstPicture->format == PICT_a8) {
		if (R128BlendOp[op].src_alpha || R128BlendOp[op].dst_alpha ||
		    pMaskPicture != NULL)
			ATI_FALLBACK(("alpha blending unsupported with "
			    "A8 dst?\n"));
	} else if (!R128GetDatatypePict(pDstPicture->format, &dstDatatype)) {
		ATI_FALLBACK(("Unsupported dest format 0x%x\n",
		    pDstPicture->format));
	}
	if (pMaskPicture != NULL && pMaskPicture->componentAlpha &&
	    R128BlendOp[op].src_alpha)
		ATI_FALLBACK(("Component alpha not supported with source alpha "
		    "blending.\n"));

	if (!R128CheckCompositeTexture(pSrcPicture))
		return FALSE;
	if (pMaskPicture != NULL && !R128CheckCompositeTexture(pMaskPicture))
		return FALSE;

	return TRUE;
}

static Bool
R128TextureSetup(PicturePtr pPict, PixmapPtr pPix, int unit, CARD32 *txsize,
    CARD32 *tex_cntl_c)
{
	int w = pPict->pDrawable->width;
	int h = pPict->pDrawable->height;
	int bytepp, shift, l2w, l2h, l2p;
	int pitch;

	pitch = pPix->devKind;
	if ((pitch & (pitch - 1)) != 0)
		ATI_FALLBACK(("NPOT pitch 0x%x unsupported\n", pitch));

	switch (pPict->format) {
	case PICT_a8:
		/* DATATYPE_RGB8 appears to expand the value into the alpha
		 * channel like we want.  We then blank out the R,G,B channels
		 * as necessary using the combiners.
		 */
		*tex_cntl_c = R128_DATATYPE_RGB8 << R128_TEX_DATATYPE_SHIFT;
		break;
	case PICT_a1r5g5b5:
		*tex_cntl_c = R128_DATATYPE_ARGB1555 << R128_TEX_DATATYPE_SHIFT;
		break;
	case PICT_a4r4g4b4:
		*tex_cntl_c = R128_DATATYPE_ARGB4444 << R128_TEX_DATATYPE_SHIFT;
		break;
	case PICT_r5g6b5:
		*tex_cntl_c = R128_DATATYPE_RGB565 << R128_TEX_DATATYPE_SHIFT;
		break;
	case PICT_a8r8g8b8:
		*tex_cntl_c = R128_DATATYPE_ARGB8888 << R128_TEX_DATATYPE_SHIFT;
		break;
	default:
		return FALSE;
	}
	bytepp = PICT_FORMAT_BPP(pPict->format) / 8;

	*tex_cntl_c |= R128_MIP_MAP_DISABLE;

	if (pPict->filter == PictFilterBilinear)
		*tex_cntl_c |= R128_MIN_BLEND_LINEAR | R128_MAG_BLEND_LINEAR;

	if (unit == 0)
		shift = 0;
	else {
		shift = 16;
		*tex_cntl_c |= R128_SEC_SELECT_SEC_ST;
	}

	/* ATILog2 returns -1 for value of 0 */
	l2w = ATILog2(w - 1) + 1;
	l2h = ATILog2(h - 1) + 1;
	l2p = ATILog2(pPix->devKind / bytepp);

	if (pPict->repeat && w == 1 && h == 1)
		l2p = 0;
	else if (pPict->repeat && l2p != l2w)
		ATI_FALLBACK(("Repeat not supported for pitch != width\n"));
	l2w = l2p;

	widths[unit] = 1 << l2w;
	heights[unit] = 1 << l2h;
	*txsize |= l2p << (R128_TEX_PITCH_SHIFT + shift);
	*txsize |= ((l2w > l2h) ? l2w : l2h) << (R128_TEX_SIZE_SHIFT + shift);
	*txsize |= l2h << (R128_TEX_HEIGHT_SHIFT + shift);

	if (pPict->transform != 0) {
		is_transform[unit] = TRUE;
		transform[unit] = pPict->transform;
	} else {
		is_transform[unit] = FALSE;
	}

	return TRUE;
}

Bool
R128PrepareComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture, PixmapPtr pSrc, PixmapPtr pMask, PixmapPtr pDst)
{
	KdScreenPriv(pDst->drawable.pScreen);
	ATIScreenInfo(pScreenPriv);
	CARD32 txsize = 0, prim_tex_cntl_c, sec_tex_cntl_c = 0, dstDatatype;
	CARD32 dst_pitch_offset, color_factor, in_color_factor, alpha_comb;
	CARD32 blend_cntl;
	int i;
	RING_LOCALS;

	accel_atis = atis;

	if (pDstPicture->format == PICT_a8)
		dstDatatype = R128_DATATYPE_Y8;
	else
		R128GetDatatypePict(pDstPicture->format, &dstDatatype);

	if (!R128TextureSetup(pSrcPicture, pSrc, 0, &txsize, &prim_tex_cntl_c))
		return FALSE;
	if (pMask != NULL && !R128TextureSetup(pMaskPicture, pMask, 1, &txsize,
	    &sec_tex_cntl_c))
		return FALSE;
	else if (pMask == NULL)
		is_transform[1] = FALSE;

	if (!ATIGetPixmapOffsetPitch(pDst, &dst_pitch_offset))
		return FALSE;

	blend_cntl = R128BlendOp[op].blendctl;
	if (PICT_FORMAT_A(pDstPicture->format) == 0 &&
	    R128BlendOp[op].dst_alpha) {
		if ((blend_cntl & R128_SBLEND_MASK) ==
		    R128_SBLEND_DST_ALPHA)
			blend_cntl = (blend_cntl & ~R128_SBLEND_MASK) |
			    R128_SBLEND_ONE;
		else if ((blend_cntl & R128_SBLEND_MASK) ==
		    R128_SBLEND_INV_DST_ALPHA)
			blend_cntl = (blend_cntl & ~R128_SBLEND_MASK) |
			    R128_SBLEND_ZERO;
	}

	BEGIN_DMA(12);
	OUT_REG(R128_REG_SCALE_3D_CNTL,
	    R128_SCALE_3D_TEXMAP_SHADE |
	    R128_SCALE_PIX_REPLICATE |
	    R128_TEX_CACHE_SPLIT |
	    R128_TEX_MAP_ALPHA_IN_TEXTURE |
	    R128_TEX_CACHE_LINE_SIZE_4QW);
	OUT_REG(ATI_REG_DST_PITCH_OFFSET, dst_pitch_offset);
	OUT_REG(ATI_REG_DP_GUI_MASTER_CNTL,
	    ATI_GMC_DST_PITCH_OFFSET_CNTL |
	    ATI_GMC_BRUSH_SOLID_COLOR |
	    (dstDatatype << 8) |
	    ATI_GMC_SRC_DATATYPE_COLOR |
	    (ATIBltRop[GXcopy] << 16) |
	    ATI_DP_SRC_SOURCE_MEMORY |
	    R128_GMC_3D_FCN_EN |
	    ATI_GMC_CLR_CMP_CNTL_DIS |
	    R128_GMC_AUX_CLIP_DIS |
	    ATI_GMC_WR_MSK_DIS);
	OUT_REG(R128_REG_MISC_3D_STATE_CNTL,
	    R128_MISC_SCALE_3D_TEXMAP_SHADE |
	    R128_MISC_SCALE_PIX_REPLICATE |
	    R128_ALPHA_COMB_ADD_CLAMP |
	    blend_cntl);
	OUT_REG(R128_REG_TEX_CNTL_C,
	    R128_TEXMAP_ENABLE |
	    ((pMask != NULL) ? R128_SEC_TEXMAP_ENABLE : 0) |
	    R128_ALPHA_ENABLE |
	    R128_TEX_CACHE_FLUSH);
	OUT_REG(R128_REG_PC_GUI_CTLSTAT, R128_PC_FLUSH_GUI);
	END_DMA();

	/* IN operator: Without a mask, only the first texture unit is enabled.
	 * With a mask, we put the source in the first unit and have it pass
	 * through as input to the 2nd.  The 2nd unit takes the incoming source
	 * pixel and modulates it with either the alpha or each of the channels
	 * in the mask, depending on componentAlpha.
	 */
	BEGIN_DMA(15);
	OUT_RING(DMA_PACKET0(R128_REG_PRIM_TEX_CNTL_C, 14));
	OUT_RING_REG(R128_REG_PRIM_TEX_CNTL_C, prim_tex_cntl_c);

	/* If this is the only stage and the dest is a8, route the alpha result 
	 * to the color (red channel, in particular), too.  Otherwise, be sure
	 * to zero out color channels of an a8 source.
	 */
	if (pMaskPicture == NULL && pDstPicture->format == PICT_a8)
		color_factor = R128_COLOR_FACTOR_ALPHA;
	else if (pSrcPicture->format == PICT_a8)
		color_factor = R128_COLOR_FACTOR_CONST_COLOR;
	else
		color_factor = R128_COLOR_FACTOR_TEX;

	if (PICT_FORMAT_A(pSrcPicture->format) == 0)
		alpha_comb = R128_COMB_ALPHA_COPY_INP;
	else
		alpha_comb = R128_COMB_ALPHA_DIS;

	OUT_RING_REG(R128_REG_PRIM_TEXTURE_COMBINE_CNTL_C,
	    R128_COMB_COPY |
	    color_factor |
	    R128_INPUT_FACTOR_INT_COLOR |
	    alpha_comb |
	    R128_ALPHA_FACTOR_TEX_ALPHA |
	    R128_INP_FACTOR_A_CONST_ALPHA);
	OUT_RING_REG(R128_REG_TEX_SIZE_PITCH_C, txsize);
	/* We could save some output by only writing the offset register that
	 * will actually be used.  On the other hand, this is easy.
	 */
	for (i = 0; i <= 10; i++) {
		OUT_RING_REG(R128_REG_PRIM_TEX_0_OFFSET_C + 4 * i,
		    ((CARD8 *)pSrc->devPrivate.ptr -
		    pScreenPriv->screen->memory_base));
	}
	END_DMA();

	if (pMask != NULL) {
		BEGIN_DMA(14);
		OUT_RING(DMA_PACKET0(R128_REG_SEC_TEX_CNTL_C, 13));
		OUT_RING_REG(R128_REG_SEC_TEX_CNTL_C, sec_tex_cntl_c);

		if (pDstPicture->format == PICT_a8) {
			color_factor = R128_COLOR_FACTOR_ALPHA;
			in_color_factor = R128_INPUT_FACTOR_PREV_ALPHA;
		} else if (pMaskPicture->componentAlpha) {
			color_factor = R128_COLOR_FACTOR_TEX;
			in_color_factor = R128_INPUT_FACTOR_PREV_COLOR;
		} else {
			color_factor = R128_COLOR_FACTOR_ALPHA;
			in_color_factor = R128_INPUT_FACTOR_PREV_COLOR;
		}

		OUT_RING_REG(R128_REG_SEC_TEXTURE_COMBINE_CNTL_C,
		    R128_COMB_MODULATE |
		    color_factor |
		    in_color_factor |
		    R128_COMB_ALPHA_MODULATE |
		    R128_ALPHA_FACTOR_TEX_ALPHA |
		    R128_INP_FACTOR_A_PREV_ALPHA);
		for (i = 0; i <= 10; i++) {
			OUT_RING_REG(R128_REG_SEC_TEX_0_OFFSET_C + 4 * i,
			    ((CARD8 *)pMask->devPrivate.ptr -
			    pScreenPriv->screen->memory_base));
		}
		END_DMA();
	}

	return TRUE;
}
#define VTX_RING_COUNT 8

#define VTX_OUT(_dstX, _dstY, _srcX, _srcY, _maskX, _maskY)		\
do {									\
	OUT_RING_F((_dstX));						\
	OUT_RING_F(((float)(_dstY)) + .125);				\
	OUT_RING_F(0.0);						\
	OUT_RING_F(1.0);						\
	OUT_RING_F((((float)(_srcX)) + 0.5) / (widths[0]));		\
	OUT_RING_F((((float)(_srcY)) + 0.5) / (heights[0]));		\
	OUT_RING_F((((float)(_maskX)) + 0.5) / (widths[1]));		\
	OUT_RING_F((((float)(_maskY)) + 0.5) / (heights[1]));		\
} while (0)

void
R128Composite(int srcX, int srcY, int maskX, int maskY, int dstX, int dstY,
    int w, int h)
{
	ATIScreenInfo *atis = accel_atis;
	int srcXend, srcYend, maskXend, maskYend;
	PictVector v;
	RING_LOCALS;

	/*ErrorF("R128Composite (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",
	    srcX, srcY, maskX, maskY,dstX, dstY, w, h);*/

	srcXend = srcX + w;
	srcYend = srcY + h;
	maskXend = maskX + w;
	maskYend = maskY + h;
	if (is_transform[0]) {
		v.vector[0] = IntToxFixed(srcX);
		v.vector[1] = IntToxFixed(srcY);
		v.vector[2] = xFixed1;
		PictureTransformPoint(transform[0], &v);
		srcX = xFixedToInt(v.vector[0]);
		srcY = xFixedToInt(v.vector[1]);
		v.vector[0] = IntToxFixed(srcXend);
		v.vector[1] = IntToxFixed(srcYend);
		v.vector[2] = xFixed1;
		PictureTransformPoint(transform[0], &v);
		srcXend = xFixedToInt(v.vector[0]);
		srcYend = xFixedToInt(v.vector[1]);
	}
	if (is_transform[1]) {
		v.vector[0] = IntToxFixed(maskX);
		v.vector[1] = IntToxFixed(maskY);
		v.vector[2] = xFixed1;
		PictureTransformPoint(transform[1], &v);
		maskX = xFixedToInt(v.vector[0]);
		maskY = xFixedToInt(v.vector[1]);
		v.vector[0] = IntToxFixed(maskXend);
		v.vector[1] = IntToxFixed(maskYend);
		v.vector[2] = xFixed1;
		PictureTransformPoint(transform[1], &v);
		maskXend = xFixedToInt(v.vector[0]);
		maskYend = xFixedToInt(v.vector[1]);
	}

	BEGIN_DMA(3 + 4 * VTX_RING_COUNT);
	OUT_RING(DMA_PACKET3(ATI_CCE_PACKET3_3D_RNDR_GEN_PRIM,
	    2 + 4 * VTX_RING_COUNT));
	OUT_RING(R128_CCE_VC_FRMT_RHW |
	    R128_CCE_VC_FRMT_S_T |
	    R128_CCE_VC_FRMT_S2_T2);
	OUT_RING(R128_CCE_VC_CNTL_PRIM_TYPE_TRI_FAN |
	    R128_CCE_VC_CNTL_PRIM_WALK_RING |
	    (4 << R128_CCE_VC_CNTL_NUM_SHIFT));

	VTX_OUT(dstX,     dstY,     srcX,    srcY,    maskX,    maskY);
	VTX_OUT(dstX,     dstY + h, srcX,    srcYend, maskX,    maskYend);
	VTX_OUT(dstX + w, dstY + h, srcXend, srcYend, maskXend, maskYend);
	VTX_OUT(dstX + w, dstY,     srcXend, srcY,    maskXend, maskY);

	END_DMA();
}

void
R128DoneComposite(void)
{
}

Bool
R128PrepareTrapezoids(PicturePtr pDstPicture, PixmapPtr pDst)
{
	KdScreenPriv(pDst->drawable.pScreen);
	ATIScreenInfo(pScreenPriv);
	CARD32 dst_pitch_offset;
	RING_LOCALS;

	accel_atis = atis;

	if (!ATIGetPixmapOffsetPitch(pDst, &dst_pitch_offset))
		return FALSE;

	BEGIN_DMA(18);
	OUT_REG(R128_REG_SCALE_3D_CNTL,
	    R128_SCALE_3D_TEXMAP_SHADE |
	    R128_SCALE_PIX_REPLICATE |
	    R128_TEX_CACHE_SPLIT |
	    R128_TEX_CACHE_LINE_SIZE_4QW);
	OUT_REG(ATI_REG_DST_PITCH_OFFSET, dst_pitch_offset);
	OUT_REG(ATI_REG_DP_GUI_MASTER_CNTL,
	    ATI_GMC_DST_PITCH_OFFSET_CNTL |
	    ATI_GMC_BRUSH_SOLID_COLOR |
	    (R128_DATATYPE_RGB8 << 8) |
	    ATI_GMC_SRC_DATATYPE_COLOR |
	    (ATIBltRop[GXcopy] << 16) |
	    ATI_DP_SRC_SOURCE_MEMORY |
	    R128_GMC_3D_FCN_EN |
	    ATI_GMC_CLR_CMP_CNTL_DIS |
	    ATI_GMC_WR_MSK_DIS);
	OUT_REG(R128_REG_MISC_3D_STATE_CNTL,
	    R128_MISC_SCALE_3D_TEXMAP_SHADE |
	    R128_MISC_SCALE_PIX_REPLICATE |
	    R128_ALPHA_COMB_ADD_CLAMP | 
	    R128BlendOp[PictOpAdd].blendctl);
	OUT_REG(R128_REG_TEX_CNTL_C,
	    R128_ALPHA_ENABLE);
	OUT_REG(R128_REG_PC_GUI_CTLSTAT, R128_PC_FLUSH_GUI);

	OUT_RING(DMA_PACKET0(R128_REG_AUX_SC_CNTL, 5));
	OUT_RING_REG(R128_REG_AUX_SC_CNTL, R128_AUX1_SC_ENB);
	OUT_RING_REG(R128_REG_AUX1_SC_LEFT, 0);
	OUT_RING_REG(R128_REG_AUX1_SC_RIGHT, pDst->drawable.width);
	OUT_RING_REG(R128_REG_AUX1_SC_TOP, 0);
	OUT_RING_REG(R128_REG_AUX1_SC_BOTTOM, pDst->drawable.height);
	END_DMA();

	return TRUE;
}

#define TRAP_VERT_RING_COUNT 4

#define TRAP_VERT(_x, _y)						\
do {									\
	OUT_RING_F((_x) + sample_x);					\
	OUT_RING_F((_y) + 0.125 + sample_y);				\
	OUT_RING_F(0.0);						\
	OUT_RING(0x01010101);						\
} while (0)

void
R128Trapezoids(KaaTrapezoid *traps, int ntraps)
{
	ATIScreenInfo *atis = accel_atis;
	RING_LOCALS;

	while (ntraps > 0) {
		int i, sample, count, vertcount;

		count = 0xffff / 4 / sample_count;
		if (count > ntraps)
			count = ntraps;
		vertcount = count * sample_count * 4;

		BEGIN_DMA(3 + vertcount * TRAP_VERT_RING_COUNT);
		OUT_RING(DMA_PACKET3(ATI_CCE_PACKET3_3D_RNDR_GEN_PRIM,
		    2 + vertcount * TRAP_VERT_RING_COUNT));
		OUT_RING(R128_CCE_VC_FRMT_DIFFUSE_ARGB);
		OUT_RING(R128_CCE_VC_CNTL_PRIM_TYPE_TRI_FAN |
		    R128_CCE_VC_CNTL_PRIM_WALK_RING |
		    (vertcount << R128_CCE_VC_CNTL_NUM_SHIFT));

		for (i = 0; i < count; i++) {
		    for (sample = 0; sample < sample_count; sample++) {
			float sample_x = sample_offsets_x[sample];
			float sample_y = sample_offsets_y[sample];
			TRAP_VERT(traps[i].tl, traps[i].ty);
			TRAP_VERT(traps[i].bl, traps[i].by);
			TRAP_VERT(traps[i].br, traps[i].by);
			TRAP_VERT(traps[i].tr, traps[i].ty);
		    }
		}
		END_DMA();

		ntraps -= count;
		traps += count;
	}
}

void
R128DoneTrapezoids(void)
{
}
