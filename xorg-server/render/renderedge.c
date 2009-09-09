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

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "renderedge.h"

/*
 * Compute the smallest value no less than y which is on a
 * grid row
 */

xFixed
RenderSampleCeilY (xFixed y, int n)
{
    return pixman_sample_ceil_y (y, n);
}

#define _div(a,b)    ((a) >= 0 ? (a) / (b) : -((-(a) + (b) - 1) / (b)))

/*
 * Compute the largest value no greater than y which is on a
 * grid row
 */
xFixed
RenderSampleFloorY (xFixed y, int n)
{
    return pixman_sample_floor_y (y, n);
}

/*
 * Step an edge by any amount (including negative values)
 */
void
RenderEdgeStep (RenderEdge *e, int n)
{
    pixman_edge_step (e, n);
}

/*
 * Initialize one edge structure given the line endpoints and a
 * starting y value
 */
void
RenderEdgeInit (RenderEdge	*e,
		int		n,
		xFixed		y_start,
		xFixed		x_top,
		xFixed		y_top,
		xFixed		x_bot,
		xFixed		y_bot)
{
    pixman_edge_init (e, n, y_start, x_top, y_top, x_bot, y_bot);
}

/*
 * Initialize one edge structure given a line, starting y value
 * and a pixel offset for the line
 */
void
RenderLineFixedEdgeInit (RenderEdge *e,
			 int	    n,
			 xFixed	    y,
			 xLineFixed *line,
			 int	    x_off,
			 int	    y_off)
{
    pixman_line_fixed_edge_init (e, n, y, (pixman_line_fixed_t *)line, x_off, y_off);
}

