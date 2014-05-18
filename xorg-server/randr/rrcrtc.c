/*
 * Copyright © 2006 Keith Packard
 * Copyright 2010 Red Hat, Inc
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include "randrstr.h"
#include "swaprep.h"
#include "mipointer.h"

RESTYPE RRCrtcType;

/*
 * Notify the CRTC of some change
 */
void
RRCrtcChanged(RRCrtcPtr crtc, Bool layoutChanged)
{
    ScreenPtr pScreen = crtc->pScreen;

    crtc->changed = TRUE;
    if (pScreen) {
        rrScrPriv(pScreen);

        RRSetChanged(pScreen);
        /*
         * Send ConfigureNotify on any layout change
         */
        if (layoutChanged)
            pScrPriv->layoutChanged = TRUE;
    }
}

/*
 * Create a CRTC
 */
RRCrtcPtr
RRCrtcCreate(ScreenPtr pScreen, void *devPrivate)
{
    RRCrtcPtr crtc;
    RRCrtcPtr *crtcs;
    rrScrPrivPtr pScrPriv;

    if (!RRInit())
        return NULL;

    pScrPriv = rrGetScrPriv(pScreen);

    /* make space for the crtc pointer */
    if (pScrPriv->numCrtcs)
        crtcs = realloc(pScrPriv->crtcs,
                        (pScrPriv->numCrtcs + 1) * sizeof(RRCrtcPtr));
    else
        crtcs = malloc(sizeof(RRCrtcPtr));
    if (!crtcs)
        return FALSE;
    pScrPriv->crtcs = crtcs;

    crtc = calloc(1, sizeof(RRCrtcRec));
    if (!crtc)
        return NULL;
    crtc->id = FakeClientID(0);
    crtc->pScreen = pScreen;
    crtc->mode = NULL;
    crtc->x = 0;
    crtc->y = 0;
    crtc->rotation = RR_Rotate_0;
    crtc->rotations = RR_Rotate_0;
    crtc->outputs = NULL;
    crtc->numOutputs = 0;
    crtc->gammaSize = 0;
    crtc->gammaRed = crtc->gammaBlue = crtc->gammaGreen = NULL;
    crtc->changed = FALSE;
    crtc->devPrivate = devPrivate;
    RRTransformInit(&crtc->client_pending_transform);
    RRTransformInit(&crtc->client_current_transform);
    pixman_transform_init_identity(&crtc->transform);
    pixman_f_transform_init_identity(&crtc->f_transform);
    pixman_f_transform_init_identity(&crtc->f_inverse);

    if (!AddResource(crtc->id, RRCrtcType, (void *) crtc))
        return NULL;

    /* attach the screen and crtc together */
    crtc->pScreen = pScreen;
    pScrPriv->crtcs[pScrPriv->numCrtcs++] = crtc;

    RRResourcesChanged(pScreen);

    return crtc;
}

/*
 * Set the allowed rotations on a CRTC
 */
void
RRCrtcSetRotations(RRCrtcPtr crtc, Rotation rotations)
{
    crtc->rotations = rotations;
}

/*
 * Set whether transforms are allowed on a CRTC
 */
void
RRCrtcSetTransformSupport(RRCrtcPtr crtc, Bool transforms)
{
    crtc->transforms = transforms;
}

/*
 * Notify the extension that the Crtc has been reconfigured,
 * the driver calls this whenever it has updated the mode
 */
Bool
RRCrtcNotify(RRCrtcPtr crtc,
             RRModePtr mode,
             int x,
             int y,
             Rotation rotation,
             RRTransformPtr transform, int numOutputs, RROutputPtr * outputs)
{
    int i, j;

    /*
     * Check to see if any of the new outputs were
     * not in the old list and mark them as changed
     */
    for (i = 0; i < numOutputs; i++) {
        for (j = 0; j < crtc->numOutputs; j++)
            if (outputs[i] == crtc->outputs[j])
                break;
        if (j == crtc->numOutputs) {
            outputs[i]->crtc = crtc;
            RROutputChanged(outputs[i], FALSE);
            RRCrtcChanged(crtc, FALSE);
        }
    }
    /*
     * Check to see if any of the old outputs are
     * not in the new list and mark them as changed
     */
    for (j = 0; j < crtc->numOutputs; j++) {
        for (i = 0; i < numOutputs; i++)
            if (outputs[i] == crtc->outputs[j])
                break;
        if (i == numOutputs) {
            if (crtc->outputs[j]->crtc == crtc)
                crtc->outputs[j]->crtc = NULL;
            RROutputChanged(crtc->outputs[j], FALSE);
            RRCrtcChanged(crtc, FALSE);
        }
    }
    /*
     * Reallocate the crtc output array if necessary
     */
    if (numOutputs != crtc->numOutputs) {
        RROutputPtr *newoutputs;

        if (numOutputs) {
            if (crtc->numOutputs)
                newoutputs = realloc(crtc->outputs,
                                     numOutputs * sizeof(RROutputPtr));
            else
                newoutputs = malloc(numOutputs * sizeof(RROutputPtr));
            if (!newoutputs)
                return FALSE;
        }
        else {
            free(crtc->outputs);
            newoutputs = NULL;
        }
        crtc->outputs = newoutputs;
        crtc->numOutputs = numOutputs;
    }
    /*
     * Copy the new list of outputs into the crtc
     */
    memcpy(crtc->outputs, outputs, numOutputs * sizeof(RROutputPtr));
    /*
     * Update remaining crtc fields
     */
    if (mode != crtc->mode) {
        if (crtc->mode)
            RRModeDestroy(crtc->mode);
        crtc->mode = mode;
        if (mode != NULL)
            mode->refcnt++;
        RRCrtcChanged(crtc, TRUE);
    }
    if (x != crtc->x) {
        crtc->x = x;
        RRCrtcChanged(crtc, TRUE);
    }
    if (y != crtc->y) {
        crtc->y = y;
        RRCrtcChanged(crtc, TRUE);
    }
    if (rotation != crtc->rotation) {
        crtc->rotation = rotation;
        RRCrtcChanged(crtc, TRUE);
    }
    if (!RRTransformEqual(transform, &crtc->client_current_transform)) {
        RRTransformCopy(&crtc->client_current_transform, transform);
        RRCrtcChanged(crtc, TRUE);
    }
    if (crtc->changed && mode) {
        RRTransformCompute(x, y,
                           mode->mode.width, mode->mode.height,
                           rotation,
                           &crtc->client_current_transform,
                           &crtc->transform, &crtc->f_transform,
                           &crtc->f_inverse);
    }
    return TRUE;
}

void
RRDeliverCrtcEvent(ClientPtr client, WindowPtr pWin, RRCrtcPtr crtc)
{
    ScreenPtr pScreen = pWin->drawable.pScreen;

    rrScrPriv(pScreen);
    RRModePtr mode = crtc->mode;

    xRRCrtcChangeNotifyEvent ce = {
        .type = RRNotify + RREventBase,
        .subCode = RRNotify_CrtcChange,
        .timestamp = pScrPriv->lastSetTime.milliseconds,
        .window = pWin->drawable.id,
        .crtc = crtc->id,
        .mode = mode ? mode->mode.id : None,
        .rotation = crtc->rotation,
        .x = mode ? crtc->x : 0,
        .y = mode ? crtc->y : 0,
        .width = mode ? mode->mode.width : 0,
        .height = mode ? mode->mode.height : 0
    };
    WriteEventsToClient(client, 1, (xEvent *) &ce);
}

static Bool
RRCrtcPendingProperties(RRCrtcPtr crtc)
{
    ScreenPtr pScreen = crtc->pScreen;

    rrScrPriv(pScreen);
    int o;

    for (o = 0; o < pScrPriv->numOutputs; o++) {
        RROutputPtr output = pScrPriv->outputs[o];

        if (output->crtc == crtc && output->pendingProperties)
            return TRUE;
    }
    return FALSE;
}

static void
crtc_bounds(RRCrtcPtr crtc, int *left, int *right, int *top, int *bottom)
{
    *left = crtc->x;
    *top = crtc->y;

    switch (crtc->rotation) {
    case RR_Rotate_0:
    case RR_Rotate_180:
    default:
        *right = crtc->x + crtc->mode->mode.width;
        *bottom = crtc->y + crtc->mode->mode.height;
        return;
    case RR_Rotate_90:
    case RR_Rotate_270:
        *right = crtc->x + crtc->mode->mode.height;
        *bottom = crtc->y + crtc->mode->mode.width;
        return;
    }
}

/* overlapping counts as adjacent */
static Bool
crtcs_adjacent(const RRCrtcPtr a, const RRCrtcPtr b)
{
    /* left, right, top, bottom... */
    int al, ar, at, ab;
    int bl, br, bt, bb;
    int cl, cr, ct, cb;         /* the overlap, if any */

    crtc_bounds(a, &al, &ar, &at, &ab);
    crtc_bounds(b, &bl, &br, &bt, &bb);

    cl = max(al, bl);
    cr = min(ar, br);
    ct = max(at, bt);
    cb = min(ab, bb);

    return (cl <= cr) && (ct <= cb);
}

/* Depth-first search and mark all CRTCs reachable from cur */
static void
mark_crtcs(rrScrPrivPtr pScrPriv, int *reachable, int cur)
{
    int i;

    reachable[cur] = TRUE;
    for (i = 0; i < pScrPriv->numCrtcs; ++i) {
        if (reachable[i] || !pScrPriv->crtcs[i]->mode)
            continue;
        if (crtcs_adjacent(pScrPriv->crtcs[cur], pScrPriv->crtcs[i]))
            mark_crtcs(pScrPriv, reachable, i);
    }
}

static void
RRComputeContiguity(ScreenPtr pScreen)
{
    rrScrPriv(pScreen);
    Bool discontiguous = TRUE;
    int i, n = pScrPriv->numCrtcs;

    int *reachable = calloc(n, sizeof(int));

    if (!reachable)
        goto out;

    /* Find first enabled CRTC and start search for reachable CRTCs from it */
    for (i = 0; i < n; ++i) {
        if (pScrPriv->crtcs[i]->mode) {
            mark_crtcs(pScrPriv, reachable, i);
            break;
        }
    }

    /* Check that all enabled CRTCs were marked as reachable */
    for (i = 0; i < n; ++i)
        if (pScrPriv->crtcs[i]->mode && !reachable[i])
            goto out;

    discontiguous = FALSE;

 out:
    free(reachable);
    pScrPriv->discontiguous = discontiguous;
}

void
RRCrtcDetachScanoutPixmap(RRCrtcPtr crtc)
{
    ScreenPtr master = crtc->pScreen->current_master;
    PixmapPtr mscreenpix;
    rrScrPriv(crtc->pScreen);

    mscreenpix = master->GetScreenPixmap(master);

    pScrPriv->rrCrtcSetScanoutPixmap(crtc, NULL);
    if (crtc->scanout_pixmap) {
        master->StopPixmapTracking(mscreenpix, crtc->scanout_pixmap);
        /*
         * Unref the pixmap twice: once for the original reference, and once
         * for the reference implicitly added by PixmapShareToSlave.
         */
        master->DestroyPixmap(crtc->scanout_pixmap->master_pixmap);
        master->DestroyPixmap(crtc->scanout_pixmap->master_pixmap);
        crtc->pScreen->DestroyPixmap(crtc->scanout_pixmap);
    }
    crtc->scanout_pixmap = NULL;
    RRCrtcChanged(crtc, TRUE);
}

static Bool
rrCreateSharedPixmap(RRCrtcPtr crtc, int width, int height,
                     int x, int y)
{
    PixmapPtr mpix, spix;
    ScreenPtr master = crtc->pScreen->current_master;
    Bool ret;
    int depth;
    PixmapPtr mscreenpix;
    PixmapPtr protopix = crtc->pScreen->current_master->GetScreenPixmap(crtc->pScreen->current_master);
    rrScrPriv(crtc->pScreen);

    /* create a pixmap on the master screen,
       then get a shared handle for it
       create a shared pixmap on the slave screen using the handle
       set the master screen to do dirty updates to the shared pixmap
       from the screen pixmap.
       set slave screen to scanout shared linear pixmap
    */

    mscreenpix = master->GetScreenPixmap(master);
    depth = protopix->drawable.depth;

    if (crtc->scanout_pixmap)
        RRCrtcDetachScanoutPixmap(crtc);

    if (width == 0 && height == 0) {
        return TRUE;
    }

    mpix = master->CreatePixmap(master, width, height, depth,
                                CREATE_PIXMAP_USAGE_SHARED);
    if (!mpix)
        return FALSE;

    spix = PixmapShareToSlave(mpix, crtc->pScreen);
    if (spix == NULL) {
        master->DestroyPixmap(mpix);
        return FALSE;
    }

    ret = pScrPriv->rrCrtcSetScanoutPixmap(crtc, spix);
    if (ret == FALSE) {
        ErrorF("failed to set shadow slave pixmap\n");
        return FALSE;
    }

    crtc->scanout_pixmap = spix;

    master->StartPixmapTracking(mscreenpix, spix, x, y);
    return TRUE;
}

static Bool
rrCheckPixmapBounding(ScreenPtr pScreen,
                      RRCrtcPtr rr_crtc, int x, int y, int w, int h)
{
    RegionRec root_pixmap_region, total_region, new_crtc_region;
    int c;
    BoxRec newbox;
    BoxPtr newsize;
    ScreenPtr slave;
    int new_width, new_height;
    PixmapPtr screen_pixmap = pScreen->GetScreenPixmap(pScreen);
    rrScrPriv(pScreen);

    PixmapRegionInit(&root_pixmap_region, screen_pixmap);
    RegionInit(&total_region, NULL, 0);

    /* have to iterate all the crtcs of the attached gpu masters
       and all their output slaves */
    for (c = 0; c < pScrPriv->numCrtcs; c++) {
        if (pScrPriv->crtcs[c] == rr_crtc) {
            newbox.x1 = x;
            newbox.x2 = x + w;
            newbox.y1 = y;
            newbox.y2 = y + h;
        } else {
            if (!pScrPriv->crtcs[c]->mode)
                continue;
            newbox.x1 = pScrPriv->crtcs[c]->x;
            newbox.x2 = pScrPriv->crtcs[c]->x + pScrPriv->crtcs[c]->mode->mode.width;
            newbox.y1 = pScrPriv->crtcs[c]->y;
            newbox.y2 = pScrPriv->crtcs[c]->y + pScrPriv->crtcs[c]->mode->mode.height;
        }
        RegionInit(&new_crtc_region, &newbox, 1);
        RegionUnion(&total_region, &total_region, &new_crtc_region);
    }

    xorg_list_for_each_entry(slave, &pScreen->output_slave_list, output_head) {
        rrScrPrivPtr    slave_priv = rrGetScrPriv(slave);
        for (c = 0; c < slave_priv->numCrtcs; c++) {
            if (slave_priv->crtcs[c] == rr_crtc) {
                newbox.x1 = x;
                newbox.x2 = x + w;
                newbox.y1 = y;
                newbox.y2 = y + h;
            }
            else {
                if (!slave_priv->crtcs[c]->mode)
                    continue;
                newbox.x1 = slave_priv->crtcs[c]->x;
                newbox.x2 = slave_priv->crtcs[c]->x + slave_priv->crtcs[c]->mode->mode.width;
                newbox.y1 = slave_priv->crtcs[c]->y;
                newbox.y2 = slave_priv->crtcs[c]->y + slave_priv->crtcs[c]->mode->mode.height;
            }
            RegionInit(&new_crtc_region, &newbox, 1);
            RegionUnion(&total_region, &total_region, &new_crtc_region);
        }
    }

    newsize = RegionExtents(&total_region);
    new_width = newsize->x2 - newsize->x1;
    new_height = newsize->y2 - newsize->y1;

    if (new_width == screen_pixmap->drawable.width &&
        new_height == screen_pixmap->drawable.height) {
        ErrorF("adjust shatters %d %d\n", newsize->x1, newsize->x2);
    } else {
        pScrPriv->rrScreenSetSize(pScreen, new_width, new_height, 0, 0);
    }

    /* set shatters TODO */
    return TRUE;
}

/*
 * Request that the Crtc be reconfigured
 */
Bool
RRCrtcSet(RRCrtcPtr crtc,
          RRModePtr mode,
          int x,
          int y, Rotation rotation, int numOutputs, RROutputPtr * outputs)
{
    ScreenPtr pScreen = crtc->pScreen;
    Bool ret = FALSE;
    Bool recompute = TRUE;
    Bool crtcChanged;
    int  o;

    rrScrPriv(pScreen);

    crtcChanged = FALSE;
    for (o = 0; o < numOutputs; o++) {
        if (outputs[o] && outputs[o]->crtc != crtc) {
            crtcChanged = TRUE;
            break;
        }
    }

    /* See if nothing changed */
    if (crtc->mode == mode &&
        crtc->x == x &&
        crtc->y == y &&
        crtc->rotation == rotation &&
        crtc->numOutputs == numOutputs &&
        !memcmp(crtc->outputs, outputs, numOutputs * sizeof(RROutputPtr)) &&
        !RRCrtcPendingProperties(crtc) && !RRCrtcPendingTransform(crtc) &&
        !crtcChanged) {
        recompute = FALSE;
        ret = TRUE;
    }
    else {
        if (pScreen->isGPU) {
            ScreenPtr master = pScreen->current_master;
            int width = 0, height = 0;

            if (mode) {
                width = mode->mode.width;
                height = mode->mode.height;
            }
            ErrorF("have a master to look out for\n");
            ret = rrCheckPixmapBounding(master, crtc,
                                        x, y, width, height);
            if (!ret)
                return FALSE;

            if (pScreen->current_master) {
                ret = rrCreateSharedPixmap(crtc, width, height, x, y);
                ErrorF("need to create shared pixmap %d", ret);

            }
        }
#if RANDR_12_INTERFACE
        if (pScrPriv->rrCrtcSet) {
            ret = (*pScrPriv->rrCrtcSet) (pScreen, crtc, mode, x, y,
                                          rotation, numOutputs, outputs);
        }
        else
#endif
        {
#if RANDR_10_INTERFACE
            if (pScrPriv->rrSetConfig) {
                RRScreenSize size;
                RRScreenRate rate;

                if (!mode) {
                    RRCrtcNotify(crtc, NULL, x, y, rotation, NULL, 0, NULL);
                    ret = TRUE;
                }
                else {
                    size.width = mode->mode.width;
                    size.height = mode->mode.height;
                    if (outputs[0]->mmWidth && outputs[0]->mmHeight) {
                        size.mmWidth = outputs[0]->mmWidth;
                        size.mmHeight = outputs[0]->mmHeight;
                    }
                    else {
                        size.mmWidth = pScreen->mmWidth;
                        size.mmHeight = pScreen->mmHeight;
                    }
                    size.nRates = 1;
                    rate.rate = RRVerticalRefresh(&mode->mode);
                    size.pRates = &rate;
                    ret =
                        (*pScrPriv->rrSetConfig) (pScreen, rotation, rate.rate,
                                                  &size);
                    /*
                     * Old 1.0 interface tied screen size to mode size
                     */
                    if (ret) {
                        RRCrtcNotify(crtc, mode, x, y, rotation, NULL, 1,
                                     outputs);
                        RRScreenSizeNotify(pScreen);
                    }
                }
            }
#endif
        }
        if (ret) {

            RRTellChanged(pScreen);

            for (o = 0; o < numOutputs; o++)
                RRPostPendingProperties(outputs[o]);
        }
    }

    if (recompute)
        RRComputeContiguity(pScreen);

    return ret;
}

/*
 * Return crtc transform
 */
RRTransformPtr
RRCrtcGetTransform(RRCrtcPtr crtc)
{
    RRTransformPtr transform = &crtc->client_pending_transform;

    if (pixman_transform_is_identity(&transform->transform))
        return NULL;
    return transform;
}

/*
 * Check whether the pending and current transforms are the same
 */
Bool
RRCrtcPendingTransform(RRCrtcPtr crtc)
{
    return memcmp(&crtc->client_current_transform.transform,
                  &crtc->client_pending_transform.transform,
                  sizeof(PictTransform)) != 0;
}

/*
 * Destroy a Crtc at shutdown
 */
void
RRCrtcDestroy(RRCrtcPtr crtc)
{
    FreeResource(crtc->id, 0);
}

static int
RRCrtcDestroyResource(void *value, XID pid)
{
    RRCrtcPtr crtc = (RRCrtcPtr) value;
    ScreenPtr pScreen = crtc->pScreen;

    if (pScreen) {
        rrScrPriv(pScreen);
        int i;

        for (i = 0; i < pScrPriv->numCrtcs; i++) {
            if (pScrPriv->crtcs[i] == crtc) {
                memmove(pScrPriv->crtcs + i, pScrPriv->crtcs + i + 1,
                        (pScrPriv->numCrtcs - (i + 1)) * sizeof(RRCrtcPtr));
                --pScrPriv->numCrtcs;
                break;
            }
        }

        RRResourcesChanged(pScreen);
    }

    if (crtc->scanout_pixmap)
        RRCrtcDetachScanoutPixmap(crtc);
    free(crtc->gammaRed);
    if (crtc->mode)
        RRModeDestroy(crtc->mode);
    free(crtc);
    return 1;
}

/*
 * Request that the Crtc gamma be changed
 */

Bool
RRCrtcGammaSet(RRCrtcPtr crtc, CARD16 *red, CARD16 *green, CARD16 *blue)
{
    Bool ret = TRUE;

#if RANDR_12_INTERFACE
    ScreenPtr pScreen = crtc->pScreen;
#endif

    memcpy(crtc->gammaRed, red, crtc->gammaSize * sizeof(CARD16));
    memcpy(crtc->gammaGreen, green, crtc->gammaSize * sizeof(CARD16));
    memcpy(crtc->gammaBlue, blue, crtc->gammaSize * sizeof(CARD16));
#if RANDR_12_INTERFACE
    if (pScreen) {
        rrScrPriv(pScreen);
        if (pScrPriv->rrCrtcSetGamma)
            ret = (*pScrPriv->rrCrtcSetGamma) (pScreen, crtc);
    }
#endif
    return ret;
}

/*
 * Request current gamma back from the DDX (if possible).
 * This includes gamma size.
 */
Bool
RRCrtcGammaGet(RRCrtcPtr crtc)
{
    Bool ret = TRUE;

#if RANDR_12_INTERFACE
    ScreenPtr pScreen = crtc->pScreen;
#endif

#if RANDR_12_INTERFACE
    if (pScreen) {
        rrScrPriv(pScreen);
        if (pScrPriv->rrCrtcGetGamma)
            ret = (*pScrPriv->rrCrtcGetGamma) (pScreen, crtc);
    }
#endif
    return ret;
}

/*
 * Notify the extension that the Crtc gamma has been changed
 * The driver calls this whenever it has changed the gamma values
 * in the RRCrtcRec
 */

Bool
RRCrtcGammaNotify(RRCrtcPtr crtc)
{
    return TRUE;                /* not much going on here */
}

static void
RRModeGetScanoutSize(RRModePtr mode, PictTransformPtr transform,
                     int *width, int *height)
{
    BoxRec box;

    if (mode == NULL) {
        *width = 0;
        *height = 0;
        return;
    }

    box.x1 = 0;
    box.y1 = 0;
    box.x2 = mode->mode.width;
    box.y2 = mode->mode.height;

    pixman_transform_bounds(transform, &box);
    *width = box.x2 - box.x1;
    *height = box.y2 - box.y1;
}

/**
 * Returns the width/height that the crtc scans out from the framebuffer
 */
void
RRCrtcGetScanoutSize(RRCrtcPtr crtc, int *width, int *height)
{
    RRModeGetScanoutSize(crtc->mode, &crtc->transform, width, height);
}

/*
 * Set the size of the gamma table at server startup time
 */

Bool
RRCrtcGammaSetSize(RRCrtcPtr crtc, int size)
{
    CARD16 *gamma;

    if (size == crtc->gammaSize)
        return TRUE;
    if (size) {
        gamma = malloc(size * 3 * sizeof(CARD16));
        if (!gamma)
            return FALSE;
    }
    else
        gamma = NULL;
    free(crtc->gammaRed);
    crtc->gammaRed = gamma;
    crtc->gammaGreen = gamma + size;
    crtc->gammaBlue = gamma + size * 2;
    crtc->gammaSize = size;
    return TRUE;
}

/*
 * Set the pending CRTC transformation
 */

int
RRCrtcTransformSet(RRCrtcPtr crtc,
                   PictTransformPtr transform,
                   struct pixman_f_transform *f_transform,
                   struct pixman_f_transform *f_inverse,
                   char *filter_name,
                   int filter_len, xFixed * params, int nparams)
{
    PictFilterPtr filter = NULL;
    int width = 0, height = 0;

    if (!crtc->transforms)
        return BadValue;

    if (filter_len) {
        filter = PictureFindFilter(crtc->pScreen, filter_name, filter_len);
        if (!filter)
            return BadName;
        if (filter->ValidateParams) {
            if (!filter->ValidateParams(crtc->pScreen, filter->id,
                                        params, nparams, &width, &height))
                return BadMatch;
        }
        else {
            width = filter->width;
            height = filter->height;
        }
    }
    else {
        if (nparams)
            return BadMatch;
    }
    if (!RRTransformSetFilter(&crtc->client_pending_transform,
                              filter, params, nparams, width, height))
        return BadAlloc;

    crtc->client_pending_transform.transform = *transform;
    crtc->client_pending_transform.f_transform = *f_transform;
    crtc->client_pending_transform.f_inverse = *f_inverse;
    return Success;
}

/*
 * Initialize crtc type
 */
Bool
RRCrtcInit(void)
{
    RRCrtcType = CreateNewResourceType(RRCrtcDestroyResource, "CRTC");
    if (!RRCrtcType)
        return FALSE;

    return TRUE;
}

/*
 * Initialize crtc type error value
 */
void
RRCrtcInitErrorValue(void)
{
    SetResourceTypeErrorValue(RRCrtcType, RRErrorBase + BadRRCrtc);
}

int
ProcRRGetCrtcInfo(ClientPtr client)
{
    REQUEST(xRRGetCrtcInfoReq);
    xRRGetCrtcInfoReply rep;
    RRCrtcPtr crtc;
    CARD8 *extra;
    unsigned long extraLen;
    ScreenPtr pScreen;
    rrScrPrivPtr pScrPriv;
    RRModePtr mode;
    RROutput *outputs;
    RROutput *possible;
    int i, j, k;
    int width, height;
    BoxRec panned_area;

    REQUEST_SIZE_MATCH(xRRGetCrtcInfoReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    /* All crtcs must be associated with screens before client
     * requests are processed
     */
    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);

    mode = crtc->mode;

    rep = (xRRGetCrtcInfoReply) {
        .type = X_Reply,
        .status = RRSetConfigSuccess,
        .sequenceNumber = client->sequence,
        .length = 0,
        .timestamp = pScrPriv->lastSetTime.milliseconds
    };
    if (pScrPriv->rrGetPanning &&
        pScrPriv->rrGetPanning(pScreen, crtc, &panned_area, NULL, NULL) &&
        (panned_area.x2 > panned_area.x1) && (panned_area.y2 > panned_area.y1))
    {
        rep.x = panned_area.x1;
        rep.y = panned_area.y1;
        rep.width = panned_area.x2 - panned_area.x1;
        rep.height = panned_area.y2 - panned_area.y1;
    }
    else {
        RRCrtcGetScanoutSize(crtc, &width, &height);
        rep.x = crtc->x;
        rep.y = crtc->y;
        rep.width = width;
        rep.height = height;
    }
    rep.mode = mode ? mode->mode.id : 0;
    rep.rotation = crtc->rotation;
    rep.rotations = crtc->rotations;
    rep.nOutput = crtc->numOutputs;
    k = 0;
    for (i = 0; i < pScrPriv->numOutputs; i++)
        for (j = 0; j < pScrPriv->outputs[i]->numCrtcs; j++)
            if (pScrPriv->outputs[i]->crtcs[j] == crtc)
                k++;
    rep.nPossibleOutput = k;

    rep.length = rep.nOutput + rep.nPossibleOutput;

    extraLen = rep.length << 2;
    if (extraLen) {
        extra = malloc(extraLen);
        if (!extra)
            return BadAlloc;
    }
    else
        extra = NULL;

    outputs = (RROutput *) extra;
    possible = (RROutput *) (outputs + rep.nOutput);

    for (i = 0; i < crtc->numOutputs; i++) {
        outputs[i] = crtc->outputs[i]->id;
        if (client->swapped)
            swapl(&outputs[i]);
    }
    k = 0;
    for (i = 0; i < pScrPriv->numOutputs; i++)
        for (j = 0; j < pScrPriv->outputs[i]->numCrtcs; j++)
            if (pScrPriv->outputs[i]->crtcs[j] == crtc) {
                possible[k] = pScrPriv->outputs[i]->id;
                if (client->swapped)
                    swapl(&possible[k]);
                k++;
            }

    if (client->swapped) {
        swaps(&rep.sequenceNumber);
        swapl(&rep.length);
        swapl(&rep.timestamp);
        swaps(&rep.x);
        swaps(&rep.y);
        swaps(&rep.width);
        swaps(&rep.height);
        swapl(&rep.mode);
        swaps(&rep.rotation);
        swaps(&rep.rotations);
        swaps(&rep.nOutput);
        swaps(&rep.nPossibleOutput);
    }
    WriteToClient(client, sizeof(xRRGetCrtcInfoReply), &rep);
    if (extraLen) {
        WriteToClient(client, extraLen, extra);
        free(extra);
    }

    return Success;
}

int
ProcRRSetCrtcConfig(ClientPtr client)
{
    REQUEST(xRRSetCrtcConfigReq);
    xRRSetCrtcConfigReply rep;
    ScreenPtr pScreen;
    rrScrPrivPtr pScrPriv;
    RRCrtcPtr crtc;
    RRModePtr mode;
    int numOutputs;
    RROutputPtr *outputs = NULL;
    RROutput *outputIds;
    TimeStamp time;
    Rotation rotation;
    int ret, i, j;
    CARD8 status;

    REQUEST_AT_LEAST_SIZE(xRRSetCrtcConfigReq);
    numOutputs = (stuff->length - bytes_to_int32(SIZEOF(xRRSetCrtcConfigReq)));

    VERIFY_RR_CRTC(stuff->crtc, crtc, DixSetAttrAccess);

    if (stuff->mode == None) {
        mode = NULL;
        if (numOutputs > 0)
            return BadMatch;
    }
    else {
        VERIFY_RR_MODE(stuff->mode, mode, DixSetAttrAccess);
        if (numOutputs == 0)
            return BadMatch;
    }
    if (numOutputs) {
        outputs = malloc(numOutputs * sizeof(RROutputPtr));
        if (!outputs)
            return BadAlloc;
    }
    else
        outputs = NULL;

    outputIds = (RROutput *) (stuff + 1);
    for (i = 0; i < numOutputs; i++) {
        ret = dixLookupResourceByType((void **) (outputs + i), outputIds[i],
                                     RROutputType, client, DixSetAttrAccess);
        if (ret != Success) {
            free(outputs);
            return ret;
        }
        /* validate crtc for this output */
        for (j = 0; j < outputs[i]->numCrtcs; j++)
            if (outputs[i]->crtcs[j] == crtc)
                break;
        if (j == outputs[i]->numCrtcs) {
            free(outputs);
            return BadMatch;
        }
        /* validate mode for this output */
        for (j = 0; j < outputs[i]->numModes + outputs[i]->numUserModes; j++) {
            RRModePtr m = (j < outputs[i]->numModes ?
                           outputs[i]->modes[j] :
                           outputs[i]->userModes[j - outputs[i]->numModes]);
            if (m == mode)
                break;
        }
        if (j == outputs[i]->numModes + outputs[i]->numUserModes) {
            free(outputs);
            return BadMatch;
        }
    }
    /* validate clones */
    for (i = 0; i < numOutputs; i++) {
        for (j = 0; j < numOutputs; j++) {
            int k;

            if (i == j)
                continue;
            for (k = 0; k < outputs[i]->numClones; k++) {
                if (outputs[i]->clones[k] == outputs[j])
                    break;
            }
            if (k == outputs[i]->numClones) {
                free(outputs);
                return BadMatch;
            }
        }
    }

    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);

    time = ClientTimeToServerTime(stuff->timestamp);

    if (!pScrPriv) {
        time = currentTime;
        status = RRSetConfigFailed;
        goto sendReply;
    }

    /*
     * Validate requested rotation
     */
    rotation = (Rotation) stuff->rotation;

    /* test the rotation bits only! */
    switch (rotation & 0xf) {
    case RR_Rotate_0:
    case RR_Rotate_90:
    case RR_Rotate_180:
    case RR_Rotate_270:
        break;
    default:
        /*
         * Invalid rotation
         */
        client->errorValue = stuff->rotation;
        free(outputs);
        return BadValue;
    }

    if (mode) {
        if ((~crtc->rotations) & rotation) {
            /*
             * requested rotation or reflection not supported by screen
             */
            client->errorValue = stuff->rotation;
            free(outputs);
            return BadMatch;
        }

#ifdef RANDR_12_INTERFACE
        /*
         * Check screen size bounds if the DDX provides a 1.2 interface
         * for setting screen size. Else, assume the CrtcSet sets
         * the size along with the mode. If the driver supports transforms,
         * then it must allow crtcs to display a subset of the screen, so
         * only do this check for drivers without transform support.
         */
        if (pScrPriv->rrScreenSetSize && !crtc->transforms) {
            int source_width;
            int source_height;
            PictTransform transform;
            struct pixman_f_transform f_transform, f_inverse;
            int width, height;

            if (pScreen->isGPU) {
                width = pScreen->current_master->width;
                height = pScreen->current_master->height;
            }
            else {
                width = pScreen->width;
                height = pScreen->height;
            }

            RRTransformCompute(stuff->x, stuff->y,
                               mode->mode.width, mode->mode.height,
                               rotation,
                               &crtc->client_pending_transform,
                               &transform, &f_transform, &f_inverse);

            RRModeGetScanoutSize(mode, &transform, &source_width,
                                 &source_height);
            if (stuff->x + source_width > width) {
                client->errorValue = stuff->x;
                free(outputs);
                return BadValue;
            }

            if (stuff->y + source_height > height) {
                client->errorValue = stuff->y;
                free(outputs);
                return BadValue;
            }
        }
#endif
    }

    if (!RRCrtcSet(crtc, mode, stuff->x, stuff->y,
                   rotation, numOutputs, outputs)) {
        status = RRSetConfigFailed;
        goto sendReply;
    }
    status = RRSetConfigSuccess;
    pScrPriv->lastSetTime = time;

 sendReply:
    free(outputs);

    rep = (xRRSetCrtcConfigReply) {
        .type = X_Reply,
        .status = status,
        .sequenceNumber = client->sequence,
        .length = 0,
        .newTimestamp = pScrPriv->lastSetTime.milliseconds
    };

    if (client->swapped) {
        swaps(&rep.sequenceNumber);
        swapl(&rep.length);
        swapl(&rep.newTimestamp);
    }
    WriteToClient(client, sizeof(xRRSetCrtcConfigReply), &rep);

    return Success;
}

int
ProcRRGetPanning(ClientPtr client)
{
    REQUEST(xRRGetPanningReq);
    xRRGetPanningReply rep;
    RRCrtcPtr crtc;
    ScreenPtr pScreen;
    rrScrPrivPtr pScrPriv;
    BoxRec total;
    BoxRec tracking;
    INT16 border[4];

    REQUEST_SIZE_MATCH(xRRGetPanningReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    /* All crtcs must be associated with screens before client
     * requests are processed
     */
    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);

    if (!pScrPriv)
        return RRErrorBase + BadRRCrtc;

    rep = (xRRGetPanningReply) {
        .type = X_Reply,
        .status = RRSetConfigSuccess,
        .sequenceNumber = client->sequence,
        .length = 1,
        .timestamp = pScrPriv->lastSetTime.milliseconds
    };

    if (pScrPriv->rrGetPanning &&
        pScrPriv->rrGetPanning(pScreen, crtc, &total, &tracking, border)) {
        rep.left = total.x1;
        rep.top = total.y1;
        rep.width = total.x2 - total.x1;
        rep.height = total.y2 - total.y1;
        rep.track_left = tracking.x1;
        rep.track_top = tracking.y1;
        rep.track_width = tracking.x2 - tracking.x1;
        rep.track_height = tracking.y2 - tracking.y1;
        rep.border_left = border[0];
        rep.border_top = border[1];
        rep.border_right = border[2];
        rep.border_bottom = border[3];
    }

    if (client->swapped) {
        swaps(&rep.sequenceNumber);
        swapl(&rep.length);
        swapl(&rep.timestamp);
        swaps(&rep.left);
        swaps(&rep.top);
        swaps(&rep.width);
        swaps(&rep.height);
        swaps(&rep.track_left);
        swaps(&rep.track_top);
        swaps(&rep.track_width);
        swaps(&rep.track_height);
        swaps(&rep.border_left);
        swaps(&rep.border_top);
        swaps(&rep.border_right);
        swaps(&rep.border_bottom);
    }
    WriteToClient(client, sizeof(xRRGetPanningReply), &rep);
    return Success;
}

int
ProcRRSetPanning(ClientPtr client)
{
    REQUEST(xRRSetPanningReq);
    xRRSetPanningReply rep;
    RRCrtcPtr crtc;
    ScreenPtr pScreen;
    rrScrPrivPtr pScrPriv;
    TimeStamp time;
    BoxRec total;
    BoxRec tracking;
    INT16 border[4];
    CARD8 status;

    REQUEST_SIZE_MATCH(xRRSetPanningReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    /* All crtcs must be associated with screens before client
     * requests are processed
     */
    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);

    if (!pScrPriv) {
        time = currentTime;
        status = RRSetConfigFailed;
        goto sendReply;
    }

    time = ClientTimeToServerTime(stuff->timestamp);

    if (!pScrPriv->rrGetPanning)
        return RRErrorBase + BadRRCrtc;

    total.x1 = stuff->left;
    total.y1 = stuff->top;
    total.x2 = total.x1 + stuff->width;
    total.y2 = total.y1 + stuff->height;
    tracking.x1 = stuff->track_left;
    tracking.y1 = stuff->track_top;
    tracking.x2 = tracking.x1 + stuff->track_width;
    tracking.y2 = tracking.y1 + stuff->track_height;
    border[0] = stuff->border_left;
    border[1] = stuff->border_top;
    border[2] = stuff->border_right;
    border[3] = stuff->border_bottom;

    if (!pScrPriv->rrSetPanning(pScreen, crtc, &total, &tracking, border))
        return BadMatch;

    pScrPriv->lastSetTime = time;

    status = RRSetConfigSuccess;

 sendReply:
    rep = (xRRSetPanningReply) {
        .type = X_Reply,
        .status = status,
        .sequenceNumber = client->sequence,
        .length = 0,
        .newTimestamp = pScrPriv->lastSetTime.milliseconds
    };

    if (client->swapped) {
        swaps(&rep.sequenceNumber);
        swapl(&rep.length);
        swapl(&rep.newTimestamp);
    }
    WriteToClient(client, sizeof(xRRSetPanningReply), &rep);
    return Success;
}

int
ProcRRGetCrtcGammaSize(ClientPtr client)
{
    REQUEST(xRRGetCrtcGammaSizeReq);
    xRRGetCrtcGammaSizeReply reply;
    RRCrtcPtr crtc;

    REQUEST_SIZE_MATCH(xRRGetCrtcGammaSizeReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    /* Gamma retrieval failed, any better error? */
    if (!RRCrtcGammaGet(crtc))
        return RRErrorBase + BadRRCrtc;

    reply = (xRRGetCrtcGammaSizeReply) {
        .type = X_Reply,
        .sequenceNumber = client->sequence,
        .length = 0,
        .size = crtc->gammaSize
    };
    if (client->swapped) {
        swaps(&reply.sequenceNumber);
        swapl(&reply.length);
        swaps(&reply.size);
    }
    WriteToClient(client, sizeof(xRRGetCrtcGammaSizeReply), &reply);
    return Success;
}

int
ProcRRGetCrtcGamma(ClientPtr client)
{
    REQUEST(xRRGetCrtcGammaReq);
    xRRGetCrtcGammaReply reply;
    RRCrtcPtr crtc;
    unsigned long len;
    char *extra = NULL;

    REQUEST_SIZE_MATCH(xRRGetCrtcGammaReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    /* Gamma retrieval failed, any better error? */
    if (!RRCrtcGammaGet(crtc))
        return RRErrorBase + BadRRCrtc;

    len = crtc->gammaSize * 3 * 2;

    if (crtc->gammaSize) {
        extra = malloc(len);
        if (!extra)
            return BadAlloc;
    }

    reply = (xRRGetCrtcGammaReply) {
        .type = X_Reply,
        .sequenceNumber = client->sequence,
        .length = bytes_to_int32(len),
        .size = crtc->gammaSize
    };
    if (client->swapped) {
        swaps(&reply.sequenceNumber);
        swapl(&reply.length);
        swaps(&reply.size);
    }
    WriteToClient(client, sizeof(xRRGetCrtcGammaReply), &reply);
    if (crtc->gammaSize) {
        memcpy(extra, crtc->gammaRed, len);
        client->pSwapReplyFunc = (ReplySwapPtr) CopySwap16Write;
        WriteSwappedDataToClient(client, len, extra);
        free(extra);
    }
    return Success;
}

int
ProcRRSetCrtcGamma(ClientPtr client)
{
    REQUEST(xRRSetCrtcGammaReq);
    RRCrtcPtr crtc;
    unsigned long len;
    CARD16 *red, *green, *blue;

    REQUEST_AT_LEAST_SIZE(xRRSetCrtcGammaReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    len = client->req_len - bytes_to_int32(sizeof(xRRSetCrtcGammaReq));
    if (len < (stuff->size * 3 + 1) >> 1)
        return BadLength;

    if (stuff->size != crtc->gammaSize)
        return BadMatch;

    red = (CARD16 *) (stuff + 1);
    green = red + crtc->gammaSize;
    blue = green + crtc->gammaSize;

    RRCrtcGammaSet(crtc, red, green, blue);

    return Success;
}

/* Version 1.3 additions */

int
ProcRRSetCrtcTransform(ClientPtr client)
{
    REQUEST(xRRSetCrtcTransformReq);
    RRCrtcPtr crtc;
    PictTransform transform;
    struct pixman_f_transform f_transform, f_inverse;
    char *filter;
    int nbytes;
    xFixed *params;
    int nparams;

    REQUEST_AT_LEAST_SIZE(xRRSetCrtcTransformReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    PictTransform_from_xRenderTransform(&transform, &stuff->transform);
    pixman_f_transform_from_pixman_transform(&f_transform, &transform);
    if (!pixman_f_transform_invert(&f_inverse, &f_transform))
        return BadMatch;

    filter = (char *) (stuff + 1);
    nbytes = stuff->nbytesFilter;
    params = (xFixed *) (filter + pad_to_int32(nbytes));
    nparams = ((xFixed *) stuff + client->req_len) - params;
    if (nparams < 0)
        return BadLength;

    return RRCrtcTransformSet(crtc, &transform, &f_transform, &f_inverse,
                              filter, nbytes, params, nparams);
}

#define CrtcTransformExtra	(SIZEOF(xRRGetCrtcTransformReply) - 32)

static int
transform_filter_length(RRTransformPtr transform)
{
    int nbytes, nparams;

    if (transform->filter == NULL)
        return 0;
    nbytes = strlen(transform->filter->name);
    nparams = transform->nparams;
    return pad_to_int32(nbytes) + (nparams * sizeof(xFixed));
}

static int
transform_filter_encode(ClientPtr client, char *output,
                        CARD16 *nbytesFilter,
                        CARD16 *nparamsFilter, RRTransformPtr transform)
{
    int nbytes, nparams;

    if (transform->filter == NULL) {
        *nbytesFilter = 0;
        *nparamsFilter = 0;
        return 0;
    }
    nbytes = strlen(transform->filter->name);
    nparams = transform->nparams;
    *nbytesFilter = nbytes;
    *nparamsFilter = nparams;
    memcpy(output, transform->filter->name, nbytes);
    while ((nbytes & 3) != 0)
        output[nbytes++] = 0;
    memcpy(output + nbytes, transform->params, nparams * sizeof(xFixed));
    if (client->swapped) {
        swaps(nbytesFilter);
        swaps(nparamsFilter);
        SwapLongs((CARD32 *) (output + nbytes), nparams);
    }
    nbytes += nparams * sizeof(xFixed);
    return nbytes;
}

static void
transform_encode(ClientPtr client, xRenderTransform * wire,
                 PictTransform * pict)
{
    xRenderTransform_from_PictTransform(wire, pict);
    if (client->swapped)
        SwapLongs((CARD32 *) wire, bytes_to_int32(sizeof(xRenderTransform)));
}

int
ProcRRGetCrtcTransform(ClientPtr client)
{
    REQUEST(xRRGetCrtcTransformReq);
    xRRGetCrtcTransformReply *reply;
    RRCrtcPtr crtc;
    int nextra;
    RRTransformPtr current, pending;
    char *extra;

    REQUEST_SIZE_MATCH(xRRGetCrtcTransformReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    pending = &crtc->client_pending_transform;
    current = &crtc->client_current_transform;

    nextra = (transform_filter_length(pending) +
              transform_filter_length(current));

    reply = calloc(1, sizeof(xRRGetCrtcTransformReply) + nextra);
    if (!reply)
        return BadAlloc;

    extra = (char *) (reply + 1);
    reply->type = X_Reply;
    reply->sequenceNumber = client->sequence;
    reply->length = bytes_to_int32(CrtcTransformExtra + nextra);

    reply->hasTransforms = crtc->transforms;

    transform_encode(client, &reply->pendingTransform, &pending->transform);
    extra += transform_filter_encode(client, extra,
                                     &reply->pendingNbytesFilter,
                                     &reply->pendingNparamsFilter, pending);

    transform_encode(client, &reply->currentTransform, &current->transform);
    extra += transform_filter_encode(client, extra,
                                     &reply->currentNbytesFilter,
                                     &reply->currentNparamsFilter, current);

    if (client->swapped) {
        swaps(&reply->sequenceNumber);
        swapl(&reply->length);
    }
    WriteToClient(client, sizeof(xRRGetCrtcTransformReply) + nextra, reply);
    free(reply);
    return Success;
}

static Bool check_all_screen_crtcs(ScreenPtr pScreen, int *x, int *y)
{
    rrScrPriv(pScreen);
    int i;
    for (i = 0; i < pScrPriv->numCrtcs; i++) {
        RRCrtcPtr crtc = pScrPriv->crtcs[i];

        int left, right, top, bottom;

        if (!crtc->mode)
            continue;

        crtc_bounds(crtc, &left, &right, &top, &bottom);

        if ((*x >= left) && (*x < right) && (*y >= top) && (*y < bottom))
            return TRUE;
    }
    return FALSE;
}

static Bool constrain_all_screen_crtcs(DeviceIntPtr pDev, ScreenPtr pScreen, int *x, int *y)
{
    rrScrPriv(pScreen);
    int i;

    /* if we're trying to escape, clamp to the CRTC we're coming from */
    for (i = 0; i < pScrPriv->numCrtcs; i++) {
        RRCrtcPtr crtc = pScrPriv->crtcs[i];
        int nx, ny;
        int left, right, top, bottom;

        if (!crtc->mode)
            continue;

        crtc_bounds(crtc, &left, &right, &top, &bottom);
        miPointerGetPosition(pDev, &nx, &ny);

        if ((nx >= left) && (nx < right) && (ny >= top) && (ny < bottom)) {
            if (*x < left)
                *x = left;
            if (*x >= right)
                *x = right - 1;
            if (*y < top)
                *y = top;
            if (*y >= bottom)
                *y = bottom - 1;

            return TRUE;
        }
    }
    return FALSE;
}

void
RRConstrainCursorHarder(DeviceIntPtr pDev, ScreenPtr pScreen, int mode, int *x,
                        int *y)
{
    rrScrPriv(pScreen);
    Bool ret;
    ScreenPtr slave;

    /* intentional dead space -> let it float */
    if (pScrPriv->discontiguous)
        return;

    /* if we're moving inside a crtc, we're fine */
    ret = check_all_screen_crtcs(pScreen, x, y);
    if (ret == TRUE)
        return;

    xorg_list_for_each_entry(slave, &pScreen->output_slave_list, output_head) {
        ret = check_all_screen_crtcs(slave, x, y);
        if (ret == TRUE)
            return;
    }

    /* if we're trying to escape, clamp to the CRTC we're coming from */
    ret = constrain_all_screen_crtcs(pDev, pScreen, x, y);
    if (ret == TRUE)
        return;

    xorg_list_for_each_entry(slave, &pScreen->output_slave_list, output_head) {
        ret = constrain_all_screen_crtcs(pDev, slave, x, y);
        if (ret == TRUE)
            return;
    }
}

Bool
RRReplaceScanoutPixmap(DrawablePtr pDrawable, PixmapPtr pPixmap, Bool enable)
{
    rrScrPriv(pDrawable->pScreen);
    int i;
    Bool size_fits = FALSE;
    Bool changed = FALSE;
    Bool ret = TRUE;

    for (i = 0; i < pScrPriv->numCrtcs; i++) {
        RRCrtcPtr crtc = pScrPriv->crtcs[i];

        if (!crtc->mode && enable)
            continue;

        changed = FALSE;
        if (crtc->mode && crtc->x == pDrawable->x &&
            crtc->y == pDrawable->y &&
            crtc->mode->mode.width == pDrawable->width &&
            crtc->mode->mode.height == pDrawable->height)
            size_fits = TRUE;

        /* is the pixmap already set? */
        if (crtc->scanout_pixmap == pPixmap) {
            /* if its a disable then don't care about size */
            if (enable == FALSE) {
                /* set scanout to NULL */
                crtc->scanout_pixmap = NULL;
                changed = TRUE;
            } else {
                /* if the size fits then we are already setup */
                if (size_fits)
                    return TRUE;
                /* if the size no longer fits then drop off */
                crtc->scanout_pixmap = NULL;
                changed = TRUE;
                ret = FALSE;
            }
        } else {
            if (!size_fits)
                return FALSE;
            if (enable) {
                crtc->scanout_pixmap = pPixmap;
                pScrPriv->rrCrtcSetScanoutPixmap(crtc, pPixmap);
                changed = TRUE;
            }
        }

        if (changed && pScrPriv->rrCrtcSet) {
            pScrPriv->rrCrtcSetScanoutPixmap(crtc, crtc->scanout_pixmap);

            (*pScrPriv->rrCrtcSet) (pDrawable->pScreen, crtc, crtc->mode, crtc->x, crtc->y,
                                    crtc->rotation, crtc->numOutputs, crtc->outputs);
        }
    }
    return ret;
}
