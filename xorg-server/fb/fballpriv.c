/*
 * Copyright © 1998 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "fb.h"

#ifdef FB_SCREEN_PRIVATE
static int fbScreenPrivateKeyIndex;
static DevPrivateKey fbScreenPrivateKey = &fbScreenPrivateKeyIndex;
DevPrivateKey fbGetScreenPrivateKey(void)
{
    return fbScreenPrivateKey;
}
#endif

static int fbGCPrivateKeyIndex;
static DevPrivateKey fbGCPrivateKey = &fbGCPrivateKeyIndex;
DevPrivateKey fbGetGCPrivateKey(void)
{
    return fbGCPrivateKey;
}

static int fbWinPrivateKeyIndex;
static DevPrivateKey fbWinPrivateKey = &fbWinPrivateKeyIndex;
DevPrivateKey fbGetWinPrivateKey(void)
{
    return fbWinPrivateKey;
}

Bool
fbAllocatePrivates(ScreenPtr pScreen, DevPrivateKey *pGCKey)
{
    if (pGCKey)
	*pGCKey = fbGCPrivateKey;
    
    if (!dixRequestPrivate(fbGCPrivateKey, sizeof(FbGCPrivRec)))
	return FALSE;
#ifdef FB_SCREEN_PRIVATE
    {
	FbScreenPrivPtr	pScreenPriv;

	pScreenPriv = (FbScreenPrivPtr) xalloc (sizeof (FbScreenPrivRec));
	if (!pScreenPriv)
	    return FALSE;
	dixSetPrivate(&pScreen->devPrivates, fbScreenPrivateKey, pScreenPriv);
    }
#endif
    return TRUE;
}

#ifdef FB_ACCESS_WRAPPER
ReadMemoryProcPtr wfbReadMemory;
WriteMemoryProcPtr wfbWriteMemory;
#endif
