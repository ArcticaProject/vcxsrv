/* $Xorg: paths.c,v 1.3 2000/08/17 19:46:31 cpqbld Exp $ */
/* Copyright International Business Machines, Corp. 1991
 * All Rights Reserved
 * Copyright Lexmark International, Inc. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM or Lexmark not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM AND LEXMARK PROVIDE THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES OF
 * ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE
 * QUALITY AND PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF THE
 * SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM OR LEXMARK) ASSUMES THE
 * ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN NO EVENT SHALL
 * IBM OR LEXMARK BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/* $XFree86: xc/lib/font/Type1/paths.c,v 1.7tsi Exp $ */

 /* PATHS    CWEB         V0021 ********                             */
/*
:h1 id=paths.PATHS Module - Path Operator Handler
 
This is the module that is responsible for building and transforming
path lists.
 
&author. Jeffrey B. Lotspiech (lotspiech@almaden.ibm.com)
 
 
:h3.Include Files
 
The included files are:
*/
 
                             /*   after the system includes  (dsr)           */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef FONTMODULE
# include  "os.h"
#endif
#include  "objects.h"
#include  "spaces.h"
#include  "paths.h"
#include  "regions.h"      /* understands about Union                      */
#include  "fonts.h"        /* understands about TEXTTYPEs                  */
#include  "pictures.h"     /* understands about handles                    */
#include  "strokes.h"      /* understands how to coerce stroke paths       */
#include  "trig.h"


/*
:h3.Routines Available to the TYPE1IMAGER User
 
The PATHS routines that are made available to the outside user are:
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
:h3.Functions Provided to Other Modules
 
The path routines that are made available to other TYPE1IMAGER modules
are defined here:
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
NOTE:  because of the casts put in the macros for Loc, ArcCA, Conic,
RoundConic, PathSegment, and JoinSegment, we cannot use the macro names
when the functions are actually defined.  We have to use the unique
names with their unique first two characters.  Thus, if anyone in the
future ever decided to change the first two characters, it would not be
enough just to change the macro (as it would for most other functions).
He would have to also change the function definition.
*/
/*
:h3.Macros Provided to Other Modules
 
The CONCAT macro is defined here and used in the STROKES module.  See
:hdref refid=pathmac..
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
:h2.Path Segment Structures
 
A path is represented as a linked list of the following structure:
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
When 'link' is NULL, we are at the last segment in the path (surprise!).
 
'last' is only non-NULL on the first segment of a path,
for all the other segments 'last' == NULL.  We test for a non-NULL
'last' (ISPATHANCHOR predicate) when we are given an alleged path
to make sure the user is not trying to pull a fast one on us.
 
A path may be a collection of disjoint paths.  Every break in the
disjoint path is represented by a MOVETYPE segment.
 
Closed paths are discussed in :hdref refid=close..
 
:h3.CopyPath() - Physically Duplicating a Path
 
This simple function illustrates moving through the path linked list.
Duplicating a segment just involves making a copy of it, except for
text, which has some auxilliary things involved.  We don't feel
competent to duplicate text in this module, so we call someone who
knows how (in the FONTS module).
*/
struct segment *
CopyPath(struct segment *p0)         /* path to duplicate                    */
{
       register struct segment *p,*n = NULL,*last = NULL,*anchor;
 
       for (p = p0, anchor = NULL; p != NULL; p = p->link) {
 
               ARGCHECK((!ISPATHTYPE(p->type) || (p != p0 && p->last != NULL)),
                       "CopyPath: invalid segment", p, NULL, (0), struct segment *);
 
               if (p->type == TEXTTYPE)
                       n = (struct segment *) CopyText(p);
               else
                       n = (struct segment *)Allocate(p->size, p, 0);
               n->last = NULL;
               if (anchor == NULL)
                       anchor = n;
               else
                       last->link = n;
               last = n;
       }
/*
At this point we have a chain of newly allocated segments hanging off
'anchor'.  We need to make sure the first segment points to the last:
*/
       if (anchor != NULL) {
               n->link = NULL;
               anchor->last = n;
       }
 
       return(anchor);
}
/*
:h3.KillPath() - Destroying a Path
 
Destroying a path is simply a matter of freeing each segment in the
linked list.  Again, we let the experts handle text.
*/
void 
KillPath(struct segment *p)         /* path to destroy                       */
{
       register struct segment *linkp;  /* temp register holding next segment*/
 
       /* return conditional based on reference count 3-26-91 PNM */
       if ( (--(p->references) > 1) ||
          ( (p->references == 1) && !ISPERMANENT(p->flag) ) )
           return;
 
       while (p != NULL) {
               if (!ISPATHTYPE(p->type)) {
                       ArgErr("KillPath: bad segment", p, NULL);
                       return;
               }
               linkp = p->link;
               if (p->type == TEXTTYPE)
                       KillText(p);
               else
                       Free(p);
               p = linkp;
       }
}
 
/*
:h2 id=location."location" Objects
 
The TYPE1IMAGER user creates and destroys objects of type "location".  These
objects locate points for the primitive path operators.  We play a trick
here and store these objects in the same "segment" structure used for
paths, with a type field == MOVETYPE.
 
This allows the Line() operator, for example, to be very trivial:
It merely stamps its input structure as a LINETYPE and returns it to the
caller--assuming, of course, the input structure was not permanent (as
it usually isn't).
 
:h3.The "movesegment" Template Structure
 
This template is used as a generic segment structure for Allocate:
*/
 
/* added reference field 1 to temporary template below 3-26-91 PNM */
static struct segment movetemplate = { MOVETYPE, 0, 1, sizeof(struct segment), 0,
                NULL, NULL, {0, 0} };
/*
:h3.Loc() - Create an "Invisible Line" Between (0,0) and a Point
 
*/
 
struct segment *
t1_Loc(struct XYspace *S,    /* coordinate space to interpret X,Y            */
       double x, double y)   /* destination point                            */
{
       register struct segment *r;
 
 
       r = (struct segment *)Allocate(sizeof(struct segment), &movetemplate, 0);
       TYPECHECK("Loc", S, SPACETYPE, r, (0), struct segment *);
 
       r->last = r;
       r->context = S->context;
       (*S->convert)(&r->dest, S, x, y);
       ConsumeSpace(S);
       return(r);
}
/*
:h3.ILoc() - Loc() With Integer Arguments
 
*/
struct segment *
ILoc(struct XYspace *S,         /* coordinate space to interpret X,Y         */
     int x, int y)              /* destination point                         */
{
       register struct segment *r;
 
       r = (struct segment *)Allocate(sizeof(struct segment), &movetemplate, 0);
       TYPECHECK("Loc", S, SPACETYPE, r, (0), struct segment *);
 
       r->last = r;
       r->context = S->context;
       (*S->iconvert)(&r->dest, S, (long) x, (long) y);
       ConsumeSpace(S);
       return(r);
}
 
/*
:h2.Straight Line Segments
 
:h3.PathSegment() - Create a Generic Path Segment
 
Many routines need a LINETYPE or MOVETYPE path segment, but do not
want to go through the external user's interface, because, for example,
they already know the "fractpel" destination of the segment and the
conversion is unnecessary.  PathSegment() is an internal routine
provided to the rest of TYPE1IMAGER for handling these cases.
*/
 
struct segment *
t1_PathSegment(int type,     /* LINETYPE or MOVETYPE                         */
	       fractpel x, fractpel y) /* where to go to, if known           */
{
       register struct segment *r;  /* newly created segment                 */
 
       r = (struct segment *)Allocate(sizeof(struct segment), &movetemplate, 0);
       r->type = type;
       r->last = r;          /* last points to itself for singleton          */
       r->dest.x = x;
       r->dest.y = y;
       return(r);
}
/*
:h3.Line() - Create a Line Segment Between (0,0) and a Point P
 
This involves just creating and filling out a segment structure:
*/
struct segment *
Line(struct segment *P)      /* relevant coordinate space                    */
{
       ARGCHECK(!ISLOCATION(P), "Line: arg not a location", P, NULL, (0), struct segment *);
 
       P = UniquePath(P);
       P->type = LINETYPE;
       return(P);
}
/*
:h2.Curved Path Segments
 
We need more points to describe curves.  So, the structures for curved
path segments are slightly different.  The first part is identical;
the curved structures are larger with the extra points on the end.
 
:h3.Bezier Segment Structure
 
We support third order Bezier curves.  They are specified with four
control points A, B, C, and D.  The curve starts at A with slope AB
and ends at D with slope CD.  The curvature at the point A is inversely
related to the length |AB|, and the curvature at the point D is
inversely related to the length |CD|.  Point A is always point (0,0).
 
*/
 
/*SHARED LINE(S) ORIGINATED HERE*/
/*
:h3.Bezier() - Generate a Bezier Segment
 
This is just a simple matter of filling out a 'beziersegment' structure:
*/
 
struct beziersegment *
Bezier(struct segment *B,           /* second control point                  */
       struct segment *C,           /* third control point                   */
       struct segment *D)           /* fourth control point (ending point)   */
{
/* added reference field of 1 to temporary template below 3-26-91  PNM */
       static struct beziersegment template =
                    { BEZIERTYPE, 0, 1, sizeof(struct beziersegment), 0,
                      NULL, NULL, { 0, 0 }, { 0, 0 }, { 0, 0 } };
 
       register struct beziersegment *r;  /* output segment                  */
 
       ARGCHECK(!ISLOCATION(B), "Bezier: bad B", B, NULL, (2,C,D), struct beziersegment *);
       ARGCHECK(!ISLOCATION(C), "Bezier: bad C", C, NULL, (2,B,D), struct beziersegment *);
       ARGCHECK(!ISLOCATION(D), "Bezier: bad D", D, NULL, (2,B,C), struct beziersegment *);
 
       r = (struct beziersegment *)Allocate(sizeof(struct beziersegment), &template, 0);
       r->last = (struct segment *) r;
       r->dest.x = D->dest.x;
       r->dest.y = D->dest.y;
       r->B.x = B->dest.x;
       r->B.y = B->dest.y;
       r->C.x = C->dest.x;
       r->C.y = C->dest.y;
 
       ConsumePath(B);
       ConsumePath(C);
       ConsumePath(D);
       return(r);
}
 
/*SHARED LINE(S) ORIGINATED HERE*/
 
/*
POP removes the first segment in a path 'p' and Frees it.  'p' is left
pointing to the end of the path:
*/
#define POP(p) \
     { register struct segment *linkp; \
       linkp = p->link; \
       if (linkp != NULL) \
               linkp->last = p->last; \
       Free(p); \
       p = linkp; }
/*
INSERT inserts a single segment in the middle of a chain.  'b' is
the segment before, 'p' the segment to be inserted, and 'a' the
segment after.
*/
#define INSERT(b,p,a)  b->link=p; p->link=a; p->last=NULL
 
/*
:h3.Join() - Join Two Objects Together
 
If these are paths, this operator simply invokes the CONCAT macro.
Why so much code then, you ask?  Well we have to check for object
types other than paths, and also check for certain path consistency
rules.
*/
 
struct segment *
Join(struct segment *p1, struct segment *p2)
{
/*
We start with a whole bunch of very straightforward argument tests:
*/
       if (p2 != NULL) {
               if (!ISPATHTYPE(p2->type)) {
 
                       if (p1 == NULL)
                               return((struct segment *)Unique(p2));
 
                       switch (p1->type) {
 
                           case REGIONTYPE:
 
                           case STROKEPATHTYPE:
                               p1 = CoercePath(p1);
                               break;
 
                           default:
                               return((struct segment *)BegHandle(p1, p2));
                       }
               }
 
               ARGCHECK((p2->last == NULL), "Join: right arg not anchor", p2, NULL, (1,p1), struct segment *);
               p2 = UniquePath(p2);
 
/*
In certain circumstances, we don't have to duplicate a permanent
location.  (We would just end up destroying it anyway).  These cases
are when 'p2' begins with a move-type segment:
*/
               if (p2->type == TEXTTYPE || p2->type == MOVETYPE) {
                       if (p1 == NULL)
                               return(p2);
                       if (ISLOCATION(p1)) {
                               p2->dest.x += p1->dest.x;
                               p2->dest.y += p1->dest.y;
                               ConsumePath(p1);
                               return(p2);
                       }
               }
       }
       else
               return((struct segment *)Unique(p1));
 
       if (p1 != NULL) {
               if (!ISPATHTYPE(p1->type))
 
                       switch (p2->type) {
 
                           case REGIONTYPE:
 
                           case STROKEPATHTYPE:
                               p2 = CoercePath(p2);
                               break;
 
                           default:
                               return((struct segment *)EndHandle(p1, p2));
                       }
 
               ARGCHECK((p1->last == NULL), "Join: left arg not anchor", p1, NULL, (1,p2), struct segment *);
               p1 = UniquePath(p1);
       }
       else
               return(p2);
 
/*
At this point all the checking is done.  We have two temporary non-null
path types in 'p1' and 'p2'.  If p1 ends with a MOVE, and p2 begins with
a MOVE, we collapse the two MOVEs into one.  We enforce the rule that
there may not be two MOVEs in a row:
*/
 
       if (p1->last->type == MOVETYPE && p2->type == MOVETYPE) {
               p1->last->flag |= p2->flag;
               p1->last->dest.x += p2->dest.x;
               p1->last->dest.y += p2->dest.y;
               POP(p2);
               if (p2 == NULL)
                       return(p1);
       }
/*
Now we check for another silly rule.  If a path has any TEXTTYPEs,
then it must have only TEXTTYPEs and MOVETYPEs, and furthermore,
it must begin with a TEXTTYPE.  This rule makes it easy to check
for the special case of text.  If necessary, we will coerce
TEXTTYPEs into paths so we don't mix TEXTTYPEs with normal paths.
*/
       if (p1->type == TEXTTYPE) {
               if (p2->type != TEXTTYPE && !ISLOCATION(p2))
                       p1 = CoerceText(p1);
       }
       else {
               if (p2->type == TEXTTYPE) {
                       if (ISLOCATION(p1)) {
                               p2->dest.x += p1->dest.x;
                               p2->dest.y += p1->dest.y;
                               Free(p1);
                               return(p2);
                       }
                       else
                               p2 = CoerceText(p2);
               }
       }
/*
Thank God!  Finally!  It's hard to believe, but we are now able to
actually do the join.  This is just invoking the CONCAT macro:
*/
       CONCAT(p1, p2);
 
       return(p1);
}
 
/*
:h3.JoinSegment() - Create a Path Segment and Join It to a Known Path
 
This internal function is quicker than a full-fledged join because
it can do much less checking.
*/
 
struct segment *
t1_JoinSegment(struct segment *before, /* path to join before new segment    */
	       int type,     /* type of new segment (MOVETYPE or LINETYPE)   */
	       fractpel x, fractpel y, /* x,y of new segment                 */
	       struct segment *after) /* path to join after new segment      */
{
       register struct segment *r;  /* returned path built here              */
 
       r = PathSegment(type, x, y);
       if (before != NULL) {
               CONCAT(before, r);
               r = before;
       }
       else
               r->context = after->context;
       if (after != NULL)
               CONCAT(r, after);
       return(r);
}
 
/*
:h2.Other Path Functions
 
*/
 
 
struct segment *
t1_ClosePath(struct segment *p0, /* path to close                            */
	     int lastonly)     /*  flag deciding to close all subpaths or... */
{
       register struct segment *p,*last = NULL,*start;  /* used in looping through path */
       register fractpel x,y;  /* current position in path                   */
       register fractpel firstx = 0,firsty = 0;  /* start position of sub path       */
       register struct segment *lastnonhint = NULL;  /* last non-hint segment in path */
 
       if (p0 != NULL && p0->type == TEXTTYPE)
               return(UniquePath(p0));
       if (p0->type == STROKEPATHTYPE)
               return((struct segment *)Unique(p0));
       /*
       * NOTE: a null closed path is different from a null open path
       * and is denoted by a closed (0,0) move segment.  We make
       * sure this path begins and ends with a MOVETYPE:
       */
       if (p0 == NULL || p0->type != MOVETYPE)
               p0 = JoinSegment(NULL, MOVETYPE, 0, 0, p0);
       TYPECHECK("ClosePath", p0, MOVETYPE, NULL, (0), struct segment *);
       if (p0->last->type != MOVETYPE)
               p0 = JoinSegment(p0, MOVETYPE, 0, 0, NULL);
 
       p0 = UniquePath(p0);
 
/*
We now begin a loop through the path,
incrementing current 'x' and 'y'.  We are searching
for MOVETYPE segments (breaks in the path) that are not already closed.
At each break, we insert a close segment.
*/
       for (p = p0, x = y = 0, start = NULL;
            p != NULL;
            x += p->dest.x, y += p->dest.y, last = p, p = p->link)
       {
 
               if (p->type == MOVETYPE) {
                       if (start != NULL && (lastonly?p->link==NULL:TRUE) &&
                             !(ISCLOSED(start->flag) && LASTCLOSED(last->flag))) {
                               register struct segment *r;  /* newly created */
 
                               start->flag |= ISCLOSED(ON);
                               r = PathSegment(LINETYPE, firstx - x,
                                                         firsty - y);
                               INSERT(last, r, p);
                               r->flag |= LASTCLOSED(ON);
                               /*< adjust 'last' if possible for a 0,0 close >*/
{
 
#define   CLOSEFUDGE    3    /* if we are this close, let's change last segment */
 
       if (r->dest.x != 0 || r->dest.y != 0) {
               if (r->dest.x <= CLOSEFUDGE && r->dest.x >= -CLOSEFUDGE
                    && r->dest.y <= CLOSEFUDGE && r->dest.y >= -CLOSEFUDGE) {
                       lastnonhint->dest.x += r->dest.x;
                       lastnonhint->dest.y += r->dest.y;
                       r->dest.x = r->dest.y = 0;
               }
       }
}
                               if (p->link != NULL) {
                                       p->dest.x += x - firstx;
                                       p->dest.y += y - firsty;
                                       x = firstx;
                                       y = firsty;
                               }
                       }
                       start = p;
                       firstx = x + p->dest.x;
                       firsty = y + p->dest.y;
               }
               else if (p->type != HINTTYPE)
                       lastnonhint = p;
       }
       return(p0);
}
 
/*
:h2.Transforming and Putting Handles on Paths
 
:h3.PathTransform() - Transform a Path
 
Transforming a path involves transforming all the points.  In order
that closed paths do not become "unclosed" when their relative
positions are slightly changed due to loss of arithmetic precision,
all point transformations are in absolute coordinates.
 
(It might be better to reset the "absolute" coordinates every time a
move segment is encountered.  This would mean that we could accumulate
error from subpath to subpath, but we would be less likely to make
the "big error" where our fixed point arithmetic "wraps".  However, I
think I'll keep it this way until something happens to convince me
otherwise.)
 
The transform is described as a "space", that way we can use our
old friend the "iconvert" function, which should be very efficient.
*/
 
struct segment *
PathTransform(struct segment *p0,      /* path to transform                  */
	      struct XYspace *S)       /* pseudo space to transform in       */
{
       register struct segment *p;   /* to loop through path with            */
       register fractpel newx,newy;  /* current transformed position in path */
       register fractpel oldx,oldy;  /* current untransformed position in path */
       register fractpel savex,savey;  /* save path delta x,y                */
 
       p0 = UniquePath(p0);
 
       newx = newy = oldx = oldy = 0;
 
       for (p=p0; p != NULL; p=p->link) {
 
               savex = p->dest.x;   savey = p->dest.y;
 
               (*S->iconvert)(&p->dest, S, p->dest.x + oldx, p->dest.y + oldy);
               p->dest.x -= newx;
               p->dest.y -= newy;
 
               switch (p->type) {
 
                   case LINETYPE:
                   case MOVETYPE:
                       break;
 
                   case CONICTYPE:
                   {
                       register struct conicsegment *cp = (struct conicsegment *) p;
 
                       (*S->iconvert)(&cp->M, S, cp->M.x + oldx, cp->M.y + oldy);
                       cp->M.x -= newx;
                       cp->M.y -= newy;
                       /*
                       * Note roundness doesn't change... linear transform
                       */
                       break;
                   }
 
 
                   case BEZIERTYPE:
                   {
                       register struct beziersegment *bp = (struct beziersegment *) p;
 
                       (*S->iconvert)(&bp->B, S, bp->B.x + oldx, bp->B.y + oldy);
                       bp->B.x -= newx;
                       bp->B.y -= newy;
                       (*S->iconvert)(&bp->C, S, bp->C.x + oldx, bp->C.y + oldy);
                       bp->C.x -= newx;
                       bp->C.y -= newy;
                       break;
                   }
 
                   case HINTTYPE:
                   {
                       register struct hintsegment *hp = (struct hintsegment *) p;
 
                       (*S->iconvert)(&hp->ref, S, hp->ref.x + oldx, hp->ref.y + oldy);
                       hp->ref.x -= newx;
                       hp->ref.y -= newy;
                       (*S->iconvert)(&hp->width, S, hp->width.x, hp->width.y);
                       /* Note: width is not relative to origin */
                       break;
                   }
 
                   case TEXTTYPE:
                   {
                        XformText(p,S);
                        break;
                   }
 
                   default:
                       Abort("PathTransform:  invalid segment");
               }
               oldx += savex;
               oldy += savey;
               newx += p->dest.x;
               newy += p->dest.y;
       }
       return(p0);
}
 
/*
:h3.PathDelta() - Return a Path's Ending Point
*/
 
void 
PathDelta(struct segment *p,       /* input path                             */
	  struct fractpoint *pt)   /* pointer to x,y to set                  */
{
       struct fractpoint mypoint;  /* I pass this to TextDelta               */
       register fractpel x,y;  /* working variables for path current point   */

       mypoint.x = mypoint.y = 0;
 
       for (x=y=0; p != NULL; p=p->link) {
               x += p->dest.x;
               y += p->dest.y;
               if (p->type == TEXTTYPE) {
                       TextDelta(p, &mypoint);
                       x += mypoint.x;
                       y += mypoint.y;
               }
       }
 
       pt->x = x;
       pt->y = y;
}
 
/*
:h2.Querying Locations and Paths
 
:h3.QueryLoc() - Return the X,Y of a Locition
*/
 
void 
QueryLoc(struct segment *P,      /* location to query, not consumed          */
	 struct XYspace *S,      /* XY space to return coordinates in        */
	 double *xP, double *yP) /* coordinates returned here                */
{
       if (!ISLOCATION(P)) {
               ArgErr("QueryLoc: first arg not a location", P, NULL);
               return;
       }
       if (S->type != SPACETYPE) {
               ArgErr("QueryLoc: second arg not a space", S, NULL);
               return;
       }
       UnConvert(S, &P->dest, xP, yP);
}
