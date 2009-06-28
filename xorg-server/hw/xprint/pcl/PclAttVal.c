/*
 */
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

#include "Pcl.h"
#include "AttrValid.h"

/*
 * define valid values and defaults for Printer pool
 */
static XpOid ValidContentOrientationsOids[] = {
    xpoid_val_content_orientation_portrait,
    xpoid_val_content_orientation_landscape,
    xpoid_val_content_orientation_reverse_portrait,
    xpoid_val_content_orientation_reverse_landscape 
};
static XpOidList ValidContentOrientations = {
    ValidContentOrientationsOids, XpNumber(ValidContentOrientationsOids)
};

static XpOid DefaultContentOrientationsOids[] = {
    xpoid_val_content_orientation_portrait,
    xpoid_val_content_orientation_landscape
};
static XpOidList DefaultContentOrientations = {
    DefaultContentOrientationsOids, XpNumber(DefaultContentOrientationsOids)
};

static XpOid ValidPlexesOids[] = {
    xpoid_val_plex_simplex, xpoid_val_plex_duplex, xpoid_val_plex_tumble
};
static XpOidList ValidPlexes = {
    ValidPlexesOids, XpNumber(ValidPlexesOids)
};

static XpOid DefaultPlexesOids[] = {
    xpoid_val_plex_simplex
};
static XpOidList DefaultPlexes = {
    DefaultPlexesOids, XpNumber(DefaultPlexesOids)
};

static unsigned long ValidPrinterResolutionsCards[] = {
    300
};
static XpOidCardList ValidPrinterResolutions = {
    ValidPrinterResolutionsCards, XpNumber(ValidPrinterResolutionsCards)
};

static unsigned long DefaultPrinterResolutionsCards[] = {
    300
};
static XpOidCardList DefaultPrinterResolutions = {
    DefaultPrinterResolutionsCards, XpNumber(DefaultPrinterResolutionsCards)
};

static XpOid ValidListfontsModesOids[] = {
    xpoid_val_xp_list_internal_printer_fonts, xpoid_val_xp_list_glyph_fonts
};
static XpOidList ValidListfontsModes = {
    ValidListfontsModesOids, XpNumber(ValidListfontsModesOids)
};

static XpOid DefaultListfontsModesOids[] = {
    xpoid_val_xp_list_glyph_fonts
};
static XpOidList DefaultListfontsModes = {
    DefaultListfontsModesOids, XpNumber(DefaultListfontsModesOids)
};

static XpOid ValidSetupProvisoOids[] = {
    xpoid_val_xp_setup_mandatory, xpoid_val_xp_setup_optional
};
static XpOidList ValidSetupProviso = {


    ValidSetupProvisoOids, XpNumber(ValidSetupProvisoOids)
};

static XpOidDocFmt ValidDocFormatsSupportedFmts[] = {
    { "PCL", "5", NULL },
};
static XpOidDocFmtList ValidDocFormatsSupported = {
    ValidDocFormatsSupportedFmts, XpNumber(ValidDocFormatsSupportedFmts)
};

static XpOidDocFmt DefaultDocFormatsSupportedFmts[] = {
    { "PCL", "5", NULL }
};
static XpOidDocFmtList DefaultDocFormatsSupported = {
    DefaultDocFormatsSupportedFmts, XpNumber(DefaultDocFormatsSupportedFmts)
};

static XpOidDocFmt ValidEmbeddedFormatsSupportedFmts[] = {
    { "HPGL", "2", NULL },
};
static XpOidDocFmtList ValidEmbeddedFormatsSupported = {
    ValidEmbeddedFormatsSupportedFmts, XpNumber(ValidEmbeddedFormatsSupportedFmts)
};

static XpOidDocFmt DefaultEmbeddedFormatsSupportedFmts[] = {
    { "HPGL", "2", NULL }
};
static XpOidDocFmtList DefaultEmbeddedFormatsSupported = {
    DefaultEmbeddedFormatsSupportedFmts, XpNumber(DefaultEmbeddedFormatsSupportedFmts)
};

static XpOidDocFmt ValidRawFormatsSupportedFmts[] = {
    { "PCL", "5", NULL },
    { "Postscript", "2", NULL },
    { "ASCII", NULL, NULL }
    
};
static XpOidDocFmtList ValidRawFormatsSupported = {
    ValidRawFormatsSupportedFmts, XpNumber(ValidRawFormatsSupportedFmts)
};

static XpOidDocFmt DefaultRawFormatsSupportedFmts[] = {
    { "PCL", "5", NULL }
};
static XpOidDocFmtList DefaultRawFormatsSupported = {
    DefaultRawFormatsSupportedFmts, XpNumber(DefaultRawFormatsSupportedFmts)
};

static XpOid ValidInputTraysOids[] = {
    xpoid_val_input_tray_manual,
    xpoid_val_input_tray_main,
    xpoid_val_input_tray_envelope,
    xpoid_val_input_tray_large_capacity,
    xpoid_val_input_tray_bottom
};
static XpOidList ValidInputTrays = {
    ValidInputTraysOids, XpNumber(ValidInputTraysOids)
};

static XpOid ValidMediumSizesOids[] = {
    xpoid_val_medium_size_iso_a3,
    xpoid_val_medium_size_iso_a4,
    xpoid_val_medium_size_na_letter,
    xpoid_val_medium_size_na_legal,
    xpoid_val_medium_size_executive,
    xpoid_val_medium_size_ledger,
    xpoid_val_medium_size_iso_c5,
    xpoid_val_medium_size_iso_designated_long,
    xpoid_val_medium_size_na_number_10_envelope,
    xpoid_val_medium_size_monarch_envelope,
    xpoid_val_medium_size_jis_b5,
};
static XpOidList ValidMediumSizes = {
    ValidMediumSizesOids, XpNumber(ValidMediumSizesOids)
};

static XpOidDocFmt DefaultDocumentFormat = {
    "PCL", "5", NULL
};


/*
 * init struct for XpValidate*Pool
 */
XpValidatePoolsRec PclValidatePoolsRec = {
    &ValidContentOrientations, &DefaultContentOrientations,
    &ValidDocFormatsSupported, &DefaultDocFormatsSupported,
    &ValidInputTrays, &ValidMediumSizes,
    &ValidPlexes, &DefaultPlexes,
    &ValidPrinterResolutions, &DefaultPrinterResolutions,
    &ValidEmbeddedFormatsSupported, &DefaultEmbeddedFormatsSupported,
    &ValidListfontsModes, &DefaultListfontsModes,
    &ValidRawFormatsSupported, &DefaultRawFormatsSupported,
    &ValidSetupProviso,
    &DefaultDocumentFormat
};
