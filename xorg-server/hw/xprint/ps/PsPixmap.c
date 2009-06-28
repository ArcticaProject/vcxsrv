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
**    *  File:		PsPixmap.c
**    *
**    *  Contents:	Pixmap functions for the PS DDX driver
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

#include "windowstr.h"
#include "gcstruct.h"
#include "privates.h"

#include "Ps.h"

#define _BitsPerPixel(d) (\
  (1 << PixmapWidthPaddingInfo[d].padBytesLog2) * 8 / \
  (PixmapWidthPaddingInfo[d].padRoundUp+1))

PixmapPtr
PsCreatePixmap(
  ScreenPtr pScreen,
  int       width,
  int       height,
  int       depth,
  unsigned  usage_hint)
{
  PixmapPtr pPixmap;

  pPixmap = (PixmapPtr)xcalloc(1, sizeof(PixmapRec));
  if( !pPixmap)  return NullPixmap;
  pPixmap->drawable.type         = DRAWABLE_PIXMAP;
  pPixmap->drawable.class        = 0;
  pPixmap->drawable.pScreen      = pScreen;
  pPixmap->drawable.depth        = depth;
  pPixmap->drawable.bitsPerPixel = _BitsPerPixel(depth);
  pPixmap->drawable.id           = 0;
  pPixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
  pPixmap->drawable.x            = 0;
  pPixmap->drawable.y            = 0;
  pPixmap->drawable.width        = width;
  pPixmap->drawable.height       = height;
  pPixmap->devKind               = 0;
  pPixmap->refcnt                = 1;
  pPixmap->devPrivates		 = NULL;

  pPixmap->devPrivate.ptr = (PsPixmapPrivPtr)xcalloc(1, sizeof(PsPixmapPrivRec));
  if( !pPixmap->devPrivate.ptr )
    { xfree(pPixmap); return NullPixmap; }
  return pPixmap;
}

/* PsScrubPixmap: Remove all content from a pixmap (used by
 * |PsPolyFillRect()| when the "solid fill" operation covers
 * the whole pixmap) */
void
PsScrubPixmap(PixmapPtr pPixmap)
{
  PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pPixmap->devPrivate.ptr;
  DisplayListPtr  disp = priv->dispList;

  while( disp )
  {
    int            i;
    DisplayListPtr oldDisp = disp;
    disp = disp->next;
    for( i=0 ; i<oldDisp->nelms ; i++ )
    {
      DisplayElmPtr elm = &oldDisp->elms[i];

      switch(elm->type)
      {
        case PolyPointCmd:
        case PolyLineCmd:
          if( elm->c.polyPts.pPoints ) xfree(elm->c.polyPts.pPoints);
          break;
        case PolySegmentCmd:
          if( elm->c.segments.pSegments ) xfree(elm->c.segments.pSegments);
          break;
        case PolyRectangleCmd:
          if( elm->c.rects.pRects ) xfree(elm->c.rects.pRects);
          break;
        case FillPolygonCmd:
          if( elm->c.polyPts.pPoints ) xfree(elm->c.polyPts.pPoints);
          break;
        case PolyFillRectCmd:
          if( elm->c.rects.pRects ) xfree(elm->c.rects.pRects);
          break;
        case PolyArcCmd:
          if( elm->c.arcs.pArcs ) xfree(elm->c.arcs.pArcs);
          break;
        case PolyFillArcCmd:
          if( elm->c.arcs.pArcs ) xfree(elm->c.arcs.pArcs);
          break;
        case Text8Cmd:
        case TextI8Cmd:
          if( elm->c.text8.string ) xfree(elm->c.text8.string);
          break;
        case Text16Cmd:
        case TextI16Cmd:
          if( elm->c.text16.string ) xfree(elm->c.text16.string);
          break;
        case PutImageCmd:
          if( elm->c.image.pData ) xfree(elm->c.image.pData);
          break;
        case BeginFrameCmd:
          break;
        case EndFrameCmd:
          break;
      }

      if (elm->type != BeginFrameCmd && elm->type != EndFrameCmd) {
	  (void) FreeGC(elm->gc, (GContext) 0);
      }
    }
    xfree(oldDisp);
  }

  priv->dispList = NULL;
}

Bool
PsDestroyPixmap(PixmapPtr pPixmap)
{
  PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pPixmap->devPrivate.ptr;

  if( --pPixmap->refcnt ) return TRUE;

  PsScrubPixmap(pPixmap);

  xfree(priv);
  dixFreePrivates(pPixmap->devPrivates);
  xfree(pPixmap);
  return TRUE;
}

DisplayListPtr
PsGetFreeDisplayBlock(PsPixmapPrivPtr priv)
{
  DisplayListPtr disp = priv->dispList;

  for(; disp ; disp=disp->next )
  {
    if( disp->nelms>=DPY_BLOCKSIZE && disp->next ) continue;
    if( disp->nelms<DPY_BLOCKSIZE ) return(disp);
    disp->next = (DisplayListPtr)xcalloc(1, sizeof(DisplayListRec));
    disp->next->next  = (DisplayListPtr)0;
    disp->next->nelms = 0;
  }
  disp = (DisplayListPtr)xcalloc(1, sizeof(DisplayListRec));
  disp->next     = (DisplayListPtr)0;
  disp->nelms    = 0;
  priv->dispList = disp;
  return(disp);
}

void
PsReplay(DisplayElmPtr elm, DrawablePtr pDrawable)
{
  switch(elm->type)
  {
    case PolyPointCmd:
      PsPolyPoint(pDrawable, elm->gc, elm->c.polyPts.mode,
                 elm->c.polyPts.nPoints, elm->c.polyPts.pPoints);
      break;
    case PolyLineCmd:
      PsPolyLine(pDrawable, elm->gc, elm->c.polyPts.mode,
                 elm->c.polyPts.nPoints, elm->c.polyPts.pPoints);
      break;
    case PolySegmentCmd:
      PsPolySegment(pDrawable, elm->gc, elm->c.segments.nSegments,
                    elm->c.segments.pSegments);
      break;
    case PolyRectangleCmd:
      PsPolyRectangle(pDrawable, elm->gc, elm->c.rects.nRects,
                      elm->c.rects.pRects);
      break;
    case FillPolygonCmd:
      PsFillPolygon(pDrawable, elm->gc, 0, elm->c.polyPts.mode,
                    elm->c.polyPts.nPoints, elm->c.polyPts.pPoints);
      break;
    case PolyFillRectCmd:
      PsPolyFillRect(pDrawable, elm->gc, elm->c.rects.nRects,
                     elm->c.rects.pRects);
      break;
    case PolyArcCmd:
      PsPolyArc(pDrawable, elm->gc, elm->c.arcs.nArcs, elm->c.arcs.pArcs);
      break;
    case PolyFillArcCmd:
      PsPolyFillArc(pDrawable, elm->gc, elm->c.arcs.nArcs, elm->c.arcs.pArcs);
      break;
    case Text8Cmd:
      PsPolyText8(pDrawable, elm->gc, elm->c.text8.x, elm->c.text8.y,
                  elm->c.text8.count, elm->c.text8.string);
      break;
    case Text16Cmd:
      PsPolyText16(pDrawable, elm->gc, elm->c.text16.x, elm->c.text16.y,
                   elm->c.text16.count, elm->c.text16.string);
      break;
    case TextI8Cmd:
      PsImageText8(pDrawable, elm->gc, elm->c.text8.x, elm->c.text8.y,
                   elm->c.text8.count, elm->c.text8.string);
      break;
    case TextI16Cmd:
      PsImageText16(pDrawable, elm->gc, elm->c.text16.x, elm->c.text16.y,
                    elm->c.text16.count, elm->c.text16.string);
      break;
    case PutImageCmd:
      PsPutScaledImage(pDrawable, elm->gc, elm->c.image.depth,
		       elm->c.image.x, elm->c.image.y,
		       elm->c.image.w, elm->c.image.h, elm->c.image.leftPad,
		       elm->c.image.format, elm->c.image.res,
		       elm->c.image.pData);
      break;
    case BeginFrameCmd:
      {
        PsOutPtr     psOut;
        ColormapPtr  cMap;
        if( PsUpdateDrawableGC(NULL, pDrawable, &psOut, &cMap)==FALSE ) return;
        PsOut_BeginFrame(psOut, 0, 0, elm->c.frame.x, elm->c.frame.y,
                         elm->c.frame.w, elm->c.frame.h);
      }
      break;
    case EndFrameCmd:
      {
        PsOutPtr     psOut;
        ColormapPtr  cMap;
        if( PsUpdateDrawableGC(NULL, pDrawable, &psOut, &cMap)==FALSE ) return;
        PsOut_EndFrame(psOut);
      }
      break;
  }
}

void
PsReplayPixmap(PixmapPtr pix, DrawablePtr pDrawable)
{
  PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
  DisplayListPtr  disp = priv->dispList;
  DisplayElmPtr   elm;

  for(; disp ; disp=disp->next )
  {
    int  i;
    for( i=0,elm=disp->elms ; i<disp->nelms ; i++,elm++ )
      PsReplay(elm, pDrawable);
  }
}

int
PsCloneDisplayElm(PixmapPtr dst, DisplayElmPtr elm, DisplayElmPtr newElm, 
		  int xoff, int yoff)
{
  int           i;
  int           size;
  int           status = 0;

  *newElm = *elm;

  /* I think this is the correct return value */
  if ((newElm->gc = PsCreateAndCopyGC(&dst->drawable, elm->gc)) == NULL) {
      return 1;
  }

  switch(elm->type)
  {
    case PolyPointCmd:
    case PolyLineCmd:
      newElm->c.polyPts.pPoints =
        (xPoint *)xalloc(elm->c.polyPts.nPoints*sizeof(xPoint));
      for( i=0 ; i<elm->c.polyPts.nPoints ; i++ )
      {
        newElm->c.polyPts.pPoints[i].x = elm->c.polyPts.pPoints[i].x+xoff;
        newElm->c.polyPts.pPoints[i].y = elm->c.polyPts.pPoints[i].y+yoff;
      }
      break;
    case PolySegmentCmd:
      newElm->c.segments.pSegments =
        (xSegment *)xalloc(elm->c.segments.nSegments*sizeof(xSegment));
      for( i=0 ; i<elm->c.segments.nSegments ; i++ )
      {
        newElm->c.segments.pSegments[i].x1 =
           elm->c.segments.pSegments[i].x1+xoff;
        newElm->c.segments.pSegments[i].y1 =
           elm->c.segments.pSegments[i].y1+yoff;
        newElm->c.segments.pSegments[i].x2 =
           elm->c.segments.pSegments[i].x2+xoff;
        newElm->c.segments.pSegments[i].y2 =
           elm->c.segments.pSegments[i].y2+yoff;
      }
      break;
    case PolyRectangleCmd:
      newElm->c.rects.pRects =
        (xRectangle *)xalloc(elm->c.rects.nRects*sizeof(xRectangle));
      for( i=0 ; i<elm->c.rects.nRects ; i++ )
      {
        newElm->c.rects.pRects[i].x      = elm->c.rects.pRects[i].x+xoff;
        newElm->c.rects.pRects[i].y      = elm->c.rects.pRects[i].y+yoff;
        newElm->c.rects.pRects[i].width  = elm->c.rects.pRects[i].width;
        newElm->c.rects.pRects[i].height = elm->c.rects.pRects[i].height;
      }
      break;
    case FillPolygonCmd:
      newElm->c.polyPts.pPoints =
        (xPoint *)xalloc(elm->c.polyPts.nPoints*sizeof(xPoint));
      for( i=0 ; i<elm->c.polyPts.nPoints ; i++ )
      {
        newElm->c.polyPts.pPoints[i].x = elm->c.polyPts.pPoints[i].x+xoff;
        newElm->c.polyPts.pPoints[i].y = elm->c.polyPts.pPoints[i].y+yoff;
      }
      break;
    case PolyFillRectCmd:
      newElm->c.rects.pRects =
        (xRectangle *)xalloc(elm->c.rects.nRects*sizeof(xRectangle));
      for( i=0 ; i<elm->c.rects.nRects ; i++ )
      {
        newElm->c.rects.pRects[i].x      = elm->c.rects.pRects[i].x+xoff;
        newElm->c.rects.pRects[i].y      = elm->c.rects.pRects[i].y+yoff;
        newElm->c.rects.pRects[i].width  = elm->c.rects.pRects[i].width;
        newElm->c.rects.pRects[i].height = elm->c.rects.pRects[i].height;
      }
      break;
    case PolyArcCmd:
      newElm->c.arcs.pArcs =
        (xArc *)xalloc(elm->c.arcs.nArcs*sizeof(xArc));
      for( i=0 ; i<elm->c.arcs.nArcs ; i++ )
      {
        newElm->c.arcs.pArcs[i].x      = elm->c.arcs.pArcs[i].x+xoff;
        newElm->c.arcs.pArcs[i].y      = elm->c.arcs.pArcs[i].y+yoff;
        newElm->c.arcs.pArcs[i].width  = elm->c.arcs.pArcs[i].width;
        newElm->c.arcs.pArcs[i].height = elm->c.arcs.pArcs[i].height;
        newElm->c.arcs.pArcs[i].angle1 = elm->c.arcs.pArcs[i].angle1;
        newElm->c.arcs.pArcs[i].angle2 = elm->c.arcs.pArcs[i].angle2;
      }
      break;
    case PolyFillArcCmd:
      newElm->c.arcs.pArcs =
        (xArc *)xalloc(elm->c.arcs.nArcs*sizeof(xArc));
      for( i=0 ; i<elm->c.arcs.nArcs ; i++ )
      {
        newElm->c.arcs.pArcs[i].x      = elm->c.arcs.pArcs[i].x+xoff;
        newElm->c.arcs.pArcs[i].y      = elm->c.arcs.pArcs[i].y+yoff;
        newElm->c.arcs.pArcs[i].width  = elm->c.arcs.pArcs[i].width;
        newElm->c.arcs.pArcs[i].height = elm->c.arcs.pArcs[i].height;
        newElm->c.arcs.pArcs[i].angle1 = elm->c.arcs.pArcs[i].angle1;
        newElm->c.arcs.pArcs[i].angle2 = elm->c.arcs.pArcs[i].angle2;
      }
      break;
    case Text8Cmd:
    case TextI8Cmd:
      newElm->c.text8.string = (char *)xalloc(elm->c.text8.count);
      memcpy(newElm->c.text8.string, elm->c.text8.string, elm->c.text8.count);
      newElm->c.text8.x += xoff;
      newElm->c.text8.y += yoff;
      break;
    case Text16Cmd:
    case TextI16Cmd:
      newElm->c.text16.string =
        (unsigned short *)xalloc(elm->c.text16.count*sizeof(unsigned short));
      memcpy(newElm->c.text16.string, elm->c.text16.string,
             elm->c.text16.count*sizeof(unsigned short));
      newElm->c.text16.x += xoff;
      newElm->c.text16.y += yoff;
      break;
    case PutImageCmd:
      size = PixmapBytePad(elm->c.image.w, elm->c.image.depth)*elm->c.image.h;
      newElm->c.image.pData = (char *)xalloc(size);
      memcpy(newElm->c.image.pData, elm->c.image.pData, size);
      newElm->c.image.x += xoff;
      newElm->c.image.y += yoff;
      break;
    case BeginFrameCmd:
    case EndFrameCmd:
      status = 1;
      break;
  }
  return(status);
}

void
PsCopyDisplayList(PixmapPtr src, PixmapPtr dst, int xoff, int yoff,
                  int x, int y, int w, int h)
{
  PsPixmapPrivPtr sPriv = (PsPixmapPrivPtr)src->devPrivate.ptr;
  PsPixmapPrivPtr dPriv = (PsPixmapPrivPtr)dst->devPrivate.ptr;
  DisplayListPtr  sDisp;
  DisplayListPtr  dDisp = PsGetFreeDisplayBlock(dPriv);
  DisplayElmPtr   elm   = &dDisp->elms[dDisp->nelms];

  elm->type = BeginFrameCmd;
  elm->c.frame.x = x;
  elm->c.frame.y = y;
  elm->c.frame.w = w;
  elm->c.frame.h = h;
  dDisp->nelms += 1;

  sDisp = sPriv->dispList;
  for(; sDisp ; sDisp=sDisp->next )
  {
    int  i;
    for( i=0,elm=sDisp->elms ; i<sDisp->nelms ; i++,elm++ )
    {
      dDisp = PsGetFreeDisplayBlock(dPriv);
      if (PsCloneDisplayElm(dst, elm, &dDisp->elms[dDisp->nelms],
			    xoff, yoff)==0)
      {
	  dDisp->nelms += 1;
      }
    }
  }

  dDisp = PsGetFreeDisplayBlock(dPriv);
  elm   = &dDisp->elms[dDisp->nelms];
  elm->type = EndFrameCmd;
  dDisp->nelms += 1;
}

PsElmPtr
PsCreateFillElementList(PixmapPtr pix, int *nElms)
{
  PsElmPtr        elms = (PsElmPtr)0;
  PsPixmapPrivPtr priv = (PsPixmapPrivPtr)pix->devPrivate.ptr;
  DisplayListPtr  disp = priv->dispList;
  PsArcEnum       styl;

  *nElms = 0;
  for(; disp ; disp=disp->next )
  {
    int           i;
    DisplayElmPtr elm = disp->elms;

    for( i=0 ; i<disp->nelms ; i++,elm++ )
    {
      if( !elm->gc ) continue; /* workaround for https://freedesktop.org/bugzilla/show_bug.cgi?id=1416 */
      if( !elm->gc->fgPixel ) continue;
      switch(elm->type)
      {
        case FillPolygonCmd:
          *nElms += 1;
          break;
        case PolyFillRectCmd:
          *nElms += elm->c.rects.nRects;
          break;
        case PolyFillArcCmd:
          *nElms += elm->c.arcs.nArcs;
          break;
        default: /* keep the compiler happy with unhandled enums */
          break;
      }
    }
  }

  if( (*nElms) )
  {
    elms = (PsElmPtr)xcalloc(1, (*nElms)*sizeof(PsElmRec));
    if( elms )
    {
      disp = priv->dispList;
      *nElms = 0;
      for(; disp ; disp=disp->next )
      {
        int           i, k;
        DisplayElmPtr elm = disp->elms;

        for( i=0 ; i<disp->nelms ; i++,elm++ )
        {
          if( !elm->gc->fgPixel ) continue;
          switch(elm->type)
          {
            case FillPolygonCmd:
              elms[*nElms].type     = PSOUT_POINTS;
              elms[*nElms].nPoints  = elm->c.polyPts.nPoints;
              elms[*nElms].c.points =
                  (PsPointPtr)xalloc(elms[*nElms].nPoints*sizeof(PsPointRec));
              for( k=0 ; k<elms[*nElms].nPoints ; k++ )
              {
                elms[*nElms].c.points[k].x = elm->c.polyPts.pPoints[k].x;
                elms[*nElms].c.points[k].y = elm->c.polyPts.pPoints[k].y;
              }
              *nElms += 1;
              break;
            case PolyFillRectCmd:
              for( k=0 ; k<elm->c.rects.nRects ; k++ )
              {
                elms[*nElms].type = PSOUT_RECT;
                elms[*nElms].nPoints  = 0;
                elms[*nElms].c.rect.x = elm->c.rects.pRects[k].x;
                elms[*nElms].c.rect.y = elm->c.rects.pRects[k].y;
                elms[*nElms].c.rect.w = elm->c.rects.pRects[k].width;
                elms[*nElms].c.rect.h = elm->c.rects.pRects[k].height;
                *nElms += 1;
              }
              break;
            case PolyFillArcCmd:
              if( elm->gc->arcMode==ArcChord ) styl = PsChord;
              else                            styl = PsPieSlice;
              for( k=0 ; k<elm->c.rects.nRects ; k++ )
              {
                elms[*nElms].type = PSOUT_ARC;
                elms[*nElms].nPoints     = 0;
                elms[*nElms].c.arc.x     = elm->c.arcs.pArcs[k].x;
                elms[*nElms].c.arc.y     = elm->c.arcs.pArcs[k].y;
                elms[*nElms].c.arc.w     = elm->c.arcs.pArcs[k].width;
                elms[*nElms].c.arc.h     = elm->c.arcs.pArcs[k].height;
                elms[*nElms].c.arc.a1    = elm->c.arcs.pArcs[k].angle1;
                elms[*nElms].c.arc.a2    = elm->c.arcs.pArcs[k].angle2;
                elms[*nElms].c.arc.style = styl;
                *nElms += 1;
              }
              break;
            default:  /* keep the compiler happy with unhandled enums */
              break;
          }
        }
      }
    }
  }
  return(elms);
}

PsElmPtr
PsCloneFillElementList(int nElms, PsElmPtr elms)
{
  int      i;
  PsElmPtr newElms;

  newElms = (PsElmPtr)xcalloc(1, nElms*sizeof(PsElmRec));
  if( !newElms ) return(newElms);
  for( i=0 ; i<nElms ; i++ )
  {
    newElms[i] = elms[i];

    if( elms[i].type==PSOUT_POINTS )
    {
      newElms[i].c.points =
             (PsPointPtr)xalloc(elms[i].nPoints*sizeof(PsElmRec));
      memcpy(newElms[i].c.points, elms[i].c.points,
             elms[i].nPoints*sizeof(PsPointRec));
    }
  }
  return(newElms);
}

void
PsDestroyFillElementList(int nElms, PsElmPtr elms)
{
  int  i;

  for( i=0 ; i<nElms ; i++ )
    { if( elms[i].type==PSOUT_POINTS ) xfree(elms[i].c.points); }

  xfree(elms);
}
