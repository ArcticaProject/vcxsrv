/*
 * Copyright © 2004 Eric Anholt
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

#include <sys/time.h>

#include "ati.h"
#include "ati_reg.h"
#include "ati_dma.h"
#include "ati_draw.h"

#ifdef USE_DRI
#include "radeon_common.h"
#include "r128_common.h"
#include "ati_sarea.h"
#endif /* USE_DRI */

#include "agp.h"

#define DEBUG_FIFO 0

extern CARD32 r128_cce_microcode[];
extern CARD32 radeon_cp_microcode[][2];
extern CARD32 r200_cp_microcode[][2];
extern CARD32 r300_cp_microcode[][2];

#if DEBUG_FIFO
static void
ATIDebugFifo(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	char *mmio = atic->reg_base;

	if (atic->is_radeon) {
		ErrorF("RADEON_REG_CP_CSQ_CNTL: 0x%08x\n",
		    MMIO_IN32(mmio, RADEON_REG_CP_CSQ_CNTL));
		ErrorF("RADEON_REG_CP_CSQ_STAT: 0x%08x\n",
		    MMIO_IN32(mmio, RADEON_REG_CP_CSQ_STAT));
		ErrorF("RADEON_REG_RBBM_STATUS: 0x%08x\n",
		    MMIO_IN32(mmio, RADEON_REG_RBBM_STATUS));
		ErrorF("RADEON_REG_RB3D_DSTCACHE_CTLSTAT: 0x%08x\n",
		    MMIO_IN32(mmio, RADEON_REG_RB3D_DSTCACHE_CTLSTAT));
	} else {
		ErrorF("R128_REG_PM4_BUFFER_CNTL: 0x%08x\n",
		    MMIO_IN32(mmio, R128_REG_PM4_BUFFER_CNTL));
		ErrorF("R128_REG_PM4_STAT: 0x%08x\n",
		    MMIO_IN32(mmio, R128_REG_PM4_STAT));
		ErrorF("R128_REG_GUI_STAT: 0x%08x\n",
		    MMIO_IN32(mmio, R128_REG_GUI_STAT));
		ErrorF("R128_REG_PC_NGUI_CTLSTAT: 0x%08x\n",
		    MMIO_IN32(mmio, R128_REG_PC_NGUI_CTLSTAT));
	}
}
#endif

static void
ATIUploadMicrocode(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	char *mmio = atic->reg_base;
	int i;

	MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_ADDR, 0);
	if (atic->is_radeon && atic->is_r300) {
		for (i = 0; i < 256; i++) {
			MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_DATAH,
			    r300_cp_microcode[i][1]);
			MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_DATAL,
			    r300_cp_microcode[i][0]);
		}
	} else if (atic->is_radeon && atic->is_r200) {
		for (i = 0; i < 256; i++) {
			MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_DATAH,
			    r200_cp_microcode[i][1]);
			MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_DATAL,
			    r200_cp_microcode[i][0]);
		}
	} else if (atic->is_radeon && atic->is_r100) {
		for (i = 0; i < 256; i++) {
			MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_DATAH,
			    radeon_cp_microcode[i][1]);
			MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_DATAL,
			    radeon_cp_microcode[i][0]);
		}
	} else {
		for (i = 0; i < 256; i++) {
			MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_DATAH,
			    r128_cce_microcode[i * 2]);
			MMIO_OUT32(mmio, ATI_REG_MICROCODE_RAM_DATAL,
			    r128_cce_microcode[i * 2 + 1]);
		}
	}
}

/* Required when reading from video memory after acceleration to make sure all
 * data has been flushed to video memory from the pixel cache.
 */
static void
ATIFlushPixelCache(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	char *mmio = atic->reg_base;
	CARD32 temp;
	TIMEOUT_LOCALS;

	if (atic->is_radeon) {
		temp = MMIO_IN32(mmio, RADEON_REG_RB3D_DSTCACHE_CTLSTAT);
		temp |= RADEON_RB3D_DC_FLUSH_ALL;
		MMIO_OUT32(mmio, RADEON_REG_RB3D_DSTCACHE_CTLSTAT, temp);

		WHILE_NOT_TIMEOUT(.2) {
			if ((MMIO_IN32(mmio, RADEON_REG_RB3D_DSTCACHE_CTLSTAT) &
			    RADEON_RB3D_DC_BUSY) == 0)
				break;
		}
	} else {
		temp = MMIO_IN32(mmio, R128_REG_PC_NGUI_CTLSTAT);
		temp |= R128_PC_FLUSH_ALL;
		MMIO_OUT32(mmio, R128_REG_PC_NGUI_CTLSTAT, temp);

		WHILE_NOT_TIMEOUT(.2) {
			if ((MMIO_IN32(mmio, R128_REG_PC_NGUI_CTLSTAT) &
			    R128_PC_BUSY) != R128_PC_BUSY)
				break;
		}
	}
	if (TIMEDOUT())
		ErrorF("Timeout flushing pixel cache.\n");
}

static void
ATIEngineReset(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	char *mmio = atic->reg_base;
	CARD32 clockcntlindex, mclkcntl;

#if DEBUG_FIFO
	ErrorF("Engine Reset!\n");
	ATIDebugFifo(atis);
#endif

	ATIFlushPixelCache(atis);

	clockcntlindex = MMIO_IN32(mmio, ATI_REG_CLOCK_CNTL_INDEX);
	if (atic->is_r300)
		R300CGWorkaround(atis);

	if (atic->is_radeon) {
		CARD32 host_path_cntl;

		mclkcntl = INPLL(mmio, RADEON_REG_MCLK_CNTL);

		OUTPLL(mmio, RADEON_REG_MCLK_CNTL, mclkcntl |
		    RADEON_FORCEON_MCLKA |
		    RADEON_FORCEON_MCLKB |
		    RADEON_FORCEON_YCLKA |
		    RADEON_FORCEON_YCLKB |
		    RADEON_FORCEON_MC |
		    RADEON_FORCEON_AIC);

		host_path_cntl = MMIO_IN32(mmio, RADEON_REG_HOST_PATH_CNTL);

		if (atic->is_r300) {
			MMIO_OUT32(mmio, RADEON_REG_RBBM_SOFT_RESET,
			    RADEON_SOFT_RESET_CP |
			    RADEON_SOFT_RESET_HI |
			    RADEON_SOFT_RESET_E2);
		} else {
			MMIO_OUT32(mmio, RADEON_REG_RBBM_SOFT_RESET,
			    RADEON_SOFT_RESET_CP |
			    RADEON_SOFT_RESET_SE |
			    RADEON_SOFT_RESET_RE |
			    RADEON_SOFT_RESET_PP |
			    RADEON_SOFT_RESET_E2 |
			    RADEON_SOFT_RESET_RB);
		}
		MMIO_IN32(mmio, RADEON_REG_RBBM_SOFT_RESET);
		MMIO_OUT32(mmio, RADEON_REG_RBBM_SOFT_RESET, 0);

		MMIO_OUT32(mmio, RADEON_REG_HOST_PATH_CNTL, host_path_cntl |
		    RADEON_HDP_SOFT_RESET);
		MMIO_IN32(mmio, RADEON_REG_HOST_PATH_CNTL);
		MMIO_OUT32(mmio, RADEON_REG_HOST_PATH_CNTL, host_path_cntl);

		MMIO_OUT32(mmio, ATI_REG_CLOCK_CNTL_INDEX, clockcntlindex);
		OUTPLL(mmio, RADEON_REG_MCLK_CNTL, mclkcntl);
		if (atic->is_r300)
			R300CGWorkaround(atis);
	} else {
		CARD32 temp;

		mclkcntl = INPLL(mmio, R128_REG_MCLK_CNTL);

		OUTPLL(mmio, R128_REG_MCLK_CNTL,
		    mclkcntl | R128_FORCE_GCP | R128_FORCE_PIPE3D_CP);

		temp = MMIO_IN32(mmio, R128_REG_GEN_RESET_CNTL);
		MMIO_OUT32(mmio, R128_REG_GEN_RESET_CNTL,
		    temp | R128_SOFT_RESET_GUI);
		temp = MMIO_IN32(mmio, R128_REG_GEN_RESET_CNTL);
		MMIO_OUT32(mmio, R128_REG_GEN_RESET_CNTL,
		    temp & ~R128_SOFT_RESET_GUI);
		temp = MMIO_IN32(mmio, R128_REG_GEN_RESET_CNTL);

		OUTPLL(mmio, R128_REG_MCLK_CNTL, mclkcntl);
		MMIO_OUT32(mmio, ATI_REG_CLOCK_CNTL_INDEX, clockcntlindex);
	}
#ifdef USE_DRI
	if (atis->using_dri) {
		ATIDRIDMAReset(atis);
		ATIDRIDMAStart(atis);
	}
#endif
}

static void
ATIWaitAvailMMIO(ATIScreenInfo *atis, int n)
{
	ATICardInfo *atic = atis->atic;
	char *mmio = atic->reg_base;
	TIMEOUT_LOCALS;

	if (atis->mmio_avail >= n) {
		atis->mmio_avail -= n;
		return;
	}
	if (atic->is_radeon) {
		WHILE_NOT_TIMEOUT(.2) {
			atis->mmio_avail = MMIO_IN32(mmio,
			    RADEON_REG_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK;
			if (atis->mmio_avail >= n)
				break;
		}
	} else {
		WHILE_NOT_TIMEOUT(.2) {
			atis->mmio_avail = MMIO_IN32(mmio, R128_REG_GUI_STAT) &
			    0xfff;
			if (atis->mmio_avail >= n)
				break;
		}
	}
	if (TIMEDOUT()) {
		ErrorF("Timeout waiting for %d MMIO slots.\n", n);
		ATIEngineReset(atis);
		ATIDrawSetup(atis->screen->pScreen);
	}
	atis->mmio_avail -= n;
}

static int
ATIGetAvailPrimary(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	char *mmio = atic->reg_base;

	if (atic->is_radeon) {
		int csq_stat, diff;
	
		csq_stat = MMIO_IN32(mmio, RADEON_REG_CP_CSQ_STAT);
		if (atic->is_r200)
			diff = ((csq_stat & R200_CSQ_WPTR_PRIMARY_MASK) >> 9) - 
			    (csq_stat & R200_CSQ_RPTR_PRIMARY_MASK);
		else
			diff = ((csq_stat & RADEON_CSQ_WPTR_PRIMARY_MASK) >> 8) - 
			    (csq_stat & RADEON_CSQ_RPTR_PRIMARY_MASK);
	
		if (diff < 0)
			return -diff;
		else
			return atis->cce_pri_size - diff;
	} else {
		return MMIO_IN32(mmio, R128_REG_PM4_STAT) &
		    R128_PM4_FIFOCNT_MASK;
	}
}

static void
ATIWaitAvailPrimary(ATIScreenInfo *atis, int n)
{
	TIMEOUT_LOCALS;

	if (atis->cce_pri_avail >= n) {
		atis->cce_pri_avail -= n;
		return;
	}

	WHILE_NOT_TIMEOUT(.2) {
		if (atis->cce_pri_avail >= n)
			break;
		atis->cce_pri_avail = ATIGetAvailPrimary(atis);
		if (atis->cce_pri_avail >= n)
			break;
	}
	if (TIMEDOUT()) {
		ErrorF("Timeout waiting for %d CCE slots (%d avail).\n", n,
		    atis->cce_pri_avail);
		ATIEngineReset(atis);
		ATIDrawSetup(atis->screen->pScreen);
	}
	atis->cce_pri_avail -= n;
}

void
ATIWaitIdle(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	char *mmio = atic->reg_base;
	TIMEOUT_LOCALS;

	if (atis->indirectBuffer != NULL)
		ATIFlushIndirect(atis, 0);

#ifdef USE_DRI
	if (atis->using_dri) {
		int ret = 0;
		int cmd = (atic->is_radeon ? DRM_RADEON_CP_IDLE :
		    DRM_R128_CCE_IDLE);
		WHILE_NOT_TIMEOUT(2) {
			ret = drmCommandNone(atic->drmFd, cmd);
			if (ret != -EBUSY)
				break;
		}
		if (TIMEDOUT()) {
			ATIDebugFifo(atis);
			FatalError("Timed out idling CCE (card hung)\n");
		}
		if (ret != 0)
			ErrorF("Failed to idle DMA, returned %d\n", ret);
		return;
	}
#endif

	if (!atic->is_radeon && (atis->using_pseudo || atis->using_dma)) {
		ATIWaitAvailPrimary(atis, atis->cce_pri_size);

		WHILE_NOT_TIMEOUT(.2) {
			if ((MMIO_IN32(mmio, R128_REG_PM4_STAT) &
			    (R128_PM4_BUSY | R128_PM4_GUI_ACTIVE)) == 0)
				break;
		}
		if (TIMEDOUT()) {
			ErrorF("Timeout idling CCE, resetting...\n");
			ATIEngineReset(atis);
			ATIDrawSetup(atis->screen->pScreen);
		}
	}

	/* Radeon CP idle is the same as MMIO idle. */
	if (atis->using_pio || atic->is_radeon) {
		/* Empty the fifo */
		ATIWaitAvailMMIO(atis, 64);

		if (atic->is_radeon) {
			WHILE_NOT_TIMEOUT(.2) {
				if ((MMIO_IN32(mmio, RADEON_REG_RBBM_STATUS) &
				    RADEON_RBBM_ACTIVE) == 0)
					break;
			}
		} else {
			WHILE_NOT_TIMEOUT(.2) {
				if ((MMIO_IN32(mmio, R128_REG_GUI_STAT) &
				    R128_GUI_ACTIVE) == 0)
					break;
			}
		}
		if (TIMEDOUT()) {
			ErrorF("Timeout idling accelerator, resetting...\n");
			ATIEngineReset(atis);
			ATIDrawSetup(atis->screen->pScreen);
		}
	}

	ATIFlushPixelCache(atis);

#if DEBUG_FIFO
	ErrorF("Idle?\n");
	ATIDebugFifo(atis);
#endif
}

dmaBuf *
ATIGetDMABuffer(ATIScreenInfo *atis)
{
	dmaBuf *buf;

	buf = (dmaBuf *)xalloc(sizeof(dmaBuf));
	if (buf == NULL)
		return NULL;

#ifdef USE_DRI
	if (atis->using_dri) {
		buf->drmBuf = ATIDRIGetBuffer(atis);
		if (buf->drmBuf == NULL) {
			xfree(buf);
			return NULL;
		}
		buf->size = buf->drmBuf->total;
		buf->used = buf->drmBuf->used;
		buf->address = buf->drmBuf->address;
		return buf;
	}
#endif /* USE_DRI */

	if (atis->using_dma)
		buf->size = atis->ring_len / 2;
	else
		buf->size = 512 * 1024;
	buf->address = xalloc(buf->size);
	if (buf->address == NULL) {
		xfree(buf);
		return NULL;
	}
	buf->used = 0;

	return buf;
}

/* Decode a type-3 packet into MMIO register writes. Only some type-3 packets
 * supported, and only partially.
 */
static void
ATIDispatchPacket3MMIO(ATIScreenInfo *atis, CARD32 header, CARD32 *addr,
    int count)
{
	ATICardInfo *atic = atis->atic;
	char *mmio = atic->reg_base;
	CARD32 settings;
	int i = 0;

	settings = addr[i++];

	if ((settings & ATI_GMC_SRC_PITCH_OFFSET_CNTL) != 0)
		MMIO_OUT32(mmio, ATI_REG_SRC_PITCH_OFFSET, addr[i++]);
	if ((settings & ATI_GMC_DST_PITCH_OFFSET_CNTL) != 0)
		MMIO_OUT32(mmio, ATI_REG_DST_PITCH_OFFSET, addr[i++]);
	if ((settings & ATI_GMC_BRUSH_MASK) == ATI_GMC_BRUSH_SOLID_COLOR)
		MMIO_OUT32(mmio, ATI_REG_DP_BRUSH_FRGD_CLR, addr[i++]);

	switch (header & (ATI_CCE_PACKETTYPE_MASK |
	    ATI_CCE_PACKET3_IT_OPCODE_MASK))
	{
	case ATI_CCE_PACKET3_PAINT_MULTI:
		while (i < count) {
			MMIO_OUT32(mmio, ATI_REG_DST_Y_X,
			    (addr[i] >> 16) | (addr[i] << 16));
			i++;
			MMIO_OUT32(mmio, ATI_REG_DST_HEIGHT_WIDTH,
			    (addr[i] >> 16) | (addr[i] << 16));
			i++;
		}
		break;
	case ATI_CCE_PACKET3_BITBLT_MULTI:
		while (i < count) {
			MMIO_OUT32(mmio, ATI_REG_SRC_Y_X,
			    (addr[i] >> 16) | (addr[i] << 16));
			i++;
			MMIO_OUT32(mmio, ATI_REG_DST_Y_X,
			    (addr[i] >> 16) | (addr[i] << 16));
			i++;
			MMIO_OUT32(mmio, ATI_REG_DST_HEIGHT_WIDTH,
			    (addr[i] >> 16) | (addr[i] << 16));
			i++;
		}
		break;
	default:
		ErrorF("Unsupported packet: 0x%x\n", header);
	}
}

/* Dispatch packets by decoding them and writing to registers.  Doesn't support
 * the type 3 packets.
 */
static void
ATIDispatchIndirectMMIO(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	dmaBuf *buf = atis->indirectBuffer;
	char *mmio = atic->reg_base;
	CARD32 *addr;
	CARD32 reg;
	int i, n, count;

	addr = (CARD32 *)((char *)buf->address + atis->indirectStart);
	count = (buf->used - atis->indirectStart) / 4;

	for (i = 0; i < count; i++) {
		CARD32 header = addr[i];

		switch (header & ATI_CCE_PACKETTYPE_MASK)
		{
		case ATI_CCE_PACKET0:
			n = ((header & ATI_CCE_PACKET0_COUNT_MASK) >> 16) + 1;
			reg = (header & ATI_CCE_PACKET0_REG_MASK) << 2;
			ATIWaitAvailMMIO(atis, n);
			while (n > 0) {
				i++;
				MMIO_OUT32(mmio, reg, addr[i]);
				if ((header & ATI_CCE_PACKET0_ONE_REG_WR) == 0)
					reg += 4;
				n--;
			}
			break;
		case ATI_CCE_PACKET1:
			reg = (header & ATI_CCE_PACKET1_REG_1) << 2;
			MMIO_OUT32(mmio, reg, addr[++i]);
			reg = ((header & ATI_CCE_PACKET1_REG_2) >>
			    ATI_CCE_PACKET1_REG_2_SHIFT) << 2;
			MMIO_OUT32(mmio, reg, addr[++i]);
			break;
		case ATI_CCE_PACKET2:
			/* PACKET2 is a no-op packet. */
			break;
		case ATI_CCE_PACKET3:
			n = ((header & ATI_CCE_PACKET3_COUNT_MASK) >> 16) + 1;
			ATIDispatchPacket3MMIO(atis, header, &addr[i], n);
			i += n;
			break;
		default:
			ErrorF("Unsupported packet: 0x%x\n", addr[i]);
		}
	}
}

/* Dispatch packets by sending them through the MMIO aperture. */
static void
R128DispatchIndirectPDMA(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	dmaBuf *buf = atis->indirectBuffer;
	char *mmio = atic->reg_base;
	CARD32 *addr;
	int count;

	addr = (CARD32 *)((char *)buf->address + atis->indirectStart);
	count = (buf->used - atis->indirectStart) / 4;

	while (count > 1) {
		ATIWaitAvailPrimary(atis, 2);
		MMIO_OUT32(mmio, R128_REG_PM4_FIFO_DATA_EVEN, *addr++);
		MMIO_OUT32(mmio, R128_REG_PM4_FIFO_DATA_ODD, *addr++);
		count -= 2;
	}

	/* Submit last DWORD if necessary. */
	if (count != 0) {
		ATIWaitAvailPrimary(atis, 2);
		MMIO_OUT32(mmio, R128_REG_PM4_FIFO_DATA_EVEN, *addr++);
		MMIO_OUT32(mmio, R128_REG_PM4_FIFO_DATA_ODD, ATI_CCE_PACKET2);
	}
}

/* Dispatch packets by sending them through the MMIO aperture, using the
 * primary CCE ring. */
static void
RadeonDispatchIndirectPDMA(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	dmaBuf *buf = atis->indirectBuffer;
	char *mmio = atic->reg_base;
	CARD32 *addr;
	int count, avail, reg, i;
	TIMEOUT_LOCALS;

	addr = (CARD32 *)((char *)buf->address + atis->indirectStart);
	count = (buf->used - atis->indirectStart) / 4;

	reg = RADEON_REG_CSQ_APER_PRIMARY;
	WHILE_NOT_TIMEOUT(3) {
		/* 3 seconds is empirical, using render_bench on an r100. */
		if (count <= 0)
			break;
		avail = ATIGetAvailPrimary(atis);
		for (i = 0; i < min(count, avail); i++) {
			MMIO_OUT32(mmio, reg, *addr++);
			if (reg == RADEON_REG_CSQ_APER_PRIMARY_END)
				reg = RADEON_REG_CSQ_APER_PRIMARY;
			else
				reg += 4;
		}
		count -= i;
	}
	if (TIMEDOUT()) {
		ErrorF("Timeout submitting packets, resetting...\n");
		ATIEngineReset(atis);
		ATIDrawSetup(atis->screen->pScreen);
	}
}


/* Dispatch packets by writing them to the (primary) ring buffer, which happens
 * to be in framebuffer memory.
 */
static void
R128DispatchIndirectDMA(ATIScreenInfo *atis)
{
	ATICardInfo *atic = atis->atic;
	dmaBuf *buf = atis->indirectBuffer;
	char *mmio = atic->reg_base;
	CARD32 *addr;
	int count, ring_count;
	TIMEOUT_LOCALS;

	addr = (CARD32 *)((char *)buf->address + atis->indirectStart);
	count = (buf->used - atis->indirectStart) / 4;
	ring_count = atis->ring_len / 4;

	WHILE_NOT_TIMEOUT(.2) {
		if (count <= 0)
			break;

		atis->ring_addr[atis->ring_write++] = *addr++;
		if (atis->ring_write >= ring_count)
			atis->ring_write = 0;
		while (atis->ring_write == atis->ring_read) {
			atis->ring_read = MMIO_IN32(mmio, ATI_REG_CCE_RPTR);
		}
		count--;
	}
	if (TIMEDOUT()) {
		ErrorF("Timeout submitting packets, resetting...\n");
		ATIEngineReset(atis);
		ATIDrawSetup(atis->screen->pScreen);
	}
		
	/* Workaround for some early Rage 128 ASIC spins where the CCE parser
	 * may read up to 32 DWORDS beyond the end of the ring buffer memory
	 * before wrapping around, if the ring buffer was empty and a <32 DWORD
	 * packet that wraps around the end of the ring buffer is submitted.
	 * To work around that, copy the beginning of the ring buffer past the
	 * end if that may happen.
	 */
	if (atis->ring_write < 32)
		memcpy(atis->ring_addr + ring_count, atis->ring_addr, 32 * 4);

	/* Update write pointer */
	MMIO_OUT32(mmio, ATI_REG_CCE_WPTR, atis->ring_write);
}

void
ATIFlushIndirect(ATIScreenInfo *atis, Bool discard)
{
	ATICardInfo *atic = atis->atic;
	dmaBuf *buf = atis->indirectBuffer;

	if ((atis->indirectStart == buf->used) && !discard)
		return;

#if DEBUG_FIFO
	ErrorF("Dispatching %d DWORDS\n", (buf->used - atis->indirectStart) /
	    4);
#endif

#ifdef USE_DRI
	if (atis->using_dri) {
		buf->drmBuf->used = buf->used;
		ATIDRIDispatchIndirect(atis, discard);
		if (discard) {
			buf->drmBuf = ATIDRIGetBuffer(atis);
			buf->size = buf->drmBuf->total;
			buf->used = buf->drmBuf->used;
			buf->address = buf->drmBuf->address;
			atis->indirectStart = 0;
		} else {
			/* Start on a double word boundary */
			atis->indirectStart = buf->used = (buf->used + 7) & ~7;
		}
		return;
	}
#endif /* USE_DRI */

	if (atis->using_dma && !atic->is_radeon)
		R128DispatchIndirectDMA(atis);
	else if (atis->using_pseudo) {
		if (atic->is_radeon)
			RadeonDispatchIndirectPDMA(atis);
		else
			R128DispatchIndirectPDMA(atis);
	} else
		ATIDispatchIndirectMMIO(atis);

	buf->used = 0;
	atis->indirectStart = 0;
}

static Bool
ATIInitAGP(ScreenPtr pScreen, int size)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	AgpInfoPtr agp_info;
	int screennum = atis->screen->mynum;

	if (atic->is_radeon)
		return FALSE;

	if (!KdAgpGARTSupported()) 
		return FALSE; 

	if (!KdAcquireGART(screennum))
		return FALSE;

	atis->agp_key = KdAllocateGARTMemory(screennum, size, 0, NULL);
	if (atis->agp_key == -1) {
		ErrorF("Failed to allocate %dKB GART memory\n", size/1024);
		KdReleaseGART(screennum);
		return FALSE;
	}

	if (!KdBindGARTMemory(screennum, atis->agp_key, 0)) {
		ErrorF("Failed to bind GART memory\n");
		KdReleaseGART(screennum);
		return FALSE;
	}

	agp_info = KdGetAGPInfo(screennum);
	if (agp_info == NULL) {
		KdUnbindGARTMemory(screennum, atis->agp_key);
		KdReleaseGART(screennum);
		return FALSE;
	}

	atis->agp_addr = KdMapDevice(agp_info->base, agp_info->size);
	if (atis->agp_addr == NULL) {
		ErrorF("Failed to map GART memory\n");
		KdUnbindGARTMemory(screennum, atis->agp_key);
		KdReleaseGART(screennum);
		free(agp_info);
		return FALSE;
	}
	KdSetMappedMode(agp_info->base, agp_info->size,
	    KD_MAPPED_MODE_FRAMEBUFFER);

	atis->agp_size = size;
	free(agp_info);

	return TRUE;
}

static void
ATIFiniAGP(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	int screennum = atis->screen->mynum;

	KdUnbindGARTMemory(screennum, atis->agp_key);
	KdReleaseGART(screennum);
	atis->agp_addr = NULL;
	atis->agp_size = 0;
}

static Bool
ATIPseudoDMAInit(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	char *mmio = atic->reg_base;

	if (atic->is_r300)
		return FALSE;

	ATIUploadMicrocode(atis);
	ATIEngineReset(atis);

	if (atic->is_r200) {
		MMIO_OUT32(mmio, RADEON_REG_CP_CSQ_CNTL,
		    RADEON_CSQ_PRIPIO_INDDIS);
		atis->cce_pri_size = MMIO_IN32(mmio, RADEON_REG_CP_CSQ_CNTL) &
		    R200_CSQ_CNT_PRIMARY_MASK;
		MMIO_OUT32(mmio, RADEON_REG_ME_CNTL, RADEON_ME_MODE_FREE_RUN);
	} else if (atic->is_radeon) {
		MMIO_OUT32(mmio, RADEON_REG_CP_CSQ_CNTL,
		    RADEON_CSQ_PRIPIO_INDDIS);
		atis->cce_pri_size = MMIO_IN32(mmio, RADEON_REG_CP_CSQ_CNTL) &
		    RADEON_CSQ_CNT_PRIMARY_MASK;
		MMIO_OUT32(mmio, RADEON_REG_ME_CNTL, RADEON_ME_MODE_FREE_RUN);
	} else {
		MMIO_OUT32(mmio, R128_REG_PM4_BUFFER_CNTL, R128_PM4_192PIO |
		    R128_PM4_BUFFER_CNTL_NOUPDATE);
		atis->cce_pri_size = 192;
		MMIO_OUT32(mmio, R128_REG_PM4_MICRO_CNTL,
		    R128_PM4_MICRO_FREERUN);
	}

	return TRUE;
}

static Bool
ATIPseudoDMAFini(ScreenPtr pScreen)
{	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	char *mmio = atic->reg_base;

	if (atic->is_radeon) {
		MMIO_OUT32(mmio, RADEON_REG_ME_CNTL, 0);
		MMIO_OUT32(mmio, RADEON_REG_CP_CSQ_CNTL,
		    RADEON_CSQ_PRIDIS_INDDIS);
	} else {
		MMIO_OUT32(mmio, R128_REG_PM4_MICRO_CNTL, 0);
		MMIO_OUT32(mmio, R128_REG_PM4_BUFFER_CNTL,
		    R128_PM4_NONPM4 | R128_PM4_BUFFER_CNTL_NOUPDATE);
	}
	atis->cce_pri_size = 0;

	ATIEngineReset(atis);

	return TRUE;
}

static Bool
ATIDMAInit(ScreenPtr pScreen, Bool use_agp)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	char *mmio = atic->reg_base;
	int dma_offset;
	CARD32 tmp;

	/* XXX: Not for radeons.  Yet? */
	if (atic->is_radeon)
		return FALSE;

	if (use_agp) {
		if (1)
			return FALSE; /* XXX */
		/* Allocate a 1MB AGP space, but only use 128k + 128 for DMA.
		 * XXX: Should use the rest for things like scratch space.
		 */
		if (!ATIInitAGP(pScreen, 1024 * 1024))
			return FALSE;
		atis->ring_addr = atis->agp_addr;
		atis->ring_len = 128 * 1024;
		dma_offset = R128_AGP_OFFSET;
	} else {
		if (1)
			return FALSE; /* XXX */
		/* Allocate a 128K buffer, plus 32 DWORDS to give space for the
		 * R128 ASIC bug workaround.
		 */
		atis->dma_space = KdOffscreenAlloc(pScreen, 128 * 1024 + 128,
		    128, TRUE, NULL, NULL);
		if (atis->dma_space == NULL)
			return FALSE;
		atis->ring_addr = (CARD32 *)(atis->dma_space->offset +
		    pScreenPriv->screen->memory_base);
		atis->ring_len = 128 * 1024;
		dma_offset = atis->dma_space->offset;
	}

	ATIUploadMicrocode(atis);
	ATIEngineReset(atis);

	atis->ring_read = 0;
	atis->ring_write = 0;

	tmp = MMIO_IN32(mmio, ATI_REG_BUS_CNTL);
	MMIO_OUT32(mmio, ATI_REG_BUS_CNTL, tmp & ~ATI_BUS_MASTER_DIS);

	MMIO_OUT32(mmio, ATI_REG_CCE_RB_BASE, dma_offset);
	MMIO_OUT32(mmio, ATI_REG_CCE_WPTR, atis->ring_write);
	MMIO_OUT32(mmio, ATI_REG_CCE_RPTR, atis->ring_read);
	MMIO_OUT32(mmio, ATI_REG_CCE_RPTR_ADDR, 0 /* XXX? */);

	if (atic->is_r200) {
		MMIO_OUT32(mmio, RADEON_REG_CP_CSQ_CNTL,
		    RADEON_CSQ_PRIBM_INDBM);
		atis->cce_pri_size = MMIO_IN32(mmio, RADEON_REG_CP_CSQ_CNTL) &
		    R200_CSQ_CNT_PRIMARY_MASK;
		MMIO_OUT32(mmio, RADEON_REG_ME_CNTL, RADEON_ME_MODE_FREE_RUN);
	} else if (atic->is_radeon) {
		MMIO_OUT32(mmio, RADEON_REG_CP_CSQ_CNTL,
		    RADEON_CSQ_PRIBM_INDBM);
		atis->cce_pri_size = MMIO_IN32(mmio, RADEON_REG_CP_CSQ_CNTL) &
		    RADEON_CSQ_CNT_PRIMARY_MASK;
		MMIO_OUT32(mmio, RADEON_REG_ME_CNTL, RADEON_ME_MODE_FREE_RUN);
	} else {
		MMIO_OUT32(mmio, R128_REG_PM4_BUFFER_WM_CNTL,
		    ((R128_WATERMARK_L/4) << R128_WMA_SHIFT) |
		    ((R128_WATERMARK_M/4) << R128_WMB_SHIFT) |
		    ((R128_WATERMARK_N/4) << R128_WMC_SHIFT) |
		    ((R128_WATERMARK_K/64) << R128_WB_WM_SHIFT));
		/* The sample code reads from an undocumneted register
		 * (PM4_BUFFER_ADDR).  Perhaps it's a write posting thing?  Do
		 * a read in case that's it.
		 */
		MMIO_IN32(mmio, R128_REG_PM4_BUFFER_CNTL);
		if (use_agp) {
			/* XXX Magic num */
			MMIO_OUT32(mmio, R128_REG_PCI_GART_PAGE, 1);
			MMIO_OUT32(mmio, R128_REG_PM4_BUFFER_CNTL,
			    ATILog2(atis->ring_len) |
			    R128_PM4_192BM |
			    R128_PM4_BUFFER_CNTL_NOUPDATE);
		} else {
			MMIO_OUT32(mmio, R128_REG_PM4_BUFFER_CNTL,
			    ATILog2(atis->ring_len) |
			    R128_PM4_192BM |
			    R128_PM4_BUFFER_CNTL_NOUPDATE |
			    R128_PM4_IN_FRAME_BUFFER);
		}
		atis->cce_pri_size = 192;
		MMIO_IN32(mmio, R128_REG_PM4_BUFFER_CNTL);
		MMIO_OUT32(mmio, R128_REG_PM4_MICRO_CNTL,
		    R128_PM4_MICRO_FREERUN);
	}

	return TRUE;
}

static Bool
ATIDMAFini(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);
	ATICardInfo(pScreenPriv);
	char *mmio = atic->reg_base;

	if (atic->is_radeon) {
		MMIO_OUT32(mmio, RADEON_REG_ME_CNTL, 0);
		MMIO_OUT32(mmio, RADEON_REG_CP_CSQ_CNTL,
		    RADEON_CSQ_PRIDIS_INDDIS);
	} else {
		MMIO_OUT32(mmio, ATI_REG_CCE_WPTR,
		    atis->ring_write | R128_PM4_BUFFER_DL_DONE);
		MMIO_OUT32(mmio, R128_REG_PM4_MICRO_CNTL, 0);
		MMIO_OUT32(mmio, R128_REG_PM4_BUFFER_CNTL,
		    R128_PM4_NONPM4 | R128_PM4_BUFFER_CNTL_NOUPDATE);
	}
	atis->cce_pri_size = 0;

	ATIEngineReset(atis);

	if (atis->using_agp)
		ATIFiniAGP(pScreen);
	else
		KdOffscreenFree(pScreen, atis->dma_space);

	return TRUE;
}

void
ATIDMASetup(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATICardInfo(pScreenPriv);
	ATIScreenInfo(pScreenPriv);

#ifdef USE_DRI
	if (atis->using_dri)
		ATIDRIDMAStart(atis);
#endif /* USE_DRI */

	if (!atis->using_dri) {
		atis->using_agp = FALSE;
		if (atic->is_agp && ATIDMAInit(pScreen, TRUE)) {
			atis->using_agp = TRUE;
			atis->using_dma = TRUE;
		} else if (ATIDMAInit(pScreen, FALSE)) {
			atis->using_agp = FALSE;
			atis->using_dma = TRUE;
		} else if (ATIPseudoDMAInit(pScreen))
			atis->using_pseudo = TRUE;
		else
			atis->using_pio = TRUE;
	}

	atis->indirectBuffer = ATIGetDMABuffer(atis);
	if (atis->indirectBuffer == FALSE)
		FatalError("Failed to allocate DMA buffer.\n");

	if (atis->using_dri)
		ErrorF("Initialized %s DRI DMA\n",
		    atis->using_agp ? "AGP" : "PCI");
	else if (atis->using_dma && atis->using_agp)
		ErrorF("Initialized AGP DMA\n");
	else if (atis->using_dma)
		ErrorF("Initialized framebuffer pseudo-DMA\n");
	else if (atis->using_pseudo)
		ErrorF("Initialized pseudo-DMA\n");
	else if (atis->using_pio)
		ErrorF("Initialized PIO\n");
}

void
ATIDMATeardown(ScreenPtr pScreen)
{
	KdScreenPriv(pScreen);
	ATIScreenInfo(pScreenPriv);

	ATIWaitIdle(atis);

#ifdef USE_DRI
	if (atis->using_dri)
		ATIDRIDMAStop(atis);
#endif /* USE_DRI */

	if (atis->using_dma)
		ATIDMAFini(pScreen);

	if (atis->using_pseudo)
		ATIPseudoDMAFini(pScreen);

	if (atis->using_pio || atis->using_pseudo || atis->using_dma) {
		xfree(atis->indirectBuffer->address);
		xfree(atis->indirectBuffer);
	}
	atis->indirectBuffer = NULL;

	atis->using_pio = FALSE;
	atis->using_pseudo = FALSE;
	atis->using_dma = FALSE;
	atis->using_agp = FALSE;
}

