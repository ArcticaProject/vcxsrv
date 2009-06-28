/*

Copyright (c) 1990  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.


Copyright IBM Corporation 1987,1988,1989
All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that 
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of IBM not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

*/

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include <stdlib.h>

#include "xf4bpp.h"
#include "mfbmap.h"
#include "mfb.h"
#include "mi.h"
#include "micmap.h"
#include "scrnintstr.h"
#include "vgaVideo.h"

#if 0
/* XXX This remains to remind of the PC98 difference */
static VisualRec vgaVisuals[] = {
/* StaticColor needs to be first so is can be used as the default */
/* vid class     bpRGB cmpE nplan rMask gMask bMask oRed oGreen oBlue */
#ifdef	PC98
{   0, StaticColor, 4, 1 << VGA_MAXPLANES, VGA_MAXPLANES, 0, 0, 0, 0, 0, 0 },
{   0, StaticGray,  4, 1 << VGA_MAXPLANES, VGA_MAXPLANES, 0, 0, 0, 0, 0, 0 },
{   0, GrayScale,   4, 1 << VGA_MAXPLANES, VGA_MAXPLANES, 0, 0, 0, 0, 0, 0 },
{   0, PseudoColor, 4, 1 << VGA_MAXPLANES, VGA_MAXPLANES, 0, 0, 0, 0, 0, 0 },
#else
{   0, StaticColor, 6, 1 << VGA_MAXPLANES, VGA_MAXPLANES, 0, 0, 0, 0, 0, 0 },
{   0, StaticGray,  6, 1 << VGA_MAXPLANES, VGA_MAXPLANES, 0, 0, 0, 0, 0, 0 },
{   0, GrayScale,   6, 1 << VGA_MAXPLANES, VGA_MAXPLANES, 0, 0, 0, 0, 0, 0 },
{   0, PseudoColor, 6, 1 << VGA_MAXPLANES, VGA_MAXPLANES, 0, 0, 0, 0, 0, 0 },
#endif
} ;
#endif

void
xf4bppNeverCalled()
{
	FatalError("xf4bppNeverCalled was nevertheless called\n");
}

/*ARGSUSED*/
static Bool
vgaScreenClose
(
	int       idx,
	ScreenPtr pScreen
)
{
	pScreen->defColormap = 0 ;
	return TRUE;
}


static GCPtr sampleGCperDepth[MAXFORMATS+1] = { 0 };
static PixmapPtr samplePixmapPerDepth[1] = { 0 };

/* GJA -- Took this from miscrinit.c.
 * We want that devKind contains the distance in bytes between two scanlines.
 * The computation that mi does is not appropriate for planar VGA.
 * Therefore we provide here our own routine.
 */

/* GJA -- WARNING: this is an internal structure declaration, taken from
 * miscrinit.c
 */
typedef struct
{
    pointer pbits; /* pointer to framebuffer */
    int width;    /* delta to add to a framebuffer addr to move one row down */
} miScreenInitParmsRec, *miScreenInitParmsPtr;

/* With the introduction of pixmap privates, the "screen pixmap" can no
 * longer be created in miScreenInit, since all the modules that could
 * possibly ask for pixmap private space have not been initialized at
 * that time.  pScreen->CreateScreenResources is called after all
 * possible private-requesting modules have been inited; we create the
 * screen pixmap here.
 */
static Bool
v16CreateScreenResources
(
    ScreenPtr pScreen
)
{
    miScreenInitParmsPtr pScrInitParms;
    pointer value;

    pScrInitParms = (miScreenInitParmsPtr)pScreen->devPrivate;

    /* if width is non-zero, pScreen->devPrivate will be a pixmap
     * else it will just take the value pbits
     */
    if (pScrInitParms->width)
    {
	PixmapPtr pPixmap;

	/* create a pixmap with no data, then redirect it to point to
	 * the screen
	 */
	pPixmap = (*pScreen->CreatePixmap)(pScreen, 0, 0, pScreen->rootDepth, 0);
	if (!pPixmap)
	    return FALSE;

	if (!(*pScreen->ModifyPixmapHeader)(pPixmap, pScreen->width,
		    pScreen->height, pScreen->rootDepth, 8 /* bits per pixel */,
/* GJA: was 	    PixmapBytePad(pScrInitParms->width, pScreen->rootDepth), */
#define BITS_PER_BYTE_SHIFT 3
		    pScrInitParms->width >> BITS_PER_BYTE_SHIFT,
		    pScrInitParms->pbits))
	    return FALSE;
	value = (pointer)pPixmap;
    }
    else
    {
	value = pScrInitParms->pbits;
    }
    xfree(pScreen->devPrivate); /* freeing miScreenInitParmsRec */
    pScreen->devPrivate = value; /* pPixmap or pbits */
    return TRUE;
}


Bool
xf4bppScreenInit( pScreen, pbits, virtx, virty, dpix, dpiy, width )
    ScreenPtr pScreen;
    pointer pbits;
    int virtx, virty;
    int dpix, dpiy;
    int width;
{
  Bool ret;
  VisualPtr visuals;
  DepthPtr depths;
  int nvisuals;
  int ndepths;
  int rootdepth;
  VisualID defaultVisual;

  rootdepth = 0;
  ret = miInitVisuals(&visuals, &depths, &nvisuals, &ndepths, &rootdepth,
		      &defaultVisual, (unsigned long)1 << 8, 6, -1);
  if (!ret)
	return FALSE;

  pScreen-> id = 0;
  pScreen->defColormap = FakeClientID(0);
  pScreen-> whitePixel = VGA_WHITE_PIXEL;
  pScreen-> blackPixel = VGA_BLACK_PIXEL;
  pScreen-> rgf = 0;
  *(pScreen-> GCperDepth) = *(sampleGCperDepth);
  *(pScreen-> PixmapPerDepth) = *(samplePixmapPerDepth);
  pScreen-> CloseScreen = vgaScreenClose;
  pScreen-> QueryBestSize = xf4bppQueryBestSize;
  pScreen-> GetImage = xf4bppGetImage;
  pScreen-> GetSpans = xf4bppGetSpans;
  pScreen-> CreateWindow = xf4bppCreateWindowForXYhardware;
  pScreen-> DestroyWindow = xf4bppDestroyWindow;
  pScreen-> PositionWindow = xf4bppPositionWindow;
  pScreen-> CopyWindow = xf4bppCopyWindow;
  pScreen-> CreatePixmap = xf4bppCreatePixmap;
  pScreen-> CreateGC = xf4bppCreateGC;
  pScreen-> CreateColormap = xf4bppInitializeColormap;
  pScreen-> DestroyColormap = (DestroyColormapProcPtr)NoopDDA;
  pScreen-> InstallColormap = miInstallColormap;
  pScreen-> UninstallColormap = miUninstallColormap;
  pScreen-> ListInstalledColormaps = miListInstalledColormaps;
  pScreen-> StoreColors = (StoreColorsProcPtr)NoopDDA;
  pScreen-> ResolveColor = xf4bppResolveColor;
  mfbFillInScreen(pScreen);

  if (!mfbAllocatePrivates(pScreen, NULL))
	return FALSE;

  if (!miScreenInit(pScreen, pbits, virtx, virty, dpix, dpiy, width,
	rootdepth, ndepths, depths, defaultVisual /* See above */,
	nvisuals, visuals))
     return FALSE;

  /* GJA -- Now we override the supplied default: */
  pScreen -> CreateScreenResources = v16CreateScreenResources;

  mfbRegisterCopyPlaneProc(pScreen,miCopyPlane); /* GJA -- R4->R5 */
  return TRUE;
}
