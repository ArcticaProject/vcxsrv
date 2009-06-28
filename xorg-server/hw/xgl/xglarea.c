/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "xgl.h"

static Bool
xglAreaMoveIn (xglAreaPtr pArea,
	       pointer	  closure)
{
    pArea->closure = closure;
    pArea->state   = xglAreaOccupied;

    return (*pArea->pRoot->funcs->MoveIn) (pArea, closure);
}

static void
xglAreaMoveOut (xglAreaPtr pArea)
{
    (*pArea->pRoot->funcs->MoveOut) (pArea, pArea->closure);

    pArea->closure = (pointer) 0;
    pArea->state   = xglAreaAvailable;
}

static xglAreaPtr
xglAreaCreate (xglRootAreaPtr pRoot,
	       int	      level,
	       int	      x,
	       int	      y,
	       int	      width,
	       int	      height)
{
    xglAreaPtr pArea;
    int	       n = 4;

    pArea = xalloc (sizeof (xglAreaRec) + pRoot->devPrivateSize);
    if (!pArea)
	return NULL;

    pArea->level   = level;
    pArea->x	   = x;
    pArea->y	   = y;
    pArea->width   = width;
    pArea->height  = height;
    pArea->pRoot   = pRoot;
    pArea->closure = (pointer) 0;
    pArea->state   = xglAreaAvailable;

    while (n--)
	pArea->pArea[n] = NULL;

    if (pRoot->devPrivateSize)
	pArea->devPrivate.ptr = pArea + 1;
    else
	pArea->devPrivate.ptr = (pointer) 0;

    if (!(*pArea->pRoot->funcs->Create) (pArea))
    {
	free (pArea);
	return NULL;
    }

    return pArea;
}

static void
xglAreaDestroy (xglAreaPtr pArea)
{
    if (!pArea)
	return;

    if (pArea->state == xglAreaOccupied)
    {
	xglAreaMoveOut (pArea);
    }
    else
    {
	int n = 4;

	while (n--)
	    xglAreaDestroy (pArea->pArea[n]);
    }

    xfree (pArea);
}

static xglAreaPtr
xglAreaGetTopScoredSubArea (xglAreaPtr pArea)
{
    if (!pArea)
	return NULL;

    switch (pArea->state) {
    case xglAreaOccupied:
	return pArea;
    case xglAreaAvailable:
	break;
    case xglAreaDivided: {
	xglAreaPtr tmp, top = NULL;
	int	   i;

	for (i = 0; i < 4; i++)
	{
	    tmp = xglAreaGetTopScoredSubArea (pArea->pArea[i]);
	    if (tmp && top)
	    {
		if ((*pArea->pRoot->funcs->CompareScore) (tmp,
							  tmp->closure,
							  top->closure) > 0)
		    top = tmp;
	    }
	    else if (tmp)
	    {
		top = tmp;
	    }
	}
	return top;
    }
    }

    return NULL;
}

static Bool
xglAreaFind (xglAreaPtr pArea,
	     int	width,
	     int	height,
	     Bool	kickOut,
	     pointer	closure)
{
    if (pArea->width < width || pArea->height < height)
	return FALSE;

    switch (pArea->state) {
    case xglAreaOccupied:
	if (kickOut)
	{
	    if ((*pArea->pRoot->funcs->CompareScore) (pArea,
						      pArea->closure,
						      closure) >= 0)
		return FALSE;

	    xglAreaMoveOut (pArea);
	} else
	    return FALSE;

    /* fall-through */
    case xglAreaAvailable:
    {
	if (pArea->level == pArea->pRoot->maxLevel ||
	    (pArea->width == width && pArea->height == height))
	{
	    if (xglAreaMoveIn (pArea, closure))
		return TRUE;
	}
	else
	{
	    int dx[4], dy[4], w[4], h[4], i;

	    dx[0] = dx[2] = dy[0] = dy[1] = 0;

	    w[0] = w[2] = dx[1] = dx[3] = width;
	    h[0] = h[1] = dy[2] = dy[3] = height;

	    w[1] = w[3] = pArea->width - width;
	    h[2] = h[3] = pArea->height - height;

	    for (i = 0; i < 2; i++)
	    {
		if (w[i])
		    pArea->pArea[i] =
			xglAreaCreate (pArea->pRoot,
				       pArea->level + 1,
				       pArea->x + dx[i],
				       pArea->y + dy[i],
				       w[i], h[i]);
	    }

	    for (; i < 4; i++)
	    {
		if (w[i] && h[i])
		    pArea->pArea[i] =
			xglAreaCreate (pArea->pRoot,
				       pArea->level + 1,
				       pArea->x + dx[i],
				       pArea->y + dy[i],
				       w[i], h[i]);
	    }

	    pArea->state = xglAreaDivided;

	    if (xglAreaFind (pArea->pArea[0], width, height, kickOut, closure))
		return TRUE;
	}
    } break;
    case xglAreaDivided:
    {
	xglAreaPtr topArea;
	int	   i, rejected = FALSE;

	for (i = 0; i < 4; i++)
	{
	    if (pArea->pArea[i])
	    {
		if (pArea->pArea[i]->width >= width &&
		    pArea->pArea[i]->height >= height)
		{
		    if (xglFindArea (pArea->pArea[i], width, height, kickOut,
				     closure))
			return TRUE;

		    rejected = TRUE;
		}
	    }
	}

	if (rejected)
	    return FALSE;

	topArea = xglAreaGetTopScoredSubArea (pArea);
	if (topArea)
	{
	    if (kickOut)
	    {
		if ((*pArea->pRoot->funcs->CompareScore) (topArea,
							  topArea->closure,
							  closure) >= 0)
		    return FALSE;
	    } else
		return FALSE;
	}

	for (i = 0; i < 4; i++)
	{
	    xglAreaDestroy (pArea->pArea[i]);
	    pArea->pArea[i] = NULL;
	}

	pArea->closure = (pointer) 0;
	pArea->state   = xglAreaAvailable;
	if (xglFindArea (pArea, width, height, TRUE, closure))
	    return TRUE;

    } break;
    }

    return FALSE;
}

Bool
xglRootAreaInit (xglRootAreaPtr	    pRoot,
		 int		    maxLevel,
		 int		    width,
		 int		    height,
		 int		    devPrivateSize,
		 xglAreaFuncsPtr    funcs,
		 pointer	    closure)
{
    pRoot->maxLevel	  = maxLevel;
    pRoot->funcs	  = funcs;
    pRoot->devPrivateSize = devPrivateSize;
    pRoot->closure	  = closure;

    pRoot->pArea = xglAreaCreate (pRoot, 0, 0, 0, width, height);
    if (!pRoot->pArea)
	return FALSE;

    return TRUE;
}

void
xglRootAreaFini (xglRootAreaPtr pRoot)
{
    xglAreaDestroy (pRoot->pArea);
}

void
xglLeaveArea (xglAreaPtr pArea)
{
    xglAreaMoveOut (pArea);
}

void
xglWithdrawArea (xglAreaPtr pArea)
{
    pArea->closure = NULL;
    pArea->state   = xglAreaAvailable;
}

Bool
xglFindArea (xglAreaPtr pArea,
	     int	width,
	     int	height,
	     Bool	kickOut,
	     pointer	closure)
{
    if (width < 1 || height < 0)
	return FALSE;

    return xglAreaFind (pArea, width, height, kickOut, closure);
}
