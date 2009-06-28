/*

Copyright 1993, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/


#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "scrnintstr.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "migc.h"

/* ARGSUSED */
_X_EXPORT void
miChangeGC(pGC, mask)
    GCPtr           pGC;
    unsigned long   mask;
{
    return;
}

_X_EXPORT void
miDestroyGC(pGC)
    GCPtr           pGC;
{
    if (pGC->pRotatedPixmap)
	(*pGC->pScreen->DestroyPixmap) (pGC->pRotatedPixmap);
    if (pGC->freeCompClip)
	REGION_DESTROY(pGC->pScreen, pGC->pCompositeClip);
    miDestroyGCOps(pGC->ops);
}

/*
 * create a private op array for a gc
 */

_X_EXPORT GCOpsPtr
miCreateGCOps(prototype)
    GCOpsPtr        prototype;
{
    GCOpsPtr        ret;

     /* XXX */ Must_have_memory = TRUE;
    ret = (GCOpsPtr) xalloc(sizeof(GCOps));
     /* XXX */ Must_have_memory = FALSE;
    if (!ret)
	return 0;
    *ret = *prototype;
    ret->devPrivate.val = 1;
    return ret;
}

_X_EXPORT void
miDestroyGCOps(ops)
    GCOpsPtr        ops;
{
    if (ops->devPrivate.val)
	xfree(ops);
}


_X_EXPORT void
miDestroyClip(pGC)
    GCPtr           pGC;
{
    if (pGC->clientClipType == CT_NONE)
	return;
    else if (pGC->clientClipType == CT_PIXMAP)
    {
	(*pGC->pScreen->DestroyPixmap) ((PixmapPtr) (pGC->clientClip));
    }
    else
    {
	/*
	 * we know we'll never have a list of rectangles, since ChangeClip
	 * immediately turns them into a region
	 */
	REGION_DESTROY(pGC->pScreen, pGC->clientClip);
    }
    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;
}

_X_EXPORT void
miChangeClip(pGC, type, pvalue, nrects)
    GCPtr           pGC;
    int             type;
    pointer         pvalue;
    int             nrects;
{
    (*pGC->funcs->DestroyClip) (pGC);
    if (type == CT_PIXMAP)
    {
	/* convert the pixmap to a region */
	pGC->clientClip = (pointer) BITMAP_TO_REGION(pGC->pScreen,
							(PixmapPtr) pvalue);
	(*pGC->pScreen->DestroyPixmap) (pvalue);
    }
    else if (type == CT_REGION)
    {
	/* stuff the region in the GC */
	pGC->clientClip = pvalue;
    }
    else if (type != CT_NONE)
    {
	pGC->clientClip = (pointer) RECTS_TO_REGION(pGC->pScreen, nrects,
						      (xRectangle *) pvalue,
								    type);
	xfree(pvalue);
    }
    pGC->clientClipType = (type != CT_NONE && pGC->clientClip) ? CT_REGION : CT_NONE;
    pGC->stateChanges |= GCClipMask;
}

_X_EXPORT void
miCopyClip(pgcDst, pgcSrc)
    GCPtr           pgcDst, pgcSrc;
{
    RegionPtr       prgnNew;

    switch (pgcSrc->clientClipType)
    {
      case CT_PIXMAP:
	((PixmapPtr) pgcSrc->clientClip)->refcnt++;
	/* Fall through !! */
      case CT_NONE:
	(*pgcDst->funcs->ChangeClip) (pgcDst, (int) pgcSrc->clientClipType,
				   pgcSrc->clientClip, 0);
	break;
      case CT_REGION:
	prgnNew = REGION_CREATE(pgcSrc->pScreen, NULL, 1);
	REGION_COPY(pgcDst->pScreen, prgnNew,
					(RegionPtr) (pgcSrc->clientClip));
	(*pgcDst->funcs->ChangeClip) (pgcDst, CT_REGION, (pointer) prgnNew, 0);
	break;
    }
}

/* ARGSUSED */
_X_EXPORT void
miCopyGC(pGCSrc, changes, pGCDst)
    GCPtr           pGCSrc;
    unsigned long   changes;
    GCPtr           pGCDst;
{
    return;
}

_X_EXPORT void
miComputeCompositeClip(pGC, pDrawable)
    GCPtr           pGC;
    DrawablePtr     pDrawable;
{
    ScreenPtr       pScreen;

    /* This prevents warnings about pScreen not being used. */
    pGC->pScreen = pScreen = pGC->pScreen;

    if (pDrawable->type == DRAWABLE_WINDOW)
    {
	WindowPtr       pWin = (WindowPtr) pDrawable;
	RegionPtr       pregWin;
	Bool            freeTmpClip, freeCompClip;

	if (pGC->subWindowMode == IncludeInferiors)
	{
	    pregWin = NotClippedByChildren(pWin);
	    freeTmpClip = TRUE;
	}
	else
	{
	    pregWin = &pWin->clipList;
	    freeTmpClip = FALSE;
	}
	freeCompClip = pGC->freeCompClip;

	/*
	 * if there is no client clip, we can get by with just keeping the
	 * pointer we got, and remembering whether or not should destroy (or
	 * maybe re-use) it later.  this way, we avoid unnecessary copying of
	 * regions.  (this wins especially if many clients clip by children
	 * and have no client clip.)
	 */
	if (pGC->clientClipType == CT_NONE)
	{
	    if (freeCompClip)
		REGION_DESTROY(pScreen, pGC->pCompositeClip);
	    pGC->pCompositeClip = pregWin;
	    pGC->freeCompClip = freeTmpClip;
	}
	else
	{
	    /*
	     * we need one 'real' region to put into the composite clip. if
	     * pregWin the current composite clip are real, we can get rid of
	     * one. if pregWin is real and the current composite clip isn't,
	     * use pregWin for the composite clip. if the current composite
	     * clip is real and pregWin isn't, use the current composite
	     * clip. if neither is real, create a new region.
	     */

	    REGION_TRANSLATE(pScreen, pGC->clientClip,
					 pDrawable->x + pGC->clipOrg.x,
					 pDrawable->y + pGC->clipOrg.y);

	    if (freeCompClip)
	    {
		REGION_INTERSECT(pGC->pScreen, pGC->pCompositeClip,
					    pregWin, pGC->clientClip);
		if (freeTmpClip)
		    REGION_DESTROY(pScreen, pregWin);
	    }
	    else if (freeTmpClip)
	    {
		REGION_INTERSECT(pScreen, pregWin, pregWin, pGC->clientClip);
		pGC->pCompositeClip = pregWin;
	    }
	    else
	    {
		pGC->pCompositeClip = REGION_CREATE(pScreen, NullBox, 0);
		REGION_INTERSECT(pScreen, pGC->pCompositeClip,
				       pregWin, pGC->clientClip);
	    }
	    pGC->freeCompClip = TRUE;
	    REGION_TRANSLATE(pScreen, pGC->clientClip,
					 -(pDrawable->x + pGC->clipOrg.x),
					 -(pDrawable->y + pGC->clipOrg.y));
	}
    }	/* end of composite clip for a window */
    else
    {
	BoxRec          pixbounds;

	/* XXX should we translate by drawable.x/y here ? */
	/* If you want pixmaps in offscreen memory, yes */
	pixbounds.x1 = pDrawable->x;
	pixbounds.y1 = pDrawable->y;
	pixbounds.x2 = pDrawable->x + pDrawable->width;
	pixbounds.y2 = pDrawable->y + pDrawable->height;

	if (pGC->freeCompClip)
	{
	    REGION_RESET(pScreen, pGC->pCompositeClip, &pixbounds);
	}
	else
	{
	    pGC->freeCompClip = TRUE;
	    pGC->pCompositeClip = REGION_CREATE(pScreen, &pixbounds, 1);
	}

	if (pGC->clientClipType == CT_REGION)
	{
	    if(pDrawable->x || pDrawable->y) {
	        REGION_TRANSLATE(pScreen, pGC->clientClip,
					  pDrawable->x + pGC->clipOrg.x, 
					  pDrawable->y + pGC->clipOrg.y);
	        REGION_INTERSECT(pScreen, pGC->pCompositeClip,
				pGC->pCompositeClip, pGC->clientClip);
	        REGION_TRANSLATE(pScreen, pGC->clientClip,
					  -(pDrawable->x + pGC->clipOrg.x), 
					  -(pDrawable->y + pGC->clipOrg.y));
	    } else {
	        REGION_TRANSLATE(pScreen, pGC->pCompositeClip,
					 -pGC->clipOrg.x, -pGC->clipOrg.y);
	        REGION_INTERSECT(pScreen, pGC->pCompositeClip,
				pGC->pCompositeClip, pGC->clientClip);
	        REGION_TRANSLATE(pScreen, pGC->pCompositeClip,
					 pGC->clipOrg.x, pGC->clipOrg.y);
	    }
	}
    }	/* end of composite clip for pixmap */
} /* end miComputeCompositeClip */
