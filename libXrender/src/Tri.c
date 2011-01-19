/*
 *
 * Copyright © 2002 Keith Packard
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xrenderint.h"

void
XRenderCompositeTriangles (Display		*dpy,
			   int			op,
			   Picture		src,
			   Picture		dst,
			    _Xconst XRenderPictFormat	*maskFormat,
			   int			xSrc,
			   int			ySrc,
			   _Xconst XTriangle	*triangles,
			   int			ntriangle)
{
    XRenderExtDisplayInfo         *info = XRenderFindDisplay (dpy);
    xRenderTrianglesReq	    *req;
    int			    n;
    long    		    len;

    RenderSimpleCheckExtension (dpy, info);
    LockDisplay(dpy);
    while (ntriangle)
    {
	GetReq(RenderTriangles, req);
	req->reqType = info->codes->major_opcode;
	req->renderReqType = X_RenderTriangles;
	req->op = (CARD8) op;
	req->src = src;
	req->dst = dst;
	req->maskFormat = maskFormat ? maskFormat->id : 0;
	req->xSrc = xSrc;
	req->ySrc = ySrc;
	n = ntriangle;
	len = ((long) n) * (SIZEOF (xTriangle) >> 2);
	if (!dpy->bigreq_size && len > (dpy->max_request_size - req->length)) {
	    n = (dpy->max_request_size - req->length) / (SIZEOF (xTriangle) >> 2);
	    len = ((long)n) * (SIZEOF (xTriangle) >> 2);
	}
	SetReqLen (req, len, len);
	len <<= 2;
	DataInt32 (dpy, (int *) triangles, len);
	ntriangle -= n;
	triangles += n;
    }
    UnlockDisplay(dpy);
    SyncHandle();
}

void
XRenderCompositeTriStrip (Display		*dpy,
			  int			op,
			  Picture		src,
			  Picture		dst,
			  _Xconst XRenderPictFormat	*maskFormat,
			  int			xSrc,
			  int			ySrc,
			  _Xconst XPointFixed	*points,
			  int			npoint)
{
    XRenderExtDisplayInfo         *info = XRenderFindDisplay (dpy);
    xRenderTriStripReq	    *req;
    int			    n;
    long    		    len;

    RenderSimpleCheckExtension (dpy, info);
    LockDisplay(dpy);
    while (npoint > 2)
    {
	GetReq(RenderTriStrip, req);
	req->reqType = info->codes->major_opcode;
	req->renderReqType = X_RenderTriStrip;
	req->op = (CARD8) op;
	req->src = src;
	req->dst = dst;
	req->maskFormat = maskFormat ? maskFormat->id : 0;
	req->xSrc = xSrc;
	req->ySrc = ySrc;
	n = npoint;
	len = ((long) n) * (SIZEOF (xPointFixed) >> 2);
	if (!dpy->bigreq_size && len > (dpy->max_request_size - req->length)) {
	    n = (dpy->max_request_size - req->length) / (SIZEOF (xPointFixed) >> 2);
	    len = ((long)n) * (SIZEOF (xPointFixed) >> 2);
	}
	SetReqLen (req, len, len);
	len <<= 2;
	DataInt32 (dpy, (int *) points, len);
	npoint -= (n - 2);
	points += (n - 2);
    }
    UnlockDisplay(dpy);
    SyncHandle();
}

void
XRenderCompositeTriFan (Display			*dpy,
			int			op,
			Picture			src,
			Picture			dst,
			_Xconst XRenderPictFormat	*maskFormat,
			int			xSrc,
			int			ySrc,
			_Xconst XPointFixed	*points,
			int			npoint)
{
    XRenderExtDisplayInfo         *info = XRenderFindDisplay (dpy);
    _Xconst XPointFixed	    *first = points;
    xPointFixed		    *p;
    xRenderTriFanReq	    *req;
    int			    n;
    long    		    len;

    RenderSimpleCheckExtension (dpy, info);
    LockDisplay(dpy);
    points++;
    npoint--;
    while (npoint > 1)
    {
	GetReqExtra(RenderTriFan, SIZEOF (xPointFixed), req);
	req->reqType = info->codes->major_opcode;
	req->renderReqType = X_RenderTriFan;
	req->op = (CARD8) op;
	req->src = src;
	req->dst = dst;
	req->maskFormat = maskFormat ? maskFormat->id : 0;
	req->xSrc = xSrc;
	req->ySrc = ySrc;
	p = (xPointFixed *) (req + 1);
	p->x = first->x;
	p->y = first->y;
	n = npoint;
	len = ((long) n) * (SIZEOF (xPointFixed) >> 2);
	if (!dpy->bigreq_size && len > (dpy->max_request_size - req->length)) {
	    n = (dpy->max_request_size - req->length) / (SIZEOF (xPointFixed) >> 2);
	    len = ((long)n) * (SIZEOF (xPointFixed) >> 2);
	}
	SetReqLen (req, len, len);
	len <<= 2;
	DataInt32 (dpy, (int *) points, len);
	npoint -= (n - 1);
	points += (n - 1);
    }
    UnlockDisplay(dpy);
    SyncHandle();
}
