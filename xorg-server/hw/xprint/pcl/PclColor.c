/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		PclColorInit.c
**    *
**    *  Contents:
**    *                 Colormap handing code of Pcl driver for the 
**    *                 print server.
**    *
**    *  Created:	4/8/96
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
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>

#include "colormapst.h"
#include "windowstr.h"
#include "resource.h"

#include "Pcl.h"
#include "fb.h"

static void lookup(unsigned char *src,
		unsigned char *dst,
		int num,
		unsigned char *map,
		int dim);
static void trilinear(unsigned char *p,
		unsigned char *out,
		unsigned char *d,
		int dim,
		unsigned char def);


/*
 * This seems to be (and is) a duplication of effort; one would think
 * that fbCreateDefColormap would be sufficient.  It almost is.  The
 * only change made in this function is that the black and white pixels
 * are allocated with three separate variables for red, green and blue
 * values, instead of the single variable in fbCreateDefColormap.  The
 * single variable leads to the one value being corrected by
 * ResolveColor three times, which leads to incorrect colors.
 */

Bool
PclCreateDefColormap(ScreenPtr pScreen)
{
    unsigned short wp_red = ~0, wp_green = ~0, wp_blue = ~0;
    unsigned short bp_red = 0, bp_green = 0, bp_blue = 0;
    VisualPtr	pVisual;
    ColormapPtr	cmap;
    Pixel wp, bp;
    
    for (pVisual = pScreen->visuals;
	 pVisual->vid != pScreen->rootVisual;
	 pVisual++)
	;

    if (CreateColormap(pScreen->defColormap, pScreen, pVisual, &cmap,
		       (pVisual->class & DynamicClass) ? AllocNone : AllocAll,
		       0)
	!= Success)
	return FALSE;
    wp = pScreen->whitePixel;
    bp = pScreen->blackPixel;
    if ((AllocColor(cmap, &wp_red, &wp_green, &wp_blue, &wp, 0) !=
       	   Success) ||
    	(AllocColor(cmap, &bp_red, &bp_green, &bp_blue, &bp, 0) !=
       	   Success))
    	return FALSE;

	pScreen->whitePixel = wp;
	pScreen->blackPixel = bp;
    
    (*pScreen->InstallColormap)(cmap);
    return TRUE;
}

/*
 * Add colormap to list of colormaps on screen
 */
Bool
PclCreateColormap(ColormapPtr pColor)
{
    PclCmapToContexts *new;
    PclScreenPrivPtr sPriv;

    sPriv = (PclScreenPrivPtr)
	dixLookupPrivate(&pColor->pScreen->devPrivates, PclScreenPrivateKey);

	/*
	 * Use existing code to initialize the values in the colormap
	 */
	fbInitializeColormap( pColor );

	/*
	 * Set up the mapping between the color map and the context
	 */
    new = (PclCmapToContexts *)xalloc( sizeof( PclCmapToContexts ) );

    if( new )
      {
	  new->colormapId = pColor->mid;
	  new->contexts = NULL;
	  new->next = sPriv->colormaps;
	  sPriv->colormaps = new;
	  
	  return TRUE;
      }
    else
      return FALSE;
}

void
PclDestroyColormap(ColormapPtr pColor)
{
    PclScreenPrivPtr sPriv;
    PclCmapToContexts *pCmap, *tCmap = 0;
    PclContextListPtr con, tCon;
    PclContextPrivPtr cPriv;
    PclPaletteMapPtr pPal;
    char t[80];

    /*
     * At DestroyContext time, colormaps may be destroyed twice, so if the
     * pointer is NULL, just crash out.
     */
    if( !pColor )
      return;
    
    /*
     * Find the colormap <-> contexts mapping 
     */
    sPriv = (PclScreenPrivPtr)
	dixLookupPrivate(&pColor->pScreen->devPrivates, PclScreenPrivateKey);
    pCmap = sPriv->colormaps;
    while( pCmap )
      {
	  if( pCmap->colormapId == pColor->mid )
	    break;
	  tCmap = pCmap;
	  pCmap = pCmap->next;
      }

    /*
     * For each context, delete the palette in the printer and
     * free the mapping.
     */
    if( pCmap )
      {
	  con = pCmap->contexts;
	  while( con )
	    {
		cPriv = dixLookupPrivate(&con->context->devPrivates,
					 PclContextPrivateKey);
		pPal = cPriv->palettes;
		while( pPal )
		  {
		      if( pPal->colormapId == pColor->mid )
			break;
		      pPal = pPal->next;
		  }
		
		if( cPriv->pPageFile )
		  {
		      sprintf( t, "\033&p%dI\033*p2C", pPal->paletteId );
		      SEND_PCL( cPriv->pPageFile, t );
		  }
		
		tCon = con;
		con = con->next;
		xfree( tCon );
	    }
	  
	  /*
	   * Delete the colormap<->contexts mapping
	   */
	  if( sPriv->colormaps == pCmap )
	    /* Delete from the front */
	    sPriv->colormaps = pCmap->next;
	  else
	    /* Delete from the middle */
	    tCmap->next = pCmap->next;
	  free( pCmap );
      }
}

void
PclInstallColormap(ColormapPtr pColor)
{
}

void
PclUninstallColormap(ColormapPtr pColor)
{
}

int
PclListInstalledColormaps(ScreenPtr pScreen,
			  XID *pCmapList)
{
    return 0;
}

void
PclStoreColors(ColormapPtr pColor,
	       int ndef,
	       xColorItem *pdefs)
{
    PclCmapToContexts *p;
    PclScreenPrivPtr sPriv;
    PclContextListPtr con;
    PclContextPrivPtr cPriv;
    PclPaletteMapPtr pMap;
    char t[80];
    int i;

    sPriv = (PclScreenPrivPtr)
	dixLookupPrivate(&pColor->pScreen->devPrivates, PclScreenPrivateKey);
    p = sPriv->colormaps;
    while( p )
      {
	  if( p->colormapId == pColor->mid )
	    break;
	  p = p->next;
      }

    if( p )
      {
	  con = p->contexts;
	  while( con )
	    {
		/*
		 * For each context, get the palette ID and update the
		 * appropriate palette.
		 */
		cPriv = dixLookupPrivate(&con->context->devPrivates,
					 PclContextPrivateKey);
		pMap = PclFindPaletteMap( cPriv, pColor, NULL );

		/*
		 * Update the palette
		 */
		sprintf( t, "\033&p%dS", pMap->paletteId );
		SEND_PCL( cPriv->pPageFile, t );
		
		if( pColor->class == PseudoColor )
		  {
		      unsigned short r, g, b;
		      unsigned int pID;
		      for( i = 0; i < ndef; i++ )
			{
			    pID = pdefs[i].pixel;
			    if ( pColor->red[i].fShared )
			      {
				  r = pColor->red[pID].co.shco.red->color;
				  g = pColor->red[pID].co.shco.green->color;
				  b = pColor->red[pID].co.shco.blue->color;
			      }
			    else
			      {
				  r = pColor->red[pID].co.local.red;
				  g = pColor->red[pID].co.local.green;
				  b = pColor->red[pID].co.local.blue;
			      }

			    if( pdefs[i].flags & DoRed )
				  r = pdefs[i].red;
			    if( pdefs[i].flags & DoGreen )
				  g = pdefs[i].green;
			    if( pdefs[i].flags & DoBlue )
				  b = pdefs[i].blue;
			    PclLookUp(pColor, cPriv, &r, &g, &b);
			    sprintf( t, "\033*v%ua%ub%uc%dI", r, g, b, pID);
			    SEND_PCL( cPriv->pPageFile, t );
			}
		  }    

		sprintf( t, "\033&p%dS", cPriv->currentPalette );
		SEND_PCL( cPriv->pPageFile, t );

		con = con->next;
	    }
      }
}

void
PclResolveColor(unsigned short *pRed,
		unsigned short *pGreen,
		unsigned short *pBlue,
		VisualPtr pVisual)
{
    /*
     * We need to map the X color range of [0,65535] to the PCL color
     * range of [0,32767].
     */
    *pRed >>= 1;
    *pGreen >>= 1;
    *pBlue >>= 1;
}

PclPaletteMapPtr
PclFindPaletteMap(PclContextPrivPtr cPriv,
		  ColormapPtr cmap,
		  GCPtr gc)
{
    PclPaletteMapPtr p = cPriv->palettes, new;

    /*
     * If the colormap is static, grab one of the special palettes.  If we come
     * into this from StoreColors, there will be no GC, but by definition we're
     * looking at a dynamic color map, so the special colors will not be
     * needed.
     */
    if( gc )
      {
	  if( cmap->pVisual->class == StaticGray )
	    return &( cPriv->staticGrayPalette );
	  else if( cmap->pVisual->class == TrueColor )
	    {
		if( gc->fillStyle == FillTiled && !( gc->tileIsPixel ) )
		  return &( cPriv->specialTrueColorPalette );
		else
		  return &( cPriv->trueColorPalette );
	    }
      }
    
    
    /* Look for the colormap ID <-> palette ID mapping */
    while( p )
      {
	  if( p->colormapId == cmap->mid )
	    return p;
	  p = p->next;
      }

    /* If the colormap isn't already there, make an entry for it */
    new = (PclPaletteMapPtr)xalloc( sizeof( PclPaletteMap ) );
    new->colormapId = cmap->mid;
    new->paletteId = cPriv->nextPaletteId++;
    new->downloaded = 0;
    new->next = cPriv->palettes;
    cPriv->palettes = new;
    return new;
}

int
PclUpdateColormap(DrawablePtr pDrawable,
		  XpContextPtr pCon,
		  GCPtr gc,
		  FILE *outFile)
{
    PclScreenPrivPtr sPriv;
    
    PclContextPrivPtr cPriv;
    PclPaletteMapPtr pMap;
    PclCmapToContexts *pCmap;
    PclContextListPtr new;
    char t[80];
    Colormap c;
    ColormapPtr cmap;
    WindowPtr win = (WindowPtr)pDrawable;
    unsigned short r, g, b, rr, gg, bb;
    int i;

    cPriv = (PclContextPrivPtr)
	dixLookupPrivate(&pCon->devPrivates, PclContextPrivateKey);
    
    c = wColormap( win );
    cmap = (ColormapPtr)LookupIDByType( c, RT_COLORMAP );
    pMap = PclFindPaletteMap( cPriv, cmap, gc );
    
    if( cPriv->currentPalette == pMap->paletteId )
      /*
       * If the requested colormap is already active, nothing needs to
       * be done.
       */
      return FALSE;

    /*
     * Now we activate the palette in the printer
     */
    sprintf( t, "\033&p%dS", pMap->paletteId );
    SEND_PCL( outFile, t );
    cPriv->currentPalette = pMap->paletteId;

    if( pMap->downloaded == 0 )
      /*
       * If the requested colormap has not been downloaded to the
       * printer, we need to do that before activating it.
       */
      {
	  /*
	   * Add the colormap to the screen-level colormap<->context mapping.
	   */
	  sPriv = (PclScreenPrivPtr)
	      dixLookupPrivate(&cmap->pScreen->devPrivates,
			       PclScreenPrivateKey);
	  pCmap = sPriv->colormaps;
	  while( pCmap && ( pCmap->colormapId != cmap->mid ) )
		pCmap = pCmap->next;
	  new = (PclContextListPtr)xalloc( sizeof( PclContextList ) );
	  new->context = pCon;
	  new->next = pCmap->contexts;
	  pCmap->contexts = new;

	  /*
	   * XXX Download the colormap
	   */
	  if( cmap->class == StaticGray )
	    {
#ifdef XP_PCL_COLOR
		sprintf( t, "\033*v18W%c%c%c%c%c%c", 0, 1, 1, 1, 1, 1 );
		SEND_PCL_COUNT( cPriv->pPageFile, t, 12 );
		
		/* Send the white reference point... */
		sprintf( t, "%c%c%c%c%c%c", 0x7f, 0xff, 0x7f, 0xff,
			0x7f, 0xff );
		SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );

		/* ... and the black reference point */
		sprintf( t, "%c%c%c%c%c%c", 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00 );
		SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );
		
		/* Now program the two colors */
		sprintf( t, "\033*v0a0b0c%ldI", (long) cmap->pScreen->blackPixel );
		SEND_PCL( cPriv->pPageFile, t );
		sprintf( t, "\033*v32767a32767b32767c%ldI",
			(long) cmap->pScreen->whitePixel );
		SEND_PCL( cPriv->pPageFile, t );
#endif /* XP_PCL_COLOR */
	    }
	  else if( cmap->class == PseudoColor )
	    {
		sprintf( t,
			"\033*v18W%c%c%c%c%c%c",
			0, 1, cmap->pVisual->nplanes, 16, 16, 16 );
		SEND_PCL_COUNT( cPriv->pPageFile, t, 12 );
		
		/* Send the white reference point... */
		if ( cPriv->ctbl != NULL )
		    sprintf( t, "%c%c%c%c%c%c", 0x00, 0xff, 0x00, 0xff,
			0x00, 0xff );
		else
		    sprintf( t, "%c%c%c%c%c%c", 0x7f, 0xff, 0x7f, 0xff,
			0x7f, 0xff );
		SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );

		/* ... and the black reference point */
		sprintf( t, "%c%c%c%c%c%c", 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00 );
		SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );
		
		for(i = 0; i < cmap->pVisual->ColormapEntries; i++ )
		  {
		      if( cmap->red[i].fShared )
			{
			    r = cmap->red[i].co.shco.red->color;
			    g = cmap->red[i].co.shco.green->color;
			    b = cmap->red[i].co.shco.blue->color;
			}
		      else
			{
			    r = cmap->red[i].co.local.red;
			    g = cmap->red[i].co.local.green;
			    b = cmap->red[i].co.local.blue;
			}
		      PclLookUp(cmap, cPriv, &r, &g, &b);
		      sprintf( t, "\033*v%ua%ub%uc%dI", r, g, b, i );
		      SEND_PCL( outFile, t );
		  }
	    }
	  else if( cmap->class == TrueColor )
	    {
		unsigned short lim;

		if( gc->fillStyle == FillTiled && !( gc->tileIsPixel ) )
		  {
		      if( cPriv->ctbl != NULL )
			{
			   /* Send the "special" colormap for 24-bit fills */
			   sprintf( t, "\033*v18W%c%c%c%c%c%c", 0, 1, 
					8,
					cmap->pVisual->bitsPerRGBValue,
					cmap->pVisual->bitsPerRGBValue,
					cmap->pVisual->bitsPerRGBValue );
			   SEND_PCL_COUNT( cPriv->pPageFile, t, 12 );

			   /* Send the white reference point... */
			   sprintf( t, "%c%c%c%c%c%c",
					0x00, 0xff,
					0x00, 0xff,
					0x00, 0xff );
			   SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );

			   /* ... and the black reference point */
			   sprintf( t, "%c%c%c%c%c%c",
					0x00, 0x00,
					0x00, 0x00,
					0x00, 0x00 );
			   SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );

			   /* Now send the color entries, RRRGGGBB */
			   i=0;
			   for( r = 0; r < 8; r++ )
			     for( g = 0; g < 8; g ++ )
			       for( b = 0; b < 4; b++ )
				  {
				      rr = (r * 0xff)/7;
				      gg = (g * 0xff)/7;
				      bb = (b * 0xff)/3;
				      PclLookUp(cmap, cPriv, &rr, &gg, &bb);
				      sprintf( t, "\033*v%ua%ub%uc%dI",
								rr, gg, bb, i );
				      SEND_PCL( outFile, t );
				      i++;
				   }
			}
		      else
			{
			   /* Send the "special" colormap for 24-bit fills */
			   sprintf( t, "\033*v18W%c%c%c%c%c%c", 0, 1, 
				8,
				cmap->pVisual->bitsPerRGBValue,
				cmap->pVisual->bitsPerRGBValue,
				cmap->pVisual->bitsPerRGBValue );
			   SEND_PCL_COUNT( cPriv->pPageFile, t, 12 );

			   /* Send the white reference point... */
			   sprintf( t, "%c%c%c%c%c%c",
					0x00, 0x07,
					0x00, 0x07,
					0x00, 0x03 );
			   SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );

			   /* ... and the black reference point */
			   sprintf( t, "%c%c%c%c%c%c",
					0x00, 0x00,
					0x00, 0x00,
					0x00, 0x00 );
			   SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );

			   /* Now send the color entries, RRRGGGBB */
			   i=0;
			   for( r = 0; r < 8; r++ )
			     for( g = 0; g < 8; g ++ )
			       for( b = 0; b < 4; b++ )
				  {
				      sprintf( t, "\033*v%ua%ub%uc%dI",
								r, g, b, i );
				      SEND_PCL( outFile, t );
				      i++;
				   }
			}

		  }
		else
		  {
		      lim = (1 << cmap->pVisual->bitsPerRGBValue) - 1;

		      /* Send the "special" colormap for 24-bit fills */
		      sprintf( t, "\033*v18W%c%c%c%c%c%c", 0, 3, 
				cmap->pVisual->nplanes,
				cmap->pVisual->bitsPerRGBValue,
				cmap->pVisual->bitsPerRGBValue,
				cmap->pVisual->bitsPerRGBValue );
		      SEND_PCL_COUNT( cPriv->pPageFile, t, 12 );
		     
		      /* Send the white reference point... */
		      sprintf( t, "%c%c%c%c%c%c",
				(lim >> 8) & 0xff, lim & 0xff,
				(lim >> 8) & 0xff, lim & 0xff,
				(lim >> 8) & 0xff, lim & 0xff);
		      SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );

		      /* ... and the black reference point */
		      sprintf( t, "%c%c%c%c%c%c", 0x00, 0x00, 0x00, 0x00,
			      0x00, 0x00 );
		      SEND_PCL_COUNT( cPriv->pPageFile, t, 6 );
		  }

	    }
	  pMap->downloaded = 1;
      }
      return TRUE;
    
}    

void PclLookUp(
    ColormapPtr cmap,
    PclContextPrivPtr cPriv,
    unsigned short *r, 
    unsigned short *g, 
    unsigned short *b
)
{
    unsigned char cdata[3];

    if( cmap->class == PseudoColor )
      {
	if( cPriv->ctbl != NULL )
	  {
	    cdata[0] = *r >> 8;
	    cdata[1] = *g >> 8;
	    cdata[2] = *b >> 8;
	    lookup(cdata, cdata, 1, cPriv->ctbl, cPriv->ctbldim);
	    *r = cdata[0];
	    *g = cdata[1];
	    *b = cdata[2];
	  }
	else
	  {
	    *r >>= 1;
	    *g >>= 1;
	    *b >>= 1;
	  }
      }
    else if( cmap->class == TrueColor )
      {
	if( cPriv->ctbl != NULL )
	  {
	    cdata[0] = *r;
	    cdata[1] = *g;
	    cdata[2] = *b;
	    lookup(cdata, cdata, 1, cPriv->ctbl, cPriv->ctbldim);
	    *r = cdata[0];
	    *g = cdata[1];
	    *b = cdata[2];
	  }
      }
    return;
}

unsigned char *PclReadMap(char *name, int *dim)
{
    FILE *fp;
    unsigned char *data;
    long size;

    if ((fp=fopen(name, "r")) == NULL) {
	return(NULL);
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    
    /* Could do this with a lookup table, if the constraint is that the
       3 map dimensions must be equal. */
    switch (size) {
    case 8*8*8*3:
	*dim = 8;
	break;
    case 16*16*16*3:
	*dim = 16;
	break;
    case 17*17*17*3:
	*dim = 17;
	break;
    case 65*65*65*3:
	*dim = 65;
	break;
    default:
	fclose(fp);
	return(NULL);
    }
    
    if ((data = (unsigned char *) xalloc(sizeof(char) * size)) == NULL) {
	fclose(fp);
	return(NULL);
    }

    fseek(fp, 0, SEEK_SET);

    if (fread(data, sizeof(char), size, fp) != (unsigned) size) {
	fclose(fp);
	free(data);
	return(NULL);
    }

    fclose(fp);
    return(data);
}

/************************************************************************
 *
 * Here is the mapper.
 *
 ************************************************************************/

#define SCL(x) ((x)*(dim-1)/255)
/* Interleaved-map lookup */
static void lookup(unsigned char *src, unsigned char *dst, int num, unsigned char *map, int dim)
{
    int i;

#define _INTERPOLATE
#ifndef _INTERPOLATE
    unsigned char *p1, *p2, *p3;

    for (i=0; i<num; i++) {
	p1 = map + (SCL(src[0])*dim*dim + SCL(src[1])*dim + SCL(src[2])) * 3;
	*dst++ = *p1++;
	*dst++ = *p1++;
	*dst++ = *p1++;
	src += 3;
    }
#else
    for (i=0; i<num; i++) {
	trilinear(src, dst, map, dim, 128);
	src += 3;
	dst += 3;
    }
#endif
}

/*
 * C code from the article
 * "Tri-linear Interpolation"
 * by Steve Hill, sah@ukc.ac.uk
 * in "Graphics Gems IV", Academic Press, 1994
 *
 * Fri Feb 16 14:12:43 PST 1996
 *	Modified to use for 8-bit color mapping -- A. Fitzhugh, 
 * 	HP Labs, Printing Technology Department
 */

/* linear interpolation from l (when a=0) to h (when a=1)*/
/* (equal to (a*h)+((1-a)*l) */
#define LERP(a,l,h)	((l)+((((h)-(l))*(a))>>8))

static void trilinear(unsigned char *p, unsigned char *out, unsigned char *d, int dim, unsigned char def)
{
#define DENS(X, Y, Z, ch) d[((X*dim+Y)*dim+Z)*3+ch]
    
    int	x0, y0, z0, 
	x1, y1, z1,
	i;
    unsigned char *dp,
	fx, fy, fz,
	d000, d001, d010, d011,
	d100, d101, d110, d111,
	dx00, dx01, dx10, dx11,
	dxy0, dxy1;
    float scale;
    
    scale = 255.0 / (dim-1);

    x0 = p[0] / scale;
    y0 = p[1] / scale;
    z0 = p[2] / scale;

    /* Fractions should range from 0-1.0 (fixed point 8-256) */
    fx = (((int) (p[0] - x0 * scale)) << 8) / 255;
    fy = (((int) (p[1] - y0 * scale)) << 8) / 255;
    fz = (((int) (p[2] - z0 * scale)) << 8) / 255;
    
    x1 = x0 + 1;
    y1 = y0 + 1;
    z1 = z0 + 1;
    
    for (i=0; i<3; i++) {
	
	if (x0 >= 0 && x1 < dim &&
	    y0 >= 0 && y1 < dim &&
	    z0 >= 0 && z1 < dim) {
	    dp = &DENS(x0, y0, z0, i);
	    d000 = dp[0];
	    d100 = dp[3];
	    dp += dim*3;
	    d010 = dp[0];
	    d110 = dp[3];
	    dp += dim*dim*3;
	    d011 = dp[0];
	    d111 = dp[3];
	    dp -= dim*3;
	    d001 = dp[0];
	    d101 = dp[3];
	} else {
#	define INRANGE(X, Y, Z) \
	    ((X) >= 0 && (X) < dim && \
	     (Y) >= 0 && (Y) < dim && \
	     (Z) >= 0 && (Z) < dim)
	    
	    d000 = INRANGE(x0, y0, z0) ? DENS(x0, y0, z0, i) : def;
	    d001 = INRANGE(x0, y0, z1) ? DENS(x0, y0, z1, i) : def;
	    d010 = INRANGE(x0, y1, z0) ? DENS(x0, y1, z0, i) : def;
	    d011 = INRANGE(x0, y1, z1) ? DENS(x0, y1, z1, i) : def;
	    
	    d100 = INRANGE(x1, y0, z0) ? DENS(x1, y0, z0, i) : def;
	    d101 = INRANGE(x1, y0, z1) ? DENS(x1, y0, z1, i) : def;
	    d110 = INRANGE(x1, y1, z0) ? DENS(x1, y1, z0, i) : def;
	    d111 = INRANGE(x1, y1, z1) ? DENS(x1, y1, z1, i) : def;
	}
	
	dx00 = LERP(fx, d000, d100);
	dx01 = LERP(fx, d001, d101);
	dx10 = LERP(fx, d010, d110);
	dx11 = LERP(fx, d011, d111);
	
	dxy0 = LERP(fy, dx00, dx10);
	dxy1 = LERP(fy, dx01, dx11);
	
	out[i] = LERP(fz, dxy0, dxy1);
    }
}
    
