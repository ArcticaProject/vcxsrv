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

#include "options.h"
#include "xf86config.h"
#include <X11/Xresource.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/SimpleMenP.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Viewport.h>
#include <ctype.h>

/*
 * Prototypes
 */
static void PopdownCallback(Widget, XtPointer, XtPointer);
static void SelectOptionCallback(Widget, XtPointer, XtPointer);
static void AddOption(Widget, XtPointer, XtPointer);
static void RemoveOption(Widget, XtPointer, XtPointer);
static void UpdateOption(Widget, XtPointer, XtPointer);
static void UpdateOptionList(void);
#ifdef USE_MODULES
static void AddDriverOption(Widget, XtPointer, XtPointer);
static void SelectModuleCallback(Widget, XtPointer, XtPointer);
static void SelectModuleOptionCallback(Widget, XtPointer, XtPointer);
static void ModuleOptionsPopdown(Widget, XtPointer, XtPointer);
#endif
static Bool EnumDatabase(XrmDatabase*, XrmBindingList, XrmQuarkList,
			 XrmRepresentation*, XrmValue*, XPointer);

/*
 * Initialization
 */
Widget optionsShell;
static XF86OptionPtr *options;
static Widget add, remov, update, list, name, value;
static char *option_str;
static int option_index, popped = False;
static char *Options = "lib/X11/Options";
XrmDatabase options_xrm;
struct {
    char *string;
    int offset;
} rebuild_xrm;
#ifdef USE_MODULES
static Widget modList, optList, desc, modOptionsShell, labelType;
static char *module_sel;
static char *types[] = {
    "none", "integer", "(non null) string", "string", "real",
    "boolean", "frequency",
};
#endif

/*
 * Implementation
 */
#ifdef USE_MODULES
static int
qcmp_str(_Xconst void *a, _Xconst void *b)
{
    return (strcmp(*(char**)a, *(char**)b));
}

void
ModuleOptionsPopup(Widget w, XtPointer user_data, XtPointer call_data)
{
    xf86cfgModuleOptions *info = module_options;

    if (modOptionsShell == NULL) {
	char **ops;
	int nops;
	Widget pane, form, viewport, bottom, popdown;

	modOptionsShell = XtCreatePopupShell("moduleOptions",
					     transientShellWidgetClass,
					     optionsShell, NULL, 0);

	pane = XtCreateManagedWidget("pane", panedWidgetClass,
				     modOptionsShell, NULL, 0);

	form = XtCreateManagedWidget("descriptions", formWidgetClass,
				     pane, NULL, 0);
	labelType = XtCreateManagedWidget("labelType", labelWidgetClass,
					  form, NULL, 0);
	XtCreateManagedWidget("module", labelWidgetClass, form, NULL, 0);
	viewport = XtCreateManagedWidget("viewM", viewportWidgetClass,
					 form, NULL, 0);
	ops = NULL;
	nops = 0;
	while (info) {
	    ++nops;
	    ops = (char**)XtRealloc((XtPointer)ops, sizeof(char*) * nops);
	    ops[nops - 1] = XtNewString(info->name);
	    info = info->next;
	}
	if (nops == 0) {
	    ops = (char**)XtMalloc(sizeof(char*));
	    ops[0] = XtNewString("");
	    nops = 1;
	}
	else
	    qsort(ops, nops, sizeof(char*), qcmp_str);
	modList = XtVaCreateManagedWidget("modL", listWidgetClass,
					  viewport, XtNlist, ops,
					  XtNnumberStrings, nops,
					  NULL);
	XtAddCallback(modList, XtNcallback, SelectModuleCallback, NULL);
	XtCreateManagedWidget("option", labelWidgetClass, form, NULL, 0);
	viewport = XtCreateManagedWidget("viewO", viewportWidgetClass,
					 form, NULL, 0);
	ops = (char**)XtMalloc(sizeof(char*));
	ops[0] = XtNewString("");
	optList = XtVaCreateManagedWidget("optL", listWidgetClass,
					  viewport, XtNlist, ops,
					  XtNnumberStrings, 1, NULL);
	XtAddCallback(optList, XtNcallback, SelectModuleOptionCallback, NULL);
	desc = XtVaCreateManagedWidget("desc", asciiTextWidgetClass,
				       form, XtNeditType, XawtextRead,
				       NULL);

	bottom = XtCreateManagedWidget("bottom", formWidgetClass,
				       pane, NULL, 0);
	popdown = XtVaCreateManagedWidget("popdown", commandWidgetClass,
					bottom, NULL);
	XtAddCallback(popdown, XtNcallback, ModuleOptionsPopdown, NULL);
	XtRealizeWidget(modOptionsShell);
	XSetWMProtocols(DPY, XtWindow(modOptionsShell), &wm_delete_window, 1);

	info = module_options;
    }

    if (module_sel && *module_sel) {
	XawListReturnStruct list;	/* hack to call ballbacks */
	char **strs;
	int nstrs, idx = 0;

	XtVaGetValues(modList, XtNlist, &strs, XtNnumberStrings, &nstrs, NULL);
	for (idx = nstrs - 1; idx > 0; idx--)
	    if (strcmp(module_sel, strs[idx]) == 0)
		break;
	while (info) {
	    if (strcmp(module_sel, info->name) == 0)
		break;
	    info = info->next;
	}
	if (info) {
	    list.string = info->name;
	    list.list_index = idx;
	    XawListHighlight(modList, idx);
	    SelectModuleCallback(modList, NULL, (XtPointer)&list);
	}
	if (option_str && *option_str) {
	    OptionInfoPtr opts = info->option;

	    idx = 0;
	    while (opts && opts->name) {
		if (strcmp(opts->name, option_str) == 0)
		    break;
		++idx;
		++opts;
	    }

	    if (opts && opts->name) {
		list.string = (char *)opts->name;
		list.list_index = idx;
		XawListHighlight(optList, idx);
		SelectModuleOptionCallback(optList, NULL, (XtPointer)&list);
	    }
	}
    }
    XtPopup(modOptionsShell, XtGrabNone);
}

/*ARGSUSED*/
static void
ModuleOptionsPopdown(Widget w, XtPointer user_data, XtPointer call_data)
{
    XtPopdown(modOptionsShell);
}

/*ARGSUSED*/
void
ModuleOptionsCancelAction(Widget w, XEvent *event,
			  String *params, Cardinal *num_params)
{
    ModuleOptionsPopdown(w, NULL, NULL);
}
#endif

void
CreateOptionsShell(void)
{
    optionsShell = XtCreatePopupShell("options", transientShellWidgetClass,
				      toplevel, NULL, 0);
}

#ifdef USE_MODULES
void
OptionsPopup(XF86OptionPtr *opts, char *driver, OptionInfoPtr drv_opts)
#else
void
OptionsPopup(XF86OptionPtr *opts)
#endif
{
    static int first = 1;
#ifdef USE_MODULES
    static Widget button, menu;
    static char label[256], menuName[16];
    Widget sme;
    char buf[256];
    int i = 0;
    Arg args[1];
    static int menuN;
#endif

    option_str = NULL;
    options = opts;
    if (first) {
	Widget pane, form, viewport, bottom, popdown;

	first = 0;

	if (optionsShell == NULL)
	    CreateOptionsShell();
	pane = XtCreateManagedWidget("pane", panedWidgetClass,
				     optionsShell, NULL, 0);

	form = XtCreateManagedWidget("commands", formWidgetClass,
				     pane, NULL, 0);
	add = XtCreateManagedWidget("add", commandWidgetClass,
				    form, NULL, 0);
	XtAddCallback(add, XtNcallback, AddOption, NULL);
	remov = XtCreateManagedWidget("remove", commandWidgetClass,
				      form, NULL, 0);
	XtAddCallback(remov, XtNcallback, RemoveOption, NULL);
	update = XtCreateManagedWidget("update", commandWidgetClass,
				       form, NULL, 0);
	XtAddCallback(update, XtNcallback, UpdateOption, NULL);
#ifdef USE_MODULES
	if (!nomodules) {
	    Widget command;

	    command = XtCreateManagedWidget("help", commandWidgetClass,
					    form, NULL, 0);
	    XtAddCallback(command, XtNcallback, ModuleOptionsPopup, NULL);
	}
#endif
	form = XtCreateManagedWidget("form", formWidgetClass,
				     pane, NULL, 0);
	XtVaCreateManagedWidget("label1", labelWidgetClass, form,
				XtNlabel, " Option \"",
				NULL);
	name = XtVaCreateManagedWidget("name", asciiTextWidgetClass, form,
				       XtNeditType, XawtextEdit,
				       NULL);
	XtVaCreateManagedWidget("label2", labelWidgetClass,
				form,
				XtNlabel, "\" \"",
				NULL);
	value = XtVaCreateManagedWidget("value", asciiTextWidgetClass, form,
					XtNeditType, XawtextEdit,
					NULL);
	XtVaCreateManagedWidget("label3", labelWidgetClass, form,
				XtNlabel, "\" ",
				NULL);
	viewport = XtCreateManagedWidget("viewport", viewportWidgetClass,
					 form, NULL, 0);
	list = XtCreateManagedWidget("list", listWidgetClass,
				     viewport, NULL, 0);
	XtAddCallback(list, XtNcallback, SelectOptionCallback, NULL);
	bottom = XtCreateManagedWidget("bottom", formWidgetClass,
				       pane, NULL, 0);
#ifdef USE_MODULES
	if (!nomodules)
	    button = XtCreateManagedWidget("driverOpts", menuButtonWidgetClass,
					    bottom, NULL, 0);
#endif
	popdown = XtVaCreateManagedWidget("popdown", commandWidgetClass,
					bottom, NULL);
#ifdef USE_MODULES
	if (!nomodules)
	    XtVaSetValues(popdown, XtNfromHoriz, button, NULL);
#endif

	XtAddCallback(popdown, XtNcallback, PopdownCallback, NULL);
	XtRealizeWidget(optionsShell);
	XSetWMProtocols(DPY, XtWindow(optionsShell), &wm_delete_window, 1);

#ifdef USE_MODULES
	if (!nomodules) {
	    char *str;

	    XtSetArg(args[0], XtNlabel, &str);
	    XtGetValues(button, args, 1);
	    XmuSnprintf(label, sizeof(label), "%s", str);
	}
#endif
    }

#ifdef USE_MODULES
    if (!nomodules) {
	if (menu)
	    XtDestroyWidget(menu);
	XmuSnprintf(menuName, sizeof(menuName), "optionM%d", menuN);
	menuN = !menuN;
	menu = XtCreatePopupShell(menuName, simpleMenuWidgetClass, button,
				  NULL, 0);
	XtVaSetValues(button, XtNmenuName, menuName, NULL);
	if (drv_opts) {
	    int len, longest = 0;
	    char fmt[32];

	    for (i = 0; drv_opts[i].name != NULL; i++) {
		len = strlen(drv_opts[i].name);
		if (len > longest)
		    longest = len;
	    }
	    XmuSnprintf(fmt, sizeof(fmt), "%c-%ds  %%s", '%', longest);
	    for (; drv_opts->name != NULL; drv_opts++) {
		char *type;

		if (drv_opts->type >= OPTV_NONE && drv_opts->type <= OPTV_FREQ)
		    type = types[drv_opts->type];
		else
		    type = "UNKNOWN";

		XmuSnprintf(buf, sizeof(buf), fmt, drv_opts->name, type);
		sme = XtVaCreateManagedWidget(drv_opts->name, smeBSBObjectClass,
					      menu, XtNlabel, buf, NULL);
		XtAddCallback(sme, XtNcallback, AddDriverOption, (XtPointer)drv_opts);
	    }
	}
	if (i) {
	    xf86cfgModuleOptions *mod = module_options;

	    while (mod) {
		if (strcmp(mod->name, driver) == 0) {
		    /* don't assign to driver, as it may be a temp string */
		    module_sel = mod->name;
		    break;
		}
		mod = mod->next;
	    }
	    XmuSnprintf(buf, sizeof(buf), "%s%s", label, driver);
	    XtSetArg(args[0], XtNlabel, buf);
	    XtSetValues(button, args, 1);
	    XtMapWidget(button);
	}
	else
	    XtUnmapWidget(button);
    }
#endif

    UpdateOptionList();
    popped = True;
    XtPopup(optionsShell, XtGrabExclusive);

    while (popped)
	XtAppProcessEvent(XtWidgetToApplicationContext(optionsShell), XtIMAll);
}

static void
UpdateOptionList(void)
{
    Arg args[2];
    char **ops, **oldops;
    int nops, oldnops;
    XF86OptionPtr opt;

    ops = NULL;
    nops = 0;
    XawListUnhighlight(list);
    XtSetArg(args[0], XtNlist, &oldops);
    XtSetArg(args[1], XtNnumberStrings, &oldnops);
    XtGetValues(list, args, 2);
    opt = *options;
    while (opt != NULL) {
	if (nops % 16 == 0)
	    ops = (char**)XtRealloc((XtPointer)ops, (nops + 16) *
				    sizeof(char*));
	ops[nops++] = XtNewString(opt->opt_name);
	opt = (XF86OptionPtr)(opt->list.next);
    }
    if (nops == 0) {
	ops = (char**)XtMalloc(sizeof(char*));
	ops[0] = XtNewString("");
	nops = 1;
    }
    XtSetArg(args[0], XtNlist, ops);
    XtSetArg(args[1], XtNnumberStrings, nops);
    XtSetValues(list, args, 2);
    if (oldnops > 0 &&
	(oldnops != 1 || XtName(list) != oldops[0])) {
	while (--oldnops >= 0)
	    XtFree(oldops[oldnops]);
	XtFree((XtPointer)oldops);
    }

    XtSetArg(args[0], XtNstring, "");
    XtSetValues(name, args, 1);
    XtSetValues(value, args, 1);

    /* force relayout */
    XtUnmanageChild(list);
    XtManageChild(list);

    XtSetSensitive(remov, False);
    XtSetSensitive(update, False);
}

/*ARGSUSED*/
static void
PopdownCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    XtPopdown(optionsShell);
    popped = False;
}

/*ARGSUSED*/
void
OptionsCancelAction(Widget w, XEvent *event,
		    String *params, Cardinal *num_params)
{
    PopdownCallback(w, NULL, NULL);
}

/*ARGSUSED*/
static void
SelectOptionCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    XF86OptionPtr option;
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;

    option_str = info->string;
    option_index = info->list_index;
    if ((option = xf86findOption(*options, info->string)) != NULL) {
	XtSetArg(args[0], XtNstring, option->opt_name);
	XtSetValues(name, args, 1);
	XtSetArg(args[0], XtNstring,
		 option->opt_val != NULL ? option->opt_val : "");
	XtSetValues(value, args, 1);
    }
    XtSetSensitive(remov, True);
    XtSetSensitive(update, True);
}

/*ARGSUSED*/
static void
AddOption(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    char *nam, *val;

    XtSetArg(args[0], XtNstring, &nam);
    XtGetValues(name, args, 1);
    XtSetArg(args[0], XtNstring, &val);
    XtGetValues(value, args, 1);
    if (xf86findOption(*options, nam) != NULL || strlen(nam) == 0)
	/* XXX xf86addNewOption will trash the option linked list if
	 * the options being added already exists.
	 */
	return;
    *options = xf86addNewOption(*options, XtNewString(nam),
				val && strlen(val) ? XtNewString(val) : NULL);
    UpdateOptionList();
}

#ifdef USE_MODULES
/*ARGSUSED*/
static void
AddDriverOption(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    OptionInfoPtr opt = (OptionInfoPtr)user_data;
    XF86OptionPtr option;

    option_str = (char *)opt->name;
    XtSetArg(args[0], XtNstring, opt->name);
    XtSetValues(name, args, 1);
    if ((option = xf86findOption(*options, opt->name)) == NULL)
	XtSetArg(args[0], XtNstring, "");
    else
	XtSetArg(args[0], XtNstring, option->opt_val);
    XtSetValues(value, args, 1);
}

/*ARGSUSED*/
static void
SelectModuleCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    xf86cfgModuleOptions *mod = module_options;
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;

    while (mod) {
	if (strcmp(mod->name, info->string) == 0)
	    break;
	mod = mod->next;
    }

    if (mod) {
	Arg args[2];
	char **list = NULL, **old;
	OptionInfoPtr opts = mod->option;
	int num = 0, oldnum;

	module_sel = mod->name;
	XtSetArg(args[0], XtNlist, &old);
	XtSetArg(args[1], XtNnumberStrings, &oldnum);
	XtGetValues(optList, args, 2);
	while (opts && opts->name) {
	    ++num;
	    list = (char**)XtRealloc((XtPointer)list, sizeof(char*) * num);
	    list[num - 1] = XtNewString(opts->name);
	    ++opts;
	}
	if (num == 0) {
	    list = (char**)XtMalloc(sizeof(char*));
	    list[0] = XtNewString("");
	    num = 1;
	}
	XtSetArg(args[0], XtNlist, list);
	XtSetArg(args[1], XtNnumberStrings, num);
	XtSetValues(optList, args, 2);
	while (--oldnum >= 0)
	    XtFree(old[oldnum]);
	XtFree((XtPointer)old);

	XtVaSetValues(desc, XtNstring, "", NULL);
	XawListUnhighlight(optList);

	/* force relayout */
	XtUnmanageChild(optList);
	XtManageChild(optList);
    }
}

static void
SelectModuleOptionCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    xf86cfgModuleOptions *mod = module_options;
    XawListReturnStruct *info = (XawListReturnStruct *)call_data;
    char *description = NULL, *type = "undefined";
    char label[256];

    if (module_sel && info->string)
	description = GetOptionDescription(module_sel, info->string);
    if (description == NULL)
	description = "** NO DESCRIPTION AVAILABLE **";

    XtVaSetValues(desc, XtNstring, description, NULL);

    while (mod) {
	if (strcmp(module_sel, mod->name) == 0)
	    break;
	mod = mod->next;
    }
    if (mod) {
	OptionInfoPtr opts = mod->option;

	while (opts && opts->name) {
	    if (strcasecmp(opts->name, info->string) == 0)
		break;
	    ++opts;
	}
	if (opts && opts->name && opts->type >= OPTV_NONE &&
	    opts->type <= OPTV_FREQ)
	    type = types[opts->type];
    }

    XmuSnprintf(label, sizeof(label), "%s.%s (%s)", module_sel, info->string,
	        type);
    XtVaSetValues(labelType, XtNlabel, label, NULL);
}
#endif

/*ARGSUSED*/
static void
RemoveOption(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    char *str;

    XtSetArg(args[0], XtNstring, &str);
    XtGetValues(name, args, 1);
    xf86removeOption(options, str);
    UpdateOptionList();
}

/*ARGSUSED*/
static void
UpdateOption(Widget w, XtPointer user_data, XtPointer call_data)
{
/*    xf86removeOption(options, option_str);
    AddOption(w, user_data, call_data);
    UpdateOptionList();*/

    Arg args[1];
    char *nam, *val;
    XF86OptionPtr option;

    XtSetArg(args[0], XtNstring, &nam);
    XtGetValues(name, args, 1);
    XtSetArg(args[0], XtNstring, &val);
    XtGetValues(value, args, 1);
    if ((option = xf86findOption(*options, option_str)) == NULL)
	return;
    XtFree(option->opt_name);
    option->opt_name = option_str = XtNewString(nam);
    XtFree(option->opt_val);
    if (val && strlen(val))
	option->opt_val = XtNewString(val);
    else
	option->opt_val = NULL;

    UpdateOptionList();
    XawListHighlight(list, option_index);
    XtSetArg(args[0], XtNstring, option->opt_name);
    XtSetValues(name, args, 1);
    XtSetArg(args[0], XtNstring, option->opt_val);
    XtSetValues(value, args, 1);

    XtSetSensitive(remov, True);
    XtSetSensitive(update, True);
}

/*ARGUSED*/
static Bool
EnumDatabase(XrmDatabase *db, XrmBindingList bindings, XrmQuarkList quarks,
	     XrmRepresentation *type, XrmValue *value, XPointer closure)
{
    char *module = XrmQuarkToString(quarks[0]),
	 *option = XrmQuarkToString(quarks[1]);

    /* handle *.Option: value */
    if (module && option == NULL) {
	option = module;
	module = "*";
    }

    /*
     * NOTE: If the Options file is changed to support any other format than
     *
     *		Module.Option: description text
     *
     * this code will also need to be updated.
     */

    if (module) {
	XrmValue xrm;
	char *type, *value, query[256];

	XmuSnprintf(query, sizeof(query), "%s.%s", module, option);
	if (XrmGetResource(options_xrm, query, "Module.Option", &type, &xrm))
	    value = (char*)xrm.addr;
	else
	    value = NULL;

	if (value) {
	    char *norm;
	    unsigned char *ptr;
	    int position;
	    int length = strlen(module) + strlen(option) + strlen(value) + 4;

	    rebuild_xrm.string = XtRealloc(rebuild_xrm.string,
					   rebuild_xrm.offset + length);
	    position = rebuild_xrm.offset +
		       sprintf(rebuild_xrm.string + rebuild_xrm.offset, "%s.%s:",
			       module, option);

	    /* removes underlines and spaces */
	    norm = strchr(rebuild_xrm.string + rebuild_xrm.offset, '.') + 1;
	    for (; *norm; norm++) {
		if (*norm == '_' || *norm == ' ' || *norm == '\t') {
		    memmove(norm, norm + 1, strlen(norm) + 1);
		    --position;
		    --length;
		}
	    }

	    for (ptr = (unsigned char*)rebuild_xrm.string + rebuild_xrm.offset;
		 *ptr; ptr++)
		*ptr = tolower(*ptr);
	    sprintf(rebuild_xrm.string + position, "%s\n", value);
	    rebuild_xrm.offset += length - 1;
	}
    }

    return (False);
}

Bool
InitializeOptionsDatabase(void)
{
    static int first = 1;
    static Bool result = True;

    if (first) {
	XrmQuark names[2];
	XrmQuark classes[2];

	first = 0;
	XrmInitialize();
	if ((options_xrm = XrmGetFileDatabase(Options)) == (XrmDatabase)0) {
	    fprintf(stderr, "Cannot open '%s' database.\n", Options);
	    return (False);
	}

	/* rebuild database, using only lowercase characters */
	names[0] = classes[0] = names[1] = classes[1] = NULLQUARK;
	(void)XrmEnumerateDatabase(options_xrm, (XrmNameList)&names,
				   (XrmClassList)&classes, XrmEnumAllLevels,
				   EnumDatabase, NULL);

	/* free previous database, as it is not guaranteed to be
         * "case insensitive" */
	XrmDestroyDatabase(options_xrm);

	/* create case insensitive database by making everything lowercase */
	if (rebuild_xrm.string == NULL ||
	    (options_xrm = XrmGetStringDatabase(rebuild_xrm.string)) ==
	    (XrmDatabase)0) {
	    fprintf(stderr, "Cannot rebuild '%s' database.\n", Options);
	    XtFree(rebuild_xrm.string);
	    return (False);
	}
	XtFree(rebuild_xrm.string);
    }

    return (result);
}

char *
GetOptionDescription(char *module, char *option)
{
    char *type;
    XrmValue value;
    char query[256];
    unsigned char *ptr;

    InitializeOptionsDatabase();

    XmuSnprintf(query, sizeof(query), "%s.%s", module, option);
    ptr = (unsigned char*)strchr(query, '.') + 1;
    for (; *ptr; ptr++) {
	if (*ptr == '_' || *ptr == ' ' || *ptr == '\t')
	    memmove(ptr, ptr + 1, strlen((char*)ptr) + 1);
    }
    for (ptr = (unsigned char*)query; *ptr; ptr++)
	*ptr = tolower(*ptr);
    if (XrmGetResource(options_xrm, query, "Module.Option", &type, &value))
	return ((char*)value.addr);

    return (NULL);
}
