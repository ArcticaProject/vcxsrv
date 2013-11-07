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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "scrnintstr.h"
#include "misync.h"
#include "misyncstr.h"
#include "misyncshm.h"
#include "pixmapstr.h"
#include <sys/mman.h>
#include <unistd.h>
#include <X11/xshmfence.h>

static DevPrivateKeyRec syncShmFencePrivateKey;

typedef struct _SyncShmFencePrivate {
    int32_t             *fence;
    int                 fd;
} SyncShmFencePrivateRec, *SyncShmFencePrivatePtr;

#define SYNC_FENCE_PRIV(pFence) \
    (SyncShmFencePrivatePtr) dixLookupPrivate(&pFence->devPrivates, &syncShmFencePrivateKey)

static void
miSyncShmFenceSetTriggered(SyncFence * pFence)
{
    SyncShmFencePrivatePtr      pPriv = SYNC_FENCE_PRIV(pFence);

    if (pPriv->fence)
        xshmfence_trigger(pPriv->fence);
    miSyncFenceSetTriggered(pFence);
}

static void
miSyncShmFenceReset(SyncFence * pFence)
{
    SyncShmFencePrivatePtr      pPriv = SYNC_FENCE_PRIV(pFence);

    if (pPriv->fence)
        xshmfence_reset(pPriv->fence);
    miSyncFenceReset(pFence);
}

static Bool
miSyncShmFenceCheckTriggered(SyncFence * pFence)
{
    SyncShmFencePrivatePtr      pPriv = SYNC_FENCE_PRIV(pFence);

    if (pPriv->fence)
        return xshmfence_query(pPriv->fence);
    else
        return miSyncFenceCheckTriggered(pFence);
}

static void
miSyncShmFenceAddTrigger(SyncTrigger * pTrigger)
{
    miSyncFenceAddTrigger(pTrigger);
}

static void
miSyncShmFenceDeleteTrigger(SyncTrigger * pTrigger)
{
    miSyncFenceDeleteTrigger(pTrigger);
}

static const SyncFenceFuncsRec miSyncShmFenceFuncs = {
    &miSyncShmFenceSetTriggered,
    &miSyncShmFenceReset,
    &miSyncShmFenceCheckTriggered,
    &miSyncShmFenceAddTrigger,
    &miSyncShmFenceDeleteTrigger
};

static void
miSyncShmScreenCreateFence(ScreenPtr pScreen, SyncFence * pFence,
                        Bool initially_triggered)
{
    SyncShmFencePrivatePtr      pPriv = SYNC_FENCE_PRIV(pFence);

    pPriv->fence = NULL;
    miSyncScreenCreateFence(pScreen, pFence, initially_triggered);
    pFence->funcs = miSyncShmFenceFuncs;
}

static void
miSyncShmScreenDestroyFence(ScreenPtr pScreen, SyncFence * pFence)
{
    SyncShmFencePrivatePtr      pPriv = SYNC_FENCE_PRIV(pFence);

    if (pPriv->fence) {
        xshmfence_trigger(pPriv->fence);
        xshmfence_unmap_shm(pPriv->fence);
        close(pPriv->fd);
    }
    miSyncScreenDestroyFence(pScreen, pFence);
}

int
miSyncInitFenceFromFD(DrawablePtr pDraw, SyncFence *pFence, int fd, BOOL initially_triggered)

{
    SyncShmFencePrivatePtr      pPriv = SYNC_FENCE_PRIV(pFence);

    miSyncInitFence(pDraw->pScreen, pFence, initially_triggered);

    pPriv->fence = xshmfence_map_shm(fd);
    if (pPriv->fence) {
        pPriv->fd = fd;
        return Success;
    }
    else
        close(fd);
    return BadValue;
}

int
miSyncFDFromFence(DrawablePtr pDraw, SyncFence *pFence)
{
    SyncShmFencePrivatePtr      pPriv = SYNC_FENCE_PRIV(pFence);

    if (!pPriv->fence) {
        pPriv->fd = xshmfence_alloc_shm();
        if (pPriv->fd < 0)
            return -1;
        pPriv->fence = xshmfence_map_shm(pPriv->fd);
        if (!pPriv->fence) {
            close (pPriv->fd);
            return -1;
        }
    }
    return pPriv->fd;
}

_X_EXPORT Bool miSyncShmScreenInit(ScreenPtr pScreen)
{
    SyncScreenFuncsPtr  funcs;

    if (!miSyncSetup(pScreen))
        return FALSE;

    if (!dixPrivateKeyRegistered(&syncShmFencePrivateKey)) {
        if (!dixRegisterPrivateKey(&syncShmFencePrivateKey, PRIVATE_SYNC_FENCE,
                                   sizeof(SyncShmFencePrivateRec)))
            return FALSE;
    }

    funcs = miSyncGetScreenFuncs(pScreen);

    funcs->CreateFence = miSyncShmScreenCreateFence;
    funcs->DestroyFence = miSyncShmScreenDestroyFence;
    return TRUE;
}

