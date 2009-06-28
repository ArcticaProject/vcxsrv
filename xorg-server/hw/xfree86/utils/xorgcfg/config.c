/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 */

#include "config.h"
#include "mouse-cfg.h"
#include "keyboard-cfg.h"
#include "card-cfg.h"
#include "monitor-cfg.h"
#include "screen-cfg.h"
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Shell.h>

/*
 * Prototypes
 */
void BackCallback(Widget, XtPointer, XtPointer);
void NextCallback(Widget, XtPointer, XtPointer);
void ApplyCallback(Widget, XtPointer, XtPointer);
void CloseCallback(Widget, XtPointer, XtPointer);
void ErrorCallback(Widget, XtPointer, XtPointer);

/*
 * Initialization
 */
XF86SetupFunction mouse_functions[] = {
    MouseDeviceAndProtocol,
};

XF86SetupFunction keyboard_functions[] = {
    KeyboardModelAndLayout,
};

XF86SetupFunction card_functions[] = {
    CardModel,
};

XF86SetupFunction monitor_functions[] = {
    MonitorLayout,
};

XF86SetupFunction screen_functions[] = {
    ScreenDialog,
};

XF86SetupFunctionList function_lists[] = {
    {mouse_functions, sizeof(mouse_functions) / sizeof(mouse_functions[0]),},
    {keyboard_functions, sizeof(keyboard_functions) / sizeof(keyboard_functions[0]),},
    {card_functions, sizeof(card_functions) / sizeof(card_functions[0]),},
    {monitor_functions, sizeof(monitor_functions) / sizeof(monitor_functions[0]),},
    {screen_functions, sizeof(screen_functions) / sizeof(screen_functions[0]),},
};

XF86SetupInfo xf86info = {
    sizeof(function_lists) / sizeof(function_lists[0]),
    MOUSE,
    function_lists,
};

Widget configp, current, ok, back, next;
static Widget shell, errcurrent, oldcurrent;

static int config_status, config_popped;

static ConfigCheckFunction config_function;

Widget ident_widget;
char *ident_string;
XF86ConfigPtr XF86Config;

/*
 * Implementation
 */
void
StartConfig(void)
{
    static int first = 1;
    Widget pane, top, bottom, cancel;
    const char *filename;

    if (!first)
	return;
    first = 0;

    /* Read initial configuration */
    if ((filename = xf86openConfigFile(getuid() == 0 ? CONFPATH : USER_CONFPATH,
				       XF86Config_path, NULL)) == NULL) {
	int length = XF86Config_path ? strlen(XF86Config_path) : -1;

	if (length > 2 &&
	    XF86Config_path[length - 2] == '-' &&
	    XF86Config_path[length - 1] == '4') {
	    XF86Config_path[length - 2] = '\0';
	    filename = xf86openConfigFile(getuid() == 0 ?
					  CONFPATH : USER_CONFPATH,
					  XF86Config_path, NULL);
	}

	if (filename == NULL) {
	    fprintf(stderr, "Cannot open config file.\n");
	    exit(1);
	}
    }
    XF86Config_path = (char *)filename;
    if ((XF86Config = xf86readConfigFile()) == NULL) {
	fprintf(stderr, "Problem when parsing config file\n");
	exit(1);
    }

    shell = XtCreatePopupShell("config", transientShellWidgetClass,
			       toplevel, NULL, 0);
    pane = XtCreateManagedWidget("pane", panedWidgetClass,
				 shell, NULL, 0);
    top = XtCreateManagedWidget("top", formWidgetClass,
				pane, NULL, 0);
    (void) XtCreateManagedWidget("label", labelWidgetClass,
				  top, NULL, 0);
    ident_widget = XtVaCreateManagedWidget("identifier", asciiTextWidgetClass,
					   top,
					   XtNeditType, XawtextEdit,
					   NULL);
    configp = XtCreateManagedWidget("work", formWidgetClass,
				    pane, NULL, 0);
    current = XtCreateManagedWidget("wellcome", labelWidgetClass,
				    configp, NULL, 0);
    bottom = XtCreateManagedWidget("bottom", formWidgetClass,
				   pane, NULL, 0);
    back = XtCreateManagedWidget("back", commandWidgetClass,
				 bottom, NULL, 0);
    XtAddCallback(back, XtNcallback, BackCallback, (XtPointer)&xf86info);
    next = XtCreateManagedWidget("next", commandWidgetClass,
				 bottom, NULL, 0);
    XtAddCallback(next, XtNcallback, NextCallback, (XtPointer)&xf86info);
    ok = XtCreateManagedWidget("ok", commandWidgetClass,
			       bottom, NULL, 0);
    XtAddCallback(ok, XtNcallback, ApplyCallback, (XtPointer)NULL);
    cancel = XtCreateManagedWidget("cancel", commandWidgetClass,
				   bottom, NULL, 0);
    XtAddCallback(cancel, XtNcallback, CloseCallback, (XtPointer)NULL);

    XtRealizeWidget(shell);

    XSetWMProtocols(DPY, XtWindow(shell), &wm_delete_window, 1);
}

/*ARGSUSED*/
Bool
ConfigLoop(ConfigCheckFunction config_fn)
{
    Arg args[1];
    config_popped = True;
    XtPopup(shell, XtGrabExclusive);

    config_function = config_fn;
    while (config_popped)
	XtAppProcessEvent(XtWidgetToApplicationContext(shell), XtIMAll);

    XtSetArg(args[0], XtNstring, &ident_string);
    XtGetValues(ident_widget, args, 1);

    return (config_status);
}

/*ARGSUSED*/
void
ConfigError(void)
{
    static int first = 1;

    if (first) {
	Widget command;

	errcurrent = XtCreateWidget("error", formWidgetClass,
			 	     configp, NULL, 0);
	(void) XtCreateManagedWidget("label", labelWidgetClass,
				      errcurrent, NULL, 0);
	command = XtCreateManagedWidget("command", commandWidgetClass,
					errcurrent, NULL, 0);
	XtAddCallback(command, XtNcallback, ErrorCallback, NULL);

	XtRealizeWidget(errcurrent);
    }

    oldcurrent = current;
    XtChangeManagedSet(&current, 1, NULL, NULL, &errcurrent, 1);
    current = errcurrent;

    XtSetSensitive(ok, False);
}

/*ARGSUSED*/
void
ErrorCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XtChangeManagedSet(&errcurrent, 1, NULL, NULL, &oldcurrent, 1);
    current = oldcurrent;

    XtSetSensitive(ok, True);
}

/*ARGSUSED*/
void
BackCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XF86SetupInfo *info = (XF86SetupInfo*)user_data;
    int idx = info->lists[info->cur_list].cur_function - 1;

    if (idx >= 0 && info->lists[info->cur_list].num_functions > 0) {
	info->lists[info->cur_list].cur_function = idx;
	if (idx - 1 == -1)
	    XtSetSensitive(back, False);
	if (!XtIsSensitive(next))
	    XtSetSensitive(next, True);
	(info->lists[info->cur_list].functions[idx])(info);
    }
}

/*ARGSUSED*/
void
NextCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XF86SetupInfo *info = (XF86SetupInfo*)user_data;
    int idx = info->lists[info->cur_list].cur_function + 1;

    if (idx < info->lists[info->cur_list].num_functions) {
	info->lists[info->cur_list].cur_function = idx;
	if (idx + 1 == info->lists[info->cur_list].num_functions)
	    XtSetSensitive(next, False);
	if (!XtIsSensitive(back))
	    XtSetSensitive(back, True);
	(info->lists[info->cur_list].functions[idx])(info);
    }
}

/*ARGSUSED*/
void
CloseCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XtPopdown(shell);
    config_popped = False;
    config_status = False;
    /* make sure it is sensitive */
    XtSetSensitive(ok, True);
    xf86info.lists[xf86info.cur_list].cur_function = 0;
}

/*ARGSUSED*/
void
ApplyCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];

    XtSetArg(args[0], XtNstring, &ident_string);
    XtGetValues(ident_widget, args, 1);

    if (config_function == NULL || (*config_function)()) {
	XtPopdown(shell);
	config_popped = False;
	config_status = True;
	xf86info.lists[xf86info.cur_list].cur_function = 0;
    }
    else
	ConfigError();
}

/*ARGSUSED*/
void
ConfigCancelAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    CloseCallback(w, NULL, NULL);
}
