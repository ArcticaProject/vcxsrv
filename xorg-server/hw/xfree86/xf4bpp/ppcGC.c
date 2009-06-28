/*

Copyright (c) 1987  X Consortium

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


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts,
All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
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
#include "scrnintstr.h"
#include "ppcGCstr.h"
#include "vgaVideo.h"
#include "ibmTrace.h"

#define ppcGCInterestValidateMask \
( GCLineStyle | GCLineWidth | GCJoinStyle | GCBackground | GCForeground \
| GCFunction | GCPlaneMask | GCFillStyle | GC_CALL_VALIDATE_BIT         \
| GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode )

static void xf4bppValidateGC(GCPtr, unsigned long, DrawablePtr);
static void xf4bppDestroyGC(GC *);

static GCFuncs vgaGCFuncs = {
	xf4bppValidateGC,
	(void (*)(GCPtr, unsigned long))NoopDDA,
	(void (*)(GCPtr, unsigned long, GCPtr))NoopDDA,
	xf4bppDestroyGC,
	xf4bppChangeClip,
	xf4bppDestroyClip,
	xf4bppCopyClip,
	{ NULL }
};


static ppcPrivGC vgaPrototypeGCPriv = {
	GXcopy,	/* unsigned char	rop */
	0,	/* unsigned char	ropOpStip */
	0,	/* unsigned char	ropFillArea */
	{0, },	/* unsigned char	unused[sizeof(long) - 3] */
	NULL,	/* mfbFillAreaProcPtr  	FillArea */
		{
		    VGA_ALLPLANES,	/* unsigned long	planemask */
		    1,			/* unsigned long	fgPixel */
		    0,			/* unsigned long	bgPixel */
		    GXcopy,		/* int			alu */
		    FillSolid,		/* int			fillStyle */
		}, /* ppcReducedRrop	colorRrop  */
	-1,	/* short lastDrawableType */
	-1,	/* short lastDrawableDepth */
	0	/* pointer devPriv */
} ;

static GCOps vgaGCOps = {
	xf4bppSolidWindowFS,	/*  void (* FillSpans)() */
	xf4bppSetSpans,		/*  void (* SetSpans)()	 */
	miPutImage,		/*  void (* PutImage)()	 */
	xf4bppCopyArea,		/*  RegionPtr (* CopyArea)()	 */
	miCopyPlane,		/*  void (* CopyPlane)() */
	xf4bppPolyPoint,		/*  void (* PolyPoint)() */
	miZeroLine,		/*  void (* Polylines)() */
	miPolySegment,		/*  void (* PolySegment)() */
	miPolyRectangle,	/*  void (* PolyRectangle)() */
	xf4bppZeroPolyArc,		/*  void (* PolyArc)()	 */
	miFillPolygon,		/*  void (* FillPolygon)() */
	miPolyFillRect,		/*  void (* PolyFillRect)() */
	xf4bppPolyFillArc,		/*  void (* PolyFillArc)() */
	miPolyText8,		/*  int (* PolyText8)()	 */
	miPolyText16,		/*  int (* PolyText16)() */
	miImageText8,		/*  void (* ImageText8)() */
	miImageText16,		/*  void (* ImageText16)() */
	xf4bppImageGlyphBlt,	/*  GJA -- void (* ImageGlyphBlt)() */
	miPolyGlyphBlt,		/*  GJA -- void (* PolyGlyphBlt)() */
	miPushPixels,		/*  void (* PushPixels)() */
	{NULL}			/* devPrivate */
};

Bool
xf4bppCreateGC( pGC )
register GCPtr pGC ;
{
	ppcPrivGC *pPriv ;
	GCOps *pOps ;

	if ( pGC->depth == 1 )
		{
		return (mfbCreateGC(pGC));
		}

	if ( !( pPriv = xalloc( sizeof( ppcPrivGC ) ) ) )
		return FALSE ;

	if ( !( pOps = xalloc( sizeof( GCOps ) ) ) ) {
		xfree(pPriv);
		return FALSE;
	}
	
        /* Now we initialize the GC fields */
	pGC->miTranslate = 1;
	pGC->unused = 0;
	pGC->planemask = VGA_ALLPLANES;
	pGC->fgPixel = VGA_BLACK_PIXEL;
	pGC->bgPixel = VGA_WHITE_PIXEL;
	pGC->funcs = &vgaGCFuncs;
	/* ops, -- see below */

	pGC->fExpose = TRUE;
	pGC->freeCompClip = FALSE;

	/* GJA: I don't like this code:
         * they allocated a mfbPrivGC, ignore the allocated data and place
         * a pointer to a ppcPrivGC in its slot.
         */
	*pPriv = vgaPrototypeGCPriv;
	dixSetPrivate(&pGC->devPrivates, mfbGetGCPrivateKey(), pPriv);

	/* Set the vgaGCOps */
	*pOps = vgaGCOps;
	pOps->devPrivate.val = 1;
	pGC->ops = pOps;

	return TRUE ;
}

static void
xf4bppDestroyGC( pGC )
    register GC	*pGC ;

{
    TRACE( ( "xf4bppDestroyGC(pGC=0x%x)\n", pGC ) ) ;

    if ( pGC->freeCompClip && pGC->pCompositeClip )
	REGION_DESTROY(pGC->pScreen, pGC->pCompositeClip);
    if(pGC->ops->devPrivate.val) xfree( pGC->ops );
    xfree(dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey()));
    return ;
}

static Mask
ppcChangePixmapGC
(
	register GC *pGC,
	register Mask changes
)
{
register ppcPrivGCPtr devPriv = (ppcPrivGCPtr)dixLookupPrivate(&pGC->devPrivates, mfbGetGCPrivateKey());
register unsigned long int idx ; /* used for stepping through bitfields */

#define LOWBIT( x ) ( x & - x ) /* Two's complement */
while ((idx = LOWBIT(changes))) {
    switch ( idx ) {

	case GCLineStyle:
	case GCLineWidth:
	    pGC->ops->Polylines = ( ! pGC->lineWidth )
		 ? miZeroLine
		 : ( ( pGC->lineStyle == LineSolid )
			 ? miWideLine : miWideDash ) ;
	    changes &= ~( GCLineStyle | GCLineWidth ) ;
	    break ;

	case GCJoinStyle:
	    changes &= ~ idx ; /* i.e. changes &= ~ GCJoinStyle */
	    break ;

	case GCBackground:
	    if ( pGC->fillStyle != FillOpaqueStippled ) {
		changes &= ~ idx ; /* i.e. changes &= ~GCBackground */
		break ;
	    } /* else Fall Through */
	case GCForeground:
	    if ( pGC->fillStyle == FillTiled ) {
		changes &= ~ idx ; /* i.e. changes &= ~GCForeground */
		break ;
	    } /* else Fall Through */
	case GCFunction:
	case GCPlaneMask:
	case GCFillStyle:
	    { /* new_fill */
		int fillStyle = devPriv->colorRrop.fillStyle ;
		/* install a suitable fillspans */
		if ( fillStyle == FillSolid )
		    pGC->ops->FillSpans = xf4bppSolidPixmapFS ;
		else if ( fillStyle == FillStippled )
		    pGC->ops->FillSpans = xf4bppStipplePixmapFS ;
		else if ( fillStyle == FillOpaqueStippled )
		    pGC->ops->FillSpans = xf4bppOpStipplePixmapFS ;
		else /*  fillStyle == FillTiled */
		    pGC->ops->FillSpans = xf4bppTilePixmapFS ;
		changes &= ~( GCBackground | GCForeground
			    | GCFunction | GCPlaneMask | GCFillStyle ) ;
		break ;
	    } /* end of new_fill */

	default:
	    ErrorF( "ppcChangePixmapGC: Unexpected GC Change\n" ) ;
	    changes &= ~ idx ; /* Remove it anyway */
	    break ;
	}
}

return 0 ;
}

/* Clipping conventions
	if the drawable is a window
	    CT_REGION ==> pCompositeClip really is the composite
	    CT_other ==> pCompositeClip is the window clip region
	if the drawable is a pixmap
	    CT_REGION ==> pCompositeClip is the translated client region
		clipped to the pixmap boundary
	    CT_other ==> pCompositeClip is the pixmap bounding box
*/

static void
xf4bppValidateGC( pGC, changes, pDrawable )
    GCPtr         pGC;
    unsigned long changes;
    DrawablePtr   pDrawable;
{
    register ppcPrivGCPtr devPriv ;
    WindowPtr pWin ;

    devPriv = (ppcPrivGCPtr)dixLookupPrivate(&pGC->devPrivates,
					     mfbGetGCPrivateKey());
    if ( pDrawable->type != devPriv->lastDrawableType ) {
	devPriv->lastDrawableType = pDrawable->type ;
	xf4bppChangeGCtype( pGC, devPriv ) ;
	changes = (unsigned)~0 ;
    }

    if ( pDrawable->depth == 1 ) {
/*	ibmAbort(); */
	xf4bppNeverCalled();
    }

    if ( pDrawable->type == DRAWABLE_WINDOW ) {
	pWin = (WindowPtr) pDrawable ;
	pGC->lastWinOrg.x = pWin->drawable.x ;
	pGC->lastWinOrg.y = pWin->drawable.y ;
    }
    else {
	pWin = (WindowPtr) NULL ;
	pGC->lastWinOrg.x = 0 ;
	pGC->lastWinOrg.y = 0 ;
    }

    changes &= ppcGCInterestValidateMask ;
    /* If Nothing REALLY Changed, Just Return */
    if ( pDrawable->serialNumber == (pGC->serialNumber & DRAWABLE_SERIAL_BITS) )
	if ( !( changes &= ~ GC_CALL_VALIDATE_BIT ) )
	    return ;

    /* GJA -- start of cfb code */
    /*
     * if the client clip is different or moved OR the subwindowMode has
     * changed OR the window's clip has changed since the last validation
     * we need to recompute the composite clip 
     */
  
     if ((changes & (GCClipXOrigin|GCClipYOrigin|GCClipMask|GCSubwindowMode)) ||
 	(pDrawable->serialNumber != (pGC->serialNumber & DRAWABLE_SERIAL_BITS))
 	)
     {
 	if (pWin) {
 	    RegionPtr   pregWin;
 	    Bool        freeTmpClip, freeCompClip;
  
 	    if (pGC->subWindowMode == IncludeInferiors) {
 		pregWin = NotClippedByChildren(pWin);
 		freeTmpClip = TRUE;
  	    }
  	    else {
 		pregWin = &pWin->clipList;
 		freeTmpClip = FALSE;
  	    }
 	    freeCompClip = pGC->freeCompClip;
  
 	    /*
 	     * if there is no client clip, we can get by with just keeping
 	     * the pointer we got, and remembering whether or not should
 	     * destroy (or maybe re-use) it later.  this way, we avoid
 	     * unnecessary copying of regions.  (this wins especially if
 	     * many clients clip by children and have no client clip.) 
 	     */
 	    if (pGC->clientClipType == CT_NONE) {
 		if (freeCompClip)
 		    REGION_DESTROY(pGC->pScreen, pGC->pCompositeClip);
 		pGC->pCompositeClip = pregWin;
 		pGC->freeCompClip = freeTmpClip;
  	    }
  	    else {
 		/*
 		 * we need one 'real' region to put into the composite
 		 * clip. if pregWin the current composite clip are real,
 		 * we can get rid of one. if pregWin is real and the
 		 * current composite clip isn't, use pregWin for the
 		 * composite clip. if the current composite clip is real
 		 * and pregWin isn't, use the current composite clip. if
 		 * neither is real, create a new region. 
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
 	}			/* end of composite clip for a window */
  	else {
 	    BoxRec      pixbounds;
  
 	    /* XXX should we translate by drawable.x/y here ? */
 	    pixbounds.x1 = 0;
 	    pixbounds.y1 = 0;
 	    pixbounds.x2 = pDrawable->width;
 	    pixbounds.y2 = pDrawable->height;
  
 	    if (pGC->freeCompClip) {
 		REGION_RESET(pGC->pScreen, pGC->pCompositeClip, &pixbounds);
  	    } else {
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
	}			/* end of composute clip for pixmap */
     }
    /* GJA -- End of cfb code */

    changes &= ~ ( GCClipXOrigin | GCClipYOrigin | GCClipMask | GCSubwindowMode
		| GC_CALL_VALIDATE_BIT ) ;

    /* If needed, Calculate the Color Reduced Raster-Op */
    if ( changes & ( GCFillStyle | GCBackground | GCForeground
		   | GCPlaneMask | GCFunction ) )
		xf4bppGetReducedColorRrop( pGC, pDrawable->depth,
					&devPriv->colorRrop ) ;

	(* ( ( pDrawable->type == DRAWABLE_WINDOW )
	     ? xf4bppChangeWindowGC
	     : ppcChangePixmapGC ) )( pGC, changes ) ;

    return ;
}
