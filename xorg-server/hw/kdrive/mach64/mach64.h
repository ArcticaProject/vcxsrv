/*
 * Copyright © 2001 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _MACH64_H_
#define _MACH64_H_
#include <vesa.h>
#include "kxv.h"

/*
 * offset from ioport beginning 
 */

#define MACH64_REG_BASE(c)	    ((c)->attr.address[1])
#define MACH64_REG_SIZE(c)	    (4096)

#define MACH64_REG_OFF(c)	    (1024)
#define MACH64_MEDIA_REG_OFF(c)	    (0)

typedef volatile CARD8	VOL8;
typedef volatile CARD16	VOL16;
typedef volatile CARD32	VOL32;

typedef struct _Reg {
    VOL32	CRTC_H_TOTAL_DISP;	    /* 0x00 */
    VOL32	CRTC_H_SYNC_STRT_WID;	    /* 0x01 */
    VOL32	CRTC_V_TOTAL_DISP;	    /* 0x02 */
    VOL32	CRTC_V_SYNC_STRT_WID;	    /* 0x03 */
    VOL32	CRTC_VLINE_CRNT_VLINE;	    /* 0x04 */
    VOL32	CRTC_OFF_PITCH;		    /* 0x05 */
    VOL32	CRTC_INT_CNTL;		    /* 0x06 */
    VOL32	CRTC_GEN_CNTL;		    /* 0x07 */
    VOL32	DSP_CONFIG;		    /* 0x08 */
    VOL32	DSP_ON_OFF;		    /* 0x09 */
    VOL32	TIMER_CONFIG;		    /* 0x0a */
    VOL32	MEM_BUF_CNTL;		    /* 0x0b */
    VOL32	unused0;		    /* 0x0c */
    VOL32	MEM_ADDR_CONFIG;	    /* 0x0d */
    VOL32	CRT_TRAP;		    /* 0x0e */
    VOL32	I2C_CNTL_0;		    /* 0x0f */
    VOL32	OVR_CLR;		    /* 0x10 */
    VOL32	OVR_WID_LEFT_RIGHT;	    /* 0x11 */
    VOL32	OVR_WID_TOP_BOTTOM;	    /* 0x12 */
    VOL32	VGA_DSP_CONFIG;		    /* 0x13 */
    VOL32	VGA_DSP_ON_OFF;		    /* 0x14 */
    VOL32	DSP2_CONFIG;		    /* 0x15 */
    VOL32	DSP2_ON_OFF;		    /* 0x16 */
    VOL32	CRTC2_OFF_PITCH;	    /* 0x17 */
    VOL32	CUR_CLR0;		    /* 0x18 */
    VOL32	CUR_CLR1;		    /* 0x19 */
    VOL32	CUR_OFFSET;		    /* 0x1a */
    VOL32	CUR_HORZ_VERT_POSN;	    /* 0x1b */
    VOL32	CUR_HORZ_VERT_OFF;	    /* 0x1c */
    VOL32	TV_OUT_INDEX;		    /* 0x1d */
    VOL32	GP_IO;			    /* 0x1e */
    VOL32	HW_DEBUG;		    /* 0x1f */
    VOL32	SCRATCH_REG0;		    /* 0x20 */
    VOL32	SCRATCH_REG1;
    VOL32	SCRATCH_REG2;
    VOL32	SCRATCH_REG3;
    VOL32	CLOCK_CNTL;
    VOL32	CONFIG_STAT1;
    VOL32	CONFIG_STAT2;
    VOL32	TV_OUT_DATA;
    VOL32	BUS_CNTL;		    /* 0x28 */
    VOL32	LCD_INDEX;		    /* 0x29 */
    VOL32	LCD_DATA;		    /* 0x2a */
    VOL32	EXT_MEM_CNTL;
    VOL32	MEM_CNTL;
    VOL32	MEM_VGA_WP_SEL;
    VOL32	MEM_VGA_RP_SEL;
    VOL32	I2C_CNTL_1;
    VOL32	DAC_REGS;		    /* 0x30 */
    VOL32	DAC_CNTL;		    /* 0x31 */
    VOL32	unused_32;
    VOL32	unused_33;
    VOL32	GEN_TEST_CNTL;		    /* 0x34 */
    VOL32	CUSTOM_MACRO_CNTL;
    VOL32	unused36;
    VOL32	CONFIG_CNTL;
    VOL32	CONFIG_CHIP_ID;
    VOL32	CONFIG_STAT0;
    VOL32	CRC_SIG;
    VOL32	unused_3b;
    VOL32	unused_3c;
    VOL32	unused_3d;
    VOL32	unused_3e;
    VOL32	unused_3f;
    VOL32	DST_OFF_PITCH;		    /* 0x40 */
    VOL32	DST_X;
    VOL32	DST_Y;
    VOL32	DST_Y_X;
    VOL32	DST_WIDTH;
    VOL32	DST_HEIGHT;
    VOL32	DST_HEIGHT_WIDTH;
    VOL32	DST_X_WIDTH;
    VOL32	DST_BRES_LNTH;
    VOL32	DST_BRES_ERR;
    VOL32	DST_BRES_INC;
    VOL32	DST_BRES_DEC;
    VOL32	DST_CNTL;
    VOL32	DST_Y_X_ALIAS;
    VOL32	TRAIL_BRES_ERR;
    VOL32	TRAIL_BRES_INC;
    VOL32	TRAIL_BRES_DEC;
    VOL32	LEAD_BRES_LNTH;
    VOL32	Z_OFF_PITCH;
    VOL32	Z_CNTL;
    VOL32	ALPHA_TST_CNTL;
    VOL32	unused55;
    VOL32	SECONDARY_STW_EXP;
    VOL32	SECONDARY_S_X_INC;
    VOL32	SECONDARY_S_Y_INC;
    VOL32	SECONDARY_S_START;
    VOL32	SECONDARY_W_X_INC;
    VOL32	SECONDARY_W_Y_INC;
    VOL32	SECONDARY_W_START;
    VOL32	SECONDARY_T_X_INC;
    VOL32	SECONDARY_T_Y_INC;
    VOL32	SECONDARY_T_START;
    VOL32	SRC_OFF_PITCH;
    VOL32	SRC_X;
    VOL32	SRC_Y;
    VOL32	SRC_Y_X;
    VOL32	SRC_WIDTH1;
    VOL32	SRC_HEIGHT1;
    VOL32	SRC_HEIGHT1_WIDTH1;
    VOL32	SRC_X_START;
    VOL32	SRC_Y_START;
    VOL32	SRC_Y_X_START;
    VOL32	SRC_WIDTH2;
    VOL32	SRC_HEIGHT2;
    VOL32	SRC_HEIGHT2_WIDTH2;
    VOL32	SRC_CNTL;
    VOL32	unused6e;
    VOL32	unused6f;
    union {
	struct {
    VOL32	SCALE_OFF;		/* 0x70 */
    VOL32	unused71;
    VOL32	unused72;
    VOL32	unused73;
    VOL32	unused74;
    VOL32	unused75;
    VOL32	unused76;
    VOL32	SCALE_WIDTH;
    VOL32	SCALE_HEIGHT;
    VOL32	unused79;
    VOL32	unused7a;
    VOL32	SCALE_PITCH;
    VOL32	SCALE_X_INC;
    VOL32	SCALE_Y_INC;
    VOL32	SCALE_VACC;
    VOL32	SCALE_3D_CNTL;		/* 0x7f */
	} scaler;
	struct {
    VOL32	TEX_0_OFF;		/* 0x70 */
    VOL32	TEX_1_OFF;
    VOL32	TEX_2_OFF;
    VOL32	TEX_3_OFF;
    VOL32	TEX_4_OFF;
    VOL32	TEX_5_OFF;
    VOL32	TEX_6_OFF;
    VOL32	TEX_7_OFF;
    VOL32	TEX_8_OFF;
    VOL32	TEX_9_OFF;
    VOL32	TEX_10_OFF;
    VOL32	S_Y_INC;
    VOL32	RED_X_INC;
    VOL32	GREEN_X_INC;		/* 0x7d */
    VOL32	unused7e;
    VOL32	unused7f;
	} texture;
    } u;
    VOL32	HOST_DATA[16];		/* 0x80 */
    VOL32	HOST_CNTL;		/* 0x90 */
    VOL32	BM_HOSTDATA;		/* 0x91 */
    VOL32	BM_ADDR;		/* 0x92 */
    VOL32	BM_GUI_TABLE_CMD;	/* 0x93 */
    VOL32	unused94;		/* 0x94 */
    VOL32	unused95;		/* 0x95 */
    VOL32	unused96;		/* 0x96 */
    VOL32	FOG_TABLE_INDEX;	/* 0x97 */
    VOL32	FOG_TABLE_DATA[8];	/* 0x98 */
    VOL32	PAT_REG0;		/* 0xa0 */
    VOL32	PAT_REG1;
    VOL32	PAT_CNTL;
    VOL32	unused_0a3;
    VOL32	unused_0a4;
    VOL32	unused_0a5;
    VOL32	unused_0a6;
    VOL32	unused_0a7;
    VOL32	SC_LEFT;
    VOL32	SC_RIGHT;
    VOL32	SC_LEFT_RIGHT;
    VOL32	SC_TOP;
    VOL32	SC_BOTTOM;
    VOL32	SC_TOP_BOTTOM;
    VOL32	USR1_DST_OFF_PITCH;
    VOL32	USR2_DST_OFF_PITCH;
    VOL32	DP_BKGD_CLR;		/* 0xb0 */
    VOL32	DP_FRGD_CLR;
    VOL32	DP_WRITE_MSK;
    VOL32	unused_0b3;
    VOL32	DP_PIX_WIDTH;
    VOL32	DP_MIX;
    VOL32	DP_SRC;
    VOL32	DP_FRGD_CLR_MIX;
    VOL32	DP_FRGD_BKGD_CLR;
    VOL32	unused_0b9;
    VOL32	DST_X_Y;
    VOL32	DST_WIDTH_HEIGHT;
    VOL32	USR_DST_PITCH;
    VOL32	unused_0bd;
    VOL32	DP_SET_GUI_ENGINE2;
    VOL32	DP_SET_GUI_ENGINE;
    VOL32	CLR_CMP_CLR;		/* 0xc0 */
    VOL32	CLR_CMP_MSK;
    VOL32	CLR_CMP_CNTL;
    VOL32	unused_0c3;
    VOL32	FIFO_STAT;
    VOL32	unused_0c5;
    VOL32	unused_0c6;
    VOL32	unused_0c7;
    VOL32	unused_0c8;
    VOL32	unused_0c9;
    VOL32	unused_0ca;
    VOL32	unused_0cb;
    VOL32	GUI_TRAJ_CNTL;
    VOL32	unused_0cd;
    VOL32	GUI_STAT;
    VOL32	unused_0cf;
    VOL32	TEX_PALETTE_INDEX;
    VOL32	STW_EXP;
    VOL32	LOG_MAX_INC;
    VOL32	S_X_INC;
    VOL32	S_Y_INC_2_SCALE_PITCH;
    VOL32	S_START;
    VOL32	W_X_INC;
    VOL32	W_Y_INC;
    VOL32	W_START;
    VOL32	T_X_INC;
    VOL32	T_Y_INC_SECONDARY_SCALE_PITCH;
    VOL32	T_START;
    VOL32	TEX_SIZE_PITCH;
    VOL32	TEX_CNTL;
    VOL32	SECONDARY_TEX_OFFSET_SECONDARY_SCALE_OFF;
    VOL32	TEX_PALETTE;
    VOL32	SCALE_PITCH_BOTH;	/* 0xe0 */
    VOL32	SECONDARY_SCALE_OFF_ACC;
    VOL32	SCALE_OFF_ACC;
    VOL32	SCALE_DST_Y_X;
    VOL32	unused_0e4;
    VOL32	unused_0e5;
    VOL32	COMPOSITE_SHADOW_ID;
    VOL32	SECONDARY_SCALE_X_INC_SPECULAR_RED_X_INC;
    VOL32	SPECULAR_RED_Y_INC;
    VOL32	SPECULAR_RED_START_SECONDARY_SCALE_HACC;;
    VOL32	SPECULAR_GREEN_X_INC;
    VOL32	SPECULAR_GREEN_Y_INC;
    VOL32	SPECULAR_GREEN_START;
    VOL32	SPECULAR_BLUE_X_INC;
    VOL32	SPECULAR_BLUE_Y_INC;
    VOL32	SPECULAR_BLUE_START;
    VOL32	RED_X_INC_SCALE_X_INC;
    VOL32	RED_Y_INC;
    VOL32	RED_START_SCALE_HACC;
    VOL32	GREEN_X_INC_SCALE_Y_INC;
    VOL32	GREEN_Y_INC_SECONDARY_SCALE_Y_INC;
    VOL32	GREEN_START_SECONDARY_SCALE_VACC;
    VOL32	BLUE_X_INC;
    VOL32	BLUE_Y_INC;
    VOL32	BLUE_START;
    VOL32	Z_X_INC;
    VOL32	Z_Y_INC;
    VOL32	Z_START;
    VOL32	ALPHA_X_INC;
    VOL32	FOG_X_INC;
    VOL32	ALPHA_Y_INC;
    VOL32	FOG_Y_INC;
    VOL32	ALPHA_START;
    VOL32	FOG_START;
    VOL32	unused_0ff;
} Reg;					/* 0x100 */

#define DST_X_DIR		(1 << 0)
#define DST_Y_DIR		(1 << 1)
#define DST_Y_MAJOR		(1 << 2)
#define DST_X_TILE		(1 << 3)
#define DST_Y_TILE		(1 << 4)
#define DST_LAST_PEL		(1 << 5)
#define DST_POLYGON_EN		(1 << 6)
#define DST_24_ROT_EN		(1 << 7)
#define DST_24_ROT(n)		((n) << 8)
#define DST_BRES_ZERO		(1 << 11)
#define DST_POLYGON_RTEDGE_DIS	(1 << 12)
#define TRAIL_X_DIR		(1 << 13)
#define TRAP_FILL_DIR		(1 << 14)
#define TRAIL_BRES_SIGN		(1 << 15)
#define SRC_PATT_EN		(1 << 16)
#define SRC_PATT_ROT_EN		(1 << 17)
#define SRC_LINEAR_EN		(1 << 18)
#define SRC_BYTE_ALIGN		(1 << 19)
#define SRC_LINE_X_DIR		(1 << 20)
#define SRC_8x8x8_BRUSH		(1 << 21)
#define FAST_FILL_EN		(1 << 22)
#define SRC_TRACK_DST		(1 << 23)
#define PAT_MONO_EN		(1 << 24)
#define PAT_CLR_4x2_EN		(1 << 25)
#define PAT_CLR_8x1_EN		(1 << 26)
#define HOST_BYTE_ALIGN		(1 << 28)
#define HOST_BIG_ENDIAN_EN	(1 << 29)

/* BUS_CNTL bits */
#define BUS_DBL_RESYNC		(1 << 0)
#define BUS_MSTR_RESET		(1 << 1)
#define BUS_FLUSH_BUF		(1 << 2)
#define BUS_STOP_REQ_DIS	(1 << 3)
#define BUS_APER_REG_DIS	(1 << 4)
#define BUS_EXTRA_PIPE_DIS	(1 << 5)
#define BUS_MASTER_DIS		(1 << 6)
#define ROM_WRT_EN		(1 << 7)
#define CHIP_HIDDEN_REV		(3 << 8)
#define BUS_PCI_READ_RETRY_EN	(1 << 13)
#define BUS_PCI_WRT_RETRY_EN	(1 << 15)
#define BUS_RETRY_WS		(0xf << 16)
#define BUS_MSTR_RD_MULT	(1 << 20)
#define BUS_MSTR_RD_LINE	(1 << 21)
#define BUS_SUSPEND		(1 << 22)
#define LAT16X			(1 << 23)
#define BUS_RD_DISCARD_EN	(1 << 24)
#define BUS_RD_ABORT_EN		(1 << 25)
#define BUS_MSTR_WS		(1 << 26)
#define BUS_EXT_REG_EN		(1 << 27)
#define BUS_MSTR_DISCONNECT_EN	(1 << 28)
#define BUS_WRT_BURST		(1 << 29)
#define BUS_READ_BURST		(1 << 30)
#define BUS_RDY_READ_DLY	(1 << 31)

#define SCALE_PIX_EXPAND    (1 << 0)
#define SCALE_Y2R_TEMP	    (1 << 1)
#define SCALE_HORZ_MODE	    (1 << 2)
#define SCALE_VERT_MODE	    (1 << 3)
#define SCALE_SIGNED_UV	    (1 << 4)
#define SCALE_GAMMA_SEL	    (3 << 5)
#define SCALE_GAMMA_BRIGHT  (0 << 5)
#define SCALE_GAMMA_22	    (1 << 5)
#define SCALE_GAMMA_18	    (2 << 5)
#define SCALE_GAMMA_14	    (3 << 5)
#define SCALE_DISP_SEL	    (1 << 7)
#define SCALE_BANDWIDTH	    (1 << 26)
#define SCALE_DIS_LIMIT	    (1 << 27)
#define SCALE_CLK_FORCE_ON  (1 << 29)
#define SCALE_OVERLAY_EN    (1 << 30)
#define SCALE_EN	    (1 << 31)

#define VIDEO_IN_VYUY422    (0xb)
#define VIDEO_IN_YVYU422    (0xc)
#define SCALER_IN_15bpp	    (0x3 << 16)
#define SCALER_IN_16bpp	    (0x4 << 16)
#define SCALER_IN_32bpp	    (0x6 << 16)
#define SCALER_IN_YUV_9	    (0x9 << 16)
#define SCALER_IN_YUV_12    (0xa << 16)
#define SCALER_IN_VYUY422   (0xb << 16)
#define SCALER_IN_YVYU422   (0xc << 16)

#define CAP_INPUT_MODE	    (1 << 0)
#define CAP_BUF_MODE	    (1 << 2)
#define CAP_START_BUF	    (1 << 3)
#define CAP_BUF_TYPE_FIELD  (0 << 4)
#define CAP_BUF_TYPE_ALTERNATING  (1 << 4)
#define CAP_BUF_TYPE_FRAME  (2 << 4)

#define OVL_BUF_MODE	    (1 << 28)
#define OVL_BUF_NEXT	    (1 << 29)

#define CAP_TRIGGER	    (3 << 0)
#define OVL_CUR_BUF	    (1 << 5)
#define OVL_BUF_STATUS	    (1 << 6)
#define CAP_BUF_STATUS	    (1 << 7)
#define CAPTURE_EN	    (1 << 31)

typedef struct _MediaReg {
    VOL32	OVERLAY_Y_X_START;	/* 0x100 */
    VOL32	OVERLAY_Y_X_END;
    VOL32	OVERLAY_VIDEO_KEY_CLR;
    VOL32	OVERLAY_VIDEO_KEY_MSK;
    VOL32	OVERLAY_GRAPHICS_KEY_CLR;
    VOL32	OVERLAY_GRAPHICS_KEY_MSK;
    VOL32	OVERLAY_KEY_CNTL;
    VOL32	unused_107;
    VOL32	OVERLAY_SCALE_INC;
    VOL32	OVERLAY_SCALE_CNTL;
    VOL32	SCALER_HEIGHT_WIDTH;
    VOL32	SCALER_TEST;
    VOL32	unused_10c;
    VOL32	SCALER_BUF0_OFFSET;
    VOL32	SCALER_BUF1_OFFSET;
    VOL32	SCALER_BUF_PITCH;
    VOL32	CAPTURE_START_END;	/* 0x110 */
    VOL32	CAPTURE_X_WIDTH;
    VOL32	VIDEO_FORMAT;
    VOL32	VBI_START_END;
    VOL32	CAPTURE_CONFIG;
    VOL32	TRIG_CNTL;
    VOL32	OVERLAY_EXCLUSIVE_HORZ;
    VOL32	OVERLAY_EXCLUSIVE_VERT;
    VOL32	VBI_WIDTH;
    VOL32	CAPTURE_DEBUG;
    VOL32	VIDEO_SYNC_TEST;
    VOL32	unused_11b;
    VOL32	SNAPSHOT_VH_COUNTS;
    VOL32	SNAPSHOT_F_COUNT;
    VOL32	N_VIF_COUNT;
    VOL32	SNAPSHOT_VIF_COUNT;
    VOL32	CAPTURE_BUF0_OFFSET;	/* 0x120 */
    VOL32	CAPTURE_BUF1_OFFSET;
    VOL32	ONESHOT_BUF_OFFSET;
    VOL32	unused_123;
    VOL32	unused_124;
    VOL32	unused_125;
    VOL32	unused_126;
    VOL32	unused_127;
    VOL32	unused_128;
    VOL32	unused_129;
    VOL32	unused_12a;
    VOL32	unused_12b;
    VOL32	SNAPSHOT2_VH_COUNTS;
    VOL32	SNAPSHOT2_F_COUNT;
    VOL32	N_VIF2_COUNT;
    VOL32	SNAPSHOT2_VIF_COUNT;
    VOL32	MPP_CONFIG;		/* 0x130 */
    VOL32	MPP_STROBE_SEQ;
    VOL32	MPP_ADDR;
    VOL32	MPP_DATA;
    VOL32	unused_134;
    VOL32	unused_135;
    VOL32	unused_136;
    VOL32	unused_137;
    VOL32	unused_138;
    VOL32	unused_139;
    VOL32	unused_13a;
    VOL32	unused_13b;
    VOL32	unused_13c;
    VOL32	unused_13d;
    VOL32	unused_13e;
    VOL32	unused_13f;
    VOL32	TVO_CNTL;		/* 0x140 */
    VOL32	unused_141[15];
    VOL32	unused_150;		/* 0x150 */
    VOL32	CRT_HORZ_VERT_LOAD;	/* 0x151 */
    VOL32	AGP_BASE;		/* 0x152 */
    VOL32	AGP_CNTL;		/* 0x153 */
    VOL32	SCALER_COLOUR_CNTL;	/* 0x154 */
    VOL32	SCALER_H_COEFF0;	/* 0x155 */
    VOL32	SCALER_H_COEFF1;	/* 0x156 */
    VOL32	SCALER_H_COEFF2;	/* 0x157 */
    VOL32	SCALER_H_COEFF3;	/* 0x158 */
    VOL32	SCALER_H_COEFF4;	/* 0x159 */
    VOL32	unused_15a;
    VOL32	unused_15b;
    VOL32	GUI_CMDFIFO_DEBUG;
    VOL32	GUI_CMDFIFO_DATA;
    VOL32	GUI_CNTL;
    VOL32	unused_15f;
    VOL32	BM_FRAME_BUF_OFFSET;	/* 0x160 */
    VOL32	BM_SYSTEM_MEM_ADDR;
    VOL32	BM_COMMAND;
    VOL32	BM_STATUS;
    VOL32	unused_164[10];
    VOL32	BM_GUI_TABLE;
    VOL32	BM_SYSTEM_TABLE;
    VOL32	unused_170[5];		/* 0x170 */
    VOL32	SCALER_BUF0_OFFSET_U;
    VOL32	SCALER_BUF0_OFFSET_V;
    VOL32	SCALER_BUF1_OFFSET_U;
    VOL32	SCALER_BUF1_OFFSET_V;
    VOL32	unused_179[7];
    VOL32	unused_180[16];		/* 0x180 */
    VOL32	setup_engine[0x40];	/* 0x190 */
    VOL32	dvd_subpicture[0x30];	/* 0x1d0 */
} MediaReg;

#define MACH64_XY(x,y)	    (((x) & 0x7fff) | (((y) & 0x7fff) << 16))
#define MACH64_YX(x,y)	    (((y) & 0x7fff) | (((x) & 0x7fff) << 16))

typedef struct _mach64Save {
    CARD32	LCD_GEN_CTRL;
} Mach64Save;

typedef struct _mach64CardInfo {
    VesaCardPrivRec	vesa;
    CARD8		*reg_base;
    Reg			*reg;
    MediaReg		*media_reg;
    Mach64Save		save;
    Bool		lcdEnabled;
} Mach64CardInfo;
    
#define getMach64CardInfo(kd)	((Mach64CardInfo *) ((kd)->card->driver))
#define mach64CardInfo(kd)	Mach64CardInfo	*mach64c = getMach64CardInfo(kd)

typedef struct _mach64Cursor {
    int		width, height;
    int		xhot, yhot;
    Bool	has_cursor;
    CursorPtr	pCursor;
    Pixel	source, mask;
} Mach64Cursor;

#define MACH64_CURSOR_WIDTH	64
#define MACH64_CURSOR_HEIGHT	64

/*
 * Xv information, optional
 */
typedef struct _mach64PortPriv {
    CARD32      YBuf0Offset;

    CARD32      YBuf1Offset;

    CARD8	currentBuf;

    int		brightness;
    int		saturation;

    RegionRec   clip;
    CARD32      colorKey;

    Bool	videoOn;
    Time        offTime;
    Time        freeTime;
    CARD32	size;
    CARD32	offset;
    KdOffscreenArea *off_screen;
} Mach64PortPrivRec, *Mach64PortPrivPtr;

Bool mach64InitVideo(ScreenPtr pScreen);
void mach64FiniVideo(ScreenPtr pScreen);

/*
 * These values are per-format, which is essentially per-depth/per-bpp.
 * Because bpp is fixed for the screen by depth, they're computed
 * per-screen per-depth.
 */

typedef struct _mach64ScreenInfo {
    VesaScreenPrivRec		vesa;
    Mach64Cursor		cursor;
    CARD32			colorKey;
    KdVideoAdaptorPtr		pAdaptor;
    KaaScreenInfoRec		kaa;
} Mach64ScreenInfo;

#define getMach64ScreenInfo(kd) ((Mach64ScreenInfo *) ((kd)->screen->driver))
#define mach64ScreenInfo(kd)    Mach64ScreenInfo *mach64s = getMach64ScreenInfo(kd)

CARD32
mach64ReadLCD (Reg *reg, int id);

void
mach64WriteLCD (Reg *reg, int id, CARD32 data);
    
void
mach64Preserve (KdCardInfo *card);

Bool
mach64MapReg (KdCardInfo *card, Mach64CardInfo *mach64c);

void
mach64UnmapReg (KdCardInfo *card, Mach64CardInfo *mach64c);

void
mach64SetMMIO (KdCardInfo *card, Mach64CardInfo *mach64c);

void
mach64ResetMMIO (KdCardInfo *card, Mach64CardInfo *mach64c);

Bool
mach64Enable (ScreenPtr pScreen);

void
mach64Disable (ScreenPtr pScreen);

Bool
mach64DPMS (ScreenPtr pScreen, int mode);

void
mach64WaitAvail(Reg *reg, int n);

void
mach64WaitIdle (Reg *reg);
    
Bool
mach64DrawSetup (ScreenPtr pScreen);

Bool
mach64DrawInit (ScreenPtr pScreen);

void
mach64DrawReinit (ScreenPtr pScreen);

void
mach64DrawEnable (ScreenPtr pScreen);

void
mach64DrawDisable (ScreenPtr pScreen);

void
mach64DrawFini (ScreenPtr pScreen);

CARD8
mach64ReadIndex (Mach64CardInfo *mach64c, CARD16 port, CARD8 index);

void
mach64WriteIndex (Mach64CardInfo *mach64c, CARD16 port, CARD8 index, CARD8 value);

Bool
mach64CursorInit (ScreenPtr pScreen);

void
mach64CursorEnable (ScreenPtr pScreen);

void
mach64CursorDisable (ScreenPtr pScreen);

void
mach64CursorFini (ScreenPtr pScreen);

void
mach64RecolorCursor (ScreenPtr pScreen, int ndef, xColorItem *pdef);

extern KdCardFuncs  mach64Funcs;

#endif /* _MACH64_H_ */
