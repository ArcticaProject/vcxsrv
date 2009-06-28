/*
 * Copyright © 2003 Anders Carlsson
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Anders Carlsson not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Anders Carlsson makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * ANDERS CARLSSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ANDERS CARLSSON BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include <kdrive-config.h>
#endif
#include "kdrive.h"
#include "kaa.h"

#define DEBUG_OFFSCREEN 0
#if DEBUG_OFFSCREEN
#define DBG_OFFSCREEN(a) ErrorF a
#else
#define DBG_OFFSCREEN(a)
#endif

#if DEBUG_OFFSCREEN
static void
KdOffscreenValidate (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    KdOffscreenArea *prev = 0, *area;

    assert (pScreenPriv->screen->off_screen_areas->area.offset == 0);
    for (area = pScreenPriv->off_screen_areas; area; area = area->next)
    {
	if (prev)
	    assert (prev->offset + prev->size == area->offset);
	    
	prev = area;
    }
    assert (prev->offset + prev->size == pScreenPriv->screen->memory_size);
}
#else
#define KdOffscreenValidate(s)
#endif

static KdOffscreenArea *
KdOffscreenKickOut (ScreenPtr pScreen, KdOffscreenArea *area)
{
    if (area->save)
	(*area->save) (pScreen, area);
    return KdOffscreenFree (pScreen, area);
}

KdOffscreenArea *
KdOffscreenAlloc (ScreenPtr pScreen, int size, int align,
		  Bool locked,
		  KdOffscreenSaveProc save,
		  pointer privData)
{
    KdOffscreenArea *area, *begin, *best;
    KdScreenPriv (pScreen);
    int tmp, real_size = 0, best_score;

    KdOffscreenValidate (pScreen);
    if (!align)
	align = 1;

    if (!size)
    {
	DBG_OFFSCREEN (("Alloc 0x%x -> EMPTY\n", size));
	return NULL;
    }

    /* throw out requests that cannot fit */
    if (size > (pScreenPriv->screen->memory_size - pScreenPriv->screen->off_screen_base))
    {
	DBG_OFFSCREEN (("Alloc 0x%x -> TOBIG\n", size));
	return NULL;
    }
    
    /* Try to find a free space that'll fit. */
    for (area = pScreenPriv->off_screen_areas; area; area = area->next)
    {
	/* skip allocated areas */
	if (area->state != KdOffscreenAvail)
	    continue;

	/* adjust size to match alignment requirement */
	real_size = size;
	tmp = area->offset % align;
	if (tmp)
	    real_size += (align - tmp);
	
	/* does it fit? */
	if (real_size <= area->size)
	    break;
    }
    
    if (!area)
    {
	/* 
	 * Kick out existing users to make space.
	 *
	 * First, locate a region which can hold the desired object.
	 */
	
	/* prev points at the first object to boot */
	best = NULL;
	best_score = MAXINT;
	for (begin = pScreenPriv->off_screen_areas; begin != NULL;
	     begin = begin->next)
	{
	    int avail, score;
	    KdOffscreenArea *scan;

	    if (begin->state == KdOffscreenLocked)
		continue;

	    /* adjust size to match alignment requirement */
	    real_size = size;
	    tmp = begin->offset % align;
	    if (tmp)
		real_size += (align - tmp);
	    
	    avail = 0;
	    score = 0;
	    /* now see if we can make room here, and how "costly" it'll be. */
	    for (scan = begin; scan != NULL; scan = scan->next)
	    {
		if (scan->state == KdOffscreenLocked) {
		    /* Can't make room here, start after this locked area. */
		    begin = scan->next;
		    break;
		}
		/* Score should only be non-zero for KdOffscreenRemovable */
		score += scan->score;
		avail += scan->size;
		if (avail >= real_size)
		    break;
	    }
	    /* Is it the best option we've found so far? */
	    if (avail >= real_size && score < best_score) {
		best = begin;
		best_score = score;
	    }
	}
	area = best;
	if (!area)
	{
	    DBG_OFFSCREEN (("Alloc 0x%x -> NOSPACE\n", size));
	    /* Could not allocate memory */
	    KdOffscreenValidate (pScreen);
	    return NULL;
	}

	/* adjust size to match alignment requirement */
	real_size = size;
	tmp = begin->offset % align;
	if (tmp)
	    real_size += (align - tmp);

	/*
	 * Kick out first area if in use
	 */
	if (area->state != KdOffscreenAvail)
	    area = KdOffscreenKickOut (pScreen, area);
	/*
	 * Now get the system to merge the other needed areas together
	 */
	while (area->size < real_size)
	{
	    assert (area->next && area->next->state == KdOffscreenRemovable);
	    (void) KdOffscreenKickOut (pScreen, area->next);
	}
    }
    
    /* save extra space in new area */
    if (real_size < area->size)
    {
	KdOffscreenArea   *new_area = xalloc (sizeof (KdOffscreenArea));
	if (!new_area)
	    return NULL;
	new_area->offset = area->offset + real_size;
	new_area->size = area->size - real_size;
	new_area->state = KdOffscreenAvail;
	new_area->save = 0;
	new_area->score = 0;
	new_area->next = area->next;
	area->next = new_area;
	area->size = real_size;
    }
    /*
     * Mark this area as in use
     */
    if (locked)
	area->state = KdOffscreenLocked;
    else
	area->state = KdOffscreenRemovable;
    area->privData = privData;
    area->save = save;
    area->score = 0;

    area->save_offset = area->offset;
    {
	int tmp = area->offset % align;
	if (tmp)
	    area->offset += (align - tmp);
    }

    KdOffscreenValidate (pScreen);
    
    DBG_OFFSCREEN (("Alloc 0x%x -> 0x%x\n", size, area->offset));
    return area;
}

void
KdOffscreenSwapOut (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);

    KdOffscreenValidate (pScreen);
    /* loop until a single free area spans the space */
    for (;;)
    {
	KdOffscreenArea *area = pScreenPriv->off_screen_areas;
	
	if (!area)
	    break;
	if (area->state == KdOffscreenAvail)
	{
	    area = area->next;
	    if (!area)
		break;
	}
	assert (area->state != KdOffscreenAvail);
	(void) KdOffscreenKickOut (pScreen, area);
	KdOffscreenValidate (pScreen);
    }    
    KdOffscreenValidate (pScreen);
    KdOffscreenFini (pScreen);
}

void
KdOffscreenSwapIn (ScreenPtr pScreen)
{
    KdOffscreenInit (pScreen);
}

/* merge the next free area into this one */
static void
KdOffscreenMerge (KdOffscreenArea *area)
{
    KdOffscreenArea	*next = area->next;

    /* account for space */
    area->size += next->size;
    /* frob pointer */
    area->next = next->next;
    xfree (next);
}

KdOffscreenArea *
KdOffscreenFree (ScreenPtr pScreen, KdOffscreenArea *area)
{
    KdScreenPriv(pScreen);
    KdOffscreenArea	*next = area->next;
    KdOffscreenArea	*prev;
    
    DBG_OFFSCREEN (("Free 0x%x -> 0x%x\n", area->size, area->offset));
    KdOffscreenValidate (pScreen);

    area->state = KdOffscreenAvail;
    area->save = 0;
    area->offset = area->save_offset;
    area->score = 0;

    /*
     * Find previous area
     */
    if (area == pScreenPriv->off_screen_areas)
	prev = 0;
    else
	for (prev = pScreenPriv->off_screen_areas; prev; prev = prev->next)
	    if (prev->next == area)
		break;
    
    /* link with next area if free */
    if (next && next->state == KdOffscreenAvail)
	KdOffscreenMerge (area);
    
    /* link with prev area if free */
    if (prev && prev->state == KdOffscreenAvail)
    {
	area = prev;
	KdOffscreenMerge (area);
    }

    KdOffscreenValidate (pScreen);
    return area;
}

void
KdOffscreenMarkUsed (PixmapPtr pPixmap)
{
    KaaPixmapPriv (pPixmap);
    KdScreenPriv (pPixmap->drawable.pScreen);
    static int iter = 0;

    if (!pKaaPixmap->area)
	return;

    /* The numbers here are arbitrary.  We may want to tune these. */
    pKaaPixmap->area->score += 100;
    if (++iter == 10) {
	KdOffscreenArea *area;
	for (area = pScreenPriv->off_screen_areas; area != NULL;
	     area = area->next)
	{
	    if (area->state == KdOffscreenRemovable)
		area->score = (area->score * 7) / 8;
	}
    }
}

Bool
KdOffscreenInit (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    KdOffscreenArea *area;

    /* Allocate a big free area */
    area = xalloc (sizeof (KdOffscreenArea));

    if (!area)
	return FALSE;

    area->state = KdOffscreenAvail;
    area->offset = pScreenPriv->screen->off_screen_base;
    area->size = pScreenPriv->screen->memory_size - area->offset;
    area->save = 0;
    area->next = NULL;
    area->score = 0;
    
    /* Add it to the free areas */
    pScreenPriv->off_screen_areas = area;
    
    KdOffscreenValidate (pScreen);

    return TRUE;
}

void
KdOffscreenFini (ScreenPtr pScreen)
{
    KdScreenPriv (pScreen);
    KdOffscreenArea *area;
    
    /* just free all of the area records */
    while ((area = pScreenPriv->off_screen_areas))
    {
	pScreenPriv->off_screen_areas = area->next;
	xfree (area);
    }
}
