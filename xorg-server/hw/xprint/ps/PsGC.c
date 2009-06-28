/*

Copyright 1996, 1998  The Open Group

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

*/
/*
 * (c) Copyright 1996 Hewlett-Packard Company
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996 Sun Microsystems, Inc.
 * (c) Copyright 1996 Novell, Inc.
 * (c) Copyright 1996 Digital Equipment Corp.
 * (c) Copyright 1996 Fujitsu Limited
 * (c) Copyright 1996 Hitachi, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the names of the copyright holders
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from said copyright holders.
 */

/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PsGC.c
**    *
**    *  Contents:	Graphics Context handling for the PS driver
**    *
**    *  Created By:	Roger Helmendach (Liberty Systems)
**    *
**    *  Copyright:	Copyright 1996 The Open Group, Inc.
**    *
**    *********************************************************
** 
********************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "Ps.h"
#include "gcstruct.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "migc.h"
#include "scrnintstr.h"

static GCOps PsGCOps = 
{
  PsFillSpans,
  PsSetSpans,
  PsPutImage,
  PsCopyArea,
  PsCopyPlane,
  PsPolyPoint,
  PsPolyLine,
  PsPolySegment,
  PsPolyRectangle,
  PsPolyArc,
  PsFillPolygon,
  PsPolyFillRect,
  PsPolyFillArc,
  PsPolyText8,
  PsPolyText16,
  PsImageText8,
  PsImageText16,
  PsImageGlyphBlt,
  PsPolyGlyphBlt,
  PsPushPixels
};


static GCFuncs PsGCFuncs = 
{
  PsValidateGC,
  PsChangeGC,
  PsCopyGC,
  PsDestroyGC,
  PsChangeClip,
  PsDestroyClip,
  PsCopyClip
};

Bool
PsCreateGC(pGC)
  GCPtr pGC;
{
  pGC->clientClip     = NULL;
  pGC->clientClipType = CT_NONE;

  pGC->ops = &PsGCOps;
  pGC->funcs = &PsGCFuncs;

  pGC->clientClip = (pointer)xalloc(sizeof(PsClipRec));
  memset(pGC->clientClip, 0, sizeof(PsClipRec));
  return TRUE;
}

static int
PsGetDrawablePrivateStuff(
  DrawablePtr     pDrawable,
  GC             *gc,
  unsigned long  *valid,
  PsOutPtr       *psOut,
  ColormapPtr    *cMap)
{
  XpContextPtr     pCon;
  PsContextPrivPtr cPriv;
  PsScreenPrivPtr  sPriv;

  switch(pDrawable->type)
  {
    case DRAWABLE_PIXMAP:
      return FALSE;
    case DRAWABLE_WINDOW:
      pCon  = PsGetContextFromWindow((WindowPtr)pDrawable);
      if( pCon==NULL ) return FALSE;
      else
      {
        Colormap    c;
        ColormapPtr cmap;

        c = wColormap((WindowPtr)pDrawable);
        cmap = (ColormapPtr)LookupIDByType(c, RT_COLORMAP);

        cPriv = (PsContextPrivPtr)
	    dixLookupPrivate(&pCon->devPrivates, PsContextPrivateKey);
        sPriv = (PsScreenPrivPtr)
	    dixLookupPrivate(&pDrawable->pScreen->devPrivates,
			     PsScreenPrivateKey);
        *gc     = cPriv->lastGC;
        *valid  = cPriv->validGC;
        *psOut  = cPriv->pPsOut;
        *cMap   = cmap;
        return TRUE;
      }
    default:
      return FALSE;
  }
}

PsContextPrivPtr
PsGetPsContextPriv( DrawablePtr pDrawable )
{
  XpContextPtr     pCon;

  switch(pDrawable->type)
  {
    case DRAWABLE_PIXMAP:
      return FALSE;
    case DRAWABLE_WINDOW:
      pCon = PsGetContextFromWindow((WindowPtr)pDrawable);
      if (pCon != NULL)
      {
	  return (PsContextPrivPtr)
	      dixLookupPrivate(&pCon->devPrivates, PsContextPrivateKey);
      }
  }
  return NULL;
}

int
PsUpdateDrawableGC(
  GCPtr        pGC,
  DrawablePtr  pDrawable,
  PsOutPtr    *psOut,
  ColormapPtr *cMap)
{
  GC               dGC;
  unsigned long    valid;
  int              i;
  PsContextPrivPtr cPriv;
  BoxPtr           boxes;

  if (!PsGetDrawablePrivateStuff(pDrawable, &dGC, &valid, psOut, cMap))
    return FALSE;
    
  switch (pDrawable->type) {

    case DRAWABLE_PIXMAP:
      /* we don't support pixmaps yet! */
      return FALSE;
      break;
    case DRAWABLE_WINDOW: 
      if( pGC )
      {
        RegionPtr pReg;
        WindowPtr pWin = (WindowPtr)pDrawable;
        Bool      freeClip;
        PsClipPtr clp = (PsClipPtr)pGC->clientClip;
        if( clp->outterClips )
          { xfree(clp->outterClips); clp->outterClips = 0; }
        clp->nOutterClips = 0;
        if( pGC->subWindowMode==IncludeInferiors )
        {
          pReg = NotClippedByChildren(pWin);
          freeClip = TRUE;
        }
        else
        {
          pReg = &pWin->clipList;
          freeClip = FALSE;
        }

        if( pReg->data )
        {
          boxes = (BoxPtr)((char *)pReg->data+sizeof(long)*2);
          clp->nOutterClips = pReg->data->numRects;
          clp->outterClips  =
                      (PsRectPtr)xalloc(clp->nOutterClips*sizeof(PsRectRec));
          for( i=0 ; i<clp->nOutterClips ; i++ )
          {
            clp->outterClips[i].x = boxes[i].x1;
            clp->outterClips[i].y = boxes[i].y1;
            clp->outterClips[i].w = (boxes[i].x2-boxes[i].x1)+1;
            clp->outterClips[i].h = (boxes[i].y2-boxes[i].y1)+1;
          }
        }

        if( freeClip ) REGION_DESTROY(pGC->pScreen, pReg);
        PsOut_Offset(*psOut, pDrawable->x, pDrawable->y);
        PsOut_Clip(*psOut, pGC->clientClipType, (PsClipPtr)pGC->clientClip);
      }
      cPriv = (PsContextPrivPtr)dixLookupPrivate(
	  &PsGetContextFromWindow((WindowPtr)pDrawable)->devPrivates,
	  PsContextPrivateKey);
      break;
  }
  return TRUE;
}

void
PsValidateGC(GCPtr pGC, unsigned long changes, DrawablePtr pDrawable)
{
  pGC->ops = &PsGCOps;
}

void
PsChangeGC(GCPtr pGC, unsigned long changes)
{
}

void
PsCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst)
{
}

void
PsDestroyGC(GCPtr pGC)
{
  PsDestroyClip(pGC);
  xfree(pGC->clientClip);
}

void
PsChangeClip(GCPtr pGC, int type, pointer pValue, int nrects)
{
  int         i;
  PsClipPtr   clp = (PsClipPtr)pGC->clientClip;
  RegionPtr   rgn;
  BoxPtr      boxes;
  xRectangle *rects;

  PsDestroyClip(pGC);
  pGC->clientClipType = type;
  switch(type)
  {
    case CT_NONE: break;
    case CT_PIXMAP:
      clp->elms = PsCreateFillElementList((PixmapPtr)pValue, &clp->nElms);
      (*pGC->pScreen->DestroyPixmap)((PixmapPtr)pValue);
      break;
    case CT_REGION:
      rgn = (RegionPtr)pValue;
      boxes = (BoxPtr)((char *)rgn->data+sizeof(long)*2);
      clp->nRects = rgn->data->numRects;
      clp->rects  = (PsRectPtr)xalloc(clp->nRects*sizeof(PsRectRec));
      for( i=0 ; i<clp->nRects ; i++ )
      {
        clp->rects[i].x = boxes[i].x1;
        clp->rects[i].y = boxes[i].y1;
        clp->rects[i].w = (boxes[i].x2-boxes[i].x1)+1;
        clp->rects[i].h = (boxes[i].y2-boxes[i].y1)+1;
      }
      REGION_DESTROY(pGC->pScreen, (RegionPtr)pValue);
      break;
    case CT_UNSORTED:
    case CT_YSORTED:
    case CT_YXSORTED:
    case CT_YXBANDED:
      rects = (xRectangle *)pValue;
      clp->nRects = nrects;
      clp->rects  = (PsRectPtr)xalloc(clp->nRects*sizeof(PsRectRec));
      for( i=0 ; i<clp->nRects ; i++ )
      {
        clp->rects[i].x = rects[i].x;
        clp->rects[i].y = rects[i].y;
        clp->rects[i].w = rects[i].width;
        clp->rects[i].h = rects[i].height;
      }
      xfree(pValue);
      break;
  }
}

void
PsDestroyClip(GCPtr pGC)
{
  PsClipPtr clp = (PsClipPtr)pGC->clientClip;

  if( clp->rects )       xfree(clp->rects);
  if( clp->outterClips ) xfree(clp->outterClips);
  clp->rects       = (PsRectPtr)0;
  clp->outterClips = (PsRectPtr)0;
  clp->nRects       = 0;
  clp->nOutterClips = 0;
  if( clp->elms ) PsDestroyFillElementList(clp->nElms, clp->elms);
  clp->elms   = (PsElmPtr)0;
  clp->nElms  = 0;
  pGC->clientClipType = CT_NONE;
}

void
PsCopyClip(GCPtr pDst, GCPtr pSrc)
{
  PsClipPtr src = (PsClipPtr)pSrc->clientClip;
  PsClipPtr dst = (PsClipPtr)pDst->clientClip;

  PsDestroyClip(pDst);
  pDst->clientClipType = pSrc->clientClipType;
  *dst = *src;
  if( src->rects )
  {
    dst->rects = (PsRectPtr)xalloc(src->nRects*sizeof(PsRectRec));
    memcpy(dst->rects, src->rects, src->nRects*sizeof(PsRectRec));
  }
  if( src->elms )
    dst->elms = PsCloneFillElementList(src->nElms, src->elms);
}


GCPtr
PsCreateAndCopyGC(DrawablePtr pDrawable, GCPtr pSrc)
{
    GCPtr pDst;
    
    if (pSrc == NULL) {
        /* https://freedesktop.org/bugzilla/show_bug.cgi?id=1416 ("'x11perf
         * -copypixpix500' crashes Xprt's PostScript DDX [PsCreateAndCopyGC"):
         * I have no clue whether this is the real fix or just wallpapering
         * over the crash (that's why we warn here loudly when this
         * happens) ... */
        fprintf(stderr, "PsCreateAndCopyGC: pSrc == NULL\n");
        return NULL;
    }
    
    if ((pDst =
	 CreateScratchGC(pDrawable->pScreen, pDrawable->depth)) == NULL) 
    {
	return NULL;
    }

    if (CopyGC(pSrc, pDst, 
	       GCFunction | GCPlaneMask | GCForeground | GCBackground |
	       GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle | 
	       GCFillStyle | GCFillRule | GCTile | GCStipple |
	       GCTileStipXOrigin | GCTileStipYOrigin | GCFont | 
	       GCSubwindowMode | GCGraphicsExposures | GCClipXOrigin |
	       GCClipYOrigin | GCClipMask | GCDashOffset | GCDashList |
	       GCArcMode) != Success)
    {
	(void)FreeGC(pDst, (GContext)0);

	return NULL;
    }
    
    return pDst;
}

