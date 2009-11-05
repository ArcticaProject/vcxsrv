/* $Xorg: ActionHook.c,v 1.4 2001/02/09 02:03:53 xorgcvs Exp $ */

/*LINTLIBRARY*/

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
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
/* $XFree86: xc/lib/Xt/ActionHook.c,v 1.2 2001/08/22 22:52:17 dawes Exp $ */

/* 
 * Contains XtAppAddActionHook, XtRemoveActionHook
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "IntrinsicI.h"


/*ARGSUSED*/
static void FreeActionHookList(
    Widget widget,		/* unused (and invalid) */
    XtPointer closure,		/* ActionHook* */
    XtPointer call_data)	/* unused */
{
    ActionHook list = *(ActionHook*)closure;
    while (list != NULL) {
	ActionHook next = list->next;
	XtFree( (XtPointer)list );
	list = next;
    }
}


XtActionHookId XtAppAddActionHook(
    XtAppContext app,
    XtActionHookProc proc,
    XtPointer closure)
{
    ActionHook hook = XtNew(ActionHookRec);
    LOCK_APP(app);
    hook->next = app->action_hook_list;
    hook->app = app;
    hook->proc = proc;
    hook->closure = closure;
    if (app->action_hook_list == NULL) {
	_XtAddCallback( &app->destroy_callbacks,
		        FreeActionHookList,
		        (XtPointer)&app->action_hook_list
		      );
    }
    app->action_hook_list = hook;
    UNLOCK_APP(app);
    return (XtActionHookId)hook;
}


void XtRemoveActionHook(
    XtActionHookId id)
{
    ActionHook *p, hook = (ActionHook)id;
    XtAppContext app = hook->app;
    LOCK_APP(app);
    for (p = &app->action_hook_list; p != NULL && *p != hook; p = &(*p)->next);
    if (p) {
	*p = hook->next;
	XtFree( (XtPointer)hook );
	if (app->action_hook_list == NULL)
	    _XtRemoveCallback(&app->destroy_callbacks, FreeActionHookList,
			      (XtPointer) &app->action_hook_list);
    }
#ifdef DEBUG
    else {
	XtAppWarningMsg(app, "badId", "xtRemoveActionHook", XtCXtToolkitError,
			"XtRemoveActionHook called with bad or old hook id",
			(String*)NULL, (Cardinal*)NULL);
    }
#endif /*DEBUG*/
    UNLOCK_APP(app);
}
