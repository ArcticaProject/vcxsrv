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

/* This file holds the driver allocated pixmaps + better initial placement code.
 */

static _X_INLINE void*
ExaGetPixmapAddress(PixmapPtr p)
{
    ExaPixmapPriv(p);

    return pExaPixmap->sys_ptr;
}

/**
 * exaCreatePixmap() creates a new pixmap.
 */
PixmapPtr
exaCreatePixmap_mixed(ScreenPtr pScreen, int w, int h, int depth,
		unsigned usage_hint)
{
    PixmapPtr pPixmap;
    ExaPixmapPrivPtr	pExaPixmap;
    int bpp;
    size_t paddedWidth;
    ExaScreenPriv(pScreen);

    if (w > 32767 || h > 32767)
	return NullPixmap;

    swap(pExaScr, pScreen, CreatePixmap);
    pPixmap = pScreen->CreatePixmap(pScreen, 0, 0, depth, usage_hint);
    swap(pExaScr, pScreen, CreatePixmap);

    if (!pPixmap)
        return NULL;

    pExaPixmap = ExaGetPixmapPriv(pPixmap);
    pExaPixmap->driverPriv = NULL;

    bpp = pPixmap->drawable.bitsPerPixel;

    paddedWidth = ((w * bpp + FB_MASK) >> FB_SHIFT) * sizeof(FbBits);
    if (paddedWidth / 4 > 32767 || h > 32767)
        return NullPixmap;

    /* We will allocate the system pixmap later if needed. */
    pPixmap->devPrivate.ptr = NULL;
    pExaPixmap->sys_ptr = NULL;
    pExaPixmap->sys_pitch = paddedWidth;

    pExaPixmap->area = NULL;
    pExaPixmap->fb_ptr = NULL;
    pExaPixmap->pDamage = NULL;

    exaSetFbPitch(pExaScr, pExaPixmap, w, h, bpp);
    exaSetAccelBlock(pExaScr, pExaPixmap,
	w, h, bpp);

    (*pScreen->ModifyPixmapHeader)(pPixmap, w, h, 0, 0,
				    paddedWidth, NULL);

    /* A scratch pixmap will become a driver pixmap right away. */
    if (!w || !h) {
	exaCreateDriverPixmap_mixed(pPixmap);
	pExaPixmap->offscreen = exaPixmapIsOffscreen(pPixmap);
    } else
	pExaPixmap->offscreen = FALSE;

    return pPixmap;
}

Bool
exaModifyPixmapHeader_mixed(PixmapPtr pPixmap, int width, int height, int depth,
		      int bitsPerPixel, int devKind, pointer pPixData)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    ExaScreenPrivPtr pExaScr;
    ExaPixmapPrivPtr pExaPixmap;
    Bool ret, is_offscreen;

    if (!pPixmap)
        return FALSE;

    pExaScr = ExaGetScreenPriv(pScreen);
    pExaPixmap = ExaGetPixmapPriv(pPixmap);

    if (pPixData) {
	if (pExaPixmap->driverPriv) {
	    if (pExaPixmap->pDamage) {
		DamageUnregister(&pPixmap->drawable, pExaPixmap->pDamage);
		DamageDestroy(pExaPixmap->pDamage);
		pExaPixmap->pDamage = NULL;
	    }

	    pExaScr->info->DestroyPixmap(pScreen, pExaPixmap->driverPriv);
	    pExaPixmap->driverPriv = NULL;
	}

	pExaPixmap->offscreen = FALSE;
	pExaPixmap->score = EXA_PIXMAP_SCORE_PINNED;
    }

    if (pExaPixmap->driverPriv) {
        if (width > 0 && height > 0 && bitsPerPixel > 0) {
            exaSetFbPitch(pExaScr, pExaPixmap,
                          width, height, bitsPerPixel);

            exaSetAccelBlock(pExaScr, pExaPixmap,
                             width, height, bitsPerPixel);
        }
    }

    is_offscreen = exaPixmapIsOffscreen(pPixmap);
    if (is_offscreen) {
	pPixmap->devPrivate.ptr = pExaPixmap->fb_ptr;
	pPixmap->devKind = pExaPixmap->fb_pitch;
    } else {
	pPixmap->devPrivate.ptr = pExaPixmap->sys_ptr;
	pPixmap->devKind = pExaPixmap->sys_pitch;
    }

    /* Only pass driver pixmaps to the driver. */
    if (pExaScr->info->ModifyPixmapHeader && pExaPixmap->driverPriv) {
	ret = pExaScr->info->ModifyPixmapHeader(pPixmap, width, height, depth,
						bitsPerPixel, devKind, pPixData);
	/* For EXA_HANDLES_PIXMAPS, we set pPixData to NULL.
	 * If pPixmap->devPrivate.ptr is non-NULL, then we've got a non-offscreen pixmap.
	 * We need to store the pointer, because PrepareAccess won't be called.
	 */
	if (ret == TRUE)
	    goto out;
    }

    swap(pExaScr, pScreen, ModifyPixmapHeader);
    ret = pScreen->ModifyPixmapHeader(pPixmap, width, height, depth,
					    bitsPerPixel, devKind, pPixData);
    swap(pExaScr, pScreen, ModifyPixmapHeader);

out:
    if (is_offscreen) {
	pExaPixmap->fb_ptr = pPixmap->devPrivate.ptr;
	pExaPixmap->fb_pitch = pPixmap->devKind;
    } else {
	pExaPixmap->sys_ptr = pPixmap->devPrivate.ptr;
	pExaPixmap->sys_pitch = pPixmap->devKind;
    }
    /* Always NULL this, we don't want lingering pointers. */
    pPixmap->devPrivate.ptr = NULL;

    return ret;
}

Bool
exaDestroyPixmap_mixed(PixmapPtr pPixmap)
{
    ScreenPtr	pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);
    Bool ret;

    if (pPixmap->refcnt == 1)
    {
	ExaPixmapPriv (pPixmap);

	if (pExaPixmap->driverPriv)
	    pExaScr->info->DestroyPixmap(pScreen, pExaPixmap->driverPriv);
	pExaPixmap->driverPriv = NULL;

	if (pExaPixmap->pDamage) {
	    if (pExaPixmap->sys_ptr)
		free(pExaPixmap->sys_ptr);
	    pExaPixmap->sys_ptr = NULL;
	    pExaPixmap->pDamage = NULL;
	}
    }

    swap(pExaScr, pScreen, DestroyPixmap);
    ret = pScreen->DestroyPixmap (pPixmap);
    swap(pExaScr, pScreen, DestroyPixmap);

    return ret;
}

Bool
exaPixmapIsOffscreen_mixed(PixmapPtr pPixmap)
{
    ScreenPtr pScreen = pPixmap->drawable.pScreen;
    ExaScreenPriv(pScreen);
    ExaPixmapPriv(pPixmap);
    pointer saved_ptr;
    Bool ret;

    if (!pExaPixmap->driverPriv)
	return FALSE;

    saved_ptr = pPixmap->devPrivate.ptr;
    pPixmap->devPrivate.ptr = ExaGetPixmapAddress(pPixmap);
    ret = pExaScr->info->PixmapIsOffscreen(pPixmap);
    pPixmap->devPrivate.ptr = saved_ptr;

    return ret;
}
