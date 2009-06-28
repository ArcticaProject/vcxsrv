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

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Composite.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/SimpleMenP.h>
#include <X11/Xaw/Dialog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "xf86config.h"
#include "mouse-cfg.h"
#include "keyboard-cfg.h"
#include "card-cfg.h"
#include "monitor-cfg.h"
#include "screen-cfg.h"
#include "screen.h"
#include "cards.h"
#include "options.h"
#include "vidmode.h"
#include "help.h"
#include "stubs.h"

#define randomize()		srand((unsigned)time((time_t*)NULL))
#ifdef PROJECT_ROOT
#define DefaultXFree86Dir	PROJECT_ROOT
#else
#define DefaultXFree86Dir	"/usr"
#endif

#define IS_KBDDRIV(S) ((strcasecmp((S),"kbd") == 0))

/*
 * Prototypes
 */
void DrawCables(void);
static void DrawCable(Display*, Window, int, int, int, int);
static void ComputerEventHandler(Widget, XtPointer, XEvent*, Boolean*);
void SelectDeviceAction(Widget, XEvent*, String*, Cardinal*);
void MoveDeviceAction(Widget, XEvent*, String*, Cardinal*);
void UnselectDeviceAction(Widget, XEvent*, String*, Cardinal*);
void RenameLayoutAction(Widget, XEvent*, String*, Cardinal*);
void DevicePopupMenu(Widget, XEvent*, String*, Cardinal*);
void DevicePopdownMenu(Widget, XEvent*, String*, Cardinal*);
void AddDeviceCallback(Widget, XtPointer, XtPointer);
void QuitCallback(Widget, XtPointer, XtPointer);
void SmeConfigureDeviceCallback(Widget, XtPointer, XtPointer);
void ConfigureDeviceCallback(Widget, XtPointer, XtPointer);
void EnableDeviceCallback(Widget, XtPointer, XtPointer);
void DisableDeviceCallback(Widget, XtPointer, XtPointer);
void RemoveDeviceCallback(Widget, XtPointer, XtPointer);
void InitializeDevices(void);
void SetConfigModeCallback(Widget, XtPointer, XtPointer);
void SelectLayoutCallback(Widget, XtPointer, XtPointer);
void DefaultLayoutCallback(Widget, XtPointer, XtPointer);
void RemoveLayoutCallback(Widget, XtPointer, XtPointer);
void OptionsCallback(Widget, XtPointer, XtPointer);
xf86cfgDevice *AddDevice(int, XtPointer, int, int);
static Bool AskConfig(void);
void WriteConfigAction(Widget, XEvent*, String*, Cardinal*);
static void ScreenSetup(Bool);
void QuitAction(Widget, XEvent*, String*, Cardinal*);
void PopdownErrorCallback(Widget, XtPointer, XtPointer);
static void ErrorCancelAction(Widget, XEvent*, String*, Cardinal*);
static void QuitCancelAction(Widget, XEvent*, String*, Cardinal*);
static void HelpCallback(Widget, XtPointer, XtPointer);
void UpdateMenuDeviceList(int);

extern void AccessXConfigureStart(void);
extern void AccessXConfigureEnd(void);
extern void CloseAccessXAction(Widget, XEvent*, String*, Cardinal*);

#ifdef HAS_NCURSES
extern void TextMode(void);
#endif

static void Usage(void);

/*
 * Initialization
 */
Widget toplevel, work, config, layout, layoutsme, layoutp, topMenu;
XtAppContext appcon;

Pixmap menuPixmap;

char *XF86Config_path = NULL;
char *XF86Module_path = NULL;
char *XFree86_path = NULL;
char *XF86Font_path = NULL;
char *XF86RGB_path = NULL;
char *XkbConfig_path = NULL;
char *XFree86Dir;
static char XF86Config_path_static[1024];
static char XkbConfig_path_static[1024];
Bool xf86config_set = False;

int textmode = False;
#ifdef USE_MODULES
int nomodules = False;
#endif
int  noverify = False;

xf86cfgComputer computer;
xf86cfgDevice cpu_device;
Cursor no_cursor;
static Widget device, layoutm, popup, commands;
static int xpos, ypos;
int sxpos, sypos;
static char no_cursor_data[] = { 0,0,0,0, 0,0,0,0 };
static GC cablegc, cablegcshadow;
Atom wm_delete_window;
static Bool config_set = False;
static Widget mouseSme, mouseMenu, keyboardSme, keyboardMenu,
       cardSme, cardMenu, monitorSme, monitorMenu;

int config_mode = CONFIG_LAYOUT;

static XtActionsRec actions[] = {
    {"filter-card", CardFilterAction},
    {"select-device", SelectDeviceAction},
    {"move-device", MoveDeviceAction},
    {"unselect-device", UnselectDeviceAction},
    {"device-popup", DevicePopupMenu},
    {"device-popdown", DevicePopdownMenu},
    {"rename-layout", RenameLayoutAction},
    {"write-config", WriteConfigAction},
    {"quit", QuitAction},
    {"vidmode-restore", VidmodeRestoreAction},
    {"config-cancel", ConfigCancelAction},
    {"options-cancel", OptionsCancelAction},
    {"error-cancel", ErrorCancelAction},
    {"quit-cancel", QuitCancelAction},
    {"addmode-cancel", CancelAddModeAction},
    {"accessx-close", CloseAccessXAction},
    {"testmode-cancel", CancelTestModeAction},
    {"help-close", HelpCancelAction},
    {"expert-close", ExpertCloseAction},
#ifdef USE_MODULES
    {"module-options-close", ModuleOptionsCancelAction},
#endif
};

static char *device_names[] = {
/* MOUSE	*/
    "mouse",
/* KEYBOARD	*/
    "keyboard",
/* CARD		*/
    "card",
/* MONITOR	*/
    "monitor",
/* SCREEN	*/
    "screen",
};

static XtResource appResources[] = {
#if 0
    {"config",  __XCONFIGFILE__,  XtRString, sizeof(char*),
      0, XtRString, "/etc/X11/"__XCONFIGFILE__},
#endif
    {"menuBitmap",  "MenuBitmap",  XtRString, sizeof(char*),
      0, XtRString, "menu10"},
};

static void
Usage(void)
{
    fprintf(stderr,
"Usage:\n"
"   xorgcfg [-option ...]\n"
"\n"
"Options:\n"
"   -config <"__XCONFIGFILE__">   Alternate configuration file.\n"
"   -modulepath <module-path>  "__XSERVERNAME__" modules location.\n"
"   -serverpath <server-path>  X server to start (if $DISPLAY is not defined).\n"
"   -fontpath   <font-path>    Font path for fonts.\n"
#ifdef HAS_NCURSES
"   -textmode                  Use this option for the text only interface.\n"
#endif
#ifdef USE_MODULES
"   -nomodules                 Use this option if xorgcfg is slow to start.\n"
"   -verbose <number>          Verbosity used in the loader (default 1).\n"
#endif
"   -verify                    Verify modules/options integrity.\n"
);

    exit(1);
}

/*
 * Implementation
 */
int
main(int argc, char *argv[])
{
    Widget pane, hpane, expert, popup, mouse, keyboard, card, monitor;
    Widget bottom, sme, smemodeline, help, quit, layopt;
    XColor color, tmp;
    Pixmap pixmap;
    XGCValues values;
    XF86ConfLayoutPtr lay;
    int i, startedx;
    char *menuPixmapPath = NULL;
    XrmValue from, to;

    if ((XFree86Dir = getenv("XWINHOME")) == NULL)
	XFree86Dir = DefaultXFree86Dir;

    chdir(XFree86Dir);

#ifdef USE_MODULES
    xf86Verbose = 1;
#endif
    noverify = True;

    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-config") == 0 ||
	    strcmp(argv[i], "-xorgconfig") == 0 ||
	    strcmp(argv[i], "-xf86config") == 0) {
	    if (i + 1 < argc) {
		XF86Config_path = argv[++i];
		config_set = True;
	    }
	} else if (strcmp(argv[i], "-modulepath") == 0) {
	    if (i + 1 < argc)
		XF86Module_path = argv[++i];
	} else if (strcmp(argv[i], "-serverpath") == 0) {
	    if (i + 1 < argc)
		XFree86_path = argv[++i];
	} else if (strcmp(argv[i], "-fontpath") == 0) {
	    if (i + 1 < argc)
		XF86Font_path = argv[++i];
        }
#ifdef HAS_NCURSES
	else if (strcmp(argv[i], "-textmode") == 0)
	    textmode = True;
#endif
#ifdef USE_MODULES
	else if (strcmp(argv[i], "-nomodules") == 0)
	    nomodules = True;
	else if (strcmp(argv[i], "-verbose") == 0) {
	    if (i + 1 < argc)
		xf86Verbose = atoi(argv[++i]);
	}
#endif
	else if (strcmp(argv[i], "-verify") == 0)
	    noverify = False;
	else
	    Usage();
    }

#ifdef HAS_NCURSES
    if (textmode) {
	TextMode();
	exit(0);
    }
#endif
    
    startedx = startx();
    if (XF86Config_path == NULL)
	XF86Config_path = XtNewString(__XCONFIGFILE__);
    if (XkbConfig_path == NULL) {
	XmuSnprintf(XkbConfig_path_static, sizeof(XkbConfig_path_static),
		    "%s/%s%s", XFree86Dir, XkbConfigDir, XkbConfigFile);
	XkbConfig_path = XkbConfig_path_static;
    }
    toplevel = XtAppInitialize(&appcon, "XOrgCfg",
		    	       NULL, 0,
			       &argc, argv,
			       NULL, NULL, 0);
    if (DPY == NULL)
	DPY = XtDisplay(toplevel);

    XtGetApplicationResources(toplevel, (XtPointer)&menuPixmapPath,
			      appResources, XtNumber(appResources), NULL, 0);
    if (menuPixmapPath && strlen(menuPixmapPath)) {
	from.size = strlen(menuPixmapPath);
	from.addr = menuPixmapPath;
	to.size = sizeof(Pixmap);
	to.addr = (XtPointer)&(menuPixmap);
	XtConvertAndStore(toplevel, XtRString, &from, XtRBitmap, &to);
    }

    XtAppAddActions(appcon, actions, XtNumber(actions));

    XawSimpleMenuAddGlobalActions(appcon);
    XtRegisterGrabAction(DevicePopupMenu, True,
			 ButtonPressMask | ButtonReleaseMask,
			 GrabModeAsync, GrabModeAsync);

    pane = XtCreateManagedWidget("pane", panedWidgetClass,
				 toplevel, NULL, 0);
    hpane = XtVaCreateManagedWidget("hpane", panedWidgetClass, pane,
				    XtNorientation, XtorientHorizontal, NULL);
    topMenu = XtCreateManagedWidget("topM", menuButtonWidgetClass,
				 hpane, NULL, 0);
    expert = XtCreateManagedWidget("expert", commandWidgetClass, hpane, NULL, 0);
    XtAddCallback(expert, XtNcallback, ExpertCallback, NULL);
    popup = XtCreatePopupShell("menu", simpleMenuWidgetClass,
			       topMenu, NULL, 0);
    sme = XtCreateManagedWidget("layout", smeBSBObjectClass,
				popup, NULL, 0);
    XtAddCallback(sme, XtNcallback, SetConfigModeCallback,
		  (XtPointer)CONFIG_LAYOUT);
    sme = XtCreateManagedWidget("screen", smeBSBObjectClass,
				popup, NULL, 0);
    XtAddCallback(sme, XtNcallback, SetConfigModeCallback,
		  (XtPointer)CONFIG_SCREEN);
    smemodeline = XtCreateManagedWidget("modeline", smeBSBObjectClass,
					popup, NULL, 0);
    XtAddCallback(smemodeline, XtNcallback, SetConfigModeCallback,
		  (XtPointer)CONFIG_MODELINE);
    sme = XtCreateManagedWidget("accessx", smeBSBObjectClass,
				popup, NULL, 0);
    XtAddCallback(sme, XtNcallback, SetConfigModeCallback,
		  (XtPointer)CONFIG_ACCESSX);

    commands = XtCreateManagedWidget("commands", formWidgetClass,
				     pane, NULL, 0);

    mouse = XtVaCreateManagedWidget("mouse", menuButtonWidgetClass,
				    commands, XtNmenuName, "mouseP", NULL);
    popup = XtCreatePopupShell("mouseP", simpleMenuWidgetClass,
			       mouse, NULL, 0);
    sme = XtCreateManagedWidget("new", smeBSBObjectClass,
				popup, NULL, 0);
    XtAddCallback(sme, XtNcallback, AddDeviceCallback, (XtPointer)MOUSE);
    mouseSme = XtCreateManagedWidget("configure", smeBSBObjectClass,
				     popup, NULL, 0);
    XtAddCallback(mouseSme, XtNcallback, SmeConfigureDeviceCallback,
		  (XtPointer)MOUSE);

    keyboard = XtVaCreateManagedWidget("keyboard", menuButtonWidgetClass,
				       commands, XtNmenuName, "keyboardP", NULL);
    popup = XtCreatePopupShell("keyboardP", simpleMenuWidgetClass,
			       keyboard, NULL, 0);
    sme = XtCreateManagedWidget("new", smeBSBObjectClass,
				popup, NULL, 0);
    XtAddCallback(sme, XtNcallback, AddDeviceCallback, (XtPointer)KEYBOARD);
    keyboardSme = XtCreateManagedWidget("configure", smeBSBObjectClass,
					popup, NULL, 0);
    XtAddCallback(keyboardSme, XtNcallback, SmeConfigureDeviceCallback,
		  (XtPointer)KEYBOARD);

    card = XtVaCreateManagedWidget("card", menuButtonWidgetClass,
				   commands, XtNmenuName, "cardP", NULL);
    popup = XtCreatePopupShell("cardP", simpleMenuWidgetClass,
			       card, NULL, 0);
    sme = XtCreateManagedWidget("new", smeBSBObjectClass,
				popup, NULL, 0);
    XtAddCallback(sme, XtNcallback, AddDeviceCallback, (XtPointer)CARD);
    cardSme = XtCreateManagedWidget("configure", smeBSBObjectClass,
				    popup, NULL, 0);
    XtAddCallback(cardSme, XtNcallback, SmeConfigureDeviceCallback,
		  (XtPointer)CARD);

    monitor = XtVaCreateManagedWidget("monitor", menuButtonWidgetClass,
				      commands, XtNmenuName, "monitorP", NULL);
    popup = XtCreatePopupShell("monitorP", simpleMenuWidgetClass,
			       monitor, NULL, 0);
    sme = XtCreateManagedWidget("new", smeBSBObjectClass,
				popup, NULL, 0);
    XtAddCallback(sme, XtNcallback, AddDeviceCallback, (XtPointer)MONITOR);
    monitorSme = XtCreateManagedWidget("configure", smeBSBObjectClass,
				       popup, NULL, 0);
    XtAddCallback(monitorSme, XtNcallback, SmeConfigureDeviceCallback,
		  (XtPointer)MONITOR);

    work = XtCreateManagedWidget("work", compositeWidgetClass,
				 pane, NULL, 0);

    bottom = XtCreateManagedWidget("bottom", formWidgetClass,
				   pane, NULL, 0);
    layoutm = XtCreateManagedWidget("select", menuButtonWidgetClass,
				    bottom, NULL, 0);
    layout = XtVaCreateManagedWidget("layout", asciiTextWidgetClass,
				     bottom,
				     XtNeditType, XawtextEdit,
				     NULL);
    layoutp = XtCreatePopupShell("menu", simpleMenuWidgetClass,
				 bottom, NULL, 0);
    sme = XtCreateManagedWidget("new", smeBSBObjectClass, layoutp,
				NULL, 0);
    XtAddCallback(sme, XtNcallback, SelectLayoutCallback, NULL);
    help = XtCreateManagedWidget("help", commandWidgetClass,
				 bottom, NULL, 0);
    XtAddCallback(help, XtNcallback, HelpCallback, NULL);
    quit = XtCreateManagedWidget("quit", commandWidgetClass,
				 bottom, NULL, 0);
    XtAddCallback(quit, XtNcallback, QuitCallback, NULL);

    XtRealizeWidget(toplevel);
    XtRealizeWidget(topMenu);

    pixmap = XCreateBitmapFromData(XtDisplay(toplevel), XtWindow(toplevel),
				   no_cursor_data, 8, 8);
    XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap, "black",
		     &color, &tmp);
    no_cursor = XCreatePixmapCursor(XtDisplay(toplevel), pixmap, pixmap,
				    &color, &color, 0, 0);

    XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap, "gray55",
		     &color, &tmp);
    values.line_width = 3;
    values.foreground = color.pixel;
    cablegcshadow = XCreateGC(XtDisplay(toplevel), XtWindow(toplevel),
			GCForeground | GCLineWidth, &values);
    XAllocNamedColor(XtDisplay(toplevel), toplevel->core.colormap, "gray85",
		     &color, &tmp);
    values.line_width = 1;
    values.foreground = color.pixel;
    cablegc = XCreateGC(XtDisplay(toplevel), XtWindow(toplevel),
			GCForeground | GCLineWidth, &values);

    computer.cpu = XtCreateManagedWidget("cpu", simpleWidgetClass,
					 work, NULL, 0);
    cpu_device.widget = computer.cpu;
    cpu_device.type = SERVER;

    XtAddEventHandler(work, ExposureMask, False,
		      ComputerEventHandler, (XtPointer)NULL);

    wm_delete_window = XInternAtom(DPY, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(DPY, XtWindow(toplevel), &wm_delete_window, 1);

    StartConfig();
    InitializeDevices();
    UpdateMenuDeviceList(MOUSE);
    UpdateMenuDeviceList(KEYBOARD);
    UpdateMenuDeviceList(CARD);
    UpdateMenuDeviceList(MONITOR);
    XtSetSensitive(smemodeline, VideoModeInitialize());

    lay = XF86Config->conf_layout_lst;
    while (lay != NULL) {
	sme = XtVaCreateManagedWidget("sme", smeBSBObjectClass,
				      layoutp,
				      XtNlabel, lay->lay_identifier,
				      XtNmenuName, lay->lay_identifier,
				      XtNleftBitmap, menuPixmap,
				      NULL);
	XtAddCallback(sme, XtNcallback, SelectLayoutCallback, (XtPointer)lay);
	if (layoutsme == NULL)
	    layoutsme = sme;
	layopt = XtCreatePopupShell(lay->lay_identifier, simpleMenuWidgetClass,
				    layoutp, NULL, 0);
	sme = XtCreateManagedWidget("default", smeBSBObjectClass,
				    layopt, NULL, 0);
	XtAddCallback(sme, XtNcallback, DefaultLayoutCallback, NULL);
	sme = XtCreateManagedWidget("remove", smeBSBObjectClass,
				    layopt, NULL, 0);
	XtAddCallback(sme, XtNcallback, RemoveLayoutCallback, NULL);
	XtRealizeWidget(layopt);

	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }
    SelectLayoutCallback(layoutsme,
			 XF86Config->conf_layout_lst, NULL);

    startaccessx();
    if (startedx) {
	switch (fork()) {
	    case 0: {
		char path[PATH_MAX];

		XmuSnprintf(path, sizeof(path), "%s/bin/twm", XFree86Dir);
		execl(path, "twm", (void *)NULL);
		exit(-127);
	    }	break;
	    case -1:
		fprintf(stderr, "Cannot fork.\n");
		exit(1);
		break;
	    default:
		break;
	}
    }

#ifdef USE_MODULES
    if (!nomodules)
	LoaderInitializeOptions();
#endif

    /* ReadCardsDatabase() must be called after LoaderInitializeOptions() */
    ReadCardsDatabase();

    if (!config_set && startedx) {
	XtFree(XF86Config_path);
#ifndef XF86CONFIG
# define XF86CONFIG __XCONFIGFILE__
#endif
#ifdef XF86CONFIGDIR
	XF86Config_path = XtNewString(XF86CONFIGDIR "/" XF86CONFIG);
#else
	XF86Config_path = XtNewString("/etc/X11/" XF86CONFIG);
#endif
    }
    XtAppMainLoop(appcon);
    if (startedx)
	endx();

    return (0);
}

static Widget shell_cf;
static int write_cf, asking_cf;
static int cf_state = 0;
#define	CF_XF86Config	1
#define	CF_XKBConfig	2
#define CF_First	CF_XF86Config
#define CF_Last		CF_XKBConfig

/*ARGSUSED*/
static void
WriteConfig(Widget w, XtPointer user_data, XtPointer call_data)
{
    asking_cf = 0;
    XtPopdown(shell_cf);
    write_cf = (long)user_data;
}

/*ARGSUSED*/
void
QuitCancelAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    WriteConfig(w, (XtPointer)-1, NULL);
}

/*ARGSUSED*/
void
WriteConfigAction(Widget w, XEvent *event,
		  String *params, Cardinal *num_params)
{
    WriteConfig(w, (XtPointer)True, NULL);
}

static Bool
AskConfig(void)
{
    static Widget dialog;

    if (shell_cf == NULL) {
	Arg args[1];
	char *l, *label;
	int len;

	shell_cf = XtCreatePopupShell("quit", transientShellWidgetClass,
				      toplevel, NULL, 0);
	dialog = XtVaCreateManagedWidget("ask", dialogWidgetClass, shell_cf,
					 XtNvalue, XF86Config_path, NULL);
	XawDialogAddButton(dialog, "yes", WriteConfig, (XtPointer)1);
	XawDialogAddButton(dialog, "no", WriteConfig, (XtPointer)0);
	XawDialogAddButton(dialog, "cancel", WriteConfig, (XtPointer)-1);
	XtRealizeWidget(shell_cf);
	XSetWMProtocols(DPY, XtWindow(shell_cf), &wm_delete_window, 1);
	XtSetArg(args[0], XtNlabel, &l);
	XtGetValues(dialog, args, 1);
	label = XtMalloc(len = (strlen(l) + strlen(XF86CONFIG) + 2));
	XmuSnprintf(label, len, "%s\n", XF86CONFIG);
	strcat(label, l);
	XtSetArg(args[0], XtNlabel, label);
	XtSetValues(dialog, args, 1);
	XtFree(label);
    }
    else {
	Arg args[2];
	Cardinal num_args = 0;
	char *l, *label = NULL, *str = "";

	XtSetArg(args[0], XtNlabel, &l);
	XtGetValues(dialog, args, 1);
	switch (cf_state) {
	    case CF_XF86Config:
		str = XF86CONFIG;
		XtSetArg(args[num_args], XtNvalue, XF86Config_path);
		++num_args;
		break;
	    case CF_XKBConfig:
		str = "XKB";
		XtSetArg(args[num_args], XtNvalue, XkbConfig_path);
		++num_args;
		break;
	}
	l = strchr(l, '\n');
	if (l != NULL) {
	    label = XtMalloc(strlen(str) + strlen(l) + 1);
	    strcpy(label, str);
	    strcat(label, l);
	    XtSetArg(args[num_args], XtNlabel, label);
	    ++num_args;
	}
	XtSetValues(dialog, args, num_args);
	if (l != NULL)
	    XtFree(label);
    }

    asking_cf = 1;

    XtPopup(shell_cf, XtGrabExclusive);
    while (asking_cf)
	XtAppProcessEvent(XtWidgetToApplicationContext(shell_cf), XtIMAll);

    if (write_cf > 0) {
	switch (cf_state) {
	    case CF_XF86Config:
		XF86Config_path = XawDialogGetValueString(dialog);
		XmuSnprintf(XF86Config_path_static,
			    sizeof(XF86Config_path_static),
			    "%s", XF86Config_path);
		XF86Config_path = XF86Config_path_static;
		break;
	    case CF_XKBConfig:
		XkbConfig_path = XawDialogGetValueString(dialog);
		XmuSnprintf(XkbConfig_path_static,
			    sizeof(XkbConfig_path_static),
			    "%s", XkbConfig_path);
		XkbConfig_path = XkbConfig_path_static;
		break;
	}
    }

    return (write_cf);
}

/*ARGSUSED*/
void
PopdownErrorCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XtPopdown((Widget)user_data);
}

/*ARGSUSED*/
void
ErrorCancelAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XtPopdown((Widget)w);
}

/*ARGSUSED*/
void
QuitAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    QuitCallback(w, NULL, NULL);
}

/*ARGSUSED*/
void
QuitCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    for (cf_state = CF_First; cf_state <= CF_Last; cf_state++) {
	if (cf_state == CF_XKBConfig && xkb_info == NULL)
	    continue;
	
	switch (AskConfig()) {
	    case 0:
		break;
	    case 1:
		if ((cf_state == CF_XF86Config &&
		     !xf86writeConfigFile(XF86Config_path, XF86Config)) ||
		    (cf_state == CF_XKBConfig &&
		     !WriteXKBConfiguration(XkbConfig_path,
					    &xkb_info->config))) {
		    static Widget shell;

		    if (shell == NULL) {
			Widget dialog;

			shell = XtCreatePopupShell("error",
				transientShellWidgetClass,
				toplevel, NULL, 0);
			dialog = XtVaCreateManagedWidget("notice",
				 dialogWidgetClass,
				 shell, XtNvalue, NULL,
				 NULL);
			XawDialogAddButton(dialog, "ok", PopdownErrorCallback,
					   (XtPointer)shell);
			XtRealizeWidget(shell);
			XSetWMProtocols(DPY, XtWindow(shell),
					&wm_delete_window, 1);
		    }
		    XtPopup(shell, XtGrabExclusive);
		    return;
		}
		break;
	    default:
		return;
	}
    }

    endx();
    exit(0);
}

void
InitializeDevices(void)
{
    xf86cfgDevice *device;
    int mouse_x, mouse_y, keyboard_x, keyboard_y,
	card_x, card_y, monitor_x, monitor_y, len;
    XF86ConfInputPtr input = XF86Config->conf_input_lst;
    XF86ConfDevicePtr card = XF86Config->conf_device_lst;
    XF86ConfMonitorPtr monitor = XF86Config->conf_monitor_lst;
    XF86OptionPtr flags = NULL;
    char buffer[4096], *tip;
    Arg args[1];

    if (XF86Config->conf_flags != NULL)
	flags = XF86Config->conf_flags->flg_option_lst;

    len = 0;
    while (flags && len < sizeof(buffer) - 1) {
	len += XmuSnprintf(buffer + len, sizeof(buffer) - len,
			   "Option     \"%s\"",
			   flags->opt_name);
	if (flags->opt_val != NULL)
	    len += XmuSnprintf(buffer + len, sizeof(buffer) - len,
			       " \"%s\"\n",
			       flags->opt_val);
	else
	    len += XmuSnprintf(buffer + len, sizeof(buffer) - len,
			       "%s", "\n");
	flags = (XF86OptionPtr)(flags->list.next);
    }

    if (len) {
	tip = XtNewString(buffer);
	XtSetArg(args[0], XtNtip, tip);
	XtSetValues(computer.cpu, args, 1);
    }

#define DEFAULT_MOUSE_WIDTH	30
#define DEFAULT_MOUSE_HEIGHT	40
#define DEFAULT_KEYBOARD_WIDTH	48
#define DEFAULT_KEYBOARD_HEIGHT	36
    mouse_x = work->core.width - (work->core.width >> 2);
    mouse_y = work->core.height - DEFAULT_MOUSE_HEIGHT;
    keyboard_x = 6;
    keyboard_y = work->core.height - DEFAULT_KEYBOARD_HEIGHT;

    while (input != NULL) {
	if (input->inp_driver) {
	    if (strcasecmp(input->inp_driver, "mouse") == 0) {
		device = AddDevice(MOUSE, (XtPointer)input, mouse_x, mouse_y);
		SetTip(device);
		if ((mouse_x += DEFAULT_MOUSE_WIDTH) > work->core.width) {
		    if ((mouse_y -= DEFAULT_MOUSE_HEIGHT) < (work->core.height >> 1))
			mouse_y = work->core.height >> 1;
		    mouse_x = work->core.width - (work->core.width >> 2);
		}
	    }
	    else if (IS_KBDDRIV(input->inp_driver)) {
		device = AddDevice(KEYBOARD, (XtPointer)input, keyboard_x, keyboard_y);
		SetTip(device);
		if ((keyboard_x += DEFAULT_KEYBOARD_WIDTH) >
		    work->core.width - (work->core.width >> 2))  {
		    if ((keyboard_y -= DEFAULT_KEYBOARD_HEIGHT) < (work->core.height >> 1))
			keyboard_y = work->core.height >> 1;
		    keyboard_x = 6;
		}
	    }
	}
	input = (XF86ConfInputPtr)(input->list.next);
    }

#define DEFAULT_CARD_WIDTH	45
#define DEFAULT_CARD_HEIGHT	46
    card_x = 6;
    card_y = (work->core.height >> 1) - 20 - DEFAULT_CARD_HEIGHT;
    while (card != NULL) {
	device = AddDevice(CARD, (XtPointer)card, card_x, card_y);
	SetTip(device);
	if ((card_x += DEFAULT_CARD_WIDTH) > work->core.width) {
	    if ((card_y -= DEFAULT_CARD_HEIGHT) < (work->core.height >> 2))
		card_y = work->core.height >> 2;
	    card_x = 6;
	}
	card = (XF86ConfDevicePtr)(card->list.next);
    }

#define DEFAULT_MONITOR_WIDTH	48
#define DEFAULT_MONITOR_HEIGHT	48
    monitor_x = 6;
    monitor_y = 6;
    while (monitor != NULL) {
	XF86ConfScreenPtr screen = XF86Config->conf_screen_lst;

	device = AddDevice(MONITOR, (XtPointer)monitor, monitor_x, monitor_y);
	SetTip(device);
	if ((monitor_x += DEFAULT_MONITOR_WIDTH) > work->core.width) {
	    if ((monitor_y += DEFAULT_MONITOR_HEIGHT) >
		(work->core.height >> 2) - DEFAULT_MONITOR_HEIGHT)
		monitor_y = (work->core.height >> 2) - DEFAULT_MONITOR_HEIGHT;
	    monitor_x = 6;
	}

	while (screen != NULL) {
	    if (screen->scrn_monitor == monitor) {
		card = XF86Config->conf_device_lst;
		while (card != NULL) {
		    if (screen->scrn_device == card) {
			xf86cfgScreen *scr = (xf86cfgScreen*)
			    XtCalloc(1, sizeof(xf86cfgScreen));
			int i;

			for (i = 0; i < computer.num_devices; i++)
			    if ((XF86ConfDevicePtr)(computer.devices[i]->config)
				== card)
				break;
			scr->screen = screen;
			scr->card = computer.devices[i];
			scr->monitor = device;
			scr->refcount = 0;
			++scr->card->refcount;
			++scr->monitor->refcount;
			computer.screens = (xf86cfgScreen**)
				XtRealloc((XtPointer)computer.screens,
					  sizeof(xf86cfgScreen*) *
					  (computer.num_screens + 1));
			CreateScreenWidget(scr);
			scr->type = SCREEN;
			computer.screens[computer.num_screens++] = scr;
			SetTip((xf86cfgDevice*)scr);
			break;
		    }
		    card = (XF86ConfDevicePtr)(card->list.next);
		}
		device->state = USED;
	    }
	    screen = (XF86ConfScreenPtr)(screen->list.next);
	}

	monitor = (XF86ConfMonitorPtr)(monitor->list.next);
    }
}

xf86cfgDevice *
AddDevice(int type, XtPointer config, int x, int y)
{
    switch (type) {
	case MOUSE:
	case KEYBOARD:
	case CARD:
	case MONITOR:
	    computer.devices = (xf86cfgDevice**)
		XtRealloc((XtPointer)computer.devices,
			  sizeof(xf86cfgDevice*) * (computer.num_devices + 1));
	    computer.devices[computer.num_devices] = (xf86cfgDevice*)
		XtCalloc(1, sizeof(xf86cfgDevice));
	    computer.devices[computer.num_devices]->config = config;
	    computer.devices[computer.num_devices]->widget =
		XtVaCreateManagedWidget(device_names[type], simpleWidgetClass,
					work,
					XtNx, x,
					XtNy, y,
					XtNtip, NULL,
					NULL);
	    computer.devices[computer.num_devices]->type = type;
	    computer.devices[computer.num_devices]->state = UNUSED;
	    computer.devices[computer.num_devices]->refcount = 0;
	    ++computer.num_devices;
	    break;
	default:
	    fprintf(stderr, "Bad argument to AddDevice.\n");
	    exit(1);
	    return (NULL);
    }

    UpdateMenuDeviceList(type);

    return (computer.devices[computer.num_devices - 1]);
}

/*ARGSUSED*/
static void
HelpCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    char *topic = NULL;

    switch (config_mode) {
	case CONFIG_LAYOUT:
	    topic = HELP_DEVICES;
	    break;
	case CONFIG_SCREEN:
	    topic = HELP_SCREEN;
	    break;
	case CONFIG_MODELINE:
	    topic = HELP_MODELINE;
	    break;
	case CONFIG_ACCESSX:
	    topic = HELP_ACCESSX;
	    break;
    }
    Help(topic);
}

void
SelectLayoutCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i, j;
    XF86ConfLayoutPtr lay = (XF86ConfLayoutPtr)user_data;
    XF86ConfInputrefPtr input;
    XF86ConfAdjacencyPtr adj;
    Widget sme, layopt;
    Arg args[1];
    char *str;

			       /* XXX Needs to check computer.layout,
				* because this function should also create
				* a new layout...
				*/
    if (lay == computer.layout && computer.layout)
	return;

    if (computer.layout != NULL) {
	for (i = 0; i < computer.num_layouts; i++) {
	    if (computer.layouts[i]->layout == computer.layout)
		break;
	}
	if (i < computer.num_layouts) {
	    XtFree((XtPointer)computer.layouts[i]->screen);
	    XtFree((XtPointer)computer.layouts[i]->position);
	}
	else {
	    computer.layouts = (xf86cfgLayout**)
		XtRealloc((XtPointer)computer.layouts, sizeof(xf86cfgLayout*) *
			  (computer.num_layouts + 1));
	    ++computer.num_layouts;
	}
	computer.layouts[i] = (xf86cfgLayout*)XtCalloc(1, sizeof(xf86cfgLayout));
	computer.layouts[i]->layout = computer.layout;
	computer.layouts[i]->num_layouts = computer.num_screens;
	computer.layouts[i]->screen = (xf86cfgScreen**)
	    XtMalloc(sizeof(xf86cfgScreen*) * computer.num_screens);
	computer.layouts[i]->position = (XPoint*)
	    XtMalloc(sizeof(XPoint) * computer.num_screens);
	for (j = 0; j < computer.num_screens; j++) {
	    computer.layouts[i]->screen[j] = computer.screens[j];
	    computer.layouts[i]->position[j].x = computer.screens[j]->widget->core.x;
	    computer.layouts[i]->position[j].y = computer.screens[j]->widget->core.y;
	}
    }

    if (lay != NULL) {
	for (i = 0; i < computer.num_layouts; i++)
	    if (computer.layouts[i]->layout == lay) {
		for (j = 0; j < computer.layouts[i]->num_layouts; j++) {
		    int k;

		    for (k = 0; k < computer.num_screens; k++)
			if (computer.screens[k] == computer.layouts[i]->screen[j]) {
			    XtMoveWidget(computer.screens[k]->widget,
					 computer.layouts[i]->position[j].x,
					 computer.layouts[i]->position[j].y);
			}
		}
		break;
	    }

	layoutsme = w;
	XtSetArg(args[0], XtNlabel, &str);
	XtGetValues(w, args, 1);
	XtSetArg(args[0], XtNstring, str);
	XtSetValues(layout, args, 1);
    }

    computer.layout = lay;

    for (i = 0; i < computer.num_devices; i++)
	computer.devices[i]->state = UNUSED;
    for (i = 0; i < computer.num_screens; i++)
	computer.screens[i]->state = UNUSED;

    if (lay == NULL) {
	char name[64];
	XF86ConfLayoutPtr l;
	int num_layouts = 0;

	l = XF86Config->conf_layout_lst;
	while (l != NULL) {
	    if (l->lay_adjacency_lst == NULL &&
		l->lay_inactive_lst == NULL &&
		l->lay_input_lst == NULL &&
		l->lay_option_lst == NULL &&
		l->lay_comment == NULL) {
		for (i = 0;
		     i < ((CompositeWidget)layout)->composite.num_children; i++)
		    if (strcmp(XtName(((CompositeWidget)layout)->composite.
			       children[i]), l->lay_identifier) == 0) {
			layoutsme = ((CompositeWidget)layout)->composite.children[i];
		    }
		computer.layout = l;
		XtSetArg(args[0], XtNstring, l->lay_identifier);
		XtSetValues(layout, args, 1);
		if (config_mode == CONFIG_LAYOUT)
		    DrawCables();
		if (config_mode == CONFIG_SCREEN)
		    ScreenSetup(True);
		return;
	    }
	    ++num_layouts;
	    l = (XF86ConfLayoutPtr)(l->list.next);
	}
	do {
	    XmuSnprintf(name, sizeof(name), "Layout%d", num_layouts);
	    ++num_layouts;
	} while (xf86findLayout(name,
		 XF86Config->conf_layout_lst) != NULL);
	l = (XF86ConfLayoutPtr)XtCalloc(1, sizeof(XF86ConfLayoutRec));

	l->lay_identifier = XtNewString(name);
	XF86Config->conf_layout_lst =
	    xf86addLayout(XF86Config->conf_layout_lst, l);
	layoutsme = XtVaCreateManagedWidget("sme", smeBSBObjectClass,
					    layoutp,
					    XtNlabel, name,
					    XtNmenuName, l->lay_identifier,
					    XtNleftBitmap, menuPixmap,
					    NULL);
	XtAddCallback(layoutsme, XtNcallback,
		      SelectLayoutCallback, (XtPointer)l);

	layopt = XtCreatePopupShell(l->lay_identifier, simpleMenuWidgetClass,
				    layoutp, NULL, 0);
	sme = XtCreateManagedWidget("default", smeBSBObjectClass,
				    layopt, NULL, 0);
	XtAddCallback(sme, XtNcallback, DefaultLayoutCallback, NULL);
	sme = XtCreateManagedWidget("remove", smeBSBObjectClass,
				    layopt, NULL, 0);
	XtAddCallback(sme, XtNcallback, RemoveLayoutCallback, NULL);
	XtRealizeWidget(layopt);

	computer.layout = l;
	XtSetArg(args[0], XtNstring, name);
	XtSetValues(layout, args, 1);
	if (config_mode == CONFIG_LAYOUT)
	    DrawCables();
	if (config_mode == CONFIG_SCREEN)
	    ScreenSetup(True);
	return;
    }

    input = lay->lay_input_lst;
    adj = lay->lay_adjacency_lst;

    for (i = 0; i < computer.num_devices; i++)
	if (computer.devices[i]->config != NULL &&
	    (computer.devices[i]->type == MOUSE ||
	     computer.devices[i]->type == KEYBOARD)) {
	    while (input != NULL) {
		if (strcmp(input->iref_inputdev_str, ((XF86ConfInputPtr)
		    (computer.devices[i]->config))->inp_identifier) == 0) {
		    computer.devices[i]->state = USED;
		    break;
		}
		input = (XF86ConfInputrefPtr)(input->list.next);
	    }
	    input = lay->lay_input_lst;
	}

    for (i = 0; i < computer.num_devices; i++)
	if (computer.devices[i]->type == CARD) {
	    while (adj != NULL) {
		XF86ConfScreenPtr screen = adj->adj_screen;

		if (computer.devices[i]->config != NULL &&
		    strcmp(screen->scrn_device_str, ((XF86ConfDevicePtr)
		    (computer.devices[i]->config))->dev_identifier) == 0) {
		    int j;

		    for (j = 0; j < computer.num_screens; j++)
			if (computer.screens[j]->card == computer.devices[i])
			    break;
		    computer.screens[j]->card->state = USED;
		    if (computer.screens[j]->monitor != NULL)
			computer.screens[j]->monitor->state = USED;
		    computer.screens[j]->state = USED;
		}

		adj = (XF86ConfAdjacencyPtr)(adj->list.next);
	    }
	    adj = lay->lay_adjacency_lst;
	}

    if (config_mode == CONFIG_LAYOUT)
	DrawCables();
    else if (config_mode == CONFIG_SCREEN)
	ScreenSetup(True);
}

/*ARGSUSED*/
void
DefaultLayoutCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Widget layopt, sme;
    int i;
    char *str;
    XF86ConfLayoutPtr prev, tmp, lay;

    str = w && XtParent(w) ? XtName(XtParent(w)) : NULL;
    if (str == NULL)
	return;

    prev = XF86Config->conf_layout_lst;
    lay = xf86findLayout(str, prev);
    if (prev == lay)
	return;

    tmp = prev;
    while (tmp != NULL) {
	if (tmp == lay)
	    break;
	prev = tmp;
	tmp = (XF86ConfLayoutPtr)(tmp->list.next);
    }

    for (i = 1; i < ((CompositeWidget)layoutp)->composite.num_children; i++)
	XtDestroyWidget(((CompositeWidget)layoutp)->composite.children[i]);
    for (i = 0; i < layoutp->core.num_popups; i++)
	XtDestroyWidget(layoutp->core.popup_list[i]);

    prev->list.next = lay->list.next;
    lay->list.next = XF86Config->conf_layout_lst;
    XF86Config->conf_layout_lst = lay;

    layoutsme = NULL;
    lay = XF86Config->conf_layout_lst;
    while (lay != NULL) {
	sme = XtVaCreateManagedWidget("sme", smeBSBObjectClass,
				      layoutp,
				      XtNlabel, lay->lay_identifier,
				      XtNmenuName, lay->lay_identifier,
				      XtNleftBitmap, menuPixmap,
				      NULL);
	XtAddCallback(sme, XtNcallback, SelectLayoutCallback, (XtPointer)lay);
	if (layoutsme == NULL)
	    layoutsme = sme;
	layopt = XtCreatePopupShell(lay->lay_identifier, simpleMenuWidgetClass,
				    layoutp, NULL, 0);
	sme = XtCreateManagedWidget("default", smeBSBObjectClass,
				    layopt, NULL, 0);
	XtAddCallback(sme, XtNcallback, DefaultLayoutCallback, NULL);
	sme = XtCreateManagedWidget("remove", smeBSBObjectClass,
				    layopt, NULL, 0);
	XtAddCallback(sme, XtNcallback, RemoveLayoutCallback, NULL);
	XtRealizeWidget(layopt);

	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }
    SelectLayoutCallback(layoutsme,
			 XF86Config->conf_layout_lst, NULL);
}

/*ARGSUSED*/
void
RemoveLayoutCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XF86ConfLayoutPtr prev, tmp, lay, rem;
    Widget sme = NULL;
    int i;
    char *str;
    Arg args[1];

    str = w && XtParent(w) ? XtName(XtParent(w)) : NULL;
    if (str == NULL)
	return;

    prev = XF86Config->conf_layout_lst;
    lay = xf86findLayout(str, prev);
    tmp = prev;
    while (tmp != NULL) {
	if (tmp == lay)
	    break;
	prev = tmp;
	tmp = (XF86ConfLayoutPtr)(tmp->list.next);
    }

    rem = lay;
    if (tmp != NULL)
	lay = (XF86ConfLayoutPtr)(tmp->list.next);
    if (lay == NULL && prev != tmp)
	lay = prev;

    if (lay != NULL) {
	int i;

	for (i = 0; i < ((CompositeWidget)layoutp)->composite.num_children;
	    i++) {
	    XtSetArg(args[0], XtNlabel, &str);
	    XtGetValues(((CompositeWidget)layoutp)->composite.children[i],
			args, 1);
	    if (strcmp(lay->lay_identifier, str) == 0) {
		layoutsme = ((CompositeWidget)layoutp)->composite.children[i];
		break;
	    }
	}
	SelectLayoutCallback(layoutsme, lay, NULL);
    }
    else {
	computer.layout = NULL;
	XtSetArg(args[0], XtNstring, "");
	XtSetValues(layout, args, 1);

	for (i = 0; i < computer.num_devices; i++)
	    computer.devices[i]->state = UNUSED;
	DrawCables();
    }

    for (i = 0; i < ((CompositeWidget)layoutp)->composite.num_children; i++) {
	XtSetArg(args[0], XtNlabel, &str);
	XtGetValues(((CompositeWidget)layoutp)->composite.children[i], args, 1);
	if (strcmp(rem->lay_identifier, str) == 0) {
	    sme = ((CompositeWidget)layoutp)->composite.children[i];
	    break;
	}
    }

    xf86removeLayout(XF86Config, rem);
    if (sme)
	XtDestroyWidget(sme);
}

void
SetTip(xf86cfgDevice *device)
{
    XF86OptionPtr option = NULL;
    char *tip, buffer[4096];
    Arg args[1];
    int len = 0;

    XtSetArg(args[0], XtNtip, &tip);
    XtGetValues(device->widget, args, 1);

    switch (device->type) {
	case MOUSE: {
	    XF86ConfInputPtr mouse = (XF86ConfInputPtr)device->config;

	    if (mouse == NULL)
		return;
	    len = XmuSnprintf(buffer, sizeof(buffer),
			      "Identifier \"%s\"\n"
			      "Driver     \"mouse\"\n",
			      mouse->inp_identifier);
	    option = mouse->inp_option_lst;
	}   break;
	case KEYBOARD: {
    	    XF86ConfInputPtr keyboard = (XF86ConfInputPtr)device->config;

	    if (keyboard == NULL)
		return;
	    len = XmuSnprintf(buffer, sizeof(buffer),
			      "Identifier \"%s\"\n"
			      "Driver     \"keyboard\"\n",
			      keyboard->inp_identifier);
	    option = keyboard->inp_option_lst;
	}   break;
	case CARD: {
	    XF86ConfDevicePtr card = (XF86ConfDevicePtr)device->config;

	    if (card == NULL)
		return;
	    len = XmuSnprintf(buffer, sizeof(buffer),
			      "Identifier \"%s\"\n"
			      "Driver     \"%s\"\n",
			      card->dev_identifier,
			      card->dev_driver);
	    option = card->dev_option_lst;
	}   break;
	case MONITOR: {
	    XF86ConfMonitorPtr monitor = (XF86ConfMonitorPtr)device->config;

	    if (monitor == NULL)
		return;
	    if (monitor->mon_vendor != NULL)
		len = XmuSnprintf(buffer, sizeof(buffer),
				  "Identifier \"%s\"\n"
				  "Vendor     \"%s\"\n",
				  monitor->mon_identifier,
				  monitor->mon_vendor);
	    else
		len = XmuSnprintf(buffer, sizeof(buffer),
				  "Identifier \"%s\"\n",
				  monitor->mon_identifier);
	    option = monitor->mon_option_lst;
	}   break;
	case SCREEN: {
	    XF86ConfScreenPtr screen = (XF86ConfScreenPtr)device->config;

	    if (screen == NULL)
		return;
	    len = XmuSnprintf(buffer, sizeof(buffer),
			      "Identifier \"%s\"\n",
			      screen->scrn_identifier);
	    if (screen->scrn_device_str != NULL)
		len += XmuSnprintf(buffer + len, sizeof(buffer),
				   "Device     \"%s\"\n",
				   screen->scrn_device_str);
	    if (screen->scrn_monitor_str != NULL)
		len += XmuSnprintf(buffer + len, sizeof(buffer),
				   "Monitor    \"%s\"\n",
				   screen->scrn_monitor_str);
	    option = screen->scrn_option_lst;
	}   break;
	case SERVER: {
	    len = XmuSnprintf(buffer, sizeof(buffer),
			      "%s\n", "Server Flags");
	    option = XF86Config->conf_flags->flg_option_lst;
	}   break;
    }

    while (option && len < sizeof(buffer) - 1) {
	len += XmuSnprintf(buffer + len, sizeof(buffer) - len,
			   "Option     \"%s\"",
			   option->opt_name);
	if (option->opt_val != NULL)
	    len += XmuSnprintf(buffer + len, sizeof(buffer) - len,
			       " \"%s\"\n",
			       option->opt_val);
	else
	    len += XmuSnprintf(buffer + len, sizeof(buffer) - len,
			       "%s", "\n");
	option = (XF86OptionPtr)(option->list.next);
    }

    tip = buffer;
    XtSetArg(args[0], XtNtip, tip);
    XtSetValues(device->widget, args, 1);
}

/*ARGSUSED*/
void
AddDeviceCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    AddDevice((long)user_data, NULL, 6, 6);
}

void
SmeConfigureDeviceCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i;

    switch ((long)user_data) {
	case MOUSE:
	case KEYBOARD:
	case CARD:
	case MONITOR:
	    for (i = 0; i < computer.num_devices; i++)
		if (computer.devices[i]->type == (long)user_data) {
		    config = computer.devices[i]->widget;
		    ConfigureDeviceCallback(w, NULL, NULL);
		}
	    break;

	/* hack for newly added devices */
	case -(MOUSE + 100):
	case -(KEYBOARD + 100):
	case -(CARD + 100):
	case -(MONITOR + 100):
	    for (i = 0; i < computer.num_devices; i++)
		if (-(computer.devices[i]->type + 100) == (long)user_data &&
		    computer.devices[i]->config == NULL) {
		    config = computer.devices[i]->widget;
		    ConfigureDeviceCallback(w, NULL, NULL);
		}
	    break;

	default:
	    for (i = 0; i < computer.num_devices; i++)
		if (computer.devices[i]->config == user_data) {
		    config = computer.devices[i]->widget;
		    ConfigureDeviceCallback(w, NULL, NULL);
		}
	    break;
    }
}

void
ConfigureDeviceCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i, j;

    if (config_mode == CONFIG_LAYOUT) {
	for (i = 0; i < computer.num_devices; i++) {
	    if (computer.devices[i]->widget == config) {
		switch (computer.devices[i]->type) {
		    case MOUSE: {
			XF86ConfInputPtr mouse =
			    MouseConfig(computer.devices[i]->config);

			if (mouse != NULL && computer.devices[i]->config == NULL) {
			    XF86Config->conf_input_lst =
				xf86addInput(XF86Config->conf_input_lst,
					     mouse);
			    computer.devices[i]->config = (XtPointer)mouse;
			}
			SetTip(computer.devices[i]);
		    }	break;
		    case KEYBOARD: {
			XF86ConfInputPtr keyboard =
			    KeyboardConfig(computer.devices[i]->config);

			if (keyboard != NULL && computer.devices[i]->config == NULL) {
			    XF86Config->conf_input_lst =
				xf86addInput(XF86Config->conf_input_lst,
					     keyboard);
			    computer.devices[i]->config = (XtPointer)keyboard;
			}
			SetTip(computer.devices[i]);
		    }	break;
		    case CARD: {
			XF86ConfDevicePtr card =
			    CardConfig(computer.devices[i]->config);

			if (card != NULL && computer.devices[i]->config == NULL) {
			    XF86Config->conf_device_lst =
				xf86addDevice(XF86Config->conf_device_lst,
					      card);
			    computer.devices[i]->config = (XtPointer)card;
			}
			SetTip(computer.devices[i]);
			for (j = 0; j < computer.num_screens; j++)
			    if (computer.screens[j]->card->widget == config)
				SetTip((xf86cfgDevice*)computer.screens[j]);
		    }	break;
		    case MONITOR: {
			XF86ConfMonitorPtr monitor =
			    MonitorConfig(computer.devices[i]->config);

			if (monitor != NULL && computer.devices[i]->config == NULL) {
			    XF86Config->conf_monitor_lst =
				xf86addMonitor(XF86Config->conf_monitor_lst,
					       monitor);
			    computer.devices[i]->config = (XtPointer)monitor;
			}
			SetTip(computer.devices[i]);
			for (j = 0; j < computer.num_screens; j++)
			    if (computer.screens[j]->monitor->widget == config)
				SetTip((xf86cfgDevice*)computer.screens[j]);
		    }	break;
		}
		/* Need to update because it may have been renamed */
		UpdateMenuDeviceList(computer.devices[i]->type);
		break;
	    }
	}
    }
    else if (config_mode == CONFIG_SCREEN) {
	for (i = 0; i < computer.num_screens; i++)
	    if (computer.screens[i]->widget == config) {
		if (ScreenConfig(computer.screens[i]->screen) != NULL)
		    SetTip((xf86cfgDevice*)computer.screens[i]);
	    }
    }
}

void
OptionsCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i;
    XF86OptionPtr *options = NULL;
#ifdef USE_MODULES
    xf86cfgModuleOptions *drv_opts = NULL;
#endif

    if (config_mode == CONFIG_SCREEN) {
	for (i = 0; i < computer.num_screens; i++)
	    if (computer.screens[i]->widget == config) {
		options = &(computer.screens[i]->screen->scrn_option_lst);
		break;
	    }
    }
    else {
	for (i = 0; i < computer.num_devices; i++)
	    if (computer.devices[i]->widget == config)
		break;

	if (i >= computer.num_devices) {
	    if (XF86Config->conf_flags == NULL)
		XF86Config->conf_flags = (XF86ConfFlagsPtr)
		    XtCalloc(1, sizeof(XF86ConfFlagsRec));
	    options = &(XF86Config->conf_flags->flg_option_lst);
	}
	else {
	    switch (computer.devices[i]->type) {
		case MOUSE:
		case KEYBOARD:
		    options = (XF86OptionPtr*)&(((XF86ConfInputPtr)
			(computer.devices[i]->config))->inp_option_lst);
#ifdef USE_MODULES
		    if (!nomodules) {
			char *drv = ((XF86ConfInputPtr)
				(computer.devices[i]->config))->inp_driver;

			if (drv) {
			    drv_opts = module_options;
			    while (drv_opts) {
				if (drv_opts->type == InputModule &&
				    strcmp(drv_opts->name, drv) == 0)
				    break;
				drv_opts = drv_opts->next;
			    }
			}
		    }
#endif

		    break;
		case CARD:
		    options = (XF86OptionPtr*)&(((XF86ConfDevicePtr)
			(computer.devices[i]->config))->dev_option_lst);
#ifdef USE_MODULES
		    if (!nomodules) {
			char *drv = ((XF86ConfDevicePtr)
				(computer.devices[i]->config))->dev_driver;

			if (drv) {
			    drv_opts = module_options;
			    while (drv_opts) {
				if (drv_opts->type == VideoModule &&
				    strcmp(drv_opts->name, drv) == 0)
				    break;
				drv_opts = drv_opts->next;
			    }
			}
		    }
#endif
		    break;
		case MONITOR:
		    options = (XF86OptionPtr*)&(((XF86ConfMonitorPtr)
			(computer.devices[i]->config))->mon_option_lst);
		    break;
	    }
	}
    }

#ifdef USE_MODULES
    OptionsPopup(options, drv_opts ? drv_opts->name : NULL,
		 drv_opts ? drv_opts->option : NULL);
#else
    OptionsPopup(options);
#endif
    if (config_mode == CONFIG_SCREEN) {
	XF86OptionPtr option, options;
	int rotate = 0;

	options = computer.screens[i]->screen->scrn_option_lst;
	if ((option = xf86findOption(options, "Rotate")) != NULL) {
	    if (option->opt_val != NULL)
		rotate = strcasecmp(option->opt_val, "CW") == 0 ? 1 :
			 strcasecmp(option->opt_val, "CCW") == 0 ? -1 : 0;
	    XtFree(option->opt_val);
	    option->opt_val = XtNewString(rotate > 0 ? "CW" : "CCW");
	    computer.screens[i]->rotate = rotate;
	}
	else
	    computer.screens[i]->rotate = 0;
	UpdateScreenUI();
	AdjustScreenUI();
	SetTip((xf86cfgDevice*)computer.screens[i]);
    }
    else {
	if (i >= computer.num_devices)
	    SetTip(&cpu_device);
	else
	    SetTip(computer.devices[i]);
    }
}

void
EnableDeviceCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i;

    if (config_mode == CONFIG_SCREEN) {
	for (i = 0; i < computer.num_screens; i++)
	    if (computer.screens[i]->widget == config) {
		computer.screens[i]->state = USED;
		computer.screens[i]->card->state = USED;
		ScreenSetup(False);
		return;
	    }
    }

    for (i = 0; i < computer.num_devices; i++)
	if (computer.devices[i]->widget == config) {
	    if (computer.devices[i]->state == USED)
		return;
	    computer.devices[i]->state = USED;
	    DrawCables();
	    break;
	}
    if (i >= computer.num_devices || computer.layout == NULL)
	return;
    switch (computer.devices[i]->type) {
	case MOUSE:
	case KEYBOARD: {
	    int nmouses = 0, nkeyboards = 0;
	    XF86ConfInputPtr input = (XF86ConfInputPtr)
		(computer.devices[i]->config);
	    XF86ConfInputrefPtr nex, iref = computer.layout->lay_input_lst;
	    XF86OptionPtr option;

	    nex = iref;
	    while (nex != NULL) {
		if (strcasecmp(nex->iref_inputdev->inp_driver, "mouse") == 0)
		    ++nmouses;
		else if (IS_KBDDRIV(nex->iref_inputdev->inp_driver))
		    ++nkeyboards;
		iref = nex;
		nex = (XF86ConfInputrefPtr)(nex->list.next);
	    }
	    nex = (XF86ConfInputrefPtr)XtCalloc(1, sizeof(XF86ConfInputrefRec));
	    nex->list.next = NULL;
	    nex->iref_inputdev = input;
	    nex->iref_inputdev_str = XtNewString(input->inp_identifier);
	    if (nmouses == 0 && computer.devices[i]->type == MOUSE) 
		option = xf86newOption(XtNewString("CorePointer"), NULL);
	    else if (nkeyboards == 0 && computer.devices[i]->type == KEYBOARD)
		option = xf86newOption(XtNewString("CoreKeyboard"), NULL);
	    else
		option = xf86newOption(XtNewString("SendCoreEvents"), NULL);
	    nex->iref_option_lst = option;
	    computer.layout->lay_input_lst =
		xf86addInputref(computer.layout->lay_input_lst, nex);
	}   break;
	case CARD:
	    for (i = 0; i < computer.num_screens; i++) {
		if (computer.screens[i]->card->widget == config &&
		    computer.screens[i]->state != USED) {
		    XF86ConfAdjacencyPtr adj;

		    adj = (XF86ConfAdjacencyPtr)
			XtCalloc(1, sizeof(XF86ConfAdjacencyRec));
		    adj->adj_screen = computer.screens[i]->screen;
		    adj->adj_screen_str = XtNewString(computer.screens[i]->
			screen->scrn_identifier);
		    computer.layout->lay_adjacency_lst = (XF86ConfAdjacencyPtr)
			xf86addListItem((GenericListPtr)computer.layout->
				    lay_adjacency_lst, (GenericListPtr)adj);
		    computer.screens[i]->state = USED;
		}
	    }
	    break;
	case MONITOR:
	    break;
    }
}

void
DisableDeviceCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i;

    if (config_mode == CONFIG_SCREEN) {
	for (i = 0; i < computer.num_screens; i++)
	    if (computer.screens[i]->widget == config) {
		computer.screens[i]->state = UNUSED;
		computer.screens[i]->card->state = UNUSED;
		ScreenSetup(False);
		return;
	    }
    }

    for (i = 0; i < computer.num_devices; i++)
	if (computer.devices[i]->widget == config) {
	    if (computer.devices[i]->state == UNUSED)
		return;
	    computer.devices[i]->state = UNUSED;
	    DrawCables();
	    break;
	}
    if (i >= computer.num_devices || computer.layout == NULL)
	return;
    switch (computer.devices[i]->type) {
	case MOUSE:
	case KEYBOARD:
	    xf86removeInputRef(computer.layout,
		(XF86ConfInputPtr)(computer.devices[i]->config));
	    break;
	case CARD: {
	    XF86ConfAdjacencyPtr adj;
	    int j;

	    if (computer.layout == NULL)
		break;
	    for (j = 0; j < computer.num_screens; j++)
		if (computer.screens[j]->card->widget == config) {
		    adj = computer.layout->lay_adjacency_lst;
		    while (adj != NULL) {
			if (adj->adj_screen == computer.screens[j]->screen) {
			    xf86removeAdjacency(computer.layout, adj);
			    break;
			}
			adj = (XF86ConfAdjacencyPtr)(adj->list.next);
		    }
		    computer.screens[j]->state = UNUSED;
		    break;
		}
	}   break;
	case MONITOR:
	    break;
    }
}

/* ARGSUSED */
void
RemoveDeviceCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i, j;

    for (i = 0; i < computer.num_screens; i++)
	if (computer.screens[i]->widget == config) {
	    RemoveScreen(computer.screens[i]->monitor,
			 computer.screens[i]->card);
	    ScreenSetup(False);
	    return;
	}

    for (i = 0; i < computer.num_devices; i++) {
	if (computer.devices[i]->widget == config) {
	    switch (computer.devices[i]->type) {
		case MOUSE:
		case KEYBOARD:
		    xf86removeInput(XF86Config,
			(XF86ConfInputPtr)(computer.devices[i]->config));
		    break;
		case CARD:
		case MONITOR:
		    break;
	    }

	    if (computer.devices[i]->type == CARD) {
		for (j = 0; j < computer.num_screens; j++)
		    if (computer.screens[j]->card == computer.devices[i]) {
			RemoveScreen(computer.screens[j]->monitor,
				     computer.devices[i]);
			--j;
		    }
		if (computer.devices[i]->refcount <= 0)
		    xf86removeDevice(XF86Config,
			(XF86ConfDevicePtr)(computer.devices[i]->config));
	    }
	    else if (computer.devices[i]->type == MONITOR) {
		for (j = 0; j < computer.num_screens; j++)
		    if (computer.screens[j]->monitor == computer.devices[i]) {
			RemoveScreen(computer.devices[i],
				     computer.screens[j]->card);
			--j;
		    }
		if (computer.devices[i]->refcount <= 0)
		    xf86removeMonitor(XF86Config,
			(XF86ConfMonitorPtr)(computer.devices[i]->config));
	    }

	    if (computer.devices[i]->refcount <= 0) {
		int type = computer.devices[i]->type;

		XtDestroyWidget(computer.devices[i]->widget);
		XtFree((XtPointer)computer.devices[i]);
		if (--computer.num_devices > i)
		    memmove(&computer.devices[i], &computer.devices[i + 1],
			    (computer.num_devices - i) * sizeof(xf86cfgDevice*));

		DrawCables();
		UpdateMenuDeviceList(type);
	    }

	    break;
	}
    }
}

void
UpdateMenuDeviceList(int type)
{
    Widget sme = NULL, menu = NULL;
    int i, count;
    static char *mouseM = "mouseM", *keyboardM = "keyboardM",
		*cardM = "cardM", *monitorM = "monitorM";

    for (i = count = 0; i < computer.num_devices; i++)
	if (computer.devices[i]->type == type)
	    ++count;

    switch (type) {
	case MOUSE:
	    sme = mouseSme;
	    menu = mouseMenu;
	    break;
	case KEYBOARD:
	    sme = keyboardSme;
	    menu = keyboardMenu;
	    break;
	case CARD:
	    sme = cardSme;
	    menu = cardMenu;
	    break;
	case MONITOR:
	    sme = monitorSme;
	    menu = monitorMenu;
	    break;
    }

    if (menu)
	for (i = ((CompositeWidget)menu)->composite.num_children - 1; i >= 0; i--)
	    XtDestroyWidget(((CompositeWidget)menu)->composite.children[i]);

    if (count < 2) {
	XtVaSetValues(sme, XtNmenuName, NULL, XtNleftBitmap, None, NULL);
	return;
    }

    switch (type) {
	case MOUSE:
	    if (mouseMenu == NULL)
		menu = mouseMenu =
		    XtCreatePopupShell(mouseM, simpleMenuWidgetClass,
				       XtParent(mouseSme), NULL, 0);
	    XtVaSetValues(mouseSme, XtNmenuName, mouseM,
			  XtNleftBitmap, menuPixmap, NULL);
	    break;
	case KEYBOARD:
	    if (keyboardMenu == NULL)
		menu = keyboardMenu =
		    XtCreatePopupShell(keyboardM, simpleMenuWidgetClass,
				       XtParent(keyboardSme), NULL, 0);
	    XtVaSetValues(keyboardSme, XtNmenuName, keyboardM,
			  XtNleftBitmap, menuPixmap, NULL);
	    break;
	case CARD:
	    if (cardMenu == NULL)
		menu = cardMenu =
		    XtCreatePopupShell(cardM, simpleMenuWidgetClass,
				       XtParent(cardSme), NULL, 0);
	    XtVaSetValues(cardSme, XtNmenuName, cardM,
			  XtNleftBitmap, menuPixmap, NULL);
	    break;
	case MONITOR:
	    if (monitorMenu == NULL)
		menu = monitorMenu =
		    XtCreatePopupShell(monitorM, simpleMenuWidgetClass,
				       XtParent(monitorSme), NULL, 0);
	    XtVaSetValues(monitorSme, XtNmenuName, monitorM,
			  XtNleftBitmap, menuPixmap, NULL);
	    break;
    }

    for (i = 0; i < computer.num_devices; i++)
	if (computer.devices[i]->type == type) {
	    char *label = NULL;

	    if (computer.devices[i]->config) {
		switch (type) {
		    case MOUSE:
		    case KEYBOARD:
			label = ((XF86ConfInputPtr)computer.devices[i]->config)
			    ->inp_identifier;
			break;
		    case CARD:
			label = ((XF86ConfDevicePtr)computer.devices[i]->config)
			    ->dev_identifier;
			break;
		    case MONITOR:
			label = ((XF86ConfMonitorPtr)computer.devices[i]->config)
			    ->mon_identifier;
			break;
		}
	    }
	    else {
		switch (type) {
		    case MOUSE:
			label = "newMouse";
			break;
		    case KEYBOARD:
			label = "newKeyboard";
			break;
		    case CARD:
			label = "newCard";
			break;
		    case MONITOR:
			label = "newMonitor";
			break;
		}
	    }

	    sme = XtCreateManagedWidget(label, smeBSBObjectClass, menu, NULL, 0);
	    XtAddCallback(sme, XtNcallback, SmeConfigureDeviceCallback,
			  computer.devices[i]->config ?
			  computer.devices[i]->config :
			  (XtPointer) (-((long)type + 100)));
	}
}

/*ARGSUSED*/
void
SelectDeviceAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    device = w;
    xpos = event->xbutton.x_root;
    ypos = event->xbutton.y_root;
    XDefineCursor(XtDisplay(device), XtWindow(device), no_cursor);

    if (config_mode == CONFIG_SCREEN) {
	sxpos = device->core.x;
	sypos = device->core.y;
    }
}

/*ARGSUSED*/
void
MoveDeviceAction(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    int dx, dy, x, y, oldx, oldy;

    if (device == NULL || device != w)
	return;

    dx = event->xbutton.x_root - xpos;
    dy = event->xbutton.y_root - ypos;

    oldx = device->core.x;
    oldy = device->core.y;
    x = device->core.x + dx;
    y = device->core.y + dy;

    if (x < 0)
	x = 0;
    else if (x + device->core.width > XtParent(device)->core.width)
	x = XtParent(device)->core.width - device->core.width;
    if (y < 0)
	y = 0;
    else if (y + device->core.height > XtParent(device)->core.height)
	y = XtParent(device)->core.height - device->core.height;

    dx = x - oldx;
    dy = y - oldy;

    XRaiseWindow(XtDisplay(device), XtWindow(device));
    XtMoveWidget(device, x, y);

    xpos += dx;
    ypos += dy;
}

/*ARGSUSED*/
void
UnselectDeviceAction(Widget w, XEvent *ev, String *params, Cardinal *num_params)
{
    if (device != NULL) {
	XUndefineCursor(XtDisplay(device), XtWindow(device));

	if (config_mode == CONFIG_SCREEN)
	    ScreenSetup(False);
	device = NULL;
    }
}

/*ARGSUSED*/
void
DevicePopupMenu(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    static Widget configure, options, enable, disable, remove;
    static int first = 1;
    int i;
    xf86cfgDevice *dev;

    if (first) {
	first = 0;

	popup = XtCreatePopupShell("popup", simpleMenuWidgetClass,
				   toplevel, NULL, 0);
	configure = XtCreateManagedWidget("configure", smeBSBObjectClass,
					  popup, NULL, 0);
	XtAddCallback(configure, XtNcallback, ConfigureDeviceCallback, NULL);
	options = XtCreateManagedWidget("options", smeBSBObjectClass,
					popup, NULL, 0);
	XtAddCallback(options, XtNcallback, OptionsCallback, NULL);
	XtCreateManagedWidget("line", smeLineObjectClass,
			      popup, NULL, 0);
	enable = XtCreateManagedWidget("enable", smeBSBObjectClass,
				       popup, NULL, 0);
	XtAddCallback(enable, XtNcallback, EnableDeviceCallback, NULL);
	disable = XtCreateManagedWidget("disable", smeBSBObjectClass,
					popup, NULL, 0);
	XtAddCallback(disable, XtNcallback, DisableDeviceCallback, NULL);
	XtCreateManagedWidget("line", smeLineObjectClass,
			      popup, NULL, 0);
	remove = XtCreateManagedWidget("remove", smeBSBObjectClass,
				       popup, NULL, 0);
	XtAddCallback(remove, XtNcallback, RemoveDeviceCallback, NULL);

	XtRealizeWidget(popup);
    }

    dev = NULL;
    if (config_mode == CONFIG_LAYOUT) {
	for (i = 0; i < computer.num_devices; i++)
	    if (computer.devices[i]->widget == w) {
		dev = computer.devices[i];
		break;
	    }
	if (i >= computer.num_devices && strcmp(XtName(w), "cpu"))
	    return;
	if (dev == NULL)
	    dev = &cpu_device;
    }
    else if (config_mode == CONFIG_SCREEN) {
	for (i = 0; i < computer.num_screens; i++)
	    if (computer.screens[i]->widget == w) {
		dev = (xf86cfgDevice*)computer.screens[i];
		break;
	    }
    }
    if (dev == NULL)
	return;

    config = w;

    if (dev->type != SERVER) {
	XtSetSensitive(configure, True);
	XtSetSensitive(remove, True);
	XtSetSensitive(options, dev->config != NULL);
	if (computer.layout == NULL || dev->config == NULL ||
	    dev->type == MONITOR) {
	    XtSetSensitive(enable, False);
	    XtSetSensitive(disable, False);
	}
	else if (dev->state == USED) {
	    XtSetSensitive(enable, False);
	    XtSetSensitive(disable, True);
	}
	else {
	    XtSetSensitive(enable, True);
	    XtSetSensitive(disable, False);
	}
    }
    else {
	XtSetSensitive(configure, False);
	XtSetSensitive(options, True);
	XtSetSensitive(enable, False);
	XtSetSensitive(disable, False);
	XtSetSensitive(remove, False);
    }

    XtMoveWidget(popup, event->xbutton.x_root, event->xbutton.y_root);
    XtPopup(popup, XtGrabNone);
}

/*ARGSUSED*/
void
DevicePopdownMenu(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (popup && XtIsRealized(popup))
	XtPopdown(popup);
}

void RenameLayoutAction(Widget w, XEvent *event,
			String *params, Cardinal *num_params)
{
    XF86ConfLayoutPtr lay = XF86Config->conf_layout_lst;
    Arg args[1];
    char *name;

    XtSetArg(args[0], XtNstring, &name);
    XtGetValues(layout, args, 1);

    if (computer.layout == NULL || (computer.layout &&
	strcasecmp(name, computer.layout->lay_identifier)) == 0)
	return;

    if (name == NULL && *name == '\0') {
	/* tell user about error */
	return;
    }

    while (lay) {
	if (strcasecmp(name, lay->lay_identifier) == 0)
	    /* tell user about error */
	    return;
	lay = (XF86ConfLayoutPtr)(lay->list.next);
    }

    XtSetArg(args[0], XtNlabel, name);
    XtSetValues(layoutsme, args, 1);
    xf86renameLayout(XF86Config, computer.layout, name);
}

/*ARGSUSED*/
static void
ComputerEventHandler(Widget w, XtPointer closure,
		     XEvent *event, Boolean *continue_to_dispatch)
{
    if (event->xexpose.count > 1)
	return;

    if (config_mode == CONFIG_LAYOUT)
	DrawCables();
}

void
DrawCables(void)
{
    Display *display;
    Window window;
    int ox, oy, i;
    xf86cfgScreen **scr = computer.screens;

    if (config_mode != CONFIG_LAYOUT)
	return;

    ox = computer.cpu->core.x + (computer.cpu->core.width >> 1);
    oy = computer.cpu->core.y + (computer.cpu->core.height >> 1);

    display = XtDisplay(work);
    window = XtWindow(work);
    XClearWindow(display, window);

    for (i = 0; i < computer.num_devices; i++) {
	if (computer.devices[i]->state == USED &&
	    computer.devices[i]->type != MONITOR)
	    DrawCable(display, window, ox, oy,
		      computer.devices[i]->widget->core.x +
			    (computer.devices[i]->widget->core.width>>1),
		      computer.devices[i]->widget->core.y +
			    (computer.devices[i]->widget->core.height>>1));

    }
    for (i = 0; i < computer.num_screens; i++) {
	if (scr[i]->monitor != NULL)
	    DrawCable(display, window,
		      scr[i]->card->widget->core.x +
			    (scr[i]->card->widget->core.width>>1),
		      scr[i]->card->widget->core.y +
			    (scr[i]->card->widget->core.height>>1),
		      scr[i]->monitor->widget->core.x +
			    (scr[i]->monitor->widget->core.width>>1),
		      scr[i]->monitor->widget->core.y +
			    (scr[i]->monitor->widget->core.height>>1));
    }
}

static void
DrawCable(Display *display, Window window, int o_x, int o_y, int d_x, int d_y)
{
    XDrawLine(display, window, cablegcshadow, o_x, o_y, d_x, d_y);
    XDrawLine(display, window, cablegc, o_x, o_y, d_x, d_y);
}

/*ARGSUSED*/
void
SetConfigModeCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i, mode = (long)user_data;
    Arg args[3];
    char *ptr;
    static Dimension height;

    if (mode == config_mode)
	return;
    XtSetArg(args[0], XtNlabel, &ptr);
    XtGetValues(w, args, 1);
    XtSetArg(args[0], XtNlabel, ptr);
    XtSetValues(topMenu, args, 1);

    if (config_mode == CONFIG_LAYOUT) {
	XtSetArg(args[0], XtNheight, &height);
	XtGetValues(commands, args, 1);
	for (i = 0; i < computer.num_devices; i++)
	    XtUnmapWidget(computer.devices[i]->widget);
	XtUnmapWidget(commands);
	XtUnmapWidget(computer.cpu);
	XtSetSensitive(commands, False);
	XtSetArg(args[0], XtNheight, 1);
	XtSetArg(args[1], XtNmin, 1);
	XtSetArg(args[2], XtNmax, 1);
	XtSetValues(commands, args, 3);
    }
    else if (config_mode == CONFIG_SCREEN) {
	for (i = 0; i < computer.num_screens; i++)
	    XtUnmapWidget(computer.screens[i]->widget);
    }
    else if (config_mode == CONFIG_MODELINE) {
	VideoModeConfigureEnd();
	XtSetSensitive(layout, True);
	XtSetSensitive(layoutm, True);
    }
    else if (config_mode == CONFIG_ACCESSX) {
	AccessXConfigureEnd();
	XtSetSensitive(layout, True);
	XtSetSensitive(layoutm, True);
    }

    config_mode = mode;
    XClearWindow(XtDisplay(work), XtWindow(work));
    if (mode == CONFIG_LAYOUT) {
	for (i = 0; i < computer.num_devices; i++)
	    XtMapWidget(computer.devices[i]->widget);
	XtSetArg(args[0], XtNheight, height);
	XtSetArg(args[1], XtNmin, height);
	XtSetArg(args[2], XtNmax, height);
	XtSetValues(commands, args, 3);
	XtMapWidget(commands);
	XtMapWidget(computer.cpu);
	XtSetSensitive(commands, True);
	DrawCables();
    }
    else if (mode == CONFIG_SCREEN) {
	for (i = 0; i < computer.num_screens; i++)
	    XtMapWidget(computer.screens[i]->widget);
	ScreenSetup(True);
    }
    else if (mode == CONFIG_MODELINE) {
	VideoModeConfigureStart();
	XtSetSensitive(layout, False);
	XtSetSensitive(layoutm, False);
    }
    else if (mode == CONFIG_ACCESSX) {
	AccessXConfigureStart();
	XtSetSensitive(layout, False);
	XtSetSensitive(layoutm, False);
    }
}

static void
ScreenSetup(Bool check)
{
    if (check) {
	int i;

	for (i = 0; i < computer.num_layouts; i++)
	    if (computer.layouts[i]->layout == computer.layout)
		break;

	/* Just to put the screens in the correct positions */
	if (i >= computer.num_layouts)
	    AdjustScreenUI();
    }

    UpdateScreenUI();
    AdjustScreenUI();
}
