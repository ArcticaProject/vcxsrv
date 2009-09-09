/***********************************************************

Copyright 1989, 1998  The Open Group

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


Copyright 1989 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef MISPANS_H
#define MISPANS_H

typedef struct {
    int         count;		/* number of spans		    */
    DDXPointPtr points;		/* pointer to list of start points  */
    int         *widths;	/* pointer to list of widths	    */
} Spans;

typedef struct {
    int		size;		/* Total number of *Spans allocated	*/
    int		count;		/* Number of *Spans actually in group   */
    Spans       *group;		/* List of Spans			*/
    int		ymin, ymax;	/* Min, max y values encountered	*/
} SpanGroup;

/* Initialize SpanGroup.  MUST BE DONE before use. */
extern _X_EXPORT void miInitSpanGroup(
    SpanGroup * /*spanGroup*/
);

/* Add a Spans to a SpanGroup. The spans MUST BE in y-sorted order */
extern _X_EXPORT void miAppendSpans(
    SpanGroup * /*spanGroup*/,
    SpanGroup * /*otherGroup*/,
    Spans * /*spans*/
);

/* Paint a span group, insuring that each pixel is painted at most once */
extern _X_EXPORT void miFillUniqueSpanGroup(
    DrawablePtr /*pDraw*/,
    GCPtr /*pGC*/,
    SpanGroup * /*spanGroup*/
);

/* Free up data in a span group.  MUST BE DONE or you'll suffer memory leaks */
extern _X_EXPORT void miFreeSpanGroup(
    SpanGroup * /*spanGroup*/
);

extern _X_EXPORT int miClipSpans(
    RegionPtr /*prgnDst*/,
    DDXPointPtr /*ppt*/,
    int * /*pwidth*/,
    int /*nspans*/,
    DDXPointPtr /*pptNew*/,
    int * /*pwidthNew*/,
    int /*fSorted*/
);

/* Rops which must use span groups */
#define miSpansCarefulRop(rop)	(((rop) & 0xc) == 0x8 || ((rop) & 0x3) == 0x2)
#define miSpansEasyRop(rop)	(!miSpansCarefulRop(rop))

#endif /* MISPANS_H */
