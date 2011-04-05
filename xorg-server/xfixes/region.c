/*
 * Copyright © 2003 Keith Packard
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

#include "xfixesint.h"
#include "scrnintstr.h"
#include <picturestr.h>
extern int RenderErrBase;
#include <regionstr.h>
#include <gcstruct.h>
#include <window.h>

RESTYPE RegionResType;

static int
RegionResFree (pointer data, XID id)
{
    RegionPtr    pRegion = (RegionPtr) data;

    RegionDestroy(pRegion);
    return Success;
}

RegionPtr
XFixesRegionCopy (RegionPtr pRegion)
{
    RegionPtr   pNew = RegionCreate(RegionExtents(pRegion),
				      RegionNumRects(pRegion));
    if (!pNew)
	return 0;
    if (!RegionCopy(pNew, pRegion))
    {
	RegionDestroy(pNew);
	return 0;
    }
    return pNew;
}

Bool
XFixesRegionInit (void)
{
    RegionResType = CreateNewResourceType(RegionResFree, "XFixesRegion");

    return RegionResType != 0;
}

int
ProcXFixesCreateRegion (ClientPtr client)
{
    int		things;
    RegionPtr	pRegion;
    REQUEST (xXFixesCreateRegionReq);
    
    REQUEST_AT_LEAST_SIZE(xXFixesCreateRegionReq);
    LEGAL_NEW_RESOURCE (stuff->region, client);
    
    things = (client->req_len << 2) - sizeof (xXFixesCreateRegionReq);
    if (things & 4)
	return BadLength;
    things >>= 3;

    pRegion = RegionFromRects(things, (xRectangle *) (stuff + 1), CT_UNSORTED);
    if (!pRegion)
	return BadAlloc;
    if (!AddResource (stuff->region, RegionResType, (pointer) pRegion))
	return BadAlloc;
    
    return Success;
}

int
SProcXFixesCreateRegion (ClientPtr client)
{
    register int n;
    REQUEST(xXFixesCreateRegionReq);
    
    swaps(&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xXFixesCreateRegionReq);
    swapl(&stuff->region, n);
    SwapRestS(stuff);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesCreateRegionFromBitmap (ClientPtr client)
{
    RegionPtr	pRegion;
    PixmapPtr	pPixmap;
    int rc;
    REQUEST (xXFixesCreateRegionFromBitmapReq);

    REQUEST_SIZE_MATCH (xXFixesCreateRegionFromBitmapReq);
    LEGAL_NEW_RESOURCE (stuff->region, client);

    rc = dixLookupResourceByType((pointer *)&pPixmap, stuff->bitmap, RT_PIXMAP,
			   client, DixReadAccess);
    if (rc != Success)
    {
	client->errorValue = stuff->bitmap;
	return rc;
    }
    if (pPixmap->drawable.depth != 1)
	return BadMatch;

    pRegion = BitmapToRegion(pPixmap->drawable.pScreen, pPixmap);

    if (!pRegion)
	return BadAlloc;
    
    if (!AddResource (stuff->region, RegionResType, (pointer) pRegion))
	return BadAlloc;
    
    return Success;
}

int
SProcXFixesCreateRegionFromBitmap (ClientPtr client)
{
    int n;
    REQUEST (xXFixesCreateRegionFromBitmapReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesCreateRegionFromBitmapReq);
    swapl(&stuff->region, n);
    swapl(&stuff->bitmap, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesCreateRegionFromWindow (ClientPtr client)
{
    RegionPtr	pRegion;
    Bool	copy = TRUE;
    WindowPtr	pWin;
    int rc;
    REQUEST (xXFixesCreateRegionFromWindowReq);
    
    REQUEST_SIZE_MATCH (xXFixesCreateRegionFromWindowReq);
    LEGAL_NEW_RESOURCE (stuff->region, client);
    rc = dixLookupResourceByType((pointer *)&pWin, stuff->window, RT_WINDOW,
			   client, DixGetAttrAccess);
    if (rc != Success)
    {
	client->errorValue = stuff->window;
	return rc;
    }
    switch (stuff->kind) {
    case WindowRegionBounding:
	pRegion = wBoundingShape(pWin);
	if (!pRegion)
	{
	    pRegion = CreateBoundingShape (pWin);
	    copy = FALSE;
	}
	break;
    case WindowRegionClip:
	pRegion = wClipShape(pWin);
	if (!pRegion)
	{
	    pRegion = CreateClipShape (pWin);
	    copy = FALSE;
	}
	break;
    default:
	client->errorValue = stuff->kind;
	return BadValue;
    }
    if (copy && pRegion)
	pRegion = XFixesRegionCopy (pRegion);
    if (!pRegion)
	return BadAlloc;
    if (!AddResource (stuff->region, RegionResType, (pointer) pRegion))
	return BadAlloc;
    
    return Success;
}

int
SProcXFixesCreateRegionFromWindow (ClientPtr client)
{
    int n;
    REQUEST (xXFixesCreateRegionFromWindowReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesCreateRegionFromWindowReq);
    swapl(&stuff->region, n);
    swapl(&stuff->window, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesCreateRegionFromGC (ClientPtr client)
{
    RegionPtr	pRegion, pClip;
    GCPtr	pGC;
    int 	rc;
    REQUEST (xXFixesCreateRegionFromGCReq);

    REQUEST_SIZE_MATCH (xXFixesCreateRegionFromGCReq);
    LEGAL_NEW_RESOURCE (stuff->region, client);

    rc = dixLookupGC(&pGC, stuff->gc, client, DixGetAttrAccess);
    if (rc != Success)
	return rc;
    
    switch (pGC->clientClipType) {
    case CT_PIXMAP:
	pRegion = BitmapToRegion(pGC->pScreen, (PixmapPtr) pGC->clientClip);
	if (!pRegion)
	    return BadAlloc;
	break;
    case CT_REGION:
	pClip = (RegionPtr) pGC->clientClip;
	pRegion = XFixesRegionCopy (pClip);
	if (!pRegion)
	    return BadAlloc;
	break;
    default:
	return BadImplementation;   /* assume sane server bits */
    }
    
    if (!AddResource (stuff->region, RegionResType, (pointer) pRegion))
	return BadAlloc;
    
    return Success;
}

int
SProcXFixesCreateRegionFromGC (ClientPtr client)
{
    int n;
    REQUEST (xXFixesCreateRegionFromGCReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesCreateRegionFromGCReq);
    swapl(&stuff->region, n);
    swapl(&stuff->gc, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesCreateRegionFromPicture (ClientPtr client)
{
    RegionPtr	pRegion;
    PicturePtr	pPicture;
    REQUEST (xXFixesCreateRegionFromPictureReq);

    REQUEST_SIZE_MATCH (xXFixesCreateRegionFromPictureReq);
    LEGAL_NEW_RESOURCE (stuff->region, client);

    VERIFY_PICTURE(pPicture, stuff->picture, client, DixGetAttrAccess);
    
    switch (pPicture->clientClipType) {
    case CT_PIXMAP:
	pRegion = BitmapToRegion(pPicture->pDrawable->pScreen,
				   (PixmapPtr) pPicture->clientClip);
	if (!pRegion)
	    return BadAlloc;
	break;
    case CT_REGION:
	pRegion = XFixesRegionCopy ((RegionPtr) pPicture->clientClip);
	if (!pRegion)
	    return BadAlloc;
	break;
    default:
	return BadImplementation;   /* assume sane server bits */
    }
    
    if (!AddResource (stuff->region, RegionResType, (pointer) pRegion))
	return BadAlloc;
    
    return Success;
}

int
SProcXFixesCreateRegionFromPicture (ClientPtr client)
{
    int n;
    REQUEST (xXFixesCreateRegionFromPictureReq);

    swaps(&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesCreateRegionFromPictureReq);
    swapl(&stuff->region, n);
    swapl(&stuff->picture, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesDestroyRegion (ClientPtr client)
{
    REQUEST (xXFixesDestroyRegionReq);
    RegionPtr	pRegion;

    REQUEST_SIZE_MATCH(xXFixesDestroyRegionReq);
    VERIFY_REGION(pRegion, stuff->region, client, DixWriteAccess);
    FreeResource (stuff->region, RT_NONE);
    return Success;
}

int
SProcXFixesDestroyRegion (ClientPtr client)
{
    int n;
    REQUEST (xXFixesDestroyRegionReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesDestroyRegionReq);
    swapl (&stuff->region, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesSetRegion (ClientPtr client)
{
    int		things;
    RegionPtr	pRegion, pNew;
    REQUEST (xXFixesSetRegionReq);

    REQUEST_AT_LEAST_SIZE(xXFixesSetRegionReq);
    VERIFY_REGION(pRegion, stuff->region, client, DixWriteAccess);
    
    things = (client->req_len << 2) - sizeof (xXFixesCreateRegionReq);
    if (things & 4)
	return BadLength;
    things >>= 3;

    pNew = RegionFromRects(things, (xRectangle *) (stuff + 1), CT_UNSORTED);
    if (!pNew)
	return BadAlloc;
    if (!RegionCopy(pRegion, pNew))
    {
	RegionDestroy(pNew);
	return BadAlloc;
    }
    RegionDestroy(pNew);
    return Success;
}

int
SProcXFixesSetRegion (ClientPtr client)
{
    int n;
    REQUEST (xXFixesSetRegionReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xXFixesSetRegionReq);
    swapl (&stuff->region, n);
    SwapRestS(stuff);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesCopyRegion (ClientPtr client)
{
    RegionPtr	pSource, pDestination;
    REQUEST (xXFixesCopyRegionReq);
    
    VERIFY_REGION(pSource, stuff->source, client, DixReadAccess);
    VERIFY_REGION(pDestination, stuff->destination, client, DixWriteAccess);
    
    if (!RegionCopy(pDestination, pSource))
	return BadAlloc;

    return Success;
}

int
SProcXFixesCopyRegion (ClientPtr client)
{
    int n;
    REQUEST (xXFixesCopyRegionReq);

    swaps (&stuff->length, n);
    REQUEST_AT_LEAST_SIZE(xXFixesCopyRegionReq);
    swapl (&stuff->source, n);
    swapl (&stuff->destination, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesCombineRegion (ClientPtr client)
{
    RegionPtr	pSource1, pSource2, pDestination;
    REQUEST (xXFixesCombineRegionReq);

    REQUEST_SIZE_MATCH (xXFixesCombineRegionReq);
    VERIFY_REGION(pSource1, stuff->source1, client, DixReadAccess);
    VERIFY_REGION(pSource2, stuff->source2, client, DixReadAccess);
    VERIFY_REGION(pDestination, stuff->destination, client, DixWriteAccess);
    
    switch (stuff->xfixesReqType) {
    case X_XFixesUnionRegion:
	if (!RegionUnion(pDestination, pSource1, pSource2))
	    return BadAlloc;
	break;
    case X_XFixesIntersectRegion:
	if (!RegionIntersect(pDestination, pSource1, pSource2))
	    return BadAlloc;
	break;
    case X_XFixesSubtractRegion:
	if (!RegionSubtract(pDestination, pSource1, pSource2))
	    return BadAlloc;
	break;
    }
    
    return Success;
}

int
SProcXFixesCombineRegion (ClientPtr client)
{
    int n;
    REQUEST (xXFixesCombineRegionReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesCombineRegionReq);
    swapl (&stuff->source1, n);
    swapl (&stuff->source2, n);
    swapl (&stuff->destination, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesInvertRegion (ClientPtr client)
{
    RegionPtr	pSource, pDestination;
    BoxRec	bounds;
    REQUEST(xXFixesInvertRegionReq);

    REQUEST_SIZE_MATCH(xXFixesInvertRegionReq);
    VERIFY_REGION(pSource, stuff->source, client, DixReadAccess);
    VERIFY_REGION(pDestination, stuff->destination, client, DixWriteAccess);

    /* Compute bounds, limit to 16 bits */
    bounds.x1 = stuff->x;
    bounds.y1 = stuff->y;
    if ((int) stuff->x + (int) stuff->width > MAXSHORT)
	bounds.x2 = MAXSHORT;
    else
	bounds.x2 = stuff->x + stuff->width;
    
    if ((int) stuff->y + (int) stuff->height > MAXSHORT)
	bounds.y2 = MAXSHORT;
    else
	bounds.y2 = stuff->y + stuff->height;

    if (!RegionInverse(pDestination, pSource, &bounds))
	return BadAlloc;

    return Success;
}

int
SProcXFixesInvertRegion (ClientPtr client)
{
    int n;
    REQUEST(xXFixesInvertRegionReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesInvertRegionReq);
    swapl (&stuff->source, n);
    swaps (&stuff->x, n);
    swaps (&stuff->y, n);
    swaps (&stuff->width, n);
    swaps (&stuff->height, n);
    swapl (&stuff->destination, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesTranslateRegion (ClientPtr client)
{
    RegionPtr	pRegion;
    REQUEST(xXFixesTranslateRegionReq);

    REQUEST_SIZE_MATCH(xXFixesTranslateRegionReq);
    VERIFY_REGION(pRegion, stuff->region, client, DixWriteAccess);

    RegionTranslate(pRegion, stuff->dx, stuff->dy);
    return Success;
}

int
SProcXFixesTranslateRegion (ClientPtr client)
{
    int n;
    REQUEST(xXFixesTranslateRegionReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesTranslateRegionReq);
    swapl (&stuff->region, n);
    swaps (&stuff->dx, n);
    swaps (&stuff->dy, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesRegionExtents (ClientPtr client)
{
    RegionPtr	pSource, pDestination;
    REQUEST(xXFixesRegionExtentsReq);

    REQUEST_SIZE_MATCH(xXFixesRegionExtentsReq);
    VERIFY_REGION(pSource, stuff->source, client, DixReadAccess);
    VERIFY_REGION(pDestination, stuff->destination, client, DixWriteAccess);

    RegionReset(pDestination, RegionExtents(pSource));

    return Success;
}

int
SProcXFixesRegionExtents (ClientPtr client)
{
    int n;
    REQUEST(xXFixesRegionExtentsReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesRegionExtentsReq);
    swapl (&stuff->source, n);
    swapl (&stuff->destination, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesFetchRegion (ClientPtr client)
{
    RegionPtr		    pRegion;
    xXFixesFetchRegionReply *reply;
    xRectangle		    *pRect;
    BoxPtr		    pExtent;
    BoxPtr		    pBox;
    int			    i, nBox;
    REQUEST(xXFixesFetchRegionReq);

    REQUEST_SIZE_MATCH(xXFixesFetchRegionReq);
    VERIFY_REGION(pRegion, stuff->region, client, DixReadAccess);

    pExtent = RegionExtents(pRegion);
    pBox = RegionRects (pRegion);
    nBox = RegionNumRects (pRegion);
    
    reply = malloc(sizeof (xXFixesFetchRegionReply) +
		    nBox * sizeof (xRectangle));
    if (!reply)
	return BadAlloc;
    reply->type = X_Reply;
    reply->sequenceNumber = client->sequence;
    reply->length = nBox << 1;
    reply->x = pExtent->x1;
    reply->y = pExtent->y1;
    reply->width = pExtent->x2 - pExtent->x1;
    reply->height = pExtent->y2 - pExtent->y1;

    pRect = (xRectangle *) (reply + 1);
    for (i = 0; i < nBox; i++)
    {
	pRect[i].x = pBox[i].x1;
	pRect[i].y = pBox[i].y1;
	pRect[i].width = pBox[i].x2 - pBox[i].x1;
	pRect[i].height = pBox[i].y2 - pBox[i].y1;
    }
    if (client->swapped)
    {
	int n;
	swaps (&reply->sequenceNumber, n);
	swapl (&reply->length, n);
	swaps (&reply->x, n);
	swaps (&reply->y, n);
	swaps (&reply->width, n);
	swaps (&reply->height, n);
	SwapShorts ((INT16 *) pRect, nBox * 4);
    }
    (void) WriteToClient(client, sizeof (xXFixesFetchRegionReply) +
			 nBox * sizeof (xRectangle), (char *) reply);
    free(reply);
    return Success;
}

int
SProcXFixesFetchRegion (ClientPtr client)
{
    int n;
    REQUEST(xXFixesFetchRegionReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesFetchRegionReq);
    swapl (&stuff->region, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesSetGCClipRegion (ClientPtr client)
{
    GCPtr	pGC;
    RegionPtr	pRegion;
    ChangeGCVal	vals[2];
    int		rc;
    REQUEST(xXFixesSetGCClipRegionReq);
    REQUEST_SIZE_MATCH(xXFixesSetGCClipRegionReq);

    rc = dixLookupGC(&pGC, stuff->gc, client, DixSetAttrAccess);
    if (rc != Success)
	return rc;

    VERIFY_REGION_OR_NONE (pRegion, stuff->region, client, DixReadAccess);

    if (pRegion)
    {
	pRegion = XFixesRegionCopy (pRegion);
	if (!pRegion)
	    return BadAlloc;
    }
    
    vals[0].val = stuff->xOrigin;
    vals[1].val = stuff->yOrigin;
    ChangeGC (NullClient, pGC, GCClipXOrigin|GCClipYOrigin, vals);
    (*pGC->funcs->ChangeClip)(pGC, pRegion ? CT_REGION : CT_NONE, (pointer)pRegion, 0);

    return Success;
}

int
SProcXFixesSetGCClipRegion (ClientPtr client)
{
    int n;
    REQUEST(xXFixesSetGCClipRegionReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesSetGCClipRegionReq);
    swapl (&stuff->gc, n);
    swapl (&stuff->region, n);
    swaps (&stuff->xOrigin, n);
    swaps (&stuff->yOrigin, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

typedef	RegionPtr (*CreateDftPtr)(WindowPtr pWin);

int
ProcXFixesSetWindowShapeRegion (ClientPtr client)
{
    WindowPtr	    pWin;
    RegionPtr	    pRegion;
    RegionPtr	    *pDestRegion;
    int rc;
    REQUEST(xXFixesSetWindowShapeRegionReq);

    REQUEST_SIZE_MATCH(xXFixesSetWindowShapeRegionReq);
    rc = dixLookupResourceByType((pointer *)&pWin, stuff->dest, RT_WINDOW,
			   client, DixSetAttrAccess);
    if (rc != Success)
    {
	client->errorValue = stuff->dest;
	return rc;
    }
    VERIFY_REGION_OR_NONE(pRegion, stuff->region, client, DixWriteAccess);
    switch (stuff->destKind) {
    case ShapeBounding:
    case ShapeClip:
    case ShapeInput:
	break;
    default:
	client->errorValue = stuff->destKind;
	return BadValue;
    }
    if (pRegion)
    {
	pRegion = XFixesRegionCopy (pRegion);
	if (!pRegion)
	    return BadAlloc;
	if (!pWin->optional)
	    MakeWindowOptional (pWin);
	switch (stuff->destKind) {
	default:
	case ShapeBounding:
	    pDestRegion = &pWin->optional->boundingShape;
	    break;
	case ShapeClip:
	    pDestRegion = &pWin->optional->clipShape;
	    break;
	case ShapeInput:
	    pDestRegion = &pWin->optional->inputShape;
	    break;
	}
	if (stuff->xOff || stuff->yOff)
	    RegionTranslate(pRegion, stuff->xOff, stuff->yOff);
    }
    else
    {
	if (pWin->optional)
	{
	    switch (stuff->destKind) {
	    default:
	    case ShapeBounding:
		pDestRegion = &pWin->optional->boundingShape;
		break;
	    case ShapeClip:
		pDestRegion = &pWin->optional->clipShape;
		break;
	    case ShapeInput:
		pDestRegion = &pWin->optional->inputShape;
		break;
	    }
	}
	else
	    pDestRegion = &pRegion; /* a NULL region pointer */
    }
    if (*pDestRegion)
	RegionDestroy(*pDestRegion);
    *pDestRegion = pRegion;
    (*pWin->drawable.pScreen->SetShape) (pWin, stuff->destKind);
    SendShapeNotify (pWin, stuff->destKind);
    return Success;
}

int
SProcXFixesSetWindowShapeRegion (ClientPtr client)
{
    int n;
    REQUEST(xXFixesSetWindowShapeRegionReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH(xXFixesSetWindowShapeRegionReq);
    swapl (&stuff->dest, n);
    swaps (&stuff->xOff, n);
    swaps (&stuff->yOff, n);
    swapl (&stuff->region, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesSetPictureClipRegion (ClientPtr client)
{
    PicturePtr		pPicture;
    RegionPtr		pRegion;
    ScreenPtr		pScreen;
    REQUEST(xXFixesSetPictureClipRegionReq);
    
    REQUEST_SIZE_MATCH (xXFixesSetPictureClipRegionReq);
    VERIFY_PICTURE(pPicture, stuff->picture, client, DixSetAttrAccess);
    pScreen = pPicture->pDrawable->pScreen;
    VERIFY_REGION_OR_NONE(pRegion, stuff->region, client, DixReadAccess);
    
    return SetPictureClipRegion (pPicture, stuff->xOrigin, stuff->yOrigin,
				 pRegion);
}

int
SProcXFixesSetPictureClipRegion (ClientPtr client)
{
    int n;
    REQUEST(xXFixesSetPictureClipRegionReq);
    
    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesSetPictureClipRegionReq);
    swapl (&stuff->picture, n);
    swapl (&stuff->region, n);
    swaps (&stuff->xOrigin, n);
    swaps (&stuff->yOrigin, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

int
ProcXFixesExpandRegion (ClientPtr client)
{
    RegionPtr	pSource, pDestination;
    REQUEST (xXFixesExpandRegionReq);
    BoxPtr	pTmp;
    BoxPtr	pSrc;
    int		nBoxes;
    int		i;

    REQUEST_SIZE_MATCH (xXFixesExpandRegionReq);
    VERIFY_REGION(pSource, stuff->source, client, DixReadAccess);
    VERIFY_REGION(pDestination, stuff->destination, client, DixWriteAccess);
    
    nBoxes = RegionNumRects(pSource);
    pSrc = RegionRects(pSource);
    if (nBoxes)
    {
	pTmp = malloc(nBoxes * sizeof (BoxRec));
	if (!pTmp)
	    return BadAlloc;
	for (i = 0; i < nBoxes; i++)
	{
	    pTmp[i].x1 = pSrc[i].x1 - stuff->left;
	    pTmp[i].x2 = pSrc[i].x2 + stuff->right;
	    pTmp[i].y1 = pSrc[i].y1 - stuff->top;
	    pTmp[i].y2 = pSrc[i].y2 + stuff->bottom;
	}
	RegionEmpty(pDestination);
	for (i = 0; i < nBoxes; i++)
	{
	    RegionRec	r;
	    RegionInit(&r, &pTmp[i], 0);
	    RegionUnion(pDestination, pDestination, &r);
	}
	free(pTmp);
    }
    return Success;
}

int
SProcXFixesExpandRegion (ClientPtr client)
{
    int n;
    REQUEST (xXFixesExpandRegionReq);

    swaps (&stuff->length, n);
    REQUEST_SIZE_MATCH (xXFixesExpandRegionReq);
    swapl (&stuff->source, n);
    swapl (&stuff->destination, n);
    swaps (&stuff->left, n);
    swaps (&stuff->right, n);
    swaps (&stuff->top, n);
    swaps (&stuff->bottom, n);
    return (*ProcXFixesVector[stuff->xfixesReqType]) (client);
}

#ifdef PANORAMIX
#include "panoramiX.h"
#include "panoramiXsrv.h"

int
PanoramiXFixesSetGCClipRegion (ClientPtr client)
{
    REQUEST(xXFixesSetGCClipRegionReq);
    int		    result = Success, j;
    PanoramiXRes    *gc;
    REQUEST_SIZE_MATCH(xXFixesSetGCClipRegionReq);

    if ((result = dixLookupResourceByType((void **)&gc, stuff->gc, XRT_GC,
					  client, DixWriteAccess))) {
	client->errorValue = stuff->gc;
	return result;
    }

    FOR_NSCREENS_BACKWARD(j) {
        stuff->gc = gc->info[j].id;
        result = (*PanoramiXSaveXFixesVector[X_XFixesSetGCClipRegion]) (client);
        if(result != Success) break;
    }

    return result;
}

int
PanoramiXFixesSetWindowShapeRegion (ClientPtr client)
{
    int		    result = Success, j;
    PanoramiXRes    *win;
    REQUEST(xXFixesSetWindowShapeRegionReq);

    REQUEST_SIZE_MATCH(xXFixesSetWindowShapeRegionReq);

    if ((result = dixLookupResourceByType((void **)&win, stuff->dest,
					  XRT_WINDOW, client,
					  DixWriteAccess))) {
	client->errorValue = stuff->dest;
	return result;
    }

    FOR_NSCREENS_FORWARD(j) {
	stuff->dest = win->info[j].id;
	result = (*PanoramiXSaveXFixesVector[X_XFixesSetWindowShapeRegion]) (client);
        if(result != Success) break;
    }

    return result;
}

int
PanoramiXFixesSetPictureClipRegion (ClientPtr client)
{
    REQUEST(xXFixesSetPictureClipRegionReq);
    int		    result = Success, j;
    PanoramiXRes    *pict;

    REQUEST_SIZE_MATCH (xXFixesSetPictureClipRegionReq);

    if ((result = dixLookupResourceByType((void **)&pict, stuff->picture,
					  XRT_PICTURE, client,
					  DixWriteAccess))) {
	client->errorValue = stuff->picture;
	return result;
    }

    FOR_NSCREENS_BACKWARD(j) {
        stuff->picture = pict->info[j].id;
	result = (*PanoramiXSaveXFixesVector[X_XFixesSetPictureClipRegion]) (client);
        if(result != Success) break;
    }

    return result;
}

#endif
