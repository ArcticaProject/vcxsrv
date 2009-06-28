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
**    *  File:		PsPixel.c
**    *
**    *  Contents:	Pixel-drawing code for the PS DDX driver
**    *
**    *  Created By:	Roger Helmendach (Liberty Systems)
**    *
**    *  Copyright:	Copyright 1995 The Open Group, Inc.
**    *
**    *********************************************************
** 
********************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>

#include "windowstr.h"
#include "gcstruct.h"

#include "Ps.h"

void
PsPolyPoint(
  DrawablePtr  pDrawable,
  GCPtr        pGC,
  int          mode,
  int          nPoints,
  xPoint      *pPoints)
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
    elm->type = PolyPointCmd;
    elm->gc   = gc;
    elm->c.polyPts.mode    = mode;
    elm->c.polyPts.nPoints = nPoints;
    elm->c.polyPts.pPoints = (xPoint *)xalloc(nPoints*sizeof(xPoint));
    memcpy(elm->c.polyPts.pPoints, pPoints, nPoints*sizeof(xPoint));
    disp->nelms += 1;
  }
  else
  {
    int         i;
    PsOutPtr    psOut;
    PsPointPtr  pts;
    ColormapPtr cMap;

    if( PsUpdateDrawableGC(pGC, pDrawable, &psOut, &cMap)==FALSE ) return;
    PsOut_Offset(psOut, pDrawable->x, pDrawable->y);
    PsOut_Color(psOut, PsGetPixelColor(cMap, pGC->fgPixel));
    pts = (PsPointPtr)xalloc(sizeof(PsPointRec)*nPoints);
    if( mode==CoordModeOrigin )
    {
      for( i=0 ; i<nPoints ; i++ )
        { pts[i].x = pPoints[i].x; pts[i].y = pPoints[i].y; }
    }
    else
    {
      pts[0].x = pPoints[0].x; pts[0].y = pPoints[0].y;
      for( i=1 ; i<nPoints ;  i++ )
      {
        pts[i].x = pts[i-1].x+pPoints[i].x;
        pts[i].y = pts[i-1].y+pPoints[i].y;
      }
    }
    PsOut_Points(psOut, nPoints, pts);
    xfree(pts);
  }
}

void
PsPushPixels(
  GCPtr       pGC,
  PixmapPtr   pBitmap,
  DrawablePtr pDrawable,
  int         width,
  int         height,
  int         x,
  int         y)
{
}
