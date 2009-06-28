/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to 
distribution  of  the software  without specific prior 
written permission. Sun and X Consortium make no 
representations about the suitability of this software for 
any purpose. It is provided "as is" without any express or 
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

/*
 * Copyright IBM Corporation 1987,1988,1989
 *
 * All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that 
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

/* Generic Color Resolution Scheme
 * P. Shupak 12/31/87
 */

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf4bpp.h"
#include "scrnintstr.h"

/*
 * New colormap routines that can support multiple Visual types.
 */

static unsigned short defstaticpalette[16][3] = {
	/*   R       G       B   */
	{ 0x0000, 0x0000, 0x0000 },	/* black */
	{ 0xFFFF, 0xFFFF, 0xFFFF },	/* white */
	{ 0xAAAA, 0xAAAA, 0xAAAA },	/* grey */
	{ 0x0000, 0x0000, 0xAAAA },	/* dark blue */
	{ 0x0000, 0x0000, 0xFFFF },	/* medium blue */
	{ 0x0000, 0xAAAA, 0xFFFF },	/* light blue */
	{ 0x0000, 0xFFFF, 0xFFFF },	/* cyan */
	{ 0x0000, 0xAAAA, 0x0000 },	/* dark green */
	{ 0x0000, 0xFFFF, 0x0000 },	/* green */
	{ 0xAAAA, 0xFFFF, 0x5555 },	/* pale green */
	{ 0xAAAA, 0x5555, 0x0000 },	/* brown */
	{ 0xFFFF, 0xAAAA, 0x0000 },	/* light brown */
	{ 0xFFFF, 0xFFFF, 0x0000 },	/* yellow */
	{ 0xAAAA, 0x0000, 0xAAAA },	/* purple */
	{ 0xFFFF, 0x0000, 0xFFFF },	/* magenta */
	{ 0xFFFF, 0x0000, 0x0000 },	/* red */
	};

Bool
xf4bppInitializeColormap(pmap)
    register ColormapPtr	pmap;
{
    register unsigned i;
    register VisualPtr pVisual;
    unsigned lim, maxent, shift;

    pVisual = pmap->pVisual;
    lim = (1 << pVisual->bitsPerRGBValue) - 1;
    shift = 16 - pVisual->bitsPerRGBValue;
    maxent = pVisual->ColormapEntries - 1;

    switch( pVisual->class )
	{
	case StaticGray:
	    for ( i = 0 ; i < maxent ; i++ ) {
		pmap->red[i].co.local.red   =
		pmap->red[i].co.local.green =
		pmap->red[i].co.local.blue  =
		    ((((i * 65535) / maxent) >> shift) * 65535) / lim;
	    }
	    break;
	case StaticColor:
	    for ( i = 0 ; i < 16 ; i++ ) {
		pmap->red[i].co.local.red   = (defstaticpalette[i][0]);
		pmap->red[i].co.local.green = (defstaticpalette[i][1]);
		pmap->red[i].co.local.blue  = (defstaticpalette[i][2]);
	    }
	    break;
	case GrayScale:
	case PseudoColor:
	    for(i=0;i<=maxent;i++) {
	        int a,b,c;
		a = i << 10;
		b = i << 12;
		c = i << 14;
		pmap->red[i].co.local.red   = a;
		pmap->red[i].co.local.green = b;
		pmap->red[i].co.local.blue  = c;
	    }
	    break;
	case TrueColor:
	case DirectColor:
	default:
	    ErrorF( "Unsupported Visual class %d\b", pVisual->class );
	    return FALSE;
	}
    return TRUE;
}

void
xf4bppResolveColor( pred, pgreen, pblue, pVisual )
register unsigned short* pred ;
register unsigned short* pgreen ;
register unsigned short* pblue ;
register VisualPtr pVisual ;
{ 
    unsigned lim, maxent, shift;

    lim = (1 << pVisual->bitsPerRGBValue) - 1;
    shift = 16 - pVisual->bitsPerRGBValue;
    maxent = pVisual->ColormapEntries - 1;

    switch( pVisual->class )
	{
	case StaticGray:
	    *pred = (30L * *pred + 59L * *pgreen + 11L * *pblue) / 100;
	    *pred = (((*pred * (maxent + 1)) >> 16) * 65535) / maxent;
	    *pblue = *pgreen = *pred = ((*pred >> shift) * 65535) / lim;
	    break;
	case StaticColor:
	    break;
	case GrayScale:
	    *pred = (30L * *pred + 59L * *pgreen + 11L * *pblue) / 100;
	    *pblue = *pgreen = *pred = ((*pred >> shift) * 65535) / lim;
	    break;
	case PseudoColor:
	    /* rescale to rgb bits */
	    *pred = ((*pred >> shift) * 65535) / lim;
	    *pgreen = ((*pgreen >> shift) * 65535) / lim;
	    *pblue = ((*pblue >> shift) * 65535) / lim;
	    break;
	case TrueColor:
	case DirectColor:
	default:
	    ErrorF( "Unsupported Visual class %d\b", pVisual->class );
	}
}

