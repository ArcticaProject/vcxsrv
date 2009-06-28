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
#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "mach64.h"

#include <X11/extensions/Xv.h>
#include "fourcc.h"

#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

static Atom xvBrightness, xvSaturation, xvColorKey;

#define IMAGE_MAX_WIDTH		720
#define IMAGE_MAX_HEIGHT	576

static void
mach64StopVideo(KdScreenInfo *screen, pointer data, Bool exit)
{
    ScreenPtr		pScreen = screen->pScreen;
    KdScreenPriv(pScreen);
    KdCardInfo		*card = pScreenPriv->card;
    Mach64ScreenInfo	*mach64s = (Mach64ScreenInfo *) screen->driver;
    Mach64CardInfo	*mach64c = (Mach64CardInfo *) card->driver;
    Mach64PortPrivPtr	pPortPriv = mach64s->pAdaptor->pPortPrivates[0].ptr;
    Reg			*reg = mach64c->reg;
    MediaReg		*media = mach64c->media_reg;

    REGION_EMPTY(screen->pScreen, &pPortPriv->clip);   

    if (!media)
	return;

    if(pPortPriv->videoOn)
    {
	mach64WaitIdle (reg);
	/* wait for buffer to be displayed */
	while (((media->TRIG_CNTL >> 5) & 1) != pPortPriv->currentBuf)
	    ;
	/* wait for buffer to be finished */
	while (((media->TRIG_CNTL >> 6) & 1) != 0)
	    ;
	mach64WaitAvail (reg, 1);
	media->OVERLAY_SCALE_CNTL = 0;
	pPortPriv->videoOn = FALSE;
	mach64WaitIdle (reg);
    }
    if (pPortPriv->off_screen)
    {
	KdOffscreenFree (pScreen, pPortPriv->off_screen);
	pPortPriv->off_screen = 0;
    }
}

static int
mach64SetPortAttribute(KdScreenInfo *screen,
		       Atom	    attribute,
		       int	    value,
		       pointer	    data)
{
    Mach64ScreenInfo	*mach64s = (Mach64ScreenInfo *) screen->driver;
    Mach64PortPrivPtr	pPortPriv = mach64s->pAdaptor->pPortPrivates[0].ptr;

    if(attribute == xvBrightness) 
    {
	if(value < -1000)
	    value = -1000;
	if (value > 1000)
	    value = 1000;
	pPortPriv->brightness = value;
    }
    else if(attribute == xvSaturation) 
    {
	if (value < -1000)
	    value = -1000;
	if (value > 1000)
	    value = 1000;
	pPortPriv->saturation = value;
    }
    else if(attribute == xvColorKey) 
    {
	if (pPortPriv->colorKey != value)
	{
	    pPortPriv->colorKey = value;
	    REGION_EMPTY(screen->pScreen, &pPortPriv->clip);   
	}
    }
    else 
	return BadMatch;

    return Success;
}

static int
mach64GetPortAttribute(KdScreenInfo *screen, 
		       Atom	    attribute,
		       int	    *value,
		       pointer	    data)
{
    Mach64PortPrivPtr pPortPriv = (Mach64PortPrivPtr)data;

    if(attribute == xvBrightness) 
	*value = pPortPriv->brightness;
    else if(attribute == xvSaturation)
	*value = pPortPriv->saturation;
    else if(attribute == xvColorKey)
	*value = pPortPriv->colorKey;
    else
	return BadMatch;

    return Success;
}

static void
mach64QueryBestSize(KdScreenInfo    *screen,
		    Bool	    motion,
		    short	    vid_w,
		    short	    vid_h,
		    short	    drw_w,
		    short	    drw_h, 
		    unsigned int    *p_w,
		    unsigned int    *p_h,
		    pointer	    data)
{
    *p_w = drw_w;
    *p_h = drw_h; 
}


static void
mach64CopyPackedData(KdScreenInfo   *screen, 
		     unsigned char  *buf,
		     int	    randr,
		     int	    srcPitch,
		     int	    dstPitch,
		     int	    srcW,
		     int	    srcH,
		     int	    top,
		     int	    left,
		     int	    h,
		     int	    w)
{
    Mach64ScreenInfo	*mach64s = (Mach64ScreenInfo *) screen->driver;
    Mach64PortPrivPtr	pPortPriv = mach64s->pAdaptor->pPortPrivates[0].ptr;
    CARD8		*src = buf, *dst;
    int			srcDown = srcPitch, srcRight = 2, srcNext;
    int			p;

    switch (randr & RR_Rotate_All) {
    case RR_Rotate_0:
	src = buf;
	srcDown = srcPitch;
	srcRight = 2;
	break;
    case RR_Rotate_90:
	src = buf + (srcH - 1) * 2;
	srcDown = -2;
	srcRight = srcPitch;
	break;
    case RR_Rotate_180:
	src = buf + srcPitch * (srcH - 1) + (srcW - 1) * 2;
	srcDown = -srcPitch;
	srcRight = -2;
	break;
    case RR_Rotate_270:
	src = buf + srcPitch * (srcW - 1);
	srcDown = 2;
	srcRight = -srcPitch;
	break;
    }

    src = src + top*srcDown + left*srcRight;

    if (pPortPriv->currentBuf == 0)
	dst = (CARD8 *) mach64s->vesa.fb + pPortPriv->YBuf0Offset;
    else
	dst = (CARD8 *) mach64s->vesa.fb + pPortPriv->YBuf1Offset;

    w >>= 1;
    srcRight >>= 1;
    srcNext = srcRight >> 1;
    while(h--) 
    {
	CARD16	*s = (CARD16 *) src;
	CARD32	*d = (CARD32 *) dst;
	p = w;
	while (p--)
	{
	    *d++ = s[0] | (s[srcNext] << 16);
	    s += srcRight;
	}
	src += srcPitch;
	dst += dstPitch;
    }
}

static void
mach64CopyPlanarData(KdScreenInfo   *screen, 
		     unsigned char  *buf,
		     int	    randr,
		     int	    srcPitch,
		     int	    srcPitch2,
		     int	    dstPitch,  /* of chroma */
		     int	    srcW,
		     int	    srcH,
		     int	    height,
		     int	    top,
		     int	    left,
		     int	    h,
		     int	    w,
		     int	    id)
{
    Mach64ScreenInfo	*mach64s = (Mach64ScreenInfo *) screen->driver;
    Mach64PortPrivPtr	pPortPriv = mach64s->pAdaptor->pPortPrivates[0].ptr;
    int			i, j;
    CARD8		*src1, *src2, *src3, *dst1;
    int			srcDown = srcPitch, srcDown2 = srcPitch2;
    int			srcRight = 2, srcRight2 = 1, srcNext = 1;

    /* compute source data pointers */
    src1 = buf;
    src2 = src1 + height * srcPitch;
    src3 = src2 + (height >> 1) * srcPitch2;
    switch (randr & RR_Rotate_All) {
    case RR_Rotate_0:
	srcDown = srcPitch;
	srcDown2 = srcPitch2;
	srcRight = 2;
	srcRight2 = 1;
	srcNext = 1;
	break;
    case RR_Rotate_90:
	src1 = src1 + srcH - 1;
	src2 = src2 + (srcH >> 1) - 1;
	src3 = src3 + (srcH >> 1) - 1;
	srcDown = -1;
	srcDown2 = -1;
	srcRight = srcPitch * 2;
	srcRight2 = srcPitch2;
	srcNext = srcPitch;
	break;
    case RR_Rotate_180:
	src1 = src1 + srcPitch * (srcH - 1) + (srcW - 1);
	src2 = src2 + srcPitch2 * ((srcH >> 1) - 1) + ((srcW >> 1) - 1);
	src3 = src3 + srcPitch2 * ((srcH >> 1) - 1) + ((srcW >> 1) - 1);
	srcDown = -srcPitch;
	srcDown2 = -srcPitch2;
	srcRight = -2;
	srcRight2 = -1;
	srcNext = -1;
	break;
    case RR_Rotate_270:
	src1 = src1 + srcPitch * (srcW - 1);
	src2 = src2 + srcPitch2 * ((srcW >> 1) - 1);
	src3 = src3 + srcPitch2 * ((srcW >> 1) - 1);
	srcDown = 1;
	srcDown2 = 1;
	srcRight = -srcPitch * 2;
	srcRight2 = -srcPitch2;
	srcNext = -srcPitch;
	break;
    }
    
    /* adjust for origin */
    src1 += top * srcDown + left * srcNext;
    src2 += (top >> 1) * srcDown2 + (left >> 1) * srcRight2;
    src3 += (top >> 1) * srcDown2 + (left >> 1) * srcRight2;
    
    if (id == FOURCC_I420)
    {
	CARD8	*srct = src2;
	src2 = src3;
	src3 = srct;
    }
    
    if (pPortPriv->currentBuf == 0)
	dst1 = (CARD8 *) mach64s->vesa.fb + pPortPriv->YBuf0Offset;
    else
	dst1 = (CARD8 *) mach64s->vesa.fb + pPortPriv->YBuf1Offset;

    w >>= 1;
    for (j = 0; j < h; j++) 
    {
	CARD32	*dst = (CARD32 *) dst1;
	CARD8	*s1l = src1;
	CARD8	*s1r = src1 + srcNext;
	CARD8	*s2 = src2;
	CARD8	*s3 = src3;

	for (i = 0; i < w; i++)
	{
	    *dst++ = *s1l | (*s1r << 16) | (*s3 << 8) | (*s2 << 24);
	    s1l += srcRight;
	    s1r += srcRight;
	    s2 += srcRight2;
	    s3 += srcRight2;
	}
	src1 += srcDown;
	dst1 += dstPitch;
	if (j & 1)
	{
	    src2 += srcDown2;
	    src3 += srcDown2;
	}
    }
}


/* Mach64ClipVideo -  

   Takes the dst box in standard X BoxRec form (top and left
   edges inclusive, bottom and right exclusive).  The new dst
   box is returned.  The source boundaries are given (x1, y1 
   inclusive, x2, y2 exclusive) and returned are the new source 
   boundaries in 16.16 fixed point. 
*/

static void
Mach64ClipVideo(BoxPtr dst, 
		INT32 *x1, 
		INT32 *x2, 
		INT32 *y1, 
		INT32 *y2,
		BoxPtr extents,            /* extents of the clip region */
		INT32 width, 
		INT32 height)
{
    INT32 vscale, hscale, delta;
    int diff;

    hscale = ((*x2 - *x1) << 16) / (dst->x2 - dst->x1);
    vscale = ((*y2 - *y1) << 16) / (dst->y2 - dst->y1);

    *x1 <<= 16; *x2 <<= 16;
    *y1 <<= 16; *y2 <<= 16;

    diff = extents->x1 - dst->x1;
    if(diff > 0) {
	dst->x1 = extents->x1;
	*x1 += diff * hscale;     
    }
    diff = dst->x2 - extents->x2;
    if(diff > 0) {
	dst->x2 = extents->x2;
	*x2 -= diff * hscale;     
    }
    diff = extents->y1 - dst->y1;
    if(diff > 0) {
	dst->y1 = extents->y1;
	*y1 += diff * vscale;     
    }
    diff = dst->y2 - extents->y2;
    if(diff > 0) {
	dst->y2 = extents->y2;
	*y2 -= diff * vscale;     
    }

    if(*x1 < 0) {
	diff =  (- *x1 + hscale - 1)/ hscale;
	dst->x1 += diff;
	*x1 += diff * hscale;
    }
    delta = *x2 - (width << 16);
    if(delta > 0) {
	diff = (delta + hscale - 1)/ hscale;
	dst->x2 -= diff;
	*x2 -= diff * hscale;
    }
    if(*y1 < 0) {
	diff =  (- *y1 + vscale - 1)/ vscale;
	dst->y1 += diff;
	*y1 += diff * vscale;
    }
    delta = *y2 - (height << 16);
    if(delta > 0) {
	diff = (delta + vscale - 1)/ vscale;
	dst->y2 -= diff;
	*y2 -= diff * vscale;
    }
} 

static void
mach64DisplayVideo(KdScreenInfo *screen,
		   int		id,
		   int		dstPitch,  /* of chroma for 4:2:0 */
		   int		x1,
		   int		y1,
		   int		x2,
		   int		y2,
		   int		dst_x1,
		   int		dst_y1,
		   int		dst_x2,
		   int		dst_y2,
		   short	src_w,
		   short	src_h,
		   short	drw_w, 
		   short	drw_h)
{
    ScreenPtr		pScreen = screen->pScreen;
    KdScreenPriv(pScreen);
    KdCardInfo		*card = pScreenPriv->card;
    Mach64ScreenInfo	*mach64s = (Mach64ScreenInfo *) screen->driver;
    Mach64CardInfo	*mach64c = (Mach64CardInfo *) card->driver;
    Mach64PortPrivPtr	pPortPriv = mach64s->pAdaptor->pPortPrivates[0].ptr;
    Reg			*reg = mach64c->reg;
    MediaReg		*media = mach64c->media_reg;
    int			HORZ_INC, VERT_INC;
    CARD32		SCALER_IN;
    int			bright;
    int			sat;

    if (id == FOURCC_UYVY)
	SCALER_IN = SCALER_IN_YVYU422;
    else
	SCALER_IN = SCALER_IN_VYUY422;

    mach64WaitAvail (reg, 4);
    
    media->VIDEO_FORMAT = SCALER_IN | VIDEO_IN_VYUY422;

    /* color key */
    media->OVERLAY_GRAPHICS_KEY_MSK = (1 << screen->fb[0].depth) - 1;
    media->OVERLAY_GRAPHICS_KEY_CLR = pPortPriv->colorKey;
    /* set key control to obey only graphics color key */
    media->OVERLAY_KEY_CNTL = 0x50;
    
    mach64WaitAvail (reg, 9);
    media->CAPTURE_DEBUG = 0;
    /* no exclusive video region */
    media->OVERLAY_EXCLUSIVE_HORZ = 0;
    media->OVERLAY_EXCLUSIVE_VERT = 0;
    /* scaling coefficients */
    media->SCALER_H_COEFF0 = 0x00002000;
    media->SCALER_H_COEFF1 = 0x0D06200D;
    media->SCALER_H_COEFF2 = 0x0D0A1C0D;
    media->SCALER_H_COEFF3 = 0x0C0E1A0C;
    media->SCALER_H_COEFF4 = 0x0C14140C;
    media->SCALER_TEST = 0;

    mach64WaitAvail (reg, 2);
    media->OVERLAY_SCALE_CNTL = (SCALE_PIX_EXPAND |
				 SCALE_GAMMA_BRIGHT |
				 SCALE_BANDWIDTH |
				 SCALE_OVERLAY_EN |
				 SCALE_EN);

    bright = (pPortPriv->brightness * 64 / 1000);
    if (bright < -0x40)
	bright = -0x40;
    if (bright > 0x3f)
	bright = 0x3f;
    bright = bright & 0x7f;
    sat = ((pPortPriv->saturation * 31 + 31000) / 2000);
    if (sat > 0x1f)
	sat = 0x1f;
    if (sat < 0)
	sat = 0;
    
    media->SCALER_COLOUR_CNTL = ((bright << 0) |	/* BRIGHTNESS */
				 (sat << 8) |	/* SATURATION_U */
				 (sat << 16) |	/* SATURATION_V */
				 (0 << 21) |	/* SCALER_VERT_ADJ_UV */
				 (0 << 28));	/* SCALER_HORZ_ADJ_UV */

    VERT_INC = (src_h << 12) / drw_h;
    HORZ_INC = (src_w << 12) / drw_w;

    mach64WaitAvail (reg, 13);

    /* lock registers to prevent non-atomic update */
    media->OVERLAY_Y_X_START = 0x80000000 | MACH64_YX (dst_x1, dst_y1);
    /* ending screen coordinate */
    media->OVERLAY_Y_X_END = 0x80000000 | MACH64_YX (dst_x2, dst_y2);
    
    media->OVERLAY_SCALE_INC = MACH64_YX(HORZ_INC, VERT_INC);

    media->SCALER_BUF0_OFFSET = pPortPriv->YBuf0Offset;
    media->SCALER_BUF1_OFFSET = pPortPriv->YBuf1Offset;
    
    media->SCALER_BUF0_OFFSET_U = pPortPriv->YBuf0Offset;
    media->SCALER_BUF1_OFFSET_U = pPortPriv->YBuf1Offset;
    
    media->SCALER_BUF0_OFFSET_V = pPortPriv->YBuf0Offset;
    media->SCALER_BUF1_OFFSET_V = pPortPriv->YBuf1Offset;
    
    media->SCALER_BUF_PITCH = dstPitch >> 1;
    media->SCALER_HEIGHT_WIDTH = MACH64_YX(src_w - (x1 >> 16), src_h - (y1 >> 16));

    media->CAPTURE_CONFIG = pPortPriv->currentBuf << 28;

    /* set XY location and unlock */
    media->OVERLAY_Y_X_START = MACH64_YX (dst_x1, dst_y1);
}

static void
mach64VideoSave (ScreenPtr pScreen, KdOffscreenArea *area)
{
    KdScreenPriv(pScreen);
    KdScreenInfo	*screen = pScreenPriv->screen;
    Mach64ScreenInfo	*mach64s = (Mach64ScreenInfo *) screen->driver;
    Mach64PortPrivPtr	pPortPriv = mach64s->pAdaptor->pPortPrivates[0].ptr;

    if (pPortPriv->off_screen == area)
	pPortPriv->off_screen = 0;
}

static int
mach64PutImage(KdScreenInfo	    *screen, 
	       DrawablePtr	    pDraw,
	       short		    src_x,
	       short		    src_y,
	       short		    drw_x,
	       short		    drw_y,
	       short		    src_w,
	       short		    src_h,
	       short		    drw_w,
	       short		    drw_h,
	       int		     id,
	       unsigned char	    *buf,
	       short		    width,
	       short		    height,
	       Bool		    sync,
	       RegionPtr	    clipBoxes,
	       pointer		    data)
{
    KdCardInfo		*card = screen->card;
    Mach64ScreenInfo	*mach64s = (Mach64ScreenInfo *) screen->driver;
    Mach64CardInfo	*mach64c = (Mach64CardInfo *) card->driver;
    Mach64PortPrivPtr	pPortPriv = (Mach64PortPrivPtr)data;
    MediaReg		*media = mach64c->media_reg;
    INT32		x1, x2, y1, y2;
    int			randr = mach64s->vesa.randr;
    int			srcPitch, srcPitch2, dstPitch;
    int			top, left, npixels, nlines, size;
    BoxRec		dstBox;
    int			dst_width = width, dst_height = height;
    int			rot_x1, rot_y1, rot_x2, rot_y2;
    int			dst_x1, dst_y1, dst_x2, dst_y2;
    int			rot_src_w, rot_src_h, rot_drw_w, rot_drw_h;

    /* Clip */
    x1 = src_x;
    x2 = src_x + src_w;
    y1 = src_y;
    y2 = src_y + src_h;

    dstBox.x1 = drw_x;
    dstBox.x2 = drw_x + drw_w;
    dstBox.y1 = drw_y;
    dstBox.y2 = drw_y + drw_h;

    Mach64ClipVideo(&dstBox, &x1, &x2, &y1, &y2, 
		  REGION_EXTENTS(pScreen, clipBoxes), width, height);

    if((x1 >= x2) || (y1 >= y2))
	return Success;

    if (!media)
	return BadAlloc;

    if (randr & (RR_Rotate_0|RR_Rotate_180))
    {
	dst_width = width;
	dst_height = height;
	rot_src_w = src_w;
	rot_src_h = src_h;
	rot_drw_w = drw_w;
	rot_drw_h = drw_h;
    }
    else
    {
	dst_width = height;
	dst_height = width;
	rot_src_w = src_h;
	rot_src_h = src_w;
	rot_drw_w = drw_h;
	rot_drw_h = drw_w;
    }
	
    switch (randr & RR_Rotate_All) {
    case RR_Rotate_0:
    default:
	dst_x1 = dstBox.x1;
	dst_y1 = dstBox.y1;
	dst_x2 = dstBox.x2;
	dst_y2 = dstBox.y2;
	rot_x1 = x1;
	rot_y1 = y1;
	rot_x2 = x2;
	rot_y2 = y2;
	break;
    case RR_Rotate_90:
	dst_x1 = dstBox.y1;
	dst_y1 = screen->height - dstBox.x2;
	dst_x2 = dstBox.y2;
	dst_y2 = screen->height - dstBox.x1;
	
	rot_x1 = y1;
	rot_y1 = (src_w << 16) - x2;
	rot_x2 = y2;
	rot_y2 = (src_w << 16) - x1;
	break;
    case RR_Rotate_180:
	dst_x1 = screen->width - dstBox.x2;
	dst_y1 = screen->height - dstBox.y2;
	dst_x2 = screen->width - dstBox.x1;
	dst_y2 = screen->height - dstBox.y1;
	rot_x1 = (src_w << 16) - x2;
	rot_y1 = (src_h << 16) - y2;
	rot_x2 = (src_w << 16) - x1;
	rot_y2 = (src_h << 16) - y1;
	break;
    case RR_Rotate_270:
	dst_x1 = screen->width - dstBox.y2;
	dst_y1 = dstBox.x1;
	dst_x2 = screen->width - dstBox.y1;
	dst_y2 = dstBox.x2;
	rot_x1 = (src_h << 16) - y2;
	rot_y1 = x1;
	rot_x2 = (src_h << 16) - y1;
	rot_y2 = x2;
	break;
    }

    switch(id) {
    case FOURCC_YV12:
    case FOURCC_I420:
	dstPitch = ((dst_width << 1) + 15) & ~15;
	srcPitch = (width + 3) & ~3;
	srcPitch2 = ((width >> 1) + 3) & ~3;
	size =  dstPitch * (int) dst_height;
	break;
    case FOURCC_UYVY:
    case FOURCC_YUY2:
    default:
	dstPitch = ((dst_width << 1) + 15) & ~15;
	srcPitch = (width << 1);
	srcPitch2 = 0;
	size = dstPitch * (int) dst_height;
	break;
    }  

    if (pPortPriv->off_screen && size != pPortPriv->size)
    {
	KdOffscreenFree (screen->pScreen, pPortPriv->off_screen);
	pPortPriv->off_screen = 0;
    }

    if (!pPortPriv->off_screen)
    {
	pPortPriv->off_screen = KdOffscreenAlloc (screen->pScreen, size * 2, 64,
						  TRUE, mach64VideoSave,
						  pPortPriv);
	if (!pPortPriv->off_screen)
	    return BadAlloc;
    }
    
    pPortPriv->offset = pPortPriv->off_screen->offset;
    pPortPriv->size = size;
    /* fixup pointers */
    
    pPortPriv->YBuf0Offset = pPortPriv->offset;
    pPortPriv->YBuf1Offset = pPortPriv->offset + size;

#if 0
    mach64WaitIdle (reg);

    if (pPortPriv->videoOn)
    {
	/* wait for buffer to be displayed */
	while (((media->TRIG_CNTL >> 5) & 1) != pPortPriv->currentBuf)
	    ;
    }
#endif
    /*
     * Use the other buffer
     */
    pPortPriv->currentBuf = 1 - pPortPriv->currentBuf;
    
    /* copy data */
    top = rot_y1 >> 16;
    left = (rot_x1 >> 16) & ~1;
    npixels = ((((rot_x2 + 0xffff) >> 16) + 1) & ~1) - left;

    switch(id) {
    case FOURCC_YV12:
    case FOURCC_I420:
	top &= ~1;
	nlines = ((((rot_y2 + 0xffff) >> 16) + 1) & ~1) - top;
	mach64CopyPlanarData(screen, buf, randr,
			     srcPitch, srcPitch2, dstPitch,  
			     rot_src_w, rot_src_h, height,
			     top, left, nlines, npixels, id);
	break;
    case FOURCC_UYVY:
    case FOURCC_YUY2:
    default:
	nlines = ((rot_y2 + 0xffff) >> 16) - top;
	mach64CopyPackedData(screen, buf, randr,
			     srcPitch, dstPitch,
			     rot_src_w, rot_src_h,
			     top, left, nlines, 
			     npixels);
	break;
    }

    mach64DisplayVideo(screen, id, dstPitch, 
		       rot_x1, rot_y1, rot_x2, rot_y2, 
		       dst_x1, dst_y1,
		       dst_x2, dst_y2,
		       rot_src_w, rot_src_h, rot_drw_w, rot_drw_h);

    /* update cliplist */
    if (!REGION_EQUAL (screen->pScreen, &pPortPriv->clip, clipBoxes))
    {
	REGION_COPY (screen->pScreen, &pPortPriv->clip, clipBoxes);
	KXVPaintRegion (pDraw, &pPortPriv->clip, pPortPriv->colorKey);
    }

    pPortPriv->videoOn = TRUE;

    return Success;
}

static int
mach64QueryImageAttributes(KdScreenInfo	    *screen, 
			   int		    id,
			   unsigned short   *w,
			   unsigned short   *h,  
			   int		    *pitches,
			   int		    *offsets)
{
    int size, tmp;

    if(*w > IMAGE_MAX_WIDTH) 
	*w = IMAGE_MAX_WIDTH;
    if(*h > IMAGE_MAX_HEIGHT) 
	*h = IMAGE_MAX_HEIGHT;

    *w = (*w + 1) & ~1;
    if(offsets) offsets[0] = 0;

    switch(id) {
    case FOURCC_YV12:
    case FOURCC_I420:
	*h = (*h + 1) & ~1;
	size = (*w + 3) & ~3;
	if(pitches) 
	    pitches[0] = size;
	size *= *h;
	if(offsets) 
	    offsets[1] = size;
	tmp = ((*w >> 1) + 3) & ~3;
	if(pitches) 
	    pitches[1] = pitches[2] = tmp;
	tmp *= (*h >> 1);
	size += tmp;
	if(offsets) 
	    offsets[2] = size;
	size += tmp;
	break;
    case FOURCC_UYVY:
    case FOURCC_YUY2:
    default:
	size = *w << 1;
	if(pitches) 
	    pitches[0] = size;
	size *= *h;
	break;
    }

    return size;
}


/* client libraries expect an encoding */
static KdVideoEncodingRec DummyEncoding[1] =
{
 {
   0,
   "XV_IMAGE",
   IMAGE_MAX_WIDTH, IMAGE_MAX_HEIGHT,
   {1, 1}
 }
};

#define NUM_FORMATS 3

static KdVideoFormatRec Formats[NUM_FORMATS] = 
{
  {15, TrueColor}, {16, TrueColor}, {24, TrueColor}
};

#define NUM_ATTRIBUTES 3

static KdAttributeRec Attributes[NUM_ATTRIBUTES] =
{
   {XvSettable | XvGettable, 0, ~0, "XV_COLORKEY"},
   {XvSettable | XvGettable, -1000, 1000, "XV_BRIGHTNESS"},
   {XvSettable | XvGettable, -1000, 1000, "XV_SATURATION"}
};

#define NUM_IMAGES 4

static KdImageRec Images[NUM_IMAGES] =
{
	XVIMAGE_YUY2,
	XVIMAGE_YV12,
	XVIMAGE_I420,
	XVIMAGE_UYVY
};

static void mach64ResetVideo(KdScreenInfo *screen) 
{
}

static int
mach64ReputImage (KdScreenInfo	    *screen,
		  DrawablePtr	    pDraw,
		  short		    drw_x,
		  short		    drw_y,
		  RegionPtr	    clipBoxes,
		  pointer	    data)
{
    Mach64PortPrivPtr	pPortPriv = (Mach64PortPrivPtr)data;
    BoxPtr		pOldExtents = REGION_EXTENTS (screen->pScreen, &pPortPriv->clip);
    BoxPtr		pNewExtents = REGION_EXTENTS (screen->pScreen, clipBoxes);

    if (pOldExtents->x1 == pNewExtents->x1 &&
	pOldExtents->x2 == pNewExtents->x2 &&
	pOldExtents->y1 == pNewExtents->y1 &&
	pOldExtents->y2 == pNewExtents->y2)
    {
	/* update cliplist */
	if (!REGION_EQUAL (screen->pScreen, &pPortPriv->clip, clipBoxes))
	{
	    REGION_COPY (screen->pScreen, &pPortPriv->clip, clipBoxes);
	    KXVPaintRegion (pDraw, &pPortPriv->clip, pPortPriv->colorKey);
	}
	return Success;
    }
    return BadMatch;
}

static KdVideoAdaptorPtr 
mach64SetupImageVideo(ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    mach64ScreenInfo(pScreenPriv);
    KdScreenInfo	*screen = pScreenPriv->screen;
    KdVideoAdaptorPtr	adapt;
    Mach64PortPrivPtr	pPortPriv;

    if(!(adapt = xcalloc(1, sizeof(KdVideoAdaptorRec) +
			    sizeof(Mach64PortPrivRec) +
			    sizeof(DevUnion))))
	return NULL;

    adapt->type = XvWindowMask | XvInputMask | XvImageMask;
    adapt->flags = VIDEO_OVERLAID_IMAGES | VIDEO_CLIP_TO_VIEWPORT;
    adapt->name = "Mach64 Video Overlay";
    adapt->nEncodings = 1;
    adapt->pEncodings = DummyEncoding;
    adapt->nFormats = NUM_FORMATS;
    adapt->pFormats = Formats;
    adapt->nPorts = 1;
    adapt->pPortPrivates = (DevUnion*)(&adapt[1]);

    pPortPriv = (Mach64PortPrivPtr)(&adapt->pPortPrivates[1]);

    adapt->pPortPrivates[0].ptr = (pointer)(pPortPriv);
    adapt->pAttributes = Attributes;
    adapt->nImages = NUM_IMAGES;
    adapt->nAttributes = NUM_ATTRIBUTES;
    adapt->pImages = Images;
    adapt->PutVideo = NULL;
    adapt->PutStill = NULL;
    adapt->GetVideo = NULL;
    adapt->GetStill = NULL;
    adapt->StopVideo = mach64StopVideo;
    adapt->SetPortAttribute = mach64SetPortAttribute;
    adapt->GetPortAttribute = mach64GetPortAttribute;
    adapt->QueryBestSize = mach64QueryBestSize;
    adapt->PutImage = mach64PutImage;
    adapt->ReputImage = mach64ReputImage;
    adapt->QueryImageAttributes = mach64QueryImageAttributes;

    pPortPriv->colorKey = mach64s->colorKey;
    pPortPriv->videoOn = FALSE;
    pPortPriv->brightness = 0;
    pPortPriv->saturation = 0;
    pPortPriv->currentBuf = 0;
    pPortPriv->off_screen = 0;
    pPortPriv->size = 0;
    pPortPriv->offset = 0;

    /* gotta uninit this someplace */
    REGION_INIT(pScreen, &pPortPriv->clip, NullBox, 0); 

    mach64s->pAdaptor = adapt;

    xvBrightness = MAKE_ATOM("XV_BRIGHTNESS");
    xvSaturation = MAKE_ATOM("XV_SATURATION");
    xvColorKey   = MAKE_ATOM("XV_COLORKEY");

    mach64ResetVideo(screen);

    return adapt;
}

Bool mach64InitVideo(ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KdScreenInfo	*screen = pScreenPriv->screen;
    KdVideoAdaptorPtr	*adaptors, *newAdaptors = NULL;
    KdVideoAdaptorPtr	newAdaptor = NULL;
    int			num_adaptors;
    KdCardInfo		*card = pScreenPriv->card;
    Mach64CardInfo	*mach64c = (Mach64CardInfo *) card->driver;
    Mach64ScreenInfo	*mach64s = (Mach64ScreenInfo *) screen->driver;
    
    mach64s->pAdaptor = NULL;

    if (!mach64c->media_reg)
	return FALSE;

    newAdaptor = mach64SetupImageVideo(pScreen);

    num_adaptors = KdXVListGenericAdaptors(screen, &adaptors);

    if(newAdaptor) 
    {
	if(!num_adaptors) 
	{
	    num_adaptors = 1;
	    adaptors = &newAdaptor;
	}
	else 
	{
	    newAdaptors = xalloc((num_adaptors + 1) * 
				 sizeof(KdVideoAdaptorPtr*));
	    if(newAdaptors) 
	    {
		memcpy(newAdaptors, adaptors, 
		       num_adaptors * sizeof(KdVideoAdaptorPtr));
		newAdaptors[num_adaptors] = newAdaptor;
		adaptors = newAdaptors;
		num_adaptors++;
	    }
	}
    }

    if(num_adaptors)
        KdXVScreenInit(pScreen, adaptors, num_adaptors);

    if(newAdaptors)
        xfree(newAdaptors);
    return TRUE;
}

void
mach64FiniVideo (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    mach64ScreenInfo(pScreenPriv);
    KdVideoAdaptorPtr adapt = mach64s->pAdaptor;

    if (adapt)
    {
	Mach64PortPrivPtr pPortPriv = (Mach64PortPrivPtr)(&adapt->pPortPrivates[1]);
	REGION_UNINIT (pScreen, &pPortPriv->clip);
	xfree (adapt);
    }
}
