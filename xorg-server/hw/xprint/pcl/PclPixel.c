/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclPixel.c
**    *
**    *  Contents:
**    *                 Pixel-drawing code for the PCL DDX driver
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

#include "windowstr.h"
#include "gcstruct.h"

#include "Pcl.h"

void
PclPolyPoint( pDrawable, pGC, mode, nPoints, pPoints )
     DrawablePtr pDrawable;
     GCPtr pGC;
     int mode;
     int nPoints;
     xPoint *pPoints;
{
    char t[80];
    FILE *outFile;
    int xoffset, yoffset;
    BoxRec box;
    int xloc, yloc, i;
#if 0
    XpContextPtr pCon;
    PclContextPrivPtr cPriv;
    PclPixmapPrivPtr pPriv;
#endif
    
    if( PclUpdateDrawableGC( pGC, pDrawable, &outFile ) == FALSE )
      return;

    xoffset = pDrawable->x;
    yoffset = pDrawable->y;
    
    /*
     * Enter HP-GL/2 and change the line style to one in which only
     * the vertices of the specified polyline are drawn.  We must also
     * temporarily change the line width so that only a single pixel
     * is drawn.  Then move to the first possible location.
     */
    xloc = pPoints[0].x + pDrawable->x;
    yloc = pPoints[0].y + pDrawable->y;

    sprintf( t, "\27%%0BPW0,0;LT0;PU;PA%d,%d", xloc, yloc );
    SEND_PCL( outFile, t );
    
    /*
     * Check each point against the clip region.  If it is outside the
     * region, don't send the PCL to the printer.
     */
    
    for( i = 0; i < nPoints; i++ )
      {
	  if( POINT_IN_REGION( pGC->pScreen, pGC->clientClip, xloc, yloc, &box ) )
	    {
		sprintf( t, ",%d,%d", xloc, yloc );
		SEND_PCL( outFile, t );
	    }
	  
	  if( mode == CoordModeOrigin )
	    {
		xloc = pPoints[i+1].x + xoffset;
		yloc = pPoints[i+1].y + yoffset;
	    }
	  else
	    {
		xloc += pPoints[i+1].x;
		yloc += pPoints[i+1].y;
	    }
      }
    
#if 0
    /*
     * Change the line style and width back to what they were before
     * this routine was called.  No, this isn't pretty...
     */
    if( pDrawable->type == DRAWABLE_WINDOW )
      {
	  pCon = PclGetContextFromWindow( (WindowPtr)pDrawable );
	  cPriv = (PclContextPrivPtr)
	      dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
	  cPriv->changeMask = GCLineWidth | GCLineStyle;
      }
    else
      {
	  pPriv = (PclPixmapPrivPtr)
	      dixLookupPrivate(&((PixmapPtr)pDrawable)->devPrivates,
			       PclPixmapPrivateKey);
	  pPriv->changeMask = GCLineWidth | GCLineStyle;
      }
#endif
    
    PclUpdateDrawableGC( pGC, pDrawable, &outFile );

    /*
     * Go back to PCL
     */
    SEND_PCL( outFile, "\27%0A" );
}

void
PclPushPixels( pGC, pBitmap, pDrawable, width, height, x, y )
     GCPtr pGC;
     PixmapPtr pBitmap;
     DrawablePtr pDrawable;
     int width;
     int height;
     int x;
     int y;
{
}
