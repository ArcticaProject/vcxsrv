/*
 * Copyright © 2009 Maarten Maathuis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <string.h>

#include "exa_priv.h"
#include "exa.h"

void
exaCreateDriverPixmap_mixed(PixmapPtr pPixmap)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);
    ExaPixmapPriv(pPixmap);
    int w = pPixmap->drawable.width, h = pPixmap->drawable.height;
    int depth = pPixmap->drawable.depth, bpp = pPixmap->drawable.bitsPerPixel;
    int usage_hint = pPixmap->usage_hint;
    int paddedWidth = pExaPixmap->sys_pitch;

    /* Already done. */
    if (pExaPixmap->driverPriv)
	return;

    if (exaPixmapIsPinned(pPixmap))
	return;

    /* Can't accel 1/4 bpp. */
    if (pExaPixmap->accel_blocked || bpp < 8)
	return;

    if (pExaScr->info->CreatePixmap2) {
	int new_pitch = 0;
        pExaPixmap->driverPriv = pExaScr->info->CreatePixmap2(pScreen, w, h, depth, usage_hint, bpp, &new_pitch);
	paddedWidth = pExaPixmap->fb_pitch = new_pitch;
    } else {
	if (paddedWidth < pExaPixmap->fb_pitch)
	    paddedWidth = pExaPixmap->fb_pitch;
	pExaPixmap->driverPriv = pExaScr->info->CreatePixmap(pScreen, paddedWidth*h, 0);
    }

    if (!pExaPixmap->driverPriv)
	return;

    (*pScreen->ModifyPixmapHeader)(pPixmap, w, h, 0, 0,
				paddedWidth, NULL);
}

void
exaDoMigration_mixed(ExaMigrationPtr pixmaps, int npixmaps, Bool can_accel)
{
    int i;

    /* If anything is pinned in system memory, we won't be able to
     * accelerate.
     */
    for (i = 0; i < npixmaps; i++) {
	if (exaPixmapIsPinned (pixmaps[i].pPix) &&
	    !exaPixmapIsOffscreen (pixmaps[i].pPix))
	{
	    can_accel = FALSE;
	    break;
	}
    }

    /* We can do nothing. */
    if (!can_accel)
	return;

    for (i = 0; i < npixmaps; i++) {
	PixmapPtr pPixmap = pixmaps[i].pPix;
	ExaPixmapPriv(pPixmap);

	if (!pExaPixmap->driverPriv)
	    exaCreateDriverPixmap_mixed(pPixmap);

	if (pExaPixmap->pDamage && exaPixmapIsOffscreen(pPixmap)) {
	    pPixmap->devKind = pExaPixmap->fb_pitch;
	    exaCopyDirtyToFb(pixmaps + i);
	}

	pExaPixmap->offscreen = exaPixmapIsOffscreen(pPixmap);
    }
}

void
exaMoveInPixmap_mixed(PixmapPtr pPixmap)
{
    ExaMigrationRec pixmaps[1];

    pixmaps[0].as_dst = FALSE;
    pixmaps[0].as_src = TRUE;
    pixmaps[0].pPix = pPixmap;
    pixmaps[0].pReg = NULL;

    exaDoMigration(pixmaps, 1, TRUE);
}

/* With mixed pixmaps, if we fail to get direct access to the driver pixmap, we
 * use the DownloadFromScreen hook to retrieve contents to a copy in system
 * memory, perform software rendering on that and move back the results with the
 * UploadToScreen hook (see exaFinishAccess_mixed).
 */
void
exaPrepareAccessReg_mixed(PixmapPtr pPixmap, int index, RegionPtr pReg)
{
    if (!ExaDoPrepareAccess(pPixmap, index)) {
	ExaPixmapPriv(pPixmap);
	Bool is_offscreen = exaPixmapIsOffscreen(pPixmap);
	ExaMigrationRec pixmaps[1];

	/* Do we need to allocate our system buffer? */
	if (!pExaPixmap->sys_ptr) {
	    pExaPixmap->sys_ptr = malloc(pExaPixmap->sys_pitch *
					 pPixmap->drawable.height);
	    if (!pExaPixmap->sys_ptr)
		FatalError("EXA: malloc failed for size %d bytes\n",
			   pExaPixmap->sys_pitch * pPixmap->drawable.height);
	}

	if (index == EXA_PREPARE_DEST || index == EXA_PREPARE_AUX_DEST) {
	    pixmaps[0].as_dst = TRUE;
	    pixmaps[0].as_src = FALSE;
	} else {
	    pixmaps[0].as_dst = FALSE;
	    pixmaps[0].as_src = TRUE;
	}
	pixmaps[0].pPix = pPixmap;
	pixmaps[0].pReg = pReg;

	if (!pExaPixmap->pDamage && (is_offscreen || !exaPixmapIsPinned(pPixmap))) {
	    Bool as_dst = pixmaps[0].as_dst;

	    /* Set up damage tracking */
	    pExaPixmap->pDamage = DamageCreate(NULL, NULL, DamageReportNone,
					       TRUE, pPixmap->drawable.pScreen,
					       pPixmap);

	    DamageRegister(&pPixmap->drawable, pExaPixmap->pDamage);
	    /* This ensures that pending damage reflects the current operation. */
	    /* This is used by exa to optimize migration. */
	    DamageSetReportAfterOp(pExaPixmap->pDamage, TRUE);

	    if (is_offscreen) {
		exaPixmapDirty(pPixmap, 0, 0, pPixmap->drawable.width,
			       pPixmap->drawable.height);

		/* We don't know which region of the destination will be damaged,
		 * have to assume all of it
		 */
		if (as_dst) {
		    pixmaps[0].as_dst = FALSE;
		    pixmaps[0].as_src = TRUE;
		    pixmaps[0].pReg = NULL;
		}
		pPixmap->devKind = pExaPixmap->fb_pitch;
		exaCopyDirtyToSys(pixmaps);
	    }

	    if (as_dst)
		exaPixmapDirty(pPixmap, 0, 0, pPixmap->drawable.width,
			       pPixmap->drawable.height);
	} else if (is_offscreen) {
	    pPixmap->devKind = pExaPixmap->fb_pitch;
	    exaCopyDirtyToSys(pixmaps);
	}

	pPixmap->devPrivate.ptr = pExaPixmap->sys_ptr;
	pPixmap->devKind = pExaPixmap->sys_pitch;
	pExaPixmap->offscreen = FALSE;
    }
}

/* Move back results of software rendering on system memory copy of mixed driver
 * pixmap (see exaPrepareAccessReg_mixed).
 */
void exaFinishAccess_mixed(PixmapPtr pPixmap, int index)
{
    ExaPixmapPriv(pPixmap);

    if (pExaPixmap->pDamage && exaPixmapIsOffscreen(pPixmap)) {
	DamageRegionProcessPending(&pPixmap->drawable);
	exaMoveInPixmap_mixed(pPixmap);
    }
}
