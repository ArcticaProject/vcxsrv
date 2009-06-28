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
/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:		mediaSizes.c
**    *
**    *  Contents:
**    *                 Routines to return the sizes associated
**    *                 with particular media and particular printers.
**    *
**    *  Created:	2/19/96
**    *
**    *  Copyright:	Copyright 1993,1995 Hewlett-Packard Company
**    *
**    *********************************************************
** 
********************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#include <X11/X.h>
#include "dixstruct.h"
#include "screenint.h"
#include "misc.h"
#include "scrnintstr.h"
#include <X11/fonts/fontstruct.h>

#include "DiPrint.h"
#include "attributes.h"

typedef struct {
    XpOid page_size;
    float width;
    float height;
} PageDimensionsRec;

static PageDimensionsRec PageDimensions[] =
{
    {xpoid_val_medium_size_na_letter,		215.9,		279.4},
    {xpoid_val_medium_size_na_legal,		215.9,		355.6},
    {xpoid_val_medium_size_executive,		184.15,		266.7},
    {xpoid_val_medium_size_folio,		210.82,		330.2},
    {xpoid_val_medium_size_invoice,		139.7,		215.9},
    {xpoid_val_medium_size_ledger,		279.4,		431.8},
    {xpoid_val_medium_size_quarto,		215.9,		275.082},
    {xpoid_val_medium_size_a,			215.9,		279.4},
    {xpoid_val_medium_size_b,			279.4,		431.8},
    {xpoid_val_medium_size_c,			431.8,		558.8},
    {xpoid_val_medium_size_d,			558.8,		863.6},
    {xpoid_val_medium_size_e,			863.6,		1117.6},
    {xpoid_val_medium_size_na_6x9_envelope,	152.4,		228.6},
    {xpoid_val_medium_size_na_10x15_envelope,	254,		381},
    {xpoid_val_medium_size_monarch_envelope,	98.298,		190.5},
    {xpoid_val_medium_size_na_10x13_envelope,	254,		330.2},
    {xpoid_val_medium_size_na_9x12_envelope,	228.6,		304.8},
    {xpoid_val_medium_size_na_number_10_envelope, 104.775,	241.3},
    {xpoid_val_medium_size_na_7x9_envelope,	177.8,		228.6},
    {xpoid_val_medium_size_na_9x11_envelope,	228.6,		279.4},
    {xpoid_val_medium_size_na_10x14_envelope,	254,		355.6},
    {xpoid_val_medium_size_na_number_9_envelope, 98.425,	225.425},
    {xpoid_val_medium_size_iso_a0,		841,		1189},
    {xpoid_val_medium_size_iso_a1,		594,		841},
    {xpoid_val_medium_size_iso_a2,		420,		594},
    {xpoid_val_medium_size_iso_a3,		297,		420},
    {xpoid_val_medium_size_iso_a4,		210,		297},
    {xpoid_val_medium_size_iso_a5,		148,		210},
    {xpoid_val_medium_size_iso_a6,		105,		148},
    {xpoid_val_medium_size_iso_a7,		74,		105},
    {xpoid_val_medium_size_iso_a8,		52,		74},
    {xpoid_val_medium_size_iso_a9,		37,		52},
    {xpoid_val_medium_size_iso_a10,		26,		37},
    {xpoid_val_medium_size_iso_b0,		1000,		1414},
    {xpoid_val_medium_size_iso_b1,		707,		1000},
    {xpoid_val_medium_size_iso_b2,		500,		707},
    {xpoid_val_medium_size_iso_b3,		353,		500},
    {xpoid_val_medium_size_iso_b4,		250,		353},
    {xpoid_val_medium_size_iso_b5,		176,		250},
    {xpoid_val_medium_size_iso_b6,		125,		176},
    {xpoid_val_medium_size_iso_b7,		88,		125},
    {xpoid_val_medium_size_iso_b8,		62,		88},
    {xpoid_val_medium_size_iso_b9,		44,		62},
    {xpoid_val_medium_size_iso_b10,		31,		44},
    {xpoid_val_medium_size_jis_b0,		1030,		1456},
    {xpoid_val_medium_size_jis_b1,		728,		1030},
    {xpoid_val_medium_size_jis_b2,		515,		728},
    {xpoid_val_medium_size_jis_b3,		364,		515},
    {xpoid_val_medium_size_jis_b4,		257,		364},
    {xpoid_val_medium_size_jis_b5,		182,		257},
    {xpoid_val_medium_size_jis_b6,		128,		182},
    {xpoid_val_medium_size_jis_b7,		91,		128},
    {xpoid_val_medium_size_jis_b8,		64,		91},
    {xpoid_val_medium_size_jis_b9,		45,		64},
    {xpoid_val_medium_size_jis_b10,		32,		45},
    {xpoid_val_medium_size_hp_2x_postcard,	148,		200},
    {xpoid_val_medium_size_hp_european_edp,	304.8,		355.6},
    {xpoid_val_medium_size_hp_mini,		139.7,		215.9},
    {xpoid_val_medium_size_hp_postcard,		100,		148},
    {xpoid_val_medium_size_hp_tabloid,		279.4,		431.8},
    {xpoid_val_medium_size_hp_us_edp,		279.4,		355.6},
    {xpoid_val_medium_size_hp_us_government_legal,	203.2,	330.2},
    {xpoid_val_medium_size_hp_us_government_letter,	203.2,	254},
    {xpoid_val_medium_size_iso_c3,		324,		458},
    {xpoid_val_medium_size_iso_c4,		229,		324},
    {xpoid_val_medium_size_iso_c5,		162,		229},
    {xpoid_val_medium_size_iso_c6,		114,		162},
    {xpoid_val_medium_size_iso_designated_long,	110,		220}
};

/*
 * XpGetResolution returns an integer representing the printer resolution
 * in dots-per-inch for the specified print context.
 *
 * Note: This routine assumes the values found in the passed context's
 *       attributes pools have been validated.
 */
int
XpGetResolution(
		XpContextPtr pContext)
{
    unsigned long resolution;

    resolution = XpGetCardAttr(pContext, XPPageAttr,
			       xpoid_att_default_printer_resolution,
			       (XpOidCardList*)NULL);
    if(0 == resolution)
	resolution = XpGetCardAttr(pContext, XPDocAttr,
				   xpoid_att_default_printer_resolution,
				   (XpOidCardList*)NULL);
    if(0 == resolution)
    {
	XpOidCardList* resolutions_supported;
	/*
	 * default-printer-resolution not specified; default to 1st entry
	 * in printer-resolutions-supported.
	 */
	resolutions_supported =
	    XpGetCardListAttr(pContext, XPPrinterAttr,
			      xpoid_att_printer_resolutions_supported,
			      (XpOidCardList*)NULL);
	resolution = XpOidCardListGetCard(resolutions_supported, 0);
	XpOidCardListDelete(resolutions_supported);
    }
    return (int)resolution;
}

/*
 * XpGetContentOrientation determines the content-orientation as
 * determined by the passed context. The page and document pools are
 * queried in turn for a specified content-orientation attribute. If none
 * is found the first content-orientation in the
 * content-orientations-supported printer attribute is taken as the
 * default. 
 *
 * Note: This routine assumes the values found in the passed context's
 *       attributes pools have been validated.
 */
XpOid
XpGetContentOrientation(
			XpContextPtr pContext)
{
    XpOid orientation;

    orientation = XpGetOidAttr(pContext, XPPageAttr,
			       xpoid_att_content_orientation,
			       (XpOidList*)NULL);
    if(xpoid_none == orientation)
	orientation = XpGetOidAttr(pContext, XPDocAttr,
				   xpoid_att_content_orientation,
				   (XpOidList*)NULL);
    if(xpoid_none == orientation)
    {
	XpOidList* content_orientations_supported;

	content_orientations_supported =
	    XpGetListAttr(pContext, XPPrinterAttr,
			  xpoid_att_content_orientations_supported,
			  (XpOidList*)NULL);
	orientation = XpOidListGetOid(content_orientations_supported, 0);
	XpOidListDelete(content_orientations_supported);
    }
    return orientation;
}

/*
 * XpGetAvailableCompression determines the available-compression as
 * determined by the passed context. The page and document pools are
 * queried in turn for a specified content-orientation attribute. If none
 * is found the first available-compression in the
 * avaiable-compressions-supported printer attribute is taken as the
 * default.
 *
 * Note: This routine assumes the values found in the passed context's
 *       attributes pools have been validated.
 */
XpOid
XpGetAvailableCompression(
			XpContextPtr pContext)
{
    XpOid compression;

    compression = XpGetOidAttr(pContext, XPPageAttr,
			       xpoid_att_available_compression,
			       (XpOidList*)NULL);
    if(xpoid_none == compression)
	compression = XpGetOidAttr(pContext, XPDocAttr,
				   xpoid_att_available_compression,
				   (XpOidList*)NULL);
    if(xpoid_none == compression)
    {
	XpOidList* available_compressions_supported;

	available_compressions_supported =
	    XpGetListAttr(pContext, XPPrinterAttr,
			  xpoid_att_available_compressions_supported,
			  (XpOidList*)NULL);
	compression = XpOidListGetOid(available_compressions_supported, 0);
	XpOidListDelete(available_compressions_supported);
    }
    return compression;
}

/*
 * XpGetPlex determines the plex as determined by the passed context. The page
 * and document pools are queried in turn for a specified plex attribute. If
 * none is found the first plex in the plexes-supported printer attribute is
 * taken as the default.
 *
 * Note: This routine assumes the values found in the passed context's
 *       attributes pools have been validated.
 */
XpOid
XpGetPlex(
	  XpContextPtr pContext)
{
    XpOid plex;

    plex = XpGetOidAttr(pContext, XPPageAttr, xpoid_att_plex,
			(XpOidList*)NULL);
    if(xpoid_none == plex)
	plex = XpGetOidAttr(pContext, XPDocAttr, xpoid_att_plex,
			    (XpOidList*)NULL);
    if(xpoid_none == plex)
    {
	XpOidList* plexes_supported;

	plexes_supported =
	    XpGetListAttr(pContext, XPPrinterAttr,
			  xpoid_att_plexes_supported,
			  (XpOidList*)NULL);
	plex = XpOidListGetOid(plexes_supported, 0);
	XpOidListDelete(plexes_supported);
    }
    return plex;
}

/*
 * XpGetPageSize returns the XpOid of the current page size (medium names
 * are page sizes in this implementation) as indicated by the passed
 * context.
 *
 * The relevant input-tray is returned in pTray. This parm must not be
 * NULL. If the input-tray is not indicated or irrelevant, xpoid_none
 * will be returned.
 *
 * This function optionally takes a XpOidMediumSS representation of the
 * medium-source-sizes-supported attribute in order to avoid parsing the
 * string value twice for calling functions that need to parse m-s-s-s
 * anyway (e.g. XpGetReproductionArea). If the caller has no other reason
 * to parse medium-source-sizes-supported, it is recommended that NULL be
 * passed.  This function will obtain medium-source-sizes-supported if it
 * needs to.
 *
 * Note: This routine assumes the values found in the passed context's
 *       attributes pools have been validated.
 */
XpOid
XpGetPageSize(XpContextPtr pContext,
	    XpOid* pTray,
	    const XpOidMediumSS* msss)
{
    XpOid medium;
    /*
     * check to see if default-medium is specified
     */
    medium = XpGetOidAttr(pContext, XPPageAttr, xpoid_att_default_medium,
			  (const XpOidList*)NULL);
    if(medium == xpoid_none)
    {
	/*
	 * default-medium not in page pool; try the document pool
	 */
	medium = XpGetOidAttr(pContext, XPDocAttr, xpoid_att_default_medium,
			      (const XpOidList*)NULL);
    }
    if(medium == xpoid_none)
    {
	/*
	 * default-medium not specified; try default-input-tray
	 */
	*pTray = XpGetOidAttr(pContext, XPPageAttr,
			      xpoid_att_default_input_tray,
			      (const XpOidList*)NULL);
	if(*pTray == xpoid_none)
	{
	    /*
	     * default-input-tray not in page pool; try the document pool
	     */
	    *pTray = XpGetOidAttr(pContext, XPDocAttr,
				  xpoid_att_default_input_tray,
				  (const XpOidList*)NULL);
	}
	if(*pTray != xpoid_none)
	{
	    /*
	     * default-input-tray found; get corresponding medium from
	     * input-trays-medium
	     */
	    XpOidTrayMediumList* input_trays_medium;
	    int i;
	    
	    input_trays_medium =
		XpGetTrayMediumListAttr(pContext, XPPrinterAttr,
					xpoid_att_input_trays_medium,
					(const XpOidList*)NULL,
					(const XpOidMediumSS*)NULL);
	    for(i = 0; i < XpOidTrayMediumListCount(input_trays_medium); i++)
	    {
		if(*pTray == XpOidTrayMediumListTray(input_trays_medium, i))
		{
		    medium = XpOidTrayMediumListMedium(input_trays_medium, i);
		    break;
		}
	    }
	    XpOidTrayMediumListDelete(input_trays_medium);
	}
    }
    else
	*pTray = xpoid_none;
    
    if(medium == xpoid_none)
    {
	XpOidMediumSS* local_msss = (XpOidMediumSS*)NULL;
	int i_mss, i_ds;
	XpOidMediumDiscreteSizeList* ds_list;
	/*
	 * no medium specified; use 1st page size found in
	 * medium-source-sizes-supported
	 */
	if((XpOidMediumSS*)NULL == msss)
	    msss = local_msss =
		XpGetMediumSSAttr(pContext, XPPrinterAttr,
				  xpoid_att_medium_source_sizes_supported,
				  (const XpOidList*)NULL,
				  (const XpOidList*)NULL);
	for(i_mss = 0;
	    i_mss < XpOidMediumSSCount(msss) && xpoid_none == medium;
	    i_mss++)
	{
	    if(XpOidMediumSS_DISCRETE == (msss->mss)[i_mss].mstag
	       &&
	       xpoid_none != (msss->mss)[i_mss].input_tray)
	    {
		ds_list =  (msss->mss)[i_mss].ms.discrete;
		for(i_ds = 0; i_ds < ds_list->count; i_ds++)
		{
		    if(xpoid_none != (ds_list->list)[i_ds].page_size)
		    {
			medium = (ds_list->list)[i_ds].page_size;
			break;
		    }
		}
	    }
	}
	XpOidMediumSSDelete(local_msss);
    }
    return medium;
}

/*
 * XpGetMediumMillimeters returns into the supplied float pointers the
 * width and height in millimeters of the passed page size identifier.
 */
void
XpGetMediumMillimeters(
		       XpOid page_size,
		       float *width,  /* return */
		       float *height) /* return */
{
    int i;

    *width = *height = 0;
    for(i = 0; i < XpNumber(PageDimensions); i++)
    {
	if(page_size == PageDimensions[i].page_size)
	{
	    *width = PageDimensions[i].width;
	    *height = PageDimensions[i].height;
	    return;
	}
    }
}

/*
 * Converts a millimeter specification into pixels given a resolution in
 * DPI.
 */
static float
MmToPixels(float mm, int resolution)
{
    float f;

    f = mm * resolution;
    f /= 25.4;
    return f;
}

/*
 * XpGetMediumDimensions returns into the supplied short pointers the
 * width and height in pixels of the medium associated with the specified
 * print context. It obtains the page size associated with the current
 * medium by calling XpGetPageSize. It passes XpGetMediumMillimeters the
 * page size, and converts the returned millimeter dimensions into pixels
 * using the resolution returned by XpGetResolution.
 *
 * Note: This routine assumes the values found in the passed context's
 *       attributes pools have been validated.
 */
void
XpGetMediumDimensions(
		      XpContextPtr pContext,
		      unsigned short *width,  /* return */
		      unsigned short *height) /* return */
{
    XpOid page_size;
    XpOid tray;
    XpOid orientation;
    
    int resolution;
    float w_mm, h_mm;
    
    page_size = XpGetPageSize(pContext, &tray, (XpOidMediumSS*)NULL);
    if(page_size == xpoid_none)
    {
	/*
	 * fail-safe: if the pools have been validated, this defaulting logic
	 *            isn't needed.
	 */
	page_size = xpoid_val_medium_size_na_letter;
    }
    XpGetMediumMillimeters(page_size, &w_mm, &h_mm);
    resolution = XpGetResolution(pContext);
    orientation = XpGetContentOrientation(pContext);
    switch(orientation)
    {
    case xpoid_val_content_orientation_landscape:
    case xpoid_val_content_orientation_reverse_landscape:
	/*
	 * transpose width and height
	 */
	*height = MmToPixels(w_mm, resolution);
	*width = MmToPixels(h_mm, resolution);
	break;

    default:
	*width = MmToPixels(w_mm, resolution);
	*height = MmToPixels(h_mm, resolution);
	break;
    }
}

/*
 * XRectangleFromXpOidArea converts an XpOidArea area specification
 * into an XRectangle. The passed resolution is used to convert from
 * millimeters (XpOidArea) into pixels (XRectangle).
 */
static void
XRectangleFromXpOidArea(
			xRectangle *pRect,
			const XpOidArea* repro,
			int resolution,
			XpOid orientation)
{
    switch(orientation)
    {
    case xpoid_val_content_orientation_landscape:
    case xpoid_val_content_orientation_reverse_landscape:
	/*
	 * transpose x and y, width and height
	 */
	pRect->y = MmToPixels(repro->minimum_x, resolution);
	pRect->x = MmToPixels(repro->minimum_y, resolution);
	pRect->height =
	    MmToPixels(repro->maximum_x - repro->minimum_x, resolution);
	pRect->width =
	    MmToPixels(repro->maximum_y - repro->minimum_y, resolution);
	break;

    default:
	pRect->x = MmToPixels(repro->minimum_x, resolution);
	pRect->y = MmToPixels(repro->minimum_y, resolution);
	pRect->width =
	    MmToPixels(repro->maximum_x - repro->minimum_x, resolution);
	pRect->height =
	    MmToPixels(repro->maximum_y - repro->minimum_y, resolution);
	break;
    }
}

/*
 * XpGetReproductionArea queries the current pool attribute values in
 * order to determine the reproduction area for the currently selected
 * medium.
 *
 * First the current page size (equivalent to current medium) and tray
 * (if specified) is retrieved via XpGetPageSize. The value of the
 * medium-source-sizes-supported attribute is interrogated until a matching
 * entry for the current page size and tray is found. The reproduction
 * area defined for the current entry is converted into an XRectangle
 * using XRectangleFromXpOidArea and returned to the caller.
 *
 * Note: This routine assumes the values found in the passed context's
 *       attributes pools have been validated.
 */
void
XpGetReproductionArea(XpContextPtr pContext,
		      xRectangle *pRect)
{
    XpOid page_size;
    XpOid tray;
    XpOidMediumSS* msss;
    int i_mss, i_ds;
    XpOidMediumDiscreteSizeList* ds_list;
    XpOidArea* repro;
    BOOL done;
    int resolution;
    XpOid orientation;
    /*
     * find the appropriate assured reproduction area for the current
     * tray and page size in the medium-source-sizes-supported attribute.
     */
    msss = XpGetMediumSSAttr(pContext, XPPrinterAttr,
			     xpoid_att_medium_source_sizes_supported,
			     (const XpOidList*)NULL,
			     (const XpOidList*)NULL);
    page_size = XpGetPageSize(pContext, &tray, msss);
    resolution = XpGetResolution(pContext);
    orientation = XpGetContentOrientation(pContext);
    
    memset(pRect, 0, sizeof(xRectangle));

    if(xpoid_none == tray)
    {
	/*
	 * no tray specified; use 1st matching page size
	 */
	for(i_mss = 0, done = xFalse;
	    i_mss < XpOidMediumSSCount(msss) && !done;
	    i_mss++)
	{
	    if(XpOidMediumSS_DISCRETE == (msss->mss)[i_mss].mstag
	       &&
	       xpoid_none != (msss->mss)[i_mss].input_tray)
	    {
		ds_list =  (msss->mss)[i_mss].ms.discrete;
		for(i_ds = 0; i_ds < ds_list->count; i_ds++)
		{
		    if(page_size == (ds_list->list)[i_ds].page_size)
		    {
			repro =
			    &(ds_list->list)[i_ds].assured_reproduction_area;
			XRectangleFromXpOidArea(pRect, repro,
						resolution, orientation);
			done = xTrue;
			break;
		    }
		}
	    }
	}
    }
    else
    {
	/*
	 * tray && page size specified; find matching entry
	 */
	for(i_mss = 0, done = xFalse;
	    i_mss < XpOidMediumSSCount(msss) && !done;
	    i_mss++)
	{
	    if(XpOidMediumSS_DISCRETE == (msss->mss)[i_mss].mstag
	       &&
	       xpoid_none != (msss->mss)[i_mss].input_tray
	       &&
	       (tray == (msss->mss)[i_mss].input_tray
		||
		xpoid_unspecified == (msss->mss)[i_mss].input_tray)
	       )
	    {
		ds_list =  (msss->mss)[i_mss].ms.discrete;
		for(i_ds = 0; i_ds < ds_list->count; i_ds++)
		{
		    if(page_size == (ds_list->list)[i_ds].page_size)
		    {
			repro =
			    &(ds_list->list)[i_ds].assured_reproduction_area;
			XRectangleFromXpOidArea(pRect, repro,
						resolution, orientation);
			if(xpoid_unspecified != (msss->mss)[i_mss].input_tray)
			{
			    /*
			     * exact match on tray takes precendence over
			     * unspecified tray entry in m-s-s-s
			     */
			    done = xTrue;
			}
			break;
		    }
		}
	    }
	}
    }
    XpOidMediumSSDelete(msss);
}

/*
 * XpGetMaxWidthHeightRes returns into the supplied width and height
 * unsigned short pointers the dimensions in millimeters of the largest
 * supported media for a specific printer.  It looks at the
 * medium-source-sizes-supported attribute (if it exists) to determine
 * the list of possible media, and calls XpGetMediumMillimeters to get the
 * dimensions for each medium.  If the m-s-s-s attribute is not defined,
 * then the dimensions for the na-letter medium is returned.
 *
 * This function also returns the largest resolution in DPI defined in
 * printer-resolutions-supported. If printer-resolutions-supported is not
 * specified, the default is obtained from the passed XpValidatePoolsRec.
 *
 * The passed XpValidatePoolsRec is also used to determine valid values
 * when parsing attribute values.
 */
void
XpGetMaxWidthHeightRes(
		       const char *printer_name,
		       const XpValidatePoolsRec* vpr,
		       float *width,
		       float *height,
		       int* resolution)
{
    const char* value;
    const char* attr_str;
    XpOidMediumSS* pool_msss;
    const XpOidMediumSS* msss;
    int i_mss, i_ds;
    XpOidMediumDiscreteSizeList* ds_list;
    float w, h;
    XpOidCardList* pool_resolutions_supported;
    const XpOidCardList* resolutions_supported;
    int i;
    int res;
    /*
     * get the max medium width and height
     */
    attr_str = XpOidString(xpoid_att_medium_source_sizes_supported);
    value = XpGetPrinterAttribute(printer_name, attr_str);
    pool_msss = XpOidMediumSSNew(value,
				 vpr->valid_input_trays,
				 vpr->valid_medium_sizes);
    if(0 == XpOidMediumSSCount(pool_msss))
	msss = XpGetDefaultMediumSS();
    else
	msss = pool_msss;
    *width = *height = 0;
    for(i_mss = 0; i_mss < XpOidMediumSSCount(msss); i_mss++)
    {
	if(XpOidMediumSS_DISCRETE == (msss->mss)[i_mss].mstag
	   &&
	   xpoid_none != (msss->mss)[i_mss].input_tray)
	{
	    ds_list = (msss->mss)[i_mss].ms.discrete;
	    for(i_ds = 0; i_ds < ds_list->count; i_ds++)
	    {
		if(xpoid_none != (ds_list->list)[i_ds].page_size)
		{
		    XpGetMediumMillimeters((ds_list->list)[i_ds].page_size,
					   &w, &h);
		    if(w > *width) *width = w;
		    if(h > *height) *height = h;
		}
	    }
	}
    }
    XpOidMediumSSDelete(pool_msss);
    /*
     * get the maximum resolution
     */
    attr_str = XpOidString(xpoid_att_printer_resolutions_supported);
    value = XpGetPrinterAttribute(printer_name, attr_str);
    pool_resolutions_supported =
	XpOidCardListNew(value, vpr->valid_printer_resolutions_supported);
    if(0 == XpOidCardListCount(pool_resolutions_supported))
	resolutions_supported = vpr->default_printer_resolutions_supported;
    else
	resolutions_supported = pool_resolutions_supported;
    *resolution = 0;
    for(i = 0; i < XpOidCardListCount(resolutions_supported); i++)
    {
	res = XpOidCardListGetCard(resolutions_supported, i);
	if(res > *resolution) *resolution = res;
    }
    XpOidCardListDelete(pool_resolutions_supported);
}

FontResolutionPtr
XpGetClientResolutions(client, num)
    ClientPtr client;
    int *num;
{
    static struct _FontResolution res;
    int resolution = XpGetResolution(XpContextOfClient(client)); 
    
    res.x_resolution = resolution;
    res.y_resolution = resolution;

    res.point_size = 120;

    *num = 1;

    return &res;
}


void XpSetFontResFunc(client)
    ClientPtr client;
{
    client->fontResFunc = XpGetClientResolutions;
}


void XpUnsetFontResFunc(client)
    ClientPtr client;
{
    client->fontResFunc = NULL;
}
