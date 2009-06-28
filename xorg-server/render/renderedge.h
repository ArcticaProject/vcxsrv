/*
 * Copyright © 2004 Keith Packard
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

#ifndef _RENDEREDGE_H_
#define _RENDEREDGE_H_

#include "picturestr.h"

#define MAX_ALPHA(n)	((1 << (n)) - 1)
#define N_Y_FRAC(n)	((n) == 1 ? 1 : (1 << ((n)/2)) - 1)
#define N_X_FRAC(n)	((1 << ((n)/2)) + 1)

#define STEP_Y_SMALL(n)	(xFixed1 / N_Y_FRAC(n))
#define STEP_Y_BIG(n)	(xFixed1 - (N_Y_FRAC(n) - 1) * STEP_Y_SMALL(n))

#define Y_FRAC_FIRST(n)	(STEP_Y_SMALL(n) / 2)
#define Y_FRAC_LAST(n)	(Y_FRAC_FIRST(n) + (N_Y_FRAC(n) - 1) * STEP_Y_SMALL(n))

#define STEP_X_SMALL(n)	(xFixed1 / N_X_FRAC(n))
#define STEP_X_BIG(n)	(xFixed1 - (N_X_FRAC(n) - 1) * STEP_X_SMALL(n))

#define X_FRAC_FIRST(n)	(STEP_X_SMALL(n) / 2)
#define X_FRAC_LAST(n)	(X_FRAC_FIRST(n) + (N_X_FRAC(n) - 1) * STEP_X_SMALL(n))

#define RenderSamplesX(x,n)	((n) == 1 ? 0 : (xFixedFrac (x) + X_FRAC_FIRST(n)) / STEP_X_SMALL(n))

/*
 * An edge structure.  This represents a single polygon edge
 * and can be quickly stepped across small or large gaps in the
 * sample grid
 */
typedef pixman_edge_t RenderEdge;

/*
 * Step across a small sample grid gap
 */
#define RenderEdgeStepSmall(edge) { \
    edge->x += edge->stepx_small;   \
    edge->e += edge->dx_small;	    \
    if (edge->e > 0)		    \
    {				    \
	edge->e -= edge->dy;	    \
	edge->x += edge->signdx;    \
    }				    \
}

/*
 * Step across a large sample grid gap
 */
#define RenderEdgeStepBig(edge) {   \
    edge->x += edge->stepx_big;	    \
    edge->e += edge->dx_big;	    \
    if (edge->e > 0)		    \
    {				    \
	edge->e -= edge->dy;	    \
	edge->x += edge->signdx;    \
    }				    \
}

xFixed
RenderSampleCeilY (xFixed y, int bpp);

xFixed
RenderSampleFloorY (xFixed y, int bpp);

void
RenderEdgeStep (RenderEdge *e, int n);

void
RenderEdgeInit (RenderEdge	*e,
		int		bpp,
		xFixed		y_start,
		xFixed		x_top,
		xFixed		y_top,
		xFixed		x_bot,
		xFixed		y_bot);

void
RenderLineFixedEdgeInit (RenderEdge *e,
			 int	    bpp,
			 xFixed	    y,
			 xLineFixed *line,
			 int	    x_off,
			 int	    y_off);

#endif /* _RENDEREDGE_H_ */
