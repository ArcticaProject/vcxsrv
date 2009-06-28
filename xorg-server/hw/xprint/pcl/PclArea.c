/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclArea.c
**    *
**    *  Contents:
**    *                 Image and Area functions for the PCL DDX driver
**    *
**    *  Created:	10/23/95
**    *
**    *********************************************************
**
********************************************************************/
/*
(c) Copyright 1996 Hewlett-Packard Company
(c) Copyright 1996 International Business Machines Corp.
(c) Copyright 1996 Sun Microsystems, Inc.
(c) Copyright 1996 Novell, Inc.
(c) Copyright 1996 Digital Equipment Corp.
(c) Copyright 1996 Fujitsu Limited
(c) Copyright 1996 Hitachi, Ltd.

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
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of the copyright holders shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from said
copyright holders.
*/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Pcl.h"
#include "pixmapstr.h"
#include "region.h"

#include "fb.h"

void
PclPutImage(DrawablePtr pDrawable,
	    GCPtr pGC,
	    int depth,
	    int x,
	    int y,
	    int w,
	    int h,
	    int leftPad,
	    int format,
	    char *pImage)
{
    PixmapPtr pPixmap;
    unsigned long oldFg, oldBg;
    XID gcv[3];
    unsigned long oldPlanemask;
    unsigned long i;
    long bytesPer;

    if( ( w == 0 ) || ( h == 0 ) )
      return;

    if( format != XYPixmap )
      {
	  pPixmap = GetScratchPixmapHeader( pDrawable->pScreen,
					   w+leftPad, h, depth,
					   BitsPerPixel( depth ),
					   PixmapBytePad( w + leftPad,
					   depth ), (pointer)pImage );
	  if( !pPixmap )
	    return;

	  if( format == ZPixmap )
	    (void)(*pGC->ops->CopyArea)( (DrawablePtr)pPixmap, pDrawable, pGC,
				  leftPad, 0, w, h, x, y );
	  else
	    (void)(*pGC->ops->CopyPlane)( (DrawablePtr)pPixmap, pDrawable, pGC,
			 leftPad, 0, w, h, x, y, 1 );
	  FreeScratchPixmapHeader( pPixmap );
      }
    else
      {
	  pPixmap = GetScratchPixmapHeader( pDrawable->pScreen,
					   w+leftPad, h, depth,
					   BitsPerPixel( depth ),
					   PixmapBytePad( w + leftPad,
					   depth ), (pointer)pImage );

	  if( !pPixmap )
	    return;

	  depth = pGC->depth;
	  oldPlanemask = pGC->planemask;
	  oldFg = pGC->fgPixel;
	  oldBg = pGC->bgPixel;
	  gcv[0] = ~0L;
	  gcv[1] = 0;
	  DoChangeGC( pGC, GCForeground | GCBackground, gcv, 0 );
	  bytesPer = (long)h * BitmapBytePad( w + leftPad );

	  for( i = 1 << (depth-1); i != 0; i >>= 1, pImage += bytesPer )
	    {
		if( i & oldPlanemask )
		  {
		      gcv[0] = i;
		      DoChangeGC( pGC, GCPlaneMask, gcv, 0 );
		      ValidateGC( pDrawable, pGC );
		      fbPutImage( (DrawablePtr)pPixmap, pGC, 1, x, y, w, h,
				  leftPad, XYBitmap, pImage );
		  }
	    }
	  gcv[0] = oldPlanemask;
	  gcv[1] = oldFg;
	  gcv[2] = oldBg;
	  DoChangeGC( pGC, GCPlaneMask | GCForeground | GCBackground,
		     gcv, 0 );

	  PclCopyArea( (DrawablePtr)pPixmap, pDrawable, pGC, leftPad,
		      0, w, h, x, y );
	  FreeScratchPixmapHeader( pPixmap );
      }
}

/*
 * PclMonoPixmapFragment()
 *
 * Given a 1-bit-deep pixmap, send the appropriate part of it to the
 * output file as a PCL raster graphics command.
 */
static void
PclMonoPixmapFragment(FILE *outFile,
		      PixmapPtr pix,
		      short x1,
		      short y1,
		      short x2,
		      short y2,
		      short dstx,
		      short dsty)
{
    char *bits, t[80], *row;
    int h, w, i;

    /*
     * Create a storage area large enough to hold the entire pixmap,
     * then use fbGetImage to get the appropriate bits.
     */
    h = y2 - y1;
    w = BitmapBytePad( x2 - x1 );

    bits = (char *)xalloc( h * w );
    fbGetImage( (DrawablePtr)pix, x1, y1, x2 - x1, h,
		XYPixmap, ~0, bits );

    /*
     * Move the cursor to the appropriate place on the page.  We have
     * to jump into HP-GL/2 to do this correctly, then go back to PCL
     * for the actual drawing.
     */
    sprintf( t, "\033%%0BPU%d,%d;\033%%1A", dstx, dsty );
    SEND_PCL( outFile, t );

    /*
     * Now, wrap the raster in the appropriate PCL code.  Right now,
     * it's going to go down the wire without any compression.  That
     * will have to be good enough for the sample implementation.
     */
    sprintf( t, "\033*t300R\033*r%dT\033*r%dS\033*r1A\033*b0M",
	    h, x2 - x1 );
    SEND_PCL( outFile, t );

    sprintf( t, "\033*b%dW", w );
    for( row = bits, i = 0; i <= h; i++, row += w )
      {
	  SEND_PCL( outFile, t );
	  SEND_PCL_COUNT( outFile, row, w );
      }

    SEND_PCL( outFile, "\033*rC" );

    /*
     * Clean things up a bit
     */
    xfree( bits );
}

static void
PclColorPixmapFragment(FILE *outFile,
		       PixmapPtr pix,
		       short x1,
		       short y1,
		       short x2,
		       short y2,
		       short dstx,
		       short dsty)
{
    char *bits, t[80], *row;
    int h, w, i;

    /*
     * Create a storage area large enough to hold the entire pixmap,
     * then use fbGetImage to get the appropriate bits.
     */
    h = y2 - y1;
    w = PixmapBytePad( x2 - x1, pix->drawable.depth );

    bits = (char *)xalloc( h * w );
    fbGetImage( (DrawablePtr)pix, x1, y1, x2 - x1, h, ZPixmap, ~0, bits );

    /*
     * Move the cursor to the appropriate place on the page.  We have
     * to jump into HP-GL/2 to do this correctly, then go back to PCL
     * for the actual drawing.
     */
    sprintf( t, "\033%%0BPU%d,%d;\033%%1A", dstx, dsty );
    SEND_PCL( outFile, t );

    /*
     * Now, wrap the raster in the appropriate PCL code.  Right now,
     * it's going to go down the wire without any compression.  That
     * will have to be good enough for the sample implementation.
     */
    sprintf( t, "\033*t300R\033*r%dt%ds1A\033*b0M",
	    h, x2 - x1 );
    SEND_PCL( outFile, t );

    sprintf( t, "\033*b%dW", w );
    for( row = bits, i = 0; i < h; i++, row += w )
      {
	  SEND_PCL( outFile, t );
	  SEND_PCL_COUNT( outFile, row, w );
      }

    SEND_PCL( outFile, "\033*rC" );

    /*
     * Clean things up a bit
     */
    xfree( bits );
}

RegionPtr
PclCopyArea(DrawablePtr pSrc,
	    DrawablePtr pDst,
	    GCPtr pGC,
	    int srcx,
	    int srcy,
	    int width,
	    int height,
	    int dstx,
	    int dsty)
{
    PixmapPtr pixSrc = (PixmapPtr)pSrc;
/*
    FILE *srcFile;
    GC srcGC;
*/
    FILE *dstFile;
    GC dstGC;
    unsigned long valid;
    RegionPtr drawRegion, region, whole, ret;
    BoxRec box;
    BoxPtr prect;
    int nrect;
    void (*doFragment)(FILE *, PixmapPtr, short, short, short, short,
		       short, short );

    /*
     * Since we don't store any information on a per-window basis, we
     * can't copy from a window.
     */
    if( pSrc->type == DRAWABLE_WINDOW )
      return NULL;

    /*
     * If we're copying from a pixmap to a pixmap, we just use the
     * fb code to do the work.
     */
    if( pDst->type == DRAWABLE_PIXMAP )
      fbCopyArea( pSrc, pDst, pGC, srcx, srcy, width, height, dstx, dsty );

/*
    PclGetDrawablePrivateStuff( pSrc, &srcGC, &valid, &srcFile );
*/
    PclGetDrawablePrivateStuff( pDst, &dstGC, &valid, &dstFile );

    /*
     * If we're copying to a window, we have to do some actual
     * drawing, instead of just handing it off to fb.  Start
     * by determining the region that will be drawn.
     */
    box.x1 = srcx;
    box.y1 = srcy;
    box.x2 = srcx + width;
    box.y2 = srcy + height;
    drawRegion = REGION_CREATE( pGC->pScreen, &box, 0 );
    REGION_TRANSLATE( pGC->pScreen, drawRegion, dstx, dsty );

    region = REGION_CREATE( pGC->pScreen, NULL, 0 );
    REGION_INTERSECT( pGC->pScreen, region, drawRegion, pGC->pCompositeClip );

    /*
     * Now select the operation to be performed on each box in the
     * region.
     */
    if( pSrc->depth == 1 )
      doFragment = PclMonoPixmapFragment;
    else
      doFragment = PclColorPixmapFragment;

    /*
     * Actually draw each section of the bitmap.
     */
    nrect = REGION_NUM_RECTS( region );
    prect = REGION_RECTS( region );

    while( nrect )
      {
	  (*doFragment)( dstFile, (PixmapPtr)pSrc, prect->x1 - dstx,
			prect->y1 - dsty, prect->x2 - dstx,
			prect->y2 - dsty, prect->x1, prect->y1 );

	  nrect--;
	  prect++;
      }

    /*
     * Update the destination's GC to the source's GC.
     */
/*
    PclSetDrawablePrivateGC( pDst, srcGC );
*/

    /*
     * Determine the region that needs to be returned.  This is the
     * region of the source that falls outside the boundary of the
     * pixmap.
     */
    box.x1 = 0;
    box.y1 = 0;
    box.x2 = pixSrc->drawable.width;
    box.y2 = pixSrc->drawable.height;
    whole = REGION_CREATE( pGC->pScreen, &box, 0 );
    ret = REGION_CREATE( pGC->pScreen, NULL, 0 );

    REGION_TRANSLATE( pGC->pScreen, drawRegion, -dstx, -dsty );
    REGION_SUBTRACT( pGC->pScreen, ret, drawRegion, whole );

    /*
     * Clean up the regions
     */
    REGION_DESTROY( pGC->pScreen, drawRegion );
    REGION_DESTROY( pGC->pScreen, region );
    REGION_DESTROY( pGC->pScreen, whole );

    if( REGION_NOTEMPTY( pGC->pScreen, ret ) )
      return ret;
    else
      {
	  REGION_DESTROY( pGC->pScreen, ret );
	  return NULL;
      }
}

RegionPtr
PclCopyPlane(DrawablePtr pSrc,
	     DrawablePtr pDst,
	     GCPtr pGC,
	     int srcx,
	     int srcy,
	     int width,
	     int height,
	     int dstx,
	     int dsty,
	     unsigned long plane)
{
    RegionPtr reg;
    GCPtr scratchGC;
    PixmapPtr scratchPix;

    /*
     * Since we don't store PCL on a per-window basis, there's no good
     * way to copy from a window.
     */
    if( pSrc->type == DRAWABLE_WINDOW )
      return NULL;

    /* Copying from a pixmap to a pixmap is already implemented by fb. */
    if( pSrc->type == DRAWABLE_PIXMAP &&
       pDst->type == DRAWABLE_PIXMAP )
      fbCopyPlane( pSrc, pDst, pGC, srcx, srcy, width, height,
                   dstx, dsty, plane );

    /*
     * We can use fbCopyPlane to do the work of grabbing the plane and
     * converting it to the desired visual.  Once that's done, we already
     * know how to do a CopyArea.
     */
    scratchPix = (*pDst->pScreen->CreatePixmap)( pDst->pScreen, width,
						height, pDst->depth,
						CREATE_PIXMAP_USAGE_SCRATCH );

    scratchGC = GetScratchGC( pDst->depth, pDst->pScreen );
    CopyGC( pGC, scratchGC, ~0L );

    fbValidateGC( scratchGC, ~0L, (DrawablePtr)scratchPix );
    fbCopyPlane( pSrc, (DrawablePtr)scratchPix, scratchGC,
		 srcx, srcy, width, height, 0, 0, plane );

    reg = PclCopyArea( (DrawablePtr)scratchPix, pDst, pGC, 0, 0, width,
		      height, dstx, dsty );

    FreeScratchGC( scratchGC );

    (*pDst->pScreen->DestroyPixmap)( scratchPix );

    return reg;
}
