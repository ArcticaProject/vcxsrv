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

#include <scrnintstr.h>

#include "attributes.h"

/*
 * default medium-source-sizes supported = na-letter w/.25" margins
 */
static XpOidMediumDiscreteSize DefaultMediumSize = {
    xpoid_val_medium_size_na_letter, xFalse, {6.35, 209.55, 6.35, 273.05}
};
static XpOidMediumDiscreteSizeList DefaultMediumSizeList = {
    &DefaultMediumSize, 1
};
static XpOidMediumSourceSize DefaultMediumSourceSize = {
    xpoid_unspecified, XpOidMediumSS_DISCRETE, { &DefaultMediumSizeList }
};
static XpOidMediumSS DefaultMediumSS = {
    &DefaultMediumSourceSize, 1
};

/*
 * if 'valid_oid_list' is NULL any oid found is considered valid
 */
XpOid
XpGetOidAttr(XpContextPtr pContext,
	     XPAttributes pool,
	     XpOid oid,
	     const XpOidList* valid_oid_list)
{
    XpOid value_oid;

    value_oid = XpOidFromString(XpGetStringAttr(pContext, pool, oid));
    if((const XpOidList*)NULL == valid_oid_list
       ||
       XpOidListHasOid(valid_oid_list, value_oid))
    {
	return value_oid;
    }
    else
    {
	return xpoid_none;
    }
}

void
XpPutOidAttr(XpContextPtr pContext,
	     XPAttributes pool,
	     XpOid oid,
	     XpOid value_oid)
{
    XpPutStringAttr(pContext, pool, oid, XpOidString(value_oid));
}

void
XpValidateOidAttr(XpContextPtr pContext,
		  XPAttributes pool,
		  XpOid oid,
		  const XpOidList* valid_oids,
		  XpOid default_oid)
{
    XpOid value_oid;
    value_oid = XpGetOidAttr(pContext, pool, oid, valid_oids);
    XpPutOidAttr(pContext, pool, oid,
		 value_oid == xpoid_none ? default_oid : value_oid);
}

/*
 * if 'valid_card_list' is NULL any cardinal found is considered valid
 */
unsigned long
XpGetCardAttr(XpContextPtr pContext,
	      XPAttributes pool,
	      XpOid oid,
	      const XpOidCardList* valid_card_list)
{
    unsigned long value_card;

    if(XpOidParseUnsignedValue(XpGetStringAttr(pContext, pool, oid),
			       (const char**)NULL,
			       &value_card))
    {
	if((const XpOidCardList*)NULL == valid_card_list
	   ||
	   XpOidCardListHasCard(valid_card_list, value_card))
	{
	    return value_card;
	}
    }
    return 0;
}

void
XpPutCardAttr(XpContextPtr pContext,
	      XPAttributes pool,
	      XpOid oid,
	      unsigned long value_card)
{
    if(value_card > 0)
    {
	char value_out[16];
	sprintf(value_out, "%lu", value_card);
	XpPutStringAttr(pContext, pool, oid, value_out);
    }
    else
	XpPutStringAttr(pContext, pool, oid, (const char*)NULL);
}

void
XpValidateCardAttr(XpContextPtr pContext,
		   XPAttributes pool,
		   XpOid oid,
		   const XpOidCardList* valid_cards,
		   unsigned long default_card)
{
    unsigned long value_card;
    value_card = XpGetCardAttr(pContext, pool, oid, valid_cards);
    XpPutCardAttr(pContext, pool, oid,
		  value_card == 0 ? default_card : value_card);
}

XpOidList*
XpGetListAttr(XpContextPtr pContext,
	      XPAttributes pool,
	      XpOid oid,
	      const XpOidList* valid_oid_list)
{
    return XpOidListNew(XpGetStringAttr(pContext, pool, oid), valid_oid_list);
}

void
XpPutListAttr(XpContextPtr pContext,
	      XPAttributes pool,
	      XpOid oid,
	      const XpOidList* list)
{
    char* value_out;

    value_out = XpOidListString(list);
    XpPutStringAttr(pContext, pool, oid, value_out);
    XpOidFree(value_out);
}

void
XpValidateListAttr(XpContextPtr pContext,
		   XPAttributes pool,
		   XpOid oid,
		   const XpOidList* valid_oids,
		   const XpOidList* default_oids)
{
    XpOidList* list = XpGetListAttr(pContext, pool, oid, valid_oids);
    if(XpOidListCount(list) == 0)
	XpPutListAttr(pContext, pool, oid, default_oids);
    else
	XpPutListAttr(pContext, pool, oid, list);
    XpOidListDelete(list);
}

XpOidCardList*
XpGetCardListAttr(XpContextPtr pContext,
		  XPAttributes pool,
		  XpOid oid,
		  const XpOidCardList* valid_card_list)
{
    return XpOidCardListNew(XpGetStringAttr(pContext, pool, oid),
			    valid_card_list);
}

void
XpPutCardListAttr(XpContextPtr pContext,
		  XPAttributes pool,
		  XpOid oid,
		  const XpOidCardList* list)
{
    char* value_out;

    value_out = XpOidCardListString(list);
    XpPutStringAttr(pContext, pool, oid, value_out);
    XpOidFree(value_out);
}

void
XpValidateCardListAttr(XpContextPtr pContext,
		       XPAttributes pool,
		       XpOid oid,
		       const XpOidCardList* valid_cards,
		       const XpOidCardList* default_cards)
{
    XpOidCardList* list = XpGetCardListAttr(pContext, pool, oid, valid_cards);
    if(XpOidCardListCount(list) == 0 && (XpOidCardList*)NULL != default_cards)
	XpPutCardListAttr(pContext, pool, oid, default_cards);
    else
	XpPutCardListAttr(pContext, pool, oid, list);
    XpOidCardListDelete(list);
}

XpOidDocFmtList*
XpGetDocFmtListAttr(XpContextPtr pContext,
		    XPAttributes pool,
		    XpOid oid,
		    const XpOidDocFmtList* valid_fmt_list)
{
    return XpOidDocFmtListNew(XpGetStringAttr(pContext, pool, oid),
			      valid_fmt_list);
}

void
XpPutDocFmtListAttr(XpContextPtr pContext,
		    XPAttributes pool,
		    XpOid oid,
		    const XpOidDocFmtList* list)
{
    char* value_out;

    value_out = XpOidDocFmtListString(list);
    XpPutStringAttr(pContext, pool, oid, value_out);
    XpOidFree(value_out);
}

void
XpValidateDocFmtListAttr(XpContextPtr pContext,
			 XPAttributes pool,
			 XpOid oid,
			 const XpOidDocFmtList* valid_fmts,
			 const XpOidDocFmtList* default_fmts)
{
    XpOidDocFmtList* list;

    list = XpGetDocFmtListAttr(pContext, pool, oid, valid_fmts);
    if(XpOidDocFmtListCount(list) == 0
       &&
       (XpOidDocFmtList*)NULL != default_fmts)
    {
	XpPutDocFmtListAttr(pContext, pool, oid, default_fmts);
    }
    else
    {
	XpPutDocFmtListAttr(pContext, pool, oid, list);
    }
    XpOidDocFmtListDelete(list);
}

XpOidMediumSS*
XpGetMediumSSAttr(XpContextPtr pContext,
		  XPAttributes pool,
		  XpOid oid,
		  const XpOidList* valid_trays,
		  const XpOidList* valid_sizes)
{
    return XpOidMediumSSNew(XpGetStringAttr(pContext, pool, oid),
			    valid_trays, valid_sizes);
}

void
XpPutMediumSSAttr(XpContextPtr pContext,
		  XPAttributes pool,
		  XpOid oid,
		  const XpOidMediumSS* msss)
{
    char* value_out;

    value_out = XpOidMediumSSString(msss);
    XpPutStringAttr(pContext, pool, oid, value_out);
    XpOidFree(value_out);
}

const XpOidMediumSS*
XpGetDefaultMediumSS()
{
    return &DefaultMediumSS;
}

XpOidTrayMediumList*
XpGetTrayMediumListAttr(XpContextPtr pContext,
			XPAttributes pool,
			XpOid oid,
			const XpOidList* valid_trays,
			const XpOidMediumSS* msss)
{
    return XpOidTrayMediumListNew(XpGetStringAttr(pContext, pool, oid),
				  valid_trays, msss);
}

void
XpPutTrayMediumListAttr(XpContextPtr pContext,
			XPAttributes pool,
			XpOid oid,
			const XpOidTrayMediumList* tm)
{
    char* value_out;

    value_out = XpOidTrayMediumListString(tm);
    XpPutStringAttr(pContext, pool, oid, value_out);
    XpOidFree(value_out);
}

void
XpValidatePrinterMediaAttrs(XpContextPtr pContext,
			    const XpOidList* valid_trays,
			    const XpOidList* valid_sizes)
{
    const XpOidMediumSS* msss;
    XpOidMediumSS* pool_msss;
    XpOidTrayMediumList* tm;

    pool_msss = XpGetMediumSSAttr(pContext, XPPrinterAttr,
				  xpoid_att_medium_source_sizes_supported,
				  valid_trays, valid_sizes);
    if(0 == XpOidMediumSSCount(pool_msss))
	msss = XpGetDefaultMediumSS();
    else
	msss = pool_msss;
    XpPutMediumSSAttr(pContext, XPPrinterAttr,
		      xpoid_att_medium_source_sizes_supported, msss);

    tm = XpGetTrayMediumListAttr(pContext, XPPrinterAttr,
				 xpoid_att_input_trays_medium,
				 valid_trays, msss);
    XpPutTrayMediumListAttr(pContext, XPPrinterAttr,
			    xpoid_att_input_trays_medium, tm);
    
    XpOidMediumSSDelete(pool_msss);
    XpOidTrayMediumListDelete(tm);
}


void
XpValidatePrinterPool(XpContextPtr pContext,
		      const XpValidatePoolsRec* vpr)
{
    /*
     * content-orientations-supported
     */
    XpValidateListAttr(pContext, XPPrinterAttr,
		       xpoid_att_content_orientations_supported,
		       vpr->valid_content_orientations_supported,
		       vpr->default_content_orientations_supported);
    /*
     * document-formats-supported
     */
    XpValidateDocFmtListAttr(pContext, XPPrinterAttr,
			     xpoid_att_document_formats_supported,
			     vpr->valid_document_formats_supported,
			     vpr->default_document_formats_supported);
    /*
     * plexes-supported
     */
    XpValidateListAttr(pContext, XPPrinterAttr, xpoid_att_plexes_supported,
		       vpr->valid_plexes_supported,
		       vpr->default_plexes_supported);
    /*
     * printer-resolutions-supported
     */
    XpValidateCardListAttr(pContext, XPPrinterAttr,
			   xpoid_att_printer_resolutions_supported,
			   vpr->valid_printer_resolutions_supported,
			   vpr->default_printer_resolutions_supported);
    /*
     * xp-embedded-formats-supported
     */
    XpValidateDocFmtListAttr(pContext, XPPrinterAttr,
			     xpoid_att_xp_embedded_formats_supported,
			     vpr->valid_xp_embedded_formats_supported,
			     vpr->default_xp_embedded_formats_supported);
    /*
     * xp-listfonts-modes-supported
     */
    XpValidateListAttr(pContext, XPPrinterAttr,
		       xpoid_att_xp_listfonts_modes_supported,
		       vpr->valid_xp_listfonts_modes_supported,
		       vpr->default_xp_listfonts_modes_supported);
    /*
     * xp-raw-formats-supported
     */
    XpValidateDocFmtListAttr(pContext, XPPrinterAttr,
			     xpoid_att_xp_raw_formats_supported,
			     vpr->valid_xp_raw_formats_supported,
			     vpr->default_xp_raw_formats_supported);
    /*
     * xp-setup-proviso
     */
    XpValidateOidAttr(pContext, XPPrinterAttr, xpoid_att_xp_setup_proviso,
		      vpr->valid_xp_setup_proviso, xpoid_none);
    /*
     * medium-source-sizes-supported
     * input-trays-mdeium
     */
    XpValidatePrinterMediaAttrs(pContext,
				vpr->valid_input_trays,
				vpr->valid_medium_sizes);
    /*
     * available-compressions-supported
     */
    XpValidateListAttr(pContext, XPPrinterAttr,
		       xpoid_att_available_compressions_supported,
		       vpr->valid_available_compressions_supported,
		       vpr->default_available_compressions_supported);
}


void
XpValidateNotificationProfile(XpContextPtr pContext)
{
    const char* value_in;
    const char* value_out;
    
    value_in = XpGetStringAttr(pContext, XPJobAttr,
			       xpoid_att_notification_profile);
    value_out = XpOidNotifyString(XpOidNotifyParse(value_in));
    XpPutStringAttr(pContext, XPJobAttr,
		    xpoid_att_notification_profile, value_out);
}

void
XpValidateJobPool(XpContextPtr pContext,
		  const XpValidatePoolsRec* vpr)
{
    /*
     * Note: the 'vpr' argument is unused in this
     *       implementation; it is reserved for future use
     */
    XpOidList* job_attrs_supported;
    /*
     * only validate attributes found in job-attributes-supported
     */
    job_attrs_supported = XpGetListAttr(pContext, XPPrinterAttr,
					xpoid_att_job_attributes_supported,
					(const XpOidList*)NULL);
    /*
     * notification-profile
     */
    if(XpOidListHasOid(job_attrs_supported, xpoid_att_notification_profile))
    {
	XpValidateNotificationProfile(pContext);
    }
    /*
     * clean up
     */
    XpOidListDelete(job_attrs_supported);
}


static void
XpValidateDocOrPagePool(XpContextPtr pContext,
			XPAttributes pool, /* XPDocAttr or XPPageAttr */
			const XpOidList* attrs_supported,
			const XpValidatePoolsRec* vpr)
{
    /*
     * content-orientation
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_content_orientation))
    {
	XpOidList* content_orientations_supported;
	content_orientations_supported =
	    XpGetListAttr(pContext, XPPrinterAttr,
			  xpoid_att_content_orientations_supported,
			  vpr->valid_content_orientations_supported);
	XpValidateOidAttr(pContext, pool, xpoid_att_content_orientation,
			  content_orientations_supported, xpoid_none);
	XpOidListDelete(content_orientations_supported);
    }
    /*
     * copy-count
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_copy_count))
	XpValidateCardAttr(pContext, pool, xpoid_att_copy_count,
			   (const XpOidCardList*)NULL, 0);
    /*
     * default-printer-resolution
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_default_printer_resolution))
    {
	XpOidCardList* printer_resolutions_supported;
	printer_resolutions_supported =
	    XpGetCardListAttr(pContext, XPPrinterAttr,
			      xpoid_att_printer_resolutions_supported,
			      vpr->valid_printer_resolutions_supported);
	XpValidateCardAttr(pContext, pool,
			   xpoid_att_default_printer_resolution,
			   printer_resolutions_supported, 0);
	XpOidCardListDelete(printer_resolutions_supported);
    }
    /*
     * default-input-tray
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_default_input_tray))
    {
	XpOidTrayMediumList* input_trays_medium;
	const char* value_in;
	XpOid value_tray;
	
	input_trays_medium =
	    XpGetTrayMediumListAttr(pContext, XPPrinterAttr,
				    xpoid_att_input_trays_medium,
				    (const XpOidList*)NULL,
				    (const XpOidMediumSS*)NULL);
	value_in =
	    XpGetStringAttr(pContext, pool, xpoid_att_default_input_tray);
	value_tray = XpOidFromString(value_in);
	if(!XpOidTrayMediumListHasTray(input_trays_medium, value_tray))
	    value_tray = xpoid_none;
	XpPutOidAttr(pContext, pool, xpoid_att_default_input_tray, value_tray);
	XpOidTrayMediumListDelete(input_trays_medium);
    }
    /*
     * default-medium
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_default_medium))
    {
	XpOidMediumSS* msss;
	const char* value_in;
	XpOid value_size;
	
	msss = XpGetMediumSSAttr(pContext, XPPrinterAttr,
				 xpoid_att_medium_source_sizes_supported,
				 (const XpOidList*)NULL,
				 (const XpOidList*)NULL);
	value_in = XpGetStringAttr(pContext, pool, xpoid_att_default_medium);
	value_size = XpOidFromString(value_in);
	if(!XpOidMediumSSHasSize(msss, value_size))
	    value_size = xpoid_none;
	XpPutOidAttr(pContext, pool, xpoid_att_default_medium, value_size);
	XpOidMediumSSDelete(msss);
    }
    /*
     * document-format
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_document_format))
    {
	XpOidDocFmtList* document_formats_supported;
	const char* value_in;
	XpOidDocFmt* document_format;
	const char* value_out;
	
	document_formats_supported =
	    XpGetDocFmtListAttr(pContext, XPPrinterAttr,
				xpoid_att_document_formats_supported,
				vpr->valid_document_formats_supported);
	value_in = XpGetStringAttr(pContext, pool, xpoid_att_document_format);
	document_format = XpOidDocFmtNew(value_in);
	if(XpOidDocFmtListHasFmt(document_formats_supported, document_format))
	    value_out = XpOidDocFmtString(document_format);
	else
	    value_out = XpOidDocFmtString(vpr->default_document_format);
	XpOidDocFmtListDelete(document_formats_supported);
	XpOidDocFmtDelete(document_format);
	XpPutStringAttr(pContext, pool, xpoid_att_document_format, value_out);
	XpOidFree(value_out);
    }
    /*
     * plex
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_plex))
    {
	XpOidList* plexes_supported;
	plexes_supported =
	    XpGetListAttr(pContext, XPPrinterAttr, xpoid_att_plexes_supported,
			  vpr->valid_plexes_supported);
	XpValidateOidAttr(pContext, pool, xpoid_att_plex,
			  plexes_supported, xpoid_none);
	XpOidListDelete(plexes_supported);
    }
    /*
     * xp-listfonts-modes
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_xp_listfonts_modes))
    {
	XpOidList* xp_listfonts_modes_supported;
	xp_listfonts_modes_supported =
	    XpGetListAttr(pContext, XPPrinterAttr,
			  xpoid_att_xp_listfonts_modes_supported,
			  vpr->valid_xp_listfonts_modes_supported);
	XpValidateListAttr(pContext, pool, xpoid_att_xp_listfonts_modes,
			   xp_listfonts_modes_supported,
			   (const XpOidList*)NULL);
	XpOidListDelete(xp_listfonts_modes_supported);
    }
    /*
     * available-compressions
     */
    if(XpOidListHasOid(attrs_supported, xpoid_att_available_compression))
    {
	XpOidList* available_compressions_supported;
	available_compressions_supported =
	    XpGetListAttr(pContext, XPPrinterAttr,
			  xpoid_att_available_compressions_supported,
			  vpr->valid_available_compressions_supported);
	XpValidateOidAttr(pContext, pool, xpoid_att_available_compression,
			  available_compressions_supported, xpoid_none);
	XpOidListDelete(available_compressions_supported);
    }
}

void
XpValidateDocumentPool(XpContextPtr pContext,
		       const XpValidatePoolsRec* vpr)
{
    XpOidList* document_attrs_supported;
    /*
     * only validate attributes found in document-attributes-supported
     */
    document_attrs_supported =
	XpGetListAttr(pContext, XPPrinterAttr,
		      xpoid_att_document_attributes_supported,
		      (const XpOidList*)NULL);
    /*
     * validate
     */
    XpValidateDocOrPagePool(pContext, XPDocAttr,
			    document_attrs_supported, vpr);
    /*
     * clean up
     */
    XpOidListDelete(document_attrs_supported);
}

void
XpValidatePagePool(XpContextPtr pContext,
		   const XpValidatePoolsRec* vpr)
{
    XpOidList* page_attrs_supported;
    /*
     * only validate attributes found in xp-page-attributes-supported
     */
    page_attrs_supported =
	XpGetListAttr(pContext, XPPrinterAttr,
		      xpoid_att_xp_page_attributes_supported,
		      (const XpOidList*)NULL);
    /*
     * validate
     */
    XpValidateDocOrPagePool(pContext, XPPageAttr,
			    page_attrs_supported, vpr);
    /*
     * clean up
     */
    XpOidListDelete(page_attrs_supported);
}

void
XpValidateAttributePool(XpContextPtr pContext,
			XPAttributes pool,
			const XpValidatePoolsRec* vpr)
{
    switch(pool)
    {
    case XPPrinterAttr:
	XpValidatePrinterPool(pContext, vpr);
	break;

    case XPDocAttr:
	XpValidateDocumentPool(pContext, vpr);
	break;

    case XPJobAttr:
	XpValidateJobPool(pContext, vpr);
	break;

    case XPPageAttr:
	XpValidatePagePool(pContext, vpr);
	break;

    default:
	break;
    }
}
