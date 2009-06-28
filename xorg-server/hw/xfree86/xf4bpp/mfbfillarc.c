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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf4bpp.h"
#include "OScompiler.h"
#include "mfbmap.h"
#include "mfb.h"
#include "maskbits.h"
#include "mi.h"
#include "mifillarc.h"
#include "wm3.h"

#include "xf86str.h" /* for pScrn->vtSema */
extern ScrnInfoPtr *xf86Screens;

static void
v16FillEllipseSolid
(
    DrawablePtr pDraw,
    xArc *arc
)
{
    int x, y, e;
    int yk, xk, ym, xm, dx, dy, xorg, yorg;
    register int slw;
    miFillArcRec info;
    int *addrlt, *addrlb;
    register int *addrl;
    register int n;
    int nlwidth;
    register int xpos;
    int startmask, endmask, nlmiddle;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	addrlt = (int *)
	       (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
	       (((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrlt = (int *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDraw)->devKind) >> 2;
    }

    miFillArcSetup(arc, &info);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;
    addrlb = addrlt;
    addrlt += nlwidth * (yorg - y);
    addrlb += nlwidth * (yorg + y + dy);
    while (y)
    {
	addrlt += nlwidth;
	addrlb -= nlwidth;
	MIFILLARCSTEP(slw);
	if (!slw)
	    continue;
	xpos = xorg - x;
	addrl = addrlt + (xpos >> PWSH);
	if (((xpos & PIM) + slw) < PPW)
	{
	    maskpartialbits(xpos, slw, startmask);
	    UPDRW(addrl,startmask);
	    if (miFillArcLower(slw))
	    {
		addrl = addrlb + (xpos >> PWSH);
		UPDRW(addrl,startmask);
	    }
	    continue;
	}
	maskbits(xpos, slw, startmask, endmask, nlmiddle);
	if (startmask)
	{
	    UPDRW(addrl,startmask); addrl++;
	}
	n = nlmiddle;
	while (n--) {
	    UPDRW(addrl,~0); addrl++;
	}
	if (endmask)
	{
	    UPDRW(addrl,endmask);
	}
	if (!miFillArcLower(slw))
	    continue;
	addrl = addrlb + (xpos >> PWSH);
	if (startmask)
	{
	    UPDRW(addrl,startmask); addrl++;
	}
	n = nlmiddle;
	while (n--) {
	    UPDRW(addrl,~0); addrl++;
	}
	if (endmask)
	{
	    UPDRW(addrl,endmask);
	}
    }
}

#define FILLSPAN(xl,xr,addr) \
    if (xr >= xl) \
    { \
	width = xr - xl + 1; \
	addrl = addr + (xl >> PWSH); \
	if (((xl & PIM) + width) < PPW) \
	{ \
	    maskpartialbits(xl, width, startmask); \
	    UPDRW(addrl,startmask); \
	} \
	else \
	{ \
	    maskbits(xl, width, startmask, endmask, nlmiddle); \
	    if (startmask) \
	    { \
		UPDRW(addrl,startmask); addrl++; \
	    } \
	    n = nlmiddle; \
		while (n--) { \
		    UPDRW(addrl,~0); addrl++; \
		} \
	    if (endmask) \
	    { \
		UPDRW(addrl,endmask); \
	    } \
	} \
    }

#define FILLSLICESPANS(flip,addr) \
    if (!flip) \
    { \
	FILLSPAN(xl, xr, addr); \
    } \
    else \
    { \
	xc = xorg - x; \
	FILLSPAN(xc, xr, addr); \
	xc += slw - 1; \
	FILLSPAN(xl, xc, addr); \
    }

static void
v16FillArcSliceSolidCopy
(
    DrawablePtr pDraw,
    GCPtr pGC,
    xArc *arc
)
{
    register int *addrl;
    register int n;
    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
    register int x, y, e;
    miFillArcRec info;
    miArcSliceRec slice;
    int xl, xr, xc;
    int *addrlt, *addrlb;
    int nlwidth;
    int width;
    int startmask, endmask, nlmiddle;

    if (pDraw->type == DRAWABLE_WINDOW)
    {
	addrlt = (int *)
	       (((PixmapPtr)(pDraw->pScreen->devPrivate))->devPrivate.ptr);
	nlwidth = (int)
	       (((PixmapPtr)(pDraw->pScreen->devPrivate))->devKind) >> 2;
    }
    else
    {
	addrlt = (int *)(((PixmapPtr)pDraw)->devPrivate.ptr);
	nlwidth = (int)(((PixmapPtr)pDraw)->devKind) >> 2;
    }

    miFillArcSetup(arc, &info);
    miFillArcSliceSetup(arc, &slice, pGC);
    MIFILLARCSETUP();
    xorg += pDraw->x;
    yorg += pDraw->y;
    addrlb = addrlt;
    addrlt += nlwidth * (yorg - y);
    addrlb += nlwidth * (yorg + y + dy);
    slice.edge1.x += pDraw->x;
    slice.edge2.x += pDraw->x;
    while (y > 0)
    {
	addrlt += nlwidth;
	addrlb -= nlwidth;
	MIFILLARCSTEP(slw);
	MIARCSLICESTEP(slice.edge1);
	MIARCSLICESTEP(slice.edge2);
	if (miFillSliceUpper(slice))
	{
	    MIARCSLICEUPPER(xl, xr, slice, slw);
	    FILLSLICESPANS(slice.flip_top, addrlt);
	}
	if (miFillSliceLower(slice))
	{
	    MIARCSLICELOWER(xl, xr, slice, slw);
	    FILLSLICESPANS(slice.flip_bot, addrlb);
	}
    }
}

static void
xf4bppPolyFillArcSolid
(
    register DrawablePtr pDraw,
    GCPtr	pGC,
    int		narcs,
    xArc	*parcs
)
{
    register xArc *arc;
    register int i;
    BoxRec box;
    RegionPtr cclip;
#if 0
    mfbPrivGC *priv;
    int rop;

    priv = (mfbPrivGC *)dixLookupPrivate(&pGC->devPrivates,
					 mfbGetGCPrivateKey());
    rop = priv->rop;
    if ((rop == RROP_NOP) || !(pGC->planemask & 1))
#else
    if ( !(pGC->planemask & 0x0F))
#endif
	return;
    cclip = pGC->pCompositeClip;
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miFillArcEmpty(arc))
	    continue;
	if (miCanFillArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
	    box.x2 = box.x1 + (int)arc->width + 1;
	    box.y2 = box.y1 + (int)arc->height + 1;
	    if (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN)
	    {
		if ((arc->angle2 >= FULLCIRCLE) ||
		    (arc->angle2 <= -FULLCIRCLE))
		    DO_WM3(pGC,v16FillEllipseSolid(pDraw, arc))
		else
		    DO_WM3(pGC,v16FillArcSliceSolidCopy(pDraw, pGC, arc)) 
		continue;
	    }
	}
	miPolyFillArc(pDraw, pGC, 1, arc);
    }
}

void
xf4bppPolyFillArc(pDraw, pGC, narcs, parcs)
    register DrawablePtr pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    if ( !xf86Screens[pDraw->pScreen->myNum]->vtSema || (pGC->fillStyle != FillSolid) ) {
	miPolyFillArc(pDraw, pGC, narcs, parcs);
    } else {
        xf4bppPolyFillArcSolid(pDraw, pGC, narcs, parcs);
    }
}
