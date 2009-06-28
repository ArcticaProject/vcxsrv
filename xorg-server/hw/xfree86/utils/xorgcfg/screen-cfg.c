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

#include "xf86config.h"
#include "screen-cfg.h"
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Viewport.h>
#ifdef USE_MODULES
#include "loader.h"
#endif

#define CW	1
#define CCW	-1

/*
 * Prototypes
 */
static void DepthCallback(Widget, XtPointer, XtPointer);
static void SelectIndexCallback(Widget, XtPointer, XtPointer);
static void UnselectIndexCallback(Widget, XtPointer, XtPointer);
static void SelectCallback(Widget, XtPointer, XtPointer);
static void UnselectCallback(Widget, XtPointer, XtPointer);
static void MoveCallback(Widget, XtPointer, XtPointer);
static void RotateCallback(Widget, XtPointer, XtPointer);

/*
 * Initialization
 */
static char *standard_modes[] = {
    "640x400",
    "640x480",
    "800x600",
    "1024x768",
    "1280x960",
    "1280x1024",
    "320x200",
    "320x240",
    "400x300",
    "1152x864",
    "1600x1200",
    "1800x1400",
    "512x384",
    "1400x1050",
    "2048x1536",
    "1920x1440",
};

static char **modes;
static int nmodes;
static int default_depth, sel_index, unsel_index;
static Widget listL, listR;
static char **defmodes;
static int ndefmodes;
static XF86ConfScreenPtr screen;
static int rotate;

/*
 * Implementation
 */
XtPointer
ScreenConfig(XtPointer conf)
{
    XF86ConfDisplayPtr disp;
    Arg args[2];
    int i, oldrotate;

    screen = (XF86ConfScreenPtr)conf;
    if (screen == NULL)
	return (NULL);

    XtSetArg(args[0], XtNstring, screen->scrn_identifier);
    XtSetValues(ident_widget, args, 1);
    if ((default_depth = screen->scrn_defaultdepth) <= 0)
	default_depth = 8;
    sel_index = unsel_index = -1;
    for (i = 0; i < computer.num_screens; i++)
	if (computer.screens[i]->screen == screen) {
	    SetScreenRotate(computer.screens[i]);
	    rotate = computer.screens[i]->rotate;
    }
    oldrotate = rotate;

    ndefmodes = 0;
    disp = screen->scrn_display_lst;
    while (disp != NULL) {
	if (disp->disp_depth == default_depth) {
	    XF86ModePtr mod = disp->disp_mode_lst;

	    while (mod != NULL) {
		if (ndefmodes % 16 == 0)
		    defmodes = (char**)
			XtRealloc((XtPointer)defmodes,
				  (ndefmodes + 16) * sizeof(char*));
		defmodes[ndefmodes++] = XtNewString(mod->mode_name);
		mod = (XF86ModePtr)(mod->list.next);
	    }
	    break;
	}
	disp = (XF86ConfDisplayPtr)(disp->list.next);
    }
    if (ndefmodes == 0) {
	defmodes = (char**)XtMalloc(sizeof(char*));
	defmodes[0] = XtNewString("640x480");
	ndefmodes = 1;
    }

    if (listL != NULL) {
	XawListUnhighlight(listL);
	XawListUnhighlight(listR);
    }

    xf86info.cur_list = SCREEN;
    XtSetSensitive(back, xf86info.lists[SCREEN].cur_function > 0);
    XtSetSensitive(next, xf86info.lists[SCREEN].cur_function <
			 xf86info.lists[SCREEN].num_functions - 1);
    (xf86info.lists[SCREEN].functions[xf86info.lists[SCREEN].cur_function])
	(&xf86info);

    if (ConfigLoop(NULL) == True) {
	XF86ModePtr prev = NULL, mod;

	/* user may have changed the default depth, read variables again */
	disp = screen->scrn_display_lst;
	while (disp != NULL) {
	    if (disp->disp_depth == default_depth)
		break;
	    disp = (XF86ConfDisplayPtr)(disp->list.next);
	}

	if (disp == NULL) {
	    disp = (XF86ConfDisplayPtr)XtCalloc(1, sizeof(XF86ConfDisplayRec));
	    screen->scrn_display_lst = (XF86ConfDisplayPtr)
		xf86addListItem((GenericListPtr)(screen->scrn_display_lst),
			    (GenericListPtr)(disp));
	    disp->disp_depth = default_depth;
	}

	if (strcasecmp(screen->scrn_identifier, ident_string))
	    xf86renameScreen(XF86Config, screen, ident_string);

	screen->scrn_defaultdepth = default_depth;

	XtSetArg(args[0], XtNlist, NULL);
	XtSetArg(args[1], XtNnumberStrings, 0);
	XtSetValues(listL, args, 2);

	XtSetArg(args[0], XtNlist, NULL);
	XtSetArg(args[1], XtNnumberStrings, 0);
	XtSetValues(listR, args, 2);

	mod = disp->disp_mode_lst;
	/* free all modes */
	while (mod != NULL) {
	    prev = mod;
	    mod = (XF86ModePtr)(mod->list.next);
	    XtFree(prev->mode_name);
	    XtFree((XtPointer)prev);
	}
	/* readd modes */
	for (i = 0; i < ndefmodes; i++) {
	    mod = XtNew(XF86ModeRec);
	    mod->mode_name = XtNewString(defmodes[i]);
	    XtFree(defmodes[i]);
	    if (i == 0)
		disp->disp_mode_lst = mod;
	    else
		prev->list.next = mod;
	    prev = mod;
	}
	if (i == 0)
	    disp->disp_mode_lst = NULL;
	else
	    mod->list.next = NULL;

	XtFree((XtPointer)defmodes);
	defmodes = NULL;
	ndefmodes = 0;

	for (i = 0; i < computer.num_screens; i++)
	    if (computer.screens[i]->screen == screen)
		computer.screens[i]->rotate = rotate;

	if (oldrotate != rotate) {
	    static char *Rotate = "Rotate";

	    if (screen->scrn_option_lst != NULL)
		xf86removeOption(&screen->scrn_option_lst, Rotate);
	    if (rotate)
		screen->scrn_option_lst =
		    xf86addNewOption(screen->scrn_option_lst,
				     XtNewString(Rotate),
				     XtNewString(rotate > 0 ? "CW" : "CCW"));
	    UpdateScreenUI();
	    AdjustScreenUI();
	}

	return ((XtPointer)screen);
    }

    XtSetArg(args[0], XtNlist, NULL);
    XtSetArg(args[1], XtNnumberStrings, 0);
    XtSetValues(listL, args, 2);

    XtSetArg(args[0], XtNlist, NULL);
    XtSetArg(args[1], XtNnumberStrings, 0);
    XtSetValues(listR, args, 2);

    for (i = 0; i < ndefmodes; i++)
	XtFree(defmodes[i]);
    XtFree((XtPointer)defmodes);
    defmodes = NULL;
    ndefmodes = 0;

    return (NULL);
}

/*ARGSUSED*/
static void
DepthCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    if (call_data != NULL)
	default_depth = (long)user_data;
}

/*ARGSUSED*/
static void
SelectIndexCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;

    sel_index = info->list_index;
}

/*ARGSUSED*/
static void
UnselectIndexCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;

    unsel_index = info->list_index;
}

/*ARGSUSED*/
static void
SelectCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[2];

    if (sel_index < 0 || sel_index >= nmodes)
	return;

    if (ndefmodes == 1 && *defmodes[0] == '\0') {
	/* make sure tmp and defentries are not the same pointer */
	char **tmp = defmodes;

	XtFree(defmodes[0]);
	defmodes = (char**)XtMalloc(sizeof(char*));
	--ndefmodes;
	XtFree((char*)tmp);
    }
    else
	defmodes = (char**)XtRealloc((XtPointer)defmodes,
				     sizeof(char*) * (ndefmodes + 1));
    defmodes[ndefmodes++] = XtNewString(modes[sel_index]);

    XtSetArg(args[0], XtNlist, defmodes);
    XtSetArg(args[1], XtNnumberStrings, ndefmodes);
    XtSetValues(listR, args, 2);

    XawListUnhighlight(listR);
    if (ndefmodes > 1 || (ndefmodes == 1 && *defmodes[0] != '\0')) {
	if (unsel_index >= ndefmodes)
	    unsel_index = ndefmodes - 1;
	XawListHighlight(listR, unsel_index = ndefmodes - 1);
    }
    else
	unsel_index = -1;

    /* force realyout */
    XtUnmanageChild(listR);
    XtManageChild(listR);
}

/*ARGSUSED*/
static void
UnselectCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[2];
    char **modes;
    Cardinal num_modes;

    if (unsel_index < 0 || unsel_index >= ndefmodes)
	return;

    XawListUnhighlight(listL);
    XtSetArg(args[0], XtNlist, &modes);
    XtSetArg(args[1], XtNnumberStrings, &num_modes);
    XtGetValues(listL, args, 2);
    if (modes) {
	for (sel_index = 0; sel_index < num_modes; sel_index++)
	    if (strcmp(defmodes[unsel_index], modes[sel_index]) == 0)
		break;
	if (sel_index < num_modes)
	    XawListHighlight(listL, sel_index);
	else
	    sel_index = -1;
    }

    XtFree(defmodes[unsel_index]);
    if (--ndefmodes > unsel_index)
	memmove(&defmodes[unsel_index], &defmodes[unsel_index + 1],
		(ndefmodes - unsel_index) * sizeof(char*));
    if (ndefmodes == 0) {
	char **tmp = defmodes;

	defmodes = (char**)XtMalloc(sizeof(char*));
	defmodes[0] = XtNewString("");
	ndefmodes = 1;
	XtFree((char*)tmp);
    }

    XtSetArg(args[0], XtNlist, defmodes);
    XtSetArg(args[1], XtNnumberStrings, ndefmodes);
    XtSetValues(listR, args, 2);

    XawListUnhighlight(listR);
    if (ndefmodes > 1 || (ndefmodes == 1 && *defmodes[0] != '\0')) {
	if (unsel_index >= ndefmodes)
	    unsel_index = ndefmodes - 1;
	XawListHighlight(listR, unsel_index);
    }
    else
	unsel_index = -1;
}

/*ARGSUSED*/
static void
MoveCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    char *tmp;
    Bool down = (long)user_data;

    if (unsel_index < 0 || unsel_index >= ndefmodes)
	return;

    if ((down && unsel_index + 1 >= ndefmodes) ||
	(!down && unsel_index - 1 < 0))
	return;

    tmp = defmodes[unsel_index];
    if (down) {
	defmodes[unsel_index] = defmodes[unsel_index + 1];
	unsel_index++;
    } else {
	defmodes[unsel_index] = defmodes[unsel_index - 1];
	unsel_index--;
    }
    defmodes[unsel_index] = tmp;

    XawListUnhighlight(listR);
    XawListHighlight(listR, unsel_index);
}

/*ARGSUSED*/
void
RotateCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    if (call_data != NULL)
	rotate = (long)user_data;
    else
	rotate = 0;
}

void
ScreenDialog(XF86SetupInfo *info)
{
    static Widget dialog, d1, d4, d8, d16, d24, labelRotate, cw, ccw;
    Arg args[2];
    XF86ConfMonitorPtr mon = screen->scrn_monitor;
    XF86ConfModeLinePtr mline = mon != NULL ? mon->mon_modeline_lst : NULL;
    int i;
#ifdef USE_MODULES
    xf86cfgModuleOptions *drv_opts = module_options;
    Bool foundRotate = False;
#endif

    while (nmodes > 0)
	XtFree(modes[--nmodes]);
    XtFree((XtPointer)modes);
    modes = NULL;
    while (mline) {
	if (nmodes % 16 == 0)
	    modes = (char**)XtRealloc((XtPointer)modes,
				      sizeof(char*) * (nmodes + 16));
	modes[nmodes++] = XtNewString(mline->ml_identifier);
	mline = (XF86ConfModeLinePtr)(mline->list.next);
    }
    for (i = 0; i < sizeof(standard_modes) / sizeof(standard_modes[0]); i++) {
	if (nmodes % 16 == 0)
	    modes = (char**)XtRealloc((XtPointer)modes,
				      sizeof(char*) * (nmodes + 16));
	modes[nmodes++] = XtNewString(standard_modes[i]);
    }

    if (dialog == NULL) {
	Widget command, viewport;

	dialog = XtCreateWidget("screenD", formWidgetClass,
				configp, NULL, 0);
	XtCreateManagedWidget("depthL", labelWidgetClass,
			      dialog, NULL, 0);
	d1 = XtCreateManagedWidget("1", toggleWidgetClass, dialog, NULL, 0);
	XtAddCallback(d1, XtNcallback, DepthCallback, (XtPointer)1);
	d4 = XtVaCreateManagedWidget("4", toggleWidgetClass, dialog,
				     XtNradioGroup, d1, NULL);
	XtAddCallback(d4, XtNcallback, DepthCallback, (XtPointer)4);
	d8 = XtVaCreateManagedWidget("8", toggleWidgetClass, dialog,
				      XtNradioGroup, d4, NULL);
	XtAddCallback(d8, XtNcallback, DepthCallback, (XtPointer)8);
	d16 = XtVaCreateManagedWidget("16", toggleWidgetClass, dialog,
				      XtNradioGroup, d8, NULL);
	XtAddCallback(d16, XtNcallback, DepthCallback, (XtPointer)16);
	d24 = XtVaCreateManagedWidget("24", toggleWidgetClass, dialog,
				      XtNradioGroup, d16, NULL);
	XtAddCallback(d24, XtNcallback, DepthCallback, (XtPointer)24);

	XtCreateManagedWidget("modeL", labelWidgetClass, dialog, NULL, 0);
	viewport = XtCreateManagedWidget("viewL", viewportWidgetClass, dialog,
					 NULL, 0);
	listL = XtCreateManagedWidget("listLeft", listWidgetClass, viewport,
					NULL, 0);
	XtAddCallback(listL, XtNcallback, SelectIndexCallback, NULL);
	command = XtCreateManagedWidget("select", commandWidgetClass,
					dialog, NULL, 0);
	XtAddCallback(command, XtNcallback, SelectCallback, NULL);
	command = XtCreateManagedWidget("unselect", commandWidgetClass,
					dialog, NULL, 0);
	XtAddCallback(command, XtNcallback, UnselectCallback, NULL);
	command = XtCreateManagedWidget("up", commandWidgetClass,
					dialog, NULL, 0);
	XtAddCallback(command, XtNcallback, MoveCallback, (XtPointer)False);
	command = XtCreateManagedWidget("down", commandWidgetClass,
					dialog, NULL, 0);
	XtAddCallback(command, XtNcallback, MoveCallback, (XtPointer)True);
	viewport = XtCreateManagedWidget("viewR", viewportWidgetClass, dialog,
					 NULL, 0);
	listR = XtCreateManagedWidget("listRight", listWidgetClass, viewport,
				      NULL, 0);
	XtAddCallback(listR, XtNcallback, UnselectIndexCallback, NULL);

	labelRotate = XtCreateManagedWidget("rotate", labelWidgetClass,
					    dialog, NULL, 0);
	cw = XtCreateManagedWidget("CW", toggleWidgetClass, dialog, NULL, 0);
	XtAddCallback(cw, XtNcallback, RotateCallback, (XtPointer)CW);
	ccw = XtVaCreateManagedWidget("CCW", toggleWidgetClass, dialog,
				      XtNradioGroup, cw, NULL);
	XtAddCallback(ccw, XtNcallback, RotateCallback, (XtPointer)CCW);

	XtRealizeWidget(dialog);
    }

#ifdef USE_MODULES
    if (!nomodules) {
	while (drv_opts) {
	    if (drv_opts->type == VideoModule &&
		strcmp(drv_opts->name, screen->scrn_device->dev_driver) == 0) {
		OptionInfoPtr opts = drv_opts->option;

		while (opts->name) {
		    if (xf86nameCompare(opts->name, "Rotate") == 0) {
			foundRotate = True;
			break;
		    }
		    opts++;
		}
		break;
	    }
	    drv_opts = drv_opts->next;
	}

	if (!foundRotate) {
	    XtUnmapWidget(labelRotate);
	    XtUnmapWidget(cw);
	    XtUnmapWidget(ccw);
	}
	else {
	    XtMapWidget(labelRotate);
	    XtMapWidget(cw);
	    XtMapWidget(ccw);
	}
    }
#else
    (void)labelRotate;
#endif
    if (rotate == CW) {
	XtVaSetValues(cw, XtNstate, True, NULL);
	XtVaSetValues(ccw, XtNstate, False, NULL);
    }
    else if (rotate == CCW) {
	XtVaSetValues(cw, XtNstate, False, NULL);
	XtVaSetValues(ccw, XtNstate, True, NULL);
    }
    else {
	XtVaSetValues(cw, XtNstate, False, NULL);
	XtVaSetValues(ccw, XtNstate, False, NULL);
    }

    XtSetArg(args[0], XtNlist, modes);
    XtSetArg(args[1], XtNnumberStrings, nmodes);
    XtSetValues(listL, args, 2);

    XtSetArg(args[0], XtNlist, defmodes);
    XtSetArg(args[1], XtNnumberStrings, ndefmodes);
    XtSetValues(listR, args, 2);

    XtSetArg(args[0], XtNstate, True);
    XtSetValues(default_depth == 1 ? d1 :
		default_depth == 4 ? d4 :
		default_depth == 16 ? d16 :
		default_depth == 24 ? d24 : d8, args, 1);

    XtChangeManagedSet(&current, 1, NULL, NULL, &dialog, 1);
    current = dialog;
}
