/* $Xorg: SetWMCW.c,v 1.4 2001/02/09 02:03:58 xorgcvs Exp $ */
/* $XdotOrg: $
 *
 * Author:  Chris D. Peterson, MIT X Consortium
 */

/* Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
 * OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * of the copyright holder.
 */

/*

Copyright 1989, 1994, 1998  The Open Group

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
#include <X11/Xatom.h>

/*	Function Name: XtSetWMColormapWindows
 *
 *	Description: Sets the value of the WM_COLORMAP_WINDOWS
 *                   property on a widget's window.
 *
 *	Arguments:  widget - specifies the widget on whose window the
 *   		           - WM_COLORMAP_WINDOWS property will be stored.
 *
 *                  list - Specifies a list of widgets whose windows are to be
 *		           listed in the WM_COLORMAP_WINDOWS property.
 *                  count - Specifies the number of widgets in list.
 *
 *	Returns: none.
 */

void
XtSetWMColormapWindows(
    Widget widget,
    Widget *list,
    Cardinal count)
{
    Window *data;
    Widget *checked, *top, *temp, hookobj;
    Cardinal i, j, checked_count;
    Boolean match;
    Atom xa_wm_colormap_windows;
    WIDGET_TO_APPCON(widget);

    LOCK_APP(app);
    if ( !XtIsRealized(widget) || (count == 0) ) {
	UNLOCK_APP(app);
	return;
    }

    top = checked = (Widget *) __XtMalloc( (Cardinal) sizeof(Widget) * count);


/*
 * The specification calls for only adding the windows that have unique
 * colormaps to the property to this function, so we will make a pass through
 * the widget list removing all the widgets with non-unique colormaps.
 *
 * We will also remove any unrealized widgets from the list at this time.
 */

    for (checked_count = 0, i = 0; i < count; i++) {
	if (!XtIsRealized(list[i])) continue;

	*checked = list[i];
	match = FALSE;

/*
 * Don't check first element for matching colormap since there is nothing
 * to check it against.
 */

	if (checked != top)
	    for (j = 0, temp = top; j < checked_count ; j++, temp++)
		if ( (*temp)->core.colormap == (*checked)->core.colormap) {
		    match = TRUE;
		    break;
		}

/*
 * If no colormap was found to match then add this widget to the linked list.
 */

	if (!match) {
	    checked++;
	    checked_count++;
	}
    }

/*
 * Now that we have the list of widgets we need to convert it to a list of
 * windows and set the property.
 */

    data = (Window *) __XtMalloc( (Cardinal) sizeof(Window) * checked_count);

    for ( i = 0 ; i < checked_count ; i++)
	data[i] = XtWindow(top[i]);

    xa_wm_colormap_windows = XInternAtom(XtDisplay(widget),
					 "WM_COLORMAP_WINDOWS", FALSE);

    XChangeProperty(XtDisplay(widget), XtWindow(widget),
		    xa_wm_colormap_windows, XA_WINDOW, 32,
		    PropModeReplace, (unsigned char *) data, (int) i);

    hookobj = XtHooksOfDisplay(XtDisplay(widget));
    if (XtHasCallbacks(hookobj, XtNchangeHook) == XtCallbackHasSome) {
	XtChangeHookDataRec call_data;

	call_data.type = XtHsetWMColormapWindows;
	call_data.widget = widget;
	call_data.event_data = (XtPointer) list;
	call_data.num_event_data = count;
	XtCallCallbackList(hookobj,
		((HookObject)hookobj)->hooks.changehook_callbacks,
		(XtPointer)&call_data);
    }

    XtFree( (char *) data);
    XtFree( (char *) top);
    UNLOCK_APP(app);
}
