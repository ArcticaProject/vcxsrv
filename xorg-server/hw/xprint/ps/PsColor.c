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
**    *  File:		PsColor.c
**    *
**    *  Contents:	Color routines for the PS driver
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
#include "mi.h"
#include "micmap.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "colormapst.h"

Bool
PsCreateColormap(ColormapPtr pColor)
{
  return miInitializeColormap(pColor);
}

void
PsDestroyColormap(ColormapPtr pColor)
{
  /* NO-OP */
}

void
PsInstallColormap(ColormapPtr pColor)
{
  miInstallColormap(pColor);
}

void
PsUninstallColormap(ColormapPtr pColor)
{
  miUninstallColormap(pColor);
}

int
PsListInstalledColormaps(
  ScreenPtr pScreen,
  XID      *pCmapList)
{
  return miListInstalledColormaps(pScreen, pCmapList);
}

void
PsStoreColors(
  ColormapPtr  pColor,
  int          ndef,
  xColorItem  *pdefs)
{
  int  i;
  for( i=0 ; i<ndef ; i++ )
  {
    if( pdefs[i].flags&DoRed )
      pColor->red[pdefs[i].pixel].co.local.red   = pdefs[i].red;
    if( pdefs[i].flags&DoGreen )
      pColor->red[pdefs[i].pixel].co.local.green = pdefs[i].green;
    if( pdefs[i].flags&DoBlue )
      pColor->red[pdefs[i].pixel].co.local.blue  = pdefs[i].blue;
  }
}

void
PsResolveColor(
  unsigned short *pRed,
  unsigned short *pGreen,
  unsigned short *pBlue,
  VisualPtr       pVisual)
{
  miResolveColor(pRed, pGreen, pBlue, pVisual);
}

PsOutColor
PsGetPixelColor(ColormapPtr cMap, int pixval)
{
  VisualPtr v = cMap->pVisual;
  switch( v->class )
  {
    case TrueColor:
    {
        PsOutColor p = pixval;       
        PsOutColor r, g, b;

        r = (p & v->redMask)   >> v->offsetRed;
        g = (p & v->greenMask) >> v->offsetGreen;
        b = (p & v->blueMask)  >> v->offsetBlue;

        r = cMap->red[r].co.local.red;
        g = cMap->green[g].co.local.green;
        b = cMap->blue[b].co.local.blue;
                
#ifdef PSOUT_USE_DEEPCOLOR
        return((r<<32)|(g<<16)|b);
#else
        r >>= 8;
        g >>= 8;
        b >>= 8;

        return((r<<16)|(g<<8)|b);
#endif /* PSOUT_USE_DEEPCOLOR */
    }
    case PseudoColor:
    case GrayScale:
    case StaticGray:
    {
        PsOutColor r, g, b;
                  
        if( pixval < 0 || pixval > v->ColormapEntries)
          return(0);

        r = cMap->red[pixval].co.local.red;
        g = cMap->red[pixval].co.local.green;
        b = cMap->red[pixval].co.local.blue;

        if ((v->class | DynamicClass) == GrayScale)
        {
          /* rescale to gray (see |miResolveColor()|) */
          r = g = b = (30L*r + 59L*g + 11L*b) / 100L;
        }
        
#ifdef PSOUT_USE_DEEPCOLOR
        return((r<<32)|(g<<16)|b);
#else
        r >>= 8;
        g >>= 8;
        b >>= 8;

        return((r<<16)|(g<<8)|b);
#endif /* PSOUT_USE_DEEPCOLOR */
    }
    default:
        FatalError("PsGetPixelColor: Unsupported visual %x\n",
                   (int)cMap->pVisual->class);
        break;
  }
  
  return 0; /* NO-OP*/
}

void
PsSetFillColor(DrawablePtr pDrawable, GCPtr pGC, PsOutPtr psOut,
               ColormapPtr cMap)
{
  switch(pGC->fillStyle)
  {
    case FillSolid:
      PsOut_Color(psOut, PsGetPixelColor(cMap, pGC->fgPixel));
      break;
    case FillTiled:
      if( !PsOut_BeginPattern(psOut, pGC->tile.pixmap,
             pGC->tile.pixmap->drawable.width,
             pGC->tile.pixmap->drawable.height, PsTile, 0, 0) )
      {
        PsReplayPixmap(pGC->tile.pixmap, pDrawable);
        PsOut_EndPattern(psOut);
      }
      PsOut_SetPattern(psOut, pGC->tile.pixmap, PsTile);
      break;
    case FillStippled:
      if( !PsOut_BeginPattern(psOut, pGC->stipple,
             pGC->stipple->drawable.width,
             pGC->stipple->drawable.height, PsStip, 0,
             PsGetPixelColor(cMap, pGC->fgPixel)) )
      {
        PsReplayPixmap(pGC->stipple, pDrawable);
        PsOut_EndPattern(psOut);
      }
      PsOut_SetPattern(psOut, pGC->stipple, PsStip);
      break;
    case FillOpaqueStippled:
      if( !PsOut_BeginPattern(psOut, pGC->stipple,
             pGC->stipple->drawable.width,
             pGC->stipple->drawable.height, PsOpStip,
             PsGetPixelColor(cMap, pGC->bgPixel),
             PsGetPixelColor(cMap, pGC->fgPixel)) )
      {
        PsReplayPixmap(pGC->stipple, pDrawable);
        PsOut_EndPattern(psOut);
      }
      PsOut_SetPattern(psOut, pGC->stipple, PsOpStip);
      break;
  }
}
