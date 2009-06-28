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

/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keith@tungstengraphics.com>
 *   Pontus Lidman <pontus.lidman@nokia.com>
 *
 *   based on the i740 driver by
 *        Kevin E. Martin <kevin@precisioninsight.com> 
 *   
 *
 */

/* I/O register offsets 
 */
#define SRX 0x3C4		/* p208 */
#define GRX 0x3CE		/* p213 */
#define ARX 0x3C0		/* p224 */

/* VGA Color Palette Registers */
#define DACMASK  0x3C6		/* p232 */
#define DACSTATE 0x3C7		/* p232 */
#define DACRX    0x3C7		/* p233 */
#define DACWX    0x3C8		/* p233 */
#define DACDATA  0x3C9		/* p233 */

/* CRT Controller Registers (CRX) */
#define START_ADDR_HI        0x0C /* p246 */
#define START_ADDR_LO        0x0D /* p247 */
#define VERT_SYNC_END        0x11 /* p249 */
#define EXT_VERT_TOTAL       0x30 /* p257 */
#define EXT_VERT_DISPLAY     0x31 /* p258 */
#define EXT_VERT_SYNC_START  0x32 /* p259 */
#define EXT_VERT_BLANK_START 0x33 /* p260 */
#define EXT_HORIZ_TOTAL      0x35 /* p261 */
#define EXT_HORIZ_BLANK      0x39 /* p261 */
#define EXT_START_ADDR       0x40 /* p262 */
#define EXT_START_ADDR_ENABLE    0x80 
#define EXT_OFFSET           0x41 /* p263 */
#define EXT_START_ADDR_HI    0x42 /* p263 */
#define INTERLACE_CNTL       0x70 /* p264 */
#define INTERLACE_ENABLE         0x80 
#define INTERLACE_DISABLE        0x00 

/* Miscellaneous Output Register 
 */
#define MSR_R          0x3CC	/* p207 */
#define MSR_W          0x3C2	/* p207 */
#define IO_ADDR_SELECT     0x01

#define MDA_BASE       0x3B0	/* p207 */
#define CGA_BASE       0x3D0	/* p207 */

/* CR80 - IO Control, p264
 */
#define IO_CTNL            0x80
#define EXTENDED_ATTR_CNTL     0x02
#define EXTENDED_CRTC_CNTL     0x01

/* GR10 - Address mapping, p221
 */
#define ADDRESS_MAPPING    0x10
#define PAGE_TO_LOCAL_MEM_ENABLE 0x10
#define GTT_MEM_MAP_ENABLE     0x08
#define PACKED_MODE_ENABLE     0x04
#define LINEAR_MODE_ENABLE     0x02
#define PAGE_MAPPING_ENABLE    0x01

/* Blitter control, p378
 */
#define BITBLT_CNTL        0x7000c
#define COLEXP_MODE            0x30
#define COLEXP_8BPP            0x00
#define COLEXP_16BPP           0x10
#define COLEXP_24BPP           0x20
#define COLEXP_RESERVED        0x30
#define BITBLT_STATUS          0x01

/* p375. 
 */
#define DISPLAY_CNTL       0x70008
#define VGA_WRAP_MODE          0x02
#define VGA_WRAP_AT_256KB      0x00
#define VGA_NO_WRAP            0x02
#define GUI_MODE               0x01
#define STANDARD_VGA_MODE      0x00
#define HIRES_MODE             0x01

/* p375
 */
#define PIXPIPE_CONFIG_0   0x70009
#define DAC_8_BIT              0x80
#define DAC_6_BIT              0x00
#define HW_CURSOR_ENABLE       0x10
#define EXTENDED_PALETTE       0x01

/* p375
 */
#define PIXPIPE_CONFIG_1   0x7000a
#define DISPLAY_COLOR_MODE     0x0F
#define DISPLAY_VGA_MODE       0x00
#define DISPLAY_8BPP_MODE      0x02
#define DISPLAY_15BPP_MODE     0x04
#define DISPLAY_16BPP_MODE     0x05
#define DISPLAY_24BPP_MODE     0x06
#define DISPLAY_32BPP_MODE     0x07

/* p375
 */
#define PIXPIPE_CONFIG_2   0x7000b
#define DISPLAY_GAMMA_ENABLE   0x08
#define DISPLAY_GAMMA_DISABLE  0x00
#define OVERLAY_GAMMA_ENABLE   0x04
#define OVERLAY_GAMMA_DISABLE  0x00


/* p380
 */
#define DISPLAY_BASE       0x70020
#define DISPLAY_BASE_MASK  0x03fffffc


/* Cursor control registers, pp383-384
 */
#define CURSOR_CONTROL     0x70080
#define CURSOR_ORIGIN_SCREEN   0x00
#define CURSOR_ORIGIN_DISPLAY  0x10
#define CURSOR_MODE            0x07
#define CURSOR_MODE_DISABLE    0x00
#define CURSOR_MODE_32_4C_AX   0x01
#define CURSOR_MODE_64_3C      0x04
#define CURSOR_MODE_64_4C_AX   0x05
#define CURSOR_MODE_64_4C      0x06
#define CURSOR_MODE_RESERVED   0x07
#define CURSOR_BASEADDR    0x70084
#define CURSOR_BASEADDR_MASK 0x1FFFFF00
#define CURSOR_X_LO        0x70088
#define CURSOR_X_HI        0x70089
#define CURSOR_X_POS           0x00
#define CURSOR_X_NEG           0x80
#define CURSOR_Y_LO        0x7008A
#define CURSOR_Y_HI        0x7008B
#define CURSOR_Y_POS           0x00
#define CURSOR_Y_NEG           0x80



/* Similar registers exist in Device 0 on the i810 (pp55-65), but I'm
 * not sure they refer to local (graphics) memory.
 *
 * These details are for the local memory control registers,
 * (pp301-310).  The test machines are not equiped with local memory,
 * so nothing is tested.  Only a single row seems to be supported.
 */
#define DRAM_ROW_TYPE      0x3000
#define DRAM_ROW_0             0x01
#define DRAM_ROW_0_SDRAM       0x01
#define DRAM_ROW_0_EMPTY       0x00
#define DRAM_ROW_CNTL_LO   0x3001
#define DRAM_PAGE_MODE_CTRL    0x10
#define DRAM_RAS_TO_CAS_OVRIDE 0x08
#define DRAM_CAS_LATENCY       0x04
#define DRAM_RAS_TIMING        0x02
#define DRAM_RAS_PRECHARGE     0x01
#define DRAM_ROW_CNTL_HI   0x3002
#define DRAM_REFRESH_RATE      0x18
#define DRAM_REFRESH_DISABLE   0x00
#define DRAM_REFRESH_60HZ      0x08
#define DRAM_REFRESH_FAST_TEST 0x10
#define DRAM_REFRESH_RESERVED  0x18
#define DRAM_SMS               0x07
#define DRAM_SMS_NORMAL        0x00
#define DRAM_SMS_NOP_ENABLE    0x01
#define DRAM_SMS_ABPCE         0x02
#define DRAM_SMS_MRCE          0x03
#define DRAM_SMS_CBRCE         0x04

/* p307
 */
#define DPMS_SYNC_SELECT   0x5002
#define VSYNC_CNTL             0x08
#define VSYNC_ON               0x00
#define VSYNC_OFF              0x08
#define HSYNC_CNTL             0x02
#define HSYNC_ON               0x00
#define HSYNC_OFF              0x02



/* p317, 319
 */
#define VCLK2_VCO_M        0x6008 /* treat as 16 bit? (includes msbs) */
#define VCLK2_VCO_N        0x600a
#define VCLK2_VCO_DIV_SEL  0x6012
#define POST_DIV_SELECT        0x70
#define POST_DIV_1             0x00
#define POST_DIV_2             0x10
#define POST_DIV_4             0x20
#define POST_DIV_8             0x30
#define POST_DIV_16            0x40
#define POST_DIV_32            0x50
#define VCO_LOOP_DIV_BY_4M     0x00
#define VCO_LOOP_DIV_BY_16M    0x04


/* Instruction Parser Mode Register 
 *    - p281
 *    - 2 new bits.
 */
#define INST_PM                  0x20c0	
#define AGP_SYNC_PACKET_FLUSH_ENABLE 0x20 /* reserved */
#define SYNC_PACKET_FLUSH_ENABLE     0x10
#define TWO_D_INST_DISABLE           0x08
#define THREE_D_INST_DISABLE         0x04
#define STATE_VAR_UPDATE_DISABLE     0x02
#define PAL_STIP_DISABLE             0x01

#define INST_DONE                0x2090
#define INST_PS                  0x20c4

#define MEMMODE                  0x20dc


/* Instruction parser error register.  p279
 */
#define IPEIR                  0x2088
#define IPEHR                  0x208C


/* General error reporting regs, p296
 */
#define EIR               0x20B0
#define EMR               0x20B4
#define ESR               0x20B8
#define IP_ERR                    0x0001
#define ERROR_RESERVED            0xffc6


/* Interrupt Control Registers 
 *   - new bits for i810
 *   - new register hwstam (mask)
 */
#define HWSTAM               0x2098 /* p290 */
#define IER                  0x20a0 /* p291 */
#define IIR                  0x20a4 /* p292 */
#define IMR                  0x20a8 /* p293 */
#define ISR                  0x20ac /* p294 */
#define HW_ERROR                 0x8000
#define SYNC_STATUS_TOGGLE       0x1000
#define DPY_0_FLIP_PENDING       0x0800
#define DPY_1_FLIP_PENDING       0x0400	/* not implemented on i810 */
#define OVL_0_FLIP_PENDING       0x0200
#define OVL_1_FLIP_PENDING       0x0100	/* not implemented on i810 */
#define DPY_0_VBLANK             0x0080
#define DPY_0_EVENT              0x0040
#define DPY_1_VBLANK             0x0020	/* not implemented on i810 */
#define DPY_1_EVENT              0x0010	/* not implemented on i810 */
#define HOST_PORT_EVENT          0x0008	/*  */
#define CAPTURE_EVENT            0x0004	/*  */
#define USER_DEFINED             0x0002
#define BREAKPOINT               0x0001


#define INTR_RESERVED            (0x6000 | 		\
				  DPY_1_FLIP_PENDING |	\
				  OVL_1_FLIP_PENDING |	\
				  DPY_1_VBLANK |	\
				  DPY_1_EVENT |		\
				  HOST_PORT_EVENT |	\
				  CAPTURE_EVENT )

/* FIFO Watermark and Burst Length Control Register 
 *
 * - different offset and contents on i810 (p299) (fewer bits per field)
 * - some overlay fields added
 * - what does it all mean?
 */
#define FWATER_BLC       0x20d8
#define MM_BURST_LENGTH     0x00700000
#define MM_FIFO_WATERMARK   0x0001F000
#define LM_BURST_LENGTH     0x00000700
#define LM_FIFO_WATERMARK   0x0000001F


/* Fence/Tiling ranges [0..7]
 */
#define FENCE            0x2000
#define FENCE_NR         8

#define FENCE_START_MASK    0x03F80000
#define FENCE_X_MAJOR       0x00000000
#define FENCE_Y_MAJOR       0x00001000
#define FENCE_SIZE_MASK     0x00000700
#define FENCE_SIZE_512K     0x00000000
#define FENCE_SIZE_1M       0x00000100
#define FENCE_SIZE_2M       0x00000200
#define FENCE_SIZE_4M       0x00000300
#define FENCE_SIZE_8M       0x00000400
#define FENCE_SIZE_16M      0x00000500
#define FENCE_SIZE_32M      0x00000600
#define FENCE_PITCH_MASK    0x00000070
#define FENCE_PITCH_1       0x00000000
#define FENCE_PITCH_2       0x00000010
#define FENCE_PITCH_4       0x00000020
#define FENCE_PITCH_8       0x00000030
#define FENCE_PITCH_16      0x00000040
#define FENCE_PITCH_32      0x00000050
#define FENCE_VALID         0x00000001


/* Registers to control page table, p274
 */
#define PGETBL_CTL       0x2020
#define PGETBL_ADDR_MASK    0xFFFFF000
#define PGETBL_ENABLE_MASK  0x00000001
#define PGETBL_ENABLED      0x00000001

/* Register containing pge table error results, p276
 */
#define PGE_ERR          0x2024
#define PGE_ERR_ADDR_MASK   0xFFFFF000
#define PGE_ERR_ID_MASK     0x00000038
#define PGE_ERR_CAPTURE     0x00000000
#define PGE_ERR_OVERLAY     0x00000008
#define PGE_ERR_DISPLAY     0x00000010
#define PGE_ERR_HOST        0x00000018
#define PGE_ERR_RENDER      0x00000020
#define PGE_ERR_BLITTER     0x00000028
#define PGE_ERR_MAPPING     0x00000030
#define PGE_ERR_CMD_PARSER  0x00000038
#define PGE_ERR_TYPE_MASK   0x00000007
#define PGE_ERR_INV_TABLE   0x00000000
#define PGE_ERR_INV_PTE     0x00000001
#define PGE_ERR_MIXED_TYPES 0x00000002
#define PGE_ERR_PAGE_MISS   0x00000003
#define PGE_ERR_ILLEGAL_TRX 0x00000004
#define PGE_ERR_LOCAL_MEM   0x00000005
#define PGE_ERR_TILED       0x00000006



/* Page table entries loaded via mmio region, p323
 */
#define PTE_BASE         0x10000
#define PTE_ADDR_MASK       0x3FFFF000
#define PTE_TYPE_MASK       0x00000006
#define PTE_LOCAL           0x00000002
#define PTE_MAIN_UNCACHED   0x00000000
#define PTE_MAIN_CACHED     0x00000006
#define PTE_VALID_MASK      0x00000001
#define PTE_VALID           0x00000001


/* Ring buffer registers, p277, overview p19
 */
#define LP_RING     0x2030
#define HP_RING     0x2040

#define RING_TAIL      0x00
#define TAIL_ADDR           0x000FFFF8

#define RING_HEAD      0x04
#define HEAD_WRAP_COUNT     0xFFE00000
#define HEAD_WRAP_ONE       0x00200000
#define HEAD_ADDR           0x001FFFFC

#define RING_START     0x08
#define START_ADDR          0x00FFFFF8

#define RING_LEN       0x0C
#define RING_NR_PAGES       0x000FF000 
#define RING_REPORT_MASK    0x00000006
#define RING_REPORT_64K     0x00000002
#define RING_REPORT_128K    0x00000004
#define RING_NO_REPORT      0x00000000
#define RING_VALID_MASK     0x00000001
#define RING_VALID          0x00000001
#define RING_INVALID        0x00000000



/* BitBlt Instructions
 *
 * There are many more masks & ranges yet to add.
 */
#define BR00_BITBLT_CLIENT   0x40000000
#define BR00_OP_COLOR_BLT    0x10000000
#define BR00_OP_SRC_COPY_BLT 0x10C00000
#define BR00_OP_FULL_BLT     0x11400000
#define BR00_OP_MONO_SRC_BLT 0x11800000
#define BR00_OP_MONO_SRC_COPY_BLT 0x11000000
#define BR00_OP_MONO_PAT_BLT 0x11C00000
#define BR00_OP_MONO_SRC_COPY_IMMEDIATE_BLT (0x61 << 22)
#define BR00_OP_TEXT_IMMEDIATE_BLT 0xc000000


#define BR00_TPCY_DISABLE    0x00000000
#define BR00_TPCY_ENABLE     0x00000010

#define BR00_TPCY_ROP        0x00000000
#define BR00_TPCY_NO_ROP     0x00000020
#define BR00_TPCY_EQ         0x00000000
#define BR00_TPCY_NOT_EQ     0x00000040

#define BR00_PAT_MSB_FIRST   0x00000000	/* ? */

#define BR00_PAT_VERT_ALIGN  0x000000e0

#define BR00_LENGTH          0x0000000F

#define BR09_DEST_ADDR       0x03FFFFFF

#define BR11_SOURCE_PITCH    0x00003FFF

#define BR12_SOURCE_ADDR     0x03FFFFFF

#define BR13_SOLID_PATTERN   0x80000000
#define BR13_RIGHT_TO_LEFT   0x40000000
#define BR13_LEFT_TO_RIGHT   0x00000000
#define BR13_MONO_TRANSPCY   0x20000000
#define BR13_USE_DYN_DEPTH   0x04000000
#define BR13_DYN_8BPP        0x00000000
#define BR13_DYN_16BPP       0x01000000
#define BR13_DYN_24BPP       0x02000000
#define BR13_ROP_MASK        0x00FF0000
#define BR13_DEST_PITCH      0x0000FFFF
#define BR13_PITCH_SIGN_BIT  0x00008000

#define BR14_DEST_HEIGHT     0xFFFF0000
#define BR14_DEST_WIDTH      0x0000FFFF

#define BR15_PATTERN_ADDR    0x03FFFFFF

#define BR16_SOLID_PAT_COLOR 0x00FFFFFF
#define BR16_BACKGND_PAT_CLR 0x00FFFFFF

#define BR17_FGND_PAT_CLR    0x00FFFFFF

#define BR18_SRC_BGND_CLR    0x00FFFFFF
#define BR19_SRC_FGND_CLR    0x00FFFFFF


/* Instruction parser instructions
 */

#define INST_PARSER_CLIENT   0x00000000
#define INST_OP_FLUSH        0x02000000
#define INST_FLUSH_MAP_CACHE 0x00000001

#define INST_DEST_BUFFER_INFO 0x06800000

#define INST_FRONT_BUFFER_INFO 0x06000000
#define FRONT_INFO_ASYNC_FLIP 1<<6
#define FRONT_INFO_PITCH_B    8

#define GFX_OP_USER_INTERRUPT ((0<<29)|(2<<23))


/* Registers in the i810 host-pci bridge pci config space which affect
 * the i810 graphics operations.  
 */
#define SMRAM_MISCC         0x70
#define GMS                    0x000000c0
#define GMS_DISABLE            0x00000000
#define GMS_ENABLE_BARE        0x00000040
#define GMS_ENABLE_512K        0x00000080
#define GMS_ENABLE_1M          0x000000c0
#define USMM                   0x00000030 
#define USMM_DISABLE           0x00000000
#define USMM_TSEG_ZERO         0x00000010
#define USMM_TSEG_512K         0x00000020
#define USMM_TSEG_1M           0x00000030  
#define GFX_MEM_WIN_SIZE       0x00010000
#define GFX_MEM_WIN_32M        0x00010000
#define GFX_MEM_WIN_64M        0x00000000

/* Overkill?  I don't know.  Need to figure out top of mem to make the
 * SMRAM calculations come out.  Linux seems to have problems
 * detecting it all on its own, so this seems a reasonable double
 * check to any user supplied 'mem=...' boot param.
 *
 * ... unfortunately this reg doesn't work according to spec on the
 * test hardware.
 */
#define WHTCFG_PAMR_DRP      0x50
#define SYS_DRAM_ROW_0_SHIFT    16
#define SYS_DRAM_ROW_1_SHIFT    20
#define DRAM_MASK           0x0f
#define DRAM_VALUE_0        0
#define DRAM_VALUE_1        8
/* No 2 value defined */
#define DRAM_VALUE_3        16
#define DRAM_VALUE_4        16
#define DRAM_VALUE_5        24
#define DRAM_VALUE_6        32
#define DRAM_VALUE_7        32
#define DRAM_VALUE_8        48
#define DRAM_VALUE_9        64
#define DRAM_VALUE_A        64
#define DRAM_VALUE_B        96
#define DRAM_VALUE_C        128
#define DRAM_VALUE_D        128
#define DRAM_VALUE_E        192
#define DRAM_VALUE_F        256	/* nice one, geezer */
#define LM_FREQ_MASK        0x10
#define LM_FREQ_133         0x10
#define LM_FREQ_100         0x00




/* These are 3d state registers, but the state is invarient, so we let
 * the X server handle it:
 */



/* GFXRENDERSTATE_COLOR_CHROMA_KEY, p135
 */
#define GFX_OP_COLOR_CHROMA_KEY  ((0x3<<29)|(0x1d<<24)|(0x2<<16)|0x1)
#define CC1_UPDATE_KILL_WRITE    (1<<28)
#define CC1_ENABLE_KILL_WRITE    (1<<27)
#define CC1_DISABLE_KILL_WRITE    0
#define CC1_UPDATE_COLOR_IDX     (1<<26)
#define CC1_UPDATE_CHROMA_LOW    (1<<25)
#define CC1_UPDATE_CHROMA_HI     (1<<24)
#define CC1_CHROMA_LOW_MASK      ((1<<24)-1)
#define CC2_COLOR_IDX_SHIFT      24
#define CC2_COLOR_IDX_MASK       (0xff<<24)
#define CC2_CHROMA_HI_MASK       ((1<<24)-1)


#define GFX_CMD_CONTEXT_SEL      ((0<<29)|(0x5<<23))
#define CS_UPDATE_LOAD           (1<<17)
#define CS_UPDATE_USE            (1<<16)
#define CS_UPDATE_LOAD           (1<<17)
#define CS_LOAD_CTX0             0
#define CS_LOAD_CTX1             (1<<8)
#define CS_USE_CTX0              0
#define CS_USE_CTX1              (1<<0)

/* 3D Rendering Engine */

#define RENDER_CLIENT            0x60000000

/* Primitive rendering instruction */

#define GFX_PRIMITIVE            0x1f000000
#define PRIMITIVE_TRIANGLE       0 << 18
#define PRIMITIVE_TRI_STRIP      1 << 18
#define PRIMITIVE_TRI_REV_STRIP  2 << 18
#define PRIMITIVE_TRI_FAN        3 << 18
#define PRIMITIVE_POLYGON        4 << 18
#define PRIMITIVE_LINE           5 << 18
#define PRIMITIVE_LINE_STRIP     6 << 18
#define PRIMITIVE_RECTANGLE      7 << 18

/* Vertex format instruction */
#define GFX_VERTEX_FORMAT        0x05000000
#define VERTEX_0_TEXCOORDS       0 << 8
#define VERTEX_1_TEXCOORDS       1 << 8
#define VERTEX_2_TEXCOORDS       2 << 8
#define VERTEX_SPECULAR_FOG      1 << 7
#define VERTEX_DIFFUSE_ALPHA     1 << 6
#define VERTEX_Z_OFFSET          1 << 5
#define VERTEX_POS_XYZ           1 << 1
#define VERTEX_POS_XYZ_RHW       2 << 1
#define VERTEX_POS_XY            3 << 1
#define VERTEX_POS_XY_RHW        4 << 1

/* Drawing Rectangle Info instruction */

#define GFX_DRAWING_RECTANGLE_INFO 0x1d800003
#define GFX_DRAWING_CLIP_DISABLE 1<<31
 
/* Boolean enable 1 */
#define GFX_BOOLEAN_ENA_1        0x03000000
#define BOOL1_ALPHA_SETUP_MASK   1<<17
#define BOOL1_ALPHA_SETUP_BIT    1<<16
#define BOOL1_FOG_ENABLE_MASK    1<<7
#define BOOL1_FOG_ENABLE_BIT     1<<6
#define BOOL1_ALPHA_TEST_MASK    1<<5
#define BOOL1_ALPHA_TEST_BIT     1<<4
#define BOOL1_BLEND_ENABLE_MASK  1<<3
#define BOOL1_BLEND_ENABLE_BIT   1<<2
#define BOOL1_Z_ENABLE_MASK      1<<1
#define BOOL1_Z_ENABLE_BIT       1<<0

/* Boolean enable 2 */
#define GFX_BOOLEAN_ENA_2        0x04000000
#define BOOL2_MAPPING_CACHE_MASK 1<<17
#define BOOL2_MAPPING_CACHE_BIT  1<<16
#define BOOL2_ALPHA_DITHER_MASK  1<<15
#define BOOL2_ALPHA_DITHER_BIT   1<<14
#define BOOL2_FOG_DITHER_MASK    1<<13
#define BOOL2_FOG_DITHER_BIT     1<<12
#define BOOL2_SPECULAR_DITHER_MASK 1<<11
#define BOOL2_SPECULAR_DITHER_BIT  1<<10
#define BOOL2_COLOR_DITHER_MASK  1<<9
#define BOOL2_COLOR_DITHER_BIT   1<<8
#define BOOL2_FB_WRITE_MASK      1<<3
#define BOOL2_FB_WRITE_BIT       1<<2
#define BOOL2_Z_WRITE_MASK       1<<1
#define BOOL2_Z_WRITE_BIT        1<<0

/* Dest buffer variables */

#define GFX_DEST_BUFFER_VARIABLES 0x1d850000

#define DEST_BUF_VAR_8BIT        0 << 8
#define DEST_BUF_VAR_555         1 << 8
#define DEST_BUF_VAR_565         2 << 8

/* map color blend stages */

#define GFX_MAP_COLOR_BLEND_STAGES 0

#define MAP_BLEND_STAGE_B        20
#define MAP_BLEND_ACC_SEL_MASK   1<<19
#define MAP_BLEND_ACC_SEL_BIT    1<<18
#define MAP_BLEND_ARG1_MASK      1<<17
#define MAP_BLEND_ARG1_B         14
#define MAP_BLEND_REPLICATE_ARG1 1<<13
#define MAP_BLEND_INVERT_ARG1    1<<12

#define MAP_BLEND_ARG2_MASK      1<<11
#define MAP_BLEND_ARG2_B         8
#define MAP_BLEND_REPLICATE_ARG2 1<<7
#define MAP_BLEND_INVERT_ARG2    1<<6

#define MAP_BLEND_COLOR_OP_MASK  1<<5
#define MAP_BLEND_COLOR_OP_B     0

#define GFX_SCISSOR_ENABLE       0x1c800000

#define SCISSOR_ENABLE_MASK      1<<1
#define SCISSOR_ENABLE_BIT       1<<0
