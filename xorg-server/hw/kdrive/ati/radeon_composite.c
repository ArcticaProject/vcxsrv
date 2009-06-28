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

extern ATIScreenInfo *accel_atis;
extern int sample_count;
extern float sample_offsets_x[255];
extern float sample_offsets_y[255];
static Bool is_transform[2];
static PictTransform *transform[2];

struct blendinfo {
	Bool dst_alpha;
	Bool src_alpha;
	CARD32 blend_cntl;
};

static struct blendinfo RadeonBlendOp[] = {
	/* Clear */
	{0, 0, RADEON_SBLEND_GL_ZERO	      | RADEON_DBLEND_GL_ZERO},
	/* Src */
	{0, 0, RADEON_SBLEND_GL_ONE	      | RADEON_DBLEND_GL_ZERO},
	/* Dst */
	{0, 0, RADEON_SBLEND_GL_ZERO	      | RADEON_DBLEND_GL_ONE},
	/* Over */
	{0, 1, RADEON_SBLEND_GL_ONE	      | RADEON_DBLEND_GL_INV_SRC_ALPHA},
	/* OverReverse */
	{1, 0, RADEON_SBLEND_GL_INV_DST_ALPHA | RADEON_DBLEND_GL_ONE},
	/* In */
	{1, 0, RADEON_SBLEND_GL_DST_ALPHA     | RADEON_DBLEND_GL_ZERO},
	/* InReverse */
	{0, 1, RADEON_SBLEND_GL_ZERO	      | RADEON_DBLEND_GL_SRC_ALPHA},
	/* Out */
	{1, 0, RADEON_SBLEND_GL_INV_DST_ALPHA | RADEON_DBLEND_GL_ZERO},
	/* OutReverse */
	{0, 1, RADEON_SBLEND_GL_ZERO	      | RADEON_DBLEND_GL_INV_SRC_ALPHA},
	/* Atop */
	{1, 1, RADEON_SBLEND_GL_DST_ALPHA     | RADEON_DBLEND_GL_INV_SRC_ALPHA},
	/* AtopReverse */
	{1, 1, RADEON_SBLEND_GL_INV_DST_ALPHA | RADEON_DBLEND_GL_SRC_ALPHA},
	/* Xor */
	{1, 1, RADEON_SBLEND_GL_INV_DST_ALPHA | RADEON_DBLEND_GL_INV_SRC_ALPHA},
	/* Add */
	{0, 0, RADEON_SBLEND_GL_ONE	      | RADEON_DBLEND_GL_ONE},
};

struct formatinfo {
	int fmt;
	Bool byte_swap;
	CARD32 card_fmt;
};

/* Note on texture formats:
 * TXFORMAT_Y8 expands to (Y,Y,Y,1).  TXFORMAT_I8 expands to (I,I,I,I)
 */
static struct formatinfo R100TexFormats[] = {
	{PICT_a8r8g8b8,	0, RADEON_TXFORMAT_ARGB8888 | RADEON_TXFORMAT_ALPHA_IN_MAP},
	{PICT_x8r8g8b8,	0, RADEON_TXFORMAT_ARGB8888},
	{PICT_a8b8g8r8,	1, RADEON_TXFORMAT_RGBA8888 | RADEON_TXFORMAT_ALPHA_IN_MAP},
	{PICT_x8b8g8r8,	1, RADEON_TXFORMAT_RGBA8888},
	{PICT_r5g6b5,	0, RADEON_TXFORMAT_RGB565},
	{PICT_a1r5g5b5,	0, RADEON_TXFORMAT_ARGB1555 | RADEON_TXFORMAT_ALPHA_IN_MAP},
	{PICT_x1r5g5b5,	0, RADEON_TXFORMAT_ARGB1555},
	{PICT_a8,	0, RADEON_TXFORMAT_I8 | RADEON_TXFORMAT_ALPHA_IN_MAP},
};

static struct formatinfo R200TexFormats[] = {
	{PICT_a8r8g8b8,	0, R200_TXFORMAT_ARGB8888 | R200_TXFORMAT_ALPHA_IN_MAP},
	{PICT_x8r8g8b8,	0, R200_TXFORMAT_ARGB8888},
	{PICT_a8r8g8b8,	1, R200_TXFORMAT_RGBA8888 | R200_TXFORMAT_ALPHA_IN_MAP},
	{PICT_x8r8g8b8,	1, R200_TXFORMAT_RGBA8888},
	{PICT_r5g6b5,	0, R200_TXFORMAT_RGB565},
	{PICT_a1r5g5b5,	0, R200_TXFORMAT_ARGB1555 | R200_TXFORMAT_ALPHA_IN_MAP},
	{PICT_x1r5g5b5,	0, R200_TXFORMAT_ARGB1555},
	{PICT_a8,	0, R200_TXFORMAT_I8 | R200_TXFORMAT_ALPHA_IN_MAP},
};

/* Common Radeon setup code */

static Bool
RadeonGetDestFormat(PicturePtr pDstPicture, CARD32 *dst_format)
{
	switch (pDstPicture->format) {
	case PICT_a8r8g8b8:
	case PICT_x8r8g8b8:
		*dst_format = RADEON_COLOR_FORMAT_ARGB8888;
		break;
	case PICT_r5g6b5:
		*dst_format = RADEON_COLOR_FORMAT_RGB565;
		break;
	case PICT_a1r5g5b5:
	case PICT_x1r5g5b5:
		*dst_format = RADEON_COLOR_FORMAT_ARGB1555;
		break;
	case PICT_a8:
		*dst_format = RADEON_COLOR_FORMAT_RGB8;
		break;
	default:
		ATI_FALLBACK(("Unsupported dest format 0x%x\n",
		    pDstPicture->format));
	}

	return TRUE;
}

/* R100-specific code */

static Bool
R100CheckCompositeTexture(PicturePtr pPict, int unit)
{
	int w = pPict->pDrawable->width;
	int h = pPict->pDrawable->height;
	int i;

	if ((w > 0x7ff) || (h > 0x7ff))
		ATI_FALLBACK(("Picture w/h too large (%dx%d)\n", w, h));

	for (i = 0; i < sizeof(R100TexFormats) / sizeof(R100TexFormats[0]); i++)
	{
		if (R100TexFormats[i].fmt == pPict->format)
			break;
	}
	if (i == sizeof(R100TexFormats) / sizeof(R100TexFormats[0]))
		ATI_FALLBACK(("Unsupported picture format 0x%x\n",
		    pPict->format));

	if (pPict->repeat && ((w & (w - 1)) != 0 || (h & (h - 1)) != 0))
		ATI_FALLBACK(("NPOT repeat unsupported (%dx%d)\n", w, h));

	if (pPict->filter != PictFilterNearest &&
	    pPict->filter != PictFilterBilinear)
		ATI_FALLBACK(("Unsupported filter 0x%x\n", pPict->filter));

	return TRUE;
}

static Bool
R100TextureSetup(PicturePtr pPict, PixmapPtr pPix, int unit)
{
	ATIScreenInfo *atis = accel_atis;
	KdScreenPriv(pPix->drawable.pScreen);
	CARD32 txfilter, txformat, txoffset, txpitch;
	int w = pPict->pDrawable->width;
	int h = pPict->pDrawable->height;
	int i;
	RING_LOCALS;

	txpitch = pPix->devKind;
	txoffset = ((CARD8 *)pPix->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);

	for (i = 0; i < sizeof(R100TexFormats) / sizeof(R100TexFormats[0]); i++)
	{
		if (R100TexFormats[i].fmt == pPict->format)
			break;
	}
	txformat = R100TexFormats[i].card_fmt;
	if (R100TexFormats[i].byte_swap)
		txoffset |= RADEON_TXO_ENDIAN_BYTE_SWAP;

	if (pPict->repeat) {
		txformat |= ATILog2(w) << RADEON_TXFORMAT_WIDTH_SHIFT;
		txformat |= ATILog2(h) << RADEON_TXFORMAT_HEIGHT_SHIFT;
	} else 
		txformat |= RADEON_TXFORMAT_NON_POWER2;
	txformat |= unit << 24; /* RADEON_TXFORMAT_ST_ROUTE_STQX */

 
	if ((txoffset & 0x1f) != 0)
		ATI_FALLBACK(("Bad texture offset 0x%x\n", txoffset));
	if ((txpitch & 0x1f) != 0)
		ATI_FALLBACK(("Bad texture pitch 0x%x\n", txpitch));

	switch (pPict->filter) {
	case PictFilterNearest:
		txfilter = (RADEON_MAG_FILTER_NEAREST |
			    RADEON_MIN_FILTER_NEAREST);
		break;
	case PictFilterBilinear:
		txfilter = (RADEON_MAG_FILTER_LINEAR |
			    RADEON_MIN_FILTER_LINEAR);
		break;
	default:
		ATI_FALLBACK(("Bad filter 0x%x\n", pPict->filter));
	}

	BEGIN_DMA(7);
	if (unit == 0) {
		OUT_RING(DMA_PACKET0(RADEON_REG_PP_TXFILTER_0, 3));
		OUT_RING_REG(RADEON_REG_PP_TXFILTER_0, txfilter);
		OUT_RING_REG(RADEON_REG_PP_TXFORMAT_0, txformat);
		OUT_RING_REG(RADEON_REG_PP_TXOFFSET_0, txoffset);
		OUT_RING(DMA_PACKET0(RADEON_REG_PP_TEX_SIZE_0, 2));
		OUT_RING_REG(RADEON_REG_PP_TEX_SIZE_0,
		    (pPix->drawable.width - 1) |
		    ((pPix->drawable.height - 1) << RADEON_TEX_VSIZE_SHIFT));
		OUT_RING_REG(RADEON_REG_PP_TEX_PITCH_0, txpitch - 32);
	} else {
		OUT_RING(DMA_PACKET0(RADEON_REG_PP_TXFILTER_1, 3));
		OUT_RING_REG(RADEON_REG_PP_TXFILTER_1, txfilter);
		OUT_RING_REG(RADEON_REG_PP_TXFORMAT_1, txformat);
		OUT_RING_REG(RADEON_REG_PP_TXOFFSET_1, txoffset);
		OUT_RING(DMA_PACKET0(RADEON_REG_PP_TEX_SIZE_1, 2));
		OUT_RING_REG(RADEON_REG_PP_TEX_SIZE_1,
		    (pPix->drawable.width - 1) |
		    ((pPix->drawable.height - 1) << RADEON_TEX_VSIZE_SHIFT));
		OUT_RING_REG(RADEON_REG_PP_TEX_PITCH_1, txpitch - 32);
	}
	END_DMA();

	if (pPict->transform != 0) {
		is_transform[unit] = TRUE;
		transform[unit] = pPict->transform;
	} else {
		is_transform[unit] = FALSE;
	}

	return TRUE;
}

Bool
R100CheckComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture)
{
	CARD32 tmp1;

	/* Check for unsupported compositing operations. */
	if (op >= sizeof(RadeonBlendOp) / sizeof(RadeonBlendOp[0]))
		ATI_FALLBACK(("Unsupported Composite op 0x%x\n", op));
	if (pMaskPicture != NULL && pMaskPicture->componentAlpha &&
	    RadeonBlendOp[op].src_alpha)
		ATI_FALLBACK(("Component alpha not supported with source "
		    "alpha blending.\n"));
	if (pDstPicture->pDrawable->width >= (1 << 11) ||
	    pDstPicture->pDrawable->height >= (1 << 11))
		ATI_FALLBACK(("Dest w/h too large (%d,%d).\n",
		    pDstPicture->pDrawable->width,
		    pDstPicture->pDrawable->height));

	if (!R100CheckCompositeTexture(pSrcPicture, 0))
		return FALSE;
	if (pMaskPicture != NULL && !R100CheckCompositeTexture(pMaskPicture, 1))
		return FALSE;

	if (pDstPicture->componentAlpha)
		return FALSE;

	if (!RadeonGetDestFormat(pDstPicture, &tmp1))
		return FALSE;

	return TRUE;
}

Bool
R100PrepareComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture, PixmapPtr pSrc, PixmapPtr pMask, PixmapPtr pDst)
{
	KdScreenPriv(pDst->drawable.pScreen);
	ATIScreenInfo(pScreenPriv);
	CARD32 dst_format, dst_offset, dst_pitch;
	CARD32 pp_cntl, blendcntl, cblend, ablend;
	int pixel_shift;
	RING_LOCALS;

	accel_atis = atis;

	RadeonGetDestFormat(pDstPicture, &dst_format);
	pixel_shift = pDst->drawable.bitsPerPixel >> 4;

	dst_offset = ((CARD8 *)pDst->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);
	dst_pitch = pDst->devKind;
	if ((dst_offset & 0x0f) != 0)
		ATI_FALLBACK(("Bad destination offset 0x%x\n", dst_offset));
	if (((dst_pitch >> pixel_shift) & 0x7) != 0)
		ATI_FALLBACK(("Bad destination pitch 0x%x\n", dst_pitch));

	if (!R100TextureSetup(pSrcPicture, pSrc, 0))
		return FALSE;
	pp_cntl = RADEON_TEX_0_ENABLE | RADEON_TEX_BLEND_0_ENABLE;

	if (pMask != NULL) {
		if (!R100TextureSetup(pMaskPicture, pMask, 1))
			return FALSE;
		pp_cntl |= RADEON_TEX_1_ENABLE;
	} else {
		is_transform[1] = FALSE;
	}

	ENTER_DRAW(pDst);
	
	RadeonSwitchTo3D(atis);

	BEGIN_DMA(12);

	OUT_RING(DMA_PACKET0(RADEON_REG_PP_CNTL, 3));
	OUT_RING_REG(RADEON_REG_PP_CNTL, pp_cntl);
	OUT_RING_REG(RADEON_REG_RB3D_CNTL,
	    dst_format | RADEON_ALPHA_BLEND_ENABLE);
	OUT_RING_REG(RADEON_REG_RB3D_COLOROFFSET, dst_offset);

	OUT_REG(RADEON_REG_RB3D_COLORPITCH, dst_pitch >> pixel_shift);

	/* IN operator: Multiply src by mask components or mask alpha.
	 * BLEND_CTL_ADD is A * B + C.
	 * If a picture is a8, we have to explicitly zero its color values.
	 * If the destination is a8, we have to route the alpha to red, I think.
	 */
	cblend = RADEON_BLEND_CTL_ADD | RADEON_CLAMP_TX |
	    RADEON_COLOR_ARG_C_ZERO;
	ablend = RADEON_BLEND_CTL_ADD | RADEON_CLAMP_TX |
	    RADEON_ALPHA_ARG_C_ZERO;

	if (pDstPicture->format == PICT_a8)
		cblend |= RADEON_COLOR_ARG_A_T0_ALPHA;
	else if (pSrcPicture->format == PICT_a8)
		cblend |= RADEON_COLOR_ARG_A_ZERO;
	else
		cblend |= RADEON_COLOR_ARG_A_T0_COLOR;
	ablend |= RADEON_ALPHA_ARG_A_T0_ALPHA;

	if (pMask) {
		if (pMaskPicture->componentAlpha &&
		    pDstPicture->format != PICT_a8)
			cblend |= RADEON_COLOR_ARG_B_T1_COLOR;
		else
			cblend |= RADEON_COLOR_ARG_B_T1_ALPHA;
		ablend |= RADEON_ALPHA_ARG_B_T1_ALPHA;
	} else {
		cblend |= RADEON_COLOR_ARG_B_ZERO | RADEON_COMP_ARG_B;
		ablend |= RADEON_ALPHA_ARG_B_ZERO | RADEON_COMP_ARG_B;
	}

	OUT_REG(RADEON_REG_PP_TXCBLEND_0, cblend);
	OUT_REG(RADEON_REG_PP_TXABLEND_0, ablend);

	/* Op operator. */
	blendcntl = RadeonBlendOp[op].blend_cntl;
	if (PICT_FORMAT_A(pDstPicture->format) == 0 &&
	    RadeonBlendOp[op].dst_alpha) {
		if ((blendcntl & RADEON_SBLEND_MASK) ==
		    RADEON_SBLEND_GL_DST_ALPHA)
			blendcntl = (blendcntl & ~RADEON_SBLEND_MASK) |
			    RADEON_SBLEND_GL_ONE;
		else if ((blendcntl & RADEON_SBLEND_MASK) ==
		    RADEON_SBLEND_GL_INV_DST_ALPHA)
			blendcntl = (blendcntl & ~RADEON_SBLEND_MASK) |
			    RADEON_SBLEND_GL_ZERO;
	}
	OUT_REG(RADEON_REG_RB3D_BLENDCNTL, blendcntl);
	END_DMA();

	LEAVE_DRAW(pDst);

	return TRUE;
}

static Bool
R200CheckCompositeTexture(PicturePtr pPict, int unit)
{
	int w = pPict->pDrawable->width;
	int h = pPict->pDrawable->height;
	int i;

	if ((w > 0x7ff) || (h > 0x7ff))
		ATI_FALLBACK(("Picture w/h too large (%dx%d)\n", w, h));

	for (i = 0; i < sizeof(R200TexFormats) / sizeof(R200TexFormats[0]); i++)
	{
		if (R200TexFormats[i].fmt == pPict->format)
			break;
	}
	if (i == sizeof(R200TexFormats) / sizeof(R200TexFormats[0]))
		ATI_FALLBACK(("Unsupported picture format 0x%x\n",
		    pPict->format));

	if (pPict->repeat && ((w & (w - 1)) != 0 || (h & (h - 1)) != 0))
		ATI_FALLBACK(("NPOT repeat unsupported (%dx%d)\n", w, h));

	if (pPict->filter != PictFilterNearest &&
	    pPict->filter != PictFilterBilinear)
		ATI_FALLBACK(("Unsupported filter 0x%x\n", pPict->filter));

	return TRUE;
}

static Bool
R200TextureSetup(PicturePtr pPict, PixmapPtr pPix, int unit)
{
	ATIScreenInfo *atis = accel_atis;
	KdScreenPriv(pPix->drawable.pScreen);
	CARD32 txfilter, txformat, txoffset, txpitch;
	int w = pPict->pDrawable->width;
	int h = pPict->pDrawable->height;
	int i;
	RING_LOCALS;

	txpitch = pPix->devKind;
	txoffset = ((CARD8 *)pPix->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);

	for (i = 0; i < sizeof(R200TexFormats) / sizeof(R200TexFormats[0]); i++)
	{
		if (R200TexFormats[i].fmt == pPict->format)
			break;
	}
	txformat = R200TexFormats[i].card_fmt;
	if (R200TexFormats[i].byte_swap)
		txoffset |= R200_TXO_ENDIAN_BYTE_SWAP;

	if (pPict->repeat) {
		txformat |= ATILog2(w) << R200_TXFORMAT_WIDTH_SHIFT;
		txformat |= ATILog2(h) << R200_TXFORMAT_HEIGHT_SHIFT;
	} else 
		txformat |= R200_TXFORMAT_NON_POWER2;
	txformat |= unit << R200_TXFORMAT_ST_ROUTE_SHIFT;

	if ((txoffset & 0x1f) != 0)
		ATI_FALLBACK(("Bad texture offset 0x%x\n", txoffset));
	if ((txpitch & 0x1f) != 0)
		ATI_FALLBACK(("Bad texture pitch 0x%x\n", txpitch));

	switch (pPict->filter) {
	case PictFilterNearest:
		txfilter = (R200_MAG_FILTER_NEAREST |
			    R200_MIN_FILTER_NEAREST);
		break;
	case PictFilterBilinear:
		txfilter = (R200_MAG_FILTER_LINEAR |
			    R200_MIN_FILTER_LINEAR);
		break;
	default:
		ATI_FALLBACK(("Bad filter 0x%x\n", pPict->filter));
	}

	if (unit == 0) {
		BEGIN_DMA(6);
		OUT_RING(DMA_PACKET0(R200_REG_PP_TXFILTER_0 + 0x20 * unit, 5));
		OUT_RING_REG(R200_REG_PP_TXFILTER_0, txfilter);
		OUT_RING_REG(R200_REG_PP_TXFORMAT_0, txformat);
		OUT_RING_REG(R200_REG_PP_TXFORMAT_X_0, 0);
		OUT_RING_REG(R200_REG_PP_TXSIZE_0,
		    (pPix->drawable.width - 1) |
		    ((pPix->drawable.height - 1) << RADEON_TEX_VSIZE_SHIFT));
		OUT_RING_REG(R200_REG_PP_TXPITCH_0, txpitch - 32);
		END_DMA();
	} else {
		BEGIN_DMA(6);
		OUT_RING(DMA_PACKET0(R200_REG_PP_TXFILTER_1, 5));
		OUT_RING_REG(R200_REG_PP_TXFILTER_1, txfilter);
		OUT_RING_REG(R200_REG_PP_TXFORMAT_1, txformat);
		OUT_RING_REG(R200_REG_PP_TXFORMAT_X_1, 0);
		OUT_RING_REG(R200_REG_PP_TXSIZE_1,
		    (pPix->drawable.width - 1) |
		    ((pPix->drawable.height - 1) << RADEON_TEX_VSIZE_SHIFT));
		OUT_RING_REG(R200_REG_PP_TXPITCH_1, txpitch - 32);
		END_DMA();
	}

	BEGIN_DMA(2);
	OUT_REG(R200_PP_TXOFFSET_0 + 0x18 * unit, txoffset);
	END_DMA();

	if (pPict->transform != 0) {
		is_transform[unit] = TRUE;
		transform[unit] = pPict->transform;
	} else {
		is_transform[unit] = FALSE;
	}

	return TRUE;
}

Bool
R200CheckComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture)
{
	CARD32 tmp1;

	/* Check for unsupported compositing operations. */
	if (op >= sizeof(RadeonBlendOp) / sizeof(RadeonBlendOp[0]))
		ATI_FALLBACK(("Unsupported Composite op 0x%x\n", op));
	if (pMaskPicture != NULL && pMaskPicture->componentAlpha &&
	    RadeonBlendOp[op].src_alpha)
		ATI_FALLBACK(("Component alpha not supported with source "
		    "alpha blending.\n"));

	if (!R200CheckCompositeTexture(pSrcPicture, 0))
		return FALSE;
	if (pMaskPicture != NULL && !R200CheckCompositeTexture(pMaskPicture, 1))
		return FALSE;

	if (!RadeonGetDestFormat(pDstPicture, &tmp1))
		return FALSE;

	return TRUE;
}

Bool
R200PrepareComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
    PicturePtr pDstPicture, PixmapPtr pSrc, PixmapPtr pMask, PixmapPtr pDst)
{
	KdScreenPriv(pDst->drawable.pScreen);
	ATIScreenInfo(pScreenPriv);
	CARD32 dst_format, dst_offset, dst_pitch;
	CARD32 pp_cntl, blendcntl, cblend, ablend;
	int pixel_shift;
	RING_LOCALS;

	RadeonGetDestFormat(pDstPicture, &dst_format);
	pixel_shift = pDst->drawable.bitsPerPixel >> 4;

	accel_atis = atis;

	dst_offset = ((CARD8 *)pDst->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);
	dst_pitch = pDst->devKind;
	if ((dst_offset & 0x0f) != 0)
		ATI_FALLBACK(("Bad destination offset 0x%x\n", dst_offset));
	if (((dst_pitch >> pixel_shift) & 0x7) != 0)
		ATI_FALLBACK(("Bad destination pitch 0x%x\n", dst_pitch));

	if (!R200TextureSetup(pSrcPicture, pSrc, 0))
		return FALSE;
	pp_cntl = RADEON_TEX_0_ENABLE | RADEON_TEX_BLEND_0_ENABLE;

	if (pMask != NULL) {
		if (!R200TextureSetup(pMaskPicture, pMask, 1))
			return FALSE;
		pp_cntl |= RADEON_TEX_1_ENABLE;
	} else {
		is_transform[1] = FALSE;
	}

	RadeonSwitchTo3D(atis);

	BEGIN_DMA(17);

	OUT_RING(DMA_PACKET0(RADEON_REG_PP_CNTL, 3));
	OUT_RING_REG(RADEON_REG_PP_CNTL, pp_cntl);
	OUT_RING_REG(RADEON_REG_RB3D_CNTL, dst_format | RADEON_ALPHA_BLEND_ENABLE);
	OUT_RING_REG(RADEON_REG_RB3D_COLOROFFSET, dst_offset);

	OUT_REG(R200_REG_SE_VTX_FMT_0, R200_VTX_XY);
	OUT_REG(R200_REG_SE_VTX_FMT_1,
	    (2 << R200_VTX_TEX0_COMP_CNT_SHIFT) |
	    (2 << R200_VTX_TEX1_COMP_CNT_SHIFT));

	OUT_REG(RADEON_REG_RB3D_COLORPITCH, dst_pitch >> pixel_shift);

	/* IN operator: Multiply src by mask components or mask alpha.
	 * BLEND_CTL_ADD is A * B + C.
	 * If a picture is a8, we have to explicitly zero its color values.
	 * If the destination is a8, we have to route the alpha to red, I think.
	 */
	cblend = R200_TXC_OP_MADD | R200_TXC_ARG_C_ZERO;
	ablend = R200_TXA_OP_MADD | R200_TXA_ARG_C_ZERO;

	if (pDstPicture->format == PICT_a8)
		cblend |= R200_TXC_ARG_A_R0_ALPHA;
	else if (pSrcPicture->format == PICT_a8)
		cblend |= R200_TXC_ARG_A_ZERO;
	else
		cblend |= R200_TXC_ARG_A_R0_COLOR;
	ablend |= R200_TXA_ARG_A_R0_ALPHA;

	if (pMask) {
		if (pMaskPicture->componentAlpha &&
		    pDstPicture->format != PICT_a8)
			cblend |= R200_TXC_ARG_B_R1_COLOR;
		else
			cblend |= R200_TXC_ARG_B_R1_ALPHA;
		ablend |= R200_TXA_ARG_B_R1_ALPHA;
	} else {
		cblend |= R200_TXC_ARG_B_ZERO | R200_TXC_COMP_ARG_B;
		ablend |= R200_TXA_ARG_B_ZERO | R200_TXA_COMP_ARG_B;
	}

	OUT_RING(DMA_PACKET0(R200_REG_PP_TXCBLEND_0, 4));
	OUT_RING_REG(R200_REG_PP_TXCBLEND_0, cblend);
	OUT_RING_REG(R200_REG_PP_TXCBLEND2_0,
	    R200_TXC_CLAMP_0_1 | R200_TXC_OUTPUT_REG_R0);
	OUT_RING_REG(R200_REG_PP_TXABLEND_0, ablend);
	OUT_RING_REG(R200_REG_PP_TXABLEND2_0,
	    R200_TXA_CLAMP_0_1 | R200_TXA_OUTPUT_REG_R0);

	/* Op operator. */
	blendcntl = RadeonBlendOp[op].blend_cntl;
	if (PICT_FORMAT_A(pDstPicture->format) == 0 &&
	    RadeonBlendOp[op].dst_alpha) {
		if ((blendcntl & RADEON_SBLEND_MASK) ==
		    RADEON_SBLEND_GL_DST_ALPHA)
			blendcntl = (blendcntl & ~RADEON_SBLEND_MASK) |
			    RADEON_SBLEND_GL_ONE;
		else if ((blendcntl & RADEON_SBLEND_MASK) ==
		    RADEON_SBLEND_GL_INV_DST_ALPHA)
			blendcntl = (blendcntl & ~RADEON_SBLEND_MASK) |
			    RADEON_SBLEND_GL_ZERO;
	}
	OUT_REG(RADEON_REG_RB3D_BLENDCNTL, blendcntl);
	END_DMA();

	return TRUE;
}

union intfloat {
	float f;
	CARD32 i;
};

struct blend_vertex {
	union intfloat x, y;
	union intfloat s0, t0;
	union intfloat s1, t1;
};

#define VTX_DWORD_COUNT 6

#define VTX_OUT(_dstX, _dstY, _srcX, _srcY, _maskX, _maskY)		\
do {									\
	OUT_RING_F(_dstX);						\
	OUT_RING_F(_dstY);						\
	OUT_RING_F(_srcX);						\
	OUT_RING_F(_srcY);						\
	OUT_RING_F(_maskX);						\
	OUT_RING_F(_maskY);						\
} while (0)

void
RadeonComposite(int srcX, int srcY, int maskX, int maskY, int dstX, int dstY,
    int w, int h)
{
	ATIScreenInfo *atis = accel_atis;
	ATICardInfo *atic = atis->atic;
	int srcXend, srcYend, maskXend, maskYend;
	RING_LOCALS;
	PictVector v;

	ENTER_DRAW(0);
	
	/*ErrorF("RadeonComposite (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",
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

	if (atic->is_r100) {
		BEGIN_DMA(4 * VTX_DWORD_COUNT + 3);
		OUT_RING(DMA_PACKET3(RADEON_CP_PACKET3_3D_DRAW_IMMD,
		    4 * VTX_DWORD_COUNT + 2));
		OUT_RING(RADEON_CP_VC_FRMT_XY |
		    RADEON_CP_VC_FRMT_ST0 |
		    RADEON_CP_VC_FRMT_ST1);
		OUT_RING(RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_FAN |
		    RADEON_CP_VC_CNTL_PRIM_WALK_RING |
		    RADEON_CP_VC_CNTL_MAOS_ENABLE |
		    RADEON_CP_VC_CNTL_VTX_FMT_RADEON_MODE |
		    (4 << RADEON_CP_VC_CNTL_NUM_SHIFT));
	} else {
		BEGIN_DMA(4 * VTX_DWORD_COUNT + 2);
		OUT_RING(DMA_PACKET3(R200_CP_PACKET3_3D_DRAW_IMMD_2,
		    4 * VTX_DWORD_COUNT + 1));
		OUT_RING(RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_FAN |
		    RADEON_CP_VC_CNTL_PRIM_WALK_RING |
		    (4 << RADEON_CP_VC_CNTL_NUM_SHIFT));
	}

	VTX_OUT(dstX,     dstY,     srcX,    srcY,    maskX,    maskY);
	VTX_OUT(dstX,     dstY + h, srcX,    srcYend, maskX,    maskYend);
	VTX_OUT(dstX + w, dstY + h, srcXend, srcYend, maskXend, maskYend);
	VTX_OUT(dstX + w, dstY,     srcXend, srcY,    maskXend, maskY);

	LEAVE_DRAW(0);

	END_DMA();
}

void
RadeonDoneComposite(void)
{
	ENTER_DRAW(0);
	LEAVE_DRAW(0);
}

Bool
RadeonPrepareTrapezoids(PicturePtr pDstPicture, PixmapPtr pDst)
{
	KdScreenPriv(pDst->drawable.pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	CARD32 dst_offset, dst_pitch;
	int pixel_shift;
	RING_LOCALS;

	pixel_shift = pDst->drawable.bitsPerPixel >> 4;

	accel_atis = atis;

	dst_offset = ((CARD8 *)pDst->devPrivate.ptr -
	    pScreenPriv->screen->memory_base);
	dst_pitch = pDst->devKind;
	if ((dst_offset & 0x0f) != 0)
		ATI_FALLBACK(("Bad destination offset 0x%x\n", dst_offset));
	if (((dst_pitch >> pixel_shift) & 0x7) != 0)
		ATI_FALLBACK(("Bad destination pitch 0x%x\n", dst_pitch));

	RadeonSwitchTo3D(atis);

	BEGIN_DMA(8);

	OUT_RING(DMA_PACKET0(RADEON_REG_PP_CNTL, 5));
	OUT_RING_REG(RADEON_REG_PP_CNTL, RADEON_TEX_BLEND_0_ENABLE);
	OUT_RING_REG(RADEON_REG_RB3D_CNTL,
	    RADEON_COLOR_FORMAT_RGB8 | RADEON_ALPHA_BLEND_ENABLE);
	OUT_RING_REG(RADEON_REG_RB3D_COLOROFFSET, dst_offset);
	OUT_RING_REG(RADEON_REG_RE_WIDTH_HEIGHT,
	    ((pDst->drawable.height - 1) << 16) |
	    (pDst->drawable.width - 1));
	OUT_RING_REG(RADEON_REG_RB3D_COLORPITCH, dst_pitch >> pixel_shift);
	OUT_REG(RADEON_REG_RB3D_BLENDCNTL, RadeonBlendOp[PictOpAdd].blend_cntl);
	END_DMA();

	if (atic->is_r100) {
		BEGIN_DMA(4);
		OUT_RING(DMA_PACKET0(RADEON_REG_PP_TXCBLEND_0, 3));
		OUT_RING_REG(RADEON_REG_PP_TXCBLEND_0,
		    RADEON_BLEND_CTL_ADD | RADEON_CLAMP_TX |
		    RADEON_COLOR_ARG_C_TFACTOR_ALPHA);
		OUT_RING_REG(RADEON_REG_PP_TXABLEND_0,
		    RADEON_BLEND_CTL_ADD | RADEON_CLAMP_TX |
		    RADEON_ALPHA_ARG_C_TFACTOR_ALPHA);
		OUT_RING_REG(RADEON_REG_PP_TFACTOR_0, 0x01000000);
		END_DMA();
	} else if (atic->is_r200) {
		BEGIN_DMA(14);
		OUT_REG(R200_REG_SE_VTX_FMT_0, R200_VTX_XY);
		OUT_REG(R200_REG_SE_VTX_FMT_1, 0);
		OUT_REG(R200_REG_PP_TXCBLEND_0,
		    R200_TXC_ARG_C_TFACTOR_COLOR);
		OUT_REG(R200_REG_PP_TXABLEND_0,
		    R200_TXA_ARG_C_TFACTOR_ALPHA);
		OUT_REG(R200_REG_PP_TXCBLEND2_0, R200_TXC_OUTPUT_REG_R0);
		OUT_REG(R200_REG_PP_TXABLEND2_0, R200_TXA_OUTPUT_REG_R0);
		OUT_REG(RADEON_REG_PP_TFACTOR_0, 0x01000000);
		END_DMA();
	}

	return TRUE;
}

#define TRAP_VERT_RING_COUNT 2

#define TRAP_VERT(_x, _y)						\
do {									\
	OUT_RING_F((_x) + sample_x);					\
	OUT_RING_F((_y) + sample_y);					\
} while (0)

void
RadeonTrapezoids(KaaTrapezoid *traps, int ntraps)
{
	ATIScreenInfo *atis = accel_atis;
	ATICardInfo *atic = atis->atic;
	RING_LOCALS;

	while (ntraps > 0) {
		int i, sample, count, vertcount;

		count = 0xffff / 4 / sample_count;
		if (count > ntraps)
			count = ntraps;
		vertcount = count * sample_count * 4;

		if (atic->is_r100) {
			BEGIN_DMA(3 + vertcount * TRAP_VERT_RING_COUNT);
			OUT_RING(DMA_PACKET3(RADEON_CP_PACKET3_3D_DRAW_IMMD,
			    2 + vertcount * TRAP_VERT_RING_COUNT));
			OUT_RING(RADEON_CP_VC_FRMT_XY);
			OUT_RING(RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_FAN |
			    RADEON_CP_VC_CNTL_PRIM_WALK_RING |
			    RADEON_CP_VC_CNTL_MAOS_ENABLE |
			    RADEON_CP_VC_CNTL_VTX_FMT_RADEON_MODE |
			    (vertcount << RADEON_CP_VC_CNTL_NUM_SHIFT));
		} else {
			BEGIN_DMA(2 + vertcount * TRAP_VERT_RING_COUNT);
			OUT_RING(DMA_PACKET3(R200_CP_PACKET3_3D_DRAW_IMMD_2,
			    1 + vertcount * TRAP_VERT_RING_COUNT));
			OUT_RING(RADEON_CP_VC_CNTL_PRIM_TYPE_TRI_FAN |
			    RADEON_CP_VC_CNTL_PRIM_WALK_RING |
			    (vertcount << RADEON_CP_VC_CNTL_NUM_SHIFT));
		}

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
RadeonDoneTrapezoids(void)
{
	ATIScreenInfo *atis = accel_atis;
	RING_LOCALS;

	BEGIN_DMA(2);
	OUT_REG(RADEON_REG_RE_WIDTH_HEIGHT, 0xffffffff);
	END_DMA();
}
