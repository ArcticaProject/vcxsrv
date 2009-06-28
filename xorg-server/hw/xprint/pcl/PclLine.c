/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclLine.c
**    *
**    *  Contents:
**    *                 Line drawing routines for the PCL driver
**    *
**    *  Created:	10/11/95
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

#include "Pcl.h"
#include "gcstruct.h"
#include "windowstr.h"

/*
 * PclPolyLine()
 * PclPolySegment()
 *
 * Generates PCL code to draw a polyline, or a collection of distinct
 * line segments, clipped by the current clip region.  Since PCL
 * supports clipping to a rectangle, and the clip region is
 * represented as a collection of visible rectangles, we can draw and
 * clip the line by repeatedly drawing the complete line, clipped to
 * each rectangle in the clip region.
 *
 * Since each box in the clipping region generates approximately 30
 * bytes of PCL code, we have to have a way to avoid having a large
 * number of boxes.  The worst problem the case where the clipping
 * region is a collection of one-pixel-high boxes, perhaps arising
 * from a bitmap clip mask, or a region defined by a non-rectangular
 * polygon.
 *
 * To alleviate this problem, we create a second clipping region,
 * which consists of the union of the bounding boxes of each line
 * segment.  (Each bounding box is also increased by some amount
 * related to the current line width to allow for non-zero-width
 * lines, and for the various end and join styles.)  This region is
 * intersected with the "real" clipping region to get the region used
 * to actually clip the polyline.  This should result in a significant
 * reduction in the number of clip rectangles, as the region-handling
 * code should consolidate many of the fragments of one-pixel-high
 * rectangles into larger rectangles.
 */

void
PclPolyLine(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int mode,
     int nPoints,
     xPoint *pPoints)
{
    char t[80];
    FILE *outFile;
    int xoffset = 0, yoffset = 0;
    int nbox;
    BoxPtr pbox;
    xRectangle *drawRects, *r;
    RegionPtr drawRegion, region;
    short fudge;
    int i;
    XpContextPtr pCon;
    PclContextPrivPtr pConPriv;

    if( PclUpdateDrawableGC( pGC, pDrawable, &outFile ) == FALSE )
      return;

    pCon = PclGetContextFromWindow( (WindowPtr) pDrawable );
    pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);

    /*
     * Allocate the storage required to deal with the clipping
     * regions.
     */
    region = REGION_CREATE( pGC->pScreen, NULL, 0 );
    drawRects = (xRectangle *)
      xalloc( ( nPoints - 1 ) * sizeof( xRectangle ) );

    /*
     * Calculate the "fudge factor" based on the line width.
     * Multiplying by three seems to be a good first guess.
     * XXX I need to think of a way to test this.
     */
    fudge = 3 * pGC->lineWidth + 1;

    /*
     * Generate the PCL code to draw the polyline, by defining it as a
     * macro which uses the HP-GL/2 line drawing function.
     */

    MACRO_START( outFile, pConPriv );
    SAVE_PCL( outFile, pConPriv, "\033%0B" );

    sprintf( t, "PU%d,%dPD\n", pPoints[0].x + pDrawable->x,
	    pPoints[0].y + pDrawable->y );
    SAVE_PCL( outFile, pConPriv, t ); /* Move to the start of the polyline */

    switch( mode )
      {
	case CoordModeOrigin:
	  xoffset = pDrawable->x;
	  yoffset = pDrawable->y;
	  SAVE_PCL( outFile, pConPriv, "PA" );
	  break;
	case CoordModePrevious:
	  xoffset = yoffset = 0;
	  SAVE_PCL( outFile, pConPriv, "PR" );
	  break;
      }

    /*
     * Build the "drawing region" as we build the PCL to draw the
     * line.
     */
    for(i = 1, r = drawRects; i < nPoints; i++, r++ )
      {
	  if( i != 1 )
	    SAVE_PCL( outFile, pConPriv, "," );

	  sprintf( t, "%d,%d", pPoints[i].x + xoffset,
		  pPoints[i].y + yoffset );
	  SAVE_PCL( outFile, pConPriv, t );

	  r->x = MIN( pPoints[i-1].x, pPoints[i].x ) + xoffset - fudge;
	  r->y = MIN( pPoints[i-1].y, pPoints[i].y ) + yoffset - fudge;
	  r->width = abs( pPoints[i-1].x - pPoints[i].x ) + 2 * fudge;
	  r->height = abs( pPoints[i-1].y - pPoints[i].y ) + 2 * fudge;
      }
    SAVE_PCL( outFile, pConPriv, ";\033%0A" ); /* End the macro */
    MACRO_END( outFile );

    /*
     * Convert the collection of rectangles into a proper region, then
     * intersect it with the clip region.
     */
    drawRegion = RECTS_TO_REGION( pGC->pScreen, nPoints - 1,
				  drawRects, CT_UNSORTED );
    if( mode == CoordModePrevious )
      REGION_TRANSLATE( pGC->pScreen, drawRegion, pPoints[0].x, pPoints[0].y );
    REGION_INTERSECT( pGC->pScreen, region, drawRegion, pGC->pCompositeClip );

    /*
     * For each rectangle in the clip region, set the HP-GL/2 "input
     * window" and render the entire polyline to it.
     */
    pbox = REGION_RECTS( region );
    nbox = REGION_NUM_RECTS( region );

    PclSendData(outFile, pConPriv, pbox, nbox, 1.0);

    /*
     * Clean up the temporary regions
     */
    REGION_DESTROY( pGC->pScreen, drawRegion );
    REGION_DESTROY( pGC->pScreen, region );
    xfree( drawRects );
}

void
PclPolySegment(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int nSegments,
     xSegment *pSegments)
{
    FILE *outFile, *dummy;
    char t[80];
    int xoffset, yoffset;
    int nbox, i;
    unsigned long valid;
    BoxPtr pbox;
    xRectangle *drawRects, *r;
    RegionPtr drawRegion, region;
    short fudge;
    XpContextPtr pCon;
    PclContextPrivPtr pConPriv;
    GC cacheGC;


    if( PclUpdateDrawableGC( pGC, pDrawable, &outFile ) == FALSE )
      return;

    pCon = PclGetContextFromWindow( (WindowPtr) pDrawable );
    pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);

    /*
     * Allocate the storage for the temporary regions.
     */
    region = REGION_CREATE( pGC->pScreen, NULL, 0 );
    drawRects = (xRectangle *)
      xalloc( nSegments * sizeof( xRectangle ) );

    /*
     * Calculate the fudge factor, based on the line width
     */
    fudge = pGC->lineWidth * 3 + 1;

    /*
     * Turn off line joining.
     */
    SEND_PCL( outFile, "\033%0BLA2,6;\033%0A" );

    /*
     * Generate the PCL code to draw the segments, by defining them as
     * a macro which uses the HP-GL/2 line drawing function.
     *
     * XXX I wonder if this should be implemented using the Encoded
     * XXX Polyline function.  Since I'm only sending it once, it's not
     * XXX necessarily too important.
     */

    MACRO_START( outFile, pConPriv );
    SAVE_PCL( outFile, pConPriv, "\033%0B" );

    xoffset = pDrawable->x;
    yoffset = pDrawable->y;

    for( i = 0, r = drawRects; i < nSegments; i++, r++ )
      {
	  r->x = MIN( pSegments[i].x1, pSegments[i].x2 ) + xoffset;
	  r->y = MIN( pSegments[i].y1, pSegments[i].y2 ) + yoffset;
	  r->width = abs( pSegments[i].x1 - pSegments[i].x2 );
	  r->height = abs( pSegments[i].y1 - pSegments[i].y2 );

	  sprintf( t, "PU%d,%d;PD%d,%d;", pSegments[i].x1 + xoffset,
		  pSegments[i].y1 + yoffset, pSegments[i].x2 +
		  xoffset, pSegments[i].y2 + yoffset );
	  SAVE_PCL( outFile, pConPriv, t );

	  r->x -= fudge;
	  r->y -= fudge;
	  r->width += 2 * fudge;
	  r->height += 2 * fudge;
      }
    SAVE_PCL( outFile, pConPriv, "\033%0A" );
    MACRO_END ( outFile );

    /*
     * Convert the collection of rectangles into a proper region, then
     * intersect it with the clip region.
     */
    drawRegion = RECTS_TO_REGION( pGC->pScreen, nSegments,
				  drawRects, CT_UNSORTED );
    REGION_INTERSECT( pGC->pScreen, region, drawRegion, pGC->pCompositeClip );

    /*
     * For each rectangle in the clip region, set the HP-GL/2 "input
     * window" and render the entire set of segments to it.
     */
    pbox = REGION_RECTS( region );
    nbox = REGION_NUM_RECTS( region );

    PclSendData(outFile, pConPriv, pbox, nbox, 1.0);

    /*
     * Now we need to reset the line join mode to whatever it was at before.
     * The easiest way is to force the cached GC's joinstyle to be different
     * from the current GC's joinstyle, then re-update the GC.  This way, we
     * don't have to duplicate code unnecessarily.
     */
    PclGetDrawablePrivateStuff( pDrawable, &cacheGC, &valid, &dummy );
    cacheGC.joinStyle = !cacheGC.joinStyle;
    PclSetDrawablePrivateGC( pDrawable, cacheGC );
    PclUpdateDrawableGC( pGC, pDrawable, &outFile );

    /*
     * Clean up
     */
    REGION_DESTROY( pGC->pScreen, drawRegion );
    REGION_DESTROY( pGC->pScreen, region );
    xfree( drawRects );
}
