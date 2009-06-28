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
#include "mouse-cfg.h"
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/Viewport.h>
#include <X11/extensions/xf86misc.h>

/*
 * Prototypes
 */
static void MouseDeviceCallback(Widget, XtPointer, XtPointer);
static void MouseProtocolCallback(Widget, XtPointer, XtPointer);
static void MouseEmulateCallback(Widget, XtPointer, XtPointer);
static void MouseApplyCallback(Widget, XtPointer, XtPointer);
static Bool MouseConfigCheck(void);

/*
 * Initialization
 */
static struct MouseProtocol {
    char *name;
    int type;
} protocols[] = {
#ifdef __SCO__
    {"OsMouse",			MTYPE_AUTOMOUSE},
#endif
#ifdef WSCONS_SUPPORT
    {"wsmouse",			MTYPE_AUTOMOUSE},
#endif
    {"Auto",			MTYPE_AUTOMOUSE},
    {"SysMouse",		MTYPE_SYSMOUSE},
    {"MouseSystems",		MTYPE_MOUSESYS},
    {"BusMouse",		MTYPE_BUSMOUSE},
    {"PS/2",			MTYPE_PS_2},
    {"Microsoft",		MTYPE_MICROSOFT},
#ifndef __FreeBSD__
    {"ImPS/2",			MTYPE_IMPS2},
    {"ExplorerPS/2",		MTYPE_EXPPS2},
    {"GlidePointPS/2",		MTYPE_GLIDEPOINTPS2},
    {"MouseManPlusPS/2",	MTYPE_MMANPLUSPS2},
    {"NetMousePS/2",		MTYPE_NETPS2},
    {"NetScrollPS/2",		MTYPE_NETSCROLLPS2},
    {"ThinkingMousePS/2",	MTYPE_THINKINGPS2},
#endif
    {"AceCad",			MTYPE_ACECAD},
    {"GlidePoint",		MTYPE_GLIDEPOINT},
    {"IntelliMouse",		MTYPE_IMSERIAL},
    {"Logitech",		MTYPE_LOGITECH},
    {"MMHitTab",		MTYPE_MMHIT},
    {"MMSeries",		MTYPE_MMSERIES},
    {"MouseMan",		MTYPE_LOGIMAN},
    {"ThinkingMouse",		MTYPE_THINKING},
};

static Widget text;
static char *device, *protocol;
static Bool emulate;
static XF86ConfInputPtr current_input;

/*
 * Implementation
 */
/*ARGSUSED*/
XtPointer
MouseConfig(XtPointer config)
{
    XF86ConfInputPtr mouse = (XF86ConfInputPtr)config;
    XF86OptionPtr option;
    char mouse_name[32];
    Arg args[1];

    static char *Device = "Device", *Protocol = "Protocol",
		*Emulate3Buttons = "Emulate3Buttons",
		*Emulate3Timeout = "Emulate3Timeout";

    current_input = mouse;

    if (mouse != NULL) {
	emulate = xf86findOption(mouse->inp_option_lst,
				 Emulate3Buttons) != NULL;
	if ((option = xf86findOption(mouse->inp_option_lst, Device)) != NULL)
	    device = option->opt_val;
	else
	    device = NULL;
	if ((option = xf86findOption(mouse->inp_option_lst, Protocol)) != NULL)
	    protocol = option->opt_val;
	else
	    protocol = NULL;

	XtSetArg(args[0], XtNstring, mouse->inp_identifier);
	XtSetValues(ident_widget, args, 1);
    }
    else {
	XF86ConfInputPtr input = XF86Config->conf_input_lst;
	int nmouses = 0;

	while (input != NULL) {
	    if (strcasecmp(input->inp_driver, "mouse") == 0)
		++nmouses;
	    input = (XF86ConfInputPtr)(input->list.next);
	}
	do {
	    XmuSnprintf(mouse_name, sizeof(mouse_name), "Mouse%d", nmouses);
	    ++nmouses;
	} while (xf86findInput(mouse_name,
		 XF86Config->conf_input_lst));

	XtSetArg(args[0], XtNstring, mouse_name);
	XtSetValues(ident_widget, args, 1);

	emulate = True;
	device = NULL;
	protocol = NULL;
    }

    xf86info.cur_list = MOUSE;
    XtSetSensitive(back, xf86info.lists[MOUSE].cur_function > 0);
    XtSetSensitive(next, xf86info.lists[MOUSE].cur_function <
			 xf86info.lists[MOUSE].num_functions - 1);
    (xf86info.lists[MOUSE].functions[xf86info.lists[MOUSE].cur_function])
	(&xf86info);

    if (ConfigLoop(MouseConfigCheck) == True) {
	XtSetArg(args[0], XtNstring, &device);
	XtGetValues(text, args, 1);
	if (mouse == NULL) {
	    mouse = XtNew(XF86ConfInputRec);
	    mouse->list.next = NULL;
	    mouse->inp_identifier = XtNewString(ident_string);
	    mouse->inp_driver = XtNewString("mouse");
	    mouse->inp_option_lst = xf86newOption(XtNewString(Device),
						  XtNewString(device));
	    xf86addNewOption(mouse->inp_option_lst,
			     XtNewString(Protocol), XtNewString(protocol));
	    if (emulate) {
		xf86addNewOption(mouse->inp_option_lst,
			         XtNewString(Emulate3Buttons), NULL);
		xf86addNewOption(mouse->inp_option_lst,
			         XtNewString(Emulate3Timeout),
				 XtNewString("50"));
	    }
	    mouse->inp_comment = NULL;
	}
	else {
	    if ((option = xf86findOption(mouse->inp_option_lst, Device)) != NULL) {
		XtFree(option->opt_val);
		option->opt_val = XtNewString(device);
		XtFree(option->opt_comment);
	    }
	    else {
		if (mouse->inp_option_lst == NULL)
		    mouse->inp_option_lst = xf86newOption(XtNewString(Device),
							  XtNewString(device));
		else
		    xf86addNewOption(mouse->inp_option_lst,
				     XtNewString(Device), XtNewString(device));
	    }

	    if ((option = xf86findOption(mouse->inp_option_lst, Protocol)) != NULL) {
		XtFree(option->opt_val);
		option->opt_val = XtNewString(protocol);
		XtFree(option->opt_comment);
	    }
	    else
		xf86addNewOption(mouse->inp_option_lst,
				 XtNewString(Protocol), XtNewString(protocol));

	    if (emulate == False) {
		xf86removeOption(&(mouse->inp_option_lst), Emulate3Buttons);
		xf86removeOption(&(mouse->inp_option_lst), Emulate3Timeout);
	    }
	    else if (emulate) {
		xf86addNewOption(mouse->inp_option_lst,
				 XtNewString(Emulate3Buttons), NULL);
		xf86addNewOption(mouse->inp_option_lst,
				 XtNewString(Emulate3Timeout), XtNewString("50"));
	    }
	}
	if (strcasecmp(mouse->inp_identifier, ident_string))
	    xf86renameInput(XF86Config, mouse, ident_string);

	return ((XtPointer)mouse);
    }

    return (NULL);
}

static Bool
MouseConfigCheck(void)
{
    Arg args[1];
    XF86ConfInputPtr mouse = XF86Config->conf_input_lst;

    XtSetArg(args[0], XtNstring, &device);
    XtGetValues(text, args, 1);

    if (ident_string == NULL || strlen(ident_string) == 0 ||
	device == NULL || strlen(device) == 0 || protocol == NULL)
	return (False);
    while (mouse != NULL) {
	if (mouse != current_input &&
	    strcasecmp(ident_string, mouse->inp_identifier) == 0)
	    return (False);
	mouse = (XF86ConfInputPtr)(mouse->list.next);
    }

    return (True);
}

static void
MouseDeviceCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;
    Arg args[1];

    XtSetArg(args[0], XtNstring, info->string);
    XtSetValues((Widget)user_data, args, 1);
    XawTextSetInsertionPoint((Widget)user_data, strlen(info->string));
}

static void
MouseProtocolCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;

    protocol = info->string;
}

static void
MouseEmulateCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    emulate = (Bool)(long)call_data;
}

static void
MouseApplyCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i;
    XF86MiscMouseSettings mouse;

    XF86MiscGetMouseSettings(XtDisplay(w), &mouse);
    XtFree(mouse.device);

    if (mouse.baudrate == 0 || mouse.baudrate < 0 || mouse.baudrate > 9600 ||
	mouse.baudrate % 1200)
	mouse.baudrate = 1200;

    mouse.type = MTYPE_AUTOMOUSE;
    for (i = 0; i < sizeof(protocols) / sizeof(protocols[0]); i++)
	if (strcmp(protocols[i].name, protocol) == 0) {
	    mouse.type = protocols[i].type;
	    break;
	}

    mouse.emulate3buttons = emulate;
    mouse.flags |= MF_REOPEN;

    mouse.device = device;

    XFlush(XtDisplay(w));
    XF86MiscSetMouseSettings(XtDisplay(w), &mouse);
}

void
MouseDeviceAndProtocol(XF86SetupInfo *info)
{
    static int first = 1, ndevices;
    static Widget mouse_dp, listD, listP, emul3, apply;
    static char **devices;
    static char *dirs[] = {
	"/dev",
#ifdef __linux__
	"/dev/input"
#endif
    };
    static char *patterns[] = {
#ifdef WSCONS_SUPPORT
	"wsmouse",
#endif
	"cuaa",
	"mice",
	"mouse",
	"ps",
	"sysmouse",
	"ttyS",
    };
    Arg args[2];
    int i;

    if (first) {
	Widget viewport;
	struct dirent *ent;
	DIR *dir;
	char **list;
	int count;

	first = 0;

	mouse_dp = XtCreateWidget("mouseDP", formWidgetClass,
				  configp, NULL, 0);

	/* DEVICE */
	for (count = 0; count < sizeof(dirs) / sizeof(dirs[0]); count++) {
	    if ((dir = opendir(dirs[count])) != NULL) {
		int i, len, xlen = strlen(dirs[count]) + 2;

		(void)readdir(dir);
		(void)readdir(dir);
		while ((ent = readdir(dir)) != NULL) {
		    for (i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
			len = strlen(patterns[i]);

			if (strncmp(patterns[i], ent->d_name, len) == 0) {
			    len = strlen(ent->d_name) + xlen;

			    devices = (char**)XtRealloc((XtPointer)devices,
							sizeof(char*) * ++ndevices);
			    devices[ndevices - 1] = XtMalloc(len);
			    XmuSnprintf(devices[ndevices - 1], len, "%s/%s",
					dirs[count], ent->d_name);
			}
		    }
		}
		closedir(dir);
	    }
	}

	(void) XtCreateManagedWidget("labelD", labelWidgetClass,
				      mouse_dp, NULL, 0);
	text = XtVaCreateManagedWidget("device", asciiTextWidgetClass,
				       mouse_dp,
				       XtNeditType, XawtextEdit,
				       NULL);
	viewport = XtCreateManagedWidget("viewportD", viewportWidgetClass,
					 mouse_dp, NULL, 0);

	listD = XtVaCreateManagedWidget("listD", listWidgetClass,
					viewport,
					XtNlist, devices,
					XtNnumberStrings, ndevices,
					NULL);
	XtAddCallback(listD, XtNcallback, MouseDeviceCallback, (XtPointer)text);

	/* PROTOCOL */
	(void) XtCreateManagedWidget("labelP", labelWidgetClass,
				      mouse_dp, NULL, 0);
	viewport = XtCreateManagedWidget("viewportP", viewportWidgetClass,
					 mouse_dp, NULL, 0);

	list = (char**)XtMalloc(sizeof(char*) *
				sizeof(protocols)/sizeof(protocols[0]));
	for (count = 0; count < sizeof(protocols)/sizeof(protocols[0]); count++)
	    list[count] = XtNewString(protocols[count].name);
	listP = XtVaCreateManagedWidget("listP", listWidgetClass,
					viewport,
					XtNlist, list,
					XtNnumberStrings, count,
					NULL);
	XtAddCallback(listP, XtNcallback, MouseProtocolCallback, NULL);

	emul3 = XtVaCreateManagedWidget("emulate3", toggleWidgetClass,
					mouse_dp, XtNstate, True, NULL);
	XtAddCallback(emul3, XtNcallback, MouseEmulateCallback, NULL);
	apply = XtCreateManagedWidget("apply", commandWidgetClass,
				      mouse_dp, NULL, 0);
	XtAddCallback(apply, XtNcallback, MouseApplyCallback, NULL);

	XtRealizeWidget(mouse_dp);
    }

    if (device != NULL) {
	for (i = 0; i < ndevices; i++)
	    if (strcmp(device, devices[i]) == 0) {
		XtSetArg(args[0], XtNstring, device);
		XtSetValues(text, args, 1);
		XawListHighlight(listD, i);
		break;
	    }

	if (i >= ndevices) {
	    devices = (char**)XtRealloc((XtPointer)devices,
					sizeof(char*) * ++ndevices);
	    devices[ndevices - 1] = XtNewString(device);
	    XawListHighlight(listD, ndevices - 1);
	    XtSetArg(args[0], XtNlist, devices);
	    XtSetArg(args[1], XtNnumberStrings, ndevices);
	    XtSetValues(listD, args, 2);
	}
	device = devices[i];
    }
    else {
	XtSetArg(args[0], XtNstring, "");
	XtSetValues(text, args, 1);
	XawListUnhighlight(listD);
    }

    if (protocol != NULL) {
	for (i = 0; i < sizeof(protocols) / sizeof(protocols[0]); i++)
	    if (strcasecmp(protocol, protocols[i].name) == 0) {
		protocol = protocols[i].name;
		XawListHighlight(listP, i);
		break;
	    }
    }
    else {
	/* "Auto" is the default */
	protocol = protocols[0].name;
	XawListHighlight(listP, 0);
    }

    XtSetArg(args[0], XtNstate, emulate);
    XtSetValues(emul3, args, 1);

    XtChangeManagedSet(&current, 1, NULL, NULL, &mouse_dp, 1);
    current = mouse_dp;
}
