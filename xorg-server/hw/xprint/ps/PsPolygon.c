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
**    *  File:		PsPolygon.c
**    *
**    *  Contents:	Draws Polygons and Rectangles for the PS DDX
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
#include "windowstr.h"

void
PsPolyRectangle(
  DrawablePtr pDrawable,
  GCPtr       pGC,
  int         nRects,
  xRectangle *pRects)
{
  if( pDrawable->type==DRAWABLE_PIXMAP )
  {
    DisplayElmPtr   elm;
    PixmapPtr       pix  = (PixmapPtr)pDrawable;
    PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
    DisplayListPtr  disp;
    GCPtr           gc;

    if ((gc = PsCreateAndCopyGC(pDrawable, pGC)) == NULL) return;

    disp = PsGetFreeDisplayBlock(priv);

    elm  = &disp->elms[disp->nelms];
    elm->type = PolyRectangleCmd;
    elm->gc   = gc;
    elm->c.rects.nRects = nRects;
    elm->c.rects.pRects = (xRectangle *)xalloc(nRects*sizeof(xRectangle));
    memcpy(elm->c.rects.pRects, pRects, nRects*sizeof(xRectangle));
    disp->nelms += 1;
  }
  else
  {
    int         i;
    PsOutPtr    psOut;
    ColormapPtr cMap;

    if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) return;
    PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
    PsOut_Color(psOut, PsGetPixelColor(cMap, pGC->fgPixel));
    PsLineAttrs(psOut, pGC, cMap);
    for( i=0 ; i<nRects ; i++ )
    {
      PsOut_DrawRect(psOut, (int)pRects[i].x, (int)pRects[i].y,
                     (int)pRects[i].width, (int)pRects[i].height);
    }
  }
}

void
PsFillPolygon(
  DrawablePtr   pDrawable,
  GCPtr         pGC,
  int           shape,
  int           mode,
  int           nPoints,
  DDXPointPtr   pPoints)
{
  if( pDrawable->type==DRAWABLE_PIXMAP )
  {
    DisplayElmPtr   elm;
    PixmapPtr       pix  = (PixmapPtr)pDrawable;
    PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
    DisplayListPtr  disp;
    GCPtr           gc;

    if ((gc = PsCreateAndCopyGC(pDrawable, pGC)) == NULL) return;

    disp = PsGetFreeDisplayBlock(priv);

    elm  = &disp->elms[disp->nelms];
    elm->type = FillPolygonCmd;
    elm->gc   = gc;
    elm->c.polyPts.mode    = mode;
    elm->c.polyPts.nPoints = nPoints;
    elm->c.polyPts.pPoints = (xPoint *)xalloc(nPoints*sizeof(xPoint));
    memcpy(elm->c.polyPts.pPoints, pPoints, nPoints*sizeof(xPoint));
    disp->nelms += 1;
  }
  else
  {
    int           i;
    PsOutPtr      psOut;
    PsPointPtr    pts;
    PsRuleEnum    rule;
    ColormapPtr   cMap;

    if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) return;
    PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
    PsSetFillColor(pDrawable, pGC, psOut, cMap);
    if( pGC->fillRule==EvenOddRule ) rule = PsEvenOdd;
    else                             rule = PsNZWinding;
    PsOut_FillRule(psOut, rule);
    pts = (PsPointPtr)xalloc(sizeof(PsPointRec)*nPoints);
    if( mode==CoordModeOrigin )
    {
      for( i=0 ; i<nPoints ; i++ )
        { pts[i].x = pPoints[i].x; pts[i].y = pPoints[i].y; }
    }
    else
    {
      i = 0;
      pts[0].x = pPoints[i].x; pts[0].y = pPoints[i].y;
      for( i=1 ; i<nPoints ;  i++ )
      {
        pts[i].x = pts[i-1].x+pPoints[i].x;
        pts[i].y = pts[i-1].y+pPoints[i].y;
      }
    }
    PsOut_Polygon(psOut, nPoints, pts);
    xfree(pts);
  }
}

void
PsPolyFillRect(
  DrawablePtr pDrawable,
  GCPtr       pGC,
  int         nRects,
  xRectangle *pRects)
{
  if( pDrawable->type==DRAWABLE_PIXMAP )
  {
    DisplayElmPtr   elm;
    PixmapPtr       pix  = (PixmapPtr)pDrawable;
    PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
    DisplayListPtr  disp;
    GCPtr           gc;

#ifdef DBE
    /* Remove previous pixmap content if we render one single rect which
     * covers the whole pixmap surface (this optimisation was added for
     * the double-buffer extension ("DBE") which uses |PolyFillRect()|
     * to clear the buffer - but it makes sense in other cases, too).
     */    
    if (nRects == 1)
    {
      if ( (pRects[0].x==0) && (pRects[0].y==0) &&
           (pRects[0].width==pDrawable->width) && (pRects[0].height==pDrawable->height) &&
           (pGC->fillStyle == FillSolid) &&
           (noDbeExtension == False))
      {
#ifdef DEBUG_gismobile
        ErrorF("PsPolyFillRect: scrubbing pixmap...\n");
#endif /* DEBUG_gismobile */
        /* Remove all content from the pixmap as it would be covered
         * by the whole rect anyway */
        PsScrubPixmap((PixmapPtr)pDrawable);
      }
    }
#endif /* DBE */

    if ((gc = PsCreateAndCopyGC(pDrawable, pGC)) == NULL) return;

    disp = PsGetFreeDisplayBlock(priv);

    elm  = &disp->elms[disp->nelms];
    elm->type = PolyFillRectCmd;
    elm->gc   = gc;
    elm->c.rects.nRects = nRects;
    elm->c.rects.pRects = (xRectangle *)xalloc(nRects*sizeof(xRectangle));
    memcpy(elm->c.rects.pRects, pRects, nRects*sizeof(xRectangle));
    disp->nelms += 1;
  }
  else
  {
    int         i;
    PsOutPtr    psOut;
    ColormapPtr cMap;

    if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) return;
    PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
    PsSetFillColor(pDrawable, pGC, psOut, cMap);
    for( i=0 ; i<nRects ; i++ )
    {
      PsOut_FillRect(psOut, (int)pRects[i].x, (int)pRects[i].y,
                     (int)pRects[i].width, (int)pRects[i].height);
    }
  }
}
