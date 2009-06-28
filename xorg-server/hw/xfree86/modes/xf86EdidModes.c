/*
 * Copyright 2006 Luc Verhaegen.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file This file covers code to convert a xf86MonPtr containing EDID-probed
 * information into a list of modes, including applying monitor-specific
 * quirks to fix broken EDID data.
 */
#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#endif

#include "xf86.h"
#include "xf86DDC.h"
#include <X11/Xatom.h>
#include "property.h"
#include "propertyst.h"
#include "xf86DDC.h"
#include "xf86Crtc.h"
#include <string.h>
#include <math.h>

/*
 * Quirks to work around broken EDID data from various monitors.
 */

typedef enum {
    DDC_QUIRK_NONE = 0,
    /* First detailed mode is bogus, prefer largest mode at 60hz */
    DDC_QUIRK_PREFER_LARGE_60 = 1 << 0,
    /* 135MHz clock is too high, drop a bit */
    DDC_QUIRK_135_CLOCK_TOO_HIGH = 1 << 1,
    /* Prefer the largest mode at 75 Hz */
    DDC_QUIRK_PREFER_LARGE_75 = 1 << 2,
    /* Convert detailed timing's horizontal from units of cm to mm */
    DDC_QUIRK_DETAILED_H_IN_CM = 1 << 3,
    /* Convert detailed timing's vertical from units of cm to mm */
    DDC_QUIRK_DETAILED_V_IN_CM = 1 << 4,
    /* Detailed timing descriptors have bogus size values, so just take the
     * maximum size and use that.
     */
    DDC_QUIRK_DETAILED_USE_MAXIMUM_SIZE = 1 << 5,
    /* Monitor forgot to set the first detailed is preferred bit. */
    DDC_QUIRK_FIRST_DETAILED_PREFERRED = 1 << 6,
    /* use +hsync +vsync for detailed mode */
    DDC_QUIRK_DETAILED_SYNC_PP = 1 << 7,
} ddc_quirk_t;

static Bool quirk_prefer_large_60 (int scrnIndex, xf86MonPtr DDC)
{
    /* Belinea 10 15 55 */
    if (memcmp (DDC->vendor.name, "MAX", 4) == 0 &&
	((DDC->vendor.prod_id == 1516) ||
	(DDC->vendor.prod_id == 0x77e)))
	return TRUE;
    
    /* Acer AL1706 */
    if (memcmp (DDC->vendor.name, "ACR", 4) == 0 &&
	DDC->vendor.prod_id == 44358)
	return TRUE;

    /* Bug #10814: Samsung SyncMaster 225BW */
    if (memcmp (DDC->vendor.name, "SAM", 4) == 0 &&
	DDC->vendor.prod_id == 596)
	return TRUE;

    /* Bug #10545: Samsung SyncMaster 226BW */
    if (memcmp (DDC->vendor.name, "SAM", 4) == 0 &&
	DDC->vendor.prod_id == 638)
	return TRUE;

    /* Acer F51 */
    if (memcmp (DDC->vendor.name, "API", 4) == 0 &&
	DDC->vendor.prod_id == 0x7602)
	return TRUE;


    return FALSE;
}

static Bool quirk_prefer_large_75 (int scrnIndex, xf86MonPtr DDC)
{
    /* Bug #11603: Funai Electronics PM36B */
    if (memcmp (DDC->vendor.name, "FCM", 4) == 0 &&
	DDC->vendor.prod_id == 13600)
	return TRUE;

    return FALSE;
}

static Bool quirk_detailed_h_in_cm (int scrnIndex, xf86MonPtr DDC)
{
    /* Bug #11603: Funai Electronics PM36B */
    if (memcmp (DDC->vendor.name, "FCM", 4) == 0 &&
	DDC->vendor.prod_id == 13600)
	return TRUE;

    return FALSE;
}

static Bool quirk_detailed_v_in_cm (int scrnIndex, xf86MonPtr DDC)
{
    /* Bug #11603: Funai Electronics PM36B */
    if (memcmp (DDC->vendor.name, "FCM", 4) == 0 &&
	DDC->vendor.prod_id == 13600)
	return TRUE;

    return FALSE;
}

static Bool quirk_detailed_use_maximum_size (int scrnIndex, xf86MonPtr DDC)
{
    /* Bug #10304: LGPhilipsLCD LP154W01-A5 */
    if (memcmp (DDC->vendor.name, "LPL", 4) == 0 &&
	(DDC->vendor.prod_id == 0 || DDC->vendor.prod_id == 0x2a00))
	return TRUE;

    return FALSE;
}

static Bool quirk_135_clock_too_high (int scrnIndex, xf86MonPtr DDC)
{
    /* Envision Peripherals, Inc. EN-7100e.  See bug #9550. */
    if (memcmp (DDC->vendor.name, "EPI", 4) == 0 &&
	DDC->vendor.prod_id == 59264)
	return TRUE;
    
    return FALSE;
}

static Bool quirk_first_detailed_preferred (int scrnIndex, xf86MonPtr DDC)
{
    /* Philips 107p5 CRT. Reported on xorg@ with pastebin. */
    if (memcmp (DDC->vendor.name, "PHL", 4) == 0 &&
	DDC->vendor.prod_id == 57364)
	return TRUE;

    /* Proview AY765C 17" LCD. See bug #15160*/
    if (memcmp (DDC->vendor.name, "PTS", 4) == 0 &&
	DDC->vendor.prod_id == 765)
	return TRUE;

    /* ACR of some sort RH #284231 */
    if (memcmp (DDC->vendor.name, "ACR", 4) == 0 &&
	DDC->vendor.prod_id == 2423)
	return TRUE;

    return FALSE;
}

static Bool quirk_detailed_sync_pp(int scrnIndex, xf86MonPtr DDC)
{
    /* Bug #12439: Samsung SyncMaster 205BW */
    if (memcmp (DDC->vendor.name, "SAM", 4) == 0 &&
	DDC->vendor.prod_id == 541)
	return TRUE;
    return FALSE;
}

typedef struct {
    Bool	(*detect) (int scrnIndex, xf86MonPtr DDC);
    ddc_quirk_t	quirk;
    char	*description;
} ddc_quirk_map_t;

static const ddc_quirk_map_t ddc_quirks[] = {
    {
	quirk_prefer_large_60,   DDC_QUIRK_PREFER_LARGE_60,
	"Detailed timing is not preferred, use largest mode at 60Hz"
    },
    {
	quirk_135_clock_too_high,   DDC_QUIRK_135_CLOCK_TOO_HIGH,
	"Recommended 135MHz pixel clock is too high"
    },
    {
	quirk_prefer_large_75,   DDC_QUIRK_PREFER_LARGE_75,
	"Detailed timing is not preferred, use largest mode at 75Hz"
    },
    {
	quirk_detailed_h_in_cm,   DDC_QUIRK_DETAILED_H_IN_CM,
	"Detailed timings give horizontal size in cm."
    },
    {
	quirk_detailed_v_in_cm,   DDC_QUIRK_DETAILED_V_IN_CM,
	"Detailed timings give vertical size in cm."
    },
    {
	quirk_detailed_use_maximum_size,   DDC_QUIRK_DETAILED_USE_MAXIMUM_SIZE,
	"Detailed timings give sizes in cm."
    },
    {
	quirk_first_detailed_preferred, DDC_QUIRK_FIRST_DETAILED_PREFERRED,
	"First detailed timing was not marked as preferred."
    },
    {
	quirk_detailed_sync_pp, DDC_QUIRK_DETAILED_SYNC_PP,
	"Use +hsync +vsync for detailed timing."
    },
    { 
	NULL,		DDC_QUIRK_NONE,
	"No known quirks"
    },
};

/*
 * TODO:
 *  - for those with access to the VESA DMT standard; review please.
 */
#define MODEPREFIX NULL, NULL, NULL, 0, M_T_DRIVER
#define MODESUFFIX 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,FALSE,FALSE,0,NULL,0,0.0,0.0

static const DisplayModeRec DDCEstablishedModes[17] = {
    { MODEPREFIX,    40000,  800,  840,  968, 1056, 0,  600,  601,  605,  628, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 800x600@60Hz */
    { MODEPREFIX,    36000,  800,  824,  896, 1024, 0,  600,  601,  603,  625, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 800x600@56Hz */
    { MODEPREFIX,    31500,  640,  656,  720,  840, 0,  480,  481,  484,  500, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 640x480@75Hz */
    { MODEPREFIX,    31500,  640,  664,  704,  832, 0,  480,  489,  491,  520, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 640x480@72Hz */
    { MODEPREFIX,    30240,  640,  704,  768,  864, 0,  480,  483,  486,  525, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 640x480@67Hz */
    { MODEPREFIX,    25200,  640,  656,  752,  800, 0,  480,  490,  492,  525, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 640x480@60Hz */
    { MODEPREFIX,    35500,  720,  738,  846,  900, 0,  400,  421,  423,  449, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 720x400@88Hz */
    { MODEPREFIX,    28320,  720,  738,  846,  900, 0,  400,  412,  414,  449, 0, V_NHSYNC | V_PVSYNC, MODESUFFIX }, /* 720x400@70Hz */
    { MODEPREFIX,   135000, 1280, 1296, 1440, 1688, 0, 1024, 1025, 1028, 1066, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 1280x1024@75Hz */
    { MODEPREFIX,    78800, 1024, 1040, 1136, 1312, 0,  768,  769,  772,  800, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 1024x768@75Hz */
    { MODEPREFIX,    75000, 1024, 1048, 1184, 1328, 0,  768,  771,  777,  806, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 1024x768@70Hz */
    { MODEPREFIX,    65000, 1024, 1048, 1184, 1344, 0,  768,  771,  777,  806, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 1024x768@60Hz */
    { MODEPREFIX,    44900, 1024, 1032, 1208, 1264, 0,  768,  768,  776,  817, 0, V_PHSYNC | V_PVSYNC | V_INTERLACE, MODESUFFIX }, /* 1024x768@43Hz */
    { MODEPREFIX,    57284,  832,  864,  928, 1152, 0,  624,  625,  628,  667, 0, V_NHSYNC | V_NVSYNC, MODESUFFIX }, /* 832x624@75Hz */
    { MODEPREFIX,    49500,  800,  816,  896, 1056, 0,  600,  601,  604,  625, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 800x600@75Hz */
    { MODEPREFIX,    50000,  800,  856,  976, 1040, 0,  600,  637,  643,  666, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 800x600@72Hz */
    { MODEPREFIX,   108000, 1152, 1216, 1344, 1600, 0,  864,  865,  868,  900, 0, V_PHSYNC | V_PVSYNC, MODESUFFIX }, /* 1152x864@75Hz */
};

static DisplayModePtr
DDCModesFromEstablished(int scrnIndex, struct established_timings *timing,
			ddc_quirk_t quirks)
{
    DisplayModePtr Modes = NULL, Mode = NULL;
    CARD32 bits = (timing->t1) | (timing->t2 << 8) |
        ((timing->t_manu & 0x80) << 9);
    int i;

    for (i = 0; i < 17; i++) {
        if (bits & (0x01 << i)) {
            Mode = xf86DuplicateMode(&DDCEstablishedModes[i]);
            Modes = xf86ModesAdd(Modes, Mode);
        }
    }

    return Modes;
}

#define LEVEL_DMT 0
#define LEVEL_GTF 1
#define LEVEL_CVT 2

static int
MonitorStandardTimingLevel(xf86MonPtr DDC)
{
    if (DDC->ver.revision >= 2) {
	if (DDC->ver.revision >= 4 && CVT_SUPPORTED(DDC->features.msc)) {
	    return LEVEL_CVT;
	}
	return LEVEL_GTF;
    }
    return LEVEL_DMT;
}

/*
 * This is not really correct.  Appendix B of the EDID 1.4 spec defines
 * the right thing to do here.  If the timing given here matches a mode
 * defined in the VESA DMT standard, we _must_ use that.  If the device
 * supports CVT modes, then we should generate a CVT timing.  If both
 * of the above fail, use GTF.
 *
 * There are some wrinkles here.  EDID 1.1 and 1.0 sinks can't really
 * "support" GTF, since it wasn't a standard yet; so if they ask for a
 * timing in this section that isn't defined in DMT, returning a GTF mode
 * may not actually be valid.  EDID 1.3 sinks often report support for
 * some CVT modes, but they are not required to support CVT timings for
 * modes in the standard timing descriptor, so we should _not_ treat them
 * as CVT-compliant (unless specified in an extension block I suppose).
 *
 * EDID 1.4 requires that all sink devices support both GTF and CVT timings
 * for modes in this section, but does say that CVT is preferred.
 */
static DisplayModePtr
DDCModesFromStandardTiming(struct std_timings *timing, ddc_quirk_t quirks,
			   int timing_level)
{
    DisplayModePtr Modes = NULL, Mode = NULL;
    int i;

    for (i = 0; i < STD_TIMINGS; i++) {
        if (timing[i].hsize && timing[i].vsize && timing[i].refresh) {
	    /* XXX check for DMT first, else... */
	    if (timing_level == LEVEL_CVT)
		Mode = xf86CVTMode(timing[i].hsize, timing[i].vsize,
				   timing[i].refresh, FALSE, FALSE);
	    else
		Mode = xf86GTFMode(timing[i].hsize, timing[i].vsize,
				   timing[i].refresh, FALSE, FALSE);

	    Mode->type = M_T_DRIVER;
            Modes = xf86ModesAdd(Modes, Mode);
        }
    }

    return Modes;
}

/*
 *
 */
static DisplayModePtr
DDCModeFromDetailedTiming(int scrnIndex, struct detailed_timings *timing,
			  Bool preferred, ddc_quirk_t quirks)
{
    DisplayModePtr Mode;

    /*
     * Refuse to create modes that are insufficiently large.  64 is a random
     * number, maybe the spec says something about what the minimum is.  In
     * particular I see this frequently with _old_ EDID, 1.0 or so, so maybe
     * our parser is just being too aggresive there.
     */
    if (timing->h_active < 64 || timing->v_active < 64) {
	xf86DrvMsg(scrnIndex, X_INFO,
		   "%s: Ignoring tiny %dx%d mode\n", __func__,
		   timing->h_active, timing->v_active);
	return NULL;
    }

    /* We don't do stereo */
    if (timing->stereo) {
        xf86DrvMsg(scrnIndex, X_INFO,
		   "%s: Ignoring: We don't handle stereo.\n", __func__);
        return NULL;
    }

    /* We only do seperate sync currently */
    if (timing->sync != 0x03) {
         xf86DrvMsg(scrnIndex, X_INFO,
		    "%s: %dx%d Warning: We only handle seperate"
                    " sync.\n", __func__, timing->h_active, timing->v_active);
    }

    Mode = xnfcalloc(1, sizeof(DisplayModeRec));

    Mode->type = M_T_DRIVER;
    if (preferred)
	Mode->type |= M_T_PREFERRED;

    if( ( quirks & DDC_QUIRK_135_CLOCK_TOO_HIGH ) &&
	timing->clock == 135000000 )
        Mode->Clock = 108880;
    else
        Mode->Clock = timing->clock / 1000.0;

    Mode->HDisplay = timing->h_active;
    Mode->HSyncStart = timing->h_active + timing->h_sync_off;
    Mode->HSyncEnd = Mode->HSyncStart + timing->h_sync_width;
    Mode->HTotal = timing->h_active + timing->h_blanking;

    Mode->VDisplay = timing->v_active;
    Mode->VSyncStart = timing->v_active + timing->v_sync_off;
    Mode->VSyncEnd = Mode->VSyncStart + timing->v_sync_width;
    Mode->VTotal = timing->v_active + timing->v_blanking;

    /* perform basic check on the detail timing */
    if (Mode->HSyncEnd > Mode->HTotal || Mode->VSyncEnd > Mode->VTotal) {
	xfree(Mode);
	return NULL;
    }

    xf86SetModeDefaultName(Mode);

    /* We ignore h/v_size and h/v_border for now. */

    if (timing->interlaced)
        Mode->Flags |= V_INTERLACE;

    if (quirks & DDC_QUIRK_DETAILED_SYNC_PP)
	Mode->Flags |= V_PVSYNC | V_PHSYNC;
    else {
	if (timing->misc & 0x02)
	    Mode->Flags |= V_PVSYNC;
	else
	    Mode->Flags |= V_NVSYNC;

	if (timing->misc & 0x01)
	    Mode->Flags |= V_PHSYNC;
	else
	    Mode->Flags |= V_NHSYNC;
    }

    return Mode;
}

static DisplayModePtr
DDCModesFromCVT(int scrnIndex, struct cvt_timings *t)
{
    DisplayModePtr modes = NULL;
    int i;

    for (i = 0; i < 4; i++) {
	if (t[i].height) {
	    if (t[i].rates & 0x10)
		modes = xf86ModesAdd(modes,
			xf86CVTMode(t[i].width, t[i].height, 50, 0, 0));
	    if (t[i].rates & 0x08)
		modes = xf86ModesAdd(modes,
			xf86CVTMode(t[i].width, t[i].height, 60, 0, 0));
	    if (t[i].rates & 0x04)
		modes = xf86ModesAdd(modes,
			xf86CVTMode(t[i].width, t[i].height, 75, 0, 0));
	    if (t[i].rates & 0x02)
		modes = xf86ModesAdd(modes,
			xf86CVTMode(t[i].width, t[i].height, 85, 0, 0));
	    if (t[i].rates & 0x01)
		modes = xf86ModesAdd(modes,
			xf86CVTMode(t[i].width, t[i].height, 60, 1, 0));
	} else break;
    }

    return modes;
}


/*
 * This is only valid when the sink claims to be continuous-frequency
 * but does not supply a detailed range descriptor.  Such sinks are
 * arguably broken.  Currently the mode validation code isn't aware of
 * this; the non-RANDR code even punts the decision of optional sync
 * range checking to the driver.  Loss.
 */
static void
DDCGuessRangesFromModes(int scrnIndex, MonPtr Monitor, DisplayModePtr Modes)
{
    DisplayModePtr Mode = Modes;

    if (!Monitor || !Modes)
        return;

    /* set up the ranges for scanning through the modes */
    Monitor->nHsync = 1;
    Monitor->hsync[0].lo = 1024.0;
    Monitor->hsync[0].hi = 0.0;

    Monitor->nVrefresh = 1;
    Monitor->vrefresh[0].lo = 1024.0;
    Monitor->vrefresh[0].hi = 0.0;

    while (Mode) {
        if (!Mode->HSync)
            Mode->HSync = ((float) Mode->Clock ) / ((float) Mode->HTotal);

        if (!Mode->VRefresh)
            Mode->VRefresh = (1000.0 * ((float) Mode->Clock)) / 
                ((float) (Mode->HTotal * Mode->VTotal));

        if (Mode->HSync < Monitor->hsync[0].lo)
            Monitor->hsync[0].lo = Mode->HSync;

        if (Mode->HSync > Monitor->hsync[0].hi)
            Monitor->hsync[0].hi = Mode->HSync;

        if (Mode->VRefresh < Monitor->vrefresh[0].lo)
            Monitor->vrefresh[0].lo = Mode->VRefresh;

        if (Mode->VRefresh > Monitor->vrefresh[0].hi)
            Monitor->vrefresh[0].hi = Mode->VRefresh;

        Mode = Mode->next;
    }
}

static ddc_quirk_t
xf86DDCDetectQuirks(int scrnIndex, xf86MonPtr DDC, Bool verbose)
{
    ddc_quirk_t	quirks;
    int i;

    quirks = DDC_QUIRK_NONE;
    for (i = 0; ddc_quirks[i].detect; i++) {
	if (ddc_quirks[i].detect (scrnIndex, DDC)) {
	    if (verbose) {
		xf86DrvMsg (scrnIndex, X_INFO, "    EDID quirk: %s\n",
			    ddc_quirks[i].description);
	    }
	    quirks |= ddc_quirks[i].quirk;
	}
    }

    return quirks;
}

/**
 * Applies monitor-specific quirks to the decoded EDID information.
 *
 * Note that some quirks applying to the mode list are still implemented in
 * xf86DDCGetModes.
 */
void
xf86DDCApplyQuirks(int scrnIndex, xf86MonPtr DDC)
{
    ddc_quirk_t quirks = xf86DDCDetectQuirks (scrnIndex, DDC, FALSE);
    int i;

    for (i = 0; i < DET_TIMINGS; i++) {
	struct detailed_monitor_section *det_mon = &DDC->det_mon[i];

	if (det_mon->type != DT)
	    continue;

	if (quirks & DDC_QUIRK_DETAILED_H_IN_CM)
	    det_mon->section.d_timings.h_size *= 10;

	if (quirks & DDC_QUIRK_DETAILED_V_IN_CM)
	    det_mon->section.d_timings.v_size *= 10;

	if (quirks & DDC_QUIRK_DETAILED_USE_MAXIMUM_SIZE) {
	    det_mon->section.d_timings.h_size = 10 * DDC->features.hsize;
	    det_mon->section.d_timings.v_size = 10 * DDC->features.vsize;
	}
    }
}

/**
 * Walks the modes list, finding the mode with the largest area which is
 * closest to the target refresh rate, and marks it as the only preferred mode.
*/
static void
xf86DDCSetPreferredRefresh(int scrnIndex, DisplayModePtr modes,
			   float target_refresh)
{
	DisplayModePtr	mode, best = modes;

	for (mode = modes; mode; mode = mode->next)
	{
	    mode->type &= ~M_T_PREFERRED;

	    if (mode == best) continue;

	    if (mode->HDisplay * mode->VDisplay >
		best->HDisplay * best->VDisplay)
	    {
		best = mode;
		continue;
	    }
	    if (mode->HDisplay * mode->VDisplay ==
		best->HDisplay * best->VDisplay)
	    {
		double	mode_refresh = xf86ModeVRefresh (mode);
		double	best_refresh = xf86ModeVRefresh (best);
		double	mode_dist = fabs(mode_refresh - target_refresh);
		double	best_dist = fabs(best_refresh - target_refresh);

		if (mode_dist < best_dist)
		{
		    best = mode;
		    continue;
		}
	    }
	}
	if (best)
	    best->type |= M_T_PREFERRED;
}

_X_EXPORT DisplayModePtr
xf86DDCGetModes(int scrnIndex, xf86MonPtr DDC)
{
    int		    i;
    DisplayModePtr  Modes = NULL, Mode;
    ddc_quirk_t	    quirks;
    Bool	    preferred;
    int		    timing_level;

    xf86DrvMsg (scrnIndex, X_INFO, "EDID vendor \"%s\", prod id %d\n",
		DDC->vendor.name, DDC->vendor.prod_id);

    quirks = xf86DDCDetectQuirks(scrnIndex, DDC, TRUE);

    preferred = PREFERRED_TIMING_MODE(DDC->features.msc);
    if (DDC->ver.revision >= 4)
	preferred = TRUE;
    if (quirks & DDC_QUIRK_FIRST_DETAILED_PREFERRED)
	preferred = TRUE;
    if (quirks & (DDC_QUIRK_PREFER_LARGE_60 | DDC_QUIRK_PREFER_LARGE_75))
	preferred = FALSE;

    timing_level = MonitorStandardTimingLevel(DDC);

    for (i = 0; i < DET_TIMINGS; i++) {
	struct detailed_monitor_section *det_mon = &DDC->det_mon[i];

        switch (det_mon->type) {
        case DT:
            Mode = DDCModeFromDetailedTiming(scrnIndex,
                                             &det_mon->section.d_timings,
					     preferred,
					     quirks);
	    preferred = FALSE;
            Modes = xf86ModesAdd(Modes, Mode);
            break;
        case DS_STD_TIMINGS:
            Mode = DDCModesFromStandardTiming(det_mon->section.std_t,
					      quirks, timing_level);
            Modes = xf86ModesAdd(Modes, Mode);
            break;
	case DS_CVT:
	    Mode = DDCModesFromCVT(scrnIndex, det_mon->section.cvt);
	    Modes = xf86ModesAdd(Modes, Mode);
	    break;
        default:
            break;
        }
    }

    /* Add established timings */
    Mode = DDCModesFromEstablished(scrnIndex, &DDC->timings1, quirks);
    Modes = xf86ModesAdd(Modes, Mode);

    /* Add standard timings */
    Mode = DDCModesFromStandardTiming(DDC->timings2, quirks, timing_level);
    Modes = xf86ModesAdd(Modes, Mode);

    if (quirks & DDC_QUIRK_PREFER_LARGE_60)
	xf86DDCSetPreferredRefresh(scrnIndex, Modes, 60);

    if (quirks & DDC_QUIRK_PREFER_LARGE_75)
	xf86DDCSetPreferredRefresh(scrnIndex, Modes, 75);

    return Modes;
}

/*
 * Fill out MonPtr with xf86MonPtr information.
 */
_X_EXPORT void
xf86DDCMonitorSet(int scrnIndex, MonPtr Monitor, xf86MonPtr DDC)
{
    DisplayModePtr Modes = NULL, Mode;
    int i, clock;
    Bool have_hsync = FALSE, have_vrefresh = FALSE, have_maxpixclock = FALSE;

    if (!Monitor || !DDC)
        return;

    Monitor->DDC = DDC;

    if (Monitor->widthmm <= 0 && Monitor->heightmm <= 0) {
	Monitor->widthmm = 10 * DDC->features.hsize;
	Monitor->heightmm = 10 * DDC->features.vsize;
    }

    /*
     * If this is a digital display, then we can use reduced blanking.
     * XXX This is a 1.3 heuristic.  1.4 explicitly defines rb support.
     */
    if (DDC->features.input_type)
        Monitor->reducedblanking = TRUE;

    Modes = xf86DDCGetModes(scrnIndex, DDC);

    /* Skip EDID ranges if they were specified in the config file */
    have_hsync = (Monitor->nHsync != 0);
    have_vrefresh = (Monitor->nVrefresh != 0);
    have_maxpixclock = (Monitor->maxPixClock != 0);

    /* Go through the detailed monitor sections */
    for (i = 0; i < DET_TIMINGS; i++) {
        switch (DDC->det_mon[i].type) {
        case DS_RANGES:
	    if (!have_hsync) {
		if (!Monitor->nHsync)
		    xf86DrvMsg(scrnIndex, X_INFO,
			    "Using EDID range info for horizontal sync\n");
		Monitor->hsync[Monitor->nHsync].lo =
		    DDC->det_mon[i].section.ranges.min_h;
		Monitor->hsync[Monitor->nHsync].hi =
		    DDC->det_mon[i].section.ranges.max_h;
		Monitor->nHsync++;
	    } else {
		xf86DrvMsg(scrnIndex, X_INFO,
			"Using hsync ranges from config file\n");
	    }

	    if (!have_vrefresh) {
		if (!Monitor->nVrefresh)
		    xf86DrvMsg(scrnIndex, X_INFO,
			    "Using EDID range info for vertical refresh\n");
		Monitor->vrefresh[Monitor->nVrefresh].lo =
		    DDC->det_mon[i].section.ranges.min_v;
		Monitor->vrefresh[Monitor->nVrefresh].hi =
		    DDC->det_mon[i].section.ranges.max_v;
		Monitor->nVrefresh++;
	    } else {
		xf86DrvMsg(scrnIndex, X_INFO,
			"Using vrefresh ranges from config file\n");
	    }

	    clock = DDC->det_mon[i].section.ranges.max_clock * 1000;
	    if (!have_maxpixclock && clock > Monitor->maxPixClock)
		Monitor->maxPixClock = clock;

            break;
        default:
            break;
        }
    }

    if (Modes) {
        /* Print Modes */
        xf86DrvMsg(scrnIndex, X_INFO, "Printing DDC gathered Modelines:\n");

        Mode = Modes;
        while (Mode) {
            xf86PrintModeline(scrnIndex, Mode);
            Mode = Mode->next;
        }

        /* Do we still need ranges to be filled in? */
        if (!Monitor->nHsync || !Monitor->nVrefresh)
            DDCGuessRangesFromModes(scrnIndex, Monitor, Modes);

        /* look for last Mode */
        Mode = Modes;

        while (Mode->next)
            Mode = Mode->next;

        /* add to MonPtr */
        if (Monitor->Modes) {
            Monitor->Last->next = Modes;
            Modes->prev = Monitor->Last;
            Monitor->Last = Mode;
        } else {
            Monitor->Modes = Modes;
            Monitor->Last = Mode;
        }
    }
}
