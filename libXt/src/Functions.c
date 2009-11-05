/* $Xorg: Functions.c,v 1.4 2001/02/09 02:03:54 xorgcvs Exp $ */

/*

Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name Sun not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

/*

Copyright 1985, 1986, 1987, 1988, 1989, 1994, 1998  The Open Group

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "IntrinsicI.h"
#include <X11/Shell.h>
#include <X11/Vendor.h>

/*
 * This file defines functional equivalents to all macros defined
 * in Intrinsic.h
 *
 */

#undef XtIsRectObj
Boolean XtIsRectObj(Widget object)
{
    return _XtCheckSubclassFlag(object, 0x02);
}


#undef XtIsWidget
Boolean XtIsWidget(Widget object)
{
    return _XtCheckSubclassFlag(object, 0x04);
}


#undef XtIsComposite
Boolean XtIsComposite(Widget object)
{
    return _XtCheckSubclassFlag(object, 0x08);
}


#undef XtIsConstraint
Boolean XtIsConstraint(Widget object)
{
    return _XtCheckSubclassFlag(object, 0x10);
}


#undef XtIsShell
Boolean XtIsShell(Widget object)
{
    return _XtCheckSubclassFlag(object, 0x20);
}


#undef XtIsOverrideShell
Boolean XtIsOverrideShell(Widget object)
{
    return _XtIsSubclassOf(object, (WidgetClass)overrideShellWidgetClass,
			   (WidgetClass)shellWidgetClass, 0x20);
}


#undef XtIsWMShell
Boolean XtIsWMShell(Widget object)
{
    return _XtCheckSubclassFlag(object, 0x40);
}


#undef XtIsVendorShell
Boolean XtIsVendorShell(Widget object)
{
    Boolean retval;

    LOCK_PROCESS;
    retval = _XtIsSubclassOf(object,
#ifdef notdef
/*
 * We don't refer to vendorShell directly, because some shared libraries
 * bind local references tightly.
 */
			   (WidgetClass)vendorShellWidgetClass,
#endif
			   transientShellWidgetClass->core_class.superclass,
			   (WidgetClass)wmShellWidgetClass, 0x40);
    UNLOCK_PROCESS;
    return retval;
}


#undef XtIsTransientShell
Boolean XtIsTransientShell(Widget object)
{
    return _XtIsSubclassOf(object, (WidgetClass)transientShellWidgetClass,
			   (WidgetClass)wmShellWidgetClass, 0x40);
}


#undef XtIsTopLevelShell
Boolean XtIsTopLevelShell(Widget object)
{
    return _XtCheckSubclassFlag(object, 0x80);
}


#undef XtIsApplicationShell
Boolean XtIsApplicationShell(Widget object)
{
    return _XtIsSubclassOf(object, (WidgetClass)applicationShellWidgetClass,
			   (WidgetClass)topLevelShellWidgetClass, 0x80);
}

#undef XtIsSessionShell
Boolean XtIsSessionShell(Widget object)
{
    return _XtIsSubclassOf(object, (WidgetClass)sessionShellWidgetClass,
			   (WidgetClass)topLevelShellWidgetClass, 0x80);
}

#undef XtMapWidget
void XtMapWidget(Widget w)
{
    Widget hookobj;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    XMapWindow(XtDisplay(w), XtWindow(w));
    hookobj = XtHooksOfDisplay(XtDisplay(w));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;

	call_data.type = XtHmapWidget;
	call_data.widget = w;
	XtCallCallbackList(hookobj,
		((HookObject)hookobj)->hooks.changehook_callbacks,
		(XtPointer)&call_data);
    }
    UNLOCK_APP(app);
}


#undef XtUnmapWidget
void XtUnmapWidget(Widget w)
{
    Widget hookobj;
    WIDGET_TO_APPCON(w);

    LOCK_APP(app);
    XUnmapWindow(XtDisplay(w), XtWindow(w));
    hookobj = XtHooksOfDisplay(XtDisplay(w));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;

	call_data.type = XtHunmapWidget;
	call_data.widget = w;
	XtCallCallbackList(hookobj,
		((HookObject)hookobj)->hooks.changehook_callbacks,
		(XtPointer)&call_data);
    }
    UNLOCK_APP(app);
}


#undef XtNewString
String XtNewString(String str)
{
    if (str == NULL)
	return NULL;
    else
	return strcpy(__XtMalloc((unsigned)strlen(str) + 1), str);
}
