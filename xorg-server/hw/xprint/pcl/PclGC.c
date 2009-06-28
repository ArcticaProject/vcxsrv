/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclGC.c
**    *
**    *  Contents:
**    *                 Graphics Context handling for the PCL driver
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

#include "gcstruct.h"

#include "Pcl.h"
#include "pixmapstr.h"
#include "colormapst.h"
#include "windowstr.h"
#include "fb.h"
#include "scrnintstr.h"
#include "resource.h"

static GCOps PclGCOps = 
{
    PclFillSpans,
    PclSetSpans,
    PclPutImage,
    PclCopyArea,
    PclCopyPlane,
    PclPolyPoint,
    PclPolyLine,
    PclPolySegment,
    PclPolyRectangle,
    PclPolyArc,
    PclFillPolygon,
    PclPolyFillRect,
    PclPolyFillArc,
    PclPolyText8,
    PclPolyText16,
    PclImageText8,
    PclImageText16,
    PclImageGlyphBlt,
    PclPolyGlyphBlt,
    PclPushPixels
}
;


static GCFuncs PclGCFuncs = 
{
    PclValidateGC,
    miChangeGC,
    miCopyGC,
    PclDestroyGC,
    miChangeClip,
    miDestroyClip,
    miCopyClip,
}
;

Bool
PclCreateGC(GCPtr pGC)
{
    if (fbCreateGC(pGC) == FALSE)
        return FALSE;

    pGC->clientClip = NULL;
    pGC->clientClipType = CT_NONE;

    pGC->ops = &PclGCOps;
    pGC->funcs = &PclGCFuncs;

    return TRUE;
}

void
PclDestroyGC(GCPtr pGC)
{
    /* fb doesn't specialize DestroyGC */
    miDestroyGC( pGC );
}


int
PclGetDrawablePrivateStuff(
     DrawablePtr pDrawable,
     GC *gc,
     unsigned long *valid,
     FILE **file)
{
    XpContextPtr pCon;
    PclContextPrivPtr cPriv;
    
    switch( pDrawable->type )
      {
	case DRAWABLE_PIXMAP:
	  /*
	   * If we ever get here, something is wrong.
	   */
	  return FALSE;

	case DRAWABLE_WINDOW:
	  pCon = PclGetContextFromWindow( (WindowPtr)pDrawable );

	  if( pCon == NULL )
	    return FALSE;
	  else
	    {
		cPriv = (PclContextPrivPtr)
		    dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
		*gc = cPriv->lastGC;
		*valid = cPriv->validGC;
		*file = cPriv->pPageFile;
		return TRUE;
	    }
	  
	default:
	  return FALSE;
      }
}

void
PclSetDrawablePrivateGC(
     DrawablePtr pDrawable,
     GC gc)
{
    PixmapPtr pix;
    XpContextPtr pCon;
    PclPixmapPrivPtr pixPriv;
    PclContextPrivPtr pPriv;
    int i;
    
    switch( pDrawable->type )
      {
	case DRAWABLE_PIXMAP:
	  pix = (PixmapPtr)pDrawable;
	  pixPriv = (PclPixmapPrivPtr)
	      dixLookupPrivate(&pix->devPrivates, PclPixmapPrivateKey);
	  
	  pixPriv->lastGC = gc;
	  pixPriv->validGC = 1;
	  break;

	case DRAWABLE_WINDOW:
	  pCon = PclGetContextFromWindow( (WindowPtr)pDrawable );
	  pPriv = (PclContextPrivPtr)
	      dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
	  
	  pPriv->validGC = 1;
	  pPriv->lastGC = gc;

	  /*
	   * Store the dash list separately, to avoid having it freed
	   * out from under us.
	   */
	  if( pPriv->dash != NULL )
	    xfree( pPriv->dash );
	  if( gc.numInDashList != 0 )
	    {
		pPriv->dash = (unsigned char *)xalloc( sizeof( unsigned char ) 
						      * gc.numInDashList );
		for( i = 0; i < gc.numInDashList; i++ )
		  pPriv->dash[i] = gc.dash[i];
	    }
	  else
	    pPriv->dash = NULL;
	  

	  /*
	   * Store the dash list separately, to avoid having it freed
	   * out from under us.
	   */
	  if( pPriv->dash != NULL )
	    xfree( pPriv->dash );
	  if( gc.numInDashList != 0 )
	    {
		pPriv->dash = (unsigned char *)xalloc( sizeof( unsigned char ) 
						      * gc.numInDashList );
		for( i = 0; i < gc.numInDashList; i++ )
		  pPriv->dash[i] = gc.dash[i];
	    }
	  else
	    pPriv->dash = NULL;
	  
	  break;
      }
}

static void
PclSendPattern(char *bits,
	       int sz,
	       int depth,
	       int h,
	       int w,
	       int patNum,
	       FILE *outFile)
{
    char t[80], *row, *mod;
    int w2;
    int i, j;
    
    SEND_PCL( outFile, "\033%0A" );

    if( depth == 1 )
      {
	  /* Each row must be word-aligned */
	  w2 = ( w / 8 ) + ( ( w%8 ) ? 1 : 0 );
/*
	  if( w2 % 2 )
	    w2++;
*/
	  
	  sprintf( t, "\033*c%dg%dW", patNum, h * w2 + 8 );
	  SEND_PCL( outFile, t );
	  
	  sprintf( t, "%c%c%c%c%c%c%c%c", 0, 0, 1, 0, h>>8, h&0xff, w>>8,
		  w&0xff );
	  SEND_PCL_COUNT( outFile, t, 8 );
	  
	  for( row = bits, i = 0; i < h; i++, row += BitmapBytePad( w ) )
	    SEND_PCL_COUNT( outFile, row, w2 );
      }
    else if( depth == 8 )
      {
	  w2 = ( w % 2 ) ? w + 1 : w;
	  
	  sprintf( t, "\033*c%dg%dW", patNum, h * w2 + 8 );
	  SEND_PCL( outFile, t );
	  
	  sprintf( t, "%c%c%c%c%c%c%c%c", 1, 0, 8, 0, h>>8, h&0xff,
		  w>>8, w&0xff );
	  SEND_PCL_COUNT( outFile, t, 8 );
	  
	  for( row = bits, i = 0; i < h; i++, 
	      row += PixmapBytePad( w, 8 ) )
	    SEND_PCL_COUNT( outFile, row, w2 );
      }
    else
      {
	  w2 = ( w % 2 ) ? w + 1 : w;
	  
	  sprintf( t, "\033*c%dg%dW", patNum, h * w2 + 8 );
	  SEND_PCL( outFile, t );
	  
	  sprintf( t, "%c%c%c%c%c%c%c%c", 1, 0, 8, 0, h>>8, h&0xff,
		  w>>8, w&0xff );
	  SEND_PCL_COUNT( outFile, t, 8 );
	  
	  mod = (char *)xalloc( w2 );
	  
	  for( row = bits, i = 0; i < h; i++, 
	      row += PixmapBytePad( w, 24 ) )
	    {
		char r, g, b;
		for( j = 0; j < w2; j++ ) {
		  r = ((row[j*4+1] >> 5) & 0x7) << 5;
		  g = ((row[j*4+2] >> 5) & 0x7) << 2;
		  b = ((row[j*4+3] >> 6) & 0x3);
		  mod[j] = r | g | b;
		}
		SEND_PCL_COUNT( outFile, mod, w2 );
	    }
	  
	  xfree( mod );
      }
    
    SEND_PCL( outFile, "\033%0B" );
}

int
PclUpdateDrawableGC(
     GCPtr pGC,
     DrawablePtr pDrawable,
     FILE **outFile)
{
    Mask changeMask = 0;
    GC dGC;
    unsigned long valid;
    int i;
    XpContextPtr pCon;
    PclContextPrivPtr cPriv;
    PclGCPrivPtr gcPriv = (PclGCPrivPtr)
	dixLookupPrivate(&pGC->devPrivates, PclGCPrivateKey);
    
    if( !PclGetDrawablePrivateStuff( pDrawable, &dGC, &valid, outFile ) )
      return FALSE;

    pCon = PclGetContextFromWindow( (WindowPtr)pDrawable );
    cPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);

    /*
     * Here's where we update the colormap.  Since there can be
     * different colormaps installed on each window, we need to check
     * before each drawing request that the correct palette is active in
     * the printer.  This is as good a place as any.
     */
    if( !PclUpdateColormap( pDrawable, pCon, pGC, *outFile ) )
      return FALSE;

    /*
     * If the drawable's last GC is NULL, this means that this is
     * the first time the drawable is being used.  Therefore, we need
     * to emit PCL for all the GC fields.
     */
    if( valid == 0 )
      changeMask = ~0;

    /*
     * If we have two different GC structures, there is no alternative
     * but to scan through them both to determine the changeMask.
     */
    else
      {
	  if( dGC.alu != pGC->alu )
	    changeMask |= GCFunction;
	  if( dGC.fgPixel != pGC->fgPixel )
	    changeMask |= GCForeground;
	  if( dGC.bgPixel != pGC->bgPixel )
	    changeMask |= GCBackground;
	  if( dGC.lineWidth != pGC->lineWidth )
	    changeMask |= GCLineWidth;
	  if( dGC.lineStyle != pGC->lineStyle )
	    changeMask |= GCLineStyle;
	  if( dGC.capStyle != pGC->capStyle )
	    changeMask |= GCCapStyle;
	  if( dGC.joinStyle != pGC->joinStyle )
	    changeMask |= GCJoinStyle;
	  if( dGC.fillStyle != pGC->fillStyle )
	    changeMask |= GCFillStyle;
	  if( dGC.tile.pixmap != pGC->tile.pixmap )
	    changeMask |= GCTile;
	  if( dGC.stipple != pGC->stipple )
	    changeMask |= GCStipple;
	  if( dGC.patOrg.x != pGC->patOrg.x )
	    changeMask |= GCTileStipXOrigin;
	  if( dGC.patOrg.y != pGC->patOrg.y )
	    changeMask |= GCTileStipYOrigin;
	  
	  if( dGC.numInDashList == pGC->numInDashList )
	  {
	    for( i = 0; i < dGC.numInDashList; i++ )
	      if( cPriv->dash[i] != pGC->dash[i] )
		{
		    changeMask |= GCDashList;
		    break;
		}
	  }
	  else
	    changeMask |= GCDashList;
      }

    /*
     * Once the changeMask has been determined, we scan it and emit
     * the appropriate PCL code to set the drawing attributes.
     */

    /* Must be in HP-GL/2 mode to set attributes */
    SEND_PCL( *outFile, "\033%0B" );

    if( changeMask & GCFunction )
      {
#ifdef XP_PCL_COLOR

	  if( pGC->alu == GXclear )
	    SEND_PCL( *outFile, "SP0;" );
	  else
	    SEND_PCL( *outFile, "SP1;" );
#else
	  if( pGC->alu == GXclear )
	    SEND_PCL( *outFile, "SP0;" );
	  else
	    SEND_PCL( *outFile, "SP1;" );
#endif /* XP_PCL_COLOR */
      }
    
#if 0
    if( changeMask & GCFunction )
      {
	  int rop = -1;
	  char t[10];
	  
	  switch( pGC->alu )
	    {
	      case GXclear:
		rop = 1;
		break;
	      case GXand:
		rop = 136;
		break;
	      case GXandReverse:
		rop = 68;
		break;
	      case GXcopy:
		rop = 204;
		break;
	      case GXandInverted:
		rop = 34;
		break;
	      case GXnoop:
		rop = 170;
		break;
	      case GXxor:
		rop = 238;
		break;
	      case GXor:
		rop = 238;
		break;
	      case GXnor:
		rop = 17;
		break;
	      case GXequiv:
		rop = 153;
		break;
	      case GXinvert:
		rop = 85;
		break;
	      case GXorReverse:
		rop = 221;
		break;
	      case GXcopyInverted:
		rop = 51;
		break;
	      case GXorInverted:
		rop = 187;
		break;
	      case GXnand:
		rop = 119;
		break;
	      case GXset:
		rop = 0;
		break;
	    }
	  if( rop != -1 )
	    {
		sprintf( t, "MC1,%d;", rop );
		SEND_PCL( *outFile, t );
#endif

    if( changeMask & GCForeground )
      switch( pGC->fgPixel )
	{
	  case 1:
	    SEND_PCL( *outFile, "SP1;" );
	    break;
	  default:
	    SEND_PCL( *outFile, "SP0;" );
	    break;
	}
    
    if( changeMask & GCForeground )
      {
#ifdef XP_PCL_COLOR
	  ColormapPtr cmap;
	  Colormap c;
	  char t[40];

	  c = wColormap( ((WindowPtr)pDrawable) );
	  cmap = (ColormapPtr)LookupIDByType( c, RT_COLORMAP );

	  if( cmap->class == TrueColor )
	    {
	      if( pGC->fillStyle != FillTiled || pGC->tileIsPixel ) {
		unsigned short r, g, b;
	
		r = (pGC->fgPixel & cmap->pVisual->redMask)
						>> (cmap->pVisual->offsetRed );
		g = (pGC->fgPixel & cmap->pVisual->greenMask)
						>> (cmap->pVisual->offsetGreen);
		b = (pGC->fgPixel & cmap->pVisual->blueMask)
						>> (cmap->pVisual->offsetBlue);

		PclLookUp(cmap, cPriv, &r, &g, &b);
		sprintf( t, "\033%%0A\033*v%ua%ub%uc0I\033%%0B", r, g, b);
		SEND_PCL( *outFile, t );
	     }
	    }
	  else /* PseudoColor or StaticGray */
	    {
		sprintf( t, "SP%ld;", (long) pGC->fgPixel );
		SEND_PCL( *outFile, t );
	    }
#else
	  ScreenPtr screen;
	  screen = pDrawable->pScreen;
	  if ( pGC->fgPixel == screen->whitePixel )
	    SEND_PCL( *outFile, "SP0;");
	  else
	    SEND_PCL( *outFile, "SP1;");
#endif /* XP_PCL_COLOR */
      }
    
    if( changeMask & GCJoinStyle )
      switch( pGC->joinStyle )
	{
	  case JoinMiter:
	    SEND_PCL( *outFile, "LA2,1;" );
	    break;
	  case JoinRound:
	    SEND_PCL( *outFile, "LA2,4;" );
	    break;
	  case JoinBevel:
	    SEND_PCL( *outFile, "LA2,5;" );
	    break;
	}
    
    if( changeMask & GCCapStyle )
      switch( pGC->capStyle )
	{
	  case CapNotLast:
	  case CapButt:
	    SEND_PCL( *outFile, "LA1,1;" );
	    break;
	  case CapRound:
	    SEND_PCL( *outFile, "LA1,4;" );
	    break;
	  case CapProjecting:
	    SEND_PCL( *outFile, "LA1,2;" );
	    break;
	}

    if( changeMask & GCLineWidth )
      {
	  float penWidth, pixelsPerMM;
	  ScreenPtr screen;
	  char temp[30];
	  
	  if( pGC->lineWidth == 0 || pGC->lineWidth == 1 )
	    /* A pen width of 0.0 mm gives a one-pixel-wide line */
	    penWidth = 0.0;
	  else
	    {
		screen = pDrawable->pScreen;
		pixelsPerMM = (float)screen->width / (float)screen->mmWidth;
		
		penWidth = pGC->lineWidth / pixelsPerMM;
	    }
	  sprintf( temp, "PW%g;", penWidth );
	  SEND_PCL( *outFile, temp );
      }
    
    if( changeMask & GCLineStyle )
      {
	  int i, num = pGC->numInDashList;
	  double total;
	  char t[30];
	  
	  switch( pGC->lineStyle )
	    {
	      case LineSolid:
		SEND_PCL( *outFile, "LT;" );
		break;
	      case LineOnOffDash:
		/*
		 * Calculate the pattern length of the dashes, in pixels,
		 * then convert to mm
		 */
		for( i = 0, total = 0.0; i < 20 && i < num; i++ )
		  total += pGC->dash[i];
		if( num % 2 )
		  for( i = num; i < 20 && i < num + num; i++ )
		    total += pGC->dash[i-num];

		total *= ( (double)pDrawable->pScreen->mmWidth /
			  (double)pDrawable->pScreen->width );

		sprintf( t, "LT8,%f,1;", total );
		SEND_PCL( *outFile, t );
		break;
	    }
      }
    

    if( changeMask & GCFillStyle )
      switch( pGC->fillStyle )
	{
	  case FillSolid:
	    SEND_PCL( *outFile, "FT1;TR0;CF;" );
	    break;
	  case FillTiled:
	    SEND_PCL( *outFile, "FT22,100;TR0;CF2,0;" );
	    break;
	  case FillOpaqueStippled:
	    SEND_PCL( *outFile, "FT22,101;TR0;CF2,0;" );
	    if( pGC->fgPixel != gcPriv->stippleFg ||
	       pGC->bgPixel != gcPriv->stippleBg )
	      changeMask |= GCStipple;
	    break;
	  case FillStippled:
	    SEND_PCL( *outFile, "FT22,102;TR1;CF2,0;" );
	    break;
	}

    if( changeMask & GCTile && !pGC->tileIsPixel )
      {
	  char *bits;
	  int h, w, sz;

	  h = pGC->tile.pixmap->drawable.height;
	  w = pGC->tile.pixmap->drawable.width;

          sz = h * PixmapBytePad(w, pGC->tile.pixmap->drawable.depth);
          bits = (char *)xalloc(sz);
          fbGetImage(&(pGC->tile.pixmap->drawable), 0, 0, w, h, XYPixmap, ~0,
                     bits);
          PclSendPattern( bits, sz, 1, h, w, 100, *outFile );
	  xfree( bits );
      }

    if( changeMask & ( GCTileStipXOrigin | GCTileStipYOrigin ) )
      {
	  char t[30];
	  
	  sprintf( t, "AC%d,%d;", pGC->patOrg.x, pGC->patOrg.y );
	  SEND_PCL( *outFile, t );
      }

    /*
     * We have to resend the stipple pattern either when the stipple itself
     * changes, or if we're in FillOpaqueStippled mode and either the
     * foreground or the background color changes.
     */
    if( changeMask & GCStipple || 
       ( pGC->fillStyle == FillOpaqueStippled &&
	( pGC->fgPixel != gcPriv->stippleFg || 
	 pGC->bgPixel != gcPriv->stippleBg ) ) )
      {
	  int h, w, i, sz, w2;
	  char *bits, *row, t[30];
	  PixmapPtr scratchPix;
	  GCPtr scratchGC;
	  
	  if( pGC->stipple != NULL )
	    {
		SEND_PCL( *outFile, "\033%0A" );
		
		h = pGC->stipple->drawable.height;
		w = pGC->stipple->drawable.width;
		sz = h * BitmapBytePad( w );

		bits = (char *)xalloc( sz );
		fbGetImage( &(pGC->stipple->drawable), 0, 0, w, h, XYPixmap, ~0, bits );

		w2 = ( w / 8 ) + ( ( w%8 ) ? 1 : 0 );
		/*
		 * XXX The PCL docs say that I need to word-align each
		 * XXX row, but I get garbage when I do...
		 */
		/*
		if( w2 % 2 )
		  w2++;
		*/
                
		sprintf( t, "\033*c102g%dW", h * w2 + 8 );
		SEND_PCL( *outFile, t );
		
		sprintf( t, "%c%c%c%c%c%c%c%c", 0, 0, 1, 0, h>>8, h&0xff, w>>8,
			w&0xff );
		SEND_PCL_COUNT( *outFile, t, 8 );
		
		for( row = bits, i = 0; i < h; i++, row += BitmapBytePad( w ) )
		  SEND_PCL_COUNT( *outFile, row, w2 );

		SEND_PCL( *outFile, "\033%0B" );
		
		xfree( bits );

		/*
		 * Also do the opaque stipple, as a tile
		 */
		if( pGC->depth != 1 )
		  sz = h * PixmapBytePad( w, pGC->depth );
		bits = (char *)xalloc( sz );
		
		scratchPix = 
		  (*pGC->pScreen->CreatePixmap)( pGC->pScreen,
						w, h, pGC->depth,
						CREATE_PIXMAP_USAGE_SCRATCH );
		scratchGC = GetScratchGC( pGC->depth, pGC->pScreen );
		CopyGC( pGC, scratchGC, ~0L );

                fbValidateGC(scratchGC, ~0L, (DrawablePtr)scratchPix);
		fbCopyPlane(&(pGC->stipple->drawable), (DrawablePtr)scratchPix,
                            scratchGC, 0, 0, w, h, 0, 0, 1);
		fbGetImage(&(scratchPix->drawable), 0, 0, w, h, XYPixmap, ~0,
			   bits);
		PclSendPattern( bits, sz, pGC->depth, h, w, 101, *outFile );
		FreeScratchGC( scratchGC );
		(*pGC->pScreen->DestroyPixmap)( scratchPix );
		xfree( bits );
	    }
      }

    if( changeMask & ( GCTileStipXOrigin | GCTileStipYOrigin ) )
      {
	  char t[30];
	  
	  sprintf( t, "AC%d,%d;", pGC->patOrg.x, pGC->patOrg.y );
	  SEND_PCL( *outFile, t );
      }
    
    if( changeMask & GCDashList )
      {
	  int num = pGC->numInDashList;
	  double total;
	  char dashes[20];
	  char t[100], t2[20];

	  /* Make up the doubled dash list, if necessary */
	  for( i = 0; i < 20 && i < num; i++ )
	    dashes[i] = pGC->dash[i];
	  
	  if( num % 2 )
	    {
		for( i = num; i < 20 && i < num + num; i++ )
		  dashes[i] = dashes[i-num];
		if( ( num *= 2 ) > 20 )
		  num = 20;
	    }
	  
	  /* Add up dash lengths to get percentage */
	  for( i = 0, total = 0; i < num; i++ )
	    total += dashes[i];
	  
	  /* Build up the HP-GL/2 for the dash list */
	  strcpy( t, "UL8" );
	  for( i = 0; i < num; i++ )
	    {
		sprintf( t2, ",%d", 
			(int)( ( (double)dashes[i] / total * 100.0 ) + 0.5 ) );
		strcat( t, t2 );
	    }
	  strcat( t, ";" );
	  SEND_PCL( *outFile, t );
      }
   

    /* Go back to PCL mode */
    SEND_PCL( *outFile, "\033%0A" );
    
    /*
     * Update the drawable's private information, which includes
     * erasing the drawable's private changeMask, since all the
     * changes have been made.
     */
    if( changeMask )
      PclSetDrawablePrivateGC( pDrawable, *pGC );
    
    return TRUE;
}

/*
 * PclComputeCompositeClip()
 *
 * I'd like to use the miComputeCompositeClip function, but it sticks
 * things into the mi GC privates, where the PCL driver can't get at
 * it.  So, rather than mess around with the mi code, I ripped it out
 * and made the appropriate changes here.
 */


void
PclComputeCompositeClip(
    GCPtr           pGC,
    DrawablePtr     pDrawable)
{
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
		REGION_DESTROY(pGC->pScreen, pGC->pCompositeClip);
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

	    REGION_TRANSLATE(pGC->pScreen, pGC->clientClip,
					 pDrawable->x + pGC->clipOrg.x,
					 pDrawable->y + pGC->clipOrg.y);

	    if (freeCompClip)
	    {
		REGION_INTERSECT(pGC->pScreen, pGC->pCompositeClip,
					    pregWin, pGC->clientClip);
		if (freeTmpClip)
		    REGION_DESTROY(pGC->pScreen, pregWin);
	    }
	    else if (freeTmpClip)
	    {
		REGION_INTERSECT(pGC->pScreen, pregWin, pregWin,
				 pGC->clientClip);
		pGC->pCompositeClip = pregWin;
	    }
	    else
	    {
		pGC->pCompositeClip = REGION_CREATE(pGC->pScreen, NullBox, 0);
		REGION_INTERSECT(pGC->pScreen, pGC->pCompositeClip,
				       pregWin, pGC->clientClip);
	    }
	    pGC->freeCompClip = TRUE;
	    REGION_TRANSLATE(pGC->pScreen, pGC->clientClip,
					 -(pDrawable->x + pGC->clipOrg.x),
					 -(pDrawable->y + pGC->clipOrg.y));
	}
    }	/* end of composite clip for a window */
    else
    {
	BoxRec          pixbounds;

	/* XXX should we translate by drawable.x/y here ? */
	pixbounds.x1 = 0;
	pixbounds.y1 = 0;
	pixbounds.x2 = pDrawable->width;
	pixbounds.y2 = pDrawable->height;

	if (pGC->freeCompClip)
	{
	    REGION_RESET(pGC->pScreen, pGC->pCompositeClip, &pixbounds);
	}
	else
	{
	    pGC->freeCompClip = TRUE;
	    pGC->pCompositeClip = REGION_CREATE(pGC->pScreen, &pixbounds, 1);
	}

	if (pGC->clientClipType == CT_REGION)
	{
	    REGION_TRANSLATE(pGC->pScreen, pGC->pCompositeClip,
					 -pGC->clipOrg.x, -pGC->clipOrg.y);
	    REGION_INTERSECT(pGC->pScreen, pGC->pCompositeClip,
				pGC->pCompositeClip, pGC->clientClip);
	    REGION_TRANSLATE(pGC->pScreen, pGC->pCompositeClip,
					 pGC->clipOrg.x, pGC->clipOrg.y);
	}
    }	/* end of composite clip for pixmap */
} 

/*
 * PclValidateGC()
 *
 * Unlike many screen GCValidate routines, this function should not need
 * to mess with setting the drawing functions.  Different drawing
 * functions are usually needed to optimize such things as drawing
 * wide or dashed lines; this functionality will be handled primarily
 * by the printer itself, while the necessary PCL code to set the
 * attributes will be done in PclUpdateDrawableGC().
 */

/*ARGSUSED*/
void
PclValidateGC(
     GCPtr pGC,
     unsigned long changes,
     DrawablePtr pDrawable)
{
    /*
     * Pixmaps should be handled by their respective validation
     * functions.
     */
    if( pDrawable->type == DRAWABLE_PIXMAP )
      {
          fbValidateGC(pGC, ~0, pDrawable);
	  return;
      }

    /*
     * Reset the drawing operations
     */
    pGC->ops = &PclGCOps;
    
    /*
     * Validate the information, and correct it if necessary.
     */

    /*
     * If necessary, compute the composite clip region.  (Code ripped
     * from migc.c)
     */
    if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
       )
    {
	PclComputeCompositeClip(pGC, pDrawable);
    }

    /*
     * PCL does not directly support the DoubleDash line style, nor is
     * there an easy way to simulate it, so we'll just change it to a
     * LineOnOffDash, which is supported by PCL.
     */
    if( ( changes & GCLineStyle ) && ( pGC->lineStyle == LineDoubleDash ) )
      pGC->lineStyle = LineOnOffDash;
    
    /*
     * Update the drawable's changeMask to reflect the changes made to the GC.
     */
/*
    PclSetDrawablePrivateGC( pDrawable, *pGC, changes );
*/
}
