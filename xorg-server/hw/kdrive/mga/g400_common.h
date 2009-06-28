/*
 * Copyright © 2004 Damien Ciabrini
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Anders Carlsson not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Anders Carlsson makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * DAMIEN CIABRINI DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ANDERS CARLSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __G400_COMMON_H__
#define __G400_COMMON_H__

/* Color registers */
#define MGA_REG_DR4             		(0x1CD0)
#define MGA_REG_DR6             		(0x1CD8)
#define MGA_REG_DR7             		(0x1CDC)
#define MGA_REG_DR8             		(0x1CE0)
#define MGA_REG_DR10            		(0x1CE8)
#define MGA_REG_DR11            		(0x1CEC)
#define MGA_REG_DR12            		(0x1CF0)
#define MGA_REG_DR14            		(0x1CF8)
#define MGA_REG_DR15            		(0x1CFC)

/* Alpha registers */
#define MGA_REG_ALPHASTART      		(0x2C70)
#define MGA_REG_ALPHAXINC       		(0x2C74)
#define MGA_REG_ALPHAYINC       		(0x2C78)
#define MGA_REG_ALPHACTRL       		(0x2C7C)
#    define MGA_SRC_ZERO                     	(0x00000000)
#    define MGA_SRC_ONE                      	(0x00000001)
#    define MGA_SRC_DST_COLOR                	(0x00000002)
#    define MGA_SRC_ONE_MINUS_DST_COLOR      	(0x00000003)
#    define MGA_SRC_ALPHA                    	(0x00000004)
#    define MGA_SRC_ONE_MINUS_SRC_ALPHA      	(0x00000005)
#    define MGA_SRC_DST_ALPHA                	(0x00000006)
#    define MGA_SRC_ONE_MINUS_DST_ALPHA      	(0x00000007)
#    define MGA_SRC_SRC_ALPHA_SATURATE       	(0x00000008)
#    define MGA_SRC_BLEND_MASK			(0x0000000f)

#    define MGA_DST_ZERO                     	(0x00000000)
#    define MGA_DST_ONE                      	(0x00000010)
#    define MGA_DST_SRC_COLOR                	(0x00000020)
#    define MGA_DST_ONE_MINUS_SRC_COLOR      	(0x00000030)
#    define MGA_DST_SRC_ALPHA                	(0x00000040)
#    define MGA_DST_ONE_MINUS_SRC_ALPHA      	(0x00000050)
#    define MGA_DST_DST_ALPHA                	(0x00000060)
#    define MGA_DST_ONE_MINUS_DST_ALPHA      	(0x00000070)
#    define MGA_DST_BLEND_MASK			(0x00000070)

#    define MGA_ALPHACHANNEL                 	(0x00000100)
#    define MGA_VIDEOALPHA                   	(0x00000200)
#    define MGA_DIFFUSEDALPHA                	(0x01000000)
#    define MGA_MODULATEDALPHA               	(0x02000000)

/* Textures registers */
#define MGA_REG_TEXORG 				(0x2c24)
#define MGA_REG_TEXORG1				(0x2ca4)
#define MGA_REG_TEXWIDTH        		(0x2C28)
#define MGA_REG_TEXHEIGHT       		(0x2C2C)
#define MGA_REG_TMR0            		(0x2C00)
#define MGA_REG_TMR1            		(0x2C04)
#define MGA_REG_TMR2            		(0x2C08)
#define MGA_REG_TMR3            		(0x2C0C)
#define MGA_REG_TMR4            		(0x2C10)
#define MGA_REG_TMR5            		(0x2C14)
#define MGA_REG_TMR6            		(0x2C18)
#define MGA_REG_TMR7            		(0x2C1C)
#define MGA_REG_TMR8            		(0x2C20)

/* Texture Control Registers */
#define MGA_REG_TEXCTL          		(0x2C30)
#    define MGA_TW4				(0x00000000)
#    define MGA_TW8           			(0x00000001)
#    define MGA_TW15          			(0x00000002)
#    define MGA_TW16          			(0x00000003)
#    define MGA_TW12          			(0x00000004)
#    define MGA_TW32				(0x00000006)
#    define MGA_TW8A          			(0x00000007)
#    define MGA_TW8AL         			(0x00000008)
#    define MGA_TW422         			(0x0000000A)
#    define MGA_TW422UYVY     			(0x0000000B)
#    define MGA_PITCHLIN			(0x00000100)
#    define MGA_NOPERSPECTIVE			(0x00200000)
#    define MGA_TAKEY           	     	(0x02000000)
#    define MGA_TAMASK				(0x04000000)
#    define MGA_CLAMPUV				(0x18000000)
#define MGA_REG_TEXCTL2         		(0x2C3C)
#    define MGA_G400_TC2_MAGIC			(0x00008000)
#    define MGA_TC2_DECALBLEND 			(0x00000001)
#    define MGA_TC2_IDECAL 			(0x00000002)
#    define MGA_TC2_DECALDIS 			(0x00000004)
#    define MGA_TC2_CKSTRANSDIS 		(0x00000010)
#    define MGA_TC2_BORDEREN 			(0x00000020)
#    define MGA_TC2_SPECEN 			(0x00000040)
#    define MGA_TC2_DUALTEX 			(0x00000080)
#    define MGA_TC2_TABLEFOG 			(0x00000100)
#    define MGA_TC2_BUMPMAP 			(0x00000200)
#    define MGA_TC2_SELECT_TMU1 		(0x80000000)
#define MGA_REG_TEXFILTER       		(0x2C58)
#    define MGA_MIN_NRST            		(0x00000000)
#    define MGA_MIN_BILIN           		(0x00000002)
#    define MGA_MIN_ANISO           		(0x0000000D)
#    define MGA_MAG_NRST            		(0x00000000)
#    define MGA_MAG_BILIN           		(0x00000020)
	
/* Multi-Textures Control Registers */
#define MGA_REG_TDUALSTAGE0   			(0x2CF8)
#define MGA_REG_TDUALSTAGE1   			(0x2CFC)
#    define MGA_TDS_COLOR_ARG2_DIFFUSE 		(0x00000000)
#    define MGA_TDS_COLOR_ARG2_SPECULAR 	(0x00000001)
#    define MGA_TDS_COLOR_ARG2_FCOL 		(0x00000002)
#    define MGA_TDS_COLOR_ARG2_PREVSTAGE 	(0x00000003)
#    define MGA_TDS_COLOR_ALPHA_DIFFUSE 	(0x00000000)
#    define MGA_TDS_COLOR_ALPHA_FCOL 		(0x00000004)
#    define MGA_TDS_COLOR_ALPHA_CURRTEX 	(0x00000008)
#    define MGA_TDS_COLOR_ALPHA_PREVTEX 	(0x0000000c)
#    define MGA_TDS_COLOR_ALPHA_PREVSTAGE 	(0x00000010)
#    define MGA_TDS_COLOR_ARG1_REPLICATEALPHA 	(0x00000020)
#    define MGA_TDS_COLOR_ARG1_INV 		(0x00000040)
#    define MGA_TDS_COLOR_ARG2_REPLICATEALPHA 	(0x00000080)
#    define MGA_TDS_COLOR_ARG2_INV 		(0x00000100)
#    define MGA_TDS_COLOR_ALPHA1INV 		(0x00000200)
#    define MGA_TDS_COLOR_ALPHA2INV 		(0x00000400)
#    define MGA_TDS_COLOR_ARG1MUL_ALPHA1 	(0x00000800)
#    define MGA_TDS_COLOR_ARG2MUL_ALPHA2 	(0x00001000)
#    define MGA_TDS_COLOR_ARG1ADD_MULOUT 	(0x00002000)
#    define MGA_TDS_COLOR_ARG2ADD_MULOUT 	(0x00004000)
#    define MGA_TDS_COLOR_MODBRIGHT_2X 		(0x00008000)
#    define MGA_TDS_COLOR_MODBRIGHT_4X 		(0x00010000)
#    define MGA_TDS_COLOR_ADD_SUB 		(0x00000000)
#    define MGA_TDS_COLOR_ADD_ADD 		(0x00020000)
#    define MGA_TDS_COLOR_ADD2X 		(0x00040000)
#    define MGA_TDS_COLOR_ADDBIAS 		(0x00080000)
#    define MGA_TDS_COLOR_BLEND 		(0x00100000)
#    define MGA_TDS_COLOR_SEL_ARG1 		(0x00000000)
#    define MGA_TDS_COLOR_SEL_ARG2 		(0x00200000)
#    define MGA_TDS_COLOR_SEL_ADD 		(0x00400000)
#    define MGA_TDS_COLOR_SEL_MUL 		(0x00600000)
#    define MGA_TDS_ALPHA_ARG1_INV 		(0x00800000)
#    define MGA_TDS_ALPHA_ARG2_DIFFUSE 		(0x00000000)
#    define MGA_TDS_ALPHA_ARG2_FCOL 		(0x01000000)
#    define MGA_TDS_ALPHA_ARG2_PREVTEX 		(0x02000000)
#    define MGA_TDS_ALPHA_ARG2_PREVSTAGE 	(0x03000000)
#    define MGA_TDS_ALPHA_ARG2_INV 		(0x04000000)
#    define MGA_TDS_ALPHA_ADD 			(0x08000000)
#    define MGA_TDS_ALPHA_ADDBIAS 		(0x10000000)
#    define MGA_TDS_ALPHA_ADD2X 		(0x20000000)
#    define MGA_TDS_ALPHA_SEL_ARG1 		(0x00000000)
#    define MGA_TDS_ALPHA_SEL_ARG2 		(0x40000000)
#    define MGA_TDS_ALPHA_SEL_ADD 		(0x80000000)
#    define MGA_TDS_ALPHA_SEL_MUL 		(0xc0000000)



/* Composition Prototypes. MMIO Access */
Bool
mgaCheckComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
		  PicturePtr pDstPicture);

Bool
mgaPrepareComposite(int op, PicturePtr pSrcPicture, PicturePtr pMaskPicture,
		    PicturePtr pDstPicture, PixmapPtr pSrc,
		    PixmapPtr pMask, PixmapPtr pDst);

void
mgaComposite(int srcX, int srcY, int maskX, int maskY,
	     int dstX, int dstY, int w, int h);

void
mgaDoneComposite(void);

#endif
