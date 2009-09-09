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

static void
exaUploadFallback(PixmapPtr pPixmap, CARD8 *src, int src_pitch)
{
    ExaPixmapPriv(pPixmap);
    RegionPtr damage = DamageRegion (pExaPixmap->pDamage);
    GCPtr pGC = GetScratchGC (pPixmap->drawable.depth,
		pPixmap->drawable.pScreen);
    int nbox, cpp = pPixmap->drawable.bitsPerPixel / 8;
    DamagePtr backup = pExaPixmap->pDamage;
    BoxPtr pbox;
    CARD8 *src2;

    /* We don't want damage optimisations. */
    pExaPixmap->pDamage = NULL;
    ValidateGC (&pPixmap->drawable, pGC);

    pbox = REGION_RECTS(damage);
    nbox = REGION_NUM_RECTS(damage);

    while (nbox--) {
	src2 = src + pbox->y1 * src_pitch + pbox->x1 * cpp;

	ExaCheckPutImage(&pPixmap->drawable, pGC,
	    pPixmap->drawable.depth, pbox->x1, pbox->y1,
	    pbox->x2 - pbox->x1, pbox->y2 - pbox->y1, 0,
	    ZPixmap, (char*) src2);

	pbox++;
    }

    FreeScratchGC (pGC);
    pExaPixmap->pDamage = backup;
}

void
exaCreateDriverPixmap_mixed(PixmapPtr pPixmap)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);
    ExaPixmapPriv(pPixmap);
    RegionPtr damage = DamageRegion (pExaPixmap->pDamage);
    void *sys_buffer = pExaPixmap->sys_ptr;
    int w = pPixmap->drawable.width, h = pPixmap->drawable.height;
    int depth = pPixmap->drawable.depth, bpp = pPixmap->drawable.bitsPerPixel;
    int usage_hint = pPixmap->usage_hint;
    int sys_pitch = pExaPixmap->sys_pitch;
    int paddedWidth = sys_pitch;
    int nbox;
    BoxPtr pbox;

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

    pExaPixmap->offscreen = TRUE;
    pExaPixmap->sys_ptr = pPixmap->devPrivate.ptr = NULL;
    pExaPixmap->sys_pitch = pPixmap->devKind = 0;

    pExaPixmap->score = EXA_PIXMAP_SCORE_PINNED;
    (*pScreen->ModifyPixmapHeader)(pPixmap, w, h, 0, 0,
				paddedWidth, NULL);

    /* scratch pixmaps */
    if (!w || !h)
	goto finish;

    /* we do not malloc memory by default. */
    if (!sys_buffer)
	goto finish;

    if (!pExaScr->info->UploadToScreen)
	goto fallback;

    pbox = REGION_RECTS(damage);
    nbox = REGION_NUM_RECTS(damage);

    while (nbox--) {
	if (!pExaScr->info->UploadToScreen(pPixmap, pbox->x1, pbox->y1, pbox->x2 - pbox->x1,
		pbox->y2 - pbox->y1, (char *) (sys_buffer) + pbox->y1 * sys_pitch + pbox->x1 * (bpp / 8), sys_pitch))
	    goto fallback;

	pbox++;
    }

    goto finish;

fallback:
    exaUploadFallback(pPixmap, sys_buffer, sys_pitch);

finish:
    free(sys_buffer);

    /* We no longer need this. */
    if (pExaPixmap->pDamage) {
	DamageUnregister(&pPixmap->drawable, pExaPixmap->pDamage);
	DamageDestroy(pExaPixmap->pDamage);
	pExaPixmap->pDamage = NULL;
    }
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
