/*
 * Copyright © 2003 Anders Carlsson
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
 * ANDERS CARLSSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ANDERS CARLSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "r128.h"
#include "kaa.h"

CARD8 r128SolidRop[16] = {
    /* GXclear      */      0x00,         /* 0 */
    /* GXand        */      0xa0,         /* src AND dst */
    /* GXandReverse */      0x50,         /* src AND NOT dst */
    /* GXcopy       */      0xf0,         /* src */
    /* GXandInverted*/      0x0a,         /* NOT src AND dst */
    /* GXnoop       */      0xaa,         /* dst */
    /* GXxor        */      0x5a,         /* src XOR dst */
    /* GXor         */      0xfa,         /* src OR dst */
    /* GXnor        */      0x05,         /* NOT src AND NOT dst */
    /* GXequiv      */      0xa5,         /* NOT src XOR dst */
    /* GXinvert     */      0x55,         /* NOT dst */
    /* GXorReverse  */      0xf5,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x0f,         /* NOT src */
    /* GXorInverted */      0xaf,         /* NOT src OR dst */
    /* GXnand       */      0x5f,         /* NOT src OR NOT dst */
    /* GXset        */      0xff,         /* 1 */
};

CARD8 r128BltRop[16] = {
    /* GXclear      */      0x00,         /* 0 */
    /* GXand        */      0x88,         /* src AND dst */
    /* GXandReverse */      0x44,         /* src AND NOT dst */
    /* GXcopy       */      0xcc,         /* src */
    /* GXandInverted*/      0x22,         /* NOT src AND dst */
    /* GXnoop       */      0xaa,         /* dst */
    /* GXxor        */      0x66,         /* src XOR dst */
    /* GXor         */      0xee,         /* src OR dst */
    /* GXnor        */      0x11,         /* NOT src AND NOT dst */
    /* GXequiv      */      0x99,         /* NOT src XOR dst */
    /* GXinvert     */      0x55,         /* NOT dst */
    /* GXorReverse  */      0xdd,         /* src OR NOT dst */
    /* GXcopyInverted*/     0x33,         /* NOT src */
    /* GXorInverted */      0xbb,         /* NOT src OR dst */
    /* GXnand       */      0x77,         /* NOT src OR NOT dst */
    /* GXset        */      0xff,         /* 1 */
};

int copydx, copydy;
int fifo_size;
char *mmio;

static void
r128WaitAvail (int n)
{
    if (fifo_size < n)
    {
	while ((fifo_size = R128_IN32 (mmio, R128_REG_GUI_STAT) & 0xfff) < n)
	    ;
    }
    
    fifo_size -= n;
}

static void
r128WaitIdle (void)
{
    int tries;
    
    r128WaitAvail (64);

    tries = 1000000;
    while (tries--)
    {
	if ((R128_IN32 (mmio, R128_REG_GUI_STAT) & R128_GUI_ACTIVE) == 0)
	    break;
    }

    R128_OUT32 (mmio, R128_REG_PC_NGUI_CTLSTAT,
		R128_IN32 (mmio, R128_REG_PC_NGUI_CTLSTAT | 0xff));

    tries = 1000000;
    while (tries--)
    {
	if ((R128_IN32 (mmio, R128_REG_PC_NGUI_CTLSTAT) & R128_PC_BUSY) != R128_PC_BUSY)
	    break;
    }
    
}

static void
r128WaitMarker (ScreenPtr pScreen, int marker)
{
    KdScreenPriv (pScreen);
    r128CardInfo (pScreenPriv);

    mmio = r128c->reg_base;

    r128WaitIdle ();
}

static Bool
r128Setup (ScreenPtr pScreen, int wait)
{
  KdScreenPriv (pScreen);
  r128ScreenInfo (pScreenPriv);
  r128CardInfo (pScreenPriv);

  fifo_size = 0;

  mmio = r128c->reg_base;
    
  if (!mmio)
      return FALSE;

  r128WaitAvail (2);
  R128_OUT32 (mmio, R128_REG_DEFAULT_OFFSET, 0);
  R128_OUT32 (mmio, R128_REG_DEFAULT_PITCH, r128s->pitch);

  r128WaitAvail (4);
  R128_OUT32 (mmio, R128_AUX_SC_CNTL, 0);
  R128_OUT32 (mmio, R128_DEFAULT_SC_BOTTOM_RIGHT, (R128_DEFAULT_SC_RIGHT_MAX
						| R128_DEFAULT_SC_BOTTOM_MAX));
  R128_OUT32 (mmio, R128_SC_TOP_LEFT, 0);
  R128_OUT32 (mmio, R128_SC_BOTTOM_RIGHT, (R128_DEFAULT_SC_RIGHT_MAX
						| R128_DEFAULT_SC_BOTTOM_MAX));
  r128WaitAvail (wait);
  return TRUE;
}

static Bool
r128PrepareSolid (PixmapPtr pPixmap, int alu, Pixel pm, Pixel fg)
{
    KdScreenPriv (pPixmap->drawable.pScreen);
    r128ScreenInfo (pScreenPriv);

    r128Setup (pPixmap->drawable.pScreen, 4);
    R128_OUT32 (mmio, R128_REG_DP_GUI_MASTER_CNTL, r128s->dp_gui_master_cntl
		| R128_GMC_BRUSH_SOLID_COLOR
		| R128_GMC_SRC_DATATYPE_COLOR
		| (r128SolidRop[alu] << R128_GMC_ROP3_SHIFT));
    R128_OUT32 (mmio, R128_REG_DP_BRUSH_FRGD_CLR, fg);
    R128_OUT32 (mmio, R128_REG_DP_WRITE_MASK, pm);
    R128_OUT32 (mmio, R128_REG_DP_CNTL,
		(R128_DST_X_LEFT_TO_RIGHT | R128_DST_Y_TOP_TO_BOTTOM));
    
    return TRUE;
}

static void
r128Solid (int x1, int y1, int x2, int y2)
{
    r128WaitAvail (2);
    R128_OUT32 (mmio, R128_REG_DST_Y_X, (y1 << 16) | x1);
    R128_OUT32 (mmio, R128_REG_DST_WIDTH_HEIGHT, ((x2 - x1) << 16) | (y2 - y1));
    
}

static void
r128DoneSolid (void)
{
}

static Bool
r128PrepareCopy (PixmapPtr pSrc, PixmapPtr pDst, int dx, int dy, int alu, Pixel pm)
{
    KdScreenPriv (pSrc->drawable.pScreen);
    r128ScreenInfo (pScreenPriv);
    
    copydx = dx;
    copydy = dy;

    r128Setup (pSrc->drawable.pScreen, 3);
    R128_OUT32 (mmio, R128_REG_DP_GUI_MASTER_CNTL, r128s->dp_gui_master_cntl
		| R128_GMC_BRUSH_SOLID_COLOR
		| R128_GMC_SRC_DATATYPE_COLOR
		| (r128BltRop[alu] << R128_GMC_ROP3_SHIFT)
		| R128_DP_SRC_SOURCE_MEMORY);
    R128_OUT32 (mmio, R128_REG_DP_WRITE_MASK, pm);
    R128_OUT32 (mmio, R128_REG_DP_CNTL,
		((dx >= 0 ? R128_DST_X_LEFT_TO_RIGHT : 0)
		 | (dy >= 0 ? R128_DST_Y_TOP_TO_BOTTOM : 0)));
		   

    return TRUE;
}

static void
r128Copy (int srcX, int srcY, int dstX, int dstY, int w, int h)
{
    if (copydx < 0)
    {
	srcX += w - 1;
	dstX += w - 1;
    }

    if (copydy < 0)
    {
	srcY += h - 1;
	dstY += h - 1;
    }

    r128WaitAvail (3);
    R128_OUT32 (mmio, R128_REG_SRC_Y_X, (srcY << 16) | srcX);
    R128_OUT32 (mmio, R128_REG_DST_Y_X, (dstY << 16) | dstX);
    R128_OUT32 (mmio, R128_REG_DST_HEIGHT_WIDTH, (h << 16) | w);
}

static void
r128DoneCopy (void)
{
}


Bool
r128DrawInit (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    r128ScreenInfo (pScreenPriv);

    memset(&r128s->kaa, 0, sizeof(KaaScreenInfoRec));
    r128s->kaa.waitMarker	= r128WaitMarker;
    r128s->kaa.PrepareSolid	= r128PrepareSolid;
    r128s->kaa.Solid		= r128Solid;
    r128s->kaa.DoneSolid	= r128DoneSolid;
    r128s->kaa.PrepareCopy	= r128PrepareCopy;
    r128s->kaa.Copy		= r128Copy;
    r128s->kaa.DoneCopy		= r128DoneCopy;

    if (!kaaDrawInit (pScreen, &r128s->kaa))
	return FALSE;

    return TRUE;
}

void
r128DrawEnable (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    r128ScreenInfo (pScreenPriv);

    r128s->pitch = pScreenPriv->screen->width >> 3;

    switch (pScreenPriv->screen->fb[0].depth) {
    case 8:
	r128s->datatype = 2;
	break;
    case 15:
	r128s->datatype = 3;
	break;	
    case 16:
	r128s->datatype = 4;
	break;
    case 24:
	r128s->datatype = 5;
	break;
    case 32:
	r128s->datatype = 6;
	break;
    default:
	FatalError ("unsupported pixel format");
    }

    r128s->dp_gui_master_cntl = ((r128s->datatype << R128_GMC_DST_DATATYPE_SHIFT)
				 | R128_GMC_CLR_CMP_CNTL_DIS
				 | R128_GMC_AUX_CLIP_DIS);
    
    kaaMarkSync (pScreen);
}

void
r128DrawDisable (ScreenPtr pScreen)
{
}

void
r128DrawFini (ScreenPtr pScreen)
{
}
