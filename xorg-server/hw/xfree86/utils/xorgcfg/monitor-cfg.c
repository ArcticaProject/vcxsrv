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
#include "monitor-cfg.h"
#include "screen.h"
#include <X11/extensions/xf86vmode.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Simple.h>

#include <ctype.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>

/*
 * Prototypes
 */
static Bool MonitorConfigCheck(void);
static void MonitorHsyncCallback(Widget, XtPointer, XtPointer);
static void MonitorVsyncCallback(Widget, XtPointer, XtPointer);
static void MonitorSelectCardCallback(Widget, XtPointer, XtPointer);

extern void DrawCables(void);

/*
 * Initialization
 */
static char *hmodes[] = {
    "Standard VGA, 640x480 @ 60 Hz",
    "Super VGA, 800x600 @ 56 Hz",
    "1024x768 @ 87 Hz int. (no 800x600)",
    "1024x768 @ 87 Hz int., 800x600 @ 56 Hz",
    "800x600 @ 60 Hz, 640x480 @ 72 Hz",
    "1024x768 @ 60 Hz, 800x600 @ 72 Hz",
    "High Frequency SVGA, 1024x768 @ 70 Hz",
    "Monitor that can do 1280x1024 @ 60 Hz",
    "Monitor that can do 1280x1024 @ 74 Hz",
    "Monitor that can do 1280x1024 @ 76 Hz",
    "Monitor that can do 1280x1024 @ 85 Hz",
    "Monitor that can do 1600x1200 @ 85 Hz",
    "Monitor that can do 1920x1440 @ 85 Hz",
    "Monitor that can do 2048x1536 @ 85 Hz"
};

static char *hmodes_trans[] = {
    "31.5",
    "31.5 - 35.1",
    "31.5, 35.5",
    "31.5, 35.15, 35.5",
    "31.5 - 37.9",
    "31.5 - 48.5",
    "31.5 - 57.0",
    "31.5 - 64.3",
    "31.5 - 79.0",
    "31.5 - 82.0",
    "31.5 - 92.0",
    "31.5 - 108.0",
    "31.5 - 128.5",
    "31.5 - 137.0"
};

static char *vmodes [] = { "50 - 70", "50 - 90", "50 - 100", "40 - 150", };

extern Widget config;
static Widget hsync, vsync, hlist, vlist, cmenu;

static parser_range mon_hsync[CONF_MAX_HSYNC];
static parser_range mon_vrefresh[CONF_MAX_VREFRESH];
static int mon_n_hsync, mon_n_vrefresh;
static XF86ConfDevicePtr oldcard, card;
static XF86ConfMonitorPtr current_monitor;

/*
 * Implementation
 */
XtPointer
MonitorConfig(XtPointer conf)
{
    XF86ConfMonitorPtr monitor = (XF86ConfMonitorPtr)conf;
    char monitor_name[48];
    Arg args[1];

    current_monitor = monitor;

    xf86info.cur_list = MONITOR;
    XtSetSensitive(back, xf86info.lists[MONITOR].cur_function > 0);
    XtSetSensitive(next, xf86info.lists[MONITOR].cur_function <
			 xf86info.lists[MONITOR].num_functions - 1);
    (xf86info.lists[MONITOR].functions[xf86info.lists[MONITOR].cur_function])
	(&xf86info);

    XawListUnhighlight(hlist);
    XawListUnhighlight(vlist);

    if (monitor != NULL) {
	XF86ConfScreenPtr screen = XF86Config->conf_screen_lst;
	char str[PARSER_RANGE_SIZE];

	XtSetArg(args[0], XtNstring, monitor->mon_identifier);
	XtSetValues(ident_widget, args, 1);

	while (screen != NULL) {
	    if (screen->scrn_monitor == monitor)
		break;

	    screen = (XF86ConfScreenPtr)(screen->list.next);
	}
	if (screen != NULL) {
	    oldcard = card = screen->scrn_device;
	    XtSetArg(args[0], XtNlabel, card->dev_identifier);
	}
	else {
	    oldcard = card = NULL;
	    XtSetArg(args[0], XtNlabel, "");
	}
	XtSetValues(cmenu, args, 1);

	mon_n_hsync = monitor->mon_n_hsync;
	memcpy(mon_hsync, monitor->mon_hsync,
	       sizeof(parser_range) * mon_n_hsync);
	*str = '\0';
	parser_range_to_string(str, mon_hsync, mon_n_hsync);
	XtSetArg(args[0], XtNstring, str);
	XtSetValues(hsync, args, 1);

	mon_n_vrefresh = monitor->mon_n_vrefresh;
	memcpy(mon_vrefresh, monitor->mon_vrefresh,
	       sizeof(parser_range) * mon_n_vrefresh);
	*str = '\0';
	parser_range_to_string(str, mon_vrefresh, mon_n_vrefresh);
	XtSetArg(args[0], XtNstring, str);
	XtSetValues(vsync, args, 1);
    }
    else {
	XF86ConfMonitorPtr monitor = XF86Config->conf_monitor_lst;
	int nmonitors = 0;

	oldcard = card = NULL;
	while (monitor != NULL) {
		++nmonitors;
	    monitor = (XF86ConfMonitorPtr)(monitor->list.next);
	}
	do {
	    XmuSnprintf(monitor_name, sizeof(monitor_name),
			"Monitor%d", nmonitors);
	    ++nmonitors;
	} while (xf86findMonitor(monitor_name,
		 XF86Config->conf_monitor_lst));

	XtSetArg(args[0], XtNstring, monitor_name);
	XtSetValues(ident_widget, args, 1);

	XtSetArg(args[0], XtNstring, "");
	XtSetValues(hsync, args, 1);
	XtSetValues(vsync, args, 1);

	XtSetArg(args[0], XtNlabel, "");
	XtSetValues(cmenu, args, 1);
    }

    if (ConfigLoop(MonitorConfigCheck) == True) {
	if (monitor == NULL) {
	    monitor = (XF86ConfMonitorPtr)
		XtCalloc(1, sizeof(XF86ConfMonitorRec));
	    monitor->mon_identifier = XtNewString(ident_string);
	}

	memcpy(monitor->mon_hsync, mon_hsync, sizeof(parser_range) *
	       (monitor->mon_n_hsync = mon_n_hsync));
	memcpy(monitor->mon_vrefresh, mon_vrefresh, sizeof(parser_range) *
	       (monitor->mon_n_vrefresh = mon_n_vrefresh));

	if (strcasecmp(monitor->mon_identifier, ident_string))
	    xf86renameMonitor(XF86Config, monitor, ident_string);

	if (oldcard != card) {
	    int i;

	    for (i = 0; i < computer.num_devices; i++)
		if (computer.devices[i]->widget == config)
		    break;
	    if (computer.devices[i]->config == NULL)
		XF86Config->conf_monitor_lst =
				xf86addMonitor(XF86Config->conf_monitor_lst,
					       monitor);
	    computer.devices[i]->config = (XtPointer)monitor;
	    ChangeScreen(monitor, monitor, card, oldcard);
	    DrawCables();
	}

	return (monitor);
    }

    return (NULL);
}

static Bool
MonitorConfigCheck(void)
{
    char *str;
    Arg args[1];
    XF86ConfMonitorPtr monitor = XF86Config->conf_monitor_lst;

    if (ident_string == NULL || strlen(ident_string) == 0)
	return (False);

    bzero(mon_hsync, sizeof(parser_range) * CONF_MAX_HSYNC);
    bzero(mon_vrefresh, sizeof(parser_range) * CONF_MAX_VREFRESH);

    XtSetArg(args[0], XtNstring, &str);
    XtGetValues(hsync, args, 1);
    if ((mon_n_hsync = string_to_parser_range(str, mon_hsync,
					      CONF_MAX_HSYNC)) <= 0)
	return (False);

    XtSetArg(args[0], XtNstring, &str);
    XtGetValues(vsync, args, 1);
    if ((mon_n_vrefresh = string_to_parser_range(str, mon_vrefresh,
						 CONF_MAX_VREFRESH)) <= 0)
	return (False);

    while (monitor != NULL) {
	if (monitor != current_monitor &&
	    strcasecmp(ident_string, monitor->mon_identifier) == 0)
	    return (False);
	monitor = (XF86ConfMonitorPtr)(monitor->list.next);
    }

    return (True);
}

int
string_to_parser_range(char *str, parser_range *range, int nrange)
{
    double val;
    int i = 0;

    if (str == NULL || *str == '\0' || range == NULL || nrange == 0)
	return (0);

    while (*str) {
	while (*str && isspace(*str))
	    ++str;
	if (!isdigit(*str)) {
	    ++str;
	    continue;
	}
	val = strtod(str, &str);
	while (*str && isspace(*str))
	    ++str;
	if (*str == ',' || *str == '\0') {
	    if (*str)
		++str;
	    range[i].lo = range[i].hi = val;
	    if (++i >= nrange || *str == '\0')
		break;
	    continue;
	}
	else if (*str != '-')
	    return (0);
	++str;
	range[i].lo = val;
	while (*str && isspace(*str))
	    ++str;
	if ((range[i].hi = strtod(str, &str)) < range[i].lo)
	    return (0);
	if (++i >= nrange)
	    break;
    }

    return (i);
}

int
parser_range_to_string(char *str, parser_range *range, int nrange)
{
    int i, len;

    if (str == NULL || range == NULL || nrange <= 0)
	return (0);

    for (i = len = 0; i < nrange; i++) {
	if (i > 0)
	    len += XmuSnprintf(str + len, PARSER_RANGE_SIZE - len, "%s",
			       ", ");
	if (range[i].lo == range[i].hi)
	    len += XmuSnprintf(str + len, PARSER_RANGE_SIZE - len, "%g",
			       range[i].lo);
	else if (range[i].lo < range[i].hi)
	    len += XmuSnprintf(str + len, PARSER_RANGE_SIZE - len, "%g - %g",
			       range[i].lo, range[i].hi);
	else
	    return (0);
    }

    return (i);
}

/*ARGSUSED*/
static void
MonitorHsyncCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;
    Arg args[1];

    XtSetArg(args[0], XtNstring, hmodes_trans[info->list_index]);
    XtSetValues(hsync, args, 1);
}

/*ARGSUSED*/
static void
MonitorVsyncCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;
    Arg args[1];

    XtSetArg(args[0], XtNstring, info->string);
    XtSetValues(vsync, args, 1);
}

/*ARGSUSED*/
static void
MonitorSelectCardCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];

    card = (XF86ConfDevicePtr)user_data;
    XtSetArg(args[0], XtNlabel, card != NULL ? card->dev_identifier : "");
    XtSetValues(cmenu, args, 1);
}

void
MonitorLayout(XF86SetupInfo *info)
{
    static int first = 1, men;
    static Widget layout, menu;
    XF86ConfDevicePtr device = XF86Config->conf_device_lst;
    Widget sme;
    Arg args[1];
    char *menuname;

    if (first) {
	Widget viewport;

	first = 0;

	layout = XtCreateWidget("monitorl", formWidgetClass,
				configp, NULL, 0);
	XtCreateManagedWidget("hlabel", labelWidgetClass, layout, NULL, 0);
	hsync = XtVaCreateManagedWidget("hsync", asciiTextWidgetClass, layout,
					XtNeditType, XawtextEdit,
					NULL);
	viewport = XtCreateManagedWidget("hviewport", viewportWidgetClass,
					 layout, NULL, 0);
	hlist = XtVaCreateManagedWidget("hlist", listWidgetClass, viewport,
					XtNlist, hmodes,
					XtNnumberStrings, sizeof(hmodes) /
					sizeof(hmodes[0]), NULL);
	XtAddCallback(hlist, XtNcallback, MonitorHsyncCallback, NULL);

	XtCreateManagedWidget("vlabel", labelWidgetClass, layout, NULL, 0);
	vsync = XtVaCreateManagedWidget("vsync", asciiTextWidgetClass, layout,
					XtNeditType, XawtextEdit,
					NULL);
	viewport = XtCreateManagedWidget("vviewport", viewportWidgetClass,
					 layout, NULL, 0);
	vlist = XtVaCreateManagedWidget("vlist", listWidgetClass, viewport,
					XtNlist, vmodes,
					XtNnumberStrings, sizeof(vmodes) /
					sizeof(vmodes[0]), NULL);
	XtAddCallback(vlist, XtNcallback, MonitorVsyncCallback, NULL);

	XtCreateManagedWidget("clabel", labelWidgetClass, layout, NULL, 0);
	cmenu = XtCreateManagedWidget("cmenu", menuButtonWidgetClass,
				      layout, NULL, 0);

	XtRealizeWidget(layout);
    }

    if (menu != NULL)
	XtDestroyWidget(menu);

    /*
     * swaps names because XtDestroyWidget will only really destroy it
     * when the code returns to XtAppMainLoop
     */
    menuname = men & 1 ? "mena" : "menb";
    menu = XtCreatePopupShell(menuname, simpleMenuWidgetClass,
			      cmenu, NULL, 0);
    XtSetArg(args[0], XtNmenuName, menuname);
    XtSetValues(cmenu, args, 1);
    ++men;
    sme = XtVaCreateManagedWidget("none", smeBSBObjectClass, menu,
				  NULL);
    XtAddCallback(sme, XtNcallback, MonitorSelectCardCallback, NULL);

    while (device != NULL) {
	XF86ConfScreenPtr screen = XF86Config->conf_screen_lst;
	Widget sme;
	Bool sensitive = True;

	while (screen != NULL) {
	    if (screen->scrn_device == device) {
		sensitive = screen->scrn_monitor == NULL ||
			    screen->scrn_monitor == current_monitor;
		break;
	    }
	    screen = (XF86ConfScreenPtr)(screen->list.next);
	}
	sme = XtCreateManagedWidget(device->dev_identifier,
				    smeBSBObjectClass, menu,
				    NULL, 0);
	if (sensitive)
	    XtAddCallback(sme, XtNcallback, MonitorSelectCardCallback, device);
	XtSetSensitive(sme, sensitive);

	device = (XF86ConfDevicePtr)(device->list.next);
    }

    XtRealizeWidget(menu);

    XtChangeManagedSet(&current, 1, NULL, NULL, &layout, 1);
    current = layout;
}
