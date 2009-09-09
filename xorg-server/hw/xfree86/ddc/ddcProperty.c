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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "xf86.h"
#include "xf86DDC.h"
#include <X11/Xatom.h>
#include "property.h"
#include "propertyst.h"
#include "xf86DDC.h"
#include <string.h>

#define EDID1_ATOM_NAME         "XFree86_DDC_EDID1_RAWDATA"
#define EDID2_ATOM_NAME         "XFree86_DDC_EDID2_RAWDATA"

static void
edidMakeAtom(int i, const char *name, CARD8 *data, int size)
{
    Atom atom;
    unsigned char *atom_data;

    if (!(atom_data = xalloc(size*sizeof(CARD8))))
	return;

    atom = MakeAtom(name, strlen(name), TRUE);
    memcpy(atom_data, data, size);
    xf86RegisterRootWindowProperty(i, atom, XA_INTEGER, 8, size, atom_data);
}

static void
addRootWindowProperties(ScrnInfoPtr pScrn, xf86MonPtr DDC)
{
    int i, scrnIndex = pScrn->scrnIndex;
    Bool makeEDID1prop = FALSE;
    Bool makeEDID2prop = FALSE;

    if (DDC->flags & MONITOR_DISPLAYID) {
	/* Don't bother, use RANDR already */
	return;
    } else if (DDC->ver.version == 1) {
	makeEDID1prop = TRUE;
    } else if (DDC->ver.version == 2) {
	int checksum1;
	int checksum2;
	makeEDID2prop = TRUE;

	/* Some monitors (eg Panasonic PanaSync4)
	 * report version==2 because they used EDID v2 spec document,
	 * although they use EDID v1 data structure :-(
	 *
	 * Try using checksum to determine when we have such a monitor.
	 */
	checksum2 = 0;
	for (i = 0; i < 256; i++)
	    checksum2 += DDC->rawData[i];
	if (checksum2 % 256) {
	    xf86DrvMsg(scrnIndex, X_INFO, "Monitor EDID v2 checksum failed\n");
	    xf86DrvMsg(scrnIndex, X_INFO,
		    "XFree86_DDC_EDID2_RAWDATA property may be bad\n");
	    checksum1 = 0;
	    for (i = 0; i < 128; i++)
		checksum1 += DDC->rawData[i];
	    if (!(checksum1 % 256)) {
		xf86DrvMsg(scrnIndex, X_INFO,
			"Monitor EDID v1 checksum passed,\n");
		xf86DrvMsg(scrnIndex, X_INFO,
			"XFree86_DDC_EDID1_RAWDATA property created\n");
		makeEDID1prop = TRUE;
	    }
	}
    } else {
	xf86DrvMsg(scrnIndex, X_PROBED, "unexpected EDID version %d.%d\n",
		DDC->ver.version, DDC->ver.revision);
	return;
    }

    if (makeEDID1prop) {
	int size = 128 +
	    (DDC->flags & EDID_COMPLETE_RAWDATA ? DDC->no_sections * 128 : 0);

	edidMakeAtom(scrnIndex, EDID1_ATOM_NAME, DDC->rawData, size);
    } 

    if (makeEDID2prop)
	edidMakeAtom(scrnIndex, EDID2_ATOM_NAME, DDC->rawData, 256);
}

Bool
xf86SetDDCproperties(ScrnInfoPtr pScrn, xf86MonPtr DDC)
{
    if (!pScrn || !pScrn->monitor || !DDC)
        return FALSE;

    if (DDC->flags & MONITOR_DISPLAYID)
	;
    else
	xf86EdidMonitorSet(pScrn->scrnIndex, pScrn->monitor, DDC);

    addRootWindowProperties(pScrn, DDC);

    return TRUE;
}
