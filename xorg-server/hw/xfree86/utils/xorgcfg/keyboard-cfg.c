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
#include "keyboard-cfg.h"
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>

#define IS_KBDDRIV(X) ((strcasecmp((X),"kbd") == 0))

/*
 * Types
 */
typedef struct {
    char *rules;
    XkbRF_RulesPtr list;
    XF86XkbDescInfo model;
    XF86XkbDescInfo layout;
    XF86XkbDescInfo variant;
    XF86XkbDescInfo option;
} XF86XkbRulesDescInfo;

/*
 * Prototypes
 */
static void KeyboardRulesCallback(Widget, XtPointer, XtPointer);
static void KeyboardModelCallback(Widget, XtPointer, XtPointer);
static void KeyboardLayoutCallback(Widget, XtPointer, XtPointer);
static void KeyboardVariantCallback(Widget, XtPointer, XtPointer);
static void KeyboardOptionsCallback(Widget, XtPointer, XtPointer);
static void KeyboardApplyCallback(Widget, XtPointer, XtPointer);
static Bool KeyboardConfigCheck(void);
static void XkbUIEventHandler(Widget, XtPointer, XEvent*, Boolean*);
static XF86XkbRulesDescInfo *GetXkbRulesDesc(char*);
static void UpdateRulesPopups(void);

/*
 * Initialization
 */
static XF86XkbRulesDescInfo **xkb_desc, *xkb_rules;
static int num_xkb_desc;
static char *XkbRulesDir = "share/X11/xkb/rules/";
#ifdef XFREE98_XKB
static char *XkbRulesFile = "xfree98";
#else
static char *XkbRulesFile = __XKBDEFRULES__;
#endif
static XF86ConfInputPtr current_input;

static char *rules, *model, *layout, *variant, *options;
static Widget kbd, rulesb, modelb, layoutb, variantb, optionsb,
	      modelp, layoutp, variantp, optionsp;
static XkbInfo **xkb_infos;
static int num_xkb_infos;
XkbInfo *xkb_info;

static Widget apply;

/*
 * Implementation
 */
/*ARGSUSED*/
XtPointer
KeyboardConfig(XtPointer config)
{
    XF86ConfInputPtr keyboard = (XF86ConfInputPtr)config;
    XF86OptionPtr option;
    Arg args[1];
    static char *XkbRules = "XkbRules", *XkbModel = "XkbModel",
		*XkbLayout = "XkbLayout", *XkbVariant = "XkbVariant",
		*XkbOptions = "XkbOptions";
    XF86XkbRulesDescInfo *info;
    char *omodel, *olayout, *ovariant, *ooptions;

    InitializeKeyboard();
    rules = xkb_rules->rules;
    if (xkb_info->config.rules_file == NULL)
	xkb_info->config.rules_file = rules;

    if (options)
	XtFree(options);
    options = NULL;

    if (xkb_info->conf == NULL)
	xkb_info->conf = keyboard;

    if (xkb_info->conf != keyboard) {
	int i;

	for (i = 0; i < num_xkb_infos; i++)
	    if (xkb_infos[i]->conf == keyboard) {
		xkb_info = xkb_infos[i];
		break;
	    }

	if (i >= num_xkb_infos) {
	    int timeout = 10;

	    xkb_info = (XkbInfo*)XtCalloc(1, sizeof(XkbInfo));
	    xkb_info->conf = keyboard;
	    xkb_infos = (XkbInfo**)
		XtRealloc((XtPointer)xkb_infos, sizeof(XkbInfo*) *
			  (num_xkb_infos + 1));
	    xkb_infos[num_xkb_infos++] = xkb_info;

	    xkb_info->conf = keyboard;
	    bzero((char*)&(xkb_info->defs), sizeof(XkbRF_VarDefsRec));
	    while (timeout > 0) {
		xkb_info->xkb =
		    XkbGetKeyboard(XtDisplay(configp),
				   XkbGBN_AllComponentsMask, XkbUseCoreKbd);
		if (xkb_info->xkb == NULL) {
		    timeout -= 1;
		    sleep(1);
		}		    
		else
		    break;
	    }
	    if (timeout <= 0) {
		fprintf(stderr, "Couldn't get keyboard\n");
	    }
	    if (xkb_info->xkb && xkb_info->xkb->names && xkb_info->xkb->geom &&
		xkb_info->xkb->names->geometry == 0)
		xkb_info->xkb->names->geometry = xkb_info->xkb->geom->name;
	}

	/* check for removed devices */
	for (i = 0; i < num_xkb_infos; i++) {
	    XF86ConfInputPtr key = XF86Config->conf_input_lst;

	    while (key != NULL) {
		if (IS_KBDDRIV(key->inp_driver) && xkb_infos[i]->conf == key)
		    break;
		key = (XF86ConfInputPtr)(key->list.next);
	    }
	    if (xkb_infos[i]->conf != NULL && key == NULL) {
		XkbFreeKeyboard(xkb_infos[i]->xkb, 0, False);
		XtFree((XtPointer)xkb_infos[i]);
		if (--num_xkb_infos > i)
		    memmove(&xkb_infos[i], &xkb_infos[i + 1],
			(num_xkb_infos - i) * sizeof(XkbInfo*));
	    }
	}
    }

    current_input = keyboard;

    if (keyboard != NULL) {
	if ((option = xf86findOption(keyboard->inp_option_lst, XkbRules)) != NULL) {
	    if (strcmp(rules, option->opt_val)) {
		XF86XkbRulesDescInfo *info = GetXkbRulesDesc(option->opt_val);

		if (info) {
		    rules = info->rules;
		    UpdateRulesPopups();
		}
	    }
	}
	if ((option = xf86findOption(keyboard->inp_option_lst, XkbModel)) != NULL)
	    xkb_info->defs.model = model = option->opt_val;
	else
	    xkb_info->defs.model = model = xkb_rules->model.name[0];
	if ((option = xf86findOption(keyboard->inp_option_lst, XkbLayout)) != NULL)
	    xkb_info->defs.layout = layout = option->opt_val;
	else
	    xkb_info->defs.layout = layout = xkb_rules->layout.name[0];
	if ((option = xf86findOption(keyboard->inp_option_lst, XkbVariant)) != NULL)
	    xkb_info->defs.variant = variant = option->opt_val;
	else
	    xkb_info->defs.variant = variant = NULL;

	if ((option = xf86findOption(keyboard->inp_option_lst, XkbOptions)) != NULL)
	    xkb_info->defs.options = options = XtNewString(option->opt_val);
	else
	    xkb_info->defs.options = options = NULL;

	XtSetArg(args[0], XtNstring, keyboard->inp_identifier);
	XtSetValues(ident_widget, args, 1);

	(void)UpdateKeyboard(False);
    }
    else {
	XF86ConfInputPtr input = XF86Config->conf_input_lst;
	char keyboard_name[48];
	int nkeyboards = 0;

	while (input != NULL) {
	    if (IS_KBDDRIV(input->inp_driver))
		++nkeyboards;
	    input = (XF86ConfInputPtr)(input->list.next);
	}
	do {
	    XmuSnprintf(keyboard_name, sizeof(keyboard_name),
			"Keyboard%d", nkeyboards);
	    ++nkeyboards;
	} while (xf86findInput(keyboard_name,
		 XF86Config->conf_input_lst));

	model = xkb_rules->model.name[0];
	layout = xkb_rules->layout.name[0];
	variant = "";
	options = XtNewString("");
	XtSetArg(args[0], XtNstring, keyboard_name);
	XtSetValues(ident_widget, args, 1);
    }

    info = xkb_rules;
    omodel = model;
    olayout = layout;
    ovariant = variant;
    ooptions = options ? XtNewString(options) : NULL;

    xf86info.cur_list = KEYBOARD;
    XtSetSensitive(back, xf86info.lists[KEYBOARD].cur_function > 0);
    XtSetSensitive(next, xf86info.lists[KEYBOARD].cur_function <
			 xf86info.lists[KEYBOARD].num_functions - 1);
    (xf86info.lists[KEYBOARD].functions[xf86info.lists[KEYBOARD].cur_function])
	(&xf86info);

    if (ConfigLoop(KeyboardConfigCheck) == True) {
	if (keyboard == NULL) {
	    keyboard = XtNew(XF86ConfInputRec);
	    keyboard->list.next = NULL;
	    keyboard->inp_identifier = XtNewString(ident_string);
	    keyboard->inp_driver = XtNewString("kbd");
	    keyboard->inp_option_lst = xf86newOption(XtNewString(XkbRules),
						     XtNewString(rules));
	    xf86addNewOption(keyboard->inp_option_lst,
			     XtNewString(XkbModel), XtNewString(model));
	    xf86addNewOption(keyboard->inp_option_lst,
			     XtNewString(XkbLayout), XtNewString(layout));
	    if (variant && *variant)
		xf86addNewOption(keyboard->inp_option_lst,
			    	 XtNewString(XkbVariant), XtNewString(variant));
	    if (options && *options) {
		xf86addNewOption(keyboard->inp_option_lst,
			    	 XtNewString(XkbOptions), options);
		options = NULL;
	    }
	    keyboard->inp_comment = NULL;
	}
	else {
	    int i;
	    char *str;

	    XtSetArg(args[0], XtNlabel, &str);
	    XtGetValues(modelb, args, 1);
	    for (i = 0; i < xkb_rules->model.nelem; i++)
		if (strcmp(xkb_rules->model.desc[i], str) == 0) {
		    model = xkb_rules->model.name[i];
		    break;
		}

	    XtSetArg(args[0], XtNlabel, &str);
	    XtGetValues(layoutb, args, 1);
	    for (i = 0; i < xkb_rules->layout.nelem; i++)
		if (strcmp(xkb_rules->layout.desc[i], str) == 0) {
		    layout = xkb_rules->layout.name[i];
		    break;
		}

	    if ((option = xf86findOption(keyboard->inp_option_lst, XkbRules))
		!= NULL) {
		XtFree(option->opt_val);
		option->opt_val = XtNewString(rules);
		XtFree(option->opt_comment);
		option->opt_comment = NULL;
	    }
	    else
		keyboard->inp_option_lst =
		    xf86addNewOption(keyboard->inp_option_lst,
				     XtNewString(XkbRules), XtNewString(rules));

	    if ((option = xf86findOption(keyboard->inp_option_lst, XkbModel))
		!= NULL) {
		XtFree(option->opt_val);
		option->opt_val = XtNewString(model);
		XtFree(option->opt_comment);
		option->opt_comment = NULL;
	    }
	    else
		keyboard->inp_option_lst =
		    xf86addNewOption(keyboard->inp_option_lst,
				     XtNewString(XkbModel), XtNewString(model));
	    XtFree(xkb_info->config.model);
	    xkb_info->config.model = XtNewString(model);

	    if ((option = xf86findOption(keyboard->inp_option_lst, XkbLayout))
		!= NULL) {
		XtFree(option->opt_val);
		option->opt_val = XtNewString(layout);
	    }
	    else
		keyboard->inp_option_lst =
		    xf86addNewOption(keyboard->inp_option_lst,
				     XtNewString(XkbLayout), XtNewString(layout));
	    XtFree(xkb_info->config.layout);
	    xkb_info->config.layout = XtNewString(layout);

	    if ((option = xf86findOption(keyboard->inp_option_lst, XkbVariant))
		!= NULL) {
		if (variant && *variant) {
		    XtFree(option->opt_val);
		    option->opt_val = XtNewString(variant);
		}
		else
		    xf86removeOption(&keyboard->inp_option_lst, XkbVariant);
	    }
	    else if (variant && *variant)
		xf86addNewOption(keyboard->inp_option_lst,
				 XtNewString(XkbVariant), XtNewString(variant));
	    XtFree(xkb_info->config.variant);
	    xkb_info->config.variant = variant && *variant ?
		XtNewString(variant) : NULL;

	    XtFree(xkb_info->config.options);
	    xkb_info->config.options = options && *options ?
		XtNewString(options) : NULL;
	    if ((option = xf86findOption(keyboard->inp_option_lst, XkbOptions))
		!= NULL) {
		if (options && *options) {
		    XtFree(option->opt_val);
		    option->opt_val = options;
		    options = NULL;
		}
		else
		    xf86removeOption(&keyboard->inp_option_lst, XkbOptions);
	    }
	    else if (options && *options) {
		xf86addNewOption(keyboard->inp_option_lst,
				 XtNewString(XkbOptions), options);
		options = NULL;
	    }
	}
	if (strcasecmp(keyboard->inp_identifier, ident_string))
	    xf86renameInput(XF86Config, keyboard, ident_string);

	xkb_info->conf = keyboard;
	xkb_info->config.rules_file = rules;

	return ((XtPointer)keyboard);
    }

    xkb_rules = info;
    rules = info->rules;
    model = omodel;
    layout = olayout;
    variant = ovariant;
    XtFree(options);
    options = ooptions;

    return (NULL);
}

static Bool
KeyboardConfigCheck(void)
{
    XF86ConfInputPtr keyboard = XF86Config->conf_input_lst;

    while (keyboard != NULL) {
	if (keyboard != current_input &&
	    strcasecmp(ident_string, keyboard->inp_identifier) == 0)
	    return (False);
	keyboard = (XF86ConfInputPtr)(keyboard->list.next);
    }

    return (True);
}

/*ARGSUSED*/
static void
XkbUIEventHandler(Widget w, XtPointer closure,
		  XEvent *event, Boolean *continue_to_dispatch)
{
    XkbUI_ViewOptsRec opts;
    XkbUI_ViewPtr view;
    int width, height, bd;

    if (event->xexpose.count > 1)
	return;

    bzero((char *)&opts, sizeof(opts));
    bd = 1;
    opts.present = XkbUI_SizeMask | XkbUI_ColormapMask |
		   XkbUI_MarginMask | XkbUI_OffsetMask;
    opts.margin_width = opts.margin_height = 0;
    opts.viewport.x = opts.viewport.y = bd;
    width = opts.viewport.width = w->core.width - 2 * bd;
    height = opts.viewport.height = w->core.height - 2 * bd;
    opts.cmap = w->core.colormap;

    if ((view = XkbUI_Init(XtDisplay(w), XtWindow(w), width, height,
	xkb_info->xkb, &opts)) != NULL) {
	XkbUI_DrawRegion(view, NULL);
	free(view);
    }
}

void
InitializeKeyboard(void)
{
    int major, minor, op, event, error;
    static int first = 1;
    int timeout = 5;
    XF86ConfInputPtr keyboard = XF86Config->conf_input_lst;
    XF86OptionPtr option;
    char name[PATH_MAX];
    FILE *file;

    if (!first)
	return;
    first = 0;

    major = XkbMajorVersion;
    minor = XkbMinorVersion;
    if (XkbQueryExtension(DPY, &op, &event, &error, &major, &minor) == 0) {
	fprintf(stderr, "Unable to initialize XKEYBOARD extension");
	exit(1);
    }

    xkb_info = (XkbInfo *)XtCalloc(1, sizeof(XkbInfo));
    xkb_info->conf = NULL;
    xkb_infos = (XkbInfo**)XtCalloc(1, sizeof(XkbInfo*));
    num_xkb_infos = 1;
    xkb_infos[0] = xkb_info;
    bzero((char*)&(xkb_info->defs), sizeof(XkbRF_VarDefsRec));

    while (timeout > 0) {
	xkb_info->xkb =
	    XkbGetKeyboard(DPY, XkbGBN_AllComponentsMask, XkbUseCoreKbd);
	if (xkb_info->xkb == NULL) {
	    timeout -= 1;
	    sleep(1);
	}
	else
	    break;
    }
    if (timeout <= 0) {
	fprintf(stderr, "Couldn't get keyboard\n");
    }
    if (xkb_info->xkb && xkb_info->xkb->names && xkb_info->xkb->geom &&
	xkb_info->xkb->names->geometry == 0)
	xkb_info->xkb->names->geometry = xkb_info->xkb->geom->name;

    /* Load configuration */
    XmuSnprintf(name, sizeof(name), "%s%s", XkbConfigDir, XkbConfigFile);
    file = fopen(name, "r");
    if (file != NULL) {
	if (XkbCFParse(file, XkbCFDflts, xkb_info->xkb, &xkb_info->config) == 0) {
	    fprintf(stderr, "Error parsing config file: ");
	    XkbCFReportError(stderr, name, xkb_info->config.error,
			     xkb_info->config.line);
	}
	fclose(file);
    }

    xkb_rules = GetXkbRulesDesc(xkb_info->config.rules_file != NULL ?
				xkb_info->config.rules_file : XkbRulesFile);
    if (xkb_rules == NULL)
    /* error message was printed */
	exit(1);

    /* XXX Assumes the first keyboard is the core keyboard */
    while (keyboard != NULL) {
	if (IS_KBDDRIV(keyboard->inp_driver))
	    break;
	keyboard = (XF86ConfInputPtr)(keyboard->list.next);
    }
    if (keyboard == NULL)
	return;

    if (xkb_info->config.rules_file != NULL)
	rules = xkb_info->config.rules_file;
    else if ((option = xf86findOption(keyboard->inp_option_lst, "XkbRules"))
	!= NULL)
	rules = option->opt_val;
    else
	rules = XkbRulesFile;

    if (strcmp(rules, xkb_rules->rules)) {
	xkb_rules = GetXkbRulesDesc(rules);
	if (xkb_rules == NULL)
	/* error message was printed */
	    exit(1);
    }
    {
	FILE *fp;
	char filename[1024];

	XmuSnprintf(filename, sizeof(filename), "%s%s",
		    XkbRulesDir, xkb_rules->rules);
	if ((fp = fopen(filename, "r")) == NULL) {
	   fprintf(stderr, "Can't open rules file\n");
	   exit(1);
	}

       if (!XkbRF_LoadRules(fp, xkb_rules->list)) {
	    fclose(fp);
	    fprintf(stderr, "Can't load rules\n");
	    exit(1);
	}
	fclose(fp);
    }

    if (xkb_info->config.rules_file == NULL)
	xkb_info->config.rules_file = xkb_rules->rules;

    if (xkb_info->config.model != NULL)
	xkb_info->defs.model = xkb_info->config.model;
    else if ((option = xf86findOption(keyboard->inp_option_lst, "XkbModel"))
	!= NULL)
	xkb_info->defs.model = option->opt_val;
    else
	xkb_info->defs.model = xkb_rules->model.name[0];

    if (xkb_info->config.layout != NULL)
	xkb_info->defs.layout = xkb_info->config.layout;
    else if ((option = xf86findOption(keyboard->inp_option_lst, "XkbLayout"))
	!= NULL)
	xkb_info->defs.layout = option->opt_val;
    else
	xkb_info->defs.layout = xkb_rules->layout.name[0];

    if (xkb_info->config.variant != NULL)
	xkb_info->defs.variant = xkb_info->config.variant;
    else if ((option = xf86findOption(keyboard->inp_option_lst, "XkbVariant"))
	!= NULL)
	xkb_info->defs.variant = option->opt_val;
    else
	xkb_info->defs.variant = NULL;

    if (xkb_info->config.options != NULL)
	xkb_info->defs.options = xkb_info->config.options;
    else if ((option = xf86findOption(keyboard->inp_option_lst, "XkbOptions"))
	!= NULL)
	xkb_info->defs.options = option->opt_val;
    else
	xkb_info->defs.options = NULL;

    if (xkb_info->xkb == NULL) {
	/* Try again */
	XkbComponentNamesRec comps;

	bzero((char*)&comps, sizeof(XkbComponentNamesRec));
	XkbRF_GetComponents(xkb_rules->list, &(xkb_info->defs), &comps);

	xkb_info->xkb = XkbGetKeyboardByName(DPY, XkbUseCoreKbd, &comps,
					     XkbGBN_AllComponentsMask, 0, 0);
    }
}

static XF86XkbRulesDescInfo *
GetXkbRulesDesc(char *rules)
{
    int i;
    XkbRF_RulesPtr list;
    char filename[1024];
    XF86XkbRulesDescInfo *info;

    if (rules == NULL)
	return (NULL);

    for (i = 0; i < num_xkb_desc; i++)
	if (strcmp(rules, xkb_desc[i]->rules) == 0)
	    return (xkb_desc[i]);

    XmuSnprintf(filename, sizeof(filename), "%s%s", XkbRulesDir, rules);
    if ((list = XkbRF_Create(0, 0)) == NULL ||
	!XkbRF_LoadDescriptionsByName(filename, NULL, list)) {
	fprintf(stderr, "Can't create rules structure\n");
	return (NULL);
    }

    info = (XF86XkbRulesDescInfo*)XtCalloc(1, sizeof(XF86XkbRulesDescInfo));
    xkb_desc = (XF86XkbRulesDescInfo**)
	XtRealloc((XtPointer)xkb_desc,
		  sizeof(XF86XkbRulesDescInfo*) * (num_xkb_desc + 1));
    xkb_desc[num_xkb_desc++] = info;
    info->rules = XtNewString(rules);
    for (i = 0; i < list->models.num_desc; i++) {
	if (i % 16 == 0) {
	    info->model.name = (char**)XtRealloc((XtPointer)info->model.name,
						 (i + 16) * sizeof(char*));
	    info->model.desc = (char**)XtRealloc((XtPointer)info->model.desc,
						 (i + 16) * sizeof(char*));
	}
	info->model.name[i] = XtNewString(list->models.desc[i].name);
	info->model.desc[i] = XtNewString(list->models.desc[i].desc);
    }
    info->model.nelem = i;

    for (i = 0; i < list->layouts.num_desc; i++) {
	if (i % 16 == 0) {
	    info->layout.name = (char**)XtRealloc((XtPointer)info->layout.name,
						  (i + 16) * sizeof(char*));
	    info->layout.desc = (char**)XtRealloc((XtPointer)info->layout.desc,
						  (i + 16) * sizeof(char*));
	}
	info->layout.name[i] = XtNewString(list->layouts.desc[i].name);
	info->layout.desc[i] = XtNewString(list->layouts.desc[i].desc);
    }
    info->layout.nelem = i;

    for (i = 0; i < list->variants.num_desc; i++) {
	if (i % 16 == 0) {
	    info->variant.name = (char**)XtRealloc((XtPointer)info->variant.name,
						   (i + 16) * sizeof(char*));
	    info->variant.desc = (char**)XtRealloc((XtPointer)info->variant.desc,
						   (i + 16) * sizeof(char*));
	}
	info->variant.name[i] = XtNewString(list->variants.desc[i].name);
	info->variant.desc[i] = XtNewString(list->variants.desc[i].desc);
    }
    info->variant.nelem = i;

    for (i = 0; i < list->options.num_desc; i++) {
	if (i % 16 == 0) {
	    info->option.name = (char**)XtRealloc((XtPointer)info->option.name,
						  (i + 16) * sizeof(char*));
	    info->option.desc = (char**)XtRealloc((XtPointer)info->option.desc,
						  (i + 16) * sizeof(char*));
	}
	info->option.name[i] = XtNewString(list->options.desc[i].name);
	info->option.desc[i] = XtNewString(list->options.desc[i].desc);
    }
    info->option.nelem = i;
    info->list = list;

    return (info);
}

static xf86ConfigSymTabRec ax_controls[] =
{
    {XkbRepeatKeysMask,	     "RepeatKeys"},
    {XkbSlowKeysMask,	     "SlowKeys"},
    {XkbBounceKeysMask,	     "BounceKeys"},
    {XkbStickyKeysMask,	     "StickyKeys"},
    {XkbMouseKeysMask,	     "MouseKeys"},
    {XkbMouseKeysAccelMask,  "MouseKeysAccel"},
    {XkbAccessXKeysMask,     "AccessxKeys"},
    {XkbAccessXTimeoutMask,  "AccessxTimeout"},
    {XkbAccessXFeedbackMask, "AccessxFeedback"},
    {XkbAudibleBellMask,     "AudibleBell"},
    {XkbOverlay1Mask,	     "Overlay1"},
    {XkbOverlay2Mask,	     "Overlay2"},
    {XkbIgnoreGroupLockMask, "IgnoreGroupLock"},
    {-1,		     ""},
};

static xf86ConfigSymTabRec ax_feedback[] =
{
    {XkbAX_SKPressFBMask,    "SlowKeysPress"},
    {XkbAX_SKAcceptFBMask,   "SlowKeysAccept"},
    {XkbAX_FeatureFBMask,    "Feature"},
    {XkbAX_SlowWarnFBMask,   "SlowWarn"},
    {XkbAX_IndicatorFBMask,  "Indicator"},
    {XkbAX_StickyKeysFBMask, "StickyKeys"},
    {XkbAX_TwoKeysMask,	     "TwoKeys"},
    {XkbAX_LatchToLockMask,  "LatchToLock"},
    {XkbAX_SKReleaseFBMask,  "SlowKeysRelease"},
    {XkbAX_SKRejectFBMask,   "SlowkeysReject"},
    {XkbAX_BKRejectFBMask,   "BounceKeysReject"},
    {XkbAX_DumbBellFBMask,   "DumbBell"},
    {-1,		     ""},
};

Bool
WriteXKBConfiguration(char *filename, XkbConfigRtrnPtr conf)
{
    FILE *fp;
    int i, count;

    if (filename == NULL || conf == NULL ||
	(fp = fopen(filename, "w")) == NULL)
	return (False);

    if (conf->rules_file != NULL)
	fprintf(fp, "Rules			 =	\"%s\"\n",
		conf->rules_file);
    if (conf->model != NULL)
	fprintf(fp, "Model			 =	\"%s\"\n",
		conf->model);
    if (conf->layout != NULL)
	fprintf(fp, "Layout			 =	\"%s\"\n",
		conf->layout);
    if (conf->variant != NULL)
	fprintf(fp, "Variant			 =	\"%s\"\n",
		conf->variant);
    if (conf->options != NULL)
	fprintf(fp, "Options			 =	\"%s\"\n",
		conf->options);
    if (conf->keymap != NULL)
	fprintf(fp, "Keymap			 =	%s\n",
		conf->keymap);
    if (conf->keycodes != NULL)
	fprintf(fp, "Keycodes		 =	%s\n",
		conf->keycodes);
    if (conf->geometry != NULL)
	fprintf(fp, "Geometry		 =	%s\n",
		conf->geometry);
    if (conf->phys_symbols != NULL)
	fprintf(fp, "RealSymbols		 =	%s\n",
		conf->phys_symbols);
    if (conf->symbols != NULL)
	fprintf(fp, "Symbols			 =	%s\n",
		conf->symbols);
    if (conf->types != NULL)
	fprintf(fp, "Types			 =	%s\n",
		conf->types);
    if (conf->compat != NULL)
	fprintf(fp, "Compat			 =	%s\n",
		conf->compat);

    if (conf->click_volume > 0)
	fprintf(fp, "ClickVolume		 =	%d\n",
		conf->click_volume);
    if (conf->bell_volume > 0)
	fprintf(fp, "BellVolume		 =	%d\n",
		conf->bell_volume);
    if (conf->bell_pitch > 0)
	fprintf(fp, "BellPitch		 =	%d\n",
		conf->bell_pitch);
    if (conf->bell_duration > 0)
	fprintf(fp, "BellDuration		 =	%d\n",
		conf->bell_duration);

    if (conf->repeat_delay > 0)
	fprintf(fp, "RepeatDelay		 =	%d\n",
		conf->repeat_delay);
    if (conf->repeat_interval > 0)
	fprintf(fp, "RepeatInterval		 =	%d\n",
		conf->repeat_interval);

    if (conf->slow_keys_delay > 0)
	fprintf(fp, "SlowKeysDelay		 =	%d\n",
		conf->slow_keys_delay);

    if (conf->debounce_delay > 0)
	fprintf(fp, "DebounceDelay		 =	%d\n",
		conf->debounce_delay);

    if (conf->mk_delay > 0)
	fprintf(fp, "MouseKeysDelay		 =	%d\n",
		conf->mk_delay);
    if (conf->mk_interval > 0)
	fprintf(fp, "MouseKeysInterval	 =	%d\n",
		conf->mk_interval);
    if (conf->mk_time_to_max > 0)
	fprintf(fp, "MouseKeysTimeToMax	 =	%d\n",
		conf->mk_time_to_max);
    if (conf->mk_max_speed > 0)
	fprintf(fp, "MouseKeysMaxSpeed	 =	%d\n",
		conf->mk_max_speed);
    fprintf(fp, "MouseKeysCurve		 =	%d\n", conf->mk_curve);

    if (conf->ax_timeout)
	fprintf(fp, "AccessXTimeout		 =	%d\n",
		conf->ax_timeout);
    if (conf->initial_ctrls != 0) {
	fprintf(fp, "Controls		%c=	",
		conf->replace_initial_ctrls ? ' ' : '+');
	for (i = count = 0; *ax_controls[i].name; i++)
	    if ((conf->initial_ctrls & ax_controls[i].token)
		== ax_controls[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_controls[i].name);
	fprintf(fp, "\n");
    }
    if (conf->axt_ctrls_on != 0) {
	fprintf(fp, "AcessXTimeoutCtrlsOn	%c=	",
		conf->replace_axt_ctrls_on ? ' ' : '+');
	for (i = count = 0; *ax_controls[i].name; i++)
	    if ((conf->axt_ctrls_on & ax_controls[i].token)
		== ax_controls[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_controls[i].name);
	fprintf(fp, "\n");
    }
    if (conf->axt_ctrls_off != 0) {
	fprintf(fp, "AcessXTimeoutCtrlsOff	%c=	",
		conf->replace_axt_ctrls_off ? ' ' : '-');
	for (i = count = 0; *ax_controls[i].name; i++)
	    if ((conf->axt_ctrls_off & ax_controls[i].token)
		== ax_controls[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_controls[i].name);
	fprintf(fp, "\n");
    }

    if (conf->initial_opts != 0) {
	fprintf(fp, "Feedback		%c=	",
		conf->replace_initial_opts ? ' ' : '+');
	for (i = count = 0; *ax_feedback[i].name; i++)
	    if ((conf->initial_opts & ax_feedback[i].token)
		== ax_feedback[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_feedback[i].name);
	fprintf(fp, "\n");
    }
    if (conf->axt_opts_on != 0) {
	fprintf(fp, "AcessXTimeoutFeedbackOn	%c=	",
		conf->replace_axt_opts_on ? ' ' : '+');
	for (i = count = 0; *ax_controls[i].name; i++)
	    if ((conf->axt_opts_on & ax_feedback[i].token)
		== ax_feedback[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_feedback[i].name);
	fprintf(fp, "\n");
    }
    if (conf->axt_opts_off != 0) {
	fprintf(fp, "AcessXTimeoutFeedbackOff%c=	",
		conf->replace_axt_opts_off ? ' ' : '-');
	for (i = count = 0; *ax_feedback[i].name; i++)
	    if ((conf->axt_opts_off & ax_feedback[i].token)
		== ax_feedback[i].token)
		fprintf(fp, "%s%s", count++ ? " + " : "",
			ax_feedback[i].name);
	fprintf(fp, "\n");
    }

    fclose(fp);

    return (True);
}

Bool
UpdateKeyboard(Bool load)
{
    XkbComponentNamesRec comps;
    XkbDescPtr xkb;

    bzero((char*)&comps, sizeof(XkbComponentNamesRec));
    XkbRF_GetComponents(xkb_rules->list, &(xkb_info->defs), &comps);

    xkb = XkbGetKeyboardByName(DPY, XkbUseCoreKbd, &comps,
			       XkbGBN_AllComponentsMask, 0, load);

    if (xkb == NULL || xkb->geom == NULL) {
	fprintf(stderr, "Couldn't get keyboard\n");
	return (False);
    }
    if (xkb_info->xkb && xkb_info->xkb->names && xkb_info->xkb->geom &&
	xkb_info->xkb->names->geometry == 0)
	xkb_info->xkb->names->geometry = xkb_info->xkb->geom->name;

    XkbFreeKeyboard(xkb_info->xkb, 0, False);

    xkb_info->xkb = xkb;

    XtFree(comps.keymap);
    XtFree(comps.keycodes);
    XtFree(comps.compat);
    XtFree(comps.types);
    XtFree(comps.symbols);
    XtFree(comps.geometry);

    if (kbd != NULL)
	XClearArea(XtDisplay(configp), XtWindow(kbd), 0, 0, 0, 0, True);

    return (True);
}

static void
KeyboardRulesCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    int i;
    FILE *fp;
    Arg args[1];
    char filename[1024], *omodel, *olayout, *ovariant, *ooptions,
	 *dmodel, *dlayout, *dvariant;
    XF86XkbRulesDescInfo *oxkb_rules, *info = GetXkbRulesDesc(XtName(w));

    if (strcmp(XtName(w), rules) == 0 || info == NULL)
    /* a error message was printed */
	return;

    XmuSnprintf(filename, sizeof(filename), "%s%s",
		XkbRulesDir, info->rules);
    if ((fp = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "Can't open rules file\n");
	return;
    }

    if (!XkbRF_LoadRules(fp, info->list)) {
	fclose(fp);
	fprintf(stderr, "Can't load rules\n");
	return;
    }
    fclose(fp);

    oxkb_rules = xkb_rules;
    omodel = xkb_info->defs.model;
    olayout = xkb_info->defs.layout;
    ovariant = xkb_info->defs.variant;
    ooptions = xkb_info->defs.options;

    if (omodel) {
	for (i = 0; i < info->model.nelem; i++) {
	    if (strcmp(omodel, info->model.name[i]) == 0)
		break;
	}
    }
    else
	i = 0;
    model = xkb_info->defs.model = info->model.name
	[i < info->model.nelem ? i : 0];
    dmodel = info->model.desc[i < info->model.nelem ? i : 0];

    if (olayout) {
	for (i = 0; i < info->layout.nelem; i++) {
	    if (strcmp(olayout, info->layout.name[i]) == 0)
		break;
	}
    }
    else
	i = 0;
    layout = xkb_info->defs.layout = info->layout.name
	[i < info->layout.nelem ? i : 0];
    dlayout = info->layout.desc[i < info->layout.nelem ? i : 0];

    if (ovariant) {
	for (i = 0; i < info->variant.nelem; i++) {
	    if (strcmp(ovariant, info->variant.name[i]) == 0)
		break;
	}
    }
    else
	i = info->variant.nelem;
    variant = xkb_info->defs.variant = i < info->variant.nelem ?
	info->variant.name[i] : NULL;
    dvariant = i < info->variant.nelem ?
	info->variant.desc[i] : NULL;

    if (ooptions) {
	char *ptr, *tmp = XtNewString(options);

	for (ptr = strtok(tmp, ","); ptr != NULL; ptr = strtok(NULL, ",")) {
	    if (strchr(ptr, ':') == NULL)
		continue;

	    for (i = 0; i < xkb_rules->option.nelem; i++)
		if (strcmp(xkb_rules->option.name[i], ptr) == 0)
		    break;

	    if (i == xkb_rules->option.nelem) {
		XtFree(options);
		options = NULL;
		/* no option with the same name */
		break;
	    }
	}
	XtFree(tmp);
    }
    else {
	XtFree(options);
	options = NULL;
    }

    oxkb_rules = xkb_rules;
    xkb_rules = info;
    rules = info->rules;

    if (!UpdateKeyboard(False)) {
	model = xkb_info->defs.model = omodel;
	layout = xkb_info->defs.layout = olayout;
	variant = xkb_info->defs.variant = ovariant;
	options = XtNewString(xkb_info->defs.options = ooptions);
	xkb_rules = oxkb_rules;
	rules = xkb_rules->rules;

	XmuSnprintf(filename, sizeof(filename), "%s%s",
		    XkbRulesDir, rules);
	if ((fp = fopen(filename, "r")) == NULL) {
	    fprintf(stderr, "Can't open rules file\n");
	    return;
	}

	if (!XkbRF_LoadRules(fp, xkb_rules->list)) {
	    fprintf(stderr, "Can't load rules\n");
	}
	fclose(fp);

	return;
    }

    UpdateRulesPopups();

    XtSetArg(args[0], XtNlabel, rules);
    XtSetValues(rulesb, args, 1);

    XtSetArg(args[0], XtNlabel, dmodel);
    XtSetValues(modelb, args, 1);

    XtSetArg(args[0], XtNlabel, dlayout);
    XtSetValues(layoutb, args, 1);

    XtSetArg(args[0], XtNlabel, dvariant ? dvariant : "");
    XtSetValues(variantb, args, 1);

    XtSetArg(args[0], XtNlabel, options ? options : "");
    XtSetValues(variantb, args, 1);
}

static void
KeyboardModelCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    int i;
    char *oldval = xkb_info->defs.model;

    for (i = 0; i < xkb_rules->model.nelem; i++)
	if (strcmp(XtName(w), xkb_rules->model.name[i]) == 0)
	    break;
    model = xkb_info->defs.model = xkb_rules->model.name[i];
    if (!UpdateKeyboard(False))
	model = xkb_info->defs.model = oldval;
    else {
	XtSetArg(args[0], XtNlabel, xkb_rules->model.desc[i]);
	XtSetValues(modelb, args, 1);
    }
}

static void
KeyboardLayoutCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    int i;
    char *oldval = xkb_info->defs.layout;

    for (i = 0; i < xkb_rules->layout.nelem; i++)
	if (strcmp(XtName(w), xkb_rules->layout.name[i]) == 0)
	    break;
    layout = xkb_info->defs.layout = xkb_rules->layout.name[i];
    if (!UpdateKeyboard(False))
	layout = xkb_info->defs.layout = oldval;
    else {
	XtSetArg(args[0], XtNlabel, xkb_rules->layout.desc[i]);
	XtSetValues(layoutb, args, 1);
    }
}

static void
KeyboardVariantCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    int i;
    char *label, *oldval = xkb_info->defs.variant;

    for (i = 0; i < xkb_rules->variant.nelem; i++)
	if (strcmp(XtName(w), xkb_rules->variant.name[i]) == 0)
	    break;
    variant = i < xkb_rules->variant.nelem ? xkb_rules->variant.name[i] : "";
    xkb_info->defs.variant = variant && *variant ? variant : NULL;

    if (!UpdateKeyboard(False))
	xkb_info->defs.variant = variant = oldval;
    else {
	label = i < xkb_rules->variant.nelem ? xkb_rules->variant.desc[i] : "";
	XtSetArg(args[0], XtNlabel, label);
	XtSetValues(variantb, args, 1);
    }
}

static void
KeyboardOptionsCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    Arg args[1];
    int i;

    for (i = 0; i < xkb_rules->option.nelem; i++)
	if (strcmp(XtName(w), xkb_rules->option.name[i]) == 0)
	    break;

    if (i < xkb_rules->option.nelem) {
	char *delim, *ptr, str[256];

	/* remove old string, don't check if the same */
	if ((delim = strchr(XtName(w), ':')) != NULL) {
	    if (delim - XtName(w) >= sizeof(str) - 2)
		return;
	    strncpy(str, XtName(w), delim - XtName(w) + 1);
	    str[delim - XtName(w) + 1] = '\0';
	}
	else
	    XmuSnprintf(str, sizeof(str), "%s:", XtName(w));
	if (options && (delim = strstr(options, str)) != NULL) {
	    if ((ptr = strchr(delim, ',')) != NULL) {
		*delim = *ptr = '\0';
		XmuSnprintf(str, sizeof(str), "%s%s", options, ptr + 1);
		XtFree(options);
		options = XtNewString(str);
	    }
	    else {
		if (delim > options)
		    delim[-1] = '\0';
		else
		    delim[0] = '\0';
	    }
	}

	/* update string, if required */
	if ((delim = strchr(XtName(w), ':')) != NULL) {
	    if (options && *options)
		XmuSnprintf(str, sizeof(str), "%s,%s", options, XtName(w));
	    else
		XmuSnprintf(str, sizeof(str), "%s", XtName(w));
	    XtFree(options);
	    options = XtNewString(str);
	}
    }
    else {
	XtFree(options);
	options = XtNewString("");
    }

    if (options == NULL)
	options = XtNewString("");

    xkb_info->defs.options = options;
    if (!UpdateKeyboard(False)) {
	*options = '\0';
	xkb_info->defs.options = NULL;
    }
    XtSetArg(args[0], XtNlabel, options);
    XtSetValues(optionsb, args, 1);
    XtSetArg(args[0], XtNtip, options);
    XtSetValues(optionsb, args, 1);
}

/*ARGSUSED*/
static void
KeyboardApplyCallback(Widget w, XtPointer user_data, XtPointer call_data)
{
    (void)UpdateKeyboard(True);
}

static void
UpdateRulesPopups(void)
{
    int i;
    char *optname;
    Widget sme, optpopup = NULL, optparent;

    /* MODEL */
    if (modelp)
	XtDestroyWidget(modelp);
    modelp = XtCreatePopupShell("modelP", simpleMenuWidgetClass,
				modelb, NULL, 0);
    for (i = 0; i < xkb_rules->model.nelem; i++) {
	sme = XtVaCreateManagedWidget(xkb_rules->model.name[i], smeBSBObjectClass,
				      modelp,
				      XtNlabel, xkb_rules->model.desc[i],
				      NULL);
	XtAddCallback(sme, XtNcallback,  KeyboardModelCallback, NULL);
    }

    /* LAYOUT */
    if (layoutp)
	XtDestroyWidget(layoutp);
    layoutp = XtCreatePopupShell("layoutP", simpleMenuWidgetClass,
				 layoutb, NULL, 0);
    for (i = 0; i < xkb_rules->layout.nelem; i++) {
	sme = XtVaCreateManagedWidget(xkb_rules->layout.name[i], smeBSBObjectClass,
				      layoutp,
				      XtNlabel, xkb_rules->layout.desc[i],
				      NULL);
	XtAddCallback(sme, XtNcallback,  KeyboardLayoutCallback, NULL);
    }

    /* VARIANT */
    if (variantp)
	XtDestroyWidget(variantp);
    variantp = XtCreatePopupShell("variantP", simpleMenuWidgetClass,
				  variantb, NULL, 0);
    sme = XtVaCreateManagedWidget("None", smeBSBObjectClass,
				  variantp,
				  XtNlabel, "None",
				  NULL);
    XtAddCallback(sme, XtNcallback,  KeyboardVariantCallback, NULL);
    for (i = 0; i < xkb_rules->variant.nelem; i++) {
	sme = XtVaCreateManagedWidget(xkb_rules->variant.name[i], smeBSBObjectClass,
				      variantp,
				      XtNlabel, xkb_rules->variant.desc[i],
				      NULL);
	XtAddCallback(sme, XtNcallback,  KeyboardVariantCallback, NULL);
    }

    /* OPTIONS */
    if (optionsp)
	XtDestroyWidget(optionsp);
    optionsp = XtCreatePopupShell("optionsP", simpleMenuWidgetClass,
			       optionsb, NULL, 0);
    sme = XtVaCreateManagedWidget("None", smeBSBObjectClass,
				  optionsp,
				  XtNlabel, "None",
				  NULL);
    XtAddCallback(sme, XtNcallback,  KeyboardOptionsCallback, NULL);
    optparent = optionsp;
    optname = NULL;
    for (i = 0; i < xkb_rules->option.nelem; i++) {
	if (!strchr(xkb_rules->option.name[i], ':')) {
	    optpopup =
		XtCreatePopupShell(optname = xkb_rules->option.desc[i],
				   simpleMenuWidgetClass,
				   optparent = optionsp, NULL, 0);
	    sme = XtVaCreateManagedWidget(xkb_rules->option.name[i],
					  smeBSBObjectClass,
					  optpopup,
					  XtNlabel, "None",
					  NULL);
	    XtAddCallback(sme, XtNcallback,  KeyboardOptionsCallback, NULL);
	}
	else {
	    optparent = optpopup;
	    optname = NULL;
	}
	sme = XtVaCreateManagedWidget(xkb_rules->option.name[i], smeBSBObjectClass,
				      optparent,
				      XtNlabel, xkb_rules->option.desc[i],
				      XtNmenuName, optname,
				      XtNleftBitmap, optname ? menuPixmap : None,
				      NULL);
	if (optparent != optionsp)
	    XtAddCallback(sme, XtNcallback,  KeyboardOptionsCallback, NULL);
    }
}

void
KeyboardModelAndLayout(XF86SetupInfo *info)
{
    static int first = 1;
    static Widget kbdml;
    Arg args[1];
    int i;

    if (first) {
	Widget popup, sme;

	first = 0;

	kbdml = XtCreateWidget("keyboardML", formWidgetClass,
			       configp, NULL, 0);

	/* RULES */
	XtCreateManagedWidget("labelR", labelWidgetClass, kbdml, NULL, 0);
	rulesb = XtVaCreateManagedWidget("rules", menuButtonWidgetClass, kbdml,
					 XtNmenuName, "rulesP",
					 NULL);
	popup = XtCreatePopupShell("rulesP", simpleMenuWidgetClass,
				   rulesb, NULL, 0);
	{
	    struct dirent *ent;
	    DIR *dir;

	    if ((dir = opendir(XkbRulesDir)) != NULL) {
		(void)readdir(dir);
		(void)readdir(dir);
		while ((ent = readdir(dir)) != NULL) {
		    if (strchr(ent->d_name, '.'))
			continue;

		    sme = XtVaCreateManagedWidget(ent->d_name, smeBSBObjectClass,
						  popup,
						  XtNlabel, ent->d_name,
						  NULL);
		    XtAddCallback(sme, XtNcallback,  KeyboardRulesCallback, NULL);
		}
		closedir(dir);
	    }
	}

	/* MODEL */
	XtCreateManagedWidget("labelM", labelWidgetClass, kbdml, NULL, 0);
	modelb = XtVaCreateManagedWidget("model", menuButtonWidgetClass, kbdml,
					 XtNmenuName, "modelP",
					 NULL);

	/* LAYOUT */
	XtCreateManagedWidget("labelL", labelWidgetClass, kbdml, NULL, 0);
	layoutb = XtVaCreateManagedWidget("layout", menuButtonWidgetClass, kbdml,
					  XtNmenuName, "layoutP",
					  XtNlabel, xkb_rules->layout.desc[0],
					  NULL);

	/* VARIANT */
	XtCreateManagedWidget("labelV", labelWidgetClass, kbdml, NULL, 0);
	variantb = XtVaCreateManagedWidget("variant", menuButtonWidgetClass, kbdml,
					  XtNmenuName, "variantP",
					  XtNlabel, "",
					  NULL);

	/* OPTIONS */
	XtCreateManagedWidget("labelO", labelWidgetClass, kbdml, NULL, 0);
	optionsb = XtVaCreateManagedWidget("options", menuButtonWidgetClass, kbdml,
					   XtNmenuName, "optionsP",
					   XtNlabel, "",
					   NULL);

	UpdateRulesPopups();

	kbd = XtCreateManagedWidget("keyboard", coreWidgetClass,
				    kbdml, NULL, 0);

	apply = XtCreateManagedWidget("apply", commandWidgetClass,
				      kbdml, NULL, 0);
	XtAddCallback(apply, XtNcallback, KeyboardApplyCallback, NULL);

	XtRealizeWidget(kbdml);

	XtAddEventHandler(kbd, ExposureMask, False, XkbUIEventHandler, NULL);
	/* Force the first update */
	XClearArea(XtDisplay(kbd), XtWindow(kbd), 0, 0, 0, 0, True);
    }

    XtSetArg(args[0], XtNlabel, xkb_rules->rules);
    XtSetValues(rulesb, args, 1);

    for (i = 0; i < xkb_rules->model.nelem; i++)
	if (strcmp(model, xkb_rules->model.name[i]) == 0) {
	    XtSetArg(args[0], XtNlabel, xkb_rules->model.desc[i]);
	    XtSetValues(modelb, args, 1);
	    break;
	}

    for (i = 0; i < xkb_rules->layout.nelem; i++)
	if (strcmp(layout, xkb_rules->layout.name[i]) == 0) {
	    XtSetArg(args[0], XtNlabel, xkb_rules->layout.desc[i]);
	    XtSetValues(layoutb, args, 1);
	    break;
	}

    if (variant)
	for (i = 0; i < xkb_rules->variant.nelem; i++)
	    if (strcmp(variant, xkb_rules->variant.name[i]) == 0) {
		XtSetArg(args[0], XtNlabel, xkb_rules->variant.desc[i]);
		XtSetValues(variantb, args, 1);
		break;
	    }

    if (options) {
	XtSetArg(args[0], XtNlabel, options);
	XtSetValues(optionsb, args, 1);
    }

    XtChangeManagedSet(&current, 1, NULL, NULL, &kbdml, 1);
    current = kbdml;
}
