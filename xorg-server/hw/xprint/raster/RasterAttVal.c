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
#include <X11/X.h>
#include "misc.h"
#include "dixstruct.h"
#include "scrnintstr.h"
#include "screenint.h"
#include <X11/extensions/Print.h>
#include "Raster.h"

#include "attributes.h"
#include "AttrValid.h"

/*
 * define valid values and defaults for Printer pool
 */
static XpOid ValidContentOrientationsOids[] = {
    xpoid_val_content_orientation_portrait,
    xpoid_val_content_orientation_landscape
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
    xpoid_val_plex_simplex
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
    150, 300, 600
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
    xpoid_val_xp_list_glyph_fonts
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
    { "Postscript", "2", NULL },
    { "PCL", "3", NULL }
};
static XpOidDocFmtList ValidDocFormatsSupported = {
    ValidDocFormatsSupportedFmts, XpNumber(ValidDocFormatsSupportedFmts)
};

static XpOidDocFmt DefaultDocFormatsSupportedFmts[] = {
    { "Postscript", "2", NULL }
};
static XpOidDocFmtList DefaultDocFormatsSupported = {
    DefaultDocFormatsSupportedFmts, XpNumber(DefaultDocFormatsSupportedFmts)
};

static XpOidDocFmtList ValidEmbeddedFormatsSupported = {
    (XpOidDocFmt *)NULL, 0
};

static XpOidDocFmtList DefaultEmbeddedFormatsSupported = {
    (XpOidDocFmt *)NULL, 0
};

static XpOidDocFmt ValidRawFormatsSupportedFmts[] = {
    { "Postscript", "2", NULL },
    { "PCL", "3", NULL }
};
static XpOidDocFmtList ValidRawFormatsSupported = {
    ValidRawFormatsSupportedFmts, XpNumber(ValidRawFormatsSupportedFmts)
};

static XpOidDocFmt DefaultRawFormatsSupportedFmts[] = {
    { "Postscript", "2", NULL }
};
static XpOidDocFmtList DefaultRawFormatsSupported = {
    DefaultRawFormatsSupportedFmts, XpNumber(DefaultRawFormatsSupportedFmts)
};

static XpOidList ValidInputTrays = {
    (XpOid *)NULL, 0
};

static XpOid ValidMediumSizesOids[] = {
    xpoid_val_medium_size_iso_a0,
    xpoid_val_medium_size_iso_a1,
    xpoid_val_medium_size_iso_a2,
    xpoid_val_medium_size_iso_a3,
    xpoid_val_medium_size_iso_a4,
    xpoid_val_medium_size_iso_a5,
    xpoid_val_medium_size_iso_a6,
    xpoid_val_medium_size_iso_a7,
    xpoid_val_medium_size_iso_a8,
    xpoid_val_medium_size_iso_a9,
    xpoid_val_medium_size_iso_a10,
    xpoid_val_medium_size_iso_b0,
    xpoid_val_medium_size_iso_b1,
    xpoid_val_medium_size_iso_b2,
    xpoid_val_medium_size_iso_b3,
    xpoid_val_medium_size_iso_b4,
    xpoid_val_medium_size_iso_b5,
    xpoid_val_medium_size_iso_b6,
    xpoid_val_medium_size_iso_b7,
    xpoid_val_medium_size_iso_b8,
    xpoid_val_medium_size_iso_b9,
    xpoid_val_medium_size_iso_b10,
    xpoid_val_medium_size_na_letter,
    xpoid_val_medium_size_na_legal,
    xpoid_val_medium_size_executive,
    xpoid_val_medium_size_folio,
    xpoid_val_medium_size_invoice,
    xpoid_val_medium_size_ledger,
    xpoid_val_medium_size_quarto,
    xpoid_val_medium_size_iso_c3,
    xpoid_val_medium_size_iso_c4,
    xpoid_val_medium_size_iso_c5,
    xpoid_val_medium_size_iso_c6,
    xpoid_val_medium_size_iso_designated_long,
    xpoid_val_medium_size_na_10x13_envelope,
    xpoid_val_medium_size_na_9x12_envelope,
    xpoid_val_medium_size_na_number_10_envelope,
    xpoid_val_medium_size_na_7x9_envelope,
    xpoid_val_medium_size_na_9x11_envelope,
    xpoid_val_medium_size_na_10x14_envelope,
    xpoid_val_medium_size_na_number_9_envelope,
    xpoid_val_medium_size_monarch_envelope,
    xpoid_val_medium_size_a,
    xpoid_val_medium_size_b,
    xpoid_val_medium_size_c,
    xpoid_val_medium_size_d,
    xpoid_val_medium_size_e,
    xpoid_val_medium_size_jis_b0,
    xpoid_val_medium_size_jis_b1,
    xpoid_val_medium_size_jis_b2,
    xpoid_val_medium_size_jis_b3,
    xpoid_val_medium_size_jis_b4,
    xpoid_val_medium_size_jis_b5,
    xpoid_val_medium_size_jis_b6,
    xpoid_val_medium_size_jis_b7,
    xpoid_val_medium_size_jis_b8,
    xpoid_val_medium_size_jis_b9,
    xpoid_val_medium_size_jis_b10
};
static XpOidList ValidMediumSizes = {
    ValidMediumSizesOids, XpNumber(ValidMediumSizesOids)
};

static XpOidDocFmt DefaultDocumentFormat = {
    "Postscript", "2", NULL
};

static XpOid ValidAvailableCompressionsOids[] = {
    xpoid_val_available_compressions_0,
    xpoid_val_available_compressions_01,
    xpoid_val_available_compressions_02,
    xpoid_val_available_compressions_03,
    xpoid_val_available_compressions_012,
    xpoid_val_available_compressions_013,
    xpoid_val_available_compressions_023,
    xpoid_val_available_compressions_0123
};

static XpOidList ValidAvailableCompressions = {
    ValidAvailableCompressionsOids, XpNumber(ValidAvailableCompressionsOids)
};

static XpOid DefaultAvailableCompressionsOids[] = {
    xpoid_val_available_compressions_0123,
    xpoid_val_available_compressions_0
};

static XpOidList DefaultAvailableCompressions = {
    DefaultAvailableCompressionsOids, XpNumber(DefaultAvailableCompressionsOids)
};


/*
 * init struct for XpValidate*Pool
 */
XpValidatePoolsRec RasterValidatePoolsRec = {
    &ValidContentOrientations, &DefaultContentOrientations,
    &ValidDocFormatsSupported, &DefaultDocFormatsSupported,
    &ValidInputTrays, &ValidMediumSizes,
    &ValidPlexes, &DefaultPlexes,
    &ValidPrinterResolutions, &DefaultPrinterResolutions,
    &ValidEmbeddedFormatsSupported, &DefaultEmbeddedFormatsSupported,
    &ValidListfontsModes, &DefaultListfontsModes,
    &ValidRawFormatsSupported, &DefaultRawFormatsSupported,
    &ValidSetupProviso,
    &DefaultDocumentFormat,
    &ValidAvailableCompressions, &DefaultAvailableCompressions
};
