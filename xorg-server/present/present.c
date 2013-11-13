/*
 * Copyright © 2013 Keith Packard
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

#ifdef HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#endif

#include "present_priv.h"
#include <gcstruct.h>
#include <misync.h>
#include <misyncstr.h>
#ifdef MONOTONIC_CLOCK
#include <time.h>
#endif

static uint64_t         present_event_id;
static struct xorg_list present_exec_queue;
static struct xorg_list present_flip_queue;

#if 0
#define DebugPresent(x) ErrorF x
#else
#define DebugPresent(x)
#endif

/*
 * Copies the update region from a pixmap to the target drawable
 */
static void
present_copy_region(DrawablePtr drawable,
                    PixmapPtr pixmap,
                    RegionPtr update,
                    int16_t x_off,
                    int16_t y_off)
{
    ScreenPtr   screen = drawable->pScreen;
    GCPtr       gc;

    gc = GetScratchGC(drawable->depth, screen);
    if (update) {
        ChangeGCVal     changes[2];

        changes[0].val = x_off;
        changes[1].val = y_off;
        ChangeGC(serverClient, gc,
                 GCClipXOrigin|GCClipYOrigin,
                 changes);
        (*gc->funcs->ChangeClip)(gc, CT_REGION, update, 0);
    }
    ValidateGC(drawable, gc);
    (*gc->ops->CopyArea)(&pixmap->drawable,
                         drawable,
                         gc,
                         0, 0,
                         pixmap->drawable.width, pixmap->drawable.height,
                         x_off, y_off);
    if (update)
        (*gc->funcs->ChangeClip)(gc, CT_NONE, NULL, 0);
    FreeScratchGC(gc);
}

static Bool
present_check_flip(RRCrtcPtr    crtc,
                   WindowPtr    window,
                   PixmapPtr    pixmap,
                   Bool         sync_flip,
                   RegionPtr    valid,
                   int16_t      x_off,
                   int16_t      y_off)
{
    ScreenPtr                   screen = window->drawable.pScreen;
    WindowPtr                   root = screen->root;
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

    if (!screen_priv)
        return FALSE;

    if (!screen_priv->info)
        return FALSE;

    if (!crtc)
        return FALSE;

    /* Check to see if the driver supports flips at all */
    if (!screen_priv->info->flip)
        return FALSE;

    /* Can't pend a flip while unflipping */
    if (screen_priv->unflip_event_id) {
        return FALSE;
    }

    /* Can't have two pending flips at the same time */
    if (screen_priv->flip_pending) {
        return FALSE;
    }

    /* Make sure the window hasn't been redirected with Composite */
    if (screen->GetWindowPixmap(window) != screen->GetScreenPixmap(screen))
        return FALSE;

    /* Check for full-screen window */
    if (!RegionEqual(&window->clipList, &root->winSize)) {
        return FALSE;
    }

    /* Source pixmap must align with window exactly */
    if (x_off || y_off) {
        return FALSE;
    }

    /* Make sure the area marked as valid fills the screen */
    if (valid && !RegionEqual(valid, &root->winSize)) {
        return FALSE;
    }

    /* Does the window match the pixmap exactly? */
    if (window->drawable.x != 0 || window->drawable.y != 0 ||
#ifdef COMPOSITE
        window->drawable.x != pixmap->screen_x || window->drawable.y != pixmap->screen_y ||
#endif
        window->drawable.width != pixmap->drawable.width ||
        window->drawable.height != pixmap->drawable.height) {
        return FALSE;
    }

    /* Ask the driver for permission */
    if (screen_priv->info->check_flip) {
        if (!(*screen_priv->info->check_flip) (crtc, window, pixmap, sync_flip)) {
            return FALSE;
        }
    }

    return TRUE;
}

static Bool
present_flip(RRCrtcPtr crtc,
             uint64_t event_id,
             uint64_t target_msc,
             PixmapPtr pixmap,
             Bool sync_flip)
{
    ScreenPtr                   screen = crtc->pScreen;
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

    return (*screen_priv->info->flip) (crtc, event_id, target_msc, pixmap, sync_flip);
}

static void
present_vblank_notify(present_vblank_ptr vblank, CARD8 kind, CARD8 mode, uint64_t ust, uint64_t crtc_msc)
{
    int         n;

    present_send_complete_notify(vblank->window, kind, mode, vblank->serial, ust, crtc_msc - vblank->msc_offset);
    for (n = 0; n < vblank->num_notifies; n++) {
        WindowPtr   window = vblank->notifies[n].window;
        CARD32      serial = vblank->notifies[n].serial;

        if (window)
            present_send_complete_notify(window, kind, mode, serial, ust, crtc_msc - vblank->msc_offset);
    }
}

static void
present_pixmap_idle(PixmapPtr pixmap, WindowPtr window, CARD32 serial, struct present_fence *present_fence)
{
    if (present_fence)
        present_fence_set_triggered(present_fence);
    if (window)
        present_send_idle_notify(window, serial, pixmap, present_fence);
}

RRCrtcPtr
present_get_crtc(WindowPtr window)
{
    ScreenPtr                   screen = window->drawable.pScreen;
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

    if (!screen_priv)
        return NULL;

    if (!screen_priv->info)
        return NULL;

    return (*screen_priv->info->get_crtc)(window);
}

uint32_t
present_query_capabilities(RRCrtcPtr crtc)
{
    present_screen_priv_ptr     screen_priv;

    if (!crtc)
        return 0;

    screen_priv = present_screen_priv(crtc->pScreen);

    if (!screen_priv)
        return 0;

    if (!screen_priv->info)
        return 0;

    return screen_priv->info->capabilities;
}

static int
present_get_ust_msc(WindowPtr window, RRCrtcPtr crtc, uint64_t *ust, uint64_t *msc)
{
    ScreenPtr                   screen = window->drawable.pScreen;
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

    if (crtc == NULL)
        return present_fake_get_ust_msc(screen, ust, msc);
    else
        return (*screen_priv->info->get_ust_msc)(crtc, ust, msc);
}

static void
present_flush(WindowPtr window)
{
    ScreenPtr                   screen = window->drawable.pScreen;
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

    if (!screen_priv)
        return;

    if (!screen_priv->info)
        return;

    (*screen_priv->info->flush) (window);
}

static int
present_queue_vblank(ScreenPtr screen,
                     RRCrtcPtr crtc,
                     uint64_t event_id,
                     uint64_t msc)
{
    Bool                        ret;

    if (crtc == NULL)
        ret = present_fake_queue_vblank(screen, event_id, msc);
    else
    {
        present_screen_priv_ptr     screen_priv = present_screen_priv(screen);
        ret = (*screen_priv->info->queue_vblank) (crtc, event_id, msc);
    }
    return ret;
}

static uint64_t
present_window_to_crtc_msc(WindowPtr window, RRCrtcPtr crtc, uint64_t window_msc, uint64_t new_msc)
{
    present_window_priv_ptr     window_priv = present_get_window_priv(window, TRUE);

    if (crtc != window_priv->crtc) {
        uint64_t        old_ust, old_msc;

        /* The old CRTC may have been turned off, in which case
         * we'll just use whatever previous MSC we'd seen from this CRTC
         */

        if (present_get_ust_msc(window, window_priv->crtc, &old_ust, &old_msc) != Success)
            old_msc = window_priv->msc;

        window_priv->msc_offset += new_msc - old_msc;
        window_priv->crtc = crtc;
    }

    return window_msc + window_priv->msc_offset;
}

static void
present_flip_idle(ScreenPtr screen)
{
    present_screen_priv_ptr screen_priv = present_screen_priv(screen);

    if (screen_priv->flip_pixmap) {
        present_pixmap_idle(screen_priv->flip_pixmap, screen_priv->flip_window,
                            screen_priv->flip_serial, screen_priv->flip_idle_fence);
        if (screen_priv->flip_idle_fence)
            present_fence_destroy(screen_priv->flip_idle_fence);
        dixDestroyPixmap(screen_priv->flip_pixmap, screen_priv->flip_pixmap->drawable.id);
        screen_priv->flip_crtc = NULL;
        screen_priv->flip_window = NULL;
        screen_priv->flip_serial = 0;
        screen_priv->flip_pixmap = NULL;
        screen_priv->flip_idle_fence = NULL;
    }
}

static void
present_unflip(ScreenPtr screen)
{
    present_screen_priv_ptr screen_priv = present_screen_priv(screen);

    assert (!screen_priv->unflip_event_id);
    assert (!screen_priv->flip_pending);

    /* Update the screen pixmap with the current flip pixmap contents
     */
    if (screen_priv->flip_pixmap) {
        present_copy_region(&screen->GetScreenPixmap(screen)->drawable,
                            screen_priv->flip_pixmap,
                            NULL, 0, 0);
    }
    screen_priv->unflip_event_id = ++present_event_id;
    DebugPresent(("u %lld\n", screen_priv->unflip_event_id));
    (*screen_priv->info->unflip) (screen, screen_priv->unflip_event_id);
}

static void
present_execute(present_vblank_ptr vblank, uint64_t ust, uint64_t crtc_msc);

static void
present_flip_notify(present_vblank_ptr vblank, uint64_t ust, uint64_t crtc_msc)
{
    WindowPtr                   window = vblank->window;
    ScreenPtr                   screen = window->drawable.pScreen;
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

    DebugPresent(("\tn %p %8lld: %08lx -> %08lx\n", vblank, vblank->target_msc,
           vblank->pixmap ? vblank->pixmap->drawable.id : 0,
                  vblank->window->drawable.id));

    assert (vblank == screen_priv->flip_pending);

    present_flip_idle(screen);

    /* Transfer reference for pixmap and fence from vblank to screen_priv */
    screen_priv->flip_crtc = vblank->crtc;
    screen_priv->flip_window = vblank->window;
    screen_priv->flip_serial = vblank->serial;
    screen_priv->flip_pixmap = vblank->pixmap;
    screen_priv->flip_idle_fence = vblank->idle_fence;

    vblank->pixmap = NULL;
    vblank->idle_fence = NULL;

    screen_priv->flip_pending = NULL;

    if (vblank->abort_flip)
        present_unflip(screen);

    if (!vblank->window_destroyed)
        present_vblank_notify(vblank, PresentCompleteKindPixmap, PresentCompleteModeFlip, ust, crtc_msc);
    present_vblank_destroy(vblank);
}

void
present_event_notify(uint64_t event_id, uint64_t ust, uint64_t msc)
{
    present_vblank_ptr  vblank, tmp;
    int                 s;

    DebugPresent(("\te %lld ust %lld msc %lld\n", event_id, ust, msc));
    xorg_list_for_each_entry_safe(vblank, tmp, &present_exec_queue, event_queue) {
        if (vblank->event_id == event_id) {
            xorg_list_del(&vblank->event_queue);
            present_execute(vblank, ust, msc);
            return;
        }
    }
    xorg_list_for_each_entry_safe(vblank, tmp, &present_flip_queue, event_queue) {
        if (vblank->event_id == event_id) {
            xorg_list_del(&vblank->event_queue);
            present_flip_notify(vblank, ust, msc);
            return;
        }
    }

    for (s = 0; s < screenInfo.numScreens; s++) {
        ScreenPtr               screen = screenInfo.screens[s];
        present_screen_priv_ptr screen_priv = present_screen_priv(screen);

        if (event_id == screen_priv->unflip_event_id) {
            DebugPresent(("\tun %lld\n", event_id));
            screen_priv->unflip_event_id = 0;
            present_flip_idle(screen);
        }
    }
}

/*
 * 'window' is being reconfigured. Check to see if it is involved
 * in flipping and clean up as necessary
 */
void
present_check_flip_window (WindowPtr window)
{
    ScreenPtr                   screen = window->drawable.pScreen;
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);
    present_window_priv_ptr     window_priv = present_window_priv(window);
    present_vblank_ptr          flip_pending = screen_priv->flip_pending;
    present_vblank_ptr          vblank;

    /* If this window hasn't ever been used with Present, it can't be
     * flipping
     */
    if (!window_priv)
        return;

    if (screen_priv->unflip_event_id)
        return;

    if (flip_pending) {
        /*
         * Check pending flip
         */
        if (flip_pending->window == window) {
            if (!present_check_flip(flip_pending->crtc, window, flip_pending->pixmap,
                                    flip_pending->sync_flip, NULL, 0, 0))
                flip_pending->abort_flip = TRUE;
        }
    } else {
        /*
         * Check current flip
         */
        if (window == screen_priv->flip_window) {
            if (!present_check_flip(screen_priv->flip_crtc, window, screen_priv->flip_pixmap, FALSE, NULL, 0, 0))
                present_unflip(screen);
        }
    }

    /* Now check any queued vblanks */
    xorg_list_for_each_entry(vblank, &window_priv->vblank, window_list) {
        if (vblank->flip && !present_check_flip(vblank->crtc, window, vblank->pixmap, FALSE, NULL, 0, 0))
            vblank->flip = FALSE;
    }
}

/*
 * Once the required MSC has been reached, execute the pending request.
 *
 * For requests to actually present something, either blt contents to
 * the screen or queue a frame buffer swap.
 *
 * For requests to just get the current MSC/UST combo, skip that part and
 * go straight to event delivery
 */

static void
present_execute(present_vblank_ptr vblank, uint64_t ust, uint64_t crtc_msc)
{
    WindowPtr                   window = vblank->window;
    present_screen_priv_ptr     screen_priv = present_screen_priv(window->drawable.pScreen);

    if (vblank->wait_fence) {
        /* XXX check fence, queue if not ready */
    }

    xorg_list_del(&vblank->event_queue);
    if (vblank->pixmap) {

        if (vblank->flip && screen_priv->flip_pending == NULL && !screen_priv->unflip_event_id) {

            DebugPresent(("\tf %p %8lld: %08lx -> %08lx\n", vblank, crtc_msc, vblank->pixmap->drawable.id, vblank->window->drawable.id));
            /* Prepare to flip by removing from the window/screen lists
             * and sticking it into the flip_pending field
             */
            screen_priv->flip_pending = vblank;
            xorg_list_del(&vblank->window_list);

            xorg_list_add(&vblank->event_queue, &present_flip_queue);
            /* Try to flip
             */
            if (present_flip(vblank->crtc, vblank->event_id, vblank->target_msc, vblank->pixmap, vblank->sync_flip))
                return;

            xorg_list_del(&vblank->event_queue);
            /* Oops, flip failed. Clear the flip_pending field
              */
            screen_priv->flip_pending = NULL;
            vblank->flip = FALSE;
        }
        DebugPresent(("\tc %p %8lld: %08lx -> %08lx\n", vblank, crtc_msc, vblank->pixmap->drawable.id, vblank->window->drawable.id));
        if (screen_priv->flip_pending) {

            /* Check pending flip
             */
            if (window == screen_priv->flip_pending->window)
                screen_priv->flip_pending->abort_flip = TRUE;
        } else if (!screen_priv->unflip_event_id) {

            /* Check current flip
             */
            if (window == screen_priv->flip_window)
                present_unflip(window->drawable.pScreen);
        }
        present_copy_region(&window->drawable, vblank->pixmap, vblank->update, vblank->x_off, vblank->y_off);

        /* present_copy_region sticks the region into a scratch GC,
         * which is then freed, freeing the region
         */
        vblank->update = NULL;
        present_flush(window);

        present_pixmap_idle(vblank->pixmap, vblank->window, vblank->serial, vblank->idle_fence);
    }
    present_vblank_notify(vblank, vblank->kind, PresentCompleteModeCopy, ust, crtc_msc);
    present_vblank_destroy(vblank);
}

int
present_pixmap(WindowPtr window,
               PixmapPtr pixmap,
               CARD32 serial,
               RegionPtr valid,
               RegionPtr update,
               int16_t x_off,
               int16_t y_off,
               RRCrtcPtr target_crtc,
               SyncFence *wait_fence,
               SyncFence *idle_fence,
               uint32_t options,
               uint64_t window_msc,
               uint64_t divisor,
               uint64_t remainder,
               present_notify_ptr notifies,
               int num_notifies)
{
    uint64_t                    ust;
    uint64_t                    target_msc;
    uint64_t                    crtc_msc;
    int                         ret;
    present_vblank_ptr          vblank;
    ScreenPtr                   screen = window->drawable.pScreen;
    present_window_priv_ptr     window_priv = present_get_window_priv(window, TRUE);
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

    if (!window_priv)
        return BadAlloc;

    if (!target_crtc) {
        /* Update the CRTC if we have a pixmap or we don't have a CRTC
         */
        if (!pixmap)
            target_crtc = window_priv->crtc;

        if (!target_crtc)
            target_crtc = present_get_crtc(window);
    }

    present_get_ust_msc(window, target_crtc, &ust, &crtc_msc);

    target_msc = present_window_to_crtc_msc(window, target_crtc, window_msc, crtc_msc);

    /* Stash the current MSC away in case we need it later
     */
    window_priv->msc = crtc_msc;

    /* Adjust target_msc to match modulus
     */
    if (crtc_msc >= target_msc) {
        if (divisor != 0) {
            target_msc = crtc_msc - (crtc_msc % divisor) + remainder;
            if (target_msc <= crtc_msc)
                target_msc += divisor;
        } else
            target_msc = crtc_msc;
    }

    /*
     * Look for a matching presentation already on the list and
     * don't bother doing the previous one if this one will overwrite it
     * in the same frame
     */

    if (!update) {
        xorg_list_for_each_entry(vblank, &window_priv->vblank, window_list) {

            if (!vblank->pixmap)
                continue;

            if (vblank->crtc != target_crtc || vblank->target_msc != target_msc)
                continue;

            present_pixmap_idle(vblank->pixmap, vblank->window, vblank->serial, vblank->idle_fence);
            present_fence_destroy(vblank->idle_fence);
            dixDestroyPixmap(vblank->pixmap, vblank->pixmap->drawable.id);

            vblank->pixmap = NULL;
            vblank->idle_fence = NULL;
        }
    }

    vblank = calloc (1, sizeof (present_vblank_rec));
    if (!vblank)
        return BadAlloc;

    xorg_list_append(&vblank->window_list, &window_priv->vblank);
    xorg_list_init(&vblank->event_queue);

    vblank->screen = screen;
    vblank->window = window;
    vblank->pixmap = pixmap;
    vblank->event_id = ++present_event_id;
    if (pixmap) {
        vblank->kind = PresentCompleteKindPixmap;
        pixmap->refcnt++;
    } else
        vblank->kind = PresentCompleteKindNotifyMSC;

    vblank->serial = serial;

    if (valid) {
        vblank->valid = RegionDuplicate(valid);
        if (!vblank->valid)
            goto no_mem;
    }
    if (update) {
        vblank->update = RegionDuplicate(update);
        if (!vblank->update)
            goto no_mem;
    }

    vblank->x_off = x_off;
    vblank->y_off = y_off;
    vblank->target_msc = target_msc;
    vblank->crtc = target_crtc;
    vblank->msc_offset = window_priv->msc_offset;
    vblank->notifies = notifies;
    vblank->num_notifies = num_notifies;

    if (!screen_priv->info || !(screen_priv->info->capabilities & PresentCapabilityAsync))
        vblank->sync_flip = TRUE;

    if (pixmap && present_check_flip (target_crtc, window, pixmap, vblank->sync_flip, valid, x_off, y_off)) {
        vblank->flip = TRUE;
        if (vblank->sync_flip)
            target_msc--;
    }

    if (idle_fence) {
        vblank->idle_fence = present_fence_create(idle_fence);
        if (!vblank->idle_fence)
            goto no_mem;
    }

    if (pixmap)
        DebugPresent(("q %lld %p %8lld: %08lx -> %08lx (crtc %p)\n",
                      vblank->event_id, vblank, target_msc,
                      vblank->pixmap->drawable.id, vblank->window->drawable.id,
                      target_crtc));

    xorg_list_add(&vblank->event_queue, &present_exec_queue);
    if (target_msc >= crtc_msc) {
        ret = present_queue_vblank(screen, target_crtc, vblank->event_id, target_msc);
        if (ret != Success) {
            xorg_list_del(&vblank->event_queue);
            goto failure;
        }
    } else
        present_execute(vblank, ust, crtc_msc);

    return Success;

no_mem:
    ret = BadAlloc;
failure:
    vblank->notifies = NULL;
    present_vblank_destroy(vblank);
    return ret;
}

void
present_abort_vblank(ScreenPtr screen, RRCrtcPtr crtc, uint64_t event_id, uint64_t msc)
{
    present_vblank_ptr  vblank, tmp;

    if (crtc == NULL)
        present_fake_abort_vblank(screen, event_id, msc);
    else
    {
        present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

        (*screen_priv->info->abort_vblank) (crtc, event_id, msc);
    }

    xorg_list_for_each_entry_safe(vblank, tmp, &present_exec_queue, event_queue) {
        if (vblank->event_id == event_id) {
            xorg_list_del(&vblank->event_queue);
            return;
        }
    }
    xorg_list_for_each_entry_safe(vblank, tmp, &present_flip_queue, event_queue) {
        if (vblank->event_id == event_id) {
            xorg_list_del(&vblank->event_queue);
            return;
        }
    }
}

int
present_notify_msc(WindowPtr window,
                   CARD32 serial,
                   uint64_t target_msc,
                   uint64_t divisor,
                   uint64_t remainder)
{
    return present_pixmap(window,
                          NULL,
                          serial,
                          NULL, NULL,
                          0, 0,
                          NULL,
                          NULL, NULL,
                          0,
                          target_msc, divisor, remainder, NULL, 0);
}

void
present_flip_destroy(ScreenPtr screen)
{
    present_screen_priv_ptr     screen_priv = present_screen_priv(screen);

    /* XXX this needs to be synchronous for server reset */

    /* Do the actual cleanup once the flip has been performed by the hardware */
    if (screen_priv->flip_pending)
        screen_priv->flip_pending->abort_flip = TRUE;
}

void
present_vblank_destroy(present_vblank_ptr vblank)
{
    /* Remove vblank from window and screen lists */
    xorg_list_del(&vblank->window_list);

    DebugPresent(("\td %p %8lld: %08lx -> %08lx\n", vblank, vblank->target_msc,
                  vblank->pixmap ? vblank->pixmap->drawable.id : 0,
                  vblank->window->drawable.id));

    /* Drop pixmap reference */
    if (vblank->pixmap)
        dixDestroyPixmap(vblank->pixmap, vblank->pixmap->drawable.id);

    /* Free regions */
    if (vblank->valid)
        RegionDestroy(vblank->valid);
    if (vblank->update)
        RegionDestroy(vblank->update);

    if (vblank->idle_fence)
        present_fence_destroy(vblank->idle_fence);

    if (vblank->notifies)
        present_destroy_notifies(vblank->notifies, vblank->num_notifies);

    free(vblank);
}

Bool
present_init(void)
{
    xorg_list_init(&present_exec_queue);
    xorg_list_init(&present_flip_queue);
    present_fake_queue_init();
    return TRUE;
}
