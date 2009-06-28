/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclArc.c
**    *
**    *  Contents:
**    *                 Arc-drawing code for the PCL DDX driver
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
#include <math.h>
#include <errno.h>

#include "Pcl.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "attributes.h"

static void
PclDoArc(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int nArcs,
     xArc *pArcs,
     void (*DoIt)(FILE *, PclContextPrivPtr, double, double, xArc))
{
    char t[80];
    FILE *outFile;
    int nbox, i;
    BoxPtr pbox;
    BoxRec r;
    RegionPtr drawRegion, region, transClip;
    short fudge;
    int xoffset, yoffset;
    XpContextPtr pCon;
    PclContextPrivPtr pConPriv;
    xRectangle repro;
    
    if( PclUpdateDrawableGC( pGC, pDrawable, &outFile ) == FALSE )
      return;
    
    fudge = 3 * pGC->lineWidth;

    pCon = PclGetContextFromWindow( (WindowPtr) pDrawable );
    pConPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    XpGetReproductionArea( pCon, &repro );
    
    /* 
     * Generate the PCL code to draw the collection of arcs, by
     * defining it as a macro which uses the HP-GL/2 arc drawing
     * function.
     */

    xoffset = pDrawable->x;
    yoffset = pDrawable->y;
    
    for( i = 0; i < nArcs; i++ )
      {
	  xArc Arc = pArcs[i];
	  double b, X, Y, ratio;
	  double angle1;

	  MACRO_START( outFile, pConPriv );
	  SAVE_PCL( outFile, pConPriv, "\033%0B" );

	  /* Calculate the start of the arc */
	  if( ( Arc.angle1 / 64 ) % 360 == 90 )
	    {
		X = 0;
		Y = -Arc.height / 2.0;
	    }
	  else if( ( Arc.angle1 / 64 ) % 360 == 270 )
	    {
		X = 0;
		Y = Arc.height / 2.0;
	    }
	  else
	    {
		/* Convert the angle to radians */
		angle1 = ( Arc.angle1 / 64.0 ) * 3.141592654 / 180.0;
	  
		b = (Arc.height / 2.0);
		X = b * cos( angle1 );
		Y = -b * sin( angle1 );
	    }
	  
	  /* Change the coordinate system to scale the ellipse */
	  ratio = (double)Arc.height / (double)Arc.width;
	  
	  sprintf( t, "SC%.2f,%.2f,%d,%d;", 
		  (repro.x - Arc.width / 2 - xoffset - Arc.x) * ratio,
		  (repro.x - Arc.width / 2 - xoffset - Arc.x +
		   repro.width) * ratio,
		  repro.y - Arc.height / 2 - yoffset - Arc.y + repro.height,
		  repro.y - Arc.height / 2 - yoffset - Arc.y);
	  SAVE_PCL( outFile, pConPriv, t );

	  DoIt( outFile, pConPriv, X, Y, Arc );
	  
	  /* Build the bounding box */
	  r.x1 = -Arc.width / 2 - fudge;
	  r.y1 = -Arc.height / 2 - fudge;
	  r.x2 = Arc.width / 2 + fudge;
	  r.y2 = Arc.height / 2 + fudge;
	  drawRegion = REGION_CREATE( pGC->pScreen, &r, 0 );

	  SAVE_PCL( outFile, pConPriv, "\033%0A" );
	  MACRO_END( outFile );
    
	  /*
	   * Intersect the bounding box with the clip region.
	   */
	  region = REGION_CREATE( pGC->pScreen, NULL, 0 );
    	  transClip = REGION_CREATE( pGC->pScreen, NULL, 0 );
	  REGION_COPY( pGC->pScreen, transClip, pGC->pCompositeClip );
	  REGION_TRANSLATE( pGC->pScreen, transClip,
			    -(xoffset + Arc.x + Arc.width / 2),
			    -(yoffset + Arc.y + Arc.height / 2) );
	  REGION_INTERSECT( pGC->pScreen, region, drawRegion, transClip );

	  /*
	   * For each rectangle in the clip region, set the HP-GL/2 "input
	   * window" and render the collection of arcs to it.
	   */
	  pbox = REGION_RECTS( region );
	  nbox = REGION_NUM_RECTS( region );
    
	  PclSendData(outFile, pConPriv, pbox, nbox, ratio);

	  /*
	   * Restore the coordinate system
	   */
	  sprintf( t, "\033%%0BSC%d,%d,%d,%d;\033%%0A", repro.x, 
		  repro.x + repro.width, repro.y + repro.height, 
		  repro.y );
	  SEND_PCL( outFile, t );
	  
	  /*
	   * Clean up the temporary regions
	   */
	  REGION_DESTROY( pGC->pScreen, drawRegion );
	  REGION_DESTROY( pGC->pScreen, region );
	  REGION_DESTROY( pGC->pScreen, transClip );
      }
}

/*
 * Draw a simple non-filled arc, centered on the origin and starting
 * at the given point.
 */
static void
DrawArc(FILE *outFile,
	PclContextPrivPtr pConPriv,
	double X,
	double Y,
	xArc A)
{
    char t[80];

    sprintf( t, "PU%d,%d;PD;AA0,0,%.2f;", (int)X, (int)Y,
	    (float)A.angle2 / -64.0 );
    SAVE_PCL(outFile, pConPriv, t);
}

void
PclPolyArc(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int nArcs,
     xArc *pArcs)
{
    PclDoArc( pDrawable, pGC, nArcs, pArcs, DrawArc );
}

/*
 * Draw a filled wedge, from the origin, to the given point, through
 * the appropriate angle, and back to the origin.
 */
static void
DoWedge(FILE *outFile,
	PclContextPrivPtr pConPriv,
	double X,
	double Y,
	xArc A)
{
    char t[80];
    
    sprintf( t, "PU0,0;WG%.2f,%.2f,%.2f;", sqrt( X * X + Y * Y ), 
	    (float)A.angle1 / -64.0,
	    (float)A.angle2 / -64.0 );
    SAVE_PCL(outFile, pConPriv, t);
}

static void
DoChord(FILE *outFile,
	PclContextPrivPtr pConPriv,
	double X,
	double Y,
	xArc A)
{
    char t[80];
    
    sprintf( t, "PU%d,%d;PM0;AA0,0,%.2f;PA%d,%d;PM2;FP;", (int)X, (int)Y, 
	    (float)A.angle2 / -64.0 , (int)X, (int)Y );
    SAVE_PCL(outFile, pConPriv, t);
}


void
PclPolyFillArc(
     DrawablePtr pDrawable,
     GCPtr pGC,
     int nArcs,
     xArc *pArcs)
{
    switch( pGC->arcMode )
      {
	case ArcChord:
	  PclDoArc( pDrawable, pGC, nArcs, pArcs, DoChord );
	  break;
	case ArcPieSlice:
	  PclDoArc( pDrawable, pGC, nArcs, pArcs, DoWedge );
	  break;
      }
}
