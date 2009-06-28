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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__SCO__) || defined(__UNIXWARE__) || \
	(defined(sun) && defined(__SVR4)) || defined(__NetBSD__)
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/extensions/XKBstr.h>
#include <X11/extensions/XKBrules.h>
#include "cards.h"
#include "config.h"
#include "xf86config.h"
#include "loader.h"

#define IS_KBDDRIV(X) ((strcmp((X),"kbd") == 0))

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "/usr"
#endif

#ifndef XKB_RULES_DIR
#define XKB_RULES_DIR PROJECT_ROOT "/share/X11/xkb/rules"
#endif

#define CONTROL_A	1
#define CONTROL_D	4
#define CONTROL_E	5
#define CONTROL_K	11
#define TAB	9
#define	MIN(a, b)	((a) < (b) ? (a) : (b))
#define	MAX(a, b)	((a) > (b) ? (a) : (b))

void TextMode(void);

static void ClearScreen(void);
static void PaintWindow(WINDOW*, char*, int, int, int, int);
static void PaintBox(WINDOW*, int, int, int, int);
static void PaintButton(WINDOW*, char*, int, int, int);
static void PrintWrap(WINDOW*, char*, int, int, int);
static int Dialog(char*, char*, int, int, char*, char*, int);
static void PaintItem(WINDOW*, char*, int, int);
static int DialogMenu(char*, char*, int, int, int, int, char**, char*, char*, int);
static void PaintCheckItem(WINDOW*, char*, int, int, int);
static int DialogCheckBox(char*, char*, int, int, int, int, char**, char*, char*, char*);
static char *DialogInput(char*, char*, int, int, char*, char*, char*, int);
static void PaintScroller(WINDOW*, int, int, int);

static int MouseConfig(void);
static int KeyboardConfig(void);
static int MonitorConfig(void);
static int CardConfig(void);
static int ScreenConfig(void);
static int LayoutConfig(void);
static int WriteXF86Config(void);

static XF86ConfLayoutPtr CopyLayout(XF86ConfLayoutPtr);
static XF86ConfAdjacencyPtr CopyAdjacency(XF86ConfAdjacencyPtr);
static XF86ConfInputrefPtr CopyInputref(XF86ConfInputrefPtr);
static XF86ConfInactivePtr CopyInactive(XF86ConfInactivePtr);
static void FreeLayout(XF86ConfLayoutPtr);

extern int string_to_parser_range(char*, parser_range*, int);
#define PARSER_RANGE_SIZE	256
/* string must have at least 256 bytes */
extern int parser_range_to_string(char*, parser_range*, int);

static Bool newconfig;

static chtype screen_attr = A_NORMAL;
static chtype dialog_attr = A_REVERSE;
static chtype highlight_border_attr = A_REVERSE;
static chtype shadow_border_attr = A_REVERSE;
static chtype title_attr = A_NORMAL;
static chtype button_active_attr = A_NORMAL;
static chtype button_inactive_attr = A_NORMAL;
static int menu_width, item_x;
static char Edit[] = "Edit ";

static char *main_menu[] = {
#define	CONF_MOUSE	0
    "Configure mouse",
#define	CONF_KEYBOARD	1
    "Configure keyboard",
#define	CONF_MONITOR	2
    "Configure monitor",
#define	CONF_CARD	3
    "Configure card",
#define	CONF_SCREEN	4
    "Configure screen",
#define	CONF_LAYOUT	5
    "Configure layout",
#define	CONF_FINISH	6
    "Write "__XCONFIGFILE__" and quit",
#define	CONF_QUIT	7
    "Quit",
};

void
TextMode(void)
{
    static int first = 1;
    int i, choice = CONF_MOUSE;

#ifdef USE_MODULES
    if (!nomodules)
	LoaderInitializeOptions();
#endif
    initscr();
    noecho();
    nonl();
    keypad(stdscr, TRUE);

    if (first) {
	const char *filename;

	first = 0;

	if (has_colors()) {
	    start_color();
	    init_pair(1, COLOR_BLACK, COLOR_BLACK);
	    screen_attr = A_BOLD | COLOR_PAIR(1);

	    init_pair(2, COLOR_BLACK, COLOR_WHITE);
	    dialog_attr = COLOR_PAIR(2);

	    init_pair(3, COLOR_BLACK, COLOR_WHITE);
	    shadow_border_attr = A_BOLD | COLOR_PAIR(3);

	    init_pair(4, COLOR_WHITE, COLOR_WHITE);
	    highlight_border_attr = A_BOLD | COLOR_PAIR(4);

	    init_pair(5, COLOR_WHITE, COLOR_BLUE);
	    title_attr = A_BOLD | COLOR_PAIR(5);
	    button_active_attr = title_attr;

	    init_pair(6, COLOR_WHITE, COLOR_BLACK);
	    button_inactive_attr = A_BOLD | COLOR_PAIR(6);
	}

	if ((filename = xf86openConfigFile(getuid() == 0 ?
					   CONFPATH : USER_CONFPATH,
					   XF86Config_path, NULL)) != NULL) {
	    XF86Config_path = (char *)filename;
	    if ((XF86Config = xf86readConfigFile()) == NULL) {
		ClearScreen();
		refresh();
		Dialog("Configuration error",
		       "Error parsing configuration file.",
		       7, 50, "  Ok  ", NULL, 0);
	    }
	}
	if (XF86Config == NULL) {
	    XF86Config = (XF86ConfigPtr)XtCalloc(1, sizeof(XF86ConfigRec));
	    newconfig = True;
	}
	else
	    newconfig = False;
    }

    ClearScreen();
    refresh();

    /*CONSTCOND*/
    while (1) {
	int cancel = FALSE;

	ClearScreen();
	refresh();
	if (Dialog( __XSERVERNAME__" Configuration",
		   "This program will create the "__XCONFIGFILE__" file, based on "
		   "menu selections you make.\n"
		   "\n"
#if defined(__SCO__) || defined(__UNIXWARE__)
		   "The "__XCONFIGFILE__" file usually resides in /etc. A "
		   "sample "__XCONFIGFILE__" file is supplied with "
#else
		   "The "__XCONFIGFILE__" file usually resides in " PROJECT_ROOT "/etc/X11 "
		   "or /etc/X11. A sample "__XCONFIGFILE__" file is supplied with "
#endif
		   __XSERVERNAME__"; it is configured for a standard VGA card and "
		   "monitor with 640x480 resolution. This program will ask for "
		   "a pathname when it is ready to write the file.\n"
		   "\n"
		   "You can either take the sample "__XCONFIGFILE__" as a base and "
		   "edit it for your configuration, or let this program "
		   "produce a base "__XCONFIGFILE__" file for your configuration and "
		   "fine-tune it.",
		   20, 60, "   Ok   ", " Cancel ", 0) != 0)
	    break;

	    while (!cancel) {
		ClearScreen();
		refresh();
		switch (DialogMenu("Main menu",
				   "Choose one of the options:",
				   17, 60, 8, sizeof(main_menu) /
				   sizeof(main_menu[0]), main_menu,
				   "   Ok   ", " Cancel ", choice)) {
		    case CONF_MOUSE:
			i = MouseConfig();
			if (i > 0 && choice == CONF_MOUSE)
			    choice = CONF_KEYBOARD;
			else if (i == 0)
			    choice = CONF_MOUSE;
			break;
		    case CONF_KEYBOARD:
			i = KeyboardConfig();
			if (i > 0 && choice <= CONF_KEYBOARD)
			    choice = CONF_MONITOR;
			else if (i == 0)
			    choice = CONF_KEYBOARD;
			break;
		    case CONF_MONITOR:
			i = MonitorConfig();
			if (i > 0 && choice <= CONF_MONITOR)
			    choice = CONF_CARD;
			else if (i == 0)
			    choice = CONF_MONITOR;
			break;
		    case CONF_CARD:
			i = CardConfig();
			if (i > 0 && choice <= CONF_CARD)
			    choice = CONF_SCREEN;
			else if (i == 0)
			    choice = CONF_CARD;
			break;
		    case CONF_SCREEN:
			i = ScreenConfig();
			if (i > 0 && choice <= CONF_SCREEN)
			    choice = CONF_LAYOUT;
			else if (i == 0)
			    choice = CONF_SCREEN;
			break;
		    case CONF_LAYOUT:
			i = LayoutConfig();
			if (i > 0 && choice <= CONF_LAYOUT)
			    choice = CONF_FINISH;
			else if (i == 0)
			    choice = CONF_LAYOUT;
			break;
		    case CONF_FINISH:
			if (WriteXF86Config() < 0)
			    break;
		    /*FALLTROUGH*/
		    case CONF_QUIT:
			endwin();
			exit(0);
		    default:
			cancel = TRUE;
			break;
		}
	}
    }

    endwin();
}

static int
WriteXF86Config(void)
{
    char *xf86config;

    ClearScreen();
    refresh();
    xf86config = DialogInput("Write "__XCONFIGFILE__, "Write configuration to file:",
			     10, 60, XF86Config_path ? XF86Config_path :
			     "/etc/X11/"__XCONFIGFILE__, "  Ok  ", " Cancel ", 0);

    if (xf86config == NULL)
	return (-1);

    if (newconfig) {
	if (XF86Config->conf_modules == NULL) {
	    static char *modules[] = {"extmod", "glx", "dri", "dbe",
				      "record", "xtrap", "type1"};
	    XF86LoadPtr load;
	    int i;

	    XF86Config->conf_modules = (XF86ConfModulePtr)
		XtCalloc(1, sizeof(XF86ConfModuleRec));

	    XF86Config->conf_modules->mod_comment =
		XtNewString("\tLoad \"freetype\"\n"
			    "\t# Load \"xtt\"\n");

	    for (i = 0; i < sizeof(modules) / sizeof(modules[0]); i++) {
		load = (XF86LoadPtr)XtCalloc(1, sizeof(XF86LoadRec));
		load->load_name = XtNewString(modules[i]);
		XF86Config->conf_modules->mod_load_lst =
		    xf86addModule(XF86Config->conf_modules->mod_load_lst, load);
	    }
	}
    }

    if (!xf86writeConfigFile(xf86config, XF86Config)) {
	char msg[1024];

	XmuSnprintf(msg, sizeof(msg), "Failed to write configuration file %s.",
		   xf86config);
	ClearScreen();
	refresh();
	(void)Dialog("Write failed!", msg, 8, 60, "  Ok  ", NULL, 0);
	XtFree(xf86config);
	return (-1);
    }
    XtFree(xf86config);

    return (1);
}

static char *protocols[] = {
#ifdef __SCO__
    "OsMouse",
#endif
#ifdef WSCONS_SUPPORT
    "wsmouse",
#endif
    "Auto",
    "SysMouse",
    "MouseSystems",
    "BusMouse",
    "PS/2",
    "Microsoft",
#ifndef __FreeBSD__
    "ImPS/2",
    "ExplorerPS/2",
    "GlidePointPS/2",
    "MouseManPlusPS/2",
    "NetMousePS/2",
    "NetScrollPS/2",
    "ThinkingMousePS/2",
#endif
    "AceCad",
    "GlidePoint",
    "IntelliMouse",
    "Logitech",
    "MMHitTab",
    "MMSeries",
    "MouseMan",
    "ThinkingMouse",
};

static int
MouseConfig(void)
{
    int i, nlist, def, proto, emul;
    char **list = NULL, *device, *str;
    XF86ConfInputPtr *inputs = NULL;
    XF86ConfInputPtr input = XF86Config->conf_input_lst;
    XF86OptionPtr option;

    nlist = 0;
    while (input) {
	if (strcmp(input->inp_driver, "mouse") == 0) {
	    list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
	    list[nlist] = XtMalloc(sizeof(Edit) +
				   strlen(input->inp_identifier) + 1);
	    sprintf(list[nlist], "%s%s", Edit, input->inp_identifier);
	    inputs = (XF86ConfInputPtr*)XtRealloc((XtPointer)inputs, (nlist + 1) *
					sizeof(XF86ConfInputPtr));
	    inputs[nlist] = input;
	    ++nlist;
	}
	input = (XF86ConfInputPtr)(input->list.next);
    }

    input = NULL;

    if (nlist) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 2) * sizeof(char*));
	list[nlist++] = XtNewString("Add new mouse");
	if (nlist == 2) {
	    i = strlen("Remove ") + strlen(inputs[0]->inp_identifier) + 1;
	    list[nlist] = XtMalloc(i);
	    XmuSnprintf(list[nlist], i, "Remove %s", inputs[0]->inp_identifier);
	    ++nlist;
	}
	else
	    list[nlist++] = XtNewString("Remove mouse");
	ClearScreen();
	refresh();
	i = DialogMenu("Mouse configuration",
		       "You can edit or remove a previously configured mouse, "
		       "or add a new one.", 14, 60, 4, nlist, list,
		       " Ok  ", " Cancel ", 0);
	if (i < 0) {
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)inputs);
	    return (-1);
	}
	if (nlist > 2 && i == nlist - 1) {
	    if (nlist > 3) {
		for (i = 0; i < nlist - 2; i++) {
		    /* XXX Remove the "Edit " from list entries */
		    memmove(list[i], list[i] + sizeof(Edit) - 1,
			    strlen(list[i]) - sizeof(Edit) + 2);
		}
		ClearScreen();
		refresh();
		i = DialogMenu("Remove mouse",
			       "Select which mouse to remove",
			       13, 60, 4, nlist - 2, list,
			       " Remove ", " Cancel ", 0);
		if (i < 0) {
		    for (i = 0; i < nlist; i++)
			XtFree(list[i]);
		    XtFree((XtPointer)list);
		    XtFree((XtPointer)inputs);
		    return (-1);
		}
		input = inputs[i];
	    }
	    else
		input = inputs[0];
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)inputs);
	    xf86removeInput(XF86Config, input);
	    return (0);
	}
	if (i < nlist - 2)
	    input = inputs[i];
    }
    for (i = 0; i < nlist; i++)
	XtFree(list[i]);
    XtFree((XtPointer)list);
    XtFree((XtPointer)inputs);

    if (input == NULL) {
	char label[32];

	input = (XF86ConfInputPtr)XtCalloc(1, sizeof(XF86ConfInputRec));
	XmuSnprintf(label, sizeof(label), "Mouse%d", nlist ? nlist - 2 : 0);
	ClearScreen();
	refresh();
	input->inp_identifier =
	    DialogInput("Mouse identifier",
			"Enter an identifier for your mouse definition:",
			11, 40, label,
			" Next >>", " Cancel ", 0);
	if (input->inp_identifier == NULL) {
	    XtFree((XtPointer)input);
	    return (-1);
	}
    }

    def = 0;
    option = xf86findOption(input->inp_option_lst, "Protocol");
    if (option)
	for (i = 0; i < sizeof(protocols)/sizeof(protocols[0]); i++)
	    if (strcasecmp(option->opt_val, protocols[i]) == 0) {
		def = i;
		break;
	    }

    ClearScreen();
    refresh();
    i = DialogMenu("Select mouse protocol",
		   "If you have a serial mouse, it probably will work with "
		   "the \"Auto\" protocol. But, if it is an old serial "
		   "mouse probably it is not PNP; in that case, most serial "
		   "mouses understand the \"Microsoft\" protocol.",
		   19, 60, 7, sizeof(protocols) /
		   sizeof(protocols[0]), protocols, " Next >>", " Cancel ", def);
    if (i < 0) {
	if (input->inp_driver == NULL) {
	    XtFree(input->inp_driver);
	    XtFree((XtPointer)input);
	}
	return (i);
    }
    proto = i;

    def = 0;
    if (input->inp_driver) {
	option = xf86findOption(input->inp_option_lst, "Emulate3Buttons");
	def = option ? 0 : 1;
    }
    ClearScreen();
    refresh();
    i = Dialog("Mouse 3 buttons emulation",
	       "If your mouse has only two buttons, it is recommended that "
	       "you enable Emulate3Buttons.\n"
	       "\n"
	       "Do you want to enable Emulate3Buttons?",
	       10, 60, " Yes ", " No ", def);
    if (i < 0)
	return (i);
    emul = !i;

    str = NULL;
    option = xf86findOption(input->inp_option_lst, "Device");
    if (option)
	str = option->opt_val;
    if (str == NULL)
#ifdef WSCONS_SUPPORT
	str = "/dev/wsmouse";
#elif defined(__FreeBSD__) || defined(__DragonFly__)
	str = "/dev/sysmouse";
#elif defined(__linux__)
	str = "/dev/input/mice";
#else
	str = "/dev/mouse";
#endif

    ClearScreen();
    refresh();
    device = DialogInput("Select mouse device",
		       "Enter mouse device:", 10, 40, str,
		       " Finish ", " Cancel ", 0);
    if (device == NULL) {
	if (input->inp_driver == NULL) {
	    XtFree(input->inp_driver);
	    XtFree((XtPointer)input);
	}
	return (-1);
    }

    /* Finish mouse configuration */
    option = xf86findOption(input->inp_option_lst, "Protocol");
    if (option) {
	XtFree((XtPointer)option->opt_val);
	option->opt_val = XtNewString(protocols[proto]);
    }
    else
	input->inp_option_lst = xf86addNewOption(input->inp_option_lst,
		XtNewString("Protocol"), XtNewString(protocols[proto]));

    option = xf86findOption(input->inp_option_lst, "Emulate3Buttons");
    if (option && !emul) {
	xf86removeOption(&input->inp_option_lst, "Emulate3Buttons");
    }
    else if (option == NULL && emul)
	input->inp_option_lst = xf86addNewOption(input->inp_option_lst,
		XtNewString("Emulate3Buttons"), NULL);

    option = xf86findOption(input->inp_option_lst, "Device");
    if (option) {
	XtFree((XtPointer)option->opt_val);
	option->opt_val = device;
    }
    else
	input->inp_option_lst = xf86addNewOption(input->inp_option_lst,
		XtNewString("Device"), device);

    if (input->inp_driver == NULL) {
	input->inp_driver = XtNewString("mouse");
	XF86Config->conf_input_lst =
	    xf86addInput(XF86Config->conf_input_lst, input);
    }

    return (1);
}

static int
KeyboardConfig(void)
{
    int i;
    char *rulesfile;
    static int first = 1;
    static XkbRF_RulesPtr rules;
    static char **models, **layouts;
    XF86ConfInputPtr *inputs = NULL, input = XF86Config->conf_input_lst;
    char **list = NULL, *model, *layout;
    int nlist, def;
    XF86OptionPtr option;

    nlist = 0;
    while (input) {
	if (IS_KBDDRIV(input->inp_driver)) {
	    list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
	    list[nlist] = XtMalloc(sizeof(Edit) +
				   strlen(input->inp_identifier) + 1);
	    sprintf(list[nlist], "%s%s", Edit, input->inp_identifier);
	    inputs = (XF86ConfInputPtr*)XtRealloc((XtPointer)inputs, (nlist + 1) *
					sizeof(XF86ConfInputPtr));
	    inputs[nlist] = input;
	    ++nlist;
	}
	input = (XF86ConfInputPtr)(input->list.next);
    }

    input = NULL;

    if (nlist) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 2) * sizeof(char*));
	list[nlist++] = XtNewString("Add new keyboard");
	if (nlist == 2) {
	    i = strlen("Remove ") + strlen(inputs[0]->inp_identifier) + 1;
	    list[nlist] = XtMalloc(i);
	    XmuSnprintf(list[nlist], i, "Remove %s", inputs[0]->inp_identifier);
	    ++nlist;
	}
	else
	    list[nlist++] = XtNewString("Remove keyboard");
	ClearScreen();
	refresh();
	i = DialogMenu("Keyboard configuration",
		       "You can edit or remove a previously configured "
		       "keyboard, or add a new one.", 14, 60, 4, nlist, list,
		       " Ok  ", " Cancel ", 0);
	if (i < 0) {
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)inputs);
	    return (-1);
	}
	if (nlist > 2 && i == nlist - 1) {
	    if (nlist > 3) {
		for (i = 0; i < nlist - 2; i++) {
		    /* XXX Remove the "Edit " from list entries */
		    memmove(list[i], list[i] + sizeof(Edit) - 1,
			    strlen(list[i]) - sizeof(Edit) + 2);
		}
		ClearScreen();
		refresh();
		i = DialogMenu("Remove keyboard",
			       "Select which keyboard to remove",
			       13, 60, 4, nlist - 2, list,
			       " Remove ", " Cancel ", 0);
		if (i < 0) {
		    for (i = 0; i < nlist; i++)
			XtFree(list[i]);
		    XtFree((XtPointer)list);
		    XtFree((XtPointer)inputs);
		    return (-1);
		}
		input = inputs[i];
	    }
	    else
		input = inputs[0];
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)inputs);
	    xf86removeInput(XF86Config, input);
	    return (0);
	}
	if (i < nlist - 2)
	    input = inputs[i];
    }
    for (i = 0; i < nlist; i++)
	XtFree(list[i]);
    XtFree((XtPointer)list);
    XtFree((XtPointer)inputs);

    if (input == NULL) {
	char label[32];

	input = (XF86ConfInputPtr)XtCalloc(1, sizeof(XF86ConfInputRec));
	XmuSnprintf(label, sizeof(label), "Keyboard%d", nlist ? nlist - 2 : 0);
	ClearScreen();
	refresh();
	input->inp_identifier =
	    DialogInput("Keyboard identifier",
			"Enter an identifier for your keyboard definition:",
			11, 40, label,
			" Next >>", " Cancel ", 0);
	if (input->inp_identifier == NULL) {
	    XtFree((XtPointer)input);
	    return (-1);
	}
    }

    if (first) {
	first = 0;
#ifdef XFREE98_XKB
	rulesfile = XKB_RULES_DIR "/xfree98";
#else
	rulesfile = XKB_RULES_DIR "/"__XKBDEFRULES__;
#endif
	rules = XkbRF_Load(rulesfile, "", True, False);
	if (rules == NULL) {
	    ClearScreen();
	    refresh();
	    Dialog("Configuration error",
		   "XKB rules file not found.\n"
		   "\n"
		   "Keyboard XKB options will be set to default values.",
		   10, 50, "  Ok  ", NULL, 0);
	    if (input->inp_driver == NULL) {
		input->inp_option_lst =
		    xf86addNewOption(input->inp_option_lst,
			XtNewString("XkbModel"), XtNewString("pc101"));
		input->inp_option_lst =
		    xf86addNewOption(input->inp_option_lst,
			XtNewString("XkbLayout"), XtNewString("us"));
		input->inp_driver = XtNewString("kbd");
		XF86Config->conf_input_lst =
		    xf86addInput(XF86Config, input);
	    }
	    return (0);
	}
	models = (char**)XtMalloc(sizeof(char*) * rules->models.num_desc);
	for (i = 0; i < rules->models.num_desc; i++)
	    models[i] = XtNewString(rules->models.desc[i].desc);
	layouts = (char**)XtMalloc(sizeof(char*) * rules->layouts.num_desc);
	for (i = 0; i < rules->layouts.num_desc; i++)
	    layouts[i] = XtNewString(rules->layouts.desc[i].desc);
    }
    else if (rules == NULL)
	return (-1);

    def = 0;
    option = xf86findOption(input->inp_option_lst, "XkbModel");
    if (option) {
	for (i = 0; i < rules->models.num_desc; i++)
	    if (strcasecmp(option->opt_val, rules->models.desc[i].name) == 0) {
		def = i;
		break;
	    }
    }
    ClearScreen();
    refresh();
    i = DialogMenu("Keyboard model",
		   "Please select one of the following keyboard types that is "
		   "the better description of your keyboard. If nothing really "
		   "matches, choose \"Generic 101-key PC\".\n",
		   20, 60, 9, rules->models.num_desc,
		   models, " Next >>", " Cancel ", def);
    if (i < 0)
	return (i);
    model = rules->models.desc[i].name;

    def = 0;
    option = xf86findOption(input->inp_option_lst, "XkbLayout");
    if (option) {
	for (i = 0; i < rules->layouts.num_desc; i++)
	    if (strcasecmp(option->opt_val, rules->layouts.desc[i].name) == 0) {
		def = i;
		break;
	    }
    }
    ClearScreen();
    refresh();
    i = DialogMenu("Keyboard layout",
	 	   "Select keyboard layout:",
		   20, 60, 11, rules->layouts.num_desc,
		   layouts, " Finish ", " Cancel ", def);
    if (i < 0)
	return (i);
    layout = rules->layouts.desc[i].name;

    /* Finish keyboard configuration */
    option = xf86findOption(input->inp_option_lst, "XkbModel");
    if (option) {
	XtFree((XtPointer)option->opt_val);
	option->opt_val = XtNewString(model);
    }
    else
	input->inp_option_lst = xf86addNewOption(input->inp_option_lst,
		XtNewString("XkbModel"), XtNewString(model));

    option = xf86findOption(input->inp_option_lst, "XkbLayout");
    if (option) {
	XtFree((XtPointer)option->opt_val);
	option->opt_val = XtNewString(layout);
    }
    else
	input->inp_option_lst = xf86addNewOption(input->inp_option_lst,
		XtNewString("XkbLayout"), XtNewString(layout));

    if (input->inp_driver == NULL) {
	input->inp_driver = XtNewString("kbd");
	XF86Config->conf_input_lst =
	    xf86addInput(XF86Config->conf_input_lst, input);
    }

    return (1);
}

static char *hsync[] = {
#define	CONF_MONITOR_HSYNC	0
    "Enter your own horizontal sync range",
    "31.5; Standard VGA, 640x480 @ 60 Hz",
    "31.5 - 35.1; Super VGA, 800x600 @ 56 Hz",
    "31.5, 35.5; 8514 Compatible, 1024x768 @ 87 Hz interlaced (no 800x600)",
    "31.5, 35.15, 35.5; Super VGA, 1024x768 @ 87 Hz int., 800x600 @ 56 Hz",
    "31.5 - 37.9; Extended Super VGA, 800x600 @ 60 Hz, 640x480 @ 72 Hz",
    "31.5 - 48.5; Non-Interlaced SVGA, 1024x768 @ 60 Hz, 800x600 @ 72 Hz",
    "31.5 - 57.0; High Frequency SVGA, 1024x768 @ 70 Hz",
    "31.5 - 64.3; Monitor that can do 1280x1024 @ 60 Hz",
    "31.5 - 79.0; Monitor that can do 1280x1024 @ 74 Hz",
    "31.5 - 82.0; Monitor that can do 1280x1024 @ 76 Hz",
    "31.5 - 92.0; Monitor that can do 1280x1024 @ 85 Hz",
    "31.5 - 108.0; Monitor that can do 1600x1200 @ 85 Hz",
    "31.5 - 128.5; Monitor that can do 1920x1440 @ 85 Hz",
    "31.5 - 137.0; Monitor that can do 2048x1536 @ 85 Hz"
};

static char *vrefresh[] = {
#define	CONF_MONITOR_VREFRESH	0
    "Enter your own vertical sync range",
    "50 - 70",
    "50 - 90",
    "50 - 100",
    "40 - 150",
};

static int
MonitorConfig(void)
{
    int i;
    XF86ConfMonitorPtr *monitors = NULL, monitor = XF86Config->conf_monitor_lst;
    char **list = NULL, *identifier = NULL, *tmp;
    int nlist, def;
    char hsync_str[256], vrefresh_str[256];

    hsync_str[0] = vrefresh_str[0] = '\0';
    nlist = 0;
    while (monitor) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
	list[nlist] = XtMalloc(sizeof(Edit) +
			       strlen(monitor->mon_identifier) + 1);
	sprintf(list[nlist], "%s%s", Edit, monitor->mon_identifier);
	monitors = (XF86ConfMonitorPtr*)XtRealloc((XtPointer)monitors, (nlist + 1) *
				    sizeof(XF86ConfMonitorPtr));
	monitors[nlist] = monitor;
	++nlist;
	monitor = (XF86ConfMonitorPtr)(monitor->list.next);
    }

    monitor = NULL;

    if (nlist) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 2) * sizeof(char*));
	list[nlist++] = XtNewString("Add new monitor");
	if (nlist == 2) {
	    i = strlen("Remove ") + strlen(monitors[0]->mon_identifier) + 1;
	    list[nlist] = XtMalloc(i);
	    XmuSnprintf(list[nlist], i, "Remove %s", monitors[0]->mon_identifier);
	    ++nlist;
	}
	else
	    list[nlist++] = XtNewString("Remove monitor");
	ClearScreen();
	refresh();
	i = DialogMenu("Monitor configuration",
		       "You can edit or remove a previously configured "
		       "monitor, or add a new one.", 14, 60, 4, nlist, list,
		       " Ok  ", " Cancel ", 0);
	if (i < 0) {
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)monitors);
	    return (-1);
	}
	if (nlist > 2 && i == nlist - 1) {
	    if (nlist > 3) {
		for (i = 0; i < nlist - 2; i++) {
		    /* XXX Remove the "Edit " from list entries */
		    memmove(list[i], list[i] + sizeof(Edit) - 1,
			    strlen(list[i]) - sizeof(Edit) + 2);
		}
		ClearScreen();
		refresh();
		i = DialogMenu("Remove monitor",
			       "Select which monitor to remove",
			       13, 60, 4, nlist - 2, list,
			       " Remove ", " Cancel ", 0);
		if (i < 0) {
		    for (i = 0; i < nlist; i++)
			XtFree(list[i]);
		    XtFree((XtPointer)list);
		    XtFree((XtPointer)monitors);
		    return (-1);
		}
		monitor = monitors[i];
	    }
	    else
		monitor = monitors[0];
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)monitors);
	    xf86removeMonitor(XF86Config, monitor);
	    return (0);
	}
	if (i < nlist - 2)
	    monitor = monitors[i];
    }
    for (i = 0; i < nlist; i++)
	XtFree(list[i]);
    XtFree((XtPointer)list);
    XtFree((XtPointer)monitors);

    if (monitor == NULL) {
	char label[32];

	monitor = (XF86ConfMonitorPtr)XtCalloc(1, sizeof(XF86ConfMonitorRec));
	XmuSnprintf(label, sizeof(label), "Monitor%d", nlist ? nlist - 2 : 0);
	ClearScreen();
	refresh();
	identifier =
	    DialogInput("Monitor identifier",
			"Enter an identifier for your monitor definition:",
			11, 40, label,
			" Next >>", " Cancel ", 0);
	if (identifier == NULL) {
	    XtFree((XtPointer)monitor);
	    return (-1);
	}
    }

    if (monitor->mon_identifier == NULL) {
	ClearScreen();
	refresh();
	i = Dialog("Monitor configuration",
		   "Now we want to set the specifications of the monitor. The "
		   "two critical parameters are the vertical refresh rate, which "
		   "is the rate at which the whole screen is refreshed, and most "
		   "importantly the horizontal sync rate, which is the rate at "
		   "which scanlines are displayed.\n"
		   "\n"
		   "The valid range for horizontal sync and vertical sync should "
		   "be documented in the manual of your monitor.",
		   15, 60, " Next >>", " Cancel ", 0);
	if (i != 0) {
	    XtFree(identifier);
	    XtFree((XtPointer)monitor);
	    return (-1);
	}
    }

    def = 0;
    if (monitor->mon_identifier) {
	int len;

	parser_range_to_string(hsync_str, &(monitor->mon_hsync[0]),
			       monitor->mon_n_hsync);
	len = strlen(hsync_str);
	for (i = 1; i < sizeof(hsync) / sizeof(hsync[0]); i++) {
	    tmp = strchr(hsync[i], ';');
	    if (strncmp(hsync_str, hsync[i], len) == 0) {
		def = i;
		break;
	    }
	}
    }
    if (hsync_str[0] == '\0')
	strcpy(hsync_str, "31.5");

    ClearScreen();
    refresh();
    i = DialogMenu("Monitor HorizSync",
		   "You must indicate the horizontal sync range of your "
		   "monitor. You can either select one of the predefined "
		   "ranges below that correspond to industry-standard monitor "
		   "types, or give a specific range.",
		   22, 78, 11, sizeof(hsync) /
		   sizeof(hsync[0]), hsync, " Next >>", " Cancel ", def);
    if (i < 0) {
	if (monitor->mon_identifier == NULL) {
	    XtFree(identifier);
	    XtFree((XtPointer)monitor);
	}
	return (-1);
    }
    if (i == CONF_MONITOR_HSYNC) {
	ClearScreen();
	refresh();
	tmp = DialogInput("Monitor HorizSync",
			  "Please enter the horizontal sync range of your "
			  "monitor, in the format used in the table of monitor "
			  "types above. You can either specify one or more "
			  "continuous ranges (e.g. 15-25, 30-50), or one or more "
			  "fixed sync frequencies.\n"
			  "\n"
			  "Horizontal sync range:", 16, 62, hsync_str,
			  "  Ok  ", " Cancel ", def);
	if (tmp == NULL) {
	    if (monitor->mon_identifier == NULL) {
		XtFree(identifier);
		XtFree((XtPointer)monitor);
	    }
	    return (-1);
	}
	XmuSnprintf(hsync_str, sizeof(hsync_str), "%s", tmp);
	XtFree(tmp);
    }
    else {
	tmp = strchr(hsync[i], ';');
	strncpy(hsync_str, hsync[i], tmp - hsync[i]);
	hsync_str[tmp - hsync[i]] = '\0';
    }

    def = 0;
    if (monitor->mon_identifier) {
	parser_range_to_string(vrefresh_str, &(monitor->mon_vrefresh[0]),
			       monitor->mon_n_vrefresh);
	for (i = 1; i < sizeof(vrefresh) / sizeof(vrefresh[0]); i++) {
	    if (strcmp(vrefresh_str, vrefresh[i]) == 0) {
		def = i;
		break;
	    }
	}
    }
    if (vrefresh_str[0] == '\0')
	strcpy(vrefresh_str, "50 - 70");
    ClearScreen();
    refresh();
    i = DialogMenu("Monitor VertRefresh",
		   "You must indicate the vertical sync range of your monitor. "
		   "You can either select one of the predefined ranges below "
		   "that correspond to industry-standard monitor types, or "
		   "give a specific range. For interlaced modes, the number "
		   "that counts is the high one (e.g. 87 Hz rather than 43 Hz).",
		   19, 60, 5, sizeof(vrefresh) /
		   sizeof(vrefresh[0]), vrefresh, " Finish ", " Cancel ", def);
    if (i < 0) {
	if (monitor->mon_identifier == NULL) {
	    XtFree(identifier);
	    XtFree((XtPointer)monitor);
	}
	return (i);
    }
    if (i == CONF_MONITOR_VREFRESH) {
	ClearScreen();
	refresh();
	tmp = DialogInput("Monitor VertRefresh",
			  "Vertical sync range:", 10, 50, vrefresh_str,
			  " Done ", " Cancel ", 0);
	if (tmp == NULL) {
	    if (monitor->mon_identifier == NULL) {
		XtFree(identifier);
		XtFree((XtPointer)monitor);
	    }
	    return (-1);
	}
	XmuSnprintf(vrefresh_str, sizeof(vrefresh_str), "%s", tmp);
	XtFree(tmp);
    }
    else
	strcpy(vrefresh_str, vrefresh[i]);

    /* Finish monitor configuration */
    monitor->mon_n_hsync = string_to_parser_range(hsync_str,
	&(monitor->mon_hsync[0]), CONF_MAX_HSYNC);
    monitor->mon_n_vrefresh = string_to_parser_range(vrefresh_str,
	&(monitor->mon_vrefresh[0]), CONF_MAX_VREFRESH);
    if (monitor->mon_identifier == NULL) {
	monitor->mon_identifier = identifier;
	XF86Config->conf_monitor_lst =
	    xf86addMonitor(XF86Config->conf_monitor_lst, monitor);
    }

    return (1);
}

static int
CardConfig(void)
{
    int i;
    XF86ConfDevicePtr *devices = NULL, device = XF86Config->conf_device_lst;
    char **list = NULL, *identifier = NULL, *driver, *busid, *tmp;
    int nlist, def;
    CardsEntry *entry = NULL;
    static char **drivers;
    static int ndrivers;
    static char *xdrivers[] = {
	"apm",
	"ark",
	"ast",
	"ati",
	"r128",
	"radeon",
	"chips",
	"cirrus",
	"cyrix",
	"fbdev",
	"glint",
	"i128",
	"i740",
	"i810",
	"imstt",
	"mga",
	"neomagic",
	"nv",
	"rendition",
	"s3",
	"s3virge",
	"savage",
	"siliconmotion",
	"sis",
	"tdfx",
	"tga",
	"trident",
	"tseng",
	"vmware",
	"vga",
	"vesa",
    };

#ifdef USE_MODULES
    if (!nomodules) {
	xf86cfgModuleOptions *opts = module_options;

	drivers = NULL;
	ndrivers = 0;
	while (opts) {
	    if (opts->type == VideoModule) {
		++ndrivers;
		drivers = (char**)XtRealloc((XtPointer)drivers,
					    ndrivers * sizeof(char*));
		/* XXX no private copy */
		drivers[ndrivers - 1] = opts->name;
	    }
	    opts = opts->next;
	}
    }
    else
#endif
    {
	ndrivers = sizeof(xdrivers) / sizeof(xdrivers[0]);
	drivers = xdrivers;
    }

    nlist = 0;
    while (device) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
	list[nlist] = XtMalloc(sizeof(Edit) +
			       strlen(device->dev_identifier) + 1);
	sprintf(list[nlist], "%s%s", Edit, device->dev_identifier);
	devices = (XF86ConfDevicePtr*)XtRealloc((XtPointer)devices, (nlist + 1) *
				    sizeof(XF86ConfDevicePtr));
	devices[nlist] = device;
	++nlist;
	device = (XF86ConfDevicePtr)(device->list.next);
    }

    device = NULL;

    if (nlist) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 2) * sizeof(char*));
	list[nlist++] = XtNewString("Add new card");
	if (nlist == 2) {
	    i = strlen("Remove ") + strlen(devices[0]->dev_identifier) + 1;
	    list[nlist] = XtMalloc(i);
	    XmuSnprintf(list[nlist], i, "Remove %s", devices[0]->dev_identifier);
	    ++nlist;
	}
	else
	    list[nlist++] = XtNewString("Remove device");
	ClearScreen();
	refresh();
	i = DialogMenu("Card configuration",
		       "You can edit or remove a previously configured "
		       "card, or add a new one.", 14, 60, 4, nlist, list,
		       " Ok  ", " Cancel ", 0);
	if (i < 0) {
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)devices);
	    return (-1);
	}
	if (nlist > 2 && i == nlist - 1) {
	    if (nlist > 3) {
		for (i = 0; i < nlist - 2; i++) {
		    /* XXX Remove the "Edit " from list entries */
		    memmove(list[i], list[i] + sizeof(Edit) - 1,
			    strlen(list[i]) - sizeof(Edit) + 2);
		}
		ClearScreen();
		refresh();
		i = DialogMenu("Remove card",
			       "Select which card to remove",
			       13, 60, 4, nlist - 2, list,
			       " Remove ", " Cancel ", 0);
		if (i < 0) {
		    for (i = 0; i < nlist; i++)
			XtFree(list[i]);
		    XtFree((XtPointer)list);
		    XtFree((XtPointer)devices);
		    return (-1);
		}
		device = devices[i];
	    }
	    else
		device = devices[0];
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)devices);
	    xf86removeDevice(XF86Config, device);
	    return (0);
	}
	if (i < nlist - 2)
	    device = devices[i];
    }
    for (i = 0; i < nlist; i++)
	XtFree(list[i]);
    XtFree((XtPointer)list);
    XtFree((XtPointer)devices);

    if (device == NULL) {
	char label[32];

	device = (XF86ConfDevicePtr)XtCalloc(1, sizeof(XF86ConfDeviceRec));
	device->dev_chipid = device->dev_chiprev = device->dev_irq = -1;
	XmuSnprintf(label, sizeof(label), "Card%d", nlist ? nlist - 2 : 0);
	ClearScreen();
	refresh();
	identifier =
	    DialogInput("Card identifier",
			"Enter an identifier for your card definition:",
			11, 40, label,
			" Next >>", " Cancel ", 0);
	if (identifier == NULL) {
	    XtFree((XtPointer)device);
	    return (-1);
	}
    }

    ClearScreen();
    refresh();
    if (Dialog("Card configuration",
	       "Now we must configure video card specific settings. At this "
	       "point you can choose to make a selection out of a database of "
	       "video card definitions.\n"
	       "\n"
	       "The database entries include information about the chipset, "
	       "what driver to run, the Ramdac and ClockChip, and comments "
	       "that will be included in the Device section. However, a lot "
	       "of definitions only hint about what driver to run (based on "
	       "the chipset the card uses) and are untested.\n"
	       "\n"
	       "Do you want to look at the card database?",
	       18, 60, " Yes ", " No ", device->dev_identifier != NULL) == 0) {
	static char **cards;
	static int ncards;

	if (cards == NULL) {
	    ReadCardsDatabase();
	    cards = GetCardNames(&ncards);
	    cards = (char**)XtRealloc((XtPointer)cards,
				      (ncards + 1) * sizeof(char*));
	    for (i = ncards; i > 0; i--)
		cards[i] = cards[i - 1];
	    cards[0] = "** Unlisted card **";
	    ++ncards;
	}
	if (device->dev_card)
	    entry = LookupCard(device->dev_card);
	def = 0;
	if (entry) {
	    for (i = 0; i < NumCardsEntry; i++)
		if (strcasecmp(CardsDB[i]->name, entry->name) == 0) {
		    def = i + 1;
		    break;
		}
	    /* make sure entry is set to null again */
	    entry = NULL;
	}

	i = DialogMenu("Card database",
		       "Select name that better matches your card:",
		       20, 70, 11, ncards, cards, "Next >>", " Cancel ", def);
	if (i > 0)
	    entry = LookupCard(cards[i]);
    }

    def = 0;
    tmp = device->dev_driver ? device->dev_driver : entry && entry->driver ?
	  entry->driver : "vga";
    for (i = 0; i < ndrivers; i++)
	if (strcmp(drivers[i], tmp) == 0) {
	    def = i;
	    break;
	}

    ClearScreen();
    refresh();
    i = DialogMenu("Card driver",
		   "You can select the driver for your card here, or just press "
		   "Enter to use the default/current:", 20, 50, 9,
		   ndrivers, drivers, "  Ok  ", " Cancel ", def);
    if (i < 0) {
	if (device->dev_identifier == NULL) {
	    XtFree(identifier);
	    XtFree((XtPointer)device);
	}
	return (-1);
    }
    driver = ndrivers ? drivers[i] : "vga";

    ClearScreen();
    refresh();
    tmp = device->dev_busid ? device->dev_busid : "";
    busid = DialogInput("Card BusID",
			"You normally does not need to fill this field "
			"if you have only one video card:", 11, 50, tmp,
			" Finish ", " Cancel ", 0);

    /* Finish card configuration */
    if (entry) {
	XtFree(device->dev_card);
	device->dev_card = XtNewString(entry->name);
	if (entry->chipset) {
	    XtFree(device->dev_chipset);
	    device->dev_chipset = XtNewString(entry->chipset);
	}
	if (entry->ramdac) {
	    XtFree(device->dev_ramdac);
	    device->dev_ramdac = XtNewString(entry->ramdac);
	}
	if (entry->clockchip) {
	    XtFree(entry->clockchip);
	    device->dev_clockchip = XtNewString(entry->clockchip);
	}
    }
    if (busid) {
	XtFree(device->dev_busid);
	if (*busid)
	    device->dev_busid = busid;
	else {
	    device->dev_busid = NULL;
	    XtFree(busid);
	}
    }
    XtFree(device->dev_driver);
    device->dev_driver = XtNewString(driver);
    if (device->dev_identifier == NULL) {
	device->dev_identifier = identifier;
	XF86Config->conf_device_lst =
	    xf86addDevice(XF86Config->conf_device_lst, device);
    }

    return (1);
}

static char *depths[] = {
    "1 bit, monochrome",
    "4 bit, 16 colors",
    "8 bit, 256 colors",
    "15 bits, 32Kb colors",
    "16 bits, 65Kb colors",
    "24 bits, 16Mb colors",
};

static char *modes[] = {
    "2048x1536",
    "1920x1440",
    "1800x1400",
    "1600x1200",
    "1400x1050",
    "1280x1024",
    "1280x960",
    "1152x864",
    "1024x768",
    "800x600",
    "640x480",
    "640x400",
    "512x384",
    "400x300",
    "320x240",
    "320x200",
};

static int
ScreenConfig(void)
{
    int i, disp_allocated;
    XF86ConfScreenPtr *screens = NULL, screen = XF86Config->conf_screen_lst;
    char **list = NULL, *identifier = NULL;
    int nlist, def;
    XF86ConfDevicePtr device = NULL;
    XF86ConfMonitorPtr monitor = NULL;
    XF86ConfDisplayPtr display;
    XF86ModePtr mode, ptr = NULL;
    char *checks;

    nlist = 0;
    while (screen) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
	list[nlist] = XtMalloc(sizeof(Edit) +
			       strlen(screen->scrn_identifier) + 1);
	sprintf(list[nlist], "%s%s", Edit, screen->scrn_identifier);
	screens = (XF86ConfScreenPtr*)XtRealloc((XtPointer)screens, (nlist + 1) *
				    sizeof(XF86ConfScreenPtr));
	screens[nlist] = screen;
	++nlist;
	screen = (XF86ConfScreenPtr)(screen->list.next);
    }

    screen = NULL;

    if (nlist) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 2) * sizeof(char*));
	list[nlist++] = XtNewString("Add new screen");
	if (nlist == 2) {
	    i = strlen("Remove ") + strlen(screens[0]->scrn_identifier) + 1;
	    list[nlist] = XtMalloc(i);
	    XmuSnprintf(list[nlist], i, "Remove %s", screens[0]->scrn_identifier);
	    ++nlist;
	}
	else
	    list[nlist++] = XtNewString("Remove screen");
	ClearScreen();
	refresh();
	i = DialogMenu("Screen configuration",
		       "You can edit or remove a previously configured "
		       "screen, or add a new one.", 14, 60, 4, nlist, list,
		       " Ok  ", " Cancel ", 0);
	if (i < 0) {
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)screens);
	    return (-1);
	}
	if (nlist > 2 && i == nlist - 1) {
	    if (nlist > 3) {
		for (i = 0; i < nlist - 2; i++) {
		    /* XXX Remove the "Edit " from list entries */
		    memmove(list[i], list[i] + sizeof(Edit) - 1,
			    strlen(list[i]) - sizeof(Edit) + 2);
		}
		ClearScreen();
		refresh();
		i = DialogMenu("Remove screen",
			       "Select which screen to remove",
			       13, 60, 4, nlist - 2, list,
			       " Remove ", " Cancel ", 0);
		if (i < 0) {
		    for (i = 0; i < nlist; i++)
			XtFree(list[i]);
		    XtFree((XtPointer)list);
		    XtFree((XtPointer)screens);
		    return (-1);
		}
		screen = screens[i];
	    }
	    else
		screen = screens[0];
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)screens);
	    xf86removeScreen(XF86Config, screen);
	    return (0);
	}
	if (i < nlist - 2)
	    screen = screens[i];
    }
    for (i = 0; i < nlist; i++)
	XtFree(list[i]);
    XtFree((XtPointer)list);
    XtFree((XtPointer)screens);

    if (screen == NULL) {
	char label[256];
	XF86ConfDevicePtr *devices = NULL;
	XF86ConfMonitorPtr *monitors = NULL;

	device = XF86Config->conf_device_lst;
	monitor = XF86Config->conf_monitor_lst;

	if (device == NULL || monitor == NULL) {
		ClearScreen();
		refresh();
		Dialog("Configuration error",
		       "You need to configure (at least) one card and one "
		       "monitor before creating a screen definition.",
		       9, 50, "  Ok  ", NULL, 0);

		return (-1);
	}

	XmuSnprintf(label, sizeof(label), "Screen%d", nlist ? nlist - 2 : 0);
	ClearScreen();
	refresh();
	identifier =
	    DialogInput("Screen identifier",
			"Enter an identifier for your screen definition:",
			11, 40, label,
			" Next >>", " Cancel ", 0);
	if (identifier == NULL)
	    return (-1);

	nlist = 0;
	list = NULL;
	while (device) {
	    list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
	    list[nlist] = XtNewString(device->dev_identifier);
	    devices = (XF86ConfDevicePtr*)XtRealloc((XtPointer)devices, (nlist + 1) *
					sizeof(XF86ConfDevicePtr));
	    devices[nlist] = device;
	    ++nlist;
	    device = (XF86ConfDevicePtr)(device->list.next);
	}
	ClearScreen();
	refresh();
	i = DialogMenu("Screen card", "Please select a video card:",
		       13, 60, 4, nlist, list, " Next >>", " Cancel ", 0);
	for (def = 0; def < nlist; def++)
	    XtFree(list[def]);
	XtFree((XtPointer)list);
	if (i < 0) {
	    XtFree(identifier);
	    XtFree((XtPointer)devices);
	    return (-1);
	}
	device = devices[i];
	XtFree((XtPointer)devices);

	nlist = 0;
	list = NULL;
	while (monitor) {
	    list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
	    list[nlist] = XtNewString(monitor->mon_identifier);
	    monitors = (XF86ConfMonitorPtr*)XtRealloc((XtPointer)monitors, (nlist + 1) *
					sizeof(XF86ConfMonitorPtr));
	    monitors[nlist] = monitor;
	    ++nlist;
	    monitor = (XF86ConfMonitorPtr)(monitor->list.next);
	}
	XmuSnprintf(label, sizeof(label),
		    "Select the monitor connected to \"%s\":",
		    device->dev_identifier);
	ClearScreen();
	refresh();
	i = DialogMenu("Screen monitor", label,
		       13, 60, 4, nlist, list, " Next >>", " Cancel ", 0);
	for (def = 0; def < nlist; def++)
	    XtFree(list[def]);
	XtFree((XtPointer)list);
	if (i < 0) {
	    XtFree(identifier);
	    XtFree((XtPointer)monitors);
	    return (-1);
	}
	monitor = monitors[i];
	XtFree((XtPointer)monitors);

	screen = (XF86ConfScreenPtr)XtCalloc(1, sizeof(XF86ConfScreenRec));
	screen->scrn_device = device;
	screen->scrn_monitor = monitor;
    }

    if (screen->scrn_defaultdepth == 1)
	def = 0;
    else if (screen->scrn_defaultdepth == 4)
	def = 1;
    else if (screen->scrn_defaultdepth == 8)
	def = 2;
    else if (screen->scrn_defaultdepth == 15)
	def = 3;
    else if (screen->scrn_defaultdepth == 16)
	def = 4;
    else if (screen->scrn_defaultdepth == 24)
	def = 5;
    else {
	if (screen->scrn_device && screen->scrn_device->dev_driver &&
	    strcmp(screen->scrn_device->dev_driver, "vga") == 0)
	    def = 1;		/* 4bpp */
	else
	    def = 2;		/* 8bpp */
    }
    ClearScreen();
    refresh();
    i = DialogMenu("Screen depth",
		   "Please specify which color depth you want to use by default:",
		   15, 60, 6, sizeof(depths) / sizeof(depths[0]), depths,
		   " Next >>", " Cancel ", def);
    if (i < 0) {
	if (screen->scrn_identifier == NULL) {
	    XtFree(identifier);
	    XtFree((XtPointer)screen);
	}
	return (-1);
    }
    else
	/* XXX depths must begin with the depth number */
	screen->scrn_defaultdepth = atoi(depths[i]);

    def = 0;	/* use def to count how many modes are selected*/
    nlist = 0;
    list = NULL;
    checks = XtMalloc(sizeof(modes) / sizeof(modes[0]));
    /* XXX list fields in the code below are not allocated */
    disp_allocated = 0;
    display = screen->scrn_display_lst;
    while (display && display->disp_depth != screen->scrn_defaultdepth)
	display = (XF86ConfDisplayPtr)(display->list.next);
    if (display == NULL) {
	display = (XF86ConfDisplayPtr)XtCalloc(1, sizeof(XF86ConfDisplayRec));
	display->disp_white.red = display->disp_black.red = -1;
	display->disp_depth = screen->scrn_defaultdepth;
	disp_allocated = 1;
    }
    else {
	mode = display->disp_mode_lst;
	while (mode) {
	    for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
		if (strcmp(modes[i], mode->mode_name) == 0) {
		    break;
		}

	    if (i == sizeof(modes) / sizeof(modes[0])) {
		list = (char**)XtRealloc((XtPointer)list,
					 (nlist + 1) * sizeof(char*));
		list[nlist] = mode->mode_name;
		checks = XtRealloc(checks, sizeof(modes) / sizeof(modes[0]) +
				   nlist + 1);
		checks[nlist] = 1;
		++def;
		nlist++;
		break;
	    }
	    mode = (XF86ModePtr)(mode->list.next);
	}
    }

    for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
	checks[i + nlist] = 0;

    mode = display->disp_mode_lst;
    while (mode) {
	for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
	    if (strcmp(modes[i], mode->mode_name) == 0) {
		++def;
		checks[i + nlist] = 1;
		break;
	}
	mode = (XF86ModePtr)(mode->list.next);
    }

    if (nlist == 0 && def == 0)
	checks[7] = 1;	/* 640x480 */
    list = (char**)XtRealloc((XtPointer)list, (nlist + sizeof(modes) /
			     sizeof(modes[0])) * sizeof(char*));
    for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++)
	list[i + nlist] = modes[i];
    nlist += sizeof(modes) / sizeof(modes[0]);

    ClearScreen();
    refresh();
    i = DialogCheckBox("Screen modes",
		       "Select the video modes for this screen:",
		       17, 60, 8, sizeof(modes) / sizeof(modes[0]), modes,
		       " Finish ", " Cancel ", checks);
    if (i < 0) {
	if (screen->scrn_identifier == NULL) {
	    XtFree(identifier);
	    XtFree((XtPointer)screen);
	    XtFree((XtPointer)list);
	    if (disp_allocated)
		XtFree((XtPointer)display);
	}
	return (-1);
    }

    mode = display->disp_mode_lst;
    while (mode) {
	ptr = (XF86ModePtr)(mode->list.next);
	XtFree(mode->mode_name);
	XtFree((XtPointer)mode);
	mode = ptr;
    }
    display->disp_mode_lst = NULL;

    for (i = 0; i < nlist; i++) {
	if (checks[i]) {
	    mode = (XF86ModePtr)XtCalloc(1, sizeof(XF86ModeRec));
	    mode->mode_name = XtNewString(list[i]);
	    if (display->disp_mode_lst == NULL)
		display->disp_mode_lst = ptr = mode;
	    else {
		ptr->list.next = mode;
		ptr = mode;
	    }
	}
    }
    XtFree((XtPointer)list);

    if (disp_allocated) {
	display->list.next = NULL;
	if (screen->scrn_display_lst == NULL)
	    screen->scrn_display_lst = display;
	else
	    screen->scrn_display_lst->list.next = display;
    }

    if (screen->scrn_identifier == NULL) {
	screen->scrn_identifier = identifier;
	screen->scrn_monitor_str = XtNewString(monitor->mon_identifier);
	screen->scrn_device_str = XtNewString(device->dev_identifier);
	XF86Config->conf_screen_lst =
	    xf86addScreen(XF86Config->conf_screen_lst, screen);
    }

    return (1);
}

static XF86ConfAdjacencyPtr
CopyAdjacency(XF86ConfAdjacencyPtr ptr)
{
    XF86ConfAdjacencyPtr adj = (XF86ConfAdjacencyPtr)
	XtCalloc(1, sizeof(XF86ConfAdjacencyRec));

    adj->adj_scrnum = ptr->adj_scrnum;
    adj->adj_screen = ptr->adj_screen;
    adj->adj_screen_str = XtNewString(ptr->adj_screen_str);
    adj->adj_top = ptr->adj_top;
    if (ptr->adj_top_str)
	adj->adj_top_str = XtNewString(ptr->adj_top_str);
    adj->adj_bottom = ptr->adj_bottom;
    if (ptr->adj_bottom_str)
	adj->adj_bottom_str = XtNewString(ptr->adj_bottom_str);
    adj->adj_left = ptr->adj_left;
    if (ptr->adj_left_str)
	adj->adj_left_str = XtNewString(ptr->adj_left_str);
    adj->adj_right = ptr->adj_right;
    if (ptr->adj_right_str)
	adj->adj_right_str = XtNewString(ptr->adj_right_str);
    adj->adj_where = ptr->adj_where;
    adj->adj_x = ptr->adj_x;
    adj->adj_y = ptr->adj_y;
    if (ptr->adj_refscreen)
	adj->adj_refscreen = XtNewString(ptr->adj_refscreen);

    return (adj);
}

static XF86ConfInactivePtr
CopyInactive(XF86ConfInactivePtr ptr)
{
    XF86ConfInactivePtr inac = (XF86ConfInactivePtr)
	XtCalloc(1, sizeof(XF86ConfInactiveRec));

    inac->inactive_device = ptr->inactive_device;
    if (ptr->inactive_device_str)
	inac->inactive_device_str = XtNewString(ptr->inactive_device_str);

    return (inac);
}

static XF86ConfInputrefPtr
CopyInputref(XF86ConfInputrefPtr ptr)
{
    XF86ConfInputrefPtr iref = (XF86ConfInputrefPtr)
	XtCalloc(1, sizeof(XF86ConfInputrefRec));
    XF86OptionPtr opt = ptr->iref_option_lst;

    iref->iref_inputdev = ptr->iref_inputdev;
    if (ptr->iref_inputdev_str)
	iref->iref_inputdev_str = XtNewString(ptr->iref_inputdev_str);
    while (opt) {
	iref->iref_option_lst = xf86addNewOption(iref->iref_option_lst,
	    XtNewString(opt->opt_name),
	    opt->opt_val ? XtNewString(opt->opt_val) : NULL);
	opt = (XF86OptionPtr)(opt->list.next);
    }

    return (iref);
}

static XF86ConfLayoutPtr
CopyLayout(XF86ConfLayoutPtr ptr)
{
    XF86ConfLayoutPtr lay = (XF86ConfLayoutPtr)
	XtCalloc(1, sizeof(XF86ConfLayoutRec));
    XF86ConfAdjacencyPtr adj = ptr->lay_adjacency_lst, padj;
    XF86ConfInactivePtr inac = ptr->lay_inactive_lst, pinac;
    XF86ConfInputrefPtr iref = ptr->lay_input_lst, piref;
    XF86OptionPtr opt = ptr->lay_option_lst;

    if (ptr->lay_identifier)
	lay->lay_identifier = XtNewString(ptr->lay_identifier);
    if (adj) {
	padj = lay->lay_adjacency_lst = CopyAdjacency(adj);
	adj = (XF86ConfAdjacencyPtr)(adj->list.next);
	while (adj) {
	    padj->list.next = CopyAdjacency(adj);
	    padj = (XF86ConfAdjacencyPtr)(padj->list.next);
	    adj = (XF86ConfAdjacencyPtr)(adj->list.next);
	}
    }
    if (inac) {
	pinac = lay->lay_inactive_lst = CopyInactive(inac);
	inac = (XF86ConfInactivePtr)(inac->list.next);
	while (inac) {
	    pinac->list.next = CopyInactive(inac);
	    pinac = (XF86ConfInactivePtr)(pinac->list.next);
	    inac = (XF86ConfInactivePtr)(inac->list.next);
	}
    }
    if (iref) {
	piref = lay->lay_input_lst = CopyInputref(iref);
	iref = (XF86ConfInputrefPtr)(iref->list.next);
	while (iref) {
	    piref->list.next = CopyInputref(iref);
	    piref = (XF86ConfInputrefPtr)(piref->list.next);
	    iref = (XF86ConfInputrefPtr)(iref->list.next);
	}
    }

    while (opt) {
	lay->lay_option_lst = xf86addNewOption(lay->lay_option_lst,
	    XtNewString(opt->opt_name),
	    opt->opt_val ? XtNewString(opt->opt_val) : NULL);
	opt = (XF86OptionPtr)(opt->list.next);
    }

    return (lay);
}

static void
FreeLayout(XF86ConfLayoutPtr lay)
{
    static XF86ConfigRec xf86config;

    xf86config.conf_layout_lst = lay;
    xf86removeLayout(&xf86config, lay);
}

static int
LayoutConfig(void)
{
    int i;
    XF86ConfLayoutPtr *layouts = NULL, rlayout = NULL,
		       layout = XF86Config->conf_layout_lst;
    XF86ConfInputPtr input = XF86Config->conf_input_lst;
    char **list = NULL, *identifier = NULL;
    XF86ConfInputPtr *mouses = NULL, *keyboards = NULL, mouse, keyboard;
    XF86ConfInputrefPtr iref, piref, mref, kref;
    XF86ConfAdjacencyPtr adj, padj;
    int nmouses, nkeyboards;
    int nlist;
    XF86OptionPtr option;
    XF86ConfScreenPtr screen, *screens;

    nmouses = nkeyboards = 0;
    while (input) {
	if (strcmp(input->inp_driver, "mouse") == 0) {
	    mouses = (XF86ConfInputPtr*)XtRealloc((XtPointer)mouses,
			(nmouses + 1) * sizeof(XF86ConfInputPtr));
	    mouses[nmouses] = input;
	    ++nmouses;
	}
	else if (IS_KBDDRIV(input->inp_driver)) {
	    keyboards = (XF86ConfInputPtr*)XtRealloc((XtPointer)keyboards,
			    (nkeyboards + 1) * sizeof(XF86ConfInputPtr));
	    keyboards[nkeyboards] = input;
	    ++nkeyboards;
	}
	input = (XF86ConfInputPtr)(input->list.next);
    }
    if (XF86Config->conf_screen_lst == NULL ||
	nmouses == 0 || nkeyboards == 0) {
	XtFree((XtPointer)mouses);
	XtFree((XtPointer)keyboards);
	ClearScreen();
	refresh();
	Dialog("Configuration error",
	       "You need to configure (at least) one screen, mouse "
	       "and keyboard before creating a layout definition.",
	       9, 50, "  Ok  ", NULL, 0);
	return (-1);
    }

    nlist = 0;
    while (layout) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
	list[nlist] = XtMalloc(sizeof(Edit) +
			       strlen(layout->lay_identifier) + 1);
	sprintf(list[nlist], "%s%s", Edit, layout->lay_identifier);
	layouts = (XF86ConfLayoutPtr*)XtRealloc((XtPointer)layouts, (nlist + 1) *
				    sizeof(XF86ConfLayoutPtr));
	layouts[nlist] = layout;
	++nlist;
	layout = (XF86ConfLayoutPtr)(layout->list.next);
    }

    layout = NULL;

    if (nlist) {
	list = (char**)XtRealloc((XtPointer)list, (nlist + 2) * sizeof(char*));
	list[nlist++] = XtNewString("Add new layout");
	if (nlist == 2) {
	    i = strlen("Remove ") + strlen(layouts[0]->lay_identifier) + 1;
	    list[nlist] = XtMalloc(i);
	    XmuSnprintf(list[nlist], i, "Remove %s", layouts[0]->lay_identifier);
	    ++nlist;
	}
	else
	    list[nlist++] = XtNewString("Remove layout");
	ClearScreen();
	refresh();
	i = DialogMenu("Layout configuration",
		       "You can edit or remove a previously configured "
		       "layout, or add a new one.", 14, 60, 4, nlist, list,
		       " Ok  ", " Cancel ", 0);
	if (i < 0) {
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)layouts);
	    XtFree((XtPointer)mouses);
	    XtFree((XtPointer)keyboards);
	    return (-1);
	}
	if (nlist > 2 && i == nlist - 1) {
	    if (nlist > 3) {
		for (i = 0; i < nlist - 2; i++) {
		    /* XXX Remove the "Edit " from list entries */
		    memmove(list[i], list[i] + sizeof(Edit) - 1,
			    strlen(list[i]) - sizeof(Edit) + 2);
		}
		ClearScreen();
		refresh();
		i = DialogMenu("Remove layout",
			       "Select which layout to remove",
			       13, 60, 4, nlist - 2, list,
			       " Remove ", " Cancel ", 0);
		if (i < 0) {
		    for (i = 0; i < nlist; i++)
			XtFree(list[i]);
		    XtFree((XtPointer)list);
		    XtFree((XtPointer)layouts);
		    XtFree((XtPointer)mouses);
		    XtFree((XtPointer)keyboards);
		    return (-1);
		}
		layout = layouts[i];
	    }
	    else
		layout = layouts[0];
	    for (i = 0; i < nlist; i++)
		XtFree(list[i]);
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)layouts);
	    XtFree((XtPointer)mouses);
	    XtFree((XtPointer)keyboards);
	    xf86removeLayout(XF86Config, layout);
	    return (0);
	}
	if (i < nlist - 2)
	    layout = layouts[i];
    }
    for (i = 0; i < nlist; i++)
	XtFree(list[i]);
    XtFree((XtPointer)list);
    XtFree((XtPointer)layouts);

    if (layout == NULL) {
	char label[32];

	layout = (XF86ConfLayoutPtr)XtCalloc(1, sizeof(XF86ConfLayoutRec));
	XmuSnprintf(label, sizeof(label), "Layout%d", nlist ? nlist - 2 : 0);
	ClearScreen();
	refresh();
	identifier =
	    DialogInput("Layout identifier",
			"Enter an identifier for your layout definition:",
			11, 40, label,
			" Next >>", " Cancel ", 0);
	if (identifier == NULL) {
	    XtFree((XtPointer)layout);
	    XtFree((XtPointer)mouses);
	    XtFree((XtPointer)keyboards);
	    return (-1);
	}
    }
    else {
	/* So that we can safely change it */
	rlayout = layout;
	layout = CopyLayout(rlayout);
    }


    mouse = keyboard = NULL;

    /*  Mouse */
    piref = NULL;
    iref = layout->lay_input_lst;
    while (iref) {
	if (strcmp(iref->iref_inputdev->inp_driver, "mouse") == 0) {
	    if (mouse == NULL)
		piref = iref;
	    if (xf86findOption(iref->iref_option_lst, "CorePointer")) {
		mouse = iref->iref_inputdev;
		piref = iref;
		break;
	    }
	}
	iref = (XF86ConfInputrefPtr)(iref->list.next);
    }
    if (mouse == NULL) {
	if (piref) {
	    mref = piref;
	    mouse = piref->iref_inputdev;
	    piref->iref_option_lst =
		xf86addNewOption(piref->iref_option_lst,
			       XtNewString("CorePointer"), NULL);
	}
	else {
	    mouse = mouses[0];
	    mref = iref = (XF86ConfInputrefPtr)XtCalloc(1, sizeof(XF86ConfInputrefRec));
	    iref->iref_inputdev_str = XtNewString(mouse->inp_identifier);
	    iref->iref_inputdev = mouse;
	    iref->iref_option_lst =
		    xf86addNewOption(iref->iref_option_lst,
				   XtNewString("CorePointer"), NULL);
	    iref->list.next = layout->lay_input_lst;
	    if (layout->lay_input_lst == NULL)
		layout->lay_input_lst = iref;
	    else {
		iref->list.next = layout->lay_input_lst;
		layout->lay_input_lst = iref;
	    }
	}
    }
    else
	mref = piref;

    /* XXX list fields are not allocated */
    if (nmouses > 1) {
	nlist = 0;
	list = (char**)XtMalloc(sizeof(char*));
	list[nlist++] = mouse->inp_identifier;
	input = XF86Config->conf_input_lst;
	while (input) {
	    if (input != mouse && strcmp(input->inp_driver, "mouse") == 0) {
		list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
		list[nlist++] = input->inp_identifier;
	    }
	    input = (XF86ConfInputPtr)(input->list.next);
	}
	ClearScreen();
	refresh();
	i = DialogMenu("Select Core Pointer",
		       "Select the mouse connected to you computer",
		       12, 60, 4, nlist, list, "  Ok  ", " Cancel ", 0);
	if (i < 0) {
	    XtFree((XtPointer)mouses);
	    XtFree((XtPointer)keyboards);
	    XtFree((XtPointer)list);
	    if (layout->lay_identifier == NULL)
		XtFree(identifier);
	    FreeLayout(layout);
	    return (-1);
	}
	if (i > 0) {
	    /* Did not select the default one */
	    iref = layout->lay_input_lst;
	    while (iref) {
		if (strcasecmp(iref->iref_inputdev_str, list[i]) == 0) {
		    if ((option = xf86findOption(iref->iref_option_lst,
						 "SendCoreEvents")) != NULL) {
			XtFree(option->opt_name);
			option->opt_name = XtNewString("CorePointer");
		    }
		    else
			iref->iref_option_lst =
			    xf86addNewOption(iref->iref_option_lst,
					  XtNewString("CorePointer"), NULL);
		    option = xf86findOption(mref->iref_option_lst,
					    "CorePointer");
		    XtFree(option->opt_name);
		    option->opt_name = XtNewString("SendCoreEvents");
		    break;
		}
		iref = (XF86ConfInputrefPtr)(iref->list.next);
	    }
	}

	/* XXX Write code to add/remove more mouses here */
    }


    /*  Keyboard */
    piref = NULL;
    iref = layout->lay_input_lst;
    while (iref) {
	if (IS_KBDDRIV(iref->iref_inputdev->inp_driver)) {
	    if (keyboard == NULL)
		piref = iref;
	    if (xf86findOption(iref->iref_option_lst, "CoreKeyboard")) {
		keyboard = iref->iref_inputdev;
		piref = iref;
		break;
	    }
	}
	iref = (XF86ConfInputrefPtr)(iref->list.next);
    }
    if (keyboard == NULL) {
	if (piref) {
	    kref = piref;
	    keyboard = piref->iref_inputdev;
	    piref->iref_option_lst =
		xf86addNewOption(piref->iref_option_lst,
			       XtNewString("CoreKeyboard"), NULL);
	}
	else {
	    keyboard = keyboards[0];
	    kref = iref = (XF86ConfInputrefPtr)XtCalloc(1, sizeof(XF86ConfInputrefRec));
	    iref->iref_inputdev_str = XtNewString(keyboard->inp_identifier);
	    iref->iref_inputdev = keyboard;
	    iref->iref_option_lst =
		    xf86addNewOption(iref->iref_option_lst,
				   XtNewString("CoreKeyboard"), NULL);
	    iref->list.next = layout->lay_input_lst;
	    if (layout->lay_input_lst == NULL)
		layout->lay_input_lst = iref;
	    else {
		iref->list.next = layout->lay_input_lst;
		layout->lay_input_lst = iref;
	    }
	}
    }
    else
	kref = piref;

    /* XXX list fields are not allocated */
    if (nkeyboards > 1) {
	nlist = 0;
	list = (char**)XtMalloc(sizeof(char*));
	list[nlist++] = keyboard->inp_identifier;
	input = XF86Config->conf_input_lst;
	while (input) {
	    if (input != keyboard && IS_KBDDRIV(input->inp_driver)) {
		list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
		list[nlist++] = input->inp_identifier;
	    }
	    input = (XF86ConfInputPtr)(input->list.next);
	}
	ClearScreen();
	refresh();
	i = DialogMenu("Select Core Keyboard",
		       "Select the keyboard connected to you computer",
		       12, 60, 4, nlist, list, "  Ok  ", " Cancel ", 0);
	if (i < 0) {
	    XtFree((XtPointer)mouses);
	    XtFree((XtPointer)keyboards);
	    XtFree((XtPointer)list);
	    if (layout->lay_identifier == NULL)
		XtFree(identifier);
	    FreeLayout(layout);
	    return (-1);
	}
	if (i > 0) {
	    /* Did not select the default one */
	    iref = layout->lay_input_lst;
	    while (iref) {
		if (strcasecmp(iref->iref_inputdev_str, list[i]) == 0) {
		    if ((option = xf86findOption(iref->iref_option_lst,
						 "SendCoreEvents")) != NULL) {
			XtFree(option->opt_name);
			option->opt_name = XtNewString("CoreKeyboard");
		    }
		    else
			iref->iref_option_lst =
			    xf86addNewOption(iref->iref_option_lst,
					  XtNewString("CoreKeyboard"), NULL);
		    option = xf86findOption(kref->iref_option_lst,
					    "CoreKeyboard");
		    XtFree(option->opt_name);
		    option->opt_name = XtNewString("SendCoreEvents");
		    break;
		}
		iref = (XF86ConfInputrefPtr)(iref->list.next);
	    }
	}

	/* XXX Write code to add/remove more keyboards here */
    }

    XtFree((XtPointer)mouses);
    XtFree((XtPointer)keyboards);

    /* Just one screen */
    if (XF86Config->conf_screen_lst->list.next == NULL) {
	ClearScreen();
	refresh();
	Dialog("Layout configuration",
	       (nmouses > 1 || nkeyboards > 1) ?
	       "As you have only one screen configured, I can now finish "
	       "creating this Layout configuration."
		:
	       "As you have only one screen, mouse and keyboard configured, "
	       "I can now finish creating this Layout configuration.",
	       12, 60, " Finish ", NULL, 0);

	goto LayoutFinish;
    }


    /* The code below just adds a screen to the right of the last
     * one, or allows removing a screen.
     * Needs some review, and adding more options.
     */

    /*CONSTCOND*/
    while (1) {
	static char *screen_opts[] = {
	    "Add a new screen to layout",
	    "Remove screen from layout",
	    "Finish layout configuration",
	};

	ClearScreen();
	refresh();
	i = DialogMenu("Layout configuration", "Please choose one option:",
		       12, 60, 3, sizeof(screen_opts) / sizeof(screen_opts[0]),
		       screen_opts, " Done ", " Cancel all changes ", 2);

	/* cancel */
	if (i < 0) {
	    XtFree(identifier);
	    FreeLayout(layout);
	    return (-1);
	}

	/* add new screen */
	else if (i == 0) {
	    nlist = 0;
	    list = NULL;
	    screens = NULL;
	    screen = XF86Config->conf_screen_lst;
	    while (screen) {
		adj = layout->lay_adjacency_lst;
		while (adj) {
		    if (adj->adj_screen == screen)
			break;
		    adj = (XF86ConfAdjacencyPtr)(adj->list.next);
		}
		if (adj == NULL) {
		    list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
		    screens = (XF86ConfScreenPtr*)XtRealloc((XtPointer)screens,
			       (nlist + 1) * sizeof(XF86ConfScreenPtr));
		    /* NOT duplicated */
		    list[nlist] = screen->scrn_identifier;
		    screens[nlist] = screen;
		    ++nlist;
		}
		screen = (XF86ConfScreenPtr)(screen->list.next);
	    }

	    if (nlist == 0)
		continue;

	    ClearScreen();
	    refresh();
	    i = DialogMenu("Layout add screen", "Choose screen to add:",
			   12, 60, 3, nlist, list,
			   " Add ", " Cancel ", 0);
	    if (i >= 0) {
		padj = layout->lay_adjacency_lst;
		adj = (XF86ConfAdjacencyPtr)
			XtCalloc(1, sizeof(XF86ConfAdjacencyRec));
		adj->adj_screen = screens[i];
		if (padj == NULL) {
		    adj->adj_where = CONF_ADJ_ABSOLUTE;
		    layout->lay_adjacency_lst = adj;
		}
		else {
		    while (padj->list.next)
			padj = (XF86ConfAdjacencyPtr)(padj->list.next);
		    padj->list.next = adj;
		    adj->adj_where = CONF_ADJ_RIGHTOF;
		    adj->adj_refscreen =
			XtNewString(padj->adj_screen->scrn_identifier);
		}
	    }
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)screens);
	}

	/* remove a screen */
	else if (i == 1) {
	    nlist = 0;
	    list = NULL;
	    screens = NULL;
	    adj = layout->lay_adjacency_lst;

	    while (adj) {
		list = (char**)XtRealloc((XtPointer)list, (nlist + 1) * sizeof(char*));
		screens = (XF86ConfScreenPtr*)XtRealloc((XtPointer)screens,
			   (nlist + 1) * sizeof(XF86ConfScreenPtr));
		list[nlist] = adj->adj_screen->scrn_identifier;
		screens[nlist] = adj->adj_screen;
		++nlist;
		adj = (XF86ConfAdjacencyPtr)(adj->list.next);
	    }

	    if (nlist == 0)
		continue;

	    ClearScreen();
	    refresh();
	    i = DialogMenu("Layout remove screen", "Choose screen to remove:",
			   12, 60, 3, nlist, list,
			   " Remove ", " Cancel ", 0);

	    adj = padj = layout->lay_adjacency_lst;
	    while (adj) {
		if (adj->adj_screen == screens[i]) {
		    padj = (XF86ConfAdjacencyPtr)(padj->list.next);
		    if (padj && adj->adj_where == CONF_ADJ_RIGHTOF &&
			padj->adj_where == CONF_ADJ_RIGHTOF) {
			XtFree(padj->adj_refscreen);
			padj->adj_refscreen = XtNewString(adj->adj_refscreen);
		    }
		    xf86removeAdjacency(layout, adj);
		    break;
		}
		padj = adj;
		adj = (XF86ConfAdjacencyPtr)(padj->list.next);
	    }
	    XtFree((XtPointer)list);
	    XtFree((XtPointer)screens);
	}

	/* finish screen configuration */
	else
	    break;
    }

LayoutFinish:
    if (layout->lay_adjacency_lst == NULL) {
	adj = (XF86ConfAdjacencyPtr)XtCalloc(1, sizeof(XF86ConfAdjacencyRec));
	adj->adj_screen = XF86Config->conf_screen_lst;
	adj->adj_screen_str = XtNewString(XF86Config->conf_screen_lst->scrn_identifier);
	adj->adj_where = CONF_ADJ_ABSOLUTE;
	layout->lay_adjacency_lst = adj;
    }
    if (rlayout) {
	/* just edited this layout */
	if (nmouses > 1 || nkeyboards > 1) {
	    XF86ConfAdjacencyPtr tadj = rlayout->lay_adjacency_lst;
	    XF86ConfInactivePtr tinac = rlayout->lay_inactive_lst;
	    XF86ConfInputrefPtr tinp = rlayout->lay_input_lst;

	    rlayout->lay_adjacency_lst = layout->lay_adjacency_lst;
	    rlayout->lay_inactive_lst = layout->lay_inactive_lst;
	    rlayout->lay_input_lst = layout->lay_input_lst;

	    layout->lay_adjacency_lst = tadj;
	    layout->lay_inactive_lst = tinac;
	    layout->lay_input_lst = tinp;
	    FreeLayout(layout);
	}
	return (0);
    }
    else {
	layout->lay_identifier = identifier;
	XF86Config->conf_layout_lst =
	    xf86addLayout(XF86Config->conf_layout_lst, layout);
    }

    return (1);
}

static void
ClearScreen(void)
{
    int i, j;

    wattrset(stdscr, screen_attr);
    for (i = 0; i < LINES; i++) {
	wmove(stdscr, i, 0);
	for (j = 0; j < COLS; j++)
	    waddch(stdscr, ACS_PLUS);
    }
    touchwin(stdscr);
}

static int
Dialog(char *title, char * prompt, int height, int width,
       char *label1, char *label2, int button)
{
    int x, x1, x2, y, key, l1len, l2len;
    WINDOW *dialog;

    x = (COLS - width) / 2;
    y = (LINES - height) / 2;
  
    dialog = newwin(height, width, y, x);
    keypad(dialog, TRUE);

    PaintWindow(dialog, title, 0, 0, height, width);
    wattrset(dialog, dialog_attr);
    PrintWrap(dialog, prompt, width - 3, 2, 3);

    l1len = strlen(label1);
    if (label2)
	l2len = strlen(label2);
    else {
	l2len = button = 0;
    }

    x1 = (width - (l1len + l2len)) / (label2 ? 3 : 2);
    x2 = x1 + x1 + l1len;
    y = height - 3;
    if (!button) {
	if (label2)
	    PaintButton(dialog, label2, y, x2, FALSE);
	PaintButton(dialog, label1, y, x1, TRUE);
    }
    else {
	PaintButton(dialog, label1, y, x1, FALSE);
	if (label2)
	    PaintButton(dialog, label2, y, x2, TRUE);
    }
    wrefresh(dialog);

    /*CONSTCOND*/
    while (1) {
	key = wgetch(dialog);
	    switch (key) {
		case KEY_LEFT:
		case KEY_RIGHT:
		    if (!button) {
			if (label2) {
			    button = 1;
			    PaintButton(dialog, label1, y, x1, FALSE);
			    PaintButton(dialog, label2, y, x2, TRUE);
			}

		    }
		    else {
			if (label2) {
			    button = 0;
			    PaintButton(dialog, label2, y, x2, FALSE);
			    PaintButton(dialog, label1, y, x1, TRUE);
			}
		    }
		    wrefresh(dialog);
		    break;
		case ' ':
		case '\r':
		case '\n':
		    delwin(dialog);
		    return button;
	}
    }
    /*NOTREACHED*/
}

static void
PaintWindow(WINDOW *win, char *title_str, int y, int x, int height, int width)
{
    int i, j;

    if (title_str != NULL) {
	j = (width - strlen(title_str)) / 2 - 1;

	wattrset(win, title_attr);
	wmove(win, x, y);
	for (i = 0; i < j; i++)
	    waddch(win, ' ');
	waddstr(win, title_str);
	for (; i < width; i++)
	    waddch(win, ' ');
    }

    wattrset(win, 0);

    for (i = 1; i < height; i++) {
	wmove(win, y + i, x);
	for (j = 0; j < width; j++)
	    if (i == height - 1 && !j)
		waddch(win, highlight_border_attr | ACS_LLCORNER);
	    else if (i == height - 1 && j == width - 1)
		waddch(win, shadow_border_attr | ACS_LRCORNER);
	    else if (i == height - 1)
		waddch(win, shadow_border_attr | ACS_HLINE);
	    else if (!j)
		waddch(win, highlight_border_attr | ACS_VLINE);
	    else if (j == width - 1)
		waddch(win, shadow_border_attr | ACS_VLINE);
	    else
		waddch(win, dialog_attr | ' ');
    }

}

static void
PaintBox(WINDOW *win, int y, int x, int height, int width)
{
    int i, j;

    wattrset(win, 0);

    for (i = 0; i < height; i++) {
	wmove(win, y + i, x);
	for (j = 0; j < width; j++)
	    if (!i && !j)
		waddch(win, shadow_border_attr | ACS_ULCORNER);
	    else if (i == height - 1 && !j)
		waddch(win, shadow_border_attr | ACS_LLCORNER);
	    else if (!i && j == width-1)
		waddch(win, highlight_border_attr | ACS_URCORNER);
	    else if (i == height - 1 && j == width - 1)
		waddch(win, highlight_border_attr | ACS_LRCORNER);
	    else if (!i)
		waddch(win, shadow_border_attr | ACS_HLINE);
	    else if (i == height - 1)
		waddch(win, highlight_border_attr | ACS_HLINE);
	    else if (!j)
		waddch(win, shadow_border_attr | ACS_VLINE);
	    else if (j == width - 1)
		waddch(win, highlight_border_attr | ACS_VLINE);
	    else
		waddch(win, dialog_attr | ' ');
    }

}

static void
PaintButton(WINDOW *win, char *label, int y, int x, int selected)
{
    int i, temp;

    wmove(win, y, x);
    wattrset(win, selected ? button_active_attr : button_inactive_attr);
    waddstr(win, selected ? "[" : " ");
    temp = strspn(label, " ");
    label += temp;
    wattrset(win, selected ? button_active_attr : button_inactive_attr);
    for (i = 0; i < temp; i++)
      waddch(win, ' ');
    wattrset(win, selected ? button_active_attr : button_inactive_attr);
    waddch(win, label[0]);
    wattrset(win, selected ? button_active_attr : button_inactive_attr);
    waddstr(win, label + 1);
    wattrset(win, selected ? button_active_attr : button_inactive_attr);
    waddstr(win, selected ? "]" : " ");
    wmove(win, y, x + temp + 1);
}

static void
PrintWrap(WINDOW *win, char *prompt, int width, int y, int x)
{
    int cur_x, cur_y, len, yinc;
    char *word, *tempstr = XtMalloc(strlen(prompt) + 1);

    cur_x = x;
    cur_y = y;

    while (*prompt == '\n') {
	++cur_y;
	++prompt;
    }

    strcpy(tempstr, prompt);

    for (word = strtok(tempstr, " \n"); word != NULL; word = strtok(NULL, " \n")) {
	yinc = 0;
	len = strlen(word);
	while (prompt[word - tempstr + len + yinc] == '\n')
	    ++yinc;
	if (cur_x + strlen(word) > width) {
	    cur_y++;
	    cur_x = x;
	}
	wmove(win, cur_y, cur_x);
	waddstr(win, word);
	getyx(win, cur_y, cur_x);
	if (yinc) {
	    cur_y += yinc;
	    cur_x = x;
	}
	else
	    cur_x++;
    }

    free(tempstr);
}

static int
DialogMenu(char *title, char *prompt, int height, int width, int menu_height,
	   int item_no, char **items, char *label1, char *label2, int choice)
{
    int i, x, y, cur_x, cur_y, box_x, box_y, key = 0, button = 0,
	scrlx = 0, max_choice, nscroll, max_scroll, x1, x2, l1len, l2len;
    WINDOW *dialog, *menu;

    max_choice = MIN(menu_height, item_no);
    max_scroll = MAX(0, item_no - max_choice);

    x = (COLS - width) / 2;
    y = (LINES - height) / 2;
  
    dialog = newwin(height, width, y, x);
    keypad(dialog, TRUE);

    PaintWindow(dialog, title, 0, 0, height, width);

    wattrset(dialog, dialog_attr);
    PrintWrap(dialog, prompt, width - 3, 2, 3);

    l1len = strlen(label1);
    l2len = strlen(label2);

    x1 = (width - (l1len + l2len)) / 3;
    x2 = x1 + x1 + l1len;

    menu_width = width - 6;
    getyx(dialog, cur_y, cur_x);
    box_y = cur_y + 1;
    box_x = (width - menu_width) / 2 - 1;

    menu = subwin(dialog, menu_height, menu_width, y + box_y + 1, x + box_x + 1);
    keypad(menu, TRUE);

    /* draw a box around the menu items */
    PaintBox(dialog, box_y, box_x, menu_height + 2, menu_width + 2);

    item_x = 3;

    if (choice > menu_height) {
	scrlx = MIN(max_scroll, choice);
	choice -= scrlx;
    }

    for (i = 0; i < max_choice; i++)
	PaintItem(menu, items[i + scrlx], i, i == choice);
    PaintScroller(menu, scrlx + choice, item_no, menu_height);
    wnoutrefresh(menu);

    x = width / 2 - 11;
    y = height - 3;
    PaintButton(dialog, label2, y, x2, FALSE);
    PaintButton(dialog, label1, y, x1, TRUE);
    wrefresh(dialog);

    /*CONSTCOND*/
    while (1) {
	i = choice;
	key = wgetch(dialog);

	if (menu_height > 1 && key == KEY_PPAGE) {
	    if (!choice) {
		if (scrlx) {
		    /* Scroll menu down */
		    getyx(dialog, cur_y, cur_x);

		    nscroll = max_choice > scrlx ? -scrlx : -max_choice;
		    scrollok(menu, TRUE);
		    wscrl(menu, nscroll);
		    scrollok(menu, FALSE);

		    PaintItem(menu, items[i = scrlx + nscroll], 0, TRUE);
		    for (++i; i <= scrlx; i++)
			PaintItem(menu, items[i], i - (scrlx + nscroll), FALSE);
		    scrlx += nscroll;
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wrefresh(dialog);
		    continue;
		}
	    }
	    i = 0;
	}
	else if (menu_height > 1 && key == KEY_NPAGE) {
	    if (choice == max_choice - 1) {
		if (scrlx < max_scroll) {
		    /* Scroll menu up */
		    getyx(dialog, cur_y, cur_x);

		    nscroll = (scrlx + max_choice > max_scroll ?
			       max_scroll : scrlx + max_choice) - scrlx;
		    scrollok(menu, TRUE);
		    wscrl(menu, nscroll);
		    scrollok(menu, FALSE);

		    scrlx += nscroll;
		    for (i = 0; i < max_choice - 1; i++)
			PaintItem(menu, items[i + scrlx], i, FALSE);
		    PaintItem(menu, items[i + scrlx], max_choice - 1, TRUE);
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wrefresh(dialog);
		    continue;
		}
	    }
	    i = max_choice - 1;
	}
	else if (key == KEY_UP) {
	    if (!choice) {
		if (scrlx) {
		    /* Scroll menu down */
		    getyx(dialog, cur_y, cur_x);
		    if (menu_height > 1) {
			PaintItem(menu, items[scrlx], 0, FALSE);
			scrollok(menu, TRUE);
			wscrl(menu, - 1);
			scrollok(menu, FALSE);
		    }
		    scrlx--;
		    PaintItem(menu, items[scrlx], 0, TRUE);
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wrefresh(dialog);
		    continue;
		}
	    }
	    else
		i = choice - 1;
	}
	else if (key == KEY_DOWN) {
	    if (choice == max_choice - 1) {
		if (scrlx + choice < item_no - 1) {
		    /* Scroll menu up */
		    getyx(dialog, cur_y, cur_x);
		    if (menu_height > 1) {
			PaintItem(menu, items[scrlx + max_choice - 1], max_choice - 1, FALSE);
			scrollok(menu, TRUE);
			scroll(menu);
			scrollok(menu, FALSE);
		    }
		    scrlx++;
		    PaintItem(menu, items[scrlx + max_choice - 1], max_choice - 1, TRUE);
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wrefresh(dialog);
		    continue;
		}
	    }
	    else
		i = MIN(choice + 1, item_no - 1);
	}

	if (i != choice) {
	    getyx(dialog, cur_y, cur_x);
	    PaintItem(menu, items[scrlx + choice], choice, FALSE);

	    choice = i;
	    PaintItem(menu, items[scrlx + choice], choice, TRUE);
	    PaintScroller(menu, scrlx + choice, item_no, menu_height);
	    wnoutrefresh(menu);
	    wmove(dialog, cur_y, cur_x);
	    wrefresh(dialog);
	    continue;
	}

	switch (key) {
	    case TAB:
	    case KEY_LEFT:
	    case KEY_RIGHT:
		if (!button) {
		    button = 1;
		    PaintButton(dialog, label1, y, x1, FALSE);
		    PaintButton(dialog, label2, y, x2, TRUE);
		}
		else {
		    button = 0;
		    PaintButton(dialog, label2, y, x2, FALSE);
		    PaintButton(dialog, label1, y, x1, TRUE);
		}
		wrefresh(dialog);
		break;
	    case ' ':
	    case '\r':
	    case '\n':
		delwin(dialog);
		return (!button ? scrlx + choice : -1);
	    default:
		for (i = scrlx + choice + 1; i < item_no; i++)
		    if (toupper(items[i][0]) == toupper(key))
			break;
		if (i == item_no) {
		    for (i = 0; i < scrlx + choice; i++)
			if (toupper(items[i][0]) == toupper(key))
			    break;
		}
		getyx(dialog, cur_y, cur_x);
		if (i < item_no && i != scrlx + choice) {
		    if (i >= scrlx && i < scrlx + max_choice) {
			/* it is already visible */
			PaintItem(menu, items[scrlx + choice], choice, FALSE);
			choice = i - scrlx;
		    }
		    else {
			scrlx = MIN(i, max_scroll);
			choice = i - scrlx;
			for (i = 0; i < max_choice; i++)
			    if (i != choice)
				PaintItem(menu, items[scrlx + i], i, FALSE);
		    }
		    PaintItem(menu, items[scrlx + choice], choice, TRUE);
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wmove(dialog, cur_y, cur_x);
		    wrefresh(dialog);
		}
		break;
	}
    }
    /*NOTREACHED*/
}

static void
PaintItem(WINDOW *win, char *item, int choice, int selected)
{
    int i;

    wattrset(win, selected ? title_attr : dialog_attr);
    wmove(win, choice, 1);
    for (i = 1; i < menu_width; i++)
	waddch(win, ' ');
    wmove(win, choice, item_x);
    wattrset(win, selected ? title_attr : dialog_attr);
    waddstr(win, item);
}

static void
PaintScroller(WINDOW *win, int offset, int lenght, int visible)
{
    int i, pos;

    if (lenght > visible)
	pos = (visible / (double)lenght) * offset;
    else
	pos = offset;
    wattrset(win, shadow_border_attr);
    for (i = 0; i < visible; i++) {
	wmove(win, i, 0);
	waddch(win, i == pos ? ACS_BLOCK : ACS_VLINE);
    }
}

static int
DialogCheckBox(char *title, char *prompt, int height, int width, int menu_height,
	       int item_no, char **items, char *label1, char *label2, char *checks)
{
    int i, x, y, cur_x, cur_y, box_x, box_y, key = 0, button = 0, choice = 0,
	scrlx = 0, max_choice, nscroll, max_scroll, x1, x2, l1len, l2len;
    WINDOW *dialog, *menu;

    max_choice = MIN(menu_height, item_no);
    max_scroll = MAX(0, item_no - max_choice);

    x = (COLS - width) / 2;
    y = (LINES - height) / 2;
  
    dialog = newwin(height, width, y, x);
    keypad(dialog, TRUE);

    PaintWindow(dialog, title, 0, 0, height, width);

    wattrset(dialog, dialog_attr);
    PrintWrap(dialog, prompt, width - 3, 2, 3);

    l1len = strlen(label1);
    l2len = strlen(label2);

    x1 = (width - (l1len + l2len)) / 3;
    x2 = x1 + x1 + l1len;

    menu_width = width - 6;
    getyx(dialog, cur_y, cur_x);
    box_y = cur_y + 1;
    box_x = (width - menu_width) / 2 - 1;

    menu = subwin(dialog, menu_height, menu_width, y + box_y + 1, x + box_x + 1);
    keypad(menu, TRUE);

    /* draw a box around the menu items */
    PaintBox(dialog, box_y, box_x, menu_height + 2, menu_width + 2);

    item_x = 3;

    for (i = 0; i < max_choice; i++)
	PaintCheckItem(menu, items[i + scrlx], i, i == 0, checks[i + scrlx]);
    PaintScroller(menu, scrlx + choice, item_no, menu_height);
    wnoutrefresh(menu);

    x = width / 2 - 11;
    y = height - 3;
    PaintButton(dialog, label2, y, x2, FALSE);
    PaintButton(dialog, label1, y, x1, TRUE);
    wrefresh(dialog);

    /*CONSTCOND*/
    while (1) {
	i = choice;
	key = wgetch(dialog);

	if (menu_height > 1 && key == KEY_PPAGE) {
	    if (!choice) {
		if (scrlx) {
		    /* Scroll menu down */
		    getyx(dialog, cur_y, cur_x);

		    nscroll = max_choice > scrlx ? -scrlx : -max_choice;
		    scrollok(menu, TRUE);
		    wscrl(menu, nscroll);
		    scrollok(menu, FALSE);

		    i = scrlx + nscroll;
		    PaintCheckItem(menu, items[i], 0, TRUE, checks[i]);
		    for (++i; i <= scrlx; i++)
			PaintCheckItem(menu, items[i], i - (scrlx + nscroll), FALSE, checks[i]);
		    scrlx += nscroll;
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wrefresh(dialog);
		    continue;
		}
	    }
	    i = 0;
	}
	else if (menu_height > 1 && key == KEY_NPAGE) {
	    if (choice == max_choice - 1) {
		if (scrlx < max_scroll) {
		    /* Scroll menu up */
		    getyx(dialog, cur_y, cur_x);

		    nscroll = (scrlx + max_choice > max_scroll ?
			       max_scroll : scrlx + max_choice) - scrlx;
		    scrollok(menu, TRUE);
		    wscrl(menu, nscroll);
		    scrollok(menu, FALSE);

		    scrlx += nscroll;
		    for (i = 0; i < max_choice - 1; i++)
			PaintCheckItem(menu, items[i + scrlx], i, FALSE, checks[i + scrlx]);
		    PaintCheckItem(menu, items[i + scrlx], max_choice - 1, TRUE, checks[i + scrlx]);
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wrefresh(dialog);
		    continue;
		}
	    }
	    i = max_choice - 1;
	}
	else if (key == KEY_UP) {
	    if (!choice) {
		if (scrlx) {
		    /* Scroll menu down */
		    getyx(dialog, cur_y, cur_x);
		    if (menu_height > 1) {
			PaintCheckItem(menu, items[scrlx], 0, FALSE, checks[scrlx]);
			scrollok(menu, TRUE);
			wscrl(menu, - 1);
			scrollok(menu, FALSE);
		    }
		    scrlx--;
		    PaintCheckItem(menu, items[scrlx], 0, TRUE, checks[scrlx]);
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wrefresh(dialog);
		    continue;
		}
	    }
	    else
		i = choice - 1;
	}
	else if (key == KEY_DOWN) {
	    if (choice == max_choice - 1) {
		if (scrlx + choice < item_no - 1) {
		    /* Scroll menu up */
		    getyx(dialog, cur_y, cur_x);
		    if (menu_height > 1) {
			PaintCheckItem(menu, items[scrlx + max_choice - 1], max_choice - 1, FALSE, checks[scrlx + max_choice - 1]);
			scrollok(menu, TRUE);
			scroll(menu);
			scrollok(menu, FALSE);
		    }
		    scrlx++;
		    PaintCheckItem(menu, items[scrlx + max_choice - 1], max_choice - 1, TRUE, checks[scrlx + max_choice - 1]);
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wrefresh(dialog);
		    continue;
		}
	    }
	    else
		i = MIN(choice + 1, item_no - 1);
	}

	if (i != choice) {
	    getyx(dialog, cur_y, cur_x);
	    PaintCheckItem(menu, items[scrlx + choice], choice, FALSE, checks[scrlx + choice]);

	    choice = i;
	    PaintCheckItem(menu, items[scrlx + choice], choice, TRUE, checks[scrlx + choice]);
	    PaintScroller(menu, scrlx + choice, item_no, menu_height);
	    wnoutrefresh(menu);
	    wmove(dialog, cur_y, cur_x);
	    wrefresh(dialog);
	    continue;
	}

	switch (key) {
	    case TAB:
	    case KEY_LEFT:
	    case KEY_RIGHT:
		if (!button) {
		    button = 1;
		    PaintButton(dialog, label1, y, x1, FALSE);
		    PaintButton(dialog, label2, y, x2, TRUE);
		}
		else {
		    button = 0;
		    PaintButton(dialog, label2, y, x2, FALSE);
		    PaintButton(dialog, label1, y, x1, TRUE);
		}
		wrefresh(dialog);
		break;
	    case ' ':
		getyx(dialog, cur_y, cur_x);
		checks[scrlx + choice] = !checks[scrlx + choice];
		PaintCheckItem(menu, items[scrlx + choice], choice, TRUE, checks[scrlx + choice]);
		wmove(dialog, cur_y, cur_x);
		wnoutrefresh(menu);
		wrefresh(dialog);
		break;
	    case '\r':
	    case '\n':
		delwin(dialog);
		return (!button ? 0 : -1);
	    default:
		for (i = scrlx + choice + 1; i < item_no; i++)
		    if (toupper(items[i][0]) == toupper(key))
			break;
		if (i == item_no) {
		    for (i = 0; i < scrlx + choice; i++)
			if (toupper(items[i][0]) == toupper(key))
			    break;
		}
		getyx(dialog, cur_y, cur_x);
		if (i < item_no && i != scrlx + choice) {
		    if (i >= scrlx && i < scrlx + max_choice) {
			/* it is already visible */
			PaintCheckItem(menu, items[scrlx + choice], choice, FALSE, checks[scrlx + choice]);
			choice = i - scrlx;
		    }
		    else {
			scrlx = MIN(i, max_scroll);
			choice = i - scrlx;
			for (i = 0; i < max_choice; i++)
			    if (i != choice)
				PaintCheckItem(menu, items[scrlx + i], i, FALSE, checks[scrlx + i]);
		    }
		    PaintCheckItem(menu, items[scrlx + choice], choice, TRUE, checks[scrlx + choice]);
		    PaintScroller(menu, scrlx + choice, item_no, menu_height);
		    wnoutrefresh(menu);
		    wmove(dialog, cur_y, cur_x);
		    wrefresh(dialog);
		}
		break;
	}
    }
    /*NOTREACHED*/
}

static void
PaintCheckItem(WINDOW *win, char *item, int choice, int selected, int checked)
{
    int i;

    wattrset(win, selected ? title_attr : dialog_attr);
    wmove(win, choice, 1);
    for (i = 1; i < menu_width; i++)
	waddch(win, ' ');
    wmove(win, choice, item_x);
    wattrset(win, selected ? title_attr : dialog_attr);
    wprintw(win, "[%c] ", checked ? 'X' : ' ');
    waddstr(win, item);
}

static char *
DialogInput(char *title, char *prompt, int height, int width, char *init,
	    char *label1, char *label2, int def_button)
{
    int i, x, y, box_y, box_x, box_width, len,
	input_x = 0, scrlx = 0, key = 0, button = -1, x1, x2, l1len, l2len;
    char instr[1024 + 1];
    WINDOW *dialog;

    x = (COLS - width) / 2;
    y = (LINES - height) / 2;

    dialog = newwin(height, width, y, x);
    keypad(dialog, TRUE);

    PaintWindow(dialog, title, 0, 0, height, width);

    wattrset(dialog, dialog_attr);
    PrintWrap(dialog, prompt, width - 3, 2, 3);

    l1len = strlen(label1);
    l2len = strlen(label2);

    x1 = (width - (l1len + l2len)) / 3;
    x2 = x1 + x1 + l1len;

    box_width = width - 6;
    getyx(dialog, y, x);
    box_y = y + 2;
    box_x = (width - box_width) / 2;
    PaintBox(dialog, y + 1, box_x - 1, 3, box_width + 2);

    x = width / 2 - 11;
    y = height - 3;
    PaintButton(dialog, label2, y, x2, def_button == 1);
    PaintButton(dialog, label1, y, x1, def_button == 0);

    memset(instr, '\0', sizeof(instr));
    wmove(dialog, box_y, box_x);
    wattrset(dialog, dialog_attr);
    if (init)
	strncpy(instr, init, sizeof(instr) - 2);

    input_x = len = strlen(instr);
    if (input_x >= box_width) {
	scrlx = input_x - box_width + 1;
	input_x = box_width - 1;
	for (i = 0; i < box_width - 1; i++)
	    waddch(dialog, instr[scrlx + i]);
    }
    else
	waddstr(dialog, instr);

    wmove(dialog, box_y, box_x + input_x);
  
    wrefresh(dialog);

    while (1) {
	key = wgetch(dialog);
	if (button == -1) {	    /* Input box selected */
	    switch (key) {
		case TAB:
		case KEY_UP:
		case KEY_DOWN:
		    break;
		case KEY_LEFT:
		    if (scrlx && !input_x) {
			--scrlx;
			wmove(dialog, box_y, box_x);
			for (i = 0; i < box_width; i++)
			    waddch(dialog, instr[scrlx + input_x + i] ? instr[scrlx + input_x + i] : ' ');
			wmove(dialog, box_y, input_x + box_x);
			wrefresh(dialog);
		    }
		    else if (input_x) {
			wmove(dialog, box_y, --input_x + box_x);
			wrefresh(dialog);
		    }
		    continue;
		case KEY_RIGHT:
		    if (input_x + scrlx < len) {
			if (input_x == box_width - 1) {
			    ++scrlx;
			    wmove(dialog, box_y, box_x);
			    for (i = scrlx; i < scrlx + box_width; i++)
				waddch(dialog, instr[i] ? instr[i] : ' ');
			    wmove(dialog, box_y, input_x + box_x);
			    wrefresh(dialog);
			}
			else {
			    wmove(dialog, box_y, ++input_x + box_x);
			    wrefresh(dialog);
			}
		    }
		    continue;
		case KEY_BACKSPACE:
		case 0177:
#if defined(__SCO__) || defined(__UNIXWARE__)
		case '\b':
#endif
		    if (input_x || scrlx) {
			wattrset(dialog, dialog_attr);

			if (scrlx + input_x < len)
			    memmove(instr + scrlx + input_x - 1,
				    instr + scrlx + input_x,
				    len - (scrlx + input_x));
			instr[--len] = '\0';

			if (!input_x) {
			    scrlx = scrlx < box_width - 1 ? 0 : scrlx - (box_width - 1);
			    wmove(dialog, box_y, box_x);
			    for (i = 0; i < box_width; i++)
				waddch(dialog, instr[scrlx + input_x + i] ? instr[scrlx + input_x + i] : ' ');
			    input_x = len - scrlx;
			}
			else {
			    wmove(dialog, box_y, --input_x + box_x);
			    for (i = scrlx + input_x; i < len &&
				 i < scrlx + box_width; i++)
				waddch(dialog, instr[i]);
			    if (i < scrlx + box_width)
				waddch(dialog, ' ');
			}
			wmove(dialog, box_y, input_x + box_x);
			wrefresh(dialog);
		    }
		    continue;
		case KEY_HOME:
		case CONTROL_A:
		    wmove(dialog, box_y, box_x);
		    if (scrlx != 0) {
			scrlx = 0;
			for (i = 0; i < box_width; i++)
			    waddch(dialog, instr[i] ? instr[i] : ' ');
		    }
		    input_x = 0;
		    wmove(dialog, box_y, box_x);
		    wrefresh(dialog);
		    break;
		case CONTROL_D:
		    if (input_x + scrlx < len) {
			memmove(instr + scrlx + input_x,
				    instr + scrlx + input_x + 1,
				    len - (scrlx + input_x));
			instr[--len] = '\0';
			for (i = scrlx + input_x; i < len &&
			     i < scrlx + box_width; i++)
			    waddch(dialog, instr[i]);
			if (i < scrlx + box_width)
			    waddch(dialog, ' ');
			wmove(dialog, box_y, input_x + box_x);
			wrefresh(dialog);
		    }
		    break;
		case CONTROL_E:
		case KEY_END:
		    if (box_width + scrlx < len) {
			input_x = box_width - 1;
			scrlx = len - box_width + 1;
			wmove(dialog, box_y, box_x);
			for (i = scrlx; i < scrlx + box_width; i++)
			    waddch(dialog, instr[i] ? instr[i] : ' ');
			wmove(dialog, box_y, input_x + box_x);
			wrefresh(dialog);
		    }
		    else {
			input_x = len - scrlx;
			wmove(dialog, box_y, input_x + box_x);
			wrefresh(dialog);
		    }
		    break;
		case CONTROL_K:
		    if (len) {
			for (i = input_x; i < box_width; i++)
			    waddch(dialog, ' ');
			for (i = scrlx + input_x; i < len; i++)
			    instr[i] = '\0';
			len = scrlx + input_x;
			wmove(dialog, box_y, box_x + input_x);
			wrefresh(dialog);
		    }
		    break;
		default:
		    if (key < 0x100 && isprint(key)) {
			if (scrlx + input_x < sizeof(instr) - 1) {
			    wattrset(dialog, dialog_attr);
			    if (scrlx + input_x < len) {
				memmove(instr + scrlx + input_x + 1,
					instr + scrlx + input_x,
					len - (scrlx + input_x));
			    }
			    instr[scrlx + input_x] = key;
			    instr[++len] = '\0';
			    if (input_x == box_width - 1) {
				scrlx++;
				wmove(dialog, box_y, box_x);
				for (i = 0; i < box_width - 1; i++)
				    waddch(dialog, instr[scrlx + i]);
			    }
			    else {
				wmove(dialog, box_y, input_x++ + box_x);
				for (i = scrlx + input_x - 1; i < len &&
				     i < scrlx + box_width; i++)
				    waddch(dialog, instr[i]);
				wmove(dialog, box_y, input_x + box_x);
			    }
			    wrefresh(dialog);
			}
			else
			    flash();	    /* Alarm user about overflow */
			continue;
		    }
		}
	    }

	switch (key) {
	    case KEY_UP:
	    case KEY_LEFT:
	        switch (button) {
		    case -1:
			button = 1;	/* Indicates "Cancel" button is selected */
			PaintButton(dialog, label1, y, x1, FALSE);
			PaintButton(dialog, label2, y, x2, TRUE);
			wrefresh(dialog);
			break;
		    case 0:
			button = -1;	/* Indicates input box is selected */
			PaintButton(dialog, label2, y, x2, FALSE);
			PaintButton(dialog, label1, y, x1, TRUE);
			wmove(dialog, box_y, box_x + input_x);
			wrefresh(dialog);
			break;
		    case 1:
			button = 0;	/* Indicates "OK" button is selected */
			PaintButton(dialog, label2, y, x2, FALSE);
			PaintButton(dialog, label1, y, x1, TRUE);
			wrefresh(dialog);
			break;
		}
		break;
	    case TAB:
	    case KEY_DOWN:
	    case KEY_RIGHT:
		switch (button) {
		    case -1:
			button = 0;	/* Indicates "OK" button is selected */
			PaintButton(dialog, label2, y, x2, FALSE);
			PaintButton(dialog, label1, y, x1, TRUE);
			wrefresh(dialog);
			break;
		    case 0:
			button = 1;	/* Indicates "Cancel" button is selected */
			PaintButton(dialog, label1, y, x1, FALSE);
			PaintButton(dialog, label2, y, x2, TRUE);
			wrefresh(dialog);
			break;
		    case 1:
			button = -1;	/* Indicates input box is selected */
			PaintButton(dialog, label2, y, x2, FALSE);
			PaintButton(dialog, label1, y, x1, TRUE);
			wmove(dialog, box_y, box_x + input_x);
			wrefresh(dialog);
			break;
		}
		break;
	    case ' ':
	    case '\r':
	    case '\n':
		delwin(dialog);
		return (button != 1 ? XtNewString(instr) : NULL);
	}
    }
}
