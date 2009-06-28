/*

   int XAAHelpSolidROP(ScrnInfoPtr pScrn, int *fg, int pm, int *rop)

	For use with solid fills emulated by solid 8x8 patterns.  You 
	give it the foreground, planemask and X rop and it will replace 
	the foreground with a new one and the rop with the appropriate 
	MS triadic raster op.  The function will return which components 
	(S-P) need to be enabled.  


   int XAAHelpPatternROP(ScrnInfoPtr pScrn, int *fg, int *bg, int pm, int *rop)

	For use with 8x8 opaque pattern fills.  You give it the foreground, 	
	and background, planemask and X rop and it will replace the 
	foreground and background with new ones and the rop with the 
	appropriate MS triadic raster op.  The function will return which 
	components (S-P) need to be enabled.  


	   ROP_PAT - Means to enable 8x8 mono patterns (all bits 
		     set for solid patterns).  Set the foreground and
		     background as returned by the function.  

	   ROP_SRC - Means a source of color == planemask should be used.


*/

#ifndef _XAAROP_H
#define _XAAROP_H

#define ROP_DST		0x00000001
#define ROP_SRC		0x00000002
#define ROP_PAT		0x00000004

#define ROP_0		0x00
#define ROP_DPSoon	0x01
#define ROP_DPSona	0x02
#define ROP_PSon	0x03
#define ROP_SDPona	0x04
#define ROP_DPon	0x05
#define ROP_PDSxnon	0x06
#define ROP_PDSaon	0x07
#define ROP_SDPnaa	0x08
#define ROP_PDSxon	0x09
#define ROP_DPna	0x0A
#define ROP_PSDnaon	0x0B
#define ROP_SPna	0x0C
#define ROP_PDSnaon	0x0D
#define ROP_PDSonon	0x0E
#define ROP_Pn		0x0F
#define ROP_PDSona	0x10
#define ROP_DSon	0x11
#define ROP_SDPxnon	0x12
#define ROP_SDPaon	0x13
#define ROP_DPSxnon	0x14
#define ROP_DPSaon	0x15
#define ROP_PSDPSanaxx	0x16
#define ROP_SSPxDSxaxn	0x17
#define ROP_SPxPDxa	0x18
#define ROP_SDPSanaxn	0x19
#define ROP_PDSPaox	0x1A
#define ROP_SDPSxaxn	0x1B
#define ROP_PSDPaox	0x1C
#define ROP_DSPDxaxn	0x1D
#define ROP_PDSox	0x1E
#define ROP_PDSoan	0x1F
#define ROP_DPSnaa	0x20
#define ROP_SDPxon	0x21
#define ROP_DSna	0x22
#define ROP_SPDnaon	0x23
#define ROP_SPxDSxa	0x24
#define ROP_PDSPanaxn	0x25
#define ROP_SDPSaox	0x26
#define ROP_SDPSxnox	0x27
#define ROP_DPSxa	0x28
#define ROP_PSDPSaoxxn	0x29
#define ROP_DPSana	0x2A
#define ROP_SSPxPDxaxn	0x2B
#define ROP_SPDSoax	0x2C
#define ROP_PSDnox	0x2D
#define ROP_PSDPxox	0x2E
#define ROP_PSDnoan	0x2F
#define ROP_PSna	0x30
#define ROP_SDPnaon	0x31
#define ROP_SDPSoox	0x32
#define ROP_Sn		0x33
#define ROP_SPDSaox	0x34
#define ROP_SPDSxnox	0x35
#define ROP_SDPox	0x36
#define ROP_SDPoan	0x37
#define ROP_PSDPoax	0x38
#define ROP_SPDnox	0x39
#define ROP_SPDSxox	0x3A
#define ROP_SPDnoan	0x3B
#define ROP_PSx		0x3C
#define ROP_SPDSonox	0x3D
#define ROP_SPDSnaox	0x3E
#define ROP_PSan	0x3F
#define ROP_PSDnaa	0x40
#define ROP_DPSxon	0x41
#define ROP_SDxPDxa	0x42
#define ROP_SPDSanaxn	0x43
#define ROP_SDna	0x44
#define ROP_DPSnaon	0x45
#define ROP_DSPDaox	0x46
#define ROP_PSDPxaxn	0x47
#define ROP_SDPxa	0x48
#define ROP_PDSPDaoxxn	0x49
#define ROP_DPSDoax	0x4A
#define ROP_PDSnox	0x4B
#define ROP_SDPana	0x4C
#define ROP_SSPxDSxoxn	0x4D
#define ROP_PDSPxox	0x4E
#define ROP_PDSnoan	0x4F
#define ROP_PDna	0x50
#define ROP_DSPnaon	0x51
#define ROP_DPSDaox	0x52
#define ROP_SPDSxaxn	0x53
#define ROP_DPSonon	0x54
#define ROP_Dn		0x55
#define ROP_DPSox	0x56
#define ROP_DPSoan	0x57
#define ROP_PDSPoax	0x58
#define ROP_DPSnox	0x59
#define ROP_DPx		0x5A
#define ROP_DPSDonox	0x5B
#define ROP_DPSDxox	0x5C
#define ROP_DPSnoan	0x5D
#define ROP_DPSDnaox	0x5E
#define ROP_DPan	0x5F
#define ROP_PDSxa	0x60
#define ROP_DSPDSaoxxn	0x61
#define ROP_DSPDoax	0x62
#define ROP_SDPnox	0x63
#define ROP_SDPSoax	0x64
#define ROP_DSPnox	0x65
#define ROP_DSx		0x66
#define ROP_SDPSonox	0x67
#define ROP_DSPDSonoxxn	0x68
#define ROP_PDSxxn	0x69
#define ROP_DPSax	0x6A
#define ROP_PSDPSoaxxn	0x6B
#define ROP_SDPax	0x6C
#define ROP_PDSPDoaxxn	0x6D
#define ROP_SDPSnoax	0x6E
#define ROP_PDSxnan	0x6F
#define ROP_PDSana	0x70
#define ROP_SSDxPDxaxn	0x71
#define ROP_SDPSxox	0x72
#define ROP_SDPnoan	0x73
#define ROP_DSPDxox	0x74
#define ROP_DSPnoan	0x75
#define ROP_SDPSnaox	0x76
#define ROP_DSan	0x77
#define ROP_PDSax	0x78
#define ROP_DSPDSoaxxn	0x79
#define ROP_DPSDnoax	0x7A
#define ROP_SDPxnan	0x7B
#define ROP_SPDSnoax	0x7C
#define ROP_DPSxnan	0x7D
#define ROP_SPxDSxo	0x7E
#define ROP_DPSaan	0x7F
#define ROP_DPSaa	0x80
#define ROP_SPxDSxon	0x81
#define ROP_DPSxna	0x82
#define ROP_SPDSnoaxn	0x83
#define ROP_SDPxna	0x84
#define ROP_PDSPnoaxn	0x85
#define ROP_DSPDSoaxx	0x86
#define ROP_PDSaxn	0x87
#define ROP_DSa		0x88
#define ROP_SDPSnaoxn	0x89
#define ROP_DSPnoa	0x8A
#define ROP_DSPDxoxn	0x8B
#define ROP_SDPnoa	0x8C
#define ROP_SDPSxoxn	0x8D
#define ROP_SSDxPDxax	0x8E
#define ROP_PDSanan	0x8F
#define ROP_PDSxna	0x90
#define ROP_SDPSnoaxn	0x91
#define ROP_DPSDPoaxx	0x92
#define ROP_SPDaxn	0x93
#define ROP_PSDPSoaxx	0x94
#define ROP_DPSaxn	0x95
#define ROP_DPSxx	0x96
#define ROP_PSDPSonoxx	0x97
#define ROP_SDPSonoxn	0x98
#define ROP_DSxn	0x99
#define ROP_DPSnax	0x9A
#define ROP_SDPSoaxn	0x9B
#define ROP_SPDnax	0x9C
#define ROP_DSPDoaxn	0x9D
#define ROP_DSPDSaoxx	0x9E
#define ROP_PDSxan	0x9F
#define ROP_DPa		0xA0
#define ROP_PDSPnaoxn	0xA1
#define ROP_DPSnoa	0xA2
#define ROP_DPSDxoxn	0xA3
#define ROP_PDSPonoxn	0xA4
#define ROP_PDxn	0xA5
#define ROP_DSPnax	0xA6
#define ROP_PDSPoaxn	0xA7
#define ROP_DPSoa	0xA8
#define ROP_DPSoxn	0xA9
#define ROP_D		0xAA
#define ROP_DPSono	0xAB
#define ROP_SPDSxax	0xAC
#define ROP_DPSDaoxn	0xAD
#define ROP_DSPnao	0xAE
#define ROP_DPno	0xAF
#define ROP_PDSnoa	0xB0
#define ROP_PDSPxoxn	0xB1
#define ROP_SSPxDSxox	0xB2
#define ROP_SDPanan	0xB3
#define ROP_PSDnax	0xB4
#define ROP_DPSDoaxn	0xB5
#define ROP_DPSDPaoxx	0xB6
#define ROP_SDPxan	0xB7
#define ROP_PSDPxax	0xB8
#define ROP_DSPDaoxn	0xB9
#define ROP_DPSnao	0xBA
#define ROP_DSno	0xBB
#define ROP_SPDSanax	0xBC
#define ROP_SDxPDxan	0xBD
#define ROP_DPSxo	0xBE
#define ROP_DPSano	0xBF
#define ROP_Psa		0xC0
#define ROP_SPDSnaoxn	0xC1
#define ROP_SPDSonoxn	0xC2
#define ROP_PSxn	0xC3
#define ROP_SPDnoa	0xC4
#define ROP_SPDSxoxn	0xC5
#define ROP_SDPnax	0xC6
#define ROP_PSDPoaxn	0xC7
#define ROP_SDPoa	0xC8
#define ROP_SPDoxn	0xC9
#define ROP_DPSDxax	0xCA
#define ROP_SPDSaoxn	0xCB
#define ROP_S		0xCC
#define ROP_SDPono	0xCD
#define ROP_SDPnao	0xCE
#define ROP_SPno	0xCF
#define ROP_PSDnoa	0xD0
#define ROP_PSDPxoxn	0xD1
#define ROP_PDSnax	0xD2
#define ROP_SPDSoaxn	0xD3
#define ROP_SSPxPDxax	0xD4
#define ROP_DPSanan	0xD5
#define ROP_PSDPSaoxx	0xD6
#define ROP_DPSxan	0xD7
#define ROP_PDSPxax	0xD8
#define ROP_SDPSaoxn	0xD9
#define ROP_DPSDanax	0xDA
#define ROP_SPxDSxan	0xDB
#define ROP_SPDnao	0xDC
#define ROP_SDno	0xDD
#define ROP_SDPxo	0xDE
#define ROP_SDPano	0xDF
#define ROP_PDSoa	0xE0
#define ROP_PDSoxn	0xE1
#define ROP_DSPDxax	0xE2
#define ROP_PSDPaoxn	0xE3
#define ROP_SDPSxax	0xE4
#define ROP_PDSPaoxn	0xE5
#define ROP_SDPSanax	0xE6
#define ROP_SPxPDxan	0xE7
#define ROP_SSPxDSxax	0xE8
#define ROP_DSPDSanaxxn	0xE9
#define ROP_DPSao	0xEA
#define ROP_DPSxno	0xEB
#define ROP_SDPao	0xEC
#define ROP_SDPxno	0xED
#define ROP_DSo		0xEE
#define ROP_SDPnoo	0xEF
#define ROP_P		0xF0
#define ROP_PDSono	0xF1
#define ROP_PDSnao	0xF2
#define ROP_PSno	0xF3
#define ROP_PSDnao	0xF4
#define ROP_PDno	0xF5
#define ROP_PDSxo	0xF6
#define ROP_PDSano	0xF7
#define ROP_PDSao	0xF8
#define ROP_PDSxno	0xF9
#define ROP_DPo		0xFA
#define ROP_DPSnoo	0xFB
#define ROP_PSo		0xFC
#define ROP_PSDnoo	0xFD
#define ROP_DPSoo	0xFE
#define ROP_1		0xFF

#define NO_SRC_ROP(rop) \
   ((rop == GXnoop) || (rop == GXset) || (rop == GXclear) || (rop == GXinvert))

int XAAHelpSolidROP(ScrnInfoPtr pScrn, int *fg, int pm, int *rop);
int XAAHelpPatternROP(ScrnInfoPtr pScrn, int *fg, int *bg, int pm, int *rop);

/* XXX These four should be static, but it breaks the 6.7.0 ABI. */
extern int XAACopyROP[16];
extern int XAACopyROP_PM[16];
extern int XAAPatternROP[16];
extern int XAAPatternROP_PM[16];

extern int XAAGetCopyROP(int i);
extern int XAAGetCopyROP_PM(int i);
extern int XAAGetPatternROP(int i);
extern int XAAGetPatternROP_PM(int i);

#endif /* _XAAROP_H */
