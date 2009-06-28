/************************************************************

Copyright (c) 1989  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

********************************************************/
/* GJA -- Took mfb code and modified it. */

/* Derived from:
 * "Algorithm for drawing ellipses or hyperbolae with a digital plotter"
 * by M. L. V. Pitteway
 * The Computer Journal, November 1967, Volume 10, Number 3, pp. 282-289
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf4bpp.h"
#include "OScompiler.h"
#include "mfbmap.h"
#include "mfb.h"
#include "maskbits.h"
#include "mi.h"
#include "mizerarc.h"
#include "wm3.h"

#include "xf86str.h" /* for pScrn->vtSema */
extern ScrnInfoPtr *xf86Screens;

/*
 * Note, LEFTMOST must be the bit leftmost in the actual screen
 * representation.  This depends on both BITMAP_BIT_ORDER and
 * IMAGE_BYTE_ORDER
 * DHD 10/92
 */

#if (BITMAP_BIT_ORDER == MSBFirst)
#if (IMAGE_BYTE_ORDER == MSBFirst)
#define LEFTMOST	((unsigned int) 0x80000000)
#else
#define LEFTMOST	((unsigned int) 0x80)
#endif
#else
#if (IMAGE_BYTE_ORDER == LSBFirst)
#define LEFTMOST	((unsigned int) 1)
#else
#define LEFTMOST	((unsigned int) 0x1000000)
#endif
#endif

#define PixelateWhite(addr,off) \
{ \
    register int *tmpaddr = &((addr)[(off)>>PWSH]); \
    UPDRW(tmpaddr,SCRRIGHT (LEFTMOST, ((off) & PIM))); \
}
#define PixelateBlack(addr,off) \
{ \
    register int *tmpaddr = &((addr)[(off)>>PWSH]); \
    UPDRW(tmpaddr,~(SCRRIGHT (LEFTMOST, ((off) & PIM)))); \
}

#define Pixelate(base,off) \
{ \
    paddr = base + ((off)>>PWSH); \
    pmask = SCRRIGHT(LEFTMOST, (off) & PIM); \
    UPDRW(paddr,(pixel & pmask)); \
}

#define DoPix(bit,base,off) if (msk & bit) Pixelate(base,off);

static void
v16ZeroArcSS
(
    DrawablePtr pDraw,
    GCPtr pGC,
    xArc *arc
)
{
    miZeroArcRec info;
    Bool do360;
    register int x, y, a, b, d, msk;
    register int k1, k3, dx, dy;
    int *addrl;
    int *yorgl, *yorgol;
    unsigned long pixel;
    int nlwidth, yoffset, dyoffset;
    int pmask;
    register int *paddr;

    if (((mfbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
				       mfbGetGCPrivateKey()))->rop ==
	RROP_BLACK)
	pixel = 0;
    else
	pixel = ~0UL;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	addrl = (int *)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
		(((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrl = (int *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDraw)->devKind) >> 2;
    }

    do360 = miZeroArcSetup(arc, &info, TRUE);
    yorgl = addrl + ((info.yorg + pDraw->y) * nlwidth);
    yorgol = addrl + ((info.yorgo + pDraw->y) * nlwidth);
    info.xorg += pDraw->x;
    info.xorgo += pDraw->x;
    MIARCSETUP();
    yoffset = y ? nlwidth : 0;
    dyoffset = 0;
    msk = info.initialMask;
    if (!(arc->width & 1))
    {
	DoPix(2, yorgl, info.xorgo);
	DoPix(8, yorgol, info.xorgo);
    }
    if (!info.end.x || !info.end.y)
    {
	msk = info.end.mask;
	info.end = info.altend;
    }
    if (do360 && (arc->width == arc->height) && !(arc->width & 1))
    {
	int xoffset = nlwidth;
	int *yorghl = yorgl + (info.h * nlwidth);
	int xorghp = info.xorg + info.h;
	int xorghn = info.xorg - info.h;

        while (1)
        {
	    PixelateWhite(yorgl + yoffset, info.xorg + x);
	    PixelateWhite(yorgl + yoffset, info.xorg - x);
	    PixelateWhite(yorgol- yoffset, info.xorg - x);
	    PixelateWhite(yorgol - yoffset, info.xorg + x);
	    if (a < 0)
	        break;
	    PixelateWhite(yorghl - xoffset, xorghp - y);
	    PixelateWhite(yorghl - xoffset, xorghn + y);
	    PixelateWhite(yorghl + xoffset, xorghn + y);
	    PixelateWhite(yorghl + xoffset, xorghp - y);
	    xoffset += nlwidth;
	    MIARCCIRCLESTEP(yoffset += nlwidth;);
        }
	x = info.w;
	yoffset = info.h * nlwidth;
    }
    else if (do360)
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = nlwidth;);
	    Pixelate(yorgl + yoffset, info.xorg + x);
	    Pixelate(yorgl + yoffset, info.xorgo - x);
	    Pixelate(yorgol - yoffset, info.xorgo - x);
	    Pixelate(yorgol - yoffset, info.xorg + x);
	    MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
	}
    }
    else
    {
	while (y < info.h || x < info.w)
	{
	    MIARCOCTANTSHIFT(dyoffset = nlwidth;);
	    if ((x == info.start.x) || (y == info.start.y))
	    {
		msk = info.start.mask;
		info.start = info.altstart;
	    }
	    DoPix(1, yorgl + yoffset, info.xorg + x);
	    DoPix(2, yorgl + yoffset, info.xorgo - x);
	    DoPix(4, yorgol - yoffset, info.xorgo - x);
	    DoPix(8, yorgol - yoffset, info.xorg + x);
	    if ((x == info.end.x) || (y == info.end.y))
	    {
		msk = info.end.mask;
		info.end = info.altend;
	    }
	    MIARCSTEP(yoffset += dyoffset;, yoffset += nlwidth;);
	}
    }
    if ((x == info.start.x) || (y == info.start.y))
	msk = info.start.mask;
    DoPix(1, yorgl + yoffset, info.xorg + x);
    DoPix(4, yorgol - yoffset, info.xorgo - x);
    if (arc->height & 1)
    {
	DoPix(2, yorgl + yoffset, info.xorgo - x);
	DoPix(8, yorgol - yoffset, info.xorg + x);
    }
}

static void
xf4bppZeroPolyArcSS
(
    DrawablePtr	pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs
)
{
    register xArc *arc;
    register int i;
    BoxRec box;
    RegionPtr cclip;

    if (!pGC->planemask & 0x0F)
	return;
    cclip = pGC->pCompositeClip;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miCanZeroArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN)
		v16ZeroArcSS(pDraw, pGC, arc);
	    else
		miZeroPolyArc(pDraw, pGC, 1, arc);
	}
	else
	    miPolyArc(pDraw, pGC, 1, arc);
    }
}

void
xf4bppZeroPolyArc(pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    if ( !xf86Screens[pDraw->pScreen->myNum]->vtSema ) {
	miZeroPolyArc(pDraw, pGC, narcs, parcs);
    } else {
        DO_WM3(pGC,xf4bppZeroPolyArcSS(pDraw, pGC, narcs, parcs));
    }
}
