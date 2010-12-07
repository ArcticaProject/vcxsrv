/*
 * Copyright © 2010 Keith Packard
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

void
RRCrtcSpriteTransformSet(RRCrtcPtr crtc,
			 PictTransform *position_transform,
			 PictTransform *image_transform,
			 struct pict_f_transform *f_position_transform,
			 struct pict_f_transform *f_image_transform)
{
    ScreenPtr			pScreen;
    rrScrPrivPtr		pScrPriv;

    pScreen = crtc->pScreen;
    pScrPriv = rrGetScrPriv(pScreen);
    crtc->client_sprite_position_transform = *position_transform;
    crtc->client_sprite_image_transform = *image_transform;
    crtc->client_sprite_f_position_transform = *f_position_transform;
    crtc->client_sprite_f_image_transform = *f_image_transform;
    if (pScrPriv->rrSetCrtcSpriteTransform)
	(*pScrPriv->rrSetCrtcSpriteTransform) (pScreen, crtc,
					       &crtc->client_sprite_f_position_transform,
					       &crtc->client_sprite_f_image_transform);
}

int
ProcRRSetCrtcSpriteTransform (ClientPtr client)
{
    REQUEST(xRRSetCrtcSpriteTransformReq);
    RRCrtcPtr		    crtc;
    PictTransform	    position_transform, image_transform;
    struct pixman_f_transform f_position_transform, f_image_transform;

    REQUEST_AT_LEAST_SIZE(xRRSetCrtcSpriteTransformReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    PictTransform_from_xRenderTransform (&position_transform, &stuff->positionTransform);
    PictTransform_from_xRenderTransform (&image_transform, &stuff->imageTransform);
    pixman_f_transform_from_pixman_transform (&f_position_transform, &position_transform);
    pixman_f_transform_from_pixman_transform (&f_image_transform, &image_transform);

    RRCrtcSpriteTransformSet (crtc, &position_transform, &image_transform,
			      &f_position_transform, &f_image_transform);
    return Success;
}

#define CrtcSpriteTransformExtra	(SIZEOF(xRRGetCrtcSpriteTransformReply) - 32)

int
ProcRRGetCrtcSpriteTransform (ClientPtr client)
{
    REQUEST(xRRGetCrtcSpriteTransformReq);
    xRRGetCrtcSpriteTransformReply	*reply;
    RRCrtcPtr			crtc;
    int				n;
    char			*extra;

    REQUEST_SIZE_MATCH (xRRGetCrtcSpriteTransformReq);
    VERIFY_RR_CRTC(stuff->crtc, crtc, DixReadAccess);

    reply = malloc(sizeof (xRRGetCrtcSpriteTransformReply));
    if (!reply)
	return BadAlloc;

    extra = (char *) (reply + 1);
    reply->type = X_Reply;
    reply->sequenceNumber = client->sequence;
    reply->length = bytes_to_int32(CrtcSpriteTransformExtra);

    xRenderTransform_from_PictTransform(&reply->positionTransform, &crtc->client_sprite_position_transform);
    xRenderTransform_from_PictTransform(&reply->imageTransform, &crtc->client_sprite_image_transform);

    if (client->swapped) {
	swaps (&reply->sequenceNumber, n);
	swapl (&reply->length, n);
	SwapLongs((CARD32 *) &reply->positionTransform, bytes_to_int32(sizeof(xRenderTransform)));
	SwapLongs((CARD32 *) &reply->imageTransform, bytes_to_int32(sizeof(xRenderTransform)));
    }
    WriteToClient (client, sizeof (xRRGetCrtcSpriteTransformReply), (char *) reply);
    free(reply);
    return Success;
}
