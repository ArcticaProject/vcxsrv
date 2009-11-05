/* $Xorg: SetSens.c,v 1.4 2001/02/09 02:03:58 xorgcvs Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts
Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or Sun not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

/*

Copyright 1987, 1988, 1998  The Open Group

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
/* $XFree86: xc/lib/Xt/SetSens.c,v 1.3 2001/12/14 19:56:29 dawes Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "IntrinsicI.h"
#include "StringDefs.h"

/*
 *	XtSetSensitive()
 */

static void SetAncestorSensitive(
    register Widget widget,
    Boolean	    ancestor_sensitive)
{
    Arg			args[1];
    register Cardinal   i;
    register WidgetList children;

    if (widget->core.ancestor_sensitive == ancestor_sensitive) return;

    XtSetArg(args[0], XtNancestorSensitive, ancestor_sensitive);
    XtSetValues(widget, args, XtNumber(args));

    /* If widget's sensitive is TRUE, propagate new ancestor_sensitive to
       children's ancestor_sensitive; else do nothing as children's
       ancestor_sensitive is already FALSE */
    
    if (widget->core.sensitive && XtIsComposite(widget)) {
	children = ((CompositeWidget) widget)->composite.children;
	for (i=0; i < ((CompositeWidget)widget)->composite.num_children; i++) {
	    SetAncestorSensitive (children[i], ancestor_sensitive);
	}
    }
} /* SetAncestorSensitive */


void XtSetSensitive(
    register Widget widget,
    _XtBoolean	    sensitive)
{
    Arg			args[1];
    register Cardinal   i;
    register WidgetList children;
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    if (widget->core.sensitive == sensitive) {
	UNLOCK_APP(app);
	return;
    }

    XtSetArg(args[0], XtNsensitive, sensitive);
    XtSetValues(widget, args, XtNumber(args));

    /* If widget's ancestor_sensitive is TRUE, propagate new sensitive to
       children's ancestor_sensitive; else do nothing as children's
       ancestor_sensitive is already FALSE */
    
    if (widget->core.ancestor_sensitive && XtIsComposite (widget)) {
	children = ((CompositeWidget) widget)->composite.children;
	for (i = 0; i < ((CompositeWidget)widget)->composite.num_children; i++){
	    SetAncestorSensitive (children[i], sensitive);
	}
    }
    UNLOCK_APP(app);
} /* XtSetSensitive */
