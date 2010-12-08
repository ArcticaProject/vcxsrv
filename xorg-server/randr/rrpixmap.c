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
#include "xace.h"

int
ProcRRQueryScanoutPixmaps (ClientPtr client)
{
    REQUEST(xRRQueryScanoutPixmapsReq);
    xRRQueryScanoutPixmapsReply	rep;
    RRScanoutPixmapInfo		*info;
    xRRScanoutPixmapInfo	*x_info;
    int				n_info;
    int				rc;
    DrawablePtr			drawable;
    ScreenPtr			screen;
    rrScrPrivPtr		screen_priv;
    int 			n, s;

    REQUEST_SIZE_MATCH(xRRQueryScanoutPixmapsReq);
    rc = dixLookupDrawable(&drawable, stuff->drawable, client, 0, DixGetAttrAccess);
    if (rc != Success) {
	client->errorValue = stuff->drawable;
	return rc;
    }

    screen = drawable->pScreen;
    screen_priv = rrGetScrPriv(screen);

    rep.type = X_Reply;
    /* rep.status has already been filled in */
    rep.length = 0;
    rep.sequenceNumber = client->sequence;

    info = RRQueryScanoutPixmapInfo(screen, &n_info);
    x_info = calloc(n_info, sizeof (xRRScanoutPixmapInfo));
    if (n_info && !x_info)
	return BadAlloc;
    rep.length += (n_info * sizeof (xRRScanoutPixmapInfo)) >> 2;
    if (client->swapped) {
	swaps(&rep.sequenceNumber, n);
	swapl(&rep.length, n);
    }

    for (s = 0; s < n_info; s++) {
	x_info[s].format = info[s].format->id;
	x_info[s].maxWidth = info[s].maxWidth;
	x_info[s].maxHeight = info[s].maxHeight;
	x_info[s].rotations = info[s].rotations;
	if (client->swapped) {
	    swapl(&x_info[s].format, n);
	    swaps(&x_info[s].maxWidth, n);
	    swaps(&x_info[s].maxHeight, n);
	    swaps(&x_info[s].rotations, n);
	}
    }

    WriteToClient(client, sizeof(rep), (char *)&rep);
    if (n_info)
	WriteToClient(client, n_info * sizeof (xRRScanoutPixmapInfo),
		      (char *) x_info);
    return Success;
}

int
ProcRRCreateScanoutPixmap (ClientPtr client)
{
    REQUEST(xRRCreateScanoutPixmapReq);
    int 		rc;
    DrawablePtr		drawable;
    ScreenPtr		screen;
    rrScrPrivPtr	screen_priv;
    PixmapPtr		pixmap;
    int			n_info;
    RRScanoutPixmapInfo	*info;
    int			s;

    REQUEST_SIZE_MATCH(xRRCreateScanoutPixmapReq);
    client->errorValue = stuff->pid;
    LEGAL_NEW_RESOURCE(stuff->pid, client);

    rc = dixLookupDrawable(&drawable, stuff->drawable, client, 0, DixGetAttrAccess);
    if (rc != Success) {
	client->errorValue = stuff->drawable;
	return rc;
    }
    screen = drawable->pScreen;
    screen_priv = rrGetScrPriv(screen);
    if (!screen_priv)
	return BadValue;

    info = RRQueryScanoutPixmapInfo(screen, &n_info);
    for (s = 0; s < n_info; s++) {
	if (info[s].format->id == stuff->format)
	    break;
    }
    if (s == n_info || !screen_priv->rrCreateScanoutPixmap) {
	client->errorValue = stuff->format;
	return BadValue;
    }
    info = &info[s];
    if (!stuff->width || stuff->width > info->maxWidth) {
	client->errorValue = stuff->width;
	return BadValue;
    }
    if (!stuff->height || stuff->height > info->maxHeight) {
	client->errorValue = stuff->height;
	return BadValue;
    }
    if ((stuff->rotations & info->rotations) != stuff->rotations) {
	client->errorValue = stuff->rotations;
	return BadValue;
    }

    pixmap = screen_priv->rrCreateScanoutPixmap (screen,
						 stuff->width, stuff->height,
						 info->depth,
						 stuff->rotations,
						 info->format);
    if (!pixmap)
	return BadAlloc;

    pixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER;
    pixmap->drawable.id = stuff->pid;
    rc = XaceHook(XACE_RESOURCE_ACCESS, client, stuff->pid, RT_PIXMAP,
		  pixmap, RT_NONE, NULL, DixCreateAccess);
    if (rc != Success) {
	screen->DestroyPixmap(pixmap);
	return rc;
    }
    if (!AddResource(stuff->pid, RT_PIXMAP, pixmap))
	return BadAlloc;
    return Success;
}
