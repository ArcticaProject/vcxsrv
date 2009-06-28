/*
 * Copyright © 2001 Keith Packard
 *
 * Partly based on code that is Copyright © The XFree86 Project Inc.
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
#include "kdrive.h"
#include "kaa.h"
#include "dixfontstr.h"

#define DEBUG_MIGRATE 0
#define DEBUG_PIXMAP 0
#if DEBUG_MIGRATE
#define DBG_MIGRATE(a) ErrorF a
#else
#define DBG_MIGRATE(a)
#endif
#if DEBUG_PIXMAP
#define DBG_PIXMAP(a) ErrorF a
#else
#define DBG_PIXMAP(a)
#endif
 
DevPrivateKey kaaScreenPrivateKey = &kaaScreenPrivateKey;
DevPrivateKey kaaPixmapPrivateKey = &kaaPixmapPrivateKey;

#define KAA_PIXMAP_SCORE_MOVE_IN    10
#define KAA_PIXMAP_SCORE_MAX	    20
#define KAA_PIXMAP_SCORE_MOVE_OUT   -10
#define KAA_PIXMAP_SCORE_MIN	    -20
#define KAA_PIXMAP_SCORE_PINNED	    1000
#define KAA_PIXMAP_SCORE_INIT	    1001

void
kaaDrawableDirty (DrawablePtr pDrawable)
{
    PixmapPtr pPixmap;
    KaaPixmapPrivPtr pKaaPixmap;

    if (pDrawable->type == DRAWABLE_WINDOW)
	pPixmap = (*pDrawable->pScreen->GetWindowPixmap)((WindowPtr) pDrawable);
    else
	pPixmap = (PixmapPtr)pDrawable;

    pKaaPixmap = KaaGetPixmapPriv(pPixmap);
    if (pKaaPixmap != NULL)
	pKaaPixmap->dirty = TRUE;
}

static void
kaaPixmapSave (ScreenPtr pScreen, KdOffscreenArea *area)
{
    PixmapPtr pPixmap = area->privData;
    KaaPixmapPriv(pPixmap);
    int dst_pitch, src_pitch, bytes;
    unsigned char *dst, *src;
    int i; 
    
    DBG_MIGRATE (("Save 0x%08x (0x%x) (%dx%d)\n",
		  pPixmap->drawable.id,
		  KaaGetPixmapPriv(pPixmap)->area ? 
		  KaaGetPixmapPriv(pPixmap)->area->offset : -1,
		  pPixmap->drawable.width,
		  pPixmap->drawable.height));
		  
    src_pitch = pPixmap->devKind;
    dst_pitch = pKaaPixmap->devKind;

    src = pPixmap->devPrivate.ptr;
    dst = pKaaPixmap->devPrivate.ptr;
    
    pPixmap->devKind = dst_pitch;
    pPixmap->devPrivate.ptr = dst;
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pKaaPixmap->area = NULL;

#if 0
    if (!pKaaPixmap->dirty)
	return;
#endif

    kaaWaitSync (pPixmap->drawable.pScreen);

    bytes = src_pitch < dst_pitch ? src_pitch : dst_pitch;

    i = pPixmap->drawable.height;
    while (i--) {
	memcpy (dst, src, bytes);
	dst += dst_pitch;
	src += src_pitch;
    }
}

static int
kaaLog2(int val)
{
    int bits;

    if (!val)
	return 0;
    for (bits = 0; val != 0; bits++)
	val >>= 1;
    return bits - 1;
}

static Bool
kaaPixmapAllocArea (PixmapPtr pPixmap)
{
    ScreenPtr	pScreen = pPixmap->drawable.pScreen;
    KaaScreenPriv (pScreen);
    KaaPixmapPriv (pPixmap);
    KdScreenPriv  (pScreen);
    int		bpp = pPixmap->drawable.bitsPerPixel;
    CARD16	h = pPixmap->drawable.height;
    CARD16	w = pPixmap->drawable.width;
    int		pitch;

    if (pKaaScr->info->flags & KAA_OFFSCREEN_ALIGN_POT && w != 1)
	w = 1 << (kaaLog2(w - 1) + 1);
    pitch = (w * bpp / 8 + pKaaScr->info->pitchAlign - 1) &
            ~(pKaaScr->info->pitchAlign - 1);
    
    pKaaPixmap->devKind = pPixmap->devKind;
    pKaaPixmap->devPrivate = pPixmap->devPrivate;
    pKaaPixmap->area = KdOffscreenAlloc (pScreen, pitch * h,
					 pKaaScr->info->offsetAlign,
					 FALSE, 
					 kaaPixmapSave, (pointer) pPixmap);
    if (!pKaaPixmap->area)
	return FALSE;
    
    DBG_PIXMAP(("++ 0x%08x (0x%x) (%dx%d)\n",
		  pPixmap->drawable.id,
		  KaaGetPixmapPriv(pPixmap)->area ? 
		  KaaGetPixmapPriv(pPixmap)->area->offset : -1,
		  pPixmap->drawable.width,
		  pPixmap->drawable.height));
    pPixmap->devKind = pitch;
    pPixmap->devPrivate.ptr = (pointer) ((CARD8 *) pScreenPriv->screen->memory_base + pKaaPixmap->area->offset);
    pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    return TRUE;
}

void
kaaMoveInPixmap (PixmapPtr pPixmap)
{
    ScreenPtr	pScreen = pPixmap->drawable.pScreen;
    KaaScreenPriv (pScreen);
    KaaPixmapPriv (pPixmap);
    int dst_pitch, src_pitch, bytes;
    unsigned char *dst, *src;
    int i;

    DBG_MIGRATE (("-> 0x%08x (0x%x) (%dx%d)\n",
		  pPixmap->drawable.id,
		  KaaGetPixmapPriv(pPixmap)->area ? 
		  KaaGetPixmapPriv(pPixmap)->area->offset : -1,
		  pPixmap->drawable.width,
		  pPixmap->drawable.height));

    src = pPixmap->devPrivate.ptr;
    src_pitch = pPixmap->devKind;
    
    if (!kaaPixmapAllocArea (pPixmap))
	return;

    pKaaPixmap->dirty = FALSE;

    if (pKaaScr->info->UploadToScreen)
    {
	if (pKaaScr->info->UploadToScreen(pPixmap, (char *) src, src_pitch))
	    return;
    }

    dst = pPixmap->devPrivate.ptr;
    dst_pitch = pPixmap->devKind;
    
    bytes = src_pitch < dst_pitch ? src_pitch : dst_pitch;

    kaaWaitSync (pPixmap->drawable.pScreen);

    i = pPixmap->drawable.height;
    while (i--) {
	memcpy (dst, src, bytes);
	dst += dst_pitch;
	src += src_pitch;
    }
}

static void
kaaMoveOutPixmap (PixmapPtr pPixmap)
{
    KaaPixmapPriv (pPixmap);
    KdOffscreenArea *area = pKaaPixmap->area;

    DBG_MIGRATE (("<- 0x%08x (0x%x) (%dx%d)\n",
		  pPixmap->drawable.id,
		  KaaGetPixmapPriv(pPixmap)->area ? 
		  KaaGetPixmapPriv(pPixmap)->area->offset : -1,
		  pPixmap->drawable.width,
		  pPixmap->drawable.height));
    if (area)
    {
	kaaPixmapSave (pPixmap->drawable.pScreen, area);
	KdOffscreenFree (pPixmap->drawable.pScreen, area);
    }
}

void
kaaPixmapUseScreen (PixmapPtr pPixmap)
{
    KaaPixmapPriv (pPixmap);

    if (pKaaPixmap->score == KAA_PIXMAP_SCORE_PINNED)
	return;

    if (pKaaPixmap->score == KAA_PIXMAP_SCORE_INIT) {
	kaaMoveInPixmap(pPixmap);
	pKaaPixmap->score = 0;
    }

    if (pKaaPixmap->score < KAA_PIXMAP_SCORE_MAX)
    {
	pKaaPixmap->score++;
	if (!kaaPixmapIsOffscreen(pPixmap) &&
	    pKaaPixmap->score >= KAA_PIXMAP_SCORE_MOVE_IN)
	    kaaMoveInPixmap (pPixmap);
    }
    KdOffscreenMarkUsed (pPixmap);
}

void
kaaPixmapUseMemory (PixmapPtr pPixmap)
{
    KaaPixmapPriv (pPixmap);

    if (pKaaPixmap->score == KAA_PIXMAP_SCORE_PINNED)
	return;

    if (pKaaPixmap->score == KAA_PIXMAP_SCORE_INIT)
	pKaaPixmap->score = 0;

    if (pKaaPixmap->score > KAA_PIXMAP_SCORE_MIN)
    {
	pKaaPixmap->score--;
	if (pKaaPixmap->area &&
	    pKaaPixmap->score <= KAA_PIXMAP_SCORE_MOVE_OUT)
	    kaaMoveOutPixmap (pPixmap);
    }
}

static Bool
kaaDestroyPixmap (PixmapPtr pPixmap)
{
    if (pPixmap->refcnt == 1)
    {
	KaaPixmapPriv (pPixmap);
	if (pKaaPixmap->area)
	{
	    DBG_PIXMAP(("-- 0x%08x (0x%x) (%dx%d)\n",
			 pPixmap->drawable.id,
			 KaaGetPixmapPriv(pPixmap)->area->offset,
			 pPixmap->drawable.width,
			 pPixmap->drawable.height));
	    /* Free the offscreen area */
	    KdOffscreenFree (pPixmap->drawable.pScreen, pKaaPixmap->area);
	    pPixmap->devPrivate = pKaaPixmap->devPrivate;
	    pPixmap->devKind = pKaaPixmap->devKind;
	}
    }
    return fbDestroyPixmap (pPixmap);
}

static PixmapPtr 
kaaCreatePixmap(ScreenPtr pScreen, int w, int h, int depth, unsigned usage_hint)
{
    PixmapPtr		pPixmap;
    KaaPixmapPrivPtr	pKaaPixmap;
    int			bpp;
    
    bpp = BitsPerPixel (depth);
    if (bpp == 32 && depth == 24)
    {
	int fb;
	KdScreenPriv (pScreen);
	
	for (fb = 0; fb < KD_MAX_FB && pScreenPriv->screen->fb[fb].depth; fb++)
	    if (pScreenPriv->screen->fb[fb].depth == 24)
	    {
		bpp = pScreenPriv->screen->fb[fb].bitsPerPixel;
		break;
	    }
    }

    pPixmap = fbCreatePixmapBpp (pScreen, w, h, depth, bpp, usage_hint);
    if (!pPixmap)
	return NULL;
    pKaaPixmap = KaaGetPixmapPriv(pPixmap);
    if (!w || !h)
	pKaaPixmap->score = KAA_PIXMAP_SCORE_PINNED;
    else
	pKaaPixmap->score = KAA_PIXMAP_SCORE_INIT;
    
    pKaaPixmap->area = NULL;
    pKaaPixmap->dirty = FALSE;

    return pPixmap;
}

Bool
kaaPixmapIsOffscreen(PixmapPtr p)
{
    ScreenPtr	pScreen = p->drawable.pScreen;
    KdScreenPriv(pScreen);

    return ((unsigned long) ((CARD8 *) p->devPrivate.ptr - 
			     (CARD8 *) pScreenPriv->screen->memory_base) <
	    pScreenPriv->screen->memory_size);
}

PixmapPtr
kaaGetOffscreenPixmap (DrawablePtr pDrawable, int *xp, int *yp)
{
    PixmapPtr	pPixmap;
    int		x, y;
    
    if (pDrawable->type == DRAWABLE_WINDOW) {
	pPixmap = (*pDrawable->pScreen->GetWindowPixmap) ((WindowPtr) pDrawable);
#ifdef COMPOSITE
	x = -pPixmap->screen_x;
	y = -pPixmap->screen_y;
#else
	x = 0;
	y = 0;
#endif
    }
    else
    {
	pPixmap = (PixmapPtr) pDrawable;
	x = 0;
	y = 0;
    }
    *xp = x;
    *yp = y;
    if (kaaPixmapIsOffscreen (pPixmap))
	return pPixmap;
    else
	return NULL;
}

Bool
kaaDrawableIsOffscreen (DrawablePtr pDrawable)
{
    PixmapPtr	pPixmap;
    if (pDrawable->type == DRAWABLE_WINDOW)
	pPixmap = (*pDrawable->pScreen->GetWindowPixmap) ((WindowPtr) pDrawable);
    else
	pPixmap = (PixmapPtr) pDrawable;
    return kaaPixmapIsOffscreen (pPixmap);
}

#if 0
static void
kaaFillTiled(int	dst_x,
	     int	dst_y,
	     int	width,
	     int	height,
	     int	src_x,
	     int	src_y,
	     int	src_width,
	     int	src_height,
	     void	(*Copy) (int	srcX,
				 int	srcY,
				 int	dstX,
				 int	dstY,
				 int	width,
				 int	height))
{
    modulus (src_x, src_width, src_x);
    modulus (src_y, src_height, src_y);
    
    while (height)
    {
	int dst_x_tmp = dst_x;
	int src_x_tmp = src_x;
	int width_tmp = width;
	int height_left = src_height - src_y;
	int height_this = min (height, height_left);
	
	while (width_tmp)
	{
	    int width_left = src_width - src_x_tmp;
	    int width_this = min (width_tmp, width_left);

	    (*Copy) (src_x_tmp, src_y,
		     dst_x_tmp, dst_y,
		     width_this, height_this);

	    width_tmp -= width_this;
	    dst_x_tmp += width_this;
	}
	height -= height_this;
	dst_y += height_this;
	src_y = 0;
    }
}
#endif

static void
kaaFillSpans(DrawablePtr pDrawable, GCPtr pGC, int n, 
	     DDXPointPtr ppt, int *pwidth, int fSorted)
{
    ScreenPtr	    pScreen = pDrawable->pScreen;
    KdScreenPriv (pScreen);
    KaaScreenPriv (pScreen);
    RegionPtr	    pClip = fbGetCompositeClip(pGC);
    PixmapPtr	    pPixmap;    
    BoxPtr	    pextent, pbox;
    int		    nbox;
    int		    extentX1, extentX2, extentY1, extentY2;
    int		    fullX1, fullX2, fullY1;
    int		    partX1, partX2;
    int		    off_x, off_y;

    if (!pScreenPriv->enabled ||
	pGC->fillStyle != FillSolid ||
	!(pPixmap = kaaGetOffscreenPixmap (pDrawable, &off_x, &off_y)) ||
	!(*pKaaScr->info->PrepareSolid) (pPixmap,
					 pGC->alu,
					 pGC->planemask,
					 pGC->fgPixel))
    {
	KdCheckFillSpans (pDrawable, pGC, n, ppt, pwidth, fSorted);
	return;
    }
    
    pextent = REGION_EXTENTS(pGC->pScreen, pClip);
    extentX1 = pextent->x1;
    extentY1 = pextent->y1;
    extentX2 = pextent->x2;
    extentY2 = pextent->y2;
    while (n--)
    {
	fullX1 = ppt->x;
	fullY1 = ppt->y;
	fullX2 = fullX1 + (int) *pwidth;
	ppt++;
	pwidth++;
	
	if (fullY1 < extentY1 || extentY2 <= fullY1)
	    continue;
	
	if (fullX1 < extentX1)
	    fullX1 = extentX1;

	if (fullX2 > extentX2)
	    fullX2 = extentX2;
	
	if (fullX1 >= fullX2)
	    continue;
	
	nbox = REGION_NUM_RECTS (pClip);
	if (nbox == 1)
	{
	    (*pKaaScr->info->Solid) (fullX1 + off_x, fullY1 + off_y,
				     fullX2 + off_x, fullY1 + 1 + off_y);
	}
	else
	{
	    pbox = REGION_RECTS(pClip);
	    while(nbox--)
	    {
		if (pbox->y1 <= fullY1 && fullY1 < pbox->y2)
		{
		    partX1 = pbox->x1;
		    if (partX1 < fullX1)
			partX1 = fullX1;
		    partX2 = pbox->x2;
		    if (partX2 > fullX2)
			partX2 = fullX2;
		    if (partX2 > partX1)
			(*pKaaScr->info->Solid) (partX1 + off_x, fullY1 + off_y,
						 partX2 + off_x, fullY1 + 1 + off_y);
		}
		pbox++;
	    }
	}
    }
    (*pKaaScr->info->DoneSolid) ();
    kaaDrawableDirty (pDrawable);
    kaaMarkSync (pDrawable->pScreen);
}

void
kaaCopyNtoN (DrawablePtr    pSrcDrawable,
	     DrawablePtr    pDstDrawable,
	     GCPtr	    pGC,
	     BoxPtr	    pbox,
	     int	    nbox,
	     int	    dx,
	     int	    dy,
	     Bool	    reverse,
	     Bool	    upsidedown,
	     Pixel	    bitplane,
	     void	    *closure)
{
    KdScreenPriv (pDstDrawable->pScreen);
    KaaScreenPriv (pDstDrawable->pScreen);
    PixmapPtr pSrcPixmap, pDstPixmap;
    int	    src_off_x, src_off_y;
    int	    dst_off_x, dst_off_y;

    /* Migrate pixmaps to same place as destination */
    if (pScreenPriv->enabled && pSrcDrawable->type == DRAWABLE_PIXMAP) {
	if (kaaDrawableIsOffscreen (pDstDrawable))
	    kaaPixmapUseScreen ((PixmapPtr) pSrcDrawable);
	else
	    kaaPixmapUseMemory ((PixmapPtr) pSrcDrawable);
    }

    if (pScreenPriv->enabled &&
	(pSrcPixmap = kaaGetOffscreenPixmap (pSrcDrawable, &src_off_x, &src_off_y)) &&
	(pDstPixmap = kaaGetOffscreenPixmap (pDstDrawable, &dst_off_x, &dst_off_y)) && 
	(*pKaaScr->info->PrepareCopy) (pSrcPixmap,
				       pDstPixmap,
				       dx,
				       dy,
				       pGC ? pGC->alu : GXcopy,
				       pGC ? pGC->planemask : FB_ALLONES))
    {
	while (nbox--)
	{
	    (*pKaaScr->info->Copy) (pbox->x1 + dx + src_off_x,
				    pbox->y1 + dy + src_off_y,
				    pbox->x1 + dst_off_x, pbox->y1 + dst_off_y,
				    pbox->x2 - pbox->x1,
				    pbox->y2 - pbox->y1);
	    pbox++;
	}
	(*pKaaScr->info->DoneCopy) ();
	kaaMarkSync (pDstDrawable->pScreen);
    }
    else
    {
	kaaWaitSync (pDstDrawable->pScreen);
	fbCopyNtoN (pSrcDrawable, pDstDrawable, pGC, 
		    pbox, nbox, dx, dy, reverse, upsidedown, 
		    bitplane, closure);
    }
    kaaDrawableDirty (pDstDrawable);
}

static RegionPtr
kaaCopyArea(DrawablePtr pSrcDrawable, DrawablePtr pDstDrawable, GCPtr pGC,
	    int srcx, int srcy, int width, int height, int dstx, int dsty)
{
    return fbDoCopy (pSrcDrawable, pDstDrawable, pGC, 
		     srcx, srcy, width, height, 
		     dstx, dsty, kaaCopyNtoN, 0, 0);
}

static void
kaaPolyFillRect(DrawablePtr pDrawable, 
		GCPtr	    pGC, 
		int	    nrect,
		xRectangle  *prect)
{
    KdScreenPriv (pDrawable->pScreen);
    KaaScreenPriv (pDrawable->pScreen);
    RegionPtr	    pClip = fbGetCompositeClip(pGC);
    PixmapPtr	    pPixmap;
    register BoxPtr pbox;
    BoxPtr	    pextent;
    int		    extentX1, extentX2, extentY1, extentY2;
    int		    fullX1, fullX2, fullY1, fullY2;
    int		    partX1, partX2, partY1, partY2;
    int		    xoff, yoff;
    int		    xorg, yorg;
    int		    n;
    
    if (!pScreenPriv->enabled ||
	pGC->fillStyle != FillSolid ||
	!(pPixmap = kaaGetOffscreenPixmap (pDrawable, &xoff, &yoff)) || 
	!(*pKaaScr->info->PrepareSolid) (pPixmap,
					 pGC->alu,
					 pGC->planemask,
					 pGC->fgPixel))
    {
	KdCheckPolyFillRect (pDrawable, pGC, nrect, prect);
	return;
    }
    
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    
    pextent = REGION_EXTENTS(pGC->pScreen, pClip);
    extentX1 = pextent->x1;
    extentY1 = pextent->y1;
    extentX2 = pextent->x2;
    extentY2 = pextent->y2;
    while (nrect--)
    {
	fullX1 = prect->x + xorg;
	fullY1 = prect->y + yorg;
	fullX2 = fullX1 + (int) prect->width;
	fullY2 = fullY1 + (int) prect->height;
	prect++;
	
	if (fullX1 < extentX1)
	    fullX1 = extentX1;

	if (fullY1 < extentY1)
	    fullY1 = extentY1;

	if (fullX2 > extentX2)
	    fullX2 = extentX2;
	
	if (fullY2 > extentY2)
	    fullY2 = extentY2;

	if ((fullX1 >= fullX2) || (fullY1 >= fullY2))
	    continue;
	n = REGION_NUM_RECTS (pClip);
	if (n == 1)
	{
	    (*pKaaScr->info->Solid) (fullX1 + xoff, fullY1 + yoff,
				     fullX2 + xoff, fullY2 + yoff);
	}
	else
	{
	    pbox = REGION_RECTS(pClip);
	    /* 
	     * clip the rectangle to each box in the clip region
	     * this is logically equivalent to calling Intersect()
	     */
	    while(n--)
	    {
		partX1 = pbox->x1;
		if (partX1 < fullX1)
		    partX1 = fullX1;
		partY1 = pbox->y1;
		if (partY1 < fullY1)
		    partY1 = fullY1;
		partX2 = pbox->x2;
		if (partX2 > fullX2)
		    partX2 = fullX2;
		partY2 = pbox->y2;
		if (partY2 > fullY2)
		    partY2 = fullY2;
    
		pbox++;
		
		if (partX1 < partX2 && partY1 < partY2)
		    (*pKaaScr->info->Solid) (partX1 + xoff, partY1 + yoff,
					     partX2 + xoff, partY2 + yoff);
	    }
	}
    }
    (*pKaaScr->info->DoneSolid) ();
    kaaDrawableDirty (pDrawable);
    kaaMarkSync (pDrawable->pScreen);
}
    
static void
kaaSolidBoxClipped (DrawablePtr	pDrawable,
		    RegionPtr	pClip,
		    FbBits	pm,
		    FbBits	fg,
		    int		x1,
		    int		y1,
		    int		x2,
		    int		y2)
{
    KdScreenPriv (pDrawable->pScreen);
    KaaScreenPriv (pDrawable->pScreen);
    PixmapPtr   pPixmap;        
    BoxPtr	pbox;
    int		nbox;
    int		xoff, yoff;
    int		partX1, partX2, partY1, partY2;

    if (!pScreenPriv->enabled ||
	!(pPixmap = kaaGetOffscreenPixmap (pDrawable, &xoff, &yoff)) ||
	!(*pKaaScr->info->PrepareSolid) (pPixmap, GXcopy, pm, fg))
    {
	kaaWaitSync (pDrawable->pScreen);
	fg = fbReplicatePixel (fg, pDrawable->bitsPerPixel);
	fbSolidBoxClipped (pDrawable, pClip, x1, y1, x2, y2,
			   fbAnd (GXcopy, fg, pm),
			   fbXor (GXcopy, fg, pm));
	kaaDrawableDirty (pDrawable);
	return;
    }
    for (nbox = REGION_NUM_RECTS(pClip), pbox = REGION_RECTS(pClip); 
	 nbox--; 
	 pbox++)
    {
	partX1 = pbox->x1;
	if (partX1 < x1)
	    partX1 = x1;
	
	partX2 = pbox->x2;
	if (partX2 > x2)
	    partX2 = x2;
	
	if (partX2 <= partX1)
	    continue;
	
	partY1 = pbox->y1;
	if (partY1 < y1)
	    partY1 = y1;
	
	partY2 = pbox->y2;
	if (partY2 > y2)
	    partY2 = y2;
	
	if (partY2 <= partY1)
	    continue;
	
	(*pKaaScr->info->Solid) (partX1 + xoff, partY1 + yoff,
				 partX2 + xoff, partY2 + yoff);
    }
    (*pKaaScr->info->DoneSolid) ();
    kaaDrawableDirty (pDrawable);
    kaaMarkSync (pDrawable->pScreen);
}

static void
kaaImageGlyphBlt (DrawablePtr	pDrawable,
		  GCPtr		pGC,
		  int		x, 
		  int		y,
		  unsigned int	nglyph,
		  CharInfoPtr	*ppciInit,
		  pointer	pglyphBase)
{
    FbGCPrivPtr	    pPriv = fbGetGCPrivate(pGC);
    CharInfoPtr	    *ppci;
    CharInfoPtr	    pci;
    unsigned char   *pglyph;		/* pointer bits in glyph */
    int		    gWidth, gHeight;	/* width and height of glyph */
    FbStride	    gStride;		/* stride of glyph */
    Bool	    opaque;
    int		    n;
    int		    gx, gy;
    void	    (*glyph) (FbBits *,
			      FbStride,
			      int,
			      FbStip *,
			      FbBits,
			      int,
			      int);
    FbBits	    *dst;
    FbStride	    dstStride;
    int		    dstBpp;
    int		    dstXoff, dstYoff;
    FbBits	    depthMask;
    
    depthMask = FbFullMask(pDrawable->depth);
    if ((pGC->planemask & depthMask) != depthMask)
    {
	KdCheckImageGlyphBlt(pDrawable, pGC, x, y, nglyph, ppciInit, pglyphBase);
	return;
    }
    glyph = 0;
    fbGetDrawable (pDrawable, dst, dstStride, dstBpp, dstXoff, dstYoff);
    switch (dstBpp) {
    case 8:	glyph = fbGlyph8; break;
    case 16:    glyph = fbGlyph16; break;
    case 24:    glyph = fbGlyph24; break;
    case 32:    glyph = fbGlyph32; break;
    }
    
    x += pDrawable->x;
    y += pDrawable->y;

    if (TERMINALFONT (pGC->font) && !glyph)
    {
	opaque = TRUE;
    }
    else
    {
	int		xBack, widthBack;
	int		yBack, heightBack;
	
	ppci = ppciInit;
	n = nglyph;
	widthBack = 0;
	while (n--)
	    widthBack += (*ppci++)->metrics.characterWidth;
	
        xBack = x;
	if (widthBack < 0)
	{
	    xBack += widthBack;
	    widthBack = -widthBack;
	}
	yBack = y - FONTASCENT(pGC->font);
	heightBack = FONTASCENT(pGC->font) + FONTDESCENT(pGC->font);
        kaaSolidBoxClipped (pDrawable,
			    fbGetCompositeClip(pGC),
			    pGC->planemask,
			    pGC->bgPixel,
			    xBack,
			    yBack,
			    xBack + widthBack,
			    yBack + heightBack);
	opaque = FALSE;
    }

    kaaWaitSync (pDrawable->pScreen);
    kaaDrawableDirty (pDrawable);
    
    ppci = ppciInit;
    while (nglyph--)
    {
	pci = *ppci++;
	pglyph = FONTGLYPHBITS(pglyphBase, pci);
	gWidth = GLYPHWIDTHPIXELS(pci);
	gHeight = GLYPHHEIGHTPIXELS(pci);
	if (gWidth && gHeight)
	{
	    gx = x + pci->metrics.leftSideBearing;
	    gy = y - pci->metrics.ascent; 
	    if (glyph && gWidth <= sizeof (FbStip) * 8 &&
		fbGlyphIn (fbGetCompositeClip(pGC), gx, gy, gWidth, gHeight))
	    {
		(*glyph) (dst + (gy + dstYoff) * dstStride,
			  dstStride,
			  dstBpp,
			  (FbStip *) pglyph,
			  pPriv->fg,
			  gx + dstXoff,
			  gHeight);
	    }
	    else
	    {
		gStride = GLYPHWIDTHBYTESPADDED(pci) / sizeof (FbStip);
		fbPutXYImage (pDrawable,
			      fbGetCompositeClip(pGC),
			      pPriv->fg,
			      pPriv->bg,
			      pPriv->pm,
			      GXcopy,
			      opaque,
    
			      gx,
			      gy,
			      gWidth, gHeight,
    
			      (FbStip *) pglyph,
			      gStride,
			      0);
	    }
	}
	x += pci->metrics.characterWidth;
    }
}

static const GCOps	kaaOps = {
    kaaFillSpans,
    KdCheckSetSpans,
    KdCheckPutImage,
    kaaCopyArea,
    KdCheckCopyPlane,
    KdCheckPolyPoint,
    KdCheckPolylines,
    KdCheckPolySegment,
    miPolyRectangle,
    KdCheckPolyArc,
    miFillPolygon,
    kaaPolyFillRect,
    miPolyFillArc,
    miPolyText8,
    miPolyText16,
    miImageText8,
    miImageText16,
    kaaImageGlyphBlt,
    KdCheckPolyGlyphBlt,
    KdCheckPushPixels,
};

static void
kaaValidateGC (GCPtr pGC, Mask changes, DrawablePtr pDrawable)
{
    fbValidateGC (pGC, changes, pDrawable);

    if (kaaDrawableIsOffscreen (pDrawable))
	pGC->ops = (GCOps *) &kaaOps;
    else
	pGC->ops = (GCOps *) &kdAsyncPixmapGCOps;
}

GCFuncs	kaaGCFuncs = {
    kaaValidateGC,
    miChangeGC,
    miCopyGC,
    miDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip
};

static int
kaaCreateGC (GCPtr pGC)
{
    if (!fbCreateGC (pGC))
	return FALSE;

    pGC->funcs = &kaaGCFuncs;

    return TRUE;
}


static void
kaaCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    RegionRec	rgnDst;
    int		dx, dy;
    PixmapPtr	pPixmap = (*pWin->drawable.pScreen->GetWindowPixmap) (pWin);

    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);

    REGION_INIT (pWin->drawable.pScreen, &rgnDst, NullBox, 0);
    
    REGION_INTERSECT(pWin->drawable.pScreen, &rgnDst, &pWin->borderClip, prgnSrc);
#ifdef COMPOSITE
    if (pPixmap->screen_x || pPixmap->screen_y)
	REGION_TRANSLATE (pWin->drawable.pScreen, &rgnDst, 
			  -pPixmap->screen_x, -pPixmap->screen_y);
#endif

    fbCopyRegion (&pPixmap->drawable, &pPixmap->drawable,
		  0,
		  &rgnDst, dx, dy, kaaCopyNtoN, 0, 0);
    
    REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
}

static void
kaaFillRegionSolid (DrawablePtr	pDrawable,
		    RegionPtr	pRegion,
		    Pixel	pixel)
{
    KdScreenPriv(pDrawable->pScreen);
    KaaScreenPriv(pDrawable->pScreen);
    PixmapPtr pPixmap;
    int xoff, yoff;

    if (pScreenPriv->enabled &&
	(pPixmap = kaaGetOffscreenPixmap (pDrawable, &xoff, &yoff)) &&
	(*pKaaScr->info->PrepareSolid) (pPixmap, GXcopy, FB_ALLONES, pixel))
    {
	int	nbox = REGION_NUM_RECTS (pRegion);
	BoxPtr	pBox = REGION_RECTS (pRegion);
	
	while (nbox--)
	{
	    (*pKaaScr->info->Solid) (pBox->x1 + xoff, pBox->y1 + yoff,
				     pBox->x2 + xoff, pBox->y2 + yoff);
	    pBox++;
	}
	(*pKaaScr->info->DoneSolid) ();
	kaaMarkSync (pDrawable->pScreen);
    }
    else
    {
	kaaWaitSync (pDrawable->pScreen);
	fbFillRegionSolid (pDrawable, pRegion, 0,
			   fbReplicatePixel (pixel, pDrawable->bitsPerPixel));
    }
    kaaDrawableDirty (pDrawable);
}

Bool
kaaDrawInit (ScreenPtr		pScreen,
	     KaaScreenInfoPtr	pScreenInfo)
{
    KaaScreenPrivPtr pKaaScr;
    KdScreenPriv(pScreen);
    KdScreenInfo *screen = pScreenPriv->screen;
#ifdef RENDER
    PictureScreenPtr	ps = GetPictureScreenIfSet(pScreen);
#endif
    
    pKaaScr = xalloc (sizeof (KaaScreenPrivRec));

    if (!pKaaScr)
	return FALSE;
    
    pKaaScr->info = pScreenInfo;
    
    dixSetPrivate(&pScreen->devPrivates, kaaScreenPrivateKey, pKaaScr);
    
    /*
     * Hook up asynchronous drawing
     */
    KdScreenInitAsync (pScreen);
    /*
     * Replace various fb screen functions
     */
    pScreen->CreateGC = kaaCreateGC;
    pScreen->CopyWindow = kaaCopyWindow;
#ifdef RENDER
    if (ps) {
	ps->Composite = kaaComposite;
	ps->RasterizeTrapezoid = kaaRasterizeTrapezoid;
    }
#endif

    /*
     * Hookup offscreen pixmaps
     */
    if ((pKaaScr->info->flags & KAA_OFFSCREEN_PIXMAPS) &&
	screen->off_screen_base < screen->memory_size)
    {
	if (!dixRequestPrivate(kaaPixmapPrivateKey, sizeof (KaaPixmapPrivRec)))
	    return FALSE;
	pScreen->CreatePixmap = kaaCreatePixmap;
	pScreen->DestroyPixmap = kaaDestroyPixmap;
    }

    return TRUE;
}

void
kaaDrawFini (ScreenPtr pScreen)
{
    KaaScreenPriv(pScreen);

    xfree (pKaaScr);
}

void
kaaMarkSync (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KaaScreenPriv(pScreen);

    pScreenPriv->card->needSync = TRUE;
    if (pKaaScr->info->markSync != NULL) {
	pScreenPriv->card->lastMarker = (*pKaaScr->info->markSync) (pScreen);
    }
}

void
kaaWaitSync (ScreenPtr pScreen)
{
    KdScreenPriv(pScreen);
    KaaScreenPriv(pScreen);
    KdCardInfo *card = pScreenPriv->card;

    if (card->needSync) {
	(*pKaaScr->info->waitMarker) (pScreen, card->lastMarker);
	card->needSync = FALSE;
    }
}
