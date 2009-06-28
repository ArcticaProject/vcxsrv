/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

********************************************************/


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <limits.h>

#include <X11/X.h>
#include <X11/Xprotostr.h>
#include "regionstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "scrnintstr.h"
#include "cfb.h"
#include "cfbmskbits.h"
#include "mifillarc.h"
#include "cfbrrop.h"
#include "mi.h"

/* gcc 1.35 is stupid */
#if defined(__GNUC__) && __GNUC__ < 2 && defined(mc68020)
#define STUPID volatile
#else
#define STUPID
#endif

static void
RROP_NAME(cfbFillEllipseSolid)(
    DrawablePtr pDraw,
    GCPtr pGC,
    xArc *arc)
{
    STUPID int x, y, e;
    STUPID int yk, xk, ym, xm, dx, dy, xorg, yorg;
    miFillArcRec info;
#if PSZ == 24
    unsigned char *addrlt, *addrlb;
#else
    CfbBits *addrlt, *addrlb;
#endif
    register CfbBits *addrl;
    register int n;
    int nlwidth;
    RROP_DECLARE
    register int xpos;
    register int slw;
    CfbBits startmask, endmask;
    int	nlmiddle;
#if PSZ == 24
    register int pidx;
    int xpos3;
#endif

#if PSZ == 24
    cfbGetByteWidthAndPointer (pDraw, nlwidth, addrlt)
#else
    cfbGetLongWidthAndPointer (pDraw, nlwidth, addrlt)
#endif

    RROP_FETCH_GC(pGC);
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
#if PSZ == 24
	xpos3 = (xpos * 3) & ~0x03;
	addrl = (CfbBits *)((char *)addrlt + xpos3);
	if (slw == 1){
	  RROP_SOLID24(addrl, xpos);
	  if (miFillArcLower(slw)){
	    addrl = (CfbBits *)((char *)addrlb + xpos3);
	    RROP_SOLID24(addrl, xpos);
          }
	  continue;
	}
	maskbits(xpos, slw, startmask, endmask, nlmiddle);
	xpos &= 3;
	pidx = xpos;
	if (startmask){
	  RROP_SOLID_MASK(addrl, startmask, pidx-1);
	  addrl++;
	  if (pidx == 3)
	    pidx = 0;
	}
	n = nlmiddle;
	while (--n >= 0){
	  RROP_SOLID(addrl, pidx);
	  addrl++;
	  if (++pidx == 3)
	    pidx = 0;
	}
	if (endmask)
	  RROP_SOLID_MASK(addrl, endmask, pidx);
	if (!miFillArcLower(slw))
	  continue;
	addrl = (CfbBits *)((char *)addrlb + xpos3);
	pidx = xpos;
	if (startmask){
	  RROP_SOLID_MASK(addrl, startmask, pidx-1);
	  addrl++;
	  if (pidx == 3)
	    pidx = 0;
	}
	n = nlmiddle;
	while (--n >= 0){
	  RROP_SOLID(addrl, pidx);
	  addrl++;
	  if (++pidx == 3)
	    pidx = 0;
	}
	if (endmask)
	  RROP_SOLID_MASK(addrl, endmask, pidx);
#else /* PSZ == 24 */
	addrl = addrlt + (xpos >> PWSH);
	if (((xpos & PIM) + slw) <= PPW)
	{
	    maskpartialbits(xpos, slw, startmask);
	    RROP_SOLID_MASK(addrl,startmask);
	    if (miFillArcLower(slw))
	    {
		addrl = addrlb + (xpos >> PWSH);
		RROP_SOLID_MASK(addrl, startmask);
	    }
	    continue;
	}
	maskbits(xpos, slw, startmask, endmask, nlmiddle);
	if (startmask)
	{
	    RROP_SOLID_MASK(addrl, startmask);
	    addrl++;
	}
	n = nlmiddle;
	RROP_SPAN(addrl,n)

	if (endmask)
	    RROP_SOLID_MASK(addrl, endmask);
	if (!miFillArcLower(slw))
	    continue;
	addrl = addrlb + (xpos >> PWSH);
	if (startmask)
	{
	    RROP_SOLID_MASK(addrl, startmask);
	    addrl++;
	}
	n = nlmiddle;
	RROP_SPAN(addrl, n);
	if (endmask)
	    RROP_SOLID_MASK(addrl, endmask);
#endif /* PSZ == 24 */
    }
    RROP_UNDECLARE
}

#if PSZ == 24
#define FILLSPAN(xl,xr,addr) \
    if (xr >= xl){ \
	n = xr - xl + 1; \
	addrl = (CfbBits *)((char *)addr + ((xl * 3) & ~0x03)); \
	if (n <= 1){ \
          if (n) \
            RROP_SOLID24(addrl, xl); \
	} else { \
	  maskbits(xl, n, startmask, endmask, n); \
          pidx = xl & 3; \
	  if (startmask){ \
	    RROP_SOLID_MASK(addrl, startmask, pidx-1); \
	    addrl++; \
	    if (pidx == 3) \
	      pidx = 0; \
	  } \
	  while (--n >= 0){ \
	    RROP_SOLID(addrl, pidx); \
	    addrl++; \
	    if (++pidx == 3) \
	      pidx = 0; \
	  } \
	  if (endmask) \
	    RROP_SOLID_MASK(addrl, endmask, pidx); \
	} \
    }
#else /* PSZ == 24 */
#define FILLSPAN(xl,xr,addr) \
    if (xr >= xl) \
    { \
	n = xr - xl + 1; \
	addrl = addr + (xl >> PWSH); \
	if (((xl & PIM) + n) <= PPW) \
	{ \
	    maskpartialbits(xl, n, startmask); \
	    RROP_SOLID_MASK(addrl, startmask); \
	} \
	else \
	{ \
	    maskbits(xl, n, startmask, endmask, n); \
	    if (startmask) \
	    { \
		RROP_SOLID_MASK(addrl, startmask); \
		addrl++; \
	    } \
	    while (n--) \
	    { \
		RROP_SOLID(addrl); \
		++addrl; \
	    } \
	    if (endmask) \
		RROP_SOLID_MASK(addrl, endmask); \
	} \
    }
#endif /* PSZ == 24 */

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
RROP_NAME(cfbFillArcSliceSolid)(
    DrawablePtr pDraw,
    GCPtr pGC,
    xArc *arc)
{
    int yk, xk, ym, xm, dx, dy, xorg, yorg, slw;
    register int x, y, e;
    miFillArcRec info;
    miArcSliceRec slice;
    int xl, xr, xc;
#if PSZ == 24
    unsigned char *addrlt, *addrlb;
#else
    CfbBits *addrlt, *addrlb;
#endif
    register CfbBits *addrl;
    register int n;
    int nlwidth;
    RROP_DECLARE
    CfbBits startmask, endmask;
#if PSZ == 24
    register int pidx;
#endif /* PSZ == 24 */

#if PSZ == 24
    cfbGetByteWidthAndPointer (pDraw, nlwidth, addrlt)
#else
    cfbGetLongWidthAndPointer (pDraw, nlwidth, addrlt)
#endif

    RROP_FETCH_GC(pGC);
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
    RROP_UNDECLARE
}

void
RROP_NAME(cfbPolyFillArcSolid) (pDraw, pGC, narcs, parcs)
    DrawablePtr	pDraw;
    GCPtr	pGC;
    int		narcs;
    xArc	*parcs;
{
    register xArc *arc;
    register int i;
    int x2, y2;
    BoxRec box;
    RegionPtr cclip;

    cclip = cfbGetCompositeClip(pGC);
    for (arc = parcs, i = narcs; --i >= 0; arc++)
    {
	if (miFillArcEmpty(arc))
	    continue;
	if (miCanFillArc(arc))
	{
	    box.x1 = arc->x + pDraw->x;
	    box.y1 = arc->y + pDraw->y;
 	    /*
 	     * Because box.x2 and box.y2 get truncated to 16 bits, and the
 	     * RECT_IN_REGION test treats the resulting number as a signed
 	     * integer, the RECT_IN_REGION test alone can go the wrong way.
 	     * This can result in a server crash because the rendering
 	     * routines in this file deal directly with cpu addresses
 	     * of pixels to be stored, and do not clip or otherwise check
 	     * that all such addresses are within their respective pixmaps.
 	     * So we only allow the RECT_IN_REGION test to be used for
 	     * values that can be expressed correctly in a signed short.
 	     */
 	    x2 = box.x1 + (int)arc->width + 1;
 	    box.x2 = x2;
 	    y2 = box.y1 + (int)arc->height + 1;
 	    box.y2 = y2;
 	    if ( (x2 <= SHRT_MAX) && (y2 <= SHRT_MAX) &&
 		    (RECT_IN_REGION(pDraw->pScreen, cclip, &box) == rgnIN) )
	    {
		if ((arc->angle2 >= FULLCIRCLE) ||
		    (arc->angle2 <= -FULLCIRCLE))
		    RROP_NAME(cfbFillEllipseSolid)(pDraw, pGC, arc);
		else
		    RROP_NAME(cfbFillArcSliceSolid)(pDraw, pGC, arc);
		continue;
	    }
	}
	miPolyFillArc(pDraw, pGC, 1, arc);
    }
}
