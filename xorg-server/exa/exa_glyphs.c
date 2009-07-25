/*
 * Copyright © 2008 Red Hat, Inc.
 * Partly based on code Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Red Hat DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL Red Hat
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Owen Taylor <otaylor@fishsoup.net>
 * Based on code by: Keith Packard
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <stdlib.h>

#include "exa_priv.h"

#include "mipict.h"

#if DEBUG_GLYPH_CACHE
#define DBG_GLYPH_CACHE(a) ErrorF a
#else
#define DBG_GLYPH_CACHE(a)
#endif

/* Width of the pixmaps we use for the caches; this should be less than
 * max texture size of the driver; this may need to actually come from
 * the driver.
 */
#define CACHE_PICTURE_WIDTH 1024

/* Maximum number of glyphs we buffer on the stack before flushing
 * rendering to the mask or destination surface.
 */
#define GLYPH_BUFFER_SIZE 256

typedef struct {
    PicturePtr source;
    ExaCompositeRectRec rects[GLYPH_BUFFER_SIZE];
    int count;
} ExaGlyphBuffer, *ExaGlyphBufferPtr;

typedef enum {
    ExaGlyphSuccess,    /* Glyph added to render buffer */
    ExaGlyphFail,       /* out of memory, etc */
    ExaGlyphNeedFlush,  /* would evict a glyph already in the buffer */
} ExaGlyphCacheResult;

void
exaGlyphsInit(ScreenPtr pScreen)
{
    ExaScreenPriv(pScreen);
    int i = 0;

    memset(pExaScr->glyphCaches, 0, sizeof(pExaScr->glyphCaches));

    pExaScr->glyphCaches[i].format = PICT_a8;
    pExaScr->glyphCaches[i].glyphWidth = pExaScr->glyphCaches[i].glyphHeight = 16;
    i++;
    pExaScr->glyphCaches[i].format = PICT_a8;
    pExaScr->glyphCaches[i].glyphWidth = pExaScr->glyphCaches[i].glyphHeight = 32;
    i++;
    pExaScr->glyphCaches[i].format = PICT_a8r8g8b8;
    pExaScr->glyphCaches[i].glyphWidth = pExaScr->glyphCaches[i].glyphHeight = 16;
    i++;
    pExaScr->glyphCaches[i].format = PICT_a8r8g8b8;
    pExaScr->glyphCaches[i].glyphWidth = pExaScr->glyphCaches[i].glyphHeight = 32;
    i++;

    assert(i == EXA_NUM_GLYPH_CACHES);
    
    for (i = 0; i < EXA_NUM_GLYPH_CACHES; i++) {
	pExaScr->glyphCaches[i].columns = CACHE_PICTURE_WIDTH / pExaScr->glyphCaches[i].glyphWidth;
	pExaScr->glyphCaches[i].size = 256;
	pExaScr->glyphCaches[i].hashSize = 557;
    }
}

static void
exaUnrealizeGlyphCaches(ScreenPtr    pScreen,
			unsigned int format)
{
    ExaScreenPriv(pScreen);
    int i;

    for (i = 0; i < EXA_NUM_GLYPH_CACHES; i++) {
	ExaGlyphCachePtr cache = &pExaScr->glyphCaches[i];
	
	if (cache->format != format)
	    continue;

	if (cache->picture) {
	    FreePicture ((pointer) cache->picture, (XID) 0);
	    cache->picture = NULL;
	}

	if (cache->hashEntries) {
	    xfree(cache->hashEntries);
	    cache->hashEntries = NULL;
	}
	
	if (cache->glyphs) {
	    xfree(cache->glyphs);
	    cache->glyphs = NULL;
	}
	cache->glyphCount = 0;
    }
}

#define NeedsComponent(f) (PICT_FORMAT_A(f) != 0 && PICT_FORMAT_RGB(f) != 0)

/* All caches for a single format share a single pixmap for glyph storage,
 * allowing mixing glyphs of different sizes without paying a penalty
 * for switching between source pixmaps. (Note that for a size of font
 * right at the border between two sizes, we might be switching for almost
 * every glyph.)
 *
 * This function allocates the storage pixmap, and then fills in the
 * rest of the allocated structures for all caches with the given format.
 */
static Bool
exaRealizeGlyphCaches(ScreenPtr    pScreen,
		      unsigned int format)
{
    ExaScreenPriv(pScreen);

    int depth = PIXMAN_FORMAT_DEPTH(format);
    PictFormatPtr pPictFormat;
    PixmapPtr pPixmap;
    PicturePtr pPicture;
    CARD32 component_alpha;
    int height;
    int i;
    int	error;

    pPictFormat = PictureMatchFormat(pScreen, depth, format);
    if (!pPictFormat)
	return FALSE;
    
    /* Compute the total vertical size needed for the format */

    height = 0;
    for (i = 0; i < EXA_NUM_GLYPH_CACHES; i++) {
	ExaGlyphCachePtr cache = &pExaScr->glyphCaches[i];
	int rows;

	if (cache->format != format)
	    continue;

	cache->yOffset = height;

	rows = (cache->size + cache->columns - 1) / cache->columns;
	height += rows * cache->glyphHeight;
    }

    /* Now allocate the pixmap and picture */
       
    pPixmap = (*pScreen->CreatePixmap) (pScreen,
					CACHE_PICTURE_WIDTH,
					height, depth, 0);
    if (!pPixmap)
	return FALSE;

    component_alpha = NeedsComponent(pPictFormat->format);
    pPicture = CreatePicture(0, &pPixmap->drawable, pPictFormat,
			     CPComponentAlpha, &component_alpha, serverClient,
			     &error);

    (*pScreen->DestroyPixmap) (pPixmap); /* picture holds a refcount */

    if (!pPicture)
	return FALSE;

    /* And store the picture in all the caches for the format */
    
    for (i = 0; i < EXA_NUM_GLYPH_CACHES; i++) {
	ExaGlyphCachePtr cache = &pExaScr->glyphCaches[i];
	int j;

	if (cache->format != format)
	    continue;

	cache->picture = pPicture;
	cache->picture->refcnt++;
	cache->hashEntries = xalloc(sizeof(int) * cache->hashSize);
	cache->glyphs = xalloc(sizeof(ExaCachedGlyphRec) * cache->size);
	cache->glyphCount = 0;

	if (!cache->hashEntries || !cache->glyphs)
	    goto bail;

	for (j = 0; j < cache->hashSize; j++)
	    cache->hashEntries[j] = -1;
	
	cache->evictionPosition = rand() % cache->size;
    }

    /* Each cache references the picture individually */
    FreePicture ((pointer) pPicture, (XID) 0);
    return TRUE;

bail:
    exaUnrealizeGlyphCaches(pScreen, format);
    return FALSE;
}

void
exaGlyphsFini (ScreenPtr pScreen)
{
    ExaScreenPriv(pScreen);
    int i;

    for (i = 0; i < EXA_NUM_GLYPH_CACHES; i++) {
	ExaGlyphCachePtr cache = &pExaScr->glyphCaches[i];

	if (cache->picture)
	    exaUnrealizeGlyphCaches(pScreen, cache->format);
    }
}

static int
exaGlyphCacheHashLookup(ExaGlyphCachePtr cache,
			GlyphPtr         pGlyph)
{
    int slot;

    slot = (*(CARD32 *) pGlyph->sha1) % cache->hashSize;
    
    while (TRUE) { /* hash table can never be full */
	int entryPos = cache->hashEntries[slot];
	if (entryPos == -1)
	    return -1;

	if (memcmp(pGlyph->sha1, cache->glyphs[entryPos].sha1, sizeof(pGlyph->sha1)) == 0){
	    return entryPos;
	}
	    
	slot--;
	if (slot < 0)
	    slot = cache->hashSize - 1;
    }
}

static void
exaGlyphCacheHashInsert(ExaGlyphCachePtr cache,
			GlyphPtr         pGlyph,
			int              pos)
{
    int slot;

    memcpy(cache->glyphs[pos].sha1, pGlyph->sha1, sizeof(pGlyph->sha1));
    
    slot = (*(CARD32 *) pGlyph->sha1) % cache->hashSize;
    
    while (TRUE) { /* hash table can never be full */
	if (cache->hashEntries[slot] == -1) {
	    cache->hashEntries[slot] = pos;
	    return;
	}
	    
	slot--;
	if (slot < 0)
	    slot = cache->hashSize - 1;
    }
}

static void
exaGlyphCacheHashRemove(ExaGlyphCachePtr cache,
			int              pos)
{
    int slot;
    int emptiedSlot = -1;

    slot = (*(CARD32 *) cache->glyphs[pos].sha1) % cache->hashSize;

    while (TRUE) { /* hash table can never be full */
	int entryPos = cache->hashEntries[slot];
	
	if (entryPos == -1)
	    return;

	if (entryPos == pos) {
	    cache->hashEntries[slot] = -1;
	    emptiedSlot = slot;
	} else if (emptiedSlot != -1) {
	    /* See if we can move this entry into the emptied slot, we can't
	     * do that if if entry would have hashed between the current position
	     * and the emptied slot. (taking wrapping into account). Bad positions
	     * are:
	     *
	     * |   XXXXXXXXXX             |
	     *     i         j            
	     *                            
	     * |XXX                   XXXX|
	     *     j                  i
	     *
	     * i - slot, j - emptiedSlot
	     *
	     * (Knuth 6.4R)
	     */
	    
	    int entrySlot = (*(CARD32 *) cache->glyphs[entryPos].sha1) % cache->hashSize;

	    if (!((entrySlot >= slot && entrySlot < emptiedSlot) ||
		  (emptiedSlot < slot && (entrySlot < emptiedSlot || entrySlot >= slot)))) 
	    {
		cache->hashEntries[emptiedSlot] = entryPos;
		cache->hashEntries[slot] = -1;
		emptiedSlot = slot;
	    }
	}
	
	slot--;
	if (slot < 0)
	    slot = cache->hashSize - 1;
    }
}

#define CACHE_X(pos) (((pos) % cache->columns) * cache->glyphWidth)
#define CACHE_Y(pos) (cache->yOffset + ((pos) / cache->columns) * cache->glyphHeight)

/* The most efficient thing to way to upload the glyph to the screen
 * is to use the UploadToScreen() driver hook; this allows us to
 * pipeline glyph uploads and to avoid creating offscreen pixmaps for
 * glyphs that we'll never use again.
 *
 * If we can't do it with UploadToScreen (because the glyph is offscreen, etc),
 * we fall back to CompositePicture.
 *
 * We need to damage the cache pixmap manually in either case because the damage
 * layer unwrapped the picture screen before calling exaGlyphs.
 */
static void
exaGlyphCacheUploadGlyph(ScreenPtr         pScreen,
			 ExaGlyphCachePtr  cache,
			 int               pos,
			 GlyphPtr          pGlyph)
{
    ExaScreenPriv(pScreen);
    PicturePtr pGlyphPicture = GlyphPicture(pGlyph)[pScreen->myNum];
    PixmapPtr pGlyphPixmap = (PixmapPtr)pGlyphPicture->pDrawable;
    ExaPixmapPriv(pGlyphPixmap);
    PixmapPtr pCachePixmap = (PixmapPtr)cache->picture->pDrawable;
    ExaMigrationRec pixmaps[1];

    if (!pExaScr->info->UploadToScreen || pExaScr->swappedOut || pExaPixmap->accel_blocked)
	goto composite;

    /* If the glyph pixmap is already uploaded, no point in doing
     * things this way */
    if (exaPixmapIsOffscreen(pGlyphPixmap))
	goto composite;

    /* UploadToScreen only works if bpp match */
    if (pGlyphPixmap->drawable.bitsPerPixel != pCachePixmap->drawable.bitsPerPixel)
	goto composite;

    /* cache pixmap must be offscreen. */
    pixmaps[0].as_dst = TRUE;
    pixmaps[0].as_src = FALSE;
    pixmaps[0].pPix = pCachePixmap;
    pixmaps[0].pReg = NULL;
    exaDoMigration (pixmaps, 1, TRUE);

    if (!exaPixmapIsOffscreen(pCachePixmap))
	goto composite;

    /* CACHE_{X,Y} are in pixmap coordinates, no need for cache{X,Y}off */
    if (pExaScr->info->UploadToScreen(pCachePixmap,
				      CACHE_X(pos),
				      CACHE_Y(pos),
				      pGlyph->info.width,
				      pGlyph->info.height,
				      (char *)pExaPixmap->sys_ptr,
				      pExaPixmap->sys_pitch))
	goto damage;

composite:
    CompositePicture (PictOpSrc,
		      pGlyphPicture,
		      None,
		      cache->picture,
		      0, 0,
		      0, 0,
		      CACHE_X(pos),
		      CACHE_Y(pos),
		      pGlyph->info.width,
		      pGlyph->info.height);

damage:
    /* The cache pixmap isn't a window, so no need to offset coordinates. */
    exaPixmapDirty (pCachePixmap,
		    CACHE_X(pos),
		    CACHE_Y(pos),
		    CACHE_X(pos) + cache->glyphWidth,
		    CACHE_Y(pos) + cache->glyphHeight);
}

static ExaGlyphCacheResult
exaGlyphCacheBufferGlyph(ScreenPtr         pScreen,
			 ExaGlyphCachePtr  cache,
			 ExaGlyphBufferPtr buffer,
			 GlyphPtr          pGlyph,
			 int               xGlyph,
			 int               yGlyph)
{
    ExaCompositeRectPtr rect;
    int pos;
    
    if (buffer->source && buffer->source != cache->picture)
	return ExaGlyphNeedFlush;

    if (!cache->picture) {
	if (!exaRealizeGlyphCaches(pScreen, cache->format))
	    return ExaGlyphFail;
    }

    DBG_GLYPH_CACHE(("(%d,%d,%s): buffering glyph %lx\n",
		     cache->glyphWidth, cache->glyphHeight, cache->format == PICT_a8 ? "A" : "ARGB",
		     (long)*(CARD32 *) pGlyph->sha1));
   
    pos = exaGlyphCacheHashLookup(cache, pGlyph);
    if (pos != -1) {
	DBG_GLYPH_CACHE(("  found existing glyph at %d\n", pos));
    } else {
	if (cache->glyphCount < cache->size) {
	    /* Space remaining; we fill from the start */
	    pos = cache->glyphCount;
	    cache->glyphCount++;
	    DBG_GLYPH_CACHE(("  storing glyph in free space at %d\n", pos));

	    exaGlyphCacheHashInsert(cache, pGlyph, pos);

	} else {
	    /* Need to evict an entry. We have to see if any glyphs
	     * already in the output buffer were at this position in
	     * the cache
	     */
	    
	    pos = cache->evictionPosition;
	    DBG_GLYPH_CACHE(("  evicting glyph at %d\n", pos));
	    if (buffer->count) {
		int x, y;
		int i;
		
		x = CACHE_X(pos);
		y = CACHE_Y(pos);

		for (i = 0; i < buffer->count; i++) {
		    if (buffer->rects[i].xSrc == x && buffer->rects[i].ySrc == y) {
			DBG_GLYPH_CACHE(("  must flush buffer\n"));
			return ExaGlyphNeedFlush;
		    }
		}
	    }

	    /* OK, we're all set, swap in the new glyph */
	    exaGlyphCacheHashRemove(cache, pos);
	    exaGlyphCacheHashInsert(cache, pGlyph, pos);

	    /* And pick a new eviction position */
	    cache->evictionPosition = rand() % cache->size;
	}

	exaGlyphCacheUploadGlyph(pScreen, cache, pos, pGlyph);
    }

    buffer->source = cache->picture;
	    
    rect = &buffer->rects[buffer->count];
    rect->xSrc = CACHE_X(pos);
    rect->ySrc = CACHE_Y(pos);
    rect->xDst = xGlyph - pGlyph->info.x;
    rect->yDst = yGlyph - pGlyph->info.y;
    rect->width = pGlyph->info.width;
    rect->height = pGlyph->info.height;

    buffer->count++;

    return ExaGlyphSuccess;
}

#undef CACHE_X
#undef CACHE_Y

static ExaGlyphCacheResult
exaBufferGlyph(ScreenPtr         pScreen,
	       ExaGlyphBufferPtr buffer,
	       GlyphPtr          pGlyph,
	       int               xGlyph,
	       int               yGlyph)
{
    ExaScreenPriv(pScreen);
    unsigned int format = (GlyphPicture(pGlyph)[pScreen->myNum])->format;
    int width = pGlyph->info.width;
    int height = pGlyph->info.height;
    ExaCompositeRectPtr rect;
    PicturePtr source;
    int i;

    if (buffer->count == GLYPH_BUFFER_SIZE)
	return ExaGlyphNeedFlush;

    if (PICT_FORMAT_BPP(format) == 1)
	format = PICT_a8;
    
    for (i = 0; i < EXA_NUM_GLYPH_CACHES; i++) {
	ExaGlyphCachePtr cache = &pExaScr->glyphCaches[i];

	if (format == cache->format &&
	    width <= cache->glyphWidth &&
	    height <= cache->glyphHeight) {
	    ExaGlyphCacheResult result = exaGlyphCacheBufferGlyph(pScreen, &pExaScr->glyphCaches[i],
								  buffer,
								  pGlyph, xGlyph, yGlyph);
	    switch (result) {
	    case ExaGlyphFail:
		break;
	    case ExaGlyphSuccess:
	    case ExaGlyphNeedFlush:
		return result;
	    }
	}
    }

    /* Couldn't find the glyph in the cache, use the glyph picture directly */

    source = GlyphPicture(pGlyph)[pScreen->myNum];
    if (buffer->source && buffer->source != source)
	return ExaGlyphNeedFlush;

    buffer->source = source;
    
    rect = &buffer->rects[buffer->count];
    rect->xSrc = 0;
    rect->ySrc = 0;
    rect->xDst = xGlyph - pGlyph->info.x;
    rect->yDst = yGlyph - pGlyph->info.y;
    rect->width = pGlyph->info.width;
    rect->height = pGlyph->info.height;

    buffer->count++;

    return ExaGlyphSuccess;
}

static void
exaGlyphsToMask(PicturePtr        pMask,
		ExaGlyphBufferPtr buffer)
{
    exaCompositeRects(PictOpAdd, buffer->source, pMask,
		      buffer->count, buffer->rects);
    
    buffer->count = 0;
    buffer->source = NULL;
}

static void
exaGlyphsToDst(CARD8		 op,
	       PicturePtr	 pSrc,
	       PicturePtr	 pDst,
	       ExaGlyphBufferPtr buffer,
	       INT16		 xSrc,
	       INT16		 ySrc,
	       INT16		 xDst,
	       INT16		 yDst)
{
    int i;

    for (i = 0; i < buffer->count; i++) {
	ExaCompositeRectPtr rect = &buffer->rects[i];

	CompositePicture (op,
			  pSrc,
			  buffer->source,
			  pDst,
			  xSrc + rect->xDst - xDst,
			  ySrc + rect->yDst - yDst,
			  rect->xSrc,
			  rect->ySrc,
			  rect->xDst,
			  rect->yDst,
			  rect->width,
			  rect->height);
    }
    
    buffer->count = 0;
    buffer->source = NULL;
}

/* Cut and paste from render/glyph.c - probably should export it instead */
static void
GlyphExtents (int		nlist,
	      GlyphListPtr	list,
	      GlyphPtr	       *glyphs,
	      BoxPtr		extents)
{
    int		x1, x2, y1, y2;
    int		n;
    GlyphPtr	glyph;
    int		x, y;
    
    x = 0;
    y = 0;
    extents->x1 = MAXSHORT;
    extents->x2 = MINSHORT;
    extents->y1 = MAXSHORT;
    extents->y2 = MINSHORT;
    while (nlist--)
    {
	x += list->xOff;
	y += list->yOff;
	n = list->len;
	list++;
	while (n--)
	{
	    glyph = *glyphs++;
	    x1 = x - glyph->info.x;
	    if (x1 < MINSHORT)
		x1 = MINSHORT;
	    y1 = y - glyph->info.y;
	    if (y1 < MINSHORT)
		y1 = MINSHORT;
	    x2 = x1 + glyph->info.width;
	    if (x2 > MAXSHORT)
		x2 = MAXSHORT;
	    y2 = y1 + glyph->info.height;
	    if (y2 > MAXSHORT)
		y2 = MAXSHORT;
	    if (x1 < extents->x1)
		extents->x1 = x1;
	    if (x2 > extents->x2)
		extents->x2 = x2;
	    if (y1 < extents->y1)
		extents->y1 = y1;
	    if (y2 > extents->y2)
		extents->y2 = y2;
	    x += glyph->info.xOff;
	    y += glyph->info.yOff;
	}
    }
}

/**
 * Returns TRUE if the glyphs in the lists intersect.  Only checks based on
 * bounding box, which appears to be good enough to catch most cases at least.
 */
static Bool
exaGlyphsIntersect(int nlist, GlyphListPtr list, GlyphPtr *glyphs)
{
    int x1, x2, y1, y2;
    int n;
    GlyphPtr glyph;
    int x, y;
    BoxRec extents;
    Bool first = TRUE;

    x = 0;
    y = 0;
    while (nlist--) {
       x += list->xOff;
       y += list->yOff;
       n = list->len;
       list++;
       while (n--) {
           glyph = *glyphs++;

           if (glyph->info.width == 0 || glyph->info.height == 0) {
               x += glyph->info.xOff;
               y += glyph->info.yOff;
               continue;
           }

           x1 = x - glyph->info.x;
           if (x1 < MINSHORT)
               x1 = MINSHORT;
           y1 = y - glyph->info.y;
           if (y1 < MINSHORT)
               y1 = MINSHORT;
           x2 = x1 + glyph->info.width;
           if (x2 > MAXSHORT)
               x2 = MAXSHORT;
           y2 = y1 + glyph->info.height;
           if (y2 > MAXSHORT)
               y2 = MAXSHORT;

           if (first) {
               extents.x1 = x1;
               extents.y1 = y1;
               extents.x2 = x2;
               extents.y2 = y2;
               first = FALSE;
           } else {
               if (x1 < extents.x2 && x2 > extents.x1 &&
                   y1 < extents.y2 && y2 > extents.y1)
               {
                   return TRUE;
               }

               if (x1 < extents.x1)
                  extents.x1 = x1;
               if (x2 > extents.x2)
                   extents.x2 = x2;
               if (y1 < extents.y1)
                   extents.y1 = y1;
               if (y2 > extents.y2)
                   extents.y2 = y2;
           }
           x += glyph->info.xOff;
           y += glyph->info.yOff;
       }
    }

    return FALSE;
}

void
exaGlyphs (CARD8 	 op,
	   PicturePtr	 pSrc,
	   PicturePtr	 pDst,
	   PictFormatPtr maskFormat,
	   INT16	 xSrc,
	   INT16	 ySrc,
	   int		 nlist,
	   GlyphListPtr	 list,
	   GlyphPtr	*glyphs)
{
    PicturePtr	pPicture;
    PixmapPtr   pMaskPixmap = 0;
    PicturePtr  pMask;
    ScreenPtr   pScreen = pDst->pDrawable->pScreen;
    int		width = 0, height = 0;
    int		x, y;
    int		xDst = list->xOff, yDst = list->yOff;
    int		n;
    GlyphPtr	glyph;
    int		error;
    BoxRec	extents = {0, 0, 0, 0};
    CARD32	component_alpha;
    ExaGlyphBuffer buffer;

    /* If we don't have a mask format but all the glyphs have the same format
     * and don't intersect, use the glyph format as mask format for the full
     * benefits of the glyph cache.
     */
    if (!maskFormat) {
       Bool sameFormat = TRUE;
       int i;

       maskFormat = list[0].format;

       for (i = 0; i < nlist; i++) {
           if (maskFormat->format != list[i].format->format) {
               sameFormat = FALSE;
               break;
           }
       }

       if (!sameFormat || (maskFormat->depth != 1 &&
			   exaGlyphsIntersect(nlist, list, glyphs))) {
	   maskFormat = NULL;
       }
    }

    if (maskFormat)
    {
	GCPtr	    pGC;
	xRectangle  rect;

	GlyphExtents (nlist, list, glyphs, &extents);

	if (extents.x2 <= extents.x1 || extents.y2 <= extents.y1)
	    return;
	width = extents.x2 - extents.x1;
	height = extents.y2 - extents.y1;

	if (maskFormat->depth == 1) {
	    PictFormatPtr a8Format = PictureMatchFormat (pScreen, 8, PICT_a8);

	    if (a8Format)
		maskFormat = a8Format;
	}

	pMaskPixmap = (*pScreen->CreatePixmap) (pScreen, width, height,
						maskFormat->depth,
						CREATE_PIXMAP_USAGE_SCRATCH);
	if (!pMaskPixmap)
	    return;
	component_alpha = NeedsComponent(maskFormat->format);
	pMask = CreatePicture (0, &pMaskPixmap->drawable,
			       maskFormat, CPComponentAlpha, &component_alpha,
			       serverClient, &error);
	if (!pMask)
	{
	    (*pScreen->DestroyPixmap) (pMaskPixmap);
	    return;
	}
	pGC = GetScratchGC (pMaskPixmap->drawable.depth, pScreen);
	ValidateGC (&pMaskPixmap->drawable, pGC);
	rect.x = 0;
	rect.y = 0;
	rect.width = width;
	rect.height = height;
	(*pGC->ops->PolyFillRect) (&pMaskPixmap->drawable, pGC, 1, &rect);
	FreeScratchGC (pGC);
	x = -extents.x1;
	y = -extents.y1;
    }
    else
    {
	pMask = pDst;
	x = 0;
	y = 0;
    }
    buffer.count = 0;
    buffer.source = NULL;
    while (nlist--)
    {
	x += list->xOff;
	y += list->yOff;
	n = list->len;
	while (n--)
	{
	    glyph = *glyphs++;
	    pPicture = GlyphPicture (glyph)[pScreen->myNum];

	    if (glyph->info.width > 0 && glyph->info.height > 0 &&
		exaBufferGlyph(pScreen, &buffer, glyph, x, y) == ExaGlyphNeedFlush)
	    {
		if (maskFormat)
		    exaGlyphsToMask(pMask, &buffer);
		else
		    exaGlyphsToDst(op, pSrc, pDst, &buffer,
				   xSrc, ySrc, xDst, yDst);

		exaBufferGlyph(pScreen, &buffer, glyph, x, y);
	    }

	    x += glyph->info.xOff;
	    y += glyph->info.yOff;
	}
	list++;
    }
    
    if (buffer.count) {
        if (maskFormat)
	    exaGlyphsToMask(pMask, &buffer);
        else
	    exaGlyphsToDst(op, pSrc, pDst, &buffer,
		           xSrc, ySrc, xDst, yDst);
    }

    if (maskFormat)
    {
	x = extents.x1;
	y = extents.y1;
	CompositePicture (op,
			  pSrc,
			  pMask,
			  pDst,
			  xSrc + x - xDst,
			  ySrc + y - yDst,
			  0, 0,
			  x, y,
			  width, height);
	FreePicture ((pointer) pMask, (XID) 0);
	(*pScreen->DestroyPixmap) (pMaskPixmap);
    }
}
