/*

Copyright 1996, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/*
 * (c) Copyright 1996 Hewlett-Packard Company
 * (c) Copyright 1996 International Business Machines Corp.
 * (c) Copyright 1996, 2000 Sun Microsystems, Inc.  All Rights Reserved.
 * (c) Copyright 1996 Novell, Inc.
 * (c) Copyright 1996 Digital Equipment Corp.
 * (c) Copyright 1996 Fujitsu Limited
 * (c) Copyright 1996 Hitachi, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the names of the copyright holders
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from said copyright holders.
 */

/*******************************************************************
**
**    *********************************************************
**    *
**    *  File:          PsCache.c
**    *
**    *  Contents:      Character-caching routines
**    *
**    *  Created By:    Jay Hobson (Sun MicroSystems)
**    *
**    *  Copyright:     Copyright 1996 The Open Group, Inc.
**    *
**    *********************************************************
**
********************************************************************/

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "Ps.h"
#include "gcstruct.h"
#include "windowstr.h"
#include <X11/fonts/fntfil.h>
#include <X11/fonts/fntfilst.h>

#define  GET	0
#define  RESET	1

struct bm_cache_list {
	struct bm_cache_list *next;
	struct bm_cache_list *prev;
        int                   height;
	long	              id;
        char                 *pBuffer;
};

struct bm_cache_head {
	struct bm_cache_list *head;
	int		      width;
	struct bm_cache_head *next;
        struct bm_cache_head *prev;
};

static struct bm_cache_head *bm_cache = NULL;

static long
PsBmUniqueId(int func)
{
    static long unique_id = 0;

    if(func == RESET)
    {
	unique_id = 0;
	return 0;
    }
    else
	return ++unique_id;
}

int 
PsBmIsImageCached(
    int gWidth,
    int gHeight,
    char *pBuffer)
{
    int return_val = 0;
    struct bm_cache_head *pList = bm_cache;

    while(pList != NULL && !return_val)
    {
        if(pList->width == gWidth)
        {
	    struct bm_cache_list *pItem = pList->head;

	    while(pItem != NULL)
	    {
	        if(pItem->height == gHeight)
	        {
		    int length = 4*(gWidth/32+(gWidth%32!=0))*gHeight;

	            if(!memcmp(pItem->pBuffer, pBuffer, sizeof(char)*length))
	            {
		        return_val = pItem->id;
		        break;
	            }
	        }
		else if(pItem->height > gHeight)
		    break;

	        pItem = pItem->next;
	    }
        }
	else if(pList->width > gWidth)
	    break;

        pList = pList->next;
    }
    return return_val;
}

int
PsBmPutImageInCache(
    int gWidth,
    int gHeight,
    char *pBuffer)
{
    int return_val = 0;
    struct bm_cache_head *pList = bm_cache;
    struct bm_cache_list *pNew;
    int length = 4*(gWidth/32+(gWidth%32!=0))*gHeight;

    if(gWidth == 1 && gHeight == 1 && pBuffer[0] == 0)
        return return_val;

    pNew = (struct bm_cache_list *)malloc(sizeof(struct bm_cache_list));
    pNew->next    = NULL;
    pNew->prev    = NULL;
    pNew->height  = gHeight;
    pNew->id      = PsBmUniqueId(GET);
    pNew->pBuffer = (char *)malloc(sizeof(char)*length);

    memcpy(pNew->pBuffer, pBuffer, length);

    while(pList != NULL)
    {
        if(pList->width == gWidth)
	{
	    struct bm_cache_list *pItem = pList->head;

	    while(pItem != NULL)
	    {
		if(pItem->height >= gHeight)
		{
		    pNew->next = pItem;
		    pNew->prev = pItem->prev;
		    if(pItem->prev != NULL)
		       pItem->prev->next = pNew;
                    else
		       pList->head = pNew;
                    pItem->prev = pNew;

		    return_val = pNew->id;

		    break;
		}
		else if(pItem->next == NULL)
		{
		    pNew->prev = pItem;
		    pItem->next = pNew;

		    return_val = pNew->id;

		    break;
		}

		pItem = pItem->next;
	    }

	    break;
        }

        pList = pList->next;
    }

    if(pList == NULL)
    {
        struct bm_cache_head *pNewList;

        pNewList = (struct bm_cache_head *)malloc(sizeof(struct bm_cache_head));
 
        pNewList->next  = NULL;
        pNewList->prev  = NULL;
        pNewList->width = gWidth;
        pNewList->head  = pNew;
 
        if(bm_cache == NULL)
        {
	    bm_cache = pNewList;
	    return_val = pNew->id;
        }
        else
        {
 	    pList = bm_cache;

	    while(pList != NULL)
	    {
	        if(pList->width > gWidth)
		{
		    pNewList->next  = pList;
		    pNewList->prev  = pList->prev;

		    if(pList->prev != NULL)
		       pList->prev->next = pNewList;
                    else
		       bm_cache = pNewList;
		    pList->prev = pNewList;

		    return_val = pNew->id;

		    break;
		}
		else if(pList->next == NULL)
                {
		    pNewList->prev  = pList;
		    pList->next = pNewList;

		    return_val = pNew->id;

		    break;
		}

		pList = pList->next;
	    }
        }
    }

    return return_val;
}


static void
PsBmClearImageCacheItem(
    struct bm_cache_list *pItem)
{
    if(pItem != NULL)
    {
	if(pItem->pBuffer != NULL)
	   free(pItem->pBuffer);
        pItem->pBuffer = NULL;

	if(pItem->next)
	   PsBmClearImageCacheItem(pItem->next);
        pItem->next = NULL;

	free(pItem);
	pItem = NULL;
    }
}

static void 
PsBmClearImageCacheList(
    struct bm_cache_head *pList)
{
    if(pList != NULL)
    {
	if(pList->head)
	    PsBmClearImageCacheItem(pList->head);
        pList->head = NULL;

	if(pList->next)
	    PsBmClearImageCacheList(pList->next);
        pList->next = NULL;

	free(pList);
	pList = NULL;
    }
}

void
PsBmClearImageCache(void)
{
   PsBmClearImageCacheList(bm_cache);

   bm_cache = NULL;

   PsBmUniqueId(RESET);
}

