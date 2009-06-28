/*
 * Copyright 2004 by Costas Stylianou <costas.stylianou@psion.com> +44(0)7850 394095
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Costas Sylianou not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission. Costas Stylianou makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * COSTAS STYLIANOU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL COSTAS STYLIANOU BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* 
 * epson13806reg.h         Epson S1D13806 LCD controller header file.
 *
 * History:
 * 15-Feb-04  C.Stylianou       PRJ NBL: Created.
 *
 */
 

#ifndef EPSON13806REG_H
#define EPSON13806REG_H


#define TT_UNUSED(x) ((void) (x))


#define EPSON13806_PHYSICAL_REG_ADDR   0x14000000
#define EPSON13806_GPIO_REGSIZE        0x001f0000

#define EPSON13806_PHYSICAL_VMEM_ADDR  0x14200000
#define EPSON13806_VMEM_SIZE           0x140000

#define PLATFORM_EPSON13806_BASE    (regbase)
#define EPSON13806_REG_BASE         (PLATFORM_EPSON13806_BASE + 0x000000)   // Register base address
#define EPSON13806_SDRAM_BASE       (PLATFORM_EPSON13806_BASE + 0x200000)   // SDRAM base address

//////////////////////////////////////////////////////////////////////////////////////////
// Register Offsets
//////////////////////////////////////////////////////////////////////////////////////////
#define EPSON13806_REVCODE              (EPSON13806_REG_BASE + 0x0000)  // Revision Code Register
#define EPSON13806_MISC                 (EPSON13806_REG_BASE + 0x0001)  // Miscellaneous Register
#define EPSON13806_GPIOCFG              (EPSON13806_REG_BASE + 0x0004)  // General IO Pins Configuration Register (16 bits)
#define EPSON13806_GPIOCFG0             (EPSON13806_REG_BASE + 0x0004)  // General IO Pins Configuration Register 0
#define EPSON13806_GPIOCFG1             (EPSON13806_REG_BASE + 0x0005)  // General IO Pins Configuration Register 1
#define EPSON13806_GPIOCTRL             (EPSON13806_REG_BASE + 0x0008)  // General IO Pins Control Register (16 bits)
#define EPSON13806_GPIOCTRL0            (EPSON13806_REG_BASE + 0x0008)  // General IO Pins Control Register 0
#define EPSON13806_GPIOCTRL1            (EPSON13806_REG_BASE + 0x0009)  // General IO Pins Control Register 1
#define EPSON13806_MDCFGSTATUS          (EPSON13806_REG_BASE + 0x000C)  // Configuration Status Register
#define EPSON13806_MEMCLKCFG            (EPSON13806_REG_BASE + 0x0010)  // Memory Clock Configuration Register
#define EPSON13806_LCDPCLKCFG           (EPSON13806_REG_BASE + 0x0014)  // LCD Pixel Clock Configuration Register
#define EPSON13806_CRTPCLKCFG           (EPSON13806_REG_BASE + 0x0018)  // CRT/TV Clock Configuration Register
#define EPSON13806_MPCLKCFG             (EPSON13806_REG_BASE + 0x001C)  // MediaPlug Clock Configuration Register
#define EPSON13806_CPUMEMWAITSEL        (EPSON13806_REG_BASE + 0x001E)  // CPU To Memory Wait State Select Register
#define EPSON13806_MEMCFG               (EPSON13806_REG_BASE + 0x0020)  // Memory Configuration Register
#define EPSON13806_DRAMREFRESH          (EPSON13806_REG_BASE + 0x0021)  // DRAM Refresh Rate Register
#define EPSON13806_DRAMTIMINGCTRL       (EPSON13806_REG_BASE + 0x002A)  // DRAM Timings Control Register (16 bits)
#define EPSON13806_DRAMTIMINGCTRL0      (EPSON13806_REG_BASE + 0x002A)  // DRAM Timings Control Register 0
#define EPSON13806_DRAMTIMINGCTRL1      (EPSON13806_REG_BASE + 0x002B)  // DRAM Timings Control Register 1
#define EPSON13806_PANELTYPE            (EPSON13806_REG_BASE + 0x0030)  // Panel Type Register
#define EPSON13806_MODRATE              (EPSON13806_REG_BASE + 0x0031)  // MOD Rate Register
#define EPSON13806_LCDHDP               (EPSON13806_REG_BASE + 0x0032)  // LCD Horizontal Display Width Register
#define EPSON13806_LCDHNDP              (EPSON13806_REG_BASE + 0x0034)  // LCD Horizontal Non-Display Period Register
#define EPSON13806_TFTFPLINESTART       (EPSON13806_REG_BASE + 0x0035)  // TFT FPLINE Start Position Register
#define EPSON13806_TFTFPLINEPULSE       (EPSON13806_REG_BASE + 0x0036)  // TFT FPLINE Pulse Width Register
#define EPSON13806_LCDVDP               (EPSON13806_REG_BASE + 0x0038)  // LCD Vertical Display Height Register (16 bits)
#define EPSON13806_LCDVDP0              (EPSON13806_REG_BASE + 0x0038)  // LCD Vertical Display Height Register 0
#define EPSON13806_LCDVDP1              (EPSON13806_REG_BASE + 0x0039)  // LCD Vertical Display Height Register 1
#define EPSON13806_LCDVNDP              (EPSON13806_REG_BASE + 0x003A)  // LCD Vertical Non-Display Period Register
#define EPSON13806_TFTFPFRAMESTART      (EPSON13806_REG_BASE + 0x003B)  // TFT FPFRAME Start Position Register
#define EPSON13806_TFTFPFRAMEPULSE      (EPSON13806_REG_BASE + 0x003C)  // TFT FPFRAME Pulse Width Register
#define EPSON13806_LCDLINECOUNT         (EPSON13806_REG_BASE + 0x003E)  // LCD Line Count Register (16 bits)
#define EPSON13806_LCDLINECOUNT0        (EPSON13806_REG_BASE + 0x003E)  // LCD Line Count Register 0
#define EPSON13806_LCDLINECOUNT1        (EPSON13806_REG_BASE + 0x003F)  // LCD Line Count Register 1
#define EPSON13806_LCDDISPMODE          (EPSON13806_REG_BASE + 0x0040)  // LCD Display Mode Register
#define EPSON13806_LCDMISC              (EPSON13806_REG_BASE + 0x0041)  // LCD Miscellaneous Register
#define EPSON13806_LCDSTART01           (EPSON13806_REG_BASE + 0x0042)  // LCD Display Start Address Register 0 and 1 (16 bits)
#define EPSON13806_LCDSTART0            (EPSON13806_REG_BASE + 0x0042)  // LCD Display Start Address Register 0
#define EPSON13806_LCDSTART1            (EPSON13806_REG_BASE + 0x0043)  // LCD Display Start Address Register 1
#define EPSON13806_LCDSTART2            (EPSON13806_REG_BASE + 0x0044)  // LCD Display Start Address Register 2
#define EPSON13806_LCDSTRIDE            (EPSON13806_REG_BASE + 0x0046)  // LCD Memory Address Offset Register (16 bits)
#define EPSON13806_LCDSTRIDE0           (EPSON13806_REG_BASE + 0x0046)  // LCD Memory Address Offset Register 0
#define EPSON13806_LCDSTRIDE1           (EPSON13806_REG_BASE + 0x0047)  // LCD Memory Address Offset Register 1
#define EPSON13806_LCDPIXELPAN          (EPSON13806_REG_BASE + 0x0048)  // LCD Pixel Panning Register
#define EPSON13806_LCDFIFOHIGH          (EPSON13806_REG_BASE + 0x004A)  // LCD Display FIFO High Threshold Control Register
#define EPSON13806_LCDFIFOLOW           (EPSON13806_REG_BASE + 0x004B)  // LCD Display FIFO Low Threshold Control Register

#define EPSON13806_LCDINKCURSCTRL       (EPSON13806_REG_BASE + 0x0070)  // LCD INK/Cursor Control Register
#define EPSON13806_LCDINKCURSSTART      (EPSON13806_REG_BASE + 0x0071)  // LCD INK/Cursor Start Address Register
#define EPSON13806_LCDCURSORXPOS        (EPSON13806_REG_BASE + 0x0072)  // LCD Cursor X Position Register (16 bits)
#define EPSON13806_LCDCURSORXPOS0       (EPSON13806_REG_BASE + 0x0072)  // LCD Cursor X Position Register 0
#define EPSON13806_LCDCURSORXPOS1       (EPSON13806_REG_BASE + 0x0073)  // LCD Cursor X Position Register 1
#define EPSON13806_LCDCURSORYPOS        (EPSON13806_REG_BASE + 0x0074)  // LCD Cursor Y Position Register (16 bits)
#define EPSON13806_LCDCURSORYPOS0       (EPSON13806_REG_BASE + 0x0074)  // LCD Cursor Y Position Register 0
#define EPSON13806_LCDCURSORYPOS1       (EPSON13806_REG_BASE + 0x0075)  // LCD Cursor Y Position Register 1
#define EPSON13806_LCDINKCURSBLUE0      (EPSON13806_REG_BASE + 0x0076)  // LCD INK/Cursor Blue Color 0 Register
#define EPSON13806_LCDINKCURSGREEN0     (EPSON13806_REG_BASE + 0x0077)  // LCD INK/Cursor Green Color 0 Register
#define EPSON13806_LCDINKCURSRED0       (EPSON13806_REG_BASE + 0x0078)  // LCD INK/Cursor Red Color 0 Register
#define EPSON13806_LCDINKCURSBLUE1      (EPSON13806_REG_BASE + 0x007A)  // LCD INK/Cursor Blue Color 1 Register
#define EPSON13806_LCDINKCURSGREEN1     (EPSON13806_REG_BASE + 0x007B)  // LCD INK/Cursor Green Colour 1 Register
#define EPSON13806_LCDINKCURSRED1       (EPSON13806_REG_BASE + 0x007C)  // LCD INK/Cursor Red Color 1 Register
#define EPSON13806_LCDINKCURSFIFO       (EPSON13806_REG_BASE + 0x007E)  // LCD INK/Cursor FIFO Threshold Register

#define EPSON13806_BLTCTRL0             (EPSON13806_REG_BASE + 0x0100)  // BitBlt Control Register 0
#define EPSON13806_BLTCTRL1             (EPSON13806_REG_BASE + 0x0101)  // BitBlt Control Register 1
#define EPSON13806_BLTROP               (EPSON13806_REG_BASE + 0x0102)  // BitBlt ROP Code/Color Expansion Register
#define EPSON13806_BLTOPERATION         (EPSON13806_REG_BASE + 0x0103)  // BitBlt Operation Register
#define EPSON13806_BLTSRCSTART01        (EPSON13806_REG_BASE + 0x0104)  // BitBlt Source Start Address Register 0 and 1 (16 bits)
#define EPSON13806_BLTSRCSTART0         (EPSON13806_REG_BASE + 0x0104)  // BitBlt Source Start Address Register 0
#define EPSON13806_BLTSRCSTART1         (EPSON13806_REG_BASE + 0x0105)  // BitBlt Source Start Address Register 1
#define EPSON13806_BLTSRCSTART2         (EPSON13806_REG_BASE + 0x0106)  // BitBlt Source Start Address Register 2
#define EPSON13806_BLTDSTSTART01        (EPSON13806_REG_BASE + 0x0108)  // BitBlt Destination Start Address Register 0 and 1 (16 bits)
#define EPSON13806_BLTDSTSTART0         (EPSON13806_REG_BASE + 0x0108)  // BitBlt Destination Start Address Register 0
#define EPSON13806_BLTDSTSTART1         (EPSON13806_REG_BASE + 0x0109)  // BitBlt Destination Start Address Register 1
#define EPSON13806_BLTDSTSTART2         (EPSON13806_REG_BASE + 0x010A)  // BitBlt Destination Start Address Register 2
#define EPSON13806_BLTSTRIDE            (EPSON13806_REG_BASE + 0x010C)  // BitBlt Memory Address Offset Register (16 bits)
#define EPSON13806_BLTSTRIDE0           (EPSON13806_REG_BASE + 0x010C)  // BitBlt Memory Address Offset Register 0
#define EPSON13806_BLTSTRIDE1           (EPSON13806_REG_BASE + 0x010D)  // BitBlt Memory Address Offset Register 1
#define EPSON13806_BLTWIDTH             (EPSON13806_REG_BASE + 0x0110)  // BitBlt Width Register (16 bits)
#define EPSON13806_BLTWIDTH0            (EPSON13806_REG_BASE + 0x0110)  // BitBlt Width Register 0
#define EPSON13806_BLTWIDTH1            (EPSON13806_REG_BASE + 0x0111)  // BitBlt Width Register 1
#define EPSON13806_BLTHEIGHT            (EPSON13806_REG_BASE + 0x0112)  // BitBlt Height Register (16 bits)
#define EPSON13806_BLTHEIGHT0           (EPSON13806_REG_BASE + 0x0112)  // BitBlt Height Register 0
#define EPSON13806_BLTHEIGHT1           (EPSON13806_REG_BASE + 0x0113)  // BitBlt Height Register 1
#define EPSON13806_BLTBGCOLOR           (EPSON13806_REG_BASE + 0x0114)  // BitBlt Background Color Register (16 bits)
#define EPSON13806_BLTBGCOLOR0          (EPSON13806_REG_BASE + 0x0114)  // BitBlt Background Color Register 0
#define EPSON13806_BLTBGCOLOR1          (EPSON13806_REG_BASE + 0x0115)  // BitBlt Background Color Register 1
#define EPSON13806_BLTFGCOLOR           (EPSON13806_REG_BASE + 0x0118)  // BitBlt Foreground Color Register (16 bits)
#define EPSON13806_BLTFGCOLOR0          (EPSON13806_REG_BASE + 0x0118)  // BitBlt Foreground Color Register 0
#define EPSON13806_BLTFGCOLOR1          (EPSON13806_REG_BASE + 0x0119)  // BitBlt Foreground Color Register 0

#define EPSON13806_LUTMODE              (EPSON13806_REG_BASE + 0x01E0)  // Look-Up Table Mode Register
#define EPSON13806_LUTADDR              (EPSON13806_REG_BASE + 0x01E2)  // Look-Up Table Address Register
#define EPSON13806_LUTDATA              (EPSON13806_REG_BASE + 0x01E4)  // Look-Up Table Data Register
#define EPSON13806_PWRSAVECFG           (EPSON13806_REG_BASE + 0x01F0)  // Power Save Configuration Register
#define EPSON13806_PWRSAVESTATUS        (EPSON13806_REG_BASE + 0x01F1)  // Power Save Status Register
#define EPSON13806_CPUMEMWATCHDOG       (EPSON13806_REG_BASE + 0x01F4)  // CPU-to-Memory Access Watchdog Timer Register
#define EPSON13806_DISPMODE             (EPSON13806_REG_BASE + 0x01FC)  // Display Mode Register

#define EPSON13806_MEDIALCMD            (EPSON13806_REG_BASE + 0x1000)  // MediaPlug LCMD Register
#define EPSON13806_MEDIARESERVEDLCMD    (EPSON13806_REG_BASE + 0x1002)  // MediaPlug Reserved LCMD Register
#define EPSON13806_MEDIACMD             (EPSON13806_REG_BASE + 0x1004)  // MediaPlug CMD Register
#define EPSON13806_MEDIARESERVEDCMD     (EPSON13806_REG_BASE + 0x1006)  // MediaPlug Reserved CMD Register
#define EPSON13806_MEDIADATA            (EPSON13806_REG_BASE + 0x1008)  // MediaPlug Data Registers (base)

#define EPSON13806_BITBLTDATA           (EPSON13806_REG_BASE + 0x100000)    // BitBLT Data Registers (base)

// BLTCTRL0 register defines
#define EPSON13806_BLTCTRL0_ACTIVE      (1<<7)  // Read: 1=busy, 0=idle / Write: 1=start, 0=no change

// BLTOPERATION register defines
#define EPSON13806_BLTOPERATION_WRITEROP            (0x00)  // Write BitBLT with ROP
#define EPSON13806_BLTOPERATION_READ                (0x01)  // Read BitBLT
#define EPSON13806_BLTOPERATION_MOVEPOSROP          (0x02)  // Move BitBLT in positive direction with ROP
#define EPSON13806_BLTOPERATION_MOVENEGROP          (0x03)  // Move BitBLT in negative direction with ROP
#define EPSON13806_BLTOPERATION_TRANSWRITE          (0x04)  // Transparent Write BitBLT
#define EPSON13806_BLTOPERATION_TRANSMOVEPOS        (0x05)  // Transparent Move BitBLT in positive direction
#define EPSON13806_BLTOPERATION_PATFILLROP          (0x06)  // Pattern fill with ROP
#define EPSON13806_BLTOPERATION_PATFILLTRANS        (0x07)  // Pattern fill with transparency
#define EPSON13806_BLTOPERATION_COLOREXP            (0x08)  // Color expansion
#define EPSON13806_BLTOPERATION_COLOREXPTRANS       (0x09)  // Color expansion with transparency
#define EPSON13806_BLTOPERATION_MOVECOLOREXP        (0x0A)  // Move BitBLT with color expansion
#define EPSON13806_BLTOPERATION_MOVECOLOREXPTRANS   (0x0B)  // Move BitBLT with color expansion and transparency
#define EPSON13806_BLTOPERATION_SOLIDFILL           (0x0C)  // Solid fill

//////////////////////////////////////////////////////////////////////////////////////////
// Epson register access macros
//////////////////////////////////////////////////////////////////////////////////////////
#define EPSON13806_REG(address)   *(VOL8 *)(address)
#define EPSON13806_REG16(address) *(VOL16 *)(address)


#endif  // EPSON13806
