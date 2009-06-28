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
 * epson13806draw.c - Implementation of hardware accelerated functions for epson S1D13806
 *               Graphic controller.
 *
 * History:
 * 28-Jan-04  C.Stylianou       PRJ NBL: Created from chipsdraw.c
 *
 */

#include    "epson13806.h"
#include    "epson13806draw.h"
#include    "epson13806reg.h"

#include    "kaa.h"

#include    "gcstruct.h"
#include    "scrnintstr.h"
#include    "pixmapstr.h"
#include    "regionstr.h"
#include    "mistruct.h"
#include    "dixfontstr.h"
#include    "fb.h"
#include    "migc.h"
#include    "miline.h"


// Functionality of BitBLT ROP register for Epson S1D13806 Graphics controller
CARD8 epson13806Rop[16] = {
    /* GXclear      */      0x00,         /* 0 */
    /* GXand        */      0x08,         /* src AND dst */
    /* GXandReverse */      0x04,         /* src AND NOT dst */
    /* GXcopy       */      0x0C,         /* src */
    /* GXandInverted*/      0x02,         /* NOT src AND dst */
    /* GXnoop       */      0x0A,         /* dst */
    /* GXxor        */      0x06,         /* src XOR dst */
    /* GXor         */      0x0E,         /* src OR dst */
    /* GXnor        */      0x01,         /* NOT src AND NOT dst */
    /* GXequiv      */      0x09,         /* NOT src XOR dst */
    /* GXinvert     */      0x05,         /* NOT dst */
    /* GXorReverse  */      0x0D,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x03,         /* NOT src */
    /* GXorInverted */      0x0B,         /* NOT src OR dst */
    /* GXnand       */      0x07,         /* NOT src OR NOT dst */
    /* GXset        */      0x0F,         /* 1 */
};



#undef __DEBUG_EPSON__
#undef __DEBUG_EPSON_FBSET__
#undef __DEBUG_EPSON_SOLID__
#undef __DEBUG_EPSON_COPY__


#ifdef __DEBUG_EPSON__
    #define EPSON_DEBUG(a) a
#else
    #define EPSON_DEBUG(a)
#endif
   
#ifdef __DEBUG_EPSON_FBSET__
    #define EPSON_DEBUG_FBSET(a) a
#else
    #define EPSON_DEBUG_FBSET(a)
#endif

#ifdef __DEBUG_EPSON_SOLID__
    #define EPSON_DEBUG_SOLID(a) a
#else
    #define EPSON_DEBUG_SOLID(a)
#endif

#ifdef __DEBUG_EPSON_COPY__
    #define EPSON_DEBUG_COPY(a) a
#else
    #define EPSON_DEBUG_COPY(a)
#endif


static unsigned int    byteStride;     // Distance between lines in the frame buffer (in bytes)
static unsigned int    bytesPerPixel;
static unsigned int    pixelStride;

static unsigned char *regbase;

/*
 * epsonSet
 *
 * Description:    Sets Epson variables
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonSet (ScreenPtr pScreen)
{
    EPSON_DEBUG_FBSET (fprintf(stderr,"+epsonSet\n"));

    KdScreenPriv(pScreen);
   
    byteStride = pScreenPriv->screen->fb[0].byteStride;
    bytesPerPixel = pScreenPriv->screen->fb[0].bitsPerPixel >> 3;
    pixelStride = pScreenPriv->screen->fb[0].pixelStride;

    EPSON_DEBUG_FBSET (fprintf(stderr,"byteStride:     [%x]\n", pScreenPriv->screen->fb[0].byteStride));
    EPSON_DEBUG_FBSET (fprintf(stderr,"bytesPerPixel:  [%x]\n", pScreenPriv->screen->fb[0].bitsPerPixel >> 3));
    EPSON_DEBUG_FBSET (fprintf(stderr,"pixelStride:    [%x]\n", pScreenPriv->screen->fb[0].pixelStride));

    EPSON_DEBUG_FBSET (fprintf(stderr,"-epsonSet\n"));
}


/*
 * epsonBg
 *
 * Description:    Sets background colour
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonBg (Pixel bg)
{
   EPSON13806_REG16(EPSON13806_BLTBGCOLOR) = bg;
}


/*
 * epsonFg
 *
 * Description:    Sets foreground colour
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonFg (Pixel fg)
{    
    EPSON13806_REG16(EPSON13806_BLTFGCOLOR) = fg;
}


/*
 * epsonWaitForHwBltDone
 *
 * Description:    Wait for previous blt to be done before programming any blt registers
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonWaitForHwBltDone (void)
{
    while (EPSON13806_REG (EPSON13806_BLTCTRL0) & EPSON13806_BLTCTRL0_ACTIVE) {}
}


/*
 * epsonDrawSync
 *
 * Description:    Sync hardware acceleration
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonWaitMarker (ScreenPtr pScreen, int marker)
{
    EPSON_DEBUG (fprintf(stderr,"+epsonDrawSync\n"));

    epsonWaitForHwBltDone ();

    EPSON_DEBUG (fprintf(stderr,"-epsonDrawSync\n"));
}


/*
 * epsonPrepareSolid
 *
 * Description:    Prepare Solid Fill i.e, can it be accelerated
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static Bool
epsonPrepareSolid (PixmapPtr    pPixmap,
           int        alu,
           Pixel    pm,
           Pixel    fg)
{
    EPSON_DEBUG_SOLID (fprintf(stderr,"+epsonPrepareSolid\n"));
    
    FbBits  depthMask;

    depthMask = FbFullMask(pPixmap->drawable.depth);
    if ((pm & depthMask) != depthMask)
        return FALSE;
	
	epsonSet (pPixmap->drawable.pScreen);
    fg &= 0xffff;
    epsonFg (fg);
    epsonBg (fg);
	
	epsonWaitForHwBltDone ();
	
	EPSON_DEBUG_SOLID (fprintf(stderr,"Solid.alu [0x%x], [%d]\n", alu ,epson13806Rop[alu]));
	EPSON13806_REG(EPSON13806_BLTROP) = epson13806Rop[alu];
	
	if (epson13806Rop[alu] == GXnoop)
	{
		EPSON13806_REG(EPSON13806_BLTOPERATION) = EPSON13806_BLTOPERATION_PATFILLROP;
	}
	else
	{
		EPSON13806_REG(EPSON13806_BLTOPERATION) = EPSON13806_BLTOPERATION_SOLIDFILL;
	}
   
   
    EPSON_DEBUG_SOLID (fprintf(stderr,"-epsonPrepareSolid\n"));
    return TRUE;
    
}


/*
 * epsonSolid
 *
 * Description:    Executes Solid Fill
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonSolid (int x1, int y1, int x2, int y2)
{
    
    EPSON_DEBUG_SOLID (fprintf(stderr,"+epsonSolid\n"));
    
    CARD32  dst_addr;
    int width, height;
    
    EPSON_DEBUG_SOLID (fprintf(stderr,"Solid X1 [%d] Y1 [%d] X2 [%d] Y2 [%d]\n", x1, y1, x2, y2));
    
    dst_addr = y1 * byteStride + x1 * bytesPerPixel;
    width = ((x2 - x1)-1);
    height = ((y2 - y1)-1);
    
    // program dst address
    EPSON13806_REG16(EPSON13806_BLTDSTSTART01) = dst_addr;
    EPSON13806_REG(EPSON13806_BLTDSTSTART2) = dst_addr >> 16;
    
    // program width and height of blit
    EPSON13806_REG16(EPSON13806_BLTWIDTH) = width;
    EPSON13806_REG16(EPSON13806_BLTHEIGHT) = height;
    
    EPSON13806_REG(EPSON13806_BLTCTRL0) = EPSON13806_BLTCTRL0_ACTIVE;

    // Wait for operation to complete
    while (EPSON13806_REG(EPSON13806_BLTCTRL0) & EPSON13806_BLTCTRL0_ACTIVE) {}
        
    EPSON_DEBUG_SOLID (fprintf(stderr,"-epsonSolid\n"));
}


/*
 * epsonDoneSolid
 *
 * Description:    Done Solid
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonDoneSolid (void)
{
    EPSON_DEBUG_SOLID (fprintf(stderr,"+epsonDoneSolid\n"));
    
    // Read from BitBLT data offset 0 to shut it down
    //(void)EPSON13806_REG(EPSON13806_BITBLTDATA);

    EPSON_DEBUG_SOLID (fprintf(stderr,"-epsonDoneSolid\n"));
    
}


/*
 * epsonPrepareCopy
 *
 * Description:    Prepares BitBLT, i.e, can it be accelerated
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static Bool
epsonPrepareCopy (PixmapPtr    pSrcPixmap,
          PixmapPtr    pDstPixmap,
          int        dx,
          int        dy,
          int        alu,
          Pixel        pm)
{
    EPSON_DEBUG_COPY (fprintf(stderr,"+epsonPrepareCopy dx [0x%x] dy [0x%x]\n", dx, dy));
    
    FbBits  depthMask;

    depthMask = FbFullMask(pDstPixmap->drawable.depth);
    
    if ((pm & depthMask) != depthMask)
        return FALSE;
    
    epsonSet (pDstPixmap->drawable.pScreen);
	epsonWaitForHwBltDone ();
    EPSON13806_REG(EPSON13806_BLTROP) = epson13806Rop[alu];
            
    EPSON_DEBUG_COPY (fprintf(stderr,"-epsonPrepareCopy\n"));
        
    return TRUE;
}


/*
 * epsonCopy
 *
 * Description:    Executes BitBLT
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonCopy (int srcX,
       int srcY,
       int dstX,
       int dstY,
       int width,
       int height)
{
    EPSON_DEBUG_COPY (fprintf(stderr,"+epsonCopy\n"));
    int    src_addr, dst_addr;
    int neg_dir = FALSE;
    
    if (!width || !height)
        return;
    
    src_addr = srcX * bytesPerPixel + srcY * byteStride;
    dst_addr = dstX * bytesPerPixel + dstY * byteStride;
    
    /*
     * See if regions overlap and dest region is beyond source region.
     * If so, we need to do a move BLT in negative direction. Only applies
     * if the BLT is not transparent.
     */
    
    if ((srcX + width  > dstX) && (srcX < dstX + width) &&
        (srcY + height > dstY) && (srcY < dstY + height) &&
        (dst_addr > src_addr)) 
    {
        neg_dir = TRUE;
            
        // negative direction : get the coords of lower right corner
        src_addr += byteStride * (height-1) + bytesPerPixel * (width-1);
        dst_addr += byteStride * (height-1) + bytesPerPixel * (width-1);
    }
    
    // program BLIT memory offset
    EPSON13806_REG16(EPSON13806_BLTSTRIDE) = byteStride/2;

    // program src and dst addresses
    EPSON13806_REG16(EPSON13806_BLTSRCSTART01) = src_addr;
    EPSON13806_REG(EPSON13806_BLTSRCSTART2) = src_addr >> 16;
    EPSON13806_REG16(EPSON13806_BLTDSTSTART01) = dst_addr;
    EPSON13806_REG(EPSON13806_BLTDSTSTART2) = dst_addr >> 16;

    // program width and height of blit
    EPSON13806_REG16(EPSON13806_BLTWIDTH) = width-1;
    EPSON13806_REG16(EPSON13806_BLTHEIGHT) = height-1;
    
    // select pos/neg move BLIT
    EPSON13806_REG(EPSON13806_BLTOPERATION) = neg_dir ? 
                EPSON13806_BLTOPERATION_MOVENEGROP : EPSON13806_BLTOPERATION_MOVEPOSROP;
    
    EPSON13806_REG(EPSON13806_BLTCTRL0) = EPSON13806_BLTCTRL0_ACTIVE;

    // Wait for operation to complete
    while (EPSON13806_REG(EPSON13806_BLTCTRL0) & EPSON13806_BLTCTRL0_ACTIVE) {}

    EPSON_DEBUG_COPY (fprintf(stderr,"-epsonCopy\n"));
}


/*
 * epsonDoneCopy
 *
 * Description:    Done Copy
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

static void
epsonDoneCopy (void)
{
    EPSON_DEBUG_COPY (fprintf(stderr,"+epsonDoneCopy\n"));
    
    // Read from BitBLT data offset 0 to shut it down
    //(void)EPSON13806_REG(EPSON13806_BITBLTDATA);

    EPSON_DEBUG_COPY (fprintf(stderr,"-epsonDoneCopy\n"));
}


/*
 * epsonDrawInit
 *
 * Description:    Configure the Epson S1D13806 for a 800x600 TFT colour display
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

Bool
epsonDrawInit (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KdScreenInfo *screen = pScreenPriv->screen;
    EpsonScrPriv *epsons = screen->driver;

   EPSON_DEBUG (fprintf(stderr,"+epsonDrawInit\n"));
   
   epsonSet(pScreen);

#if 0
    EPSON13806_REG(EPSON13806_MISC) = 0x00;
    EPSON13806_REG(EPSON13806_DISPMODE) = 0x00;
    EPSON13806_REG16(EPSON13806_GPIOCFG) = 0xffff;
    EPSON13806_REG16(EPSON13806_GPIOCTRL) = 0x0001;
    
    EPSON13806_REG(EPSON13806_MEMCLKCFG) = 0x01;
    EPSON13806_REG(EPSON13806_LCDPCLKCFG) = 0x00;
    EPSON13806_REG(EPSON13806_CRTPCLKCFG) = 0x02;
    EPSON13806_REG(EPSON13806_MPCLKCFG) = 0x02;
    EPSON13806_REG(EPSON13806_CPUMEMWAITSEL) = 0x01;
    EPSON13806_REG(EPSON13806_MEMCFG) = 0x80;
    EPSON13806_REG(EPSON13806_DRAMREFRESH) = 0x03;
    EPSON13806_REG16(EPSON13806_DRAMTIMINGCTRL) = 0x0100;
    
    // 5ms delay for internal LCD SDRAM to initialize
     usleep(5000);
    
    EPSON13806_REG(EPSON13806_PANELTYPE) = 0x25;
    EPSON13806_REG(EPSON13806_MODRATE) = 0x00;
    EPSON13806_REG(EPSON13806_LCDHDP) = 0x63;
    EPSON13806_REG(EPSON13806_LCDHNDP) = 0x1f;
    EPSON13806_REG(EPSON13806_TFTFPLINESTART) = 0x01;
    EPSON13806_REG(EPSON13806_TFTFPLINEPULSE) = 0x0b;
    EPSON13806_REG16(EPSON13806_LCDVDP0) = 0x0257;
    EPSON13806_REG(EPSON13806_LCDVNDP) = 0x1b;
    EPSON13806_REG(EPSON13806_TFTFPFRAMESTART) = 0x0a;
    EPSON13806_REG(EPSON13806_TFTFPFRAMEPULSE) = 0x01;
    EPSON13806_REG(EPSON13806_LCDDISPMODE) = 0x85;
    EPSON13806_REG(EPSON13806_LCDMISC) = 0x00;
    EPSON13806_REG16(EPSON13806_LCDSTART01) = 0x0000;
    EPSON13806_REG(EPSON13806_LCDSTART2) = 0x00;
    EPSON13806_REG16(EPSON13806_LCDSTRIDE) = byteStride>>1;
    EPSON13806_REG(EPSON13806_LCDPIXELPAN) = 0x00;
    EPSON13806_REG(EPSON13806_LCDFIFOHIGH) = 0x00;
    EPSON13806_REG(EPSON13806_LCDFIFOLOW) = 0x00;
#endif


    EPSON13806_REG(EPSON13806_BLTCTRL0) = 0x00;
    EPSON13806_REG(EPSON13806_BLTCTRL1) = 0x01;     // We're using 16 bpp
    EPSON13806_REG16(EPSON13806_BLTSTRIDE) = byteStride>>1; // program BLIT memory offset
    
#if 0
    EPSON13806_REG(EPSON13806_LUTMODE) = 0x00;
    EPSON13806_REG(EPSON13806_LUTADDR) = 0x00;
    EPSON13806_REG(EPSON13806_PWRSAVECFG) = 0x10;
    EPSON13806_REG(EPSON13806_PWRSAVESTATUS) = 0x00;
    EPSON13806_REG(EPSON13806_CPUMEMWATCHDOG) = 0x00;
    EPSON13806_REG(EPSON13806_DISPMODE) = 0x01;
    
    // Enable backlight voltage
    EPSON13806_REG16(EPSON13806_GPIOCTRL) |= 1<<1;
    // 10ms delay after turning on LCD.
    usleep(10000);
#endif

    // Instruct the BitBLT unit to fill the screen with black, i.e clear fb.
    static int addr = 0x00000000;
    EPSON13806_REG16(EPSON13806_BLTDSTSTART01) = addr;
    EPSON13806_REG(EPSON13806_BLTDSTSTART2) = addr >> 16;
    EPSON13806_REG16(EPSON13806_BLTFGCOLOR) = 0x0000;
    EPSON13806_REG(EPSON13806_BLTOPERATION) = EPSON13806_BLTOPERATION_SOLIDFILL; // solid fill blt
    EPSON13806_REG16(EPSON13806_BLTWIDTH) = (0x0320-1);
    EPSON13806_REG16(EPSON13806_BLTHEIGHT) = (0x0258-1);
    EPSON13806_REG(EPSON13806_BLTCTRL0) = EPSON13806_BLTCTRL0_ACTIVE;

#if 0
    // Enable LCD data
    EPSON13806_REG(EPSON13806_LCDDISPMODE) &= ~(1<<7);
    
    // Turn on backlight full
    EPSON13806_REG16(EPSON13806_GPIOCTRL) |= 0x00fc;
#endif
    
    memset(&epsons->kaa, 0, sizeof(KaaScreenInfoRec));
    epsons->kaa.waitMarker	= epsonWaitMarker;
    epsons->kaa.PrepareSolid	= epsonPrepareSolid;
    epsons->kaa.Solid		= epsonSolid;
    epsons->kaa.DoneSolid	= epsonDoneSolid;
    epsons->kaa.PrepareCopy	= epsonPrepareCopy;
    epsons->kaa.Copy		= epsonCopy;
    epsons->kaa.DoneCopy	= epsonDoneCopy;
    epsons->kaa.flags		= KAA_OFFSCREEN_PIXMAPS;

    if (!kaaDrawInit (pScreen, &epsons->kaa))
        return FALSE;
    
    EPSON_DEBUG (fprintf(stderr,"-epsonDrawInit\n"));
    return TRUE;
}


/*
 * epsonDrawEnable
 *
 * Description:    Enables hardware acceleration
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

void
epsonDrawEnable (ScreenPtr pScreen)
{
    EPSON_DEBUG (fprintf(stderr,"+epsonDrawEnable\n"));
    epsonWaitForHwBltDone ();
    kaaMarkSync (pScreen);
    EPSON_DEBUG (fprintf(stderr,"-epsonDrawEnable\n"));
}


/*
 * epsonDrawDisable
 *
 * Description:    Disables hardware acceleration
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

void
epsonDrawDisable (ScreenPtr pScreen)
{
    EPSON_DEBUG (fprintf(stderr,"+epsonDrawDisable\n"));
}


/*
 * epsonDrawFini
 *
 * Description:    Finish hardware acceleration
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

void
epsonDrawFini (ScreenPtr pScreen)
{
    EPSON_DEBUG (fprintf(stderr,"+epsonDrawFini\n"));
}


/*
 * initEpson13806
 *
 * Description:    Maps Epson S1D13806 registers
 *
 * History:
 * 11-Feb-04  C.Stylianou       NBL: Created.
 *
 */

void
initEpson13806(void)
{
    EPSON_DEBUG (fprintf(stderr,"+initEpson\n"));
    
    // Map Epson S1D13806 registers
    regbase = KdMapDevice (EPSON13806_PHYSICAL_REG_ADDR, EPSON13806_GPIO_REGSIZE);
    if (!regbase)
        perror("ERROR: regbase\n");   // Sets up register mappings in header files.

#if 0
    CARD8 rev_code;
    rev_code = EPSON13806_REG (EPSON13806_REVCODE);
    if ((rev_code >> 2) != 0x07) 
        perror("ERROR: EPSON13806 Display Controller NOT FOUND!\n");
#endif
    
    EPSON_DEBUG (fprintf(stderr,"-initEpson\n"));
}
